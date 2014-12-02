//=================================================================	
//	PlayCapt.c  / Play Configurtion for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

//=================================================================	
#ifdef WIN32

#include	<stdio.h>
#include	<stdlib.h>
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
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"
#include	"rs.h"
#include	"cif.h"
#include 	"playcapt.h"
//=================================================================	
// Linux
#else
#define _FILE_OFFSET_BITS 64
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include	<time.h>
#include 	<memory.h>

#include 	"../tsp100.h"
#include	"dll_error.h"
#include 	"../include/logfile.h"
#include 	"wdm_drv.h"
#include 	"dma_drv.h"
#include 	"mainmode.h"
#include 	"rs.h"
#include 	"cif.h"
#include 	"playcapt.h"
#include	"ctl_mgr.h"
#endif

#include	"Reg590S.h"
#include	"Reg593.h"



/////////////////////////////////////////////////////////////////
CLldPlayCapt::CLldPlayCapt(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 1;

	TL_nLIFormat = -1;//FORMAT_5592==0, FORMAT_5376==1
}
CLldPlayCapt::~CLldPlayCapt()
{
}

/*^^***************************************************************************
 * Description : SDRAM bank count, 0~7
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CLldPlayCapt::SetSDRAMBankConfig(int nBankConfig)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint_FCall("SetSDRAMBankConfig", nBankConfig, 0);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
		return TLV_NO_DEVICE;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if(!CHK_DEV(hDevice_usb))
			return TLV_NO_DEVICE;
	}
	else
	{
		if(!CHK_DEV(hDevice))
			return TLV_NO_DEVICE;
	}
#endif

	LockDmaMutex();
	KCmdInf.dwCmdParm1 = TSP_SDRAM_BANK_CONFIG;
	KCmdInf.dwCmdParm2 = (DWORD) nBankConfig & 0x07;	// 3bit

	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		KCmdInf.dwCmdParm2 = (KCmdInf.dwCmdParm2<<16) + TSPL_nBankOffset;
	}

	KCmdInf.dwCmdParm2 += (TSPL_nMemoryCtrl_mode <<11);

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

#ifdef WIN32
	TLV_DeviceIoControl(hDevice,
		IOCTL_WRITE_TO_MEMORY,
		&KCmdInf,
		sizeof(KCmdInf),
		NULL, 
		0, 
		&dwRet,
		0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}
	else
	{
		TLV_DeviceIoControl(hDevice,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}

#endif

	UnlockDmaMutex();
	return 0;
}

/*^^***************************************************************************
 * Description : SDRAM bank offset, ~1024, a power of 2
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CLldPlayCapt::SetSDRAMBankOffsetConfig(int nBankConfig)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint_FCall("SetSDRAMBankOffsetConfig", nBankConfig, 0);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
		return TLV_NO_DEVICE;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
			return TLV_NO_DEVICE;
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
			return TLV_NO_DEVICE;
	}
#endif
	LockDmaMutex();
	KCmdInf.dwCmdParm1 = TSP_SDRAM_BANK_OFFSET_CONFIG;
	KCmdInf.dwCmdParm2 = (DWORD) nBankConfig & 0x07ff;	// 11bit

	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		KCmdInf.dwCmdParm2 += (TSPL_nBankCount <<16);
	}

	KCmdInf.dwCmdParm2 += (TSPL_nMemoryCtrl_mode <<11);

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	TLV_DeviceIoControl(hDevice,
		IOCTL_WRITE_TO_MEMORY,
		&KCmdInf,
		sizeof(KCmdInf),
		NULL, 
		0, 
		&dwRet,
		0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}
	else
	{
		TLV_DeviceIoControl(hDevice,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}
#endif
	UnlockDmaMutex();
	return 0;
}

/*^^***************************************************************************
 * Description : SDRAM bank count/offset
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_SET_SDRAM_BANK_INFO(int nBankNumber, int nBankOffset)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;

	if (IsAttachedBdTyp__Virtual())
	{
		TSPL_nBankOffset = nBankOffset;
		TSPL_nBankCount = nBankNumber;

		_MyCnxt()->_fpga_sdram_bnk_offset = TSPL_nBankOffset;
		_MyCnxt()->_fpga_sdram_bnk_cnt = TSPL_nBankCount;
		DupBdCnxt_Step5((void *)_MyCnxt());
		return 0;
	}

	LockEpldMutex();
	LldPrint("TSPL_SET_SDRAM_BANK_INFO", nBankNumber, nBankOffset, 0);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
	{
		UnlockEpldMutex();
		return TLV_NO_DEVICE;
	}
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
		{
			UnlockEpldMutex();
			return TLV_NO_DEVICE;
		}
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
		{
			UnlockEpldMutex();
			return TLV_NO_DEVICE;
		}
	}
#endif

	//TVB593
	TSPL_nBankOffset = nBankOffset;
	TSPL_nBankCount = nBankNumber;
	
	_MyCnxt()->_fpga_sdram_bnk_offset = TSPL_nBankOffset;
	_MyCnxt()->_fpga_sdram_bnk_cnt = TSPL_nBankCount;
	DupBdCnxt_Step5((void *)_MyCnxt());
	
	if ( nBankNumber < 0 || nBankNumber > 7 )
		nBankNumber = 7;
	++nBankNumber;

	if ( nBankOffset < 2 || nBankOffset > 1024 )
		nBankOffset = 1024;

	LockDmaMutex();
	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_BANK_INFO;
	KCmdInf.dwCmdParm2 = (nBankNumber * nBankOffset * 1024) >> 2;

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

	UnlockDmaMutex();
	UnlockEpldMutex();
	return 0;
}

/*^^***************************************************************************
 * Description : SDRAM bank count
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_SET_SDRAM_BANK_CONFIG(int nBankConfig)
{
	LldPrint("TSPL_SET_SDRAM_BANK_CONFIG", nBankConfig, 0, 0);
	return SetSDRAMBankConfig(nBankConfig);
}

/*^^***************************************************************************
 * Description : SDRAM bank offset
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig)
{
	LldPrint("TSPL_SET_SDRAM_BANK_OFFSET_CONFIG", nBankConfig, 0, 0);
	return SetSDRAMBankOffsetConfig(nBankConfig);
}


/*^^***************************************************************************
 * Description : Set TSIO direction
 *				
 * Entry : nDirection
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :
 *
 **************************************************************************^^*/
int 	_stdcall    CLldPlayCapt::TSPL_SET_TSIO_DIRECTION(int nDirection)
{
	int	dwStatus;
	int	dwControl = 0;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LockEpldMutex();
	LldPrint("TSPL_SET_TSIO_DIRECTION", nDirection, 0, 0);
//	LldPrint_Trace("TSPL_SET_TSIO_DIRECTION", nDirection, (int)0, (double)0, NULL);

	if ( IsAttachedBdTyp_HaveDmxCmtlTest() )
	{
		//ATSC-M/H
		//if ( nDirection < TSIO_PLAY_WITH_310INPUT || nDirection > TSIO_ASI_LOOPTHRU )
		if ( nDirection < TSIO_PLAY_WITH_310INPUT || nDirection > TSIO_FILE_LOOP_PLAY )
		{
			LldPrint_1("[LLD]Invalid TS IO Direciton", nDirection);
			UnlockEpldMutex();
			return -1;
		}

		if ( nDirection == TSIO_310_LOOPTHRU )
			dwControl = 3;//bit 1:1, bit 0:1
		else if ( nDirection == TSIO_ASI_LOOPTHRU )
			dwControl = 1;//bit 1:0, bit 0:1
		//ATSC-M/H
		else if ( nDirection == TSIO_310_CAPTURE_PLAY )
			dwControl = 7;//bits[2,0]=1,1,1
		else if ( nDirection == TSIO_ASI_CAPTURE_PLAY )
			dwControl = 5;//bits[2,0]=1,0,1
		else if ( nDirection == TSIO_FILE_LOOP_PLAY )
			dwControl = 4;//bits[2,0]=1,0,0

		else
			dwControl = 0;//bit 1:?, bit 0:0, file play mode

		if ( IsAttachedBdTyp_NewAddrMap() )
		{
			if ( (TSPL_nModulatorType == TVB380_ISDBT_13_MODE || TSPL_nModulatorType == TVB380_ISDBS_MODE) && CurrentTSSrc == 3)	//2011/6/15 ISDB-S ASI	//2011/5/3
			{
				if ( nDirection == TSIO_310_LOOPTHRU )
					dwControl = 7;//bits[2,0]=1,1,1
				else if ( nDirection == TSIO_ASI_LOOPTHRU )
					dwControl = 5;//bits[2,0]=1,0,1
				else
					dwControl = 4;//bits[2,0]=1,0,0
			}
			dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
			if(IsAttachedBdTyp_599() || IsAttachedBdTyp_598())
			{
				int	restamp_out_bit;
				int   bit_mask;
				if(IsAttachedBdTyp_598() && (TSPL_nModulatorType == TVB380_MULTI_VSB_MODE || TSPL_nModulatorType == TVB380_MULTI_QAMB_MODE || TSPL_nModulatorType == TVB380_MULTI_DVBT_MODE))
				{
					restamp_out_bit = 26;
					bit_mask = ~((0x1 << 26) + (0x7 << 4));
				}
				else
				{
					restamp_out_bit = 15;
					bit_mask = ~((0x1 << 15) + (0x7 << 4));
				}
/*int OpMode;
				if(dwControl == 4)
					OpMode = 1;
				else if(dwControl == 0)
					OpMode = 0;
				else 
					OpMode = 2;*/
				if(IsAttachedBdTyp_599_598_ver2())
				{
					dwStatus = (1 << restamp_out_bit) + (dwControl<<4) + (dwStatus & bit_mask);// + OpMode;
				}
				else
				{
					if((dwControl == 4 || dwControl == 0))
						dwStatus = (1 << restamp_out_bit) + (dwControl<<4) + (dwStatus & bit_mask);// + OpMode;
					else
						dwStatus = (0 << restamp_out_bit) + (dwControl<<4) + (dwStatus & bit_mask);// + OpMode;
				}
			}
			else
				dwStatus = (dwControl<<4) + (dwStatus&0x303);
			WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);

			dwStatus = 1;
		}
		else
		{
			dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_TS_IO_CNTL, dwControl);
		}

		if (dwStatus != 0)
			CurrentTSSrc = nDirection;
		if(dwControl == 0 || dwControl == 4)
			TSPL_nMemoryCtrl_mode = 0;
		else
		{
			if(TSPL_nModulatorType == TVB380_ISDBT_13_MODE || TSPL_nModulatorType == TVB380_ISDBS_MODE ||
				TSPL_nModulatorType == TVB380_ATSC_MH_MODE || TSPL_nModulatorType == TVB380_DVBT2_MODE ||
				TSPL_nModulatorType == TVB380_DVBC2_MODE)
			{
				TSPL_nMemoryCtrl_mode = 2;
			}
			else
			{
				TSPL_nMemoryCtrl_mode = 1;
			}
			
		}
		if(IsAttachedBdTyp_599() || IsAttachedBdTyp_598())
			SetSDRAMBankOffsetConfig(TSPL_nBankOffset);
		else
			TSPL_nMemoryCtrl_mode = 0;
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
	}
	UnlockEpldMutex();
	return 0;
}

