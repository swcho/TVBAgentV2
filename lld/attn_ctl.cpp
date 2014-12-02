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
#include 	"attn_ctl.h"


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


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CAttnCtl::CAttnCtl(void)
{

}
CAttnCtl::~CAttnCtl()
{
}

//AGC
double CAttnCtl::CalcAGC_Atten(double* pRFLevel, double rf, int amp, double* pAGC_Level_Min, double* pAGC_Level_Max)
{
	int i, j;
	double agc_atten = 0, cur_rf_level = 5128;
	//FIXED
	int rf_at = 100;

	LldPrint_FCall("CalcAGC_Atten", amp, 0);
	for ( i = 0; pRFLevel[i] != 5128; i+=5 )
	{
		if ( rf > pRFLevel[i] && rf <= pRFLevel[i+1] )
		{
			if ( amp )
			{
				cur_rf_level = pRFLevel[i+3];
			}
			else
			{
				cur_rf_level = pRFLevel[i+2];
			}
		}
	}

	if ( cur_rf_level == 5128 )
	{
		LldPrint_Error("[LLD]===CalcAGC_Atten, Not supported RF range.", 0, 0);
		return agc_atten;
	}
	for ( i = 0; pRFLevel[i] != 5128; i+=5 )
	{
		for ( j = 0; pRFLevel[j] != 5128; j+=5 )
		{
			if ( pRFLevel[i+4] == 100 && pRFLevel[j+4] == 101 )
			{
				if ( rf > pRFLevel[i] && rf <= pRFLevel[j+1] )
				{
					if ( amp )
					{
						agc_atten = pRFLevel[j+3] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+3];
						*pAGC_Level_Min = pRFLevel[j+3];
					}
					else
					{
						agc_atten = pRFLevel[j+2] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+2];
						*pAGC_Level_Min = pRFLevel[j+2];
					}
					
					//FIXED
					rf_at = 100;
					
					break;
				}
			}
			else if ( pRFLevel[i+4] == 102 && pRFLevel[j+4] == 103 )
			{
				if ( rf > pRFLevel[i] && rf <= pRFLevel[j+1] )
				{
					if ( amp )
					{
						agc_atten = pRFLevel[j+3] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+3];
						*pAGC_Level_Min = pRFLevel[j+3];
					}
					else
					{
						agc_atten = pRFLevel[j+2] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+2];
						*pAGC_Level_Min = pRFLevel[j+2];
					}

					//FIXED
					rf_at = 102;
					
					break;
				}
			}
			else if ( pRFLevel[i+4] == 103 && pRFLevel[j+4] == 104 )
			{
				if ( rf > pRFLevel[i] && rf <= pRFLevel[j+1] )
				{
					if ( amp )
					{
						agc_atten = pRFLevel[j+3] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+3];
						*pAGC_Level_Min = pRFLevel[j+3];
					}
					else
					{
						agc_atten = pRFLevel[j+2] - cur_rf_level;

						*pAGC_Level_Max = pRFLevel[i+2];
						*pAGC_Level_Min = pRFLevel[j+2];
					}

					//FIXED
					rf_at = 103;
					
					break;
				}
			}
		}
	}

	LldPrint_ddi("[LLD]AGC, Min, Max at ", *pAGC_Level_Min, *pAGC_Level_Max, rf_at);
	j = -1;
	for ( i = 0; pRFLevel[i] != 5128; i+=5 )
	{
		if ( pRFLevel[i+4] == rf_at )
		{
			if ( amp )
			{
				*pAGC_Level_Max = pRFLevel[i+3];
				*pAGC_Level_Min = pRFLevel[i+3];
			}
			else
			{
				*pAGC_Level_Max = pRFLevel[i+2];
				*pAGC_Level_Min = pRFLevel[i+2];
			}
			j = 0;
		}
		else if ( j >= 0 )
		{
			if ( amp )
			{
				if ( *pAGC_Level_Min > pRFLevel[i+3] )
				{
					*pAGC_Level_Min = pRFLevel[i+3];
				}
				else if ( *pAGC_Level_Max < pRFLevel[i+3] )
				{
					*pAGC_Level_Max = pRFLevel[i+3];
				}
			}
			else
			{
				if ( *pAGC_Level_Min > pRFLevel[i+2] )
				{
					*pAGC_Level_Min = pRFLevel[i+2];
				}
				else if ( *pAGC_Level_Max < pRFLevel[i+2] )
				{
					*pAGC_Level_Max = pRFLevel[i+2];
				}
			}

			if ( pRFLevel[i+4] == rf_at+1 )
			{
				j = (int)(*pAGC_Level_Min);
				*pAGC_Level_Min = *pAGC_Level_Max;
				*pAGC_Level_Max = (double)j;
				
				//2011/7/7 fixed
				agc_atten = *pAGC_Level_Min - cur_rf_level;

				LldPrint_ddd("[LLD]AGC, Min, Max, Agc", *pAGC_Level_Min, *pAGC_Level_Max, agc_atten);
				break;
			}
		}
	}

	return agc_atten;
}

