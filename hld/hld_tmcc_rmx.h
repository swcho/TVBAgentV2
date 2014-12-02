
#ifndef __HLD_TMCC_RMX_H__
#define __HLD_TMCC_RMX_H__

#include "../include/common_def.h"
#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif


/////////////////////////////////////////////////////////////////
#if	0
enum TMCC_CONSTELLATION
{
	ISDBT_MOD_DQPSK = 0,
	ISDBT_MOD_QPSK,
	ISDBT_MOD_16QAM,
	ISDBT_MOD_64QAM,
};

enum TMCC_CODERATE
{
	
	ISDBT_RATE_1_2 = 0,
	ISDBT_RATE_2_3,
	ISDBT_RATE_3_4,
	ISDBT_RATE_5_6,
	ISDBT_RATE_7_8,
};

enum TMCC_BTYPE
{
	ISDBT_BTYPE_TV = 0,
	ISDBT_BTYPE_RAD1,
	ISDBT_BTYPE_RAD3,
	ISDBT_BTYPE_TV_1SEG,
};

enum TMCC_GUARD_INTERVAL
{
	ISDBT_GUARD_1_32 = 0,
	ISDBT_GUARD_1_16,
	ISDBT_GUARD_1_8,
	ISDBT_GUARD_1_4
};

enum TMCC_INTP
{
	ISDBT_INP_TS188 = 0,
};
#endif

/* */
struct LayerPars_
{
	int m_NumberOfSegments;
	int m_Constellation;
	int m_CodeRate;
	int m_TimeInterleave;
	int m_TpPerFrame;
	double m_Bps;
	double m_Selected_Bps;
};

struct InParams
{
	int m_BrodcastType;
	int m_Mode;
	int m_GuardInterval;
	int m_PartialRecev;
	struct LayerPars_ m_Layers[3];
	double m_Bps;
};

struct OutParams
{
	int m_TotalTpPerFrame;
	int m_DelayOfSymbol;
	int m_TpPerFrame;	//	total sum of all layer
	double m_Bps;
	struct LayerPars_ m_Layers[3];
	double m_FrameLength;
};

/////////////////////////////////////////////////////////////////
#if defined(WIN32)
#else
#define HANDLE void*
typedef long long __int64;
#endif

//#define _SEARCH_PCR_
#ifdef _SEARCH_PCR_
#include	<time.h>
#define CHECK_TIME_START	QueryPerformanceFrequency((_LARGE_INTEGER*)&gFreq);QueryPerformanceCounter((_LARGE_INTEGER*)&gStart);
#define CHECK_TIME_END		QueryPerformanceCounter((_LARGE_INTEGER*)&gEnd);gTime = (float)((double)(gEnd-gStart)/gFreq*1000);
#endif

/////////////////////////////////////////////////////////////////
#define MAX_BUFFER_BYTE_SIZE_layered	(SUB_BANK_MAX_BYTE_SIZE*8)
#define MAX_BUFFERED_READ_SIZE	(SUB_BANK_MAX_BYTE_SIZE*3)
#define DUMMY_CNT		256
#define PACKET_LEN	204

/////////////////////////////////////////////////////////////////
class CHldTmccRmx
{
private:
	int	my_hld_id;
	void	*my_hld;

	int	_188_loopthru_mode;

	double fraction_accumulation; 

#if defined(WIN32)
#else
	pthread_mutex_t	TL_hMutex;
	pthread_mutexattr_t	TL_hMutexAttr;
	pthread_t	TL_thread;
	pthread_t	TL_TMCC_thread;
#endif

	int	TL_nPositionChanged;	//	changed file position on scroll bar.
	long	TL_a_low;
	long	TL_a_high;
	struct InParams	TL_InParams;	//	user param.
	struct OutParams	TL_OutParams;	//	playback param caculated using TL_InParams
	int	TL_ThreadDone;
	int	TL_EndOfRemux;
	
	unsigned char	*RsltDta_ReadByHwWriter;
	unsigned int	TL_nWritePos;
	unsigned int	TL_nReadPos;
	unsigned int	TL_nBufferCnt;
	unsigned char	*RsltDta_UsrSpace;

