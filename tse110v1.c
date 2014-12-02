/************************************************************************
 * File          : tvb380v4.c for TVB390 boards
 * Description   : Device Driver file for TVB380 Boards.
 *                 Select one of "define" in the top of tsp100.h 
 *                 to make specific device driver
 * Author        : Minsuk Lee (mslee@hansung.ac.kr)
 * Creator	: 2005.09
 * Co-Author     : Cho Yong Seob (Yuri Cho)
 * Modified      : 1st Aug. 2003  yuricho
                   11th Dec 2003  yuricho
		   26th Sep. 2005  sskim
		   2nd Jun. 2006 sskim
************************************************************************/
// 1. In playing, doing 2 copy: Board->Kernel->User 
//    ==> alloc DMA buffer in user, and do Board->User DMA

/***********************************************************************/
/* INCLUDES                                                            */
/***********************************************************************/
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/kdev_t.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <asm/dma.h>
#include <asm/io.h>

#include "tsp100.h"		// 

/***********************************************************************/
/* Global Variables                                                    */
/***********************************************************************/
struct TSP_Device {
	int inuse;
	ULONG *reg_base;
	ULONG *sram_base;
	unsigned int dma_base;
};

static struct TSP_Device TSP[MAX_TSP_DEVICE];
static int 	     	 tsp102_major;
static int 	       	 num_tsp102;
static spinlock_t 	 tsp102_lock = SPIN_LOCK_UNLOCKED;

/***********************************************************************/
/* Defines                                                             */
/***********************************************************************/
#define inode_2_tp(inode) (TSP + ((MINOR((inode)->i_rdev) & 0xf) - 1))
#define ARG_PARM1	  (((KCMD_ARGS *)arg)->dwCmdParm1)
#define ARG_PARM2	  (((KCMD_ARGS *)arg)->dwCmdParm2)
#define ARG_PARM3	  (((KCMD_ARGS *)arg)->dwCmdParm3)
#define ARG_BUFFER	  (((KCMD_ARGS *)arg)->pdwBuffer)

/************************************************************************/
// Download sdcxxx.rbf file (firmware)
static void Configure_Altera(ULONG *pl_BaseAddr, KALTERA_RBF *pKAlteraRbf)
{
	ULONG 		maskdata;
	unsigned char 	buf;
	int		i, j;

	pKAlteraRbf->Size = pKAlteraRbf->Size & 0xfffff;    // May not exceed 1MBytes
	
	for (i = 0; i < (int)(pKAlteraRbf->Size); i++) {
		buf = *(pKAlteraRbf->pBuffer + i);
		for (j = 0; j < 8; j++) {	// send a bit clock low to high
			maskdata = (ULONG)((buf & 0x01) << 1);
			writel(0x000000c0 | maskdata, pl_BaseAddr+0x400000); //  dclk = 0 with data  
			writel(0x000000c4 | maskdata, pl_BaseAddr+0x400000); //  dclk = 1 with data  
			buf >>= 1; //    LSB first
		} // j-for loop  		
	} // i-for loop
}		


