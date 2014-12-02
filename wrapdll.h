/*
 * TPG0590VC C++ Project: wrapdll.h - HLD wrapper class header
 *
 * Copyright (c) TELEVIEW
 * All rights reserved.
 *
 *  TPG0590VC C++ Application controls TVB590/595 Modulator Boards.
 *  It was converted from VB program.
 *  Created: July 2009
 */

#ifndef _WRAP_DLL_H_
#define _WRAP_DLL_H_
//---------------------------------------------------------------------------
#ifdef WIN32
	typedef INT		(WINAPI* FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,char *,int);
	typedef INT		(WINAPI* FTYPE_INT_INT_LONG)(int, long);
	typedef INT		(WINAPI* FTYPE_INT_INT_LONG_LONG)(int,long,long);
	typedef INT		(WINAPI* FTYPE_INT_INT_INT)(int,int);
    typedef INT		(WINAPI* FTYPE_INT_INT_INT_INT_INT)(int,int,int,int);
    typedef INT		(WINAPI* FTYPE_INT_INT_INT_PCHAR_LONG_LONG_LONG)(int,int,char *,long,long,long);
    typedef INT		(WINAPI* FTYPE_INT_PCHAR_INT)(int,char *,int);
    typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR)(int,char *);
    typedef INT		(WINAPI* FTYPE_INT_INT)(int);
	typedef DWORD	(WINAPI* FTYPE_DWORD_INT_INT_DWORD)(int,int,unsigned long);
	typedef LONG	(WINAPI* FTYPE_LONG_INT_INT_DWORD_DWORD)(int,int,unsigned long,unsigned long);
	typedef INT		(WINAPI* FTYPE_INT_LONG_LONG_LONG_PCHAR_PCHAR)(int,long,long,long,char *,char *);
	typedef INT		(WINAPI* FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_DOUBLE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT)(int,int,int,int,int,int,int,double,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,char *,int);
    typedef INT		(WINAPI* FTYPE_INT_INT_LONG_LONG_DOUBLE)(int,long,long,double);
    typedef INT		(WINAPI* FTYPE_INT_INT_PINT_PINT)(int,int *,int *);
    typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR_PCHAR)(int,char *,char *);
	typedef LONG	(WINAPI* FTYPE_LONG_INT_PCHAR_INT) (int,char *,int);
    typedef INT		(WINAPI* FTYPE_INT_VOID)(void);
    typedef INT		(WINAPI* FTYPE_INT_INT_LONG_DOUBLE)(int,long,double);
	typedef DOUBLE	(WINAPI* FTYPE_DOUBLE_INT_LONG_LONG)(int,long,long);
	typedef INT     (WINAPI* FTYPE_INT_INT_INT_DOUBLE)(int,int,double);
	typedef INT     (WINAPI* FTYPE_INT_INT_INT_INT)(int,int,int);
	typedef INT     (WINAPI* FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG)(int,long,long,long,long,long,long,long,long);
    typedef INT     (WINAPI* FTYPE_INT_INT_LONG_DOUBLE_LONG)(int,long,double,long);
    typedef INT     (WINAPI* FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG)(int,long,long,long,long,long,long);
    typedef INT     (WINAPI* FTYPE_INT_INT_INT_PCHAR)(int,int,char *);
    typedef DOUBLE  (WINAPI* FTYPE_DOUBLE_INT_LONG_DOUBLE_LONG)(int,long,double,long);
    typedef INT     (WINAPI* FTYPE_INT_INT_PCHAR_PCHAR_LONG_PCHAR_LONG_LONG)(int,char *,char *,long,char *,long,long);
	typedef INT     (WINAPI* FTYPE_INT_PCHAR_LONG_DOUBLE_LONG_LONG_LONG_PCHAR)(int,char *,long,double,long,long,long,char *);
	typedef INT     (WINAPI* FTYPE_INT_INT_PCHAR_LONG_DOUBLE_LONG_LONG)(int,char *,long,double,long,long);
	typedef DWORD   (WINAPI* FTYPE_DWORD_INT)(int);
	typedef DOUBLE  (WINAPI* FTYPE_DOUBLE_INT)(int);
    typedef DOUBLE  (WINAPI* FTYPE_DOUBLE_INT_LONG)(int,long);
    typedef LONG    (WINAPI* FTYPE_LONG_INT_LONG)(int,long);
    typedef INT     (WINAPI* FTYPE_INT_INT_INT_INT_INT_INT)(int,int,int,int,int);
	typedef INT     (WINAPI* FTYPE_INT_INT_INT_INT_INT_PCHAR_INT_PCHAR_INT_PCHAR_INT_PCHAR)(int,int,int,int,char *,int,char *,int,char *,int,char *);
    typedef INT     (WINAPI* FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG)(int,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long,long);
    typedef LONG    (WINAPI* FTYPE_LONG_INT)(int);
	typedef INT		(WINAPI* FTYPE_INT_PCHAR)(char *);
	typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR_INT)(int,char *,int);
	typedef INT     (WINAPI* FTYPE_INT_INT_LONG_LONG_LONG)(int,long,long,long);
    typedef INT		(WINAPI* FTYPE_INT_INT_PUNINT_PUNINT)(int,unsigned int *,unsigned int *);
	//2011/11/30 TVB594
	typedef INT     (WINAPI* FTYPE_INT_INT_PINT)(int, int*);
	//2012/2/15 NIT
	typedef INT		(WINAPI* FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT)(int, int*, int*, int*, int*, int*);
	typedef INT		(WINAPI* FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT_PINT_PINT)(int, int*, int*, int*, int*, int*, int*, int*);
	typedef INT		(WINAPI* FTYPE_INT_INT_PINT_PUINT_PINT_PINT_PINT_PINT_PINT_PINT_PINT)(int, int*, unsigned int*, int*, int*, int*, int*, int*, int*, int*);
	
	//2012/3/22
	typedef PCHAR	(WINAPI* FTYPE_PCHAR_INT)(int);

	//2012/4/12 SINGLE TONE
	typedef INT		(WINAPI* FTYPE_INT_LONG_DWORD_LONG)(int, long, unsigned long, long);

	//2012/8/7 SI/PID improve
	typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR_INT_PPSTRUCT)(int, char *, int, struct _TSPH_TS_INFO **);
	typedef INT		(WINAPI* FTYPE_INT_INT_PPSTRUCT)(int, struct _TSPH_TS_INFO **);
	//2012/8/31 new rf level control
	typedef INT		(WINAPI* FTYPE_INT_INT_LONG_DOUBLE_PLONG_LONG)(int, long, double, long *, long);
	typedef INT		(WINAPI* FTYPE_INT_INT_LONG_PDOUBLE_PDOUBLE_LONG)(int, long, double *, double *, long);
    typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR_PSTRUCT)(int,char *,T2MI_Parsing_Info *);
