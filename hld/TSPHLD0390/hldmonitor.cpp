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
#include "inc/hldmgr.h"
#else
#include "hldmgr.h"
#endif
#include	"../include/hld_structure.h"

/////////////////////////////////////////////////////////////////
CHldMon::CHldMon(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;

}
CHldMon::~CHldMon()
{
}


int CHldMon::HLD_LauchTask_CaptureAndPlay(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

#ifdef	TSPHLD0390_EXPORTS
	pLLD->TL_InitBufferVariables();	//	non sense. this shoud be located at other position.
	pLLD->InitPlayParameters_IsdbT13();

	pLLD->_SysSta->TL_nIP_Initialized = 0;
	pLLD->_FIf->InitPlayFrameStatusVariables();	//	non sense. this shoud be located at other position.

	pLLD->Launch_LoopThruCapPlayTask();

#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	return 0;
}
//2012/7/10 DVB-T2 ASI
int CHldMon::HLD_LauchTask_Capture(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

#ifdef	TSPHLD0390_EXPORTS
	pLLD->TL_InitBufferVariables();	//	non sense. this shoud be located at other position.

	pLLD->_SysSta->TL_nIP_Initialized = 0;
	pLLD->_FIf->InitPlayFrameStatusVariables();	//	non sense. this shoud be located at other position.

	pLLD->Launch_LoopThruCapTask();
#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	return 0;
}

int CHldMon::HLD_TH_START_MON(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

	if(pLLD->_SysSta->IsCapMod_DvbT2() || pLLD->_SysSta->IsCapMod_DvbC2() || pLLD->_SysSta->IsCapMod_IsdbT13_or_AtscMH() || pLLD->_SysSta->IsCapMod_IsdbS())
		return 0;

	if (pLLD->_SysSta->is_new_playback())
	{
		pLLD->SetFinishAsi(1);
		playback_receive_request(pLLD, PLAY_MESSAGE_STOP, NULL);
		playback_receive_request(pLLD, PLAY_MESSAGE_QUIT, NULL);
	}

#ifdef	TSPHLD0390_EXPORTS
	pLLD->_FIf->CloseFile_of_OpendFile();
	pLLD->_FIf->InitHwAndFileSta_OnMon();
	pLLD->TL_InitConvertNItoNA();		//	tdmb

	pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);	//	transit task state.
	if (pLLD->IsSupriseRemoval())
	{
		return 0;
	}
	if(	pLLD->MultiStreamStartMon() == 0)
		pLLD->_FIf->SetHwOptAndPrepareMonitor_OnMonStart__HasEndlessWhile();

	pLLD->StopAndClose_TmccRemuxer();

	if ( pLLD->_SysSta->IsState_IsdbT13LoopThru() )	//	may return 0, in case of isdbt-13 and atsc-mh loop-thru.
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of isdbt13 at mon-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}
	if ( pLLD->_SysSta->IsState_AtscMhLoopThru() )	//	may return 0, in case of isdbt-13 and atsc-mh loop-thru.
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of atscmh at mon-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}
	//2011/8/4 ISDB-S ASI
	if ( pLLD->_SysSta->IsState_IsdbSLoopThru() )	//	may return 0, in case of isdbs loop-thru.
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of isdbt13 at mon-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}

	//2012/7/6 DVB-T2 ASI
	if(pLLD->_SysSta->IsState_DvbT2LoopThru() || pLLD->_SysSta->IsState_DvbC2LoopThru())
	{
		_CHldMgr.HLD_LauchTask_Capture(nSlot);
	}
#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	pLLD->Free_IQPlayCapBuf_OnMonStart();


	return 0;
}

int CHldMon::HLD_TH_CONT_MON(int nSlot)
{
#ifdef TSPHLD0390_EXPORTS
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	
	return 0;
}



