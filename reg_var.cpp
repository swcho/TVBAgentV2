// Registry Variable maninpulation

#include "stdafx.h"
#ifdef WIN32
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include "main.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_ind.h"
#include "util_dep.h"
#include "baseutil.h"
#include "PlayForm.h"


using namespace System; 
using namespace Microsoft::Win32;
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "variable.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include "hld_api.h"
#endif

extern APPLICATION_CONFIG      *gpConfig;
extern TEL_GENERAL_VAR          gGeneral;
extern CUTIL_IND                gUtilInd;
extern CUTILITY                 gUtility;
extern CWRAP_DLL                gWrapDll;

CREG_VAR                        gRegVar;

#ifdef WIN32
using namespace System;
using namespace TPG0590VC;
using namespace System::Windows::Forms;
using namespace System::Threading;
using namespace System::IO;
using namespace System::Globalization;
using namespace System::Diagnostics;
using namespace System::Text;
#endif
char    *gstrModTypeReg[] = {
        "DVB-T", "VSB", "QAM-A", "QAM-B", "QPSK",
        "TDMB", "16VSB", "DVB-H", "DVB-S2", "ISDB-T",
        "ISDB-T-13", "DTMB", "CMMB", "DVB-T2", "RESERVED",
		"ATSC-MH", "IQ-PLAY", "ISDB-S", "DVB-C2", "MULTI-J.83B", "MULTI-VSB", "MULTI-DVB-T", "TS-IO"};	//2010/12/06 ISDB-S //2011/2/23

//---------------------------------------------------------------------------
CREG_VAR* gc_reg_var = NULL;


//---------------------------------------------------------------------------
CREG_VAR::CREG_VAR()
{
}


//---------------------------------------------------------------------------
// Parse input parameter.
// Set variables
// Option List
//  -slot                   1
//  -help
//  -auto-run
//  -folder
//  -file                   5
//  -mode
//  -playrate
//  -type
//  -rf
//  -if                     10
//  -symbol
//  -coderate
//  -const
//  -bw
//  -tx                     15
//  -guard
//  -inberleaving
//  -frame-header
//  -carrier-number
//  -frame-header-pn        20
//  -pilot-insert
//  -spectrum
//  -noise-mode
//  -noise-power
//  -atten                  25
//  -ext-atten
//  -input-source
//  -mpe-fec
//  -time-slice
//  -in-depth               30
//  -cell-id
//  -pilot
//  -roll-off
//  -max-playrate
//  -null-tp                35
//  -restamping
//  -continuity
//  -timeoffset
//  -reset-playrate
//  -av-decoding            40
//  -ip-streaming
//  -bypass-amp
//  -exit-time
//  -exit-date
//  -duration               45
void CREG_VAR::modCommandLine(long nActiveBoardNum)
{
#ifdef WIN32
    int     iNumParam = 0;
	array<String^>^arguments = Environment::GetCommandLineArgs();
	iNumParam = arguments->Length;
#else		// linux
	// huy: ? 
	// different from window version 
	int 	iNumParam = 0;
#endif
    char    str[256];                       // parameter option. ParamStr(0) = execution name, ParamStr(1) = parameter 1
    char    str2[100];                      // parameter value
    char    strFilename[256];
    long    nBoardNum;
    int     i,j;
    char    strMsg[2048];
    long    lValue, tmp;
    time_t      tCur;
    struct tm   nt;
	//2010/12/14
#ifdef WIN32
	String^	strHelp = "";
#endif
    //-------------------------------------------------------
    // No Parameter
    if (iNumParam <= 1)
        return;

    //kslee 2010/4/20
	nBoardNum = nActiveBoardNum;
	
	//kslee 2010/4/27
	//gGeneral.gnRunCommandFlag = 1;

	for (i = 1; i < iNumParam;)
    {
        //---------------------------------------
        // str = option, str1 = parameter
		
#if defined(WIN32)
		gBaseUtil.ChangeCharFromString(arguments[i], str);
#else
		gUtility.MyStrCpy(str, 256, gUtility.Get_Main_Parameter(i));
#endif
        if (str[0] != '-')
        {
            i++;
            continue;
        }

        //----------------------------
        if (strcmp(str,"-help") == 0)
        {
#ifdef WIN32
			//2010/12/14
			String^ FileName = Application::StartupPath + "\\" + Application::ProductName + ".txt";
			StreamWriter^ sw = gcnew StreamWriter(FileName, false);
			strHelp = "-slot 0,1,2,...,23(USB#1),22(USB#2),21(USB#3),20(USB#4)\r\n" +
					  "-folder c:\\ts\r\n" + 
					  "-file test.trp\r\n" +
					  "-mode loop, once\r\n" +
					  "-playrate 19362658\r\n" +
					  "-type dvb-t, 8vsb, qam-ac, qam-b, qpsk, tdmb, 16vsb, dvb-h, \r\n" + 
					  "         dvb-s2, isdb-t, isdb-t-13, dtmb, cmmb, atsc-mh, dvb-t2\r\n" +
					  "-rf 473000000\r\n" + 
					  "-if 36000000, 44000000\r\n" +
					  "-symbol [1000000~7200000 : DVB-C(J.83 A,C)],\r\n" +
					  "           [1000000~45000000 : DVB-S, RRC FILTER=OFF],\r\n" +
					  "           [1000000~22000000 : DVB-S, RRC FILTER=ON],\r\n" +
					  "           [1000000~45000000 : DVB-S2, ROLL-OFF=NONE, IF=36MHz],\r\n" +
					  "           [1000000~25000000 : DVB-S2, ROLL-OFF=NONE, IF=44MHz],\r\n" +
					  "           [1000000~45000000 : DVB-S2, ROLL-OFF=0.2/0.25/0.35],\r\n" +
					  "           [1000000~45000000 : TVB590S, TVB593, DVB-S, DVB-S2]\r\n";
			gUtility.DisplayMessage(strHelp);
			sw->Write(strHelp);
			strHelp = "-coderate [1/2, 2/3, 3/4, 5/6, 7/8 : DVB-T, DVB-H, DVB-S, ISDB-S],\r\n" +
					  "             [1/4, 1/3, 2/5, 1/2, 3/5, 2/3, 3/4, 4/5, 5/6, 8/9, 9/10 : DVB-S2, CONSTELLATION=QPSK],\r\n" +
					  "             [3/5, 2/3, 3/4, 5/6, 8/9, 9/10 : DVB-S2, CONSTELLATION=8PSK],\r\n" +
					  "             [2/3, 3/4, 4/5, 5/6, 8/9, 9/10 : DVB-S2, CONSTELLATION=16APSK],\r\n" +
					  "             [3/4, 4/5, 5/6, 8/9, 9/10 : DVB-S2, CONSTELLATION=32APSK],\r\n" +
					  "             [0.4, 0.6, 0.8 : DTMB]\r\n" +
					  "-const [QPSK, 16QAM, 64QAM : DVB-T, DVB-H],\r\n" +       
				      "          [16QAM, 32QAM, 64QAM, 128QAM, 256QAM : DVB-C(J.83 A,C)],\r\n" +
					  "          [64QAM, 256QAM : DVB-C(J.83 B)],\r\n" +
					  "          [QPSK, 8PSK, 16APSK, 32APSK : DVB-S2],\r\n" +
					  "          [4QAM-NR, 4QAM, 16QAM, 32QAM, 64QAM : DTMB],\r\n" +
					  "          [BPSK, QPSK, TC8PSK : ISDB-S]\r\n" +
					  "-bw [6MHz, 7MHz, 8MHz : DVB-T],\r\n" +
					  "       [6MHz, 7MHz, 8MHz, 5MHz : DVB-H, DVB-T2]\r\n" +
					  "-tx [2K, 8K : DVB-T],\r\n" +
					  "       [2K, 8K, 4K : DVB-H]\r\n" +  
					  "-guard [1/4, 1/8, 1/16, 1/32 : DVB-T, DVB-H]\r\n" +
					  "-mpe-fec [off, on : DVB-H]\r\n" +
				      "-time-slice [off, on : DVB-H]\r\n" +
					  "-in-depth [off, on : DVB-H]\r\n" +
					  "-cell-id [0~65535 : DVB-H]\r\n";
		    gUtility.DisplayMessage(strHelp);		  
			sw->Write(strHelp);		  
					  
			strHelp = "-spectrum [normal, inverse : DVB-S, DVB-S2]\r\n" +
				      "-roll-off [0.20. 0.25, 0.35, none : DVB-S2]\r\n" +
					  "-pilot [off, on : DVB-S2]\r\n" +
					  "-interleaving [128-1-1, 64-2, 32-4, 16-8, 8-16, 128-1, 128-2,\r\n" +
					  "                 128-3, 128-4, 128-5, 128-6, 128-7, 128-8 : DVB-C(J.83 B)],\r\n" +
					  "                 [52/240, 52/720 : DTMB]\r\n"
					  "-frame-header [PN420, PN595, PN945 : DTMB]\r\n" +
					  "-carrier-number [1(ADTB-T), 3780(DTMB) : DTMB]\r\n" +
					  "-frame-header-pn [fixed, rotated :DTMB]\r\n" +
					  "-pilot-insert [off, on : DTMB]\r\n" +
					  "-use-tmcc [off, on : ISDB-T]\r\n" +
					  "-multi-ts [off, on : ISDB-S]\r\n" +
					  "-emergency-alarm [off, on : ISDB-T, ISDB-S]\r\n" +
					  "-max-playrate off, on\r\n" +
					  "-null-tp off, on\r\n" +
					  "-bypass-amp off, on\r\n" +
					  "-atten 0~31.5\r\n"	+
					  "-cnr 0~50 : -noise-mode=7/10/15/23\r\n" +
					  "-noise-mode none, 7, 10, 15, 23\r\n" +
					  "-restamping off, on\r\n" +
					  "-continuity off, on\r\n" +
					  "-timeoffset on, on(current time), on(user time), off : TDT/TOT UDT TIME SETTING\r\n" +
					  "-user-date yyyy-mm-dd : TDT/TOT UDT TIME SETTING\r\n" +
					  "-user-time hh-mm-ss : TDT/TOT UDT TIME SETTING\r\n";
					  
		    gUtility.DisplayMessage(strHelp);		  
			sw->Write(strHelp);

			strHelp = "-sub-loop-start-time hh:mm:ss\r\n" +
					  "-sub-loop-end-time hh:mm:ss\r\n" +
					  "-reset-playrate off, on\r\n" +
					  "-error-lose-packet # : ONCE EVERY # PACKET\r\n" +
					  "-error-bits-packet # : ONCE EVERY # PACKET\r\n" +
					  "-error-bits-count # : # PER PACKET\r\n" +
					  "-error-bytes-packet # : ONCE EVERY # PACKET\r\n" +
					  "-error-bytes-count # : # PER PACKET\r\n" +
					  "-av-decoding off on\r\n" +
					  "-ip-streaming off, on\r\n" +
					  "-auto-run play, record, monitor, play-stop\r\n"
					  "-duration 1~(min.)\r\n" +
					  "-exit-date mm/dd/yyyy\r\n" +
					  "-exit-time Hh:Mm:Ss\r\n" +
					  "* Please, type [-type] argument before other commands are entered.\r\n" +
					  "** Refer to " + FileName + ".";
			gUtility.DisplayMessage(strHelp);
			sw->Write(strHelp);
			sw->Flush();
			sw->Close();
			delete sw;
#else
            gUtility.MySprintf(strMsg, 2048, (char *) "-slot 0,1,2,...,23(USB#1),22(USB#2),21(USB#3),20(USB#4)\n-folder c:\\ts\n-file test.trp\n-mode loop, once\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-playrate 19362658\n-type dvb-t, 8vsb, qam-ac, qam-b, qpsk, tdmb, 16vsb, dvb-h, dvb-s2, isdb-t, isdb-t-13, dtmb\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-rf 473000000\n-if 36000000, 44000000\n-symbol [1000000~8000000], [1000000~45000000]\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-coderate [1/2, 2/3, 3/4, 5/6, 7/8]\n          [1/4, 1/3, 2/5, 1/2, 3/5, 2/3, 3/4, 4/5, 5/6, 8/9, 9/10]\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "          [0.4, 0.6, 0.8]\n-const [QPSK, 16QAM, 64QAM],\n       [16QAM, 32QAM, 64QAM, 128QAM, 256QAM],\n       [64QAM, 256QAM],\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "       [QPSK, 8PSK, 16APSK, 32APSK],\n       [4QAM-NR, 4QAM, 16QAM, 32QAM, 64QAM]\n-bw 6MHz, 7MHz, 8MHz, 5MHz\n-tx 2K, 8K, 4K\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-guard 1/4, 1/8, 1/16, 1/32\n-interleaving [128-1-1, 64-2, 32-4, 16-8, 8-16, 128-1, 128-2, 128-3, 128-4, 128-5, 128-6, 128-7, 128-8]\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "              [52/240, 52/720]\n-frame-header PN420, PN595, PN945\n-carrier-number 1(ADTB-T), 3780(DTMB)\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-frame-header-pn fixed, rotated\n-pilot-insert off, on\n-spectrum normal, inverse");
            gUtility.DisplayMessage(strMsg);

            gUtility.MySprintf(strMsg, 2048, (char *) "-noise-mode none, 7, 10, 15, 23\n-noise-power -70~20\n-atten 0~31.5\n-ext-atten 0~127\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-mpe-fec on, off\n-time-slice on, off\n-in-depth on, off\n-cell-id 0~65535\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-pilot on, off\n-roll-off 0.20. 0.25, 0.35, none\n-max-playrate on, off\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-null-tp on, off\n-restamping on, off\n-reset-playrate on, off\n-av-decoding on, off\n");
            gUtility.MyStrCat(strMsg, 2048, (char *) "-ip-streaming on, off\n-continuity on, off\n-timeoffset on, on(current time), on(user time), off\n-user-date yyyy-mm-dd\n");
            gUtility.MyStrCat(strMsg, 2048,  (char *) "-bypass-amp on, off\n-auto-run play, record, monitor\n-duration 1~(min.)\n-exit-date mm/dd/yyyy\n");
            gUtility.MyStrCat(strMsg, 2048,  (char *) "-user-time hh-mm-ss\n-exit-time Hh:Mm:Ss\n* Please, type [-type] argument before others are typed *");
            gUtility.DisplayMessage(strMsg);
#endif
            return;
        }

        //--------------------------------------------------------------------
        // Last parameter
        if (i >= iNumParam)
            break;
#if defined(WIN32)
        gBaseUtil.ChangeCharFromString(arguments[i+1], str2);
#else
       gUtility.MyStrCpy(str2, 100, gUtility.Get_Main_Parameter(i+1));
#endif
		if (str2[0] == '-')
        {
            i = i+1;
            continue;
        }
        
        //--------------------------------------------------------------------
        if (strcmp(str,"-slot") == 0)
        {
            lValue = atoi(str2);
            if (lValue >= 0 && lValue <= MAX_BOARD_COUNT)
            {
                if (gpConfig->gBC[lValue].gnBoardStatus == 1)
				{
                    nBoardNum = lValue;
					//gGeneral.gnActiveBoard = lValue;
				}
            }
        } else if (strcmp(str,"-auto-run") == 0)
        {
            if (strcmp(str2,"play") == 0)
			{
				gpConfig->gBC[nBoardNum].gnCmdAutoRun = 1;
#ifdef WIN32
				gpConfig->gBC[nBoardNum].gnInputSource = FILE_SINGLE_IN;
#endif
			}
            else if (strcmp(str2,"record") == 0)
                gpConfig->gBC[nBoardNum].gnCmdAutoRun = 2;
            else if (strcmp(str2,"monitor") == 0)
                gpConfig->gBC[nBoardNum].gnCmdAutoRun = 3;
			//2010/12/14
			else if (strcmp(str2, "play-stop") == 0)
			{
				gpConfig->gBC[nBoardNum].gnCmdAutoRun = 4;
#ifdef WIN32
				gpConfig->gBC[nBoardNum].gnInputSource = FILE_SINGLE_IN;
#endif
			}
			//2012/4/2
			else if (strcmp(str2, "play-all") == 0)
				gGeneral.gnPlay_AllBd = 1;
        } else if (strcmp(str,"-folder") == 0 || strcmp(str,"-file") == 0)
        {
            // check if path include blank
            gUtility.MyStrCpy(strFilename,  256, str2);
            j = i+2;
            if (j <= iNumParam)
            {
#if defined(WIN32)
                gBaseUtil.ChangeCharFromString(arguments[j], str2);
#else
				gUtility.MyStrCpy(str2, 100, gUtility.Get_Main_Parameter(j));
#endif
				while (str2[0] != '-')      // if not next option
                {
                    gUtility.MyStrCat(strFilename, 256, " ");
                    gUtility.MyStrCat(strFilename, 256, str2);
                    j++;
                    if (j > iNumParam)
                    {
                        if (strcmp(str,"-folder") == 0)
						{
							//strFilename[strlen(strFilename) - 1] = '\0';   
							gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory,  256, strFilename);
						}
						else
                            gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdFileName,  256, strFilename);
                        return;
                    }
#if defined(WIN32)
					gBaseUtil.ChangeCharFromString(arguments[j], str2);
#else
					gUtility.MyStrCpy(str2, 100, gUtility.Get_Main_Parameter(j));
#endif
                }
                i = j - 1;
                if (strcmp(str,"-folder") == 0)
				{
					//strFilename[strlen(strFilename) - 1] = '\0';
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory,  256, strFilename);
				}
				else
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdFileName,  256, strFilename);
                continue;
            }
        } else if (strcmp(str,"-mode") == 0)
        {
            if (strcmp(str2,"once") == 0)
                gpConfig->gBC[nBoardNum].gbRepeatMode = false;
            else
                gpConfig->gBC[nBoardNum].gbRepeatMode = true;
        } else if (strcmp(str,"-playrate") == 0)
        {
            gpConfig->gBC[nBoardNum].gnCmdPlayrate = atoi(str2);
        } else if (strcmp(str,"-type") == 0)
        {
            if (strcmp(str2,"dvb-t") == 0)
                lValue = DVB_T;
            else if (strcmp(str2,"8vsb") == 0)
                lValue = VSB_8;
            else if (strcmp(str2,"qam-ac") == 0)
                lValue = QAM_A;
            else if (strcmp(str2,"qam-b") == 0)
                lValue = QAM_B;
            else if (strcmp(str2,"qpsk") == 0)
                lValue = QPSK;
            else if (strcmp(str2,"tdmb") == 0)
                lValue = TDMB;
            else if (strcmp(str2,"16vsb") == 0)
                lValue = VSB_16;
            else if (strcmp(str2,"dvb-h") == 0)
                lValue = DVB_H;
            else if (strcmp(str2,"dvb-s2") == 0)
                lValue = DVB_S2;
            else if (strcmp(str2,"isdb-t") == 0)
                lValue = ISDB_T;
            else if (strcmp(str2,"isdb-t-13") == 0)
                lValue = ISDB_T_13;
            else if (strcmp(str2,"dtmb") == 0)
                lValue = DTMB;
			else if (strcmp(str2,"cmmb") == 0)
				lValue = CMMB;
			else if (strcmp(str2,"atsc-mh") == 0)
				lValue = ATSC_MH;
			//DVB-T2
			else if (strcmp(str2,"dvb-t2") == 0)
				lValue = DVB_T2;
			//2010/12/15
			else if(strcmp(str2, "isdb-s") == 0)
				lValue = ISDB_S;
            else
                lValue = QAM_A;

            tmp = gpConfig->gBC[nBoardNum].gnModulatorMode;
            gpConfig->gBC[nBoardNum].gnModulatorMode = lValue;
            gUtilInd.SyncModulatorMode(nBoardNum);
            if (gpConfig->gBC[nBoardNum].gnModulatorMode != -1 && gpConfig->gBC[nBoardNum].gnModulatorMode != tmp)
                gpConfig->gBC[nBoardNum].gnCmdInitFW = true;
        } else if (strcmp(str,"-rf") == 0)
        {
            gpConfig->gBC[nBoardNum].gnRFOutFreq = atoi(str2);
        } else if (strcmp(str,"-if") == 0)
        {
            lValue = atoi(str2);
            if (lValue == 36000000)
                gpConfig->gBC[nBoardNum].gnIFOutFreq = IF_OUT_36MHZ;
            else
                gpConfig->gBC[nBoardNum].gnIFOutFreq = IF_OUT_44MHZ;

        } else if (strcmp(str,"-symbol") == 0)
        {
            gpConfig->gBC[nBoardNum].gnSymbolRate = atoi(str2);
        } else if (strcmp(str,"-coderate") == 0)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK)
            {
                if (strcmp(str2,"1/2") == 0)
                    lValue = 0;
                else if (strcmp(str2,"2/3") == 0)
                    lValue = 1;
                else if (strcmp(str2,"3/4") == 0)
                    lValue = 2;
                else if (strcmp(str2,"5/6") == 0)
                    lValue = 3;
                else if (strcmp(str2,"7/8") == 0)
                    lValue = 4;
                else
                    lValue = 0;
				gpConfig->gBC[nBoardNum].gnCodeRate = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
            {
                if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
                {
                    if (strcmp(str2,"1/4") == 0)
                        lValue = 0;
                    else if (strcmp(str2,"1/3") == 0)
                        lValue = 1;
                    else if (strcmp(str2,"2/5") == 0)
                        lValue = 2;
                    else if (strcmp(str2,"1/2") == 0)
                        lValue = 3;
                    else if (strcmp(str2,"3/5") == 0)
                        lValue = 4;
                    else if (strcmp(str2,"2/3") == 0)
                        lValue = 5;
                    else if (strcmp(str2,"3/4") == 0)
                        lValue = 6;
                    else if (strcmp(str2,"4/5") == 0)
                        lValue = 7;
                    else if (strcmp(str2,"5/6") == 0)
                        lValue = 8;
                    else if (strcmp(str2,"8/9") == 0)
                        lValue = 9;
                    else if (strcmp(str2,"9/10") == 0)
                        lValue = 10;
                    else
                        lValue = 0;
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
                {
                    if (strcmp(str2,"3/5") == 0)
                        lValue = 0;
                    else if (strcmp(str2,"2/3") == 0)
                        lValue = 1;
                    else if (strcmp(str2,"3/4") == 0)
                        lValue = 2;
                    else if (strcmp(str2,"5/6") == 0)
                        lValue = 3;
                    else if (strcmp(str2,"8/9") == 0)
                        lValue = 4;
                    else if (strcmp(str2,"9/10") == 0)
                        lValue = 5;
                    else
                        lValue = 0;
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
                {
                    if (strcmp(str2,"2/3") == 0)
                        lValue = 0;
                    else if (strcmp(str2,"3/4") == 0)
                        lValue = 1;
                    else if (strcmp(str2,"4/5") == 0)
                        lValue = 2;
                    else if (strcmp(str2,"5/6") == 0)
                        lValue = 3;
                    else if (strcmp(str2,"8/9") == 0)
                        lValue = 4;
                    else if (strcmp(str2,"9/10") == 0)
                        lValue = 5;
                    else
                        lValue = 0;
                } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
                {
                    if (strcmp(str2,"3/4") == 0)
                        lValue = 0;
                    else if (strcmp(str2,"4/5") == 0)
                        lValue = 1;
                    else if (strcmp(str2,"5/6") == 0)
                        lValue = 2;
                    else if (strcmp(str2,"8/9") == 0)
                        lValue = 3;
                    else if (strcmp(str2,"9/10") == 0)
                        lValue = 4;
                    else
                        lValue = 0;
                }
				gpConfig->gBC[nBoardNum].gnCodeRate = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
            {
                    if (strcmp(str2,"0.4") == 0)
                        lValue = CONST_DTMB_CODE_7488_3008;
                    else if (strcmp(str2,"0.6") == 0)
                        lValue = CONST_DTMB_CODE_7488_4512;
                    else if (strcmp(str2,"0.8") == 0)
                        lValue = CONST_DTMB_CODE_7488_6016;
                    else
                        lValue = CONST_DTMB_CODE_7488_3008;
				gpConfig->gBC[nBoardNum].gnCodeRate = lValue;
            }
			//2010/12/15 ISDB-S ==============================================================
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
			{
				if(strcmp(str2, "1/2") == 0)
					lValue = CONST_ISDBS_CODE_1_2;
				else if(strcmp(str2, "2/3") == 0)
					lValue = CONST_ISDBS_CODE_2_3;
				else if(strcmp(str2, "3/4") == 0)
					lValue = CONST_ISDBS_CODE_3_4;
				else if(strcmp(str2, "5/6") == 0)
					lValue = CONST_ISDBS_CODE_5_6;
				else if(strcmp(str2, "7/8") == 0)
					lValue = CONST_ISDBS_CODE_7_8;
				else
					lValue = 0;
				gpConfig->gBC[nBoardNum].gnCodeRate = lValue;
			}
			//================================================================================
        } else if (strcmp(str,"-const") == 0)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
            {
                if (strcmp(str2,"QPSK") == 0)
                    lValue = CONST_QPSK;
                else if (strcmp(str2,"16QAM") == 0)
                    lValue = CONST_16QAM;
                else if (strcmp(str2,"64QAM") == 0)
                    lValue = CONST_64QAM;
                else
                    lValue = CONST_QPSK;

                gpConfig->gBC[nBoardNum].gnConstellation = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
            {
                if (strcmp(str2,"16QAM") == 0)
                    lValue = CONST_QAM_A_16;
                else if (strcmp(str2,"32QAM") == 0)
                    lValue = CONST_QAM_A_32;
                else if (strcmp(str2,"64QAM") == 0)
                    lValue = CONST_QAM_A_64;
                else if (strcmp(str2,"128QAM") == 0)
                    lValue = CONST_QAM_A_128;
                else if (strcmp(str2,"256QAM") == 0)
                    lValue = CONST_QAM_A_256;
                else
                    lValue = CONST_QAM_A_16;
                gpConfig->gBC[nBoardNum].gnQAMMode = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
            {
                if (strcmp(str2,"64QAM") == 0)
                    lValue = CONST_QAM_B_64;
                else if (strcmp(str2,"256QAM") == 0)
                    lValue = CONST_QAM_B_256;
                else
                    lValue = CONST_QAM_B_64;
                gpConfig->gBC[nBoardNum].gnQAMMode = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
            {
                if (strcmp(str2,"QPSK") == 0)
                    lValue = CONST_DVB_S2_QPSK;
                else if (strcmp(str2,"8PSK") == 0)
                    lValue = CONST_DVB_S2_8PSK;
                else if (strcmp(str2,"16APSK") == 0)
                    lValue = CONST_DVB_S2_16APSK;
                else if (strcmp(str2,"32APSK") == 0)
                    lValue = CONST_DVB_S2_32APSK;
                else
                    lValue = CONST_DVB_S2_QPSK;
                gpConfig->gBC[nBoardNum].gnConstellation = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
            {
                if (strcmp(str2,"4QAM-NR") == 0)
                    lValue = CONST_DTMB_4QAM_NR;
                else if (strcmp(str2,"4QAM") == 0)
                    lValue = CONST_DTMB_4QAM;
                else if (strcmp(str2,"16QAM") == 0)
                    lValue = CONST_DTMB_16QAM;
                else if (strcmp(str2,"32QAM") == 0)
                    lValue = CONST_DTMB_32QAM;
                else if (strcmp(str2,"64QAM") == 0)
                    lValue = CONST_DTMB_64QAM;
                else
                    lValue = CONST_DTMB_4QAM_NR;
                gpConfig->gBC[nBoardNum].gnConstellation = lValue;
            }
			//2010/12/15 ISDB-S ==========================================================================
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
			{
				if(strcmp(str2, "BPSK") == 0)
					lValue = CONST_ISDBS_BPSK;
				else if(strcmp(str2, "QPSK") == 0)
					lValue = CONST_ISDBS_QPSK;
				else
					lValue = CONST_ISDBS_TC8PSK;
				gpConfig->gBC[nBoardNum].gnConstellation = lValue;
			}
			//============================================================================================
        } else if (strcmp(str,"-bw") == 0)
        {
            if (strcmp(str2,"1.7MHz") == 0)
                lValue = DVBT2_BW_1Dot7MHZ;
            if (strcmp(str2,"6MHz") == 0)
			{
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
					lValue = DVBH_BW_6MHZ;
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					lValue = DVBT2_BW_6MHZ;
				else
					lValue = BW_6MHZ;
			}
            else if (strcmp(str2,"7MHz") == 0)
			{
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
					lValue = DVBH_BW_7MHZ;
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					lValue = DVBT2_BW_7MHZ;
				else
					lValue = BW_7MHZ;
			}
            else if (strcmp(str2,"8MHz") == 0)
			{
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
					lValue = DVBH_BW_8MHZ;
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					lValue = DVBT2_BW_8MHZ;
				else
					lValue = BW_8MHZ;
			}
            else if (strcmp(str2,"5MHz") == 0)
			{
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
					lValue = DVBH_BW_5MHZ;
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					lValue = DVBT2_BW_5MHZ;
			}
            else
			{
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
					lValue = DVBH_BW_8MHZ;
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					lValue = DVBT2_BW_8MHZ;
				else
					lValue = BW_8MHZ;
			}
            gpConfig->gBC[nBoardNum].gnBandwidth = lValue;
        } else if (strcmp(str,"-tx") == 0)
        {
            if (strcmp(str2,"2K") == 0)
                lValue = TX_2K;
            else if (strcmp(str2,"8K") == 0)
                lValue = TX_8K;
            else if (strcmp(str2,"4K") == 0)
                lValue = TX_4K;
            else
                lValue = TX_2K;
            gpConfig->gBC[nBoardNum].gnTxmode = lValue;
        } else if (strcmp(str,"-guard") == 0)
        {
            if (strcmp(str2,"1/4") == 0)
                lValue = GI_1_OF_4;
            else if (strcmp(str2,"1/8") == 0)
                lValue = GI_1_OF_8;
            else if (strcmp(str2,"1/16") == 0)
                lValue = GI_1_OF_16;
            else if (strcmp(str2,"1/32") == 0)
                lValue = GI_1_OF_32;
            else
                lValue = GI_1_OF_4;
            gpConfig->gBC[nBoardNum].gnGuardInterval = lValue;
        } else if (strcmp(str,"-interleaving") == 0)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
            {
                if (strcmp(str2,"128-1") == 0)
                    lValue = INTERLEAVE_128_1_;
                else if (strcmp(str2,"64-2") == 0)
                    lValue = INTERLEAVE_64_2;
                else if (strcmp(str2,"32-4") == 0)
                    lValue = INTERLEAVE_32_4;
                else if (strcmp(str2,"16-8") == 0)
                    lValue = INTERLEAVE_16_8;
                else if (strcmp(str2,"8-16") == 0)
                    lValue = INTERLEAVE_8_16;
                else if (strcmp(str2,"128-1") == 0)
                    lValue = INTERLEAVE_128_1;
                else if (strcmp(str2,"128-2") == 0)
                    lValue = INTERLEAVE_128_2;
                else if (strcmp(str2,"128-3") == 0)
                    lValue = INTERLEAVE_128_3;
                else if (strcmp(str2,"128-4") == 0)
                    lValue = INTERLEAVE_128_4;
                else if (strcmp(str2,"128-5") == 0)
                    lValue = INTERLEAVE_128_5;
                else if (strcmp(str2,"128-6") == 0)
                    lValue = INTERLEAVE_128_6;
                else if (strcmp(str2,"128-7") == 0)
                    lValue = INTERLEAVE_128_7;
                else if (strcmp(str2,"128-8") == 0)
                    lValue = INTERLEAVE_128_8;
                else
                    lValue = INTERLEAVE_128_1_;
                gpConfig->gBC[nBoardNum].gnQAMInterleave = lValue;
            } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
            {
                if (strcmp(str2,"52/240") == 0)
                    lValue = CONST_DTMB_INTERLEAVE_0;
                else if (strcmp(str2,"52/720") == 0)
                    lValue = CONST_DTMB_INTERLEAVE_1;
                else
                    lValue = CONST_DTMB_INTERLEAVE_0;
                gpConfig->gBC[nBoardNum].gnQAMInterleave = lValue;
            } else
            {
                gpConfig->gBC[nBoardNum].gnQAMInterleave = 0;
            }
        } else if (strcmp(str,"-frame-header") == 0)
        {
            if (strcmp(str2,"PN420") == 0)
                lValue = CONST_DTMB_FRAME_HEADER_MODE_1;
            else if (strcmp(str2,"PN595") == 0)
                lValue = CONST_DTMB_FRAME_HEADER_MODE_2;
            else if (strcmp(str2,"PN945") == 0)
                lValue = CONST_DTMB_FRAME_HEADER_MODE_3;
            else
                lValue = CONST_DTMB_FRAME_HEADER_MODE_1;
            gpConfig->gBC[nBoardNum].gnFrameHeader = lValue;
        } else if (strcmp(str,"-carrier-number") == 0)
        {
            if (strcmp(str2,"1(ADTB-T)") == 0)
                lValue = CONST_DTMB_CARRIER_NUMBER_0;
            else if (strcmp(str2,"3780(DTMB)") == 0)
                lValue = CONST_DTMB_CARRIER_NUMBER_1;
            else
                lValue = CONST_DTMB_CARRIER_NUMBER_0;
            gpConfig->gBC[nBoardNum].gnCarrierNumber = lValue;
        } else if (strcmp(str,"-frame-header-pn") == 0)
        {
            if (strcmp(str2,"fixed") == 0)
                lValue = CONST_DTMB_FRAME_HEADER_PN_FIXED;
            else
                lValue = CONST_DTMB_FRAME_HEADER_PN_ROTATED;
            gpConfig->gBC[nBoardNum].gnFrameHeaderPN = lValue;
        } else if (strcmp(str,"-pilot-insert") == 0)
        {
            if (strcmp(str2,"off") == 0)
                lValue = CONST_DTMB_PILOT_INSERTION_OFF;
            else
                lValue = CONST_DTMB_PILOT_INSERTION_ON;
            gpConfig->gBC[nBoardNum].gnPilotInsertion = lValue;
        } else if (strcmp(str,"-spectrum") == 0)
        {
            if (strcmp(str2,"inverse") == 0)
                lValue = SPECTRUM_INVERSE;
            else
                lValue = SPECTRUM_NORMAL;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = lValue;
        } else if (strcmp(str,"-noise-mode") == 0)
        {
            if (strcmp(str2,"none") == 0)
                lValue = PRBS_MODE_NONE;
            else if (strcmp(str2,"7") == 0)
                lValue = PRBS_MODE_2_EXP_7_1;
            else if (strcmp(str2,"10") == 0)
                lValue = PRBS_MODE_2_EXP_10_1;
            else if (strcmp(str2,"15") == 0)
                lValue = PRBS_MODE_2_EXP_15_1;
            else if (strcmp(str2,"23") == 0)
                lValue = PRBS_MODE_2_EXP_23_1;
            else
                lValue = PRBS_MODE_NONE;
            gpConfig->gBC[nBoardNum].gnPRBSmode = lValue;
        }
		//2010/12/15 mod. CNR
		else if(strcmp(str, "-cnr") == 0)
        {
            gpConfig->gBC[nBoardNum].gnPRBSscale = (float) atoi(str2);
        } else if (strcmp(str,"-atten") == 0)
        {
#ifdef WIN32
			if (gUtilInd.IsNumericTel(str2) == true)
			{
				String ^str_temp;
				gBaseUtil.ChangeStringFromChar(&str_temp, str2);
				gpConfig->gBC[nBoardNum].gdwAttenVal = DOUBLE::Parse(str_temp);
			}
#else
            gUtility.MySscanfOneFloat(str2,(char *) "%f", (float *)&gpConfig->gBC[nBoardNum].gdwAttenVal);
#endif
        } else if (strcmp(str,"-ext-atten") == 0)
        {
#ifdef WIN32
			if (gUtilInd.IsNumericTel(str2) == true)
			{
				String ^str_temp2;
				gBaseUtil.ChangeStringFromChar(&str_temp2, str2);
				gpConfig->gBC[nBoardNum].gdwAttenExtVal = DOUBLE::Parse(str_temp2);
			}
#else
            gUtility.MySscanfOneFloat(str2,(char *) "%f", (float *)&gpConfig->gBC[nBoardNum].gdwAttenExtVal);
#endif
        } else if (strcmp(str,"-input-source") == 0)
        {
            if (strcmp(str2,"FILE") == 0)
                lValue = FILE_SRC;
            else if (strcmp(str2,"SMPTE-310M") == 0)
                lValue = SMPTE310M_SRC;
            else if (strcmp(str2,"DVB-ASI") == 0)
                lValue = DVBASI_SRC;
            else
                lValue = FILE_SRC;
            gpConfig->gBC[nBoardNum].gnModulatorSource = lValue;
        } else if (strcmp(str,"-mpe-fec") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnMPE_FEC = lValue;
        } else if (strcmp(str,"-time-slice") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnTime_Slice = lValue;
        } else if (strcmp(str,"-in-depth") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnIn_Depth = lValue;
        } else if (strcmp(str,"-cell-id") == 0)
        {
			gpConfig->gBC[nBoardNum].gnCell_Id = atoi(str2);
        } else if (strcmp(str,"-pilot") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnPilot = lValue;
        } else if (strcmp(str,"-roll-off") == 0)
        {
            if (strcmp(str2,"0.20") == 0)
                lValue = ROLL_OFF_FACTOR_020;
            else if (strcmp(str2,"0.25") == 0)
                lValue = ROLL_OFF_FACTOR_025;
            else if (strcmp(str2,"0.35") == 0)
                lValue = ROLL_OFF_FACTOR_035;
            else if (strcmp(str2,"none") == 0)
                lValue = ROLL_OFF_FACTOR_NONE;
            else
                lValue = ROLL_OFF_FACTOR_020;
            gpConfig->gBC[nBoardNum].gnRollOffFactor = lValue;
        } else if (strcmp(str,"-max-playrate") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnOutputClockSource = lValue;
            if (lValue == 1)
            {
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = gUtilInd.CalcBurtBitrate(nBoardNum);
            }
        } else if (strcmp(str,"-null-tp") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnStopMode = lValue;
        } else if (strcmp(str,"-restamping") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnRestamping = lValue;
        } else if (strcmp(str,"-continuity") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnContinuity = lValue;
        } else if (strcmp(str,"-timeoffset") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else if (strcmp(str2,"on(current time)") == 0)
                lValue = 2;
			else if(strcmp(str2,"on(user time)") == 0)
				lValue = 3;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnDateTimeOffset = lValue;
		}else if(strcmp(str,"-user-date") == 0)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, 20, str2);
		}else if(strcmp(str,"-user-time") == 0)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time, 20, str2);
        } else if (strcmp(str,"-reset-playrate") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnCalcPlayRate = lValue;
        } else if (strcmp(str,"-av-decoding") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnUseAVDecoding = lValue;
        } else if (strcmp(str,"-ip-streaming") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnUseIPStreaming = lValue;
        } else if (strcmp(str,"-bypass-amp") == 0)
        {
            if (strcmp(str2,"on") == 0)
                lValue = 1;
            else
                lValue = 0;
            gpConfig->gBC[nBoardNum].gnBypassAMP = lValue;
        } else if (strcmp(str,"-exit-time") == 0)
        {
            long nCurrentInSec, nExitTimeInSec;
            
            if (strlen(str2) == 8)      //00:00:00
            {
                nExitTimeInSec = gUtilInd.Get_Sec_From_HHMMSS(str2);
                tCur = time(NULL);
#if defined(WIN32)
                localtime_s(&nt, &tCur);
#else
				localtime_r(&tCur, &nt);
#endif
                nCurrentInSec = nt.tm_hour*3600 + nt.tm_min*60+nt.tm_sec;
                if (nCurrentInSec >= nExitTimeInSec)
                {
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdAutoExitTime, 24, "");
                    gpConfig->gBC[nBoardNum].gnCmdAutoExitDone = 0;
                } else
                {
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdAutoExitTime, 24, str2);
                    gpConfig->gBC[nBoardNum].gnCmdAutoExitDone = 1;
                }
            } else
            {
                gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdAutoExitTime, 24, (char *) "");
                gpConfig->gBC[nBoardNum].gnCmdAutoExitDone = 0;
            }
        } else if (strcmp(str,"-exit-date") == 0)
        {
            long nExitMonth, nExitDay, nExitYear;
            long nCurrentMonth, nCurrentDay, nCurrentYear;

            tCur = time(NULL);
#if defined(WIN32)
			localtime_s(&nt, &tCur);
#else
			localtime_r(&tCur, &nt);
#endif
            nCurrentMonth = nt.tm_mon+1;
            nCurrentDay = nt.tm_mday;
            nCurrentYear = nt.tm_year + 1900;

            if (strlen(str2) == 10) //mm/dd/yyyy
            {
                gUtilInd.Get_MDY_From_MMDDYYYY(str2, &nExitMonth, &nExitDay, &nExitYear);
                gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnCmdAutoExitDate, 24, "%02d/%02d/%04d", nExitMonth, nExitDay, nExitYear);

                if (nExitYear <= nCurrentYear)
                {
                    if (nExitMonth <= nCurrentMonth)
                        if (nExitDay <= nCurrentDay)
                            gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnCmdAutoExitDate, 24, "%02d/%02d/%04d", nt.tm_mon+1, nt.tm_mday, nt.tm_year+1900);
                }

            } else
            {
                gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnCmdAutoExitDate, 24, "%02d/%02d/%04d", nt.tm_mon+1, nt.tm_mday, nt.tm_year+1900);
            }
        } else if (strcmp(str,"-duration") == 0)
        {
            lValue = atoi(str2);
            if (lValue < 1)
                gpConfig->gBC[nBoardNum].gnCmdDurationTime = 60;
            else
                gpConfig->gBC[nBoardNum].gnCmdDurationTime = lValue*60;

            tCur = time(NULL);
            tCur = tCur + gpConfig->gBC[nBoardNum].gnCmdDurationTime;
#if defined(WIN32)
            localtime_s(&nt, &tCur);
#else
			localtime_r(&tCur, &nt);
#endif

            gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnCmdAutoExitDate, 24, "%02d/%02d/%04d", nt.tm_mon+1, nt.tm_mday, nt.tm_year+1900);
            gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnCmdAutoExitTime,  24, "%02d:%02d:%02d", nt.tm_hour, nt.tm_min, nt.tm_sec);
            gpConfig->gBC[nBoardNum].gnCmdAutoExitDone = 1;
        }
		//2010/12/15 CommandLine =====================================================================================================================
		// mod. tmcc
		else if(strcmp(str, "-use-tmcc") == 0)
		{
			if(strcmp(str2, "on") == 0)
				gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
			else
				gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
		}
		// mod. multi-ts combiner for ISDB-S
		else if(strcmp(str, "-multi-ts") == 0)
		{
			if(strcmp(str2, "on") == 0)
				gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
			else
				gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
		}
		// mod. emergency-alarm broadcasting
		else if(strcmp(str, "-emergency-alarm") == 0)
		{
			if(strcmp(str2, "on") == 0)
				gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting = 1;
			else
				gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting = 0;
		}
		else if(strcmp(str, "-sub-loop-start-time") == 0)
		{
			double dwOffset;
			int nTimeOffset = gUtilInd.Get_Sec_From_HHMMSS(str2);
			dwOffset = (double)nTimeOffset * (double)gpConfig->gBC[nBoardNum].gdwPlayRate / 8.0;
			gpConfig->gBC[nBoardNum].gnStartTimeOffset = nTimeOffset;
			gpConfig->gBC[nBoardNum].gnStartPos = (__int64)dwOffset;
			
#ifdef WIN32
#if 0 //2013/2/13 todo delete after finishing new design
			FormCollection^ forms1;
			forms1 = Application::OpenForms;
	
			for(int i=0; i < forms1->Count; i++)
			{
				if(forms1[i]->Name == "PlayForm")
				{
					safe_cast<PlayForm^>(forms1[i])->LabTimer->Text = gcnew String(str2);
					break;
				}
			}
#endif
#endif
		}
		else if(strcmp(str, "-sub-loop-end-time") == 0)
		{
			double dwOffset2;
			int nTimeOffset2 = gUtilInd.Get_Sec_From_HHMMSS(str2);
			dwOffset2 = (double)nTimeOffset2 * (double)gpConfig->gBC[nBoardNum].gdwPlayRate / 8.0;
			gpConfig->gBC[nBoardNum].gnEndTimeOffset = nTimeOffset2;
			gpConfig->gBC[nBoardNum].gnEndPos = (__int64)dwOffset2;
			
#ifdef WIN32
#if 0 //2013/2/13 todo delete after finishing new design
			FormCollection^ forms1;
			forms1 = Application::OpenForms;
	
			for(int i=0; i < forms1->Count; i++)
			{
				if(forms1[i]->Name == "PlayForm")
				{
					safe_cast<PlayForm^>(forms1[i])->LabTimer->Text = safe_cast<PlayForm^>(forms1[i])->LabTimer->Text + "~" + gcnew String(str2);
					break;
				}
			}
#endif
#endif
			gpConfig->gBC[nBoardNum].gnCurrentPos = gpConfig->gBC[nBoardNum].gnStartPos;
			gpConfig->gBC[nBoardNum].gnStartPosChanged = 1;
			gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 1;
#ifdef WIN32
			gWrapDll.TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_START, (double)gpConfig->gBC[nBoardNum].gnStartPos);
			gWrapDll.TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double)gpConfig->gBC[nBoardNum].gnCurrentPos);
