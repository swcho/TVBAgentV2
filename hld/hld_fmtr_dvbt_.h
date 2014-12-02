
#ifndef __HLD_FMTER_DVBT__H__
#define __HLD_FMTER_DVBT__H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"
#include	"../include/hld_structure.h"


class CHldFmtrDvbT_
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

#if defined(WIN32)
	HANDLE TL_T2MI_hMutex;
	HANDLE TL_T2MI_hMutex2;
#else
	pthread_mutex_t TL_T2MI_hMutex;
	pthread_mutexattr_t TL_T2MI_hMutexAttr;
	pthread_t TL_T2MI_thread;

	pthread_mutex_t TL_T2MI_hMutex2;
	pthread_mutexattr_t TL_T2MI_hMutexAttr2;
	pthread_t TL_T2MI_thread2;
#endif

	char debug_string[100];

public:
//DVB-T2 - Multi PLP
	int PLP_COUNT;
	int PLP_ROTATION_M[MAX_PLP_COUNT];
	int PLP_ID_M[MAX_PLP_COUNT];
	int PLP_MOD_M[MAX_PLP_COUNT];
	int PLP_COD_M[MAX_PLP_COUNT];
	int PLP_FEC_TYPE_M[MAX_PLP_COUNT];
	int PLP_MODE_M[MAX_PLP_COUNT]; //high(2) or normal(1) efficiency mode, 0(not specified if T2_VERSION == "0000")
	int PLP_NUM_BLOCKS_M[MAX_PLP_COUNT];
	int PLP_BITRATE_M[MAX_PLP_COUNT];
	int PLP_TS_BITRATE_M[MAX_PLP_COUNT];
	char PLP_TS_M[MAX_PLP_COUNT][1024];

	char PLP_TIME_IL_LENGTH_M[MAX_PLP_COUNT];


	int TL_T2MI_PumpingThreadDone;
	int TL_T2MI_ConvertingThreadDone;


//DVB-T2
	int TL_DVB_T2_BW;
	unsigned long FREQUENCY;
	int FFT_SIZE, GUARD_INTERVAL, L1_MOD, PILOT_PATTERN, BW_EXT;
	int NETWORK_ID, T2_SYSTEM_ID, CELL_ID, S1, PLP_MOD, PLP_COD, PLP_FEC_TYPE, HEM;
	int NUM_T2_FRAMES, NUM_DATA_SYMBOLS, PLP_NUM_BLOCKS, PID;


public:
	CHldFmtrDvbT_(void);
	virtual ~CHldFmtrDvbT_();

	void	SetCommonMethod_5(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void	CreateT2miMutex(void);
	void	DestroyT2miMutex(void);
	void	LockT2miMutex(void);
	void	UnlockT2miMutex(void);

#ifdef WIN32
	double RunT2MiParser(char *szFile, T2MI_Parsing_Info *szResult);
#else
	double RunT2MiParser(char *szFile, char *szResult);
#endif
	int SetT2MiParam(
		int _BW, int _FFT_SIZE, int _GUARD_INTERVAL, int _L1_MOD, int _PILOT_PATTERN, int _BW_EXT, double _FREQUENCY,
		int _NETWORK_ID, int _T2_SYSTEM_ID, int _CELL_ID, int _S1, int _PLP_MOD, int _PLP_COD, int _PLP_FEC_TYPE, int _HEM, 
		int _NUM_T2_FRAMES, int _NUM_DATA_SYMBOLS, int _PLP_NUM_BLOCKS, int _PID
		//DVB-T2 - Multi PLP MUX
#ifdef _MULTI_PLP_MUX_0
		, int _PLP_ROTATION, int _PLP_COUNT, int _PLP_ID, int _PLP_BITRATE, int _PLP_TS_BITRATE, char *_PLP_TS, int _PLP_TI_LENGTH
#endif
		);
	int GetT2MiParam(int *num_data_symbol, int *plp_num_block);

};


#endif

