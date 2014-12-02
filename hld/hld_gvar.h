
#ifndef __HLD_GVAR_H__
#define __HLD_GVAR_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#include 	"hld_type.h"
#endif

#include	"local_def.h"
#include	"../include/hld_const.h"
#include	"hld_bd_log.h"

#if defined(WIN32)
#else
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef long long __int64;
#endif

#define MAX_PID_SIZE (0x1FFF+1)

#define	NUM_PCCC_COMP		6
#define NUM_OF_TPC			12
#define NUM_OF_DECODERLOOP	(NUM_OF_TPC/3)
#define NUM_OF_PCCCDATA		(NUM_OF_TPC << 1)	
#define PKTSIZE				188
#define PKT_PER_SLOT		156


#define TS_PACKET_SIZE	188
#define MAX_PID_SIZE	(0x1FFF+1)
#define	MAX_PID_SEARCH	16
#define	MAX_PID_TBL		32
#define	MAX_PID_SEARCH_RANGE SUB_BANK_MAX_BYTE_SIZE*16
#define	MAX_REPEAT_TIME 10
#define PID_DB_PATH		"ts_info"


struct ts
{
	unsigned char Data[204];
};
struct layer
{
	int Constel;	// Carrier Modulation Scheme
	int CodRate;	// Convolutional Coding Rate
	int NumSegm;	// Number of Segment

	int Bps;
	int TpPerFrame;
	int Time_Interleaving;//0(Mode 1),0(2),0(3), 
						  //4(Mode 1),2(2),1(3), 
						  //8(Mode 1),4(2),2(3)
						  //16(Mode 1),8(2),4(3)
};



// TVB595V1
struct MOD_PARAM
{
	//DWORD	dwModulation_Mode;
	DWORD	dwInput;
	DWORD	dwFreq;
	DWORD	dwSymbol;
	DWORD	dwCoderate;
	DWORD	dwTVB390_Type;
	DWORD	dwTVB390_RF;
	double	dwTVB390_Atten;//fixed, DWORD->double
	DWORD	dwTVB390_SymRate;
	DWORD	dwTVB390_Bandwidth;
	DWORD	dwTVB390_Constellation;
	DWORD	dwTVB390_CodeRate;
	DWORD	dwTVB390_TxMode;
	DWORD	dwTVB390_GuardInterval;
	DWORD	dwTVB390_Interleave;
	DWORD	dwTVB390_IF;
	DWORD	dwTVB390_SpectrumInversion;
	DWORD	dwTVB390_PRBS_Mode;
	//DWORD	dwTVB390_PRBS_Scale;
	double	dwTVB390_PRBS_Scale;
	DWORD	dwTVB390_MPE_FEC;
	DWORD	dwTVB390_TIME_SLICE;
	DWORD	dwTVB390_IN_DEPTH;
	DWORD	dwTVB390_CELL_ID;
	DWORD	dwTVB390_Pilot;
	DWORD	dwTVB390_Roll_Off_Factor;

	int		dwFlag;
};


class CHldGVar
{
private:
	CHldBdLog	*_Prt;
	int m_system_pkg_to_support_multi_rfout;

public:
	int m_nBoardId;
	int m_nBoardLocation;	//	same # as _CBdSysConf->_bd_cnxt[__id__].__id__
	int my_stream_id_of_virtual_bd;	//	_CBdSysConf->_bd_cnxt[__id__].__id_ts_seq__
	int	cnt_my_sub_ts_vsb;
	int	cnt_my_sub_ts_qam;
	//2012/6/28 multi dvb-t
	int	cnt_my_sub_ts_dvbt;

	int	iHW_BANK_NUMBER;//0~7
	int iHW_BANK_OFFSET;//1~1024

	int TL_gCurrentModType;

	int TL_InputDirection;

	//2012/11/09
	int Start_Output_flag;

