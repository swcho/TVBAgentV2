
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>

#ifdef WIN32
#else
#include	<string.h>
#include	<stdlib.h>
#endif

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_fmtr_tdmb.h"


//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrTdmb::CHldFmtrTdmb(void)
{
	TL_szBufferNA = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);
	TL_nRecordNA = 0;
}

CHldFmtrTdmb::~CHldFmtrTdmb()
{
	if ( TL_szBufferNA ) free(TL_szBufferNA);
}
void	CHldFmtrTdmb::SetCommonMethod_7(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}
void	CHldFmtrTdmb::SetTdmbSyncBuf_Allocated(unsigned char	*_bufsync, unsigned char	*_bufrslt, unsigned char	*_buftmp, unsigned char	*_buf2ndplay)
{
	TL_bTdmbSync = _bufsync;
	TL_bTdmbRslt = _bufrslt;
	TL_bTdmbTmp = _buftmp;
	TL_bTdmbPlay_2 = _buf2ndplay;
}
void	CHldFmtrTdmb::SetTdmbSyncBuf_Pos(unsigned int	*_w, unsigned int	*_r, unsigned int	*_c)
{
	TL_nWritePos_TdmbPlay_2 = _w;
	TL_nReadPos_TdmbPlay_2 = _r;
	TL_nBufferCnt_TdmbPlay_2 = _c;
}

int	CHldFmtrTdmb::FillTemp2Buf_from_GivenBuf(unsigned char *_buf, int __siz)
{
	memcpy(TL_bTdmbSync + TL_TdmbWIndex, _buf, __siz);
	TL_TdmbWIndex += __siz;
	TL_TdmbBCount += __siz;
	if ( TL_TdmbWIndex >= MAX_BUFFER_BYTE_SIZE )
		TL_TdmbWIndex = 0;

	return	__siz;
}
int	CHldFmtrTdmb::FillRsltBuf_from_Temp2Buf_w_ChkBoundary(int __siz)
{
	int	i;

	if (TL_TdmbRIndex <= (MAX_BUFFER_BYTE_SIZE - __siz))
	{
		memcpy(TL_bTdmbRslt, TL_bTdmbSync + TL_TdmbRIndex, __siz);
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - TL_TdmbRIndex;
	
		memcpy(TL_bTdmbRslt, TL_bTdmbSync + TL_TdmbRIndex, i);
		memcpy(TL_bTdmbRslt + i, TL_bTdmbSync, __siz - i);
	} 

	return	__siz;
}
int	CHldFmtrTdmb::FillCalcSyncBuf_from_GivenBuf_w_ChkBoundary(unsigned char *_buf, unsigned int __siz)
{
	int	i;

	if (*TL_nWritePos_TdmbPlay_2 <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - __siz)))
	{
		memcpy(TL_bTdmbPlay_2 + *TL_nWritePos_TdmbPlay_2, _buf, __siz);
		*TL_nWritePos_TdmbPlay_2 += __siz;
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - *TL_nWritePos_TdmbPlay_2;

		memcpy(TL_bTdmbPlay_2 + *TL_nWritePos_TdmbPlay_2, _buf, i);
		memcpy(TL_bTdmbPlay_2, _buf + i, __siz - i);

		*TL_nWritePos_TdmbPlay_2 += __siz;
		if (*TL_nWritePos_TdmbPlay_2 >= MAX_BUFFER_BYTE_SIZE)
			*TL_nWritePos_TdmbPlay_2 -= MAX_BUFFER_BYTE_SIZE;
	}
	*TL_nBufferCnt_TdmbPlay_2 += __siz;

	return	__siz;
}

void	CHldFmtrTdmb::SetNewRdAlignPos_of_Temp2Buf(int _skip_bytes)
{
	TL_TdmbRIndex += _skip_bytes;
	if (TL_TdmbRIndex >= MAX_BUFFER_BYTE_SIZE)
		TL_TdmbRIndex -= MAX_BUFFER_BYTE_SIZE;
	TL_TdmbBCount -= _skip_bytes;
}

