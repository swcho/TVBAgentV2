//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_ATTN_CTL_H_
#define	__TLV_ATTN_CTL_H_

#include 	"Chip_util.h"
#include	"dds_ctl.h"

#ifdef __cplusplus
extern "C" {
#endif

//AGC
#define MAX_RF_LEVEL		0	//dBm
#define MIN_ATTEN			0	//dB
#define MAX_ATTEN			31.5
#define MAX_TAT4710			127	//dB
#define MAX_TAT4710_LOSS	5


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CAttnCtl	:	public	CDdsCtl
{
private:


public:
	CAttnCtl(void);
	~CAttnCtl();

	double CalcAGC_Atten(double* pRFLevel, double rf, int amp, double* pAGC_Level_Min, double* pAGC_Level_Max);
	double CalcRFLevel(long modulator_type, double rf, long agc_on_off, double* pAGC_Level_Min, double* pAGC_Level_Max, double* pAGC_Level_Offset);

	int	GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *rf_level_min, double *rf_level_max);
	int	GET_MODULATOR_RF_LEVEL_RANGE_Improve(long modulator_type, double *rf_level_min, double *rf_level_max);
	int	SET_TVB59x_RF_LEVEL_9775(long modulator_type);
	double    GetTAT4710_Offset();

};

#ifdef __cplusplus
}
#endif



#endif
