
#ifndef __HLD_LLDCALLER_H__
#define __HLD_LLDCALLER_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#endif

#include	"../include/logfile.h"

#include	<time.h>

#if defined(WIN32)
#else
#include	<pthread.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<stdarg.h>
#include	"../include/lld_const.h"
#include	"../include/lld_api.h"

#define LoadLibrary(path)     dlopen((path), RTLD_LAZY);
#define GetProcAddress(handle,module)  dlsym((handle),(module));
#define FreeLibrary(handle)   dlclose((handle));

#endif

#include	"../include/hld_type.h"
#include	"hld_gvar.h"


class CLldCriticalSection;
class CALock
{
public:
	CALock(CLldCriticalSection *critSec);
	virtual ~CALock();
protected:
	CLldCriticalSection *m_pCritSec;
};

class CLldCriticalSection
{
public:
	friend class CALock;
	CLldCriticalSection();
	virtual ~CLldCriticalSection();
protected:
	VOID Lock();
	VOID Unlock();

protected:
#if defined(WIN32)
	CRITICAL_SECTION  m_CritSec;
#else
	int m_CritSec;
#endif
};


class Hlldcaller	:	public CLldCriticalSection
{
public:
	CHldGVar	*__Sta_;
	CHldBdLog	*__CtlLog_;

	int cur_max_playrate_sta;

	//2012/9/28
	int TL_Cont_cnt;

	unsigned char	*TL_pbBuffer;
	DWORD	*TL_pdwDMABuffer;		//	dma address has been attached to dma src/dest address for play/capture.
									//	the buffer space of memory is allocated by LLD.

/* On-board memory bank related variables */
	int 	TL_nIdCurBank;		//	activated dma bank pointer. for play and cap.
	int 	TL_nSubBankIdx;
	DWORD	TL_dwAddrDestBoardSDRAM;	//	dma destination addr. is on the tvb-bd.

private:
	void	InitFunc(void);

public:
	Hlldcaller();
	virtual ~Hlldcaller();

	int		SetDmaPtr_HostSide_Lldallocated(void);
	int		SetDmaPtr_HostSide(void);
	DWORD	*DmaPtr_HostSide(void);

	BOOL	InitialDLL(LPCTSTR str);
	VOID	ReleaseDLL();
	
	//TVB590S
	INT		TSPL_GET_AD9775(long dwCommand);
	INT		TSPL_SET_AD9775(long reg, long data);

	INT		TSPL_GET_AD9787(long dwCommand);
	INT		TSPL_SET_AD9787(long reg, long data);

