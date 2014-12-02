
#if defined(WIN32)
#include	<Windows.h>
#else
#define	_FILE_OFFSET_BITS 64
#endif

#include	<stdio.h>
#include	<math.h>
#include	"LLDWrapper.h"
#include	"VLCWrapper.h"

#ifdef WIN32
#include "DVBT2_MultiplePLP.h"
#else
#include	"dvbt2_multipleplp.h"
#include	"../include/lld_const.h"
#include	"../include/lld_api.h"
#endif

#if defined(WIN32)

extern VLCWrapper *g_VLCWrapper;	//	????
#else
static VLCWrapper *g_VLCWrapper;	//	????
#endif

extern "C" _declspec(dllexport) int _stdcall	TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType);

#define GET_PROC(a,b,c)			{ b = (a)GetProcAddress(m_hInstance, c); }

//----------------------------------------------------------------------------------
CHld::CHld(int _my_id)	:	CHldFmtrIp(_my_id, (void *)this), CHldFmtrLoopThru(_my_id, (void *)this)
{
	my_hld_id = _my_id;

	TL_nDemuxBlockTest = 0;

	_HLog = new CHldBdLog();
	_SysSta = new CHldGVar();
	_FIf = new CHldFsRdWr();
	__Cmmb	= new CHldFmtrCmmb();	//	frame analyzer
	_FIf->SetCommonMethod_80(_SysSta, _HLog);
	_SysSta->SetCommonMethod_81(_HLog);

	_FIf->SetFmtrDependentVar_(&_FIf->TL_gPacketSize, &_FIf->TL_gTotalSendData, &TL_CurrentBitrate);
	_FIf->SetCommonMethod_20(_SysSta, _HLog);

	SetCommonMethod_1(_SysSta, _FIf, _HLog);
	SetCommonMethod_2(_SysSta, _FIf, _HLog);
	SetCommonMethod_3(_SysSta, _FIf, _HLog);
	SetCommonMethod_4(_SysSta, _FIf, _HLog, __Cmmb);
	SetCommonMethod_5(_SysSta, _FIf, _HLog);
	__Cmmb->SetCommonMethod_6(_SysSta, _FIf, _HLog);
	SetCommonMethod_7(_SysSta, _FIf, _HLog);
	SetCommonMethod_8(_SysSta, _FIf, _HLog);
	SetCommonMethod_81(_SysSta, _FIf, _HLog);
	SetCommonMethod_82(_SysSta, _FIf, _HLog);
	SetCommonMethod_C2(_SysSta, _FIf, _HLog);
	SetCommonMethod_9(_SysSta, _FIf, _HLog);
	SetTdmbSyncBuf_Allocated(_FIf->__Buf_Temp_2(), _FIf->__BufPlay_Has_RstDtaAllConversion(), _FIf->__Buf_Temp(), _FIf->__BufPlay_2());
	SetTdmbSyncBuf_Pos(&_FIf->TL_nWritePos, &_FIf->TL_nReadPos, &_FIf->TL_nBufferCnt);
}

CHld::~CHld()
{
	delete	__Cmmb;
	delete	_SysSta;
	delete	_FIf;
	delete	_HLog;

}

