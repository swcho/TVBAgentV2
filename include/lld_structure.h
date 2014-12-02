#ifndef	__INCLUDE_LLD_STRUCTURE_	
#define	__INCLUDE_LLD_STRUCTURE_
#if defined(WIN32)
#include	<Windows.h>
#else
typedef unsigned int DWORD;
#endif

//#include "../include/hld_type.h"
/////////////////////////////////////////////////////////////////
typedef	struct	_bd_conf_cnxt
{
/////////////////////////////////////////////// the following vals are common for hld and lld.
	int	__hld_mgr_activated__;

	int	__id__;	//	-1. if no bd information is assgined.
	int	__iam_real__;	//	0 : virtual, 1 : real
	int	__id_my_phy__;	//	-1 : not installed
	int	__id_ts_seq__;	//	0 ~, and the 0 means real slot stream.
	int	__cnt_my_sub_ts_vsb__;	//	0 : support one real ts only
	int	__cnt_my_sub_ts_qam__;	//	0 : support one real ts only
	//2012/6/28 multi dvb-t
	int __cnt_my_sub_ts_dvbt__; //	0 : support one real ts only
	//2013/7/17 Eeprom R/W
	int __dac_i_offset;
	int __dac_q_offset;
/////////////////////////////////////////////// the following vals are initialized at routine TVBxxx_DETECT_BD()
#ifdef WIN32
	HANDLE	_dev_hnd;
#else
	int	_dev_hnd;
#endif
	void	*_dev_hnd_usb;	//		struct usb_dev_handle	*hDevice_usb;
	char	_location_name_string[__MAX_NAME_STR_BD_LOCATION_];
	int _bd_typ_small_mem_space;	//	means need remap to interface fpga reg. ___ADDR_REMAP_METHOD___
	int _bd_typ_597_v1_v2_;

///////////////////////////////////////////////	the following vals are initialized at routine TVBxxx_INIT_BD()
#ifdef WIN32
	HANDLE	_dma_mutex_hnd;
	HANDLE	_epld_mutex_hnd;
#else
	pthread_mutex_t _dma_mutex_hnd;
	int _dma_mutex_cnt;
#endif
	DWORD	*_dma_buf_page_unmapped;
	int _bd_typ_id;	//	___ID_BDs___
	int _bd_authorization;	//	why use this?
	int _bd_revision;
	int _bd_config;	//	???
	int _bd_use_AMP;
	int _bd_revision_tvb595;	//	___ID_BDs_REV___
	int _bd_revision_ext;	//	___REG__MAP_593___, ___ID_BDs_REV_EX_and_ModelNumber___
	int _bd_option;

	int _bd_location;	//	dummy, not use
	int _bd_bus_number;	//	dummy, not use

/////////////////////////////////////////////// the following vals are initialized at routine TVB380_open()
	int _modulator_enabled;	//	authorized license number. bit ored value of TLV_MODULATOR_TYPE.
	int _modulator_typ_running_now;	//	TLV_MODULATOR_TYPE
	int _use_10m_ref_clock;	//	not use. PCI593_REG_SELECT_EXT_OSC
	unsigned long	_if_freq_offset;	//	IF_OUT_36MHZ, IF_OUT_44MHZ

	int _fpga_id;		//	fpga type. red value of reg TSP_MEM_ADDR_Chip_STATUS_REG.
	int _fpga_ver;
	int _fpga_bld;	//	red value of reg TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_BLD
	int _fpga_rbf;	//	red value of reg TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_BLD
	int _fpga_iq_play;	//	reg. TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_CAP
	int _fpga_iq_capture;	//	reg. TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_CAP

/////////////////////////////////////////////// the following vals are initialized at routine TSPL_SET_SDRAM_BANK_INFO()
	int _fpga_sdram_bnk_offset; //	64, 128, 256, 512, 1024 -- run time variable.
	int _fpga_sdram_bnk_cnt;	//	????. 0 ~ 7 -- run time variable.

	//2013/5/23
	int	_fpga_NumOfLED;
	int _fpga_UsbResetSupport;
	int _fpga_ClockGeneratorModel;

	int _hmc833_SerialPortMode;
	int _hmc1033_SerialPortMode;
	char szLog_SubFolder[16];
}	_BD_CONF_CNXT;













#endif