	/* LLD APIs */
	VOID *	TSPL_READ_BLOCK(long);
	int 	TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, unsigned long dwBuffSize,DWORD* dest);
	INT		TSPL_SET_PLAY_RATE(long play_freq, long nOutputClockSource);
	//USING DDS INC./DEC
	INT		TSPL_SEL_DDSCLOCK_INC(long play_freq);
	INT		TSPL_SET_SOURCE(int nNewTS_source);
	INT		TSPL_GET_CUR_BANK_GROUP(void);
	INT		TSPL_SET_TSIO_DIRECTION(int nDirection);
	INT		TSPL_GET_TSIO_DIRECTION(void);
	//TVB390V8
	INT		TSPL_GET_TSIO_STATUS(int option);
	INT		TSPL_GET_DMA_REG_INFO(int);
	INT		TSPL_SET_DMA_REG_INFO(int, int);
	INT		TSPL_SET_DEMUX_CONTROL_TEST(int);
	//+
	INT		TSPL_SET_SDCON_MODE(int nMode);
	INT		TSPL_RESET_SDCON(void);
	INT		TSPL_I2C_WRITE( unsigned char address, unsigned char *tx_buffer, int txlen, int* actlen);
	INT		TSPL_I2C_READ( unsigned char address, unsigned char *rx_buffer, int maxlen, int* actlen);
	INT		TSPL_II2C_WRITE(unsigned char address, unsigned char *tx_buffer,  int txlen, int* actlen);
	INT		TSPL_II2C_READ(unsigned char address, unsigned char *rx_buffer,  int maxlen, int* actlen);
	INT		TSPL_I2C_WRITEREAD( unsigned char dev_addr, int reg_addr, unsigned char *data, int count, int *actlen);
	INT		TSPL_II2C_WRITEREAD( unsigned char dev_addr, int reg_addr, unsigned char *data, int count, int *actlen);
	INT		TSPL_SET_NIM(int reset,  int lnb_vtg,  int lnb_on_off,  int	lnb_tone);
	INT		TSPL_GET_AUTHORIZATION(void);
	INT		TSPL_SET_SDRAM_BANK_CONFIG(int config);		// yscho0426 : config= 0,1,2,3 ==> 2/4/6/8
	INT		TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig);
	INT		TSPL_INIT_MODULATOR(int modulation_mode, int InputSelection, int CodingMode, long dwfreq, long symbol_freq);
	INT		TSPL_SET_MODULATOR_CODERATE(int CodingMode);
	INT		TSPL_SET_MODULATOR_FREQ(int modulation_mode, long dwfreq, long symbol_freq);
	INT		TSPL_SET_MODULATOR_SYMRATE(int modulation_mode, long dwfreq, long symbol_freq);
	VOID*	TSPL_GET_DMA_ADDR(void);
	INT		TSPL_GET_DMA_STATUS(void);
	INT		TSPL_SET_SDRAM_BANK_INFO(int nBankNumber, int nBankOffset);

	// TVB595V1
	INT		TVB380_SET_BOARD_CONFIG_STATUS(long modulation_mode, long status);
	INT		TSPL_SET_BOARD_LED_STATUS(int status_LED, int fault_LED);
	INT		TSPL_REG_EVENT(void*);
	INT		TSPL_REG_COMPLETE_EVENT(void*);
	INT		TSPL_SET_COMPLETE_EVENT(void);
	INT		TSPL_SET_FIFO_CONTROL(int dma_direction, int dma_size);
	INT		TSPL_GET_FIFO_CONTROL(int dma_direction, int dma_size);

	// TVB590V9 - RF/IF AMP	
	INT		TSPL_GET_BOARD_CONFIG_STATUS(void);	
	INT		TSPL_GET_BOARD_ID(void);
	//TVB590S V2
	INT		TSPL_GET_BOARD_REV(void);
	INT		TVB380_SET_STOP_MODE(
							long stop_mode);				/* 0=no send, 1=send null packet when modulation is stopped */
	//sskim20061020
	INT		TVB380_IS_ENABLED_TYPE(
							long modulator_type);			/* Check that the modulator type
															0=DVB-T, 1=8VSB, 2=QAM-A, 3=QAM-B, 4=QPSk is enabled */
	INT		TVB380_SET_CONFIG(								/* Download the RBF file for each modulator type when that is changed */
							long modulator_type,
							long IF_frequency);				/* 36M, 36.125M, 44M or 70M(QPSK) */
	INT 	TVB380_SET_MODULATOR_FREQ(
							long modulator_type,			/* 0=DVB-T, 1=8VSB, 2=QAM-A, 3=QAM-B, 4=QPSk */
							unsigned long output_frequency,	/* RF channel frequency */ /* 2150MHz support - int -> unsigned long */
							long symbol_rate_or_bandwidth);	/* bandwidth if DVB-T, else symbol rate */
	INT		TVB380_SET_MODULATOR_SYMRATE(
							long modulator_type, 
							unsigned long output_frequency,
							long symbol_rate_or_bandwidth);
	INT		TVB380_SET_MODULATOR_CODERATE(
							long modulator_type,
							long code_rate);
	INT		TVB380_SET_MODULATOR_TXMODE(
							long modulator_type,			/* always 0=DVB-T */
							long tx_mode);					/* 0=2K, 1=8K*/
	INT		TVB380_SET_MODULATOR_BANDWIDTH(
							long modulator_type,			/* always 0=DVB-T */
							long bandwidth,					/* 0=6M, 1=7M, 2=8M */
							unsigned long output_frequency);/* 2150MHz support - int -> unsigned long */
	//2011/11/01 added PAUSE
	INT		TVB380_SET_MODULATOR_OUTPUT(
							long modulator_type,			/*  */
							long output);			/* 0=enable, 1=disable */

	INT		TVB380_SET_MODULATOR_GUARDINTERVAL(
							long modulator_type,			/* always 0=DVB-T */
							long guard_interval);			/* 0=1/4, 1=1/8, 2=1/16, 3=1/32 */
	INT		TVB380_SET_MODULATOR_CONSTELLATION(
							long modulator_type,
							long constellation);
	INT		TVB380_SET_MODULATOR_INTERLEAVE(
							long modulator_type,			/* always 3=QAM-B */
							long interleaving);				/* 0=128-1,...,12=128-8 */
	INT		TVB380_SET_MODULATOR_IF_FREQ(
							long modulator_type,
							long IF_frequency);				/* 36M, 36.125M, 44M or 70M(QPSK) */
	INT		TVB380_SET_MODULATOR_SPECTRUM_INVERSION(
							long modulator_type,
							long spectral_inversion);		/* 0=normal, 1=inversed */

	INT		TVB380_SET_MODULATOR_ATTEN_VALUE(
							long modulator_type,
							double atten_val,				/* 0.0 ~ 31.5, step=0.5 */
							long UseTAT4710);				//2011/6/29 added UseTAT4710
	//2013/5/3 ASI bitrate
	INT		Func_TVB59x_SET_TsPacket_CNT_Mode(int _val);
	INT		Func_TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock);

	INT		Func_TVB59x_Modulator_Status_Control(long modulator, long index, long val);
	INT		Func_TVB59x_Get_Modulator_Status(int _val);

	//2012/9/6 Pcr Restamping control
	INT		TVB59x_SET_PCR_STAMP_CNTL(int _val);

	INT		TVB59x_SET_Output_TS_Type(int _val);

	INT		TVB59x_SET_Reset_Control_REG(int _val);

	//2012/8/29 new rf control
	INT		TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(
							long modulator_type,
							double rf_level_value,
							long *AmpFlag,
							long UseTAT4710);

	//2012/8/29 new rf control
	INT		TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(
							long modulator_type,
							double *rf_level_min,
							double *rf_level_max,
							long UseTAT4710);

	//2012/4/12 SINGLE TONE
	INT		TVB380_SET_MODULATOR_SINGLE_TONE(
						long modulator_type,
						unsigned long freq,
						long singleTone);
	
	//AGC
	INT		TVB380_SET_MODULATOR_AGC(
							long modulator_type,
							long agc_on_off,		/* 0=off, 1=on */
							long UseTAT4710);		//2011/6/29 added UseTAT4710

	//sskim20081009 - RF power level
	DOUBLE		TVB380_GET_MODULATOR_RF_POWER_LEVEL(
							long modulator_type,
							long info_type);				/* ??? */

	INT		TVB380_SET_MODULATOR_PRBS_INFO(long modulator_type,			/* all */
																   long mode,	/* 0=none, 1=2^7-1, 2=2^10-1, 3=2^15-1, 4=2^23-1 */
																   double noise_power);	/* -70 ~ 20 */
	INT		TVB380_SET_MODULATOR_PRBS_MODE(long modulator_type,			/* always 5=TDMB */
													   long mode);					/* 0=none, 1=2^7-1, 2=2^10-1, 3=2^15-1, 4=2^23-1 */
	INT		TVB380_SET_MODULATOR_PRBS_SCALE(long modulator_type,		/* always 5=TDMB */
														double noise_power);	/* -70 ~ 20 */


	INT		TVB380_SET_MODULATOR_INIT_CIF(void);
	INT		TVB380_SET_MODULATOR_RUN_CIF(unsigned char  *szSrc, unsigned char *szDest);
	
	INT 		TVBxxx_DETECT_BD(int _multi_bd);
	INT		TVBxxx_INIT_BD(int _my_num, void *_my_cxt);
	INT 	TVBxxx_GET_CNXT_ALL_BD(void *basic_inform);
	INT 	TVBxxx_GET_CNXT_MINE(void *_cnxt);
	INT 	TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *_from_cnxt);

	INT		TVB380_OPEN(int nInitModulatorType,	int nInitIF);
	INT		TVB380_CLOSE(void);
	INT		TSPL_GET_ENCRYPTED_SN(int type, char* sn);
	INT		TSPL_CHECK_LN(char* ln);
	INT		TSPL_CNT_MULTI_VSB_RFOUT(void);
	//2012/6/28 multi dvb-t
	INT		TSPL_CNT_MULTI_DVBT_RFOUT(void);
	INT		TSPL_CNT_MULTI_QAM_RFOUT(void);

	//I/Q PLAY/CAPTURE
	INT		TSPL_GET_FPGA_INFO(int info);
	
	INT		TSE110_SET_CONFIG(int port_a_mode, 
		int port_b_mode, 
		int tx_clock, 
		int input_port, 
		int output_a_mode, 
		int output_b_mode);
	INT		TSE110_GET_SYNC_STATUS(int mode);
	INT		TSE110_GET_SIGNAL_STATUS(int port);
	INT		TSE110_GET_SYNC_FORMAT(void);

	INT		TSPL_GET_SYNC_POSITION(int mode, 
		int type, 
		unsigned char *szBuf, 
		int nlen, 
		int nlen_srch, 
		int nlen_step);
	INT		TSPL_DO_DEINTERLEAVING(unsigned char *szSrc, unsigned char *szDest);
	INT		TSPL_DO_RS_DECODING(unsigned char *szSrc, 
		unsigned char *szDest, 
		int *format,
		int *err_blk_cnt,
		int *recovered_err_cnt,
		int bypass);
	INT		TSPL_CONVERT_TO_NI(unsigned char  *szSrc, unsigned char *szDest, int format);

	//sskim20070205 - v5.1.0
	INT		TVB380_SET_MODULATOR_DVBH(long modulator_type,//==7
		long tx_mode, 
		long in_depth_interleave, 
		long time_slice, 
		long mpe_fec, 
		long cell_id);
	INT		TSPL_GET_LAST_ERROR(void);

	//sskim20070209 - 4.7.0
	INT TVB380_SET_MODULATOR_PILOT(long modulator_type,//==TVB380_DVBS2_MODE==8
											  long  pilot_on_off);
	//sskim20061128 - v5.0.5 sskim-test
	ULONG TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address);
	INT	TSPL_WRITE_CONTROL_REG(int Is_PCI_Control,unsigned long address,unsigned long dwData);
	//+

	//sskim20070322 - DVB-S2 : Roll-off factor added
	INT TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(long modulator_type,//==TVB380_DVBS2_MODE==8
											  long  roll_off_factor);

	//sskim20080724 - BERT
	INT	TVB380_SET_MODULATOR_BERT_MEASURE(long modulator_type,	/* DVB-T, 8VSB, QAM-A/B, QPSK, ISDB-T */
										  long packet_type);	/* 0=normal(NO BERT), 1=all 0's, 2=all 1's, 3=PRBS 2^15-1, 4=PRBS 2^23-1 */

	//sskim20081010 - DTMB
	INT	TVB380_SET_MODULATOR_DTMB(long modulator_type,			/* DTMB only */
											long constellation, /* 0=4QAM-NR, 1=4QAM, 2=16QAM, 3=32QAM, 4=64QAM */
											long code_rate,		/* 0=0.4, 1=0.6, 2=0.8 */
											long interleaver,	/* 0=52/240, 1=52/720 */
											long frame_header,	/* 0=PN420, 1=PN595, 2=PN945 */
											long carrier_number,/* 0=1(ADTB-T), 1=3780(DTMB) */
											long frame_header_pn,/* 0=Fixed P/N, 1=Rotated P/N */
											long pilot_insertion);/* 0=Disabled, 1=Enabled */

	INT	TVB380_SET_MODULATOR_SDRAM_CLOCK(long modulator_type,	/* */
											long sdram_clock);	/* 0(USE SYSTEM CLOCK, It must be 0 except DTMB), 1(USE SYMBOL CLOCK, DTMB must be 1) */

