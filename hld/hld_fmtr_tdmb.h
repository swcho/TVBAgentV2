
#ifndef __HLD_FMTER_TDMB_H__
#define __HLD_FMTER_TDMB_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"

#if defined(WIN32)
#else
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef long long __int64;
#endif


class CHldFmtrTdmb
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	int				TL_nRSErrorCnt;
	int				TL_nRSRecoveredCnt;
	int				TL_nLIFormat;

	int				TL_nSFindSync;
	int 			TL_nMBLKCount;

	unsigned char	*TL_bTdmbRslt;		//	TL_pbBuffer
	unsigned char	*TL_bTdmbTmp;		//	TL_pbBufferTmp

	unsigned char	*TL_bTdmbSync;		//	temp buffer. store buffer for G.704 param
	unsigned int	TL_TdmbWIndex;
	unsigned int	TL_TdmbRIndex;
	unsigned int	TL_TdmbBCount;

	unsigned char	*TL_bTdmbPlay_2;	//	TL_sz2ndBufferPlay, 204 byte buffer pool for G.704 LI format and CIF for TDMB
	unsigned int	*TL_nWritePos_TdmbPlay_2;
	unsigned int	*TL_nReadPos_TdmbPlay_2;
	unsigned int	*TL_nBufferCnt_TdmbPlay_2;

public:
	/* ETI NA Sync Buffer Paramters */
	unsigned char	*TL_szBufferNA; //	store buffer for  ETI NA param.
	unsigned int	TL_nNWIndex;
	unsigned int	TL_nNRIndex;
	unsigned int	TL_nNBCount;
	int 			TL_nNFindSync;
	int 			TL_nRecordNA;
	
/* TDMB - ETI ext. */
	int TL_IsNIorNA;
	int TL_NIorNASyncPos;

private:
	int		FillTemp2Buf_from_GivenBuf(unsigned char *_buf, int __siz);
	int		FillRsltBuf_from_Temp2Buf_w_ChkBoundary(int __siz);
	void	SetNewRdAlignPos_of_Temp2Buf(int _skip_bytes);
	int		FillCalcSyncBuf_from_GivenBuf_w_ChkBoundary(unsigned char *_buf, unsigned int __siz);
	void	TL_FillTemp2SyncBuffer(void);
	void	TL_FillNASyncBuffer(void);
	void	TL_ConvertToNI(void);
	void	TL_ConvertNItoCIF(void);
	void	TL_ConvertNAtoCIF(void);
	int		CheckNIorNASync(char *szFilePath, int nLength, int *pSyncPos);
	int		SearchNIorNASync(char *szFilePath, int nLength);

public:
	CHldFmtrTdmb(void);
	virtual ~CHldFmtrTdmb();

	void	SetCommonMethod_7(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void	SetTdmbSyncBuf_Allocated(unsigned char	*_bufsync, unsigned char	*_bufrslt, unsigned char	*_buftmp, unsigned char	*_buf2ndplay);
	void	SetTdmbSyncBuf_Pos(unsigned int	*_w, unsigned int	*_r, unsigned int	*_c);
	void	InitializeNAToNI(bool Init);
	void	TL_InitConvertNItoNA(void);
	int		FillTdmb_CifBuf_OnPlayCont(void);

	//2012/11/8 verify ETI format
	int		Verify_ETI_Format(char *szFilePath);

};


#endif

