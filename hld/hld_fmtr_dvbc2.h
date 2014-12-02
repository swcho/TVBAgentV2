
#ifndef __HLD_FMTER_DVBC2__H__
#define __HLD_FMTER_DVBC2__H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"


class CHldFmtrDvbC2_
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];

#if defined(WIN32)
	HANDLE TL_C2MI_hMutex;
	HANDLE TL_C2MI_hMutex2;
#else
	pthread_mutex_t TL_C2MI_hMutex;
	pthread_mutexattr_t TL_C2MI_hMutexAttr;
	pthread_t TL_C2MI_thread;

	pthread_mutex_t TL_C2MI_hMutex2;
	pthread_mutexattr_t TL_C2MI_hMutexAttr2;
	pthread_t TL_C2MI_thread2;
#endif


public:
	//	DVB-C2
	int __C2_BW;
	int __C2_L1;
	int __C2_Guard;
	int __C2_Network;
	int __C2_System;
	int __C2_StartFreq;
	int __C2_NumNoth;
	int __C2_RevTone;
	int __C2_NotchStart;
	int __C2_NotchWidth;
	int __C2_Dslice_type;
	int __C2_Dslice_FecHeder;
	int __C2_Dslice_BBHeder;
	int __C2_Dslice_TonePos;
	int __C2_Dslice_OffRight;
	int __C2_Dslice_OffLeft;
	int __C2_Plp_Mod;
	int __C2_Plp_Code;
	int __C2_Plp_Fec;
	//2011/6/2
	int __C2_CreateFile;

	//2011/5/19 DVB-C2 MULTI PLP ========>>
	int __C2_PLP_Count;
	int __C2_PLP_ID_M[MAX_PLP_COUNT];
	int __C2_PLP_DSType_M[MAX_PLP_COUNT];
	int __C2_PLP_FEC_H_T_M[MAX_PLP_COUNT];
	int __C2_PLP_MOD_M[MAX_PLP_COUNT];
	int __C2_PLP_COD_M[MAX_PLP_COUNT];
	int __C2_PLP_FEC_M[MAX_PLP_COUNT];
	int __C2_PLP_BLK_M[MAX_PLP_COUNT];
	int __C2_PLP_HEM_M[MAX_PLP_COUNT];
	int __C2_PLP_TS_Bitrate_M[MAX_PLP_COUNT];
	char __C2_PLP_TS_M[MAX_PLP_COUNT][1024];

	int TL_C2MI_PumpingThreadDone;
	int TL_C2MI_ConvertingThreadDone;

public:
	CHldFmtrDvbC2_(void);
	virtual ~CHldFmtrDvbC2_();

	void	SetCommonMethod_C2(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void	CreateC2miMutex(void);
	void	DestroyC2miMutex(void);
	void	LockC2miMutex(void);
	void	UnlockC2miMutex(void);
	int	DummyWaitDvbC2_OnPlayCont__HasEndlessWhile(void);

	double RunC2MiParser(char *szFile, char *szResult);
	int SetC2MiParam(	int C2_BW,
						int C2_L1,
						int C2_Guard,
						int C2_Network,
						int C2_System,
						int C2_StartFreq,
						int C2_NumNoth,
						int C2_RevTone,
						int C2_NotchStart,
						int C2_NotchWidth,
						int C2_Dslice_type,
						int C2_Dslice_FecHeder,
						int C2_Dslice_BBHeder,
						int C2_Dslice_TonePos,
						int C2_Dslice_OffRight,
						int C2_Dslice_OffLeft,
						int C2_Plp_Mod,
						int C2_Plp_Code,
						int C2_Plp_Fec,
						//2011/5/19 DVB-C2 MULTI PLP	
						int C2_Plp_Count,
						int C2_Plp_ID,
						int C2_Plp_Blk,
						int C2_Plp_TS_Bitrate,
						char *C2_Plp_TS,
						int C2_createFile 
					);

};


#endif

