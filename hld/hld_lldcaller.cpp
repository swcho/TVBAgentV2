
#if defined(WIN32)
#include	<Windows.h>
#else
#define	_FILE_OFFSET_BITS 64
#endif

#include	<stdio.h>
#include	<math.h>
#include	"hld_lldcaller.h"

//#include	"dvbt2_input_processing.h"
#ifdef WIN32
#include	"DVBT2_MultiplePLP.h"
#else
#include	"dvbt2_multipleplp.h"
#include	"../include/lld_const.h"
#endif

#define I2CLOG(a) 
#define I2CLOG1(a,b)
#define I2CLOG2(a,b,c) 
#define I2CLOG_BLOCK(a,b,c)

extern "C" _declspec(dllexport) int _stdcall	TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType);

#define GET_PROC(a,b,c)			{ b = (a)GetProcAddress(m_hInstance, c); }

CALock::CALock(CLldCriticalSection *critSec)
{
	m_pCritSec = critSec;
	m_pCritSec->Lock();
}

CALock::~CALock()
{
	m_pCritSec->Unlock();
}

CLldCriticalSection::CLldCriticalSection()
{
#if defined(WIN32)
	InitializeCriticalSection (&m_CritSec);
#else
#endif
}
CLldCriticalSection::~CLldCriticalSection()
{
#if defined(WIN32)
	DeleteCriticalSection (&m_CritSec);
#else
#endif
}

VOID CLldCriticalSection::Lock()
{
#if defined(WIN32)
	EnterCriticalSection (&m_CritSec);
#else
#endif
}
VOID CLldCriticalSection::Unlock()
{
#if defined(WIN32)
	LeaveCriticalSection (&m_CritSec);
#else
#endif
}


Hlldcaller::Hlldcaller()
{
	m_hInstance = NULL;

	cur_max_playrate_sta = 0;

	TL_nIdCurBank = 0;
	TL_nSubBankIdx = 0;
	TL_dwAddrDestBoardSDRAM = 0;

#ifdef TEST_LOG
	fptr = fopen("D:\\work\\030_DVB_T2\\05_program\\logfile\\t2mi_log11.txt","w");   
#endif
	ReleaseDLL();
}

Hlldcaller::~Hlldcaller()
{
	ReleaseDLL();
#ifdef TEST_LOG
   fclose(fptr);
#endif

}

int	Hlldcaller::SetDmaPtr_HostSide_Lldallocated(void)
{
	TL_pdwDMABuffer = (DWORD *) TSPL_GET_DMA_ADDR();
	if ( TL_pdwDMABuffer == NULL )
	{
#if defined(WIN32)		
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to get DMA address 0");
		return 0;
#else
		if ( __Sta_->IsAttachedTvbTyp_Usb() )	//	why???
		{
			return 0;
		}
#endif
	}
	//__CtlLog_->HldPrint_1("Hld-Bd-Ctl. DMA ADDRESS", (unsigned int)TL_pdwDMABuffer);

	return	1;
}
int	Hlldcaller::SetDmaPtr_HostSide(void)
{
	if ( TL_pdwDMABuffer == NULL )	//	TSPL_GET_DMA_ADDR();
	{
		return	0;
	}
	return	1;
}
DWORD	*Hlldcaller::DmaPtr_HostSide(void)
{
	return	TL_pdwDMABuffer;
}

