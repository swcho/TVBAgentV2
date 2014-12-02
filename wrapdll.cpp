/*
 * TPG0590VC C++ Project: wrapdll.cpp - HLD wrapper class header
 *
 * Copyright (c) TELEVIEW
 * All rights reserved.
 *
 *  TPG0590VC C++ Application controls TVB590/595 Modulator Boards.
 *  It was converted from VB program.
 *  Created: July 2009
 */
#include "stdafx.h"
#if defined(WIN32)
#include "stdio.h"
#include <sys/stat.h> // _stati64()
#include <winsock2.h>
#include <iphlpapi.h>
#include "math.h"
#include "time.h"
#include "main.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_ind.h"
#include "util_dep.h"
//#include "HLD_DEF.h"
//#include "LLD_DEF.h"
#include "PlayForm.h"
#include "baseutil.h"
#include "Form_Wait.h"
#else
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//#include "lld_def.h"
#include "variable.h"
#include "hld_api.h"
#include "resource.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include "mainutil.h"

#endif

APPLICATION_CONFIG      *gpConfig;
TEL_GENERAL_VAR         gGeneral;
CWRAP_DLL               gWrapDll;
char					gstrGeneral[256];

extern CREG_VAR         gRegVar;
extern CUTILITY         gUtility;
extern CUTIL_IND        gUtilInd;

#if defined(WIN32)
using namespace TPG0590VC;
using namespace System::Windows::Forms;
#define GET_PROC(a,b,c)			{ b = (a)GetProcAddress(m_hInstance, c); }
#else
extern  PlayForm		gGui;
#endif
//---------------------------------------------------------------------------
CWRAP_DLL* gc_wrap_dll = NULL;

//---------------------------------------------------------------------------
CWRAP_DLL::CWRAP_DLL()
{
} 

//---------------------------------------------------------------------------
#ifdef WIN32 
void CWRAP_DLL::Init_Variables()
{
    int     i, j;

    for (i = 0; i <= MAX_BOARD_COUNT; i++)
    {
        gpConfig->gBC[i].bPlayingProgress = 0;             //flag for playing mode activity
        gpConfig->gBC[i].bRecordInProgress = 0;           //flag for recording mode activity
        gpConfig->gBC[i].bDelayinProgress = 0;            //flag for monitoring mode activity
        gpConfig->gBC[i].bDeleteAsked = 0;                //flag for getting the delete confirm wait SLCT/EXIT
        gpConfig->gBC[i].bForcedPlayRate = 0;

        gpConfig->gBC[i].nLastHLDThreadState = 0;         // 0 if no file playing , 0~19 during play
        gpConfig->gBC[i].gnOutputClockSource = 0;
        gpConfig->gBC[i].nWaitProgressCnt = 0;

        gpConfig->gBC[i].nFocusInput = -1;

        gGeneral.gnTSCount[i] = 0;
        gGeneral.gnLastSec[i] = 0;
        gGeneral.gnPlayRate[i][0] = 0.0;
        gGeneral.gnPlayRate[i][1] = 0.0;
        gGeneral.gnPlayRate[i][2] = 0.0;
        gGeneral.gnPlayRate[i][3] = 0.0;
        gGeneral.gnPlayRate[i][4] = 0.0;
        
		//2011/6/24 FAULT LED fixed
		gpConfig->nBoardRealSlot[i] = -1;
		gpConfig->nBoardSlotNum[i] = -1;
        // Get Board Id
        //----------------------------------------------------------------------------
        //20070309-NO MULTI-BOARD
        //----------------------------------------------------------------------------
		gpConfig->gBC[i].gnBoardStatus = -1;
		gpConfig->gBC[i].gnBoardId = -1;
		gpConfig->gBC[i].gnBoardRev = -1;
        
		gpConfig->gBC[i].gnErrorStatus = 0;

        gpConfig->gBC[i].gnOpenSystem = 0;
        gpConfig->gBC[i].gnUseFrontInput = 0;
        gpConfig->gBC[i].gnUseDemuxblockTest = 0;

        //----------------------------------------------------------------------------
        //sskim20071130 - A/V decoding, IP Streaming
        //----------------------------------------------------------------------------
        gpConfig->gBC[i].gnUseIPStreaming = 0;
        gpConfig->gBC[i].gnIPStreamingMode = NO_IP_STREAM;
        gUtility.MyStrCpy(gpConfig->gBC[i].gszIPStreamingInfo, 256,  "");
        gUtility.MyStrCpy(gpConfig->gBC[i].gszIPStreamingInputInfo, 256, "");
        gpConfig->gBC[i].gnIPSubBankOffset = 1024;
        gpConfig->gBC[i].gnUseAVDecoding = 0;
        
        gpConfig->gBC[i].gnIPTotalProgram = 0;
        gpConfig->gBC[i].gnIPCurrentProgram = -1;
        gGeneral.gnVLCRunning = -1;

        //----------------------------------------------------------------------------
        //sskim20070528 - keisoku
        //----------------------------------------------------------------------------
        gpConfig->gBC[i].gnRFOutFreqUnit  = RF_OUT_HZ;
        gpConfig->gBC[i].gnRemoveFileEnabled = 0;
        gpConfig->gBC[i].gnStartPos =  0;
        gpConfig->gBC[i].gnCurrentPos = 0;
        gpConfig->gBC[i].gnEndPos = 0;
        gpConfig->gBC[i].gnStartSliderPos = 0;
        gpConfig->gBC[i].gnEndSliderPos = 0;
                
        gpConfig->gBC[i].gnStartPosChanged = 0;
        gpConfig->gBC[i].gnCurrentPosChanged = 0;
        gpConfig->gBC[i].gnEndPosChanged = 0;
        gpConfig->gBC[i].gnCurrentPosScrolled = 0;
        
        gpConfig->gBC[i].gnStartTimeOffset = 0;
        gpConfig->gBC[i].gnEndTimeOffset = 0;
        gpConfig->gBC[i].gnOffsetMargin = 0;

        gpConfig->gBC[i].gnUseSubLoop = 0;
        gpConfig->gBC[i].gnMaxRFLevel = 0;
        gpConfig->gBC[i].gnMinRFLevel = 0;
        gpConfig->gBC[i].gnPrevPlayrate = 0;
        gpConfig->gBC[i].gnPrevSymbolrate = 0;
        gpConfig->gBC[i].gnRepeatCount = 0;
        
        gpConfig->gBC[i].gInvalidBitrate = 0;
        gpConfig->gBC[i].gRFPowerLevel = 0.0;
        gpConfig->gBC[i].gnVideoPosX = 0;
        gpConfig->gBC[i].gnVideoPosY = 0;
        gpConfig->gBC[i].gnVideoWidth = VLC_VIDEO_WIDTH_S;
        gpConfig->gBC[i].gnVideoHeight = VLC_VIDEO_HEIGHT_S;
        gpConfig->gBC[i].gnTAT4710 = -1;

        gpConfig->gBC[i].gnActiveMixerUsed = 0;

		gpConfig->gBC[i].gnBoardAuth = 0;
		gpConfig->gBC[i].gnIQ_play_support = 0;
		gpConfig->gBC[i].gnIQ_capture_support = 0;

		//AGC - RF Level -> Atten/AGC
		gpConfig->gBC[i].gnAGC = 0;

		//2010/5/28
		gpConfig->gBC[i].gnBitrate_Adjustment_Flag = -1;
		
		gUtility.MyStrCpy(gpConfig->gBC[i].gszIQ_capture_filePath,512, "");

		//2010/12/07 ISDB-S =======================================================================================================================
		gpConfig->gBC[i].gnSlotCount = MAX_SLOT_COUNT;

		for(j = 0 ; j < MAX_TS_COUNT ; j++)
		{
			gpConfig->gBC[i].gnConstellation_M[j] = -1;
			gpConfig->gBC[i].gnCoderate_M[j] = -1;
			gpConfig->gBC[i].gnSlotCount_M[j] = 0;
			gUtility.MyStrCpy(gpConfig->gBC[i].gszTS_M[j], 512, "");
			gpConfig->gBC[i].gnT2MI_PLP_Time_Interleave[j] = 0;
		}
		gpConfig->gBC[i].gnIP_T2MI_PLP_Time_Interleave = 0;
		//=========================================================================================================================================

        //----------------------------------------------------------------------------
        //sskim20080212 - SCHEDULED TASKS
        //----------------------------------------------------------------------------
        gUtility.MyStrCpy(gpConfig->gBC[i].gnCmdAutoExitTime,24, "");
        gpConfig->gBC[i].gnCmdAutoExitDone = 0;
        gUtility.MyStrCpy(gpConfig->gBC[i].gnCmdAutoExitDate,24, "");
        gpConfig->gBC[i].gnCmdDurationTime = 0;

			
		//2011/6/2
		gpConfig->gBC[i].gnDVB_C2_CreateFile = 0;
		//2011/10/24 added PAUSE
		gpConfig->gBC[i].gnPause = 0;

		//2011/11/28 TVB594
		gpConfig->gBC[i].gn_OwnerSlot = -1;
		gpConfig->gBC[i].gnRealandvir_cnt = -1;
		gpConfig->gBC[i].gnRealandvir_location = 0;
		gpConfig->gBC[i].gn_IsVirtualSlot = 0;
		gpConfig->gBC[i].gn_StreamNum = -1;
		gpConfig->gnInstalledBoard_InitMode[i] = 1;
        gUtility.MyStrCpy(gpConfig->gszInstalledBoard_Info[i],256, "");
		//2012/9/3 new rf level control
		gpConfig->gBC[i].gdRfLevelValue = 40;
		gpConfig->gBC[i].gdRfLevelRange_max = 0;
		gpConfig->gBC[i].gdRfLevelRange_min = 0;

		//2012/9/6 pcr restamp
		gpConfig->gBC[i].gnPcrReStampingFlag = 0;	//default is 0;

		//2012/9/28 BERT fix
		gpConfig->gBC[i].gnBert_Pid = 0x100;
		gpConfig->gBC[i].gnETI_Format = 2;
		gpConfig->gBC[i].gnATSC_MH_Format = 0;
		gpConfig->gBC[i].gnInputSource = 0;
		gpConfig->gBC[i].gnOutputType = 0;
		gpConfig->gBC[i].gnPlaybackTime = 0;

		gpConfig->gBC[i].gnRfLevel_Increment = 1;

		gpConfig->gBC[i].gnTsOutput_Mode = 0;

		gpConfig->gBC[i].gnBoardStatus_ContCnt = -1;
		gpConfig->gBC[i].gnBoardDac_i_offset = 0;
		gpConfig->gBC[i].gnBoardDac_q_offset = 0;
		gpConfig->gBC[i].gnChangeModFlag = 0;
		gpConfig->gBC[i].gnBoardStatusReset = 1;
    }
    
    gUtility.MyStrCpy(gGeneral.szStatusMessage,256, "");
    gGeneral.gFactoryDefault = 0;
    gGeneral.gLCID = Application::CurrentCulture->LCID;
	gGeneral.gnApplicationRunFlag = 1;

	//2010/6/4
	gGeneral.StartInit = 0;
	//2011/4/13
	gGeneral.gnChangeAdapt_Flag = 0;
	//2011/8/18
	gGeneral.gnValid_PlayStop = 0;
	gGeneral.gnBoardIndex_autoplay = 0;

	//2012/9/26 resize & rearrange
	gGeneral.gnResizeIndex = 0;
	gGeneral.gnSetParam_Asi = 0;
    gGeneral.gnRecevingSocket = INVALID_SOCKET;
    gGeneral.gnSendingSocket = INVALID_SOCKET;
}

#else 
void CWRAP_DLL::Init_Variables()
{
    int     i, j;

	char temp_l[8];
	gUtility.MyStrCpy(gGeneral.str_board_id_info,256, (char *) "");
    for (i = 0; i <= MAX_BOARD_COUNT; i++)
    {
        gpConfig->gBC[i].bPlayingProgress = 0;             //flag for playing mode activity
        gpConfig->gBC[i].bRecordInProgress = 0;           //flag for recording mode activity
        gpConfig->gBC[i].bDelayinProgress = 0;            //flag for monitoring mode activity
        gpConfig->gBC[i].bDeleteAsked = 0;                //flag for getting the delete confirm wait SLCT/EXIT
        gpConfig->gBC[i].fCurFocus = FILELISTWINDOW;
        gpConfig->gBC[i].bForcedPlayRate = 0;

        gpConfig->gBC[i].nLastHLDThreadState = 0;         // 0 if no file playing , 0~19 during play
        gpConfig->gBC[i].gnOutputClockSource = 0;
        gpConfig->gBC[i].nWaitProgressCnt = 0;

        gpConfig->gBC[i].nFocusInput = -1;

        gGeneral.gnTSCount = 0;
        gGeneral.gnLastSec = 0;
        gGeneral.gnPlayRate[0] = 0.0;
        gGeneral.gnPlayRate[1] = 0.0;
        gGeneral.gnPlayRate[2] = 0.0;
        gGeneral.gnPlayRate[3] = 0.0;
        gGeneral.gnPlayRate[4] = 0.0;
        
		//2010/9/30
		gpConfig->gBC[i].gnBoardId = -1;

		//2011/6/24 FAULT LED fixed
		gpConfig->nBoardRealSlot[i] = -1;
 		gpConfig->nBoardSlotNum[i] = -1;
       // Get Board Id
        //----------------------------------------------------------------------------
        //20070309-NO MULTI-BOARD
        //----------------------------------------------------------------------------
		gpConfig->gBC[i].gnBoardStatus = -1;
		gpConfig->gBC[i].gnBoardId = -1;
		gpConfig->gBC[i].gnBoardRev = -1;

		//kslee 2010/1/20 TVB590S V2
		gpConfig->gBC[i].gnBoardRev = TSPL_GET_BOARD_REV_EX(i);
		//kslee 2010/1/22 TVB590S V2
		gpConfig->gBC[i].gnErrorStatus = 0;
        gpConfig->gBC[i].gnOpenSystem = 0;
        gpConfig->gBC[i].gnUseFrontInput = 0;
        gpConfig->gBC[i].gnUseDemuxblockTest = 0;

        // TVB590V9 - RF/IF AMP
        gpConfig->gBC[i].gnRFAmpUsed = TSPL_GET_BOARD_CONFIG_STATUS_EX(i);
        
        //----------------------------------------------------------------------------
        //sskim20071130 - A/V decoding, IP Streaming
        //----------------------------------------------------------------------------
        gpConfig->gBC[i].gnUseIPStreaming = 0;
        gpConfig->gBC[i].gnIPStreamingMode = NO_IP_STREAM;
        gUtility.MyStrCpy(gpConfig->gBC[i].gszIPStreamingInfo, 256,  (char *) "");
        gUtility.MyStrCpy(gpConfig->gBC[i].gszIPStreamingInputInfo, 256, (char *) "");
        gpConfig->gBC[i].gnIPSubBankOffset = 1024;
        gpConfig->gBC[i].gnUseAVDecoding = 0;
        
        gpConfig->gBC[i].gnIPTotalProgram = 0;
        gpConfig->gBC[i].gnIPCurrentProgram = -1;
        gGeneral.gnVLCRunning = -1;

        //----------------------------------------------------------------------------
        //sskim20070528 - keisoku
        //----------------------------------------------------------------------------
        gpConfig->gBC[i].gnRFOutFreqUnit  = RF_OUT_HZ;
        gpConfig->gBC[i].gnRemoveFileEnabled = 0;
        gpConfig->gBC[i].gnStartPos =  0;
        gpConfig->gBC[i].gnCurrentPos = 0;
        gpConfig->gBC[i].gnEndPos = 0;
        gpConfig->gBC[i].gnStartSliderPos = 0;
        gpConfig->gBC[i].gnEndSliderPos = 0;
                
        gpConfig->gBC[i].gnStartPosChanged = 0;
        gpConfig->gBC[i].gnCurrentPosChanged = 0;
        gpConfig->gBC[i].gnEndPosChanged = 0;
        gpConfig->gBC[i].gnCurrentPosScrolled = 0;
        
        gpConfig->gBC[i].gnStartTimeOffset = 0;
        gpConfig->gBC[i].gnEndTimeOffset = 0;
        gpConfig->gBC[i].gnOffsetMargin = 0;

        gpConfig->gBC[i].gnUseSubLoop = 0;
        gpConfig->gBC[i].gnUseSubLoop_Command = 0;
        gpConfig->gBC[i].gnMaxRFLevel = 0;
        gpConfig->gBC[i].gnMinRFLevel = 0;
        gpConfig->gBC[i].gnPrevPlayrate = 0;
        gpConfig->gBC[i].gnPrevSymbolrate = 0;
        gpConfig->gBC[i].gnRepeatCount = 0;
        
        gpConfig->gBC[i].gInvalidBitrate = 0;
        gpConfig->gBC[i].gRFPowerLevel = 0.0;
        gpConfig->gBC[i].gnVideoPosX = 0;
        gpConfig->gBC[i].gnVideoPosY = 0;
        gpConfig->gBC[i].gnVideoWidth = VLC_VIDEO_WIDTH_S;
        gpConfig->gBC[i].gnVideoHeight = VLC_VIDEO_HEIGHT_S;
        gpConfig->gBC[i].gnTAT4710 = -1;

        gpConfig->gBC[i].gnActiveMixerUsed = 0;

		gpConfig->gBC[i].gnBoardAuth = 0;
		gpConfig->gBC[i].gnIQ_play_support = 0;
		gpConfig->gBC[i].gnIQ_capture_support = 0;

		//AGC - RF Level -> Atten/AGC
		gpConfig->gBC[i].gnAGC = 0;

		//2010/5/28
		gpConfig->gBC[i].gnBitrate_Adjustment_Flag = -1;

		//2010/7/18 I/Q PLAY/CAPTURE
		gpConfig->gBC[i].gnIQ_play_support = TSPL_GET_FPGA_INFO_EX(i, 2);
		gpConfig->gBC[i].gnIQ_capture_support = TSPL_GET_FPGA_INFO_EX(i, 3);
//		gUtility.MyStrCpy(gpConfig->gBC[i].gszIQ_capture_fileName,512, (char *)"");
		gUtility.MyStrCpy(gpConfig->gBC[i].gszIQ_capture_filePath,512, (char *)"");
		gpConfig->gBC[i].gnTsOutput_Mode = 0;


		//2010/12/07 ISDB-S =======================================================================================================================
		gpConfig->gBC[i].gnSlotCount = MAX_SLOT_COUNT;

		for(j = 0 ; j < MAX_TS_COUNT ; j++)
		{
			gpConfig->gBC[i].gnConstellation_M[j] = -1;
			gpConfig->gBC[i].gnCoderate_M[j] = -1;
			gpConfig->gBC[i].gnSlotCount_M[j] = 0;
			gUtility.MyStrCpy(gpConfig->gBC[i].gszTS_M[j], 512, (char *)"");
		}
		//=========================================================================================================================================

		//----------------------------------------------------------------------------
        //sskim20080212 - SCHEDULED TASKS
        //----------------------------------------------------------------------------
        gUtility.MyStrCpy(gpConfig->gBC[i].gnCmdAutoExitTime,24, (char *) "");
        gpConfig->gBC[i].gnCmdAutoExitDone = 0;
        gUtility.MyStrCpy(gpConfig->gBC[i].gnCmdAutoExitDate,24, (char *) "");
        gpConfig->gBC[i].gnCmdDurationTime = 0;

		gUtility.MyStrCpy(gpConfig->gBC[i].gszCmmb_Mdif,256,(char *)"");
		gpConfig->gBC[i].gnCmmb_Mdif = -1;
		gpConfig->gBC[i].gnCmmb_const = -1;
		gpConfig->gBC[i].gnCmmb_rscoding = -1;
		gpConfig->gBC[i].gnCmmb_bytecrossing = -1;
		gpConfig->gBC[i].gnCmmb_ldpc = -1;
		gpConfig->gBC[i].gnCmmb_scramble = -1;
		gpConfig->gBC[i].gnCmmb_timeslice = -1;

		//2010/6/28 AUTO-PLAY
		gpConfig->gBC[i].gnAuto_Play = 0;
		gUtility.MyStrCpy(gpConfig->gBC[i].gszAuto_Play_FileName, 256, (char *) "");
		//2011/3/4 Fixed
		gpConfig->gBC[i].gnChangeModFlag = 0;
		gpConfig->gBC[i].gnDefaultSymbol = 0;
		gpConfig->gBC[i].gnCmdAutoRun = 0;

		gpConfig->gBC[i].gnDVB_C2_CreateFile = 0;
		//2011/2/23 DVB-C2 ========================================================================================================================
		gpConfig->gBC[i].gnDVB_C2_L1 = 0;
		gpConfig->gBC[i].gnDVB_C2_Guard = 0;
		gpConfig->gBC[i].gnDVB_C2_Network = 0;
		gpConfig->gBC[i].gnDVB_C2_System = 0;
		gpConfig->gBC[i].gnDVB_C2_StartFreq = 0;
		gpConfig->gBC[i].gnDVB_C2_NumNoth = 0;
		gpConfig->gBC[i].gnDVB_C2_ReservedTone = 0;
		gpConfig->gBC[i].gnDVB_C2_NotchStart = 0;
		gpConfig->gBC[i].gnDVB_C2_NotchWidth = 0;
		gpConfig->gBC[i].gnDVB_C2_Dslice_type = 0;
		gpConfig->gBC[i].gnDVB_C2_Dslice_FecHeader = 0;
		//==========================================================================================================================================
		//2011/10/24 added PAUSE
		gpConfig->gBC[i].gnPause = 0;

		//2011/11/28 TVB594
		gpConfig->gBC[i].gn_OwnerSlot = -1;
		gpConfig->gBC[i].gnRealandvir_cnt = -1;
		gpConfig->gBC[i].gnRealandvir_location = 0;
		gpConfig->gBC[i].gn_IsVirtualSlot = 0;
		gpConfig->gBC[i].gn_StreamNum = -1;
		gpConfig->gnInstalledBoard_InitMode[i] = 1;
		//2012/9/3 new rf level control
		gpConfig->gBC[i].gdRfLevelValue = 40;
		gpConfig->gBC[i].gdRfLevelRange_max = 0;
		gpConfig->gBC[i].gdRfLevelRange_min = 0;

		//2012/9/6 pcr restamp
		gpConfig->gBC[i].gnPcrReStampingFlag = 0;	//default is 0;
		//2012/9/28 BERT fix
		gpConfig->gBC[i].gnBert_Pid = 0x100;
		gpConfig->gBC[i].gnETI_Format = 2;
    }
    gUtility.MyStrCpy(gGeneral.szStatusMessage,256, (char *) "");
    gGeneral.gFactoryDefault = 0;
	gGeneral.gndownflag = 0;
#ifdef STANDALONE
	gGeneral.gnUsb_Status = 0;
	gGeneral.gnSdcard_Status = 0;
	gGeneral.gnSetSystemClock = 0;
	gGeneral.gnRunTSparser = 0;
#endif
	gGeneral.gnPIDCount = 0;
	gGeneral.gLCID = 82;
	m_Init = 1;

	gGeneral.gnApplicationRunFlag = 1;

}

#endif

//---------------------------------------------------------------------------
// Combo_MODULATOR_TYPE_Click
// -1: not changed
void CWRAP_DLL::Change_Modulator_Type(long nBoardNum, long lModType)
{
    gRegVar.SaveVariables(nBoardNum);
	gpConfig->gBC[nBoardNum].gnModulatorMode = lModType;
    gRegVar.SaveNewModulatorMode(nBoardNum, lModType);  // Save to "Startup"/"TVB380ModulationMode"
#ifndef WIN32
	Sleep(300);
	//printf("===== Restore Variables: NewModType = %d\n", (int)lModType);
#endif
    gRegVar.RestoreVariables(nBoardNum);
        
	//2011/5/4 AD9852 OVER CLOCK
#ifdef WIN32
	if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		TSPL_SET_AD9852_MAX_CLOCK_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnAD9852_Overclock);
#endif
    gUtilInd.CalcBurtBitrate(nBoardNum);
    Configure_Modulator(1);
#ifndef WIN32
	Sleep(300);
#endif
	
    TVB380_SET_STOP_MODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);
    TSPH_SET_SDRAM_BANK_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gnSubBankNumber, gpConfig->gBC[nBoardNum].gnSubBankOffset);
    //TSPH_START_MONITOR(nBoardNum, 0);
#ifndef WIN32 
	TSPH_START_MONITOR(nBoardNum, 0);
#endif
}

