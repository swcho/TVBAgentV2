
#if defined(WIN32)
#include	<Windows.h>
#include	<process.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<unistd.h>
#include	<pthread.h>
#include	<time.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	"../include/hld_const.h"
#include	"../include/hld_structure.h"
#include	"hld_tmcc_rmx.h"
#include	"LLDWrapper.h"

//////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)
#else
#define false 0
#define true 1
#define FALSE false
#define TRUE  true

typedef long WORD;
typedef unsigned char BYTE;
#define Sleep(n)	do { usleep(n * 1000); } while (0)
#endif

#define	_PRESAVE_to_CALC_TSINFO_sec_		10		//	25 sec

#define	_MUXED_RSLT_BUFFER_SIZE_			(SUB_BANK_MAX_BYTE_SIZE*4)			//	4-Mbytes
#define	_MUXED_RSLT_BUFFER_SIZE_UsrSpace_	(SUB_BANK_MAX_BYTE_SIZE*2)			//	2-Mbytes
//#define	_CAPTURED_INPUT_BUFFER_SIZE__max_	(TMCC_REMUXER_PRE_BUFFER_SIZE*2)	//	64-Mbytes. 10-sec for 50 Mbps-ts
#define	_CAPTURED_INPUT_BUFFER_SIZE__max_	(TMCC_REMUXER_PRE_BUFFER_SIZE)	//	32-Mbytes. 5-sec for 50 Mbps-ts
#define	_CAPTURED_INPUT_BUFFER_SIZE__min_	(_MUXED_RSLT_BUFFER_SIZE_*1)		//	8-Mbytes. 10-sec for 6.4 Mbps-ts
#define	_MUXED_RSLT_BUFFER_SIZE__highlevel_	(_MUXED_RSLT_BUFFER_SIZE_*4/5)
	//	_CAPTURED_INPUT_BUFFER_SIZE__min_ and _MUXED_RSLT_BUFFER_SIZE__highlevel_ are relative to the level of cnt_byte_availabe_caped in CHldFmtrLoopThru()

static const char	*Constellation[4]	= { "DQPSK", "QPSK", "16QAM", "64QAM" };
static const char	*Coderate[5]			= { "1/2", "2/3", "3/4", "5/6", "7/8" };
static const char	*BroadcastType[3]	= { "TV", "RADIO-1", "RADIO-3" };

//////////////////////////////////////////////////////////////////////////////////////
CHldTmccRmx::CHldTmccRmx(int _my_id, void *_hld)
{
	int	i;

	my_hld_id = _my_id;
	my_hld = _hld;

	_188_loopthru_mode = 0;

	TL_nPositionChanged = 0;
	TL_a_low = 0;
	TL_a_high = 0;
	TL_ThreadDone = 1;
	TL_EndOfRemux = 1;

	RsltDta_ReadByHwWriter = NULL;
	TL_nWritePos = 0;
	TL_nReadPos = 0;
	TL_nBufferCnt = 0;
	RsltDta_UsrSpace = NULL;

	g_hMutex = NULL;
	gOutputFile = NULL;
	gEndOfFile = false;    
	gPacketSize = 188;
	gOptionIndex = 0; 	
	gLayerOtherPidMap = -1;//0=Layer A, 1=Layer B, 2=Layer C, 3(-1)=Ignore
	gLayerMultiPidMap = 0;

	for ( i = 0; i < 4; i++ )
	{
		gLayerPidInfo[i] = NULL;
		gLayerPidCount[i] = 0;
	}

	InputDta188Pkt_WrByHwCapturer = NULL;
	InputDta188Pkt_WrPos = 0;
	for (i = 0; i < 3; i++)
	{
		InputDta188Pkt_RdPos[i] = 0;
		InputDta188Pkt_Cnt[i] = 0;
	}
	InputDta188Pkt_initial_sync_pos = 0;
	InputDta188Pkt_padding_cond = 0;
	InputDta188Pkt_fullness = 0;
	InputDta188Pkt_fullness_Criteria = _CAPTURED_INPUT_BUFFER_SIZE__max_;
	InputDta188Pkt_SaveFs = NULL;

	for (i = 0; i < 3; i++)
	{
		gBF[i]=NULL;
		gRD[i]=0;
		gWD[i]=0;
		gCT[i]=0;
		gBF2[i]=NULL;
		gRD2[i]=0;
		gWD2[i]=0;
		gCT2[i]=0;
		gwPID[i]=-1;
		gPkt0[i]=0;
		gPkt1[i]=0;
		giiPCR0[i]=0;
		giiPCR1[i]=0;
		gBF_layered[i]=NULL;
		gRD_layered[i]=0;
		gWD_layered[i]=0;
		gCT_layered[i]=0;
	}
	gDummyCount = 0;
	gDummyTotal = 0;
	gDummyBufPos = 0;
	gDummyBufLen = 0;
	gDummyBuf = NULL;
	gUseLayeredBuffer = 0;
	gData_Read = 0;
	gData_Write = 0;
	gData_Count = 0;

#ifdef _SEARCH_PCR_
	gTime = 0.;
	a_pcr_0 = 0;
	a_pcr_1 = 0;
	a_ts = 0;
	a_ts_add = 0;
	a_pcr_pid = -1;
#endif

	for (i = 0; i < 3; i++)
	{
#if defined(WIN32)
		gLayerFile[i] = INVALID_HANDLE_VALUE;
#else
		gLayerFile[i] = NULL;
#endif
	}
}
CHldTmccRmx::~CHldTmccRmx()
{

}
void	CHldTmccRmx::CreateMutex__(void)
{
#if defined(WIN32)
	g_hMutex = CreateMutex(NULL, FALSE, "TMCC_SHARED_MUTEX");
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		g_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "TMCC_SHARED_MUTEX");
	}
#else
	pthread_mutexattr_init(&TL_hMutexAttr);
	pthread_mutexattr_settype(&TL_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_hMutex, &TL_hMutexAttr);
