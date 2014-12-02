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
#include <linux/version.h>
#include "tvb.h"		// 

/***********************************************************************/
/* Global Variables                                                    */
/***********************************************************************/
struct TSP_Device {
	int inuse;
	DWORD *reg_base;
	DWORD *sram_base;
	ULONG dma_base;

	struct pci_dev *_dev_pci_;
	struct pci_dev *_dev_pcie_;
	//TVB590S
	int dev_id;
	int ven_id;
	int bus;
	int slot;
	int func;

};

static struct TSP_Device TSP[MAX_TSP_DEVICE];
static int 	     	 tsp102_major;
static int 	       	 num_tsp102;
// huy: modify  
static spinlock_t 	 tsp102_lock;

/***********************************************************************/
/* Defines                                                             */
/***********************************************************************/
#define inode_2_tp(inode) (TSP + ((MINOR((inode)->i_rdev) & 0xf) - 1))
#define ARG_PARM1	  (((KCMD_ARGS *)arg)->dwCmdParm1)
#define ARG_PARM2	  (((KCMD_ARGS *)arg)->dwCmdParm2)
#define ARG_PARM3	  (((KCMD_ARGS *)arg)->dwCmdParm3)
#define ARG_BUFFER	  (((KCMD_ARGS *)arg)->pdwBuffer)

//kslee:modify ===================================================================
#define KERNEL_DEBUG_
#define PEX8111_DEVICE_ID 0x8111
#define PEX8111_VENDOR_ID 0x10B5
#define EEPROM_STATUS_FALSE 0
#define EEPROM_STATUS_TRUE 1
#define PLX8311_EE_OPCODE_READ             3
#define PLX8311_EE_OPCODE_READ_STATUS      5
#define PLX8311_EE_OPCODE_WRITE_ENABLE     6
#define PLX8311_EE_OPCODE_WRITE            2
#define PLX8311_EE_OPCODE_WRITE_STATUS     1
#define MAIN_CNTL_REG_INDEX			0x84
#define MAIN_CNTL_REG_DATA				0x88
//==============================================================================

/************************************************************************/
// Download sdcxxx.rbf file (firmware)
static void Configure_Altera(DWORD *pl_BaseAddr, KALTERA_RBF *pKAlteraRbf)
{
	DWORD 		maskdata;
	unsigned char 	buf;
	int		i, j;
	//TVB590S
	//pKAlteraRbf->Size = pKAlteraRbf->Size & 0xfffff;    // May not exceed 1MBytes
	int use_2m_space = (pKAlteraRbf->Size >> 31);
	pKAlteraRbf->Size = pKAlteraRbf->Size & 0xffffff;

	for (i = 0; i < (int)(pKAlteraRbf->Size); i++) {
		buf = *(pKAlteraRbf->pBuffer + i);
		for (j = 0; j < 8; j++) {	// send a bit clock low to high
			maskdata = (DWORD)((buf & 0x01) << 1);
			//TVB590S
			if ( use_2m_space == 1 )
			{
				maskdata = (DWORD)(buf & 0x01);
				writel(0x00000000 | maskdata, pl_BaseAddr+0x40000);
				writel(0x00000002 | maskdata, pl_BaseAddr+0x40000);
			}
			else
			{
				writel(0x000000c0 | maskdata, pl_BaseAddr+0x400000); //  dclk = 0 with data  
				writel(0x000000c4 | maskdata, pl_BaseAddr+0x400000); //  dclk = 1 with data  
			}
			buf >>= 1; //    LSB first
		} // j-for loop  		
	} // i-for loop
}		