	unsigned char	*InputDta188Pkt_WrByHwCapturer;
	unsigned int	InputDta188Pkt_WrPos;
	unsigned int	InputDta188Pkt_RdPos[3];
	unsigned int	InputDta188Pkt_Cnt[3];
	unsigned int	InputDta188Pkt_initial_sync_pos;
	unsigned int	InputDta188Pkt_padding_cond;

	unsigned int	InputDta188Pkt_fullness;
	unsigned int	InputDta188Pkt_fullness_Criteria;
	unsigned char	*InputDta188Pkt_SaveFs;

	HANDLE	g_hMutex;
	HANDLE	gLayerFile[3];
	FILE	*gOutputFile;
	bool	gEndOfFile;
	int	gPacketSize;
	char	*gOptionArgs;			
	int	gOptionIndex;
	int	*gLayerPidInfo[4];	//	pid-info of each layer. this info are setted by user.
	int	gLayerPidCount[4];	//	cnt of pids of each layer. a/b/c-layer and total. this info are setted by user.
	int	gLayerOtherPidMap;//0=Layer A, 1=Layer B, 2=Layer C, 3(-1)=Ignore. means that all pid donot defined in gLayerPidInfo[] will be processed as undefined pid-list.
	int	gLayerMultiPidMap;

	unsigned char	*gBF[3];		//	buffer to save red data for each layer.
	int	gRD[3];	//	rd pointer of gBF. this position may be at the sync-pos always.
	int	gWD[3];
	int	gCT[3];	//	cnt of available data in gBF.

	unsigned char	*gBF2[3];	//	inter-buf to mix real and dummy pkt.
	int	gRD2[3];
	int	gWD2[3];
	int	gCT2[3];
	int	gwPID[3];	//	tmp val
	int	gPkt0[3];
	int	gPkt1[3];
	__int64	giiPCR0[3];
	__int64	giiPCR1[3];
	int	gDummyCount;	//	active cnt of gDummyPkt.
	int	gDummyTotal;
	int	gDummyPkt[DUMMY_CNT];	//	cnt of dummy-pkt which may be added into out stream to match output bitrate of each pid-slot. the DUMMY_CNT is max-pid-slot.
	int	gDummyPos[DUMMY_CNT];	//	the position of orig-stream. to fill null pkt at here.
	int	gDummyBufPos;
	int	gDummyBufLen;
	unsigned char	*gDummyBuf;
	unsigned char	*gBF_layered[3];
	int	gRD_layered[3];
	int	gWD_layered[3];
	int	gCT_layered[3];
	int	gUseLayeredBuffer;
	unsigned char	gDataBuffer[MAX_BUFFER_BYTE_SIZE];
	int	gData_Read;
	int	gData_Write;
	int	gData_Count;

#ifdef _SEARCH_PCR_
	__int64	gFreq, gStart, gEnd;
	float	gTime;
	__int64	a_pcr_0;
	__int64	a_pcr_1;
	int	a_ts;
	int	a_ts_add;
	int	a_pcr_pid;
	unsigned char	null_tp[204];
#endif

public:

private:
	void	CreateMutex__(void);
	void	LockMutex__(void);
	void	UnlockMutex__(void);
	void	DestroyMutex__(void);
	int IsValidHandle(int lay_ind);
	void	CloseTs(int _lay);
	int OpenTs(char *szFilePath);
	void	SeekTs(int _lay, long _low, long *_high, long _high_, long _low_);	//	off_t
	int Fread(int _lay, unsigned char *_buf, unsigned long _size, unsigned long *_red_size);
	void	SetEof(int idx);
	int AllocDummyBuf(int size);
	void	FreeDummyBuf(void);
	void	AllocRsltBuf(void);
	void	FreeRsltBuf(void);
	void	AllocInputBuf(void);
	void	FreeInputBuf(void);
	int AllocInterBuf(int _lay);
	void	FreeInterBuf(int _lay);
	int TL_SyncLockFunction(char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step) ;
	int FindSyncPosAtLayerBuf_and_SetgRDVal(int idx);	//	layer index
	int GET_PCR(unsigned char* pPacket, __int64* pPCR);
	void ReinitAtScroll(void);
	void TsAttrPktSize(void);
	int RdOneRawPktFromLayerBuffer(int lay_idx, unsigned char *_buf, int _rd_pos, int _pkt_size);
	int RdOneSyncPktFromLayerBuffer(int lay_idx, unsigned char *_buf, int _pkt_size);
	void QueueLayerBufferFromGivenBuf(int lay_idx, unsigned char *buffer, unsigned long size);
	void QueueLayerBufferFromDummyBuf(int lay_idx, int _pkt_size);
	int QueueLayerBufferFromRedTsData(int lay_idx);
	int ChkDtaAvailable(int _lay);
	void InitDummyInsertCond(void);
	int ReformatOrigTs_w_DummyPkt(int idx, int _pkt_size);
	int CalcBitrateAndDummyCond(int lay_idx, int rd_pos, int pkt_size, long pid_of_this_pkt, __int64 val_pcr_of_this_pkt);
	int RdDtaFromRsltBuffer(int _rd_size);
	int WrDtaIntoRsltBuffer(int _wr_size, unsigned char *_wr_dta);
	int RdDtaFromInputBuffer(int _lay, int _rd_size, unsigned char *_rd_dta);
	int WrDtaIntoInputBuffer_EachLayer(int _lay, int _wr_size, unsigned char *_wr_dta);
	int WrDtaIntoInputBuffer(int _wr_size, unsigned char *_wr_dta);
	int FILE_READ(void *buffer, size_t size, size_t count, int idx);
	void  SetBits(unsigned char* pSrc, int* pIndex, int k, int nBits);
	void  CreateTmccInfo(unsigned char*  pTmccInfo, struct InParams  Pars);
	void  GetPacketLayerBuf(int nLayerIndex, unsigned char*  pPacket);
	void  GetPacket(int nLayerIndex, unsigned char*  pPacket);
	int GetParams(struct OutParams* pOutPars, struct InParams* pPars);
	void  CreateRemuxedTs(FILE*  OutFile, struct InParams	Pars, struct OutParams	OutPars);
#if defined(WIN32)
	static void	MainLoop(void* param);
#else
	static void	*MainLoop(void* param);
#endif

public:
	CHldTmccRmx(int _my_id, void *_hld);
	virtual ~CHldTmccRmx();

