
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"hld_fs_rdwr.h"

#define VLC_DUMMY_PATH		"dummy"
#define G_BUFFER_SIZE		(32*0x100000*2)	//Mbytes

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
CHldFsRdWr::CHldFsRdWr(void)
{
	TL_hMutex = PTHREAD_MUTEX_INITIALIZER;
	TL_szBufferPlay = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);
	TL_pbBuffer = (unsigned char *)malloc(SUB_BANK_MAX_BYTE_SIZE);
	TL_pbBufferTmp = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);
	TL_szBufferSync = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);

	AP_hFile = NULL;
	TL_pdwDMABuffer = NULL;

	next_file_ready = FALSE;
	TL_dwBytesRead = 0;

	TL_i64TotalFileSize = 0;

	TL_gNumLoop 		= 0;
	TL_gFirstRead		= 1;
	TL_gBitrate 		= 0;
	TL_gPacketSize		= PKTSIZE;
	TL_gSyncPos 		= 0;
	
/* Offset control */
	g_StartOffsetL = g_StartOffsetH  = 0;
	g_CurrentOffsetL = g_CurrentOffsetH = 0;
	g_EndOffsetL = g_EndOffsetH = 0;
	g_CurrentOffsetChanged = 0;
	g_SectionRepeated = 0;
	

	TL_nEndPlayCnt = 16;

	CreateFileMutex();
	//2012/7/18 DVB-T2 ASI
	CreateBufferMutex();

//////////////////////////////////////////////////////////////////////////////////////
	Use_TMCC = 0;


	Emergency_Broadcasting = 0;

	g_DvbT2_FileCaptureFlag = 0;

	//fp_bert = fopen("c:\\ts\\bert_cap.dat","wb");
	fp_bert = NULL;
	//fp_read = fopen("c:\\ts\\file_read.dat","wb");
	//fp_write = fopen("c:\\ts\\tdmb_test.dat","wb");
	//fp_middle_buf = fopen("c:\\ts\\file_middle.dat","wb");
	fp_read = NULL;
	fp_write = NULL;
	fp_middle_buf = NULL;

}

CHldFsRdWr::~CHldFsRdWr()
{
	if ( TL_szBufferPlay ) free(TL_szBufferPlay);
	if ( TL_pbBuffer ) free(TL_pbBuffer);
	if ( TL_pbBufferTmp ) free(TL_pbBufferTmp);
	if ( TL_szBufferSync ) free(TL_szBufferSync);

	DestroyFileMutex();
	//2012/7/18 DVB-T2 ASI
	DestroyBufferMutex();
	if(fp_bert != NULL)
		fclose(fp_bert);
	if(fp_read != NULL)
		fclose(fp_read);
	if(fp_write != NULL)
		fclose(fp_write);
	
	if(fp_middle_buf != NULL)
		fclose(fp_middle_buf);

}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void CHldFsRdWr::InitPlayBufVariables(void)
{
	TL_nWIndex = 0;		TL_nRIndex = 0;		TL_nBCount = 0;	
	TL_nWritePos = 0;		TL_nReadPos = 0;		TL_nBufferCnt = 0;
	TL_nSyncStatus = 0;

#ifdef TSPHLD0110_EXPORTS
	TL_nSyncStatus = (TSE110_GET_SYNC_STATUS(1)==1)? 1 : 0;
#endif
}
void CHldFsRdWr::InitPlayAnd2ndPlayBufVariables(void)
{
	TL_nWIndex = 0;		TL_nRIndex = 0;		TL_nBCount = 0;		TL_nFindSync = 1;
	TL_nWritePos = 0;		TL_nReadPos = 0;		TL_nBufferCnt = 0;

	TL_nWIndex_IP = TL_nRIndex_IP = TL_nBCount_IP = 0;
}
void CHldFsRdWr::InitPlayBufVariables_OnRec(void)
{
#if defined(WIN32)
	AP_hFile = INVALID_HANDLE_VALUE;
#else
	AP_hFile = NULL;
#endif
	TL_nWIndex = TL_nRIndex = TL_nBCount = 0;
	TL_nFindSync = -1;
}
void CHldFsRdWr::InitPlayFrameStatusVariables(void)
{
	g_FrameDrop = -1;//0;
	TL_gPacketSize = 204;//???
	TSSyncLock = FALSE;
	TL_gTotalSendData	= 0;
	TL_nIdCurBank = 0;
	TL_i64TotalFileSize = 0;
}
void CHldFsRdWr::InitHwAndFileSta_OnMon(void)
{
	TL_nIdCurBank = 0;
	TSSyncLock = FALSE;
	TL_i64TotalFileSize	= 0;
}
void CHldFsRdWr::InitVariables_OnRec(void)
{
	TL_gTotalSendData = 0;
	TL_nIdCurBank = 0;
	TSSyncLock = FALSE;
	TL_i64TotalFileSize	= 0;
}
void CHldFsRdWr::InitVariables_OnPlay(void)
{
	TL_dwBytesRead	= 0;
	TL_nIdCurBank		= 0;
	TL_nSubBankIdx	= 0;
	TL_gNumLoop		= 0;
	TL_gFirstRead		= 1;
	TL_gPacketSize	= 188;
	TL_gSyncPos		= 0;
	TL_gTotalSendData	= 0;
	TL_giiTimeOffset	= 0;
	g_FrameDrop = -1;
	TL_dwAddrDestBoardSDRAM = (TL_nIdCurBank == 0? 0 : BANK_SIZE_4);

}
//////////////////////////////////////////////////////////////////////////////////////
DWORD CHldFsRdWr::ReadBytes_PlayBuf(void)
{
	return	TL_dwBytesRead;
}
void CHldFsRdWr::Set_ReadBytes_PlayBuf(int __vb)
{
	TL_dwBytesRead = __vb;
}

//////////////////////////////////////////////////////////////////////////////////////
void CHldFsRdWr::SetPlay__FName(char *_file)
{
	__CtlLog_->HldPrint_1_s("Hld-Dta. SetPlay__FName", _file);
	memcpy(g_Current_Target, _file, MAX_PATH);
}
void CHldFsRdWr::SetPlayParam_FName(char *_file)
{
	__CtlLog_->HldPrint_1_s("Hld-Dta. SetPlayParam_FName", _file);
	strcpy(PlayParm.AP_lst.szfn, _file); 
}
char *CHldFsRdWr::GetPlayParam_FName(void)
{
	return	PlayParm.AP_lst.szfn; 
}
void CHldFsRdWr::SetPlayParam_PlayRate(long _r__)
{
	__CtlLog_->HldPrint_1("Hld-Dta. SetPlayParam_PlayRate", _r__);
	PlayParm.AP_lst.nPlayRate = _r__;
}
long CHldFsRdWr::GetPlayParam_PlayRate(void)
{
	return	PlayParm.AP_lst.nPlayRate;
}
void CHldFsRdWr::SetPlayParam_OutClkSrc(long _c__)
{
	PlayParm.AP_lst.nOutputClockSource = _c__;
}
long CHldFsRdWr::GetPlayParam_OutClkSrc(void)
{
	return	PlayParm.AP_lst.nOutputClockSource;
}
void CHldFsRdWr::SetPlayParam_Repeat(int _r__)
{
	PlayParm.AP_fRepeat = _r__;
}
int CHldFsRdWr::IsTheFileNot_PlayParam_FName(char *_fn)
{
	return	strcmp(PlayParm.AP_lst.szfn, _fn);
}
int CHldFsRdWr::IsT2miFile(void)
{
	return	(CheckExtention(PlayParm.AP_lst.szfn, (char*)".t2") | CheckExtention(PlayParm.AP_lst.szfn, (char*)".t2mi"));
}
//2011/7/13 added
int CHldFsRdWr::IsT2miFile(char *sz_File)
{
	return	(CheckExtention(sz_File, (char*)".t2") | CheckExtention(sz_File, (char*)".t2mi"));
}
//2011/7/21 added
int CHldFsRdWr::IsC2miFile(void)
{
	return	CheckExtention(PlayParm.AP_lst.szfn, (char*)".c2");
}
//2011/7/21 added
int CHldFsRdWr::IsC2miFile(char *sz_File)
{
	return	CheckExtention(sz_File, (char*)".c2");
}
int CHldFsRdWr::IsNaFile(void)
{
	return	CheckExtention(PlayParm.AP_lst.szfn, (char*)".na");
}
int CHldFsRdWr::IsEtiFile(void)
{
	return	CheckExtention(PlayParm.AP_lst.szfn, (char*)".eti");
}

