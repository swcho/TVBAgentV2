//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_TVB360U_H_
#define	__TLV_TVB360U_H_

#include 	"Chip_util.h"
#include	"attn_ctl.h"

#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CLldTvb360u	:	public	CAttnCtl
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

	long	usr_reqed_play_freq_in_herz;
	long	usr_reqed_nOutputClockSource;

	void	RESET_PLAY_RATE(void);

public:
	CLldTvb360u(void);
	~CLldTvb360u();

	int _stdcall TSPL_SEL_DDSCLOCK_INC_594(long play_freq_in_herz, long multi_module_tag);
	int _stdcall TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource);	// 0 for PLL clock source, 1 for 19.392 MHz oscillator
	int	_stdcall TSPL_SEL_DDSCLOCK_INC(long play_freq_in_herz);
	int	_stdcall TSPL_SET_PLAY_RATE(long play_freq_in_herz, 
			long nOutputClockSource);	// 0 for PLL clock source, 1 for 19.392 MHz oscillator
	int _stdcall TVB380_SET_MODULATOR_BANDWIDTH(long modulator_type,long bandwidth, unsigned long output_frequency);//2150MHz support - int -> unsigned long
	int _stdcall TVB380_SET_MODULATOR_GUARDINTERVAL(long modulator_type,long guard_interval);
	//2011/11/01 added PAUSE
	int _stdcall TVB380_SET_MODULATOR_OUTPUT(long modulator_type,long output);
	
	int _stdcall TVB380_SET_MODULATOR_CONSTELLATION(long modulator_type,long constellation);
	int _stdcall TVB380_SET_MODULATOR_FREQ(long modulator_type,unsigned long output_frequency, long symbol_rate_or_bandwidth);//2150MHz support - int -> unsigned long
	int _stdcall TVB380_SET_MODULATOR_SYMRATE(long modulator_type, unsigned long output_frequency, long symbol_rate_or_bandwidth);//2150MHz support - int -> unsigned long
	int _stdcall TVB380_SET_MODULATOR_CODERATE(long modulator_type, long code_rate);
	int _stdcall TVB380_SET_MODULATOR_TXMODE(long modulator_type, long tx_mode);
	int _stdcall TVB380_SET_MODULATOR_INTERLEAVE(long modulator_type, long interleaving);
	int _stdcall TVB380_SET_MODULATOR_IF_FREQ(long modulator_type, long IF_frequency);
	int _stdcall TVB380_SET_MODULATOR_SPECTRUM_INVERSION(long modulator_type, long spectral_inversion);
	int _stdcall TVB380_IS_ENABLED_TYPE(long modulator_type);
	int _stdcall TVB380_SET_STOP_MODE(long stop_mode);
	int _stdcall TVB390_PLL2_DOWNLOAD(long value);
	int _stdcall TVB380_SET_MODULATOR_PRBS_MODE(long modulator_type, long mode);
	int _stdcall TVB380_SET_MODULATOR_PRBS_SCALE(long modulator_type, double scale);//long -> double
	int _stdcall TVB380_SET_MODULATOR_PRBS_INFO(long modulator_type, long mode, double scale);//long -> double
	int _stdcall ScanTAT4710(void);
	int _stdcall SetTAT4710(double atten_value);
	//2012/2/1 TAT4710
	void _stdcall CloseTAT4710(void);

	int _stdcall TVB380_SET_MODULATOR_AGC(long modulator_type, long agc_on_off, long UseTAT4710);	//2011/6/29 added UseTAT4710
	int _stdcall TVB380_SET_MODULATOR_ATTEN_VALUE(long modulator_type, double atten_value, long UseTAT4710);	//2011/6/29 added UseTAT4710
	int _stdcall TVB380_SET_MODULATOR_DVBH(long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id);
	int _stdcall TVB380_SET_MODULATOR_PILOT(long modulator_type, long  pilot_on_off);
	int _stdcall TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(long modulator_type, long  roll_off_factor);//0(0.20), 1(0.25), 2(0.35), 3(none)
	int _stdcall TVB380_SET_BOARD_CONFIG_STATUS(long modulator_type, long status);
	double _stdcall TVB380_GET_MODULATOR_RF_POWER_LEVEL(long modulator_type, long info_type);
	int _stdcall TVB380_SET_MODULATOR_BERT_MEASURE(long modulator_type, long packet_type);
	int _stdcall TVB380_SET_MODULATOR_DTMB(long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion);
	int _stdcall TVB380_SET_MODULATOR_SDRAM_CLOCK(long modulator_type, long sdram_clock);
	int _stdcall TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction);
	int _stdcall TSPL_RESET_IP_CORE(long modulator_type, long reset_control);
	int _stdcall TSPL_SET_MH_MODE(long modulator_type, long mh_mode);
	int _stdcall TSPL_SET_MH_PID(long modulator_type, long mh_pid);

	void	AssertReleaseResetSignal(unsigned int _bits);
	int 	ResetModBlkAfterChangingHwBlkPara(int _opt);
	int 	ResetModBlkAfterChangingHwDacPara(int _opt);
	//2012/4/12 SINGLE TONE
	int _stdcall TVB380_SET_MODULATOR_SINGLE_TONE(long modulator_type, unsigned long freq, long singleTone);
	int 	Get_SingleTone_Value(long modulator_type, unsigned long freq);

	int 	ResetMultiModBlk_n(int _mod_n);	//	0, 1, 2, 3
	int 	SelMultiModAsiOut_n(int _ts_n); //	0, 1, 2, 3
	int 	SelMultiMod310Out_n(int _ts_n); //	0, 1, 2, 3
	int 	SelMultiModTsOutput_n(int _ctl);
	int 	SelMultiModOperationMode_n(int _ctl);
	int	TSPL_SET_PLAY_RATE_VirtualBd_n(
		int _mod_n, //	1, 2, 3
		long	play_freq_in_herz,
		long	nOutputClockSource); // 0 for PLL clock source, 1 for 19.392 MHz oscillator
	int	TSPL_SEL_DDSCLOCK_INC_VirtualBd_n(int _mod_n, long play_freq_in_herz);
	int	TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n);	//	0, 1, 2, 3
	int	TVB380_SET_MODULATOR_INTERLEAVE_VirtualBd_n(long interleaving);
	int	TVB380_SET_MODULATOR_CONSTELLATION_VirtualBd_n(long modulator_type, long constellation);
	int	TVB380_SET_MODULATOR_NULL_TP_VirtualBd_n(long en_dis);
	int	TVB380_SET_MODULATOR_DISABLE_VirtualBd_n(long _dis);	//	1:disable mod.
	int	TVB380_SET_MODULATOR_PRBS_INFO_VirtualBd_n(long mod_typ, long mode, double scale);
	int	TVB380_SET_MODULATOR_BERT_GEN_VirtualBd_n(long mod_typ, long packet_type);
	int	TVB380_GET_wr_BUF_STS_MultiBd_n(int _mod_n);	//	0, 1, 2, 3
	int	TVB380_GET_wr_cap_BUF_STS_MultiBd_n(void);

	//2012/6/28 multi dvb-t
	int	TVB380_SET_MODULATOR_BANDWIDTH_VirtualBd_n(long modulator_type, long bandwidth);
	int	TVB380_SET_MODULATOR_CODERATE_VirtualBd_n(long modulator_type, long coderate);
	int	TVB380_SET_MODULATOR_GUARDINTERVAL_VirtualBd_n(long modulator_type, long guardinterval);
	int	TVB380_SET_MODULATOR_TXMODE_VirtualBd_n(long modulator_type, long txmode);
	int	TVB380_SET_MODULATOR_CELLID_VirtualBd_n(long modulator_type, long cellid);

	//2012/8/27 new rf level control
	int _stdcall TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(long modulator_type, double rf_level_value, 
                                       long *AmpFlag, long UseTAT4710);
	//2012/8/29 new rf level control
	int _stdcall TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(long modulator_type, double *_p_rf_level_min, 
                                       double *_p_rf_level_max, long UseTAT4710);
	//2012/9/6 Pcr Restamping control
	int _stdcall TVB59x_SET_PCR_STAMP_CNTL(int _val);
	
	//2013/5/31 TVB599 TS output type(ASI, 310M)
	int _stdcall TVB59x_SET_Output_TS_Type(int _val);
	int _stdcall TVB59x_SET_TsPacket_CNT_Mode(int _val);
	int _stdcall TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock);
	int _stdcall TVB59x_Modulator_Status_Control(int modulator, int index, int val);
	int _stdcall TVB59x_Get_Modulator_Status(int index);

	int _stdcall TVB59x_SET_Reset_Control_REG(int _val);
};

#ifdef __cplusplus
}
#endif



#endif
