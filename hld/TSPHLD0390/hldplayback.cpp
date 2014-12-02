//*************************************************************************
//	HLDmain.c / TSPHLD0381.DLL, TSPHLD0390.DLL, TSPHLD0431.DLL
//
//	TVB370/380/390/590/595, TSE110 high level DLL
//
//	
//	Jan. 17, 2009
//	Copyright (C) 2006-2009
//	Teleview Corporation
//	
//	This DLLs are developed to support Teleview TVB370/380/390/590/595, TSE110 board.
//
//*************************************************************************

#if defined(WIN32)
#include	<stdio.h>
#include	<windows.h>
#include	<io.h>
#include	<fcntl.h>
#include	<process.h>
#include	<time.h>
#else
#define	_FILE_OFFSET_BITS	64
#define	_LARGEFILE_SOURCE	1
#include	<stdio.h>
#include	<pthread.h>
#endif

#if defined(WIN32)
#include	"inc/hldmgr.h"
#else
#include	"hldmgr.h"
#include	"../../include/lld_const.h"
#endif
#include	"../include/hld_structure.h"


/////////////////////////////////////////////////////////////////
CHldPlay::CHldPlay(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;

}
CHldPlay::~CHldPlay()
{
}


int CHldPlay::HLD_TH_START_PLAY(int nSlot, int IP_RESET_ONLY)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

	int		nBankBlockSize = pLLD->_FIf->SdramSubBankSize();

//////////////////////////////////////////////////////////////////////////////////////	prepare enough data. in iq-mode
	if(pLLD->_SysSta->IsModTyp_IqPlay())
	{
		if (pLLD->Play_IQData_UntilStopCond__HasEndlessWhile() == 0)
		{
			return	0;
		}
	}
	//2012/3/15 BERT 187
	int nBertType = pLLD->_FIf->_VarTstPktTyp();

//////////////////////////////////////////////////////////////////////////////////////	open file to play.
	pLLD->_FIf->TL_Cont_cnt = 0;
	if (pLLD->_FIf->OpenFile_OnPlayStart() == 0)
	{
		//2012/3/15 BERT 187
#if 1
		if((nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23) && pLLD->_SysSta->is_Bert_187_Mod())
		{

		}
		else
		{
			return	0;
		}
#else
		return 0;
#endif
	}
	pLLD->_FIf->TL_CheckBufferStatus(0, pLLD->_FIf->GetPlayParam_PlayRate());
	pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_PLAY);	//	transit state.

//////////////////////////////////////////////////////////////////////////////////////	init. variables and buffer state.
	pLLD->Init_Valiables_OnPlayStart();
	pLLD->InitPlayParameters_IsdbT13();

	pLLD->InitBertVariable_OnPlayStart();

//////////////////////////////////////////////////////////////////////////////////////	task to fill HW-buf. and the re-consturcted data is to be writen by main-task
	if (pLLD->LaunchIpBufProducer_Task(&nBankBlockSize) == 0)
	{
		return	0;
	}
//////////////////////////////////////////////////////////////////////////////////////	task to write isbdS data to HW
	pLLD->LaunchIsdbS_WrTask();
	pLLD->InitIsdbSVariables_OnPlayStart();


//////////////////////////////////////////////////////////////////////////////////////	prepare hw to play.
	if (pLLD->_FIf->SetHwOptAndPrepareHwToPlay_OnPlayStart__HasEndlessWhile(
		nBankBlockSize,
		pLLD->_FIf->GetPlayParam_PlayRate(),
		pLLD->_FIf->GetPlayParam_OutClkSrc()) == 0)	//	hw-ctl. and pre-fill fpga-dma-data.
	{
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////////////	restart remuxer. isdbt-13seg only
	if (pLLD->StopAndClose_Restart_TmccRemuxer() == 0)	//	fail to restart.
	{
		pLLD->_SysSta->SetMainTask_LoopState_(TH_START_MON);
		return 0;
	}
	if ( pLLD->_SysSta->IsModTyp_IsdbT_13() )
	{
		pLLD->_FIf->TSPL_RESET_IP_CORE(pLLD->_SysSta->TL_gCurrentModType, 0x000003FF);	//	why???
	}

//////////////////////////////////////////////////////////////////////////////////////	start codec, if need.
	pLLD->RunVlc_OnPlayStart((void *)pLLD);

//////////////////////////////////////////////////////////////////////////////////////	init variables has depandency on time stamp.
	pLLD->_FIf->InitAdaptVariables_OnPlayStart();
	pLLD->__Cmmb->InitCmmbVariables_OnPlayStart();
//	pLLD->InitIsdbSVariables_OnPlayStart();

//////////////////////////////////////////////////////////////////////////////////////	dvb-t2 or isdbt-1seg
	if (pLLD->_SysSta->is_new_playback() && pLLD->_FIf->IsT2miFile() == 0)
	{
		pLLD->_FIf->TL_gNumLoop = 0;
		if ((pLLD->_FIf->TmccUsing()) && (pLLD->Current_Mode == 0) && pLLD->_SysSta->IsModTyp_IsdbT_1())
		{
			pLLD->_SysSta->SetMainTask_LoopState_(TH_START_MON);
			return 0;
		}
		pLLD->SetFinishAsi(0);
		pLLD->_SysSta->SetMainTask_LoopState_(TH_START_PLAY);
		playback_receive_request(pLLD, PLAY_MESSAGE_INIT, NULL);
		playback_receive_request(pLLD, PLAY_MESSAGE_PLAY, NULL);
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_PLAY);
		return 0;
	}

	pLLD->MultiStreamStartPlay();

	//2012/11/09
	pLLD->_SysSta->Start_Output_flag = 0;

	return 0;
}

