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
#endif
#include	"../include/hld_structure.h"

/////////////////////////////////////////////////////////////////
CHldCap::CHldCap(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;

}
CHldCap::~CHldCap()
{
}


int CHldCap::HLD_TH_START_REC(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

#ifdef	TSPHLD0390_EXPORTS
	pLLD->InitBertCariable_OnRecStart();
	pLLD->_FIf->OpenFile_OnRecStart();

//////////////////////////////////////////////////////////////////////////////////////
	if ( pLLD->_SysSta->IsState_IsdbT13LoopThru() )
	{
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_REC);
		return 0;
	}
	if ( pLLD->_SysSta->IsState_AtscMhLoopThru() )
	{
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_REC);
		return 0;
	}
	//2011/8/5 ISDB-S ASI
	if ( pLLD->_SysSta->IsState_IsdbSLoopThru() )
	{
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_REC);
		return 0;
	}

	pLLD->_SysSta->SetFlag_CapLoopThru(0);
	pLLD->_SysSta->SetFlag_PlayLoopThru(0);
	pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_REC);
	
//////////////////////////////////////////////////////////////////////////////////////
	pLLD->_FIf->InitVariables_OnRec();
	pLLD->TL_InitConvertNItoNA();	//	tdmb

	pLLD->_FIf->SetHwOptAndPrepareCapture_OnRecStart__HasEndlessWhile();
	
	if ( pLLD->_SysSta->IsState_IsdbT13LoopThru() )		//	not need. because we checked already above. --> ??? depend on the timing of app-control
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of isdbt13 at rec-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}
	if ( pLLD->_SysSta->IsModTyp_IsdbT_13() )
	{
		pLLD->_FIf->SetHwDmaDiection_LoopThru();
	}

	if ( pLLD->_SysSta->IsState_AtscMhLoopThru() )	//	not need. because we checked already above. --> ??? depend on the timing of app-control
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of atscmh at rec-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}
	if ( pLLD->_SysSta->IsModTyp_AtscMH() )
	{
		pLLD->_FIf->SetHwDmaDiection_LoopThru();
	}
	//2011/8/5 ISDB-S ASI
	if ( pLLD->_SysSta->IsState_IsdbSLoopThru() )	//	not need. because we checked already above. --> ??? depend on the timing of app-control
	{
		pLLD->_HLog->HldPrint("Hld-Mgr. Evt to start cap-plat-task of isdbs at rec-start-state");
		_CHldMgr.HLD_LauchTask_CaptureAndPlay(nSlot);
	}
	if ( pLLD->_SysSta->IsModTyp_IsdbS() )
	{
		pLLD->_FIf->SetHwDmaDiection_LoopThru();
	}

	pLLD->RunVlc_OnRecStart(pLLD);

#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	pLLD->Prepare_IQPlayCapBuf_OnRecStart();

	pLLD->MultiStreamStartRec();

	return 0;
}

int CHldCap::HLD_TH_CONT_REC(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}
	int		i, nRet;
#ifdef	TSPHLD0390_EXPORTS
	if (pLLD->Capture_IQData_UntilStopCond__HasEndlessWhile() == 0)
	{
		return 0;
	}

	if ( pLLD->_SysSta->IsState_IsdbT13LoopThru() )
	{
		;
	}
	else
	if ( pLLD->_SysSta->IsState_AtscMhLoopThru() )
	{
		;
	}
	else if ( pLLD->_SysSta->IsState_IsdbSLoopThru() )
	{
		;
	}
	else
	{
		if (pLLD->MultiStreamContRec() == 1)
		{
			return 0;
		}

		for (i = 0; (i < pLLD->_SysSta->iHW_BANK_NUMBER+1) && (pLLD->_SysSta->IsTaskState_ContRec()) && !pLLD->_SysSta->ReqedNewAction_User(); i++)
		{
			pLLD->_FIf->StartDmaTransfer_Capture(0);		//	cap start one bank.
			if (pLLD->_FIf->__Buf_DmaPtr_Transfer_Bidirectional() != NULL)
			{
				nRet = pLLD->TL_ProcessCaptureSubBlock();
				pLLD->SendIpStreaming_OnRecCont__HasEndlessWhile(nRet);
			}
			else
			{
				pLLD->_HLog->HldPrint("Hld-Mgr. FAIL to read BLOCK in REC");
			}
		}
	}

#elif	TSPHLD0104_EXPORTS
#elif	TSPHLD0110_EXPORTS
#endif

	return 0;
}



