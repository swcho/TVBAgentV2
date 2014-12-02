
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<fcntl.h>

#include	"../include/common_def.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"../include/hld_structure.h"
#include	"hld_gvar.h"
#ifdef WIN32
#else
#include <stdarg.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////
CHldGVar::CHldGVar(void)
{
	m_system_pkg_to_support_multi_rfout = 1;

	m_nBoardId = -1;

	my_stream_id_of_virtual_bd = 0;

	iHW_BANK_NUMBER = 7;
	iHW_BANK_OFFSET = 1024;

	TL_gCurrentModType	= 0;

	TL_InputDirection = 0;

	now_active_playing_task = 0;
	now_active_capturing_task = 0;

	TL_UseIP = 0;

	/* IP_STREAM, A/V CODEC */
	g_VLC_Ready = VLC_RUN_NO_READY;


	TL_fThreadState = TH_NONE;

	TL_fThreadActive = FALSE;
	ResetAllReqedState_User();
	AP_fKillThreadRequest = FALSE;

	TL_nIP_Initialized = 0;

	AP_fStartPauseRequest = 0;

	multits_1_hld = NULL;
	multits_2_hld = NULL;
	multits_3_hld = NULL;
	multits_4_hld = NULL;

	cnt_my_sub_ts_vsb = 0;
	cnt_my_sub_ts_qam = 0;
	//2012/6/28 multi dvb-t
	cnt_my_sub_ts_dvbt = 0;
	//2012/4/27
	OnNullTp = 0;
}