#if TVB597A_STANDALONE
	INT TVB597F_SET_FLASH(char *szFile);
#endif

	//TSP104
	INT	TSP104_INIT(int nSlotPosition);
	INT	TSP104_OPEN(void);
	INT	TSP104_CLOSE(void);

	INT		TSP104_SELECT_NIM_TYPE(int nNim_Type);
	INT		TSP104_RESET_NIM(int nim_type, 
		int reset, 
		int lnb_vtg, 
		int lnb_on_off, 
		int lnb_tone);

	//TSP104 - demodulator
	INT	TSP104_TVB_TUNE(long dwfreq);
	double TSP104_TVB_GET_SNR(void);
	INT TSP104_TVB_SET_DEMODE_CONF(int mode);
	UCHAR TSP104_TVB_GET_CONST_DATA(int Index);

	INT		TSP104_TOM_TUNE(long dwRfFreq, long dwBandwidth); //Hz
	double	TSP104_TOM_GET_BER(void);
	double	TSP104_TOM_GET_SNR(void);	
	INT		TSP104_TOM_GET_AGC_STATUS(void);
	INT		TSP104_TOM_GET_TPS_STATUS(void);
	INT		TSP104_TOM_GET_FEC_STATUS(void);

	//ISDB-T
	INT		TSP104_TDB_TUNE(long dwRfFreq); //Hz
	double	TSP104_TDB_GET_BER(void);
	double	TSP104_TDB_GET_SNR(void);	
	INT		TSP104_TDB_GET_LOCK_STATUS(void);
	INT		TSP104_TDB_TUNE_WR_RD(long szReg, long szData);

	//TSE110
	INT	TSE110_OPEN(void);
	INT	TSE110_CLOSE(void);

