#ifndef	__INCLUDE_LLD_API_	
#define	__INCLUDE_LLD_API_

#ifdef WIN32
#else
#define	_stdcall
typedef unsigned int DWORD;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LLD_WIN_BUILD
int 	_stdcall	TSPL_GET_BOARD_ID(void);
int 	_stdcall	TSPL_GET_AUTHORIZATION(void);				/* No implemented */
void*	_stdcall	TSPL_GET_DMA_ADDR(void);
void*	_stdcall	TSPL_READ_BLOCK(long dwBlockSize);			/* DMA size, up to 1MBytes */
#ifdef WIN32
int 	_stdcall	TSPL_WRITE_BLOCK(long dwBuffSize,			/* DMA size, up to 1MBytes */
									 long *dest);				/* always 0 */
#else
int 	TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, unsigned long dwBuffSize,DWORD* dest);
#endif
int		_stdcall	TSPL_GET_DMA_STATUS(void);
int		_stdcall	TSPL_SET_PLAY_RATE(long play_freq,			/* bps */
							 long nUseMaxPlayrate);				/* 0=use input playrate, 1=use max. playrate */

int		_stdcall	TSPL_SET_SDRAM_BANK_INFO(
							 int nSub_Bank_Count,				/* 0~7 */
							 int nSub_Bank_Offset);				/* ~1024, power of 2 */
int		_stdcall	TSPL_SET_SDRAM_BANK_CONFIG(
							 int nSub_Bank_Count);				/* 0~7 */
int		_stdcall	TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(
							 int nSub_Bank_Offset);				/* ~1024, power of 2 */
int		_stdcall	TSPL_GET_CUR_BANK_GROUP(void);
int 	_stdcall	TSPL_RESET_SDCON(void);
int 	_stdcall	TSPL_SET_SDCON_MODE(int nMode);
int 	_stdcall	TSPL_GET_TSIO_DIRECTION(void);
int 	_stdcall	TSPL_SET_TSIO_DIRECTION(int nDirection);	/*  */
int		_stdcall	TVB380_SET_STOP_MODE(
							long stop_mode);					/* 0=no send, 1=send null packet when modulation is stopped */
int		_stdcall	TVB380_IS_ENABLED_TYPE(
							long modulator_type);				/* Check that the modulator type
																0=DVB-T, 1=8VSB, 2=QAM-A, 3=QAM-B, 4=QPSk
																5=TDMB, 6=16VSB, 7=DVB-H, 8=DVB-S2 is enabled */
int 	_stdcall	TVB380_SET_CONFIG(							/* Download the RBF file for each modulator type when that is changed */
							long modulator_type,
							long IF_frequency);					/* 36M, 44M (or 36.125M) */
int 	_stdcall	TVB380_SET_MODULATOR_FREQ(
							long modulator_type,				/* 0=DVB-T, 1=8VSB, 2=QAM-A, 3=QAM-B, 4=QPSk */
							unsigned long output_frequency,		/* Center Frequency */
							long symbol_rate_or_bandwidth);		/* bandwidth if DVB-T, else symbol rate */
int 	_stdcall	TVB380_SET_MODULATOR_SYMRATE(
							long modulator_type, 
							unsigned long output_frequency,		/* Center Frequency */
							long symbol_rate_or_bandwidth);		/* DVB-T/H : Bandwidth, else : Symbol Rate */
int 	_stdcall	TVB380_SET_MODULATOR_CODERATE(
							long modulator_type,
							long code_rate);
int		_stdcall	TVB380_SET_MODULATOR_TXMODE(
							long modulator_type,				
							long tx_mode);						/* 0=2K, 1=8K*/
int		_stdcall	TVB380_SET_MODULATOR_BANDWIDTH(
							long modulator_type,				
							long bandwidth,						/* 0=6M, 1=7M, 2=8M */
							unsigned long output_frequency);