/************************************************************************/
int tsp102_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, ULONG arg)
{
	struct TSP_Device *tp = inode_2_tp(inode);
	ULONG dwDummy;
	unsigned char* buf=NULL;

	KALTERA_RBF kRBF;
	KCMD_ARGS kCmd;
	copy_from_user(&kCmd, (void*)arg, sizeof(KCMD_ARGS));
	
	switch (cmd) {
		case IOCTL_TEST_TSP :
			break;

		case IOCTL_WRITE_TO_MEMORY :
			writel(kCmd.dwCmdParm2, tp->sram_base + (kCmd.dwCmdParm1 & 0x7fffff));
			break;
			
		case IOCTL_READ_FROM_MEMORY :
			kCmd.dwCmdParm2 = readl(tp->sram_base + (kCmd.dwCmdParm1 & 0x7fffff));
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
			
		case IOCTL_READ_PCI9054_REG :
			kCmd.dwCmdParm2 = readl(tp->reg_base + (kCmd.dwCmdParm1 & 0x3f));
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;

		case IOCTL_WRITE_PCI9054_REG :
			writel(kCmd.dwCmdParm2, tp->reg_base + (kCmd.dwCmdParm1 & 0x3f));
			break;

		case IOCTL_TSP100_EPLD_DOWN :
			copy_from_user(&kRBF, (void*)arg, sizeof(KALTERA_RBF));
			buf = vmalloc(kRBF.Size);
			copy_from_user(buf, (void*)kRBF.pBuffer, kRBF.Size);
			kRBF.pBuffer = buf;
			
			Configure_Altera(tp->sram_base, &kRBF);

			vfree(kRBF.pBuffer);
			break;

		case IOCTL_TVB350_EPLD_DOWN :
			break;

		case IOCTL_FILE_DATA_DOWN :
			kCmd.dwCmdParm2 = copy_from_user((void *)(tp->dma_base), (void *)kCmd.pdwBuffer, kCmd.dwCmdParm1);
			break;
			
		case IOCTL_FILE_DATA_UP :
			kCmd.dwCmdParm2 = copy_to_user((void *)kCmd.pdwBuffer, (void *)(tp->dma_base), kCmd.dwCmdParm1);
			break;

		case IOCTL_GET_DMA_MEMORY :
			//	Set PCI9054 DMA related registers
			writel((ULONG) 0x143, tp->reg_base + 32);
			writel((ULONG) virt_to_bus((void *)(tp->dma_base)), tp->reg_base + 33);
			writel((ULONG) 0x0, tp->reg_base + 34);
			writel((ULONG) TSP_DMA_SIZE, tp->reg_base + 35);
			writel((ULONG) 0x09, tp->reg_base + 36);
			
			dwDummy = readl(tp->reg_base + 0);
			//printk("[DRIVER] in GET_DMA_MEMORY dummy 0 = %lx\n", dwDummy);
			dwDummy = readl(tp->reg_base + 1);
			//printk("[DRIVER] in GET_DMA_MEMORY dummy 1 = %lx\n", dwDummy);
			//printk("[DRIVER] IOCTL, GET_DMA_MEMORY %x %lX\n", cmd, ((DMA_ALLOC_ARGS *)arg)->LinearAddr);
			break;

		default :
			printk("[DRIVER] Wrong IOCTL, %x\n", cmd);
			return -EINVAL;
	}
	
	return 0;
}

/************************************************************************/
int tsp102_open(struct inode *inode, struct file *mfile)
{
	struct TSP_Device *tp;
	unsigned flags;

	if (((MINOR((inode)->i_rdev) & 0xf) - 1) >= num_tsp102)
		return -ENODEV;

	spin_lock_irqsave(&tsp102_lock, flags);
	tp = inode_2_tp(inode);
	if (tp->inuse) {
		spin_unlock_irqrestore(&tsp102_lock, flags);
		return -ENOSPC;
	} else if (!tp->reg_base) {
		spin_unlock_irqrestore(&tsp102_lock, flags);
		return -ENODEV;
	} else
		tp->inuse = 1;
	spin_unlock_irqrestore(&tsp102_lock, flags);

	printk("[DRIVER] Board %d Opened\n", MINOR((inode)->i_rdev) & 0xf);
	return 0;
}

/************************************************************************/
int tsp102_release(struct inode *inode, struct file *mfile)
{
	struct TSP_Device *tp = inode_2_tp(inode);
	tp->inuse = 0;
	printk("[DRIVER] Board %d Closed\n", MINOR((inode)->i_rdev) & 0xf);

	return 0;
}

/************************************************************************/
/* defined in /usr/include/pci/header.h					*/
static unsigned pci_base[] = {
	PCI_BASE_ADDRESS_0,		
	PCI_BASE_ADDRESS_1,
	PCI_BASE_ADDRESS_2,
	PCI_BASE_ADDRESS_3,
	PCI_BASE_ADDRESS_4,
	PCI_BASE_ADDRESS_5,
	0
};