#endif
}
void	CHldTmccRmx::DestroyMutex__(void)
{
#if defined(WIN32)
	if ( g_hMutex )
	{
		ReleaseMutex(g_hMutex);
		CloseHandle(g_hMutex);
		g_hMutex = NULL;
	}
#else
	pthread_mutexattr_destroy(&TL_hMutexAttr);
	pthread_mutex_destroy(&TL_hMutex);
#endif
}
void	CHldTmccRmx::LockMutex__(void)
{
#if defined(WIN32)
	if ( g_hMutex )
	{
		WaitForSingleObject(g_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_hMutex);
#endif
}
void	CHldTmccRmx::UnlockMutex__(void)
{
#if defined(WIN32)
	if ( g_hMutex )
	{
		ReleaseMutex(g_hMutex);
	}
#else
	pthread_mutex_unlock(&TL_hMutex);
#endif
}

int	CHldTmccRmx::IsValidHandle(int lay_ind)
{
	if (_188_loopthru_mode)
	{
		return	1;
	}
	CHld	*_hld_ = (CHld *)my_hld;
	//2012/9/28 bert fixed
	int nBertType = _hld_->_FIf->_VarTstPktTyp();
	if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23)
	{
		return 1;
	}
#if defined(WIN32)
	if ( gLayerFile[lay_ind] == INVALID_HANDLE_VALUE )
	{
		return 0;
	}
#else
	if ( (FILE*)gLayerFile[lay_ind] == NULL )
	{
		return 0;
	}
#endif
	return 1;
}
void	CHldTmccRmx::CloseTs(int _lay)
{
#if defined(WIN32)
	if ( gLayerFile[_lay] != INVALID_HANDLE_VALUE )
	{
		CloseHandle(gLayerFile[_lay]);
		gLayerFile[_lay] = INVALID_HANDLE_VALUE;
	}
#else
	if ( gLayerFile[_lay] != NULL )
	{
		fclose((FILE*)gLayerFile[_lay]);
		gLayerFile[_lay] = NULL;
	}
#endif
}
int	CHldTmccRmx::OpenTs(char *szFilePath)
{
	int	i, _ret;

	for (i = 0; i < 3; i++ )
	{
		if (_188_loopthru_mode)
		{
			goto	_init_buffer;
		}
		
#if defined(WIN32)
		gLayerFile[i] = CreateFile(
				szFilePath, 
				GENERIC_READ, 
				FILE_SHARE_READ, 
				NULL, 
				OPEN_EXISTING, 
				FILE_FLAG_NO_BUFFERING, 
				NULL);
		if ( gLayerFile[i] == INVALID_HANDLE_VALUE )
		{
			return -1;
		}
#else
		gLayerFile[i] = fopen(szFilePath, "rb");
		if ( gLayerFile[i] == NULL )
		{
			return -1;
		}
#endif

_init_buffer:
		_ret = AllocInterBuf(i);
		if (_ret < 0)
		{
			return	_ret;
		}
	}

	fraction_accumulation = 0;


	return	1;
}
void	CHldTmccRmx::SeekTs(int _lay, long _low, long *_high, long _high_, long _low_)	//	off_t
{
	if (gLayerFile[_lay] == NULL)	return;

#if defined(WIN32)
	SetFilePointer(gLayerFile[_lay], _low, _high, FILE_BEGIN);
#else
	off_t	_off;
	_off = ((off_t)_high_<<32) | (off_t)_low_;
	fseeko((FILE*)gLayerFile[_lay], (off_t)_off, SEEK_SET);
#endif
}
int CHldTmccRmx::Fread(int _lay, unsigned char *_buf, unsigned long _size, unsigned long *_red_size)
{
	int	nRet;

	if (_188_loopthru_mode)
	{
		*_red_size = (unsigned long)RdDtaFromInputBuffer(_lay, (int)_size, _buf);
		return	1;
	}

#if defined(WIN32)
	nRet = ReadFile(gLayerFile[_lay], _buf, _size, _red_size, NULL);	
#else
	nRet = 1;
	*_red_size = fread(_buf, 1, _size, (FILE*)gLayerFile[_lay]);
#endif
	return	nRet;
}
void	CHldTmccRmx::SetEof(int idx)
{
	if (_188_loopthru_mode)
	{
		return;
	}

	if (idx != 0 && IsValidHandle(idx))
	{
		if(TL_InParams.m_Layers[0].m_NumberOfSegments == 0)
			gEndOfFile = true;
		else
			SeekTs(idx, 0, NULL, 0, 0);
	}
	else
	{
		gEndOfFile = true;
	}
}
int	CHldTmccRmx::AllocDummyBuf(int size)
{
	gDummyBuf = (unsigned char*)malloc(size);
	if ( !gDummyBuf )
	{
		return	0;
	}
	memset(gDummyBuf, 0x00, size);
	return	size;
}
void	CHldTmccRmx::FreeDummyBuf(void)
{
	if ( gDummyBuf )
	{
		free(gDummyBuf);
		gDummyBuf = NULL;
	}
	gDummyBufPos = gDummyBufLen = 0;
}
void	CHldTmccRmx::AllocRsltBuf(void)
{
#if defined(WIN32)
	RsltDta_ReadByHwWriter = (unsigned char*)VirtualAlloc(NULL, _MUXED_RSLT_BUFFER_SIZE_, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
	RsltDta_ReadByHwWriter = (unsigned char*)malloc(_MUXED_RSLT_BUFFER_SIZE_);
#endif
	TL_nWritePos = 0;
	TL_nReadPos = 0;
	TL_nBufferCnt = 0;

	RsltDta_UsrSpace = (unsigned char*)malloc(_MUXED_RSLT_BUFFER_SIZE_UsrSpace_);
}
void	CHldTmccRmx::FreeRsltBuf(void)
{
#if defined(WIN32)
	if ( RsltDta_ReadByHwWriter )
	{
		VirtualFree(RsltDta_ReadByHwWriter, 0 , MEM_RELEASE);
		RsltDta_ReadByHwWriter = NULL;
	}
#else
	if ( RsltDta_ReadByHwWriter )
	{
		free(RsltDta_ReadByHwWriter);
		RsltDta_ReadByHwWriter = NULL;
	}
#endif
	if (RsltDta_UsrSpace)
	{
		free(RsltDta_UsrSpace);
		RsltDta_UsrSpace = NULL;
	}
}
void	CHldTmccRmx::AllocInputBuf(void)
{
	InputDta188Pkt_WrByHwCapturer = (unsigned char*)malloc(_CAPTURED_INPUT_BUFFER_SIZE__max_);
	InputDta188Pkt_WrPos = 0;
	for (int i = 0; i < 3; i++)
	{
		InputDta188Pkt_RdPos[i] = 0;
		InputDta188Pkt_Cnt[i] = 0;
	}
	InputDta188Pkt_initial_sync_pos = 0;
	InputDta188Pkt_padding_cond = 0;
	InputDta188Pkt_fullness = 0;
	InputDta188Pkt_fullness_Criteria = _CAPTURED_INPUT_BUFFER_SIZE__max_;
	InputDta188Pkt_SaveFs = (unsigned char*)malloc(_CAPTURED_INPUT_BUFFER_SIZE__max_);

	memset(InputDta188Pkt_WrByHwCapturer, 0xff, _CAPTURED_INPUT_BUFFER_SIZE__max_);
}
void	CHldTmccRmx::FreeInputBuf(void)
{
	if (InputDta188Pkt_WrByHwCapturer)
	{
		free(InputDta188Pkt_WrByHwCapturer);
		InputDta188Pkt_WrByHwCapturer = NULL;
	}
	if (InputDta188Pkt_SaveFs)
	{
		free(InputDta188Pkt_SaveFs);
		InputDta188Pkt_SaveFs = NULL;
	}
}
int	CHldTmccRmx::AllocInterBuf(int _lay)
{
	gBF[_lay] = (unsigned char*)malloc(MAX_BUFFERED_READ_SIZE);
	if ( !gBF[_lay] )
	{
		return -2;
	}
	memset(gBF[_lay], 0x00, MAX_BUFFERED_READ_SIZE);
	gRD[_lay] = gWD[_lay] = gCT[_lay] = 0;
	
	gBF2[_lay] = (unsigned char*)malloc(MAX_BUFFERED_READ_SIZE);
	if ( !gBF2[_lay] )
	{
		return -3;
	}
	memset(gBF2[_lay], 0x00, MAX_BUFFERED_READ_SIZE);
	gRD2[_lay] = gWD2[_lay] = gCT2[_lay] = 0;
	gwPID[_lay] = -1;
	gPkt0[_lay]=0;
	gPkt1[_lay]=0;
	giiPCR0[_lay]=0;
	giiPCR1[_lay]=0;
	
	gBF_layered[_lay] = (unsigned char*)malloc(MAX_BUFFER_BYTE_SIZE_layered);
	if ( !gBF_layered[_lay] )
	{
		return -4;
	}
	memset(gBF_layered[_lay], 0x00, MAX_BUFFER_BYTE_SIZE_layered);
	gRD_layered[_lay] = gWD_layered[_lay] = gCT_layered[_lay] = 0;
	
	gData_Read = gData_Write = gData_Count = 0;

	return	1;
}
void	CHldTmccRmx::FreeInterBuf(int _lay)
{
	if ( gBF[_lay] )
	{
		free(gBF[_lay]);
		gBF[_lay] = NULL;
	}
	if ( gBF2[_lay] )
	{
		free(gBF2[_lay]);
		gBF2[_lay] = NULL;
	}
	if ( gBF_layered[_lay] )
	{
		free(gBF_layered[_lay]);
		gBF_layered[_lay] = NULL;
	}
	gUseLayeredBuffer = 0;
}

int	CHldTmccRmx::TL_SyncLockFunction(char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step) 
{
	int		i, j;
	bool	err;

	if (nlen < nlen_srch)
		return -1;

	for (i = 0; i < nlen_srch; i++)
	{
		if (szBuf[i] == 0x47) {

			//-----------------------------------------------
			// check if 188 bytes TS Stream ?
			err = false;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*188 + i] != 0x47) {
					err = true;
					break;
				}
			}

			if (err == false) {
				*iSize = 188;
				return i;
			}

			//-----------------------------------------------
			// check if 204 bytes TS Stream ?
			err = false;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*204 + i] != 0x47) {
					err = true;
					break;
				}
			}

			if (err == false) {
				*iSize = 204;
				return i;
			}

			//-----------------------------------------------
			// check if 208 bytes TS Stream ?
			err = false;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*208 + i] != 0x47) {
					err = true;
					break;
				}
			}

			if (err == false) {
				*iSize = 208;
				return i;
			}

		}

	}

	return -1;
}
int	CHldTmccRmx::FindSyncPosAtLayerBuf_and_SetgRDVal(int idx)	//	layer index
{
	int	i, j, lTemp, ret_pos;

	if ( gRD[idx] <= MAX_BUFFERED_READ_SIZE - gCT[idx] )	//	no wrap around.
	{
		j = TL_SyncLockFunction((char*)gBF[idx] + gRD[idx], 	gCT[idx], (int*)&i, gCT[idx], 3);
		if ( j != -1 )	//	ok found sync position.
		{
			gRD[idx] += j;
			if (gRD[idx] >= MAX_BUFFERED_READ_SIZE)
			{
				gRD[idx] -= MAX_BUFFERED_READ_SIZE;
			}
			gCT[idx] -= j;
			ret_pos = j;
		}
		else
		{
			gRD[idx] += gCT[idx];
			gCT[idx] = 0;
			ret_pos = -2;	//	need refill data
		}
	}
	else	//	wrap around
	{
		lTemp = MAX_BUFFERED_READ_SIZE - gRD[idx];
	
		j = TL_SyncLockFunction((char*)gBF[idx] + gRD[idx], 	lTemp, (int*)&i, lTemp, 3);
		if ( j != -1 )	//	ok found sync position.
		{
			gRD[idx] += j;
			if (gRD[idx] >= MAX_BUFFERED_READ_SIZE)
			{
				gRD[idx] -= MAX_BUFFERED_READ_SIZE;
			}
			gCT[idx] -= j;
			ret_pos = j;
		}
		else	//	fail to find sync position
		{
			gRD[idx] = 0;	//	set first position
			gCT[idx] -= lTemp;
	
			j = TL_SyncLockFunction((char*)gBF[idx] + gRD[idx], gCT[idx], (int*)&i, gCT[idx], 3);
			if ( j != -1 )	//	ok found sync position.
			{
				gRD[idx] += j;
				if (gRD[idx] >= MAX_BUFFERED_READ_SIZE)
				{
					gRD[idx] -= MAX_BUFFERED_READ_SIZE;
				}
				gCT[idx] -= j;
				ret_pos = j;
			}
			else
			{
				gRD[idx] += gCT[idx];
				gCT[idx] = 0;
				ret_pos = -2;	//	need refill data
			}
		}
	}
	return	ret_pos;
}
int CHldTmccRmx::GET_PCR(unsigned char* pPacket, __int64* pPCR)
{
	WORD	wPID;
	BYTE	bADExist;
	BYTE	bADLen;
	__int64	iiPCR = 0;

	*pPCR = 0;
	wPID = (pPacket[1]&0x1F)*256+pPacket[2];

	bADExist = pPacket[3] & 0x20;
	bADLen   = pPacket[4];

	if (bADExist && (bADLen >=7) )
	{
		if (pPacket[5] & 0x10)
		{
			//--- Get PCR Value
			iiPCR = pPacket[6];		
			iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
			iiPCR += pPacket[7] << 17;
			iiPCR += pPacket[8] << 9;
			iiPCR += pPacket[9] << 1;
			iiPCR += (pPacket[10] & 0x80) >> 7;
			iiPCR *= 300;

			iiPCR += (pPacket[10] & 0x01) << 8;
			iiPCR += pPacket[11];

			*pPCR = iiPCR;
		}
	}

	return wPID;
}

