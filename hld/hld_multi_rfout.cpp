
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

#include	"hld_multi_rfout.h"
#include	"LLDWrapper.h"

//////////////////////////////////////////////////////////////////////////////////////
CHldMultiRfOut::CHldMultiRfOut(int _my_id, void *_hld)
{
	my_hld_id = _my_id;
	my_hld = _hld;

#if defined(WIN32)
	_multi_ts_mutex = INVALID_HANDLE_VALUE;
#endif

}

CHldMultiRfOut::~CHldMultiRfOut()
{
}

void	CHldMultiRfOut::SetCommonMethod_9(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void	*CHldMultiRfOut::MultiTsMutex(void)
{
#if defined(WIN32)
	return	(void *)_multi_ts_mutex;
#else
	return	(void *)&_multi_ts_mutex;
#endif
}
void	CHldMultiRfOut::CreateMultiTsMutexForReal(void)
{
	if (!__Sta__->IsAttachedTvbTyp_SupportMultiTs())
	{
		return;
	}
	if (__Sta__->IsAttachedTvbTyp__Virtual())
	{
		return;
	}
#if defined(WIN32)
	if ( _multi_ts_mutex == INVALID_HANDLE_VALUE )	//	donot remove this line.
	{
		_multi_ts_mutex = CreateMutex(NULL, FALSE, NULL);
	}
#else
	pthread_mutexattr_init(&_multi_ts_mutex_attr);
	pthread_mutexattr_settype(&_multi_ts_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&_multi_ts_mutex, &_multi_ts_mutex_attr);
#endif

}
void	CHldMultiRfOut::DupMultiTsMutexForVirtual(void *_mutx)
{
	if (!__Sta__->IsAttachedTvbTyp_SupportMultiTs())
	{
		return;
	}
	if (!__Sta__->IsAttachedTvbTyp__Virtual())
	{
		return;
	}
#if defined(WIN32)
	_multi_ts_mutex = (HANDLE)_mutx;
#else
	memcpy(&_multi_ts_mutex, _mutx, sizeof(pthread_mutex_t));
#endif
}
void	CHldMultiRfOut::DestroyMultiTsMutex(void)
{
	if (!__Sta__->IsAttachedTvbTyp_SupportMultiTs())
	{
		return;
	}
	if (__Sta__->IsAttachedTvbTyp__Virtual())
	{
		return;
	}
#if defined(WIN32)	
	if ( _multi_ts_mutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(_multi_ts_mutex);
		CloseHandle(_multi_ts_mutex);
		_multi_ts_mutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&_multi_ts_mutex_attr);
	pthread_mutex_destroy(&_multi_ts_mutex);
#endif
}
void	CHldMultiRfOut::LockMultiTsMutex(void)
{
#if defined(WIN32)
	if ( _multi_ts_mutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(_multi_ts_mutex, INFINITE);
	}
#else
	pthread_mutex_lock(&_multi_ts_mutex);
#endif
}
void	CHldMultiRfOut::UnlockMultiTsMutex(void)
{
#if defined(WIN32)
	if ( _multi_ts_mutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(_multi_ts_mutex);
	}
#else
	pthread_mutex_unlock(&_multi_ts_mutex);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
int		CHldMultiRfOut::MultiStreamStartMon_594(void)
{
	CHld	*_hld_1, *_hld_2, *_hld_3;

	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return 0;
	}
	_hld_1 = (CHld *)__Sta__->multits_1_hld;
	_hld_2 = (CHld *)__Sta__->multits_2_hld;
	_hld_3 = (CHld *)__Sta__->multits_3_hld;

	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_ContPlay()))		return 1;
	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_StartRec()))		return 1;
	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_ContRec()))		return 1;

	if ((_hld_2 != NULL) && (_hld_2->_SysSta->IsTaskState_ContPlay()))		return 1;
	if ((_hld_3 != NULL) && (_hld_3->_SysSta->IsTaskState_ContPlay()))		return 1;

	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500101, 0x00000000);	//	TSP_MEM_ADDR_OPMODE_CNTL
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0xFFFFFFFE);	//	TSP_MEM_ADDR_RESET_CNTL
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0x00000000);	//	TSP_MEM_ADDR_RESET_CNTL
	
	return 1;
}
void	CHldMultiRfOut::MultiStreamStartRec_594(void)
{
	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return;
	}
	if (__Sta__->IsAttachedTvbTyp_594_Virtual())	//	support one ts only
	{
		return;
	}
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500101, 0x00000002);	//	TSP_MEM_ADDR_OPMODE_CNTL
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0xFFFFFFFE);	//	TSP_MEM_ADDR_RESET_CNTL
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0x00000000);	//	TSP_MEM_ADDR_RESET_CNTL
}
int	CHldMultiRfOut::MultiStreamContRec_594(void)
{
	CHld	*_hld;
	DWORD	dwRet;
	int	nBankBlockSize = __FIf__->SdramSubBankSize();

	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return 0;
	}

	_hld = (CHld *)my_hld;
	do
	{
		if (__FIf__->IsInvalidFileHandle())
		{
			continue;
		}

		dwRet = __FIf__->TSPL_READ_CONTROL_REG(0, 0x600086);	//	_ADR_594_BUF_STATUS_CAPTURE_
		if ( (dwRet>>22 & 0x01) || (dwRet>>23 & 0x01) )
		{
		}
		else
		{
		}

		if ( nBankBlockSize > (int)((dwRet & 0x001FFFFF)*4) )
		{
			Sleep(100);
			continue;
		}

		__FIf__->TL_pdwDMABuffer = (DWORD*)__FIf__->TSPL_READ_BLOCK(nBankBlockSize);
		if (__FIf__->TL_pdwDMABuffer != NULL)
		{
			while ( __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User())
			{
				if ( __FIf__->TSPL_GET_DMA_STATUS() )
					break;
			}
			_hld->TL_ProcessCaptureSubBlock();
		}
	} while ( __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User());

	return 1;
}
void	CHldMultiRfOut::MultiStreamStartPlay_594(void)
{
	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return;
	}

	LockMultiTsMutex();

	__Sta__->iHW_BANK_OFFSET = 256;	//	256Kbyte

	__FIf__->TSPL_SET_PLAY_RATE_594(
			__FIf__->GetPlayParam_PlayRate(),	//	make sure. this val is same as TL_CurrentBitrate.
			__Sta__->my_stream_id_of_virtual_bd,
			__FIf__->GetPlayParam_OutClkSrc());

	__FIf__->TL_pdwDMABuffer = (DWORD *) __FIf__->TSPL_GET_DMA_ADDR();

	UnlockMultiTsMutex();

	__FIf__->TSPL_WRITE_CONTROL_REG(1, 0x1B, 0x100F767C);

	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500101, 0x00000001);	//	TSP_MEM_ADDR_OPMODE_CNTL
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0xFFFFFFE);
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500120, 0x00000000);
}
int	CHldMultiRfOut::MultiStreamCpDmaBuf_594(char *_dst, char *_src, int _size)
{
	if (!__Sta__->IsAttachedTvbTyp_594())
{
		return	0;
	}
	memcpy(_dst, _src, _size);
	return	_size;
}
int	CHldMultiRfOut::MultiStreamContPlay_594(void)
{
	DWORD	dwRet;
	int	nBankBlockSize = __FIf__->SdramSubBankSize();

	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return	0;
	}
	LockMultiTsMutex();

	MultiStreamCpDmaBuf_594((char *)__FIf__->TL_pdwDMABuffer, (char *)__FIf__->TL_pbBuffer, nBankBlockSize);

