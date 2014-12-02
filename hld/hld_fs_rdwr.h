
#ifndef __HLD_FS_RDWR_H__
#define __HLD_FS_RDWR_H__

#if defined(WIN32)
#include	"io.h"
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#endif
#include	"fcntl.h"
#include	"sys/types.h"
#include	"sys/stat.h"

#include	"local_def.h"
#include	"../include/hld_const.h"

#include	"hld_ctl_hwbuf.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"
#include	"hld_utils.h"
#include	"hld_adapt.h"

class CHldFsRdWr	:	public	CHldCtlHwBuf,	public	CHldAdapt
{
private:
	unsigned short	pid_of_ip_ts;

	FILE *fp_bert;
	FILE *fp_read;
	FILE *fp_write;
	FILE *fp_middle_buf;

public:
#if defined(WIN32)
	HANDLE TL_hMutex;
//2012/7/18 DVB-T2 ASI
	HANDLE TL_hBufMutex;
#else
	pthread_mutex_t TL_hMutex;
	pthread_mutexattr_t TL_hMutexAttr;
//2012/7/18 DVB-T2 ASI
	pthread_mutex_t TL_hBufMutex;
	pthread_mutexattr_t TL_hBufMutexAttr;
#endif

/* Play/Capture file handler */
	HANDLE	AP_hFile;
/* Capture related variables */
	char	AP_szfnRecordFile[MAX_PATH];
/* Play related variables */
	struct	tPlayParm	PlayParm;
	int 	next_file_ready;		//	next file name in playlist.
	DWORD	TL_dwBytesRead;		//	byte size of red data at this turn.

/* Offset control */
	DWORD g_StartOffsetL, g_StartOffsetH;
	DWORD g_CurrentOffsetL, g_CurrentOffsetH;
	DWORD g_EndOffsetL, g_EndOffsetH;
	int g_CurrentOffsetChanged;	//	changed scroll bar pos. discontinuouslly.
	int g_SectionRepeated;

//	int TL_gNumLoop;
	int TL_gFirstRead;
	int TL_gBitrate;
	int TL_gSyncPos;

	char g_Current_Target[MAX_PATH];	//	filename, which is opened for playback
	int 	TL_nEndPlayCnt;

	INT64	TL_i64TotalFileSize;	//	bytes size of the file is capture file.

	int TL_nWIndex_IP, TL_nRIndex_IP, TL_nBCount_IP;

	int g_DvbT2_FileCaptureFlag;
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
public:
	int Use_TMCC;
	
//FIXED - ISDB-T Emergency Broadcasting Control
	int Emergency_Broadcasting;

/* Buffers and variables */
	unsigned char	*TL_szBufferPlay;	//	playback buff
	unsigned int	TL_nWIndex;		//	playback wr-buf pointer
	unsigned int	TL_nRIndex;		//	playback rd-buf pointer, pkt-size aligned position.
	unsigned int	TL_nBCount;		//	bytes in playbuf.
	int 			TL_nFindSync;	//	sync ok. if 0.


//	unsigned char	*TL_pbBuffer;
	unsigned char	*TL_pbBufferTmp;		//	temp buffer.

	unsigned char	*TL_szBufferSync;		//	temp buffer-2. store buffer for G.704 param. and also used for ip-temp buffer.
	unsigned int	TL_nSWIndex;
	unsigned int	TL_nSRIndex;
	unsigned int	TL_nSBCount;


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
public:
	CHldFsRdWr();
	virtual ~CHldFsRdWr();

	void InitPlayBufVariables(void);
	void InitPlayAnd2ndPlayBufVariables(void);
	void InitPlayBufVariables_OnRec(void);
	void InitPlayFrameStatusVariables(void);
	void InitHwAndFileSta_OnMon(void);
	void InitVariables_OnRec(void);
	void InitVariables_OnPlay(void);

	DWORD ReadBytes_PlayBuf(void);
	void Set_ReadBytes_PlayBuf(int __vb);
	void SetPlay__FName(char *_file);
	void SetPlayParam_FName(char *_file);
	char *GetPlayParam_FName(void);
	void SetPlayParam_PlayRate(long _r__);
	long GetPlayParam_PlayRate(void);
	void SetPlayParam_OutClkSrc(long _c__);
	long GetPlayParam_OutClkSrc(void);
	void SetPlayParam_Repeat(int _r__);
	int IsTheFileNot_PlayParam_FName(char *_fn);
	int IsT2miFile(void);
	//2011/7/13 added
	int IsT2miFile(char *sz_File);
	//2011/7/21 added
	int IsC2miFile();
	//2011/7/21 added
	int IsC2miFile(char *sz_File);
	int IsNaFile(void);
	int IsEtiFile(void);

	void SetBitRate(int _br__);
	int GetBitRate(void);

	void	CreateFileMutex(void);
	void	DestroyFileMutex(void);
	int		LockFileMutex_Timeout(void);
	void	LockFileMutex(void);
	void	UnlockFileMutex(void);

	//2012/7/18 DVB-T2 ASI
	void	CreateBufferMutex(void);
	void	DestroyBufferMutex(void);
	void	LockBufferMutex(void);
	void	UnlockBufferMutex(void);