CHldGVar::~CHldGVar()
{
}
void	CHldGVar::SetCommonMethod_81(CHldBdLog	*__hLog__)
{
	_Prt	=	__hLog__;
}
void	CHldGVar::SetMyChildHld(void *_my_real, void *_1st_vir, void *_2nd_vir, void *_3rd_vir)
{
	multits_1_hld = _my_real;
	multits_2_hld = _1st_vir;
	multits_3_hld = _2nd_vir;
	multits_4_hld = _3rd_vir;
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldGVar::IsModTyp_Valid(int _typ)
{
	if ( _typ >= 0 && _typ <= 18 )
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_DvbT(void)
{
	if (TL_gCurrentModType == 0)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_Vsb(void)
{
	if (TL_gCurrentModType == 1)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_QamA(void)
{
	if (TL_gCurrentModType == 2)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_QamB(void)
{
	if (TL_gCurrentModType == 3)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_Qpsk(void)
{
	if (TL_gCurrentModType == 4)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_Tdmb(void)
{
	if (TL_gCurrentModType == 5)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_16Vsb(void)
{
	if (TL_gCurrentModType == 6)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_DvbH(void)
{
	if (TL_gCurrentModType == 7)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_DvbS2(void)
{
	if (TL_gCurrentModType == 8)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_IsdbT_1(void)
{
	if (TL_gCurrentModType == 9)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_IsdbT_13(void)
{
	if (TL_gCurrentModType == 10)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_Dtmb(void)
{
	if (TL_gCurrentModType == 11)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_Cmmb(void)
{
	if (TL_gCurrentModType == 12)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_DvbT2(void)
{
	if (TL_gCurrentModType == 13)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_xxxx(void)
{
	if (TL_gCurrentModType == 14)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_AtscMH(void)
{
	if (TL_gCurrentModType == 15)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_IqPlay(void)
{
	if (TL_gCurrentModType == 16)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_IsdbS(void)
{
	if (TL_gCurrentModType == 17)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsModTyp_DvbC2(void)
{
	if (TL_gCurrentModType == 18)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::is_new_playback()
{
	return (TL_gCurrentModType == 9 || TL_gCurrentModType == 13 || TL_gCurrentModType == 18);
}

//2012/3/16 BERT 187
int CHldGVar::is_Bert_187_Mod()
{
	if(TL_gCurrentModType == 5 || TL_gCurrentModType == 9 /*||TL_gCurrentModType == 10*/ || TL_gCurrentModType == 12 || TL_gCurrentModType == 13 || 
		TL_gCurrentModType == 15 ||	TL_gCurrentModType == 16/* ||	TL_gCurrentModType == 17 */|| TL_gCurrentModType == 18)
	{
		return 0;
	}
	return 1;
		
}
int CHldGVar::_CntAdditionalVsbRfOut_593_591s(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TL_gCurrentModType != 20)	//	TVB380_VSB8_MODE
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;

/*
	if (cnt_my_sub_ts_vsb <= 1)			return	0;	//	real only
	if (TL_gCurrentModType != 1)			return	0;	//	TVB380_VSB8_MODE
	return	cnt_my_sub_ts_vsb;
*/
}
int CHldGVar::_CntAdditionalQamRfOut_593_591s(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TL_gCurrentModType != 19)	//	TVB380_QAMB_MODE
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;

/*
	if (cnt_my_sub_ts_qam <= 1)			return	0;	//	real only
	if (TL_gCurrentModType != 3)			return	0;	//	TVB380_QAMB_MODE
	return	cnt_my_sub_ts_qam;
*/
}
//2012/6/28 multi dvb-t
int CHldGVar::_CntAdditionalDvbTRfOut_593(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TL_gCurrentModType != 21)	//	TVB380_MULTI_DVBT_MODE
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;
}


//////////////////////////////////////////////////////////////////////////////////////	state
void CHldGVar::SetFlag_CapLoopThru(unsigned int _val)
{
	now_active_capturing_task = _val;
}
unsigned int CHldGVar::Flag_CapLoopThru(void)
{
	return	now_active_capturing_task;
}
void CHldGVar::SetFlag_PlayLoopThru(unsigned int _val)
{
	now_active_playing_task = _val;
}
unsigned int CHldGVar::Flag_PlayLoopThru(void)
{
	return	now_active_playing_task;
}
void CHldGVar::SetFlag_DtaPathDirection(int _val)	//	TSIO_CUR_STATE__
{
	TL_InputDirection = _val;
//	_Prt->HldPrint_Tsio(TL_InputDirection);
}
int CHldGVar::Flag_DtaPathDirection(void)
{
	return	TL_InputDirection;
}
int CHldGVar::IsAsiLoopThru_DtaPathDirection(void)
{
	return	(TL_InputDirection == TSIO_ASI_LOOPTHRU);
}
int CHldGVar::IsAsior310_LoopThru_DtaPathDirection(void)
{
	return	((TL_InputDirection == TSIO_310_LOOPTHRU) || (TL_InputDirection == TSIO_ASI_LOOPTHRU));
}
int CHldGVar::IsAsior310_CapPlayLoopThru_DtaPathDirection(void)
{
	return	((TL_InputDirection == TSIO_310_CAPTURE_PLAY) || (TL_InputDirection == TSIO_ASI_CAPTURE_PLAY));
}

//////////////////////////////////////////////////////////////////////////////////////	user request
void CHldGVar::SetReqedStartPlay_User(int _val)
{
	AP_fStartPlayRequest = _val;
}
void CHldGVar::SetReqedStartRecord_User(int _val)
{
	AP_fStartRecordRequest = _val;	//	true when user requested rec. for isbdt13 or atsc-mh
}
void CHldGVar::SetReqedStartMonitor_User(int _val)
{
	AP_fStartMonitorRequest = _val;
}
void CHldGVar::SetReqedStartDelay_User(int _val)
{
//	AP_fStartDelayRequest = _val;
	AP_fStartPauseRequest = _val;
}

void CHldGVar::SetReqedKillTask_User(int _val)
{
	AP_fKillThreadRequest = _val;
}
int CHldGVar::ReqedKillTask_User(void)
{
	return	AP_fKillThreadRequest;
}

void CHldGVar::ResetAllReqedState_User(void)
{
	AP_fStartPlayRequest = FALSE;
	AP_fStartRecordRequest = FALSE;
	AP_fStartMonitorRequest = FALSE;
	AP_fStartDelayRequest = FALSE;
//	AP_fKillThreadRequest = FALSE;
}

int CHldGVar::ReqedStartPlay_User(void)
{
	return	AP_fStartPlayRequest;
}
int CHldGVar::ReqedStartRecord_User(void)
{
	return	AP_fStartRecordRequest;
}
int CHldGVar::ReqedStartMonitor_User(void)
{
	return	AP_fStartMonitorRequest;
}
int CHldGVar::ReqedStartDelay_User(void)
{
	//return	AP_fStartDelayRequest;
	return	AP_fStartPauseRequest;
}
int CHldGVar::ReqedNewAction_User(void)
{
	if (AP_fStartPlayRequest || AP_fStartRecordRequest || AP_fStartMonitorRequest || AP_fStartDelayRequest)
	{
		return	1;
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////	steady state
void CHldGVar::SetMainTask_LoopState_(int _val)
{
	_Prt->HldPrint_InternalState(TL_fThreadState, _val);
	TL_fThreadState = _val;
}
int CHldGVar::MainTask_LoopState(void)
{
	return	TL_fThreadState;
}
int CHldGVar::IsTaskState_StopCond_VlcPumping(void)
{
	if (ReqedNewAction_User() || AP_fKillThreadRequest)
	{
		return	1;
	}
	return	0;
}

int CHldGVar::IsTaskState_StartRec(void)
{
	if (TL_fThreadState == TH_START_REC)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_ContRec(void)
{
	if (TL_fThreadState == TH_CONT_REC)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_StopRec(void)
{
	if (TL_fThreadState == TH_STOP_REC)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_StartPlay(void)
{
	if (TL_fThreadState == TH_START_PLAY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_ContPlay(void)
{
	if (TL_fThreadState == TH_CONT_PLAY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_EndPlay(void)
{
	if (TL_fThreadState == TH_END_PLAY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_StartMon(void)
{
	if (TL_fThreadState == TH_START_MON)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskState_ContMon(void)
{
	if (TL_fThreadState == TH_CONT_MON)
	{
		return	1;
	}
	return	0;
}

int CHldGVar::DetermineMainTaskState_from_UserReq(void)
{
	if (AP_fStartPlayRequest)
	{
		SetMainTask_LoopState_(TH_START_PLAY);
	}
	else if (AP_fStartRecordRequest)
	{
		SetMainTask_LoopState_(TH_START_REC);
	}
	else if (AP_fStartMonitorRequest)
	{
		SetMainTask_LoopState_(TH_START_MON);
	}
	else if (AP_fStartDelayRequest)
	{
		SetMainTask_LoopState_(TH_START_DELAY);
	}

	if (ReqedNewAction_User())	//	what is it?
	{
		Sleep(100);	//	why?
	}
	ResetAllReqedState_User();

	if (TL_fThreadState == TH_NONE)
	{
		Sleep(10);
		return 0;	//	nothing to do. dummy loop
	}

	return	0xff;
}
//////////////////////////////////////////////////////////////////////////////////////	transition state
int CHldGVar::HappenedCond_StopRecording(void)
{
	if ((ReqedNewAction_User() || ReqedKillTask_User()) && IsTaskState_ContRec())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::HappenedCond_StopPlay(void)
{
	if ((ReqedNewAction_User() || ReqedKillTask_User()) && IsTaskState_ContPlay())
	{
		return	1;
	}
	return	0;
}
//////////////////////////////////////////////////////////////////////////////////////	ip-streaming state
void CHldGVar::Set_IPUsing(int _val)
{
	switch(TL_UseIP)	//	current state
	{
	case	0:	//	inactive and intial state.
		TL_UseIP = _val;
		break;

	case	1:	//	running
		switch(_val)
		{
		case	0:	//	inactivation user requested.
			if(IsModTyp_IsdbS())
			{
				TL_UseIP = 2;	//	isdb-s stop/transition condition.
			}
			else
			{
				TL_UseIP = _val;
			}
			break;

		case	1:
		default:
			TL_UseIP = _val;
			break;
		}
		break;

	case	2:	//	transition state
		TL_UseIP = _val;
		break;

	default:
		TL_UseIP = _val;
		break;
	}

}
int CHldGVar::IPUsing(void)
{
	switch(TL_UseIP)
	{
	case	1:
		return	1;	//	running
	case	2:
		return	2;	//	isdb-s stop/transition condition.
	}
	return	0;		//	inactive
}

int	CHldGVar::_VlcSta(void)
{
	return	g_VLC_Ready;
}

void CHldGVar::Set_VlcState(int _val)
{
	_Prt->HldPrint_VlcSta(g_VLC_Ready, _val);
	g_VLC_Ready = _val;
}
int CHldGVar::Vlc_HasBeenActivated(void)
{
	if (g_VLC_Ready != VLC_RUN_NO_READY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_SendIpStreaming(void)
{
	if (g_VLC_Ready == VLC_SEND_IP_STREAM)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_RecvIpStreaming(void)
{
	if (g_VLC_Ready == VLC_RECV_IP_STREAM)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_RecvIpStreamingRec(void)
{
	if (g_VLC_Ready == VLC_RECV_IP_STREAM_REC)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_FileDecodePlayback(void)
{
	if (g_VLC_Ready == VLC_RUN_FILE_DISPLAY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_AsiDecodePlayback(void)
{
	if (g_VLC_Ready == VLC_RUN_ASI_DISPLAY)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::Vlc_Recv_or_RecvRec(void)
{
	if (Vlc_RecvIpStreaming() || Vlc_RecvIpStreamingRec())
	{
		return	1;
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldGVar::IsAttachedTvbTyp_Usb(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	_BD_ID_595C__:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_CntlBitDiffDmaHostDirection(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
//2011/8/5 ISDB-S ASI
int CHldGVar::IsAttachedTvb590S_CntlBitDiffDmaHostDirection(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_590s__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_SupportIsdbTLoopThru(void)
{
	switch (m_nBoardId)
	{
//	case	_BD_ID_590v10_x__:
	case	_BD_ID_595Bv3__:
	case	_BD_ID_597__:
	case	0x3D:
	case	_BD_ID_597v2__:
//	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_SupportIsdbSLoopThru(void)
{
	switch (m_nBoardId)
	{
	//case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_SupportAtscMhLoopThru(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595Bv3__:
	case	_BD_ID_597__:
	case	0x3D:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_AllCase(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_390v8__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_591__:
	case	_BD_ID_594__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_594(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_594__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_591S(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_593(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_599(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_598(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_594_Virtual(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_594__:
		if (my_stream_id_of_virtual_bd != 0)
		{
			return	1;
		}
		break;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp__Virtual(void)
{
	if (my_stream_id_of_virtual_bd != 0)
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsAttachedTvbTyp_SupportMultiTs(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_594__:
	case	_BD_ID_591S__:
	case	_BD_ID_593__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldGVar::IsState_IsdbT13LoopThru(void)
{
	if (IsAttachedTvbTyp_SupportIsdbTLoopThru() && IsModTyp_IsdbT_13() && IsAsiLoopThru_DtaPathDirection())
	{
		return	1;
	}
	return	0;
}
//2011/8/4 ISDB-S ASI
int CHldGVar::IsState_IsdbSLoopThru(void)
{
	if (IsAttachedTvbTyp_SupportIsdbSLoopThru() && IsModTyp_IsdbS() && IsAsiLoopThru_DtaPathDirection())
	{
		return	1;
	}
	return	0;
}

int CHldGVar::IsState_AtscMhLoopThru(void)
{
	if (IsAttachedTvbTyp_SupportAtscMhLoopThru() && IsModTyp_AtscMH() && IsAsior310_CapPlayLoopThru_DtaPathDirection())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsCapMod_IsdbT13_or_AtscMH(void)
{	
	if (((IsModTyp_IsdbT_13() && IsAsiLoopThru_DtaPathDirection()) || (IsModTyp_AtscMH() && IsAsior310_CapPlayLoopThru_DtaPathDirection())) && Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsPlayMod_IsdbT13_or_AtscMH(void)
{
	if (((IsModTyp_IsdbT_13() && IsAsiLoopThru_DtaPathDirection()) || (IsModTyp_AtscMH() && IsAsior310_CapPlayLoopThru_DtaPathDirection())) && Flag_PlayLoopThru())
	{
		return	1;
	}
	return	0;
}
//2011/8/4 ISDB-S ASI
int CHldGVar::IsPlayMod_IsdbS(void)
{
	if ((IsModTyp_IsdbS() && IsAsiLoopThru_DtaPathDirection()) && Flag_PlayLoopThru())
	{
		return	1;
	}
	return	0;
}
//2011/8/4 ISDB-S ASI
int CHldGVar::IsCapMod_IsdbS(void)
{
	if ((IsModTyp_IsdbS() && IsAsiLoopThru_DtaPathDirection()) && Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}

//2012/7/6 DVB-T2 ASI ====================================================================================================
int CHldGVar::IsAttachedTvbTyp_SupportDvbT2LoopThru(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		if(IsModTyp_DvbT2())
			return	1;
	}
	return	0;
}
int CHldGVar::IsState_DvbT2LoopThru(void)
{
	if (IsAttachedTvbTyp_SupportDvbT2LoopThru() && IsModTyp_DvbT2() && IsAsior310_LoopThru_DtaPathDirection())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskStartingCond_DvbT2_Cap(void)
{
	if (IsModTyp_DvbT2() && !Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsCapMod_DvbT2(void)
{
	if (((IsModTyp_DvbT2() && IsAsiLoopThru_DtaPathDirection())) && Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
//=========================================================================================================================
//2012/7/6 DVB-C2 ASI ====================================================================================================
int CHldGVar::IsAttachedTvbTyp_SupportDvbC2LoopThru(void)
{
	switch (m_nBoardId)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		if(IsModTyp_DvbC2())
			return	1;
	}
	return	0;
}
int CHldGVar::IsState_DvbC2LoopThru(void)
{
	if (IsAttachedTvbTyp_SupportDvbC2LoopThru() && IsModTyp_DvbC2() && IsAsior310_LoopThru_DtaPathDirection())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskStartingCond_DvbC2_Cap(void)
{
	if (IsModTyp_DvbC2() && !Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsCapMod_DvbC2(void)
{
	if (((IsModTyp_DvbC2() && IsAsiLoopThru_DtaPathDirection())) && Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
//=========================================================================================================================


int CHldGVar::IsTaskStartingCond_IsdbT13_CapPlay(void)
{
	if (IsModTyp_IsdbT_13() && !Flag_PlayLoopThru() && !Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
int CHldGVar::IsTaskStartingCond_AtscMh_CapPlay(void)
{
	if (IsModTyp_AtscMH() && !Flag_PlayLoopThru() && !Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}
//2011/8/4 ISDB-S ASI
int CHldGVar::IsTaskStartingCond_IsdbS_CapPlay(void)
{
	if (IsModTyp_IsdbS() && !Flag_PlayLoopThru() && !Flag_CapLoopThru())
	{
		return	1;
	}
	return	0;
}

//2012/4/27
void CHldGVar::SetNullTP_User(int nullTp)
{
	OnNullTp = nullTp;
}
int  CHldGVar::IsNullTP_Enabled()
{
	if(OnNullTp == 1)
		return 1;
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////
/*^^***************************************************************************
 * Description : Check valid board ID
 *				
 * Entry : device handle
 *
 * Return: 
 *
 * Notes :  TSPL_nBoardTypeID == 
 *			TVB380V4==0x29, TVB390V6(No demand mode)==0x2A,
 *			TVB595V1,2==0x3B
 *			TVB590V9.2==0x2F
 *
 **************************************************************************^^*/
int CHldGVar::CHK_ID( int id, ... )
{
	int i = id;
	va_list marker;

	va_start( marker, id );     /* Initialize variable arguments. */
	//sskim20080229
	//while( i != -1 )
	while( i >= 0 )
	{
		i = va_arg( marker, int);
		//2010/10/4 PCI/USB MULTIBOARD
		if ( i < 0x0A || i > 0xFF )
		{
			break;
		}
		if ( m_nBoardId == i )
		{
			return 1;
		}
	}
	va_end( marker );              /* Reset variable arguments.      */

	return 0;
}

int CHldGVar::SET_PARAM( int flag, ... )
{
	_Prt->HldPrint_1_s("Donot use this : ", "SET_PARAM");
#if	0
	struct MOD_PARAM *pMod = &m_ModParam;
	int i = flag;
	pMod->dwFlag = i;

	va_list marker;
	va_start( marker, flag );     /* Initialize variable arguments. */
	while( i != -1 )
	{
		i = va_arg( marker, int);

		switch (pMod->dwFlag)
		{
			case 0:
				break;

			case 1://RF
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_RF = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_SymRate = i;
				return 0;

			case 2://SYMBOL
				//DVB-T, 8VSB, TDMB, 16VSB, DVB-H
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_RF = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_SymRate = i;
				return 0;

			case 3://CODERATE
				//8VSB, QAM-A,B, TDMB, 16VSB
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_CodeRate = i;
				return 0;

			case 4://BANDWIDTH
				//DVB-T, DVB-H
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_Bandwidth = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_RF = i;
				return 0;

			case 5://CONSTELLATION
				//8VSB, QPSK, TDMB, 16VSB
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_Constellation = i;
				return 0;

			case 6://TX, 
				//DVB-T, DVB-H
				pMod->dwTVB390_Type = i;

				//DVB-T
				if ( pMod->dwTVB390_Type == 0 )
				{
					i = va_arg( marker, int);
					pMod->dwTVB390_TxMode = i;
				}
				else
				{
					i = va_arg( marker, int);
					pMod->dwTVB390_TxMode = i;

					i = va_arg( marker, int);
					pMod->dwTVB390_IN_DEPTH = i;

					i = va_arg( marker, int);	
					pMod->dwTVB390_TIME_SLICE = i;
						
					i = va_arg( marker, int);	
					pMod->dwTVB390_MPE_FEC = i;

					i = va_arg( marker, int);
					pMod->dwTVB390_CELL_ID = i;
				}
				return 0;

			case 7://GUARDINTERVAL
				//DVB-T, DVB-H
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_GuardInterval = i;
				return 0;

			case 8://INTERLEAVE
				//QAM-B
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_Interleave = i;
				return 0;
			
			case 9://IF
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_IF = i;
				return 0;
			
			case 10://SPECTRUM_INVERSION
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_SpectrumInversion = i;
				return 0;

			case 11://ATTEN.
				pMod->dwTVB390_Type = i;

				//fixed, atten. ==> "double type"
				//i = va_arg( marker, int);
				pMod->dwTVB390_Atten = va_arg( marker, double);
				return 0;

			case 12://PRBS MODE
			case 13://PRBS SCALE
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_PRBS_Mode = i;
	
				i = va_arg( marker, int);
				pMod->dwTVB390_PRBS_Scale = i;
				return 0;

			case 14://MPE FEC
			case 15://TIME SLICE
			case 16://IN DEPTH
			case 17://CELL ID
				//DVB-T, DVB-H
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_TxMode = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_IN_DEPTH = i;

				i = va_arg( marker, int);	
				pMod->dwTVB390_TIME_SLICE = i;
					
				i = va_arg( marker, int);	
				pMod->dwTVB390_MPE_FEC = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_CELL_ID = i;
				return 0;
			
			case 18://PILOT
				//DVB-S2
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_Pilot = i;
				return 0;

			case 19://ROLL-OFF FACTOR
				//DVB-S2
				pMod->dwTVB390_Type = i;

				i = va_arg( marker, int);
				pMod->dwTVB390_Roll_Off_Factor = i;
				return 0;

			default:
				break;
		}
	}
	va_end( marker );              /* Reset variable arguments.      */
#endif

	return 0;
}


int	CHldGVar::TL_ChangeParams(void *pParam)
{
	_Prt->HldPrint_1_s("Donot use this : ", "TL_ChangeParams");
#if	0
	struct MOD_PARAM	*pMod;
	pMod = (struct MOD_PARAM *) pParam;

	switch (pMod->dwFlag)
	{
		case 0:
			break;

		case 1://RF
			TVB380_SET_MODULATOR_FREQ(pMod->dwTVB390_Type, 
				pMod->dwTVB390_RF, 
				pMod->dwTVB390_SymRate);
			break;

		case 2://SYMBOL
			//DVB-T, 8VSB, TDMB, 16VSB, DVB-H
			TVB380_SET_MODULATOR_SYMRATE(pMod->dwTVB390_Type, 
				pMod->dwTVB390_RF, 
				pMod->dwTVB390_SymRate);
			break;

		case 3://CODERATE
			//8VSB, QAM-A,B, TDMB, 16VSB
			TVB380_SET_MODULATOR_CODERATE(pMod->dwTVB390_Type, 
				pMod->dwTVB390_CodeRate);
			break;

		case 4://BANDWIDTH
			//DVB-T, DVB-H
			TVB380_SET_MODULATOR_BANDWIDTH(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Bandwidth, 
				pMod->dwTVB390_RF);
			break;

		case 5://CONSTELLATION
			//8VSB, QPSK, TDMB, 16VSB
			TVB380_SET_MODULATOR_CONSTELLATION(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Constellation);
			break;

		case 6://TX
			//DVB-T, DVB-H
			//DVB-T
			if ( pMod->dwTVB390_Type == 0 )
			{
				TVB380_SET_MODULATOR_TXMODE(pMod->dwTVB390_Type, 
					pMod->dwTVB390_TxMode);
			}
			else
			{
				TVB380_SET_MODULATOR_DVBH(pMod->dwTVB390_Type,
					pMod->dwTVB390_TxMode,
					pMod->dwTVB390_IN_DEPTH,
					pMod->dwTVB390_TIME_SLICE,
					pMod->dwTVB390_MPE_FEC,
					pMod->dwTVB390_CELL_ID);
			}
			break;

		case 7://GUARDINTERVAL
			//DVB-T, DVB-H
			TVB380_SET_MODULATOR_GUARDINTERVAL(pMod->dwTVB390_Type, 
				pMod->dwTVB390_GuardInterval);
			break;

		case 8://INTERLEAVE
			//QAM-B
			TVB380_SET_MODULATOR_INTERLEAVE(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Interleave);
			break;
		
		case 9://IF
			TVB380_SET_MODULATOR_IF_FREQ(pMod->dwTVB390_Type, 
				pMod->dwTVB390_IF);
			break;
		
		case 10://SPECTRUM_INVERSION
			TVB380_SET_MODULATOR_SPECTRUM_INVERSION(pMod->dwTVB390_Type, 
				pMod->dwTVB390_SpectrumInversion);
			break;

		case 11://ATTEN.
			TVB380_SET_MODULATOR_ATTEN_VALUE(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Atten, 0);
			break;

		case 12://PRBS MODE
			//TDMB
			if ( pMod->dwTVB390_Type == 5 )
			{
				TVB380_SET_MODULATOR_PRBS_MODE(pMod->dwTVB390_Type, 
					pMod->dwTVB390_PRBS_Mode);
			}
			else
			{
				TVB380_SET_MODULATOR_PRBS_INFO(pMod->dwTVB390_Type, 
					pMod->dwTVB390_PRBS_Mode, 
					pMod->dwTVB390_PRBS_Scale);
			}
			break;
		case 13://PRBS SCALE
			//TDMB
			if ( pMod->dwTVB390_Type == 5 )
			{
				TVB380_SET_MODULATOR_PRBS_SCALE(pMod->dwTVB390_Type, 
					pMod->dwTVB390_PRBS_Scale);
			}
			else
			{
				TVB380_SET_MODULATOR_PRBS_INFO(pMod->dwTVB390_Type, 
					pMod->dwTVB390_PRBS_Mode, 
					pMod->dwTVB390_PRBS_Scale);
			}
			break;

		case 14://MPE FEC
		case 15://TIME SLICE
		case 16://IN DEPTH
		case 17://CELL ID
			//DVB-T, DVB-H
			TVB380_SET_MODULATOR_DVBH(pMod->dwTVB390_Type,
				pMod->dwTVB390_TxMode,
				pMod->dwTVB390_IN_DEPTH,
				pMod->dwTVB390_TIME_SLICE,
				pMod->dwTVB390_MPE_FEC,
				pMod->dwTVB390_CELL_ID);
			break;

		case 18://PILOT
			//DVB-S2
			TVB380_SET_MODULATOR_PILOT(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Pilot);
			break;

		case 19://ROLL-OFF FACTOR
			//DVB-S2
			TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(pMod->dwTVB390_Type, 
				pMod->dwTVB390_Roll_Off_Factor);
			break;

		default:
			break;
	}
#endif

	return S_OK;
}

int CHldGVar::SetModulatorTyp(long type)
{
	TL_gCurrentModType = type;
	_Prt->HldPrint_ModtrTyp(TL_gCurrentModType);

	return 0;
}
int CHldGVar::GetCurTaskSta(void)
{
	return TL_fThreadState;
}

void CHldGVar::SetT2MI_Stream_Generation(int _val)
{
	TL_T2MI_StreamGeneration = _val;
}
int  CHldGVar::IsT2MI_Stream_Generation()
{
	return TL_T2MI_StreamGeneration;
}
void CHldGVar::SetT2MI_OutputFileName(char *filename)
{
	strcpy(OutputFileName, filename);
}
char *CHldGVar::GetT2MI_OutputFileName()
{
	return OutputFileName;
}