int CHld::AtStartingPos_RcvIp_or_RdFile_OnPlayCont__HasEndlessWhile(void)
{
	int		nBankBlockSize = (_FIf->SdramSubBankSize());

#ifdef IP_STREAM
	if ( _SysSta->IPUsing() && (_SysSta->Vlc_Recv_or_RecvRec()) )
	{
		_FIf->Set_ReadBytes_PlayBuf(nBankBlockSize);
		if (Get_RcvIpDta( _FIf->__Buf_Temp(), _FIf->TL_dwBytesRead) == 0)
		{
			return 0;
		}
		_FIf->FillPlayBuf_from_GivenBuf_w_ChkBoundary(_FIf->__Buf_Temp(), _FIf->TL_dwBytesRead);
		_FIf->TL_gFirstRead = 0;
	}
	else if ( g_VLCWrapper && (_SysSta->Vlc_Recv_or_RecvRec()) )
	{
		//if ( g_hPumpingEvent ) 
		{
			g_VLCWrapper->g_PumpingWait = 0;
		//SetEvent(g_hPumpingEvent);
			while (_SysSta->IsTaskState_ContPlay() && !_SysSta->ReqedNewAction_User() && !g_VLCWrapper->g_PumpingWait ) 
			{
				Sleep(5);
			}
		}
	}
	else	//	read from file.
	{
		/* ISDB-T 1SEG (=9), ISDB-T 13SEG (=10) ONLY. FROM TMCC REMUXER. */
		if ( (_SysSta->IsModTyp_IsdbT_13()) && ( _FIf->TmccUsing() ) )
		{
			if (RdPlayDta_From_TmccRemuxer() == 0)
			{
				return 0;
			}
		}
		else
		{
			_FIf->Fread_into_PlayBuf_RunningPos_1stRead();
		}
	}
#else
	_FIf->Fread_into_PlayBuf_RunningPos_1stRead();
	
#endif

	return	1;
}
int CHld::ProcessDta_1stLoop_RcvIp_or_RdFile_Dta_OnPlayCont(void)
{
	int nRet;
	int		nBankBlockSize = (_FIf->SdramSubBankSize());

	/* TDMB(=5) */
	if ( _SysSta->IsModTyp_Tdmb() )
	{
		//No check sync.
	}
	/* CMMB(=12) */
	else if ( _SysSta->IsModTyp_Cmmb() )
	{
		//No check sync.
	}
	/* CHECK SYNC. POSITION AND PACKET SIZE */
	else
	{
		//fixed - 6.10.05, 20 -> 3
		nRet = _FIf->TL_SyncLockFunction((char*)(_FIf->TL_szBufferPlay), nBankBlockSize, &(_FIf->TL_gPacketSize), nBankBlockSize, 3);
		if ( nRet != -1 )
		{
			_FIf->TL_gSyncPos = nRet;
			if( _SysSta->IsModTyp_AtscMH() )
			{
				m_MHE_packet_PID = _FIf->Get_MPE_Packet_PID(_FIf->TL_szBufferPlay, nBankBlockSize, _FIf->TL_gSyncPos, _FIf->TL_gPacketSize);
				InitMHEInfo(m_MHE_packet_PID);
			}
		}
		else
		{
			_HLog->HldPrint_1_s("Hld-Bd-Ctl. FAIL to calculate PACKET SIZE and BITRATE of ",_FIf->GetPlayParam_FName());
			_FIf->TL_gFirstRead = 1;
			return 0;
		}

		if ( _FIf->TL_gPCR_Restamping == 1 )
		{
			_FIf->BSTARTClick(_FIf->GetPlayParam_PlayRate(), _FIf->GetBitRate());
		}
	}

	return	1;
}
int CHld::ProcessDta_2ndLoop_RcvIp_or_RdFile_Dta_OnPlayCont(void)
{
	int nRet;
	int		nBankBlockSize = (_FIf->SdramSubBankSize());
	DWORD	dwRet;

	/* TDMB(=5) */
	if ( _SysSta->IsModTyp_Tdmb() )
	{
		//No check sync.
		InitializeNAToNI(0);
	}
	/* CMMB(=12) */
	else if ( _SysSta->IsModTyp_Cmmb() )
	{
		TL_InitBufferVariables();
	}
	/* ISDB-T-13(=10) ONLY. */
	else if ( _SysSta->IsModTyp_IsdbT_13() && (_FIf->IsTheFileNot_PlayParam_FName(_FIf->g_Current_Target) != 0))
	{
		_FIf->SetPlay__FName(_FIf->GetPlayParam_FName());
		TL_InitBufferVariables();
		InitPlayParameters_IsdbT13();
		return 0;
	}
	/* CHECK PACKET SIZE AGAIN. */
	else
	{
		nRet = _FIf->TL_SyncLockFunction((char*)(_FIf->TL_szBufferPlay + _FIf->TL_nWIndex), nBankBlockSize, (int*)&dwRet, nBankBlockSize, 3);
		if( _SysSta->IsModTyp_AtscMH() )
		{
			m_MHE_packet_PID = _FIf->Get_MPE_Packet_PID((_FIf->TL_szBufferPlay + _FIf->TL_nWIndex), nBankBlockSize, nRet, dwRet);
			InitMHEInfo(m_MHE_packet_PID);
		}
		
		if ( (int)dwRet != _FIf->TL_gPacketSize )
		{
			_FIf->TL_gPacketSize = dwRet;
			TL_InitBufferVariables();
			InitPlayParameters_IsdbT13();
			return 0;
		}
		_FIf->TL_gSyncPos = nRet;

		//FIXED - Bitrate adjustment
		if ( _FIf->IsTheFileNot_PlayParam_FName(_FIf->g_Current_Target) != 0 )
		{
			_FIf->SetPlay__FName(_FIf->GetPlayParam_FName());
			if ( _FIf->TL_gPCR_Restamping == 1 )
			{
				_FIf->TL_gBitrate = _FIf->TL_Calculate_Bitrate(
						_FIf->TL_szBufferPlay + _FIf->TL_nWIndex, nBankBlockSize, _FIf->TL_gSyncPos, _FIf->TL_gPacketSize,
						&m_MHE_packet_PID, &m_MHE_packet_index);
				if ( _FIf->TL_gBitrate > 100 )
				{
					_FIf->BSTARTClick(_FIf->PlayParm.AP_lst.nPlayRate, _FIf->TL_gBitrate);
				}
			}
		}
		else
		{
			if ( _FIf->TL_gPCR_Restamping == 1 )
			{
				_FIf->BSTARTClick(_FIf->PlayParm.AP_lst.nPlayRate, _FIf->TL_gBitrate);
			}
		}
	}

	return	1;
}
int CHld::AtStartingPos_FmtrDta_RcvIp_or_RdFile_OnPlayCont(void)
{
	DWORD	dwRet;
	int		nBankBlockSize = (_FIf->SdramSubBankSize());

	_FIf->SetNewWrAlignPos_of_PlayBuf(nBankBlockSize);
	
	/* TDMB */
	if ( _SysSta->IsModTyp_Tdmb() )
	{
		FillTdmb_CifBuf_OnPlayCont();
	}
	/* CMMB(=12) */
	else if ( _SysSta->IsModTyp_Cmmb() )
	{
		__Cmmb->TL_FindStartOfMultiplexFrame();
	}
	/* ISDB-T(=9). 204TS TO 204TS */
	else if ( _SysSta->IsModTyp_IsdbT_1() )
	{
		dwRet = _FIf->TL_gPacketSize;//==204, 208
		_FIf->FindSyncPos_and_FillSyncBuf(_FIf->TL_gPacketSize, &dwRet);
	}
	/* ISDB-T 13SEG (=10) ONLY. DELAY ADJUSTMENT */
	else if ( _SysSta->IsModTyp_IsdbT_13() )
	{
		TL_DelayAdjustment();
	}
	/* 204/208TS TO 188TS */
	else
	{
		dwRet = 188;
		_FIf->FindSyncPos_and_FillSyncBuf(_FIf->TL_gPacketSize, &dwRet);
	}

	return	1;
}
int CHld::FillPlayBuf_by_Dta_of_RdFile_OnPlayCont__HasEndlessWhile(int *_red_thisturn)	//	read file or remuxer and fill buf TL_szBufferPlay
{
	int		nBankBlockSize = (_FIf->SdramSubBankSize());
	unsigned char	*_b_to_read;

	_b_to_read = _FIf->__BufPlay();

#ifdef IP_STREAM
	if ( _SysSta->IPUsing() && (_SysSta->Vlc_Recv_or_RecvRec()) )
	{
	}
	else if ( g_VLCWrapper && (_SysSta->Vlc_Recv_or_RecvRec()) )
	{
	}
	else
	{
		_HLog->HldPrint_FillPlayBuf_FileDta(_SysSta->TL_gCurrentModType);

		/* ISDB-T 1SEG (=9), ISDB-T 13SEG (=10) ONLY. FROM TMCC REMUXER. */
		if ( (_SysSta->IsModTyp_IsdbT_13()) && ( _FIf->TmccUsing() ) )
		{
			char *tmp;
			int ReadPos, BufferCnt, EndOfRemux;
			tmp = TMCC_READ_REMUX_DATA(nBankBlockSize, &ReadPos, &BufferCnt, &EndOfRemux);
			if (tmp)
			{
				memcpy(_b_to_read+_FIf->TL_nWIndex, tmp, nBankBlockSize);
				_FIf->TL_dwBytesRead = nBankBlockSize;

				if ( EndOfRemux == 1 )
				{
					if ( BufferCnt < nBankBlockSize )
					{
						_FIf->TL_dwBytesRead = BufferCnt;
					}
				}
			}
			else
			{
				if ( EndOfRemux == 1 )
					memset(_b_to_read + _FIf->TL_nWIndex, 0, nBankBlockSize);
				else
					return 0;
			}
		}
		/* FROM FILE */
		else
		{
			//if ( _SysSta->IsModTyp_DvbT2() && _FIf->IsT2miFile() == 0 )
			//{
			//	LockT2miMutex();
			//}
			if ( _FIf->TL_nWIndex + nBankBlockSize > MAX_BUFFER_BYTE_SIZE )
			{
				nBankBlockSize = MAX_BUFFER_BYTE_SIZE - _FIf->TL_nWIndex;
			}

			//FIXED - Bitrate adjustment
			if ( _FIf->gfRatio > 1 && _FIf->TL_nBCount > (unsigned int)(MAX_BUFFER_BYTE_SIZE - nBankBlockSize) )
			{
			}
			else
			{
				_FIf->Fread_into_PlayBuf_RunningPos(nBankBlockSize);
			}
		}
	}
#else	//	IP_STREAM
#endif
	*_red_thisturn  = nBankBlockSize;

	return	1;
}
int CHld::UpdPlayBufWIndex_SendIpQ_OnPlayCont(int _red_thisturn)
{
	/* TS BUFFER FILLED. */
	if ( _FIf->TL_dwBytesRead > 0 )
	{
		if ( _SysSta->IPUsing() && (_SysSta->Vlc_Recv_or_RecvRec()) )
		{
			;
		}
		else
		{
			_FIf->SetNewWrAlignPos_of_PlayBuf(_red_thisturn);
		}
	}

	return	1;
}
int CHld::AdjLoopStartingPos_OnPlayEnd(void)
{
	DWORD	dwRet;

	/* RESTAMPING, TDT/TOT */
	//TDT/TOT - USER DATE/TIME
	if (_FIf->PlayParm.AP_fRepeat == 0 && (_FIf->IsUsrReqed_DateTime_or_Restamp()))
	{
		_FIf->SetTimeDiff_at_LoopEnd();	//	to adj TDT/TOT

		/* RESTAMPING */
		if ( _FIf->IsUsrReqed_Restamp() )
		{
			//2014/1/6 fixed adaptation
			_FIf->TL_nAdaptationPos = _FIf->TL_nWIndex - _FIf->TL_nRIndex;
			if(_FIf->TL_nAdaptationPos < 0)
				_FIf->TL_nAdaptationPos = _FIf->TL_nAdaptationPos + MAX_BUFFER_BYTE_SIZE;
			/* Calc. time offset */
			double dblTime;// to sec
			double dblBitrate = _FIf->TL_gBitrate/8.0;// to bytes
			if ( dblBitrate <= 0 ) 
			{
				dblBitrate = _FIf->PlayParm.AP_lst.nPlayRate/8.0;
			}
			
			/* valid_sent_data = senddata * 188/packetsize */
			if (dblBitrate != 0) {	
				if (_FIf->TL_gPacketSize !=0)
					dblTime = ((_FIf->TL_gTotalSendData*188 + _FIf->TL_nAdaptationPos) /_FIf->TL_gPacketSize)/dblBitrate;
				else
					dblTime = _FIf->TL_gTotalSendData/dblBitrate;
				_FIf->TL_giiTimeOffset = (__int64) (27000000*dblTime);
			}

			/* 204/208 Packet */
			//if ( _FIf->TL_gPacketSize == 204 || _FIf->TL_gPacketSize == 208 )
			//{
			//	if (dblBitrate != 0) 
			//	{	
			//		dblTime = _FIf->TL_gTotalSendData/dblBitrate;
			//		_FIf->TL_giiTimeOffset = (__int64) (27000000*dblTime);
			//	}
			//}
		}

		/* RESTAMPING LOGGING */
		if ( _FIf->TL_gRestampLog == 1 )
			_HLog->HldPrint("Hld-Mgr. ==========================================================");

		/* START OFFSET CHANGE. OBSOLETE */
#if defined(WIN32)
		dwRet = SetFilePointer(_FIf->AP_hFile, gDblStartOffsetL, &(gDblStartOffsetH), FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)_FIf->AP_hFile, (off_t)(gDblStartOffsetL<<32 + gDblStartOffsetH), SEEK_SET);
#endif
		_FIf->TL_gFirstRead = 1;
		//2014/1/4
		_FIf->TL_gNumLoop++;
		_SysSta->SetMainTask_LoopState_(TH_CONT_PLAY);
		_FIf->TL_CheckBufferStatus(2, _FIf->GetPlayParam_PlayRate());

		Restart_TmccRemuxerAtEof_IsdbT_1_13();
		return 0;
	}

	return	1;
}