int tvb_unlocked_ioctl(struct file *filp, unsigned int cmd, ULONG arg)
{

	struct TSP_Device *tp = filp->private_data; 
	//printk("[Debug] private data %X \n", filp->private_data);
	//struct TSP_Device *tp = &TSP[0]; 
	ULONG dwDummy;
	DWORD nReg;
	unsigned char* buf=NULL;

	KALTERA_RBF kRBF;
	KCMD_ARGS kCmd;
	copy_from_user(&kCmd, (void*)arg, sizeof(KCMD_ARGS));
	//printk("[Huy-from kernel space] command : %X \n", cmd);
	
	switch (cmd) {
		case IOCTL_TEST_TSP :
			//TVB590S - 0x00=DEV_ID, 0x01=VEN_ID, 0x02=BUS, 0x03=SLOT, 0x04=FUNCTION
			dwDummy = kCmd.dwCmdParm1;
			if ( dwDummy == 0 )
			{
				kCmd.dwCmdParm2 = tp->dev_id;
			}
			else if ( dwDummy == 1 )
			{
				kCmd.dwCmdParm2 = tp->ven_id;
			}
			else if ( dwDummy == 2 )
			{
				kCmd.dwCmdParm2 = tp->bus;
			}
			else if ( dwDummy == 3 )
			{
				kCmd.dwCmdParm2 = tp->slot;
			}
			else if ( dwDummy == 4 )
			{
				kCmd.dwCmdParm2 = tp->func;
			}
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;

		case IOCTL_WRITE_TO_MEMORY :
#ifdef KERNEL_DEBUG_
		  	printk("[TELEVIEW] [Debug]  write-to-memory  base: %X cmd  : %X , value: %X  \n", tp->sram_base,  kCmd.dwCmdParm1 & 0x7fffff, kCmd.dwCmdParm2);
#endif
			writel(kCmd.dwCmdParm2, tp->sram_base + (kCmd.dwCmdParm1 & 0x7fffff));
			break;
			
		case IOCTL_READ_FROM_MEMORY :
	
#ifdef KERNEL_DEBUG_
		  printk("[TELEVIEW] [Debug]  read-from-memory  : %X \n", kCmd.dwCmdParm1 & 0x7fffff);
		  printk("[TELEVIEW] [Debug]  read-from-memory  : user-add: %X \n",arg);
		  printk("[TELEVIEW] [Debug]  read-from-memory  : size : %d \n",sizeof(KCMD_ARGS));
#endif
			kCmd.dwCmdParm2 = readl(tp->sram_base + (kCmd.dwCmdParm1 & 0x7fffff));
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
			
		case IOCTL_READ_PCI9054_REG :
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  PCI access  read command : %X \n", kCmd.dwCmdParm1 );
#endif
			kCmd.dwCmdParm2 = readl(tp->reg_base + (kCmd.dwCmdParm1 & 0x3f));
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;

		case IOCTL_WRITE_PCI9054_REG :
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  PCI access write  command : %X \n", kCmd.dwCmdParm1 );
#endif
			writel(kCmd.dwCmdParm2, tp->reg_base + (kCmd.dwCmdParm1 & 0x3f));
			break;

		case IOCTL_TSP100_EPLD_DOWN :
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW]RBF SIZE %ull\n", kRBF.Size);
#endif
			copy_from_user(&kRBF, (void*)arg, sizeof(KALTERA_RBF));
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 1\n");
#endif
			//TVB590S
			/*
			buf = vmalloc(kRBF.Size);
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 2\n");
#endif
			copy_from_user(buf, (void*)kRBF.pBuffer, kRBF.Size);
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 3\n");
#endif
			*/
			buf = vmalloc(kRBF.Size & 0xffffff);
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 4\n");
#endif
			copy_from_user(buf, (void*)kRBF.pBuffer, kRBF.Size & 0xffffff);
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 5\n");
#endif
			kRBF.pBuffer = buf;
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 6\n");
#endif
			
			Configure_Altera(tp->sram_base, &kRBF);

#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 7\n");
#endif
			vfree(kRBF.pBuffer);
#ifdef KERNEL_DEBUG_
printk("[TELEVIEW] pos 8\n");
#endif
			break;

		case IOCTL_TVB350_EPLD_DOWN :
			break;

		case IOCTL_FILE_DATA_DOWN :
               // printk("DMA ADDRESS : 0x%X\n", tp->dma_base);
               // tp->dma_base = tp->dma_base +  0x100000;
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
			//printk("[TELEVIEW] in GET_DMA_MEMORY dummy 0 = %lx\n", dwDummy);
			dwDummy = readl(tp->reg_base + 1);
			//printk("[TELEVIEW] in GET_DMA_MEMORY dummy 1 = %lx\n", dwDummy);
			//printk("[TELEVIEW] IOCTL, GET_DMA_MEMORY %x %lX\n", cmd, ((DMA_ALLOC_ARGS *)arg)->LinearAddr);
			break;
//kslee:modify ==================================================================================================
		case IOCTL_GET_FAST_DOWN:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_GET_FAST_DOWN \n");
#endif
			kCmd.dwCmdParm2 = 2;
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
		case IOCTL_ENABLED_RW_EEPROM:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_ENABLED_RW_EEPROM \n");
#endif
			kCmd.dwCmdParm2 = 1;
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
		case IOCTL_READ_PEX8111_PCI_EXPRESS_REG:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_READ_PEX8111_PCI_EXPRESS_REG \n");
#endif
			pci_read_config_dword(tp->_dev_pcie_, (kCmd.dwCmdParm1 & 0xFFFFFFFF), &nReg);
			kCmd.dwCmdParm2 = nReg;
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
		case IOCTL_READ_PEX8111_PCI_CONFIGURATION_REG:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_READ_PEX8111_PCI_CONFIGURATION_REG \n");
#endif
			pci_read_config_dword(tp->_dev_pci_, (kCmd.dwCmdParm1 & 0xFFFFFFFF), &nReg);
			kCmd.dwCmdParm2 = nReg;
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
		case IOCTL_READ_EEPROM_AT25640:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_READ_EEPROM_AT25640 \n");
#endif
			PlxEepromRead(tp->_dev_pcie_, (DWORD)(kCmd.dwCmdParm1 & 0xFFFFFFFF), &nReg);
			kCmd.dwCmdParm2 = nReg;
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
			break;
		case IOCTL_WRITE_EEPROM_AT25640:
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug]  IOCTL_WRITE_EEPROM_AT25640 \n");
#endif
			PlxEepromWrite(tp->_dev_pcie_, (DWORD)(kCmd.dwCmdParm1 & 0xFFFFFFFF), (DWORD)(kCmd.dwCmdParm2 & 0xFFFFFFFF));
			break;
