//=================================================================	
//	WDM_Drv.c / Device Open/Close for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

//=================================================================	
#ifdef WIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<windows.h>
#include	<winioctl.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<commctrl.h>
#include	<time.h>
#include	<setupapi.h>
#include	<memory.h>
// TVB595V1
#include	<math.h>
	
#include	"initguid.h"
#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"

//=================================================================	
// Linux
#else
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include 	<memory.h>
#include	<math.h>
	
#include 	"../tsp100.h"
#include 	"../include/lld_api.h"
#include 	"../include/logfile.h"
//	#include 	"wdm_drv.h"
//	#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif
//	#include 	"playcapt.h"
#include 	"wdm_drv_wrapper.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
DLL_EXPORT int	_stdcall TVB380_close()
{
	return	TVB380_close__();
}
DLL_EXPORT int	_stdcall TVB380_open(int nInitModulatorType, int nInitIF)
{
	return	TVB380_open__(nInitModulatorType, nInitIF);
}
/////////////////////////////////////////////////////////////////
#elif defined(TSPLLD0431_EXPORTS) || defined(TSE110V1)
DLL_EXPORT int	_stdcall TSE110_close()
{
	return	TSE110_close__();
}
DLL_EXPORT int	_stdcall TSE110_open()
{
	return	TSE110_open__();
}
#endif
/////////////////////////////////////////////////////////////////
DLL_EXPORT int _stdcall TSPL_GET_BOARD_REV(void)
{
	return	TSPL_GET_BOARD_REV__();
}
DLL_EXPORT int _stdcall TSPL_GET_BOARD_ID(void)
{
	return	TSPL_GET_BOARD_ID__();
}
DLL_EXPORT int _stdcall TSPL_GET_AUTHORIZATION(void)
{
	return	TSPL_GET_AUTHORIZATION__();
}
DLL_EXPORT int _stdcall TSPL_GET_FPGA_INFO(int info)
{
	return	TSPL_GET_FPGA_INFO__(info);
}
DLL_EXPORT int _stdcall TSPL_GET_ENCRYPTED_SN(int type, char* sn)
{
	return	TSPL_GET_ENCRYPTED_SN__(type, sn);
}
DLL_EXPORT int _stdcall TSPL_WRITE_CONTROL_REG(int Is_PCI_Control, unsigned long address, unsigned long dwData)
{
	return	TSPL_WRITE_CONTROL_REG__(Is_PCI_Control, address, dwData);
}
DLL_EXPORT unsigned long _stdcall TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address)
{
	return	TSPL_READ_CONTROL_REG__(Is_PCI_Control, address);
}
DLL_EXPORT int _stdcall TSPL_GET_LAST_ERROR(void)
{
	return	TSPL_GET_LAST_ERROR__();
}
DLL_EXPORT int _stdcall TSPL_GET_BOARD_CONFIG_STATUS(void)
{
	return	TSPL_GET_BOARD_CONFIG_STATUS__();
}
#if defined(WIN32) || defined(TVB595V1)
DLL_EXPORT int _stdcall TSPL_SET_BOARD_LED_STATUS(int status_LED, int fault_LED)
{
	return	TSPL_SET_BOARD_LED_STATUS__(status_LED, fault_LED);
}
DLL_EXPORT int _stdcall TSPL_REG_EVENT(void* pvoid)
{
	return	TSPL_REG_EVENT__(pvoid);
}
DLL_EXPORT int _stdcall TSPL_REG_COMPLETE_EVENT(void* pvoid)
{
	return	TSPL_REG_COMPLETE_EVENT__(pvoid);
}
DLL_EXPORT int _stdcall TSPL_SET_COMPLETE_EVENT(void)
{
	return	TSPL_SET_COMPLETE_EVENT__();
}
#endif
#if TVB597A_STANDALONE
DLL_EXPORT int _stdcall TVB597F_SET_FLASH(char *szFile)
{
	return	TVB597F_SET_FLASH__(szFile);
}
#else
DLL_EXPORT int _stdcall TVB597F_SET_FLASH(char *szFile)
{
	return	TVB597F_SET_FLASH__(szFile);
}
#endif