_again:
	dwRet = __FIf__->TSPL_READ_CONTROL_REG(0, 0x600081);	//	_ADR_594_BUF_STATUS_
	dwRet = __FIf__->TSPL_READ_CONTROL_REG(0, 0x600082 + __Sta__->my_stream_id_of_virtual_bd);	//	_ADR_594_BUF_STATUS_STREAM_
	if ( (dwRet>>22 & 0x01) || (dwRet>>23 & 0x01) )
	{
	}
	else
	{
	}
	if ((MAX_MULTI_STREAM_BUFFER_SIZE - nBankBlockSize) < (int)((dwRet & 0x0007FFFF)*4))	//	fullness. greater than 1.5 Mbyte
	{
		Sleep(10);	//	need flow control
		if (!__Sta__->IsTaskState_ContPlay() || __Sta__->ReqedNewAction_User())
		{
			UnlockMultiTsMutex();
			return	1;
		}
		goto	_again;
	}
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500100, __Sta__->my_stream_id_of_virtual_bd);	//	TSP_MEM_ADDR_STREAM_TAG_CNTL
	__FIf__->StartDmaTransfer_Play_Any();

	UnlockMultiTsMutex();
	return	1;
}
void	CHldMultiRfOut::MultiStreamSelTs_ofAsi310Out_594(int _ts_n)
{
	if (!__Sta__->IsAttachedTvbTyp_594())
	{
		return;
	}
	_ts_n &= 0xf;
	__FIf__->TSPL_WRITE_CONTROL_REG(0, 0x500110, _ts_n);
}