#endif
//---------------------------------------------------------------------------
class CWRAP_DLL
{
private:

public:
    CWRAP_DLL();
#ifndef WIN32
	int m_Init; 
#endif
	void Init_Variables();
	//----------------------------------------------------------------------------
    void	Change_Modulator_Type(long nBoardNum, long lModType);
    void	Close_System(int nBoardNum);
    void	Configure_Modulator(int init);
    char	*Get_AdaptorInfo(int iSlot, int iType);
    void	Get_Enabled_Modulator_Type(long nBoradNum);
    void	Open_System(long nBoradNum);

	void    Set_Burst_Bitrate(long nBoardNum, long lOutputClockSource);
    void    Set_CNR(long nBoardNum, float fCNR);
    void	Set_DTMB_Parameters(long nBoardNum);
    void	Set_DVBT_Parameters(long nBoardNum, int iWhich);
    void	Set_DVBS2_Parameters(long nBoardNum, int iWhich);
    int     Set_Play_Rate_Ex(long nBoardNum, long playFreqHz, long nUseMaxPlayrate);
    void	Set_QAMA_Parameters(long nBoardNum, int iWhich);
    void	Set_QAMB_Parameters(long nBoardNum, int iWhich);
    void	Set_QPSK_Parameters(long nBoardNum, int iWhich);
	unsigned long Set_RF_Frequency(long nBoardNum, unsigned long dwFreq);
    unsigned long Set_Symbolrate(long nBoardNum, unsigned long dwSymbolrate);

	void	SetCurrentPlayOffset(long nBoardNum, long nCurrentSliderPos, double dwFileSize);
    void	SetFileNameNextPlay(long nBoardNum);
    int		SetStreamSourcePort(long nSource);
    void	Start_Playing(long nBoardNum, long lStartPosPercent, char *szFileName);
    int		Start_Recording(long nBoardNum);
    void	Stop_Delaying(long nBoardNum);
    void	Stop_Playing(long nBoardNum);
    void	Stop_Recording(long nBoardNum);
    void	ToggleLoopMode(long nBoardNum);
#ifndef WIN32
	void	UpdateRFPowerLevel(long nBoardNum, double atten, long agc);
	//===================================================================================
#endif
	//2011/1/6 ISDB-T TMCC Setting ======================================================
	void	InitTMCCVarible(long nBoardNum);
	int	UpdateRemuxInfo(long nBoardNum, char *strFilePath, char *szRemuxInfo);
#ifdef WIN32
	int    HaveTmccInformation(long nBoardNum, char *strFilePath);
#endif
	//----------------------------------------------------------------------------
    //--- Simple HLD/LLD API
    int     Check_Ln_Ex(int nSlot, char *ln);
    double  Get_Current_Record_Point(long nBoardNum);
    int     Get_Current_Thread_State(long nBoardNum);
    double  Get_Modulator_Bert_Result_Ex(long nBoardNum, long modulator_type);
    float   Get_Modulator_Rf_Power_Level_Ex(int nSlot, long modulator_type, long lType);
    int     Get_Play_Buffer_Status(long nBoardNum);
    long	Get_Playrate(int iSlot, char *strFile, int iType);
    long    Get_Program(int nSlot, long nIndex);
    int	    Get_Remux_Datarate(int nSlot, long layer_index);
    int	    Get_Remux_Info(int nSlot, char *szFile, int layer_index);

