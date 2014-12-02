//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_PLAY__CAPT_H_
#define	__TLV_PLAY__CAPT_H_

#include 	"bd_init.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CLldPlayCapt	:	public	CBdInit
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

	int	TL_nLIFormat;//FORMAT_5592==0, FORMAT_5376==1

public:
	CLldPlayCapt(void);
	~CLldPlayCapt();

	int	SetSDRAMBankConfig(int nBankConfig);
	int	SetSDRAMBankOffsetConfig(int nBankConfig);
	int	_stdcall TSPL_SET_SDRAM_BANK_INFO(int nBankNumber, int nBankOffset);
	int	_stdcall TSPL_SET_SDRAM_BANK_CONFIG(int nBankConfig);
	int	_stdcall TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig);
	int 	_stdcall    TSPL_SET_TSIO_DIRECTION(int nDirection);
	int	_stdcall TSPL_GET_TSIO_STATUS(int option);
	int	_stdcall TSPL_GET_TSIO_DIRECTION(void);
	int	_stdcall TSPL_GET_CUR_BANK_GROUP(void);
	int 	_stdcall    TSPL_SET_BANK_COUNTER(int nBank);
	int 	_stdcall    TSPL_SET_SDCON_MODE(int nMode);
//	int 	_stdcall TSPL_RESET_SDCON(void);
	int	_stdcall TSE110_GET_SYNC_STATUS(int mode);
	int	_stdcall TSE110_GET_SIGNAL_STATUS(int port);
	int	_stdcall TSE110_GET_SYNC_FORMAT(void);
	int	_stdcall TSE110_SET_CONFIG(int port_a_mode,
															 int port_b_mode,
															 int tx_clock,
															 int input_port,
															 int output_a_mode,
															 int output_b_mode);
	int	_stdcall TSPL_GET_SYNC_POSITION(int mode, int type, unsigned char *szBuf, 
																  int nlen, int nlen_srch, int nlen_step);
	int	_stdcall TSPL_DO_DEINTERLEAVING(unsigned char *szSrc, unsigned char *szDest);
	int	_stdcall TSPL_DO_RS_DECODING(unsigned char *szSrc, 
															   unsigned char *szDest, 
															   int *format,
															   int *err_blk_cnt,
															   int *recovered_err_cnt,
															   int bypass);
	int	_stdcall TSPL_CONVERT_TO_NI(unsigned char  *szSrc, unsigned char *szDest, int format);
	int	_stdcall TVB380_SET_MODULATOR_INIT_CIF(void);
	int	_stdcall TVB380_SET_MODULATOR_RUN_CIF(unsigned char  *szSrc, unsigned char *szDest);
	int _stdcall TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate);



};

#ifdef __cplusplus
}
#endif

extern	CLldPlayCapt	LLDIf;


#endif

