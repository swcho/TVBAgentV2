////////////////////////////////////////////////////////////////
//
// This is Netchip2282 APIs for accessing the USB device.
// The code accesses the hardware via WinDriver functions.
// 
// Copyright (c) 2006 - 2007 TELEVIEW.  http://www.teleview.com
// 
////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>

#include "NcRemotePci.h"

int ___get_TSPL_nBoardTypeID(void);

typedef struct _kcmd_args {
	ULONG dwCmdParm1;
	ULONG dwCmdParm2;
	ULONG dwCmdParm3;
	ULONG *pdwBuffer;
} KCMD_ARGS;

#define EP_DIR(ep) ((ep>>ENDPOINT_DIRECTION) & 0x01)
#define TRANSFER_TIMEOUT 15000//120000//30000 // in msecs
#define FALSE 0
#define TRUE 1
///////////////////////////////////////////////////////////////////////////////
// ... functions
///////////////////////////////////////////////////////////////////////////////

#define BYTES_IN_LINE 16
#define HEX_CHARS_PER_BYTE 3
#define HEX_STOP_POS BYTES_IN_LINE * HEX_CHARS_PER_BYTE
static ULONG ConverHexBufferToInt(PVOID pBuf, DWORD dwBytes)
{
	ULONG nRet = 0;
	PBYTE pbData = (PBYTE)pBuf;
	DWORD dwOffset;

	for (dwOffset = 0; dwOffset < dwBytes; dwOffset++)
	{
		nRet += ((UINT32)pbData[dwOffset] << dwOffset*8);
	}
	return nRet;
}

static DWORD WDU_Transfer(WDU_DEVICE_HANDLE hDevice, DWORD dwPipeNum, DWORD fRead, 
    DWORD dwOptions, PVOID pBuffer, DWORD dwBufferSize, PDWORD pdwBytesTransferred, 
    PBYTE pSetupPacket, DWORD dwTimeout)
{
#if 0
	struct USB_TRANSFER_PARAM param;
	struct usbdevfs_ioctl	wrapper;

	param.dwPipeNum = dwPipeNum;
	param.fRead = fRead; 
    	param.dwOptions = dwOptions; 
    	param.pBuffer = (unsigned long*)pBuffer; 
    	param.dwBufferSize = dwBufferSize; 
    	param.pdwBytesTransferred = pdwBytesTransferred; 
       param.pSetupPacket = pSetupPacket; 
       param.dwTimeout = dwTimeout;

	wrapper.ifno = 0;//# of interface
	wrapper.ioctl_code = USB_TRANSFER_REQUEST;
	wrapper.data = (void *)&param;
#endif

	if ( fRead )
		*pdwBytesTransferred = usb_bulk_read(hDevice, dwPipeNum, (char *)pBuffer, dwBufferSize, dwTimeout);
	else
		*pdwBytesTransferred = usb_bulk_write(hDevice, dwPipeNum, (char *)pBuffer, dwBufferSize, dwTimeout);
	return (dwBufferSize == *pdwBytesTransferred) ? 0 : 1;

}

static DWORD WDU_HaltTransfer(WDU_DEVICE_HANDLE hDevice, DWORD dwPipeNum)
{
    DWORD dwStatus = 0;
    return dwStatus;
}

static ULONG TransferData(WDU_DEVICE_HANDLE hDevice, DWORD dwPipeNum, DWORD fRead, 
    DWORD dwOptions, PVOID pBuffer, DWORD dwBufferSize, PDWORD pdwBytesTransferred, 
    PBYTE pSetupPacket, DWORD dwTimeout)
{
	DWORD dwError;

	dwError = WDU_Transfer(hDevice, dwPipeNum, fRead==1, dwOptions, pBuffer, 
		dwBufferSize, pdwBytesTransferred, pSetupPacket, TRANSFER_TIMEOUT);

	if (dwError)
	{
		//ERR("ReadWritePipesMenu: WDU_Transfer() failed: error 0x%lx (\"%s\")\n",	dwError, Stat2Str(dwError));
		//printf("ReadWritePipesMenu: WDU_Transfer() failed: error 0x%lx \n",	dwError);
	}
	else
	{
		//printf("ReadWritePipesMenu: WDU_Transfer() : Transferred 0x%x \n",	(int)(*pdwBytesTransferred));
		if (dwBufferSize <= 10 && fRead==1 && pBuffer)
		{
			//DIAG_PrintHexBuffer(pBuffer, *pdwBytesTransferred, TRUE);

			return ConverHexBufferToInt(pBuffer, *pdwBytesTransferred);
		}
		else
		{
			//if ( *pdwBytesTransferred <= 10)
			//	DIAG_PrintHexBuffer(pBuffer, *pdwBytesTransferred, TRUE);
		}
	}

	return dwError;
	
}

///////////////////////////////////////////////////////////////////////////////
// Remote PCI HAL functions
///////////////////////////////////////////////////////////////////////////////

NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegRead_UlongBar0    (PNC_RPCI_EXT prx,  USHORT Address, PULONG Data)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
	dwPipeNum = CFGOUT_EP_ADDRESS;
	fread = EP_DIR(dwPipeNum);
	dwSize = 6;
    	
    	pBuffer[0] = 0x1F;//011111 Memory-Mapped Configurtion Reg. Byte Enables
    	pBuffer[1] = 0x00;

    	pBuffer[3] = (Address >> 8) & 0xFF;
    	pBuffer[2] = (Address >> 0) & 0xFF;
    	
    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0,
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);

	memset(pBuffer, 0, 10);
    	dwPipeNum = CFGIN_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 4;
	*Data = TransferData(prx, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize, 
		&dwBytesTransferred, 
		NULL, 
		TRANSFER_TIMEOUT);

	return nRet;
}

NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegWrite_UlongBar0   (PNC_RPCI_EXT prx,  USHORT Address, ULONG Data)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = CFGOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 10;
    	
    	pBuffer[0] = 0x1F;//011111 Memory-Mapped Configurtion Reg. Byte Enables
    	pBuffer[1] = 0x00;

    	pBuffer[3] = (Address >> 8) & 0xFF;
    	pBuffer[2] = (Address >> 0) & 0xFF;

    	pBuffer[9] = (Data >> 24) & 0xFF;
    	pBuffer[8] = (Data >> 16) & 0xFF;
    	pBuffer[7] = (Data >> 8) & 0xFF;
    	pBuffer[6] = (Data >> 0) & 0xFF;

    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0,
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);

    	return nRet;
}

NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegRead_UlongConfig  (PNC_RPCI_EXT prx,  USHORT Address, PULONG Data)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = CFGOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 6;
    	
    	pBuffer[0] = 0x0F;//001111 PCI Configuration Reg. Byte Enables
    	pBuffer[1] = 0x00;

    	pBuffer[3] = (Address >> 8) & 0xFF;
    	pBuffer[2] = (Address >> 0) & 0xFF;
    	
    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0,
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);

	memset(pBuffer, 0, 10);
    	dwPipeNum = CFGIN_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 4;
	*Data = TransferData(prx, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize, 
		&dwBytesTransferred, 
		NULL, 
		TRANSFER_TIMEOUT);

	return nRet;
}

NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegWrite_UlongConfig (PNC_RPCI_EXT prx,  USHORT Address, ULONG Data)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = CFGOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 10;

    	pBuffer[0] = 0x0F;//001111 PCI Configuration Reg. Byte Enables
    	pBuffer[1] = 0x00;
    	
    	pBuffer[3] = (Address >> 8) & 0xFF;
    	pBuffer[2] = (Address >> 0) & 0xFF;

    	pBuffer[9] = (Data >> 24) & 0xFF;
    	pBuffer[8] = (Data >> 16) & 0xFF;
    	pBuffer[7] = (Data >> 8) & 0xFF;
    	pBuffer[6] = (Data >> 0) & 0xFF;

    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0,
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);

    	return nRet;
}

NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteConfigUlong   (PNC_RPCI_EXT prx,  ULONG Config,   ULONG Value)	
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 10;

    	pBuffer[1] = 0x40;//01000000 PARK_EXTERNAL_REQUESTOR<<PCI_ARBITER_PARK_SELECT
    	pBuffer[0] = 0xAF;//10101111 (CFG_READ_OR_WRITE << PCI_MASTER_COMMAND_SELECT) | (1<<PCI_MASTER_START) | (PCI_MASTER_WRITE<<PCI_MASTER_READ_WRITE) | (0x0F<<PCI_MASTER_BYTE_ENABLES)
    	
    	pBuffer[5] = (Config >> 24) & 0xFF;
    	pBuffer[4] = (Config >> 16) & 0xFF;
    	pBuffer[3] = (Config >> 8) & 0xFF;
    	pBuffer[2] = (Config >> 0) & 0xFF;
    	
    	pBuffer[9] = (Value >> 24) & 0xFF;
    	pBuffer[8] = (Value >> 16) & 0xFF;
    	pBuffer[7] = (Value >> 8) & 0xFF;
    	pBuffer[6] = (Value >> 0) & 0xFF;

    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0, 
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);

  //  	return nRet;
}

ULONG NcRpci_ReadConfigUlong(WDU_DEVICE_HANDLE hDevice, ULONG Config)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 6;

    	pBuffer[1] = 0x40;//01000000 PARK_EXTERNAL_REQUESTOR<<PCI_ARBITER_PARK_SELECT
    	pBuffer[0] = 0xBF;//10111111 (CFG_READ_OR_WRITE << PCI_MASTER_COMMAND_SELECT) | (1<<PCI_MASTER_START) | (PCI_MASTER_READ<<PCI_MASTER_READ_WRITE) | (0x0F<<PCI_MASTER_BYTE_ENABLES)
    	
    	pBuffer[5] = (Config >> 24) & 0xFF;
    	pBuffer[4] = (Config >> 16) & 0xFF;
    	pBuffer[3] = (Config >> 8) & 0xFF;
    	pBuffer[2] = (Config >> 0) & 0xFF;
    	
	TransferData(hDevice, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize,
		&dwBytesTransferred,
		NULL,
		TRANSFER_TIMEOUT);    	    		
	
//////////////////////////
	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIIN_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 4;
	nRet = TransferData(hDevice, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize, 
		&dwBytesTransferred, 
		NULL, 
		TRANSFER_TIMEOUT);

	return nRet;
}

NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteRegisterUlong       (PNC_RPCI_EXT prx,  ULONG Register, ULONG Value)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 10;

	pBuffer[1] = 0x40;//01000000 PARK_EXTERNAL_REQUESTOR<<PCI_ARBITER_PARK_SELECT
    	pBuffer[0] = 0x2F;//00101111 (MEM_READ_OR_WRITE << PCI_MASTER_COMMAND_SELECT) | (1<<PCI_MASTER_START) | (PCI_MASTER_WRITE<<PCI_MASTER_READ_WRITE) | (0x0F<<PCI_MASTER_BYTE_ENABLES)
    	    	
    	pBuffer[5] = (Register >> 24) & 0xFF;
    	pBuffer[4] = (Register >> 16) & 0xFF;
    	pBuffer[3] = (Register >> 8) & 0xFF;
    	pBuffer[2] = (Register >> 0) & 0xFF;
    	
    	pBuffer[9] = (Value >> 24) & 0xFF;
    	pBuffer[8] = (Value >> 16) & 0xFF;
    	pBuffer[7] = (Value >> 8) & 0xFF;
    	pBuffer[6] = (Value >> 0) & 0xFF;

    	nRet = TransferData(prx, 
    		dwPipeNum, 
    		fread==1, 
    		0, 
    		(PVOID)pBuffer, 
    		dwSize, 
    		&dwBytesTransferred, 
    		NULL, 
    		TRANSFER_TIMEOUT);
}