	FILE	*_FOpen(char *_fn, char *_mod);
	void	_FSeek(FILE *_f, long _sek, unsigned long _where);
	long	_FTell(FILE *_f);
	long	_FRead(FILE *_f, char *_buf, unsigned long _siz);

#if defined(WIN32)
	void	_FSize_(HANDLE _h, unsigned long *_bytesl, unsigned long *_bytesh);
#else
	void	_FSize_(HANDLE _h, off_t *_bytesl);
#endif
	unsigned long	_FRead_(HANDLE _h, unsigned char *_buf, unsigned long *_bytes);
	unsigned long	_FWrite_(HANDLE _h, unsigned char *_buf, unsigned long _cnt, unsigned long *_bytes);
#if defined(WIN32)
	void	_FSeek_(HANDLE _h, unsigned long _sek, long *_sekh);
#else
	void	_FSeek_(HANDLE _h, off_t _sek);
#endif

	int	IsInvalidFileHandle(void);
	void	Set_RecFileName(char *_file);
	void	OpenFile_OnRecStart(void);
	void	OpenFile_RecMode_1(HANDLE *_h_, char *_file);
	int	OpenFile_PlayMode(void);
	int FOpen_readonly_stdio(char *_file);
	void	FClose__stdio(int _fi);
	void	FClose__StreamFile(HANDLE *_fh);
	int FChk_Existense(char *_file);

	int Fseek_StartngOff_at_UserDefinedOffset(void);
	void	CloseFile_of_OpendFile_Win32(void);
	void	CloseFile_of_OpendFile_Lnx(void);
	void	CloseFile_of_OpendFile(void);
	int	OpenFile_OnPlayStart(void);
	int ChkAndOpenPlayFile_OnPlayCont(void);
	int _Fseek_StartingOff_OnPlayCont(void);
	void _Fseek_CurOff_OnPlayCont(void);
	int _Fseek_EndOff_OnPlayCont(void);
	int Fseek_Current_Offset_UserReq(void);
	int Fseek_SectionRepeat_UserReq(void);
	int FlushBufAndIncLoopnum_OnPlayEnd(void);
	DWORD	Fread_into_PlayBuf_StartingPos(void);
	DWORD	Fread_into_PlayBuf_RunningPos(int _zs);
	DWORD	Fread_into_PlayBuf_RunningPos_1stRead(void);
	int IsEof(void);
	DWORD	_FWrite_TmpBuf(HANDLE _h, long wr_siz, unsigned char *_tmp_);
	void	CreateDummtFile(char *_file_path);
	void	Create_AFileName_at_CurDir(char *_fullpath, char *_fn);

	int	Ready_to_PlayRepeatFile_OnPlayEnd(void);

	int SetCurrentOffset(int nOffsetType, double dwOffset);
	double GetCur_Rec_Pos(void);
	INT64 BytesCapFile(void);
	int SetRepeat_PlayMode(int fRepeatMode);
	int SetFileName_NxtPlay(char *szFile);

	int	TL_CloseRecording(void);
	int	TL_StopPlaying(void);

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
	void Set_TmccUsing(int _val);
	int TmccUsing(void);

	void FindSyncPos_and_FillSyncBuf(int nPacketSize, DWORD* dwBYTEsRead);
	int FillOnePacketToQueue_and_addNull_ifNeed(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp);
	int FillOnePacketToQueue(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp);
	//FIXED - DVB-T2 TS -> T2MI
	int ReadOnePacketFromeQueue(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp);
	unsigned int Fill_Playback_DmaBuffer(int isRecording, DWORD dwBYTEsRead);
	int SetTmccRmx_Mode(long use_tmcc_remuxer);


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
	void Insert_One_188NullPkt_intoPlayBuf(int _cnt);
	void	CpyIpRcvDta_into_TmpBuf(int *_rd_p, int *_cnt_p, unsigned char *_buf, int _cp_siz, unsigned char *_tmp_);
	void	CpyIpRcvDta_into_PlayBuf_188Ts(int _siz, int _c_insertnull, int _c_removenull, unsigned char *_tmp_);
	unsigned short	PidOfIpTs(void);
	int InsertNullPkt_into_IpTempBuf_188Ts_2(unsigned char *_src_, int dta_cnt, int _c_insertnull, int _c_removenull, unsigned char *_tmp_);
	int FillPlayBuf_from_GivenBuf(unsigned char *_buf, int __siz);
	int FillPlayBuf_from_SrcDmaBuf(void);
	int FillPlayBuf_from_GivenBuf_w_ChkBoundary(unsigned char *_buf, DWORD __siz);
	int FillGivenBuf_from_PlayBuf_w_ChkBoundary(unsigned char *_buf, int __siz);
	int FillRsltBuf_from_PlayBuf_w_ChkBoundary(int __siz);
	int FillRsltBuf_from_CalcSyncBuf_w_ChkBoundary(unsigned int __siz);
	int FillTemp2Buf_from_PlayBufSkipPktSize_w_ChkBoundary(void);
	int Temp2Buf_IsNot_Sync_Aligned(void);

	void	SetNewWrAlignPos_of_PlayBuf(int _written_bytes);
	void	SetNewRdAlignPos_of_PlayBuf(int _skip_bytes);
	void	SetNewRdAlignPos_of_BufPlay_2(int _skip_bytes);

	unsigned char	*__BufPlay(void);
	unsigned char	*__BufPlay_Has_RstDtaAllConversion(void);
	unsigned char	*__Buf_Temp(void);
	unsigned char	*__Buf_Temp_2(void);
	unsigned char	*__BufPlay_2(void);
	DWORD	*__Buf_DmaPtr_Transfer_Bidirectional(void);

	unsigned int	*__RdInd_of_BufPlay(void);
	unsigned int	*__WrInd_of_BufPlay(void);
	unsigned int	*__Cnt_of_BufPlay(void);

};

#endif

