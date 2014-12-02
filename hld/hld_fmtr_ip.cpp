
#if defined(WIN32)
#include	<Windows.h>
#include	<process.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include	<stdio.h>

#include	"../include/hld_structure.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"hld_fmtr_ip.h"

#include	"LLDWrapper.h"
#include	"VLCWrapper.h"

#if defined(WIN32)
extern VLCWrapper *g_VLCWrapper;	//	????
#else
static VLCWrapper *g_VLCWrapper;	//	????
#endif

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrIp::CHldFmtrIp(int _my_id, void *_hld)	:	CHldFmtrIsdbS(_my_id, _hld),	CHldMultiRfOut(_my_id, _hld)
{
	my_hld_id = _my_id;
	my_hld = _hld;

	prev_max_playrate_sta = 0;

	//IP UDP/RTP
	TL_ip_recorded = INVALID_HANDLE_VALUE;

	TL_ip_read = TL_ip_write = TL_ip_count = 0;
	TL_ip_buffer = NULL;
	ip_tmp_buf = NULL;


#if defined(WIN32)
	TL_ip_hMutex = INVALID_HANDLE_VALUE;
	TL_ip_hMutex_Cntrl = INVALID_HANDLE_VALUE;
#else
#endif

	g_IP_InputBitrate = 0;
	g_IP_AverageInputBitrate = 0;
	g_IP_DemuxBitrate = 0;
	g_IP_AverageDemuxBitrate = 0;

	g_IP_ProgramCount = 0;
	g_IP_CurrentProgram = -1;
	g_IP_ProgramChanged = -1;


	memset(g_VLC_Str, 0, MAX_PATH);

	gDblStartOffsetL = 0;
	gDblStartOffsetH = 0;

	TL_nSupriseRemoval = 0;
	under_over_vector = 1;
	inc_dec_cnt = 0;
}

CHldFmtrIp::~CHldFmtrIp()
{
	if ( TL_ip_buffer != NULL )
	{
		free(TL_ip_buffer);
	}
	if (ip_tmp_buf != NULL)
	{
		free(ip_tmp_buf);
	}
	DestroyIpMutex();
	DestroyIpMutex_Cntrl();

}
void	CHldFmtrIp::SetCommonMethod_1(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void	CHldFmtrIp::CreateIpMutex(void)
{
#if defined(WIN32)
	TL_ip_hMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_ip_hMutexAttr);
	pthread_mutexattr_settype(&TL_ip_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_ip_hMutex, &TL_ip_hMutexAttr);
#endif

}
void	CHldFmtrIp::DestroyIpMutex(void)
{
#if defined(WIN32)	
	if ( TL_ip_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_ip_hMutex);
		TL_ip_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_ip_hMutexAttr);
	pthread_mutex_destroy(&TL_ip_hMutex);
#endif
}
void	CHldFmtrIp::LockIpMutex(void)
{
#if defined(WIN32)
	if ( TL_ip_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_ip_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_ip_hMutex);
#endif
}
void	CHldFmtrIp::UnlockIpMutex(void)
{
#if defined(WIN32)
	if ( TL_ip_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_ip_hMutex);
	}
#else
	pthread_mutex_unlock(&TL_ip_hMutex);
#endif
}
void	CHldFmtrIp::CreateIpMutex_Cntrl(void)
{
#if defined(WIN32)
	TL_ip_hMutex_Cntrl = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_ip_hMutexAttr_Cntrl);
	pthread_mutexattr_settype(&TL_ip_hMutexAttr_Cntrl, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_ip_hMutex_Cntrl, &TL_ip_hMutexAttr_Cntrl);
#endif

}
void	CHldFmtrIp::DestroyIpMutex_Cntrl(void)
{
#if defined(WIN32)	
	if ( TL_ip_hMutex_Cntrl != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_ip_hMutex_Cntrl);
		TL_ip_hMutex_Cntrl = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_ip_hMutexAttr_Cntrl);
	pthread_mutex_destroy(&TL_ip_hMutex_Cntrl);
#endif
}
void	CHldFmtrIp::LockIpMutex_Cntrl(void)
{
#if defined(WIN32)
	if ( TL_ip_hMutex_Cntrl != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_ip_hMutex_Cntrl, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_ip_hMutex_Cntrl);
#endif
}
void	CHldFmtrIp::UnlockIpMutex_Cntrl(void)
{
#if defined(WIN32)
	if ( TL_ip_hMutex_Cntrl != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_ip_hMutex_Cntrl);
	}
#else
	pthread_mutex_unlock(&TL_ip_hMutex_Cntrl);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldFmtrIp::CntBytesInIpBuf(void)
{
	return	TL_ip_count;
}
unsigned int	CHldFmtrIp::CntIpLostPkts(void)
{
	return	get_eth_pkt_lost_cnt();
}

int	CHldFmtrIp::CntBytesIntialFillIpBuf(void)
{
	return	TL_ip_bfp;
}
void	CHldFmtrIp::InitVariables_OnStartingIpTask(void)
{
	CreateIpMutex();
	CreateIpMutex_Cntrl();

#ifdef STANDALONE
	printf("call InitVariables_OnStartingIpTask\n");
#endif
	TL_ip_read = TL_ip_write = TL_ip_count = 0;
	TL_ip_buffer = (unsigned char*)malloc(G_BUFFER_SIZE);
#ifdef STANDALONE
	if(TL_ip_buffer == NULL)
		printf("fail malloc TL_ip_buffer\n");
#endif
	ip_tmp_buf = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);
#ifdef STANDALONE
	if(ip_tmp_buf == NULL)
		printf("fail malloc TL_ip_buffer\n");
#endif
	TL_ip_PumpingThreadDone = 0;

	TL_ip_PCR0 = 0;
	TL_ip_PCR1 = 0;
	TL_ip_PCR0_pos = 0;
	TL_ip_PCR1_pos = 0;
	TL_ip_PID = 0;

	TL_ip_playrate_prev = -1;
	TL_ip_playrate_cur = -1;

	ip_pkt_monotor_sta = __IP_STATE_buf_fill_wait__;
	cnt_ip_recv_pkt_bytes = 0;
	cnt_ip_wr_hw_pkt_bytes = 0;
	ip_pkt_monotor_orientation = 0;
}
void	CHldFmtrIp::ClrVariables_OnKillIpTask(void)
{
	if ( TL_ip_buffer != NULL )
	{
#ifdef STANDALONE
	printf("free TL_ip_buffer\n");
#endif
		free(TL_ip_buffer);
		TL_ip_buffer = NULL;
	}
	if (ip_tmp_buf != NULL)
	{
#ifdef STANDALONE
	printf("free ip_tmp_buf\n");
#endif
		free(ip_tmp_buf);
		ip_tmp_buf = NULL;
	}
	DestroyIpMutex();
	DestroyIpMutex_Cntrl();
	TL_ip_PumpingThreadDone = 1;
#ifdef STANDALONE
	printf("end this function ClrVariables_OnKillIpTask\n");
#endif

}

//--------------------------------------------------------------------------
//	
//	Initialize Varialbes for Converting 204/208 Packet to 188 Packet Stream
//
void CHldFmtrIp::TL_InitBufferVariables(void)
{
	__FIf__->InitPlayAnd2ndPlayBufVariables();
	InitIsdbSPlayBufVariables();
}

int	CHldFmtrIp::Init_Valiables_OnPlayStart(void)
{
	__FIf__->InitVariables_OnPlay();

	/* 204/208TS to 188TS CONVERSION */
	__Sta__->TL_nIP_Initialized = 0;
	TL_InitBufferVariables();

	/* TDMB ONLY. NI/NA TO CIF CONVERSION. */
	InitializeNAToNI(1);

	return 1;
}

int CHldFmtrIp::IsSupriseRemoval(void)
{
	return	TL_nSupriseRemoval;
}