NC_RPCI_API ULONG   NC_RPCI_EXP NcRpci_ReadRegisterUlong        (PNC_RPCI_EXT prx,  ULONG Register)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	UCHAR pBuffer[10];
	ULONG nRet = 0;

	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIOUT_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 6;

    	pBuffer[1] = 0x40;//01000000 PARK_EXTERNAL_REQUESTOR<<PCI_ARBITER_PARK_SELECT
    	pBuffer[0] = 0x3F;//00111111 (MEM_READ_OR_WRITE << PCI_MASTER_COMMAND_SELECT) | (1<<PCI_MASTER_START) | (PCI_MASTER_READ<<PCI_MASTER_READ_WRITE) | (0x0F<<PCI_MASTER_BYTE_ENABLES)
    	
    	pBuffer[5] = (Register >> 24) & 0xFF;
    	pBuffer[4] = (Register >> 16) & 0xFF;
    	pBuffer[3] = (Register >> 8) & 0xFF;
    	pBuffer[2] = (Register >> 0) & 0xFF;
    	
	TransferData(prx, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize,
		&dwBytesTransferred,
		NULL,
		TRANSFER_TIMEOUT);    	    		
	
//////////////////////////
	memset(pBuffer, 0, 10);
    	dwPipeNum = PCIIN_EP_ADDRESS;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = 4;
	nRet = TransferData(prx, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize, 
		&dwBytesTransferred, 
		NULL, 
		TRANSFER_TIMEOUT);

	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// USB interrupt support
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Initialize USB interrupts
NC_RPCI_API NTSTATUS NC_RPCI_EXP 
NcRpci_InitializeUsbInterrupt(
    PNC_RPCI_EXT prx, 
    ULONG UsbIrqEnb1,   // See NET2280 spec: 11.5.8 USBIRQENB1
    NC_CLIENT_CALLBACK InterruptCallback,
    PVOID AnyDeviceObject
    )
{
	ULONG nRet = 0;
	
	nRet = Net2280RegWrite_UlongBar0(prx, USBIRQENB1, UsbIrqEnb1);
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// Arm for USB interrupt
NC_RPCI_API NTSTATUS NC_RPCI_EXP 
NcRpci_ArmUsbInterrupt(
    PNC_RPCI_EXT prx
    )
{
	ULONG nRet = 0, Data;
	
	nRet = Net2280RegRead_UlongBar0(prx, IRQSTAT1, &Data);
	nRet = Net2280RegRead_UlongBar0(prx, USBIRQENB1, &Data);

	Net2280RegWrite_UlongBar0(prx, USBIRQENB1, 0x00000000);
	Net2280RegWrite_UlongBar0(prx, IRQSTAT1, 0x00000000);
	Net2280RegWrite_UlongBar0(prx, USBIRQENB1, (1<<USB_INTERRUPT_ENABLE)|(1<< PCI_INTA_INTERRUPT_ENABLE));// 0x81000000);
	
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// NET2280 DMA controller functions:
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Map DMA controllers (Optional):
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_MapNcDma(PNC_RPCI_EXT prx)
{
	ULONG nRet = 0, Data;
	int ep = FIRST_USER_ENDPOINT;
	for(; ep <= ENDPOINT_COUNT; ep++)
	{
		Net2280RegRead_UlongBar0(prx, EP_CFG+EPPAGEOFFSET(ep), &Data);
		
		Net2280RegWrite_UlongBar0(prx, DMASTAT+DMACHANNELOFFSET(ep), 1 << DMA_ABORT);
		Net2280RegWrite_UlongBar0(prx, EP_STAT+EPPAGEOFFSET(ep), 1 << FIFO_FLUSH);
	}
	
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// Start DMA cycles on an endpoint using a NET2280 DMA controller (Optional):
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_KickNcDma(PNC_RPCI_EXT prx, PNC_URB pNcURB, PNC_DMA pNcDMA)
{
	ULONG nRet = 0;
	ULONG DmaCount, DmaControl, DmaStat, DmaAddress;
	int ep;
	int DmaChannelOffset;

	if ( pNcURB == NULL || pNcDMA == NULL )
	{
		return 0;
	}

	DmaStat = (1 << DMA_START);
	DmaAddress = pNcDMA->RemotePciAddress;
		
	DmaControl = (1<<DMA_ENABLE);
	DmaControl |= pNcDMA->DmaControl;
	
	DmaCount = pNcURB->TransferBufferLength;
	if ( EP_DIR(pNcURB->UsbEp) )
	{
		DmaCount  |= (1<<DMA_DIRECTION);
	}

	ep = (pNcURB->UsbEp) & 0x1F;
	DmaChannelOffset = DMACHANNELOFFSET(ep);

	Net2280RegWrite_UlongBar0(prx, DMAADDR+DmaChannelOffset, DmaAddress);
	Net2280RegWrite_UlongBar0(prx, DMACOUNT+DmaChannelOffset, DmaCount);
	Net2280RegWrite_UlongBar0(prx, DMACTL+DmaChannelOffset, DmaControl);
	Net2280RegWrite_UlongBar0(prx, DMASTAT+DmaChannelOffset, DmaStat);

	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// USB transfer functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Start USB transfer:
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_StartUsbTransfer(PNC_RPCI_EXT prx, PNC_URB pNcURB, PNC_DMA pNcDMA)
{
	DWORD fread, dwPipeNum, dwSize, dwBytesTransferred;
	ULONG nRet  = 0;
	UCHAR* pBuffer;

	if ( pNcURB == NULL )
		return -1;
		
	nRet = NcRpci_KickNcDma(prx, pNcURB, pNcDMA);
	//printf("NcRpci_KickNcDma :: return=%d\n", (int)nRet);

	/* */
    	dwPipeNum = pNcURB->UsbEp;
    	fread = EP_DIR(dwPipeNum);
    	dwSize = pNcURB->TransferBufferLength;
    	pBuffer =(UCHAR *)pNcURB->TransferBuffer;
    				
	nRet = TransferData(prx, 
		dwPipeNum, 
		fread==1, 
		0, 
		(PVOID)pBuffer, 
		dwSize,
		&dwBytesTransferred,
		NULL,
		TRANSFER_TIMEOUT);
	//printf("NcRpci_StartUsbTransfer :: return=%d, %d transferred\n", (int)nRet, (int)dwBytesTransferred);

	pNcURB->TransferBufferLength = dwBytesTransferred;
	
	return nRet;
}


///////////////////////////////////////////////////////////////////////////////
// Cancel USB transfer:
NC_RPCI_API ULONG       NC_RPCI_EXP NcRpci_CancelUsbTransfer(PNC_RPCI_EXT prx, PNC_URB pNcURB)
{
	ULONG nRet = 0;
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// Cancel USB transfer and wait:
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_CancelUsbTransferAndWait(PNC_RPCI_EXT prx, PNC_URB pNcURB, PLARGE_INTEGER pWaitTime)
{
	ULONG nRet = 0;

	if ( pNcURB == NULL )
		return -1;
	
	nRet = WDU_HaltTransfer(prx, pNcURB->UsbEp);
	//printf("NcRpci_CancelUsbTransferAndWait :: return=%d\n", (int)nRet);
	return nRet;
}


/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Interrupt handler
///////////////////////////////////////////////////////////////////////////////
VOID InterruptCallback(PNC_RPCI_EXT prx, ULONG Interrupt_IrqStat1 )
{
	//  - NOTE: Interrupt_IrqStat1 might be zero. This handler might 
	//    occasionally get called when there is no interrupt event. In this
	//    case, Interrupt_IrqStat1 is zero; except for rearming interrupts,
	//    no further action is required.
	if (Interrupt_IrqStat1 != 0)
	{   // The USB device interrupted:
	//  - At least one of the NET2280's enabled interrupts has occurred,
	//    causing the NET2280 STATIN transfer to complete. NET2280 
	//    interrupt status is returned in Interrupt_IrqStat1
	//  - See NET2280: STATIN endpoint, IRQSTAT1, IRQENB1
	//  - Most often, the interrupt is due to your PCI chip 
	//    asserting its INTA pin
	// TODO: Clear the source of all interrupts before rearming:
	//  - For instance, if the interrupt callback is due to the INTA pin, 
	//    the INTA pin must be cleared now, before re-arming for another 
	//    interrupt. (Since the INTA pin is an input to the NET2280, driven 
	//    by your PCI chip, you must write to your PCI chip to de-assert 
	//    the INTA pin.)
	}

	// Re-arm for another interrupt
	NcRpci_ArmUsbInterrupt(prx);
}


///////////////////////////////////////////////////////////////////////////////
// PCI enumeration
//  - Required only when using an "unknown" PCI adapter: We dynamically
//    configure the PCI device. It shouldn't be needed once a single, 
//    known PCI adapter chip is connected. Then fixed addresses can be applied!
///////////////////////////////////////////////////////////////////////////////
static ULONG GetBar( ULONG SpaceMask )
{   // Calculate a BAR suitable for writing to the BAR based on the BAR's
    // space requirement:
    //  - This is part of "dynamic configuration." It shouldn't be needed once
    //    a single PCI adapter chip is chosen. Fixed addresses can be applied!
    //  - Space Mask is the value read from the BAR after writing all ones
    //  - There is plenty of space. For ease-of-use, spread BARs out over
    //    a convenient increment
    static ULONG NextBar = BASE_INCREMENT;
    ULONG ThisBar;
    // "Normalize" the Space Mask and its BAR control bits
    //  - For example Space Mask 0xffffff03 simply becomes 0x100
    ULONG SpaceSize = 0 - (SpaceMask & ~0x0f);

    // Calculate for next BAR based on this BAR's requirement
    if (SpaceSize <= BASE_INCREMENT)
    {   // Requirement is small; less than (or equal to) our increment
        //  - Only need to advance by one increment
        ThisBar = NextBar;
        NextBar += BASE_INCREMENT;
    }
    else
    {   // Requirement is larger than our increment
        //  - The caller's BAR must align within its Space Mask bits
        //  - The next BAR accounts for alignment plus required space
        ThisBar = (NextBar & SpaceMask) + SpaceSize;
        NextBar = ThisBar + SpaceSize;
    }
    
    return ThisBar;
}

extern "C"	void StartDevice(WDU_DEVICE_HANDLE hDevice)
{
	ULONG NtStatus = 0;
	ULONG BarOffset = 0;
	ULONG SpaceMask;

	// Initial NET2280 DEVINIT settings
	//  - Settings should account for optional EEPROM and on-board 8051 program
	ULONG DevInit;
	Net2280RegRead_UlongBar0(hDevice, DEVINIT, &DevInit);
	Net2280RegWrite_UlongBar0(hDevice, DEVINIT, 
		// Starting with current DEVINIT settings...
		DevInit & ~(
		// These DEVINIT bits do *not* get set:
		(1<<FORCE_PCI_RESET) |  // Make sure remote PCI comes out of reset
		(1<<PCI_SOFT_RESET) |   // EEPROM or 8051 might have setup NET2280 config registers
		0) | (
		// These DEVINIT bits *do* get set:
		(1<<PCI_ENABLE) |        // Allow remote PCI device to access NET2280 BAR spaces
		0));

	// Program NET2280 to accept PCI memory bus master cycles from 
	// remote PCI devices
	Net2280RegWrite_UlongConfig(hDevice, PCICMD, 
		NC_BIT(MEMORY_WRITE_AND_INVALIDATE_ENABLE) |
		NC_BIT(BUS_MASTER_ENABLE) |
		NC_BIT(MEMORY_SPACE_ENABLE) |
		0);
    
	// Program remote PCI device to accept PCI memory and I/O cycles
	NcRpci_WriteConfigUlong(hDevice, IDSEL + PCICMD, 
		NC_BIT(BUS_MASTER_ENABLE) |
		NC_BIT(MEMORY_SPACE_ENABLE) |
		NC_BIT(IO_SPACE_ENABLE) |
		0);

	// Dynamic PCI device enumeration:
	//  - Configure remote device's BAR registers
	//  - Note: Dynamic configuration applies when we don't know what PCI
	//    board is connected. 
	//  - This section should be replaced with hard-coded values once a
	//    single PCI adapter is chosen. (See ClientZero for example)
	
//	ULONG RunningBar = BASE_INCREMENT;  // First BAR set to this value
	for (BarOffset = PCIBASE0; BarOffset <= PCIROMBASE; BarOffset += sizeof(ULONG))
	{
		switch (BarOffset)
		{
			case PCIBASE0:
			case PCIBASE1:
			case PCIBASE2:
			case PCIBASE3:
			case PCIBASE4:
			case PCIBASE5:
			case PCIROMBASE:

			// First, write "all ones" to config register
			NcRpci_WriteConfigUlong(hDevice, IDSEL + BarOffset, 0xFFFFFFFF);

			// Read back resource requirements and allocate BAR resources
			SpaceMask = NcRpci_ReadConfigUlong(hDevice, IDSEL + BarOffset);

			if (SpaceMask >= 0x10)
			{   // Device's BAR requires resources
				//  - Set the BAR
				//  - (I/O and memory types are not distinguished)
				NcRpci_WriteConfigUlong(hDevice, IDSEL + BarOffset, GetBar(SpaceMask));
			}
		}
	}
    


	// TODO: Setup DMA controllers:
	//  - You can use NET2280 DMA or your adapter's DMA controller(s)
	//  - Difficult choices: The NET2280's four DMA controllers are optimized
	//    for USB; on the other hand, your hardware might not be able to take 
	//    advantage of them. 
	//  - If your hardware does not have a DMA controller, the choice is easy
	//  - The following switch assumes one DMA method or the other however 
	//    both methods can be used together safely. For instance, consider 
	//    using NET2280 DMA to download large tables or microcode.

#ifdef CLIENT_USING_NET2280_DMA
	// You are using NET2280 DMA controllers:
	//  - Call RPCI's DMA mapping function now (mapping only needs 
	//    to be done once, usually in StartDevice())
	NtStatus = NcRpci_MapNcDma(hDevice);
	// Now, to transfer data using NET2280 DMA include an NC_DMA structure
	// in your call to NcRpci_StartUsbTransfer()
#else
	// You are using your PCI adapter's DMA hardware:
	//  - Setup your DMA to address NET2280 BAR2 address space. The
	//    NET2280 BAR2 feature maps NET2280 FIFOs to its BAR2 addresses,
	//    allowing your DMA to transfer data over arbitrary-sized
	//    ranges in PCI space. (See NET2280 spec: 6.4 Target Transactions) 
	//  - Be sure to program NET2280 FIFOCTL as needed and PCIBASE2 
	//    appropriately.
	//  - NET2280 spec: "Starting with [FIFOCTL] bit 16, as each 
	//    successive bit is changed to a 0, the range doubles."
	//  - NET2280 BAR2 must be set such that it is a multiple of the 
	//    following PCI Base2 Range setting. That is, if the range 
	//    is 64K then BAR2 must be a multiple of 64K.
	ULONG FifoControl = 
		//((0-4)<<PCI_BASE2_RANGE) |          // 0-4 is 0xfffc, which sets the range to 256K, or 64K per endpoint
		//(0-0x400000) |
		(0-4*DMA_BUFFER_SIZE) |
		(0<<PCI_BASE2_SELECT) |
		(0<<FIFO_CONFIGURATION_SELECT) |
		0;
	NtStatus = Net2280RegWrite_UlongBar0(hDevice, FIFOCTL, FifoControl);
	Net2280RegWrite_UlongConfig(hDevice, PCIBASE2, GetBar(FifoControl));

	ULONG val, nEPA;
	Net2280RegRead_UlongBar0(hDevice, FIFOCTL, &val);
	printf("FIFOCTL=0x%08X\n", val);	
	Net2280RegRead_UlongBar0(hDevice, 0x320, &val);
	printf("EP_CFG=0x%08X\n", val);
	Net2280RegRead_UlongConfig(hDevice, PCIBASE2, &nEPA);
	printf("PCIBASE2=0x%08X\n", nEPA);

	ULONG PciBase0,PciBase2;
	PciBase0 = NcRpci_ReadConfigUlong(hDevice, 0x10000+PCIBASE0);
	PciBase2 = NcRpci_ReadConfigUlong(hDevice, 0x10000+PCIBASE2);

	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x04*4, 0x00000001 );	//LAS0BA	: ===> SPACE0 Enable
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x08*4, 0x03200000 );	//MARBR : 0000 0011 0010 0000 0000 0000 0000 0000 ===> PCI Read No WRite Mode, Delayed ReadMode, Local Bus Direct Slave Release Bus Mode
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x18*4, 0x89430043 );	//LBRD0
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x20*4, 0x00000143 );
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x21*4, nEPA );		//PCI Remote Address
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x22*4, 0 );			//Local Address
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x23*4, DMA_BUFFER_SIZE );	//DMA Size
	NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x24*4, 0x09 );		//DMA Read Mode

