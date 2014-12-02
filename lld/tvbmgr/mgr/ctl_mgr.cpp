
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

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<memory.h>

#include	"ctl_mgr.h"
#include 	"playcapt.h"

static	const	char	*__dbg_str_msg_key[]	=
{
	"MSG_INIT_TVB_MGR_1st",

//////////////////////////////////////////////////////////////////////
	"MSG_tvbxxx_STS_Get",
	"MSG_tvbxxx_CTL_Set",

//////////////////////////////////////////////////////////////////////
	"MSG_tvbxxx_TVB380_close",	//	retrun	int
	"MSG_tvbxxx_TVB380_open",	//	retrun	int
	"MSG_tvbxxx_TSE110_close",	//	retrun	int
	"MSG_tvbxxx_TSE110_open",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_BOARD_REV",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_BOARD_ID",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_AUTHORIZATION",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_FPGA_INFO",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_ENCRYPTED_SN",	//	retrun	int
	"MSG_tvbxxx_TSPL_WRITE_CONTROL_REG",	//	retrun	int
	"MSG_tvbxxx_TSPL_READ_CONTROL_REG",	//	return	unsigned long
	"MSG_tvbxxx_TSPL_GET_LAST_ERROR",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_BOARD_CONFIG_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_BOARD_LED_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSPL_REG_EVENT",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_EVENT",	//	retrun	int
	"MSG_tvbxxx_TSPL_REG_COMPLETE_EVENT",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_COMPLETE_EVENT",	//	retrun	int
	"MSG_tvbxxx_TVB380_IS_ENABLED_TYPE",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_DMA_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSPL_PUT_DATA",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_DATA",	//	retrun	void*
	"MSG_tvbxxx_TSPL_SET_FIFO_CONTROL",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_FIFO_CONTROL",	//	retrun	int
	"MSG_tvbxxx_TSPL_READ_BLOCK",	//	retrun	void*
	"MSG_tvbxxx_TSPL_WRITE_BLOCK_TEST",	//	retrun	void
	"MSG_tvbxxx_TSPL_WRITE_BLOCK",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_DMA_ADDR",	//	retrun	void*
	"MSG_tvbxxx_TSPL_GET_DMA_REG_INFO",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_DMA_REG_INFO",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_DEMUX_CONTROL_TEST",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_SDRAM_BANK_INFO",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_SDRAM_BANK_CONFIG",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_SDRAM_BANK_OFFSET_CONFIG",	//	retrun	int
	"MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_PLAY_RATE",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_TSIO_DIRECTION",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_TSIO_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_TSIO_DIRECTION",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_CUR_BANK_GROUP",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_BANK_COUNTER",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_SDCON_MODE",	//	retrun	int
	"MSG_tvbxxx_TSPL_RESET_SDCON",	//	retrun	int
	"MSG_tvbxxx_TSE110_GET_SYNC_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSE110_GET_SIGNAL_STATUS",	//	retrun	int
	"MSG_tvbxxx_TSE110_GET_SYNC_FORMAT",	//	retrun	int
	"MSG_tvbxxx_TSE110_SET_CONFIG",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_SYNC_POSITION",	//	retrun	int
	"MSG_tvbxxx_TSPL_DO_DEINTERLEAVING",	//	retrun	int
	"MSG_tvbxxx_TSPL_DO_RS_DECODING",	//	retrun	int
	"MSG_tvbxxx_TSPL_CONVERT_TO_NI",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_INIT_CIF",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_RUN_CIF",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_MAX_PLAYRATE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_CONFIG",	//	retrun	int
	"MSG_tvbxxx_TSPL_GET_AD9775",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_AD9775",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_BANDWIDTH",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_GUARDINTERVAL",	//	retrun	int
	//2011/11/01 added PAUSE
	"MSG_tvbxxx_TVB380_SET_MODULATOR_OUTPUT",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_CONSTELLATION",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_FREQ",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_SYMRATE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_CODERATE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_TXMODE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_INTERLEAVE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_IF_FREQ",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_SPECTRUM_INVERSION",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_STOP_MODE",	//	retrun	int
	"MSG_tvbxxx_TVB390_PLL2_DOWNLOAD",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_MODE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_SCALE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_INFO",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_AGC",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_ATTEN_VALUE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_DVBH",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_PILOT",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_ROLL_OFF_FACTOR",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_BOARD_CONFIG_STATUS",	//	retrun	int
	"MSG_tvbxxx_TVB380_GET_MODULATOR_RF_POWER_LEVEL",	//	retrun	double
	"MSG_tvbxxx_TVB380_SET_MODULATOR_BERT_MEASURE",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_DTMB",	//	retrun	int
	"MSG_tvbxxx_TVB380_SET_MODULATOR_SDRAM_CLOCK",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_DMA_DIRECTION",	//	retrun	int
	"MSG_tvbxxx_TSPL_RESET_IP_CORE",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_MH_MODE",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_MH_PID",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_SYMBOL_CLOCK",	//	retrun	int
	"MSG_tvbxxx_TVB380_init",	//	retrun	int
	"MSG_tvbxxx_TSE110_init",	//	retrun	int
	"MSG_tvbxxx_TSPL_CHECK_LN",	//	retrun	int
	"MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC_594",	//	retrun	int
	"MSG_tvbxxx_TSPL_SET_PLAY_RATE_594",	//	retrun	int

	//2012/4/12 SINGLE TONE
	"MSG_tvbxxx_TVB380_SET_MODULATOR_SINGLE_TONE",	//	retrun	int
//////////////////////////////////////////////////////////////////////
	"MSG_tvbxxx_ctl_MAX",
	"null",
};