//////////////////////////////////////////////////////////////////////////////////////
// TVB595V1
#ifdef	TSPHLD_VLC
extern int VLC_Stop(void);
#endif
DWORD CHldFmtrIp::DeviceEventThreadProc(LPVOID lpParam)
{
	CHld *lpThis = (CHld*)lpParam;
#if defined(WIN32)	
	if ( lpThis->m_hDeviceEvent[0] != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(lpThis->m_hDeviceEvent[0], INFINITE);
		lpThis->TL_nSupriseRemoval = 1;

		//IP_STREAM, A/V CODEC
#ifdef	TSPHLD_VLC
		if ( !lpThis->_SysSta->IPUsing() && lpThis->_SysSta->Vlc_HasBeenActivated() )
		{
#if defined(WIN32)
			g_VLCWrapper->VLC_Stop();
#else
#endif
			lpThis->_SysSta->g_VLC_Ready = 0;
		}
#endif
		lpThis->_FIf->TL_CheckBufferStatus(0, lpThis->_FIf->GetPlayParam_PlayRate());
//		HldPrint("Hld-Bd-Ctl. TSPH START MONITOR");
		lpThis->_SysSta->SetReqedStartMonitor_User(TRUE);
#if 1
		ExitProcess(1);
		return 0;
#endif
	}
#else
#endif
	return 0;
}

void CHldFmtrIp::Finish_Vlc_OnUserMonReq(void)
{
#ifdef	TSPHLD_VLC
	if (g_VLCWrapper && __Sta__->Vlc_HasBeenActivated())
	{
#if defined(WIN32)
		g_VLCWrapper->VLC_Stop();
#else
#endif
		__Sta__->Set_VlcState(VLC_RUN_NO_READY);
	}
#endif
	__FIf__->TL_CheckBufferStatus(0, __FIf__->GetPlayParam_PlayRate());
}

void CHldFmtrIp::Start_IpStreaming_OnUserReq(
	char *szFilePath,
	long nPlayrate,
	double dStartOffset,
	long nOption,
	long nRepeatMode,
	long nExtendedMode,
	char *szExtendedInfo)
{
	unsigned long	my_ip, src_ip, mcast_ip;
	int port_num = DEFAULT_PORT;
	
	if ( strstr(szExtendedInfo, "dst=display") )
	{
		LockIpMutex_Cntrl();
		__Sta__->Set_IPUsing(0);
		UnlockIpMutex_Cntrl();
	}
	else
	{
		LockIpMutex_Cntrl();
		__Sta__->Set_IPUsing(1);
		UnlockIpMutex_Cntrl();
		prev_max_playrate_sta = __FIf__->cur_max_playrate_sta;
//		__FIf__->TSPL_SET_MAX_PLAYRATE(__Sta__->TL_gCurrentModType, 1);
	}

	if ( __Sta__->IPUsing() )
	{
		if ( nExtendedMode == VLC_SEND_IP_STREAM )
		{
			__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "Start_IpStreaming_OnUserReq : VLC_SEND_IP_STREAM");
		}
		else if ( nExtendedMode == VLC_RECV_IP_STREAM || nExtendedMode == VLC_RECV_IP_STREAM_REC )
		{
			my_ip = inet_addr(TL_local_ip);
			src_ip = inet_addr(TL_src_ip);
			mcast_ip = inet_addr(TL_rx_multicast_ip);
			port_num = TL_rx_udp_port;
			run_ip_streaming_srv(my_ip, src_ip, mcast_ip, port_num, (int)TL_fec_udp_off, (int)TL_fec_inact);
			//2014/01/09 IP Delay Time
			SetDelayTime_Millisecond(1000);

			if ( nExtendedMode == VLC_RECV_IP_STREAM_REC )
			{
				__FIf__->OpenFile_RecMode_1(&TL_ip_recorded, szFilePath);
			}

			TL_gPCR_RestampingPre = __FIf__->TL_gPCR_Restamping;
			TL_gRestampingPre = __FIf__->TL_gRestamping;
		}
	}
	else
	{
#if defined(WIN32)
		if ( g_VLCWrapper == NULL )
		{
			g_VLCWrapper = new VLCWrapper;
		}
#else
#endif
	}
}
int CHldFmtrIp::Start_IpStreaming_OnUserReq_ExtCntl(
	char *szFilePath,
	long nPlayrate,
	double dStartOffset,
	long nOption,
	long nRepeatMode,
	long nExtendedMode,
	char *szExtendedInfo)
{
	int	fi;
	char szDummyFilepath[MAX_PATH];

	__FIf__->CreateDummtFile(szDummyFilepath);

	switch (nExtendedMode)
	{
	case VLC_RECV_IP_STREAM:
	case VLC_RECV_IP_STREAM_REC:
		{
		if ( __Sta__->IPUsing() )
		{
		}
		else if ( !szExtendedInfo || strlen(szExtendedInfo) <= 0 )
		{
			__HLog__->HldPrint("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, INPUT target may be empty.");
			return -30;//TLV_FAIL_TO_START_IP_STREAMING
		}

		g_IP_ProgramCount = 0;
		g_IP_CurrentProgram = -1;
		g_IP_ProgramChanged = -1;
		__Sta__->Set_VlcState(nExtendedMode);
		sprintf(g_VLC_Str, "%s", szExtendedInfo);

		if ( nExtendedMode == VLC_RECV_IP_STREAM_REC)
			__FIf__->Set_RecFileName(szFilePath);
		else
			__FIf__->Set_RecFileName("");

		__FIf__->SetPlayParam_FName(szDummyFilepath);
		if(__FIf__->GetPlayParam_PlayRate() <= 0L)
		{
			__FIf__->SetPlayParam_PlayRate(19392658L);
		}
		if ( (nPlayrate == 0L) || (nPlayrate == -1L) || (nPlayrate < 1L) || (nPlayrate > 90000000L) )
		{
			nPlayrate = 19392658L;
		}
		__FIf__->SetPlayParam_PlayRate(nPlayrate);
#ifdef	__USE_MAX_PLAYRATE_IP_PLAYBACK__
		__FIf__->SetPlayParam_OutClkSrc(1);
#else
		if (__Sta__->IsModTyp_IsdbS() || __Sta__->IsModTyp_DvbT2() || __Sta__->IsModTyp_DvbC2())
		{
			__FIf__->SetPlayParam_OutClkSrc(1);	//	max clk
		}
		else
		{
			__FIf__->SetPlayParam_OutClkSrc(0);
		}
#endif
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING ", szFilePath);
		__HLog__->HldPrint_1("Hld-Bd-Ctl. TSPH_START_IP_STREAMING ", nPlayrate);
		__FIf__->SetPlayParam_Repeat(1);

		__Sta__->SetReqedStartPlay_User(TRUE);

		gDblStartOffsetL = 0;
		gDblStartOffsetH = 0;
		__FIf__->InitLoopPcrRestamp_Param();

		return 0;
		}
		break;

	case VLC_SEND_IP_STREAM:
#if	1
		__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "Start_IpStreaming_OnUserReq_ExtCntl : VLC_SEND_IP_STREAM");
#else
		{
		if ( IPUsing() )
		{
		}
		else if ( !szExtendedInfo || strlen(szExtendedInfo) <= 0 )
		{
			HldPrint("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, OUTPUT target may be empty");
			return -30;//TLV_FAIL_TO_START_IP_STREAMING
		}

		g_IP_ProgramCount = 0;
		g_IP_CurrentProgram = -1;
		g_IP_ProgramChanged = -1;
		Set_VlcState(nExtendedMode);
		sprintf(g_VLC_Str, "%s", szExtendedInfo);

		if ((szFilePath != NULL) && (strlen(szFilePath)<=MAX_PATH))
		{
#if defined(WIN32)
			fi = _open(szFilePath, _O_BINARY);
#else
			fi = open(szFilePath, O_RDONLY);
#endif
			if (fi != -1)
			{
				strcpy(PlayParm.AP_lst.szfn, szFilePath); 
#if defined(WIN32)
				_close(fi);
#else
				close(fi);
#endif
				if ( PlayParm.AP_lst.nPlayRate <= 0L )
				{
					PlayParm.AP_lst.nPlayRate = 19392658L;
				}
				if ( (nPlayrate == 0L) || (nPlayrate == -1L) || (nPlayrate < 1L) || (nPlayrate > 90000000L) )
				{
					nPlayrate = 19392658L;
				}
				PlayParm.AP_lst.nPlayRate = nPlayrate;
				PlayParm.AP_lst.nOutputClockSource = nOption & 0xFF;
				HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING ", szFilePath);
				PlayParm.AP_fRepeat = nRepeatMode==0?0:1;	// 0 or 1
				SetReqedStartPlay_User(TRUE);

				if (dStartOffset >= 0 ) 
				{
					__int64 ddFileLen = (__int64)dStartOffset;
					gDblStartOffsetL = (long)((ddFileLen & 0xFFFFFFFF)/1024)*1024;
					gDblStartOffsetH = (long)(ddFileLen >> 32);
				}
				return 0;
			}
		}
		HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, fail to open file ",szFilePath);
		return -1;
		}
#endif
		break;

	case VLC_RUN_ASI_DISPLAY:
		{
		if ( szFilePath != NULL )
		{
#ifdef	TSPHLD0390_EXPORTS
			if ( nOption == 0 ) //FILE source
			{
				__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to set TSIO DIRECTION in START REC");
				return -1;
			}
#endif
			__FIf__->SetPlayParam_FName(szDummyFilepath);

			g_IP_ProgramCount = 0;
			g_IP_CurrentProgram = -1;
			g_IP_ProgramChanged = -1;
			__Sta__->Set_VlcState(VLC_SEND_IP_STREAM);
			sprintf(g_VLC_Str, "%s", szExtendedInfo);

			if ( __FIf__->TSPL_SET_TSIO_DIRECTION(nOption) == -1 )
			{
				__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to set TSIO DIRECTION in START REC");
				return -1;
			}

			__FIf__->Set_RecFileName(szFilePath);
			__HLog__->HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, ASI/310 ", szFilePath);
			__Sta__->SetReqedStartRecord_User(TRUE);
			return 0;
		}
		__HLog__->HldPrint("Hld-Bd-Ctl. INVALID RECORD_FILE TO START");
		return -20; //TLV_FAIL_TO_CREATE_RECORD_FILE;
		}
		break;

	case VLC_RUN_FILE_DISPLAY:
	case VLC_RUN_NO_READY:
		{
		if ( __Sta__->IPUsing() )
		{
		}
		else if ( !szExtendedInfo || strlen(szExtendedInfo) <= 0 )
		{
			__HLog__->HldPrint("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, OUTPUT target may be empty");
			return -30;//TLV_FAIL_TO_START_IP_STREAMING
		}
		__Sta__->Set_VlcState(VLC_SEND_IP_STREAM);
		g_IP_ProgramCount = 0;
		g_IP_CurrentProgram = -1;
		g_IP_ProgramChanged = -1;

		sprintf(g_VLC_Str, "%s", szExtendedInfo);
		if ((szFilePath != NULL) && (strlen(szFilePath)<=MAX_PATH))
		{
			fi = __FIf__->FOpen_readonly_stdio(szFilePath);
			if (fi != -1)
			{
				__FIf__->SetPlayParam_FName(szFilePath);
				__FIf__->FClose__stdio(fi);

				if ( __FIf__->GetPlayParam_PlayRate() <= 0L )
				{
					__FIf__->SetPlayParam_PlayRate(19392658L);
				}
				if ( (nPlayrate == 0L) || (nPlayrate == -1L) || (nPlayrate < 1L) || (nPlayrate > 90000000L) )
				{
					nPlayrate = 19392658L;
				}

				__FIf__->SetPlayParam_PlayRate(nPlayrate);
				__FIf__->SetPlayParam_OutClkSrc(nOption & 0xFF);
				__HLog__->HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING ", szFilePath);
				__FIf__->SetPlayParam_Repeat(nRepeatMode==0?0:1);

				__Sta__->SetReqedStartPlay_User(TRUE);

				if (dStartOffset >= 0 ) 
				{
					__int64 ddFileLen = (__int64)dStartOffset;
					gDblStartOffsetL = (long)((ddFileLen & 0xFFFFFFFF)/1024)*1024;
					gDblStartOffsetH = (long)(ddFileLen >> 32);
				}
				return 0;
			}
		}
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. TSPH_START_IP_STREAMING, fail to open file ",szFilePath);
		return -1;
		}
		break;

	default :
		{
		__HLog__->HldPrint("Hld-Bd-Ctl. donot use this case. 123456");
		__Sta__->Set_VlcState(VLC_RUN_NO_READY);
		memset(g_VLC_Str, 0, MAX_PATH);

		return	Req_Sta_byUser_StartPlay(szFilePath, nPlayrate, dStartOffset, nOption, nRepeatMode);
		}
		break;
	}

	return -1;// invalid file name, can not open the file
}

