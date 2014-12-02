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
#include	"../include/logfile.h"
#include	"wdm_drv.h"
#include	"dma_drv.h"
#include	"mainmode.h"
#endif

#include	"Reg590S.h"
#include	"Reg593.h"
#include	"rf_level_tbl.h"
#include 	"dac_ctl.h"

#include	"../include/system_const.h"


#define 	MAX_CNR 50
#define AD9857_SCALE_OFFSET	3


/////////////////////////////////////////////////////////////////
#ifdef WIN32
extern	TCHAR	szCurDir[_MAX_PATH];
#endif
/////////////////////////////////////////////////////////////////
CDacCtl::CDacCtl(void)
{
	CurrentTSSrc = -1;//current TS direction

	g_ScaleOffset = 0.;	//SCALE ADJUST for .1dB CONTROL
	gRF_143KHZ = RF_143KHZ;//0=No shift, 1= ISDB-T 143KHz shift
	gSpectral_Inversion = 0;
	gClock_Mode = 0;//TVB590S
	gChannel_Freq_Offset = IF_OUT_36MHZ;//Hz
	gCenter_Freq = 1000000;//Hz, fixed - 473000000 -> 1000000
	gSymbol_Rate = 1000000;//Sps
	gRRC_Filter = ROLL_OFF_FACTOR_NONE;//DVB-S2, 0(0.20), 1(0.25), 2(0.35), 3(none)
	gBypass_AMP = 0;//TVB595V1, TVB590V9.2, (1=USE, 2=BYPASS if V1.0), (1=BYPASS, 2=USE if higher than V1.0)
	//FIXED - IF +- offset
	gRF_143KHZ_sign = -1;//or 1

	//TVB590S V3
	gQAMA_Constllation = 0;
	gQAMB_Constllation = 0;


	gCurrentTAT4710 = -1;
	gAGC_MaxAtten= 0.;
	gAGC_MinAtten = 0.;
	gAGC_AttenOffset = 0.;
	gAGC_CurrentAtten = 0.;
	gAGC_CurrentLevel = 0.;
	gAGC_On_Off = 0;

//RF power level
	gMaxLevel = 0;
	gMinLevel = 0;
	gCurrentLevel = 0;
	gCurrentAtten = 0;
	gRFLevel = NULL;
//TVB593
	gRFLevel_AD9775 = NULL;


}
CDacCtl::~CDacCtl()
{
}

int CDacCtl::IsLockAD9775(void)
{
	if ( IsAttachedBdTyp_UseAD9775() )
	{
		if ((TSPL_GET_AD9775(0) >> 1) & 0x1)
		{
			return	1;
		}
		return	0;
	}
	return	1;
}
int _stdcall CDacCtl::TSPL_GET_AD9775(long reg)
{
	int nRet = 0;
	long AD9775;
	AD9775 = (1<<16) + (1<<15) + (reg<<8);

	LldPrint_Trace("TSPL_GET_AD9775", reg, 0, (double)0, NULL);
	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		nRet = TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_DAC_CONTROL, AD9775);
		Sleep(1);//???
		nRet = WDM_Read_TSP(PCI590S_REG_DAC_DATA_READ);
	}
	//TVB593, TVB497
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		nRet = TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_CONTROL, AD9775);
		Sleep(1);//???
		nRet = WDM_Read_TSP(PCI593_REG_DAC_DATA_READ);
	}
	return (nRet & 0xFF);
}

int _stdcall CDacCtl::TSPL_SET_AD9775(long reg, long data)
{
	int nRet = 0;
	long AD9775;

	LldPrint_FCall("TSPL_SET_AD9775", reg, data);
	LldPrint_Trace("TSPL_SET_AD9775", reg, data, 0, NULL);

	if ( reg == -1 )
	{
		AD9775 = (1<<17);
	}
	else
	{
		AD9775 = (1<<16) + (reg<<8) + data;
	}

	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		nRet = TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_DAC_CONTROL, AD9775);
		Sleep(1);//???

		if ( reg == -1 )
		{
			AD9775 = 0;
			TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_DAC_CONTROL, AD9775);
			Sleep(1);//???
		}
	}
	//TVB593, TVB497
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		nRet = TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_CONTROL, AD9775);
		Sleep(1);//???

		if ( reg == -1 )
		{
			AD9775 = 0;
			TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_CONTROL, AD9775);
			Sleep(1);//???
		}
	}

	return nRet;
}

/*^^***************************************************************************
 * Description : Control AD9775
 *				
 * Entry : modulation_mode, dwfreq, symbol_freq
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CDacCtl::Default_Configure_AD9775(long modulation_mode, unsigned long dwfreq, long symbol_freq)
{
	unsigned long dwRet = 0;

	LldPrint_FCall("Default_Configure_AD9775", modulation_mode, dwfreq);

	if(modulation_mode == TVB380_TSIO_MODE)
		return 0;

	//RESET
	TSPL_SET_AD9775(-1, 0x00);
	TSPL_SET_AD9775(0x00, 0x04);
	TSPL_SET_AD9775(0x01, 0x81/*0xE1*/);
	TSPL_SET_AD9775(0x02, 0x20);
	TSPL_SET_AD9775(0x03, 0x00);
	//TVB593, TVB497
	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
			TSPL_SET_AD9775(0x04, 0xC0);
		}
		else
		{
			TSPL_SET_AD9775(0x04, 0x80);
		}
	}
	else
	{
		TSPL_SET_AD9775(0x04, 0x80);
	}
	TSPL_SET_AD9775(0x05, 0x00);

	dwRet = 0x5;
	if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE )
	{
		dwRet = 0xF;
	}
	TSPL_SET_AD9775(0x06, dwRet);

	//TSPL_SET_AD9775(0x07, 0x00);
	//TSPL_SET_AD9775(0x08, 0x00);
	//TSPL_SET_AD9775(0x09, 0x00);

	dwRet = 0x5;
	if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE )
	{
		dwRet = 0xF;
	}
	TSPL_SET_AD9775(0x0A, dwRet);

	//TSPL_SET_AD9775(0x0B, 0x00);
	//TSPL_SET_AD9775(0x0C, 0x00);

	Sleep(1);
	dwRet = TSPL_GET_AD9775(0x00);

	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_DAC_PLL_LOCK_CONTROL, (dwRet>>1) & 0x01);
	}
	//TVB593, TVB497
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_PLL_LOCK_CONTROL, (dwRet>>1) & 0x01);
	}

	return 0;
}
int	CDacCtl::CntlDac_AD9775(long modulation_mode, unsigned long dwfreq, long symbol_freq)	//	dac is ad9775
{
	DWORD dwData, temp_data;
	int nClock_Mode=0;

	double Interpol_rate;
	double PLL_Divider;
	double Modulation_Mode;
	double Im_Reject;
	double fDDS, fDATA, fDAC;
	double symbol_clock;
	int N, reg_data;
	//int Coeff_Val, Coeff_Addr;
	
	LldPrint_FCall("CntlDac_AD9775", 0, 0);
	LldPrint("[LLD]===CONFIG AD9775 : Type, RF, BW/SYMBOLRATE", (int)modulation_mode, (int)dwfreq, (int)symbol_freq);
	
	if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE || modulation_mode == TVB380_MULTI_DVBT_MODE)
	{
		symbol_clock = (double)symbol_freq;
		if ( symbol_freq == DVB_T_5M_BAND ) //DVB-H, 5MHz
			symbol_clock = 5;
		else
			symbol_clock += 6;//symbol_freq == bandwidth
	
		symbol_clock = symbol_clock*8;
	}
	//ATSC-M/H
	else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
	{
		symbol_freq = 4500000;
		symbol_clock = (symbol_freq / 1000000.0) * (684./286.) * 4.;
	}
	else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE )
	{
		if (symbol_freq < 1000000 || symbol_freq > 8000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 1);
			return -1;
		}
	
		N = 4;
	
		symbol_clock = 2*symbol_freq*N;
		if ( symbol_clock  > 90000000 )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 2);
			return -1;
		}
		symbol_clock /= 1000000.;
	}
	else if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
	{
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 1
		int MaximumSym;
	
		if(IsAttachedBdTyp_590s_SmallAddrSpace() || IsAttachedBdTyp_497() || IsAttachedBdTyp_591())
		{
			MaximumSym = 45000000;
		}
		else
		{
			MaximumSym = 70000000;
		}
		if (symbol_freq < 1000000 || symbol_freq > MaximumSym)
			return -1;
#else
		if (symbol_freq < 1000000 || symbol_freq > 45000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 3);
			return -1;
		}