/////////////////////////////////////////////////////////////////
#ifdef WIN32
DLL_EXPORT int _stdcall TVB380_IS_ENABLED_TYPE(long modulator_type)
{
	return	TVB380_IS_ENABLED_TYPE__(modulator_type);
}
#else
DLL_EXPORT int TVB380_IS_ENABLED_TYPE(long modulator_type)
{
	return	TVB380_IS_ENABLED_TYPE__(modulator_type);
}
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DLL_EXPORT int _stdcall TSPL_GET_DMA_STATUS(void)
{
	return	TSPL_GET_DMA_STATUS__();
}
DLL_EXPORT int _stdcall TSPL_SET_FIFO_CONTROL(int nDMADirection, int nDMASize)
{
	return	TSPL_SET_FIFO_CONTROL__(nDMADirection, nDMASize);
}
DLL_EXPORT int _stdcall TSPL_GET_FIFO_CONTROL(int nDMADirection, int nDMASize)
{
	return	TSPL_GET_FIFO_CONTROL__(nDMADirection, nDMASize);
}
DLL_EXPORT void* _stdcall TSPL_READ_BLOCK(long dwDMASize)
{
	return	TSPL_READ_BLOCK__(dwDMASize);
}
#ifdef WIN32
DLL_EXPORT int _stdcall TSPL_WRITE_BLOCK(DWORD dwBuffSize,DWORD *dest)
{
	return	TSPL_WRITE_BLOCK__(dwBuffSize, dest);
}
#else
DLL_EXPORT int _stdcall TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, ULONG dwBuffSize, DWORD *dest)
{
	return	TSPL_WRITE_BLOCK__(pdwSrcBuff, dwBuffSize, dest);
}
#endif
DLL_EXPORT void* _stdcall TSPL_GET_DMA_ADDR(void)
{
	return	TSPL_GET_DMA_ADDR__();
}
DLL_EXPORT int _stdcall TSPL_GET_DMA_REG_INFO(int addr)
{
	return	TSPL_GET_DMA_REG_INFO__(addr);
}
DLL_EXPORT int _stdcall TSPL_SET_DMA_REG_INFO(unsigned char nOffset, DWORD dwData)
{
	return	TSPL_SET_DMA_REG_INFO__(nOffset, dwData);
}
DLL_EXPORT int TSPL_SET_DEMUX_CONTROL_TEST(int nState)
{
	return	TSPL_SET_DEMUX_CONTROL_TEST__(nState);
}
DLL_EXPORT int	_stdcall TSPL_SET_SDRAM_BANK_INFO(int nBankNumber, int nBankOffset)
{
	return	TSPL_SET_SDRAM_BANK_INFO__(nBankNumber, nBankOffset);
}
DLL_EXPORT int	_stdcall TSPL_SET_SDRAM_BANK_CONFIG(int nBankConfig)
{
	return	TSPL_SET_SDRAM_BANK_CONFIG__(nBankConfig);
}
DLL_EXPORT int	_stdcall TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig)
{
	return	TSPL_SET_SDRAM_BANK_OFFSET_CONFIG__(nBankConfig);
}
DLL_EXPORT int	_stdcall TSPL_SET_PLAY_RATE(long play_freq_in_herz, long nOutputClockSource)
{
	return	TSPL_SET_PLAY_RATE__(play_freq_in_herz, nOutputClockSource);
}
DLL_EXPORT int 	_stdcall    TSPL_SET_TSIO_DIRECTION(int nDirection)
{
	return	TSPL_SET_TSIO_DIRECTION__(nDirection);
}
DLL_EXPORT int	_stdcall TSPL_GET_TSIO_STATUS(int option)
{
	return	TSPL_GET_TSIO_STATUS__(option);
}
DLL_EXPORT int	_stdcall TSPL_GET_TSIO_DIRECTION(void)
{
	return	TSPL_GET_TSIO_DIRECTION__();
}
DLL_EXPORT int	_stdcall TSPL_GET_CUR_BANK_GROUP(void)
{
	return	TSPL_GET_CUR_BANK_GROUP__();
}
DLL_EXPORT int 	_stdcall    TSPL_SET_SDCON_MODE(int nMode)
{
	return	TSPL_SET_SDCON_MODE__(nMode);
}
DLL_EXPORT int 	_stdcall TSPL_RESET_SDCON(void)
{
	return	TSPL_RESET_SDCON__();
}
DLL_EXPORT int	_stdcall TSE110_GET_SYNC_STATUS(int mode)
{
	return	TSE110_GET_SYNC_STATUS__(mode);
}
DLL_EXPORT int	_stdcall TSE110_GET_SIGNAL_STATUS(int port)
{
	return	TSE110_GET_SIGNAL_STATUS__(port);
}
DLL_EXPORT int	_stdcall TSE110_GET_SYNC_FORMAT(void)
{
	return	TSE110_GET_SYNC_FORMAT__();
}
DLL_EXPORT int	_stdcall TSE110_SET_CONFIG(int port_a_mode,int port_b_mode,int tx_clock,int input_port,int output_a_mode,int output_b_mode)
{
	return	TSE110_SET_CONFIG__(port_a_mode, port_b_mode, tx_clock, input_port, output_a_mode, output_b_mode);
}
DLL_EXPORT int	_stdcall TSPL_GET_SYNC_POSITION(int mode, int type, unsigned char *szBuf, int nlen, int nlen_srch, int nlen_step)
{
	return	TSPL_GET_SYNC_POSITION__(mode, type, szBuf, nlen, nlen_srch, nlen_step);
}
DLL_EXPORT int	_stdcall TSPL_DO_DEINTERLEAVING(unsigned char *szSrc, unsigned char *szDest)
{
	return	TSPL_DO_DEINTERLEAVING__(szSrc, szDest);
}
DLL_EXPORT int	_stdcall TSPL_DO_RS_DECODING(unsigned char *szSrc, unsigned char *szDest, int *format,int *err_blk_cnt,int *recovered_err_cnt,int bypass)
{
	return	TSPL_DO_RS_DECODING__(szSrc, szDest, format, err_blk_cnt, recovered_err_cnt, bypass);
}
DLL_EXPORT int	_stdcall TSPL_CONVERT_TO_NI(unsigned char  *szSrc, unsigned char *szDest, int format)
{
	return	TSPL_CONVERT_TO_NI__(szSrc, szDest, format);
}
DLL_EXPORT int	_stdcall TVB380_SET_MODULATOR_INIT_CIF(void)
{
	return	TVB380_SET_MODULATOR_INIT_CIF__();
}
DLL_EXPORT int	_stdcall TVB380_SET_MODULATOR_RUN_CIF(unsigned char  *szSrc, unsigned char *szDest)
{
	return	TVB380_SET_MODULATOR_RUN_CIF__(szSrc, szDest);
}
DLL_EXPORT int _stdcall TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate)
{
	return	TSPL_SET_MAX_PLAYRATE__(modulator_type, use_max_playrate);
}
DLL_EXPORT int _stdcall TVB380_SET_CONFIG(long modulator_type, long IF_Frequency)
{
	return	TVB380_SET_CONFIG__(modulator_type, IF_Frequency);
}
DLL_EXPORT int _stdcall TSPL_GET_AD9775(long reg)
{
	return	TSPL_GET_AD9775__(reg);
}
DLL_EXPORT int _stdcall TSPL_SET_AD9775(long reg, long data)
{
	return	TSPL_SET_AD9775__(reg, data);
}