int CHldPlay::HLD_TH_CONT_PLAY(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];

	if ( !pLLD )
	{
		return 1;
	}

	if(pLLD->_SysSta->ReqedStartDelay_User())
	{
		Sleep(1);
		return 0;
	}

	//2012/3/15 BERT 187
	int nBertType = pLLD->_FIf->_VarTstPktTyp();

//////////////////////////////////////////////////////////////////////////////////////	isdbS
	if (pLLD->IsdbS_ContFilePlayback() == 0)	//	endless until changing user state. and return 0 always, if isdbs-mod.
	{
		return 0;
	}

	if((nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23) && pLLD->_SysSta->is_Bert_187_Mod())
	{
		if(!pLLD->_SysSta->IsModTyp_IsdbT_13())
		{
			if(pLLD->Play_BERT187_EmptyFile_UntilStopContPlay__HasEndlessWhile() == 0)
				return 0;
		}
	}

	if(pLLD->_SysSta->IsModTyp_IqPlay())
	{
		if(pLLD->Play_IQData_UntilStopContPlay__HasEndlessWhile() == 0)
		//	endless until changing user state. and return 0 always, if iq-mod.
		{
			return 0;
		}
	}

	if(pLLD->_SysSta->is_new_playback() && pLLD->_FIf->IsT2miFile() == 0)	//	dbt-t2 or isdbt-1seg
	{
//		pLLD->FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile();
		if (pLLD->_FIf->g_CurrentOffsetChanged == 1)
		{
			long offset = pLLD->_FIf->g_CurrentOffsetL;
			playback_receive_request(pLLD, PLAY_MESSAGE_SEEK, &offset);
			pLLD->_FIf->g_CurrentOffsetChanged = 0;	
			
		}
		return 0;
	}

	int	nBankBlockSize = pLLD->_FIf->SdramSubBankSize();
	DWORD	dwRet;
	

