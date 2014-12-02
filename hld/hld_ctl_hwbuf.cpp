
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<math.h>
#include	<string.h>

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif
#include	"hld_ctl_hwbuf.h"


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
CHldCtlHwBuf::CHldCtlHwBuf(void)
{

	TL_gTotalSendData	= 0;

	play_buffer = 0;
	capture_buffer = 0;

	play_buffer_status = 0;
	user_play_buffer_status = 0;
	wait_msec_bank_out = 0;

	reg_rd_interval = 0;
	reg_rd_interval_2 = 0;
	asi_in_is_locked = 0;
	rd_cnt_asi_in_ts = 0;


}

CHldCtlHwBuf::~CHldCtlHwBuf()
{
}

//----------------------------------------------------------------------------------
void	CHldCtlHwBuf::TL_ClearBuffer(int	ind)
{
	buf_queue_config[ind].stream_pos_1st = 0;
	buf_queue_config[ind].stream_pos_last = 0;
	buf_queue_config[ind].stream_progressing = 0;
}


//////////////////////////////////////////////////////////////////////////////////////	variables
int CHldCtlHwBuf::SizeOfBankBlk_CurConfed(void)
{
	int	_nBankBlockSize_ = (SdramSubBankSize());
	return	_nBankBlockSize_;
}

//////////////////////////////////////////////////////////////////////////////////////	buf state
int CHldCtlHwBuf::RdFpgaCapPlay_BufLevel(int pos)
{
	unsigned long dwRet;
	int nPW, nPR, nPC, nCW, nCR, nCC;

	dwRet = TSPL_READ_CONTROL_REG(0, 0x600021);

	nPW = dwRet & 0xFF; 
	nPR = (dwRet>>8) & 0xFF; 


	if ( nPW < nPR )
		nPC = 0x80 - nPR + nPW;
	else
		nPC = nPW - nPR;
	

	nCW = (dwRet>>16) & 0x7F; 
	nCR = (dwRet>>24) & 0x7F; 

	if ( nCW < nCR )
		nCC = 0x80 - nCR + nCW;
	else
		nCC = nCW - nCR;
	play_buffer = nPC;
	capture_buffer = nCC;
	//char debugStr[100];
	//sprintf(debugStr, "nPW = 0x%x, nPR = 0x%x, nPC = 0x%x, nCW = 0x%x, nCR = 0x%x, nCC = 0x%x\n", nPW, nPR, nPC, nCW, nCR, nCC);
	//OutputDebugString(debugStr);
	return 0;
}
int CHldCtlHwBuf::IsFpgaCapBuf_Underrun_toMinLevel(void)
{
	int nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	int shift_value = 13;
	if(__Sta_->IsModTyp_DvbT2() || __Sta_->IsModTyp_DvbC2())
		nBankBlockSize = nBankBlockSize * 2;
	if(__Sta_->IsAttachedTvbTyp_599() || __Sta_->IsAttachedTvbTyp_598())
		shift_value = 14;
	if (((capture_buffer*4) << shift_value) < nBankBlockSize)
	{
		return	1;
	}
	return	0;
}
int CHldCtlHwBuf::IsFpgaCapBuf_Underrun_toGivenLevel(void)
{
	int 	nBankBlockSize = (SdramSubBankSize());
	int shift_value = 13;
	if(__Sta_->IsAttachedTvbTyp_599() || __Sta_->IsAttachedTvbTyp_598())
		shift_value = 14;

	if (((capture_buffer*4)<<shift_value) < nBankBlockSize)
	{
		return	1;
	}
	return	0;
}
int CHldCtlHwBuf::IsFpgaPlayBuf_Overrun_toMaxLevel(void)
{
	int shift_value = 13;
	int playBuffer_MaxLevel = PLAY_BUF_MAX_LVL_when_LOOPTHTU;
	if(__Sta_->IsAttachedTvbTyp_599() || __Sta_->IsAttachedTvbTyp_598())
	{
		shift_value = 14;
		playBuffer_MaxLevel = PLAY_BUF_MAX_LVL_when_LOOPTHTU * 2;
	}
	if (((play_buffer*4)<<shift_value) >= playBuffer_MaxLevel)
	{
		return	1;
	}
	return	0;
}
int CHldCtlHwBuf::IsFpgaPlayBuf_Overrun_toMaxLevel_3M(void)
{
	int shift_value = 13;
	int playBuffer_MaxLevel = PLAY_BUF_MAX_LVL_when_LOOPTHTU_3M;
	if(__Sta_->IsAttachedTvbTyp_599() || __Sta_->IsAttachedTvbTyp_598())
	{
		shift_value = 14;
		playBuffer_MaxLevel = PLAY_BUF_MAX_LVL_when_LOOPTHTU_3M * 2;
	}
	if (((play_buffer*4)<<shift_value) >= playBuffer_MaxLevel)
	{
		return	1;
	}
	return	0;
}
int CHldCtlHwBuf::WaitFullness_IsdbT13CapBuf__HasEndlessWhile(void)
{
	if (__Sta_->IsState_IsdbT13LoopThru())
	{
		while (1)
		{
			if ( __Sta_->ReqedNewAction_User() ) 
				return 0;

			RdFpgaCapPlay_BufLevel(200);
			if ( IsFpgaCapBuf_Underrun_toGivenLevel() )
			{
				Sleep(50);		//FIXED - 50 -> 1
				continue;
			}
			break;
		}
	}
	return	1;
}
int CHldCtlHwBuf::WaitFullness_AtscMhCapBuf__HasEndlessWhile(void)
{
	if (__Sta_->IsModTyp_AtscMH())
	{
		while (1)
		{
			if ( __Sta_->ReqedNewAction_User() ) 
				return 0;
	
			RdFpgaCapPlay_BufLevel(200);
			if ( IsFpgaCapBuf_Underrun_toGivenLevel() )
			{
				Sleep(50);		//FIXED - 50 -> 1
				continue;
			}
			break;
		}
	}
	return	1;
}
int CHldCtlHwBuf::WaitFullness_IsdbSCapBuf__HasEndlessWhile(void)	//2011/8/5 ISDB-S ASI
{
	if (__Sta_->IsState_IsdbSLoopThru())
	{
		while (1)
		{
			if ( __Sta_->ReqedNewAction_User() ) 
				return 0;

			RdFpgaCapPlay_BufLevel(200);
			if ( IsFpgaCapBuf_Underrun_toGivenLevel() )
			{
				Sleep(50);		//FIXED - 50 -> 1
				continue;
			}
			break;
		}
	}
	return	1;
}
int	CHldCtlHwBuf::WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(int _pos)
{
	while (1)
	{
		if ( __Sta_->ReqedNewAction_User() ) 
			return 0;
	
		RdFpgaCapPlay_BufLevel(200);
		if (IsFpgaPlayBuf_Overrun_toMaxLevel())
		{
			//FIXED - 50 -> 1
			Sleep(50);
			//printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			continue;
		}
		break;
	}
	return	1;
}