int		_stdcall	TVB380_SET_MODULATOR_GUARDINTERVAL(
							long modulator_type,				
							long guard_interval);				/* 0=1/4, 1=1/8, 2=1/16, 3=1/32 */
int		_stdcall	TVB380_SET_MODULATOR_CONSTELLATION(
							long modulator_type,
							long constellation);
int		_stdcall	TVB380_SET_MODULATOR_INTERLEAVE(
							long modulator_type,				
							long interleaving);					/* 0=128-1,...,12=128-8 */
int		_stdcall	TVB380_SET_MODULATOR_IF_FREQ(
							long modulator_type,
							long IF_frequency);					/* 36M, 44M (or 36.125M) */
int		_stdcall	TVB380_SET_MODULATOR_SPECTRUM_INVERSION(
							long modulator_type,
							long spectral_inversion);			/* 0=normal, 1=inversed */
int		_stdcall	TVB380_SET_MODULATOR_PRBS_MODE(
							long modulator_type,				/* always 5(TDMB) */
							long mode);							/* 0=none, 1=2^7-1, 2=2^10-1, 3=2^15-1, 4=2^23-1 */
int		_stdcall	TVB380_SET_MODULATOR_PRBS_SCALE(
							long modulator_type,				/* always 5(TDMB) */
							double noise_power);				/* -70 ~ 20*/ //sskim20080620 - long -> double
int		_stdcall	TVB380_SET_MODULATOR_PRBS_INFO(
							long modulator_type,				
							long mode,							/* 0=none, 1=2^7-1, 2=2^10-1, 3=2^15-1, 4=2^23-1 */
							double noise_power);				/* -70 ~ 20*/ //sskim20080620 - long -> double
int		_stdcall	TSPL_GET_SYNC_POSITION(int mode,			/* 0=G.703 ETI, 1=G.704, 2=G.703 STI */
							int type,							/* 0=G.704 Sync, 1=ETI NA Sync */
							unsigned char *szBuf,				/* input source buffer, up to 1M bytes */
							int nlen,				
							int nlen_search,
							int nlen_step);
int		_stdcall	TSPL_DO_DEINTERLEAVING(
							unsigned char *szSrc,				/* G.704 and ETI NA sync searched source buffer, 6144 bytes size */ 
							unsigned char *szDest);				/* de-interleaved output buffer, 5760 bytes size */
int		_stdcall	TSPL_DO_RS_DECODING(
							unsigned char *szSrc,				/* de-interleaved source buffer, 5760 bytes size */ 
							unsigned char *szDest,				/* rs-decoded output buffer, 5376 or 5592 bytes size */
							int *format,						/* 1=G.704.5376 format, 0=G.704.5592 format */
							int *err_blk_cnt,					/* rs decoding errored block count */
							int *recovered_err_cnt,				/* rs decoding recovered error count */
							int bypass);						/* bypass(=1) rs decoding, only extract LI data */
int		_stdcall	TSPL_CONVERT_TO_NI(
							unsigned char  *szSrc,				/* LI(rs-decoded output) buffer, 5376 or 5592 bytes size */
							unsigned char *szDest,				/* G.703 NI output buffer, 6144 bytes size */
							int format);						/* 1=G.704.5376 format, 0=G.704.5592 format */
int		_stdcall	TVB380_SET_MODULATOR_INIT_CIF(void);
int		_stdcall	TVB380_SET_MODULATOR_RUN_CIF(
							unsigned char  *szSrc, 
							unsigned char *szDest);
int		_stdcall	TVB380_SET_MODULATOR_ATTEN_VALUE(
							long modulator_type, 
							double atten_value,				/* 0.0 ~ 31.5, step=0.5 */
							long UseTAT4710);				//2011/6/29 added UseTAT4710
int		_stdcall	TSPL_GET_ENCRYPTED_SN(int type,				/* 0=S/N, 1=L/N */
										  char* sn);
