//=================================================================	
//	tvb360u.c / Modulator Manager for TVB370/380/390/590(E,S)/595/597A
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
#include	<math.h>

#include	"initguid.h"
#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"
#include 	"tvb360u.h"

//=================================================================	
// Linux
#else
#define _FILE_OFFSET_BITS 64
#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<memory.h>
#include	<math.h>

#include	"../tsp100.h"
#include	"dll_error.h"
#include	"../include/logfile.h"
#include	"wdm_drv.h"
#include	"dma_drv.h"
#include	"mainmode.h"
#include 	"tvb360u.h"
#endif

//TVB590S
#include	"Reg590S.h"

//TVB593
#include	"Reg593.h"
#include	"rf_level_tbl.h"
#include	"single_tone.h"


#define 	MAX_CNR 50
#define AD9857_SCALE_OFFSET	3

#ifdef WIN32
#define CHECK_TIME_START	QueryPerformanceFrequency((_LARGE_INTEGER*)&gFreq);QueryPerformanceCounter((_LARGE_INTEGER*)&gStart);
#define CHECK_TIME_END		QueryPerformanceCounter((_LARGE_INTEGER*)&gEnd);gTime = (float)((double)(gEnd-gStart)/gFreq*1000);

static time_t ltime0, ltime1;
static int gRet = 0;

static __int64 gFreq, gStart, gEnd;
static float gTime = 0.;
#endif

__int64 gPacketCnt_old, gClockCnt_old;
__int64 gPacketCnt_old_out, gClockCnt_old_out;
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CLldTvb360u::CLldTvb360u(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 1;

	usr_reqed_play_freq_in_herz = 0;
	usr_reqed_nOutputClockSource = 0;

}
CLldTvb360u::~CLldTvb360u()
{
}

void	CLldTvb360u::RESET_PLAY_RATE(void)
{
}
int	_stdcall CLldTvb360u::TSPL_SEL_DDSCLOCK_INC_594(long play_freq_in_herz, long multi_module_tag)
{
	DWORD 		dwRet;
	double		dtmpVal;

	LldPrint_FCall("TSPL_SEL_DDSCLOCK_INC_594", play_freq_in_herz, multi_module_tag);
	if (IsAttachedBdTyp_594())	
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 32.) * 33554432 / (38.7853169/2.);

		dwRet = (int)dtmpVal;
		if ( play_freq_in_herz >= 19392000 && play_freq_in_herz < 19393000 )
		{
			dwRet += (1<<31);
		}

		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_DDSCLOCK_INC1 + (0x100 * multi_module_tag), dwRet); 

		return 0;
	}
	return 0;
}
int	_stdcall CLldTvb360u::TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource)	// 0 for PLL clock source, 1 for 19.392 MHz oscillator
{
	LldPrint_FCall("TSPL_SET_PLAY_RATE_594", play_freq_in_herz, nOutputClockSource);

	if (IsAttachedBdTyp_594())	
	{
		usr_reqed_play_freq_in_herz = play_freq_in_herz;
		usr_reqed_nOutputClockSource = nOutputClockSource;
		if(TSPL_SEL_DDSCLOCK_INC_594(play_freq_in_herz,multi_module_tag/*module tag index(0~3) */) != 0)
		{
			return -1;
		}
		return 0;
	}
	return 0;
}

/*^^***************************************************************************
 * Description : DDS Clock Increment value setting
 *				
 * Entry : nBankConfig
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  DDS Clock Increment= (play_freq * 1000 / 8) * (16777216 / 13500000)  for byte clock
 *
 **************************************************************************^^*/
//static	int TSPL_SEL_DDSCLOCK_INC(long play_freq_in_herz)
int	_stdcall CLldTvb360u::TSPL_SEL_DDSCLOCK_INC(long play_freq_in_herz)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;
	int		IncrementValue;
	double		dtmpVal;

	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp_594())	
	{
		return 0;
	}

	LldPrint_FCall("TSPL_SEL_DDSCLOCK_INC", play_freq_in_herz, 0);
#if 0	
	if(IsAttachedBdTyp_599())
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (66./2.);
	}
	else 
#endif
	if (IsAttachedBdTyp_DdsIncNewEquation() || IsAttachedBdTyp_599() || IsAttachedBdTyp_598())
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (38.7853169/2.);
	}
	else
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (64/2.);
	}

	IncrementValue = (int)dtmpVal;

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
	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_DDSCLOCK_INC;
	KCmdInf.dwCmdParm2 = (DWORD) IncrementValue;	// 24bit

	//TVB593, TVB497
	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_DDSCLOCK_INC);
		KCmdInf.dwCmdParm2 = (((dwStatus>>25)&0x7F)<<25) + (IncrementValue&0x1FFFFFF);
	}

	LockDmaMutex();
	
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
 * Description : Set play rate
 *				
 * Entry : play_freq_in_herz(bps), nOutputClockSource(0)
 *
 * Return: nonzero(fail), 0(success)
 *
 * Notes :  nOutputClockSource is reserved.
 *
 **************************************************************************^^*/
int	_stdcall CLldTvb360u::TSPL_SET_PLAY_RATE(long play_freq_in_herz, 
		long nOutputClockSource)	// 0 for PLL clock source, 1 for 19.392 MHz oscillator
{
	DWORD	dwByte1st, dwByte2nd;
	DWORD	dwBitRate = 9696;

	//USING DDS INC./DEC
#if 0
	if (TSPL_SEL_DDSCLOCK_INC(play_freq_in_herz) != 0)
	{
		return -1;
	}
#else
	unsigned long dwStatus;
	
	LldPrint_lul("[Set] Playback rate, Max, dummy", play_freq_in_herz, nOutputClockSource, 0);

	if (IsAttachedBdTyp__Virtual())	
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			return	TSPL_SET_PLAY_RATE_VirtualBd_n(my_ts_id, play_freq_in_herz, nOutputClockSource);
		}
		else	//	594.
		{
		}
		return 0;
	}

	LldPrint_FCall("TSPL_SET_PLAY_RATE", play_freq_in_herz, nOutputClockSource);

	usr_reqed_play_freq_in_herz = play_freq_in_herz;
	usr_reqed_nOutputClockSource = nOutputClockSource;

	if ( IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits() )	//	manual key-word "PLAY RATE CTRL Register"
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_MAX_PLAY_RATE);

		//dwStatus = ((nOutputClockSource == 1 ? 1 : 0)<<26) + ((dwStatus>>26)&0x3FFFFFF); 
		dwStatus = ((nOutputClockSource == 1 ? 1 : 0)<<26) + (dwStatus&0x3FFFFFF);
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, dwStatus);
	}
	else if ( IsAttachedBdTyp_497() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_MAX_PLAY_RATE);
		dwStatus = (((dwStatus>>27)&0x01)<<27) +  ((nOutputClockSource == 1 ? 1 : 0)<<26) + (dwStatus&0x3FFFFFF); 
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, dwStatus);
	}
	else
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_MAX_PLAY_RATE, nOutputClockSource == 1 ? 1 : 0);
	}
	
	/* set play rate */
	/* internal OSC */
	if ( nOutputClockSource != 1 && ((nOutputClockSource == 2) || 
		((play_freq_in_herz < 19393000L) && (play_freq_in_herz > 19392000L))) )
	{
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, PCI590S_EXT_OSC_SELECTION);
		}
		else if ( IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
			dwStatus = (((dwStatus>>26)&0x01)<<26) + (PCI593_EXT_OSC_SELECTION<<25) + (dwStatus&0x1FFFFFF);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, dwStatus);
		}
		else if ( IsAttachedBdTyp_497() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
			dwStatus = (((dwStatus>>26)&0x03)<<26) + (PCI593_EXT_OSC_SELECTION<<25) + (dwStatus&0x1FFFFFF);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, dwStatus);
		}
		else
		{
			dwBitRate = (DWORD)(play_freq_in_herz / 2000);
			/* Set feed back */
			dwByte1st = (dwBitRate & 0x000000ff) << 24;	/* bit 31..24 -> bit 7..0 */
			dwByte2nd = (dwBitRate & 0x0000ff00) << 8;	/* bit 23..16 -> bit 15..8 */
			if (WDM_WR2SDRAM(CARD_PLL_FB_DIV | dwByte1st | dwByte2nd) == 0)
			{
				LldPrint_Error("tvb360 : pos...", 1, 1);
				return -1;
			}

			/* compare frequency becomes 1 kHz */
			dwByte1st = (19392 & 0x000000ff) << 24;		/* bit 31..24 -> bit 7..0 */
			dwByte2nd = (19392 & 0x0000ff00) << 8;		/* bit 23..16 -> bit 15..8 */
			if (WDM_WR2SDRAM(CARD_PLL_REFDIV | dwByte1st | dwByte2nd) == 0)
			{
				LldPrint_Error("tvb360 : pos...", 1, 2);
				return -1;
			}

			if (WDM_WR2SDRAM(CARD_PLAY_SLCT_OSC | CARD_OUT_SLCT_OSC) == 0)	
			{
				LldPrint_Error("tvb360 : pos...", 1, 3);
			   return -1;
			}
		}
	}
	else
	{
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, PCI590S_NO_EXT_OSC_SELECTION);
		}
		else if ( IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
			dwStatus = (((dwStatus>>26)&0x01)<<26) + (PCI593_NO_EXT_OSC_SELECTION<<25) + (dwStatus&0x1FFFFFF);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, dwStatus);
		}
		else if ( IsAttachedBdTyp_497() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
			dwStatus = (((dwStatus>>26)&0x03)<<26) + (PCI593_NO_EXT_OSC_SELECTION<<25) + (dwStatus&0x1FFFFFF);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_SELECT_EXT_OSC, dwStatus);
		}
		else
		{
			if (WDM_WR2SDRAM(CARD_SLCT_PLL) == 0)
			{
				LldPrint_Error("tvb360 : pos...", 1, 4);
			   return -1;
			}
		}

		if(TSPL_SEL_DDSCLOCK_INC(play_freq_in_herz) != 0)
		{
			LldPrint_Error("tvb360 : pos...", 1, 5);
			return -1;
		}
	}


	//USING DDS INC./DEC
#endif

	return 0;
}

//------------------------------------------------------------------------------
// Modulator Manager API
//------------------------------------------------------------------------------
// DVB-T/H
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_BANDWIDTH(long modulator_type,long bandwidth, unsigned long output_frequency)//2150MHz support - int -> unsigned long
{
//	CHECK_TIME_START

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	if (IsAttachedBdTyp_594())
	{
		output_frequency += 6000000;
	}
	else
	{
		if (CntAdditionalVsbRfOut_593_591s())
		{
			output_frequency += 9000000;
		}
		else if (CntAdditionalQamRfOut_593_591s())
		{
			if(IsAttachedBdTyp_591S())
				output_frequency += 3000000;
			else
				output_frequency += 9000000;
		}
		//2012/6/27 multi dvb-t
		else if(CntAdditionalDvbTRfOut_593())
		{
			if(IsAttachedBdTyp_598())
			{
				if(bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 9000000;
				}
				else if(bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 10500000;
				}
				else if(bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 12000000;
				}
			}
			else
			{
				if(bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 3000000;
				}
				else if(bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 3500000;
				}
				else if(bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 4000000;
				}
			}
			TVB380_SET_MODULATOR_BANDWIDTH_VirtualBd_n(modulator_type, bandwidth);
		}
	}

//------------------------------------------------------------------------------
	bandwidth &= 0x00000007;
	gSymbol_Rate = bandwidth;

	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_BANDWIDTH", bandwidth, output_frequency, 0);

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		//2011/3/25
		if(modulator_type != TVB380_DVBC2_MODE)
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_BANDWIDTH);
			if(modulator_type == TVB380_DVBT2_MODE)
			{
				int interpolation = 0;
				if(bandwidth == 4)
					interpolation = 2;
				dwStatus = (((dwStatus>>7)&0x1FFFFFF)<<7) + ((interpolation &0x3) << 5) + (dwStatus & 0x1F);	
			}
			else
				dwStatus = (((dwStatus>>13)&0x3FFFF)<<13) + (bandwidth<<11) + (dwStatus&0x7FF);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_BANDWIDTH, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 6);
				return -1;
			}
		}
	}
	else
	{
		//2011/3/25
		if(modulator_type != TVB380_DVBC2_MODE)
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_BANDWIDTH, bandwidth))
			{
				LldPrint_Error("tvb360 : pos...", 1, 7);
				return -1;
			}
		}
	}
		
	WaitMsec(100);//Sleep(100);
	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
	WaitMsec(100);//Sleep(100);
	if(nClockGeneratorModel == HW_HMC1033_988)
	{
		if(CntlClkGen_HMC_1033_988_Symclock(modulator_type, bandwidth) < 0)
		{
			LldPrint_Error("tvb360 : pos...", 1, 700);
			return -1;
		}
	}
	else
	{
		if ( CntlClkGen_AD9852_Symclock(modulator_type, bandwidth) < 0 )
		{
			LldPrint_Error("tvb360 : pos...", 1, 8);
			return -1;
		}
	}
	CntlDac_AD9xxx(modulator_type, output_frequency, bandwidth);

	ResetModBlkAfterChangingHwDacPara(3);

	RESET_PLAY_RATE();

	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_GUARDINTERVAL(long modulator_type,long guard_interval)
{
	unsigned long dwStatus;

	guard_interval &= 0x00000003;

	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		if(IsAttachedBdTyp__Virtual())
		{
			TVB380_SET_MODULATOR_GUARDINTERVAL_VirtualBd_n(modulator_type, guard_interval);
			return 0;
		}
	}

	LldPrint("TVB380_SET_MODULATOR_GUARDINTERVAL", modulator_type, guard_interval, 0);

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_GUARD_INTERVAL);

		dwStatus = (((dwStatus>>8)&0xFFFFFF)<<8) + (guard_interval<<6) + (dwStatus&0x3F);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_GUARD_INTERVAL, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 9);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_GUARD_INTERVAL, guard_interval))
		{
			LldPrint_Error("tvb360 : pos...", 1, 10);
			return -1;
		}
	}

#if 1
	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_mod_blk_);
	}
	else