DLL_EXPORT int _stdcall TSPL_GET_AD9787(long reg)
{
	return	TSPL_GET_AD9787__(reg);
}
DLL_EXPORT int _stdcall TSPL_SET_AD9787(long reg, long data)
{
	return	TSPL_SET_AD9787__(reg, data);
}

DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_BANDWIDTH(long modulator_type,long bandwidth, unsigned long output_frequency)
{
	return	TVB380_SET_MODULATOR_BANDWIDTH__(modulator_type, bandwidth, output_frequency);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_GUARDINTERVAL(long modulator_type,long guard_interval)
{
	return	TVB380_SET_MODULATOR_GUARDINTERVAL__(modulator_type, guard_interval);
}

//2011/11/01 added PAUSE
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_OUTPUT(long modulator_type,long output)
{
	return	TVB380_SET_MODULATOR_OUTPUT__(modulator_type, output);
}

DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_CONSTELLATION(long modulator_type,long constellation)
{
	return	TVB380_SET_MODULATOR_CONSTELLATION__(modulator_type, constellation);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_FREQ(long modulator_type,unsigned long output_frequency, long symbol_rate_or_bandwidth)
{
	return	TVB380_SET_MODULATOR_FREQ__(modulator_type, output_frequency, symbol_rate_or_bandwidth);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_SYMRATE(long modulator_type, unsigned long output_frequency, long symbol_rate_or_bandwidth)
{
	return	TVB380_SET_MODULATOR_SYMRATE__(modulator_type, output_frequency, symbol_rate_or_bandwidth);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_CODERATE(long modulator_type, long code_rate)
{
	return	TVB380_SET_MODULATOR_CODERATE__(modulator_type, code_rate);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_TXMODE(long modulator_type, long tx_mode)
{
	return	TVB380_SET_MODULATOR_TXMODE__(modulator_type, tx_mode);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_INTERLEAVE(long modulator_type, long interleaving)
{
	return	TVB380_SET_MODULATOR_INTERLEAVE__(modulator_type, interleaving);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_IF_FREQ(long modulator_type, long IF_frequency)
{
	return	TVB380_SET_MODULATOR_IF_FREQ__(modulator_type, IF_frequency);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_SPECTRUM_INVERSION(long modulator_type, long spectral_inversion)
{
	return	TVB380_SET_MODULATOR_SPECTRUM_INVERSION__(modulator_type, spectral_inversion);
}
//	DLL_EXPORT int _stdcall TVB380_IS_ENABLED_TYPE(long modulator_type)	{}
DLL_EXPORT int _stdcall TVB380_SET_STOP_MODE(long stop_mode)
{
	return	TVB380_SET_STOP_MODE__(stop_mode);
}
DLL_EXPORT int _stdcall TVB390_PLL2_DOWNLOAD(long value)
{
	return	TVB390_PLL2_DOWNLOAD__(value);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_PRBS_MODE(long modulator_type, long mode)
{
	return	TVB380_SET_MODULATOR_PRBS_MODE__(modulator_type, mode);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_PRBS_SCALE(long modulator_type, double scale)
{
	return	TVB380_SET_MODULATOR_PRBS_SCALE__(modulator_type, scale);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_PRBS_INFO(long modulator_type, long mode, double scale)
{
	return	TVB380_SET_MODULATOR_PRBS_INFO__(modulator_type, mode, scale);
}
//2011/6/29 added UseTAT4710
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_AGC(long modulator_type, long agc_on_off, long UseTAT4710)
{
	return	TVB380_SET_MODULATOR_AGC__(modulator_type, agc_on_off, UseTAT4710);
}
//2011/6/29	added UseTAT4710
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_ATTEN_VALUE(long modulator_type, double atten_value, long UseTAT4710)
{
	return	TVB380_SET_MODULATOR_ATTEN_VALUE__(modulator_type, atten_value, UseTAT4710);
}

//2012/8/27 new rf level control
DLL_EXPORT int _stdcall TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710)
{
	return	TVB59x_SET_MODULATOR_RF_LEVEL_VALUE__(modulator_type, rf_level_value, AmpFlag, UseTAT4710);
}
//2012/8/29 new rf level control
DLL_EXPORT int _stdcall TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *_p_rf_level_min, 
                                       double *_p_rf_level_max, long UseTAT4710)
{
	return	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE__(modulator_type, _p_rf_level_min, _p_rf_level_max, UseTAT4710);
}
//2012/9/6 Pcr Restamping control
DLL_EXPORT int _stdcall TVB59x_SET_PCR_STAMP_CNTL(int _val)
{
	return TVB59x_SET_PCR_STAMP_CNTL__(_val);
}

DLL_EXPORT int _stdcall TVB59x_SET_Output_TS_Type(int _val)
{
	return TVB59x_SET_Output_TS_Type__(_val);
}

DLL_EXPORT int _stdcall TVB59x_SET_Reset_Control_REG(int _val)
{
	printf("[Debug] call LLD TVB59x_SET_Reset_Control_REG %d\n", _val);
	return TVB59x_SET_Reset_Control_REG__(_val);
}
DLL_EXPORT int _stdcall TSPL_TVB59x_SET_TsPacket_CNT_Mode(int _val)
{
	return TVB59x_SET_TsPacket_CNT_Mode__(_val);
}
DLL_EXPORT int _stdcall TSPL_TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock)
{
	return TVB59x_Get_Asi_Input_rate__(delta_packet, delta_clock);
}
DLL_EXPORT int _stdcall TVB59x_Modulator_Status_Control(long modulator, long index, long val)
{
	return TVB59x_Modulator_Status_Control__(modulator, index, val);
}

DLL_EXPORT int _stdcall TVB59x_Get_Modulator_Status(int index)
{
	return TVB59x_Get_Modulator_Status__(index);
}

DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_DVBH(long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id)
{
	return	TVB380_SET_MODULATOR_DVBH__(modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_PILOT(long modulator_type, long  pilot_on_off)
{
	return	TVB380_SET_MODULATOR_PILOT__(modulator_type, pilot_on_off);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(long modulator_type, long  roll_off_factor)
{
	return	TVB380_SET_MODULATOR_ROLL_OFF_FACTOR__(modulator_type, roll_off_factor);
}
DLL_EXPORT int _stdcall TVB380_SET_BOARD_CONFIG_STATUS(long modulator_type, long status)
{
	return	TVB380_SET_BOARD_CONFIG_STATUS__(modulator_type, status);
}
DLL_EXPORT double _stdcall TVB380_GET_MODULATOR_RF_POWER_LEVEL(long modulator_type, long info_type)
{
	return	TVB380_GET_MODULATOR_RF_POWER_LEVEL__(modulator_type, info_type);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_BERT_MEASURE(long modulator_type, long packet_type)
{
	return	TVB380_SET_MODULATOR_BERT_MEASURE__(modulator_type, packet_type);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_DTMB(long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion)
{
	return	TVB380_SET_MODULATOR_DTMB__(modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);
}
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_SDRAM_CLOCK(long modulator_type, long sdram_clock)
{
	return	TVB380_SET_MODULATOR_SDRAM_CLOCK__(modulator_type, sdram_clock);
}
DLL_EXPORT int _stdcall TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction)
{
	return	TSPL_SET_DMA_DIRECTION__(modulator_type, dma_direction);
}
DLL_EXPORT int _stdcall TSPL_RESET_IP_CORE(long modulator_type, long reset_control)
{
	return	TSPL_RESET_IP_CORE__(modulator_type, reset_control);
}
DLL_EXPORT int _stdcall TSPL_SET_MH_MODE(long modulator_type, long mh_mode)
{
	return	TSPL_SET_MH_MODE__(modulator_type, mh_mode);
}
DLL_EXPORT int _stdcall TSPL_SET_MH_PID(long modulator_type, long mh_pid)
{
	return	TSPL_SET_MH_PID__(modulator_type, mh_pid);
}
DLL_EXPORT int _stdcall TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock)
{
	return	TSPL_SET_SYMBOL_CLOCK__(modulator_type, symbol_clock);
}

/////////////////////////////////////////////////////////////////
DLL_EXPORT int _stdcall TVBxxx_DETECT_BD(int _multi_bd)
{
	return	TVBxxx_DETECT_BD__(_multi_bd);
}
DLL_EXPORT int _stdcall TVBxxx_INIT_BD(int _my_num, void *_my_cxt)
{
	return	TVBxxx_INIT_BD__(_my_num, _my_cxt);
}
DLL_EXPORT int	_stdcall TVBxxx_GET_CNXT_ALL_BD(void *basic_inform)
{
	return	TVBxxx_GET_CNXT_ALL_BD__(basic_inform);
}
DLL_EXPORT int	_stdcall TVBxxx_GET_CNXT_MINE(void *_cxt)
{
	return	TVBxxx_GET_CNXT_MINE__(_cxt);
}
DLL_EXPORT int	_stdcall TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *basic_inform)
{
	return	TVBxxx_DUP_BD__(from_slot_num, to_slot_num, basic_inform);
}

DLL_EXPORT int _stdcall TSPL_CNT_MULTI_VSB_RFOUT(void)
{
	return	TSPL_CNT_MULTI_VSB_RFOUT__();
}
DLL_EXPORT int _stdcall TSPL_CNT_MULTI_QAM_RFOUT(void)
{
	return	TSPL_CNT_MULTI_QAM_RFOUT__();
}
//2012/6/28
DLL_EXPORT int _stdcall TSPL_CNT_MULTI_DVBT_RFOUT(void)
{
	return	TSPL_CNT_MULTI_DVBT_RFOUT__();
}

#ifdef WIN32
DLL_EXPORT int _stdcall TSPL_CHECK_LN(char* ln)
{
	return	TSPL_CHECK_LN__(ln);
}
//2011/5/4 AD9852 MAX CLK
DLL_EXPORT int _stdcall TSPL_SET_AD9852_MAX_CLOCK(long value)
{
	return	TSPL_SET_AD9852_MAX_CLOCK__(value);
}

#else
DLL_EXPORT	int TSPL_CHECK_LN(char* ln)
{
	return	TSPL_CHECK_LN__(ln);
}
#endif

DLL_EXPORT int _stdcall TSPL_SEL_DDSCLOCK_INC_594(long play_freq_in_herz, long multi_module_tag)
{
	return	TSPL_SEL_DDSCLOCK_INC_594__(play_freq_in_herz, multi_module_tag);
}
DLL_EXPORT int _stdcall TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource)
{
	return	TSPL_SET_PLAY_RATE_594__(play_freq_in_herz, multi_module_tag, nOutputClockSource);
}


DLL_EXPORT int	_stdcall TVB380_GET_wr_BUF_STS_MultiBd_n(int _mod_n)
{
	return	TVB380_GET_wr_BUF_STS_MultiBd_n__(_mod_n);
}
DLL_EXPORT int	_stdcall TVB380_GET_wr_cap_BUF_STS_MultiBd_n(void)
{
	return	TVB380_GET_wr_cap_BUF_STS_MultiBd_n__();
}
DLL_EXPORT int	_stdcall TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n)	//	0, 1, 2, 3
{
	return	TSPL_SEL_TS_TAG_VirtualBd_n__(_mod_n);
}
DLL_EXPORT int 	_stdcall SelMultiModAsiOut_n(int _ts_n)
{
	return	SelMultiModAsiOut_n__(_ts_n);
}
DLL_EXPORT int 	_stdcall SelMultiMod310Out_n(int _ts_n)
{
	return	SelMultiMod310Out_n__(_ts_n);
}
DLL_EXPORT int 	_stdcall SelMultiModTsOutput_n(int _ctl)
{
	return	SelMultiModTsOutput_n__(_ctl);
}
DLL_EXPORT int 	_stdcall SelMultiModOperationMode_n(int _ctl)
{
	return	SelMultiModOperationMode_n__(_ctl);
}
//2012/4/12 SINGLE TONE
DLL_EXPORT int _stdcall TVB380_SET_MODULATOR_SINGLE_TONE(long modulator_type, unsigned long freq, long singleTone)
{
	return	TVB380_SET_MODULATOR_SINGLE_TONE__(modulator_type, freq, singleTone);
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////



