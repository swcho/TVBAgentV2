
#ifndef	TELEVIEW_TVB_MSG_Q_H
#define	TELEVIEW_TVB_MSG_Q_H
/******************************************************************************\
 *
 * File Name   : tlv400e.h
 *
 * Description : header for tlv400e.cpp
 *
 * Copyright 2008 Teleview. All Rights Reserved.
 *
 * Author : Sung, Ju Hyun - Feb 2008
 *
\******************************************************************************/

#include	"tlv_msg.h"


/////////////////////////////////////////////////////////////////////
typedef	enum
{
	MSG_INIT_TVB_MGR_1st			= 0,

//////////////////////////////////////////////////////////////////////
	MSG_tvbxxx_STS_Get,
	MSG_tvbxxx_CTL_Set,

//////////////////////////////////////////////////////////////////////
	MSG_tvbxxx_TVB380_close,	//	retrun	int
	MSG_tvbxxx_TVB380_open,	//	retrun	int
	MSG_tvbxxx_TSE110_close,	//	retrun	int
	MSG_tvbxxx_TSE110_open,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_BOARD_REV,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_BOARD_ID,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_AUTHORIZATION,	//	retrun	int

	MSG_tvbxxx_TSPL_GET_FPGA_INFO,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_ENCRYPTED_SN,	//	retrun	int
	MSG_tvbxxx_TSPL_WRITE_CONTROL_REG,	//	retrun	int
	MSG_tvbxxx_TSPL_READ_CONTROL_REG,	//	return	unsigned long
	MSG_tvbxxx_TSPL_GET_LAST_ERROR,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_BOARD_CONFIG_STATUS,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_BOARD_LED_STATUS,	//	retrun	int
	MSG_tvbxxx_TSPL_REG_EVENT,	//	retrun	int

	MSG_tvbxxx_TSPL_SET_EVENT,	//	retrun	int		//	20
	MSG_tvbxxx_TSPL_REG_COMPLETE_EVENT,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_COMPLETE_EVENT,	//	retrun	int
	MSG_tvbxxx_TVB380_IS_ENABLED_TYPE,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_DMA_STATUS,	//	retrun	int
	MSG_tvbxxx_TSPL_PUT_DATA,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_DATA,	//	retrun	void*
	MSG_tvbxxx_TSPL_SET_FIFO_CONTROL,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_FIFO_CONTROL,	//	retrun	int
	MSG_tvbxxx_TSPL_READ_BLOCK,	//	retrun	void*

	MSG_tvbxxx_TSPL_WRITE_BLOCK_TEST,	//	retrun	void		//	30
	MSG_tvbxxx_TSPL_WRITE_BLOCK,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_DMA_ADDR,	//	retrun	void*
	MSG_tvbxxx_TSPL_GET_DMA_REG_INFO,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_DMA_REG_INFO,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_DEMUX_CONTROL_TEST,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_INFO,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_CONFIG,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_OFFSET_CONFIG,	//	retrun	int
	MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC,	//	retrun	int

	MSG_tvbxxx_TSPL_SET_PLAY_RATE,	//	retrun	int		//	40
	MSG_tvbxxx_TSPL_SET_TSIO_DIRECTION,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_TSIO_STATUS,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_TSIO_DIRECTION,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_CUR_BANK_GROUP,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_BANK_COUNTER,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_SDCON_MODE,	//	retrun	int
	MSG_tvbxxx_TSPL_RESET_SDCON,	//	retrun	int
	MSG_tvbxxx_TSE110_GET_SYNC_STATUS,	//	retrun	int
	MSG_tvbxxx_TSE110_GET_SIGNAL_STATUS,	//	retrun	int
	MSG_tvbxxx_TSE110_GET_SYNC_FORMAT,	//	retrun	int
	MSG_tvbxxx_TSE110_SET_CONFIG,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_SYNC_POSITION,	//	retrun	int
	MSG_tvbxxx_TSPL_DO_DEINTERLEAVING,	//	retrun	int
	MSG_tvbxxx_TSPL_DO_RS_DECODING,	//	retrun	int
	MSG_tvbxxx_TSPL_CONVERT_TO_NI,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_INIT_CIF,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_RUN_CIF,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_MAX_PLAYRATE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_CONFIG,	//	retrun	int
	MSG_tvbxxx_TSPL_GET_AD9775,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_AD9775,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_BANDWIDTH,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_GUARDINTERVAL,	//	retrun	int
	//2011/11/01 added PAUSE
	MSG_tvbxxx_TVB380_SET_MODULATOR_OUTPUT,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_CONSTELLATION,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_FREQ,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_SYMRATE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_CODERATE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_TXMODE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_INTERLEAVE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_IF_FREQ,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_SPECTRUM_INVERSION,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_STOP_MODE,	//	retrun	int
	MSG_tvbxxx_TVB390_PLL2_DOWNLOAD,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_MODE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_SCALE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_INFO,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_AGC,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_ATTEN_VALUE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_DVBH,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_PILOT,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_ROLL_OFF_FACTOR,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_BOARD_CONFIG_STATUS,	//	retrun	int
	MSG_tvbxxx_TVB380_GET_MODULATOR_RF_POWER_LEVEL,	//	retrun	double
	MSG_tvbxxx_TVB380_SET_MODULATOR_BERT_MEASURE,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_DTMB,	//	retrun	int
	MSG_tvbxxx_TVB380_SET_MODULATOR_SDRAM_CLOCK,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_DMA_DIRECTION,	//	retrun	int
	MSG_tvbxxx_TSPL_RESET_IP_CORE,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_MH_MODE,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_MH_PID,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_SYMBOL_CLOCK,	//	retrun	int
	MSG_tvbxxx_TVB380_init,	//	retrun	int
	MSG_tvbxxx_TSE110_init,	//	retrun	int
	MSG_tvbxxx_TSPL_CHECK_LN,	//	retrun	int

	MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC_594,	//	retrun	int
	MSG_tvbxxx_TSPL_SET_PLAY_RATE_594,	//	retrun	int
	//2012/4/12 SINGLE TONE
	MSG_tvbxxx_TVB380_SET_MODULATOR_SINGLE_TONE,	//	retrun	int
//////////////////////////////////////////////////////////////////////
	MSG_tvbxxx_ctl_MAX,

}	ENUM_TVB_MSG_KEY;

/////////////////////////////////////////////////////////////////////
typedef	struct	_si_msg_form
{
	unsigned long	msg_key;
	char				msg_ptr[_USE_QUEUE_MSG_LEN - 4];
}	_TVB_MSG_FORM;

#endif

