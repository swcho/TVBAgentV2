
#ifndef __HLD_FMTER_IP_H__
#define __HLD_FMTER_IP_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif


#include	"hld_fmtr_bert.h"
#include	"hld_fmtr_iq.h"
#include	"hld_fmtr_isdbs.h"
#include	"hld_fmtr_dvbt_.h"
#include	"hld_fmtr_dvbc2.h"		//2011/7/21 added
#include	"hld_fmtr_cmmb.h"
#include	"hld_fmtr_tdmb.h"
#include	"hld_multi_rfout.h"

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"
#include	"srv_rcver.h"

//	#define	__USE_MAX_PLAYRATE_IP_PLAYBACK__

#define	__CRITERIA_IP_BUF_INITIAL_FILLL__				8	//	8-sec
#define	__CRITERIA_IP_BUF_STABLE_COND___				10	//	100%
#define	__CRITERIA_IP_BUF_STABLE_COND_under__			3	//	30%
#define	__CRITERIA_IP_BUF_MONITOR_START__				200	//	200-subblocks
#define	__CRITERIA_DIFF_IP_BUF_IN_OUT__outofservice__	30	//	3%
#define	__CRITERIA_DIFF_IP_BUF_IN_OUT__diverge__		2	//	0.2%
#define	__CRITERIA_ORIENTATION_DELAY_ins_rmv__			5
typedef	enum	__ip_state__
{
	__IP_STATE_buf_fill_wait__	=	0,
	__IP_STATE_playback_progress__,
	__IP_STATE_transition_buf_underover__,

	__IP_STATE_CMD_init_buf__	=	0,
	__IP_STATE_CMD_wr_hw__,
	__IP_STATE_CMD_rd_ip__,
	__IP_STATE_CMD_wrap_max_buf__,
	__IP_STATE_CMD_contents_modified__,
	__IP_STATE_CMD_detected_under__,
	__IP_STATE_CMD_detected_stable__,

}	__IP_STATE__;

