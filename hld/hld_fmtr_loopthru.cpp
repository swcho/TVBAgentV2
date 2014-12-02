
#if defined(WIN32)
#include	<Windows.h>
#include	<process.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<pthread.h>
#endif
#include	<stdio.h>
#include	<math.h>

#include	"../include/hld_structure.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"hld_fmtr_loopthru.h"
#include	"LLDWrapper.h"


#if defined(WIN32)
#define CHECK_TIME_START	QueryPerformanceFrequency((_LARGE_INTEGER*)&gFreq);QueryPerformanceCounter((_LARGE_INTEGER*)&gStart);
#define CHECK_TIME_END		QueryPerformanceCounter((_LARGE_INTEGER*)&gEnd);gTime = (float)((double)(gEnd-gStart)/gFreq*1000);
#else
#define CHECK_TIME_START 
#define CHECK_TIME_END 
#endif

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrLoopThru::CHldFmtrLoopThru(int _my_id, void *_hld)	:	CHldFmtrAtscMH(_my_id, _hld),	CHldFmtrIsdbT13(_my_id, _hld)
{
	my_hld_id = _my_id;
	my_hld = _hld;

	inter_buf_capplay = NULL;

	inter_buf_rd_ptr = 0;
	inter_buf_wr_ptr = 0;

	cnt_byte_availabe_caped = 0;
	test_count_min = 0;
	test_count_max = 0;

	gSymbolClock = TS_SYMBOL_CLOCK_ISDB_T_13;
	gFreq = 0;
	gStart = 0;
	gEnd = 0;
	gTime = 0.;
	gPacketSize = 0;
	nEndASI = 0;

}

CHldFmtrLoopThru::~CHldFmtrLoopThru()
{
	if (inter_buf_capplay != NULL)
	{
		free(inter_buf_capplay);
	}
	inter_buf_capplay = NULL;


}
void	CHldFmtrLoopThru::AllocateCapPlayBufIsdbTAtscMH(void)
{
	if (inter_buf_capplay == NULL)
	{
		inter_buf_capplay = (unsigned char*)malloc(TMCC_REMUXER_PRE_BUFFER_SIZE);	//	32Mbyte
		if (inter_buf_capplay == NULL)
		{
			printf("Fail to alloc. buffer---1\n");
		}
	}
}
void	CHldFmtrLoopThru::FreeCapPlayBufIsdbTAtscMH(void)
{
	if (inter_buf_capplay != NULL)
	{
		free(inter_buf_capplay);
	}
	inter_buf_capplay = NULL;
}

void	CHldFmtrLoopThru::SetCommonMethod_8(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}
void CHldFmtrLoopThru::InitCapParam_IsdbT13_AtscMH(void)
{
	inter_buf_rd_ptr = inter_buf_wr_ptr = cnt_byte_availabe_caped = 0;
	cnt_byte_availabe_caped = test_count_min = test_count_max = 0;
}
void CHldFmtrLoopThru::SetSymClock(double _clk)
{
	gSymbolClock = _clk;
}