//////////////////////////////////////////////////////////////////////////////////////	read cmmb data. and write hw. endless loop of state TH_CONT_PLAY
	if (pLLD->__Cmmb->PlaybackCmmbFile_OnPlayCont__HasEndlessWhile() == 0)
	{
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////////////
	pLLD->_FIf->next_file_ready = FALSE;	//	may reset by PlayParm.AP_fRepeat.
	if (pLLD->_FIf->IsInvalidFileHandle())
	{
		if (pLLD->_FIf->ChkAndOpenPlayFile_OnPlayCont() == 0)
		{
			return 0;
		}
		pLLD->Restart_TmccRemuxerFor_IsdbT_1_13();

		if (pLLD->LaunchVlcCodec_and_WaitStop_OnPlayCont((void *)pLLD) == 0)	//	endless until changing user state(vlc-state).
		{
			return 0;
		}
		return 0;
	}
	else	//	file-handle is available.
	{
		if (pLLD->CaclIpBitrateAdj_or_VlcCodecResize_OnPlayCont((void *)pLLD) == 0)	//	vlc codec control.
		{
			return 0;
		}
		if ( pLLD->_FIf->g_CurrentOffsetChanged == 1 )	//	user param.
		{
			pLLD->CurOffsetChanged_by_User_OnPlayCont();
		}
		else if ( pLLD->_FIf->g_SectionRepeated == 2 )	//	end offset, its user param.
		{
			pLLD->SectionRepeatChanged_by_User_OnPlayCont();
		}

//////////////////////////////////////////////////////////////////////////////////////
		pLLD->_FIf->TL_dwBytesRead = 0;		//	rd-byte-size of this turn. the following funcs. may update this val. if success reading data from file or ip.
		if ( pLLD->_SysSta->IsModTyp_DvbT2() && pLLD->_FIf->IsT2miFile() == 0 )
		{
			//	never reach here.
		}
		else if ( pLLD->_SysSta->IsModTyp_IsdbS() )
		{
			//	never reach here.
		}
		else
		{
			pLLD->_FIf->MapTvbBdDestAddr_fromBnkId();
		}

		if ( pLLD->_FIf->TL_gFirstRead )	//	file read 1st time.
		{
			pLLD->FirstFRead_OnPlayCont();	//	read file or remuxer or ip. and set initial cond.
			return 0;
		} 

//////////////////////////////////////////////////////////////////////////////////////
		if ( pLLD->_SysSta->IsModTyp_DvbT2() && pLLD->_FIf->IsT2miFile() == 0 )
		{
		}
		else if ( pLLD->_SysSta->IsModTyp_IsdbS() )
		{
		}
		else
		{
			if ( pLLD->_FIf->TL_nBufferCnt > (DWORD)nBankBlockSize )	//	not happend. i think.
			{
				pLLD->_FIf->TL_dwBytesRead = nBankBlockSize;
				goto _FILL_PLAY_BLOCK_;
			}
		}

//////////////////////////////////////////////////////////////////////////////////////	getting source data from ip or file.
//////////////////////////////////////////////////////////////////////////////////////	and the data is to be stored into TL_szBufferPlay.
		if (pLLD->FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile() == 0)	//	recv ip data. and fill buf TL_szBufferPlay. return 1 if not ip-mode.
		{
			return	0;
		}
		if (pLLD->FillPlayBuf_by_Dta_of_RdFile_OnPlayCont__HasEndlessWhile(&nBankBlockSize) == 0)	//	read file or remuxer and fill buf TL_szBufferPlay
		{
			if ( pLLD->_SysSta->IsModTyp_IsdbS() )
			{
				pLLD->UnlockIsdbSMutex();
			}
			return	0;
		}
		if ( pLLD->_SysSta->IsModTyp_IsdbS() )
		{
			if (pLLD->TL_sz3rdBufferPlay == NULL)
			{
				pLLD->UnlockIsdbSMutex();
				return	0;
			}
		}
		pLLD->UpdPlayBufWIndex_SendIpQ_OnPlayCont(nBankBlockSize);	//	set the indicator of buffer cnt. has valid data.

//////////////////////////////////////////////////////////////////////////////////////
		if ( pLLD->_SysSta->IsModTyp_Tdmb() )
		{
			pLLD->FillTdmb_CifBuf_OnPlayCont();
		}
		else if ( pLLD->_SysSta->IsModTyp_Cmmb() )
		{
			pLLD->__Cmmb->TL_FindStartOfMultiplexFrame();
				//	not reach here. i think???. because PlaybackCmmbFile_OnPlayCont__HasEndlessWhile() always return 0. sure???
		}
		else if ( pLLD->_SysSta->IsModTyp_IsdbT_1() )
		{
			dwRet = pLLD->_FIf->TL_gPacketSize;	//==204, 208
			pLLD->_FIf->FindSyncPos_and_FillSyncBuf(pLLD->_FIf->TL_gPacketSize, &dwRet);
		}
		else if ( pLLD->_SysSta->IsModTyp_IsdbT_13() )
		{
			pLLD->TL_DelayAdjustment();
		}
		else		//	the other case.
		{
			dwRet = 188;
			pLLD->_FIf->FindSyncPos_and_FillSyncBuf(pLLD->_FIf->TL_gPacketSize, &dwRet);	//	fill TL_sz2ndBufferPlay and TL_pbBuffer
		}

//////////////////////////////////////////////////////////////////////////////////////
_FILL_PLAY_BLOCK_:
		if ( pLLD->_FIf->Fill_Playback_DmaBuffer(0, nBankBlockSize) == 0 )
			//	this routine does not depend on modulator type.
			//	1:recording, 0:play... fill the buf TL_pbBuffer by data of (TL_sz2ndBufferPlay + TL_nReadPos)
			//	and write the data to file for recording. or write dma memory TL_pdwDMABuffer for playback.
		{
#ifdef IP_STREAM
			if ( pLLD->_SysSta->IPUsing() && (pLLD->_SysSta->Vlc_Recv_or_RecvRec()) )
			{
				return 0;	//	FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile()
			}
			if ( g_VLCWrapper && (pLLD->_SysSta->Vlc_Recv_or_RecvRec()) )
			{
				return 0;	//	FillPlayBuf_by_Dta_of_RcvIp_OnPlayCont__HasEndlessWhile
			}
#endif

			if (pLLD->_FIf->TL_dwBytesRead < (unsigned int)nBankBlockSize)
			{
				//FIXED - Bitrate adjustment
				if ( pLLD->_FIf->gfRatio > 1 )
				{
					if ( pLLD->_FIf->FEOF(pLLD->_FIf->AP_hFile) && pLLD->_FIf->TL_nBufferCnt < (unsigned int)nBankBlockSize )
					{
						pLLD->_SysSta->SetMainTask_LoopState_(TH_END_PLAY);
					}
				}
				else
				{
					pLLD->_SysSta->SetMainTask_LoopState_(TH_END_PLAY);
#ifdef STANDALONE
	printf("[Debug] call end play =====\n");
#endif
				}
			}
			return 0;
		}

		pLLD->SendIpStreaming_OnPlayCont__HasEndlessWhile();	//	not suppoted now.
		pLLD->_FIf->SetAnchorTime();

//////////////////////////////////////////////////////////////////////////////////////	file playback of isdb-t13 and atsc-mh
		if(!pLLD->_SysSta->IsAttachedTvbTyp_599() && !pLLD->_SysSta->IsAttachedTvbTyp_598())
		{
			if (  pLLD->_SysSta->IsAttachedTvbTyp_SupportIsdbTLoopThru() && pLLD->_SysSta->IsModTyp_IsdbT_13())
			{
				if (pLLD->_FIf->WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
				{
					return 0;
				}
			}
			if ( pLLD->_SysSta->IsModTyp_AtscMH() )
			{
				if (pLLD->_FIf->WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
				{
					return 0;
				}
			}
			if(  pLLD->_SysSta->IsAttachedTvbTyp_SupportDvbT2LoopThru() && pLLD->_SysSta->IsModTyp_DvbT2())
			{
				if (pLLD->_FIf->WaitConsumePlayBuf_toMaxLevel_DvbT2(200) == 0)
				{
					return 0;
				}
			}
		}
		if (pLLD->MultiStreamContPlay() == 1)
		{
			pLLD->_FIf->IncBankAddrHwDest_Sdram(pLLD->_FIf->GetPlayParam_PlayRate());
			return 0;
		}

	//SYSTEMTIME system_time;
	//GetLocalTime(&system_time);
	//static long duration_miliseconds[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//static int index_tmp = 0;
	//static SYSTEMTIME end = system_time;
	//SYSTEMTIME start;
	//long hour, minute, seconds, milisecond, useconds;
	//char str_tmp[128];
	//start = end;
	//end = system_time;

	//hour = (end.wHour - start.wHour)*60*60*1000;
	//minute = (end.wMinute - start.wMinute)*60*1000;
	//seconds = (end.wSecond - start.wSecond)*1000;
	//milisecond = end.wMilliseconds - start.wMilliseconds;


	//duration_miliseconds[index_tmp] =  hour + minute + seconds + milisecond;
	//sprintf(str_tmp, "[HLD] [Consumer] Time %d ========\n", duration_miliseconds[index_tmp]);
	//OutputDebugString(str_tmp);

//////////////////////////////////////////////////////////////////////////////////////
		pLLD->_FIf->StartDmaTransfer_Play_Any();	//	dma start. the data of TL_pdwDMABuffer may transter to HW.
		pLLD->_FIf->IncBankAddrHwDest_Sdram(pLLD->_FIf->GetPlayParam_PlayRate());
	}

	return 0;
}

int CHldPlay::HLD_TH_END_PLAY(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

	if (pLLD->_FIf->IsInvalidFileHandle())
	{
		if (pLLD->_FIf->next_file_ready == TRUE)
		{
			pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_PLAY);
			pLLD->_FIf->FlushBufAndIncLoopnum_OnPlayEnd();
			if ( pLLD->_SysSta->IsModTyp_IsdbS() && pLLD->_FIf->TmccUsing())	//	multi
			{
				pLLD->TL_combiner_ready = 0;
			}
			else
			{
				pLLD->TL_combiner_ready = 0;
				sprintf((char*)pLLD->m_TS_path[0], "%s", pLLD->_FIf->PlayParm.AP_lst.szfn);
			}
		}
		return	0;
	}
	else
	{
		//	file is in open-state.
	}

	pLLD->ChangeStaIq_OnPlayEnd();	//	iq-capture only
	if (pLLD->AdjLoopStartingPos_OnPlayEnd() == 0)
	{
		return	0;
	}
	pLLD->_FIf->CloseFile_of_OpendFile();
	pLLD->WaitAppCntl_NxtFileReady_OnPlayEnd__HasEndlessWhile();	//	???
#if	0
	if (pLLD->_FIf->next_file_ready == TRUE)
	{
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_PLAY);
	}
	if (pLLD->_FIf->Ready_to_PlayRepeatFile_OnPlayEnd() == 0)
	{
		if ( pLLD->_SysSta->IsModTyp_IsdbS() && pLLD->_FIf->TmccUsing())
		{
			pLLD->TL_combiner_ready = 0;
		}
		else
		{
			pLLD->TL_combiner_ready = 0;
			sprintf((char*)pLLD->m_TS_path[0], "%s", pLLD->_FIf->PlayParm.AP_lst.szfn);
		}
		return	0;
	}
#endif

	return 0;
}