class CHldFmtrIp	:
	public	CHldFmtrBert,
	public	CHldFmtrIq,
	public	CHldFmtrIsdbS,
	public	CHldFmtrDvbT_,
	public	CHldFmtrTdmb,
	public	CHldFmtrDvbC2_,
	public	CHldMultiRfOut,
	public	CIpApi
{
private:
	int	my_hld_id;
	void	*my_hld;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	int prev_max_playrate_sta;
	int inc_dec_cnt;
	float under_over_vector;

	float	__estimated_in_speed_bytespersec;
	float	__estimated_out_speed_bytespersec;

	int ip_pkt_monotor_sta;
	unsigned int cnt_ip_recv_pkt_bytes;
	unsigned int cnt_ip_wr_hw_pkt_bytes;
	int ip_pkt_monotor_orientation;

	int TL_DiffIpRecvBuf;
	int TL_ip_bfp;
	unsigned char	*TL_ip_buffer;	//	data pool. has been  received via ip-network.
	int TL_ip_read; 	//	nxt rd tpr of TL_ip_buffer[]
	int TL_ip_write;	//	nxt wr ptr of TL_ip_buffer[]
	int TL_ip_count;	//	bytes of TL_ip_buffer[]
	int TL_ip_PumpingThreadDone;
	unsigned char	*ip_tmp_buf;

#ifdef WIN32
#else
	pthread_t	g_thread1;
#endif

public:
	unsigned long	__past_msec;

	HANDLE	TL_ip_recorded;

	int	accum_percent_[5];

	//	IP UDP/RTP
	int TL_CurrentBitrate;
	char TL_dest_ip[32];
	char TL_tx_multicast_ip[32];
	long TL_tx_udp_port;
	long TL_ip_protocol;
	long TL_snd_pull_ts;
	long TL_col_udp_off;
	long TL_col_off;
	long TL_col_na;
	char TL_src_ip[32];
	char TL_rx_multicast_ip[32];
	long TL_rx_udp_port;
	char TL_local_ip[32];
	long TL_fec_udp_off;
	long TL_fec_inact;
	int TL_gPCR_RestampingPre;
	int TL_gRestampingPre;

	HANDLE	m_hDeviceEvent[2];
	HANDLE	m_hEventThread;
	DWORD	m_EventThreadId;
	int		TL_nSupriseRemoval;

	__int64 TL_ip_PCR0;
	__int64 TL_ip_PCR1;
	int TL_ip_PCR0_pos;
	int TL_ip_PCR1_pos;
	WORD TL_ip_PID;
	int TL_ip_playrate_prev;	//	prev val of TL_ip_playrate_cur
	int TL_ip_playrate_cur;	//	ip playrate adjusted.

#if defined(WIN32)
	HANDLE TL_ip_hMutex;
	HANDLE TL_ip_hMutex_Cntrl;
#else
	pthread_mutex_t TL_ip_hMutex;
	pthread_mutexattr_t TL_ip_hMutexAttr;
	pthread_mutex_t TL_ip_hMutex_Cntrl;
	pthread_mutexattr_t TL_ip_hMutexAttr_Cntrl;
#endif

	int g_IP_InputBitrate;
	int g_IP_AverageInputBitrate;
	int g_IP_DemuxBitrate;
	int g_IP_AverageDemuxBitrate;

	int g_IP_ProgramCount;
	int g_IP_ProgramId[64];
	int g_IP_CurrentProgram;
	int g_IP_ProgramChanged;//-1=no changed, 0<= changed program #

	char g_VLC_Str[MAX_PATH];
	LONG	gDblStartOffsetL;
	LONG	gDblStartOffsetH;

	//2012/7/20 DVB-T2 ASI
	int g_CurrentPacketSize;


public:
	CHldFmtrIp(int _my_id, void *_hld);
	virtual ~CHldFmtrIp();

	void	SetCommonMethod_1(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	int CntBytesInIpBuf(void);
	unsigned int	CntIpLostPkts(void);
	int CntBytesIntialFillIpBuf(void);
	void	InitVariables_OnStartingIpTask(void);
	void	ClrVariables_OnKillIpTask(void);

	void	CreateIpMutex(void);
	void	DestroyIpMutex(void);
	void	LockIpMutex(void);
	void	UnlockIpMutex(void);
	void	CreateIpMutex_Cntrl(void);
	void	DestroyIpMutex_Cntrl(void);
	void	LockIpMutex_Cntrl(void);
	void	UnlockIpMutex_Cntrl(void);

	void TL_InitBufferVariables(void);
	int Init_Valiables_OnPlayStart(void);

	int IsSupriseRemoval(void);

	static	DWORD WINAPI DeviceEventThreadProc(LPVOID lpParam);
	void Finish_Vlc_OnUserMonReq(void);
	void Start_IpStreaming_OnUserReq(
		char *szFilePath,
		long nPlayrate,
		double dStartOffset,
		long nOption,
		long nRepeatMode,
		long nExtendedMode,
		char *szExtendedInfo);
	int Start_IpStreaming_OnUserReq_ExtCntl(
		char *szFilePath,
		long nPlayrate,
		double dStartOffset,
		long nOption,
		long nRepeatMode,
		long nExtendedMode,
		char *szExtendedInfo);
	void Finish_IpStreaming_OnUserMonReq(void);
	void Set_IpStreaming_TxBitrate(long play_freq_in_herz);
	void RunVlc_OnRecStart(void *lld);
	void RunVlc_OnPlayStart(void *lld);
	void SendIpStreaming_OnRecCont__HasEndlessWhile(int _ok_cap);
	void SendIpStreaming_OnPlayCont__HasEndlessWhile(void);
	int LaunchVlcCodec_and_WaitStop_OnPlayCont(void *lld);
	int CalcBitRateAtStreamingStart(void);
	int InitRcvIpEnv_OnPlayStart(int *_nBankBlockSize);

	void RestartVlc_OnUserReq(void *lld);
	void StopVlc_OnUserReq(void);
	long StsIpRx_OnUserReq(long nStatus);
	long SetIpProg_OnUserReq(long nIndex);
	long GetIpProg_OnUserReq(long nIndex);

	int VlcShowVid_OnUserReq(int nShowWindow);
	int VlcMoveVid_OnUserReq(int nX, int nY, int nW, int nH);
	int IsVlcVidVisible_OnUserReq(void);
	int SetVlcVidWin_OnUserReq(long hWnd);
	int CaclIpBitrateAdj_or_VlcCodecResize_OnPlayCont(void *lld);

	int FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile(void);
	int CalcedInTsBitRate(void);
	void CalcInTsBitRate(unsigned char *_rd_buf, int _rd_cnt);
	void CalcInTsBitRateBasedOnBytes(int _rd_cnt);
	void ProducerIpStreamingBuffer_2(unsigned char *_rd_buf, int _rd_cnt);

	int IsThere_EnoughIpDta(void);
	int Get_RcvIpDta(unsigned char *buff, int size);
	void CpInputIpDta_into_PlayBuf(void);
	int	SetRx_IpStreamingInfo(char* src_ip, char* rx_multicast_ip, long rx_udp_port, char* local_ip, long fec_udp_off, long fec_inact);
	int SetTx_IpStreamingInfo(char* dest_ip, char* tx_multicast_ip, long tx_udp_port, long ip_protocol, long snd_pull_ts, long col_udp_off, long col_off, long col_na);

	int Req_Sta_byUser_StartDelay(int nPort);
	int Req_Sta_byUser_StartMonitor(int nPort);
	int Req_Sta_byUser_StartPlay(char *szfnNewPlayFile, LONG PlayFreq, double DblStartOffset, long nUseMaxPlayrate, long fRepeatMode);
	int Req_Sta_byUser_StartRevord(char* szNewRecordFile, int nPort);

	void	CHECK_INIT_IP(void);
	void SetASIbufferStatus(int nVal);

#if defined(WIN32)
	void CHECK_LOCK(long a, long b);
	void CHECK_UNLOCK(long a);
#else
	void CHECK_LOCK(long a, long b);
	void CHECK_UNLOCK(long a);
#endif

	int IsBufStableCond(void);
	void UpdPktMonitorSta(int _cmd, unsigned int _size, int _insert, int _remove);

	int	LaunchIpBufProducer_Task(int *_bnk_size);
#ifdef WIN32
	static	void HLD_ip_PumpingThread(PVOID param);
#else
	static	void* HLD_ip_PumpingThread(PVOID param);
#endif


};


#endif

