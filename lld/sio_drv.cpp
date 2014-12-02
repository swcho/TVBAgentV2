//	
//	SIO_Drv.c / TSP100DLL.DLL
//
//	Send and receive character to front panel
//
//	Copyright (C) 2000-20001
//	Teleview Corporation
//	
//	PUBLIC
//		int	TSPL_PUT_CHAR(long dwCommand)
//		DWORD	TSPL_GET_CHAR(void)
//
//	Extern variable - fDMA_Busy
//

#include	<stdio.h>
#include	<stdlib.h>
#ifdef	WIN32
#include	<conio.h>
#include	<windows.h>
#include	<winioctl.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<commctrl.h>
#include	<stdio.h>
#include	<time.h>
#include	<setupapi.h>
#include	<memory.h>

#include	"initguid.h"
#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"

#include	"Reg590S.h"
#include	"Reg593.h"
#endif
#include	"sio_drv.h"


/////////////////////////////////////////////////////////////////
CSioDrv::CSioDrv(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;

}
CSioDrv::~CSioDrv()
{
}

//
//	Write a 32 bit word to address 0x400.000 of board
//	return 0 on fail
//

int	_stdcall CSioDrv::TSPL_PUT_CHAR(long dwCommand)
{
#ifdef	WIN32
	KCMD_ARGS		KCmdInf;
	DWORD 			dwRet;
	int			dwBank;
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32  
	if (hDevice == NULL)
		return TLV_NO_DEVICE;	// fault return
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if (hDevice_usb == NULL)
			return TLV_NO_DEVICE;	// fault return
	}
	else
	{
		if (hDevice == NULL)
			return TLV_NO_DEVICE;	// fault return
	}
#endif

	dwBank = 0;
	while (dwBank == 0)
	{
		if (fDMA_Busy ==0)
		{
			dwBank = (WDM_Read_TSP(TSP_MEM_ADDR_TX_EMPTY)) & 0x1;	// 1 if RS232C transmitter is ready
		}
		else
			return 0;	// DEBUG Sleep(1);
        }
	
	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_TXD;
	KCmdInf.dwCmdParm2 = (DWORD) (dwCommand & 0xff);

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32  
	TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
	else
	{
		TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
#endif
#endif
	return 0;
}

DWORD _stdcall CSioDrv::TSPL_GET_CHAR(void)
{
#ifdef	WIN32
	DWORD		dwRead_val;

	if (fDMA_Busy == 0)
	{	
		/* Should not interfere DMA operation */
		dwRead_val = (WDM_Read_TSP(TSP_MEM_ADDR_RXD)) & 0xff;
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			WDM_WR2SDRAM_ADDR(PCI590S_REG_FRONT_232_RX_BUF_CLR, CARD_CLEAR_RXD);
		}
		else if ( IsAttachedBdTyp_NewAddrMap() )
		{
			WDM_WR2SDRAM_ADDR(PCI593_REG_FRONT_232_RX_BUF_CLR, CARD_CLEAR_RXD);
		}
		else
		{
			WDM_WR2SDRAM(CARD_CLEAR_RXD);
		}

		return (DWORD)dwRead_val;
	}
	else 
		return 0;	// PROTECT corrupting DMA
#else
	return	0;
#endif

}