	int     Is_Video_Window_Visible(int nSlot);
    int     Move_Video_Window(int nSlot, int nX, int nY, int nW, int nH);
    unsigned long  Read_Input_Status(int nSlot);
    __int64 Read_Input_Tscount_Ex(int nSlot);
	int     Read_Ip_Streaming_Input_Status(int nSlot, long nStatus);
    int	    Run_Ts_Parser2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo);
    int     Set_Board_Config_Status_Ex(int nSlot, long modulator_type, long status);
    int     Set_Current_Offset(int nSlot, int nOffsetType,	//0=start offset, 1==current offset, 2==end offset
										double dwOffset);
    int	    Set_Error_Injection(int nSlot, long error_lost, long error_lost_packet,
										 long error_bits, long error_bits_packet, long error_bits_count,
										 long error_bytes, long error_bytes_packet, long error_bytes_count);
    int	    Set_Layer_Info(int nSlot,
									int other_pid_map_to_layer,
									int multi_pid_map,
									int total_pid_count, char* total_pid_info,
									int a_pid_count, char* a_pid_info,
									int b_pid_count, char* b_pid_info,
									int c_pid_count, char* c_pid_info);
    int	    Set_Modulator_Bert_Measure_Ex(int nSlot, long modulator_type, long packet_type, long bert_pid);
    int     Set_Modulator_Prbs_Info_Ex(int nSlot, long modulator_type, long mode, double noise_power);
    void    Set_Program(long nBoardNum, int nIndex);
    int	    Set_Remux_Info(int nSlot,
									long btype, long mode, long guard_interval, long partial_reception, long bitrate,
									long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
									long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
									long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
    int     Set_Sdram_Bank_info(long nBoardNum, int nBankCount, int nBankOffset);
    int	    Set_Stop_Mode_Ex(int nSlot, long stop_mode);
    int     Set_Tmcc_Remuxer(int nSlot, long use_tmcc_remuxer);
	int     Show_Video_Window(int nSlot, int nShowWindow);
    int	    Start_Monitor(int nSlot, int nPort);
    long    Tsph_Exit_Process(int nSlot);
#ifndef WIN32
	int 	Set_Loop_Adaptation(int nSlot, 
											 long pcr_restamping,		/* 0=0ff, 1=on */
											 long continuity_conunter,	/* 0=0ff, 1=on */
											 long tdt_tot,				/* 0=off, 1=2nd loop, 2=current date/time, 3=user date/time */
											 char* user_date,			/* yyyy-mm-dd (tdt_tot==3), ex) 2009-11-28 */
											 char* user_time);			/* hh-mm-ss (tdt_tot==3), ex) 01-18-50 */
	//2011/4/28
	int		CheckExtention(char *szFilePath, char *szExt);
#else
	int		TSPH_SET_C2MI_PARAMS(	
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

	int		TSPL_GET_AD9775_EX(int nSlot, long reg);
	int		TSPL_SET_AD9775_EX(int nSlot, long reg, long data);
	int		TSPH_TRY_ALLOC_IQ_MEMORY(int nSlot, int mem_size);
	int		TSPH_SET_IQ_MODE(int nSlot, int mode, int memory_use, int memory_size, int capture_size);
	int		TSPH_SET_COMBINER_INFO(int nSlot, int ts_count, char *ts_path, long modulation, long code_rate, long slot_count);
	int		TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType);
	int 	TSPH_IS_COMBINED_TS(int nSlot, char *ts_path);
	int		TSPH_GET_TS_ID(int nSlot, char* szFile);
	unsigned long	TSPL_READ_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address);
	long    TSPL_WRITE_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long nAddr, unsigned long nData);
	int		TSPH_SET_LOOP_ADAPTATION(int nSlot, long pcr_restamping, long continuity_conunter, long tdt_tot, char* user_date, char* user_time);
	int		TSPH_SET_T2MI_PARAMS(int nSlot, 
			int BW, int FFT_SIZE, int GUARD_INTERVAL, int L1_MOD, int PILOT_PATTERN, int BW_EXT, double FREQUENCY,
			int NETWORK_ID, int T2_SYSTEM_ID, int CELL_ID, int S1, int PLP_MOD, int PLP_COD, int PLP_FEC_TYPE, int HEM, 
			int NUM_T2_FRAMES, int NUM_DATA_SYMBOLS, int PLP_NUM_BLOCKS, int PID
			, int PLP_ROTATION, int PLP_COUNT, int PLP_ID, int PLP_BITRATE, int PLP_TS_BITRATE, char *PLP_TS, int PLP_Time_Interleave
			);
	int		TVB380_SET_MODULATOR_BANDWIDTH_EX(int nSlot, long modulator_type, long bandwidth, double output_frequency);
	int		TSPH_GET_T2MI_PARAMS(int nSlot, int *num_data_symbol, int *plp_num_block);
#ifdef WIN32
	int		TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, T2MI_Parsing_Info *szResult);
#else
	int		TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, char *szResult);
#endif
	int		TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(int nSlot, char* ts_path);
	int 	TSPL_RESET_SDCON_EX(int nSlot);
	long 	TSPH_GET_MHE_PACKET_INFO(int nSlot, char *szPlayTrackList, int nType);
	int		TSPH_GET_LOOP_COUNT(int nSlot);