//////////////////////////////////////////////////////////////////////////////////
static	CCtlMgr		*_CtlMgr = NULL;
static	CMsg			*Tmsg = NULL;
//2011/4/12
static int msg_index;
//////////////////////////////////////////////////////////////////////////////////
void	CCtlMgr::Prt_RcvedMsg (unsigned int _cmd)
{
return;


	static	char	__msg_prt_buf[128];

	if (_cmd >= MSG_tvbxxx_ctl_MAX)
	{
		return;
	}
	switch(_cmd)
	{
	case	MSG_tvbxxx_TSPL_SET_DMA_REG_INFO:
	case	MSG_tvbxxx_TSPL_WRITE_CONTROL_REG:
	case	MSG_tvbxxx_TSPL_SET_BOARD_LED_STATUS:
	case	MSG_tvbxxx_TSPL_GET_AD9775:
	case	MSG_tvbxxx_TSPL_GET_DMA_STATUS:
	case	MSG_tvbxxx_TSPL_WRITE_BLOCK:
//	case	MSG_tvbxxx_TVB380_GET_MODULATOR_RF_POWER_LEVEL:
		break;
	default:
		sprintf(__msg_prt_buf, "[Cm] : %s", __dbg_str_msg_key[_cmd]);
		LLDIf.LldPrint_2(__msg_prt_buf, 0, 0);
		break;
	}
}
//////////////////////////////////////////////////////////////////////////////////
void	CCtlMgr::InitOnManager1st (void)
{
	int	wait_loop = 0;

//////////////////////////////////////////////////////////////////////////////////
#ifdef	__USE_TIMER_MSG__
	Tmsg->ActTmMsg(__MSG_CORE__ + msg_index);
#endif
	
//////////////////////////////////////////////////////////////////////////////////
#if	0
	while(!is_ok_hw_init())
	{
		usleep(1000000);
		wait_loop++;
		if (wait_loop >= 40)
		{
			break;
		}
	}
	LldPrint_2 ("[Cm] init : [%d]\n", wait_loop);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
void	CCtlMgr::PollWTimeout(void)
{
	if (pause_system_polling == TLV_YES)
	{
		return;
	}
}
void	CCtlMgr::wait_until(long *_done_flag, long _timeout)
{
	while(_timeout > 0)
	{
		if (*_done_flag == TLV_NO)
		{
			break;
		}
		usleep(100);	//	1-msec
		_timeout -= 100;
	}
	if (_timeout <= 0)
	{
		LLDIf.LldPrint_2("[Cm] : TIMEOUT", __req_sts_cnxt._cmd, __req_ctl_cnxt._cmd);
	}
}
int	CCtlMgr::ReqMgrCntl_HwStsApi(long _cmd,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	int	_ret;

	LLDIf.LockHwMutex();

	pause_system_polling = TLV_YES;

	__req_sts_cnxt._cmd = _cmd;
	__req_sts_cnxt._ret_val = TLV_ERR;

	__req_sts_cnxt._arg_int1 = _arg_int1;		__req_sts_cnxt._arg_int2 = _arg_int2;
	__req_sts_cnxt._arg_int3 = _arg_int3;		__req_sts_cnxt._arg_int4 = _arg_int4;
	__req_sts_cnxt._arg_int5 = _arg_int5;		__req_sts_cnxt._arg_int6 = _arg_int6;
	__req_sts_cnxt._arg_int7 = _arg_int7;		__req_sts_cnxt._arg_int8 = _arg_int8;
	__req_sts_cnxt._arg_int9 = _arg_int9;

	__req_sts_cnxt._request = TLV_YES;

	_tvbxxx_msg_snd(MSG_tvbxxx_STS_Get, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);

	wait_until(&__req_sts_cnxt._request, wait_msec);
	_ret = __req_sts_cnxt._ret_val;

	pause_system_polling = TLV_NO;
	LLDIf.UnlockHwMutex();

	return	_ret;
}
void	*CCtlMgr::ReqMgrCntl_HwStsApiPtr(long _cmd,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	void		*_ret_void;

	LLDIf.LockHwMutex();

	pause_system_polling = TLV_YES;

	__req_sts_cnxt._cmd = _cmd;
	__req_sts_cnxt._ret_void = NULL;

	__req_sts_cnxt._arg_int1 = _arg_int1;		__req_sts_cnxt._arg_int2 = _arg_int2;
	__req_sts_cnxt._arg_int3 = _arg_int3;		__req_sts_cnxt._arg_int4 = _arg_int4;
	__req_sts_cnxt._arg_int5 = _arg_int5;		__req_sts_cnxt._arg_int6 = _arg_int6;
	__req_sts_cnxt._arg_int7 = _arg_int7;		__req_sts_cnxt._arg_int8 = _arg_int8;
	__req_sts_cnxt._arg_int9 = _arg_int9;

	__req_sts_cnxt._request = TLV_YES;

	_tvbxxx_msg_snd(MSG_tvbxxx_STS_Get, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);

	wait_until(&__req_sts_cnxt._request, wait_msec);
	_ret_void = __req_sts_cnxt._ret_void;

	pause_system_polling = TLV_NO;
	LLDIf.UnlockHwMutex();

	return	_ret_void;
}
double	CCtlMgr::ReqMgrCntl_HwStsApiDouble(long _cmd,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	double	_ret_double;

	LLDIf.LockHwMutex();

	pause_system_polling = TLV_YES;

	__req_sts_cnxt._cmd = _cmd;
	__req_sts_cnxt._ret_double = 0.;

	__req_sts_cnxt._arg_int1 = _arg_int1;		__req_sts_cnxt._arg_int2 = _arg_int2;
	__req_sts_cnxt._arg_int3 = _arg_int3;		__req_sts_cnxt._arg_int4 = _arg_int4;
	__req_sts_cnxt._arg_int5 = _arg_int5;		__req_sts_cnxt._arg_int6 = _arg_int6;
	__req_sts_cnxt._arg_int7 = _arg_int7;		__req_sts_cnxt._arg_int8 = _arg_int8;
	__req_sts_cnxt._arg_int9 = _arg_int9;

	__req_sts_cnxt._request = TLV_YES;

	_tvbxxx_msg_snd(MSG_tvbxxx_STS_Get, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);

	wait_until(&__req_sts_cnxt._request, wait_msec);
	_ret_double = __req_sts_cnxt._ret_double;

	pause_system_polling = TLV_NO;
	LLDIf.UnlockHwMutex();

	return	_ret_double;
}
void	CCtlMgr::ExecMgrCntl_HwStsApi(void)
{
	int	_ret;
	double	_ret_double;
	void		*_ret_void;

	if (__req_sts_cnxt._request == TLV_YES)
	{
//		LldPrint_2("[Cm] cmgr-cntl-ch-api [%d].[%d,%d,%d]\n",__req_sts_cnxt._cmd,__req_sts_cnxt._disp,__req_sts_cnxt._arg_int1,__req_sts_cnxt._arg_int2);
		switch(__req_sts_cnxt._cmd)
		{
		case	MSG_tvbxxx_TSPL_GET_BOARD_REV:	//	retrun	int
			_ret = LLDIf.TSPL_GET_BOARD_REV();
			break;
		case	MSG_tvbxxx_TSPL_GET_BOARD_ID:	//	retrun	int
			_ret = LLDIf.TSPL_GET_BOARD_ID();
			break;
		case	MSG_tvbxxx_TSPL_GET_AUTHORIZATION:	//	retrun	int
			_ret = LLDIf.TSPL_GET_AUTHORIZATION();
			break;
		case	MSG_tvbxxx_TSPL_GET_FPGA_INFO:	//	retrun	int
			_ret = LLDIf.TSPL_GET_FPGA_INFO(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_GET_ENCRYPTED_SN:	//	retrun	int
			_ret = LLDIf.TSPL_GET_ENCRYPTED_SN(__req_sts_cnxt._arg_int1, (char *)__req_sts_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_READ_CONTROL_REG:	//	return	unsigned long
			_ret = LLDIf.TSPL_READ_CONTROL_REG(__req_sts_cnxt._arg_int1, __req_sts_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_GET_LAST_ERROR: //	retrun	int
			_ret = LLDIf.TSPL_GET_LAST_ERROR();
			break;
		case	MSG_tvbxxx_TSPL_GET_BOARD_CONFIG_STATUS:	//	retrun	int
			_ret = LLDIf.TSPL_GET_BOARD_CONFIG_STATUS();
			break;
		case	MSG_tvbxxx_TSPL_GET_DMA_STATUS: //	retrun	int
			_ret = LLDIf.TSPL_GET_DMA_STATUS();
			break;
		case	MSG_tvbxxx_TSPL_GET_DATA:	//	retrun	void*
			_ret_void = LLDIf.TSPL_GET_DATA(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_GET_FIFO_CONTROL:	//	retrun	int
			_ret = LLDIf.TSPL_GET_FIFO_CONTROL(__req_sts_cnxt._arg_int1, __req_sts_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_READ_BLOCK: //	retrun	void*
			_ret_void = LLDIf.TSPL_READ_BLOCK(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_GET_DMA_ADDR:	//	retrun	void*
			_ret_void = LLDIf.TSPL_GET_DMA_ADDR();
			break;
		case	MSG_tvbxxx_TSPL_GET_DMA_REG_INFO:	//	retrun	int
			_ret = LLDIf.TSPL_GET_DMA_REG_INFO(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSE110_GET_SYNC_STATUS:	//	retrun	int
			_ret = LLDIf.TSE110_GET_SYNC_STATUS(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSE110_GET_SIGNAL_STATUS:	//	retrun	int
			_ret = LLDIf.TSE110_GET_SIGNAL_STATUS(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSE110_GET_SYNC_FORMAT:	//	retrun	int
			_ret = LLDIf.TSE110_GET_SYNC_FORMAT();
			break;
		case	MSG_tvbxxx_TSPL_GET_SYNC_POSITION:	//	retrun	int
			_ret = LLDIf.TSPL_GET_SYNC_POSITION(
				__req_sts_cnxt._arg_int1,
				__req_sts_cnxt._arg_int2,
				(unsigned char *)__req_sts_cnxt._arg_int3,
				__req_sts_cnxt._arg_int4,
				__req_sts_cnxt._arg_int5,
				__req_sts_cnxt._arg_int6);
			break;
		case	MSG_tvbxxx_TSPL_GET_AD9775: //	retrun	int
			_ret = LLDIf.TSPL_GET_AD9775(__req_sts_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TVB380_GET_MODULATOR_RF_POWER_LEVEL: //	retrun	double
			_ret_double = LLDIf.TVB380_GET_MODULATOR_RF_POWER_LEVEL(__req_sts_cnxt._arg_int1,
				__req_sts_cnxt._arg_int2);
			break;

		default:
			LLDIf.LldPrint_2("[Cm] undefined cmd...GetSts",__req_sts_cnxt._cmd,0);
			break;
		}

		__req_sts_cnxt._ret_val = _ret;
		__req_sts_cnxt._ret_double = _ret_double;
		__req_sts_cnxt._ret_void = _ret_void;

//		__req_sts_cnxt._cmd = 0;
		__req_sts_cnxt._request = TLV_NO;
	}
}
int	CCtlMgr::ReqMgrCntl_HwCtlApi(long _cmd,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	double _arg_dbl,
	long wait_msec)
{
	int	_ret;

	LLDIf.LockHwMutex();

	pause_system_polling = TLV_YES;

	__req_ctl_cnxt._cmd = _cmd;
	__req_ctl_cnxt._ret_val = TLV_ERR;

	__req_ctl_cnxt._arg_int1 = _arg_int1;		__req_ctl_cnxt._arg_int2 = _arg_int2;
	__req_ctl_cnxt._arg_int3 = _arg_int3;		__req_ctl_cnxt._arg_int4 = _arg_int4;
	__req_ctl_cnxt._arg_int5 = _arg_int5;		__req_ctl_cnxt._arg_int6 = _arg_int6;
	__req_ctl_cnxt._arg_int7 = _arg_int7;		__req_ctl_cnxt._arg_int8 = _arg_int8;
	__req_ctl_cnxt._arg_int9 = _arg_int9;		__req_ctl_cnxt._arg_double = _arg_dbl;

	__req_ctl_cnxt._request = TLV_YES;

	_tvbxxx_msg_snd(MSG_tvbxxx_CTL_Set, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);

	wait_until(&__req_ctl_cnxt._request, wait_msec);
	_ret = __req_ctl_cnxt._ret_val;

	pause_system_polling = TLV_NO;
	LLDIf.UnlockHwMutex();

	return	_ret;
}
void	CCtlMgr::ExecMgrCntl_HwCtlApi(void)
{
	int	_ret;

	if (__req_ctl_cnxt._request == TLV_YES)
	{
		switch(__req_ctl_cnxt._cmd)
		{
		case	MSG_tvbxxx_TVB380_close:	//	retrun	int
			_ret = LLDIf.TVBxxx_CLOSE_BD();
			break;
		case	MSG_tvbxxx_TVB380_open: //	retrun	int
			_ret = LLDIf.TVB380_open(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSE110_close:	//	retrun	int
//			_ret = LLDIf.TSE110_close();
			break;
		case	MSG_tvbxxx_TSE110_open: //	retrun	int
//			_ret = LLDIf.TSE110_open();
			break;
		case	MSG_tvbxxx_TSPL_WRITE_CONTROL_REG:	//	retrun	int
			_ret = LLDIf.TSPL_WRITE_CONTROL_REG(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2, __req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TSPL_SET_BOARD_LED_STATUS:	//	retrun	int
			_ret = LLDIf.TSPL_SET_BOARD_LED_STATUS(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_REG_EVENT:	//	retrun	int
			_ret = LLDIf.TSPL_REG_EVENT((void *)__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_EVENT:	//	retrun	int
			_ret = LLDIf.TSPL_SET_EVENT();
			break;
		case	MSG_tvbxxx_TSPL_REG_COMPLETE_EVENT: //	retrun	int
			_ret = LLDIf.TSPL_REG_COMPLETE_EVENT((void *)__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_COMPLETE_EVENT: //	retrun	int
			_ret = LLDIf.TSPL_SET_COMPLETE_EVENT();
			break;
		case	MSG_tvbxxx_TVB380_IS_ENABLED_TYPE:	//	retrun	int
			_ret = LLDIf.TVB380_IS_ENABLED_TYPE(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_PUT_DATA:	//	retrun	int
			_ret = LLDIf.TSPL_PUT_DATA(__req_ctl_cnxt._arg_int1, (DWORD *)__req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_FIFO_CONTROL:	//	retrun	int
			_ret = LLDIf.TSPL_SET_FIFO_CONTROL(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_WRITE_BLOCK_TEST:	//	retrun	void
			LLDIf.TSPL_WRITE_BLOCK_TEST(
				(DWORD *)__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				(DWORD *)__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TSPL_WRITE_BLOCK:	//	retrun	int
			_ret = LLDIf.TSPL_WRITE_BLOCK(
				(DWORD *)__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				(DWORD *)__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TSPL_SET_DMA_REG_INFO:	//	retrun	int
			_ret = LLDIf.TSPL_SET_DMA_REG_INFO(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_DEMUX_CONTROL_TEST: //	retrun	int
			_ret = LLDIf.TSPL_SET_DEMUX_CONTROL_TEST(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_INFO:	//	retrun	int
			_ret = LLDIf.TSPL_SET_SDRAM_BANK_INFO(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_CONFIG:	//	retrun	int
			_ret = LLDIf.TSPL_SET_SDRAM_BANK_CONFIG(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_SDRAM_BANK_OFFSET_CONFIG:	//	retrun	int
			_ret = LLDIf.TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC:	//	retrun	int
			_ret = LLDIf.TSPL_SEL_DDSCLOCK_INC(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_PLAY_RATE:	//	retrun	int
			_ret = LLDIf.TSPL_SET_PLAY_RATE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_TSIO_DIRECTION: //	retrun	int
			_ret = LLDIf.TSPL_SET_TSIO_DIRECTION(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_GET_TSIO_STATUS:	//	retrun	int
			_ret = LLDIf.TSPL_GET_TSIO_STATUS(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_GET_TSIO_DIRECTION: //	retrun	int
			_ret = LLDIf.TSPL_GET_TSIO_DIRECTION();
			break;
		case	MSG_tvbxxx_TSPL_GET_CUR_BANK_GROUP: //	retrun	int
			_ret = LLDIf.TSPL_GET_CUR_BANK_GROUP();
			break;
		case	MSG_tvbxxx_TSPL_SET_BANK_COUNTER:	//	retrun	int
			_ret = LLDIf.TSPL_SET_BANK_COUNTER(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SET_SDCON_MODE: //	retrun	int
			_ret = LLDIf.TSPL_SET_SDCON_MODE(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_RESET_SDCON:	//	retrun	int
			_ret = LLDIf.TSPL_RESET_SDCON();
			break;
		case	MSG_tvbxxx_TSE110_SET_CONFIG:	//	retrun	int
			_ret = LLDIf.TSE110_SET_CONFIG(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3,
				__req_ctl_cnxt._arg_int4,
				__req_ctl_cnxt._arg_int5,
				__req_ctl_cnxt._arg_int6);
			break;
		case	MSG_tvbxxx_TSPL_DO_DEINTERLEAVING:	//	retrun	int
			_ret = LLDIf.TSPL_DO_DEINTERLEAVING(
				(unsigned char *)__req_ctl_cnxt._arg_int1,
				(unsigned char *)__req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_DO_RS_DECODING: //	retrun	int
			_ret = LLDIf.TSPL_DO_RS_DECODING(
				(unsigned char *)__req_ctl_cnxt._arg_int1,
				(unsigned char *)__req_ctl_cnxt._arg_int2,
				(int *)__req_ctl_cnxt._arg_int3,
				(int *)__req_ctl_cnxt._arg_int4,
				(int *)__req_ctl_cnxt._arg_int5,
				__req_ctl_cnxt._arg_int6);
			break;
		case	MSG_tvbxxx_TSPL_CONVERT_TO_NI:	//	retrun	int
			_ret = LLDIf.TSPL_CONVERT_TO_NI(
				(unsigned char *)__req_ctl_cnxt._arg_int1,
				(unsigned char *)__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_INIT_CIF:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_INIT_CIF();
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_RUN_CIF:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_RUN_CIF(
				(unsigned char *)__req_ctl_cnxt._arg_int1,
				(unsigned char *)__req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_MAX_PLAYRATE:	//	retrun	int
			_ret = LLDIf.TSPL_SET_MAX_PLAYRATE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_CONFIG:	//	retrun	int
			_ret = LLDIf.TVB380_SET_CONFIG(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_AD9775: //	retrun	int
			_ret = LLDIf.TSPL_SET_AD9775(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_BANDWIDTH:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_BANDWIDTH(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_GUARDINTERVAL:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_GUARDINTERVAL(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		//2011/11/01 added PAUSE
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_OUTPUT:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_OUTPUT(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_CONSTELLATION:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_CONSTELLATION(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_FREQ:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_FREQ(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_SYMRATE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_SYMRATE(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_CODERATE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_CODERATE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_TXMODE: //	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_TXMODE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_INTERLEAVE: //	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_INTERLEAVE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_IF_FREQ:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_IF_FREQ(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_SPECTRUM_INVERSION: //	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_SPECTRUM_INVERSION(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_STOP_MODE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_STOP_MODE(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TVB390_PLL2_DOWNLOAD:	//	retrun	int
			_ret = LLDIf.TVB390_PLL2_DOWNLOAD(__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_MODE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_PRBS_MODE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_SCALE: //	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_PRBS_SCALE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_double);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_PRBS_INFO:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_PRBS_INFO(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_double);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_AGC:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_AGC(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2, __req_ctl_cnxt._arg_int3);	//2011/6/29 added UseTAT4710
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_ATTEN_VALUE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_ATTEN_VALUE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_double, __req_ctl_cnxt._arg_int2);		//2011/6/29 added UseTAT4710
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_DVBH:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_DVBH(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3,
				__req_ctl_cnxt._arg_int4,
				__req_ctl_cnxt._arg_int5,
				__req_ctl_cnxt._arg_int6);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_PILOT:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_PILOT(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_ROLL_OFF_FACTOR:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_BOARD_CONFIG_STATUS:	//	retrun	int
			_ret = LLDIf.TVB380_SET_BOARD_CONFIG_STATUS(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_BERT_MEASURE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_BERT_MEASURE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_DTMB:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_DTMB(
				__req_ctl_cnxt._arg_int1,
				__req_ctl_cnxt._arg_int2,
				__req_ctl_cnxt._arg_int3,
				__req_ctl_cnxt._arg_int4,
				__req_ctl_cnxt._arg_int5,
				__req_ctl_cnxt._arg_int6,
				__req_ctl_cnxt._arg_int7,
				__req_ctl_cnxt._arg_int8);
			break;
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_SDRAM_CLOCK:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_SDRAM_CLOCK(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_DMA_DIRECTION:	//	retrun	int
			_ret = LLDIf.TSPL_SET_DMA_DIRECTION(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_RESET_IP_CORE:	//	retrun	int
			_ret = LLDIf.TSPL_RESET_IP_CORE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_MH_MODE:	//	retrun	int
			_ret = LLDIf.TSPL_SET_MH_MODE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_MH_PID: //	retrun	int
			_ret = LLDIf.TSPL_SET_MH_PID(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_SYMBOL_CLOCK:	//	retrun	int
			_ret = LLDIf.TSPL_SET_SYMBOL_CLOCK(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_CHECK_LN:	//	retrun	int
			_ret = LLDIf.TSPL_CHECK_LN((char *)__req_ctl_cnxt._arg_int1);
			break;
		case	MSG_tvbxxx_TSPL_SEL_DDSCLOCK_INC_594:	//	retrun	int
			_ret = LLDIf.TSPL_SEL_DDSCLOCK_INC_594(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2);
			break;
		case	MSG_tvbxxx_TSPL_SET_PLAY_RATE_594:	//	retrun	int
			_ret = LLDIf.TSPL_SET_PLAY_RATE_594(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2, __req_ctl_cnxt._arg_int3);
			break;
		//2012/4/12 SINGLE TONE
		case	MSG_tvbxxx_TVB380_SET_MODULATOR_SINGLE_TONE:	//	retrun	int
			_ret = LLDIf.TVB380_SET_MODULATOR_SINGLE_TONE(__req_ctl_cnxt._arg_int1, __req_ctl_cnxt._arg_int2, __req_ctl_cnxt._arg_int3);
			break;

		default:
			LLDIf.LldPrint_2("[Cm] undefined cmd...SetCtl",__req_ctl_cnxt._cmd,0);
			break;
		}
		__req_ctl_cnxt._ret_val = _ret;

//		__req_ctl_cnxt._cmd = 0;
		__req_ctl_cnxt._request = TLV_NO;
	}
}

//////////////////////////////////////////////////////////////////////////////////
void	CCtlMgr::MainCtlManager (void *userPara)
{
	char			CrecvMsg[sizeof(_TVB_MSG_FORM)];
	_TVB_MSG_FORM	*msgPtr;
	int			ret_msg;

//	printf("[mgr] : +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ lauch : [%d][%d]\n", sizeof(_TVB_MSG_FORM), sizeof(_MGR_CNTL_REQ_STS));
	msgPtr = (_TVB_MSG_FORM  *)CrecvMsg;
	while(TLV_YES)
	{
		ret_msg = Tmsg->Rcv(__MSG_CORE__ + msg_index, (int *)CrecvMsg, sizeof(_TVB_MSG_FORM));
//////////////////////////////////////////////////////////////////////////////////
		if (ret_msg == __MSG_TIMEOUT__)	// 100-msec
		{
			PollWTimeout();
			continue;
		}

//////////////////////////////////////////////////////////////////////////////////
		switch (msgPtr->msg_key)
		{
//////////////////////////////////////////////////////////////////////////////////	internal msg
		case	MSG_INIT_TVB_MGR_1st:
			InitOnManager1st();
			break;

//////////////////////////////////////////////////////////////////////////////////	external cmd
		case	MSG_tvbxxx_STS_Get:
			Prt_RcvedMsg((unsigned int)__req_sts_cnxt._cmd);
			ExecMgrCntl_HwStsApi();
			break;
		case	MSG_tvbxxx_CTL_Set:
			Prt_RcvedMsg((unsigned int)__req_ctl_cnxt._cmd);
			ExecMgrCntl_HwCtlApi();
			break;

//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
		default:
			printf("[Cm] unknown msg-key [0x%x]\n", (int)msgPtr->msg_key);
			Tmsg->MsgFlush(0);	// clear queue
			break;
		}
	}

	_exit (0);
}
//////////////////////////////////////////////////////////////////////////////////
void	CCtlMgr::_tvbxxx_msg_snd(unsigned long tvb_msg_key,
		long _arg_int1,
		long _arg_int2,
		long _arg_int3,
		long _arg_int4,
		long _arg_int5,
		long _arg_int6,
		long _arg_int7,
		long _arg_int8,
		long _arg_int9,
		double _arg_dbl)
{
	char				SndMsg[sizeof(_TVB_MSG_FORM)];
	_TVB_MSG_FORM		*MsgPtr;
	unsigned long	*vir_tmp;
	double			*dbl_tmp;
//	printf("[mgr] : MSG-SND : [%d].[%d][%d]\n", tvb_msg_key, __req_sts_cnxt._cmd, __req_ctl_cnxt._cmd);

	MsgPtr = (_TVB_MSG_FORM *)SndMsg;
	MsgPtr->msg_key = tvb_msg_key;

#if	0
	vir_tmp = (unsigned long *)MsgPtr->msg_ptr;

	*vir_tmp++ = (unsigned long)_arg_int1;
	*vir_tmp++ = (unsigned long)_arg_int2;
	*vir_tmp++ = (unsigned long)_arg_int3;
	*vir_tmp++ = (unsigned long)_arg_int4;
	*vir_tmp++ = (unsigned long)_arg_int5;
	*vir_tmp++ = (unsigned long)_arg_int6;
	*vir_tmp++ = (unsigned long)_arg_int7;
	*vir_tmp++ = (unsigned long)_arg_int8;
	*vir_tmp++ = (unsigned long)_arg_int9;
	dbl_tmp = (double *)vir_tmp;
	*dbl_tmp = _arg_dbl;
#endif
	Tmsg->Snd (__MSG_CORE__ + msg_index, (int *)SndMsg, sizeof(_TVB_MSG_FORM));
}

//////////////////////////////////////////////////////////////////////////////////
void	*CCtlMgr::EntryCtlManager (void *context)
{
	CCtlMgr		*ptr_mgr;

	ptr_mgr = (CCtlMgr *)context;
	ptr_mgr->MainCtlManager(NULL);
}

//////////////////////////////////////////////////////////////////////////////////
CCtlMgr::CCtlMgr(void)
{
	pthread_t	_tid;
	int _policy;
	struct sched_param _sche_param;


	pause_system_polling = TLV_NO;

	pthread_create(&_tid, NULL, EntryCtlManager, (void *)this);
	if(pthread_getschedparam(_tid, &_policy, &_sche_param) == 0)
	{
		_policy = SCHED_FIFO;
		_sche_param.sched_priority += 10;
		pthread_setschedparam(_tid, _policy, &_sche_param);
	}
}

CCtlMgr::~CCtlMgr()
{
}

//////////////////////////////////////////////////////////////////////////////////
//2011/4/12
//int	_tvbxxx_create_ctl_mgr (void)
int	_tvbxxx_create_ctl_mgr (int nSlot)
{
//	printf("[mgr] : +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ create\n");
	if (_CtlMgr != NULL)
	{
		return	TLV_OK;
	}
	//2011/4/12
	msg_index = nSlot;
	Tmsg = new	CMsg(__MSG_CORE__ + msg_index, sizeof(_TVB_MSG_FORM));
	_CtlMgr = new CCtlMgr();
	_CtlMgr->_tvbxxx_msg_snd(MSG_INIT_TVB_MGR_1st, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);

	return	TLV_OK;
}
//////////////////////////////////////////////////////////////////////////////////
int	_tvbxxx_msg_snd_get_sts(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	return	_CtlMgr->ReqMgrCntl_HwStsApi(_key, _arg_int1, _arg_int2, _arg_int3,
		_arg_int4, _arg_int5, _arg_int6, _arg_int7, _arg_int8, _arg_int9,
		_DFLT_CMD_TIMEOUT__);
}
void	*_tvbxxx_msg_snd_get_sts_ptr(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	return	_CtlMgr->ReqMgrCntl_HwStsApiPtr(_key, _arg_int1, _arg_int2, _arg_int3,
		_arg_int4, _arg_int5, _arg_int6, _arg_int7, _arg_int8, _arg_int9,
		_DFLT_CMD_TIMEOUT__);
}
double	_tvbxxx_msg_snd_get_sts_double(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec)
{
	return	_CtlMgr->ReqMgrCntl_HwStsApiDouble(_key, _arg_int1, _arg_int2, _arg_int3,
		_arg_int4, _arg_int5, _arg_int6, _arg_int7, _arg_int8, _arg_int9,
		_DFLT_CMD_TIMEOUT__);
}
int	_tvbxxx_msg_snd_set_ctl(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	double _arg_dbl)
{
	return	_CtlMgr->ReqMgrCntl_HwCtlApi(_key, _arg_int1, _arg_int2, _arg_int3,
		_arg_int4, _arg_int5, _arg_int6, _arg_int7, _arg_int8, _arg_int9, _arg_dbl,
		_DFLT_CMD_TIMEOUT__);
}
#endif