/************************************************************************/
// init_hardware is called just once on init_module()
int init_hardware(void)
{
	struct TSP_Device *tp;
	struct pci_dev *_dev_ = NULL;	
	int i;

    	if (0/*!pcibios_present()*/) {
    		printk("%s: no PCI bios\n",TSP_MODULE_NAME);
		return -1;
	}
	
	for (num_tsp102 = 0; num_tsp102 < MAX_TSP_DEVICE; num_tsp102++) {
		tp = TSP + num_tsp102;
		memset(tp, 0, sizeof(*tp));
		
		_dev_ = pci_get_device(VENDOR, DEV_ID_042X, _dev_/*NULL*/);
		if ( _dev_ == NULL ) 
			break;

		for (i = 0; pci_base[i]; i++) {
			unsigned addr, mask;
			int type;

			pci_read_config_dword(_dev_, pci_base[i], &addr);
			local_irq_disable();
			pci_write_config_dword(_dev_, pci_base[i], ~0);
			pci_read_config_dword(_dev_, pci_base[i], &mask);
			pci_write_config_dword(_dev_, pci_base[i], addr);
			local_irq_enable();				

			if (!mask) {
				printk("region %d does not exists\n", i);
				break;
			}

			if (mask & PCI_BASE_ADDRESS_SPACE) {
				type = 0; mask &= PCI_BASE_ADDRESS_IO_MASK;
			} else {
				type = 1; mask &= PCI_BASE_ADDRESS_MEM_MASK;
			}
			//printk("[DRIVER] PCI Region %d: %s Size 0x%X\n", i, type ? "MEM" : "I/O", ~mask + 1);

			if (type && (~mask + 1) == TSP_REG_SIZE)
				tp->reg_base = (ULONG *)addr;
			if (type && (~mask + 1) == TSP_SRAM_SIZE)
				tp->sram_base = (ULONG *)addr;
		}
		printk("[DRIVER] %d base (P): reg:0x%p, sram:0x%p\n", num_tsp102, tp->reg_base, tp->sram_base);

		// IOREMAP
		tp->reg_base = (ULONG *)ioremap((int)tp->reg_base, TSP_REG_SIZE);
		
		if (!tp->reg_base) {
			printk("[DRIVER] reg_base - ioremap fail\n");
			return -1;
		}
		tp->sram_base = (ULONG *)ioremap((int)tp->sram_base, TSP_SRAM_SIZE);
		if (!tp->sram_base) {
			printk("[DRIVER] sram_base - ioremap fail\n");
			return -1;
		}
		tp->dma_base = (ULONG)__get_dma_pages(GFP_KERNEL, get_order(TSP_DMA_SIZE));
		if (!tp->dma_base) {
			printk("DMA Memory Allocation Failure\n");
			return -1;
		}
	}
	printk("[DRIVER] ************** DEV_ID_042X = 0x%x\n",DEV_ID_042X);

	if (!num_tsp102) {
		printk("[DRIVER] No TSP board found\n");
		return -1;
	} else {
		printk("[DRIVER1] %d TSP Board%s found\n", num_tsp102, (num_tsp102 == 1) ? "" : "s");
		for (tp = TSP; tp->reg_base; tp++) {
			printk("/dev/%s-%d  reg:0x%p, sram:0x%p dma:0x%x\n",TSP_MODULE_NAME,
				(tp - TSP) + 1, tp->reg_base, tp->sram_base, tp->dma_base);
		}
		printk("/dev/%s is redirected to /dev/%s-1\n",TSP_MODULE_NAME,TSP_MODULE_NAME);
	}
	return 0;
}

/************************************************************************/
struct file_operations tsp102_fop = {
	open:		tsp102_open,
	ioctl:		tsp102_ioctl,
	release:	tsp102_release,
	owner:		THIS_MODULE,
};

/************************************************************************/
int tsp102_proc_status(char *buf)
{
	struct TSP_Device *tp;
	char *p = buf;

	p += sprintf(p, "%s: (device major = %d)\n", TSP_MODULE_NAME, tsp102_major);
	for (tp = TSP; tp->reg_base; tp++) {
		p += sprintf(p, "Board %d  reg:0x%p, sram:0x%p dma:0x%x\n",
			(tp - TSP), tp->reg_base, tp->sram_base, tp->dma_base);
	}
	return p - buf;
}

/************************************************************************/
int tsp102_proc_output(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = tsp102_proc_status(page);

	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}

/************************************************************************/
int tse110_init_module(void)
{
	if (init_hardware() < 0)
		return -EIO;

	if ((tsp102_major = (register_chrdev(TSP_MAJOR, TSP_MODULE_NAME, &tsp102_fop))) < 0) {
		printk("Error in Register %s driver\n",TSP_MODULE_NAME);
		return -EIO;
	};
	create_proc_read_entry(TSP_MODULE_NAME, 0, 0, tsp102_proc_output, NULL);
	printk("%s Device Major = %d. see /proc/%s\n", TSP_MODULE_NAME, tsp102_major, TSP_MODULE_NAME);
	return 0;
}

/************************************************************************/
void tse110_cleanup_module(void)
{
	struct TSP_Device *tp;

	for (tp = TSP; tp->reg_base; tp++) {
		iounmap(tp->reg_base);
		iounmap(tp->sram_base);
		free_pages(tp->dma_base, get_order(TSP_DMA_SIZE));
	}
	if ((unregister_chrdev(tsp102_major, TSP_MODULE_NAME)) < 0)
		printk("Error in Un-Register %s driver\n",TSP_MODULE_NAME);
	remove_proc_entry(TSP_MODULE_NAME, NULL);
	printk("TSP Driver Down.\n");
}

module_init(tse110_init_module);
module_exit(tse110_cleanup_module);

/************************************************************************/
MODULE_AUTHOR("Minsuk Lee, mslee@hansung.ac.kr");
MODULE_LICENSE("GPL");
/* MODULE_LICENSE("Copyright (c) 2003 Teleview. All rights reserved."); */