int		_stdcall	TSPL_CHECK_LN(char* ln);			
int		_stdcall	TSPL_CNT_MULTI_VSB_RFOUT(void);
int		_stdcall	TSPL_CNT_MULTI_QAM_RFOUT(void);
int		_stdcall	TVB380_open(int nInitModulatorType, 
								int nInitIF);					/* 36M, 44M (or 36.125M) */
int		_stdcall	TVB380_close(void);
int	_stdcall TSPL_GET_TSIO_STATUS(void);
int _stdcall TSPL_GET_DMA_REG_INFO(int addr);
int _stdcall TSPL_SET_DMA_REG_INFO(unsigned char nOffset, DWORD dwData);
int		_stdcall	TVB380_SET_MODULATOR_DVBH(
							long modulator_type,				/* always 7(DVB-H) */
							long  tx_mode,						/* 0=Off, 1=On */
							long  in_depth_interleave,			/* 0=Off, 1=On */ 
  							long  time_slice,					/* 0=Off, 1=On */
							long  mpe_fec,						/* 0=Off, 1=On */
							long  cell_id);						/* 0~65535 */
unsigned long _stdcall TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address);
int _stdcall		TSPL_WRITE_CONTROL_REG(int Is_PCI_Control, 
							unsigned long address, 
							unsigned long dwData);
int		_stdcall	TSPL_GET_LAST_ERROR(void);					/* Only for internal usage */
int		_stdcall	TVB380_SET_MODULATOR_PILOT(
							long modulator_type,				/* always 8(DVB-S2) */
							long  pilot_on_off);				/* 1=Off, 0=On */
int		_stdcall	TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(
							long modulator_type,				/* always 8(DVB-S2) */
							long roll_off_factor);				/* 0=0.20, 1=0.25, 2=0.35, 3=none */
int 	_stdcall	TSPL_GET_BOARD_CONFIG_STATUS(void);			/* RF/IF AMP used=1(-6dBm amp),=2(0dBm amp), no used=0 */
int		_stdcall	TVB380_SET_BOARD_CONFIG_STATUS(long modulator_type, 
												   long status);	/* RF/IF AMP used=1, bypassed=10 */
int		_stdcall	TSPL_SET_BOARD_LED_STATUS(int status_LED,	/* status_LED off=0, on=1 */
											  int fault_LED);	/* fault_LED off=0, on=1 */
int		_stdcall	TSPL_REG_EVENT(void* pvoid);
int _stdcall TSPL_REG_COMPLETE_EVENT(void* pvoid);
int _stdcall TSPL_SET_COMPLETE_EVENT(void);
int		_stdcall	TSPL_SET_FIFO_CONTROL(int nDMADirection, int nDMASize);
int _stdcall TSPL_GET_FIFO_CONTROL(int nDMADirection, int nDMASize);
double		_stdcall	TVB380_GET_MODULATOR_RF_POWER_LEVEL(
							long modulator_type, 
							long info_type);					/* 0=Max. Level, 1=Min. Level, 2=Current Level, 3=Check TAT4710 */
int		_stdcall	TVB380_SET_MODULATOR_BERT_MEASURE(
													  long modulator_type,	/* DVB-T, 8VSB, QAM-A/B, QPSK, ISDB-T */
													  long packet_type);	/* 0=normal(NO BERT), 1=all 0's, 2=all 1's, 3=PRBS 2^15-1, 4=PRBS 2^23-1 */
int		_stdcall	TVB380_SET_MODULATOR_DTMB(
							long modulator_type,				/* always 11, DTMB only */
							long constellation,					/* 0=4QAM-NR, 1=4QAM, 2=16QAM, 3=32QAM, 4=64QAM */
							long code_rate,						/* 0=0.4, 1=0.6, 2=0.8 */
							long interleaver,					/* 0=52/240, 1=52/720 */
							long frame_header,					/* 0=PN420, 1=PN595, 2=PN945 */
							long carrier_number,				/* 0=1(ADTB-T), 1=3780(DTMB) */
							long frame_header_pn,				/* 0=Fixed P/N, 1=Rotated P/N */
							long pilot_insertion);				/* 0=Disabled, 1=Enabled */