BOOL Hlldcaller::InitialDLL(LPCTSTR str)
{
	ReleaseDLL();
	m_hInstance = LoadLibrary(str);
	if(m_hInstance == NULL)
	{
		//char msg[32];
		//wsprintf(msg, "Can not find %s",str);
		//MessageBox(NULL,str, "ERROR", MB_OK);
#if defined(WIN32)
#else
		printf("fail-to-load-lld-lib : [%s]...[%s]\n", (str != NULL) ? str : "null", dlerror());
#endif
		return FALSE;
	}

	GET_PROC(FTYPE_INT_LONG,					tspl_get_ad9775, "TSPL_GET_AD9775");
	GET_PROC(FTYPE_INT_LONG_LONG,				tspl_set_ad9775, "TSPL_SET_AD9775");
	GET_PROC(FTYPE_INT_LONG,					tspl_get_ad9787, "TSPL_GET_AD9787");
	GET_PROC(FTYPE_INT_LONG_LONG,				tspl_set_ad9787, "TSPL_SET_AD9787");
	GET_PROC(FTYPE_PVOID_LONG,					tspl_read_block,	"TSPL_READ_BLOCK");
#if defined(WIN32)
	GET_PROC(FTYPE_INT_DWORD_PDWORD,			tspl_write_block,	"TSPL_WRITE_BLOCK");
#else
	GET_PROC(FTYPE_INT_PDWORD_ULONG_PDWORD,		tspl_write_block,	"TSPL_WRITE_BLOCK");
#endif
	GET_PROC(FTYPE_INT_INT_INT,					tspl_set_sdram_bank_info, "TSPL_SET_SDRAM_BANK_INFO");
	GET_PROC(FTYPE_INT_LONG_LONG,				tspl_set_play_rate, "TSPL_SET_PLAY_RATE");
	GET_PROC(FTYPE_INT_LONG,					tspl_sel_ddsclock_inc, "TSPL_SEL_DDSCLOCK_INC");
	GET_PROC(FTYPE_INT_INT,						tspl_set_source,	"TSPL_SET_SOURCE");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_cur_bank_group,"TSPL_GET_CUR_BANK_GROUP");
	GET_PROC(FTYPE_INT_INT,						tspl_set_tsio_direction,"TSPL_SET_TSIO_DIRECTION");
	GET_PROC(FTYPE_INT_INT,					tspl_get_tsio_status,"TSPL_GET_TSIO_STATUS");
	GET_PROC(FTYPE_INT_INT,						tspl_get_dma_reg_info,"TSPL_GET_DMA_REG_INFO");
	GET_PROC(FTYPE_INT_INT_INT,					tspl_set_dma_reg_info,"TSPL_SET_DMA_REG_INFO");
	GET_PROC(FTYPE_INT_INT,						tspl_set_demux_control_test,"TSPL_SET_DEMUX_CONTROL_TEST");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_tsio_direction,"TSPL_GET_TSIO_DIRECTION");
	GET_PROC(FTYPE_INT_INT,						tspl_set_sdcon_mode,	"TSPL_SET_SDCON_MODE");
	GET_PROC(FTYPE_INT_VOID,					tspl_reset_sdcon,		"TSPL_RESET_SDCON");
	GET_PROC(FTYPE_INT_BYTE_PBYTE_INT_PINT,		tspl_i2c_write,			"TSPL_I2C_WRITE");
	GET_PROC(FTYPE_INT_BYTE_PBYTE_INT_PINT,		tspl_i2c_read,			"TSPL_I2C_READ");
	GET_PROC(FTYPE_INT_BYTE_PBYTE_INT_PINT,		tspl_ii2c_write,		"TSPL_II2C_WRITE");
	GET_PROC(FTYPE_INT_BYTE_PBYTE_INT_PINT,		tspl_ii2c_read,			"TSPL_II2C_READ");
	GET_PROC(FTYPE_INT_BYTE_INT_PBYTE_INT_PINT,	tspl_i2c_writeread,		"TSPL_I2C_WRITEREAD");
	GET_PROC(FTYPE_INT_BYTE_INT_PBYTE_INT_PINT,	tspl_ii2c_writeread,		"TSPL_II2C_WRITEREAD");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT,			tspl_set_nim,			"TSPL_SET_NIM");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_authorization, "TSPL_GET_AUTHORIZATION");
	GET_PROC(FTYPE_INT_INT,						tspl_set_sdram_bank_config, "TSPL_SET_SDRAM_BANK_CONFIG");	// yscho0426
	GET_PROC(FTYPE_INT_INT,						tspl_set_sdram_bank_offset_config, "TSPL_SET_SDRAM_BANK_OFFSET_CONFIG");
	GET_PROC(FTYPE_INT_INT_INT_INT_LONG_LONG,	tspl_init_modulator, "TSPL_INIT_MODULATOR");
	GET_PROC(FTYPE_INT_INT,						tspl_set_modulator_coderate, "TSPL_SET_MODULATOR");
	GET_PROC(FTYPE_INT_INT_LONG_LONG,			tspl_set_modulator_freq, "TSPL_SET_MODULATOR_FREQ");
	GET_PROC(FTYPE_INT_INT_LONG_LONG,			tspl_set_modulator_symrate,"TSPL_SET_MODULATOR_SYMRATE");
	GET_PROC(FTYPE_PVOID_VOID,					tspl_get_dma_addr,"TSPL_GET_DMA_ADDR");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_dma_status,"TSPL_GET_DMA_STATUS");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_board_config_status, "TVB380_SET_BOARD_CONFIG_STATUS");
	GET_PROC(FTYPE_INT_INT_INT,					tspl_set_board_led_status, "TSPL_SET_BOARD_LED_STATUS");
	GET_PROC(FTYPE_INT_PVOID,					tspl_reg_event,	"TSPL_REG_EVENT");
	GET_PROC(FTYPE_INT_PVOID,					tspl_reg_complete_event,	"TSPL_REG_COMPLETE_EVENT");
	GET_PROC(FTYPE_INT_VOID,					tspl_set_complete_event,	"TSPL_SET_COMPLETE_EVENT");
	GET_PROC(FTYPE_INT_INT_INT,					tspl_set_fifo_control, "TSPL_SET_FIFO_CONTROL");
	GET_PROC(FTYPE_INT_INT_INT,					tspl_get_fifo_control, "TSPL_GET_FIFO_CONTROL");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_board_config_status, "TSPL_GET_BOARD_CONFIG_STATUS");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_board_id, "TSPL_GET_BOARD_ID");
	GET_PROC(FTYPE_INT_VOID,					tspl_get_board_rev, "TSPL_GET_BOARD_REV");
	GET_PROC(FTYPE_INT_LONG,					tvb380_set_stop_mode, "TVB380_SET_STOP_MODE");
	GET_PROC(FTYPE_INT_LONG,					tvb380_is_enabled_type, "TVB380_IS_ENABLED_TYPE");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_config, "TVB380_SET_CONFIG");
	GET_PROC(FTYPE_INT_LONG_ULONG_LONG,			tvb380_set_modulator_freq, "TVB380_SET_MODULATOR_FREQ");
	GET_PROC(FTYPE_INT_LONG_ULONG_LONG,			tvb380_set_modulator_symrate, "TVB380_SET_MODULATOR_SYMRATE");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_coderate, "TVB380_SET_MODULATOR_CODERATE");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_txmode, "TVB380_SET_MODULATOR_TXMODE");
	GET_PROC(FTYPE_INT_LONG_LONG_ULONG,			tvb380_set_modulator_bandwidth, "TVB380_SET_MODULATOR_BANDWIDTH");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_guardinterval, "TVB380_SET_MODULATOR_GUARDINTERVAL");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_constellation, "TVB380_SET_MODULATOR_CONSTELLATION");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_interleave, "TVB380_SET_MODULATOR_INTERLEAVE");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_if_freq, "TVB380_SET_MODULATOR_IF_FREQ");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_spectrum_inversion, "TVB380_SET_MODULATOR_SPECTRUM_INVERSION");
	GET_PROC(FTYPE_INT_LONG_DOUBLE_LONG,		tvb380_set_modulator_atten_value, "TVB380_SET_MODULATOR_ATTEN_VALUE");		//2011/6/29 added UseTAT4710
	//2012/8/29 new rf level control
	GET_PROC(FTYPE_INT_LONG_DOUBLE_PLONG_LONG,	tvb59x_set_modulator_rf_level_value, "TVB59x_SET_MODULATOR_RF_LEVEL_VALUE");
	GET_PROC(FTYPE_INT_LONG_PDOUBLE_PDOUBLE_LONG,	tvb59x_get_modulator_rf_level_range, "TVB59x_GET_MODULATOR_RF_LEVEL_RANGE");
	//2012/9/6 Pcr Restamping control
	GET_PROC(FTYPE_INT_INT,						tvb59x_set_pcr_stamp_cntl, "TVB59x_SET_PCR_STAMP_CNTL");

	GET_PROC(FTYPE_INT_INT,						tvb59x_set_output_ts_type, "TVB59x_SET_Output_TS_Type");
	GET_PROC(FTYPE_INT_INT,						call_tspl_tvb59x_set_tspacket_cnt_mode, "TSPL_TVB59x_SET_TsPacket_CNT_Mode");
	GET_PROC(FTYPE_INT_PINT_PINT,				call_tspl_tvb59x_get_asi_input_rate, "TSPL_TVB59x_Get_Asi_Input_rate");

	GET_PROC(FTYPE_INT_LONG_LONG_LONG,			tvb59x_modulator_status_control, "TVB59x_Modulator_Status_Control");
	GET_PROC(FTYPE_INT_INT,						tvb59x_get_modulator_status, "TVB59x_Get_Modulator_Status");
	GET_PROC(FTYPE_INT_INT,						tvb59x_set_reset_control_reg, "TVB59x_SET_Reset_Control_REG");

	GET_PROC(FTYPE_INT_LONG_LONG_LONG,			tvb380_set_modulator_agc, "TVB380_SET_MODULATOR_AGC");		//2011/6/29	added UseTAT4710 
	GET_PROC(FTYPE_DOUBLE_LONG_LONG,			tvb380_get_modulator_rf_power_level, "TVB380_GET_MODULATOR_RF_POWER_LEVEL");
	GET_PROC(FTYPE_INT_LONG_LONG_DOUBLE,		tvb380_set_modulator_prbs_info, "TVB380_SET_MODULATOR_PRBS_INFO");
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_prbs_mode, "TVB380_SET_MODULATOR_PRBS_MODE");
	GET_PROC(FTYPE_INT_LONG_DOUBLE,				tvb380_set_modulator_prbs_scale, "TVB380_SET_MODULATOR_PRBS_SCALE");
	GET_PROC(FTYPE_INT_VOID,					tvb380_set_modulator_init_cif, "TVB380_SET_MODULATOR_INIT_CIF");
	GET_PROC(FTYPE_INT_PUCHAR_PUCHAR,			tvb380_set_modulator_run_cif, "TVB380_SET_MODULATOR_RUN_CIF");
	GET_PROC(FTYPE_INT_INT,					tvbxxx_detect_bd,	"TVBxxx_DETECT_BD");
	GET_PROC(FTYPE_INT_INT_PVOID,					tvbxxx_init_bd,	"TVBxxx_INIT_BD");
	GET_PROC(FTYPE_INT_PVOID,					tvbxxx_get_cnxt_all_bd,	"TVBxxx_GET_CNXT_ALL_BD");
	GET_PROC(FTYPE_INT_PVOID,					tvbxxx_get_cnxt_mine,	"TVBxxx_GET_CNXT_MINE");
	GET_PROC(FTYPE_INT_INT_INT_PVOID,					tvbxxx_dup_bd,	"TVBxxx_DUP_BD");

	GET_PROC(FTYPE_INT_INT_INT,					tvb380_open,	"TVB380_open");
	GET_PROC(FTYPE_INT_VOID,					tvb380_close,	"TVB380_close");
	GET_PROC(FTYPE_INT_INT_PCHAR,				tspl_get_encrypted_sn,	"TSPL_GET_ENCRYPTED_SN");
	GET_PROC(FTYPE_INT_PCHAR,					tspl_check_ln,	"TSPL_CHECK_LN");
	GET_PROC(FTYPE_INT_VOID,					tspl_cnt_multi_vsb_rfout,	"TSPL_CNT_MULTI_VSB_RFOUT");
	GET_PROC(FTYPE_INT_VOID,					tspl_cnt_multi_qam_rfout,	"TSPL_CNT_MULTI_QAM_RFOUT");
	//2012/6/28 multi dvb-t
	GET_PROC(FTYPE_INT_VOID,					tspl_cnt_multi_dvbt_rfout,	"TSPL_CNT_MULTI_DVBT_RFOUT");

	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT_INT,	tse110_set_config, "TSE110_SET_CONFIG");
	GET_PROC(FTYPE_INT_INT,						tse110_get_sync_status,	"TSE110_GET_SYNC_STATUS");
	GET_PROC(FTYPE_INT_INT,						tse110_get_signal_status,	"TSE110_GET_SIGNAL_STATUS");
	GET_PROC(FTYPE_INT_VOID,					tse110_get_sync_format,	"TSE110_GET_SYNC_FORMAT");
	GET_PROC(FTYPE_INT_INT_INT_PUCHAR_INT_INT_INT,	tspl_get_sync_position, "TSPL_GET_SYNC_POSITION");
	GET_PROC(FTYPE_INT_PUCHAR_PUCHAR,				tspl_do_deinterleaving, "TSPL_DO_DEINTERLEAVING");
	GET_PROC(FTYPE_INT_PUCHAR_PUCHAR_PINT_PINT_PINT_INT, tspl_do_rs_decoding, "TSPL_DO_RS_DECODING");
	GET_PROC(FTYPE_INT_PUCHAR_PUCHAR_INT,			tspl_convert_to_ni, "TSPL_CONVERT_TO_NI");

	GET_PROC(FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG, tvb380_set_modulator_dvbh, "TVB380_SET_MODULATOR_DVBH");
	GET_PROC(FTYPE_INT_VOID, tspl_get_last_error, "TSPL_GET_LAST_ERROR");
	GET_PROC(FTYPE_INT_LONG_LONG, tvb380_set_modulator_pilot, "TVB380_SET_MODULATOR_PILOT");
	GET_PROC(FTYPE_ULONG_INT_ULONG,						tspl_read_control_reg,"TSPL_READ_CONTROL_REG");
	GET_PROC(FTYPE_INT_INT_DWORD_DWORD,					tspl_write_control_reg,"TSPL_WRITE_CONTROL_REG");
	GET_PROC(FTYPE_INT_LONG_LONG, tvb380_set_modulator_roll_off_factor, "TVB380_SET_MODULATOR_ROLL_OFF_FACTOR");
	GET_PROC(FTYPE_INT_LONG_LONG, tvb380_set_modulator_bert_measure, "TVB380_SET_MODULATOR_BERT_MEASURE");
	GET_PROC(FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG, tvb380_set_modulator_dtmb, "TVB380_SET_MODULATOR_DTMB");
	GET_PROC(FTYPE_INT_LONG_LONG, tvb380_set_modulator_sdram_clock, "TVB380_SET_MODULATOR_SDRAM_CLOCK");

	//TSP104
	GET_PROC(FTYPE_INT_INT,		tsp104_init,	"TSP104_init");
	GET_PROC(FTYPE_INT_VOID,	tsp104_open,	"TSP104_open");
	GET_PROC(FTYPE_INT_VOID,	tsp104_close,	"TSP104_close");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT,		tsp104_reset_nim,			"TSP104_RESET_NIM");
	GET_PROC(FTYPE_INT_INT,						tsp104_select_nim_type,	"TSP104_SELECT_NIM_TYPE");
	GET_PROC(FTYPE_INT_LONG,		tsp104_tvb_tune,			"TSP104_TVB_TUNE");
	GET_PROC(FTYPE_DOUBLE_VOID,		tsp104_tvb_get_snr,			"TSP104_TVB_GET_SNR");
	GET_PROC(FTYPE_INT_INT,			tsp104_tvb_set_demode_conf, "TSP104_TVB_SET_DEMODE_CONF");
	GET_PROC(FTYPE_UCHAR_INT,		tsp104_tvb_get_const_data,	"TSP104_TVB_GET_CONST_DATA");
	GET_PROC(FTYPE_INT_LONG_LONG,	tsp104_tom_tune,			"TSP104_TOM_TUNE");
	GET_PROC(FTYPE_DOUBLE_VOID,		tsp104_tom_get_snr,			"TSP104_TOM_GET_SNR");
	GET_PROC(FTYPE_DOUBLE_VOID,		tsp104_tom_get_ber,			"TSP104_TOM_GET_BER");
	GET_PROC(FTYPE_INT_VOID,		tsp104_tom_get_agc,			"TSP104_TOM_GET_AGC_STATUS");
	GET_PROC(FTYPE_INT_VOID,		tsp104_tom_get_tps,			"TSP104_TOM_GET_TPS_STATUS");
	GET_PROC(FTYPE_INT_VOID,		tsp104_tom_get_fec,			"TSP104_TOM_GET_FEC_STATUS");
	//ISDB-T
	GET_PROC(FTYPE_INT_LONG,		tsp104_tdb_tune,			"TSP104_TDB_TUNE");
	GET_PROC(FTYPE_DOUBLE_VOID,		tsp104_tdb_get_snr,			"TSP104_TDB_GET_SNR");
	GET_PROC(FTYPE_DOUBLE_VOID,		tsp104_tdb_get_ber,			"TSP104_TDB_GET_BER");
	GET_PROC(FTYPE_INT_VOID,		tsp104_tdb_get_lock,		"TSP104_TDB_GET_LOCK_STATUS");
	GET_PROC(FTYPE_INT_LONG_LONG,	tsp104_tdb_tune_wr_rd,		"TSP104_TDB_TUNE_WR_RD");

	//TSE110
	GET_PROC(FTYPE_INT_VOID,	tse110_open,	"TSE110_open");
	GET_PROC(FTYPE_INT_VOID,	tse110_close,	"TSE110_close");