void CHldTmccRmx::ReinitAtScroll(void)
{
	int	i;

	if ( TL_nPositionChanged == 1 )
	{
		TL_nPositionChanged = 0;
		for ( i = 0; i < 3; i++ )
		{
			gRD[i] = gWD[i] = gCT[i] = 0;
			gRD2[i] = gWD2[i] = gCT2[i] = 0;
			gwPID[i] = -1;
			gPkt0[i]=0;
			gPkt1[i]=0;
			giiPCR0[i]=0;
			giiPCR1[i]=0;
		}
		gDummyBufPos = gDummyBufLen = 0;

		SeekTs(0, TL_a_low, &TL_a_high, TL_a_high, TL_a_low);
		SeekTs(1, TL_a_low, &TL_a_high, TL_a_high, TL_a_low);
		SeekTs(2, TL_a_low, &TL_a_high, TL_a_high, TL_a_low);
	}
}
void CHldTmccRmx::TsAttrPktSize(void)
{
	int	i, nRet;
	unsigned long	dwRead=0;
	char	*szBuf;
	CHld	*_hld_ = (CHld *)my_hld;
	int nBertType = _hld_->_FIf->_VarTstPktTyp();
	if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23)
	{
		gPacketSize = 188;
		return;
	}

	if (_188_loopthru_mode)
	{
		gPacketSize = 188;
		return;
	}

	szBuf = (char*)malloc(0x100000);
	for (i = 0; i < 3; i++ )
	{
		if ( IsValidHandle(i) )
		{
			nRet = Fread(i, (unsigned char *)szBuf, SUB_BANK_MAX_BYTE_SIZE, &dwRead);
			TL_SyncLockFunction(szBuf, 0x100000, &gPacketSize, 0x100000, 3);
			SeekTs(i, 0, NULL, 0, 0);
			break;
		}
	}
	free(szBuf);
}
int CHldTmccRmx::RdOneRawPktFromLayerBuffer(int lay_idx, unsigned char *_buf, int _rd_pos, int _pkt_size)
{
	int	lTemp;

	if ( _rd_pos <= MAX_BUFFERED_READ_SIZE - _pkt_size )
	{
		memcpy(_buf, gBF[lay_idx] + _rd_pos, _pkt_size);
	}
	else
	{
		lTemp = MAX_BUFFERED_READ_SIZE - _rd_pos;
		memcpy((char*)_buf, gBF[lay_idx] + _rd_pos, lTemp);
		memcpy((char*)_buf + lTemp, gBF[lay_idx], _pkt_size - lTemp);
	}
	return	_rd_pos;
}
int CHldTmccRmx::RdOneSyncPktFromLayerBuffer(int lay_idx, unsigned char *_buf, int _pkt_size)
{
	RdOneRawPktFromLayerBuffer(lay_idx, _buf, gRD[lay_idx], _pkt_size);

	gRD[lay_idx] += _pkt_size;
	if (gRD[lay_idx] >= MAX_BUFFERED_READ_SIZE)
	{
		gRD[lay_idx] -= MAX_BUFFERED_READ_SIZE;
	}
	gCT[lay_idx] -= _pkt_size;

	return	_pkt_size;
}
void CHldTmccRmx::QueueLayerBufferFromGivenBuf(int lay_idx, unsigned char *buffer, unsigned long size)
{
	int	lTemp;

	if (gWD[lay_idx] < (int)(MAX_BUFFERED_READ_SIZE - size) )
	{
		memcpy(gBF[lay_idx] + gWD[lay_idx], buffer, size);
		gWD[lay_idx] += size;
	}
	else
	{
		lTemp = MAX_BUFFERED_READ_SIZE - gWD[lay_idx];
		memcpy(gBF[lay_idx] + gWD[lay_idx], buffer, lTemp);
		memcpy(gBF[lay_idx], buffer + lTemp, size - lTemp);
//		memcpy(gBF[lay_idx], buffer, size-lTemp);
		gWD[lay_idx] = size - lTemp;
	}
	gCT[lay_idx] += size;
}
void CHldTmccRmx::QueueLayerBufferFromDummyBuf(int lay_idx, int _pkt_size)
{
	unsigned long dwRead = 0;

	if ( gCT[lay_idx] < _pkt_size )
	{
		dwRead = SUB_BANK_MAX_BYTE_SIZE;
		if ( dwRead > (unsigned long)(gDummyBufLen - gDummyBufPos) )
		{
			dwRead = (unsigned long)(gDummyBufLen - gDummyBufPos);
		}

		QueueLayerBufferFromGivenBuf(lay_idx, gDummyBuf + gDummyBufPos, dwRead);

		gDummyBufPos += dwRead;
		if ( gDummyBufPos >= gDummyBufLen-1 )	//	consumme all data
		{
			FreeDummyBuf();
		}
	}
}
int CHldTmccRmx::QueueLayerBufferFromRedTsData(int lay_idx)	//	0/1/2
{
	unsigned char *data_buffer = NULL;
	unsigned long dwRead = 0;
	int	nRet;

	data_buffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE*sizeof(unsigned char));
	if ( !data_buffer )
	{
		return 0;
	}
	memset(data_buffer, 0x00, SUB_BANK_MAX_BYTE_SIZE);
	
	nRet = Fread(lay_idx, data_buffer, SUB_BANK_MAX_BYTE_SIZE, &dwRead);
	if ( nRet == 1 && dwRead == 0 ) //	EOF
	{
		if ( data_buffer ) 
		{
			free(data_buffer);
			data_buffer = NULL;
		}
		return 0;
	}
	QueueLayerBufferFromGivenBuf(lay_idx, data_buffer, dwRead);
	if ( data_buffer ) 
	{
		free(data_buffer);
		data_buffer = NULL;
	}
	return	SUB_BANK_MAX_BYTE_SIZE;
}
int CHldTmccRmx::ChkDtaAvailable(int _lay)
{
	if (_188_loopthru_mode)
	{
		switch(InputDta188Pkt_padding_cond)
		{
		case	0:	//	not enough dta.
			if (InputDta188Pkt_Cnt[_lay] >= (InputDta188Pkt_fullness_Criteria*3/4))
			{
				InputDta188Pkt_padding_cond = 1;
			}
			return	0;
		case	1:
//			break;	//	donot toggle
			if (InputDta188Pkt_Cnt[_lay] < (InputDta188Pkt_fullness_Criteria*1/4))
			{
				InputDta188Pkt_padding_cond = 0;
				return	0;
			}
			break;
		default:
			break;
		}
	}
	return	1;
}
void CHldTmccRmx::InitDummyInsertCond(void)
{
	int	i;

	gDummyCount = gDummyTotal = 0;
	for ( i = 0; i < DUMMY_CNT; i++ )
	{
		gDummyPkt[i] = gDummyPos[i] = 0;
	}
}
int CHldTmccRmx::ReformatOrigTs_w_DummyPkt(int idx, int _pkt_size)
{
	int	i, j, k;
	int	lTemp;
	unsigned char  null_tp[204];

	memset( null_tp, 0xFF, 204);						
	null_tp[0] = 0x47;
	null_tp[1] = 0x1F;
	null_tp[2] = 0xFF;
	null_tp[3] = idx;
	null_tp[4] = idx;
	null_tp[5] = idx;

	for ( i = 0; i < gDummyCount; i++ )
	{
		gDummyTotal += (gDummyPkt[i]*_pkt_size);
	}
	if ( gDummyTotal > 0 )
	{
		k = gDummyTotal + gCT[idx];	//	total cnt. dummy and real data to process.
		if (AllocDummyBuf(k*sizeof(unsigned char)) == 0)
		{
			return	0;
		}
	}
	gDummyBufPos = gDummyBufLen = 0;	//	0 start from.
	
	if ( gDummyBuf == NULL )
	{
		gDummyCount = 0;
	}
	
	for ( i = 0; i < gDummyCount; i++ )
	{
		if ( i == 0 )	//	fill real data. which is the 1st hit pkt matching to current pid-list.
		{
			if ( gRD[idx] <= gDummyPos[i] )
			{
				lTemp = gDummyPos[i] - gRD[idx];
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gRD[idx], lTemp);
				gDummyBufLen += lTemp;
			}
			else
			{
				lTemp = MAX_BUFFERED_READ_SIZE - gRD[idx];
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gRD[idx], lTemp);
				gDummyBufLen += lTemp;
	
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx], gDummyPos[i]);
				gDummyBufLen += gDummyPos[i];
			}
		}
	
		for ( j = 0; j < gDummyPkt[i]; j++ )	//	fill null data.
		{
			memcpy(gDummyBuf + gDummyBufLen, null_tp, _pkt_size);
			gDummyBufLen += _pkt_size;
		}
	
		if ( i < gDummyCount-1 )
		{
			lTemp = gDummyPos[i+1] - gDummyPos[i];	//	cnt of pkt between two pcrs.
			if ( lTemp >= 0 )
			{
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gDummyPos[i], lTemp);	//	fill real pkt between two pcrs.
				gDummyBufLen += lTemp;
			}
			else
			{
				lTemp = MAX_BUFFERED_READ_SIZE - gDummyPos[i];
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gDummyPos[i], lTemp);
				gDummyBufLen += lTemp;
	
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx], gDummyPos[i+1]);
				gDummyBufLen += gDummyPos[i+1];
			}
		}
		else	//	last one. copy(consume) all real data in the gRF.
		{
			if ( gWD[idx] > gDummyPos[i] && gWD[idx] < MAX_BUFFERED_READ_SIZE )
			{
				lTemp = gWD[idx] - gDummyPos[i];
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gDummyPos[i], lTemp);
				gDummyBufLen += lTemp;
			}
			else
			{
				lTemp = MAX_BUFFERED_READ_SIZE - gDummyPos[i];
				memcpy(gDummyBuf + gDummyBufLen, gBF[idx] + gDummyPos[i], lTemp);
				gDummyBufLen += lTemp;
	
				if ( gWD[idx] > 0 )
				{
					memcpy(gDummyBuf + gDummyBufLen, gBF[idx], gWD[idx]);
					gDummyBufLen += gWD[idx];
				}
			}