#else
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_START, (double)gpConfig->gBC[nBoardNum].gnStartPos);
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double)gpConfig->gBC[nBoardNum].gnCurrentPos);
#endif
			gpConfig->gBC[nBoardNum].gnEndPosChanged = 1;
#ifdef WIN32
			gWrapDll.TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_END, (double)gpConfig->gBC[nBoardNum].gnEndPos);
#else
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_END, (double)gpConfig->gBC[nBoardNum].gnEndPos);
#endif
			gpConfig->gBC[nBoardNum].gnUseSubLoop = 1;
		}
		//Error Injection
		else if(strcmp(str, "-error-lose-packet") == 0)
		{
			lValue = atoi(str2);
			gpConfig->gBC[nBoardNum].gnErrLostPacket = lValue;
			gpConfig->gBC[nBoardNum].gnErrLost = 1;
		}
		else if(strcmp(str, "-error-bits-packet") == 0)
		{
			lValue = atoi(str2);
			gpConfig->gBC[nBoardNum].gnErrBitsPacket = lValue;
			gpConfig->gBC[nBoardNum].gnErrBits = 1;
		}
		else if(strcmp(str, "-error-bits-count") == 0)
		{
			lValue = atoi(str2);
			gpConfig->gBC[nBoardNum].gnErrBitsCount = lValue;
			gpConfig->gBC[nBoardNum].gnErrBits = 1;
		}
		else if(strcmp(str, "-error-bytes-packet") == 0)
		{
			lValue = atoi(str2);
			gpConfig->gBC[nBoardNum].gnErrBytesPacket = lValue;
			gpConfig->gBC[nBoardNum].gnErrBytes = 1;
		}
		else if(strcmp(str, "-error-bytes-count") == 0)
		{
			lValue = atoi(str2);
			gpConfig->gBC[nBoardNum].gnErrBytesCount = lValue;
			gpConfig->gBC[nBoardNum].gnErrBytes = 1;
		}
		//============================================================================================================================================
		//2010/6/10
		else if (strcmp(str,"-l1-mod") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-guard") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_GUARD = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-fft") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_FFT = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-plp-mod") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[0] = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-plp-cod") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[0] = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-plp-fec-type") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[0] = atoi(str2);
        }
		else if (strcmp(str,"-t2mi_plp_num_block") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[0] = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-num-data-symbol") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-pilot-pattern") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-bw") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_BW = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-bwt") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_BWT = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-hem") == 0)
        {
			gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0] = atoi(str2);
        }
		else if (strcmp(str,"-t2mi-adjustment-flag") == 0)
        {
			gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = atoi(str2);
        }
		//2010/12/14 CommandLine Added ============================================================
		else if( strcmp(str, "-use-lose-packets") == 0)
		{
			if(strcmp(str2,"on") == 0)
				gpConfig->gBC[nBoardNum].gnErrLost = 1;
			else
				gpConfig->gBC[nBoardNum].gnErrLost = 0;
		}
		else if(strcmp(str, "-lose-packets") == 0)
		{
			gpConfig->gBC[nBoardNum].gnErrLostPacket = atoi(str2);
		}
		else if(strcmp(str, "-use-bits-error") == 0)
		{
			if(strcmp(str2,"on") == 0)
				gpConfig->gBC[nBoardNum].gnErrBits = 1;
			else
				gpConfig->gBC[nBoardNum].gnErrBits = 0;
		}
		else if(strcmp(str, "bits-error-packets") == 0)
		{
			gpConfig->gBC[nBoardNum].gnErrBitsPacket = atoi(str2);
		}
		//==========================================================================================
        i += 2;
    }

    CheckGlobalVarValidity(nBoardNum);

    //-------------------------------------------------------
    //--- Check ParamStr, ParamCount

    //-----------------------------------
    gUtility.MyStrCpy(strMsg,2048, "");
    for (i = 1; i < iNumParam;i++)
    {
#if defined(WIN32)
		gBaseUtil.ChangeCharFromString(arguments[i], str2);
#else
		gUtility.MyStrCat(strMsg, 2048, gUtility.Get_Main_Parameter(i));
#endif
		gUtility.MyStrCat(strMsg, 2048, str2);
	}


}