int CHld::CurOffsetChanged_by_User_OnPlayCont(void)
{
	_FIf->_Fseek_CurOff_OnPlayCont();
	if ( _SysSta->IsModTyp_Tdmb() )
	{
		InitializeNAToNI(0);
	}
	SetOffset_TmccRemuxer(0);

	return	1;
}
int CHld::SectionRepeatChanged_by_User_OnPlayCont(void)
{
	if (_FIf->_Fseek_EndOff_OnPlayCont() == 0)
	{
		if ( _SysSta->IsModTyp_Tdmb() )
		{
			InitializeNAToNI(0);
		}
	}
	SetOffset_TmccRemuxerRepeatCond();

	return	1;
}
int CHld::FirstFRead_OnPlayCont(void)	//	read file or remuxer or ip. and set initial cond.
{
	if (_FIf->_Fseek_StartingOff_OnPlayCont() == 0)
	{
		SetOffset_TmccRemuxer(0);
	}
	if (AtStartingPos_RcvIp_or_RdFile_OnPlayCont__HasEndlessWhile() == 0)
	{
		return	0;
	}
	if ( _FIf->TL_gNumLoop == 0 ) //	playback loop number.
	{	
		if (ProcessDta_1stLoop_RcvIp_or_RdFile_Dta_OnPlayCont() == 0)
		{
			return	0;
		}
	}
	else	//	not 1st loop.
	{
		if (ProcessDta_2ndLoop_RcvIp_or_RdFile_Dta_OnPlayCont() == 0)
		{
			return	0;
		}
	}
	AtStartingPos_FmtrDta_RcvIp_or_RdFile_OnPlayCont();

	return	0;
}