/*^^***************************************************************************
 * Description : Get the current TSIO status
 *				
 * Entry : nDirection
 *
 * Return: TS I/O status
 *
 * Notes :
 *
 * parameters: 
 *              - option: reset [bit 0]
                                type [bit 8 - 15], bit 8: Waiting, bit 9: Play or Record, bit 10: ASI/310M IN
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_GET_TSIO_STATUS(int option)
{
	int	dwStatus;
	int	type;
	static int nCC_old = 0;
	int nRet = 0;
	if(fDMA_Busy == 1)
		return -1;

	if((option & 0x1) > 0)
	{
		WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)0x80);
		dwStatus = 0;
		nCC_old = 0;
		WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)dwStatus);
		return 0;
	}
	type = (option >> 8) & 0xFF;
	dwStatus = WDM_Read_TSP(TSP_MEM_ADDR_STATUS);
	if(nCC_old != (dwStatus & 0xF))
	{
		//OutputDebugString("===ERROR Continity Counter\n");
		LldPrint_1("[Board Status] ERROR Continity Counter, CC ", nCC_old);
		nCC_old = (dwStatus & 0xF);
		nRet = nRet + 1;
	}

	nCC_old++;
	if(nCC_old > 15)
		nCC_old = 0;
	
	if(type == 0x5 || type == 0x1)
	{
		if((dwStatus & 0x40) > 0)
		{
			//OutputDebugString("===DAC PLL unlock\n");
			LldPrint_1("[Board Status] DAC PLL unlock, type", type);
			nRet = nRet + 0x40;
		}
		if(IsAttachedBdTyp_UseVcoPllN_Hmc833() == 0)
		{
			if((dwStatus & 0x80) > 0)
			{
			//	OutputDebugString("===TS IN Sync Loss\n");
				LldPrint_1("[Board Status] Up-converter PLL1 unlock, type", type);
				nRet = nRet + 0x80;
			}
			if((dwStatus & 0x100) > 0)
			{
				//OutputDebugString("===Up-converter PLL2 unlock\n");
				LldPrint_1("[Board Status] Up-converter PLL2 unlock, type", type);
				nRet = nRet + 0x100;
			}
		}
		else
		{
			if(TSPL_HMC833_status == 1)
			{
				LldPrint_1("[Board Status] Up-converter PLL2 unlock, type", type);
				nRet = nRet + 0x100;
			}
		}
	}
	else if(type == 0x2 || type == 0x6)
	{
		if((dwStatus & 0x10) > 0)
		{
			//OutputDebugString("===Playout Sync Loss\n");
			LldPrint_1("[Board Status] Playout Sync Loss, type", type);
			nRet = nRet + 0x10;
		}
		//if((dwStatus & 0x20) > 0)
		//{
		//	OutputDebugString("===TS IN Sync Loss\n");
		//	LldPrint_1("[Board Status] TS IN Sync Loss, type", type);
		//}
		if((dwStatus & 0x40) > 0)
		{
			//OutputDebugString("===DAC PLL unlock\n");
			LldPrint_1("[Board Status] DAC PLL unlock, type", type);
			nRet = nRet + 0x40;
		}
		if(IsAttachedBdTyp_UseVcoPllN_Hmc833() == 0)
		{
			if((dwStatus & 0x80) > 0)
			{
			//	OutputDebugString("===TS IN Sync Loss\n");
				LldPrint_1("[Board Status] Up-converter PLL1 unlock, type", type);
				nRet = nRet + 0x80;
			}
			if((dwStatus & 0x100) > 0)
			{
				//OutputDebugString("===Up-converter PLL2 unlock\n");
				LldPrint_1("[Board Status] Up-converter PLL2 unlock, type", type);
				nRet = nRet + 0x100;
			}
		}
		else
		{
			if(TSPL_HMC833_status == 1)
			{
				LldPrint_1("[Board Status] Up-converter PLL2 unlock, type", type);
				nRet = nRet + 0x100;
			}
		}
		if((dwStatus & 0x200) > 0)
		{
			//OutputDebugString("===Rate Adapter block TS align Error\n");
			LldPrint_1("[Board Status] Rate Adapter block TS align Error, type", type);
			nRet = nRet + 0x200;
		}
		if((dwStatus & 0x400) > 0)
		{
			//OutputDebugString("===Modulator Output Buffer Empty\n");
			LldPrint_1("[Board Status] Modulator Output Buffer Empty, type", type);
			nRet = nRet + 0x400;
		}
		if((dwStatus & 0x800) > 0)
		{
			//OutputDebugString("===Modulator Output Buffer Full\n");
			LldPrint_1("[Board Status] Modulator Output Buffer Full, type", type);
			nRet = nRet + 0x800;
		}

	}

	LldPrint_Trace("TSPL_GET_TSIO_STATUS", dwStatus, (int)0, (double)0, NULL);
	return nRet;
}

/*^^***************************************************************************
 * Description : Get the current TSIO direction
 *				
 * Entry : 
 *
 * Return: TS I/O Direction
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_GET_TSIO_DIRECTION(void)
{
	if (CurrentTSSrc == -1)		// Set default direction
		TSPL_SET_TSIO_DIRECTION(0);
	LldPrint_Trace("TSPL_GET_TSIO_DIRECTION", CurrentTSSrc, (int)0, (double)0, NULL);
	return CurrentTSSrc;
}

/*^^***************************************************************************
 * Description : Get the current SDRAM bank group
 *				
 * Entry : 
 *
 * Return: current bank group to be ready 
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_GET_CUR_BANK_GROUP(void)
{
	DWORD	dwBank;

#if	0
	dwBank = WDM_ReadFromSDRAM() & 0xe0000000;
//	if (dwBank == 0xffff)
//		return 0xffff;	// fail alarm
	dwBank >>= 29;
#else
	dwBank = WDM_ReadFromSDRAM() & 0x38;
//	if (dwBank == 0xffff)
//		return 0xffff;	// fail alarm
	dwBank >>= 3;
#endif
	LldPrint_Trace("TSPL_GET_CUR_BANK_GROUP", (int)(dwBank < 4? 0 : 1), (int)0, (double)0, NULL);
	return (dwBank < 4? 0 : 1);
}

/*^^***************************************************************************
 * Description : Set the current SDRAM bank group #
 *				
 * Entry : nBank
 *
 * Return: 
 *
 * Notes : No implemeted function
 *
 **************************************************************************^^*/
int 	_stdcall    CLldPlayCapt::TSPL_SET_BANK_COUNTER(int nBank)
{
	int	dwStatus;

	LldPrint_FCall("TSPL_SET_BANK_COUNTER", nBank, 0);
//	LldPrint_Trace("TSPL_SET_BANK_COUNTER", nBank, (int)0, (double)0, NULL);

	if (nBank == 0)
		dwStatus = WDM_WR2SDRAM(CARD_RESET_READ_BANK_000);
	else
		dwStatus = WDM_WR2SDRAM(CARD_SET_READ_BANK_100);

	return 0;
}

/*^^***************************************************************************
 * Description : Set TS I/O operation mode
 *				
 * Entry : nBank
 *
 * Return: 
 *
 * Notes : No implemeted function
 *
 **************************************************************************^^*/
int 	_stdcall    CLldPlayCapt::TSPL_SET_SDCON_MODE(int nMode)
{
	int	dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LockEpldMutex();
	LldPrint("TSPL_SET_SDCON_MODE", nMode, 0, 0);
//	LldPrint_Trace("TSPL_SET_SDCON_MODE", nMode, (int)0, (double)0, NULL);

	//I/Q PLAY/CAPTURE
	if ( nMode == TSPL_SDCON_IQ_PLAY_MODE || nMode == TSPL_SDCON_IQ_CAPTURE_MODE || nMode == TSPL_SDCON_IQ_NONE )
	{
		if ( nMode == TSPL_SDCON_IQ_PLAY_MODE )
		{
			dwStatus = 2;
		}
		else if ( nMode == TSPL_SDCON_IQ_CAPTURE_MODE )
		{
			dwStatus = 1;
		}
		else
		{
			dwStatus = 0;
		}
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_IQ_PLAY_CAPTURE_CTRL, (unsigned long)dwStatus);
		UnlockEpldMutex();
		return 0;
	}
	
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		if (nMode == TSPL_SDCON_PLAY_MODE)
			dwStatus = PCI590S_OPERATION_MODE_PLAY;
		else if (nMode == TSPL_SDCON_CAPTURE_MODE)
			dwStatus = PCI590S_OPERATION_MODE_CAPTURE;
		else
			dwStatus = PCI590S_OPERATION_MODE_STOP;
		WDM_WR2SDRAM_ADDR(PCI590S_REG_OPERATION_MODE, (unsigned long)dwStatus);
		UnlockEpldMutex();
		return 0;
	}
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);

		if (nMode == TSPL_SDCON_PLAY_MODE)
			dwStatus = (((dwStatus>>2)&0x3FFFFFFF)<<2) + PCI593_OPERATION_MODE_PLAY;
		else if (nMode == TSPL_SDCON_CAPTURE_MODE)
			dwStatus = (((dwStatus>>2)&0x3FFFFFFF)<<2) + PCI593_OPERATION_MODE_CAPTURE;
		else
			dwStatus = (((dwStatus>>2)&0x3FFFFFFF)<<2) + PCI593_OPERATION_MODE_STOP;

		WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);

		UnlockEpldMutex();
		return 0;
	}

	if (nMode == TSPL_SDCON_PLAY_MODE)
		dwStatus = WDM_WR2SDRAM(SDCON_PLAY_MODE);
	else if (nMode == TSPL_SDCON_CAPTURE_MODE)
		dwStatus = WDM_WR2SDRAM(SDCON_CAPTURE_MODE);
	else if (nMode == TSPL_SDCON_DELAY_MODE)
		dwStatus = WDM_WR2SDRAM(SDCON_DELAY_MODE);
	else
		dwStatus = WDM_WR2SDRAM(SDCON_LOOPTHRU_MODE);

	UnlockEpldMutex();
	return dwStatus == 0? 0 : 0;
}