////////////////////////////////////////////////////////////////////	now. move the mixed data into gRF2.
////////////////////////////////////////////////////////////////////	the size of gRF2 is MAX_BUFFERED_READ_SIZE
////////////////////////////////////////////////////////////////////	so, somtimes useful data may be remained in gDummyBuf.
			if ( gDummyBufLen >= MAX_BUFFERED_READ_SIZE - _pkt_size )
			{
				lTemp = MAX_BUFFERED_READ_SIZE - gRD2[idx];
				memcpy(gBF2[idx] + gRD2[idx], gDummyBuf, lTemp);
				memcpy(gBF2[idx], gDummyBuf + lTemp, MAX_BUFFERED_READ_SIZE-lTemp);
				gDummyBufPos = MAX_BUFFERED_READ_SIZE;
	
				gCT2[idx] = MAX_BUFFERED_READ_SIZE;
				gWD2[idx] = gRD2[idx] + gCT2[idx];
				if ( gWD2[idx] >= MAX_BUFFERED_READ_SIZE )
				{
					gWD2[idx] -= MAX_BUFFERED_READ_SIZE;
				}
			}
			else	//	we can copy all gDummyBuf data into gBF2.
			{
				if ( gRD2[idx] <= MAX_BUFFERED_READ_SIZE - gDummyBufLen )
				{
					memcpy(gBF2[idx] + gRD2[idx], gDummyBuf, gDummyBufLen);
				}
				else
				{
					lTemp = MAX_BUFFERED_READ_SIZE - gRD2[idx];
					memcpy(gBF2[idx] + gRD2[idx], gDummyBuf, lTemp);
					memcpy(gBF2[idx], gDummyBuf + lTemp, gDummyBufLen-lTemp);
				}
	
				gWD2[idx] += gDummyTotal;
				if ( gWD2[idx] >= MAX_BUFFERED_READ_SIZE )
				{
					gWD2[idx] -= MAX_BUFFERED_READ_SIZE;
				}
				gCT2[idx] += gDummyTotal;
	
				FreeDummyBuf();
			}
		}
	}
	
	return	_pkt_size;
}
int CHldTmccRmx::CalcBitrateAndDummyCond(int lay_idx, int rd_pos, int pkt_size, long pid_of_this_pkt, __int64 val_pcr_of_this_pkt)
{
	int	i;
	double tmp_bitrate = 0., tmp = 0.;

	for (i = 0; i < gLayerPidCount[lay_idx]; i++ )	//	iteration to search pid matching with given list.
	{
		if ( gLayerPidInfo[lay_idx][i] == pid_of_this_pkt )	// pid_of_this_pkt is one of the given pid-list.
		{
			if ((val_pcr_of_this_pkt > 0) && (gwPID[lay_idx] == -1 || gwPID[lay_idx] == pid_of_this_pkt))	//	found same pid and pcr-val.
			{
				if ( giiPCR0[lay_idx] == 0 )	//	initial val
				{
					giiPCR0[lay_idx] = val_pcr_of_this_pkt;
					giiPCR1[lay_idx] = val_pcr_of_this_pkt;
					gwPID[lay_idx] = pid_of_this_pkt;
					gPkt0[lay_idx] = 0;
					gPkt1[lay_idx] = 0;
				}
				else	//	already encountered pcr-val at least one time.
				{
					if ( val_pcr_of_this_pkt > giiPCR1[lay_idx] )	//	already encountered pcr-val at least two times.
					{
						giiPCR0[lay_idx] = giiPCR1[lay_idx];	//	prev.
						giiPCR1[lay_idx] = val_pcr_of_this_pkt;	//	current
						gPkt1[lay_idx] = gPkt0[lay_idx];	//	how many pkts accumulated?
						gPkt0[lay_idx] = 0;
	
						if ( giiPCR1[lay_idx] > giiPCR0[lay_idx] )
						{
							tmp_bitrate = (((gPkt1[lay_idx] - gPkt0[lay_idx])*pkt_size*8.*27000000.) / (giiPCR1[lay_idx] - giiPCR0[lay_idx]));
						}
	
						if ( tmp_bitrate > 100 && gDummyCount < DUMMY_CNT )
						{
							tmp = ((TL_OutParams.m_Layers[lay_idx].m_Bps/1000.) * ((giiPCR1[lay_idx]-giiPCR0[lay_idx])/27000.) / (188.*8.));

							int num_packet = (int)tmp;
							fraction_accumulation += tmp - num_packet*1.0;
							if (fraction_accumulation > 1.0)
							{
								num_packet += 1;
								fraction_accumulation -= 1.0;

							}

							gDummyPkt[gDummyCount] = num_packet - gPkt1[lay_idx];				

	
							if ( gDummyPkt[gDummyCount] > 0 )	//	need insert null.
							{
								gDummyPos[gDummyCount] = rd_pos;
								++gDummyCount;
							}
							else
							{
								gDummyPkt[gDummyCount] = 0;
							}
						}
					}
					else
					{
						giiPCR0[lay_idx] = val_pcr_of_this_pkt;
						giiPCR1[lay_idx] = val_pcr_of_this_pkt;
						gwPID[lay_idx] = pid_of_this_pkt;
						gPkt0[lay_idx] = 0;
						gPkt1[lay_idx] = 0;
					}
				}
			}
			++gPkt0[lay_idx];
			break;
		}
	}
	return	0;
}
int CHldTmccRmx::RdDtaFromRsltBuffer(int _rd_size)
{
	unsigned int	lTemp;

	if (TL_nReadPos <= (unsigned int)((_MUXED_RSLT_BUFFER_SIZE_ - _rd_size)))
	{
		memcpy(RsltDta_UsrSpace, RsltDta_ReadByHwWriter + TL_nReadPos, _rd_size);
	}
	else 
	{
		lTemp = _MUXED_RSLT_BUFFER_SIZE_-TL_nReadPos;

		memcpy(RsltDta_UsrSpace, RsltDta_ReadByHwWriter+TL_nReadPos, lTemp);
		memcpy(RsltDta_UsrSpace + lTemp, RsltDta_ReadByHwWriter, _rd_size - lTemp);
	} 
	TL_nReadPos += _rd_size;
	if (TL_nReadPos >= _MUXED_RSLT_BUFFER_SIZE_)
		TL_nReadPos -= _MUXED_RSLT_BUFFER_SIZE_;

	TL_nBufferCnt -= _rd_size;

	return	_rd_size;
}
int CHldTmccRmx::WrDtaIntoRsltBuffer(int _wr_size, unsigned char *_wr_dta)
{
	unsigned int	lTemp;

	if (TL_nWritePos <= (unsigned int)((_MUXED_RSLT_BUFFER_SIZE_ - _wr_size)))
	{
		memcpy(RsltDta_ReadByHwWriter + TL_nWritePos, _wr_dta, _wr_size);
	
		TL_nWritePos += _wr_size;
	}
	else 
	{
		lTemp = _MUXED_RSLT_BUFFER_SIZE_ - TL_nWritePos;
	
		memcpy(RsltDta_ReadByHwWriter + TL_nWritePos, _wr_dta, lTemp);
		memcpy(RsltDta_ReadByHwWriter, _wr_dta + lTemp, _wr_size - lTemp);
	
		TL_nWritePos += _wr_size;
		if (TL_nWritePos >= _MUXED_RSLT_BUFFER_SIZE_)
			TL_nWritePos -= _MUXED_RSLT_BUFFER_SIZE_;
	}
	TL_nBufferCnt += _wr_size;

	return	_wr_size;
}
int CHldTmccRmx::RdDtaFromInputBuffer(int _lay, int _rd_size, unsigned char *_rd_dta)
{
	unsigned int	lTemp;

	if (InputDta188Pkt_RdPos[_lay] <= (unsigned int)((_CAPTURED_INPUT_BUFFER_SIZE__max_ - _rd_size)))
	{
		memcpy(_rd_dta, InputDta188Pkt_WrByHwCapturer + InputDta188Pkt_RdPos[_lay], _rd_size);
	}
	else 
	{
		lTemp = _CAPTURED_INPUT_BUFFER_SIZE__max_ - InputDta188Pkt_RdPos[_lay];

		memcpy(_rd_dta, InputDta188Pkt_WrByHwCapturer + InputDta188Pkt_RdPos[_lay], lTemp);
		memcpy(_rd_dta + lTemp, InputDta188Pkt_WrByHwCapturer, _rd_size - lTemp);
	} 
	InputDta188Pkt_RdPos[_lay] += _rd_size;
	if (InputDta188Pkt_RdPos[_lay] >= _CAPTURED_INPUT_BUFFER_SIZE__max_)
		InputDta188Pkt_RdPos[_lay] -= _CAPTURED_INPUT_BUFFER_SIZE__max_;

	InputDta188Pkt_Cnt[_lay] -= _rd_size;

	if (InputDta188Pkt_Cnt[_lay] < (_CAPTURED_INPUT_BUFFER_SIZE__max_*1/4))
	{
		lTemp = 0;	//	trace point
	}

	return	_rd_size;
}
int CHldTmccRmx::WrDtaIntoInputBuffer_EachLayer(int _lay, int _wr_size, unsigned char *_wr_dta)
{
	unsigned int	lTemp;

	if (InputDta188Pkt_WrPos <= (unsigned int)((_CAPTURED_INPUT_BUFFER_SIZE__max_ - _wr_size)))
	{
		memcpy(InputDta188Pkt_WrByHwCapturer + InputDta188Pkt_WrPos, _wr_dta, _wr_size);
	
		InputDta188Pkt_WrPos += _wr_size;
	}
	else 
	{
		lTemp = _CAPTURED_INPUT_BUFFER_SIZE__max_ - InputDta188Pkt_WrPos;
	
		memcpy(InputDta188Pkt_WrByHwCapturer + InputDta188Pkt_WrPos, _wr_dta, lTemp);
		memcpy(InputDta188Pkt_WrByHwCapturer, _wr_dta + lTemp, _wr_size - lTemp);
	
		InputDta188Pkt_WrPos += _wr_size;
		if (InputDta188Pkt_WrPos >= _CAPTURED_INPUT_BUFFER_SIZE__max_)
			InputDta188Pkt_WrPos -= _CAPTURED_INPUT_BUFFER_SIZE__max_;
	}
	InputDta188Pkt_Cnt[_lay] += _wr_size;

	return	_wr_size;
}
int CHldTmccRmx::WrDtaIntoInputBuffer(int _wr_size, unsigned char *_wr_dta)
{
	WrDtaIntoInputBuffer_EachLayer(0, _wr_size, _wr_dta);
	InputDta188Pkt_Cnt[1] += _wr_size;
	InputDta188Pkt_Cnt[2] += _wr_size;
	for (int i = 0; i < 3; i++)
	{
		if (InputDta188Pkt_Cnt[i] >= _CAPTURED_INPUT_BUFFER_SIZE__max_)
		{
			InputDta188Pkt_Cnt[i] = _CAPTURED_INPUT_BUFFER_SIZE__max_;	//	to avoid wrap-around of max-range (unsigned int)val.
		}
	}

	return	_wr_size;
}