int CHld::GetPlay_BufSts(void)
{
	if (_FIf->user_play_buffer_status == _FIf->play_buffer_status)
	{
		if (_FIf->play_buffer_status == BUFF_STS_COMPLETED_A_STREAM_OUT_NOW)
		{
			//_HLog->HldPrint("Hld-Bd-Ctl. TSPH_GET_PLAY_BUFFER_STATUS[%d]======= [%i]\n",nSlot, BUFF_STS_COMPLETED_A_STREAM_OUT_NOW);
			return	0;
		}
	}
	else
	{
		_FIf->user_play_buffer_status = _FIf->play_buffer_status;
	}
	//_HLog->HldPrint("Hld-Bd-Ctl. TSPH_GET_PLAY_BUFFER_STATUS[%d] [%i]\n",nSlot, user_play_buffer_status);
	return	_FIf->user_play_buffer_status;
}
void	CHld::WaitAppCntl_NxtFileReady_OnPlayEnd__HasEndlessWhile(void)	//	non sense
{
	/* WAIT UNTIL APPLICAITON SET "next_file_ready" flag again. */
	_FIf->TL_nEndPlayCnt = 0;
	DWORD dwOneBankTime = (1048576*8*8) / _FIf->PlayParm.AP_lst.nPlayRate;

	//I/Q PLAY/CAPTURE
	if ( TL_gIQ_Play_Capture == 1 )
	{
		dwOneBankTime = (unsigned long)(500000000. / _FIf->PlayParm.AP_lst.nPlayRate);//???
	}
	else
	{
		dwOneBankTime = (unsigned long)(90000000. / _FIf->PlayParm.AP_lst.nPlayRate);//???
	}
	dwOneBankTime *= 1000;
	while ( dwOneBankTime -= 10 )
	{
		//FIXED
		if ( dwOneBankTime < 10 )
		{
			break;
		}

		if ( _SysSta->ReqedNewAction_User() )
			break;

		_FIf->TL_CheckBufferStatus(3, _FIf->GetPlayParam_PlayRate());

		if (_FIf->PlayParm.AP_fRepeat == 0)	
		{
			if ( _FIf->next_file_ready == TRUE )
				break;
		}
		else	
		{
			_FIf->next_file_ready = TRUE;
			break;
		}


		Sleep(10);
	}

}

