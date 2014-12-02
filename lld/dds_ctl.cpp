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
#include	"dll_error.h"
#include	"../include/logfile.h"
#include	"wdm_drv.h"
#include	"dma_drv.h"
#include	"mainmode.h"
#endif

#include	"Reg590S.h"
#include	"Reg593.h"
#include	"rf_level_tbl.h"
#include 	"dds_ctl.h"

#include	"../include/system_const.h"

#define		CLK_CONST_DEF(a,b,c)	pow((double)a,b) / (double)c

#define 	MAX_CNR 50
#define AD9857_SCALE_OFFSET	3


/////////////////////////////////////////////////////////////////
#ifdef WIN32
extern	TCHAR	szCurDir[_MAX_PATH];
#endif
/////////////////////////////////////////////////////////////////
CDdsCtl::CDdsCtl(void)
{


}
CDdsCtl::~CDdsCtl()
{
}

void CDdsCtl::RdRegistry_FreqStep(int modulation_mode, int	*_step, unsigned long *_sht)
{
	*_step = UMC_STEP_FREQ;		//	default
	if ( modulation_mode == TVB380_TDMB_MODE )
	{
		*_step = 8;//8KHz
	}

	//2011/5/9 DVB-C2
	if(modulation_mode == TVB380_DVBC2_MODE)
	{
		*_step = 10;//50KHz -> 10KHz
	}

#ifdef WIN32
	int	reg_data = 0;
	unsigned long dwSize;
	unsigned char regBuf[10];
	HKEY	hndKey;
	char	szRegString_Path[255];

	sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nBoardLocation, szRegString_KEY[modulation_mode]);
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nMaxBoardCount, szRegString_KEY[modulation_mode]);
	}

	reg_data = RegOpenKey(HKEY_CURRENT_USER,szRegString_Path,&hndKey);
	if ( reg_data == ERROR_SUCCESS)
	{
		reg_data = RegQueryValueEx(hndKey, szRegStr_RfOutStep, NULL, NULL, (LPBYTE)regBuf, &dwSize);
		RegCloseKey(hndKey);
		if ( reg_data == ERROR_SUCCESS)
		{
			reg_data = atoi((char*)regBuf);
			if ( reg_data >= 1 && reg_data <= 1000 ) // 1~1000KHz
			{
				*_step = reg_data;
			}
		}
	}

	//ISDB-T 143KHz SHIFT
	if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
		//FIXED - IF +- offset
		//gRF_143KHZ = RF_143KHZ;
		reg_data = RegOpenKey(HKEY_CURRENT_USER,szRegString_Path,&hndKey);
		if ( reg_data == ERROR_SUCCESS)
		{
			reg_data = RegQueryValueEx(hndKey, szRegStr_Rfshft, NULL, NULL, (LPBYTE)regBuf, &dwSize);
			RegCloseKey(hndKey);
			if ( reg_data == ERROR_SUCCESS)
			{
				reg_data = atoi((char*)regBuf);
				if ( reg_data == 0 )
				{
					*_sht = 0;
				}
			}
		}
	}

#endif

}
void CDdsCtl::RdRegistry_SymClkAdj(int modulation_mode, int *_adj)
{
	*_adj = 1;	//	default enabled.

#ifdef WIN32
	int	reg_data = 0;
	unsigned long dwSize;
	unsigned char regBuf[10];
	HKEY	hndKey;
	char	szRegString_Path[255];

	sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nBoardLocation, szRegString_KEY[modulation_mode]);

	if ( IsAttachedBdTyp_UsbTyp() )
	{
		sprintf(szRegString_Path, "%s%s_%d\\%s", szRegString_ROOT_KEY, szRegString_PARENT_KEY[3], TSPL_nMaxBoardCount, szRegString_KEY[modulation_mode]);
	}

	reg_data = RegOpenKey(HKEY_CURRENT_USER,szRegString_Path,&hndKey);
	if ( reg_data == ERROR_SUCCESS)
	{
		reg_data = RegQueryValueEx(hndKey, szRegStr_SysClkAdj, NULL, NULL, (LPBYTE)regBuf, &dwSize);
		RegCloseKey(hndKey);
		if ( reg_data == ERROR_SUCCESS)
		{
			reg_data = atoi((char*)regBuf);
			if ( reg_data == 0 )
			{
				*_adj = 0;
			}
		}
	}
#endif
}
void CDdsCtl::CalcClkCntlCnst_(int modulation_mode, int _en_adj, int _sym_freq, int *_cntl, double *_cnst)
{
	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis() )
	{
		*_cntl = AD9852AST_SYMCLK_CONTROL_REGISTER_VALUE;
		*_cnst = CLK_CONST_DEF(2,48,192);//AD9852AST
	
		if ( IsAttachedBdTyp_AD9852Clk_ASx() )
		{
			if(IsAttachedBdTyp_AD9852Clk_ASx_V())	//	597v2 rev2.1, 593 rev2.2, 590s rev2.2
			{
				unsigned long dwStatus = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);
				LldPrint_1("[LLD] AD9852 INFO VALUE", dwStatus);
				if((dwStatus & 0x3) == 0x1)
				{
					LldPrint("[LLD] 10MHz AD9852 REF INPUT");
				}
				else
				{
					LldPrint("[LLD] 20MHz AD9852 REF INPUT");
				}
	
				if(((dwStatus >> 2) & 0x3) == 0x1)
				{
					LldPrint("[LLD] AD9852 ASV model(300MHz)");
				}
				else
				{
					LldPrint("[LLD] AD9852 AST model(200MHz)");
				}
				*_cntl = 0x040F0140;
				*_cnst = CLK_CONST_DEF(2,48,300);//AD9852ASV, PCI Express
			}
			else	//	IsAttachedBdTyp_AD9852Clk_ASx_V()
			{
				*_cntl = 0x040A0140;
				*_cnst = CLK_CONST_DEF(2,48,200);//AD9852AST, PCI Express
			}
		}
	
		if(IsAttachedBdTyp_ClkGenAdjForDvbS2() && modulation_mode == TVB380_DVBS2_MODE)
		{
#ifdef WIN32
			if(ad9852_Max_clk == 1) //2011/5/4
			{
				//2011/4/29 45Msps 조건 개선 시험 현재 590C DVB-S2일때 AD9852 clock = symbol rate x2 일것임.  이것을 AD9852 clock = symbol rate 으로 변경한 시험 버전 하나 제공 바랍니다. 그외 변경 사항 없음.
				*_cntl = 0x040F0140;
				*_cnst = CLK_CONST_DEF(2,48,300);//AD9852ASV, PCI Express
			}
#endif
		}
			
		if ( IsAttachedBdTyp_497() )
		{
			/*
				*_cntl = 0x040A0140;
				*_cnst = CLK_CONST_DEF(2,48,100);
			*/
			*_cntl = 0x04140140;
			*_cnst = CLK_CONST_DEF(2,48,200);
		}
		
		if ( CHK_ID(0x00,_BD_ID_499__,0x00) )
		{
			*_cntl = 0x040F0140;
			*_cnst = CLK_CONST_DEF(2,48,300);//AD9852ASV, PCI Express
		}
	
		LldPrint_2("[LLD] AD9852 Symbol CLK_CONTROL, CLK_CONST", *_cntl, (int)*_cnst);
	
		//2011/4/25
		if ( IsAttachedBdTyp_ClkGenAdjForIsdbT13() && (modulation_mode == TVB380_ISDBT_13_MODE) && CurrentTSSrc == 3)
		{	
			*_cntl &= 0xFFFFFEFF;
		}
		//ATSC-M/H
		if ( modulation_mode == TVB380_ATSC_MH_MODE && (CurrentTSSrc == 4 || CurrentTSSrc == 5))
		{
			*_cntl &= 0xFFFFFEFF;
		}
		//2011/6/15 ISDB-S ASI
		if ( IsAttachedBdTyp_UseAD9775() && (modulation_mode == TVB380_ISDBS_MODE) && CurrentTSSrc == 3)
		{	
			*_cntl &= 0xFFFFFEFF;
		}
		//2012/7/11 DVB-T2 ASI
		if ( (modulation_mode == TVB380_DVBT2_MODE || modulation_mode == TVB380_DVBC2_MODE) && CurrentTSSrc == 3)
		{
			*_cntl &= 0xFFFFFEFF;
		}

	}

///////////////////////////////////////////////////////////////////////////	QAMA
	if ( IsAttachedBdTyp_ClkGenAdjForQamA() && modulation_mode == TVB380_QAMA_MODE && _en_adj == 1 )
	{
		//open symbol table
		FILE *fp;
#ifdef WIN32
		char szTable[MAX_PATH];
		sprintf(szTable, "%s\\%s%d.txt", szCurDir, "qam_a_symbol_table", TSPL_nBoardRevision);
		fopen_s(&fp, szTable, "rt");
#else
		char szTable[64];
		sprintf(szTable, "%s%d.txt","qam_a_symbol_table", TSPL_nBoardRevision);
		fp = fopen(szTable, "rt");
#endif
		if ( !fp )
		{
			LldPrint_s_s("[LLD]===CONFIG AD9852 SYMBOL ADJUST, No symbol table found", szTable);
		}
		else
		{
			int _from, _to;
			char buf[32];
			char seps[]   = ",";
			char *token;

			while ( !feof(fp) )
			{
				fgets(buf, 32, fp);
				token = strtok( buf, seps );
				if( token != NULL )
				{
					_from = atoi(token);
				}

				token = strtok( NULL, seps );

				if( token != NULL )
				{
					_to = atoi(token);
				}

				LldPrint_2("[LLD]===CONFIG AD9852 SYMBOL CLOCK ADJUST", _from, _to);

				int clock_const, ref_mul, pll_range;
				switch (TSPL_nBoardRevision)
				{
					case __REV_390_590__://REV3
						if ( _sym_freq >= _from && _sym_freq <= _to )
						{
							clock_const = 224;//32*7 MHz
							ref_mul = 7;
							pll_range = 1;
	
							*_cntl = (*_cntl & 0xFFE00000) + ((ref_mul & 0x1F) << 16) + (*_cntl & 0x0000FFFF);
							*_cntl |= (pll_range << 22);
							*_cnst = CLK_CONST_DEF(2,48,clock_const);
						}
						break;
					case __REV_590_express://REV4
						if ( _sym_freq >= _from && _sym_freq <= _to )
						{
							clock_const = 180;//20*9 MHz
							ref_mul = 9;
							pll_range = 0;
	
							*_cntl = (*_cntl & 0xFFE00000) + ((ref_mul & 0x1F) << 16) + (*_cntl & 0x0000FFFF);
							*_cntl |= (pll_range << 22);
							*_cnst = CLK_CONST_DEF(2,48,clock_const);
						}
						break;
					case __REV_595_v1_1://REV5
						if ( _sym_freq >= _from && _sym_freq <= _to )
						{
							clock_const = 180;//20*9 MHz
							ref_mul = 9;
							pll_range = 0;
	
							*_cntl = (*_cntl & 0xFFE00000) + ((ref_mul & 0x1F) << 16) + (*_cntl & 0x0000FFFF);
							*_cntl |= (pll_range << 22);
							*_cnst = CLK_CONST_DEF(2,48,clock_const);
						}
						break;
					default:
						break;
				}
			}
			fclose(fp);
		}
	}
}
int CDdsCtl::CalcOutFreq_(int modulation_mode, int *_sym_freq, double _cnst, __int64 *_out_freq)
{
	double	outfreq_float;
	double	symbol_clock;
	int	reg_data = 0;

	if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499())
	{
		//2012/6/27 multi dvb-t
		//if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
		if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE || modulation_mode == TVB380_MULTI_DVBT_MODE)
		{
			switch (*_sym_freq)
			{
				case DVB_T_6M_BAND:
					symbol_clock = 48;
					break;
				case DVB_T_7M_BAND:
					symbol_clock = 56;
					break;
				case DVB_T_8M_BAND:
					symbol_clock = 64;//MHz
					break;
				case DVB_T_5M_BAND:
					symbol_clock = 40;
					break;
				default:
					LldPrint_Error("dds-ctl : pos...", 5, 32);
					return -1;//invalid bandwidth
			}

			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
		{
			*_sym_freq = 4500000;
			symbol_clock = (*_sym_freq / 1000000.0) * (684./286.) * 4.;

			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE )
		{
			if (*_sym_freq < 1000000 || *_sym_freq > 8000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 33);
				return -1;
			}

			reg_data=4;

			symbol_clock = 2*(*_sym_freq)*reg_data;
			if ( symbol_clock  > 90000000 )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 34);
				return -1;
			}
			symbol_clock /= 1000000.;
	
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
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
			if (*_sym_freq < 1000000 || *_sym_freq > MaximumSym)
				return -1;
#else
			if (*_sym_freq < 1000000 || *_sym_freq > 45000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 35);
				return -1;
			}
#endif			
			if ( *_sym_freq >= 1000000 && *_sym_freq < 2500000 )
			{
				reg_data=4;
			}
			else if ( *_sym_freq < 5000000 )
			{
				reg_data=4;
			}
			else if ( *_sym_freq < 10000000 )
			{
				reg_data=2;
			}
			//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 1
			else if ( *_sym_freq <= MaximumSym )
#else
			else if ( *_sym_freq <= 45000000 )
