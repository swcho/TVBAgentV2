#ifndef _UTIL_INDEPENDANT_H_
#define _UTIL_INDEPENDANT_H_
#include <stdio.h>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class CUTIL_IND
{                             
private:

public:
    CUTIL_IND();

    //--------------------------------------------------------
    // Modulator/Hardware Related
    //--------------------------------------------------------
    long    AdjustBankOffset(long nBoardNum, long playRate);
    void    Adjust_Spectrum(long nBoardNum);

    long    CalcBurtBitrate(long nBoardNum);
    long    CalcBurtBitrate_DVB_S2(long nBoardNum);
    long    CalcSymbolRate(long nBoardNum);
    long    CalcSymbolRate_DVBS2(long nBoardNum);
    bool    Check_Multi_Board();
    void    CheckLoopAdaptation(long nBoardNum);

    long    Get_At_Bat_File(char *strLine, char *TempPath);
    char    *Get_BERT_String(long nBoardNum);
    unsigned long    Get_DTMB_Frequency(long chNum);
	//2011/11/22 added ISDB-S CHANNEL
    unsigned long    Get_ISDBS_Frequency(long chNum);

    unsigned long    Get_ISDBT_Frequency(long codeNum, long chNum);
    unsigned long    Get_OFDM_Frequency(long bCATV, long codeNum, long chNum);
    void    Get_TS_Parameters(char *input_string, unsigned short *pPID, char *pSI, unsigned long *pPGMNUM,
                              unsigned short *pPCR, char *pDesc, int *pBitrate);
    unsigned long    Get_TDMB_Frequency(long channelType, long chNum);
    void    Get_Two_Parameters(char *input_string, long *pChNum, long *pFreq);
    unsigned long    Get_VSB_Frequency(long bCATV, long chNum);
    void    GetScheduledTask();

    void    LogMessage(char *strMessage);
    void    LogMessageInt(int iErrNum);
    char    *MakeCommandArgs(long nBoardNum, long nRunType, char *TargetFile);

#ifdef WIN32
    void    OpenAndAppendLNData(char *ln_data);
#else
    void    OpenAndAppendLNData(char *ln_data, char *sn_data, int opt);
#endif
    void    ResetElapsedtimeCounter(long nBoardNum, long dwStartOffseMsec);

    void    Set_Input_Source(long nBoardNum, long lSource);
    void    Set_Next_Input_Source(long nBoardNum);
    int     SetLabPlayRateCalc(char *szFileName);
    void    Start_Stop_Playing(long nBoardNum, char *strfilename);
    void    SyncModulatorMode(long nBoardNum);
    char    *szElapsedTimeinHMSdFormat(long nBoardNum);
    char    *szMsecTimeToHMSdFormat(long dwMSecTime);
    char    *szSecTimeToHMSformat(unsigned long dwSecTime);

    void    Toggle_Video_Size(long nBoardNum);

	void	GetIpTable();
    //--------------------------------------------------------
    // General
    //--------------------------------------------------------
	char	*Convert_To_LowerCase(char *str);
    char    *Get_Commaed_String(int iDigit);
    long    Get_Current_SecTime();
    __int64 Get_File_Size_BYTE(char *strFilename);
    __int64 Get_File_Size_KB(char *strFilename);	//kslee 2010/4/2 long -> __int64
    void    Get_Filename_From_FullPath(char *strFullPath, char *strDirectory, char *strFile);
    long    Get_MDY_From_MMDDYYYY(char *strDate, long *pMM, long *pDD, long *pYY);

    long    Get_One_Line_From_File(FILE *hFile, char *strLine, int iSize);

    long    Get_Sec_From_HHMMSS(char *strTime);
    void    GetFileSizeInSec(char *szFileName, long nBoardNum);
    long    GetCurrentTimeinMsec(long nBoardNum);
    bool    Is_Correct_Extension(char *strFile);
#ifdef WIN32
	bool	Is_Correct_Extension(System::String^ extName);
#endif
    bool    Initialize_Application();

	bool	IsNumericTel(char *szVal);

    void    SNMP_Init();
    void    SNMP_Term();
    long    SNMP_DataSend(char *szSnmpMsg);
    
	long	Get_nKeyVal(int nKey);

	//2010/5/28 DVB_T2
	long	CalcT2MI_Bitrate(long nBoardNum, long nCalcMaxBitrate, long PLP_MOD, long PLP_COD, long PLP_FEC_TYPE,
							double PLP_NUM_BLOCKS, long GUARD_INTERVAL, long FFT_SIZE, long S1, long NUM_DATA_SYMBOLS,
							long PILOT_PATTERN, long BW, long BWT_EXT, long L1_POST_SIZE, long HEM, long NPD);

	//2010/5/31 DVB-T2
	long	Calulate_L1_Post_Size(long L1_MOD, long FFT_SIZE, long L1_POST_INFO_SIZE);

	long	CalcDatarate(long Constellation, int Coderate, int SlotCount);

	//2011/5/9 DVB-C2
	long	CalcC2MI_Bitrate(long nBandWidth, long nL1TIMode, long nStartFreq, long nGuardInterval, long nDsliceType, long nFecHeaderType, long nPlpFecType, 
							long nPlpMod, long nPlpCod, long nHem, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth);

	int  getFreqIndex();

	//2011/5/31 DVB-C2 MULTI PLP
	long	CalcC2MI_PLPBitrate(long nBandWidth, long nL1TIMode, long nGuardInterval, long nPlpCod, long nPlpFecType, long nPlpNumBlk, long nHem);

	//2011/6/21
	long	CalcC2MI_Ctot(long nStartFreq, long nGuardInterval, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth);
	long	CalcC2MI_plpCells(long nPlpFecType, long nPlpMod, long nFecHeaderType, long nPlpNumBlk);

	//2012/5/9
	int		ModulatorTypeValidity(int board_id, int board_rev, int modulator);
	//2012/8/29 new rf level control
	int		IsAttachedBdTyp_NewRFLevel_Cntl(int nBoardNum);
	int		IsEnabled_CN(int nBoardNum);
	int		IsEnabled_DacOffset(int nBoardNum);
	//2012/9/6 pcr restamp
	int		IsSupported_Mod_PcrRestamp_By_HW(int nMod); 
	int		IsVisible_IF_(int nBoardNum);
	//2013/3/14 ================================
	int		IsSupportDvbT_Cell_ID(int nBoardNum);
	int		IsSupportSpectrumInversion(int nBoardNum);
	int		IsVisible_RRC_Filter(int BoardNum);
	int		IsSupportDvbS2_RollOff_None(int BoardNum);
	int		IsSupportDvbT2_TimeIL_Length(int nBoardNum);
	int		IsSupportHMC833(int nBoardNum);
	//=========================================

	int		IsAttachedBdTyp_SupportEepromRW(int nBoardNum);
	void     Write_LN_Eeprom(int nBoardNum, char *szLN);
	//2012/1/12 USB/SDCARD mount check////////////////////////////////////////////////////////////////////////////
#ifdef STANDALONE
	int	chk_existence_string(FILE *_file, char *key_str, char *key_str_opt);
	int	check_usb_dev_attached(int device);
	int	check_usb_fs_mounted(int device);
	int	check_sdcard_dev_attached(int device);
	int	check_sdcard_fs_mounted(int device);
#endif
	////////////////////////////////////////////////////////////////////////////////////


private:
#if defined(WIN32)
	static  DWORD SNMP_ThreadProc();
#else
	static  void 	*SNMP_ThreadProc(void *arg);
	static  long 	SNMP_Message_String(char *strBuffer, void *msgbuf);


#endif

};

extern CUTIL_IND* gc_util_modulator;

#endif	//_UTIL_INDEPENDANT_H_