#endif
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//2011/11/01 added PAUSE
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_OUTPUT(long modulator_type,long output)
{
	unsigned long dwStatus;

	output &= 0x00000001;

	if (IsAttachedBdTyp__Virtual())
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			TVB380_SET_MODULATOR_DISABLE_VirtualBd_n(output);
			return	0;
		}
	}
	LldPrint("TVB380_SET_MODULATOR_OUTPUT", modulator_type, output, 0);

	//TVB593, TVB497
	if ( CHK_ID(0x00,_BD_ID_593__,_BD_ID_497__,_BD_ID_499__,_BD_ID_597v2__,_BD_ID_591__,_BD_ID_591S__,_BD_ID_599__,_BD_ID_598__,0x00) )
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_MOD_COMMAND1);

		if(modulator_type == TVB380_DVBC2_MODE)
		{
			dwStatus = ((dwStatus) & 0x7FFFFFFF) + (output << 31);
		}
		else
		{
			dwStatus = (((dwStatus>>1)&0x7FFFFFFF)<<1) + output;
		}
		if(TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_MOD_COMMAND1, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 89);
			return -1;
		}
	}
	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_CONSTELLATION(long modulator_type,long constellation)
{
	//TVB590S
	long scale_factor = 0;

	//TVB593
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_CONSTELLATION", modulator_type, constellation, 0);


	//TVB590S V3
	if ( modulator_type == TVB380_QAMA_MODE )
	{
		gQAMA_Constllation = constellation;
	}
	else if ( modulator_type == TVB380_QAMB_MODE )
	{
		gQAMB_Constllation = constellation;
	}
	else if ( modulator_type == TVB380_MULTI_QAMB_MODE )
	{
		if (IsAttachedBdTyp__Virtual())
		{
			return 0;
		}
		gQAMB_Constllation = constellation;
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			TVB380_SET_MODULATOR_CONSTELLATION_VirtualBd_n(modulator_type, constellation);
		}
	}
	//2012/6/28 multi dvb-t
	else if( modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		if(IsAttachedBdTyp__Virtual())
		{
			TVB380_SET_MODULATOR_CONSTELLATION_VirtualBd_n(modulator_type, constellation);
			return 0;
		}
	}

	constellation &= 0x00000007;
	if ( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_DVBH_MODE|| modulator_type == TVB380_MULTI_DVBT_MODE )
	{
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_CONSTELLATION);

			dwStatus = (((dwStatus>>6)&0x3FFFFFF)<<6) + (constellation<<4) + (dwStatus&0xF);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CONSTELLATION, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 11);
				return -1;
			}
		}
		else
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CONSTELLATION, constellation))
			{
				LldPrint_Error("tvb360 : pos...", 1, 12);
				return -1;
			}
		}
	}
	else if ( modulator_type == TVB380_DVBS2_MODE )
	{
		constellation &= 0x00000003;//QPSK, 8PSK

		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_CONSTELLATION);

			dwStatus = (((dwStatus>>4)&0xFFFFFFF)<<4) + (constellation<<2) + (dwStatus&0x3);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CONSTELLATION, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 13);
				return -1;
			}
		}
		else
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CONSTELLATION, constellation))
			{
				LldPrint_Error("tvb360 : pos...", 1, 14);
				return -1;
			}
		}
	}
	else if ( modulator_type == TVB380_CMMB_MODE )
	{
		//TVB593
		scale_factor = 0x400;
		if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_SCALE_FACTOR, scale_factor) )
		{
			LldPrint_Error("tvb360 : pos...", 1, 15);
			return -1;
		}
	}
	//DVB-T2
	else if (( modulator_type == TVB380_DVBT2_MODE ) || (modulator_type == TVB380_DVBC2_MODE))
	{
	}
	else
	{
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
			
			if  ( IsAttachedBdTyp_591() )
			{
				if ( modulator_type == TVB380_QAMA_MODE )
				{
					dwStatus = (((dwStatus >> 5) & 0x3) << 5) + (constellation << 2) + (dwStatus & 0x3);
				}
				else if ( modulator_type == TVB380_QAMB_MODE )
				{
					dwStatus = (((dwStatus >> 3) & 0x3F) << 3) + (constellation << 2) + (dwStatus & 0x3);
				}
				else
					dwStatus = (constellation<<2) + (dwStatus&0x3);
			}
			else
			{
				if ( modulator_type == TVB380_QAMA_MODE )
				{
					dwStatus = (((dwStatus >> 5) & 0x7) << 5) + (constellation << 2) + (dwStatus & 0x3);
				}
				else if ( modulator_type == TVB380_QAMB_MODE )
				{
					dwStatus = (((dwStatus >> 3) & 0x7F) << 3) + (constellation << 2) + (dwStatus & 0x3);
				}
				else
					dwStatus = (((dwStatus >> 3) & 0x7F) << 3) + (constellation<<2) + (dwStatus&0x3);
			}
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_TXMODE, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 16);
				return -1;
			}
		}
		else
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_TXMODE, constellation))
			{
				LldPrint_Error("tvb360 : pos...", 1, 17);
				return -1;
			}
		}

		if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
		{
			if ( modulator_type == TVB380_QAMA_MODE )
			{
				if ( constellation == 1 || constellation == 3 )//CONST_QAM_A_32 or CONST_QAM_A_128
				{
					scale_factor = 0x5F0;
				}
				else
				{
					scale_factor = 0x450;
				}
			}
			else if ( modulator_type == TVB380_QAMB_MODE )
			{
				scale_factor = 0x450;
			}

			if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_SCALE_FACTOR, scale_factor) )
			{
				LldPrint_Error("tvb360 : pos...", 1, 18);
				return -1;
			}
		}

		if(IsAttachedBdTyp_591())
		{
			if ( modulator_type == TVB380_QAMA_MODE )
			{
				if ( constellation == 1 || constellation == 3 )//CONST_QAM_A_32 or CONST_QAM_A_128
				{
					scale_factor = 0x350;
				}
				else
				{
					scale_factor = 0x260;
				}
			}
			else if ( modulator_type == TVB380_QAMB_MODE )
			{
				scale_factor = 0x260;
			}

			if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_SCALE_FACTOR, scale_factor) )
			{
				LldPrint_Error("tvb360 : pos...", 1, 18);
				return -1;
			}
		}
	}

#if 1
	if(modulator_type == TVB380_MULTI_DVBT_MODE || modulator_type == TVB380_MULTI_QAMB_MODE)
	{
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_mod_blk_);
	}
	else
#endif
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//------------------------------------------------------------------------------
// All type
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_FREQ(long modulator_type,unsigned long output_frequency, long symbol_rate_or_bandwidth)//2150MHz support - int -> unsigned long
{
	//FIXED - IF +- offset
	int CurrentIF = gChannel_Freq_Offset;

	//FIXED - ROWSWIN offset
#if 1
	int ROWSWIN_FREQ = 0;
	int ROWSWIN_FREQ_OFFSET = 0;
	int nVal0, nVal1, nVal2, nRet;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	if (IsAttachedBdTyp_594())	
	{
	}
	else
	{
		if (CntAdditionalVsbRfOut_593_591s())
		{
			output_frequency += 9000000;
		}
		else if (CntAdditionalQamRfOut_593_591s())
		{
			if(IsAttachedBdTyp_591S())
				output_frequency += 3000000;
			else
				output_frequency += 9000000;
		}
		else if(CntAdditionalDvbTRfOut_593())
		{
			if(IsAttachedBdTyp_598())
			{
				if(symbol_rate_or_bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 9000000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 10500000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 12000000;
				}
			}
			else
			{
				if(symbol_rate_or_bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 3000000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 3500000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 4000000;
				}
			}
		}
	}
	LldPrint("TVB380_SET_MODULATOR_FREQ", modulator_type,output_frequency, symbol_rate_or_bandwidth);

	//TVB593, TVB497
	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis() )
	{
		if ( gCenter_Freq < 52000000 && output_frequency >= 52000000 )
		{
			nVal0 =	ROWSWIN_REG_VAL0;
			nVal1 = ROWSWIN_REG_VAL1_36M;
			if ( CurrentIF != IF_OUT_36MHZ )	
			{
				nVal1 = ROWSWIN_REG_VAL1_44M;
			}
			nVal2 = ROWSWIN_REG_VAL2;

			if ( modulator_type == TVB380_DTMB_MODE )
			{
				nVal0 =	ROWSWIN_REG_VAL0_DTMB;
				nVal1 = ROWSWIN_REG_VAL1_36M_DTMB;
				if ( CurrentIF != IF_OUT_36MHZ )	
				{
					nVal1 = ROWSWIN_REG_VAL1_44M_DTMB;
				}
				nVal2 = ROWSWIN_REG_VAL2_DTMB;
			}

			nRet = Config_TRF178_PLL2(nVal0, nVal1, nVal2);
			Sleep(1);
		}

		if ( output_frequency >= 51000000 && output_frequency < 52000000 )
		{
			ROWSWIN_FREQ_OFFSET = 1000000;
		}
		else if ( output_frequency >= 50000000 && output_frequency < 51000000 )
		{
			ROWSWIN_FREQ_OFFSET = 2000000;
		}
		else if ( output_frequency >= 49000000 && output_frequency < 50000000 )
		{
			ROWSWIN_FREQ_OFFSET = 3000000;
		}
		else if ( output_frequency >= 48000000 && output_frequency < 49000000 )
		{
			ROWSWIN_FREQ_OFFSET = 4000000;
		}

		if ( ROWSWIN_FREQ_OFFSET != 0 )
		{
			ROWSWIN_FREQ = ROWSWIN_FREQ_36M + ROWSWIN_FREQ_OFFSET;
			if ( CurrentIF != IF_OUT_36MHZ )
			{
				ROWSWIN_FREQ = ROWSWIN_FREQ_44M + ROWSWIN_FREQ_OFFSET;
			}

			if ( ROWSWIN_FREQ == ROWSWIN_FREQ_36M + 1000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe45; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_36M + 2000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe49; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_36M + 3000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe4d; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_36M + 4000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe51; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_44M + 1000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe25; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_44M + 2000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe29; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_44M + 3000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe2d; nVal2 = 0x92;
			}
			else if ( ROWSWIN_FREQ == ROWSWIN_FREQ_44M + 4000000 )
			{
				nVal0 = 0x34; nVal1 = 0xe31; nVal2 = 0x92;
			}

			nRet = Config_TRF178_PLL2(nVal0, nVal1, nVal2);
			Sleep(1);
		}
	}	
#endif
	
	gCenter_Freq = output_frequency;
	gSymbol_Rate = symbol_rate_or_bandwidth;

	if (CntlClkGen_RFout_Carrier(modulator_type, output_frequency) < 0)
		//FIXED - IF +- offset
		return CurrentIF;

	//FIXED - IF +- offset
	CurrentIF = (gChannel_Freq_Offset + (gRF_143KHZ_sign*gRF_143KHZ));
	
	//ATSC-M/H
	if ( modulator_type == TVB380_VSB8_MODE || modulator_type == TVB380_VSB16_MODE || modulator_type == TVB380_ATSC_MH_MODE || modulator_type == TVB380_MULTI_VSB_MODE )
	{
		symbol_rate_or_bandwidth = 4500000;
	}
	else if ( modulator_type == TVB380_TDMB_MODE )
	{
		symbol_rate_or_bandwidth = 2048000;
	}

	//sskim20080925 - TEST
	//Sleep(10);
	//TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
	{
	}
	else
	{
		CntlDac_AD9xxx(modulator_type, output_frequency, symbol_rate_or_bandwidth);
	}

	//TVB593, TVB497
	if ( (IsAttachedBdTyp_AttenTyp_1() || IsAttachedBdTyp_499()) && 
		(modulator_type == TVB380_ISDBT_13_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
	{
		//FIXED - IF +- offset
		return CurrentIF;
	}
	
	//TVB593, TVB497
	if ( (IsAttachedBdTyp_AttenTyp_2() || IsAttachedBdTyp_499()) 
		&& (modulator_type == TVB380_ATSC_MH_MODE 
		&& (CurrentTSSrc == 4/*TSIO_310_CAPTURE_PLAY*/ || CurrentTSSrc == 5/*TSIO_ASI_CAPTURE_PLAY*/)) )
	{
		//FIXED - IF +- offset
		return CurrentIF;
	}
	//2011/6/15 ISDB-S ASI
	if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
		(modulator_type == TVB380_ISDBS_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
	{
		//FIXED - IF +- offset
		return CurrentIF;
	}

	//2012/7/11 DVB-T2 ASI
	if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
		(modulator_type == TVB380_DVBT2_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
	{
		//FIXED - IF +- offset
		return CurrentIF;
	}

	ResetModBlkAfterChangingHwDacPara(3);
	RESET_PLAY_RATE();

	//FIXED - IF +- offset
	return CurrentIF;
}

//------------------------------------------------------------------------------
// QAM-A, QAM-B, QPSK, DVB-S2
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_SYMRATE(long modulator_type, unsigned long output_frequency, long symbol_rate_or_bandwidth)//2150MHz support - int -> unsigned long
{
	LldPrint_lul("TVB380_SET_MODULATOR_SYMRATE", modulator_type, output_frequency, symbol_rate_or_bandwidth);

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	if (IsAttachedBdTyp_594())	
	{
		output_frequency += 6000000;
	}
	else
	{
		if (CntAdditionalVsbRfOut_593_591s())
		{
			output_frequency += 9000000;
		}
		else if (CntAdditionalQamRfOut_593_591s())
		{
			if(IsAttachedBdTyp_591S())
				output_frequency += 3000000;
			else
				output_frequency += 9000000;
		}
		else if(CntAdditionalDvbTRfOut_593())
		{
			if(IsAttachedBdTyp_598())
			{
				if(symbol_rate_or_bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 9000000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 10500000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 12000000;
				}
			}
			else
			{
				if(symbol_rate_or_bandwidth == DVB_T_6M_BAND)
				{
					output_frequency += 3000000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_7M_BAND)
				{
					output_frequency += 3500000;
				}
				else if(symbol_rate_or_bandwidth == DVB_T_8M_BAND)
				{
					output_frequency += 4000000;
				}
			}
		}
	}

	gCenter_Freq = output_frequency;
	gSymbol_Rate = symbol_rate_or_bandwidth;

	if(nClockGeneratorModel == HW_HMC1033_988)
	{
		if(CntlClkGen_HMC_1033_988_Symclock(modulator_type, symbol_rate_or_bandwidth) < 0)
		{
			LldPrint_Error("tvb360 : pos...", 1, 701);
			return -1;
		}
	}
	else
	{
		if ( CntlClkGen_AD9852_Symclock(modulator_type, symbol_rate_or_bandwidth) < 0 )
		{
			LldPrint_Error("tvb360 : pos...", 1, 8);
			return -1;
		}
	}

	Sleep(10);//fixed - ubuntu?
	CntlDac_AD9xxx(modulator_type, output_frequency, symbol_rate_or_bandwidth);

	Sleep(10);//fixed - ubuntu?

	ResetModBlkAfterChangingHwDacPara(3);
	RESET_PLAY_RATE();

	return 0;
}

//------------------------------------------------------------------------------
// DVB-T/H, QPSK, DVB-S2
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_CODERATE(long modulator_type, long code_rate)
{
	//TVB593
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_SYMRATE", modulator_type, code_rate, 0);

	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		if(IsAttachedBdTyp__Virtual())
		{
			TVB380_SET_MODULATOR_CODERATE_VirtualBd_n(modulator_type, code_rate);
			return 0;
		}
	}

	if ( modulator_type == TVB380_DVBS2_MODE )
		code_rate &= 0x0000000F;
	else
		code_rate &= 0x00000007;


	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_CODE_RATE);

		if ( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_DVBH_MODE || modulator_type == TVB380_MULTI_DVBT_MODE)
			dwStatus = (((dwStatus>>11)&0x1FFFFF)<<11) + (code_rate<<8) + (dwStatus&0xFF);
		else if ( modulator_type == TVB380_QPSK_MODE )
			dwStatus = (((dwStatus>>5)&0x7FFFFFF)<<5) + (code_rate<<2) + (dwStatus&0x3);
		else if ( modulator_type == TVB380_DVBS2_MODE )
			dwStatus = (((dwStatus>>8)&0xFFFFFF)<<8) + (code_rate<<4) + (dwStatus&0xF);

		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CODE_RATE, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 20);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CODE_RATE, code_rate))
		{
			LldPrint_Error("tvb360 : pos...", 1, 21);
			return -1;
		}	
	}

#if 1
	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_mod_blk_);
	}
	else
#endif
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//------------------------------------------------------------------------------
// DVB-T/H, QAM-A, QAM-B
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_TXMODE(long modulator_type, long tx_mode)
{
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_TXMODE", modulator_type, tx_mode, 0);
	
	//2013/11/25 DVB-T2 MISO
	if(modulator_type == TVB380_DVBT2_MODE)
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
		dwStatus = (dwStatus & 0xFFFFFDFF) + ((tx_mode & 0x1) << 9);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_TXMODE, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 2, 1);
			return -1;
		}
		return 0;
	}

	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		if(IsAttachedBdTyp__Virtual())
		{
			TVB380_SET_MODULATOR_TXMODE_VirtualBd_n(modulator_type, tx_mode);
			return 0;
		}
	}

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		if ( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			tx_mode = ((tx_mode != 0) ? 2 : 0);
		}
		else if ( modulator_type == TVB380_DVBH_MODE )
		{
		}
		else
		{
			LldPrint_Error("tvb360 : pos...", 1, 22);
			return -1;
		}

		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);

		dwStatus = (((dwStatus>>4)&0xFFFFFFF)<<4) + (tx_mode<<2) + (dwStatus&0x3);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_CODE_RATE, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 23);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_TXMODE, tx_mode))
		{
			LldPrint_Error("tvb360 : pos...", 1, 24);
			return -1;
		}
	}

#if 1
	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_mod_blk_);
	}
	else
#endif
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//------------------------------------------------------------------------------
// QAM-B. TVB380_QAMB_MODE
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_INTERLEAVE(long modulator_type, long interleaving)
{
	DWORD dwControlWord = 0;
	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())	
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			TVB380_SET_MODULATOR_INTERLEAVE_VirtualBd_n(interleaving);
			return	0;
		}
	}
	LldPrint("TVB380_SET_MODULATOR_INTERLEAVE", modulator_type, interleaving, 0);

	if ( interleaving < 5 )
		dwControlWord = interleaving*2 + 1;
	else
		dwControlWord = (interleaving-5)*2;

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_INTERLEAVING);

		dwStatus = (((dwStatus>>7)&0x1FFFFFF)<<7) + (dwControlWord<<3) + (dwStatus&0x7);

		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_INTERLEAVING, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 25);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_INTERLEAVING, dwControlWord))
		{
			LldPrint_Error("tvb360 : pos...", 1, 26);
			return -1;
		}
	}

	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//------------------------------------------------------------------------------