//---------------------------------------------------------------------------
// Read Startup Modulation Type and IF Freq for the calling of DllMain()
// Linux Only
#ifndef WIN32
int CREG_VAR::Get_StartUp_Value_From_Registry(int iType, int nSlot)
{
	FILE	*pFileStart = NULL;
	FILE	*Reg;
    char    strKey[256];
	int		iRet = 0;
#ifdef STANDALONE
	gUtility.MySprintf(strKey, 256, (char *) "/sysdb/startupMod_%d.cfg", nSlot);	//2010/9/28 MULTIBOARD
#else
	gUtility.MySprintf(strKey, 256, (char *) "./config/startupMod_%d.cfg", nSlot);	//2010/9/28 MULTIBOARD
#endif
	pFileStart = fopen(strKey, "r");
	Reg = pFileStart;

	if (iType == 0)
		iRet = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380ModulationMode", 0);
		
	if (pFileStart)
		fclose(pFileStart);


	return iRet;
}

#endif

#ifdef WIN32
//2012/10/22 system configuration
void CREG_VAR::GetSystemConfiguration()
{
	char strKey[256];
	RegistryKey^ Reg;
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\TeleviewSystem\\COMMON_CONFIGRATION");
	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));
	gGeneral.gnOverFrequency = 0;
	if(!Reg)
	{
		return;
	}

	gGeneral.gnOverFrequency = gUtility.Get_Registry_Integer_Value(Reg, strKey, "OverFrequency_Enabled", 0);
	gGeneral.gnRetentionPeriod = gUtility.Get_Registry_Integer_Value(Reg, strKey, "Retention_Period", 0);
	if(gGeneral.gnRetentionPeriod < 0 || gGeneral.gnRetentionPeriod > 2)
		gGeneral.gnRetentionPeriod = 0;
	Reg->Close();
	delete Reg;
}
void CREG_VAR::SetSystemConfiguration()
{
	char strKey[256];
	RegistryKey^ Reg;
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\TeleviewSystem\\COMMON_CONFIGRATION");

	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));

	if(!Reg)
	{
		return;
	}
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "OverFrequency_Enabled", gGeneral.gnOverFrequency);
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "Retention_Period", gGeneral.gnRetentionPeriod);
	Reg->Close();
}

void CREG_VAR::GetInitSystemValue(int nBoardNum, char *BoardLocation, int nBoard_ID)
{
	char strKey[256];
	RegistryKey^ Reg;
    char    str1[256];
	int nDefaultValue = 1;
	gUtility.MySprintf(str1, 256, "SYS_%s", BoardLocation);
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\TeleviewSystem\\%s", str1);

	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));

	if(!Reg)
	{
		return;
	}

	if(nBoard_ID == 0xB || nBoard_ID == 0xF)
		nDefaultValue = ASI_OUTPUT_MODE;

	gpConfig->gnInstalledBoard_InitMode[nBoardNum] = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ModulationType", nDefaultValue);
	Reg->Close();
	delete Reg;
}

#endif 

//---------------------------------------------------------------------------
// called from
//      Form_Load()
//      Combo_ADAPTOR_Click()
//      Combo_MODULATOR_TYPE_Click()
// Registry information
//      HKEY_CURRENT_USER/Software/TELEVIEW
//          TPG0590_X
//              16VSB, DTMB, DVB-H, DVB-S2, DVB-T,
//              ISDB-T, ISDB-T-13, QAM-A, QAM-B, QPSK, TDMB, VSB
//              StartUp

#ifdef WIN32 
void CREG_VAR::RestoreVariables(long nBoardNum)
{
    char    strKey[256];
    char    str1[256];
    float   fValue;
	char	temp[100];


    //---------------------------------------------
    // Crate Registry and set root key
#ifdef _MS_VC
	RegistryKey^ Reg;
#else
    TRegistry *Registry = new TRegistry(KEY_READ);
    Registry->RootKey = HKEY_CURRENT_USER;
#endif

	char szSN[40];
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gUtility.MyStrCpy(szSN, 40, gWrapDll.Get_AdaptorInfo(gpConfig->gBC[nBoardNum].gn_OwnerSlot,0));
		gUtility.MySprintf(str1, 256, "TPG0590_%s_%d", szSN, gpConfig->gBC[nBoardNum].gn_StreamNum);
	}
	else
	{
		gUtility.MyStrCpy(szSN, 40, gWrapDll.Get_AdaptorInfo(nBoardNum,0));
		gUtility.MySprintf(str1, 256, "TPG0590_%s", szSN);
	}
    //---------------------------------------------
    // decide slot number
    //---------------------------------------------
    // Get Modulation Mode from "StartUp"
	
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\%s\\StartUp", str1);

#ifdef _MS_VC
	unsigned long reg_temp1 = 0, reg_temp2 = 0;
			Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));
	if(!Reg)
	{
		return;
	}
#endif
	char AppVer[16];
    gUtility.MyStrCpy(AppVer, 16, gUtility.Get_Registry_String_Value(Reg, strKey,"ApplicationVersion", "V0.0.0"));

    gpConfig->gBC[nBoardNum].gnSubBankNumber = gUtility.Get_Registry_Integer_Value(Reg, strKey, "SubBankNum", 7);
    gpConfig->gBC[nBoardNum].gnSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, "SubBankOffset", 1024);

    gpConfig->gBC[nBoardNum].gnUseFrontInput = gUtility.Get_Registry_Integer_Value(Reg, strKey, "FrontInputEnabled", 0);
    gpConfig->gBC[nBoardNum].gnUseDemuxblockTest = gUtility.Get_Registry_Integer_Value(Reg, strKey, "DemuxBlockTestEnabled", 0);
    
    // sskim20070528 - keisoku
    gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = gUtility.Get_Registry_Integer_Value(Reg, strKey, "RFOutFreqUnit", 2);          // MHz
	//2011/7/13 fixed
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
		gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = 2;
	}

	//2012/8/17
	gpConfig->gBC[nBoardNum].gnRfLevel_Unit = gUtility.Get_Registry_Integer_Value(Reg, strKey, "RFLevelUnit", 0);          //dBm

    gpConfig->gBC[nBoardNum].gnRemoveFileEnabled = gUtility.Get_Registry_Integer_Value(Reg, strKey, "RemoveFileEnabled", 0);
    gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TDMBSubBankOffset", 128);

	//2011/6/29 TAT4710
	gpConfig->gBC[nBoardNum].gnUseTAT4710 = gUtility.Get_Registry_Integer_Value(Reg, strKey, "USETAT4710", -1);
	//2014/3/25 EEPROM USE TAT4710
	if(gpConfig->gBC[nBoardNum].gnBoardId == 12 && gpConfig->gBC[nBoardNum].gnUseTAT4710 == -1)
	{
		unsigned long _address, _data, _checksum, _result;
		_address = 0x500;
		_data = gWrapDll.TSPL_READ_CONTROL_REG_EX(nBoardNum, 3, _address);
		if(_data == 0x4710)
		{
			_checksum = _data;
			_address = 0x502;
			_data = gWrapDll.TSPL_READ_CONTROL_REG_EX(nBoardNum, 3, _address);
			_result = _data;
			_checksum = _checksum + _data;
			_address = 0x504;
			_data = gWrapDll.TSPL_READ_CONTROL_REG_EX(nBoardNum, 3, _address);
			_address = 0x506;
			_data = (_data << 16) + gWrapDll.TSPL_READ_CONTROL_REG_EX(nBoardNum, 3, _address);
			if(_data == _checksum)
			{
				gpConfig->gBC[nBoardNum].gnUseTAT4710 = _result;
			}
		}
	}

	if(gpConfig->gBC[nBoardNum].gnUseTAT4710 == -1)
		gpConfig->gBC[nBoardNum].gnUseTAT4710 = 0;

    //--- multilang
    gGeneral.gMultiLang = gUtility.Get_Registry_Integer_Value(Reg, strKey, "EnglishVersionOnly", 1);
	if((gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xB && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE_V4 ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET_V3_%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET_V3_%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
			
			if(gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			if(gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
		
			//kslee 2010/5/18 
			//if(strcmp(gGeneral.gszAppVer, AppVer) != 0)
			//{
			//	gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			//	gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
			//}
		}
	}
	else
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
			
			if(gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			if(gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
		
			////kslee 2010/5/18 
			//if(strcmp(gGeneral.gszAppVer, AppVer) != 0)
			//{
			//	gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			//	gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
			//}
		}
	}
	//2012/2/20 FilePlay Debug Mode
	gpConfig->gBC[nBoardNum].gnDebugMode_FilePlay = gUtility.Get_Registry_Integer_Value(Reg, strKey, "DebugMode", 0);


    //----------------------------------------------------------------------------
    // SNMP
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszSnmpRemoteHost, 24, gUtility.Get_Registry_String_Value(Reg, strKey,"RemoteHost", "127.0.0.1"));
    gpConfig->gBC[nBoardNum].gnSnmpLocalPort = gUtility.Get_Registry_Integer_Value(Reg, strKey, "LocalPort", 8001);
    gpConfig->gBC[nBoardNum].gnSnmpRemotePort = gUtility.Get_Registry_Integer_Value(Reg, strKey, "RemotePort", 8240);

	//kslee 2010/3/18
    gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IP_Rx_Bitrate_Control", 0);
	//kslee 2010/4/5
	if(gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control >= 0)
		gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control = 100;		//2010/4/5 97 => 100

    //---------------------------------------------
    // Set strKey: HKEY_CURRENT_USER/Software/TELEVIEW/TPG0590_x/ModType
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\%s\\%s", str1, gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode]);
#ifdef _MS_VC
	Reg->Close();
	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));
	if(!Reg)
	{

		return;
	}
#endif

    //---------------------------------------------
    //--- for FileList/PlayList
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory,  256, gUtility.Get_Registry_String_Value(Reg, strKey,"tpgts", "C:\\TS"));
#ifdef WIN32
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszSingleFileName,  256, gUtility.Get_Registry_String_Value(Reg, strKey,"SingleFileName", ""));

	//List(20)
	for(int i_list = 0; i_list <  MAX_PLAY_LIST_COUNT; i_list++)
	{
		char regTmpName[128];
		gUtility.MySprintf(regTmpName, 128, "ListPath_%d", i_list);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szPlayFileList[i_list],  256, gUtility.Get_Registry_String_Value(Reg, strKey, regTmpName, ""));
		gUtility.MySprintf(regTmpName, 128, "ListFileName_%d", i_list);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szPlayListFileName[i_list],  256, gUtility.Get_Registry_String_Value(Reg, strKey, regTmpName, ""));
		gUtility.MySprintf(regTmpName, 128, "ListPlaybackRate_%d", i_list);
	    gpConfig->gBC[nBoardNum].gnPlayListPlaybackRate[i_list] = gUtility.Get_Registry_Integer_Value(Reg, strKey, regTmpName, 19392658);
		gUtility.MySprintf(regTmpName, 128, "ListAtscMhFormat_%d", i_list);
	    gpConfig->gBC[nBoardNum].gnPlayListAtscMh_Format[i_list] = gUtility.Get_Registry_Integer_Value(Reg, strKey, regTmpName, 0);
	}
#endif
    //---------------------------------------------
    //--- general, restamping
	gUtility.MyStrCpy(temp, 100, gUtility.Get_Registry_String_Value(Reg, strKey, "RepeatMode", "False"));
	if (strcmp(temp, "False") == 0)
		gpConfig->gBC[nBoardNum].gbRepeatMode = false;
	else
		gpConfig->gBC[nBoardNum].gbRepeatMode = true;
    gpConfig->gBC[nBoardNum].gdwPlayRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "PlayRate", 19392658);
    gpConfig->gBC[nBoardNum].gnCalcPlayRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ReCalcPlayRate", 0);
    gpConfig->gBC[nBoardNum].gnStartOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, "StartOffset", 0);
    gpConfig->gBC[nBoardNum].gnStopMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "StopMode", 0);
	//LOOP_ADAPT
    gpConfig->gBC[nBoardNum].gnRestamping = gUtility.Get_Registry_Integer_Value(Reg, strKey, "Restamping", 0);
    gpConfig->gBC[nBoardNum].gnContinuity = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ContinuityCounter", 0);
    gpConfig->gBC[nBoardNum].gnDateTimeOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, "DateTimeOffset", 0);
	//TDT/TOT - USER DATE/TIME
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, 20, gUtility.Get_Registry_String_Value(Reg, strKey, "DateTimeOffset_Date", "1900-01-01"));
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time, 20, gUtility.Get_Registry_String_Value(Reg, strKey, "DateTimeOffset_Time", "12-00-00"));

	//kslee 2010/3/18
	//PCR RESTAMP
	gpConfig->gBC[nBoardNum].gnPCR_Restamping = gUtility.Get_Registry_Integer_Value(Reg, strKey, "PCR_Restamping", 0);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
		gpConfig->gBC[nBoardNum].gnPCR_Restamping = 0;
	}

    //---------------------------------------------
    //--- Modulator
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
        gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, "TVB380RFOutFreq", 1100000000);	//kslee 2010/3/31
    else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
        gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, "TVB380RFOutFreq", 367513000);		//kslee 2010/3/31
	else
        gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, "TVB380RFOutFreq", 473000000);		//kslee 2010/3/31

	//2011/5/4 AD9852 OVER CLOCK
