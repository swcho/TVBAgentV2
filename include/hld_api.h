#ifndef __INCLUDE_HLD_API__
#define __INCLUDE_HLD_API__
#ifdef WIN32
#include	"../include/hld_structure.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
#else
#define	_stdcall
int _stdcall DllMain(int fdwReason, int nSlot1, int nSlot1_Modulator, int nSlot1_IF);
#endif


int _stdcall	TSPH_START_PLAY(int nSlot, char *szNewPlayFile, long lPlayRate, double dwStartOffset, long nUseMaxPlayrate, long lRepeatMode);
int	_stdcall	TSPH_START_MONITOR(int nSlot, int nPort);
int	_stdcall	TSPH_START_RECORD(int nSlot, char* szNewRecordFile, int nPort);
int	_stdcall	TSPH_START_DELAY(int nSlot, int nPort);
int	_stdcall	TSPH_SET_REPEAT_PLAY_MODE(int nSlot, int nRepeatMode);
int _stdcall	TSPH_GET_CURRENT_THREAD_STATE(int nSlot);
double _stdcall TSPH_GET_CURRENT_RECORD_POINT(int nSlot);
int	_stdcall	TSPH_SET_FILENAME_NEXT_PLAY(int nSlot, char *szNextPlayFile);
int _stdcall	TSPH_SET_SDRAM_BANK_INFO(int nSlot, int nBankCount, int nBankOffset);
int _stdcall	TSPH_GET_PLAY_BUFFER_STATUS(int nSlot);
int	_stdcall	TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType);
int _stdcall	TSPH_SET_MODULATOR_TYPE(int nSlot, long type);
int _stdcall	TSPL_GET_BOARD_CONFIG_STATUS_EX(int nSlot);
int	_stdcall	TSPL_SET_BOARD_LED_STATUS_EX(int nSlot, int status_LED, int fault_LED);
int _stdcall	TSPL_GET_BOARD_ID_EX(int nSlot);
int _stdcall	TSPL_RESET_SDCON_EX(int nSlot);
int _stdcall	TSPL_GET_ENCRYPTED_SN_EX(int nSlot, int type, char* sn);
int _stdcall	TSPL_CHECK_LN_EX(int nSlot, char* ln);
int	_stdcall	TSPL_SET_PLAY_RATE_EX(int nSlot, long play_freq_in_herz, long nUseMaxPlayrate);
int _stdcall    TSPL_SET_TSIO_DIRECTION_EX(int nSlot, int nDireciton);
int _stdcall TSPL_GET_TSIO_STATUS_EX(int nSlot);
int _stdcall TSPL_GET_DMA_REG_INFO_EX(int nSlot, int addr);
int _stdcall TSPL_SET_DMA_REG_INFO_EX(int nSlot, int addr, int data);
int _stdcall TSPL_SET_DEMUX_CONTROL_TEST_EX(int nSlot, int data);
int	_stdcall	TVB380_SET_STOP_MODE_EX(int nSlot, long stop_mode);
int	_stdcall	TVB380_IS_ENABLED_TYPE_EX(int nSlot, long modulator_type);
int _stdcall	TVB380_SET_CONFIG_EX(int nSlot, long modulator_type, long IF_Frequency);
int _stdcall	TVB380_SET_MODULATOR_FREQ_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth);
int _stdcall	TVB380_SET_MODULATOR_SYMRATE_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth);
int _stdcall	TVB380_SET_MODULATOR_CODERATE_EX(int nSlot, long modulator_type, long code_rate);
int	_stdcall	TVB380_SET_MODULATOR_TXMODE_EX(int nSlot, long modulator_type, long tx_mode);
int	_stdcall	TVB380_SET_MODULATOR_BANDWIDTH_EX(int nSlot, long modulator_type, long bandwidth, double output_frequency);
int	_stdcall	TVB380_SET_MODULATOR_GUARDINTERVAL_EX(int nSlot, long modulator_type, long guard_interval);
int	_stdcall	TVB380_SET_MODULATOR_CONSTELLATION_EX(int nSlot, long modulator_type,long constellation);
int	_stdcall	TVB380_SET_MODULATOR_INTERLEAVE_EX(int nSlot, long modulator_type, long interleaving);
int	_stdcall	TVB380_SET_MODULATOR_IF_FREQ_EX(int nSlot, long modulator_type, long IF_frequency);
int	_stdcall	TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(int nSlot, long modulator_type, long spectral_inversion);
int _stdcall	TVB380_SET_MODULATOR_ATTEN_VALUE_EX(int nSlot, long modulator_type, double atten_value, long UseTAT4710);
int _stdcall	TVB380_SET_MODULATOR_PRBS_MODE_EX(int nSlot, long modulator_type, long mode);
int _stdcall	TVB380_SET_MODULATOR_PRBS_SCALE_EX(int nSlot, long modulator_type, double noise_power);
int _stdcall	TVB380_SET_MODULATOR_PRBS_INFO_EX(int nSlot, long modulator_type, long mode, double noise_power);
int	_stdcall	TSPL_GET_DMA_STATUS_EX(int nSlot);
int _stdcall	TVB380_SET_MODULATOR_DVBH_EX(int nSlot, long modulator_type, long  tx_mode, long  in_depth_interleave, long  time_slice, long  mpe_fec, long  cell_id);
unsigned long _stdcall TSPL_READ_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address);
int _stdcall	TSPL_WRITE_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address, unsigned long dwData);
unsigned long 	_stdcall	TSPL_READ_INPUT_STATUS_EX(int nSlot);
double _stdcall TSPL_READ_INPUT_TSCOUNT_EX(int nSlot);
int _stdcall	TSPL_GET_LAST_ERROR_EX(int nSlot);
int _stdcall	TVB380_SET_MODULATOR_PILOT_EX(int nSlot, long modulator_type, long  pilot_on_off);
int	_stdcall	TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(int nSLot, long modulator_type, long roll_off_factor);
int _stdcall	TSPH_SET_CURRENT_OFFSET(int nSlot, 
										int nOffsetType,	//0=start offset, 1==current offset, 2==end offset
										double dwOffset);