//////////////////////////////////////////////////////////////////////////////////////
int		CHldMultiRfOut::MultiStreamStartMon_593(void)
{
	CHld	*_hld_1, *_hld_2, *_hld_3, *_hld_4;

	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return 0;
	}

	if(__Sta__->IsAsior310_LoopThru_DtaPathDirection())
		return 0;

	_hld_1 = (CHld *)__Sta__->multits_1_hld;	//	real-slot
	_hld_2 = (CHld *)__Sta__->multits_2_hld;
	_hld_3 = (CHld *)__Sta__->multits_3_hld;
	_hld_4 = (CHld *)__Sta__->multits_4_hld;
	
	if(__Sta__->IsNullTP_Enabled() == 0)
	{
		__FIf__->TVB380_SET_MODULATOR_OUTPUT(__Sta__->m_nBoardId, 1);
	}

	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_ContPlay()))		return 1;
	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_StartRec()))		return 1;
	if ((_hld_1 != NULL) && (_hld_1->_SysSta->IsTaskState_ContRec()))		return 1;

	if ((_hld_2 != NULL) && (_hld_2->_SysSta->IsTaskState_ContPlay()))		return 1;
	if ((_hld_3 != NULL) && (_hld_3->_SysSta->IsTaskState_ContPlay()))		return 1;
	if ((_hld_4 != NULL) && (_hld_4->_SysSta->IsTaskState_ContPlay()))		return 1;

	__FIf__->TSPL_SelMultiModOperationMode_n(0);
//	__FIf__->TSPL_SelMultiModTsOutput_n(1);
//	__FIf__->TSPL_SelMultiModExtTsInput_n(1);	//	???
//	__FIf__->TSPL_SelMultiModOperationMode_n(1);
#if 0
	if(_hld_1 != NULL)
		_hld_1->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_1->_SysSta->m_nBoardId, 0);
	if(_hld_2 != NULL)
		_hld_2->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_2->_SysSta->m_nBoardId, 0);
	if(_hld_3 != NULL)
		_hld_3->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_3->_SysSta->m_nBoardId, 0);
	if(_hld_4 != NULL)
		_hld_4->_FIf->TVB380_SET_MODULATOR_OUTPUT(_hld_4->_SysSta->m_nBoardId, 0);
#endif
	return 1;
}
void	CHldMultiRfOut::MultiStreamStartRec_593(void)
{
	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return;
	}
	if (__Sta__->IsAttachedTvbTyp__Virtual())	//	support one ts only
	{
		return;
	}