#ifdef WIN32
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		gpConfig->gBC[nBoardNum].gnAD9852_Overclock = gUtility.Get_Registry_Integer_Value(Reg, strKey, "AD9852_OVERCLOCK", 0);
#endif
	int DefaultValue;
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 20000000;
	else
		DefaultValue = 3000000;
    gpConfig->gBC[nBoardNum].gnSymbolRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380SymbolRate", DefaultValue);
    
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
		DefaultValue = 2;
	else
		DefaultValue = 0;
	gpConfig->gBC[nBoardNum].gnCodeRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380CodeRate", DefaultValue);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
		DefaultValue = 3;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
		DefaultValue = 0;
	else
		DefaultValue = 2;

	gpConfig->gBC[nBoardNum].gnBandwidth = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Bandwidth", DefaultValue);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && gUtilInd.IsSupportHMC833(nBoardNum) == 0))
		gpConfig->gBC[nBoardNum].gnBandwidth = 0;

	gpConfig->gBC[nBoardNum].gnTxmode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Txmode", 0);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 3;
	else
		DefaultValue = 0;
	gpConfig->gBC[nBoardNum].gnGuardInterval = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380GuardInterval", DefaultValue);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 2;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 3;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
		DefaultValue = 4;
	else
		DefaultValue = 0;
    gpConfig->gBC[nBoardNum].gnConstellation = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Constellation", DefaultValue);
	
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
		DefaultValue = 1;
	else
		DefaultValue = 0;
    gpConfig->gBC[nBoardNum].gnQAMMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380QAMMode", DefaultValue);

    gpConfig->gBC[nBoardNum].gnQAMInterleave = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380QAMInterleaving", 0);
    gpConfig->gBC[nBoardNum].gnIFOutFreq = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IFOutFreq", 0);
    
	//2010/7/20 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{
		gpConfig->gBC[nBoardNum].gnSpectrumInverse = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380SpectralInversion", 1);
	}
	else
	{
		gpConfig->gBC[nBoardNum].gnSpectrumInverse = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380SpectralInversion", 0);
	}
	//2010/8/31 FIXED - IF +- offset
	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		gpConfig->gBC[nBoardNum].gnCurrentIF = 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		gpConfig->gBC[nBoardNum].gnCurrentIF = 44000000;
	else
		gpConfig->gBC[nBoardNum].gnCurrentIF = 36125000;

	//2010/12/06 ISDB-S =============================================================================================================
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
			gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_2_3;
		else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK)
			gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_1_2;
		gpConfig->gBC[nBoardNum].gnPCR_Restamping = 1;
	}
	//===============================================================================================================================

    gpConfig->gBC[nBoardNum].gnPRBSmode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380PRBSMode", 0);
    gpConfig->gBC[nBoardNum].gnPRBSscale = gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380PRBSScale", 0.0);

	//2011/10/17 added PI
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
	{
		gpConfig->gBC[nBoardNum].gnLNB_Index = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380LNBFreqIndex", 0);
	}
    //---------------------------------------------
    //--- RF/IF Amp :
    gpConfig->gBC[nBoardNum].gnBypassAMP = gUtility.Get_Registry_Integer_Value(Reg, strKey, "BypassAMP", 1);

    if (gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599
    {
        if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 21.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 24.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 17.0;
            else
                fValue = 0.0;
        } else
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 14.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 18.0;
            else
                fValue = 0.0;
        }
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnRFAmpUsed == 1)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 25.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 28.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 14.0;
            else
                fValue = 0.0;
        } else if (gpConfig->gBC[nBoardNum].gnRFAmpUsed == 2)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 25.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 28.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 17.0;
            else
                fValue = 0.0;
        } else
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 18.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 26.0;
            else
                fValue = 0.0;
        }
    }

	//2011/11/29 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		gpConfig->gBC[nBoardNum].gnBypassAMP = 0;
		fValue = 0;
	}
	//2012/9/3 new rf level control
	if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 1)
	{
		fValue = -35;
		gpConfig->gBC[nBoardNum].gdRfLevelValue = (double)gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB59x_RF_LEVEL", fValue);
	}
	else
		gpConfig->gBC[nBoardNum].gdwAttenVal = (double)gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380AttenVal", fValue);

    //---------------------------------------------
    //--- Input Source
    gpConfig->gBC[nBoardNum].gnModulatorSource = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380InputSource", 0);

	//2011/7/6
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY/* || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2*/)
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}

	if(gpConfig->gBC[nBoardNum].gnBoardId == 0x15 && (gpConfig->gBC[nBoardNum].gnModulatorSource == 2 || gpConfig->gBC[nBoardNum].gnModulatorSource == 3))
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}
	//2012/3/27 TVB593 Multiple vsb/qam-b
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1 && 
		(gpConfig->gBC[nBoardNum].gnModulatorSource == 2 || gpConfig->gBC[nBoardNum].gnModulatorSource == 3))
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}

    //---------------------------------------------
    //--- DVB-H
    gpConfig->gBC[nBoardNum].gnMPE_FEC = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380MPE_FEC", 0);
    gpConfig->gBC[nBoardNum].gnTime_Slice = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Time_Slice", 0);
    gpConfig->gBC[nBoardNum].gnIn_Depth = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380In_Depth", 0);
    gpConfig->gBC[nBoardNum].gnCell_Id = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Cell_ID", 0);
	gpConfig->gBC[nBoardNum].gnPilot = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Pilot", 0);

    //sskim20070223 - v5.2.0, 1==NTSC Carrier, RF = USER RF + 1.750MHz
    gpConfig->gBC[nBoardNum].gnFreqPolicy = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380FreqPolicy", 0);
    gpConfig->gBC[nBoardNum].gnRollOffFactor = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380RollOffFactor", 0);

    //----------------------------------------------------------------------------
    //--- A/V decoding, IP Streaming
    gpConfig->gBC[nBoardNum].gnIPStreamingMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingMode", 0);
    gpConfig->gBC[nBoardNum].gnIPStreamingPath = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingPath", 0);
    gpConfig->gBC[nBoardNum].gnIPStreamingAccess = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingAccess", 0);
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnIPStreamingAddress,24,  gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IPStreamingAddress", "192.168.0.1"));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnIPStreamingPort, 10, gUtility.Get_Registry_String_Value(Reg, strKey, "TVB380IPStreamingPort", "1234"));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIPStreamingInfo, sizeof(gpConfig->gBC[nBoardNum].gszIPStreamingInfo), gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IPStreamingInfo", ""));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo, sizeof(gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo), gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IPStreamingInputInfo", ""));
    gpConfig->gBC[nBoardNum].gnUseIPStreaming = gUtility.Get_Registry_Integer_Value(Reg, strKey, "IPStreamingEnabled", 0);
    gpConfig->gBC[nBoardNum].gnIPSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, "IPSubBankOffset", 512);
    gpConfig->gBC[nBoardNum].gnUseAVDecoding = gUtility.Get_Registry_Integer_Value(Reg, strKey, "AVDecodingEnabled", 0);
	
	String	^IP_str;
	int		IP_index;
	IP_str = gcnew String(gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
	IP_index = IP_str->IndexOf("ts-out=");
	if(IP_index > 0)
	{
		IP_str = IP_str->Substring(0, IP_index - 1);
	}
	gBaseUtil.ChangeCharFromString(IP_str, gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
	delete IP_str;
#ifdef WIN32
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
		DefaultValue = REMUX_FILE_IN;
	else
		DefaultValue = FILE_SINGLE_IN;
	//2013/3/18 Input source
	gpConfig->gBC[nBoardNum].gnInputSource = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVBxxx_InputSource", DefaultValue);
	if(gpConfig->gBC[nBoardNum].gnModulatorSource == 0)
	{
		if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
		{
			if(gpConfig->gBC[nBoardNum].gnInputSource > PRBS_DATA_IN)
				gpConfig->gBC[nBoardNum].gnInputSource = FILE_SINGLE_IN;
		}
		else
		{
			gpConfig->gBC[nBoardNum].gnInputSource = IP_IN;
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
	{
		gpConfig->gBC[nBoardNum].gnInputSource = DVB_ASI_IN;
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC)
	{
		gpConfig->gBC[nBoardNum].gnInputSource = SMPTE_310M_IN;
	}
	gpConfig->gBC[nBoardNum].gnChannelNum = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVBxxx_Channel_Number", 0);
#endif
    //----------------------------------------------------------------------------
    //--- No UI Parameters
    gpConfig->gBC[nBoardNum].gnOutputClockSource = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380MaxPlayrate", 0);
	//2010/3/25
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 || gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)	//2010/12/17 ISDB-S
	{
		gpConfig->gBC[nBoardNum].gnOutputClockSource = 1;
	}
#ifdef WIN32
	else
		gpConfig->gBC[nBoardNum].gnOutputClockSource = 0;
#endif
	gpConfig->gBC[nBoardNum].gnNationalCode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380NationalCode", 0);
    gpConfig->gBC[nBoardNum].gnChannelType = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380ChannelType", 0);

    //----------------------------------------------------------------------------
    gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = gUtility.Get_Registry_Integer_Value(Reg, strKey, "UseTMCCRemuxer", 0);
    gpConfig->gBC[nBoardNum].gdwAttenExtVal = gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380AttenExtVal", 0.0);

    //----------------------------------------------------------------------------
    //--- DTMB
    gpConfig->gBC[nBoardNum].gnFrameHeader = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeader", 0);
    gpConfig->gBC[nBoardNum].gnCarrierNumber = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380CarrierNumber", 0);
    gpConfig->gBC[nBoardNum].gnFrameHeaderPN = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeaderPN", 1);
    gpConfig->gBC[nBoardNum].gnPilotInsertion = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380PilotInsertion", 0);

    gpConfig->gBC[nBoardNum].gRFPowerLevelOffset = gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevelOffset", 0);	// 20090812
	//AGC - RF Level -> Atten/AGC
	gpConfig->gBC[nBoardNum].gnAGC = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380AGC", 1);
 	//2011/11/29 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		gpConfig->gBC[nBoardNum].gnAGC = 0;
	}
   
	gpConfig->gBC[nBoardNum].gDAC_Mod_Mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380DAC_MOD_MODE", 0);

	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IP_RxIP", "192.168.0.1"));
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IP_RxMulticastIP", "0.0.0.0"));
	gpConfig->gBC[nBoardNum].gnIP_RxPort = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxPort", 1234);
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,"TVB380IP_RxLocalIP", "0.0.0.0"));
	gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxUseMulticastIP", 0);

	//2010/5/28 DVB-T2
	gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BITRATE_ADJUSTMENT", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_BW = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW", 4);
	gpConfig->gBC[nBoardNum].gnT2MI_BWT = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_FFT = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT", 3);
	gpConfig->gBC[nBoardNum].gnT2MI_GUARD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD", 3);
	gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN", 6);
	gpConfig->gBC[nBoardNum].gnT2MI_MISO = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_PAPR = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID", 12421);
	if(gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID = 12421;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID", 32796);
	if(gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID = 32769;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID", 0);
	if(gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID = 0;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_PID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID", 4096);
	if(gpConfig->gBC[nBoardNum].gnT2MI_PID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_PID > 8191)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_PID = 4096;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME", 2);
	if(gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME < 2)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME = 2;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL", 60);
	char str_regTemp[64];
	gUtility.MyStrCpy(str_regTemp, 64, "");
	for(int ii = 0; ii < MAX_PLP_TS_COUNT; ii++)
	{
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_ID%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, -1);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_TYPE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_TYPE[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_MOD%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 3);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_COD%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 5);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FEC_TYPE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_HEM%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_ROTATION%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_NUM_BLOCK%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 20);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FILEPATH%d", ii);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii], 512,  gUtility.Get_Registry_String_Value(Reg, strKey, str_regTemp, ""));
		if (gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii]) == false)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii], 512, "");
		}

		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_PLAYRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_MAXPLAYRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_MAX_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FILE_BITRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
	}
	//====================================================================================================================================
	
	//2010/9/13 FIXED - ISDB-T Emergency Broadcasting Control
	gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting =	gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380EMERGENCY_BROADCASTING", 0);

	//2010/7/20	I/Q PLAY/CAPTURE
	gpConfig->gBC[nBoardNum].gnIQ_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MODE", 0);
	if (gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		gpConfig->gBC[nBoardNum].gnIQ_mode = 1;
	gpConfig->gBC[nBoardNum].gnIQ_play_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_PLAY_MODE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_capture_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTURE_MODE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_mem_use = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_USE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_mem_size = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_SIZE", 512);
	//2011/11/18 IQ NEW FILE FORMAT
	gpConfig->gBC[nBoardNum].gnIQ_CaptureSize = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTUREDFILE_SIZE", -1);
	gpConfig->gBC[nBoardNum].gnIQ_ErrorCheck = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_FLAG", 1);
	gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_SIZE", -1);
	
	//2010/12/06 ISDB-S===========================================================================================================================
	for(int nIdx = 0 ; nIdx < MAX_TS_COUNT ; nIdx++)
	{
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS%d", nIdx);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[nIdx], 512,  gUtility.Get_Registry_String_Value(Reg, strKey,temp, ""));
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Bitarte%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_Constellation%d", nIdx);
		gpConfig->gBC[nBoardNum].gnConstellation_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_Coderare%d", nIdx);
		gpConfig->gBC[nBoardNum].gnCoderate_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_SlotNum%d", nIdx);
		gpConfig->gBC[nBoardNum].gnSlotCount_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Selected%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_Selected_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
		gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_ID%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_ID_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);


	}
	//============================================================================================================================================

	//2011/2/23 DVB-C2 ===========================================================================================================================
	gpConfig->gBC[nBoardNum].gnDVB_C2_BW = gUtility.Get_Registry_Integer_Value(Reg, strKey,                 "TVB380C2_BANDWIDTH",           284);
	gpConfig->gBC[nBoardNum].gnDVB_C2_L1 = gUtility.Get_Registry_Integer_Value(Reg, strKey,                 "TVB380C2_L1TIMODE",            0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Guard = gUtility.Get_Registry_Integer_Value(Reg, strKey,              "TVB380C2_GUARD_INTERVAL",      0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Network = gUtility.Get_Registry_Integer_Value(Reg, strKey,            "TVB380C2_NETWORK_ID",          0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_System = gUtility.Get_Registry_Integer_Value(Reg, strKey,             "TVB380C2_SYSTEM_ID",           0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq = gUtility.Get_Registry_Integer_Value(Reg, strKey,          "TVB380C2_START_FREQ",          217824);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth = gUtility.Get_Registry_Integer_Value(Reg, strKey,            "TVB380C2_NUM_NOTH",            0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone = gUtility.Get_Registry_Integer_Value(Reg, strKey,       "TVB380C2_RESERVED_TONE",       0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart = gUtility.Get_Registry_Integer_Value(Reg, strKey,         "TVB380C2_NOTCH_START",         0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth = gUtility.Get_Registry_Integer_Value(Reg, strKey,         "TVB380C2_NOTCH_WIDTH",         0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos = gUtility.Get_Registry_Integer_Value(Reg, strKey,     "TVB380C2_DSLICE_TUNEPOS",      142);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_OFFSET_RIGHT", 142);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft = gUtility.Get_Registry_Integer_Value(Reg, strKey,  "TVB380C2_DSLICE_OFFSET_LEFT",  -142);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type = gUtility.Get_Registry_Integer_Value(Reg, strKey,		"TVB380C2_DSLICE_TYPE",          0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader = gUtility.Get_Registry_Integer_Value(Reg, strKey,   "TVB380C2_DSLICE_FECHEADER",     0);

	//2011/5/17 DVB-C2 MULTI-PLP
	for(int tt = 0 ; tt < DVB_C2_MAX_PLP_TS_COUNT ; tt++)
	{
		gUtility.MySprintf(temp, 100, "TVB380C2_PLP_ID%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,        temp,          -1);
		gUtility.MySprintf(temp, 100, "TVB380C2_PLP_MODURATION%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,            temp,      1);
		gUtility.MySprintf(temp, 100, "TVB380C2_PLP_CODERATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,           temp,        4);
		gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FECTYPE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,            temp,         1);
		gUtility.MySprintf(temp, 100, "TVB380C2_PLP_BLOCK%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,    temp,      100);
		gUtility.MySprintf(temp, 100, "TVB380C2_DSLICE_BBHEADER%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_BBHeader[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,    temp,      0);
		gUtility.MySprintf(str_regTemp, 64, "TVB380C2_PLP_FILEPATH%d", tt);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt], 512,  gUtility.Get_Registry_String_Value(Reg, strKey, str_regTemp, ""));
		if (gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt]) == false)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt], 512, "");
		}

		gUtility.MySprintf(str_regTemp, 64, "TVB380C2_PLP_FILE_BITRATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, "TVB380C2_PLP_PLAYRATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
	}
	//============================================================================================================================================

	//2011/7/18 DVB-T2 IP ========================================================================================================================
	gpConfig->gBC[nBoardNum].gnIP_T2MI_BW = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW_IP", 4);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_BWT = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_FFT = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_GUARD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_L1_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PILOT_PATTERN = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN_IP", 6);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_MISO = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PAPR = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID_IP", 12421);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID = 12421;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID_IP", 32796);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID = 32769;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID_IP", 0);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID = 0;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID_IP", 4096);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_PID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_PID > 8191)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_PID = 4096;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME_IP", 2);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME < 2)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME = 2;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_DATA_SYMBOL = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL_IP", 60);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ID_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_TYPE = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_TYPE_IP", 1);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MOD_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_COD_IP", 5);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FEC_TYPE_IP", 1);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ROTATION_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_NUM_BLOCK = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_NUM_BLOCK_IP", 20);
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_T2MI_PLP_FilePath, 512,  gUtility.Get_Registry_String_Value(Reg, strKey, "TVB380T2MI_PLP_FILEPATH_IP", ""));
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Playrate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_PLAYRATE_IP", 1);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ORG_Playrate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FILE_BITRATE_IP", 0);
	//====================================================================================================================================
	//2011/12/13 ISDB-S CHANNEL
	gpConfig->gBC[nBoardNum].gnISDBS_channelNum = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ISDB_S_CHANNEL", 0);
	//2011/12/15 DVB-C2 IP +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_ID_IP", 0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Mod = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_MOD_IP", 1);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Code = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_COD_IP", 4);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Fec = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_FEC_TYPE_IP", 1);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Blk = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_BLK_IP", 100);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_HEM = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", 0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_File_Bitrate = 0;
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_FileName, 512,  "INPUT SOURCE [IP]");
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Plp_Bitrate = 1;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//----------------------------------------------------------------------------
    //--- Default RF Level
    double     RM0, RM1, RM2, MaxLevel, DefaultLevel;

    RM0 = 0.0;      // TVB595V3, AMP ON, DVB-T, 473MHz
    RM1 = RM0;

    //--- TVB595
    if (gpConfig->gBC[nBoardNum].gnBoardId == 59 ||
        gpConfig->gBC[nBoardNum].gnBoardId == 60 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 61 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 10)		// 20090812
    {
        RM1 = RM0;      // V1,V2,V3
    }
    //--- TVB590/390 - 45,47,48
    else if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if (gpConfig->gBC[nBoardNum].gnBoardId == 45)    // V8,..., V9.1
            RM1 = -27;
        else if (gpConfig->gBC[nBoardNum].gnBoardId == 47 || gpConfig->gBC[nBoardNum].gnBoardId == 48)
            RM1 = -1;                                   // V9.2,9.3,9.4, V10
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 20)
			RM1 = RM0;
		//2010/10/5 TVB593
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			RM1 = RM0;
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 44)
    {
        RM1 = -28;                                      // TVB380 V7, V8
    }

	//------------------------------------------------------------------------------------------
	// 20090812
	// 6.9.14c - S/S2 0dBm
    long isActiveMixer;
	long isS_S2_0dBm;

	isActiveMixer = 0;
	isS_S2_0dBm = 0;

	if (gpConfig->gBC[nBoardNum].gnBoardId == 48 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 60 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 61 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 27 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
	{
		// 6.10.02 - add TVB595C
		if (gpConfig->gBC[nBoardNum].gnBoardId == 48)
		{
			if ( (gpConfig->gBC[nBoardNum].gnActiveMixerUsed % 2) == 1 ||
				 gpConfig->gBC[nBoardNum].gnBoardId == 10)
                isActiveMixer = 1;
			else
                isActiveMixer = 0;
                
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			isActiveMixer = 1;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 60)
		{
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit1
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit5
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit10
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 61)
		{
			isActiveMixer = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isActiveMixer = isActiveMixer / 2;		// bit1
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		// bit5
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		

 			if (isActiveMixer > 0 && ( (isActiveMixer % 2) == 1) )
				isActiveMixer = 1;
            else
				isActiveMixer = 0;
                
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit1
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit5
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit10
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		}       
	}
	//6.9.14c - gnActiveMixerUsed (nBoardNum) -> isActiveMixer
	//------------------------------------------------------------------------------------------


    //--- 6.9.14
    if (gpConfig->gBC[nBoardNum].gnBoardId == 48 ||
        gpConfig->gBC[nBoardNum].gnBoardId == 60 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 61 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 27 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
        if (gpConfig->gBC[nBoardNum].gnActiveMixerUsed == 1)
            RM1 = RM0;

    //--- TVB590V9.2, V10.0, TVB595V1.1 or higher
    //--- 6.9.14
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        switch (gpConfig->gBC[nBoardNum].gnModulatorMode)
        {
            case QPSK:
            case DVB_S2:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (1050000000,...,1100000000)
					{
                        RM2 = RM1 - 17;
						
						if (isS_S2_0dBm == 1)
                            RM2 = RM1 - 18;                     
					}
					else                                                    // (1035000000,...,1100000000 1200000000]
                        RM2 = RM1 - 22;
                } else		// AMP OFF
                {
                    if (isActiveMixer == 1)      // (1050000000,...,1100000000)
					{
                        RM2 = RM1 - 40;
						
						//6.9.14c - S/S2 0dBm
                        if (isS_S2_0dBm == 1)
                            RM2 = RM1 + 5;
					}
					else                                                    // (1035000000,...,1100000000 1200000000]
                        RM2 = RM1 - 46;
                }
                break;

            case QAM_A:
            case QAM_B:
			case MULTIPLE_QAMB:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 1;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 0;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 28;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 27;
                }
                break;

            case DTMB:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 3;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 4;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 30;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 30;
                }
                break;

            default:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 1;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 1;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 28;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 27;
                }
                break;

        }
    } else      //if (gpConfig->gBC[nBoardNum].gnBoardId >= 47)     // 47,48,59,60
    {
        RM2 = RM1;
    }

    MaxLevel = RM2;
    DefaultLevel = MaxLevel - gpConfig->gBC[nBoardNum].gdwAttenVal;
    if (DefaultLevel > 0.0 || DefaultLevel < -200.0)
        DefaultLevel = 0.0;

    gpConfig->gBC[nBoardNum].gRFPowerLevel = gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevel", DefaultLevel);

    //----------------------------------------------------------------------------
    //--- BERT
    gpConfig->gBC[nBoardNum].gnBertPacketType = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380BERTType", 0);

    //----------------------------------------------------------------------------
    //--- NANO SOLTECH
    gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = gUtility.Get_Registry_Integer_Value(Reg, strKey, "NanoSolTech", 0);
    gpConfig->gBC[nBoardNum].gRFPowerLevel_NanoSolTech = gUtility.Get_Registry_Float_Value(Reg, strKey, "RFPowerLevel_NanoSolTech", 0.0);
    gpConfig->gBC[nBoardNum].gnCommPort_NanoSolTech = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ComPort_NanoSolTech", 1);     // COM1
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
        gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = 0;
	//AGC - RF Level -> Atten/AGC
	gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = 0;
    //----------------------------------------------------------------------------
    //--- Error Injection
    gpConfig->gBC[nBoardNum].gnErrLost = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_Lost", 0);
    gpConfig->gBC[nBoardNum].gnErrBits = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_Bits", 0);
    gpConfig->gBC[nBoardNum].gnErrBytes = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_Bytes", 0);
    gpConfig->gBC[nBoardNum].gnErrBitsCount = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsCount", 1);
    gpConfig->gBC[nBoardNum].gnErrBytesCount = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesCount", 1);
    gpConfig->gBC[nBoardNum].gnErrLostPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_LostPacket", 10000);
    gpConfig->gBC[nBoardNum].gnErrBitsPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsPacket", 10000);
    gpConfig->gBC[nBoardNum].gnErrBytesPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesPacket", 10000);

    //----------------------------------------------------------------------------
    //--- Multi Lang/Hex
#ifndef WIN32
    gpConfig->gBC[nBoardNum].gnLoadPidInfo = gUtility.Get_Registry_Integer_Value(Reg, strKey, "LoadPidInfo", 0);
#endif
    gpConfig->gBC[nBoardNum].gnHexDisplay = gUtility.Get_Registry_Integer_Value(Reg, strKey, "HexDisplay", 0);
    gpConfig->gBC[nBoardNum].gnHexDisplayTMCC = gUtility.Get_Registry_Integer_Value(Reg, strKey, "HexDisplayTmcc", 0);

	//2012/11/2 pcr restamping
	gpConfig->gBC[nBoardNum].gnPcrReStampingFlag = gUtility.Get_Registry_Integer_Value(Reg, strKey, "PCR restamping", 0);

    //----------------------------------------------------------------------------
    // Check Validity
    CheckGlobalVarValidity(nBoardNum);
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // Set Spectrum Inverse: set gnSpectrumInverse
    gUtilInd.Adjust_Spectrum(nBoardNum);

#ifdef _MS_VC
	//Registry->Close();
	Reg->Close();
#endif
    delete Reg;
}
#else // linux 
void CREG_VAR::RestoreVariables(long nBoardNum)
{
    char    strKey[256];
    char    str1[256];
    double   fValue;
	char	temp[100];
	char	strKeyStartup[256];
    //---------------------------------------------
    // Crate Registry and set root key
	FILE	*pFileStart = NULL;
	FILE	*pFileMod = NULL;
	FILE	*Reg;

    //---------------------------------------------
    // decide slot number
    if (gGeneral.gnMultiBoardUsed)
        gUtility.MySprintf(str1, 256, (char *) "TPG0590_%d", nBoardNum);
    else
        gUtility.MySprintf(str1, 256, (char *) "TPG0590_%d", gGeneral.gnBoardNum);

    //---------------------------------------------
    // Get Modulation Mode from "StartUp"
	gUtility.MySprintf(strKey, 256, (char *) "Software\\VB and VBA Program Settings\\%s\\StartUp", str1);
#ifdef STANDALONE
	gUtility.MySprintf(strKeyStartup, 256, (char *) "/sysdb/startup_%d.cfg", nBoardNum);
#else
	gUtility.MySprintf(strKeyStartup, 256, (char *) "./config/startup_%d.cfg", nBoardNum);
#endif
	pFileStart = fopen(strKeyStartup, "r");

	Reg = pFileStart;
	//gpConfig->gBC[nBoardNum].gnModulatorMode = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "TVB380ModulationMode", 2);
    //--- multilang
    gGeneral.gMultiLang = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "EnglishVersionOnly", 1);
    gpConfig->gBC[nBoardNum].gnUseFrontInput = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "FrontInputEnabled", 0);
     gpConfig->gBC[nBoardNum].gnSubBankNumber = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"SubBankNum", 7);
    gpConfig->gBC[nBoardNum].gnSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"SubBankOffset", 1024);
    gpConfig->gBC[nBoardNum].gnUseDemuxblockTest = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "DemuxBlockTestEnabled", 0);
    // sskim20070528 - keisoku
    gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "RFOutFreqUnit", 2);          // MHz
	gpConfig->gBC[nBoardNum].gnRemoveFileEnabled = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "RemoveFileEnabled", 0);
    gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *) "TDMBSubBankOffset", 128);
	//2011/6/29 TAT4710
	gpConfig->gBC[nBoardNum].gnUseTAT4710 = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"USETAT4710", 0);

    //----------------------------------------------------------------------------
    // SNMP
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszSnmpRemoteHost, 24, gUtility.Get_Registry_String_Value(Reg, strKeyStartup,(char *)"RemoteHost", (char *)"127.0.0.1"));
    gpConfig->gBC[nBoardNum].gnSnmpLocalPort = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"LocalPort", 8001);
    gpConfig->gBC[nBoardNum].gnSnmpRemotePort = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"RemotePort", 8240);

	//2011/7/13 fixed
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
		gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = 2;
	}

	if((gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xB && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE_V4 ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET_V3_%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, temp, 0);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET_V3_%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, temp, 0);
			
			if(gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			if(gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
		
		}
	}
	else
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, temp, 0);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET%d", f_i);
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, temp, 0);
			
			if(gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i] = 0;
			if(gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] < -100 || gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] > 100)
				gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i] = 0;
		
		}
	}

	//2012/2/20 FilePlay Debug Mode
	gpConfig->gBC[nBoardNum].gnDebugMode_FilePlay = gUtility.Get_Registry_Integer_Value(Reg, strKeyStartup, (char *)"DebugMode", 0);

	
	//2011/6/29
#ifdef STANDALONE
	gUtility.MySprintf(strKey, 256, (char *) "/sysdb/mod_%s_%d.cfg", gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode], nBoardNum);
#else
	gUtility.MySprintf(strKey, 256, (char *) "./config/mod_%s_%d.cfg", gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode], nBoardNum);
#endif
	pFileMod = fopen(strKey,"r");
	char AppVer[16];

    //---------------------------------------------
    // decide gnModulatorMode based on enabled type
   gUtilInd.SyncModulatorMode(nBoardNum);
    //---------------------------------------------
	//kslee 2010/3/18
    gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IP_Rx_Bitrate_Control", 0);
	//kslee 2010/4/5
	if(gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control >= 0)
		gpConfig->gBC[nBoardNum].gnIP_Rx_Bitrate_Control = 100;		//2010/4/5 97 => 100

    //---------------------------------------------
    // Set strKey: HKEY_CURRENT_USER/Software/VB and VBA Program Settings/TPG0590_x/ModType
    gUtility.MySprintf(strKey, 256, (char *) "Software\\VB and VBA Program Settings\\%s\\%s", str1, gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode]);
	Reg = pFileMod;

    //---------------------------------------------
    //--- for FileList/PlayList