#ifdef WIN32
	int		TSPL_SET_AD9852_MAX_CLOCK_EX(int nSlot, long value);
#endif
	int		TVB380_SET_BOARD_CONFIG_STATUS_EX(int nSlot, long modulator_type, long status);
	int 	TSPL_GET_AUTHORIZATION_EX(int nSlot);
	int		TSPL_GET_FPGA_INFO_EX(int nSlot, int info);
	int		TVB380_SET_STOP_MODE_EX(int nSlot, long stop_mode);
	int 	TSPH_SET_SDRAM_BANK_INFO(int nSlot, int nBankCount, int nBankOffset);
	int		TSPH_START_MONITOR(int nSlot, int nPort);
	int     TSPL_SET_TSIO_DIRECTION_EX(int nSlot, int nDireciton);
	int 	TSPH_SET_MODULATOR_TYPE(int nSlot, long type);
	int		TSPH_SET_ERROR_INJECTION(int nSlot, long error_lost, long error_lost_packet,
										 long error_bits, long error_bits_packet, long error_bits_count,
										 long error_bytes, long error_bytes_packet, long error_bytes_count);
	int 	TVB380_SET_CONFIG_EX(int nSlot, long modulator_type, long IF_Frequency);
	int		TVB380_SET_MODULATOR_IF_FREQ_EX(int nSlot, long modulator_type, long IF_frequency);
	int 	TVB380_SET_MODULATOR_FREQ_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth);
	int 	TVB380_SET_MODULATOR_CODERATE_EX(int nSlot, long modulator_type, long code_rate);
	int 	TVB380_SET_MODULATOR_DVBH_EX(int nSlot, long modulator_type, long  tx_mode, long  in_depth_interleave, long  time_slice, long  mpe_fec, long  cell_id);
	int		TVB380_SET_MODULATOR_TXMODE_EX(int nSlot, long modulator_type, long tx_mode);
	int		TVB380_SET_MODULATOR_GUARDINTERVAL_EX(int nSlot, long modulator_type, long guard_interval);
	int		TVB380_SET_MODULATOR_CONSTELLATION_EX(int nSlot, long modulator_type,long constellation);
	int 	TVB380_SET_MODULATOR_SYMRATE_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth);
	int		TVB380_SET_MODULATOR_INTERLEAVE_EX(int nSlot, long modulator_type, long interleaving);
	int 	TVB380_SET_MODULATOR_PRBS_MODE_EX(int nSlot, long modulator_type, long mode);
	int 	TVB380_SET_MODULATOR_PRBS_SCALE_EX(int nSlot, long modulator_type, double noise_power);
	int 	TVB380_SET_MODULATOR_PRBS_INFO_EX(int nSlot, long modulator_type, long mode, double noise_power);
	int		TSPH_SET_TMCC_REMUXER(int nSlot, long use_tmcc_remuxer);
	int		TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(int nSLot, long modulator_type, long roll_off_factor);
	int 	TVB380_SET_MODULATOR_PILOT_EX(int nSlot, long modulator_type, long  pilot_on_off);
	int		TSPL_SET_MAX_PLAYRATE_EX(int nSlot, long modulator_type, long use_max_playrate);
	int		TVB380_SET_MODULATOR_BERT_MEASURE_EX(int nSlot, long modulator_type, long packet_type, long bert_pid);
	int		TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(int nSlot, long modulator_type, long sdram_clock);
	int 	TSPL_GET_ENCRYPTED_SN_EX(int nSlot, int type, char* sn);
	int		TVB380_IS_ENABLED_TYPE_EX(int nSlot, long modulator_type);
	int		TSPL_GET_LAST_ERROR_EX(int nSlot);
	int		TSPL_SET_PLAY_RATE_EX(int nSlot, long play_freq_in_herz, long nUseMaxPlayrate);
	int		TVB380_SET_MODULATOR_DTMB_EX(int nSlot, long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion);
	int		TSPH_SET_FILENAME_NEXT_PLAY(int nSlot, char *szNextPlayFile);
	int		TSPH_SET_ISDBS_BASE_TS(int nSlot, char* ts_path);
	int		TSPH_SET_RX_IP_STREAMING_INFO(int nSlot, char* src_ip, char* rx_multicast_ip, long rx_udp_port, char* local_ip, long fec_udp_off, long fec_inact);
	int 	TSPH_START_IP_STREAMING(int nSlot, 
									char *szFilePath,		//File path to be played or recorded
									long nPlayrate,			//In bps
									double dwStartOffset,	//Start offest in bytes
									long nOption,			//Restamping
									long nRepeatMode,		//Play mode
									long nExtendedMode,		//Operation mode thru. IP
									char *szExtendedInfo);	//Source or Target thru. IP
	int 	TSPH_START_PLAY(int nSlot, char *szNewPlayFile, long lPlayRate, double dwStartOffset, long nUseMaxPlayrate, long lRepeatMode);
	int		TSPH_START_RECORD(int nSlot, char* szNewRecordFile, int nPort);
	int		TSPH_SET_REPEAT_PLAY_MODE(int nSlot, int nRepeatMode);
	int		TSPH_GET_REMUX_INFO(int nSlot, char *szFile, int layer_index);
	int 	TSPL_CHECK_LN_EX(int nSlot, char* ln);
	double  TSPH_GET_CURRENT_RECORD_POINT(int nSlot);
	int 	TSPH_GET_CURRENT_THREAD_STATE(int nSlot);
	double 	TVB380_GET_MODULATOR_BERT_RESULT_EX(int nSlot, long modulator_type);
	int 	TSPH_GET_PLAY_BUFFER_STATUS(int nSlot);
	//2011/8/31
	int 	TSPH_IP_RECV_STATUS(int nSlot, unsigned int *buf_bytes_avable, unsigned int *cnt_lost_pkt);

	long 	TSPH_GET_PROGRAM(int nSlot, long nIndex);
	int		TSPH_GET_REMUX_DATARATE(int nSlot, long layer_index);
	int 	TSPH_IS_VIDEO_WINDOW_VISIBLE(int nSlot);
	int 	TSPH_MOVE_VIDEO_WINDOW(int nSlot, int nX, int nY, int nW, int nH);
	unsigned long 		TSPL_READ_INPUT_STATUS_EX(int nSlot);
	double  TSPL_READ_INPUT_TSCOUNT_EX(int nSlot);
	int 	TSPH_READ_IP_STREAMING_INPUT_STATUS(int nSlot, long nStatus);					//nStatus==0(Input Bitrate), 2(Demux Bitrate)
	int		TSPH_RUN_TS_PARSER2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo);

	int		TSPH_SET_LAYER_INFO(int nSlot,
									int other_pid_map_to_layer,
									int multi_pid_map,
									int total_pid_count, char* total_pid_info,
									int a_pid_count, char* a_pid_info,
									int b_pid_count, char* b_pid_info,
									int c_pid_count, char* c_pid_info);
	long 	TSPH_SET_PROGRAM(int nSlot, long nIndex);
	int		TSPH_SET_REMUX_INFO(int nSlot,
									long btype, long mode, long guard_interval, long partial_reception, long bitrate,
									long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
									long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
									long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate);
	int		TSPH_SHOW_VIDEO_WINDOW(int nSlot, int nShowWindow);
	long    TSPH_EXIT_PROCESS(int nSlot);
	int 	TSPH_GET_BOARD_LOCATION(void);
	int 	TSPL_GET_BOARD_ID_EX(int nSlot);
	int 	TVB380_SET_MODULATOR_AGC_EX(int nSlot, long modulator_type, long agc_on_off, long UseTAT4710);
	int 	TVB380_SET_MODULATOR_ATTEN_VALUE_EX(int nSlot, long modulator_type, double atten_value, long UseTAT4710);
	double 	TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(int nSlot, long modulator_type, long info_type);
	int		TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(int nSlot, long modulator_type, long spectral_inversion);
	int 	TSPH_SET_CURRENT_OFFSET(int nSlot, 
										int nOffsetType,	//0=start offset, 1==current offset, 2==end offset
										double dwOffset);
	int		TSPL_SET_BOARD_LED_STATUS_EX(int nSlot, int status_LED, int fault_LED);
	int		TSPH_RUN_ATSC_MH_PARSER(int nSlot, char *szFile, char *szResult);
	int		TSPH_RUN_MFS_PARSER(int nSlot, char *szFile, char *szResult);
	int 	TSPL_GET_BOARD_REV_EX(int nSlot);
	int		TSPL_GET_BOARD_CONFIG_STATUS_EX(int nSlot);
	int     TSPH_CLEAR_REMUX_INFO(int nSlot);
	int		TSPH_RUN_C2MI_PARSER(int nSlot, char *szFile, char *szResult);
	//2011/5/27
	int		LoadHLDLibrary();
	void	ReleaseHLDLibrary();	

	//2011/10/24 added PAUSE
	int		TSPH_START_DELAY(int nSlot, int nPort);
	//2011/11/10 ISDB-T 13seg 188 TS Loopthru
	int		TSPH_IS_LOOPTHRU_ISDBT13_188(int nSlot);
	int		TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser(int nSlot, struct _TSPH_TS_INFO **ppTsInfo);
	int		TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(int nSlot);
	int		TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(int nSlot);
	//2011/11/21 IQ NEW FILE FORMAT
	void    IQ_CapturedFile_Error_Check();
	//2011/11/28 TVB594
	int		TSPH__STREAM_NUMBER(int nSlot);
	int		TSPH__SEL_TS_of_ASI310OUT(int nSlot, int _ts_n);

	//2012/2/15 NIT
	int		TSPH_GET_NIT_SATELLITE_INFO(int nSlot, 
										int *descriptor_flag,
										int *freq,
										int *rolloff,
										int *modulation_system,
										int *modulation,
										int *symbolrate,
										int *coderate);

	int		TSPH_GET_NIT_CABLE_INFO(int nSlot, 
									int *descriptor_flag,
									int *freq,
									int *modulation,
									int *symbolrate,
									int *coderate);
	
	int 	TSPH_GET_NIT_TERRESTRIAL_INFO(int nSlot, int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod);


	//2012/3/21
	int		TSPH_GetRealBdCnt_N(void);
	int		TSPH_GetRealAndVirBdMap(int _r_bd_id, int *_map);
	int		TSPH_GetBdId_N(int _Nth);
	char	*TSPH_GetBdName_N(int _Nth);
	int		TSPH_ConfTvbSytem(void);
	int		TSPH_InitAllRealBd(void);
	int		TSPH_InitVirBd(int _r_bd_id, int _v_cnt);
	int		TSPH_ActivateOneBd(int _bd_id, int _init_modulator, int _init_if_freq);
	int		TSPL_CNT_MULTI_VSB_RFOUT_EX(int nSlot);
	int		TSPL_CNT_MULTI_QAM_RFOUT_EX(int nSlot);
	//2012/6/28 multi dvb-t
	int		TSPL_CNT_MULTI_DVBT_RFOUT_EX(int nSlot);
	//2012/4/12 SINGLE TONE
	int 	TVB380_SET_MODULATOR_SINGLE_TONE_EX(int nSlot, long modulator_type, unsigned long freq, long singleTone);
	//2012/8/31 new rf level control
	int 	TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX(
			int nSlot, 
			long modulator_type, 
			double rf_level_value, 
			long *AmpFlag, 
			long UseTAT4710);
	int 	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX(
			int nSlot, 
			long modulator_type, 
			double *rf_level_min, 
			double *rf_level_max, 
			long UseTAT4710);
	//2012/8/31 new rf level control
	int		TVB59x_SET_PCR_STAMP_CNTL_EX(int nSlot, int _val);
	int		TSPL_GET_AD9787_EX(int nSlot, long reg);
	int		TSPL_SET_AD9787_EX(int nSlot, long reg, long data);