//////////////////////////////////////////////////////////////////////////////////////
void CHldFmtrTdmb::InitializeNAToNI(bool Init)
{
	if ( !__Sta__->IsModTyp_Tdmb() )
	{
		return;
	}
	
/* TDMB - ETI ext. */
	TL_IsNIorNA = CheckNIorNASync(__FIf__->PlayParm.AP_lst.szfn, 0x100000*3, &TL_NIorNASyncPos);
	
/* NA to NI and NI to CIF */
/* TDMB - ETI ext. */
	if (Init 	|| __FIf__->IsNaFile() || (__FIf__->IsEtiFile() && TL_IsNIorNA == 1) )
	{
		/* initialize the buffers */
		/* NA to NI */
		TL_InitConvertNItoNA();

		/* initialize the variables for NA converting */
		if ( __FIf__->TVB380_SET_MODULATOR_INIT_CIF() == -1 )
		{
			__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to initialize for CIF");
			__Sta__->SetMainTask_LoopState_(TH_NONE);
		}

		__FIf__->InitLoopRestamp_Param();
	}
}

//--------------------------------------------------------------------------
//	
//	Initialize Varialbes for NI to NA Conversion 
//
void CHldFmtrTdmb::TL_InitConvertNItoNA(void)
{
	__FIf__->InitPlayBufVariables();

	TL_TdmbWIndex = 0;	TL_TdmbRIndex = 0;	TL_TdmbBCount = 0;

	TL_nSFindSync = 1;
	TL_nNWIndex = 0;	TL_nNRIndex = 0;	TL_nNBCount = 0;	TL_nNFindSync = 1;	TL_nMBLKCount = 0;

}
//--------------------------------------------------------------------------
//	
//	Store G.704 Sync searched data to Sync buffer
//
void CHldFmtrTdmb::TL_FillTemp2SyncBuffer(void)
{
	int nSyncPos = -1, nSearchStep = 0, nNextIndex = 0;
	long lTemp = 0;
	unsigned int dwBytesToRead = G704_FRAME_SIZE*2;
	unsigned char	*_b_rst;

	dwBytesToRead = G704_FRAME_SIZE;
	if ( __FIf__->TL_nBCount <= dwBytesToRead+G704_FRAME_SIZE ) 
		return;

	_b_rst = TL_bTdmbRslt;

_FIND_SYNC_SEARCH_:
	/* find G.704 sync. */
	if ( TL_nSFindSync == 1 )
	{
		do {
			__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(_b_rst, 1024);
			nSyncPos = __FIf__->TSPL_GET_SYNC_POSITION(1, 0, _b_rst, 1024, 1024, 3);
			if ( nSyncPos >= 0 )
			{
				__FIf__->SetNewRdAlignPos_of_PlayBuf(nSyncPos);
				break;
			}
			__FIf__->SetNewRdAlignPos_of_PlayBuf(1024/2);

		} while ( __FIf__->TL_nBCount > 1024 );

		if ( nSyncPos < 0 ) 
			return;

		TL_nSFindSync = 0;
	}

	/* fill the sync buffer */
	while ( __FIf__->TL_nBCount > dwBytesToRead + G704_FRAME_SIZE )
	{
		__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(_b_rst, dwBytesToRead);
		__FIf__->SetNewRdAlignPos_of_PlayBuf(dwBytesToRead);

		if ( (*_b_rst & G704_SYNC_MASK) == G704_SYNC )
		{
			if ( ((*(_b_rst + G704_FRAME_SIZE) >> 6) & 0x01) == 1 )
			{
				;
			}
		}
		else if ( ((*_b_rst >> 6) & 0x01) == 1 )
		{
			if ( (*(_b_rst + G704_FRAME_SIZE) & G704_SYNC_MASK) == G704_SYNC )
			{
				;
			}
		}
		else
		{
			TL_nSFindSync = 1;
			goto _FIND_SYNC_SEARCH_;
		}
		FillTemp2Buf_from_GivenBuf(_b_rst, dwBytesToRead);
	}
}