#endif
			{
				reg_data=1;
			}
			else
			{
				LldPrint_Error("dds-ctl : pos...", 5, 36);
				return -1;
			}
			
			//TEST - SCLEE
			symbol_clock = (*_sym_freq)*reg_data;
			if ( symbol_clock  > 90000000 )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 37);
				return -1;
			}
			symbol_clock /= 1000000.;
			
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_TDMB_MODE )
		{
			*_sym_freq = 2048000;
			symbol_clock = (*_sym_freq / 1000000.0) * 12.;

			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
#ifdef WIN32
			if(*_sym_freq == 0)
				symbol_clock = 65.015872;	//6MHz
			else if(*_sym_freq == 1)
				symbol_clock = 75.851851;	//7MHz
			else
				symbol_clock = 86.68783;		//8MHz
#else
			symbol_clock = 65.015872;	//6MHz
#endif
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_DTMB_MODE )
		{
			symbol_clock = 60.48;//MHz
			if(*_sym_freq == 0)
				symbol_clock = symbol_clock * 6.0 / 8.0;
			else if(*_sym_freq == 1)
				symbol_clock = symbol_clock * 7.0 / 8.0;
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_CMMB_MODE )
		{
			symbol_clock = 60.0;//MHz
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
		{
			if ( *_sym_freq >= DVB_T_6M_BAND && *_sym_freq <= DVB_T_8M_BAND )
			{
				if(TSPL_T2_x7_Mod_Clock == 1)
					symbol_clock = (*_sym_freq + 6.)*8.;
				else
					symbol_clock = (*_sym_freq + 6.)*8.*(8./7.);
			}
			else
			{
				if ( *_sym_freq == DVB_T_5M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 5*8.;
					else
						symbol_clock = 5*8.*(8./7.);
				}
				else if ( *_sym_freq == DVB_T_1_7M_BAND )
				{
					//symbol_clock = (131. / 71.) * 8. * 4;   //1.7*8.*(8./7.);
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = (131. / 71.) * 7.;
					else
						symbol_clock = (131. / 71.) * 8.;   //1.7*8.*(8./7.);
				}
				else if ( *_sym_freq == DVB_T_10M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 10*8.;
					else
						symbol_clock = 10*8.*(8./7.);
				}
			}
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		//I/Q PLAY/CAPTURE - ???
		else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
		{
			outfreq_float= (*_sym_freq / 1000000.) * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		//ISDB-S
		else if ( modulation_mode == TVB380_ISDBS_MODE )
		{
			symbol_clock = 57.72;//28.860*2
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else
		{
			LldPrint_Error("dds-ctl : pos...", 5, 38);
			return -1;//invalid mode
		}
	}
	else
	{
		if ( modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE )
		{
			switch (*_sym_freq)
			{
				case DVB_T_6M_BAND:
					symbol_clock = 48;
					break;
				case DVB_T_7M_BAND:
					symbol_clock = 56;
					break;
				case DVB_T_8M_BAND:
					symbol_clock = 64;//MHz
					break;
				case DVB_T_5M_BAND:
					symbol_clock = 40;
					break;
				default:
					LldPrint_Error("dds-ctl : pos...", 5, 39);
					return -1;//invalid bandwidth
			}

			//AD9852ASQ/AD9852AST
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
		{
			*_sym_freq = 4500000;
			symbol_clock = (*_sym_freq / 1000000.0) * (684./286.) * 4.;
			//AD9852ASQ/AD9852AST
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE )
		{
			if (*_sym_freq < 1000000 || *_sym_freq > 8000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 40);
				return -1;
			}

			symbol_clock = (double)(*_sym_freq * 8) / 1000000.0;
			//AD9852ASQ/AD9852AST
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_QPSK_MODE )
		{
			if (*_sym_freq < 1000000 || *_sym_freq > 45000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 41);
				return -1;
			}
			
			if ( *_sym_freq >= 1000000 && *_sym_freq < 5000000 )
			{
				symbol_clock = 4 * (*_sym_freq);
			}
			else if ( *_sym_freq >= 5000000 && *_sym_freq < 10000000 )
			{
				symbol_clock = 2 * (*_sym_freq);
			}
			else
			{
				symbol_clock = 1 * (*_sym_freq);
			}

			if (gRRC_Filter != ROLL_OFF_FACTOR_NONE)
			{
				symbol_clock *= 2;
			}
			symbol_clock = (double)symbol_clock / 1000000.0;

			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_DVBS2_MODE  )
		{
			if (*_sym_freq < 1000000 || *_sym_freq > 45000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 42);
				return -1;
			}

#if 0
			//2011/4/29 45Msps 조건 개선 시험 현재 590C DVB-S2일때 AD9852 clock = symbol rate x2 일것임.  이것을 AD9852 clock = symbol rate 으로 변경한 시험 버전 하나 제공 바랍니다. 그외 변경 사항 없음.
			symbol_clock = *_sym_freq;
#else
			symbol_clock = 2 * (*_sym_freq);
#endif
			symbol_clock = (double)symbol_clock / 1000000.0;

			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_TDMB_MODE )
		{
			*_sym_freq = 2048000;

			symbol_clock = (*_sym_freq / 1000000.0) * 12.;
			//AD9852ASQ/AD9852AST
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
#ifdef WIN32
			if(*_sym_freq == 0)
				symbol_clock = 65.015872;	//6MHz
			else if(*_sym_freq == 1)
				symbol_clock = 75.851851;	//7MHz
			else
				symbol_clock = 86.68783;		//8MHz
#else
			symbol_clock = 65.015872;	//6MHz
#endif
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_DTMB_MODE )
		{
			symbol_clock = 60.48;//MHz
			if(*_sym_freq == 0)
				symbol_clock = symbol_clock * 6.0 / 8.0;
			else if(*_sym_freq == 1)
				symbol_clock = symbol_clock * 7.0 / 8.0;
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else if ( modulation_mode == TVB380_CMMB_MODE )
		{
			symbol_clock = 60.0;//MHz
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		//DVB-T2
		else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
		{
			if ( *_sym_freq >= DVB_T_6M_BAND && *_sym_freq <= DVB_T_8M_BAND )
			{
				if(TSPL_T2_x7_Mod_Clock == 1)
					symbol_clock = (*_sym_freq + 6.)*8.;
				else
					symbol_clock = (*_sym_freq + 6.)*8.*(8./7.);
			}
			else
			{
				if ( *_sym_freq == DVB_T_5M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 5*8.;
					else
						symbol_clock = 5*8.*(8./7.);
				}
				else if ( *_sym_freq == DVB_T_1_7M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = (131. / 71.) * 7.;
					else
						symbol_clock = (131. / 71.) * 8.;   //1.7*8.*(8./7.);
				}
				else if ( *_sym_freq == DVB_T_10M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 10*8.;
					else
						symbol_clock = 10*8.*(8./7.);
				}
			}
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		//I/Q PLAY/CAPTURE
		else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
		{
			if (*_sym_freq < 4000000 || *_sym_freq > 16000000)
			{
				LldPrint_Error("dds-ctl : pos...", 5, 43);
				return -1;
			}

			outfreq_float= (*_sym_freq / 1000000.) * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		//ISDB-S
		else if ( modulation_mode == TVB380_ISDBS_MODE )
		{
			symbol_clock = 57.72;//28.860*2
			outfreq_float= symbol_clock * _cnst;
			*_out_freq= (__int64)outfreq_float;
		}
		else
		{
			LldPrint_Error("dds-ctl : pos...", 5, 44);
			return -1;//invalid mode
		}
	}
	LldPrint_2("Output Freq.(float/long)", (int)outfreq_float, (int)*_out_freq);

	return	0;
}

int CDdsCtl::CntlTrf178_NewBd_Case_1(unsigned long dwfreq, int step_freq)		//	593/597v2 or 590s higher rev3
{
	int		reg_data = 0;
	double	fLO, fUMC, fRF;
	int		RF_sw = 0;
	double	fUMC2;
	double 	outfreq_tmp;
	int		channel_num = 0;
	LldPrint_FCall("CntlTrf178_NewBd_Case_1", 0, 0);

	fRF = dwfreq/1000.;
	fLO = fRF*2.;

	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1() )	//	593/597v2
	{
		if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1_1() || IsAttachedBdTyp_499())	//	597v2 or 593 higher rev2
		{
			//2011/3/14 FIXED
			//if ( fLO > 0. && fLO <= 170. )
			if ( fLO > 0. && fLO <= 150000. )
			{
				//fUMC = 1000000.;
				//2012/7/10 TEST
				if(fLO < 114000)
					fUMC = 1050000.;
				else if(fLO < 116000)
					fUMC = 1100000.;
				else if(fLO < 122000)
					fUMC = 1000000.;
				else if(fLO < 126000)
					fUMC = 1050000.;
				else if(fLO < 130000)
					fUMC = 1100000.;
				else if(fLO < 140000)
					fUMC = 1000000.;
				else if(fLO < 148000)
					fUMC = 1050000.;
				else
					fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<16) + (1<<4) + (1<<2);
			}
			//2011/3/14 FIXED
			//else if ( fLO > 170. && fLO <= 300. )
			else if ( fLO > 150000. && fLO <= 300000. )
			{
				//fUMC = 1000000.;
				//2012/7/10 TEST
				if(fLO < 158000)
					fUMC = 1000000.;
				else if(fLO < 160000)
					fUMC = 1200000.;
				else if(fLO < 168000)
					fUMC = 1050000.;
				else if(fLO < 170000)
					fUMC = 1250000.;
				else if(fLO < 172000)
					fUMC = 1100000.;
				else
				fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<12) + (1<<4) + (1<<2);
			}
			//FIXED - 600. -> 540.
			else if ( fLO > 300000. && fLO <= 540000. )
			{
				fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<13) + (1<<12) + (1<<11) + (1<<4) + (1<<2);
			}
			//FIXED - 830. -> 900., 600. -> 540.
			else if ( fLO > 540000. && fLO < 900000. )
			{
				fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<13) + (1<<11) + (1<<10) + (1<<4) + (1<<2);
			}
			//FIXED - 830. -> 900.
			else if ( fLO >= 900000. && fLO <= 1200000. )
			{
				fUMC = -1.;
				fUMC2 = fLO;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<13) + (1<<11) + (1<<10) + (1<<1) + 1;
			}
			//else if ( fLO > 1200000. && fLO <= 2000000. )
			else if ( fLO > 1200000. && fLO <= 1940000. )
			{
				fUMC = -1.;
				fUMC2 = fLO;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<10) + (1<<1) + 1;
			}
			//else if ( fLO > 2000000. && fLO <= 2150000. )
			else if ( fLO > 1940000. && fLO <= 2150000. )
			{
				fUMC = -1.;
				fUMC2 = fLO/2;
				RF_sw = (1<<17) + (1<<16) + (1<<15) + (1<<10) + (1<<7) + (1<<3) + 1;
			}
			//else if ( fLO > 2150000. && fLO <= 4000000. )
			else if ( fLO > 2150000. && fLO <= 3800000. )
			{
				fUMC = -1.;
				fUMC2 = fLO/2.;
				RF_sw = (1<<17) + (1<<15) + (1<<14) + (1<<7) + (1<<3) + 1;
			}
			else
			{
				fUMC = fLO/4.;
				fUMC2 = fUMC;
				RF_sw = (1<<14) + (1<<6) + (1<<5) + (1<<3);
			}
		}
		else
		{
			if ( fLO > 0. && fLO <= 220000. )
			{
				//fUMC = 1000000.;
				//2012/7/10 TEST
				if(fLO < 114000)
					fUMC = 1050000.;
				else if(fLO < 116000)
					fUMC = 1100000.;
				else if(fLO < 122000)
					fUMC = 1000000.;
				else if(fLO < 126000)
					fUMC = 1050000.;
				else if(fLO < 130000)
					fUMC = 1100000.;
				else if(fLO < 140000)
					fUMC = 1000000.;
				else if(fLO < 148000)
					fUMC = 1050000.;
				else if(fLO < 158000)
					fUMC = 1000000.;
				else if(fLO < 160000)
					fUMC = 1200000.;
				else if(fLO < 168000)
					fUMC = 1050000.;
				else if(fLO < 170000)
					fUMC = 1250000.;
				else if(fLO < 172000)
					fUMC = 1100000.;
				else
					fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<12) + (1<<8) + (1<<4) + (1<<2);
			}
			else if ( fLO > 220000. && fLO <= 496000. )
			{
				fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<13) + (1<<12) + (1<<11) + (1<<8) + (1<<4) + (1<<2);
			}
			//2011/4/25 fixed 415 -> 450
			//else if ( fLO > 496. && fLO < 830. )
			else if ( fLO > 496000. && fLO < 900000. )
			{
				fUMC = 1000000.;
				fUMC2 = fLO + fUMC;
				RF_sw = (1<<13) + (1<<11) + (1<<10) + (1<<8) + (1<<4) + (1<<2);
			}
			//2011/4/25 fixed 415 -> 450
			//else if ( fLO >= 830. && fLO <= 1200. )
			else if ( fLO >= 900000. && fLO <= 1200000. )
			{
				fUMC = -1.;
				fUMC2 = fLO;
				RF_sw = (1<<13) + (1<<11) + (1<<10) + (1<<8) + (1<<1) + 1;
			}
			//else if ( fLO > 1200000. && fLO <= 2000000. )
			else if ( fLO > 1200000. && fLO <= 1940000. )
			{
				fUMC = -1.;
				fUMC2 = fLO;
				RF_sw = (1<<10) + (1<<8) + (1<<1) + 1;
			}
			//else if ( fLO > 2000000. && fLO <= 2150000. )
			else if ( fLO > 1940000. && fLO <= 2150000. )
			{
				fUMC = -1.;
				fUMC2 = fLO/2;
				RF_sw = (1<<10) + (1<<8) + (1<<7) + (1<<3) + 1;
			}
			//else if ( fLO > 2150000. && fLO <= 4000000. )
			else if ( fLO > 2150000. && fLO <= 3800000. )
			{
				fUMC = -1.;
				fUMC2 = fLO/2.;
				RF_sw = (1<<9) + (1<<7) + (1<<3) + 1;
			}
			else
			{
				fUMC = fLO/4.;
				fUMC2 = fUMC;
				RF_sw = (1<<9) + (1<<6) + (1<<5) + (1<<3);
			}
		}
	}
	else
	{
		if ( fLO > 0000. && fLO <= 170000. )
		{
			//fUMC = 1000000.;
				//2012/7/10 TEST
				if(fLO < 114000)
					fUMC = 1050000.;
				else if(fLO < 116000)
					fUMC = 1100000.;
				else if(fLO < 122000)
					fUMC = 1000000.;
				else if(fLO < 126000)
					fUMC = 1050000.;
				else if(fLO < 130000)
					fUMC = 1100000.;
				else if(fLO < 140000)
					fUMC = 1000000.;
				else if(fLO < 148000)
					fUMC = 1050000.;
				else if(fLO < 158000)
					fUMC = 1000000.;
				else if(fLO < 160000)
					fUMC = 1200000.;
				else if(fLO < 168000)
					fUMC = 1050000.;
				else if(fLO < 170000)
					fUMC = 1250000.;
				else
					fUMC = 1100000.;
			fUMC2 = fLO + fUMC;
			RF_sw = (1<<10) + (1<<4) + (1<<2);
		}
		else if ( fLO > 170000. && fLO <= 300000. )
		{
			if(fLO < 172000)
				fUMC = 1100000.;
			fUMC = 1000000.;
			fUMC2 = fLO + fUMC;
			RF_sw = (1<<14) + (1<<11) + (1<<10) + (1<<9) + (1<<4) + (1<<2);
		}
		else if ( fLO > 300000. && fLO <= 600000. )
		{
			fUMC = 1000000.;
			fUMC2 = fLO + fUMC;
			RF_sw = (1<<15) + (1<<14) + (1<<13) + (1<<11) + (1<<10) + (1<<9) + (1<<4) + (1<<2);
		}
		//FIXED - 830. -> 900.
		else if ( fLO > 600000. && fLO < 900000. )
		{
			fUMC = 1000000.;
			fUMC2 = fLO + fUMC;
			RF_sw = (1<<15) + (1<<13) + (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<4) + (1<<2);
		}
		//FIXED - 830. -> 900.
		else if ( fLO >= 900000. && fLO <= 1200000. )
		{
			fUMC = -1.;
			fUMC2 = fLO;
			RF_sw = (1<<15) + (1<<13) + (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<1) + 1;
		}
		//else if ( fLO > 1200000. && fLO <= 2000000. )
		else if ( fLO > 1200000. && fLO <= 1940000. )
		{
			fUMC = -1.;
			fUMC2 = fLO;
			RF_sw = (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<1) + 1;
		}
		//else if ( fLO > 2000000. && fLO <= 2150000. )
		else if ( fLO > 1940000. && fLO <= 2150000. )
		{
			fUMC = -1.;
			fUMC2 = fLO/2;
			RF_sw = (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<7) + (1<<3) + 1;
		}
		//else if ( fLO > 2150000. && fLO <= 4000000. )
		else if ( fLO > 2150000. && fLO <= 3800000. )
		{
			fUMC = -1.;
			fUMC2 = fLO/2.;
			RF_sw = (1<<11) + (1<<9) + (1<<8) + (1<<7) + (1<<3) + 1;
		}
		else/* if ( fLO > 3800000. && fLO <= 4300000. )*/
		{
			fUMC = fLO/4.;
			fUMC2 = fUMC;
			RF_sw = (1<<8) + (1<<6) + (1<<5) + (1<<3);
		}
		//else
		//{
		//	LldPrint_Error("dds-ctl : pos...", 5, 4);
		//	return -1;
		//}
	}
