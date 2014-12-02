//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_WDM_DRV_WRAPPER_H_
#define	__TLV_WDM_DRV_WRAPPER_H_

#ifdef WIN32
#define DLL_EXPORT	extern "C" _declspec(dllexport)
#else
#define DLL_EXPORT
#define _stdcall 
#endif

#ifdef __cplusplus
//	extern "C" {
#endif
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
unsigned long	WDM_Read_TSP(unsigned long dwMemAddr);
int	WDM_WR2SDRAM(unsigned long dwCommand);
/////////////////////////////////////////////////////////////////
int 	TSPL_SET_CONFIG_DOWNLOAD(long addr, unsigned long data);
/////////////////////////////////////////////////////////////////
int	WDM_WR2SDRAM_ADDR(unsigned long dwAddress, unsigned long dwCommand);
/////////////////////////////////////////////////////////////////
unsigned long RemapAddress(unsigned long address);
/////////////////////////////////////////////////////////////////
int SetSDRAMBankConfig(int nBankConfig);
int _stdcall ___TSPL_GET_AD9775(long reg);
int _stdcall ___TSPL_SET_AD9775(long reg, long data);
/////////////////////////////////////////////////////////////////
#ifdef WIN32
void	Close_System(void);
#endif
int ___read_modulator_option();
int ___set_sync_modulator_type(int modulator_type);

int ___get_TSPL_nBoardTypeID(void);
void ___set_board_location(int val);

#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
int	_stdcall TVB380_close__();
int	_stdcall TVB380_open__(int nInitModulatorType, int nInitIF);
/////////////////////////////////////////////////////////////////
#elif defined(TSPLLD0431_EXPORTS) || defined(TSE110V1)
int	_stdcall TSE110_close__();
int	_stdcall TSE110_open__();
#endif
/////////////////////////////////////////////////////////////////
int _stdcall TSPL_GET_BOARD_REV__(void);
int _stdcall TSPL_GET_BOARD_ID__(void);
int _stdcall TSPL_GET_AUTHORIZATION__(void);
int _stdcall TSPL_GET_FPGA_INFO__(int info);
int _stdcall TSPL_GET_ENCRYPTED_SN__(int type, char* sn);
int _stdcall TSPL_WRITE_CONTROL_REG__(int Is_PCI_Control, unsigned long address, unsigned long dwData);
unsigned long _stdcall TSPL_READ_CONTROL_REG__(int Is_PCI_Control, unsigned long address);
int _stdcall TSPL_GET_LAST_ERROR__(void);
int _stdcall TSPL_GET_BOARD_CONFIG_STATUS__(void);
#if defined(WIN32) || defined(TVB595V1)
int _stdcall TSPL_SET_BOARD_LED_STATUS__(int status_LED, int fault_LED);
int _stdcall TSPL_REG_EVENT__(void* pvoid);
int _stdcall TSPL_SET_EVENT__(void);
int _stdcall TSPL_REG_COMPLETE_EVENT__(void* pvoid);
int _stdcall TSPL_SET_COMPLETE_EVENT__(void);
#endif
#if TVB597A_STANDALONE
int _stdcall TVB597F_SET_FLASH__(char *szFile);
#else
int _stdcall TVB597F_SET_FLASH__(char *szFile);
#endif