#ifdef STANDALONE
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory,  256, gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "tpgts", (char *) "/usb/ts"));
#else
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory,  256, gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "tpgts", (char *) "/ts"));
#endif
    //---------------------------------------------
    //--- general, restamping
	gUtility.MyStrCpy(temp, 100, gUtility.Get_Registry_String_Value(Reg, strKey, (char *) "RepeatMode", (char *) "False"));
	if (strcmp(temp, "False") == 0)
		gpConfig->gBC[nBoardNum].gbRepeatMode = false;
	else
		gpConfig->gBC[nBoardNum].gbRepeatMode = true;
    gpConfig->gBC[nBoardNum].gdwPlayRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "PlayRate", 19392658);
    gpConfig->gBC[nBoardNum].gnCalcPlayRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ReCalcPlayRate", 0);
    gpConfig->gBC[nBoardNum].gnStartOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "StartOffset", 0);
    gpConfig->gBC[nBoardNum].gnStopMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "StopMode", 0);
	//LOOP_ADAPT
    gpConfig->gBC[nBoardNum].gnRestamping = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "Restamping", 0);
    gpConfig->gBC[nBoardNum].gnContinuity = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ContinuityCounter", 0);
    gpConfig->gBC[nBoardNum].gnDateTimeOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "DateTimeOffset", 0);
	//TDT/TOT - USER DATE/TIME
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, 20, gUtility.Get_Registry_String_Value(Reg, strKey, (char *) "DateTimeOffset_Date", (char *) "1900-01-01"));
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time, 20, gUtility.Get_Registry_String_Value(Reg, strKey, (char *) "DateTimeOffset_Time", (char *) "12-00-00"));

	//kslee 2010/3/18
	//PCR RESTAMP
	gpConfig->gBC[nBoardNum].gnPCR_Restamping = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"PCR_Restamping", 0);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
		gpConfig->gBC[nBoardNum].gnPCR_Restamping = 0;
	}
    //---------------------------------------------
    //--- Modulator
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, (char *) "TVB380RFOutFreq", 1100000000);	//kslee 2010/3/31
    else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
        gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, (char *)"TVB380RFOutFreq", 367513000);		//kslee 2010/3/31
	else
        gpConfig->gBC[nBoardNum].gnRFOutFreq = gUtility.Get_Registry_UnsignedLong_Value(Reg, strKey, (char *)"TVB380RFOutFreq", 473000000);		//kslee 2010/3/31
	int DefaultValue;
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 20000000;
	else
		DefaultValue = 3000000;
    gpConfig->gBC[nBoardNum].gnSymbolRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380SymbolRate", DefaultValue);
    
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
		DefaultValue = 2;
	else
		DefaultValue = 0;
	gpConfig->gBC[nBoardNum].gnCodeRate = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380CodeRate", DefaultValue);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
		DefaultValue = 3;
	else
		DefaultValue = 2;
	gpConfig->gBC[nBoardNum].gnBandwidth = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Bandwidth", DefaultValue);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && gUtilInd.IsSupportHMC833(nBoardNum) == 0))
		gpConfig->gBC[nBoardNum].gnBandwidth = 0;

    gpConfig->gBC[nBoardNum].gnTxmode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Txmode", 0);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 3;
	else
		DefaultValue = 0;
	gpConfig->gBC[nBoardNum].gnGuardInterval = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380GuardInterval", DefaultValue);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		DefaultValue = 2;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		DefaultValue = 3;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
		DefaultValue = 4;
	else
		DefaultValue = 0;
    gpConfig->gBC[nBoardNum].gnConstellation = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380Constellation", DefaultValue);
	
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
		DefaultValue = 4;
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
		DefaultValue = 1;
	else
		DefaultValue = 0;
    gpConfig->gBC[nBoardNum].gnQAMMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380QAMMode", DefaultValue);

    gpConfig->gBC[nBoardNum].gnQAMInterleave = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380QAMInterleaving", 0);
    gpConfig->gBC[nBoardNum].gnIFOutFreq = gUtility.Get_Registry_Integer_Value(Reg, strKey, "TVB380IFOutFreq", 0);
    
	//2010/11/17 I/Q PLAY/CAPTURE
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{
		gpConfig->gBC[nBoardNum].gnSpectrumInverse = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380SpectralInversion", 1);
	}
	else
	{
		gpConfig->gBC[nBoardNum].gnSpectrumInverse = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380SpectralInversion", 0);
	}
	if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
		gpConfig->gBC[nBoardNum].gnCurrentIF = 36000000;
	else if(gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
		gpConfig->gBC[nBoardNum].gnCurrentIF = 44000000;
	else
		gpConfig->gBC[nBoardNum].gnCurrentIF = 36125000;

	//2010/12/06 ISDB-S =============================================================================================================
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
			gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_2_3;
		else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK)
			gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_1_2;
		gpConfig->gBC[nBoardNum].gnPCR_Restamping = 1;
	}
	//===============================================================================================================================

    gpConfig->gBC[nBoardNum].gnPRBSmode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380PRBSMode", 0);
    gpConfig->gBC[nBoardNum].gnPRBSscale = gUtility.Get_Registry_Float_Value(Reg, strKey, (char *) "TVB380PRBSScale", 0.0);


	//2011/10/17 added PI
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
	{
		gpConfig->gBC[nBoardNum].gnLNB_Index = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380LNBFreqIndex", 0);
	}
    //---------------------------------------------
    //--- RF/IF Amp :
    gpConfig->gBC[nBoardNum].gnBypassAMP = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "BypassAMP", 1);

    if (gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 21.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 24.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 17.0;
            else
                fValue = 0.0;
        } else
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 14.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 18.0;
            else
                fValue = 0.0;
        }
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnRFAmpUsed == 1)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 25.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 28.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 14.0;
            else
                fValue = 0.0;
        } else if (gpConfig->gBC[nBoardNum].gnRFAmpUsed == 2)
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 25.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 28.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 ||
                     gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
                fValue = 17.0;
            else
                fValue = 0.0;
        } else
        {
            if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
                fValue = 18.0;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
                fValue = 26.0;
            else
                fValue = 0.0;
        }
    }

	//2011/11/29 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		gpConfig->gBC[nBoardNum].gnBypassAMP = 0;
		fValue = 0;
	}
	//2012/9/3 new rf level control
	if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 1)
	{
		fValue = -35;
		gpConfig->gBC[nBoardNum].gdRfLevelValue = (double)gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB59x_RF_LEVEL", fValue);
	}
	else
		gpConfig->gBC[nBoardNum].gdwAttenVal = (double)gUtility.Get_Registry_Float_Value(Reg, strKey, "TVB380AttenVal", fValue);

    //---------------------------------------------
    //--- Input Source
    gpConfig->gBC[nBoardNum].gnModulatorSource = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380InputSource", 0);
	//2011/7/6
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY/* || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2*/)
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}

	if(gpConfig->gBC[nBoardNum].gnBoardId == 0x15 && (gpConfig->gBC[nBoardNum].gnModulatorSource == 2 || gpConfig->gBC[nBoardNum].gnModulatorSource == 3))
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}
	//2012/3/27 TVB593 Multiple vsb/qam-b
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1 && 
		(gpConfig->gBC[nBoardNum].gnModulatorSource == 2 || gpConfig->gBC[nBoardNum].gnModulatorSource == 3))
	{
		gpConfig->gBC[nBoardNum].gnModulatorSource = 0;
	}

    //---------------------------------------------
    //--- DVB-H
    gpConfig->gBC[nBoardNum].gnMPE_FEC = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380MPE_FEC", 0);
    gpConfig->gBC[nBoardNum].gnTime_Slice = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380Time_Slice", 0);
    gpConfig->gBC[nBoardNum].gnIn_Depth = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380In_Depth", 0);
    gpConfig->gBC[nBoardNum].gnCell_Id = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380Cell_ID", 0);
    gpConfig->gBC[nBoardNum].gnPilot = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380Pilot", 0);

    //sskim20070223 - v5.2.0, 1==NTSC Carrier, RF = USER RF + 1.750MHz
    gpConfig->gBC[nBoardNum].gnFreqPolicy = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380FreqPolicy", 0);
    gpConfig->gBC[nBoardNum].gnRollOffFactor = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380RollOffFactor", 0);

    //----------------------------------------------------------------------------
    //--- A/V decoding, IP Streaming
    gpConfig->gBC[nBoardNum].gnIPStreamingMode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380IPStreamingMode", 0);
    gpConfig->gBC[nBoardNum].gnIPStreamingPath = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380IPStreamingPath", 0);
    gpConfig->gBC[nBoardNum].gnIPStreamingAccess = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380IPStreamingAccess", 0);
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnIPStreamingAddress,24,  gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "TVB380IPStreamingAddress", (char *) "192.168.0.1"));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnIPStreamingPort, 10, gUtility.Get_Registry_String_Value(Reg, strKey, (char *) "TVB380IPStreamingPort", (char *) "1234"));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIPStreamingInfo, sizeof(gpConfig->gBC[nBoardNum].gszIPStreamingInfo), gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "TVB380IPStreamingInfo", (char *) ""));
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo, sizeof(gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo), gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "TVB380IPStreamingInputInfo", (char *) ""));
    gpConfig->gBC[nBoardNum].gnUseIPStreaming = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "IPStreamingEnabled", 0);
    gpConfig->gBC[nBoardNum].gnIPSubBankOffset = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "IPSubBankOffset", 512);
    gpConfig->gBC[nBoardNum].gnUseAVDecoding = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "AVDecodingEnabled", 0);
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
        gpConfig->gBC[nBoardNum].gnUseAVDecoding = 0;

    //----------------------------------------------------------------------------
    //--- No UI Parameters
    gpConfig->gBC[nBoardNum].gnOutputClockSource = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380MaxPlayrate", 0);
	//2010/3/25 //2010/6/9
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)	//2010/12/17 ISDB-S
	{
		gpConfig->gBC[nBoardNum].gnOutputClockSource = 1;
	}
	gpConfig->gBC[nBoardNum].gnNationalCode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380NationalCode", 0);
    gpConfig->gBC[nBoardNum].gnChannelType = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380ChannelType", 0);

    //----------------------------------------------------------------------------
    gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "UseTMCCRemuxer", 0);
    gpConfig->gBC[nBoardNum].gdwAttenExtVal = gUtility.Get_Registry_Float_Value(Reg, strKey, (char *) "TVB380AttenExtVal", 0.0);

    //----------------------------------------------------------------------------
    //--- DTMB
    gpConfig->gBC[nBoardNum].gnFrameHeader = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380FrameHeader", 0);
    gpConfig->gBC[nBoardNum].gnCarrierNumber = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380CarrierNumber", 0);
    gpConfig->gBC[nBoardNum].gnFrameHeaderPN = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380FrameHeaderPN", 1);
    gpConfig->gBC[nBoardNum].gnPilotInsertion = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380PilotInsertion", 0);

    gpConfig->gBC[nBoardNum].gRFPowerLevelOffset = gUtility.Get_Registry_Float_Value(Reg, strKey, (char *) "TVB380RFPowerLevelOffset", 0);	// 20090812
	//AGC - RF Level -> Atten/AGC
	gpConfig->gBC[nBoardNum].gnAGC = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380AGC", 1);
 	//2011/11/29 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		gpConfig->gBC[nBoardNum].gnAGC = 0;
	}
   
	
	gpConfig->gBC[nBoardNum].gDAC_Mod_Mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380DAC_MOD_MODE", 0);

	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,(char *)"TVB380IP_RxIP", (char *)"192.168.0.1"));
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,(char *)"TVB380IP_RxMulticastIP", (char *)"0.0.0.0"));
	gpConfig->gBC[nBoardNum].gnIP_RxPort = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IP_RxPort", 1234);
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxLocalIP, 64,  gUtility.Get_Registry_String_Value(Reg, strKey,(char *)"TVB380IP_RxLocalIP", (char *)"0.0.0.0"));
	gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IP_RxUseMulticastIP", 0);


	//2010/5/28 DVB-T2
	gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_BITRATE_ADJUSTMENT", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_BW = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_BW", 4);
	gpConfig->gBC[nBoardNum].gnT2MI_BWT = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_BWT", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_FFT = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_FFT", 3);
	gpConfig->gBC[nBoardNum].gnT2MI_GUARD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_GUARD", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_L1_MOD", 3);
	gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PILOT_PATTERN", 6);
	gpConfig->gBC[nBoardNum].gnT2MI_MISO = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_MISO", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_PAPR = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PAPR", 0);
	gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_NETWORK_ID", 12421);
	if(gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID = 12421;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380T2MI_T2_SYSTEM_ID", 32796);
	if(gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID = 32769;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380T2MI_CELL_ID", 0);
	if(gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID = 0;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_PID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380T2MI_PID", 4096);
	if(gpConfig->gBC[nBoardNum].gnT2MI_PID < 0 || gpConfig->gBC[nBoardNum].gnT2MI_PID > 8191)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_PID = 4096;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380T2MI_NUM_T2_FRAME", 2);
	if(gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME < 2)
	{
		gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME = 2;
	}
	gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380T2MI_NUM_DATA_SYMBOL", 60);
	char str_regTemp[64];
	gUtility.MyStrCpy(str_regTemp, 64, (char *)"");
	for(int ii = 0; ii < MAX_PLP_TS_COUNT; ii++)
	{
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_ID%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, -1);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_MOD%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 3);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_COD%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 5);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_FEC_TYPE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_HEM%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_ROTATION%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_NUM_BLOCK%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 20);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_FILEPATH%d", ii);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii], 512,  gUtility.Get_Registry_String_Value(Reg, strKey, str_regTemp, (char *)""));
		if (gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii]) == false)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii], 512, (char *)"");
		}

		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_PLAYRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_MAXPLAYRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_MAX_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380T2MI_PLP_FILE_BITRATE%d", ii);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[ii] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
	}
	//====================================================================================================================================
	
#ifndef WIN32
	//2010/6/28 AUTO-PLAY
	gpConfig->gBC[nBoardNum].gnAuto_Play = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380AUTO_PLAY/STOP_STATUS", 0);
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszAuto_Play_FileName, sizeof(gpConfig->gBC[nBoardNum].gszAuto_Play_FileName), gUtility.Get_Registry_String_Value(Reg, strKey,(char *) "TVB380AUTO_FILENAME", (char *) ""));
#endif
	//2010/11/17 FIXED - ISDB-T Emergency Broadcasting Control
	gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting =	gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380EMERGENCY_BROADCASTING", 0);

	//2010/11/17	I/Q PLAY/CAPTURE
	gpConfig->gBC[nBoardNum].gnIQ_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_MODE", 0);
	if (gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		gpConfig->gBC[nBoardNum].gnIQ_mode = 1;
	gpConfig->gBC[nBoardNum].gnIQ_play_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_PLAY_MODE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_capture_mode = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_CAPTURE_MODE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_mem_use = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_MEM_USE", 0);
	gpConfig->gBC[nBoardNum].gnIQ_mem_size = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_MEM_SIZE", 512);
	//2011/11/18 IQ NEW FILE FORMAT
	gpConfig->gBC[nBoardNum].gnIQ_CaptureSize = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_CAPTUREDFILE_SIZE", -1);
	gpConfig->gBC[nBoardNum].gnIQ_ErrorCheck = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_ERRORCHECK_FLAG", 1);
	gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380IQ_ERRORCHECK_SIZE", -1);
	
	//2010/12/06 ISDB-S===========================================================================================================================
	for(int nIdx = 0 ; nIdx < MAX_TS_COUNT ; nIdx++)
	{
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_TS%d", nIdx);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[nIdx], 512,  gUtility.Get_Registry_String_Value(Reg, strKey,temp, (char *)""));
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_TS_Bitarte%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_Constellation%d", nIdx);
		gpConfig->gBC[nBoardNum].gnConstellation_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_Coderare%d", nIdx);
		gpConfig->gBC[nBoardNum].gnCoderate_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_SlotNum%d", nIdx);
		gpConfig->gBC[nBoardNum].gnSlotCount_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_TS_Selected%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_Selected_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, 0);
		gUtility.MySprintf(temp, 100, (char *)"TVB380COMBINER_TS_ID%d", nIdx);
		gpConfig->gBC[nBoardNum].gnTS_ID_M[nIdx] = gUtility.Get_Registry_Integer_Value(Reg, strKey, temp, -1);


	}
	//============================================================================================================================================

	//2011/2/23 DVB-C2 ===========================================================================================================================
	gpConfig->gBC[nBoardNum].gnDVB_C2_L1 = gUtility.Get_Registry_Integer_Value(Reg, strKey,                 (char *)"TVB380C2_L1TIMODE",            0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Guard = gUtility.Get_Registry_Integer_Value(Reg, strKey,              (char *)"TVB380C2_GUARD_INTERVAL",      0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Network = gUtility.Get_Registry_Integer_Value(Reg, strKey,            (char *)"TVB380C2_NETWORK_ID",          0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_System = gUtility.Get_Registry_Integer_Value(Reg, strKey,             (char *)"TVB380C2_SYSTEM_ID",           0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq = gUtility.Get_Registry_Integer_Value(Reg, strKey,          (char *)"TVB380C2_START_FREQ",          217824);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth = gUtility.Get_Registry_Integer_Value(Reg, strKey,            (char *)"TVB380C2_NUM_NOTH",            0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone = gUtility.Get_Registry_Integer_Value(Reg, strKey,       (char *)"TVB380C2_RESERVED_TONE",       0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart = gUtility.Get_Registry_Integer_Value(Reg, strKey,         (char *)"TVB380C2_NOTCH_START",         0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth = gUtility.Get_Registry_Integer_Value(Reg, strKey,         (char *)"TVB380C2_NOTCH_WIDTH",         0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type = gUtility.Get_Registry_Integer_Value(Reg, strKey,        (char *)"TVB380C2_DSLICE_TYPE",          0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader = gUtility.Get_Registry_Integer_Value(Reg, strKey,   (char *)"TVB380C2_DSLICE_FECHEADER",     0);
	//2011/5/17 DVB-C2 MULTI-PLP
	for(int tt = 0 ; tt < DVB_C2_MAX_PLP_TS_COUNT ; tt++)
	{
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_PLP_ID%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,        temp,          -1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_PLP_MODURATION%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,            temp,      1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_PLP_CODERATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,           temp,        4);
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_PLP_FECTYPE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,            temp,         1);
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_PLP_BLOCK%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,    temp,      100);
		gUtility.MySprintf(temp, 100, (char *)"TVB380C2_DSLICE_BBHEADER%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey,    temp,      0);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380C2_PLP_FILEPATH%d", tt);
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt], 512,  gUtility.Get_Registry_String_Value(Reg, strKey, str_regTemp, (char *)""));
		if (gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt]) == false)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt], 512, (char *)"");
		}

		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380C2_PLP_FILE_BITRATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 0);
		gUtility.MySprintf(str_regTemp, 64, (char *)"TVB380C2_PLP_PLAYRATE%d", tt);
		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[tt] = gUtility.Get_Registry_Integer_Value(Reg, strKey, str_regTemp, 1);
	}
	//============================================================================================================================================

	//2011/7/18 DVB-T2 IP ========================================================================================================================
	gpConfig->gBC[nBoardNum].gnIP_T2MI_BW = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_BW_IP", 4);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_BWT = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_BWT_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_FFT = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_FFT_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_GUARD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_GUARD_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_L1_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_L1_MOD_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PILOT_PATTERN = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PILOT_PATTERN_IP", 6);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_MISO = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_MISO_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PAPR = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PAPR_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_NETWORK_ID_IP", 12421);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID = 12421;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_T2_SYSTEM_ID_IP", 32796);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID = 32769;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_CELL_ID_IP", 0);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID > 65535)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID = 0;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PID_IP", 4096);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_PID < 0 || gpConfig->gBC[nBoardNum].gnIP_T2MI_PID > 8191)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_PID = 4096;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_NUM_T2_FRAME_IP", 2);
	if(gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME < 2)
	{
		gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME = 2;
	}
	gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_DATA_SYMBOL = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_NUM_DATA_SYMBOL_IP", 60);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_ID_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_MOD_IP", 3);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_COD_IP", 5);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_FEC_TYPE_IP", 1);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_HEM_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_ROTATION_IP", 0);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_NUM_BLOCK = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_NUM_BLOCK_IP", 20);
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_T2MI_PLP_FilePath, 512,  gUtility.Get_Registry_String_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_FILEPATH_IP", (char *)""));
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Playrate = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_PLAYRATE_IP", 1);
	gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ORG_Playrate = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_FILE_BITRATE_IP", 0);
	//====================================================================================================================================
	//2011/12/13 ISDB-S CHANNEL
	gpConfig->gBC[nBoardNum].gnISDBS_channelNum = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"ISDB_S_CHANNEL", 0);
	//2011/12/15 DVB-C2 IP +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_ID = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380C2MI_PLP_ID_IP", 0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Mod = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380C2MI_PLP_MOD_IP", 1);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Code = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380C2MI_PLP_COD_IP", 4);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Fec = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380C2MI_PLP_FEC_TYPE_IP", 1);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Blk = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380C2MI_PLP_BLK_IP", 100);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_HEM = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *)"TVB380T2MI_PLP_HEM_IP", 0);
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_File_Bitrate = 0;
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_FileName, 512,  (char *)"INPUT SOURCE [IP]");
	gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Plp_Bitrate = 1;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//----------------------------------------------------------------------------
    //--- Default RF Level
    double     RM0, RM1, RM2, MaxLevel, DefaultLevel;

    RM0 = 0.0;      // TVB595V3, AMP ON, DVB-T, 473MHz
    RM1 = RM0;

    //--- TVB595
    if (gpConfig->gBC[nBoardNum].gnBoardId == 59 ||
        gpConfig->gBC[nBoardNum].gnBoardId == 60 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 61 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 10)		// 20090812
    {
        RM1 = RM0;      // V1,V2,V3
    }
    //--- TVB590/390 - 45,47,48
    else if (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if (gpConfig->gBC[nBoardNum].gnBoardId == 45)    // V8,..., V9.1
            RM1 = -27;
        else if (gpConfig->gBC[nBoardNum].gnBoardId == 47 || gpConfig->gBC[nBoardNum].gnBoardId == 48)
            RM1 = -1;                                   // V9.2,9.3,9.4, V10
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 20)
			RM1 = RM0;
		//2010/10/5 TVB593
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			RM1 = RM0;
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 44)
    {
        RM1 = -28;                                      // TVB380 V7, V8
    }

	//------------------------------------------------------------------------------------------
	// 20090812
	// 6.9.14c - S/S2 0dBm
    long isActiveMixer;
	long isS_S2_0dBm;

	isActiveMixer = 0;
	isS_S2_0dBm = 0;

	if (gpConfig->gBC[nBoardNum].gnBoardId == 48 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 60 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 61 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
	{
		// 6.10.02 - add TVB595C
		if (gpConfig->gBC[nBoardNum].gnBoardId == 48)
		{
			if ( (gpConfig->gBC[nBoardNum].gnActiveMixerUsed % 2) == 1 ||
				 gpConfig->gBC[nBoardNum].gnBoardId == 10)
                isActiveMixer = 1;
			else
                isActiveMixer = 0;
                
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
			gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			isActiveMixer = 1;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 60)
		{
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit1
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit5
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit10
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		} else if (gpConfig->gBC[nBoardNum].gnBoardId == 61)
		{
			isActiveMixer = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isActiveMixer = isActiveMixer / 2;		// bit1
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		// bit5
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		
			isActiveMixer = isActiveMixer / 2;		

 			if (isActiveMixer > 0 && ( (isActiveMixer % 2) == 1) )
				isActiveMixer = 1;
            else
				isActiveMixer = 0;
                
			isS_S2_0dBm = gpConfig->gBC[nBoardNum].gnActiveMixerUsed;

			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit1
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit5
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;
			isS_S2_0dBm = isS_S2_0dBm / 2;		// bit10
			isS_S2_0dBm = isS_S2_0dBm / 2;

			if (isS_S2_0dBm > 0 && ( (isS_S2_0dBm % 2) == 1) )
				isS_S2_0dBm = 1;
            else
				isS_S2_0dBm = 0;
		}       
	}
	//6.9.14c - gnActiveMixerUsed (nBoardNum) -> isActiveMixer
	//------------------------------------------------------------------------------------------


    //--- 6.9.14
    if (gpConfig->gBC[nBoardNum].gnBoardId == 48 ||
        gpConfig->gBC[nBoardNum].gnBoardId == 60 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 61 || 
        gpConfig->gBC[nBoardNum].gnBoardId == 10 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
        if (gpConfig->gBC[nBoardNum].gnActiveMixerUsed == 1)
            RM1 = RM0;

    //--- TVB590V9.2, V10.0, TVB595V1.1 or higher
    //--- 6.9.14
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        switch (gpConfig->gBC[nBoardNum].gnModulatorMode)
        {
            case QPSK:
            case DVB_S2:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (1050000000,...,1100000000)
					{
                        RM2 = RM1 - 17;
						
						if (isS_S2_0dBm == 1)
                            RM2 = RM1 - 18;                     
					}
					else                                                    // (1035000000,...,1100000000 1200000000]
                        RM2 = RM1 - 22;
                } else		// AMP OFF
                {
                    if (isActiveMixer == 1)      // (1050000000,...,1100000000)
					{
                        RM2 = RM1 - 40;
						
						//6.9.14c - S/S2 0dBm
                        if (isS_S2_0dBm == 1)
                            RM2 = RM1 + 5;
					}
					else                                                    // (1035000000,...,1100000000 1200000000]
                        RM2 = RM1 - 46;
                }
                break;

            case QAM_A:
            case QAM_B:
			case MULTIPLE_QAMB:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 1;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 0;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 28;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 27;
                }
                break;

            case DTMB:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 3;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 4;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 30;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 30;
                }
                break;

            default:
                if (gpConfig->gBC[nBoardNum].gnBypassAMP == 0)                // AMP ON
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 1;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 1;
                } else
                {
                    if (isActiveMixer == 1)      // (450000000,...,473000000,...,500000000]
                        RM2 = RM1 - 28;
                    else                                                    // (400000000,...,473000000,...,600000000]
                        RM2 = RM1 - 27;
                }
                break;

        }
    } else      //if (gpConfig->gBC[nBoardNum].gnBoardId >= 47)     // 47,48,59,60
    {
        RM2 = RM1;
    }

    MaxLevel = RM2;
    DefaultLevel = MaxLevel - gpConfig->gBC[nBoardNum].gdwAttenVal;
    if (DefaultLevel > 0.0 || DefaultLevel < -200.0)
        DefaultLevel = 0.0;

    gpConfig->gBC[nBoardNum].gRFPowerLevel = gUtility.Get_Registry_Float_Value(Reg, strKey, (char *) "TVB380RFPowerLevel", DefaultLevel);

    //----------------------------------------------------------------------------
    //--- BERT
    gpConfig->gBC[nBoardNum].gnBertPacketType = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "TVB380BERTType", 0);

    //----------------------------------------------------------------------------
    //--- NANO SOLTECH
    gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "NanoSolTech", 0);
    gpConfig->gBC[nBoardNum].gRFPowerLevel_NanoSolTech = gUtility.Get_Registry_Float_Value(Reg, strKey, (char *) "RFPowerLevel_NanoSolTech", 0.0);
    gpConfig->gBC[nBoardNum].gnCommPort_NanoSolTech = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ComPort_NanoSolTech", 1);     // COM1
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
        gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = 0;
	//AGC - RF Level -> Atten/AGC
	gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech = 0;
    //----------------------------------------------------------------------------
    //--- Error Injection
    gpConfig->gBC[nBoardNum].gnErrLost = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_Lost", 0);
    gpConfig->gBC[nBoardNum].gnErrBits = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_Bits", 0);
    gpConfig->gBC[nBoardNum].gnErrBytes = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_Bytes", 0);
    gpConfig->gBC[nBoardNum].gnErrBitsCount = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_BitsCount", 1);
    gpConfig->gBC[nBoardNum].gnErrBytesCount = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_BytesCount", 1);
    gpConfig->gBC[nBoardNum].gnErrLostPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_LostPacket", 10000);
    gpConfig->gBC[nBoardNum].gnErrBitsPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_BitsPacket", 10000);
    gpConfig->gBC[nBoardNum].gnErrBytesPacket = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "ErrInject_BytesPacket", 10000);

    //----------------------------------------------------------------------------
    //--- Multi Lang/Hex