// All type
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_IF_FREQ(long modulator_type, long IF_frequency)
{	
	int nVal0, nVal1, nVal2, nRet;

	if (IsAttachedBdTyp__Virtual())	
	{
		return 0;
	}
	LldPrint("TVB380_SET_MODULATOR_IF_FREQ", modulator_type, IF_frequency, 0);

	nVal0 =	0x410;
	nVal1 = 0x111D01;
	if ( IF_frequency != IF_OUT_36MHZ )	
	{
		nVal1 = 0x111A81;
	}
	nVal2 = 0x92;

	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis() )
	{
		nVal0 =	ROWSWIN_REG_VAL0;
		nVal1 = ROWSWIN_REG_VAL1_36M;
		if ( IF_frequency != IF_OUT_36MHZ )	
		{
			nVal1 = ROWSWIN_REG_VAL1_44M;
		}
		nVal2 = ROWSWIN_REG_VAL2;

		if ( modulator_type == TVB380_DTMB_MODE )
		{
			nVal0 =	ROWSWIN_REG_VAL0_DTMB;
			nVal1 = ROWSWIN_REG_VAL1_36M_DTMB;
			if ( IF_frequency != IF_OUT_36MHZ )	
			{
				nVal1 = ROWSWIN_REG_VAL1_44M_DTMB;
			}
			nVal2 = ROWSWIN_REG_VAL2_DTMB;
		}
	}

	nRet = Config_TRF178_PLL2(nVal0, nVal1, nVal2);
	gChannel_Freq_Offset = IF_frequency;	

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	RESET_PLAY_RATE();

	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_SPECTRUM_INVERSION(long modulator_type, long spectral_inversion)
{
	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())	
	{
		return 0;
	}

	LldPrint("TVB380_SET_MODULATOR_SPECTRUM_INVERSION", modulator_type, spectral_inversion, 0);

	gSpectral_Inversion = spectral_inversion;

	if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
	{
		if ( modulator_type == TVB380_QPSK_MODE || modulator_type == TVB380_DVBS2_MODE )
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);

				if ( modulator_type == TVB380_QPSK_MODE )
					dwStatus = (gSpectral_Inversion<<8) + (dwStatus&0xFF);
				else
					dwStatus = (gSpectral_Inversion<<13) + (dwStatus&0x1FFF);

				if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 27);
					return -1;
				}
			}
			else
			{
				TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, (gSpectral_Inversion<<3) + gClock_Mode);
			}
		}
		//2012/1/10 I/Q PLAY
		else if(modulator_type == TVB380_IQ_PLAY_MODE)
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);

				dwStatus = (gSpectral_Inversion<<2) + (dwStatus&0x01);

				if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 127);
					return -1;
				}
			}
			else
			{
				TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, (gSpectral_Inversion<<2));
			}
		}
		//2012/4/12 
		else if(modulator_type == TVB380_VSB8_MODE || modulator_type == TVB380_ATSC_MH_MODE)
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
				if(modulator_type == TVB380_VSB8_MODE)
					dwStatus = (gSpectral_Inversion<<5) + (dwStatus&0x1F);
				else
					dwStatus = (gSpectral_Inversion<<5) + (dwStatus&0x1FFF001F);

				if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 127);
					return -1;
				}
			}
		}
		else if(modulator_type == TVB380_QAMA_MODE)
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
				dwStatus = (gSpectral_Inversion<<7) + (dwStatus&0x7F);

				if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 127);
					return -1;
				}
			}
		}
		else if(modulator_type == TVB380_QAMB_MODE)
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
				dwStatus = (gSpectral_Inversion<<9) + (dwStatus&0x1FF);

				if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 127);
					return -1;
				}
			}
		}

	}
	RESET_PLAY_RATE();

	return 0;
}

int _stdcall CLldTvb360u::TVB380_IS_ENABLED_TYPE(long modulator_type)
{
	int nEnabled = 0;
	
//	LldPrint_FCall("TVB380_IS_ENABLED_TYPE", modulator_type, 0);

	if ( IsAttachedBdTyp_AllCase() )
	{
		//sskim20080703 - ISDB-T, NO LICENSE
#if 0
		if (hDevice == NULL || modulator_type == -1 )
			return 0;
		return 1;
#else
#if TVB597A_STANDALONE
		if (hDevice == NULL || modulator_type == -1 )
			return 0;
		return 1;	
#endif
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
		if ( !CHK_DEV(hDevice) )
			return 0;
#else
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			if ( !CHK_DEV(hDevice_usb) )
				return 0;
		}
		else
		{
			if ( !CHK_DEV(hDevice) )
				return 0;
		}
#endif
		if ( modulator_type == -1 || TSPL_nModulatorEnabled <= 0)
			return 0;
#endif
		if ( modulator_type == TVB380_DVBT_MODE ) /* DVB-T*/
			nEnabled = (TSPL_nModulatorEnabled & 0x01);
		else if ( modulator_type == TVB380_VSB8_MODE ) /* 8-VSB*/
			nEnabled = (TSPL_nModulatorEnabled >> 1 & 0x01);
		else if ( modulator_type == TVB380_QAMA_MODE ) /* QAM-A*/
			nEnabled = (TSPL_nModulatorEnabled >> 2 & 0x01);
		else if ( modulator_type == TVB380_QAMB_MODE ) /* QAM-B*/
			nEnabled = (TSPL_nModulatorEnabled >> 3 & 0x01);
		else if ( modulator_type == TVB380_QPSK_MODE ) /* QPSK*/
			nEnabled = (TSPL_nModulatorEnabled >> 4 & 0x01);
		else if ( modulator_type == TVB380_TDMB_MODE ) /* TDMB*/
			nEnabled = (TSPL_nModulatorEnabled >> 5 & 0x01);
		else if ( modulator_type == TVB380_VSB16_MODE ) /* 16-VSB*/
			nEnabled = (TSPL_nModulatorEnabled >> 6 & 0x01);
		else if ( modulator_type == TVB380_DVBH_MODE ) /* DVB-H*/
			nEnabled = (TSPL_nModulatorEnabled >> 7 & 0x01);
		else if ( modulator_type == TVB380_DVBS2_MODE ) /* DVB-S2*/
			nEnabled = (TSPL_nModulatorEnabled >> 8 & 0x01);
		//ATSC-M/H - FIXME
		//else if ( modulator_type == 15 ) /* TPG430BM */
		//	nEnabled = (TSPL_nModulatorEnabled >> 15 & 0x01);

		else if ( modulator_type == TVB380_ISDBT_MODE ) /* ISDB-T*/
			nEnabled = (TSPL_nModulatorEnabled >> 9 & 0x01);
		else if ( modulator_type == TVB380_ISDBT_13_MODE ) /* ISDB-T*/
			nEnabled = (TSPL_nModulatorEnabled >> 9 & 0x01);
		else if ( modulator_type == TVB380_DTMB_MODE ) /* DTMB*/
			nEnabled = (TSPL_nModulatorEnabled >> 10 & 0x01);
		else if ( modulator_type == TVB380_CMMB_MODE ) /* CMMB */
			nEnabled = (TSPL_nModulatorEnabled >> 11 & 0x01);
		else if ( modulator_type == TVB380_ATSC_MH_MODE ) /* ATSC-M/H */
			nEnabled = (TSPL_nModulatorEnabled >> 14 & 0x01);
		else if ( modulator_type == TVB380_DVBT2_MODE ) /* DVB-T2 */
			nEnabled = (TSPL_nModulatorEnabled >> 12 & 0x01);
		else if ( modulator_type == TVB380_IQ_PLAY_MODE ) /* IQ Play */
			nEnabled = (TSPL_nModulatorEnabled >> 16 & 0x01);
		//ISDB-S
		else if ( modulator_type == TVB380_ISDBS_MODE ) /* ISDB-S */
			nEnabled = (TSPL_nModulatorEnabled >> 17 & 0x01);
		else if ( modulator_type == TVB380_DVBC2_MODE ) /* DVB-C2 */
			nEnabled = (TSPL_nModulatorEnabled >> 15 & 0x01);
		//2012/4/9 Multiple option
		else if ( modulator_type == TVB380_MULTI_VSB_MODE)
		{
			if(((TSPL_nModulatorEnabled >> 20) & 0x3) > 0)
				nEnabled = 1;
		}
		else if ( modulator_type == TVB380_MULTI_QAMB_MODE)
		{
			if(((TSPL_nModulatorEnabled >> 18) & 0x3) > 0)
				nEnabled = 1;
		}
		//2012/6/27 multi dvb-t
		else if ( modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(((TSPL_nModulatorEnabled >> 22) & 0x3) > 0)
				nEnabled = 1;
		}
		

		return nEnabled;
	}

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
		return 0;// Device is not ready
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
			return 0;
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
			return 0;
	}
#endif
	
	if ( modulator_type == TVB380_DVBT_MODE ) /* DVB-T*/
		nEnabled = (TSPL_nAuthorization & 0x01);
	else if ( modulator_type == TVB380_VSB8_MODE ) /* 8-VSB*/
		nEnabled = (TSPL_nAuthorization >> _bits_sht_en_8vsb_bd_grp0_ & 0x01);
	else if ( modulator_type == TVB380_QAMA_MODE ) /* QAM-A*/
		nEnabled = (TSPL_nAuthorization >> _bits_sht_en_qama_bd_grp0_ & 0x01);
	else if ( modulator_type == TVB380_QAMB_MODE ) /* QAM-B*/
		nEnabled = (TSPL_nAuthorization >> _bits_sht_en_qamb_bd_grp0_ & 0x01);
	else if ( modulator_type == TVB380_QPSK_MODE ) /* QPSK*/
		nEnabled = (TSPL_nAuthorization >> _bits_sht_en_qpsk_bd_grp0_ & 0x01);
	else if ( modulator_type == TVB380_TDMB_MODE ) /* TDMB*/
		nEnabled = (TSPL_nAuthorization >> _bits_sht_en_tdmb_bd_grp0_ & 0x01);
	
	return nEnabled;
}

int _stdcall CLldTvb360u::TVB380_SET_STOP_MODE(long stop_mode)
{
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			TVB380_SET_MODULATOR_NULL_TP_VirtualBd_n(stop_mode);
			return	0;
		}
		return 0;
	}
	LldPrint("TVB380_SET_STOP_MODE", stop_mode, 0, 0);

	//TVB593, TVB497
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_STOP_OUTPUT);

		dwStatus = (((dwStatus>>2)&0x3FFFFFFF)<<2) + (stop_mode<<1) + (dwStatus&0x1);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_STOP_OUTPUT, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 28);
			return -1;
		}
	}
	else

	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_STOP_OUTPUT, stop_mode))
	{
		LldPrint_Error("tvb360 : pos...", 1, 29);
		return -1;
	}

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	//sskim20080604
	//Sleep(10);
	//WDM_WR2SDRAM(CARD_RESET_BOARD);

	return 0;
}

int _stdcall CLldTvb360u::TVB390_PLL2_DOWNLOAD(long value)
{
	LldPrint_FCall("TVB390_PLL2_DOWNLOAD", value, 0);

	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
		return 0;

	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, value))
	{
		LldPrint_Error("tvb360 : pos...", 1, 30);
		return -1;
	}

	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);

	return 0;
}

//------------------------------------------------------------------------------
// TDMB
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_PRBS_MODE(long modulator_type, long mode)
{
	//TVB593
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_PRBS_MODE", modulator_type, mode, 0);

	if ( modulator_type != TVB380_TDMB_MODE ) 
	{
		LldPrint_Error("tvb360 : pos...", 1, 31);
		return -1;
	}

	if ( mode < 0 || mode > 4 )
	{
		LldPrint_Error("tvb360 : pos...", 1, 32);
		return -1;
	}
	
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_PRBS_INFO);

		//FIXED - 7FFFF -> 1FFF
		dwStatus = (((dwStatus>>19)&0x1FFF)<<19) + (mode<<16) + (dwStatus&0xFFFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_INFO, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 33);
			return -1;
		}
	}
	else

	//
	// download PRBS mode
	// 0 : NONE, 1: 2^7 - 1, 2: 2^10 - 1,  3: 2^15 - 1, 4: 2^23 - 1
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_MODE, mode))
	{
		LldPrint_Error("tvb360 : pos...", 1, 34);
		return -1;
	}

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_PRBS_SCALE(long modulator_type, double scale)//long -> double
{
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint("TVB380_SET_MODULATOR_PRBS_SCALE", modulator_type, (int)scale, 0);

	if ( modulator_type != TVB380_TDMB_MODE ) 
	{
		LldPrint_Error("tvb360 : pos...", 1, 35);
		return -1;
	}

	//TVB593
	unsigned long dwStatus;
	
	double scale_offset = -7.3;
	scale += scale_offset;

	//scale : CNR
	if ( scale < 0 || scale > MAX_CNR )
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > MAX_CNR ) scale = MAX_CNR;
	}

	double dwCNR = scale;
	double dwCarrierPower = 5368737;
	//scale : SCALE
	scale = pow((dwCarrierPower / 2.), 0.5) * (1. / pow(10.,(dwCNR / 20.)));
	if ( scale < 0 || scale > 8000)
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > 8000 ) scale = 8000;
	}

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_PRBS_INFO);

		//FIXED - 0x1FFF -> 0x7FFFF
		dwStatus = (((dwStatus>>13)&0x7FFFF)<<13) + (int)scale;
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_INFO, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 36);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_SCALE, (int)scale))
		{
			LldPrint_Error("tvb360 : pos...", 1, 37);
			return -1;
		}
	}

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	return 0;
}

//------------------------------------------------------------------------------
// All type
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_PRBS_INFO(long modulator_type, long mode, double scale)//long -> double
{
	int val = 0;

	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
			return	TVB380_SET_MODULATOR_PRBS_INFO_VirtualBd_n(modulator_type, mode, scale);
		}
		return 0;
	}
	LldPrint_4("TVB380_SET_MODULATOR_PRBS_SCALE", 0,modulator_type, mode, scale);

	if ( modulator_type == TVB380_TDMB_MODE ) 
	{
		if ( TVB380_SET_MODULATOR_PRBS_MODE(modulator_type, mode) == 0 )
		{
			return TVB380_SET_MODULATOR_PRBS_SCALE(modulator_type, scale);
		}
		LldPrint_Error("tvb360 : pos...", 1, 38);
		return -1;
	}

	if ( mode < 0 || mode > 4 )
	{
		LldPrint_Error("tvb360 : pos...", 1, 39);
		return -1;
	}
	
	double scale_offset = 0;
	if ( modulator_type == TVB380_DVBT_MODE ) scale_offset = -0.8;
	//2012/6/27 multi dvb-t
	else if ( modulator_type == TVB380_MULTI_DVBT_MODE ) scale_offset = -0.8;
	else if ( modulator_type == TVB380_VSB8_MODE ) scale_offset = -3.0;
	else if ( modulator_type == TVB380_QAMA_MODE ) scale_offset = -10.0;	//2011/8/2 -3.0 ==> -10.0
	else if ( modulator_type == TVB380_QAMB_MODE ) scale_offset = -10.0;	//2011/8/2 -3.0 ==> -10.0
	else if ( modulator_type == TVB380_QPSK_MODE ) scale_offset = 0.0;
	else if ( modulator_type == TVB380_TDMB_MODE ) scale_offset = -7.3;
	else if ( modulator_type == TVB380_VSB16_MODE ) scale_offset = -3.0;
	else if ( modulator_type == TVB380_DVBH_MODE ) scale_offset = -0.8;
	else if ( modulator_type == TVB380_DVBS2_MODE ) 
	{
		if ( gRRC_Filter == ROLL_OFF_FACTOR_NONE )
			scale_offset = 0.0;
		else
			scale_offset = -3.0;
	}
	else if ( modulator_type == TVB380_ISDBT_MODE ) scale_offset = -12.8;
	else if ( modulator_type == TVB380_ISDBT_13_MODE ) scale_offset = -1.5;
	else if ( modulator_type == TVB380_DTMB_MODE ) scale_offset = -3.0;
	else if ( modulator_type == TVB380_CMMB_MODE ) scale_offset = -0.8;
	else if ( modulator_type == TVB380_ATSC_MH_MODE ) scale_offset = -3.0;
	else if ( modulator_type == TVB380_DVBT2_MODE ) scale_offset = -0.8;
	else if ( modulator_type == TVB380_DVBC2_MODE ) scale_offset = -0.8;
	//ISDB-S ???
	else if ( modulator_type == TVB380_ISDBS_MODE ) scale_offset = 0.0;
	else if ( modulator_type == TVB380_MULTI_VSB_MODE ) scale_offset = -3.0;
	else if ( modulator_type == TVB380_MULTI_QAMB_MODE ) scale_offset = -10.0;	//2011/8/2 -3.0 ==> -10.0
	scale += scale_offset;
	if ( scale < 0 ) scale = 0;

	//scale : CNR
	if ( scale < 0 || scale > 50 )
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > MAX_CNR ) scale = MAX_CNR;
	}

	double dwCNR = scale;
	double dwCarrierPower = 0;
	if ( modulator_type == TVB380_DVBT_MODE )			dwCarrierPower = 5299348;
	//2012/6/27 multi dvb-t
	else if ( modulator_type == TVB380_MULTI_DVBT_MODE )			dwCarrierPower = 5299348;
	else if ( modulator_type == TVB380_VSB8_MODE )		dwCarrierPower = 5169951;
	else if ( modulator_type == TVB380_QAMA_MODE )		dwCarrierPower = 5968774;
	else if ( modulator_type == TVB380_QAMB_MODE )		dwCarrierPower = 5968774;
	else if ( modulator_type == TVB380_QPSK_MODE )		dwCarrierPower = 9680000;
	else if ( modulator_type == TVB380_TDMB_MODE )		dwCarrierPower = 5368737;
	else if ( modulator_type == TVB380_VSB16_MODE )		dwCarrierPower = 5169951;
	else if ( modulator_type == TVB380_DVBH_MODE )		dwCarrierPower = 5299348;
	else if ( modulator_type == TVB380_DVBS2_MODE )		dwCarrierPower = 9106853;
	else if ( modulator_type == TVB380_ISDBT_MODE )		dwCarrierPower = 5497700;
	else if ( modulator_type == TVB380_ISDBT_13_MODE )	dwCarrierPower = 5276337;
	else if ( modulator_type == TVB380_DTMB_MODE )		dwCarrierPower = 3252709;
	else if ( modulator_type == TVB380_CMMB_MODE )		dwCarrierPower = 5299348;
	else if ( modulator_type == TVB380_ATSC_MH_MODE )	dwCarrierPower = 5169951;
	else if ( modulator_type == TVB380_DVBT2_MODE )		dwCarrierPower = 5299348;
	else if ( modulator_type == TVB380_DVBC2_MODE )		dwCarrierPower = 5299348;
	//ISDB-S ???
	else if ( modulator_type == TVB380_ISDBS_MODE )		dwCarrierPower = 5497700;
	else if ( modulator_type == TVB380_MULTI_VSB_MODE )		dwCarrierPower = 5169951;
	else if ( modulator_type == TVB380_MULTI_QAMB_MODE )		dwCarrierPower = 5968774;
	else
		;
	//SCALE
	scale = pow((dwCarrierPower / 2.), 0.5) * (1. / pow(10.,(dwCNR / 20.)));

	//DVB-T2 - 552 * 3.5 => 552 * 2.75
	if (( modulator_type == TVB380_DVBT2_MODE ))
	{
		scale = (552. * 2.75)/(pow(10,(dwCNR/20.)));
	}
	//2011/6/3 DVB-C2
	else if( modulator_type == TVB380_DVBC2_MODE )
	{
		scale = 909. / (pow(10,(dwCNR/20.)));
	}
	//CMMB
	else if ( modulator_type == TVB380_CMMB_MODE )
	{
		scale = ((1050./sqrt(2.)) * 2.)/(pow(10,(dwCNR/20.)));
	}

	if ( scale < 0 || scale > 8000)
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > 8000 ) scale = 8000;
	}

	val = (mode << 16);
	val += (int)scale;

	//
	// download PRBS mode and scale
	//
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_PRBS_INFO);

		dwStatus = (((dwStatus>>19)&0x1FFF)<<19) + val;
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_INFO, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 40);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_PRBS_INFO, val))
		{
			LldPrint_Error("tvb360 : pos...", 1, 41);
			return -1;
		}
	}

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	return 0;
}