//============================================================================================================
		default :
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] Wrong IOCTL, %x\n", cmd);
#endif
			return -EINVAL;
	}
	
	return 0;
}

//kslee:modify ==================================================================================================
int PlxEepromRead(struct pci_dev *_dev_, DWORD offset, DWORD *nValue)
{
	unsigned char i;
	unsigned char EepData;
	DWORD RegValue;
	int	nRet;
	*nValue = 0;

	if(Plx8311_EepromWait(_dev_) == EEPROM_STATUS_FALSE)
	{
		return EEPROM_STATUS_FALSE;
	}
	Plx8311_EepromDataWrite(_dev_, PLX8311_EE_OPCODE_READ);

	RegValue = Plx8311_Register_read(_dev_, 0x1004, &nRet);
	RegValue = (RegValue >> 23) & 0x3;

	if(RegValue == 0)
		RegValue = 2;

	if(RegValue == 3)
	{
		Plx8311_EepromDataWrite(_dev_, 0);
	}

	if(RegValue == 2 || RegValue == 3)
	{
		Plx8311_EepromDataWrite(_dev_, (unsigned char)(offset >> 8));
	}

	Plx8311_EepromDataWrite(_dev_, (unsigned char)offset);

	for(i=0;i<2;i++)
	{
		Plx8311_EepromDataRead(_dev_, &EepData);
		if(i == 0)
			*nValue |= EepData;
		else
			*nValue |= (DWORD)((EepData << 8) & 0xFF00);
	}
	
	Plx8311_Register_write(_dev_, 0x1004, 0);

	return EEPROM_STATUS_TRUE;
}