#if	0
	//TMCC REMUXER
	GET_PROC(FTYPE_INT_VOID,	tmcc_stop_remux, "TMCC_STOP_REMUX");
	GET_PROC(FTYPE_INT_VOID,	tmcc_start_remux, "TMCC_START_REMUX");
	GET_PROC(FTYPE_INT_VOID,	tmcc_close_remux_source, "TMCC_CLOSE_REMUX_SOURCE");
	GET_PROC(FTYPE_INT_PCHAR,	tmcc_open_remux_source, "TMCC_OPEN_REMUX_SOURCE");
	GET_PROC(FTYPE_PCHAR_INT_PINT_PINT_PINT, tmcc_read_remux_data, "TMCC_READ_REMUX_DATA");
	GET_PROC(FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG,	tmcc_set_remux_info, "TMCC_SET_REMUX_INFO");
	GET_PROC(FTYPE_INT_LONG_LONG_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR, tmcc_set_layer_info, "TMCC_SET_LAYER_INFO");
	GET_PROC(FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG, tmcc_set_remux_source_position, "TMCC_SET_REMUX_SOURCE_POSITION");
	GET_PROC(FTYPE_LONG_LONG_PLONG, tmcc_get_remux_source_position, "TMCC_GET_REMUX_SOURCE_POSITION");
	GET_PROC(FTYPE_INT_PUCHAR_INT, tmcc_write_remux_data, "TMCC_WRITE_REMUX_DATA");
#endif

	//I/Q PLAY/CAPTURE
	GET_PROC(FTYPE_INT_INT, tspl_get_fpga_info, "TSPL_GET_FPGA_INFO");

	//TVB593
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_set_dma_direction, "TSPL_SET_DMA_DIRECTION");
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_reset_ip_core,		"TSPL_RESET_IP_CORE");
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_set_mh_mode,		"TSPL_SET_MH_MODE");
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_set_mh_pid,		"TSPL_SET_MH_PID");
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_set_symbol_clock,	"TSPL_SET_SYMBOL_CLOCK");
	GET_PROC(FTYPE_INT_LONG_LONG,	tspl_set_max_playrate,	"TSPL_SET_MAX_PLAYRATE");

	//	DVB-C2
	GET_PROC(FTYPE_INT_LONG_LONG_LONG,	tspl_set_play_rate_594,	"TSPL_SET_PLAY_RATE_594");

//2011/5/4 AD9852 MAX clk
#ifdef WIN32
	GET_PROC(FTYPE_INT_LONG,		tspl_set_ad9852_max_clock,	"TSPL_SET_AD9852_MAX_CLOCK");
#endif	
	//2011/11/01 added PAUSE
	GET_PROC(FTYPE_INT_LONG_LONG,				tvb380_set_modulator_output, "TVB380_SET_MODULATOR_OUTPUT");
#if TVB597A_STANDALONE
	GET_PROC(FTYPE_INT_PCHAR,		tvb597f_set_flash,		"TVB597F_SET_FLASH");
#endif

	GET_PROC(FTYPE_INT_INT,	_GET_wr_BUF_STS_MultiBd_n,	"TVB380_GET_wr_BUF_STS_MultiBd_n");
	GET_PROC(FTYPE_INT_VOID,	_GET_wr_cap_BUF_STS_MultiBd_n,	"TVB380_GET_wr_cap_BUF_STS_MultiBd_n");
	GET_PROC(FTYPE_INT_INT,	_SEL_TS_TAG_VirtualBd_n,	"TSPL_SEL_TS_TAG_VirtualBd_n");
	GET_PROC(FTYPE_INT_INT,	__SelMultiModAsiOut_n,	"SelMultiModAsiOut_n");
	GET_PROC(FTYPE_INT_INT,	__SelMultiMod310Out_n,	"SelMultiMod310Out_n");
	GET_PROC(FTYPE_INT_INT,	__SelMultiModTsOutput_n,	"SelMultiModTsOutput_n");
	GET_PROC(FTYPE_INT_INT,	__SelMultiModOperationMode_n,	"SelMultiModOperationMode_n");
	//2012/4/12 SINGLE TONE
	GET_PROC(FTYPE_INT_LONG_ULONG_LONG,			tvb380_set_modulator_single_tone, "TVB380_SET_MODULATOR_SINGLE_TONE");
	return TRUE;
}