#endif

	///////////////////////////////////////////////////////////////////////////
	// TODO: Setup USB interrupts
	NtStatus = NcRpci_InitializeUsbInterrupt(
		hDevice,                                // PNC_RPCI_EXT prx, 
			(   // Enable these interrupts in NET2280 USBIRQENB1 register:
			//  - USB Interrupt Enable (bit 31) must be set to get *any* 
			//    interrupt (plus at least one other bit)
			//  - By far, the most popular interrupt is PCI INTA Interrupt
			NC_BIT(USB_INTERRUPT_ENABLE) |  
			// NC_BIT(POWER_STATE_CHANGE_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_ARBITER_TIMEOUT_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_PARITY_ERROR_INTERRUPT_ENABLE) | 
			NC_BIT(PCI_INTA_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_PME_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_SERR_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_PERR_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_MASTER_ABORT_RECEIVED_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_TARGET_ABORT_RECEIVED_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_RETRY_ABORT_INTERRUPT_ENABLE) | 
			// NC_BIT(PCI_MASTER_CYCLE_DONE_INTERRUPT_ENABLE) | 
			// NC_BIT(VIRTUALIZED_ENDPOINT_INTERRUPT_ENABLE) | 
			// NC_BIT(GPIO_INTERRUPT_ENABLE) | 
			// NC_BIT(DMA_D_INTERRUPT_ENABLE) | 
			// NC_BIT(DMA_C_INTERRUPT_ENABLE) | 
			// NC_BIT(DMA_B_INTERRUPT_ENABLE) | 
			// NC_BIT(DMA_A_INTERRUPT_ENABLE) | 
			// NC_BIT(EEPROM_DONE_INTERRUPT_ENABLE) | 
			// NC_BIT(VBUS_INTERRUPT_ENABLE) | 
			// NC_BIT(CONTROL_STATUS_INTERRUPT_ENABLE) | 
			// NC_BIT(ROOT_PORT_RESET_INTERRUPT_ENABLE) | 
			// NC_BIT(SUSPEND_REQUEST_INTERRUPT_ENABLE) | 
			// NC_BIT(SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE) | 
			// NC_BIT(RESUME_INTERRUPT_ENABLE) | 
			// NC_BIT(SOF_INTERRUPT_ENABLE) | 
			0
		),                                      // ULONG UsbIrqEnb1,
		(NC_CLIENT_CALLBACK)InterruptCallback,  // NC_CLIENT_CALLBACK InterruptCallback,
		NULL                                     // PVOID AnyDeviceObject
		);
	if (NtStatus != 0)
	{   
		// Failed to setup USB interrupts
		//NCPRINTF(VOLUME_MINIMUM, ("StartDevice(): NcRpci_InitializeUsbInterrupt(): Failed: NtStatus:%x \n", NtStatus));
		//return NtStatus;
	}

	// Arm for USB interrupts
	NcRpci_ArmUsbInterrupt(hDevice);
	