#ifndef WIN32
    gpConfig->gBC[nBoardNum].gnLoadPidInfo = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "LoadPidInfo", 0);
#endif
    gpConfig->gBC[nBoardNum].gnHexDisplay = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "HexDisplay", 0);
    gpConfig->gBC[nBoardNum].gnHexDisplayTMCC = gUtility.Get_Registry_Integer_Value(Reg, strKey, (char *) "HexDisplayTmcc", 0);

	//2012/11/2 pcr restamping
	gpConfig->gBC[nBoardNum].gnPcrReStampingFlag = gUtility.Get_Registry_Integer_Value(Reg, strKey, "PCR restamping", 0);

    //----------------------------------------------------------------------------
    // Check Validity
    CheckGlobalVarValidity(nBoardNum);
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // Set Spectrum Inverse: set gnSpectrumInverse
    gUtilInd.Adjust_Spectrum(nBoardNum);

#if defined(WIN32)
	//Registry->Close();
	Reg->Close();
	delete Reg;
#else
	if (pFileStart)
		fclose(pFileStart);

	if (pFileMod)
		fclose(pFileMod);
#endif

}
#endif 

//---------------------------------------------------------------------------
void CREG_VAR::SaveVariables(long nBoardNum)
{
    char    strKey[256];
    char    strKeyStartup[256];
    char    str1[256];
	char	temp[100];

#ifdef WIN32
	char	strKey2[256];
    char    strKeyStartup2[256];
#endif
    //---------------------------------------------
    // Crate Registry and set root key
#if defined(WIN32)
#ifdef _MS_VC
	//CRegKey	*Registry = new CRegKey;
	//char	*Registry = new char[10];
	RegistryKey^   Reg;
#else
    TRegistry *Registry = new TRegistry;
    Registry->RootKey = HKEY_CURRENT_USER;
#endif
#else
	FILE	*pFileStart = NULL;
	FILE	*pFileMod = NULL;
	FILE	*Reg;
#endif

    //---------------------------------------------
	char szSN[40];
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gUtility.MyStrCpy(szSN, 40, gWrapDll.Get_AdaptorInfo(gpConfig->gBC[nBoardNum].gn_OwnerSlot,0));
		gUtility.MySprintf(str1, 256, "TPG0590_%s_%d", szSN, gpConfig->gBC[nBoardNum].gn_StreamNum);
	}
	else
	{
		gUtility.MyStrCpy(szSN, 40, gWrapDll.Get_AdaptorInfo(nBoardNum,0));
		gUtility.MySprintf(str1, 256, "TPG0590_%s", szSN);
	}
    //---------------------------------------------
    // decide slot number
#if 0
    if (gGeneral.gnMultiBoardUsed)
        gUtility.MySprintf(str1, 256, "TPG0590_%d", nBoardNum);
    else
        gUtility.MySprintf(str1, 256, "TPG0590_%d", gGeneral.gnBoardNum);
#endif
    //---------------------------------------------
    // Get Modulation Mode from "StartUp"
    gUtility.MySprintf(strKeyStartup, 256, "Software\\TELEVIEW\\%s\\StartUp", str1);
    gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\%s\\%s", str1, gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode]);

#ifdef WIN32
	//kslee 2010/4/19
    gUtility.MySprintf(strKeyStartup2, 256, ".DEFAULT\\Software\\TELEVIEW\\%s\\StartUp", str1);
    gUtility.MySprintf(strKey2, 256, ".DEFAULT\\Software\\TELEVIEW\\%s\\%s", str1, gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode]);
#endif
    //---------------------------------------------
    CheckGlobalVarValidity(nBoardNum);
	
	//kslee 2010/5/18 
	//Application Version Information

#ifdef WIN32
//kslee 2010/4/19 스케줄러 실행으로 인해 시스템이 프로그램을 실행 했을 경우 대비.
	try
	{
		Reg = Registry::Users->CreateSubKey(gcnew String(strKeyStartup2));

	    //gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "TVB380ModulationMode", gpConfig->gBC[nBoardNum].gnModulatorMode);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "EnglishVersionOnly", gGeneral.gMultiLang);
		//----------------------------------------------------------------------------
		// No UI Parameters
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "FrontInputEnabled", gpConfig->gBC[nBoardNum].gnUseFrontInput);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "SubBankNum", gpConfig->gBC[nBoardNum].gnSubBankNumber);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "SubBankOffset", gpConfig->gBC[nBoardNum].gnSubBankOffset);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "DemuxBlockTestEnabled", gpConfig->gBC[nBoardNum].gnUseDemuxblockTest);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "RFOutFreqUnit", gpConfig->gBC[nBoardNum].gnRFOutFreqUnit);
		//2012/8/17
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "RFLevelUnit", gpConfig->gBC[nBoardNum].gnRfLevel_Unit);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "RemoveFileEnabled", gpConfig->gBC[nBoardNum].gnRemoveFileEnabled);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "TDMBSubBankOffset", gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset);
		
		//2011/6/29 TAT4710
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "USETAT4710", gpConfig->gBC[nBoardNum].gnUseTAT4710);
		
		//kslee 2010/5/18
		//Application Version Information 
		gUtility.Set_Registry_String_Value(Reg, strKeyStartup2, "ApplicationVersion", gGeneral.gszAppVer);
		//2012/2/20 FilePlay Debug Mode
		gUtility.Set_Registry_String_Value(Reg, strKeyStartup2, "DebugMode", 0);
		//----------------------------------------------------------------------------
		// SNMP
		gUtility.Set_Registry_String_Value(Reg, strKeyStartup2, "RemoteHost", gpConfig->gBC[nBoardNum].gszSnmpRemoteHost);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "LocalPort", gpConfig->gBC[nBoardNum].gnSnmpLocalPort);
		gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, "RemotePort", gpConfig->gBC[nBoardNum].gnSnmpRemotePort);
		//2011/5/30 DAC FREQ RANGE
		if((gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || 
			(gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
			(gpConfig->gBC[nBoardNum].gnBoardId == 0xB && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) ||
			gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE_V4 ; f_i++)
			{
				gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET_V3_%d", f_i);
				gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, temp, gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i]);
				gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET_V3_%d", f_i);
				gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, temp, gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i]);
			}
		}
		else
		{
			for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE ; f_i++)
			{
				gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET%d", f_i);
				gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, temp, gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i]);
				gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET%d", f_i);
				gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup2, temp, gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i]);
			}
		}
		//gUtility.Set_Registry_UnsignedLong_Value(Reg, strKeyStartup2, "RegistryWriteCount", gGeneral.gnRegistry_write_count + 1);
		Reg->Close();
	
		Reg = Registry::Users->CreateSubKey(gcnew String(strKey2));
		//---------------------------------------------
		// filelist/playlist, playrate, repeatmode,...
		gUtility.Set_Registry_String_Value(Reg, strKey, "tpgts", gpConfig->gBC[nBoardNum].gszMasterDirectory);
#ifdef WIN32
		gUtility.Set_Registry_String_Value(Reg, strKey, "SingleFileName", gpConfig->gBC[nBoardNum].gszSingleFileName);
		//List(20)
		for(int i_list = 0; i_list <  MAX_PLAY_LIST_COUNT; i_list++)
		{
			char regTmpName[128];
			gUtility.MySprintf(regTmpName, 128, "ListPath_%d", i_list);
			gUtility.Set_Registry_String_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].szPlayFileList[i_list]);
			gUtility.MySprintf(regTmpName, 128, "ListFileName_%d", i_list);
			gUtility.Set_Registry_String_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].szPlayListFileName[i_list]);
			gUtility.MySprintf(regTmpName, 128, "ListPlaybackRate_%d", i_list);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].gnPlayListPlaybackRate[i_list]);
			gUtility.MySprintf(regTmpName, 128, "ListAtscMhFormat_%d", i_list);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].gnPlayListAtscMh_Format[i_list]);
		}
#endif
		//gUtility.Set_Registry_String_Value(Registry, strKey, "cur_file", gpConfig->gBC[nBoardNum].szCurFileName);
		//kslee fixed
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "RepeatMode", gpConfig->gBC[nBoardNum].gbRepeatMode);
		if(gpConfig->gBC[nBoardNum].gbRepeatMode == true)
			gUtility.MyStrCpy(temp, 100, "True");
		else
			gUtility.MyStrCpy(temp, 100, "False");
		gUtility.Set_Registry_String_Value(Reg, strKey, "RepeatMode", temp);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "PlayRate", gpConfig->gBC[nBoardNum].gdwPlayRate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ReCalcPlayRate", gpConfig->gBC[nBoardNum].gnCalcPlayRate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "StartOffset", gpConfig->gBC[nBoardNum].gnStartOffset);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "StopMode", gpConfig->gBC[nBoardNum].gnStopMode);

	//2011/5/4 AD9852 OVER CLOCK
#ifdef WIN32
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			gUtility.Set_Registry_Integer_Value(Reg, strKey, "AD9852_OVERCLOCK", gpConfig->gBC[nBoardNum].gnAD9852_Overclock);
#endif
		//---------------------------------------------
		// LOOP_ADAPT
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "Restamping", gpConfig->gBC[nBoardNum].gnRestamping);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ContinuityCounter", gpConfig->gBC[nBoardNum].gnContinuity);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "DateTimeOffset", gpConfig->gBC[nBoardNum].gnDateTimeOffset);
		//TDT/TOT - USER DATE/TIME
		gUtility.Set_Registry_String_Value(Reg, strKey, "DateTimeOffset_Date", gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date);
		gUtility.Set_Registry_String_Value(Reg, strKey, "DateTimeOffset_Time", gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);


		//---------------------------------------------
		// Modulator
		gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380RFOutFreq", gpConfig->gBC[nBoardNum].gnRFOutFreq);
		gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380SymbolRate", gpConfig->gBC[nBoardNum].gnSymbolRate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380CodeRate", gpConfig->gBC[nBoardNum].gnCodeRate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Bandwidth", gpConfig->gBC[nBoardNum].gnBandwidth);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Txmode", gpConfig->gBC[nBoardNum].gnTxmode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380GuardInterval", gpConfig->gBC[nBoardNum].gnGuardInterval);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Constellation", gpConfig->gBC[nBoardNum].gnConstellation);

		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380QAMMode", gpConfig->gBC[nBoardNum].gnQAMMode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380QAMInterleaving", gpConfig->gBC[nBoardNum].gnQAMInterleave);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IFOutFreq", gpConfig->gBC[nBoardNum].gnIFOutFreq);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380SpectralInversion", gpConfig->gBC[nBoardNum].gnSpectrumInverse);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380PRBSMode", gpConfig->gBC[nBoardNum].gnPRBSmode);
		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380PRBSScale", gpConfig->gBC[nBoardNum].gnPRBSscale);
		//2012/9/3 new rf level control
		if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 1)
			gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB59x_RF_LEVEL", gpConfig->gBC[nBoardNum].gdRfLevelValue);
		else
			gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380AttenVal", gpConfig->gBC[nBoardNum].gdwAttenVal);
		//2011/10/17 added PI
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		{
			gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380LNBFreqIndex", gpConfig->gBC[nBoardNum].gnLNB_Index);
		}
#ifdef WIN32	
		//2013/3/18 Input source
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVBxxx_InputSource", gpConfig->gBC[nBoardNum].gnInputSource);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVBxxx_Channel_Number", gpConfig->gBC[nBoardNum].gnChannelNum);
#endif
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380InputSource", gpConfig->gBC[nBoardNum].gnModulatorSource);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380MPE_FEC", gpConfig->gBC[nBoardNum].gnMPE_FEC);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Time_Slice", gpConfig->gBC[nBoardNum].gnTime_Slice);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380In_Depth", gpConfig->gBC[nBoardNum].gnIn_Depth);
		gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380Cell_ID", gpConfig->gBC[nBoardNum].gnCell_Id);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Pilot", gpConfig->gBC[nBoardNum].gnPilot);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380RollOffFactor", gpConfig->gBC[nBoardNum].gnRollOffFactor);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeader", gpConfig->gBC[nBoardNum].gnFrameHeader);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380CarrierNumber", gpConfig->gBC[nBoardNum].gnCarrierNumber);

		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeaderPN", gpConfig->gBC[nBoardNum].gnFrameHeaderPN);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380PilotInsertion", gpConfig->gBC[nBoardNum].gnPilotInsertion);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380MaxPlayrate", gpConfig->gBC[nBoardNum].gnOutputClockSource);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380NationalCode", gpConfig->gBC[nBoardNum].gnNationalCode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380ChannelType", gpConfig->gBC[nBoardNum].gnChannelType);
		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380AttenExtVal", gpConfig->gBC[nBoardNum].gdwAttenExtVal);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "BypassAMP", gpConfig->gBC[nBoardNum].gnBypassAMP);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "UseTMCCRemuxer", gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer);

		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevel", gpConfig->gBC[nBoardNum].gRFPowerLevel);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "NanoSolTech", gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech);
		gUtility.Set_Registry_Float_Value(Reg, strKey, "RFPowerLevel_NanoSolTech", gpConfig->gBC[nBoardNum].gRFPowerLevel_NanoSolTech);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ComPort_NanoSolTech", gpConfig->gBC[nBoardNum].gnCommPort_NanoSolTech);

		//---------------------------------------------
		// ERROR INJECTION
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Lost", gpConfig->gBC[nBoardNum].gnErrLost);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Bits", gpConfig->gBC[nBoardNum].gnErrBits);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Bytes", gpConfig->gBC[nBoardNum].gnErrBytes);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsCount", gpConfig->gBC[nBoardNum].gnErrBitsCount);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesCount", gpConfig->gBC[nBoardNum].gnErrBytesCount);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_LostPacket", gpConfig->gBC[nBoardNum].gnErrLostPacket);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsPacket", gpConfig->gBC[nBoardNum].gnErrBitsPacket);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesPacket", gpConfig->gBC[nBoardNum].gnErrBytesPacket);

		//----------------------------------------------------------------------------
		// A/V decoding, IP Streaming
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingMode", gpConfig->gBC[nBoardNum].gnIPStreamingMode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingPath", gpConfig->gBC[nBoardNum].gnIPStreamingPath);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingAccess", gpConfig->gBC[nBoardNum].gnIPStreamingAccess);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingAddress", gpConfig->gBC[nBoardNum].gnIPStreamingAddress);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingPort", gpConfig->gBC[nBoardNum].gnIPStreamingPort);

		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingInfo", gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingInputInfo", gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "IPStreamingEnabled", gpConfig->gBC[nBoardNum].gnUseIPStreaming);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "IPSubBankOffset", gpConfig->gBC[nBoardNum].gnIPSubBankOffset);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "AVDecodingEnabled", gpConfig->gBC[nBoardNum].gnUseAVDecoding);

		//----------------------------------------------------------------------------
		// BERT
	   gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380BERTType", gpConfig->gBC[nBoardNum].gnBertPacketType);
#ifndef WIN32
	   gUtility.Set_Registry_Integer_Value(Reg, strKey, "LoadPidInfo", gpConfig->gBC[nBoardNum].gnLoadPidInfo);
#endif
	   gUtility.Set_Registry_Integer_Value(Reg, strKey, "HexDisplay", gpConfig->gBC[nBoardNum].gnHexDisplay);
	   gUtility.Set_Registry_Integer_Value(Reg, strKey, "HexDisplayTmcc", gpConfig->gBC[nBoardNum].gnHexDisplayTMCC);

		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevelOffset", gpConfig->gBC[nBoardNum].gRFPowerLevelOffset);	//20090812
		//AGC - RF Level -> Atten/AGC
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380AGC", gpConfig->gBC[nBoardNum].gnAGC);

		//2010/2/11 DAC - Offset, Gain
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_OFFSET", gpConfig->gBC[nBoardNum].gDAC_I_Offset);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_OFFSET", gpConfig->gBC[nBoardNum].gDAC_Q_Offset);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_GAIN", gpConfig->gBC[nBoardNum].gDAC_I_Gain);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_GAIN", gpConfig->gBC[nBoardNum].gDAC_Q_Gain);
		//kslee 2010/3/18 
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_OFFSET_COARSE", gpConfig->gBC[nBoardNum].gDAC_I_Offset_Coarse);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_OFFSET_COARSE", gpConfig->gBC[nBoardNum].gDAC_Q_Offset_Coarse);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_MOD_MODE", gpConfig->gBC[nBoardNum].gDAC_Mod_Mode);

		//IP UDP/RTP
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxIP", gpConfig->gBC[nBoardNum].gszIP_RxIP);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxMulticastIP", gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxPort", gpConfig->gBC[nBoardNum].gnIP_RxPort);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxLocalIP", gpConfig->gBC[nBoardNum].gszIP_RxLocalIP);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxUseMulticastIP", gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP);
		
		//2010/5/28 DVB-T2
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BITRATE_ADJUSTMENT", gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW", gpConfig->gBC[nBoardNum].gnT2MI_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT", gpConfig->gBC[nBoardNum].gnT2MI_BWT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT", gpConfig->gBC[nBoardNum].gnT2MI_FFT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD", gpConfig->gBC[nBoardNum].gnT2MI_GUARD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD", gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN", gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO", gpConfig->gBC[nBoardNum].gnT2MI_MISO);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR", gpConfig->gBC[nBoardNum].gnT2MI_PAPR);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID", gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID", gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID", gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID", gpConfig->gBC[nBoardNum].gnT2MI_PID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME", gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL", gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL);
		//2010/12/20 DVB-T2 MULTI-PLP =======================================================================================================
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_TYPE", gpConfig->gBC[nBoardNum].gnT2MI_PLP_TYPE);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MOD", gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_COD", gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FEC_TYPE", gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_HEM", gpConfig->gBC[nBoardNum].gnT2MI_HEM);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_NUM_BLOCK", gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK);
		char str_regTemp[64];
		gUtility.MyStrCpy(str_regTemp, 64, "");
		for(int ii = 0; ii < MAX_PLP_TS_COUNT; ii++)
		{
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_ID%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_TYPE%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_TYPE[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_MOD%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_COD%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FEC_TYPE%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_HEM%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_ROTATION%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_NUM_BLOCK%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FILEPATH%d", ii);
			gUtility.Set_Registry_String_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_PLAYRATE%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_MAXPLAYRATE%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_MAX_Playrate[ii]);
			gUtility.MySprintf(str_regTemp, 64, "TVB380T2MI_PLP_FILE_BITRATE%d", ii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp, gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[ii]);

		}
		//===================================================================================================================================
		//2010/9/13 FIXED - ISDB-T Emergency Broadcasting Control
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380EMERGENCY_BROADCASTING", gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting);

		//2010/7/20 I/Q PLAY/CAPTURE
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MODE", gpConfig->gBC[nBoardNum].gnIQ_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_PLAY_MODE", gpConfig->gBC[nBoardNum].gnIQ_play_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTURE_MODE", gpConfig->gBC[nBoardNum].gnIQ_capture_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_USE", gpConfig->gBC[nBoardNum].gnIQ_mem_use);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_SIZE", gpConfig->gBC[nBoardNum].gnIQ_mem_size);
		//2011/11/18 IQ NEW FILE FORMAT
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTUREDFILE_SIZE", gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_FLAG", gpConfig->gBC[nBoardNum].gnIQ_ErrorCheck);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_SIZE", gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize);

		//2010/12/06 ISDB-S	============================================================================================
		char temp[100];
		for(int nIdx = 0 ; nIdx < MAX_TS_COUNT ; nIdx++)
		{
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS%d", nIdx);
			gUtility.Set_Registry_String_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gszTS_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Bitarte%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_Constellation%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnConstellation_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_Coderare%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnCoderate_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_SlotNum%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnSlotCount_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Selected%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_Selected_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_ID%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_ID_M[nIdx]);
		}
		//==============================================================================================================

		//2011/2/23	DVB-C2 ====================================================================================================================
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_BANDWIDTH",           gpConfig->gBC[nBoardNum].gnDVB_C2_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_L1TIMODE",            gpConfig->gBC[nBoardNum].gnDVB_C2_L1);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_GUARD_INTERVAL",      gpConfig->gBC[nBoardNum].gnDVB_C2_Guard);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NETWORK_ID",          gpConfig->gBC[nBoardNum].gnDVB_C2_Network);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_SYSTEM_ID",           gpConfig->gBC[nBoardNum].gnDVB_C2_System);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_START_FREQ",          gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NUM_NOTH",            gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_RESERVED_TONE",       gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NOTCH_START",         gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NOTCH_WIDTH",         gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_TUNEPOS",      gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_OFFSET_RIGHT", gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_OFFSET_LEFT",  gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_TYPE",		 gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_FECHEADER",	 gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader);
		//2011/5/17 DVB-C2 MULTI-PLP
		for(int tt = 0 ; tt < DVB_C2_MAX_PLP_TS_COUNT ; tt++)
		{
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_ID%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_MODURATION%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_CODERATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FECTYPE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_BLOCK%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_DSLICE_BBHEADER%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_BBHeader[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FILEPATH%d", tt);
			gUtility.Set_Registry_String_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FILE_BITRATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_PLAYRATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[tt]);
			//gUtility.MySprintf(temp, 100, "TVB380C2_PLP_MAXPLAYRATE%d", tt);
			//gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_MAX_Playrate[tt]);
		}
	//2011/12/15 DVB-C2 IP +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_ID_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_MOD_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Mod);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_COD_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Code);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_FEC_TYPE_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Fec);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_BLK_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Blk);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_HEM);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		//=====================================================================================================================================

		//2011/7/18 DVB-T2 IP =================================================================================================================
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_BWT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_FFT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_GUARD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_L1_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PILOT_PATTERN);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_MISO);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PAPR);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_DATA_SYMBOL);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_TYPE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_TYPE);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MOD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_COD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FEC_TYPE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ROTATION_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_NUM_BLOCK_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_NUM_BLOCK);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380T2MI_PLP_FILEPATH_IP", gpConfig->gBC[nBoardNum].gszIP_T2MI_PLP_FilePath);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_PLAYRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Playrate);
//		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MAXPLAYRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MAX_Playrate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FILE_BITRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ORG_Playrate);
		//===================================================================================================================================
		//2011/12/13 ISDB-S CHANNEL
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ISDB_S_CHANNEL", gpConfig->gBC[nBoardNum].gnISDBS_channelNum);

		//2012/11/2 pcr restamping
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "PCR restamping", gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);


		Reg->Close();
	}
	catch ( Exception^ ex )
	{
		ex->ToString();
	}