//EXT. ATTEN
#if defined(WIN32)
#include "../include/USBLibrary.h"

#define MSG_SIZE 1000

typedef struct _DEVICE {
	char *name;
	unsigned int number;
	unsigned int txrate;
	struct _DEVICE *next;
	char active;
} *PDEVICE, DEVICE;
static PDEVICE ptDevice = (PDEVICE) 0;
static PDEVICE listOfDevice = (PDEVICE) 0;
//2012/2/1 TAT4710
CFCPipeUSB pipe;
static int openPipe = 0;
static unsigned int deviceNumber = 0;
#endif


int CLldTvb360u::ScanTAT4710(void)
{
	int nTAT4710 = 0;
	unsigned int nbConnectedDevices, /*nbActiveDevice,*/ i;
	char ** strConnectedDevices;
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("ScanTAT4710", 0, 0);
#ifdef WIN32
	nbConnectedDevices = GetUsbDeviceListName(&strConnectedDevices);
	if(nbConnectedDevices == 0)
	{
		if(ptDevice)
		{
			if(openPipe == 1)
			{
				pipe.Close();
				openPipe = 0;
			}
			free(ptDevice);
			ptDevice = NULL;
			return 0;
		}
	}
	if ( !ptDevice )
	{
 		//nbConnectedDevices = GetUsbDeviceListName(&strConnectedDevices);
		for ( i = 0; i < nbConnectedDevices; ++i) 
		{
			// Search if the device has been detected
			for (ptDevice = listOfDevice; ptDevice; ptDevice = ptDevice->next) 
			{
				if (!strcmp(ptDevice->name, strConnectedDevices[i])) 
				{
					// If the device has been stoppped, restart it
					if (ptDevice->active == 'N') 
					{
						ptDevice->active = 'Y';
					}
					break;
				}
			}

			// If the device has not been registered, register it and create a new task
			if (!ptDevice) 
			{
				ptDevice = (PDEVICE) malloc(sizeof(DEVICE));
				ptDevice->name = _strdup(strConnectedDevices[i]);
				ptDevice->txrate = 0;
				ptDevice->number = deviceNumber++;
				ptDevice->next = listOfDevice;
				ptDevice->active = 'Y';
				listOfDevice = ptDevice;
			}
		}
	}

	if ( ptDevice != (PDEVICE) 0 )
	{
		nTAT4710 = 1;
		if(openPipe == 0)
		{
			if (pipe.Open(ptDevice->name))
			{
				LldPrint_Error("Fail to open pipe", 1, 43);
				return 0;
			}
			openPipe = 1;
		}
	}
#endif

	return nTAT4710;
}
void CLldTvb360u::CloseTAT4710(void)
{
#ifdef WIN32
	if(openPipe == 1)
	{
		pipe.Close();
	}
	if ( ptDevice != (PDEVICE) 0 )
	{
		LldPrint_2("CLOSE TAT4710", 0, 0);
		free(ptDevice);
		ptDevice = NULL;
	}
#endif
}

int CLldTvb360u::SetTAT4710(double atten_value)
{
	int ret;
	double atten = atten_value;

	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("SetTAT4710", 0, 0);
#if defined(WIN32)
//	CFCPipeUSB pipe;
	unsigned long  msgLength=MSG_SIZE, msgWritten;
	char txMsg[MSG_SIZE], rxMsg[MSG_SIZE];
	memset(txMsg, 0x00, MSG_SIZE);

	if ( atten < MIN_ATTEN )		atten = MIN_ATTEN;
	else if ( atten > MAX_TAT4710 ) atten = MAX_TAT4710;

	// make data buffer
	if ( atten >= 64.0 )	{ txMsg[7] = txMsg[6] = '1';	atten -= 64.0; }
	if ( atten >= 32.0 )	{ txMsg[5] = '1';	atten -= 32.0; }
	if ( atten >= 16.0 )	{ txMsg[4] = '1';	atten -= 16.0; }
	if ( atten >= 8.0 )		{ txMsg[3] = '1';	atten -= 8.0; }
	if ( atten >= 4.0 ) 	{ txMsg[2] = '1';	atten -= 4.0; }
	if ( atten >= 2.0 )		{ txMsg[1] = '1';	atten -= 2.0; }
	if ( atten >= 1.0 ) 	{ txMsg[0] = '1';	atten -= 1.0; }

	// create device handle
	if ( !ptDevice )
	{
		unsigned int nbConnectedDevices, /*nbActiveDevice,*/ i;
		char ** strConnectedDevices;
 		nbConnectedDevices = GetUsbDeviceListName(&strConnectedDevices);
		for ( i = 0; i < nbConnectedDevices; ++i) 
		{
			// Search if the device has been detected
			for (ptDevice = listOfDevice; ptDevice; ptDevice = ptDevice->next) 
			{
				if (!strcmp(ptDevice->name, strConnectedDevices[i])) 
				{
					// If the device has been stoppped, restart it
					if (ptDevice->active == 'N') 
					{
						ptDevice->active = 'Y';
					}
					break;
				}
			}

			// If the device has not been registered, register it and create a new task
			if (!ptDevice) 
			{
				ptDevice = (PDEVICE) malloc(sizeof(DEVICE));
				ptDevice->name = _strdup(strConnectedDevices[i]);
				ptDevice->txrate = 0;
				ptDevice->number = deviceNumber++;
				ptDevice->next = listOfDevice;
				ptDevice->active = 'Y';
				listOfDevice = ptDevice;
			}
		}
	}
	if ( ptDevice == (PDEVICE) 0 ) 
	{
		LldPrint_Error("tvb360 : pos...", 1, 42);
		return -1;
	}
	if ( ptDevice != (PDEVICE) 0 )
	{
		if(openPipe == 0)
		{
			if (pipe.Open(ptDevice->name))
			{
				LldPrint_Error("Fail to open pipe", 1, 43);
				return -1;
			}
			openPipe = 1;
		}
	}
//for(int i = 0; i < 3 ; i++)
//{
	Sleep(200L);

	// apply 
	// open the device
//	if (pipe.Open(ptDevice->name))
//	{
//		LldPrint_Error("tvb360 : pos...", 1, 43);
//		LldPrint_Error("[TAT4710 ERROR] ",43, GetLastError());
		//return -1;
//		return -1;
//	}
	//if (pipe.WritePipe(txMsg, msgLength, &msgWritten) || (msgLength != msgWritten)) 
	ret = pipe.WritePipe(txMsg, msgLength, &msgWritten);
	if (ret != 0 || (msgLength != msgWritten)) 
	{
		LldPrint_Error("tvb360 : pos...", 1, 44);
		LldPrint_Error("[TAT4710 ERROR 44] ",0, msgWritten);
		//return -1;
//		pipe.Close();
		return -1;
	}
//	LldPrint_Error("[TAT4710 ERROR 44] ",ret, msgWritten);
	//if (pipe.ReadPipe (rxMsg, msgLength))
	ret = pipe.ReadPipe (rxMsg, msgLength);
	if(ret != 0)
	{
		LldPrint_Error("tvb360 : pos...", 1, 45);
		//return -1;
//		pipe.Close();
		return -1;
	}
//	LldPrint_Error("[TAT4710 ERROR 45] ",0, GetLastError());
	Sleep(200L);

	// close handle
//	pipe.Close();
//}
#endif

	return 0;
}

//2011/6/29 added UseTAT4710
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_AGC(long modulator_type, long agc_on_off, long UseTAT4710)
{
	LldPrint_Trace("TVB380_SET_MODULATOR_AGC", modulator_type, agc_on_off, (double)0, NULL);
	if(UseTAT4710 == 2 && ScanTAT4710())
	{
		SetTAT4710(0);
		return 0;
	}

	gEnableTAT4710 = UseTAT4710;
	//2011/6/29 added UseTAT4710
	if(UseTAT4710 == 1)
		gCurrentTAT4710 = ScanTAT4710();
	//gCurrentTAT4710 = 1;
	gAGC_On_Off = agc_on_off;
	return 0;
}

//2011/6/29 added UseTAT4710
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_ATTEN_VALUE(long modulator_type, double atten_value, long UseTAT4710)
{
	double atten = atten_value;
	int dwData = 0;
	gEnableTAT4710 = UseTAT4710;
	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual() || modulator_type == TVB380_TSIO_MODE)
	{
		return 0;
	}


	LldPrint_FCall("TVB380_SET_MODULATOR_ATTEN_VALUE", modulator_type, (int)atten_value);

	//fixed - called after RF/SYMBOL set
	if ( gCenter_Freq == 1000000 && gSymbol_Rate == 1000000 ) 
	{
		return 0;
	}

	//EXT. ATTEN
	DBG_PLATFORM_BRANCH();
#if defined(WIN32)
	if ( modulator_type == -1 )
	{
		//AGC
		//2011/6/29 added UseTAT4710
		if(UseTAT4710 == 1)
			SetTAT4710(atten_value);
		return 0;
	}
#endif
	
	if ( atten < MIN_ATTEN ) 
		atten = MIN_ATTEN;
	else if ( atten > MAX_ATTEN ) 
		atten = MAX_ATTEN;

	//AGC
	double dwAGC_Level_Offset = 0., dwAGC_Level_Max = 0., dwAGC_Level_Min = 0., RM2=0., dwDiff = 0.;
	RM2 = CalcRFLevel(modulator_type, gCenter_Freq, gAGC_On_Off, &dwAGC_Level_Min, &dwAGC_Level_Max, &dwAGC_Level_Offset);
	gAGC_AttenOffset = dwAGC_Level_Offset;
	gAGC_MaxAtten = MAX_ATTEN - gAGC_AttenOffset;
	gAGC_MinAtten = MIN_ATTEN;
	if ( atten < gAGC_MinAtten )
	{
		atten = gAGC_MinAtten;
	}
	else if ( atten > gAGC_MaxAtten ) 
	{
		atten = gAGC_MaxAtten;
	}
	else
	{
		atten += dwAGC_Level_Offset;
	}
	gAGC_CurrentAtten = atten;
	gAGC_CurrentLevel = -(dwAGC_Level_Min+atten_value);
	if ( gAGC_On_Off == 0 )
	{
		gAGC_CurrentLevel = RM2 - atten_value;
	}

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
	}
	else
	{
		//CMMB, DVB-T2
		if ( modulator_type == TVB380_CMMB_MODE || modulator_type == TVB380_DVBT2_MODE )
		{
			//gAGC_CurrentLevel -= 8;
			gAGC_CurrentLevel -= 2;
		}
	}
	
	//AGC
	//fixed - gCurrentTAT4710 -> gCurrentTAT4710 > 0
	if ( gCurrentTAT4710 > 0 && UseTAT4710 == 1)
	{
		dwDiff = atten_value-gAGC_MaxAtten;
		if ( dwDiff > 0 )
		{
			//2011/6/29 added UseTAT4710
			if(UseTAT4710 == 1)
				SetTAT4710(dwDiff);
			gAGC_CurrentAtten += dwDiff;
		}
		//fixed
		else
		{
			if(UseTAT4710 == 1)
				SetTAT4710(0);
		}
		gAGC_CurrentLevel -= MAX_TAT4710_LOSS;
		gAGC_MaxAtten += MAX_TAT4710;
	}
	
	if ( atten >= 16.0 )
	{
		dwData += ( 1 << 5 );	atten -= 16.0;
	}
	if ( atten >= 8.0 )
	{
		dwData += ( 1 << 4 );	atten -= 8.0;
	}
	if ( atten >= 4.0 )
	{
		dwData += ( 1 << 3 );	atten -= 4.0;
	}
	if ( atten >= 2.0 )
	{
		dwData += ( 1 << 2 );	atten -= 2.0;
	}
	if ( atten >= 1.0 )
	{
		dwData += ( 1 << 1 );	atten -= 1.0;
	}
	if ( atten >= 0.5 )
	{
		dwData += ( 1 << 0 );	atten -= 0.5;
	}

	if ( IsAttachedBdTyp_AttenTyp_AssignNewBitsPos() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);

		dwStatus = (((dwStatus>>7)&0x1FFFFFF)<<7) + (dwData<<1) + (dwStatus&0x01);
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 46);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, dwData))
		{
			LldPrint_Error("tvb360 : pos...", 1, 47);
			return -1;
		}
	}

	if (IsAttachedBdTyp_594())	
	{
//		return TVB594_RESET(1);
	}