#endif
		if ( symbol_freq >= 1000000 && symbol_freq < 2500000 )
		{
			N=4;
			nClock_Mode = 2;
		}
		else if ( symbol_freq < 5000000 )
		{
			N=4;
			nClock_Mode = 2;
		}
		else if ( symbol_freq < 10000000 )
		{
			N=2;
			nClock_Mode = 1;
		}
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 1
		else if ( symbol_freq <= MaximumSym )
#else
		else if ( symbol_freq <= 45000000 )
#endif
		{
			N=1;
			nClock_Mode = 0;
		}
		else
		{
			LldPrint_Error("dac-ctl : pos...", 6, 4);
			return -1;
		}
		gClock_Mode = nClock_Mode;
				
		//TEST - SCLEE
		symbol_clock = symbol_freq*N;
		if ( symbol_clock  > 90000000 )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 5);
			return -1;
		}
		symbol_clock /= 1000000.;
	
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
			LldPrint_1x("[LLD]TVB593 MODULATOR PARAM/AD9857_ADDR_QPSK_CLOCK_MODE=", (unsigned int)temp_data);
	
			if ( modulation_mode == TVB380_QPSK_MODE )
				temp_data = (((temp_data>>7)&0x03)<<7) + (nClock_Mode<<5) + (temp_data&0x1F);
			else
				temp_data = (((temp_data>>13)&0x01)<<13) + (nClock_Mode<<11) + (temp_data&0x7FF);
					
			if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, temp_data))
			{
				LldPrint_Error("dac-ctl : pos...", 6, 6);
				return -1;
			}
	
			LldPrint_1x("[LLD]TVB593...", (unsigned int)temp_data);
		}
		else
	
		if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, ((gSpectral_Inversion<<3) & 0x1) + nClock_Mode) )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 7);
			return -1;
		}
		//Sleep(50);//???
	}
	else if ( modulation_mode == TVB380_TDMB_MODE )
	{
		symbol_freq = 2048000;
		symbol_clock = (symbol_freq / 1000000.0) * 12.;
	}
	else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
#ifdef WIN32
		if(symbol_freq == 0)
			symbol_clock = 65.015872;	//6MHz
		else if(symbol_freq == 1)
			symbol_clock = 75.851851;	//7MHz
		else
			symbol_clock = 86.68783;		//8MHz
#else
		symbol_clock = 65.015872;
#endif
	}
	else if ( modulation_mode == TVB380_DTMB_MODE )
	{
		symbol_clock = 60.48;
		if(symbol_freq == 0)
			symbol_clock = symbol_clock * 6. / 8.;
		else if(symbol_freq == 1)
			symbol_clock = symbol_clock * 7. / 8.;
	}
	else if ( modulation_mode == TVB380_CMMB_MODE )
	{
		symbol_clock = 60.0;
	}
	//DVB-T2
	else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
	{
		symbol_clock = (double)symbol_freq;
		if ( symbol_freq == DVB_T_5M_BAND ) //DVB-H, 5MHz
			symbol_clock = 5;
		/*
		else if ( symbol_freq == DVB_T_1_7M_BAND )
			symbol_clock = 1.7;
		else if ( symbol_freq == DVB_T_10M_BAND )
			symbol_clock = 10.;
		*/
		else
			symbol_clock += 6;//symbol_freq == bandwidth
		//2012/11/23 DVB-T2 Bandwidth 1.7MHz Test
		if(symbol_freq == DVB_T_1_7M_BAND)
		{
			if(TSPL_T2_x7_Mod_Clock == 1)
				symbol_clock = (131. / 71.) * 7.;
			else
				symbol_clock = (131. / 71.) * 8.;
		}
		else
		{
			if(TSPL_T2_x7_Mod_Clock == 1)
				symbol_clock = symbol_clock*8.;
			else
				symbol_clock = symbol_clock*8.*(8./7.);
		}
	}
	//I/Q PLAY/CAPTURE - ???
	else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
	{
		symbol_clock = (double)(symbol_freq / 1000000.);
	}
	//ISDB-S
	else if ( modulation_mode == TVB380_ISDBS_MODE )
	{
		symbol_clock = 57.72;//28.860*2
	}
	
	fDDS = symbol_clock;//Mhz
	fDATA = fDDS;
	if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
	{
		fDATA /= 7.;
	}
	//ATSC-M/H
	else if ( modulation_mode == TVB380_VSB8_MODE ||  modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE )
	{
		fDATA /= 4.;
	}
	else if (modulation_mode == TVB380_MULTI_VSB_MODE )
	{
			fDATA /= 1.;
	}
	else if ( modulation_mode == TVB380_TDMB_MODE )
	{
		fDATA /= 3.;
	}
	//TEST - SCLEE
	else if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
	{
		//2012/7/20 RRC FILTER OFF
		if(modulation_mode == TVB380_DVBS2_MODE && gRRC_Filter == ROLL_OFF_FACTOR_NONE && (IsAttachedBdTyp_599() || IsAttachedBdTyp_593() || IsAttachedBdTyp_597v2() || IsAttachedBdTyp_598()))
			fDATA *= 1;
		else
			fDATA *= 2;
	}
	//TVB593
	else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
		fDATA /= 8.;//8,126,984 Hz
	}
	else if ( modulation_mode == TVB380_DTMB_MODE )
	{
		//fDATA = 15.;//Interpol_rate:8(2^3), PLL_Divider:2(2^1)
		fDATA = fDATA / 4.;
	}
	else if ( modulation_mode == TVB380_CMMB_MODE )
	{
		fDATA = 10.;//Interpol_rate:8(2^3), PLL_Divider:2(2^2)
	}
	else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
	{
		fDATA /= 8.;
	}
	else if(modulation_mode == TVB380_MULTI_DVBT_MODE)
	{
		fDATA = (fDATA / 7.) * 4.; 
	}

	//I/Q PLAY/CAPTURE - ???
#if 0
	if ( modulation_mode == TVB380_IQ_PLAY_MODE )
	{
		fDAC = fDATA;

		if ( fDATA < 7. )
		{
			Interpol_rate = 3;
			PLL_Divider = 3;
		}
		else if ( fDATA < 14. )
		{
			Interpol_rate = 3;
			PLL_Divider = 2;
		}
		else if ( fDATA < 28. )
		{
			Interpol_rate = 3;
			PLL_Divider = 1;
		}
		else //if ( fDATA < 50. )
		{
			Interpol_rate = 3;
			PLL_Divider = 0;
		}
	}
	else