void	Hlldcaller::InitFunc(void)
{
	tspl_get_ad9775 = NULL;
	tspl_set_ad9775 = NULL;
	tspl_get_ad9787 = NULL;
	tspl_set_ad9787 = NULL;
	tspl_read_block = NULL;
	tspl_write_block= NULL;
	tspl_set_play_rate= NULL;
	tspl_sel_ddsclock_inc = NULL;
	tspl_set_source= NULL;
	tspl_get_cur_bank_group= NULL;
	tspl_set_tsio_direction= NULL;
	tspl_get_tsio_direction= NULL;
	tspl_get_tsio_status= NULL;
	tspl_get_dma_reg_info= NULL;
	tspl_set_dma_reg_info= NULL;
	tspl_set_demux_control_test= NULL;
	tspl_set_sdcon_mode= NULL;
	tspl_reset_sdcon= NULL;
	tspl_i2c_write= NULL;
	tspl_i2c_read= NULL;
	tspl_ii2c_write= NULL;
	tspl_ii2c_read= NULL;
	tspl_i2c_writeread= NULL;
	tspl_ii2c_writeread= NULL;
	tspl_get_authorization = NULL;
	tspl_set_sdram_bank_config = NULL;
	tspl_set_sdram_bank_offset_config = NULL;
	tspl_init_modulator = NULL;
	tspl_set_modulator_coderate = NULL;
	tspl_set_modulator_freq = NULL;
	tspl_set_modulator_symrate = NULL;
	tspl_get_dma_addr = NULL;
	tspl_get_dma_status = NULL;
	tspl_set_sdram_bank_info = NULL;
	tvb380_set_board_config_status = NULL;
	tspl_set_board_led_status = NULL;
	tspl_reg_event = NULL;
	tspl_reg_complete_event = NULL;
	tspl_set_complete_event = NULL;
	tspl_set_fifo_control = NULL;
	tspl_get_fifo_control = NULL;
	tspl_get_board_config_status = NULL;
	tspl_get_board_id = NULL;
	tspl_get_board_rev = NULL;
	tvb380_set_stop_mode = NULL;
	tvb380_is_enabled_type = NULL;
	tvb380_set_config = NULL;
	tvb380_set_modulator_freq = NULL;
	tvb380_set_modulator_symrate = NULL;
	tvb380_set_modulator_coderate = NULL;
	tvb380_set_modulator_txmode = NULL;
	tvb380_set_modulator_bandwidth = NULL;
	tvb380_set_modulator_guardinterval = NULL;
	//2011/11/01 added PAUSE
	tvb380_set_modulator_output = NULL;
	tvb380_set_modulator_constellation = NULL;
	tvb380_set_modulator_interleave = NULL;
	tvb380_set_modulator_if_freq = NULL;
	tvb380_set_modulator_spectrum_inversion = NULL;
	tvb380_set_modulator_atten_value = NULL;
	//2012/8/29 new rf level control
	tvb59x_set_modulator_rf_level_value = NULL;
	tvb59x_get_modulator_rf_level_range = NULL;
	//2012/9/6 Pcr Restamping control
	tvb59x_set_pcr_stamp_cntl = NULL;
	tvb59x_set_output_ts_type = NULL;
	tvb59x_set_reset_control_reg = NULL;
	call_tspl_tvb59x_set_tspacket_cnt_mode = NULL;
	call_tspl_tvb59x_get_asi_input_rate = NULL;

	tvb59x_modulator_status_control = NULL;
	tvb59x_get_modulator_status = NULL;

	tvb380_set_modulator_agc = NULL;
	tvb380_get_modulator_rf_power_level = NULL;
	tvb380_set_modulator_prbs_info = NULL;
	tvb380_set_modulator_prbs_mode = NULL;
	tvb380_set_modulator_prbs_scale = NULL;
	tvb380_set_modulator_init_cif = NULL;
	tvb380_set_modulator_run_cif = NULL;

	tvbxxx_detect_bd = NULL;
	tvbxxx_init_bd = NULL;
	tvbxxx_get_cnxt_all_bd = NULL;
	tvbxxx_get_cnxt_mine = NULL;
	tvbxxx_dup_bd = NULL;

	tvb380_open = NULL;
//	tvb380_close = NULL;
	tspl_get_encrypted_sn = NULL;
	tspl_check_ln = NULL;
	tspl_cnt_multi_vsb_rfout = NULL;
	tspl_cnt_multi_qam_rfout = NULL;
	//2012/6/28 multi dvb-t
	tspl_cnt_multi_dvbt_rfout = NULL;

	tse110_set_config = NULL;
	tse110_get_sync_status = NULL;
	tse110_get_signal_status = NULL;
	tse110_get_sync_format = NULL;

	tspl_get_sync_position = NULL;
	tspl_do_deinterleaving = NULL;
	tspl_do_rs_decoding = NULL;
	tspl_convert_to_ni = NULL;

	tvb380_set_modulator_dvbh = NULL;
	tspl_get_last_error = NULL;
	tvb380_set_modulator_pilot = NULL;
	tspl_read_control_reg = NULL;
	tspl_write_control_reg = NULL;
	tvb380_set_modulator_roll_off_factor = NULL;
	tvb380_set_modulator_bert_measure = NULL;
	tvb380_set_modulator_dtmb = NULL;
	tvb380_set_modulator_sdram_clock = NULL;

	//TSP104
	tsp104_init = NULL;
	tsp104_open = NULL;
	tsp104_close = NULL;
	tsp104_reset_nim = NULL;
	tsp104_select_nim_type = NULL;
	tsp104_tvb_tune = NULL;
	tsp104_tvb_get_snr = NULL;
	tsp104_tvb_set_demode_conf = NULL;
	tsp104_tvb_get_const_data = NULL;
	tsp104_tom_tune = NULL;
	tsp104_tom_get_snr = NULL;
	tsp104_tom_get_ber = NULL;
	tsp104_tom_get_agc = NULL;
	tsp104_tom_get_tps = NULL;
	tsp104_tom_get_fec = NULL;

	//ISDB-T
	tsp104_tdb_tune = NULL;
	tsp104_tdb_get_snr = NULL;
	tsp104_tdb_get_ber = NULL;
	tsp104_tdb_get_lock = NULL;
	tsp104_tdb_tune_wr_rd = NULL;

	//TSE110
	tse110_open = NULL;
	tse110_close = NULL;

	//TMCC REMUXER
	tmcc_stop_remux = NULL;
	tmcc_start_remux = NULL;
	tmcc_close_remux_source = NULL;
	tmcc_open_remux_source = NULL;
	tmcc_open_remux_source = NULL;
	tmcc_set_remux_info = NULL;
	tmcc_set_layer_info = NULL;
	tmcc_set_remux_source_position = NULL;
	tmcc_get_remux_source_position = NULL;

	//I/Q PLAY/CAPTURE
	tspl_get_fpga_info = NULL;

	//TVB593
	tspl_set_dma_direction =  NULL;
	tspl_reset_ip_core =  NULL;
	tspl_set_mh_mode =  NULL;
	tspl_set_mh_pid =  NULL;
	tspl_set_symbol_clock =  NULL;
	tspl_set_max_playrate =  NULL;

	//	DVB-C2
	tspl_set_play_rate_594 = NULL;
	
//2011/5/4 AD9852 MAX clk
#ifdef WIN32
	tspl_set_ad9852_max_clock = NULL;
#endif
#if TVB597A_STANDALONE
	tvb597f_set_flash = NULL;
#endif

	_GET_wr_BUF_STS_MultiBd_n = NULL;
	_GET_wr_cap_BUF_STS_MultiBd_n = NULL;
	_SEL_TS_TAG_VirtualBd_n = NULL;
	__SelMultiModAsiOut_n = NULL;
	__SelMultiMod310Out_n = NULL;
	__SelMultiModTsOutput_n = NULL;
	__SelMultiModOperationMode_n = NULL;

	//2012/4/12 SINGLE TONE
	tvb380_set_modulator_single_tone = NULL;

}
VOID	Hlldcaller::ReleaseDLL()
{
	InitFunc();

#ifdef WIN32
	if(m_hInstance != NULL)
	{
		if (__Sta_->m_nBoardId > 0)
		{
			tvb380_close();
		}
	}
#endif
	tvb380_close = NULL;

	if(m_hInstance != NULL)
	{
		FreeLibrary(m_hInstance);
		m_hInstance = NULL;
	}
}

