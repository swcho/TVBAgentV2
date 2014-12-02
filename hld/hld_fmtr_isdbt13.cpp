
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

#include	"hld_fmtr_isdbt13.h"
#include	"LLDWrapper.h"


//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrIsdbT13::CHldFmtrIsdbT13(int _my_id, void *_hld)	:	CHldTmccRmx(_my_id, _hld)
{
	my_hld_id = _my_id;
	my_hld = _hld;

	_fmter_sta_	= _STA_ISDBT13_undefined;

	nTsWrCounter = 0;
	nTsRdCounter = 0;

	nCnt = 0;
	nLenRd = 0;
	nFileLength = 0;

	HeaderC = HeaderB = HeaderA = 0x00;

	RdData = NULL;
	memset(&LayerA, 0x00, sizeof(layer));
	memset(&LayerB, 0x00, sizeof(layer));
	memset(&LayerC, 0x00, sizeof(layer));
	BufferC = BufferB = BufferA = NULL;
	nLenA = nLenB = nLenC = 0;
	nAddrA = nAddrB = nAddrC = 0;

	memset(TsData, 0x00, 204);
	memset(TmData, 0x00, 204);

	__dwRet = 188;

	now_active_tmcc_remuxing = 0;
//	test_buffer_temp = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE/2);

//TMCC REMUXER
	Current_Mode = 0;
	Current_GuardInterval = 0;
	Current_Partial_Reception = 0;
	Current_BoradcastType = 0;
	memset(&Current_LayerA, 0x00, sizeof(layer));
	memset(&Current_LayerB, 0x00, sizeof(layer));
	memset(&Current_LayerC, 0x00, sizeof(layer));

#if defined(WIN32)
	_dbg_file = INVALID_HANDLE_VALUE;
#endif

}

CHldFmtrIsdbT13::~CHldFmtrIsdbT13()
{
	/* ISDB-T 13SEG */
	if ( BufferA != NULL )		free(BufferA);
	if ( BufferB != NULL )		free(BufferB);
	if ( BufferC != NULL )		free(BufferC);
	if ( RdData != NULL )		free(RdData);

//	free(test_buffer_temp);
//	test_buffer_temp = NULL;
}

void	CHldFmtrIsdbT13::SetCommonMethod_82(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}
//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrIsdbT13::DbgWrCaptureDta(int _size, unsigned char *_buf)
{
	return	0;

#if defined(WIN32)
	unsigned long	_ret;

	if (_size == 0)
	{
		if ( _dbg_file != INVALID_HANDLE_VALUE )
		{
			CloseHandle(_dbg_file);
			_dbg_file = INVALID_HANDLE_VALUE;
		}
		return	_size;
	}
	if ( _dbg_file == INVALID_HANDLE_VALUE )
	{
		_dbg_file = CreateFile((char *)".\\isdbt_188_cap_raw.ts", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL);
	}
	WriteFile(_dbg_file, _buf, _size, &_ret, NULL);
#endif

	return	_size;
}
//////////////////////////////////////////////////////////////////////////////////////
int CHldFmtrIsdbT13::TL_Get_Datarate(int Mode, int GuardInterval, layer* pLayerA, layer* pLayerB, layer* pLayerC)
{
	int Bitrate[3];
	Bitrate[0] = TL_Get_Datarate(Mode, GuardInterval, pLayerA);
	if ( Bitrate[0] == -1 )
		Bitrate[0] = 0;
	Bitrate[1] = TL_Get_Datarate(Mode, GuardInterval, pLayerB);
	if ( Bitrate[1] == -1 )
		Bitrate[1] = 0;
	Bitrate[2] = TL_Get_Datarate(Mode, GuardInterval, pLayerC);
	if ( Bitrate[2] == -1 )
		Bitrate[2] = 0;

	return (Bitrate[0]+Bitrate[1]+Bitrate[2]);
}

int CHldFmtrIsdbT13::TL_IP_Initialize_SyncPosBuf(void)
{
	unsigned char tmpData[204];
	int kk;
	unsigned long lTemp;

	tmpData[0] = 0x47;
	tmpData[1] = 0x1f;
	for(kk=2; kk<204; kk++) 
	{
		tmpData[kk] = 0xff;
	}
	tmpData[PKTSIZE] = 0xfd;

	int BytesToRead = 204;
	for(kk=1; kk<10; kk++)
	{
		if (__FIf__->TL_nWritePos <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - BytesToRead)))
		{
			memcpy(__FIf__->TL_sz2ndBufferPlay + __FIf__->TL_nWritePos, tmpData, BytesToRead);
			__FIf__->TL_nWritePos += BytesToRead;
		}
		else 
		{
			lTemp = MAX_BUFFER_BYTE_SIZE-__FIf__->TL_nWritePos;

			memcpy(__FIf__->TL_sz2ndBufferPlay + __FIf__->TL_nWritePos, tmpData, lTemp);
			memcpy(__FIf__->TL_sz2ndBufferPlay, tmpData + lTemp, BytesToRead - lTemp);

			__FIf__->TL_nWritePos += BytesToRead;
			if (__FIf__->TL_nWritePos >= MAX_BUFFER_BYTE_SIZE)
				__FIf__->TL_nWritePos -= MAX_BUFFER_BYTE_SIZE;
		}
		__FIf__->TL_nBufferCnt += BytesToRead;