int	CHldCtlHwBuf::WaitConsumePlayBuf_toMaxLevel_DvbT2(int _pos)
{
	while (1)
	{
		if ( __Sta_->ReqedNewAction_User() ) 
			return 0;
	
		RdFpgaCapPlay_BufLevel(200);
		if (IsFpgaPlayBuf_Overrun_toMaxLevel_3M())
		{
			//FIXED - 50 -> 1
			Sleep(50);
			//printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			continue;
		}
		break;
	}
	return	1;
}

//////////////////////////////////////////////////////////////////////////////////////
void CHldCtlHwBuf::SetHwFifoCntl_(int _play_rec, int _size)
{
	if (__Sta_->IsAttachedTvbTyp_Usb())	//	why???
	{
		TSPL_SET_FIFO_CONTROL(_play_rec, _size);
	}
}
void CHldCtlHwBuf::SetHwDmaDiection_LoopThru(void)
{
	__CtlLog_->HldPrint_CntWait("hld-internal processing . SetHwDmaDiection_LoopThru", 40);
	if (__Sta_->IsAttachedTvbTyp_CntlBitDiffDmaHostDirection())
	{
		TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, 2);
	}
	else
	{
		//2011/8/5 ISDB-S ASI
		if (__Sta_->IsAttachedTvb590S_CntlBitDiffDmaHostDirection())
			TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, (1<<4));
		else
			TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, (1<<3));
	}
}
void CHldCtlHwBuf::SetHwDmaDiection_Play(void)
{
	__CtlLog_->HldPrint_CntWait("hld-internal processing . SetHwDmaDiection_Play", 41);
	if (__Sta_->IsAttachedTvbTyp_CntlBitDiffDmaHostDirection())
	{
		TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, 1);
	}
	else
	{
		//2011/8/5 ISDB-S ASI
		if(__Sta_->IsAttachedTvb590S_CntlBitDiffDmaHostDirection())
			TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, (1<<3));
		else
			TSPL_SET_DMA_DIRECTION(__Sta_->TL_gCurrentModType, (1<<2));
	}
}
//////////////////////////////////////////////////////////////////////////////////////
void	CHldCtlHwBuf::ChkLockAsiInput_Reg0x600042(void)
{
	ULONG	reg;
	if (reg_rd_interval++ > 50)
	{
		reg_rd_interval = 0;
		reg = TSPL_READ_CONTROL_REG(0, 0x600042);
		asi_in_is_locked = reg;	//	...;
	}
}
int CHldCtlHwBuf::IsAsiInputLocked(void)
{
	if(__Sta_->IsModTyp_IsdbT_13() || __Sta_->IsModTyp_IsdbS() || __Sta_->IsModTyp_AtscMH() || __Sta_->IsModTyp_DvbT2() || __Sta_->IsModTyp_DvbC2()) 
		return	asi_in_is_locked;
	else
		return (int)TSPL_READ_CONTROL_REG(0, 0x600042);
}