#if	0
	//TMCC REMUXER
	INT	TMCC_STOP_REMUX(void);
	INT	TMCC_START_REMUX(void);
	INT	TMCC_CLOSE_REMUX_SOURCE(void);
	INT	TMCC_OPEN_REMUX_SOURCE(char *file_path);
	PCHAR TMCC_READ_REMUX_DATA(int dwBytesToRead, int *pReadPos, int *pBufferCnt, int *pEndOfRemux);
	INT	TMCC_SET_REMUX_INFO(long btype, long mode, long guard_interval, long partial_reception, long bitrate,
		long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
		long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
		long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
	INT	TMCC_SET_LAYER_INFO(long other_pid_map_to_layer,
		long multi_pid_map,
		long total_pid_count, char* total_pid_info,
		long a_pid_count, char* a_pid_info,
		long b_pid_count, char* b_pid_info,
		long c_pid_count, char* c_pid_info);
	//sskim20080630
	INT	TMCC_SET_REMUX_SOURCE_POSITION(long a_low, long a_high,
		long b_low, long b_high,
		long c_low, long c_high);
	LONG TMCC_GET_REMUX_SOURCE_POSITION(long layer_index, long *high);
	//TMCC REMUXING LIVELY
	INT TMCC_WRITE_REMUX_DATA(unsigned char *pData, int nLen);
#endif

	//TVB593
	INT	TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction);
	INT	TSPL_RESET_IP_CORE(long modulator_type, long reset_control);
	INT	TSPL_SET_MH_MODE(long modulator_type, long mh_mode);
	INT	TSPL_SET_MH_PID(long modulator_type, long mh_pid);
	INT	TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock);
	INT	TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate);

	//	DVB-C2
	INT 	TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource);