int PlxEepromWrite(struct pci_dev *_dev_, DWORD offset, DWORD val)
{
	unsigned char i;
	DWORD RegValue;
	DWORD nRet;
	unsigned short value;

	value = (unsigned short)(val & 0xFFFF);

	if(Plx8311_EepromWait(_dev_) == EEPROM_STATUS_FALSE)
	{
		return EEPROM_STATUS_FALSE;
	}

	Plx8311_EepromDataWrite(_dev_, PLX8311_EE_OPCODE_WRITE_ENABLE);

	Plx8311_Register_write(_dev_, 0x1004, 0);

	Plx8311_EepromDataWrite(_dev_, PLX8311_EE_OPCODE_WRITE);

	RegValue = Plx8311_Register_read(_dev_, 0x1004, &nRet);
	RegValue = (RegValue >> 23) & 0x3;

	if(RegValue == 0)
		RegValue = 2;

	if(RegValue == 3)
	{
		Plx8311_EepromDataWrite(_dev_, 0);
	}

	if(RegValue == 2 || RegValue == 3)
	{
		Plx8311_EepromDataWrite(_dev_, (unsigned char)(offset >> 8));
	}

	Plx8311_EepromDataWrite(_dev_, (unsigned char)offset);

	for(i=0; i < 2; i++)
	{
		Plx8311_EepromDataWrite(_dev_, (unsigned char)value);
		value >>= 8;
	}

	Plx8311_Register_write(_dev_, 0x1004, 0);


	return EEPROM_STATUS_TRUE;
}

int Plx8311_EepromWait(struct pci_dev *_dev_)
{
	unsigned char status;
	unsigned short Timeout;
	Timeout = 200;
	do
	{
		Plx8311_EepromDataWrite(_dev_, PLX8311_EE_OPCODE_READ_STATUS);
		Plx8311_EepromDataRead(_dev_, &status);
		Plx8311_Register_write(_dev_, 0x1004, 0);

		if((status & (1 << 0)) == 0)
			return EEPROM_STATUS_TRUE;

		Timeout--;
	}
	while(Timeout);
	return EEPROM_STATUS_FALSE;
}
int Plx8311_EepromDataWrite(struct pci_dev *_dev_, unsigned char data)
{
	DWORD RegValue;
	if(Plx8311_Eeprom_CheckBusy(_dev_) == EEPROM_STATUS_FALSE)
	{
		return EEPROM_STATUS_FALSE;
	}

	RegValue = (1 << 18) | (1 << 16) | data;

	Plx8311_Register_write(_dev_, 0x1004, RegValue);

	return Plx8311_Eeprom_CheckBusy(_dev_);
}

int Plx8311_EepromDataRead(struct pci_dev *_dev_, unsigned char *data)
{
	DWORD RegValue;
	int rc;
	DWORD nRet;
	if(Plx8311_Eeprom_CheckBusy(_dev_) == EEPROM_STATUS_FALSE)
	{
		return EEPROM_STATUS_FALSE;
	}

	RegValue = (1 << 18) | (1 << 17);

	Plx8311_Register_write(_dev_, 0x1004, RegValue);

	rc = Plx8311_Eeprom_CheckBusy(_dev_);

	if(rc == EEPROM_STATUS_TRUE)
	{
		RegValue = Plx8311_Register_read(_dev_, 0x1004, &nRet);
		*data = (unsigned char)((RegValue >> 8) & 0xFF);
	}

	return rc;
}

int Plx8311_Eeprom_CheckBusy(struct pci_dev *_dev_)
{
	DWORD Timeout;
	int nRet;
	DWORD value;
	Timeout = 100000;

	do
	{
		value = Plx8311_Register_read(_dev_, 0x1004, &nRet);
		Timeout--;
	} while(Timeout && (value & (1 << 19)));

	if(Timeout == 0)
		return EEPROM_STATUS_FALSE;

	return EEPROM_STATUS_TRUE;
}