//--------------------------------------------------------------------------
//	
//	Store ETI NA Sync searched data to NA Sync buffer
//
void CHldFmtrTdmb::TL_FillNASyncBuffer(void)
{
	int nSyncPos = -1, nSearchStep = 0, nNextIndex;
	int tmp0, tmp1;
	long lTemp = 0;
	unsigned int dwBytesToRead = G704_BLOCK_SIZE;
	unsigned char	*_b_rst;

	if ( TL_TdmbBCount <= dwBytesToRead + G704_BLOCK_SIZE ) 
		return;

	_b_rst = TL_bTdmbRslt;

_FIND_NA_SYNC_SEARCH_:
	/* find ETI NA sync. */
	if ( TL_nNFindSync == 1 )
	{
		do {
			FillRsltBuf_from_Temp2Buf_w_ChkBoundary(1024);
			nSyncPos = __FIf__->TSPL_GET_SYNC_POSITION(1, 1, _b_rst, 1024, 1024, 4);
			if ( nSyncPos >= 0 )
			{
				SetNewRdAlignPos_of_Temp2Buf(nSyncPos);
				break;
			}
			SetNewRdAlignPos_of_Temp2Buf(1024/2);
		} while ( TL_TdmbBCount > 1024 );

		if ( nSyncPos < 0 )
			return;

		TL_nNFindSync = 0;
		TL_nMBLKCount = BCNT(*(TL_bTdmbSync + TL_TdmbRIndex + TS1));
	}
	
	/* fill the sync buffer */
	while ( TL_TdmbBCount > dwBytesToRead + G704_BLOCK_SIZE )
	{
		nNextIndex = TL_TdmbRIndex + TS1_N;
		if ( nNextIndex >= MAX_BUFFER_BYTE_SIZE )
		{
			nNextIndex -= MAX_BUFFER_BYTE_SIZE;
		}
		
		tmp0 = BCNT(*(TL_bTdmbSync + nNextIndex));
		tmp1 = INCB(TL_nMBLKCount);
		if ( tmp0 != tmp1 )
		{
			TL_nNFindSync = 1;
			goto _FIND_NA_SYNC_SEARCH_;
		}
		TL_nMBLKCount = tmp0;
		
		FillRsltBuf_from_Temp2Buf_w_ChkBoundary(dwBytesToRead);
		SetNewRdAlignPos_of_Temp2Buf(dwBytesToRead);

		memcpy(TL_szBufferNA + TL_nNWIndex, _b_rst, dwBytesToRead);

		TL_nNWIndex += dwBytesToRead;
		TL_nNBCount += dwBytesToRead;

		if ( TL_nNWIndex >= MAX_BUFFER_BYTE_SIZE )
			TL_nNWIndex = 0;
	}
}

//--------------------------------------------------------------------------
//	
//	Convert NA stream to NI stream
//
void CHldFmtrTdmb::TL_ConvertToNI(void)
{
	long lTemp = 0;
	unsigned int dwBytesToRead = 6144;
	unsigned char	*_b_rst, *_b_tmp2;

	_b_rst = TL_bTdmbRslt;
	_b_tmp2 = TL_bTdmbTmp;

	while ( TL_nNBCount > (unsigned int)dwBytesToRead )
	{
		if (TL_nNRIndex <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - dwBytesToRead)))
		{
			memcpy(_b_rst, TL_szBufferNA + TL_nNRIndex, dwBytesToRead);
		}
		else 
		{
			lTemp = MAX_BUFFER_BYTE_SIZE-TL_nNRIndex;

			memcpy(_b_rst, TL_szBufferNA+TL_nNRIndex, lTemp);
			memcpy(_b_rst+lTemp, TL_szBufferNA, dwBytesToRead - lTemp);
		} 
		TL_nNRIndex += dwBytesToRead;
		if (TL_nNRIndex >= MAX_BUFFER_BYTE_SIZE)
			TL_nNRIndex -= MAX_BUFFER_BYTE_SIZE;
		
		TL_nNBCount -= dwBytesToRead;

		__FIf__->TSPL_DO_DEINTERLEAVING(_b_rst, _b_tmp2);
	
		TL_nRSErrorCnt = 0;
		TL_nRSRecoveredCnt = 0;
		__FIf__->TSPL_DO_RS_DECODING(_b_tmp2, _b_rst, &TL_nLIFormat, &TL_nRSErrorCnt, &TL_nRSRecoveredCnt, 0);

		__FIf__->TSPL_CONVERT_TO_NI(_b_rst, _b_tmp2, TL_nLIFormat);
		FillCalcSyncBuf_from_GivenBuf_w_ChkBoundary(_b_tmp2, dwBytesToRead);
	}
}