#endif
	{
		//interpolation
		//TVB590S V2 - TEST
		//if ( fDATA < 50.0 )
		if ( fDATA < 40.0 )
		{
			Interpol_rate = 3;
		}
		else if ( fDATA < 80.0 )
		{
			Interpol_rate = 2.;
		}
		else if ( fDATA < 160.0 )
		{
			Interpol_rate = 1.;
		}
		else
		{
			Interpol_rate = 0.;
			LldPrint_1("Invalid DAC input data rate, DAC can't support over 160MSps MSps", (int)fDATA);
		}
		//2012/11/23 DVB-T2 1.7MHz
		if(modulation_mode == TVB380_DVBT2_MODE && symbol_freq == DVB_T_1_7M_BAND)
			Interpol_rate = 2.;

		fDAC = fDATA * pow(2., Interpol_rate);
		if ( fDAC > 400. )
		{
			LldPrint_1("Invalid DAC input frequency, DAC can't support over 400MSps MSps", (int)fDAC);
		}
		//2012/3/7 TVB590S Ver2 PLL UNLOCK fixed.
		if(IsAttachedBdTyp_590S_Ver2())
		{
			//PLL Divider
			if ( fDAC < 56. )
			{
				PLL_Divider = 3.;
			}
			//FIXED - 112.0 => 100.0 => 112.0
			else if ( fDAC < 112.0 )
			{
				PLL_Divider = 2.;
			}
			//FIXED - 224.0 => 216.0 => 244.0
			else if ( fDAC < 224.0 )
			{
				PLL_Divider = 1.;
			}
			else
			{
				PLL_Divider = 0.;
			}
		}
		else
		{
			//PLL Divider
			if ( fDAC < 56. )
			{
				PLL_Divider = 3.;
			}
			//FIXED - 112.0 => 100.0 => 112.0
			else if ( fDAC < 112.0 )
			{
				PLL_Divider = 2.;
			}
			//FIXED - 224.0 => 216.0 => 244.0
			else if ( fDAC < 244.0 )
			{
				PLL_Divider = 1.;
			}
			else
			{
				PLL_Divider = 0.;
			}
		}

		//ISDB-S
		if ( modulation_mode == TVB380_ISDBS_MODE )
		{
			fDAC = fDATA;
			Interpol_rate = 2;
			PLL_Divider = 0;
		}
	}

	Modulation_Mode = 0;
	Im_Reject = 0;

	//AD9775
	reg_data = ((int)Interpol_rate<<6) + ((int)Modulation_Mode<<4) + ((int)Im_Reject<<1) + 1;
	TSPL_SET_AD9775(0x01, reg_data);
	TSPL_SET_AD9775(0x03, (long)PLL_Divider);
//	LldPrint("Set ad9775 : fDDS, fDATA, fDAC", fDDS, fDATA, fDAC);
//	LldPrint("PLL_Devider=0x%x, Interpol_rate=0x%x, Modulation_Mode=0x%x, Im_Reject=0x%0x, nClock_Mode=0x%x, reg1=0x%x, reg3=0x%x, ", 
//		(int)pow(2.,PLL_Divider), (int)pow(2.,Interpol_rate), (int)Modulation_Mode, (int)Im_Reject, nClock_Mode, reg_data, (int)PLL_Divider);

	Sleep(1);
	reg_data = TSPL_GET_AD9775(0x00);