//PLX_8111_REG_WRITE
DWORD Plx8311_Register_write(struct pci_dev *_dev_, DWORD offset, DWORD data)
{
	DWORD RegValue;

	if(offset < 0x1000 || offset > 0x1064)
	{
		return EEPROM_STATUS_FALSE;
	}

	offset -= 0x1000;

	pci_read_config_dword(_dev_, MAIN_CNTL_REG_INDEX, &RegValue);

	pci_write_config_dword(_dev_, MAIN_CNTL_REG_INDEX, offset);

	pci_write_config_dword(_dev_, MAIN_CNTL_REG_DATA, data);

	pci_write_config_dword(_dev_, MAIN_CNTL_REG_INDEX, RegValue);

	return EEPROM_STATUS_TRUE;
}

// PLX_8111_REG_READ
DWORD Plx8311_Register_read(struct pci_dev *_dev_, DWORD offset, DWORD *pstatus)
{
	DWORD value;
	DWORD   RegSave;
	if(offset < 0x1000 || offset > 0x1064)
	{
		*pstatus = 0;
		return EEPROM_STATUS_FALSE;
	}

	offset -= 0x1000;

	pci_read_config_dword(_dev_, MAIN_CNTL_REG_INDEX, &RegSave);
	
	pci_write_config_dword(_dev_, MAIN_CNTL_REG_INDEX, offset);

	pci_read_config_dword(_dev_, MAIN_CNTL_REG_DATA, &value);

	pci_write_config_dword(_dev_, MAIN_CNTL_REG_INDEX, RegSave);

	return value;
}

//============================================================================================================

/************************************************************************/
int tsp102_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, ULONG arg)
{
	struct TSP_Device *tp = inode_2_tp(inode);
	ULONG dwDummy;
	unsigned char* buf=NULL;

	KALTERA_RBF kRBF;
	KCMD_ARGS kCmd;
	copy_from_user(&kCmd, (void*)arg, sizeof(KCMD_ARGS));
	//printk("[Huy-from kernel space] command : %X \n", cmd);
	
	switch (cmd) {
		case IOCTL_TEST_TSP :
			//TVB590S - 0x00=DEV_ID, 0x01=VEN_ID, 0x02=BUS, 0x03=SLOT, 0x04=FUNCTION
			dwDummy = kCmd.dwCmdParm1;
			if ( dwDummy == 0 )
			{
				kCmd.dwCmdParm2 = tp->dev_id;
			}
			else if ( dwDummy == 1 )
			{
				kCmd.dwCmdParm2 = tp->ven_id;
			}
			else if ( dwDummy == 2 )
			{
				kCmd.dwCmdParm2 = tp->bus;
			}
			else if ( dwDummy == 3 )
			{
				kCmd.dwCmdParm2 = tp->slot;
			}
			else if ( dwDummy == 4 )
			{
				kCmd.dwCmdParm2 = tp->func;
			}
			copy_to_user((void*)arg, &kCmd, sizeof(KCMD_ARGS));
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
			//TVB590S
			/*
			buf = vmalloc(kRBF.Size);
			copy_from_user(buf, (void*)kRBF.pBuffer, kRBF.Size);
			*/
			buf = vmalloc(kRBF.Size & 0x1fffff);
			copy_from_user(buf, (void*)kRBF.pBuffer, kRBF.Size & 0x1fffff);
			kRBF.pBuffer = buf;
			
			Configure_Altera(tp->sram_base, &kRBF);

			vfree(kRBF.pBuffer);
			break;

		case IOCTL_TVB350_EPLD_DOWN :
			break;

		case IOCTL_FILE_DATA_DOWN :
               // printk("DMA ADDRESS : 0x%X\n", tp->dma_base);
               // tp->dma_base = tp->dma_base +  0x100000;
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
			//printk("[TELEVIEW] in GET_DMA_MEMORY dummy 0 = %lx\n", dwDummy);
			dwDummy = readl(tp->reg_base + 1);
			//printk("[TELEVIEW] in GET_DMA_MEMORY dummy 1 = %lx\n", dwDummy);
			//printk("[TELEVIEW] IOCTL, GET_DMA_MEMORY %x %lX\n", cmd, ((DMA_ALLOC_ARGS *)arg)->LinearAddr);
			break;

		default :
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] Wrong IOCTL, %x\n", cmd);
#endif
			return -EINVAL;
	}
	
	return 0;
}