//2011/5/4 AD9852 MAX clk
#ifdef WIN32
	INT TSPL_SET_AD9852_MAX_CLOCK(long value);
#endif
	INT TSPL_GET_wr_BUF_STS_MultiBd_n(int _mod_n);	//	0, 1, 2, 3
	INT TSPL_GET_wr_cap_BUF_STS_MultiBd_n(void);
	INT TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n);
	INT 	TSPL_SelMultiModAsiOut_n(int _ts_n); //	0, 1, 2, 3
	INT 	TSPL_SelMultiMod310Out_n(int _ts_n); //	0, 1, 2, 3
	INT 	TSPL_SelMultiModTsOutput_n(int _ctl);
	INT 	TSPL_SelMultiModOperationMode_n(int _ctl);

protected:
	HINSTANCE m_hInstance;
	
	//TVB590S
	FTYPE_INT_LONG							tspl_get_ad9775;
	FTYPE_INT_LONG_LONG						tspl_set_ad9775;

	FTYPE_INT_LONG							tspl_get_ad9787;
	FTYPE_INT_LONG_LONG						tspl_set_ad9787;

	FTYPE_PVOID_LONG						tspl_read_block;
#if defined(WIN32)	
	FTYPE_INT_DWORD_PDWORD					tspl_write_block;