//	LldPrint("reg0=", reg_data);
	//TVB593 - DVB-T/H, BW=6MHz
	//if unlock, retry...
	if ( ((reg_data>>1) & 0x01) != 1 ) 
	{
		dwData = 100;
		while ( dwData > 0 )
		{
			reg_data = TSPL_GET_AD9775(0x00);
//			LldPrint_2("reg0...", (int)dwData, (unsigned int)reg_data);

			if ( ((reg_data>>1) & 0x01) == 1 ) 
				break;

			Sleep(10);
			dwData -= 10;
		}
	}

	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_DAC_PLL_LOCK_CONTROL, (reg_data>>1) & 0x01);
	}
	//TVB593
	else
	{
		TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_PLL_LOCK_CONTROL, (reg_data>>1) & 0x01);
	}

	return	0;
}
int	CDacCtl::CntlDac_AD9857(long modulation_mode, unsigned long dwfreq, long symbol_freq)	//	dac is ad9857
{
	DWORD dwData, temp_data;
	__int64 llOutputFTW;
	int nOutputFTW, nREF_Multiplier=0, nCIC_Interpolation=0;
	int nClock_Mode=0, nBYPASS_INV_CIC=0;
	double dwSYSCLK, dwOutputFTW;
	int nParam;
	DWORD dwQsymbol;

	LldPrint_FCall("CntlDac_AD9857", 0, 0);
	LldPrint("[LLD]===CONFIG AD9857, Type, RF, BW/SYMBOLRATE", (int)modulation_mode, (int)dwfreq, (int)symbol_freq);
	// calculate FTW
	if ( modulation_mode == TVB380_DVBT_MODE  || modulation_mode == TVB380_DVBH_MODE )
	{
		nREF_Multiplier = 20;
		nCIC_Interpolation = 5;
	
		if ( symbol_freq == DVB_T_5M_BAND ) //DVB-H, 5MHz
			symbol_freq = 5;
		else
			symbol_freq += 6;//symbol_freq == bandwidth
		dwSYSCLK = (double)symbol_freq * (8./7.)* nREF_Multiplier;
	}
	else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE)
	{
		if (symbol_freq < 1000000 || symbol_freq > 8000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 8);
			return -1;
		}
	
		nCIC_Interpolation = (int)(floor(50000000. / (symbol_freq*2.)));
		if ( nCIC_Interpolation > 20 )
			nCIC_Interpolation = 20;
			
		dwSYSCLK = (symbol_freq * 2) * 4 * nCIC_Interpolation;
	
		if ( symbol_freq >= 1000000 && symbol_freq < 2500000 )
		{
			dwData = 4 * (symbol_freq*2);
			nClock_Mode = 2;
		}
		else if ( symbol_freq >= 2500000 && symbol_freq < 5000000 )
		{
			dwData = 2 * (symbol_freq*2);
			nClock_Mode = 1;
		}
		else
		{
			dwData = 1 * (symbol_freq*2);
			nClock_Mode = 0;
		}
		nREF_Multiplier = (int)(dwSYSCLK / dwData);

		if (IsAttachedBdTyp_591())
		{
			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QAM_CLOCK_MODE);
			LldPrint_1x("[LLD]TVB591 MODULATOR PARAM/AD9857_ADDR_QAM_CLOCK_MODE=", (unsigned int)temp_data);
			if (modulation_mode == TVB380_QAMA_MODE)
			{
				nClock_Mode <<= 5;
				nClock_Mode = nClock_Mode + (temp_data & 0x1F);
			}
			else if (modulation_mode == TVB380_QAMB_MODE)
			{
				nClock_Mode <<= 7;
				nClock_Mode = (temp_data & 0x7F) + nClock_Mode;
			}
		}

		if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QAM_CLOCK_MODE, nClock_Mode) )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 9);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(50);
	
		dwSYSCLK /= 1000000.0;
	
		if ( nCIC_Interpolation == 1 )
		{
			nBYPASS_INV_CIC = 1;
		}
	}
	else if ( modulation_mode == TVB380_QPSK_MODE )
	{
		if (symbol_freq < 1000000 || symbol_freq > 45000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 10);
			return -1;
		}
	
		if (gRRC_Filter != ROLL_OFF_FACTOR_NONE)
		{
			dwQsymbol = symbol_freq*2;
		}
		else
		{
			dwQsymbol = symbol_freq*1;
		}
	
		nCIC_Interpolation = (int)(floor(50000000. / dwQsymbol));
		if ( nCIC_Interpolation > 20 )
			nCIC_Interpolation = 20;
					
		dwSYSCLK = dwQsymbol * 4 * nCIC_Interpolation;
			
		if ( symbol_freq >= 1000000 && symbol_freq < 5000000 )
		{
			dwData = 4 * dwQsymbol;
			nClock_Mode = 2;
		}
		else if ( symbol_freq >= 5000000 && symbol_freq < 10000000 )
		{
			dwData = 2 * dwQsymbol;
			nClock_Mode = 1;
		}
		else
		{
			dwData = 1 * dwQsymbol;
			nClock_Mode = 0;
		}
		if (gRRC_Filter != ROLL_OFF_FACTOR_NONE)
		{
			nClock_Mode += (1<<2);
		}
		LldPrint_5("dwSYSCLK, QPSK_Clock_Mode, Qsymbol, fref, gRRC_Filter", (int)dwSYSCLK, nClock_Mode, (int)dwQsymbol, (int)dwData, (int)gRRC_Filter);
		//==
			
		nREF_Multiplier = (int)(dwSYSCLK / dwData);
	
		//TVB593, TVB497
		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
			LldPrint_1x("[LLD]TVB593 MODULATOR PARAM/AD9857_ADDR_QPSK_CLOCK_MODE=", (unsigned int)temp_data);
	
			temp_data = (((temp_data>>7)&0x01)<<7) + (nClock_Mode<<5) + (temp_data&0x1F);
			if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, temp_data))
			{
				LldPrint_Error("dac-ctl : pos...", 6, 11);
				return -1;
			}
	
			LldPrint_1x("[LLD]TVB593...", (unsigned int)temp_data);
		}
		else
	
		if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, nClock_Mode) )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 12);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(50);
	
		dwSYSCLK /= 1000000.0;
	
		if ( nCIC_Interpolation == 1 )
		{
			nBYPASS_INV_CIC = 1;
		}
	}
	else if ( modulation_mode == TVB380_DVBS2_MODE )
	{
		if (symbol_freq < 1000000 || symbol_freq > 45000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 13);
			return -1;
		}
		
		if ( gRRC_Filter == ROLL_OFF_FACTOR_NONE ) //RRC NONE
		{
			dwData = 1 * symbol_freq;
		}
		else 
		{
			dwData = 2 * symbol_freq;
		}
		nCIC_Interpolation = (int)(floor(50000000. / dwData));
		if ( symbol_freq < 5000000 ) //< 5MSps
		{
			if ( gRRC_Filter == ROLL_OFF_FACTOR_NONE) //RRC NONE
			{
				if ( nCIC_Interpolation > 10 )
					nCIC_Interpolation = 10;
			}
			else
			{
				if ( nCIC_Interpolation > 5 )
					nCIC_Interpolation = 5;
			}
		}
		else
		{
			if ( nCIC_Interpolation > 20 )
				nCIC_Interpolation = 20;
		}
		
		dwSYSCLK = dwData * 4 * nCIC_Interpolation;
		
		if ( gRRC_Filter == ROLL_OFF_FACTOR_NONE) //RRC NONE
		{
			if ( symbol_freq < 10000000 ) //< 10MSps
			{
				nClock_Mode = 1;
				dwData = 2 * symbol_freq;
			}
			else 
			{
				nClock_Mode = 0;
				dwData = 1 * symbol_freq;
			}
		}
		else
		{
			nClock_Mode = 1;
			dwData = 2 * symbol_freq;
		}
		nREF_Multiplier = (int)(dwSYSCLK / (double)dwData);

		if ( IsAttachedBdTyp_UseIndirectRegAccess() )
		{
			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
			LldPrint_1x("[LLD]TVB593 MODULATOR PARAM/AD9857_ADDR_QPSK_CLOCK_MODE=", (unsigned int)temp_data);

			temp_data = (nClock_Mode<<11) + (temp_data&0x7FF);
			if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, temp_data))
			{
				LldPrint_Error("dac-ctl : pos...", 6, 14);
				return -1;
			}

			LldPrint_1x("[LLD]TVB593...", (unsigned int)temp_data);
		}
		else if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, nClock_Mode) )
		{
			LldPrint_Error("dac-ctl : pos...", 6, 15);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(50);
		
		dwSYSCLK /= 1000000.0;
	}
	else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
	{
		nREF_Multiplier = 4;
		nCIC_Interpolation = 4;
		dwSYSCLK = (double)4.5 * (684./286.) * 4 * nREF_Multiplier;
		if (IsAttachedBdTyp_594())
		{
			nCIC_Interpolation = 1;
			dwSYSCLK = (double)4.5 * (684./286.) * 3 * nREF_Multiplier;
		}
	}
	else if ( modulation_mode == TVB380_TDMB_MODE )
	{
		nREF_Multiplier = 16;
		nCIC_Interpolation = 4;
		dwSYSCLK = 2.048 * 4 * nREF_Multiplier;
	}
	else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
		nREF_Multiplier = 10;
		nCIC_Interpolation = 5;
		dwSYSCLK = 16.253968 * nREF_Multiplier;
	}
	else if ( modulation_mode == TVB380_DTMB_MODE )
	{
		nREF_Multiplier = 12;
		nCIC_Interpolation = 3;
		if(symbol_freq == 0)
			dwSYSCLK = ((60.48 * 6. / 8.) / 4.) * nREF_Multiplier;
		else if(symbol_freq == 1)
			dwSYSCLK = ((60.48 * 7. / 8.) / 4.) * nREF_Multiplier;
		else
			dwSYSCLK = 15.12 * nREF_Multiplier;
	}
	else if ( modulation_mode == TVB380_CMMB_MODE )
	{
		nREF_Multiplier = 10;
		nCIC_Interpolation = 5;

		dwSYSCLK = 20.* nREF_Multiplier;
	}
	else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
	{
		nREF_Multiplier = 20;
		nCIC_Interpolation = 5;

		if ( symbol_freq == DVB_T_5M_BAND )
			symbol_freq = 5;
		/*
		else if ( symbol_freq == DVB_T_1_7M_BAND )
			symbol_freq = 1.7;
		else if ( symbol_freq == DVB_T_10M_BAND )
		{
			symbol_freq = 10.;
			nREF_Multiplier = 16;
		}
		*/		
		else
			symbol_freq += 6;//symbol_freq == bandwidth
		dwSYSCLK = (double)symbol_freq * (8./7.) * nREF_Multiplier;
	}
	else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
	{
		/*
		nREF_Multiplier = ((int)(200000000. / symbol_freq) / 4) * 4;
		nCIC_Interpolation = nREF_Multiplier / 4;
		dwSYSCLK = (double)(symbol_freq / 1000000.) * nREF_Multiplier;
		*/
		if (symbol_freq < 4000000 || symbol_freq > 16000000)
		{
			LldPrint_Error("dac-ctl : pos...", 6, 16);
			return -1;
		}

		dwQsymbol = symbol_freq;
		nCIC_Interpolation = (int)(floor(50000000. / dwQsymbol));
		if ( nCIC_Interpolation > 5 )
			nCIC_Interpolation = 5;
				
		dwSYSCLK = dwQsymbol * 4 * nCIC_Interpolation;
		nREF_Multiplier = (int)(dwSYSCLK / (double)dwQsymbol);

		dwSYSCLK /= 1000000.0;
	}
	else if ( modulation_mode == TVB380_ISDBS_MODE )
	{
		nREF_Multiplier = 10;
		nCIC_Interpolation = 5;
		dwSYSCLK = 16.253968 * nREF_Multiplier;
	}
	else
	{
		LldPrint_Error("dac-ctl : pos...", 6, 17);
		return -1;	
	}

	//FIXED - IF +- offset
	/*
	if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
		dwOutputFTW = ((gChannel_Freq_Offset-gRF_143KHZ) / 1000000.0)*4294967296./dwSYSCLK;
	}
	else

	dwOutputFTW = (gChannel_Freq_Offset / 1000000.0)*4294967296./dwSYSCLK;
	*/
	//dwOutputFTW = ((gChannel_Freq_Offset-gRF_143KHZ) / 1000000.0)*4294967296./dwSYSCLK;
	dwOutputFTW = ((gChannel_Freq_Offset + (gRF_143KHZ_sign*gRF_143KHZ)) / 1000000.0)*4294967296./dwSYSCLK;

	llOutputFTW = (__int64)dwOutputFTW;

	LldPrint_4("CENTER_FREQ_OFFSET, MULTIPLIER, CIC_INTER, SYSCLK", gChannel_Freq_Offset, nREF_Multiplier, nCIC_Interpolation, dwSYSCLK);
	LldPrint_2("FTW(float/long) ", (int)dwOutputFTW, (int)llOutputFTW);
	LldPrint("Control AD9857. using the calculated dac para.");

	// write to register
	dwData = 0x00002080;
	dwData += nREF_Multiplier;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 18);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 19);
		return -1;
	}

	dwData = 0x00002100;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 20);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 21);
		return -1;
	}

	dwData = 0x00002200;
	nOutputFTW= (int)(llOutputFTW >> 0) & 0x000000FF;
	dwData += nOutputFTW;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 22);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 23);
		return -1;
	}

	dwData = 0x00002300;
	nOutputFTW= (int)(llOutputFTW >> 8) & 0x000000FF;
	dwData += nOutputFTW;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 24);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 25);
		return -1;
	}

	dwData = 0x00002400;
	nOutputFTW= (int)(llOutputFTW >> 16) & 0x000000FF;
	dwData += nOutputFTW;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 26);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 27);
		return -1;
	}

	dwData = 0x00002500;
	nOutputFTW= (int)(llOutputFTW >> 24) & 0x000000FF;
	dwData += nOutputFTW;
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 28);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 29);
		return -1;
	}

	dwData = 0x00002600;
	dwData += nBYPASS_INV_CIC;