//	if (pDevice)
//		WDU_PutDeviceInfo(pDevice);
}

extern "C"	ULONG DispatchControl(WDU_DEVICE_HANDLE hDevice, void* SystemBuffer, int ControlCode)
{
	ULONG nRet = 0;
	ULONG PciBase0, PciBase2;
	KCMD_ARGS *pKReg=NULL;

	PciBase0 = NcRpci_ReadConfigUlong(hDevice, IDSEL+PCIBASE0);
	PciBase2 = NcRpci_ReadConfigUlong(hDevice, IDSEL+PCIBASE2);
	//printf("DispatchControl[0x%x], BAR0[0x%08X], BAR2[0x%08X]\n", ControlCode, (int)PciBase0, (int)PciBase2);
	//PciBase0[0x00010000], PciBase2[0x02000000]

	pKReg = (KCMD_ARGS *)SystemBuffer;
	
	switch (ControlCode)
	{
		case IOCTL_SET_FIFO_CONTROL : // TVB595V1
		{
			//PCI Remote Address
			ULONG nEPA;
			Net2280RegRead_UlongConfig(hDevice, PCIBASE2, &nEPA);
				
			if ( pKReg->dwCmdParm1 == 1 )
				//sskim20080311 - fixed
				//nEPA += pKReg->dwCmdParm2;
				nEPA += DMA_BUFFER_SIZE;
			NcRpci_WriteRegisterUlong(hDevice, PciBase0 + 0x21*4, nEPA );
			
			break;
		}

		case IOCTL_TVB350_EPLD_DOWN :
		{
			NC_URB NcUrb;
			NC_DMA NcDma;

			memset(&NcUrb, sizeof(NcUrb), 0);
			memset(&NcDma, sizeof(NcDma), 0);

			NcUrb.TransferBufferLength = (pKReg->dwCmdParm2 & 0xFFFFFF);
			NcUrb.TransferBuffer = (void*)pKReg->pdwBuffer;
			NcUrb.UsbEp = EPA;
			NcDma.RemotePciAddress = PciBase2 + pKReg->dwCmdParm1*4;
			NcDma.DmaControl = 1;
//			if ( ControlCode == IOCTL_FILE_DATA_DOWN_DMA_ADDR_HOLD )
//				NcDma.DmaControl += (1<<DMA_ADDRESS_HOLD);

			nRet = NcRpci_StartUsbTransfer(hDevice, &NcUrb, &NcDma);
			if  ( nRet != 0 )
			{
				NcRpci_CancelUsbTransferAndWait(hDevice, &NcUrb, NULL);
			}
			
			break;
		}
			
		case IOCTL_FILE_DATA_DOWN :
//		case IOCTL_FILE_DATA_DOWN_DMA_ADDR_HOLD : 
		{
			NC_URB NcUrb;
			NC_DMA NcDma;

			memset(&NcUrb, sizeof(NcUrb), 0);
			memset(&NcDma, sizeof(NcDma), 0);

			NcUrb.TransferBufferLength = (pKReg->dwCmdParm2 & 0xFFFFFF);
			NcUrb.TransferBuffer = (void*)pKReg->pdwBuffer;
			NcUrb.UsbEp = EPA;
			NcDma.RemotePciAddress = PciBase2 + pKReg->dwCmdParm1*4;
			NcDma.DmaControl = 0;
//			if ( ControlCode == IOCTL_FILE_DATA_DOWN_DMA_ADDR_HOLD )
//				NcDma.DmaControl += (1<<DMA_ADDRESS_HOLD);

#ifdef CLIENT_USING_NET2280_DMA
			nRet = NcRpci_StartUsbTransfer(hDevice, &NcUrb, &NcDma);
#else
			nRet = NcRpci_StartUsbTransfer(hDevice, &NcUrb, NULL);
#endif
			if  ( nRet != 0 )
			{
				NcRpci_CancelUsbTransferAndWait(hDevice, &NcUrb, NULL);
			}
			
			break;
		}
			
		case IOCTL_FILE_DATA_UP :
//		case IOCTL_FILE_DATA_UP_DMA_ADDR_HOLD : 
		{
			NC_URB NcUrb;
			NC_DMA NcDma;

			memset(&NcUrb, sizeof(NcUrb), 0);
			memset(&NcDma, sizeof(NcDma), 0);

			NcUrb.TransferBufferLength = (pKReg->dwCmdParm2 & 0xFFFFFF);
			NcUrb.TransferBuffer = (void*)pKReg->pdwBuffer;
			NcUrb.UsbEp = EPB+(1<<ENDPOINT_DIRECTION);
			NcDma.RemotePciAddress = PciBase2 + pKReg->dwCmdParm1*4;
			NcDma.DmaControl = 0;
//			if ( ControlCode == IOCTL_FILE_DATA_UP_DMA_ADDR_HOLD )
//				NcDma.DmaControl += (1<<DMA_ADDRESS_HOLD);

#ifdef CLIENT_USING_NET2280_DMA
			nRet = NcRpci_StartUsbTransfer(hDevice, &NcUrb, &NcDma);
#else
			nRet = NcRpci_StartUsbTransfer(hDevice, &NcUrb, NULL);
#endif							
			if  ( nRet != 0 )
			{
				NcRpci_CancelUsbTransferAndWait(hDevice, &NcUrb, NULL);
			}
			break;			
		}
				
		case IOCTL_WRITE_PCI9054_REG :
			NcRpci_WriteRegisterUlong(hDevice, 
				PciBase0 + (pKReg->dwCmdParm1 & 0x3f)*4, pKReg->dwCmdParm2 );
			break;

		case IOCTL_READ_PCI9054_REG :
			pKReg->dwCmdParm2 = NcRpci_ReadRegisterUlong(hDevice,
				PciBase0 + (pKReg->dwCmdParm1 & 0x3f)*4 );
			break;

		case IOCTL_WRITE_TO_MEMORY :
		{
			if ( ___get_TSPL_nBoardTypeID() == 360 )
			{
				if ( (pKReg->dwCmdParm1 >= 0x200 && pKReg->dwCmdParm1 < 0x300)
				 	 ||	 (pKReg->dwCmdParm1 >= 0x700 && pKReg->dwCmdParm1 < 0x800) )
				{
					// On Tuner Frequency/Symbolrate Setting, need some delay
					// If this is not present, it is fail to set RF frequency
//					LPRINT(fn_Log,"[DRIVER] IOCTL, WRITE_TO_MEMORY FOR RF out\n");		
				}
			}
			NcRpci_WriteRegisterUlong(hDevice, 
				PciBase2 + (pKReg->dwCmdParm1 & 0x7fffff)*4, pKReg->dwCmdParm2 );

			break;
		}

		case IOCTL_READ_FROM_MEMORY :
		{
			pKReg->dwCmdParm2 = NcRpci_ReadRegisterUlong(hDevice, 
				PciBase2 + (pKReg->dwCmdParm1 & 0x7fffff)*4 );
			
			if ( ___get_TSPL_nBoardTypeID() == 360 )
			{
	//			dwDummy = pKReg->dwCmdParm2;
	//			kCmd.dwCmdParm2 =swaplong(dwDummy);
			}
			
			break;
		}
//kslee:modify =====================================================================================================
		case IOCTL_ENABLED_RW_EEPROM:
		{
			pKReg->dwCmdParm2 = 1;
			break;
		}
		case IOCTL_READ_EEPROM_AT25640:
		{
			USHORT RegValue;
			Net2282_Eeprom_Read(hDevice, (UINT32)pKReg->dwCmdParm1, &RegValue);
			pKReg->dwCmdParm2 = RegValue;
			break;
		}
		case IOCTL_WRITE_EEPROM_AT25640:
		{
			Net2282_Eeprom_Write(hDevice, (UINT32)pKReg->dwCmdParm1, (USHORT)pKReg->dwCmdParm2);
			break;
		}
		case IOCTL_GET_FAST_DOWN:
		{
			pKReg->dwCmdParm2 = 1;
			break;
		}
//===============================================================================================================
		case IOCTL_TSP100_EPLD_DOWN:    
		{
#if 0			
			int i,j;
			ULONG data;
			UCHAR buf;
			KALTERA_RBF *pRBF_Reg;
			pRBF_Reg = (KALTERA_RBF *)SystemBuffer;

			for ( i = 0; i < (int)pRBF_Reg->Size; i++)
			{
				buf = *(pRBF_Reg->pBuffer + i);

				for ( j = 0; j < 8; j++ )
				{
					if (___get_TSPL_nBoardTypeID() == 360)
					{
						data = (ULONG)((buf & 0x01) << 25);
						NcRpci_WriteRegisterUlong(hDevice, PciBase2 + 0x000000, ((0xc0000000) | data));
						NcRpci_WriteRegisterUlong(hDevice, PciBase2 + 0x000000, ((0xc4000000) | data));
					}
					else
					{
						data = (ULONG)((buf & 0x01) << 1);
						NcRpci_WriteRegisterUlong(hDevice, PciBase2 + 0x400000*4, ((0x000000c0) | data));
						NcRpci_WriteRegisterUlong(hDevice, PciBase2 + 0x400000*4, ((0x000000c4) | data));
					}

					buf >>= 1;
				}

				//printf("Downloading...%dBytes\n", i);
			}
#endif			
			break;
		}

#if 0
		case IOCTL_TVB4961_EPLD_DOWN:    
		{
			int i;
			unsigned long data;
			KALTERA_RBF *pRBF_Reg;
			pRBF_Reg = (KALTERA_RBF *)SystemBuffer;

			for ( i = 0; i < (int)pRBF_Reg->Size; i+=sizeof(unsigned long))
			{
				memcpy(&data, pRBF_Reg->pBuffer + i, sizeof(unsigned long));
				NcRpci_WriteRegisterUlong(hDevice, PciBase2 + 0x4000*4, data);
			}
			break;
		}
#endif			

		default:
			break;
		
	}
		
	return nRet;
}
#define NET2282_EE_OPCODE_READ             3
#define NET2282_EE_OPCODE_READ_STATUS      5
#define NET2282_EE_OPCODE_WRITE_ENABLE     6
#define NET2282_EE_OPCODE_WRITE            2
#define NET2282_EE_OPCODE_WRITE_STATUS     1
int Net2282_Eeprom_CheckBusy(WDU_DEVICE_HANDLE hDevice)
{
	ULONG Timeout;
	LONG nRet;
	ULONG value;
	Timeout = 100000;
	do
	{
		nRet = Net2280RegRead_UlongBar0(hDevice, EECTL, &value);
		Timeout--;
	} while(Timeout && (value & (1 << 19)));

	if(Timeout == 0)
		return FALSE;

	return TRUE;
}