//AGC
double CAttnCtl::CalcRFLevel(long modulator_type, double rf, long agc_on_off, double* pAGC_Level_Min, double* pAGC_Level_Max, double* pAGC_Level_Offset)
{
	double dwAGC_Level_Offset = 0., dwAGC_Level_Max = 0., dwAGC_Level_Min = 0.;
	double RM0, RM1, RM2;
	int i;

	double dwTvb593_QamA_Offset = 0;
	//TVB593
	unsigned long dwStatus;
	
	LldPrint_FCall("CalcRFLevel", agc_on_off, 0);
	RM0 = MAX_RF_LEVEL;//TVB595V3, AMP ON, DVB-T, 473MHz

	if  ( IsAttachedBdTyp_UsbTyp_ExceptFor597v2() )
	{
		//V1, V2
		if  ( IsAttachedBdTyp_595v2() )
		{
			RM1 = RM0;
		}
		//V3
		else if  ( IsAttachedBdTyp_595v3() )
		{
			RM1 = RM0;
		}
		//V4
		else if  ( IsAttachedBdTyp_595v4() )
		{
			RM1 = RM0;
		}
		//TVB595D
		else if  ( IsAttachedBdTyp_597() )
		{
			RM1 = RM0;
		}
	}
	//TVB590/390, TVB593, TVB497
	else if ( IsAttachedBdTyp_WhyThisGrpNeed_4() )
	{
		//V8,...,V9.1
		if  ( IsAttachedBdTyp_390v8() )
		{
			RM1 = RM0;
		}
		else if  ( IsAttachedBdTyp_594() )
		{
			RM1 = -1.;
		}
		//V9.2,9.3,9.4
		else if  ( IsAttachedBdTyp_590v9() )
		{
			RM1 = -1.;
		}
		//V10
		else if  ( IsAttachedBdTyp_590v10() )
		{
			RM1 = -1.;//???
		}
		else if  ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			RM1 = RM0;
		}
		//TVB593, TVB497
		else if  ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
		{
			RM1 = RM0;
		}
	}
	//TVB380 V7, V8
	else if ( IsAttachedBdTyp_390v7() )
	{
		RM1 = RM0;
	}
	else
	{
		*pAGC_Level_Min = dwAGC_Level_Min;
		*pAGC_Level_Max = dwAGC_Level_Max;
		*pAGC_Level_Offset = dwAGC_Level_Offset;
		return 0;
	}

	if ( IsAttachedBdTyp_AttenTyp_2() )
	{
		//TVB595D - 597A
		if ( IsAttachedBdTyp_597() )
		{
			RM1 = RM0;
		}
		//TVB595B
		else if ( IsAttachedBdTyp_595v3() )
		{
			//S/S2 0dBm
			if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
			{
				RM1 = RM0;
			}
		}
		//TVB595C
		else if ( IsAttachedBdTyp_595v4() )
		{
			//Active Mixer
			if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
			{
				RM1 = RM0;
			}

			//S/S2 0dBm
			if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
			{
				RM1 = RM0;
			}
		}
		//TVB590B, TVB590C
		else if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			//Active Mixer
			if ( (TSPL_nAuthorization & 0x01) == 1 )
			{
				RM1 = RM0;
			}

			//S/S2 0dBm
			if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
			{
				RM1 = RM0;
			}
		}
		//TVB590S
		else if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			RM1 = RM0;
		}
		//TVB593, TVB497
		else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
		{
			RM1 = RM0;
		}
	}

	//TVB590V9.2, V10.0, TVB595V1.1 or higher, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAgcFreqTbl() )
	{
		if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1() )
		{
			switch (modulator_type)
			{
				case TVB380_TDMB_MODE:
					gRFLevel = &gLevel_TDMB_20_V3[0];
					gRFLevel_AD9775 = &gLevel_TDMB_20_V3_AD9775[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_DVB_S_S2_20_V3[0];
					gRFLevel_AD9775 = &gLevel_DVB_S_S2_20_V3_AD9775[0];
					break;

				case TVB380_QAMB_MODE:
					if ( gQAMB_Constllation == 0 )
					{
						gRFLevel = &gLevel_QAM_64B_20_V3[0];
						gRFLevel_AD9775 = &gLevel_QAM_64B_20_V3_AD9775[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_256B_20_V3[0];
						gRFLevel_AD9775 = &gLevel_QAM_256B_20_V3_AD9775[0];
					}
					break;

				case TVB380_QAMA_MODE:
					if ( gQAMA_Constllation == 0 || gQAMA_Constllation == 2 || gQAMA_Constllation == 4 )
					{
						gRFLevel = &gLevel_QAM_16_64_256A_20_V3[0];
						gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_20_V3_AD9775[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_32_128A_20_V3[0];
						gRFLevel_AD9775 = &gLevel_QAM_32_128A_20_V3_AD9775[0];

					}
					break;

				case TVB380_VSB8_MODE:
					gRFLevel = &gLevel_8VSB_20_V3[0];
					gRFLevel_AD9775 = &gLevel_8VSB_20_V3_AD9775[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
					gRFLevel = &gLevel_DVB_TH_20_V3[0];
					gRFLevel_AD9775 = &gLevel_DVB_TH_20_V3_AD9775[0];
					break;

				case TVB380_ISDBS_MODE:
					gRFLevel = &gLevel_ISDB_S_20_V3[0];
					gRFLevel_AD9775 = &gLevel_ISDB_S_20_V3_AD9775[0];
					break;
				
				default: 
					gRFLevel = &gLevel_DVB_TH_20_V3[0];
					gRFLevel_AD9775 = &gLevel_DVB_TH_20_V3_AD9775[0];
					break;
			}

			//AD9775 - REG : 0x6, 0xA 
			for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=4 )
			{
				if ( rf > gRFLevel_AD9775[i] && rf <= gRFLevel_AD9775[i+1] )
				{
					TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
					TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

					LldPrint_ddd("[LLD]AD9775 :: RF ", rf, gRFLevel_AD9775[i+2], gRFLevel_AD9775[i+3]);

					break;
				}
			}
		}
		else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
		{
			//FIXED - TVB593 V2, TVB497
			//2011/3/11 FIXED
			if(IsAttachedBdTyp_591())
			{
				switch (modulator_type)
				{
					case TVB380_VSB8_MODE:
						gRFLevel = &gLevel_TVB591_VSB[0];
						break;

					case TVB380_TDMB_MODE:
					case TVB380_QAMA_MODE:
					case TVB380_QAMB_MODE:
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					case TVB380_IQ_PLAY_MODE:
						gRFLevel = &gLevel_TVB591_QAM_A_B_TDMB_DVB_TH[0];
						break;
					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_TVB591_ISDB_T_1[0];
						break;
					case TVB380_ISDBT_13_MODE:
						gRFLevel = &gLevel_TVB591_ISDB_T_13[0];
						break;
					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB591_DTMB[0];
						break;
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB591_ATSC_MH[0];
						break;
				}

			}
			else if ( IsAttachedBdTyp_497() )
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_ATSC_MH_593_V2[0];
						gRFLevel_AD9775 = &gLevel_ATSC_MH_593_V2_AD9775[0];
						break;

					case TVB380_DVBT2_MODE:
						gRFLevel = &gLevel_DVB_T2_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_T2_593_V2_AD9775[0];
						break;
					//2011/6/3
					case	TVB380_DVBC2_MODE:
						gRFLevel = &gLevel_DVB_C2_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_T2_593_V2_AD9775[0];
						break;

					case TVB380_CMMB_MODE:
						gRFLevel = &gLevel_CMMB_593_V2[0];
						gRFLevel_AD9775 = &gLevel_CMMB_593_V2_AD9775[0];
						break;

					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_DTMB_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DTMB_593_V2_AD9775[0];
						break;

					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TDMB_593_V2[0];
						gRFLevel_AD9775 = &gLevel_TDMB_593_V2_AD9775[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_DVB_S_S2_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_S_S2_593_V2_AD9775[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_MULTI_QAMB_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 )
						{
							gRFLevel = &gLevel_QAM_64B_593_V2[0];
							gRFLevel_AD9775 = &gLevel_QAM_64B_593_V2_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_QAM_256B_593_V2[0];
							gRFLevel_AD9775 = &gLevel_QAM_256B_593_V2_AD9775[0];
						}
						break;

					case TVB380_QAMA_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
						{
							gRFLevel = &gLevel_QAM_16_64_256A_593_V2[0];
							gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_593_V2_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_QAM_32_128A_593_V2[0];
							gRFLevel_AD9775 = &gLevel_QAM_32_128A_593_V2_AD9775[0];
						}
						break;

					case TVB380_VSB8_MODE:
					case TVB380_MULTI_VSB_MODE:
						gRFLevel = &gLevel_8VSB_593_V2[0];
						gRFLevel_AD9775 = &gLevel_8VSB_593_V2_AD9775[0];
						break;
					
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					//2012/6/27 Multi DVB-T
					case TVB380_MULTI_DVBT_MODE:
						gRFLevel = &gLevel_DVB_TH_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_V2_AD9775[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_ISDB_T_593_V2[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_593_V2_AD9775[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
						gRFLevel = &gLevel_ISDB_T_13_593_V2[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_13_593_V2_AD9775[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_ISDB_S_593_V2[0];
						gRFLevel_AD9775 = &gLevel_ISDB_S_593_V2_AD9775[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_DVB_TH_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_V2_AD9775[0];
						break;
				}

				//FIXED - TVB593 V2.1
				if ( TSPL_nBoardRevision >= __REV_2_1__of_bd_593v2__ || IsAttachedBdTyp_597v2() )
				{
					gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
				}
				
				//AD9775 - REG : 0x6, 0xA, 0x5, 0x9
				for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
				{
					if ( rf > gRFLevel_AD9775[i] && rf <= gRFLevel_AD9775[i+1] )
					{
						TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
						TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
						TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
						TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

						LldPrint_5("[LLD]AD9775 :: RF", (int)rf, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

						break;
					}
				}
			}
			//2012/2/7 TVB591S 
			else if(IsAttachedBdTyp_591S())
			{
				//2012/7/23 TVB591S V2
				if(IsAttachedBdTyp_591S_V2())
				{
					switch (modulator_type)
					{
						case TVB380_ATSC_MH_MODE:
							gRFLevel = &gLevel_TVB591S_V2_ATSC_MH[0];
							break;
						case TVB380_ISDBT_MODE:
						case TVB380_TDMB_MODE:
							gRFLevel = &gLevel_TVB591S_V2_TDMB_ISDB_T_1[0];
							break;

						case TVB380_QPSK_MODE:
						case TVB380_DVBS2_MODE:
							gRFLevel = &gLevel_TVB591S_V2_DVB_S_S2[0];
							break;

						case TVB380_QAMB_MODE:
						case TVB380_QAMA_MODE:
						case TVB380_MULTI_QAMB_MODE:
							gRFLevel = &gLevel_TVB591S_V2_QAM_A_B[0];
							break;

						case TVB380_VSB8_MODE:
						case TVB380_DVBT_MODE:
						case TVB380_DVBH_MODE:
						case TVB380_MULTI_VSB_MODE:
							gRFLevel = &gLevel_TVB591S_V2_DVB_T_H_8VSB[0];
							break;
						
						case TVB380_ISDBT_13_MODE:
							gRFLevel = &gLevel_TVB591S_V2_ISDB_T_13[0];
							break;
						case TVB380_DTMB_MODE:
							gRFLevel = &gLevel_TVB591S_V2_DTMB[0];
							break;

						case TVB380_ISDBS_MODE:
							gRFLevel = &gLevel_TVB591S_V2_ISDB_S[0];
							break;

						default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
							gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
							break;
					}
				}
				else
				{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ATSC_MH[0];
						break;
					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_TDMB[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_TVB591S_V1_DVB_S_S2[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_QAMA_MODE:
					case TVB380_MULTI_QAMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_QAM_A_B[0];
						break;

					case TVB380_VSB8_MODE:
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					case TVB380_MULTI_VSB_MODE:
					//2012/6/27 Multi DVB-T
					case TVB380_MULTI_DVBT_MODE:

						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_1[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_13_DTMB[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_S[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;
				}
				}
				gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
				
				//AD9775 - REG : 0x6, 0xA, 0x5, 0x9
				for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
				{
					if ( rf > gRFLevel_AD9775[i] && rf <= gRFLevel_AD9775[i+1] )
					{
						TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
						TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
						TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
						TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

						LldPrint_5("[LLD]AD9775 :: RF", (int)rf, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

						break;
					}
				}
			}
			else if(TSPL_nBoardRevision >= __REV_2_0__of_bd_593__ || IsAttachedBdTyp_597v2())
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB597A_V2_ATSC_MH[0];
						gRFLevel_AD9775 = &gLevel_ATSC_MH_593_V2_AD9775[0];
						break;

					case TVB380_DVBT2_MODE:
						gRFLevel = &gLevel_TVB597A_V2_DVB_T2[0];
						gRFLevel_AD9775 = &gLevel_DVB_T2_593_V2_AD9775[0];
						break;
					//2011/6/3
					case TVB380_DVBC2_MODE:
						gRFLevel = &gLevel_TVB597A_V2_DVB_C2[0];
						gRFLevel_AD9775 = &gLevel_DVB_T2_593_V2_AD9775[0];
						break;

					case TVB380_CMMB_MODE:
						gRFLevel = &gLevel_TVB597A_V2_CMMB[0];
						gRFLevel_AD9775 = &gLevel_CMMB_593_V2_AD9775[0];
						break;

					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
						gRFLevel_AD9775 = &gLevel_DTMB_593_V2_AD9775[0];
						break;

					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TVB597A_V2_TDMB[0];
						gRFLevel_AD9775 = &gLevel_TDMB_593_V2_AD9775[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_TVB597A_V2_DVB_S_S2[0];
						gRFLevel_AD9775 = &gLevel_DVB_S_S2_593_V2_AD9775[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_MULTI_QAMB_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 )
						{
							gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
							gRFLevel_AD9775 = &gLevel_QAM_64B_593_V2_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
							gRFLevel_AD9775 = &gLevel_QAM_256B_593_V2_AD9775[0];
						}
						break;

					case TVB380_QAMA_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
						{
							gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
							gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_593_V2_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
							gRFLevel_AD9775 = &gLevel_QAM_32_128A_593_V2_AD9775[0];
						}
						break;

					case TVB380_VSB8_MODE:
					case TVB380_MULTI_VSB_MODE:
						gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
						gRFLevel_AD9775 = &gLevel_8VSB_593_V2_AD9775[0];
						break;
					
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					//2012/6/27 multi dvb-t
					case TVB380_MULTI_DVBT_MODE:
						gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_V2_AD9775[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_TVB597A_V2_ISDB_T_1[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_593_V2_AD9775[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
						gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_13_593_V2_AD9775[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_TVB597A_V2_ISDB_S[0];
						gRFLevel_AD9775 = &gLevel_ISDB_S_593_V2_AD9775[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_DVB_TH_593_V2[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_V2_AD9775[0];
						break;
				}

				//FIXED - TVB593 V2.1
				if ( TSPL_nBoardRevision >= __REV_2_1__of_bd_593v2__ || IsAttachedBdTyp_597v2())
				{
					
					gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
				}
				
				//AD9775 - REG : 0x6, 0xA, 0x5, 0x9
				for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
				{
					if ( rf > gRFLevel_AD9775[i] && rf <= gRFLevel_AD9775[i+1] )
					{
						if(modulator_type == TVB380_QAMA_MODE && i == 6)
						{
							if(IsAttachedBdTyp_593() && (TSPL_nBoardRevision >= __REV_2_1__of_bd_593v2__ && 
									TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3))
							{
								TSPL_SET_AD9775(0x06, (long)(gRFLevel_AD9775[i+2] - 1));
								TSPL_SET_AD9775(0x0A, (long)(gRFLevel_AD9775[i+3] - 1));
								dwTvb593_QamA_Offset = -3;
							}
							else
							{
								TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
								TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
							}
						}
						else	
						{
							TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
							TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
						}
						TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
						TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

						LldPrint_5("[LLD]AD9775 :: RF", (int)rf, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

						break;
					}
				}
			}
			else
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_ATSC_MH_593[0];
						gRFLevel_AD9775 = &gLevel_ATSC_MH_593_AD9775[0];
						break;

					case TVB380_DVBT2_MODE:
						gRFLevel = &gLevel_DVB_T2_593[0];
						gRFLevel_AD9775 = &gLevel_DVB_T2_593_AD9775[0];
						break;
					//2011/6/3
					case	TVB380_DVBC2_MODE:
						gRFLevel = &gLevel_DVB_C2_593[0];
						gRFLevel_AD9775 = &gLevel_DVB_C2_593_AD9775[0];
						break;

					case TVB380_CMMB_MODE:
						gRFLevel = &gLevel_CMMB_593[0];
						gRFLevel_AD9775 = &gLevel_CMMB_593_AD9775[0];
						break;

					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_DTMB_593[0];
						gRFLevel_AD9775 = &gLevel_DTMB_593_AD9775[0];
						break;

					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TDMB_593[0];
						gRFLevel_AD9775 = &gLevel_TDMB_593_AD9775[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_DVB_S_S2_593[0];
						gRFLevel_AD9775 = &gLevel_DVB_S_S2_593_AD9775[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_MULTI_QAMB_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 )
						{
							gRFLevel = &gLevel_QAM_64B_593[0];
							gRFLevel_AD9775 = &gLevel_QAM_64B_593_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_QAM_256B_593[0];
							gRFLevel_AD9775 = &gLevel_QAM_256B_593_AD9775[0];
						}
						break;

					case TVB380_QAMA_MODE:
						dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
						dwStatus = (dwStatus>>2)&0x01;
						if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
						{
							gRFLevel = &gLevel_QAM_16_64_256A_593[0];
							gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_593_AD9775[0];
						}
						else
						{
							gRFLevel = &gLevel_QAM_32_128A_593[0];
							gRFLevel_AD9775 = &gLevel_QAM_32_128A_593_AD9775[0];
						}
						break;

					case TVB380_VSB8_MODE:
					case TVB380_MULTI_VSB_MODE:
						gRFLevel = &gLevel_8VSB_593[0];
						gRFLevel_AD9775 = &gLevel_8VSB_593_AD9775[0];
						break;
					
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					//2012/6/27 multi dvb-t
					case TVB380_MULTI_DVBT_MODE:
						gRFLevel = &gLevel_DVB_TH_593[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_AD9775[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_ISDB_T_593[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_593_AD9775[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
						gRFLevel = &gLevel_ISDB_T_13_593[0];
						gRFLevel_AD9775 = &gLevel_ISDB_T_13_593_AD9775[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_DVB_TH_593[0];
						gRFLevel_AD9775 = &gLevel_DVB_TH_593_AD9775[0];
						break;
				}

				//AD9775 - REG : 0x6, 0xA 
				for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=4 )
				{
					if ( rf > gRFLevel_AD9775[i] && rf <= gRFLevel_AD9775[i+1] )
					{
						TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
						TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

						LldPrint_ddd("[LLD]AD9775 :: RF", rf, gRFLevel_AD9775[i+2], gRFLevel_AD9775[i+3]);

						break;
					}
				}
			}
		}
		else
		{
			switch (modulator_type)
			{
				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_DVB_S2[0];
					
					if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp() )
					{
						//TVB595D - 597A
						if ( IsAttachedBdTyp_597_or_597v2() )
						{
							gRFLevel = &gLevel_DVB_S2_Active[0];
						}
						//TVB595B
						else if ( IsAttachedBdTyp_595v3() )
						{
							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_S2_0dBm[0];

								if ( gChannel_Freq_Offset == IF_OUT_44MHZ )
								{
									gRFLevel = &gLevel_DVB_S2_0dBm_IF44[0];
								}
							}
						}
						//TVB595C
						else if ( IsAttachedBdTyp_595v4() )
						{
							//Active Mixer
							if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
							{
								gRFLevel = &gLevel_DVB_S2_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_S2_0dBm[0];

								if ( gChannel_Freq_Offset == IF_OUT_44MHZ )
								{
									gRFLevel = &gLevel_DVB_S2_0dBm_IF44[0];
								}
							}
						}
						//TVB590B, TVB590C
						else if ( IsAttachedBdTyp_590v10() )
						{
							//Active Mixer
							if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
							{
								gRFLevel = &gLevel_DVB_S2_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_S2_0dBm[0];

								if ( gChannel_Freq_Offset == IF_OUT_44MHZ )
								{
									gRFLevel = &gLevel_DVB_S2_0dBm_IF44[0];
								}
							}
						}
						//TVB590S
						else if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
						{
							//TVB590S V2
							if ( TSPL_nBoardRevision >= __REV_2_0__of_bd_590s__ )
							{
								gRFLevel = &gLevel_DVB_S2_20_V2[0];
							}
							else
							{
								gRFLevel = &gLevel_DVB_S2_20[0];
							}
						}
					}
					else if ( IsAttachedBdTyp_390v8() )
					{
						gRFLevel = &gLevel_DVB_S2_45[0];
					}
					else if ( IsAttachedBdTyp_390v7() )
					{
						gRFLevel = &gLevel_DVB_S2_44[0];
					}

					break;

				case TVB380_QAMA_MODE:
				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					gRFLevel = &gLevel_QAM_A[0];

					if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp() )
					{
						//TVB595D - 597A
						if ( IsAttachedBdTyp_597_or_597v2() )
						{
							gRFLevel = &gLevel_QAM_A_Active[0];
						}
						//TVB595B
						else if ( IsAttachedBdTyp_595v3() )
						{
							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_QAM_A_0dBm[0];
							}
						}
						//TVB595C
						else if ( IsAttachedBdTyp_595v4() )
						{
							//Active Mixer
							if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
							{
								gRFLevel = &gLevel_QAM_A_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_QAM_A_0dBm[0];
							}
						}
						//TVB590B, TVB590C
						else if ( IsAttachedBdTyp_590v10() )
						{
							//Active Mixer
							if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
							{
								gRFLevel = &gLevel_QAM_A_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_QAM_A_0dBm[0];
							}
						}
						//TVB590S
						else if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
						{
							//TVB590S V2
							if ( TSPL_nBoardRevision >= __REV_2_0__of_bd_590s__ )
							{
								gRFLevel = &gLevel_QAM_A_20_V2[0];
							}
							else
							{
								gRFLevel = &gLevel_QAM_A_20[0];
							}
						}
					}
					else if ( IsAttachedBdTyp_390v8() )
					{
						gRFLevel = &gLevel_QAM_A_45[0];
					}
					else if ( IsAttachedBdTyp_390v7() )
					{
						if ( modulator_type == TVB380_QAMA_MODE )
						{
							gRFLevel = &gLevel_QAM_A_44[0];
						}
						else
						{
							gRFLevel = &gLevel_QAM_B_44[0];
						}
					}

					break;

				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_DTMB[0];

					if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp() )
					{
						//TVB595D - 597A
						if ( IsAttachedBdTyp_597_or_597v2() )
						{
							gRFLevel = &gLevel_DTMB_Active[0];
						}
						//TVB595B
						else if ( IsAttachedBdTyp_595v3() )
						{
							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DTMB_0dBm[0];
							}
						}
						//TVB595C
						else if ( IsAttachedBdTyp_595v4() )
						{
							//Active Mixer
							if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
							{
								gRFLevel = &gLevel_DTMB_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DTMB_0dBm[0];
							}
						}
						//TVB590B, TVB590C, TVB590S
						else if ( IsAttachedBdTyp_590v10_or_590s() )
						{
							//Active Mixer
							if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
							{
								gRFLevel = &gLevel_DTMB_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DTMB_0dBm[0];
							}
						}
					}
					else if ( IsAttachedBdTyp_390v8() )
					{
						gRFLevel = &gLevel_DTMB_45[0];
					}

					break;
					
				default:
					
					gRFLevel = &gLevel_DVB_T[0];

					if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp() )
					{
						//TVB595D - 597A
						if ( IsAttachedBdTyp_597() )
						{
							//TVB593
							if ( (modulator_type == TVB380_DVBT2_MODE ) || (modulator_type == TVB380_DVBC2_MODE))
							{
								if(modulator_type == TVB380_DVBT2_MODE )
									gRFLevel = &gLevel_DVB_T2_Active[0];
								else
									gRFLevel = &gLevel_DVB_C2_Active[0];
							}
							else
					
								gRFLevel = &gLevel_DVB_T_Active[0];
						}
						//TVB595B
						else if ( IsAttachedBdTyp_595v3() )
						{
							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_T_0dBm[0];
							}
						}
						//TVB595C
						else if ( IsAttachedBdTyp_595v4() )
						{
							//Active Mixer
							if ( ((TSPL_nAuthorization>>_bits_sht_act_mixer_bd_grp2_) & 0x07) == 1 )
							{
								gRFLevel = &gLevel_DVB_T_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp2_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_T_0dBm[0];
							}
						}
						//TVB590B, TVB590C
						else if ( IsAttachedBdTyp_590v10() )
						{
							//Active Mixer
							if ( (TSPL_nAuthorization & _bits_mask_act_mixer_bd_grp1_) == 1 )
							{
								gRFLevel = &gLevel_DVB_T_Active[0];
							}

							//S/S2 0dBm
							if ( ((TSPL_nAuthorization>>_bits_sht_s_s2_0_dBm_bd_grp1_) & 0x01) == 1 )
							{
								gRFLevel = &gLevel_DVB_T_0dBm[0];
							}
						}
						//TVB590S
						else if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
						{
							//TVB590S V2
							if ( TSPL_nBoardRevision >= __REV_2_0__of_bd_590s__ )
							{
								gRFLevel = &gLevel_DVB_T_20_V2[0];
							}
							else
							{
								gRFLevel = &gLevel_DVB_T_20[0];
							}
						}
					}
					else if ( IsAttachedBdTyp_390v8() )
					{
						gRFLevel = &gLevel_DVB_T_45[0];
					}
					else if ( IsAttachedBdTyp_390v7() )
					{
						if ( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_DVBH_MODE )
						{
							gRFLevel = &gLevel_DVB_T_44[0];
						}
						//ATSC-M/H
						else if ( modulator_type == TVB380_VSB8_MODE || modulator_type == TVB380_ATSC_MH_MODE )
						{
							gRFLevel = &gLevel_8VSB_44[0];
						}
						else if ( modulator_type == TVB380_TDMB_MODE )
						{
							gRFLevel = &gLevel_TDMB_44[0];
						}
					}

					break;
			}

		}

		RM2 = RM1;

		//AGC - i+=4 -> i+=5
		for ( i = 0; gRFLevel[i] != 5128; i+=5 )
		{
			if ( rf > gRFLevel[i] && rf <= gRFLevel[i+1] )
			{
				//AMP ON
				if ( gBypass_AMP == 2 )
				{
					RM2 = RM1 - gRFLevel[i+3];

					//AGC
					dwAGC_Level_Offset = CalcAGC_Atten(gRFLevel, gCenter_Freq, 1, &dwAGC_Level_Min, &dwAGC_Level_Max);
				}
				//AMP OFF
				else
				{
					RM2 = RM1 - gRFLevel[i+2];

					//AGC
					dwAGC_Level_Offset = CalcAGC_Atten(gRFLevel, gCenter_Freq, 0, &dwAGC_Level_Min, &dwAGC_Level_Max);
				}

				if  ( IsAttachedBdTyp_390v8() )
				{
					//no amp.
					if ( TSPL_nBoardUseAMP == 0 )
					{
						RM2 = RM1 - gRFLevel[i+2];

						//AGC
						dwAGC_Level_Offset = CalcAGC_Atten(gRFLevel, gCenter_Freq, 0, &dwAGC_Level_Min, &dwAGC_Level_Max);
					}
					//24dBm amp.
					else
					{
						RM2 = RM1 - gRFLevel[i+3];

						//AGC
						dwAGC_Level_Offset = CalcAGC_Atten(gRFLevel, gCenter_Freq, 1, &dwAGC_Level_Min, &dwAGC_Level_Max);
					}
				}
				else if  ( IsAttachedBdTyp_390v7() )
				{
					RM2 = RM1 - gRFLevel[i+2];

					//AGC
					dwAGC_Level_Offset = CalcAGC_Atten(gRFLevel, gCenter_Freq, 0, &dwAGC_Level_Min, &dwAGC_Level_Max);
				}

				break;
			}
		}
	}
	else
	{
		RM2 = RM1;
	}

	if ( agc_on_off == 0 )
	{
		dwAGC_Level_Offset = 0;
		dwAGC_Level_Min = 0;
		dwAGC_Level_Max = MAX_ATTEN;
	}

	*pAGC_Level_Min = (dwAGC_Level_Min + dwTvb593_QamA_Offset);
	*pAGC_Level_Max = (dwAGC_Level_Max + dwTvb593_QamA_Offset);
	*pAGC_Level_Offset = dwAGC_Level_Offset;

	return (RM2 + dwTvb593_QamA_Offset);
}

double CAttnCtl::GetTAT4710_Offset()
{
	if(IsAttachedBdTyp_599())
	{
		if(gCenter_Freq <= 40000000)
			return 1;
		else if(gCenter_Freq <= 200000000)
			return 1.5;
		else if(gCenter_Freq <= 250000000)
			return 2;
		else if(gCenter_Freq <= 370000000)
			return 2.5;
		else if(gCenter_Freq <= 400000000)
			return 3;
		else if(gCenter_Freq <= 500000000)
			return 3.5;
		else if(gCenter_Freq <= 600000000)
			return 4.5;
		else if(gCenter_Freq <= 700000000)
			return 4;
		else if(gCenter_Freq <= 800000000)
			return 4.5;
		else if(gCenter_Freq <= 900000000)
			return 3.5;
		else if(gCenter_Freq <= 1050000000)
			return 5;
		else if(gCenter_Freq <= 1060000000)
			return 6;
		else if(gCenter_Freq <= 1200000000)
			return 5.5;
		else if(gCenter_Freq <= 1300000000)
			return 5;
		else if(gCenter_Freq <= 1400000000)
			return 5.5;
		else if(gCenter_Freq <= 1500000000)
			return 6;
		else if(gCenter_Freq <= 1600000000)
			return 7;
		else if(gCenter_Freq <= 2000000000)
			return 8;
		else if(gCenter_Freq <= 2100000000)
			return 7.5;
		else
			return 9.5;
	}
	else if(IsAttachedBdTyp_598())
	{
		if(gCenter_Freq <= 40000000)
			return 1;
		else if(gCenter_Freq <= 129000000)
			return 1.5;
		else if(gCenter_Freq <= 150000000)
			return 1;
		else if(gCenter_Freq <= 200000000)
			return 2;
		else if(gCenter_Freq <= 220000000)
			return 2.5;
		else if(gCenter_Freq <= 235000000)
			return 2;
		else if(gCenter_Freq <= 370000000)
			return 3;
		else if(gCenter_Freq <= 400000000)
			return 3.5;
		else if(gCenter_Freq <= 500000000)
			return 4.5;
		else if(gCenter_Freq <= 600000000)
			return 6;
		else if(gCenter_Freq <= 700000000)
			return 3.5;
		else if(gCenter_Freq <= 900000000)
			return 5.5;
		else if(gCenter_Freq <= 1000000000)
			return 5;
		else if(gCenter_Freq <= 1050000000)
			return 4.5;
		else if(gCenter_Freq <= 1060000000)
			return 5;
		else if(gCenter_Freq <= 1080000000)
			return 5.5;
		else if(gCenter_Freq <= 1300000000)
			return 5;
		else if(gCenter_Freq <= 1600000000)
			return 6;
		else if(gCenter_Freq <= 1800000000)
			return 6.5;
		else if(gCenter_Freq <= 1900000000)
			return 8;
		else if(gCenter_Freq <= 2000000000)
			return 7.5;
		else if(gCenter_Freq <= 2100000000)
			return 9.5;
		else
			return 11.5;
	}
	else if(IsAttachedBdTyp_593())
	{
		if(gCenter_Freq <= 150000000)
			return 1.5;
		else if(gCenter_Freq <= 200000000)
			return 2;
		else if(gCenter_Freq <= 235000000)
			return 2.5;
		else if(gCenter_Freq <= 370000000)
			return 3;
		else if(gCenter_Freq <= 400000000)
			return 3.5;
		else if(gCenter_Freq <= 500000000)
			return 4.5;
		else if(gCenter_Freq <= 600000000)
			return 5;
		else if(gCenter_Freq <= 700000000)
			return 4;
		else if(gCenter_Freq <= 900000000)
			return 5.5;
		else if(gCenter_Freq <= 1000000000)
			return 4.5;
		else if(gCenter_Freq <= 1050000000)
			return 4;
		else if(gCenter_Freq <= 1060000000)
			return 5;
		else if(gCenter_Freq <= 1080000000)
			return 4.5;
		else if(gCenter_Freq <= 1200000000)
			return 5;
		else if(gCenter_Freq <= 1300000000)
			return 5.5;
		else if(gCenter_Freq <= 1900000000)
			return 5;
		else if(gCenter_Freq <= 2000000000)
			return 6.5;
		else if(gCenter_Freq <= 2100000000)
			return 8.5;
		else
			return 6.5;
	}
}

#if 0
#else
int	CAttnCtl::GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *rf_level_min, double *rf_level_max)
{
	int i;

	//TVB593
	unsigned long dwStatus;
	double rfLevel_Modulator_Gap = 0;
	double dwQamA_level_gap = 0;
	unsigned long multiModeFreqGap = 0;
	if(modulator_type == TVB380_TSIO_MODE)
		return 1;

	if((IsAttachedBdTyp_593() || IsAttachedBdTyp_591S() || IsAttachedBdTyp_598()) && (modulator_type == TVB380_MULTI_VSB_MODE || modulator_type == TVB380_MULTI_QAMB_MODE || modulator_type == TVB380_MULTI_DVBT_MODE))
	{
		if(IsAttachedBdTyp_593() && modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(gSymbol_Rate == DVB_T_6M_BAND)
				multiModeFreqGap = 3000000;
			else if(gSymbol_Rate == DVB_T_7M_BAND)
				multiModeFreqGap = 3500000;
			else
				multiModeFreqGap = 4000000;
		}
		else if(IsAttachedBdTyp_598() && modulator_type == TVB380_MULTI_DVBT_MODE)
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
	LldPrint_FCall("GET_MODULATOR_RF_LEVEL_RANGE", modulator_type, 0);

	if(gSingleTone == 1 && IsAttachedBdTyp_597A_V4())
	{
		gRFLevel = &gLevel_TVB597A_V4_SINGLETONE[0];
	}
	else if(gSingleTone == 1 && (IsAttachedBdTyp_599() || IsAttachedBdTyp_598()))
	{
		gRFLevel = &gLevel_TVB599A_V1_SINGLETONE[0];
	}
	//else if(gSingleTone == 1 && IsAttachedBdTyp_598())
	//{
	//	gRFLevel = &gLevel_TVB598_V1_SINGLETONE[0];
	//}
	else if(gSingleTone == 1 && IsAttachedBdTyp_TVB593_597A_V3())
	{
		if(IsAttachedBdTyp_597v2())
		{
			gRFLevel = &gLevel_TVB597A_V3_SINGLETONE[0];
		}
		else
		{
			gRFLevel = &gLevel_TVB593_V3_SINGLETONE[0];
		}
	}
	else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1() )
	{
		switch (modulator_type)
		{
			case TVB380_TDMB_MODE:
				gRFLevel = &gLevel_TDMB_20_V3[0];
				break;

			case TVB380_QPSK_MODE:
			case TVB380_DVBS2_MODE:
				gRFLevel = &gLevel_DVB_S_S2_20_V3[0];
				break;

			case TVB380_QAMB_MODE:
				if ( gQAMB_Constllation == 0 )
				{
					gRFLevel = &gLevel_QAM_64B_20_V3[0];
				}
				else
				{
					gRFLevel = &gLevel_QAM_256B_20_V3[0];
				}
				break;

			case TVB380_QAMA_MODE:
				if ( gQAMA_Constllation == 0 || gQAMA_Constllation == 2 || gQAMA_Constllation == 4 )
				{
					gRFLevel = &gLevel_QAM_16_64_256A_20_V3[0];
				}
				else
				{
					gRFLevel = &gLevel_QAM_32_128A_20_V3[0];
				}
				break;

			case TVB380_VSB8_MODE:
				gRFLevel = &gLevel_8VSB_20_V3[0];
				break;
			
			case TVB380_DVBT_MODE:
			case TVB380_DVBH_MODE:
				gRFLevel = &gLevel_DVB_TH_20_V3[0];
				break;

			case TVB380_ISDBS_MODE:
				gRFLevel = &gLevel_ISDB_S_20_V3[0];
				break;
			
			default: 
				gRFLevel = &gLevel_DVB_TH_20_V3[0];
				break;
		}
	}
	else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
	{
		//FIXED - TVB593 V2, TVB497
		//2011/3/11 FIXED
		if(IsAttachedBdTyp_591())
		{
			switch (modulator_type)
			{
				case TVB380_VSB8_MODE:
					gRFLevel = &gLevel_TVB591_VSB[0];
					break;

				case TVB380_TDMB_MODE:
				case TVB380_QAMA_MODE:
				case TVB380_QAMB_MODE:
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				case TVB380_IQ_PLAY_MODE:
					gRFLevel = &gLevel_TVB591_QAM_A_B_TDMB_DVB_TH[0];
					break;
				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_TVB591_ISDB_T_1[0];
					break;
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_TVB591_ISDB_T_13[0];
					break;
				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_TVB591_DTMB[0];
					break;
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_TVB591_ATSC_MH[0];
					break;
			}

		}
		//2012/2/7 TVB591S 
		else if(IsAttachedBdTyp_591S())
		{
			//2012/7/23 TVB591S V2
			if(IsAttachedBdTyp_591S_V2())
			{
				gRFLevel = &gLevel_TVB591S_V2_R2_BASE[0];
				rfLevel_Modulator_Gap = gLevel_TVB591S_V2_R2_Modulator_GAP[modulator_type];	
			}
			else
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ATSC_MH[0];
						break;
					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_TDMB[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_TVB591S_V1_DVB_S_S2[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_QAMA_MODE:
					case TVB380_MULTI_QAMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_QAM_A_B[0];
						break;

					case TVB380_VSB8_MODE:
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					case TVB380_MULTI_VSB_MODE:
					//2012/6/27 Multi DVB-T
					case TVB380_MULTI_DVBT_MODE:

						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_1[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_13_DTMB[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_S[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;
				}
			}
		}
		else if(IsAttachedBdTyp_597A_V4())
		{
			gRFLevel = &gLevel_TVB597A_V4_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB597A_V4_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_599()/* || IsAttachedBdTyp_598()*/)
		{
			gRFLevel = &gLevel_TVB599A_V1_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB599A_V1_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_598())
		{
			gRFLevel = &gLevel_TVB598_V1_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB598_V1_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_TVB593_597A_V3())
		{
			if(IsAttachedBdTyp_597v2())
			{
				gRFLevel = &gLevel_TVB597A_V3_BASE[0];
				rfLevel_Modulator_Gap = gLevel_TVB597A_V3_Modulator_GAP[modulator_type];	
			}
			else
			{
				if(IsAttachedBdTyp_TVB593_V4())
				{
					gRFLevel = &gLevel_TVB593_V4_BASE[0];
					rfLevel_Modulator_Gap = gLevel_TVB593_V4_Modulator_GAP[modulator_type];	
				}
				else
				{
					gRFLevel = &gLevel_TVB593_V3_BASE[0];
					rfLevel_Modulator_Gap = gLevel_TVB593_V3_Modulator_GAP[modulator_type];	
				}
			}
		}
		else if(IsAttachedBdTyp_597v2())
		{
			gRFLevel = &gLevel_TVB597A_V2_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB597A_V2_Modulator_GAP[modulator_type];	
		}
		else if(TSPL_nBoardRevision >= __REV_2_0__of_bd_593__)
		{
			if(modulator_type == TVB380_QAMA_MODE && IsAttachedBdTyp_593() && TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3)
			{
				if ( gCenter_Freq > 100000000 && gCenter_Freq <= 200000000 )
					dwQamA_level_gap = -3;
			}
			gRFLevel = &gLevel_TVB593_V2_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB593_V2_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_499())
		{
			switch (modulator_type)
			{
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ATSC_MH[0];
					break;

				case TVB380_DVBT2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T2[0];
					break;
				//2011/6/3
				case TVB380_DVBC2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_C2[0];
					break;

				case TVB380_CMMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_CMMB[0];
					break;

				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
					break;

				case TVB380_TDMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_TDMB[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_S_S2[0];
					break;

				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 )
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					else
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					break;

				case TVB380_QAMA_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					else
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					if(IsAttachedBdTyp_593() && TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3)
					{
						if ( gCenter_Freq > 100000000 && gCenter_Freq <= 200000000 )
							dwQamA_level_gap = -3;
					}

					break;

				case TVB380_VSB8_MODE:
				case TVB380_MULTI_VSB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				//2012/6/27 multi dvb-t
				case TVB380_MULTI_DVBT_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
					break;

				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_1[0];
					break;
				
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
					break;

				case TVB380_ISDBS_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_S[0];
					break;

				default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
					gRFLevel = &gLevel_DVB_TH_593_V2[0];
					break;
			}
		}
		else
		{
			switch (modulator_type)
			{
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_ATSC_MH_593[0];
					break;

				case TVB380_DVBT2_MODE:
					gRFLevel = &gLevel_DVB_T2_593[0];
					break;
				//2011/6/3
				case	TVB380_DVBC2_MODE:
					gRFLevel = &gLevel_DVB_C2_593[0];
					break;

				case TVB380_CMMB_MODE:
					gRFLevel = &gLevel_CMMB_593[0];
					break;

				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_DTMB_593[0];
					break;

				case TVB380_TDMB_MODE:
					gRFLevel = &gLevel_TDMB_593[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_DVB_S_S2_593[0];
					break;

				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 )
					{
						gRFLevel = &gLevel_QAM_64B_593[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_256B_593[0];
					}
					break;

				case TVB380_QAMA_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
					{
						gRFLevel = &gLevel_QAM_16_64_256A_593[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_32_128A_593[0];
					}
					break;

				case TVB380_VSB8_MODE:
				case TVB380_MULTI_VSB_MODE:
					gRFLevel = &gLevel_8VSB_593[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				//2012/6/27 multi dvb-t
				case TVB380_MULTI_DVBT_MODE:
					gRFLevel = &gLevel_DVB_TH_593[0];
					break;

				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_ISDB_T_593[0];
					break;
				
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_ISDB_T_13_593[0];
					break;

				default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
					gRFLevel = &gLevel_DVB_TH_593[0];
					break;
			}
		}
	}


	//AGC - i+=4 -> i+=5
	for ( i = 0; gRFLevel[i] != 5128; i+=5 )
	{
		if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel[i+1] )
		{
			*rf_level_min = (0 - gRFLevel[i + 2] - rfLevel_Modulator_Gap + dwQamA_level_gap);
			*rf_level_max = (0 - gRFLevel[i + 3] - rfLevel_Modulator_Gap + dwQamA_level_gap);
			break;
		}
	}
	return 1;
}

int	CAttnCtl::SET_TVB59x_RF_LEVEL_9775(long modulator_type)
{
	int i;

	//TVB593
	unsigned long dwStatus;
	unsigned long multiModeFreqGap = 0;
	if(modulator_type == TVB380_TSIO_MODE)
		return 1;

	if((IsAttachedBdTyp_593() || IsAttachedBdTyp_591S() || IsAttachedBdTyp_598()) && (modulator_type == TVB380_MULTI_VSB_MODE || modulator_type == TVB380_MULTI_QAMB_MODE || modulator_type == TVB380_MULTI_DVBT_MODE))
	{
		if(IsAttachedBdTyp_593() && modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(gSymbol_Rate == DVB_T_6M_BAND)
				multiModeFreqGap = 3000000;
			else if(gSymbol_Rate == DVB_T_7M_BAND)
				multiModeFreqGap = 3500000;
			else
				multiModeFreqGap = 4000000;
		}
		else if(IsAttachedBdTyp_598() && modulator_type == TVB380_MULTI_DVBT_MODE)
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

	LldPrint_FCall("SET_TVB59x_RF_LEVEL_9775", modulator_type, 0);

	if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1() )
	{
		switch (modulator_type)
		{
			case TVB380_TDMB_MODE:
				gRFLevel_AD9775 = &gLevel_TDMB_20_V3_AD9775[0];
				break;

			case TVB380_QPSK_MODE:
			case TVB380_DVBS2_MODE:
				gRFLevel_AD9775 = &gLevel_DVB_S_S2_20_V3_AD9775[0];
				break;

			case TVB380_QAMB_MODE:
				if ( gQAMB_Constllation == 0 )
				{
					gRFLevel_AD9775 = &gLevel_QAM_64B_20_V3_AD9775[0];
				}
				else
				{
					gRFLevel_AD9775 = &gLevel_QAM_256B_20_V3_AD9775[0];
				}
				break;

			case TVB380_QAMA_MODE:
				if ( gQAMA_Constllation == 0 || gQAMA_Constllation == 2 || gQAMA_Constllation == 4 )
				{
					gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_20_V3_AD9775[0];
				}
				else
				{
					gRFLevel_AD9775 = &gLevel_QAM_32_128A_20_V3_AD9775[0];

				}
				break;

			case TVB380_VSB8_MODE:
				gRFLevel_AD9775 = &gLevel_8VSB_20_V3_AD9775[0];
				break;
			
			case TVB380_DVBT_MODE:
			case TVB380_DVBH_MODE:
				gRFLevel_AD9775 = &gLevel_DVB_TH_20_V3_AD9775[0];
				break;

			case TVB380_ISDBS_MODE:
				gRFLevel_AD9775 = &gLevel_ISDB_S_20_V3_AD9775[0];
				break;
			
			default: 
				gRFLevel_AD9775 = &gLevel_DVB_TH_20_V3_AD9775[0];
				break;
		}

		//AD9775 - REG : 0x6, 0xA 
		for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=4 )
		{
			if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel_AD9775[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel_AD9775[i+1] )
			{
				TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
				TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

				LldPrint_ddd("[LLD]AD9775 :: RF ", gCenter_Freq, gRFLevel_AD9775[i+2], gRFLevel_AD9775[i+3]);

				break;
			}
		}
	}
	else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
	{
		//FIXED - TVB593 V2, TVB497
		//2011/3/11 FIXED
		if(IsAttachedBdTyp_499())
		{
			unsigned int regData;
			for ( i = 0; gLevel_499_AD9787[i] != 5128; i+=4 )
			{
				if ( (gCenter_Freq - multiModeFreqGap) > gLevel_499_AD9787[i] && (gCenter_Freq - multiModeFreqGap) <= gLevel_499_AD9787[i+1] )
				{
					
					regData = (((gLevel_499_AD9787[i+3] >> 7) & 0x03) << 24) + ((gLevel_499_AD9787[i+3] & 0x7F) << 17) + (((gLevel_499_AD9787[i+2] >> 8) & 0x1) << 16) + ((gLevel_499_AD9787[i+2] & 0xFF) << 8);
					TSPL_SET_AD9787(0x55100, regData);
					TSPL_SET_AD9787(0x55000, 0x026C0000);

					LldPrint_ddd("[LLD]AD9787 :: RF ", gCenter_Freq, gLevel_499_AD9787[i+2], gLevel_499_AD9787[i+3]);
	
					break;
				}
			}
		}
		else if(IsAttachedBdTyp_599())
		{
			gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
			for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
			{
				if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel_AD9775[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel_AD9775[i+1] )
				{
					TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
					TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

					TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
					TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

					LldPrint_5("[LLD]AD9775 :: RF", gCenter_Freq, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

					break;
				}
			}
		}
		else if(IsAttachedBdTyp_598())
		{
			gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
			for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
			{
				if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel_AD9775[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel_AD9775[i+1] )
				{
					TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
					TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

					TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
					TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

					LldPrint_5("[LLD]AD9775 :: RF", gCenter_Freq, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

					break;
				}
			}
		}
		else if(TSPL_nBoardRevision >= __REV_2_0__of_bd_593__ || IsAttachedBdTyp_597v2() || IsAttachedBdTyp_591() || IsAttachedBdTyp_591S())
		{
			gRFLevel_AD9775 = &gLevel_593_V21_AD9775[0];
			
			//AD9775 - REG : 0x6, 0xA, 0x5, 0x9
			for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=6 )
			{
				if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel_AD9775[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel_AD9775[i+1] )
				{
					if(modulator_type == TVB380_QAMA_MODE && i == 6)
					{
						if(IsAttachedBdTyp_593() && (TSPL_nBoardRevision >= __REV_2_1__of_bd_593v2__ && 
								TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3))
						{
							TSPL_SET_AD9775(0x06, (long)(gRFLevel_AD9775[i+2] - 1));
							TSPL_SET_AD9775(0x0A, (long)(gRFLevel_AD9775[i+3] - 1));
						}
						else
						{
							TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
							TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
						}
					}
					else	
					{
						TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
						TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);
					}
					TSPL_SET_AD9775(0x05, (long)gRFLevel_AD9775[i+4]);
					TSPL_SET_AD9775(0x09, (long)gRFLevel_AD9775[i+5]);

					LldPrint_5("[LLD]AD9775 :: RF", gCenter_Freq, (unsigned int)gRFLevel_AD9775[i+2], (unsigned int)gRFLevel_AD9775[i+3], (unsigned int)gRFLevel_AD9775[i+4], (unsigned int)gRFLevel_AD9775[i+5]);

					break;
				}
			}
		}
		else
		{
			switch (modulator_type)
			{
				case TVB380_ATSC_MH_MODE:
					gRFLevel_AD9775 = &gLevel_ATSC_MH_593_AD9775[0];
					break;

				case TVB380_DVBT2_MODE:
					gRFLevel_AD9775 = &gLevel_DVB_T2_593_AD9775[0];
					break;
				//2011/6/3
				case	TVB380_DVBC2_MODE:
					gRFLevel_AD9775 = &gLevel_DVB_C2_593_AD9775[0];
					break;

				case TVB380_CMMB_MODE:
					gRFLevel_AD9775 = &gLevel_CMMB_593_AD9775[0];
					break;

				case TVB380_DTMB_MODE:
					gRFLevel_AD9775 = &gLevel_DTMB_593_AD9775[0];
					break;

				case TVB380_TDMB_MODE:
					gRFLevel_AD9775 = &gLevel_TDMB_593_AD9775[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel_AD9775 = &gLevel_DVB_S_S2_593_AD9775[0];
					break;

				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 )
					{
						gRFLevel_AD9775 = &gLevel_QAM_64B_593_AD9775[0];
					}
					else
					{
						gRFLevel_AD9775 = &gLevel_QAM_256B_593_AD9775[0];
					}
					break;

				case TVB380_QAMA_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
					{
						gRFLevel_AD9775 = &gLevel_QAM_16_64_256A_593_AD9775[0];
					}
					else
					{
						gRFLevel_AD9775 = &gLevel_QAM_32_128A_593_AD9775[0];
					}
					break;

				case TVB380_VSB8_MODE:
				case TVB380_MULTI_VSB_MODE:
					gRFLevel_AD9775 = &gLevel_8VSB_593_AD9775[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				//2012/6/27 multi dvb-t
				case TVB380_MULTI_DVBT_MODE:
					gRFLevel_AD9775 = &gLevel_DVB_TH_593_AD9775[0];
					break;

				case TVB380_ISDBT_MODE:
					gRFLevel_AD9775 = &gLevel_ISDB_T_593_AD9775[0];
					break;
				
				case TVB380_ISDBT_13_MODE:
					gRFLevel_AD9775 = &gLevel_ISDB_T_13_593_AD9775[0];
					break;

				default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
					gRFLevel_AD9775 = &gLevel_DVB_TH_593_AD9775[0];
					break;
			}

			//AD9775 - REG : 0x6, 0xA 
			for ( i = 0; gRFLevel_AD9775[i] != 5128; i+=4 )
			{
				if ( (gCenter_Freq - multiModeFreqGap) > gRFLevel_AD9775[i] && (gCenter_Freq - multiModeFreqGap) <= gRFLevel_AD9775[i+1] )
				{
					TSPL_SET_AD9775(0x06, (long)gRFLevel_AD9775[i+2]);
					TSPL_SET_AD9775(0x0A, (long)gRFLevel_AD9775[i+3]);

					LldPrint_ddd("[LLD]AD9775 :: RF", gCenter_Freq, gRFLevel_AD9775[i+2], gRFLevel_AD9775[i+3]);

					break;
				}
			}
		}
	}

	return 1;
}
int	CAttnCtl::GET_MODULATOR_RF_LEVEL_RANGE_Improve(long modulator_type, double *rf_level_min, double *rf_level_max)
{
	int i;

	//TVB593
	unsigned long dwStatus;
	double rfLevel_Modulator_Gap = 0;
	double dwQamA_level_gap = 0;
	unsigned long multiModeFreqGap = 0;

	if(modulator_type == TVB380_TSIO_MODE)
		return 1;

	if((IsAttachedBdTyp_593() || IsAttachedBdTyp_591S() || IsAttachedBdTyp_598()) && (modulator_type == TVB380_MULTI_VSB_MODE || modulator_type == TVB380_MULTI_QAMB_MODE || modulator_type == TVB380_MULTI_DVBT_MODE))
	{
		if(IsAttachedBdTyp_593() && modulator_type == TVB380_MULTI_DVBT_MODE)
		{
			if(gSymbol_Rate == DVB_T_6M_BAND)
				multiModeFreqGap = 3000000;
			else if(gSymbol_Rate == DVB_T_7M_BAND)
				multiModeFreqGap = 3500000;
			else
				multiModeFreqGap = 4000000;
		}
		else if(IsAttachedBdTyp_598() && modulator_type == TVB380_MULTI_DVBT_MODE)
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
	LldPrint_FCall("GET_MODULATOR_RF_LEVEL_RANGE", modulator_type, 0);

	if(gSingleTone == 1 && IsAttachedBdTyp_597A_V4())
	{
		gRFLevel = &gLevel_TVB597A_V4_SINGLETONE[0];
	}
	else if(gSingleTone == 1 && (IsAttachedBdTyp_599() || IsAttachedBdTyp_598()))
	{
		gRFLevel = &gLevel_TVB599A_V1_SINGLETONE[0];
	}
	//else if(gSingleTone == 1 && IsAttachedBdTyp_598())
	//{
	//	gRFLevel = &gLevel_TVB598_V1_SINGLETONE[0];
	//}
	else if(gSingleTone == 1 && IsAttachedBdTyp_TVB593_597A_V3())
	{
		if(IsAttachedBdTyp_597v2())
		{
			gRFLevel = &gLevel_TVB597A_V3_SINGLETONE[0];
		}
		else
		{
			gRFLevel = &gLevel_TVB593_V3_SINGLETONE[0];
		}
	}
	else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1() )
	{
		switch (modulator_type)
		{
			case TVB380_TDMB_MODE:
				gRFLevel = &gLevel_TDMB_20_V3[0];
				break;

			case TVB380_QPSK_MODE:
			case TVB380_DVBS2_MODE:
				gRFLevel = &gLevel_DVB_S_S2_20_V3[0];
				break;

			case TVB380_QAMB_MODE:
				if ( gQAMB_Constllation == 0 )
				{
					gRFLevel = &gLevel_QAM_64B_20_V3[0];
				}
				else
				{
					gRFLevel = &gLevel_QAM_256B_20_V3[0];
				}
				break;

			case TVB380_QAMA_MODE:
				if ( gQAMA_Constllation == 0 || gQAMA_Constllation == 2 || gQAMA_Constllation == 4 )
				{
					gRFLevel = &gLevel_QAM_16_64_256A_20_V3[0];
				}
				else
				{
					gRFLevel = &gLevel_QAM_32_128A_20_V3[0];
				}
				break;

			case TVB380_VSB8_MODE:
				gRFLevel = &gLevel_8VSB_20_V3[0];
				break;
			
			case TVB380_DVBT_MODE:
			case TVB380_DVBH_MODE:
				gRFLevel = &gLevel_DVB_TH_20_V3[0];
				break;

			case TVB380_ISDBS_MODE:
				gRFLevel = &gLevel_ISDB_S_20_V3[0];
				break;
			
			default: 
				gRFLevel = &gLevel_DVB_TH_20_V3[0];
				break;
		}
	}
	else if ( IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2() )
	{
		//FIXED - TVB593 V2, TVB497
		//2011/3/11 FIXED
		if(IsAttachedBdTyp_591())
		{
			switch (modulator_type)
			{
				case TVB380_VSB8_MODE:
					gRFLevel = &gLevel_TVB591_VSB[0];
					break;

				case TVB380_TDMB_MODE:
				case TVB380_QAMA_MODE:
				case TVB380_QAMB_MODE:
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				case TVB380_IQ_PLAY_MODE:
					gRFLevel = &gLevel_TVB591_QAM_A_B_TDMB_DVB_TH[0];
					break;
				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_TVB591_ISDB_T_1[0];
					break;
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_TVB591_ISDB_T_13[0];
					break;
				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_TVB591_DTMB[0];
					break;
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_TVB591_ATSC_MH[0];
					break;
			}

		}
		//2012/2/7 TVB591S 
		else if(IsAttachedBdTyp_591S())
		{
			//2012/7/23 TVB591S V2
			if(IsAttachedBdTyp_591S_V2())
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB591S_V2_ATSC_MH[0];
						break;
					case TVB380_ISDBT_MODE:
					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TVB591S_V2_TDMB_ISDB_T_1[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_TVB591S_V2_DVB_S_S2[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_QAMA_MODE:
					case TVB380_MULTI_QAMB_MODE:
						gRFLevel = &gLevel_TVB591S_V2_QAM_A_B[0];
						break;

					case TVB380_VSB8_MODE:
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					case TVB380_MULTI_VSB_MODE:
						gRFLevel = &gLevel_TVB591S_V2_DVB_T_H_8VSB[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
						gRFLevel = &gLevel_TVB591S_V2_ISDB_T_13[0];
						break;
					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB591S_V2_DTMB[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_TVB591S_V2_ISDB_S[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;
				}
			}
			else
			{
				switch (modulator_type)
				{
					case TVB380_ATSC_MH_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ATSC_MH[0];
						break;
					case TVB380_TDMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_TDMB[0];
						break;

					case TVB380_QPSK_MODE:
					case TVB380_DVBS2_MODE:
						gRFLevel = &gLevel_TVB591S_V1_DVB_S_S2[0];
						break;

					case TVB380_QAMB_MODE:
					case TVB380_QAMA_MODE:
					case TVB380_MULTI_QAMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_QAM_A_B[0];
						break;

					case TVB380_VSB8_MODE:
					case TVB380_DVBT_MODE:
					case TVB380_DVBH_MODE:
					case TVB380_MULTI_VSB_MODE:
					//2012/6/27 Multi DVB-T
					case TVB380_MULTI_DVBT_MODE:

						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;

					case TVB380_ISDBT_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_1[0];
						break;
					
					case TVB380_ISDBT_13_MODE:
					case TVB380_DTMB_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_T_13_DTMB[0];
						break;

					case TVB380_ISDBS_MODE:
						gRFLevel = &gLevel_TVB591S_V1_ISDB_S[0];
						break;

					default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
						gRFLevel = &gLevel_TVB591S_V1_DVB_T_H_VSB[0];
						break;
				}
			}
		}
		else if(IsAttachedBdTyp_597A_V4())
		{
			gRFLevel = &gLevel_TVB597A_V4_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB597A_V4_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_599()/* || IsAttachedBdTyp_598()*/)
		{
			gRFLevel = &gLevel_TVB599A_V1_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB599A_V1_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_598())
		{
			gRFLevel = &gLevel_TVB598_V1_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB598_V1_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_TVB593_597A_V3())
		{
			if(IsAttachedBdTyp_597v2())
			{
				gRFLevel = &gLevel_TVB597A_V3_BASE[0];
				rfLevel_Modulator_Gap = gLevel_TVB597A_V3_Modulator_GAP[modulator_type];	
			}
			else
			{
				if(IsAttachedBdTyp_TVB593_V4())
				{
					gRFLevel = &gLevel_TVB593_V4_BASE[0];
					rfLevel_Modulator_Gap = gLevel_TVB593_V4_Modulator_GAP[modulator_type];	
				}
				else
				{
					gRFLevel = &gLevel_TVB593_V3_BASE[0];
					rfLevel_Modulator_Gap = gLevel_TVB593_V3_Modulator_GAP[modulator_type];	
				}
			}
		}
		else if(IsAttachedBdTyp_597v2())
		{
			gRFLevel = &gLevel_TVB597A_V2_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB597A_V2_Modulator_GAP[modulator_type];	
		}
		else if(TSPL_nBoardRevision >= __REV_2_0__of_bd_593__)
		{
			if(modulator_type == TVB380_QAMA_MODE && IsAttachedBdTyp_593() && TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3)
			{
				if ( gCenter_Freq > 100000000 && gCenter_Freq <= 200000000 )
					dwQamA_level_gap = -3;
			}
			gRFLevel = &gLevel_TVB593_V2_BASE[0];
			rfLevel_Modulator_Gap = gLevel_TVB593_V2_Modulator_GAP[modulator_type];	
		}
		else if(IsAttachedBdTyp_499())
		{
			switch (modulator_type)
			{
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ATSC_MH[0];
					break;

				case TVB380_DVBT2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T2[0];
					break;
				//2011/6/3
				case TVB380_DVBC2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_C2[0];
					break;

				case TVB380_CMMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_CMMB[0];
					break;

				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
					break;

				case TVB380_TDMB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_TDMB[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_S_S2[0];
					break;

				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 )
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					else
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					break;

				case TVB380_QAMA_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					else
					{
						gRFLevel = &gLevel_TVB597A_V2_QAM_A_B[0];
					}
					if(IsAttachedBdTyp_593() && TSPL_nBoardRevision < __REV_6_0_of_bd_593v3_597v3)
					{
						if ( gCenter_Freq > 100000000 && gCenter_Freq <= 200000000 )
							dwQamA_level_gap = -3;
					}

					break;

				case TVB380_VSB8_MODE:
				case TVB380_MULTI_VSB_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				//2012/6/27 multi dvb-t
				case TVB380_MULTI_DVBT_MODE:
					gRFLevel = &gLevel_TVB597A_V2_DVB_T_H_VSB[0];
					break;

				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_1[0];
					break;
				
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_T_13_DTMB[0];
					break;

				case TVB380_ISDBS_MODE:
					gRFLevel = &gLevel_TVB597A_V2_ISDB_S[0];
					break;

				default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
					gRFLevel = &gLevel_DVB_TH_593_V2[0];
					break;
			}
		}
		else
		{
			switch (modulator_type)
			{
				case TVB380_ATSC_MH_MODE:
					gRFLevel = &gLevel_ATSC_MH_593[0];
					break;

				case TVB380_DVBT2_MODE:
					gRFLevel = &gLevel_DVB_T2_593[0];
					break;
				//2011/6/3
				case	TVB380_DVBC2_MODE:
					gRFLevel = &gLevel_DVB_C2_593[0];
					break;

				case TVB380_CMMB_MODE:
					gRFLevel = &gLevel_CMMB_593[0];
					break;

				case TVB380_DTMB_MODE:
					gRFLevel = &gLevel_DTMB_593[0];
					break;

				case TVB380_TDMB_MODE:
					gRFLevel = &gLevel_TDMB_593[0];
					break;

				case TVB380_QPSK_MODE:
				case TVB380_DVBS2_MODE:
					gRFLevel = &gLevel_DVB_S_S2_593[0];
					break;

				case TVB380_QAMB_MODE:
				case TVB380_MULTI_QAMB_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 )
					{
						gRFLevel = &gLevel_QAM_64B_593[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_256B_593[0];
					}
					break;

				case TVB380_QAMA_MODE:
					dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ADDR_TXMODE);
					dwStatus = (dwStatus>>2)&0x01;
					if ( dwStatus == 0 || dwStatus == 2 || dwStatus == 4 )
					{
						gRFLevel = &gLevel_QAM_16_64_256A_593[0];
					}
					else
					{
						gRFLevel = &gLevel_QAM_32_128A_593[0];
					}
					break;

				case TVB380_VSB8_MODE:
				case TVB380_MULTI_VSB_MODE:
					gRFLevel = &gLevel_8VSB_593[0];
					break;
				
				case TVB380_DVBT_MODE:
				case TVB380_DVBH_MODE:
				//2012/6/27 multi dvb-t
				case TVB380_MULTI_DVBT_MODE:
					gRFLevel = &gLevel_DVB_TH_593[0];
					break;

				case TVB380_ISDBT_MODE:
					gRFLevel = &gLevel_ISDB_T_593[0];
					break;
				
				case TVB380_ISDBT_13_MODE:
					gRFLevel = &gLevel_ISDB_T_13_593[0];
					break;

				default: //TVB380_DVB_T_MODE, TVB380_DVB_H_MODE, TVB380_IQ_PLAY_MODE
					gRFLevel = &gLevel_DVB_TH_593[0];
					break;
			}
		}
	}
	unsigned long Fn, Fn2;
	double Cn_Min, Cn2_Min, Cn_Max, Cn2_Max; 
	double Vx_Max, Vx_Min;
	int index_;
	//AGC - i+=4 -> i+=5
	for ( i = 0; gRFLevel[i] != 5128; i+=5 )
	{
		if ( (gCenter_Freq + multiModeFreqGap) > gRFLevel[i] && (gCenter_Freq + multiModeFreqGap) <= gRFLevel[i+1] )
		{
			if((gCenter_Freq + multiModeFreqGap) == gRFLevel[i+1])
			{
				*rf_level_min = 0 - gRFLevel[i + 2] - rfLevel_Modulator_Gap + dwQamA_level_gap;
				*rf_level_max = 0 - gRFLevel[i + 3] - rfLevel_Modulator_Gap + dwQamA_level_gap;
			}
			else
			{
				if(i == 0 || i == 1)
				{
					Fn2 = gRFLevel[11];
					Fn = gRFLevel[6];
					Cn2_Max = gRFLevel[13];
					Cn_Max = gRFLevel[8];
					Cn2_Min = gRFLevel[12];
					Cn_Min = gRFLevel[7];
				}
				else
				{
					Fn2 = gRFLevel[i+1];
					Fn = gRFLevel[i + 1 - 5];
					Cn2_Max = gRFLevel[i + 3];
					Cn_Max = gRFLevel[i + 3 - 5];
					Cn2_Min = gRFLevel[i + 2];
					Cn_Min = gRFLevel[i + 2 - 5];
				}
				Vx_Max = (((double)(gCenter_Freq - Fn) * (Cn2_Max - Cn_Max)) / (double)(Fn2 - Fn)) + Cn_Max;
				Vx_Min = (((double)(gCenter_Freq - Fn) * (Cn2_Min - Cn_Min)) / (double)(Fn2 - Fn)) + Cn_Min;
				*rf_level_min = 0 - Vx_Min - rfLevel_Modulator_Gap + dwQamA_level_gap;
				*rf_level_max = 0 - Vx_Max - rfLevel_Modulator_Gap + dwQamA_level_gap;
			}
			break;
		}
	}
	return 1;
}

#endif