#endif

#ifdef WIN32

#ifdef _MS_VC
	//if (Registry->Create(HKEY_CURRENT_USER,strKeyStartup) != ERROR_SUCCESS)
	//	return;
	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKeyStartup));
	if(!Reg)
		return;
#endif
#else 
//2011/6/29
#ifdef STANDALONE
	gUtility.MySprintf(strKey, 256, (char *) "/sysdb/startup_%d.cfg", nBoardNum);
#else
	gUtility.MySprintf(strKey, 256, (char *) "./config/startup_%d.cfg", nBoardNum);
#endif
	pFileStart = fopen(strKey, "w");
//2011/6/29
#ifdef STANDALONE
	gUtility.MySprintf(strKey, 256, (char *) "/sysdb/mod_%s_%d.cfg", gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode], nBoardNum);
#else
	gUtility.MySprintf(strKey, 256, (char *) "./config/mod_%s_%d.cfg", gstrModTypeReg[gpConfig->gBC[nBoardNum].gnModulatorMode], nBoardNum);
#endif
	pFileMod = fopen(strKey,"w");

#endif


#if !defined(WIN32)
	Reg = pFileStart;
#endif


	//---------------------------------------------
//#ifndef WIN32
//	gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "TVB380ModulationMode", gpConfig->gBC[nBoardNum].gnModulatorMode);
//#endif
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "EnglishVersionOnly", gGeneral.gMultiLang);
    //----------------------------------------------------------------------------
    // No UI Parameters
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "FrontInputEnabled", gpConfig->gBC[nBoardNum].gnUseFrontInput);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "SubBankNum", gpConfig->gBC[nBoardNum].gnSubBankNumber);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "SubBankOffset", gpConfig->gBC[nBoardNum].gnSubBankOffset);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "DemuxBlockTestEnabled", gpConfig->gBC[nBoardNum].gnUseDemuxblockTest);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "RFOutFreqUnit", gpConfig->gBC[nBoardNum].gnRFOutFreqUnit);
    //2012/8/17
	gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "RFLevelUnit", gpConfig->gBC[nBoardNum].gnRfLevel_Unit);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "RemoveFileEnabled", gpConfig->gBC[nBoardNum].gnRemoveFileEnabled);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "TDMBSubBankOffset", gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset);
	//2011/6/29 TAT4710
	gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "USETAT4710", gpConfig->gBC[nBoardNum].gnUseTAT4710);

	//----------------------------------------------------------------------------
    // SNMP
    gUtility.Set_Registry_String_Value(Reg, strKeyStartup, "RemoteHost", gpConfig->gBC[nBoardNum].gszSnmpRemoteHost);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "LocalPort", gpConfig->gBC[nBoardNum].gnSnmpLocalPort);
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, "RemotePort", gpConfig->gBC[nBoardNum].gnSnmpRemotePort);

	//2011/5/30 DAC FREQ RANGE
	if((gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xB && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE_V4 ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET_V3_%d", f_i);
			gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, temp, gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i]);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET_V3_%d", f_i);
			gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, temp, gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i]);
		}
	}
	else
	{
		for(int f_i = 0 ; f_i < MAX_DAC_FREQ_RANGE ; f_i++)
		{
			gUtility.MySprintf(temp, 100, "TVB380DAC_I_OFFSET%d", f_i);
			gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, temp, gpConfig->gBC[nBoardNum].gDAC_I_Offset[f_i]);
			gUtility.MySprintf(temp, 100, "TVB380DAC_Q_OFFSET%d", f_i);
			gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, temp, gpConfig->gBC[nBoardNum].gDAC_Q_Offset[f_i]);
		}
	}
	//kslee 2010/4/19
	//gUtility.Set_Registry_UnsignedLong_Value(Reg, strKeyStartup2, "RegistryWriteCount", gGeneral.gnRegistry_write_count + 1);
	//kslee 2010/5/18
	//Application Version Information 
	gUtility.Set_Registry_String_Value(Reg, strKeyStartup, "ApplicationVersion", gGeneral.gszAppVer);
	//2012/2/20 FilePlay Debug Mode
	gUtility.Set_Registry_String_Value(Reg, strKeyStartup, "DebugMode", 0);

#if defined(WIN32)

#ifdef _MS_VC
	
	//Registry->Close();
	//if (Registry->Create(HKEY_CURRENT_USER,strKey) != ERROR_SUCCESS)
	//	return;
	Reg->Close();
	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));
	if(!Reg)
		return;
#endif

#else
	Reg = pFileMod;
#endif

    

	//---------------------------------------------
    // filelist/playlist, playrate, repeatmode,...
    gUtility.Set_Registry_String_Value(Reg, strKey, "tpgts", gpConfig->gBC[nBoardNum].gszMasterDirectory);
#ifdef WIN32
	gUtility.Set_Registry_String_Value(Reg, strKey, "SingleFileName", gpConfig->gBC[nBoardNum].gszSingleFileName);
	//List(20)
	for(int i_list = 0; i_list <  MAX_PLAY_LIST_COUNT; i_list++)
	{
		char regTmpName[128];
		gUtility.MySprintf(regTmpName, 128, "ListPath_%d", i_list);
		gUtility.Set_Registry_String_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].szPlayFileList[i_list]);
		gUtility.MySprintf(regTmpName, 128, "ListFileName_%d", i_list);
		gUtility.Set_Registry_String_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].szPlayListFileName[i_list]);
		gUtility.MySprintf(regTmpName, 128, "ListPlaybackRate_%d", i_list);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].gnPlayListPlaybackRate[i_list]);
		gUtility.MySprintf(regTmpName, 128, "ListAtscMhFormat_%d", i_list);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, regTmpName, gpConfig->gBC[nBoardNum].gnPlayListAtscMh_Format[i_list]);
	}
#endif
    //gUtility.Set_Registry_String_Value(Registry, strKey, "cur_file", gpConfig->gBC[nBoardNum].szCurFileName);
    //kslee fixed
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "RepeatMode", gpConfig->gBC[nBoardNum].gbRepeatMode);
    if(gpConfig->gBC[nBoardNum].gbRepeatMode == true)
		gUtility.MyStrCpy(temp, 100, "True");
	else
		gUtility.MyStrCpy(temp, 100, "False");
	gUtility.Set_Registry_String_Value(Reg, strKey, "RepeatMode", temp);
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "PlayRate", gpConfig->gBC[nBoardNum].gdwPlayRate);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ReCalcPlayRate", gpConfig->gBC[nBoardNum].gnCalcPlayRate);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "StartOffset", gpConfig->gBC[nBoardNum].gnStartOffset);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "StopMode", gpConfig->gBC[nBoardNum].gnStopMode);

	//2011/5/4 AD9852 OVER CLOCK
#ifdef WIN32
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			gUtility.Set_Registry_Integer_Value(Reg, strKey, "AD9852_OVERCLOCK", gpConfig->gBC[nBoardNum].gnAD9852_Overclock);
#endif
    //---------------------------------------------
    // LOOP_ADAPT
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "Restamping", gpConfig->gBC[nBoardNum].gnRestamping);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ContinuityCounter", gpConfig->gBC[nBoardNum].gnContinuity);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "DateTimeOffset", gpConfig->gBC[nBoardNum].gnDateTimeOffset);
	//TDT/TOT - USER DATE/TIME
	gUtility.Set_Registry_String_Value(Reg, strKey, "DateTimeOffset_Date", gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date);
	gUtility.Set_Registry_String_Value(Reg, strKey, "DateTimeOffset_Time", gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);

	//kslee 2010/3/18
	//PCR RESTAMP
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "PCR_Restamping", gpConfig->gBC[nBoardNum].gnPCR_Restamping);
	

    //---------------------------------------------
    // Modulator
    gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380RFOutFreq", gpConfig->gBC[nBoardNum].gnRFOutFreq);
    gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380SymbolRate", gpConfig->gBC[nBoardNum].gnSymbolRate);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380CodeRate", gpConfig->gBC[nBoardNum].gnCodeRate);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Bandwidth", gpConfig->gBC[nBoardNum].gnBandwidth);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Txmode", gpConfig->gBC[nBoardNum].gnTxmode);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380GuardInterval", gpConfig->gBC[nBoardNum].gnGuardInterval);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Constellation", gpConfig->gBC[nBoardNum].gnConstellation);

    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380QAMMode", gpConfig->gBC[nBoardNum].gnQAMMode);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380QAMInterleaving", gpConfig->gBC[nBoardNum].gnQAMInterleave);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IFOutFreq", gpConfig->gBC[nBoardNum].gnIFOutFreq);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380SpectralInversion", gpConfig->gBC[nBoardNum].gnSpectrumInverse);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380PRBSMode", gpConfig->gBC[nBoardNum].gnPRBSmode);
    gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380PRBSScale", gpConfig->gBC[nBoardNum].gnPRBSscale);
	//2012/9/3 new rf level control
	if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 1)
		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB59x_RF_LEVEL", gpConfig->gBC[nBoardNum].gdRfLevelValue);
	else	
		gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380AttenVal", gpConfig->gBC[nBoardNum].gdwAttenVal);
	//2011/10/17 added PI
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
	{
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380LNBFreqIndex", gpConfig->gBC[nBoardNum].gnLNB_Index);
	}
#ifdef WIN32
	//2013/3/18 Input source
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVBxxx_InputSource", gpConfig->gBC[nBoardNum].gnInputSource);

	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVBxxx_Channel_Number", gpConfig->gBC[nBoardNum].gnChannelNum);
#endif
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380InputSource", gpConfig->gBC[nBoardNum].gnModulatorSource);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380MPE_FEC", gpConfig->gBC[nBoardNum].gnMPE_FEC);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Time_Slice", gpConfig->gBC[nBoardNum].gnTime_Slice);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380In_Depth", gpConfig->gBC[nBoardNum].gnIn_Depth);
    gUtility.Set_Registry_UnsignedLong_Value(Reg, strKey, "TVB380Cell_ID", gpConfig->gBC[nBoardNum].gnCell_Id);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380Pilot", gpConfig->gBC[nBoardNum].gnPilot);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380RollOffFactor", gpConfig->gBC[nBoardNum].gnRollOffFactor);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeader", gpConfig->gBC[nBoardNum].gnFrameHeader);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380CarrierNumber", gpConfig->gBC[nBoardNum].gnCarrierNumber);

    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380FrameHeaderPN", gpConfig->gBC[nBoardNum].gnFrameHeaderPN);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380PilotInsertion", gpConfig->gBC[nBoardNum].gnPilotInsertion);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380MaxPlayrate", gpConfig->gBC[nBoardNum].gnOutputClockSource);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380NationalCode", gpConfig->gBC[nBoardNum].gnNationalCode);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380ChannelType", gpConfig->gBC[nBoardNum].gnChannelType);
    gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380AttenExtVal", gpConfig->gBC[nBoardNum].gdwAttenExtVal);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "BypassAMP", gpConfig->gBC[nBoardNum].gnBypassAMP);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "UseTMCCRemuxer", gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer);

    gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevel", gpConfig->gBC[nBoardNum].gRFPowerLevel);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "NanoSolTech", gpConfig->gBC[nBoardNum].gnUseAmp_NanoSolTech);
    gUtility.Set_Registry_Float_Value(Reg, strKey, "RFPowerLevel_NanoSolTech", gpConfig->gBC[nBoardNum].gRFPowerLevel_NanoSolTech);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ComPort_NanoSolTech", gpConfig->gBC[nBoardNum].gnCommPort_NanoSolTech);

    //---------------------------------------------
    // ERROR INJECTION
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Lost", gpConfig->gBC[nBoardNum].gnErrLost);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Bits", gpConfig->gBC[nBoardNum].gnErrBits);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_Bytes", gpConfig->gBC[nBoardNum].gnErrBytes);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsCount", gpConfig->gBC[nBoardNum].gnErrBitsCount);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesCount", gpConfig->gBC[nBoardNum].gnErrBytesCount);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_LostPacket", gpConfig->gBC[nBoardNum].gnErrLostPacket);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BitsPacket", gpConfig->gBC[nBoardNum].gnErrBitsPacket);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "ErrInject_BytesPacket", gpConfig->gBC[nBoardNum].gnErrBytesPacket);

    //----------------------------------------------------------------------------
    // A/V decoding, IP Streaming
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingMode", gpConfig->gBC[nBoardNum].gnIPStreamingMode);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingPath", gpConfig->gBC[nBoardNum].gnIPStreamingPath);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IPStreamingAccess", gpConfig->gBC[nBoardNum].gnIPStreamingAccess);
    gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingAddress", gpConfig->gBC[nBoardNum].gnIPStreamingAddress);
    gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingPort", gpConfig->gBC[nBoardNum].gnIPStreamingPort);

    gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingInfo", gpConfig->gBC[nBoardNum].gszIPStreamingInfo);
    gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IPStreamingInputInfo", gpConfig->gBC[nBoardNum].gszIPStreamingInputInfo);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "IPStreamingEnabled", gpConfig->gBC[nBoardNum].gnUseIPStreaming);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "IPSubBankOffset", gpConfig->gBC[nBoardNum].gnIPSubBankOffset);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "AVDecodingEnabled", gpConfig->gBC[nBoardNum].gnUseAVDecoding);

    //----------------------------------------------------------------------------
    // BERT
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380BERTType", gpConfig->gBC[nBoardNum].gnBertPacketType);
#ifndef WIN32
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "LoadPidInfo", gpConfig->gBC[nBoardNum].gnLoadPidInfo);
#endif
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "HexDisplay", gpConfig->gBC[nBoardNum].gnHexDisplay);
    gUtility.Set_Registry_Integer_Value(Reg, strKey, "HexDisplayTmcc", gpConfig->gBC[nBoardNum].gnHexDisplayTMCC);

    gUtility.Set_Registry_Float_Value(Reg, strKey, "TVB380RFPowerLevelOffset", gpConfig->gBC[nBoardNum].gRFPowerLevelOffset);	//20090812
	//AGC - RF Level -> Atten/AGC
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380AGC", gpConfig->gBC[nBoardNum].gnAGC);

	//2010/2/11 DAC - Offset, Gain
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_OFFSET", gpConfig->gBC[nBoardNum].gDAC_I_Offset);
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_OFFSET", gpConfig->gBC[nBoardNum].gDAC_Q_Offset);
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_GAIN", gpConfig->gBC[nBoardNum].gDAC_I_Gain);
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_GAIN", gpConfig->gBC[nBoardNum].gDAC_Q_Gain);
	
	//kslee 2010/3/18 
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_I_OFFSET_COARSE", gpConfig->gBC[nBoardNum].gDAC_I_Offset_Coarse);
	//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_Q_OFFSET_COARSE", gpConfig->gBC[nBoardNum].gDAC_Q_Offset_Coarse);
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380DAC_MOD_MODE", gpConfig->gBC[nBoardNum].gDAC_Mod_Mode);

	//IP UDP/RTP
	gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxIP", gpConfig->gBC[nBoardNum].gszIP_RxIP);
	gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxMulticastIP", gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP);
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxPort", gpConfig->gBC[nBoardNum].gnIP_RxPort);
	gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380IP_RxLocalIP", gpConfig->gBC[nBoardNum].gszIP_RxLocalIP);
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IP_RxUseMulticastIP", gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP);
	
		//2010/5/28 DVB-T2
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BITRATE_ADJUSTMENT", gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW", gpConfig->gBC[nBoardNum].gnT2MI_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT", gpConfig->gBC[nBoardNum].gnT2MI_BWT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT", gpConfig->gBC[nBoardNum].gnT2MI_FFT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD", gpConfig->gBC[nBoardNum].gnT2MI_GUARD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD", gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN", gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO", gpConfig->gBC[nBoardNum].gnT2MI_MISO);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR", gpConfig->gBC[nBoardNum].gnT2MI_PAPR);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID", gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID", gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID", gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID", gpConfig->gBC[nBoardNum].gnT2MI_PID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME", gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL", gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL);
		//2010/12/20 DVB-T2 MULTI-PLP =======================================================================================================
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_TYPE", gpConfig->gBC[nBoardNum].gnT2MI_PLP_TYPE);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MOD", gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_COD", gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FEC_TYPE", gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_HEM", gpConfig->gBC[nBoardNum].gnT2MI_HEM);
		//gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_NUM_BLOCK", gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK);
		char str_regTemp2[64];
		gUtility.MyStrCpy(str_regTemp2, 64, "");
		for(int iii = 0; iii < MAX_PLP_TS_COUNT; iii++)
		{
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_ID%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_TYPE%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_TYPE[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_MOD%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_COD%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_FEC_TYPE%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_HEM%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_ROTATION%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_NUM_BLOCK%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_FILEPATH%d", iii);
			gUtility.Set_Registry_String_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_PLAYRATE%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_MAXPLAYRATE%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_MAX_Playrate[iii]);
			gUtility.MySprintf(str_regTemp2, 64, "TVB380T2MI_PLP_FILE_BITRATE%d", iii);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, str_regTemp2, gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[iii]);
		}
		//===================================================================================================================================

		//2010/9/13 FIXED - ISDB-T Emergency Broadcasting Control
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380EMERGENCY_BROADCASTING", gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting);

		//2010/7/20 I/Q PLAY/CAPTURE
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MODE", gpConfig->gBC[nBoardNum].gnIQ_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_PLAY_MODE", gpConfig->gBC[nBoardNum].gnIQ_play_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTURE_MODE", gpConfig->gBC[nBoardNum].gnIQ_capture_mode);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_USE", gpConfig->gBC[nBoardNum].gnIQ_mem_use);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_MEM_SIZE", gpConfig->gBC[nBoardNum].gnIQ_mem_size);
		//2011/11/18 IQ NEW FILE FORMAT
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_CAPTUREDFILE_SIZE", gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_FLAG", gpConfig->gBC[nBoardNum].gnIQ_ErrorCheck);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380IQ_ERRORCHECK_SIZE", gpConfig->gBC[nBoardNum].gnIQ_ErrorCheckSize);

		//2010/12/06 ISDB-S	============================================================================================
		for(int nIdx = 0 ; nIdx < MAX_TS_COUNT ; nIdx++)
		{
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS%d", nIdx);
			gUtility.Set_Registry_String_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gszTS_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Bitarte%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_Constellation%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnConstellation_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_Coderare%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnCoderate_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_SlotNum%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnSlotCount_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_Selected%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_Selected_M[nIdx]);
			gUtility.MySprintf(temp, 100, "TVB380COMBINER_TS_ID%d", nIdx);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnTS_ID_M[nIdx]);
		}
		//==============================================================================================================
#ifndef WIN32
	//2010/6/28 AUTO-PLAY
	if(gpConfig->gBC[nBoardNum].bPlayingProgress == true)
		gUtility.Set_Registry_Integer_Value(Reg, strKey, (char *) "TVB380AUTO_PLAY/STOP_STATUS", 1);
	else
		gUtility.Set_Registry_Integer_Value(Reg, strKey, (char *) "TVB380AUTO_PLAY/STOP_STATUS", 0);

	//PlayList
	if(gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
	{
		gUtility.Set_Registry_String_Value(Reg, strKey, (char *)"TVB380AUTO_FILENAME", gpConfig->gBC[nBoardNum].szPlayFileList[gpConfig->gBC[nBoardNum].nPlayListIndexCur]);
	}
	//FileList
	else if(gpConfig->gBC[nBoardNum].nFileListIndexCount > 0)
	{
		gUtility.Set_Registry_String_Value(Reg, strKey, (char *)"TVB380AUTO_FILENAME", gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
	}
	else
	{
		gUtility.Set_Registry_String_Value(Reg, strKey, (char *)"TVB380AUTO_FILENAME", (char *)"");
	}
#endif

		//2011/2/23	DVB-C2 ====================================================================================================================
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_BANDWIDTH",           gpConfig->gBC[nBoardNum].gnDVB_C2_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_L1TIMODE",            gpConfig->gBC[nBoardNum].gnDVB_C2_L1);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_GUARD_INTERVAL",      gpConfig->gBC[nBoardNum].gnDVB_C2_Guard);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NETWORK_ID",          gpConfig->gBC[nBoardNum].gnDVB_C2_Network);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_SYSTEM_ID",           gpConfig->gBC[nBoardNum].gnDVB_C2_System);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_START_FREQ",          gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NUM_NOTH",            gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_RESERVED_TONE",       gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NOTCH_START",         gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_NOTCH_WIDTH",         gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_TUNEPOS",      gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_OFFSET_RIGHT", gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_OFFSET_LEFT",  gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_TYPE",		 gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2_DSLICE_FECHEADER",	 gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader);
		//2011/5/17 DVB-C2 MULTI-PLP
		for(int tt = 0 ; tt < DVB_C2_MAX_PLP_TS_COUNT ; tt++)
		{
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_ID%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[tt]);
			//gUtility.MySprintf(temp, 100, "TVB380C2_DSLICE_TYPE%d", tt);
			//gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type[tt]);
			//gUtility.MySprintf(temp, 100, "TVB380C2_DSLICE_FECHEADER%d", tt);
			//gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_MODURATION%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_CODERATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FECTYPE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_BLOCK%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_DSLICE_BBHEADER%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_BBHeader[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FILEPATH%d", tt);
			gUtility.Set_Registry_String_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_FILE_BITRATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[tt]);
			gUtility.MySprintf(temp, 100, "TVB380C2_PLP_PLAYRATE%d", tt);
			gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[tt]);
			//gUtility.MySprintf(temp, 100, "TVB380C2_PLP_MAXPLAYRATE%d", tt);
			//gUtility.Set_Registry_Integer_Value(Reg, strKey, temp, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_MAX_Playrate[tt]);
		}
		//=====================================================================================================================================
	//2011/12/15 DVB-C2 IP +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_ID_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_MOD_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Mod);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_COD_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Code);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_FEC_TYPE_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Fec);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380C2MI_PLP_BLK_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Blk);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", gpConfig->gBC[nBoardNum].gnDVB_C2_IP_HEM);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		//2011/7/18 DVB-T2 IP =================================================================================================================
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BW_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_BW);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_BWT_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_BWT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_FFT_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_FFT);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_GUARD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_GUARD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_L1_MOD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_L1_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PILOT_PATTERN_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PILOT_PATTERN);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_MISO_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_MISO);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PAPR_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PAPR);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NETWORK_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_T2_SYSTEM_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_CELL_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_T2_FRAME_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_NUM_DATA_SYMBOL_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_DATA_SYMBOL);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ID_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_TYPE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_TYPE);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MOD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_COD_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FEC_TYPE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_HEM_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_ROTATION_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_NUM_BLOCK_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_NUM_BLOCK);
		gUtility.Set_Registry_String_Value(Reg, strKey, "TVB380T2MI_PLP_FILEPATH_IP", gpConfig->gBC[nBoardNum].gszIP_T2MI_PLP_FilePath);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_PLAYRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Playrate);
//		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_MAXPLAYRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MAX_Playrate);
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "TVB380T2MI_PLP_FILE_BITRATE_IP", gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ORG_Playrate);
		//===================================================================================================================================
		//2011/12/13 ISDB-S CHANNEL
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ISDB_S_CHANNEL", gpConfig->gBC[nBoardNum].gnISDBS_channelNum);
		//2012/11/2 pcr restamping
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "PCR restamping", gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);

#if defined(WIN32)

#ifdef _MS_VC

	//Registry->Close();
	Reg->Close();
	delete Reg;
#endif

#else
	if (pFileStart)
		fclose(pFileStart);

	if (pFileMod)
		fclose(pFileMod);
#endif


    
}