int CHld::SetTsio_Direction__HasEndlessWhile(int nDirection)
{
	int	gRet;
	int tmp = _SysSta->Flag_DtaPathDirection();

	//2012/4/3
	if(_SysSta->_CntAdditionalVsbRfOut_593_591s() || _SysSta->_CntAdditionalQamRfOut_593_591s() ||
		 _SysSta->_CntAdditionalDvbTRfOut_593())	//2012/6/28 multi dvb-t
	{
		CHld	*_hld_2, *_hld_3, *_hld_4;
		
		_hld_2 = (CHld *)_SysSta->multits_2_hld;
		_hld_3 = (CHld *)_SysSta->multits_3_hld;
		_hld_4 = (CHld *)_SysSta->multits_4_hld;
		
		if (_hld_2 != NULL  && (_hld_2->_SysSta->IsTaskState_ContPlay() == 0))
		{
			if(_hld_2->_SysSta->IsNullTP_Enabled() == 0)
				_hld_2->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_2->_SysSta->m_nBoardId, 1);
		}
		if (_hld_3 != NULL  && (_hld_3->_SysSta->IsTaskState_ContPlay() == 0))
		{
			if(_hld_3->_SysSta->IsNullTP_Enabled() == 0)
				_hld_3->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_3->_SysSta->m_nBoardId, 1);
		}
		if (_hld_4 != NULL  && (_hld_4->_SysSta->IsTaskState_ContPlay() == 0))
		{
			if(_hld_4->_SysSta->IsNullTP_Enabled() == 0)
				_hld_4->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_4->_SysSta->m_nBoardId, 1);
		}
		if( nDirection == TSIO_310_LOOPTHRU || nDirection == TSIO_ASI_LOOPTHRU )
		{
			_FIf->TVB380_SET_MODULATOR_OUTPUT(_SysSta->m_nBoardId, 0);
		}
		else
		{
			if(_SysSta->IsNullTP_Enabled() == 0)
				_FIf->TVB380_SET_MODULATOR_OUTPUT(_SysSta->m_nBoardId, 1);
		}

	}
	
	//I/Q PLAY/CAPTURE
	if ( Flag_IqPlayCap() )
	{
		if ( nDirection == TSIO_310_LOOPTHRU || nDirection == TSIO_ASI_LOOPTHRU )
		{
			_FIf->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_CAPTURE_MODE);
		}
		else
		{
			_FIf->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_PLAY_MODE);
		}
	}

	//ATSC-M/H - TEST
	if ( _SysSta->IsModTyp_AtscMH() )
	{
		if ( nDirection == TSIO_PLAY_WITH_310INPUT )
		{
			_SysSta->SetFlag_DtaPathDirection(nDirection);
		}
		else
		{
			if ( nDirection == TSIO_310_LOOPTHRU ) 
			{
				nDirection = TSIO_310_CAPTURE_PLAY;
			}
			else if ( nDirection == TSIO_ASI_LOOPTHRU ) 
			{
				nDirection = TSIO_ASI_CAPTURE_PLAY;
			}
			_SysSta->SetFlag_DtaPathDirection(TSIO_PLAY_WITH_310INPUT);
		}
	}
	else
	{
		_SysSta->SetFlag_DtaPathDirection(nDirection);
	}

	while ( _SysSta->Flag_PlayLoopThru() || _SysSta->Flag_CapLoopThru() )
	{
		if ( _SysSta->TL_fThreadActive == FALSE )
		{
			return -1;
		}                                     
		Sleep(10);
	}

	//ATSC-M/H - TEST
	if ( _SysSta->IsModTyp_AtscMH() )
	{
		_SysSta->SetFlag_DtaPathDirection(nDirection);
	}

	gRet = _FIf->TSPL_SET_TSIO_DIRECTION(nDirection);

	if ( _SysSta->IsTaskState_ContMon() )
	{
#if 0
		if ( _SysSta->IsAttachedTvbTyp_SupportIsdbTLoopThru() && _SysSta->IsModTyp_IsdbT_13() && (nDirection == TSIO_ASI_LOOPTHRU || tmp == TSIO_ASI_LOOPTHRU) )
		{
			_SysSta->SetMainTask_LoopState_(TH_START_MON);
		}
		//ATSC-M/H
		if ( _SysSta->IsModTyp_AtscMH() && ((nDirection == TSIO_310_CAPTURE_PLAY || tmp == TSIO_310_CAPTURE_PLAY) || (nDirection == TSIO_ASI_CAPTURE_PLAY || tmp == TSIO_ASI_CAPTURE_PLAY)) )
		{
			_SysSta->SetMainTask_LoopState_(TH_START_MON);
		}
		//2011/8/5 ISDB-S ASI
		if ( _SysSta->IsAttachedTvbTyp_SupportIsdbSLoopThru() && _SysSta->IsModTyp_IsdbS() && (nDirection == TSIO_ASI_LOOPTHRU || tmp == TSIO_ASI_LOOPTHRU) )
		{
			_SysSta->SetMainTask_LoopState_(TH_START_MON);
		}
		//2012/7/6 DVB-T2 ASI
		if ( _SysSta->IsAttachedTvbTyp_SupportDvbT2LoopThru() && _SysSta->IsModTyp_DvbT2() && (nDirection == TSIO_ASI_LOOPTHRU || tmp == TSIO_ASI_LOOPTHRU) )
		{
			_SysSta->SetMainTask_LoopState_(TH_START_MON);
		}
		//2012/7/24 DVB-C2 ASI
		if ( _SysSta->IsAttachedTvbTyp_SupportDvbC2LoopThru() && _SysSta->IsModTyp_DvbC2() && (nDirection == TSIO_ASI_LOOPTHRU || tmp == TSIO_ASI_LOOPTHRU) )
		{
			_SysSta->SetMainTask_LoopState_(TH_START_MON);
		}
#endif
	}

	return gRet;
}

