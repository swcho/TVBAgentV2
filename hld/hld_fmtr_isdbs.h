
#ifndef __HLD_FMTER_ISDB_S_H__
#define __HLD_FMTER_ISDB_S_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"
#include	"hld_fmtr_cmmb.h"

class CHldFmtrIsdbS
{
private:
	int	my_hld_id;
	void	*my_hld;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;
	CHldFmtrCmmb	*__Cmmb__;	//	frame analyzer

#if defined(WIN32)
	HANDLE TL_ISDBS_hMutex;
#else
public:
	pthread_mutex_t TL_ISDBS_hMutex;
	pthread_mutexattr_t TL_ISDBS_hMutexAttr;
	pthread_t TL_ISDBS_thread;
private:
#endif

	int tmcc_tx_mode[7];
	int tmcc_tx_slot[7];
	int tmcc_TS_ID_per_slot[MAX_SLOT_COUNT];
	int tmcc_TS_PID_per_TS_ID[MAX_TS_COUNT];
	int nTC8PSK_TS[MAX_TS_COUNT];
	int nQPSK_TS[MAX_TS_COUNT];
	int nBPSK_TS[MAX_TS_COUNT];
	int nTC8PSK_TS_Count;
	int nQPSK_TS_Count;
	int nBPSK_TS_Count;
	int __nSlot[MAX_TS_COUNT];
	int nSlot_Unit[MAX_TS_COUNT];
	int nSlot_Valid[MAX_TS_COUNT];
	int nSlot_Dummy[MAX_TS_COUNT];
	int nConst[MAX_TS_COUNT];
	int nCode[MAX_TS_COUNT];

	FILE	*fp_mts[MAX_TS_COUNT];
	//2012/9/26
	int		nPacket_Size__[MAX_TS_COUNT];

	char	TS_path[MAX_TS_COUNT][1024];
	int	StuffRatio[MAX_TS_COUNT][10];
	int	AddedNullPacket[MAX_TS_COUNT];
	unsigned int	TotalReadPacket[MAX_TS_COUNT];
	float	fRatio[MAX_TS_COUNT];
	double	SR_mts, SE_mts, CR_mts;

	unsigned char	*ts_buf_superframe;
	int	off__ts_buf_superframe;
	unsigned char tail_superframe[8][12];
	int	tmcc_change_indicator;

	unsigned char	*ts_buf_to_process_dta;
	int	nWritePos, nReadPos, nBufferCnt;	//	pos variables of the ts_buf_to_process_dta
	int	off__ts_buf_to_process_dta;

	unsigned char	*ts_buf_cmbnd_rslt;

	int	nFrameNum;
	int	nSlotNum;

	unsigned long	prev_cnt_process_dta_frm_layer;
	unsigned long	tot_cnt_process_dta_frm_layer;
	unsigned long	elap_tm_to_calc_runtime_br;
	unsigned long	prev_clk_to_calc_runtime_br;
	unsigned long	_runtime_br;

	unsigned char	tmcc_replace_mode[4];
	unsigned char	tmcc_replace_mode_slot[4];
	unsigned char	replaced_mode_and_act_info[48];
	unsigned char	tmcc_change_flag_;
	unsigned char	tmcc_change_flag_old_;

	FILE	*fp_dump;
	FILE	*fp_dump2;

public:
	int TL_ISDBS_PumpingThreadDone;
	int TL_ISDBS_ConvertingThreadDone;

	int nCombinedTS;
//2011/4/4 ISDB-S Bitrate
	char gszISDBS_baseTS[512];

//ISDB-S
	int TL_combiner_ready;
	int TL_combiner_TS_count;
	unsigned char m_TS_path[MAX_TS_COUNT][1024];	//	file list, given values by user.
	int m_nConst[MAX_TS_COUNT];
	int m_nCode[MAX_TS_COUNT];
	int m_nSlot[MAX_TS_COUNT];
	int TL_nFrameNum;
	int TL_nSlotNum;