	unsigned int now_active_playing_task;		//	thread flag. self-indicate of running.
	unsigned int now_active_capturing_task;	//	thread flag. self-indicate of running.
	
//	IP_STREAM, A/V CODEC
	int g_VLC_Ready;
	int TL_nIP_Initialized;

	int TL_UseIP;	//	user requested ip activation.

// State machine related variables
	int 	TL_fThreadActive;
	int 	TL_fThreadState;
	int 	AP_fStartPlayRequest;
	int 	AP_fStartRecordRequest;
	int 	AP_fStartMonitorRequest;
	int 	AP_fStartDelayRequest;
	int 	AP_fKillThreadRequest;

	//2011/10/24 added PAUSE 
	int		AP_fStartPauseRequest;

	struct	MOD_PARAM m_ModParam;
	int 	m_nApply;

	void	*multits_1_hld;	//	real slot
	void	*multits_2_hld;	//	1st-virtual
	void	*multits_3_hld;
	void	*multits_4_hld;

	//2012/4/27
	int		OnNullTp;

	//2013/1/23 T2MI stream generation
	int		TL_T2MI_StreamGeneration;
	char	OutputFileName[512];

public:
	CHldGVar(void);
	virtual ~CHldGVar();

	void	SetCommonMethod_81(CHldBdLog	*__hLog__);
	void	SetMyChildHld(void *_my_real, void *_1st_vir, void *_2nd_vir, void *_3rd_vir);
	int IsModTyp_Valid(int _typ);
	int	IsModTyp_DvbT(void);
	int	IsModTyp_Vsb(void);
	int	IsModTyp_QamA(void);
	int	IsModTyp_QamB(void);
	int	IsModTyp_Qpsk(void);
	int	IsModTyp_Tdmb(void);
	int	IsModTyp_16Vsb(void);
	int	IsModTyp_DvbH(void);
	int	IsModTyp_DvbS2(void);
	int	IsModTyp_IsdbT_1(void);
	int	IsModTyp_IsdbT_13(void);
	int	IsModTyp_Dtmb(void);
	int	IsModTyp_Cmmb(void);
	int	IsModTyp_DvbT2(void);
	int	IsModTyp_xxxx(void);
	int	IsModTyp_AtscMH(void);
	int	IsModTyp_IqPlay(void);
	int	IsModTyp_IsdbS(void);
	int	IsModTyp_DvbC2(void);
	int	is_new_playback();
	//2012/3/16 BERT 187
	int is_Bert_187_Mod(void);
	int	_CntAdditionalVsbRfOut_593_591s(void);
	int	_CntAdditionalQamRfOut_593_591s(void);
	//2012/6/28 multi dvb-t
	int	_CntAdditionalDvbTRfOut_593(void);

	void	SetFlag_CapLoopThru(unsigned int _val);
	unsigned int Flag_CapLoopThru(void);
	void	SetFlag_PlayLoopThru(unsigned int _val);
	unsigned int Flag_PlayLoopThru(void);
	void SetFlag_DtaPathDirection(int _val);	//	TSIO_CUR_STATE__
	int Flag_DtaPathDirection(void);
	int IsAsiLoopThru_DtaPathDirection(void);
	int IsAsior310_LoopThru_DtaPathDirection(void);
	int IsAsior310_CapPlayLoopThru_DtaPathDirection(void);

	void SetReqedStartPlay_User(int _val);
	void SetReqedStartRecord_User(int _val);
	void SetReqedStartMonitor_User(int _val);
	void SetReqedStartDelay_User(int _val);

	void SetReqedKillTask_User(int _val);
	int	ReqedKillTask_User(void);
	void ResetAllReqedState_User(void);

	int ReqedStartPlay_User(void);
	int ReqedStartRecord_User(void);
	int ReqedStartMonitor_User(void);
	int ReqedStartDelay_User(void);

	int ReqedNewAction_User(void);