int CHldTmccRmx::FILE_READ(void *buffer, size_t size, size_t count, int idx)
{
	CHld	*_hld_ = (CHld *)my_hld;
	int nPacketSize=count;
	unsigned char	data_packet[204];
	//2012/9/28 bert fixed
	int nBertType = _hld_->_FIf->_VarTstPktTyp();
	int nBertPid = _hld_->_FIf->_VarTstPktPid();
	unsigned char *bert_buf;
	if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23)
	{
		bert_buf = (unsigned char *)buffer;
		bert_buf[0] = 0x47;
		bert_buf[1] = ((nBertPid >> 8) & 0x1F);
		bert_buf[2] = (nBertPid & 0xFF);
		bert_buf[3] = 0x10 + _hld_->_FIf->TL_Cont_cnt;
		for(int i = 4; i < nPacketSize; i++)
		{
			bert_buf[i] = 0x00;
		}
		_hld_->_FIf->TL_Cont_cnt++;
		if(_hld_->_FIf->TL_Cont_cnt > 15)
			_hld_->_FIf->TL_Cont_cnt = 0;

		return nPacketSize;
	}

	ReinitAtScroll();
		
_FILL_DATA_BUFFER_:
////////////////////////////////////////////////////////////////////////////
	if ( gDummyBuf && gDummyBufPos > 0 )	// there is usefull data in gDummyBuf... refer to ReformatOrigTs_w_DummyPkt()
	{
		QueueLayerBufferFromDummyBuf(idx, nPacketSize);
		goto	_rd_from_buf;
	}

////////////////////////////////////////////////////////////////////////////
	if ( gCT[idx] < nPacketSize )			//	no enough data in layer buf... refer to ReformatOrigTs_w_DummyPkt()
	{
		if (IsValidHandle(idx) == 0)
		{
			return 0;
		}

		if (QueueLayerBufferFromRedTsData(idx) == 0)	//	read from real data source.
		{
			return 0;
		}

////////////////////////////////////////////////////////////////////////////
		memcpy(gBF2[idx], gBF[idx], MAX_BUFFERED_READ_SIZE);
		gCT2[idx] = gCT[idx];
		gWD2[idx] = gWD[idx];
		gRD2[idx] = gRD[idx];

		WORD wPID;
		__int64 iiPCR;
		int pid_count = gLayerPidCount[idx];
		int nRD = gRD[idx];
		int nCT = gCT[idx];
		int nWD = gWD[idx];

////////////////////////////////////////////////////////////////////////////
		InitDummyInsertCond();
		while ( nCT >= nPacketSize )	//	iteration until consumme all data
		{
			RdOneRawPktFromLayerBuffer(idx, data_packet, nRD, nPacketSize);
			if ( data_packet[0] != 0x47 )	//	sync. check.....
			{
				nRD += 1;
				if (nRD >= MAX_BUFFERED_READ_SIZE)
				{
					nRD -= MAX_BUFFERED_READ_SIZE;
				}
				nCT -= 1;
				continue;	//	resync.
			}

			wPID = GET_PCR((unsigned char*)data_packet, &iiPCR);
			CalcBitrateAndDummyCond(idx, nRD, nPacketSize, wPID, iiPCR);

			nRD += nPacketSize;
			if (nRD >= MAX_BUFFERED_READ_SIZE)
			{
				nRD -= MAX_BUFFERED_READ_SIZE;
			}
			nCT -= nPacketSize;
		}
		
////////////////////////////////////////////////////////////////////////////
		if (ReformatOrigTs_w_DummyPkt(idx, nPacketSize) == 0)
		{
			return 0;
		}

////////////////////////////////////////////////////////////////////////////
		memcpy(gBF[idx], gBF2[idx], MAX_BUFFERED_READ_SIZE);
		gCT[idx] = gCT2[idx];
		gWD[idx] = gWD2[idx];
		gRD[idx] = gRD2[idx];
	}

////////////////////////////////////////////////////////////////////////////
_rd_from_buf:
	if ( gCT[idx] >= nPacketSize )
	{
		if ( *((char*)gBF[idx] + gRD[idx]) != 0x47 )	//	sync lost
		{
			int ret_pos;
			do
			{
				ret_pos = FindSyncPosAtLayerBuf_and_SetgRDVal(idx);
				if (ret_pos < 0)
				{
					goto	_FILL_DATA_BUFFER_;
				}
				break;
				Sleep(1);
			} while ( 1 );
		}

////////////////////////////////////////////////////////////////////////////
		RdOneSyncPktFromLayerBuffer(idx, (unsigned char *)buffer, nPacketSize);

		if ( nPacketSize > 188 )
		{
			memset((char*)buffer+188, 0x00, nPacketSize-188);
		}
	}
	
	return nPacketSize;
}

void  CHldTmccRmx::SetBits(unsigned char* pSrc, int* pIndex, int k, int nBits)
{
	unsigned int tmp;
    int  nIdx = *pIndex;

    pSrc += nIdx >> 3;

    tmp = (pSrc[0] << 24) 
		| (pSrc[1] << 16) 
		| (pSrc[2] << 8) 
		| (pSrc[3] << 0);
    tmp |= (nBits << (32 - (nIdx & 7) - k));

    pSrc[0] = tmp >> 24;
    pSrc[1] = tmp >> 16;
    pSrc[2] = tmp >> 8;
    pSrc[3] = tmp >> 0;

    nIdx += k;
    *pIndex = nIdx;
}