//--------------------------------------------------------------------------
//--- Generate Common Interleaved Frame
// Buffering->Find Sync->Buffering->
// Energy Dispersal->Convolutional Encoding->Time Interleaving->Generate Common Interleaved Frame
//
void CHldFmtrTdmb::TL_ConvertNItoCIF(void)
{
	int nSyncPos = -1;
	long lTemp = 0;
	unsigned int dwBytesToRead = 6144;
	unsigned char WrBuff[7300];
	unsigned char	*_b_rst;

	_b_rst = TL_bTdmbRslt;

	if ( __FIf__->TL_nBCount <= dwBytesToRead ) 
		return;

_FIND_204_SYNC_SEARCH_:
	/* find 204/208 sync. */
	if ( __FIf__->TL_nFindSync == 1 )
	{
		do {
			__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(_b_rst, dwBytesToRead);

			if( ((_b_rst[1] == 0x07) && (_b_rst[2] == 0x3a) && (_b_rst[3] == 0xb6)) 
				|| ((_b_rst[1] == 0xf8) && (_b_rst[2] == 0xc5) 	&& (_b_rst[3] == 0x49)) ) 
			{
				nSyncPos = 0;
				break;
			}
			else
			{
				__FIf__->SetNewRdAlignPos_of_PlayBuf(1);
			}

		} while ( __FIf__->TL_nBCount > dwBytesToRead );

		if ( nSyncPos < 0 ) 
			return;

		__FIf__->TL_nFindSync = 0;
	}

	/* fill the sync buffer */
	while ( __FIf__->TL_nBCount > dwBytesToRead*2 )
	{
		__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(_b_rst, dwBytesToRead*2);
		__FIf__->SetNewRdAlignPos_of_PlayBuf(dwBytesToRead);
		
		if( ((*(_b_rst + 1) == 0x07) && (*(_b_rst + 2) == 0x3a) && (*(_b_rst + 3) == 0xb6)) 
			|| ((*(_b_rst + 1) == 0xf8) && (*(_b_rst + 2) == 0xc5) && (*(_b_rst + 3) == 0x49)) ) 
		{
		}
		else
		{
			if( ((*(_b_rst + dwBytesToRead + 1) == 0x07) 
				&& (*(_b_rst + dwBytesToRead + 2) == 0x3a) 
				&& (*(_b_rst + dwBytesToRead + 3) == 0xb6)) 
				|| ((*(_b_rst + dwBytesToRead + 1) == 0xf8) 
				&& (*(_b_rst + dwBytesToRead + 2) == 0xc5) 
				&& (*(_b_rst + dwBytesToRead + 3) == 0x49)) ) 
			{
			}
			else
			{
				nSyncPos = -1;
				__FIf__->TL_nFindSync = 1;
				goto _FIND_204_SYNC_SEARCH_;
			}
		}

		if ( __FIf__->TVB380_SET_MODULATOR_RUN_CIF(_b_rst, WrBuff) != -1 )
		{
			FillCalcSyncBuf_from_GivenBuf_w_ChkBoundary(WrBuff, 7300);
		}
	}
}

//--------------------------------------------------------------------------
//	
//	Convert NA stream to CIF
//
void CHldFmtrTdmb::TL_ConvertNAtoCIF(void)
{
	long lTemp = 0;
	unsigned int dwBytesToRead = 6144;
	unsigned char WrBuff[7300];
	unsigned char	*_b_rst, *_b_tmp2;

	_b_rst = TL_bTdmbRslt;
	_b_tmp2 = TL_bTdmbTmp;

	while ( TL_nNBCount > (unsigned int)dwBytesToRead )
	{
		if (TL_nNRIndex <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - dwBytesToRead)))
		{
			memcpy(_b_rst, TL_szBufferNA + TL_nNRIndex, dwBytesToRead);
		}
		else 
		{
			lTemp = MAX_BUFFER_BYTE_SIZE-TL_nNRIndex;

			memcpy(_b_rst, TL_szBufferNA+TL_nNRIndex, lTemp);
			memcpy(_b_rst+lTemp, TL_szBufferNA, dwBytesToRead - lTemp);
		} 
		TL_nNRIndex += dwBytesToRead;
		if (TL_nNRIndex >= MAX_BUFFER_BYTE_SIZE)
			TL_nNRIndex -= MAX_BUFFER_BYTE_SIZE;
		
		TL_nNBCount -= dwBytesToRead;

		__FIf__->TSPL_DO_DEINTERLEAVING(_b_rst, _b_tmp2);
	
		TL_nRSErrorCnt = 0;
		TL_nRSRecoveredCnt = 0;
		__FIf__->TSPL_DO_RS_DECODING(_b_tmp2, _b_rst, &TL_nLIFormat,
			&TL_nRSErrorCnt, &TL_nRSRecoveredCnt, 0);

		__FIf__->TSPL_CONVERT_TO_NI(_b_rst, _b_tmp2, TL_nLIFormat);

		if ( __FIf__->TVB380_SET_MODULATOR_RUN_CIF(_b_tmp2, WrBuff) != -1 )
		{
			FillCalcSyncBuf_from_GivenBuf_w_ChkBoundary(WrBuff, 7300);
		}
	}
}

