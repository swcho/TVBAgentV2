
#ifndef __HLD_FMTER_IQ_H__
#define __HLD_FMTER_IQ_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"


class CHldFmtrIq
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];
	long gBandWidth_Symbolrate;


public:
	unsigned int TL_gFPGA_ID;
	unsigned int TL_gFPGA_VER;
	unsigned int TL_gFPGA_IQ_Play;
	unsigned int TL_gFPGA_IQ_Capture;
	unsigned int TL_gIQ_Play_Capture;
	unsigned int TL_gIQ_Memory_Based;
	unsigned int TL_gIQ_Memory_Size;
	unsigned char* gIQBuffer;
	unsigned long gIQRead;
	unsigned long gIQWrite;
	unsigned long gIQCount;
	unsigned long gIQBufferSize;
	//2011/11/22 IQ NEW FILE FORMAT
	unsigned int TL_gIQ_Capture_Size;
	__int64		 gFreeDiskSize;


public:
	CHldFmtrIq(void);
	virtual ~CHldFmtrIq();

	void	SetCommonMethod_3(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void SetFlag_IqPlayCap(unsigned int _val);
	unsigned int Flag_IqPlayCap(void);
	void Prepare_IQPlayCapBuf_OnRecStart(void);
	void Free_IQPlayCapBuf_OnMonStart(void);
	int Capture_IQData_UntilStopCond__HasEndlessWhile(void);
	int Play_IQData_UntilStopCond__HasEndlessWhile(void);
	int ChangeStaIq_OnPlayEnd(void);
	int SetFpgaModulatorTyp(long modulator_type, long IF_Frequency);
	int TryAlloc_IqMem(int mem_size);
	int SetIqMode(int mode, int memory_use, int memory_size, int capture_size); //2011/11/22 IQ NEW FILE FORMAT added capture_size
	int Play_IQData_UntilStopContPlay__HasEndlessWhile(void);

	void SetSymbolrate(long _val);
	long GetSymbolrate(void);
	__int64 GetFreeDiskBytes(char *strFilename);
};


#endif