void  CHldTmccRmx::CreateTmccInfo(unsigned char*  pTmccInfo, struct InParams  Pars)
{
	int  nIndex = 0, nPartialReception, nLayerIdx;
    memset(pTmccInfo, 0, 14);

	if (Pars.m_BrodcastType != ISDBT_BTYPE_TV
		&& Pars.m_BrodcastType != ISDBT_BTYPE_TV_1SEG) 
	{
		SetBits(pTmccInfo, &nIndex, 2, 1);
        nPartialReception = (Pars.m_BrodcastType == ISDBT_BTYPE_RAD3);
    } 
	else 
	{
        SetBits(pTmccInfo, &nIndex, 2, 0); 
		nPartialReception = Pars.m_PartialRecev;
    }
    SetBits(pTmccInfo, &nIndex, 4, 0xF); 
    SetBits(pTmccInfo, &nIndex, 1, 0); 

	//<-- fill TMCC information
    SetBits(pTmccInfo, &nIndex, 1, nPartialReception); 
    for ( nLayerIdx=0; nLayerIdx < 3; nLayerIdx++ ) 
	{
		if (Pars.m_Layers[nLayerIdx].m_NumberOfSegments == 0) 
		{
            SetBits(pTmccInfo, &nIndex, 3, 7); 
            SetBits(pTmccInfo, &nIndex, 3, 7); 
            SetBits(pTmccInfo, &nIndex, 3, 7); 
            SetBits(pTmccInfo, &nIndex, 4, 0xF); 
        } 
		else 
		{
            SetBits(pTmccInfo, &nIndex, 3, Pars.m_Layers[nLayerIdx].m_Constellation); 
            SetBits(pTmccInfo, &nIndex, 3, Pars.m_Layers[nLayerIdx].m_CodeRate); 
			SetBits(pTmccInfo, &nIndex, 3, Pars.m_Layers[nLayerIdx].m_TimeInterleave); 
            SetBits(pTmccInfo, &nIndex, 4, Pars.m_Layers[nLayerIdx].m_NumberOfSegments); 
        }
    }

    SetBits(pTmccInfo, &nIndex, 1, 1); 
	for (nLayerIdx=0; nLayerIdx < 3; nLayerIdx++) 
	{
        SetBits(pTmccInfo, &nIndex, 3, 7); 
        SetBits(pTmccInfo, &nIndex, 3, 7); 
        SetBits(pTmccInfo, &nIndex, 3, 7); 
        SetBits(pTmccInfo, &nIndex, 4, 0xF); 
    }
    
    if (Pars.m_BrodcastType != ISDBT_BTYPE_TV
		&& Pars.m_BrodcastType != ISDBT_BTYPE_TV_1SEG) 
	{
		//no phase correction
		SetBits(pTmccInfo, &nIndex, 3, 0x7);
        SetBits(pTmccInfo, &nIndex, 12, 0xFFF);
    } 
	else
	{
        SetBits(pTmccInfo, &nIndex, 15, 0x7FFF);
    }
}
void  CHldTmccRmx::GetPacketLayerBuf(int nLayerIndex, unsigned char*  pPacket)	//	nLayerIndex. 0/1/2
{
	int lTemp = 0;

	if ( TL_InParams.m_Layers[nLayerIndex].m_NumberOfSegments == 0 
		|| gCT_layered[nLayerIndex] < 204 )
	{
		int nTpCount2 = (TL_OutParams.m_Layers[nLayerIndex].m_TpPerFrame);
		while ( gCT_layered[nLayerIndex] < nTpCount2*204 && TL_ThreadDone == 0 )
		{
			Sleep(10);
		}
		goto _get_packet_;
	}
	else
	{
_get_packet_:
		lTemp = 0;
		if ( gRD_layered[nLayerIndex] < MAX_BUFFER_BYTE_SIZE_layered - 204 )
		{
			memcpy(pPacket, gBF_layered[nLayerIndex] + gRD_layered[nLayerIndex], 204);
			gRD_layered[nLayerIndex] += 204;
		}
		else
		{
			lTemp = MAX_BUFFER_BYTE_SIZE_layered - gRD_layered[nLayerIndex];
			memcpy(pPacket, gBF_layered[nLayerIndex] + gRD_layered[nLayerIndex], lTemp);
			memcpy(pPacket + lTemp, gBF_layered[nLayerIndex], 204-lTemp);
			gRD_layered[nLayerIndex] = 204-lTemp;
		}
		gCT_layered[nLayerIndex] -= 204;
	}
}
void  CHldTmccRmx::GetPacket(int nLayerIndex, unsigned char*  pPacket)	//	nLayerIndex. 0/1/2
{
	if (ChkDtaAvailable(nLayerIndex) == 0)
	{
		goto	_padding_now;
	}

#if 1
	if ( gUseLayeredBuffer == 1 )
	{
		GetPacketLayerBuf(nLayerIndex, pPacket);
		return;
	}
#endif

    if ( gEndOfFile || (IsValidHandle(nLayerIndex) == 0) ) 
	{
_padding_now:
        pPacket[0] = 0x47;
        pPacket[1] = 0x1F;
        pPacket[2] = 0xFF;
        pPacket[3] = 0x10;
        memset(pPacket + 4, 0x00, 184);
    } 
	else 
	{
		int nRet;
		int i, is_found=0;
		int count = gLayerPidCount[nLayerIndex];
		int total_count = gLayerPidCount[3];
		WORD wPID;
		
		if ( count == 0 )	//	there is no pid-list in this layer... sure???
		{
			goto _padding_now;
		}
		nRet = FILE_READ(pPacket, 1, gPacketSize, nLayerIndex);

		do
		{
			wPID = (pPacket[1]&0x1F)*256+pPacket[2];

			if ( (wPID == 0x1FFF) && (pPacket[3] == nLayerIndex) && (pPacket[4] == nLayerIndex) && (pPacket[5] == nLayerIndex) )
			{
				is_found = 1;	//	exactlly match null. we will get this one.
				break;
			}

			for ( i = 0; i < count; i++ )
			{
				if ( gLayerPidInfo[nLayerIndex][i] == wPID )	//	found a pid within the pid-list of this layer. we will get this one.
				{
					is_found = 1;
					break;
				}
			}
			if ( is_found == 1 )
			{
				break;
			}
			
			if ( total_count > 0 && (gLayerOtherPidMap >= 0 && gLayerOtherPidMap < 3) && wPID != 0x1FFF )
			{
				for ( i = 0; i < total_count; i++ )
				{
					if ( gLayerPidInfo[3][i] == wPID )	//	match one of PIDs
						break;
				}
				if ( i == total_count )		//	there is no pid list for this pkt.
				{
					if ( gLayerOtherPidMap == nLayerIndex )	//	undefined layer. so we process this pid as other map.
					{
						break;
					}
					else
					{
						goto _padding_now;//???
					}
				}
			}

			if ( count == 0 || (wPID == 0x1FFF && gLayerOtherPidMap == nLayerIndex) )
			{
				goto _padding_now;//???
			}

			nRet = FILE_READ(pPacket, 1, gPacketSize, nLayerIndex);	//	read another one pkt.

		} while ( nRet == gPacketSize );

		if ( nRet != gPacketSize ) 	//	nok read pkt
		{
			SetEof(nLayerIndex);
            goto _padding_now;
        }
    }
}

int CHldTmccRmx::GetParams(struct OutParams* pOutPars, struct InParams* pPars)
{
	int i;
	struct LayerPars_*  pLayer;

	// Number Of TPS
	pOutPars->m_TpPerFrame = 0;
	for ( i=0; i<3; i++) 
	{
		pLayer = &(pPars->m_Layers[i]);

		pOutPars->m_Layers[i].m_TpPerFrame = 0;
//		printf("Layer %c: segs=%d", 'A' + i, pLayer->m_NumberOfSegments);
		if (pLayer->m_NumberOfSegments != 0) 
		{
			switch ( pLayer->m_Constellation )
			{
			case ISDBT_MOD_DQPSK :
			case ISDBT_MOD_QPSK :
				if ( pLayer->m_CodeRate == ISDBT_RATE_1_2 )
					pOutPars->m_Layers[i].m_TpPerFrame = 12;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_2_3 )
					pOutPars->m_Layers[i].m_TpPerFrame = 16;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_3_4 )
					pOutPars->m_Layers[i].m_TpPerFrame = 18;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_5_6 )
					pOutPars->m_Layers[i].m_TpPerFrame = 20;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_7_8 )
					pOutPars->m_Layers[i].m_TpPerFrame = 21;
				else 
					pOutPars->m_Layers[i].m_TpPerFrame = 12;
				break;

			case ISDBT_MOD_16QAM :
				if ( pLayer->m_CodeRate == ISDBT_RATE_1_2 )
					pOutPars->m_Layers[i].m_TpPerFrame = 24;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_2_3 )
					pOutPars->m_Layers[i].m_TpPerFrame = 32;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_3_4 )
					pOutPars->m_Layers[i].m_TpPerFrame = 36;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_5_6 )
					pOutPars->m_Layers[i].m_TpPerFrame = 40;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_7_8 )
					pOutPars->m_Layers[i].m_TpPerFrame = 42;
				else 
					pOutPars->m_Layers[i].m_TpPerFrame = 24;
				break;

			case ISDBT_MOD_64QAM :
				if ( pLayer->m_CodeRate == ISDBT_RATE_1_2 )
					pOutPars->m_Layers[i].m_TpPerFrame = 36;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_2_3 )
					pOutPars->m_Layers[i].m_TpPerFrame = 48;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_3_4 )
					pOutPars->m_Layers[i].m_TpPerFrame = 54;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_5_6 )
					pOutPars->m_Layers[i].m_TpPerFrame = 60;
				else if ( pLayer->m_CodeRate == ISDBT_RATE_7_8 )
					pOutPars->m_Layers[i].m_TpPerFrame = 63;
				else 
					pOutPars->m_Layers[i].m_TpPerFrame = 36;
				break;

			default :
				pOutPars->m_Layers[i].m_TpPerFrame = 0;
				break;
			}
			pOutPars->m_Layers[i].m_TpPerFrame *= (int)pow((double)2, pPars->m_Mode-1);
			pOutPars->m_Layers[i].m_TpPerFrame *= pLayer->m_NumberOfSegments;

			pOutPars->m_TpPerFrame += pOutPars->m_Layers[i].m_TpPerFrame;//????
		}
//		printf("\n");
	}

	// Bitrate, Total Tp per frame
	pOutPars->m_Bps = 0.;
	pOutPars->m_TotalTpPerFrame = 0;

	double frame_length=0.;//msec.
	switch ( pPars->m_Mode )
	{
	case 1:
		if (pPars->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 53.0145;
			pOutPars->m_TotalTpPerFrame = 1056;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 54.621;
			pOutPars->m_TotalTpPerFrame = 1088;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 57.834;
			pOutPars->m_TotalTpPerFrame = 1152;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 64.26;
			pOutPars->m_TotalTpPerFrame = 1280;
		}
		break;
	case 2:
		if (pPars->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 106.029;
			pOutPars->m_TotalTpPerFrame = 2112;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 109.242;
			pOutPars->m_TotalTpPerFrame = 2176;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 115.668;
			pOutPars->m_TotalTpPerFrame = 2304;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 128.52;
			pOutPars->m_TotalTpPerFrame = 2560;
		}
		break;
	case 3:
		if (pPars->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 212.058;
			pOutPars->m_TotalTpPerFrame = 4224;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 218.484;
			pOutPars->m_TotalTpPerFrame = 4352;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 231.336;
			pOutPars->m_TotalTpPerFrame = 4608;
		}
		else if (pPars->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 257.04;
			pOutPars->m_TotalTpPerFrame = 5120;
		}
		break;
	default :
		break;
	}
	pOutPars->m_FrameLength = frame_length;
	pOutPars->m_Bps = pOutPars->m_TpPerFrame * 188 * 8 * (1/frame_length) * 1000;//bit/sec.
	for (i=0; i<3; i++) 
	{
		pLayer = &(pPars->m_Layers[i]);
		pOutPars->m_Layers[i].m_Bps = pOutPars->m_Layers[i].m_TpPerFrame * 188 * 8 * (1/frame_length) * 1000;
	}

	pOutPars->m_DelayOfSymbol = 0;

	int NumOfSegs = pPars->m_Layers[0].m_NumberOfSegments + 
		pPars->m_Layers[1].m_NumberOfSegments + 
		pPars->m_Layers[2].m_NumberOfSegments;

	if ( (pPars->m_BrodcastType == ISDBT_BTYPE_TV && NumOfSegs != 13)
		|| (pPars->m_BrodcastType == ISDBT_BTYPE_RAD1 && NumOfSegs != 1)
		|| (pPars->m_BrodcastType == ISDBT_BTYPE_RAD3 && NumOfSegs != 3)
		|| (pPars->m_BrodcastType == ISDBT_BTYPE_TV_1SEG && NumOfSegs != 1) 
//2011/2/1
#ifdef WIN32
#else
		|| (pPars->m_BrodcastType == -1)
#endif
		)
	{
		return -1;
	}

	if ( pPars->m_BrodcastType == ISDBT_BTYPE_TV && pPars->m_PartialRecev == 1 && pPars->m_Layers[0].m_NumberOfSegments != 1 )
	{
		return -2;
	}

	if ( pPars->m_BrodcastType == ISDBT_BTYPE_TV_1SEG && (pPars->m_PartialRecev != 1 || pPars->m_Layers[0].m_NumberOfSegments != 1) )
	{
		return -3;
	}
	
	return 0;
}