/////////////////////////////////////////////////////////////////
#ifdef WIN32
int _stdcall TVB380_IS_ENABLED_TYPE__(long modulator_type);
#else
int TVB380_IS_ENABLED_TYPE__(long modulator_type);
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int _stdcall TSPL_GET_DMA_STATUS__(void);
int _stdcall TSPL_PUT_DATA__(DWORD dwDMASize,DWORD *dest);
void* _stdcall TSPL_GET_DATA__(long dwDMASize);
int _stdcall TSPL_SET_FIFO_CONTROL__(int nDMADirection, int nDMASize);
int _stdcall TSPL_GET_FIFO_CONTROL__(int nDMADirection, int nDMASize);
void* _stdcall TSPL_READ_BLOCK__(long dwDMASize);
void	_stdcall TSPL_WRITE_BLOCK_TEST__(unsigned long *pdwSrcBuff, unsigned long dwBuffSize, unsigned long *dest);
#ifdef WIN32
int _stdcall TSPL_WRITE_BLOCK__(DWORD dwBuffSize,DWORD *dest);
#else
int _stdcall TSPL_WRITE_BLOCK__(DWORD *pdwSrcBuff, ULONG dwBuffSize, DWORD *dest);
#endif
void* _stdcall TSPL_GET_DMA_ADDR__(void);
int _stdcall TSPL_GET_DMA_REG_INFO__(int addr);
int _stdcall TSPL_SET_DMA_REG_INFO__(unsigned char nOffset, DWORD dwData);
int TSPL_SET_DEMUX_CONTROL_TEST__(int nState);
int	_stdcall TSPL_SET_SDRAM_BANK_INFO__(int nBankNumber, int nBankOffset);
int	_stdcall TSPL_SET_SDRAM_BANK_CONFIG__(int nBankConfig);
int	_stdcall TSPL_SET_SDRAM_BANK_OFFSET_CONFIG__(int nBankConfig);
int	_stdcall TSPL_SEL_DDSCLOCK_INC__(long play_freq_in_herz);
int	_stdcall TSPL_SET_PLAY_RATE__(long play_freq_in_herz, long nOutputClockSource);
int 	_stdcall    TSPL_SET_TSIO_DIRECTION__(int nDirection);
int	_stdcall TSPL_GET_TSIO_STATUS__(int option);
int	_stdcall TSPL_GET_TSIO_DIRECTION__(void);
int	_stdcall TSPL_GET_CUR_BANK_GROUP__(void);
int 	_stdcall    TSPL_SET_BANK_COUNTER__(int nBank);
int 	_stdcall    TSPL_SET_SDCON_MODE__(int nMode);
int 	_stdcall TSPL_RESET_SDCON__(void);
int	_stdcall TSE110_GET_SYNC_STATUS__(int mode);
int	_stdcall TSE110_GET_SIGNAL_STATUS__(int port);
int	_stdcall TSE110_GET_SYNC_FORMAT__(void);
int	_stdcall TSE110_SET_CONFIG__(int port_a_mode,int port_b_mode,int tx_clock,int input_port,int output_a_mode,int output_b_mode);
int	_stdcall TSPL_GET_SYNC_POSITION__(int mode, int type, unsigned char *szBuf, int nlen, int nlen_srch, int nlen_step);
int	_stdcall TSPL_DO_DEINTERLEAVING__(unsigned char *szSrc, unsigned char *szDest);
int	_stdcall TSPL_DO_RS_DECODING__(unsigned char *szSrc, unsigned char *szDest, int *format,int *err_blk_cnt,int *recovered_err_cnt,int bypass);
int	_stdcall TSPL_CONVERT_TO_NI__(unsigned char  *szSrc, unsigned char *szDest, int format);
int	_stdcall TVB380_SET_MODULATOR_INIT_CIF__(void);
int	_stdcall TVB380_SET_MODULATOR_RUN_CIF__(unsigned char  *szSrc, unsigned char *szDest);
int _stdcall TSPL_SET_MAX_PLAYRATE__(long modulator_type, long use_max_playrate);
int _stdcall TVB380_SET_CONFIG__(long modulator_type, long IF_Frequency);
int _stdcall TSPL_GET_AD9775__(long reg);
int _stdcall TSPL_SET_AD9775__(long reg, long data);

int _stdcall TSPL_GET_AD9787__(long reg);
int _stdcall TSPL_SET_AD9787__(long reg, long data);

int _stdcall TVB380_SET_MODULATOR_BANDWIDTH__(long modulator_type,long bandwidth, unsigned long output_frequency);
int _stdcall TVB380_SET_MODULATOR_GUARDINTERVAL__(long modulator_type,long guard_interval);
//2011/11/01 added PAUSE
int _stdcall TVB380_SET_MODULATOR_OUTPUT__(long modulator_type,long output);

int _stdcall TVB380_SET_MODULATOR_CONSTELLATION__(long modulator_type,long constellation);
int _stdcall TVB380_SET_MODULATOR_FREQ__(long modulator_type,unsigned long output_frequency, long symbol_rate_or_bandwidth);
int _stdcall TVB380_SET_MODULATOR_SYMRATE__(long modulator_type, unsigned long output_frequency, long symbol_rate_or_bandwidth);
int _stdcall TVB380_SET_MODULATOR_CODERATE__(long modulator_type, long code_rate);
int _stdcall TVB380_SET_MODULATOR_TXMODE__(long modulator_type, long tx_mode);
int _stdcall TVB380_SET_MODULATOR_INTERLEAVE__(long modulator_type, long interleaving);
int _stdcall TVB380_SET_MODULATOR_IF_FREQ__(long modulator_type, long IF_frequency);
int _stdcall TVB380_SET_MODULATOR_SPECTRUM_INVERSION__(long modulator_type, long spectral_inversion);
int _stdcall TVB380_SET_STOP_MODE__(long stop_mode);
int _stdcall TVB390_PLL2_DOWNLOAD__(long value);
int _stdcall TVB380_SET_MODULATOR_PRBS_MODE__(long modulator_type, long mode);
int _stdcall TVB380_SET_MODULATOR_PRBS_SCALE__(long modulator_type, double scale);
int _stdcall TVB380_SET_MODULATOR_PRBS_INFO__(long modulator_type, long mode, double scale);
int _stdcall TVB380_SET_MODULATOR_AGC__(long modulator_type, long agc_on_off, long UseTAT4710);		//2011/6/29 added UseTAT4710
int _stdcall TVB380_SET_MODULATOR_ATTEN_VALUE__(long modulator_type, double atten_value, long UseTAT4710);		//2011/6/29 added UseTAT4710
int _stdcall TVB380_SET_MODULATOR_DVBH__(long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id);
int _stdcall TVB380_SET_MODULATOR_PILOT__(long modulator_type, long  pilot_on_off);
int _stdcall TVB380_SET_MODULATOR_ROLL_OFF_FACTOR__(long modulator_type, long  roll_off_factor);
int _stdcall TVB380_SET_BOARD_CONFIG_STATUS__(long modulator_type, long status);
double _stdcall TVB380_GET_MODULATOR_RF_POWER_LEVEL__(long modulator_type, long info_type);
int _stdcall TVB380_SET_MODULATOR_BERT_MEASURE__(long modulator_type, long packet_type);
int _stdcall TVB380_SET_MODULATOR_DTMB__(long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion);
int _stdcall TVB380_SET_MODULATOR_SDRAM_CLOCK__(long modulator_type, long sdram_clock);
int _stdcall TSPL_SET_DMA_DIRECTION__(long modulator_type, long dma_direction);
int _stdcall TSPL_RESET_IP_CORE__(long modulator_type, long reset_control);
int _stdcall TSPL_SET_MH_MODE__(long modulator_type, long mh_mode);
int _stdcall TSPL_SET_MH_PID__(long modulator_type, long mh_pid);
int _stdcall TSPL_SET_SYMBOL_CLOCK__(long modulator_type, long symbol_clock);