//---------------------------------------------------------------------------
// Called From FormClose
void CWRAP_DLL::Close_System(int nBoardNum)
{
    //-----------------------------------
    //Stop Recording/Playing
    Stop_Recording(nBoardNum);
    Stop_Playing(nBoardNum);
     
    //-----------------------------------
    // Save Parameters to Registry
    if (gGeneral.gFactoryDefault == 0)
	{
		gRegVar.SaveNewModulatorMode(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
        gRegVar.SaveVariables(nBoardNum);
	}
    //-----------------------------------
    //--- RF output disabled
    TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, FILE_SRC);
#ifdef WIN32
	if((gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2) &&
		gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
	{
		TSPH_START_MONITOR(nBoardNum, 0);
	}
#else
    Sleep(2000);
 	TSPL_RESET_SDCON_EX(nBoardNum);
    Sleep(2000);

#endif
}

//---------------------------------------------------------------------------
// Called From
// - open_modulator: init=0
// - IF Change: init = 0
// - SPECTRUM Change: init = 0
// - Modulator Type Change: init = 1
// if init == 1, download RBF
#ifdef WIN32 

void CWRAP_DLL::Configure_Modulator(int init)
{
    long dwModulationIFFreq;
    long nBoardNum = gGeneral.gnActiveBoard;
    long nModulatorMode;
    long Txmode;
    long nRFOutFreq;
    long nCodeRate;
    long nBertType;

    //==========================================================================
    // Before all Common
    //==========================================================================
    // Set Current Modulator Type
    gUtilInd.SyncModulatorMode(nBoardNum);
    nModulatorMode = gpConfig->gBC[nBoardNum].gnModulatorMode;

    TSPH_SET_MODULATOR_TYPE(nBoardNum, nModulatorMode);

    //-----------------------------------
    // IF Freq.
    if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
    {
        dwModulationIFFreq = 36000000;
    } else if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
    {
        dwModulationIFFreq = 44000000;
    } else
    {
        dwModulationIFFreq = 36125000;
    }

	if (init == 1)
        TVB380_SET_CONFIG_EX(nBoardNum, nModulatorMode, dwModulationIFFreq);

	if(nModulatorMode == ASI_OUTPUT_MODE)
	{
		TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(nBoardNum, nModulatorMode, 0); 
		return;
	}

	//TDT/TOT - USER DATE/TIME
	TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, 
		gpConfig->gBC[nBoardNum].gnDateTimeOffset, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
	

	TSPH_SET_ERROR_INJECTION(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket, 
			gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount,
			gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);


    switch (nModulatorMode)
    {
        case DVB_T:
        case DVB_H:
        case QAM_A:
        case QPSK:
        case DVB_S2:
		case CMMB:	//CMMB
		case DVB_T2:	//DVB-T2 kslee 2010/4/20
		case IQ_PLAY:	//2010/7/18 I/Q PLAY/CAPTURE
		case DVB_C2:
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
            // Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
        //2011/2/28 fixed
		case VSB_8:
        case VSB_16:
		case ATSC_MH: //ATSC-M/H kslee 2010/2/3
		case MULTIPLE_VSB:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 4500000;
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
 
		case QAM_B:
		case MULTIPLE_QAMB:
			if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
				gpConfig->gBC[nBoardNum].gnSymbolRate = 5056941;
            else
                gpConfig->gBC[nBoardNum].gnSymbolRate = 5360537;

			if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
        case TDMB:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 2048000;
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
            
        case ISDB_T:
        case ISDB_T_13:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 8126984;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
            
        case DTMB:
            // Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;

            gpConfig->gBC[nBoardNum].gnSymbolRate = 7560000;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            break;
		//2010/12/06 ISDB-S =============================================================================================================
		case ISDB_S:
			gpConfig->gBC[nBoardNum].gnSymbolRate = 28860000;
			gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
			// Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
			nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
		//===============================================================================================================================

    }
    
    TVB380_SET_MODULATOR_IF_FREQ_EX(nBoardNum, nModulatorMode, dwModulationIFFreq);
	//2012/3/20
	if((gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15) && (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH))
	{
		if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
        {
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
		else
		{
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
	}
	else
	{
		TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
	}
	if(nModulatorMode == QPSK || nModulatorMode == DVB_S2 || nModulatorMode == QAM_A || nModulatorMode == QAM_B || nModulatorMode == IQ_PLAY || nModulatorMode == MULTIPLE_QAMB)   //TVB590S - gnBandwidth(gnActiveBoard) -> gnSymbolRate(gnActiveBoard)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
	}
	else
	{
		if(nModulatorMode == DVB_T2)
		{
			if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN || gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth + 4);
				else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth + 2);
				else
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth - 2);
			}
			else if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnT2MI_BW + 4);
				else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnT2MI_BW + 2);
				else
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnT2MI_BW - 2);
			}
			else
			{
				if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 0)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 4);
				else if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 1)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 2);
				else
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW - 2);
			}
		}
		else if(nModulatorMode == DVB_H)
		{
			if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)		
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth + 3);
			else
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth - 1);
		}
		else
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);

	}
    //==========================================================================
    // Modulator Specific
    //==========================================================================
    switch (nModulatorMode)
    {
        case DVB_T:
        case DVB_H:         // Set bandwidth, TxMode, DVB-H mode, coderate, guardinterval, condtellation
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
            if(nModulatorMode == DVB_H)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth + 3, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth - 1, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			}
			else
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
            if (nModulatorMode == DVB_H)
            {
                Txmode = gpConfig->gBC[nBoardNum].gnTxmode;
                if (gpConfig->gBC[nBoardNum].gnTxmode == TX_4K)
                    Txmode = 1;
                else if (gpConfig->gBC[nBoardNum].gnTxmode == TX_8K)
                    Txmode = 2;
                       
                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, nModulatorMode, Txmode, gpConfig->gBC[nBoardNum].gnIn_Depth,
                        gpConfig->gBC[nBoardNum].gnTime_Slice, gpConfig->gBC[nBoardNum].gnMPE_FEC, gpConfig->gBC[nBoardNum].gnCell_Id);
            }
            else
            {
				if(gpConfig->gBC[nBoardNum].gnBoardId == 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
					gpConfig->gBC[nBoardNum].gnBoardId == 21 || gpConfig->gBC[nBoardNum].gnBoardId == 22 || gpConfig->gBC[nBoardNum].gnBoardId == 15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
	                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode,
		                    0,
			                0,
				            0,
					        gpConfig->gBC[nBoardNum].gnCell_Id);
				}
				else
					TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
            break;

        case ISDB_T:
        case ISDB_T_13:     // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case VSB_8:
        case VSB_16:    // Set Symbolrate
		case ATSC_MH:	//ATSC-M/H kslee 2010/2/3
		case MULTIPLE_VSB:
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case QAM_A:     // Set Symbolrate, constellation
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;

        case QAM_B:     // Set Symborate, constellation, interleave
		case MULTIPLE_QAMB:
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMMode);
            TVB380_SET_MODULATOR_INTERLEAVE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;

        case QPSK:      // Set Symbolrate, coderate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;

        case TDMB:      // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case DVB_S2:    // Set Symbolrate, Coderate, contellation
            //TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);

            // 1/4, 1/3, 2/5 added
            if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate;
            } else
            {
                if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
                {
                    if (gpConfig->gBC[nBoardNum].gnCodeRate >= 3)
                        nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;
                    else
                        nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 4;
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
                {
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;        // 5(2/3),...,10(9/10)
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 6;        // 6(3/4),...,10(9/10)
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, nCodeRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
            break;

        case DTMB:      // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            Set_DTMB_Parameters(nBoardNum);
            break;
		case CMMB:
			TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);
			TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
			TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
			TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
			break;
		case DVB_T2:
			if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN || gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			}
			else if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			}
			else
			{
				if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 1)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnIP_T2MI_BW - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			}
			break;
		//2010/7/18 I/Q PLAY/CAPTURE
		case IQ_PLAY:
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			break;
		//2010/12/07 ISDB-S ========================================================================================================================
		case ISDB_S:
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			break;
		//==========================================================================================================================================
		//2011/2/25 DVB-C2 =========================================================================================================================
		case DVB_C2:
			TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			break;
		//==========================================================================================================================================

    }

    //==========================================================================
    // After all Common
    //==========================================================================
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
		//2011/11/30
        if ((gpConfig->gBC[nBoardNum].gnBoardId >= 47 && gpConfig->gBC[nBoardNum].gnBoardId != _TVB594_BD_ID_) || gpConfig->gBC[nBoardNum].gnBoardId == 10 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			//2012/8/31 new rf level control
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
				TVB380_SET_BOARD_CONFIG_STATUS_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBypassAMP);
		}

		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
			TVB380_SET_MODULATOR_ATTEN_VALUE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnUseTAT4710);	//2011/6/29 added UseTAT4710
        
		if (nModulatorMode == TDMB)
        {
            TVB380_SET_MODULATOR_PRBS_MODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSmode);
            TVB380_SET_MODULATOR_PRBS_SCALE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSscale);
        } else
        {
            TVB380_SET_MODULATOR_PRBS_INFO_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSmode, gpConfig->gBC[nBoardNum].gnPRBSscale);

            // TVB390V8
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
                SetStreamSourcePort(gpConfig->gBC[nBoardNum].gnModulatorSource);
			}
        }
    }

    switch (nModulatorMode)
    {
        case ISDB_T:
        case ISDB_T_13:
			TSPH_SET_TMCC_REMUXER(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
            break;
            
        case QPSK:
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
            {
                // 6.9.12 - must be called before freq./symbol setting ???
                if (gpConfig->gBC[nBoardNum].gnRollOffFactor != RRC_FILTER_OFF)
                    gpConfig->gBC[nBoardNum].gnRollOffFactor = RRC_FILTER_ON;
                TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRollOffFactor);
				if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);

            }
            break;
            
        case DVB_S2:
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
            {
                // PILOT OFF(0), ON(1) -> PILOT OFF(1), ON(0)
                TVB380_SET_MODULATOR_PILOT_EX(nBoardNum, nModulatorMode, (gpConfig->gBC[nBoardNum].gnPilot + 1) % 2);
                TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRollOffFactor);
				if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            }
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;
		//2010/5/28 DVB-T2
		case DVB_T2:
			long PLP_COD;
			long t2_bandwidth;
			if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 4;
			else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 2;
			else
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW - 2;
			//2010/12/21 DVB-T2 MULTI-PLP ========================================================================================
			for(int i = 0 ; i < MAX_PLP_TS_COUNT ; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0)
				{
					PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[i];
					if(PLP_COD == 1)
						PLP_COD = 2;
					else if(PLP_COD == 2)
						PLP_COD = 3;
					else if(PLP_COD == 3)
						PLP_COD = 1;
		
					TSPH_SET_T2MI_PARAMS(nBoardNum, 
									t2_bandwidth,
									gpConfig->gBC[nBoardNum].gnT2MI_FFT,
									gpConfig->gBC[nBoardNum].gnT2MI_GUARD,
									gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD,
									gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN,
									gpConfig->gBC[nBoardNum].gnT2MI_BWT,
									nRFOutFreq,
									gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID,
									gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID,
									gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID,
									0,
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[i],
									PLP_COD,
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[i],
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[i],
									gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME,
									gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL,
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[i],
									gpConfig->gBC[nBoardNum].gnT2MI_PID,
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[i],
									i,
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i],
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[i],
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[i],
									gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i],
									gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[i]);
				}
			}
			
			//====================================================================================================================
			break;
		//2010/12/07 ISDB-S ========================================================================================================================
		case ISDB_S:
			int ts_num;
			//2011/4/4 ISDB-S Bitrate
			__int64 dwSize;
			dwSize = 0;
			int ISDBS_runTime;
			int ISDBS_runTime_old;
			ISDBS_runTime = 0;
			ISDBS_runTime_old = 0;
			//-----------------------
			
			TSPH_SET_TMCC_REMUXER(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
            ts_num = 0;
			for(int i = 0 ; i < MAX_TS_COUNT ; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[i]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[i] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[i] <= MAX_SLOT_COUNT)
				{
					TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, gpConfig->gBC[nBoardNum].gszTS_M[i], gpConfig->gBC[nBoardNum].gnConstellation_M[i],
						gpConfig->gBC[nBoardNum].gnCoderate_M[i], gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
					//2011/4/4 ISDB-S Bitrate
					dwSize = gUtilInd.Get_File_Size_BYTE(gpConfig->gBC[nBoardNum].gszTS_M[i]);
					ISDBS_runTime = (int)(dwSize*8/gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i]);
					if(ISDBS_runTime > ISDBS_runTime_old)
					{
						gpConfig->gBC[nBoardNum].dwFileSize = dwSize;
						gpConfig->gBC[nBoardNum].gnPlaybackTime = ISDBS_runTime;
						ISDBS_runTime_old = ISDBS_runTime;
						gpConfig->gBC[nBoardNum].gnISDBS_BaseBitrate = gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i];
						gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS, 512, gpConfig->gBC[nBoardNum].gszTS_M[i]);
				
					}
					//========================
					ts_num = ts_num + 1;
				}
			}

			if(ts_num == 0)
			{
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS, 512, "");			
				TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, "", -1, -1, 0);
			}
			break;
		//==========================================================================================================================================

		//2011/2/23 DVB-C2 =========================================================================================================================
		case DVB_C2:
			for(int i = 0; i < DVB_C2_MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					TSPH_SET_C2MI_PARAMS(
						nBoardNum, 
						gpConfig->gBC[nBoardNum].gnBandwidth,
						gpConfig->gBC[nBoardNum].gnDVB_C2_L1,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Guard,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Network,
						gpConfig->gBC[nBoardNum].gnDVB_C2_System,
						gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq,
						gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth,
						gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone,
						gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart,
						(gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth + 1),
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_BBHeader[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft,
						(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[i] + 1),
						(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[i] + 1),
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[i],
						i,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_CreateFile
						);
				}
			}
			break;
		//==========================================================================================================================================
	}
	//2010/8/31 FIXED - IF +- offset
	long IF_offset;
	
	//2011/11/30 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnCurrentIF;
	}

	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 44000000;
	else
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36125000;

	//2010/7/18 I/Q PLAY/CAPTURE
	gpConfig->gBC[nBoardNum].gnIQ_play_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 2);
	gpConfig->gBC[nBoardNum].gnIQ_capture_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 3);
	//single tone
	gpConfig->gBC[nBoardNum].gnSingleToneEnabled = TSPL_GET_FPGA_INFO_EX(nBoardNum, 6);

	//TS packet count control
	gpConfig->gBC[nBoardNum].gnSupport_ts_Pkt_cnt_cntl = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 8);
	
	//RBF revision
	gpConfig->gBC[nBoardNum].gnSupport_Rbf_revision = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 9);

	if(nModulatorMode == IQ_PLAY || (/*gpConfig->gBC[nBoardNum].gnIQ_play_support == 1 && */gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0)))
		TSPH_SET_IQ_MODE(nBoardNum, gpConfig->gBC[nBoardNum].gnIQ_mode, gpConfig->gBC[nBoardNum].gnIQ_mem_use, gpConfig->gBC[nBoardNum].gnIQ_mem_size, gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);

    //==========================================================================
    // Set RF Power Leve, BERT
    //==========================================================================
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
		{
			FormCollection^ forms;
			forms = Application::OpenForms;
	
			for(int i=0; i < forms->Count; i++)
			{
				if(forms[i]->Name == "PlayForm")
				{
					safe_cast<PlayForm^>(forms[i])->UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
					break;
				}
			}
		}
        if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			TSPL_SET_MAX_PLAYRATE_EX(nBoardNum, 0, gpConfig->gBC[nBoardNum].gnOutputClockSource); 
    }
    
	if(gpConfig->gBC[nBoardNum].gnInputSource == PRBS_DATA_IN)
		nBertType = gpConfig->gBC[nBoardNum].gnBertPacketType + 5;
	else
		nBertType = 0;

	TVB380_SET_MODULATOR_BERT_MEASURE_EX(nBoardNum, nModulatorMode, nBertType, gpConfig->gBC[nBoardNum].gnBert_Pid);

	int iUseSystemClock = 0;
	if(gpConfig->gBC[nBoardNum].gnBoardId == 27 )
	{
		iUseSystemClock = 4;
	}
    if(nModulatorMode == ISDB_T_13)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId >= 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			iUseSystemClock = 1;
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 27 )
			iUseSystemClock = 6;
	}
	else if (nModulatorMode == DTMB)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			iUseSystemClock = 1;
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 27 )
			iUseSystemClock = 6;
	}
	//2010/11/12 DVB-T2 - Multi PLP
	else if(nModulatorMode == DVB_T2)
	{
		iUseSystemClock = 1;
		if(gpConfig->gBC[nBoardNum].gnBoardId == 27 )
			iUseSystemClock = 6;
	}
	//2011/2/23 DVB-C2
	else if(nModulatorMode == DVB_C2)
	{
		iUseSystemClock = 1;
		if(gpConfig->gBC[nBoardNum].gnBoardId == 27 )
			iUseSystemClock = 6;
	}
	TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(nBoardNum, nModulatorMode, iUseSystemClock);  //use system clock

	//2010/2/11	DAC - Offset, Gain
	if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB599
	{
		long nGain;
		int value;
		int reg1, reg2;
		long nDirection,  nOffsetB9_B2, nOffsetB1_B0;
		int freqIndex;
		//2011/5/30 DAC FREQ RANGE
		if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || (gpConfig->gBC[nBoardNum].gnBoardId == 15 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) ||
			(gpConfig->gBC[nBoardNum].gnBoardId == 20 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x3) || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB599
		{
			freqIndex = gUtilInd.getFreqIndex();
			if(freqIndex < 0)
			{
				freqIndex = 0;
			}
			int offset;
			value = gpConfig->gBC[nBoardNum].gDAC_I_Offset[freqIndex];
			if(gpConfig->gBC[nBoardNum].gnBoardId == 11)
			{
				//2012/9/18
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x9)
				{
					offset = gDacValue_TVB597_V_4_x[freqIndex][2];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB597_V_3_x[freqIndex][2];
				}
				else
				{
					offset = gDacValue_TVB597_V_2_x[freqIndex][2];
				}
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 15)
			{
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x8)
				{
					offset = gDacValue_TVB593_V_4_x[freqIndex][2];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB593_V_3_x[freqIndex][2];
				}
				else
				{
					offset = gDacValue_TVB593_V_2_x[freqIndex][2];
				}
			}
			//2012/2/6 TVB591S
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 0x16)
			{
				//2012/7/23 TVB591S V2
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
					offset = gDacValue_TVB591S_V_2_x[freqIndex][2];
				else
					offset = gDacValue_TVB591S_V_1_x[freqIndex][2];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)	//2013/6/5 TVB599
			{
				offset = gDacValue_TVB599[freqIndex][2];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB598
			{
				offset = gDacValue_TVB599[freqIndex][2];
			}
			else
			{
				offset = gDacValue_TVB590S_V_3_x[freqIndex][2];
			}
			value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_i_offset;
		}
		else
		{
			value = gpConfig->gBC[nBoardNum].gDAC_I_Offset[0];
		}

		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

		value = Math::Abs(value);

		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x8, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x7, reg2);

		//2011/5/30 DAC FREQ RANGE
		if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || (gpConfig->gBC[nBoardNum].gnBoardId == 15 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) ||
			(gpConfig->gBC[nBoardNum].gnBoardId == 20 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x3) || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB599
		{
			freqIndex = gUtilInd.getFreqIndex();
			if(freqIndex < 0)
			{
				freqIndex = 0;
			}
			int offset;
			value = gpConfig->gBC[nBoardNum].gDAC_Q_Offset[freqIndex];
			if(gpConfig->gBC[nBoardNum].gnBoardId == 11)
			{
				//2012/9/18
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x9)
				{
					offset = gDacValue_TVB597_V_4_x[freqIndex][3];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB597_V_3_x[freqIndex][3];
				}
				else
				{
					offset = gDacValue_TVB597_V_2_x[freqIndex][3];
				}
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 15)
			{
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x8)
				{
					offset = gDacValue_TVB593_V_4_x[freqIndex][3];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB593_V_3_x[freqIndex][3];
				}
				else
				{
					offset = gDacValue_TVB593_V_2_x[freqIndex][3];
				}
			}
			//2012/2/6 TVB591S
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 0x16)
			{
				//2012/7/23 TVB591S V2
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
					offset = gDacValue_TVB591S_V_2_x[freqIndex][3];
				else
					offset = gDacValue_TVB591S_V_1_x[freqIndex][3];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)	//2013/6/5 TVB599
			{
				offset = gDacValue_TVB599[freqIndex][3];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB598
			{
				offset = gDacValue_TVB599[freqIndex][3];
			}
			else
			{
				offset = gDacValue_TVB590S_V_3_x[freqIndex][3];
			}
			value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_q_offset;
		}
		else
		{
			value = gpConfig->gBC[nBoardNum].gDAC_Q_Offset[0];
		}

		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

		value = Math::Abs(value);

		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xC, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xB, reg2);
		//2010/12/10 TVB593V2 ==========================================================================
		if((gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 2) || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2012/1/31 TVB591S //2011/2/15 added 11(TVB597V2)
		{
			;
		}
		else
		{
		//==============================================================================================
			//I Gain
			nGain = 0;
			TSPL_SET_AD9775_EX(nBoardNum, 0x5, nGain);

			//Q Gain
			nGain = 0;
			TSPL_SET_AD9775_EX(nBoardNum, 0x9, nGain);
		}
	}
	
}
#else 

void CWRAP_DLL::Configure_Modulator(int init)
{
    long dwModulationIFFreq;
    long nBoardNum = gGeneral.gnActiveBoard;
    long nModulatorMode;
    long Txmode;
    long nRFOutFreq;
    long nCodeRate;
    long nBertType;

    //==========================================================================
    // Before all Common
    //==========================================================================
    // Set Current Modulator Type
    gUtilInd.SyncModulatorMode(nBoardNum);
    nModulatorMode = gpConfig->gBC[nBoardNum].gnModulatorMode;

    TSPH_SET_MODULATOR_TYPE(nBoardNum, nModulatorMode);

	//TDT/TOT - USER DATE/TIME
	TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, 
		gpConfig->gBC[nBoardNum].gnDateTimeOffset, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
	
    //-----------------------------------
    // IF Freq.
    if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
    {
        dwModulationIFFreq = 36000000;
    } else if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
    {
        dwModulationIFFreq = 44000000;
    } else
    {
        dwModulationIFFreq = 36125000;
    }

	TSPH_SET_ERROR_INJECTION(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket, 
			gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount,
			gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);

    if (init == 1)
    {
        m_Init = TVB380_SET_CONFIG_EX(nBoardNum, nModulatorMode, dwModulationIFFreq);

		if ( m_Init == -1 )
		{
			printf("Fail reset\n");
			return;
		}		
    }

	if(nModulatorMode == ASI_OUTPUT_MODE)
	{
		TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(nBoardNum, nModulatorMode, 0); 
		SetStreamSourcePort(gpConfig->gBC[nBoardNum].gnModulatorSource);
		return;
	}


    switch (nModulatorMode)
    {
        case DVB_T:
        case DVB_H:
        case QAM_A:
        case QPSK:
        case DVB_S2:
		case CMMB:	//CMMB
		case DVB_T2:	//DVB-T2 kslee 2010/4/20
		case IQ_PLAY:	//2010/7/18 I/Q PLAY/CAPTURE
		case DVB_C2:
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
            // Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
        //2011/2/28 fixed
		case VSB_8:
        case VSB_16:
		case ATSC_MH: //ATSC-M/H kslee 2010/2/3
		case MULTIPLE_VSB:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 4500000;
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
 
		case QAM_B:
		case MULTIPLE_QAMB:
			if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
				gpConfig->gBC[nBoardNum].gnSymbolRate = 5056941;
            else
                gpConfig->gBC[nBoardNum].gnSymbolRate = 5360537;

			if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
        case TDMB:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 2048000;
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
            
        case ISDB_T:
        case ISDB_T_13:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 8126984;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
            break;
            
        case DTMB:
            // Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
            if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000;
            else
                nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;

            gpConfig->gBC[nBoardNum].gnSymbolRate = 7560000;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            break;
		//2010/12/06 ISDB-S =============================================================================================================
		case ISDB_S:
			gpConfig->gBC[nBoardNum].gnSymbolRate = 28860000;
			gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
			// Freq. Policy : 1==NTSC Carrier, RF = USER RF + 1.750MHz
			nRFOutFreq = gpConfig->gBC[nBoardNum].gnRFOutFreq;
		//===============================================================================================================================

    }
    
    TVB380_SET_MODULATOR_IF_FREQ_EX(nBoardNum, nModulatorMode, dwModulationIFFreq);
	//2012/3/20
	if((gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15) && (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH))
	{
		if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
        {
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
		else
		{
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
	}
	else
	{
		TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
	}
	if(nModulatorMode == QPSK || nModulatorMode == DVB_S2 || nModulatorMode == QAM_A || nModulatorMode == QAM_B || nModulatorMode == IQ_PLAY || nModulatorMode == MULTIPLE_QAMB)   //TVB590S - gnBandwidth(gnActiveBoard) -> gnSymbolRate(gnActiveBoard)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
	}
	else
	{
		if(nModulatorMode == DVB_T2)
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnT2MI_BW);
		//2011/2/28 DVB-C2
		else if(nModulatorMode == DVB_C2)
		{
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
		}
		else
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, nModulatorMode, nRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);

	}
    //==========================================================================
    // Modulator Specific
    //==========================================================================
    switch (nModulatorMode)
    {
        case DVB_T:
        case DVB_H:         // Set bandwidth, TxMode, DVB-H mode, coderate, guardinterval, condtellation
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
            if(nModulatorMode == DVB_H)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth + 3, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth - 1, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			}
			else
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
            if (nModulatorMode == DVB_H)
            {
                Txmode = gpConfig->gBC[nBoardNum].gnTxmode;
                if (gpConfig->gBC[nBoardNum].gnTxmode == TX_4K)
                    Txmode = 1;
                else if (gpConfig->gBC[nBoardNum].gnTxmode == TX_8K)
                    Txmode = 2;
                       
                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, nModulatorMode, Txmode, gpConfig->gBC[nBoardNum].gnIn_Depth,
                        gpConfig->gBC[nBoardNum].gnTime_Slice, gpConfig->gBC[nBoardNum].gnMPE_FEC, gpConfig->gBC[nBoardNum].gnCell_Id);
            }
            else
            {
				if(gpConfig->gBC[nBoardNum].gnBoardId == 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
					gpConfig->gBC[nBoardNum].gnBoardId == 21 || gpConfig->gBC[nBoardNum].gnBoardId == 22 || gpConfig->gBC[nBoardNum].gnBoardId == 15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
	                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode,
		                    0,
			                0,
				            0,
					        gpConfig->gBC[nBoardNum].gnCell_Id);
				}
				else
					TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
            break;

        case ISDB_T:
        case ISDB_T_13:     // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case VSB_8:
        case VSB_16:    // Set Symbolrate
		case ATSC_MH:	//ATSC-M/H kslee 2010/2/3
 		case MULTIPLE_VSB:
           TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case QAM_A:     // Set Symbolrate, constellation
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;

        case QAM_B:     // Set Symborate, constellation, interleave
 		case MULTIPLE_QAMB:
           TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMMode);
            TVB380_SET_MODULATOR_INTERLEAVE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;

        case QPSK:      // Set Symbolrate, coderate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;

        case TDMB:      // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

        case DVB_S2:    // Set Symbolrate, Coderate, contellation
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);

            // 1/4, 1/3, 2/5 added
            if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate;
            } else
            {
                if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
                {
                    if (gpConfig->gBC[nBoardNum].gnCodeRate >= 3)
                        nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;
                    else
                        nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 4;
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
                {
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;        // 5(2/3),...,10(9/10)
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 6;        // 6(3/4),...,10(9/10)
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, nCodeRate);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
            break;

        case DTMB:      // Set Symbolrate
            TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
            Set_DTMB_Parameters(nBoardNum);
            break;
		case CMMB:
			TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);
			TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
			TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
			TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
			break;
		//DVB-T2 kslee 2010/4/20
		case DVB_T2:
			if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			else
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnT2MI_BW - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			break;
		//2010/7/18 I/Q PLAY/CAPTURE
		case IQ_PLAY:
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			break;
		//2010/12/07 ISDB-S ========================================================================================================================
		case ISDB_S:
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			break;
		//==========================================================================================================================================
		//2011/2/25 DVB-C2 =========================================================================================================================
		case DVB_C2:
			TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,nModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			break;
		//==========================================================================================================================================

    }

    //==========================================================================
    // After all Common
    //==========================================================================
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
		//2011/11/30
        if ((gpConfig->gBC[nBoardNum].gnBoardId >= 47 && gpConfig->gBC[nBoardNum].gnBoardId != _TVB594_BD_ID_) || gpConfig->gBC[nBoardNum].gnBoardId == 10 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			//2012/8/31 new rf level control
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
				TVB380_SET_BOARD_CONFIG_STATUS_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnBypassAMP);
		}
		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
			TVB380_SET_MODULATOR_ATTEN_VALUE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnUseTAT4710);	//2011/6/29 added UseTAT4710

		if (nModulatorMode == TDMB)
        {
            TVB380_SET_MODULATOR_PRBS_MODE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSmode);
            TVB380_SET_MODULATOR_PRBS_SCALE_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSscale);
        } else
        {
            TVB380_SET_MODULATOR_PRBS_INFO_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnPRBSmode, gpConfig->gBC[nBoardNum].gnPRBSscale);

            // TVB390V8
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
                SetStreamSourcePort(gpConfig->gBC[nBoardNum].gnModulatorSource);
				if(gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC)
				{	TSPH_CLEAR_REMUX_INFO(nBoardNum);
					TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(nBoardNum);
					gpConfig->gBC[nBoardNum].gnIsdbT_LoopThru_flag = 0;
				}
			}
        }
    }

    switch (nModulatorMode)
    {
        case ISDB_T:
        case ISDB_T_13:
			TSPH_SET_TMCC_REMUXER(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
            break;
            
        case QPSK:
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
            {
                // 6.9.12 - must be called before freq./symbol setting ???
                if (gpConfig->gBC[nBoardNum].gnRollOffFactor != RRC_FILTER_OFF)
                    gpConfig->gBC[nBoardNum].gnRollOffFactor = RRC_FILTER_ON;
                TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRollOffFactor);
				if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);

            }
            break;
            
        case DVB_S2:
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
            {
                // PILOT OFF(0), ON(1) -> PILOT OFF(1), ON(0)
                TVB380_SET_MODULATOR_PILOT_EX(nBoardNum, nModulatorMode, (gpConfig->gBC[nBoardNum].gnPilot + 1) % 2);
                TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnRollOffFactor);
				if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, nModulatorMode, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            }
            break;
		case DVB_T2:
			long PLP_COD;
			long t2_bandwidth;
			if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 4;
			else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 2;
			else
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW - 2;
			
			if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
			{
				PLP_COD = gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD;
				if(PLP_COD == 1)
					PLP_COD = 2;
				else if(PLP_COD == 2)
					PLP_COD = 3;
				else if(PLP_COD == 3)
					PLP_COD = 1;

				TSPH_SET_T2MI_PARAMS(nBoardNum, 
								t2_bandwidth,
								gpConfig->gBC[nBoardNum].gnT2MI_FFT,
								gpConfig->gBC[nBoardNum].gnT2MI_GUARD,
								gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD,
								gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN,
								gpConfig->gBC[nBoardNum].gnT2MI_BWT,
								gpConfig->gBC[nBoardNum].gnRFOutFreq,
								gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID,
								gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID,
								gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID,
								0,
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD,
								PLP_COD,
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE,
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM,
								gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME,
								gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL,
								20,
								gpConfig->gBC[nBoardNum].gnT2MI_PID,
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation,
								0,
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID,
								1,
								0,
								(char *)"INPUT SOURCE [IP]",
								gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Time_Interleave);
			}
			else
			{
				//2010/12/21 DVB-T2 MULTI-PLP ========================================================================================
				for(int i = 0 ; i < MAX_PLP_TS_COUNT ; i++)
				{
					if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0)
					{
						PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[i];
						if(PLP_COD == 1)
							PLP_COD = 2;
						else if(PLP_COD == 2)
							PLP_COD = 3;
						else if(PLP_COD == 3)
							PLP_COD = 1;
			
						TSPH_SET_T2MI_PARAMS(nBoardNum, 
										t2_bandwidth,
										gpConfig->gBC[nBoardNum].gnT2MI_FFT,
										gpConfig->gBC[nBoardNum].gnT2MI_GUARD,
										gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD,
										gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN,
										gpConfig->gBC[nBoardNum].gnT2MI_BWT,
										nRFOutFreq,
										gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID,
										gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID,
										gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID,
										0,
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[i],
										PLP_COD,
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[i],
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[i],
										gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME,
										gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL,
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[i],
										gpConfig->gBC[nBoardNum].gnT2MI_PID,
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[i],
										i,
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i],
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[i],
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[i],
										gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i],
										gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[i]);
					}
				}
			}
			//====================================================================================================================
			break;
		//2010/12/07 ISDB-S ========================================================================================================================
		case ISDB_S:
			int ts_num;
			//2011/4/4 ISDB-S Bitrate
			__int64 dwSize;
			dwSize = 0;
			int ISDBS_runTime;
			int ISDBS_runTime_old;
			ISDBS_runTime = 0;
			ISDBS_runTime_old = 0;
			//-----------------------
			
			TSPH_SET_TMCC_REMUXER(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
            ts_num = 0;
			for(int i = 0 ; i < MAX_TS_COUNT ; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[i]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[i] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[i] <= MAX_SLOT_COUNT)
				{
					TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, gpConfig->gBC[nBoardNum].gszTS_M[i], gpConfig->gBC[nBoardNum].gnConstellation_M[i],
						gpConfig->gBC[nBoardNum].gnCoderate_M[i], gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
					//2011/4/4 ISDB-S Bitrate
					dwSize = gUtilInd.Get_File_Size_BYTE(gpConfig->gBC[nBoardNum].gszTS_M[i]);
					ISDBS_runTime = (int)(dwSize*8/gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i]);
					if(ISDBS_runTime > ISDBS_runTime_old)
					{
						gpConfig->gBC[nBoardNum].dwFileSize = dwSize;
						gGeneral.gnRunTime = ISDBS_runTime;
						ISDBS_runTime_old = ISDBS_runTime;
						gpConfig->gBC[nBoardNum].gnISDBS_BaseBitrate = gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i];
						gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS, 512, gpConfig->gBC[nBoardNum].gszTS_M[i]);
				
					}
					//========================
					ts_num = ts_num + 1;
				}
			}

			if(ts_num == 0)
			{
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS, 512, (char *)"");			
				TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, (char *)"", -1, -1, 0);
			}
			break;
		//==========================================================================================================================================

		//2011/2/23 DVB-C2 =========================================================================================================================
		case DVB_C2:
			for(int i = 0; i < DVB_C2_MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					TSPH_SET_C2MI_PARAMS(
						nBoardNum, 
						gpConfig->gBC[nBoardNum].gnBandwidth,
						gpConfig->gBC[nBoardNum].gnDVB_C2_L1,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Guard,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Network,
						gpConfig->gBC[nBoardNum].gnDVB_C2_System,
						gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq,
						gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth,
						gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone,
						gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart,
						(gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth + 1),
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader,
						gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft,
						(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[i] + 1),
						(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[i] + 1),
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[i],
						i,
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i],
						gpConfig->gBC[nBoardNum].gnDVB_C2_CreateFile
						);
				}
			}
			break;
		//==========================================================================================================================================
	}
	//2010/8/31 FIXED - IF +- offset
	long IF_offset;
	
	//2011/11/30 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnCurrentIF;
	}

	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 44000000;
	else
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36125000;

	//2010/7/18 I/Q PLAY/CAPTURE
	gpConfig->gBC[nBoardNum].gnIQ_play_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 2);
	gpConfig->gBC[nBoardNum].gnIQ_capture_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 3);
	//single tone
	gpConfig->gBC[nBoardNum].gnSingleToneEnabled = TSPL_GET_FPGA_INFO_EX(nBoardNum, 6);

	if(nModulatorMode == IQ_PLAY || (/*gpConfig->gBC[nBoardNum].gnIQ_play_support == 1 && */gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0)))
		TSPH_SET_IQ_MODE(nBoardNum, gpConfig->gBC[nBoardNum].gnIQ_mode, gpConfig->gBC[nBoardNum].gnIQ_mem_use, gpConfig->gBC[nBoardNum].gnIQ_mem_size, gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);

    //==========================================================================
    // Set RF Power Leve, BERT
    //==========================================================================
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {

		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
		{
			//AGC - RF Level -> Atten/AGC
			UpdateRFPowerLevel(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
		}

        if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			TSPL_SET_MAX_PLAYRATE_EX(nBoardNum, 0, gpConfig->gBC[nBoardNum].gnOutputClockSource); 
    }
        
    nBertType = gpConfig->gBC[nBoardNum].gnBertPacketType;

    TVB380_SET_MODULATOR_BERT_MEASURE_EX(nBoardNum, nModulatorMode, nBertType, gpConfig->gBC[nBoardNum].gnBert_Pid);

	int iUseSystemClock = 0;
    if(nModulatorMode == ISDB_T_13)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId >= 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			iUseSystemClock = 1;
	}
	else if (nModulatorMode == DTMB)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			iUseSystemClock = 1;
	}
	//2010/11/12 DVB-T2 - Multi PLP
	else if(nModulatorMode == DVB_T2)
	{
		iUseSystemClock = 1;
	}
	//2011/2/23 DVB-C2
	else if(nModulatorMode == DVB_C2)
	{
		iUseSystemClock = 1;
	}
	TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(nBoardNum, nModulatorMode, iUseSystemClock);  //use system clock

	//2010/2/11	DAC - Offset, Gain
	if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2012/1/31 TVB591S //2011/2/15 added 11(TVB597V2)	//2010/10/5
	{
		long nGain;
		int value;
		int reg1, reg2;
		long nDirection,  nOffsetB9_B2, nOffsetB1_B0;
		int freqIndex;
		//2011/5/30 DAC FREQ RANGE
		if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || (gpConfig->gBC[nBoardNum].gnBoardId == 15 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) ||
			(gpConfig->gBC[nBoardNum].gnBoardId == 20 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x3) || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	/* 2012/1/31 TVB591S */ 
		{
			freqIndex = gUtilInd.getFreqIndex();
			if(freqIndex < 0)
			{
				freqIndex = 0;
			}
			int offset;
			value = gpConfig->gBC[nBoardNum].gDAC_I_Offset[freqIndex];
			if(gpConfig->gBC[nBoardNum].gnBoardId == 11)
			{
				//2012/9/18
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x9)
				{
					offset = gDacValue_TVB597_V_4_x[freqIndex][2];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB597_V_3_x[freqIndex][2];
				}
				else
				{
					offset = gDacValue_TVB597_V_2_x[freqIndex][2];
				}
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 15)
			{
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x8)
				{
					offset = gDacValue_TVB593_V_4_x[freqIndex][2];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB593_V_3_x[freqIndex][2];
				}
				else
				{
					offset = gDacValue_TVB593_V_2_x[freqIndex][2];
				}
			}
			//2012/2/6 TVB591S
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 0x16)
			{
				//2012/7/23 TVB591S V2
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
					offset = gDacValue_TVB591S_V_2_x[freqIndex][2];
				else
					offset = gDacValue_TVB591S_V_1_x[freqIndex][2];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)	//2013/6/5 TVB599
			{
				offset = gDacValue_TVB599[freqIndex][2];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB598
			{
				offset = gDacValue_TVB599[freqIndex][2];
			}
			else
			{
				offset = gDacValue_TVB590S_V_3_x[freqIndex][2];
			}
			value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_i_offset;
		}
		else
		{
			value = gpConfig->gBC[nBoardNum].gDAC_I_Offset[0];
		}

		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

		value = abs(value);

		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x8, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x7, reg2);

		//2011/5/30 DAC FREQ RANGE
		if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || (gpConfig->gBC[nBoardNum].gnBoardId == 15 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) ||
			(gpConfig->gBC[nBoardNum].gnBoardId == 20 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x3) || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	/* 2012/1/31 TVB591S */ 
		{
			freqIndex = gUtilInd.getFreqIndex();
			if(freqIndex < 0)
			{
				freqIndex = 0;
			}
			int offset;
			value = gpConfig->gBC[nBoardNum].gDAC_Q_Offset[freqIndex];
			if(gpConfig->gBC[nBoardNum].gnBoardId == 11)
			{
				//2012/9/18
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x9)
				{
					offset = gDacValue_TVB597_V_4_x[freqIndex][3];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB597_V_3_x[freqIndex][3];
				}
				else
				{
					offset = gDacValue_TVB597_V_2_x[freqIndex][3];
				}
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 15)
			{
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x8)
				{
					offset = gDacValue_TVB593_V_4_x[freqIndex][3];
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
				{
					offset = gDacValue_TVB593_V_3_x[freqIndex][3];
				}
				else
				{
					offset = gDacValue_TVB593_V_2_x[freqIndex][3];
				}
			}
			//2012/2/6 TVB591S
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 0x16)
			{
				//2012/7/23 TVB591S V2
				if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
					offset = gDacValue_TVB591S_V_2_x[freqIndex][3];
				else
					offset = gDacValue_TVB591S_V_1_x[freqIndex][3];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)	//2013/6/5 TVB599
			{
				offset = gDacValue_TVB599[freqIndex][3];
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/6/5 TVB598
			{
				offset = gDacValue_TVB599[freqIndex][3];
			}
			else
			{
				offset = gDacValue_TVB590S_V_3_x[freqIndex][3];
			}
			value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_q_offset;
		}
		else
		{
			value = gpConfig->gBC[nBoardNum].gDAC_Q_Offset[0];
		}

		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

		value = abs(value);

		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xC, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xB, reg2);
		//2010/12/10 TVB593V2 ==========================================================================
		if((gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 2) || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2012/1/31 TVB591S //2011/2/15 added 11(TVB597V2)
		{
			;
		}
		else
		{
		//==============================================================================================
			//I Gain
			nGain = 0;
			TSPL_SET_AD9775_EX(nBoardNum, 0x5, nGain);

			//Q Gain
			nGain = 0;
			TSPL_SET_AD9775_EX(nBoardNum, 0x9, nGain);
		}
	}
	
}