int		_stdcall	TVB380_SET_MODULATOR_SDRAM_CLOCK(			
							long modulator_type,				/* */
							long sdram_clock);					/* 0(USE SYSTEM CLOCK, It must be 0 except DTMB), 1(USE SYMBOL CLOCK, DTMB must be 1) */
int _stdcall		TSPL_GET_AD9775(long reg);
int _stdcall		TSPL_SET_AD9775(long reg, long data);
int		_stdcall TSPL_GET_AD9787(long reg);
int		_stdcall TSPL_SET_AD9787(long reg, long data);
int _stdcall		TVB380_SET_MODULATOR_AGC(long modulator_type, 
											 long agc_on_off,		/* 0=off, 1=on */
											long UseTAT4710);		//2011/6/29 added UseTAT4710
int		_stdcall	TSPL_GET_BOARD_REV(void);
int		_stdcall	TSPL_GET_FPGA_INFO(int info);/* 0=ID, 1=version, 2=IQ play support, 3=IQ capture support */
int		_stdcall	TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction);
int		_stdcall	TSPL_RESET_IP_CORE(long modulator_type, long reset_control);
int		_stdcall	TSPL_SET_MH_MODE(long modulator_type, long mh_mode);
int		_stdcall	TSPL_SET_MH_PID(long modulator_type, long mh_pid);
int		_stdcall	TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock);
int		_stdcall	TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate);
#ifdef WIN32
int		_stdcall	TSPL_SET_AD9852_MAX_CLOCK(long value);
#endif
int		_stdcall	TVB380_SET_MODULATOR_OUTPUT(
							long modulator_type,				
							long output);				/* 0=enable, 1=disable */
int _stdcall TSPL_SEL_DDSCLOCK_INC_594(long play_freq_in_herz, long multi_module_tag);
int _stdcall TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource);
int _stdcall TVB380_SET_MODULATOR_SINGLE_TONE(long modulator_type, unsigned long freq, long singleTone);
int		_stdcall	TVBxxx_DETECT_BD(int _multi_bd);
int		_stdcall	TVBxxx_INIT_BD(int _my_num, void *_my_cxt);
int		_stdcall	TVBxxx_GET_CNXT_ALL_BD(void *basic_inform);
int		_stdcall	TVBxxx_GET_CNXT_MINE(void *_cxt);
int		_stdcall	TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *basic_inform);
int		_stdcall	TVB380_GET_wr_BUF_STS_MultiBd_n(int _mod_n);	//	0, 1, 2, 3
int		_stdcall	TVB380_GET_wr_cap_BUF_STS_MultiBd_n(void);
int		_stdcall	TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n);	//	0, 1, 2, 3
int		_stdcall	SelMultiModAsiOut_n(int _ts_n); //	0, 1, 2, 3
int 	_stdcall	SelMultiMod310Out_n(int _ts_n); //	0, 1, 2, 3
int 	_stdcall	SelMultiModTsOutput_n(int _ctl);
int 	_stdcall	SelMultiModOperationMode_n(int _ctl);
int		_stdcall	TSPL_CNT_MULTI_DVBT_RFOUT(void);
int		_stdcall	TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710);
int		_stdcall	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *_p_rf_level_min, 
                                       double *_p_rf_level_max, long UseTAT4710);
int _stdcall TVB59x_SET_PCR_STAMP_CNTL(int _val);
int _stdcall TVB59x_SET_Output_TS_Type(int _val);
int _stdcall TVB59x_SET_Reset_Control_REG(int _val);
int _stdcall TSPL_TVB59x_SET_TsPacket_CNT_Mode(int _val);
int _stdcall TSPL_TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock);
int _stdcall TVB59x_Modulator_Status_Control(long modulator, long index, long val);
int _stdcall TVB59x_Get_Modulator_Status(int index);

#endif
#ifdef __cplusplus
}
#endif


#endif