int _stdcall	TSPH_START_IP_STREAMING(int nSlot, 
									char *szFilePath,		//File path to be played or recorded
									long nPlayrate,			//In bps
									double dwStartOffset,	//Start offest in bytes
									long nOption,			//Restamping
									long nRepeatMode,		//Play mode
									long nExtendedMode,		//Operation mode thru. IP
									char *szExtendedInfo);	//Source or Target thru. IP
int _stdcall	TSPH_READ_IP_STREAMING_INPUT_STATUS(int nSlot, long nStatus);//nStatus==0(Input Bitrate), 2(Demux Bitrate)
long _stdcall	TSPH_GET_PROGRAM(int nSlot, long nIndex);
long _stdcall	TSPH_SET_PROGRAM(int nSlot, long nIndex);
int _stdcall	TSPH_SHOW_VIDEO_WINDOW(int nSlot, int nShowWindow);
int _stdcall	TSPH_MOVE_VIDEO_WINDOW(int nSlot, int nX, int nY, int nW, int nH);
int _stdcall	TSPH_IS_VIDEO_WINDOW_VISIBLE(int nSlot);
int	_stdcall	TVB380_SET_BOARD_CONFIG_STATUS_EX(int nSlot, long modulator_type, long status);
int _stdcall TSPL_REG_COMPLETE_EVENT_EX(int nSlot, void* pvoid);
int _stdcall TSPL_SET_COMPLETE_EVENT_EX(int nSlot);
long _stdcall   TSPH_EXIT_PROCESS(int nSlot);
int _stdcall	TSPL_SET_FIFO_CONTROL_EX(int nSlot, long dma_direction, long dma_size);
double _stdcall	TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(int nSlot, long modulator_type, long info_type);
int	_stdcall	TSPH_GET_REMUX_INFO(int nSlot, char *szFile, int layer_index);
int	_stdcall	TSPH_SET_REMUX_INFO(int nSlot,
									long btype, long mode, long guard_interval, long partial_reception, long bitrate,
									long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
									long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
									long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
int	_stdcall	TSPH_GET_REMUX_DATARATE(int nSlot, long layer_index);
int	_stdcall	TSPH_SET_TMCC_REMUXER(int nSlot, long use_tmcc_remuxer);
int	_stdcall	TSPH_SET_LAYER_INFO(int nSlot,
									int other_pid_map_to_layer,
									int multi_pid_map,
									int total_pid_count, char* total_pid_info,
									int a_pid_count, char* a_pid_info,
									int b_pid_count, char* b_pid_info,
									int c_pid_count, char* c_pid_info);
int	_stdcall	TVB380_SET_MODULATOR_BERT_MEASURE_EX(int nSlot, long modulator_type, long packet_type, long bert_pid);
double _stdcall	TVB380_GET_MODULATOR_BERT_RESULT_EX(int nSlot, long modulator_type);
int	_stdcall	TVB380_SET_MODULATOR_DTMB_EX(int nSlot, long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion);
int	_stdcall	TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(int nSlot, long modulator_type, long sdram_clock);
int	_stdcall	TSPH_SET_ERROR_INJECTION(int nSlot, long error_lost, long error_lost_packet,
										 long error_bits, long error_bits_packet, long error_bits_count,
										 long error_bytes, long error_bytes_packet, long error_bytes_count);
int _stdcall	TSPL_GET_AUTHORIZATION_EX(int nSlot);
int	_stdcall	TSPH_RUN_MFS_PARSER(int nSlot, char *szFile, char *szResult);
int _stdcall	TSPL_GET_AD9775_EX(int nSlot, long reg);
int _stdcall	TSPL_SET_AD9775_EX(int nSlot, long reg, long data);
int _stdcall	TVB380_SET_MODULATOR_AGC_EX(int nSlot, long modulator_type, long agc_on_off, long UseTAT4710);
int	_stdcall	TSPH_SET_LOOP_ADAPTATION(int nSlot, 
										 long pcr_restamping,		/* 0=0ff, 1=on */
										 long continuity_conunter,	/* 0=0ff, 1=on */
										 long tdt_tot,				/* 0=off, 1=2nd loop, 2=current date/time, 3=user date/time */
										 char* user_date,			/* yyyy-mm-dd (tdt_tot==3), ex) 2009-11-28 */
										 char* user_time);			/* hh-mm-ss (tdt_tot==3), ex) 01:18:50 */
int	_stdcall	TSPH_GET_LOOP_COUNT(int nSlot);
int _stdcall	TSPL_GET_BOARD_REV_EX(int nSlot);
int	_stdcall	TSPH_GET_MHE_PACKET_INFO(int nSlot, char *szFile, int iType);
int	_stdcall	TSPH_SET_RX_IP_STREAMING_INFO(int nSlot, char* src_ip, char* multicast_ip, long udp_port, char* local_ip, long fec_udp_off, long fec_inact);
#ifdef WIN32
int	_stdcall	TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, T2MI_Parsing_Info *szResult);
#else
int	_stdcall	TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, char *szResult);
#endif
int	_stdcall	TSPH_SET_T2MI_PARAMS(int nSlot, int BW, int FFT_SIZE, int GUARD_INTERVAL, int L1_MOD, int PILOT_PATTERN, int BW_EXT, double FREQUENCY, int NETWORK_ID, int T2_SYSTEM_ID, int CELL_ID, int S1, int PLP_MOD, int PLP_COD, int PLP_FEC_TYPE, int HEM, int NUM_T2_FRAMES, int NUM_DATA_SYMBOLS, int PLP_NUM_BLOCKS, int PID, int PLP_ROTATION, int PLP_COUNT, int PLP_ID, int PLP_BITRATE, int PLP_TS_BITRATE, char *PLP_TS, int Time_IL_Length);
int	_stdcall	TSPH_RUN_ATSC_MH_PARSER(int nSlot, char *szFile, char *szResult);
int _stdcall	TSPL_GET_FPGA_INFO_EX(int nSlot, int info);
int	_stdcall	TSPH_SET_C2MI_PARAMS( 
									int nSlot, 
									int C2_BW,
									int C2_L1,
									int C2_Guard,
									int C2_Network,
									int C2_System,
									int C2_StartFreq,
									int C2_NumNoth,
									int C2_RevTone,
									int C2_NotchStart,
									int C2_NotchWidth,
									int C2_Dslice_type,
									int C2_Dslice_FecHeder,
									int C2_Dslice_BBHeder,
									int C2_Dslice_TonePos,
									int C2_Dslice_OffRight,
									int C2_Dslice_OffLeft,
									int C2_Plp_Mod,
									int C2_Plp_Code,
									int C2_Plp_Fec,
									int C2_Plp_Count,
									int C2_Plp_ID,
									int C2_Plp_Blk,
									int C2_Plp_TS_Bitrate,
									char *C2_Plp_TS,
									int C2_createFile);