#if 1
	g_ScaleOffset = (atten_value - (int)atten_value)*10.;
	if ( g_ScaleOffset >= 5.)
	{
		g_ScaleOffset -= 5.;
	}
	g_ScaleOffset *= AD9857_SCALE_OFFSET;
	if (g_ScaleOffset > 0)
	{
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			g_ScaleOffset /= AD9857_SCALE_OFFSET;

			dwData = 0;//File gain value
			if ( modulator_type == TVB380_QPSK_MODE || modulator_type == TVB380_DVBS2_MODE )
			{
				if ( g_ScaleOffset > 0 && g_ScaleOffset <= 1 )		dwData = 0x17;
				else if ( g_ScaleOffset > 1 && g_ScaleOffset <= 2 ) dwData = 0x2F;
				else if ( g_ScaleOffset > 2 && g_ScaleOffset <= 3 ) dwData = 0x46;
				else if ( g_ScaleOffset > 3 && g_ScaleOffset <= 4 ) dwData = 0x5C;
				else if ( g_ScaleOffset > 4 && g_ScaleOffset < 5 )	dwData = 0x73;
				else if ( g_ScaleOffset > 5 && g_ScaleOffset <= 6 ) dwData = 0x89;
				else if ( g_ScaleOffset > 6 && g_ScaleOffset <= 7 ) dwData = 0x9F;
				else if ( g_ScaleOffset > 7 && g_ScaleOffset <= 8 ) dwData = 0xB4;
				else if ( g_ScaleOffset > 8 && g_ScaleOffset <= 9 ) dwData = 0xCA;
				else if ( g_ScaleOffset > 9 && g_ScaleOffset < 10 )	dwData = 0xDF;
			}
			else
			{
				if ( g_ScaleOffset > 0 && g_ScaleOffset <= 1 )		dwData = 0x09;
				else if ( g_ScaleOffset > 1 && g_ScaleOffset <= 2 ) dwData = 0x11;
				else if ( g_ScaleOffset > 2 && g_ScaleOffset <= 3 ) dwData = 0x1A;
				else if ( g_ScaleOffset > 3 && g_ScaleOffset <= 4 ) dwData = 0x23;
				else if ( g_ScaleOffset > 4 && g_ScaleOffset < 5 )	dwData = 0x2B;
				else if ( g_ScaleOffset > 5 && g_ScaleOffset <= 6 ) dwData = 0x33;
				else if ( g_ScaleOffset > 6 && g_ScaleOffset <= 7 ) dwData = 0x3B;
				else if ( g_ScaleOffset > 7 && g_ScaleOffset <= 8 ) dwData = 0x44;
				else if ( g_ScaleOffset > 8 && g_ScaleOffset <= 9 ) dwData = 0x4C;
				else if ( g_ScaleOffset > 9 && g_ScaleOffset < 10 )	dwData = 0x54;
			}
			TSPL_SET_AD9775(0x05, (long)dwData);
			TSPL_SET_AD9775(0x09, (long)dwData);
		}
		else if ( IsAttachedBdTyp_AttenTyp_4() )
		{
		}
		else
		{
			CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
		}

		if ( IsAttachedBdTyp_AttenTyp_1() && (modulator_type == TVB380_ISDBT_13_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		if ( IsAttachedBdTyp_AttenTyp_2() && (modulator_type == TVB380_ATSC_MH_MODE
			&& (CurrentTSSrc == 4/*TSIO_310_CAPTURE_PLAY*/ || CurrentTSSrc == 5/*TSIO_ASI_CAPTURE_PLAY*/)) )
		{
			return 0;
		}
		if ( IsAttachedBdTyp_AttenTyp_3() && (modulator_type == TVB380_ISDBS_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		//2012/7/11 DVB-T2 ASI
		if ( IsAttachedBdTyp_AttenTyp_3() && ((modulator_type == TVB380_DVBT2_MODE || modulator_type == TVB380_DVBC2_MODE) && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}

		ResetModBlkAfterChangingHwDacPara(3);
	}
#endif

	//DVB-T2 - 1024 * 3.5 = 3584 = 0xE00 =>  1024 * 2.75 = 0xB00 => 600
	if ( modulator_type == TVB380_DVBT2_MODE )
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_MODULATOR_DVB_T2_GAIN_CTRL, 0x600);
	}
	else if (modulator_type == TVB380_DVBC2_MODE)
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_MODULATOR_DVB_T2_GAIN_CTRL, 0x600);
	}
	//CMMB - 0x800
	else if ( modulator_type == TVB380_CMMB_MODE )
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_MODULATOR_DVB_T2_GAIN_CTRL, 0x800);
	}

	return 0;
}

//=================================================================	
// DVB_H
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_DVBH(long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id)
{
	//TVB593
	unsigned long dwStatus;

	LldPrint_6("TVB380_SET_MODULATOR_DVBH", modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);

	unsigned long mode = (cell_id<<16) + 
		(mpe_fec<<4) + 
		(time_slice<<3) + 
		(in_depth_interleave<<2) + 
		tx_mode;

	if ( modulator_type != TVB380_DVBH_MODE && modulator_type != TVB380_DVBT_MODE && modulator_type != TVB380_MULTI_DVBT_MODE )
	{
		LldPrint_Error("tvb360 : pos...", 1, 48);
		return -1;
	}

	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		if(IsAttachedBdTyp__Virtual())
		{
			TVB380_SET_MODULATOR_CELLID_VirtualBd_n(modulator_type, cell_id);
			return 0;
		}
	}

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		if ( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_MULTI_DVBT_MODE )
		{
			tx_mode = ((tx_mode != 0) ? 2 : 0);
		}
		else if ( modulator_type == TVB380_DVBH_MODE )
		{
		}
		else
		{
			LldPrint_Error("tvb360 : pos...", 1, 202);
			return -1;
		}
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_DVB_H_MODE_CNTL);

		dwStatus = (cell_id<<16) + (mpe_fec<<15) + (time_slice<<14) + (in_depth_interleave<<13) + (((dwStatus>>4)&0x1FF)<<4) + (tx_mode<<2) + (dwStatus&0x3);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_H_MODE_CNTL, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 49);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_H_MODE_CNTL, mode))
		{
			LldPrint_Error("tvb360 : pos...", 1, 50);
			return -1;
		}
	}

#if 1
	if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_mod_blk_);
	}
	else
#endif
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);

	return 0;
}

//=================================================================	
// DVB_S2
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_PILOT(long modulator_type, long  pilot_on_off)
{
	//TVB593
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_PILOT", modulator_type, pilot_on_off, 0);

	if ( modulator_type != TVB380_DVBS2_MODE )
	{
		LldPrint_Error("tvb360 : pos...", 1, 51);
		return -1;
	}

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_DVB_S2_PILOT_CNTL);

		dwStatus = (((dwStatus>>9)&0x7FFFFF)<<9) + (pilot_on_off<<8) + (dwStatus&0xFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_S2_PILOT_CNTL, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 52);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_S2_PILOT_CNTL, pilot_on_off))
		{
			LldPrint_Error("tvb360 : pos...", 1, 53);
			return -1;
		}
	}

	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST_sub_opt__mod_n_blk_);
	RESET_PLAY_RATE();

	return 0;
}

//DVB-S2 : Roll-off factor added
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(long modulator_type, long  roll_off_factor)//0(0.20), 1(0.25), 2(0.35), 3(none)
{
	//TVB593
	unsigned long dwStatus;

	LldPrint("TVB380_SET_MODULATOR_ROLL_OFF_FACTOR", modulator_type, roll_off_factor, 0);

	if ( modulator_type == TVB380_QPSK_MODE )
	{
		roll_off_factor = ((roll_off_factor != 0) ? 1 : ROLL_OFF_FACTOR_NONE);
		if ( roll_off_factor != (long)gRRC_Filter )
		{
			if ( IsAttachedBdTyp_UseIndirectRegAccess() )
			{
				dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_DVB_S2_ROLL_OFF_FACTOR_CNTL);

				dwStatus = (((dwStatus>>8)&0xFFFFFF)<<8) + (roll_off_factor<<7) + (dwStatus&0x7F);
				if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_S2_ROLL_OFF_FACTOR_CNTL, dwStatus))
				{
					LldPrint_Error("tvb360 : pos...", 1, 54);
					return -1;
				}

				gRRC_Filter = roll_off_factor;
			}
			else
			{

				gRRC_Filter = roll_off_factor;
				CntlClkGen_AD9852_Symclock(modulator_type, gSymbol_Rate);
				if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
				{
				}
				else
				{
					CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
				}
				ResetModBlkAfterChangingHwDacPara(3);

			//TVB593
			}
		}
		return 0;
	}

	if ( modulator_type != TVB380_DVBS2_MODE )
	{
		LldPrint_Error("tvb360 : pos...", 1, 55);
		return -1;
	}

	roll_off_factor &= 0x00000003;

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_DVB_S2_ROLL_OFF_FACTOR_CNTL);

		dwStatus = (((dwStatus>>11)&0x1FFFFF)<<11) + (roll_off_factor<<9) + (dwStatus&0x1FF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_S2_ROLL_OFF_FACTOR_CNTL, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 56);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DVB_S2_ROLL_OFF_FACTOR_CNTL, roll_off_factor))
		{
			LldPrint_Error("tvb360 : pos...", 1, 57);
			return -1;
		}
	}

	if ( roll_off_factor != (long)gRRC_Filter )
	{
		gRRC_Filter = roll_off_factor;
		if ( IsAttachedBdTyp_UseAD9775() )
		{
			//2012/7/20 RRC FILTER OFF
			if(modulator_type == TVB380_DVBS2_MODE && (IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2()))
			{
				CntlClkGen_AD9852_Symclock(modulator_type, gSymbol_Rate);
				CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
			}
			else if(modulator_type == TVB380_DVBS2_MODE && (IsAttachedBdTyp_599() || IsAttachedBdTyp_598()))
			{
				CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
			}
		}
		else
		{
			CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
		}
	}

	ResetModBlkAfterChangingHwDacPara(3);
	RESET_PLAY_RATE();
	
	return 0;
}

// TVB595V1
/*^^***************************************************************************
 * Description : Set the installed board's status - RF/IF AMP is used(1) or not(0)
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB590, TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldTvb360u::TVB380_SET_BOARD_CONFIG_STATUS(long modulator_type, long status)
{
	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LldPrint("TVB380_SET_BOARD_CONFIG_STATUS", modulator_type, status, 0);

	//fixed - called after RF/SYMBOL set
	if ( gCenter_Freq == 1000000 && gSymbol_Rate == 1000000 ) 
	{
		return 0;
	}
	
	//TVB590V9.2, V10.0, TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_NewAmpBypassModeCntl() )	//V1.1 or higher
	{
		status = (status + 1) % 2;
	}

	//fixed
	//if ( gBypass_AMP == 0 || (unsigned long)(status+1) != gBypass_AMP )
	{
		gBypass_AMP = (status+1);

		if ( IsAttachedBdTyp_AttenTyp_AssignNewBitsPos() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_BOARD_CONFIG_INFO);

			dwStatus = (((dwStatus>>1)&0x7FFFFFFF)<<1) + status;
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_BOARD_CONFIG_INFO, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 58);
				return -1;
			}
		}
		else	
		{
			TSPL_WRITE_CONTROL_REG(0, TSP_BOARD_CONFIG_INFO, status+2);//fixed, status -> status+2
		}
		Sleep(10);

		if ( (IsAttachedBdTyp_AttenTyp_1() || IsAttachedBdTyp_499()) && 
			(modulator_type == TVB380_ISDBT_13_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		if ( (IsAttachedBdTyp_AttenTyp_2() || IsAttachedBdTyp_499()) 
			&& (modulator_type == TVB380_ATSC_MH_MODE 
			&& (CurrentTSSrc == 4/*TSIO_310_CAPTURE_PLAY*/ || CurrentTSSrc == 5/*TSIO_ASI_CAPTURE_PLAY*/)) )
		{
			return 0;
		}

		//2011/6/15 ISDB-S ASI
		if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
			(modulator_type == TVB380_ISDBS_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		//2012/7/11 DVB-T2 ASI
		if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
			((modulator_type == TVB380_DVBT2_MODE || modulator_type == TVB380_DVBC2_MODE) && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		//ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
		//ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__ts_io_blk_);
	}

	return 0;
}

/*^^***************************************************************************
 * Description : Get RF power level information
 *				
 * Entry : type = 0(Max. Level), 1(Min. Level), 2(Current Level), 3(Current Atten), 4(Current TAT4710 Atten.)
 *
 * Return: 
 *
 * Notes :  only for ATTN. control supported boards (ID=44, 45, 47, 59)
 *
 **************************************************************************^^*/
double _stdcall CLldTvb360u::TVB380_GET_MODULATOR_RF_POWER_LEVEL(long modulator_type, long info_type)
{
	int nRFPower = 0;
	
	LldPrint_Trace("TVB380_GET_MODULATOR_RF_POWER_LEVEL", modulator_type, info_type, (double)0, NULL);
	if ( info_type == 0 )
		return gMaxLevel;
	else if ( info_type == 1 )
		return gMinLevel;
	else if ( info_type == 2 )
		return gCurrentLevel;
	else if ( info_type == 3 )
		return gCurrentAtten;
	else if ( info_type == 4 )
		return gCurrentTAT4710;//-1:no board installed
	//AGC
	else if ( info_type == 5 )
		return gAGC_MaxAtten;
	else if ( info_type == 6 )
		return gAGC_MinAtten;
	else if ( info_type == 7 )
		return gAGC_AttenOffset;
	else if ( info_type == 8 )
		return gAGC_CurrentAtten;
	else if ( info_type == 9 )
		return gAGC_CurrentLevel;
	return -1;
}

/*^^***************************************************************************
 * Description : Set the BERT SERIAL stream measurement
 *				
 * Entry : type = 0(normal), 1(all 0's), 2(all 1's), 3(PRBS 2^15-1), 4(PRBS 2^23-1)
 *
 * Return: 
 *
 * Notes :  only for DVB-T, 8VSB, QAM-A,B, QPSK, ISDB-T 1,13 SEG.
 *			1(all 0's), 2(all 1's) reserved
 *
 **************************************************************************^^*/
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_BERT_MEASURE(long modulator_type, long packet_type)
{
	//TVB593
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		if (IsAttachedBdTyp_SupportMultiRfOut_593_591s())
		{
//			return	TVB380_SET_MODULATOR_BERT_GEN_VirtualBd_n(modulator_type, mode, scale);
		}
	}
	LldPrint("TVB380_SET_MODULATOR_BERT_MEASURE", modulator_type, packet_type, 0);
	if ( modulator_type == TVB380_DVBT_MODE 
		|| modulator_type == TVB380_VSB8_MODE
		|| modulator_type == TVB380_QAMA_MODE
		|| modulator_type == TVB380_QAMB_MODE
		|| modulator_type == TVB380_QPSK_MODE
		|| modulator_type == TVB380_DVBH_MODE
		|| modulator_type == TVB380_CMMB_MODE
		|| modulator_type == TVB380_ATSC_MH_MODE
		//DVB-T2
		|| modulator_type == TVB380_MULTI_VSB_MODE
		|| modulator_type == TVB380_MULTI_QAMB_MODE
		|| modulator_type == TVB380_MULTI_DVBT_MODE) //2012/6/27 multi dvb-t
	{
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_BERT_STREAM_MEASUREMENT);

			dwStatus = (((dwStatus>>23)&0x1FF)<<23) + (packet_type<<20) + (dwStatus&0xFFFFF);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_BERT_STREAM_MEASUREMENT, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 59);
				return -1;
			}
		}
		else
		{
			WDM_WR2SDRAM_ADDR(TSP_MEM_BERT_STREAM_MEASUREMENT, packet_type);
		}
	}
	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_DTMB(long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion)
{
	int data = 0;
	unsigned long dwStatus;

	LldPrint_8("TVB380_SET_MODULATOR_DTMB", modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);

	if ( modulator_type != TVB380_DTMB_MODE 
		|| (constellation < CONST_DTMB_4QAM_NR || constellation >= CONST_DTMB_MAX)
		|| (code_rate < CONST_DTMB_CODE_7488_3008 || code_rate >= CONST_DTMB_CODE_MAX)
		|| (interleaver < CONST_DTMB_INTERLEAVE_0 || interleaver >= CONST_DTMB_INTERLEAVE_MAX)
		|| (frame_header < CONST_DTMB_FRAME_HEADER_MODE_1 || frame_header >= CONST_DTMB_FRAME_HEADER_MODE_MAX)
		|| (carrier_number < CONST_DTMB_CARRIER_NUMBER_0 || carrier_number >= CONST_DTMB_CARRIER_NUMBER_MAX)
		|| (frame_header_pn < CONST_DTMB_FRAME_HEADER_PN_FIXED || frame_header_pn >= CONST_DTMB_FRAME_HEADER_PN_MAX)
		|| (pilot_insertion < CONST_DTMB_PILOT_INSERTION_OFF || pilot_insertion >= CONST_DTMB_PILOT_INSERTION_MAX))
	{
		LldPrint_Error("tvb360 : pos...", 1, 60);
		return -1;
	}

	data = constellation;
	data += (code_rate << 3 );
	data += (interleaver << 5 );
	data += (frame_header << 6 );
	data += (carrier_number << 8 );
	data += (frame_header_pn << 9 );
	data += (pilot_insertion << 10 );

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_MODULATOR_DTMB);

		dwStatus = (((dwStatus>>13)&0x7FFFF)<<13) + (data<<2) + (dwStatus&0x3);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MODULATOR_DTMB, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 61);
			return -1;
		}

		TSPL_RESET_SDCON_backwardcompatible(4);
	}
	else
	{
		TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MODULATOR_DTMB, data);
	}
	return 0;
}