#if 0 //2012/2/7 TEST
	/* OUTPUT BAND */
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		reg_data = (int)WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);
		LldPrint_1("[LLD]TVB593 MODULATOR PARAM/TRF178_ATTN_ADDR_CONTROL_REG", reg_data);

		reg_data = (RF_sw<<8) + (reg_data&0xFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 9);
			return -1;
		}

		LldPrint_1("[LLD]TVB593...", reg_data);
	}
	else
	{
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_OUTPUT_BAND_CONTROL_REG, RF_sw))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 10);
			return -1;
		}
	}
	LldPrint_1x("OUTPUT BAND ", RF_sw);
#endif


	//int interp_len = 400;//55~414 => 0~359
	int interp_len = 395;//400=>360=>395 (55~449 => 0~394)
	int result_len = interp_len;
	int sample_count = 66;//60 => 66
	int sample[] = {140,155,170,190,220,80,95,70,80,130,
					170,170,180,200,230,270,300,380,430,240,
					250,250,230,220,220,220,160,140,140,140,
					140,140,160,20,45,70,100,120,140,160,
					180,100,100,80,100,100,40,20,40,60,
					80,100,100,150,150,160,180,200,190,178,
					166,154,142,130,118,106};

	double *result = (double*)calloc(result_len, sizeof(double)*result_len);
	DoInterp(0, sample, sample_count, interp_len, result, result_len, 2);

	int UNC_OFFSET = 0;
	//if ( fRF >= 55. && fRF <= 414. )
	//if ( fRF >= 55. && fRF <= 449. )
	//if ( fRF >= 74. && fRF <= 449. )
	if ( fRF >= 86000. && fRF < 500000. )
	{
		UNC_OFFSET = (int)result[(int)((fRF - 55000) / 1000)];
	}
	if ( fUMC > 0 )
	{
		fUMC += (UNC_OFFSET * 1000);
	}
	fUMC2 += (UNC_OFFSET * 1000);

	free(result);

	int start_freq = (UMC_START_FREQ * 1000);
	//if ( fRF >= 415. && fRF <= 499. )
	if ( fRF >= 450000. && fRF < 500000. )
	{
		start_freq = (int)(fRF*2);
		//2011/6/27
		start_freq = start_freq / 1000;
		start_freq = start_freq * 1000;
	}

	if ( fRF > 1900000. && fRF < 2000000. )
	{
		start_freq = (int)(fRF / 2);
		//2011/6/27
		start_freq = start_freq / 1000;
		start_freq = start_freq * 1000;
	}

	if ( fRF > 970000. && fRF < 1000000. )
	{
		start_freq = (int)fRF;
		//2011/6/27
		start_freq = start_freq / 1000;
		start_freq = start_freq * 1000;
	}

	/* START REG */
	reg_data = 1;	
	reg_data <<= 28;
	reg_data += (6 << 24);
	reg_data += (start_freq / 1000);
	LldPrint_1("START_FREQ MHz", start_freq);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 5);
		return -1;
	}

	//TVB590S V2 or higher, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499())
	{
		//TVB593, TVB497
		if (IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl() || IsAttachedBdTyp_499())	//	593/497/597v2 or 593 higher rev2
		{
			Sleep(10);
			if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
			{
				LldPrint_Error("dds-ctl : pos...", 5, 6);
				return -1;
			}
		}
	}

	//sskim20080925 - TEST, must be 2 >=
	//Sleep(10);
	Sleep(10);//fixed - ubuntu? 2 -> 10

	//FIXED
	if ( fUMC < 0 )
	{
		fUMC = start_freq;
	}

	if ( fUMC > 0 )
	{
		outfreq_tmp = (double)(fUMC - (double)start_freq) / (double)step_freq;
		LldPrint_5("fRF, fLO, fUMC1, fUMC2, outfreq_tmp", (int)fRF, (int)fLO, (int)fUMC, (int)fUMC2, (int)outfreq_tmp);
		channel_num = (int)(outfreq_tmp);

		/* UMC1 - FREQUENCY/CHANNEL REG */
		reg_data = 0;
		reg_data <<= 28;
		reg_data += channel_num;
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 7);
			return -1;
		}
	}

	outfreq_tmp = (double)(fUMC2 - (double)start_freq) / (double)step_freq;
	channel_num = (int)(outfreq_tmp);
	LldPrint_5("fRF, fLO, fUMC1, fUMC2, outfreq_tmp", (int)fRF, (int)fLO, (int)fUMC, (int)fUMC2, (int)outfreq_tmp);
	//FIXED - 415 -> 450
	if ( fRF >= 450000. && fRF < 500000. )
	{
		//FIXED
		//channel_num = 0;
		channel_num = (int)(((2*fRF - (double)start_freq)) / (double)step_freq);
	}

	/* UMC2 - FREQUENCY/CHANNEL REG */
	reg_data = 0;
	reg_data <<= 28;
	reg_data += channel_num;
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 8);
		return -1;
	}
#if 1 //2012/2/7 TEST
	/* OUTPUT BAND */
	if ( IsAttachedBdTyp_UseIndirectRegAccess() )
	{
		reg_data = (int)WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);
		LldPrint_1("[LLD]TVB593 MODULATOR PARAM/TRF178_ATTN_ADDR_CONTROL_REG", reg_data);

		reg_data = (RF_sw<<8) + (reg_data&0xFF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 9);
			return -1;
		}

		LldPrint_1("[LLD]TVB593...", reg_data);
	}
	else
	{
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_OUTPUT_BAND_CONTROL_REG, RF_sw))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 10);
			return -1;
		}
	}
	LldPrint_1x("OUTPUT BAND ", RF_sw);