//----------------------------------------------------------------------------------
/* TVB390V8 Demux Block Test Mode */
//
// Record 1 Mbyte without checking sync. bytes
//
int	CHld::TL_ProcessDemuxBlockCapture(HANDLE hFile, int NewModeRequest)
{
	int i;
	DWORD	dwSizeWritten = 0;
#if defined(WIN32)
	if ( hFile == INVALID_HANDLE_VALUE )
		return 0;

	for (i = 0; (i < _SysSta->iHW_BANK_NUMBER+1) && (_SysSta->IsTaskState_ContRec()) && !NewModeRequest; i++)
	{
		dwSizeWritten = 0;

		_FIf->TL_pdwDMABuffer = (unsigned long*)_FIf->TSPL_READ_BLOCK((_FIf->SdramSubBankSize()));
		if (_FIf->TL_pdwDMABuffer != NULL)
		{
			WriteFile(hFile, (unsigned char *)_FIf->TL_pdwDMABuffer, (_FIf->SdramSubBankSize()), &dwSizeWritten, NULL);
			_HLog->HldPrint_1("(W:)",(int)dwSizeWritten);
			_FIf->TL_i64TotalFileSize += dwSizeWritten;	
		}
		else
		{
			_HLog->HldPrint("Hld-Bd-Ctl. FAIL to read BLOCK in REC");
		}
	}
#endif	
	return 0;
}