void CHldFmtrIp::Finish_IpStreaming_OnUserMonReq(void)
{
	if (__Sta__->IPUsing())
	{
//		__FIf__->TSPL_SET_MAX_PLAYRATE(__Sta__->TL_gCurrentModType, prev_max_playrate_sta);
		LockIpMutex_Cntrl();
		__Sta__->Set_IPUsing(0);
		UnlockIpMutex_Cntrl();

		if (__Sta__->Vlc_SendIpStreaming())
		{
		}
		else if (__Sta__->Vlc_RecvIpStreaming() || __Sta__->Vlc_RecvIpStreamingRec())
		{
			close_ip_streaming_srv();
			flush_ip_recv_buffer(0);

			__FIf__->SetRestamp_Flag(TL_gPCR_RestampingPre, TL_gRestampingPre);
		}
		__Sta__->Set_VlcState(VLC_RUN_NO_READY);

		__FIf__->FClose__StreamFile(&TL_ip_recorded);
	}

}
void CHldFmtrIp::Set_IpStreaming_TxBitrate(long play_freq_in_herz)
{
	//if (__Sta__->IPUsing() && __Sta__->Vlc_HasBeenActivated() )
	{
		TL_CurrentBitrate = play_freq_in_herz;
		__HLog__->HldPrint("...donot use this func : Set_IpStreaming_TxBitrate");
	}
}
void CHldFmtrIp::RunVlc_OnRecStart(void *lld)
{
	CHld	*_lld;

	_lld = (CHld *)lld;
#ifdef IP_STREAM
	if ( __Sta__->IPUsing() &&__Sta__->Vlc_HasBeenActivated() )
	{
	}
	else if ( g_VLCWrapper && __Sta__->Vlc_HasBeenActivated() )
	{
#if defined(WIN32)
		g_VLCWrapper->VLC_Run(_lld);
#else
#endif
	}
#endif
}
void CHldFmtrIp::RunVlc_OnPlayStart(void *lld)
{
	RunVlc_OnRecStart(lld);
}