#endif
	return	0;
}
int CDdsCtl::CntlTrf178_NewBd_Case_2(unsigned long dwfreq, int step_freq)	//	tvb590s rev2s
{
	int		reg_data = 0;
	double	fLO, fUMC, fRF = dwfreq/1000.;
	int		RF_sw = 0;
	double	fUMC2;
	double 	outfreq_tmp;
	int		channel_num = 0;
	int		start_freq = UMC_START_FREQ;
	LldPrint_FCall("CntlTrf178_NewBd_Case_2", 0, 0);

	//2011/8/18
	fRF = dwfreq/1000000.;
	fLO = fRF*2.;
	
	if ( fLO > 0. && fLO < 1000. )
	{
		fUMC = 1000.;
		fUMC2 = fLO + fUMC;
		RF_sw = (1<<4) + (1<<2);
	}
	else if ( fLO >= 1000. && fLO <= 2000. )
	{
		fUMC = 0.;
		fUMC2 = fLO;
		RF_sw = (1<<1) + 1;
	}
	else if ( fLO > 2000. && fLO <= 4000. )
	{
		fUMC = 0.;
		fUMC2 = fLO/2.;
		RF_sw = (1<<7) + (1<<4) + (1<<3) + 1;
	}
	else if ( fLO > 4000. && fLO <= 4300. )
	{
		fUMC = fUMC2 = fLO/4.;
		RF_sw = (1<<6) + (1<<5) + (1<<3);
	}
	else
	{
		LldPrint_Error("dds-ctl : pos...", 5, 11);
		return -1;
	}
	
	outfreq_tmp = (double)(fUMC - start_freq) * 1000. / (double)step_freq;
	LldPrint_5("fRF, fLO, fUMC1, fUMC2, outfreq_tmp", (int)fRF, (int)fLO, (int)fUMC, (int)fUMC2, (int)outfreq_tmp);
	channel_num = (int)outfreq_tmp;
	
	/* UMC1 - FREQUENCY/CHANNEL REG */
	reg_data = 0;
	reg_data <<= 28;
	reg_data += channel_num;
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 12);
		return -1;
	}
	
	outfreq_tmp = (double)(fUMC2 - start_freq) * 1000. / (double)step_freq;
	channel_num = (int)outfreq_tmp;
	
	/* UMC2 - FREQUENCY/CHANNEL REG */
	reg_data = 0;
	reg_data <<= 28;
	reg_data += channel_num;
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 13);
		return -1;
	}
	
	/* OUTPUT BAND */
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_OUTPUT_BAND_CONTROL_REG, RF_sw))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 14);
		return -1;
	}
	LldPrint_1x("OUTPUT BAND", RF_sw);

	return	0;
}
int CDdsCtl::CntlTrf178_NewBd_Case_3(unsigned long dwfreq, int step_freq)	//	tvb590s rev1
{
	int		reg_data = 0;
	double	fLO, fUMC, fRF = dwfreq/1000.;
	int		RF_sw = 0;
	double 	outfreq_tmp;
	int		channel_num = 0;
	int		start_freq = UMC_START_FREQ;
	LldPrint_FCall("CntlTrf178_NewBd_Case_3", 0, 0);

	//2011/8/18
	fRF = dwfreq/1000000.;
	
	fLO = fRF;
	
	if ( fLO >= 500. && fLO < 1000. )
	{
		fUMC = 2*fLO;
		RF_sw = 1 + (1<<2);
	}
	else if ( fLO >= 1000. && fLO < 4000./3. )
	{
		fUMC = fLO;
		RF_sw = (1<<1) + (1<<2);
	}
	else if ( fLO >= 4000./3. && fLO <= 2000. )
	{
		fUMC = fLO;
		RF_sw = (1<<1) + (1<<3);
	}
	else
	{
		LldPrint_Error("dds-ctl : pos...", 5, 15);
		return -1;
	}
	
	outfreq_tmp = (double)(fUMC - start_freq) * 1000. / (double)step_freq;
	LldPrint_5("fRF, fLO, fUMC, outfreq_tmp", (int)fRF, (int)fLO, (int)fUMC, (int)outfreq_tmp, 0);
	channel_num = (int)outfreq_tmp;
	
	/* FREQUENCY/CHANNEL REG */
	reg_data = 0;
	reg_data <<= 28;
	reg_data += channel_num;
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 16);
		return -1;
	}
	
	/* OUTPUT BAND */
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_OUTPUT_BAND_CONTROL_REG, RF_sw))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 17);
		return -1;
	}
	LldPrint_1x("OUTPUT BAND", RF_sw);

	return	0;
}
int CDdsCtl::CntlTrf178_OldBd_Case_1(int modulation_mode, unsigned long dwfreq, int step_freq)
{
	int		reg_data = 0;
	double	oper_freq = 0;//MHz
	double	outfreq_tmp;
	int		start_freq = UMC_START_FREQ;
	int		channel_num = 0;
	LldPrint_FCall("CntlTrf178_OldBd_Case_1", 0, 0);

	//TVB497 +TVO1624
	if ( IsAttachedBdTyp_497() )
	{
		oper_freq = 2.*(dwfreq /*+ 3*10762238*/);
		oper_freq /= 1000000.;
	}
	else if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk() )	/* Board Id >= 44 and for all modulator mode */
	{
		//////////////////////////////////////////////////////////
		//950~2150MHz support
		if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
		{
			if ( dwfreq < RF_MIN_DVB_S || dwfreq > RF_MAX )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 18);
				return -1;
			}
	
			if ( dwfreq >= (unsigned long)(RF_1GHZ+(int)gChannel_Freq_Offset) )
			{
				//FIXED - IF +- offset
				gRF_143KHZ_sign = 1;
				oper_freq = (double)(dwfreq - gChannel_Freq_Offset - gRF_143KHZ) / 1000000.000000;
			}
			else
			{
				//FIXED - IF +- offset
				oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ) / 1000000.000000;
			}
		}
		else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
		{
			if ( dwfreq < RF_MIN || dwfreq > RF_MAX )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 19);
				return -1;
			}
	
			if ( dwfreq >= (unsigned long)(RF_1GHZ+(int)gChannel_Freq_Offset) )
			{
				//FIXED - IF +- offset
				gRF_143KHZ_sign = 1;
				oper_freq = (double)(dwfreq - gChannel_Freq_Offset - gRF_143KHZ) / 1000000.0000000;
			}
			else
			{
				if ( gChannel_Freq_Offset == IF_OUT_36MHZ )
				{
					oper_freq = (double)(dwfreq + (gChannel_Freq_Offset-gRF_143KHZ) + ROWSWIN_FREQ_36M) / 1000000.000000;
				}
				else
				{
					oper_freq = (double)(dwfreq + (gChannel_Freq_Offset-gRF_143KHZ) + ROWSWIN_FREQ_44M) / 1000000.000000;
	
					//1036 ~ 1043 MHz
					if ( dwfreq >= (RF_1GHZ+IF_OUT_36MHZ) && dwfreq < (RF_1GHZ+IF_OUT_44MHZ) )
					{
						oper_freq = (double)(dwfreq + (gChannel_Freq_Offset-gRF_143KHZ)) / 1000000.000000;
					}
				}
			}
		}
		else
		{
			if ( dwfreq < RF_MIN || dwfreq > RF_MAX )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 20);
				return -1;
			}
	
			if ( dwfreq >= (unsigned long)(RF_1GHZ+(int)gChannel_Freq_Offset) )
			{
				oper_freq = (double)(dwfreq - gChannel_Freq_Offset) / 1000000.000000;
	
				//FIXED - IF +- offset
				if ( modulation_mode == TVB380_TDMB_MODE || modulation_mode == TVB380_DVBC2_MODE) //2011/5/9 DVB-C2
				{
				}
				else
				{
					gRF_143KHZ_sign = 1;
					oper_freq = (double)(dwfreq - gChannel_Freq_Offset - gRF_143KHZ) / 1000000.000000;
				}
			}
			else
			{
				if ( gChannel_Freq_Offset == IF_OUT_36MHZ )
				{
					oper_freq = (double)(dwfreq + gChannel_Freq_Offset + ROWSWIN_FREQ_36M) / 1000000.000000;
				}
				else
				{
					oper_freq = (double)(dwfreq + gChannel_Freq_Offset + ROWSWIN_FREQ_44M) / 1000000.000000;
	
					//1036 ~ 1043 MHz
					if ( dwfreq >= (RF_1GHZ+IF_OUT_36MHZ) && dwfreq < (RF_1GHZ+IF_OUT_44MHZ) )
					{
						oper_freq = (double)(dwfreq + gChannel_Freq_Offset) / 1000000.000000;
					}
				}
	
				//FIXED - IF +- offset
				if ( modulation_mode == TVB380_TDMB_MODE || modulation_mode == TVB380_DVBC2_MODE) //2011/5/9 DVB-C2
				{
				}
				else
				{
					if ( gChannel_Freq_Offset == IF_OUT_36MHZ )
					{
						oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ + ROWSWIN_FREQ_36M) / 1000000.000000;
					}
					else
					{
						oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ + ROWSWIN_FREQ_44M) / 1000000.000000;
	
						//1036 ~ 1043 MHz
						if ( dwfreq >= (RF_1GHZ+IF_OUT_36MHZ) && dwfreq < (RF_1GHZ+IF_OUT_44MHZ) )
						{
							oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ) / 1000000.000000;
						}
					}
				}
			}
		}
	
		//950 ~ 963(955) MHz
		if ( dwfreq < (RF_1GHZ - gChannel_Freq_Offset) )
		{
			if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
			{
				oper_freq = (double)(dwfreq + RF_948MHZ) / 1000000.000000;
	
				//FIXED - IF +- offset
				if ( modulation_mode == TVB380_TDMB_MODE )
				{
				}
				else
				{
					oper_freq = (double)(dwfreq + RF_948MHZ - gRF_143KHZ) / 1000000.000000;
				}
			}
		}
		//2037(2045) ~ MHz
		else if ( dwfreq > (RF_2GHZ/* + gChannel_Freq_Offset*/) )
		{
			oper_freq = (double)(dwfreq - RF_948MHZ) / 1000000.000000;
	
			//FIXED - IF +- offset
			if ( modulation_mode == TVB380_TDMB_MODE || modulation_mode == TVB380_DVBC2_MODE) //2011/5/9 DVB-C2
			{
			}
			else
			{
				gRF_143KHZ_sign = 1;
				oper_freq = (double)(dwfreq - RF_948MHZ - gRF_143KHZ) / 1000000.000000;
			}
		}
		//////////////////////////////////////////////////////////
	}
	else
	{
		if ( modulation_mode == TVB380_QPSK_MODE 
			|| modulation_mode == TVB380_DVBS2_MODE  )
		{
			if ( dwfreq < 1036000000 /*|| dwfreq > 2036000000*/ )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 21);
				return -1;
			}
	
			if ( gChannel_Freq_Offset != IF_OUT_36MHZ )
				gChannel_Freq_Offset = IF_OUT_36MHZ;
	
			//FIXED - IF +- offset
			gRF_143KHZ_sign = 1;
			oper_freq = (double)(dwfreq - gChannel_Freq_Offset - gRF_143KHZ) / 1000000.000000;
		}
		else if ( modulation_mode == TVB380_TDMB_MODE )
		{
			if ( dwfreq < RF_MIN || dwfreq > (unsigned long)(RF_2GHZ+(int)gChannel_Freq_Offset) )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 22);
				return -1;
			}
	
			if ( dwfreq >= (unsigned long)(RF_1GHZ+(int)gChannel_Freq_Offset) )
			{
				oper_freq = (double)(dwfreq - gChannel_Freq_Offset) / 1000000.000000;
			}
			else
			{
				if ( gChannel_Freq_Offset == IF_OUT_36MHZ )
					oper_freq = (double)(dwfreq + gChannel_Freq_Offset + ROWSWIN_FREQ_36M) / 1000000.000000;
				else
					oper_freq = (double)(dwfreq + gChannel_Freq_Offset + ROWSWIN_FREQ_44M) / 1000000.000000;
			}
		}
		else
		{
			if ( dwfreq < RF_MIN || dwfreq > (RF_1GHZ+RF_MIN) )
			{
				LldPrint_Error("dds-ctl : pos...", 5, 23);
				return -1;
			}
	
			//FIXED - IF +- offset
			if ( gChannel_Freq_Offset == IF_OUT_36MHZ )
				oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ + ROWSWIN_FREQ_36M) / 1000000.000000;
			else
				oper_freq = (double)(dwfreq + gChannel_Freq_Offset - gRF_143KHZ + ROWSWIN_FREQ_44M) / 1000000.000000;
		}
	}
	
	outfreq_tmp = (double)(oper_freq - start_freq) * 1000. / (double)step_freq;
	LldPrint_2("Freq ", (int)oper_freq, (int)outfreq_tmp);
	//FIXED - ROWSWIN offset
	if ( outfreq_tmp < 0 )
	{
		outfreq_tmp = 0;
	}
	channel_num = (int)outfreq_tmp;
	
	/* FREQUENCY/CHANNEL REG */
	reg_data = 0;
	reg_data <<= 28;
	reg_data += channel_num;
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 24);
		return -1;
	}
	
	//TVB497 +TVO1624
	if ( IsAttachedBdTyp_497() )
	{
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 25);
			return -1;
		}
	}
	
	if ( IsAttachedBdTyp_591() )
	{
		return	0;
	}
	reg_data = 0;
	
	/* Board Id >= 44 and for all modulator mode */
	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk() )
	{
		//////////////////////////////////////////////////////////
		//950~2150MHz support
		if ( dwfreq >= RF_1GHZ + gChannel_Freq_Offset )
		{
			reg_data = 1;
		}
		else
		{
			reg_data = 0;
		}
	
		if ( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE  )
		{
			reg_data = 1;
			//2037(2045)~, 950~963(955))
			if ( dwfreq > (RF_2GHZ/* + gChannel_Freq_Offset*/) || dwfreq < (RF_1GHZ - gChannel_Freq_Offset) )
			{
				reg_data = 0;
			}
		}
		else
		{
			//2037(2045) ~ MHz
			if ( dwfreq > RF_2GHZ/* + gChannel_Freq_Offset*/ )
			{
				reg_data = 0;
			}
	
			if ( gChannel_Freq_Offset == IF_OUT_44MHZ )
			{
				//1036 ~ 1043 MHz
				if ( dwfreq >= (RF_1GHZ+IF_OUT_36MHZ) && dwfreq < (RF_1GHZ+IF_OUT_44MHZ) )
				{
					reg_data = 1;
				}
			}
		}
		//////////////////////////////////////////////////////////
	}
	else
	{
		if ( modulation_mode == TVB380_TDMB_MODE )
		{
			if ( dwfreq >= 1036000000 )
			{
				if ( IsAttachedBdTyp_380v3_upconverter() )
					reg_data = 0;
				else
					reg_data = 1;
			}
			else
			{
				if ( IsAttachedBdTyp_380v3_upconverter() )
					reg_data = 1;
				else
					reg_data = 0;
			}
		}
		else if ( modulation_mode == TVB380_QPSK_MODE 
			|| modulation_mode == TVB380_DVBS2_MODE  )
		{
			if ( IsAttachedBdTyp_380v3_upconverter() )
				reg_data = 0;
			else
				reg_data = 1;
		}
		else
		{
			if ( IsAttachedBdTyp_380v3_upconverter() )
				reg_data = 1;
			else
				reg_data = 0;
		}
	}
	
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_OUTPUT_BAND_CONTROL_REG, reg_data))
	{
		LldPrint_Error("dds-ctl : pos...", 5, 26);
		return -1;
	}
	LldPrint_1x("OUTPUT BAND", reg_data);

	return	0;
}
void CDdsCtl::CntlAD9852SymClk_NewBd_IndirectAccess(__int64 outfreq_long, int nCLK_CONTROL)
{
	int		temp_data;
	LldPrint_FCall("CntlAD9852SymClk_NewBd_IndirectAccess", 0, 0);

	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1, (0x3 << 25));//IO RESET
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1, (0x0 << 25));//IO RESET
	
	WaitMsec(1);//Sleep(1);
	temp_data = 2;
	LldPrint_2("AD9852 SymClk : Addr", temp_data, 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 40) & 0xff), 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 32) & 0xff), 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 24) & 0xff), 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 16) & 0xff), 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 8) & 0xff), 0);
	LldPrint_2("AD9852 SymClk : Data", (int)((outfreq_long >> 0) & 0xff), 0);
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL2,
		(unsigned long)(((outfreq_long >> 24) & 0xff) << 24) |		//	3rd
		(unsigned long)(((outfreq_long >> 16) & 0xff) << 16) |
		(unsigned long)(((outfreq_long >> 8) & 0xff) << 8) |
		(unsigned long)(((outfreq_long >> 0) & 0xff) << 0));		//	6th
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1,
		(1 << 24) | (0 << 23) | (6 << 20) | (temp_data << 16) |
		(unsigned long)(((outfreq_long >> 40)& 0xff) << 8) |			//	1st
		(unsigned long)(((outfreq_long >> 32)& 0xff) << 0));			//	2nd
	WaitMsec(1);//Sleep(1);

	//UPDATE CLOCK
	temp_data = 5;
	LldPrint_2("AD9852 UdpClk : Addr", temp_data, 0);
	LldPrint_2("AD9852 UdpClk : Data", ((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 24) & 0xff), 0);
	LldPrint_2("AD9852 UdpClk : Data", ((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 16) & 0xff), 0);
	LldPrint_2("AD9852 UdpClk : Data", ((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 8) & 0xff), 0);
	LldPrint_2("AD9852 UdpClk : Data", ((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 0) & 0xff), 0);
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL2,
		(((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 8) & 0xff) << 24) | 	//	3rd
		(((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 0) & 0xff) << 16) |
		(0 << 8) |
		(0 << 0));
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1,
		(1 << 24) | (0 << 23) | (4 << 20) | (temp_data << 16) |
		(((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 24) & 0xff) << 8) | 		//	1st
		(((AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE >> 16) & 0xff) << 0)); 		//	2nd
	WaitMsec(1);//Sleep(1);

	//CONTROL WORD
	//reg. address
	temp_data = 7;
	LldPrint_2("AD9852 Control : Addr", temp_data, 0);
	LldPrint_2("AD9852 Control : Data", ((nCLK_CONTROL >> 24) & 0xff), 0);
	LldPrint_2("AD9852 Control : Data", ((nCLK_CONTROL >> 16) & 0xff), 0);
	LldPrint_2("AD9852 Control : Data", ((nCLK_CONTROL >> 8) & 0xff), 0);
	LldPrint_2("AD9852 Control : Data", ((nCLK_CONTROL >> 0) & 0xff), 0);
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL2,
		(((nCLK_CONTROL >> 8) & 0xff) << 24) |		//	3rd
		(((nCLK_CONTROL >> 0) & 0xff) << 16) |
		(0 << 8) |
		(0 << 0));
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1,
		(1 << 24) | (0 << 23) | (4 << 20) | (temp_data << 16) |
		(((nCLK_CONTROL >> 24) & 0xff) << 8) |			//	1st
		(((nCLK_CONTROL >> 16) & 0xff) << 0));			//	2nd
	
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1, (0x1 << 27));
	WDM_WR2SDRAM_ADDR(_ADR_597v2_FPGA_SYMBOL_CLOCK_DDS_CTRL1, 0);
	WaitMsec(1);//Sleep(1);

}
void CDdsCtl::CntlAD9852SymClk_NewBd_DirectAccess(__int64 outfreq_long, int nCLK_CONTROL)
{
	int		i;
	int		temp_data, sig;
	LldPrint_FCall("CntlAD9852SymClk_NewBd_DirectAccess", 0, 0);

#if TVB597A_STANDALONE
ooo;
	if ( CHK_ID(0x00,_BD_ID_593__,_BD_ID_597v2__,0x00) )
		return TLV_NO_ERR;
#endif

	WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, (1 << 4));//IO RESET
	
	//FREQUENCY TUNNING WORD
	//reg. address
	temp_data = 2;
	LldPrint_1("FTW::address", temp_data);
	for (i=0; i < 8; i++)
	{
		sig = 0;
		if ( ((temp_data >> (7-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
	
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}

	//data
	for (i=0; i < 48; i++)
	{
		if ( (i % 8) == 0 )
			LldPrint_1("FTW::data", (unsigned int)((outfreq_long >> (40-i)) & 0xFF));
	
		sig = 0;
		if ( ((outfreq_long >> (47-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
	
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}

	//UPDATE CLOCK
	//reg. address
	temp_data = 5;
	LldPrint_1("UPDATE CLOCK::address", temp_data);
	for (i=0; i < 8; i++)
	{
		sig = 0;
		if ( ((temp_data >> (7-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
	
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}

	//data
	temp_data = AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE;
	for (i=0; i < 32; i++)
	{
		if ( (i % 8) == 0 )
			LldPrint_1("UPDATE CLOCK::data", (temp_data >> (24-i)) & 0xFF);
	
		sig = 0;
		if ( ((temp_data >> (31-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
		
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}

	//CONTROL WORD
	//reg. address
	temp_data = 7;
	LldPrint_1("CONTROL WORD::address", temp_data);
	for (i=0; i < 8; i++)
	{
		sig = 0;
		if ( ((temp_data >> (7-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
	
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}

	//data
	temp_data = nCLK_CONTROL;
	for (i=0; i < 32; i++)
	{
		if ( (i % 8) == 0 )
			LldPrint_1("CONTROL WORD::data", (temp_data >> (24-i)) & 0xFF);
	
		sig = 0;
		if ( ((temp_data >> (31-i)) & 0x01) == 0x01 )
			sig = (1 << 2);
		
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock low
		sig += ( 1 << 1 );
		WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, sig);//clock high
	}
	WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, 0);
	WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, (1 << 3) );// IO UPDATE
	WDM_WR2SDRAM_ADDR(AD9852_SYMBOL_CLOCK_ADDR, 0);

}
int CDdsCtl::CntlAD9852SymClk_OldBd_DirectAccess(__int64 outfreq_long, int nCLK_CONTROL)
{
	int		i;
	int		temp_data;
	int		outfreq_int;
	LldPrint_FCall("CntlAD9852SymClk_OldBd_DirectAccess", 0, 0);

	// phase adjust register download
	temp_data = AD9852_SYMCLK_PHASE_ADJUST_REGISTER_VALUE;
	temp_data &= 0x0000FFFF;
	temp_data <<= 16; // move to MSB
	LldPrint_1("PAR ", temp_data);
	for(i=0; i < 2; i++)
	{
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SYMCLK_SERIAR_ADDR0*8+i), temp_data, 0))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 45);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(1);
		temp_data <<= 8;
	}

	// frequency tunning word download
	for(i=0; i < 6;i++)
	{
		outfreq_int= (int)(outfreq_long >> (40-i*8)) & 0x000000FF;
		outfreq_int= (outfreq_int << 24); // move data to MSB

		LldPrint_1("FTW ", outfreq_int);
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SYMCLK_SERIAR_ADDR1*8+i), outfreq_int, 0))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 46);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(1);
	}

	// update clock download
	temp_data= AD9852_SYMCLK_UPDATE_CLOCK_REGISTER_VALUE;
	temp_data &= 0xFFFFFFFF;
	LldPrint_1("UC ", temp_data);
	for(i=0; i < 4; i++)
	{
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SYMCLK_SERIAR_ADDR2*8+i), temp_data, 0))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 47);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(1);
		temp_data <<= 8; // move to MSB
	}

	// control word download
	//AD9852ASQ/AD9852AST
	temp_data = nCLK_CONTROL;

	temp_data &= 0xFFFFFFFF;
	LldPrint_1("CW ", temp_data);
	for(i=0; i < 4; i++)
	{
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SYMCLK_SERIAR_ADDR3*8+i), temp_data, 0))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 48);
			return -1;
		}
		//6.9.14 - TEST, Vista, DualCore AMD Athlon 64 x2, 2800 MHz (14x200) 5600+
		Sleep(1);
		temp_data <<= 8; // move to MSB
	}

	return	0;
}

