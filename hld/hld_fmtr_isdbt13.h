
#ifndef __HLD_FMTER_ISDBT_13_H__
#define __HLD_FMTER_ISDBT_13_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_tmcc_rmx.h"
#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"

typedef	enum	_cmd_isdbt13__
{
	_CMD_ISDBT13_undefined	=	0,

	_CMD_ISDBT13_detected_pkt_size,
	_CMD_ISDBT13_capture_dta,
	_CMD_ISDBT13_play_dta,

	_CMD_ISDBT13_self_sta_chg,

	_CMD_ISDBT13_max
}	_CMD_ISDBT13__;
typedef	enum	_sta_isdbt13__
{
	_STA_ISDBT13_undefined	=	0,

	_STA_ISDBT13_normalfile_playback,

	_STA_ISDBT13_loopthru_initial_requsted,
	_STA_ISDBT13_loopthru_initial_starting,

	_STA_ISDBT13_loopthru_204_onstarting,
	_STA_ISDBT13_loopthru_204_ongoing,
	_STA_ISDBT13_loopthru_204_onendreqed,

	_STA_ISDBT13_loopthru_188_onrestarting,
	_STA_ISDBT13_loopthru_188_ongoing,
	_STA_ISDBT13_loopthru_188_onpause,
	_STA_ISDBT13_loopthru_188_onendreqed,

	_STA_ISDBT13_playback_ip_reqstarting,
	_STA_ISDBT13_playback_ip_ongoing,
	_STA_ISDBT13_playback_ip_reqend,

	_STA_ISDBT13_max
}	_STA_ISDBT13__;

class CHldFmtrIsdbT13	:	public	CHldTmccRmx
{
private:
	int	my_hld_id;
	void	*my_hld;

	int	_fmter_sta_;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];
#if defined(WIN32)
	HANDLE	_dbg_file;
#endif

	char			*__tmp;
	int 			__ReadPos, __BufferCnt, __EndOfRemux;
	DWORD			__dwRet;

	int nTsWrCounter;
	int nTsRdCounter;

	int nCnt;
	int nLenRd;
	int nFileLength;

	unsigned char HeaderA;
	unsigned char HeaderB;
	unsigned char HeaderC;

	unsigned char *RdData;
	unsigned char TsData[204];
	unsigned char TmData[204];
	layer LayerA, LayerB, LayerC;
	unsigned char *BufferA;
	unsigned char *BufferB;
	unsigned char *BufferC;
	int nLenA, nLenB, nLenC;
	int nAddrA, nAddrB, nAddrC;

//	unsigned char	*test_buffer_temp;	//	temp buf to remux of the cap data of IsdbT13 and AtscMH loop-thru.


//	int Current_Mode;		//used by TMCC Remuxer
	int Current_GuardInterval;
	int Current_Partial_Reception;
	int Current_BoradcastType;
	layer Current_LayerA, Current_LayerB, Current_LayerC;

	int TL_nFindFrameIndicator;
	int Mode;		//used by GIST's Delay Adjustment
	int GuardInterval;
	int Partial_Reception;
	int BoradcastType;

public:
	unsigned int	now_active_tmcc_remuxing;

	int Current_Mode;		//used by TMCC Remuxer

private:
	int DbgWrCaptureDta(int _size, unsigned char *_buf);
	int TL_Get_Datarate(int Mode, int GuardInterval, layer* pLayerA, layer* pLayerB, layer* pLayerC);
	int TL_Get_Datarate(int Mode, int GuardInterval, layer* pLayer);
	int TL_IP_Initialize_SyncPosBuf(void);
	int	TL_FrameIndicatorFunction(unsigned char *szBuf, int nlen,
									unsigned char* pHeaderA,
									unsigned char* pHeaderB,
									unsigned char* pHeaderC,
									int *pDelayLenA,
									int *pDelayLenB,
									int *pDelayLenC,
									int* pMode,
									layer* pLayerA,
									layer* pLayerB,
									layer* pLayerC,
									int *pGuardInterval, 
									int *pPartial_Reception,
									int *pBroadcastType);
	int DoCalculateDelayAdjustLength(int Mode, int Constel, int CodRate, int NumSegm);

public:
	CHldFmtrIsdbT13(int _my_id, void *_hld);
	virtual ~CHldFmtrIsdbT13();

	void	SetCommonMethod_82(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);

	void InitPlayParameters_IsdbT13(void);
	DWORD	*PktSize_IsdbT13_AtscMH(void);

///////////////////////////////////////////////////////////////////////
	int ActivateTmccRmxer(void);	//	not use
	int Start_TmccRemuxer(unsigned char* pBuffer);	//	not use
	int DeactivateTmccRmxer(void);	//	not use
	int SetOffset_TmccRemuxer(int _setcur_as_start);
	int Restart_TmccRemuxerFor_IsdbT_1_13(void);
	int Restart_TmccRemuxerAtEof_IsdbT_1_13(void);
	int SetOffset_TmccRemuxerRepeatCond(void);
	void StopAndClose_TmccRemuxer(void);
	int StopAndClose_Restart_TmccRemuxer(void);
	int RdPlayDta_From_TmccRemuxer(void);
	void TL_DelayAdjustment(void);	//	isdbt-13 only

///////////////////////////////////////////////////////////////////////	app. api
	int GetRmxDtaRate(long layer_index);
	int SetRmx_Info(
		long btype, long mode, long guard_interval, long partial_reception, long bitrate,
		long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
		long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
		long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
	int GetRmx_Info(char *szFile, int layer_index);
	void	CLEAR_REMUX_INFO(void);
	long	PlayRate_IsdbT13(long play_freq_in_herz, long nOutputClockSource);
	int ProcessStaMachineIsdbT13(int para_cmd, unsigned char *para_ptr, int para_any, unsigned char **para_ptr2);
	void	ChgStaMachineIsdbT13(int _sta);
	int StaMachineIsdbT13(void);
	int IsLoopthru188_IsdbT13(void);
	int IsLoopthru188InputFullness_IsdbT13(void);
	void	PauseLoopthru188_IsdbT13(unsigned char *_fn);
	void	Buf_Clear_Loopthru188_IsdbT13(void);
	//2012/8/23
	void	PauseLoopthru188_IsdbT13(struct _TSPH_TS_INFO **ppTsInfo);


};


#endif