void CHldFmtrIp::SendIpStreaming_OnRecCont__HasEndlessWhile(int _ok_cap)
{
#ifdef IP_STREAM
	if ( _ok_cap == 0 )
	{
		if ( __Sta__->IPUsing() && __Sta__->Vlc_SendIpStreaming() )
		{
		}
		else if ( g_VLCWrapper && __Sta__->Vlc_SendIpStreaming() )
		{
			//if ( g_hPumpingEvent ) 
			{
				g_VLCWrapper->g_PumpingWait = 0;
				//SetEvent(g_hPumpingEvent);
				while (!g_VLCWrapper->g_PumpingWait && __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User() ) 
				{
					Sleep(5);
				}
			}
		}
	}
#endif
}
void CHldFmtrIp::SendIpStreaming_OnPlayCont__HasEndlessWhile(void)
{
#ifdef IP_STREAM
	if ( __Sta__->IPUsing() )
	{
	}
	else if ( g_VLCWrapper && __Sta__->Vlc_SendIpStreaming() )
	{
#if	0
		__HLog__->HldPrint("Hld-Bd-Ctl. : not used src-pos : jjj ");
#else
		//if ( g_hPumpingEvent ) 
		{
			g_VLCWrapper->g_PumpingWait = 0;
			//SetEvent(g_hPumpingEvent);
			while (!g_VLCWrapper->g_PumpingWait && __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() ) 
			{
				Sleep(5);
			}
		}
#endif
	}
#endif
}
int CHldFmtrIp::LaunchVlcCodec_and_WaitStop_OnPlayCont(void *lld)
{
	DWORD	dwRet;
	CHld	*_lld;

	_lld = (CHld *)lld;
#ifdef IP_STREAM
	if ( __Sta__->IPUsing() && __Sta__->Vlc_HasBeenActivated() )
	{
	}
	else if ( g_VLCWrapper && __Sta__->Vlc_HasBeenActivated() )
	{
		if ( __FIf__->IsTheFileNot_PlayParam_FName(g_VLCWrapper->VLC_Current_Target) != 0 )
		{
			g_IP_ProgramChanged = -1;
			while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
			{
#if defined(WIN32)
				dwRet = g_VLCWrapper->VLC_Read_Buffered_Count(_lld);
#else
#endif
				if ( dwRet != 0 )
				{
					Sleep(10);
					continue;
				}
#if defined(WIN32)
				g_VLCWrapper->VLC_Stop();
#else
#endif
				__Sta__->SetReqedStartPlay_User(TRUE);
				return 0;
			}
		}
	}
#endif

	return	1;
}
int CHldFmtrIp::CalcBitRateAtStreamingStart(void)	//	use bitrate, given value by user.
{
	__FIf__->SetBitRate(__FIf__->GetPlayParam_PlayRate());
	//TL_ip_bfp = (__FIf__->GetPlayParam_PlayRate()/8)*__CRITERIA_IP_BUF_INITIAL_FILLL__;
	double dwBufferSize;
	dwBufferSize = (double)__FIf__->GetPlayParam_PlayRate() / 8.;
	//if (__Sta__->IsModTyp_DvbT2() || __Sta__->IsModTyp_IsdbS())
	dwBufferSize *= 2.5;
	TL_ip_bfp = (int)dwBufferSize;
	if (TL_ip_bfp > (G_BUFFER_SIZE/2))
	{
		TL_ip_bfp = (G_BUFFER_SIZE/2);
	}
	return	__FIf__->GetBitRate();
}
int CHldFmtrIp::InitRcvIpEnv_OnPlayStart(int *_nBankBlockSize)
{
#ifdef IP_STREAM
	int	bankoffset = 0;

	if ( __Sta__->IPUsing() && (__Sta__->Vlc_Recv_or_RecvRec()) )
	{
		CalcBitRateAtStreamingStart();

		if ( __FIf__->GetBitRate() < 2000000. ) bankoffset = 16;
		else if ( __FIf__->GetBitRate() >= 2000000. && __FIf__->GetBitRate() < 3000000. ) bankoffset = 32;
		else if ( __FIf__->GetBitRate() >= 3000000. && __FIf__->GetBitRate() < 6000000. ) bankoffset = 64;
		else if ( __FIf__->GetBitRate() >= 6000000. && __FIf__->GetBitRate() < 12000000. ) bankoffset = 128;
		else if ( __FIf__->GetBitRate() >= 12000000. && __FIf__->GetBitRate() < 24000000. ) bankoffset = 256;
		else if ( __FIf__->GetBitRate() >= 24000000. && __FIf__->GetBitRate() < 48000000. ) bankoffset = 512;
		else bankoffset = 1024;

		if(__Sta__->IsModTyp_DvbT2() || __Sta__->IsModTyp_DvbC2())
		{
			bankoffset *= 2;
			if (bankoffset > 1024)	
				bankoffset = 1024;
		}
		//bankoffset *= 4;
		//if (bankoffset > 1024)	
		//	bankoffset = 1024;

		accum_percent_[0] = accum_percent_[1] = accum_percent_[2] = accum_percent_[3] = accum_percent_[4] = 0;
		under_over_vector = 1.0;
		__estimated_out_speed_bytespersec = (float)__FIf__->GetPlayParam_PlayRate()/8;
		__estimated_in_speed_bytespersec = (float)__FIf__->GetBitRate()/8;


//		if ( __Sta__->iHW_BANK_OFFSET != bankoffset )
		{
			__Sta__->iHW_BANK_OFFSET = bankoffset;
			__FIf__->TSPL_SET_SDRAM_BANK_INFO(__Sta__->iHW_BANK_NUMBER, __Sta__->iHW_BANK_OFFSET);
			__FIf__->TSPL_SET_SDRAM_BANK_CONFIG(__Sta__->iHW_BANK_NUMBER);
			__FIf__->TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(__Sta__->iHW_BANK_OFFSET);

			*_nBankBlockSize = (__FIf__->SdramSubBankSize());
		}
	
		TL_CurrentBitrate = (int)__FIf__->GetBitRate();
		__FIf__->SetPlayParam_PlayRate(TL_CurrentBitrate);

		return	0;
	}
#endif

	return	1;
}

void CHldFmtrIp::RestartVlc_OnUserReq(void *lld)
{
	CHld	*_lld;

	_lld = (CHld *)lld;
	if ( g_VLCWrapper && __Sta__->Vlc_HasBeenActivated() )
	{
#if defined(WIN32)
		g_VLCWrapper->VLC_Restart(_lld);
#else
#endif
	}
}
void CHldFmtrIp::StopVlc_OnUserReq(void)
{
	if ( g_VLCWrapper && __Sta__->Vlc_HasBeenActivated() )
	{
#if defined(WIN32)
		g_VLCWrapper->VLC_Stop();
#else
#endif
	}
}
long CHldFmtrIp::StsIpRx_OnUserReq(long nStatus)
{
	switch (nStatus)
	{
	case 0://INPUT_BITRATE:
		if ( __Sta__->IPUsing() )
		{
			if ( __Sta__->Vlc_SendIpStreaming() )
			{
				return -1;	//	not support
			}
			if ( __Sta__->Vlc_Recv_or_RecvRec() )
			{
				return (long)bitrate_ip_recv_buffer();
			}
		}
		return (long)g_IP_InputBitrate;

	case 1://AVERAGE_INPUT_BITRATE:
		return (long)g_IP_AverageInputBitrate;

	case 2://DEMUX_BITRATE:
		return (long)g_IP_DemuxBitrate;

	case 3://AVERAGE_DEMUX_BITRATE:
		return (long)g_IP_AverageDemuxBitrate;
	}
	return 0;
}
long CHldFmtrIp::SetIpProg_OnUserReq(long nIndex)
{
	if ( g_IP_ProgramCount > 0 )
	{
		if ( g_IP_CurrentProgram >= 0 && g_IP_CurrentProgram != nIndex )
		{
			g_IP_ProgramChanged = nIndex;
		}
	}
	return 0;
}
long CHldFmtrIp::GetIpProg_OnUserReq(long nIndex)
{
	//total program Count
	if ( nIndex == -1 )
	{
		return (long)g_IP_ProgramCount;
	}

	//current program
	if ( nIndex == -2 )
	{
		return (long)g_IP_CurrentProgram;
	}

	//program list
	if ( nIndex >= 0 && nIndex < g_IP_ProgramCount ) // < 64
	{
		return g_IP_ProgramId[nIndex];
	}

	return -1;
}