//
// Play 1 Mbyte without checking sync. bytes
//
int	CHld::TL_ProcessDemuxBlockPlay(HANDLE hFile, int NewModeRequest)
{
	int i;
	DWORD	dwBytesRead = 0;
#if defined(WIN32)
	if ( hFile == INVALID_HANDLE_VALUE )
		return 0;

	for (i = 0; (i < _SysSta->iHW_BANK_NUMBER+1) && (_SysSta->IsTaskState_ContPlay()) && !NewModeRequest; i++)
	{
		dwBytesRead = 0;

		memset(_FIf->TL_szBufferPlay, 0, (_FIf->SdramSubBankSize()));
		ReadFile(hFile, _FIf->TL_szBufferPlay, (_FIf->SdramSubBankSize()), &dwBytesRead, NULL);

		memcpy((void *)_FIf->TL_pdwDMABuffer, _FIf->TL_szBufferPlay, dwBytesRead);
		_FIf->TSPL_WRITE_BLOCK(NULL, (unsigned long)dwBytesRead, (unsigned long *)_FIf->TL_dwAddrDestBoardSDRAM);

		/* Reset buffer state */
		_FIf->TL_CheckBufferStatus(1, _FIf->GetPlayParam_PlayRate());
	}
#endif
	return 0;
}







//////////////////////////////////////////////////////////////////////




int CHld::TL_Calculate_L1_Post_Size(int L1_MOD, int FFT_SIZE, int L1_POST_INFO_SIZE)
{
	double K_post_ex_pad, N_post_FEC_Block, K_L1_PADDING, K_post, K_sig, N_L1_mult, N_punc_temp, N_bch_parity, R_elf_16k_LDPC_1_2, N_post_temp, N_post, N_MOD_per_Block, N_MOD_Total;
	
	double N_ldpc = 16200;
	double K_bch = 7032;

	double N_mod = 1;
	if ( L1_MOD == 0x000 )		N_mod = 1;
	else if ( L1_MOD == 0x001 )	N_mod = 2;
	else if ( L1_MOD == 0x002 )	N_mod = 4;
	else if ( L1_MOD == 0x003 )	N_mod = 6;

	double N_P2 = 16;//==FFT 1k
	if ( FFT_SIZE == 3 )						N_P2 = 16;
	else if ( FFT_SIZE == 0 )					N_P2 = 8;
	else if ( FFT_SIZE == 2 )					N_P2 = 4;
	else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	N_P2 = 2;
	else if ( FFT_SIZE == 4 )					N_P2 = 1;
	else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	N_P2 = 1;
	
	K_post_ex_pad = L1_POST_INFO_SIZE + 32;
	N_post_FEC_Block = (int)ceil(K_post_ex_pad / K_bch);
	K_L1_PADDING = (int)ceil(K_post_ex_pad / N_post_FEC_Block) * N_post_FEC_Block - K_post_ex_pad;
	K_post = (int)(K_post_ex_pad + K_L1_PADDING);
	K_sig  = (int)(K_post / N_post_FEC_Block);
	if (N_P2 == 1)
	{
		N_L1_mult = 2 * N_mod;
	}
	else
	{
		N_L1_mult = N_P2 * N_mod;
	}

	N_punc_temp  = (int)floor((6./5.) * (K_bch - K_sig));
	if ( N_punc_temp < (N_L1_mult - 1) )
	{
		N_punc_temp = (N_L1_mult - 1);
	}

	R_elf_16k_LDPC_1_2 = 4./9.;
	N_bch_parity = 168.;
	N_post_temp = (int)(K_sig + N_bch_parity + N_ldpc * (1 - R_elf_16k_LDPC_1_2) - N_punc_temp);
	if (N_P2 == 1)
	{
		N_post = (int)ceil(N_post_temp / (2*N_mod)) * (2*N_mod);
	}
	else
	{
		N_post = (int)ceil(N_post_temp / (N_mod * N_P2)) * (N_mod * N_P2);
	}

	N_MOD_per_Block =  (int)(N_post / N_mod);
	N_MOD_Total = (int)(N_MOD_per_Block * N_post_FEC_Block);

	return (int)N_MOD_Total;
}