//TVB590S
INT Hlldcaller::TSPL_GET_AD9775(long reg)
{
	if(tspl_get_ad9775)
		return tspl_get_ad9775(reg);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_AD9775(long reg, long data)
{
	if(tspl_set_ad9775)
		return tspl_set_ad9775(reg, data);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_GET_AD9787(long reg)
{
	if(tspl_get_ad9787)
		return tspl_get_ad9787(reg);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_AD9787(long reg, long data)
{
	if(tspl_set_ad9787)
		return tspl_set_ad9787(reg, data);
	return TSPL_NO_INTERFACE;
}

VOID *Hlldcaller::TSPL_READ_BLOCK(long dwBlockSize)
{
	if(tspl_read_block)
		return tspl_read_block(dwBlockSize);
	return NULL;
}

int Hlldcaller::TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, unsigned long dwBuffSize,DWORD* dest)
{
#if defined(WIN32)
	if(tspl_write_block)
		tspl_write_block(dwBuffSize, dest);
#else
	if(tspl_write_block)
		tspl_write_block(pdwSrcBuff, dwBuffSize, dest);
#endif
	return dwBuffSize;
}

INT Hlldcaller::TSPL_SET_PLAY_RATE(long play_freq, long nOutputClockSource)
{
	if(tspl_set_play_rate)
		return tspl_set_play_rate(play_freq, nOutputClockSource);
	return TSPL_NO_INTERFACE;
}

//USING DDS INC./DEC
INT Hlldcaller::TSPL_SEL_DDSCLOCK_INC(long play_freq)
{
	if(tspl_sel_ddsclock_inc)
		return tspl_sel_ddsclock_inc(play_freq);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_SOURCE(int nNewTS_source)
{
	if(tspl_set_source)
		return tspl_set_source(nNewTS_source);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_GET_CUR_BANK_GROUP(void)
	//	get bank index. the bank is the one seperated partial area of total sdram.
	//	refer to TSPL_SET_SDRAM_BANK_INFO(). TSPL_SET_SDRAM_BANK_CONFIG(). TSPL_SET_SDRAM_BANK_OFFSET_CONFIG()
{
	if(tspl_get_cur_bank_group)
		return tspl_get_cur_bank_group();
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_TSIO_DIRECTION(int nDirection)
{
	if(tspl_set_tsio_direction)
		return tspl_set_tsio_direction(nDirection);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_GET_TSIO_DIRECTION(void)
{
	if(tspl_get_tsio_direction)
		return tspl_get_tsio_direction();
	return TSPL_NO_INTERFACE;
}

//TVB390V8
INT Hlldcaller::TSPL_GET_TSIO_STATUS(int option)
{
	if(tspl_get_tsio_status)
		return tspl_get_tsio_status(option);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_GET_DMA_REG_INFO(int addr)
{
	if(tspl_get_dma_reg_info)
		return tspl_get_dma_reg_info(addr);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_DMA_REG_INFO(int addr, int data)
{
	if(tspl_set_dma_reg_info)
		return tspl_set_dma_reg_info(addr, data);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_DEMUX_CONTROL_TEST(int control)
{
	if(tspl_set_demux_control_test)
		return tspl_set_demux_control_test(control);
	return TSPL_NO_INTERFACE;
}
//+

INT	Hlldcaller::TSPL_SET_SDCON_MODE(int nMode)
{
	if(tspl_set_sdcon_mode)
		return tspl_set_sdcon_mode(nMode);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_RESET_SDCON(void)
{
	if(tspl_reset_sdcon)
		return tspl_reset_sdcon();
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_I2C_WRITE( unsigned char address, unsigned char *tx_buffer, int txlen, int* actlen)
{
	I2CLOG_BLOCK("WRITE",txlen,tx_buffer);
	if(tspl_i2c_write)
		return tspl_i2c_write(address, tx_buffer+1, txlen, actlen);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_I2C_READ( unsigned char address, unsigned char *rx_buffer, int maxlen, int* actlen)
{
	int ret;
	if(tspl_i2c_read)
	{
		ret = tspl_i2c_read(address, rx_buffer+1, maxlen, actlen);
		I2CLOG_BLOCK("READ",*actlen,rx_buffer);
		return ret;
	}
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_I2C_WRITEREAD( unsigned char dev_addr, int reg_addr, unsigned char *data, int count, int *actlen)
{
	int ret;
	if(tspl_i2c_writeread)
	{
		ret = tspl_i2c_writeread(dev_addr, reg_addr, data,count, actlen);
		I2CLOG_BLOCK("WRITEREAD",count,data);
		return ret;
	}
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_II2C_WRITEREAD( unsigned char dev_addr, int reg_addr, unsigned char *data, int count, int *actlen)
{
	int ret;
	if(tspl_ii2c_writeread)
	{
		ret = tspl_ii2c_writeread(dev_addr, reg_addr, data,count, actlen);
		I2CLOG_BLOCK("WRITEREAD",count,data);
		return ret;
	}
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_II2C_WRITE(unsigned char address, unsigned char *tx_buffer,  int txlen, int* actlen)
{
	if(tspl_ii2c_write)
		return tspl_ii2c_write(address, tx_buffer, txlen, actlen);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_II2C_READ(unsigned char address, unsigned char *tx_buffer,  int txlen, int* actlen)
{
	if(tspl_ii2c_read)
		return tspl_ii2c_read(address, tx_buffer, txlen, actlen);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_NIM(int reset,  int lnb_vtg,  int lnb_on_off,  int	lnb_tone)
{
	if(tspl_set_nim)
		return tspl_set_nim(reset, lnb_vtg, lnb_on_off, lnb_tone);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_GET_AUTHORIZATION(void)
{
	if(tspl_get_authorization)
		return tspl_get_authorization();
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_SDRAM_BANK_CONFIG(int config)
{
	if(tspl_set_sdram_bank_config)
		return tspl_set_sdram_bank_config(config);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_SDRAM_BANK_INFO(int nBankNumber, int nBankOffset)
{
	if(tspl_set_sdram_bank_info)
		return tspl_set_sdram_bank_info(nBankNumber, nBankOffset);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig)
{
	if(tspl_set_sdram_bank_offset_config)
		return tspl_set_sdram_bank_offset_config(nBankConfig);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_INIT_MODULATOR(int modulation_mode, int InputSelection, int CodingMode, long dwfreq, long symbol_freq)
{
	if(tspl_init_modulator)
		return tspl_init_modulator(modulation_mode, InputSelection, CodingMode, dwfreq, symbol_freq);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_MODULATOR_CODERATE(int CodingMode)
{	
	if(tspl_set_modulator_coderate)
		return tspl_set_modulator_coderate(CodingMode);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_MODULATOR_FREQ(int modulation_mode, long dwfreq, long symbol_freq)
{
	if(tspl_set_modulator_freq)
		return tspl_set_modulator_freq(modulation_mode, dwfreq, symbol_freq);
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSPL_SET_MODULATOR_SYMRATE(int modulation_mode, long dwfreq, long symbol_freq)
{
	if(tspl_set_modulator_symrate)
		return tspl_set_modulator_symrate(modulation_mode, dwfreq, symbol_freq);
	return TSPL_NO_INTERFACE;
}

VOID* Hlldcaller::TSPL_GET_DMA_ADDR(void)
{
	if(tspl_get_dma_addr)
		return tspl_get_dma_addr();
	return NULL;
}
INT Hlldcaller::TSPL_GET_DMA_STATUS(void)
{
	if(tspl_get_dma_status)
		return tspl_get_dma_status();
	return TSPL_NO_INTERFACE;
}

// TVB595V1
INT		Hlldcaller::TVB380_SET_BOARD_CONFIG_STATUS(long modulator_type, long status)
{
	if (tvb380_set_board_config_status)
		return tvb380_set_board_config_status(modulator_type, status);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_SET_BOARD_LED_STATUS(int status_LED, int fault_LED)
{
	if (tspl_set_board_led_status)
		return tspl_set_board_led_status(status_LED, fault_LED);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_REG_EVENT(void* pvoid)
{
	if (tspl_reg_event)
		return tspl_reg_event(pvoid);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_REG_COMPLETE_EVENT(void* pvoid)
{
	if (tspl_reg_complete_event)
		return tspl_reg_complete_event(pvoid);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_COMPLETE_EVENT(void)
{
	if (tspl_set_complete_event)
		return tspl_set_complete_event();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_SET_FIFO_CONTROL(int dma_direction, int dma_size)
{
	if (tspl_set_fifo_control)
		return tspl_set_fifo_control(dma_direction, dma_size);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_GET_FIFO_CONTROL(int dma_direction, int dma_size)
{
	if (tspl_get_fifo_control)
		return tspl_get_fifo_control(dma_direction, dma_size);
	return TSPL_NO_INTERFACE;
}

// TVB590V9 - RF/IF AMP
INT		Hlldcaller::TSPL_GET_BOARD_CONFIG_STATUS(void)
{
	if (tspl_get_board_config_status)
		return tspl_get_board_config_status();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_GET_BOARD_ID(void)
{
	if (tspl_get_board_id)
		return tspl_get_board_id();
	return TSPL_NO_INTERFACE;
}

//I/Q PLAY/CAPTURE
INT		Hlldcaller::TSPL_GET_FPGA_INFO(int info)
{
	if (tspl_get_fpga_info)
		return tspl_get_fpga_info(info);
	return TSPL_NO_INTERFACE;
}

//TVB590S V2
INT		Hlldcaller::TSPL_GET_BOARD_REV(void)
{
	if (tspl_get_board_rev)
		return tspl_get_board_rev();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_STOP_MODE(
						long stop_mode)
{
	if(tvb380_set_stop_mode)
		return tvb380_set_stop_mode(stop_mode);
	return TSPL_NO_INTERFACE;
}

//sskim20061020
INT		Hlldcaller::TVB380_IS_ENABLED_TYPE(
						long modulator_type)
{
	if (tvb380_is_enabled_type)
		return tvb380_is_enabled_type(modulator_type);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_CONFIG(
						long modulator_type, long IF_Frequency)
{
	if (tvb380_set_config)
		return tvb380_set_config(modulator_type, IF_Frequency);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TVB380_SET_MODULATOR_FREQ(
						long modulator_type,			
						unsigned long output_frequency,//2150MHz support - int -> unsigned long
						long symbol_rate_or_bandwidth)
{
	if (tvb380_set_modulator_freq)
		return tvb380_set_modulator_freq(modulator_type, output_frequency, symbol_rate_or_bandwidth);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_SYMRATE(
						long modulator_type, 
						unsigned long output_frequency,
						long symbol_rate_or_bandwidth)
{
	if (tvb380_set_modulator_symrate)
		return tvb380_set_modulator_symrate(modulator_type, output_frequency, symbol_rate_or_bandwidth);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_CODERATE(
						long modulator_type,
						long code_rate)
{
	if (tvb380_set_modulator_coderate)
		return tvb380_set_modulator_coderate(modulator_type, code_rate);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_TXMODE(
						long modulator_type,
						long tx_mode)
{
	if (tvb380_set_modulator_txmode)
		return tvb380_set_modulator_txmode(modulator_type, tx_mode);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_BANDWIDTH(
						long modulator_type,
						long bandwidth,
						unsigned long output_frequency)//2150MHz support - int -> unsigned long
{
	if (tvb380_set_modulator_bandwidth)
		return tvb380_set_modulator_bandwidth(modulator_type, bandwidth, output_frequency);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_GUARDINTERVAL(
						long modulator_type,
						long guard_interval)
{
	if (tvb380_set_modulator_guardinterval)
		return tvb380_set_modulator_guardinterval(modulator_type, guard_interval);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_OUTPUT(
						long modulator_type,
						long output)
{
	if (tvb380_set_modulator_output)
		return tvb380_set_modulator_output(modulator_type, output);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_CONSTELLATION(
						long modulator_type,
						long constellation)
{
	if (tvb380_set_modulator_constellation)
		return tvb380_set_modulator_constellation(modulator_type, constellation);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_INTERLEAVE(
						long modulator_type,
						long interleaving)
{
	if (tvb380_set_modulator_interleave)
		return tvb380_set_modulator_interleave(modulator_type, interleaving);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_IF_FREQ(
						long modulator_type,
						long IF_frequency)
{
	if (tvb380_set_modulator_if_freq)
		return tvb380_set_modulator_if_freq(modulator_type, IF_frequency);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TVB380_SET_MODULATOR_SPECTRUM_INVERSION(
						long modulator_type,
						long spectral_inversion)
{
	if (tvb380_set_modulator_spectrum_inversion)
		return tvb380_set_modulator_spectrum_inversion(modulator_type, spectral_inversion);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_ATTEN_VALUE(
						long modulator_type,
						double atten_val,
						long UseTAT4710)	//2011/6/29 added UseTAT4710
{
	if (tvb380_set_modulator_atten_value)
		return tvb380_set_modulator_atten_value(modulator_type, atten_val, UseTAT4710);
	return TSPL_NO_INTERFACE;
}
//2012/9/6 Pcr Restamping control
INT		Hlldcaller::TVB59x_SET_PCR_STAMP_CNTL(int _val)
{
	if(tvb59x_set_pcr_stamp_cntl)
		return tvb59x_set_pcr_stamp_cntl(_val);
	return TSPL_NO_INTERFACE;
}
//2013/5/31 TVB599
INT		Hlldcaller::TVB59x_SET_Output_TS_Type(int _val)
{
	if(tvb59x_set_output_ts_type)
		return tvb59x_set_output_ts_type(_val);
	return TSPL_NO_INTERFACE;
}

//2013/6/10 TVB599 Reset Control
INT		Hlldcaller::TVB59x_SET_Reset_Control_REG(int _val)
{
	if(tvb59x_set_reset_control_reg)
		return tvb59x_set_reset_control_reg(_val);
	return TSPL_NO_INTERFACE;
}
//2013/5/3 ASI bitrate
INT		Hlldcaller::Func_TVB59x_SET_TsPacket_CNT_Mode(int _val)
{
	if(call_tspl_tvb59x_set_tspacket_cnt_mode)
		return call_tspl_tvb59x_set_tspacket_cnt_mode(_val);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::Func_TVB59x_Get_Asi_Input_rate(int *delta_packet, int *delta_clock)
{
	if(call_tspl_tvb59x_get_asi_input_rate)
		return call_tspl_tvb59x_get_asi_input_rate(delta_packet, delta_clock);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::Func_TVB59x_Modulator_Status_Control(long modulator, long index, long val)
{
	if(tvb59x_modulator_status_control)
		return tvb59x_modulator_status_control(modulator, index, val);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::Func_TVB59x_Get_Modulator_Status(int _val)
{
	if(tvb59x_get_modulator_status)
		return tvb59x_get_modulator_status(_val);
	return TSPL_NO_INTERFACE;
}

//2012/8/29 new rf level control
INT		Hlldcaller::TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(
						long modulator_type,
						double rf_level_value,
						long *AmpFlag,
						long UseTAT4710)
{
	if (tvb59x_set_modulator_rf_level_value)
		return tvb59x_set_modulator_rf_level_value(modulator_type, rf_level_value, AmpFlag, UseTAT4710);
	return TSPL_NO_INTERFACE;
}

//2012/8/29 new rf level control
INT		Hlldcaller::TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(
						long modulator_type,
						double *rf_level_min,
						double *rf_level_max,
						long UseTAT4710)
{
	if (tvb59x_get_modulator_rf_level_range)
		return tvb59x_get_modulator_rf_level_range(modulator_type, rf_level_min, rf_level_max, UseTAT4710);
	return TSPL_NO_INTERFACE;
}


//2012/4/12 SINGLE TONE
INT		Hlldcaller::TVB380_SET_MODULATOR_SINGLE_TONE(
						long modulator_type,
						unsigned long freq,
						long singleTone)
{
	if (tvb380_set_modulator_single_tone)
		return tvb380_set_modulator_single_tone(modulator_type, freq, singleTone);
	return TSPL_NO_INTERFACE;
}

//AGC
INT		Hlldcaller::TVB380_SET_MODULATOR_AGC(
						long modulator_type,
						long agc_on_off,
						long UseTAT4710)
{
	if (tvb380_set_modulator_agc)
		return tvb380_set_modulator_agc(modulator_type, agc_on_off, UseTAT4710);
	return TSPL_NO_INTERFACE;
}

//sskim20081009 - RF power level
DOUBLE		Hlldcaller::TVB380_GET_MODULATOR_RF_POWER_LEVEL(
						long modulator_type,
						long info_type)
{
	if (tvb380_get_modulator_rf_power_level)
		return tvb380_get_modulator_rf_power_level(modulator_type, info_type);
	return (double)TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_PRBS_INFO(
						long modulator_type,
						long mode,
						double noise_power)
{
	if (tvb380_set_modulator_prbs_info)
		return tvb380_set_modulator_prbs_info(modulator_type, mode, noise_power);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_PRBS_MODE(
						long modulator_type,
						long mode)
{
	if (tvb380_set_modulator_prbs_mode)
		return tvb380_set_modulator_prbs_mode(modulator_type, mode);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_PRBS_SCALE(
						long modulator_type,
						double noise_power)
{
	if (tvb380_set_modulator_prbs_scale)
		return tvb380_set_modulator_prbs_scale(modulator_type, noise_power);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_INIT_CIF(void)
{
	if (tvb380_set_modulator_init_cif)
		return tvb380_set_modulator_init_cif();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_RUN_CIF(unsigned char  *szSrc, unsigned char *szDest)
{
	if (tvb380_set_modulator_run_cif)
		return tvb380_set_modulator_run_cif(szSrc, szDest);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVBxxx_DETECT_BD(int _multi_bd)
{
	if (tvbxxx_detect_bd)
		return tvbxxx_detect_bd(_multi_bd);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TVBxxx_INIT_BD(int _my_num, void *_my_cxt)
{
	if (tvbxxx_init_bd)
		return tvbxxx_init_bd(_my_num, _my_cxt);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TVBxxx_GET_CNXT_ALL_BD(void *basic_inform)
{
	if (tvbxxx_get_cnxt_all_bd)
		return tvbxxx_get_cnxt_all_bd(basic_inform);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TVBxxx_GET_CNXT_MINE(void *_cnxt)
{
	if (tvbxxx_get_cnxt_mine)
		return tvbxxx_get_cnxt_mine(_cnxt);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *_from_cnxt)
{
	if (tvbxxx_dup_bd)
		return tvbxxx_dup_bd(from_slot_num, to_slot_num, _from_cnxt);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_OPEN(int nInitModulatorType, int nInitIF)
{
	if (tvb380_open)
		return tvb380_open(nInitModulatorType, nInitIF);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_CLOSE(void)
{
	if (tvb380_close)
		return tvb380_close();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_GET_ENCRYPTED_SN(int type, char* sn)
{
	if (tspl_get_encrypted_sn)
		return tspl_get_encrypted_sn(type, sn);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_CHECK_LN(char* ln)
{
	if (tspl_check_ln)
		return tspl_check_ln(ln);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_CNT_MULTI_VSB_RFOUT(void)
{
	if (tspl_cnt_multi_vsb_rfout)
		return tspl_cnt_multi_vsb_rfout();
	return TSPL_NO_INTERFACE;
}
//2012/6/28 multi dvb-t
INT 	Hlldcaller::TSPL_CNT_MULTI_DVBT_RFOUT(void)
{
	if (tspl_cnt_multi_dvbt_rfout)
		return tspl_cnt_multi_dvbt_rfout();
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_CNT_MULTI_QAM_RFOUT(void)
{
	if (tspl_cnt_multi_qam_rfout)
		return tspl_cnt_multi_qam_rfout();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_SET_CONFIG(int port_a_mode,
									   int port_b_mode,
									   int tx_clock,
									   int input_port,
									   int output_a_mode,
									   int output_b_mode)
{
	if (tse110_set_config)
		return tse110_set_config(port_a_mode, port_b_mode, 
				tx_clock, input_port, 
				output_a_mode, output_b_mode);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_GET_SYNC_STATUS(int mode)
{
	if (tse110_get_sync_status)
		return tse110_get_sync_status(mode);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_GET_SIGNAL_STATUS(int port)
{
	if (tse110_get_signal_status)
		return tse110_get_signal_status(port);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_GET_SYNC_POSITION(int mode, 
											int type, 
											unsigned char *szBuf, 
											int nlen, 
											int nlen_srch, 
											int nlen_step)
{
	if (tspl_get_sync_position)
		return tspl_get_sync_position(mode, type, szBuf, nlen, nlen_srch, nlen_step);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_GET_SYNC_FORMAT(void)
{
	if (tse110_get_sync_format)
		return tse110_get_sync_format();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_DO_DEINTERLEAVING(unsigned char *szSrc, unsigned char *szDest)
{
	if (tspl_do_deinterleaving)
		return tspl_do_deinterleaving(szSrc, szDest);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_DO_RS_DECODING(unsigned char *szSrc, 
														   unsigned char *szDest, 
														   int *format,
														   int *err_blk_cnt,
														   int *recovered_err_cnt,
														   int bypass)
{
	if (tspl_do_rs_decoding)
		return tspl_do_rs_decoding(szSrc, szDest, format, err_blk_cnt, recovered_err_cnt, bypass);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSPL_CONVERT_TO_NI(unsigned char *szSrc, unsigned char *szDest, int format)
{
	if (tspl_convert_to_ni)
		return tspl_convert_to_ni(szSrc, szDest, format);
	return TSPL_NO_INTERFACE;
}

//sskim20070205 - v5.1.0
INT		Hlldcaller::TVB380_SET_MODULATOR_DVBH(long modulator_type,
			long tx_mode, 
			long in_depth_interleave, 
			long time_slice, 
			long mpe_fec, 
			long cell_id)
{
	if (tvb380_set_modulator_dvbh)
		return tvb380_set_modulator_dvbh(modulator_type, tx_mode, in_depth_interleave, time_slice, 
			mpe_fec, cell_id);
	return TSPL_NO_INTERFACE;
}
INT		Hlldcaller::TSPL_GET_LAST_ERROR(void)
{
	if (tspl_get_last_error)
		return tspl_get_last_error();
	return TSPL_NO_INTERFACE;
}

//sskim20070209 - 4.7.0
INT		Hlldcaller::TVB380_SET_MODULATOR_PILOT(long modulator_type,//==TVB380_DVBS2_MODE==8
											  long  pilot_on_off)
{
	if (tvb380_set_modulator_pilot)
		return tvb380_set_modulator_pilot(modulator_type, pilot_on_off);
	return TSPL_NO_INTERFACE;
}


//sskim20061128 - v5.0.5 sskim-test
ULONG Hlldcaller::TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address)
{
	if(tspl_read_control_reg)
		return tspl_read_control_reg(Is_PCI_Control, address);
	return (ULONG)TSPL_NO_INTERFACE;

}
INT	Hlldcaller::TSPL_WRITE_CONTROL_REG(int Is_PCI_Control,unsigned long address,unsigned long dwData)
{
	if(tspl_write_control_reg)
		return tspl_write_control_reg(Is_PCI_Control, address, dwData);
	return TSPL_NO_INTERFACE;
}	
//+

//sskim20070322 - DVB-S2 : Roll-off factor added
INT Hlldcaller::TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(long modulator_type,//==TVB380_DVBS2_MODE==8
											  long  roll_off_factor)
{
	if (tvb380_set_modulator_roll_off_factor)
		return tvb380_set_modulator_roll_off_factor(modulator_type, roll_off_factor);
	return TSPL_NO_INTERFACE;
}

//sskim20080724 - BERT
INT	Hlldcaller::TVB380_SET_MODULATOR_BERT_MEASURE(long modulator_type,
										  long packet_type)
{
	if (tvb380_set_modulator_bert_measure)
		return tvb380_set_modulator_bert_measure(modulator_type, packet_type);
	return TSPL_NO_INTERFACE;
}

//sskim20081010 - DTMB
INT		Hlldcaller::TVB380_SET_MODULATOR_DTMB(long modulator_type,
			long constellation, 
			long code_rate, 
			long interleaver, 
			long frame_header, 
			long carrier_number,
			long frame_header_pn,
			long pilot_insertion)
{
	if (tvb380_set_modulator_dtmb)
		return tvb380_set_modulator_dtmb(modulator_type, constellation, code_rate, interleaver, 
			frame_header, carrier_number, frame_header_pn, pilot_insertion);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TVB380_SET_MODULATOR_SDRAM_CLOCK(long modulator_type, long sdram_clock)
{
	if (tvb380_set_modulator_sdram_clock)
		return tvb380_set_modulator_sdram_clock(modulator_type, sdram_clock);
	return TSPL_NO_INTERFACE;
}
																			 
//TSP104
INT		Hlldcaller::TSP104_INIT(int nSlotPosition)
{
	if (tsp104_init)
		return tsp104_init(nSlotPosition);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSP104_OPEN(void)
{
	if (tsp104_open)
		return tsp104_open();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSP104_CLOSE(void)
{
	if (tsp104_close)
		return tsp104_close();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSP104_SELECT_NIM_TYPE(int nim_type)
{
	if (tsp104_select_nim_type)
		return tsp104_select_nim_type(nim_type);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSP104_RESET_NIM(int nim_type, 
		int reset, 
		int lnb_vtg, 
		int lnb_on_off, 
		int lnb_tone)
{
	if (tsp104_reset_nim)
		return tsp104_reset_nim(nim_type, reset, lnb_vtg, lnb_on_off, lnb_tone);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSP104_TVB_TUNE(long  dwfreq)
{
	if (tsp104_tvb_tune)
		return tsp104_tvb_tune(dwfreq);
	return TSPL_NO_INTERFACE;
}

double Hlldcaller::TSP104_TVB_GET_SNR(void)
{
	if (tsp104_tvb_get_snr)
		return tsp104_tvb_get_snr();
	return TSPL_NO_INTERFACE;
}

INT Hlldcaller::TSP104_TVB_SET_DEMODE_CONF(int mode)
{
	if (tsp104_tvb_set_demode_conf)
		return tsp104_tvb_set_demode_conf(mode);
	return TSPL_NO_INTERFACE;
}

UCHAR Hlldcaller::TSP104_TVB_GET_CONST_DATA(int Index)
{
	if (tsp104_tvb_get_const_data)
		return tsp104_tvb_get_const_data(Index);
	return (UCHAR)TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSP104_TOM_TUNE(long dwRfFreq, long dwBandwidth)
{
	if (tsp104_tom_tune)
		return tsp104_tom_tune(dwRfFreq, dwBandwidth);
	return TSPL_NO_INTERFACE;
}
double Hlldcaller::TSP104_TOM_GET_BER(void)
{
	if (tsp104_tom_get_ber)
		return tsp104_tom_get_ber();
	return TSPL_NO_INTERFACE;
}
double Hlldcaller::TSP104_TOM_GET_SNR(void)
{
	if (tsp104_tom_get_snr)
		return tsp104_tom_get_snr();
	return TSPL_NO_INTERFACE;
}
INT Hlldcaller::TSP104_TOM_GET_AGC_STATUS(void)
{
	if (tsp104_tom_get_agc)
		return tsp104_tom_get_agc();
	return TSPL_NO_INTERFACE;
}
INT	Hlldcaller::TSP104_TOM_GET_TPS_STATUS(void)
{
	if (tsp104_tom_get_tps)
		return tsp104_tom_get_tps();
	return TSPL_NO_INTERFACE;
}
INT	Hlldcaller::TSP104_TOM_GET_FEC_STATUS(void)
{
	if (tsp104_tom_get_fec)
		return tsp104_tom_get_fec();
	return TSPL_NO_INTERFACE;
}

//ISDB-T
INT	Hlldcaller::TSP104_TDB_TUNE(long dwRfFreq)
{
	if (tsp104_tdb_tune)
		return tsp104_tdb_tune(dwRfFreq);
	return TSPL_NO_INTERFACE;
}
double Hlldcaller::TSP104_TDB_GET_BER(void)
{
	if (tsp104_tdb_get_ber)
		return tsp104_tdb_get_ber();
	return TSPL_NO_INTERFACE;
}
double Hlldcaller::TSP104_TDB_GET_SNR(void)
{
	if (tsp104_tdb_get_snr)
		return tsp104_tdb_get_snr();
	return TSPL_NO_INTERFACE;
}
INT	Hlldcaller::TSP104_TDB_GET_LOCK_STATUS(void)
{
	if (tsp104_tdb_get_lock)
		return tsp104_tdb_get_lock();
	return TSPL_NO_INTERFACE;
}
INT	Hlldcaller::TSP104_TDB_TUNE_WR_RD(long szReg, long szData)
{
	if (tsp104_tdb_tune_wr_rd)
		return tsp104_tdb_tune_wr_rd(szReg, szData);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_OPEN(void)
{
	if (tse110_open)
		return tse110_open();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TSE110_CLOSE(void)
{
	if (tse110_close)
		return tse110_close();
	return TSPL_NO_INTERFACE;
}

#if	0
//--------------------------------------------------------------------------
//TMCC REMUXER
INT		Hlldcaller::TMCC_STOP_REMUX(void)
{
	if (tmcc_stop_remux)
		return tmcc_stop_remux();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TMCC_START_REMUX(void)
{
	if (tmcc_start_remux)
		return tmcc_start_remux();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TMCC_CLOSE_REMUX_SOURCE(void)
{
	if (tmcc_close_remux_source)
		return tmcc_close_remux_source();
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TMCC_OPEN_REMUX_SOURCE(char *szFilePath)
{
	if (tmcc_open_remux_source)
		return tmcc_open_remux_source(szFilePath);
	return TSPL_NO_INTERFACE;
}

PCHAR	Hlldcaller::TMCC_READ_REMUX_DATA(int dwBytesToRead, int *pReadPos, int *pBufferCnt, int *pEndOfRemux)
{
	if (tmcc_read_remux_data)
		return tmcc_read_remux_data(dwBytesToRead, pReadPos, pBufferCnt, pEndOfRemux);
	return NULL;
}

INT		Hlldcaller::TMCC_SET_REMUX_INFO(long btype, long mode, long guard_interval, long partial_reception, long bitrate,
							   long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
							   long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
							   long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
	if (tmcc_set_remux_info)
		return tmcc_set_remux_info(btype, mode, guard_interval,  partial_reception, bitrate,
			a_segments,  a_modulation,  a_code_rate,  a_time_interleave, a_bitrate,
			b_segments,  b_modulation,  b_code_rate,  b_time_interleave, b_bitrate,
			c_segments,  c_modulation,  c_code_rate,  c_time_interleave, c_bitrate);
	return TSPL_NO_INTERFACE;
}

INT		Hlldcaller::TMCC_SET_LAYER_INFO(long other_pid_map_to_layer,
							   long multi_pid_map,
							   long total_pid_count, char* total_pid_info,
							   long a_pid_count, char* a_pid_info,
							   long b_pid_count, char* b_pid_info,
							   long c_pid_count, char* c_pid_info)
{
	if (tmcc_set_layer_info)
		return tmcc_set_layer_info(other_pid_map_to_layer,
							   multi_pid_map,
							   total_pid_count, total_pid_info,
							   a_pid_count, a_pid_info,
							   b_pid_count, b_pid_info,
							   c_pid_count, c_pid_info);
	return TSPL_NO_INTERFACE;
}

//sskim20080630
INT		Hlldcaller::TMCC_SET_REMUX_SOURCE_POSITION(long a_low, long a_high,
													long b_low, long b_high,
													long c_low, long c_high)
{
	if (tmcc_set_remux_source_position)
		return tmcc_set_remux_source_position(a_low, a_high, b_low, b_high, c_low, c_high);
	return TSPL_NO_INTERFACE;
}

LONG	Hlldcaller::TMCC_GET_REMUX_SOURCE_POSITION(long layer_index, long *high)
{
	if (tmcc_get_remux_source_position)
		return tmcc_get_remux_source_position(layer_index, high);
	return TSPL_NO_INTERFACE;
}

//ISDB-T ASI INPUT TO RF OUTPUT
INT	Hlldcaller::TMCC_WRITE_REMUX_DATA(unsigned char *pData, int nLen)
{
	if (tmcc_write_remux_data)
		return tmcc_write_remux_data(pData, nLen);
	return TSPL_NO_INTERFACE;
}
#endif

//TVB593
INT	Hlldcaller::TSPL_SET_DMA_DIRECTION(long modulator_type, long dma_direction)
{
	if(tspl_set_dma_direction)
		return tspl_set_dma_direction(modulator_type, dma_direction);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_RESET_IP_CORE(long modulator_type, long reset_control)
{
	if(tspl_reset_ip_core)
		return tspl_reset_ip_core(modulator_type, reset_control);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_MH_MODE(long modulator_type, long mh_mode)
{
	if(tspl_set_mh_mode)
		return tspl_set_mh_mode(modulator_type, mh_mode);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_MH_PID(long modulator_type, long mh_pid)
{
	if(tspl_set_mh_pid)
		return tspl_set_mh_pid(modulator_type, mh_pid);
	return TSPL_NO_INTERFACE;
}

INT	Hlldcaller::TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock)
{
	if(tspl_set_symbol_clock)
		return tspl_set_symbol_clock(modulator_type, symbol_clock);
	return TSPL_NO_INTERFACE;
}

//2011/5/4 AD9852 MAX CLK
#ifdef WIN32
INT	Hlldcaller::TSPL_SET_AD9852_MAX_CLOCK(long value)
{
	if(tspl_set_ad9852_max_clock)
		return tspl_set_ad9852_max_clock(value);
	return TSPL_NO_INTERFACE;
}
#endif

INT	Hlldcaller::TSPL_SET_MAX_PLAYRATE(long modulator_type, long use_max_playrate)
{
	cur_max_playrate_sta = use_max_playrate;
	if(tspl_set_max_playrate)
		return tspl_set_max_playrate(modulator_type, use_max_playrate);
	return TSPL_NO_INTERFACE;
}

INT 	Hlldcaller::TSPL_SET_PLAY_RATE_594(long play_freq_in_herz, long multi_module_tag, long nOutputClockSource)
{
	if(tspl_set_play_rate_594)
		return tspl_set_play_rate_594(play_freq_in_herz, multi_module_tag, nOutputClockSource);
	return TSPL_NO_INTERFACE;
}

#if TVB597A_STANDALONE
INT		Hlldcaller::TVB597F_SET_FLASH(char *szFile)
{
	if (tvb597f_set_flash)
		return tvb597f_set_flash(szFile);
	return TSPL_NO_INTERFACE;
}
#endif

INT Hlldcaller::TSPL_GET_wr_BUF_STS_MultiBd_n(int _mod_n)
{
	if(_GET_wr_BUF_STS_MultiBd_n)
		return _GET_wr_BUF_STS_MultiBd_n(_mod_n);
	return TSPL_NO_INTERFACE;
}
INT Hlldcaller::TSPL_GET_wr_cap_BUF_STS_MultiBd_n(void)
{
	if(_GET_wr_cap_BUF_STS_MultiBd_n)
		return _GET_wr_cap_BUF_STS_MultiBd_n();
	return TSPL_NO_INTERFACE;
}
INT Hlldcaller::TSPL_SEL_TS_TAG_VirtualBd_n(int _mod_n)
{
	if(_SEL_TS_TAG_VirtualBd_n)
		return _SEL_TS_TAG_VirtualBd_n(_mod_n);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_SelMultiModAsiOut_n(int _ts_n) //	0, 1, 2, 3
{
	if(__SelMultiModAsiOut_n)
		return __SelMultiModAsiOut_n(_ts_n);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_SelMultiMod310Out_n(int _ts_n) //	0, 1, 2, 3
{
	if(__SelMultiMod310Out_n)
		return __SelMultiMod310Out_n(_ts_n);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_SelMultiModTsOutput_n(int _ctl)
	//	0: select stored files as modulator TS source
	//	1: select external TS input as modulator TS source
{
	if(__SelMultiModTsOutput_n)
		return __SelMultiModTsOutput_n(_ctl);
	return TSPL_NO_INTERFACE;
}
INT 	Hlldcaller::TSPL_SelMultiModOperationMode_n(int _ctl)
	//	00 : STOP
	//	01 : PLAY
	//	10 : CAPTURE
	//	11 : Reserved
{
	if(__SelMultiModOperationMode_n)
		return __SelMultiModOperationMode_n(_ctl);
	return TSPL_NO_INTERFACE;
}