int CHldFmtrIp::VlcShowVid_OnUserReq(int nShowWindow)
{
	if ( g_VLCWrapper && !__Sta__->IPUsing() )
	{
#if defined(WIN32)
		g_VLCWrapper->ShowWindow(nShowWindow);
#else
#endif
		return 0;
	}
	return -1;
}
int CHldFmtrIp::VlcMoveVid_OnUserReq(int nX, int nY, int nW, int nH)
{
	if ( g_VLCWrapper && !__Sta__->IPUsing() )
	{
		if ( g_VLCWrapper->m_nVideo_X == nX && g_VLCWrapper->m_nVideo_Y == nY && g_VLCWrapper->m_nVideo_W == nW && g_VLCWrapper->m_nVideo_H == nH )
		{
			return 0;
		}

		g_VLCWrapper->m_nVideo_X = nX;
		g_VLCWrapper->m_nVideo_Y = nY;
		g_VLCWrapper->m_nVideo_W = nW;
		g_VLCWrapper->m_nVideo_H = nH;
#if defined(WIN32)
		g_VLCWrapper->MoveResizeWindow(nX, nY, nW, nH);
#else
#endif
		return 0;
	}
	return -1;
}
int CHldFmtrIp::IsVlcVidVisible_OnUserReq(void)
{
	if ( g_VLCWrapper && !__Sta__->IPUsing() )
	{
#if defined(WIN32)
		return g_VLCWrapper->IsWindowVisible();
#else
#endif
	}
	return -1;
}
int CHldFmtrIp::SetVlcVidWin_OnUserReq(long hWnd)
{
	if ( g_VLCWrapper && !__Sta__->IPUsing() )
	{
#if defined(WIN32)
		g_VLCWrapper->SetOutputWindow((void*)&hWnd);
#else
#endif
	}
	return 0;	
}
int CHldFmtrIp::CaclIpBitrateAdj_or_VlcCodecResize_OnPlayCont(void *lld)
{
	CHld	*_lld;

	_lld = (CHld *)lld;
	/*-----1-----*/
#ifdef IP_STREAM
	if ( __Sta__->IPUsing() )
	{
	}
	else if ( g_VLCWrapper && g_IP_ProgramChanged != -1 && g_IP_ProgramChanged != g_IP_CurrentProgram )
	{
#if defined(WIN32)
		g_VLCWrapper->VLC_Set_Program(_lld);
#else
#endif
		g_IP_ProgramChanged = -1;

		//TEST - VIDEO WINDOW MOVE/RESIZE
		Sleep(1000);
#if defined(WIN32)
		g_VLCWrapper->MoveResizeWindow(g_VLCWrapper->m_nVideo_X, g_VLCWrapper->m_nVideo_Y, g_VLCWrapper->m_nVideo_W, g_VLCWrapper->m_nVideo_H);
#else
#endif

		return 0;
	}
#endif

	return	1;
}

int CHldFmtrIp::FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile(void)	//	buf target to fill dta is TL_szBufferPlay
{
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

#ifdef IP_STREAM
	if ( __Sta__->IPUsing() && (__Sta__->Vlc_Recv_or_RecvRec()) )
	{
		CpInputIpDta_into_PlayBuf();
		if (__FIf__->TL_nBCount < (unsigned int)nBankBlockSize)
		{
//			__HLog__->HldPrint("No-Data in Play-buffer-pool");
			return	0;
		}
	}
	else if ( g_VLCWrapper && (__Sta__->Vlc_Recv_or_RecvRec()) )
	{
		//if ( g_hPumpingEvent ) 
		{
			g_VLCWrapper->g_PumpingWait = 0;
			//SetEvent(g_hPumpingEvent);
			while (!g_VLCWrapper->g_PumpingWait && __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() ) 
			{
				Sleep(5);
			}
		}
	}
#endif

	return	1;
}