void  CHldTmccRmx::CreateRemuxedTs(FILE*  OutFile, struct InParams  Pars, struct OutParams  OutPars)
{
    unsigned char  remuxedData[204];
	unsigned char*  tmpData;
	int  nPos, nPos2, TpCount, nTpCount2, nLayerIdx, nSymbolNumber, Val;
	bool  bIIPSent = false;
	int  nPacketCount = 1024 << (Pars.m_Mode - 1);
    nPacketCount = nPacketCount + (nPacketCount >> (5 - Pars.m_GuardInterval));
	FILE *fp_param = NULL;

#if	0
	if (_188_loopthru_mode)
	{
		while(1)
		{
			if (TL_ThreadDone == 1)
			{
				return;
			}
			if (TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL())
			{
				break;
			}
			Sleep(10);
		}
	}
#endif

//	fp_param = fopen("C:\\param_bxk.txt", "wt");

	//frames loop
    for (nPos=0; !gEndOfFile; nPos++) 
	{
	    TpCount = 0;
		bIIPSent = false;
		for ( nSymbolNumber = 0; nSymbolNumber < 204; nSymbolNumber++ ) 
		{
			for ( nLayerIdx = 0; nLayerIdx < 4; nLayerIdx++ ) 
			{
				nTpCount2 = (nLayerIdx == 3) ? (nPacketCount - OutPars.m_TpPerFrame) : (OutPars.m_Layers[nLayerIdx].m_TpPerFrame);
				nTpCount2 = ((nTpCount2 * (nSymbolNumber + 1)) / 204) - ((nTpCount2 * nSymbolNumber) / 204);

				for ( nPos2 = 0; nPos2 < nTpCount2; nPos2++ ) 
				{
					memset(remuxedData, 0, 204);
					remuxedData[0] = 0x47;
				
					if ( nLayerIdx == 3 )  
					{
						if ( nPos2 == 0 ) 
						{
							remuxedData[1] = 0x1F | 0x40;
							remuxedData[2] = 0xFF;
							remuxedData[3] = 0x10;
							//IIP packet
							tmpData = remuxedData + 4;
							Val = nPacketCount - 1;
							*tmpData++ = Val >> 8;
							*tmpData++ = Val;
							tmpData++;			//sync + ac effective position
							*tmpData++ = (Pars.m_Mode << 6) | (Pars.m_GuardInterval << 4) | (Pars.m_Mode << 2) | (Pars.m_GuardInterval << 0);
							CreateTmccInfo(tmpData, Pars);
							tmpData += 14;
							tmpData += 4;		//CRC 
							*tmpData++ = 0;		//IIP_branch_number
							*tmpData++ = 0;		//last IIP branch number
							*tmpData++ = 0;		//network information length
							memset(tmpData, 0xFF, 159); //stuffing 
						} 
						else 
						{
							//NULL TP
							remuxedData[1] = 0x1F;
							remuxedData[2] = 0xFF;
							remuxedData[3] = 0x10;
						}
					} 
					else 
					{
						GetPacket(nLayerIdx, remuxedData);
					}

					tmpData = remuxedData + 188;
					tmpData[0] = (2 << 6) | (1 << 5);
					// 1st packet in frame
					if ( TpCount == 0 )
					{
						tmpData[0] |= 1 << 1;
					}
					// odd frame
					if ( nPos & 1 )
					{
						tmpData[0] |= 1 << 0;
					}
					if ( nLayerIdx == 3 ) 
					{
						//IIP TP
						if (!bIIPSent) 
						{
							tmpData[1] = 8 << 4;		
							bIIPSent = true;
						}
					} 
					else 
					{
						//layer
						tmpData[1] = ((nLayerIdx + 1) << 4);	
					}
					tmpData[1] |= 0xf;				// Count-down information
					tmpData[2] = (1 << 7);			// AC invalidity
					tmpData[2] |= (3 << 5);			// reserved
					tmpData[2] |= TpCount >> 8;				// TSP count
					tmpData[3] = TpCount;
					tmpData[4] = 0xFF;				// AC data
					tmpData[5] = 0xFF;
					tmpData[6] = 0xFF;
					tmpData[7] = 0xFF;

					//set parity
					memset(tmpData + 8, 0xFF, 8);
					if ( OutFile )
					{
						fwrite(remuxedData, 1, 204, OutFile);
					}

////////////////////////////////////////////////////////////////////////////
					while ((TL_nBufferCnt > _MUXED_RSLT_BUFFER_SIZE__highlevel_) && (TL_ThreadDone == 0))
					{
						Sleep(10);
					}
					LockMutex__();
					WrDtaIntoRsltBuffer(204, remuxedData);
					UnlockMutex__();

////////////////////////////////////////////////////////////////////////////
					TpCount++;
				}
			}
        }

		if (TL_ThreadDone == 1)
		{
			break;
		}
    }

	if ( OutFile )
	{
		fclose(OutFile);
	}

	if ( fp_param )
	{
		fclose(fp_param);
	}
}
void	CHldTmccRmx::_MainLoop(void)
{
#if defined(WIN32)
	FILE *fp = fopen("C:\\tmcc_remuxer_dump", "rt");
#else
	FILE *fp = fopen("./tmcc_remuxer_dump", "rt");
#endif
	if ( fp )
	{
		fclose(fp);
#if defined(WIN32)
		fp = fopen("C:\\dump.trp", "wb");
#else
		fp = fopen("./dump.trp", "wb");
#endif
		CreateRemuxedTs(fp, TL_InParams, TL_OutParams);
		fclose(fp);
	}
	else
	{
		CreateRemuxedTs(NULL, TL_InParams, TL_OutParams);
	}
	TL_EndOfRemux = 1;
}
#if defined(WIN32)
void	CHldTmccRmx::MainLoop(void* param)
#else
void	*CHldTmccRmx::MainLoop(void* param)
#endif
{
	CHld	*_hld_ = (CHld *)param;
	_hld_->_MainLoop();
}
int CHldTmccRmx::TMCC_START_REMUX(void)
{
	TL_EndOfRemux = 0;
	TL_ThreadDone = 0;

	if ( IsValidHandle(0) )
	{
		SeekTs(0, 0, NULL, 0, 0);
		SeekTs(1, 0, NULL, 0, 0);
		SeekTs(2, 0, NULL, 0, 0);
	}

	gEndOfFile = false;

	if (GetParams(&TL_OutParams, &TL_InParams) < 0)
	{
		TL_EndOfRemux = 1;

        //printf("!!! Check ISDB-T parameters. !!!\n");
		return -1;
	}

#if defined(WIN32)	
	return _beginthread(MainLoop, 0, (void *)my_hld);
#else
	return pthread_create(&TL_TMCC_thread, NULL, MainLoop, (void *)my_hld);
#endif
}
int CHldTmccRmx::TMCC_STOP_REMUX(void)
{
	int i;

	TL_ThreadDone = 1;
	while ( TL_EndOfRemux == 0 )
		Sleep(10);

	for ( i = 0; i < 3; i++ )
	{
		gRD[i] = gWD[i] = gCT[i] = 0;

		gRD2[i] = gWD2[i] = gCT2[i] = 0;
		gwPID[i] = -1;
		gPkt0[i]=0;
		gPkt1[i]=0;
		giiPCR0[i]=0;
		giiPCR1[i]=0;
	}
	FreeDummyBuf();

	return 0;
}
int CHldTmccRmx::TMCC_SET_REMUX_INFO(
	long btype, long mode, long guard_interval, long partial_reception, long bitrate,
	long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
	long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
	long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
	//initialize
	memset(&TL_InParams, 0, sizeof(struct InParams));
	TL_InParams.m_BrodcastType = btype;//0,1,2
	TL_InParams.m_Mode = mode;//1,2,3
	TL_InParams.m_GuardInterval = guard_interval;//0(1/32),1(1/16),2(1/8),3(1/4)
	TL_InParams.m_PartialRecev = partial_reception;//0(No), 1(Yes)
	TL_InParams.m_Bps = bitrate;

	TL_InParams.m_Layers[0].m_NumberOfSegments = a_segments;
	TL_InParams.m_Layers[0].m_Constellation = a_modulation;
	TL_InParams.m_Layers[0].m_CodeRate = a_code_rate;
	TL_InParams.m_Layers[0].m_TimeInterleave = a_time_interleave;
	TL_InParams.m_Layers[0].m_Selected_Bps = a_bitrate;

	TL_InParams.m_Layers[1].m_NumberOfSegments = b_segments;
	TL_InParams.m_Layers[1].m_Constellation = b_modulation;
	TL_InParams.m_Layers[1].m_CodeRate = b_code_rate;
	TL_InParams.m_Layers[1].m_TimeInterleave = b_time_interleave;
	TL_InParams.m_Layers[1].m_Selected_Bps = b_bitrate;

	TL_InParams.m_Layers[2].m_NumberOfSegments = c_segments;
	TL_InParams.m_Layers[2].m_Constellation = c_modulation;
	TL_InParams.m_Layers[2].m_CodeRate = c_code_rate;
	TL_InParams.m_Layers[2].m_TimeInterleave = c_time_interleave;
	TL_InParams.m_Layers[2].m_Selected_Bps = c_bitrate;

	if (GetParams(&TL_OutParams, &TL_InParams) < 0)
	{
        //printf("!!! Check ISDB-T parameters. !!!\n");
		return -1;
	}

	return 0;
}