void	CHldCtlHwBuf::RdCntAsiInputTs_Reg0x600044(void)
{
	ULONG	reg;
	//if (reg_rd_interval_2++ > 50)
	{
		reg_rd_interval_2 = 0;
		reg = TSPL_READ_CONTROL_REG(0, 0x600044);
		rd_cnt_asi_in_ts = reg;	//	...;
	}
}
int CHldCtlHwBuf::BitrateOfAsiInCapedTs(void)
{
	if(__Sta_->IsModTyp_IsdbT_13() || __Sta_->IsModTyp_IsdbS() || __Sta_->IsModTyp_AtscMH() || __Sta_->IsModTyp_DvbT2() || __Sta_->IsModTyp_DvbC2()) 
		return	rd_cnt_asi_in_ts;
	else
		return (int)TSPL_READ_CONTROL_REG(0, 0x600044);
}

//////////////////////////////////////////////////////////////////////////////////////
int CHldCtlHwBuf::initialCntl_for_IsdbT13AtscMh_CaptureMode(void)
{
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_CAPTURE_MODE) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set SDCON MODE in MON");
	}

	if ( __Sta_->IsModTyp_IsdbT_13() )
	{
		if ( ApplyPlayRate_Calced_or_UserReqed(TS_BITRATE_ISDB_T_13, 0) == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set PLAY RATE in MON");
		}
	}
	else if ( __Sta_->IsModTyp_AtscMH() )
	{
		if ( ApplyPlayRate_Calced_or_UserReqed(TS_BITRATE_MH, 0) == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set PLAY RATE in MON");
		}
	}
	//2011/8/4 ISDB-S ASI
	else if ( __Sta_->IsModTyp_IsdbS() )
	{
		if ( ApplyPlayRate_Calced_or_UserReqed(TS_BITRATE_ISDBS, 0) == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set PLAY RATE in MON");
		}
	}
	if ( TSPL_RESET_SDCON() == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to reset SDCON in MON");
	}

	if ( __Sta_->IsModTyp_IsdbT_13() )
	{
		//TVB593
		TSPL_RESET_IP_CORE(__Sta_->TL_gCurrentModType, 0x00001FFF);	//	what is it?
		Sleep(1);
		TSPL_RESET_IP_CORE(__Sta_->TL_gCurrentModType, 0x000003FF);	//	what is it?
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldCtlHwBuf::SetHwOptAndPrepareMonitor_OnMonStart__HasEndlessWhile(void)
{
	if ( TSPL_SET_SDRAM_BANK_INFO(__Sta_->iHW_BANK_NUMBER, __Sta_->iHW_BANK_OFFSET) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set BANK INFOMATION in START MON");
	}
	if ( TSPL_SET_TSIO_DIRECTION(TSPL_GET_TSIO_DIRECTION()) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set TSIO DIRECTION in START MON");
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_LOOP_THRU_MODE) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set SDCON MODE 1");
	}
	else
	{
		if ( __Sta_->IsAttachedTvbTyp_AllCase() )
		{
			;
		}
		else
		{
			while( (TSPL_GET_CUR_BANK_GROUP() == 0) && !__Sta_->ReqedNewAction_User() ) 
			{	
				Sleep(10);
			}
		}
	}
	if ( TSPL_RESET_SDCON() == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to reset SDCON in START MON");
	}
	if ( __Sta_->IsAttachedTvbTyp_Usb() )
	{
		;
	}
	else
	{
		TSPL_WRITE_CONTROL_REG(1, 42, 4);	//	???
	}
	if ( __Sta_->IsModTyp_IsdbT_13() )	// ISDB-T 13SEG (=10) ONLY. Modulator block reset.
	{
		TSPL_RESET_IP_CORE(__Sta_->TL_gCurrentModType, 0x00001FFF);
	}

	return	0;
}
int	CHldCtlHwBuf::SetHwOptAndPrepareCapture_OnRecStart__HasEndlessWhile(void)
{
	int	nBankBlockSize = (SdramSubBankSize());

	if ( TSPL_SET_SDRAM_BANK_INFO(__Sta_->iHW_BANK_NUMBER, __Sta_->iHW_BANK_OFFSET) == -1 )		//	set bank size. user defined
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set BANK INFOMATION in START REC");
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_CAPTURE_MODE) == -1 )	//	
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set SDCON MODE 1");
	}
	else
	{
		/* TVB390, TVB380 ONLY. Wait to fill the on board buffer. */ //TVB593
		if ( __Sta_->IsAttachedTvbTyp_AllCase() )
		{
			;
		}
		else
		{
			while ((TSPL_GET_CUR_BANK_GROUP() == TL_nIdCurBank) && !__Sta_->ReqedNewAction_User())
			{	
				Sleep(10);
			}
		}
	}
	
	if ( __Sta_->IsAttachedTvbTyp_AllCase() )
	{
		if ( TSPL_RESET_SDCON() == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to reset SDCON in START REC");
			__Sta_->SetMainTask_LoopState_(TH_NONE);
		}
		SetHwFifoCntl_(1, nBankBlockSize);	// CAPTURE MODE
	}

	return	0;
}
int	CHldCtlHwBuf::SetHwOptAndPrepareHwToPlay_OnPlayStart__HasEndlessWhile(int _nBankBlockSize, long _p_rate, long _clk_src)
{

	if ( __Sta_->IsAttachedTvbTyp_AllCase() )
	{
		;
	}
	else
	{
		if ( TSPL_RESET_SDCON() == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to reset SDCON in START PLAY");
			__Sta_->SetMainTask_LoopState_(TH_NONE);
		}
	}
	if ( TSPL_SET_SDRAM_BANK_INFO( __Sta_->iHW_BANK_NUMBER, __Sta_->iHW_BANK_OFFSET) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set BANK INFOMATION in START PLAY");
	}
	if ( ApplyPlayRate_Calced_or_UserReqed(_p_rate, _clk_src) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set PLAY RATE in START PLAY");
	}
	if ( TSPL_SET_TSIO_DIRECTION(TSPL_GET_TSIO_DIRECTION()) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set TSIO DIRECTION in START PLAY");
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to set SDCON MODE 0");
	}
	else
	{
		if ( __Sta_->IsAttachedTvbTyp_AllCase() )
		{
			;
		}
		else
		{
			/* V7/DTMB - "while" should be skipped. */
			while( (TSPL_GET_CUR_BANK_GROUP() == 0) && !__Sta_->ReqedNewAction_User() ) 
			{	
				Sleep(10);
			}
		}
	}

	if (SetDmaPtr_HostSide_Lldallocated() == 0)
	{
		return 0;
	}

	if (  __Sta_->IsAttachedTvbTyp_SupportIsdbTLoopThru() && __Sta_->IsModTyp_IsdbT_13() )
	{
		SetHwDmaDiection_Play();
	}
	if ( __Sta_->IsModTyp_AtscMH() )
	{
		SetHwDmaDiection_Play();
	}
	//2011/8/5 ISDB-S ASI
	if (  __Sta_->IsAttachedTvbTyp_SupportIsdbSLoopThru() && __Sta_->IsModTyp_IsdbS() )
	{
		SetHwDmaDiection_Play();
	}
	//2013/4/2
	if ( __Sta_->IsAttachedTvbTyp_SupportDvbT2LoopThru() && __Sta_->IsModTyp_DvbT2())
	{
		SetHwDmaDiection_Play();
	}
	if ( __Sta_->IsAttachedTvbTyp_SupportDvbC2LoopThru() && __Sta_->IsModTyp_DvbC2())
	{
		SetHwDmaDiection_Play();
	}

	if ( __Sta_->IsAttachedTvbTyp_AllCase() )
	{
		if ( TSPL_RESET_SDCON() == -1 )
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to reset SDCON in START PLAY");
			__Sta_->SetMainTask_LoopState_(TH_NONE);
		}

		if ( __Sta_->IsAttachedTvbTyp_Usb() )
		{
			TSPL_SET_FIFO_CONTROL(0, _nBankBlockSize);	// plaback direction
			memset((void*)TL_pdwDMABuffer, 0x00, SUB_BANK_MAX_BYTE_SIZE);
		}

#if 0
			for (i = 0; (i < __Sta_->iHW_BANK_NUMBER+1) && (__Sta_->IsTaskState_ContPlay()) && !__Sta_->ReqedNewAction_User(); i++)
			{
				if (  __Sta_->IsAttachedTvbTyp_SupportIsdbTLoopThru() && __Sta_->IsModTyp_IsdbT_13() )	//	loopthrough isdbt-13
				{
					if (WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
					{
						return 0;
					}
				}
				if ( __Sta_->IsModTyp_AtscMH() )	//	loopthrough atsc-mh
				{
					if (WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
					{
						return 0;
					}
				}
				//2011/8/5 ISDB-S ASI
				if (  __Sta_->IsAttachedTvbTyp_SupportIsdbSLoopThru() && __Sta_->IsModTyp_IsdbS() )	//	loopthrough isdbs
				{
					if (WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
					{
						return 0;
					}
				}

				StartDmaTransfer_Play(_nBankBlockSize);

				TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
			}
			TL_nIdCurBank = TSPL_GET_CUR_BANK_GROUP();
			TL_dwAddrDestBoardSDRAM = (TL_nIdCurBank == 0? 0 : BANK_SIZE_4);
#endif
	}

	return	1;
}

int	CHldCtlHwBuf::WaitDmaTrans__HasEndlessWhile(void)
{
	int	timeout_ = 0;
	while (!__Sta_->ReqedNewAction_User())
	{
		if (TSPL_GET_DMA_STATUS())
		{
			break;	//	status completion
		}
		Sleep(1);
		timeout_++;
		if (timeout_ > 5000)	//	5-sec
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. dma transfer timeout...1");
			return	-1;
		}
	}
	return	0;
}
int	CHldCtlHwBuf::WaitDmaTrans_CaptureDirection__HasEndlessWhile(void)
{
	int	timeout_ = 0;
	while ( __Sta_->IsTaskState_ContRec() && !__Sta_->ReqedNewAction_User() )
	{
		Sleep(10);
		if (TSPL_GET_DMA_STATUS())
		{
			break;	//	status completion
		}
		timeout_++;
		if (timeout_ > 800)	//	8-sec. capture direction
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. dma transfer timeout...2");
			return	-1;
		}
	}
	return	0;
}
int	CHldCtlHwBuf::WaitDmaTrans_PlayDirection__HasEndlessWhile(void)
{
	int	timeout_ = 0;
	while ( __Sta_->IsTaskState_ContPlay() && !__Sta_->ReqedNewAction_User() )
	{
		Sleep(10);
		if (TSPL_GET_DMA_STATUS())
		{
			break;	//	status completion
		}
		timeout_++;
		if (timeout_ > 500)	//	5-sec
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. dma transfer timeout...3");
			return	-1;
		}
	}
	return	0;
}
int	CHldCtlHwBuf::WaitDmaTrans_CaptureDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile(void)
{
	int	timeout_ = 0;
	while ( __Sta_->IsCapMod_IsdbT13_or_AtscMH() || __Sta_->IsCapMod_IsdbS() || __Sta_->IsCapMod_DvbT2() || __Sta_->IsModTyp_DvbC2())	//2011/8/8 ISDB-S ASI
	{
		Sleep(10);
		if (TSPL_GET_DMA_STATUS())
		{
			break;	//	status completion
		}
		timeout_++;
		if (timeout_ > 800)	//	5-sec. capture direction
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. dma transfer timeout...4");
			return	-1;
		}
	}
	return	0;
}
int	CHldCtlHwBuf::WaitDmaTrans_PlayDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile(void)
{
	int	timeout_ = 0;
	while ( __Sta_->IsPlayMod_IsdbT13_or_AtscMH() || __Sta_->IsPlayMod_IsdbS() || __Sta_->IsModTyp_DvbT2() || __Sta_->IsModTyp_DvbC2()) //2011/8/5 ISDB-S ASI
	{
		Sleep(10);
		if (TSPL_GET_DMA_STATUS())
		{
			break;	//	status completion
		}
		timeout_++;
		if (timeout_ > 500)	//	5-sec
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. dma transfer timeout...5");
			return	-1;
		}
	}
	return	0;
}
void	CHldCtlHwBuf::StartDmaTransfer(int _bnk_size)
{
	__CtlLog_->HldPrint_WrHw(__Sta_->TL_gCurrentModType, 3);
	__CtlLog_->HldPrint_Tmr(0, "Wr-Dta-Dma..Step---1", _bnk_size);
#if defined(WIN32)				
	TSPL_WRITE_BLOCK( NULL, (unsigned long )_bnk_size, (DWORD *)TL_dwAddrDestBoardSDRAM);
	WaitDmaTrans__HasEndlessWhile();
#else
	LockFileMutex();
	TSPL_WRITE_BLOCK( (DWORD *)TL_pdwDMABuffer, (unsigned long)_bnk_size, (DWORD *)TL_dwAddrDestBoardSDRAM);
	WaitDmaTrans__HasEndlessWhile();
	UnlockFileMutex();
#endif
}
void	CHldCtlHwBuf::StartDmaTransfer_Play(int _bnk_size)
{
	__CtlLog_->HldPrint_WrHw(__Sta_->TL_gCurrentModType, 1);
	__CtlLog_->HldPrint_Tmr(1, "Wr-Dta-Dma..Step---1");
#if defined(WIN32)				
	TSPL_WRITE_BLOCK( NULL, (unsigned long )_bnk_size, (DWORD *)TL_dwAddrDestBoardSDRAM);
	WaitDmaTrans_PlayDirection__HasEndlessWhile();
#else
	LockFileMutex();
	TSPL_WRITE_BLOCK( (DWORD *)TL_pdwDMABuffer, (unsigned long )_bnk_size, (DWORD *)TL_dwAddrDestBoardSDRAM);
	WaitDmaTrans_PlayDirection__HasEndlessWhile();
	UnlockFileMutex();
#endif
	__CtlLog_->HldPrint_Tmr(1, "Wr-Dta-Dma..Step---2");
}
void	CHldCtlHwBuf::StartDmaTransfer_Play_Loopthru(int _bnk_size)
{
	__CtlLog_->HldPrint_RdHw_LoopThruMode(__Sta_->TL_gCurrentModType);
	__CtlLog_->HldPrint_Tmr(2, "Wr-Dta-Dma..Step---1");
#if defined(WIN32)
	TSPL_WRITE_BLOCK( NULL, (unsigned long )_bnk_size, (DWORD *)0);
	WaitDmaTrans_PlayDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile();
#else
	LockFileMutex();
	TSPL_WRITE_BLOCK( (DWORD *)TL_pdwDMABuffer, (unsigned long )_bnk_size, (DWORD *)0);
	WaitDmaTrans_PlayDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile();
	UnlockFileMutex();
#endif
	__CtlLog_->HldPrint_Tmr(2, "Wr-Dta-Dma..Step---2");
}
void CHldCtlHwBuf::StartDmaTransfer_Play_Any(void)
{
	int		nBankBlockSize = (SdramSubBankSize());

	__CtlLog_->HldPrint_WrHw(__Sta_->TL_gCurrentModType, 4);
	__CtlLog_->HldPrint_Tmr(3, "Wr-Dta-Dma..Step---1");
#if defined(WIN32)		
	TSPL_WRITE_BLOCK( NULL, (unsigned long )nBankBlockSize, (DWORD *)TL_dwAddrDestBoardSDRAM);
	WaitDmaTrans_PlayDirection__HasEndlessWhile();
#else
//	refer to Fill_Playback_DmaBuffer()
	LockFileMutex();
	WaitDmaTrans_PlayDirection__HasEndlessWhile();
	UnlockFileMutex();
#endif
	__CtlLog_->HldPrint_Tmr(3, "Wr-Dta-Dma..Step---2");
}
void CHldCtlHwBuf::StartDmaTransfer_Play_SpecificBuf(DWORD *_buf, int _bnk_size)
{
#if defined(WIN32)		
#else
	LockFileMutex();
	__CtlLog_->HldPrint_WrHw(__Sta_->TL_gCurrentModType, 5);
	TSPL_WRITE_BLOCK(_buf, _bnk_size, 0);
	WaitDmaTrans__HasEndlessWhile();
	UnlockFileMutex();
#endif
}