int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_SDRAM_CLOCK(long modulator_type, long sdram_clock)
{
	int data = 0;
	unsigned long dwStatus;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint("TVB380_SET_MODULATOR_SDRAM_CLOCK", modulator_type, sdram_clock, 0);

	data = modulator_type;
	if(modulator_type == TVB380_DVBC2_MODE)
		data = modulator_type - 5;

	//I/Q PLAY/CAPTURE
    if ( modulator_type == TVB380_IQ_PLAY_MODE )
	{
		data = 1;
	}

	data += (sdram_clock<<4);
	data += (1<<7);
	
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		WDM_WR2SDRAM_ADDR(PCI590S_REG_SDRAM_CLOCK_SELECTION, data);
	}
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		if(IsAttachedBdTyp_599() || IsAttachedBdTyp_598())
		{
			sdram_clock = sdram_clock + 4;
		}
		data = (sdram_clock<<8) + (modulator_type&0xFF);
		WDM_WR2SDRAM_ADDR(PCI593_REG_SDRAM_CLOCK_SELECTION, data);
	}
	else
	{
		WDM_WR2SDRAM(data);
	}

	if ( modulator_type == TVB380_DTMB_MODE )
	{
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_DTMB_INIT_TI_SDRAM);

			dwStatus = (1<<13) + (dwStatus&0x1FFF);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DTMB_INIT_TI_SDRAM, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 62);
				return -1;
			}
			
			dwStatus = (0<<13) + (dwStatus&0x1FFF);
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_DTMB_INIT_TI_SDRAM, dwStatus))
			{
				LldPrint_Error("tvb360 : pos...", 1, 63);
				return -1;
			}
		}
		else
		{
			WDM_WR2SDRAM_ADDR(TSP_MEM_DTMB_INIT_TI_SDRAM, 1);
			WDM_WR2SDRAM_ADDR(TSP_MEM_DTMB_INIT_TI_SDRAM, 0);
		}

		Sleep(1);
	}
	if ( IsAttachedBdTyp_AllCase_w_SpecialBd())
		ResetSdram();

	return 0;
}

//TVB593 - ISDB-T, ATSC-M/H
int _stdcall CLldTvb360u::TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction)
{
	unsigned long dwStatus;

	LldPrint_FCall("TSPL_SET_DMA_DIRECTION", modulator_type, dma_direction);

//#ifdef WIN32
//	LockDmaMutex();
//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return 0;
//	}
//#endif
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_USB_DMA_DIRECTION);

		if ( modulator_type == TVB380_ISDBT_13_MODE )
		{
			dwStatus = (((dwStatus>>4)&0xFFFFFFF)<<4) + (dma_direction<<2) + (dwStatus&0x3);
		}
		//2012/7/5 DM
		else if(modulator_type == TVB380_DVBC2_MODE)
		{
			dwStatus = (((dwStatus>>31)&0x1)<<31) + (dma_direction<<29) + (dwStatus&0x1FFFFFFF);
		}
		else
		{
			dwStatus = (((dwStatus>>5)&0x7FFFFFF)<<5) + (dma_direction<<3) + (dwStatus&0x7);
		}
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_USB_DMA_DIRECTION, dwStatus))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 64);
			return -1;
		}
	}
	else
	{
		//2011/6/17 ISDB-S ASI
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(0x52000, (unsigned long)dma_direction))
			{
//#ifdef WIN32
//				UnlockDmaMutex();
//#endif
				LldPrint_Error("tvb360 : pos...", 1, 65);
				return -1;
			}
		}
		else
		{
			if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_USB_DMA_DIRECTION, (unsigned long)dma_direction))
			{
//#ifdef WIN32
//				UnlockDmaMutex();
//#endif
				LldPrint_Error("tvb360 : pos...", 1, 66);
				return -1;
			}
		}
	}

//#ifdef WIN32
//	UnlockDmaMutex();
//#endif

	return 0;
}

//TVB593 - ISDB-T
int _stdcall CLldTvb360u::TSPL_RESET_IP_CORE(long modulator_type, long reset_control)
{
	unsigned long dwStatus;

	LldPrint_FCall("TSPL_RESET_IP_CORE", modulator_type, reset_control);

//#ifdef WIN32
//	LockDmaMutex();
//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return 0;
//	}
//#endif
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ISDBT_MODULATOR_RESET);
		//dwStatus |= (reset_control<<16);
		dwStatus = (reset_control<<16) + (dwStatus&0xFFFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ISDBT_MODULATOR_RESET, dwStatus))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 67);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ISDBT_MODULATOR_RESET, reset_control))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 68);
			return -1;
		}
	}

//#ifdef WIN32
//	UnlockDmaMutex();
//#endif
	return 0;
}

//TVB593 - ATSC-M/H
int _stdcall CLldTvb360u::TSPL_SET_MH_MODE(long modulator_type, long mh_mode)
{
	unsigned long dwStatus;

	LldPrint("TSPL_SET_MH_MODE", modulator_type, mh_mode, 0);

//#ifdef WIN32
//	LockDmaMutex();
//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return 0;
//	}
//#endif

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_MH_MODE);

		dwStatus = (((dwStatus>>3)&0x3FFFFFF)<<3) + (mh_mode<<2) + (dwStatus&0x3);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MH_MODE, dwStatus))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 69);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MH_MODE, (mh_mode<<3)))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 70);
			return -1;
		}
	}

//#ifdef WIN32
//	UnlockDmaMutex();
//#endif

	return 0;
}

//TVB593 - ATSC-M/H (TVB598,599 DVB-T2)
int _stdcall CLldTvb360u::TSPL_SET_MH_PID(long modulator_type, long mh_pid)
{
	DWORD dwStatus;

	LldPrint("TSPL_SET_MH_PID", modulator_type, mh_pid, 0);

//#ifdef WIN32
//	LockDmaMutex();
//	if ( fDMA_Busy == 1 )
//	{
//		UnlockDmaMutex();
//		return 0;
//	}
//#endif

	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_MH_PID);
		if(modulator_type == TVB380_DVBT2_MODE)
			dwStatus = (1 << 31) + ((mh_pid & 0x1FFF) << 18) + (dwStatus&0x3FFFF);
		else
			dwStatus = (mh_pid<<16) + (dwStatus&0xFFFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MH_PID, dwStatus))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 71);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_MH_PID, mh_pid))
		{
//#ifdef WIN32
//			UnlockDmaMutex();
//#endif
			LldPrint_Error("tvb360 : pos...", 1, 72);
			return -1;
		}
	}

//#ifdef WIN32
//	UnlockDmaMutex();
//#endif

	return 0;
}

void 	CLldTvb360u::AssertReleaseResetSignal(unsigned int _bits)
{
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, _bits);
	}
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, _bits);
	}
}
int 	CLldTvb360u::ResetModBlkAfterChangingHwBlkPara(int _opt)
{
	LldPrint_FCall("ResetModBlkAfterChangingHwBlkPara", _opt, 0);

	if ( IsAttachedBdTyp_HaveBlkRstFunc() )
	{
	}
	else
	{
		WDM_WR2SDRAM(CARD_RESET_BOARD);
		return	0;
	}
	switch(_opt)
	{
	case	_CMD_MOD_RST__all_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_ALL_ASSERT);
		break;
	case	_CMD_MOD_RST__dta_pumping_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_DP_ASSERT);
		break;
	case	_CMD_MOD_RST__ts_io_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_TS_ASSERT);
		break;
	case	_CMD_MOD_RST__all_mod_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD_ASSERT);
		break;
	case	_CMD_MOD_RST__init_sdram_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_INIT_SDRAM);
		break;
	case	_CMD_MOD_RST__mod_1_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD1_ASSERT);
		break;
	case	_CMD_MOD_RST__mod_2_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD2_ASSERT);
		break;
	case	_CMD_MOD_RST__mod_3_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD3_ASSERT);
		break;
	case	_CMD_MOD_RST__mod_4_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD4_ASSERT);
		break;
	case	_CMD_MOD_RST__dp_1_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_DP1_ASSERT);
		break;
	case	_CMD_MOD_RST__dp_2_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_DP2_ASSERT);
		break;
	case	_CMD_MOD_RST__dp_3_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_DP3_ASSERT);
		break;
	case	_CMD_MOD_RST__dp_4_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_DP4_ASSERT);
		break;
	case	_CMD_MOD_RST__dp_mod_blk_:
		AssertReleaseResetSignal(PCI593_CARD_RESET_MOD_ASSERT | PCI593_CARD_RESET_DP_ASSERT);
		break;
	case	_CMD_MOD_RST_sub_opt__mod_n_blk_:
	default:
		if (CntAdditionalVsbRfOut_593_591s() || CntAdditionalQamRfOut_593_591s() || CntAdditionalDvbTRfOut_593())
		{
			ResetMultiModBlk_n(my_ts_id);
		}
		else
		{
			AssertReleaseResetSignal(PCI593_CARD_RESET_MOD_ASSERT | PCI593_CARD_RESET_DP_ASSERT);
		}
		break;
	}
	Sleep(100);	//	some delay
	if (IsModulatorTSLock())
	{
		LldPrint_2("ResetModBlkAfterChangingHwBlkPara - succeed ts-lock", _opt, 0);
	}
	AssertReleaseResetSignal(PCI590S_CARD_RESET_ALL_RELEASE);
	return	0;
}
int 	CLldTvb360u::ResetModBlkAfterChangingHwDacPara(int _opt)
{
	int	timeout_;

	if (IsAttachedBdTyp_594())	
	{
		return TVB594_RESET(1);
	}

	if ( IsAttachedBdTyp_HaveBlkRstFunc() )
	{
	}
	else
	{
		WDM_WR2SDRAM(CARD_RESET_BOARD);
		return	0;
	}

	for (timeout_ = 0; timeout_ < 10; timeout_++)
	{
		Sleep(10);
		if (IsLockAD9775())
		{
			ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
			break;
		}
	}
	if (timeout_ >= 10)
	{
		LldPrint_2("ResetModBlkAfterChangingHwDacPara - Lock timeout", _opt, 0);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);	//	force reset anyway.
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int 	CLldTvb360u::ResetMultiModBlk_n(int _mod_n)	//	0, 1, 2, 3
{
	unsigned long	reg;

	reg = (0x11 << (8 + _mod_n));
	WDM_WR2SDRAM_ADDR(_mRF_RESET_CTRL, reg);
	Sleep(10);
	WDM_WR2SDRAM_ADDR(_mRF_RESET_CTRL, 0);
	return	0;
}
int 	CLldTvb360u::SelMultiModAsiOut_n(int _ts_n)	//	0, 1, 2, 3
{
	unsigned long	reg;

		reg = WDM_Read_TSP_Indirectly(_mRF_OPERATION_MODE_TS_IO_CTRL);
		reg = (reg & 0xfffffcff);
		switch(_ts_n)
		{
		case	1:		reg |= (0x1 << 8);	break;
		case	2:		reg |= (0x2 << 8);	break;
		case	3:		reg |= (0x3 << 8);	break;
		case	0:
		default:
			reg |= (0x0 << 8);	break;
		}
		WDM_WR2SDRAM_ADDR(_mRF_OPERATION_MODE_TS_IO_CTRL, reg);
	return	0;
}
int 	CLldTvb360u::SelMultiMod310Out_n(int _ts_n)	//	0, 1, 2, 3
{
	unsigned long	reg;

		reg = WDM_Read_TSP_Indirectly(_mRF_OPERATION_MODE_TS_IO_CTRL);
		reg = (reg & 0xfffff3ff);
		switch(_ts_n)
		{
		case	1:		reg |= (0x1 << 10);	break;
		case	2:		reg |= (0x2 << 10);	break;
		case	3:		reg |= (0x3 << 10);	break;
		case	0:
		default:
			reg |= (0x0 << 10);	break;
		}
		WDM_WR2SDRAM_ADDR(_mRF_OPERATION_MODE_TS_IO_CTRL, reg);
	return	0;
}
int 	CLldTvb360u::SelMultiModTsOutput_n(int _ctl)
	//	0: select stored files as modulator TS source
	//	1: select external TS input as modulator TS source
{
	unsigned long	reg;

	_ctl &= 0x1;
	reg = WDM_Read_TSP_Indirectly(_mRF_OPERATION_MODE_TS_IO_CTRL);
	reg = (reg & 0xffffffef);
	reg |= (_ctl << 4);
	WDM_WR2SDRAM_ADDR(_mRF_OPERATION_MODE_TS_IO_CTRL, reg);
	ResetMultiModBlk_n(my_ts_id);	//	???
	return	0;
	}
int 	CLldTvb360u::SelMultiModOperationMode_n(int _ctl)
	//	00 : STOP
	//	01 : PLAY
	//	10 : CAPTURE
	//	11 : Reserved
{
	unsigned long	reg;

	_ctl &= 0x3;
	reg = WDM_Read_TSP_Indirectly(_mRF_OPERATION_MODE_TS_IO_CTRL);
	reg = (reg & 0xfffffffc);
	reg |= (_ctl << 0);
	WDM_WR2SDRAM_ADDR(_mRF_OPERATION_MODE_TS_IO_CTRL, reg);
	ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
#if 0
	switch(my_ts_id)
	{
	case	0:
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__mod_1_blk_);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_1_blk_);
		break;
	case	1:	
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__mod_2_blk_);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_2_blk_);
		break;
	case	2:
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__mod_3_blk_);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_3_blk_);
		break;
	case	3:
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__mod_4_blk_);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__dp_4_blk_);
		break;
	}
#endif
	return	0;
}
int	CLldTvb360u::TSPL_SET_PLAY_RATE_VirtualBd_n(
	int	_mod_n,	//	1, 2, 3
	long	play_freq_in_herz,
	long	nOutputClockSource)	// 0 for PLL clock source, 1 for 19.392 MHz oscillator
{
	unsigned long	val_, addr_;

	switch(_mod_n)
	{
	case	1:		addr_ = _mRF_PLAY_RATE_CTRL_2;		break;
	case	2:		addr_ = _mRF_PLAY_RATE_CTRL_3;		break;
	case	3:		addr_ = _mRF_PLAY_RATE_CTRL_4;		break;
	default:
		return	0;
	}
	usr_reqed_play_freq_in_herz = play_freq_in_herz;
	usr_reqed_nOutputClockSource = nOutputClockSource;

	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ = ((nOutputClockSource == 1 ? 1 : 0)<<26) + (val_&0x3FFFFFF);	//	1:max-playrate, 0:user-defined-playrate
	WDM_WR2SDRAM_ADDR(addr_, val_);

	if ((nOutputClockSource != 1) && ((nOutputClockSource == 2) || 
		((play_freq_in_herz < 19393000L) && (play_freq_in_herz > 19392000L))))	//	internal OSC
	{
		val_ = WDM_Read_TSP_Indirectly(addr_);
		val_ = (((val_>>26)&0x01)<<26) | (PCI593_EXT_OSC_SELECTION<<25) | (val_&0x1FFFFFF);
		WDM_WR2SDRAM_ADDR(addr_, val_);
	}
	else
	{
		val_ = WDM_Read_TSP_Indirectly(addr_);
		val_ = (((val_>>26)&0x01)<<26) + (PCI593_NO_EXT_OSC_SELECTION<<25) + (val_&0x1FFFFFF);
		WDM_WR2SDRAM_ADDR(addr_, val_);

		if(TSPL_SEL_DDSCLOCK_INC_VirtualBd_n(_mod_n, play_freq_in_herz) != 0)
		{
			LldPrint_Error("tvb360 : pos...", 1, 5);
			return -1;
		}
	}
	return 0;
}
int	CLldTvb360u::TSPL_SEL_DDSCLOCK_INC_VirtualBd_n(int _mod_n, long play_freq_in_herz)
{
	int		IncrementValue;
	double	dtmpVal;
	unsigned long	val_, addr_;

	switch(_mod_n)
	{
	case	1:		addr_ = _mRF_PLAY_RATE_CTRL_2;		break;
	case	2:		addr_ = _mRF_PLAY_RATE_CTRL_3;		break;
	case	3:		addr_ = _mRF_PLAY_RATE_CTRL_4;		break;
	default:
		return	0;
	}
#if 0
	if(IsAttachedBdTyp_599())
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (66./2.);
	}
	else 