int Net2282_EepromDataWrite(WDU_DEVICE_HANDLE hDevice, UCHAR data)
{
	ULONG RegValue;

	if(Net2282_Eeprom_CheckBusy(hDevice) == FALSE)
	{
		return FALSE;
	}

	RegValue = (1 << 18) | (1 << 16) | data;

	Net2280RegWrite_UlongBar0(hDevice, EECTL, RegValue);

	return Net2282_Eeprom_CheckBusy(hDevice);
}

int Net2282_EepromDataRead(WDU_DEVICE_HANDLE hDevice, UCHAR *data)
{
	ULONG RegValue;
	int rc;
	LONG nRet;

	if(Net2282_Eeprom_CheckBusy(hDevice) == FALSE)
		return FALSE;

	RegValue = (1 << 18) | (1 << 17);

	Net2280RegWrite_UlongBar0(hDevice, EECTL, RegValue);

	rc = Net2282_Eeprom_CheckBusy(hDevice);
	
	if(rc == TRUE)
	{
		nRet = Net2280RegRead_UlongBar0(hDevice, EECTL, &RegValue);
		*data = (UCHAR)(RegValue >> 8);
	}

	return rc;
}

int Net2282_EepromWait(WDU_DEVICE_HANDLE hDevice)
{
	UCHAR status;
	USHORT Timeout;

	Timeout = 200;
	do
	{
		Net2282_EepromDataWrite(hDevice, NET2282_EE_OPCODE_READ_STATUS);
		Net2282_EepromDataRead(hDevice, &status);
		Net2280RegWrite_UlongBar0(hDevice, EECTL, 0);

		if((status & (1 << 0)) == 0)
			return TRUE;

		Timeout--;
	}
	while(Timeout);

	return FALSE;
}
int Net2282_Eeprom_Read(WDU_DEVICE_HANDLE hDevice, UINT32 offset, USHORT *value)
{
	UCHAR i;
	UCHAR EepData;
	ULONG RegValue;
	LONG nRet;

	*value = 0;

	if(Net2282_EepromWait(hDevice) == FALSE)
		return FALSE;

	Net2282_EepromDataWrite(hDevice, NET2282_EE_OPCODE_READ);

	nRet = Net2280RegRead_UlongBar0(hDevice, EECTL, &RegValue);
	RegValue = (RegValue >> 23) & 0x3;

	if(RegValue == 0)
		RegValue = 2;

	if(RegValue == 3)
	{
		Net2282_EepromDataWrite(hDevice, 0);
	}

	if(RegValue == 2 || RegValue == 3)
	{
		Net2282_EepromDataWrite(hDevice, (UCHAR)(offset >> 8));
	}

	Net2282_EepromDataWrite(hDevice, (UCHAR)offset);

	for(i = 0; i < 2; i++)
	{
		Net2282_EepromDataRead(hDevice, &EepData);
		if(i == 0)
			*value |= EepData;
		else
			*value |= ((USHORT)EepData<<8);
	}

	Net2280RegWrite_UlongBar0(hDevice, EECTL, 0);

	return TRUE;
}