// TSE110
/*^^***************************************************************************
 * Description : EIT/STI Sync. search
 *				
 * Entry : mode(0=G.703 Sync, 1=G.704 Sync, 3=G.703 STI Sync)
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
#ifdef WIN32
extern		HANDLE	hDmaMutex;
#endif
int	_stdcall CLldPlayCapt::TSE110_GET_SYNC_STATUS(int mode)
{
	DWORD	dwValue;

	LldPrint_Trace("TSE110_GET_SYNC_STATUS", mode, (int)0, (double)0, NULL);
//#ifdef WIN32
	//LockDmaMutex();

//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return -1;
//	}
//#endif
	dwValue  = WDM_Read_TSP(TSE_BOARD_STATUS_ADDR);
	if ( mode == 0 ) 
		dwValue = (dwValue>>6) & 0x01;
	else if ( mode == 1 )
		dwValue = (dwValue>>7) & 0x01;
	else if ( mode == 2 )
		dwValue = (dwValue>>8) & 0x01;
	else 
		dwValue = 0;
//#ifdef WIN32
//	UnlockDmaMutex();
//#endif
	return dwValue;
}

/*^^***************************************************************************
 * Description : EIT/STI Siganl status(Lock/Unlock)
 *				
 * Entry : port(0=Input A Port, 1=Input B Port)
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSE110_GET_SIGNAL_STATUS(int port)
{
	DWORD	dwValue;
	
	LldPrint_Trace("TSE110_GET_SIGNAL_STATUS", port, (int)0, (double)0, NULL);
//#ifdef WIN32
//	LockDmaMutex();

//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return -1;
//	}
//#endif
	dwValue = WDM_Read_TSP(TSE_ETI_STATUS_ADDR);
	if ( port == 0 )
		dwValue = dwValue & 0x01;
	else if ( port == 1 )
		dwValue = (dwValue>>1) & 0x01;
	else
	{
//#ifdef WIN32
//		UnlockDmaMutex();
//#endif
		return 0;
	}
//#ifdef WIN32
//	UnlockDmaMutex();
//#endif

	return dwValue ? 0 : 1;
}

/*^^***************************************************************************
 * Description : G.703, NA format
 *				
 * Entry : 
 *
 * Return: 0(FORMAT_5592), 1(FORMAT_5376)
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSE110_GET_SYNC_FORMAT(void)
{
	LldPrint_Trace("TSE110_GET_SYNC_FORMAT", 0, (int)0, (double)0, NULL);
	return TL_nLIFormat;
}

/*^^***************************************************************************
 * Description : Configuration TSE110 I/O
 *				
 * Entry : port_a_mode(0), port_b_mode(0), tx_clock, input_port
 *			ouput_a_mode, ouput_b_mode
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSE110_SET_CONFIG(int port_a_mode,
														 int port_b_mode,
														 int tx_clock,
														 int input_port,
														 int output_a_mode,
														 int output_b_mode)
{
	DWORD		dwValue;
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;

	LldPrint_Trace("TSE110_SET_CONFIG", 0, (int)0, (double)0, NULL);
	dwValue = (port_b_mode << 9);
	dwValue += (port_a_mode << 7);
	dwValue += (tx_clock << 5);
	dwValue += (input_port << 4);
	dwValue += (output_b_mode << 2);
	dwValue += output_a_mode;

	LockDmaMutex();

	KCmdInf.dwCmdParm1 = TSE_ETI_COMMAND_ADDR;
	KCmdInf.dwCmdParm2 = dwValue;

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	TLV_DeviceIoControl(hDevice,
		IOCTL_WRITE_TO_MEMORY,
		&KCmdInf,
		sizeof(KCmdInf),
		NULL, 
		0, 
		&dwRet,
		0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}
	else
	{
		TLV_DeviceIoControl(hDevice,
			IOCTL_WRITE_TO_MEMORY,
			&KCmdInf,
			sizeof(KCmdInf),
			NULL, 
			0, 
			&dwRet,
			0);
	}
#endif

	UnlockDmaMutex();

	return dwValue;
}

//=================================================================	
//
#define	ETI_MULTIFRAME_SIZE		6144
#define	ETI_DEINTLV_SIZE		5760
#define	ETI_LI_SIZE_5592		5592
#define	ETI_LI_SIZE_5376		5376
#define	ARRAY_I_SIZE			24
#define	ARRAY_J_SIZE			240

#define	FORMAT_5592		0
#define	FORMAT_5376		1
#define	FORMAT_OFFSET	30
#define	LIFORMAT(x)		(((*(x + FORMAT_OFFSET)) >> 1) & 0x1)

#define	ETI_SYMSIZE		8
#define	ETI_GENPOLY		0x187
#define	ETI_FCS			120
#define	ETI_PRIM		1
#define	ETI_RS_BLOCK	240
#define	ETI_PAD			(NN - ETI_RS_BLOCK)

/*^^***************************************************************************
 * Description : EIT/STI Sync. search
 *				
 * Entry : mode(0=G.703 ETI, 1=G.704, 2=G.703 STI), type(0=G.704 Sync, 1=ETI NA Sync)
 *			szBuf(input source, up to 1M bytes), position(found sync position)
 * Return: -1(fail), searched sync. position
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_GET_SYNC_POSITION(int mode, int type, unsigned char *szBuf, 
															  int nlen, int nlen_srch, int nlen_step)
{
	int		i, j, mblkcnt = 0, sblkcnt = 0;
	bool	err;
	//char text[255];

	LldPrint_Trace("TSPL_GET_SYNC_POSITION", mode, (int)type, (double)0, NULL);
	switch (mode)
	{
		case 0:
			if ( type != 0 )
				return 0;

			if (nlen < nlen_srch)
				return -1;
			
			for (i = 0; i < nlen_srch; i++)
			{
				 //fixed - check 0xF0 
				if ( szBuf[i] == 0xFF || szBuf[i] == 0xF0 ) 
				{
					err = FALSE;
					for (j = 0; j < nlen_step; j++) 
					{
						//fixed 
						if ( (i+3 >= nlen) 
							|| ((j*6144*2)+i+3 < nlen && (szBuf[(j*6144*2)+i+1] != 0x07 || szBuf[(j*6144*2)+i+2] != 0x3A || szBuf[(j*6144*2)+i+3] != 0xB6)) )
						{
							err = TRUE;
							break;
						}
					}

					if (err == FALSE) 
					{
						return i;
					}
				}
			}
			break;
			
		case 1:
			{
				if (nlen < nlen_srch)
						return -1;

				if ( type == 0 )
				{
					for (i = 0; i < nlen_srch; i++)
					{
						if ((szBuf[i] & G704_SYNC_MASK) == G704_SYNC) 
						{
							err = FALSE;
							for (j = 0; j < nlen_step; j++) 
							{
								if ( ((2*j+1)*G704_FRAME_SIZE+i >= nlen) 
									|| ((szBuf[(2*j)*G704_FRAME_SIZE+i] & G704_SYNC_MASK) != G704_SYNC)
									|| (((szBuf[(2*j+1)*G704_FRAME_SIZE+i] >> 6) & 0x01) != 1))
								{
									err = TRUE;
									break;
								}
							}

							if (err == FALSE) 
							{
								return i;
							}
						}
						else if (((szBuf[i] >> 6) & 0x01) == 1)
						{
							err = FALSE;
							for (j = 0; j < nlen_step; j++) 
							{
								if ( ((2*j+1)*G704_FRAME_SIZE+i >= nlen) 
									|| (((szBuf[(2*j)*G704_FRAME_SIZE+i] >> 6) & 0x01) != 1)
									|| ((szBuf[(2*j+1)*G704_FRAME_SIZE+i] & G704_SYNC_MASK) != G704_SYNC))
								{
									err = TRUE;
									break;
								}
							}

							if (err == FALSE) 
							{
								return i;
							}
						}
					}
				}
				else
				{
					for (i = 0; i < nlen_srch; i += G704_FRAME_SIZE)
					{
						if ( TS1_N+i < nlen && TS1+i < nlen && TS1_NN+i  < nlen 
							&& (BCNT(szBuf[TS1_N+i]) == INCB(BCNT(szBuf[TS1+i]))) 
							&& (BCNT(szBuf[TS1_NN+i]) == INCB(BCNT(szBuf[TS1_N+i]))))
						{
							err = FALSE;

							mblkcnt = BCNT(szBuf[TS1+i]);
							for (j = 0; j < nlen_step; j++)
							{
								if ( (TS1_N+i)+G704_BLOCK_SIZE*j < nlen  
									&& BCNT(szBuf[(TS1_N+i)+G704_BLOCK_SIZE*j]) == INCB(mblkcnt) )
								{
									mblkcnt = BCNT(szBuf[(TS1+i)+G704_BLOCK_SIZE*j]);
									sblkcnt	= SCNT(szBuf[(TS1+i)+G704_BLOCK_SIZE*j]);
									if ( sblkcnt != 0 || mblkcnt != 0 )
									{
										mblkcnt = BCNT(szBuf[(TS1_N+i)+G704_BLOCK_SIZE*j]);

										err = TRUE;
										continue;
									}
									else
									{
										err = FALSE;
										break;
									}
								}
								else
								{
									err = TRUE;
									continue;
								}
							}

							if (err == FALSE) 
							{
								return i+G704_BLOCK_SIZE*j;
							}
						}
					}
				}
			}
			break;

		case 2:
			break;
		default:
			break;
	}

	return -1;
}

/*^^***************************************************************************
 * Description : Do de-interleaving for NA to NI conversion
 *				
 * Entry : szSrc = G.704 and ETI NA sync searched source buffer, 6144 bytes size
 *			szDest[OUT] = de-interleaved output buffer, 5760 bytes size
 * Return: 0
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_DO_DEINTERLEAVING(unsigned char *szSrc, unsigned char *szDest)
{
	int	i, j, p=0, q, divI, modI;

	LldPrint_FCall("TSPL_DO_DEINTERLEAVING", 0, 0);
//	LldPrint_Trace("TSPL_DO_DEINTERLEAVING", 0, (int)0, (double)0, NULL);
	if ( szSrc == NULL || szDest == NULL )
		return -1;

	memset(szDest, 0x00, ETI_DEINTLV_SIZE);
	for (q = 0; q < ETI_MULTIFRAME_SIZE; q++)
	{
		if (((q % G704_FRAME_SIZE) != 0) && ((q % G704_FRAME_SIZE) != 16)) 
		{
			divI = p / 1920;
			modI = p % 8;
			i = (divI << 3) + modI;
			j = (p -(1920 * divI) - modI) / 8;
			*(szDest + ((i * ARRAY_J_SIZE) + j)) = *(szSrc + q);
			p++;
		}
	}

	return 0;
}

/*^^***************************************************************************
 * Description : Do rs-decoding for NA to NI conversion
 *				
 * Entry :szSrc = de-interleaved source buffer, 5760 bytes size
 *		szDest[OUT] = rs-decoded output buffer, 5376 or 5592 bytes size
 * Return: 0
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TSPL_DO_RS_DECODING(unsigned char *szSrc, 
														   unsigned char *szDest, 
														   int *format,
														   int *err_blk_cnt,
														   int *recovered_err_cnt,
														   int bypass)
{
	int		i, j, r, s;
	int		rc, sc;

	void	*handle;
	struct	rs *rs;
	DTYPE	tblock[NN];
	int		derrlocs[NA5376_ROOTS];
	int		derrors;
	int		erasures = 0;
	int		total_err = 0, decode_err = 0;

	int		LI_format=-1;	// 0 : 5592,  1 : 5376
	int		rootsize;		// 5592(format==0) or 5376(format==1)

	LldPrint_FCall("TSPL_DO_RS_DECODING", 0, 0);
//	LldPrint_Trace("TSPL_DO_RS_DECODING", 0, (int)0, (double)0, NULL);

	/* find format */
	LI_format = LIFORMAT(szSrc);
	TL_nLIFormat = LI_format;

	/* set output size */
	if ( LI_format == FORMAT_5592 )
		rootsize = NA5592_ROOTS;
	else
		rootsize = NA5376_ROOTS;

	*format = LI_format;
	memset(szDest, 0x00, ETI_MULTIFRAME_SIZE);

	/* rs decoding */
	if ( bypass == 0 )
	{
		/* init decoder handle */
		if((handle = init_rs_char(ETI_SYMSIZE, ETI_GENPOLY, ETI_FCS, ETI_PRIM, rootsize, 0)) == NULL)
		{
			printf("init_rs_char failed!\n");
			return -1;
		}
		
		rs = (struct rs *)handle;
		for (i = 0; i < ARRAY_I_SIZE ; i++) 
		{
			memset(tblock, 0, NN);	// pre-padding zeros
			memcpy((&tblock[0] + ETI_PAD), (szSrc + (i * ARRAY_J_SIZE)), ETI_RS_BLOCK);
			if (LI_format == FORMAT_5592)
				derrors = decode_rs_5592(rs,tblock,derrlocs,erasures);
			else
				derrors = decode_rs_5376(rs,tblock,derrlocs,erasures);

			if (derrors == -1) 
				decode_err++;
			else
				total_err += derrors;

			memcpy((szSrc + (i * ARRAY_J_SIZE)), (&tblock[0] + ETI_PAD), ETI_RS_BLOCK);
		}
		free_rs_char(handle);
	}

	/* extract ETI-LI format */
	rc = sc = 0;
	// 5592 case
	if (LI_format == FORMAT_5592) 
	{
		for (i = 0; i < ARRAY_I_SIZE ; i++) 
		{
			for (j = 0; j < ARRAY_J_SIZE; j++) 
			{
				if (((i % 8) < 2) && ((j % 30) != 0) && (j <= 234)) 
				{
					r = (i / 8) * 1864 + (i % 2) * 227 - (j / 30) + (j - 1);
					if (r >= ETI_LI_SIZE_5592) 
					{
						printf("buffer ptr error[%d] %d %d %d\n", LI_format, i, j, r);							
					}
					else 
					{
							*(szDest + r) = *(szSrc + (i * ARRAY_J_SIZE) + j);
							rc++;
					}
				}
				else if (((i % 8) >= 2) && (j <= 234)) 
				{
					s = (i / 8) * 1864 + (i % 8) * 235 + (j - 16);
					if (s >= ETI_LI_SIZE_5592) 
					{
						printf("buffer ptr error[%d] %d %d %d\n", LI_format, i, j, s);							
					}
					else 
					{
						*(szDest + s) = *(szSrc + (i * ARRAY_J_SIZE) + j);
						sc++;
					}
				}
				else 
				{
				}
			}
		}
	}
	// 5376 case
	else 
	{
		for (i = 0; i < ARRAY_I_SIZE ; i++) 
		{
			for (j = 0; j < ARRAY_J_SIZE; j++) 
			{
				if (((i % 8) < 2) && ((j % 30) != 0) && (j <= 225)) 
				{
					r = (i / 8) * 1792 + (i % 2) * 218 - (j / 30) + (j - 1);
					if (r >= ETI_LI_SIZE_5376) 
					{
						printf("buffer ptr error[%d] %d %d %d\n", LI_format, i, j, r);							
					}
					else 
					{
						*(szDest + r) = *(szSrc + (i * ARRAY_J_SIZE) + j);
						rc++;
					}
				}
				else if (((i % 8) >= 2) && (j <= 225)) 
				{
					s = (i / 8) * 1792 + (i % 8) * 226 + (j - 16);
					if (s >= ETI_LI_SIZE_5376) 
					{
						printf("buffer ptr error[%d] %d %d %d\n", LI_format, i, j, s);							
					}
					else 
					{
						*(szDest + s) = *(szSrc + (i * ARRAY_J_SIZE) + j);
						sc++;
					}
				}
				else 
				{
				}
			}
		}

	}

	*err_blk_cnt = decode_err;
	*recovered_err_cnt = total_err;

	return 0;
}