#endif

//---------------------------------------------------------------------------
char *CWRAP_DLL::Get_AdaptorInfo(int iSlot, int iType)
{
    gUtility.MyStrCpy(gstrGeneral,256, "");
    TSPL_GET_ENCRYPTED_SN_EX(iSlot, iType, gstrGeneral);
    return gstrGeneral;
}

//---------------------------------------------------------------------------
// called from: SyncModulator
// 0=DVB-T ~ 11=DTMB (DVBT,VSB,QAMA,QAMB,QPSK,  TDMB,VSB16,DVBH,DVBS2,ISDBT,   ISDBT13,DTMB)
// Set gpConfig->gBC[nBoardNum].gbEnabledType[i]
void CWRAP_DLL::Get_Enabled_Modulator_Type(long nBoardNum)
{
	int		i;

	for (i = 0; i < MAX_MODULATORMODE; i++)
	{
		if(i == ISDB_T_13)
			gpConfig->gBC[nBoardNum].gbEnabledType[i] = gpConfig->gBC[nBoardNum].gbEnabledType[ISDB_T];
		else if(i == ASI_OUTPUT_MODE && (gpConfig->gBC[nBoardNum].gnBoardId == 0xB || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16))	//2013/5/27 TVB599 0xC
			gpConfig->gBC[nBoardNum].gbEnabledType[i] = 1;
		else
			gpConfig->gBC[nBoardNum].gbEnabledType[i] = TVB380_IS_ENABLED_TYPE_EX(nBoardNum, i);
	}
}

//---------------------------------------------------------------------------
// Set bank
// Set playrate
// ConfigureModulator()
#ifdef WIN32 

void CWRAP_DLL::Open_System(long nBoardNum)
{
    //----------------------------------------------------------------
    gpConfig->gBC[nBoardNum].gnOpenSystem = 1;

    //----------------------------------------------------------------
    // Hardware Initialization: state, bankinfo, playrate
    TSPH_START_MONITOR(nBoardNum, 0);
    TSPH_SET_SDRAM_BANK_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gnSubBankNumber, gpConfig->gBC[nBoardNum].gnSubBankOffset);
    
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
        TSPL_SET_PLAY_RATE_EX(nBoardNum, FIXED_PLAY_RATE_ISDB_T, gpConfig->gBC[nBoardNum].gnOutputClockSource);
    else
        TSPL_SET_PLAY_RATE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate, gpConfig->gBC[nBoardNum].gnOutputClockSource);

    //----------------------------------------------------------------
    //kslee 091222
	if (gpConfig->gBC[nBoardNum].gnCmdInitFW || gpConfig->gBC[nBoardNum].downflag == 1)
    {

	    TVB380_SET_STOP_MODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);
        if(gpConfig->gBC[nBoardNum].gnCmdInitFW == false)
			gRegVar.RestoreVariables(gGeneral.gnActiveBoard);
		//2011/5/4 AD9852 OVER CLOCK
		if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			TSPL_SET_AD9852_MAX_CLOCK_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnAD9852_Overclock);

		gpConfig->gBC[nBoardNum].gnCmdInitFW = false;
        Configure_Modulator(1);     // download RBF
		//kslee 091222
		if(gpConfig->gBC[nBoardNum].downflag == 1)
		{
			gUtilInd.SyncModulatorMode(nBoardNum);
			gpConfig->gBC[nBoardNum].downflag = 0;
		}
    } else
    {
        Configure_Modulator(0);     // No download RBF
    }

    //----------------------------------------------------------------
    TVB380_SET_STOP_MODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);
#if 0
    //----------------------------------------------------------------
	FormCollection^ forms;
	forms = Application::OpenForms;
	
	for(int i=0; i < forms->Count; i++)
	{
		if(forms[i]->Name == "PlayForm")
		{
			safe_cast<PlayForm^>(forms[i])->Initial_Check(nBoardNum);
			break;
		}
	}
#endif
}

#else 
void CWRAP_DLL::Open_System(long nBoardNum)
{
    //----------------------------------------------------------------
    gpConfig->gBC[nBoardNum].gnOpenSystem = 1;

    //----------------------------------------------------------------
    // Hardware Initialization: state, bankinfo, playrate
    TSPH_SET_SDRAM_BANK_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gnSubBankNumber, gpConfig->gBC[nBoardNum].gnSubBankOffset);
    
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
        TSPL_SET_PLAY_RATE_EX(nBoardNum, FIXED_PLAY_RATE_ISDB_T, gpConfig->gBC[nBoardNum].gnOutputClockSource);
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
		TSPL_SET_PLAY_RATE_EX(nBoardNum, 2433331, gpConfig->gBC[nBoardNum].gnOutputClockSource);
	else
        TSPL_SET_PLAY_RATE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate, gpConfig->gBC[nBoardNum].gnOutputClockSource);

    //----------------------------------------------------------------
	if (gpConfig->gBC[nBoardNum].gnCmdInitFW || gGeneral.gndownflag == 1)
    {
        gpConfig->gBC[nBoardNum].gnCmdInitFW = false;
        Configure_Modulator(1);     // download RBF
		//kslee 091222
		if(gGeneral.gndownflag == 1)
		{
			gUtilInd.SyncModulatorMode(nBoardNum);
			gGeneral.gndownflag = 0;
		}
    }
	else
    {
        Configure_Modulator(0);     // No download RBF
    }

    //----------------------------------------------------------------
    TVB380_SET_STOP_MODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);

    //----------------------------------------------------------------
}
#endif

//--------------------------------------------------------------------------
// return playrate
void CWRAP_DLL::Set_Burst_Bitrate(long nBoardNum, long lOutputClockSource)
{
    gpConfig->gBC[nBoardNum].gnOutputClockSource = lOutputClockSource;
	TSPL_SET_MAX_PLAYRATE_EX(nBoardNum, 0, gpConfig->gBC[nBoardNum].gnOutputClockSource);
}

//--------------------------------------------------------------------------
void CWRAP_DLL::Set_CNR(long nBoardNum, float fCNR)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;

    if (gpConfig->gBC[nBoardNum].gnPRBSmode != PRBS_MODE_NONE)
    {
        if (fCNR >= MIN_CNR && fCNR <= MAX_CNR)
        {
            if (gpConfig->gBC[nBoardNum].gnPRBSscale != fCNR)
            {
                gpConfig->gBC[nBoardNum].gnPRBSscale = fCNR;

                TVB380_SET_MODULATOR_PRBS_INFO_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnPRBSmode, (double) gpConfig->gBC[nBoardNum].gnPRBSscale);
            }
            
            gUtilInd.LogMessage ("");
        } else
            gUtilInd.LogMessageInt(TLV_FAIL_TO_SET_CNR);
    }
}

//---------------------------------------------------------------------------
// iWhich :
//          1: FrameHeaderPN
//          2: Pilot Insertion
void CWRAP_DLL::Set_DTMB_Parameters(long nBoardNum)
{
    long    lConst = gpConfig->gBC[nBoardNum].gnConstellation;
    long    lCoderate = gpConfig->gBC[nBoardNum].gnCodeRate;
    
    //-----------------------------------------------
    // Check Check_DTMB_Params
    //   CONST_4QAM_NR or 32QAM
    //       CODE_7488_3008 or CODE_7488_4512 ==> Invalid Param
    if (lConst == CONST_DTMB_4QAM_NR || lConst == CONST_DTMB_32QAM)
    {
        if (lCoderate == CONST_DTMB_CODE_7488_3008 || lCoderate == CONST_DTMB_CODE_7488_4512)
            gUtilInd.LogMessageInt(TLV_INVALID_DTMB_PARAMS);
        else
            gUtilInd.LogMessage("");
    } else
    {
        gUtilInd.LogMessage("");
    }

    //-----------------------------------------------
    TVB380_SET_MODULATOR_DTMB_EX(nBoardNum, DTMB, 
        gpConfig->gBC[nBoardNum].gnConstellation,
        gpConfig->gBC[nBoardNum].gnCodeRate,
        gpConfig->gBC[nBoardNum].gnQAMInterleave,
        gpConfig->gBC[nBoardNum].gnFrameHeader,
        gpConfig->gBC[nBoardNum].gnCarrierNumber,
        gpConfig->gBC[nBoardNum].gnFrameHeaderPN,
        gpConfig->gBC[nBoardNum].gnPilotInsertion);
}

//---------------------------------------------------------------------------
// Modulator Type
// DVB-T            : Bandwidth, Coderate, GuardInterval, TxMode, Constellation
// VSB_8            : Nothing
// QAM_A            : Constellation, (Symbolrate)
// QAM_B            : Constellation, Interleaver, (Symbolrate)
// QPSK             : Coderate, Spectrum, RRC Filter, (Symbolrate)
// DVB_S2           : Constellation, Coderate, PILOT, ROLL-OFF, Spectrum, (Symbolrate)
// TDMB             : Nothing
// VSB_16           : Nothing
// DVB_H            : DVB-T + Cell ID, MPE-FEC, TimeSlice, InDepthInterleaver
// ISDB_T           : Nothing
// DTMB             : Constellation, Coderate, Interleave, FrameHeader, CarrierNumber, FrameHeaderP/N, PILOT Insertion

//---------------------------------------------------------------------------
// DVB-T            : Bandwidth, Coderate, TxMode, GuardInterval, Constellation
// DVB-H:           :
// iWhich:  0: All
//          1: Bandwidth
//          2: Coderate
//          3: Guard Interval
//          4: TxMode
//          5: Constellation
//          6: Additional
void CWRAP_DLL::Set_DVBT_Parameters(long nBoardNum, int iWhich)
{
    long            lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    unsigned long   lRFFreq =  gpConfig->gBC[nBoardNum].gnRFOutFreq;
    long            Txmode;

    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {
        case 1:
            if(lModType == DVB_H)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,lModType, gpConfig->gBC[nBoardNum].gnBandwidth + 3, lRFFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,lModType, gpConfig->gBC[nBoardNum].gnBandwidth - 1, lRFFreq);
			}
			else
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,lModType, gpConfig->gBC[nBoardNum].gnBandwidth, lRFFreq);
            break;
        case 2:
            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;

        case 3:  // Guard Interval
            TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnGuardInterval);
            break;

        case 4: // TxMode
        case 6: // Additional
            if (lModType == DVB_H)
            {
                Txmode = gpConfig->gBC[nBoardNum].gnTxmode;
                if (gpConfig->gBC[nBoardNum].gnTxmode == TX_4K)
                    Txmode = 1;
                else if (gpConfig->gBC[nBoardNum].gnTxmode == TX_8K)
                    Txmode = 2;

                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, lModType, Txmode,
                        gpConfig->gBC[nBoardNum].gnIn_Depth,
                        gpConfig->gBC[nBoardNum].gnTime_Slice,
                        gpConfig->gBC[nBoardNum].gnMPE_FEC,
                        gpConfig->gBC[nBoardNum].gnCell_Id);
            } else
            {
				if(gpConfig->gBC[nBoardNum].gnBoardId == 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
					gpConfig->gBC[nBoardNum].gnBoardId == 21 || gpConfig->gBC[nBoardNum].gnBoardId == 22 || gpConfig->gBC[nBoardNum].gnBoardId == 15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
	                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnTxmode,
		                    0,
			                0,
				            0,
					        gpConfig->gBC[nBoardNum].gnCell_Id);
				}
				else
					TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnTxmode);
            }
            break;

        case 5:     // constellation
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnConstellation);
            break;
    }
}

//---------------------------------------------------------------------------
// DVB_S2           : Constellation, Coderate, PILOT, ROLL-OFF, Spectrum, (Symbolrate)
// iWhich:  0: All
//          1:
//          2: Coderate
//          3: RollOff
//          4: Spectrum
//          5: Constellation
//          6: Pilot
// nCalcBurtBitrate after Coderate change
// nCalcSymbolRate after coderate change

#ifdef WIN32 
void CWRAP_DLL::Set_DVBS2_Parameters(long nBoardNum, int iWhich)
{
    long        lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    long        nCodeRate;
    long        lConst = gpConfig->gBC[nBoardNum].gnConstellation;
    
    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {

        case 3:     // Roll Off
            TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;

        case 4:     // Spectrum
            Configure_Modulator(0);
            break;

        case 2:     // Coderate
        case 5:     // Constellation
            if (iWhich == 5)
            {
                TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnConstellation);
                // change Coderate UI.
				FormCollection^ forms;
				forms = Application::OpenForms;
	
				for(int i=0; i < forms->Count; i++)
				{
					if(forms[i]->Name == "PlayForm")
					{
						safe_cast<PlayForm^>(forms[i])->Display_DVBS2_Coderate(nBoardNum);
						break;
					}
				}
            }

            if (lConst == CONST_DVB_S2_QPSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate;
            } else if (lConst == CONST_DVB_S2_8PSK)
            {
                if (gpConfig->gBC[nBoardNum].gnCodeRate >= 3)
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;
                else
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 4;
            } else if (lConst == CONST_DVB_S2_16APSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;  //   '5(2/3),...,10(9/10)
            } else if (lConst == CONST_DVB_S2_32APSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 6;  // '6(3/4),...,10(9/10)
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, lModType, nCodeRate);
            break;

        case 6: // Pilot
            TVB380_SET_MODULATOR_PILOT_EX(nBoardNum, lModType, (gpConfig->gBC[nBoardNum].gnPilot + 1) % 2);
            break;

    }
}

#else 
void CWRAP_DLL::Set_DVBS2_Parameters(long nBoardNum, int iWhich)
{
    long        lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    long        nCodeRate;
    long        lConst = gpConfig->gBC[nBoardNum].gnConstellation;
    
    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {

        case 3:     // Roll Off
            TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;

        case 4:     // Spectrum
            Configure_Modulator(0);
            break;

        case 2:     // Coderate
        case 5:     // Constellation
            if (iWhich == 5)
            {
                TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnConstellation);
            }
            if (lConst == CONST_DVB_S2_QPSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate;
            } else if (lConst == CONST_DVB_S2_8PSK)
            {
                if (gpConfig->gBC[nBoardNum].gnCodeRate >= 3)
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;
                else
                    nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 4;
            } else if (lConst == CONST_DVB_S2_16APSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 5;  //   '5(2/3),...,10(9/10)
            } else if (lConst == CONST_DVB_S2_32APSK)
            {
                nCodeRate = gpConfig->gBC[nBoardNum].gnCodeRate + 6;  // '6(3/4),...,10(9/10)
            }

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, lModType, nCodeRate);
            break;

        case 6: // Pilot
            TVB380_SET_MODULATOR_PILOT_EX(nBoardNum, lModType, (gpConfig->gBC[nBoardNum].gnPilot + 1) % 2);
            break;

    }
}
#endif

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Play_Rate_Ex(long nBoardNum, long playFreqHz, long nUseMaxPlayrate)
{
    long lRet = 0;

    lRet = playFreqHz;

    //--- Set Playrate
    TSPL_SET_PLAY_RATE_EX(nBoardNum, lRet, nUseMaxPlayrate);
    gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
    return lRet;
}

//---------------------------------------------------------------------------
// QPSK
// iWhich:  0: All
//          1:
//          2: Coderate
//          3: RRC_Filter
//          4: Spectrum
//          5:
// nCalcBurtBitrate after Coderate change
// nCalcSymbolRate after coderate change
void CWRAP_DLL::Set_QPSK_Parameters(long nBoardNum, int iWhich)
{
    long        lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;

    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {
        case 2:     // Coderate
            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case 3:     // RRC Filter
            TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;
        case 4:     // Spectrum
            Configure_Modulator(0);
            break;
    }
}

//---------------------------------------------------------------------------
// QAMA
// iWhich:  0: All
//          1:
//          2:
//          3:
//          4:
//          5: Constellation
void CWRAP_DLL::Set_QAMA_Parameters(long nBoardNum, int iWhich)
{
    long        lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;

    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {
        case 5:     // Constellation
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
    }
}

//---------------------------------------------------------------------------
// QAMB
//          2: Interleaving
//          5: Constellation
void CWRAP_DLL::Set_QAMB_Parameters(long nBoardNum, int iWhich)
{
    long        lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;

    if (iWhich == 0)
    {
        return;
    }

    switch(iWhich)
    {
        case 2:         // Interleaving
            TVB380_SET_MODULATOR_INTERLEAVE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;
            
        case 5:         // Constellation
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
    }
}