	void	_MainLoop(void);
	int TMCC_START_REMUX(void);
	int TMCC_STOP_REMUX(void);
	int TMCC_SET_REMUX_INFO(
		long btype, long mode, long guard_interval, long partial_reception, long bitrate,
		long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
		long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
		long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
	int TMCC_OPEN_REMUX_SOURCE(char *szFilePath);
	int TMCC_CLOSE_REMUX_SOURCE(void);
	char	*TMCC_READ_REMUX_DATA(int dwBytesToRead, int *pReadPos, int *pBufferCnt, int *pEndOfRemux);
	int TMCC_SET_LAYER_INFO(
		long other_pid_map_to_layer,
		long multi_pid_map,
		long total_count, char* total_pid_info,
		long a_pid_count, char* a_pid_info,
		long b_pid_count, char* b_pid_info,
		long c_pid_count, char* c_pid_info);
	int TMCC_SET_REMUX_SOURCE_POSITION(long a_low, long a_high, long b_low, long b_high, long c_low, long c_high);
	long TMCC_GET_REMUX_SOURCE_POSITION(long layer_index, long* high);
	int TMCC_WRITE_REMUX_DATA(unsigned char *pData, int nLen);
	void TMCC_SET_188_LOOPTHRU_MODE(int val);
	void TMCC_WR_188_LPTHRU_CAPTURE_DTA(int _wr_size, unsigned char *_wr_dta);
	int TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(unsigned char *_fn);
	//2012/8/23
	int TMCC_WR_188_LPTHRU_CAPTURE_DTA_FS(struct _TSPH_TS_INFO **ppTsInfo);
	int TMCC_WR_188_LPTHRU_CAPTURE_DTA_FULL(void);
	
	void TMCC_init_InputDta188Pkt_fullness(void);


};


#endif