//---------------------------------------------------------------------------
void CREG_VAR::SaveNewModulatorMode(long nBoardNum, long lModType)
{
#ifdef WIN32
	char strKey[256];
	RegistryKey^ Reg;
    char    str1[256];
	gUtility.MySprintf(str1, 256, "SYS_%s", gpConfig->gszInstalledBoard_Info[nBoardNum]);
	gUtility.MySprintf(strKey, 256, "Software\\TELEVIEW\\TeleviewSystem\\%s", str1);

	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKey));

	if(!Reg)
	{
		return;
	}
	gUtility.Set_Registry_Integer_Value(Reg, strKey, "ModulationType", lModType);

	Reg->Close();

	//kslee 2010/4/19
    gUtility.MySprintf(strKey, 256, ".DEFAULT\\Software\\TELEVIEW\\TeleviewSystem\\%s", str1);
	try{
		Reg = Registry::Users->CreateSubKey(gcnew String(strKey));	
		gUtility.Set_Registry_Integer_Value(Reg, strKey, "ModulationType", lModType);
		Reg->Close();
	}
	catch(Exception^ e)
	{

		e->ToString();
	}

 
    delete Reg;
#else
    char    strKeyStartup[256];
    char    str1[256];

	FILE	*pFileStart = NULL;
	FILE	*Reg;
//2011/6/29
#ifdef STANDALONE
	gUtility.MySprintf(strKeyStartup, 256, (char *) "/sysdb/startupMod_%d.cfg", nBoardNum);
#else
	gUtility.MySprintf(strKeyStartup, 256, (char *) "./config/startupMod_%d.cfg", nBoardNum);
#endif
	pFileStart = fopen(strKeyStartup, "w");
	Reg = pFileStart;
    gUtility.Set_Registry_Integer_Value(Reg, strKeyStartup, (char *) "TVB380ModulationMode", lModType);
    
	if (pFileStart)
		fclose(pFileStart);
#endif
}
//===============================================================
// Check the Range of Variables
//===============================================================
void CREG_VAR::CheckGlobalVarValidity(long nBoardNum)
{
    //--------------------------------------------------------------------------
    // Create Folder. Not exist --> set current dir
    //gUtility.Create_Directory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
    //if (!gUtility.Is_Directory_Exist(gpConfig->gBC[nBoardNum].gszMasterDirectory))
    //{
    //    GetCurrentDirectory(256,gpConfig->gBC[nBoardNum].gszMasterDirectory);
    //}

    //--------------------------------------------------------------------------
    //--- # of SubBank should be 0~7 : Use 1M ~ 8M at each 8M memory
    if (gpConfig->gBC[nBoardNum].gnSubBankNumber > MAX_BANK_COUNT)
        gpConfig->gBC[nBoardNum].gnSubBankNumber = MAX_BANK_COUNT;
    else if (gpConfig->gBC[nBoardNum].gnSubBankNumber <= MIN_BANK_COUNT)
        gpConfig->gBC[nBoardNum].gnSubBankNumber = MAX_BANK_COUNT;

    //--------------------------------------------------------------------------
    //--- Bank offset should be 1~1024: gnSubBankOffset(nBoardNum) * 1K memory in each 1M subbank
    if (gpConfig->gBC[nBoardNum].gnSubBankOffset < MIN_BANK_OFFSET)
        gpConfig->gBC[nBoardNum].gnSubBankOffset = MIN_BANK_OFFSET;
    else if (gpConfig->gBC[nBoardNum].gnSubBankOffset > MAX_BANK_OFFSET)
        gpConfig->gBC[nBoardNum].gnSubBankOffset = MAX_BANK_OFFSET;

    //--------------------------------------------------------------------------
    //--- Modulator RF Output Frequency Should be 52M~1.750G

    //---TVB380V4, V6.1, TVB390V7, V8
    if (gpConfig->gBC[nBoardNum].gnBoardId == 41 ||
        gpConfig->gBC[nBoardNum].gnBoardId == 42 ||
        gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if ( (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
            (gpConfig->gBC[nBoardNum].gnModulatorMode  == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode  == DVB_S2))	//2013/5/27 TVB599 0xC
        {
        } else
        {
            if (gpConfig->gBC[nBoardNum].gnRFOutFreq < 52000000)
                gpConfig->gBC[nBoardNum].gnRFOutFreq = 52000000;
        }
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 43)    // TVB370V6
    {
        if (gpConfig->gBC[nBoardNum].gnRFOutFreq > 85000000)
            gpConfig->gBC[nBoardNum].gnRFOutFreq = 85000000;
        else if (gpConfig->gBC[nBoardNum].gnRFOutFreq < 52000000)
            gpConfig->gBC[nBoardNum].gnRFOutFreq = 52000000;
    } else  // TVB380V3
    {
        if (gpConfig->gBC[nBoardNum].gnRFOutFreq > 1000000000)
            gpConfig->gBC[nBoardNum].gnRFOutFreq = 1000000000;
        else if (gpConfig->gBC[nBoardNum].gnRFOutFreq < 470000000)
            gpConfig->gBC[nBoardNum].gnRFOutFreq = 470000000;
    }
    
    //--------------------------------------------------------------------------
    //--- Symbolrate
    if (gpConfig->gBC[nBoardNum].gnModulatorMode  == DVB_S2)
    {
        if (gpConfig->gBC[nBoardNum].gnSymbolRate > 45000000)
            gpConfig->gBC[nBoardNum].gnSymbolRate = 45000000;
        else if (gpConfig->gBC[nBoardNum].gnSymbolRate < 1)
            gpConfig->gBC[nBoardNum].gnSymbolRate = 1;
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnSymbolRate > 90000000)
            gpConfig->gBC[nBoardNum].gnSymbolRate = 90000000;
        else if (gpConfig->gBC[nBoardNum].gnSymbolRate < 1)
            gpConfig->gBC[nBoardNum].gnSymbolRate = 1;
    }

    //--------------------------------------------------------------------------
    //--- Play Rate should be less than 90 Mbps
    if (gpConfig->gBC[nBoardNum].gdwPlayRate > 90000000)
        /*gpConfig->gBC[nBoardNum].gdwPlayRate = 90000000*/;
    else if (gpConfig->gBC[nBoardNum].gdwPlayRate < 1)
        gpConfig->gBC[nBoardNum].gdwPlayRate = 1;

    //--------------------------------------------------------------------------
    //--- Modulation Mode should be 0:DVB-T, 1:VSB, 2:QAM-A, 3:QAM-B, 4:QPSK, 5:TDMB, 6:16VSB, 7:DVB-T, 8:DVB-S2, 9:ISDB-T
    if (gpConfig->gBC[nBoardNum].gnModulatorMode >= MAX_MODULATORMODE)
        gpConfig->gBC[nBoardNum].gnModulatorMode = MAX_MODULATORMODE - 1;
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode < DVB_T)
        gpConfig->gBC[nBoardNum].gnModulatorMode = DVB_T;

    //--------------------------------------------------------------------------
    //--- Code Rate should be 0~4 (1/2,2/3,3/4,5/6,7/8 in QPSK
    //--- Code Rate should be 0~7 (1/2,3/5,2/3,3/4,4/5,5/6,8/9,9/10)
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate < 0)
            gpConfig->gBC[nBoardNum].gnCodeRate = 0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate > 10)
            gpConfig->gBC[nBoardNum].gnCodeRate = 10;
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate < CONST_DTMB_CODE_7488_3008)
            gpConfig->gBC[nBoardNum].gnCodeRate = CONST_DTMB_CODE_7488_3008;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate > CONST_DTMB_CODE_7488_6016)
            gpConfig->gBC[nBoardNum].gnCodeRate = CONST_DTMB_CODE_7488_6016;
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate < 0)
            gpConfig->gBC[nBoardNum].gnCodeRate = 0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate > 4)
            gpConfig->gBC[nBoardNum].gnCodeRate = 4;
    }
    
    //--------------------------------------------------------------------------
    // --- Bandwidth should be 0(6MHz),1(7MHz),2(8MHz), 3(5MHz)
	//DVB-T2 2010/3/25
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
        if (gpConfig->gBC[nBoardNum].gnBandwidth < DVBT2_BW_1Dot7MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = DVBT2_BW_8MHZ;
        else if (gpConfig->gBC[nBoardNum].gnBandwidth > DVBT2_BW_8MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = DVBT2_BW_8MHZ;
	}
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
    {
        if (gpConfig->gBC[nBoardNum].gnBandwidth < DVBH_BW_5MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = DVBH_BW_8MHZ;
        else if (gpConfig->gBC[nBoardNum].gnBandwidth > DVBH_BW_8MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = DVBH_BW_8MHZ;
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnBandwidth < BW_6MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = BW_8MHZ;
        else if (gpConfig->gBC[nBoardNum].gnBandwidth > BW_8MHZ)
            gpConfig->gBC[nBoardNum].gnBandwidth = BW_8MHZ;
    }

    //--------------------------------------------------------------------------
    // --- Bandwidth should be 0(6MHz),1(7MHz),2(8MHz), 3(5MHz)
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
    {
        if (gpConfig->gBC[nBoardNum].gnTxmode < TX_2K)
            gpConfig->gBC[nBoardNum].gnTxmode = TX_2K;
        else if (gpConfig->gBC[nBoardNum].gnTxmode > TX_4K)
            gpConfig->gBC[nBoardNum].gnTxmode = TX_4K;
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnTxmode < TX_2K)
            gpConfig->gBC[nBoardNum].gnTxmode = TX_2K;
        else if (gpConfig->gBC[nBoardNum].gnTxmode > TX_8K)
            gpConfig->gBC[nBoardNum].gnTxmode = TX_8K;
    }

    //------------------------------------------------------------------------------
    //--- Guard Interval should be 0(1/4),1(1/8),2(1/16),3(1/32)
    if (gpConfig->gBC[nBoardNum].gnGuardInterval < GI_1_OF_4)
        gpConfig->gBC[nBoardNum].gnGuardInterval = GI_1_OF_4;
    else if (gpConfig->gBC[nBoardNum].gnGuardInterval > GI_1_OF_32)
        gpConfig->gBC[nBoardNum].gnGuardInterval = GI_1_OF_32;

    //------------------------------------------------------------------------------
    //--- Constellation
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation < CONST_DVB_S2_QPSK)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_DVB_S2_QPSK;
        else if (gpConfig->gBC[nBoardNum].gnConstellation > CONST_DVB_S2_32APSK)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_DVB_S2_32APSK;
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation < CONST_DTMB_4QAM_NR)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_DTMB_4QAM_NR;
        else if (gpConfig->gBC[nBoardNum].gnConstellation > CONST_DTMB_64QAM)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_DTMB_64QAM;
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation < CONST_QPSK)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_QPSK;
        else if (gpConfig->gBC[nBoardNum].gnConstellation > CONST_64QAM)
            gpConfig->gBC[nBoardNum].gnConstellation = CONST_64QAM;
    }

    //------------------------------------------------------------------------------
    //--- QAM Mode
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode < CONST_QAM_A_16)
            gpConfig->gBC[nBoardNum].gnQAMMode = CONST_QAM_A_16;
        else if (gpConfig->gBC[nBoardNum].gnQAMMode > CONST_QAM_A_256)
            gpConfig->gBC[nBoardNum].gnQAMMode = CONST_QAM_A_256;
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode < CONST_QAM_B_64)
            gpConfig->gBC[nBoardNum].gnQAMMode = CONST_QAM_B_64;
        else if (gpConfig->gBC[nBoardNum].gnQAMMode > CONST_QAM_B_256)
            gpConfig->gBC[nBoardNum].gnQAMMode = CONST_QAM_B_256;
    }

    //------------------------------------------------------------------------------
    //--- QAM-B Interleave should be 0(128-1),...,12(128-8)
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMInterleave < INTERLEAVE_128_1_)
            gpConfig->gBC[nBoardNum].gnQAMInterleave = INTERLEAVE_128_1_;
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave > INTERLEAVE_128_8)
            gpConfig->gBC[nBoardNum].gnQAMInterleave = INTERLEAVE_128_8;
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMInterleave < CONST_DTMB_INTERLEAVE_0)
            gpConfig->gBC[nBoardNum].gnQAMInterleave = CONST_DTMB_INTERLEAVE_0;
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave > CONST_DTMB_INTERLEAVE_1)
            gpConfig->gBC[nBoardNum].gnQAMInterleave = CONST_DTMB_INTERLEAVE_1;
    }

    //------------------------------------------------------------------------------
    //--- IF should be 0(36MHz),1(44MHz),2(36.125MHz)
    if (gpConfig->gBC[nBoardNum].gnIFOutFreq < IF_OUT_36MHZ)
        gpConfig->gBC[nBoardNum].gnIFOutFreq = IF_OUT_36MHZ;
    else if (gpConfig->gBC[nBoardNum].gnIFOutFreq > IF_OUT_36_125MHZ)
        gpConfig->gBC[nBoardNum].gnIFOutFreq = IF_OUT_36_125MHZ;

    //------------------------------------------------------------------------------
    //-- Front Input enable/diable
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 41 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if (gpConfig->gBC[nBoardNum].gnUseFrontInput > 1)
            gpConfig->gBC[nBoardNum].gnUseFrontInput = 0;
    }

    //------------------------------------------------------------------------------
    //--- PRBS mode should be 0(none),1(2^7-1),2(2^10-1),3(2^15-1),4(2^23-1)
    if (gpConfig->gBC[nBoardNum].gnPRBSmode < PRBS_MODE_NONE)
        gpConfig->gBC[nBoardNum].gnPRBSmode = PRBS_MODE_NONE;
    else if (gpConfig->gBC[nBoardNum].gnPRBSmode > PRBS_MODE_2_EXP_23_1)
        gpConfig->gBC[nBoardNum].gnPRBSmode = PRBS_MODE_2_EXP_23_1;

    //------------------------------------------------------------------------------
    //--- PRBS scale should be from 0 to 32767 if TDMB or from 0 to 8191
    if (gpConfig->gBC[nBoardNum].gnPRBSscale < MIN_CNR)
        gpConfig->gBC[nBoardNum].gnPRBSscale = MIN_CNR;
    else if (gpConfig->gBC[nBoardNum].gnPRBSscale > MAX_CNR)
        gpConfig->gBC[nBoardNum].gnPRBSscale = MAX_CNR;

    //------------------------------------------------------------------------------
    //--- Atten. should be from MIN_ATTEN to MAX_ATTEN
    if (gpConfig->gBC[nBoardNum].gdwAttenVal < MIN_ATTEN)
        gpConfig->gBC[nBoardNum].gdwAttenVal = MIN_ATTEN;
	//AGC - RF Level -> Atten/AGC
    //else if (gpConfig->gBC[nBoardNum].gdwAttenVal > MAX_ATTEN)
    //    gpConfig->gBC[nBoardNum].gdwAttenVal = MAX_ATTEN;
	else if(gpConfig->gBC[nBoardNum].gdwAttenVal > MAX_ATTEN + MAX_EXT_ATTEN)
		gpConfig->gBC[nBoardNum].gdwAttenVal = MAX_ATTEN + MAX_EXT_ATTEN;

    if (gpConfig->gBC[nBoardNum].gdwAttenExtVal < 0)
        gpConfig->gBC[nBoardNum].gdwAttenExtVal = 0;
    else if (gpConfig->gBC[nBoardNum].gdwAttenExtVal > MAX_EXT_ATTEN)
        gpConfig->gBC[nBoardNum].gdwAttenExtVal = MAX_EXT_ATTEN;

    //------------------------------------------------------------------------------
    // DVB-H
    if (gpConfig->gBC[nBoardNum].gnMPE_FEC < 0)
        gpConfig->gBC[nBoardNum].gnMPE_FEC = 0;
    else if (gpConfig->gBC[nBoardNum].gnMPE_FEC > 1)
        gpConfig->gBC[nBoardNum].gnMPE_FEC = 1;

    if (gpConfig->gBC[nBoardNum].gnTime_Slice < 0)
        gpConfig->gBC[nBoardNum].gnTime_Slice = 0;
    else if (gpConfig->gBC[nBoardNum].gnTime_Slice > 1)
        gpConfig->gBC[nBoardNum].gnTime_Slice = 1;

    if (gpConfig->gBC[nBoardNum].gnIn_Depth < 0)
        gpConfig->gBC[nBoardNum].gnIn_Depth = 0;
    else if (gpConfig->gBC[nBoardNum].gnIn_Depth > 1)
        gpConfig->gBC[nBoardNum].gnIn_Depth = 1;

    if (gpConfig->gBC[nBoardNum].gnCell_Id < MIN_CELL_ID)
        gpConfig->gBC[nBoardNum].gnCell_Id = MIN_CELL_ID;
    else if (gpConfig->gBC[nBoardNum].gnCell_Id > MAX_CELL_ID)
        gpConfig->gBC[nBoardNum].gnCell_Id = MAX_CELL_ID;

    if (gpConfig->gBC[nBoardNum].gnPilot < 0)
        gpConfig->gBC[nBoardNum].gnPilot = 0;
    else if (gpConfig->gBC[nBoardNum].gnPilot > 1)
        gpConfig->gBC[nBoardNum].gnPilot = 1;

    //------------------------------------------------------------------------------
    //--- DVB-S2
    if (gpConfig->gBC[nBoardNum].gnRollOffFactor < ROLL_OFF_FACTOR_020)
        gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_020;
    else if (gpConfig->gBC[nBoardNum].gnRollOffFactor > ROLL_OFF_FACTOR_NONE)
        gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_NONE;
	
	if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || /*gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || */gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 27)	//TODO TVB599
	{
		if(gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
			gpConfig->gBC[nBoardNum].gnRollOffFactor = ROLL_OFF_FACTOR_035;
	}

    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
    {
		if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||	gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
		{
			if(gpConfig->gBC[nBoardNum].gnSymbolRate > 45000000)
				gpConfig->gBC[nBoardNum].gnSymbolRate = 45000000;
		}
		else
		{
			if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
			{
				if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_44MHZ)
	            {
		            if (gpConfig->gBC[nBoardNum].gnSymbolRate > 25000000)
			            gpConfig->gBC[nBoardNum].gnSymbolRate = 25000000;
				} else
	            {
		            if (gpConfig->gBC[nBoardNum].gnSymbolRate > 45000000)
			            gpConfig->gBC[nBoardNum].gnSymbolRate = 45000000;
				}
	        } else
		    {
			    if (gpConfig->gBC[nBoardNum].gnSymbolRate > 25000000)
				    gpConfig->gBC[nBoardNum].gnSymbolRate = 25000000;
			}
		}
	}

    //------------------------------------------------------------------------------
    //--- TDMB, CMMB
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
        if (gpConfig->gBC[nBoardNum].gnModulatorSource != FILE_SRC)
            gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;

    //------------------------------------------------------------------------------
    //--- DTMB
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnFrameHeader < CONST_DTMB_FRAME_HEADER_MODE_1)
            gpConfig->gBC[nBoardNum].gnFrameHeader = CONST_DTMB_FRAME_HEADER_MODE_1;
        else if (gpConfig->gBC[nBoardNum].gnFrameHeader > CONST_DTMB_FRAME_HEADER_MODE_3)
            gpConfig->gBC[nBoardNum].gnFrameHeader = CONST_DTMB_FRAME_HEADER_MODE_3;

        if (gpConfig->gBC[nBoardNum].gnCarrierNumber < CONST_DTMB_CARRIER_NUMBER_0)
            gpConfig->gBC[nBoardNum].gnCarrierNumber = CONST_DTMB_CARRIER_NUMBER_0;
        else if (gpConfig->gBC[nBoardNum].gnCarrierNumber > CONST_DTMB_CARRIER_NUMBER_1)
            gpConfig->gBC[nBoardNum].gnCarrierNumber = CONST_DTMB_CARRIER_NUMBER_1;

        if (gpConfig->gBC[nBoardNum].gnFrameHeaderPN < CONST_DTMB_FRAME_HEADER_PN_FIXED)
            gpConfig->gBC[nBoardNum].gnFrameHeaderPN = CONST_DTMB_FRAME_HEADER_PN_FIXED;
        else if (gpConfig->gBC[nBoardNum].gnFrameHeaderPN > CONST_DTMB_FRAME_HEADER_PN_ROTATED)
            gpConfig->gBC[nBoardNum].gnFrameHeaderPN = CONST_DTMB_FRAME_HEADER_PN_ROTATED;

        if (gpConfig->gBC[nBoardNum].gnPilotInsertion < CONST_DTMB_PILOT_INSERTION_OFF)
            gpConfig->gBC[nBoardNum].gnPilotInsertion = CONST_DTMB_PILOT_INSERTION_OFF;
        else if (gpConfig->gBC[nBoardNum].gnPilotInsertion > CONST_DTMB_PILOT_INSERTION_ON)
            gpConfig->gBC[nBoardNum].gnPilotInsertion = CONST_DTMB_PILOT_INSERTION_ON;
    }

    //------------------------------------------------------------------------------
    //--- IP Streaming
    if (gpConfig->gBC[nBoardNum].gnIPStreamingMode < NO_IP_STREAM ||
        gpConfig->gBC[nBoardNum].gnIPStreamingMode >= MAX_IPSTREAMINGMODE)
    {
        gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
    }

    if (gpConfig->gBC[nBoardNum].gnIPStreamingPath < 0 ||
        gpConfig->gBC[nBoardNum].gnIPStreamingPath > 1)
    {
        gpConfig->gBC[nBoardNum].gnIPStreamingPath = 0;
    }

    if (gpConfig->gBC[nBoardNum].gnIPStreamingAccess < 0 ||
        gpConfig->gBC[nBoardNum].gnIPStreamingAccess > 3)
    {
        gpConfig->gBC[nBoardNum].gnIPStreamingAccess = 0;
    }

    if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
        gpConfig->gBC[nBoardNum].gnUseAVDecoding = 0;
    else if (gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1)
        gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;

    //------------------------------------------------------------------------------
    // STOP MODE
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8 ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB)	//Added CMMB
            gpConfig->gBC[nBoardNum].gnStopMode = 0;

    if (gpConfig->gBC[nBoardNum].gnBoardId  < 45 && gpConfig->gBC[nBoardNum].gnBoardId != 10 && gpConfig->gBC[nBoardNum].gnBoardId != 20 && 
		gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 &&
		gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 27 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
        gpConfig->gBC[nBoardNum].gnBertPacketType = NO_BERT_OPERTION;
}

//---------------------------------------------------------------------------
void CREG_VAR::Restore_Factory_Default()
{
    char    strKeyRoot[256];
    char    strKeyStartup[256];
    int     iDefaultVal = -1;

    //---------------------------------------------
    // Crate Registry and set root key
#if defined(WIN32)

	//CRegKey	*Registry = new CRegKey;
	//char	*Registry = NULL;
	RegistryKey^ Reg;
	RegistryKey^ Root_Reg;
    //---------------------------------------------
    // decide slot number
    //---------------------------------------------
    // Remove Current Setting
    gUtility.MySprintf(strKeyStartup, 256, "Software\\TELEVIEW");
    gUtility.MySprintf(strKeyRoot, 256, "Software");
	Root_Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKeyRoot));
	Reg = Registry::CurrentUser->CreateSubKey(gcnew String(strKeyStartup));
	if(!Reg)
		return;

	gUtility.Delete_Registry_Key(Root_Reg, "TELEVIEW");

	Reg->Close();

	gUtility.MySprintf(strKeyStartup, 256, ".DEFAULT\\Software\\TELEVIEW");
    gUtility.MySprintf(strKeyRoot, 256, ".DEFAULT\\Software");
	try{
		Root_Reg = Registry::Users->CreateSubKey(gcnew String(strKeyRoot));
		Reg = Registry::Users->CreateSubKey(gcnew String(strKeyStartup));
		if(!Reg)
			return;
		gUtility.Delete_Registry_Key(Root_Reg, "TELEVIEW");
		Reg->Close();
	}
	catch(Exception^  e)
	{
		e->ToString();
		return;
	}
	delete Reg;

#endif

}