//---------------------------------------------------------------------------
// Set RF Frequency.
// Return 0: Error, not setting
//        other: the frequency in Hz
#ifdef WIN32 
unsigned long CWRAP_DLL::Set_RF_Frequency(long nBoardNum, unsigned long dwFreq)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	unsigned long	nMin = 0;
	unsigned long	nMax = 0;
	unsigned long	nMin2 = 0;
	unsigned long	nMax2 = 0;
    //--------------------------------------------------------
    // Range Check
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if(gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			if (lModType == QPSK || lModType == DVB_S2)
			{
				nMin = RF_47_MIN_DVB_S;
				nMax = RF_47_MAX_DVB_S;
			} else
			{
				nMin = RF_47_MIN;
				nMax = RF_47_MAX;
			}
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
				//2010/10/5 TVB593
				if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
					nMin = RF_47_MIN;
					nMax = RF_47_MAX;
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 2)
				{
					nMin = RF_47_MIN;
					nMax = RF_47_MAX;
				}
				else
				{
					nMin = RF_20_MIN;
					nMax = RF_20_MAX;
				}
			}

			if(gpConfig->gBC[nBoardNum].gnBoardId == 0x15)
			{
				nMin = RF_TVB591_MIN;
				nMax = RF_TVB591_MAX;
			}
		}
		else
		{
			if (lModType == QPSK || lModType == DVB_S2)
			{
				if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
				{
					nMin = RF_44_MIN_DVB_S;
					nMax = RF_44_MAX_DVB_S;
				}
				else
				{
					nMin = RF_44_MIN_IF_44_DVB_S;
					nMax = RF_44_MAX_IF_44_DVB_S;
				}
			}
			else
			{
				if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
				{
					nMin = RF_44_MIN;
					nMax = RF_44_MAX;
					nMin2 = RF_44_MIN_H;
					nMax2 = RF_44_MAX_H;
				}
				else
				{
					nMin = RF_44_MIN_IF_44;
					nMax = RF_44_MAX_IF_44;
					nMin2 = RF_44_MIN_H_IF_44;
					nMax2 = RF_44_MAX_H_IF_44;
				}
			}
		}
	//TVB380V4, TVB390V6.1
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 41 || gpConfig->gBC[nBoardNum].gnBoardId == 42)
    {
        if (lModType == QPSK)
        {
			nMin = RF_41_MIN_DVB_S;
			nMax = RF_41_MAX_DVB_S;
        }else if(lModType == TDMB)
        {
			nMin = RF_41_MIN_TDMB;
			nMax = RF_41_MAX_TDMB;
        }
		else
        {
			nMin = RF_41_MIN;
			nMax = RF_41_MAX;
        }
    //TVB379V6
	} else if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
    {
		nMin = RF_43_MIN;
		nMax = RF_43_MAX;
    } else
    {
        if (lModType == QPSK)
        {
			nMin = RF_MIN_DVB_S;
			nMax = RF_MAX_DVB_S;
        } else
        {
			nMin = RF_MIN;
			nMax = RF_MAX;
        }
    }
	if(gGeneral.gnOverFrequency == 1)
	{
		if(gUtilInd.IsSupportHMC833(nBoardNum))
		{
			nMin = RF_MIN_HMC833;
			nMax = RF_MAX_HMC833;
		}
	}

	if(!((dwFreq >= nMin && dwFreq <= nMax) || (dwFreq >= nMin2 && dwFreq <= nMax2)))
	{
		gUtilInd.LogMessageInt(TLV_OUT_OF_RF_RANGE);
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF &&
			gpConfig->gBC[nBoardNum].gnBoardId != 11 && gpConfig->gBC[nBoardNum].gnBoardId != 0x16 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 27 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
			return gpConfig->gBC[nBoardNum].gnRFOutFreq;
	}
	gpConfig->gBC[nBoardNum].gnRFOutFreq = dwFreq;

    //--------------------------------------------------------
    // Set Spectrum
    gUtilInd.Adjust_Spectrum(nBoardNum);
	//2012/3/20
	if((gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15) && (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH))
	{
		if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
        {
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
		else
		{
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
	}
	else
	    TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);

    //--------------------------------------------------------
    // Set Frequency
    if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
        TVB380_SET_MODULATOR_IF_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq);
	//DVB-T2 kslee 2010/4/20
    if (lModType == DVB_T || lModType == DVB_H || lModType == DVB_T2 || lModType == MULTIPLE_DVBT)
    {
        //2010/6/1
		if(lModType == DVB_T2)
		{
			int bandwidth_t2;
			if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN || gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnBandwidth + 4;
				else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnBandwidth + 2;
				else
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnBandwidth - 2;
			}
			else if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnT2MI_BW + 4;
				else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnT2MI_BW + 2;
				else
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnT2MI_BW - 2;
			}
			else
			{
				if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 0)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 4;
				else if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 1)
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 2;
				else
					bandwidth_t2 = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW - 2;
			}
			//1==NTSC Carrier, RF = USER RF + 1.750MHz
		    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, bandwidth_t2);
			else	
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, bandwidth_t2);
		}
		else
		{
			int bandwidth_tmp;
			if(lModType == DVB_H)
			{
				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					bandwidth_tmp = gpConfig->gBC[nBoardNum].gnBandwidth + 3;
				else
					bandwidth_tmp = gpConfig->gBC[nBoardNum].gnBandwidth - 1;
			}
			else
				bandwidth_tmp = gpConfig->gBC[nBoardNum].gnBandwidth;
		    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, bandwidth_tmp);
			else	
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, bandwidth_tmp);
		}
    }
	//2011/2/28 DVB-C2
	else if(lModType == DVB_C2)
	{
	    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnBandwidth);
		else	
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
	}
	else
    {
		//2010/10/7
		if((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			(lModType == ISDB_T || lModType == ISDB_T_13)) 	//2013/5/27 TVB599 0xC
		{
			if(((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) ||
				(gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			}
			else
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, (((int)(gpConfig->gBC[nBoardNum].gnRFOutFreq / 1000000)) * 1000000), gpConfig->gBC[nBoardNum].gnSymbolRate);
		}
		else
		{
	        //1==NTSC Carrier, RF = USER RF + 1.750MHz
	        if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnSymbolRate);
			else
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
		}
	}
	long IF_offset;
	//2011/11/30 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnCurrentIF;
	}
	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 44000000;
	else
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36125000;
    //--------------------------------------------------------
    // Set RF Power Level
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
		{
			FormCollection^ forms;
			forms = Application::OpenForms;
		
			for(int i=0; i < forms->Count; i++)
			{
				if(forms[i]->Name == "PlayForm")
				{
					safe_cast<PlayForm^>(forms[i])->UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
					break;
				}
			}
		}
    }

    //--------------------------------------------------------
    // Set Channel Number according to Frequency in Main

    return gpConfig->gBC[nBoardNum].gnRFOutFreq;
}
#else 
unsigned long CWRAP_DLL::Set_RF_Frequency(long nBoardNum, unsigned long dwFreq)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	unsigned long	nMin = 0;
	unsigned long	nMax = 0;
	unsigned long	nMin2 = 0;
	unsigned long	nMax2 = 0;
    //--------------------------------------------------------
    // Range Check
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if(gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			if (lModType == QPSK || lModType == DVB_S2)
			{
				nMin = RF_47_MIN_DVB_S;
				nMax = RF_47_MAX_DVB_S;
			} else
			{
				nMin = RF_47_MIN;
				nMax = RF_47_MAX;
			}
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
				//2010/10/5 TVB593
				if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
					nMin = RF_47_MIN;
					nMax = RF_47_MAX;
				}
				else if(gpConfig->gBC[nBoardNum].gnBoardRev >= 2)
				{
					nMin = RF_47_MIN;
					nMax = RF_47_MAX;
				}
				else
				{
					nMin = RF_20_MIN;
					nMax = RF_20_MAX;
				}
			}

			if(gpConfig->gBC[nBoardNum].gnBoardId == 0x15)
			{
				nMin = RF_TVB591_MIN;
				nMax = RF_TVB591_MAX;
			}
		}
		else
		{
			if (lModType == QPSK || lModType == DVB_S2)
			{
				if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
				{
					nMin = RF_44_MIN_DVB_S;
					nMax = RF_44_MAX_DVB_S;
				}
				else
				{
					nMin = RF_44_MIN_IF_44_DVB_S;
					nMax = RF_44_MAX_IF_44_DVB_S;
				}
			}
			else
			{
				if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
				{
					nMin = RF_44_MIN;
					nMax = RF_44_MAX;
					nMin2 = RF_44_MIN_H;
					nMax2 = RF_44_MAX_H;
				}
				else
				{
					nMin = RF_44_MIN_IF_44;
					nMax = RF_44_MAX_IF_44;
					nMin2 = RF_44_MIN_H_IF_44;
					nMax2 = RF_44_MAX_H_IF_44;
				}
			}
		}
	//TVB380V4, TVB390V6.1
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 41 || gpConfig->gBC[nBoardNum].gnBoardId == 42)
    {
        if (lModType == QPSK)
        {
			nMin = RF_41_MIN_DVB_S;
			nMax = RF_41_MAX_DVB_S;
        }else if(lModType == TDMB)
        {
			nMin = RF_41_MIN_TDMB;
			nMax = RF_41_MAX_TDMB;
        }
		else
        {
			nMin = RF_41_MIN;
			nMax = RF_41_MAX;
        }
    //TVB379V6
	} else if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
    {
		nMin = RF_43_MIN;
		nMax = RF_43_MAX;
    } else
    {
        if (lModType == QPSK)
        {
			nMin = RF_MIN_DVB_S;
			nMax = RF_MAX_DVB_S;
        } else
        {
			nMin = RF_MIN;
			nMax = RF_MAX;
        }
    }
	if(!((dwFreq >= nMin && dwFreq <= nMax) || (dwFreq >= nMin2 && dwFreq <= nMax2)))
	{
        gUtilInd.LogMessageInt(TLV_OUT_OF_RF_RANGE);
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF &&
			gpConfig->gBC[nBoardNum].gnBoardId != 11 && gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)		//2013/5/27 TVB599 0xC
			return gpConfig->gBC[nBoardNum].gnRFOutFreq;
	}
	gpConfig->gBC[nBoardNum].gnRFOutFreq = dwFreq;

    //--------------------------------------------------------
    // Set Spectrum
    gUtilInd.Adjust_Spectrum(nBoardNum);
	//2012/3/20
	if((gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15) && (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH))
	{
		if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
        {
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
		else
		{
			if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, ((gpConfig->gBC[nBoardNum].gnSpectrumInverse + 1) % 2));
			}
			else
			{
				TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			}
		}
	}
	else
	    TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnSpectrumInverse);

    //--------------------------------------------------------
    // Set Frequency
    if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
        TVB380_SET_MODULATOR_IF_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq);
	//DVB-T2 kslee 2010/4/20
    if (lModType == DVB_T || lModType == DVB_H || lModType == DVB_T2)
    {
        //2010/6/1
		if(lModType == DVB_T2)
		{
			//1==NTSC Carrier, RF = USER RF + 1.750MHz
		    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnT2MI_BW);
			else	
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnT2MI_BW);
		}
		else
		{
		    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnBandwidth);
			else	
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
		}
    }
	//2011/2/28 DVB-C2
	else if(lModType == DVB_C2)
	{
	    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnBandwidth);
		else	
			gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
	}
	else
    {
		//2010/10/7
		if((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			(lModType == ISDB_T || lModType == ISDB_T_13)) 	//2013/5/27 TVB599 0xC
		{
			if(((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) ||
				(gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
			{
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
			}
			else
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, (((int)(gpConfig->gBC[nBoardNum].gnRFOutFreq / 1000000)) * 1000000), gpConfig->gBC[nBoardNum].gnSymbolRate);
		}
		else
		{
	        //1==NTSC Carrier, RF = USER RF + 1.750MHz
	        if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnSymbolRate);
			else
				gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
		}
	}
	long IF_offset;
	//2011/11/30 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnCurrentIF = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnCurrentIF;
	}
	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 44000000;
	else
		IF_offset = gpConfig->gBC[nBoardNum].gnCurrentIF - 36125000;

	//--------------------------------------------------------
    // Set RF Power Level
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
		//2012/8/31 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
		{
			UpdateRFPowerLevel(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);

		}
    }
    //--------------------------------------------------------
    // Set Channel Number according to Frequency in Main

    return gpConfig->gBC[nBoardNum].gnRFOutFreq;
}
#endif


//--------------------------------------------------------------------------
// return playrate
#ifdef WIN32 
unsigned long CWRAP_DLL::Set_Symbolrate(long nBoardNum, unsigned long dwSymbolrate)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	unsigned long	nMin = 0;
	unsigned long	nMax = 0;
    if(lModType == QAM_A)
	{
		nMin = QAM_A_SYMBOL_MIN;
		nMax = QAM_A_SYMBOL_MAX;
	}
	else if(lModType == QPSK)
	{
		//2010/10/7
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 27 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
		{
			if(dwSymbolrate > DVB_S_SYMBOL_RRC_ON_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != RRC_FILTER_OFF)
			{
				FormCollection^ forms;
				forms = Application::OpenForms;
		
				for(int i=0; i < forms->Count; i++)
				{
					if(forms[i]->Name == "PlayForm")
					{
						safe_cast<PlayForm^>(forms[i])->Combo_PARAM2->SelectedIndex = RRC_FILTER_OFF;
						gpConfig->gBC[nBoardNum].gnRollOffFactor = RRC_FILTER_OFF;
						TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
						safe_cast<PlayForm^>(forms[i])->UpdateSymbolToolTip(nBoardNum);
						break;
					}
				}
			}
		}

		if(gpConfig->gBC[nBoardNum].gnRollOffFactor == RRC_FILTER_ON)
		{
			nMin = DVB_S_SYMBOL_RRC_ON_MIN;
			nMax = DVB_S_SYMBOL_RRC_ON_MAX;
		}
		else
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_MAX;
		}
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 0
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	//2012/1/31 TVB591S //2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_20_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_20_MAX;
		}
#else
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 27)	//2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_20_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_20_MAX;
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_11_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_11_MAX;
		}
#endif

	}
	else if (lModType == DVB_S2)
    {
		//2010/10/7
		//if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
		//	gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 27 && gpConfig->gBC[nBoardNum].gnBoardId != 12)	//2013/5/27 TVB599 0xC
		//{
		//	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		//	{
		//		if(dwSymbolrate > DVB_S2_SYMBOL_RRC_OFF_IF_44_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != ROLL_OFF_FACTOR_NONE)
		//		{
		//			FormCollection^ forms;
		//			forms = Application::OpenForms;
		//	
		//			for(int i=0; i < forms->Count; i++)
		//			{
		//				if(forms[i]->Name == "PlayForm")
		//				{
		//					safe_cast<PlayForm^>(forms[i])->Combo_PARAM4->SelectedIndex = ROLL_OFF_FACTOR_NONE;
		//					gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_NONE;
		//					TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
		//					safe_cast<PlayForm^>(forms[i])->UpdateSymbolToolTip(nBoardNum);
		//					break;
		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		if(dwSymbolrate > DVB_S2_SYMBOL_RRC_ON_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != ROLL_OFF_FACTOR_NONE)
		//		{
		//			FormCollection^ forms;
		//			forms = Application::OpenForms;
		//	
		//			for(int i=0; i < forms->Count; i++)
		//			{
		//				if(forms[i]->Name == "PlayForm")
		//				{
		//					safe_cast<PlayForm^>(forms[i])->Combo_PARAM4->SelectedIndex = ROLL_OFF_FACTOR_NONE;
		//					gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_NONE;
		//					TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
		//					safe_cast<PlayForm^>(forms[i])->UpdateSymbolToolTip(nBoardNum);
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}

		if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
        {
			if (gpConfig->gBC[nBoardNum].gnIFOutFreq ==IF_OUT_44MHZ)
            {
				nMin = DVB_S2_SYMBOL_RRC_OFF_IF_44_MIN;
				nMax = DVB_S2_SYMBOL_RRC_OFF_IF_44_MAX;
            } else
            {
				nMin = DVB_S2_SYMBOL_RRC_OFF_MIN;
				nMax = DVB_S2_SYMBOL_RRC_OFF_MAX;
            }
		}
		else
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_MAX;
		}
        //TVB590S
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 0
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	//2012/1/31 TVB591S //2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_20_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_20_MAX;
		}
#else
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 27)
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_20_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_20_MAX;
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_11_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_11_MAX;
		}
#endif
	}
	else
	{
		nMin = dwSymbolrate;
		nMax = dwSymbolrate;
	}
	if(!(dwSymbolrate >= nMin && dwSymbolrate <= nMax))
	{
		gUtilInd.LogMessageInt(TLV_SYMBOL_RATE_OUT_OF_RANGE);
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 27 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
		{
			return FALSE;
		}
	}
	gpConfig->gBC[nBoardNum].gnSymbolRate = dwSymbolrate;
    if (lModType == QAM_A || lModType == QAM_B || lModType == QPSK || lModType == DVB_S2 || lModType == IQ_PLAY || lModType == MULTIPLE_QAMB)
    {
        TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, dwSymbolrate);
    }

    return TRUE;
}
#else
unsigned long CWRAP_DLL::Set_Symbolrate(long nBoardNum, unsigned long dwSymbolrate)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	unsigned long	nMin = 0;
	unsigned long	nMax = 0;

    if(lModType == QAM_A)
	{
		nMin = QAM_A_SYMBOL_MIN;
		nMax = QAM_A_SYMBOL_MAX;
	}
	else if(lModType == QPSK)
	{
		//2010/10/7
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
		{
			if(dwSymbolrate > DVB_S_SYMBOL_RRC_ON_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != RRC_FILTER_OFF)
			{
				gpConfig->gBC[nBoardNum].gnRollOffFactor = RRC_FILTER_OFF;
				TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
				gGui.SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
			}
		}

		if(gpConfig->gBC[nBoardNum].gnRollOffFactor == RRC_FILTER_ON)
		{
			nMin = DVB_S_SYMBOL_RRC_ON_MIN;
			nMax = DVB_S_SYMBOL_RRC_ON_MAX;
		}
		else
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_MAX;
		}
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 1
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_20_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_20_MAX;
		}
#else
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20)	//2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_20_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_20_MAX;
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	TODO TVB599
		{
			nMin = DVB_S_SYMBOL_RRC_OFF_11_MIN;
			nMax = DVB_S_SYMBOL_RRC_OFF_11_MAX;
		}
#endif

	}
	else if (lModType == DVB_S2)
    {
		//2010/10/7
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
		{
			if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
			{
				if(dwSymbolrate > DVB_S2_SYMBOL_RRC_OFF_IF_44_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != ROLL_OFF_FACTOR_NONE)
				{
					gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_NONE;
					TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
					gGui.SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
				}
			}
			else
			{
				if(dwSymbolrate > DVB_S2_SYMBOL_RRC_ON_MAX && gpConfig->gBC[nBoardNum].gnRollOffFactor != ROLL_OFF_FACTOR_NONE)
				{
					gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_NONE;
					TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRollOffFactor); 
					gGui.SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
				}
			}
		}

		if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
        {
			if (gpConfig->gBC[nBoardNum].gnIFOutFreq ==IF_OUT_44MHZ)
            {
				nMin = DVB_S2_SYMBOL_RRC_OFF_IF_44_MIN;
				nMax = DVB_S2_SYMBOL_RRC_OFF_IF_44_MAX;
            } else
            {
				nMin = DVB_S2_SYMBOL_RRC_OFF_MIN;
				nMax = DVB_S2_SYMBOL_RRC_OFF_MAX;
            }
		}
		else
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_MAX;
		}
        //TVB590S
		//2011/9/1 TEST DVB-S/S2 Maximum Symbolrate 45M ==> 70M
#if 1
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_20_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_20_MAX;
		}
#else
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20)	//2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_20_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_20_MAX;
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	//TODO TVB599
		{
			nMin = DVB_S2_SYMBOL_RRC_ON_11_MIN;
			nMax = DVB_S2_SYMBOL_RRC_ON_11_MAX;
		}
#endif
	}
	else
	{
		nMin = dwSymbolrate;
		nMax = dwSymbolrate;
	}
	if(!(dwSymbolrate >= nMin && dwSymbolrate <= nMax))
	{
        gUtilInd.LogMessageInt(TLV_SYMBOL_RATE_OUT_OF_RANGE);
		if(gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
			gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
		{
            return false;
		}
	}
	gpConfig->gBC[nBoardNum].gnSymbolRate = dwSymbolrate;
    if (lModType == QAM_A || lModType == QAM_B || lModType == MULTIPLE_QAMB || lModType == QPSK || lModType == DVB_S2 || lModType == IQ_PLAY)
    {
        TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, lModType, gpConfig->gBC[nBoardNum].gnRFOutFreq, dwSymbolrate);
    }

    return true;
}


#endif
//---------------------------------------------------------------------------
// SetCurrentPlayOffset
//
void CWRAP_DLL::SetCurrentPlayOffset(long nBoardNum, long nCurrentSliderPos, double dwFileSize)
{
    double dwStartPos;
    double dwEndPos;
    double dwCurrentPos;

    if (gpConfig->gBC[nBoardNum].gnStartPosChanged == 1)
    {
        dwStartPos = (double)gpConfig->gBC[nBoardNum].gnStartTimeOffset * (double)(gpConfig->gBC[nBoardNum].gdwPlayRate)/8.0;
 		//2012/5/15
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
		{
			dwStartPos = dwStartPos / 7300.0 * 6144;
		}
        gpConfig->gBC[nBoardNum].gnStartPos = (__int64) dwStartPos;
        gpConfig->gBC[nBoardNum].gnCurrentPos = gpConfig->gBC[nBoardNum].gnStartPos;
        gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 1;
        
        TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_START, (double) gpConfig->gBC[nBoardNum].gnStartPos);
        TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double) gpConfig->gBC[nBoardNum].gnCurrentPos);
        
        if (gpConfig->gBC[nBoardNum].gnEndPosChanged == 1)
        {
            dwEndPos = (double)gpConfig->gBC[nBoardNum].gnEndTimeOffset * (double)gpConfig->gBC[nBoardNum].gdwPlayRate/8.0;
	 		//2012/5/15
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
			{
				dwEndPos = dwEndPos / 7300.0 * 6144;
			}
            gpConfig->gBC[nBoardNum].gnEndPos = (__int64) dwEndPos;                           

            if (gpConfig->gBC[nBoardNum].gnEndTimeOffset >= gpConfig->gBC[nBoardNum].gnStartTimeOffset + gpConfig->gBC[nBoardNum].gnOffsetMargin)
                TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_END, (double) gpConfig->gBC[nBoardNum].gnEndPos);
        }
    } else
    {
        if (nCurrentSliderPos > 0 && nCurrentSliderPos < 99)
        {
            dwCurrentPos = dwFileSize * nCurrentSliderPos / 99.0;   
            gpConfig->gBC[nBoardNum].gnCurrentPos = (__int64) dwCurrentPos;
            TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double) gpConfig->gBC[nBoardNum].gnCurrentPos);   
            gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 1;
        } else
        {
            gpConfig->gBC[nBoardNum].gnStartPosChanged = 0;
            gpConfig->gBC[nBoardNum].gnEndPosChanged = 0;
            gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 0;
            TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_RELEASE, 0);
        }    
    }
}

//---------------------------------------------------------------------------
void CWRAP_DLL::SetFileNameNextPlay(long nBoardNum)
{
    int nCheck;
#ifdef WIN32
	if (gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
#else
    if (gpConfig->gBC[nBoardNum].nPlayListIndexCount == 0)
#endif
    {
        nCheck = TSPH_SET_FILENAME_NEXT_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
    }
    else
    {
#ifdef WIN32
        gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, sizeof(gpConfig->gBC[nBoardNum].szCurFileName), "%s\\%s",
            gpConfig->gBC[nBoardNum].szPlayFileList[gpConfig->gBC[nBoardNum].nPlayListIndexCur],
			gpConfig->gBC[nBoardNum].szPlayListFileName[gpConfig->gBC[nBoardNum].nPlayListIndexCur]);
#else
        gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, sizeof(gpConfig->gBC[nBoardNum].szCurFileName), "%s/%s",
            gpConfig->gBC[nBoardNum].gszMasterDirectory,
            gpConfig->gBC[nBoardNum].szPlayFileList[gpConfig->gBC[nBoardNum].nPlayListIndexCur]);
#endif
        nCheck = TSPH_SET_FILENAME_NEXT_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
#ifdef WIN32
#else
        if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
			gpConfig->gBC[nBoardNum].gdwPlayRate = gUtilInd.CalcBurtBitrate(nBoardNum);
        else
			gpConfig->gBC[nBoardNum].gdwPlayRate = gWrapDll.Get_Playrate(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0);
#endif
    }
}

//---------------------------------------------------------------------------
int CWRAP_DLL::SetStreamSourcePort(long nSource)
{
    int iRet;

    iRet = TSPL_SET_TSIO_DIRECTION_EX(gGeneral.gnActiveBoard, nSource);
    if (iRet != 0)
        gUtilInd.LogMessageInt(iRet);
    else
        gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource = nSource;
        
    gpConfig->gBC[gGeneral.gnActiveBoard].gnOffsetMargin = 0;
    return iRet;
}

//---------------------------------------------------------------------------
// Start Playing
#ifdef WIN32
void CWRAP_DLL::Start_Playing(long nBoardNum, long lStartPosPercent, char *szFileName)
{
    long    nBankNumber;
    long    nBankOffset;
    long    nRestampingOpt;
    double  dwStartOffset;
    int     nRet, i;
    
    char    szNewFileName[256];
    char    szURL_MRL[256];
    char    szTemp[256];
    //----------------------------------------------------------------------------
    // Set Bank info
    //----------------------------------------------------------------------------

	if((gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT) && 
		gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		if(gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnModulatorSource == SMPTE310M_SRC)
		{
			gUtilInd.LogMessage("Can not play a file in ASI or 310M.");
			return;
		}
	}

	//2011/11/17 IQ NEW FILE FORMAT
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY && gpConfig->gBC[nBoardNum].gnSymbolRate < 1)
	{
		return;
	}

	if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
	{
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
		{
			gUtility.MyStrCpy(szFileName, 512, gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS);
		}
		if(gUtility.Is_File_Exist(szFileName) == false)
			return;
	}
	else if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
	{
		if(gUtility.Is_File_Exist(szFileName) == false)
			return;
	}
	else if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
	{
		gUtility.MyStrCpy(szFileName, 512, "");
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
		{
			for(i = 0; i < MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]) > 0)
				{
					if(gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]) == true)
					{
						gUtility.MyStrCpy(szFileName, 512, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]);
						break;
					}
				}
			}
			if(gpConfig->gBC[nBoardNum].gnOutputType == OUTPUT_FILE)
			{
				if(strlen(gpConfig->gBC[nBoardNum].gszOutput_Filename) < 1)
					return;
			}
		}
		else
		{
			for(i = 0; i < MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					if(gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) == true)
					{
						gUtility.MyStrCpy(szFileName, 512, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]);
						break;
					}
				}
			}
		}
	}

    nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
    nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;

    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
    {
        ;  // nothing
    }
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		;  // nothing
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    {
        nBankOffset = gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset;
    }
	else if(/*(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN) || */
						gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
	{
		nBankOffset = 1024;
	}
	else if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
    {
        nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
    }

    TSPH_SET_SDRAM_BANK_INFO(nBoardNum, nBankNumber, nBankOffset);
    
	//----------------------------------------------------------------------------
    // check IP Streaming
    //----------------------------------------------------------------------------
    if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
    {
        if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
        {
            time_t      tCur;
            struct tm   newtime;

            tCur = time(NULL);
#ifdef WIN32
            localtime_s(&newtime, &tCur);
            gUtility.MySprintf(szNewFileName, 256, "%02d%02d_%02d%02d%02d.TRP",
                    newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
#else
			localtime_r(&tCur, &newtime);
            gUtility.MySprintf(szNewFileName, 256, (char *) "%02d%02d_%02d%02d%02d.TRP",
                    newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
#endif
        }
    }
    
    gpConfig->gBC[nBoardNum].bPlayingProgress =  true;
    nRestampingOpt = 0;
    dwStartOffset = 0;
	
 //   //--- current filename
 //   //2011/4/4 ISDB-S Bitrate
	//if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
	//{
	//	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS);
	//}
	//else
	//{
	//	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, szFileName);
	//}
    //--- SetFileSize, Runtime (Basic Program: ShowFileSize)
    gUtilInd.GetFileSizeInSec(szFileName, nBoardNum);

    //--- start offset
    SetCurrentPlayOffset(nBoardNum, lStartPosPercent, (double) gpConfig->gBC[nBoardNum].dwFileSize);     // lStartPosPercent: slide postion

	//2010/12/06 ISDB-S =====================================================================================================================
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
		{
			if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
			{
				TSPH_SET_COMBINER_INFO(nBoardNum, 0, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnConstellation,
					gpConfig->gBC[nBoardNum].gnCodeRate, gpConfig->gBC[nBoardNum].gnSlotCount);
			}
			else
			{
				int ts_num, nSlotCount;
                ts_num = 0;
				nSlotCount = 0;

				for(int ii = 0 ; ii < MAX_TS_COUNT ; ii++)
				{
					if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[ii] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[ii]) > 0 &&
						gpConfig->gBC[nBoardNum].gnSlotCount_M[ii] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[ii] <= MAX_SLOT_COUNT)
					{
						ts_num = ts_num + 1;
						nSlotCount = nSlotCount + gpConfig->gBC[nBoardNum].gnSlotCount_M[ii];
					}
				}
				if(ts_num == 0 || nSlotCount != 48)
				{
					gpConfig->gBC[nBoardNum].bPlayingProgress = false;
					gUtilInd.LogMessage("Please, set Multi-TS Combiner Parameters.");
					return;
				}
				//2011/4/4 ISDB-S Bitrate
				else
				{
					TSPH_SET_ISDBS_BASE_TS(nBoardNum, gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS);
				}
				//=======================
			}
		}
		else if(gpConfig->gBC[nBoardNum].gnInputSource == PRBS_DATA_IN)
		{
			TSPH_SET_COMBINER_INFO(nBoardNum, 0, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnConstellation,
				gpConfig->gBC[nBoardNum].gnCodeRate, gpConfig->gBC[nBoardNum].gnSlotCount);
		}
	}
	//=======================================================================================================================================



    //--- IP Streaming/VLC
    if (/*gGeneral.gnVLCRunning < 0 &&*/
        gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
            if (gpConfig->gBC[nBoardNum].gnIPStreamingMode != NO_IP_STREAM)
            {
                // set the recoorded file name
                if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
                {
                    FILE    *hFile = NULL;

                    // create and check record file
#ifdef WIN32
					gUtility.MySprintf(szTemp, 256, "%s\\%s", gpConfig->gBC[nBoardNum].gszMasterDirectory,szNewFileName);
					hFile = gUtility.MyFopen(hFile, szTemp,"ab");
#else
                    gUtility.MySprintf(szTemp, 256, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory,szNewFileName);
					hFile = gUtility.MyFopen(hFile, szTemp,(char *)"ab");
#endif
                    if (hFile == NULL)
                    {
                        gUtilInd.LogMessageInt(TLV_FAIL_TO_CREATE_RECORD_FILE);
                        return;
                    }
                    fclose(hFile);
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, szTemp);
                }

                // set the target ULR/MRL
                if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == SEND_IP_STREAM)
                    gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
                else
                    gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo);

            } else if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
            {
                gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
            } else
            {
                gUtility.MyStrCpy(szURL_MRL, 256, (char *)"");
            }

			//kslee 2010/3/18
			//IP UDP/RTP
			if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, (char *)"0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
			else
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
			//2011/7/20 added IP
			long MaxPlayrate;
			MaxPlayrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
            else
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            MaxPlayrate/*gpConfig->gBC[nBoardNum].gdwPlayRate*/, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);

    } else if (gGeneral.gnVLCRunning < 0 &&
        gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
            gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
            gUtility.MyStrCpy(szURL_MRL, 256, (char *)"sout=#duplicate{dst=display} no-sout-all");
            
			//kslee 2010/3/18
			//IP UDP/RTP
			if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, (char *)"0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
			else
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);

			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
#ifndef WIN32
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							2433331, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            1);	// true: loop, false:single //1 -> gpConfig->gBC[nBoardNum].gnOutputClockSource
#endif
            else
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
            if (nRet == 0)
                gGeneral.gnVLCRunning = nBoardNum;
    } else
    {
		//2012/2/20 FilePlay Debug Mode
		if(gpConfig->gBC[nBoardNum].gnDebugMode_FilePlay == 1)
		{
			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource, true);
#ifndef WIN32
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							2433331, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            0);	// true: loop, false:single //1 -> gpConfig->gBC[nBoardNum].gnOutputClockSource
#endif
			else
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            true);
		}
		else 
		{

			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource, false);
#ifndef WIN32
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							2433331, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            0);	// true: loop, false:single //1 -> gpConfig->gBC[nBoardNum].gnOutputClockSource
#endif
           else
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false);
		}
    }

    if (nRet != 0)
    {
        gUtilInd.LogMessageInt(nRet);
        gpConfig->gBC[nBoardNum].bPlayingProgress = false;
    } else
    {
		if((gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) && gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))		//2013/5/27 TVB599 0xC
		{
#ifdef WIN32
			gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);
#else
			TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);
#endif
		}
	
		if (gGeneral.gnVLCRunning >= 0 &&
            (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM || gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC))
            gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0 * (gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));

            gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
            gpConfig->gBC[nBoardNum].nLastHLDThreadState = 0;
		int _Val_;
		if((gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)&& gpConfig->gBC[nBoardNum].bPlayingProgress == true)
		{
			if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
				_Val_ = ((gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3); 
			else
				_Val_ = gpConfig->gBC[nBoardNum].gn_StreamNum;
			TSPH__SEL_TS_of_ASI310OUT(nBoardNum, _Val_);
		}
		if(gpConfig->gBC[nBoardNum].gnInputSource == IP_IN)
			gpConfig->gBC[nBoardNum].gnBoardStatusReset = 15;
		else
			gpConfig->gBC[nBoardNum].gnBoardStatusReset = 5;
    }

    gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
}