#ifdef WIN32
	int		TSPH_SET_T2MI_STREAM_GENERATION(int nSlot, int Output_T2mi, char* ts_path);
	int		TVB59x_SET_Output_TS_Type_EX(int nSlot, int _val);
	int		TVB59x_SET_Reset_Control_REG_EX(int nSlot, int _val);
	int		TSPL_SET_SDCON_MODE_EX(int nSlot, int _val);
	int		Func_TVB59x_SET_TsPacket_CNT_Mode_EX(int nSlot, int _val);
	int		Func_TVB59x_Get_Asi_Input_rate_EX(int nSlot, int *delta_packet, int *delta_clock);
	//int		Func_TVB59x_Set_Symbol_Clock_EX(int nSlot, int modulator_mode, int symbol_clock);
	int		Func_TVB59x_Modulator_Status_Control_EX(int nSlot, int modulator, int index, int val);
	int		Func_TVB59x_Get_Modulator_Status_EX(int nSlot, int _val);
	int		Func_TSPL_GET_TSIO_STATUS_EX(int nSlot, int option);
#endif
private:
	HINSTANCE m_hInstance;

	FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT		tsph_set_c2mi_params;
	FTYPE_INT_INT_LONG																											tspl_get_ad9775_ex;							
	FTYPE_INT_INT_LONG_LONG																										tspl_set_ad9775_ex;
	FTYPE_INT_INT_LONG																											tspl_get_ad9787_ex;							
	FTYPE_INT_INT_LONG_LONG																										tspl_set_ad9787_ex;
	FTYPE_INT_INT_INT																											tsph_try_alloc_iq_memory;
    FTYPE_INT_INT_INT_INT_INT_INT																								tsph_set_iq_mode;
    FTYPE_INT_INT_INT_PCHAR_LONG_LONG_LONG																						tsph_set_combiner_info;
    FTYPE_INT_PCHAR_INT																											tsph_cal_play_rate;
    FTYPE_INT_INT_PCHAR																											tsph_is_combined_ts;
    FTYPE_INT_INT_PCHAR																											tsph_get_ts_id;
	FTYPE_DWORD_INT_INT_DWORD																									tspl_read_control_reg_ex;
	FTYPE_LONG_INT_INT_DWORD_DWORD																								tspl_write_control_reg_ex;
    FTYPE_INT_LONG_LONG_LONG_PCHAR_PCHAR																						tsph_set_loop_adaptation;
    FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_DOUBLE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT      tsph_set_t2mi_params;
    FTYPE_INT_INT_LONG_LONG_DOUBLE																								tvb380_set_modulator_bandwidth_ex;
	FTYPE_INT_INT_PINT_PINT																										tsph_get_t2mi_params;
