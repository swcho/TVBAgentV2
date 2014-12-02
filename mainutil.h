// mainutil class header
#ifndef _MAIN_UTIL_H_
#define _MAIN_UTIL_H_
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class PlayForm
{
private:

public:
    PlayForm();

private:
	static void *TimerMain_Process(void *arg);
public:
    //-----------------------------------------------------------
	bool Initialize();
	void Terminate();
	void SaveStreamListInfo(long nBoardNum, long nModulatorMode);
	
	void OnCheckedAGC(long lAgc);
	void OnCheckedBYPASSAMP(long lByPass);
	void OnBnClickedComf1();
	void OnBnClickedComf2();
	void OnBnClickedComf3();
	void OnBnClickedComf4();
	void OnBnClickedComf5();
	void OnBnClickedComf6();
	void OnLbnSelchangeFilelistbox();
	void OnLbnSelchangePlaylist();
	
	void OnCbnSelchangeParam1(int iCurIx);
	void OnCbnSelchangeParam2(int iCurIx);
	void OnCbnSelchangeParam3(int iCurIx);
	void OnCbnSelchangeParam4(int iCurIx);
	void OnCbnSelchangeParam5(int iCurIx);
	void OnCbnSelchangeParam6(int iCurIx);
	void OnCbnSelchangeParam7(int iCurIx);
	void OnEnChangeElaboutputfrequency(unsigned long dwFreq);
	void OnEnChangeRfOutputLevel(double dwRFLevel);
	void OnEnChangeElabplayrate(int lPlayrate);
	void OnEnChangeCnr(double dwCNR);
	void OnEnChangeElaboutputsymrate(long dwSynbolrate);
	void OnCbnSelchangeAdaptor(int index);
	void OnBnClickedBurstBitrate(long lCheck);
	void OnBnClickedCalcSymbol(long lCheck);
	void OnBnClickedComdir(char *strDirectory);
	void OnBnClickedParam(int bChecked);
	
	void OnCbnSelchangeIf(long tModulatorIFFreq);
	void OnCbnSelchangeModulatorType(int tModType);
	void OnSliderChange(int iPercent);
	void Check_590S_Board();

	void Initial_Check(long nBoardNum);
	void AddFile2PlayList();
	void RemovePlayList(long index);
	void SetFileListFromDirectory(char *strDir);
	void UpdateAgcUI(long nBoardNum, double atten, long agc);
	
	void UpdateRFPowerLevelUI(long nBoardNum);
	void SNMP_Send_Status(int iMsgType);
	
	void UpdateModulatorConfigUI(long nBoardNum);
	void Display_Init();
	void SNMP_All_DataSend();
	
	void Update_File_Play_List();
	
	void UpdateFileListDisplay();
	void UpdatePlayListDisplay();
	void Display_File_Property(long nBoardNum, char *strFilename);
	
	void Display_RF_CNR(long nBoardNum);
	
	void Display_Modulator_Parameter(long nBoardNum);
	void UpdateCmmbUI(long nBoardNum, char *szListFileName);
	void ChangeAdaptor(long lIndex);
	
	void MoveListUpDown(int nKey);
	
	void SNMP_DataArrival(char *strMsg);
	
	void SetSubloop_Time();
	
	void UpdateMRLFromSNMP(int nMode);
	
	void HMsecTimerJob();
	void TimerSub_Process();
	
	void Display_ElapsedTime(long nBoardNum);
	
	void UpdateIPStreamingUI(long nBoardNum);
	void Set_Playrate_Symbolrate_On_ParameterChange(long nBoardNum);
	void Display_PlayRate(long dwRate);
	void Display_SymbolRate(long dwRate);

	//kslee 2010/4/13
	int	SnmpRequestedCheckLN(char *strLN);
	int SnmpRequestedUpdateLN(char *strLN);

	//--- 2010/03/07 ~
	
	int Get_Run_Time_Of_CurrentSet(int nBoardNum);
	
	void Wait_End_Of_FirmwareUpdate();

	// huy: 2011/01/25
	void firmware_update();

	//kslee 2010/5/3
	void UpdateMHEpacketInfo(long nBoardNum, char *szFilePath);
	void UpdateT2MIUI(long nBoardNum, char *szListFileName);

	//2010/5/27
	void GetPlayListFileIndex(long nBoardNum, char *str);

	//2010/6/1
	void SetT2MI();

	//2010/6/28 AUTO-PLAY
	void Set_Auto_Play();

	//2011/1/4 TMCC Setting
	void Set_TMCC_Remuxing();
	void GetPIDInfoLineOne(int index);
	void SendTMCCData();

	//2011/3/25 DVB-T2 MULTI-PLP
	void OnBtnClicked_Search();
	void SetFileListDvbt2_Isdbs(char *strDir);
	void GetFileName_Dvbt2_Isdbs(int index);

	//2011/4/18 ISDB-S
	void SetMultiTS_Combiner();
	void Set_Dac_I_Q_Offset();

	void SetDVBC2_parameters();

	//2012/1/12 USB/SDCARD mount check
#ifdef STANDALONE
	void Check_mount_usb_sdcard();
	void RebootSystem(int opt);
	//2012/6/5
//	void Save_TMCC_Parameters_();
//	void Load_TMCC_Parameters_();
#endif

	//2012/2/29 WIN_Merge =====================================================
	void SetDefaultValue_noTMCC(char *filename);
	void OnClicked_Pause();
	void SetPause_TVB594(int nBoardNum);
	void Get_NIT_Delivery_Descriptor_Info();
    void Check_ISDBT13_Loopthru_188TS();
	//=========================================================================
	//2012/12/21 added
	void OnClicked_Btn_Stop();
	void SetNewFileSubLoopPos();
	void Display_RF_Level_Range(int Set_rf_level_flag);
	int GetMultiOptionLevelOffset(int mod, int BoardID, unsigned long RFfreq);
	void OnValueChanged_RFLevel(double dwRf_level);
	void Set_ItemList_HW();
	void SetSnmpFuncCall_AsiIn(int nParam);
	void DeleteLogFile();
};

extern PlayForm* gc_PlayForm;

#endif	//_MAIN_UTIL_H_