#else
	FTYPE_INT_PDWORD_ULONG_PDWORD		tspl_write_block;
#endif
	FTYPE_INT_INT_INT						tspl_set_sdram_bank_info;
	FTYPE_INT_LONG_LONG						tspl_set_play_rate;
	//USING DDS INC./DEC
	FTYPE_INT_LONG							tspl_sel_ddsclock_inc;
	FTYPE_INT_INT							tspl_set_source;
	FTYPE_INT_VOID							tspl_get_cur_bank_group;
	FTYPE_INT_INT							tspl_set_tsio_direction;
	FTYPE_INT_VOID							tspl_get_tsio_direction;
	//TFVB390V8
	FTYPE_INT_INT							tspl_get_tsio_status;
	FTYPE_INT_INT							tspl_get_dma_reg_info;
	FTYPE_INT_INT_INT						tspl_set_dma_reg_info;
	FTYPE_INT_INT							tspl_set_demux_control_test;
	//+
	FTYPE_INT_INT							tspl_set_sdcon_mode;
	FTYPE_INT_VOID							tspl_reset_sdcon;
	FTYPE_INT_BYTE_PBYTE_INT_PINT			tspl_i2c_write;
	FTYPE_INT_BYTE_PBYTE_INT_PINT			tspl_i2c_read;
	FTYPE_INT_BYTE_PBYTE_INT_PINT			tspl_ii2c_write;
	FTYPE_INT_BYTE_PBYTE_INT_PINT			tspl_ii2c_read;
	FTYPE_INT_BYTE_INT_PBYTE_INT_PINT		tspl_i2c_writeread;
	FTYPE_INT_BYTE_INT_PBYTE_INT_PINT		tspl_ii2c_writeread;
	FTYPE_INT_INT_INT_INT_INT				tspl_set_nim;
	FTYPE_INT_VOID							tspl_get_authorization;
	FTYPE_INT_INT							tspl_set_sdram_bank_config;
	FTYPE_INT_INT							tspl_set_sdram_bank_offset_config;
	FTYPE_INT_INT_INT_INT_LONG_LONG			tspl_init_modulator;
	FTYPE_INT_INT							tspl_set_modulator_coderate;
	FTYPE_INT_INT_LONG_LONG					tspl_set_modulator_freq;
	FTYPE_INT_INT_LONG_LONG					tspl_set_modulator_symrate;
	FTYPE_PVOID_VOID						tspl_get_dma_addr;
	FTYPE_INT_VOID							tspl_get_dma_status;

	// TVB595V1
	FTYPE_INT_LONG_LONG						tvb380_set_board_config_status;
	FTYPE_INT_INT_INT						tspl_set_board_led_status;
	FTYPE_INT_PVOID							tspl_reg_event;
	FTYPE_INT_PVOID							tspl_reg_complete_event;
	FTYPE_INT_VOID							tspl_set_complete_event;
	FTYPE_INT_INT_INT						tspl_set_fifo_control;
	FTYPE_INT_INT_INT						tspl_get_fifo_control;

	// TVB590V9 - RF/IF AMP	
	FTYPE_INT_VOID							tspl_get_board_config_status;
	FTYPE_INT_VOID							tspl_get_board_id;
	//TVB590S V2
	FTYPE_INT_VOID							tspl_get_board_rev;
	FTYPE_INT_LONG							tvb380_set_stop_mode;
	//sskim20061020
	FTYPE_INT_LONG							tvb380_is_enabled_type;
	FTYPE_INT_LONG_LONG						tvb380_set_config;

	//2150MHz support - LONG -> ULONG
	FTYPE_INT_LONG_ULONG_LONG				tvb380_set_modulator_freq;
	FTYPE_INT_LONG_ULONG_LONG				tvb380_set_modulator_symrate;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_coderate;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_txmode;
	//2150MHz support - LONG -> ULONG
	FTYPE_INT_LONG_LONG_ULONG				tvb380_set_modulator_bandwidth;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_guardinterval;
	//2011/11/01 added PAUSE
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_output;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_constellation;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_interleave;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_if_freq;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_spectrum_inversion;
	FTYPE_INT_LONG_DOUBLE_LONG				tvb380_set_modulator_atten_value;		//2011/6/29 added UseTAT4710
	//2012/8/29 new rf control
	FTYPE_INT_LONG_DOUBLE_PLONG_LONG		tvb59x_set_modulator_rf_level_value;
	FTYPE_INT_LONG_PDOUBLE_PDOUBLE_LONG		tvb59x_get_modulator_rf_level_range;

	//2012/9/6 Pcr Restamping control
	FTYPE_INT_INT							tvb59x_set_pcr_stamp_cntl;

	FTYPE_INT_INT							tvb59x_set_output_ts_type;
	//2013/5/3 ASI bitrate
	FTYPE_INT_INT							call_tspl_tvb59x_set_tspacket_cnt_mode;
	FTYPE_INT_PINT_PINT						call_tspl_tvb59x_get_asi_input_rate;

	FTYPE_INT_LONG_LONG_LONG				tvb59x_modulator_status_control;
	FTYPE_INT_INT							tvb59x_get_modulator_status;

	FTYPE_INT_INT							tvb59x_set_reset_control_reg;

	//AGC
	FTYPE_INT_LONG_LONG_LONG				tvb380_set_modulator_agc;		//2011/6/29 added UseTAT4710
	//sskim20081009 - RF power level
	FTYPE_DOUBLE_LONG_LONG					tvb380_get_modulator_rf_power_level;
	FTYPE_INT_LONG_LONG_DOUBLE				tvb380_set_modulator_prbs_info;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_prbs_mode;
	FTYPE_INT_LONG_DOUBLE					tvb380_set_modulator_prbs_scale;
	
	FTYPE_INT_VOID							tvb380_set_modulator_init_cif;
	FTYPE_INT_PUCHAR_PUCHAR					tvb380_set_modulator_run_cif;

	FTYPE_INT_INT					tvbxxx_detect_bd;
	FTYPE_INT_INT_PVOID					tvbxxx_init_bd;
	FTYPE_INT_PVOID					tvbxxx_get_cnxt_all_bd;
	FTYPE_INT_PVOID					tvbxxx_get_cnxt_mine;
	FTYPE_INT_INT_INT_PVOID					tvbxxx_dup_bd;

	FTYPE_INT_INT_INT						tvb380_open;
	FTYPE_INT_VOID							tvb380_close;
	FTYPE_INT_INT_PCHAR						tspl_get_encrypted_sn;
	FTYPE_INT_PCHAR							tspl_check_ln;
	FTYPE_INT_VOID							tspl_cnt_multi_vsb_rfout;
	//2012/6/28 multi dvb-t
	FTYPE_INT_VOID							tspl_cnt_multi_dvbt_rfout;
	FTYPE_INT_VOID							tspl_cnt_multi_qam_rfout;
	
	FTYPE_INT_INT_INT_INT_INT_INT_INT		tse110_set_config;
	FTYPE_INT_INT							tse110_get_sync_status;
	FTYPE_INT_INT							tse110_get_signal_status;
	FTYPE_INT_VOID							tse110_get_sync_format;

	FTYPE_INT_INT_INT_PUCHAR_INT_INT_INT	tspl_get_sync_position;
	FTYPE_INT_PUCHAR_PUCHAR					tspl_do_deinterleaving;
	FTYPE_INT_PUCHAR_PUCHAR_PINT_PINT_PINT_INT tspl_do_rs_decoding;
	FTYPE_INT_PUCHAR_PUCHAR_INT				tspl_convert_to_ni;

	//sskim20070205 - v5.1.0
	FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG tvb380_set_modulator_dvbh;
	FTYPE_INT_VOID							tspl_get_last_error;

	//sskim20070209 - 4.7.0
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_pilot;

	//sskim20061128 - v5.0.5 sskim-test
	FTYPE_ULONG_INT_ULONG					tspl_read_control_reg;
	FTYPE_INT_INT_DWORD_DWORD				tspl_write_control_reg;
	//+

	//sskim20070322 - DVB-S2 : Roll-off factor added
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_roll_off_factor;

	//sskim20080724 - BERT
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_bert_measure;

	//sskim20081010 - DTMB
	FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG tvb380_set_modulator_dtmb;
	FTYPE_INT_LONG_LONG						tvb380_set_modulator_sdram_clock;

	//TSP104
	FTYPE_INT_INT		tsp104_init;
	FTYPE_INT_VOID		tsp104_open;
	FTYPE_INT_VOID		tsp104_close;
	FTYPE_INT_INT_INT_INT_INT_INT			tsp104_reset_nim;
	FTYPE_INT_INT		tsp104_select_nim_type;
	FTYPE_INT_LONG		tsp104_tvb_tune;
	FTYPE_DOUBLE_VOID	tsp104_tvb_get_snr;
	FTYPE_INT_INT		tsp104_tvb_set_demode_conf;
	FTYPE_UCHAR_INT		tsp104_tvb_get_const_data;
	FTYPE_INT_LONG_LONG	tsp104_tom_tune;
	FTYPE_DOUBLE_VOID	tsp104_tom_get_snr;
	FTYPE_DOUBLE_VOID	tsp104_tom_get_ber;
	FTYPE_INT_VOID		tsp104_tom_get_agc;
	FTYPE_INT_VOID		tsp104_tom_get_tps;
	FTYPE_INT_VOID		tsp104_tom_get_fec;

	//ISDB-T
	FTYPE_INT_LONG		tsp104_tdb_tune;
	FTYPE_DOUBLE_VOID	tsp104_tdb_get_snr;
	FTYPE_DOUBLE_VOID	tsp104_tdb_get_ber;
	FTYPE_INT_VOID		tsp104_tdb_get_lock;
	FTYPE_INT_LONG_LONG	tsp104_tdb_tune_wr_rd;

	//TSP104
	FTYPE_INT_VOID		tse110_open;
	FTYPE_INT_VOID		tse110_close;
	
	//TMCC REMUXER
	FTYPE_INT_VOID		tmcc_stop_remux;
	FTYPE_INT_VOID		tmcc_start_remux;
	FTYPE_INT_VOID		tmcc_close_remux_source;
	FTYPE_INT_PCHAR		tmcc_open_remux_source;
	FTYPE_PCHAR_INT_PINT_PINT_PINT  tmcc_read_remux_data;
	FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG	tmcc_set_remux_info;
	FTYPE_INT_LONG_LONG_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR tmcc_set_layer_info;
	//sskim20080630
	FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG tmcc_set_remux_source_position;
	FTYPE_LONG_LONG_PLONG tmcc_get_remux_source_position;
	//ISDB-T ASI INPUT TO RF OUTPUT
	FTYPE_INT_PUCHAR_INT tmcc_write_remux_data;

	//I/Q PLAY/CAPTURE
	FTYPE_INT_INT	tspl_get_fpga_info;

	//TVB593
	FTYPE_INT_LONG_LONG tspl_set_dma_direction;
	FTYPE_INT_LONG_LONG tspl_reset_ip_core;
	FTYPE_INT_LONG_LONG tspl_set_mh_mode;
	FTYPE_INT_LONG_LONG tspl_set_mh_pid;
	FTYPE_INT_LONG_LONG tspl_set_symbol_clock;
	FTYPE_INT_LONG_LONG tspl_set_max_playrate;

#if TVB597A_STANDALONE
	FTYPE_INT_PCHAR tvb597f_set_flash;
#endif

	FTYPE_INT_LONG_LONG_LONG	tspl_set_play_rate_594;

//2011/5/4 AD9852 MAX clk
#ifdef WIN32
	FTYPE_INT_LONG			tspl_set_ad9852_max_clock;
#endif

	FTYPE_INT_INT _GET_wr_BUF_STS_MultiBd_n;
	FTYPE_INT_VOID _GET_wr_cap_BUF_STS_MultiBd_n;
	FTYPE_INT_INT _SEL_TS_TAG_VirtualBd_n;
	FTYPE_INT_INT 	__SelMultiModAsiOut_n;
	FTYPE_INT_INT 	__SelMultiMod310Out_n;
	FTYPE_INT_INT 	__SelMultiModTsOutput_n;
	FTYPE_INT_INT 	__SelMultiModOperationMode_n;

	//2012/4/12 SINGLE TONE
	FTYPE_INT_LONG_ULONG_LONG				tvb380_set_modulator_single_tone;

};

#endif