/************************************************************************/
int tsp102_open(struct inode *inode, struct file *mfile)
{
	struct TSP_Device *tp;
	unsigned flags;

#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] opening the device driver MINOR: %d \n", MINOR(inode->i_rdev));
#endif

	if (((MINOR((inode)->i_rdev) & 0xf) - 1) >= num_tsp102)
		return -ENODEV;

//	printk("[TELEVIEW] Debug \n");
	// huy: add 		
	spin_lock_init(&tsp102_lock);
	
	spin_lock_irqsave(&tsp102_lock, flags);
	tp = inode_2_tp(inode);
	mfile->private_data = tp;
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] [Debug] private data %X \n", mfile->private_data);
#endif
	if (tp->inuse) {
		spin_unlock_irqrestore(&tsp102_lock, flags);
		return -ENOSPC;
	} else if (!tp->reg_base) {
		spin_unlock_irqrestore(&tsp102_lock, flags);
		return -ENODEV;
	} else
		tp->inuse = 1;
	spin_unlock_irqrestore(&tsp102_lock, flags);
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] Board %d Opened\n", MINOR((inode)->i_rdev) & 0xf);
#endif
	return 0;
}

/************************************************************************/
int tsp102_release(struct inode *inode, struct file *mfile)
{
	struct TSP_Device *tp = inode_2_tp(inode);
	tp->inuse = 0;
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] Board %d Closed\n", MINOR((inode)->i_rdev) & 0xf);
#endif

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
	struct pci_dev *_dev8111_ = NULL;	
	int i;
	//kslee:modify
	unsigned int nPex8111_secondary_bus;
	//TVB590S
	int _REG_SIZE, _SRAM_SIZE;

    	if (0/*!pcibios_present()*/) {
#ifdef KERNEL_DEBUG_
    		printk("[TELEVIEW] %s: no PCI bios\n",TSP_MODULE_NAME);
#endif
		return -1;
	}
	
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] [Debug] %s, %d \n", __FUNCTION__, __LINE__);
#endif
	for (num_tsp102 = 0; num_tsp102 < MAX_TSP_DEVICE; num_tsp102++) {
		tp = TSP + num_tsp102;
		memset(tp, 0, sizeof(*tp));

		//TVB590S
		/*
		_dev_ = pci_get_device(VENDOR, DEV_ID_042X, _dev_);
		if ( _dev_ == NULL )
			break;
		*/
		do
		{
			_dev_ = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, _dev_);
			if ( _dev_ == NULL || _dev_->device == DEV_ID_042X )
				break;
#ifdef TVB380V4
			//TVB593
			if ( _dev_->device == DEV_ID_TVB590S || _dev_->device == DEV_ID_TVB593 )
			{
				tp->_dev_pci_ = _dev_;
				break;
			}
#endif
		} while(1);
		if ( _dev_ == NULL )
			break;
		
		
		// huy: add
		pci_enable_device(_dev_);
		pci_set_master(_dev_);
	//TEST - SLOT NUMBER
		/*
#ifdef TVB380V4
		i = PCI_SLOT(_dev_->devfn);
		if ( i <= 0 || i >= MAX_TSP_DEVICE )
		{
		}
		else
		{
			tp = TSP + i-1;
			memset(tp, 0, sizeof(*tp));
		}
#endif
		*/
//kslee:modify	========================================================================================
		tp->_dev_pcie_ = NULL;
		//find pex8111
		do
		{
			_dev8111_ = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, _dev8111_);
			if(_dev8111_ == NULL)
				break;
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] [KSLEE] Vendor ID [0x%X], Device ID [0x%x] \n", _dev8111_->vendor, _dev8111_->device );
#endif
			if(_dev8111_->device == PEX8111_DEVICE_ID && _dev8111_->vendor == PEX8111_VENDOR_ID)
			{
				pci_read_config_dword(_dev8111_, 0x18, &nPex8111_secondary_bus);
				nPex8111_secondary_bus = ((nPex8111_secondary_bus >> 8) & 0xFF);
				if(nPex8111_secondary_bus == _dev_->bus->number)
				{
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] [Debug] Find Pex8111, bus[0x%x] \n", _dev_->bus->number );
#endif
					tp->_dev_pcie_ = _dev8111_;
					break;
				}
			}
		} while (1);
