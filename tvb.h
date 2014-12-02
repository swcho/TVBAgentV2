#ifndef __TSP100_H__
#define __TSP100_H__

/************************************************************************
 * File : TSP100.H
 * Description : Common Header file for linux device driver for TSP boars
 * Author : 
 * Last Modified : 2nd Jun. 2006
************************************************************************/

/************************************************************************
In the make file, one of the follwings should be defined in CFLAGS like as 
                  CFLAGS = -DTSP102
#define TSP102
#define TVB140
#define TQK150
#define TOM160
#define TVB360
#define TVB380	//TVB380
#define TVB380V4	//TVB390V4, V6, V7. V8 board
#define TVB370V6	//TVB370V6(8VSB only), TVB370V7(IF only)
************************************************************************/

/************************************************************************/
/* Device Driver Registration 						*/
/* PCI Identification 							*/
/************************************************************************/
#define TSP_MAJOR		0			// if 0, kernel will assign major number
#define VENDOR			0x9445

#ifdef TSP102
#define TSP_MODULE_NAME	"tsp102"
#define	DEV_ID_042X		0x0421
#endif

#ifdef TVB140
#define TSP_MODULE_NAME "tvb140"
#define DEV_ID_042X     0x0422
#endif

#ifdef TQK150
#define TSP_MODULE_NAME	"tqk150"
#define	DEV_ID_042X		0x0423
#endif

#ifdef TOM160
#define TSP_MODULE_NAME	"tom160"
#define	DEV_ID_042X		0x0424
#endif

#ifdef TVB380
#define TSP_MODULE_NAME "tvb380v4"
#define DEV_ID_042X		0x0381
#endif

#ifdef TVB380V4
#define TSP_MODULE_NAME "tvb380v4"
#define DEV_ID_042X		0x0390
#endif

#ifdef TVB370V6
#define TSP_MODULE_NAME "tvb370v6"
#define DEV_ID_042X		0x0370
#endif

#ifdef TSE110V1
#define TSP_MODULE_NAME "tse110v1"
#define DEV_ID_042X		0x0431
#endif

#define TVB360_MODULE_NAME	"tvb360"
#define	TVB360_DEV_ID_042X	0x0425

#define MAX_TSP_DEVICE		8			// We allow maximum 8 boards per PC

/************************************************************************/
/* RAM & DMA Size 							*/
/************************************************************************/
#define TSP_REG_SIZE		256
#define TSP_SRAM_SIZE		0x2000000		// 32 M
#define TSP_DMA_SIZE		0x100000		//(1024*1024)		//  1 MBytes	//2011/4/19

/************************************************************************/
/* IO Control Command 							*/
/************************************************************************/
#define IOCTL_TEST_TSP				0x800
#define IOCTL_WRITE_TO_MEMORY		0x801
#define IOCTL_READ_FROM_MEMORY		0x802
#define IOCTL_READ_PCI9054_REG		0x803
#define IOCTL_WRITE_PCI9054_REG		0x804
#define IOCTL_TSP100_EPLD_DOWN		0x805
#define IOCTL_TVB350_EPLD_DOWN		0x806
#define IOCTL_FILE_DATA_DOWN		0x807
#define IOCTL_FILE_DATA_UP			0x808
#define IOCTL_GET_DMA_MEMORY		0x809

// TVB595V1
#define IOCTL_REGISTER_EVENT		0x901
#define IOCTL_SIGNAL_EVENT			0x902
#define IOCTL_SET_FIFO_CONTROL		0x903

#define IOCTL_END_OF_APPLICATION		0x811
#define IOCTL_GET_FAST_DOWN			0x812
#define IOCTL_READ_PEX8111_PCI_EXPRESS_REG		0x813
#define IOCTL_READ_PEX8111_PCI_CONFIGURATION_REG		0x814
#define IOCTL_ENABLED_RW_EEPROM			0x815
#define IOCTL_READ_EEPROM_AT25640		0x816
#define IOCTL_WRITE_EEPROM_AT25640		0x817
#define IOCTL_OSRUSBFX2_NET2282_READ		0x910

/************************************************************************/
/* Type definition 							*/
/************************************************************************/
#ifndef bool
#define bool int
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef long long __int64;

typedef struct _kcmd_args {
	ULONG dwCmdParm1;
	ULONG dwCmdParm2;
	ULONG dwCmdParm3;
	DWORD *pdwBuffer;
} KCMD_ARGS;

typedef struct _kaltera_rbf {
	ULONG Size;
	unsigned char *pBuffer;		
} KALTERA_RBF;

typedef struct _dma_alloc_args 
{
	DWORD *LinearAddr;
} DMA_ALLOC_ARGS;

//TVB590S
#if !defined(PCI_ANY_ID)
#define PCI_ANY_ID	(~0)
#endif
#define DEV_ID_TVB590S		0x0598
#define TVB590S_SRAM_SIZE	0x200000

//TVB593
#define DEV_ID_TVB593		0x0593
#define TVB593_SRAM_SIZE	0x200000
#define TVB593_REG_SIZE		512

//kslee:modify ===============================================================================================
int PlxEepromRead(struct pci_dev *_dev_, DWORD offset, DWORD *nValue);
int PlxEepromWrite(struct pci_dev *_dev_, DWORD offset, DWORD val);
int Plx8311_EepromDataWrite(struct pci_dev *_dev_, unsigned char data);
int Plx8311_EepromDataRead(struct pci_dev *_dev_, unsigned char *data);
int Plx8311_Eeprom_CheckBusy(struct pci_dev *_dev_);
int Plx8311_EepromWait(struct pci_dev *_dev_);
DWORD Plx8311_Register_read(struct pci_dev *_dev_, DWORD offset, DWORD *pstatus);
DWORD Plx8311_Register_write(struct pci_dev *_dev_, DWORD offset, DWORD data);
//=========================================================================================================
#endif
