
#ifndef __HLD_CTL_HWBUF_H__
#define __HLD_CTL_HWBUF_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#endif

#include	"local_def.h"

#include	"hld_utils.h"

class CHldCtlHwBuf	:	public	CHldUtils
{
private:
	int	reg_rd_interval;
	int	asi_in_is_locked;	//	meas that the playback is normal.
	int	reg_rd_interval_2;
	int	rd_cnt_asi_in_ts;	//	meas that the playback is normal.

#if defined(WIN32)
#else
	pthread_mutex_t *Hw_hMutex;
	void LockFileMutex(void);
	void UnlockFileMutex(void);
#endif
public:
	/* Buffer queue related variables */
	int 	play_buffer_status;
	int 	user_play_buffer_status;
	int 	wait_msec_bank_out;
	_BUF_QUEUE_CONFIG	buf_queue_config[MAX_BANK_NUMBER];

	int		play_buffer;		//	play buffer count in fpga
	int		capture_buffer;	//	cap buffer count in fpga

	__int64	TL_gTotalSendData;	//	tot-bytes of playback.

public:
	CHldCtlHwBuf();
	virtual ~CHldCtlHwBuf();

	void	TL_ClearBuffer(int	ind);

	int	SizeOfBankBlk_CurConfed(void);

	int	RdFpgaCapPlay_BufLevel(int pos);
	int	IsFpgaCapBuf_Underrun_toMinLevel(void);
	int	IsFpgaCapBuf_Underrun_toGivenLevel(void);
	int	IsFpgaPlayBuf_Overrun_toMaxLevel(void);
	int	IsFpgaPlayBuf_Overrun_toMaxLevel_3M(void);
	void	SetHwFifoCntl_(int _play_rec, int _size);
	void	SetHwDmaDiection_LoopThru(void);
	void	SetHwDmaDiection_Play(void);

	int	WaitFullness_IsdbT13CapBuf__HasEndlessWhile(void);
	int	WaitFullness_AtscMhCapBuf__HasEndlessWhile(void);
	int	WaitFullness_IsdbSCapBuf__HasEndlessWhile(void);
	int	WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(int _pos);
	int	WaitConsumePlayBuf_toMaxLevel_DvbT2(int _pos);

	void	ChkLockAsiInput_Reg0x600042(void);
	int IsAsiInputLocked(void);
	void	RdCntAsiInputTs_Reg0x600044(void);
	int BitrateOfAsiInCapedTs(void);

	int initialCntl_for_IsdbT13AtscMh_CaptureMode(void);

	int SetHwOptAndPrepareMonitor_OnMonStart__HasEndlessWhile(void);
	int SetHwOptAndPrepareCapture_OnRecStart__HasEndlessWhile(void);
	int SetHwOptAndPrepareHwToPlay_OnPlayStart__HasEndlessWhile(int _nBankBlockSize, long _p_rate, long _clk_src);

	int WaitDmaTrans__HasEndlessWhile(void);
	int WaitDmaTrans_CaptureDirection__HasEndlessWhile(void);
	int WaitDmaTrans_PlayDirection__HasEndlessWhile(void);
	int WaitDmaTrans_CaptureDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile(void);
	int WaitDmaTrans_PlayDirectionFor_IsdbT13_or_AtscMH__HasEndlessWhile(void);
	void	StartDmaTransfer(int _bnk_size);
	void	StartDmaTransfer_Play(int _bnk_size);
	void	StartDmaTransfer_Play_Loopthru(int _bnk_size);
	void	StartDmaTransfer_Play_Any(void);
	void	StartDmaTransfer_Play_SpecificBuf(DWORD *_buf, int _bnk_size);
	void	*StartDmaTransfer_Capture(int _bnk_size);
	void	*StartDmaTransfer_Capture_Loopthru(int _bnk_size);

	void	FillDmaSrcDta_PlayDirection(unsigned char *_src, int _bnk_size);
	void	UpdDmaDestPos_PlayDirection(DWORD _base, DWORD _inc);

	void	TL_CheckBufferStatus(int written_or_end, long _play_rate);

	int MapTvbBdDestAddr_fromBnkId(void);
	int IncBankAddrHwDest_Sdram(long _pr);
	int IncBankAddrHwDest_Sdram_2(void);
	int SetSdram_BankInfo(int nBankCount, int nBankOffset);
	int SetSdram_BankOffsey_Conf(int nBankConfig);
	int SetSdram_Bank_Conf(int nBankConfig);
	int ApplyPlayRate_Calced_or_UserReqed(long _play_freq_in_herz, long _nOutputClockSource);
	int SdramSubBankSize(void);

#if defined(WIN32)
#else
	void HwBuf_CreateMutex(pthread_mutex_t *_hMutex);
#endif
};


#endif