	unsigned char	*TL_sz3rdBufferPlay;	//	buffer pool of combined ts(ISDB-S), combined ts may stored in  the pool before writing HW.
	unsigned int	TL_nWritePos3;			//	writing pointer of TL_sz3rdBufferPlay
	unsigned int	TL_nReadPos3;			//	reading pointer of TL_sz3rdBufferPlay
	unsigned int	TL_nBufferCnt3; 		//	the byte-cnt of data residue in TL_sz3rdBufferPlay.

private:
	unsigned long	_clk_msec_(void);
	void	EstimateRuntimeBr(int init, unsigned long cnt_fram_layer);
	FILE	*OpenSource(char *_filen);
	int	RdOnePktIp(unsigned char *buf, int raw_packet_size);
	int	RdOnePktAsi(unsigned char *buf, int raw_packet_size);
	int	RdOnePkt(FILE *pb, unsigned char *buf, int raw_packet_size, int act_ts_id, char *base_tsf);
	int	FillOnePkt_intoProcessBuf(int single_ts, int act_ts_slot);
	int	FillTailSuperframe(void);
	void	GetUserParam_AtCombinerRestart(void);
	void	SwapUserParam_forQpsk_AtCombinerRestart(int single_ts);
	void	InitSlotConf_AtCombinerRestart(void);
	void	GetTmccMode_AtCombinerRestart(void);
	void	GetTsPid_AtCombinerRestart(void);
	void	CalcStuffRatio(int _ts_slot);
	void	CalcStuff_forMTs_AtCombinerRestart(void);
	void	add_frame_info(unsigned char* buf, int *cur_kk, int *cur_nSlotNum, int *cur_nFrameNum, int is_dummy_slot, int modulation, int code_rate);

	FILE	*FopenCombinedFile_AtReplaceRestart(int *_pkt_size);
	void	ParseTmcc_ofCombinedFile(int _pkt_size, unsigned char *_ts);
	void	GetReplacedModeAndActInfo(void);

public:
	CHldFmtrIsdbS(int _my_id, void *_hld);
	virtual ~CHldFmtrIsdbS();

	void	AllocateCapPlayBufIsdbS(void);
	void	FreeCapPlayBufIsdbS(void);
	void	SetCommonMethod_4(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__,
		CHldFmtrCmmb	*__framalyzer__);
	void InitIsdbSPlayBufVariables(void);
	void	CreateIsdbSMutex(void);
	void	DestroyIsdbSMutex(void);
	void	LockIsdbSMutex(void);
	void	UnlockIsdbSMutex(void);
	void InitIsdbSVariables_OnPlayStart(void);
	void TL_ConvertToCombinedTS__HasEndlessWhile(int single_ts);

	void TL_ReplaceCombinedTS__HasEndlessWhile();

	//2011/8/4 ISDB-S ASI
	void TL_ReplaceCombinedTS_IN_ASI__HasEndlessWhile(int nPacketSize, DWORD* dwBYTEsRead);

	int read_superframe(FILE *pb, unsigned char *buf, int raw_packet_size);
	int CalcNullStuffCount(unsigned int totalReadPacket,  int *stuffRatio);
	int SetIsCombinedTs_IsdbS(char* ts_path);
	int CalcPlayRate_SpecifiedFile(char *szFile, int iType);
	int CalcPlayRate_IsdbS_SpecifiedFile(char* ts_path);
	int SetCombinerInfo_IsdbS(int ts_count, char *ts_path, long modulation, long code_rate, long slot_count);
	int SetBaseTs_IsdbS(char *ts_path);
	int Isdbs_FramedTS_GetPacketSize(FILE *pb);
	void TL_WRITE_BLOCK(void);
	int IsdbS_ContFilePlayback(void);

	void	LaunchIsdbS_WrTask(void);

	void	Get_NIT_Satellite_Info(int *descriptor_flag, int *freq, int *rolloff, int *modulation_system, 
									int *moduration, int *symbolrate, int *coderate);
	void	Get_NIT_Cable_Info(int *descriptor_flag, int *freq, int *moduration, int *symbolrate, int *coderate);
	void	Get_NIT_Terrestrial_Info(int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod);

#ifdef WIN32
	static	void HLD_ISDBS_PumpingThread(PVOID param);
#else
	static	void* HLD_ISDBS_PumpingThread(PVOID param);
#endif


//CKIM A 20120827 {
	void	Get_NIT_Satellite_Info2(int *descriptor_flag, int *freq, int *rolloff, int *modulation_system, 
									int *moduration, int *symbolrate, int *coderate);
	void	Get_NIT_Cable_Info2(int *descriptor_flag, int *freq, int *moduration, int *symbolrate, int *coderate);
	void	Get_NIT_Terrestrial_Info2(int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod);
//CKIM A 20120827 }
};


#endif