//	__FIf__->TSPL_SelMultiModTsOutput_n(0);
	__FIf__->TSPL_SelMultiModOperationMode_n(2);
}
int	CHldMultiRfOut::MultiStreamContRec_593(void)
{
	CHld	*_hld;
	DWORD	dwRet;
	int	nBankBlockSize = __FIf__->SdramSubBankSize();

	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return 0;
	}

	_hld = (CHld *)my_hld;
	do
	{
		if (__FIf__->IsInvalidFileHandle())
		{
			continue;
		}
		dwRet = __FIf__->TSPL_GET_wr_cap_BUF_STS_MultiBd_n();
		if ( nBankBlockSize > (int)((dwRet & 0x001FFFFF)*4))	//	Buffer fullness in Word unit
		{
			Sleep(100);
			continue;
		}

		__FIf__->TL_pdwDMABuffer = (DWORD*)__FIf__->TSPL_READ_BLOCK(nBankBlockSize);
		if (__FIf__->TL_pdwDMABuffer != NULL)
		{
			while ( __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User())
			{
				if ( __FIf__->TSPL_GET_DMA_STATUS() )
					break;
			}
			_hld->TL_ProcessCaptureSubBlock();
		}
	} while ( __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User());

	return 1;
}
void	CHldMultiRfOut::MultiStreamStartPlay_593(void)
{
	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return;
	}

	LockMultiTsMutex();

	//__Sta__->iHW_BANK_OFFSET = 256;	//	256Kbyte, 0x40000

	__FIf__->TSPL_SET_PLAY_RATE(__FIf__->GetPlayParam_PlayRate(),
			__FIf__->GetPlayParam_OutClkSrc());

	__FIf__->TL_pdwDMABuffer = (DWORD *) __FIf__->TSPL_GET_DMA_ADDR();

	UnlockMultiTsMutex();

	__FIf__->TVB380_SET_MODULATOR_OUTPUT(__Sta__->m_nBoardId, 0);
	__FIf__->TSPL_SelMultiModTsOutput_n(0);
	__FIf__->TSPL_SelMultiModOperationMode_n(1);
}
int	CHldMultiRfOut::MultiStreamCpDmaBuf_593(char *_dst, char *_src, int _size)
{
	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return	0;
	}
	memcpy(_dst, _src, _size);
	return	_size;
}
int	CHldMultiRfOut::MultiStreamContPlay_593(void)
{
	DWORD	dwRet;
	int	nBankBlockSize = __FIf__->SdramSubBankSize();
	int	nBufferLevel;
	int	nMaxBufferSize;
//TEST
	//char str[100];

	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return	0;
	}
	__HLog__->HldPrint_Tmr(__Sta__->my_stream_id_of_virtual_bd, "Wr-Dta-Dma..Step---1");
_again:
	LockMultiTsMutex();

	MultiStreamCpDmaBuf_593((char *)__FIf__->TL_pdwDMABuffer, (char *)__FIf__->TL_pbBuffer, nBankBlockSize);


	//sprintf(str, "my_stream_id_of_virtual_bd = %d, nBankBlockSize = %d\n", __Sta__->my_stream_id_of_virtual_bd, nBankBlockSize);
	//OutputDebugString(str);
	dwRet = __FIf__->TSPL_GET_wr_BUF_STS_MultiBd_n(__Sta__->my_stream_id_of_virtual_bd);	//	Buffer fullness in Word unit
	//char tmp_str[256];
	//sprintf(tmp_str, "0x64000, WrBuf_ptr: 0x%X, RdBuf_Ptr: 0x%X, Fullness: 0x%X\n", ((dwRet >>28) & 0xF), ((dwRet >>24) & 0xF), ((dwRet & 0x7FFFF) * 8));
	//OutputDebugString(tmp_str);