int _stdcall	TSPH_SET_IQ_MODE(int nSlot, int mode, int memory_based, int memory_size, int capture_size);
int _stdcall	TSPH_TRY_ALLOC_IQ_MEMORY(int nSlot, int mem_size);
int _stdcall	TSPL_SET_MAX_PLAYRATE_EX(int nSlot, long modulator_type, long use_max_playrate);
int	_stdcall	TSPH_SET_COMBINER_INFO(int nSlot, int ts_count, char *ts_path, long modulation, long code_rate, long slot_count);
int	_stdcall	TSPH_GET_TS_ID(int nSlot, char* szFile);
#ifdef WIN32
//2011/5/4 AD9852 MAX CLK
int _stdcall TSPL_SET_AD9852_MAX_CLOCK_EX(long value);
#endif
int	_stdcall	TSPH_CLEAR_REMUX_INFO(int nSlot);
int _stdcall	TSPH_SET_ISDBS_BASE_TS(int nSlot, char* ts_path);
int _stdcall	TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(int nSlot, char* ts_path);
int	_stdcall	TSPH_RUN_C2MI_PARSER(int nSlot, char *szFile, char *szResult); //currently not used, for future.
int _stdcall	TSPH_GET_T2MI_PARAMS(int nSlot, int *num_data_symbol, int *plp_num_block);
int _stdcall	TSPH_IS_COMBINED_TS(int nSlot, char *ts_path);
int _stdcall	TSPH_IP_RECV_STATUS(int nSlot, unsigned int *_buf_bytes_avable, unsigned int *_cnt_lost_pkt);
int _stdcall	TSPH_IS_LOOPTHRU_ISDBT13_188(int nSlot);
int _stdcall	TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(int nSlot);
int _stdcall	TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(int nSlot);
int _stdcall	TSPH_GET_NIT_SATELLITE_INFO(int nSlot, 
											int *descriptor_flag,
											int *freq,
											int *rolloff,
											int *modulation_system,
											int *modulation,
											int *symbolrate,
											int *coderate);