/*^^***************************************************************************
 * Description : Convert NA to NI
 *				
 * Entry :szSrc = LI(rs-decoded output) buffer, 5376 or 5592 bytes size
 *		szDest[OUT] = G.703 NI output buffer, 6144 bytes size, format = G.704 LI format
 * Return: 0
 *
 * Notes :
 *
 **************************************************************************^^*/
static int g_FSYNC = 1;
static unsigned char data_fsync[2][4];
int	_stdcall CLldPlayCapt::TSPL_CONVERT_TO_NI(unsigned char  *szSrc, unsigned char *szDest, int format)
{
	int		i, size;
	
	LldPrint_FCall("TSPL_CONVERT_TO_NI", 0, 0);
//	LldPrint_Trace("TSPL_CONVERT_TO_NI", 0, (int)0, (double)0, NULL);

	data_fsync[0][0] = 0xff;
	data_fsync[0][1] = 0x07;
	data_fsync[0][2] = 0x3a;
	data_fsync[0][3] = 0xb6;

	data_fsync[1][0] = 0xff;
	data_fsync[1][1] = 0xf8;
	data_fsync[1][2] = 0xc5;
	data_fsync[1][3] = 0x49;

	if (format == FORMAT_5592)
		size = ETI_LI_SIZE_5592;
	else
		size = ETI_LI_SIZE_5376;

//	printf("convert %d ETI-LI format -> NI\n", size);

	memset(szDest, 0x55, ETI_MULTIFRAME_SIZE);
	
	// 1. insert FSYNC 
	// 2. copy LI data, then stuffing
	
	g_FSYNC = (g_FSYNC+1)%2;
	for (i = 0; i < 4; i++) 
		*(szDest + i) = data_fsync[g_FSYNC][i];

	memcpy((szDest + i), szSrc, size); 
	
	return 0;
}

/*^^***************************************************************************
 * Description : Initialize resources for CIF generation of TDMB
 *				
 * Entry :
 *		
 * Return: 0
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TVB380_SET_MODULATOR_INIT_CIF(void)
{
	LldPrint_FCall("TVB380_SET_MODULATOR_INIT_CIF", 0, 0);
//	LldPrint_Trace("TVB380_SET_MODULATOR_INIT_CIF", 0, (int)0, (double)0, NULL);
#if defined(TSPLLD0431_EXPORTS) || defined(TSE110V1)
#else
	MyFreeFunc();  // Free all allocated memory
	return MyInitFunc();//-1(memory allocation failed) or 0
#endif
	return 0;
}

/*^^***************************************************************************
 * Description : Do CIF generation for TDMB
 *				
 * Entry :szSrc = ETI data
 *		szDest = CIF data
 *		
 * Return: -1(fail), 0(success)
 *
 * Notes :
 *
 **************************************************************************^^*/