#if defined(WIN32)
#else
void	CHldCtlHwBuf::HwBuf_CreateMutex(pthread_mutex_t *_hMutex)
{
	Hw_hMutex = _hMutex;
}

void	CHldCtlHwBuf::LockFileMutex(void)
{
	pthread_mutex_lock(Hw_hMutex);
}
void	CHldCtlHwBuf::UnlockFileMutex(void)
{
	pthread_mutex_unlock(Hw_hMutex);
}
#endif



void	*CHldCtlHwBuf::StartDmaTransfer_Capture(int _bnk_size)
{
	void	*_dma_p = NULL;

	_dma_p = TSPL_READ_BLOCK(SdramSubBankSize());
	if (_dma_p != NULL)
	{
		WaitDmaTrans_CaptureDirection__HasEndlessWhile();
	}
	TL_pdwDMABuffer = (DWORD*)_dma_p;
	return	_dma_p;
}
void	*CHldCtlHwBuf::StartDmaTransfer_Capture_Loopthru(int _bnk_size)
{
	void	*_dma_p = NULL;
	int		nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;
	if(__Sta_->IsModTyp_DvbT2() || __Sta_->IsModTyp_DvbC2())
		nBankBlockSize = nBankBlockSize * 2;

	SetHwFifoCntl_(1, nBankBlockSize);	// CAPTURE MODE
	SetHwDmaDiection_LoopThru();

	_dma_p = TSPL_READ_BLOCK(nBankBlockSize);
	if (_dma_p != NULL)
	{
		WaitDmaTrans_CaptureDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile();
	}
	TL_pdwDMABuffer = (DWORD*)_dma_p;

	return	_dma_p;
}