int _stdcall	TSPH_GET_NIT_CABLE_INFO(int nSlot, int *descriptor_flag, int *freq, int *modulation, int *symbolrate, int *coderate);
int _stdcall	TVB380_SET_MODULATOR_SINGLE_TONE_EX(int nSlot, long modulator_type, unsigned long freq, long singleTone);
int _stdcall	TSPH__STREAM_NUMBER(int vSlot);
int _stdcall	TSPH__SEL_TS_of_ASI310OUT(int nSlot, int _ts_n);
int _stdcall	TSPH_GetRealBdCnt_N(void);
int _stdcall	TSPH_GetRealAndVirBdMap(int _r_bd_id, int *_map);
int _stdcall	TSPH_GetBdId_N(int _Nth);	//	___ID_BDs___
char *_stdcall	TSPH_GetBdName_N(int _Nth);	//	the string of location name.
int _stdcall	TSPH_ConfTvbSytem(void);	//	return cnt of installed real bd.
int _stdcall	TSPH_InitOneRealBd(int _bd_id);
int _stdcall	TSPH_InitAllRealBd(void);
int _stdcall	TSPH_InitVirBd(int _r_bd_id, int _v_cnt);
int _stdcall	TSPH_ActivateOneBd(int _bd_id, int _init_modulator, int _init_if_freq);
int _stdcall	TSPL_CNT_MULTI_VSB_RFOUT_EX(int nSlot);
int _stdcall	TSPL_CNT_MULTI_QAM_RFOUT_EX(int nSlot);
int _stdcall	TSPH_GET_NIT_TERRESTRIAL_INFO(int nSlot, int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod);
int _stdcall	TSPL_CNT_MULTI_DVBT_RFOUT_EX(int nSlot);
int _stdcall	TSPH_RUN_TS_PARSER2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo);
int _stdcall	TSPH_FREE_TS_PARSER_MEMORY(int nSlot); //currently not used, for future.
int _stdcall	TSPH_GET_PMT_INFO(int nSlot, int *pNumPgmInfo, struct _TSPH_TS_PGM_INFO **ppPgmInfo); //currently not used, for future.
int _stdcall	TSPH_GET_PID_INFO(int nSlot, int *pNumPidInfo, struct _TSPH_TS_PID_INFO **ppPidInfo); //currently not used, for future.
int _stdcall	TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser(int nSlot, struct _TSPH_TS_INFO **ppTsInfo);
int _stdcall	TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX(int nSlot, long modulator_type, double rf_level_value, long *AmpFlag, long UseTAT4710);
int _stdcall	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX(int nSlot, long modulator_type, double *rf_level_min, double *rf_level_max, long UseTAT4710);
int _stdcall	TVB59x_SET_PCR_STAMP_CNTL_EX(int nSlot, int _val);
int _stdcall TSPL_GET_AD9787_EX(int nSlot, long reg);
int _stdcall TSPL_SET_AD9787_EX(int nSlot, long reg, long data);
int _stdcall TSPH_SET_T2MI_STREAM_GENERATION(int nSlot, int Output_T2mi, char* ts_path);
int _stdcall TVB59x_SET_Output_TS_Type_EX(int nSlot, int _val);
int _stdcall TVB59x_SET_Reset_Control_REG_EX(int nSlot, int _val);
int _stdcall    TSPL_SET_SDCON_MODE_EX(int nSlot, int nMode);
int _stdcall TSPL_TVB59x_SET_TsPacket_CNT_Mode_EX(int nSlot, int _val);
int _stdcall TSPL_TVB59x_Get_Asi_Input_rate_EX(int nSlot, int *delta_packet, int *delta_clock);
int _stdcall TVB59x_Modulator_Status_Control_EX(int nSlot, int modulator, int index, int val);
int _stdcall TVB59x_Get_Modulator_Status_EX(int nSlot, int index);
int _stdcall	TSPH_CloseOneRealBd(int _bd_id);

#ifdef __cplusplus
}
#endif

#endif	//__INCLUDE_HLD_API__	