void CHldFsRdWr::SetBitRate(int _br__)
{
	__CtlLog_->HldPrint_1("Hld-Dta. SetBitRate", _br__);
	TL_gBitrate = _br__;
}
int CHldFsRdWr::GetBitRate(void)
{
	return	TL_gBitrate;
}

//////////////////////////////////////////////////////////////////////////////////////
void	CHldFsRdWr::CreateFileMutex(void)
{
#if defined(WIN32)
	TL_hMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_hMutexAttr);
	pthread_mutexattr_settype(&TL_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_hMutex, &TL_hMutexAttr);
	HwBuf_CreateMutex(&TL_hMutex);
#endif

}
void	CHldFsRdWr::DestroyFileMutex(void)
{
#if defined(WIN32)	
	if ( TL_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_hMutex);
		CloseHandle(TL_hMutex);
		TL_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_hMutexAttr);
	pthread_mutex_destroy(&TL_hMutex);
	TL_hMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
}
int	CHldFsRdWr::LockFileMutex_Timeout(void)
{
#if defined(WIN32)
	if ( WaitForSingleObject(TL_hMutex, 1000) == WAIT_TIMEOUT )
	{
		ReleaseMutex(TL_hMutex);
		return	0;
	}
#else
	pthread_mutex_lock(&TL_hMutex);
#endif
	return	1;
}
void	CHldFsRdWr::LockFileMutex(void)
{
#if defined(WIN32)
	if ( TL_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_hMutex);
#endif
}
void	CHldFsRdWr::UnlockFileMutex(void)
{
#if defined(WIN32)
	ReleaseMutex(TL_hMutex);
#else
	pthread_mutex_unlock(&TL_hMutex);
#endif
}

//2012/7/18 DVB-T2 ASI//////////////////////////////////////////////////////////////////////////////////////
void	CHldFsRdWr::CreateBufferMutex(void)
{
#if defined(WIN32)
	TL_hBufMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_hBufMutexAttr);
	pthread_mutexattr_settype(&TL_hBufMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_hBufMutex, &TL_hBufMutexAttr);
#endif

}
void	CHldFsRdWr::DestroyBufferMutex(void)
{
#if defined(WIN32)	
	if ( TL_hBufMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_hBufMutex);
		CloseHandle(TL_hBufMutex);
		TL_hBufMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_hBufMutexAttr);
	pthread_mutex_destroy(&TL_hBufMutex);