void CHldFmtrIp::SetASIbufferStatus(int nVal)
{
	TL_ip_count = nVal;
}
int CHldFmtrIp::CalcedInTsBitRate(void)
{
	__HLog__->HldPrint("[ip-recv] : unactive func. currentlly... CalcedInTsBitRate");
	return	(int)__estimated_in_speed_bytespersec;
}
void CHldFmtrIp::CalcInTsBitRate(unsigned char *_rd_buf, int _rd_cnt)
{
return; //	not use

	int		_rate;

	_rate = __FIf__->SyncPosAssume188((char *)_rd_buf, _rd_cnt);
	if (_rate < 0)
	{
		return;
	}
	_rate = __FIf__->TL_Calculate_Bitrate(_rd_buf, _rd_cnt, _rate, 188, NULL, NULL);
	__estimated_in_speed_bytespersec -= (__estimated_in_speed_bytespersec/50);	//	5-tap
	__estimated_in_speed_bytespersec += (((float)_rate/8)/50);	//	5-tap
}
void CHldFmtrIp::CalcInTsBitRateBasedOnBytes(int _rd_cnt)
{
return;	//	not use

	float	_rate;

	switch (ip_pkt_monotor_sta)
	{
	case	__IP_STATE_buf_fill_wait__:
		break;
	case	__IP_STATE_playback_progress__:
#ifdef WIN32
		__past_msec = __FIf__->_past_msec_(0);
#else
		__past_msec = (unsigned long)__FIf__->_msec_();
#endif
		if (__past_msec > 0)
		{
			_rate = (float)_rd_cnt/(float)__past_msec;
			if (__estimated_out_speed_bytespersec > 0)
			{
				__estimated_out_speed_bytespersec -= (__estimated_out_speed_bytespersec/50);
				__estimated_out_speed_bytespersec += (_rate/50);
			}
		}
		break;
	}
}
void CHldFmtrIp::ProducerIpStreamingBuffer_2(unsigned char *_rd_buf, int _rd_cnt)
{
	int		_wr_hw_pkts, _recved_pkts, _diff_perc_unit_0_1;
	int		_c_nulll_insert, _c_nulll_remove, _siz;
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

///////////////////////////////////////////////////////////////////////
	CalcInTsBitRate(_rd_buf, _rd_cnt);
	UpdPktMonitorSta(__IP_STATE_CMD_rd_ip__, (unsigned int)_rd_cnt, 0, 0);

	_wr_hw_pkts = _recved_pkts = _diff_perc_unit_0_1 = 0;
	_c_nulll_insert = _c_nulll_remove = 0;
	if (cnt_ip_wr_hw_pkt_bytes >= ((unsigned int)nBankBlockSize*__CRITERIA_IP_BUF_MONITOR_START__))
	{
		if (cnt_ip_recv_pkt_bytes >= ((unsigned int)nBankBlockSize*__CRITERIA_IP_BUF_MONITOR_START__))
		{
//			_recved_pkts = (int)((cnt_ip_recv_pkt_bytes - TL_ip_count - _rd_cnt)/188);
			_recved_pkts = (int)((cnt_ip_recv_pkt_bytes - _rd_cnt)/188);
			_wr_hw_pkts = (int)(cnt_ip_wr_hw_pkt_bytes/188);
			_diff_perc_unit_0_1 = (_wr_hw_pkts - _recved_pkts)*1000/_recved_pkts;

			if ((_diff_perc_unit_0_1 >= __CRITERIA_DIFF_IP_BUF_IN_OUT__outofservice__) ||
				(_diff_perc_unit_0_1 <= -__CRITERIA_DIFF_IP_BUF_IN_OUT__outofservice__))
			{
				UpdPktMonitorSta(__IP_STATE_CMD_init_buf__, 0, 0, 0);
				goto	_clred;
			}
			else if ((_diff_perc_unit_0_1 >= 10) || (_diff_perc_unit_0_1 <= -10))	//	1%
			{
				goto	_clred;	//	ignore until to converge high or low boundary.
			}
			else if (_diff_perc_unit_0_1 >= __CRITERIA_DIFF_IP_BUF_IN_OUT__diverge__)
			{
				ip_pkt_monotor_orientation++;
				if (ip_pkt_monotor_orientation >= __CRITERIA_ORIENTATION_DELAY_ins_rmv__)
				{
					ip_pkt_monotor_orientation = __CRITERIA_ORIENTATION_DELAY_ins_rmv__;
					_c_nulll_insert = 1;
				}
			}
			else if (_diff_perc_unit_0_1 <= -__CRITERIA_DIFF_IP_BUF_IN_OUT__diverge__)
			{
				ip_pkt_monotor_orientation--;
				if (ip_pkt_monotor_orientation <= -__CRITERIA_ORIENTATION_DELAY_ins_rmv__)
				{
					ip_pkt_monotor_orientation = -__CRITERIA_ORIENTATION_DELAY_ins_rmv__;
					_c_nulll_remove = 1;
				}
			}

			if ((cnt_ip_recv_pkt_bytes >= 2000000000) || (cnt_ip_wr_hw_pkt_bytes >= 2000000000))	//	2-Gbyte
			{
				UpdPktMonitorSta(__IP_STATE_CMD_wrap_max_buf__, 0, 0, 0);
			}
		}
	}
	__HLog__->HldPrint_IpBuf_Monitor_2(__Sta__->TL_gCurrentModType, TL_ip_count, _diff_perc_unit_0_1, _recved_pkts, _wr_hw_pkts, _c_nulll_insert, _c_nulll_remove);
	__HLog__->HldPrint_CntWait_2("[ip-recv] : lost-pkt-cnt", 10, get_eth_pkt_lost_cnt());


_clred:
	//2014/01/08 IP Improve
	_c_nulll_insert = _c_nulll_remove = 0;

	_siz = __FIf__->InsertNullPkt_into_IpTempBuf_188Ts_2(_rd_buf, _rd_cnt, _c_nulll_insert, _c_nulll_remove, ip_tmp_buf);
	if (_siz != _rd_cnt)
	{
		UpdPktMonitorSta(__IP_STATE_CMD_contents_modified__, 0, _c_nulll_insert, _c_nulll_remove);
	}
	if ( __Sta__->Vlc_RecvIpStreamingRec() )
	{
		__FIf__->_FWrite_TmpBuf(TL_ip_recorded, _siz, ip_tmp_buf);
	}
	__FIf__->FillOnePacketToQueue(_siz, TL_ip_buffer, &TL_ip_write, &TL_ip_read, &TL_ip_count, (int)G_BUFFER_SIZE, ip_tmp_buf);	//	recv ip data and cpy into TL_ip_buffer.

}
int CHldFmtrIp::IsThere_EnoughIpDta(void)
{
	LockIpMutex();
	if (!IsBufStableCond())
	{
		UnlockIpMutex();
		return	0;
	}
	UnlockIpMutex();
	if (TL_ip_count >= __FIf__->SdramSubBankSize()*2)
	{
		return	1;
	}
	return	0;
}
int CHldFmtrIp::Get_RcvIpDta(unsigned char *buff, int size)
{
	if (TL_ip_count < size)
	{
		return	0;
	}
	LockIpMutex();
	if (!IsBufStableCond())
	{
		UnlockIpMutex();
		return	0;
	}
	__FIf__->ReadOnePacketFromeQueue(
		(int)size, (unsigned char *)TL_ip_buffer,
		(int*)&TL_ip_write, (int*)&TL_ip_read, (int*)&TL_ip_count, (int)G_BUFFER_SIZE, (unsigned char *)buff);
#if 0
	char debugstr[100];
	sprintf(debugstr, "TL_ip_write, = %d, TL_ip_read = %d, TL_ip_count = %d\n", TL_ip_write, TL_ip_read, TL_ip_count);
	OutputDebugString(debugstr);
#endif

	UpdPktMonitorSta(__IP_STATE_CMD_wr_hw__, (unsigned int)size, 0, 0);
	CalcInTsBitRateBasedOnBytes(size);
	UnlockIpMutex();
	return	size;
}
void CHldFmtrIp::CpInputIpDta_into_PlayBuf(void)
{
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

	if (TL_ip_count < nBankBlockSize)
	{
		return;
	}
	LockIpMutex();
	if (!IsBufStableCond())
	{
		UnlockIpMutex();
		return;
	}
	__FIf__->Set_ReadBytes_PlayBuf(nBankBlockSize);
	__FIf__->CpyIpRcvDta_into_TmpBuf(&TL_ip_read, &TL_ip_count, TL_ip_buffer, nBankBlockSize, ip_tmp_buf);
	__FIf__->CpyIpRcvDta_into_PlayBuf_188Ts(nBankBlockSize, 0, 0, ip_tmp_buf);
	UpdPktMonitorSta(__IP_STATE_CMD_wr_hw__, (unsigned int)nBankBlockSize, 0, 0);
	CalcInTsBitRateBasedOnBytes(nBankBlockSize);
	UnlockIpMutex();
}
int	CHldFmtrIp::SetRx_IpStreamingInfo(char* src_ip, char* rx_multicast_ip, long rx_udp_port, char* local_ip, long fec_udp_off, long fec_inact)
{
	memcpy(TL_src_ip, src_ip, 32);
	memcpy(TL_rx_multicast_ip, rx_multicast_ip, 32);
	TL_rx_udp_port = rx_udp_port;
	memcpy(TL_local_ip, local_ip, 32);
	TL_fec_udp_off = fec_udp_off;
	TL_fec_inact = fec_inact;

//	__HLog__->HldPrint("Hld-Bd-Ctl. SRC=%s, MCAST=%s, PORT=%d, LOCAL=%s\n", src_ip, rx_multicast_ip, (int)rx_udp_port, local_ip);

	return 0;
}
int	CHldFmtrIp::SetTx_IpStreamingInfo(char* dest_ip, char* tx_multicast_ip, long tx_udp_port, long ip_protocol, long snd_pull_ts, long col_udp_off, long col_off, long col_na)
{
	memcpy(TL_dest_ip, dest_ip, 320);
	memcpy(TL_tx_multicast_ip, tx_multicast_ip, 32);
	TL_tx_udp_port = tx_udp_port;
	TL_ip_protocol = ip_protocol;
	TL_snd_pull_ts = snd_pull_ts;
	TL_col_udp_off = col_udp_off;
	TL_col_off = col_off;
	TL_col_na = col_na;

	return 0;
}
int	CHldFmtrIp::Req_Sta_byUser_StartDelay(int nPort)
{
	//__Sta__->SetReqedStartDelay_User(TRUE);
	__Sta__->SetReqedStartDelay_User(nPort);
	return 0;
}
int	CHldFmtrIp::Req_Sta_byUser_StartMonitor(int nPort)
{
	__HLog__->HldPrint("App-Call : Req_Sta_byUser_StartMonitor");
	Finish_IpStreaming_OnUserMonReq();
	Finish_Vlc_OnUserMonReq();

	__Sta__->SetReqedStartMonitor_User(TRUE);

#ifdef	TSPHLD_VLC
#if defined(WIN32)
#else
	::unlink("VLC_SHARED_CONTROL.odb");
	::unlink("VLC_SHARED_CONTROL.mon");
	::unlink("VLC_SHARED_BUFFER.odb");
	::unlink("VLC_SHARED_BUFFER.mon");
#endif	
#endif
	__HLog__->HldPrint("App-Call : Req_Sta_byUser_StartMonitor---");

	return 0;
}
int CHldFmtrIp::Req_Sta_byUser_StartPlay(char *szfnNewPlayFile, LONG PlayFreq, double DblStartOffset, long nUseMaxPlayrate, long fRepeatMode)
{
	CHld	*_hld_ = (CHld *)my_hld;

	LockIpMutex_Cntrl();
	__Sta__->Set_IPUsing(0);
	UnlockIpMutex_Cntrl();
	if ((szfnNewPlayFile != NULL) && (strlen(szfnNewPlayFile)<=MAX_PATH))
	{
		if (__FIf__->FChk_Existense(szfnNewPlayFile))
		{
			__FIf__->SetPlayParam_FName(szfnNewPlayFile);
			if ( __FIf__->GetPlayParam_PlayRate() <= 0L )
			{
				__FIf__->SetPlayParam_PlayRate(19392658L);
			}
			if ( Flag_IqPlayCap() )
			{
				if ((PlayFreq == 0L) ||	(PlayFreq == -1L) || (PlayFreq < 1L) || (PlayFreq > 500000000L) )
				{
					PlayFreq = 344391584L;	//10762237L*32;
				}
			}
			__FIf__->SetPlayParam_PlayRate(PlayFreq);
			__FIf__->SetPlayParam_OutClkSrc(nUseMaxPlayrate & 0xFF);
			__FIf__->SetPlayParam_Repeat(fRepeatMode==0?0:1);	// 0 or 1

			if ( __Sta__->IsModTyp_AtscMH() )
			{
				_hld_->InitMHEInfo(_hld_->m_MHE_packet_PID);
				_hld_->InitMHELoopPath(_hld_->m_MHE_packet_PID, 0/*TSIO_PLAY_WITH_310INPUT*/);
			}
			__Sta__->SetReqedStartPlay_User(TRUE);

			if (DblStartOffset >= 0 ) 
			{
				__int64	ddFileLen = (__int64)DblStartOffset;
				gDblStartOffsetL = (long)((ddFileLen & 0xFFFFFFFF)/1024)*1024;
				gDblStartOffsetH = (long)(ddFileLen >> 32);
			}
			if(__Sta__->IsModTyp_IsdbT_13() || __Sta__->IsModTyp_IsdbT_1())
			{
				if (__FIf__->TmccUsing() && _hld_->Current_Mode == 0)
					return -1;
			}
			return 0;
		}
	}

	//2012/3/15 BERT 187
#if 1
	int nBertType = __FIf__->_VarTstPktTyp();
	if((nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23) && __Sta__->is_Bert_187_Mod())
	{
		if ( __FIf__->GetPlayParam_PlayRate() <= 0L )
		{
			__FIf__->SetPlayParam_PlayRate(19392658L);
		}
		__FIf__->SetPlayParam_PlayRate(PlayFreq);
		__FIf__->SetPlayParam_OutClkSrc(nUseMaxPlayrate & 0xFF);
		__FIf__->SetPlayParam_Repeat(fRepeatMode==0?0:1);	// 0 or 1
		__Sta__->SetReqedStartPlay_User(TRUE);
		return 0;
	}
#endif

	return -1;
}
int	CHldFmtrIp::Req_Sta_byUser_StartRevord(char* szNewRecordFile, int nPort)
{
#ifdef TSPHLD0110_EXPORTS
ooo;
	if ( nPort == 1 )
	{
		TL_nRecordNA = 1;
	}
	else
	{
		TL_nRecordNA = 0;
	}
#endif

	if ( szNewRecordFile != NULL )
	{
#ifdef	TSPHLD0390_EXPORTS
		if ( nPort == 0 )	//FILE source
		{
//			HldPrint("Hld-Bd-Ctl. FAIL to set TSIO DIRECTION in START REC\n");
			return -1;
		}
#endif


		if ( __Sta__->IsState_IsdbT13LoopThru() && (__Sta__->Flag_PlayLoopThru() || __Sta__->Flag_CapLoopThru()))
		{
		}
		else
		if ( __Sta__->IsModTyp_AtscMH() && (__Sta__->Flag_PlayLoopThru() || __Sta__->Flag_CapLoopThru()) )
		{
		}
		//2011/8/5 ISDB-S ASI
		else if( __Sta__->IsState_IsdbSLoopThru() && (__Sta__->Flag_PlayLoopThru() || __Sta__->Flag_CapLoopThru()))
		{
		}
		//2012/7/23 DVB-T2 ASI
		else if( (__Sta__->IsState_DvbT2LoopThru() || __Sta__->IsState_DvbC2LoopThru())  && __Sta__->Flag_CapLoopThru())
		{
			if(__FIf__->AP_hFile == INVALID_HANDLE_VALUE)
			{
				__FIf__->Set_RecFileName(szNewRecordFile);
				__FIf__->OpenFile_OnRecStart();
				__FIf__->g_DvbT2_FileCaptureFlag = 1;
			}
			else
			{
				__FIf__->g_DvbT2_FileCaptureFlag = 0;
			}
			return 0;
		}
		else if ( __FIf__->TSPL_SET_TSIO_DIRECTION(nPort) == -1 )
		{
			return -1;
		}
		__FIf__->Set_RecFileName(szNewRecordFile);

		__Sta__->SetReqedStartRecord_User(TRUE);
		return 0;
	}
	return -20; //TLV_FAIL_TO_CREATE_RECORD_FILE
}

