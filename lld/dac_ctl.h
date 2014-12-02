//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_DAC_CTL_H_
#define	__TLV_DAC_CTL_H_

#include 	"Chip_util.h"
#include	"interp.h"

#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CDacCtl	:	public	CChipUtil,	public	CUtilInterp
{
private:

public:
	double g_ScaleOffset;	//SCALE ADJUST for .1dB CONTROL
	unsigned long gRF_143KHZ;//0=No shift, 1= ISDB-T 143KHz shift
	unsigned long gSpectral_Inversion;
	unsigned long gClock_Mode;//TVB590S
//	unsigned long gChannel_Freq_Offset;//Hz
	unsigned long gCenter_Freq;//Hz, fixed - 473000000 -> 1000000
	unsigned long gSymbol_Rate;//Sps
	unsigned long gRRC_Filter;//DVB-S2, 0(0.20), 1(0.25), 2(0.35), 3(none)
	unsigned long gBypass_AMP;//TVB595V1, TVB590V9.2, (1=USE, 2=BYPASS if V1.0), (1=BYPASS, 2=USE if higher than V1.0)
	//FIXED - IF +- offset
	unsigned long gRF_143KHZ_sign;//or 1

	//TVB590S V3
	unsigned long gQAMA_Constllation;
	unsigned long gQAMB_Constllation;

	double gCurrentTAT4710;
	double gAGC_MaxAtten;
	double gAGC_MinAtten;
	double gAGC_AttenOffset;
	double gAGC_CurrentAtten;
	double gAGC_CurrentLevel;
	long	  gAGC_On_Off;

//RF power level
	double gMaxLevel;
	double gMinLevel;
	double gCurrentLevel;
	double gCurrentAtten;
	double *gRFLevel;
//TVB593
	double *gRFLevel_AD9775;

	int CurrentTSSrc;//current TS direction

	//2012/9/18 SingleTone Flag
	int		gSingleTone;

	int		gEnableTAT4710;

private:
	int	CntlDac_AD9775(long modulation_mode, unsigned long dwfreq, long symbol_freq);	//	dac is ad9775
	int	CntlDac_AD9857(long modulation_mode, unsigned long dwfreq, long symbol_freq);	//	dac is ad9857

public:
	CDacCtl(void);
	~CDacCtl();

	int	IsLockAD9775(void);
	int _stdcall TSPL_GET_AD9775(long reg);
	int _stdcall TSPL_SET_AD9775(long reg, long data);
	int	Default_Configure_AD9775(long modulation_mode, unsigned long dwfreq, long symbol_freq);
	int	CntlDac_AD9xxx(long modulation_mode, unsigned long dwfreq, long symbol_freq);	//2150MHz support - int -> unsigned longv

	//2013/1/4 TVB499
	int	Configure_AD9787_Register(long modulation_mode, 
										 unsigned long dwfreq, //2150MHz support - int -> unsigned long
										 long symbol_freq);
	int _stdcall TSPL_GET_AD9787(long reg);
	int _stdcall TSPL_SET_AD9787(long reg, long data);

};

#ifdef __cplusplus
}
#endif

#endif

