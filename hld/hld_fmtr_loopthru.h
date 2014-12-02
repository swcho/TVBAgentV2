
#ifndef __HLD_FMTER_LOOPTHRU_H__
#define __HLD_FMTER_LOOPTHRU_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"
#include	"hld_fmtr_atscmh.h"
#include	"hld_fmtr_isdbt13.h"


class CHldFmtrLoopThru	:	public	CHldFmtrAtscMH,	public	CHldFmtrIsdbT13
{
private:
	int	my_hld_id;
	void	*my_hld;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];

	unsigned char	*inter_buf_capplay; 		//	cap buff of IsdbT13 and AtscMH loop-thru.
	unsigned int	inter_buf_rd_ptr;
	unsigned int	inter_buf_wr_ptr;

	double	gSymbolClock;
	__int64 gFreq, gStart, gEnd;
	float	gTime;

	int		gPacketSize;
	
	//2012/7/18 DVB-T2 ASI
	int		nSyncFind;
	int		nEndASI;

public:
	unsigned char	*rd_buf_rmx;
	unsigned int	cnt_byte_availabe_caped;	//	bytes of inter_buf_capplay which is cap-buf.
	unsigned int	test_count_min;
	unsigned int	test_count_max;

public:
	CHldFmtrLoopThru(int _my_id, void *_hld);
	virtual ~CHldFmtrLoopThru();


	void	AllocateCapPlayBufIsdbTAtscMH(void);
	void	FreeCapPlayBufIsdbTAtscMH(void);
	void	SetCommonMethod_8(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);

	void InitCapParam_IsdbT13_AtscMH(void);
	void SetSymClock(double _clk);

	int AdjCapPlayClkSpeed(int pos);

	int SrchSync_CapDta_IsdbT13_AtscMH(int *_nSyncStartPos, int *_iTSize);
	int WrFs_and_FillInterBuf_CapDta_IsdbT13_AtscMH(int nSyncStartPos, int *_RestByte);
	int FillPlayBuf_from_CaptureBuf(void);
	int WrFs_and_FillInterBuf_from_GivenBuf(int _size, unsigned char *_buf);
	void Rd188Captured_RemuedDta(void);
	int RemuxDta_and_PreparePlayBuf_UsingCapDta_IsdbT13_AtscMH(void);
	void CntlFpga_PlayMode_and_WrDta_IsdbT13_AtscMH__HasEndlessWhile(void);

	void ApplySymClock_AdjustedThisTurn(void);

	void	Launch_LoopThruCapPlayTask(void);

	void Set_LoopThru_PacketSize(int _val);
	int Get_LoopThru_PacketSize(void);

	//2012/7/10 DVB-T2 ASI
	void	Launch_LoopThruCapTask(void);
	int		Init_capture_buffer_dvbt2(PVOID param);
	int		WrFs_and_FillInterBuf_CapDta_DvbT2(int nblocksize);
	int		GetAsiInputData(PVOID param, unsigned char *buf, int nBlockSize);
	int		checkSyncByte(int curSyncPos, int packetSize, int nBlockSize/*, char *buf*/);
	int		GetFinishAsi();
	void	SetFinishAsi(int val);

#if defined(WIN32)
	static	void	TaskPlay_and_LoopThru_IsdbT13_AtscMH(PVOID param);
	static	void	TaskCap_and_LoopThru_IsdbT13_AtscMH(PVOID param);
//2012/7/18 DVB-T2 ASI
	static	void	TaskCap_and_LoopThru_DvbT2(PVOID param);
#else
	static	void*	TaskPlay_and_LoopThru_IsdbT13_AtscMH(PVOID param);
	static	void*	TaskCap_and_LoopThru_IsdbT13_AtscMH(PVOID param);
//2012/7/18 DVB-T2 ASI
	static	void*	TaskCap_and_LoopThru_DvbT2(PVOID param);
#endif

};


#endif