void CHldCtlHwBuf::FillDmaSrcDta_PlayDirection(unsigned char *_src, int _bnk_size)
{
	memcpy((void *)TL_pdwDMABuffer, _src, _bnk_size);
}
void CHldCtlHwBuf::UpdDmaDestPos_PlayDirection(DWORD _base, DWORD _inc)
{
	TL_dwAddrDestBoardSDRAM = _base + _inc;
}


//////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------
//
//	check_buffer_status 
//
void	CHldCtlHwBuf::TL_CheckBufferStatus(int written_or_end, long _play_rate)
	//	its not cntl-func. just set(update) the cap-play status to indicate the current hld state. and app. use the state to know what is working.
{
	int	ind;
	int	starting_out_1st, complete_out_last;
	int	sub_bank_cnt_for_wait_time;
	double	play_msec_for_1_subbank;

	starting_out_1st = 0;
	complete_out_last = 0;
	
	if (written_or_end == 0)
	{
		play_buffer_status = BUFF_STS_BUFFERING;
		user_play_buffer_status = -1;
		wait_msec_bank_out = 0;
		for (ind = 0; ind < MAX_BANK_NUMBER; ind++)	{
			TL_ClearBuffer(ind);
		}
		return;
	}
	else if (written_or_end == 1)
	{
		buf_queue_config[0].stream_progressing = 1;
		buf_queue_config[0].stream_pos_last = 0;
		wait_msec_bank_out = 0;

		for (ind = 0; ind < MAX_BANK_NUMBER; ind++)
		{
			if (buf_queue_config[ind].stream_progressing == 1)
			{
				buf_queue_config[ind].stream_pos_1st++;
				buf_queue_config[ind].stream_pos_last++;

				if (buf_queue_config[ind].stream_pos_last >= ((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1))
				{
					complete_out_last = 1;
					TL_ClearBuffer(ind);
				}
				else if (buf_queue_config[ind].stream_pos_1st >= ((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1))
				{
					starting_out_1st = 1;
				}
			}
		}
	}
	else if (written_or_end == 2)
	{
		for (ind = MAX_BANK_NUMBER - 1; ind > 0; ind--)
		{
			if (buf_queue_config[ind - 1].stream_progressing == 1)
			{
				buf_queue_config[ind].stream_pos_1st =
					buf_queue_config[ind - 1].stream_pos_1st;
				buf_queue_config[ind].stream_pos_last =
					buf_queue_config[ind - 1].stream_pos_last;
				buf_queue_config[ind].stream_progressing = 1;

				if (buf_queue_config[ind].stream_pos_1st >= ((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1)) {
					starting_out_1st = 1;
				}
			}
		}
		buf_queue_config[0].stream_pos_1st = ((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1);
	}
	else if (written_or_end == 3)
	{
		wait_msec_bank_out += 10;	// in msec
		play_msec_for_1_subbank =	// in msec
			((double)(SdramSubBankSize()) * 8 * 1000)/(double)_play_rate;
		if (play_msec_for_1_subbank >= 1)
			sub_bank_cnt_for_wait_time = wait_msec_bank_out/(int)play_msec_for_1_subbank;
		else
			sub_bank_cnt_for_wait_time = 0;
		
		for (ind = MAX_BANK_NUMBER - 1; ind >= 0; ind--)
		{
			if (buf_queue_config[ind].stream_progressing == 1)
			{
				if ((ind != 0) &&
					(sub_bank_cnt_for_wait_time > 
					(((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1) - buf_queue_config[ind].stream_pos_last)))
				{
					complete_out_last = 1;
					TL_ClearBuffer(ind);

					break;
				}
				else if (buf_queue_config[ind].stream_pos_1st >= ((__Sta_->iHW_BANK_NUMBER+1) * 2 - 1))
				{
					starting_out_1st = 1;
					break;
				}
			}
		}
	}

	if (user_play_buffer_status == play_buffer_status)
	{
		if (complete_out_last == 1) {
			play_buffer_status = BUFF_STS_COMPLETED_A_STREAM_OUT_NOW;
		} else if (starting_out_1st == 1) {
			play_buffer_status = BUFF_STS_PLAY_STREAM_OUT;
		} else {
			play_buffer_status = BUFF_STS_BUFFERING;
		}
	}
}


int CHldCtlHwBuf::MapTvbBdDestAddr_fromBnkId(void)
{
	if (TL_nSubBankIdx == 0)
	{
		TL_dwAddrDestBoardSDRAM = (TL_nIdCurBank == 0? 0 : BANK_SIZE_4);
		//	we configured the sdram max size as 2MByte. therefore the dest pointer should 2M for secondary sdram.
	}
	return	1;
}
int CHldCtlHwBuf::IncBankAddrHwDest_Sdram(long _pr)
{
	TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
	TL_CheckBufferStatus(1, _pr);
	if (++TL_nSubBankIdx == (__Sta_->iHW_BANK_NUMBER+1))
	{
		TL_nIdCurBank = (TL_nIdCurBank + 1) & 0x01;
		TL_nSubBankIdx = 0;
		if ( __Sta_->IsAttachedTvbTyp_AllCase() )
		{
			if(__Sta_->Start_Output_flag == 0 && __Sta_->IsModTyp_Tdmb())
			{
				TVB380_SET_MODULATOR_OUTPUT(__Sta_->TL_gCurrentModType, 0);
				__Sta_->Start_Output_flag = 1;
			}
		}
		else
		{
			while( (TSPL_GET_CUR_BANK_GROUP() == TL_nIdCurBank) && !__Sta_->ReqedNewAction_User() ) 
			{	
				Sleep(10);
				TL_CheckBufferStatus(3, _pr);
			}
		}
	}
	return	1;
}
int CHldCtlHwBuf::IncBankAddrHwDest_Sdram_2(void)
{
	TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
	if (++TL_nSubBankIdx == (__Sta_->iHW_BANK_NUMBER+1))
	{
		TL_nIdCurBank = (TL_nIdCurBank + 1) & 0x01;
		TL_nSubBankIdx = 0;
	}
	return	1;
}

int CHldCtlHwBuf::SetSdram_BankInfo(int nBankCount, int nBankOffset)
{
	if (__Sta_->IsState_IsdbT13LoopThru() && (__Sta_->Flag_PlayLoopThru() || __Sta_->Flag_CapLoopThru()))
	{
		return 0;
	}

	if ( __Sta_->IsModTyp_AtscMH() && (__Sta_->Flag_PlayLoopThru() || __Sta_->Flag_CapLoopThru()) )
	{
		return 0;
	}
	//2011/8/5 ISDB-S ASI
	if (__Sta_->IsState_IsdbSLoopThru() && (__Sta_->Flag_PlayLoopThru() || __Sta_->Flag_CapLoopThru()))
	{
		return 0;
	}
	//2012/7/11 DVB-T2 ASI
	if (__Sta_->IsState_DvbT2LoopThru() && (__Sta_->Flag_PlayLoopThru() || __Sta_->Flag_CapLoopThru()))
	{
		return 0;
	}
	//2012/7/24 DVB-C2 ASI
	if (__Sta_->IsState_DvbC2LoopThru() && (__Sta_->Flag_PlayLoopThru() || __Sta_->Flag_CapLoopThru()))
	{
		return 0;
	}

	__Sta_->iHW_BANK_NUMBER = nBankCount;
	__Sta_->iHW_BANK_OFFSET = nBankOffset;

	if ( TSPL_SET_SDRAM_BANK_INFO(nBankCount, nBankOffset) == -1 )
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL: can not set SDRAM BANK info, unstable board status");
	}
	if (TSPL_SET_SDRAM_BANK_CONFIG(nBankCount) == -1)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL: can not set SDRAM BANK mode, unstable board status");
	}
	/* V7/DTMB - "nBankOffset = 0x400;" should be applied. */
	if (TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(nBankOffset) == -1)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL: can not set SDRAM BANK offset, unstable board status");
	}

	return 0;
}
int CHldCtlHwBuf::SetSdram_BankOffsey_Conf(int nBankConfig)
{
	__Sta_->iHW_BANK_OFFSET = nBankConfig;
//	__CtlLog_->HldPrint("Hld-Bd-Ctl. bank offset= %d\n", nBankConfig);
	if (TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(nBankConfig) == -1)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL: can not set SDRAM BANK offset, unstable board status");
	}
	return 0;
}
int CHldCtlHwBuf::SetSdram_Bank_Conf(int nBankConfig)
{
	__Sta_->iHW_BANK_NUMBER = nBankConfig;
//	__CtlLog_->HldPrint("Hld-Bd-Ctl. bank count= %d\n", nBankConfig);
	if (TSPL_SET_SDRAM_BANK_CONFIG(nBankConfig) == -1)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL: can not set SDRAM BANK mode, unstable board status");
	}
	return 0;
}
int CHldCtlHwBuf::ApplyPlayRate_Calced_or_UserReqed(long _play_freq_in_herz, long _nOutputClockSource)
{
	__CtlLog_->HldPrint_2("Hld-Bd-Ctl. playfreq and out-clk-src ", _play_freq_in_herz, _nOutputClockSource);
	return	TSPL_SET_PLAY_RATE(_play_freq_in_herz, _nOutputClockSource);
}
int	CHldCtlHwBuf::SdramSubBankSize(void)
{
#if 1 
	if ( __Sta_->IsModTyp_DvbT2())
		return (1024 << 10);
	else
		return	(__Sta_->iHW_BANK_OFFSET << 10);
#else
	return	(__Sta_->iHW_BANK_OFFSET << 10);
#endif
}