#ifdef WIN32
    FTYPE_INT_INT_PCHAR_PSTRUCT																									tsph_run_t2mi_parser;
#else
    FTYPE_INT_INT_PCHAR_PCHAR																									tsph_run_t2mi_parser;
#endif
    FTYPE_INT_INT_PCHAR																												tsph_isdbs_calc_combined_ts_bitrate;
	FTYPE_INT_INT																												tspl_reset_sdcon_ex;
	FTYPE_LONG_INT_PCHAR_INT																									tsph_get_mhe_packet_info;
	FTYPE_INT_INT																												tsph_get_loop_count;
	FTYPE_INT_INT_LONG																											tspl_set_ad9852_max_clock_ex;				
    FTYPE_INT_INT_LONG_LONG																										tvb380_set_board_config_status_ex;
    FTYPE_INT_INT_PCHAR_PCHAR																									tsph_run_atsc_mh_parser;
    FTYPE_INT_VOID																												tsph_get_board_location;
    FTYPE_INT_INT																												tspl_get_board_id_ex;
    FTYPE_INT_INT_LONG_LONG_LONG																								tvb380_set_modulator_agc_ex;
	FTYPE_INT_INT_LONG_DOUBLE_LONG																								tvb380_set_modulator_atten_value_ex;		
	FTYPE_DOUBLE_INT_LONG_LONG																									tvb380_get_modulator_rf_power_level_ex;		
	FTYPE_INT_INT_PCHAR_PCHAR 																									tsph_run_mfs_parser;				
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_spectrum_inversion_ex;	
    FTYPE_INT_INT_INT_DOUBLE 																									tsph_set_current_offset;					
    FTYPE_INT_INT_INT_INT 																										tspl_set_board_led_status_ex;				
    FTYPE_INT_INT 																												tspl_get_board_rev_ex;						
	FTYPE_INT_INT 																												tspl_get_board_config_status_ex;
    FTYPE_INT_INT 																												tspl_get_authorization_ex;			
    FTYPE_INT_INT_INT 																											tspl_get_fpga_info_ex;					
	FTYPE_INT_INT_LONG 																											tvb380_set_stop_mode_ex;					
	FTYPE_INT_INT_INT_INT 																										tsph_set_sdram_bank_info;					
	FTYPE_INT_INT_INT 																											tsph_start_monitor;							
	FTYPE_INT_INT_INT 																											tspl_set_tsio_direction_ex;					
    FTYPE_INT_INT_LONG																											tsph_set_modulator_type;					
    FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 																		tsph_set_error_injection;					
    FTYPE_INT_INT_LONG_LONG																										tvb380_set_config_ex;						
    FTYPE_INT_INT_LONG_LONG																										tvb380_set_modulator_if_freq_ex;			
    FTYPE_INT_INT_LONG_DOUBLE_LONG 																								tvb380_set_modulator_freq_ex;				
	FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG 																				tvb380_set_modulator_dvbh_ex;				
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_txmode_ex;				
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_coderate_ex;			
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_guardinterval_ex;		
    FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_constellation_ex;		
    FTYPE_INT_INT_LONG_DOUBLE_LONG																								tvb380_set_modulator_symrate_ex;			
    FTYPE_INT_INT_LONG_LONG																										tvb380_set_modulator_interleave_ex;			
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_prbs_mode_ex;			
    FTYPE_INT_INT_LONG_DOUBLE 																									tvb380_set_modulator_prbs_scale_ex;			
    FTYPE_INT_INT_LONG_LONG_DOUBLE 																								tvb380_set_modulator_prbs_info_ex;			
	FTYPE_INT_INT_LONG 																											tsph_set_tmcc_remuxer;						
	FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_roll_off_factor_ex;	
	FTYPE_INT_INT_LONG_LONG																										tvb380_set_modulator_pilot_ex;				
	FTYPE_INT_INT_LONG_LONG 																									tspl_set_max_playrate_ex;					
    FTYPE_INT_INT_LONG_LONG_LONG																								tvb380_set_modulator_bert_measure_ex;		
    FTYPE_INT_INT_LONG_LONG 																									tvb380_set_modulator_sdram_clock_ex;		
    FTYPE_INT_INT_INT_PCHAR 																									tspl_get_encrypted_sn_ex;					
    FTYPE_INT_INT_LONG 																											tvb380_is_enabled_type_ex;					
    FTYPE_INT_INT 																												tspl_get_last_error_ex;						
	FTYPE_INT_INT_LONG_LONG 																									tspl_set_play_rate_ex;						
	FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 																		tvb380_set_modulator_dtmb_ex;				
	FTYPE_INT_INT_PCHAR 																										tsph_set_filename_next_play;				
    FTYPE_INT_INT_PCHAR 																										tsph_set_isdbs_base_ts;						
    FTYPE_INT_INT_PCHAR_PCHAR_LONG_PCHAR_LONG_LONG 																				tsph_set_rx_ip_streaming_info;				
    FTYPE_INT_PCHAR_LONG_DOUBLE_LONG_LONG_LONG_PCHAR																			tsph_start_ip_streaming;					
	FTYPE_INT_INT_PCHAR_LONG_DOUBLE_LONG_LONG 																					tsph_start_play;							
    FTYPE_INT_INT_PCHAR_INT 																									tsph_start_record;							
    FTYPE_INT_INT_INT 																											tsph_set_repeat_play_mode;					
	FTYPE_INT_INT_PCHAR_INT 																									tsph_get_remux_info;						
	FTYPE_INT_INT_PCHAR 																										tspl_check_ln_ex;							
	FTYPE_DOUBLE_INT																											tsph_get_current_record_point;				
    FTYPE_INT_INT																												tsph_get_current_thread_state;				
    FTYPE_DOUBLE_INT_LONG																										tvb380_get_modulator_bert_result_ex;		
    FTYPE_INT_INT																												tsph_get_play_buffer_status;				
    FTYPE_LONG_INT_LONG																											tsph_get_program;							
    FTYPE_INT_INT_LONG																											tsph_get_remux_datarate;					
	FTYPE_INT_INT																												tsph_is_video_window_visible;				
	FTYPE_INT_INT_INT_INT_INT_INT																								tsph_move_video_window;						
	FTYPE_DWORD_INT																												tspl_read_input_status_ex;					
	FTYPE_DOUBLE_INT																											tspl_read_input_tscount_ex;					
    FTYPE_INT_INT_LONG																											tsph_read_ip_streaming_input_status;		
    FTYPE_INT_INT_INT_INT_INT_PCHAR_INT_PCHAR_INT_PCHAR_INT_PCHAR																tsph_set_layer_info;						
	FTYPE_LONG_INT_LONG																											tsph_set_program;							
    FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 			tsph_set_remux_info;						
    FTYPE_INT_INT_INT																											tsph_show_video_window;						
	FTYPE_LONG_INT																												tsph_exit_process;	
    FTYPE_INT_INT	 																											tsph_clear_remux_info;	
	FTYPE_INT_INT_PCHAR_PCHAR																									tsph_run_c2mi_parser;
	//2011/8/31 added
	FTYPE_INT_INT_PUNINT_PUNINT																									tsph_ip_recv_status;
    //2011/10/24 added PAUSE
	FTYPE_INT_INT_INT 																											tsph_start_delay;					
    //2011/11/10 ISDB-T 188 TS LoopThru
	FTYPE_INT_INT	 																											tsph_is_loopthru_isdbt13_188;		
    FTYPE_INT_INT_PPSTRUCT	 																									tsph_pause_loopthru_isdbt13_parser;		
    FTYPE_INT_INT	 																											tsph_buf_clear_loopthru_isdbt13_188;		
    FTYPE_INT_INT	 																											tsph_is_loopthru_inbuf_full_isdbt13_188;		
    //2011/11/28 TVB594
    FTYPE_INT_INT	 																											tsph__stream_number;		
	FTYPE_INT_INT_INT 																											tsph__sel_ts_of_asi310out;
	//2012/2/15
	FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT_PINT_PINT																			tsph_get_nit_satellite_info;
	FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT																						tsph_get_nit_cable_info;
	FTYPE_INT_INT_PINT_PUINT_PINT_PINT_PINT_PINT_PINT_PINT_PINT																	tsph_get_nit_terrestrial_info;
	
	//2012/3/21
    FTYPE_INT_VOID	 																											tsph_getrealbdcnt_n;
    FTYPE_INT_INT_PINT	 																										tsph_getrealandvirbdmap;
    FTYPE_INT_INT	 																											tsph_getbdid_n;
    FTYPE_PCHAR_INT	 																											tsph_getbdname_n;
    FTYPE_INT_VOID	 																											tsph_conftvbsystem;
    FTYPE_INT_VOID	 																											tsph_initallrealbd;
    FTYPE_INT_INT_INT	 																										tsph_initvirbd;
    FTYPE_INT_INT_INT_INT																										tsph_activateonebd;
    FTYPE_INT_INT	 																											tspl_cnt_multi_vsb_rfout_ex;
    FTYPE_INT_INT	 																											tspl_cnt_multi_qam_rfout_ex;
    //2012/6/28 multi dvb-t
	FTYPE_INT_INT	 																											tspl_cnt_multi_dvbt_rfout_ex;

	//2012/4/12 SINGLE TONE
	FTYPE_INT_LONG_DWORD_LONG																									tvb380_set_modulator_single_tone_ex;
	//2012/8/7 improve
	FTYPE_INT_INT_PCHAR_INT_PPSTRUCT																							tsph_run_ts_parser2;
	//2012/8/31 new rf level control
	FTYPE_INT_INT_LONG_DOUBLE_PLONG_LONG																						tvb59x_set_modulator_rf_level_value_ex;
	FTYPE_INT_INT_LONG_PDOUBLE_PDOUBLE_LONG																						tvb59x_get_modulator_rf_level_range_ex;
	FTYPE_INT_INT_INT																											tvb59x_set_pcr_stamp_cntl_ex;
#ifdef WIN32
    FTYPE_INT_INT_INT_PCHAR 																									tsph_set_t2mi_stream_generation;					
	FTYPE_INT_INT_INT																											tvb59x_set_output_ts_type_ex;
	FTYPE_INT_INT_INT																											tvb59x_set_reset_control_reg_ex;
	FTYPE_INT_INT_INT																											tspl_set_sdcon_mode_ex;
	FTYPE_INT_INT_INT																											call_tvb59x_set_tspacket_cnt_mode_ex;
	FTYPE_INT_INT_PINT_PINT																							call_tvb59x_get_asi_input_rate_ex;
	
	FTYPE_INT_INT_LONG_LONG_LONG																			call_tvb59x_modulator_status_control_ex;
	FTYPE_INT_INT_INT																											call_tvb59x_get_modulator_status_ex;
	FTYPE_INT_INT_INT																											call_tspl_get_tsio_status_ex;
#endif
#endif
};	
extern CWRAP_DLL* gc_wrap_dll;

#endif	//_WRAP_DLL_H_