/*^^***************************************************************************
 * Description :
 *				
 * Entry : offset, data32, bExternal
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  // Structure of 32bit data for AD9852_BASE_ADDR
 *			bit7	bit6	bit5	bit4		bit3	bit2	bit1	bit0			
 *			--------------------------------------------------------------------------
 *			BYTE0	1		0		0		data7		data6	data5	data4	x
 *			BYTE1	1		0		1		data3		data2	data1	data0	x
 *			BYTE2	1		1		0		addr6		addr5	addr4	addr3	addr2
 *			BYTE3	1		1		1		addr1		addr0	Extern	x		x
 *
 **************************************************************************^^*/
int CDdsCtl::TAD142_SET_AD9852(unsigned char	offset, DWORD data32, unsigned char bExternal)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;
	//DWORD		dwTemp;
	int			iRet;
	BYTE		data = (BYTE) (data32 >> 24);
	DWORD		dwData;		// 상위 data '100' 이하 4bit 는 AD9852 data 8bit 중 상위 

	LldPrint_FCall("TAD142_SET_AD9852", offset, data32);
                           	// 상위 data '101' 이하 4bit 는 AD9852 data 8bit 중 하위 
 
                           	// 상위 data '110' 이하 5bit 는 AD9852 addr 7bit 중 상위 5bit
                          	// 상위 data '111' 이하 2bit 는 AD9852 addr 7bit 중 하위 2bit 
                           	//           이후 1bit는 internal, external 구분...이 bit가 '1' 이면  External, 
                           	//           '0' 이면 internal 입니다. 
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
	LldPrint_x_CIC(":::... wr-ad9852-reg", offset, data32, bExternal);
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
	{
		LldPrint_Error("dds-ctl : pos...", 5, 1);
		return TLV_NO_DEVICE;
	}
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
		{
			LldPrint_Error("dds-ctl : pos...", 5, 2);
			return TLV_NO_DEVICE;
		}
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
		{
			LldPrint_Error("dds-ctl : pos...", 5, 3);
			return TLV_NO_DEVICE;
		}
	}
#endif

	LockDmaMutex();

	//dwData  = (DWORD) ( (0x80 | ((data & 0xF0)   >> 3)) << 24);
	dwData  = (DWORD) ( (0x80 | ((data & 0xF0)   >> 3)));
	//dwData  = (DWORD) ( (0x80 | 0x15) );
	KCmdInf.dwCmdParm1 = AD9852_BASE_ADDR;
	KCmdInf.dwCmdParm2 = dwData;	// Data 

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
#ifdef WIN32
	iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		iRet = TLV_DeviceIoControl_usb(hDevice_usb,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
	else
	{
		iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
#endif
	//sskim20080925 - TEST
	//Sleep(10);
	//Sleep(1);

	//dwData = (DWORD) ( (0xA0 | ((data & 0x0F)   << 1)) << 24);
	dwData = (DWORD) ( (0xA0 | ((data & 0x0F)   << 1)));
	//dwData  = (DWORD) ( (0xA0 | 0x15) );
	KCmdInf.dwCmdParm1 = AD9852_BASE_ADDR;
	KCmdInf.dwCmdParm2 = dwData;	// Data 

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
#ifdef WIN32
	iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		iRet = TLV_DeviceIoControl_usb(hDevice_usb,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
	else
	{
		iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
#endif

	//sskim20080925 - TEST
	//Sleep(10);
	//Sleep(1);

	//dwData = (DWORD) ( (0xC0 | ((offset & 0x7C) >> 2)) << 24);
	dwData = (DWORD) ( (0xC0 | ((offset & 0x7C) >> 2)));
	//dwData  = (DWORD) ( (0xC0 | 0x15) );
	KCmdInf.dwCmdParm1 = AD9852_BASE_ADDR;
	KCmdInf.dwCmdParm2 = dwData;	// Data 

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
#ifdef WIN32
	iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		iRet = TLV_DeviceIoControl_usb(hDevice_usb,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
	else
	{
		iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
#endif

	//sskim20080925 - TEST
	//Sleep(10);
	//Sleep(1);

	dwData = (DWORD) ( (0xE0 | ((offset & 0x03) << 3))); 
	dwData |= (DWORD) ( (bExternal & 0x01) << 2);
	// dwData = dwData << 24;
	dwData = dwData;
	// dwData  = (DWORD) ( (0xE0 | 0x15) );
	KCmdInf.dwCmdParm1 = AD9852_BASE_ADDR;
	KCmdInf.dwCmdParm2 = dwData;	// Data 

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
#ifdef WIN32
	iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		iRet = TLV_DeviceIoControl_usb(hDevice_usb,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
	else
	{
		iRet = TLV_DeviceIoControl(hDevice,
		   IOCTL_WRITE_TO_MEMORY,
		   &KCmdInf,
		   sizeof(KCmdInf),
		   NULL, 0, &dwRet,0);
	}
#endif

	UnlockDmaMutex();

	//=============================================================
	DBG_PLATFORM_BRANCH();
#ifdef WIN32
	return (iRet ? 0:-1);
#else
	return 0;
#endif
}

/*^^***************************************************************************
 * Description : ROSWIN gain control
 *				
 * Entry : R_Reg, N_Reg, F_Reg
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int CDdsCtl::Config_TRF178_PLL2(int R_Reg, int N_Reg, int F_Reg)
{
	LldPrint_Trace("Config_TRF178_PLL2", R_Reg, N_Reg, (double)F_Reg, NULL);
	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
		return 0;
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, R_Reg))
		return -1;
	Sleep(2);

	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, N_Reg))
		return -1;
	Sleep(2);

	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, F_Reg))
		return -1;
	Sleep(2);

	TSPL_RESET_SDCON_backwardcompatible(3);
#endif
	return 0;
}
/*^^***************************************************************************
 * Description : PNP-1500-P22 configuration
 *				
 * Entry : nBoardId(Slot #), nModulatorType, nIF(36000000 or 44000000)
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int CDdsCtl::Reset_TRF178(int nBoardId, int nModulatorType, int nIF)
{
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("Reset_TRF178", nBoardId, nModulatorType);

	if(nModulatorType == TVB380_TSIO_MODE)
		return 0;
	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
		return 0;

#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
	int	reg_data = 0;
	
	int start_freq = UMC_START_FREQ;
	int stop_freq = UMC_STOP_FREQ;
	int step_freq = UMC_STEP_FREQ;
	int ref_freq = UMC_REF_FREQ;

	unsigned long __sht;

////////////////////////////////////////////////////////////
	RdRegistry_FreqStep(nModulatorType, &step_freq, &__sht);

////////////////////////////////////////////////////////////
	if ( IsAttachedBdTyp_WhyThisGrpNeed_14() || IsAttachedBdTyp_499() )
	{
		/* START REG */
		reg_data = 1;	
		reg_data <<= 28;
		reg_data += (9 << 24);
		reg_data += (start_freq/1000);
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("wdm_drv : pos...", 2, 12);
			return -1;
		}

		//TVB590S V2 or higher, TVB593, TVB497
		if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
		{
			if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl() || IsAttachedBdTyp_499() )
			{
				Sleep(10);
				if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
				{
					LldPrint_Error("wdm_drv : pos...", 2, 13);
					return -1;
				}
			}
		}

		Sleep(10);//fixed - ubuntu? 2 -> 10

		/* STOP REG */
		reg_data = 2;
		reg_data <<= 28;
		reg_data += (9 << 24);
		reg_data += (stop_freq/1000);
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("wdm_drv : pos...", 2, 14);
			return -1;
		}

		//TVB590S V2 or higher, TVB593, TVB497
		if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
		{
			if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl() || IsAttachedBdTyp_499() )
			{
				Sleep(10);
				if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
				{
					LldPrint_Error("wdm_drv : pos...", 2, 15);
					return -1;
				}
			}
		}

		Sleep(10);//fixed - ubuntu? 2 -> 10

		/* STEP REG */
		reg_data = 3;
		reg_data <<= 28;
		reg_data += (3 << 24);
		reg_data += step_freq;
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("wdm_drv : pos...", 2, 16);
			return -1;
		}

		//TVB590S V2 or higher, TVB593, TVB497
		if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
		{
			if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl() || IsAttachedBdTyp_499() )
			{
				Sleep(10);
				if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
				{
					LldPrint_Error("wdm_drv : pos...", 2, 17);
					return -1;
				}
			}
		}

		Sleep(10);//fixed - ubuntu? 2 -> 10

		/* REFERENCE REG */
		reg_data = 4;
		reg_data <<= 28;
		reg_data += (6 << 24);
		reg_data += ref_freq;

		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL1_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("wdm_drv : pos...", 2, 18);
			return -1;
		}

		//TVB590S V2 or higher, TVB593, TVB497
		if ( IsAttachedBdTyp_UseAD9775() || IsAttachedBdTyp_499() )
		{
			if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl() || IsAttachedBdTyp_499() )
			{
				Sleep(10);
				if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
				{
					LldPrint_Error("wdm_drv : pos...", 2, 19);
					return -1;
				}
			}
		}

		Sleep(10);//fixed - ubuntu? 2 -> 10

		int nVal0, nVal1, nVal2, nRet;
		nVal0 =	0x410;
		nVal1 = 0x111D01;
		if ( nIF != IF_OUT_36MHZ )	
		{
			nVal1 = 0x111A81;
		}
		nVal2 = 0x92;

		if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis() )
		{
			nVal0 =	ROWSWIN_REG_VAL0;
			nVal1 = ROWSWIN_REG_VAL1_36M;
			if ( nIF != IF_OUT_36MHZ )	
			{
				nVal1 = ROWSWIN_REG_VAL1_44M;
			}
			nVal2 = ROWSWIN_REG_VAL2;

			if ( nModulatorType == TVB380_DTMB_MODE )
			{
				nVal0 =	ROWSWIN_REG_VAL0_DTMB;
				nVal1 = ROWSWIN_REG_VAL1_36M_DTMB;
				if ( nIF != IF_OUT_36MHZ )	
				{
					nVal1 = ROWSWIN_REG_VAL1_44M_DTMB;
				}
				nVal2 = ROWSWIN_REG_VAL2_DTMB;
			}
		}

		nRet = Config_TRF178_PLL2(nVal0, nVal1, nVal2);
		if ( nRet != 0 )
		{
			LldPrint_Error("wdm_drv : pos...", 2, 20);
			return -1;
		}
	}
	