#else
void CWRAP_DLL::Start_Playing(long nBoardNum, long lStartPosPercent, char *szFileName)
{
    long    nBankNumber;
    long    nBankOffset;
    long    nRestampingOpt;
    double  dwStartOffset;
    int     nRet;
    int i;
    char    szNewFileName[256];
    char    szURL_MRL[256];
    char    szTemp[256];


    //----------------------------------------------------------------------------
    // Set Bank info
    //----------------------------------------------------------------------------
    nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
    nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;

    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
    {
        ;  // nothing
    }
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		;  // nothing
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    {
        nBankOffset = gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset;
    }
	else if(/*gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || */gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
	{
		nBankOffset = 1024;
	}
	else if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
    {
        nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
    }
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		if(gpConfig->gBC[nBoardNum].gdwPlayRate < 2000000)
			nBankOffset = 32;
		else if(gpConfig->gBC[nBoardNum].gdwPlayRate < 4000000)
			nBankOffset = 64;
		else if(gpConfig->gBC[nBoardNum].gdwPlayRate < 8000000)
			nBankOffset = 128;
		else if(gpConfig->gBC[nBoardNum].gdwPlayRate < 16000000)
			nBankOffset = 256;
		else if(gpConfig->gBC[nBoardNum].gdwPlayRate < 32000000)
			nBankOffset = 512;
		else
			nBankOffset = 1024;
	}


    TSPH_SET_SDRAM_BANK_INFO(nBoardNum, nBankNumber, nBankOffset);
    
    //----------------------------------------------------------------------------
    // check IP Streaming
    //----------------------------------------------------------------------------
    if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
    {
        if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
        {
            time_t      tCur;
            struct tm   newtime;

            tCur = time(NULL);
			localtime_r(&tCur, &newtime);
            gUtility.MySprintf(szNewFileName, 256, (char *) "%02d%02d_%02d%02d%02d.ts",
                    newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
        }
    }
    
    nRestampingOpt = 0;
    dwStartOffset = 0;

    //-----------------------------------------------------------
    //--- current filename
    //2011/4/4 ISDB-S Bitrate
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
	{
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS);
	}
	else
	{
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, szFileName);
	}
	//printf("Start Playing Filename : %s\n", gpConfig->gBC[nBoardNum].szCurFileName);

    //--- SetFileSize, Runtime (Basic Program: ShowFileSize)
    gUtilInd.GetFileSizeInSec(szFileName, nBoardNum);

    //--- start offset
    SetCurrentPlayOffset(nBoardNum, lStartPosPercent, (double) gpConfig->gBC[nBoardNum].dwFileSize);     // lStartPosPercent: slide postion

	//2010/12/06 ISDB-S =====================================================================================================================
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
		{
			TSPH_SET_COMBINER_INFO(nBoardNum, 0, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnConstellation,
				gpConfig->gBC[nBoardNum].gnCodeRate, 48);//gpConfig->gBC[nBoardNum].gnSlotCount);
		}
		else if(gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_ALL_0 ||
			gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_ALL_1 ||
			gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_PRBS_2_15 ||
			gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_PRBS_2_23)
		{
			TSPH_SET_COMBINER_INFO(nBoardNum, 0, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnConstellation,
				gpConfig->gBC[nBoardNum].gnCodeRate, 48);//gpConfig->gBC[nBoardNum].gnSlotCount);
		}
		else
		{
			int ts_num;
            ts_num = 0;
			for(int ii = 0 ; ii < MAX_TS_COUNT ; ii++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[ii] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[ii]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[ii] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[ii] <= MAX_SLOT_COUNT)
				{
					ts_num = ts_num + 1;
				}
			}

			if(ts_num == 0)
			{
				gpConfig->gBC[nBoardNum].bPlayingProgress = false;
				gUtilInd.LogMessage("Please, set MULTI-TS COMBINER PARAMETERS.");
				return;
			}
			//2011/4/4 ISDB-S Bitrate
			else
			{
				TSPH_SET_ISDBS_BASE_TS(nBoardNum, gpConfig->gBC[nBoardNum].gszISDBS_MultiCombine_BaseTS);
			}
			//=======================
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
			if(CheckExtention(gpConfig->gBC[nBoardNum].szCurFileName, "t2") == false && CheckExtention(gpConfig->gBC[nBoardNum].szCurFileName, "t2mi") == false)
			{
				for(i = 0; i <  MAX_PLP_TS_COUNT; i++)
				{
					if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]) > 0)
					{
						if(gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]) == true)
						{
							gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]);
							break;
						}
					}
				}
				if(i >= MAX_PLP_TS_COUNT)
				{
					gpConfig->gBC[nBoardNum].bPlayingProgress =  false;
					return;
				}
			}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
			for(i = 0; i < MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					if(gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) == true)
					{
						gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]);
						//printf("DVB-C2 Filename [%s]\n", gpConfig->gBC[nBoardNum].szCurFileName);
						break;
					}
				}
			}
			if(i >= MAX_PLP_TS_COUNT)
			{
				gpConfig->gBC[nBoardNum].bPlayingProgress =  false;
				return;
			}
	}
	//=======================================================================================================================================



    //--- IP Streaming/VLC
    if (/*gGeneral.gnVLCRunning < 0 &&*/
        gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
            if (gpConfig->gBC[nBoardNum].gnIPStreamingMode != NO_IP_STREAM)
            {
                // set the recoorded file name
                if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
                {
                    FILE    *hFile = NULL;

                    // create and check record file
                    gUtility.MySprintf(szTemp, 256, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory,szNewFileName);
					hFile = gUtility.MyFopen(hFile, szTemp,(char *)"ab");
                    if (hFile == NULL)
                    {
                        gUtilInd.LogMessageInt(TLV_FAIL_TO_CREATE_RECORD_FILE);
                        return;
                    }
                    fclose(hFile);
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, szTemp);
                }

                // set the target ULR/MRL
                if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == SEND_IP_STREAM)
                    gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
                else
                    gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo);

            } else if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
            {
                gUtility.MyStrCpy(szURL_MRL, 256, gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
            } else
            {
                gUtility.MyStrCpy(szURL_MRL, 256, (char *) "");
            }

			if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, (char *)"0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
			else
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);


            if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
            else
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
    } else if (gGeneral.gnVLCRunning < 0 &&
        gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
            gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
            gUtility.MyStrCpy(szURL_MRL, 256, (char *) "sout=#duplicate{dst=display} no-sout-all");
            
			if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, (char *)"0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
			else
				TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);

			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
            else
                nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false, gpConfig->gBC[nBoardNum].gnIPStreamingMode, szURL_MRL);
            if (nRet == 0)
                gGeneral.gnVLCRunning = nBoardNum;
    } else
    {
		//2012/2/20 FilePlay Debug Mode
		if(gpConfig->gBC[nBoardNum].gnDebugMode_FilePlay == 1)
		{
			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource, true);
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							2433331, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            1);	// true: loop, false:single //1 -> gpConfig->gBC[nBoardNum].gnOutputClockSource
			else
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            true);
		}
		else 
		{

			if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
                            FIXED_PLAY_RATE_ISDB_T, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource, false);
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							2433331, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            0);	// true: loop, false:single //1 -> gpConfig->gBC[nBoardNum].gnOutputClockSource
           else
                nRet = TSPH_START_PLAY(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName,
							gpConfig->gBC[nBoardNum].gdwPlayRate, dwStartOffset, gpConfig->gBC[nBoardNum].gnOutputClockSource,
                            false);
		}
    }

    if (nRet != 0)
    {
        gUtilInd.LogMessageInt(nRet);
        gpConfig->gBC[nBoardNum].bPlayingProgress = false;
    } else
    {
        if (gGeneral.gnVLCRunning >= 0 &&
            (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM || gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC))
            gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0 * (gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));

            gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
            gpConfig->gBC[nBoardNum].nLastHLDThreadState = 0;
			gpConfig->gBC[nBoardNum].bPlayingProgress =  true;
    }

    gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
}

#endif

#ifndef WIN32
int CWRAP_DLL::CheckExtention(char *szFilePath, char *szExt)
{
	char *pdest;
	int result, len, ext_len;

	len = strlen(szFilePath);
	ext_len = strlen(szExt);
	pdest = strstr(szFilePath, szExt);
	result = pdest - szFilePath + 1;

	return result == (len - ext_len + 1) ? 1 : 0;
}
#endif

//---------------------------------------------------------------------------
// Start Recording
//      decide and set Bankoffset
//      Set filename: mmdd_hhmmss.trp
//      VLC check
//      Start Record
// return 0 if success
#ifdef WIN32 

int CWRAP_DLL::Start_Recording(long nBoardNum)
{
    long        nBankNumber;
    long        nBankOffset;
    double      dwTSCount;
    int         nRet;
    time_t      tCur;
    struct tm   newtime;
    char        szNewFileName[256];
    char        szURL_MRL[256];

    //----------------------------------------------------------------------------
    //--- Set Bank info
    nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
    nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;
	//2010/7/18 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && gpConfig->gBC[nBoardNum].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
	{
		; 
	}
	else
	{
		nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
	}
		// 6.9.16 - DIRECT MODLATION
    nBankOffset = nBankOffset * 2;
	if (nBankOffset > gpConfig->gBC[nBoardNum].gnSubBankOffset)
		nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;

	TSPH_SET_SDRAM_BANK_INFO(nBoardNum, nBankNumber, nBankOffset);

    //----------------------------------------------------------------------------
    //--- Set Filename, AV Decoding. IP Streaming
    dwTSCount = 1;
    tCur = time(NULL);
    localtime_s(&newtime, &tCur);
    gUtility.MySprintf(szNewFileName, 256, "%02d%02d_%02d%02d%02d.ts",
                    newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

    //----------------------------------------------------------------------------
    // Record operation begins here
    gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 512, "%s\\%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, szNewFileName);

	String ^directory;
	directory = gcnew String(gpConfig->gBC[nBoardNum].gszMasterDirectory);
	if(!System::IO::Directory::Exists(directory))
	{
		System::IO::Directory::CreateDirectory(directory);
	}

	//2010/7/18 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && gpConfig->gBC[nBoardNum].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
	{
		if(strlen(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath) < 7)
		{
			String^ szModTag = "";
			String^ szFileName = "";
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T)
			{
				szModTag = "DVB-T";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8)
			{
				szModTag = "8VSB";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
			{
				szModTag = "QAM-A";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
			{
				szModTag = "QAM-B";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK)
			{
				szModTag = "DVB-S";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
			{
				szModTag = "TDMB";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
			{
				szModTag = "16VSB";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
			{
				szModTag = "DVB-H";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			{
				szModTag = "DVB-S2";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
			{
				szModTag = "ISDB-T";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
			{
				szModTag = "ISDB-T-13";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
			{
				szModTag = "DTMB";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
			{
				szModTag = "CMMB";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
			{
				szModTag = "ATSC-MH";
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
			{
				szModTag = "DVB-T2";
			}
			szFileName = gcnew String(gpConfig->gBC[nBoardNum].gszMasterDirectory) + "\\" + (newtime.tm_mon + 1).ToString("0#") + newtime.tm_mday.ToString("0#") + "_" + newtime.tm_hour.ToString("0#") +
				newtime.tm_min.ToString("0#") + newtime.tm_sec.ToString("0#") + "_" + szModTag;

			szFileName = szFileName + ".iq";
			
			gBaseUtil.ChangeCharFromString(szFileName, gpConfig->gBC[nBoardNum].gszIQ_capture_filePath);

		}
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, gpConfig->gBC[nBoardNum].gszIQ_capture_filePath);
	}
	//else
	//{

	//	//----------------------------------------------------------------------------
	//	// BERT
	//	if (gpConfig->gBC[nBoardNum].gnBertPacketType == NO_BERT_OPERTION)
	//	{
	//		// file open as append and close
	//		FILE    *hFile = NULL;
	//
	//		hFile = gUtility.MyFopen(hFile, gpConfig->gBC[nBoardNum].szCurFileName, "ab");
	//		fclose(hFile);
	//	}
	//}
        
    //----------------------------------------------------------------------------
    if (gGeneral.gnVLCRunning < 0 &&
        gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 &&
        dwTSCount > 0 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {

		//2011/7/20 added IP
		long MaxPlayrate;
		MaxPlayrate = gUtilInd.CalcBurtBitrate(nBoardNum);
		//kslee 2010/3/18
		//IP UDP/RTP
		if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
			TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, "0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
		else
			TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);

        nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, MaxPlayrate/*gpConfig->gBC[nBoardNum].gdwPlayRate*/,
                                       0, gpConfig->gBC[nBoardNum].gnModulatorSource, false, RECV_IP_STREAM_REC, szURL_MRL);
        if (nRet == 0)
            gGeneral.gnVLCRunning = nBoardNum;
    } else
    {
        nRet = TSPH_START_RECORD(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnModulatorSource);
    }

    //-------------------------------------------------------------------------
    if (nRet != 0)
    {
        gUtilInd.LogMessageInt(nRet);
        gpConfig->gBC[nBoardNum].bRecordInProgress = false;
    } else
    {
		if((gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))		//2013/5/27 TVB599 0xC
		{
			gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);
		}

		gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0*(gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));
        char    text[256];
        gUtility.MySprintf(text, 256, "TLV_RECORDING:%s",szNewFileName);
        gUtilInd.LogMessage(text);

        gpConfig->gBC[nBoardNum].bRecordInProgress = true;
        gpConfig->gBC[nBoardNum].nWaitProgressCnt = 0;

		//2011/12/2 TVB594
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			if(gpConfig->gBC[nBoardNum].gnRealandvir_cnt > 1)
			{
				for(int i = 0 ; i <= MAX_BOARD_COUNT ; i++)
				{
					if(((gpConfig->gBC[nBoardNum].gnRealandvir_location >> i) & 0x1) == 1)
					{
						if(gpConfig->gBC[i].bPlayingProgress == true)
						{
							gpConfig->gBC[i].gnPause = 0;
							TSPH_START_DELAY(i, 0);

	                        //stop quickly
		                    TSPL_SET_TSIO_DIRECTION_EX(i, FILE_SRC);
			                gpConfig->gBC[i].bPlayingProgress = false;

	                        TSPH_START_MONITOR(i, 0);
						}
					}
				}
			}
		}
	}
    
    return nRet;
}

#else
int CWRAP_DLL::Start_Recording(long nBoardNum)
{
    long        nBankNumber;
    long        nBankOffset;
    double      dwTSCount;
    int         nRet;
    time_t      tCur;
    struct tm   newtime;
    char        szNewFileName[256];
    char        szURL_MRL[256];

    //----------------------------------------------------------------------------
    //--- Set Bank info
    nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
    nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;
	//2010/7/18 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && gpConfig->gBC[nBoardNum].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
	{
		; 
	}
	else
	{
		nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
	}
		// 6.9.16 - DIRECT MODLATION
        nBankOffset = nBankOffset * 2;
		if (nBankOffset > gpConfig->gBC[nBoardNum].gnSubBankOffset)
			nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;

	TSPH_SET_SDRAM_BANK_INFO(nBoardNum, nBankNumber, nBankOffset);

    //----------------------------------------------------------------------------
    //--- Set Filename, AV Decoding. IP Streaming
    dwTSCount = 1;
    tCur = time(NULL);
	localtime_r(&tCur, &newtime);
    gUtility.MySprintf(szNewFileName, 256, (char *) "%02d%02d_%02d%02d%02d.ts",
                    newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

	
    //----------------------------------------------------------------------------
    // Record operation begins here
	gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 512, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, szNewFileName);
	//2010/7/18 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && gpConfig->gBC[nBoardNum].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
	{
		if(strlen(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath) < 7)
		{
			char szModTag[16];
			char  szFileName[256];
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-T");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"8VSB");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"QAM-A");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"QAM-B");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-S");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"TDMB");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"16VSB");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-H");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-S2");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"ISDB-T");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"ISDB-T-13");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DTMB");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"CMMB");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"ATSC-MH");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-T2");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"ISDB-S");
			}
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
			{
				gUtility.MySprintf(szModTag, 16, (char *)"%s", (char *)"DVB-C2");
			}
			gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath, 512, (char *)"%s/%02d%02d%02d%02d%02d_%s.iq", gpConfig->gBC[nBoardNum].gszMasterDirectory, (newtime.tm_mon + 1),
								newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec, szModTag);

		}

		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, gpConfig->gBC[nBoardNum].gszIQ_capture_filePath);
	}
	else
	{

		//----------------------------------------------------------------------------
		// BERT
		if (gpConfig->gBC[nBoardNum].gnBertPacketType == NO_BERT_OPERTION)
		{
			// file open as append and close
			FILE    *hFile = NULL;
	
			hFile = gUtility.MyFopen(hFile, gpConfig->gBC[nBoardNum].szCurFileName, (char *)"ab");
			fclose(hFile);
		}
	}
        
    //----------------------------------------------------------------------------
    if (gGeneral.gnVLCRunning < 0 &&
        gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 &&
        dwTSCount > 0 &&
        gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
		if(gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP == 0)
			TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, (char *)"0.0.0.0", gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);
		else
			TSPH_SET_RX_IP_STREAMING_INFO(nBoardNum, gpConfig->gBC[nBoardNum].gszIP_RxIP, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, gpConfig->gBC[nBoardNum].gnIP_RxPort, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 0, 0);

        nRet = TSPH_START_IP_STREAMING(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gdwPlayRate,
                                       0, gpConfig->gBC[nBoardNum].gnModulatorSource, false, RECV_IP_STREAM_REC, szURL_MRL);
        if (nRet == 0)
            gGeneral.gnVLCRunning = nBoardNum;
    } else
    {
    	//printf("******* START RECORD: Filename = (%s)\n", gpConfig->gBC[nBoardNum].szCurFileName);
        nRet = TSPH_START_RECORD(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnModulatorSource);
    }

    //-------------------------------------------------------------------------
    if (nRet != 0)
    {
        gUtilInd.LogMessageInt(nRet);
        gpConfig->gBC[nBoardNum].bRecordInProgress = false;
    } else
    {
        gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0*(gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));
        if (gpConfig->gBC[nBoardNum].gnBertPacketType == NO_BERT_OPERTION)
        {
            char    text[256];
            gUtility.MySprintf(text, 256, (char *) "TLV_RECORDING:%s",szNewFileName);
            gUtilInd.LogMessage(text);

        }
        gpConfig->gBC[nBoardNum].bRecordInProgress = true;
        gpConfig->gBC[nBoardNum].nWaitProgressCnt = 0;

		//2011/12/2 TVB594
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			if(gpConfig->gBC[nBoardNum].gnRealandvir_cnt > 0)
			{
				for(int i = 0 ; i < MAX_BOARD_COUNT ; i++)
				{
					if(((gpConfig->gBC[nBoardNum].gnRealandvir_location >> i) & 0x1) == 1)
					{
						if(gpConfig->gBC[i].bPlayingProgress == true)
						{
							gpConfig->gBC[i].gnPause = 0;
							TSPH_START_DELAY(i, 0);

	                        //stop quickly
		                    TSPL_SET_TSIO_DIRECTION_EX(i, FILE_SRC);
			                gpConfig->gBC[i].bPlayingProgress = false;

	                        TSPH_START_MONITOR(i, 0);
						}
					}
				}
			}
		}
	}
    
    return nRet;
}

#endif
//---------------------------------------------------------------------------
// Stop Delaying
void CWRAP_DLL::Stop_Delaying(long nBoardNum)
{
    if (gpConfig->gBC[nBoardNum].bDelayinProgress == true)
    {
        gpConfig->gBC[nBoardNum].bDelayinProgress = false;
        TSPH_START_MONITOR(nBoardNum, 6);
        gUtilInd.LogMessageInt(TLV_DELAYING_STOPPED);
    }
}

//---------------------------------------------------------------------------
// Stop Playing
void CWRAP_DLL::Stop_Playing(long nBoardNum)
{
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
    {
#ifdef WIN32
		gpConfig->gBC[nBoardNum].gnSingleTone = 0;
		TVB380_SET_MODULATOR_SINGLE_TONE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSingleTone); 
#endif
		//2011/10/24 added PAUSE
		gpConfig->gBC[nBoardNum].gnPause = 0;
		TSPH_START_DELAY(nBoardNum, 0);
        //stop quickly
		TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, FILE_SRC);
        gpConfig->gBC[nBoardNum].bPlayingProgress = false;

        TSPH_START_MONITOR(nBoardNum, 0);
        gUtilInd.LogMessageInt(TLV_PLAYING_STOPPED);

#ifdef WIN32
		if((gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))		//2013/5/27 TVB599 0xC
		{
			gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, 1);
		}
#endif
		//2011/12/2 TVB594
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			int ownerSlot;
			int _Val_;
			if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
			{
				ownerSlot = gpConfig->gBC[nBoardNum].gn_OwnerSlot;
				if(gpConfig->gBC[ownerSlot].bPlayingProgress == true)
				{
					if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
						_Val_ = ((gpConfig->gBC[ownerSlot].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[ownerSlot].gn_StreamNum & 0x3);
					else
						_Val_ = gpConfig->gBC[ownerSlot].gn_StreamNum;

					TSPH__SEL_TS_of_ASI310OUT(ownerSlot, _Val_);
				}
				else
				{
					if(gpConfig->gBC[ownerSlot].gnRealandvir_cnt > 1)
					{
						for(int i = 0 ; i <= MAX_BOARD_COUNT ; i++)
						{
							if(((gpConfig->gBC[ownerSlot].gnRealandvir_location >> i) & 0x1) == 1)
							{
								if(gpConfig->gBC[i].bPlayingProgress == true && i != nBoardNum)
								{
									if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
										_Val_ = ((gpConfig->gBC[i].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[i].gn_StreamNum & 0x3); 
									else
										_Val_ = gpConfig->gBC[i].gn_StreamNum;
									TSPH__SEL_TS_of_ASI310OUT(i, _Val_);
								}
							}
						}
					}
				}
			}
			else
			{
				ownerSlot = nBoardNum;
				if(gpConfig->gBC[ownerSlot].gnRealandvir_cnt > 1)
				{
					for(int i = 0 ; i  <= MAX_BOARD_COUNT ; i++)
					{
						if(((gpConfig->gBC[ownerSlot].gnRealandvir_location >> i) & 0x1) == 1)
						{
							if(gpConfig->gBC[i].bPlayingProgress == true)
							{
								if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
									_Val_ = ((gpConfig->gBC[i].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[i].gn_StreamNum & 0x3); 
								else
									_Val_ = gpConfig->gBC[i].gn_StreamNum;
								TSPH__SEL_TS_of_ASI310OUT(i, _Val_);
								break;
							}
						}
					}
				}
			}
		}

        //----------------------------------------------------------------------------
        //sskim20071130 - A/V decoding, IP Streaming
        //----------------------------------------------------------------------------
        if (gGeneral.gnVLCRunning == nBoardNum &&
            (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 || gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1))
            gGeneral.gnVLCRunning = -1;

        //TVB595V1, 2010/10/5 TVB593
        if (gpConfig->gBC[nBoardNum].gnBoardId == 59 ||  gpConfig->gBC[nBoardNum].gnBoardId == 60 || gpConfig->gBC[nBoardNum].gnBoardId == 61 || gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27)	//2013/5/27 TVB599 0xC
            TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
#ifdef WIN32    
		gGeneral.gnValid_PlayStop = 1;
		gpConfig->gBC[nBoardNum].gnBoardStatusReset = 1;
#endif
	}
}

//---------------------------------------------------------------------------
// Stop Recording
void CWRAP_DLL::Stop_Recording(long nBoardNum)
{
    if (gpConfig->gBC[nBoardNum].bRecordInProgress == true)
    {
        gpConfig->gBC[nBoardNum].bRecordInProgress = false;
        
        TSPH_START_MONITOR(nBoardNum, 0);
        gUtilInd.LogMessageInt(TLV_RECODRING_STOPPED);
        
#ifdef WIN32
 		if((gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))		//2013/5/27 TVB599 0xC
		{
			gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, 1);
		}
#endif
       //----------------------------------------------------------------------------
        //sskim20071130 - A/V decoding, IP Streaming
        //----------------------------------------------------------------------------
        if (gGeneral.gnVLCRunning == nBoardNum &&
            (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 || gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1))
            gGeneral.gnVLCRunning = -1;

        //TVB595V1
        if (gpConfig->gBC[nBoardNum].gnBoardId == 59 ||  gpConfig->gBC[nBoardNum].gnBoardId == 60 || gpConfig->gBC[nBoardNum].gnBoardId == 61 || gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27)	//2013/5/27 TVB599 0xC
            TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
		
		//2011/11/18 IQ NEW FILE FORMAT
#ifdef WIN32
		if(gpConfig->gBC[nBoardNum].gnIQ_capture_support == 1 && gpConfig->gBC[nBoardNum].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
		{
			IQ_CapturedFile_Error_Check();
		}
#endif
	}
}
#ifdef WIN32
//2011/11/21 IQ NEW FILE FORMAT
#define	SWAP(j)						\
{									\
	j = ((j & 0xff) << 24) |		\
		(((j >> 8) & 0xff) << 16) | \
		(((j >> 16) & 0xff) << 8) | \
		(((j >> 24)& 0xff));		\
}
void CWRAP_DLL::IQ_CapturedFile_Error_Check()
{
	FormCollection^ forms1;
	forms1 = Application::OpenForms;
	__int64 readSize = 0;
	__int64 fileSize = 0;
	__int64 limit_errorcheck;
	long	nRet = 0;
	long	readData;
	long	CC = 0;
	long    pre_CC = 0;
	__int64    progressStep;
	long	progressValue = 1;

	long nBoardNum = gGeneral.gnActiveBoard;

	if(gpConfig->gBC[nBoardNum].gnIQ_ErrorCheck == 1)
	{
		Sleep(1000);
		Form_Wait FW;
		
		if(gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize > 0)
		{
			limit_errorcheck = ((__int64)gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize * 1024 * 1024);
		}
		else
		{
			limit_errorcheck = -1;
		}
		
		for(int i=0; i < forms1->Count; i++)
		{
			if(forms1[i]->Name == "PlayForm")
			{
				FW.Show(safe_cast<PlayForm^>(forms1[i])->Owner);
				break;
			}
		}
		if(gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath) == true)
		{
			FILE *fp = NULL;
			fp = gUtility.MyFopen(fp, gpConfig->gBC[nBoardNum].gszIQ_capture_filePath, "rb");
			if(fp == NULL)
			{
				MessageBox::Show("[IQ] Captured file fail to open.");
				return;
			}
			
			fseek(fp, 0, SEEK_END);
			fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			nRet = fread(&readData, sizeof(long), 1, fp);
			nRet = fread(&readData, sizeof(long), 1, fp);

			if(fileSize < (16 * 1024 * 1024))
			{
				FW.Close();
				return;
			}

			if(limit_errorcheck == -1 || limit_errorcheck > fileSize)
			{
				progressStep = (fileSize - (16 * 1024 * 1024)) / 100;
				limit_errorcheck = (fileSize - (16 * 1024 * 1024));
			}
			else
			{
				limit_errorcheck = limit_errorcheck - (16 * 1024 * 1024);
				progressStep = limit_errorcheck /100;
			}

			FW.Init_Form(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath, (long)(fileSize / (1024 * 1024)), (long)(limit_errorcheck / (1024 * 1024)));

			while(1)
			{
				
				nRet = fread(&readData, sizeof(long), 1, fp);
				if(feof(fp))
					break;
				if (!nRet)
				{
					MessageBox::Show("[IQ] Captured file fail to read.");
					break;
				}
				readSize = readSize + 4;
				

				if((progressStep * progressValue)  < readSize)
				{
					FW.SetProgressValue(progressValue);
					progressValue++;
				}

				if(limit_errorcheck < readSize)
				{
					break;
				}
					

				SWAP(readData);
				CC = (readData >> 28) & 0xf;
				if(pre_CC == 0)
				{
					if(CC == 1)
					{
						pre_CC = 15;
					}
					else
					{
						pre_CC = CC - 1;
					}
				}
				if(CC == 1)
				{
					if(pre_CC != 15)
					{
						MessageBox::Show("[IQ CAPTURE] Continuity count error. [" + (readSize + 8).ToString() + "]bytes" );
						break;
					}
				}
				else if( CC != (pre_CC + 1))
				{
					MessageBox::Show("[IQ CAPTURE] Continuity count error. [" + (readSize + 8).ToString() + "]bytes" );
					break;
				}

				pre_CC = CC;
			}
			fclose(fp);
		}
		else
		{

		}
		FW.Close();
	}
}
#endif
//---------------------------------------------------------------------------
// Toggle Loop Mode
void CWRAP_DLL::ToggleLoopMode(long nBoardNum)
{
    int nRet;

    if (gpConfig->gBC[nBoardNum].gbRepeatMode == true)
    {
        gUtilInd.LogMessageInt(TLV_ONCE_PLAY_MODE);
        if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
        {
            nRet = TSPH_SET_REPEAT_PLAY_MODE(nBoardNum, 0);
            if (nRet != 0)
                 gUtilInd.LogMessageInt(nRet);
        }
        gpConfig->gBC[nBoardNum].gbRepeatMode = false;

    } else
    {
         gUtilInd.LogMessageInt(TLV_LOOP_PLAY_MODE);
        if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
        {
            nRet = TSPH_SET_REPEAT_PLAY_MODE(nBoardNum, 0);
            if (nRet != 0)
                 gUtilInd.LogMessageInt(nRet);
        }
        gpConfig->gBC[nBoardNum].gbRepeatMode = true;
    }
}

//---------------------------------------------------------------------------
// Set RF Power Level.
#ifndef WIN32
void CWRAP_DLL::UpdateRFPowerLevel(long nBoardNum, double atten, long agc)
{
	double dwMax, dwMin, dwLevel, dwAGCAttenOffset, dwRet;
    char str[256];
	//2010/7/14
	dwRet = TVB380_SET_MODULATOR_AGC_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, agc, gpConfig->gBC[nBoardNum].gnUseTAT4710);
    dwRet = TVB380_SET_MODULATOR_ATTEN_VALUE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, atten, gpConfig->gBC[nBoardNum].gnUseTAT4710);

	dwMax = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 5);
    dwMin = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 6);
    dwLevel = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 9);
    dwAGCAttenOffset = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 7);
    //dwAtten = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gnModulatorMode(nBoardNum), 8)
    
	//kslee091218 
	gpConfig->gBC[nBoardNum].gnDisRFLevel = dwLevel;
	gpConfig->gBC[nBoardNum].gnAtten_Max = dwMax;
	gpConfig->gBC[nBoardNum].gnAtten_Min = dwMin;

    gpConfig->gBC[nBoardNum].gnMaxRFLevel = (double)dwMax;
    gpConfig->gBC[nBoardNum].gnMinRFLevel = (double)dwMin;
}
//2011/1/6 ISDB-T TMCC Setting ======================================================================================
void CWRAP_DLL::InitTMCCVarible(long nBoardNum)
{
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
	{
		gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.partial = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info), (char *)"");
	}
	else
	{
		gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.partial = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 13;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info), (char *)"");
	}
}
#else
void CWRAP_DLL::InitTMCCVarible(long nBoardNum)
{
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
	{
		gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.partial = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info), (char *)"");
	}
	else
	{
		gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.partial = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 13;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info), (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = 0;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, sizeof(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info), (char *)"");
	}
}
#endif