int	_stdcall CLldPlayCapt::TVB380_SET_MODULATOR_RUN_CIF(unsigned char  *szSrc, unsigned char *szDest)
{
	LldPrint_FCall("TVB380_SET_MODULATOR_RUN_CIF", 0, 0);
//	LldPrint_Trace("TVB380_SET_MODULATOR_RUN_CIF", 0, (int)0, (double)0, NULL);
#if defined(TSPLLD0431_EXPORTS) || defined(TSE110V1)
	return 0;
#else

	if ( MyDoReadETI(&m_Frame, szSrc) != TRUE)
		return -1;//Fail to read ETI data

	//fixed, mode IV supported
	if(FramePhase == (m_FrameInfo.FP%4)) 
	{
		MyDoEnerDisp(&m_EnerDisp, m_Frame);				// Energy Dispersal
		MyDoConvEnco(&m_ConvEnco, m_EnerDisp);			// Convolutional Encoding
		MyDoTimeInterleav(&m_TimeInt, m_ConvEnco);		// Time Interleaving
		MyDoCif(m_pCif, m_TimeInt);						// Generate Common Interleaved Frame

		memset(WrBuff, 0, 7300);
		WrBuff[0] = 0x07;
		WrBuff[1] = 0x3a;
		WrBuff[2] = 0xb6;	
		WrBuff[3] = (m_FrameInfo.MID & 0x03) | (FramePhase << 4);
		memcpy(&WrBuff[4], &m_ConvEnco.Fic.Data[0], m_ConvEnco.Fic.Length);
		memcpy(&WrBuff[388], m_pCif, 6912);

		FramePhase = (++FramePhase)%4;
	
		memcpy(szDest, WrBuff, 7300);

		return 0;
	}

	return -1;//No frame
#endif
}

//TVB593
int _stdcall CLldPlayCapt::TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate)
{
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint("TSPL_SET_MAX_PLAYRATE", modulator_type, use_max_playrate, 0);
//	LldPrint_Trace("TSPL_SET_MAX_PLAYRATE", modulator_type, (int)use_max_playrate, (double)0, NULL);

	//TVB593
	unsigned long dwStatus;

//#ifdef WIN32
//	LockDmaMutex();
//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return 0;
//	}
//#endif
	if ( IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_MAX_PLAY_RATE);
		dwStatus = (use_max_playrate<<26) + (dwStatus&0x3FFFFFF); 
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, dwStatus);
	}
	else if ( IsAttachedBdTyp_497() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_MAX_PLAY_RATE);
		dwStatus = (((dwStatus>>27)&0x01)<<27) + (use_max_playrate<<26) + (dwStatus&0x3FFFFFF); 
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, dwStatus);
	}
	else
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, use_max_playrate);
	}
//#ifdef WIN32
//	UnlockDmaMutex();
//#endif

	return 0;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CLldPlayCapt	LLDIf;

unsigned long	WDM_Read_TSP(unsigned long dwMemAddr)
{
	return	LLDIf.WDM_Read_TSP(dwMemAddr);
}
int	WDM_WR2SDRAM(unsigned long dwCommand)
{
	return	LLDIf.WDM_WR2SDRAM(dwCommand);
}

/////////////////////////////////////////////////////////////////
int 	TSPL_SET_CONFIG_DOWNLOAD(long addr, unsigned long data)
{
	return	LLDIf.TSPL_SET_CONFIG_DOWNLOAD(addr, data);
}

/////////////////////////////////////////////////////////////////
int	WDM_WR2SDRAM_ADDR(unsigned long dwAddress, unsigned long dwCommand)
{
	return	LLDIf.WDM_WR2SDRAM_ADDR(dwAddress, dwCommand);
}

/////////////////////////////////////////////////////////////////
unsigned long RemapAddress(unsigned long address)
{
	return	LLDIf.RemapAddress(address);
}

/////////////////////////////////////////////////////////////////
int SetSDRAMBankConfig(int nBankConfig)
{
	return	LLDIf.SetSDRAMBankConfig(nBankConfig);
}
int _stdcall ___TSPL_GET_AD9775(long reg)
{
	return	LLDIf.TSPL_GET_AD9775(reg);
}
int _stdcall ___TSPL_SET_AD9775(long reg, long data)
{
	return	LLDIf.TSPL_SET_AD9775(reg, data);
}
/////////////////////////////////////////////////////////////////
int ___read_modulator_option()
{
	return	LLDIf.read_modulator_option();
}
int ___set_sync_modulator_type(int modulator_type)
{
	return	LLDIf.set_sync_modulator_type(modulator_type);
}