//------------------------------------------------------------------------
/* TEST/DEBUGGING */
#define	SYNC_PATTERN		0x47474747
#define	BLOCK_SIZE			65535
#define SKIP16M_POS			(16*1024*1024)

void CHld::TL_TestTSPattern(char* szFileName)
{
	FILE	*rdfp;
	int		cnt;
	int		csum;
	int		rd;
	int		err, block;


	if( (rdfp = fopen( szFileName, "rb" )) != NULL )
	{
		if (fseek(rdfp, SKIP16M_POS, SEEK_SET)){
			_HLog->HldPrint("too small file size");
			fclose( rdfp );
			return;
		}

		_HLog->HldPrint_1("skip garbage bytes", SKIP16M_POS);

		err = 0;
		block = 0;
		while( !feof( rdfp ) )
		{
			fread( &rd, sizeof( int ), 1, rdfp );
			if (rd == SYNC_PATTERN){
				csum = rd;
				cnt  = 0;
			}
			else{
				csum += rd;
				cnt++;
			}

			if (cnt == BLOCK_SIZE){
				block++;
				if ((block % 10) == 0)
					_HLog->HldPrint_1("block tested", block);
				if (csum != 0){
					_HLog->HldPrint_2("chksum error at block, csum ", block, csum);
					err++;
				}
			}
		}
		
		fclose( rdfp );
		_HLog->HldPrint("chksum test completed, total block  ( MB), error block ", block, (block >> 2) , err);
	}
	else{
      _HLog->HldPrint_1_s("File could not be opened", szFileName );
	}

	return;

	/*
	HANDLE hFile;
	unsigned long dwRet, dwBytesRead;
	__int64 iiStartOffsetL, iiStartOffsetH; 
	unsigned char* pBlock = NULL;

	pBlock = (unsigned char*)malloc(nBlockSize);
	if (pBlock == NULL)
	{
		_HLog->HldPrint("Hld-Bd-Ctl. FAIL to alloc. the memory [%d] for Pattern Test [%s]\n", nBlockSize, szFileName);
		return;
	}

	hFile = CreateFile(szFileName,	GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		free(pBlock);

		_HLog->HldPrint("Hld-Bd-Ctl. FAIL to open a FILE[%s] for Pattern Test\n", szFileName);
		return;
	}
	
	// Apply start offset
	iiStartOffsetL = (long)((iiOffset & 0xFFFFFFFF)/1024)*1024;
	iiStartOffsetH = (long)(iiOffset >> 32);
	dwRet = SetFilePointer(hFile, (long)iiStartOffsetL, (long*)&iiStartOffsetH, FILE_BEGIN);

	// TEST : check-sum
	int	cnt, csum, err, block, rd;
	unsigned int i;

	cnt = csum = err = block = 0;
	i = 0;
	do
	{
		// Read block to test
		dwRet = ReadFile(hFile, pBlock, nBlockSize, &dwBytesRead, NULL);
		
		for ( i = 0; i < dwBytesRead; i += sizeof(int) )
		{
			memcpy(&rd, pBlock+i, sizeof(int));

			if ( rd == SYNC_PATTERN )
			//if ( rd == (int)pattern )
			{
				csum = rd;
				cnt  = 0;
			}
			else
			{
				csum += rd;
				cnt++;
			}

			if ( cnt == BLOCK_SIZE )
			//if ( cnt == nBlockSize )
			{	
				block++;
				if ((block % 10) == 0)
				{
					_HLog->HldPrint("Hld-Bd-Ctl. [%d] block tested\n", block);
				}
				if (csum != 0)
				{
					_HLog->HldPrint("Hld-Bd-Ctl. chksum error at %d block, csum=%08x\n", block, csum);
					err++;
				}
			}
		}
		
	} while ( dwBytesRead == (unsigned int)nBlockSize );
			
	free(pBlock);
	CloseHandle(hFile);
	*/
}