#ifdef WIN32
int CWRAP_DLL::HaveTmccInformation(long nBoardNum, char *strFilePath)
{
    int     i, j, nBit;
    long    nRet;
    long    nRemux_Total_Bitrate;
	long   nSegBitrate_sum = 0;
    long    nNumOfSeg[3];
    long    nPartialReception, nBroadcastType;
    long    nRemuxInfo[3];
    long    NumOfSegs;

	for (j = 0; j <= 2; j++)
    {
        nRet = TSPH_GET_REMUX_INFO(nBoardNum, strFilePath, j + 3);
		nRemuxInfo[j] = TSPH_GET_REMUX_INFO(nBoardNum, strFilePath, j);
		if(nRet > 0)
		{
			nSegBitrate_sum = nSegBitrate_sum + nRet;
		}
		nNumOfSeg[j] = 0;
        for (i = 1; i <= 19; i++)
        {
            if (nRemuxInfo[j] & 1)          // *************** nRemuxInfo[] And 1********* CHECK CHECK
                nBit = 1;
            else
                nBit = 0;

            if (i <= 4)
                nNumOfSeg[j] = (long) (nNumOfSeg[j] + pow(( float) 2,( i - 1)) * nBit);
            else if (i == 18)
                nPartialReception = nBit;
            else if(i == 19)
                nBroadcastType = nBit;

            nRemuxInfo[j] = (long) (nRemuxInfo[j] / 2);
        }
	}
	nRemux_Total_Bitrate = TSPH_GET_REMUX_INFO(nBoardNum, strFilePath, 6);
	if(nRemux_Total_Bitrate < 0)
		return 0;

	if(nRemux_Total_Bitrate != nSegBitrate_sum)
		return 0;

	NumOfSegs = nNumOfSeg[0] + nNumOfSeg[1] + nNumOfSeg[2];

	if (nBroadcastType == 0)
    {
        if (nPartialReception == 1)
        {
            if (NumOfSegs <= 0 || NumOfSegs > 13)
			{
				return 0;
			}
        } else
        {
            if (NumOfSegs != 13)
			{
				return 0;
			}
        }
    } else if (nBroadcastType == 1)
    {
        if (NumOfSegs != 1)
		{
				return 0;
		}
    } else if (nBroadcastType == 2)
    {
        if (nPartialReception == 1)
        {
            if (NumOfSegs <= 0 || NumOfSegs > 3)
			{
				return 0;
			}
        } else
        {
            if (NumOfSegs != 3)
			{
				return 0;
			}
        }
    }

	return 1;
}
#endif
//--------------------------------------------------------------------------
// Get TMCC Remux Information
// Return Remux Info String
int CWRAP_DLL::UpdateRemuxInfo(long nBoardNum, char *szFilePath, char *szRemuxInfo)
{
    long    nBit,j;
    int     i;
    long    nRemuxInfo[3];
    long    nRemux_Bitrate[3];
    long    nNumOfSeg[3];
    long    nCodeRate[3];
    long    nConstellation[3];
    long    nTimeInterleaving[3];
    long    nMode, nGuardInterval;
    long    nPartialReception, nBroadcastType;
    char    strTemp[100];
    long    nRemux_Total_Bitrate;
	
	//2011/11/22 improve ISDB-T UI
	long	nRet = 1;
    
	if (gUtility.Is_File_Exist(szFilePath) == false)
	{
		gUtility.MyStrCpy(szRemuxInfo, 2048, "File is not exist!!!");
		return 0;
	}

#ifndef WIN32
	//2011/1/6 ISDB-T TMCC Setting
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
	{
		InitTMCCVarible(nBoardNum);
		//2011/1/27 
		gGui.Set_TMCC_Remuxing();
	}
//#ifdef STANDALONE
//		gGeneral.gnPIDCount = 0;
//		gGui.SNMP_Send_Status(TVB390_TMCC_PID_COUNT);
//	}
//#else
//		printf("START Run_Ts_Parser\n");	
//		i = gWrapDll.Run_Ts_Parser(nBoardNum, szFilePath, gpConfig->gBC[nBoardNum].gdwPlayRate);
//		printf("END Run_Ts_Parser\n");
//	}
//	if (i == -1)
//		return 0;
//#endif
#endif

    for (j = 0; j <= 2; j++)
    {
        nRemuxInfo[j] = TSPH_GET_REMUX_INFO(nBoardNum, szFilePath, j);
        nRemux_Bitrate[j] = TSPH_GET_REMUX_INFO(nBoardNum, szFilePath, j + 3);
                
        nNumOfSeg[j] = 0;
        nCodeRate[j] = 0;
        nConstellation[j] = 0;
        nTimeInterleaving[j] = 0;

        nMode = 0;
        nGuardInterval = 0;
        nPartialReception = 0;
        nBroadcastType = 0;
        nBit = 0;

        for (i = 1; i <= 19; i++)
        {
            if (nRemuxInfo[j] & 1)          // *************** nRemuxInfo[] And 1********* CHECK CHECK
                nBit = 1;
            else
                nBit = 0;

            if (i <= 4)
                nNumOfSeg[j] = (long) (nNumOfSeg[j] + pow(( float) 2,( i - 1)) * nBit);
            else if (i <= 7)
                nCodeRate[j] =  (long) (nCodeRate[j] + pow(( float) 2,(i - 5)) * nBit);
            else if (i <= 10)
                nConstellation[j] =  (long) (nConstellation[j] + pow( (float) 2,(i - 8)) * nBit);
            else if (i <= 13)
                nTimeInterleaving[j] =  (long) (nTimeInterleaving[j] + pow( (float) 2,(i - 11)) * nBit);
            else if (i <= 15)
                nMode =  (long) (nMode + pow( (float) 2,(i - 14)) * nBit) ;
            else if (i <= 17)
                nGuardInterval =  (long) (nGuardInterval + pow( (float) 2,(i - 16)) * nBit);
            else if (i <= 18)
                nPartialReception = nBit;
            else
                nBroadcastType = nBit;

            nRemuxInfo[j] = (long) (nRemuxInfo[j] / 2);
        }
    }
    
    nRemux_Total_Bitrate = TSPH_GET_REMUX_INFO(nBoardNum, szFilePath, 6);

    //------------------------------------------------------------------
    // Set Information

    //--- broadcast type
    if (nBroadcastType == 0)
    {
        gUtility.MyStrCpy(szRemuxInfo,2048, "broadcast type=television\r\n");
    } else
    {
        if (nPartialReception == 1)
            gUtility.MyStrCpy(szRemuxInfo, 2048, "broadcast type=radio 3 seg.\r\n");
        else
            gUtility.MyStrCpy(szRemuxInfo, 2048, "broadcast type=radio 1 seg.\r\n");
    }

    //--- mode
    gUtility.MySprintf(strTemp, 100, "mode=%d\r\n", nMode);
    gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);

    //--- guard interval
    if (nGuardInterval == 0)
        gUtility.MyStrCat(szRemuxInfo, 2048, "guard interval=1/32\r\n");
    else if (nGuardInterval == 1)
        gUtility.MyStrCat(szRemuxInfo, 2048, "guard interval=1/16\r\n");
    else if (nGuardInterval == 2)
        gUtility.MyStrCat(szRemuxInfo, 2048, "guard interval=1/8\r\n");
    else if (nGuardInterval == 3)
        gUtility.MyStrCat(szRemuxInfo, 2048, "guard interval=1/4\r\n");
    else
        gUtility.MyStrCat(szRemuxInfo, 2048, "guard interval=unknown\r\n");

    //--- Partial Reception
    if (nPartialReception == 1)
        gUtility.MyStrCat(szRemuxInfo, 2048, "partial reception=yes\r\n");
    else if (nPartialReception == 0)
        gUtility.MyStrCat(szRemuxInfo, 2048, "partial reception=no\r\n");
    else
        gUtility.MyStrCat(szRemuxInfo, 2048, "partial reception=unknown\r\n");

    //--- Total Bitrate
    if (nRemux_Total_Bitrate > 0)
    {
        gUtility.MySprintf(strTemp, 100, "data rate=%d bps\r\n", nRemux_Total_Bitrate);
        gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);
    } else
    {
        gUtility.MyStrCat(szRemuxInfo, 2048, "data rate=unknown\r\n");
    }

    //--- Layer Information
    for (j = 0; j <= 2; j++)
    {
        gUtility.MyStrCat(szRemuxInfo, 2048, "\r\n");
        gUtility.MySprintf(strTemp, 100, "layer %c\r\n", 65+j); // layer ABC
        gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);
        gUtility.MySprintf(strTemp, 100, "segment=%d\r\n", nNumOfSeg[j]); //
        gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);

        if (nNumOfSeg[j] > 0 && nNumOfSeg[j] <= 13)
        {
            if (nConstellation[j] == 0)
                gUtility.MySprintf(strTemp, 100, "modulation=DQPSK\r\n");
            else if (nConstellation[j] == 1)
                gUtility.MySprintf(strTemp, 100, "modulation=QPSK\r\n");
            else if (nConstellation[j] == 2)
                gUtility.MySprintf(strTemp, 100, "modulation=16QAM\r\n");
            else if (nConstellation[j] == 3)
                gUtility.MySprintf(strTemp, 100, "modulation=64QAM\r\n");
            else
                gUtility.MySprintf(strTemp, 100, "modulation=unknown\r\n");
            gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);
            
            if (nCodeRate[j] == 0)
                gUtility.MySprintf(strTemp, 100, "code rate=1/2\r\n");
            else if (nCodeRate[j] == 1)
                gUtility.MySprintf(strTemp, 100, "code rate=2/3\r\n");
            else if (nCodeRate[j] == 2)
                gUtility.MySprintf(strTemp, 100, "code rate=3/4\r\n");
            else if (nCodeRate[j] == 3)
                gUtility.MySprintf(strTemp, 100, "code rate=5/6\r\n");
            else if (nCodeRate[j] == 4)
                gUtility.MySprintf(strTemp, 100, "code rate=7/8\r\n");
            else
                gUtility.MySprintf(strTemp, 100, "code rate=unknown\r\n");
            gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);
                        
			//----------------------------------------------------------------------
			//--- 20090812
            gUtility.MySprintf(strTemp,  100, "time interleave=%d\r\n", nTimeInterleaving[j]);
            gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);

            gUtility.MyStrCat(szRemuxInfo, 2048, "time interleave=");
            if (nMode == 0)
			{
                if (nTimeInterleaving[j] == 0)
					gUtility.MyStrCat(szRemuxInfo, 2048, "0");
                else if (nTimeInterleaving[j] == 1)
					gUtility.MyStrCat(szRemuxInfo, 2048, "4");
                else if (nTimeInterleaving[j] == 2)
					gUtility.MyStrCat(szRemuxInfo, 2048, "8");
                else if (nTimeInterleaving[j] == 3)
					gUtility.MyStrCat(szRemuxInfo, 2048, "16");
				else
					gUtility.MyStrCat(szRemuxInfo, 2048, "unknown");    
			} else if (nMode == 1)
			{
                if (nTimeInterleaving[j] == 0)
					gUtility.MyStrCat(szRemuxInfo, 2048, "0");
                else if (nTimeInterleaving[j] == 1)
					gUtility.MyStrCat(szRemuxInfo, 2048, "2");
                else if (nTimeInterleaving[j] == 2)
					gUtility.MyStrCat(szRemuxInfo, 2048, "4");
                else if (nTimeInterleaving[j] == 3)
					gUtility.MyStrCat(szRemuxInfo, 2048, "8");
                else
					gUtility.MyStrCat(szRemuxInfo, 2048, "unknown");    
			} else
			{
                if (nTimeInterleaving[j] == 0)
					gUtility.MyStrCat(szRemuxInfo, 2048, "0");
                else if (nTimeInterleaving[j] == 1)
					gUtility.MyStrCat(szRemuxInfo, 2048, "1");
                else if (nTimeInterleaving[j] == 2)
					gUtility.MyStrCat(szRemuxInfo, 2048, "2");
                else if (nTimeInterleaving[j] == 3)
					gUtility.MyStrCat(szRemuxInfo, 2048, "3");
                else
					gUtility.MyStrCat(szRemuxInfo, 2048, "unknown");    
			}
            gUtility.MyStrCat(szRemuxInfo, 2048, "\r\n");
			//----------------------------------------------------------------------

            if (nRemux_Bitrate[j] > 0)
                gUtility.MySprintf(strTemp,  100, "data rate=%d bps\r\n", nRemux_Bitrate[j]);
            else
                gUtility.MySprintf(strTemp,  100, "data rate=unknown\r\n");
            gUtility.MyStrCat(szRemuxInfo, 2048, strTemp);
        }
    }

    //--- Check validity
    long    NumOfSegs;
    char    szErrorMsg[200];
    
    NumOfSegs = nNumOfSeg[0] + nNumOfSeg[1] + nNumOfSeg[2];
    gUtility.MyStrCpy(szErrorMsg, 200, "!!! TMCC parameters required !!!\r\n");

    if (nBroadcastType == 0)
    {
        if (nPartialReception == 1)
        {
            if (NumOfSegs <= 0 || NumOfSegs > 13)
			{
				gUtility.MyStrCpy(szRemuxInfo, 2048, szErrorMsg);
				nRet = 0;
			}
        } else
        {
            if (NumOfSegs != 13)
			{
				gUtility.MyStrCpy(szRemuxInfo, 2048, szErrorMsg);
				nRet = 0;
			}
        }
    } else if (nBroadcastType == 1)
    {
        if (NumOfSegs != 1)
		{
			gUtility.MyStrCpy(szRemuxInfo, 2048, szErrorMsg);
			nRet = 0;
		}
    } else if (nBroadcastType == 2)
    {
        if (nPartialReception == 1)
        {
            if (NumOfSegs <= 0 || NumOfSegs > 3)
			{
				gUtility.MyStrCpy(szRemuxInfo, 2048, szErrorMsg);
				nRet = 0;
			}
        } else
        {
            if (NumOfSegs != 3)
			{
				gUtility.MyStrCpy(szRemuxInfo, 2048, szErrorMsg);
				nRet = 0;
			}
        }
    }
#ifndef WIN32
	if(nRet == 1)
	{
			gpConfig->gBC[nBoardNum].tmccInfo.broadcast = nBroadcastType;
			gpConfig->gBC[nBoardNum].tmccInfo.mode = nMode - 1;
			gpConfig->gBC[nBoardNum].tmccInfo.guard = nGuardInterval;
			gpConfig->gBC[nBoardNum].tmccInfo.partial = nPartialReception;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = nNumOfSeg[0];
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = nConstellation[0];
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = nCodeRate[0];
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = nTimeInterleaving[0];
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = nRemux_Bitrate[0];
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = nNumOfSeg[1];
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = nConstellation[1];
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = nCodeRate[1];
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = nTimeInterleaving[1];
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = nRemux_Bitrate[1];
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = nNumOfSeg[2];
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = nConstellation[2];
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = nCodeRate[2];
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = nTimeInterleaving[2];
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = nRemux_Bitrate[2];
	}
#endif
	return nRet;
}


//==========================================================================
//--- Simple HLD/LLD API
//==========================================================================

//---------------------------------------------------------------------------
int CWRAP_DLL::Check_Ln_Ex(int nSlot, char *ln)
{
    return TSPL_CHECK_LN_EX(nSlot, ln);
}