void	CHldFmtrIp::CHECK_INIT_IP(void)
{
	if ( __Sta__->IsModTyp_IsdbT_13() && __Sta__->IsTaskState_ContPlay() )
	{
		__FIf__->TSPL_RESET_IP_CORE(__Sta__->TL_gCurrentModType, 0x00001FFF);
		Sleep(1);
		__FIf__->TSPL_RESET_IP_CORE(__Sta__->TL_gCurrentModType, 0x000003FF);
	}
}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)
void CHldFmtrIp::CHECK_LOCK(long a, long b)
{
	if ( a == 0 || a == 1 )
	{
		if (__FIf__->TL_hMutex != INVALID_HANDLE_VALUE )
			WaitForSingleObject(__FIf__->TL_hMutex, INFINITE);
	}
}
void CHldFmtrIp::CHECK_UNLOCK(long a)
{
	if ( a == 0 || a == 1 )
	{
		if ( __FIf__->TL_hMutex != INVALID_HANDLE_VALUE )
			ReleaseMutex(__FIf__->TL_hMutex);
	}
}
#else
void CHldFmtrIp::CHECK_LOCK(long a, long b)
{
	if ( a == 0 || a == 2 )
	{
		if ( __Sta__->IPUsing() && __Sta__->Vlc_HasBeenActivated() )
		{
			pthread_mutex_lock(&TL_ip_hMutex);
		}
	}
}
void CHldFmtrIp::CHECK_UNLOCK(long a)
{
	if ( a == 0 || a == 2 )
	{
		if ( __Sta__->IPUsing() && __Sta__->Vlc_HasBeenActivated() )
		{
			pthread_mutex_unlock(&TL_ip_hMutex);
		}
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrIp::IsBufStableCond(void)
{
	switch (ip_pkt_monotor_sta)
	{
	case	__IP_STATE_buf_fill_wait__:
	case	__IP_STATE_playback_progress__:
		if (TL_ip_count < (TL_ip_bfp*__CRITERIA_IP_BUF_STABLE_COND_under__/10))
		{
			UpdPktMonitorSta(__IP_STATE_CMD_detected_under__, 0, 0, 0);
			return	0;
		}
		break;
	case	__IP_STATE_transition_buf_underover__:
		if (TL_ip_count >= (TL_ip_bfp*__CRITERIA_IP_BUF_STABLE_COND___/10))
		{
			UpdPktMonitorSta(__IP_STATE_CMD_detected_stable__, 0, 0, 0);
		}
		return	0;
	}
	return	1;
}
void CHldFmtrIp::UpdPktMonitorSta(int _cmd, unsigned int _size, int _insert, int _remove)
{
	switch (ip_pkt_monotor_sta)
	{
	case	__IP_STATE_buf_fill_wait__:
		cnt_ip_recv_pkt_bytes = 0;
		cnt_ip_wr_hw_pkt_bytes = 0;
		ip_pkt_monotor_orientation = 0;
		switch(_cmd)
		{
		case	__IP_STATE_CMD_init_buf__:
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-1");
			break;
		case	__IP_STATE_CMD_wr_hw__:
			ip_pkt_monotor_sta = __IP_STATE_playback_progress__;
//			__FIf__->_past_msec_(1);
			__HLog__->HldPrint("[ip-recv] : initial starting ip pkt monitor.");
			break;
		case	__IP_STATE_CMD_detected_under__:
			ip_pkt_monotor_sta = __IP_STATE_transition_buf_underover__;
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-4");
			break;
		case	__IP_STATE_CMD_detected_stable__:
			ip_pkt_monotor_sta = __IP_STATE_playback_progress__;
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-5");
			break;
		}
		break;

	case	__IP_STATE_playback_progress__:
		switch(_cmd)
		{
		case	__IP_STATE_CMD_init_buf__:
			ip_pkt_monotor_sta = __IP_STATE_buf_fill_wait__;
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-2");
			break;
		case	__IP_STATE_CMD_wr_hw__:
			cnt_ip_wr_hw_pkt_bytes += (unsigned int)_size;
			break;
		case	__IP_STATE_CMD_rd_ip__:
			cnt_ip_recv_pkt_bytes += (unsigned int)_size;
			break;
		case	__IP_STATE_CMD_wrap_max_buf__:
			__HLog__->HldPrint_2("[ip-recv] : adj amount of pkts.", cnt_ip_recv_pkt_bytes, cnt_ip_wr_hw_pkt_bytes);
			if ((cnt_ip_recv_pkt_bytes >= 1000000000) && (cnt_ip_wr_hw_pkt_bytes >= 1000000000))
			{
				cnt_ip_recv_pkt_bytes -= 1000000000;
				cnt_ip_wr_hw_pkt_bytes -= 1000000000;
			}
			else
			{
				ip_pkt_monotor_sta = __IP_STATE_buf_fill_wait__;
				__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-3");
			}
			break;
		case	__IP_STATE_CMD_contents_modified__:
			if (_insert == 1)
			{
				cnt_ip_wr_hw_pkt_bytes -= 188;	//	minus no real data.
			}
			else if (_remove == 1)
			{
				cnt_ip_wr_hw_pkt_bytes += 188;	//	plus no real data added between in/out postion.
			}
			break;
		case	__IP_STATE_CMD_detected_under__:
			ip_pkt_monotor_sta = __IP_STATE_transition_buf_underover__;
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-6");
			break;
		}
		break;

	case	__IP_STATE_transition_buf_underover__:
		if (_cmd == __IP_STATE_CMD_detected_stable__)
		{
			ip_pkt_monotor_sta = __IP_STATE_buf_fill_wait__;
			__HLog__->HldPrint("[ip-recv] : flush estimated pkt-rate.-7");
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
int	CHldFmtrIp::LaunchIpBufProducer_Task(int *_bnk_size)
{
	CHld	*_hld_ = (CHld *)my_hld;
	int IpLock  = -1;		//-1:Unknown, 0:unlock, 1:lock
	//2014/01/08 IP improve
	while(__Sta__->Vlc_Recv_or_RecvRec())
	{
		if(bytes_ip_recv_buffer() <= 0)
		{
			IpLock  = 0;
			Sleep(10);
			continue;
		}
		int ipBitrate = bitrate_ip_recv_buffer();
		if(ipBitrate > 0)
		{
			__FIf__->SetPlayParam_PlayRate(ipBitrate);
			Set_IpStreaming_TxBitrate(ipBitrate);
			CalcBitRateAtStreamingStart();
			IpLock  = 1;
			break;
		}
	}

	if(IpLock == 0)
		return 0;

	if (_hld_->InitRcvIpEnv_OnPlayStart(_bnk_size) == 0)	//	task state is to be TH_START_PLAY after routine call TSPH_START_IP_STREAMING() by app.
	{
#if defined(WIN32)
		if ( _beginthread(_hld_->HLD_ip_PumpingThread, 0, (PVOID)_hld_) == -1 ) 
		{
			__HLog__->HldPrint("Hld-Mgr. FAIL to create pumping thread(VLC)");
		}
#else
		if ( pthread_create(&g_thread1, NULL, _hld_->HLD_ip_PumpingThread, (PVOID)_hld_) != 0 )
		{
			__HLog__->HldPrint("Hld-Mgr. FAIL to create pumping thread(VLC)");
		}
#endif
		Sleep(100); //	waiting task lauch.

		while ( CntBytesIntialFillIpBuf() >= CntBytesInIpBuf() )	//	initial waiting to get enough data. refer to HLD_ip_PumpingThread()
		{
			if ( __Sta__->ReqedNewAction_User() )
			{
				if (__Sta__->is_new_playback())
				{
					return 0;
				}
#ifdef STANDALONE
	printf("[Debug] Step 1 =====\n");
#endif
				break;
			}
			Sleep(10);
		}
		if(__Sta__->IsModTyp_DvbT2() || __Sta__->IsModTyp_DvbC2())
		{
			int	nSyncStartPos = -1, iTSize = 188;
			nSyncStartPos = _hld_->_FIf->TL_SyncLockFunction((char*)TL_ip_buffer, TL_ip_count, &iTSize, TL_ip_count, 3);

			if (nSyncStartPos != -1)
			{
				/*_hld_->TL_CurrentBitrate = */__FIf__->TL_Calculate_Bitrate(TL_ip_buffer, TL_ip_count, nSyncStartPos, iTSize, NULL, NULL);
				_hld_->g_CurrentPacketSize = iTSize;
#if 0
				__FIf__->SetPlayParam_PlayRate(TL_CurrentBitrate);
				__FIf__->TSPL_SET_PLAY_RATE(__FIf__->GetPlayParam_PlayRate(), __FIf__->GetPlayParam_OutClkSrc());
				accum_percent_[0] = accum_percent_[1] = accum_percent_[2] = accum_percent_[3] = accum_percent_[4] = 0;
				under_over_vector = 1.0;
				__estimated_out_speed_bytespersec = (float)_hld_->TL_CurrentBitrate/8;
				__estimated_in_speed_bytespersec = (float)_hld_->TL_CurrentBitrate/8;
				CalcBitRateAtStreamingStart();
#endif
			}
		}
	}
#ifdef STANDALONE
	printf("[Debug] Step 2 =====\n");
#endif
	return	1;
}
#ifdef WIN32
void CHldFmtrIp::HLD_ip_PumpingThread(PVOID param)
#else
void* CHldFmtrIp::HLD_ip_PumpingThread(PVOID param)
#endif
{
	CHld	*_hld_;
	DWORD	dwBytesRead;
	int	_rd_bytes_unit, nRet;

	_hld_ = (CHld *)param;
	if ( !_hld_ )
	{
#ifdef WIN32
		return;
#else
		return 0;
#endif
	}

	_hld_->_HLog->HldPrint("Hld-Mgr. Launch IpTask for receiving");

	_rd_bytes_unit = (_hld_->_FIf->SdramSubBankSize());
	_hld_->InitVariables_OnStartingIpTask();
	_rd_bytes_unit /= 4;
	while ( _hld_->_SysSta->IPUsing() && (_hld_->_SysSta->Vlc_Recv_or_RecvRec()) )
	{
		nRet = (int)_hld_->bytes_ip_recv_buffer();
		if ( nRet < _rd_bytes_unit )
		{
			Sleep(1);
			continue;
		}

		_hld_->LockIpMutex();

		dwBytesRead = _hld_->get_ip_streaming_buf((char*)_hld_->_FIf->__Buf_Temp_2(), _rd_bytes_unit, 0);	//	consume all-data of receiver.
		_hld_->ProducerIpStreamingBuffer_2(_hld_->_FIf->__Buf_Temp_2(), dwBytesRead);

		_hld_->UnlockIpMutex();
	}

	_hld_->ClrVariables_OnKillIpTask();

	_hld_->_HLog->HldPrint("Hld-Mgr. Term IpTask for receiving");

#ifdef WIN32
	return;
#else
	return 0;
#endif	
}