#if 0
	int pWrBuf;
	int pRdBuf;
	pWrBuf = ((dwRet >>28) & 0xF);
	pRdBuf = ((dwRet >>24) & 0xF);

	if((pWrBuf - pRdBuf) < 0)
		nBufferLevel = 0x10 + (pWrBuf - pRdBuf);
	else
		nBufferLevel = pWrBuf - pRdBuf;
	nBufferLevel = nBufferLevel * 0x20000 * 2; 
	nMaxBufferSize = MAX_MULTI_STREAM_BUFFER_SIZE * 0.5;
	if(nMaxBufferSize < nBufferLevel)
#else

	if(__Sta__->IsAttachedTvbTyp_598())
	{
		nBufferLevel = (dwRet & 0x0007FFFF) * 8;
		nMaxBufferSize = MAX_MULTI_STREAM_BUFFER_SIZE * 1.5;
	}
	else
	{
		nBufferLevel = (dwRet & 0x0007FFFF)*4;
		nMaxBufferSize = MAX_MULTI_STREAM_BUFFER_SIZE;
	}
	if ((nMaxBufferSize - nBankBlockSize) < nBufferLevel)	//	fullness. greater than 1.5 Mbyte
#endif
	{
		UnlockMultiTsMutex();
		Sleep(10);	//	need flow control
		if (!__Sta__->IsTaskState_ContPlay() || __Sta__->ReqedNewAction_User())
		{
			UnlockMultiTsMutex();
			return	1;
		}

		//sprintf(str, "my_stream_id_of_virtual_bd = %d TRY AGAIN\n", __Sta__->my_stream_id_of_virtual_bd);
		//OutputDebugString(str);
		goto	_again;
	}
	__FIf__->TSPL_SEL_TS_TAG_VirtualBd_n(__Sta__->my_stream_id_of_virtual_bd);
	__FIf__->StartDmaTransfer_Play_Any();

	UnlockMultiTsMutex();
	__HLog__->HldPrint_Tmr(__Sta__->my_stream_id_of_virtual_bd, "Wr-Dta-Dma..Step---2");
	return	1;
}
void	CHldMultiRfOut::MultiStreamSelTs_ofAsi310Out_593(int _ts_n)
{
	if ((__Sta__->_CntAdditionalVsbRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(__Sta__->_CntAdditionalDvbTRfOut_593() == 0))	//2012/6/28 multi dvb-t
	{
		return;
	}
	_ts_n &= 0xf;
	__FIf__->TSPL_SelMultiModAsiOut_n(_ts_n);
	__FIf__->TSPL_SelMultiMod310Out_n(_ts_n);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
int		CHldMultiRfOut::MultiStreamStartMon(void)
{
	return (MultiStreamStartMon_594() | MultiStreamStartMon_593());
}
void	CHldMultiRfOut::MultiStreamStartRec(void)
{
	MultiStreamStartRec_594();
	MultiStreamStartRec_593();
}
int	CHldMultiRfOut::MultiStreamContRec(void)
{
	int	ret;
	ret = MultiStreamContRec_594();
	ret |= MultiStreamContRec_593();
	return	ret;
}
void	CHldMultiRfOut::MultiStreamStartPlay(void)
{
	MultiStreamStartPlay_594();
	MultiStreamStartPlay_593();
}
int	CHldMultiRfOut::MultiStreamCpDmaBuf(char *_dst, char *_src, int _size)
{
	int	ret;
	ret = MultiStreamCpDmaBuf_594(_dst, _src, _size);
	ret |= MultiStreamCpDmaBuf_593(_dst, _src, _size);
	return	ret;
}
int	CHldMultiRfOut::MultiStreamContPlay(void)
{
	int	ret;
	ret = MultiStreamContPlay_594();
	ret |= MultiStreamContPlay_593();
	return	ret;
}
void	CHldMultiRfOut::MultiStreamSelTs_ofAsi310Out(int _ts_n)
{
	MultiStreamSelTs_ofAsi310Out_594(_ts_n);
	MultiStreamSelTs_ofAsi310Out_593(_ts_n);
}