//====================================================================================================

		//TVB590S
		tp->dev_id = _dev_->device;
		tp->ven_id = _dev_->vendor;
		tp->bus = _dev_->bus->number;
		tp->slot = PCI_SLOT(_dev_->devfn);
		tp->func = PCI_FUNC(_dev_->devfn);
		_REG_SIZE = TSP_REG_SIZE;
		_SRAM_SIZE = TSP_SRAM_SIZE;
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] [Debug] %s, %d (%d, %d) \n", __FUNCTION__, __LINE__, tp->slot, tp->func );
#endif

		if ( _dev_->device == DEV_ID_TVB590S )
		{
			_SRAM_SIZE = TVB590S_SRAM_SIZE;
		}
		//TVB593
		else if ( _dev_->device == DEV_ID_TVB593 )
		{
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug] %s, %d \n", __FUNCTION__, __LINE__);
#endif
			_SRAM_SIZE = TVB593_SRAM_SIZE;
			_REG_SIZE = TVB593_REG_SIZE;
		}
		
		for (i = 0; pci_base[i]; i++) {
			u32 addr, mask;
			int type;


			pci_read_config_dword(_dev_, pci_base[i], &addr);
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug] %s, %d address: %X, value: %X  \n", __FUNCTION__, __LINE__, pci_base[i], addr);
#endif
			local_irq_disable();
			//pci_write_config_dword(_dev_, pci_base[i], ~0);
			pci_write_config_dword(_dev_, pci_base[i], 0xFFFFFFFF);
			pci_read_config_dword(_dev_, pci_base[i], &mask);
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] [Debug] %s, %d address: %X, value: %X  \n", __FUNCTION__, __LINE__, pci_base[i], mask);
#endif
			pci_write_config_dword(_dev_, pci_base[i], addr);
			local_irq_enable();				

			if (!mask) {
#ifdef KERNEL_DEBUG_
				printk("[TELEVIEW] region %d does not exists\n", i);
#endif
				break;
			}

			if (mask & PCI_BASE_ADDRESS_SPACE) {
				type = 0; mask &= PCI_BASE_ADDRESS_IO_MASK;
			} else {
				type = 1; mask &= PCI_BASE_ADDRESS_MEM_MASK;
			}
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] PCI Region %d: %s Size 0x%X\n", i, type ? "MEM" : "I/O", ~mask + 1);
#endif

			//TVB590S
			/*
			if (type && (~mask + 1) == TSP_REG_SIZE)
				tp->reg_base = (ULONG *)addr;

			if (type && (~mask + 1) == TSP_SRAM_SIZE)
				tp->sram_base = (ULONG *)addr;
			*/
			if (tp->reg_base == 0 && (type && (~mask + 1) == _REG_SIZE))
			{
				tp->reg_base = (DWORD *)addr;
#ifdef KERNEL_DEBUG_
				printk("[TELEVIEW] [Debug] reg_base bar: %d \n",  i);
#endif
			//printk("[TELEVIEW] [Debug] %s, %d \n", __FUNCTION__, __LINE__);
			}
			
			if (type && (~mask + 1) == _SRAM_SIZE)
			{
#ifdef KERNEL_DEBUG_
				printk("[Debug] sram_base bar: %d \n",  i);
#endif
				tp->sram_base = (DWORD *)addr;
		//	printk("[TELEVIEW] [Debug] %s, %d \n", __FUNCTION__, __LINE__);
			}
		}
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] %d base (P): reg:0x%p, sram:0x%p\n", num_tsp102, tp->reg_base, tp->sram_base);
#endif

		// IOREMAP
		//TVB590S - TSP_REG_SIZE -> _REG_SIZE, TSP_SRAM_SIZE -> _SRAM_SIZE
		tp->reg_base = (DWORD *)ioremap((ULONG)tp->reg_base, _REG_SIZE);
		
		if (!tp->reg_base) {
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] reg_base - ioremap fail\n");
#endif
			return -1;
		}
		//TVB590S - TSP_REG_SIZE -> _REG_SIZE, TSP_SRAM_SIZE -> _SRAM_SIZE
		tp->sram_base = (DWORD *)ioremap((ULONG)tp->sram_base, _SRAM_SIZE);
		if (!tp->sram_base) {
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] sram_base - ioremap fail\n");
#endif
			return -1;
		}
		tp->dma_base = (ULONG)__get_dma_pages(GFP_KERNEL | GFP_DMA, get_order(TSP_DMA_SIZE));
		if (!tp->dma_base) {
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] DMA Memory Allocation Failure\n");
#endif
			return -1;
		}



	}
	//printk("[TELEVIEW] ************** DEV_ID_042X = 0x%x, num_tsp102=%d\n",DEV_ID_042X, num_tsp102);

	if (!num_tsp102) {
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] No TSP board found\n");
#endif
		return -1;
	} else {
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW]  %d TSP Board%s found\n", num_tsp102, (num_tsp102 == 1) ? "" : "s");
#endif
		for (tp = TSP; tp->reg_base; tp++) {
#ifdef KERNEL_DEBUG_
			printk("[TELEVIEW] /dev/%s-%d  reg:0x%p, sram:0x%p dma:0x%x\n",TSP_MODULE_NAME,
				(tp - TSP) + 1, tp->reg_base, tp->sram_base, tp->dma_base);
#endif
		}
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] /dev/%s is redirected to /dev/%s-1\n",TSP_MODULE_NAME,TSP_MODULE_NAME);
#endif
	}
	return 0;
}