//////////////////////////////////////////////////////////
	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin() )
	{
		if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk() )
		{
			//////////////////////////////////////////////////////////
			//950~2150MHz support
			if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (gSpectral_Inversion << 1);
				}
				else
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
			}
			else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
			}
			else if ( modulation_mode == TVB380_TDMB_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
			}
			else if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE )
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (gSpectral_Inversion << 1);
				}
				else
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
			}
			else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
			}
			else if ( modulation_mode == TVB380_DTMB_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
			}
			else if ( modulation_mode == TVB380_CMMB_MODE )
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (gSpectral_Inversion << 1);
				}
				else
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
			}
			//DVB-T2
			else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (gSpectral_Inversion << 1);
				}
				else
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
			}
			//I/Q PLAY/CAPTURE
			else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (gSpectral_Inversion << 1);
				}
				else
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
			}
			//ISDB-S ???
			else if ( modulation_mode == TVB380_ISDBS_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
			}
			else
			{
				if ( dwfreq >= (RF_1GHZ+gChannel_Freq_Offset) )
				{
					dwData += (((gSpectral_Inversion + 1) % 2) << 1);
				}
				else
				{
					dwData += (gSpectral_Inversion << 1);
				}
			}
			//////////////////////////////////////////////////////////
		}
		else
		{
			if ( modulation_mode == TVB380_DVBT_MODE 
				|| modulation_mode == TVB380_DVBH_MODE )
			{
				dwData += (((gSpectral_Inversion + 1) % 2) << 1);
			}
			else if ( modulation_mode == TVB380_TDMB_MODE )
			{
				dwData += (gSpectral_Inversion << 1);
				//if ( dwfreq >= 1036000000 )
				//	dwData += (0/*==gSpectral_Inversion*/ << 1);
				//else
				//	dwData += (1/*==gSpectral_Inversion*/ << 1);
			}
			else
			{
				dwData += (gSpectral_Inversion << 1);
			}
		}
	}
	else
	{
		dwData += (gSpectral_Inversion << 1);
	}

	dwData += (nCIC_Interpolation << 2);
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 30);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 31);
		return -1;
	}

	dwData = 0x000027FF;

	//IF/RF SCALE
	if ( IsAttachedBdTyp_380v3_ifrf_scaler() )
	{
		dwData = 0x00002700;
		if ( dwData == 0x00002700 )
		{
			if ( modulation_mode == TVB380_DVBT_MODE )
				dwData += 0xC8;//200
			else if ( modulation_mode == TVB380_QPSK_MODE )
				dwData += 0x8C;//140
			else if ( modulation_mode == TVB380_QAMB_MODE )
				dwData += 0x07;//7
			else if ( modulation_mode == TVB380_QAMA_MODE )
			{
				nParam = TSPL_GET_READ_PARAM();
				nParam &= 0x07; //Constellation Mode
				
				if ( nParam == 0 ) dwData += 0x50;//80
				else if ( nParam == 1 ) dwData += 0x64;//100
				else if ( nParam == 2 ) dwData += 0x50;//80
				else if ( nParam == 3 ) dwData += 0x1E;//30
				else if ( nParam == 4 ) dwData += 0x50;//80
			}
			//ATSC-M/H
			else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE)
			{
				dwData += 0xFF;//255
			}
			else if ( modulation_mode == TVB380_TDMB_MODE )
			{
				if ( dwfreq >= 1036000000 )
					dwData += 0xFF;//255
				else
					dwData += 0x80;//128
			}
		}
	}

//////////////////////////////////////////////////////////
	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin() )
	{
#if defined(WIN32)
		unsigned long dwSize;
		unsigned char regBuf[10];
		HKEY	hndKey;
		char	szRegString_Path[255];

		sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nBoardLocation, szRegString_KEY[modulation_mode]);
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nMaxBoardCount, szRegString_KEY[modulation_mode]);
		}	

		temp_data = RegOpenKey(HKEY_CURRENT_USER,szRegString_Path,&hndKey);
		if ( temp_data == ERROR_SUCCESS)
		{
			temp_data = RegQueryValueEx(hndKey, szRegStr_RfOutScale, NULL, NULL, (LPBYTE)regBuf, &dwSize);
			RegCloseKey(hndKey);
			if ( temp_data == ERROR_SUCCESS)
			{
				dwData = atoi((char*)regBuf);
				if ( dwData > 0 )
				{
					dwData += 0x00002700;
				}
				else
				{
					dwData = 0x00002700;
				}
			}
			else
			{
				dwData = 0x00002700;
			}
		}
		else