#endif
	return 0;
}

/*^^***************************************************************************
 * Description : Control the selected modulator's carrier
 *				
 * Entry : modulation_mode, dwfreq
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CDdsCtl::CntlClkGen_RFout_Carrier(int modulation_mode, unsigned long dwfreq)//2150MHz support - int -> unsigned long
{
	int		i;
	double	outfreq_float;
	__int64		outfreq_long;
	int		outfreq_int;
	int		temp_data;
	double	oper_freq = 0;//MHz
	int		step_freq = UMC_STEP_FREQ;

	LldPrint_FCall("CntlClkGen_RFout_Carrier", modulation_mode, dwfreq);
	DBG_PLATFORM_BRANCH();

////////////////////////////////////////////////////////////
	RdRegistry_FreqStep(modulation_mode, &step_freq, &gRF_143KHZ);

////////////////////////////////////////////////////////////FIXED - IF +- offset
	gRF_143KHZ = (unsigned long)fmod((double)dwfreq, 1000000);
	gRF_143KHZ_sign = -1;
	if ( modulation_mode == TVB380_TDMB_MODE || modulation_mode == TVB380_DVBC2_MODE) //2011/5/9 DVB-C2
	{
		gRF_143KHZ = 0;
	}

	//FIXED - IF +- offset //TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAD9775() )
	{
		gRF_143KHZ = 0;
	}

////////////////////////////////////////////////////////////	cntl. trf178
	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
	{
		Cntl_HMC833(dwfreq, step_freq);
		return 0;
	}
	else if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc() || IsAttachedBdTyp_499()) 
	{
		if ( IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1() || IsAttachedBdTyp_499())	//	593/597v2 or 590s higher rev3
		{	
			CntlTrf178_NewBd_Case_1(dwfreq, step_freq);	//	593/597v2 or 590s higher rev3
		}
		else if ( TSPL_nBoardRevision >= __REV_2_0__of_bd_593__ )	//	tvb590s rev2
		{
			CntlTrf178_NewBd_Case_2(dwfreq, step_freq);	//	tvb590s rev2s
		}
		else	//	tvb590s rev1
		{
			CntlTrf178_NewBd_Case_3(dwfreq, step_freq);	//	tvb590s rev1
		}

		return 0;
	}

	if ( IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin() || IsAttachedBdTyp_497() )	//TVB497 +TVO1624
	{
		CntlTrf178_OldBd_Case_1(modulation_mode, dwfreq, step_freq);
		return 0;
	}

////////////////////////////////////////////////////////////	the other case. they does not use trf178
	// download AD9852 configure data
	outfreq_float= (double)(dwfreq - gChannel_Freq_Offset) / 8. / 1000000.0 * 938249922368.85333333333333333333; // (2^48 / 300)
	outfreq_long= (__int64)outfreq_float;
	LldPrint_2("Output Freq-.(float/long)", (int)outfreq_float, (int)outfreq_long);

	// phase adjust register download
	temp_data= PHASE_ADJUST_REGISTER_VALUE;
	temp_data &= 0x0000FFFF;
	temp_data <<= 16; // move to MSB
	LldPrint_1("PAR ", temp_data);
	for(i=0; i < 2; i++)
	{
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SERIAR_ADDR0*8+i), temp_data, 1))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 27);
			return -1;
		}
		//sskim20080925 - TEST
		//Sleep(1);
		temp_data <<= 8;
	}

	// frequency tuning word download
	for(i=0; i < 6;i++)
	{
		outfreq_int= (int)(outfreq_long >> (40-i*8)) & 0x000000FF;
		outfreq_int= (outfreq_int << 24); // move data to MSB

		LldPrint_1("FTW ", outfreq_int);
	 	if (TAD142_SET_AD9852((BYTE)(AD9852_SERIAR_ADDR1*8+i), outfreq_int, 1))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 28);
			return -1;
		}
		//sskim20080925 - TEST
		//Sleep(1);
	}

	// update clock download
	temp_data= UPDATE_CLOCK_REGISTER_VALUE;
	temp_data &= 0xFFFFFFFF;
	LldPrint_1("UC ", temp_data);
	for(i=0; i < 4; i++)
	{
		if (TAD142_SET_AD9852((BYTE)(AD9852_SERIAR_ADDR2*8+i), temp_data, 1))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 29);
			return -1;
		}
		//sskim20080925 - TEST
		//Sleep(1);
		temp_data <<= 8; // move to MSB
	}

	// control word download
	temp_data = CONTROL_REGISTER_VALUE;
	if ( IsAttachedBdTyp_380v3_ifrf_scaler() )
	{
		temp_data = CONTROL_REGISTER_SCALED;
	}
	temp_data &= 0xFFFFFFFF;
	LldPrint_1("CW ", temp_data);
	for(i=0; i < 4; i++)
	{
		if (TAD142_SET_AD9852((BYTE)(AD9852_SERIAR_ADDR3*8+i), temp_data, 1))
		{
			LldPrint_Error("dds-ctl : pos...", 5, 30);
			return -1;
		}
		temp_data <<= 8; // move to MSB
	}

	if ( IsAttachedBdTyp_380v3_ifrf_scaler() )
	{
		temp_data = DIGITAL_MULTIPLIER_VALUE;
		temp_data &= 0x0000FFFF;
		temp_data <<= 16; // move to MSB
		LldPrint_1("DM ", temp_data);
		for(i=0; i < 2; i++)
		{
	 		if (TAD142_SET_AD9852((BYTE)(AD9852_SERIAR_ADDR4*8+i), temp_data, 1))
			{
				LldPrint_Error("dds-ctl : pos...", 5, 31);
				return -1;
			}
			temp_data <<= 8;
		}
	}
	return 0;
}

/*^^***************************************************************************
 * Description : Control the selected modulator's symbol clock
 *				
 * Entry : modulation_mode, dwfreq
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CDdsCtl::CntlClkGen_AD9852_Symclock(int modulation_mode, int symbol_freq_or_bandwidth)
{
	__int64		outfreq_long;
	int	symbol_adjust = 1;
	int		nCLK_CONTROL = AD9852ASQ_SYMCLK_CONTROL_REGISTER_VALUE;	//AD9852ASQ/AD9852AST
	double	dwCLK_CONST = CLK_CONST_DEF(2,48,288);	//AD9852ASQ

	LldPrint_FCall("CntlClkGen_AD9852_Symclock", modulation_mode, 0);
	DBG_PLATFORM_BRANCH();

	RdRegistry_SymClkAdj(modulation_mode, &symbol_adjust);
	CalcClkCntlCnst_(modulation_mode, symbol_adjust, symbol_freq_or_bandwidth, &nCLK_CONTROL, &dwCLK_CONST);

	LldPrint("[LLD]===CONFIG AD9852 SYMBOL CLOCK, Type, RF, BW/SYMBOLRATE", (int)modulation_mode, (int)0, (int)symbol_freq_or_bandwidth);

	if (CalcOutFreq_(modulation_mode, &symbol_freq_or_bandwidth, dwCLK_CONST, &outfreq_long) == -1)
	{
		return -1;
	}

	// download AD9852_SYMCLK configure data
	if (IsAttachedBdTyp_IndirectCntlMethodAd9852Reg() || IsAttachedBdTyp_499())
	{
		CntlAD9852SymClk_NewBd_IndirectAccess(outfreq_long, nCLK_CONTROL);
		return	0;	//	return
	}
	if ( IsAttachedBdTyp_DiffCntlMethodAd9852Reg() )
	{
		CntlAD9852SymClk_NewBd_DirectAccess(outfreq_long, nCLK_CONTROL);
		return	0;	//	return
	}
	CntlAD9852SymClk_OldBd_DirectAccess(outfreq_long, nCLK_CONTROL);

	return	0;
}

//TVB593 - ISDB-T, ATSC-M/H
int _stdcall CDdsCtl::TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock)
{
	int		i;
	double 	outfreq_float;
	__int64 outfreq_long;
	int		outfreq_int;
	double	symbol_clk;

	int nCLK_CONTROL = AD9852ASQ_SYMCLK_CONTROL_REGISTER_VALUE;
	double dwCLK_CONST = CLK_CONST_DEF(2,48,288);//AD9852ASQ

	LldPrint("TSPL_SET_SYMBOL_CLOCK", modulator_type, symbol_clock, 0);
//2014/10/20
//#ifdef WIN32
	//LockDmaMutex();
	//if ( fDMA_Busy == 1 )
	//{
	//	UnlockDmaMutex();
	//	return 0;
	//}
//#endif
	if(nClockGeneratorModel == HW_HMC1033_988)
	{
		int	nHmc988_Divide_Ratio;	//HMC988 REG 02
		double dwHmc1033_Clock;		//MHz
		int nRF_Divider;			//HMC1033 vco reg 02 [5:0]
		double dwFvco;				//MHz
		int	nHmc1033_Integer;		//HMC1033 REG 03h Frequency Register - Integer Part
		int nHmc1033_Fractional;	//HMC1033 REG 04h Frequency Register - Fractional Part
		double temp_Fractional;
		symbol_clk = (double)symbol_clock;//65.015872;
		symbol_clk /= 1000000.;
		//HMC988 Divide Ratio
		if(symbol_clk < 6.25)
		{
			nHmc988_Divide_Ratio = 3;
		}
		else if(symbol_clk < 12.5)
		{
			nHmc988_Divide_Ratio = 2;
		}
		else if(symbol_clk < 25.)
		{
			nHmc988_Divide_Ratio = 1;
		}
		else
		{
			nHmc988_Divide_Ratio = 0;
		}
		//Set HMC988
		Set_HMC988_register(nHmc988_Divide_Ratio);

		//HMC1033 Clock (MHz)
		dwHmc1033_Clock = symbol_clk * (double)(1 << nHmc988_Divide_Ratio);
		
		//RF divider(N)
		for(int i = 62; i >= 0; i--)
		{
			if((dwHmc1033_Clock * (double)i) <= 3000.)
			{
				nRF_Divider = i;
				break;
			}
			if(i > 2)
				i--;
		}

		//Fvco (MHz)
		dwFvco = dwHmc1033_Clock * (double)nRF_Divider;

		//HMC1033 REG 03h Frequency Register - Integer Part
		nHmc1033_Integer = (int)(dwFvco / 20.);

		//HMC1033 REG 04h Frequency Register - Fractional Part
		temp_Fractional = ((dwFvco - (double)(nHmc1033_Integer * 20)) / 20.0) * 16777216.0;
		nHmc1033_Fractional = (int)temp_Fractional;
		if((temp_Fractional - (double)nHmc1033_Fractional) >= 0.5)
			nHmc1033_Fractional++;
		Set_HMC1033_register(nRF_Divider, nHmc1033_Integer, nHmc1033_Fractional);
//2014/10/20		
//#ifdef WIN32
		//UnlockDmaMutex();
//#endif
		return 0;
	}

	LldPrint_2("[LLD]===TSPL_SET_SYMBOL_CLOCK, Type, SYMBOL CLOCK", (int)modulator_type, (int)symbol_clock);

	nCLK_CONTROL = AD9852AST_SYMCLK_CONTROL_REGISTER_VALUE;
	dwCLK_CONST = CLK_CONST_DEF(2,48,192);//AD9852AST
	if ( IsAttachedBdTyp_AD9852Clk_ASx_2() )
	{
		if(IsAttachedBdTyp_AD9852Clk_ASx_V())	//	597v2 rev2.1, 593 rev2.2, 590s rev2.2
		{
			unsigned long dwStatus = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);
			LldPrint_1("[LLD] AD9852 INFO VALUE = ", dwStatus);
			if((dwStatus & 0x3) == 0x1)
			{
				LldPrint("[LLD] 10MHz AD9852 REF INPUT");
			}
			else
			{
				LldPrint("[LLD] 20MHz AD9852 REF INPUT");
			}

			if(((dwStatus >> 2) & 0x3) == 0x1)
			{
				LldPrint("[LLD] AD9852 ASV model(300MHz)");
			}
			else
			{
				LldPrint("[LLD] AD9852 AST model(200MHz)");
			}
			nCLK_CONTROL = 0x040F0140;
			dwCLK_CONST = CLK_CONST_DEF(2,48,300);//AD9852ASV, PCI Express
		}
		else
		{
			nCLK_CONTROL = 0x040A0140;
			dwCLK_CONST = CLK_CONST_DEF(2,48,200);//AD9852AST, PCI Express
		}
//		nCLK_CONTROL = 0x040A0140;
//		dwCLK_CONST = CLK_CONST_DEF(2,48,200);//AD9852AST, PCI Express
	}

	//TVB497
	if ( IsAttachedBdTyp_497() )
	{
		nCLK_CONTROL = 0x040A0140;
		dwCLK_CONST = CLK_CONST_DEF(2,48,100);
	}
	else if(IsAttachedBdTyp_499() )
	{
			nCLK_CONTROL = 0x040F0140;
			dwCLK_CONST = CLK_CONST_DEF(2,48,300);//AD9852ASV, PCI Express
	}

	symbol_clk = (double)symbol_clock;//65.015872;
	symbol_clk /= 1000000.;
	outfreq_float= symbol_clk * dwCLK_CONST;
	outfreq_long= (__int64)outfreq_float;
	
	if ( IsAttachedBdTyp_WhyThisGrpNeed_13() || IsAttachedBdTyp_499())
	{
		if (IsAttachedBdTyp_IndirectCntlMethodAd9852Reg() || IsAttachedBdTyp_499())
		{
			CntlAD9852SymClk_NewBd_IndirectAccess(outfreq_long, nCLK_CONTROL);
		}
		else if ( IsAttachedBdTyp_DiffCntlMethodAd9852Reg() )
		{
			CntlAD9852SymClk_NewBd_DirectAccess(outfreq_long, nCLK_CONTROL);
		}
	}
	else
	{
		for(i=0; i < 6;i++)
		{
			outfreq_int= (int)(outfreq_long >> (40-i*8)) & 0x000000FF;
			outfreq_int= (outfreq_int << 24); // move data to MSB

			LldPrint_1("FTW ", outfreq_int);
	 		if (TAD142_SET_AD9852((BYTE)(AD9852_SYMCLK_SERIAR_ADDR1*8+i), outfreq_int, 0))
			{
//2014/10/20
//#ifdef WIN32
				//UnlockDmaMutex();
//#endif
				LldPrint_Error("dds-ctl : pos...", 5, 49);
				return -1;
			}
		}

		WDM_WR2SDRAM_ADDR(TSP_MEM_ISDBT_SYMBOL_CLOCK_CTRL, 0x00000000);
		WDM_WR2SDRAM_ADDR(TSP_MEM_ISDBT_SYMBOL_CLOCK_CTRL, 0x00000001);
		WDM_WR2SDRAM_ADDR(TSP_MEM_ISDBT_SYMBOL_CLOCK_CTRL, 0x00000000);
	}
//2014/10/20
//#ifdef WIN32
	//UnlockDmaMutex();
//#endif
	return 0;
}

//2011/5/4 AD9852 MAX CLK
#ifdef WIN32
int _stdcall CDdsCtl::TSPL_SET_AD9852_MAX_CLOCK(long value)
{
	LldPrint("TSPL_SET_AD9852_MAX_CLOCK", value, 0, 0);
	ad9852_Max_clk = value;
	return 0;
}
#endif

//2012/7/9 HMC833
int CDdsCtl::Cntl_HMC833(unsigned long dwfreq, int step_freq)
{
	int		reg_data = 0;
	double	fLO, fRF;
	int		RF_sw = 0;
	int		VCO_reg02_N, VCO_reg02_Gain;
	int		VCO_reg03_DivideMode;
	int		VCO_reg02, VCO_reg03;
	double	Fvco, reg04_temp;
	int		reg03_value, reg04_value;
	int		reg05_value;
	static int set_flag = 0;
	int		Hmc833_mode;	// 0: HMC mode, 1: Open mode
	int		Hmc1033_mode;	// 0: HMC mode, 1: Open mode
	int		sel_hmc_open;
	int		address_shift_value;
	int		data_shift_value;

	LldPrint_1x("[LLD]call Cntl_HMC833", dwfreq);

	Hmc833_mode = gnHmc833_SerialPortMode;
	if(Hmc833_mode == -1)
	{
		LldPrint_Error("HMC833 error", 0, Hmc833_mode);
		return -1;
	}
	Hmc1033_mode = gnHmc1033_SerialPortMode;
	sel_hmc_open = ((Hmc833_mode & 0x1) << 1) + ((Hmc1033_mode & 0x1) << 2);
	fRF = dwfreq/1000.;
	fLO = fRF*2.;

	//VCO_REG 0x02h[5:0]
	VCO_reg02_N = (int)(3000000. / fLO);
	if((VCO_reg02_N % 2) == 1)
	{
		VCO_reg02_N--;
	}
	if(VCO_reg02_N > 62)
		VCO_reg02_N = 62;
	if(fLO >= 1500000)
		VCO_reg02_N = 1;

	LldPrint_1x("[HMC833] N value", VCO_reg02_N);

	//VCO_REG 0x02h[7:6]
	if(fLO <= 1000000.)
		VCO_reg02_Gain = 3;
	else if(fLO < 1500000.)
		VCO_reg02_Gain = 0;
	else if(fLO <= 1700000.)
		VCO_reg02_Gain = 2;
	else if(fLO <= 2800000.)
		VCO_reg02_Gain = 0;
	else if(fLO <= 3000000.)
		VCO_reg02_Gain = 3;
	else if(fLO <= 3200000.)
		VCO_reg02_Gain = 2;
	else 
		VCO_reg02_Gain = 0;

	LldPrint_1x("[HMC833] PLL gain", VCO_reg02_Gain);

	//VCO_REG 0x03h[0]
	if(fLO <= 3000000.)
		VCO_reg03_DivideMode = 1;
	else
		VCO_reg03_DivideMode = 0;

	LldPrint_1x("[HMC833] Divide Mode", VCO_reg03_DivideMode);

	//Fvco
	if(VCO_reg03_DivideMode == 1)
		Fvco = fLO * (double)VCO_reg02_N;
	else
		Fvco = fLO * (double)VCO_reg02_N / 2.;

	// Reg 0x03h Frequency Register - Integer Part
	reg03_value = (int)(Fvco / 20000);

	LldPrint_1x("[HMC833] REG 0x03h", reg03_value);
	

	// Reg 0x04h Frequency Register - Fractional Part
	reg04_temp = ((double)(Fvco - (double)(reg03_value * 20000.)) / 20000. * 16777216.);   
	reg04_value = (int)reg04_temp;
	if((double)(reg04_temp - (double)reg04_value) >= 0.5)
	{
		reg04_value++;
	}
	LldPrint_1x("[HMC833] REG 0x04h", reg04_value);
	
	if(fLO <= 150000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (1 << 24) + (1 << 12) + (1 << 8);
	}
	else if(fLO <= 300000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (0x7 << 23) + (1 << 20) + (1 << 12) + (1 << 8);
	}
	else if(fLO <= 500000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (0x7 << 23) + (0x7 << 19) + (0x7 << 11) + (1 << 8);
	}
	else if(fLO <= 860000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (1 << 22) + (1 << 13) + (0x3 << 10) + (1 << 8);
	}
	else if(fLO <= 1200000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (0x7 << 23) + (1 << 21) + (0x3 << 18) + (1 << 13) + (0x3 << 10) + (1 << 8);
	}
	else if(fLO <= 2200000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (0x7 << 23) + (1 << 18) + (1 << 13) + (0x3 << 10) + (1 << 8);
	}
	else if(fLO <= 2900000.0)
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (1 << 25) + (0x3 << 22) + (1 << 13) + (0x3 << 10) + (1 << 8);
	}
	else
	{
		RF_sw = ((sel_hmc_open & 0x7) << 28) + (1 << 27) + (1 << 25) + (0x3 << 22) + (1 << 9);
	}

	LldPrint_1x("[HMC833] RF_SW", RF_sw);

	//0x58200
	reg_data = (int)WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);
	reg_data = (((RF_sw >> 8) & 0xFFFFFF) << 8) + (reg_data&0xFF);

	if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 1);
		return -1;
	}

	if(Hmc833_mode == 0)
	{
		address_shift_value = 25;
		data_shift_value = 1;
	}
	else
	{
		address_shift_value = 3;
		data_shift_value = 8;
	}
if(set_flag == 0)
{
#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	LldPrint_1x("[HMC] Set 0x58200", reg_data);

	//0x58100 REG 0x9h
	reg_data = (0x9 << address_shift_value) + (0x547FFF << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x9", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 2);
		return -1;
	}
	
#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0xAh
	reg_data = (0xA << address_shift_value) + (0x2046 << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0xA", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 3);
		return -1;
	}

#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0xBh
	reg_data = (0xB << address_shift_value) + (0x7C021 << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0xA", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 4);
		return -1;
	}
set_flag = 0;
}
#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0x5h VCO_REG0x2
	VCO_reg02 = (VCO_reg02_Gain << 6) + VCO_reg02_N;
	reg05_value = (VCO_reg02 << 7) + (0x2 << 3);
	reg_data = (0x5 << address_shift_value) + (reg05_value << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x5", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 5);
		return -1;
	}
	
#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0x5h VCO_REG0x3
	VCO_reg03 = (2 << 5) + (2 << 3) + VCO_reg03_DivideMode;
	reg05_value = (VCO_reg03 << 7) + (0x3 << 3);
	reg_data = (0x5 << address_shift_value) + (reg05_value << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x5", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 6);
		return -1;
	}

#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0x5h 
	reg05_value = 0;
	reg_data = (0x5 << address_shift_value) + (reg05_value << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x5", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 7);
		return -1;
	}

#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0x3h 
	reg_data = (0x3 << address_shift_value) + (reg03_value << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x3", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 8);
		return -1;
	}

#ifdef WIN32
	Sleep(10);
#else
	Sleep(1);
#endif
	//0x58100 REG 0x4h 
	reg_data = (0x4 << address_shift_value) + (reg04_value << data_shift_value);
	LldPrint_1x("[HMC] Set 0x58100 Reg0x4", reg_data);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC : pos...", 1, 9);
		return -1;
	}
#ifdef WIN32
	Sleep(10);
#else
	Sleep(5);
#endif
	//0x58100 REG 0x12 check lock status
	if(Hmc833_mode == 0)
		reg_data = (1 << 31) + (0x12 << address_shift_value);
	else
		reg_data = 0x12;
	int timeout = 10;
	unsigned long nStatus;
	while(1)
	{
		timeout--;
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, reg_data))
		{
			LldPrint_Error("HMC : pos...", 1, 10);
			return -1;
		}
#ifdef WIN32
		Sleep(10);
#else
		Sleep(5);
#endif
		nStatus = WDM_Read_TSP(FPGA_HMC833_READ_DATA_REG2);
		if(((nStatus >> data_shift_value) & 0x2) > 0)
		{
			TSPL_HMC833_status = 0;
			break;
		}
		
		if(timeout == 0)
		{
			LldPrint_Error("HMC : check Lock status [time out].", 1, 11);
			TSPL_HMC833_status = 1;
			break;
		}
	}
	return	0;
}

//2013/5/22 TVB599
int CDdsCtl::CntlClkGen_HMC_1033_988_Symclock(int modulation_mode, int symbol_freq_or_bandwidth)
{
	double symbol_clock;		//MHz
	int	nHmc988_Divide_Ratio;	//HMC988 REG 02
	double dwHmc1033_Clock;		//MHz
	int nRF_Divider;			//HMC1033 vco reg 02 [5:0]
	double dwFvco;				//MHz
	int	nHmc1033_Integer;		//HMC1033 REG 03h Frequency Register - Integer Part
	int nHmc1033_Fractional;	//HMC1033 REG 04h Frequency Register - Fractional Part
	double temp_Fractional;
	//Symbol clock
	if(modulation_mode == TVB380_DVBT_MODE || modulation_mode == TVB380_DVBH_MODE || modulation_mode == TVB380_MULTI_DVBT_MODE )
	{
		switch(symbol_freq_or_bandwidth)
		{
				case DVB_T_6M_BAND:
					symbol_clock = 48;
					break;
				case DVB_T_7M_BAND:
					symbol_clock = 56;
					break;
				case DVB_T_8M_BAND:
					symbol_clock = 64;//MHz
					break;
				case DVB_T_5M_BAND:
					symbol_clock = 40;
					break;
				default:
					LldPrint_Error("CntlClkGen_HMC_1033_988_Symclock : pos...", 6, 1);
					return -1;//invalid bandwidth
		}
	}
	else if(modulation_mode == TVB380_VSB8_MODE || modulation_mode == TVB380_VSB16_MODE || modulation_mode == TVB380_ATSC_MH_MODE || modulation_mode == TVB380_MULTI_VSB_MODE )
	{
		symbol_freq_or_bandwidth = 4500000;
		symbol_clock = ((double)symbol_freq_or_bandwidth / 1000000.0) * (684./286.) * 4.;
	}
	else if ( modulation_mode == TVB380_QAMA_MODE || modulation_mode == TVB380_QAMB_MODE || modulation_mode == TVB380_MULTI_QAMB_MODE )
	{
			if (symbol_freq_or_bandwidth < 1000000 || symbol_freq_or_bandwidth > 8000000)
			{
				LldPrint_Error("CntlClkGen_HMC_1033_988_Symclock : pos...", 6, 2);
				return -1;
			}
			symbol_clock = (double)(symbol_freq_or_bandwidth * 8) / 1000000.0;
	}
	else if( modulation_mode == TVB380_QPSK_MODE || modulation_mode == TVB380_DVBS2_MODE )
	{
			if (symbol_freq_or_bandwidth < 1000000 || symbol_freq_or_bandwidth > 45000000)
			{
				LldPrint_Error("CntlClkGen_HMC_1033_988_Symclock : pos...", 6, 3);
				return -1;
			}
			
			if ( symbol_freq_or_bandwidth >= 1000000 && symbol_freq_or_bandwidth < 5000000 )
			{
				symbol_clock = 4 * symbol_freq_or_bandwidth;
			}
			else if ( symbol_freq_or_bandwidth >= 5000000 && symbol_freq_or_bandwidth < 10000000 )
			{
				symbol_clock = 2 * symbol_freq_or_bandwidth;
			}
			else
			{
				symbol_clock = 1 * symbol_freq_or_bandwidth;
			}

			symbol_clock = symbol_clock / 1000000.0;
	}
	else if ( modulation_mode == TVB380_TDMB_MODE )
	{
			symbol_freq_or_bandwidth = 2048000;
			symbol_clock = ((double)symbol_freq_or_bandwidth / 1000000.0) * 12.;
	}
	else if ( modulation_mode == TVB380_ISDBT_MODE || modulation_mode == TVB380_ISDBT_13_MODE )
	{
#ifdef WIN32
		if(symbol_freq_or_bandwidth == 0)
			symbol_clock = 65.015872;	//6MHz
		else if(symbol_freq_or_bandwidth == 1)
			symbol_clock = 75.851851;	//7MHz
		else
			symbol_clock = 86.68783;		//8MHz
#else
		symbol_clock = 65.015872;	//6MHz
#endif
	}
	else if ( modulation_mode == TVB380_DTMB_MODE )
	{
			symbol_clock = 60.48;//MHz
			if(symbol_freq_or_bandwidth == 0)
				symbol_clock = symbol_clock * 6.0 / 8.0;
			else if(symbol_freq_or_bandwidth == 1)
				symbol_clock = symbol_clock * 7.0 / 8.0;
	}
	else if ( modulation_mode == TVB380_CMMB_MODE )
	{
			symbol_clock = 60.0;//MHz
	}
	else if (( modulation_mode == TVB380_DVBT2_MODE ) || (modulation_mode == TVB380_DVBC2_MODE))
	{
			if ( symbol_freq_or_bandwidth >= DVB_T_6M_BAND && symbol_freq_or_bandwidth <= DVB_T_8M_BAND )
			{
				if(TSPL_T2_x7_Mod_Clock == 1)
					symbol_clock = ((double)symbol_freq_or_bandwidth + 6.)*8.;
				else
					symbol_clock = ((double)symbol_freq_or_bandwidth + 6.)*8.*(8./7.);
			}
			else
			{
				if ( symbol_freq_or_bandwidth == DVB_T_5M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 5.*8.;
					else
						symbol_clock = 5.*8.*(8./7.);
				}
				else if ( symbol_freq_or_bandwidth == DVB_T_1_7M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = (131. / 71.) * 7.;
					else
						symbol_clock = (131. / 71.) * 8.;   //1.7*8.*(8./7.);
				}
				else if ( symbol_freq_or_bandwidth == DVB_T_10M_BAND )
				{
					if(TSPL_T2_x7_Mod_Clock == 1)
						symbol_clock = 10.*8.;
					else
						symbol_clock = 10.*8.*(8./7.);
				}
			}
	}
	else if ( modulation_mode == TVB380_IQ_PLAY_MODE )
	{
			if (symbol_freq_or_bandwidth < 4000000 || symbol_freq_or_bandwidth > 16000000)
			{
				LldPrint_Error("CntlClkGen_HMC_1033_988_Symclock : pos...", 6, 5);
				return -1;
			}
			symbol_clock = (double)symbol_freq_or_bandwidth / 1000000.;
	}
	else  if ( modulation_mode == TVB380_ISDBS_MODE )
	{
			symbol_clock = 57.72;//28.860*2
	}
	else
	{
		LldPrint_Error("CntlClkGen_HMC_1033_988_Symclock : pos...", 6, 6);
	}

	//HMC988 Divide Ratio
	if(symbol_clock < 6.25)
	{
		nHmc988_Divide_Ratio = 3;
	}
	else if(symbol_clock < 12.5)
	{
		nHmc988_Divide_Ratio = 2;
	}
	else if(symbol_clock < 25.)
	{
		nHmc988_Divide_Ratio = 1;
	}
	else
	{
		nHmc988_Divide_Ratio = 0;
	}
	//Set HMC988
	Set_HMC988_register(nHmc988_Divide_Ratio);

	//HMC1033 Clock (MHz)
	dwHmc1033_Clock = symbol_clock * (double)(1 << nHmc988_Divide_Ratio);
	
	//RF divider(N)
	for(int i = 62; i >= 0; i--)
	{
		if((dwHmc1033_Clock * (double)i) <= 3000.)
		{
			nRF_Divider = i;
			break;
		}
		if(i > 2)
			i--;
	}

	//Fvco (MHz)
	dwFvco = dwHmc1033_Clock * (double)nRF_Divider;

	//HMC1033 REG 03h Frequency Register - Integer Part
	nHmc1033_Integer = (int)(dwFvco / 20.);

	//HMC1033 REG 04h Frequency Register - Fractional Part
	temp_Fractional = ((dwFvco - (double)(nHmc1033_Integer * 20)) / 20.0) * 16777216.0;
	nHmc1033_Fractional = (int)temp_Fractional;
	if((temp_Fractional - (double)nHmc1033_Fractional) >= 0.5)
		nHmc1033_Fractional++;
	Set_HMC1033_register(nRF_Divider, nHmc1033_Integer, nHmc1033_Fractional);
	return 0;
}
int CDdsCtl::Set_HMC988_register(int nDivideRatio)
{
	static int nDivideRatio_prev = -1;
	static int nInitializeHMC988 = 0;
	unsigned long nValue;
	LldPrint_1("Set_HMC988_register : Divide Ratio", nDivideRatio);
	if(nDivideRatio != nDivideRatio_prev)
	{
		if(nInitializeHMC988 == 0)
		{
			//master enable setting
			nValue = 0x789;
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC988_CTRL_REG, nValue);
			if(IsAttachedBdTyp_598())
			{
				Sleep(10);
			}
			//configuration setting
			nValue = 0x1421;
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC988_CTRL_REG, nValue);
			if(IsAttachedBdTyp_598())
			{
				Sleep(10);
			}
			//GPO setting
			nValue = 0x29;
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC988_CTRL_REG, nValue);
			if(IsAttachedBdTyp_598())
			{
				Sleep(10);
			}
			nInitializeHMC988 = 1;
		}
		//REG 02h Divide Ratio
		nValue = (nDivideRatio << 7) + (2 << 3) + 1;
		TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC988_CTRL_REG, nValue);
		if(IsAttachedBdTyp_598())
		{
			Sleep(10);
		}
		nDivideRatio_prev = nDivideRatio;
	}
	return 0;
}
int CDdsCtl::Set_HMC1033_register(int nRF_Divider, int nInteger, int nFractional)
{
	static int nInitializeHMC1033 = 0;
	int nValue;
	int nRegData;
	int Hmc1033_mode;
	int address_shift_value;
	int data_shift_value;
	int rd_flag;
	int timeout = 0;

	LldPrint_4("Set_HMC1033_register : RF divider, Integer part, Fractional part", nRF_Divider, nInteger, nFractional, 0);

	Hmc1033_mode = gnHmc1033_SerialPortMode;
	
	if(Hmc1033_mode == 0)
	{
		address_shift_value = 25;
		data_shift_value = 1;
		rd_flag = 1;
	}
	else
	{
		address_shift_value = 3;
		data_shift_value = 8;
		rd_flag = 0;
	}

	if(nInitializeHMC1033 == 0)
	{
		nInitializeHMC1033 = 1;
		//REG 09h Charge pump Register
		//nValue = 0x1246E0C0;
		nValue = (0x9 << address_shift_value) + (0x237060 << data_shift_value);
		LldPrint_1x("[HMC1033] Set 0x57300 Reg0x9", nValue);
		TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
		if(IsAttachedBdTyp_598())
		{
			Sleep(10);
			timeout = 0;
			while(1)
			{
				nValue = (rd_flag << 31) + (0x9 << address_shift_value);
				TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
				Sleep(1);
				nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
				if(((nValue >> data_shift_value) & 0xFFFFFF) == 0x237060)
					break;
				if(++timeout == 10)
				{
					LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0x9 timeout", nValue);
					break;
				}
			}
		}
		//REG 0Ah VCO AutoCal Configuration Register
		//nValue = 0x1400400C;
		nValue = (0xA << address_shift_value) + (0x2006 << data_shift_value);
		LldPrint_1x("[HMC1033] Set 0x57300 Reg0xA", nValue);
		TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
		if(IsAttachedBdTyp_598())
		{
			Sleep(10);
			timeout = 0;
			while(1)
			{
				nValue = (rd_flag << 31) + (0xA << address_shift_value);
				TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
				Sleep(1);
				nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
				if(((nValue >> data_shift_value) & 0xFFFFFF) == 0x2006)
					break;
				if(++timeout == 10)
				{
					LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0xA timeout", nValue);
					break;
				}
			}
		}
		//REG 0Bh PD Register
		//nValue = 0x161F00C2;
		nValue = (0xB << address_shift_value) + (0xF8061 << data_shift_value);
		LldPrint_1x("[HMC1033] Set 0x57300 Reg0xB", nValue);
		TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
		if(IsAttachedBdTyp_598())
		{
			Sleep(10);
			timeout = 0;
			while(1)
			{
				nValue = (rd_flag << 31) + (0xB << address_shift_value);
				TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
				Sleep(1);
				nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
				if(((nValue >> data_shift_value) & 0xFFFFFF) == 0xF8061)
					break;
				if(++timeout == 10)
				{
					LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0xB timeout", nValue);
					break;
				}
			}
		}
		//REG 05h // VCO Configuration setting
		//nValue = 0xA009F30;
		nValue = (0x5 << address_shift_value) + (0x4F98 << data_shift_value);
		LldPrint_1x("[HMC1033] Set 0x57300 Reg0x5", nValue);
		TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
		if(IsAttachedBdTyp_598())
		{
			Sleep(10);
			timeout = 0;
			while(1)
			{
				nValue = (rd_flag << 31) + (0x5 << address_shift_value);
				TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
				Sleep(1);
				nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
				if(((nValue >> data_shift_value) & 0xFFFFFF) == 0x4F98)
					break;
				if(++timeout == 10)
				{
					LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0xB timeout", nValue);
					break;
				}
			}
		}
	}
	
	// VCO SPI register REG 05h
	nRegData = (nRF_Divider << 7) + (2 << 3);
	nValue = (0x5 << address_shift_value) + (nRegData << data_shift_value);
	LldPrint_1x("[HMC1033] Set 0x57300 Reg0x5", nValue);
	TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
	if(IsAttachedBdTyp_598())
	{
		Sleep(10);
		timeout = 0;
		while(1)
		{
			nValue = (rd_flag << 31) + (0x5 << address_shift_value);
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
			Sleep(1);
			nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
			if(((nValue >> data_shift_value) & 0xFFFFFF) == nRegData)
				break;
			if(++timeout == 10)
			{
				LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0x5 timeout pos 1", nValue);
				break;
			}
		}
	}
	//REG 05h
	//nValue = 0xA000000;
	nRegData = 0;
	nValue = (0x5 << address_shift_value) + (nRegData << data_shift_value);
	LldPrint_1x("[HMC1033] Set 0x57300 Reg0x5", nValue);
	TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
	if(IsAttachedBdTyp_598())
	{
		Sleep(10);
		timeout = 0;
		while(1)
		{
			nValue = (rd_flag << 31) + (0x5 << address_shift_value);
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
			Sleep(1);
			nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
			if(((nValue >> data_shift_value) & 0xFFFFFF) == nRegData)
				break;
			if(++timeout == 10)
			{
				LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0x5 timeout pos 2", nValue);
				break;
			}
		}
	}
	
	//REG 03h Frequency Register - Integer Part
	nValue = (0x3 << address_shift_value) + (nInteger << data_shift_value);
	LldPrint_1x("[HMC1033] Set 0x57300 Reg0x3", nValue);
	TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
	if(IsAttachedBdTyp_598())
	{
		Sleep(10);
		timeout = 0;
		while(1)
		{
			nValue = (rd_flag << 31) + (0x3 << address_shift_value);
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
			Sleep(1);
			nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
			if(((nValue >> data_shift_value) & 0xFFFFFF) == nInteger)
				break;
			if(++timeout == 10)
			{
				LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0x3 timeout", nValue);
				break;
			}
		}
	}

	//REG 04h Frequency Register - Fractional Part
	nValue = (0x4 << address_shift_value) + (nFractional << data_shift_value);
	LldPrint_1x("[HMC1033] Set 0x57300 Reg0x4", nValue);
	TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
	if(IsAttachedBdTyp_598())
	{
		Sleep(10);
		timeout = 0;
		while(1)
		{
			nValue = (rd_flag << 31) + (0x4 << address_shift_value);
			TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, nValue);
			Sleep(1);
			nValue = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
			if(((nValue >> data_shift_value) & 0xFFFFFF) == nFractional)
				break;
			if(++timeout == 10)
			{
				LldPrint_1x("[Error HMC1033] Set 0x57300 Reg0x4 timeout", nValue);
				break;
			}
		}
	}
	return 0;
}