/////////////////////////////////////////////////////////////////
int ___get_TSPL_nBoardTypeID(void)
{
	return	LLDIf.TSPL_nBoardTypeID;
}
void ___set_board_location(int val)
{
	LLDIf.board_location = val;
	printf("[usb] : bd-location : [%d]\n", val);
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
int	_stdcall TVB380_close__()
{
#if defined(WIN32)
	return	LLDIf.TVBxxx_CLOSE_BD();
#else
	return	LLDIf.TVBxxx_CLOSE_BD();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_close, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TVB380_open__(int nInitModulatorType, int nInitIF)
{
#if defined(WIN32)
	return	LLDIf.TVB380_open(nInitModulatorType, nInitIF);
#else
	return	LLDIf.TVB380_open(nInitModulatorType, nInitIF);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_open, nInitModulatorType, nInitIF, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
/////////////////////////////////////////////////////////////////
#elif defined(TSPLLD0431_EXPORTS) || defined(TSE110V1)
int	_stdcall TSE110_close__()
{
#if defined(WIN32)
	return	LLDIf.TSE110_close();
#else
	return	LLDIf.TSE110_close();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSE110_close, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSE110_open__()
{
#if defined(WIN32)
	return	LLDIf.TSE110_open();
#else
	return	LLDIf.TSE110_open();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSE110_open, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
#endif
/////////////////////////////////////////////////////////////////
int _stdcall TSPL_GET_BOARD_REV__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_BOARD_REV();
#else
	return	LLDIf.TSPL_GET_BOARD_REV();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_BOARD_REV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_BOARD_ID__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_BOARD_ID();
#else
	return	LLDIf.TSPL_GET_BOARD_ID();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_BOARD_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_AUTHORIZATION__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_AUTHORIZATION();
#else
	return	LLDIf.TSPL_GET_AUTHORIZATION();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_AUTHORIZATION, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_FPGA_INFO__(int info)
{
#if defined(WIN32)
	if(info == 100)
		return LLDIf.ScanTAT4710();
	else
		return	LLDIf.TSPL_GET_FPGA_INFO(info);
#else
	return	LLDIf.TSPL_GET_FPGA_INFO(info);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_FPGA_INFO, info, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_ENCRYPTED_SN__(int type, char* sn)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_ENCRYPTED_SN(type, sn);
#else
	return	LLDIf.TSPL_GET_ENCRYPTED_SN(type, sn);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_ENCRYPTED_SN, type, (long)sn, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_WRITE_CONTROL_REG__(int Is_PCI_Control, unsigned long address, unsigned long dwData)
{
#if defined(WIN32)
	return	LLDIf.TSPL_WRITE_CONTROL_REG(Is_PCI_Control, address, dwData);
#else
	return	LLDIf.TSPL_WRITE_CONTROL_REG(Is_PCI_Control, address, dwData);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_WRITE_CONTROL_REG, Is_PCI_Control, address, dwData, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
unsigned long _stdcall TSPL_READ_CONTROL_REG__(int Is_PCI_Control, unsigned long address)
{
#if defined(WIN32)
	return	LLDIf.TSPL_READ_CONTROL_REG(Is_PCI_Control, address);
#else
	return	LLDIf.TSPL_READ_CONTROL_REG(Is_PCI_Control, address);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_READ_CONTROL_REG, Is_PCI_Control, address, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_LAST_ERROR__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_LAST_ERROR();
#else
	return	LLDIf.TSPL_GET_LAST_ERROR();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_LAST_ERROR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_BOARD_CONFIG_STATUS__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_BOARD_CONFIG_STATUS();
#else
	return	LLDIf.TSPL_GET_BOARD_CONFIG_STATUS();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_BOARD_CONFIG_STATUS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
#if defined(WIN32) || defined(TVB595V1)
int _stdcall TSPL_SET_BOARD_LED_STATUS__(int status_LED, int fault_LED)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_BOARD_LED_STATUS(status_LED, fault_LED);
#else
	return	LLDIf.TSPL_SET_BOARD_LED_STATUS(status_LED, fault_LED);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_BOARD_LED_STATUS, status_LED, fault_LED, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_REG_EVENT__(void* pvoid)
{
#if defined(WIN32)
	return	LLDIf.TSPL_REG_EVENT(pvoid);
#else
	return	LLDIf.TSPL_REG_EVENT(pvoid);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_REG_EVENT, (long)pvoid, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_EVENT__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_EVENT();
#else
	return	LLDIf.TSPL_SET_EVENT();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_EVENT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_REG_COMPLETE_EVENT__(void* pvoid)
{
#if defined(WIN32)
	return	LLDIf.TSPL_REG_COMPLETE_EVENT(pvoid);
#else
	return	LLDIf.TSPL_REG_COMPLETE_EVENT(pvoid);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_REG_COMPLETE_EVENT, (long)pvoid, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_COMPLETE_EVENT__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_COMPLETE_EVENT();
#else
	return	LLDIf.TSPL_SET_COMPLETE_EVENT();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_COMPLETE_EVENT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
#endif
#if TVB597A_STANDALONE
int _stdcall TVB597F_SET_FLASH__(char *szFile)
{
	return	0;
}
#else
int _stdcall TVB597F_SET_FLASH__(char *szFile)
{
	return	0;
}
#endif

/////////////////////////////////////////////////////////////////
#ifdef WIN32
int _stdcall TVB380_IS_ENABLED_TYPE__(long modulator_type)
{
	return	LLDIf.TVB380_IS_ENABLED_TYPE(modulator_type);
}
#else
int TVB380_IS_ENABLED_TYPE__(long modulator_type)
{
	return	LLDIf.TVB380_IS_ENABLED_TYPE(modulator_type);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_IS_ENABLED_TYPE, modulator_type, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
}
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int _stdcall TSPL_GET_DMA_STATUS__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_DMA_STATUS();
#else
	return	LLDIf.TSPL_GET_DMA_STATUS();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_DMA_STATUS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_PUT_DATA__(DWORD dwDMASize,DWORD *dest)
{
#if defined(WIN32)
	return	LLDIf.TSPL_PUT_DATA(dwDMASize, dest);
#else
	return	LLDIf.TSPL_PUT_DATA(dwDMASize, dest);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_PUT_DATA, dwDMASize, (long)dest, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
void* _stdcall TSPL_GET_DATA__(long dwDMASize)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_DATA(dwDMASize);
#else
	return	LLDIf.TSPL_GET_DATA(dwDMASize);
//	return	_tvbxxx_msg_snd_get_sts_ptr(MSG_tvbxxx_TSPL_GET_DATA, dwDMASize, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_SET_FIFO_CONTROL__(int nDMADirection, int nDMASize)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_FIFO_CONTROL(nDMADirection, nDMASize);
#else
	return	LLDIf.TSPL_SET_FIFO_CONTROL(nDMADirection, nDMASize);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_FIFO_CONTROL, nDMADirection, nDMASize, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_GET_FIFO_CONTROL__(int nDMADirection, int nDMASize)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_FIFO_CONTROL(nDMADirection, nDMASize);
#else
	return	LLDIf.TSPL_GET_FIFO_CONTROL(nDMADirection, nDMASize);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_FIFO_CONTROL, nDMADirection, nDMASize, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
void* _stdcall TSPL_READ_BLOCK__(long dwDMASize)
{
#if defined(WIN32)
	return	LLDIf.TSPL_READ_BLOCK(dwDMASize);
#else
	return	LLDIf.TSPL_READ_BLOCK(dwDMASize);
//	return	_tvbxxx_msg_snd_get_sts_ptr(MSG_tvbxxx_TSPL_READ_BLOCK, dwDMASize, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
void	_stdcall TSPL_WRITE_BLOCK_TEST__(DWORD *pdwSrcBuff, unsigned long dwBuffSize, DWORD *dest)
{
#if defined(WIN32)
	LLDIf.TSPL_WRITE_BLOCK_TEST(pdwSrcBuff, dwBuffSize, dest);
#else
	LLDIf.TSPL_WRITE_BLOCK_TEST(pdwSrcBuff, dwBuffSize, dest);
//	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_WRITE_BLOCK_TEST, (long)pdwSrcBuff, dwBuffSize, (long)dest, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
#ifdef WIN32
int _stdcall TSPL_WRITE_BLOCK__(DWORD dwBuffSize,DWORD *dest)
{
	return	LLDIf.TSPL_WRITE_BLOCK(dwBuffSize, dest);
}
#else
int _stdcall TSPL_WRITE_BLOCK__(DWORD *pdwSrcBuff, ULONG dwBuffSize, DWORD *dest)
{
	return	LLDIf.TSPL_WRITE_BLOCK(pdwSrcBuff, dwBuffSize, dest);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_WRITE_BLOCK, (long)pdwSrcBuff, dwBuffSize, (long)dest, 0, 0, 0, 0, 0, 0, 0.);
}
#endif
void* _stdcall TSPL_GET_DMA_ADDR__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_DMA_ADDR();
#else
	return	LLDIf.TSPL_GET_DMA_ADDR();
//	return	_tvbxxx_msg_snd_get_sts_ptr(MSG_tvbxxx_TSPL_GET_DMA_ADDR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_GET_DMA_REG_INFO__(int addr)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_DMA_REG_INFO(addr);
#else
	return	LLDIf.TSPL_GET_DMA_REG_INFO(addr);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_DMA_REG_INFO, addr, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_SET_DMA_REG_INFO__(unsigned char nOffset, DWORD dwData)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_DMA_REG_INFO(nOffset, dwData);
#else
	return	LLDIf.TSPL_SET_DMA_REG_INFO(nOffset, dwData);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_DMA_REG_INFO, nOffset, dwData, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int TSPL_SET_DEMUX_CONTROL_TEST__(int nState)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_DEMUX_CONTROL_TEST(nState);
#else
	return	LLDIf.TSPL_SET_DEMUX_CONTROL_TEST(nState);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_DEMUX_CONTROL_TEST, nState, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_SET_SDRAM_BANK_INFO__(int nBankNumber, int nBankOffset)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_SDRAM_BANK_INFO(nBankNumber, nBankOffset);
#else
	return	LLDIf.TSPL_SET_SDRAM_BANK_INFO(nBankNumber, nBankOffset);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_SDRAM_BANK_INFO, nBankNumber, nBankOffset, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_SET_SDRAM_BANK_CONFIG__(int nBankConfig)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_SDRAM_BANK_CONFIG(nBankConfig);
#else
	return	LLDIf.TSPL_SET_SDRAM_BANK_CONFIG(nBankConfig);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_SDRAM_BANK_CONFIG, nBankConfig, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_SET_SDRAM_BANK_OFFSET_CONFIG__(int nBankConfig)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(nBankConfig);
#else
	return	LLDIf.TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(nBankConfig);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_SDRAM_BANK_OFFSET_CONFIG, nBankConfig, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_SEL_DDSCLOCK_INC__(long play_freq_in_herz)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SEL_DDSCLOCK_INC(play_freq_in_herz);
#else
	return	LLDIf.TSPL_SEL_DDSCLOCK_INC(play_freq_in_herz);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC, play_freq_in_herz, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_SET_PLAY_RATE__(long play_freq_in_herz, long nOutputClockSource)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_PLAY_RATE(play_freq_in_herz, nOutputClockSource);
#else
	return	LLDIf.TSPL_SET_PLAY_RATE(play_freq_in_herz, nOutputClockSource);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_PLAY_RATE, play_freq_in_herz, nOutputClockSource, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int 	_stdcall    TSPL_SET_TSIO_DIRECTION__(int nDirection)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_TSIO_DIRECTION(nDirection);
#else
	return	LLDIf.TSPL_SET_TSIO_DIRECTION(nDirection);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_TSIO_DIRECTION, nDirection, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_GET_TSIO_STATUS__(int option)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_TSIO_STATUS(option);
#else
	return	LLDIf.TSPL_GET_TSIO_STATUS(option);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_GET_TSIO_STATUS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_GET_TSIO_DIRECTION__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_TSIO_DIRECTION();
#else
	return	LLDIf.TSPL_GET_TSIO_DIRECTION();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_GET_TSIO_DIRECTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_GET_CUR_BANK_GROUP__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_CUR_BANK_GROUP();
#else
	return	LLDIf.TSPL_GET_CUR_BANK_GROUP();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_GET_CUR_BANK_GROUP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int 	_stdcall    TSPL_SET_BANK_COUNTER__(int nBank)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_BANK_COUNTER(nBank);
#else
	return	LLDIf.TSPL_SET_BANK_COUNTER(nBank);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_BANK_COUNTER, nBank, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int 	_stdcall    TSPL_SET_SDCON_MODE__(int nMode)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_SDCON_MODE(nMode);
#else
	return	LLDIf.TSPL_SET_SDCON_MODE(nMode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_SDCON_MODE, nMode, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int 	_stdcall TSPL_RESET_SDCON__(void)
{
#if defined(WIN32)
	return	LLDIf.TSPL_RESET_SDCON();
#else
	return	LLDIf.TSPL_RESET_SDCON();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_RESET_SDCON, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSE110_GET_SYNC_STATUS__(int mode)
{
#if defined(WIN32)
	return	LLDIf.TSE110_GET_SYNC_STATUS(mode);
#else
	return	LLDIf.TSE110_GET_SYNC_STATUS(mode);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSE110_GET_SYNC_STATUS, mode, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int	_stdcall TSE110_GET_SIGNAL_STATUS__(int port)
{
#if defined(WIN32)
	return	LLDIf.TSE110_GET_SIGNAL_STATUS(port);
#else
	return	LLDIf.TSE110_GET_SIGNAL_STATUS(port);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSE110_GET_SIGNAL_STATUS, port, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int	_stdcall TSE110_GET_SYNC_FORMAT__(void)
{
#if defined(WIN32)
	return	LLDIf.TSE110_GET_SYNC_FORMAT();
#else
	return	LLDIf.TSE110_GET_SYNC_FORMAT();
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSE110_GET_SYNC_FORMAT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int	_stdcall TSE110_SET_CONFIG__(int port_a_mode,int port_b_mode,int tx_clock,int input_port,int output_a_mode,int output_b_mode)
{
#if defined(WIN32)
	return	LLDIf.TSE110_SET_CONFIG(port_a_mode, port_b_mode, tx_clock, input_port, output_a_mode, output_b_mode);
#else
	return	LLDIf.TSE110_SET_CONFIG(port_a_mode, port_b_mode, tx_clock, input_port, output_a_mode, output_b_mode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSE110_SET_CONFIG, port_a_mode, port_b_mode, tx_clock, input_port, output_a_mode, output_b_mode, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_GET_SYNC_POSITION__(int mode, int type, unsigned char *szBuf, int nlen, int nlen_srch, int nlen_step)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_SYNC_POSITION(mode, type, szBuf, nlen, nlen_srch, nlen_step);
#else
	return	LLDIf.TSPL_GET_SYNC_POSITION(mode, type, szBuf, nlen, nlen_srch, nlen_step);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_SYNC_POSITION, mode, type, (long)szBuf, nlen, nlen_srch, nlen_step, 0, 0, 0, 0);
#endif
}
int	_stdcall TSPL_DO_DEINTERLEAVING__(unsigned char *szSrc, unsigned char *szDest)
{
#if defined(WIN32)
	return	LLDIf.TSPL_DO_DEINTERLEAVING(szSrc, szDest);
#else
	return	LLDIf.TSPL_DO_DEINTERLEAVING(szSrc, szDest);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_DO_DEINTERLEAVING, (long)szSrc, (long)szDest, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_DO_RS_DECODING__(unsigned char *szSrc, unsigned char *szDest, int *format,int *err_blk_cnt,int *recovered_err_cnt,int bypass)
{
#if defined(WIN32)
	return	LLDIf.TSPL_DO_RS_DECODING(szSrc, szDest, format, err_blk_cnt, recovered_err_cnt, bypass);
#else
	return	LLDIf.TSPL_DO_RS_DECODING(szSrc, szDest, format, err_blk_cnt, recovered_err_cnt, bypass);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_DO_RS_DECODING, (long)szSrc, (long)szDest, (long)format, (long)err_blk_cnt, (long)recovered_err_cnt, bypass, 0, 0, 0, 0.);
#endif
}
int	_stdcall TSPL_CONVERT_TO_NI__(unsigned char  *szSrc, unsigned char *szDest, int format)
{
#if defined(WIN32)
	return	LLDIf.TSPL_CONVERT_TO_NI(szSrc, szDest, format);
#else
	return	LLDIf.TSPL_CONVERT_TO_NI(szSrc, szDest, format);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_CONVERT_TO_NI, (long)szSrc, (long)szDest, format, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TVB380_SET_MODULATOR_INIT_CIF__(void)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_INIT_CIF();
#else
	return	LLDIf.TVB380_SET_MODULATOR_INIT_CIF();
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_INIT_CIF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int	_stdcall TVB380_SET_MODULATOR_RUN_CIF__(unsigned char  *szSrc, unsigned char *szDest)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_RUN_CIF(szSrc, szDest);
#else
	return	LLDIf.TVB380_SET_MODULATOR_RUN_CIF(szSrc, szDest);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_RUN_CIF, (long)szSrc, (long)szDest, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_MAX_PLAYRATE__(long modulator_type, long use_max_playrate)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_MAX_PLAYRATE(modulator_type, use_max_playrate);
#else
	return	LLDIf.TSPL_SET_MAX_PLAYRATE(modulator_type, use_max_playrate);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_MAX_PLAYRATE, modulator_type, use_max_playrate, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_CONFIG__(long modulator_type, long IF_Frequency)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_CONFIG(modulator_type, IF_Frequency);
#else
	return	LLDIf.TVB380_SET_CONFIG(modulator_type, IF_Frequency);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_CONFIG, modulator_type, IF_Frequency, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_GET_AD9775__(long reg)
{
#if defined(WIN32)
	return	LLDIf.TSPL_GET_AD9775(reg);
#else
	return	LLDIf.TSPL_GET_AD9775(reg);
//	return	_tvbxxx_msg_snd_get_sts(MSG_tvbxxx_TSPL_GET_AD9775, reg, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TSPL_SET_AD9775__(long reg, long data)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_AD9775(reg, data);
#else
	return	LLDIf.TSPL_SET_AD9775(reg, data);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_AD9775, reg, data, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}

int _stdcall TSPL_SET_AD9787__(long reg, long data)
{
	return	LLDIf.TSPL_SET_AD9787(reg, data);
}

int _stdcall TSPL_GET_AD9787__(long reg)
{
	return	LLDIf.TSPL_GET_AD9787(reg);
}

int _stdcall TVB380_SET_MODULATOR_BANDWIDTH__(long modulator_type,long bandwidth, unsigned long output_frequency)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_BANDWIDTH(modulator_type, bandwidth, output_frequency);
#else
	return	LLDIf.TVB380_SET_MODULATOR_BANDWIDTH(modulator_type, bandwidth, output_frequency);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_BANDWIDTH, modulator_type, bandwidth, output_frequency, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_GUARDINTERVAL__(long modulator_type,long guard_interval)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_GUARDINTERVAL(modulator_type, guard_interval);
#else
	return	LLDIf.TVB380_SET_MODULATOR_GUARDINTERVAL(modulator_type, guard_interval);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_GUARDINTERVAL, modulator_type, guard_interval, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
//2011/11/01 added PAUSE
int _stdcall TVB380_SET_MODULATOR_OUTPUT__(long modulator_type,long output)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_OUTPUT(modulator_type, output);
#else
	return	LLDIf.TVB380_SET_MODULATOR_OUTPUT(modulator_type, output);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_OUTPUT, modulator_type, output, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}

int _stdcall TVB380_SET_MODULATOR_CONSTELLATION__(long modulator_type,long constellation)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_CONSTELLATION(modulator_type, constellation);
#else
	return	LLDIf.TVB380_SET_MODULATOR_CONSTELLATION(modulator_type, constellation);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_CONSTELLATION, modulator_type, constellation, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_FREQ__(long modulator_type,unsigned long output_frequency, long symbol_rate_or_bandwidth)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_FREQ(modulator_type, output_frequency, symbol_rate_or_bandwidth);
#else
	return	LLDIf.TVB380_SET_MODULATOR_FREQ(modulator_type, output_frequency, symbol_rate_or_bandwidth);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_FREQ, modulator_type, output_frequency, symbol_rate_or_bandwidth, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_SYMRATE__(long modulator_type, unsigned long output_frequency, long symbol_rate_or_bandwidth)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_SYMRATE(modulator_type, output_frequency, symbol_rate_or_bandwidth);
#else
	return	LLDIf.TVB380_SET_MODULATOR_SYMRATE(modulator_type, output_frequency, symbol_rate_or_bandwidth);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_SYMRATE, modulator_type, output_frequency, symbol_rate_or_bandwidth, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_CODERATE__(long modulator_type, long code_rate)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_CODERATE(modulator_type, code_rate);
#else
	return	LLDIf.TVB380_SET_MODULATOR_CODERATE(modulator_type, code_rate);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_CODERATE, modulator_type, code_rate, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_TXMODE__(long modulator_type, long tx_mode)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_TXMODE(modulator_type, tx_mode);
#else
	return	LLDIf.TVB380_SET_MODULATOR_TXMODE(modulator_type, tx_mode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_TXMODE, modulator_type, tx_mode, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_INTERLEAVE__(long modulator_type, long interleaving)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_INTERLEAVE(modulator_type, interleaving);
#else
	return	LLDIf.TVB380_SET_MODULATOR_INTERLEAVE(modulator_type, interleaving);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_INTERLEAVE, modulator_type, interleaving, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_IF_FREQ__(long modulator_type, long IF_frequency)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_IF_FREQ(modulator_type, IF_frequency);
#else
	return	LLDIf.TVB380_SET_MODULATOR_IF_FREQ(modulator_type, IF_frequency);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_IF_FREQ, modulator_type, IF_frequency, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_SPECTRUM_INVERSION__(long modulator_type, long spectral_inversion)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_SPECTRUM_INVERSION(modulator_type, spectral_inversion);
#else
	return	LLDIf.TVB380_SET_MODULATOR_SPECTRUM_INVERSION(modulator_type, spectral_inversion);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_SPECTRUM_INVERSION, modulator_type, spectral_inversion, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_STOP_MODE__(long stop_mode)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_STOP_MODE(stop_mode);
#else
	return	LLDIf.TVB380_SET_STOP_MODE(stop_mode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_STOP_MODE, stop_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB390_PLL2_DOWNLOAD__(long value)
{
#if defined(WIN32)
	return	LLDIf.TVB390_PLL2_DOWNLOAD(value);
#else
	return	LLDIf.TVB390_PLL2_DOWNLOAD(value);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB390_PLL2_DOWNLOAD, value, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_PRBS_MODE__(long modulator_type, long mode)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_MODE(modulator_type, mode);
#else
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_MODE(modulator_type, mode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_MODE, modulator_type, mode, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_PRBS_SCALE__(long modulator_type, double scale)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_SCALE(modulator_type, scale);
#else
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_SCALE(modulator_type, scale);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_SCALE, modulator_type, 0, 0, 0, 0, 0, 0, 0, 0, scale);
#endif
}
int _stdcall TVB380_SET_MODULATOR_PRBS_INFO__(long modulator_type, long mode, double scale)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_INFO(modulator_type, mode, scale);
#else
	return	LLDIf.TVB380_SET_MODULATOR_PRBS_INFO(modulator_type, mode, scale);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_INFO, modulator_type, mode, 0, 0, 0, 0, 0, 0, 0, scale);
#endif
}
//2011/6/29 added UseTAT4710 
int _stdcall TVB380_SET_MODULATOR_AGC__(long modulator_type, long agc_on_off, long UseTAT4710)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_AGC(modulator_type, agc_on_off, UseTAT4710);
#else
	return	LLDIf.TVB380_SET_MODULATOR_AGC(modulator_type, agc_on_off, UseTAT4710);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_AGC, modulator_type, agc_on_off, UseTAT4710, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
//2011/6/29 added UseTAT4710 
int _stdcall TVB380_SET_MODULATOR_ATTEN_VALUE__(long modulator_type, double atten_value, long UseTAT4710)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_ATTEN_VALUE(modulator_type, atten_value, UseTAT4710);
#else
	return	LLDIf.TVB380_SET_MODULATOR_ATTEN_VALUE(modulator_type, atten_value, UseTAT4710);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_ATTEN_VALUE, modulator_type, UseTAT4710, 0, 0, 0, 0, 0, 0, 0, atten_value);
#endif
}

//2012/8/27 new rf level control
int _stdcall TVB59x_SET_MODULATOR_RF_LEVEL_VALUE__(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710)
{
#if defined(WIN32)
	return	LLDIf.TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(modulator_type, rf_level_value, AmpFlag, UseTAT4710);
#else
	return	LLDIf.TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(modulator_type, rf_level_value, AmpFlag, UseTAT4710);
#endif
}

int _stdcall TVB59x_SET_Output_TS_Type__(int _val)
{
#if defined(WIN32)
	return	LLDIf.TVB59x_SET_Output_TS_Type(_val);
#else
	return	LLDIf.TVB59x_SET_Output_TS_Type(_val);
#endif
}

//2013/6/10 TVB599 Reset Control
int _stdcall TVB59x_SET_Reset_Control_REG__(int _val)
{
	return	LLDIf.TVB59x_SET_Reset_Control_REG(_val);
}

//2012/9/6 Pcr Restamping control
int _stdcall TVB59x_SET_PCR_STAMP_CNTL__(int _val)
{
	return	LLDIf.TVB59x_SET_PCR_STAMP_CNTL(_val);
}

int _stdcall TVB59x_SET_TsPacket_CNT_Mode__(int _val)
{
	return LLDIf.TVB59x_SET_TsPacket_CNT_Mode(_val);

}

int _stdcall TVB59x_Get_Asi_Input_rate__(int *delta_packet, int *delta_clock)
{
	return LLDIf.TVB59x_Get_Asi_Input_rate(delta_packet, delta_clock);
}

int _stdcall TVB59x_Modulator_Status_Control__(int modulator, int index, int val)
{
	return LLDIf.TVB59x_Modulator_Status_Control(modulator, index, val);
}
int _stdcall TVB59x_Get_Modulator_Status__(int index)
{
	return LLDIf.TVB59x_Get_Modulator_Status(index);
}

//2012/8/29 new rf level control
int _stdcall TVB59x_GET_MODULATOR_RF_LEVEL_RANGE__(long modulator_type, double *rf_level_min, 
                                       double *rf_level_max, long UseTAT4710)
{
#if defined(WIN32)
	return	LLDIf.TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(modulator_type, rf_level_min, rf_level_max, UseTAT4710);
#else
	return	LLDIf.TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(modulator_type, rf_level_min, rf_level_max, UseTAT4710);
#endif
}


int _stdcall TVB380_SET_MODULATOR_DVBH__(long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_DVBH(modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);
#else
	return	LLDIf.TVB380_SET_MODULATOR_DVBH(modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_DVBH, modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_PILOT__(long modulator_type, long  pilot_on_off)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_PILOT(modulator_type, pilot_on_off);
#else
	return	LLDIf.TVB380_SET_MODULATOR_PILOT(modulator_type, pilot_on_off);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_PILOT, modulator_type, pilot_on_off, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_ROLL_OFF_FACTOR__(long modulator_type, long  roll_off_factor)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(modulator_type, roll_off_factor);
#else
	return	LLDIf.TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(modulator_type, roll_off_factor);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_ROLL_OFF_FACTOR, modulator_type, roll_off_factor, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_BOARD_CONFIG_STATUS__(long modulator_type, long status)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_BOARD_CONFIG_STATUS(modulator_type, status);
#else
	return	LLDIf.TVB380_SET_BOARD_CONFIG_STATUS(modulator_type, status);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_BOARD_CONFIG_STATUS, modulator_type, status, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
double _stdcall TVB380_GET_MODULATOR_RF_POWER_LEVEL__(long modulator_type, long info_type)
{
#if defined(WIN32)
	return	LLDIf.TVB380_GET_MODULATOR_RF_POWER_LEVEL(modulator_type, info_type);
#else
	return	LLDIf.TVB380_GET_MODULATOR_RF_POWER_LEVEL(modulator_type, info_type);
//	return	_tvbxxx_msg_snd_get_sts_double(MSG_tvbxxx_TVB380_GET_MODULATOR_RF_POWER_LEVEL, modulator_type, info_type, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}
int _stdcall TVB380_SET_MODULATOR_BERT_MEASURE__(long modulator_type, long packet_type)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_BERT_MEASURE(modulator_type, packet_type);
#else
	return	LLDIf.TVB380_SET_MODULATOR_BERT_MEASURE(modulator_type, packet_type);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_BERT_MEASURE, modulator_type, packet_type, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_DTMB__(long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_DTMB(modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);
#else
	return	LLDIf.TVB380_SET_MODULATOR_DTMB(modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_DTMB, modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion, 0, 0.);
#endif
}
int _stdcall TVB380_SET_MODULATOR_SDRAM_CLOCK__(long modulator_type, long sdram_clock)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_SDRAM_CLOCK(modulator_type, sdram_clock);
#else
	return	LLDIf.TVB380_SET_MODULATOR_SDRAM_CLOCK(modulator_type, sdram_clock);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_SDRAM_CLOCK, modulator_type, sdram_clock, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_DMA_DIRECTION__(long modulator_type, long dma_direction)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_DMA_DIRECTION(modulator_type, dma_direction);
#else
	return	LLDIf.TSPL_SET_DMA_DIRECTION(modulator_type, dma_direction);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_DMA_DIRECTION, modulator_type, dma_direction, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_RESET_IP_CORE__(long modulator_type, long reset_control)
{
#if defined(WIN32)
	return	LLDIf.TSPL_RESET_IP_CORE(modulator_type, reset_control);
#else
	return	LLDIf.TSPL_RESET_IP_CORE(modulator_type, reset_control);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_RESET_IP_CORE, modulator_type, reset_control, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_MH_MODE__(long modulator_type, long mh_mode)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_MH_MODE(modulator_type, mh_mode);
#else
	return	LLDIf.TSPL_SET_MH_MODE(modulator_type, mh_mode);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_MH_MODE, modulator_type, mh_mode, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_MH_PID__(long modulator_type, long mh_pid)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_MH_PID(modulator_type, mh_pid);
#else
	return	LLDIf.TSPL_SET_MH_PID(modulator_type, mh_pid);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_MH_PID, modulator_type, mh_pid, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_SYMBOL_CLOCK__(long modulator_type, long symbol_clock)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_SYMBOL_CLOCK(modulator_type, symbol_clock);
#else
	return	LLDIf.TSPL_SET_SYMBOL_CLOCK(modulator_type, symbol_clock);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_SYMBOL_CLOCK, modulator_type, symbol_clock, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
/////////////////////////////////////////////////////////////////
int _stdcall TVBxxx_DETECT_BD__(int _multi_bd)
{
	return	LLDIf.TVBxxx_DETECT_BD(_multi_bd);
}
int	_stdcall TVBxxx_INIT_BD__(int _my_num, void *_my_cxt)
{
	return	LLDIf.TVBxxx_INIT_BD(_my_num, _my_cxt);
}
int	_stdcall TVBxxx_GET_CNXT_ALL_BD__(void *basic_inform)
{
	return	LLDIf.TVBxxx_GET_CNXT_ALL_BD(basic_inform);
}
int	_stdcall TVBxxx_GET_CNXT_MINE__(void *_cxt)
{
	return	LLDIf.TVBxxx_GET_CNXT_MINE(_cxt);
}
int	_stdcall TVBxxx_DUP_BD__(int from_slot_num, int to_slot_num, void *basic_inform)
{
	return	LLDIf.TVBxxx_DUP_BD(from_slot_num, to_slot_num, basic_inform);
}
#ifdef WIN32
int _stdcall TSPL_CHECK_LN__(char* ln)
{
	return	LLDIf.TSPL_CHECK_LN(ln);
}

//2011/5/4 AD9852 MAX CLK
int _stdcall TSPL_SET_AD9852_MAX_CLOCK__(long value)
{
	return LLDIf.TSPL_SET_AD9852_MAX_CLOCK(value);
}
#else
int TSPL_CHECK_LN__(char* ln)
{
	return	LLDIf.TSPL_CHECK_LN(ln);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_CHECK_LN, (long)ln, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
}
#endif
int TSPL_CNT_MULTI_VSB_RFOUT__(void)
{
	return	LLDIf.TSPL_CNT_MULTI_VSB_RFOUT();
}
int TSPL_CNT_MULTI_QAM_RFOUT__(void)
{
	return	LLDIf.TSPL_CNT_MULTI_QAM_RFOUT();
}
//2012/6/28 multi dvb-t
int TSPL_CNT_MULTI_DVBT_RFOUT__(void)
{
	return	LLDIf.TSPL_CNT_MULTI_DVBT_RFOUT();
}

int _stdcall TSPL_SEL_DDSCLOCK_INC_594__(long play_freq_in_herz, long multi_module_tag)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SEL_DDSCLOCK_INC_594(play_freq_in_herz, multi_module_tag);
#else
	return	LLDIf.TSPL_SEL_DDSCLOCK_INC_594(play_freq_in_herz, multi_module_tag);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC_594, play_freq_in_herz, multi_module_tag, 0, 0, 0, 0, 0, 0, 0, 0.);
#endif
}
int _stdcall TSPL_SET_PLAY_RATE_594__(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource)
{
#if defined(WIN32)
	return	LLDIf.TSPL_SET_PLAY_RATE_594(play_freq_in_herz, multi_module_tag, nOutputClockSource);
#else
	return	LLDIf.TSPL_SET_PLAY_RATE_594(play_freq_in_herz, multi_module_tag, nOutputClockSource);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TSPL_SET_PLAY_RATE_594, play_freq_in_herz, multi_module_tag, nOutputClockSource, 0, 0, 0, 0, 0, 0, 0.);
#endif
}

#ifdef WIN32
int TSPL_PUT_CHAR__(long dwCommand)
{
	return	LLDIf.TSPL_PUT_CHAR(dwCommand);
}
int TSPL_GET_CHAR__(void)
{
	return	LLDIf.TSPL_GET_CHAR();
}
#endif

int	_stdcall TVB380_GET_wr_BUF_STS_MultiBd_n__(int _mod_n)
{
	return	LLDIf.TVB380_GET_wr_BUF_STS_MultiBd_n(_mod_n);
}
int	_stdcall TVB380_GET_wr_cap_BUF_STS_MultiBd_n__(void)
{
	return	LLDIf.TVB380_GET_wr_cap_BUF_STS_MultiBd_n();
}
int	_stdcall TSPL_SEL_TS_TAG_VirtualBd_n__(int _mod_n)
{
	return	LLDIf.TSPL_SEL_TS_TAG_VirtualBd_n(_mod_n);
}
int 	_stdcall SelMultiModAsiOut_n__(int _ts_n)
{
	return	LLDIf.SelMultiModAsiOut_n(_ts_n);
}
int 	_stdcall SelMultiMod310Out_n__(int _ts_n)
{
	return	LLDIf.SelMultiMod310Out_n(_ts_n);
}
int 	_stdcall SelMultiModTsOutput_n__(int _ctl)
{
	return	LLDIf.SelMultiModTsOutput_n(_ctl);
}
int 	_stdcall SelMultiModOperationMode_n__(int _ctl)
{
	return	LLDIf.SelMultiModOperationMode_n(_ctl);
}
//2012/4/12 SINGLE TONE
int _stdcall TVB380_SET_MODULATOR_SINGLE_TONE__(long modulator_type, unsigned long freq, long singleTone)
{
#if defined(WIN32)
	return	LLDIf.TVB380_SET_MODULATOR_SINGLE_TONE(modulator_type, freq, singleTone);
#else
	return	LLDIf.TVB380_SET_MODULATOR_SINGLE_TONE(modulator_type, freq, singleTone);
//	return	_tvbxxx_msg_snd_set_ctl(MSG_tvbxxx_TVB380_SET_MODULATOR_SINGLE_TONE, modulator_type, freq, singleTone, 0, 0, 0, 0, 0, 0, 0);
#endif
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////