int CHldFmtrTdmb::Verify_ETI_Format(char *szFilePath)
{
	return SearchNIorNASync(szFilePath, 100000 * 4);
}
int CHldFmtrTdmb::SearchNIorNASync(char *szFilePath, int nLength)
{
	int	nSyncStartPos = -1;
	int	iTSize = 0;
	int isNA = -1;//==NI
	int nReadSize;
	FILE *fp;
	unsigned char *szBuffer;

	fp = fopen(szFilePath, "rb");
	if ( !fp )
	{
		return -1;
	}
	szBuffer = (unsigned char *)malloc(nLength);
	if ( !szBuffer )
	{
		fclose(fp);
		return -1;
	}
	nReadSize = fread(szBuffer, 1, nLength, fp);

	//G.703 Sync
	nSyncStartPos = __FIf__->TSPL_GET_SYNC_POSITION(0, 0, szBuffer, nReadSize, nReadSize, 3);
	if ( nSyncStartPos >= 0 )
	{
		isNA = 0;
	}
	else
	{
		//G.704 Sync
		nSyncStartPos = __FIf__->TSPL_GET_SYNC_POSITION(1, 0, szBuffer, nReadSize, nReadSize, 3);
		if ( nSyncStartPos >= 0 ) 
		{
			//ETI NA Sync
			iTSize = __FIf__->TSPL_GET_SYNC_POSITION(1, 1, szBuffer+nSyncStartPos, nReadSize-nSyncStartPos, nReadSize-nSyncStartPos, 4);
			if ( iTSize >= 0 )
			{
				isNA = 1;
			}
		}
	}
	if ( szBuffer )
	{
		free(szBuffer);
		szBuffer = NULL;
	}
	if (fp)
	{
		fclose(fp);
	}

	return isNA;
}

int CHldFmtrTdmb::CheckNIorNASync(char *szFilePath, int nLength, int *pSyncPos)
{
	int	nSyncStartPos = -1;
	int	iTSize = 0;
	int isNA = 0;//==NI
	int nReadSize;
	FILE *fp;
	unsigned char *szBuffer;

	fp = fopen(szFilePath, "rb");
	if ( !fp )
	{
		return -1;
	}
	szBuffer = (unsigned char *)malloc(nLength);
	if ( !szBuffer )
	{
		fclose(fp);
		return -1;
	}
	nReadSize = fread(szBuffer, 1, nLength, fp);

	//G.703 Sync
	nSyncStartPos = __FIf__->TSPL_GET_SYNC_POSITION(0, 0, szBuffer, nReadSize, nReadSize, 3);
	if ( nSyncStartPos >= 0 )
	{
		isNA = 0;
		*pSyncPos = nSyncStartPos;
	}
	else
	{
		//G.704 Sync
		nSyncStartPos = __FIf__->TSPL_GET_SYNC_POSITION(1, 0, szBuffer, nReadSize, nReadSize, 3);
		if ( nSyncStartPos >= 0 ) 
		{
			//ETI NA Sync
			iTSize = __FIf__->TSPL_GET_SYNC_POSITION(1, 1, szBuffer+nSyncStartPos, nReadSize-nSyncStartPos, nReadSize-nSyncStartPos, 4);
			if ( iTSize >= 0 )
			{
				isNA = 1;
				*pSyncPos = (nSyncStartPos+iTSize);
			}
		}
	}
	if ( szBuffer )
	{
		free(szBuffer);
		szBuffer = NULL;
	}
	if (fp)
	{
		fclose(fp);
	}

	return isNA;
}
int CHldFmtrTdmb::FillTdmb_CifBuf_OnPlayCont(void)
	//	process TL_szBufferPlay data. and the result has been stored in the buffer TL_sz2ndBufferPlay
{
	/* NA to NI and NI to CIF */
	/* TDMB - ETI ext. */
	if (__FIf__->IsNaFile() || (__FIf__->IsEtiFile() && TL_IsNIorNA == 1) )	//	NA
	{
		TL_FillTemp2SyncBuffer();
		TL_FillNASyncBuffer();
		TL_ConvertNAtoCIF();
	}
	/* NI to CIF */
	else	//	NI
	{
		//HldPrint("Hld-Bd-Ctl. ===========TL_ConvertNItoCIF============");
		TL_ConvertNItoCIF();
	}

	return	1;
}