int CHldFmtrLoopThru::AdjCapPlayClkSpeed(int pos)
{
	double diff_time, diff_count, diff_symbol_clock = 0;

	__FIf__->RdFpgaCapPlay_BufLevel(200);

	//if(__Sta__->IsState_DvbT2LoopThru() || __Sta__->IsState_DvbC2LoopThru())
	return 0;

	if ( __FIf__->TL_gTotalSendData <= TMCC_REMUXER_PRE_BUFFER_SIZE )	//	32M
	{
		test_count_max = test_count_min = cnt_byte_availabe_caped;

		CHECK_TIME_START
	}
	else
	{
		if ( cnt_byte_availabe_caped < test_count_min )
			test_count_min = cnt_byte_availabe_caped;
		else if ( cnt_byte_availabe_caped > test_count_max )
			test_count_max = cnt_byte_availabe_caped;
		
		//CAPTURE : SLOW, PLAY : FAST
		diff_count = fabs((double)cnt_byte_availabe_caped-(double)test_count_max);
		if ( diff_count >= TS_BUFFER_MARGINE )
		{
			CHECK_TIME_END

			diff_time = gTime/1000.;
			if ( diff_time > 0 )
			{
				diff_symbol_clock = 0. - (diff_count*8. / diff_time);
				gSymbolClock = gSymbolClock + diff_symbol_clock*2.;
				
				if ( (__Sta__->IsModTyp_AtscMH() && ((TS_SYMBOL_CLOCK_MH - TS_SYMBOL_CLOCK_OFFSET_MH) <= gSymbolClock && (TS_SYMBOL_CLOCK_MH + TS_SYMBOL_CLOCK_OFFSET_MH) >= gSymbolClock))
					|| (__Sta__->IsModTyp_IsdbT_13() && ((TS_SYMBOL_CLOCK_ISDB_T_13 - TS_SYMBOL_CLOCK_OFFSET_ISDB_T_13) <= gSymbolClock && (TS_SYMBOL_CLOCK_ISDB_T_13 + TS_SYMBOL_CLOCK_OFFSET_ISDB_T_13) >= gSymbolClock)) 
					|| (__Sta__->IsModTyp_IsdbS() && ((TS_SYMBOL_CLOCK_ISDBS - TS_SYMBOL_CLOCK_OFFSET_ISDBS) <= gSymbolClock && (TS_SYMBOL_CLOCK_ISDBS + TS_SYMBOL_CLOCK_OFFSET_ISDBS) >= gSymbolClock)) ) //2011/8/4 ISDB-S ASI
				{
					sprintf(debug_string, "slow...diff_count=%f, count=%f, max=%f\n", diff_count, cnt_byte_availabe_caped/1048576., test_count_max/1048576.);
					__HLog__->HldPrint( debug_string);
					sprintf(debug_string, "diff_time=%f\n", diff_time);
					__HLog__->HldPrint( debug_string);
					sprintf(debug_string, "diff_symbol_clock=%f, %f\n\n", diff_symbol_clock, gSymbolClock);
					__HLog__->HldPrint( debug_string);

					//TVB593
					__FIf__->TSPL_SET_SYMBOL_CLOCK(__Sta__->TL_gCurrentModType, (long)gSymbolClock);
				}
			}

			test_count_max = test_count_min = cnt_byte_availabe_caped;

			CHECK_TIME_START
		}

		//CAPTURE : FAST, PLAY : SLOW
		diff_count = fabs((double)cnt_byte_availabe_caped-(double)test_count_min);
		if ( diff_count >= TS_BUFFER_MARGINE )
		{
			CHECK_TIME_END

			diff_time = gTime/1000.;
			if ( diff_time > 0 )
			{
				diff_symbol_clock = (diff_count*8. / diff_time);
				gSymbolClock = gSymbolClock + diff_symbol_clock*2.;
			
				if ( (__Sta__->IsModTyp_AtscMH() && ((TS_SYMBOL_CLOCK_MH - TS_SYMBOL_CLOCK_OFFSET_MH) <= gSymbolClock && (TS_SYMBOL_CLOCK_MH + TS_SYMBOL_CLOCK_OFFSET_MH) >= gSymbolClock))
					|| (__Sta__->IsModTyp_IsdbT_13() && ((TS_SYMBOL_CLOCK_ISDB_T_13 - TS_SYMBOL_CLOCK_OFFSET_ISDB_T_13) <= gSymbolClock && (TS_SYMBOL_CLOCK_ISDB_T_13 + TS_SYMBOL_CLOCK_OFFSET_ISDB_T_13) >= gSymbolClock))
					|| (__Sta__->IsModTyp_IsdbS() && ((TS_SYMBOL_CLOCK_ISDBS - TS_SYMBOL_CLOCK_OFFSET_ISDBS) <= gSymbolClock && (TS_SYMBOL_CLOCK_ISDBS + TS_SYMBOL_CLOCK_OFFSET_ISDBS) >= gSymbolClock)) ) //2011/8/4 ISDB-S ASI
				{
					sprintf(debug_string, "fast...diff_count=%f, count=%f, min=%f\n", diff_count, cnt_byte_availabe_caped/1048576., test_count_min/1048576.);
					__HLog__->HldPrint( debug_string);
					sprintf(debug_string, "diff_time=%f\n", diff_time);
					__HLog__->HldPrint( debug_string);
					sprintf(debug_string, "diff_symbol_clock=%f, %f\n\n", diff_symbol_clock, gSymbolClock);
					__HLog__->HldPrint( debug_string);

					//TVB593
					__FIf__->TSPL_SET_SYMBOL_CLOCK(__Sta__->TL_gCurrentModType, (long)gSymbolClock);
				}
			}

			test_count_max = test_count_min = cnt_byte_availabe_caped;

			CHECK_TIME_START
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrLoopThru::SrchSync_CapDta_IsdbT13_AtscMH(int *_nSyncStartPos, int *_iTSize)
{
	int nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	int nSyncStartPos = 0, iTSize = 204;

	if ( __FIf__->TSSyncLock == TRUE)
	{
		if(StaMachineIsdbT13() == _STA_ISDBT13_loopthru_initial_requsted)
		{
			ProcessStaMachineIsdbT13(_CMD_ISDBT13_detected_pkt_size, NULL, Get_LoopThru_PacketSize(), NULL);
		}
	}

	if ( __FIf__->TSSyncLock == FALSE )
	{
		//2011/8/5 ISDB-S ASI
		if(__Sta__->IsModTyp_IsdbS())
		{
			nSyncStartPos = __FIf__->TL_SyncLockFunction_ISDBS((unsigned char *)__FIf__->DmaPtr_HostSide(), nBankBlockSize, &iTSize, nBankBlockSize, 3);
			__FIf__->TL_gPacketSize = iTSize;
		}
		else
		{
			nSyncStartPos = __FIf__->TL_SyncLockFunction((char *)__FIf__->DmaPtr_HostSide(), nBankBlockSize, &iTSize, 5630, 20);
		}
		if ( nSyncStartPos >= 0 )
		{
			//2011/11/14
			Set_LoopThru_PacketSize(iTSize);
			__FIf__->TSSyncLock = TRUE;
	
			if ( __Sta__->IsModTyp_AtscMH() && (iTSize == 188 || iTSize == 204 || iTSize == 208) )
			{
				__FIf__->TL_gPacketSize = iTSize;
	
				m_MHE_packet_PID = __FIf__->Get_MPE_Packet_PID(
						(unsigned char*)__FIf__->DmaPtr_HostSide(), nBankBlockSize, nSyncStartPos, iTSize);
				InitMHEInfo(m_MHE_packet_PID);
			}
			//ProcessStaMachineIsdbT13(_CMD_ISDBT13_detected_pkt_size, NULL, iTSize, NULL);
		}
		*_nSyncStartPos = nSyncStartPos;
		*_iTSize = iTSize;
	}


	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrLoopThru::WrFs_and_FillInterBuf_CapDta_IsdbT13_AtscMH(int nSyncStartPos, int *_RestByte)
{
	int	i;
	unsigned long dwRet;

	__HLog__->HldPrint_RdHw_LoopThruMode(__Sta__->TL_gCurrentModType);
	if ( __FIf__->TSSyncLock == TRUE && nSyncStartPos == 0 )	//	????
//	if ( __FIf__->TSSyncLock == TRUE )
	{
		if ( !__FIf__->IsInvalidFileHandle() && __Sta__->IsTaskState_ContRec() )
		{
			__FIf__->_FWrite_(__FIf__->AP_hFile, (unsigned char *)(__FIf__->DmaPtr_HostSide() + nSyncStartPos), *_RestByte, &dwRet);
		}

		if ( inter_buf_capplay )
		{
			memcpy(inter_buf_capplay + inter_buf_wr_ptr, (unsigned char *)__FIf__->DmaPtr_HostSide(), *_RestByte);	//	*_RestByte is fixed value DMA_TRANS_UNIT_CAP_PLAY
			inter_buf_wr_ptr += *_RestByte;
			if ( inter_buf_wr_ptr >= TMCC_REMUXER_PRE_BUFFER_SIZE )
				inter_buf_wr_ptr -= TMCC_REMUXER_PRE_BUFFER_SIZE;
			cnt_byte_availabe_caped += *_RestByte;

			__FIf__->TL_i64TotalFileSize += *_RestByte;
			__FIf__->TL_gTotalSendData += *_RestByte;
		}
	}

	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrLoopThru::FillPlayBuf_from_CaptureBuf(void)
{
	int	nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	int	i;

	__FIf__->FillPlayBuf_from_GivenBuf(inter_buf_capplay + inter_buf_rd_ptr, nBankBlockSize);	//	nBankBlockSize is fixed value DMA_TRANS_UNIT_CAP_PLAY

	
	inter_buf_rd_ptr += nBankBlockSize;
	if ( inter_buf_rd_ptr >= TMCC_REMUXER_PRE_BUFFER_SIZE )
		inter_buf_rd_ptr -= TMCC_REMUXER_PRE_BUFFER_SIZE;
	cnt_byte_availabe_caped -= nBankBlockSize;

	return	0;
}
int CHldFmtrLoopThru::WrFs_and_FillInterBuf_from_GivenBuf(int _size, unsigned char *_buf)
{
	unsigned long	dwRet;
	unsigned int	lTemp;

	if ( !__FIf__->IsInvalidFileHandle() && __Sta__->IsTaskState_ContRec() )
	{
		__FIf__->_FWrite_(__FIf__->AP_hFile, _buf, _size, &dwRet);
	}
	if ( inter_buf_capplay )
	{
		if (inter_buf_wr_ptr <= (unsigned int)((TMCC_REMUXER_PRE_BUFFER_SIZE - _size)))
		{
			memcpy(inter_buf_capplay + inter_buf_wr_ptr, _buf, _size);
		}
		else 
		{
			lTemp = TMCC_REMUXER_PRE_BUFFER_SIZE - inter_buf_wr_ptr;
			memcpy(inter_buf_capplay + inter_buf_wr_ptr, _buf, lTemp);
			memcpy(inter_buf_capplay, _buf + lTemp, _size - lTemp);
		}
		inter_buf_wr_ptr += _size;
		if (inter_buf_wr_ptr >= TMCC_REMUXER_PRE_BUFFER_SIZE)
			inter_buf_wr_ptr -= TMCC_REMUXER_PRE_BUFFER_SIZE;
		cnt_byte_availabe_caped += _size;

		__FIf__->TL_i64TotalFileSize += _size;
		__FIf__->TL_gTotalSendData += _size;
	}

	return	0;
}
void CHldFmtrLoopThru::Rd188Captured_RemuedDta(void)
{
	int	_ret;

	if (!IsLoopthru188_IsdbT13())
	{
		return;
	}

	if (cnt_byte_availabe_caped > (TMCC_REMUXER_PRE_BUFFER_SIZE*2/3))
		//	this level control is relative to the level of RsltDta_ReadByHwWriter and InputDta188Pkt_WrByHwCapturer in the CHldTmccRmx() 
	{
		return;
	}
	_ret = ProcessStaMachineIsdbT13(_CMD_ISDBT13_play_dta, NULL, DMA_TRANS_UNIT_CAP_PLAY*2, &rd_buf_rmx);
	if (_ret == 2)	//	188 pkt
	{
		WrFs_and_FillInterBuf_from_GivenBuf(DMA_TRANS_UNIT_CAP_PLAY*2, rd_buf_rmx);
	}
}
int CHldFmtrLoopThru::RemuxDta_and_PreparePlayBuf_UsingCapDta_IsdbT13_AtscMH(void)	//	copy capture data into TL_szBufferPlay
{
	int		nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;

	if ( __Sta__->IsModTyp_IsdbT_13() )
	{
		FillPlayBuf_from_CaptureBuf();
//		TL_DelayAdjustment();
	}
	else if ( __Sta__->IsModTyp_AtscMH() )
	{
		FillPlayBuf_from_CaptureBuf();
//		FindSyncPos_and_FillSyncBuf(TL_gPacketSize, &__dwRet);
	}
	else if ( __Sta__->IsModTyp_IsdbS() )	//	2011/8/4 ISDB-S ASI
	{
		FillPlayBuf_from_CaptureBuf();
//		FindSyncPos_and_FillSyncBuf(TL_gPacketSize, &__dwRet);
	}

	return	1;
}
void CHldFmtrLoopThru::CntlFpga_PlayMode_and_WrDta_IsdbT13_AtscMH__HasEndlessWhile(void)
{
	int nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;

	__HLog__->HldPrint_WrHw_LoopThruMode(__Sta__->TL_gCurrentModType);
	__FIf__->SetHwFifoCntl_(0, nBankBlockSize);	//	PLAY MODE
	__FIf__->SetHwDmaDiection_Play();
	/* FILL 2ND PLAY BUFFER(pLLD->TL_sz2ndBufferPlay) */
	if ( __FIf__->Fill_Playback_DmaBuffer(0, nBankBlockSize) == 0 )
	{
		return;
	}
#ifdef WIN32
	__FIf__->StartDmaTransfer_Play_Loopthru(nBankBlockSize);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
void CHldFmtrLoopThru::ApplySymClock_AdjustedThisTurn(void)
{
	gFreq = 0;
	gStart = 0;
	gEnd = 0;
	gTime = 0.;

	if (__Sta__->IsModTyp_IsdbT_13())
	{
		SetSymClock(TS_SYMBOL_CLOCK_ISDB_T_13);
	}
	else if (__Sta__->IsModTyp_AtscMH())
	{
		SetSymClock(TS_SYMBOL_CLOCK_MH);
	}
	//2011/8/5 ISDB-S ASI
	else if(__Sta__->IsModTyp_IsdbS())
	{
		SetSymClock(TS_SYMBOL_CLOCK_ISDBS);
	}
//	TVB593
	if(__Sta__->IsModTyp_IsdbT_13() || __Sta__->IsModTyp_AtscMH() || __Sta__->IsModTyp_IsdbS())	//2011/8/5 ISDB-S ASI
	{
		__FIf__->TSPL_SET_SYMBOL_CLOCK(__Sta__->TL_gCurrentModType, (long)gSymbolClock);
	}
}


//////////////////////////////////////////////////////////////////////////////////////
void	CHldFmtrLoopThru::Launch_LoopThruCapPlayTask(void)
{
	CHld	*_hld_ = (CHld *)my_hld;

	//2011/11/14
	Set_LoopThru_PacketSize(0);
	AllocateCapPlayBufIsdbTAtscMH();
	__FIf__->initialCntl_for_IsdbT13AtscMh_CaptureMode();

	__FIf__->gfRatio = 1.0;

	if ( __Sta__->IsTaskStartingCond_IsdbT13_CapPlay() ||
		__Sta__->IsTaskStartingCond_AtscMh_CapPlay() ||
		__Sta__->IsTaskStartingCond_IsdbS_CapPlay())
	{
#if defined(WIN32)
		SetProcessAffinityMask(GetCurrentProcess(), 1); 

		_beginthread(_hld_->TaskPlay_and_LoopThru_IsdbT13_AtscMH, 0, (PVOID)_hld_);
		_beginthread(_hld_->TaskCap_and_LoopThru_IsdbT13_AtscMH, 0, (PVOID)_hld_);
#else
		pthread_t	 a_thread_play, a_thread_capture;
		pthread_create(&a_thread_play, NULL, _hld_->TaskPlay_and_LoopThru_IsdbT13_AtscMH, (PVOID)_hld_);
		pthread_create(&a_thread_capture, NULL, _hld_->TaskCap_and_LoopThru_IsdbT13_AtscMH, (PVOID)_hld_);
#endif
	}
}

//2012/7/10 DVB-T2 ASI
void	CHldFmtrLoopThru::Launch_LoopThruCapTask(void)
{
	CHld	*_hld_ = (CHld *)my_hld;

	//2011/11/14
	Set_LoopThru_PacketSize(0);
	AllocateCapPlayBufIsdbTAtscMH();
	__FIf__->initialCntl_for_IsdbT13AtscMh_CaptureMode();

	__FIf__->gfRatio = 1.0;

	if (__Sta__->IsTaskStartingCond_DvbT2_Cap() || __Sta__->IsTaskStartingCond_DvbC2_Cap())
	{
#if defined(WIN32)
		SetProcessAffinityMask(GetCurrentProcess(), 1); 
		_beginthread(_hld_->TaskCap_and_LoopThru_DvbT2, 0, (PVOID)_hld_);
#else
		pthread_t	 a_thread_play, a_thread_capture;
		pthread_create(&a_thread_capture, NULL, _hld_->TaskCap_and_LoopThru_DvbT2, (PVOID)_hld_);
#endif
	}
}



#if defined(WIN32)
void	CHldFmtrLoopThru::TaskPlay_and_LoopThru_IsdbT13_AtscMH(PVOID param)
#else
void*	CHldFmtrLoopThru::TaskPlay_and_LoopThru_IsdbT13_AtscMH(PVOID param)
#endif
{
	CHld	*_hld_;
	int	nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	int	_ret;

	_hld_ = (CHld *)param;

	_hld_->_HLog->HldPrint("Hld-Mgr. Launch loopthru PlayTask for isddT13 and AtscMh");

	if (_hld_->_SysSta->IsModTyp_IsdbT_13())
	{
		_hld_->SetSymClock(TS_SYMBOL_CLOCK_ISDB_T_13);
	}
	else if (_hld_->_SysSta->IsModTyp_AtscMH())
	{
		_hld_->SetSymClock(TS_SYMBOL_CLOCK_MH);
	}
	else if (_hld_->_SysSta->IsModTyp_IsdbS())	//2011/8/4 ISDB-S ASI
	{
		_hld_->SetSymClock(TS_SYMBOL_CLOCK_ISDBS);
	}

	_hld_->InitCapParam_IsdbT13_AtscMH();
	_hld_->_SysSta->SetFlag_PlayLoopThru(1);	//	starting loopthru of isdb...
	while ( _hld_->_SysSta->IsPlayMod_IsdbT13_or_AtscMH() || _hld_->_SysSta->IsPlayMod_IsdbS())	//2011/8/4 ISDB-S ASI
	{
		_hld_->Rd188Captured_RemuedDta();
		if ( _hld_->cnt_byte_availabe_caped > TMCC_REMUXER_PRE_BUFFER_SIZE - (TMCC_REMUXER_PRE_BUFFER_SIZE/2.) )		//	wait initial data. should have enough data.
		{
			printf("[ASI IN] Enough data \n");
			break;
		}
		Sleep(10);
	}

/////////////////////////////////////////////////////////////////
	while ( _hld_->_SysSta->IsPlayMod_IsdbT13_or_AtscMH() || _hld_->_SysSta->IsPlayMod_IsdbS())	//2011/8/4 ISDB-S ASI
	{
		//_hld_->AdjCapPlayClkSpeed(0);	// CHECK FPGA PLAY BUFFER HAS DATA TO BE PLAYED

		_hld_->Rd188Captured_RemuedDta();

		if (_hld_->_FIf->IsFpgaPlayBuf_Overrun_toMaxLevel())
		{
			Sleep(10);
			continue;
		}
		if ( _hld_->cnt_byte_availabe_caped < (unsigned int)nBankBlockSize )	// CHECK STREAM BUFFER IS UNDERFLOW
		{
			Sleep(10);
			continue;
		}

		_hld_->_FIf->LockFileMutex();
		_ret = _hld_->RemuxDta_and_PreparePlayBuf_UsingCapDta_IsdbT13_AtscMH();	//	copy captured data inter_buf_capplay into TL_szBufferPlay
		if (_ret == 0)
		{
			_hld_->_FIf->UnlockFileMutex();
			continue;
		}
		if ( _hld_->_SysSta->IsModTyp_IsdbT_13() )
		{
			_hld_->TL_DelayAdjustment();	//	fill adj ONE PKT into TL_sz2ndBufferPlay. and orig source frame is based on the TL_szBufferPlay
		}
		else if ( _hld_->_SysSta->IsModTyp_AtscMH() )
		{
			_hld_->_FIf->FindSyncPos_and_FillSyncBuf(_hld_->_FIf->TL_gPacketSize, _hld_->PktSize_IsdbT13_AtscMH());	
		}
		else if( _hld_->_SysSta->IsModTyp_IsdbS() )	//2011/8/4 ISDB-S ASI
		{
			DWORD	dwRet = 204;
			_hld_->TL_ReplaceCombinedTS_IN_ASI__HasEndlessWhile(_hld_->_FIf->TL_gPacketSize, &dwRet);
		}

		_hld_->CntlFpga_PlayMode_and_WrDta_IsdbT13_AtscMH__HasEndlessWhile();		//	playback. source data is TL_sz2ndBufferPlay.
		_hld_->_FIf->UnlockFileMutex();
	}

	//_hld_->_FIf->UnlockFileMutex();

	while ( _hld_->_SysSta->Flag_CapLoopThru() )
	{
		Sleep(10);
	}
	_hld_->_HLog->HldPrint("Hld-Mgr. Term loopthru PlayTask for isddT13 and AtscMh");

	_hld_->ChgStaMachineIsdbT13(_STA_ISDBT13_loopthru_188_onendreqed);

	_hld_->ApplySymClock_AdjustedThisTurn();
	_hld_->InitCapParam_IsdbT13_AtscMH();

	_hld_->now_active_tmcc_remuxing = 0;
	_hld_->_SysSta->SetFlag_PlayLoopThru(0);

	_hld_->FreeCapPlayBufIsdbTAtscMH();

#if defined(WIN32)	
	unsigned long dwProcAffinityMask, dwSystemAffinityMask;
	GetProcessAffinityMask(GetCurrentProcess(), &dwProcAffinityMask, &dwSystemAffinityMask); 
	SetProcessAffinityMask(GetCurrentProcess(), dwSystemAffinityMask); 

	//OutputDebugString("===>PLAY END\n");
#endif
#if defined(WIN32)
	return;
#else
	return 0;
#endif
}
#if defined(WIN32)
void	CHldFmtrLoopThru::TaskCap_and_LoopThru_IsdbT13_AtscMH(PVOID param)
#else
void*	CHldFmtrLoopThru::TaskCap_and_LoopThru_IsdbT13_AtscMH(PVOID param)
#endif
{
	CHld	*_hld_;
	int	nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	int	nSyncStartPos = 0, iTSize = 204, RestByte, _ret = 1;

	_hld_ = (CHld *)param;

	_hld_->_HLog->HldPrint("Hld-Mgr. Launch loopthru CapTask for isddT13 and AtscMh");

	_hld_->ChgStaMachineIsdbT13(_STA_ISDBT13_loopthru_initial_requsted);

	_hld_->_SysSta->SetFlag_CapLoopThru(1);	//	starting loopthru-mode of IsdbT13_or_AtscMH
	while (_hld_->_SysSta->IsCapMod_IsdbT13_or_AtscMH() || _hld_->_SysSta->IsCapMod_IsdbS())
	{
//		if (_hld_->_FIf->LockFileMutex_Timeout() == 0)
//		{
//			continue;
//		}

		_hld_->AdjCapPlayClkSpeed(100);		// CHECK FPGA CAPTURE BUFFER IS UNDERFLOW
		_hld_->_FIf->ChkLockAsiInput_Reg0x600042();
		_hld_->_FIf->RdCntAsiInputTs_Reg0x600044();

		if (_hld_->_FIf->IsFpgaCapBuf_Underrun_toMinLevel())
		{
//			_hld_->_FIf->UnlockFileMutex();
			Sleep(10);
			continue;	//	wait fullness of capture buffer
		}

		_hld_->_FIf->LockFileMutex();
		_hld_->_FIf->StartDmaTransfer_Capture_Loopthru(0);	//	to be started dma. and cap-dta stored in dma-buf.
		if ( _hld_->_FIf->TL_gTotalSendData < nBankBlockSize*MAX_BANK_NUMBER )		// SKIP THE PREVIOUS BANK DATA
		{
			_hld_->_FIf->TL_gTotalSendData += nBankBlockSize;
		}
		else
		{
			nSyncStartPos = 0;
			_hld_->SrchSync_CapDta_IsdbT13_AtscMH(&nSyncStartPos, &iTSize);
			RestByte = nBankBlockSize - nSyncStartPos;
			if (nSyncStartPos == 0)
			{
				if(_hld_->_FIf->TmccUsing())
				{
					_ret = _hld_->ProcessStaMachineIsdbT13(_CMD_ISDBT13_capture_dta, (unsigned char *)_hld_->_FIf->DmaPtr_HostSide(), RestByte, NULL);
				}
				if (_ret == 1)	//	not 188 pkt
				{
					_hld_->WrFs_and_FillInterBuf_CapDta_IsdbT13_AtscMH(nSyncStartPos, &RestByte);	//	get capture data into inter_buf_capplay from hw-buf.
				}
//				_hld_->Rd188Captured_RemuedDta();
			}
		}

		_hld_->_FIf->UnlockFileMutex();
	}

	//_hld_->_FIf->UnlockFileMutex();
	_hld_->_SysSta->SetFlag_CapLoopThru(0);

	_hld_->_HLog->HldPrint("Hld-Mgr. Term loopthru CapTask for isddT13 and AtscMh");

#if defined(WIN32)
	return;
#else
	return 0;
#endif
}

void	CHldFmtrLoopThru::Set_LoopThru_PacketSize(int _val)
{
	gPacketSize = _val;
}

int		CHldFmtrLoopThru::Get_LoopThru_PacketSize(void)
{
	return gPacketSize;
}

//2012/7/18 DVB-T2 ASI
#if defined(WIN32)
void	CHldFmtrLoopThru::TaskCap_and_LoopThru_DvbT2(PVOID param)
#else
void*	CHldFmtrLoopThru::TaskCap_and_LoopThru_DvbT2(PVOID param)
#endif
{
	CHld	*_hld_;
	int	nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY * 2;
	int	nSyncStartPos = -1, iTSize = 204, RestByte, _ret;
	int AsiLock;
	_hld_ = (CHld *)param;

	_hld_->_HLog->HldPrint("Hld-Mgr. Launch loopthru CapTask for isddT13 and AtscMh");


	_hld_->_SysSta->SetFlag_CapLoopThru(1);	//	starting loopthru-mode of IsdbT13_or_AtscMH
	//buffering 8Mbyte
_SYNC_SEARCH_:
	//8Mbyte buffering
	_ret = _hld_->Init_capture_buffer_dvbt2((PVOID)_hld_);
//	OutputDebugString("===>Init_capture_buffer_dvbt2 END\n");
	if(_ret == 0)
		goto _SYNC_SEARCH_;
	else if(_ret == 1)
	{
		//_hld_->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_->_SysSta->TL_gCurrentModType, 0);
		_hld_->SetFinishAsi(0);
		playback_receive_request(_hld_, PLAY_MESSAGE_INIT, NULL);
		playback_receive_request(_hld_, PLAY_MESSAGE_PLAY, NULL);
	}
	else
		goto __END_TASK__;

//	OutputDebugString("===>Start TaskCap_and_LoopThru_DvbT2 main \n");
#if 0
	SYSTEMTIME system_time;
	SYSTEMTIME system_time_old;
	GetLocalTime(&system_time_old);
	long hour, minute, seconds, milisecond, useconds;
	long sum_miliseconds;
	char str_debug[128];
#endif
	while (_hld_->_SysSta->IsCapMod_DvbT2() || _hld_->_SysSta->IsCapMod_DvbC2())	//2012/7/10 DVB-T2 ASI
	{
		if(_hld_->_SysSta->ReqedNewAction_User())
			break;

		_hld_->AdjCapPlayClkSpeed(100);		// CHECK FPGA CAPTURE BUFFER IS UNDERFLOW
		_hld_->_FIf->ChkLockAsiInput_Reg0x600042();
		_hld_->_FIf->RdCntAsiInputTs_Reg0x600044();

		AsiLock = _hld_->_FIf->IsAsiInputLocked();
		if((AsiLock & 0x8) < 1)
		{
			_hld_->SetFinishAsi(1);
			//_hld_->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_->_SysSta->TL_gCurrentModType, 1);
			playback_receive_request(_hld_, PLAY_MESSAGE_STOP, NULL);
			playback_receive_request(_hld_, PLAY_MESSAGE_QUIT, NULL);
			goto _SYNC_SEARCH_;
		}
		
		if (_hld_->_FIf->IsFpgaCapBuf_Underrun_toMinLevel())
		{
			Sleep(10);
			continue;	//	wait fullness of capture buffer
		}

//		OutputDebugString("1\n");
		_hld_->_FIf->LockFileMutex();
		_hld_->_FIf->LockBufferMutex();
		_hld_->_FIf->StartDmaTransfer_Capture_Loopthru(0);	//	to be started dma. and cap-dta stored in dma-buf.
		_hld_->_FIf->UnlockFileMutex();
		//_hld_->_FIf->LockBufferMutex();
		_hld_->WrFs_and_FillInterBuf_CapDta_DvbT2(nBankBlockSize);	//	get capture data into inter_buf_capplay from hw-buf.
//		OutputDebugString("4\n");
		_hld_->_FIf->UnlockBufferMutex();
#if 0
		GetLocalTime(&system_time);

		hour = (system_time.wHour - system_time_old.wHour)*60*60*1000;
		minute = (system_time.wMinute - system_time_old.wMinute)*60*1000;
		seconds = (system_time.wSecond - system_time_old.wSecond)*1000;
		milisecond = system_time.wMilliseconds - system_time_old.wMilliseconds;

		sum_miliseconds = hour + minute + seconds + milisecond;
		sprintf(str_debug, "Time = %d\n", sum_miliseconds); 
		OutputDebugString(str_debug);
		system_time_old = system_time;
#endif
	}

__END_TASK__:
	//_hld_->_FIf->UnlockFileMutex();
	_hld_->_SysSta->SetFlag_CapLoopThru(0);
//	OutputDebugString("===>END TaskCap_and_LoopThru_DvbT2 main \n");
	if (_hld_->_SysSta->is_new_playback())
	{
		_hld_->SetFinishAsi(1);
		playback_receive_request(_hld_, PLAY_MESSAGE_STOP, NULL);
		playback_receive_request(_hld_, PLAY_MESSAGE_QUIT, NULL);
//		OutputDebugString("===>END DvbT2 \n");
	}

	_hld_->FreeCapPlayBufIsdbTAtscMH();

	_hld_->_HLog->HldPrint("Hld-Mgr. Term loopthru CapTask for isddT13 and AtscMh");
#if defined(WIN32)	
	unsigned long dwProcAffinityMask, dwSystemAffinityMask;
	GetProcessAffinityMask(GetCurrentProcess(), &dwProcAffinityMask, &dwSystemAffinityMask); 
	SetProcessAffinityMask(GetCurrentProcess(), dwSystemAffinityMask); 

	//OutputDebugString("===>PLAY END\n");
#endif
#if defined(WIN32)
	return;
#else
	return 0;
#endif
}

int	CHldFmtrLoopThru::Init_capture_buffer_dvbt2(PVOID param)
{
	CHld	*_hld_;
	int AsiLock;
	int	nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY * 2;
	int	nSyncStartPos = -1, iTSize = 204;
	unsigned long dwRet;

	_hld_ = (CHld *)param;
	inter_buf_wr_ptr = 0;
	inter_buf_rd_ptr = 0;
	cnt_byte_availabe_caped = 0;
	__FIf__->TL_i64TotalFileSize = 0;
	__FIf__->TL_gTotalSendData = 0;
	nSyncFind = 0;

	while(_hld_->_SysSta->IsCapMod_DvbT2() || _hld_->_SysSta->IsCapMod_DvbC2())
	{
		_hld_->AdjCapPlayClkSpeed(100);		// CHECK FPGA CAPTURE BUFFER IS UNDERFLOW
		_hld_->_FIf->ChkLockAsiInput_Reg0x600042();
		_hld_->_FIf->RdCntAsiInputTs_Reg0x600044();

		AsiLock = _hld_->_FIf->IsAsiInputLocked();
		if((AsiLock & 0x8) < 1)
		{
			inter_buf_wr_ptr = 0;
			cnt_byte_availabe_caped = 0;
			Sleep(10);
			continue;
		}

		if (_hld_->_FIf->IsFpgaCapBuf_Underrun_toMinLevel())
		{
			Sleep(10);
			continue;	//	wait fullness of capture buffer
		}
		_hld_->_FIf->LockFileMutex();
		_hld_->_FIf->StartDmaTransfer_Capture_Loopthru(0);	//	to be started dma. and cap-dta stored in dma-buf.
		_hld_->_FIf->UnlockFileMutex();
		if ( _hld_->_FIf->TL_gTotalSendData < nBankBlockSize*(MAX_BANK_NUMBER / 2))		// SKIP THE PREVIOUS BANK DATA
		{
			_hld_->_FIf->TL_gTotalSendData += nBankBlockSize;
		}
		else
		{
			if ( inter_buf_capplay )
			{
#if 0 //not use
				if(nSyncStartPos == -1)
					nSyncStartPos = _hld_->_FIf->TL_SyncLockFunction((char*)__FIf__->DmaPtr_HostSide(), nBankBlockSize, &iTSize, nBankBlockSize, 3);
				else //next syncPos
				{
					if(checkSyncByte(&nSyncStartPos, iTSize, nBankBlockSize, (char*)__FIf__->DmaPtr_HostSide()) == 0)
					{
						inter_buf_wr_ptr = 0;
						cnt_byte_availabe_caped = 0;
						nSyncStartPos = -1;
						continue;
					}
				}
#endif				
				memcpy(inter_buf_capplay + inter_buf_wr_ptr, (unsigned char *)__FIf__->DmaPtr_HostSide(), nBankBlockSize);	//	*_RestByte is fixed value DMA_TRANS_UNIT_CAP_PLAY
				inter_buf_wr_ptr += nBankBlockSize;
				cnt_byte_availabe_caped += nBankBlockSize;
				__FIf__->TL_i64TotalFileSize += nBankBlockSize;
				__FIf__->TL_gTotalSendData += nBankBlockSize;
				if(__FIf__->AP_hFile != INVALID_HANDLE_VALUE && __FIf__->g_DvbT2_FileCaptureFlag == 1)
				{
					__FIf__->_FWrite_(__FIf__->AP_hFile, (unsigned char *)__FIf__->DmaPtr_HostSide(), nBankBlockSize, &dwRet);
					__FIf__->TL_i64TotalFileSize += nBankBlockSize;
				}
				else if(__FIf__->AP_hFile != INVALID_HANDLE_VALUE && __FIf__->g_DvbT2_FileCaptureFlag == 0)
				{
					__FIf__->CloseFile_of_OpendFile();
				}
			}
		}
		if(inter_buf_wr_ptr >= DVBT2_LOOPTHRU_ENABLE_BUFFER) //4Mbyte
		{
			nSyncStartPos = _hld_->_FIf->TL_SyncLockFunction((char*)inter_buf_capplay, cnt_byte_availabe_caped, &iTSize, cnt_byte_availabe_caped, 3);

			if (nSyncStartPos != -1)
			{
				_hld_->TL_CurrentBitrate = __FIf__->TL_Calculate_Bitrate(inter_buf_capplay, DVBT2_LOOPTHRU_LEVEL, nSyncStartPos, iTSize, NULL, NULL);
				_hld_->g_CurrentPacketSize = iTSize;
				inter_buf_rd_ptr = nSyncStartPos;
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return -1;
}
#if 0 //not use
int CHldFmtrLoopThru::checkSyncByte(int curSyncPos, int packetSize, int nBlockSize/*, char *buf*/)
{
	int nextSyncPos;
	nextSyncPos = packetSize - ((nBlockSize - *curSyncPos) % packetSize);
//	for(int i = 0; i < 3; i++)
//	{
//		if( buf[nextSyncPos + packetSize * i] != 0x47)
//			return 0;
//	}
	return nextSyncPos;
}
#endif
int CHldFmtrLoopThru::WrFs_and_FillInterBuf_CapDta_DvbT2(int nblocksize)
{
	int	i;
	unsigned long dwRet;

	if ( inter_buf_capplay )
	{
		memcpy(inter_buf_capplay + inter_buf_wr_ptr, (unsigned char *)__FIf__->DmaPtr_HostSide(), nblocksize);	//	*_RestByte is fixed value DMA_TRANS_UNIT_CAP_PLAY
		inter_buf_wr_ptr += nblocksize;
		if ( inter_buf_wr_ptr >= TMCC_REMUXER_PRE_BUFFER_SIZE )
			inter_buf_wr_ptr -= TMCC_REMUXER_PRE_BUFFER_SIZE;
		cnt_byte_availabe_caped += nblocksize;
		if(__FIf__->AP_hFile != INVALID_HANDLE_VALUE && __FIf__->g_DvbT2_FileCaptureFlag == 1)
		{
			__FIf__->_FWrite_(__FIf__->AP_hFile, (unsigned char *)__FIf__->DmaPtr_HostSide(), nblocksize, &dwRet);
			__FIf__->TL_i64TotalFileSize += nblocksize;
		}
		else if(__FIf__->AP_hFile != INVALID_HANDLE_VALUE && __FIf__->g_DvbT2_FileCaptureFlag == 0)
		{
			__FIf__->CloseFile_of_OpendFile();
		}
//		__FIf__->TL_i64TotalFileSize += nblocksize;
//		__FIf__->TL_gTotalSendData += nblocksize;
//		char str[100];
//		sprintf(str, "1 cnt_byte_availabe_caped = %d, wd_ptr = %d, rd_ptr = %d\n", cnt_byte_availabe_caped, inter_buf_wr_ptr, inter_buf_rd_ptr);
//		OutputDebugString(str);
	}

	return	0;
}
int	CHldFmtrLoopThru::GetFinishAsi()
{
	return nEndASI;
}

void CHldFmtrLoopThru::SetFinishAsi(int _val)
{
	nEndASI = _val;
}

int CHldFmtrLoopThru::GetAsiInputData(PVOID param, unsigned char *buf, int nBlockSize)
{
	int cur_sync_pos = -1;
	static int next_sync_pos = -1;
	int iTSize;
	int tmp;
	CHld	*_hld_;
	_hld_ = (CHld *)param;

	if(inter_buf_capplay)
	{
	//	if(nSyncFind == 0)
	//	{
	//		sync_pos = _hld_->_FIf->TL_SyncLockFunction((char*)inter_buf_capplay, cnt_byte_availabe_caped, &iTSize, cnt_byte_availabe_caped, 3);
	//		inter_buf_rd_ptr = sync_pos;
	//		nSyncFind = 1;
	//	}
		if((cnt_byte_availabe_caped - nBlockSize) < DVBT2_LOOPTHRU_LEVEL)
		{
			goto _NO_DATA_AVAILABLE_;
		}
		
		if((inter_buf_wr_ptr - inter_buf_rd_ptr) >= 0)
		{
			if( (inter_buf_wr_ptr - inter_buf_rd_ptr) < nBlockSize)
			{
				//printf("not enough data \n");
				goto _NO_DATA_AVAILABLE_;				
			}
		}
		else
		{
			if((inter_buf_wr_ptr + (TMCC_REMUXER_PRE_BUFFER_SIZE - inter_buf_rd_ptr)) < nBlockSize)
			{
				goto _NO_DATA_AVAILABLE_;				
			}
		}
		


		if((inter_buf_rd_ptr + nBlockSize) > TMCC_REMUXER_PRE_BUFFER_SIZE)
		{
			tmp = TMCC_REMUXER_PRE_BUFFER_SIZE - inter_buf_rd_ptr;
			memcpy(buf, inter_buf_capplay + inter_buf_rd_ptr, tmp);
			memcpy(buf + tmp, inter_buf_capplay, (nBlockSize - tmp));
			inter_buf_rd_ptr = (nBlockSize - tmp); 
		}
		else
		{
			memcpy(buf, inter_buf_capplay + inter_buf_rd_ptr, nBlockSize);
			inter_buf_rd_ptr = inter_buf_rd_ptr + nBlockSize;
		}
		cnt_byte_availabe_caped = cnt_byte_availabe_caped - nBlockSize;
	}
	else
	{
		goto _NO_DATA_AVAILABLE_;	
	}
	//char str[100];
	//sprintf(str, "2 cnt_byte_availabe_caped = %d, wd_ptr = %d, rd_ptr = %d\n", cnt_byte_availabe_caped, inter_buf_wr_ptr, inter_buf_rd_ptr);
	//OutputDebugString(str);

	return nBlockSize;

_NO_DATA_AVAILABLE_:
	//Sleep(10);
	return 0;
}