//		fwrite(&TmData[0], 1, 204, fp);
	}

	return 0;
}
int CHldFmtrIsdbT13::TL_Get_Datarate(int Mode, int GuardInterval, layer* pLayer)
{
	int TpPerFrame=0, TotalTpPerFrame=0;
	double Frame_Length=0.;//msec.

	if (pLayer->NumSegm != 0) 
	{
		switch ( pLayer->Constel )
		{
		case ISDBT_MOD_DQPSK :
		case ISDBT_MOD_QPSK :
			if ( pLayer->CodRate == ISDBT_RATE_1_2 )
				pLayer->TpPerFrame = 12;
			else if ( pLayer->CodRate == ISDBT_RATE_2_3 )
				pLayer->TpPerFrame = 16;
			else if ( pLayer->CodRate == ISDBT_RATE_3_4 )
				pLayer->TpPerFrame = 18;
			else if ( pLayer->CodRate == ISDBT_RATE_5_6 )
				pLayer->TpPerFrame = 20;
			else if ( pLayer->CodRate == ISDBT_RATE_7_8 )
				pLayer->TpPerFrame = 21;
			else 
				pLayer->TpPerFrame = 12;
			break;

		case ISDBT_MOD_16QAM :
			if ( pLayer->CodRate == ISDBT_RATE_1_2 )
				pLayer->TpPerFrame = 24;
			else if ( pLayer->CodRate == ISDBT_RATE_2_3 )
				pLayer->TpPerFrame = 32;
			else if ( pLayer->CodRate == ISDBT_RATE_3_4 )
				pLayer->TpPerFrame = 36;
			else if ( pLayer->CodRate == ISDBT_RATE_5_6 )
				pLayer->TpPerFrame = 40;
			else if ( pLayer->CodRate == ISDBT_RATE_7_8 )
				pLayer->TpPerFrame = 42;
			else 
				pLayer->TpPerFrame = 24;
			break;

		case ISDBT_MOD_64QAM :
			if ( pLayer->CodRate == ISDBT_RATE_1_2 )
				pLayer->TpPerFrame = 36;
			else if ( pLayer->CodRate == ISDBT_RATE_2_3 )
				pLayer->TpPerFrame = 48;
			else if ( pLayer->CodRate == ISDBT_RATE_3_4 )
				pLayer->TpPerFrame = 54;
			else if ( pLayer->CodRate == ISDBT_RATE_5_6 )
				pLayer->TpPerFrame = 60;
			else if ( pLayer->CodRate == ISDBT_RATE_7_8 )
				pLayer->TpPerFrame = 63;
			else 
				pLayer->TpPerFrame = 36;
			break;

		default :
			pLayer->TpPerFrame = 0;
			break;
		}
		pLayer->TpPerFrame *= (int)pow(2., Mode-1);
		pLayer->TpPerFrame *= pLayer->NumSegm;

		TpPerFrame += pLayer->TpPerFrame;//????
	}
	//sskim20080709 - fixed
	else
	{
		pLayer->TpPerFrame = 0;
	}

	// Bitrate, Total Tp per frame
	switch ( Mode )
	{
	case 1:
		if (GuardInterval == ISDBT_GUARD_1_32)
		{
			Frame_Length = 53.0145;
			TotalTpPerFrame = 1056;
		}
		else if (GuardInterval == ISDBT_GUARD_1_16)
		{
			Frame_Length = 54.621;
			TotalTpPerFrame = 1088;
		}
		else if (GuardInterval == ISDBT_GUARD_1_8)
		{
			Frame_Length = 57.834;
			TotalTpPerFrame = 1152;
		}
		else if (GuardInterval == ISDBT_GUARD_1_4)
		{
			Frame_Length = 64.26;
			TotalTpPerFrame = 1280;
		}
		break;
	case 2:
		if (GuardInterval == ISDBT_GUARD_1_32)
		{
			Frame_Length = 106.029;
			TotalTpPerFrame = 2112;
		}
		else if (GuardInterval == ISDBT_GUARD_1_16)
		{
			Frame_Length = 109.242;
			TotalTpPerFrame = 2176;
		}
		else if (GuardInterval == ISDBT_GUARD_1_8)
		{
			Frame_Length = 115.668;
			TotalTpPerFrame = 2304;
		}
		else if (GuardInterval == ISDBT_GUARD_1_4)
		{
			Frame_Length = 128.52;
			TotalTpPerFrame = 2560;
		}
		break;
	case 3:
		if (GuardInterval == ISDBT_GUARD_1_32)
		{
			Frame_Length = 212.058;
			TotalTpPerFrame = 4224;
		}
		else if (GuardInterval == ISDBT_GUARD_1_16)
		{
			Frame_Length = 218.484;
			TotalTpPerFrame = 4352;
		}
		else if (GuardInterval == ISDBT_GUARD_1_8)
		{
			Frame_Length = 231.336;
			TotalTpPerFrame = 4608;
		}
		else if (GuardInterval == ISDBT_GUARD_1_4)
		{
			Frame_Length = 257.04;
			TotalTpPerFrame = 5120;
		}
		break;

	default :
		TotalTpPerFrame = 0;
		break;
	}
	//sskim20080709 - fixed
	if ( Frame_Length <= 0 )
	{
		pLayer->Bps = 0;
	}
	else
		pLayer->Bps = (int)(TpPerFrame * PKTSIZE * 8. * (1./Frame_Length) * 1000.);
	
	return pLayer->Bps;
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldFmtrIsdbT13::GetRmxDtaRate(long layer_index)
{
	layer *pLayer;
	if ( layer_index == 0 ) 
		pLayer = &Current_LayerA;
	else if ( layer_index == 1 ) 
		pLayer = &Current_LayerB;
	else if ( layer_index == 2 ) 
		pLayer = &Current_LayerC;
	else
		return -1;

	TL_Get_Datarate(Current_Mode, Current_GuardInterval, pLayer);
	return pLayer->Bps;
}
int	CHldFmtrIsdbT13::SetRmx_Info(
	long btype, long mode, long guard_interval, long partial_reception, long bitrate,
	long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
	long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
	long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
	layer *pLayer;
	int NumOfSegs = a_segments + b_segments + c_segments;
	if ( (btype == 0 && NumOfSegs != 13)
		|| (btype == 1 && NumOfSegs != 1)
		|| (btype == 2 && NumOfSegs != 3) 
		|| (btype == 3 && NumOfSegs != 1) )
	{
		return -1;
	}
	if ( btype == 0 && partial_reception == 1 && a_segments != 1 )
	{
		return -2;
	}
	if ( btype == 3 && (partial_reception != 1 || a_segments != 1) )
	{
		return -3;
	}
	
	Current_Mode = mode;
	Current_GuardInterval = guard_interval;
	Current_Partial_Reception = partial_reception;
	Current_BoradcastType = btype;

	pLayer = &Current_LayerA;
	pLayer->Constel = a_modulation;
	pLayer->CodRate = a_code_rate;
	pLayer->NumSegm = a_segments;
	
	pLayer = &Current_LayerB;
	pLayer->Constel = b_modulation;
	pLayer->CodRate = b_code_rate;
	pLayer->NumSegm = b_segments;

	pLayer = &Current_LayerC;
	pLayer->Constel = c_modulation;
	pLayer->CodRate = c_code_rate;
	pLayer->NumSegm = c_segments;

	TMCC_SET_REMUX_INFO(btype, mode, guard_interval,  partial_reception, bitrate,
		a_segments,  a_modulation,  a_code_rate,  a_time_interleave, a_bitrate,
		b_segments,  b_modulation,  b_code_rate,  b_time_interleave, b_bitrate,
		c_segments,  c_modulation,  c_code_rate,  c_time_interleave, c_bitrate);

	return 0;
}
int	CHldFmtrIsdbT13::GetRmx_Info(char *szFile, int layer_index)
{
	FILE	*file_stream;
	int		syncStart, iTSize;
	size_t	readByte;
	BYTE	*AP_szBuffer;

	int nMode, nGuardInterval, nPartial_Reception, nBoradcastType, nRet;
	unsigned char szHeaderA, szHeaderB, szHeaderC;

	layer stLayerA, stLayerB, stLayerC, *pLayer;
	int nLengthA, nLengthB, nLengthC;
	int nTryCount = 10;

	if ( __Sta__->IsModTyp_IsdbT_1() || __Sta__->IsModTyp_IsdbT_13() )
	{
	}
	else
	{
		//__HLog__->HldPrint("Hld-Bd-Ctl. It's only for ISDB-T/13SEG.");
		return	-1;
	}

	//-----------------------------------------------------------------------
	// Memory allocation
	AP_szBuffer = (unsigned char*)malloc(0x100000);	// 0x100000 = 1M

	if (AP_szBuffer == NULL)
	{
		__HLog__->HldPrint("Hld-Bd-Ctl. FAIL alloc-memory");
		return	-1;
	}

	//-----------------------------------------------------------------------
	// Open and Read target file
	file_stream = fopen(szFile, "rb");
	if (file_stream == NULL)
	{
		__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to open FILE ");
		free (AP_szBuffer);
		return	-1;
	}

	do
	{
		readByte = fread (AP_szBuffer,  1, 0x100000, file_stream);
		if (readByte < 0x100000)
		{
			break;
		}

		//-----------------------------------------------------------------------
		// Check Packet Size (188,204, and 208)
		syncStart = __FIf__->TL_SyncLockFunction((char*)AP_szBuffer, readByte, &iTSize, readByte, 20);

		//-----------------------------------------------------------------------
		// Calculate Bitrate
		if (syncStart != -1)
		{
			//Find the frame indicator information
			nRet = TL_FrameIndicatorFunction(AP_szBuffer, readByte,
				&szHeaderA, &szHeaderB, &szHeaderC, &nLengthA, &nLengthB, &nLengthC,
				&nMode, 
				&stLayerA, &stLayerB, &stLayerC,
				&nGuardInterval, &nPartial_Reception, &nBoradcastType);
			if ( nRet == -1 )
			{
				--nTryCount;

				continue;
			}

			fclose (file_stream);
			free (AP_szBuffer);

			if ( layer_index == 0 || layer_index == 3)
				pLayer = &stLayerA;
			else if ( layer_index == 1 || layer_index == 4)
				pLayer = &stLayerB;
			else if ( layer_index == 2 || layer_index == 5)
				pLayer = &stLayerC;
			else
			{
			}

			//ISDB-T paramters including Layer paramters
			if ( layer_index == 0 || layer_index == 1 || layer_index == 2 )
			{
				nRet = (nBoradcastType << 18 ) 
						| (nPartial_Reception << 17) 
						| (nGuardInterval << 15) 
						| (nMode << 13)
						| (pLayer->Time_Interleaving << 10) 
						| (pLayer->Constel << 7) 
						| (pLayer->CodRate << 4) 
						| (pLayer->NumSegm);
			}
			//data rate
			else if ( layer_index == 3 || layer_index == 4 || layer_index == 5 )
			{
				TL_Get_Datarate(nMode, nGuardInterval,	pLayer);
				nRet = pLayer->Bps;
			}
			//total data rate
			else
			{
				nRet = TL_Get_Datarate(nMode, nGuardInterval,
					&stLayerA, &stLayerB, &stLayerC);
			}

			return	nRet;
		}
	} while ((--nTryCount > 0) && !feof(file_stream));

	//__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to query remux information");
	fclose (file_stream);
	free (AP_szBuffer);

	return -1;
}
int	CHldFmtrIsdbT13::TL_FrameIndicatorFunction(unsigned char *szBuf, int nlen,
									unsigned char* pHeaderA,
									unsigned char* pHeaderB,
									unsigned char* pHeaderC,
									int *pDelayLenA,
									int *pDelayLenB,
									int *pDelayLenC,
									int *pMode,
									layer* pLayerA,
									layer* pLayerB,
									layer* pLayerC,
									int *pGuardInterval, 
									int *pPartial_Reception,
									int *pBroadcastType) 
{
	int	i, j, nPos;
	long	lTemp = 0;
	unsigned int	dwBytesToRead = nlen;
	unsigned char	TsData_[204];

	j = __FIf__->TL_SyncLockFunction((char*)szBuf, dwBytesToRead, (int*)&i, dwBytesToRead, 3);
#if defined(WIN32)
	wsprintf(debug_string,"::SYNC=%d\n", j);
#else
	sprintf(debug_string,"::SYNC=%d\n", j);
#endif

	if  ( j == -1 || (i == PKTSIZE || i == -1) )
	{
		return -1;
	}
	nPos = j;

	memset(TsData_, 0x00, 204);
	*pHeaderA = *pHeaderB = *pHeaderC = 0x00;
	*pDelayLenA = *pDelayLenB = *pDelayLenC = 0;
	*pMode = 0;
	memset(pLayerA, 0x00, sizeof(layer));
	memset(pLayerB, 0x00, sizeof(layer));
	memset(pLayerC, 0x00, sizeof(layer));
	
	*pGuardInterval = -1;
	*pPartial_Reception = -1;
	*pBroadcastType = 0;

	while (nPos < nlen) 
	{
		if (szBuf[nPos] == 0x47) 
		{
			memcpy(TsData_, &szBuf[nPos], 204);
			if((TsData_[189] >> 4) == 0x08) 
			{
				*pMode = ((TsData_[7] >> 6) & 0x03);

				*pGuardInterval = ((TsData_[7] >> 4) & 0x03);
				*pPartial_Reception = (TsData_[8] & 0x01);
				*pBroadcastType = ((TsData_[8] >> 6) & 0x01);

				pLayerA->Constel = ((TsData_[9] >> 5) & 0x07);
				pLayerA->CodRate = ((TsData_[9] >> 2) & 0x07);
				pLayerA->Time_Interleaving = ((TsData_[9] & 0x03) << 1) | ((TsData_[10] >> 7) & 0x01);
				pLayerA->NumSegm = ((TsData_[10] >> 3) & 0x0f);
				//sskim20080625 - 12 -> 13
				if ( pLayerA->NumSegm < 1 || pLayerA->NumSegm > 13 )
					pLayerA->NumSegm = 0;

				pLayerB->Constel = ((TsData_[10]) & 0x07);
				pLayerB->CodRate = ((TsData_[11] >> 5) & 0x07);
				pLayerB->Time_Interleaving = ((TsData_[11] >> 2) & 0x07);
				pLayerB->NumSegm = ((TsData_[11] << 2) & 0x0c) | ((TsData_[12] >> 6) & 0x03);
				//sskim20080625 - 12 -> 13
				if ( pLayerB->NumSegm < 1 || pLayerB->NumSegm > 13 )
					pLayerB->NumSegm = 0;

				pLayerC->Constel = ((TsData_[12] >> 3) & 0x07);
				pLayerC->CodRate = ((TsData_[12]) & 0x07);
				pLayerC->Time_Interleaving = ((TsData_[13] >> 5) & 0x07);
				pLayerC->NumSegm = ((TsData_[13] >> 1) & 0x0f);
				//sskim20080625 - 12 -> 13
				if ( pLayerC->NumSegm < 1 || pLayerC->NumSegm > 13 )
					pLayerC->NumSegm = 0;
			}

			// To Obtain the frame indicator
			else if(((TsData_[189] >> 4) == 0x01) && ((*pHeaderA & 0x80) != 0x80))
			{
				*pHeaderA = TsData_[PKTSIZE];
			}
			else if(((TsData_[189] >> 4) == 0x02) && ((*pHeaderB & 0x80) != 0x80))
			{
				*pHeaderB = TsData_[PKTSIZE];
			}
			else if(((TsData_[189] >> 4) == 0x03) && ((*pHeaderC & 0x80) != 0x80))
			{
				*pHeaderC = TsData_[PKTSIZE];
			}

			nPos += 204;
		}
		else
		{
			nPos++;
		}
	}

	*pDelayLenA = DoCalculateDelayAdjustLength(*pMode, pLayerA->Constel, pLayerA->CodRate, pLayerA->NumSegm);
	*pDelayLenB = DoCalculateDelayAdjustLength(*pMode, pLayerB->Constel, pLayerB->CodRate, pLayerB->NumSegm);
	*pDelayLenC = DoCalculateDelayAdjustLength(*pMode, pLayerC->Constel, pLayerC->CodRate, pLayerC->NumSegm);

#if defined(WIN32)
	wsprintf(debug_string,"::Header A/B/C=0x%x,0x%x,0x%x\n", 
		*pHeaderA, *pHeaderB, *pHeaderC);
//	__HLog__->HldPrint(debug_string);
	wsprintf(debug_string,"::DelayLength A/B/C=0x%x,0x%x,0x%x\n", 
		*pDelayLenA, *pDelayLenB, *pDelayLenC);
//	__HLog__->HldPrint(debug_string);
#else
	sprintf(debug_string,"::Header A/B/C=0x%x,0x%x,0x%x\n", 
		*pHeaderA, *pHeaderB, *pHeaderC);
//	__HLog__->HldPrint(debug_string);
	sprintf(debug_string,"::DelayLength A/B/C=0x%x,0x%x,0x%x\n", 
		*pDelayLenA, *pDelayLenB, *pDelayLenC);
//	__HLog__->HldPrint(debug_string);
#endif

	//sskim20080827
	TL_Get_Datarate(*pMode, *pGuardInterval, pLayerA, pLayerB, pLayerC);

	return j;//first sync. position
}
/* ISDB-T 13SEG */
int CHldFmtrIsdbT13::DoCalculateDelayAdjustLength(int Mode, int Constel, int CodRate, int NumSegm)
{
	int nDelayAdjustLength;
	
	switch(Constel){
	case 2:
		switch(CodRate) {
		case 0:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 24 * NumSegm - 11;
			break;
		case 1:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 32 * NumSegm - 11;
			break;
		case 2:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 36 * NumSegm - 11;
			break;
		case 3:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 40 * NumSegm - 11;
			break;
		case 4:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 42 * NumSegm - 11;
			break;
		default:
			nDelayAdjustLength = 0;
		break;
		}
		break;
	case 3:
		switch(CodRate) {
		case 0:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 36 * NumSegm - 11;
			break;
		case 1:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 48 * NumSegm - 11;
			break;
		case 2:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 54 * NumSegm - 11;
			break;
		case 3:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 60 * NumSegm - 11;
			break;
		case 4:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 63 * NumSegm - 11;
			break;
		default:
			nDelayAdjustLength = 0;
			break;
		}
		break;
	default:
		switch(CodRate) {
		case 0:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 12 * NumSegm - 11;
			break;
		case 1:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 16 * NumSegm - 11;
			break;
		case 2:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 18 * NumSegm - 11;
			break;
		case 3:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 20 * NumSegm - 11;
			break;
		case 4:
			nDelayAdjustLength = (int)pow(2., Mode-1) * 21 * NumSegm - 11;
			break;
		default:
			nDelayAdjustLength = 0;
			break;
		}
		break;
	}

	return nDelayAdjustLength;
}
void	CHldFmtrIsdbT13::CLEAR_REMUX_INFO(void)
{
	Current_Mode = 0;
	Current_GuardInterval = 0;
	Current_Partial_Reception = 0;
	Current_BoradcastType = 0;
	memset(&Current_LayerA, 0x00, sizeof(layer));
	memset(&Current_LayerB, 0x00, sizeof(layer));
	memset(&Current_LayerC, 0x00, sizeof(layer));
	__FIf__->Emergency_Broadcasting = 0;
	TMCC_SET_REMUX_INFO(0, 0, 0,  0, 0, 0,  0,  0,  0, 0, 0,  0,  0,  0, 0, 0,  0,  0,  0, 0);

}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void CHldFmtrIsdbT13::InitPlayParameters_IsdbT13(void)
{
	if ( !__Sta__->IsModTyp_IsdbT_13() )
	{
		return;
	}

	GuardInterval = -1;
	Partial_Reception = -1;
	BoradcastType = 0;
	TL_nFindFrameIndicator = 0;

	nTsWrCounter = 0;
	nTsRdCounter = 0;
	nFileLength = 0;
	HeaderC = HeaderB = HeaderA = 0x00;

	memset(TsData, 0x00, 204);
	memset(TmData, 0x00, 204);

	memset(&LayerA, 0x00, sizeof(layer));
	memset(&LayerB, 0x00, sizeof(layer));
	memset(&LayerC, 0x00, sizeof(layer));
	nLenA = nLenB = nLenC = 0;
	nAddrA = nAddrB = nAddrC = 0;

	nCnt = 0;
	nLenRd = 204 * 6000;

	if ( RdData == NULL )
		RdData = (unsigned char *)malloc(nLenRd + 204);

	if ( BufferA == NULL )
		BufferA = (unsigned char *)malloc(668304);

	if ( BufferB == NULL )
		BufferB = (unsigned char *)malloc(668304);

	if ( BufferC == NULL )
		BufferC = (unsigned char *)malloc(668304);

}
DWORD	*CHldFmtrIsdbT13::PktSize_IsdbT13_AtscMH(void)
{
	return	&__dwRet;
}

int CHldFmtrIsdbT13::ActivateTmccRmxer(void)
{
#if	1
	__HLog__->HldPrint("...donot use this func : ActivateTmccRmxer");
#else
	/* ACTIVATE TMCC REMUXER */
	if ( now_active_tmcc_remuxing == 0 )
	{
		if ( Start_TmccRemuxer(test_buffer_temp) == 0 )
		{
			now_active_tmcc_remuxing = 0;
			return	0;
		}
		now_active_tmcc_remuxing = 1;
	}
#endif
	return	1;
}
int CHldFmtrIsdbT13::DeactivateTmccRmxer(void)
{
	if ( now_active_tmcc_remuxing == 1 )
	{
		TMCC_STOP_REMUX();
		Sleep(100);
		TMCC_CLOSE_REMUX_SOURCE();
	}

	return	1;
}
int CHldFmtrIsdbT13::SetOffset_TmccRemuxer(int _setcur_as_start)
{
	if ( __Sta__->IsModTyp_IsdbT_13() && ( __FIf__->TmccUsing() ) )
	{
		if (_setcur_as_start)
		{
			__FIf__->g_CurrentOffsetL = __FIf__->g_StartOffsetL;
			__FIf__->g_CurrentOffsetH = __FIf__->g_StartOffsetH;
		}
		TMCC_SET_REMUX_SOURCE_POSITION(
			__FIf__->g_CurrentOffsetL, __FIf__->g_CurrentOffsetH,
			__FIf__->g_CurrentOffsetL, __FIf__->g_CurrentOffsetH,
			__FIf__->g_CurrentOffsetL, __FIf__->g_CurrentOffsetH);
	}
	return	0;
}
int CHldFmtrIsdbT13::Restart_TmccRemuxerFor_IsdbT_1_13(void)
{
	/* ISDB-T 1SEG (=9), ISDB-T 13SEG (=10) ONLY. TMCC REMUXER Initialized */
	if ( __Sta__->IsModTyp_IsdbT_13() && ( __FIf__->TmccUsing() ) )
	{
		TMCC_STOP_REMUX();
		if ( __FIf__->IsTheFileNot_PlayParam_FName(__FIf__->g_Current_Target) != 0 )
		{
			TMCC_CLOSE_REMUX_SOURCE();
			TMCC_OPEN_REMUX_SOURCE(__FIf__->GetPlayParam_FName());
		}
		if (TMCC_START_REMUX() == -1)
		{
			__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
			__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to start TMCC REMUXER");
			return 0;
		}
		/* TMCC REMUXER CURRENT OFFSET CHANGE. */
		if ( __FIf__->g_StartOffsetL > 0 || __FIf__->g_StartOffsetH > 0 )
		{
			SetOffset_TmccRemuxer(1);
		}
	}
	return	0;
}
int CHldFmtrIsdbT13::Restart_TmccRemuxerAtEof_IsdbT_1_13(void)
{
	/* TMCC REMUXER RESTARTED. */
	if ( (__Sta__->IsModTyp_IsdbT_13()) && ( __FIf__->TmccUsing() ) )
	{
		TMCC_STOP_REMUX();
		if (TMCC_START_REMUX() == -1)
		{
			__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
			__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to start TMCC REMUXER\n");
			return 0;
		}
	
		/* CURRENT OFFSET CHANGE. */
		if ( __FIf__->g_StartOffsetL > 0 || __FIf__->g_StartOffsetH > 0 )
		{
			SetOffset_TmccRemuxer(1);
		}
	}
	return 1;
}
int CHldFmtrIsdbT13::SetOffset_TmccRemuxerRepeatCond(void)
{
	if ( (__Sta__->IsModTyp_IsdbT_13()) && ( __FIf__->TmccUsing() ) )
	{
		__FIf__->g_CurrentOffsetL = TMCC_GET_REMUX_SOURCE_POSITION(0, (long*)&__FIf__->g_CurrentOffsetH);
	
		if ( (__FIf__->g_CurrentOffsetH > __FIf__->g_EndOffsetH) ||
			(__FIf__->g_CurrentOffsetH == __FIf__->g_EndOffsetH &&
				__FIf__->g_CurrentOffsetL > __FIf__->g_EndOffsetL ) )
		{
			SetOffset_TmccRemuxer(1);
		}
	}
	return 1;
}

int CHldFmtrIsdbT13::Start_TmccRemuxer(unsigned char* pBuffer)
{
#if	1
	__HLog__->HldPrint("...donot use this func : Start_TmccRemuxer");
#else
	int nBankBlockSize = DMA_TRANS_UNIT_CAP_PLAY;

	int nMode, nGuardInterval, nPartial_Reception, nBoradcastType, nRet;
	unsigned char szHeaderA, szHeaderB, szHeaderC;

	layer stLayerA, stLayerB, stLayerC;
	int nLengthA, nLengthB, nLengthC;

	nRet = TL_FrameIndicatorFunction(pBuffer, nBankBlockSize,
		&szHeaderA, &szHeaderB, &szHeaderC, &nLengthA, &nLengthB, &nLengthC,
		&nMode, 
		&stLayerA, &stLayerB, &stLayerC,
		&nGuardInterval, &nPartial_Reception, &nBoradcastType);
	if ( nRet == -1 )
	{
		return 0;
	}

	TMCC_SET_REMUX_INFO(nBoradcastType, nMode, nGuardInterval, nPartial_Reception, 0/*long bitrate*/,
		stLayerA.NumSegm, stLayerA.Constel, stLayerA.CodRate, stLayerA.Time_Interleaving, stLayerA.Bps,
		stLayerB.NumSegm, stLayerB.Constel, stLayerB.CodRate, stLayerB.Time_Interleaving, stLayerB.Bps,
		stLayerC.NumSegm, stLayerC.Constel, stLayerC.CodRate, stLayerC.Time_Interleaving, stLayerC.Bps);

	TMCC_STOP_REMUX();
	Sleep(100);
	TMCC_CLOSE_REMUX_SOURCE();

	char szDummyPath[MAX_PATH];
	sprintf(szDummyPath, "%s\\%s", __FIf__->g_szCurDir, TMCC_REMUXER_DUMMY_PATH);
	nRet = TMCC_OPEN_REMUX_SOURCE(szDummyPath);
	if ( nRet != 0 )
	{
		TMCC_CLOSE_REMUX_SOURCE();
		return 0;
	}

	nRet = TMCC_START_REMUX();
	if ( nRet == -1)
	{
		__HLog__->HldPrint("Hld-Bd-Ctl. FAIL to start TMCC REMUXER");

		TMCC_STOP_REMUX();
		Sleep(100);
		TMCC_CLOSE_REMUX_SOURCE();
	}

	return (nRet == -1 ? 0 : 1);
#endif
	return 0;
}

void CHldFmtrIsdbT13::StopAndClose_TmccRemuxer(void)
{
	/* ISDB-T 1SEG (=9), ISDB-T 13SEG (=10) ONLY. TMCC REMUXER Uninitialized */
	if ( (__Sta__->IsModTyp_IsdbT_13()) && ( __FIf__->TmccUsing() ) )
	{
		TMCC_STOP_REMUX();
		Sleep(100);
		TMCC_CLOSE_REMUX_SOURCE();
	}
}
int CHldFmtrIsdbT13::StopAndClose_Restart_TmccRemuxer(void)
{
	/* ISDB-T 1SEG (=9), ISDB-T 13SEG (=10) ONLY. TMCC REMUXER Initialized */
	if ( (__Sta__->IsModTyp_IsdbT_13()) && ( __FIf__->TmccUsing() ) )
	{
		TMCC_STOP_REMUX();
		Sleep(100);
		TMCC_CLOSE_REMUX_SOURCE();
		TMCC_OPEN_REMUX_SOURCE(__FIf__->GetPlayParam_FName());
		if (TMCC_START_REMUX() == -1)
		{
			return 0;
		}
	}
	return 1;
}

int CHldFmtrIsdbT13::RdPlayDta_From_TmccRemuxer(void)
{
	char	*tmp;
	int		ReadPos, BufferCnt, EndOfRemux;
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

	tmp = TMCC_READ_REMUX_DATA(nBankBlockSize, &ReadPos, &BufferCnt, &EndOfRemux);
	if (tmp)
	{
		__FIf__->TL_gFirstRead = 0;
	
		__FIf__->TL_dwBytesRead = nBankBlockSize;
		memcpy(__FIf__->TL_szBufferPlay+__FIf__->TL_nWIndex, tmp, nBankBlockSize);
	}
	else
	{
		return 0;
	}
	return 1;
}


void CHldFmtrIsdbT13::TL_DelayAdjustment(void)	//	isdbt-13 only. the rslt data is stored in TL_sz2ndBufferPlay, and TL_szBufferPlay is src-data.
{
	int	i, j;
	long	lTemp = 0;
	unsigned int	dwBytesToRead = nLenRd;
	unsigned char	*_b_sync;
	unsigned char	*Buf;
	unsigned int	dwReadSize;

	_b_sync = __FIf__->__BufPlay_2();

	if ( __FIf__->TL_nBCount < dwBytesToRead ) 
	{
		return;
	}
	if ( TL_nFindFrameIndicator == 0 )
	{
		__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(RdData, dwBytesToRead);
		__FIf__->SetNewRdAlignPos_of_PlayBuf(dwBytesToRead);

		//Find the frame indicator information
		nCnt = TL_FrameIndicatorFunction(RdData, nLenRd,
			&HeaderA, &HeaderB, &HeaderC, &nLenA, &nLenB, &nLenC,
			&Mode, 
			&LayerA, &LayerB, &LayerC,
			&GuardInterval, &Partial_Reception,	&BoradcastType);

		if ( nCnt == -1 )
		{
			nCnt = 0;
			return;
		}

		//sskim20080701
		nCnt = 0;
		nAddrA = 0;
		nAddrB = 0;
		nAddrC = 0;

		// Null TS
		TmData[0] = 0x47;
		TmData[1] = 0x1f;
		for(i=2; i<204; i++)
		{
			TmData[i] = 0xff;
		}

		for(i=0; i<3276; i++) 
		{
			memcpy(&BufferA[204 * i], TmData, 204);
			BufferA[204 * i+PKTSIZE] = ~HeaderA;			// Store the frame indicator to Buffer

			memcpy(&BufferB[204 * i], TmData, 204);
			BufferB[204 * i+PKTSIZE] = ~HeaderB;

			memcpy(&BufferC[204 * i], TmData, 204);
			BufferC[204 * i+PKTSIZE] = ~HeaderC;
		}

		//sskim20080630
		if ( !__Sta__->TL_nIP_Initialized )
		{
			__Sta__->TL_nIP_Initialized = 1;
			
		
			//Start of IP initialize
			//sskim20080702 - fixed
			TL_IP_Initialize_SyncPosBuf();
			//End of IP initialize

			//sskim20080704 - ADJUST FRAME DROP
			__FIf__->g_PacketCount = 1024 << (Mode - 1);
			__FIf__->g_PacketCount = __FIf__->g_PacketCount + (__FIf__->g_PacketCount >> (5 - GuardInterval));
			if ( __FIf__->g_PacketCount < 1088 )
			{
				__FIf__->g_PacketCount = 1088;
			}
			else if ( __FIf__->g_PacketCount > 5120 )
			{
				__FIf__->g_PacketCount = 5120;
			}
		}

		TL_nFindFrameIndicator = 1;
	}

	dwBytesToRead = nLenRd;
	while ( TL_nFindFrameIndicator == 1 || __FIf__->TL_nBCount >= dwBytesToRead )
	{
		if ( TL_nFindFrameIndicator == 1 )
		{
			TL_nFindFrameIndicator = 2;
		}
		else
		{
			if ( __FIf__->TL_nBCount < dwBytesToRead )
			{
				break;
			}

			i = 0;
			if ( __FIf__->TL_nFindSync > 0 )
			{
				i = (dwBytesToRead-__FIf__->TL_nFindSync);
				memcpy(RdData, RdData+__FIf__->TL_nFindSync, i);
			}
			
			Buf = &RdData[i];
			dwReadSize = dwBytesToRead-i;
			__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(Buf, dwReadSize);
			__FIf__->SetNewRdAlignPos_of_PlayBuf(dwReadSize);
		}

		while (nCnt <= nLenRd - 204)
		{
			if (RdData[nCnt] == 0x47) 
			{
				memcpy(TsData, &RdData[nCnt], 204);

				if((TsData[PKTSIZE] & 0x02) == 0x02) 
				{
					nTsRdCounter = 0;
				}

				if(nTsRdCounter != (256*(TsData[190]&0x1f)+TsData[191])) 
				{
					nCnt += 204;
					nTsRdCounter++;
					__FIf__->g_FrameDrop = 1;
					continue;
				}
				else
				{
					if ( __FIf__->g_FrameDrop == 1 )
					{
						TL_IP_Initialize_SyncPosBuf();
						__FIf__->g_FrameDrop = -1;
					}
				}

				nTsRdCounter++;

				if(((TsData[189] >> 4) & 0x0f) == 0x01) 		// For Layer A
				{
					if((TsData[PKTSIZE] & 0x02) == 0x02) 
					{
						nTsWrCounter = 0;
					}

					memcpy(TmData, &BufferA[204 * nAddrA], PKTSIZE);
					TmData[PKTSIZE] = (TsData[PKTSIZE] & 0xfe) | (BufferA[204 * nAddrA + PKTSIZE] & 0x01);	// Delay the frame indicator
					memcpy(&TmData[189], &TsData[189], 15);
					TmData[190] = (TmData[190] & 0xe0) | ((nTsWrCounter >> 8) & 0x1f);
					TmData[191] = (nTsWrCounter & 0xff);
					//FIXED - ISDB-T Emergency Broadcasting Control
					TmData[PKTSIZE] = (((TmData[PKTSIZE]>>4)&0x0F)<<4) + (__FIf__->Emergency_Broadcasting<<3) + (TmData[PKTSIZE]&0x07);

					__FIf__->CheckAdaptation(&TmData[0], 204);

					//////////////////////////////////////////////////////////////////////
					//sskim20080725 - BERT
					#if 1
					if(((TsData[189] >> 4) & 0x0f) != 0x08 && ((TsData[189] >> 4) & 0x0f) != 0x04)
						__FIf__->TL_BERT_SET_DATA(TmData);
					#endif
					//////////////////////////////////////////////////////////////////////

					// FILL 2ND PLAY BUFFER(_b_sync)
					__FIf__->FillOnePacketToQueue(204, _b_sync, (int*)&__FIf__->TL_nWritePos, (int*)&__FIf__->TL_nReadPos, (int*)&__FIf__->TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TmData);

					memcpy(&BufferA[204 * nAddrA], TsData, 204);
					if(nAddrA < (nLenA-1)) 
					{
						nAddrA++;
					}
					else 
					{						
						nAddrA = 0;
					}
				}
				else if(((TsData[189] >> 4) & 0x0f) == 0x02) 		// For Layer B
				{
					if((TsData[PKTSIZE] & 0x02) == 0x02) 
					{
						nTsWrCounter = 0;
					}
					memcpy(TmData, &BufferB[204 * nAddrB], PKTSIZE);
					TmData[PKTSIZE] = (TsData[PKTSIZE] & 0xfe) | (BufferB[204 * nAddrB + PKTSIZE] & 0x01);	// Delay the frame indicator
					memcpy(&TmData[189], &TsData[189], 15);
					TmData[190] = (TmData[190] & 0xe0) | ((nTsWrCounter >> 8) & 0x1f);
					TmData[191] = (nTsWrCounter & 0xff);
					//FIXED - ISDB-T Emergency Broadcasting Control
					TmData[PKTSIZE] = (((TmData[PKTSIZE]>>4)&0x0F)<<4) + (__FIf__->Emergency_Broadcasting<<3) + (TmData[PKTSIZE]&0x07);

					__FIf__->CheckAdaptation(&TmData[0], 204);

					//////////////////////////////////////////////////////////////////////
					//sskim20080725 - BERT
					#if 1
					if(((TsData[189] >> 4) & 0x0f) != 0x08 && ((TsData[189] >> 4) & 0x0f) != 0x04)
						__FIf__->TL_BERT_SET_DATA(TmData);
					#endif
					//////////////////////////////////////////////////////////////////////

					// FILL 2ND PLAY BUFFER(_b_sync)
					__FIf__->FillOnePacketToQueue(204, _b_sync, (int*)&__FIf__->TL_nWritePos, (int*)&__FIf__->TL_nReadPos, (int*)&__FIf__->TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TmData);

					memcpy(&BufferB[204 * nAddrB], TsData, 204);
					if(nAddrB < (nLenB-1)) 
					{
						nAddrB++;
					}
					else 
					{					
						nAddrB = 0;
					}
				}
				else if(((TsData[189] >> 4) & 0x0f) == 0x03) 		// For Layer C
				{
					if((TsData[PKTSIZE] & 0x02) == 0x02) 
					{
						nTsWrCounter = 0;
					}

					memcpy(TmData, &BufferC[204 * nAddrC], PKTSIZE);
					TmData[PKTSIZE] = (TsData[PKTSIZE] & 0xfe) | (BufferC[204 * nAddrC + PKTSIZE] & 0x01);	// Delay the frame indicator
					memcpy(&TmData[189], &TsData[189], 15);
					TmData[190] = (TmData[190] & 0xe0) | ((nTsWrCounter >> 8) & 0x1f);
					TmData[191] = (nTsWrCounter & 0xff);
					//FIXED - ISDB-T Emergency Broadcasting Control
					TmData[PKTSIZE] = (((TmData[PKTSIZE]>>4)&0x0F)<<4) + (__FIf__->Emergency_Broadcasting<<3) + (TmData[PKTSIZE]&0x07);

					__FIf__->CheckAdaptation(&TmData[0], 204);

					//////////////////////////////////////////////////////////////////////
					//sskim20080725 - BERT
					#if 1
					if(((TsData[189] >> 4) & 0x0f) != 0x08 && ((TsData[189] >> 4) & 0x0f) != 0x04)
						__FIf__->TL_BERT_SET_DATA(TmData);
					#endif
					//////////////////////////////////////////////////////////////////////

					// FILL 2ND PLAY BUFFER(_b_sync)
					__FIf__->FillOnePacketToQueue(204, _b_sync, (int*)&__FIf__->TL_nWritePos, (int*)&__FIf__->TL_nReadPos, (int*)&__FIf__->TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TmData);

					memcpy(&BufferC[204 * nAddrC], TsData, 204);
					if(nAddrC < (nLenC-1)) 
					{
						nAddrC++;
					}
					else 
					{					
						nAddrC = 0;
					}
				}
				else 						// For Other Data
				{
					if((TsData[PKTSIZE] & 0x02) == 0x02) 
					{
						nTsWrCounter = 0;
					}

					memcpy(TmData, TsData, 204);
					TmData[190] = (TmData[190] & 0xe0) | ((nTsWrCounter >> 8) & 0x1f);
					TmData[191] = (nTsWrCounter & 0xff);

					//FIXED - ISDB-T Emergency Broadcasting Control
					if ( ((TsData[PKTSIZE+1] >> 4) & 0x0f) == 0x08 )
					{
						TmData[8] = (((TmData[8]>>2)&0x3F)<<2) + (__FIf__->Emergency_Broadcasting<<1) + (TmData[8]&0x01);
					}
					else if ( ((TsData[PKTSIZE+1] >> 4) & 0x0f) != 0x00 )
					{
						TmData[PKTSIZE] = (((TmData[PKTSIZE]>>4)&0x0F)<<4) + (__FIf__->Emergency_Broadcasting<<3) + (TmData[PKTSIZE]&0x07);
					}

					__FIf__->CheckAdaptation(&TmData[0], 204);

					//////////////////////////////////////////////////////////////////////
					//sskim20080725 - BERT
					#if 1
					if(((TsData[189] >> 4) & 0x0f) != 0x08 && ((TsData[189] >> 4) & 0x0f) != 0x04)
						__FIf__->TL_BERT_SET_DATA(TmData);
					#endif
					//////////////////////////////////////////////////////////////////////

					// FILL 2ND PLAY BUFFER(_b_sync)
					__FIf__->FillOnePacketToQueue(204, _b_sync, (int*)&__FIf__->TL_nWritePos, (int*)&__FIf__->TL_nReadPos, (int*)&__FIf__->TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TmData);

				}

				nCnt += 204;
				if(nTsWrCounter == 8191) 
				{
					nTsWrCounter = 0;
				}
				else 
				{
					nTsWrCounter++;
				}
			}
			else
			{
				//nCnt++;
				j = __FIf__->TL_SyncLockFunction((char*)RdData+nCnt, 
					nLenRd - 204 - nCnt, 
					(int*)&i, 
					nLenRd - 204 - nCnt, 
					3);
				if  ( j == -1 )
				{
					//sskim20080701 - fixed
					nCnt = 0;

					break;
				}

				nCnt += j;
			}
		}

		__FIf__->TL_nFindSync = nCnt;
		nCnt = 0;
		
	}
}


//////////////////////////////////////////////////////////////////////////////////////
long	CHldFmtrIsdbT13::PlayRate_IsdbT13(long play_freq_in_herz, long nOutputClockSource)
{

	if (__Sta__->IsState_IsdbT13LoopThru())
	{
		play_freq_in_herz = TS_BITRATE_ISDB_T_13;
	}
	return	play_freq_in_herz;
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldFmtrIsdbT13::ProcessStaMachineIsdbT13(int para_cmd, unsigned char *para_ptr, int para_any, unsigned char **para_ptr2)
{
	int	_ret;
	int	ReadPos, BufferCnt, EndOfRemux;

	_ret = 1;
	if (!__Sta__->IsModTyp_IsdbT_13())
	{
		return	_ret;
	}

	switch(_fmter_sta_)
	{
	case	_STA_ISDBT13_undefined:
		if (para_cmd == _CMD_ISDBT13_self_sta_chg)
		{
			if (__Sta__->IPUsing() && (para_any == _STA_ISDBT13_playback_ip_reqstarting))
			{
				ChgStaMachineIsdbT13(para_any);
			}
		}
		break;

	case	_STA_ISDBT13_normalfile_playback:
		break;

	case	_STA_ISDBT13_loopthru_188_onrestarting:
		if (__FIf__->TmccUsing())
		{
			if (para_cmd == _CMD_ISDBT13_capture_dta)
			{
				_fmter_sta_ = _STA_ISDBT13_loopthru_188_ongoing;	//	detected raw-ts data.
				TMCC_SET_188_LOOPTHRU_MODE(1);
				StopAndClose_Restart_TmccRemuxer();
			}
		}
		break;
	case	_STA_ISDBT13_loopthru_188_ongoing:	//	incomming raw-ts data.
		if (para_cmd == _CMD_ISDBT13_capture_dta)
		{
			DbgWrCaptureDta(para_any, para_ptr);
			TMCC_WR_188_LPTHRU_CAPTURE_DTA(para_any, para_ptr);
			_ret = 2;
		}
		else if (para_cmd == _CMD_ISDBT13_play_dta)
		{
			*para_ptr2 = (unsigned char *)TMCC_READ_REMUX_DATA(para_any, &ReadPos, &BufferCnt, &EndOfRemux);
			if (*para_ptr2 != NULL)
			{
				_ret = 2;
			}
		}
		break;
	case	_STA_ISDBT13_loopthru_188_onpause:	//	pause
		if (para_cmd == _CMD_ISDBT13_capture_dta)
		{
			_ret = 2;	//	discard capture dta.
		}
		else if (para_cmd == _CMD_ISDBT13_play_dta)
		{
			//	ignore request
		}
		break;
	case	_STA_ISDBT13_loopthru_188_onendreqed:
		StopAndClose_TmccRemuxer();
		TMCC_SET_188_LOOPTHRU_MODE(0);
		_fmter_sta_ = _STA_ISDBT13_undefined;
		DbgWrCaptureDta(0, NULL);
		break;

	case	_STA_ISDBT13_loopthru_initial_requsted:
		if (para_cmd == _CMD_ISDBT13_detected_pkt_size)
		{
			if (para_any == PKTSIZE)
			{
				if (__FIf__->TmccUsing())
				{
					_fmter_sta_ = _STA_ISDBT13_loopthru_188_ongoing;	//	detected raw-ts data.
					TMCC_SET_188_LOOPTHRU_MODE(1);
					StopAndClose_Restart_TmccRemuxer();
				}
			}
			else if (para_any == 204)
			{
				_fmter_sta_ = _STA_ISDBT13_loopthru_204_ongoing;	//	detected formatted data.
			}
		}
		break;

	case	_STA_ISDBT13_playback_ip_reqstarting:
		break;
	case	_STA_ISDBT13_playback_ip_ongoing:
		break;
	case	_STA_ISDBT13_playback_ip_reqend:
		break;

	default:
		break;
	}
	return	_ret;
}
void	CHldFmtrIsdbT13::ChgStaMachineIsdbT13(int _sta)
{
	switch(_sta)
	{
	case	_STA_ISDBT13_loopthru_188_onendreqed:
		if (IsLoopthru188_IsdbT13())
		{
			_fmter_sta_ = _sta;	//	_STA_ISDBT13_loopthru_188_onendreqed
			ProcessStaMachineIsdbT13(_CMD_ISDBT13_max, NULL, 0, NULL);
		}
		break;
	default:
		_fmter_sta_ = _sta;
		break;
	}
}
int	CHldFmtrIsdbT13::StaMachineIsdbT13(void)
{
	return	(int)_fmter_sta_;
}

int	CHldFmtrIsdbT13::IsLoopthru188_IsdbT13(void)
{
	int	_ret = 0;

	if ((_fmter_sta_ == _STA_ISDBT13_loopthru_188_ongoing) || (_fmter_sta_ == _STA_ISDBT13_loopthru_188_onpause))
	{
		_ret = 1;
	}
	return	_ret;
}
int	CHldFmtrIsdbT13::IsLoopthru188InputFullness_IsdbT13(void)
{
	int	_ret = 0;
	int AsiLock;
	if (IsLoopthru188_IsdbT13())
	{
		AsiLock = __FIf__->IsAsiInputLocked();
		if((AsiLock & 0x8) < 1)
			return -1;

		_ret = TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL();
	}
	return	_ret;
}
void	CHldFmtrIsdbT13::PauseLoopthru188_IsdbT13(unsigned char *_fn)
{
	if (IsLoopthru188_IsdbT13())
	{
		ChgStaMachineIsdbT13(_STA_ISDBT13_loopthru_188_onpause);
		if (_fn != NULL)
		{
			TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(_fn);
		}
	}
}

void	CHldFmtrIsdbT13::PauseLoopthru188_IsdbT13(struct _TSPH_TS_INFO **ppTsInfo)
{
	if (IsLoopthru188_IsdbT13())
	{
		ChgStaMachineIsdbT13(_STA_ISDBT13_loopthru_188_onpause);
		TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(ppTsInfo);
	}
}

void	CHldFmtrIsdbT13::Buf_Clear_Loopthru188_IsdbT13(void)
{
	if (IsLoopthru188_IsdbT13())
	{
		TMCC_init_InputDta188Pkt_fullness();
		//ChgStaMachineIsdbT13(_STA_ISDBT13_loopthru_188_onrestarting);
	}
}



