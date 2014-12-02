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
#include	"../../include/logfile.h"
#include	"../LLDWrapper.h"
#include	"../VLCWrapper.h"

#if defined(WIN32)
#include	"inc/hldmgr.h"
#else
#include	"hldmgr.h"
#include	"../../include/lld_const.h"
#include	"../../include/lld_api.h"
#include	"../../include/hld_type.h"
#endif

#include	"../../include/system_const.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
static	CBdSysConf	*_CBdSysConf;

/////////////////////////////////////////////////////////////////
int	g_MultiBoard = 1;
int	TL_gLastError = 0;

/////////////////////////////////////////////////////////////////
CHldMgr	_CHldMgr;
/////////////////////////////////////////////////////////////////
CHldMgr::CHldMgr(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;
}
CHldMgr::~CHldMgr()
{
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldMgr::HLD_MainTaskStateLoop_Play(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
#ifdef WIN32
#else
	int nRet;
#endif
	if ( !pLLD )
	{
		return 1;
	}

	switch(pLLD->_SysSta->MainTask_LoopState())
	{
	case	TH_START_PLAY:
		return HLD_TH_START_PLAY(nSlot, 0);
	case	TH_CONT_PLAY:
#if defined(WIN32)
		return HLD_TH_CONT_PLAY(nSlot);
#else
		//pthread_mutex_lock(&pLLD->_FIf->TL_hMutex);
		nRet = HLD_TH_CONT_PLAY(nSlot);
		//pthread_mutex_unlock(&pLLD->_FIf->TL_hMutex);
		return nRet; 
#endif
	case	TH_END_PLAY:
		return HLD_TH_END_PLAY(nSlot);

	case	TH_START_DELAY:	//	Delaying : no used
		pLLD->_FIf->TSPL_RESET_SDCON();
		pLLD->_FIf->TSPL_SET_SDCON_MODE(TSPL_SDCON_DELAY_MODE);
		pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_DELAY);
		break;
	case	TH_CONT_DELAY:	//	Delaying : no used
		Sleep(100);
		break;
	default:
		break;
	}
	return 0;
}
int	CHldMgr::HLD_MainTaskStateLoop_Rec(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
#ifdef WIN32
#else
	int nRet;
#endif
	if ( !pLLD )
	{
		return 1;
	}

	switch(pLLD->_SysSta->MainTask_LoopState())
	{
	case	TH_START_REC:
		return HLD_TH_START_REC(nSlot);
	case	TH_CONT_REC:
#if defined(WIN32)
		return HLD_TH_CONT_REC(nSlot);
#else
		pthread_mutex_lock(&pLLD->_FIf->TL_hMutex);
		nRet = HLD_TH_CONT_REC(nSlot);
		pthread_mutex_unlock(&pLLD->_FIf->TL_hMutex);
		return nRet; 
#endif
	case	TH_STOP_REC:
		pLLD->_SysSta->SetMainTask_LoopState_(TH_START_MON);
		return 0;
	}
	return 0;
}
int	CHldMgr::HLD_MainTaskStateLoop_Monitor(int nSlot)
{
	CHld *pLLD = g_LLDWrapper[nSlot];
	if ( !pLLD )
	{
		return 1;
	}

	switch(pLLD->_SysSta->MainTask_LoopState())
	{
	case	TH_START_MON:
		if (  pLLD->_SysSta->IsState_IsdbT13LoopThru() )
		{
			if ( pLLD->_SysSta->Flag_PlayLoopThru() || pLLD->_SysSta->Flag_CapLoopThru() )
			{
				pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);
				return 0;
			}
		}
		if ( pLLD->_SysSta->IsModTyp_AtscMH() )
		{
			if ( pLLD->_SysSta->Flag_PlayLoopThru() || pLLD->_SysSta->Flag_CapLoopThru() )
			{
				pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);
				return 0;
			}
		}
		if ( pLLD->_SysSta->IsModTyp_DvbT2() )
		{
			//2012/7/11 DVB-T2 ASI
			if( pLLD->_SysSta->IsState_DvbT2LoopThru() )
			{
				if ( pLLD->_SysSta->Flag_CapLoopThru() )
				{
					pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);
					return 0;
				}
			}
			else
			{
				while ( pLLD->TL_T2MI_PumpingThreadDone == 0 || pLLD->TL_T2MI_ConvertingThreadDone == 0 )
				{
					Sleep(10);
				}
			}
		}
		if ( pLLD->_SysSta->IsModTyp_IsdbS() )
		{
			//2011/8/5 ISDB-S ASI
			if (  pLLD->_SysSta->IsState_IsdbSLoopThru() )
			{
				if ( pLLD->_SysSta->Flag_PlayLoopThru() || pLLD->_SysSta->Flag_CapLoopThru() )
				{
					pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);
					return 0;
				}
			}
			else
			{
				while ( pLLD->TL_ISDBS_PumpingThreadDone == 0  )
				{
					Sleep(10);
				}
			}
		}
		//2011/7/21 added
		if ( pLLD->_SysSta->IsModTyp_DvbC2() )
		{
			//2012/7/11 DVB-C2 ASI
			if( pLLD->_SysSta->IsState_DvbC2LoopThru() )
			{
				if ( pLLD->_SysSta->Flag_CapLoopThru() )
				{
					pLLD->_SysSta->SetMainTask_LoopState_(TH_CONT_MON);
					return 0;
				}
			}
			else
			{
				while ( pLLD->TL_C2MI_PumpingThreadDone == 0 || pLLD->TL_C2MI_ConvertingThreadDone == 0 )
				{
					Sleep(10);
				}
			}
		}
		return HLD_TH_START_MON(nSlot);
	case	TH_CONT_MON:
		return HLD_TH_CONT_MON(nSlot);
	}
	return 0;
}
int	CHldMgr::HLD_StateMainLoop(int nSlot)
{
	DWORD   dwSizeWritten;
	CHld *pLLD = g_LLDWrapper[nSlot];

	if ( !pLLD )
	{
		return 1;
	}

	if (pLLD->_SysSta->HappenedCond_StopRecording())
	{
		if ( (pLLD->_SysSta->IsTaskState_ContRec() && pLLD->TL_nRecordNA == 0) || (pLLD->_SysSta->IsTaskState_ContMon()) )
		{
			dwSizeWritten = pLLD->_FIf->Fill_Playback_DmaBuffer(1, (pLLD->_FIf->SdramSubBankSize()));
			pLLD->_FIf->CloseFile_of_OpendFile();
		}
		else	//	not happend this case.
		{
			pLLD->_FIf->TL_CloseRecording();
		}
	}
	else if (pLLD->_SysSta->HappenedCond_StopPlay())
	{
//		pLLD->_HLog->HldPrint("Hld-Mgr. TL_StopPlaying");
		pLLD->_FIf->TL_StopPlaying();
//		pLLD->_HLog->HldPrint("Hld-Mgr. TL_StopPlaying---");
	}

//////////////////////////////////////////////////////////////////////////////////////
	if (pLLD->_SysSta->ReqedKillTask_User())
	{
		return	1;	//	requested termination.
	}
	if (pLLD->_SysSta->DetermineMainTaskState_from_UserReq() == 0)
	{
		return	0;	//	nothing to do
	}

//////////////////////////////////////////////////////////////////////////////////////
	switch(pLLD->_SysSta->MainTask_LoopState())
	{
	case	TH_START_MON:
	case	TH_CONT_MON:
		return	HLD_MainTaskStateLoop_Monitor(nSlot);
	case	TH_START_REC:
	case	TH_CONT_REC:
	case	TH_STOP_REC:
		return	HLD_MainTaskStateLoop_Rec(nSlot);
	case	TH_START_PLAY:
	case	TH_CONT_PLAY:
	case	TH_END_PLAY:
	case	TH_START_DELAY:
	case	TH_CONT_DELAY:
		return	HLD_MainTaskStateLoop_Play(nSlot);
	}
	return 0;
}