int CHldTmccRmx::TMCC_OPEN_REMUX_SOURCE(char *szFilePath)
{
	int nRet;

	CHld	*_hld_ = (CHld *)my_hld;
	//2012/9/28 bert fixed
	int nBertType = _hld_->_FIf->_VarTstPktTyp();
	nRet = OpenTs(szFilePath);
	if (nRet < 0)
	{
		if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23)
		{
			;
		}
		else
			return	nRet;
	}
	FreeDummyBuf();
	CreateMutex__();	//	mutex.
	AllocRsltBuf();
	AllocInputBuf();
	TsAttrPktSize();	//	detect source pkt size.
	
	TL_nPositionChanged = 0;
	TL_a_low = 0;
	TL_a_high = 0;
	
	char *pdest = strrchr(szFilePath, '\\');
	if ( pdest != NULL )
	{
		if ( strcmp(pdest+1, "dummy") == 0 )
		{
			gUseLayeredBuffer = 1;
		}
	}

	return 0;
}

int CHldTmccRmx::TMCC_CLOSE_REMUX_SOURCE(void)
{
	int i;

	for ( i = 0; i < 3; i++ )
	{
		CloseTs(i);
		FreeInterBuf(i);
	}
	FreeRsltBuf();
	FreeInputBuf();
	DestroyMutex__();

	return 0;
}

char	*CHldTmccRmx::TMCC_READ_REMUX_DATA(int dwBytesToRead, int *pReadPos, int *pBufferCnt, int *pEndOfRemux)
{
	if (((int)TL_nBufferCnt < dwBytesToRead) || (TL_ThreadDone == 1) || (!RsltDta_ReadByHwWriter) || (!RsltDta_UsrSpace))
	{
		*pReadPos = TL_nReadPos;
		*pBufferCnt = TL_nBufferCnt;
		*pEndOfRemux = TL_EndOfRemux;

		return NULL;
	}

	LockMutex__();
	RdDtaFromRsltBuffer(dwBytesToRead);

	*pReadPos = TL_nReadPos;
	*pBufferCnt = TL_nBufferCnt;
	*pEndOfRemux = TL_EndOfRemux;

	UnlockMutex__();

	return (char*)RsltDta_UsrSpace;
}

int CHldTmccRmx::TMCC_SET_LAYER_INFO(
		long other_pid_map_to_layer,
		long multi_pid_map,
		long total_count, char* total_pid_info,
		long a_pid_count, char* a_pid_info,
		long b_pid_count, char* b_pid_info,
		long c_pid_count, char* c_pid_info)
{
	char *string;
	char seps[] = ",";
	char *token;
	int i = 0, j;

	LockMutex__();

	for ( i = 0; i < 4; i++ )
	{
		if ( gLayerPidInfo[i] )
			free(gLayerPidInfo[i]);

		gLayerPidCount[i] = 0;
	}
	gLayerOtherPidMap = other_pid_map_to_layer;
	gLayerMultiPidMap = multi_pid_map;

	for ( i = 0; i < 4; i++ )
	{
		if ( i == 0 )
		{
			gLayerPidCount[i] = a_pid_count;
			string = a_pid_info;
		}
		else if ( i == 1 )
		{
			gLayerPidCount[i] = b_pid_count;
			string = b_pid_info;
		}
		else if ( i == 2 )
		{
			gLayerPidCount[i] = c_pid_count;
			string = c_pid_info;
		}
		else
		{
			gLayerPidCount[i] = total_count;
			string = total_pid_info;
		}
	
		if ( gLayerPidCount[i] > 0 )
		{
			//2014/10/08 linux x64 modify
			//gLayerPidInfo[i] = (int*)malloc(gLayerPidCount[i]*sizeof(int));
			gLayerPidInfo[i] = (int*)malloc(0x2000 * sizeof(int));
			if ( !gLayerPidInfo[i] )
			{
				gLayerPidCount[i] = 0;
			}
			else
			{
				j = 0;
				token = strtok(string, seps);
				while( token != NULL )
				{
					int __pid;
					if ((token[0] == '0') && (token[1] == 'x'))
					{
						__pid = strtoul(token, NULL, 16);
					}
					else
					{
						__pid = atoi(token);
					}
					gLayerPidInfo[i][j++]=__pid;
					token = strtok(NULL, seps);
				}
			}
		}
		else
		{
			gLayerPidInfo[i] = NULL;
		}
	}

	UnlockMutex__();

	return 0;
}

int CHldTmccRmx::TMCC_SET_REMUX_SOURCE_POSITION(long a_low, long a_high, long b_low, long b_high, long c_low, long c_high)
{
	int i;
	for  (i = 0; i < 3; i++ )
	{
		if (IsValidHandle(i) == 0)
		{
			return -1;
		}
	}

	LockMutex__();

	TL_nPositionChanged = 1;
	TL_a_low = a_low;
	TL_a_high = a_high;

	UnlockMutex__();

	return 0;
}

long CHldTmccRmx::TMCC_GET_REMUX_SOURCE_POSITION(long layer_index, long* high)
{
	long low = 0;

	if (_188_loopthru_mode)
	{
		high = NULL;
		return 0L;
	}

	if ( IsValidHandle(layer_index) )
	{
#if defined(WIN32)	
		*high = 0;
		low = SetFilePointer(gLayerFile[layer_index], 0, high, FILE_CURRENT);
#else
		low = ftello((FILE*)gLayerFile[layer_index]) & 0xFFFFFFFF;
		*high = (ftello((FILE*)gLayerFile[layer_index])>>32) & 0xFFFFFFFF;
#endif
		return low;
	}

	high = NULL;
	return 0L;
}

int CHldTmccRmx::TMCC_WRITE_REMUX_DATA(unsigned char *pData, int nLen)
{
	return 0;
}
void CHldTmccRmx::TMCC_SET_188_LOOPTHRU_MODE(int val)
{
	_188_loopthru_mode = val;
}
void CHldTmccRmx::TMCC_WR_188_LPTHRU_CAPTURE_DTA(int _wr_size, unsigned char *_wr_dta)
{
	WrDtaIntoInputBuffer(_wr_size, _wr_dta);
	if (!TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL())
	{
		CHld	*_hld_ = (CHld *)my_hld;
		int	_bitrate;

		InputDta188Pkt_initial_sync_pos = _hld_->_FIf->SyncPosAssume188((char *)InputDta188Pkt_WrByHwCapturer, InputDta188Pkt_Cnt[0]);
		_bitrate = _hld_->_FIf->TL_Calculate_Bitrate(InputDta188Pkt_WrByHwCapturer, InputDta188Pkt_Cnt[0], InputDta188Pkt_initial_sync_pos, 188, NULL, NULL);
		if (_bitrate <= 0)
		{
			//	keep prev cond.
		}
		else
		{
			InputDta188Pkt_fullness_Criteria = (_bitrate/8)*_PRESAVE_to_CALC_TSINFO_sec_;
		}
		if (InputDta188Pkt_fullness_Criteria > _CAPTURED_INPUT_BUFFER_SIZE__max_)
		{
			InputDta188Pkt_fullness_Criteria = _CAPTURED_INPUT_BUFFER_SIZE__max_;
		}
		if (InputDta188Pkt_fullness_Criteria < _CAPTURED_INPUT_BUFFER_SIZE__min_)
		{
			InputDta188Pkt_fullness_Criteria = _CAPTURED_INPUT_BUFFER_SIZE__min_;
		}

		InputDta188Pkt_fullness += _wr_size;
		if (TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL())
		{
			memcpy(InputDta188Pkt_SaveFs, InputDta188Pkt_WrByHwCapturer, InputDta188Pkt_fullness_Criteria);
		}
	}
}
int CHldTmccRmx::TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(unsigned char *_fn)
{
	unsigned long	_ret;

	if (InputDta188Pkt_SaveFs == NULL)
	{
		return -1;
	}

#if defined(WIN32)
	HANDLE	_file;

	_file = CreateFile((char *)_fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if ( _file == INVALID_HANDLE_VALUE )
	{
		return -1;
	}
	WriteFile(_file, InputDta188Pkt_SaveFs, InputDta188Pkt_fullness_Criteria, &_ret, NULL);
	CloseHandle(_file);
#else
	FILE	*_file;

	_file = fopen((char *)_fn, (char *)"wb");
	if ( _file == NULL )
	{
		return -1;
	}
	_ret = fwrite(InputDta188Pkt_SaveFs, 1, InputDta188Pkt_fullness_Criteria, _file);
	fclose(_file);
#endif

	return	(int)_ret;
}
//2012/8/23
int CHldTmccRmx::TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(struct _TSPH_TS_INFO **ppTsInfo)
{
	CHld	*_hld_ = (CHld *)my_hld;

	if (InputDta188Pkt_SaveFs == NULL)
	{
		return -1;
	}
	_hld_->_FIf->RunTs_Parser(InputDta188Pkt_SaveFs, InputDta188Pkt_fullness_Criteria, 19392658, 0, ppTsInfo);
	return	0;
}

int CHldTmccRmx::TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL(void)
{
	if (InputDta188Pkt_fullness >= InputDta188Pkt_fullness_Criteria)
	{
		return	1;
	}
	return	0;
}

void CHldTmccRmx::TMCC_init_InputDta188Pkt_fullness(void)
{
	LockMutex__();

	InputDta188Pkt_WrPos = 0;
	for (int i = 0; i < 3; i++)
	{
		InputDta188Pkt_RdPos[i] = 0;
		InputDta188Pkt_Cnt[i] = 0;
	}
	InputDta188Pkt_initial_sync_pos = 0;
	InputDta188Pkt_padding_cond = 0;
	InputDta188Pkt_fullness = 0;
	InputDta188Pkt_fullness_Criteria = _CAPTURED_INPUT_BUFFER_SIZE__max_;

	UnlockMutex__();
}

