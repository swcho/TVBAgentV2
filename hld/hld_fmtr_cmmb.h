
#ifndef __HLD_FMTER_CMMB_H__
#define __HLD_FMTER_CMMB_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_fs_rdwr.h"
#include	"hld_ctl_hwbuf.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"


class CHldFmtrCmmb
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	int m_nRemaindedBlockSize;
	int m_nStartOfMultiplexFrame0, m_nStartOfMultiplexFrame1;
	int LengthOfSubFrame[0x0F];
	int Parameters[0x3F][8];

	char debug_string[100];

	//2012/3/19 TEST
	FILE *fp;

public:


public:
	CHldFmtrCmmb(void);
	virtual ~CHldFmtrCmmb();

	void	SetCommonMethod_6(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void InitCmmbVariables_OnPlayStart(void);
	int ChkCmmbPlayFile_OnPlayCont(void);
	int PlaybackCmmbFile_OnPlayCont__HasEndlessWhile(void);
	void TL_FindStartOfMultiplexFrame(void);
	void StreamAnalyzer(unsigned char*, int, long*, int*);
	int MultipexFrameHeader(unsigned char*, int, int*, int*, int*, int*, int*);
	void MultiplexConfiguration(unsigned char*, int, int*, int*);
	int RunMfs_CmmbParser(char *szFile, char *szResult);


};


#endif