//----------------------------------------------------------------------------------
#if defined(WIN32)
void	CHldMgr::HLDThreadEngine(PVOID param)
#else
void	*CHldMgr::HLDThreadEngine(PVOID param)
#endif
{
	long nSlot = (long)param;
	CHld *pLLD = g_LLDWrapper[nSlot];

	if ( !pLLD )
	{
#if defined(WIN32)	
		_endthread();
		return;
#else
		pthread_exit(NULL);
		return 0;
#endif
	}

	if ( pLLD->_SysSta->IsAttachedTvbTyp_Usb() )
	{
#if defined(WIN32)
		pLLD->m_hDeviceEvent[0] = CreateEvent(NULL, FALSE, FALSE, szEvt0);
		pLLD->m_hDeviceEvent[1] = CreateEvent(NULL, FALSE, FALSE, szEvt1);
		if ( pLLD->m_hDeviceEvent[0] != INVALID_HANDLE_VALUE )
			pLLD->_FIf->TSPL_REG_EVENT((void*)&pLLD->m_hDeviceEvent[0]);
		if ( pLLD->m_hDeviceEvent[1] != INVALID_HANDLE_VALUE )
			pLLD->_FIf->TSPL_REG_COMPLETE_EVENT((void*)&pLLD->m_hDeviceEvent[1]);

		pLLD->m_hEventThread = ::CreateThread(NULL, 
			0, 
			pLLD->DeviceEventThreadProc, 
			(LPVOID)pLLD, 
			0, 
			&pLLD->m_EventThreadId);
#endif		
		pLLD->TL_nSupriseRemoval = 0;
	}
	pLLD->_SysSta->TL_fThreadActive = TRUE;

	_CBdSysConf->Cnxt(nSlot)->__hld_mgr_activated__ = 1;

#ifdef	TSPHLD0110_EXPORTS
#else
#if defined(WIN32)
	FILE* fp=NULL;
#else
	FILE* fp=(FILE*)NULL;
#endif
	fp = fopen(".\\restamp_data_logged", "r");
	if ( fp != NULL )
	{
		fclose(fp);
		pLLD->_FIf->TL_gRestampLog = 1;
	}
	else
	{
		pLLD->_FIf->TL_gRestampLog = 0;
	}
#endif

	pLLD->_HLog->HldPrint("Hld-Mgr. Launch MainTask");

//////////////////////////////////////////////////////////////////////////////////////
	while (!_CHldMgr.HLD_StateMainLoop(nSlot)) 
	{
//		pLLD->_HLog->HldPrint_Tmr(9, "hld-mgr", pLLD->_SysSta->MainTask_LoopState());
		Sleep(10);
	}
	pLLD->_HLog->HldPrint("Hld-Mgr. Term MainTask");

//////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
	if ( pLLD->gIQBuffer )
	{
        VirtualFree(pLLD->gIQBuffer, 0, MEM_RELEASE);
        pLLD->gIQBuffer = NULL;
	}
#else
	if ( pLLD->gIQBuffer )
	{
		free(pLLD->gIQBuffer);
		pLLD->gIQBuffer = NULL;
	}
#endif

	pLLD->_SysSta->SetFlag_DtaPathDirection(TSIO_PLAY_WITH_310INPUT);
	while ( pLLD->_SysSta->Flag_PlayLoopThru() || pLLD->_SysSta->Flag_CapLoopThru() )
	{
		Sleep(100);
	}

	//"RF output disabled", not called at application(Close_System)
	pLLD->_FIf->TSPL_SET_TSIO_DIRECTION(TSIO_PLAY_WITH_310INPUT/*FILE SOURCE*/);
	pLLD->_FIf->TSPL_SET_SDCON_MODE(TSPL_SDCON_LOOP_THRU_MODE);
	
//	pLLD->_SysSta->TL_fThreadActive = FALSE;
	pLLD->_SysSta->ResetAllReqedState_User();
	pLLD->_HLog->HldPrint("Hld-Mgr. Terminate thread engine");

	pLLD->_FIf->TVB380_CLOSE();
	_CBdSysConf->Cnxt(nSlot)->__hld_mgr_activated__ = 0;

#if defined(WIN32)	
	_endthread();
	return;
#else
	pthread_exit(NULL);
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////
CBdSysConf::CBdSysConf(void)
{
#if defined(WIN32)
	FILE* fp=NULL;
#else
	FILE* fp=(FILE*)NULL;
#endif
	g_MultiBoard = 1;
	fp = fopen(".\\no_multi_board", "rt");
	if ( fp != NULL )
	{
		g_MultiBoard = 0;
		fclose(fp);
	}
	
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;

	time_t tCur;
	struct tm newtime;
	tCur = time(NULL);
#ifdef WIN32
	localtime_s(&newtime, &tCur);
#else
	localtime_r(&tCur, &newtime);
#endif
	sprintf(szDateInfo, "%04d%02d%02d%02d%02d%02d", (newtime.tm_year + 1900),
		(newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

	memset(&_bd_cnxt[0], 0, sizeof(_BD_CONF_CNXT)*_MAX_INST_CNT_);
	for (int i = 0; i < _MAX_INST_CNT_; i++)
	{
		_bd_cnxt[i].__id__ = -1;
		_bd_cnxt[i].__id_my_phy__ = -1;
	}
	cnt_phy_bd = 0;
	cnt_phy_and_vir_bd = 0;

	DetectInstalledTvbBd(g_MultiBoard);
}
CBdSysConf::~CBdSysConf()
{
}
_BD_CONF_CNXT	*CBdSysConf::Cnxt(int _rd_id)
{
	return	&_bd_cnxt[_rd_id];
}
int	CBdSysConf::GetRealBdCnt_N(void)
{
	return	cnt_phy_bd;
}
int	CBdSysConf::GetBdVirCnt_N(void)
{
	return	(cnt_phy_and_vir_bd - cnt_phy_bd);
}
int	CBdSysConf::GetBdCnt_N(void)	//	sum of real and vir
{
	return	cnt_phy_and_vir_bd;
}
int	CBdSysConf::GetRealAndVirBdMap(int _r_bd_id, int *_map)
{
	int	ind, cnt;

	for (ind = __SLOT_ALLOC_FROM__, cnt = 0; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
{
		if (_bd_cnxt[ind].__id_my_phy__ == _r_bd_id)
		{
			*_map++ = ind;
			cnt++;
		}
	}
	return	cnt;
}
int	CBdSysConf::GetBdType_N(int _Nth)	//	___ID_BDs___
{
	return	_bd_cnxt[_Nth]._bd_typ_id;
}
char	*CBdSysConf::GetBdName_N(int _Nth)
{
	return	_bd_cnxt[_Nth]._location_name_string;
}

CHld	*CBdSysConf::CreateOneCHld(int _id)
{
	char	dll_lld[100];
	CHld	*_hld;

#ifdef WIN32
	sprintf(dll_lld, "TSPLLD038%d.DLL", _id);
#else
	sprintf(dll_lld, "/usr/lib/libtsplld038%d.so.1.0.0", _id);
#endif
	_hld = new	CHld(_id);
	if (_hld->_FIf->InitialDLL(dll_lld) == FALSE)
	{
		_hld->_FIf->ReleaseDLL();
		delete	_hld;
		return	NULL;
	}

	return	_hld;
}
void	CBdSysConf::DeleteOneCHld(CHld *_hld)
	{
		_hld->_FIf->ReleaseDLL();
		delete	_hld;
	}
int	CBdSysConf::AsignSlotNofRealTvbBd(CHld *_hld)
{
	int	ret, ind, real_seq;

	ret = _hld->_FIf->TVBxxx_GET_CNXT_ALL_BD((void *)&_bd_cnxt[0]);
	for (ind = __SLOT_ALLOC_FROM__, real_seq = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		if (_bd_cnxt[ind].__iam_real__)
		{
			_bd_cnxt[ind].__id__ = real_seq;
			_bd_cnxt[ind].__id_my_phy__ = real_seq;
			_bd_cnxt[ind].__id_ts_seq__ = 0;
			real_seq++;
			cnt_phy_bd++;
			cnt_phy_and_vir_bd++;
		}
	}
	return	real_seq;
}
int	CBdSysConf::AsignSlotNofVirTvbBd(int _r_bd_id_to_copy_from, int _ts_seq_)
{
	int	ind, new_id;
	int ts_seq__;
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		if (_bd_cnxt[ind].__id__ == -1)
			break;
		if(_bd_cnxt[ind].__id_my_phy__ == _r_bd_id_to_copy_from)
		{
			ts_seq__ = _bd_cnxt[ind].__id_ts_seq__;
		}
	}
	new_id = ind;

	memcpy(&_bd_cnxt[new_id], &_bd_cnxt[_r_bd_id_to_copy_from], sizeof(_BD_CONF_CNXT));
	_bd_cnxt[new_id].__id__ = new_id;
	_bd_cnxt[new_id].__iam_real__ = 0;
	_bd_cnxt[new_id].__id_my_phy__ = _r_bd_id_to_copy_from;
	_bd_cnxt[new_id].__id_ts_seq__ = ts_seq__ + 1;

	return	new_id;
}
int	CBdSysConf::DetectInstalledTvbBd(int _multi_bd)
{
	CHld	*_hld;
	int	ret;

	_hld = CreateOneCHld(0);	//	0 is dummy(not use)
	if (_hld == NULL)
	{
		return -1;
	}
	ret = _hld->_FIf->TVBxxx_DETECT_BD(_multi_bd);
	if (ret > __SLOT_ALLOC_FROM__)
	{
		AsignSlotNofRealTvbBd(_hld);
	}
	DeleteOneCHld(_hld);

	return	ret;
}
int	CBdSysConf::InitOneRealBd(int _bd_id, CHld *_hld)
{
	int	ret;

#ifdef WIN32
	strcpy_s(_bd_cnxt[_bd_id].szLog_SubFolder, 16, szDateInfo);
#else
	strcpy(_bd_cnxt[_bd_id].szLog_SubFolder, szDateInfo);
#endif
	ret = _hld->_FIf->TVBxxx_INIT_BD(_bd_id, &_bd_cnxt[_bd_id]);

	_hld->_HLog->OpenF_HldPrint(_bd_id, szDateInfo);

	_hld->_FIf->TVBxxx_GET_CNXT_MINE(&_bd_cnxt[_bd_id]);

	//2012/3/22
	_hld->_SysSta->m_nBoardId = _bd_cnxt[_bd_id]._bd_typ_id;
	return	ret;
}
int	CBdSysConf::InitOneVirBd(int _r_bd_id, int _v_bd_id, CHld *_hld)
{
	CHld	*r_hld;
	int nRet;
	r_hld = g_LLDWrapper[_r_bd_id];
	
	_hld->DupMultiTsMutexForVirtual(r_hld->MultiTsMutex());
#if defined(WIN32)
	if (_beginthread(_CHldMgr.HLDThreadEngine, 0, (PVOID)_hld->my_hld_id) == -1)
	{
		_hld->_HLog->HldPrint("Hld-Mgr. FAIL to create thread");
	}
#else	
	pthread_create(&_CHldMgr.a_thread[_hld->my_hld_id], NULL, _CHldMgr.HLDThreadEngine, (PVOID)_hld->my_hld_id);
#endif
	//return	_hld->_FIf->TVBxxx_DUP_BD(_r_bd_id, _v_bd_id, Cnxt(_r_bd_id));
	nRet = _hld->_FIf->TVBxxx_DUP_BD(_r_bd_id, _v_bd_id, Cnxt(_v_bd_id));
	_hld->_HLog->OpenF_HldPrint(_v_bd_id, szDateInfo);
	return	nRet;
}
int	CBdSysConf::ActivateOneRealBd(CHld *_hld, int _init_modulator, int _init_if_freq)
{
//////////////////////////////////////////////////////////////
	if ( _hld->_FIf->TVB380_OPEN(	_init_modulator, _init_if_freq) != 0 )
	{
		TL_gLastError = _hld->_FIf->TSPL_GET_LAST_ERROR();
		_hld->_HLog->HldPrint("Hld-Mgr. FAIL to configure board!!!");
//		DeleteOneCHld(_hld);
		return -1;
	}

	_hld->_SysSta->TL_gCurrentModType = _init_modulator;

	_hld->TL_gFPGA_ID = _hld->_FIf->TSPL_GET_FPGA_INFO(0);
	_hld->TL_gFPGA_VER = _hld->_FIf->TSPL_GET_FPGA_INFO(1);
	_hld->TL_gFPGA_IQ_Play = _hld->_FIf->TSPL_GET_FPGA_INFO(2);
	_hld->TL_gFPGA_IQ_Capture = _hld->_FIf->TSPL_GET_FPGA_INFO(3);

	_hld->_HLog->HldPrint_2("Hld-Mgr. FPGA::ID, VERSION", (int)_hld->TL_gFPGA_ID, (int)_hld->TL_gFPGA_VER);

	//6.9.16 - fixed, DVB/EIT
	_hld->_FIf->m_nLoopAdaptaionOption = 0;

//what is it?	CALock	cs(_hld);
	_hld->_HLog->HldPrint("Hld-Mgr. SUCCESS to create device handler");
	TL_gLastError = _hld->_FIf->TSPL_GET_LAST_ERROR();

	_hld->CreateMultiTsMutexForReal();
#if defined(WIN32)
	if (_beginthread(_CHldMgr.HLDThreadEngine, 0, (PVOID)_hld->my_hld_id) == -1)
	{
		_hld->_HLog->HldPrint("Hld-Mgr. FAIL to create thread");
	}
#else	
	pthread_create(&_CHldMgr.a_thread[_hld->my_hld_id], NULL, _CHldMgr.HLDThreadEngine, (PVOID)_hld->my_hld_id);
#endif

	return 0;
}

extern "C" _declspec(dllexport)	int _stdcall TSPH_GetRealBdCnt_N(void)
{
	return	_CBdSysConf->GetRealBdCnt_N();
		}
extern "C" _declspec(dllexport)	int _stdcall TSPH_GetRealAndVirBdMap(int _r_bd_id, int *_map)
	//	return val is the cnt of active slot#, and the B'd #-seq may filled in *map.
	//	ex. *_map have such conf. [2, 5, 6, 7] for tvb593 multi-qam service. the 2 is the given # _r_bd_id.
{
	return	_CBdSysConf->GetRealAndVirBdMap(_r_bd_id, _map);
	}
extern "C" _declspec(dllexport)	int _stdcall TSPH_GetBdId_N(int _Nth)	//	___ID_BDs___
{
	return	_CBdSysConf->GetBdType_N(_Nth);
}
extern "C" _declspec(dllexport)	char * _stdcall TSPH_GetBdName_N(int _Nth)	//	the string of location name.
{
	return	_CBdSysConf->GetBdName_N(_Nth);
}

extern "C" _declspec(dllexport)	int _stdcall TSPH_ConfTvbSytem(void)	//	return cnt of installed real bd.
	{
#ifdef WIN32
	if(GetPriorityClass(GetCurrentProcess()) == REALTIME_PRIORITY_CLASS || GetPriorityClass(GetCurrentProcess()) == HIGH_PRIORITY_CLASS)
	{
		;
	}
	else
	{
		if(!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
		{
			return 0;
			//_hld->_HLog->HldPrint_1("Fail to set process priority, error code ", (int)GetLastError());
		}
	}
#endif
	_CBdSysConf = new CBdSysConf();
	return	_CBdSysConf->GetRealBdCnt_N();
	}
extern "C" _declspec(dllexport)	int _stdcall TSPH_InitOneRealBd(int _bd_id)
{
	g_LLDWrapper[_bd_id] = _CBdSysConf->CreateOneCHld(_bd_id);
	if (g_LLDWrapper[_bd_id] == NULL)	return	-1;

	g_LLDWrapper[_bd_id]->_SysSta->m_nBoardLocation = _bd_id;
	g_LLDWrapper[_bd_id]->_SysSta->my_stream_id_of_virtual_bd = 0;
	_CBdSysConf->InitOneRealBd(_bd_id, g_LLDWrapper[_bd_id]);
	return	_CBdSysConf->GetBdType_N(_bd_id);
}
extern "C" _declspec(dllexport)	int _stdcall TSPH_InitAllRealBd(void)
{
	int	ind, cnt;

	printf("[dbg-seq] - 0002\n");
	for (ind = __SLOT_ALLOC_FROM__, cnt = 0; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		if ((_CBdSysConf->Cnxt(ind)->__id__ != -1) && (_CBdSysConf->Cnxt(ind)->__iam_real__ == 1))
		{
			TSPH_InitOneRealBd(ind);
			cnt++;
		}
	}
	return	cnt;
}
extern "C" _declspec(dllexport)	int _stdcall TSPH_InitVirBd(int _r_bd_id, int _v_cnt)
{
	int	ind, new_id;
	CHld	*_my_real = NULL, *_1st_vir = NULL, *_2nd_vir = NULL, *_3rd_vir = NULL;

	_my_real = g_LLDWrapper[_r_bd_id];
	for (ind = 0; ind < _v_cnt; ind++)
	{
		new_id = _CBdSysConf->AsignSlotNofVirTvbBd(_r_bd_id, ind);
		g_LLDWrapper[new_id] = _CBdSysConf->CreateOneCHld(new_id);
		if (g_LLDWrapper[new_id] == NULL)	return	-1;

		g_LLDWrapper[new_id]->_SysSta->m_nBoardLocation = new_id;
		g_LLDWrapper[new_id]->_SysSta->my_stream_id_of_virtual_bd = _CBdSysConf->Cnxt(new_id)->__id_ts_seq__;
		g_LLDWrapper[new_id]->_SysSta->cnt_my_sub_ts_vsb = _CBdSysConf->Cnxt(new_id)->__cnt_my_sub_ts_vsb__;
		g_LLDWrapper[new_id]->_SysSta->cnt_my_sub_ts_qam = _CBdSysConf->Cnxt(new_id)->__cnt_my_sub_ts_qam__;
		//2012/6/28 multi dvb-t
		g_LLDWrapper[new_id]->_SysSta->cnt_my_sub_ts_dvbt = _CBdSysConf->Cnxt(new_id)->__cnt_my_sub_ts_dvbt__;
		g_LLDWrapper[new_id]->_SysSta->m_nBoardId = _CBdSysConf->Cnxt(new_id)->_bd_typ_id;
		_CBdSysConf->InitOneVirBd(_r_bd_id, new_id, g_LLDWrapper[new_id]);
#if 0
		switch(g_LLDWrapper[new_id]->_SysSta->my_stream_id_of_virtual_bd)
		{
		case	1:		_1st_vir = g_LLDWrapper[new_id];	break;
		case	2:		_2nd_vir = g_LLDWrapper[new_id];	break;
		case	3:		_3rd_vir = g_LLDWrapper[new_id];	break;
		}
#endif
	}

	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		if (_CBdSysConf->Cnxt(ind)->__id__ == -1)
			break;
		if(_CBdSysConf->Cnxt(ind)->__id_my_phy__ == _r_bd_id)
		{
			if(_CBdSysConf->Cnxt(ind)->__id_ts_seq__ == 1)
				_1st_vir = g_LLDWrapper[ind];
			else if(_CBdSysConf->Cnxt(ind)->__id_ts_seq__ == 2)
				_2nd_vir = g_LLDWrapper[ind];
			else if(_CBdSysConf->Cnxt(ind)->__id_ts_seq__ == 3)
				_3rd_vir = g_LLDWrapper[ind];
		}
	}
	

	if (_my_real != NULL)	_my_real->_SysSta->SetMyChildHld((void *)_my_real, (void *)_1st_vir, (void *)_2nd_vir, (void *)_3rd_vir);
	if (_1st_vir != NULL)	_1st_vir->_SysSta->SetMyChildHld((void *)_my_real, (void *)_1st_vir, (void *)_2nd_vir, (void *)_3rd_vir);
	if (_2nd_vir != NULL)	_2nd_vir->_SysSta->SetMyChildHld((void *)_my_real, (void *)_1st_vir, (void *)_2nd_vir, (void *)_3rd_vir);
	if (_3rd_vir != NULL)	_3rd_vir->_SysSta->SetMyChildHld((void *)_my_real, (void *)_1st_vir, (void *)_2nd_vir, (void *)_3rd_vir);

	return	_CBdSysConf->GetBdType_N(_r_bd_id);
}
extern "C" _declspec(dllexport)	int _stdcall TSPH_ActivateOneBd(int _bd_id, int _init_modulator, int _init_if_freq)
{
	int	ret;

	//2012/3/28 
	int nVirBd_VsbCnt;
	int nVirBd_QambCnt;
	//2012/6/28 multi dvb-t
	int nVirBd_DvbTCnt;

	int nVirBd_Cnt;

	if (_CBdSysConf->Cnxt(_bd_id)->__id__ == -1)	return	-1;
	if (g_LLDWrapper[_bd_id] == NULL)	return	-1;

	ret = _CBdSysConf->ActivateOneRealBd(g_LLDWrapper[_bd_id], _init_modulator, _init_if_freq);
	if (ret == -1)
	{
		_CBdSysConf->DeleteOneCHld(g_LLDWrapper[_bd_id]);
		g_LLDWrapper[_bd_id] = NULL;
		return -1;
	}

	switch(_CBdSysConf->Cnxt(_bd_id)->_bd_typ_id)
	{
	case	_BD_ID_594__:
		//2012/3/28
		nVirBd_VsbCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_VSB_RFOUT(); 
		if(nVirBd_VsbCnt > 1)
			TSPH_InitVirBd(_bd_id, (nVirBd_VsbCnt - 1));
		break;
	case	_BD_ID_593__:
	case	_BD_ID_598__:
		//2012/3/28
		nVirBd_VsbCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_VSB_RFOUT(); 
		nVirBd_QambCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_QAM_RFOUT();
		//2012/6/28 multi dvb-t
		nVirBd_DvbTCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_DVBT_RFOUT();
		if(nVirBd_VsbCnt < nVirBd_QambCnt)
		{
			nVirBd_Cnt = nVirBd_QambCnt;
		}
		else
		{
			nVirBd_Cnt = nVirBd_VsbCnt;
		}
		//2012/6/28 multi dvb-t
		if(nVirBd_DvbTCnt > nVirBd_Cnt)
		{
			nVirBd_Cnt = nVirBd_DvbTCnt;
		}
		if(nVirBd_Cnt > 1)
			TSPH_InitVirBd(_bd_id, (nVirBd_Cnt - 1));
		break;
	case	_BD_ID_591S__:
		//2012/3/28
		nVirBd_VsbCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_VSB_RFOUT(); 
		nVirBd_QambCnt = g_LLDWrapper[_bd_id]->_FIf->TSPL_CNT_MULTI_QAM_RFOUT();
		if(nVirBd_VsbCnt < nVirBd_QambCnt)
		{
			nVirBd_Cnt = nVirBd_QambCnt;
		}
		else
		{
			nVirBd_Cnt = nVirBd_VsbCnt;
		}
		if(nVirBd_Cnt > 1)
			TSPH_InitVirBd(_bd_id, (nVirBd_Cnt - 1));
		break;
	}
	return	ret;
}
extern "C" _declspec(dllexport)	int _stdcall TSPH_ActivateAllRealBd(void)	//	test purpose
{
	int	ind, cnt;

	for (ind = __SLOT_ALLOC_FROM__, cnt = 0; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		if ((_CBdSysConf->Cnxt(ind)->__id__ != -1) && (_CBdSysConf->Cnxt(ind)->__iam_real__ == 1))
		{
			TSPH_ActivateOneBd(ind, 1, 44000000);
			cnt++;
		}
	}
	return	cnt;
}
extern "C" _declspec(dllexport)	int _stdcall TSPH__STREAM_NUMBER(int nSlot)
{
	if (g_LLDWrapper[nSlot] == NULL)	return	-1;
	return	_CBdSysConf->Cnxt(nSlot)->__id_ts_seq__;
}
extern "C" _declspec(dllexport)	int _stdcall TSPH__SEL_TS_of_ASI310OUT(int nSlot, int _ts_n)
{
	if (g_LLDWrapper[nSlot] == NULL)	return	-1;
	g_LLDWrapper[nSlot]->MultiStreamSelTs_ofAsi310Out(_ts_n);
	return	0;
}

//2012/3/28
extern "C" _declspec(dllexport)	int _stdcall TSPL_CNT_MULTI_VSB_RFOUT_EX(int nSlot)
{
	if (g_LLDWrapper[nSlot] == NULL)	return	-1;
	return g_LLDWrapper[nSlot]->_FIf->TSPL_CNT_MULTI_VSB_RFOUT();
}
extern "C" _declspec(dllexport)	int _stdcall TSPL_CNT_MULTI_QAM_RFOUT_EX(int nSlot)
{
	if (g_LLDWrapper[nSlot] == NULL)	return	-1;
	return g_LLDWrapper[nSlot]->_FIf->TSPL_CNT_MULTI_QAM_RFOUT();
}
//2012/6/28 multi dvb-t
extern "C" _declspec(dllexport)	int _stdcall TSPL_CNT_MULTI_DVBT_RFOUT_EX(int nSlot)
{
	if (g_LLDWrapper[nSlot] == NULL)	return	-1;
	return g_LLDWrapper[nSlot]->_FIf->TSPL_CNT_MULTI_DVBT_RFOUT();
}


//----------------------------------------------------------------------------------
extern "C" _declspec(dllexport)	void _stdcall ProcessDetach(void)
{
	int cnt = 0;
	for (int nSlot = 0; nSlot <= _MAX_INST_CNT_; nSlot++)
	{
		cnt = 0;
		if (g_LLDWrapper[nSlot] && _CBdSysConf->Cnxt(nSlot)->__hld_mgr_activated__)
		{
			g_LLDWrapper[nSlot]->close_ip_streaming_srv();
			g_LLDWrapper[nSlot]->_SysSta->SetReqedStartMonitor_User(TH_START_MON);
			g_LLDWrapper[nSlot]->_SysSta->SetReqedKillTask_User(TRUE);
			g_LLDWrapper[nSlot]->_SysSta->TL_fThreadActive = FALSE;
			while(_CBdSysConf->Cnxt(nSlot)->__hld_mgr_activated__)
			{
				if(cnt++ > 10)
				{
					break;
				}
				Sleep(100);
			}
			_CBdSysConf->DeleteOneCHld(g_LLDWrapper[nSlot]);
		}
	}
	Sleep(10);
}

extern "C" _declspec(dllexport)	int _stdcall TSPH_CloseOneRealBd(int _bd_id)
{
#ifdef WIN32
	CloseHandle(_CBdSysConf->Cnxt(_bd_id)->_dev_hnd);
	_CBdSysConf->Cnxt(_bd_id)->_dev_hnd = NULL;
#else
	_CBdSysConf->Cnxt(_bd_id)->_dev_hnd = NULL;
#endif
	return 0;
}


//----------------------------------------------------------------------------------
//
//	DLL main entry
#if defined(WIN32)
_declspec(dllexport) int _stdcall DllMain(HANDLE hinstDll, DWORD fdwReason, LPVOID lpReserved)
#else
extern "C" int DllMain(int fdwReason, int Slot, int ModulatorType, int IF)
#endif
{
	switch (fdwReason) 
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		ProcessDetach();
		break;

	default:
		break;
	}
	return(1);
}

//////////////////////////////////////////////////////////////////////////////////////	inerface to call any bd-resources.