int Net2282_Eeprom_Write(WDU_DEVICE_HANDLE hDevice, UINT32 offset, USHORT value)
{
	UCHAR i;
	UINT32 RegValue;
	LONG nRet;

	if(Net2282_EepromWait(hDevice) == FALSE)
		return FALSE;

	Net2282_EepromDataWrite(hDevice, NET2282_EE_OPCODE_WRITE_ENABLE);

	Net2280RegWrite_UlongBar0(hDevice, EECTL, 0);

	Net2282_EepromDataWrite(hDevice, NET2282_EE_OPCODE_WRITE);

	Net2280RegRead_UlongBar0(hDevice, EECTL, &RegValue);
	RegValue = (RegValue >> 23) & 0x3;

	if(RegValue == 0)
		RegValue = 2;

	if(RegValue == 3)
	{
		Net2282_EepromDataWrite(hDevice, 0);
	}

	if(RegValue == 2 || RegValue == 3)
	{
		Net2282_EepromDataWrite(hDevice, (UCHAR)(offset >> 8));
	}

	Net2282_EepromDataWrite(hDevice, (UCHAR)offset);

	for(i = 0; i < 2; i++)
	{
		Net2282_EepromDataWrite(hDevice, (UCHAR)value);
		value >>= 8;
	}

	Net2280RegWrite_UlongBar0(hDevice, EECTL, 0);

	return TRUE;
}