#endif

		dwData = 0x00002700;
		if ( dwData == 0x00002700 )
		{
			if ( modulation_mode == TVB380_DVBT_MODE 
				|| modulation_mode == TVB380_DVBH_MODE )
				dwData += 0xD2;//210
			else if ( modulation_mode == TVB380_QPSK_MODE 
				|| modulation_mode == TVB380_DVBS2_MODE )
				dwData += 0xFF;//255
			else if ( modulation_mode == TVB380_QAMB_MODE ||  modulation_mode == TVB380_MULTI_QAMB_MODE)
				dwData += 0xD2;//210
			else if ( modulation_mode == TVB380_QAMA_MODE )
			{
				nParam = TSPL_GET_READ_PARAM();
				nParam &= 0x07; //Constellation Mode
				
				if ( nParam == 0 ) dwData += 0xD2;//210
				else if ( nParam == 1 ) dwData += 0xFA;//250
				else if ( nParam == 2 ) dwData += 0xD2;//210
				else if ( nParam == 3 ) dwData += 0xFA;//250
				else if ( nParam == 4 ) dwData += 0xD2;//210
			}
			//ATSC-M/H
			else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
			{
				dwData += 0xFF;//255
			}
			else if ( modulation_mode == TVB380_TDMB_MODE )
			{
				if ( dwfreq >= 1036000000 )
					dwData += 0xFF;//255
				else
					dwData += 0xFF;//255
			}
			
			if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk() )
			{
				dwData = 0x000027FF;

				//////////////////////////////////////////////////////////
				//950~2150MHz support
				if ( modulation_mode == TVB380_DVBS2_MODE || modulation_mode == TVB380_QPSK_MODE )
				{
					//950 ~ 963(955), 2037(2045) ~ MHz
					if ( (dwfreq >= RF_MIN_DVB_S && dwfreq < (RF_1GHZ - gChannel_Freq_Offset)) || dwfreq > (RF_2GHZ/* + gChannel_Freq_Offset*/) )
						dwData = 0x00002723;

					if ( IsAttachedBdTyp_DacAdjAccordingToBaseBandFreq() )
					{
						//950 ~ 963(955)
						if ( (dwfreq >= RF_MIN_DVB_S && dwfreq < (RF_1GHZ - gChannel_Freq_Offset)) )
						{
							if ( IsAttachedBdTyp_597() )
							{
								//Active Mixer
								if ( TSPL_nBoardRevision_Ext == TVB597A_REV_EXT )
								{
									dwData = 0x00002741;//65(decimal)
								}
							}
							else if ( IsAttachedBdTyp_595v3() )
							{
								//S/S2 0dBm
								if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
								{
									dwData = 0x000027FF;
								}
							}
							else if ( IsAttachedBdTyp_595v4() )
							{
								//Active Mixer
								if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
								{
									dwData = 0x00002741;
								}

								//S/S2 0dBm
								if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
								{
									dwData = 0x000027FF;
								}
							}
							//TVB590B, TVB590C
							else if ( IsAttachedBdTyp_590v10() )
							{
								//Active Mixer
								if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
								{
									dwData = 0x00002741;
								}

								//S/S2 0dBm
								if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
								{
									dwData = 0x000027FF;
								}
							}
						}
					}
				}
				else
				{
					//2037(2045) ~  MHz
					if ( dwfreq > (RF_2GHZ/* + gChannel_Freq_Offset*/) )
						//dwData = 0x00002719;
						dwData = 0x00002723;
				}
				//////////////////////////////////////////////////////////

				if ( IsAttachedBdTyp_DacAdjAccordingToBaseBandFreq() )	//	???
				{
					if ( dwfreq > RF_2GHZ ) //2.05 >= 
					{
						//TVB595D - 597A
						if ( IsAttachedBdTyp_597() )
						{
							//Active Mixer
							if ( TSPL_nBoardRevision_Ext == TVB597A_REV_EXT )
							{
								dwData = 0x00002741;//65(decimal)
							}
						}
						//TVB595B
						else if ( IsAttachedBdTyp_595v3() )
						{
							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								dwData = 0x000027FF;
							}
						}
						//TVB595C
						else if ( IsAttachedBdTyp_595v4() )
						{
							//Active Mixer
							if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
							{
								dwData = 0x00002741;
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								dwData = 0x000027FF;
							}
						}
						//TVB590B, TVB590C
						else if ( IsAttachedBdTyp_590v10() )
						{
							//Active Mixer
							if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
							{
								dwData = 0x00002741;
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
							{
								dwData = 0x000027FF;
							}
						}
					}
				}
				
				if ( IsAttachedBdTyp_390v7_IF_only() )
				{
					dwData = 0x00002700;
					if ( modulation_mode == TVB380_DVBT_MODE 
						|| modulation_mode == TVB380_DVBH_MODE )
						dwData += 0x64;//100
					else if ( modulation_mode == TVB380_QPSK_MODE 
						|| modulation_mode == TVB380_DVBS2_MODE )
					{
						if ( modulation_mode == TVB380_DVBS2_MODE 
							&& gRRC_Filter == ROLL_OFF_FACTOR_NONE) //RRC NONE
						{
							dwData += 0x90;//144
						}
						else
						{
							dwData += 0xFF;//255
						}
					}
					else if ( modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE )
						dwData += 0x1E;//30
					else if ( modulation_mode == TVB380_QAMA_MODE )
					{
						nParam = TSPL_GET_READ_PARAM();
						nParam &= 0x07; //Constellation Mode
						
						if ( nParam == 0 ) dwData += 0x2F;//47
						else if ( nParam == 1 ) dwData += 0x2F;//47
						else if ( nParam == 2 ) dwData += 0x2F;//47
						else if ( nParam == 3 ) dwData += 0x2F;//47
						else if ( nParam == 4 ) dwData += 0x2F;//47
					}
					//ATSC-M/H
					else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
					{
						dwData += 0x64;//100
					}
					else if ( modulation_mode == TVB380_TDMB_MODE )
					{
						if ( dwfreq >= 1036000000 )
							dwData += 0x50;//80
						else
							dwData += 0x50;//80
					}
				}
			}
		}
	}

	//SCALE ADJUST for .1dB CONTROL
#if 1
	if ( (int)g_ScaleOffset >= 0 && (int)g_ScaleOffset <= 0xFF )
	{
		dwData -= (int)g_ScaleOffset;
	}
#endif

	//DVB-T2
	if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
	{
		//dwData = 0x000027B5;
		dwData = 0x000027FF;
	}
	LldPrint_1x("SET AD9857 REGISTER", dwData);
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 32);
		return -1;
	}

	dwData = 0x00000001;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 33);
		return -1;
	}

	dwData = 0x00004000;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 34);
		return -1;
	}

	dwData = 0x00000000;
	if ( TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_REG_CONTROL, dwData) )
	{
		LldPrint_Error("dac-ctl : pos...", 6, 35);
		return -1;
	}

	return	0;
}