/************************************************************************/
struct file_operations tsp102_fop = {
	open:		tsp102_open,
	unlocked_ioctl:		tvb_unlocked_ioctl,
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
			(tp - TSP), tp->reg_base, tp->sram_base, tp->dma_base); }
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
int tvb380_init_module(void)
{

	// for debugging 

	if (init_hardware() < 0)
		return -EIO;

	if ((tsp102_major = (register_chrdev(TSP_MAJOR, TSP_MODULE_NAME, &tsp102_fop))) < 0) {
#ifdef KERNEL_DEBUG_
		printk("[TELEVIEW] Error in Register %s driver\n",TSP_MODULE_NAME);
#endif
		return -EIO;
	};
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	create_proc_read_entry(TSP_MODULE_NAME, 0, 0, tsp102_proc_output, NULL);
#else
    proc_create_data(TSP_MODULE_NAME, 0, 0, tsp102_proc_output, NULL);
#endif
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] %s Device Major = %d. see /proc/%s\n", TSP_MODULE_NAME, tsp102_major, TSP_MODULE_NAME);
#endif
	return 0;
}

/************************************************************************/
void tvb380_cleanup_module(void)
{
	struct TSP_Device *tp;

	for (tp = TSP; tp->reg_base; tp++) {
		iounmap(tp->reg_base);
		iounmap(tp->sram_base);
		free_pages(tp->dma_base, get_order(TSP_DMA_SIZE));
	}
	/*
	if ((unregister_chrdev(tsp102_major, TSP_MODULE_NAME)) < 0)
		printk("Error in Un-Register %s driver\n",TSP_MODULE_NAME);
	*/
	unregister_chrdev(tsp102_major, TSP_MODULE_NAME);
	remove_proc_entry(TSP_MODULE_NAME, NULL);
#ifdef KERNEL_DEBUG_
	printk("[TELEVIEW] TSP Driver Down.\n");
#endif
}

module_init(tvb380_init_module);
module_exit(tvb380_cleanup_module);

/************************************************************************/
MODULE_AUTHOR("Minsuk Lee, mslee@hansung.ac.kr");
MODULE_LICENSE("GPL");
/* MODULE_LICENSE("Copyright (c) 2003 Teleview. All rights reserved."); */