//---------------------------------------------------------------------------
double CWRAP_DLL::Get_Current_Record_Point(long nBoardNum)
{
    return TSPH_GET_CURRENT_RECORD_POINT(nBoardNum);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Get_Current_Thread_State(long nBoardNum)
{
    return TSPH_GET_CURRENT_THREAD_STATE(nBoardNum);
}

//---------------------------------------------------------------------------
double CWRAP_DLL::Get_Modulator_Bert_Result_Ex(long nBoardNum, long modulator_type)
{
    return TVB380_GET_MODULATOR_BERT_RESULT_EX(nBoardNum, modulator_type);
}

//---------------------------------------------------------------------------
float CWRAP_DLL::Get_Modulator_Rf_Power_Level_Ex(int nSlot, long modulator_type, long  lType)
{
    return (float) TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nSlot, modulator_type, lType);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Get_Play_Buffer_Status(long nBoardNum)
{
    return TSPH_GET_PLAY_BUFFER_STATUS(nBoardNum);
}

//---------------------------------------------------------------------------
// iType: 0: bitrate, 1:packet size
long CWRAP_DLL::Get_Playrate(int iSlot, char *strFile, int iType)
{
    return TSPH_CAL_PLAY_RATE(iSlot, strFile, iType);
}

//---------------------------------------------------------------------------
long    CWRAP_DLL::Get_Program(int nSlot, long nIndex)
{
#ifdef WIN32
    return TSPH_GET_PROGRAM(nSlot, nIndex);
#else
	return 1;
#endif
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Get_Remux_Datarate(int nSlot, long layer_index)
{
    return TSPH_GET_REMUX_DATARATE(nSlot, layer_index);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Get_Remux_Info(int nSlot, char *szFile, int layer_index)
{
    return TSPH_GET_REMUX_INFO(nSlot, szFile, layer_index);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Is_Video_Window_Visible(int nSlot)
{
#ifdef WIN32
    return TSPH_IS_VIDEO_WINDOW_VISIBLE(nSlot);
#else
	return 1;
#endif
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Move_Video_Window(int nSlot, int nX, int nY, int nW, int nH)
{
#ifdef WIN32
    return TSPH_MOVE_VIDEO_WINDOW(nSlot, nX, nY, nW, nH);
#else
	return 1;
#endif
}

//---------------------------------------------------------------------------
unsigned long CWRAP_DLL::Read_Input_Status(int nSlot)
{
    return TSPL_READ_INPUT_STATUS_EX(nSlot);
}

//---------------------------------------------------------------------------
__int64 CWRAP_DLL::Read_Input_Tscount_Ex(int nSlot)
{
    return  (__int64) TSPL_READ_INPUT_TSCOUNT_EX(nSlot);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Read_Ip_Streaming_Input_Status(int nSlot, long nStatus)
{
    return TSPH_READ_IP_STREAMING_INPUT_STATUS(nSlot, nStatus);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Run_Ts_Parser2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo)
{
    return TSPH_RUN_TS_PARSER2(nSlot, szFile, default_bitrate, ppTsInfo);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Board_Config_Status_Ex(int nSlot, long modulator_type, long status)
{
    return TVB380_SET_BOARD_CONFIG_STATUS_EX(nSlot, modulator_type, status);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Current_Offset(int nSlot, int nOffsetType, 	double dwOffset)	//0=start offset, 1==current offset, 2==end offset
{
    return TSPH_SET_CURRENT_OFFSET(nSlot, nOffsetType, dwOffset);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Error_Injection(int nSlot, long error_lost, long error_lost_packet,
										 long error_bits, long error_bits_packet, long error_bits_count,
										 long error_bytes, long error_bytes_packet, long error_bytes_count)
{
    return TSPH_SET_ERROR_INJECTION(nSlot, error_lost, error_lost_packet,
                             error_bits, error_bits_packet, error_bits_count,
                             error_bytes, error_bytes_packet, error_bytes_count);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Layer_Info(int nSlot,
									int other_pid_map_to_layer,
									int multi_pid_map,
									int total_pid_count, char* total_pid_info,
									int a_pid_count, char* a_pid_info,
									int b_pid_count, char* b_pid_info,
									int c_pid_count, char* c_pid_info)
{
    return TSPH_SET_LAYER_INFO(nSlot, other_pid_map_to_layer, multi_pid_map,
									total_pid_count, total_pid_info,
									a_pid_count, a_pid_info,
									b_pid_count, b_pid_info,
									c_pid_count, c_pid_info);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Modulator_Bert_Measure_Ex(int nSlot, long modulator_type, long packet_type, long bert_pid)
{
    return TVB380_SET_MODULATOR_BERT_MEASURE_EX(nSlot, modulator_type, packet_type, bert_pid);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Modulator_Prbs_Info_Ex(int nSlot, long modulator_type, long mode, double noise_power)
{
    return TVB380_SET_MODULATOR_PRBS_INFO_EX(nSlot, modulator_type, mode, noise_power);
}

//---------------------------------------------------------------------------
void CWRAP_DLL::Set_Program(long nBoardNum, int nIndex)
{
#ifdef WIN32
    TSPH_SET_PROGRAM(nBoardNum, nIndex);
#endif
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Remux_Info(int nSlot,
									long btype, long mode, long guard_interval, long partial_reception, long bitrate,
									long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
									long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
									long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
    return TSPH_SET_REMUX_INFO(nSlot,
									btype, mode, guard_interval, partial_reception, bitrate,
									a_segments, a_modulation, a_code_rate, a_time_interleave, a_bitrate,
									b_segments, b_modulation, b_code_rate, b_time_interleave, b_bitrate,
									c_segments, c_modulation, c_code_rate, c_time_interleave, c_bitrate);

}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Sdram_Bank_info(long nBoardNum, int nBankCount, int nBankOffset)
{
    return TSPH_SET_SDRAM_BANK_INFO(nBoardNum, nBankCount, nBankOffset);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Stop_Mode_Ex(int nSlot, long stop_mode)
{
    return TVB380_SET_STOP_MODE_EX(nSlot, stop_mode);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Show_Video_Window(int nSlot, int nShowWindow)
{
#ifdef WIN32
    return TSPH_SHOW_VIDEO_WINDOW(nSlot, nShowWindow);
#else
	return 1;
#endif
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Set_Tmcc_Remuxer(int nSlot, long use_tmcc_remuxer)
{
    return TSPH_SET_TMCC_REMUXER(nSlot, use_tmcc_remuxer);
}

//---------------------------------------------------------------------------
int CWRAP_DLL::Start_Monitor(int nSlot, int nPort)
{
    return TSPH_START_MONITOR(nSlot, nPort);
}

//---------------------------------------------------------------------------
long   CWRAP_DLL::Tsph_Exit_Process(int nSlot)
{
#ifdef WIN32
    return TSPH_EXIT_PROCESS(nSlot);
#else
	return 1;
#endif
}
//---------------------------------------------------------------------------
#ifdef WIN32

int	   CWRAP_DLL::TSPH_SET_C2MI_PARAMS(	
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
								int C2_createFile)
{
	if(tsph_set_c2mi_params != NULL)
		return tsph_set_c2mi_params(nSlot, 
							 C2_BW,
							 C2_L1,
							 C2_Guard,
							 C2_Network,
							 C2_System,
							 C2_StartFreq,
							 C2_NumNoth,
							 C2_RevTone,
							 C2_NotchStart,
							 C2_NotchWidth,
							 C2_Dslice_type,
							 C2_Dslice_FecHeder,
							 C2_Dslice_BBHeder,
							 C2_Dslice_TonePos,
							 C2_Dslice_OffRight,
							 C2_Dslice_OffLeft,
							 C2_Plp_Mod,
							 C2_Plp_Code,
							 C2_Plp_Fec,
							 C2_Plp_Count,
							 C2_Plp_ID,
							 C2_Plp_Blk,
							 C2_Plp_TS_Bitrate,
							 C2_Plp_TS,
							 C2_createFile);
	return -1;
}
int		CWRAP_DLL::TSPL_GET_AD9775_EX(int nSlot, long reg)
{
	if(tspl_get_ad9775_ex != NULL)
		return tspl_get_ad9775_ex(nSlot, reg);
	return -1;
}
int		CWRAP_DLL::TSPL_SET_AD9775_EX(int nSlot, long reg, long data)
{
	if(tspl_set_ad9775_ex != NULL)
		return tspl_set_ad9775_ex(nSlot, reg, data);
	return -1;
}
int		CWRAP_DLL::TSPH_TRY_ALLOC_IQ_MEMORY(int nSlot, int mem_size)
{
	if(tsph_try_alloc_iq_memory != NULL)
		return tsph_try_alloc_iq_memory(nSlot, mem_size);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_IQ_MODE(int nSlot, int mode, int memory_use, int memory_size, int capture_size)
{
	if(tsph_set_iq_mode != NULL)
		return tsph_set_iq_mode(nSlot, mode, memory_use, memory_size, capture_size);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_COMBINER_INFO(int nSlot, int ts_count, char *ts_path, long modulation, long code_rate, long slot_count)
{
	if(tsph_set_combiner_info != NULL)
		return tsph_set_combiner_info(nSlot, ts_count, ts_path, modulation, code_rate, slot_count);
	return -1;
}
int		CWRAP_DLL::TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType)
{
	if(tsph_cal_play_rate != NULL)
		return tsph_cal_play_rate(nSlot, szFile, iType);
	return -1;
}
int 	CWRAP_DLL::TSPH_IS_COMBINED_TS(int nSlot, char *ts_path)
{
	if(tsph_is_combined_ts != NULL)
		return tsph_is_combined_ts(nSlot, ts_path);
	return -1;
}
int		CWRAP_DLL::TSPH_GET_TS_ID(int nSlot, char* szFile)
{
	if(tsph_get_ts_id != NULL)
		return tsph_get_ts_id(nSlot, szFile);
	return -1;
}
unsigned long	CWRAP_DLL::TSPL_READ_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address)
{
	if(tspl_read_control_reg_ex != NULL)
		return tspl_read_control_reg_ex(nSlot, Is_PCI_Control, address);
	return -1;
}
long    CWRAP_DLL::TSPL_WRITE_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long nAddr, unsigned long nData)
{
	if(tspl_write_control_reg_ex != NULL)
		return tspl_write_control_reg_ex(nSlot, Is_PCI_Control, nAddr, nData);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_LOOP_ADAPTATION(int nSlot, long pcr_restamping, long continuity_conunter, long tdt_tot, char* user_date, char* user_time)
{
	if(tsph_set_loop_adaptation != NULL)
		return tsph_set_loop_adaptation(nSlot, pcr_restamping, continuity_conunter, tdt_tot, user_date, user_time);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_T2MI_PARAMS(int nSlot, 
			int BW, int FFT_SIZE, int GUARD_INTERVAL, int L1_MOD, int PILOT_PATTERN, int BW_EXT, double FREQUENCY,
			int NETWORK_ID, int T2_SYSTEM_ID, int CELL_ID, int S1, int PLP_MOD, int PLP_COD, int PLP_FEC_TYPE, int HEM, 
			int NUM_T2_FRAMES, int NUM_DATA_SYMBOLS, int PLP_NUM_BLOCKS, int PID
			, int PLP_ROTATION, int PLP_COUNT, int PLP_ID, int PLP_BITRATE, int PLP_TS_BITRATE, char *PLP_TS, int PLP_Time_Interleave
			)
{
	if(tsph_set_t2mi_params != NULL)
		return tsph_set_t2mi_params(nSlot, 
			BW, FFT_SIZE, GUARD_INTERVAL, L1_MOD, PILOT_PATTERN, BW_EXT, FREQUENCY,
			NETWORK_ID, T2_SYSTEM_ID, CELL_ID, S1, PLP_MOD, PLP_COD, PLP_FEC_TYPE, HEM, 
			NUM_T2_FRAMES, NUM_DATA_SYMBOLS, PLP_NUM_BLOCKS, PID
			, PLP_ROTATION, PLP_COUNT, PLP_ID, PLP_BITRATE, PLP_TS_BITRATE, PLP_TS, PLP_Time_Interleave
			);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_BANDWIDTH_EX(int nSlot, long modulator_type, long bandwidth, double output_frequency)
{
	if(tvb380_set_modulator_bandwidth_ex != NULL)
		return tvb380_set_modulator_bandwidth_ex(nSlot, modulator_type, bandwidth, output_frequency);
	return -1;
}
int		CWRAP_DLL::TSPH_GET_T2MI_PARAMS(int nSlot, int *num_data_symbol, int *plp_num_block)
{
	if(tsph_get_t2mi_params != NULL)
		return tsph_get_t2mi_params(nSlot, num_data_symbol, plp_num_block);
	return -1;
}
#ifdef WIN32
int		CWRAP_DLL::TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, T2MI_Parsing_Info *szResult)
#else
int		CWRAP_DLL::TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, char *szResult)
#endif
{
	if(tsph_run_t2mi_parser != NULL)
		return tsph_run_t2mi_parser(nSlot, szFile, szResult);
	return -1;
}
int		CWRAP_DLL::TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(int nSlot, char* ts_path)
{
	if(tsph_isdbs_calc_combined_ts_bitrate != NULL)
		return tsph_isdbs_calc_combined_ts_bitrate(nSlot, ts_path);
	return -1;
}
int 	CWRAP_DLL::TSPL_RESET_SDCON_EX(int nSlot)
{
	if(tspl_reset_sdcon_ex != NULL)
		return tspl_reset_sdcon_ex(nSlot);
	return -1;
}
long 	CWRAP_DLL::TSPH_GET_MHE_PACKET_INFO(int nSlot, char *szPlayTrackList, int nType)
{
	if(tsph_get_mhe_packet_info != NULL)
		return tsph_get_mhe_packet_info(nSlot, szPlayTrackList, nType);
	return -1;
}
int		CWRAP_DLL::TSPH_GET_LOOP_COUNT(int nSlot)
{
	if(tsph_get_loop_count != NULL)
		return tsph_get_loop_count(nSlot);
	return -1;
}
#ifdef WIN32
int		CWRAP_DLL::TSPL_SET_AD9852_MAX_CLOCK_EX(int nSlot, long value)
{
	if(tspl_set_ad9852_max_clock_ex != NULL)
		return tspl_set_ad9852_max_clock_ex(nSlot, value);
	return -1;
}
#endif
int		CWRAP_DLL::TVB380_SET_BOARD_CONFIG_STATUS_EX(int nSlot, long modulator_type, long status)
{
	if(tvb380_set_board_config_status_ex != NULL)
		return tvb380_set_board_config_status_ex(nSlot, modulator_type, status);
	return -1;
}
int 	CWRAP_DLL::TSPL_GET_AUTHORIZATION_EX(int nSlot)
{
	if(tspl_get_authorization_ex != NULL)
		return tspl_get_authorization_ex(nSlot);
	return -1;
}
int		CWRAP_DLL::TSPL_GET_FPGA_INFO_EX(int nSlot, int info)
{
	if(tspl_get_fpga_info_ex != NULL)
		return tspl_get_fpga_info_ex(nSlot, info);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_STOP_MODE_EX(int nSlot, long stop_mode)
{
	if(tvb380_set_stop_mode_ex != NULL)
		return tvb380_set_stop_mode_ex(nSlot, stop_mode);
	return -1;
}
int 	CWRAP_DLL::TSPH_SET_SDRAM_BANK_INFO(int nSlot, int nBankCount, int nBankOffset)
{
	if(tsph_set_sdram_bank_info != NULL)
		return tsph_set_sdram_bank_info(nSlot, nBankCount, nBankOffset);
	return -1;
}
int		CWRAP_DLL::TSPH_START_MONITOR(int nSlot, int nPort)
{
	if(tsph_start_monitor != NULL)
		return tsph_start_monitor(nSlot, nPort);
	return -1;
}

//2011/10/24 added PAUSE
int		CWRAP_DLL::TSPH_START_DELAY(int nSlot, int nPort)
{
	if(tsph_start_delay != NULL)
		return tsph_start_delay(nSlot, nPort);
	return -1;
}

int     CWRAP_DLL::TSPL_SET_TSIO_DIRECTION_EX(int nSlot, int nDireciton)
{
	if(tspl_set_tsio_direction_ex != NULL)
		return tspl_set_tsio_direction_ex(nSlot, nDireciton);
	return -1;
}
int 	CWRAP_DLL::TSPH_SET_MODULATOR_TYPE(int nSlot, long type)
{
	if(tsph_set_modulator_type != NULL)
		return tsph_set_modulator_type(nSlot, type);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_ERROR_INJECTION(int nSlot, long error_lost, long error_lost_packet,
										 long error_bits, long error_bits_packet, long error_bits_count,
										 long error_bytes, long error_bytes_packet, long error_bytes_count)
{
	if(tsph_set_error_injection != NULL)
		return tsph_set_error_injection(nSlot, error_lost, error_lost_packet, error_bits, error_bits_packet, error_bits_count, error_bytes, error_bytes_packet, error_bytes_count);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_CONFIG_EX(int nSlot, long modulator_type, long IF_Frequency)
{
	if(tvb380_set_config_ex != NULL)
		return tvb380_set_config_ex(nSlot, modulator_type, IF_Frequency);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_IF_FREQ_EX(int nSlot, long modulator_type, long IF_frequency)
{
	if(tvb380_set_modulator_if_freq_ex != NULL)
		return tvb380_set_modulator_if_freq_ex(nSlot, modulator_type, IF_frequency);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_FREQ_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth)
{
	if(tvb380_set_modulator_freq_ex != NULL)
		return tvb380_set_modulator_freq_ex(nSlot, modulator_type, output_frequency, symbol_rate_or_bandwidth);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_CODERATE_EX(int nSlot, long modulator_type, long code_rate)
{
	if(tvb380_set_modulator_coderate_ex != NULL)
		return tvb380_set_modulator_coderate_ex(nSlot, modulator_type, code_rate);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_DVBH_EX(int nSlot, long modulator_type, long  tx_mode, long  in_depth_interleave, long  time_slice, long  mpe_fec, long  cell_id)
{
	if(tvb380_set_modulator_dvbh_ex != NULL)
		return tvb380_set_modulator_dvbh_ex(nSlot, modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_TXMODE_EX(int nSlot, long modulator_type, long tx_mode)
{
	if(tvb380_set_modulator_txmode_ex != NULL)
		return tvb380_set_modulator_txmode_ex(nSlot, modulator_type, tx_mode);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_GUARDINTERVAL_EX(int nSlot, long modulator_type, long guard_interval)
{
	if(tvb380_set_modulator_guardinterval_ex != NULL)
		return tvb380_set_modulator_guardinterval_ex(nSlot, modulator_type, guard_interval);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_CONSTELLATION_EX(int nSlot, long modulator_type,long constellation)
{
	if(tvb380_set_modulator_constellation_ex != NULL)
		return tvb380_set_modulator_constellation_ex(nSlot, modulator_type, constellation);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_SYMRATE_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth)
{
	if(tvb380_set_modulator_symrate_ex != NULL)
		return tvb380_set_modulator_symrate_ex(nSlot, modulator_type, output_frequency, symbol_rate_or_bandwidth);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_INTERLEAVE_EX(int nSlot, long modulator_type, long interleaving)
{
	if(tvb380_set_modulator_interleave_ex != NULL)
		return tvb380_set_modulator_interleave_ex(nSlot, modulator_type, interleaving);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_PRBS_MODE_EX(int nSlot, long modulator_type, long mode)
{
	if(tvb380_set_modulator_prbs_mode_ex != NULL)
		return tvb380_set_modulator_prbs_mode_ex(nSlot, modulator_type, mode);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_PRBS_SCALE_EX(int nSlot, long modulator_type, double noise_power)
{
	if(tvb380_set_modulator_prbs_scale_ex != NULL)
		return tvb380_set_modulator_prbs_scale_ex(nSlot, modulator_type, noise_power);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_PRBS_INFO_EX(int nSlot, long modulator_type, long mode, double noise_power)
{
	if(tvb380_set_modulator_prbs_info_ex != NULL)
		return tvb380_set_modulator_prbs_info_ex(nSlot, modulator_type, mode, noise_power);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_TMCC_REMUXER(int nSlot, long use_tmcc_remuxer)
{
	if(tsph_set_tmcc_remuxer != NULL)
		return tsph_set_tmcc_remuxer(nSlot, use_tmcc_remuxer);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(int nSLot, long modulator_type, long roll_off_factor)
{
	if(tvb380_set_modulator_roll_off_factor_ex != NULL)
		return tvb380_set_modulator_roll_off_factor_ex(nSLot, modulator_type, roll_off_factor);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_PILOT_EX(int nSlot, long modulator_type, long  pilot_on_off)
{
	if(tvb380_set_modulator_pilot_ex != NULL)
		return tvb380_set_modulator_pilot_ex(nSlot, modulator_type, pilot_on_off);
	return -1;
}
int		CWRAP_DLL::TSPL_SET_MAX_PLAYRATE_EX(int nSlot, long modulator_type, long use_max_playrate)
{
	if(tspl_set_max_playrate_ex != NULL)
		return tspl_set_max_playrate_ex(nSlot, modulator_type, use_max_playrate);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_BERT_MEASURE_EX(int nSlot, long modulator_type, long packet_type, long bert_pid)
{
	if(tvb380_set_modulator_bert_measure_ex != NULL)
		return tvb380_set_modulator_bert_measure_ex(nSlot, modulator_type, packet_type, bert_pid);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(int nSlot, long modulator_type, long sdram_clock)
{
	if(tvb380_set_modulator_sdram_clock_ex != NULL)
		return tvb380_set_modulator_sdram_clock_ex(nSlot, modulator_type, sdram_clock);
	return -1;
}
int 	CWRAP_DLL::TSPL_GET_ENCRYPTED_SN_EX(int nSlot, int type, char* sn)
{
	if(tspl_get_encrypted_sn_ex != NULL)
		return tspl_get_encrypted_sn_ex(nSlot, type, sn);
	return -1;
}
int		CWRAP_DLL::TVB380_IS_ENABLED_TYPE_EX(int nSlot, long modulator_type)
{
	if(tvb380_is_enabled_type_ex != NULL)
		return tvb380_is_enabled_type_ex(nSlot, modulator_type);
	return -1;
}
int		CWRAP_DLL::TSPL_GET_LAST_ERROR_EX(int nSlot)
{
	if(tspl_get_last_error_ex != NULL)
		return tspl_get_last_error_ex(nSlot);
	return -1;
}
int		CWRAP_DLL::TSPL_SET_PLAY_RATE_EX(int nSlot, long play_freq_in_herz, long nUseMaxPlayrate)
{
	if(tspl_set_play_rate_ex != NULL)
		return tspl_set_play_rate_ex(nSlot, play_freq_in_herz, nUseMaxPlayrate);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_DTMB_EX(int nSlot, long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion)
{
	if(tvb380_set_modulator_dtmb_ex != NULL)
		return tvb380_set_modulator_dtmb_ex(nSlot, modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_FILENAME_NEXT_PLAY(int nSlot, char *szNextPlayFile)
{
	if(tsph_set_filename_next_play != NULL)
		return tsph_set_filename_next_play(nSlot, szNextPlayFile);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_ISDBS_BASE_TS(int nSlot, char* ts_path)
{
	if(tsph_set_isdbs_base_ts != NULL)
		return tsph_set_isdbs_base_ts(nSlot, ts_path);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_RX_IP_STREAMING_INFO(int nSlot, char* src_ip, char* rx_multicast_ip, long rx_udp_port, char* local_ip, long fec_udp_off, long fec_inact)
{
	if(tsph_set_rx_ip_streaming_info != NULL)
		return tsph_set_rx_ip_streaming_info(nSlot, src_ip, rx_multicast_ip, rx_udp_port, local_ip, fec_udp_off, fec_inact);
	return -1;
}
int 	CWRAP_DLL::TSPH_START_IP_STREAMING(int nSlot, 
									char *szFilePath,		//File path to be played or recorded
									long nPlayrate,			//In bps
									double dwStartOffset,	//Start offest in bytes
									long nOption,			//Restamping
									long nRepeatMode,		//Play mode
									long nExtendedMode,		//Operation mode thru. IP
									char *szExtendedInfo)	//Source or Target thru. IP
{
	if(tsph_start_ip_streaming != NULL)
		return tsph_start_ip_streaming(nSlot, szFilePath, nPlayrate, dwStartOffset, nOption, nRepeatMode, nExtendedMode, szExtendedInfo);
	return -1;
}
int 	CWRAP_DLL::TSPH_START_PLAY(int nSlot, char *szNewPlayFile, long lPlayRate, double dwStartOffset, long nUseMaxPlayrate, long lRepeatMode)
{
	if(tsph_start_play != NULL)
		return tsph_start_play(nSlot, szNewPlayFile, lPlayRate, dwStartOffset, nUseMaxPlayrate, lRepeatMode);
	return -1;
}
int		CWRAP_DLL::TSPH_START_RECORD(int nSlot, char* szNewRecordFile, int nPort)
{
	if(tsph_start_record != NULL)
		return tsph_start_record(nSlot, szNewRecordFile, nPort);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_REPEAT_PLAY_MODE(int nSlot, int nRepeatMode)
{
	if(tsph_set_repeat_play_mode != NULL)
		return tsph_set_repeat_play_mode(nSlot, nRepeatMode);
	return -1;
}
int		CWRAP_DLL::TSPH_GET_REMUX_INFO(int nSlot, char *szFile, int layer_index)
{
	if(tsph_get_remux_info != NULL)
		return tsph_get_remux_info(nSlot, szFile, layer_index);
	return -1;
}
int 	CWRAP_DLL::TSPL_CHECK_LN_EX(int nSlot, char* ln)
{
	if(tspl_check_ln_ex != NULL)
		return tspl_check_ln_ex(nSlot, ln);
	return -1;
}
double  CWRAP_DLL::TSPH_GET_CURRENT_RECORD_POINT(int nSlot)
{
	if(tsph_get_current_record_point != NULL)
		return tsph_get_current_record_point(nSlot);
	return -1;
}
int 	CWRAP_DLL::TSPH_GET_CURRENT_THREAD_STATE(int nSlot)
{
	if(tsph_get_current_thread_state != NULL)
		return  tsph_get_current_thread_state(nSlot);
	return -1;
}
double 	CWRAP_DLL::TVB380_GET_MODULATOR_BERT_RESULT_EX(int nSlot, long modulator_type)
{
	if(tvb380_get_modulator_bert_result_ex != NULL)
		return tvb380_get_modulator_bert_result_ex(nSlot, modulator_type);
	return -1;
}
int 	CWRAP_DLL::TSPH_GET_PLAY_BUFFER_STATUS(int nSlot)
{
	if(tsph_get_play_buffer_status != NULL)
		return tsph_get_play_buffer_status(nSlot);
	return -1;
}

//2011/8/31 added
int 	CWRAP_DLL::TSPH_IP_RECV_STATUS(int nSlot, unsigned int *buf_bytes_avable, unsigned int *cnt_lost_pkt)
{
	if(tsph_ip_recv_status != NULL)
		return tsph_ip_recv_status(nSlot, buf_bytes_avable, cnt_lost_pkt);
	return -1;
}

long 	CWRAP_DLL::TSPH_GET_PROGRAM(int nSlot, long nIndex)
{
	if(tsph_get_program != NULL)
		return tsph_get_program(nSlot, nIndex);
	return -1;
}
int		CWRAP_DLL::TSPH_GET_REMUX_DATARATE(int nSlot, long layer_index)
{
	if(tsph_get_remux_datarate != NULL)
		return tsph_get_remux_datarate(nSlot, layer_index);
	return -1;
}
int 	CWRAP_DLL::TSPH_IS_VIDEO_WINDOW_VISIBLE(int nSlot)
{
	if(tsph_is_video_window_visible != NULL)
		return tsph_is_video_window_visible(nSlot);
	return -1;
}
int 	CWRAP_DLL::TSPH_MOVE_VIDEO_WINDOW(int nSlot, int nX, int nY, int nW, int nH)
{
	if(tsph_move_video_window != NULL)
		return tsph_move_video_window(nSlot, nX, nY, nW, nH);
	return -1;
}
unsigned long 		CWRAP_DLL::TSPL_READ_INPUT_STATUS_EX(int nSlot)
{
	if(tspl_read_input_status_ex != NULL)
		return tspl_read_input_status_ex(nSlot);
	return -1;
}
double  CWRAP_DLL::TSPL_READ_INPUT_TSCOUNT_EX(int nSlot)
{
	if(tspl_read_input_tscount_ex != NULL)
		return tspl_read_input_tscount_ex(nSlot);
	return -1;
}
int 	CWRAP_DLL::TSPH_READ_IP_STREAMING_INPUT_STATUS(int nSlot, long nStatus)					//nStatus==0(Input Bitrate), 2(Demux Bitrate)
{
	if(tsph_read_ip_streaming_input_status != NULL)
		return tsph_read_ip_streaming_input_status(nSlot, nStatus);
	return -1;
}
int CWRAP_DLL::TSPH_RUN_TS_PARSER2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo)
{
	if(tsph_run_ts_parser2 != NULL)
		return tsph_run_ts_parser2(nSlot, szFile, default_bitrate, ppTsInfo);
	return -1;
}

int		CWRAP_DLL::TSPH_SET_LAYER_INFO(int nSlot,
									int other_pid_map_to_layer,
									int multi_pid_map,
									int total_pid_count, char* total_pid_info,
									int a_pid_count, char* a_pid_info,
									int b_pid_count, char* b_pid_info,
									int c_pid_count, char* c_pid_info)
{
	if(tsph_set_layer_info != NULL)
		return tsph_set_layer_info(nSlot, other_pid_map_to_layer, multi_pid_map, total_pid_count, total_pid_info, a_pid_count,
									a_pid_info, b_pid_count, b_pid_info, c_pid_count, c_pid_info);
	return -1;
}
long 	CWRAP_DLL::TSPH_SET_PROGRAM(int nSlot, long nIndex)
{
	if(tsph_set_program != NULL)
		return tsph_set_program(nSlot, nIndex);
	return -1;
}
int		CWRAP_DLL::TSPH_SET_REMUX_INFO(int nSlot,
									long btype, long mode, long guard_interval, long partial_reception, long bitrate,
									long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
									long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
									long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
	if(tsph_set_remux_info != NULL)
		return tsph_set_remux_info(nSlot, btype, mode, guard_interval, partial_reception, bitrate, a_segments, a_modulation, a_code_rate, a_time_interleave, a_bitrate, 
								b_segments, b_modulation, b_code_rate, b_time_interleave, b_bitrate, c_segments, c_modulation, c_code_rate, c_time_interleave, c_bitrate);
	return -1;
}
int		CWRAP_DLL::TSPH_SHOW_VIDEO_WINDOW(int nSlot, int nShowWindow)
{
	if(tsph_show_video_window != NULL)
		return tsph_show_video_window(nSlot, nShowWindow);
	return -1;
}
long    CWRAP_DLL::TSPH_EXIT_PROCESS(int nSlot)
{
	if(tsph_exit_process != NULL)
		return tsph_exit_process(nSlot);
	return -1;
}
int 	CWRAP_DLL::TSPH_GET_BOARD_LOCATION(void)
{
	if(tsph_get_board_location != NULL)
		return tsph_get_board_location();
	return -1;
}
int 	CWRAP_DLL::TSPL_GET_BOARD_ID_EX(int nSlot)
{
	if(tspl_get_board_id_ex != NULL)
		return tspl_get_board_id_ex(nSlot);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_AGC_EX(int nSlot, long modulator_type, long agc_on_off, long UseTAT4710)
{
	if(tvb380_set_modulator_agc_ex != NULL)
		return tvb380_set_modulator_agc_ex(nSlot, modulator_type, agc_on_off, UseTAT4710);
	return -1;
}
int 	CWRAP_DLL::TVB380_SET_MODULATOR_ATTEN_VALUE_EX(int nSlot, long modulator_type, double atten_value, long UseTAT4710)
{
	if(tvb380_set_modulator_atten_value_ex != NULL)
		return tvb380_set_modulator_atten_value_ex(nSlot, modulator_type, atten_value, UseTAT4710);
	return -1;
}

//2012/9/6 pcr restamp
int		CWRAP_DLL::TVB59x_SET_PCR_STAMP_CNTL_EX(int nSlot, int _val)
{
	if(tvb59x_set_pcr_stamp_cntl_ex != NULL)
		return tvb59x_set_pcr_stamp_cntl_ex(nSlot, _val);
	return -1;
}
#ifdef WIN32
//2013/5/31 TVB599 TS output type(ASI, 310M)
int		CWRAP_DLL::TVB59x_SET_Output_TS_Type_EX(int nSlot, int _val)
{
	if(tvb59x_set_output_ts_type_ex != NULL)
		return tvb59x_set_output_ts_type_ex(nSlot, _val);
	return -1;
}

//2013/6/10 TVB599 Reset Control Register
int		CWRAP_DLL::TVB59x_SET_Reset_Control_REG_EX(int nSlot, int _val)
{
	if(tvb59x_set_reset_control_reg_ex != NULL)
		return tvb59x_set_reset_control_reg_ex(nSlot, _val);
	return -1;
}
//2013/7/2 TVB599
int		CWRAP_DLL::TSPL_SET_SDCON_MODE_EX(int nSlot, int _val)
{
	if(tspl_set_sdcon_mode_ex != NULL)
		return tspl_set_sdcon_mode_ex(nSlot, _val);
	return -1;
}int		CWRAP_DLL::Func_TVB59x_SET_TsPacket_CNT_Mode_EX(int nSlot, int _val)
{
	if(call_tvb59x_set_tspacket_cnt_mode_ex != NULL)
		return call_tvb59x_set_tspacket_cnt_mode_ex(nSlot, _val);
	return -1;
}
int		CWRAP_DLL::Func_TVB59x_Get_Asi_Input_rate_EX(int nSlot, int *delta_packet, int *delta_clock)
{
	if(call_tvb59x_get_asi_input_rate_ex != NULL)
		return call_tvb59x_get_asi_input_rate_ex(nSlot, delta_packet, delta_clock);
	return -1;
}

int		CWRAP_DLL::Func_TVB59x_Modulator_Status_Control_EX(int nSlot, int modulator, int index, int val)
{
	if(call_tvb59x_modulator_status_control_ex != NULL)
		return call_tvb59x_modulator_status_control_ex(nSlot, modulator, index, val);
	return -1;
}

int		CWRAP_DLL::Func_TVB59x_Get_Modulator_Status_EX(int nSlot, int _val)
{
	if(call_tvb59x_get_modulator_status_ex != NULL)
		return call_tvb59x_get_modulator_status_ex(nSlot, _val);
	return -1;
}

int		CWRAP_DLL::Func_TSPL_GET_TSIO_STATUS_EX(int nSlot, int option)
{
	if(call_tspl_get_tsio_status_ex != NULL)
		return call_tspl_get_tsio_status_ex(nSlot, option);
	return -1;
}

//int		CWRAP_DLL::Func_TVB59x_Set_Symbol_Clock_EX(int nSlot, int modulator_mode, int symbol_clock)
//{
//	if(call_tspl_set_symbol_clock_ex != NULL)
//		return call_tspl_set_symbol_clock_ex(nSlot, modulator_mode, symbol_clock);
//	return -1;
//}
#endif
//2012/8/31 new rf level control
int		CWRAP_DLL::TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX(
			int nSlot, 
			long modulator_type, 
			double rf_level_value, 
			long *AmpFlag, 
			long UseTAT4710)
{
	if(tvb59x_set_modulator_rf_level_value_ex != NULL)
		return tvb59x_set_modulator_rf_level_value_ex(nSlot, modulator_type, rf_level_value, AmpFlag, UseTAT4710);
	return -1;
}

//2012/8/31 new rf level control
int		CWRAP_DLL::TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX(
			int nSlot, 
			long modulator_type, 
			double *rf_level_min, 
			double *rf_level_max, 
			long UseTAT4710)
{
	if(tvb59x_get_modulator_rf_level_range_ex != NULL)
		return tvb59x_get_modulator_rf_level_range_ex(nSlot, modulator_type, rf_level_min, rf_level_max, UseTAT4710);
	return -1;
}
double 	CWRAP_DLL::TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(int nSlot, long modulator_type, long info_type)
{
	if(tvb380_get_modulator_rf_power_level_ex != NULL)
		return tvb380_get_modulator_rf_power_level_ex(nSlot, modulator_type, info_type);
	return -1;
}
int		CWRAP_DLL::TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(int nSlot, long modulator_type, long spectral_inversion)
{
	if(tvb380_set_modulator_spectrum_inversion_ex != NULL)
		return tvb380_set_modulator_spectrum_inversion_ex(nSlot, modulator_type, spectral_inversion);
	return -1;
}
int 	CWRAP_DLL::TSPH_SET_CURRENT_OFFSET(int nSlot, 
										int nOffsetType,	//0=start offset, 1==current offset, 2==end offset
										double dwOffset)
{
	if(tsph_set_current_offset != NULL)
		return tsph_set_current_offset(nSlot, nOffsetType, dwOffset);
	return -1;
}
int		CWRAP_DLL::TSPL_SET_BOARD_LED_STATUS_EX(int nSlot, int status_LED, int fault_LED)
{
	if(tspl_set_board_led_status_ex != NULL)
		return tspl_set_board_led_status_ex(nSlot, status_LED, fault_LED);
	return -1;
}
int		CWRAP_DLL::TSPH_RUN_ATSC_MH_PARSER(int nSlot, char *szFile, char *szResult)
{
	if(tsph_run_atsc_mh_parser != NULL)
		return tsph_run_atsc_mh_parser(nSlot, szFile, szResult);
	return -1;
}
int		CWRAP_DLL::TSPH_RUN_MFS_PARSER(int nSlot, char *szFile, char *szResult)
{
	if(tsph_run_mfs_parser != NULL)
		return tsph_run_mfs_parser(nSlot, szFile, szResult);
	return -1;
}
int 	CWRAP_DLL::TSPL_GET_BOARD_REV_EX(int nSlot)
{
	if(tspl_get_board_rev_ex != NULL)
		return tspl_get_board_rev_ex(nSlot);
	return -1;
}
int		CWRAP_DLL::TSPL_GET_BOARD_CONFIG_STATUS_EX(int nSlot)
{
	if(tspl_get_board_config_status_ex != NULL)
		return tspl_get_board_config_status_ex(nSlot);
	return -1;
}
int     CWRAP_DLL::TSPH_CLEAR_REMUX_INFO(int nSlot)
{
	if(tsph_clear_remux_info != NULL)
		return tsph_clear_remux_info(nSlot);
	return -1;
}
int		CWRAP_DLL::TSPH_RUN_C2MI_PARSER(int nSlot, char *szFile, char *szResult)
{
	if(tsph_run_c2mi_parser != NULL)
		return tsph_run_c2mi_parser(nSlot, szFile, szResult);
	return -1;
}

//2011/11/10 ISDB-T 13seg 188 TS Loopthru
int     CWRAP_DLL::TSPH_IS_LOOPTHRU_ISDBT13_188(int nSlot)
{
	if(tsph_is_loopthru_isdbt13_188 != NULL)
		return tsph_is_loopthru_isdbt13_188(nSlot);
	return -1;
}
int     CWRAP_DLL::TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser(int nSlot, struct _TSPH_TS_INFO **ppTsInfo)
{
	if(tsph_pause_loopthru_isdbt13_parser != NULL)
		return tsph_pause_loopthru_isdbt13_parser(nSlot, ppTsInfo);
	return -1;
}
int     CWRAP_DLL::TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(int nSlot)
{
	if(tsph_buf_clear_loopthru_isdbt13_188 != NULL)
		return tsph_buf_clear_loopthru_isdbt13_188(nSlot);
	return -1;
}
int     CWRAP_DLL::TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(int nSlot)
{
	if(tsph_is_loopthru_inbuf_full_isdbt13_188 != NULL)
		return tsph_is_loopthru_inbuf_full_isdbt13_188(nSlot);
	return -1;
}

int     CWRAP_DLL::TSPH__STREAM_NUMBER(int nSlot)
{
	if(tsph__stream_number != NULL)
		return tsph__stream_number(nSlot);
	return -1;
}
int     CWRAP_DLL::TSPH__SEL_TS_of_ASI310OUT(int nSlot, int _ts_n)
{
	if(tsph__sel_ts_of_asi310out != NULL)
		return tsph__sel_ts_of_asi310out(nSlot, _ts_n);
	return -1;
}

//2012/2/15 NIT
int		CWRAP_DLL::TSPH_GET_NIT_SATELLITE_INFO(int nSlot, 
										int *descriptor_flag,
										int *freq,
										int *rolloff,
										int *modulation_system,
										int *modulation,
										int *symbolrate,
										int *coderate)
{
	if(tsph_get_nit_satellite_info != NULL)
		return tsph_get_nit_satellite_info(nSlot, descriptor_flag, freq, rolloff, modulation_system, modulation, symbolrate, coderate);
	return -1;
}

int		CWRAP_DLL::TSPH_GET_NIT_CABLE_INFO(int nSlot, 
									int *descriptor_flag,
									int *freq,
									int *modulation,
									int *symbolrate,
									int *coderate)
{
	if(tsph_get_nit_cable_info != NULL)
		return tsph_get_nit_cable_info(nSlot, descriptor_flag, freq, modulation, symbolrate, coderate);
	return -1;
}

int 	CWRAP_DLL::TSPH_GET_NIT_TERRESTRIAL_INFO(int nSlot, int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod)
{
	if(tsph_get_nit_terrestrial_info != NULL)
		return tsph_get_nit_terrestrial_info(nSlot, descriptor_flag, freq, bw, time_slicing, mpe_fec, constellation, coderate, guard, txmod);
	return -1;
}


//2012/4/12 SINGLE TONE
int		CWRAP_DLL::TVB380_SET_MODULATOR_SINGLE_TONE_EX(int nSlot, long modulator_type, unsigned long freq, long singleTone)
{
	if(tvb380_set_modulator_single_tone_ex != NULL)
		return tvb380_set_modulator_single_tone_ex(nSlot, modulator_type, freq, singleTone);
	return -1;
}
//2012/3/21 
int CWRAP_DLL::TSPH_GetRealBdCnt_N(void)
{
	if(tsph_getrealbdcnt_n != NULL)
		return tsph_getrealbdcnt_n();
	return -1;
}
int CWRAP_DLL::TSPH_GetRealAndVirBdMap(int _r_bd_id, int *_map)
{
	if(tsph_getrealandvirbdmap != NULL)
		return tsph_getrealandvirbdmap(_r_bd_id, _map);
	return -1;
}
int CWRAP_DLL::TSPH_GetBdId_N(int _Nth)
{
	if(tsph_getbdid_n != NULL)
		return tsph_getbdid_n(_Nth);
	return -1;
}

char *CWRAP_DLL::TSPH_GetBdName_N(int _Nth)
{
	if(tsph_getbdname_n != NULL)
		return tsph_getbdname_n(_Nth);
	return NULL;
}
int CWRAP_DLL::TSPH_ConfTvbSytem(void)
{
	if(tsph_conftvbsystem != NULL)
		return tsph_conftvbsystem();
	return -1;
}
int CWRAP_DLL::TSPH_InitAllRealBd(void)
{
	if(tsph_initallrealbd != NULL)
		return tsph_initallrealbd();
	return -1;
}
int CWRAP_DLL::TSPH_InitVirBd(int _r_bd_id, int _v_cnt)
{
	if(tsph_initvirbd != NULL)
		return tsph_initvirbd(_r_bd_id, _v_cnt);
	return -1;
}
int CWRAP_DLL::TSPH_ActivateOneBd(int _bd_id, int _init_modulator, int _init_if_freq)
{
	if(tsph_activateonebd != NULL)
		return tsph_activateonebd(_bd_id, _init_modulator, _init_if_freq);
	return -1;

}

//2012/3/28 Multiple VSB,QAM-B
int CWRAP_DLL::TSPL_CNT_MULTI_VSB_RFOUT_EX(int nSlot)
{
	if(tspl_cnt_multi_vsb_rfout_ex != NULL)
		return tspl_cnt_multi_vsb_rfout_ex(nSlot);
	return -1;
}
int CWRAP_DLL::TSPL_CNT_MULTI_QAM_RFOUT_EX(int nSlot)
{
	if(tspl_cnt_multi_qam_rfout_ex != NULL)
		return tspl_cnt_multi_qam_rfout_ex(nSlot);
	return -1;
}
//2012/6/28 multi dvb-t
int CWRAP_DLL::TSPL_CNT_MULTI_DVBT_RFOUT_EX(int nSlot)
{
	if(tspl_cnt_multi_dvbt_rfout_ex != NULL)
		return tspl_cnt_multi_dvbt_rfout_ex(nSlot);
	return -1;
}

int		CWRAP_DLL::TSPL_GET_AD9787_EX(int nSlot, long reg)
{
	if(tspl_get_ad9787_ex != NULL)
		return tspl_get_ad9787_ex(nSlot, reg);
	return -1;
}
int		CWRAP_DLL::TSPL_SET_AD9787_EX(int nSlot, long reg, long data)
{
	if(tspl_set_ad9787_ex != NULL)
		return tspl_set_ad9787_ex(nSlot, reg, data);
	return -1;
}

#ifdef WIN32
int  CWRAP_DLL::TSPH_SET_T2MI_STREAM_GENERATION(int nSlot, int Output_T2mi, char* ts_path)
{
	if(tsph_set_t2mi_stream_generation != NULL)
		return tsph_set_t2mi_stream_generation(nSlot, Output_T2mi, ts_path);
	return -1;
}
#endif

//2011/5/27 
int CWRAP_DLL::LoadHLDLibrary()
{
	ReleaseHLDLibrary();
	m_hInstance = LoadLibrary("TSPHLD0381.dll");
	if(m_hInstance == NULL)
	{
		return -1;
	}

	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT,					tsph_set_c2mi_params, "TSPH_SET_C2MI_PARAMS");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tspl_get_ad9775_ex, "TSPL_GET_AD9775_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tspl_set_ad9775_ex, "TSPL_SET_AD9775_EX");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tspl_get_ad9787_ex, "TSPL_GET_AD9787_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tspl_set_ad9787_ex, "TSPL_SET_AD9787_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					tsph_try_alloc_iq_memory, "TSPH_TRY_ALLOC_IQ_MEMORY");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT																							,					tsph_set_iq_mode, "TSPH_SET_IQ_MODE");
	GET_PROC(FTYPE_INT_INT_INT_PCHAR_LONG_LONG_LONG																					,					tsph_set_combiner_info, "TSPH_SET_COMBINER_INFO");
	GET_PROC(FTYPE_INT_PCHAR_INT																									,					tsph_cal_play_rate, "TSPH_CAL_PLAY_RATE");
	GET_PROC(FTYPE_INT_INT_PCHAR																									,					tsph_is_combined_ts, "TSPH_IS_COMBINED_TS");
	GET_PROC(FTYPE_INT_INT_PCHAR																									,					tsph_get_ts_id, "TSPH_GET_TS_ID");
	GET_PROC(FTYPE_DWORD_INT_INT_DWORD																								,					tspl_read_control_reg_ex, "TSPL_READ_CONTROL_REG_EX");
	GET_PROC(FTYPE_LONG_INT_INT_DWORD_DWORD																							,					tspl_write_control_reg_ex, "TSPL_WRITE_CONTROL_REG_EX");
	GET_PROC(FTYPE_INT_LONG_LONG_LONG_PCHAR_PCHAR																					,					tsph_set_loop_adaptation, "TSPH_SET_LOOP_ADAPTATION");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT_INT_INT_DOUBLE_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_INT_PCHAR_INT ,					tsph_set_t2mi_params, "TSPH_SET_T2MI_PARAMS");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_DOUBLE																							,					tvb380_set_modulator_bandwidth_ex, "TVB380_SET_MODULATOR_BANDWIDTH_EX");
	GET_PROC(FTYPE_INT_INT_PINT_PINT																								,					tsph_get_t2mi_params, "TSPH_GET_T2MI_PARAMS");
#ifdef WIN32
	GET_PROC(FTYPE_INT_INT_PCHAR_PSTRUCT																								,					tsph_run_t2mi_parser, "TSPH_RUN_T2MI_PARSER");
#else
	GET_PROC(FTYPE_INT_INT_PCHAR_PCHAR																								,					tsph_run_t2mi_parser, "TSPH_RUN_T2MI_PARSER");
#endif
	GET_PROC(FTYPE_INT_INT_PCHAR																									,					tsph_isdbs_calc_combined_ts_bitrate, "TSPH_ISDBS_CALC_COMBINED_TS_BITRATE");
	GET_PROC(FTYPE_INT_INT																											,					tspl_reset_sdcon_ex, "TSPL_RESET_SDCON_EX");
	GET_PROC(FTYPE_LONG_INT_PCHAR_INT																								,					tsph_get_mhe_packet_info, "TSPH_GET_MHE_PACKET_INFO");
	GET_PROC(FTYPE_INT_INT																											,					tsph_get_loop_count, "TSPH_GET_LOOP_COUNT");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tspl_set_ad9852_max_clock_ex, "TSPL_SET_AD9852_MAX_CLOCK_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tvb380_set_board_config_status_ex, "TVB380_SET_BOARD_CONFIG_STATUS_EX");
	GET_PROC(FTYPE_INT_INT_PCHAR_PCHAR																								,					tsph_run_atsc_mh_parser, "TSPH_RUN_ATSC_MH_PARSER");
	GET_PROC(FTYPE_INT_VOID																											,					tsph_get_board_location, "TSPH_GET_BOARD_LOCATION");
	GET_PROC(FTYPE_INT_INT																											,					tspl_get_board_id_ex, "TSPL_GET_BOARD_ID_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG																							,					tvb380_set_modulator_agc_ex, "TVB380_SET_MODULATOR_AGC_EX");
	GET_PROC(FTYPE_INT_INT_LONG_DOUBLE_LONG																							,					tvb380_set_modulator_atten_value_ex, "TVB380_SET_MODULATOR_ATTEN_VALUE_EX");
	GET_PROC(FTYPE_DOUBLE_INT_LONG_LONG																								,					tvb380_get_modulator_rf_power_level_ex, "TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX");
	GET_PROC(FTYPE_INT_INT_PCHAR_PCHAR 																								,					tsph_run_mfs_parser, "TSPH_RUN_MFS_PARSER");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_spectrum_inversion_ex, "TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX");
	GET_PROC(FTYPE_INT_INT_INT_DOUBLE 																								,					tsph_set_current_offset, "TSPH_SET_CURRENT_OFFSET");
	GET_PROC(FTYPE_INT_INT_INT_INT 																									,					tspl_set_board_led_status_ex, "TSPL_SET_BOARD_LED_STATUS_EX");
	GET_PROC(FTYPE_INT_INT 																											,					tspl_get_board_rev_ex, "TSPL_GET_BOARD_REV_EX");
	GET_PROC(FTYPE_INT_INT 																											,					tspl_get_board_config_status_ex, "TSPL_GET_BOARD_CONFIG_STATUS_EX");
	GET_PROC(FTYPE_INT_INT 																											,					tspl_get_authorization_ex, "TSPL_GET_AUTHORIZATION_EX");
	GET_PROC(FTYPE_INT_INT_INT 																										,					tspl_get_fpga_info_ex, "TSPL_GET_FPGA_INFO_EX");
	GET_PROC(FTYPE_INT_INT_LONG 																									,					tvb380_set_stop_mode_ex, "TVB380_SET_STOP_MODE_EX");
	GET_PROC(FTYPE_INT_INT_INT_INT 																									,					tsph_set_sdram_bank_info, "TSPH_SET_SDRAM_BANK_INFO");
	GET_PROC(FTYPE_INT_INT_INT 																										,					tsph_start_monitor, "TSPH_START_MONITOR");
	GET_PROC(FTYPE_INT_INT_INT 																										,					tspl_set_tsio_direction_ex, "TSPL_SET_TSIO_DIRECTION_EX");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tsph_set_modulator_type, "TSPH_SET_MODULATOR_TYPE");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 																	,					tsph_set_error_injection, "TSPH_SET_ERROR_INJECTION");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tvb380_set_config_ex, "TVB380_SET_CONFIG_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tvb380_set_modulator_if_freq_ex, "TVB380_SET_MODULATOR_IF_FREQ_EX");
	GET_PROC(FTYPE_INT_INT_LONG_DOUBLE_LONG 																						,					tvb380_set_modulator_freq_ex, "TVB380_SET_MODULATOR_FREQ_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG 																			,					tvb380_set_modulator_dvbh_ex, "TVB380_SET_MODULATOR_DVBH_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_txmode_ex, "TVB380_SET_MODULATOR_TXMODE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_coderate_ex, "TVB380_SET_MODULATOR_CODERATE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_guardinterval_ex, "TVB380_SET_MODULATOR_GUARDINTERVAL_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_constellation_ex, "TVB380_SET_MODULATOR_CONSTELLATION_EX");
	GET_PROC(FTYPE_INT_INT_LONG_DOUBLE_LONG																							,					tvb380_set_modulator_symrate_ex, "TVB380_SET_MODULATOR_SYMRATE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tvb380_set_modulator_interleave_ex, "TVB380_SET_MODULATOR_INTERLEAVE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_prbs_mode_ex, "TVB380_SET_MODULATOR_PRBS_MODE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_DOUBLE 																								,					tvb380_set_modulator_prbs_scale_ex, "TVB380_SET_MODULATOR_PRBS_SCALE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_DOUBLE 																						,					tvb380_set_modulator_prbs_info_ex, "TVB380_SET_MODULATOR_PRBS_INFO_EX");
	GET_PROC(FTYPE_INT_INT_LONG 																									,					tsph_set_tmcc_remuxer, "TSPH_SET_TMCC_REMUXER");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_roll_off_factor_ex, "TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG																								,					tvb380_set_modulator_pilot_ex, "TVB380_SET_MODULATOR_PILOT_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tspl_set_max_playrate_ex, "TSPL_SET_MAX_PLAYRATE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG																							,					tvb380_set_modulator_bert_measure_ex, "TVB380_SET_MODULATOR_BERT_MEASURE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tvb380_set_modulator_sdram_clock_ex, "TVB380_SET_MODULATOR_SDRAM_CLOCK_EX");
	GET_PROC(FTYPE_INT_INT_INT_PCHAR 																								,					tspl_get_encrypted_sn_ex, "TSPL_GET_ENCRYPTED_SN_EX");
	GET_PROC(FTYPE_INT_INT_LONG 																									,					tvb380_is_enabled_type_ex, "TVB380_IS_ENABLED_TYPE_EX");
	GET_PROC(FTYPE_INT_INT 																											,					tspl_get_last_error_ex, "TSPL_GET_LAST_ERROR_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG 																								,					tspl_set_play_rate_ex, "TSPL_SET_PLAY_RATE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 																	,					tvb380_set_modulator_dtmb_ex, "TVB380_SET_MODULATOR_DTMB_EX");
	GET_PROC(FTYPE_INT_INT_PCHAR 																									,					tsph_set_filename_next_play, "TSPH_SET_FILENAME_NEXT_PLAY");
	GET_PROC(FTYPE_INT_INT_PCHAR 																									,					tsph_set_isdbs_base_ts, "TSPH_SET_ISDBS_BASE_TS");
	GET_PROC(FTYPE_INT_INT_PCHAR_PCHAR_LONG_PCHAR_LONG_LONG 																		,					tsph_set_rx_ip_streaming_info, "TSPH_SET_RX_IP_STREAMING_INFO");
	GET_PROC(FTYPE_INT_PCHAR_LONG_DOUBLE_LONG_LONG_LONG_PCHAR																		,					tsph_start_ip_streaming, "TSPH_START_IP_STREAMING");
	GET_PROC(FTYPE_INT_INT_PCHAR_LONG_DOUBLE_LONG_LONG 																				,					tsph_start_play, "TSPH_START_PLAY");
	GET_PROC(FTYPE_INT_INT_PCHAR_INT 																								,					tsph_start_record, "TSPH_START_RECORD");
	GET_PROC(FTYPE_INT_INT_INT 																										,					tsph_set_repeat_play_mode, "TSPH_SET_REPEAT_PLAY_MODE");
	GET_PROC(FTYPE_INT_INT_PCHAR_INT 																								,					tsph_get_remux_info, "TSPH_GET_REMUX_INFO");
	GET_PROC(FTYPE_INT_INT_PCHAR 																									,					tspl_check_ln_ex, "TSPL_CHECK_LN_EX");
	GET_PROC(FTYPE_DOUBLE_INT																										,					tsph_get_current_record_point, "TSPH_GET_CURRENT_RECORD_POINT");
	GET_PROC(FTYPE_INT_INT																											,					tsph_get_current_thread_state, "TSPH_GET_CURRENT_THREAD_STATE");
	GET_PROC(FTYPE_DOUBLE_INT_LONG																									,					tvb380_get_modulator_bert_result_ex, "TVB380_GET_MODULATOR_BERT_RESULT_EX");
	GET_PROC(FTYPE_INT_INT																											,					tsph_get_play_buffer_status, "TSPH_GET_PLAY_BUFFER_STATUS");
	GET_PROC(FTYPE_LONG_INT_LONG																									,					tsph_get_program, "TSPH_GET_PROGRAM");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tsph_get_remux_datarate, "TSPH_GET_REMUX_DATARATE");
	GET_PROC(FTYPE_INT_INT																											,					tsph_is_video_window_visible, "TSPH_IS_VIDEO_WINDOW_VISIBLE");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT_INT																							,					tsph_move_video_window, "TSPH_MOVE_VIDEO_WINDOW");
	GET_PROC(FTYPE_DWORD_INT																										,					tspl_read_input_status_ex, "TSPL_READ_INPUT_STATUS_EX");
	GET_PROC(FTYPE_DOUBLE_INT																										,					tspl_read_input_tscount_ex, "TSPL_READ_INPUT_TSCOUNT_EX");
	GET_PROC(FTYPE_INT_INT_LONG																										,					tsph_read_ip_streaming_input_status, "TSPH_READ_IP_STREAMING_INPUT_STATUS");
	GET_PROC(FTYPE_INT_INT_INT_INT_INT_PCHAR_INT_PCHAR_INT_PCHAR_INT_PCHAR															,					tsph_set_layer_info, "TSPH_SET_LAYER_INFO");
	GET_PROC(FTYPE_LONG_INT_LONG																									,					tsph_set_program, "TSPH_SET_PROGRAM");
	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG 		,					tsph_set_remux_info, "TSPH_SET_REMUX_INFO");
	GET_PROC(FTYPE_INT_INT_INT																										,					tsph_show_video_window, "TSPH_SHOW_VIDEO_WINDOW");
	GET_PROC(FTYPE_LONG_INT																											,					tsph_exit_process, "TSPH_EXIT_PROCESS");
	GET_PROC(FTYPE_INT_INT																											,					tsph_clear_remux_info, "TSPH_CLEAR_REMUX_INFO");
	GET_PROC(FTYPE_INT_INT_PCHAR_PCHAR 																								,					tsph_run_c2mi_parser, "TSPH_RUN_C2MI_PARSER");
	//2011/8/31 added
	GET_PROC(FTYPE_INT_INT_PUNINT_PUNINT																							,					tsph_ip_recv_status, "TSPH_IP_RECV_STATUS");
	//2011/10/24 added PAUSE
	GET_PROC(FTYPE_INT_INT_INT 																										,					tsph_start_delay, "TSPH_START_DELAY");
	
	//2011/11/10 ISDB-T 13seg 188 TS Loopthru
	GET_PROC(FTYPE_INT_INT																											,					tsph_is_loopthru_isdbt13_188, "TSPH_IS_LOOPTHRU_ISDBT13_188");
	GET_PROC(FTYPE_INT_INT_PPSTRUCT																									,					tsph_pause_loopthru_isdbt13_parser, "TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser");
	GET_PROC(FTYPE_INT_INT																											,					tsph_buf_clear_loopthru_isdbt13_188, "TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188");
	GET_PROC(FTYPE_INT_INT																											,					tsph_is_loopthru_inbuf_full_isdbt13_188, "TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188");
	//2011/11/28 TVB594
	GET_PROC(FTYPE_INT_INT																											,					tsph__stream_number, "TSPH__STREAM_NUMBER");
	GET_PROC(FTYPE_INT_INT_INT 																										,					tsph__sel_ts_of_asi310out, "TSPH__SEL_TS_of_ASI310OUT");

	//2012/2/15 NIT
	GET_PROC(FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT_PINT_PINT 																		,					tsph_get_nit_satellite_info, "TSPH_GET_NIT_SATELLITE_INFO");
	GET_PROC(FTYPE_INT_INT_PINT_PINT_PINT_PINT_PINT 																				,					tsph_get_nit_cable_info, "TSPH_GET_NIT_CABLE_INFO");
	GET_PROC(FTYPE_INT_INT_PINT_PUINT_PINT_PINT_PINT_PINT_PINT_PINT_PINT															,					tsph_get_nit_terrestrial_info, "TSPH_GET_NIT_TERRESTRIAL_INFO");

	//2012/4/12 SINGLE TONE
	GET_PROC(FTYPE_INT_LONG_DWORD_LONG																								,					tvb380_set_modulator_single_tone_ex, "TVB380_SET_MODULATOR_SINGLE_TONE_EX");
	//2012/3/21
	GET_PROC(FTYPE_INT_VOID																											,					tsph_getrealbdcnt_n, "TSPH_GetRealBdCnt_N");
	GET_PROC(FTYPE_INT_INT_PINT																										,					tsph_getrealandvirbdmap, "TSPH_GetRealAndVirBdMap");
	GET_PROC(FTYPE_INT_INT																											,					tsph_getbdid_n, "TSPH_GetBdId_N");
	GET_PROC(FTYPE_PCHAR_INT																										,					tsph_getbdname_n, "TSPH_GetBdName_N");
	GET_PROC(FTYPE_INT_VOID																											,					tsph_conftvbsystem, "TSPH_ConfTvbSytem");
	GET_PROC(FTYPE_INT_VOID																											,					tsph_initallrealbd, "TSPH_InitAllRealBd");
	GET_PROC(FTYPE_INT_INT_INT																										,					tsph_initvirbd, "TSPH_InitVirBd");
	GET_PROC(FTYPE_INT_INT_INT_INT																									,					tsph_activateonebd, "TSPH_ActivateOneBd");
	GET_PROC(FTYPE_INT_INT																											,					tspl_cnt_multi_vsb_rfout_ex, "TSPL_CNT_MULTI_VSB_RFOUT_EX");
	GET_PROC(FTYPE_INT_INT																											,					tspl_cnt_multi_qam_rfout_ex, "TSPL_CNT_MULTI_QAM_RFOUT_EX");
	//2012/6/28 multi dvb-t
	GET_PROC(FTYPE_INT_INT																											,					tspl_cnt_multi_dvbt_rfout_ex, "TSPL_CNT_MULTI_DVBT_RFOUT_EX");
	//2012/8/7 SI/PID improve
	GET_PROC(FTYPE_INT_INT_PCHAR_INT_PPSTRUCT																						,					tsph_run_ts_parser2, "TSPH_RUN_TS_PARSER2");	
	//2012/8/31 new rf level control
	GET_PROC(FTYPE_INT_INT_LONG_DOUBLE_PLONG_LONG																					,					tvb59x_set_modulator_rf_level_value_ex, "TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX");
	GET_PROC(FTYPE_INT_INT_LONG_PDOUBLE_PDOUBLE_LONG																				,					tvb59x_get_modulator_rf_level_range_ex, "TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX");
	//2012/9/6 pcr restamp
	GET_PROC(FTYPE_INT_INT_INT																										,					tvb59x_set_pcr_stamp_cntl_ex, "TVB59x_SET_PCR_STAMP_CNTL_EX");
#ifdef WIN32
	GET_PROC(FTYPE_INT_INT_INT_PCHAR 																								,					tsph_set_t2mi_stream_generation, "TSPH_SET_T2MI_STREAM_GENERATION");
	GET_PROC(FTYPE_INT_INT_INT																										,					tvb59x_set_output_ts_type_ex, "TVB59x_SET_Output_TS_Type_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					tvb59x_set_reset_control_reg_ex, "TVB59x_SET_Reset_Control_REG_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					tspl_set_sdcon_mode_ex, "TSPL_SET_SDCON_MODE_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					call_tvb59x_set_tspacket_cnt_mode_ex, "TSPL_TVB59x_SET_TsPacket_CNT_Mode_EX");
	GET_PROC(FTYPE_INT_INT_PINT_PINT																							,					call_tvb59x_get_asi_input_rate_ex, "TSPL_TVB59x_Get_Asi_Input_rate_EX");

	GET_PROC(FTYPE_INT_INT_LONG_LONG_LONG																		,					call_tvb59x_modulator_status_control_ex, "TVB59x_Modulator_Status_Control_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					call_tvb59x_get_modulator_status_ex		, "TVB59x_Get_Modulator_Status_EX");
	GET_PROC(FTYPE_INT_INT_INT																										,					call_tspl_get_tsio_status_ex	, "TSPL_GET_TSIO_STATUS_EX");
#endif
	return 0;									
}
void CWRAP_DLL::ReleaseHLDLibrary()
{

	tsph_set_c2mi_params						 = NULL;
	tspl_get_ad9775_ex							 = NULL;
	tspl_set_ad9775_ex							 = NULL;
	tspl_get_ad9787_ex							 = NULL;
	tspl_set_ad9787_ex							 = NULL;
	tsph_try_alloc_iq_memory					 = NULL;
    tsph_set_iq_mode							 = NULL;
    tsph_set_combiner_info						 = NULL;
    tsph_cal_play_rate							 = NULL;
    tsph_is_combined_ts							 = NULL;
    tsph_get_ts_id								 = NULL;
	tspl_read_control_reg_ex					 = NULL;
	tspl_write_control_reg_ex					 = NULL;
    tsph_set_loop_adaptation					 = NULL;
    tsph_set_t2mi_params						 = NULL;
    tvb380_set_modulator_bandwidth_ex			 = NULL;
	tsph_get_t2mi_params						 = NULL;
    tsph_run_t2mi_parser						 = NULL;
    tsph_isdbs_calc_combined_ts_bitrate			 = NULL;
	tspl_reset_sdcon_ex							 = NULL;
	tsph_get_mhe_packet_info					 = NULL;
	tsph_get_loop_count							 = NULL;
	tspl_set_ad9852_max_clock_ex				 = NULL;
    tvb380_set_board_config_status_ex			 = NULL;
    tsph_run_atsc_mh_parser						 = NULL;
    tsph_get_board_location						 = NULL;
    tspl_get_board_id_ex						 = NULL;
    tvb380_set_modulator_agc_ex					 = NULL;
	tvb380_set_modulator_atten_value_ex			 = NULL;
	tvb380_get_modulator_rf_power_level_ex		 = NULL;
	tsph_run_mfs_parser							 = NULL;
	tvb380_set_modulator_spectrum_inversion_ex	 = NULL;
    tsph_set_current_offset						 = NULL;
    tspl_set_board_led_status_ex				 = NULL;
    tspl_get_board_rev_ex						 = NULL;
	tspl_get_board_config_status_ex				 = NULL;
    tspl_get_authorization_ex					 = NULL;
    tspl_get_fpga_info_ex						 = NULL;
	tvb380_set_stop_mode_ex						 = NULL;
	tsph_set_sdram_bank_info					 = NULL;
	tsph_start_monitor							 = NULL;
	tspl_set_tsio_direction_ex					 = NULL;
    tsph_set_modulator_type						 = NULL;
    tsph_set_error_injection					 = NULL;
    tvb380_set_config_ex						 = NULL;
    tvb380_set_modulator_if_freq_ex				 = NULL;
    tvb380_set_modulator_freq_ex				 = NULL;
	tvb380_set_modulator_dvbh_ex				 = NULL;
	tvb380_set_modulator_txmode_ex				 = NULL;
	tvb380_set_modulator_coderate_ex			 = NULL;
	tvb380_set_modulator_guardinterval_ex		 = NULL;
    tvb380_set_modulator_constellation_ex		 = NULL;
    tvb380_set_modulator_symrate_ex				 = NULL;
    tvb380_set_modulator_interleave_ex			 = NULL;
	tvb380_set_modulator_prbs_mode_ex			 = NULL;
    tvb380_set_modulator_prbs_scale_ex			 = NULL;
    tvb380_set_modulator_prbs_info_ex			 = NULL;
	tsph_set_tmcc_remuxer						 = NULL;
	tvb380_set_modulator_roll_off_factor_ex		 = NULL;
	tvb380_set_modulator_pilot_ex				 = NULL;
	tspl_set_max_playrate_ex					 = NULL;
    tvb380_set_modulator_bert_measure_ex		 = NULL;
    tvb380_set_modulator_sdram_clock_ex			 = NULL;
    tspl_get_encrypted_sn_ex					 = NULL;
    tvb380_is_enabled_type_ex					 = NULL;
    tspl_get_last_error_ex						 = NULL;
	tspl_set_play_rate_ex						 = NULL;
	tvb380_set_modulator_dtmb_ex				 = NULL;
	tsph_set_filename_next_play					 = NULL;
    tsph_set_isdbs_base_ts						 = NULL;
    tsph_set_rx_ip_streaming_info				 = NULL;
    tsph_start_ip_streaming						 = NULL;
	tsph_start_play								 = NULL;
    tsph_start_record							 = NULL;
    tsph_set_repeat_play_mode					 = NULL;
	tsph_get_remux_info							 = NULL;
	tspl_check_ln_ex							 = NULL;
	tsph_get_current_record_point				 = NULL;
    tsph_get_current_thread_state				 = NULL;
    tvb380_get_modulator_bert_result_ex			 = NULL;
    tsph_get_play_buffer_status					 = NULL;
    tsph_get_program							 = NULL;
    tsph_get_remux_datarate						 = NULL;
	tsph_is_video_window_visible				 = NULL;
	tsph_move_video_window						 = NULL;
	tspl_read_input_status_ex					 = NULL;
	tspl_read_input_tscount_ex					 = NULL;
    tsph_read_ip_streaming_input_status			 = NULL;
    tsph_set_layer_info							 = NULL;
	tsph_set_program							 = NULL;
    tsph_set_remux_info							 = NULL;
    tsph_show_video_window						 = NULL;
	tsph_exit_process							 = NULL;
	tsph_clear_remux_info						 = NULL; 
	tsph_run_c2mi_parser						 = NULL;	
	//2011/8/31 added
	tsph_ip_recv_status							 = NULL;
	//2011/10/24 added PAUSE
	tsph_start_delay							 = NULL;

	//2011/11/10 ISDB-T 188 TS LoopThru
	tsph_is_loopthru_isdbt13_188				 = NULL;
	tsph_pause_loopthru_isdbt13_parser					= NULL;
	tsph_buf_clear_loopthru_isdbt13_188			 = NULL;
	tsph_is_loopthru_inbuf_full_isdbt13_188		 = NULL;
	//2011/11/28 TVB594
	tsph__stream_number							 = NULL;		
	tsph__sel_ts_of_asi310out					 = NULL;
	//2012/2/15 NIT
	tsph_get_nit_satellite_info					 = NULL;
	tsph_get_nit_cable_info						 = NULL;
	tsph_get_nit_terrestrial_info				 = NULL;

	//2012/4/12 SINGLE TONE
	tvb380_set_modulator_single_tone_ex			 = NULL;
	//2012/3/21
	tsph_getrealbdcnt_n							 = NULL;
	tsph_getrealandvirbdmap						 = NULL;
	tsph_getbdid_n 								 = NULL;
	tsph_getbdname_n							 = NULL;
	tsph_conftvbsystem							 = NULL;
	tsph_initallrealbd							 = NULL;
	tsph_initvirbd 								 = NULL;
	tsph_activateonebd							 = NULL;
	tspl_cnt_multi_vsb_rfout_ex					 = NULL;
	tspl_cnt_multi_qam_rfout_ex					 = NULL;
	//2012/6/28 multi dvb-t
	tspl_cnt_multi_dvbt_rfout_ex					 = NULL;
	//2012/8/7 SI/PID improve
	tsph_run_ts_parser2								= NULL;
	//2012/8/31 new rf level control
	tvb59x_set_modulator_rf_level_value_ex		 = NULL;
	tvb59x_get_modulator_rf_level_range_ex		 = NULL;
	//2012/9/6 pcr restamp
	tvb59x_set_pcr_stamp_cntl_ex				 = NULL;
#ifdef WIN32
	tsph_set_t2mi_stream_generation			 = NULL;
	tvb59x_set_output_ts_type_ex					= NULL;
	tspl_set_sdcon_mode_ex							= NULL;
	call_tvb59x_set_tspacket_cnt_mode_ex	= NULL;
	call_tvb59x_get_asi_input_rate_ex				= NULL;
	call_tvb59x_modulator_status_control_ex = NULL;
	call_tvb59x_get_modulator_status_ex		 = NULL;
	call_tspl_get_tsio_status_ex = NULL;
#endif

	if(m_hInstance != NULL)
	{
		FreeLibrary(m_hInstance);
		m_hInstance = NULL;
	}
}

#else
int CWRAP_DLL::Set_Loop_Adaptation(int nSlot, 
										 long pcr_restamping,		/* 0=0ff, 1=on */
										 long continuity_conunter,	/* 0=0ff, 1=on */
										 long tdt_tot,				/* 0=off, 1=2nd loop, 2=current date/time, 3=user date/time */
										 char* user_date,			/* yyyy-mm-dd (tdt_tot==3), ex) 2009-11-28 */
										 char* user_time)			/* hh-mm-ss (tdt_tot==3), ex) 01-18-50 */
{
	TSPH_SET_LOOP_ADAPTATION(nSlot, pcr_restamping, continuity_conunter, tdt_tot,user_date, user_time);
}
#endif