#endif
}
void	CHldFsRdWr::LockBufferMutex(void)
{
#if defined(WIN32)
	if ( TL_hBufMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_hBufMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_hBufMutex);
#endif
}
void	CHldFsRdWr::UnlockBufferMutex(void)
{
#if defined(WIN32)
	ReleaseMutex(TL_hBufMutex);
#else
	pthread_mutex_unlock(&TL_hBufMutex);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////
FILE	*CHldFsRdWr::_FOpen(char *_fn, char *_mod)
{
	return	fopen(_fn, _mod);
}
void	CHldFsRdWr::_FSeek(FILE *_f, long _sek, unsigned long _where)	//	should consider off_t for linux.
{
	fseek(_f, _sek, _where);
}
long	CHldFsRdWr::_FTell(FILE *_f)	//	should consider off_t for linux.
{
	return	ftell(_f);
}
long	CHldFsRdWr::_FRead(FILE *_f, char *_buf, unsigned long _siz)	//	should consider off_t for linux.
{
	return	fread(_buf, 1, _siz, _f);
}

#if defined(WIN32)
void	CHldFsRdWr::_FSize_(HANDLE _h, unsigned long *_bytesl, unsigned long *_bytesh)
#else
void	CHldFsRdWr::_FSize_(HANDLE _h, off_t *_bytesl)
#endif
{
#if defined(WIN32)
	unsigned long	_bl, _bh;

	_bl = GetFileSize(_h, &_bh);
	*_bytesl = _bl;
	*_bytesh = _bh;
#else
	fseeko((FILE *)_h, 0L, SEEK_END);
	*_bytesl = ftello((FILE *)_h);
	fseeko((FILE *)_h, 0L, SEEK_SET);
#endif
}
unsigned long	CHldFsRdWr::_FRead_(HANDLE _h, unsigned char *_buf, unsigned long *_bytes)
{
	*_bytes = 0;
#if defined(WIN32)
	if ( _h != INVALID_HANDLE_VALUE )
	{
		ReadFile(_h, _buf, SUB_BANK_MAX_BYTE_SIZE, _bytes, NULL);
	}
#else
	if ( _h != NULL )
	{
		*_bytes = fread(_buf, 1, SUB_BANK_MAX_BYTE_SIZE, (FILE *)_h);
	}
#endif
	return	*_bytes;
}
unsigned long	CHldFsRdWr::_FWrite_(HANDLE _h, unsigned char *_buf, unsigned long _cnt, unsigned long *_bytes)
{
	*_bytes = 0;
#if defined(WIN32)
	if ( _h != INVALID_HANDLE_VALUE )
	{
		WriteFile(_h, _buf, _cnt, _bytes, NULL);
	}
#else
	if ( _h != NULL )
	{
		*_bytes = fwrite(_buf, 1, _cnt, (FILE* )_h);
	}
#endif
	return	*_bytes;
}

#if defined(WIN32)
void	CHldFsRdWr::_FSeek_(HANDLE _h, unsigned long _sek, long *_sekh)
#else
void	CHldFsRdWr::_FSeek_(HANDLE _h, off_t _sek)
#endif
{
#if defined(WIN32)
	SetFilePointer(_h, _sek, _sekh, FILE_BEGIN);
#else
	fseeko((FILE *)_h, _sek, SEEK_SET);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
int	CHldFsRdWr::IsInvalidFileHandle(void)
{
	int nBertType = _VarTstPktTyp();
	if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_SYNC_187_PRBS_2_23)
		return 0;
#if defined(WIN32)
	if (AP_hFile == INVALID_HANDLE_VALUE)
#else
	if (AP_hFile == NULL)
#endif
	{
		return	1;
	}
	return	0;
}

void	CHldFsRdWr::Set_RecFileName(char *_file)
{
	__CtlLog_->HldPrint_1_s("Hld-Dta. Set_RecFileName", _file);
	strcpy(AP_szfnRecordFile, _file); 
}
void	CHldFsRdWr::OpenFile_OnRecStart(void)
{
#if defined(WIN32)
	AP_hFile = CreateFile(AP_szfnRecordFile,
			GENERIC_WRITE,
			0, 
			NULL, 
			CREATE_ALWAYS, 
			//6.9.16 - fixed
			FILE_FLAG_WRITE_THROUGH, 
			//FILE_FLAG_NO_BUFFERING,
			NULL);
#else
	AP_hFile = fopen(AP_szfnRecordFile, "wb");
#endif

	TL_i64TotalFileSize = 0;
}
void	CHldFsRdWr::OpenFile_RecMode_1(HANDLE *_h_, char *_file)
{
	TL_i64TotalFileSize = 0;

#if defined(WIN32)
	if ( *_h_ != INVALID_HANDLE_VALUE )
	{
		CloseHandle(*_h_);
		*_h_ = INVALID_HANDLE_VALUE;
	}
	*_h_ = CreateFile(_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
#else
	if ( *_h_ != NULL )
	{
		fclose((FILE*)*_h_);
		*_h_ = NULL;
	}
	*_h_ = fopen(_file, "wb");
#endif
}

int	CHldFsRdWr::OpenFile_PlayMode(void)
{
#if defined(WIN32)
	AP_hFile = CreateFile(PlayParm.AP_lst.szfn,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if ( AP_hFile == INVALID_HANDLE_VALUE)
	{
		__CtlLog_->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open a PLAY FILE ", PlayParm.AP_lst.szfn);
		__Sta_->SetMainTask_LoopState_(TH_NONE);
		return 0;
	}
#else
	AP_hFile = fopen(PlayParm.AP_lst.szfn, "rb");
	if ( AP_hFile == NULL)
	{
		__CtlLog_->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open a PLAY FILE ", PlayParm.AP_lst.szfn);
		__Sta_->SetMainTask_LoopState_(TH_NONE);
		return 0;
	}
#endif

	return	1;	
}
int	CHldFsRdWr::FOpen_readonly_stdio(char *_file)
{
	int	fi;
#if defined(WIN32)
	fi = _open(_file, _O_BINARY);
#else
	fi = open(_file, O_RDONLY);
#endif
	return	fi;
}
void	CHldFsRdWr::FClose__stdio(int _fi)
{
#if defined(WIN32)
	_close(_fi);
#else
	close(_fi);
#endif
}
void	CHldFsRdWr::FClose__StreamFile(HANDLE *_fh)
{
#if defined(WIN32)
	if ( *_fh != INVALID_HANDLE_VALUE )
	{
		CloseHandle(*_fh);
		*_fh = INVALID_HANDLE_VALUE;
	}
#else
	if ((FILE*)*_fh != NULL )
	{
		fclose((FILE*)*_fh);
		*_fh	= NULL;
	}
#endif
}
int	CHldFsRdWr::FChk_Existense(char *_file)
{
#if defined(WIN32)
	int 	fi;
#else
	FILE *fi=NULL;
#endif

#if defined(WIN32)
	fi = _open(_file, _O_BINARY);
	if (fi != -1)
#else
	fi = fopen(_file, "rb");
	if (fi != NULL)
#endif
	{
#if defined(WIN32)
		_close(fi);
#else
		fclose(fi);
#endif
		return	1;
	}
	return	0;
}
int	CHldFsRdWr::Fseek_StartngOff_at_UserDefinedOffset(void)
{
	DWORD	dwRet;

	g_CurrentOffsetL = g_CurrentOffsetH = 0;
	if ( g_StartOffsetL > 0 || g_StartOffsetH > 0 )
	{
		g_CurrentOffsetL = g_StartOffsetL;
		g_CurrentOffsetH = g_StartOffsetH;
#if defined(WIN32)
		dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
#endif
	}
	return	1;	
}
int CHldFsRdWr::_Fseek_StartingOff_OnPlayCont(void)
{
	DWORD	dwRet;

	/* CURRENT OFFSET CHANGE */
	g_CurrentOffsetL = g_CurrentOffsetH = 0;
	if ( g_StartOffsetL > 0 || g_StartOffsetH > 0 )
	{
		g_CurrentOffsetL = g_StartOffsetL;
		g_CurrentOffsetH = g_StartOffsetH;

#if defined(WIN32)
		dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
		dwRet = fseek((FILE*)AP_hFile, g_CurrentOffsetL, SEEK_SET);
#endif

		return	0;
	}
#if defined(WIN32)
		dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
		dwRet = fseek((FILE*)AP_hFile, g_CurrentOffsetL, SEEK_SET);
#endif
	return	1;
}
void CHldFsRdWr::_Fseek_CurOff_OnPlayCont(void)
{
	DWORD	dwRet;

#if defined(WIN32)
	dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
	dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
	dwRet = fseek((FILE*)AP_hFile, g_CurrentOffsetL, SEEK_SET);	//	???
#endif
	g_CurrentOffsetChanged = 0;
}
int CHldFsRdWr::_Fseek_EndOff_OnPlayCont(void)	//	chnaged end offset by user
{
	DWORD	dwRet;
	int	_ret = 1;

#if defined(WIN32)
	g_CurrentOffsetL = GetVLFilePointer(AP_hFile, (long*)&g_CurrentOffsetH);
#else
	g_CurrentOffsetL = ftello((FILE*)AP_hFile) & 0xFFFFFFFF;
	g_CurrentOffsetH = (ftello((FILE*)AP_hFile)>>32) & 0xFFFFFFFF;
#endif

	if ( (g_CurrentOffsetH > g_EndOffsetH) || (g_CurrentOffsetH == g_EndOffsetH && g_CurrentOffsetL > g_EndOffsetL ) )
	{
		g_CurrentOffsetL = g_StartOffsetL;
		g_CurrentOffsetH = g_StartOffsetH;
#if defined(WIN32)
		dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
		dwRet = fseek((FILE*)AP_hFile, g_CurrentOffsetL, SEEK_SET);	//	???
#endif
		_ret = 0;
	}
	return	_ret;
}
int	CHldFsRdWr::Fseek_Current_Offset_UserReq(void)
{
	DWORD	dwRet;

	if ( g_CurrentOffsetChanged == 1 )
	{
#if defined(WIN32)
		dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
		dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
#endif
		g_CurrentOffsetChanged = 0;
		return	0;
	}
	return	1;	
}
int	CHldFsRdWr::Fseek_SectionRepeat_UserReq(void)
{
	DWORD	dwRet;

	if ( g_SectionRepeated == 2 )
	{
#if defined(WIN32)
		g_CurrentOffsetL = GetVLFilePointer(AP_hFile, (long*)&g_CurrentOffsetH);
#else
		g_CurrentOffsetL = ftello((FILE*)AP_hFile) & 0xFFFFFFFF;
		g_CurrentOffsetH = (ftello((FILE*)AP_hFile)>>32) & 0xFFFFFFFF;
#endif

		if ( (g_CurrentOffsetH > g_EndOffsetH) || (g_CurrentOffsetH == g_EndOffsetH && g_CurrentOffsetL > g_EndOffsetL ) )
		{
			g_CurrentOffsetL = g_StartOffsetL;
			g_CurrentOffsetH = g_StartOffsetH;
#if defined(WIN32)
			dwRet = SetFilePointer(AP_hFile, (long)g_CurrentOffsetL, (long*)&g_CurrentOffsetH, FILE_BEGIN);
#else
			dwRet = fseeko((FILE*)AP_hFile, (off_t)(g_CurrentOffsetH<<32 + g_CurrentOffsetL), SEEK_SET);
#endif
		}
	}

	return	1;	
}

void	CHldFsRdWr::CloseFile_of_OpendFile_Win32(void)
{
#if defined(WIN32)
	if ( AP_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(AP_hFile);
		AP_hFile = INVALID_HANDLE_VALUE;
	}
#endif
}
void	CHldFsRdWr::CloseFile_of_OpendFile_Lnx(void)
{
#if defined(WIN32)
#else
	if ( (FILE*)AP_hFile != NULL )
	{
		fclose((FILE*)AP_hFile);
		AP_hFile = NULL;
	}
#endif
}
void	CHldFsRdWr::CloseFile_of_OpendFile(void)
{
	CloseFile_of_OpendFile_Win32();
	CloseFile_of_OpendFile_Lnx();
}
int	CHldFsRdWr::OpenFile_OnPlayStart(void)
{
	if (OpenFile_PlayMode() == 0)
	{
		return 0;
	}

/* START OFFSET CHANGE. */
	g_CurrentOffsetChanged = 0;
	if ( g_CurrentOffsetL > 0 || g_CurrentOffsetH > 0 )
	{
		g_CurrentOffsetChanged = 1;
	}
	SetPlay__FName(PlayParm.AP_lst.szfn);

	return	1;	
}

int CHldFsRdWr::ChkAndOpenPlayFile_OnPlayCont(void)
{
	if (OpenFile_PlayMode() == 0)
	{
		return 0;
	}
	Fseek_StartngOff_at_UserDefinedOffset();

	return	1;
}

int CHldFsRdWr::FlushBufAndIncLoopnum_OnPlayEnd(void)
{
	if (IsInvalidFileHandle())
	{
		if (next_file_ready == TRUE)
		{
			TL_CheckBufferStatus(2, GetPlayParam_PlayRate());

			TL_gFirstRead = 1;
			TL_gNumLoop++;
		}
		return 0;
	}

	return	1;
}
DWORD	CHldFsRdWr::Fread_into_PlayBuf_StartingPos(void)	//	TL_szBufferPlay
{
	DWORD	dwRet;

	memset(TL_szBufferPlay, 0x00, SdramSubBankSize());	//	why?
#if defined(WIN32)
	dwRet = ReadFile(AP_hFile, TL_szBufferPlay, SdramSubBankSize(), &(TL_dwBytesRead), NULL);
#else
	TL_dwBytesRead = fread(TL_szBufferPlay, 1, SdramSubBankSize(), (FILE*)AP_hFile);
#endif
	return	TL_dwBytesRead;
}
DWORD	CHldFsRdWr::Fread_into_PlayBuf_RunningPos(int _zs)	//	TL_szBufferPlay
{
	DWORD	dwRet;

//	memset(TL_szBufferPlay + _FIf->TL_nWIndex, 0, _zs);
#if defined(WIN32)
	char str[100];
//	sprintf(str, "Start File pos : %d\n",ftell((FILE*)AP_hFile));
//	OutputDebugString(str);
	dwRet = ReadFile(AP_hFile, TL_szBufferPlay + TL_nWIndex, _zs, &(TL_dwBytesRead), NULL);
	if(fp_read != NULL)
	{
		fwrite(TL_szBufferPlay + TL_nWIndex, 1, TL_dwBytesRead, fp_read);
	}
	if(dwRet == 0)
	{

		sprintf(str, "END of File\n");
		OutputDebugString(str);
	}
#else
	TL_dwBytesRead = fread(TL_szBufferPlay + TL_nWIndex, 1, _zs, (FILE*)AP_hFile);
#endif
	return	TL_dwBytesRead;
}
DWORD	CHldFsRdWr::Fread_into_PlayBuf_RunningPos_1stRead(void)	//	TL_szBufferPlay
{
	TL_gFirstRead = 0;
	//2012/10/17
	//TL_nRIndex = TL_nWIndex;
	return	Fread_into_PlayBuf_RunningPos(SdramSubBankSize());
}
int	CHldFsRdWr::IsEof(void)
{
	if (TL_dwBytesRead < (unsigned int)SdramSubBankSize())
	{
		__Sta_->SetMainTask_LoopState_(TH_END_PLAY);
		return	1;
	}
	return	0;
}
DWORD	CHldFsRdWr::_FWrite_TmpBuf(HANDLE _h, long wr_siz, unsigned char *_tmp_)
{
	DWORD	dwRet;

#if defined(WIN32)
	if ( _h != INVALID_HANDLE_VALUE )
	{
		WriteFile(_h, _tmp_, wr_siz, &dwRet, NULL);
		TL_i64TotalFileSize += dwRet;
	}
#else
	if ( _h != NULL )
	{
		dwRet = fwrite(_tmp_, 1, wr_siz, (FILE *)_h);
		TL_i64TotalFileSize += dwRet;
	}
#endif
	return	dwRet;
}
//////////////////////////////////////////////////////////////////////////////////////
void	CHldFsRdWr::CreateDummtFile(char *_file_path)
{
#ifdef WIN32
	sprintf(_file_path, "%s\\%s", g_szCurDir, VLC_DUMMY_PATH);
#else
	sprintf(_file_path, "%s/%s", g_szCurDir, VLC_DUMMY_PATH);
#endif
	FILE *hFile = fopen(_file_path, "r");
	if ( hFile )
	{
		fclose(hFile);
	}
	else
	{
		hFile = fopen(_file_path, "w+");
		if ( hFile )
		{
			fputs("Don't remove!!!", hFile);
			fclose(hFile);
		}
	}
}
void	CHldFsRdWr::Create_AFileName_at_CurDir(char *_fullpath, char *_fn)
{
#ifdef WIN32
	sprintf(_fullpath, "%s\\%s", g_szCurDir, _fn);
#else
	sprintf(_fullpath, "%s/%s", g_szCurDir, _fn);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


int	CHldFsRdWr::Ready_to_PlayRepeatFile_OnPlayEnd(void)
{
	/* REPEAT PLAY MODE. */
	if (next_file_ready == TRUE)
	{
		TL_CheckBufferStatus(2, GetPlayParam_PlayRate());

		TL_gFirstRead = 1;
		//TL_nRIndex = 0;
		TL_gNumLoop++;

		return 0;
	}

	return	1;
}


int CHldFsRdWr::SetCurrentOffset(int nOffsetType, double dwOffset)
	//	app set (dwOffset = 3),(nOffsetType = 3) at play-start
{
	__int64	ddFileLen = (__int64)dwOffset;
	DWORD dwLow, dwHigh;
	dwLow = (DWORD)((ddFileLen & 0xFFFFFFFF)/1024)*1024;
	dwHigh = (DWORD)(ddFileLen >> 32);

	switch (nOffsetType)
	{
		case 0://start offset
			g_StartOffsetL = dwLow;
			g_StartOffsetH = dwHigh;
			g_SectionRepeated = 1;
			break;

		case 1://current offset
			g_CurrentOffsetL = dwLow;
			g_CurrentOffsetH = dwHigh;
			g_CurrentOffsetChanged = 1;
			break;

		case 2://end offset
			g_EndOffsetL = dwLow;
			g_EndOffsetH = dwHigh;
			g_SectionRepeated = 2;
			break;

		case 3://start/end offset release
			g_CurrentOffsetChanged = 0;
			g_CurrentOffsetL = 0;
			g_CurrentOffsetH = 0;

			g_SectionRepeated = 0;
			g_EndOffsetL = 0;
			g_EndOffsetH = 0;
			g_StartOffsetL = 0;
			g_StartOffsetH = 0;
			break;

		default:
			break;
	}

	return 0;
}
double CHldFsRdWr::GetCur_Rec_Pos(void)
{
	return	(double)(TL_i64TotalFileSize - (SdramSubBankSize())*MAX_BANK_NUMBER*2);
}
INT64 CHldFsRdWr::BytesCapFile(void)
{
	return	TL_i64TotalFileSize;
}

int	CHldFsRdWr::SetRepeat_PlayMode(int fRepeatMode)
{
//	HldPrint("Hld-Bd-Ctl. SET PLAY REPEAT MODE [%i]\n",fRepeatMode);
	PlayParm.AP_fRepeat = fRepeatMode==0?0:1;	// 0 or 1
	return 0;
}
int	CHldFsRdWr::SetFileName_NxtPlay(char *szFile)
{
	if ((szFile != NULL) && (strlen(szFile)<=MAX_PATH))
	{
		if ((!IsInvalidFileHandle()) && (strcmp (PlayParm.AP_lst.szfn, szFile) == 0))
		{
			next_file_ready = TRUE;
			return	0;
		}
		SetPlayParam_FName(szFile);
		next_file_ready = TRUE;
		return	0;
	}

	return	-1;
}

//----------------------------------------------------------------------------------
//
//	Close recording file and turn to Monitor mode
//
int	CHldFsRdWr::TL_CloseRecording(void)
{
	DWORD   dwlMABufSize, dwSizeWritten;
	BOOL    err;
	int	boundary;
	BYTE	dummy[PKTSIZE];

	if (0)
	{
//		dwSizeWritten = _FRmx->Fill_Playback_DmaBuffer(1, DISK_RW_UNIT_SIZE);
//		CloseFile_of_OpendFile();
	}
	else	//	not happend this case.
	{
		boundary = (int)(TL_i64TotalFileSize % PKTSIZE);
		dwlMABufSize = PKTSIZE - boundary;
		dwSizeWritten = dwlMABufSize;
#if defined(WIN32)
		if (AP_hFile != INVALID_HANDLE_VALUE)
		{
			err = WriteFile(AP_hFile, dummy, dwlMABufSize, &dwSizeWritten, NULL);    
			CloseFile_of_OpendFile_Win32();
			if(dwSizeWritten != dwlMABufSize) 
				return -1;	// Fail to write
		}
#else
		if (AP_hFile != NULL)
		{
			dwSizeWritten = fwrite(dummy, 1, dwlMABufSize, (FILE*)AP_hFile);
			CloseFile_of_OpendFile_Lnx();
			if(dwSizeWritten != dwlMABufSize) 
				return -1;	// Fail to write
		}
#endif
	}
	return 0;
}

//----------------------------------------------------------------------------------
//
//	stop playing file
//
int	CHldFsRdWr::TL_StopPlaying()
{
	CloseFile_of_OpendFile();
//	_AHLog->HldPrint("Hld-Bd-Ctl. CLOSE PLAY & START MONITIOR\n");
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
unsigned char	gNullPack[204] = {0x47,0x1F, 0xFF, 0x10,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00};				// 200

void CHldFsRdWr::Set_TmccUsing(int _val)
{
	Use_TMCC = _val;
}
int CHldFsRdWr::TmccUsing(void)
{
	if (Use_TMCC == 1)
	{
		return	1;
	}
	return	0;
}



void CHldFsRdWr::FindSyncPos_and_FillSyncBuf(int nPacketSize, DWORD* dwBYTEsRead)
	//	process TL_szBufferPlay data. and the result has been stored in the buffer TL_sz2ndBufferPlay
{
	int nSyncPos = -1, nSearchStep = 0, nNextIndex, nPacketLen;
	long lTemp = 0;
	unsigned int dwBytesToRead = nPacketSize*10;

	if ( TL_nBCount <= dwBytesToRead )	//	there is not  enough data in the playbuffer.
	{
		return;
	}

	if(TL_dwBytesRead <= 0)
		return;

	__CtlLog_->HldPrint_FindSync_PlayCond(__Sta_->TL_gCurrentModType, TL_nFindSync);

//////////////////////////////////////////////////////////////////////////////////////
_FIND_204_SYNC_SEARCH_:		//	find 204/208 sync.
	if ( TL_nFindSync == 1 )
	{
		do {
			FillRsltBuf_from_PlayBuf_w_ChkBoundary(dwBytesToRead);

			nSyncPos = TL_SyncLockFunction((char*)TL_pbBuffer, dwBytesToRead, &nPacketLen, dwBytesToRead, 3);

			if ( nSyncPos >= 0 && nPacketLen == nPacketSize)
			{
				SetNewRdAlignPos_of_PlayBuf(nSyncPos);
				break;
			}
			SetNewRdAlignPos_of_PlayBuf(dwBytesToRead/2);

		} while ( TL_nBCount > dwBytesToRead );

		if ( nSyncPos < 0 ) 
		{
			//OutputDebugString("Not found sync\n");
			return;
		}
		TL_nFindSync = 0;	//	sync pos is ok of the playback buf.
	}

//////////////////////////////////////////////////////////////////////////////////////
	while ( TL_nBCount > dwBytesToRead )	//	until consume all playbuffer data.
	{
		//FIXED - Bitrate adjustment
		if ( TL_nBufferCnt > MAX_BUFFER_BYTE_SIZE - gfRatio*nPacketSize )
		{
			break;
		}

		FillRsltBuf_from_PlayBuf_w_ChkBoundary(nPacketSize);	//	get one pkt from playbuf.
		SetNewRdAlignPos_of_PlayBuf(nPacketSize);					//	consumed one pkt

//////////////////////////////////////////////////////////////////////////////////////
		//FIXED - ISDB-T Emergency Broadcasting Control
		/* ISDB-T(=9). 204TS TO 204TS */
		if ( __Sta_->IsModTyp_IsdbT_1() )
		{
			if ( ((TL_pbBuffer[PKTSIZE+1] >> 4) & 0x0f) == 0x08 )
			{
				TL_pbBuffer[8] = (((TL_pbBuffer[8]>>2)&0x3F)<<2) + (Emergency_Broadcasting<<1) + (TL_pbBuffer[8]&0x01);
			}
			else
			{
				//if ( (((TL_pbBuffer[PKTSIZE+1] >> 4) & 0x0f) == 0x01) 
				//	|| (((TL_pbBuffer[PKTSIZE+1] >> 4) & 0x0f) == 0x02) 
				//	|| (((TL_pbBuffer[PKTSIZE+1] >> 4) & 0x0f) == 0x03) )
				if ( ((TL_pbBuffer[PKTSIZE+1] >> 4) & 0x0f) != 0x00 ) 
				{
					TL_pbBuffer[PKTSIZE] = (((TL_pbBuffer[PKTSIZE]>>4)&0x0F)<<4) + (Emergency_Broadcasting<<3) + (TL_pbBuffer[PKTSIZE]&0x07);
				}
			}
		}

//////////////////////////////////////////////////////////////////////////////////////
		CheckAdaptation(&TL_pbBuffer[0], nPacketSize);

//////////////////////////////////////////////////////////////////////
#if 1
		//2011/8/3 ISDB-S BERT
		if(!__Sta_->IsModTyp_IsdbS()) 
			TL_BERT_SET_DATA(TL_pbBuffer);	//	BERT
#endif

//////////////////////////////////////////////////////////////////////	insert one pkt to TL_sz2ndBufferPlay
		if ( TL_gPCR_Restamping == 1 && gfRatio > 1.0 )
		{
			FillOnePacketToQueue_and_addNull_ifNeed(*dwBYTEsRead, TL_sz2ndBufferPlay, (int*)&TL_nWritePos, (int*)&TL_nReadPos, (int*)&TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TL_pbBuffer);
		}
		else
		{
			FillOnePacketToQueue(*dwBYTEsRead, TL_sz2ndBufferPlay, (int*)&TL_nWritePos, (int*)&TL_nReadPos, (int*)&TL_nBufferCnt, MAX_BUFFER_BYTE_SIZE, TL_pbBuffer);
		}

		nNextIndex = TL_nRIndex + nPacketSize;
		if ( nNextIndex >= MAX_BUFFER_BYTE_SIZE )
		{
			nNextIndex -= MAX_BUFFER_BYTE_SIZE;
		}

		if (*(TL_szBufferPlay + TL_nRIndex) != 0x47)
		{
			
			//SetNewRdAlignPos_of_PlayBuf(nPacketSize);
			
			if(TL_nRIndex < nPacketSize)
			{
				int _val_tmp;
				_val_tmp = nPacketSize - TL_nRIndex;
				TL_nRIndex = MAX_BUFFER_BYTE_SIZE - _val_tmp;
			}
			else
			{
				TL_nRIndex = TL_nRIndex - nPacketSize;
			}
			TL_nBCount += nPacketSize;
			if(fp_middle_buf != NULL)
				fwrite((TL_szBufferPlay + TL_nRIndex - (188 * 10)),1,188*20,fp_middle_buf);
			TL_nFindSync = 1;
			goto _FIND_204_SYNC_SEARCH_;
		}
	}
}


int CHldFsRdWr::FillOnePacketToQueue_and_addNull_ifNeed(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp)
{
	int j, k;
	WORD	wPID;		
    BYTE	bADExist;
	BYTE	bADLen;
    BYTE    *bData = tp;
	
    //-------------------------------------------------------------------------------------
    // Check Adaptation/PCR check
    if (bData[0] == 0x47)
    {
		wPID = (bData[1]&0x1F)*256+bData[2];
		bADExist = bData[3] & 0x20;
		bADLen   = bData[4];

		//FIXED - ISDB-S  ???
		/*
		//----------------------------------------------------------------
		//--- Adaptation and PCR Check
		//----------------------------------------------------------------
		if (bADExist)
		{
			if (bADLen >= 7)		// min length for PCR or OPCR
			{
				if (bData[5] & 0x10)	// PCR (or OPCR = 0x08)
				{
					double  dblTime;
					int     iNumPkt = (int)(giAddedNullPacket - giFirstPCRNullPacket); //in unit packet
					iNumPkt = iNumPkt * PKTSIZE * 8;        // in unit bits

					dblTime = iNumPkt*1.0 / giTargetBitrate;
					__int64 iiOffset = (__int64) (27000000*dblTime);
					iiOffset = 0;//???
					//FIXED
					Replace_PCR(&bData[6], iiOffset, wPID);
				}
			}
		}
		*/
    }
    
	//-------------------------------------------------------------------------------------
    FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, tp);
    gTotalReadPacket++;
    giAddedNullPacket++;

	//-------------------------------------------------------------------------------------
	if (gStuffRatio[0] >= 10)
	{
        //--------------------------------------------------
		for (j=0; j < gStuffRatio[0]/10;j++)
        {
			FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
		}

        //--------------------------------------------------
		if (gTotalReadPacket % 10 == 0)
		{
			k = gStuffRatio[0] % 10;
			for (j=0; j < k;j++)
            {
               FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
               giAddedNullPacket++;
            }
		}
	} 
	else
	{
		if (gTotalReadPacket % 10 == 0)
		{
			for (j=0; j < gStuffRatio[0];j++)
            {
                FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
                giAddedNullPacket++;
            }
		}
	}

	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 100 == 0)
	{
		for (j=0; j < gStuffRatio[1];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}
    
	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 1000 == 0)
	{
		for (j=0; j < gStuffRatio[2];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
       }
	}

	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 10000 == 0)
	{
		for (j=0; j < gStuffRatio[3];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}

	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 100000 == 0)
	{
		for (j=0; j < gStuffRatio[4];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}
	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 1000000 == 0)
	{
		for (j=0; j < gStuffRatio[5];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}

	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 10000000 == 0)
	{
		for (j=0; j < gStuffRatio[6];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}

	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 100000000 == 0)
	{
		for (j=0; j < gStuffRatio[7];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}
	//-------------------------------------------------------------------------------------
	if (gTotalReadPacket % 1000000000 == 0)
	{
		for (j=0; j < gStuffRatio[8];j++)
        {
            FillOnePacketToQueue(packet_size, buffer, wr, rd, cnt, buf_size, gNullPack);
            giAddedNullPacket++;
        }
	}

    return 1;
}

int CHldFsRdWr::FillOnePacketToQueue(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp)
	//	packet_size : ts-pkt size to fill
	//	buffer : target buffer.
	//	*wr : runningn pointer to write.
	//	*rd : reading pointer.
	//	*cnt : cnt of bytes. plus packet_size.
	//	buf_size : total target buf size.
	//	*tp : buf to insert.
{
	int lTemp;
    if (*wr <= (buf_size - packet_size))
	{
		memcpy(buffer + (*wr), tp, packet_size);
		*wr += packet_size;
	}
	else 
	{
		lTemp = buf_size-(*wr);
		memcpy(buffer + (*wr), tp, lTemp);
		memcpy(buffer, tp + lTemp, packet_size - lTemp);

		*wr += packet_size;
		if (*wr >= buf_size) *wr -= buf_size;
	}
	*cnt += packet_size;
    return 1;
}

//FIXED - DVB-T2 TS -> T2MI
int CHldFsRdWr::ReadOnePacketFromeQueue(int packet_size, unsigned char *buffer, int *wr, int *rd, int *cnt, int buf_size, unsigned char *tp)
	//	packet_size : ts-pkt size to read
	//	buffer : target buffer pool to read from.
	//	*wr : runningn pointer to write.
	//	*rd : reading pointer.
	//	*cnt : cnt of bytes. minus packet_size.
	//	buf_size : total target buf size.
	//	*tp : buf to store the read data.
{
	int lTemp;
    if (*rd <= (buf_size - packet_size))
	{
		memcpy(tp, buffer + (*rd), packet_size);
	}
	else 
	{
		lTemp = buf_size-(*rd);

		memcpy(tp, buffer + (*rd), lTemp);
		memcpy(tp+lTemp, buffer, packet_size - lTemp);
	} 

	(*rd) += packet_size;
	if ((*rd) >= buf_size)
		(*rd) -= buf_size;
	
	(*cnt) -= packet_size;

	//if ( tp[0] != 0x47 )
	//	__CtlLog_->HldPrint( "r=%d, w=%d, c=%d, !\n", (*rd), (*wr), (*cnt));
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
//	
//	Fill play buffer or save stream
//
unsigned int CHldFsRdWr::Fill_Playback_DmaBuffer(int isRecording, DWORD dwBYTEsRead)
	//	1:recording, 0:play... fill the buf TL_pbBuffer by data of (TL_sz2ndBufferPlay + TL_nReadPos)
	//	and write the data to file for recording. or write dma memory TL_pdwDMABuffer for playback.
{
	unsigned int lTemp = 0;
	unsigned int dwBytesToRead = dwBYTEsRead;

#if 0	//test
	char str[100];
	sprintf(str, "TL_nBufferCnt = %d, dwBytesToRead = %d\n",TL_nBufferCnt, dwBytesToRead);
	if(TL_nBufferCnt <= dwBytesToRead)
		OutputDebugString(str);
#endif
	while ( TL_nBufferCnt > dwBytesToRead )
	{
		FillRsltBuf_from_CalcSyncBuf_w_ChkBoundary(dwBytesToRead);
		SetNewRdAlignPos_of_BufPlay_2(dwBytesToRead);

		if ( isRecording == 1 )	// recording
		{
			_FWrite_(AP_hFile, TL_pbBuffer, (unsigned long)dwBytesToRead, (unsigned long*)(&lTemp));
		}
		else if ( isRecording == 0 )	// playing
		{
#if defined(WIN32)
			if ( TL_pdwDMABuffer != NULL )
#endif				
			{
				lTemp = dwBytesToRead;

#if defined(WIN32)
				if (__Sta_->IsAttachedTvbTyp_594())
				{
					//	has virtual slot.
				}
				else if (__Sta_->_CntAdditionalVsbRfOut_593_591s() || __Sta_->_CntAdditionalQamRfOut_593_591s() ||
					 __Sta_->_CntAdditionalDvbTRfOut_593())	//2012/6/28 multi dvb-t
				{
					//	has virtual slot.
				}
				else
				{
					memcpy((void *) TL_pdwDMABuffer, TL_pbBuffer, dwBytesToRead);
					if(fp_bert != NULL)
					{
						fwrite(TL_pbBuffer, 1, dwBytesToRead, fp_bert);
					}
					if(fp_write != NULL)
					{
						fwrite(TL_pbBuffer, 1, dwBytesToRead, fp_write);
					}
				}
#else
				StartDmaTransfer_Play_SpecificBuf((DWORD *)TL_pbBuffer, dwBytesToRead);
#endif
				break;
			}
		}
	}

	return lTemp;
}

int	CHldFsRdWr::SetTmccRmx_Mode(long use_tmcc_remuxer)
{
	//FIXED - ISDB-T Emergency Broadcasting Control
	//Use_TMCC = use_tmcc_remuxer;
	Use_TMCC = (use_tmcc_remuxer&0x01);
	Emergency_Broadcasting = ((use_tmcc_remuxer>>1)&0x01);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
static unsigned char	__NullPkt[188] =
{
    0x47,0x1F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
void CHldFsRdWr::Insert_One_188NullPkt_intoPlayBuf(int _cnt)
{
	for (int i = 0; i < _cnt; i++)
	{
		FillOnePacketToQueue(188, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, MAX_BUFFER_BYTE_SIZE, __NullPkt);
	}
}
void	CHldFsRdWr::CpyIpRcvDta_into_TmpBuf(int *_rd_p, int *_cnt_p, unsigned char *_buf, int _cp_siz, unsigned char *_tmp_)
{
	DWORD	dwRet;

	if ( *_rd_p < (int)(G_BUFFER_SIZE - _cp_siz) )
	{
		memcpy(_tmp_, _buf + *_rd_p, _cp_siz);
		*_rd_p += _cp_siz;
	}
	else
	{
		dwRet = (int)G_BUFFER_SIZE - *_rd_p;
	
		memcpy(_tmp_, _buf + *_rd_p, dwRet);
		memcpy(_tmp_ + dwRet, _buf, _cp_siz - dwRet);
	
		*_rd_p = _cp_siz - dwRet;
	}
	*_cnt_p -= (int)_cp_siz;
}
void	CHldFsRdWr::CpyIpRcvDta_into_PlayBuf_188Ts(int _siz, int _c_insertnull, int _c_removenull, unsigned char *_tmp_)
{
	int	i, i_null, i_sync;
	unsigned char	*_src;

	if (_siz > 0)
	{
		FillOnePacketToQueue((int)_siz, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _tmp_);
		return;
	}

	if ((_c_insertnull == 0) && (_c_removenull == 0))
	{
_fill_:
		FillOnePacketToQueue((int)TL_dwBytesRead, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _tmp_);
		return;
	}

	_src = _tmp_;
	i_sync = -1;
	for (i = 0; i < 188; i++)
	{
		if ((_src[i] == 0x47) && (_src[i + 188] == 0x47) && (_src[i + 188 + 188] == 0x47))
		{
			i_sync = i;
		}
	}
	if (i_sync < 0)
	{
		goto	_fill_;
	}

	if (_c_insertnull)
	{
//		__CtlLog_->HldPrint("Ip-Insert-Null");
		if (i_sync != 0)
		{
			FillOnePacketToQueue(i_sync, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _src);
		}
		Insert_One_188NullPkt_intoPlayBuf(_c_insertnull);
		FillOnePacketToQueue(TL_dwBytesRead - i_sync,
			TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, &_src[i_sync]);
	}
	else if (_c_removenull)
	{
		goto	_fill_;

		FillOnePacketToQueue(i_sync, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _src);
		_src += i_sync;
		for (i = i_sync, i_null = _c_removenull; i < TL_dwBytesRead; )
		{
			if ((TL_dwBytesRead - i) < 188)
			{
				break;
			}
			if (i_null > 0)
			{
				if ((_src[i + 0] == __NullPkt[0]) && (_src[i + 1] == __NullPkt[1]) && (_src[i + 2] == __NullPkt[2]))
				{
					i_null--;
//					__CtlLog_->HldPrint("Ip-Remove-Null");
				}
				else
				{
					FillOnePacketToQueue(188, TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _src);
				}
				i += 188;
				_src += 188;		//	the _tmp_ is NOT circular buf.
			}
			else
			{
				break;
			}
		}
		if (i < TL_dwBytesRead)
		{
			FillOnePacketToQueue(TL_dwBytesRead - i,
				TL_szBufferPlay, (int*)&TL_nWIndex, (int*)&TL_nRIndex, (int*)&TL_nBCount, (int)MAX_BUFFER_BYTE_SIZE, _src);
		}
	}
}
unsigned short	CHldFsRdWr::PidOfIpTs(void)
{
	return	pid_of_ip_ts;
}
int	CHldFsRdWr::InsertNullPkt_into_IpTempBuf_188Ts_2(unsigned char *_src_, int dta_cnt, int _c_insertnull, int _c_removenull, unsigned char *_tmp_)
{
	int	i_null, i_sync, skip_size, remainder_src, j, null_cnt, cpy_running_ptr;
	unsigned char	*_src, *_dst;

	if (dta_cnt <= (188*3))
	{
		memcpy(_tmp_, _src_, dta_cnt);
		return	dta_cnt;
	}
	if ((_c_insertnull == 0) && (_c_removenull == 0))
	{
		memcpy(_tmp_, _src_, dta_cnt);
		return	dta_cnt;
	}

	_src = _src_;
	i_sync = SyncPosAssume188((char *)_src, dta_cnt);
	if (i_sync < 0)
	{
		memcpy(_tmp_, _src_, dta_cnt);
		return	dta_cnt;
	}
	pid_of_ip_ts = ((_src[i_sync + 1] & 0x1F) << 8) | (_src[i_sync + 2] & 0xFF);

	remainder_src = dta_cnt - i_sync;
	_src = _src_;
	_dst = _tmp_;
	if (i_sync != 0)
	{
		memcpy(_dst, _src, i_sync);
		_dst += i_sync;
		_src += i_sync;
	}

	if (_c_insertnull)
	{
		memcpy(_dst, __NullPkt, 188);
		_dst += 188;
		memcpy(_dst, _src, remainder_src);
		return	(dta_cnt + 188);
	}
	else
	{
		for (i_null = i_sync; i_null < (dta_cnt - 188); i_null += 188)
		{
			if ((_src[i_null] == 0x47) && (_src[i_null + 1] == 0x1f) && (_src[i_null + 2] == 0xff) && (_src[i_null + 3] == 0x00))
			{
				break;
			}
		}
		if (i_null >= (dta_cnt - 188))
		{
			memcpy(_tmp_, _src_, dta_cnt);
			return	dta_cnt;
		}
		else
		{
			if (i_null != i_sync)
			{
				memcpy(_dst, _src, i_null - i_sync);
				_dst += (i_null - i_sync);
				_src += (i_null - i_sync);
				remainder_src -= (i_null - i_sync);
				if (remainder_src > 188)
				{
					memcpy(_dst, &_src[188], remainder_src - 188);
				}
				return	(dta_cnt - 188);
			}
			else
			{
				memcpy(_dst, &_src[188], remainder_src - 188);
				return	(dta_cnt - 188);
			}
		}
	}
}

int	CHldFsRdWr::FillPlayBuf_from_GivenBuf(unsigned char *_buf, int __siz)
{
	memcpy(TL_szBufferPlay + TL_nWIndex, _buf, __siz);
	SetNewWrAlignPos_of_PlayBuf(__siz);

	return	__siz;
}
int	CHldFsRdWr::FillPlayBuf_from_SrcDmaBuf(void)
{
	memcpy(TL_szBufferPlay + TL_nWIndex, ((unsigned char *)TL_pdwDMABuffer), SdramSubBankSize());
	SetNewWrAlignPos_of_PlayBuf(SdramSubBankSize());

	return	SdramSubBankSize();
}
int	CHldFsRdWr::FillPlayBuf_from_GivenBuf_w_ChkBoundary(unsigned char *_buf, DWORD __siz)
{
	int	i;

	if (TL_nWIndex <= (MAX_BUFFER_BYTE_SIZE - __siz))
	{
		memcpy(TL_szBufferPlay + TL_nWIndex, _buf, __siz);
		TL_nWIndex += __siz;
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - TL_nWIndex;
	
		memcpy(TL_szBufferPlay + TL_nWIndex, _buf, i);
		memcpy(TL_szBufferPlay, _buf + i, __siz - i);
		TL_nWIndex = __siz - i;
	} 
	TL_nBCount += __siz;

	return	__siz;
}
int	CHldFsRdWr::FillGivenBuf_from_PlayBuf_w_ChkBoundary(unsigned char *_buf, int __siz)
{
	int	i;

	if (TL_nRIndex <= (unsigned int)(MAX_BUFFER_BYTE_SIZE - __siz))
	{
		memcpy(_buf, TL_szBufferPlay + TL_nRIndex, __siz);
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - TL_nRIndex;
	
		memcpy(_buf, TL_szBufferPlay + TL_nRIndex, i);
		memcpy(_buf + i, TL_szBufferPlay, __siz - i);
	} 

	return	__siz;
}
int	CHldFsRdWr::FillRsltBuf_from_PlayBuf_w_ChkBoundary(int __siz)
{
	int	i;

	if (TL_nRIndex <= (unsigned int)(MAX_BUFFER_BYTE_SIZE - __siz))
	{
		memcpy(TL_pbBuffer, TL_szBufferPlay + TL_nRIndex, __siz);
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - TL_nRIndex;
	
		memcpy(TL_pbBuffer, TL_szBufferPlay + TL_nRIndex, i);
		memcpy(TL_pbBuffer + i, TL_szBufferPlay, __siz - i);
	} 

	return	__siz;
}
int	CHldFsRdWr::FillRsltBuf_from_CalcSyncBuf_w_ChkBoundary(unsigned int __siz)
{
	unsigned int	i;

	if (TL_nReadPos <= (MAX_BUFFER_BYTE_SIZE - __siz))
	{
		memcpy(TL_pbBuffer, TL_sz2ndBufferPlay + TL_nReadPos, __siz);
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - TL_nReadPos;
	
		memcpy(TL_pbBuffer, TL_sz2ndBufferPlay + TL_nReadPos, i);
		memcpy(TL_pbBuffer + i, TL_sz2ndBufferPlay, __siz - i);
	} 

	return	__siz;
}

int	CHldFsRdWr::FillTemp2Buf_from_PlayBufSkipPktSize_w_ChkBoundary(void)
{
	int	i, j;

	j = TL_nRIndex;
	j += TL_gPacketSize;
	if (j >= MAX_BUFFER_BYTE_SIZE)
		j -= MAX_BUFFER_BYTE_SIZE;
	
	if (j <= (MAX_BUFFER_BYTE_SIZE - TL_gPacketSize))
	{
		memcpy(TL_szBufferSync, TL_szBufferPlay + j, TL_gPacketSize);
	}
	else 
	{
		i = MAX_BUFFER_BYTE_SIZE - j;
	
		memcpy(TL_szBufferSync, TL_szBufferPlay + j, i);
		memcpy(TL_szBufferSync + i, TL_szBufferPlay, TL_gPacketSize - i);
	}

	return	TL_gPacketSize;
}
int	CHldFsRdWr::Temp2Buf_IsNot_Sync_Aligned(void)
{
	if ( TL_szBufferSync[0] != 0x47 )
	{
		return	1;
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////
void	CHldFsRdWr::SetNewWrAlignPos_of_PlayBuf(int _written_bytes)
{
	TL_nWIndex += _written_bytes;
	TL_nBCount += _written_bytes;
	if ( TL_nWIndex >= MAX_BUFFER_BYTE_SIZE )
		TL_nWIndex = 0;
}
void	CHldFsRdWr::SetNewRdAlignPos_of_PlayBuf(int _skip_bytes)
{
	TL_nRIndex += _skip_bytes;
	if (TL_nRIndex >= MAX_BUFFER_BYTE_SIZE)
		TL_nRIndex -= MAX_BUFFER_BYTE_SIZE;
	TL_nBCount -= _skip_bytes;
}
void	CHldFsRdWr::SetNewRdAlignPos_of_BufPlay_2(int _skip_bytes)
{
	TL_nReadPos += _skip_bytes;
	if (TL_nReadPos >= MAX_BUFFER_BYTE_SIZE)
		TL_nReadPos -= MAX_BUFFER_BYTE_SIZE;
	TL_nBufferCnt -= _skip_bytes;
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned char	*CHldFsRdWr::__BufPlay(void)
{
	return	TL_szBufferPlay;
}
unsigned char	*CHldFsRdWr::__BufPlay_Has_RstDtaAllConversion(void)
{
	return	TL_pbBuffer;
}
unsigned char	*CHldFsRdWr::__Buf_Temp(void)	//	general purpose
{
	return	TL_pbBufferTmp;
}
unsigned char	*CHldFsRdWr::__Buf_Temp_2(void)	//	general purpose
{
	return	TL_szBufferSync;
}
unsigned char	*CHldFsRdWr::__BufPlay_2(void)
{
	return	TL_sz2ndBufferPlay;
}
DWORD	*CHldFsRdWr::__Buf_DmaPtr_Transfer_Bidirectional(void)
{
	return	TL_pdwDMABuffer;
}

unsigned int	*CHldFsRdWr::__RdInd_of_BufPlay(void)
{
	return	&TL_nRIndex;
}
unsigned int	*CHldFsRdWr::__WrInd_of_BufPlay(void)
{
	return	&TL_nWIndex;
}
unsigned int	*CHldFsRdWr::__Cnt_of_BufPlay(void)
{
	return	&TL_nBCount;
}