#endif
	if (IsAttachedBdTyp_DdsIncNewEquation() || IsAttachedBdTyp_599() || IsAttachedBdTyp_598() )
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (38.7853169/2.);
	}
	else
	{
		dtmpVal = ((play_freq_in_herz/1000000.) / 8.) * 33554432 / (64/2.);
	}
	IncrementValue = (int)dtmpVal;

	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ = (((val_>>25)&0x7F)<<25) | (IncrementValue&0x1FFFFFF);

	WDM_WR2SDRAM_ADDR(addr_, val_);

	return 0;
}
int	CLldTvb360u::TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n)	//	0, 1, 2, 3
{
	unsigned long	val_, addr_;

	addr_ = _mRF_STREAM_TAG;	
	switch(_mod_n)	//	my_ts_id
	{
	case	3:		val_ = 3;		break;
	case	2:		val_ = 2;		break;
	case	1:		val_ = 1;		break;
	case	0:
	default:
		val_ = 0;
		break;
	}
	WDM_WR2SDRAM_ADDR(addr_, val_);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_INTERLEAVE_VirtualBd_n(long interleaving)
{
	unsigned long	val_, addr_;

	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	case	0:
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ = (interleaving << 3) | (val_ & 0x7);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	ResetMultiModBlk_n(my_ts_id);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_CONSTELLATION_VirtualBd_n(long modulator_type, long constellation)
{
	unsigned long	val_, addr_;
	long scale_factor = 0x450;
	
	if(modulator_type == TVB380_MULTI_QAMB_MODE)
	{
		for(int i = 1 ; i < 4 ; i++)
		{
			switch(i)
			{
			case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
			case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
			case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
			default:
				continue;
			}
			val_ = WDM_Read_TSP_Indirectly(addr_);
			val_ &= (~(0x1 << 2));
			val_ |= (constellation << 2);
			WDM_WR2SDRAM_ADDR(addr_, val_);
			switch(i)
			{
			case	1:		addr_ = _mRF_MODULATOR_SCALE_FACTOR_CTRL2;		break;
			case	2:		addr_ = _mRF_MODULATOR_SCALE_FACTOR_CTRL3;		break;
			case	3:		addr_ = _mRF_MODULATOR_SCALE_FACTOR_CTRL4;		break;
			default:
				continue;
			}
			TSPL_SET_CONFIG_DOWNLOAD(addr_, scale_factor);
			Sleep(1);
		}
	}
	else if(modulator_type == TVB380_MULTI_DVBT_MODE)
	{
		switch(my_ts_id)
		{
		case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
		case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
		case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
		default:
			return 0;
		}
		val_ = WDM_Read_TSP_Indirectly(addr_);
		val_ &= (~(0x3 << 4));
		val_ |= (constellation << 4);
		WDM_WR2SDRAM_ADDR(addr_, val_);
		Sleep(1);
	}
//		ResetMultiModBlk_n(i);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_NULL_TP_VirtualBd_n(long en_dis)	//	1:enable
{
	unsigned long	val_, addr_;

	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	case	0:
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0x1 << 1));
	val_ |= (en_dis << 1);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	ResetMultiModBlk_n(my_ts_id);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_DISABLE_VirtualBd_n(long _dis)	//	1:disable mod.
{
	unsigned long	val_, addr_;

	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	case	0:
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0x1 << 0));
	val_ |= (_dis << 0);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	ResetMultiModBlk_n(my_ts_id);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_PRBS_INFO_VirtualBd_n(long mod_typ, long mode, double scale)
{
	int val = 0;
	unsigned long dwStatus;
	double scale_offset = 0;
	double dwCNR;
	double dwCarrierPower;
	unsigned long	addr_;

	if ( mode < 0 || mode > 4 )
	{
		LldPrint_Error("tvb360 : pos...", 1, 101);
		return -1;
	}

	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL2;		break;
	case	2:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL3;		break;
	case	3:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL4;		break;
	case	0:
	default:
		return -1;
	}

	if ( mod_typ == TVB380_MULTI_VSB_MODE ) scale_offset = -3.0;
	else if ( mod_typ == TVB380_MULTI_QAMB_MODE ) scale_offset = -10.0;
	//2012/6/27 multi dvb-t
	else if ( mod_typ == TVB380_MULTI_DVBT_MODE ) scale_offset = -0.8;
	scale += scale_offset;
	if ( scale < 0 ) scale = 0;

	if ( scale < 0 || scale > 50 )	//	CNR
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > MAX_CNR ) scale = MAX_CNR;
	}

	dwCNR = scale;
	dwCarrierPower = 0;
	if ( mod_typ == TVB380_MULTI_VSB_MODE )		dwCarrierPower = 5169951;
	else if ( mod_typ == TVB380_MULTI_QAMB_MODE )		dwCarrierPower = 5968774;
	else if ( mod_typ == TVB380_MULTI_DVBT_MODE )			dwCarrierPower = 5299348;
	else		;
	scale = pow((dwCarrierPower / 2.), 0.5) * (1. / pow(10.,(dwCNR / 20.)));

	if ( scale < 0 || scale > 8000)
	{
		if ( scale < 0 ) scale = 0;
		if ( scale > 8000 ) scale = 8000;
	}
	val = (mode << 16);
	val += (int)scale;