	void SetMainTask_LoopState_(int _val);
	int MainTask_LoopState(void);
	int IsTaskState_StopCond_VlcPumping(void);

	int IsTaskState_StartRec(void);
	int IsTaskState_ContRec(void);
	int IsTaskState_StopRec(void);
	int IsTaskState_StartPlay(void);
	int IsTaskState_ContPlay(void);
	int IsTaskState_EndPlay(void);
	int IsTaskState_StartMon(void);
	int IsTaskState_ContMon(void);
	int DetermineMainTaskState_from_UserReq(void);

	int HappenedCond_StopRecording(void);
	int HappenedCond_StopPlay(void);

	void Set_IPUsing(int _val);
	int IPUsing(void);
	int _VlcSta(void);
	void Set_VlcState(int _val);
	int Vlc_HasBeenActivated(void);
	int Vlc_SendIpStreaming(void);
	int Vlc_RecvIpStreaming(void);
	int Vlc_RecvIpStreamingRec(void);
	int Vlc_FileDecodePlayback(void);
	int Vlc_AsiDecodePlayback(void);
	int Vlc_Recv_or_RecvRec(void);

	int IsAttachedTvbTyp_Usb(void);
	int IsAttachedTvbTyp_CntlBitDiffDmaHostDirection(void);
	int IsAttachedTvbTyp_SupportIsdbTLoopThru(void);
	int IsAttachedTvbTyp_SupportAtscMhLoopThru(void);
	int IsAttachedTvbTyp_AllCase(void);
	int IsAttachedTvbTyp_594(void);
	int IsAttachedTvbTyp_591S(void);
	int IsAttachedTvbTyp_593(void);
	int IsAttachedTvbTyp_599(void);
	int IsAttachedTvbTyp_598(void);
	int IsAttachedTvbTyp_594_Virtual(void);
	int IsAttachedTvbTyp__Virtual(void);
	int IsAttachedTvbTyp_SupportMultiTs(void);

	int IsState_IsdbT13LoopThru(void);
	int IsState_AtscMhLoopThru(void);
	int	IsCapMod_IsdbT13_or_AtscMH(void);
	int IsPlayMod_IsdbT13_or_AtscMH(void);
	int IsTaskStartingCond_IsdbT13_CapPlay(void);
	int IsTaskStartingCond_AtscMh_CapPlay(void);
	//2011/8/4 ISDB-S ASI
	int IsAttachedTvbTyp_SupportIsdbSLoopThru(void);
	int IsState_IsdbSLoopThru(void);
	int IsTaskStartingCond_IsdbS_CapPlay(void);
	int	IsCapMod_IsdbS(void);
	int IsPlayMod_IsdbS(void);
	int IsAttachedTvb590S_CntlBitDiffDmaHostDirection(void);
	//2012/7/6 DVB-T2 ASI
	int IsState_DvbT2LoopThru(void);
	int IsAttachedTvbTyp_SupportDvbT2LoopThru(void);
	int IsTaskStartingCond_DvbT2_Cap(void);
	int	IsCapMod_DvbT2(void);
	//2012/7/24 DVB-C2 ASI
	int IsState_DvbC2LoopThru(void);
	int IsAttachedTvbTyp_SupportDvbC2LoopThru(void);
	int IsTaskStartingCond_DvbC2_Cap(void);
	int	IsCapMod_DvbC2(void);

	int CHK_ID(int id, ... );
	int SET_PARAM(int flag, ...);
	int TL_ChangeParams(void *pParam);

	int SetModulatorTyp(long type);
	int GetCurTaskSta(void);

	//2012/4/27
	void SetNullTP_User(int nullTp);
	int  IsNullTP_Enabled();

	void SetT2MI_Stream_Generation(int _val);
	int  IsT2MI_Stream_Generation();
	void SetT2MI_OutputFileName(char *filename);
	char *GetT2MI_OutputFileName();
};


#endif