/////////////////////////////////////////////////////////////////
int _stdcall TVBxxx_DETECT_BD__(int _multi_bd);
int	_stdcall TVBxxx_INIT_BD__(int _my_num, void *_my_cxt);
int	_stdcall TVBxxx_GET_CNXT_ALL_BD__(void *basic_inform);
int	_stdcall TVBxxx_GET_CNXT_MINE__(void *_cxt);
int	_stdcall TVBxxx_DUP_BD__(int from_slot_num, int to_slot_num, void *basic_inform);

#ifdef WIN32
int _stdcall TSPL_CHECK_LN__(char* ln);
//2011/5/4 AD9852 MAX CLK
int _stdcall TSPL_SET_AD9852_MAX_CLOCK__(long value);
#else
int TSPL_CHECK_LN__(char* ln);
#endif
int TSPL_CNT_MULTI_VSB_RFOUT__(void);
int TSPL_CNT_MULTI_QAM_RFOUT__(void);
//2012/6/28 multi dvb-t
int TSPL_CNT_MULTI_DVBT_RFOUT__(void);

int _stdcall TSPL_SEL_DDSCLOCK_INC_594__(long play_freq_in_herz, long multi_module_tag);
int _stdcall TSPL_SET_PLAY_RATE_594__(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource);

#ifdef WIN32
int TSPL_PUT_CHAR__(long dwCommand);
int TSPL_GET_CHAR__(void);
#endif
int	_stdcall TVB380_GET_wr_BUF_STS_MultiBd_n__(int _mod_n);
int	_stdcall TVB380_GET_wr_cap_BUF_STS_MultiBd_n__(void);
int	_stdcall TSPL_SEL_TS_TAG_VirtualBd_n__(int _mod_n);
int 	_stdcall SelMultiModAsiOut_n__(int _ts_n);
int 	_stdcall SelMultiMod310Out_n__(int _ts_n);
int 	_stdcall SelMultiModTsOutput_n__(int _ctl);
int 	_stdcall SelMultiModOperationMode_n__(int _ctl);
//2012/4/12
int _stdcall TVB380_SET_MODULATOR_SINGLE_TONE__(long modulator_type, unsigned long freq, long singleTone);

//2012/8/27
int _stdcall TVB59x_SET_MODULATOR_RF_LEVEL_VALUE__(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710);
//2012/8/27
int _stdcall TVB59x_GET_MODULATOR_RF_LEVEL_RANGE__(long modulator_type, double *rf_level_min, 
                                       double *rf_level_max, long UseTAT4710);

//2012/9/6 Pcr Restamping control
int _stdcall TVB59x_SET_PCR_STAMP_CNTL__(int _val);
int _stdcall TVB59x_SET_Output_TS_Type__(int _val);
int _stdcall TVB59x_SET_Reset_Control_REG__(int _val);
int _stdcall TVB59x_SET_TsPacket_CNT_Mode__(int _val);
int _stdcall TVB59x_Get_Asi_Input_rate__(int *delta_packet, int *delta_clock);
int _stdcall TVB59x_Modulator_Status_Control__(int modulator, int index, int val);
int _stdcall TVB59x_Get_Modulator_Status__(int index);

#ifdef __cplusplus
//	}
#endif



#endif