// download PRBS mode and scale
	dwStatus = WDM_Read_TSP_Indirectly(addr_);
	dwStatus = (((dwStatus>>19)&0x1FFF)<<19) + val;
	if(TSPL_SET_CONFIG_DOWNLOAD(addr_, dwStatus))
	{
		LldPrint_Error("tvb360 : pos...", 1, 102);
		return -1;
	}
	ResetMultiModBlk_n(my_ts_id);

	return 0;
}
int CLldTvb360u::TVB380_SET_MODULATOR_BERT_GEN_VirtualBd_n(long mod_typ, long packet_type)
{
	unsigned long dwStatus;
	unsigned long	addr_;

//	ǥ 66: BERT Control Register (0x0050.017D)
	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL2;		break;
	case	2:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL3;		break;
	case	3:		addr_ = _mRF_BERT_NOISE_GENERATION_CTRL4;		break;
	case	0:
	default:
		return -1;
	}
	if (mod_typ == TVB380_MULTI_VSB_MODE || mod_typ == TVB380_MULTI_QAMB_MODE || mod_typ == TVB380_MULTI_DVBT_MODE)
	{
//		dwStatus = WDM_Read_TSP_Indirectly(addr_);
		dwStatus = packet_type & 0x7;
		if(TSPL_SET_CONFIG_DOWNLOAD(addr_, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 103);
			return -1;
		}
	}
	ResetMultiModBlk_n(my_ts_id);
	return 0;
}
int CLldTvb360u::TVB380_GET_wr_BUF_STS_MultiBd_n(int _mod_n)	//	0, 1, 2, 3
{
	unsigned long	addr_, val_;
	int	ret_;

	switch(_mod_n)	//	my_ts_id
	{
	case	3:		addr_ = _mRF_Stream_Buffer_Status_4;		break;
	case	2:		addr_ = _mRF_Stream_Buffer_Status_3;		break;
	case	1:		addr_ = _mRF_Stream_Buffer_Status_2;		break;
	case	0:
	default:
		addr_ = _mRF_Stream_Buffer_Status_1;		break;
	}
	val_ = WDM_Read_TSP(addr_);
	ret_ = val_;	//	(val_ >> 28) & 0xf;
//	LldPrint_1x_nochk_prtoption("m-bd-wr-buf", ret_);
	return	ret_;
}
int CLldTvb360u::TVB380_GET_wr_cap_BUF_STS_MultiBd_n(void)
{
	unsigned long	val_;
	int	ret_;

	val_ = WDM_Read_TSP(_mRF_Capture_Buffer_Status);
	ret_ = val_;	//	(val_ >> 28) & 0xf;
	return	ret_;
}
//2012/4/12 SINGLE TONE
int _stdcall CLldTvb360u::TVB380_SET_MODULATOR_SINGLE_TONE(long modulator_type, unsigned long freq, long singleTone)
{
	//TVB593
	unsigned long dwStatus = 0;
	int _singleToneValue = 0;
	LldPrint_lul("TVB380_SET_MODULATOR_SINGLE_TONE", modulator_type, freq, singleTone);

	if ( IsAttachedBdTyp_SingleTone() )
	{
		gSingleTone = singleTone;
		if(singleTone == 1)
		{
			_singleToneValue = Get_SingleTone_Value(modulator_type, freq);
			if(_singleToneValue == -1)
				return -1;
		}

		dwStatus = _singleToneValue;
		if(TSPL_SET_CONFIG_DOWNLOAD(PCI593_SINGLE_TONE, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 100);
			return -1;
		}
	}
	return 0;
}
int CLldTvb360u::Get_SingleTone_Value(long modulator_type, unsigned long freq)
{
	int *singleToneTable = NULL;
	int ret = -1;
	int i = 0;
	switch(modulator_type)
	{
	case TVB380_VSB8_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_V_T_H_TD_MH[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_V_T_H[0];
			else
			singleToneTable = &gSingleTone_TVB591S_V_T_H_MH[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_VSB[0];
		}
		break;
	case TVB380_DVBT_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_V_T_H_TD_MH[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_V_T_H[0];
			else
			singleToneTable = &gSingleTone_TVB591S_V_T_H_MH[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;
	case TVB380_DVBH_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_V_T_H_TD_MH[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_V_T_H[0];
			else
			singleToneTable = &gSingleTone_TVB591S_V_T_H_MH[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;
	case TVB380_ATSC_MH_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_V_T_H_TD_MH[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_MH[0];
			else
			singleToneTable = &gSingleTone_TVB591S_V_T_H_MH[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_ATSCMH[0];
		}
		break;
	case TVB380_QAMA_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_QAM_A_B[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_QAM_A_B[0];
			else
			singleToneTable = &gSingleTone_TVB591S_QAM_A_B[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;
	case TVB380_QAMB_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_QAM_A_B[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_QAM_A_B[0];
			else
			singleToneTable = &gSingleTone_TVB591S_QAM_A_B[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;

	case TVB380_QPSK_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_S_S2[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_S_S2[0];
			else
			singleToneTable = &gSingleTone_TVB591S_S_S2[0];
		}
		break;
	case TVB380_DVBS2_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_S_S2[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_S_S2[0];
			else
			singleToneTable = &gSingleTone_TVB591S_S_S2[0];
		}
		break;
	case TVB380_TDMB_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_V_T_H_TD_MH[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_IT1_TD[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IT1_TD[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;

	case TVB380_ISDBT_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_IT1[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_IT1_TD[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IT1_TD[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_ISDBT1[0];
		}
		break;
	
	case TVB380_ISDBT_13_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_IT13_DT[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_IT13[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IT13_DT[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_T_H_QAB_TD_IT13[0];
		}
		break;
	case TVB380_DTMB_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_IT13_DT[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_DT[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IT13_DT[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_DTMB[0];
		}
		break;
	
	case TVB380_CMMB_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_CM[0];
		}
		break;
	
	case TVB380_DVBT2_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_T2[0];
		}
		break;
	
	case TVB380_IQ_PLAY_MODE:
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_IQ[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_IQ[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IQ[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB591_IQ[0];
		}
		break;
	
	case TVB380_ISDBS_MODE:	
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_IS[0];
		}
		else if(IsAttachedBdTyp_591S())
		{
			if(IsAttachedBdTyp_591S_V2())
				singleToneTable = &gSingleTone_TVB591SV2_IS[0];
			else
			singleToneTable = &gSingleTone_TVB591S_IS[0];
		}
		break;
	
	case TVB380_DVBC2_MODE:	
		if(IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB593_597V2_C2[0];
		}
		break;
	}
	if(IsAttachedBdTyp_597A_V4())
	{
		singleToneTable = &gSingleTone_TVB597AV4_BASE[0];
	}
	else if(IsAttachedBdTyp_599())
	{
		singleToneTable = &gSingleTone_TVB599AV1_BASE[0];
	}
	else if(IsAttachedBdTyp_598())
	{
		singleToneTable = &gSingleTone_TVB599AV1_BASE[0];
	}
	else if(IsAttachedBdTyp_TVB593_597A_V3())
	{
		if(IsAttachedBdTyp_597v2())
		{
			singleToneTable = &gSingleTone_TVB597AV3_BASE[0];
		}
		else
		{
			singleToneTable = &gSingleTone_TVB593V3_BASE[0];
		}
	}
	

	if(singleToneTable == NULL)
		return ret;

	while(1)
	{
		if(singleToneTable[i] == -1)
			break;
		if( (unsigned long)((unsigned long)singleToneTable[i] * 1000000) < freq &&
			(unsigned long)((unsigned long)singleToneTable[i + 1] * 1000000) >= freq )
		{
			ret = singleToneTable[i + 2];
			break;
		}
		i = i + 3;
	}
	return ret;
}

//2012/6/28 multi dvb-t =================================================================================
int	CLldTvb360u::TVB380_SET_MODULATOR_BANDWIDTH_VirtualBd_n(long modulator_type, long bandwidth)
{
	unsigned long	val_, addr_;
	
	for(int i = 1 ; i < 4 ; i++)
	{
		switch(i)
		{
		case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
		case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
		case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
		default:
			continue;
		}
		val_ = WDM_Read_TSP_Indirectly(addr_);
		val_ &= (~(0x3 << 11));
		val_ |= (bandwidth << 11);
		WDM_WR2SDRAM_ADDR(addr_, val_);
		Sleep(1);
	}
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_CODERATE_VirtualBd_n(long modulator_type, long coderate)
{
	unsigned long	val_, addr_;
	
	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0x7 << 8));
	val_ |= (coderate << 8);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	Sleep(1);
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_GUARDINTERVAL_VirtualBd_n(long modulator_type, long guardinterval)
{
	unsigned long	val_, addr_;
	
	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0x3 << 6));
	val_ |= (guardinterval << 6);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	Sleep(1);
	
	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_TXMODE_VirtualBd_n(long modulator_type, long txmode)
{
	unsigned long	val_, addr_;
	
	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0x3 << 2));
	val_ |= (txmode << 2);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	Sleep(1);

	return 0;
}
int	CLldTvb360u::TVB380_SET_MODULATOR_CELLID_VirtualBd_n(long modulator_type, long cellid)
{
	unsigned long	val_, addr_;
	
	switch(my_ts_id)
	{
	case	1:		addr_ = _mRF_MODULATOR_PARAMETER_2;		break;
	case	2:		addr_ = _mRF_MODULATOR_PARAMETER_3;		break;
	case	3:		addr_ = _mRF_MODULATOR_PARAMETER_4;		break;
	default:
		return 0;
	}
	val_ = WDM_Read_TSP_Indirectly(addr_);
	val_ &= (~(0xffff << 16));
	val_ |= (cellid << 16);
	WDM_WR2SDRAM_ADDR(addr_, val_);
	Sleep(1);

	return 0;
}

//2012/8/29 new rf level control
int _stdcall CLldTvb360u::TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *_p_rf_level_min, 
                                       double *_p_rf_level_max, long UseTAT4710)
{

	double rf_level_min = -30;
	double rf_level_max = 0;
	double rf_level_range_min;
	double rf_level_range_max;
	//TVB599 & 598 TAT4710 offset
	double nTat4710_offset = 0;
	if(UseTAT4710 == 2 && ScanTAT4710())
	{
		SetTAT4710(0);
		return 0;
	}
	gEnableTAT4710 = UseTAT4710;
//2013/9/26 RF LEVEL TEST
#if 0
	double dw_rf_level_min = -30.0;
	double dw_rf_level_max = 0.;
	GET_MODULATOR_RF_LEVEL_RANGE_Improve(modulator_type, &dw_rf_level_min, &dw_rf_level_max);	
	rf_level_range_min = (double)dw_rf_level_min - 31.;
	rf_level_range_max = (double)dw_rf_level_max;
#else
	GET_MODULATOR_RF_LEVEL_RANGE(modulator_type, &rf_level_min, &rf_level_max);
	rf_level_range_min = (double)rf_level_min - 31.;
	rf_level_range_max = (double)rf_level_max;
#endif

#ifdef WIN32	
	if(UseTAT4710 == 1 && ScanTAT4710())
	{
		rf_level_range_min = rf_level_range_min - 127.;
		if(IsAttachedBdTyp_599() || IsAttachedBdTyp_598() || IsAttachedBdTyp_593())
		{
			nTat4710_offset = GetTAT4710_Offset();
		}
		else
			nTat4710_offset = 5;
	}
#endif
	*_p_rf_level_min = rf_level_range_min - nTat4710_offset;
	*_p_rf_level_max = rf_level_range_max - nTat4710_offset;
	
	return 0;
}

//2012/8/27 new rf level control
int _stdcall CLldTvb360u::TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710)
{
	double atten = 0;
	int dwData = 0;
	double rf_level_min = -30;
	double rf_level_max = 0;
	double rf_level_range_min;
	double rf_level_range_max;
	double rf_level_range_amp_on;
	double rf_level_range_min_usetat4710;
	double nTat4710_offset = 0;
	int ampStatus;

	//TVB593
	unsigned long dwStatus = 0;
	unsigned long multiModeFreqGap = 0;

	gEnableTAT4710 = UseTAT4710;

	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LldPrint_ldll("TVB59x_SET_MODULATOR_RF_LEVEL_VALUE", modulator_type, rf_level_value, *AmpFlag, UseTAT4710);

	//fixed - called after RF/SYMBOL set
	if ( gCenter_Freq <= 1000000 && gSymbol_Rate <= 1000000 ) 
	{
		return 0;
	}
	
	if(IsAttachedBdTyp_593() && (modulator_type == TVB380_MULTI_VSB_MODE || modulator_type == TVB380_MULTI_QAMB_MODE || modulator_type == TVB380_MULTI_DVBT_MODE))
	{
		if(modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(gSymbol_Rate == DVB_T_6M_BAND)
				multiModeFreqGap = 3000000;
			else if(gSymbol_Rate == DVB_T_7M_BAND)
				multiModeFreqGap = 3500000;
			else
				multiModeFreqGap = 4000000;
		}
		else
			multiModeFreqGap = 9000000;
	}
	if(IsAttachedBdTyp_598() && (modulator_type == TVB380_MULTI_VSB_MODE || modulator_type == TVB380_MULTI_QAMB_MODE || modulator_type == TVB380_MULTI_DVBT_MODE))
	{
		if(modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(gSymbol_Rate == DVB_T_6M_BAND)
				multiModeFreqGap = 3000000 * 3;
			else if(gSymbol_Rate == DVB_T_7M_BAND)
				multiModeFreqGap = 3500000 * 3;
			else
				multiModeFreqGap = 4000000 * 3;
		}
		else
			multiModeFreqGap = 9000000;
	}
//	if((gCenter_Freq - multiModeFreqGap) > 2150000000)
//		return 0;

	//EXT. ATTEN
	DBG_PLATFORM_BRANCH();
#if defined(WIN32)
	if ( modulator_type == -1 )
	{
		//AGC
		//2011/6/29 added UseTAT4710
		if(UseTAT4710 == 1 && ScanTAT4710())
			SetTAT4710(0);
		return 0;
	}
#endif
	SET_TVB59x_RF_LEVEL_9775(modulator_type);
//2013/9/26 RF LEVEL TEST
#if 0
	double dw_rf_level_min = -30.0;
	double dw_rf_level_max = 0.;
	GET_MODULATOR_RF_LEVEL_RANGE_Improve(modulator_type, &dw_rf_level_min, &dw_rf_level_max);

	rf_level_range_min = (double)dw_rf_level_min - 31.;
	rf_level_range_min_usetat4710 = rf_level_range_min;

#ifdef WIN32
	if(UseTAT4710 == 1 && ScanTAT4710())
		rf_level_range_min_usetat4710 = rf_level_range_min - 127.;
#endif
	rf_level_range_max = (double)dw_rf_level_max;
	rf_level_range_amp_on = (double)dw_rf_level_min;
#else
	GET_MODULATOR_RF_LEVEL_RANGE(modulator_type, &rf_level_min, &rf_level_max);

	rf_level_range_min = (double)rf_level_min - 31.;
	rf_level_range_min_usetat4710 = rf_level_range_min;

#ifdef WIN32
	if(UseTAT4710 == 1 && ScanTAT4710())
	{
		rf_level_range_min_usetat4710 = rf_level_range_min - 127.;
		if(IsAttachedBdTyp_599() || IsAttachedBdTyp_598() || IsAttachedBdTyp_593())
		{
			nTat4710_offset = GetTAT4710_Offset();
		}
		else
			nTat4710_offset = 5;
	}
#endif
	rf_level_range_min = rf_level_range_min - nTat4710_offset;
	rf_level_range_max = (double)rf_level_max - nTat4710_offset;
	rf_level_range_amp_on = (double)rf_level_min - nTat4710_offset;
#endif
	if(rf_level_value > rf_level_range_amp_on)
	{
		atten = rf_level_range_max - rf_level_value;
		ampStatus = 2;
	}
	else
	{
		atten = rf_level_range_amp_on - rf_level_value;
		ampStatus = 1;
	}
//2013/9/6 598 LEVEL 
#if 0
	if(IsAttachedBdTyp_598())
		ampStatus = 2;
#endif
	*AmpFlag = ((ampStatus - 1) % 2);

#ifdef WIN32
	if(UseTAT4710 == 1  && ScanTAT4710())
	{
		if(rf_level_range_min > rf_level_value)
			SetTAT4710((rf_level_range_min - rf_level_value));
		else
			SetTAT4710(0);
	}
#endif
	rf_level_value = atten;
	if ( atten >= 16.0 )
	{
		dwData += ( 1 << 5 );	atten -= 16.0;
	}
	if ( atten >= 8.0 )
	{
		dwData += ( 1 << 4 );	atten -= 8.0;
	}
	if ( atten >= 4.0 )
	{
		dwData += ( 1 << 3 );	atten -= 4.0;
	}
	if ( atten >= 2.0 )
	{
		dwData += ( 1 << 2 );	atten -= 2.0;
	}
	if ( atten >= 1.0 )
	{
		dwData += ( 1 << 1 );	atten -= 1.0;
	}
	if ( atten >= 0.5 )
	{
		dwData += ( 1 << 0 );	atten -= 0.5;
	}

	if ( IsAttachedBdTyp_AttenTyp_AssignNewBitsPos() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);

		dwStatus = (((dwStatus>>7)&0x1FFFFFF)<<7) + (dwData<<1) + (dwStatus&0x01);
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, dwStatus))
		{
			LldPrint_Error("tvb360 : pos...", 1, 46);
			return -1;
		}
	}
	else
	{
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, dwData))
		{
			LldPrint_Error("tvb360 : pos...", 1, 47);
			return -1;
		}
	}

#if 1
	g_ScaleOffset = (rf_level_value - (int)rf_level_value)*10.;
	if ( g_ScaleOffset >= 5.)
	{
		g_ScaleOffset -= 5.;
	}
	g_ScaleOffset *= AD9857_SCALE_OFFSET;
	if (g_ScaleOffset > 0)
	{
		if ( IsAttachedBdTyp_AttenTyp_4() )
		{
		}
		else
		{
			CntlDac_AD9xxx(modulator_type, gCenter_Freq, gSymbol_Rate);
		}

		if ( (IsAttachedBdTyp_AttenTyp_1() || IsAttachedBdTyp_499()) && 
			(modulator_type == TVB380_ISDBT_13_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		if ( (IsAttachedBdTyp_AttenTyp_2() || IsAttachedBdTyp_499()) && 
			(modulator_type == TVB380_ATSC_MH_MODE
			&& (CurrentTSSrc == 4/*TSIO_310_CAPTURE_PLAY*/ || CurrentTSSrc == 5/*TSIO_ASI_CAPTURE_PLAY*/)) )
		{
			return 0;
		}
		if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
			(modulator_type == TVB380_ISDBS_MODE && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}
		//2012/7/11 DVB-T2 ASI
		if ( (IsAttachedBdTyp_AttenTyp_3() || IsAttachedBdTyp_499()) && 
			((modulator_type == TVB380_DVBT2_MODE || modulator_type == TVB380_DVBC2_MODE) && CurrentTSSrc == 3/*TSIO_ASI_LOOPTHRU*/) )
		{
			return 0;
		}

		ResetModBlkAfterChangingHwDacPara(3);
	}
#endif
	if(gBypass_AMP != ampStatus)
	{
		TVB380_SET_BOARD_CONFIG_STATUS(modulator_type, (ampStatus % 2));
	}

	return 0;
}

//========================================================================================================

//2012/9/6 Pcr Restamping control
int _stdcall CLldTvb360u::TVB59x_SET_PCR_STAMP_CNTL(int _val)
{
	LldPrint("TVB59x_SET_PCR_STAMP_CNTL", _val, 0, 0);
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	unsigned long regValue = 0x87000000;
	int	dwStatus;
	int	restamp_out_bit;
	int   bit_mask;
	if(( IsAttachedBdTyp_598() || IsAttachedBdTyp_599()) && (TSPL_nModulatorType == TVB380_MULTI_VSB_MODE || TSPL_nModulatorType == TVB380_MULTI_QAMB_MODE || TSPL_nModulatorType == TVB380_MULTI_DVBT_MODE))
	{
		restamp_out_bit = 20;
		bit_mask = ~(0x1 << 20);
	}
	else
	{
		restamp_out_bit = 9;
		bit_mask = ~(0x1 << 9);
	}

	if(_val == 3 || _val == 2)
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
		dwStatus = ((_val & 0x1)<<restamp_out_bit) + (dwStatus & bit_mask);
		WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);
		return 0;
	}

	regValue = regValue + (_val & 0xFFFFFF);
	WDM_WR2SDRAM_ADDR(PCI593_PCR_RESTAMP_CTRL, (unsigned long)regValue);
	Sleep(10);
#ifdef WIN32
	if(_val == 1 || ((IsAttachedBdTyp_599() || IsAttachedBdTyp_598()) && TSPL_nModulatorType == TVB380_QAMB_MODE))
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
		dwStatus = (dwStatus & bit_mask);//(dwStatus & 0xF1FF);
		WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);

	}
	else if(_val == 0)
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
		dwStatus = (0x1<<restamp_out_bit) + (dwStatus & bit_mask);
		WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);
	}
#else
	dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
	dwStatus = (dwStatus & 0xFFFFFDFF);
	WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);
#endif
	return 0;
}

//2013/5/31 TVB599 TS output
int _stdcall CLldTvb360u::TVB59x_SET_Output_TS_Type(int _val)
{
	LldPrint("TVB59x_SET_Output_TS_Type", _val, 0, 0);
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	int	dwStatus;
	int	restamp_out_bit;
	int   bit_mask;
	if((IsAttachedBdTyp_598() || IsAttachedBdTyp_599()) && 
		(TSPL_nModulatorType == TVB380_MULTI_VSB_MODE || TSPL_nModulatorType == TVB380_MULTI_QAMB_MODE || TSPL_nModulatorType == TVB380_MULTI_DVBT_MODE))
	{
		restamp_out_bit = 25;
		bit_mask = ~(0x1 << 25);
	}
	else
	{
		restamp_out_bit = 14;
		bit_mask = ~(0x1 << 14);
	}
	//LockDmaMutex();
	dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
	dwStatus = ((_val & 0x1) << restamp_out_bit) + (dwStatus & bit_mask);
	WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);
	//UnlockDmaMutex();
	return 0;
}

int _stdcall CLldTvb360u::TVB59x_SET_Reset_Control_REG(int _val)
{
	LldPrint("TVB59x_SET_Reset_Control_REG", _val, 0, 0);
	//LockDmaMutex();
	if(_val == 8)
	{
		ModulatorResetFlag = 1;
	}
	else
	{
		AssertReleaseResetSignal((0x1 << _val));
		AssertReleaseResetSignal(0);
	}
	//UnlockDmaMutex();
	return 0;
}
//2013/5/3 ASI
int _stdcall CLldTvb360u::TVB59x_SET_TsPacket_CNT_Mode(int _val)
{
	LldPrint("TVB59x_SET_TsPacket_CNT_Mode", _val, 0, 0);
	int	dwStatus;
	//LockDmaMutex();
	
	dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_OPERATION_MODE);
	dwStatus = ((_val & 0x1)<<7) + (dwStatus & 0xFFFFFF7F);
	WDM_WR2SDRAM_ADDR(PCI593_REG_OPERATION_MODE, (unsigned long)dwStatus);
	
	//UnlockDmaMutex();
	gPacketCnt_old = 0;
	gClockCnt_old = 0;
	gPacketCnt_old_out = 0;
	gClockCnt_old_out = 0;
	return 0;
}

int _stdcall CLldTvb360u::TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock)
{
	int iData;
	//const unsigned int constClockSignal = 38785316;
	//const unsigned int constMaxPacketNum = 4294967295;
	__int64 nPacketCnt;
	__int64 nClockCnt;
	__int64 delta_PacketCnt, delta_ClockCnt;
	__int64 nPacketCnt_out;
	__int64 nClockCnt_out;
	__int64 delta_PacketCnt_out, delta_ClockCnt_out;
	int bitrate, bitrate_out;
	iData = 0x11;
	WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)iData);
	iData = 0x2;
	WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)iData);
	nPacketCnt = WDM_Read_TSP(PCI593_REG_PUMPING_TS_COUNTER);
	iData = 0x6;
	WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)iData);
	nClockCnt = WDM_Read_TSP(PCI593_REG_PUMPING_TS_COUNTER);
	iData = 0xA;
	WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)iData);
	nPacketCnt_out = WDM_Read_TSP(PCI593_REG_PUMPING_TS_COUNTER);
	iData = 0xE;
	WDM_WR2SDRAM_ADDR(PCI593_TS_COUNTER_CTRL, (unsigned long)iData);
	nClockCnt_out = WDM_Read_TSP(PCI593_REG_PUMPING_TS_COUNTER);

	delta_PacketCnt = nPacketCnt - gPacketCnt_old;
	delta_ClockCnt = nClockCnt - gClockCnt_old;
	if(delta_PacketCnt <= 0)
		delta_PacketCnt = delta_PacketCnt + 4294967295;
	if(delta_ClockCnt <= 0)
		delta_ClockCnt = delta_ClockCnt + 4294967295;
	
	bitrate = (int)((double)((double)delta_PacketCnt / (double)delta_ClockCnt) * 38785316.0 * 188.0 * 8.0);
	gPacketCnt_old = nPacketCnt;
	gClockCnt_old = nClockCnt;

	delta_PacketCnt_out = nPacketCnt_out - gPacketCnt_old_out;
	delta_ClockCnt_out = nClockCnt_out - gClockCnt_old_out;
	if(delta_PacketCnt_out <= 0)
		delta_PacketCnt_out = delta_PacketCnt_out + 4294967295;
	if(delta_ClockCnt_out <= 0)
		delta_ClockCnt_out = delta_ClockCnt_out + 4294967295;
	
	bitrate_out = (int)((double)((double)delta_PacketCnt_out / (double)delta_ClockCnt_out) * 38785316.0 * 188.0 * 8.0);
	gPacketCnt_old_out = nPacketCnt_out;
	gClockCnt_old_out = nClockCnt_out;

	*delta_packet = (int)delta_PacketCnt;
	*delta_clock = bitrate_out;
	//sprintf(str, "IN(packet[%I64d], clock[%I64d], bitrate[%d]), OUT(packet[%I64d], clock[%I64d], bitrate[%d])\n", 
	//	delta_PacketCnt, delta_ClockCnt, bitrate, delta_PacketCnt_out, delta_ClockCnt_out, bitrate_out);
	//OutputDebugString(str);
	return bitrate;
}

int _stdcall CLldTvb360u::TVB59x_Modulator_Status_Control(int modulator, int index, int val)
{
	int shiftValue;
	unsigned long dwStatus;	
	unsigned long nMaskVal;
	dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_MOD_COMMAND1);

	switch(modulator)
	{
	case TVB380_QPSK_MODE:
		if(index == 0)
		{
			shiftValue = 9;		//Bypass Rate Adapt
			val = val & 0x1;
			nMaskVal = 0x3DFF;
		}
		else if(index == 1)
		{
			shiftValue = 10;	//Select PCR status
			val = val & 0x1;
			nMaskVal = 0x3BFF;
		}
		else if(index == 2)
		{
			shiftValue = 11;	//Status Selection
			val = val & 0x3;
			nMaskVal = 0x27FF;
		}
		else if(index == 3)
		{
			shiftValue = 13;	//Status Clear
			val = val & 0x1;
			nMaskVal = 0x1FFF;
		}
		else
			return -1;

		dwStatus = (val << shiftValue) + (dwStatus & nMaskVal);
		break;
	default:
		return -1;
	}
	//LockDmaMutex();
	if(TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_MOD_COMMAND1, dwStatus))
	{
		LldPrint_Error("tvb360 : pos...", 1, 6);
		//UnlockDmaMutex();
		return -1;
	}
	

	if(index == 0)
	{
		Sleep(10);
		ResetModBlkAfterChangingHwBlkPara(_CMD_MOD_RST__all_blk_);
	}
	//UnlockDmaMutex();
	return 0;
}

int _stdcall CLldTvb360u::TVB59x_Get_Modulator_Status(int index)
{
	static int parity_error_cnt = 0;
	unsigned long dwStatus;	
	int nRet;
	dwStatus = TSPL_T2_parity_errCnt;
	if(index == 0)
	{
		nRet = ((dwStatus >> 18) & 0xF); 
	}
	else if(index == 1)
	{
		nRet = (dwStatus & 0xFFFF);
	}
	else if(index == 2)
	{
		nRet = ((dwStatus >> 16) & 0xFFFF);
		if(parity_error_cnt != nRet)
		{
			parity_error_cnt = nRet;
			if(parity_error_cnt != 0)
			{
				LldPrint_Error("===[DVB-T2 Parity ERROR]", nRet, parity_error_cnt);
			}
		}
		return dwStatus;
	}
	return nRet;
}