/*^^***************************************************************************
 * Description : Control AD9857
 *				
 * Entry : modulation_mode, dwfreq, symbol_freq
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CDacCtl::CntlDac_AD9xxx(long modulation_mode, unsigned long dwfreq, long symbol_freq)	//2150MHz support - int -> unsigned long
{

	LldPrint_FCall("CntlDac_AD9xxx", modulation_mode, dwfreq);
	DBG_PLATFORM_BRANCH();

	if ( IsAttachedBdTyp_UseAD9775() )
	{
		CntlDac_AD9775(modulation_mode, dwfreq, symbol_freq);
	}
	else if( IsAttachedBdTyp_499() )
	{
		Configure_AD9787_Register(modulation_mode, dwfreq, symbol_freq);
	}
	else
	{
		CntlDac_AD9857(modulation_mode, dwfreq, symbol_freq);
	}

	return 0;
}


//2013/1/4 TVB499 added ===============================================================
int _stdcall CDacCtl::TSPL_GET_AD9787(long reg)
{
	int nRet = 0;
	long AD9775;
	AD9775 = reg;

	//DBG_PRINT_NOISY(dbg_noisy, "TSPL_GET_AD9787", reg, 0, (double)0, NULL);
	//DBG_PRINT(dbg_notice, "TSPL_GET_AD9787", reg, 0);
	nRet = TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_DAC_CONTROL, AD9775);
	//	LPRINT(fn_Log, "TSPL_SET_AD9775::address=0x%x, reg=0x%x,0x%x\n", PCI590S_REG_DAC_CONTROL, reg, AD9775);
	Sleep(1);//???
	nRet = WDM_Read_TSP(PCI593_REG_DAC_DATA_READ);
	//DBG_PRINT(dbg_notice, "TSPL_GET_AD9787", nRet, 0);
	return (nRet & 0xFFFFFFFF);
}

int _stdcall CDacCtl::TSPL_SET_AD9787(long reg, long data)
{
	int nRet = 0;
	long AD9775;

	//DBG_PRINT_NOISY(dbg_notice, "TSPL_SET_AD9787", reg, data, 0, NULL);
	//DBG_PRINT(dbg_notice, "TSPL_SET_AD9787", reg, (unsigned long)data);
	TSPL_SET_CONFIG_DOWNLOAD(reg, (unsigned long)data);
	Sleep(1);//???

	if ( reg == -1 )
	{
		AD9775 = 0;
		TSPL_SET_CONFIG_DOWNLOAD(reg, AD9775);
		LldPrint_2("[LLD]Set AD9787 :: reg, data ", reg, data);
		//LPRINT(fn_Log, "TSPL_SET_AD9787::address=0x%x, reg=0x%x,data=0x%x, 0x%x\n", PCI590S_REG_DAC_CONTROL, reg, data, AD9775);
		Sleep(1);//???
	}
	return nRet;
}

int	CDacCtl::Configure_AD9787_Register(long modulation_mode, 
									 unsigned long dwfreq, //2150MHz support - int -> unsigned long
									 long symbol_freq)
{
	DWORD temp_data;
	int nREF_Multiplier=0, nCIC_Interpolation=0;
	int nClock_Mode=0, nBYPASS_INV_CIC=0;
	int nPLL_VCO_Divisor, nPLL_Loop_Divisor, nPLL_enable, nDAC_Interpolation;
	int nPLL_Band_Select;

	//DBG_PRINT(dbg_notice, "Configure_AD9787_Register", modulation_mode, dwfreq);
	DBG_PLATFORM_BRANCH();
#if defined(WIN32)
	char	szRegString_Path[255];

	sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nMaxBoardCount, szRegString_KEY[modulation_mode]);
#endif
	//TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_499() )
	{
		double Interpol_rate;
		double fDDS, fDATA, fDAC;
		double symbol_clock;
		int RF_sw = 0, N, reg_data;
		//int Coeff_Val, Coeff_Addr;
		int fVCO = 0;
		nPLL_enable = 0;
		//LPRINT(fn_Log,"[LLD]===CONFIG AD9787, Type[%d], RF[%d], BW/SYMBOLRATE[%d]\n", (int)modulation_mode, (int)dwfreq, (int)symbol_freq);
		
		Interpol_rate = 8.;
		nDAC_Interpolation = 3;
		if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
		{
			symbol_clock = (double)symbol_freq;
			if ( symbol_freq == DVB_T_5M_BAND ) //DVB-H, 5MHz
				symbol_clock = 5;
			else
				symbol_clock += 6;//symbol_freq == bandwidth

			symbol_clock = symbol_clock*8;
		}
		//ATSC-M/H
		else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE )
		{
			symbol_freq = 4500000;
			symbol_clock = (symbol_freq / 1000000.0) * (684./286.) * 4.;
		}
		else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE )
		{
			if (symbol_freq < 1000000 || symbol_freq > 8000000)
				return -1;
			if( symbol_freq < 2000000 )
			{
				N = 8;
				nClock_Mode = 3;
			}
			else
			{
				nClock_Mode = 2;
				N = 4;
			}
			
			if(	modulation_mode == TVB380_QAMB_MODE )
			{
				nClock_Mode = 2;
				N = 4;
			}
			
			nPLL_enable = 1;
			symbol_clock = 2*symbol_freq*N;
			if ( symbol_clock  > 90000000 )
			{
				//LPRINT(fn_Log,"Invalid data rate [%f Sps]\n", symbol_clock);
				return -1;
			}
			symbol_clock /= 1000000.;

			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QAM_CLOCK_MODE);
			//LPRINT(fn_Log,"[LLD]TVB593 MODULATOR PARAM/AD9857_ADDR_QPSK_CLOCK_MODE=0x%X\n", (unsigned int)temp_data);
			if ( modulation_mode == TVB380_QAMA_MODE )
				temp_data = (nClock_Mode<<5) + (temp_data & 0x1F);
			else
				temp_data = (nClock_Mode<<7) + (temp_data&0x7F);
			
			if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QAM_CLOCK_MODE, temp_data))
			{
				//LPRINT(fn_Log,"TSPL_SET_CONFIG_DOWNLOAD::Failed[0x%X]\n", (int)temp_data);
				return -1;
			}

			//LPRINT(fn_Log,"[LLD]TVB499...0x%x\n", (unsigned int)temp_data);
		}
		else if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
		{
			if (symbol_freq < 1000000 || symbol_freq > 45000000)
				return -1;

			if ( symbol_freq < 2000000 )
			{
				N=8;
				nClock_Mode = 3;
			}
			else if ( symbol_freq < 10000000 )
			{
				N=4;
				nClock_Mode = 2;
			}
			else if ( symbol_freq < 20000000 )
			{
				N=2;
				nClock_Mode = 1;
			}
			else if ( symbol_freq <= 45000000 )
			{
				N=1;
				nClock_Mode = 0;
			}
			else
			{
				//LPRINT(fn_Log, "Invalid symbol rate [%d Sps]\n", (int)symbol_freq);
				return -1;
			}
			nPLL_enable = 1;
			gClock_Mode = nClock_Mode;
			
			symbol_clock = 2*symbol_freq*N;
			//TEST - SCLEE
			symbol_clock = symbol_freq*N;
			if ( symbol_clock  > 90000000 )
			{
				//LPRINT(fn_Log,"Invalid data rate [%f Sps]\n", symbol_clock);
				return -1;
			}
			symbol_clock /= 1000000.;

			temp_data = WDM_Read_TSP_Indirectly(AD9857_ADDR_QPSK_CLOCK_MODE);
			//LPRINT(fn_Log,"[LLD]TVB593 MODULATOR PARAM/AD9857_ADDR_QPSK_CLOCK_MODE=0x%X\n", (unsigned int)temp_data);

			if ( modulation_mode == TVB380_QPSK_MODE )
				temp_data = (((temp_data>>7)&0x03)<<7) + (nClock_Mode<<5) + (temp_data&0x1F);
			else
				temp_data = (((temp_data>>13)&0x01)<<13) + (nClock_Mode<<11) + (temp_data&0x7FF);
			
			if(TSPL_SET_CONFIG_DOWNLOAD(AD9857_ADDR_QPSK_CLOCK_MODE, temp_data))
			{
				//LPRINT(fn_Log,"TSPL_SET_CONFIG_DOWNLOAD::Failed[0x%X]\n", (int)temp_data);
				return -1;
			}

			//LPRINT(fn_Log,"[LLD]TVB593...0x%x\n", (unsigned int)temp_data);
		}
		else if ( modulation_mode == TVB380_TDMB_MODE )
		{
			symbol_freq = 2048000;
			symbol_clock = (symbol_freq / 1000000.0) * 12.;
		}
		else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
#ifdef WIN32
			if(symbol_freq == 0)
				symbol_clock = 65.015872;	//6MHz
			else if(symbol_freq == 1)
				symbol_clock = 75.851851;	//7MHz
			else
				symbol_clock = 86.68783;		//8MHz
#else
			symbol_clock = 65.015872;
#endif
		}
		else if ( modulation_mode == TVB380_DTMB_MODE )
		{
			Interpol_rate = 4.;
			nDAC_Interpolation = 2;
			symbol_clock = 60.48;
		}
		else if ( modulation_mode == TVB380_CMMB_MODE )
		{
			symbol_clock = 60.0;
		}
		//DVB-T2
		else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
		{
			symbol_clock = (double)symbol_freq;
			if ( symbol_freq == DVB_T_5M_BAND ) //DVB-H, 5MHz
				symbol_clock = 5;
			/*
			else if ( symbol_freq == DVB_T_1_7M_BAND )
				symbol_clock = 1.7;
			else if ( symbol_freq == DVB_T_10M_BAND )
				symbol_clock = 10.;
			*/
			else
				symbol_clock += 6;//symbol_freq == bandwidth
			symbol_clock = symbol_clock*8.*(8./7.);
		}
		//I/Q PLAY/CAPTURE - ???
		else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
		{
			symbol_clock = (double)(symbol_freq / 1000000.);
		}
		//ISDB-S
		else if ( modulation_mode == TVB380_ISDBS_MODE )
		{
			nPLL_enable = 1;
			symbol_clock = 57.72;//28.860*2
		}
		
		if(Interpol_rate == 16)
			nPLL_Loop_Divisor = 3;
		else if(Interpol_rate == 8)
			nPLL_Loop_Divisor = 2;
		else if(Interpol_rate == 4)
			nPLL_Loop_Divisor = 1;
		else
			nPLL_Loop_Divisor = 0;
		fDDS = symbol_clock;//Mhz
		fDATA = fDDS;
		if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
		{
			fDATA /= 7.;
		}
		//ATSC-M/H
		else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE )
		{
			fDATA /= 4.;
		}
		else if ( modulation_mode == TVB380_TDMB_MODE )
		{
			fDATA /= 3.;
		}
		//TEST - SCLEE
		else if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
		{
			fDATA *= 2;
		}
		//TVB593
		else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
			fDATA /= 8.;//8,126,984 Hz
		}
		else if ( modulation_mode == TVB380_DTMB_MODE )
		{
			fDATA = 15.12;//Interpol_rate:8(2^3), PLL_Divider:2(2^1)
		}
		else if ( modulation_mode == TVB380_CMMB_MODE )
		{
			fDATA = 10.;//Interpol_rate:8(2^3), PLL_Divider:2(2^2)
		}
		else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
		{
			fDATA /= 8.;
		}

		fDAC = fDATA * Interpol_rate;
		
		if(nPLL_enable == 0)
		{
			fVCO = 0;
		}
		else
		{
			if(fDAC < 250.)
			{
				nPLL_VCO_Divisor = 3;
				fVCO = (int)fDAC * 8; 
			}
			else if( fDAC < 500.)
			{
				nPLL_VCO_Divisor = 2;
				fVCO = (int)fDAC * 4;
			}
			else if( fDAC < 1000.)
			{
				nPLL_VCO_Divisor = 1;
				fVCO = (int)fDAC * 2;
			}
			else 
			{
				nPLL_VCO_Divisor = 0;
				fVCO = (int)fDAC * 1;
			}
			
		}
		//RESET
		reg_data = 0x04000000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//release RESET
		reg_data = 0x00000000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//AD9787 ADDRESS 0x00 1 BYTE
		reg_data = 0x02000000;
		TSPL_SET_AD9787(PCI593_REG_DAC_PLL_LOCK_CONTROL, reg_data);
		reg_data = 0x02200000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//AD9787 ADDRESS 0x01 2 BYTE
		reg_data = (0x33 << 24) + (nDAC_Interpolation << 22);
		TSPL_SET_AD9787(PCI593_REG_DAC_PLL_LOCK_CONTROL, reg_data);
		reg_data = 0x02410000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//AD9787 ADDRESS 0x02 2 BYTE
		reg_data = 0x00040000;
		TSPL_SET_AD9787(PCI593_REG_DAC_PLL_LOCK_CONTROL, reg_data);
		reg_data = 0x02420000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//AD9787 ADDRESS 0x03 4 BYTE
		reg_data = 0x80000400;
		TSPL_SET_AD9787(PCI593_REG_DAC_PLL_LOCK_CONTROL, reg_data);
		reg_data = 0x02830000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		Sleep(1000);
		//AD9787 ADDRESS 0x04 3 BYTE
		nPLL_Band_Select = 0x3F;
		reg_data = (0x6F << 24) + ((nPLL_enable & 0x01) << 23) + ((nPLL_VCO_Divisor & 0x03) << 21) + 
			((nPLL_Loop_Divisor & 0x03) << 19) + (0x3 << 16) + ((nPLL_Band_Select & 0x3F) << 10) + (0x3 << 8);
		TSPL_SET_AD9787(PCI593_REG_DAC_PLL_LOCK_CONTROL, reg_data);
		reg_data = 0x02640000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);
		//release RESET
		reg_data = 0x00000000;
		TSPL_SET_AD9787(PCI593_REG_DAC_CONTROL, reg_data);

		//READ DATA
		//AD9787 ADDRESS 0x00 1 BYTE
		reg_data = 0x03200000;
		reg_data = TSPL_GET_AD9787(reg_data);
		//LPRINT(fn_Log,"[AD9787] reg0 = 0x%x\n", reg_data);
		//AD9787 ADDRESS 0x01 2 BYTE
		reg_data = 0x03410000;
		reg_data = TSPL_GET_AD9787(reg_data);
		//LPRINT(fn_Log,"[AD9787] reg1 = 0x%x\n", reg_data);
		//AD9787 ADDRESS 0x02 2 BYTE
		reg_data = 0x03420000;
		reg_data = TSPL_GET_AD9787(reg_data);
		//LPRINT(fn_Log,"[AD9787] reg2 = 0x%x\n", reg_data);
		//AD9787 ADDRESS 0x03 4 BYTE
		reg_data = 0x03830000;
		reg_data = TSPL_GET_AD9787(reg_data);
		//LPRINT(fn_Log,"[AD9787] reg3 = 0x%x\n", reg_data);
		//AD9787 ADDRESS 0x04 3 BYTE
		reg_data = 0x03640000;
		reg_data = TSPL_GET_AD9787(reg_data);
		//LPRINT(fn_Log,"[AD9787] reg4 = 0x%x\n", reg_data);

		return 0;
	}


	//LPRINT(fn_Log,"Invalid TVB499\n");

	return -1;
}

