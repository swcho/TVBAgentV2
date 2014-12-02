#include "stdafx.h"
#ifdef WIN32
#include <stdio.h>
#include "PlayForm.h"
#include "resource.h"
#include "wrapdll.h"
#include "util_ind.h"
#include "util_dep.h"
#include "baseutil.h"
#include "reg_var.h"
#else
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include 	<pthread.h>
#include 	<dirent.h>			// for directory file
#include 	<linux/fs.h> 		// BLKGETSIZE, BLKSSZGET
#include	<math.h>
#include    "variable.h"
#include 	"reg_var.h"
#include 	"wrapdll.h"
#include 	"util_dep.h"
#include 	"util_ind.h"
#include 	"mainutil.h"
#include	"hld_api.h"
#include	"lld_const.h"
//-----------------------------------------------
//2010/4/9 prototype for getting ipaddress
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/if_ether.h>

//---20100811
//TVB597
//#define VER_MAJOR	3
//#define VER_MINOR       0
//#define VER_BILDER      13 
//TVB599
#define VER_MAJOR	3
#define VER_MINOR       1
#define VER_BILDER      4 
//----------------------------------------------------------
int  get_address_strings();
int  get_gateway_address(char *strgw);
//-----------------------------------------------
#endif

extern APPLICATION_CONFIG		*gpConfig;
extern TEL_GENERAL_VAR			gGeneral;
extern CWRAP_DLL				gWrapDll;
extern char						gstrGeneral[256];
extern CUTIL_IND                gUtilInd;
extern CUTILITY                 gUtility;
extern CREG_VAR					gRegVar;

extern char						recv_msg[256];
#ifdef WIN32

using namespace TPG0590VC;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;
using namespace System::IO;
using namespace System::Diagnostics;
#else
extern char recv_msg[256];
extern PlayForm		gGui;

int		giInitialized = 0;
//2012/7/5
int directorySort(const void *cmp1, const void *cmp2);
struct _TSPH_TS_INFO *pTsInfo = NULL;
#endif

//===============================================================================
//---------------------------------------------------------------------------
// called every 100 milisecond
// HMsecTiemrJob: Check Board Status/HLD Status, Display Timer
// KeyInput
#ifdef WIN32
void PlayForm::TimerMain_Process()
#else
void *PlayForm::TimerMain_Process(void *arg)
#endif
{
	if(gGeneral.gnActiveBoard < 0)
#ifdef WIN32
		return;
#else
		return NULL;
#endif
#ifdef WIN32
    HMsecTimerJob();
    TimerKey2_Process();
	Display_UI_VLC_Screen();
#else
	static int iTurn = 0;
	while (giInitialized)
	{
		Sleep(500);
		gGui.Check_590S_Board();
		gGui.HMsecTimerJob();
		iTurn = (iTurn + 1) % 2;
		if (iTurn == 0)
		{
			gGui.TimerSub_Process();		// every seconds
#ifdef STANDALONE
			gGui.Check_mount_usb_sdcard();
#endif
		}	
	}
#endif
}

//---------------------------------------------------------------------------
// called every 1000 milisecond

#ifdef WIN32 
void PlayForm::TimerSub_Process()
{
    long    nBoardNum = gGeneral.gnActiveBoard;
    char    str[100];
    long    nRet, nStatus;
    int     i;
    int     error_status[20];
    long    nCurrentSec;
    __int64    dwTSCount;
    long    dwPlayRate;
	static int  timerCnt = 1;
	if(nBoardNum < 0)
		return;
#ifdef WIN32
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
    {
		gUtility.MySprintf(str, 100, "%d",gpConfig->gBC[nBoardNum].gnRepeatCount);
		gBaseUtil.SetText_Common(Text_Wraps, str);
    }
#endif
	for(nBoardNum = 0; nBoardNum < gGeneral.gnActiveBoardCount; nBoardNum++)
	{
		if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
			continue;
		if ( (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId  == 10 || gpConfig->gBC[nBoardNum].gnBoardId  == 20 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
			gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId  == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	&&
            gpConfig->gBC[nBoardNum].bPlayingProgress == false)  	//2013/5/27 TVB599 0xC
        {
            if ( (gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
                  gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC) &&
                  gpConfig->gBC[nBoardNum].bRecordInProgress == false)
            {
#ifdef STANDALONE
				if(gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC && (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S))
				{
					gGeneral.gnInputRate = -100;
					SNMP_Send_Status(TVB390_TS_IN_RATE);
				
					if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188 && gpConfig->gBC[nBoardNum].gnIsdbT_LoopThru_flag == 0)
					{
						nCurrentSec = gUtilInd.Get_Current_SecTime();
						dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
						if (dwTSCount <= 0)
							return;
				
						if((nCurrentSec - gGeneral.gnLastSec) == 0)
							return;
			
						dwPlayRate = (long) ((double)((double)((double)dwTSCount - (double)gGeneral.gnTSCount[nBoardNum]) * (double)nStatus * 8.0) / (double)((double)((double)nCurrentSec - (double)gGeneral.gnLastSec[nBoardNum]) / 1000.0));
						
	                    gGeneral.gnTSCount[nBoardNum] = dwTSCount;
    	                gGeneral.gnLastSec[nBoardNum] = nCurrentSec;
					
        	            if (gGeneral.gnPlayRate[nBoardNum][0] == 0.0)
            	        {
                	        gGeneral.gnPlayRate[nBoardNum][0] = (double) dwPlayRate;
                    	    return;
	                    } else if (gGeneral.gnPlayRate[nBoardNum][1] == 0.0)
    	                {
        	                gGeneral.gnPlayRate[nBoardNum][1] = (double) dwPlayRate;
            	            return;
                	    } else if (gGeneral.gnPlayRate[nBoardNum][2] == 0.0)
                    	{
                        	gGeneral.gnPlayRate[nBoardNum][2] = (double) dwPlayRate;
                        	return;
          	        	} else if (gGeneral.gnPlayRate[nBoardNum][3] == 0.0)
            	        {
                	        gGeneral.gnPlayRate[nBoardNum][3] = (double) dwPlayRate;
                        	return;
                    	} else if (gGeneral.gnPlayRate[nBoardNum][4] == 0.0)
                    	{
                        	gGeneral.gnPlayRate[nBoardNum][4] = (double) dwPlayRate;
                        	return;
                    	}
                    
                    	if (gGeneral.gnPlayRate[nBoardNum][0] != 0.0 && gGeneral.gnPlayRate[nBoardNum][1] != 0.0 && gGeneral.gnPlayRate[nBoardNum][2] != 0.0 &&
                        	gGeneral.gnPlayRate[nBoardNum][3] != 0.0 && gGeneral.gnPlayRate[nBoardNum][4] != 0.0)
                    	{
                        	dwPlayRate = (long) (gGeneral.gnPlayRate[nBoardNum][0] + gGeneral.gnPlayRate[nBoardNum][1] + gGeneral.gnPlayRate[nBoardNum][2]+
                            	            gGeneral.gnPlayRate[nBoardNum][3] + gGeneral.gnPlayRate[nBoardNum][4]);
                        	dwPlayRate = dwPlayRate/5;
                        
                        	if (dwPlayRate > 1 && dwPlayRate <= 99000000)
	                    	{
								gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate = dwPlayRate;
                        	}
			
							gGeneral.gnPlayRate[nBoardNum][0] = 0.0;
                    		gGeneral.gnPlayRate[nBoardNum][1] = 0.0;
                    		gGeneral.gnPlayRate[nBoardNum][2] = 0.0;
							gGeneral.gnPlayRate[nBoardNum][3] = 0.0;
                    		gGeneral.gnPlayRate[nBoardNum][4] = 0.0;
						}
						Check_ISDBT13_Loopthru_188TS();
					}
				}
				else
#endif
				if( gpConfig->gBC[nBoardNum].gnBoardId == 20/* || gpConfig->gBC[nBoardNum].gnBoardId == 11*/)
				{
					if(gpConfig->gBC[nBoardNum].gnErrorStatus == 100)
					{
						gpConfig->gBC[nBoardNum].gnErrorStatus = 0;
#ifdef WIN32
						gWrapDll.TSPL_RESET_SDCON_EX(nBoardNum);
#else
						TSPL_RESET_SDCON_EX(nBoardNum);
#endif
						Sleep(1);

					}
				}

                nRet = gWrapDll.Read_Input_Status(nBoardNum);
                if (nRet <= 0)
                    return;

				
                nStatus = nRet;

                for (i = 0; i <= 19; i++)
                {
                    error_status[i] = nStatus & 1;
                    nStatus = nStatus/2;
                }

					//2011/6/13 ISDB-S ASI In/Output
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
				{
					int isdbs_Valid = 0;
					{
						if(error_status[19] == 0)
						{
							if(nBoardNum == gGeneral.gnActiveBoard)
								gUtilInd.LogMessage("[DVB ASI]TS is not Framed TS.");
							isdbs_Valid = 0;
#ifdef WIN32
							if(nBoardNum == gGeneral.gnActiveBoard)
								Display_UI_ASI_Lock_or_Unlock(0);	//unlock
							if(gpConfig->gBC[nBoardNum].gnAsiLock_status != 0)
								gpConfig->gBC[nBoardNum].gnBoardStatusReset = 10;
							gpConfig->gBC[nBoardNum].gnAsiLock_status = 0;
							
#endif
						}
						else
						{
							isdbs_Valid = 1;
#ifdef WIN32
							if(nBoardNum == gGeneral.gnActiveBoard)
								Display_UI_ASI_Lock_or_Unlock(1);	//lock
							if(gpConfig->gBC[nBoardNum].gnAsiLock_status != 1)
								gpConfig->gBC[nBoardNum].gnBoardStatusReset = 10;
							gpConfig->gBC[nBoardNum].gnAsiLock_status = 1;
#endif
						}
					}
					if(isdbs_Valid == 1)
					{
                        nCurrentSec = gUtilInd.Get_Current_SecTime();
						dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
                        if (dwTSCount <= 0)
                            return;
                        
						nStatus = 204;
						
						if((nCurrentSec - gGeneral.gnLastSec[nBoardNum]) == 0)
							return;

                        dwPlayRate = (long) ( ((dwTSCount - gGeneral.gnTSCount[nBoardNum]) * nStatus * 8) / (nCurrentSec - gGeneral.gnLastSec[nBoardNum]) );

                        gGeneral.gnTSCount[nBoardNum] = dwTSCount;
                        gGeneral.gnLastSec[nBoardNum] = nCurrentSec;

                        if (gGeneral.gnPlayRate[nBoardNum][0] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][0] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][1] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][1] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][2] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][2] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][3] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][3] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][4] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][4] = (float) dwPlayRate;
                            return;
                        }
                        
                        if (gGeneral.gnPlayRate[nBoardNum][0] != 0.0 && gGeneral.gnPlayRate[nBoardNum][1] != 0.0 && gGeneral.gnPlayRate[nBoardNum][2] != 0.0 &&
                            gGeneral.gnPlayRate[nBoardNum][3] != 0.0 && gGeneral.gnPlayRate[nBoardNum][4] != 0.0)
                        {
                            dwPlayRate = (long) (gGeneral.gnPlayRate[nBoardNum][0] + gGeneral.gnPlayRate[nBoardNum][1] + gGeneral.gnPlayRate[nBoardNum][2]+
                                                 gGeneral.gnPlayRate[nBoardNum][3] + gGeneral.gnPlayRate[nBoardNum][4]);
                            dwPlayRate = dwPlayRate/5;
							if(nBoardNum == gGeneral.gnActiveBoard)
							{
								if (dwPlayRate > 1)
								{
									gUtility.MySprintf(str, 100, "Input bitrate %d bps", dwPlayRate);
									gUtilInd.LogMessage(str);
								} else
									gUtilInd.LogMessageInt(TLV_INPUT_BITRATE);
							}
                            gGeneral.gnPlayRate[nBoardNum][0] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][1] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][2] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][3] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][4] = 0.0;
                        }
					}
				}
                else if (error_status[3] == 0 && gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
                {
					if(nBoardNum == gGeneral.gnActiveBoard)
					{
#ifndef WIN32
						gGeneral.gnInputRate = -1;
						SNMP_Send_Status(TVB390_TS_IN_RATE);
#endif
						gUtilInd.LogMessageInt(TLV_ASI_UNLOCKED);
					}
					gpConfig->gBC[nBoardNum].gnErrorStatus = 3;
#ifdef WIN32
					if(nBoardNum == gGeneral.gnActiveBoard)
						Display_UI_ASI_Lock_or_Unlock(0);	//unlock
					if(gpConfig->gBC[nBoardNum].gnAsiLock_status != 0)
					{
						gWrapDll.Func_TVB59x_SET_TsPacket_CNT_Mode_EX(nBoardNum, 0);
						gpConfig->gBC[nBoardNum].gnAsiLock_status = 0;
						gpConfig->gBC[nBoardNum].gnBoardStatusReset = 10;
					}
					gpConfig->gBC[nBoardNum].gnAsi_Init_Flag = 0;
#endif
				} else if (error_status[4] == 0 && gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC)
                {
					if(gGeneral.gnActiveBoard == nBoardNum)
					{
#ifndef WIN32
						gGeneral.gnInputRate = -1;
						SNMP_Send_Status(TVB390_TS_IN_RATE);
#endif
						gUtilInd.LogMessageInt(TLV_310M_UNLOCKED);
					}
					gpConfig->gBC[nBoardNum].gnErrorStatus = 4;
#ifdef WIN32
					if(nBoardNum == gGeneral.gnActiveBoard)
						Display_UI_ASI_Lock_or_Unlock(0);	//unlock
					if(gpConfig->gBC[nBoardNum].gnAsiLock_status != 0)
					{
						gWrapDll.Func_TVB59x_SET_TsPacket_CNT_Mode_EX(nBoardNum, 0);
						gpConfig->gBC[nBoardNum].gnAsiLock_status = 0;
						gpConfig->gBC[nBoardNum].gnBoardStatusReset = 10;
					}
					gpConfig->gBC[nBoardNum].gnAsi_Init_Flag = 0;
#endif
                } else
                {
					//2011/6/23 fixed
#ifdef WIN32
					if(gpConfig->gBC[nBoardNum].gnAsiLock_status != 1)
						gpConfig->gBC[nBoardNum].gnBoardStatusReset = 10;
					gpConfig->gBC[nBoardNum].gnAsiLock_status = 1;
					if(nBoardNum == gGeneral.gnActiveBoard)
						Display_UI_ASI_Lock_or_Unlock(1);	//unlock
#endif
					if(gGeneral.gnActiveBoard == nBoardNum)
					{
						// Modulator FIFO Full
						if (error_status[13] == 1)
							gUtilInd.LogMessageInt(TLV_FIFO_FULL);
						// Modulator FIFO Empty
						else if (error_status[12] == 1)
							gUtilInd.LogMessageInt(TLV_FIFO_EMPTY);
					}
					if(gpConfig->gBC[nBoardNum].gnBoardId == 20/* || gpConfig->gBC[nBoardNum].gnBoardId == 11*/)
					{
						if(gpConfig->gBC[nBoardNum].gnErrorStatus == 3 || gpConfig->gBC[nBoardNum].gnErrorStatus == 4 || 
							gpConfig->gBC[nBoardNum].gnErrorStatus == 9 || gpConfig->gBC[nBoardNum].gnErrorStatus == 10)
						{
							gpConfig->gBC[nBoardNum].gnErrorStatus = 100;	//board reset should be called
						}

					}
                    //----------- TS Counter
                   // if (gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
                   //     gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
					if(gpConfig->gBC[nBoardNum].gnSupport_ts_Pkt_cnt_cntl == 0 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
                    {
                        nCurrentSec = gUtilInd.Get_Current_SecTime();
                        dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
                        if (dwTSCount <= 0)
                            return;
                        if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
						{
#ifdef WIN32
							if(gWrapDll.TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188)
#else
							if(TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188)
#endif
								nStatus = 188;
							else
								nStatus = 204;
						}
						else
							nStatus = 188;
						if((nCurrentSec - gGeneral.gnLastSec[nBoardNum]) == 0)
							return;
#if 0 //Linux need or not need ?
						if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && nStatus == 188)
						{
							Check_ISDBT13_Loopthru_188TS();
						}
#endif

                        dwPlayRate = (long) ( ((dwTSCount - gGeneral.gnTSCount[nBoardNum]) * nStatus * 8) / (nCurrentSec - gGeneral.gnLastSec[nBoardNum]) );

                        gGeneral.gnTSCount[nBoardNum] = dwTSCount;
                        gGeneral.gnLastSec[nBoardNum] = nCurrentSec;

                        if (gGeneral.gnPlayRate[nBoardNum][0] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][0] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][1] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][1] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][2] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][2] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][3] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][3] = (float) dwPlayRate;
                            return;
                        } else if (gGeneral.gnPlayRate[nBoardNum][4] == 0.0)
                        {
                            gGeneral.gnPlayRate[nBoardNum][4] = (float) dwPlayRate;
                            return;
                        }
                        
                        if (gGeneral.gnPlayRate[nBoardNum][0] != 0.0 && gGeneral.gnPlayRate[nBoardNum][1] != 0.0 && gGeneral.gnPlayRate[nBoardNum][2] != 0.0 &&
                            gGeneral.gnPlayRate[nBoardNum][3] != 0.0 && gGeneral.gnPlayRate[nBoardNum][4] != 0.0)
                        {
                            dwPlayRate = (long) (gGeneral.gnPlayRate[nBoardNum][0] + gGeneral.gnPlayRate[nBoardNum][1] + gGeneral.gnPlayRate[nBoardNum][2]+
                                                 gGeneral.gnPlayRate[nBoardNum][3] + gGeneral.gnPlayRate[nBoardNum][4]);
                            dwPlayRate = dwPlayRate/5;
                            
                            if (dwPlayRate > 1 && dwPlayRate <= 99000000)
		                    {
								//2011/11/10 ISDB-T 13seg 188 TS Loopthru ===============
				                if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
								{
									gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate = dwPlayRate;
								}
                                //========================================================
				                if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
								{
									gUtility.MySprintf(str, 100, "Input bitrate %d bps, Packet Size : %d bytes", dwPlayRate, nStatus);
								}
								else
								{
									gUtility.MySprintf(str, 100, "Input bitrate %d bps", dwPlayRate);
								}
								//ATSC-M/H kslee 2010/2/3
								if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
								{
#ifdef WIN32
									gUtility.MySprintf(str, 100, "Input bitrate %d bps, MHE packet PID : 0x%x", dwPlayRate, 
														gWrapDll.TSPH_GET_MHE_PACKET_INFO(nBoardNum, "", 1));				
#else
									gUtility.MySprintf(str, 100, (char *)"Input bitrate %d bps, MHE packet PID : 0x%x", dwPlayRate, 
														TSPH_GET_MHE_PACKET_INFO(nBoardNum, (char *)"", 1));				
#endif
								}
								if(gGeneral.gnActiveBoard == nBoardNum)
								{
									gUtilInd.LogMessage(str);
#ifndef WIN32
									gGeneral.gnInputRate = dwPlayRate;
#endif
								}
                            } else
							{
								if(gGeneral.gnActiveBoard == nBoardNum)
									gUtilInd.LogMessageInt(TLV_INPUT_BITRATE);
							}
                            
                            gGeneral.gnPlayRate[nBoardNum][0] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][1] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][2] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][3] = 0.0;
                            gGeneral.gnPlayRate[nBoardNum][4] = 0.0;
                        }
#ifdef WIN32
#else
						SNMP_Send_Status(TVB390_TS_IN_RATE);
#endif
                    }
#ifdef WIN32
					else
					{
						int nInputBitrate, nPacketCnt;
						int nOutputBitrate;;
						unsigned int bufferDataSize, ndummy;
						char debug_string[256];
						nInputBitrate = gWrapDll.Func_TVB59x_Get_Asi_Input_rate_EX(nBoardNum, &nPacketCnt, &nOutputBitrate);
						if(gGeneral.gnActiveBoard == nBoardNum && timerCnt == 9)
						{
							gUtility.MySprintf(str, 100, "Input bitrate %d bps", nInputBitrate);
							gUtilInd.LogMessage(str);
						}
						
#if 0
						if(gpConfig->gBC[nBoardNum].gnAsi_Init_Flag == 5)
						{
							if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
							{
								if(gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_1Dot7MHZ)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = ((131. / 71.) * 8. * 1000000.); 
								}
								else if(gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_5MHZ)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = (5.*8.*(8./7.) * 1000000.);
								}
								else if(gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_6MHZ)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = (6.*8.*(8./7.) * 1000000.);
								}
								else if(gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_7MHZ)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = (7.*8.*(8./7.) * 1000000.);
								}
								else if(gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_8MHZ)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = (8.*8.*(8./7.) * 1000000.);
								}
								else
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = (10.*8.*(8./7.) * 1000000.);
								}
							}
							gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate = 0;
							gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate = 0;
							gpConfig->gBC[nBoardNum].gnAsi_InOut_Count = 0;
							gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Max = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock + (gpConfig->gBC[nBoardNum].gnAsi_SymbolClock / 100000);
							gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Min = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock - (gpConfig->gBC[nBoardNum].gnAsi_SymbolClock / 100000);
							gpConfig->gBC[nBoardNum].gnAsi_Init_Flag++;
						}
						else if(gpConfig->gBC[nBoardNum].gnAsi_Init_Flag < 5)
						{
							gpConfig->gBC[nBoardNum].gnAsi_Init_Flag++;
						}
						else
						{
							gWrapDll.TSPH_IP_RECV_STATUS(nBoardNum, &bufferDataSize, &ndummy);
							//gUtility.MySprintf(debug_string, 256, "BufferSize: %d\n", bufferDataSize);
							//OutputDebugString(debug_string);
							if(bufferDataSize == 0)
								return;
							if((nInputBitrate + 1000000) > nOutputBitrate)
							{
								gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate = gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate + nInputBitrate;
								gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate = gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate + nOutputBitrate;
								gpConfig->gBC[nBoardNum].gnAsi_InOut_Count++;
							}
							if(bufferDataSize <= (0x100000 * 10) && (gpConfig->gBC[nBoardNum].gnAsi_SymbolClock > gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Min))
							{
								nInputBitrate = (int)(gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate / gpConfig->gBC[nBoardNum].gnAsi_InOut_Count);
								nOutputBitrate = (int)(gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate / gpConfig->gBC[nBoardNum].gnAsi_InOut_Count);
								if((gpConfig->gBC[nBoardNum].gnAsi_SymbolClock - ((nOutputBitrate - nInputBitrate) * 2)) > gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Min)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock - ((nOutputBitrate - nInputBitrate) * 2);
								}
								else
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Min;
								}
								gUtility.MySprintf(debug_string, 256, "in[%d],out[%d],symClk[%d]\n", nInputBitrate, nOutputBitrate, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								OutputDebugString(debug_string);
								//gWrapDll.Func_TVB59x_Set_Symbol_Clock_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate = 0;
								gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate = 0;
								gpConfig->gBC[nBoardNum].gnAsi_InOut_Count = 0;
							}
							else if(bufferDataSize >= (0x100000 * 20) && (gpConfig->gBC[nBoardNum].gnAsi_SymbolClock < gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Max))
							{
								nInputBitrate = (int)(gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate / gpConfig->gBC[nBoardNum].gnAsi_InOut_Count);
								nOutputBitrate = (int)(gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate / gpConfig->gBC[nBoardNum].gnAsi_InOut_Count);
								if((gpConfig->gBC[nBoardNum].gnAsi_SymbolClock + ((nInputBitrate - nOutputBitrate) * 2)) > gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Max)
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock + ((nInputBitrate - nOutputBitrate) * 2);
								}
								else
								{
									gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock_Min;
								}
								gUtility.MySprintf(debug_string, 256, "in[%d],out[%d],symClk[%d]\n", nInputBitrate, nOutputBitrate, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								OutputDebugString(debug_string);
								//gWrapDll.Func_TVB59x_Set_Symbol_Clock_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								gpConfig->gBC[nBoardNum].gnAsi_IN_Bitrate = 0;
								gpConfig->gBC[nBoardNum].gnAsi_OUT_Bitrate = 0;
								gpConfig->gBC[nBoardNum].gnAsi_InOut_Count = 0;
							}
						}

						if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK)
						{
							if(gpConfig->gBC[nBoardNum].gnAsi_Init_Flag == 10)
							{
					            long iBitrate = dwPlayRate;
								double dwSymRate;
								long tmp_Symrate;
								long nSymRate;
                                if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
                                    dwSymRate = ((double)((double)iBitrate / 2.0) * (204.0 / 188.0) /(1.0/2.0));
                                else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
                                    dwSymRate = (((double)iBitrate / 2.0) * (204.0 / 188.0) / (2.0/3.0));
                                else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
                                    dwSymRate = (((double)iBitrate / 2.0) * (204.0 / 188.0) / (3.0/4.0));
                                else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
                                    dwSymRate = (((double)iBitrate / 2.0) * (204.0 / 188.0) / (5.0/6.0));
                                else 
                                    dwSymRate = (((double)iBitrate / 2.0) * (204.0 / 188.0) / (7.0/8.0));
					
								nSymRate = (long)dwSymRate;
								//tmp_Symrate = nSymRate / 1000;
								//tmp_Symrate = nSymRate - (tmp_Symrate * 1000);
								//if(tmp_Symrate >= 500)
								//	nSymRate = (((nSymRate / 1000) + 1) * 1000);
								//else
								//	nSymRate = ((nSymRate / 1000) * 1000);

								this->Display_SymbolRate(nSymRate);
								
								//gWrapDll.Func_TVB59x_Modulator_Status_Control_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 0, 1);

								gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = nSymRate;
								gpConfig->gBC[nBoardNum].gnAsi_Init_Flag++;
							}
							else if(gpConfig->gBC[nBoardNum].gnAsi_Init_Flag < 20)
								gpConfig->gBC[nBoardNum].gnAsi_Init_Flag++;
							else
							{
								gpConfig->gBC[nBoardNum].gnAsi_SymbolClock = gpConfig->gBC[nBoardNum].gnAsi_SymbolClock + dwPlayRate - delta_clockCnt;
								//gWrapDll.Func_TVB59x_Set_Symbol_Clock_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								gUtility.MySprintf(debug_string, 256, "in[%d],out[%d],symClk[%d]\n", dwPlayRate, delta_clockCnt, gpConfig->gBC[nBoardNum].gnAsi_SymbolClock);
								OutputDebugString(debug_string);
							}
						}
#endif
					}
#endif
		        }
			}
			else
			{
				gpConfig->gBC[nBoardNum].gnAsi_Init_Flag = 0;
			}
		}
	}
	if((++timerCnt) == 10)
		timerCnt = 0;
}
#else
void PlayForm::TimerSub_Process()
{
    long    nBoardNum = gGeneral.gnActiveBoard;
    char    str[100];
    long    nRet, nStatus;
    int     i;
    int     error_status[20];
    long    nCurrentSec;
    __int64    dwTSCount;
    long    dwPlayRate;
    if ( (gpConfig->gBC[nBoardNum].gnBoardId >= 45 || gpConfig->gBC[nBoardNum].gnBoardId  == 10 || gpConfig->gBC[nBoardNum].gnBoardId  == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId  == 12 || gpConfig->gBC[nBoardNum].gnBoardId  == 16)	/* 2012/1/31 TVB591S */  &&
        gpConfig->gBC[nBoardNum].bPlayingProgress == false)  	//2011/2/15 added 11(TVB597V2)// '45,47,48,59,60,10(595D),20(590S)
    {
        if ( (gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
              gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC) &&
              gpConfig->gBC[nBoardNum].bRecordInProgress == false)
        {
	    //kslee 2010/5/14
#ifdef STANDALONE
			if(gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC && (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S))
			{
				gGeneral.gnInputRate = -100;
				SNMP_Send_Status(TVB390_TS_IN_RATE);
				
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188 && gpConfig->gBC[nBoardNum].gnIsdbT_LoopThru_flag == 0)
				{
					nCurrentSec = gUtilInd.Get_Current_SecTime();
					dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
					if (dwTSCount <= 0)
						return;
			
					if((nCurrentSec - gGeneral.gnLastSec) == 0)
						return;
			
					dwPlayRate = (long) ((double)((double)((double)dwTSCount - (double)gGeneral.gnTSCount) * (double)nStatus * 8.0) / (double)((double)((double)nCurrentSec - (double)gGeneral.gnLastSec) / 1000.0));
						
                    gGeneral.gnTSCount = dwTSCount;
                    gGeneral.gnLastSec = nCurrentSec;
					
                    if (gGeneral.gnPlayRate[0] == 0.0)
                    {
                        gGeneral.gnPlayRate[0] = (double) dwPlayRate;
                        return;
                    } else if (gGeneral.gnPlayRate[1] == 0.0)
                    {
                        gGeneral.gnPlayRate[1] = (double) dwPlayRate;
                        return;
                    } else if (gGeneral.gnPlayRate[2] == 0.0)
                    {
                        gGeneral.gnPlayRate[2] = (double) dwPlayRate;
                        return;
                    } else if (gGeneral.gnPlayRate[3] == 0.0)
                    {
                        gGeneral.gnPlayRate[3] = (double) dwPlayRate;
                        return;
                    } else if (gGeneral.gnPlayRate[4] == 0.0)
                    {
                        gGeneral.gnPlayRate[4] = (double) dwPlayRate;
                        return;
                    }
                    
                    if (gGeneral.gnPlayRate[0] != 0.0 && gGeneral.gnPlayRate[1] != 0.0 && gGeneral.gnPlayRate[2] != 0.0 &&
                        gGeneral.gnPlayRate[3] != 0.0 && gGeneral.gnPlayRate[4] != 0.0)
                    {
                        dwPlayRate = (long) (gGeneral.gnPlayRate[0] + gGeneral.gnPlayRate[1] + gGeneral.gnPlayRate[2]+
                                        gGeneral.gnPlayRate[3] + gGeneral.gnPlayRate[4]);
                        dwPlayRate = dwPlayRate/5;
                        
                        if (dwPlayRate > 1 && dwPlayRate <= 99000000)
	                    {
							gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate = dwPlayRate;
                        }
			
						gGeneral.gnPlayRate[0] = 0.0;
                    	gGeneral.gnPlayRate[1] = 0.0;
                    	gGeneral.gnPlayRate[2] = 0.0;
						gGeneral.gnPlayRate[3] = 0.0;
                    	gGeneral.gnPlayRate[4] = 0.0;
					}
					Check_ISDBT13_Loopthru_188TS();
				}
			}
			else
#endif
			{
				//kslee 2010/4/16 TVB590S V2
				if( gpConfig->gBC[nBoardNum].gnBoardId == 20)
				{
					if(gpConfig->gBC[nBoardNum].gnErrorStatus == 100)
					{
						gpConfig->gBC[nBoardNum].gnErrorStatus = 0;
						TSPL_RESET_SDCON_EX(nBoardNum);
						Sleep(1);
					}
				}
                nRet = gWrapDll.Read_Input_Status(nBoardNum);
                if (nRet <= 0)
                    return;

                nStatus = nRet;

                for (i = 0; i <= 19; i++)
                {
                    error_status[i] = nStatus & 1;
                    nStatus = nStatus/2;
                }

				int isdbs_Valid = 0;
					//2011/6/13 ISDB-S ASI In/Output
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
				{
					isdbs_Valid = 0;
					if(error_status[19] == 0)
					{
						isdbs_Valid = 0;
					}
					else
					{
						isdbs_Valid = 1;
    				}
    				if(isdbs_Valid == 1)
					{
                        nCurrentSec = gUtilInd.Get_Current_SecTime();
						dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
                        if (dwTSCount <= 0)
                            return;
                        
    					nStatus = 204;
    					
						//kslee
						if((nCurrentSec - gGeneral.gnLastSec) == 0)
							return;

	                    //dwPlayRate = (long) ( ((dwTSCount - gGeneral.gnTSCount) * nStatus * 8) / (nCurrentSec - gGeneral.gnLastSec) );
						dwPlayRate = (long) ((double)((double)((double)dwTSCount - (double)gGeneral.gnTSCount) * (double)nStatus * 8.0) / (double)((double)((double)nCurrentSec - (double)gGeneral.gnLastSec) / 1000.0));
	                    
						gGeneral.gnTSCount = dwTSCount;
	                    gGeneral.gnLastSec = nCurrentSec;
	
	                    if (gGeneral.gnPlayRate[0] == 0.0)
	                    {
	                        gGeneral.gnPlayRate[0] = (double) dwPlayRate;
	                        return;
	                    }
						else if (gGeneral.gnPlayRate[1] == 0.0)
	                    {
	                        gGeneral.gnPlayRate[1] = (double) dwPlayRate;
	                        return;
	                    }
						else if (gGeneral.gnPlayRate[2] == 0.0)
	                    {
	                        gGeneral.gnPlayRate[2] = (double) dwPlayRate;
	                        return;
	                    }
						else if (gGeneral.gnPlayRate[3] == 0.0)
                        {
                            gGeneral.gnPlayRate[3] = (double) dwPlayRate;
                            return;
			            } 
						else if (gGeneral.gnPlayRate[4] == 0.0)
                        {
							gGeneral.gnPlayRate[4] = (double) dwPlayRate;
                            return;
                        }
                        gGeneral.gnInputRate = 0;
                        if (gGeneral.gnPlayRate[0] != 0.0 && gGeneral.gnPlayRate[1] != 0.0 && gGeneral.gnPlayRate[2] != 0.0 &&
                            gGeneral.gnPlayRate[3] != 0.0 && gGeneral.gnPlayRate[4] != 0.0)
				        {
	                        dwPlayRate = (long) (gGeneral.gnPlayRate[0] + gGeneral.gnPlayRate[1] + gGeneral.gnPlayRate[2]+
	                                             gGeneral.gnPlayRate[3] + gGeneral.gnPlayRate[4]);
	                        dwPlayRate = dwPlayRate/5;
	                        
	                        if (dwPlayRate > 1)
	                        {
	                            gUtility.MySprintf(str, 100, (char *)"Input bitrate %d bps", dwPlayRate);
	                            
								gUtilInd.LogMessage(str);
								gGeneral.gnInputRate = dwPlayRate;
	                        }
						
							else
	                            gUtilInd.LogMessageInt(TLV_INPUT_BITRATE);
	                        gGeneral.gnPlayRate[0] = 0.0;
	                        gGeneral.gnPlayRate[1] = 0.0;
	                        gGeneral.gnPlayRate[2] = 0.0;
	                        gGeneral.gnPlayRate[3] = 0.0;
	                        gGeneral.gnPlayRate[4] = 0.0;
	                    }
					}
				}
				else if (error_status[3] == 0 && gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
				{
					gGeneral.gnInputRate = -1;
					SNMP_Send_Status(TVB390_TS_IN_RATE);
                    gUtilInd.LogMessageInt(TLV_ASI_UNLOCKED);
					gpConfig->gBC[nBoardNum].gnErrorStatus = 3;

				}
				else if (error_status[4] == 0 && gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC)
				{
					gGeneral.gnInputRate = -1;
					SNMP_Send_Status(TVB390_TS_IN_RATE);
                    gUtilInd.LogMessageInt(TLV_310M_UNLOCKED);
					gpConfig->gBC[nBoardNum].gnErrorStatus = 4;
				}
				else
				{
					// Modulator FIFO Full
		            if (error_status[13] == 1)
                        gUtilInd.LogMessageInt(TLV_FIFO_FULL);
					// Modulator FIFO Empty
					else if (error_status[12] == 1)
                        gUtilInd.LogMessageInt(TLV_FIFO_EMPTY);
					if(gpConfig->gBC[nBoardNum].gnBoardId == 20)
					{
						if(gpConfig->gBC[nBoardNum].gnErrorStatus == 3 || gpConfig->gBC[nBoardNum].gnErrorStatus == 4 || 
							gpConfig->gBC[nBoardNum].gnErrorStatus == 9 || gpConfig->gBC[nBoardNum].gnErrorStatus == 10)
						{
							gpConfig->gBC[nBoardNum].gnErrorStatus = 100;	//board reset should be called
						}

					}
						//----------- TS Counter
					if (gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
						gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
		            {
				        nCurrentSec = gUtilInd.Get_Current_SecTime();
					    dwTSCount = gWrapDll.Read_Input_Tscount_Ex(nBoardNum);
						if (dwTSCount <= 0)
							return;
						if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
						{
							if(TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188)
								nStatus = 188;
							else
								nStatus = 204;
						}
						else
							nStatus = 188;

						if((nCurrentSec - gGeneral.gnLastSec) == 0)
							return;

						if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 && nStatus == 188)
						{
							Check_ISDBT13_Loopthru_188TS();
						}
						
						dwPlayRate = (long) ((double)((double)((double)dwTSCount - (double)gGeneral.gnTSCount) * (double)nStatus * 8.0) / (double)((double)((double)nCurrentSec - (double)gGeneral.gnLastSec) / 1000.0));

                   		gGeneral.gnTSCount = dwTSCount;
                   		gGeneral.gnLastSec = nCurrentSec;
						
                   		if (gGeneral.gnPlayRate[0] == 0.0)
                   		{
                   		    gGeneral.gnPlayRate[0] = (double) dwPlayRate;
                   		    return;
                   		} else if (gGeneral.gnPlayRate[1] == 0.0)
                   		{
                   		    gGeneral.gnPlayRate[1] = (double) dwPlayRate;
                   		    return;
                   		} else if (gGeneral.gnPlayRate[2] == 0.0)
                   		{
                   		    gGeneral.gnPlayRate[2] = (double) dwPlayRate;
                   		    return;
                   		} else if (gGeneral.gnPlayRate[3] == 0.0)
                   		{
                   		    gGeneral.gnPlayRate[3] = (double) dwPlayRate;
                   		    return;
                   		} else if (gGeneral.gnPlayRate[4] == 0.0)
                   		{
                   		    gGeneral.gnPlayRate[4] = (double) dwPlayRate;
                   		    return;
                   		}
                   		
						gGeneral.gnInputRate = 0;
						
                   		if (gGeneral.gnPlayRate[0] != 0.0 && gGeneral.gnPlayRate[1] != 0.0 && gGeneral.gnPlayRate[2] != 0.0 &&
                   		    gGeneral.gnPlayRate[3] != 0.0 && gGeneral.gnPlayRate[4] != 0.0)
                   		{
                   		    dwPlayRate = (long) (gGeneral.gnPlayRate[0] + gGeneral.gnPlayRate[1] + gGeneral.gnPlayRate[2]+
                                            gGeneral.gnPlayRate[3] + gGeneral.gnPlayRate[4]);
                   		    dwPlayRate = dwPlayRate/5;
                   		    
                   		    if (dwPlayRate > 1 && dwPlayRate <= 99000000)
	               		    {
								//2011/11/10 ISDB-T 13seg 188 TS Loopthru ===============
			       		        if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
								{
									gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate = dwPlayRate;
								}
                   		        //========================================================
						
						
			       		        if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
								{
									gUtility.MySprintf(str, 100, (char *)"Input bitrate %d bps, Packet Size : %d bytes", dwPlayRate, nStatus);
								}
								else
								{
									gUtility.MySprintf(str, 100, (char *)"Input bitrate %d bps", dwPlayRate);
								}
								//ATSC-M/H kslee 2010/2/3
								if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
								{
									gUtility.MySprintf(str, 100, (char *)"Input bitrate %d bps, MHE packet PID : 0x%x", dwPlayRate, 
														TSPH_GET_MHE_PACKET_INFO(nBoardNum, (char *)"", 1));				
								}
                   		        gUtilInd.LogMessage(str);
								gGeneral.gnInputRate = dwPlayRate;
						
                   		    }
                       		gGeneral.gnPlayRate[0] = 0.0;
                       		gGeneral.gnPlayRate[1] = 0.0;
                       		gGeneral.gnPlayRate[2] = 0.0;
							gGeneral.gnPlayRate[3] = 0.0;
                       		gGeneral.gnPlayRate[4] = 0.0;
						}
						SNMP_Send_Status(TVB390_TS_IN_RATE);
					}                               
				}
			}
		}
    }
}
#endif

//2011/11/10 ISDB-T 13seg 188 TS LoopThru +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32
int PlayForm::Check_ISDBT13_Loopthru_188TS()
{
	int nBoardNum = gGeneral.gnActiveBoard;
	int cnt = 0;
	int nRet = -1;
	if(gWrapDll.TSPH_IS_LOOPTHRU_ISDBT13_188(nBoardNum) == 188)
	{
		HCURSOR hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
		if(hCursor)
			::SetCursor(hCursor);
		gUtilInd.LogMessageInt(TLV_BUFERING);
		gWrapDll.TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(nBoardNum);

		while(1)
		{
			nRet = gWrapDll.TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(nBoardNum); 

			if(nRet == -1 || nRet == 1)
				break;
			cnt++;
			if((cnt % 10) == 0)
			{
				gUtilInd.LogMessageInt(TLV_WAITING);
			}
			Sleep(500);
		}
		
		hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		if(hCursor)
			::SetCursor(hCursor);
	}
	return nRet;
}

#else
void PlayForm::Check_ISDBT13_Loopthru_188TS()
{
	int nBoardNum = gGeneral.gnActiveBoard;
    long        total_count = 0, a_count = 0;
    char        total_info[1024], a_info[1024];
	char		strTemp[16];
	//pTsInfo = NULL;
	if(gpConfig->gBC[nBoardNum].gnIsdbT_LoopThru_flag == 0 && TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(nBoardNum) == 1)
	{
		printf("Start ISDBT_LOOPTHRU_parser\n");
		pTsInfo = NULL;
		TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser(nBoardNum, &pTsInfo);
		gpConfig->gBC[nBoardNum].gnIsdbT_LoopThru_flag = 1;

		gUtility.MyStrCpy(total_info, sizeof(total_info), (char *)"");

		gUtility.MyStrCpy(a_info, sizeof(a_info), (char *)"");
		printf("SET ASI TMCC, pid Count : %d\n", pTsInfo->num_pid_info);    
		for (int i = 0; i < pTsInfo->num_pid_info; i++)
		{
			gUtility.MySprintf(strTemp, 16, (char *)"%d,", pTsInfo->pid_info[i].PID);
			gUtility.MyStrCat(total_info, sizeof(total_info), strTemp);
			gUtility.MyStrCat(a_info, sizeof(a_info), strTemp);
			total_count++;
			a_count++;
		}
	printf("ASI in bitrate %d\n", gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate);	
		// Set TMCC Parameters Default Value
		gWrapDll.Set_Remux_Info(nBoardNum,
            0, 3, 1, 0, gpConfig->gBC[nBoardNum].gnIsdbt13_LoopThru_bitrate,
            13, 3, 4, 0, 22551326,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0);
   
		gWrapDll.Set_Layer_Info(nBoardNum, -1, 0,
			total_count, total_info,
			a_count, a_info,
			0, (char *)"",
			0, (char *)"");

		//2012/3/2 TMCC
		gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
		gpConfig->gBC[nBoardNum].tmccInfo.partial = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 13;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 22551326;
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = (char)a_count;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, a_info);
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = -1;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = -1;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, (char *)"");
		gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = (char)total_count;
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, total_info);

		SNMP_Send_Status(TVB390_TMCC_BROADCAST);
		SNMP_Send_Status(TVB390_TMCC_MODE);
		SNMP_Send_Status(TVB390_TMCC_GUARD);
		SNMP_Send_Status(TVB390_TMCC_PARTIAL);
		SNMP_Send_Status(TVB390_TMCC_SEG_A);
		SNMP_Send_Status(TVB390_TMCC_MOD_A);
		SNMP_Send_Status(TVB390_TMCC_COD_A);
		SNMP_Send_Status(TVB390_TMCC_TIME_A);
		SNMP_Send_Status(TVB390_TMCC_DATA_A);
		SNMP_Send_Status(TVB390_TMCC_SEG_B);
		SNMP_Send_Status(TVB390_TMCC_MOD_B);
		SNMP_Send_Status(TVB390_TMCC_COD_B);
		SNMP_Send_Status(TVB390_TMCC_TIME_B);
		SNMP_Send_Status(TVB390_TMCC_DATA_B);
		SNMP_Send_Status(TVB390_TMCC_SEG_C);
		SNMP_Send_Status(TVB390_TMCC_MOD_C);
		SNMP_Send_Status(TVB390_TMCC_COD_C);
		SNMP_Send_Status(TVB390_TMCC_TIME_C);
		SNMP_Send_Status(TVB390_TMCC_DATA_C);

		gWrapDll.SetStreamSourcePort(FILE_SRC);
		Sleep(1000);
		gWrapDll.SetStreamSourcePort(DVBASI_SRC);
		TSPH_START_MONITOR(nBoardNum, 0);
	}
}
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// called from CloseSystem, Adaptor change, Modulator Type change
void PlayForm::SaveStreamListInfo(long nBoardNum, long nModulatorMode)
{
#ifdef WIN32
	return;
#endif
    // save filelist/playlist to playlist-modulator-board
    long    index, index1;
    char    szFilePath[256];
    FILE    *hFile = NULL;
    char    str[100];
	int		nCount, nCount1;

    //------------------------------------------------------------
    // open playlist-modulatormode-slot
#ifdef STANDALONE
	gUtility.MySprintf(szFilePath, 256, (char *) "/sysdb/playlist-%d-%d", nModulatorMode, nBoardNum);
#else
	gUtility.MySprintf(szFilePath, 256, (char *) "%s/playlist-%d-%d", gpConfig->gBC[nBoardNum].gszMasterDirectory, nModulatorMode, nBoardNum);
#endif
	hFile = gUtility.MyFopen(hFile, szFilePath, (char *)"w");

    if (hFile == NULL)
        return;

	nCount =  gpConfig->gBC[nBoardNum].nPlayListIndexCount;
	nCount1 = gpConfig->gBC[nBoardNum].nFileListIndexCount;
    //------------------------------------------------------------
    // Save the current index of FILE LIST at 1st line
    index1 = 0;     // find selected file index in the file list
    if (nCount1 > 0)
    {
        for (index = 0; index < nCount1; index++)
        {
            if (gpConfig->gBC[nBoardNum].nFileListIndexCur == index)
            {
                index1 = index;
                break;
            }
        }
    }
    gUtility.MySprintf(str, 100, (char *) "%d\n", index1);
    fwrite(str, 1, strlen(str), hFile);

    //------------------------------------------------------------
    // Save the current index of PLAY LIST at 2nd line
    index1 = 0;
    if (nCount > 0)
    {
        for (index = 0; index < nCount; index++)
        {
            if (gpConfig->gBC[nBoardNum].nPlayListIndexCur == index)
            {
                index1 = index;
                break;
            }
        }
        gUtility.MySprintf(str, 100, (char *)"%d\n", index1);
        fwrite(str, 1, strlen(str), hFile);

        // Save PLAY LIST items
        for (index = 0; index < nCount; index++)
        {
            gUtility.MySprintf(str, 100, (char *) "%s\n", gpConfig->gBC[nBoardNum].szPlayFileList[index]);
            fwrite(str, 1, strlen(str), hFile);
        }
    } else
    {
        gUtility.MySprintf(str, 100, (char *) "%d\n", index1);
        fwrite(str, 1, strlen(str), hFile);
    }
    fclose(hFile);
}

//=================================================================================
#ifdef WIN32 
void PlayForm::OnCheckedAGC()
#else
void PlayForm::OnCheckedAGC(long lAgc)
#endif
{

	//2011/4/13 added
	if(gGeneral.gnChangeAdapt_Flag == 1) 
		return;
	long nBoardNum = gGeneral.gnActiveBoard;

#ifdef WIN32
	//2013/3/12 TODO
	//if(gBaseUtil.isChecked_Check(Check_AGC) == true)
	//{
	//	this->Output_Agc_On->Checked = true;
	//	this->Output_Agc_Off->Checked = false;
	//}
	//else
	//{
	//	this->Output_Agc_Off->Checked = true;
	//	this->Output_Agc_On->Checked = false;
	//}
	//gpConfig->gBC[nBoardNum].gnAGC = gBaseUtil.isChecked_Check(Check_AGC);
#else
	gpConfig->gBC[nBoardNum].gnAGC = lAgc;
#endif
	UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
	SNMP_Send_Status(TVB390_USE_AGC);
	SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_ATTEN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MAX);
	SNMP_Send_Status(TVB390_ATTEN_MIN);
	SNMP_Send_Status(TVB390_ATTEN_MAX);
}

//=================================================================================
#ifdef WIN32 

void PlayForm::OnCheckedBYPASSAMP()
{

	//2011/4/13 added
	if(gGeneral.gnChangeAdapt_Flag == 1) 
		return;

	long nBoardNum = gGeneral.gnActiveBoard;

	//2012/5/24
	if(gpConfig->gBC[nBoardNum].bRecordInProgress == true)
	{
		return;
	}

    gWrapDll.TVB380_SET_BOARD_CONFIG_STATUS_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBypassAMP);
    
    //AGC - RF Level -> Atten/AGC
    UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
    
	SNMP_Send_Status(TVB390_USE_AMP);
	SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_ATTEN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MAX);
	SNMP_Send_Status(TVB390_ATTEN_MIN);
	SNMP_Send_Status(TVB390_ATTEN_MAX);
}
#else
void PlayForm::OnCheckedBYPASSAMP(long lByPass)
{

	//2011/4/13 added
	if(gGeneral.gnChangeAdapt_Flag == 1) 
		return;

	long nBoardNum = gGeneral.gnActiveBoard;
	//2011/1/13 FIXED
	if( gpConfig->gBC[nBoardNum].gnBoardId >= 47 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || 
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
	{
		gpConfig->gBC[nBoardNum].gnBypassAMP = lByPass;
	}	
	else
	{
		gpConfig->gBC[nBoardNum].gnBypassAMP = 1;		
	}
	TVB380_SET_BOARD_CONFIG_STATUS_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBypassAMP);
    //AGC - RF Level -> Atten/AGC
    UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
    
	SNMP_Send_Status(TVB390_USE_AMP);
	SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_ATTEN);
	SNMP_Send_Status(TVB390_ATTEN_MIN);
	SNMP_Send_Status(TVB390_ATTEN_MAX);
}
#endif

//2011/11/22 improve ISDB-T UI
#ifdef WIN32 

void PlayForm::SetDefaultValue_noTMCC(char *filename)
{
	int nBoardNum = gGeneral.gnActiveBoard;
    FILE            *hFile = NULL;
    long        total_count = 0, a_count = 0;
    char        total_info[4096], a_info[4096];
	char		strTemp[16];
	int i;
	
	if(gUtility.Is_File_Exist(filename) == false)
		return;
	struct _TSPH_TS_INFO *pTsInfo = NULL;

	if((gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13) && gpConfig->gBC[nBoardNum].gnHaveTMCC == 0 &&
		(gpConfig->gBC[nBoardNum].gnModulatorSource != DVBASI_SRC && gpConfig->gBC[nBoardNum].gnModulatorSource != SMPTE310M_SRC))
	{
		HCURSOR hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
		if(hCursor)
			::SetCursor(hCursor);

		i = gWrapDll.Run_Ts_Parser2(nBoardNum, filename, 0, &pTsInfo);
		if (i == -1)
			return;

		if(pTsInfo == NULL)
			return;
		gUtility.MyStrCpy(total_info, sizeof(total_info), "");
		gUtility.MyStrCpy(a_info, sizeof(a_info), "");
	    
		for (i = 0; i < pTsInfo->num_pid_info; i++)
		{
			gUtility.MySprintf(strTemp, 16, "%d,", pTsInfo->pid_info[i].PID);
			gUtility.MyStrCat(total_info, sizeof(total_info), strTemp);
			gUtility.MyStrCat(a_info, sizeof(a_info), strTemp);
			total_count++;
			a_count++;
		}
    
		// Set TMCC Parameters Default Value
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
		{
			gWrapDll.Set_Remux_Info(nBoardNum,
				1, 3, 1, 1, gpConfig->gBC[nBoardNum].gdwPlayRate,
				1, 3, 4, 0, 1734717,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0);
		}
		else
		{
			gWrapDll.Set_Remux_Info(nBoardNum,
				0, 3, 1, 0, gpConfig->gBC[nBoardNum].gdwPlayRate,
				13, 3, 4, 0, 22551326,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0);
		}
		gWrapDll.Set_Layer_Info(nBoardNum, -1, 0,
			total_count, total_info,
			a_count, a_info,
			0, "",
			0, "");
		hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		if(hCursor)
			::SetCursor(hCursor);
	}
}
#else 
//2011/11/22 improve ISDB-T UI
void PlayForm::SetDefaultValue_noTMCC(char *filename)
{
	int nBoardNum = gGeneral.gnActiveBoard;
    long        total_count = 0, a_count = 0;
    char        total_info[1024], a_info[1024];
	char		strTemp[16];
	int i;

	pTsInfo = NULL;
	if(gUtility.Is_File_Exist(filename) == false)
		return;

	if((gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13) && gpConfig->gBC[nBoardNum].gnHaveTMCC == 0 &&
		(gpConfig->gBC[nBoardNum].gnModulatorSource != DVBASI_SRC && gpConfig->gBC[nBoardNum].gnModulatorSource != SMPTE310M_SRC))
	{
		i = gWrapDll.Run_Ts_Parser2(nBoardNum, filename, 0, &pTsInfo);
		if (i == -1)
			return;

		if(pTsInfo == NULL)
			return;

		gUtility.MyStrCpy(total_info, sizeof(total_info), (char *)"");

		gUtility.MyStrCpy(a_info, sizeof(a_info), (char *)"");
	    
		for (i = 0; i < pTsInfo->num_pid_info; i++)
		{
			gUtility.MySprintf(strTemp, 16, "%d,", pTsInfo->pid_info[i].PID);
			gUtility.MyStrCat(total_info, sizeof(total_info), strTemp);
			gUtility.MyStrCat(a_info, sizeof(a_info), strTemp);
			total_count++;
			a_count++;
		}
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
		{
			gWrapDll.Set_Remux_Info(nBoardNum,
				0, 3, 1, 0, gpConfig->gBC[nBoardNum].gdwPlayRate,
				1, 3, 4, 0, 1734717,
				12, 3, 4, 0, 0,
				0, 0, 0, 0, 0);
		}
		else
		{
			// Set TMCC Parameters Default Value
			gWrapDll.Set_Remux_Info(nBoardNum,
                0, 3, 1, 0, gpConfig->gBC[nBoardNum].gdwPlayRate,
                13, 3, 4, 0, 22551326,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0);
		}
   
		gWrapDll.Set_Layer_Info(nBoardNum, -1, 0,
			total_count, total_info,
			a_count, a_info,
			0, (char *)"",
			0, (char *)"");

		//2012/3/2 TMCC
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
		{
			gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
			gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
			gpConfig->gBC[nBoardNum].tmccInfo.partial = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 1734717;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = (char)a_count;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, a_info);
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 12;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = 3;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = 4;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = -1;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, (char *)"");
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = -1;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, (char *)"");
			gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = (char)total_count;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, total_info);
		}
		else
		{
			gpConfig->gBC[nBoardNum].tmccInfo.broadcast = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.mode = 2;
			gpConfig->gBC[nBoardNum].tmccInfo.guard = 1;
			gpConfig->gBC[nBoardNum].tmccInfo.partial = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = 13;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = 3;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = 4;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = 2;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate = 22551326;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = (char)a_count;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, a_info);
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = -1;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, (char *)"");
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = -1;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate = 0;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = -1;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, (char *)"");
			gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = (char)total_count;
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, total_info);
		}
	}
	SNMP_Send_Status(TVB390_TMCC_BROADCAST);
	SNMP_Send_Status(TVB390_TMCC_MODE);
	SNMP_Send_Status(TVB390_TMCC_GUARD);
	SNMP_Send_Status(TVB390_TMCC_PARTIAL);
	SNMP_Send_Status(TVB390_TMCC_SEG_A);
	SNMP_Send_Status(TVB390_TMCC_MOD_A);
	SNMP_Send_Status(TVB390_TMCC_COD_A);
	SNMP_Send_Status(TVB390_TMCC_TIME_A);
	SNMP_Send_Status(TVB390_TMCC_DATA_A);
	SNMP_Send_Status(TVB390_TMCC_SEG_B);
	SNMP_Send_Status(TVB390_TMCC_MOD_B);
	SNMP_Send_Status(TVB390_TMCC_COD_B);
	SNMP_Send_Status(TVB390_TMCC_TIME_B);
	SNMP_Send_Status(TVB390_TMCC_DATA_B);
	SNMP_Send_Status(TVB390_TMCC_SEG_C);
	SNMP_Send_Status(TVB390_TMCC_MOD_C);
	SNMP_Send_Status(TVB390_TMCC_COD_C);
	SNMP_Send_Status(TVB390_TMCC_TIME_C);
	SNMP_Send_Status(TVB390_TMCC_DATA_C);
}
#endif
//-----------------------------------------------------------------------------------------------------------------------------
#ifdef WIN32
void PlayForm::OnBnClickedComf3()
{
	if(gGeneral.gnChangeAdapt_Flag != 1)
	{
		gUtilInd.Set_Next_Input_Source(gGeneral.gnActiveBoard);
		int nBoardNum = gGeneral.gnActiveBoard;
		if((gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16) &&
			gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))		//2013/5/27 TVB599 0xC
		{
			if(gpConfig->gBC[nBoardNum].gnInputSource == DVB_ASI_IN ||  gpConfig->gBC[nBoardNum].gnInputSource == SMPTE_310M_IN)
			{
				if(gpConfig->gBC[nBoardNum].bRecordInProgress == false)
					gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);
			}
			else
			{
				gWrapDll.TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, 1);
			}
		}
	}

    SNMP_Send_Status(TVB390_SET_INPUT_SOURCE);

	Display_Label_File_Start();
	Display_Label_LabFileSize();
	Display_Slider_LabTimer();
	Display_ComF1();
	Display_ComF2();
	Display_ComF6();
	Display_Radio_RF();
	Display_Radio_File_generation();
	//2011/7/18 DVB-T2 IP
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_C2)
		SetDVBT2_IP_Params();

	//2011/11/10 ISDB-T 13seg 188 TS LoopThru
	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T) && 
		(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC))
	{
		gWrapDll.TSPH_CLEAR_REMUX_INFO(gGeneral.gnActiveBoard);
		gWrapDll.InitTMCCVarible(gGeneral.gnActiveBoard);
	}
	else if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T) &&
				gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
	{
		gWrapDll.TSPH_CLEAR_REMUX_INFO(gGeneral.gnActiveBoard);
		gWrapDll.InitTMCCVarible(gGeneral.gnActiveBoard);
	}

	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_C2 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_S ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ATSC_MH) && 
		(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC))
	{
		gWrapDll.TSPH_SET_SDRAM_BANK_INFO(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnSubBankNumber, 1024);
		gWrapDll.TSPH_START_MONITOR(gGeneral.gnActiveBoard, 0);
	}
	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 12 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 16) && gGeneral.gnChangeAdapt_Flag != 1)
	{
		if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
			gWrapDll.TSPL_SET_SDCON_MODE_EX(gGeneral.gnActiveBoard, 2);
	}
	//2011/12/2 TVB594
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == _TVB594_BD_ID_)
	{
		if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
		{
			gBaseUtil.SetEnableControl(ComF6,FALSE);
			this->Playback_sub_StartCapture->Enabled = false;
		}
		else
		{
			gBaseUtil.SetEnableControl(ComF6,TRUE);
			this->Playback_sub_StartCapture->Enabled = true;
		}
	}
}
#else
void PlayForm::OnBnClickedComf3()
{
	gUtilInd.Set_Next_Input_Source(gGeneral.gnActiveBoard);
    SNMP_Send_Status(TVB390_SET_INPUT_SOURCE);

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2)
		SetT2MI();
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_C2)
		SetDVBC2_parameters();


	//2011/11/10 ISDB-T 13seg 188 TS LoopThru
	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T) && 
		(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC))
	{
		TSPH_CLEAR_REMUX_INFO(gGeneral.gnActiveBoard);
		TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(gGeneral.gnActiveBoard);
		gWrapDll.Set_Tmcc_Remuxer(gGeneral.gnActiveBoard, 1);
		gpConfig->gBC[gGeneral.gnActiveBoard].gnIsdbT_LoopThru_flag = 0;
	}
	else if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T) &&
				gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
	{
		if( gpConfig->gBC[gGeneral.gnActiveBoard].gnHaveTMCC == 0)
		{
			SetDefaultValue_noTMCC(gpConfig->gBC[gGeneral.gnActiveBoard].szCurFileName);
			gWrapDll.Set_Tmcc_Remuxer(gGeneral.gnActiveBoard, 1);
		}
		else
			gWrapDll.Set_Tmcc_Remuxer(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnUseTMCCRemuxer + gpConfig->gBC[gGeneral.gnActiveBoard].gnEmergencyBroadcasting * 2);
	}

	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_C2 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_S ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ATSC_MH) && 
		(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC))
	{
		TSPH_START_MONITOR(gGeneral.gnActiveBoard, 0);
	}
}
#endif
//-----------------------------------------------------------------------
// Parameter change
//					PARAM1		PARAM2		PARAM3		PARAM4		PARAM5		PARAM6		PARAM7		Check	Command		List
/*
    DVB_T = 0,		BAND		Const		Coderate	Tx			Guard		-		
    VSB_8,			-
    QAM_A,			Const
    QAM_B,			Const		Interleave
    QPSK,			Coderate	Spectrum	RRC Flter
    TDMB,			-
    VSB_16,			-
    DVB_H,			BAND		Const		Coderate	Tx			Guard		-			-			x		O			x
    DVB_S2,			Const		coderate	Pilot		Rolloff		Spectrum
    ISDB_T,			-			-			-			-			-			-			-			O		O			O
    ISDB_T_13,
    DTMB,			Const		Coderate	Interleave	FrameHeader	CarrierNum	FrameHeaerPN Pilot
*/
#ifdef WIN32
void PlayForm::OnCbnSelchangeParam1()
#else
void PlayForm::OnCbnSelchangeParam1(int iCurIx)
#endif
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int			nBoardNum = gGeneral.gnActiveBoard;
    int			iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;
#ifdef WIN32
	double dwSymbolrate;
	int			iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM1);
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DVB_T:	// band
		case DVB_H:	// band
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
			if(iMod == MULTIPLE_DVBT)
			{
				int nVirCnt;
				int RealVirSlot[4];
#ifdef WIN32
				gWrapDll.TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
				nVirCnt = gWrapDll.TSPL_CNT_MULTI_DVBT_RFOUT_EX(nBoardNum);
#else
				TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
				nVirCnt = TSPL_CNT_MULTI_DVBT_RFOUT_EX(nBoardNum);
#endif
				if(nVirCnt > 1)
				{
					for(int i = 1; i < nVirCnt; i++)
					{
						if(gpConfig->gBC[RealVirSlot[i]].bPlayingProgress == true)
						{
#ifdef WIN32
                           gUtilInd.LogMessageInt(TLV_INVALID_WHEN_PLAYING_OR_RECORDING);
                           gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, gpConfig->gBC[nBoardNum].gnBandwidth);
#endif
                           return;
						}
					}
				}
			}

            gpConfig->gBC[nBoardNum].gnBandwidth = iCurIx;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 1);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            if (iMod == DVB_T)
                SNMP_Send_Status(TVB390_DVB_T_H_BANDWIDTH);
            else
                SNMP_Send_Status(TVB390_DVB_H_BANDWIDTH);
			if(iMod == MULTIPLE_DVBT)
			{
#ifdef WIN32
			    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
					gpConfig->gBC[nBoardNum].gnCurrentIF = gWrapDll.TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnBandwidth);
				else	
					gpConfig->gBC[nBoardNum].gnCurrentIF = gWrapDll.TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
#else
			    if (gpConfig->gBC[nBoardNum].gnFreqPolicy == 1)
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq + 1750000, gpConfig->gBC[nBoardNum].gnBandwidth);
				else	
					gpConfig->gBC[nBoardNum].gnCurrentIF = TVB380_SET_MODULATOR_FREQ_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
#endif
			}
			break;

		case QAM_A:	// const
            gpConfig->gBC[nBoardNum].gnQAMMode = iCurIx;
            gWrapDll.Set_QAMA_Parameters(nBoardNum, 5); // Constellation
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            SNMP_Send_Status(TVB390_QAM_A_CONSTELLATION);
			break;

		case QAM_B:	// const
		case MULTIPLE_QAMB:
			if(iMod == MULTIPLE_QAMB)
			{
				int nVirCnt;
				int RealVirSlot[4];
#ifdef WIN32
				gWrapDll.TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
				nVirCnt = gWrapDll.TSPL_CNT_MULTI_QAM_RFOUT_EX(nBoardNum);
#else
				TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
				nVirCnt = TSPL_CNT_MULTI_QAM_RFOUT_EX(nBoardNum);
#endif
				if(nVirCnt > 1)
				{
					for(int i = 1; i < nVirCnt; i++)
					{
						if(gpConfig->gBC[RealVirSlot[i]].bPlayingProgress == true)
						{
#ifdef WIN32
                           gUtilInd.LogMessageInt(TLV_INVALID_WHEN_PLAYING_OR_RECORDING);
                           gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, gpConfig->gBC[nBoardNum].gnQAMMode);
#endif
                           return;
						}
					}
				}
			}
            gpConfig->gBC[nBoardNum].gnQAMMode = iCurIx;

			if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
				gpConfig->gBC[nBoardNum].gnSymbolRate = 5056941;
            else
                gpConfig->gBC[nBoardNum].gnSymbolRate = 5360537;
			
			gWrapDll.Set_QAMB_Parameters(nBoardNum, 5); // Constellation
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
#ifdef WIN32
			dwSymbolrate = (double)((double)gpConfig->gBC[gGeneral.gnActiveBoard].gnSymbolRate / 1000.0);
			Label_minimum_symrate_value->Text = dwSymbolrate.ToString();
			gWrapDll.TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
#else
			TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnSymbolRate);
#endif
			SNMP_Send_Status(TVB390_QAM_B_CONSTELLATION);
			break;

		case QPSK:	// coderate
            gpConfig->gBC[nBoardNum].gnCodeRate = iCurIx;
            gWrapDll.Set_QPSK_Parameters(nBoardNum, 2);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            SNMP_Send_Status(TVB390_QPSK_CODERATE);
			break;

		case DVB_S2:// const
            if (gpConfig->gBC[nBoardNum].gnConstellation == iCurIx)
                break;

            gpConfig->gBC[nBoardNum].gnConstellation = iCurIx;
            gWrapDll.Set_DVBS2_Parameters(nBoardNum, 5);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
#ifdef WIN32
            Display_DVBS2_Coderate(nBoardNum);         
#endif
            SNMP_Send_Status(TVB390_DVB_S2_CONSTELLATION);
			break;

		case DTMB:	// FRAME HEADER P/N
			gpConfig->gBC[nBoardNum].gnFrameHeaderPN = iCurIx;
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER_PN);
			break;

		case CMMB:	//CMMB
			long nMF_ID;
#ifdef WIN32
			nMF_ID = gBaseUtil.SelectedIndex_Combo(Combo_PARAM1);
			//2011/4/11 fixed
			if(gBaseUtil.CountItems_Combo(Combo_PARAM1) < 1)
				return;
			if( nMF_ID >= 0 && nMF_ID < gBaseUtil.CountItems_Combo(Combo_PARAM1))
			{
				//2011/2/14 FIXED
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][3] >= 0)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][3]);
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][0] >= 0)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][0]);
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][1] >= 0)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM4, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][1]);
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][2] >= 0)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM5, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][2]);
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][4] >= 0)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM6, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][4]);
				if(gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][5] >= 1)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM7, gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][5] - 1);
			}
#else
			nMF_ID = iCurIx;
			if( nMF_ID >= 0 && nMF_ID < gpConfig->gBC[nBoardNum].gnCMMB_Params_Count)
			{
				gpConfig->gBC[nBoardNum].gnCmmb_Mdif = nMF_ID;
				gpConfig->gBC[nBoardNum].gnCmmb_const = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][3];
				gpConfig->gBC[nBoardNum].gnCmmb_rscoding = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][0];
				gpConfig->gBC[nBoardNum].gnCmmb_bytecrossing = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][1];
				gpConfig->gBC[nBoardNum].gnCmmb_ldpc = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][2];
				gpConfig->gBC[nBoardNum].gnCmmb_scramble = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][4];
				gpConfig->gBC[nBoardNum].gnCmmb_timeslice = gpConfig->gBC[nBoardNum].gnCMMB_Params[nMF_ID][5] - 1;
			}
			SNMP_Send_Status(TVB390_CMMB_MDIF);
			SNMP_Send_Status(TVB390_CMMB_CONSTELLATION);
			SNMP_Send_Status(TVB390_CMMB_RSCODING);
			SNMP_Send_Status(TVB390_CMMB_BYTECROSSING);
			SNMP_Send_Status(TVB390_CMMB_LDPC);
			SNMP_Send_Status(TVB390_CMMB_SCRAMBLE);
			SNMP_Send_Status(TVB390_CMMB_TIMESLICE);
			SNMP_Send_Status(TVB390_CMMB_MDIF_ITEM);
#endif
			break;

		case DVB_T2:
#ifdef WIN32
			if(gpConfig->gBC[nBoardNum].gnBandwidth != iCurIx)
			{
	HCURSOR hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	if(hCursor)
		::SetCursor(hCursor);
				char str[512];
				gpConfig->gBC[nBoardNum].gnBandwidth = iCurIx;
				gBaseUtil.SetText_Text(ELabPlayRate, gUtilInd.CalcBurtBitrate(nBoardNum));

				if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
					gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnBandwidth + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
					gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnBandwidth + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnBandwidth - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				
				gUtility.MySprintf(str, 512, "%s", gpConfig->gBC[nBoardNum].szCurFileName);

				if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
				{
					gGeneral.gnBitrate = gWrapDll.TSPH_RUN_T2MI_PARSER(nBoardNum, str, &(gpConfig->gBC[nBoardNum].gsT2mi_info));
					Display_PlayRate(gGeneral.gnBitrate);
					gBaseUtil.SetText_Label(LabPlayRateCalc, gGeneral.gnBitrate);
				}
				else if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
				{
					for(int k = 0; k < gpConfig->gBC[nBoardNum].nPlayListIndexCount; k++)
					{
						gUtility.MySprintf(str, 512, "%s\\%s", gpConfig->gBC[nBoardNum].szPlayFileList[k], gpConfig->gBC[nBoardNum].szPlayListFileName[k]);
						gpConfig->gBC[nBoardNum].gnPlayListPlaybackRate[k] = gWrapDll.TSPH_RUN_T2MI_PARSER(nBoardNum, str, &(gpConfig->gBC[nBoardNum].gsT2mi_info));
					}
					Display_T2MI_PlayList_Playbackrate(nBoardNum);
				}
				else
				{
					long PlaybackRate;
					long PLP_MOD, PLP_COD, PLP_FEC_TYPE;
					long PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE;
					long S1, NUM_DATA_SYMBOLS, PILOT_PATTERN;
					long BW, BWT_EXT, L1_POST_SIZE, HEM, NPD;
					long TLV_BW, TLV_FFT_SIZE;
					long L1_MOD, L1_POST_INFO_SIZE, L1CONF_LEN, L1DYN_CURR_LEN;
					long nBoardNum = gGeneral.gnActiveBoard;
					long TLV_PLP_COD;
					L1CONF_LEN = 191;
					L1DYN_CURR_LEN = 127;
					//2010/12/28 DVB-T2 MULTI-PLP ==================================================================================
					int plp_cnt = 0;
					int NUM_RF = 1;
					int S2 = 0;
					int NUM_AUX = 0;
					for(int mm = 0; mm < MAX_PLP_TS_COUNT; mm++)
					{
						if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[mm] >= 0 && strlen(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[mm]) > 0)
							plp_cnt++;
					}
					if(plp_cnt > 1)
					{
						L1CONF_LEN = (35 + (35 * NUM_RF) + ((S2 & 0x01) == 1 ? 34 : 0) + (plp_cnt * 89) + 32 + (NUM_AUX * 32));
						L1DYN_CURR_LEN = (71 + (plp_cnt * 48) + 8 + (NUM_AUX * 48));
					}
					//===============================================================================================================
					L1_POST_INFO_SIZE = L1CONF_LEN + L1DYN_CURR_LEN;
					L1_MOD = gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD;
					GUARD_INTERVAL = gpConfig->gBC[nBoardNum].gnT2MI_GUARD;
					TLV_FFT_SIZE = gpConfig->gBC[nBoardNum].gnT2MI_FFT;
					if(TLV_FFT_SIZE == 0)
						FFT_SIZE = 3;
					else if(TLV_FFT_SIZE == 1)
						FFT_SIZE = 0;
					else if(TLV_FFT_SIZE == 2)
						FFT_SIZE = 2;
					else if(TLV_FFT_SIZE == 3)
					{
						if(GUARD_INTERVAL <= 3)
							FFT_SIZE = 1;
						else
							FFT_SIZE = 6;
					}
					else if(TLV_FFT_SIZE == 4)
						FFT_SIZE = 4;
					else if(TLV_FFT_SIZE == 5)
					{
						if(GUARD_INTERVAL <= 3)
							FFT_SIZE = 5;
						else
							FFT_SIZE = 7;
					}
					S1 = 0; //SISO
					NUM_DATA_SYMBOLS = gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL;
					PILOT_PATTERN = gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN;
					BW = gpConfig->gBC[nBoardNum].gnT2MI_BW;
					BWT_EXT = gpConfig->gBC[nBoardNum].gnT2MI_BWT;

					NPD = 0; //Null packet deletion
					L1_POST_SIZE = gUtilInd.Calulate_L1_Post_Size(L1_MOD, FFT_SIZE, L1_POST_INFO_SIZE);
					//2010/12/22 DVB-T2 MULTI-PLP =========================================================================================
					PlaybackRate = 0;
					int tmp_value;
					for(int i = 0; i < MAX_PLP_TS_COUNT; i++)
					{
						tmp_value = 0;
						if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0)
						{
							PLP_MOD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[i];
							TLV_PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[i];
							PLP_FEC_TYPE = gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[i];
							PLP_NUM_BLOCKS = gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[i];
							HEM = gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0];
	
							//2010/6/3
							if(TLV_PLP_COD == 0)
								PLP_COD = 0;
							else if(TLV_PLP_COD == 1)
								PLP_COD = 2;
							else if(TLV_PLP_COD == 2)
								PLP_COD = 3;
							else if(TLV_PLP_COD == 3)
								PLP_COD = 1;
							else if(TLV_PLP_COD == 4)
								PLP_COD = 4;
							else
								PLP_COD = 5;
							tmp_value = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 0, 
								PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE, 
								S1, NUM_DATA_SYMBOLS, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD);
						}
						PlaybackRate = PlaybackRate + tmp_value;
					}
					Display_PlayRate(PlaybackRate);

				}
	hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	if(hCursor)
		::SetCursor(hCursor);
			}
#else
			if(gpConfig->gBC[nBoardNum].gnT2MI_BW != iCurIx)
			{
				char str[256];
				gpConfig->gBC[nBoardNum].gnT2MI_BW = iCurIx;
				if(gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
				{
					gpConfig->gBC[nBoardNum].gdwPlayRate = gUtilInd.CalcBurtBitrate(nBoardNum);
				}
				if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnT2MI_BW + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnT2MI_BW + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				else
					TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnT2MI_BW - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				
				int	index;
				int	nCount;
	
				if(gpConfig->gBC[nBoardNum].fCurFocus == FILELISTWINDOW)
				{
					index = gpConfig->gBC[nBoardNum].nFileListIndexCur;
					nCount = gpConfig->gBC[nBoardNum].nFileListIndexCount;
				}
				else
				{
					index = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
					nCount = gpConfig->gBC[nBoardNum].nPlayListIndexCount;
				}
			    // Limit FileList ListIndex to valid range
			    if (nCount < 1)
				{
				    SNMP_Send_Status(TVB390_TS);

					SNMP_Send_Status(TVB390_DVB_T2_BANDWIDTH);
					return; // nothing to display
				}
				// Adjust Filelist Index
				if(gpConfig->gBC[nBoardNum].fCurFocus == FILELISTWINDOW)
				{
					if (index < 0)        // let the index have valid value
						gpConfig->gBC[nBoardNum].nFileListIndexCur = 0;
					else if (index >= nCount)
						gpConfig->gBC[nBoardNum].nFileListIndexCur = nCount - 1;

					index = gpConfig->gBC[nBoardNum].nFileListIndexCur;
					gUtility.MySprintf(str, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory , gpConfig->gBC[nBoardNum].szFileFileList[index]);
				}
				else
				{
					if (index < 0)        // let the index have valid value
						gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
					else if (index >= nCount)
						gpConfig->gBC[nBoardNum].nPlayListIndexCur = nCount - 1;

					index = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
					gUtility.MySprintf(str, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory , gpConfig->gBC[nBoardNum].szPlayFileList[index]);
				}
				//-----------------------------------
				gUtilInd.SetLabPlayRateCalc(str);
				//2010/11/12
				if(gGeneral.gnT2MI_playrate > 1)
				{
					Display_PlayRate(gGeneral.gnT2MI_playrate);
				}
				else
				{
					long PlaybackRate;
					long PLP_MOD, PLP_COD, PLP_FEC_TYPE;
					long PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE;
					long S1, NUM_DATA_SYMBOLS, PILOT_PATTERN;
					long BW, BWT_EXT, L1_POST_SIZE, HEM, NPD;
					long TLV_BW, TLV_FFT_SIZE;
					long L1_MOD, L1_POST_INFO_SIZE, L1CONF_LEN, L1DYN_CURR_LEN;
					long nBoardNum = gGeneral.gnActiveBoard;
					long TLV_PLP_COD;
					L1CONF_LEN = 191;
					L1DYN_CURR_LEN = 127;
					//2010/12/28 DVB-T2 MULTI-PLP ==================================================================================
					int plp_cnt = 0;
					int NUM_RF = 1;
					int S2 = 0;
					int NUM_AUX = 0;
					for(int mm = 0; mm < MAX_PLP_TS_COUNT; mm++)
					{
						if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[mm] >= 0 && strlen(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[mm]) > 0)
							plp_cnt++;
					}
					if(plp_cnt > 1)
					{
						L1CONF_LEN = (35 + (35 * NUM_RF) + ((S2 & 0x01) == 1 ? 34 : 0) + (plp_cnt * 89) + 32 + (NUM_AUX * 32));
						L1DYN_CURR_LEN = (71 + (plp_cnt * 48) + 8 + (NUM_AUX * 48));
					}
					//===============================================================================================================
					L1_POST_INFO_SIZE = L1CONF_LEN + L1DYN_CURR_LEN;
					L1_MOD = gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD;
					GUARD_INTERVAL = gpConfig->gBC[nBoardNum].gnT2MI_GUARD;
					TLV_FFT_SIZE = gpConfig->gBC[nBoardNum].gnT2MI_FFT;
					if(TLV_FFT_SIZE == 0)
						FFT_SIZE = 3;
					else if(TLV_FFT_SIZE == 1)
						FFT_SIZE = 0;
					else if(TLV_FFT_SIZE == 2)
						FFT_SIZE = 2;
					else if(TLV_FFT_SIZE == 3)
					{
						if(GUARD_INTERVAL <= 3)
							FFT_SIZE = 1;
						else
							FFT_SIZE = 6;
					}
					else if(TLV_FFT_SIZE == 4)
						FFT_SIZE = 4;
					else if(TLV_FFT_SIZE == 5)
					{
						if(GUARD_INTERVAL <= 3)
							FFT_SIZE = 5;
						else
							FFT_SIZE = 7;
					}
					S1 = 0; //SISO
					NUM_DATA_SYMBOLS = gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL;
					PILOT_PATTERN = gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN;
					BW = gpConfig->gBC[nBoardNum].gnT2MI_BW;
					if(TLV_BW == 0)	//6MHz
						BW = 2;
					else if(TLV_BW == 1) //7MHz
						BW = 3;
					else if(TLV_BW == 2) //8MHz
						BW = 4;
					else
						BW = 1;			//5MHz
					BWT_EXT = gpConfig->gBC[nBoardNum].gnT2MI_BWT;

					NPD = 0; //Null packet deletion
					L1_POST_SIZE = gUtilInd.Calulate_L1_Post_Size(L1_MOD, FFT_SIZE, L1_POST_INFO_SIZE);
					//2010/12/22 DVB-T2 MULTI-PLP =========================================================================================
					PlaybackRate = 0;
					int tmp_value;
					for(int i = 0; i < MAX_PLP_TS_COUNT; i++)
					{
						tmp_value = 0;
						if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0)
						{
							PLP_MOD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[i];
							TLV_PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[i];
							PLP_FEC_TYPE = gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[i];
							PLP_NUM_BLOCKS = gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[i];
							HEM = gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0];
	
							//2010/6/3
							if(TLV_PLP_COD == 0)
								PLP_COD = 0;
							else if(TLV_PLP_COD == 1)
								PLP_COD = 2;
							else if(TLV_PLP_COD == 2)
								PLP_COD = 3;
							else if(TLV_PLP_COD == 3)
								PLP_COD = 1;
							else if(TLV_PLP_COD == 4)
								PLP_COD = 4;
							else
								PLP_COD = 5;
							tmp_value = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 0, 
								PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE, 
								S1, NUM_DATA_SYMBOLS, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD);
						}
						PlaybackRate = PlaybackRate + tmp_value;
					}
					Display_PlayRate(PlaybackRate);

				}
				SNMP_Send_Status(TVB390_TS);
				SNMP_Send_Status(TVB390_DVB_T2_BANDWIDTH);

			}
#endif
			break;
		//2010/12/07 ISDB-S =========================================================================================================================
		case ISDB_S:
			if(gpConfig->gBC[nBoardNum].gnConstellation != iCurIx)
			{
				gpConfig->gBC[nBoardNum].gnConstellation = iCurIx;
#ifdef WIN32
				if(iCurIx == CONST_ISDBS_BPSK)
				{
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, CONST_ISDBS_CODE_1_2);
					gBaseUtil.SetEnableControl(Combo_PARAM2, FALSE);
					this->Modulator_Param2->Enabled = false;
				}
				else if(iCurIx == CONST_ISDBS_TC8PSK)
				{
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, CONST_ISDBS_CODE_2_3);
					gBaseUtil.SetEnableControl(Combo_PARAM2, FALSE);
					this->Modulator_Param2->Enabled = false;
				}
				else
				{
					gBaseUtil.SetEnableControl(Combo_PARAM2, TRUE);
					this->Modulator_Param2->Enabled = true;
				}
				OnCbnSelchangeParam2();
#else
				if(iCurIx == CONST_ISDBS_BPSK)
				{
					OnCbnSelchangeParam2(CONST_ISDBS_CODE_1_2);
				}
				else if(iCurIx == CONST_ISDBS_TC8PSK)
				{
					OnCbnSelchangeParam2(CONST_ISDBS_CODE_2_3);
				}
				else
				{
					OnCbnSelchangeParam2(gpConfig->gBC[nBoardNum].gnCodeRate);
				}
#endif
		        //Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
			}
			break;
		//===========================================================================================================================================
		//2011/2/25 DVB-C2 ==========================================================================================================================
		case DVB_C2:
			if(gpConfig->gBC[nBoardNum].gnBandwidth != iCurIx)
			{
				gpConfig->gBC[nBoardNum].gnBandwidth = iCurIx;
#ifdef WIN32
				gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
				Set_Center_Freq(gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq);
#else
				TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum,iMod, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);
#endif
			}
			break;
		//===========================================================================================================================================
		//2011/9/7 IQ PLAY
		case IQ_PLAY:
            if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
            {
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 16 ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	//TODO TVB599
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					break;
				}
				
#ifdef WIN32				
				RestoreDisplay(nBoardNum);
#else
				SNMP_Send_Status(TVB390_IQPLAY_SPECTRUM);
#endif
                gWrapDll.Configure_Modulator(0);
            }
			break;
		//2011/11/29 TVB594
		case VSB_8:
		case ATSC_MH:
		//case MULTIPLE_VSB:
            if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
			{
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					break;
				}
#ifdef WIN32				
				RestoreDisplay(nBoardNum);
#else
				SNMP_Send_Status(TVB390_IQPLAY_SPECTRUM);
#endif
                gWrapDll.Configure_Modulator(0);
			}
			break;
	}
}
//-----------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnCbnSelchangeParam2()
#else
void PlayForm::OnCbnSelchangeParam2(int iCurIx)
#endif
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;
	long	lConst;
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM2);
#endif
    //CMMB
    if(iMod == CMMB)
        return;
#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif

	switch (iMod)
	{
		case DVB_T:	// const
		case DVB_H:	// const
		//2012/6/28 multi dvb-t
		case MULTIPLE_DVBT:
            gpConfig->gBC[nBoardNum].gnConstellation = iCurIx;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 5);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            if (iMod == DVB_T)
                SNMP_Send_Status(TVB390_DVB_T_H_CONSTELLATION);
            else
                SNMP_Send_Status(TVB390_DVB_H_CONSTELLATION);
			break;
		case QAM_B:	// interleave
		case MULTIPLE_QAMB:
            gpConfig->gBC[nBoardNum].gnQAMInterleave = iCurIx;
            gWrapDll.Set_QAMB_Parameters(nBoardNum, 2); // Interleave
            SNMP_Send_Status(TVB390_QAM_B_INTERLEAVE);
			break;

		case QAM_A:	// Spectrum Inversion
			if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
			{
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					break;
				}

#ifdef WIN32
				RestoreDisplay(nBoardNum);
#else
				SNMP_Send_Status(TVB390_IQPLAY_SPECTRUM);
#endif
                gWrapDll.Configure_Modulator(0);
			}
			break;

		case QPSK:	// RRC Filter
			//2010/10/7
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || 
				gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
			{
	            if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
		        {
			        gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					SNMP_Send_Status(TVB390_QPSK_SPECTRUM);
	            }
			}
			else
			{
				gpConfig->gBC[nBoardNum].gnRollOffFactor = iCurIx;
			    gWrapDll.Set_QPSK_Parameters(nBoardNum, 3);
#ifdef WIN32
				CheckSymbolRate();
				UpdateSymbolToolTip(nBoardNum);
#endif
	            SNMP_Send_Status(TVB390_QPSK_RRC_FILTER);
			}
			break;

		case DVB_S2:// coderate
            gpConfig->gBC[nBoardNum].gnCodeRate = iCurIx;
            gWrapDll.Set_DVBS2_Parameters(nBoardNum, 2);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            SNMP_Send_Status(TVB390_DVB_S2_CODERATE);
			break;

		case DTMB:	// constellation
            gpConfig->gBC[nBoardNum].gnConstellation = iCurIx;
                
            //--- Enable Disable coderate
            lConst = gpConfig->gBC[nBoardNum].gnConstellation;
            if (lConst == CONST_DTMB_4QAM_NR || lConst == CONST_DTMB_32QAM) //&&
            {
#ifdef WIN32
                gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, CONST_DTMB_CODE_7488_6016);
#endif
				gpConfig->gBC[nBoardNum].gnCodeRate = CONST_DTMB_CODE_7488_6016;
#ifdef WIN32
				gBaseUtil.SetEnableControl(Combo_PARAM3, FALSE);
            } else
				gBaseUtil.SetEnableControl(Combo_PARAM3, TRUE);
#else
			}
#endif

            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_CONSTELLATION);

			break;
		//2010/12/07 =================================================================================================================================
		case ISDB_S:	//coderate
			gpConfig->gBC[nBoardNum].gnCodeRate = iCurIx;
			if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK)
				gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_1_2;
			else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
				gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_2_3;

#ifdef WIN32
			if(gBaseUtil.SelectedIndex_Combo(Combo_PARAM1) == CONST_ISDBS_BPSK)
			{
				if(gBaseUtil.SelectedIndex_Combo(Combo_PARAM2) != CONST_ISDBS_CODE_1_2)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, CONST_ISDBS_CODE_1_2);
			}
			else if(gBaseUtil.SelectedIndex_Combo(Combo_PARAM1) == CONST_ISDBS_TC8PSK)
			{
				if(gBaseUtil.SelectedIndex_Combo(Combo_PARAM2) != CONST_ISDBS_CODE_2_3)
					gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, CONST_ISDBS_CODE_2_3);
			}
#endif
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
			break;
		//============================================================================================================================================
		//2011/4/26 DVB_C2 ===========================================================================================================================
		case DVB_C2:
			if(gpConfig->gBC[nBoardNum].gnDVB_C2_Guard == iCurIx)
				return;
			gpConfig->gBC[nBoardNum].gnDVB_C2_Guard = iCurIx;
			if( iCurIx == C2_GUARD_INDEX_0 )
			{
#ifdef WIN32
				Num_Start->Increment = 24;
#endif
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth < 0 || gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth > 1)
					gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth = 0;

				if(gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart < 0 || gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart > 8191)
					gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart = 0;
			}	
#ifdef WIN32
			else
			{
				Num_Start->Increment = 12;
			}

			Set_Center_Freq(gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq);
			for(int i = 0; i < DVB_C2_MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					gWrapDll.TSPH_SET_C2MI_PARAMS(
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
#else
			SetDVBC2_parameters();
#endif
			break;
	}
}
//-----------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnCbnSelchangeParam3()
#else
void PlayForm::OnCbnSelchangeParam3(int iCurIx)
#endif
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;	
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM3); 
#endif

	    //CMMB
    if(iMod == CMMB)
        return;
#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DVB_T:	// coderate
		case DVB_H:	// coderate
		case MULTIPLE_DVBT:
            gpConfig->gBC[nBoardNum].gnCodeRate = iCurIx;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 2);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            if (iMod == DVB_T)
                SNMP_Send_Status(TVB390_DVB_T_H_CODERATE);
            else
                SNMP_Send_Status(TVB390_DVB_H_CODERATE);
			break;

		case QAM_B:	// Spectrum Inversion
		//case MULTIPLE_QAMB:
			if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
			{
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF ||
					gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					break;
				}
#ifdef WIN32
				RestoreDisplay(nBoardNum);
#else
				SNMP_Send_Status(TVB390_IQPLAY_SPECTRUM);
#endif
                gWrapDll.Configure_Modulator(0);
			}
			break;

		case QPSK:	// spectrum
            if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
            {
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || 
					gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					SNMP_Send_Status(TVB390_QPSK_SPECTRUM);
					break;
				}
#ifdef WIN32
				RestoreDisplay(nBoardNum);
#endif
                gWrapDll.Configure_Modulator(0);
#ifndef WIN32
				Sleep(300);
#endif
                SNMP_Send_Status(TVB390_QPSK_SPECTRUM);
            }
			break;

		case DVB_S2:// pilot
            gpConfig->gBC[nBoardNum].gnPilot = iCurIx;
            gWrapDll.Set_DVBS2_Parameters(nBoardNum, 6);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            SNMP_Send_Status(TVB390_DVB_S2_PILOT);
			break;

		case DTMB:	// coderate
			gpConfig->gBC[nBoardNum].gnCodeRate = iCurIx;
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_CODERATE);

			break;
	}
}	 
//-----------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnCbnSelchangeParam4()
#else
void PlayForm::OnCbnSelchangeParam4(int iCurIx)
#endif
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM4); 
#endif
	char	text[100];
    //CMMB
    if(iMod == CMMB)
        return;
#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DVB_T:	// txmode
		case DVB_H:	// txmode
		case MULTIPLE_DVBT:
            gpConfig->gBC[nBoardNum].gnTxmode = iCurIx;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 4);
            if (iMod == DVB_T)
                SNMP_Send_Status(TVB390_DVB_T_H_TXMODE);
            else
                SNMP_Send_Status(TVB390_DVB_H_TXMODE);
			break;

		case DVB_S2:// roll off
            gpConfig->gBC[nBoardNum].gnRollOffFactor = iCurIx;
            gWrapDll.Set_DVBS2_Parameters(nBoardNum, 3);

            if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
            {
				;
            }
#ifdef WIN32
			else
            {
				gBaseUtil.SelectedText_Combo(Combo_PARAM4, text);
                if (atoi(text) > 25000000)
                    Display_SymbolRate(25000000);
            }
			CheckSymbolRate();
			UpdateSymbolToolTip(nBoardNum);
#endif
            SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
			break;

		case DTMB:	// pilot insertion 
            gpConfig->gBC[nBoardNum].gnPilotInsertion = iCurIx;
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_PILOT_INSERTION);
			break;
	}
}			
//-----------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnCbnSelchangeParam5()
#else
void PlayForm::OnCbnSelchangeParam5(int iCurIx)
#endif
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;	
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM5);
#endif
    //CMMB
    if(iMod == CMMB)
        return;

#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DVB_T:	// guard
		case DVB_H:	// guard
		case MULTIPLE_DVBT:
            gpConfig->gBC[nBoardNum].gnGuardInterval = iCurIx;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 3);
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            if (iMod == DVB_T)
                SNMP_Send_Status(TVB390_DVB_T_H_GUARD_INTERVAL);
            else
                SNMP_Send_Status(TVB390_DVB_H_GUARD_INTERVAL);
			break;

		case DVB_S2:// spectrum
            if (gpConfig->gBC[nBoardNum].gnSpectrumInverse != iCurIx)
            {
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = iCurIx;
                if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || 
					gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId ==12 || gpConfig->gBC[nBoardNum].gnBoardId ==16)	//2013/5/27 TVB599 0xC
				{
#ifdef WIN32
					gWrapDll.TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#else
					TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
#endif
					SNMP_Send_Status(TVB390_DVB_S2_SPECTRUM);
					break;
				}
#ifdef WIN32
                RestoreDisplay(nBoardNum);
#endif
                gWrapDll.Configure_Modulator(0);
                SNMP_Send_Status(TVB390_DVB_S2_SPECTRUM);
            }
			break;

		case DTMB:	// interleave(b/m)
			gpConfig->gBC[nBoardNum].gnQAMInterleave = iCurIx;
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_INTERLEAVE);
			break;
	}
}
//-----------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnCbnSelchangeParam6()
#else
void PlayForm::OnCbnSelchangeParam6(int iCurIx)
#endif
{
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM6);
#endif
    //CMMB
    if(iMod == CMMB)
        return;

#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DTMB:	// frame header
            gpConfig->gBC[nBoardNum].gnFrameHeader = iCurIx;
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
            SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER);
			break;
	}
}
//-----------------------------------------------------------------------
#ifdef WIN32
void PlayForm::OnCbnSelchangeParam7()
#else
void PlayForm::OnCbnSelchangeParam7(int iCurIx)
#endif
{
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;	
#ifdef WIN32
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM7);
#endif
    //CMMB
    if(iMod == CMMB)
        return;
#ifdef WIN32
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
#endif
	switch (iMod)
	{
		case DTMB:	// carrier number
			gpConfig->gBC[nBoardNum].gnCarrierNumber = iCurIx;
            Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
            gWrapDll.Set_DTMB_Parameters(nBoardNum);
#ifdef WIN32
            if (gpConfig->gBC[nBoardNum].gnCarrierNumber == CONST_DTMB_CARRIER_NUMBER_1)
				gBaseUtil.SetEnableControl(Combo_PARAM4, FALSE);
            else
				gBaseUtil.SetEnableControl(Combo_PARAM4, TRUE);	
#endif
			SNMP_Send_Status(TVB390_DTMB_CARRIER_NUMBER);
			break;
	}
}



//------------------------------------------------------

void PlayForm::OnSliderChange(int nSlider_Pos)
{

	long    nBoardNum = gGeneral.gnActiveBoard;
	if(gpConfig->gBC[nBoardNum].gnCurrentPosScrolled == 1)
	{
		if(gpConfig->gBC[nBoardNum].gnCurrentPosChanged == 0 )
		{
			//2012/5/15
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
				gpConfig->gBC[nBoardNum].gnCurrentPos = (__int64)((double)gpConfig->gBC[nBoardNum].dwFileSize / 7300.0 * 6144.0) * nSlider_Pos / 99;
			else
				gpConfig->gBC[nBoardNum].gnCurrentPos = gpConfig->gBC[nBoardNum].dwFileSize * nSlider_Pos / 99;
#ifdef WIN32
			gWrapDll.TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double) gpConfig->gBC[nBoardNum].gnCurrentPos);
#else
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, (double) gpConfig->gBC[nBoardNum].gnCurrentPos);
#endif
			gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 1;
		}
		gpConfig->gBC[nBoardNum].gnCurrentPosScrolled = 0;
	}
}
//------------------------------------------------------------------------------
//TVB590S
#ifdef WIN32 
void PlayForm::Check_590S_Board()
#else 
void PlayForm::Check_590S_Board()
#endif
{
#ifndef WIN32 
	 if(gpConfig->gBC[gGeneral.gnActiveBoard].gnChangeModFlag == 1)
		 return;
#endif
	long nStatus;
#ifdef WIN32
	static int cnt = 0;
	static int error_pll = 0;
	static int boardStatusCheck = 0;
#endif
	int		i;

	if(gGeneral.gnActiveBoard < 0)
		return;

	for (i = 0; i < MAX_MODULATORMODE; i++)
	{
		if(gpConfig->gBC[gGeneral.gnActiveBoard].gbEnabledType[i] == 1)
		{
			break;
		}
	}
	
	if(i >= MAX_MODULATORMODE && gpConfig->gBC[gGeneral.gnActiveBoard].gn_IsVirtualSlot == 0)
		return;

	
	for(i = 0 ; i  <= MAX_BOARD_COUNT; i++)
	{
		if(gpConfig->nBoardRealSlot[i] > -1)
		{
			int nBoardNum = gpConfig->nBoardRealSlot[i];
			if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
				continue;
			//2010/6/29 Fault/Status trigger
			if(gpConfig->gBC[nBoardNum].gnBoardId == 59 || gpConfig->gBC[nBoardNum].gnBoardId == 60 || 
				gpConfig->gBC[nBoardNum].gnBoardId == 61 || gpConfig->gBC[nBoardNum].gnBoardId == 10)
			{
				if((gpConfig->gBC[nBoardNum].gnBoardAuth & 0x1000) > 0)
				{
					if(gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
					{
#ifdef WIN32
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 1);
						Sleep(1);
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
#else
						TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 1);
						Sleep(1);
						TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
#endif
					}
				}
			}
			//2010/10/5 TVB593
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16)	/* 2012/1/31 TVB591S */ //2011/2/15 added 11(TVB597V2)
			{
				if(gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
				{
#ifdef WIN32
					gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 1);
					Sleep(1);
					gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
#else
					TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 1);
					Sleep(1);
					TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
#endif
				}
#ifdef WIN32
				else
				{
					if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && gpConfig->gBC[nBoardNum].gnT2MI_StreamGeneration == 1)
					{
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 1);
						Sleep(1);
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 0, 0);
					}
				}
#endif
			}
			else if(gpConfig->gBC[nBoardNum].gnBoardId == 16 || gpConfig->gBC[nBoardNum].gnBoardId == 12)
			{
				if(gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
				{
#ifdef WIN32
					if((gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 ||
						gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH) && (gpConfig->gBC[nBoardNum].gnInputSource == DVB_ASI_IN || gpConfig->gBC[nBoardNum].gnInputSource == SMPTE_310M_IN))
					{
						if(gpConfig->gBC[nBoardNum].gnErrorStatus == 3 || gpConfig->gBC[nBoardNum].gnErrorStatus == 4 || gGeneral.gnSetParam_Asi == 1)
						{
							gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 2);
							Sleep(1);
							gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 0);
						}
					}
					else
#endif
					{
#ifdef WIN32
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 2);
						Sleep(1);
						gWrapDll.TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 0);
#else
						TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 2);
						Sleep(1);
						TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 3, 0);
#endif
					}
				}
			}
#ifdef WIN32			
			if(boardStatusCheck == 9)
			{
				if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
				{

					if(gpConfig->gBC[nBoardNum].gnBoardStatusReset  > 1)
					{
						gpConfig->gBC[nBoardNum].gnBoardStatusReset--;
					}
					else if(gpConfig->gBC[nBoardNum].gnBoardStatusReset == 1)
					{
						if(gWrapDll.Func_TSPL_GET_TSIO_STATUS_EX(nBoardNum, 1) < 0)
							gpConfig->gBC[nBoardNum].gnBoardStatusReset = 1;
						else
							gpConfig->gBC[nBoardNum].gnBoardStatusReset = 0;
					}
					else
					{
						int option;
						
						if(gpConfig->gBC[nBoardNum].bPlayingProgress == true || gpConfig->gBC[nBoardNum].bRecordInProgress == true)
						{
							option = 0x200;
						}
						else if(gpConfig->gBC[nBoardNum].gnInputSource == DVB_ASI_IN || gpConfig->gBC[nBoardNum].gnInputSource == SMPTE_310M_IN)
						{
							if(gpConfig->gBC[nBoardNum].gnAsiLock_status == 0 || (gGeneral.gnActiveBoard == nBoardNum && gGeneral.gnSetParam_Asi == 1))
							{
								option = 0x500;
							}
							else
							{
								option = 0x600;
							}
						}
						else
						{
							option = 0x100;
						}
						nStatus = gWrapDll.Func_TSPL_GET_TSIO_STATUS_EX(nBoardNum, option); 
						if(nStatus > 0)
						{
							if(gGeneral.gnActiveBoard == nBoardNum)
							{
								if(nStatus & 0x1)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("FPGA Continuity counter error");
								}
								else if(nStatus & 0x40)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("DAC PLL unlock");
								}
								else if(nStatus & 0x100)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Up-converter PLL2 unlock");
								}
								else if(nStatus & 0x10)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Playout sync loss");
								}
								else if(nStatus & 0x20)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("TS IN sync loss");
								}
								else if(nStatus & 0x80)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Up-converter PLL1 unlock");
								}
								else if(nStatus & 0x200)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Rate adapter block TS align error");
									if((gpConfig->gBC[nBoardNum].gnInputSource == DVB_ASI_IN || gpConfig->gBC[nBoardNum].gnInputSource == SMPTE_310M_IN) &&
										(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H))
									{
#ifdef WIN32
										gWrapDll.TVB59x_SET_Reset_Control_REG_EX(nBoardNum, 3);
#else
										TVB59x_SET_Reset_Control_REG_EX(nBoardNum, 3);
#endif
									}
								}
								else if(nStatus & 0x400)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Modulator output buffer empty");
								}
								else if(nStatus & 0x800)
								{
									if(gGeneral.gnActiveBoard == nBoardNum)
										gUtilInd.LogMessage("Modulator output buffer full");
								}
							}
							gpConfig->gBC[nBoardNum].gnBoardStatusReset = 1;
						}
					}
				}
#ifdef WIN32
				else if((gpConfig->gBC[nBoardNum].gnModulatorMode != ASI_OUTPUT_MODE) && (gpConfig->gBC[nBoardNum].bPlayingProgress == true || gpConfig->gBC[nBoardNum].bRecordInProgress == true) && 
					(gpConfig->gBC[nBoardNum].gnBoardId == 15 || gpConfig->gBC[nBoardNum].gnBoardId == 22))
				{
					nStatus = gWrapDll.TSPL_GET_AD9775_EX(gGeneral.gnActiveBoard, 0);
					nStatus = (long)(nStatus / 2);
					if((nStatus & 1) == 0)
					{
						if(gGeneral.gnActiveBoard == nBoardNum)
							gUtilInd.LogMessage("DAC(AD9775) PLL unlocked.");
					}
				}
#endif
			}
#endif
		}
	}
#ifdef WIN32
	boardStatusCheck++;
	if(boardStatusCheck == 10)
		boardStatusCheck = 0;
#endif

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == ASI_OUTPUT_MODE)
	{
#ifndef WIN32
		 //Set_Auto_Play();
#endif
		return;
	}
#ifdef WIN32
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2 && gpConfig->gBC[gGeneral.gnActiveBoard].gnOutputType == OUTPUT_FILE)
		return;
#endif

	//2010/10/5 TVB593
	if((gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 20 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0xF || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 11 ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0x16 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 12 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 16) && 
		(gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false && gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == false))	//2013/5/27 TVB599 0xC
	{
#ifdef WIN32
		nStatus = gWrapDll.TSPL_GET_AD9775_EX(gGeneral.gnActiveBoard, 0);
#else
		nStatus = TSPL_GET_AD9775_EX(gGeneral.gnActiveBoard, 0);
#endif
		nStatus = (long)(nStatus / 2);
		if((nStatus & 1) == 0)
		{
			gpConfig->gBC[gGeneral.gnActiveBoard].gnErrorStatus = 10;
#ifdef WIN32
			
			if(error_pll > 9)
			{
				gUtilInd.LogMessage("DAC(AD9775) PLL unlocked.");
				cnt = 0;
			}
			else
			{
				error_pll++;
			}
			if(gBaseUtil.IsEnabled_Common(ComF1) == true)
			{	
				//gBaseUtil.SetEnableControl(ComF1, FALSE);
				gBaseUtil.SetEnableControl(ComF6, FALSE);
				this->Playback_sub_StartCapture->Enabled = false;
				this->Playback_sub_StartPlayback->Enabled = false;
			}
#endif
		}
		else
		{
#ifdef WIN32
			gWrapDll.TSPL_WRITE_CONTROL_REG_EX(gGeneral.gnActiveBoard, 0, 0x55100, 1);
			error_pll = 0;
			if(cnt == 0)
			{
				gUtilInd.LogMessage("");
				cnt = 1;
			}
			if(gBaseUtil.IsEnabled_Common(ComF1) == false)
			{	
				if(gGeneral.gnRearrange_flag == 0)
					gBaseUtil.SetEnableControl(ComF1, TRUE);
			}
#else
			TSPL_WRITE_CONTROL_REG_EX(gGeneral.gnActiveBoard, 0, 0x55100, 1);
            //Set_Auto_Play();
#endif
		}
	}
}
//---------------------------------------------------------------------------
// From open_system: check
// - gnCmdPlayrate
// - gnCmdFileName
// - License Manager
// Description: Set ELabPlayRate
//              Check # of board, check LN
#ifdef WIN32
void PlayForm::Initial_Check(long nBoardNum)
{

//2013/3/12 TODO
    if (strlen(gpConfig->gBC[nBoardNum].gnCmdFileName) > 0)
    {
		if(gpConfig->gBC[nBoardNum].gnCmdAutoRun == 0)
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdFileName, 256, "");
		else
		{
			gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 512, "%s\\%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gnCmdFileName);
		}
    }
	//----------------------------------------------------------------
    if (gpConfig->gBC[nBoardNum].gnCmdPlayrate > 0)
    {
        Display_PlayRate(gpConfig->gBC[nBoardNum].gnCmdPlayrate);
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnCmdAutoRun == 1 || gpConfig->gBC[nBoardNum].gnCmdAutoRun == 4)	//2010/12/14 CommandLine added
        {
            long    playRate;

            playRate = gWrapDll.Get_Playrate(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0);

            if (playRate > 0)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = playRate;
            else
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = gpConfig->gBC[nBoardNum].gdwPlayRate;

            if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = 2433331;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = 38785316;

            Display_PlayRate(gpConfig->gBC[nBoardNum].gnCmdPlayrate);
        }
    }

    //-----------------------------------
    // Confirm License Information
    if (gpConfig->gBC[nBoardNum].gnBoardId == -1)       // No Board exist
    {
        gUtilInd.LogMessageInt(TLV_FAIL_TO_DETECT_BOARD);
		gUtility.DisplayMessage("There is no available board. Will be exit!");
        exit(0);
    } else if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        if (gpConfig->gBC[nBoardNum].gbEnabledType[gpConfig->gBC[nBoardNum].gnModulatorMode] <= 0)
        {
            gpConfig->gBC[nBoardNum].gnOpenSystem = 0;
			//--- Do License Input
			gFormLicenseInput.SetBoardID(gpConfig->gBC[nBoardNum].gnBoardId);
			gFormLicenseInput.SetBoardNum(nBoardNum);
			gFormLicenseInput.SetBoardSN(gWrapDll.Get_AdaptorInfo(nBoardNum, 0));
			if(gGeneral.InputInit == 1)
			{
				gGeneral.InputInit = 0;
			}
			else
			{
				gFormLicenseInput.ShowDialog(this);
			}
			if(gpConfig->gBC[nBoardNum].downflag == 1)
			{
				gWrapDll.Open_System(nBoardNum);
				//2010/1/7 Added
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszOldLN, 40, gWrapDll.Get_AdaptorInfo(nBoardNum, 1));

			}
			else
			{
				this->Group_Status->Visible = false;
				this->Group_Rate->Visible = false;
				this->Group_Path->Visible = false;
				this->Group_Rf->Visible = false;
				this->Group_Modulator->Visible = false;
				this->Menu_Playback->Enabled = false;
				this->Menu_Configuration->Enabled = false;
				this->Advanced_IP_AV->Visible = false;
				this->Advanced_Err_Insertion->Visible = false;
				this->Advanced_IQ->Visible = false;
				this->Advanced_Schedule->Visible = false;
				this->Advanced_PCRRestamping->Visible = false;
				this->Advanced_Separator1->Visible = false;
				this->Advanced_System->Visible = false;
				gBaseUtil.ClearItem_Combo(Combo_MODULATOR_TYPE);
				gBaseUtil.SetText_Combo(Combo_MODULATOR_TYPE, "");
			}
        }
		else
		{
			gBaseUtil.SetEnableControl(ComF1, TRUE);
			gBaseUtil.SetEnableControl(ComF2, TRUE);
			UpdateModulatorConfigUI(nBoardNum);
		}
    }
	else
	{
		gBaseUtil.SetEnableControl(ComF1, TRUE);
		gBaseUtil.SetEnableControl(ComF2, TRUE);
		UpdateModulatorConfigUI(nBoardNum);
	}
}
#else
void PlayForm::Initial_Check(long nBoardNum)
{

    char			text[100];

    if (strlen(gpConfig->gBC[nBoardNum].gnCmdFileName) > 0)
    {
        int i,j,k;

		j = gpConfig->gBC[nBoardNum].nPlayListIndexCount;
        k = 0;
        
        for (i = 0; i < j; i++)
        {
			if (strcmp(gpConfig->gBC[nBoardNum].szPlayFileList[i], gpConfig->gBC[nBoardNum].gnCmdFileName) == 0)
            {
            	gpConfig->gBC[nBoardNum].nPlayListIndexCur = i;
				OnLbnSelchangePlaylist();
                k = 1;
                break;
            }
        }

        if (k == 1)
		{
			if(gpConfig->gBC[nBoardNum].gnCmdAutoRun == 0)
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdFileName, 256, "");
		}
		else
        {
            j = gpConfig->gBC[nBoardNum].nFileListIndexCount;
            for (i = 0; i < j; i++)
            {
                if (strcmp(gpConfig->gBC[nBoardNum].szFileFileList[i], gpConfig->gBC[nBoardNum].gnCmdFileName) == 0)
                {
					gpConfig->gBC[nBoardNum].nFileListIndexCur = i;
					OnLbnSelchangeFilelistbox();
					break;
                }
            }
			if(gpConfig->gBC[nBoardNum].gnCmdAutoRun == 0)
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnCmdFileName, 256, "");
        }
    }
	//----------------------------------------------------------------
    // CmdPlayrate: Input Playrate from command line
    if (gpConfig->gBC[nBoardNum].gnCmdPlayrate > 0)
    {
        Display_PlayRate(gpConfig->gBC[nBoardNum].gnCmdPlayrate);
    } else
    {
        if (gpConfig->gBC[nBoardNum].gnCmdAutoRun == 1 || gpConfig->gBC[nBoardNum].gnCmdAutoRun == 4)	//2010/12/14 CommandLine added
        {
            long    playRate;
			int		index;
			index = gpConfig->gBC[nBoardNum].nFileListIndexCur;
            if (index >= 0)
			{
				gUtility.MyStrCpy(text, 100, gpConfig->gBC[nBoardNum].szFileFileList[index]);
				gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, text);
			}

            playRate = gWrapDll.Get_Playrate(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0);

            if (playRate > 0)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = playRate;
            else
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = gpConfig->gBC[nBoardNum].gdwPlayRate;

            if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = 2433331;
            else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                gpConfig->gBC[nBoardNum].gnCmdPlayrate = 38785316;

            Display_PlayRate(gpConfig->gBC[nBoardNum].gnCmdPlayrate);
        }
    }

    //-----------------------------------
    // Confirm License Information
    if (gpConfig->gBC[nBoardNum].gnBoardId == -1)       // No Board exist
    {
        gUtilInd.LogMessageInt(TLV_FAIL_TO_DETECT_BOARD);
		gUtility.DisplayMessage("There is no available board. Will be exit!");
        exit(0);
    } else if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	/* 2012/1/31 TVB591S */ //2011/2/15 added 11(TVB597V2) //2010/10/5 added 0xF //44,45,47,48,59,60,10(595D),20(590S)
    {
        if (gpConfig->gBC[nBoardNum].gbEnabledType[gpConfig->gBC[nBoardNum].gnModulatorMode] <= 0)
        {
            gpConfig->gBC[nBoardNum].gnOpenSystem = 0;
        }
		else
		{
			UpdateModulatorConfigUI(nBoardNum);
		}
    }
	else
	{
		UpdateModulatorConfigUI(nBoardNum);
	}
}
#endif
//---------------------------------------------------------------------------
void PlayForm::UpdateRFPowerLevelUI(long nBoardNum)
{
}

//------------------------------------------------------
#ifdef WIN32 
void PlayForm::OnBnClickedParam()
{
    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

	if (iMod != ISDB_T && iMod != ISDB_T_13 && iMod != ISDB_S)
		return;
	
	HCURSOR hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	if(hCursor)
		::SetCursor(hCursor);
	//2010/12/07 ISDB-S ================================================================================
	if(iMod == ISDB_S)
	{
		if(gBaseUtil.isChecked_Check(Check_USE_TMCC) == true)
		{
			this->Configuration_Modulator->Enabled = true;
			for(int jjj = 0; jjj < this->Configuration_Modulator->DropDownItems->Count; jjj++)
			{
				this->Configuration_Modulator->DropDownItems[jjj]->Enabled = true;
			}
			Combo_PARAM1->Enabled = false;
			this->Modulator_Param1->Enabled = false;

			Combo_PARAM2->Enabled = false;
			this->Modulator_Param2->Enabled = false;
			
			gBaseUtil.SetEnableControl(Command_PARAM, TRUE);
			this->Modulator_Param5->Enabled = true;


			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
			//2012/9/10 new menu 
			this->Modulator_Param3->Checked = true;

			this->Panel_File->Visible = false;

			this->Btn_View_Param->Visible = true;
			this->Btn_View_Param->Enabled = false;
			//2010/12/23 FIXED
			//gBaseUtil.SetEnableControl(ComDir, FALSE);
			//
			////2011/4/4 ISDB-S Bitrate
			//gBaseUtil.SetShowContol(LTSRATE, SW_HIDE);
			//gBaseUtil.SetShowContol(LPACKETSIZE, SW_HIDE);
			//gBaseUtil.SetShowContol(LabPlayRateCalc, SW_HIDE);
			//gBaseUtil.SetShowContol(Label_TS, SW_HIDE);
			
			int ts_num, m;
			ts_num = 0;
			////-----------------------
			for(m = 0 ; m < MAX_TS_COUNT ; m++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[m] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[m]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[m] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[m] <= MAX_SLOT_COUNT)
				{
					gWrapDll.TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, gpConfig->gBC[nBoardNum].gszTS_M[m], gpConfig->gBC[nBoardNum].gnConstellation_M[m],
						gpConfig->gBC[nBoardNum].gnCoderate_M[m], gpConfig->gBC[nBoardNum].gnSlotCount_M[m]);
					ts_num = ts_num + 1;
				}
			}
			if(ts_num == 0)
			{
				gWrapDll.TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, "", -1, -1, 0);
			}
		} else
		{
			this->Configuration_Modulator->Enabled = true;
			for(int jjj = 0; jjj < this->Configuration_Modulator->DropDownItems->Count; jjj++)
			{
				this->Configuration_Modulator->DropDownItems[jjj]->Enabled = true;
			}
			Combo_PARAM1->Enabled = true;
			this->Modulator_Param1->Enabled = true;

			if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK || gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
			{
				Combo_PARAM2->Enabled = false;
				this->Modulator_Param2->Enabled = false;
			}
			else
			{
				Combo_PARAM2->Enabled = true;
				this->Modulator_Param2->Enabled = true;
			}
			
			gBaseUtil.SetEnableControl(Command_PARAM, FALSE);
			this->Modulator_Param5->Enabled = false;


			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
			//2012/9/10 new menu 
			this->Modulator_Param3->Checked = false;

			this->Panel_File->Visible = true;
			
			this->Btn_View_Param->Visible = false;

			////2010/12/23 FIXED
			//gBaseUtil.SetEnableControl(ComDir, TRUE);

			////2011/4/4 ISDB-S Bitrate
			//gBaseUtil.SetShowContol(LTSRATE, SW_SHOW);
			//gBaseUtil.SetShowContol(LPACKETSIZE, SW_SHOW);
			//gBaseUtil.SetShowContol(LabPlayRateCalc, SW_SHOW);
			//gBaseUtil.SetShowContol(Label_TS, SW_SHOW);

			//if(gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
			//{
			//	this->Configuration_Modulator->Enabled = false;
			//}

			//Display_File_Property(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
		}
	}
	else
	{
	//==================================================================================================
		if(gBaseUtil.isChecked_Check(Check_USE_TMCC) == true)
		{
			gBaseUtil.SetEnableControl(Command_PARAM, FALSE);
			gBaseUtil.SetEnableControl(Btn_View_Param, TRUE);
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
			//2012/9/10 new menu 
			this->Modulator_Param1->Checked = true;
			this->Modulator_Param3->Enabled = false;
		} else
		{
			gBaseUtil.SetEnableControl(Command_PARAM, TRUE);
			gBaseUtil.SetEnableControl(Btn_View_Param, FALSE);
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
			//2012/9/10 new menu 
			this->Modulator_Param1->Checked = false;
			this->Modulator_Param3->Enabled = true;
		}
	}

	//2010/9/13 FIXED - ISDB-T Emergency Broadcasting Control
	gWrapDll.Set_Tmcc_Remuxer(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);

	//2010/12/07 ==================================================================================================================================
	if(iMod == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
		{
			if(gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
			{
			    char		str[256];
			    char		strTemp[256];
				int bitrate;
				//2013/3/12 TODO
				//gBaseUtil.SelectedText_List(FileListBox, strTemp);
				gUtility.MySprintf(str, 256, "%s\\%s", gpConfig->gBC[nBoardNum].gszMasterDirectory , strTemp);
				bitrate = gWrapDll.TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(nBoardNum, str);
				Display_PlayRate(bitrate);
			}
			else
				Display_PlayRate(gUtilInd.CalcBurtBitrate(nBoardNum));
		}
		else
		{
			long i, j, k;
			j = 0;
			for(i = 0 ; i < MAX_TS_COUNT ; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1)
				{
					k = gUtilInd.CalcDatarate(gpConfig->gBC[nBoardNum].gnConstellation_M[i], gpConfig->gBC[nBoardNum].gnCoderate_M[i],
											gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
					j = j + k;
				}
			}
			if( j > 1)
			{
				Display_PlayRate(j);
			}
			UpdateCombinerUI(nBoardNum);

		}
	}
	//=============================================================================================================================================
	hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	if(hCursor)
		::SetCursor(hCursor);

    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
        SNMP_Send_Status(TVB390_ISDB_T_USE_TMCC);
    else
        SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);
}
#else
void PlayForm::OnBnClickedParam(int bChecked)
{
    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

	if (iMod != ISDB_T && iMod != ISDB_T_13 && iMod != ISDB_S && iMod != DVB_T2)	//2010/12/07 ISDB-S, 2010/12/22 DVB-T2 MULTI-PLP
		return;

	if(iMod == ISDB_S)
	{
		gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = bChecked;
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
		{
			int ts_num, m;
			ts_num = 0;
			////-----------------------
			for(m = 0 ; m < MAX_TS_COUNT ; m++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[m] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[m]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[m] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[m] <= MAX_SLOT_COUNT)
				{
					TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, gpConfig->gBC[nBoardNum].gszTS_M[m], gpConfig->gBC[nBoardNum].gnConstellation_M[m],
						gpConfig->gBC[nBoardNum].gnCoderate_M[m], gpConfig->gBC[nBoardNum].gnSlotCount_M[m]);
					ts_num = ts_num + 1;
				}
			}
			if(ts_num == 0)
			{
				TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, (char *)"", -1, -1, 0);
			}
		} else
		{
			long lindex = 0;
			char str[256];

			if (gpConfig->gBC[nBoardNum].nPlayListIndexCount == 0)
            {
				if(gpConfig->gBC[nBoardNum].nFileListIndexCount <= 0)
                    return;
                    
				lindex = gpConfig->gBC[nBoardNum].nFileListIndexCur;
				if (lindex < 0)
                    lindex = 0;

                gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 512, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].szFileFileList[lindex]);
            } else
            {
				lindex = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
                if (lindex < 0)
                    lindex = 0;

                gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 512, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].szPlayFileList[lindex]);
            }
			Display_File_Property(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
		}
	}
	else
	{
	//==================================================================================================
		if(bChecked == 1)
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
		} else
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
		}
	}

    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
        SNMP_Send_Status(TVB390_ISDB_T_USE_TMCC);
    else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
        SNMP_Send_Status(TVB390_ISDB_S_TMCC_USE);
	else
        SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);

	gWrapDll.Set_Tmcc_Remuxer(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);

	//2010/12/07 ==================================================================================================================================
	if(iMod == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
		{
			if(gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
			{
				int bitrate;
				bitrate = TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
				Display_PlayRate(bitrate);
			}
			else
			{
				Display_PlayRate(gUtilInd.CalcBurtBitrate(nBoardNum));
			}
		}
		else
		{
			long i, j, k;
			j = 0;
			for(i = 0 ; i < MAX_TS_COUNT ; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[i]) > 0 &&
					gpConfig->gBC[nBoardNum].gnSlotCount_M[i] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[i] <= MAX_SLOT_COUNT)
				{
					k = gUtilInd.CalcDatarate(gpConfig->gBC[nBoardNum].gnConstellation_M[i], gpConfig->gBC[nBoardNum].gnCoderate_M[i],
											gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
					j = j + k;
				}
			}
			Display_PlayRate(j);
		}
	}
	//=============================================================================================================================================
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
        SNMP_Send_Status(TVB390_ISDB_T_USE_TMCC);
    else
        SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);
}
#endif

//---------------------------------------------------------------
void PlayForm::UpdateAgcUI(long nBoardNum, double atten, long agc)
{
	double dwMax, dwMin, dwLevel, dwAGCAttenOffset, dwRet;
	double dwLevelMax, dwLevelMin;
	//2011/4/13 added
	if(gGeneral.gnChangeAdapt_Flag == 0) 
	{	
#ifdef WIN32	
		dwRet = gWrapDll.TVB380_SET_MODULATOR_AGC_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, agc, gpConfig->gBC[nBoardNum].gnUseTAT4710);	//2011/6/29 added UseTAT4710
        dwRet = gWrapDll.TVB380_SET_MODULATOR_ATTEN_VALUE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, atten, gpConfig->gBC[nBoardNum].gnUseTAT4710);	//2011/6/29 added UseTAT4710
        dwMax = gWrapDll.TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 5);
        dwMin = gWrapDll.TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 6);
        dwLevel = gWrapDll.TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 9);
        dwAGCAttenOffset = gWrapDll.TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 7);
#else
		if( gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20  || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || 
			gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
				gpConfig->gBC[nBoardNum].gnAGC = agc;
		}
		else
			gpConfig->gBC[nBoardNum].gnAGC = 0;
		//2010/7/14
		dwRet = TVB380_SET_MODULATOR_AGC_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnAGC, gpConfig->gBC[nBoardNum].gnUseTAT4710);
        dwRet = TVB380_SET_MODULATOR_ATTEN_VALUE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, atten, gpConfig->gBC[nBoardNum].gnUseTAT4710);
        dwMax = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 5);
        dwMin = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 6);
        dwLevel = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 9);
        dwAGCAttenOffset = TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, 7);
#endif
		gpConfig->gBC[nBoardNum].gnDisRFLevel = dwLevel;
	
		//2012/4/16 Multiple VSB,QAM-B
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			int MultiLevelOffset;
			MultiLevelOffset = GetMultiOptionLevelOffset(gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBoardId, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			if((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
			{
				MultiLevelOffset = 0;
			}
			gpConfig->gBC[nBoardNum].gnDisRFLevel = gpConfig->gBC[nBoardNum].gnDisRFLevel - MultiLevelOffset;
		}
#ifdef WIN32
		//2012/2/7 RF LEVEL OFFSET
		char strFilePath[512];
		double dwRFLevelOffset;
		gUtility.MySprintf(strFilePath,512,"%s\\%s_%d_%d.txt",gGeneral.gnStrCurDir, RF_LEVEL_OFFSET_FILE_NAME, nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
		if(gUtility.Is_File_Exist(strFilePath))
		{
			dwRFLevelOffset = Get_RF_Level_Offset(strFilePath, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			gpConfig->gBC[nBoardNum].gnDisRFLevel = gpConfig->gBC[nBoardNum].gnDisRFLevel + dwRFLevelOffset;
		}
#endif

		gpConfig->gBC[nBoardNum].gnAtten_Max = dwMax;
		gpConfig->gBC[nBoardNum].gnAtten_Min = dwMin;

		//2011/4/13
		gpConfig->gBC[nBoardNum].gdwAGCAttenOffset = dwAGCAttenOffset;
	    
		gpConfig->gBC[nBoardNum].gnMaxRFLevel = (float)dwMax;
		gpConfig->gBC[nBoardNum].gnMinRFLevel = (float)dwMin;
 
		dwLevelMax = gpConfig->gBC[nBoardNum].gnDisRFLevel + atten;
        dwLevelMin = dwLevelMax - dwMax;

		gpConfig->gBC[nBoardNum].gdwLevel_Max_display = dwLevelMax;
		gpConfig->gBC[nBoardNum].gdwLevel_Min_display = dwLevelMin;
	}

	//2011/12/1 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		int ownerSlot = gpConfig->gBC[nBoardNum].gn_OwnerSlot;
		gpConfig->gBC[nBoardNum].gnDisRFLevel = gpConfig->gBC[ownerSlot].gnDisRFLevel;
		gpConfig->gBC[nBoardNum].gnAtten_Max = gpConfig->gBC[ownerSlot].gnAtten_Max;
		gpConfig->gBC[nBoardNum].gnAtten_Min = gpConfig->gBC[ownerSlot].gnAtten_Min;
		gpConfig->gBC[nBoardNum].gdwAGCAttenOffset = gpConfig->gBC[ownerSlot].gdwAGCAttenOffset;
		gpConfig->gBC[nBoardNum].gdwLevel_Max_display = gpConfig->gBC[ownerSlot].gdwLevel_Max_display;
		gpConfig->gBC[nBoardNum].gdwLevel_Min_display = gpConfig->gBC[ownerSlot].gdwLevel_Min_display;
		gpConfig->gBC[nBoardNum].gnMaxRFLevel = gpConfig->gBC[ownerSlot].gnMaxRFLevel;
		gpConfig->gBC[nBoardNum].gnMinRFLevel = gpConfig->gBC[ownerSlot].gnMinRFLevel;
	}

#ifdef WIN32
	Display_Amplitude_Atten_Value(nBoardNum);
#endif
}
//------------------------------------------------------------------------------------------------------------------

#ifdef WIN32
void PlayForm::SNMP_Send_Status(int iMsgType)
{
    char    strMsg[100];
    char    text[256];
    long    nBoardNum = gGeneral.gnActiveBoard;
	int		iValue, i;
    switch(iMsgType)
    {
        //----------------------------------------------------
        //--- BOARD (4)
        case TVB390_BOARD_ID:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_BOARD_ID, gpConfig->gBC[nBoardNum].gnBoardId);
            break;
        case TVB390_BOARD_AUTHORIZATION:
            iValue = 0;
            for (i = MAX_MODULATORMODE-1; i >= 0; i--)
			{
				if(i != ISDB_T_13)
				{
					iValue = (iValue << 1) + gpConfig->gBC[nBoardNum].gbEnabledType[i];
				}
			}
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_BOARD_AUTHORIZATION, iValue);
            break;
        case TVB390_BOARD_LOCATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_BOARD_LOCATION, gpConfig->nBoardSlotNum[nBoardNum]);
            break;
        case TVB390_BOARD_INSTALLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_BOARD_INSTALLATION, gGeneral.gnBoardNum);
            break;

        //----------------------------------------------------
        //--- STATUS(15)
        case TVB390_BITRATE:
			gBaseUtil.GetText_Label(LabPlayRateCalc, text);
            iValue = atoi(text);
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_BITRATE, iValue);
            break;
        case TVB390_PACKET_SIZE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_PACKET_SIZE, gGeneral.gnPacketSize);
            break;
        case TVB390_FILE_PATH:
            gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_FILE_PATH, gpConfig->gBC[nBoardNum].gszMasterDirectory);
            break;
        case TVB390_FILE_LIST_COUNT:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_FILE_LIST_COUNT, gpConfig->gBC[nBoardNum].nFileListIndexCount);
            break;

        case TVB390_FILE_LIST_INDEX:
           gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_FILE_LIST_INDEX, gpConfig->gBC[nBoardNum].nFileListIndexCur);
            break;
        case TVB390_FILE_LIST_NAME:
			break;
        case TVB390_PLAY_LIST_COUNT:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_PLAY_LIST_COUNT, gpConfig->gBC[nBoardNum].nPlayListIndexCount);
            break;
        case TVB390_PLAY_LIST_INDEX:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_PLAY_LIST_INDEX, gpConfig->gBC[nBoardNum].nPlayListIndexCur);
            break;
        case TVB390_PLAY_LIST_NAME:
            if (gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
                gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_PLAY_LIST_NAME, gpConfig->gBC[nBoardNum].szPlayFileList[gpConfig->gBC[nBoardNum].nPlayListIndexCur]);
            else
                gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_PLAY_LIST_NAME, "-");
            break;

        case TVB390_INPUT_STATUS:
			if(gpConfig->gBC[nBoardNum].gnInputSource == DVB_ASI_IN || gpConfig->gBC[nBoardNum].gnInputSource == SMPTE_310M_IN)
				iValue = gWrapDll.Read_Input_Status(nBoardNum);
			else
				iValue = 0;
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_INPUT_STATUS, iValue);
            break;
        case TVB390_ELAPSED_TIME:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ELAPSED_TIME, gGeneral.gnElapsedTime);
            break;
        case TVB390_RUN_TIME:
            iValue = gpConfig->gBC[nBoardNum].gnPlaybackTime;//gGeneral.gnRunTime;
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_RUN_TIME, iValue);
            break;
        case TVB390_RF_LEVEL_MIN:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_RF_LEVEL_MIN, (int) gpConfig->gBC[nBoardNum].gnMinRFLevel);
            break;
        case TVB390_RF_LEVEL_MAX:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_RF_LEVEL_MAX, (int) gpConfig->gBC[nBoardNum].gnMaxRFLevel);
            break;
		case TVB390_ATTEN_MIN:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ATTEN_MIN, (int) gpConfig->gBC[nBoardNum].gnAtten_Min);
			break;
		case TVB390_ATTEN_MAX:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ATTEN_MAX, (int) gpConfig->gBC[nBoardNum].gnAtten_Max);
			break;

        //----------------------------------------------------
        //--- RUN(9)
        case TVB390_START_PLAYING:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_START_PLAYING, gpConfig->gBC[nBoardNum].bPlayingProgress);
            break;
        case TVB390_SET_PLAY_MODE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_PLAY_MODE, gpConfig->gBC[nBoardNum].gbRepeatMode);
            break;
        case TVB390_SET_INPUT_SOURCE:
			if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == FALSE)
			{
				gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, gpConfig->gBC[nBoardNum].gnModulatorSource);
			}
			else
			{
				if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
				{
					if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
					{
						gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, gpConfig->gBC[nBoardNum].gnModulatorSource);
					}
					else
					{
						if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == SEND_IP_STREAM)
						{
							gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, 4);
						}
						else if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
						{
							gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, 5);
						}
						else if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
						{
							gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, 6);
						}
					}
				}
				else
				{
					gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SET_INPUT_SOURCE, gpConfig->gBC[nBoardNum].gnModulatorSource);
				}
			}
			break;
        case TVB390_ADD_LIST:
            return;
        case TVB390_DELETE_LIST:
            return;
        case TVB390_START_RECORDING:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_START_RECORDING, gpConfig->gBC[nBoardNum].bRecordInProgress);
            break;
        case TVB390_MOVE_LIST_INDEX:
            break;
        case TVB390_APP_EXECUTE:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_APP_EXECUTE, gGeneral.gnApplicationRunFlag);
            break;

        //----------------------------------------------------
        //--- Modulator(36)

        //--- General(2)
        case TVB390_SELECT_SLOT:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SELECT_SLOT, gpConfig->nBoardRealSlot[gBaseUtil.SelectedIndex_Combo(Combo_ADAPTOR)]);
            break;
        case TVB390_MODULATOR_TYPE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_MODULATOR_TYPE, gpConfig->gBC[nBoardNum].gnModulatorMode);
            break;

        //--- DVB-T(5)
        case TVB390_DVB_T_H_BANDWIDTH:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T_H_BANDWIDTH, gpConfig->gBC[nBoardNum].gnBandwidth);
            break;
        case TVB390_DVB_T_H_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T_H_CONSTELLATION, gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_T_H_CODERATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T_H_CODERATE, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_T_H_TXMODE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T_H_TXMODE, gpConfig->gBC[nBoardNum].gnTxmode);
            break;
        case TVB390_DVB_T_H_GUARD_INTERVAL:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T_H_GUARD_INTERVAL, gpConfig->gBC[nBoardNum].gnGuardInterval);
            break;

        //--- VSB(0), QAMA(1), QAMB(2)
        case TVB390_QAM_A_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QAM_A_CONSTELLATION, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
        case TVB390_QAM_B_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QAM_B_CONSTELLATION, gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
        case TVB390_QAM_B_INTERLEAVE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QAM_B_INTERLEAVE, gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;

        //--- TDMB(0), QPSK(3)
        case TVB390_QPSK_CODERATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QPSK_CODERATE, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_QPSK_RRC_FILTER:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QPSK_RRC_FILTER, gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;
        case TVB390_QPSK_SPECTRUM:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_QPSK_SPECTRUM, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            break;

        //--- DVB-H(9)
        case TVB390_DVB_H_BANDWIDTH:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_BANDWIDTH, gpConfig->gBC[nBoardNum].gnBandwidth);
            break;
        case TVB390_DVB_H_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_CONSTELLATION, gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_H_CODERATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_CODERATE, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_H_TXMODE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_TXMODE, gpConfig->gBC[nBoardNum].gnTxmode);
            break;
        case TVB390_DVB_H_GUARD_INTERVAL:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_GUARD_INTERVAL, gpConfig->gBC[nBoardNum].gnGuardInterval);
            break;
        case TVB390_DVB_H_MPE_FEC:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_MPE_FEC, gpConfig->gBC[nBoardNum].gnMPE_FEC);
            break;
        case TVB390_DVB_H_TIME_SLICE:
            gUtility.MySprintf(strMsg, 100,"%d %d", TVB390_DVB_H_TIME_SLICE, gpConfig->gBC[nBoardNum].gnTime_Slice);
            break;
        case TVB390_DVB_H_IN_DEPTH_INTERLEAVE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_IN_DEPTH_INTERLEAVE, gpConfig->gBC[nBoardNum].gnIn_Depth);
            break;
        case TVB390_DVB_H_CELL_ID:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_H_CELL_ID, gpConfig->gBC[nBoardNum].gnCell_Id);
            break;

        //--- DVB-S2(5)
        case TVB390_DVB_S2_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_S2_CONSTELLATION, gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_S2_CODERATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_S2_CODERATE, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_S2_PILOT:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_S2_PILOT, gpConfig->gBC[nBoardNum].gnPilot);
            break;
        case TVB390_DVB_S2_ROLL_OFF:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_S2_ROLL_OFF, gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;
        case TVB390_DVB_S2_SPECTRUM:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_S2_SPECTRUM, gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            break;

        //--- ISDBT(1+1)
        case TVB390_ISDB_T_USE_TMCC:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ISDB_T_USE_TMCC, (gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + 1) % 2);
            break;
        case TVB390_ISDB_T_13SEG_USE_TMCC:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ISDB_T_13SEG_USE_TMCC, (gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + 1) % 2);
            break;
            
        //--- DTMB(7)
        case TVB390_DTMB_CONSTELLATION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_CONSTELLATION, gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DTMB_CODERATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_CODERATE, gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DTMB_INTERLEAVE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_INTERLEAVE, gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;
        case TVB390_DTMB_FRAME_HEADER:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_FRAME_HEADER, gpConfig->gBC[nBoardNum].gnFrameHeader);
            break;
        case TVB390_DTMB_CARRIER_NUMBER:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_CARRIER_NUMBER, gpConfig->gBC[nBoardNum].gnCarrierNumber);
            break;
        case TVB390_DTMB_FRAME_HEADER_PN:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_FRAME_HEADER_PN, gpConfig->gBC[nBoardNum].gnFrameHeaderPN);
            break;
        case TVB390_DTMB_PILOT_INSERTION:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DTMB_PILOT_INSERTION, gpConfig->gBC[nBoardNum].gnPilotInsertion);
            break;

        //----------------------------------------------------
        // RF-IF OUTPUT (9)
        case TVB390_RF:
            gUtility.MySprintf(strMsg, 100, "%d %u", TVB390_RF, gpConfig->gBC[gGeneral.gnActiveBoard].gnRFOutFreq);     //in Hz
            break;
        case TVB390_IF:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_IF, gpConfig->gBC[nBoardNum].gnIFOutFreq);
            break;
        case TVB390_LEVEL:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_LEVEL, (int)gpConfig->gBC[nBoardNum].gnDisRFLevel);
			break;
        case TVB390_CNR:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_CNR, (int) gpConfig->gBC[nBoardNum].gnPRBSscale);
            break;
        case TVB390_CNR_MODE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_CNR_MODE, gpConfig->gBC[nBoardNum].gnPRBSmode);
            break;
        case TVB390_NULL_TP_ON_STOP:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_NULL_TP_ON_STOP, gpConfig->gBC[nBoardNum].gnStopMode);
            break;
        case TVB390_USE_AMP:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_AMP, ((gpConfig->gBC[nBoardNum].gnBypassAMP + 1) % 2));
            break;
        case TVB390_USE_TAT4720:
            break;
        case TVB390_USE_TAT4720_COM:
            break;
		case TVB390_ATTEN:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_ATTEN, (int)gpConfig->gBC[nBoardNum].gdwAttenVal);
			break;

		case TVB390_USE_AGC:
			if(gpConfig->gBC[nBoardNum].gnAGC == 1)
			{
				gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_AGC, 1);
			}
			else
			{
				gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_AGC, 0);
			}
			break;
        //----------------------------------------------------
        // IN-OUTPUT-RATE (4)
        case TVB390_TS:
            gBaseUtil.GetText_Text(ELabPlayRate, text);
			iValue = atoi(text);
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_TS, iValue);
            break;
        case TVB390_TS_MAX:
            break;
        case TVB390_SYMBOL_RATE:
			gBaseUtil.GetText_Text(ELabOutputSymRate, text);
			iValue = atoi(text);
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_SYMBOL_RATE, iValue);
            break;
        case TVB390_SYMBOL_RATE_DEFAULT:
            break;

        //----------------------------------------------------
        // LOOP ADAPTATION (4)
        case TVB390_PCR_PTS_DTS:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_PCR_PTS_DTS, gpConfig->gBC[nBoardNum].gnRestamping);
            break;
        case TVB390_CONTINUITY_COUNTER:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_CONTINUITY_COUNTER, gpConfig->gBC[nBoardNum].gnContinuity);
            break;
        case TVB390_TOT_TDT:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_TOT_TDT, gpConfig->gBC[nBoardNum].gnDateTimeOffset);
            break;
        case TVB390_TOT_TDT_UDT_TIME_SETTING:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_TOT_TDT_UDT_TIME_SETTING, gpConfig->gBC[nBoardNum].gnDateTimeOffset-1);
            break;
		case TVB390_PCR_RESTAMP:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_PCR_RESTAMP, gpConfig->gBC[nBoardNum].gnPCR_Restamping);
            break;

        //----------------------------------------------------
        // LOOP SUB LOOP (4)
        case TVB390_USE_SUB_LOOP:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_SUB_LOOP, gpConfig->gBC[nBoardNum].gnUseSubLoop);
            break;
        case TVB390_START_TIME:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_START_TIME, gpConfig->gBC[nBoardNum].gnStartTimeOffset);
            break;
        case TVB390_END_TIME:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_END_TIME, gpConfig->gBC[nBoardNum].gnEndTimeOffset);
            break;
        case TVB390_USE_FIXED_TS_RATE:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_FIXED_TS_RATE, gpConfig->gBC[nBoardNum].gnCalcPlayRate);
            break;
            
        //----------------------------------------------------
        // LOOP IP STREAMING
        case TVB390_USE_IP_STREAMING:
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_IP_STREAMING, gpConfig->gBC[nBoardNum].gnUseIPStreaming);
            break;
        case TVB390_IP_OUTPUT_ACCESS:
			return;
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_IP_OUTPUT_ACCESS, gpConfig->gBC[nBoardNum].gnIPStreamingAccess);
            break;
        case TVB390_IP_OUTPUT_ADDRESS_URL:
			return;
            gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_IP_OUTPUT_ADDRESS_URL, gpConfig->gBC[nBoardNum].gnIPStreamingAddress);
            break;
        case TVB390_IP_OUTPUT_PORT:
			return;
            iValue = atoi(gpConfig->gBC[nBoardNum].gnIPStreamingPort);
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_IP_OUTPUT_PORT, iValue);
            break;
        case TVB390_IP_INPUT_ACCESS:
			return;
            gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_IP_INPUT_ACCESS, gpConfig->gBC[nBoardNum].gnIPStreamingAccess);
            break;
        case TVB390_IP_INPUT_ADDRESS_URL:
            gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_IP_INPUT_ADDRESS_URL, gpConfig->gBC[nBoardNum].gszIP_RxIP);
			break;
        case TVB390_IP_INPUT_PORT:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_IP_INPUT_PORT, gpConfig->gBC[nBoardNum].gnIP_RxPort);
            break;
		case TVB390_IP_INPUT_MULTICAST:
            gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_IP_INPUT_MULTICAST, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP);
            break;
        case TVB390_IP_INPUT_LOCAL:
            gUtility.MySprintf(strMsg, 100, "%d %s", TVB390_IP_INPUT_LOCAL, gpConfig->gBC[nBoardNum].gszIP_RxLocalIP);
            break;
        case TVB390_USE_MULTICAST_IP:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_USE_MULTICAST_IP, gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP);
            break;
		case TVB390_DVB_T2_BANDWIDTH:
			gUtility.MySprintf(strMsg, 100, "%d %d", TVB390_DVB_T2_BANDWIDTH, gpConfig->gBC[nBoardNum].gnBandwidth);
			break;

    }

    gUtilInd.SNMP_DataSend(strMsg);
}
#else

//extern struct _SNMP_MSG_BUF msgbuf_remote;
extern struct _SNMP_SIMPLE_MSG_BUF msgbuf_simple_remote;

void PlayForm::SNMP_Send_Status(int iMsgType)
{
    char    strMsg[100];
    char    text[256];
    long    nBoardNum = gGeneral.gnActiveBoard;
	int		iValue, i;

	msgbuf_simple_remote.mtype = iMsgType;

    switch(iMsgType)
    {
        //----------------------------------------------------
        //--- BOARD (4)
        case TVB390_BOARD_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->gBC[nBoardNum].gnBoardId);
            break;
        case TVB390_BOARD_AUTHORIZATION:
            iValue = 0;
            for (i = MAX_MODULATORMODE-1; i >= 0; i--)
			{
				iValue = (iValue << 1) + gpConfig->gBC[nBoardNum].gbEnabledType[i];
			}
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, iValue);
            break;
        case TVB390_BOARD_LOCATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->nBoardSlotNum[nBoardNum]);
        	break;
        case TVB390_BOARD_INSTALLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnBoardNum);
  			//msgbuf_remote.param.board_location = gGeneral.gnBoardNum;
            break;
        case TVB390_BOARD_LN:
			//2010/7/1
			char board_LN[256];
			gUtility.MyStrCpy(board_LN,256, (char *) "");
			gUtility.MyStrCpy(board_LN,256, (char *)gWrapDll.Get_AdaptorInfo(nBoardNum,1));
			if(strcmp(board_LN, "") == 0)
			{
				gUtility.MyStrCpy(board_LN,256, (char *)"11111111111111111111111111111111");
			}
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, board_LN);
			break;

        case TVB390_BOARD_SN:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gWrapDll.Get_AdaptorInfo(nBoardNum,0));
            break;
        case TVB390_BOARD_ID_INFO:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gGeneral.str_board_id_info);
            break;
        //----------------------------------------------------
        //--- STATUS(15)
        case TVB390_BITRATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnBitrate);
            break;
        case TVB390_PACKET_SIZE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnPacketSize);
            break;
        case TVB390_DISK_FREE_SIZE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnDiskSize);
            break;
        case TVB390_FILE_PATH:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszMasterDirectory);
            break;
        case TVB390_FILE_LIST_COUNT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->gBC[nBoardNum].nFileListIndexCount);
            break;

        case TVB390_FILE_LIST_INDEX:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->gBC[nBoardNum].nFileListIndexCur);
	        break;
        case TVB390_FILE_LIST_NAME:
			{
				int iCount = gpConfig->gBC[nBoardNum].nFileListIndexCount;
				if (gpConfig->gBC[nBoardNum].nFileListIndexCur >= 0 && gpConfig->gBC[nBoardNum].nFileListIndexCur < iCount)
				{
					gUtility.MyStrCpy(text, 256, gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
				} else
					gUtility.MyStrCpy(text, 256,  (char *)"");
				sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, text);
			}
			break;
        case TVB390_PLAY_LIST_COUNT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->gBC[nBoardNum].nPlayListIndexCount);
            break;
        case TVB390_PLAY_LIST_INDEX:
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, gpConfig->gBC[nBoardNum].nPlayListIndexCur);
            break;
        case TVB390_PLAY_LIST_NAME:
            if (gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
			{
				char str[256];
				GetPlayListFileIndex(nBoardNum, str);
 				sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, str);
			}
            else
				sprintf(msgbuf_simple_remote.strParam, "%d -", iMsgType);
            break;

        case TVB390_INPUT_STATUS:
            iValue = gWrapDll.Read_Input_Status(nBoardNum);
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, iValue);
            break;
        case TVB390_ELAPSED_TIME:
  			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gGeneral.gnSnmpElapsedTime);
            break;
        case TVB390_RUN_TIME:
			if((gpConfig->gBC[nBoardNum].bPlayingProgress == true || 
				gpConfig->gBC[nBoardNum].bRecordInProgress == true) && gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
				return;
			iValue = Get_Run_Time_Of_CurrentSet(nBoardNum);
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, iValue);
            break;
        case TVB390_RF_LEVEL_MIN:
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum))
 				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gdRfLevelRange_min * 10));
			else
 				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnMinRFLevel * 10));
			break;
        case TVB390_RF_LEVEL_MAX:
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum))
 				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gdRfLevelRange_max * 10));
			else
	 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnMaxRFLevel * 10));
            break;
		case TVB390_ATTEN_MIN:
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnAtten_Min * 10));
			break;
		case TVB390_ATTEN_MAX:
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnAtten_Max * 10));
			break;
		case TVB390_LOOP_COUNT:	// read only
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnRepeatCount);
			break;

        case TVB390_TS_IN_RATE:	
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gGeneral.gnInputRate);
            break;

		case TVB390_TS_RECORD_SIZE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].dwLastRecordedBytes/1000));
			break;
        //----------------------------------------------------
        //--- RUN(9)
        case TVB390_START_PLAYING:
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].bPlayingProgress);
            break;
        case TVB390_SET_PLAY_MODE:
 			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gbRepeatMode);
            break;
        case TVB390_SET_INPUT_SOURCE:
			if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == FALSE)
			{
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnModulatorSource);
			}
			else
			{
				if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == FILE_SRC)
				{
					if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
					{
						sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnModulatorSource);
					}
					else
					{
						if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == SEND_IP_STREAM)
						{
							sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 4);
						}
						else if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
						{
							sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 5);
						}
						else if(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
						{
							sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 6);
						}
					}
				}
				else
				{
					sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnModulatorSource);
				}
			}
			break;
        case TVB390_ADD_LIST:
            return;
        case TVB390_DELETE_LIST:
            return;
        case TVB390_START_RECORDING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].bRecordInProgress);
            break;
        case TVB390_SELECT_LIST:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].fCurFocus);
            break;
        case TVB390_MOVE_LIST_INDEX:
            break;
        case TVB390_APP_EXECUTE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gGeneral.gnApplicationRunFlag);
            break;

        //----------------------------------------------------
        //--- Modulator(36)

        //--- General(2)
        case TVB390_SELECT_SLOT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)(gpConfig->gnCurSlotIndex + 1));
            break;
        case TVB390_MODULATOR_TYPE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnModulatorMode);
            break;

        //--- DVB-T(5)
        case TVB390_DVB_T_H_BANDWIDTH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnBandwidth);
            break;
        case TVB390_DVB_T_H_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_T_H_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_T_H_TXMODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnTxmode);
            break;
        case TVB390_DVB_T_H_GUARD_INTERVAL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnGuardInterval);
            break;

        //--- VSB(0), QAMA(1), QAMB(2)
        case TVB390_QAM_A_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
        case TVB390_QAM_B_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnQAMMode);
            break;
        case TVB390_QAM_B_INTERLEAVE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;

        //--- TDMB(0), QPSK(3)
        case TVB390_QPSK_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_QPSK_RRC_FILTER:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;
        case TVB390_QPSK_SPECTRUM:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            break;

        //--- DVB-H(9)
        case TVB390_DVB_H_BANDWIDTH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnBandwidth);
            break;
        case TVB390_DVB_H_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_H_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_H_TXMODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnTxmode);
            break;
        case TVB390_DVB_H_GUARD_INTERVAL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnGuardInterval);
            break;
        case TVB390_DVB_H_MPE_FEC:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnMPE_FEC);
            break;
        case TVB390_DVB_H_TIME_SLICE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnTime_Slice);
            break;
        case TVB390_DVB_H_IN_DEPTH_INTERLEAVE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnIn_Depth);
            break;
        case TVB390_DVB_H_CELL_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCell_Id);
            break;

        //--- DVB-S2(5)
        case TVB390_DVB_S2_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DVB_S2_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DVB_S2_PILOT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnPilot);
            break;
        case TVB390_DVB_S2_ROLL_OFF:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnRollOffFactor);
            break;
        case TVB390_DVB_S2_SPECTRUM:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnSpectrumInverse);
            break;

        //--- ISDBT(1+1)
        case TVB390_ISDB_T_USE_TMCC:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)((gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer+1) % 2));
            break;
        case TVB390_ISDB_T_13SEG_USE_TMCC:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)((gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer+1) % 2));
            break;
            
        //--- DTMB(7)
        case TVB390_DTMB_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnConstellation);
            break;
        case TVB390_DTMB_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCodeRate);
            break;
        case TVB390_DTMB_INTERLEAVE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnQAMInterleave);
            break;
        case TVB390_DTMB_FRAME_HEADER:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnFrameHeader);
            break;
        case TVB390_DTMB_CARRIER_NUMBER:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCarrierNumber);
            break;
        case TVB390_DTMB_FRAME_HEADER_PN:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnFrameHeaderPN);
            break;
        case TVB390_DTMB_PILOT_INSERTION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnPilotInsertion);
            break;

        //----------------------------------------------------
        // RF-IF OUTPUT (9)
        case TVB390_RF:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)(gpConfig->gBC[nBoardNum].gnRFOutFreq/1000));
            break;
        case TVB390_IF:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnIFOutFreq);
            break;
        case TVB390_LEVEL:
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnDisRFLevel * 10));
			else
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gdRfLevelValue * 10));
printf("RF LEVEL : %s\n", msgbuf_simple_remote.strParam);
			break;
        case TVB390_CNR:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnPRBSscale * 10));
            break;
        case TVB390_CNR_MODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnPRBSmode);
            break;
        case TVB390_NULL_TP_ON_STOP:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnStopMode);
            break;
        case TVB390_USE_AMP:
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)((gpConfig->gBC[nBoardNum].gnBypassAMP + 1) % 2));
			else
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)(gpConfig->gBC[nBoardNum].gnBypassAMP));
            break;
        case TVB390_USE_TAT4720:
            break;
        case TVB390_USE_TAT4720_COM:
            break;
		//kslee091218
		case TVB390_ATTEN:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gdwAttenVal * 10));
			break;

		case TVB390_USE_AGC:
			if(gpConfig->gBC[nBoardNum].gnAGC == 1)
			{
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 1);
			}
			else
			{
				sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 0);
			}
			break;

		case TVB390_BERT_TYPE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnBertPacketType);
			break;
		case TVB390_BERT_PID_VALUE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnBert_Pid);
			break;
			
        //----------------------------------------------------
        // IN-OUTPUT-RATE (4)
        case TVB390_TS:
			if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
				iValue = 2433331;
			else
				iValue = gpConfig->gBC[nBoardNum].gdwPlayRate;
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, iValue);
            break;
        case TVB390_TS_MAX:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnOutputClockSource);
            break;
        case TVB390_SYMBOL_RATE:
			iValue = gpConfig->gBC[nBoardNum].gnSymbolRate;
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, iValue);
            break;
        case TVB390_SYMBOL_RATE_DEFAULT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnDefaultSymbol);
            break;

        //----------------------------------------------------
        // LOOP ADAPTATION (4)
        case TVB390_PCR_PTS_DTS:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnRestamping);
            break;
        case TVB390_CONTINUITY_COUNTER:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnContinuity);
            break;
        case TVB390_TOT_TDT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnDateTimeOffset);
            break;
        case TVB390_TOT_TDT_UDT_TIME_SETTING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) (gpConfig->gBC[nBoardNum].gnDateTimeOffset-1));
            break;

        case TVB390_RESTAMP_DATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date);
            break;

        case TVB390_RESTAMP_TIME:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;

        //----------------------------------------------------
        // LOOP SUB LOOP (4)
        case TVB390_USE_SUB_LOOP:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnUseSubLoop);
            break;
        case TVB390_START_TIME:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnStartTimeOffset);
            break;
        case TVB390_END_TIME:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnEndTimeOffset);
            break;
        case TVB390_USE_FIXED_TS_RATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnCalcPlayRate);
            break;
            
        //----------------------------------------------------
        // LOOP IP STREAMING
        case TVB390_USE_IP_STREAMING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int) gpConfig->gBC[nBoardNum].gnUseIPStreaming);
            break;
        case TVB390_IP_RXIP:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszIP_RxIP);
			break;
		case TVB390_IP_RXMULTICASTIP:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP);
            break;
        case TVB390_IP_USEMULTICAST:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP);
            break;
		//2010/6/22 IP UDP/RTP
		case TVB390_IP_INPUTRATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_InputRate);
			break;
        case TVB390_IP_INPUT_ADDRESS_URL:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnIPStreamingAddress);
            break;
        case TVB390_IP_INPUT_PORT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_RxPort);
            break;

		//----------------------------------------------------
		// ERROR INJECTION
		case TVB390_ERROR_USE_PACKET:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrLost);
			break;

		case TVB390_ERROR_NUM_PACKET_PACKET:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrLostPacket);
			break;

		case TVB390_ERROR_USE_BIT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBits);
			break;

		case TVB390_ERROR_NUM_PACKET_BIT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBitsPacket);
			break;

		case TVB390_ERROR_NUM_PER_PACKET_BIT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBitsCount);
			break;

		case TVB390_ERROR_USE_BYTE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBytes);
			break;

		case TVB390_ERROR_NUM_PACKET_BYTE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBytesPacket);
			break;

		case TVB390_ERROR_NUM_PER_PACKET_BYTE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnErrBytesCount);
			break;
	
		case TVB390_RESERVED_TICK_COUNT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, 1);
			break;
		case TVB390_CHECK_LICENSE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", TVB390_BOARD_AUTHORIZATION, gGeneral.gnLNOpt);
			break;
		case TVB390_UPDATE_LICENSE:
            iValue = 0;
            for (i = MAX_MODULATORMODE-1; i >= 0; i--)
			{
				iValue = (iValue << 1) + gpConfig->gBC[nBoardNum].gbEnabledType[i];
			}
			sprintf(msgbuf_simple_remote.strParam, "%d %d", TVB390_BOARD_AUTHORIZATION, iValue);
			break;

		case TVB390_FIRMWARE_VERSION:		// Send Firmware version.
#ifdef STANDALONE
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, VER_MAJOR*10000 + VER_MINOR * 100 + VER_BILDER);
#else
#endif
			break;
		case TVB390_FIRMWARE_WRITE:			// Send Firmware writing status
#ifdef STANDALONE
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gMenuCnt);
#else
#endif
            break;
		case TVB390_IP_ADDRESS:
            break;
		case TVB390_SUBNET_MASK:
            break;
		case TVB390_GATEWAY:
            break;
		case TVB390_DHCP_ENABLE:
            break;
//------------------
		case TVB390_BOARD_REV:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnBoardRev);
			break;
		case TVB390_CMMB_MDIF:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_Mdif);
			break;
		case TVB390_CMMB_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_const);
			break;
		case TVB390_CMMB_RSCODING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_rscoding);
			break;
		case TVB390_CMMB_BYTECROSSING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_bytecrossing);
			break;
		case TVB390_CMMB_LDPC:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_ldpc);
			break;
		case TVB390_CMMB_SCRAMBLE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_scramble);
			break;
		case TVB390_CMMB_TIMESLICE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCmmb_timeslice);
			break;
		case TVB390_CMMB_MDIF_ITEM:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszCmmb_Mdif);
			break;
		//------------------

		//----------------------------------------------------
		case TVB390_PCR_RESTAMP:
            sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnPCR_Restamping);
            break;
		//kslee 2010/4/20
		case TVB390_DVB_T2_BANDWIDTH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_BW);
			break;

		case TVB390_DVB_T2_PARAMETER:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[0]);
			break;
		case TVB390_DVB_T2_PARAMETER2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[1]);
			break;
		case TVB390_DVB_T2_PARAMETER3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[2]);
			break;
		case TVB390_DVB_T2_PARAMETER4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[3]);
			break;
		case TVB390_DVB_T2_PARAMETER5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[4]);
			break;
		case TVB390_DVB_T2_PARAMETER6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[5]);
			break;
		case TVB390_DVB_T2_PARAMETER7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[6]);
			break;
		case TVB390_DVB_T2_PARAMETER8:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[7]);
			break;
		case TVB390_DVB_T2_PARAMETER9:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[8]);
			break;
		case TVB390_DVB_T2_PARAMETER10:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[9]);
			break;
		case TVB390_DVB_T2_PARAMETER11:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[10]);
			break;
		case TVB390_DVB_T2_PARAMETER12:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[11]);
			break;
		case TVB390_DVB_T2_PARAMETER13:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[12]);
			break;
		case TVB390_DVB_T2_PARAMETER14:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[13]);
			break;
		case TVB390_DVB_T2_PARAMETER15:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[14]);
			break;
		case TVB390_DVB_T2_PARAMETER16:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[15]);
			break;
		case TVB390_DVB_T2_PARAMETER17:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[16]);
			break;
		case TVB390_ATSC_MH_PARAMETER:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[0]);
			break;
		case TVB390_ATSC_MH_PARAMETER2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[1]);
			break;
		case TVB390_ATSC_MH_PARAMETER3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[2]);
			break;
		case TVB390_ATSC_MH_PARAMETER4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[3]);
			break;
		case TVB390_ATSC_MH_PARAMETER5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[4]);
			break;
		case TVB390_ATSC_MH_PARAMETER6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[5]);
			break;
		case TVB390_ATSC_MH_PARAMETER7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[6]);
			break;
		case TVB390_ATSC_MH_PARAMETER8:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[7]);
			break;
		case TVB390_ATSC_MH_PARAMETER9:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[8]);
			break;

		case TVB390_ISDB_T_PARAMETER:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[0]);
			break;
		case TVB390_ISDB_T_PARAMETER2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[1]);
			break;
		case TVB390_ISDB_T_PARAMETER3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[2]);
			break;
		case TVB390_ISDB_T_PARAMETER4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[3]);
			break;
		case TVB390_ISDB_T_PARAMETER5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[4]);
			break;
		case TVB390_ISDB_T_PARAMETER6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[5]);
			break;
		case TVB390_ISDB_T_PARAMETER7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[6]);
			break;
		case TVB390_ISDB_T_PARAMETER8:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[7]);
			break;
		case TVB390_ISDB_T_PARAMETER9:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[8]);
			break;
		case 	TVB390_DVB_T2_BWT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_BWT);
			break;
		case 	TVB390_DVB_T2_FFT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_FFT);
			break;
		case 	TVB390_DVB_T2_GUARD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_GUARD);
			break;
		case 	TVB390_DVB_T2_L1_MOD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD);
			break;
		case 	TVB390_DVB_T2_PILOT_PATTERN:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN);
			break;
		case 	TVB390_DVB_T2_NETWORK_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID);
			break;
		case 	TVB390_DVB_T2_SYSTEM_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID);
			break;
		case 	TVB390_DVB_T2_CELL_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID);
			break;
		case 	TVB390_DVB_T2_PID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PID);
			break;
		case 	TVB390_DVB_T2_NUM_T2_FRAME:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME);
			break;
		case 	TVB390_DVB_T2_NUM_DATA_SYMBOL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL);
			break;
//2011/3/25 DVB-T2 MULTI-PLP =================================================================================================================
		case 	TVB390_DVB_T2_PLP_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[0]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[0]);
			break;
		case 	TVB390_DVB_T2_PLP_COD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[0]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[0]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[0]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[0]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[0]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[0]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[0]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[1]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[1]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[1]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[1]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[1]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[1]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[1]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[1]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[1]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[1]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[2]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[2]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[2]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[2]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[2]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[2]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[2]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[2]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[2]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[2]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[3]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[3]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[3]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[3]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[3]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[3]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[3]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[3]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[3]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[3]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[4]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[4]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[4]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[4]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[4]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[4]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[4]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[4]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[4]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[4]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[5]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[5]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[5]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[5]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[5]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[5]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[5]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[5]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[5]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[5]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[6]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[6]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[6]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[6]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[6]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[6]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[6]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[6]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[6]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[6]);
			break;

		case 	TVB390_DVB_T2_PLP_ID_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[7]);
			break;
		case 	TVB390_DVB_T2_PLP_MOD_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[7]);
			break;
		case 	TVB390_DVB_T2_PLP_COD_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[7]);
			break;
		case 	TVB390_DVB_T2_PLP_FEC_TYPE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[7]);
			break;
		case 	TVB390_DVB_T2_PLP_NUM_BLOCK_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[7]);
			break;
		case 	TVB390_DVB_T2_PLP_HEM_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[7]);
			break;
		case 	TVB390_DVB_T2_PLP_ROT_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[7]);
			break;
		case 	TVB390_DVB_T2_PLP_TS_PATH_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[7]);
			break;
		case 	TVB390_DVB_T2_PLP_FILE_BITRATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[7]);
			break;
		case 	TVB390_DVB_T2_PLP_PLP_BITRATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[7]);
			break;
		case	TVB390_DVB_T2_ISDB_S_FILE_NAME:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].szDvbt2_FileName);
			break;
		case 	TVB390_DVB_T2_ISDB_S_FILE_COUNT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDvbt2_FileListCount);
			break;
		case TVB390_DVB_T2_ISDB_S_DIRECTORY:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].szDvbt2_Directory);
			break;
		case    TVB390_DVB_T2_ISDB_S_FILE_BITRATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDvbt2_bitrate);
			break;

//=============================================================================================================================================
		//2010/11/19 TVB593 EMERGENCY BROADCASTING
		case	TVB390_EMERGENCY_BROADCASTING:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting);
			break;
		//2011/1/13 ISDB-T TMCC Setting ===============================================================================================================================================================
		case	TVB390_TMCC_BROADCAST:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.broadcast);
			break;
		case	TVB390_TMCC_MODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.mode);
			break;
		case	TVB390_TMCC_GUARD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.guard);
			break;
		case	TVB390_TMCC_PARTIAL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.partial);
			break;
		//Layer A
		case	TVB390_TMCC_SEG_A:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg);
			break;
		case	TVB390_TMCC_MOD_A:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod);
			break;
		case	TVB390_TMCC_COD_A:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod);
			break;
		case	TVB390_TMCC_TIME_A:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerA.time);
			break;
		case	TVB390_TMCC_DATA_A:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerA.datarate);
			break;
		//Layer B
		case	TVB390_TMCC_SEG_B:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg);
			break;
		case	TVB390_TMCC_MOD_B:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod);
			break;
		case	TVB390_TMCC_COD_B:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod);
			break;
		case	TVB390_TMCC_TIME_B:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerB.time);
			break;
		case	TVB390_TMCC_DATA_B:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerB.datarate);
			break;
		//Layer C
		case	TVB390_TMCC_SEG_C:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg);
			break;
		case	TVB390_TMCC_MOD_C:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod);
			break;
		case	TVB390_TMCC_COD_C:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod);
			break;
		case	TVB390_TMCC_TIME_C:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerC.time);
			break;
		case	TVB390_TMCC_DATA_C:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].tmccInfo.layerC.datarate);
			break;
		//TS INFO
		case	TVB390_TMCC_PID_COUNT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnPIDCount);
			break;
		case	TVB390_TMCC_PID_INFO:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gGeneral.gszPIDInfo);
			break;
		//=============================================================================================================================================================================================
		//2011/4/18 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		case	TVB390_ISDB_S_CONSTELLATION:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation);
			break;
		case	TVB390_ISDB_S_CODERATE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCodeRate);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_0: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[0]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[0]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_1: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[1]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[1]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_2: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[2]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[2]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_3: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[3]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[3]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_4: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[4]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[4]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_5: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[5]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[5]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_6: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[6]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[6]);
			break;

		case	TVB390_ISDB_S_MULTI_TS_ID_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_ID_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_PATH_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gszTS_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_FILERATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_DATARATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTS_Selected_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CONSTELLATION_7: 
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnConstellation_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_CODERATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCoderate_M[7]);
			break;
		case	TVB390_ISDB_S_MULTI_TS_SLOT_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSlotCount_M[7]);
			break;
		case	TVB390_ISDB_S_TMCC_USE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer);
			break;
		case	TVB390_ISDB_S_IS_COMBINED_TS:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnCombinedTS);
			break;
		case	TVB390_ISDB_S_TS_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnIsdbs_ts_id);
			break;
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		//2011/9/9 DVB-C2 ==============================================================================================================================================================
		case	TVB390_DVBC2_BANDWIDTH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnBandwidth);
			break;
		case	TVB390_DVBC2_GUARDINTERVAL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Guard);
			break;
		case	TVB390_DVBC2_STARTFREQ:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq);
			break;
		case	TVB390_DVBC2_L1TIMODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_L1);
			break;
		case	TVB390_DVBC2_NETID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Network);
			break;
		case	TVB390_DVBC2_SYSID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_System);
			break;
		case	TVB390_DVBC2_RESERVEDTONE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone);
			break;
		case	TVB390_DVBC2_NUMNOTCH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth);
			break;
		case	TVB390_DVBC2_NOTCHSTART:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart);
			break;
		case	TVB390_DVBC2_NOTCHWIDTH:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth);
			break;
		case	TVB390_DVBC2_DSLICE_TYPE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type);
			break;
		case	TVB390_DVBC2_DSLICE_FEC_H:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader);
			break;
		case 	TVB390_DVBC2_PLP_ID_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[0]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[0]);
			break;
		case 	TVB390_DVBC2_PLP_COD_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[0]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[0]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[0]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[0]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[0]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[0]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_0:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[0]);
			break;
		case 	TVB390_DVBC2_PLP_ID_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[1]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[1]);
			break;
		case 	TVB390_DVBC2_PLP_COD_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[1]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[1]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[1]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[1]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[1]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[1]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[1]);
			break;
		case 	TVB390_DVBC2_PLP_ID_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[2]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[2]);
			break;
		case 	TVB390_DVBC2_PLP_COD_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[2]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[2]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[2]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[2]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[2]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[2]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[2]);
			break;
		case 	TVB390_DVBC2_PLP_ID_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[3]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[3]);
			break;
		case 	TVB390_DVBC2_PLP_COD_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[3]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[3]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[3]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[3]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[3]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[3]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[3]);
			break;
		case 	TVB390_DVBC2_PLP_ID_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[4]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[4]);
			break;
		case 	TVB390_DVBC2_PLP_COD_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[4]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[4]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[4]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[4]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[4]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[4]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[4]);
			break;
		case 	TVB390_DVBC2_PLP_ID_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[5]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[5]);
			break;
		case 	TVB390_DVBC2_PLP_COD_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[5]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[5]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[5]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[5]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[5]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[5]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[5]);
			break;
		case 	TVB390_DVBC2_PLP_ID_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[6]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[6]);
			break;
		case 	TVB390_DVBC2_PLP_COD_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[6]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[6]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[6]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[6]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[6]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[6]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[6]);
			break;
		case 	TVB390_DVBC2_PLP_ID_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[7]);
			break;
		case 	TVB390_DVBC2_PLP_MOD_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[7]);
			break;
		case 	TVB390_DVBC2_PLP_COD_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[7]);
			break;
		case 	TVB390_DVBC2_PLP_FEC_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[7]);
			break;
		case 	TVB390_DVBC2_PLP_BLK_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[7]);
			break;
		case 	TVB390_DVBC2_PLP_HEM_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[7]);
			break;
		case 	TVB390_DVBC2_PLP_TSPATH_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[7]);
			break;
		case 	TVB390_DVBC2_PLP_FILEBITRATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[7]);
			break;
		case 	TVB390_DVBC2_PLP_PLPBITRATE_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[7]);
			break;
		//==============================================================================================================================================================================
		//2011/9/26 DVB-T2 IP ======================================================================================================================================
		case TVB390_DVBT2_IP_PLP_MOD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD);
			break;
		case TVB390_DVBT2_IP_PLP_COD:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD);
			break;
		case TVB390_DVBT2_IP_PLP_FEC:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE);
			break;
		case TVB390_DVBT2_IP_PLP_ID:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID);
			break;
		case TVB390_DVBT2_IP_PLP_HEM:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM);
			break;
		case TVB390_DVBT2_IP_PLP_ROT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation);
			break;
		//==========================================================================================================================================================
		//2011/10/26 DAC OFFSET ====================================================================================================================================
		case TVB390_DAC_RF_RANGE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSnmp_RF_Index);
			break;
		case TVB390_DAC_I_OFFSET:
			iValue = gUtilInd.getFreqIndex();
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gDAC_I_Offset[iValue]);
			break;
		case TVB390_DAC_Q_OFFSET:
			iValue = gUtilInd.getFreqIndex();
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gDAC_Q_Offset[iValue]);
			break;
		case TVB390_DAC_I_OFFSET_READ:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSnmp_DAC_I_Offest);
			break;
		case TVB390_DAC_Q_OFFSET_READ:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSnmp_DAC_Q_Offset);
			break;
		case TVB390_DAC_MODULATION_MODE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gDAC_Mod_Mode);
			break;
		//==========================================================================================================================================================
		//2011/11/08 I/Q PLAY ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		case TVB390_IQPLAY_SPECTRUM:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnSpectrumInverse);
			break;
		case TVB390_IQPLAY_USE_CAPTURE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIQ_capture_mode);
			break;
		case TVB390_IQPLAY_MEM_USE:	
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIQ_mem_use);
			break;
		case TVB390_IQPLAY_MEMORY_SIZE:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIQ_mem_size);
			break;
		case TVB390_IQPLAY_CAPTURE_SUPPORT:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIQ_capture_support);
			break;
		case TVB390_IQPLAY_CAPTURE_FILENAME:
			sprintf(msgbuf_simple_remote.strParam, "%d %s", iMsgType, (char *)gpConfig->gBC[nBoardNum].gszIQ_capture_filePath);
			break;
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		case TVB390_USB_MOUNT_STATUS:
#ifdef STANDALONE
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnUsb_Status);
#endif
			break;
		case TVB390_SDCARD_MOUNT_STATUS:
#ifdef STANDALONE
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gGeneral.gnSdcard_Status);
#endif
			break;
		case TVB390_DVB_T2_PLP_IL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[0]);
			break;
		case TVB390_DVB_T2_PLP_IL_1:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[1]);
			break;
		case TVB390_DVB_T2_PLP_IL_2:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[2]);
			break;
		case TVB390_DVB_T2_PLP_IL_3:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[3]);
			break;
		case TVB390_DVB_T2_PLP_IL_4:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[4]);
			break;
		case TVB390_DVB_T2_PLP_IL_5:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[5]);
			break;
		case TVB390_DVB_T2_PLP_IL_6:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[6]);
			break;
		case TVB390_DVB_T2_PLP_IL_7:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[7]);
			break;
		case TVB390_DVB_T2_IP_PLP_IL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Time_Interleave);
			break;
		case TVB59x_TS_OUTPUT_SEL:
			sprintf(msgbuf_simple_remote.strParam, "%d %d", iMsgType, (int)gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
			break;
		default:
			return;

    }
    gUtilInd.SNMP_DataSend(strMsg);
}
#endif

//---------------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::Display_Init()
{
    int     nBoardNum = gGeneral.gnActiveBoard;

	Set_ItemList_Combo_Adaptor();

	//-----------------------------------------------------------------
//    UpdateModulatorConfigUI(nBoardNum);
    //------------------------------------------------------------------
    //--- Initial_Check : it is from open_system
    //------------------------------------------------------------------
    Initial_Check(gGeneral.gnActiveBoard);

    //-----------------------------------

    //-----------------------------------
    // Caption = TVB590 PCI-5: DVB-T
    Display_Main_Title(gGeneral.gnActiveBoard);
    Set_Frequency_According_To_Unit(gpConfig->gBC[nBoardNum].gnRFOutFreq);
}
#else
void PlayForm::Display_Init()
{
    char    text[512];
    int     i,j;
    int     nBoardNum = gGeneral.gnActiveBoard;
	//2010/7/13
	int		nSlotNum;
	char	szDesc[256];

	//-----------------------------------------------------------------
    //--- Add Adaptor List : nBoardSlotNum[] : on Adaptor Change
    //-----------------------------------------------------------------

	Set_ItemList_HW();

	//-----------------------------------------------------------------
    UpdateModulatorConfigUI(nBoardNum);
    
    //------------------------------------------------------------------
    //--- Initial_Check : it is from open_system
    //------------------------------------------------------------------
    // - gnCmdPlayrate, gnCmdFileName, License Manager
    Initial_Check(gGeneral.gnActiveBoard);

}

#endif

#ifdef WIN32
//------------------------------------------------------------------------------
void PlayForm::SNMP_All_DataSend()
{
    long    nBoardNum = gGeneral.gnActiveBoard;

    //1~4------------------------------------------------------------
    //TVB590-Board (4)
    SNMP_Send_Status(TVB390_BOARD_ID);
    SNMP_Send_Status(TVB390_BOARD_AUTHORIZATION);
    SNMP_Send_Status(TVB390_BOARD_LOCATION);
    SNMP_Send_Status(TVB390_BOARD_INSTALLATION);

    //1~15------------------------------------------------------------
    //TVB590-Status (15)
    SNMP_Send_Status(TVB390_BITRATE);
    SNMP_Send_Status(TVB390_PACKET_SIZE);
    SNMP_Send_Status(TVB390_DISK_FREE_SIZE);
    SNMP_Send_Status(TVB390_FILE_PATH);
    SNMP_Send_Status(TVB390_FILE_LIST_COUNT);

    SNMP_Send_Status(TVB390_FILE_LIST_INDEX);
    SNMP_Send_Status(TVB390_FILE_LIST_NAME);
    SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
    SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
    SNMP_Send_Status(TVB390_PLAY_LIST_NAME);

    SNMP_Send_Status(TVB390_INPUT_STATUS);
    SNMP_Send_Status(TVB390_ELAPSED_TIME);
    SNMP_Send_Status(TVB390_RUN_TIME);
    SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
    SNMP_Send_Status(TVB390_RF_LEVEL_MAX);

    //------------------------------------------------------------
    //TVB590-Run
    SNMP_Send_Status(TVB390_START_PLAYING);
    SNMP_Send_Status(TVB390_SET_PLAY_MODE);
    SNMP_Send_Status(TVB390_SET_INPUT_SOURCE);
    SNMP_Send_Status(TVB390_ADD_LIST);
    SNMP_Send_Status(TVB390_DELETE_LIST);
    SNMP_Send_Status(TVB390_START_RECORDING);
    SNMP_Send_Status(TVB390_SELECT_LIST);
    SNMP_Send_Status(TVB390_MOVE_LIST_INDEX);
    SNMP_Send_Status(TVB390_APP_EXECUTE);
    
    //------------------------------------------------------------
    //TVB590-Modulator
    SNMP_Send_Status(TVB390_SELECT_SLOT);
    SNMP_Send_Status(TVB390_MODULATOR_TYPE);

    SNMP_Send_Status(TVB390_DVB_T_H_BANDWIDTH);
    SNMP_Send_Status(TVB390_DVB_T_H_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_T_H_CODERATE);
    SNMP_Send_Status(TVB390_DVB_T_H_TXMODE);
    SNMP_Send_Status(TVB390_DVB_T_H_GUARD_INTERVAL);

    SNMP_Send_Status(TVB390_QAM_A_CONSTELLATION);
    SNMP_Send_Status(TVB390_QAM_B_CONSTELLATION);
    SNMP_Send_Status(TVB390_QAM_B_INTERLEAVE);

    SNMP_Send_Status(TVB390_QPSK_CODERATE);
    SNMP_Send_Status(TVB390_QPSK_RRC_FILTER);
    SNMP_Send_Status(TVB390_QPSK_SPECTRUM);

    SNMP_Send_Status(TVB390_DVB_H_BANDWIDTH);
    SNMP_Send_Status(TVB390_DVB_H_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_H_CODERATE);
    SNMP_Send_Status(TVB390_DVB_H_TXMODE);
    SNMP_Send_Status(TVB390_DVB_H_GUARD_INTERVAL);
    SNMP_Send_Status(TVB390_DVB_H_MPE_FEC);
    SNMP_Send_Status(TVB390_DVB_H_TIME_SLICE);
    SNMP_Send_Status(TVB390_DVB_H_IN_DEPTH_INTERLEAVE);
    SNMP_Send_Status(TVB390_DVB_H_CELL_ID);

    SNMP_Send_Status(TVB390_DVB_S2_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_S2_CODERATE);
    SNMP_Send_Status(TVB390_DVB_S2_PILOT);
    SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
    SNMP_Send_Status(TVB390_DVB_S2_SPECTRUM);

    SNMP_Send_Status(TVB390_ISDB_T_USE_TMCC);

    SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);

    SNMP_Send_Status(TVB390_DTMB_CONSTELLATION);
    SNMP_Send_Status(TVB390_DTMB_CODERATE);
    SNMP_Send_Status(TVB390_DTMB_INTERLEAVE);
    SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER);
    SNMP_Send_Status(TVB390_DTMB_CARRIER_NUMBER);
    SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER_PN);
    SNMP_Send_Status(TVB390_DTMB_PILOT_INSERTION);

    //------------------------------------------------------------
    //TVB590-RF-IF OUTPUT
    SNMP_Send_Status(TVB390_RF);
    SNMP_Send_Status(TVB390_IF);
    SNMP_Send_Status(TVB390_LEVEL);
    SNMP_Send_Status(TVB390_CNR);
    SNMP_Send_Status(TVB390_CNR_MODE);
    SNMP_Send_Status(TVB390_NULL_TP_ON_STOP);
    SNMP_Send_Status(TVB390_USE_AMP);
    SNMP_Send_Status(TVB390_USE_TAT4720);
    SNMP_Send_Status(TVB390_USE_TAT4720_COM);
	SNMP_Send_Status(TVB390_ATTEN);
	SNMP_Send_Status(TVB390_USE_AGC);

    //------------------------------------------------------------
    //TVB590-in-output-rate
    SNMP_Send_Status(TVB390_TS);
    SNMP_Send_Status(TVB390_TS_MAX);
    SNMP_Send_Status(TVB390_SYMBOL_RATE);
    SNMP_Send_Status(TVB390_SYMBOL_RATE_DEFAULT);

    //------------------------------------------------------------
    //TVB590-loop-adaptation
    SNMP_Send_Status(TVB390_PCR_PTS_DTS);
    SNMP_Send_Status(TVB390_CONTINUITY_COUNTER);
    SNMP_Send_Status(TVB390_TOT_TDT);
    SNMP_Send_Status(TVB390_TOT_TDT_UDT_TIME_SETTING);
	SNMP_Send_Status(TVB390_PCR_RESTAMP);

    //------------------------------------------------------------
    //TVB590-Sub-loop
    SNMP_Send_Status(TVB390_USE_SUB_LOOP);
    SNMP_Send_Status(TVB390_START_TIME);
    SNMP_Send_Status(TVB390_END_TIME);
    SNMP_Send_Status(TVB390_USE_FIXED_TS_RATE);

    //------------------------------------------------------------
    //TVB590-Sub-loop
    SNMP_Send_Status(TVB390_USE_IP_STREAMING);
    SNMP_Send_Status(TVB390_IP_INPUT_ADDRESS_URL);
    SNMP_Send_Status(TVB390_IP_INPUT_PORT);
    SNMP_Send_Status(TVB390_IP_INPUT_MULTICAST);
    SNMP_Send_Status(TVB390_IP_INPUT_LOCAL);
	//2010/3/25
	SNMP_Send_Status(TVB390_USE_MULTICAST_IP);

	SNMP_Send_Status(TVB390_DVB_T2_BANDWIDTH);
}
#else
void PlayForm::SNMP_All_DataSend()
{
    long    nBoardNum = gGeneral.gnActiveBoard;

    //1~7------------------------------------------------------------
    //TVB590-Board (6)
    SNMP_Send_Status(TVB390_BOARD_ID);
    SNMP_Send_Status(TVB390_BOARD_AUTHORIZATION);
    SNMP_Send_Status(TVB390_BOARD_LOCATION);
    SNMP_Send_Status(TVB390_BOARD_INSTALLATION);
    SNMP_Send_Status(TVB390_BOARD_LN);
    SNMP_Send_Status(TVB390_BOARD_SN);
	//2010/9/30 MULTIBOARD
	SNMP_Send_Status(TVB390_BOARD_ID_INFO);

    //1~19------------------------------------------------------------
    //TVB590-Status (19)
    SNMP_Send_Status(TVB390_BITRATE);
    SNMP_Send_Status(TVB390_PACKET_SIZE);
	if (gpConfig->gBC[nBoardNum].gnOutputClockSource)
		gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gdwPlayRate);
	else
		gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gGeneral.gnBitrate);			
    SNMP_Send_Status(TVB390_DISK_FREE_SIZE);
    SNMP_Send_Status(TVB390_FILE_PATH);
    SNMP_Send_Status(TVB390_FILE_LIST_COUNT);

    SNMP_Send_Status(TVB390_FILE_LIST_INDEX);
    SNMP_Send_Status(TVB390_FILE_LIST_NAME);
    SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
    SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
    SNMP_Send_Status(TVB390_PLAY_LIST_NAME);

    SNMP_Send_Status(TVB390_INPUT_STATUS);
    SNMP_Send_Status(TVB390_ELAPSED_TIME);
    SNMP_Send_Status(TVB390_RUN_TIME);
    SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
    SNMP_Send_Status(TVB390_RF_LEVEL_MAX);

    SNMP_Send_Status(TVB390_ATTEN_MIN);
    SNMP_Send_Status(TVB390_ATTEN_MAX);
    SNMP_Send_Status(TVB390_LOOP_COUNT);
    SNMP_Send_Status(TVB390_TS_IN_RATE);
    SNMP_Send_Status(TVB390_TS_RECORD_SIZE);
	
    //------------------------------------------------------------
    //TVB590-Run
    SNMP_Send_Status(TVB390_START_PLAYING);
    SNMP_Send_Status(TVB390_SET_PLAY_MODE);
    SNMP_Send_Status(TVB390_SET_INPUT_SOURCE);
    // write only: SNMP_Send_Status(TVB390_ADD_LIST);
    // write only: SNMP_Send_Status(TVB390_DELETE_LIST);
    SNMP_Send_Status(TVB390_START_RECORDING);
  	SNMP_Send_Status(TVB390_SELECT_LIST);
    // write only: SNMP_Send_Status(TVB390_MOVE_LIST_INDEX);
    // write only: SNMP_Send_Status(TVB390_APP_EXECUTE);
 	SNMP_Send_Status(TVB390_SET_FILE_POS);
    // write only: SNMP_Send_Status(TVB390_CHECK_LICENSE);
    // write only: SNMP_Send_Status(TVB390_UPDATE_LICENSE);
    // write only: SNMP_Send_Status(TVB390_SET_FACTORY_DEFAULT);
    
    //------------------------------------------------------------
    //TVB590-Modulator
    SNMP_Send_Status(TVB390_SELECT_SLOT);
    SNMP_Send_Status(TVB390_MODULATOR_TYPE);

    SNMP_Send_Status(TVB390_DVB_T_H_BANDWIDTH);
    SNMP_Send_Status(TVB390_DVB_T_H_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_T_H_CODERATE);
    SNMP_Send_Status(TVB390_DVB_T_H_TXMODE);
    SNMP_Send_Status(TVB390_DVB_T_H_GUARD_INTERVAL);

    SNMP_Send_Status(TVB390_QAM_A_CONSTELLATION);
    SNMP_Send_Status(TVB390_QAM_B_CONSTELLATION);
    SNMP_Send_Status(TVB390_QAM_B_INTERLEAVE);

    SNMP_Send_Status(TVB390_QPSK_CODERATE);
    SNMP_Send_Status(TVB390_QPSK_RRC_FILTER);
    SNMP_Send_Status(TVB390_QPSK_SPECTRUM);

    SNMP_Send_Status(TVB390_DVB_H_BANDWIDTH);
    SNMP_Send_Status(TVB390_DVB_H_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_H_CODERATE);
    SNMP_Send_Status(TVB390_DVB_H_TXMODE);
    SNMP_Send_Status(TVB390_DVB_H_GUARD_INTERVAL);
    SNMP_Send_Status(TVB390_DVB_H_MPE_FEC);
    SNMP_Send_Status(TVB390_DVB_H_TIME_SLICE);
    SNMP_Send_Status(TVB390_DVB_H_IN_DEPTH_INTERLEAVE);
    SNMP_Send_Status(TVB390_DVB_H_CELL_ID);

    SNMP_Send_Status(TVB390_DVB_S2_CONSTELLATION);
    SNMP_Send_Status(TVB390_DVB_S2_CODERATE);
    SNMP_Send_Status(TVB390_DVB_S2_PILOT);
    SNMP_Send_Status(TVB390_DVB_S2_ROLL_OFF);
    SNMP_Send_Status(TVB390_DVB_S2_SPECTRUM);

    SNMP_Send_Status(TVB390_ISDB_T_USE_TMCC);

    SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);

    SNMP_Send_Status(TVB390_DTMB_CONSTELLATION);
    SNMP_Send_Status(TVB390_DTMB_CODERATE);
    SNMP_Send_Status(TVB390_DTMB_INTERLEAVE);
    SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER);
    SNMP_Send_Status(TVB390_DTMB_CARRIER_NUMBER);
    SNMP_Send_Status(TVB390_DTMB_FRAME_HEADER_PN);
    SNMP_Send_Status(TVB390_DTMB_PILOT_INSERTION);

    //------------------------------------------------------------
    //TVB590-RF-IF OUTPUT
    SNMP_Send_Status(TVB390_RF);
    SNMP_Send_Status(TVB390_IF);
    SNMP_Send_Status(TVB390_LEVEL);
    SNMP_Send_Status(TVB390_CNR);
    SNMP_Send_Status(TVB390_CNR_MODE);
    SNMP_Send_Status(TVB390_NULL_TP_ON_STOP);
    SNMP_Send_Status(TVB390_USE_AMP);
    SNMP_Send_Status(TVB390_USE_TAT4720);
    SNMP_Send_Status(TVB390_USE_TAT4720_COM);
	
    SNMP_Send_Status(TVB390_ATTEN);
    SNMP_Send_Status(TVB390_USE_AGC);
    SNMP_Send_Status(TVB390_BERT_TYPE);
    SNMP_Send_Status(TVB390_BERT_PID_VALUE);

    //------------------------------------------------------------
    //TVB590-in-output-rate
    SNMP_Send_Status(TVB390_TS);
    SNMP_Send_Status(TVB390_TS_MAX);
    SNMP_Send_Status(TVB390_SYMBOL_RATE);
    SNMP_Send_Status(TVB390_SYMBOL_RATE_DEFAULT);

    //------------------------------------------------------------
    //TVB590-loop-adaptation
    SNMP_Send_Status(TVB390_PCR_PTS_DTS);
    SNMP_Send_Status(TVB390_CONTINUITY_COUNTER);
    SNMP_Send_Status(TVB390_TOT_TDT);
    SNMP_Send_Status(TVB390_TOT_TDT_UDT_TIME_SETTING);
    SNMP_Send_Status(TVB390_RESTAMP_DATE);
    SNMP_Send_Status(TVB390_RESTAMP_TIME);

    //------------------------------------------------------------
    //TVB590-Sub-loop
    SNMP_Send_Status(TVB390_USE_SUB_LOOP);
    SNMP_Send_Status(TVB390_START_TIME);
    SNMP_Send_Status(TVB390_END_TIME);
    SNMP_Send_Status(TVB390_USE_FIXED_TS_RATE);
	Sleep(10);

    //------------------------------------------------------------
    //TVB590-IP Streaming
    SNMP_Send_Status(TVB390_USE_IP_STREAMING);
    SNMP_Send_Status(TVB390_IP_RXIP);
    SNMP_Send_Status(TVB390_IP_RXMULTICASTIP);
    SNMP_Send_Status(TVB390_IP_USEMULTICAST);
    SNMP_Send_Status(TVB390_IP_INPUTRATE);
    SNMP_Send_Status(TVB390_IP_INPUT_ADDRESS_URL);
    SNMP_Send_Status(TVB390_IP_INPUT_PORT);
	
    //------------------------------------------------------------
    //TVB590-Error-Injection
    SNMP_Send_Status(TVB390_ERROR_USE_PACKET);
    SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_PACKET);
    SNMP_Send_Status(TVB390_ERROR_USE_BIT);
    SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_BIT);
    SNMP_Send_Status(TVB390_ERROR_NUM_PER_PACKET_BIT);
    SNMP_Send_Status(TVB390_ERROR_USE_BYTE);
    SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_BYTE);
    SNMP_Send_Status(TVB390_ERROR_NUM_PER_PACKET_BYTE);
//added 2010/4/7
	SNMP_Send_Status(TVB390_FIRMWARE_VERSION);
	SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
	SNMP_Send_Status(TVB390_IP_ADDRESS);
	SNMP_Send_Status(TVB390_SUBNET_MASK);
	SNMP_Send_Status(TVB390_GATEWAY);
	SNMP_Send_Status(TVB390_DHCP_ENABLE);
//------------------
	//kslee 2010/4/16
	SNMP_Send_Status(TVB390_BOARD_REV);
	//kslee 2010/4/16 JAVA TVBManager
	SNMP_Send_Status(TVB390_CMMB_MDIF);
	SNMP_Send_Status(TVB390_CMMB_CONSTELLATION);
	SNMP_Send_Status(TVB390_CMMB_RSCODING);
	SNMP_Send_Status(TVB390_CMMB_BYTECROSSING);
	SNMP_Send_Status(TVB390_CMMB_LDPC);
	SNMP_Send_Status(TVB390_CMMB_SCRAMBLE);
	SNMP_Send_Status(TVB390_CMMB_TIMESLICE);
	SNMP_Send_Status(TVB390_CMMB_MDIF_ITEM);

	//kslee 2010/3/24
	SNMP_Send_Status(TVB390_PCR_RESTAMP);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		//kslee 2010/4/20
		SNMP_Send_Status(TVB390_DVB_T2_BANDWIDTH);

		//kslee 2010/5/24
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER2);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER3);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER4);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER5);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER6);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER7);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER8);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER9);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER10);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER11);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER12);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER13);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER14);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER15);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER16);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER17);

		//2010/6/1 DVB-T2
		SNMP_Send_Status(TVB390_DVB_T2_BW);
		SNMP_Send_Status(TVB390_DVB_T2_BWT);
		SNMP_Send_Status(TVB390_DVB_T2_FFT);
		SNMP_Send_Status(TVB390_DVB_T2_GUARD);
		SNMP_Send_Status(TVB390_DVB_T2_L1_MOD);
		SNMP_Send_Status(TVB390_DVB_T2_PILOT_PATTERN);
		SNMP_Send_Status(TVB390_DVB_T2_NETWORK_ID);
		SNMP_Send_Status(TVB390_DVB_T2_SYSTEM_ID);
		SNMP_Send_Status(TVB390_DVB_T2_CELL_ID);
		SNMP_Send_Status(TVB390_DVB_T2_PID);
		SNMP_Send_Status(TVB390_DVB_T2_NUM_T2_FRAME);
		SNMP_Send_Status(TVB390_DVB_T2_NUM_DATA_SYMBOL);

		//2011/3/28 DVB-T2 MULTI-PLP =======================================================================
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_1);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_1);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_2);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_2);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_3);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_3);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_4);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_4);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_5);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_5);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_6);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_6);

		SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_7);
		SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_7);
	
	}
	//==================================================================================================
//	SNMP_Send_Status(TVB390_DVB_T2_FREQ);
	//SNMP_Send_Status(TVB390_DVB_T2_PLAYBACKRATE);
	//SNMP_Send_Status(TVB390_DVB_T2_MAXPLAYBACKRATE);
	//SNMP_Send_Status(TVB390_DVB_T2_ERROR_MSG);
	
	//2010/11/19 TVB593	EMERGENCY BROADCASTING
	SNMP_Send_Status(TVB390_EMERGENCY_BROADCASTING);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{	
		//2011/4/18 ISDB-S +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		SNMP_Send_Status(TVB390_ISDB_S_CONSTELLATION);
		SNMP_Send_Status(TVB390_ISDB_S_CODERATE);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_0);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_0);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_1);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_1);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_2);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_2);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_3);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_3);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_4);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_4);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_5);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_5);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_6);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_6);

		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_7);
		SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_7);

		SNMP_Send_Status(TVB390_ISDB_S_TMCC_USE);
		SNMP_Send_Status(TVB390_ISDB_S_IS_COMBINED_TS);
	}
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Sleep(10);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{	
		//2011/9/19 DVB-C2 =================================================================================
		SNMP_Send_Status(TVB390_DVBC2_BANDWIDTH);
		SNMP_Send_Status(TVB390_DVBC2_GUARDINTERVAL);
		SNMP_Send_Status(TVB390_DVBC2_STARTFREQ);
		SNMP_Send_Status(TVB390_DVBC2_L1TIMODE);
		SNMP_Send_Status(TVB390_DVBC2_NETID);
		SNMP_Send_Status(TVB390_DVBC2_SYSID);
		SNMP_Send_Status(TVB390_DVBC2_RESERVEDTONE);
		SNMP_Send_Status(TVB390_DVBC2_NUMNOTCH);
		SNMP_Send_Status(TVB390_DVBC2_NOTCHSTART);
		SNMP_Send_Status(TVB390_DVBC2_NOTCHWIDTH);
		SNMP_Send_Status(TVB390_DVBC2_DSLICE_TYPE);
		SNMP_Send_Status(TVB390_DVBC2_DSLICE_FEC_H);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_0);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_1);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_2);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_3);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_4);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_5);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_6);
		SNMP_Send_Status(TVB390_DVBC2_PLP_ID_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_COD_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_7);
		SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_7);
	}
		//==================================================================================================

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{	
		//2011/9/26 DVB-T2 IP ==============================================================================
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_MOD);
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_COD);
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_FEC);
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_ID);
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_HEM);
		SNMP_Send_Status(TVB390_DVBT2_IP_PLP_ROT);
		SNMP_Send_Status(TVB390_DVB_T2_IP_PLP_IL);
		//==================================================================================================
	}
	Sleep(10);
	//2011/10/27 DAC OFFSET ==============================================================================
	SNMP_Send_Status(TVB390_DAC_I_OFFSET);
	SNMP_Send_Status(TVB390_DAC_Q_OFFSET);
	SNMP_Send_Status(TVB390_DAC_MODULATION_MODE);
	//==================================================================================================
	//2011/11/08 I/Q PLAY ================================
	SNMP_Send_Status(TVB390_IQPLAY_SPECTRUM);
	SNMP_Send_Status(TVB390_IQPLAY_USE_CAPTURE);
	SNMP_Send_Status(TVB390_IQPLAY_MEM_USE);
	SNMP_Send_Status(TVB390_IQPLAY_MEMORY_SIZE);
	SNMP_Send_Status(TVB390_IQPLAY_CAPTURE_SUPPORT);
	//====================================================
}
#endif
//----------------------------------------------------------------------------------------------
#ifdef WIN32 
#else
void PlayForm::Display_File_Property(long nBoardNum, char *strFilename)
{
    int     index;
    __int64    lSizeKB;		//kslee 2010/4/2 long -> __int64
    //-----------------------------------------------------------------
    //--- TS Rate, Packet Size, Playback Time
    index = gUtilInd.SetLabPlayRateCalc(strFilename);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
	{
		lSizeKB = gUtilInd.Get_File_Size_KB(strFilename);
		if (lSizeKB > 0)
        {
            double  dblSize;
			dblSize = (dblSize / 6144) * 7300;
			if(gpConfig->gBC[nBoardNum].gdwPlayRate >= 1)
			{
				gGeneral.gnRunTime = (long) (dblSize/gpConfig->gBC[nBoardNum].gdwPlayRate*8);
				//PCR RESTAMP
				if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
				{
					if(gGeneral.gnBitrate > 0)
					{
						//2010/5/28
						gGeneral.gnRunTime = ((long)(dblSize / (double)gGeneral.gnBitrate * 8.0));
					}
				}
			}
		}
		if(index == 0)
		{
			gpConfig->gBC[nBoardNum].gnETI_Format = 0;
		}
		else if(index == 1)
		{
			gpConfig->gBC[nBoardNum].gnETI_Format = 1;
		}
		else
		{
			gpConfig->gBC[nBoardNum].gnETI_Format = -1;
		}
	}
	//DVB-T2 kslee 2010/4/20
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		//2010/11/12 4097 -> 65526
		char szResult[65526];
		gUtility.MyStrCpy(szResult, 65526, (char *)"");
		TSPH_RUN_T2MI_PARSER(nBoardNum, strFilename, szResult);
		gUtility.Get_T2MI_PARAMETERS_200(nBoardNum, szResult);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER2);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER3);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER4);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER5);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER6);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER7);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER8);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER9);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER10);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER11);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER12);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER13);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER14);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER15);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER16);
		SNMP_Send_Status(TVB390_DVB_T2_PARAMETER17);
	}
	//---ileSize,  Playback Time
	gUtilInd.GetFileSizeInSec(strFilename, nBoardNum);
	//-----------------------------------------------------------------
	//--- display remux info
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
    {
        char    strInfo[2048];
		gUtility.MyStrCpy(strInfo, 2048, (char *)"");
		//2011/11/22 improve ISDB-T UI
		gpConfig->gBC[nBoardNum].gnHaveTMCC = gWrapDll.UpdateRemuxInfo(nBoardNum, strFilename, strInfo);

		if((gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC) && gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
		}
		else if(gpConfig->gBC[nBoardNum].gnHaveTMCC == 1)
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
		}
		else
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
		}
		TSPH_SET_TMCC_REMUXER(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
        SNMP_Send_Status(TVB390_ISDB_S_TMCC_USE);
        SNMP_Send_Status(TVB390_ISDB_T_13SEG_USE_TMCC);
		gUtility.Get_RemuxInfo(nBoardNum, strInfo);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER2);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER3);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER4);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER5);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER6);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER7);
		SNMP_Send_Status(TVB390_ISDB_T_PARAMETER8);
    }
    //ATSC-M/H kslee 2010/2/3
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
	{
		UpdateMHEpacketInfo(nBoardNum, strFilename);
	}

    if (gpConfig->gBC[nBoardNum].gnCmdPlayrate > 0)
    {
        Display_PlayRate(gpConfig->gBC[nBoardNum].gnCmdPlayrate);
        gGeneral.gnBitrate = gpConfig->gBC[nBoardNum].gnCmdPlayrate;
        gpConfig->gBC[nBoardNum].gnCmdPlayrate = 0;
    }

	if (index > 0)
    {
		if(index != 2)
		{
			if (gpConfig->gBC[nBoardNum].bPlayingProgress == false ||
					gpConfig->gBC[nBoardNum].bForcedPlayRate == true)
			{
				if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
					Display_PlayRate(gpConfig->gBC[nBoardNum].gdwPlayRate);
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
					Display_PlayRate(gGeneral.gnBitrate);

				//CMMB, DVB_T2 2010/23/25
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
					Display_PlayRate(gGeneral.gnBitrate);
				//2010/6/9
				else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
				{
					
					if(gGeneral.gnT2MI_playrate > 1)
					{
						Display_PlayRate(gGeneral.gnT2MI_playrate);
					}
					else
					{	
						long PlaybackRate = 0;
						for(int i = 0; i < MAX_PLP_TS_COUNT; i++)
						{
							if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0)
							{
								PlaybackRate = PlaybackRate + gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[i];
							}
						}
						//===============================================================================================
						Display_PlayRate(PlaybackRate);
					}
				}
			}
			//2010/12/06 ISDB-S =====================================================================================================================
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
			{
				int org_playrate, playrate;
				org_playrate = gGeneral.gnBitrate;

				if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
				{
					playrate = gUtilInd.CalcBurtBitrate(nBoardNum);
				}
				else
				{
					int i,j,k;
					j = 0;
					for(i = 0 ; i < MAX_TS_COUNT ; i++)
					{
						if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1)
						{
							k = gUtilInd.CalcDatarate(gpConfig->gBC[nBoardNum].gnConstellation_M[i], gpConfig->gBC[nBoardNum].gnCoderate_M[i],
								gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
							j = j + k;
						}
					}
					playrate = j;
				}
				Display_PlayRate(playrate);
				gGeneral.gnBitrate = playrate;
				if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
				{
					if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
					{
						if(playrate > org_playrate)
							gGeneral.gnBitrate = org_playrate;
					}
				}
			}
			//=======================================================================================================================================

		}	
    }
	//2011/11/17 IQ NEW FILE FORMAT
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{
		Display_SymbolRate(gpConfig->gBC[nBoardNum].gnSymbolRate);
		if(gpConfig->gBC[nBoardNum].gnSymbolRate >= 1000000 && gpConfig->gBC[nBoardNum].gnSymbolRate <= 90000000) 
			gWrapDll.Set_Symbolrate(nBoardNum, gpConfig->gBC[nBoardNum].gnSymbolRate);
	}
	if(index != -2)
	{
		lSizeKB = gUtilInd.Get_File_Size_KB(strFilename);
		if (lSizeKB > 0)
		{
            double  dblSize = (double)(lSizeKB*1000);
            char    text[100];
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
			{
				dblSize = (dblSize / 6144) * 7300;
			}
			if(gpConfig->gBC[nBoardNum].gdwPlayRate >= 1)
			{
				gGeneral.gnRunTime = (long) (dblSize/gpConfig->gBC[nBoardNum].gdwPlayRate*8);
				//kslee 2010/3/18
				//PCR RESTAMP
				if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
				{
					if(gGeneral.gnBitrate > 0)
					{
						//2010/5/28
						gGeneral.gnRunTime = ((long)(dblSize / (double)gGeneral.gnBitrate * 8.0));
					}
				}
			}
		}
	}
	//2011/2/11 ISDB-S Combined TS
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		gpConfig->gBC[nBoardNum].gnCombinedTS = 0;
		if(gGeneral.gnPacketSize == 204 || gGeneral.gnPacketSize == 188)
		{
			if(TSPH_IS_COMBINED_TS(nBoardNum, strFilename) == 0 && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer != 1)
			{
				//2011/3/18 ISDB-S =================================================================================
				int bitrate;
				bitrate = TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(nBoardNum, strFilename);
				if(bitrate > 0)
				{
					Display_PlayRate(bitrate);
					gGeneral.gnBitrate = bitrate;
					lSizeKB = gUtilInd.Get_File_Size_KB(strFilename);
					if (lSizeKB > 0)
                    {
                        double  dblSize = (double)(lSizeKB*1000);
						if(gpConfig->gBC[nBoardNum].gdwPlayRate >= 1)
						{
							
							if(gGeneral.gnPacketSize == 204)
							{
								gGeneral.gnRunTime = (long) (dblSize / 52170000.0 * 8.0 * 188.0 / 204.0);
							}
							else
							{
								gGeneral.gnRunTime = (long) (dblSize / 52170000.0 * 8.0);
							}
							
							//kslee 2010/3/18
							//PCR RESTAMP
							if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
							{
								if(gGeneral.gnBitrate > 0)
								{
									//2010/5/28
									if(gGeneral.gnPacketSize == 204)
									{
										gGeneral.gnRunTime = ((long)(dblSize / 52170000.0 * 8.0 * 188.0 / 204.0));
									}
									else
									{
										gGeneral.gnRunTime = ((long)(dblSize / 52170000.0 * 8.0));
									}
								}
							}
						}
					}
				}
				//==================================================================================================
				gpConfig->gBC[nBoardNum].gnCombinedTS = 1;
			}
		}
	}
	//2011/5/23 DVB-C2 MULTI PLP ====================>>
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
		int tot_playrate = 0;
		char strTemp[256];
		for(int i = 0; i < DVB_C2_MAX_PLP_TS_COUNT; i++)
		{
			if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
			{
				tot_playrate = tot_playrate + gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[i];
			}
		}
		char szResult[65526];
		gUtility.MyStrCpy(szResult, 65526, (char *)"");

		int index;
		int fileari = 1;
		if(gpConfig->gBC[nBoardNum].nFileListIndexCount < 1)
		{
			fileari = 0;
		}
		else
		{
			strcpy(strTemp, gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
		}
		if((strstr(strTemp, ".c2") || strstr(strTemp, ".C2")) == false || fileari == 0)
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
			if(tot_playrate > 0)
			{
				Display_PlayRate(tot_playrate);
			}
		}
		else
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
			tot_playrate = TSPH_RUN_C2MI_PARSER(nBoardNum, strFilename, szResult);
			if(tot_playrate > 0)
			{
				gGeneral.gnBitrate = tot_playrate;
				Display_PlayRate(tot_playrate);
			}
		}
	}

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
		UpdateCmmbUI(nBoardNum, strFilename);
		if(gpConfig->gBC[nBoardNum].gnCMMB_Params_Count > 0)
		{
			OnCbnSelchangeParam1(0);
		}
		//JAVA TVBManager
		SNMP_Send_Status(TVB390_CMMB_MDIF);
		SNMP_Send_Status(TVB390_CMMB_CONSTELLATION);
		SNMP_Send_Status(TVB390_CMMB_RSCODING);
		SNMP_Send_Status(TVB390_CMMB_BYTECROSSING);
		SNMP_Send_Status(TVB390_CMMB_LDPC);
		SNMP_Send_Status(TVB390_CMMB_SCRAMBLE);
		SNMP_Send_Status(TVB390_CMMB_TIMESLICE);
		SNMP_Send_Status(TVB390_CMMB_MDIF_ITEM);
	}
//#ifndef STANDALONE
	//2011/11/22 improve ISDB-T UI
	if( gpConfig->gBC[nBoardNum].gnHaveTMCC == 0)
	{
		SetDefaultValue_noTMCC(strFilename);
	}
//#endif
    SNMP_Send_Status(TVB390_RUN_TIME);
    SNMP_Send_Status(TVB390_BITRATE);
    SNMP_Send_Status(TVB390_PACKET_SIZE);
    SNMP_Send_Status(TVB390_TS);
    SNMP_Send_Status(TVB390_SYMBOL_RATE);
	//2011/4/18 ISDB-S
    SNMP_Send_Status(TVB390_ISDB_S_IS_COMBINED_TS);

    //2011/1/6 ISDB-T TMCC Setting ============================================================================================
	if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
    {
		SendTMCCData();
	}
	//=========================================================================================================================
}

#endif


//---------------------------------------------------------------
#ifdef WIN32 
void PlayForm::UpdateCmmbInformation(long nBoardNum, char *szListFileName)
{
	char szResult[4096];
	unsigned int i, j; 
	
	if(strlen(szListFileName) < 1)
		return;

	for(i = 0 ; i < MAX_CMMB_TIME_SLOT_COUNT; i++)
	{
		for(j = 0; j < MAX_CMMB_PARAM_COUNT; j++)
		{
			gpConfig->gBC[nBoardNum].gnCMMB_Params[i][j] = -1;
		}
	}
	gpConfig->gBC[nBoardNum].gnCMMB_Params_Count = 0;

//	Combo_PARAM1->Text = "";
//	gBaseUtil.ClearItem_Combo(Combo_PARAM1);

	for(i=0; i<4096;i++)
		szResult[i] = 0;


	int res = gWrapDll.TSPH_RUN_MFS_PARSER(nBoardNum, szListFileName, szResult);
	long ii, jj;
	char temp[4096];
	char param[16];
	long index1, index2;
	ii = 0;
	index1 = 0;
	for(i = 0; i < strlen(szResult); i++)
	{
		if(szResult[i] == ',')
		{
			temp[ii] = '\0';
			jj = 0;
			index2 = 0;
			for(j = 0; j < strlen(temp); j++)
			{
				if(temp[j] == ' ')
				{
					param[jj] = '\0';
					long a = atol(param);
					if(a < 0)
					{
						jj = 0;
						index2++;
						continue;
					}
					gpConfig->gBC[nBoardNum].gnCMMB_Params[index1][index2] = atol(param);
					jj = 0;
					index2++;
					continue;
				}
				param[jj++] = temp[j];
			}
			ii = 0;
			index1++;
			continue;
		}
		temp[ii++] = szResult[i];
	}
	gpConfig->gBC[nBoardNum].gnCMMB_Params_Count = index1;
	index1 = 0;

	//for(ii = 0; ii < gpConfig->gBC[nBoardNum].gnCMMB_Params_Count; ii++)
	//{
	//	if(gpConfig->gBC[nBoardNum].gnCMMB_Params[ii][7] >= 0)
	//	{
	//		Combo_PARAM1->Items->Add(gpConfig->gBC[nBoardNum].gnCMMB_Params[ii][7].ToString());
	//	}
	//}
}

#else
#endif
//---------------------------------------------------------------------------
#ifdef WIN32 
void PlayForm::ChangeAdaptor(long lIndex)
{
    long	nBoardNum, nPrevBoardNum;
    nBoardNum = gpConfig->nBoardRealSlot[lIndex];
    nPrevBoardNum = gGeneral.gnActiveBoard;
    gGeneral.nConfirmPlaying = 0;

    //--- if same board, then return
    if (nBoardNum == gGeneral.gnActiveBoard)
        return;


    gGeneral.nConfirmPlaying = 1;
    gGeneral.gnActiveBoard = nBoardNum;

	//---------------------------------------------------------------
    // Save FILE/PLAY List
    SaveStreamListInfo(nPrevBoardNum, gpConfig->gBC[nPrevBoardNum].gnModulatorMode);
	gpConfig->gBC[nBoardNum].nFileListIndexCur = -1;
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 512, (char *)"");
	LabPlayRateCalc->Text = "-";
	Label_TS->Text = "-";
	Display_LabFileSize("00:00:00");
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC) 
		gGeneral.gnChangeAdapt_Flag = 1;
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == MULTIPLE_DVBT)
	{
		if(gpConfig->gBC[gGeneral.gnActiveBoard].gn_IsVirtualSlot == 0 && gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == false && gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false)
		{
			int nVirCnt;
			int RealVirSlot[4];
			gWrapDll.TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB)
				nVirCnt = gWrapDll.TSPL_CNT_MULTI_VSB_RFOUT_EX(nBoardNum);
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
				nVirCnt = gWrapDll.TSPL_CNT_MULTI_QAM_RFOUT_EX(nBoardNum);
			else
				nVirCnt = gWrapDll.TSPL_CNT_MULTI_DVBT_RFOUT_EX(nBoardNum);

			if(nVirCnt > 1)
			{
				for(int i = 1; i < nVirCnt; i++)
				{
					if(gpConfig->gBC[RealVirSlot[i]].bPlayingProgress == true)
					{
						gGeneral.gnChangeAdapt_Flag = 1;
						break;
					}
				}
			}
		}
	}
    //--- Save/Restore Variables
	//2012/3/26 Multiple VSB,QAM-B
	if(gpConfig->gBC[nPrevBoardNum].gn_IsVirtualSlot == 0)
	{
		gRegVar.SaveNewModulatorMode(nBoardNum, gpConfig->gBC[nPrevBoardNum].gnModulatorMode);
	}
	gRegVar.SaveVariables(nPrevBoardNum);
	//2012/3/26 Multiple VSB,QAM-B
    if(gpConfig->gBC[nBoardNum].gnCmdAutoRun == 0 && gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 0)
	{
		gRegVar.RestoreVariables(nBoardNum);
	//2011/5/4 AD9852 OVER CLOCK
		if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			gWrapDll.TSPL_SET_AD9852_MAX_CLOCK_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnAD9852_Overclock);
	}
    gUtilInd.LogMessage ("");


	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnBoardId = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBoardId;
		gpConfig->gBC[nBoardNum].gnBoardRev = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBoardRev;
		gpConfig->gBC[nBoardNum].gnModulatorMode = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnModulatorMode;
		gWrapDll.TSPH_SET_MODULATOR_TYPE(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
		gRegVar.RestoreVariables(nBoardNum);
		gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreqUnit;
		//2012/6/28 multi dvb-t
		gpConfig->gBC[nBoardNum].gnBandwidth = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBandwidth;
		gpConfig->gBC[nBoardNum].gnQAMMode = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnQAMMode;
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
		{
			gpConfig->gBC[nBoardNum].gnSymbolRate = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnSymbolRate;
		}
		gpConfig->gBC[nBoardNum].gnBypassAMP = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBypassAMP;
		gpConfig->gBC[nBoardNum].gnAGC = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnAGC;
		
		gpConfig->gBC[nBoardNum].gdRfLevelValue = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelValue;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_max = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelRange_max;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_min = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelRange_min;

		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);

			if(gpConfig->gBC[nBoardNum].gnBoardId == 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
				gpConfig->gBC[nBoardNum].gnBoardId == 21 || gpConfig->gBC[nBoardNum].gnBoardId == 22 || gpConfig->gBC[nBoardNum].gnBoardId == 15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
			{
                gWrapDll.TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode,
	                    0,
		                0,
			            0,
				        gpConfig->gBC[nBoardNum].gnCell_Id);
			}
			else
				gWrapDll.TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);

            gWrapDll.TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            gWrapDll.TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
            gWrapDll.TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
		}
		Display_VirtualBoard_ModulatorName(nBoardNum);
		UpdateModulatorConfigUI(nBoardNum);
	}
	else
	{
	    if (gpConfig->gBC[nBoardNum].gnOpenSystem == 0)
		{
			gWrapDll.Open_System(nBoardNum);
	    } 
		//I/Q PLAY/CAPTURE
		gpConfig->gBC[nBoardNum].gnIQ_play_support = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 2);
		gpConfig->gBC[nBoardNum].gnIQ_capture_support = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 3);
		//single tone
		gpConfig->gBC[nBoardNum].gnSingleToneEnabled = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 6);
		//TS packet count control
		gpConfig->gBC[nBoardNum].gnSupport_ts_Pkt_cnt_cntl = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 8);
	
		//RBF revision
		gpConfig->gBC[nBoardNum].gnSupport_Rbf_revision = gWrapDll.TSPL_GET_FPGA_INFO_EX(nBoardNum, 9);

	    UpdateModulatorConfigUI(nBoardNum);
	}

	int _Val_;
	if((gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		&& gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
			_Val_ = ((gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3); 
		else
			_Val_ = gpConfig->gBC[nBoardNum].gn_StreamNum;
		gWrapDll.TSPH__SEL_TS_of_ASI310OUT(nBoardNum, _Val_);
	}

    //--------------------------------------------------------------
    // if playlist is not exsit

	if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].bPlayingProgress == 1 &&
		(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM || gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC))
	{
		gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0 * (gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));
	}
}

#else

//---------------------------------------------------------------------------
void PlayForm::ChangeAdaptor(long lIndex)
{
    long	i, nBoardNum, nPrevBoardNum;
	int		nCount;

    nBoardNum = gpConfig->nBoardRealSlot[lIndex];
    nPrevBoardNum = gGeneral.gnActiveBoard;
    gGeneral.nConfirmPlaying = 0;

    //--- if same board, then return
    if (nBoardNum == gGeneral.gnActiveBoard)
        return;

	//printf("[dbg-seq] - change adaptor : [d'%d]\n", nBoardNum);

    gGeneral.nConfirmPlaying = 1;
    gGeneral.gnActiveBoard = nBoardNum;

	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorSource == SMPTE310M_SRC) 
		gGeneral.gnChangeAdapt_Flag = 1;
	

	//---------------------------------------------------------------
    // Save FILE/PLAY List
    SaveStreamListInfo(nPrevBoardNum, gpConfig->gBC[nPrevBoardNum].gnModulatorMode);

    //--- Save/Restore Variables
	gRegVar.SaveVariables(nPrevBoardNum);
	//2010/6/10
    if(gpConfig->gBC[nBoardNum].gnCmdAutoRun == 0 && gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 0)
	{
		gRegVar.RestoreVariables(nBoardNum);
	}
    gUtilInd.LogMessage ("");

	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true)
	{
		gGeneral.gnChangeAdapt_Flag = 0;
		return;
	}
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gnBoardId = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBoardId;
		gpConfig->gBC[nBoardNum].gnBoardRev = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBoardRev;
		gpConfig->gBC[nBoardNum].gnModulatorMode = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnModulatorMode;
		TSPH_SET_MODULATOR_TYPE(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
		gRegVar.RestoreVariables(nBoardNum);
		gpConfig->gBC[nBoardNum].gnRFOutFreqUnit = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreqUnit;
		//2012/6/28 multi dvb-t
		gpConfig->gBC[nBoardNum].gnBandwidth = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBandwidth;
		gpConfig->gBC[nBoardNum].gnQAMMode = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnQAMMode;
		gpConfig->gBC[nBoardNum].gnBypassAMP = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnBypassAMP;
		gpConfig->gBC[nBoardNum].gnAGC = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnAGC;
		
		gpConfig->gBC[nBoardNum].gdRfLevelValue = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelValue;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_max = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelRange_max;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_min = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdRfLevelRange_min;

		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
            
			TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBandwidth, gpConfig->gBC[nBoardNum].gnRFOutFreq);

			if(gpConfig->gBC[nBoardNum].gnBoardId == 48 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 || 
				gpConfig->gBC[nBoardNum].gnBoardId == 21 || gpConfig->gBC[nBoardNum].gnBoardId == 22 || gpConfig->gBC[nBoardNum].gnBoardId == 15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
			{
                TVB380_SET_MODULATOR_DVBH_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode,
	                    0,
		                0,
			            0,
				        gpConfig->gBC[nBoardNum].gnCell_Id);
			}
			else
				TVB380_SET_MODULATOR_TXMODE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnTxmode);

            TVB380_SET_MODULATOR_CODERATE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnCodeRate);
            TVB380_SET_MODULATOR_GUARDINTERVAL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnGuardInterval);
            TVB380_SET_MODULATOR_CONSTELLATION_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnConstellation);
		}
		UpdateModulatorConfigUI(nBoardNum);
	}
	else
	{
	    if (gpConfig->gBC[nBoardNum].gnOpenSystem == 0)
		{
			gWrapDll.Open_System(nBoardNum);
			Initial_Check(nBoardNum);
	    } else
		{
			//2010/7/18 I/Q PLAY/CAPTURE
			gpConfig->gBC[nBoardNum].gnIQ_play_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 2);
			gpConfig->gBC[nBoardNum].gnIQ_capture_support = TSPL_GET_FPGA_INFO_EX(nBoardNum, 3);
			//single tone
			gpConfig->gBC[nBoardNum].gnSingleToneEnabled = TSPL_GET_FPGA_INFO_EX(nBoardNum, 6);
	        UpdateModulatorConfigUI(nBoardNum);
		
		}
	}

	int _Val_;
	if((gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		&&	gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
			_Val_ = ((gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3); 
		else
			_Val_ = gpConfig->gBC[nBoardNum].gn_StreamNum;
		TSPH__SEL_TS_of_ASI310OUT(nBoardNum, _Val_);
	}

    //--------------------------------------------------------------
    // if playlist is not exsit

	if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].bPlayingProgress == 1 &&
		(gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM || gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC))
	{
		gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0 * (gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));
	}
    else if (gpConfig->gBC[nBoardNum].nPlayListIndexCount <= 0)
    {
			if(gpConfig->gBC[nBoardNum].nFileListIndexCount == 0)
				gpConfig->gBC[nBoardNum].nFileListIndexCur = -1;

            if (gpConfig->gBC[nBoardNum].nFileListIndexCur >= 0)
            {
            	UpdateFileListDisplay();
                if (gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                {
                    gpConfig->gBC[nBoardNum].dwFileSize = (__int64) (356400.0*(gpConfig->gBC[nBoardNum].gdwPlayRate/8.0));
                } else if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
                {
                    __int64 dwSize;
                    dwSize = 0;

                    if (gUtility.Is_File_Exist(gpConfig->gBC[nBoardNum].szCurFileName) == true)
                    {
                        dwSize = gUtilInd.Get_File_Size_BYTE(gpConfig->gBC[nBoardNum].szCurFileName);

                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            dwSize = dwSize*7300/6144;
                    }
                    gpConfig->gBC[nBoardNum].dwFileSize = dwSize;
                }
            }
    } else
    {
		nCount = gpConfig->gBC[nBoardNum].nPlayListIndexCount;
        if (gpConfig->gBC[nBoardNum].nPlayListIndexCur >= 0 && gpConfig->gBC[nBoardNum].nPlayListIndexCur < nCount)
            UpdatePlayListDisplay();
    }
}


#endif

//------------------------------------------------------------------------------
#ifdef WIN32 

void PlayForm::SNMP_DataArrival()

{
    char    msg[200];
    char    str[100];
    int     msg_type, i;
    long    nParam;
    unsigned long   dwParam;
    long    nBoardNum = gGeneral.gnActiveBoard;
    long    lMod = gpConfig->gBC[nBoardNum].gnModulatorMode;

	int init = 0;
	int cnt = 0;

    gUtility.MyStrCpy(msg, 200, recv_msg);

    sscanf_s(msg, "%d %d", &msg_type, &nParam);
    
	switch (msg_type)
    {
        //=======================================================================
        //-----------------------------------------------------------------------
        // TVB590-RUN (8)
        case TVB390_START_PLAYING:      // 1.3.6.1.4.1.10187.590.1.3.1
            if (nParam == 0 && gpConfig->gBC[nBoardNum].bPlayingProgress == false)  //already stopped
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].bPlayingProgress == true)   // already playing
                return;
            OnBnClickedComf1();
            break;
            
        case TVB390_SET_PLAY_MODE:
            if (nParam == 0 && gpConfig->gBC[nBoardNum].gbRepeatMode == 0)  // already once
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].gbRepeatMode == 1)  // already loop
                return;
            OnBnClickedComf2();
            break;
            
        case TVB390_SET_INPUT_SOURCE:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                return;     // can't change during playback/recording/TDMB

            if (gpConfig->gBC[nBoardNum].gnBoardId < 45 && gpConfig->gBC[nBoardNum].gnBoardId != 10 && gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 16 &&
				gpConfig->gBC[nBoardNum].gnBoardId != 0xF && gpConfig->gBC[nBoardNum].gnBoardId != 11 && gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId !=12)		//2013/5/27 TVB599 0xC
                return;
			
			if(nParam < 0 || nParam > 6)
				return;

			if(nParam == FILE_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 1;
				gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = RECV_IP_STREAM;
			}
			else if (nParam == DVBASI_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
				gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;
			}
			else if (nParam == SMPTE310M_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
				gpConfig->gBC[nBoardNum].gnModulatorSource = DVBASI_SRC;
			}
			else if(nParam == 5)
			{
				gpConfig->gBC[nBoardNum].gnModulatorSource = SMPTE310M_SRC;
			}
			else
			{
				return;
			}
			OnBnClickedComf3();
            break;

        case TVB390_ADD_LIST:
            break;
            
        case TVB390_DELETE_LIST:
            break;
            
        case TVB390_START_RECORDING:
            if (nParam == 0 && gpConfig->gBC[nBoardNum].bRecordInProgress == false)  // already stopped
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].bRecordInProgress == true)   // already recording
                return;

            if (nParam == 1)
                OnBnClickedComf6();
            else
                OnBnClickedComf1();
            break;

        case TVB390_SELECT_LIST:
			//2013/3/12 TODO
   //         if (nParam == 0)    // filelist
			//{
			//	gBaseUtil.SetFocus_Common(FileListBox);
			//}
			//else
			//{   
			//	OnLbnSelchangePlaylist();
			//	gBaseUtil.SetFocus_Common(PlayList);
			//}
			break;

        case TVB390_MOVE_LIST_INDEX:
			//2013/3/12 TODO
   //         if (nParam == 1)
			//{
			//	if (gpConfig->gBC[nBoardNum].bPlayingProgress == false &&
   //                 gpConfig->gBC[nBoardNum].bDelayinProgress == false &&
   //                 gpConfig->gBC[nBoardNum].bRecordInProgress == false)
			//	{                
			//		{
   //                     i = gBaseUtil.CountItems_List(FileListBox) - 1;

			//			if (i < 0)
			//				return;

			//			if(gBaseUtil.SelectedIndex_List(FileListBox) == 0)
			//			{
			//				if(gBaseUtil.CountItems_List(PlayList) > 0)
			//				{
			//					gBaseUtil.SetCurrentSel_List(PlayList, gBaseUtil.CountItems_List(PlayList) -1);
			//					UpdatePlayListDisplay();
   //                             return;
			//				}
			//			}
			//			DecFileListCursor();
   //                     
			//		} else
			//		{
			//			i = gBaseUtil.CountItems_List(PlayList) - 1;

			//			if (i < 0)
			//			{
			//				return;
			//			}
   //                     
			//			if(gBaseUtil.SelectedIndex_List(PlayList) == 0)
			//			{
			//				if(gBaseUtil.CountItems_List(FileListBox) > 0)
			//				{
			//					gBaseUtil.SetCurrentSel_List(FileListBox, gBaseUtil.CountItems_List(FileListBox) -1);
			//					UpdateFileListDisplay();
   //                             return;
			//				}
			//			}
   //                     DecPlayListCursor();
			//		}						
			//	}
			//}
			//else
			//{
			//	if (gpConfig->gBC[nBoardNum].bPlayingProgress == false &&
   //                 gpConfig->gBC[nBoardNum].bDelayinProgress == false &&
   //                 gpConfig->gBC[nBoardNum].bRecordInProgress == false)
			//	{                
			//		{
   //                     i = gBaseUtil.CountItems_List(FileListBox) - 1;

			//			if (i < 0)
			//				return;
   //                     
			//			if(gBaseUtil.SelectedIndex_List(FileListBox) == i && gBaseUtil.CountItems_List(PlayList) > 0)
			//			{
			//				gBaseUtil.SetCurrentSel_List(PlayList, 0);
			//				UpdatePlayListDisplay();
   //                         return;
			//			}
			//			IncFileListCursor();
			//		} else
			//		{
			//			i = gBaseUtil.CountItems_List(PlayList) - 1;

			//			if (i < 0)
			//			{
			//				return;
			//			}
   //                     
			//			if(gBaseUtil.SelectedIndex_List(PlayList) == i)
			//			{
			//				gBaseUtil.SetCurrentSel_List(FileListBox, 0);
			//				UpdateFileListDisplay();
			//				return;
			//			}
			//			IncPlayListCursor();
			//		}

			//	}       
			//}

			//if(gBaseUtil.CountItems_List(PlayList) > 0)
			//	gpConfig->gBC[nBoardNum].nPlayListIndexCur = gBaseUtil.SelectedIndex_List(PlayList);
			//if(gBaseUtil.CountItems_List(FileListBox) > 0)
			//	gpConfig->gBC[nBoardNum].nFileListIndexCur = gBaseUtil.SelectedIndex_List(FileListBox);

			//SNMP_Send_Status(TVB390_FILE_PATH);
			//SNMP_Send_Status(TVB390_FILE_LIST_COUNT);
			//SNMP_Send_Status(TVB390_FILE_LIST_INDEX);
			//SNMP_Send_Status(TVB390_FILE_LIST_NAME);
			//SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
			//SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
			//SNMP_Send_Status(TVB390_PLAY_LIST_NAME);
            break;
		case TVB390_APP_EXECUTE:
			if(nParam == 1)
				return;
			this->Close();
			break;
        //=======================================================================
        //-----------------------------------------------------------------------
        // TVB590-MODULATOR (1+1+)
        case TVB390_SELECT_SLOT:  // 1~23
            for (i = 0; i <= MAX_BOARD_COUNT; i++)
            {
                if (gpConfig->nBoardRealSlot[i] == nParam)
                {
                    gBaseUtil.SetCurrentSel_Combo(Combo_ADAPTOR, i);
					OnCbnSelchangeAdaptor();
                    break;
                }
            }
            break;

        case TVB390_MODULATOR_TYPE:
            //--- find corresponding index
            for (i = 0; i < gpConfig->gBC[nBoardNum].giNumModulator; i++)
            {
                if (gpConfig->gBC[nBoardNum].giTypeComboMod[i] == nParam)
                {
					gBaseUtil.SetCurrentSel_Combo(Combo_MODULATOR_TYPE, i);
					OnCbnSelchangeModulatorType();
                    return;
                }
            }
            break;

        //-----------------------------------------------
        // DVB-T/H Parameter

        case TVB390_DVB_T_H_BANDWIDTH:
        case TVB390_DVB_H_BANDWIDTH:
            if (lMod != DVB_T && lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 3)
                return;

			if (lMod == DVB_T && nParam > 2)
				return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;
            
        case TVB390_DVB_T_H_CONSTELLATION:
        case TVB390_DVB_H_CONSTELLATION:
            if (lMod != DVB_T && lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 2)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, nParam);
			OnCbnSelchangeParam2();
            break;

        case TVB390_DVB_H_CODERATE:
        case TVB390_DVB_T_H_CODERATE:
            if (lMod != DVB_T && lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 4)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, nParam);
			OnCbnSelchangeParam3();
            break;

        case TVB390_DVB_H_TXMODE:
        case TVB390_DVB_T_H_TXMODE:
            if (lMod != DVB_T && lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 2)
                return;

			if (lMod == DVB_T && nParam > 1)
				return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM4, nParam);
			OnCbnSelchangeParam4();
            break;

        case TVB390_DVB_H_GUARD_INTERVAL:
        case TVB390_DVB_T_H_GUARD_INTERVAL:
            if (lMod != DVB_T && lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 3)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM5, nParam);
			OnCbnSelchangeParam5();
            break;

        //-----------------------------------------------
        // QAM-A Parameter
        case TVB390_QAM_A_CONSTELLATION:
            if (lMod != QAM_A)
                return;

            if (nParam < 0 || nParam > 4)
                return;
 
            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

        //-----------------------------------------------
        // QAM-B Parameter
        case TVB390_QAM_B_CONSTELLATION:
            if (lMod != QAM_B && lMod != MULTIPLE_QAMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

        case TVB390_QAM_B_INTERLEAVE:
            if (lMod != QAM_B && lMod != MULTIPLE_QAMB)
                return;

            if (nParam < 0 || nParam > 12)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, nParam);
			OnCbnSelchangeParam2();
            break;

        //-----------------------------------------------
        // QPSK Parameter
        case TVB390_QPSK_CODERATE:
            if (lMod != QPSK)
                return;

            if (nParam < 0 || nParam > 4)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

        case TVB390_QPSK_RRC_FILTER:
            if (lMod != QPSK || gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, nParam);
			OnCbnSelchangeParam2();
            break;

        case TVB390_QPSK_SPECTRUM:
            if (lMod != QPSK)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, nParam);
			OnCbnSelchangeParam3();
            break;

        //--------------------------------------------------------------
        // DVB-H Parameter
        case TVB390_DVB_H_MPE_FEC:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnMPE_FEC = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_MPE_FEC);
            break;

        case TVB390_DVB_H_TIME_SLICE:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnTime_Slice = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_TIME_SLICE);
            break;

        case TVB390_DVB_H_IN_DEPTH_INTERLEAVE:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			i = gBaseUtil.SelectedIndex_Combo(Combo_PARAM2);
            if (i == TX_8K)
                nParam = 1;
            gpConfig->gBC[nBoardNum].gnIn_Depth = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_IN_DEPTH_INTERLEAVE);
            break;
            
        case TVB390_DVB_H_CELL_ID:
            if (lMod != DVB_H)
                return;

            gpConfig->gBC[nBoardNum].gnCell_Id = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_CELL_ID);
            break;

        //--------------------------------------------------------------
        // DVB-S2 Parameter
        case TVB390_DVB_S2_CONSTELLATION:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 3)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

        case TVB390_DVB_S2_CODERATE:
            if (lMod != DVB_S2)
                return;

			if(nParam < 0 || nParam >= gBaseUtil.CountItems_Combo(Combo_PARAM2))
				return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, nParam);
			OnCbnSelchangeParam2();
            break;
            
        case TVB390_DVB_S2_PILOT:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, nParam);
			OnCbnSelchangeParam3();
            break;

        case TVB390_DVB_S2_ROLL_OFF:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 3)
                return;

			if((gpConfig->gBC[nBoardNum].gnBoardId == 20) && (nParam == 3)) 
				return;
			
			gBaseUtil.SetCurrentSel_Combo(Combo_PARAM4, nParam);
			OnCbnSelchangeParam4();
            break;

        case TVB390_DVB_S2_SPECTRUM:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM5, nParam);
			OnCbnSelchangeParam5();
            break;

        //--------------------------------------------------------------
        // ISDBT Parameter
        case TVB390_ISDB_T_USE_TMCC:
        case TVB390_ISDB_T_13SEG_USE_TMCC:
            if (lMod != ISDB_T && lMod != ISDB_T_13)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			if(nParam == 0)	
			{
				gBaseUtil.SetCheckInfo(Check_USE_TMCC, FALSE);
				//2012/9/10 new menu 
				this->Modulator_Param1->Checked = false;
			}
			else
			{
				gBaseUtil.SetCheckInfo(Check_USE_TMCC, TRUE);
				//2012/9/10 new menu 
				this->Modulator_Param1->Checked = true;
			}
			OnBnClickedParam();
            break;

        //--------------------------------------------------------------
        // DTMB Parameter
        case TVB390_DTMB_CONSTELLATION:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 4)
                return;

			gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, nParam);
            OnCbnSelchangeParam2();
            break;
            
        case TVB390_DTMB_CODERATE:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 2)
                return;
			if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM_NR || gpConfig->gBC[nBoardNum].gnConstellation ==CONST_DTMB_32QAM)
				return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM3, nParam);
			OnCbnSelchangeParam3();
            break;
            
        case TVB390_DTMB_INTERLEAVE:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM5, nParam);
			OnCbnSelchangeParam5();
            break;

        case TVB390_DTMB_FRAME_HEADER:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 2)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM6, nParam);
			OnCbnSelchangeParam6();
            break;

        case TVB390_DTMB_CARRIER_NUMBER:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM7, nParam);
			OnCbnSelchangeParam7();
            break;

        case TVB390_DTMB_FRAME_HEADER_PN:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

        case TVB390_DTMB_PILOT_INSERTION:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;
			if (gpConfig->gBC[nBoardNum].gnCarrierNumber == CONST_DTMB_CARRIER_NUMBER_1)
				return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM4, nParam);
			OnCbnSelchangeParam4();
            break;

            
        //==============================================================
        //--------------------------------------------------------------
        // TVB590-RF-IF-OUTPUT (9)
        case TVB390_RF:
            dwParam = (unsigned long) nParam;
			double dwFreq;
			if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == RF_OUT_HZ)
				dwFreq = (double)dwParam;
			else if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == RF_OUT_KHZ)
				dwFreq = (double)dwParam / 1000;
			else
				dwFreq = (double)dwParam / 1000000;
            gBaseUtil.SetText_Text(ELabOutputFrequency, dwFreq.ToString());
			OnEnChangeElaboutputfrequency();
            break;

        case TVB390_IF:
            break;
            
        case TVB390_LEVEL:
            break;

        case TVB390_CNR:
            break;

        case TVB390_CNR_MODE:
            break;

        case TVB390_NULL_TP_ON_STOP:
            if (nParam < 0 || nParam > 1)
                return;

            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            gpConfig->gBC[nBoardNum].gnStopMode = nParam;
            gWrapDll.Set_Stop_Mode_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);
            SNMP_Send_Status(TVB390_NULL_TP_ON_STOP);
            break;

        case TVB390_USE_AMP:
            if (nParam < 0 || nParam > 1)
                return;
			nParam = (nParam + 1) % 2;

			if(gpConfig->gBC[nBoardNum].gnBypassAMP == nParam)
			{
				return;
			}
			//2012/8/31 new rf level control
			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 1)
				return;

			gpConfig->gBC[nBoardNum].gnBypassAMP = nParam;

			gWrapDll.TVB380_SET_BOARD_CONFIG_STATUS_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBypassAMP);
    
			//AGC - RF Level -> Atten/AGC
			UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC);
    
			SNMP_Send_Status(TVB390_USE_AMP);
			SNMP_Send_Status(TVB390_LEVEL);            
			SNMP_Send_Status(TVB390_ATTEN);
			SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
			SNMP_Send_Status(TVB390_RF_LEVEL_MAX);
			SNMP_Send_Status(TVB390_ATTEN_MIN);
			SNMP_Send_Status(TVB390_ATTEN_MAX);
            break;

        case TVB390_USE_TAT4720:
            break;
            
        case TVB390_USE_TAT4720_COM:
            break;
		case TVB390_ATTEN:
            gUtility.MySprintf(str, 100, "%d", nParam);
            gBaseUtil.SetText_Numeric(NUMERIC_ATTEN, str);		//2011/5/25 Text_RF_OUTPUT_LEVEL, str);
			OnEnChangeRfOutputLevel();
            break;
		case TVB390_USE_AGC:
			break;
        //==============================================================
        //--------------------------------------------------------------
        // TVB590-IN-OUTPUT-RATE: (4)
        case TVB390_TS:
			if (lMod == TDMB)
                return;
            
			gUtility.MySprintf(str, 100, "%d", nParam);
            gBaseUtil.SetText_Text(ELabPlayRate, str);
			OnEnChangeElabplayrate();
            break;

        case TVB390_TS_MAX:
            if (nParam < 0 || nParam > 1)
                return;
            break;
            
		case TVB390_SYMBOL_RATE:
			nParam = nParam / 1000;
			if(nParam < 1)
				nParam = 1;
            gUtility.MySprintf(str, 100, "%d", nParam);
			gBaseUtil.SetText_Text(ELabOutputSymRate, str);
			OnEnChangeElaboutputsymrate();
            break;
            
        case TVB390_SYMBOL_RATE_DEFAULT:
            if (nParam < 0 || nParam > 1)
                return;
            break;

        //==============================================================
        //--------------------------------------------------------------
        // TVB590-LOOP-ADAPTATION:4
        case TVB390_PCR_PTS_DTS:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gpConfig->gBC[nBoardNum].gnRestamping = nParam;

            SNMP_Send_Status(TVB390_PCR_PTS_DTS);
			gWrapDll.TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;

        case TVB390_CONTINUITY_COUNTER:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnContinuity = nParam;
			SNMP_Send_Status(TVB390_CONTINUITY_COUNTER);
			gWrapDll.TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;

        case TVB390_TOT_TDT:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnDateTimeOffset = nParam;
            SNMP_Send_Status(TVB390_TOT_TDT);
			gWrapDll.TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;

        case TVB390_TOT_TDT_UDT_TIME_SETTING:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 2)
                return;
            if (gpConfig->gBC[nBoardNum].gnDateTimeOffset == 0)
                return;
                
            gpConfig->gBC[nBoardNum].gnDateTimeOffset = nParam+1;
			
            SNMP_Send_Status(TVB390_TOT_TDT_UDT_TIME_SETTING);
			gWrapDll.TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			break;
		case TVB390_PCR_RESTAMP:
			if (lMod == CMMB || lMod == TDMB || lMod == ISDB_T || lMod == ISDB_T_13)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            if (nParam < 0 || nParam > 1)
                return;
			if(gpConfig->gBC[nBoardNum].gnRestamping == 0)
			{
				gpConfig->gBC[nBoardNum].gnRestamping = 1;
			}
			gpConfig->gBC[nBoardNum].gnPCR_Restamping = nParam;

            SNMP_Send_Status(TVB390_PCR_PTS_DTS);
			SNMP_Send_Status(TVB390_PCR_RESTAMP);
			gWrapDll.TSPH_SET_LOOP_ADAPTATION(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;

        //==============================================================
        //--------------------------------------------------------------
        // TVB590-SUBLOOP:4
        case TVB390_USE_SUB_LOOP:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

			if (nParam < 0 || nParam > 2)
                return;
			//Command only
			if(nParam == 2)
			{
				gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 1;
			}
			else
				gpConfig->gBC[nBoardNum].gnUseSubLoop = nParam;
			if(nParam == 0)
				gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 0;

			SetSubloop_Time();
            SNMP_Send_Status(TVB390_USE_SUB_LOOP);
            break;

        case TVB390_START_TIME:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

			if (gpConfig->gBC[nBoardNum].gnUseSubLoop == 0 && gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 0)
                return;
            gpConfig->gBC[nBoardNum].gnStartTimeOffset = nParam;
			SetSubloop_Time();
			SNMP_Send_Status(TVB390_START_TIME);
            break;

        case TVB390_END_TIME:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

			if (gpConfig->gBC[nBoardNum].gnUseSubLoop == 0 && gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 0)
                return;
            gpConfig->gBC[nBoardNum].gnEndTimeOffset = nParam;
            SetSubloop_Time();
			SNMP_Send_Status(TVB390_END_TIME);
            break;

        case TVB390_USE_FIXED_TS_RATE:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

			if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnCalcPlayRate = nParam;
            SNMP_Send_Status(TVB390_USE_FIXED_TS_RATE);
            break;

        //==============================================================
        //--------------------------------------------------------------
        // TVB590-TOOL-IP:(7)
	    case TVB390_USE_IP_STREAMING:
	    case TVB390_IP_OUTPUT_ACCESS:
	    case TVB390_IP_OUTPUT_ADDRESS_URL:
	    case TVB390_IP_OUTPUT_PORT:
	    case TVB390_IP_INPUT_ACCESS:
	    case TVB390_IP_INPUT_ADDRESS_URL:
	    case TVB390_IP_INPUT_PORT:
	    case TVB390_IP_INPUT_MULTICAST:
	    case TVB390_IP_INPUT_LOCAL:
		//2010/3/25
		case TVB390_USE_MULTICAST_IP:

            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

			if (lMod == CMMB || lMod == TDMB)
                return;

			/************************************ OPTION-FORM	*/
            //FormIpStreaming->Show();
            if (msg_type == TVB390_USE_IP_STREAMING)
            {
            }
            else if (msg_type == TVB390_IP_OUTPUT_ACCESS)
            {
            }
            else if (msg_type == TVB390_IP_OUTPUT_ADDRESS_URL)
            {
            }
            else if (msg_type == TVB390_IP_OUTPUT_PORT)
            {
            }
            else if (msg_type == TVB390_IP_INPUT_ACCESS)
            {
            }
            else if (msg_type == TVB390_IP_INPUT_ADDRESS_URL)
            {
				for(int j = 0; j < (int)strlen(msg); j++)
				{
					if(init == 1)
					{
						str[cnt++] = msg[j];
					}
					if(msg[j] == ' ')
					{
						init = 1;
					}
				
				}
				str[cnt] = '\0';
                gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxIP, 20, str);
                SNMP_Send_Status(TVB390_IP_INPUT_ADDRESS_URL);
            }
            else if (msg_type == TVB390_IP_INPUT_PORT)
            {
                gpConfig->gBC[nBoardNum].gnIP_RxPort = nParam;
                SNMP_Send_Status(TVB390_IP_INPUT_PORT);
            }
			else if(msg_type == TVB390_IP_INPUT_MULTICAST)
			{
				for(int j = 0; j < (int)strlen(msg); j++)
				{
					if(init == 1)
					{
						str[cnt++] = msg[j];
					}
					if(msg[j] == ' ')
					{
						init = 1;
					}
				
				}
				str[cnt] = '\0';
                gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, 64, str);
				SNMP_Send_Status(TVB390_IP_INPUT_MULTICAST);
			}
			else if(msg_type == TVB390_IP_INPUT_LOCAL)
			{
			}
			else if(msg_type == TVB390_USE_MULTICAST_IP)
			{
				if (nParam < 0 || nParam > 1)
					return;
				gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP = nParam;
				SNMP_Send_Status(TVB390_USE_MULTICAST_IP);
			}

			/************************************/
            break;
		case TVB390_DVB_T2_BANDWIDTH:
            if (lMod != DVB_T2)
                return;

            if (nParam < 0 || nParam > 3)
                return;

            gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, nParam);
			OnCbnSelchangeParam1();
            break;

		default:
			break;
    }
}
#else
void PlayForm::SNMP_DataArrival(char *strMsg)

{
	 char	 msg[256];
	 char	 str[256];
	 int	 msg_type, i, j;
	 int	 nParam;
	 unsigned long	 dwParam;
	 long	 nBoardNum = gGeneral.gnActiveBoard;
	 long	 lMod = gpConfig->gBC[nBoardNum].gnModulatorMode;
	
	 int init = 0;
	 int cnt = 0;
	 long iValue;
	
	 gUtility.MyStrCpy(msg, 256, strMsg);
	 sscanf(msg, "%d %d", &msg_type, &nParam);
	 //printf("msg_type = %d, nParam = %d\n", msg_type, nParam);
	switch (msg_type)
    {
		 case TVB390_FILE_PATH: 	 // 1.3.6.1.4.1.10187.590.1.3.1
			if (gpConfig->gBC[nBoardNum].bRecordInProgress == true ||
 		       gpConfig->gBC[nBoardNum].bPlayingProgress == true)
			{
		        return;
			}
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(cnt == 117 && str[116] == ':')
				 str[cnt -1] = '\0';
			 OnBnClickedComdir(str);
			 break;
        //=======================================================================
        //-----------------------------------------------------------------------
        // TVB590-RUN (8)
        case TVB390_START_PLAYING:      // 1.3.6.1.4.1.10187.590.1.3.1
            if (nParam == 0 && gpConfig->gBC[nBoardNum].bPlayingProgress == false)  //already stopped
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].bPlayingProgress == true)   // already playing
                return;
            if (nParam == 1)
                OnBnClickedComf1();
            else
				OnClicked_Btn_Stop();
            
			 //2010/6/28	AUTO-PLAY
			 gRegVar.SaveVariables(nBoardNum);
			 SaveStreamListInfo(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
            break;
            
        case TVB390_SET_PLAY_MODE:
            if (nParam == 0 && gpConfig->gBC[nBoardNum].gbRepeatMode == 0)  // already once
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].gbRepeatMode == 1)  // already loop
                return;
            OnBnClickedComf2();
            break;
            
        case TVB390_SET_INPUT_SOURCE:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true ||
                gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                return;     // can't change during playback/recording/TDMB


            if (gpConfig->gBC[nBoardNum].gnBoardId < 45 && gpConfig->gBC[nBoardNum].gnBoardId != 10 && gpConfig->gBC[nBoardNum].gnBoardId != 20 && gpConfig->gBC[nBoardNum].gnBoardId != 0xF && 
				gpConfig->gBC[nBoardNum].gnBoardId != 11 && gpConfig->gBC[nBoardNum].gnBoardId != 0x15 && gpConfig->gBC[nBoardNum].gnBoardId != 0x16 && gpConfig->gBC[nBoardNum].gnBoardId != 12 && gpConfig->gBC[nBoardNum].gnBoardId != 16)	//2013/5/27 TVB599 0xC
                return;
			
			if(nParam < 0 || nParam > 6)
				return;

			if(nParam == FILE_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
				gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
			}
			else if (nParam == DVBASI_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
				gpConfig->gBC[nBoardNum].gnModulatorSource = DVBASI_SRC;
			}
			else if (nParam == SMPTE310M_SRC)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
				gpConfig->gBC[nBoardNum].gnModulatorSource = SMPTE310M_SRC;
			}
			else if(nParam == 5)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 1;
				gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;
				gpConfig->gBC[nBoardNum].gnIPStreamingMode = RECV_IP_STREAM;
			}
			else
			{
				return;
			}
			OnBnClickedComf3();
//#ifdef WIN32
//#else
//#ifndef STANDALONE
//			if(gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC && (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T))
//			{
//				i = gWrapDll.Run_Ts_Parser(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gdwPlayRate);
//			}
//			SetDefaultValue_noTMCC(NULL);
//#endif
//
//#endif
			break;

        case TVB390_ADD_LIST:
            OnBnClickedComf4();
            break;
            
        case TVB390_DELETE_LIST:
            OnBnClickedComf5();
            break;
            
        case TVB390_START_RECORDING:
            if (nParam == 0 && gpConfig->gBC[nBoardNum].bRecordInProgress == false)  // already stopped
                return;
            if (nParam == 1 && gpConfig->gBC[nBoardNum].bRecordInProgress == true)   // already recording
                return;

            if (nParam == 1)
                OnBnClickedComf6();
            else
                //OnBnClickedComf1();
				OnClicked_Btn_Stop();
            break;

        case TVB390_SELECT_LIST:
            if (nParam == 0)    // filelist
			{
                OnLbnSelchangeFilelistbox();
				gpConfig->gBC[nBoardNum].fCurFocus  = FILELISTWINDOW;
			}
			else
			{   
				OnLbnSelchangePlaylist();
				gpConfig->gBC[nBoardNum].fCurFocus  = PLAYLISTWINDOW;
			}
			break;

        case TVB390_MOVE_LIST_INDEX:
            if (nParam == 1)
	 			MoveListUpDown(TPG_KEY_UP);		// move index
			 else if (nParam == 0)					// move index
			 	MoveListUpDown(TPG_KEY_DOWN);
			 //2011/1/4 TMCC
			 else if(nParam >= 10000)
			 {
				 int iIx = nParam-10000;
				 if (iIx < gpConfig->gBC[nBoardNum].nFileListIndexCount)
				 {
					 gpConfig->gBC[nBoardNum].nFileListIndexCur = iIx;
				 }
				 if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
					 SetT2MI();
				 OnLbnSelchangeFilelistbox();		 // JAVAJAVA
			 }
			 //2010/5/27
			 else if( nParam >= 1000)
			 {
				 int iIx = nParam-1000;
				 if (iIx < gpConfig->gBC[nBoardNum].nPlayListIndexCount)
				 	gpConfig->gBC[nBoardNum].nPlayListIndexCur = iIx;
					//gBaseUtil.SetCurrentSel_List(PlayList, iIx);
				 OnLbnSelchangePlaylist();		 // JAVAJAVA
			 }
			 else if (nParam >= 100)
			 {
				 int iIx = nParam-100;
				 if (iIx < gpConfig->gBC[nBoardNum].nFileListIndexCount)
				 	gpConfig->gBC[nBoardNum].nFileListIndexCur = iIx;
			 }
			 
			 if(gpConfig->gBC[nBoardNum].gnModulatorMode != ISDB_S && gpConfig->gBC[nBoardNum].gnModulatorMode != DVB_T2)
			 {
				Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
			 }
			 SNMP_Send_Status(TVB390_FILE_LIST_NAME);
			 SNMP_Send_Status(TVB390_PLAY_LIST_NAME);
			 SNMP_Send_Status(TVB390_FILE_PATH);
			 SNMP_Send_Status(TVB390_FILE_LIST_COUNT);
			 SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
			 SNMP_Send_Status(TVB390_FILE_LIST_INDEX);
			 SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);

			 if (gpConfig->gBC[nBoardNum].gnOutputClockSource)
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gdwPlayRate);
			 else
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gGeneral.gnBitrate);			 
			 SNMP_Send_Status(TVB390_DISK_FREE_SIZE);							 
			 //printf("==== playrate=%d, size=%d\n", gpConfig->gBC[nBoardNum].gdwPlayRate, gGeneral.gnDiskSize);
			 break;
		case TVB390_APP_EXECUTE:
			 //kslee 2010/4/27
			 if(nParam == 2)
			 {
			 	 //--------------------------------------------
			 	 //---- 20100427 Saving Variables
    			 gRegVar.SaveVariables(gGeneral.gnActiveBoard);
				 SaveStreamListInfo(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);
    			 //--------------------------------------------
			 	 return;
			 }
			 else if(nParam == 1)
			 {
				return;
			 }
			 Terminate();
			 exit(0);
			 break;

		 case TVB390_CHECK_LICENSE:
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 SnmpRequestedCheckLN(str);
			 gWrapDll.Get_Enabled_Modulator_Type(nBoardNum);
			 SNMP_Send_Status(TVB390_CHECK_LICENSE);
			break;

		 case TVB390_UPDATE_LICENSE:
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(SnmpRequestedUpdateLN(str) == 1)
			 {
				gWrapDll.Get_Enabled_Modulator_Type(nBoardNum);
				int mode = gpConfig->gBC[nBoardNum].gnModulatorMode;
				if(gpConfig->gBC[nBoardNum].gbEnabledType[mode] == 0)
				{
					for(int iii = 0; iii < MAX_MODULATORMODE; iii++)
		            {
						if(gpConfig->gBC[nBoardNum].gbEnabledType[iii] == 1)
                		{
							gpConfig->gBC[nBoardNum].gnChangeModFlag = 1;
							OnCbnSelchangeModulatorType(iii);
							gpConfig->gBC[nBoardNum].gnChangeModFlag = 0;
                		    break;
                		}
        		    }
				}
				else
				{
					int     iiii, iindex;
	
				    iindex = 0;
					for (iiii = 0; iiii < MAX_MODULATORMODE; iiii++)
                    {
                        if (gpConfig->gBC[nBoardNum].gbEnabledType[iiii] == 1)
                        {
                            gpConfig->gBC[nBoardNum].giTypeComboMod[iindex] = iiii;
                            iindex++;
                        }
                    }
                    gpConfig->gBC[nBoardNum].giNumModulator = iindex;

				}
			 }
			SNMP_Send_Status(TVB390_UPDATE_LICENSE);
		    SNMP_Send_Status(TVB390_BOARD_LN);
		 	 break;
			 
		 //=======================================================================
		 //-----------------------------------------------------------------------
		 // TVB590-MODULATOR (1+1+)
		 case TVB390_SELECT_SLOT:  // 1~10	//kslee 2010/4/8
			nParam = nParam - 1;
			if(nParam < 0 || nParam > MAX_BOARD_COUNT)
				return;
			gpConfig->gnCurSlotIndex = nParam;
			OnCbnSelchangeAdaptor(nParam);
			SNMP_Send_Status(TVB390_RUN_TIME); 
            break;

        case TVB390_MODULATOR_TYPE:
            //--- find corresponding index
			////printf("[dbg-seq] - service a msg at app of agent side : [d'%d:%d]\n", nBoardNum, gpConfig->gBC[nBoardNum].giNumModulator);
            for (i = 0; i < gpConfig->gBC[nBoardNum].giNumModulator; i++)
            {
                if (gpConfig->gBC[nBoardNum].giTypeComboMod[i] == nParam)
                {
					 gpConfig->gBC[nBoardNum].gnChangeModFlag = 1;
					 OnCbnSelchangeModulatorType(nParam);
					 gpConfig->gBC[nBoardNum].gnUseSubLoop = 0;
					 gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 0;
					 SetSubloop_Time();
					 gpConfig->gBC[nBoardNum].gnChangeModFlag = 0;
					 SNMP_Send_Status(TVB390_MODULATOR_TYPE);
					 SNMP_Send_Status(TVB390_USE_SUB_LOOP);
                    return;
                }
            }
            break;

        //-----------------------------------------------
        // DVB-T/H Parameter

        case TVB390_DVB_T_H_BANDWIDTH:
        case TVB390_DVB_H_BANDWIDTH:
            if (lMod != DVB_T && lMod != DVB_H && lMod != MULTIPLE_DVBT)
                return;

            if (nParam < 0 || nParam > 3)
                return;

			if ((lMod == DVB_T || lMod == MULTIPLE_DVBT) && nParam > 2)
				return;

			 OnCbnSelchangeParam1(nParam);
            break;
            
        case TVB390_DVB_T_H_CONSTELLATION:
        case TVB390_DVB_H_CONSTELLATION:
            if (lMod != DVB_T && lMod != DVB_H && lMod != MULTIPLE_DVBT)
                return;

            if (nParam < 0 || nParam > 2)
                return;

			 OnCbnSelchangeParam2(nParam);
            break;

        case TVB390_DVB_H_CODERATE:
        case TVB390_DVB_T_H_CODERATE:
            if (lMod != DVB_T && lMod != DVB_H && lMod != MULTIPLE_DVBT)
                return;

            if (nParam < 0 || nParam > 4)
                return;

			 OnCbnSelchangeParam3(nParam);
            break;

        case TVB390_DVB_H_TXMODE:
        case TVB390_DVB_T_H_TXMODE:
            if (lMod != DVB_T && lMod != DVB_H && lMod != MULTIPLE_DVBT)
                return;

            if (nParam < 0 || nParam > 2)
                return;

			if (lMod == DVB_T && nParam > 1)
				return;

			OnCbnSelchangeParam4(nParam);
            break;

        case TVB390_DVB_H_GUARD_INTERVAL:
        case TVB390_DVB_T_H_GUARD_INTERVAL:
            if (lMod != DVB_T && lMod != DVB_H && lMod != MULTIPLE_DVBT)
                return;

            if (nParam < 0 || nParam > 3)
                return;

			 OnCbnSelchangeParam5(nParam);
            break;

        //-----------------------------------------------
        // QAM-A Parameter
        case TVB390_QAM_A_CONSTELLATION:
            if (lMod != QAM_A)
                return;

            if (nParam < 0 || nParam > 4)
                return;
 
			 OnCbnSelchangeParam1(nParam);
            break;

        //-----------------------------------------------
        // QAM-B Parameter
        case TVB390_QAM_B_CONSTELLATION:
            if (lMod != QAM_B && lMod != MULTIPLE_QAMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam1(nParam);
            break;

        case TVB390_QAM_B_INTERLEAVE:
            if (lMod != QAM_B && lMod != MULTIPLE_QAMB)
                return;

            if (nParam < 0 || nParam > 12)
                return;

			 OnCbnSelchangeParam2(nParam);
            break;

        //-----------------------------------------------
        // QPSK Parameter
        case TVB390_QPSK_CODERATE:
            if (lMod != QPSK)
                return;

            if (nParam < 0 || nParam > 4)
                return;

			 OnCbnSelchangeParam1(nParam);
            break;

        case TVB390_QPSK_RRC_FILTER:
            if (lMod != QPSK || gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam2(nParam);
            break;

        case TVB390_QPSK_SPECTRUM:
            if (lMod != QPSK)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam3(nParam);
            break;

        //--------------------------------------------------------------
        // DVB-H Parameter
        case TVB390_DVB_H_MPE_FEC:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnMPE_FEC = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_MPE_FEC);
            break;

        case TVB390_DVB_H_TIME_SLICE:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnTime_Slice = nParam;
            gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);     // Set DVB-H Parameter
            SNMP_Send_Status(TVB390_DVB_H_TIME_SLICE);
            break;

        case TVB390_DVB_H_IN_DEPTH_INTERLEAVE:
            if (lMod != DVB_H)
                return;

            if (nParam < 0 || nParam > 1)
                return;
			i = gpConfig->gBC[nBoardNum].gnTxmode;
			 if (i == TX_8K)
				 nParam = 1;
			 gpConfig->gBC[nBoardNum].gnIn_Depth = nParam;
			 gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);	 // Set DVB-H Parameter
			 SNMP_Send_Status(TVB390_DVB_H_IN_DEPTH_INTERLEAVE);
			 break;
			 
		 case TVB390_DVB_H_CELL_ID:
			 if (lMod != DVB_H)
				 return;
	
			 gpConfig->gBC[nBoardNum].gnCell_Id = nParam;
			 gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);	 // Set DVB-H Parameter
			 SNMP_Send_Status(TVB390_DVB_H_CELL_ID);
			 break;
	
		 //--------------------------------------------------------------
		 // DVB-S2 Parameter
		 case TVB390_DVB_S2_CONSTELLATION:
			 if (lMod != DVB_S2)
				 return;
	
			 if (nParam < 0 || nParam > 3)
				 return;

			 OnCbnSelchangeParam1(nParam);
            break;

        case TVB390_DVB_S2_CODERATE:
            if (lMod != DVB_S2)
                return;

			 OnCbnSelchangeParam2(nParam);
            break;
            
        case TVB390_DVB_S2_PILOT:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam3(nParam);
            break;

        case TVB390_DVB_S2_ROLL_OFF:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 3)
                return;

			if((gpConfig->gBC[nBoardNum].gnBoardId == 20) && (nParam == 3)) 
				return;
			 OnCbnSelchangeParam4(nParam);
            break;

        case TVB390_DVB_S2_SPECTRUM:
            if (lMod != DVB_S2)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam5(nParam);
			break;

        //--------------------------------------------------------------
        // ISDBT Parameter
        case TVB390_ISDB_T_USE_TMCC:
        case TVB390_ISDB_T_13SEG_USE_TMCC:
            if (lMod != ISDB_T && lMod != ISDB_T_13)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnBnClickedParam(nParam);
            break;

        //--------------------------------------------------------------
        // DTMB Parameter
        case TVB390_DTMB_CONSTELLATION:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 4)
                return;

			 OnCbnSelchangeParam2(nParam);
            break;
            
        case TVB390_DTMB_CODERATE:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 2)
                return;
			if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM_NR || gpConfig->gBC[nBoardNum].gnConstellation ==CONST_DTMB_32QAM)
				return;

			 OnCbnSelchangeParam3(nParam);
            break;
            
        case TVB390_DTMB_INTERLEAVE:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam5(nParam);
            break;

        case TVB390_DTMB_FRAME_HEADER:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 2)
                return;

			 OnCbnSelchangeParam6(nParam);
            break;

        case TVB390_DTMB_CARRIER_NUMBER:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam7(nParam);
            break;

        case TVB390_DTMB_FRAME_HEADER_PN:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;

			 OnCbnSelchangeParam1(nParam);
            break;

        case TVB390_DTMB_PILOT_INSERTION:
            if (lMod != DTMB)
                return;

            if (nParam < 0 || nParam > 1)
                return;
			if (gpConfig->gBC[nBoardNum].gnCarrierNumber == CONST_DTMB_CARRIER_NUMBER_1)
				return;

			 OnCbnSelchangeParam4(nParam);
            break;

            
        //==============================================================
        //--------------------------------------------------------------
        // TVB590-RF-IF-OUTPUT (9)
        case TVB390_RF:
            dwParam = (unsigned long) nParam * 1000;
			if(gpConfig->gBC[nBoardNum].gnRFOutFreq == dwParam)
				return;
			 OnEnChangeElaboutputfrequency(dwParam);
			 SNMP_Send_Status(TVB390_LEVEL);
			SNMP_Send_Status(TVB390_DAC_I_OFFSET);
			SNMP_Send_Status(TVB390_DAC_Q_OFFSET);
			break;

        case TVB390_IF:
            if (nParam < 0 || nParam > 1)
                return;
			
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20 || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || 
				gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0x1B || gpConfig->gBC[nBoardNum].gnBoardId == 16)	/* 2012/1/31 TVB591S */ //2011/2/15 added 11(TVB597V2)	//2010/10/5 added 0xF 
				return;
			 OnCbnSelchangeIf(nParam);
            break;
            
        case TVB390_LEVEL:
 			if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum))
			{
				if(gpConfig->gBC[nBoardNum].gdRfLevelValue != ((double)nParam / 10.0))
					OnValueChanged_RFLevel(((double)nParam / 10.0));
			}
            break;

        case TVB390_CNR:
            gUtility.MySprintf(str, 256, (char *)"%d", nParam);
			 OnEnChangeCnr(((double) nParam / 10.0));
			break;

        case TVB390_CNR_MODE:
            if (nParam  < 0 || nParam > 4)
                return;

            gpConfig->gBC[nBoardNum].gnPRBSmode = nParam;

            if (gpConfig->gBC[nBoardNum].gnPRBSmode == 0)   // None
            {
                gWrapDll.Set_Modulator_Prbs_Info_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode,
                        gpConfig->gBC[nBoardNum].gnPRBSmode, gpConfig->gBC[nBoardNum].gnPRBSscale);
            } else
            {
                gWrapDll.Set_Modulator_Prbs_Info_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode,
                        gpConfig->gBC[nBoardNum].gnPRBSmode, gpConfig->gBC[nBoardNum].gnPRBSscale);

            }
            SNMP_Send_Status(TVB390_CNR_MODE);
            break;

        case TVB390_NULL_TP_ON_STOP:
            if (nParam < 0 || nParam > 1)
                return;

            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            gpConfig->gBC[nBoardNum].gnStopMode = nParam;
            gWrapDll.Set_Stop_Mode_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnStopMode);
            SNMP_Send_Status(TVB390_NULL_TP_ON_STOP);
            break;

        case TVB390_USE_AMP:
            if (nParam < 0 || nParam > 1)
                return;
			nParam = (nParam + 1) % 2;
			 OnCheckedBYPASSAMP(nParam);
            break;

        case TVB390_USE_TAT4720:
            break;
            
        case TVB390_USE_TAT4720_COM:
            break;
		case TVB390_ATTEN:
			 OnEnChangeRfOutputLevel(((double) nParam / 10.0));
            break;
		case TVB390_USE_AGC:
            if (nParam < 0 || nParam > 1)
                return;
			if(gpConfig->gBC[nBoardNum].gnAGC == nParam)
				return;
			 OnCheckedAGC(nParam);
			break;
		 case TVB390_BERT_TYPE:
			 gpConfig->gBC[nBoardNum].gnBertPacketType = nParam;
			 gWrapDll.Set_Modulator_Bert_Measure_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, nParam, gpConfig->gBC[nBoardNum].gnBert_Pid);
			 //2012/10/4 bert
			 if(gpConfig->gBC[nBoardNum].gnBertPacketType >= TS_HEAD_184_ALL_0 && gpConfig->gBC[nBoardNum].gnBertPacketType <= TS_HEAD_184_PRBS_2_23)
			 {
				 if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
				 {
					 char total_info[16];
					 char a_info[16];
					 gUtility.MySprintf(total_info, 16, "%d,", gpConfig->gBC[nBoardNum].gnBert_Pid);
					 gUtility.MySprintf(a_info, 16, "%d,", gpConfig->gBC[nBoardNum].gnBert_Pid);

					 gWrapDll.Set_Remux_Info(nBoardNum,
						 0, 3, 1, 0, 22551326,
						 13, 3, 4, 0, 22551326,
						 0, 0, 0, 0, 0,
						 0, 0, 0, 0, 0);
					 gWrapDll.Set_Layer_Info(nBoardNum, -1, 0,
						 1, total_info,
						 1, a_info,
						 0, "",
						 0, "");
				 }
			 }
			 else
			 {
				 if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
				 {
					 TSPH_CLEAR_REMUX_INFO(nBoardNum);
				 }
			 }
			 
			 SNMP_Send_Status(TVB390_BERT_TYPE);
			 break;
		//2012/10/5
		 case TVB390_BERT_PID_VALUE:
			 gpConfig->gBC[nBoardNum].gnBert_Pid = nParam;
			 gWrapDll.Set_Modulator_Bert_Measure_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, nParam, gpConfig->gBC[nBoardNum].gnBert_Pid);
			 SNMP_Send_Status(TVB390_BERT_PID_VALUE);
			 break;
        //==============================================================
        //--------------------------------------------------------------
        // TVB590-IN-OUTPUT-RATE: (4)
        case TVB390_TS:
			if (lMod == TDMB)
                return;
            
			gUtility.MySprintf(str, 256, (char *)"%d", nParam);
			 OnEnChangeElabplayrate(nParam);
			 if (gpConfig->gBC[nBoardNum].gnOutputClockSource)
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gdwPlayRate);
			 else
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gGeneral.gnBitrate);			 
			 
			 SNMP_Send_Status(TVB390_DISK_FREE_SIZE);
			 SNMP_Send_Status(TVB390_RUN_TIME);
            break;

        case TVB390_TS_MAX:
            if (nParam < 0 || nParam > 1)
                return;
			 OnBnClickedBurstBitrate(nParam);
			 if (gpConfig->gBC[nBoardNum].gnOutputClockSource)
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gdwPlayRate);
			 else
				 gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gGeneral.gnBitrate);			 
			 SNMP_Send_Status(TVB390_DISK_FREE_SIZE);							 
			break;
            
        case TVB390_SYMBOL_RATE:
			 OnEnChangeElaboutputsymrate(nParam);
            break;
            
        case TVB390_SYMBOL_RATE_DEFAULT:
            if (nParam < 0 || nParam > 1)
                return;
			 OnBnClickedCalcSymbol(nParam);
            break;

        //==============================================================
        //--------------------------------------------------------------
		 // TVB590-LOOP-ADAPTATION:6
        case TVB390_PCR_PTS_DTS:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            if (nParam < 0 || nParam > 1)
                return;

            gpConfig->gBC[nBoardNum].gnRestamping = nParam;

			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);

            SNMP_Send_Status(TVB390_PCR_PTS_DTS);
            break;

        case TVB390_CONTINUITY_COUNTER:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnContinuity = nParam;
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			 SNMP_Send_Status(TVB390_CONTINUITY_COUNTER);
            break;

        case TVB390_TOT_TDT:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 1)
                return;
            gpConfig->gBC[nBoardNum].gnDateTimeOffset = nParam;
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			 SNMP_Send_Status(TVB390_TOT_TDT);

            break;

        case TVB390_TOT_TDT_UDT_TIME_SETTING:
			if (lMod == CMMB || lMod == TDMB)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
            if (nParam < 0 || nParam > 2)
                return;
            if (gpConfig->gBC[nBoardNum].gnDateTimeOffset == 0)
                return;
                
            gpConfig->gBC[nBoardNum].gnDateTimeOffset = nParam+1;
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			 SNMP_Send_Status(TVB390_TOT_TDT_UDT_TIME_SETTING);
			 break;
	
		 case TVB390_RESTAMP_DATE:
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, 20, str);
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			 SNMP_Send_Status(TVB390_RESTAMP_DATE);
			 break;

		 case TVB390_RESTAMP_TIME:
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
			 }
			 str[cnt] = '\0';
			 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time, 20, str);
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
			 SNMP_Send_Status(TVB390_RESTAMP_TIME);

			break;
		//kslee 2010/3/24
		case TVB390_PCR_RESTAMP:
			if (lMod == CMMB || lMod == TDMB || lMod == ISDB_T || lMod == ISDB_T_13)
                return;

			if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;

            if (nParam < 0 || nParam > 1)
                return;
			if(gpConfig->gBC[nBoardNum].gnRestamping == 0)
			{
				gpConfig->gBC[nBoardNum].gnRestamping = 1;
			}
			gpConfig->gBC[nBoardNum].gnPCR_Restamping = nParam;

            SNMP_Send_Status(TVB390_PCR_PTS_DTS);
			SNMP_Send_Status(TVB390_PCR_RESTAMP);
			gWrapDll.Set_Loop_Adaptation(nBoardNum, gpConfig->gBC[nBoardNum].gnRestamping + gpConfig->gBC[nBoardNum].gnPCR_Restamping * 2, gpConfig->gBC[nBoardNum].gnContinuity, gpConfig->gBC[nBoardNum].gnDateTimeOffset,
				gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date, gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);
            break;
        //==============================================================
        //--------------------------------------------------------------
        // TVB590-SUBLOOP:4
        case TVB390_USE_SUB_LOOP:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
			if (nParam < 0 || nParam > 2)
                return;
			//Command only
			if(nParam == 2)
			{
				gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 1;
			}
			else
				gpConfig->gBC[nBoardNum].gnUseSubLoop = nParam;
			if(nParam == 0)
				gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 0;

			SetSubloop_Time();
			 SNMP_Send_Status(TVB390_USE_SUB_LOOP);
			 break;
	
		 case TVB390_START_TIME:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
			 if (gpConfig->gBC[nBoardNum].gnUseSubLoop == 0 && gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 0)
				 return;
			 gpConfig->gBC[nBoardNum].gnStartTimeOffset = nParam;
			 SetSubloop_Time();
			 SNMP_Send_Status(TVB390_START_TIME);
			 break;
	
		 case TVB390_END_TIME:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
			 if (gpConfig->gBC[nBoardNum].gnUseSubLoop == 0 && gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 0)
				 return;
			 gpConfig->gBC[nBoardNum].gnEndTimeOffset = nParam;
			 SetSubloop_Time();
			 SNMP_Send_Status(TVB390_END_TIME);
			 break;
	
		 case TVB390_USE_FIXED_TS_RATE:
            if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
                gpConfig->gBC[nBoardNum].bRecordInProgress == true)
                return;
			 if (nParam < 0 || nParam > 1)
				 return;
			 gpConfig->gBC[nBoardNum].gnCalcPlayRate = nParam;
			 SNMP_Send_Status(TVB390_USE_FIXED_TS_RATE);
			 break;
	
		 //==============================================================
		 //--------------------------------------------------------------
		 // TVB590-TOOL-IP:(7)
		 case TVB390_USE_IP_STREAMING:
		 case TVB390_IP_RXIP:
		 case TVB390_IP_RXMULTICASTIP:
		 case TVB390_IP_USEMULTICAST:
		 case TVB390_IP_INPUT_ADDRESS_URL:
		 case TVB390_IP_INPUT_PORT:
			 if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
				 gpConfig->gBC[nBoardNum].bRecordInProgress == true)
				 return;
	
			 if (msg_type == TVB390_USE_IP_STREAMING)
			 {
				 if (nParam >= 0 && nParam <=1)
				 {
					 gpConfig->gBC[nBoardNum].gnUseIPStreaming = nParam;
					 SNMP_Send_Status(TVB390_USE_IP_STREAMING);
				 }
			 }
			 else if (msg_type == TVB390_IP_RXIP)
			 {
				for(j = 0; j < (int)strlen(msg); j++)
				{
					if(init == 1)
					{
						str[cnt++] = msg[j];
					}
					if(msg[j] == ' ')
					{
						init = 1;
					}
				
				}
				str[cnt] = '\0';
                gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxIP, 20, str);

				 SNMP_Send_Status(TVB390_IP_RXIP);
			 }
			 else if (msg_type == TVB390_IP_RXMULTICASTIP)
			 {
				for(j = 0; j < (int)strlen(msg); j++)
				{
					if(init == 1)
					{
						str[cnt++] = msg[j];
					}
					if(msg[j] == ' ')
					{
						init = 1;
					}
				
				}
				str[cnt] = '\0';
                gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIP_RxMulticatIP, 64, str);
				 SNMP_Send_Status(TVB390_IP_RXMULTICASTIP);
			 }
			 else if (msg_type == TVB390_IP_USEMULTICAST)
			 {
				if (nParam < 0 || nParam > 1)
					return;
				gpConfig->gBC[nBoardNum].gnIP_RxMulticatIP = nParam;
				SNMP_Send_Status(TVB390_IP_USEMULTICAST);
			 }
			 else if (msg_type == TVB390_IP_INPUT_ADDRESS_URL)
			 {
				for(j = 0; j < (int)strlen(msg); j++)
				{
					if(init == 1)
					{
				  		str[cnt++] = msg[j];
					}
					if(msg[j] == ' ')
					{
						init = 1;
					}
				}
				str[cnt] = '\0';
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnIPStreamingAddress, 20, str);
				 SNMP_Send_Status(TVB390_IP_INPUT_ADDRESS_URL);
			 }
			 else if (msg_type == TVB390_IP_INPUT_PORT)
			 {
				gpConfig->gBC[nBoardNum].gnIP_RxPort = nParam;
				SNMP_Send_Status(TVB390_IP_INPUT_PORT);
			 }
			 /************************************/
			 break;


		//==============================================================
		//--------------------------------------------------------------
		// ERROR-INJECTION
		case TVB390_ERROR_USE_PACKET:

			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnErrLost = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_USE_PACKET);
			break;

		case TVB390_ERROR_NUM_PACKET_PACKET:
			gpConfig->gBC[nBoardNum].gnErrLostPacket = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_PACKET);
			break;

		case TVB390_ERROR_USE_BIT:
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnErrBits = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_USE_BIT);
			break;

		case TVB390_ERROR_NUM_PACKET_BIT:
			gpConfig->gBC[nBoardNum].gnErrBitsPacket = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_BIT);
			break;

		case TVB390_ERROR_NUM_PER_PACKET_BIT:
			gpConfig->gBC[nBoardNum].gnErrBitsCount = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_NUM_PER_PACKET_BIT);
			break;

		case TVB390_ERROR_USE_BYTE:
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnErrBytes = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_USE_BYTE);
			break;

		case TVB390_ERROR_NUM_PACKET_BYTE:
			gpConfig->gBC[nBoardNum].gnErrBytesPacket = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_NUM_PACKET_BYTE);
			break;

		case TVB390_ERROR_NUM_PER_PACKET_BYTE:
			gpConfig->gBC[nBoardNum].gnErrBytesCount = nParam;
			gWrapDll.Set_Error_Injection(nBoardNum, gpConfig->gBC[nBoardNum].gnErrLost, gpConfig->gBC[nBoardNum].gnErrLostPacket,
				gpConfig->gBC[nBoardNum].gnErrBits, gpConfig->gBC[nBoardNum].gnErrBitsPacket, gpConfig->gBC[nBoardNum].gnErrBitsCount, 
				gpConfig->gBC[nBoardNum].gnErrBytes, gpConfig->gBC[nBoardNum].gnErrBytesPacket, gpConfig->gBC[nBoardNum].gnErrBytesCount);
			SNMP_Send_Status(TVB390_ERROR_NUM_PER_PACKET_BYTE);
			break;
		case TVB390_FIRMWARE_VERSION:		
			// nothing. read only
			break;

		case TVB390_FIRMWARE_WRITE:
#ifdef STANDALONE

		        gpConfig->gBC[nBoardNum].gnChangeModFlag = 1;

			firmware_update();
#endif			
			break;

		case TVB390_REBOOT:
#ifdef STANDALONE
			RebootSystem(nParam);
#endif
			break;

		case TVB390_IP_ADDRESS:				// set ip address
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			get_address_strings();		// get subnetmask
			sprintf(msg,"/sbin/ifconfig eth0 inet %s netmask %s", str, gGeneral.gnSubnetMask);
			system(msg);
			sprintf(msg,"/sbin/route add default gw %s", gGeneral.gnGateway);
			system(msg);
			break;
			
		case TVB390_SUBNET_MASK:			// set subnet mask
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			get_address_strings();		// get subnetmask
			sprintf(msg,"/sbin/ifconfig eth0 inet %s netmask %s", gGeneral.gnIpAddress, str);
			system(msg);
			sprintf(msg,"/sbin/route add default gw %s", gGeneral.gnGateway);
			system(msg);
			break;

		case TVB390_GATEWAY:				// set gateway
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			sprintf(msg,"/sbin/route add default gw %s", str);
			system(msg);
			break;

		case TVB390_DHCP_ENABLE:
			break;
//------------------
		//added 2010/4/16
		case TVB390_CMMB_MDIF:
			 if (lMod != CMMB)
				 return;
			OnCbnSelchangeParam1(nParam);
			SNMP_Send_Status(TVB390_CMMB_MDIF);
			SNMP_Send_Status(TVB390_CMMB_CONSTELLATION);
			SNMP_Send_Status(TVB390_CMMB_RSCODING);
			SNMP_Send_Status(TVB390_CMMB_BYTECROSSING);
			SNMP_Send_Status(TVB390_CMMB_LDPC);
			SNMP_Send_Status(TVB390_CMMB_SCRAMBLE);
			SNMP_Send_Status(TVB390_CMMB_TIMESLICE);
			SNMP_Send_Status(TVB390_CMMB_MDIF_ITEM);
			break;
		//kslee 2010/4/20
		case TVB390_DVB_T2_BANDWIDTH:
			 if (lMod != DVB_T2)
				 return;
	
			 if (nParam < 0 || nParam > 4)
				 return;
			 OnCbnSelchangeParam1(nParam);
			 break;
//2010/6/3-------------------------------------------------------------------------------------	
		case TVB390_DVB_T2_BW:
			 if (lMod != DVB_T2)
				 return;
			 if (nParam < 0 || nParam > 4)
				 return;
			OnCbnSelchangeParam1(nParam);			 
			SNMP_Send_Status(TVB390_DVB_T2_BANDWIDTH);
			 break;
		case TVB390_DVB_T2_BWT:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_BWT = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_BWT);
			 break;
		case TVB390_DVB_T2_FFT:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_FFT = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_FFT);
			 break;
		case TVB390_DVB_T2_GUARD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_GUARD = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_GUARD);
			 break;
		case TVB390_DVB_T2_L1_MOD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_L1_MOD);
			 break;
		case TVB390_DVB_T2_PILOT_PATTERN:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PILOT_PATTERN);
			 break;
		case TVB390_DVB_T2_NETWORK_ID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_NETWORK_ID);
			 break;
		case TVB390_DVB_T2_SYSTEM_ID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_SYSTEM_ID);
			 break;
		case TVB390_DVB_T2_CELL_ID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_CELL_ID);
			 break;
		case TVB390_DVB_T2_PID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PID = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PID);
			 break;
		case TVB390_DVB_T2_NUM_T2_FRAME:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_NUM_T2_FRAME);
			 break;
		case TVB390_DVB_T2_NUM_DATA_SYMBOL:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_NUM_DATA_SYMBOL);
			 break;
 //2011/3/25 DVB-T2 MULTI-PLP ==============================================================================================================
		case TVB390_DVB_T2_PLP_ID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[0] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID);
			 break;
		case TVB390_DVB_T2_PLP_MOD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD);
			 break;
		case TVB390_DVB_T2_PLP_COD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE);
			 break;
		case TVB390_DVB_T2_PLP_HEM:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK);
			 break;
		case TVB390_DVB_T2_PLP_ROT:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[0] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[0], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH [%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[0]);
			 }
			 else if(strcmp(str, "INIT") == 0)
			 {
				gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[0], 512, "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[0], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[0] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[0] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE);
			 break;

		case TVB390_DVB_T2_PLP_ID_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[1] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_1);
			 break;
		case TVB390_DVB_T2_PLP_MOD_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_1);
			 break;
		case TVB390_DVB_T2_PLP_COD_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_1);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_1);
			 break;
		case TVB390_DVB_T2_PLP_HEM_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_1);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_1);
			 break;
		case TVB390_DVB_T2_PLP_ROT_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[1] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_1);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_1:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[1], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH _1[%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[1]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[1], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_1);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[1] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_1);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[1] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_1);
			 break;

		case TVB390_DVB_T2_PLP_ID_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[2] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_2);
			 break;
		case TVB390_DVB_T2_PLP_MOD_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_2);
			 break;
		case TVB390_DVB_T2_PLP_COD_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_2);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_2);
			 break;
		case TVB390_DVB_T2_PLP_HEM_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_2);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_2);
			 break;
		case TVB390_DVB_T2_PLP_ROT_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[2] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_2);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_2:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[2], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH _2[%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[2]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[2], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_2);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[2] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_2);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[2] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_2);
			 break;

		case TVB390_DVB_T2_PLP_ID_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[3] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_3);
			 break;
		case TVB390_DVB_T2_PLP_MOD_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_3);
			 break;
		case TVB390_DVB_T2_PLP_COD_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_3);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_3);
			 break;
		case TVB390_DVB_T2_PLP_HEM_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_3);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_3);
			 break;
		case TVB390_DVB_T2_PLP_ROT_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[3] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_3);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_3:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[3], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH_3 [%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[3]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[3], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_3);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[3] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_3);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[3] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_3);
			 break;

		case TVB390_DVB_T2_PLP_ID_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[4] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_4);
			 break;
		case TVB390_DVB_T2_PLP_MOD_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_4);
			 break;
		case TVB390_DVB_T2_PLP_COD_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_4);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_4);
			 break;
		case TVB390_DVB_T2_PLP_HEM_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_4);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_4);
			 break;
		case TVB390_DVB_T2_PLP_ROT_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[4] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_4);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_4:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[4], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH_4 [%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[4]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[4], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_4);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[4] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_4);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[4] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_4);
			 break;

		case TVB390_DVB_T2_PLP_ID_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[5] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_5);
			 break;
		case TVB390_DVB_T2_PLP_MOD_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_5);
			 break;
		case TVB390_DVB_T2_PLP_COD_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_5);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_5);
			 break;
		case TVB390_DVB_T2_PLP_HEM_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_5);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_5);
			 break;
		case TVB390_DVB_T2_PLP_ROT_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[5] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_5);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_5:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[5], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH_5 [%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[5]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[5], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_5);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[5] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_5);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[5] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_5);
			 break;

		case TVB390_DVB_T2_PLP_ID_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[6] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_6);
			 break;
		case TVB390_DVB_T2_PLP_MOD_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_6);
			 break;
		case TVB390_DVB_T2_PLP_COD_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_6);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_6);
			 break;
		case TVB390_DVB_T2_PLP_HEM_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_6);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_6);
			 break;
		case TVB390_DVB_T2_PLP_ROT_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[6] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_6);
			 break;
		case TVB390_DVB_T2_PLP_TS_PATH_6:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
				 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[6], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH _6[%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[6]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[6], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_6);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[6] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_6);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[6] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_6);
			 break;

		case TVB390_DVB_T2_PLP_ID_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[7] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_ID_7);
			 break;
		case TVB390_DVB_T2_PLP_MOD_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_MOD_7);
			 break;
		case TVB390_DVB_T2_PLP_COD_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_COD_7);
			 break;
		case TVB390_DVB_T2_PLP_FEC_TYPE_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_FEC_TYPE_7);
			 break;
		case TVB390_DVB_T2_PLP_HEM_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_HEM_7);
			 break;
		case TVB390_DVB_T2_PLP_NUM_BLOCK_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_7);
			 break;
		case TVB390_DVB_T2_PLP_ROT_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[7] = nParam;
    		 SNMP_Send_Status(TVB390_DVB_T2_PLP_ROT_7);
			 break;
 		case TVB390_DVB_T2_PLP_TS_PATH_7:
			 if (lMod != DVB_T2 && lMod != ISDB_S)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[7], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_T2_PLP_TS_PATH_7 [%s]\n", gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[7]);
			 }
			 else
				 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[7], 512, str);
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_TS_PATH_7);
			 break;
		case TVB390_DVB_T2_PLP_FILE_BITRATE_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[7] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_FILE_BITRATE_7);
			 break;
		case TVB390_DVB_T2_PLP_PLP_BITRATE_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[7] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_PLP_BITRATE_7);
			 break;

		case TVB390_DVB_T2_SET_T2MI:
			 if (lMod != DVB_T2)
				 return;
			 if(nParam == 2)
			 {
				 SetSnmpFuncCall_AsiIn(nParam);
				 return;
			 }
			 SetT2MI();
			 SetSnmpFuncCall_AsiIn(nParam);
			 break;
		case TVB390_DVB_T2_SEARCH:
			 if (lMod != DVB_T2)
				 return;
			 OnBtnClicked_Search();
			 break;
		case TVB390_DVB_T2_ISDB_S_DIRECTORY:
			 if (lMod != DVB_T2 && lMod != ISDB_S && lMod != DVB_C2)
				 return;
			 for(j = 0; j < (int)strlen(msg); j++)
			 {
				 if(init == 1)
				 {
				  	 str[cnt++] = msg[j];
				 }
				 if(msg[j] == ' ')
				 {
					 init = 1;
				 }
				
			 }
			 str[cnt] = '\0';
			 gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szDvbt2_Directory, 256, str);
			 SetFileListDvbt2_Isdbs(str);
			 SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_DIRECTORY);
			 break;
		case TVB390_DVB_T2_ISDB_S_FILE_INDEX:
			 if (lMod != DVB_T2 && lMod != ISDB_S && lMod != DVB_C2)
				 return;

			 GetFileName_Dvbt2_Isdbs(nParam);
			 break;
			


//======================================================================================================================================================
		 //2010/11/19 TVB593 EMERGENCYBROADCASTING
		case TVB390_EMERGENCY_BROADCASTING:
			if(lMod != ISDB_T && lMod != ISDB_T_13 && lMod != ISDB_S)	//2011/4/18 ISDB-S
				return;

			if(nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting = nParam;
			gWrapDll.Set_Tmcc_Remuxer(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
			SNMP_Send_Status(TVB390_EMERGENCY_BROADCASTING);
			break;

		//2011/1/4 TMCC Setting	==================================================================================================================================================================
		case TVB390_TMCC_BROADCAST:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 2)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.broadcast = (char)nParam;
			break;
		case TVB390_TMCC_MODE:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 2)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.mode = (char)nParam;
			break;
		case TVB390_TMCC_GUARD:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 3)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.guard = (char)nParam;
			break;
		case TVB390_TMCC_PARTIAL:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 1)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.partial = nParam;
			break;
		//LAYER A
		case TVB390_TMCC_SEG_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 13)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg = nParam;
			break;
		case TVB390_TMCC_MOD_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 3)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod = nParam;
			break;
		case TVB390_TMCC_COD_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod = nParam;
			break;
		case TVB390_TMCC_TIME_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.time = nParam;
			break;
		case TVB390_TMCC_SEL_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate = nParam;
			break;
		case TVB390_TMCC_COUNT_A:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = nParam;
			break;
		case TVB390_TMCC_LAYER_INFO_A_1:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, (char *)"");
			break;
		case TVB390_TMCC_LAYER_INFO_A_2:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, str);
			break;
		case TVB390_TMCC_LAYER_INFO_A_3:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerA.info, 1024, str);
			break;
		//LAYER B
		case TVB390_TMCC_SEG_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 13)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg = nParam;
			break;
		case TVB390_TMCC_MOD_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 3)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod = nParam;
			break;
		case TVB390_TMCC_COD_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod = nParam;
			break;
		case TVB390_TMCC_TIME_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.time = nParam;
			break;
		case TVB390_TMCC_SEL_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate = nParam;
			break;
		case TVB390_TMCC_COUNT_B:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = nParam;
			break;
		case TVB390_TMCC_LAYER_INFO_B_1:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, (char *)"");
			break;
		case TVB390_TMCC_LAYER_INFO_B_2:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, str);
			break;
		case TVB390_TMCC_LAYER_INFO_B_3:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerB.info, 1024, str);
			break;
		//LAYER C
		case TVB390_TMCC_SEG_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 13)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg = nParam;
			break;
		case TVB390_TMCC_MOD_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 3)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod = nParam;
			break;
		case TVB390_TMCC_COD_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod = nParam;
			break;
		case TVB390_TMCC_TIME_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 4)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.time = nParam;
			break;
		case TVB390_TMCC_SEL_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate = nParam;
			break;
		case TVB390_TMCC_COUNT_C:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = nParam;
			break;
		case TVB390_TMCC_LAYER_INFO_C_1:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, (char *)"");
			//printf("TVB390_TMCC_LAYER_INFO_C_1 %s\n", gpConfig->gBC[nBoardNum].tmccInfo.layerC.info);
			break;
		case TVB390_TMCC_LAYER_INFO_C_2:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, str);
			//printf("TVB390_TMCC_LAYER_INFO_C_2 %s\n", gpConfig->gBC[nBoardNum].tmccInfo.layerC.info);
			break;
		case TVB390_TMCC_LAYER_INFO_C_3:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layerC.info, 1024, str);
			break;
		case TVB390_OTHER_PID_MAP:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 3)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = nParam;
			break;
		case TVB390_MULTI_PID_MAP:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam > 1)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map = nParam;
			break;
		case TVB390_TMCC_TOTAL_LAYER_COUNT:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0)
				return;
			gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count = nParam;
			break;
		case TVB390_TMCC_TOTAL_LAYER_INFO_1:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, (char *)"");
			break;
		case TVB390_TMCC_TOTAL_LAYER_INFO_2:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, str);
			break;
		case TVB390_TMCC_TOTAL_LAYER_INFO_3:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
		 	sscanf(msg, (char *) "%d %s", &msg_type, str);
			gUtility.MyStrCat(gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info, 1024, str);
			break;
		case TVB390_TMCC_PID_INDEX:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 0 || nParam >= gGeneral.gnPIDCount)
				return;
			GetPIDInfoLineOne(nParam);
			break;
		case TVB390_TMCC_SET_COMMAND:
			if(lMod != ISDB_T && lMod != ISDB_T_13)
				return;
			if(nParam < 1 || nParam > 2)
				return;
			if(nParam == 1)
			{	
				Set_TMCC_Remuxing();
				gUtility.MySprintf(gGeneral.gszPIDInfo, sizeof(gGeneral.gszPIDInfo), (char *)"-1:NULL");
				SendTMCCData();
				SNMP_Send_Status(TVB390_TMCC_PID_INFO);
			}
//#ifdef STANDALONE
			else
			{
				//if(gGeneral.gnPIDCount == 0)
				gGeneral.gnPIDCount = 0;
				//SNMP_Send_Status(TVB390_TMCC_PID_COUNT);
				{	
					//printf("START Run_Ts_Parser\n");
					if(gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC)
						gWrapDll.Run_Ts_Parser2(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0, &pTsInfo);

					//gGeneral.gnRunTSparser = 1;
					//printf("END Run_Ts_Parser\n");
					SendTMCCData();
				}
			}
//#endif
			break;
		//========================================================================================================================================================================================
		//2011/4/18 ISDB-S +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		case TVB390_ISDB_S_CONSTELLATION:
			if(lMod != ISDB_S)
				return;
			if(nParam < 0 || nParam >= CONST_ISDBS_MAX)
				return;
			
			OnCbnSelchangeParam1(nParam);
			SNMP_Send_Status(TVB390_ISDB_S_CONSTELLATION);
			break;
		case TVB390_ISDB_S_CODERATE:
			if(lMod != ISDB_S)
				return;
			if(nParam < 0 || nParam >= CONST_ISDBS_CODE_MAX)
				return;
			OnCbnSelchangeParam2(nParam);
			SNMP_Send_Status(TVB390_ISDB_S_CODERATE);
			break;
		case TVB390_ISDB_S_MULTI_TS_ID_0:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_0:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[0], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_0 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[0]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[0], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_0:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_0:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_0: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_0:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_0);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_0:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[0] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_0);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_1:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_1:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[1], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_1 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[1]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[1], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_1:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_1:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_1: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_1:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_1);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_1:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[1] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_1);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_2:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_2:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[2], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_2 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[2]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[2], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_2:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_2:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_2: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_2:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_2);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_2:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[2] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_2);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_3:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_3:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[3], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_3 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[3]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[3], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_3:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_3:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_3: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_3:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_3);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_3:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[3] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_3);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_4:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_4:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[4], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_4 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[4]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[4], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_4:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_4:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_4: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_4:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_4);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_4:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[4] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_4);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_5:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_5:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[5], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_5 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[5]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[5], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_5:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_5:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_5: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_5:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_5);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_5:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[5] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_5);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_6:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_6:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[6], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_6 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[6]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[6], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_6:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_6:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_6: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_6:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_6);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_6:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[6] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_6);
			break;

		case TVB390_ISDB_S_MULTI_TS_ID_7:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_ID_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_ID_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_PATH_7:
			if(lMod != ISDB_S)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gszTS_M[7], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_ISDB_S_MULTI_TS_PATH_7 [%s]\n", gpConfig->gBC[nBoardNum].gszTS_M[7]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszTS_M[7], 512, str);
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_PATH_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_FILERATE_7:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_FILERATE_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_DATARATE_7:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnTS_Selected_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_DATARATE_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_CONSTELLATION_7: 
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnConstellation_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CONSTELLATION_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_CODERATE_7:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnCoderate_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_CODERATE_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_SLOT_7:
			if(lMod != ISDB_S)
				return;
			gpConfig->gBC[nBoardNum].gnSlotCount_M[7] = nParam;
			SNMP_Send_Status(TVB390_ISDB_S_MULTI_TS_SLOT_7);
			break;
		case TVB390_ISDB_S_MULTI_TS_SET_CMD:
			if(lMod != ISDB_S)
				return;
			SetMultiTS_Combiner();
			SNMP_Send_Status(TVB390_RUN_TIME);
			break;
		case TVB390_ISDB_S_TMCC_USE:
			if(lMod != ISDB_S)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;
			OnBnClickedParam(nParam);
			SNMP_Send_Status(TVB390_RUN_TIME);
			SNMP_Send_Status(TVB390_ISDB_S_TMCC_USE);
			break;
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
		//2011/9/9 DVB-C2 ========================================================================================================================================================================
		case TVB390_DVBC2_BANDWIDTH:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 2)
				return;

			OnCbnSelchangeParam1(nParam);

			SNMP_Send_Status(TVB390_DVBC2_BANDWIDTH);
			break;
		case TVB390_DVBC2_GUARDINTERVAL:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			OnCbnSelchangeParam2(nParam);

			SNMP_Send_Status(TVB390_DVBC2_GUARDINTERVAL);
			SNMP_Send_Status(TVB390_DVBC2_NOTCHSTART);
			SNMP_Send_Status(TVB390_DVBC2_NOTCHWIDTH);
			break;
		case TVB390_DVBC2_STARTFREQ:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 16777215)
				return;

			 OnCbnSelchangeParam1(nParam);
			gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq = nParam;
			SetDVBC2_parameters();


			SNMP_Send_Status(TVB390_DVBC2_STARTFREQ);
			break;
		case TVB390_DVBC2_L1TIMODE:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 3)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_L1 = nParam;

			SNMP_Send_Status(TVB390_DVBC2_L1TIMODE);
			break;
		case TVB390_DVBC2_NETID:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 65534)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Network = nParam;

			SNMP_Send_Status(TVB390_DVBC2_NETID);
			break;
		case TVB390_DVBC2_SYSID:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 65534)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_System = nParam;

			SNMP_Send_Status(TVB390_DVBC2_SYSID);
			break;
		case TVB390_DVBC2_RESERVEDTONE:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone = nParam;

			SNMP_Send_Status(TVB390_DVBC2_RESERVEDTONE);
			break;
		case TVB390_DVBC2_NUMNOTCH:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth = nParam;

			SNMP_Send_Status(TVB390_DVBC2_NUMNOTCH);
			break;
		case TVB390_DVBC2_NOTCHSTART:
			if(lMod != DVB_C2)
				return;
			if(gpConfig->gBC[nBoardNum].gnDVB_C2_Guard == 0)
			{
				if (nParam < 0 || nParam > 8191)
					return;
			}
			else
			{
				if (nParam < 0 || nParam > 16382)
					return;
			}

			gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart = nParam;

			SNMP_Send_Status(TVB390_DVBC2_NOTCHSTART);
			break;
		case TVB390_DVBC2_NOTCHWIDTH:
			if(lMod != DVB_C2)
				return;
			if(gpConfig->gBC[nBoardNum].gnDVB_C2_Guard == 0)
			{
				if (nParam < 0 || nParam > 1)
					return;
			}
			else
			{
				if (nParam < 0 || nParam > 3)
					return;
			}

			gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth = nParam;

			SNMP_Send_Status(TVB390_DVBC2_NOTCHWIDTH);
			break;
		case TVB390_DVBC2_DSLICE_TYPE:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type = nParam;

			SNMP_Send_Status(TVB390_DVBC2_DSLICE_TYPE);
			break;
		case TVB390_DVBC2_DSLICE_FEC_H:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader = nParam;

			SNMP_Send_Status(TVB390_DVBC2_DSLICE_FEC_H);
			break;

		case TVB390_DVBC2_PLP_ID_0:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[0] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_0);
			break;


		case TVB390_DVBC2_PLP_MOD_0:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_0);
			break;
		case TVB390_DVBC2_PLP_COD_0:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_0);
			break;
		case TVB390_DVBC2_PLP_FEC_0:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_0);
			break;
		case TVB390_DVBC2_PLP_BLK_0:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_0);
			break;

		case TVB390_DVBC2_PLP_HEM_0:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_0);
			break;

		case TVB390_DVBC2_PLP_TSPATH_0:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[0], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[0]);
			 }
			 else if(strcmp(str, "INIT") == 0)
			 {
				gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[0], 512, "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[0], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_0);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_0:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_0);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_0:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[0] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_0);
			break;

		case TVB390_DVBC2_PLP_ID_1:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[1] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_1);
			break;


		case TVB390_DVBC2_PLP_MOD_1:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_1);
			break;
		case TVB390_DVBC2_PLP_COD_1:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_1);
			break;
		case TVB390_DVBC2_PLP_FEC_1:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_1);
			break;
		case TVB390_DVBC2_PLP_BLK_1:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_1);
			break;

		case TVB390_DVBC2_PLP_HEM_1:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_1);
			break;

		case TVB390_DVBC2_PLP_TSPATH_1:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[1], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_1 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[1]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[1], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_1);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_1:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_1);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_1:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[1] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_1);
			break;

		case TVB390_DVBC2_PLP_ID_2:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[2] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_2);
			break;


		case TVB390_DVBC2_PLP_MOD_2:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_2);
			break;
		case TVB390_DVBC2_PLP_COD_2:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_2);
			break;
		case TVB390_DVBC2_PLP_FEC_2:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_2);
			break;
		case TVB390_DVBC2_PLP_BLK_2:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_2);
			break;

		case TVB390_DVBC2_PLP_HEM_2:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_2);
			break;

		case TVB390_DVBC2_PLP_TSPATH_2:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[2], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_2 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[2]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[2], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_2);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_2:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_2);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_2:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[2] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_2);
			break;

		case TVB390_DVBC2_PLP_ID_3:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[3] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_3);
			break;


		case TVB390_DVBC2_PLP_MOD_3:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_3);
			break;
		case TVB390_DVBC2_PLP_COD_3:
			if(lMod != DVB_C2)

				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_3);
			break;
		case TVB390_DVBC2_PLP_FEC_3:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_3);
			break;
		case TVB390_DVBC2_PLP_BLK_3:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_3);
			break;

		case TVB390_DVBC2_PLP_HEM_3:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_3);
			break;

		case TVB390_DVBC2_PLP_TSPATH_3:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[3], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_3 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[3]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[3], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_3);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_3:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_3);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_3:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[3] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_3);
			break;

		case TVB390_DVBC2_PLP_ID_4:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[4] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_4);
			break;


		case TVB390_DVBC2_PLP_MOD_4:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_4);
			break;
		case TVB390_DVBC2_PLP_COD_4:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_4);
			break;
		case TVB390_DVBC2_PLP_FEC_4:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_4);
			break;
		case TVB390_DVBC2_PLP_BLK_4:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_4);
			break;

		case TVB390_DVBC2_PLP_HEM_4:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_4);
			break;

		case TVB390_DVBC2_PLP_TSPATH_4:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[4], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_4 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[4]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[4], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_4);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_4:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_4);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_4:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[4] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_4);
			break;

		case TVB390_DVBC2_PLP_ID_5:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[5] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_5);
			break;


		case TVB390_DVBC2_PLP_MOD_5:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_5);
			break;
		case TVB390_DVBC2_PLP_COD_5:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_5);
			break;
		case TVB390_DVBC2_PLP_FEC_5:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_5);
			break;
		case TVB390_DVBC2_PLP_BLK_5:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_5);
			break;

		case TVB390_DVBC2_PLP_HEM_5:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_5);
			break;

		case TVB390_DVBC2_PLP_TSPATH_5:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[5], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_5 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[5]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[5], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_5);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_5:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_5);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_5:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[5] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_5);
			break;

		case TVB390_DVBC2_PLP_ID_6:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[6] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_6);
			break;


		case TVB390_DVBC2_PLP_MOD_6:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_6);
			break;
		case TVB390_DVBC2_PLP_COD_6:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_6);
			break;
		case TVB390_DVBC2_PLP_FEC_6:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_6);
			break;
		case TVB390_DVBC2_PLP_BLK_6:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_6);
			break;

		case TVB390_DVBC2_PLP_HEM_6:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_6);
			break;

		case TVB390_DVBC2_PLP_TSPATH_6:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[6], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_6 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[6]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[6], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_6);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_6:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_6);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_6:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[6] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_6);
			break;

		case TVB390_DVBC2_PLP_ID_7:
			if(lMod != DVB_C2)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[7] = nParam;
			SNMP_Send_Status(TVB390_DVBC2_PLP_ID_7);
			break;


		case TVB390_DVBC2_PLP_MOD_7:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_MOD_7);
			break;
		case TVB390_DVBC2_PLP_COD_7:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 4)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_COD_7);
			break;
		case TVB390_DVBC2_PLP_FEC_7:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FEC_7);
			break;
		case TVB390_DVBC2_PLP_BLK_7:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_BLK_7);
			break;

		case TVB390_DVBC2_PLP_HEM_7:
			if(lMod != DVB_C2)
				return;
	
			if (nParam < 0 || nParam > 1)
				return;

			gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_HEM_7);
			break;

		case TVB390_DVBC2_PLP_TSPATH_7:
			if(lMod != DVB_C2)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			 if(strcmp(str, "SET") == 0)
			 {
				 gUtility.MySprintf(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[7], 512, "%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szRemux_FileName);
				 //printf("====TVB390_DVB_C2_PLP_TS_PATH_7 [%s]\n", gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[7]);
			 }
			 else
				gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[7], 512, str);
			SNMP_Send_Status(TVB390_DVBC2_PLP_TSPATH_7);
			break;
		case TVB390_DVBC2_PLP_FILEBITRATE_7:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_FILEBITRATE_7);
			break;

		case TVB390_DVBC2_PLP_PLPBITRATE_7:
			if(lMod != DVB_C2)
				return;
	
			gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[7] = nParam;

			SNMP_Send_Status(TVB390_DVBC2_PLP_PLPBITRATE_7);
			break;

		case TVB390_DVBC2_SET_CMD:
			if(lMod != DVB_C2)
				return;
			//printf("======TVB390_DVBC2_SET_CMD==========\n");
			SetDVBC2_parameters();
			break;
		//========================================================================================================================================================================================
		//2011/9/26 DVB-T2 IP =================================================================================================================================
		case TVB390_DVBT2_IP_PLP_MOD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_MOD);
			 break;
		case TVB390_DVBT2_IP_PLP_COD:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_COD);
			 break;
		case TVB390_DVBT2_IP_PLP_FEC:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_FEC);
			 break;
		case TVB390_DVBT2_IP_PLP_ID:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_ID);
			 break;
		case TVB390_DVBT2_IP_PLP_HEM:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_HEM);
			 break;
		case TVB390_DVBT2_IP_PLP_ROT:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation = nParam;
    		 SNMP_Send_Status(TVB390_DVBT2_IP_PLP_ROT);
			 break;
		//=====================================================================================================================================================
		//2011/10/27 DAC OFFSET ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		case TVB390_DAC_RF_RANGE:
			if(nParam < 0 || nParam > MAX_DAC_FREQ_RANGE_V4)
				return;
			gpConfig->gBC[nBoardNum].gnSnmp_RF_Index = nParam;
			gpConfig->gBC[nBoardNum].gnSnmp_DAC_I_Offest = gpConfig->gBC[nBoardNum].gDAC_I_Offset[nParam];
			gpConfig->gBC[nBoardNum].gnSnmp_DAC_Q_Offset = gpConfig->gBC[nBoardNum].gDAC_Q_Offset[nParam];
			//printf("DAC RF INDEX : %d, %d, %d\n", nParam, gpConfig->gBC[nBoardNum].gnSnmp_DAC_I_Offest, gpConfig->gBC[nBoardNum].gnSnmp_DAC_Q_Offset);
			SNMP_Send_Status(TVB390_DAC_I_OFFSET_READ);
			SNMP_Send_Status(TVB390_DAC_Q_OFFSET_READ);
			SNMP_Send_Status(TVB390_DAC_RF_RANGE);
			break;
		case TVB390_DAC_I_OFFSET:
			if(nParam < -50 || nParam > 50)
				return;
			//printf("DAC I OFFSET : %d\n", nParam);
			iValue = gUtilInd.getFreqIndex();
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[iValue] = nParam;
			Set_Dac_I_Q_Offset();
			SNMP_Send_Status(TVB390_DAC_I_OFFSET);
			break;
		case TVB390_DAC_Q_OFFSET:
			if(nParam < -50 || nParam > 50)
				return;
			//printf("DAC Q OFFSET : %d\n", nParam);
			iValue = gUtilInd.getFreqIndex();
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[iValue] = nParam;
			Set_Dac_I_Q_Offset();
			SNMP_Send_Status(TVB390_DAC_Q_OFFSET);
			break;
		case TVB390_DAC_I_OFFSET_READ:
			if(nParam < -50 || nParam > 50)
				return;
			iValue = gpConfig->gBC[nBoardNum].gnSnmp_RF_Index;
			gpConfig->gBC[nBoardNum].gDAC_I_Offset[iValue] = nParam;
			//printf("TVB390_DAC_I_OFFSET_READ : %d, %d, %d\n", nParam, gpConfig->gBC[nBoardNum].gnSnmp_RF_Index, gpConfig->gBC[nBoardNum].gDAC_I_Offset[iValue]);
			SNMP_Send_Status(TVB390_DAC_I_OFFSET_READ);
			break;
		case TVB390_DAC_Q_OFFSET_READ:
			if(nParam < -50 || nParam > 50)
				return;
			iValue = gpConfig->gBC[nBoardNum].gnSnmp_RF_Index;
			gpConfig->gBC[nBoardNum].gDAC_Q_Offset[iValue] = nParam;
			//printf("TVB390_DAC_Q_OFFSET_READ : %d, %d, %d\n", nParam, gpConfig->gBC[nBoardNum].gnSnmp_RF_Index, gpConfig->gBC[nBoardNum].gDAC_Q_Offset[iValue]);
			SNMP_Send_Status(TVB390_DAC_Q_OFFSET_READ);
			break;
		case TVB390_DAC_MODULATION_MODE:
			if(nParam < 0 || nParam > 3)
				return;
			//printf("TVB390_DAC_MODULATION_MODE : %d\n", nParam);
			gpConfig->gBC[nBoardNum].gDAC_Mod_Mode = 0;
			iValue = TSPL_GET_AD9775_EX(gGeneral.gnActiveBoard, 0x1);
			iValue = ((long)(iValue /64) * 64) + (nParam * 16) + 1;
			TSPL_SET_AD9775_EX(nBoardNum, 0x1, iValue);
			SNMP_Send_Status(TVB390_DAC_MODULATION_MODE);
			break;
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		//2011/11/8 I/Q PLAY =====================================================================
		case TVB390_IQPLAY_SPECTRUM:
			if(nParam < 0 || nParam > 1)
				return;
			if (lMod != IQ_PLAY)
				return;
			
			OnCbnSelchangeParam1(nParam);
			break;
		case TVB390_IQPLAY_USE_CAPTURE:
			if(nParam < 0 || nParam > 1)
				return;
			if (lMod != IQ_PLAY)
				return;
			gpConfig->gBC[nBoardNum].gnIQ_capture_mode = nParam;
			gpConfig->gBC[nBoardNum].gnIQ_mode = gpConfig->gBC[nBoardNum].gnIQ_capture_mode;
			TSPH_SET_IQ_MODE(nBoardNum, gpConfig->gBC[nBoardNum].gnIQ_mode, gpConfig->gBC[nBoardNum].gnIQ_mem_use, gpConfig->gBC[nBoardNum].gnIQ_mem_size, gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);
			SNMP_Send_Status(TVB390_IQPLAY_USE_CAPTURE);
			break;
		case TVB390_IQPLAY_MEM_USE:
			if(nParam < 0 || nParam > 1)
				return;
			if (lMod != IQ_PLAY)
				return;
			gpConfig->gBC[nBoardNum].gnIQ_mem_use = nParam;
			TSPH_SET_IQ_MODE(nBoardNum, gpConfig->gBC[nBoardNum].gnIQ_mode, gpConfig->gBC[nBoardNum].gnIQ_mem_use, gpConfig->gBC[nBoardNum].gnIQ_mem_size, gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);
			SNMP_Send_Status(TVB390_IQPLAY_MEM_USE);
			break;
		case TVB390_IQPLAY_MEMORY_SIZE:
			if(nParam < 0 || nParam > 512)
				return;
			if (lMod != IQ_PLAY)
				return;
			gpConfig->gBC[nBoardNum].gnIQ_mem_size = nParam;
			TSPH_SET_IQ_MODE(nBoardNum, gpConfig->gBC[nBoardNum].gnIQ_mode, gpConfig->gBC[nBoardNum].gnIQ_mem_use, gpConfig->gBC[nBoardNum].gnIQ_mem_size, gpConfig->gBC[nBoardNum].gnIQ_CaptureSize);
			SNMP_Send_Status(TVB390_IQPLAY_MEMORY_SIZE);
			break;

		case TVB390_IQPLAY_CAPTURE_FILENAME:
			if (lMod != IQ_PLAY)
				return;
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszIQ_capture_filePath, 512, str);
			SNMP_Send_Status(TVB390_IQPLAY_CAPTURE_FILENAME);
			break;
#ifdef STANDALONE
		case TVB390_SET_SYSTEM_CLOCK:
			char set_cmd[256];
			for(j = 0; j < (int)strlen(msg); j++)
			{
				if(init == 1)
				{
					str[cnt++] = msg[j];
				}
				if(msg[j] == ' ')
				{
					init = 1;
				}
			}
			str[cnt] = '\0';
			sprintf(set_cmd, "date -s \"%s\"", str);
			if(gpConfig->gBC[nBoardNum].bPlayingProgress != true && gpConfig->gBC[nBoardNum].bRecordInProgress != true)
			{
				//printf("SYSTEM TIME : %s\n", str);
				system(set_cmd);
			}
			break;
#endif
		case TVB390_FILE_POS_CHANGE:
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_CURRENT, 0);
			gpConfig->gBC[nBoardNum].dwStartTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum);
			break;

		case TVB390_DVB_T2_PLP_IL:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[0] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL);
			 break;
		case TVB390_DVB_T2_PLP_IL_1:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[1] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_1);
			 break;
		case TVB390_DVB_T2_PLP_IL_2:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[2] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_2);
			 break;
		case TVB390_DVB_T2_PLP_IL_3:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[3] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_3);
			 break;
		case TVB390_DVB_T2_PLP_IL_4:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[4] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_4);
			 break;
		case TVB390_DVB_T2_PLP_IL_5:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[5] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_5);
			 break;
		case TVB390_DVB_T2_PLP_IL_6:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[6] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_6);
			 break;
		case TVB390_DVB_T2_PLP_IL_7:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[7] = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_PLP_IL_7);
			 break;
		case TVB390_DVB_T2_IP_PLP_IL:
			 if (lMod != DVB_T2)
				 return;
			 gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Time_Interleave = nParam;
			 SNMP_Send_Status(TVB390_DVB_T2_IP_PLP_IL);
			 break;
		case TVB59x_TS_OUTPUT_SEL:
printf("TVB59x_TS_OUTPUT_SEL, value: %d\n", nParam);
			 gpConfig->gBC[nBoardNum].gnTsOutput_Mode = nParam;
			 TVB59x_SET_Output_TS_Type_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
			 SNMP_Send_Status(TVB59x_TS_OUTPUT_SEL);
			 break;

		//==========================================================================================
	}
}

#endif

//2011/5//30 DAC FREQ RANGE
void PlayForm::Set_Dac_I_Q_Offset()
{
	long nBoardNum = gGeneral.gnActiveBoard;
	int freqIndex;
	int offset;
	int value;
	int reg1, reg2;
	long nDirection,  nOffsetB9_B2, nOffsetB1_B0;

	if(gpConfig->gBC[nBoardNum].gnBoardId == 11 || (gpConfig->gBC[nBoardNum].gnBoardId == 15 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) ||
		(gpConfig->gBC[nBoardNum].gnBoardId == 20 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x3) || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
	{
		freqIndex = gUtilInd.getFreqIndex();
		if(freqIndex < 0)
		{
			return;
		}
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
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)
		{
			offset = gDacValue_TVB599[freqIndex][2];
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			offset = gDacValue_TVB599[freqIndex][2];
		}
		else
		{
			offset = gDacValue_TVB590S_V_3_x[freqIndex][2];
		}
		value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_i_offset;

		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

#ifdef WIN32 
		value = Math::Abs(value);
#else
		value = abs(value);
		//printf("[Set_Dac_I_Q_Offset] I value set, %d, %d\n", value);
#endif
		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

#ifdef WIN32
		gWrapDll.TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x8, reg1);
		gWrapDll.TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x7, reg2);
#else
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x8, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0x7, reg2);
#endif
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
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 12)
		{
			offset = gDacValue_TVB599[freqIndex][3];
		}
		else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			offset = gDacValue_TVB599[freqIndex][3];
		}
		else
		{
			offset = gDacValue_TVB590S_V_3_x[freqIndex][3];
		}
		value = value + offset + gpConfig->gBC[nBoardNum].gnBoardDac_q_offset;
		
		if(value < 0)
		{
			nDirection = 0x80;
		}
		else
		{
			nDirection = 0x0;
		}

#ifdef WIN32
		value = Math::Abs(value);
#else
		value = abs(value);
		//printf("[Set_Dac_I_Q_Offset] Q value set, %d, %d\n", value);
#endif
		nOffsetB9_B2 = (value & 0x3FC) >> 2;
		nOffsetB1_B0 = value & 0x3;

		reg1 = nDirection | nOffsetB1_B0;
		reg2 = (nOffsetB9_B2 & 0xFF);

#ifdef WIN32
		gWrapDll.TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xC, reg1);
		gWrapDll.TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xB, reg2);
#else
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xC, reg1);
		TSPL_SET_AD9775_EX(gGeneral.gnActiveBoard, 0xB, reg2);
#endif
	}

}
//----------------------------------------------------------------------------------------------------------------
void PlayForm::SetSubloop_Time()
{
    long    nBoardNum = gGeneral.gnActiveBoard;
	char	str[256];
	if(gpConfig->gBC[nBoardNum].gnUseSubLoop == 1 || gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 1)
	{
        long    dwTotalTimeOffset;
        long    dwTimeOffset;
        double  dwOffset;
#ifdef WIN32 
		String^ strTimeS;
		String^ strTimeE;

        //-------------------------------------------------------------------
        // Total time
		//gBaseUtil.GetText_Label(LabFileSize, str);
		dwTotalTimeOffset = gpConfig->gBC[nBoardNum].gnPlaybackTime; //gUtilInd.Get_Sec_From_HHMMSS(str);
#else
		dwTotalTimeOffset = Get_Run_Time_Of_CurrentSet(nBoardNum);
#endif
        //-------------------------------------------------------------------
        // Start Time
		dwTimeOffset = gpConfig->gBC[nBoardNum].gnStartTimeOffset;

        dwOffset = (double)(dwTimeOffset) * (double)(gpConfig->gBC[nBoardNum].gdwPlayRate) /8.0;
        gpConfig->gBC[nBoardNum].gnStartTimeOffset = dwTimeOffset;
        gpConfig->gBC[nBoardNum].gnStartPos = (__int64) dwOffset;

        //-------------------------------------------------------------------
        // End Time
		dwTimeOffset = gpConfig->gBC[nBoardNum].gnEndTimeOffset;
        dwOffset = (double)(dwTimeOffset) * (double)(gpConfig->gBC[nBoardNum].gdwPlayRate) / 8.0;
        gpConfig->gBC[nBoardNum].gnEndTimeOffset = dwTimeOffset;
        gpConfig->gBC[nBoardNum].gnEndPos = (__int64) dwOffset;
        //-------------------------------------------------------------------
        if (dwTotalTimeOffset > gpConfig->gBC[nBoardNum].gnStartTimeOffset &&
            dwTotalTimeOffset >= gpConfig->gBC[nBoardNum].gnEndTimeOffset &&
            gpConfig->gBC[nBoardNum].gnStartTimeOffset < gpConfig->gBC[nBoardNum].gnEndTimeOffset)
        {
            gpConfig->gBC[nBoardNum].gnCurrentPos = gpConfig->gBC[nBoardNum].gnStartPos;
            gpConfig->gBC[nBoardNum].gnStartPosChanged = 1;
            gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 1;

            gWrapDll.Set_Current_Offset(nBoardNum, OFFSET_START, (double) gpConfig->gBC[nBoardNum].gnStartPos);
            gWrapDll.Set_Current_Offset(nBoardNum, OFFSET_CURRENT, (double) gpConfig->gBC[nBoardNum].gnCurrentPos);

            gpConfig->gBC[nBoardNum].gnEndPosChanged = 1;
            gWrapDll.Set_Current_Offset(nBoardNum, OFFSET_END, (double) gpConfig->gBC[nBoardNum].gnEndPos);

#ifdef WIN32
			gUtility.MyStrCpy(str, 256, gUtilInd.szSecTimeToHMSformat((unsigned long)gpConfig->gBC[nBoardNum].gnStartTimeOffset));
			strTimeS = gcnew String(str);
			gUtility.MyStrCpy(str, 256, gUtilInd.szSecTimeToHMSformat((unsigned long)gpConfig->gBC[nBoardNum].gnEndTimeOffset));
			strTimeE = gcnew String(str);

			delete strTimeS;
			delete strTimeE;
#endif
        } 
	}
	else
    {
        gpConfig->gBC[nBoardNum].gnStartSliderPos = 0;
        gpConfig->gBC[nBoardNum].gnStartPosChanged = 0;
        gpConfig->gBC[nBoardNum].gnEndSliderPos = 0;
        gpConfig->gBC[nBoardNum].gnEndPosChanged = 0;

        gWrapDll.Set_Current_Offset(nBoardNum, OFFSET_RELEASE, 0);
    }
}

//---------------------------------------------------------------------------
// called from TimerMain every 100 msec
// - Check ThreadState
#ifdef WIN32 
void PlayForm::HMsecTimerJob()
{
    int     nNewHLDThreadState[MAX_BOARD_COUNT+1];
    double  dwCurRecordedBytes[MAX_BOARD_COUNT+1];
    long    i;
    char    text[100];
	long    nBoardNum = gGeneral.gnActiveBoard;

    for (i = 0; i <= MAX_BOARD_COUNT; i++)
    {
        if (gpConfig->gBC[i].gnBoardStatus == 1)    // board exist
        {
            //-----------------------------------
            // If In Playing
            if (gpConfig->gBC[i].bPlayingProgress == 1)
            {
                // Get the current thread status
                nNewHLDThreadState[i] = gWrapDll.Get_Current_Thread_State(i);

				gpConfig->gBC[i].gnRepeatCount = gWrapDll.TSPH_GET_LOOP_COUNT(i);

                // Display LabTimer
                // Do Action according to Thread State = 6,7,8
                if (gpConfig->gBC[i].nLastHLDThreadState == 6)      // Wait
                {
                    if (nNewHLDThreadState[i] == 6)                 // Wait --> Wait: Keep waiting... Maybe forever
                    {
                        Display_LabTimer(i, "--:--:--.-");
                    } else if (nNewHLDThreadState[i] == 7)          // Wait --> Start : Start
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);              // Set time variable
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    }
                    else if (nNewHLDThreadState[i] == 8)            // Wait --> Stopped
                    {
                        Display_LabTimer(i, "--:--:--.-");
                        gUtilInd.ResetElapsedtimeCounter(i, 0);
                    } else
                    {
                        gWrapDll.Stop_Playing(i);                            // Waoit --> ?
					 	Display_Main_Title(nBoardNum);
						Display_UI_Group_on_Playing_or_Recording();
                   }
                } else if (gpConfig->gBC[i].nLastHLDThreadState == 7) //Continue to play
                {
                    if (nNewHLDThreadState[i] == 6)                 // Play --> Wait. New Play
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);              // Reset timer
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    } else if (nNewHLDThreadState[i] == 7)          // Play --> Play
                    {
                        Display_ElapsedTime(i);
                    } else if (nNewHLDThreadState[i] == 8)          // Play --> Stop Set the next file name
                    {
                        if (gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
                            gWrapDll.SetFileNameNextPlay(i);
                        else
                        {
							if (gpConfig->gBC[i].nPlayListIndexCur >= (gpConfig->gBC[i].nPlayListIndexCount - 1))
                                gpConfig->gBC[i].nPlayListIndexCur = 0;
                            else
                                gpConfig->gBC[i].nPlayListIndexCur = gpConfig->gBC[i].nPlayListIndexCur + 1;

                            gWrapDll.SetFileNameNextPlay(i);
                        }
                    } else
					{
                        gWrapDll.Stop_Playing(i);
						Display_Main_Title(nBoardNum);
						Display_UI_Group_on_Playing_or_Recording();
					}
                } else if (gpConfig->gBC[i].nLastHLDThreadState == 8)     //Stop
                {
                    if (nNewHLDThreadState[i] == 6)                 // Stop --> Wait'No play anymore
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);     //               'Reset timer
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    } else if (nNewHLDThreadState[i] == 7)             // Stop --> Play              'Continue to play
                    {
                        Display_ElapsedTime(i);
                    } else if  (nNewHLDThreadState[i] == 8)         // Stop --> Stop
                    {
                        Display_ElapsedTime(i);
						//2010/11/12 
						nNewHLDThreadState[i] = 7;
                    } else
					{
                        gWrapDll.Stop_Playing(i);
						Display_Main_Title(nBoardNum);
						Display_UI_Group_on_Playing_or_Recording();
						//2011/10/24 added PAUSE
					}
                } else
                {
                    Display_LabTimer(i, "--:--:--.-");

                    if (gpConfig->gBC[i].gnBoardId >= 45 || gpConfig->gBC[i].gnBoardId ==10 || gpConfig->gBC[i].gnBoardId ==20)
                    {
                        if (nNewHLDThreadState[i] == 2)     //'TH_CONT_MON at HLD
                            nNewHLDThreadState[i] = 6;      //TH_START_PLAY at HLD
                    }
                }

                gpConfig->gBC[i].nLastHLDThreadState = nNewHLDThreadState[i];

                // TVB595V1
                UpdateIPStreamingUI(i);
                
			//----------------------------------------------------------------------------
            //sskim20071012 - IP Streaming
            //----------------------------------------------------------------------------
            }
            else    // not playing
            {
                if (i == gGeneral.gnActiveBoard)
                {
                    Show_Hide_Program_List(false);  // hide and clear
                }
            }

            //-----------------------------------
            // If In Delaying
            if (gpConfig->gBC[i].bDelayinProgress == true)
            {
                gUtility.MyStrCpy(text, 100, gUtilInd.szElapsedTimeinHMSdFormat(i));
                Display_LabTimer(i, text);
            }
                    
            //-----------------------------------
            // If In Recording
            if (gpConfig->gBC[i].bRecordInProgress == true)
            {
                //sskim20080723 - BERT
                //if (i == gGeneral.gnActiveBoard)
                //{
                //    if (gpConfig->gBC[i].gnModulatorMode !=TDMB &&
                //        gpConfig->gBC[i].gnBertPacketType >= TS_HEAD_184_ALL_0)
                //    {
                //        double dwBER;
                //        dwBER = gWrapDll.Get_Modulator_Bert_Result_Ex(i, gpConfig->gBC[i].gnModulatorMode);
                //        if (dwBER == 0.0)
                //        {
                //            Display_LabFileSize("0.00E-8");
                //        }
                //        else
                //        {
                //            gUtility.MySprintf(text, 100, "%.2e",dwBER);
                //            Display_LabFileSize(text);
                //        }
                //    }
                //}

                dwCurRecordedBytes[i] = gWrapDll.Get_Current_Record_Point(i);

                if (i == gGeneral.gnActiveBoard)
                {
                    //if (gpConfig->gBC[i].gnModulatorMode !=TDMB &&
                    //    gpConfig->gBC[i].gnBertPacketType >= TS_HEAD_184_ALL_0)
                    //{
                    //}
                    //else
                    //{
                        UpdateFileSizeDisplayInMB(nBoardNum, dwCurRecordedBytes[i]);
                    
                        //sskim20080725 - BERT
                        if (dwCurRecordedBytes[i] <= 0)
                            gUtilInd.LogMessageInt(TLV_WAITING);
                    //}
                }
                                                        
                if (gpConfig->gBC[i].dwLastRecordedBytes > 0 &&
                    dwCurRecordedBytes[i] == gpConfig->gBC[i].dwLastRecordedBytes)
                {
                    gpConfig->gBC[i].nWaitProgressCnt = gpConfig->gBC[i].nWaitProgressCnt+1;
                    gUtility.MyStrCpy(text,  100, gUtilInd.szElapsedTimeinHMSdFormat(i));
                    Display_LabTimer(i, text);
                } else if (dwCurRecordedBytes[i] > 0)
                {
                    gpConfig->gBC[i].nWaitProgressCnt = 0;
                    gUtility.MyStrCpy(text, 100,  gUtilInd.szElapsedTimeinHMSdFormat(i));
                    Display_LabTimer(i, text);
                } else
                {
                    gUtilInd.ResetElapsedtimeCounter(i, 0);
                }
                                
                gpConfig->gBC[i].dwLastRecordedBytes = (__int64) dwCurRecordedBytes[i];
                
                UpdateIPStreamingUI(i);   

				//2010/7/18 I/Q PLAY/CAPTURE
				if(gpConfig->gBC[i].gnIQ_capture_support == 1 && gpConfig->gBC[i].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
				{
					nNewHLDThreadState[i] = gWrapDll.Get_Current_Thread_State(i);
					if(gpConfig->gBC[i].nLastHLDThreadState == 3)	//TH_START_REC
					{
						if(nNewHLDThreadState[i] == 3)				//TH_START_REC
							;
						else if(nNewHLDThreadState[i] == 4)			//TH_CONT_REC
							;
						else
						{
							gWrapDll.Stop_Recording(i);
							Display_Main_Title(nBoardNum);
							Display_UI_Group_on_Playing_or_Recording();
						}
					}
					else if(gpConfig->gBC[i].nLastHLDThreadState == 4)	//TH_CONT_REC
					{
						if(nNewHLDThreadState[i] == 1)				//TH_START_MON
							;
						else if(nNewHLDThreadState[i] == 2)				//TH_CONT_MON
						{
							gWrapDll.Stop_Recording(i);
							Display_Main_Title(nBoardNum);
							Display_UI_Group_on_Playing_or_Recording();
						}
					}
					gpConfig->gBC[i].nLastHLDThreadState = nNewHLDThreadState[i];
				}
            }
        }
    }

    //-------------------------
    // Display Status Message
    Display_Status_Bar(0, gGeneral.szStatusMessage);
}
#else
void PlayForm::HMsecTimerJob()
{
    int     nNewHLDThreadState[MAX_BOARD_COUNT+1];
    double  dwCurRecordedBytes[MAX_BOARD_COUNT+1];
    long    i;
    char    text[100];
	long    nBoardNum = gGeneral.gnActiveBoard;
	int		iTemp;

//for debug
	static int debug_cnt = 0;

    for (i = 0; i <= MAX_BOARD_COUNT; i++)
    {
        if (gpConfig->gBC[i].gnBoardStatus == 1)    // board exist
        {
            //-----------------------------------
            // If In Playing
            if (gpConfig->gBC[i].bPlayingProgress == 1)
            {
#ifdef STANDALONE
				//TVB597LAN - TEST ???
				if(gpConfig->gBC[i].gnUseIPStreaming == TRUE)
				{
					UpdateIPStreamingUI(i);
					continue;
				}
#endif
                // Get the current thread status
                nNewHLDThreadState[i] = gWrapDll.Get_Current_Thread_State(i);

				//2009/12/10 fixed
				iTemp = TSPH_GET_LOOP_COUNT(i);
				if (gpConfig->gBC[i].gnRepeatCount != iTemp)
				{
					gpConfig->gBC[i].gnRepeatCount = iTemp;		//kslee 2010/3/9
					SNMP_Send_Status(TVB390_LOOP_COUNT);	
				}
                // Display LabTimer
                // Do Action according to Thread State = 6,7,8
                if (gpConfig->gBC[i].nLastHLDThreadState == 6)      // Wait
                {
                    if (nNewHLDThreadState[i] == 7)          // Wait --> Start : Start
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);              // Set time variable
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    }
                    else if (nNewHLDThreadState[i] == 8)            // Wait --> Stopped
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);
                    } else
                    {
                        gWrapDll.Stop_Playing(i);                            // Waoit --> ?
						SNMP_Send_Status(TVB390_START_RECORDING);
						SNMP_Send_Status(TVB390_START_PLAYING);
                   }
                } else if (gpConfig->gBC[i].nLastHLDThreadState == 7) //Continue to play
                {
                    if (nNewHLDThreadState[i] == 6)                 // Play --> Wait. New Play
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);              // Reset timer
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    } else if (nNewHLDThreadState[i] == 7)          // Play --> Play
                    {
						if(debug_cnt == 100)
						{
							TSPL_WRITE_CONTROL_REG_EX(i, 0, (unsigned long)0x59000, (unsigned long)0x52000);
							//printf("[DEBUG +++], 0x52000 : %u\n", (unsigned long)TSPL_READ_CONTROL_REG_EX(i, 0, (unsigned long)0x63000));
						}
						debug_cnt++;
                        Display_ElapsedTime(i);
                    } else if (nNewHLDThreadState[i] == 8)          // Play --> Stop Set the next file name
                    {
                        if (gpConfig->gBC[i].nPlayListIndexCount == 0)
                            gWrapDll.SetFileNameNextPlay(i);
                        else
                        {
							if (gpConfig->gBC[i].nPlayListIndexCur >= (gpConfig->gBC[i].nPlayListIndexCount - 1))
                                gpConfig->gBC[i].nPlayListIndexCur = 0;
                            else
                                gpConfig->gBC[i].nPlayListIndexCur = gpConfig->gBC[i].nPlayListIndexCur + 1;
							
							gWrapDll.SetFileNameNextPlay(i);	
                        }
                    } else
					{
                        gWrapDll.Stop_Playing(i);
						SNMP_Send_Status(TVB390_START_RECORDING);
						SNMP_Send_Status(TVB390_START_PLAYING);
					}
                } else if (gpConfig->gBC[i].nLastHLDThreadState == 8)     //Stop
                {
                    if (nNewHLDThreadState[i] == 6)                 // Stop --> Wait'No play anymore
                    {
                        gUtilInd.ResetElapsedtimeCounter(i, 0);     //               'Reset timer
                        gUtilInd.LogMessageInt(TLV_PLAYING);
                    } else if (nNewHLDThreadState[i] == 7)             // Stop --> Play              'Continue to play
                    {
                        Display_ElapsedTime(i);
                    } else if  (nNewHLDThreadState[i] == 8)         // Stop --> Stop
                    {
						Display_ElapsedTime(i);
						if (gpConfig->gBC[i].nPlayListIndexCount == 0)
                            gWrapDll.SetFileNameNextPlay(i);
                        else
                        {
							if (gpConfig->gBC[i].nPlayListIndexCur >= (gpConfig->gBC[i].nPlayListIndexCount - 1))
                                gpConfig->gBC[i].nPlayListIndexCur = 0;
                            else
                                gpConfig->gBC[i].nPlayListIndexCur = gpConfig->gBC[i].nPlayListIndexCur + 1;
							
							gWrapDll.SetFileNameNextPlay(i);	
                        }
						//2010/11/12 
						nNewHLDThreadState[i] = 7;
                    } else
					{
                        gWrapDll.Stop_Playing(i);
						SNMP_Send_Status(TVB390_START_RECORDING);
						SNMP_Send_Status(TVB390_START_PLAYING);
					}
                } else
                {
                    if (gpConfig->gBC[i].gnBoardId >= 45 || gpConfig->gBC[i].gnBoardId ==10 || gpConfig->gBC[i].gnBoardId ==20)
                    {
                        if (nNewHLDThreadState[i] == 2)     //'TH_CONT_MON at HLD
                            nNewHLDThreadState[i] = 6;      //TH_START_PLAY at HLD
                    }
                }

                gpConfig->gBC[i].nLastHLDThreadState = nNewHLDThreadState[i];

                // TVB595V1
                UpdateIPStreamingUI(i);
            }

            // If In Recording
            if (gpConfig->gBC[i].bRecordInProgress == true)
            {
                dwCurRecordedBytes[i] = gWrapDll.Get_Current_Record_Point(i);
                if (gpConfig->gBC[i].dwLastRecordedBytes > 0 &&
                    dwCurRecordedBytes[i] == gpConfig->gBC[i].dwLastRecordedBytes)
                {
                    gpConfig->gBC[i].nWaitProgressCnt = gpConfig->gBC[i].nWaitProgressCnt+1;
                } else if (dwCurRecordedBytes[i] > 0)
                {
                    gpConfig->gBC[i].nWaitProgressCnt = 0;
                } else
                {
                    gUtilInd.ResetElapsedtimeCounter(i, 0);
                }
                                
                gpConfig->gBC[i].dwLastRecordedBytes = (__int64) dwCurRecordedBytes[i];
				SNMP_Send_Status(TVB390_TS_RECORD_SIZE);
				if (gpConfig->gBC[nBoardNum].gnOutputClockSource)
					gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gdwPlayRate);
				else
					gGeneral.gnDiskSize = gUtility.GetFreeDiskInSecondFormat(gpConfig->gBC[nBoardNum].gszMasterDirectory, gGeneral.gnBitrate);			
				SNMP_Send_Status(TVB390_DISK_FREE_SIZE);               
                UpdateIPStreamingUI(i);                               

				//2010/7/18 I/Q PLAY/CAPTURE
				if(gpConfig->gBC[i].gnIQ_capture_support == 1 && gpConfig->gBC[i].gnIQ_capture_mode == 1 && ((gpConfig->gBC[nBoardNum].gnUseDemuxblockTest & 0x2) > 0))
				{
					nNewHLDThreadState[i] = gWrapDll.Get_Current_Thread_State(i);
					if(gpConfig->gBC[i].nLastHLDThreadState == 3)	//TH_START_REC
					{
						if(nNewHLDThreadState[i] == 3)				//TH_START_REC
							;
						else if(nNewHLDThreadState[i] == 4)			//TH_CONT_REC
							;
						else
						{
							gWrapDll.Stop_Recording(i);
						}
					}
					else if(gpConfig->gBC[i].nLastHLDThreadState == 4)	//TH_CONT_REC
					{
						if(nNewHLDThreadState[i] == 1)				//TH_START_MON
							;
						else if(nNewHLDThreadState[i] == 2)				//TH_CONT_MON
						{
							gWrapDll.Stop_Recording(i);
						}
					}
					gpConfig->gBC[i].nLastHLDThreadState = nNewHLDThreadState[i];
				}
			}
			if( i == gGeneral.gnActiveBoard)
			{
				SNMP_Send_Status(TVB390_TS);
				SNMP_Send_Status(TVB390_RUN_TIME);
			}
        }
    }
}	

#endif

//---------------------------------------------------------------------------
// called from HMsecTimerJob
// Display LabeTimer according to buffer status

#ifdef WIN32 
void PlayForm::Display_ElapsedTime(long nBoardNum)
{
	long    buffer_status[MAX_BOARD_COUNT+1];
    char    str[100];

    buffer_status[nBoardNum] = gWrapDll.Get_Play_Buffer_Status(nBoardNum);
    SNMP_Send_Status(TVB390_ELAPSED_TIME);

    //------------------------------------------------------------------------
    if (buffer_status[nBoardNum] == 0)
    {
        gUtilInd.LogMessageInt(TLV_BUFERING);
        gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
		gpConfig->gBC[nBoardNum].gnIP_TimeCnt = 0;
    } else if (buffer_status[nBoardNum] == 1 || buffer_status[nBoardNum] == 2)
    {
		if(gpConfig->gBC[nBoardNum].gnPause == 1)
		{
			gUtilInd.GetCurrentTimeinMsec(nBoardNum);
			return;
		}
        gUtility.MyStrCpy(str, 100, gUtilInd.szElapsedTimeinHMSdFormat(nBoardNum));
		if(gGeneral.gnActiveBoard == nBoardNum)
		{
			
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && gpConfig->gBC[nBoardNum].gnT2MI_StreamGeneration == 1)
			{
				double dwfilesize = gWrapDll.Get_Current_Record_Point(nBoardNum);
				long file_timelen;
				file_timelen = (unsigned long) (dwfilesize * 8.0 / (double)gpConfig->gBC[nBoardNum].gdwPlayRate * 10.0);
				gUtility.MyStrCpy(str,100, gUtilInd.szMsecTimeToHMSdFormat(file_timelen));
				UpdateFileSizeDisplayInMB(nBoardNum, dwfilesize);
				Display_LabTimer(nBoardNum, str);
			}
			else
				Display_LabTimer(nBoardNum, str);
		}
		if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
		{
			if((gGeneral.gnElapsedTime - gpConfig->gBC[nBoardNum].gnIP_TimeCnt) >= 2)
			{
				gpConfig->gBC[nBoardNum].gnIP_TimeCnt = gGeneral.gnElapsedTime;
				char str_msg[256];
				unsigned int ipdatasize = 0;
				unsigned int lostpacketCnt = 0;
				gWrapDll.TSPH_IP_RECV_STATUS(nBoardNum, &ipdatasize, &lostpacketCnt);
				gUtility.MySprintf(str_msg, 256, "Lost Packet Count : %u,   IP Data Size : %u KBytes", lostpacketCnt, (ipdatasize / 1024));
				
				if(gGeneral.gnActiveBoard == nBoardNum)
					gUtilInd.LogMessage(str_msg);
			}
		}
    } else if (buffer_status[nBoardNum] == 3)
    {
        gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);

        //------------------------------------------------------------------------
        if (gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN)
        {
            if (gpConfig->gBC[nBoardNum].gbRepeatMode == false)
            {
                Display_LabTimer(nBoardNum, "--:--:--.-");
                gWrapDll.Stop_Playing(nBoardNum);
				Display_Main_Title(nBoardNum);
				Display_UI_Group_on_Playing_or_Recording();
            }
        } else
        {
            if (gpConfig->gBC[nBoardNum].nPlayListIndexCur == gpConfig->gBC[nBoardNum].nPlayListIndexStart &&
                gpConfig->gBC[nBoardNum].gbRepeatMode == false)
            {
                Display_LabTimer(nBoardNum, "--:--:--.-");
                gWrapDll.Stop_Playing(nBoardNum);
				Display_Main_Title(nBoardNum);
				Display_UI_Group_on_Playing_or_Recording();
            }
			//2010/9/13
			if(gpConfig->gBC[nBoardNum].nPlayListIndexCount == 1)
				return;
            
			Display_Main_Title(nBoardNum);
            //2013/3/12 TODO
			//if (nBoardNum == gGeneral.gnActiveBoard)
   //         {
   //             SetPlayListIndex(gpConfig->gBC[nBoardNum].nPlayListIndexDisplay);
   //         }
            
            //------------------------------------------------------------------------
			if (gpConfig->gBC[nBoardNum].gnCalcPlayRate == 0)
            {
                long  dwPlayRate;

                if (gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
                {
                    //if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
                    //    dwPlayRate = gUtilInd.CalcBurtBitrate(nBoardNum);
                   // else
                   //     dwPlayRate = gWrapDll.Get_Playrate(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0);
					dwPlayRate = gpConfig->gBC[nBoardNum].gnPlayListPlaybackRate[gpConfig->gBC[nBoardNum].nPlayListIndexCur];
                    if (dwPlayRate < 0)
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            dwPlayRate = 2433331;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                            dwPlayRate = FIXED_PLAY_RATE_ISDB_T;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                            dwPlayRate = 38785316;
                        else
                            dwPlayRate = 19392658;
                    } else
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            dwPlayRate = 2433331;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                            dwPlayRate = 38785316;
                    }
                    if (nBoardNum == gGeneral.gnActiveBoard)
                    {
						if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
						{
							if(gpConfig->gBC[nBoardNum].gnPlayListAtscMh_Format[gpConfig->gBC[nBoardNum].nPlayListIndexCur]  != gpConfig->gBC[nBoardNum].gnOutputClockSource)
							{
								gWrapDll.Set_Burst_Bitrate(nBoardNum, gpConfig->gBC[nBoardNum].gnPlayListAtscMh_Format[gpConfig->gBC[nBoardNum].nPlayListIndexCur]);
								Display_PlayRate(dwPlayRate);
							}
						}
						else
							Display_PlayRate(dwPlayRate);
						gUtilInd.GetFileSizeInSec(gpConfig->gBC[nBoardNum].szCurFileName, nBoardNum);
	                    Display_LabFileSize(gUtilInd.szSecTimeToHMSformat((unsigned long)gpConfig->gBC[nBoardNum].gnPlaybackTime));
						SNMP_Send_Status(TVB390_RUN_TIME);
                    }
                    else
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                            gWrapDll.Set_Play_Rate_Ex(nBoardNum, FIXED_PLAY_RATE_ISDB_T, gpConfig->gBC[nBoardNum].gnOutputClockSource);
                        else
                            gWrapDll.Set_Play_Rate_Ex(nBoardNum, dwPlayRate, gpConfig->gBC[nBoardNum].gnOutputClockSource);

                        gpConfig->gBC[nBoardNum].gdwPlayRate = dwPlayRate;
                    }

                    //----------------------------------------------------------------------------
                    if (gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1 &&
                        (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                         gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13))
                        ;
                    else
                    {
                        double dwStartPos;
                        dwStartPos = gpConfig->gBC[nBoardNum].gnStartTimeOffset * gpConfig->gBC[nBoardNum].gdwPlayRate/8.0;
                        if (dwStartPos != gpConfig->gBC[nBoardNum].gnStartPos)
                            gWrapDll.SetCurrentPlayOffset(nBoardNum, -1, (double) gpConfig->gBC[nBoardNum].dwFileSize);
                    }
                    
                    //----------------------------------------------------------------------------
                    if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
                    {
                        long nBankNumber;
                        long nBankOffset;

                        nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
                        nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;
                        
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                        {
                            ;
                        }
						//2010/7/18
						else if (gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
						{
							;
						}
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            nBankOffset = gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset;
                        else if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
                            nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
						
                        gWrapDll.Set_Sdram_Bank_info(nBoardNum, nBankNumber, nBankOffset);                        
                    }
                    
                }                
            }
			else
			{
                    gUtilInd.GetFileSizeInSec(gpConfig->gBC[nBoardNum].szCurFileName, nBoardNum);
					SNMP_Send_Status(TVB390_RUN_TIME);
			}
        }
    }
}

#else
void PlayForm::Display_ElapsedTime(long nBoardNum)
{
	long    buffer_status[MAX_BOARD_COUNT+1];
    char    str[100];

    buffer_status[nBoardNum] = gWrapDll.Get_Play_Buffer_Status(nBoardNum);
	SNMP_Send_Status(TVB390_ELAPSED_TIME);

    //------------------------------------------------------------------------
    if (buffer_status[nBoardNum] == 0)
    {
        gUtilInd.LogMessageInt(TLV_BUFERING);
        gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
		//2011/8/31 added
		gpConfig->gBC[nBoardNum].gnIP_TimeCnt = 0;
	} else if (buffer_status[nBoardNum] == 1 || buffer_status[nBoardNum] == 2)
    {
		//2011/10/24 added PAUSE
		if(gpConfig->gBC[nBoardNum].gnPause == 1)
		{
			gUtilInd.GetCurrentTimeinMsec(nBoardNum);
			return;
		}
        gUtility.MyStrCpy(str, 100, gUtilInd.szElapsedTimeinHMSdFormat(nBoardNum));
		if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
		{
			if((gGeneral.gnElapsedTime - gpConfig->gBC[nBoardNum].gnIP_TimeCnt) >= 2)
			{
				gpConfig->gBC[nBoardNum].gnIP_TimeCnt = gGeneral.gnElapsedTime;
				char str_msg[256];
				unsigned int ipdatasize = 0;
				unsigned int lostpacketCnt = 0;
				TSPH_IP_RECV_STATUS(nBoardNum, &ipdatasize, &lostpacketCnt);
				gUtility.MySprintf(str_msg, 256, (char *)"Lost Packet Count : %u,   IP Data Size : %u KBytes", lostpacketCnt, (ipdatasize / 1000));

				if(gGeneral.gnActiveBoard == nBoardNum)
					gUtilInd.LogMessage(str_msg);
			}
		}
    } else if (buffer_status[nBoardNum] == 3)
    {
        gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);

        //------------------------------------------------------------------------
        if (gpConfig->gBC[nBoardNum].nPlayListIndexCount == 0)
        {
            if (gpConfig->gBC[nBoardNum].gbRepeatMode == false)
            {
                gWrapDll.Stop_Playing(nBoardNum);
				SNMP_Send_Status(TVB390_START_RECORDING);
				SNMP_Send_Status(TVB390_START_PLAYING);
            }
        } else
        {
            if (gpConfig->gBC[nBoardNum].nPlayListIndexDisplay >= (gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1) )
                gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = 0;
            else
                gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = gpConfig->gBC[nBoardNum].nPlayListIndexDisplay + 1;
                        
            if (gpConfig->gBC[nBoardNum].nPlayListIndexDisplay == gpConfig->gBC[nBoardNum].nPlayListIndexStart &&
                gpConfig->gBC[nBoardNum].gbRepeatMode == false)
            {
                gWrapDll.Stop_Playing(nBoardNum);
				SNMP_Send_Status(TVB390_START_RECORDING);
				SNMP_Send_Status(TVB390_START_PLAYING);
            }
			//2010/9/13
			if(gpConfig->gBC[nBoardNum].nPlayListIndexCount == 1)
				return;
            
			if (nBoardNum == gGeneral.gnActiveBoard)
            {
				gpConfig->gBC[nBoardNum].nPlayListIndexCur = gpConfig->gBC[nBoardNum].nPlayListIndexDisplay;
                UpdatePlayListDisplay();
                gpConfig->gBC[nBoardNum].fCurFocus = PLAYLISTWINDOW;
            }
            
            //------------------------------------------------------------------------
			if (gpConfig->gBC[nBoardNum].gnCalcPlayRate == 0)
            {
                long  dwPlayRate;

                if (gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
                {
                    //if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
                    //    dwPlayRate = gUtilInd.CalcBurtBitrate(nBoardNum);
                    //else
                    //    dwPlayRate = gWrapDll.Get_Playrate(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, 0);
                    dwPlayRate = gpConfig->gBC[nBoardNum].gdwPlayRate;
                    if (dwPlayRate < 0)
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            dwPlayRate = 2433331;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                            dwPlayRate = FIXED_PLAY_RATE_ISDB_T;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                            dwPlayRate = 38785316;
                        else
                            dwPlayRate = 19392658;
                    } else
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            dwPlayRate = 2433331;
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
                            dwPlayRate = 38785316;
                    }
                    
           
                   // gpConfig->gBC[nBoardNum].gdwPlayRate = dwPlayRate;
                    if (nBoardNum == gGeneral.gnActiveBoard)
                    {
						gWrapDll.Set_Play_Rate_Ex(nBoardNum, dwPlayRate, gpConfig->gBC[nBoardNum].gnOutputClockSource);
						gUtilInd.GetFileSizeInSec(gpConfig->gBC[nBoardNum].szCurFileName, nBoardNum);
						SNMP_Send_Status(TVB390_RUN_TIME);
                    }
                    else
                    {
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                            gWrapDll.Set_Play_Rate_Ex(nBoardNum, FIXED_PLAY_RATE_ISDB_T, gpConfig->gBC[nBoardNum].gnOutputClockSource);
                        else
                            gWrapDll.Set_Play_Rate_Ex(nBoardNum, dwPlayRate, gpConfig->gBC[nBoardNum].gnOutputClockSource);
                    }

                    //----------------------------------------------------------------------------
                    if (gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1 &&
                        (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                         gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13))
                        ;
                    else
                    {
                        double dwStartPos;
                        dwStartPos = gpConfig->gBC[nBoardNum].gnStartTimeOffset * gpConfig->gBC[nBoardNum].gdwPlayRate/8.0;
                        if (dwStartPos != gpConfig->gBC[nBoardNum].gnStartPos)
                            gWrapDll.SetCurrentPlayOffset(nBoardNum, -1, (double) gpConfig->gBC[nBoardNum].dwFileSize);
                    }
                    
                    //----------------------------------------------------------------------------
                    if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
                    {
                        long nBankNumber;
                        long nBankOffset;

                        nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
                        nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;
                        
                        if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
                            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
                        {
                            ;
                        }
						//2010/7/18
						else if (gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
						{
							;
						}
                        else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
                            nBankOffset = gpConfig->gBC[nBoardNum].gnTDMBSubBankOffset;
                        else if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
                            nBankOffset = gUtilInd.AdjustBankOffset(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate);
                        
						gWrapDll.Set_Sdram_Bank_info(nBoardNum, nBankNumber, nBankOffset);
						SNMP_Send_Status(TVB390_TS);
						SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
                    }
                    
                }                
            }
			else
			{
                    gUtilInd.GetFileSizeInSec(gpConfig->gBC[nBoardNum].szCurFileName, nBoardNum);
					SNMP_Send_Status(TVB390_RUN_TIME);
			}
        }
    }
}

#endif

//============================================================================
#ifdef WIN32 
void PlayForm::UpdateIPStreamingUI(long nBoardNum)
{
    int     j;
    char    str[100];
    double  dwCurRecordedBytes[MAX_BOARD_COUNT+1];
	int		itop, ileft;

    if (nBoardNum != gGeneral.gnActiveBoard)
        return;
        
    if ( (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 ||  gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1) &&
          gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
			if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode != NO_IP_STREAM)
            {
                // IP Input Bitrate
                long dwBitrate;
                dwBitrate = gWrapDll.Read_Ip_Streaming_Input_Status(nBoardNum, 0);

                if(gBaseUtil.IsVisible_Common(Label_BURST_BITRATE) == false)
				{
					//--- move BURST_BITRATE-->PLAYRATE
					gBaseUtil.GetTopAndLeft(Label_PLAY_RATE, &itop, &ileft);
					gBaseUtil.SetTopAndLeft(Label_BURST_BITRATE, itop, ileft);

					gBaseUtil.GetTopAndLeft(ELabPlayRate, &itop, &ileft);
					gBaseUtil.SetTopAndLeft(Text_IP_BITRATE, itop, ileft);

					gBaseUtil.SetShowContol(Label_BURST_BITRATE, SW_SHOW);
					gBaseUtil.SetShowContol(Text_IP_BITRATE, SW_SHOW);
					gBaseUtil.SetShowContol(Label_PLAY_RATE, SW_HIDE);
					gBaseUtil.SetShowContol(ELabPlayRate, SW_HIDE);
                }

				gUtility.MySprintf(str, 100, "%d",dwBitrate);
				gBaseUtil.SetText_Text(Text_IP_BITRATE, str);

                // Recoded data by IP
                if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
                {
                    dwCurRecordedBytes[nBoardNum] = gWrapDll.Get_Current_Record_Point(nBoardNum);
                    if (dwCurRecordedBytes[nBoardNum] > 0 && nBoardNum == gGeneral.gnActiveBoard)
                        UpdateFileSizeDisplayInMB(nBoardNum, dwCurRecordedBytes[nBoardNum]);
                }
            }

            // Get Program Information
            gpConfig->gBC[nBoardNum].gnIPTotalProgram = gWrapDll.Get_Program(nBoardNum, -1);     // total program count
            if (gpConfig->gBC[nBoardNum].gnIPTotalProgram > 0)
            {
                gpConfig->gBC[nBoardNum].gnIPCurrentProgram = gWrapDll.Get_Program(nBoardNum, -2);  // current program

                for  (j = 0; j < gpConfig->gBC[nBoardNum].gnIPTotalProgram; j++)
                {
                    gpConfig->gBC[nBoardNum].gnIPProgramList[j] = gWrapDll.Get_Program(nBoardNum, j);
                }

 //2013/3/12 delete
    //           if (gpConfig->gBC[nBoardNum].gnIPTotalProgram > 1)
     //           {
     //               gUtility.MySprintf(str, 100, "Program(%d)", gpConfig->gBC[nBoardNum].gnIPCurrentProgram);
					//gBaseUtil.SetText_Label(Label_PROGRAM, str);
     //           }
            }

 //2013/3/12 delete
           // Update Program Information List
    //        if (gpConfig->gBC[nBoardNum].gnIPTotalProgram > 1)
    //        {
    //            
				//Show_Hide_Program_List(true);
				//nCount = gBaseUtil.CountItems_Combo(Combo_PROGRAM);
				//
    //            if (nCount != gpConfig->gBC[nBoardNum].gnIPTotalProgram)
    //                gBaseUtil.ClearItem_Combo(Combo_PROGRAM);

    //            if (nCount == 0)
    //            {
    //                for (j = 0; j < gpConfig->gBC[nBoardNum].gnIPTotalProgram; j++)
    //                {
    //                    gUtility.MySprintf(str, 100, "%d", gpConfig->gBC[nBoardNum].gnIPProgramList[j]);
				//		gBaseUtil.AddItem_Combo(Combo_PROGRAM, str);
    //                    if (gpConfig->gBC[nBoardNum].gnIPCurrentProgram == gpConfig->gBC[nBoardNum].gnIPProgramList[j])
				//			gBaseUtil.SetCurrentSel_Combo(Combo_PROGRAM, j);
    //                }
    //            }
    //        } else
    //        {
    //            Show_Hide_Program_List(false);
    //        }
    } else
    {
            Show_Hide_Program_List(false);

			if(gBaseUtil.IsVisible_Common(Label_BURST_BITRATE) == TRUE)
            {
				gBaseUtil.SetShowContol(Label_BURST_BITRATE, SW_HIDE);
				gBaseUtil.SetShowContol(Text_IP_BITRATE, SW_HIDE);
				gBaseUtil.SetShowContol(Label_PLAY_RATE, SW_SHOW);
				gBaseUtil.SetShowContol(ELabPlayRate, SW_SHOW);
            }
    }
}

#else
void PlayForm::UpdateIPStreamingUI(long nBoardNum)
{
    int     j;
    char    str[100];
    double  dwCurRecordedBytes[MAX_BOARD_COUNT+1];
	int		itop, ileft;
	int		nCount;

    if (nBoardNum != gGeneral.gnActiveBoard)
        return;
        
    if ( (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 ||  gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1) &&
          gpConfig->gBC[nBoardNum].gnModulatorMode != TDMB)
    {
		//IP UDP/RTP
		if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode != NO_IP_STREAM)
        {
            // IP Input Bitrate
            long dwBitrate;
            dwBitrate = gWrapDll.Read_Ip_Streaming_Input_Status(nBoardNum, 0);
			
			gpConfig->gBC[nBoardNum].gnIP_InputRate = dwBitrate;
			SNMP_Send_Status(TVB390_IP_INPUTRATE);

            // Recoded data by IP
            if (gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
            {
                dwCurRecordedBytes[nBoardNum] = gWrapDll.Get_Current_Record_Point(nBoardNum);
            }
        }

        // Get Program Information
        gpConfig->gBC[nBoardNum].gnIPTotalProgram = gWrapDll.Get_Program(nBoardNum, -1);     // total program count
        if (gpConfig->gBC[nBoardNum].gnIPTotalProgram > 0)
        {
            gpConfig->gBC[nBoardNum].gnIPCurrentProgram = gWrapDll.Get_Program(nBoardNum, -2);  // current program

            for  (j = 0; j < gpConfig->gBC[nBoardNum].gnIPTotalProgram; j++)
            {
                gpConfig->gBC[nBoardNum].gnIPProgramList[j] = gWrapDll.Get_Program(nBoardNum, j);
            }
        }
    }
}
#endif

//---------------------------------------------------------------------------
// Called From Parameter changed for setting playrate and symbolrate
// DVBTH : Coderate, Constellation, GuardInterval, Bandwidth
// QAM_A : Constellation
// QAM_B : Constellation
// QPSK  : Coderate
// DVBS2 : Coderate, Constellation, pilot
// DTMB  : Coderate, CarrierNumber, Constellation, FrameHeader, Interleave

#ifdef WIN32 
void PlayForm::Set_Playrate_Symbolrate_On_ParameterChange(long nBoardNum)
{
    long dwRate = gUtilInd.CalcBurtBitrate(nBoardNum);
	Label_Payload_rate->Text = dwRate.ToString();
	
	if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1/* && gBaseUtil.IsEnabled_Common(ELabPlayRate) == TRUE*/)
    {
		Display_PlayRate(dwRate);
    }

	if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A || gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
				gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2 || gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		CheckSymbolRate();
	CheckPlayRate();
}
#else
void PlayForm::Set_Playrate_Symbolrate_On_ParameterChange(long nBoardNum)
{
	//=== if max and stopped, then set gdwplayrate
	if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1 && gpConfig->gBC[nBoardNum].bPlayingProgress == false)
    {
        long dwRate = gUtilInd.CalcBurtBitrate(nBoardNum);
        Display_PlayRate(dwRate);
	}

    if (gpConfig->gBC[nBoardNum].gnModulatorMode != QAM_A &&
		gpConfig->gBC[nBoardNum].gnModulatorMode != QAM_B &&
		gpConfig->gBC[nBoardNum].gnModulatorMode != MULTIPLE_QAMB &&
		gpConfig->gBC[nBoardNum].gnModulatorMode != QPSK &&
		gpConfig->gBC[nBoardNum].gnModulatorMode != DVB_S2)
        return;

	if(gpConfig->gBC[nBoardNum].gnDefaultSymbol == 1 || gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
	{
        long dwRate = gUtilInd.CalcSymbolRate(nBoardNum);
		Display_SymbolRate(dwRate);
		SNMP_Send_Status(TVB390_SYMBOL_RATE);
    }
}

#endif

//----------------------------------------------------------------------------------------------
void PlayForm::Display_PlayRate(long dwRate)
{
#ifdef WIN32
	if(gGeneral.StartInit == 0 && gpConfig->gBC[gGeneral.gnActiveBoard].gnCmdAutoRun != 1/*&& gGeneral.gnRunCommandFlag != 1*/)
	{
		return;
	}
	gBaseUtil.SetText_Text(ELabPlayRate, dwRate);
	gpConfig->gBC[gGeneral.gnActiveBoard].gnPlaybackBitrate = dwRate;

	//if(gBaseUtil.IsVisible_Common(Check_BURST_BITRATE) && gBaseUtil.IsEnabled_Common(Check_BURST_BITRATE) && gBaseUtil.isChecked_Check(Check_BURST_BITRATE) && gBaseUtil.IsEnabled_Common(ELabPlayRate) == false) 
	//{
	//	OnEnChangeElabplayrate();
	//}
#else
	OnEnChangeElabplayrate(dwRate);
#endif
}

//---------------------------------------------------------------------------
void PlayForm::Display_SymbolRate(long dwRate)
{
#ifdef WIN32
//2012/8/24
//	gBaseUtil.SetText_Text(ELabOutputSymRate, dwRate);
	//dwRate = dwRate / 1000;
	if(dwRate < 1)
		dwRate = 1;

	double dwtmp;
	dwtmp = (double)((double)dwRate / 1000.0);
	gBaseUtil.SetText_Text(ELabOutputSymRate, dwtmp);
#else
	OnEnChangeElaboutputsymrate(dwRate);
#endif
}

#ifdef WIN32 
void PlayForm::UpdateMHEpacketInfo(long nBoardNum, char *szFilePath)
{
	long mh_mode;
	long mh_pid;
	mh_pid = gWrapDll.TSPH_GET_MHE_PACKET_INFO(nBoardNum, szFilePath, 0);

	if((mh_pid > 0) && (mh_pid != 0xF))
	{
		mh_mode = 1;
		//2013/3/12 TODO
	}
	else
	{
		mh_mode = 0;
		mh_pid = 0xF;
		//2013/3/12 TODO
	}

	//2010/5/18 ATSC-M/H TPC
	char szResult[8192];
	int nRet;
	gUtility.MySprintf(szResult, sizeof(szResult), "");
	nRet = gWrapDll.TSPH_RUN_ATSC_MH_PARSER(nBoardNum, szFilePath, szResult);
	if(nRet == 1)
	{
		gpConfig->gBC[nBoardNum].gnATSC_MH_Format = 0;
		gWrapDll.Set_Burst_Bitrate(nBoardNum, 0);
	}
	else if(nRet == 0)
	{
		gpConfig->gBC[nBoardNum].gnATSC_MH_Format = 1;
		gWrapDll.Set_Burst_Bitrate(nBoardNum, 1);
	}
}

#else
void PlayForm::UpdateMHEpacketInfo(long nBoardNum, char *szFilePath)
{
	char szRemuxInfo[2100];
	long mh_mode;
	long mh_pid;

	gUtility.MySprintf(szRemuxInfo, sizeof(szRemuxInfo), (char *)"");
	mh_pid = TSPH_GET_MHE_PACKET_INFO(nBoardNum, szFilePath, 0);

	if((mh_pid > 0) && (mh_pid != 0xF))
	{
		mh_mode = 1;
		gUtility.MySprintf(szRemuxInfo, sizeof(szRemuxInfo), (char *)"MHE packet PID : 0x%X\r\n\r\n", mh_pid);
	}
	else
	{
		mh_mode = 0;
		mh_pid = 0xF;
		gUtility.MySprintf(szRemuxInfo, sizeof(szRemuxInfo), (char *)"!!! No found MHE packet information !!!\r\n\r\n");
	}
	//2010/5/18 ATSC-M/H TPC
	char szResult[2048];
	gUtility.MySprintf(szResult, sizeof(szResult), (char *)"");
	TSPH_RUN_ATSC_MH_PARSER(nBoardNum, szFilePath, szResult);
	gUtility.MyStrCat(szRemuxInfo,2100,szResult);
	gUtility.Get_ATSC_MH_PARAMETERS_256(nBoardNum, szRemuxInfo);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER2);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER3);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER4);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER5);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER6);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER7);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER8);
	SNMP_Send_Status(TVB390_ATSC_MH_PARAMETER9);

}

#endif

void PlayForm::UpdateT2MIUI(long nBoardNum, char *szListFileName)
{
#ifdef WIN32 
	gWrapDll.TSPH_RUN_T2MI_PARSER(nBoardNum, szListFileName, &(gpConfig->gBC[nBoardNum].gsT2mi_info));
	//2013/3/12 TODO
#endif

}
#ifdef WIN32 
void PlayForm::OnClicked_Pause()
{
	long nBoardNum = gGeneral.gnActiveBoard;
	
	if(gpConfig->gBC[nBoardNum].gnPause == 0)
	{
		gpConfig->gBC[nBoardNum].gnPause_StartTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum) - 1;
		gWrapDll.TSPH_START_DELAY(gGeneral.gnActiveBoard, 1);
		gpConfig->gBC[nBoardNum].gnPause = 1;
	}
	else if(gpConfig->gBC[nBoardNum].gnPause == 1)
	{
		gpConfig->gBC[nBoardNum].gnPause_EndTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum);
		gpConfig->gBC[nBoardNum].dwStartTime = gpConfig->gBC[nBoardNum].dwStartTime + (gpConfig->gBC[nBoardNum].gnPause_EndTime - gpConfig->gBC[nBoardNum].gnPause_StartTime);
		gWrapDll.TSPH_START_DELAY(gGeneral.gnActiveBoard, 0);
		gpConfig->gBC[nBoardNum].gnPause = 0;
	}
	//2011/12/2 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		SetPause_TVB594(nBoardNum);
	}
}
#else
void PlayForm::OnClicked_Pause()
{
	long nBoardNum = gGeneral.gnActiveBoard;
	
	if(gpConfig->gBC[nBoardNum].gnPause == 0)
	{
		gpConfig->gBC[nBoardNum].gnPause_StartTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum) - 1;
		TSPH_START_DELAY(gGeneral.gnActiveBoard, 1);
		gpConfig->gBC[nBoardNum].gnPause = 1;
	}
	else if(gpConfig->gBC[nBoardNum].gnPause == 1)
	{
		gpConfig->gBC[nBoardNum].gnPause_EndTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum);
		gpConfig->gBC[nBoardNum].dwStartTime = gpConfig->gBC[nBoardNum].dwStartTime + (gpConfig->gBC[nBoardNum].gnPause_EndTime - gpConfig->gBC[nBoardNum].gnPause_StartTime);
		TSPH_START_DELAY(gGeneral.gnActiveBoard, 0);
		gpConfig->gBC[nBoardNum].gnPause = 0;
	}
	//2011/12/2 TVB594
	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
	{
		SetPause_TVB594(nBoardNum);
	}
}

#endif

//2011/12/2 TVB594
void PlayForm::SetPause_TVB594(int nBoardNum)
{
	int regVal = 0;
	int _OwnerSlot = nBoardNum;
	int i;

	if(gpConfig->gBC[nBoardNum].gnPause == 1)
		regVal = (1 << gpConfig->gBC[nBoardNum].gn_StreamNum);

	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		_OwnerSlot = gpConfig->gBC[nBoardNum].gn_OwnerSlot;
		if(gpConfig->gBC[_OwnerSlot].gnRealandvir_cnt > 1)
		{
			for(i = 0 ; i <= MAX_BOARD_COUNT ; i++)
			{
				if(((gpConfig->gBC[_OwnerSlot].gnRealandvir_location >> i) & 0x1) == 1)
				{
					if(i != nBoardNum)
					{
						if(gpConfig->gBC[i].gnPause == 1)
							regVal = regVal | (1 << gpConfig->gBC[i].gn_StreamNum);
					}
				}
			}
		}
		if(gpConfig->gBC[_OwnerSlot].gnPause == 1)
			regVal = regVal | (1 << gpConfig->gBC[_OwnerSlot].gn_StreamNum);
	}
	else
	{
		if(gpConfig->gBC[_OwnerSlot].gnRealandvir_cnt > 0)
		{
			for(i = 0 ; i <= MAX_BOARD_COUNT ; i++)
			{
				if(((gpConfig->gBC[_OwnerSlot].gnRealandvir_location >> i) & 0x1) == 1)
				{
					if(gpConfig->gBC[i].gnPause == 1)
						regVal = regVal | (1 << gpConfig->gBC[i].gn_StreamNum);
				}
			}
		}

	}
#ifdef WIN32 
	gWrapDll.TSPL_WRITE_CONTROL_REG_EX(_OwnerSlot, 0, 0x500102, regVal);
#else
	TSPL_WRITE_CONTROL_REG_EX(_OwnerSlot, 0, 0x500102, regVal);
#endif
}
//2012/2/16 NIT
#ifdef WIN32 
void PlayForm::Get_NIT_Delivery_Descriptor_Info()
{
	int descriptor_flag;
	int freq;
	int modulation;
	int symbolrate;
	int coderate;
	__int64 tmp_val;
	int sym_flag = 0;
	int freq_flag = 0;
	int modul_flag = 0;
	int code_flag = 0;
	int modul_sys;
	int rolloff;
	int nBoardNum = gGeneral.gnActiveBoard;
	String ^str_val;
	String ^str_mod;
	String ^str_msg;
	String ^str_rolloff;
	unsigned long setFreq;
	int i;
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
	{
		if(Combo_PARAM1->Items->Count < 1 || Combo_PARAM2->Items->Count < 1)
			return;
		gWrapDll.TSPH_GET_NIT_SATELLITE_INFO(nBoardNum, &descriptor_flag, &freq, &rolloff, &modul_sys, &modulation, &symbolrate, &coderate);
		if(descriptor_flag == 1)
		{
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			{
				if(modulation == 1 || modulation == 2)
				{
					modul_flag = 1;
					if(modulation == 1)
						str_mod = "QPSK";
					else
						str_mod = "8PSK";
				}

				tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
						  (((symbolrate >> 20) & 0xF) * 10000000) + 
						  (((symbolrate >> 16) & 0xF) * 1000000) + 
						  (((symbolrate >> 12) & 0xF) * 100000) + 
						  (((symbolrate >> 8) & 0xF)  * 10000) + 
						  (((symbolrate >> 4) & 0xF)  * 1000) + 
						  (((symbolrate >> 0) & 0xF)  * 100);
				if((tmp_val % 1000) > 0)
				{
					tmp_val = tmp_val / 1000 + 1;
					tmp_val = tmp_val * 1000;
				}
				if(DVB_S_SYMBOL_RRC_OFF_20_MIN <= tmp_val && tmp_val <= DVB_S_SYMBOL_RRC_OFF_20_MAX)
				{
					sym_flag = 1;
					symbolrate = (int)tmp_val;
				}
				
				//coderate
				if(coderate == 0)
					str_val =  "not defined";
				else if(coderate == 1)
					str_val =  "1/2";
				else if(coderate == 2)
					str_val =  "2/3";
				else if(coderate == 3)
					str_val =  "3/4";
				else if(coderate == 4)
					str_val =  "5/6";
				else if(coderate == 5)
					str_val =  "7/8";
				else if(coderate == 6)
					str_val =  "8/9";
				else if(coderate == 7)
					str_val =  "3/5";
				else if(coderate == 8)
					str_val =  "4/5";
				else if(coderate == 9)
					str_val =  "9/10";
				else
					str_val =  "not defined";

				//Roll Off
				if(rolloff == 0)
					str_rolloff = "0.35";
				else if(rolloff == 1)
					str_rolloff = "0.25";
				else if(rolloff == 2)
					str_rolloff = "0.20";
				else
					str_rolloff = "reserved";

		
				if(modul_flag == 1 && sym_flag == 1)
				{
					str_msg = "From the stream, the following information has been analyed.\r\n" + 
						"Symbol Rate : " + symbolrate.ToString() + " (sps)\r\n" +
						"Modulation : " + str_mod + "\r\n" +
						"Code rate : " + str_val + "\r\n" +
						"Roll-off : " + str_rolloff + "\r\n" +
						"Do you want to apply this information ? ";

					::DialogResult result;
					result = MessageBox::Show(str_msg, "TPG0590VC NIT", MessageBoxButtons::YesNo);
					if(result == ::DialogResult::Yes)
					{
						Display_SymbolRate((long)symbolrate);
			
						gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, (modulation - 1));
						OnCbnSelchangeParam1();
						if(coderate < 1 || coderate > 9)
							gBaseUtil.SetCurrentSel_Combo(Combo_PARAM2, (Combo_PARAM2->Items->Count - 1));
						else
						{
							for(i = 0; i < Combo_PARAM2->Items->Count ; i++)
							{
								if(Combo_PARAM2->Items[i]->ToString()->Equals(str_val))
								{
									Combo_PARAM2->SelectedIndex = i;
									break;
								}
							}
						}
						OnCbnSelchangeParam2();

						if(rolloff >= 0 && rolloff <= 2)
						{
							for(i = 0; i < Combo_PARAM4->Items->Count ; i++)
							{
								if(Combo_PARAM4->Items[i]->ToString()->Equals(str_rolloff))
								{
									Combo_PARAM4->SelectedIndex = i;
									break;
								}
							}
							OnCbnSelchangeParam4();
						}
					}
				}
			}
			else
			{
				if(modulation == 1)
				{
					str_mod = "QPSK";
					modul_flag = 1;
				}
				tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
						  (((symbolrate >> 20) & 0xF) * 10000000) + 
						  (((symbolrate >> 16) & 0xF) * 1000000) + 
						  (((symbolrate >> 12) & 0xF) * 100000) + 
						  (((symbolrate >> 8) & 0xF)  * 10000) + 
						  (((symbolrate >> 4) & 0xF)  * 1000) + 
						  (((symbolrate >> 0) & 0xF)  * 100);
				if((tmp_val % 1000) > 0)
				{
					tmp_val = tmp_val / 1000 + 1;
					tmp_val = tmp_val * 1000;
				}
				if(DVB_S_SYMBOL_RRC_OFF_20_MIN <= tmp_val && tmp_val <= DVB_S_SYMBOL_RRC_OFF_20_MAX)
				{
					sym_flag = 1;
					symbolrate = (int)tmp_val;
				}
					
				//coderate
				if(coderate == 0)
					str_val =  "not defined";
				else if(coderate == 1)
					str_val =  "1/2";
				else if(coderate == 2)
					str_val =  "2/3";
				else if(coderate == 3)
					str_val =  "3/4";
				else if(coderate == 4)
					str_val =  "5/6";
				else if(coderate == 5)
					str_val =  "7/8";
				else
					str_val =  "not defined";

				if(modul_flag == 1 && sym_flag == 1)
				{
					str_msg = "From the stream, the following information has been analyed.\r\n" + 
						"Symbol Rate : " + symbolrate.ToString() + " (sps)\r\n" +
						"Modulation : " + str_mod + "\r\n" +
						"Code Rate : " + str_val + "\r\n" +
						"Do you want to apply this information ? ";
					::DialogResult result;
					result = MessageBox::Show(str_msg, "TPG0590VC NIT", MessageBoxButtons::YesNo);
					if(result == ::DialogResult::Yes)
					{
						Display_SymbolRate((long)symbolrate);
			
						if(coderate < 1 || coderate > 9)
							gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, (Combo_PARAM1->Items->Count - 1));
						else
						{
							for(i = 0; i < Combo_PARAM1->Items->Count ; i++)
							{
								if(Combo_PARAM1->Items[i]->ToString()->Equals(str_val))
								{
									Combo_PARAM1->SelectedIndex = i;
									break;
								}
							}
						}
						OnCbnSelchangeParam1();
					}
				}

			}
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
	{
		if(Combo_PARAM1->Items->Count < 1)
			return;
		gWrapDll.TSPH_GET_NIT_CABLE_INFO(nBoardNum, &descriptor_flag, &freq, &modulation, &symbolrate, &coderate);
		if(descriptor_flag == 1)
		{
			tmp_val = (((freq >> 28) & 0xF) * 1000000000) +
				      (((freq >> 24) & 0xF) * 100000000) +
					  (((freq >> 20) & 0xF) * 10000000) + 
					  (((freq >> 16) & 0xF) * 1000000) + 
					  (((freq >> 12) & 0xF) * 100000) + 
					  (((freq >> 8) & 0xF)  * 10000) + 
					  (((freq >> 4) & 0xF)  * 1000) + 
					  (((freq >> 0) & 0xF)  * 100);
			if(48000000 <= tmp_val && tmp_val <= 2150000000)
			{
				freq_flag = 1;
				setFreq = (unsigned long)tmp_val;
			}

			tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
					  (((symbolrate >> 20) & 0xF) * 10000000) + 
					  (((symbolrate >> 16) & 0xF) * 1000000) + 
					  (((symbolrate >> 12) & 0xF) * 100000) + 
					  (((symbolrate >> 8) & 0xF)  * 10000) + 
					  (((symbolrate >> 4) & 0xF)  * 1000) + 
					  (((symbolrate >> 0) & 0xF)  * 100);
			if((tmp_val % 1000) > 0)
			{
				tmp_val = tmp_val / 1000 + 1;
				tmp_val = tmp_val * 1000;
			}
			if(QAM_A_SYMBOL_MIN <= tmp_val && tmp_val <= QAM_A_SYMBOL_MAX)
			{
				sym_flag = 1;
				symbolrate = (int)tmp_val;
			}			

			if(modulation == 0)
				str_val =  "not defined";
			else if(modulation == 1)
				str_val =  "16QAM";
			else if(modulation == 2)
				str_val =  "32QAM";
			else if(modulation == 3)
				str_val =  "64QAM";
			else if(modulation == 4)
				str_val =  "128QAM";
			else if(modulation == 5)
				str_val =  "256QAM";
			else
				str_val =  "not defined";

			if(freq_flag == 1 && sym_flag == 1)
			{
				str_msg = "From the stream, the following information has been analyed.\r\n" + 
					"Frequency : " + setFreq.ToString() + " (Hz)\r\n" +
					"Symbol Rate : " + symbolrate.ToString() + " (sps)\r\n" +
					"Modulation : " + str_val + "\r\n" +
					"Do you want to apply this information ? ";
				::DialogResult result;
				result = MessageBox::Show(str_msg, "TPG0590VC NIT", MessageBoxButtons::YesNo);
				if(result == ::DialogResult::Yes)
				{
					Display_SymbolRate((long)symbolrate);
					if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == 0)
					{
						ELabOutputFrequency->Text = setFreq.ToString();
					}
					else if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == 1)
					{
						ELabOutputFrequency->Text = ((double)setFreq / 1000.0).ToString();
					}
					else
						ELabOutputFrequency->Text = ((double)setFreq / 1000000.0).ToString();
					if(modulation < 1 || modulation > 5)
						gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, (Combo_PARAM1->Items->Count - 1));
					else
					{
						for(i = 0; i < Combo_PARAM1->Items->Count ; i++)
						{
							if(Combo_PARAM1->Items[i]->ToString()->Equals(str_val))
							{
								Combo_PARAM1->SelectedIndex = i;
								break;
							}
						}
						if(i >= Combo_PARAM1->Items->Count)
							gBaseUtil.SetCurrentSel_Combo(Combo_PARAM1, (Combo_PARAM1->Items->Count - 1));
					}
					OnCbnSelchangeParam1();
				}
			}
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
	{
		if(Combo_PARAM1->Items->Count < 1)
			return;
		unsigned int ui_freq;
		int bw, time_slicing_indepth, mpe_fec, constellation, guard, txmod;
		String ^str_bw, ^str_constellation, ^str_guard, ^str_txmod;
		String ^str_timeSlicing, ^str_mpeFec, ^str_indepthInterleave;

		gWrapDll.TSPH_GET_NIT_TERRESTRIAL_INFO(nBoardNum, &descriptor_flag, &ui_freq, &bw, &time_slicing_indepth, &mpe_fec, 
												&constellation, &coderate, &guard, &txmod);
		if(descriptor_flag == 1)
		{
			tmp_val = ui_freq * 10;
			if((tmp_val % 100000) >= 50000)
			{
				tmp_val = tmp_val / 100000 + 1;
				tmp_val = tmp_val * 100000;
			}
			else
			{
				tmp_val = tmp_val / 100000;
				tmp_val = tmp_val * 100000;
			}
			if(48000000 <= tmp_val && tmp_val <= 2150000000)
			{
				freq_flag = 1;
				setFreq = (unsigned long)tmp_val;
			}
			
			//bandwidth
			if(bw == 0)
				str_bw = "8 MHz";
			else if(bw == 1)
				str_bw = "7 MHz";
			else if(bw == 2)
				str_bw = "6 MHz";
			else if(bw == 3)
				str_bw = "5 MHz";
			else
				str_bw = "reserved";

			//constellation
			if(constellation == 0)
				str_constellation =  "QPSK";
			else if(constellation == 1)
				str_constellation =  "16QAM";
			else if(constellation == 2)
				str_constellation =  "64QAM";
			else
				str_constellation =  "reserved";

			//code rate
			if(coderate == 0)
				str_val = "1/2";
			else if(coderate == 1)
				str_val = "2/3";
			else if(coderate == 2)
				str_val = "3/4";
			else if(coderate == 3)
				str_val = "5/6";
			else if(coderate == 4)
				str_val = "7/8";
			else
				str_val = "reserved";

			//tx mode
			if(txmod == 0)
				str_txmod = "2K";
			else if(txmod == 1)
				str_txmod = "8K";
			else if(txmod == 2)
				str_txmod = "4K";
			else
				str_txmod = "reserved";

			//guard interval
			if(guard == 0)
				str_guard = "1/32";
			else if(guard == 1)
				str_guard = "1/16";
			else if(guard == 2)
				str_guard = "1/8";
			else if(guard == 3)
				str_guard = "1/4";
			else
				str_guard = "Invalid value";

			//MPE FEC
			if(mpe_fec == 0)
			{
				str_mpeFec = "on";
			}
			else
			{
				str_mpeFec = "off";
			}

			//TIME SLICE
			if((time_slicing_indepth & 0x01) == 0)
				str_timeSlicing = "on";
			else
				str_timeSlicing = "off";

			//Indepth Intereave
			if((time_slicing_indepth & 0x02) > 0)
			{
				str_indepthInterleave = "on";
			}
			else
			{
				str_indepthInterleave = "off";
			}

			
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
			{
				str_msg = "From the stream, the following information has been analyed.\r\n" + 
					"RF Frequency : " + setFreq.ToString() + "(Hz)\r\n" +
					"Bandwidth : " + str_bw + "\r\n" +
					"Constellation : " + str_constellation + "\r\n" +
					"Code Rate : " + str_val + "\r\n" +
					"Tx Mode : " + str_txmod + "\r\n" +
					"Guard Interval : " + str_guard + "\r\n" +
					"MPE-FEC : " + str_mpeFec + "\r\n" +
					"TIME SLICE : " + str_timeSlicing + "\r\n" +
					"IN-DEPTH INTERLEAVE : " + str_indepthInterleave + "\r\n" +
					"Do you want to apply this information ? ";
			}
			else
			{
				
				str_msg = "From the stream, the following information has been analyed.\r\n" + 
					"RF Frequency : " + setFreq.ToString() + "(Hz)\r\n" +
					"Bandwidth : " + str_bw + "\r\n" +
					"Constellation : " + str_constellation + "\r\n" +
					"Code Rate : " + str_val + "\r\n" +
					"Tx Mode : " + str_txmod + "\r\n" +
					"Guard Interval : " + str_guard + "\r\n" +
					"Do you want to apply this information ? ";
			}
			::DialogResult result;
			result = MessageBox::Show(str_msg, "TPG0590VC NIT", MessageBoxButtons::YesNo);
			if(result == ::DialogResult::Yes)
			{
				if(freq_flag == 1)
				{
					if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == 0)
					{
						ELabOutputFrequency->Text = setFreq.ToString();
					}
					else if(gpConfig->gBC[nBoardNum].gnRFOutFreqUnit == 1)
					{
						ELabOutputFrequency->Text = ((double)setFreq / 1000.0).ToString();
					}
					else
						ELabOutputFrequency->Text = ((double)setFreq / 1000000.0).ToString();
				}

				//bandwidth
				if(bw >= 0 && bw <= 3)
				{
					for(i = 0; i < Combo_PARAM1->Items->Count ; i++)
					{
						if(Combo_PARAM1->Items[i]->ToString()->Equals(str_bw))
						{
							Combo_PARAM1->SelectedIndex = i;
							OnCbnSelchangeParam1();
							break;
						}
					}
				}

				//constellation
				if(constellation >= 0 && constellation <= 2)
				{
					for(i = 0; i < Combo_PARAM2->Items->Count ; i++)
					{
						if(Combo_PARAM2->Items[i]->ToString()->Equals(str_constellation))
						{
							Combo_PARAM2->SelectedIndex = i;
							OnCbnSelchangeParam2();
							break;
						}
					}
				}

				//coderate
				if(constellation >= 0 && constellation <= 4)
				{
					for(i = 0; i < Combo_PARAM3->Items->Count ; i++)
					{
						if(Combo_PARAM3->Items[i]->ToString()->Equals(str_val))
						{
							Combo_PARAM3->SelectedIndex = i;
							OnCbnSelchangeParam3();
							break;
						}
					}
				}

				//Tx Mode
				if(txmod >= 0 && txmod <= 2)
				{
					for(i = 0; i < Combo_PARAM4->Items->Count ; i++)
					{
						if(Combo_PARAM4->Items[i]->ToString()->Equals(str_txmod))
						{
							Combo_PARAM4->SelectedIndex = i;
							OnCbnSelchangeParam4();
							break;
						}
					}
				}

				//Guard Interval
				if(guard >= 0 && guard <= 3)
				{
					for(i = 0; i < Combo_PARAM5->Items->Count ; i++)
					{
						if(Combo_PARAM5->Items[i]->ToString()->Equals(str_guard))
						{
							Combo_PARAM5->SelectedIndex = i;
							OnCbnSelchangeParam5();
							break;
						}
					}
				}

				if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
				{
					gpConfig->gBC[nBoardNum].gnIn_Depth = ((time_slicing_indepth & 0x02) >> 1);
					gpConfig->gBC[nBoardNum].gnMPE_FEC = (mpe_fec + 1) % 2;
					gpConfig->gBC[nBoardNum].gnTime_Slice = ((time_slicing_indepth & 0x01) + 1) % 2;
					gWrapDll.Set_DVBT_Parameters(nBoardNum, 6);
				}
			}
		}
	}
}

#else
void PlayForm::Get_NIT_Delivery_Descriptor_Info()
{
	int descriptor_flag;
	int freq;
	int modulation;
	int symbolrate;
	int coderate;
	__int64 tmp_val;
	int sym_flag = 0;
	int freq_flag = 0;
	int modul_flag = 0;
	int code_flag = 0;
	int modul_sys = 0;
	int rolloff = 0;
	int nBoardNum = gGeneral.gnActiveBoard;
	unsigned long setFreq;
	char	snmpStr[128];
	int i;
	strcpy(snmpStr, "None");
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
	{
		TSPH_GET_NIT_SATELLITE_INFO(nBoardNum, &descriptor_flag, &freq, &rolloff, &modul_sys, &modulation, &symbolrate, &coderate);
		if(descriptor_flag == 1)
		{
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
			{
				if(modulation == 1 || modulation == 2)
				{
					modul_flag = 1;
				}

				tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
						  (((symbolrate >> 20) & 0xF) * 10000000) + 
						  (((symbolrate >> 16) & 0xF) * 1000000) + 
						  (((symbolrate >> 12) & 0xF) * 100000) + 
						  (((symbolrate >> 8) & 0xF)  * 10000) + 
						  (((symbolrate >> 4) & 0xF)  * 1000) + 
						  (((symbolrate >> 0) & 0xF)  * 100);
				if((tmp_val % 1000) > 0)
				{
					tmp_val = tmp_val / 1000 + 1;
					tmp_val = tmp_val * 1000;
				}
				if(DVB_S_SYMBOL_RRC_OFF_20_MIN <= tmp_val && tmp_val <= DVB_S_SYMBOL_RRC_OFF_20_MAX)
				{
					sym_flag = 1;
					symbolrate = (int)tmp_val;
				}
				if(modul_flag == 1 && sym_flag == 1)
				{
					sprintf(snmpStr, "DVB-S2,%d,%d,%d,%d", symbolrate, modulation, coderate, rolloff);
				}
			}
			else
			{
				if(modulation == 1)
				{
					modul_flag = 1;
				}
				tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
						  (((symbolrate >> 20) & 0xF) * 10000000) + 
						  (((symbolrate >> 16) & 0xF) * 1000000) + 
						  (((symbolrate >> 12) & 0xF) * 100000) + 
						  (((symbolrate >> 8) & 0xF)  * 10000) + 
						  (((symbolrate >> 4) & 0xF)  * 1000) + 
						  (((symbolrate >> 0) & 0xF)  * 100);
				if((tmp_val % 1000) > 0)
				{
					tmp_val = tmp_val / 1000 + 1;
					tmp_val = tmp_val * 1000;
				}
				if(DVB_S_SYMBOL_RRC_OFF_20_MIN <= tmp_val && tmp_val <= DVB_S_SYMBOL_RRC_OFF_20_MAX)
				{
					sym_flag = 1;
					symbolrate = (int)tmp_val;
				}

				if(modul_flag == 1 && sym_flag == 1)
				{
					sprintf(snmpStr, "DVB-S,%d,%d,%d", symbolrate, modulation, coderate);
				}
			}
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
	{
		TSPH_GET_NIT_CABLE_INFO(nBoardNum, &descriptor_flag, &freq, &modulation, &symbolrate, &coderate);
		if(descriptor_flag == 1)
		{
			tmp_val = (((freq >> 28) & 0xF) * 1000000000) +
				      (((freq >> 24) & 0xF) * 100000000) +
					  (((freq >> 20) & 0xF) * 10000000) + 
					  (((freq >> 16) & 0xF) * 1000000) + 
					  (((freq >> 12) & 0xF) * 100000) + 
					  (((freq >> 8) & 0xF)  * 10000) + 
					  (((freq >> 4) & 0xF)  * 1000) + 
					  (((freq >> 0) & 0xF)  * 100);
			if(48000000 <= tmp_val && tmp_val <= 2150000000)
			{
				freq_flag = 1;
				setFreq = (unsigned long)tmp_val;
			}

			tmp_val = (((symbolrate >> 24) & 0xF) * 100000000) +
					  (((symbolrate >> 20) & 0xF) * 10000000) + 
					  (((symbolrate >> 16) & 0xF) * 1000000) + 
					  (((symbolrate >> 12) & 0xF) * 100000) + 
					  (((symbolrate >> 8) & 0xF)  * 10000) + 
					  (((symbolrate >> 4) & 0xF)  * 1000) + 
					  (((symbolrate >> 0) & 0xF)  * 100);
			if((tmp_val % 1000) > 0)
			{
				tmp_val = tmp_val / 1000 + 1;
				tmp_val = tmp_val * 1000;
			}
			if(QAM_A_SYMBOL_MIN <= tmp_val && tmp_val <= QAM_A_SYMBOL_MAX)
			{
				sym_flag = 1;
				symbolrate = (int)tmp_val;
			}			
			if(freq_flag == 1 && sym_flag == 1)
			{
				sprintf(snmpStr, "QAM-A,%d,%d,%d", setFreq, symbolrate, modulation);
			}

		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
	{
		unsigned int ui_freq;
		int bw, time_slicing_indepth, mpe_fec, constellation, guard, txmod;

		TSPH_GET_NIT_TERRESTRIAL_INFO(nBoardNum, &descriptor_flag, &ui_freq, &bw, &time_slicing_indepth, &mpe_fec, 
												&constellation, &coderate, &guard, &txmod);
		if(descriptor_flag == 1)
		{
			tmp_val = ui_freq * 10;
			if((tmp_val % 100000) >= 50000)
			{
				tmp_val = tmp_val / 100000 + 1;
				tmp_val = tmp_val * 100000;
			}
			else
			{
				tmp_val = tmp_val / 100000;
				tmp_val = tmp_val * 100000;
			}
			if(48000000 <= tmp_val && tmp_val <= 2150000000)
			{
				freq_flag = 1;
				setFreq = (unsigned long)tmp_val;
			}
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
			{
				sprintf(snmpStr, "DVB-H,%d,%d,%d,%d,%d,%d,%d,%d", setFreq, bw, constellation, coderate, txmod, guard, mpe_fec, time_slicing_indepth);
			}
			else
			{
				sprintf(snmpStr, "DVB-H,%d,%d,%d,%d", setFreq, bw, constellation, coderate, txmod, guard);
			}
		}
	}
	//send snmp
}
#endif

//2012/8/31 new rf level control
#ifdef WIN32
#else
void PlayForm::OnValueChanged_RFLevel(double dwRf_level)
{
	int nBoardNum = gGeneral.gnActiveBoard;
	long iAmp_status;
	double rf_level_min, rf_level_max;
	double dwRFLevelOffset = 0.;
	int MultiLevelOffset = 0;
	char strFilePath[512];
	
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
		return;

	rf_level_min = gpConfig->gBC[nBoardNum].gdRfLevelRange_min;
	rf_level_max = gpConfig->gBC[nBoardNum].gdRfLevelRange_max;
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
	{
		MultiLevelOffset = GetMultiOptionLevelOffset(gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBoardId, gpConfig->gBC[nBoardNum].gnRFOutFreq);
		if(((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || (gpConfig->gBC[nBoardNum].gnBoardId == 16))
		{
			MultiLevelOffset = 0;
		}
		rf_level_min = rf_level_min - MultiLevelOffset;
		rf_level_max = rf_level_max - MultiLevelOffset;
	}

	if(rf_level_min > dwRf_level)
	{
		return;
	}
	else if(rf_level_max < dwRf_level)
	{
		return;
	}

	dwRf_level = dwRf_level + MultiLevelOffset;
				
	if(gpConfig->gBC[nBoardNum].gnSingleTone == 1 && 
		(((gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0xB || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0xF) && 
		gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardRev >= 0x6) || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 12 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 16))
	{		
		TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX( nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, dwRf_level, &iAmp_status, gpConfig->gBC[nBoardNum].gnUseTAT4710);
	}
	else
	{
		gpConfig->gBC[nBoardNum].gdRfLevelValue = dwRf_level;
		TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX( nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gdRfLevelValue, &iAmp_status, gpConfig->gBC[nBoardNum].gnUseTAT4710);
	}
	gpConfig->gBC[nBoardNum].gnBypassAMP = iAmp_status;
	//send snmp data
	SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_USE_AMP);
	SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MAX);
}
#endif

#ifdef WIN32
#else

//=================================================================================
PlayForm::PlayForm()
{
}
//=================================================================================
bool PlayForm::Initialize()
{

	//------------------------------------------------------------------
    if (!gUtilInd.Initialize_Application())
    {
		gUtility.DisplayMessage((char *) "Already executed. Will be exit!");
        return false;
    }

	DIR	*dp;
	struct dirent *d;
	struct stat sb;
	char	strMsg[256];

	dp = opendir("./");
	if (dp != NULL)
	{
		while ((d=readdir(dp)) != NULL)
		{
			sprintf(strMsg,"./%s", d->d_name);
			stat(strMsg, &sb);
			{
				if (S_ISREG(sb.st_mode))
				{
					if(strstr(d->d_name, "teleview_usbdevice_"))
					{
						unlink(strMsg);
					}
				}
			}
					
		}
		closedir(dp);
	}

/////////////////////////////////////////////////////////////
	int 	i;

//#ifdef STANDALONE
//	i = gRegVar.Get_StartUp_Value_From_Registry(0, 9);		// Modulation Type
//	DllMain(0, 9, i, 36000000); 		// ATTACH // slot, Type1, IF 
//#else
//	for(int j = 1 ; j < 13 ; j++)
//	{
//		i = gRegVar.Get_StartUp_Value_From_Registry(0, j);		// Modulation Type
//		DllMain(0, j, i, 36000000); 		// ATTACH // slot, Type1, IF
//	}
//#endif

/////////////////////////////////////////////////////////////
	//------------------------------------------------------------------
	// Initialize HW System
	//------------------------------------------------------------------
	gGeneral.gnRealBd_Cnt = TSPH_ConfTvbSytem();
	int nRet;
	gGeneral.gnRealBd_Cnt = TSPH_InitAllRealBd();

	if(gGeneral.gnRealBd_Cnt < 1)
	{
		gUtility.DisplayMessage("Fail to detect any board. Check device drivers are installed.");
//		this->init_flag = false;
		return false;
	}
	gGeneral.gnActiveBoardCount = 0;
	gGeneral.gnBoardNum = 0;
	gGeneral.gnActiveBoard = -1;

	int nRealandVirBdMap[MAX_REAL_VIR_BD_CNT];
	for(i = 0; i < (gGeneral.gnRealBd_Cnt); i++)	//	__SLOT_ALLOC_FROM__
	{
		nRet = TSPH_GetBdId_N(i);
		gpConfig->gBC[i].gnBoardId = nRet;
		gpConfig->gBC[i].gnBoardRev = TSPL_GET_BOARD_REV_EX(i);
		gUtility.MyStrCpy(gpConfig->gszInstalledBoard_Info[i], 256, TSPH_GetBdName_N(i));
		gpConfig->gnInstalledBoard_InitMode[i] = gRegVar.Get_StartUp_Value_From_Registry(0, i);
		gpConfig->gnInstalledBoard_InitMode[i] = gUtilInd.ModulatorTypeValidity(gpConfig->gBC[i].gnBoardId, gpConfig->gBC[i].gnBoardRev, gpConfig->gnInstalledBoard_InitMode[i]);

		nRet = TSPH_ActivateOneBd(i, gpConfig->gnInstalledBoard_InitMode[i], 36000000);
		if(nRet == -1)
		{
			gpConfig->gBC[i].gnBoardStatus = 0;
			gpConfig->nBoardSlotNum[i] = -1;
		}
		else
		{
			if (gpConfig->gBC[i].gnBoardId == 48 || gpConfig->gBC[i].gnBoardId == 60 || gpConfig->gBC[i].gnBoardId == 10 || gpConfig->gBC[i].gnBoardId == 20)
				gpConfig->gBC[i].gnActiveMixerUsed = TSPL_GET_AUTHORIZATION_EX(i);
			gpConfig->gBC[i].gnBoardAuth = TSPL_GET_AUTHORIZATION_EX(i);

			//I/Q PLAY/CAPTURE
			gpConfig->gBC[i].gnIQ_play_support = TSPL_GET_FPGA_INFO_EX(i, 2);
			gpConfig->gBC[i].gnIQ_capture_support = TSPL_GET_FPGA_INFO_EX(i, 3);

			//single tone
			gpConfig->gBC[i].gnSingleToneEnabled = TSPL_GET_FPGA_INFO_EX(i, 6);

			// TVB590V9 - RF/IF AMP
			gpConfig->gBC[i].gnRFAmpUsed = TSPL_GET_BOARD_CONFIG_STATUS_EX(i);

			gpConfig->gBC[i].gnModulatorMode = gpConfig->gnInstalledBoard_InitMode[i];	
			//---------------------------------------------
			// decide gnModulatorMode based on enabled type
			gUtilInd.SyncModulatorMode(i);

			gRegVar.RestoreVariables(i);

			//---------------------------------------------
			if(gGeneral.gnActiveBoard == -1)
			{
				gGeneral.gnActiveBoard = i;
			}
			gGeneral.gnBoardNum = gGeneral.gnBoardNum + (1 << i);
			gGeneral.gnActiveBoardCount++;
			gpConfig->gBC[i].gnBoardStatus = 1;
			gpConfig->nBoardSlotNum[i] = i;

			if(gpConfig->gBC[i].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[i].gnBoardId == 0xf || gpConfig->gBC[i].gnBoardId == 0x16 || gpConfig->gBC[i].gnBoardId == 16)
			{
				nRet = TSPH_GetRealAndVirBdMap(i, nRealandVirBdMap);
				gpConfig->gBC[i].gnRealandvir_cnt = nRet;
				for(int _vCnt = 0; _vCnt < nRet; _vCnt++)
				{
					if(nRealandVirBdMap[_vCnt] == i)
					{
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_IsVirtualSlot = 0;
					}
					else
					{
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_OwnerSlot = i;
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_IsVirtualSlot = 1;
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardId = gpConfig->gBC[i].gnBoardId;
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardRev = gpConfig->gBC[i].gnBoardRev;
						gGeneral.gnBoardNum = gGeneral.gnBoardNum + (1 << nRealandVirBdMap[_vCnt]);
						gGeneral.gnActiveBoardCount++;
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardStatus = 1;
						gpConfig->nBoardSlotNum[nRealandVirBdMap[_vCnt]] = nRealandVirBdMap[_vCnt];
					}
					gpConfig->gBC[i].gnRealandvir_location = gpConfig->gBC[i].gnRealandvir_location + (1 << nRealandVirBdMap[_vCnt]);
					gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_StreamNum = TSPH__STREAM_NUMBER(nRealandVirBdMap[_vCnt]);
				}
			}
		}
		//printf("[dbg-seq] - Initialize : [0x%x:%x:%x]\n", i, gpConfig->gBC[i].gnBoardId, gpConfig->gBC[i].gnRealandvir_location);
	}
	if (gGeneral.gnActiveBoardCount == 0)
	{
		gUtility.DisplayMessage("Refer the log files(Hld___#slot.log, lld_bd_#slot.log) at the working directory!!!.");
//		this->init_flag = false;
		return false;
	}

	gUtilInd.GetIpTable();

	//---------------------------
	// init variables before modCommandLine
	for (i = 0; i <= MAX_BOARD_COUNT; i++)
	{
		gpConfig->gBC[i].gnCmdInitFW = false;
		gpConfig->gBC[i].gnCmdPlayrate = -1;			// gdwPlayRate(i)
		gUtility.MyStrCpy(gpConfig->gBC[i].gnCmdFileName,256, "");
		gpConfig->gBC[i].gnCmdAutoRun = 0;
	}

#if	0
		int nVirBD_VsbCnt;
		int nVirBD_QambCnt;
		//2012/6/28 multi dvb-t
		int nVirBD_DvbtCnt;
		int nVirBD_Cnt;
	
		for(i = 0; i < gGeneral.gnRealBd_Cnt; i++)
		{
			if(gpConfig->gBC[i].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[i].gnBoardId == 0xF || gpConfig->gBC[i].gnBoardId == 0x16 || gpConfig->gBC[i].gnBoardId == 16)
			{
				if(gpConfig->gBC[i].gnRealandvir_cnt == 1)
				{
					if(gpConfig->gBC[i].gnBoardId == _TVB594_BD_ID_)
					{
						nVirBD_VsbCnt = gWrapDll.TSPL_CNT_MULTI_VSB_RFOUT_EX(i);
						if(nVirBD_VsbCnt > 1)
						{
							gWrapDll.TSPH_InitVirBd(i, (nVirBD_VsbCnt - 1));
						}
					}
					else
					{
						nVirBD_VsbCnt = gWrapDll.TSPL_CNT_MULTI_VSB_RFOUT_EX(i);
						nVirBD_QambCnt = gWrapDll.TSPL_CNT_MULTI_QAM_RFOUT_EX(i);
						//2012/6/28 multi dvb-t
						nVirBD_DvbtCnt = gWrapDll.TSPL_CNT_MULTI_DVBT_RFOUT_EX(i);
						if(nVirBD_VsbCnt < nVirBD_QambCnt)
						{
							nVirBD_Cnt = nVirBD_QambCnt;
						}
						else
						{
							nVirBD_Cnt = nVirBD_VsbCnt;
						}
						//2012/6/28 multi dvb-t
						if(nVirBD_DvbtCnt > nVirBD_Cnt)
						{
							nVirBD_Cnt = nVirBD_DvbtCnt;
						}
	
						if(nVirBD_Cnt > 1)
						{
							gWrapDll.TSPH_InitVirBd(i, (nVirBD_Cnt - 1));
						}
					}
					nRet = gWrapDll.TSPH_GetRealAndVirBdMap(i, nRealandVirBdMap);
					gpConfig->gBC[i].gnRealandvir_cnt = nRet;
					for(int _vCnt = 0; _vCnt < nRet; _vCnt++)
					{
						if(nRealandVirBdMap[_vCnt] == i)
						{
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_IsVirtualSlot = 0;
						}
						else
						{
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_OwnerSlot = i;
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_IsVirtualSlot = 1;
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardId = gpConfig->gBC[i].gnBoardId;
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardRev = gpConfig->gBC[i].gnBoardRev;
							gGeneral.gnBoardNum = gGeneral.gnBoardNum + (1 << nRealandVirBdMap[_vCnt]);
							gGeneral.gnActiveBoardCount++;
							gpConfig->gBC[nRealandVirBdMap[_vCnt]].gnBoardStatus = 1;
							gpConfig->nBoardSlotNum[nRealandVirBdMap[_vCnt]] = nRealandVirBdMap[_vCnt];
						}
						gpConfig->gBC[i].gnRealandvir_location = gpConfig->gBC[i].gnRealandvir_location + (1 << nRealandVirBdMap[_vCnt]);
						gpConfig->gBC[nRealandVirBdMap[_vCnt]].gn_StreamNum = gWrapDll.TSPH__STREAM_NUMBER(nRealandVirBdMap[_vCnt]);
					}
				}
			}
		}
#endif	
	
	//2011/11/29 TVB594
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gn_IsVirtualSlot == 1)
	{
		gGeneral.gnActiveBoard = gpConfig->gBC[gGeneral.gnActiveBoard].gn_OwnerSlot;
	}

	//------------------------------------------------------------------
	gRegVar.modCommandLine(gGeneral.gnActiveBoard);

	//------------------------------------------------------------------
	//--- Open_System
	gUtilInd.SyncModulatorMode(gGeneral.gnActiveBoard);
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode != gpConfig->gnInstalledBoard_InitMode[gGeneral.gnActiveBoard])
	{
		gRegVar.RestoreVariables(gGeneral.gnActiveBoard);
		gWrapDll.Change_Modulator_Type(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode);
		UpdateModulatorConfigUI(gGeneral.gnActiveBoard);
		gpConfig->gBC[gGeneral.gnActiveBoard].gnOpenSystem = 1;
//		gpConfig->gBC[gGeneral.gnActiveBoard].downflag = 0;
	}
	else
		gWrapDll.Open_System(gGeneral.gnActiveBoard);

    //------------------------------------------------------------------
    // Initialize UI Display + Initial Check
    //------------------------------------------------------------------
	giInitialized = 1;		// for thread running
	
	gUtilInd.SNMP_Init();
    Display_Init();
	int mode = gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode;
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gbEnabledType[mode] == 0)
	{
		for(int iii = 0; iii < MAX_MODULATORMODE; iii++)
		{
			if(gpConfig->gBC[gGeneral.gnActiveBoard].gbEnabledType[iii] == 1)
			{
				gpConfig->gBC[gGeneral.gnActiveBoard].gnChangeModFlag = 1;
				OnCbnSelchangeModulatorType(iii);
				gpConfig->gBC[gGeneral.gnActiveBoard].gnChangeModFlag = 0;
				break;
			}
		}
	}

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_S2)
	{
		gWrapDll.Set_Symbolrate(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnSymbolRate);
	}

	
	pthread_t m_ThreadId;
	pthread_create(&m_ThreadId, NULL, TimerMain_Process, this);
	DebugOutEx((char *) "Initialize: End");
	gWrapDll.Start_Monitor(gGeneral.gnActiveBoard, 0);

	//printf("[dbg-seq] - 0007\n");

	SNMP_All_DataSend();
	return true;
}

//=================================================================================
void PlayForm::Set_Auto_Play()
{
	long nBoardNum = gGeneral.gnActiveBoard;
	int i;

	//checked Input source
	if(gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC && gpConfig->gBC[nBoardNum].gnUseIPStreaming == 0 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == NO_IP_STREAM)
	{
		//Status PLAY ?
		if(gpConfig->gBC[nBoardNum].gnAuto_Play == 1)
		{
			//printf("AUTO PLAY \n");
			//PlayList
			if(gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
			{
				for(i = 0 ; i < gpConfig->gBC[nBoardNum].nPlayListIndexCount ; i++)
				{
					if(strcmp(gpConfig->gBC[nBoardNum].szPlayFileList[i], gpConfig->gBC[nBoardNum].gszAuto_Play_FileName) == 0)
					{
						gpConfig->gBC[nBoardNum].nPlayListIndexCur = i;
						OnLbnSelchangePlaylist();		 // JAVAJAVA
						OnBnClickedComf1();
						break;
					}
				}
			}
			//FileList
			else if(gpConfig->gBC[nBoardNum].nFileListIndexCount > 0)
			{
				for(i = 0 ; i < gpConfig->gBC[nBoardNum].nFileListIndexCount ; i++)
				{
					if(strcmp(gpConfig->gBC[nBoardNum].szFileFileList[i], gpConfig->gBC[nBoardNum].gszAuto_Play_FileName) == 0)
					{
						gpConfig->gBC[nBoardNum].nFileListIndexCur = i;
						OnLbnSelchangeFilelistbox();

#ifdef STANDALONE
						//2012/6/5 TMCC parameter save. for STANDALONE
						if((gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T) && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
						{
//printf("call Load_TMCC_Parameters_\n");
							SetDefaultValue_noTMCC(gpConfig->gBC[nBoardNum].szCurFileName);
							//Load_TMCC_Parameters_();
							//Set_TMCC_Remuxing();
						}
#endif

						OnBnClickedComf1();
						break;
					}
				}
			}
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnAuto_Play == 1)
	{
		OnBnClickedComf1();
	}
	gpConfig->gBC[nBoardNum].gnAuto_Play = 0;
}

//=================================================================================
void PlayForm::Terminate()
{
	giInitialized = 0;		// for thread running
	Sleep(1000);

	DebugOutEx((char *) "Terminate: Start");
    gUtilInd.SNMP_Term();
	Sleep(100);
	
    for (int i = 0; i <= MAX_BOARD_COUNT; i++)
    {
        if (gpConfig->gBC[i].gnBoardStatus == 1)
        {
            //----- Save Play/File List Index
            if (gGeneral.gnActiveBoard == i && gGeneral.gFactoryDefault == 0)
                SaveStreamListInfo(i, gpConfig->gBC[i].gnModulatorMode);

            gWrapDll.Close_System(i);
        }
    }
    

	//----------------------------------------------------
	// Detatch LLD in HLD: DllMain(
	//for(int j = 1 ; j < gGeneral.gnRealBd_Cnt ; j++)
	{
		DllMain(3, 0, 0, 36000000);			// Detatch // slot, Type1, IF
	}
#ifdef STANDALONE
#else
	DeleteLogFile();
#endif
    delete gpConfig;
	return;
}

void PlayForm::DeleteLogFile()
{
	int limitDay;
	time_t tCur;
	struct tm newtime;
	tCur = time(NULL);
	int Array_days[12] = { 31, 28,31,30,31,30,31,31,30,31,30,31}; 
	localtime_r(&tCur, &newtime);
    int year, month, day;
	year = newtime.tm_year + 1900;
	month = newtime.tm_mon + 1;
	day = newtime.tm_mday;
	char LogDir[512];
	limitDay = 7;
	if((day - limitDay) <= 0)
	{
		if( (month - 1) <= 0)
		{
			year = year - 1;
			month = 12;
			day = 31 + day - limitDay;
		}
		else
		{
			month = month - 1;
			if(month == 2)
			{
				if(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
					day = 29 + day - limitDay;
				else
					day = 28 + day - limitDay;
			}
			else
			{
				day = Array_days[month -1] + day - limitDay;
			}
		}
	}
	else
	{
		day = day - limitDay;
	}
	char BaseDate[16];
	char TargetDate[16];
	gUtility.MySprintf(BaseDate, 16, "%04d%02d%02d", year, month, day);
	gUtility.MySprintf(LogDir, 512, "%s/TPG.LOG", gGeneral.gnStrCurDir);
	chdir(LogDir);
	DIR *d_fh;
	struct dirent *entry;
	if((d_fh = opendir(LogDir)) == NULL)
	{
		//printf("Could not open directory: %s\n", LogDir);
		return;
	}
	while((entry = readdir(d_fh)) != NULL)
	{
		if(strncmp(entry->d_name, "..", 2) != 0 && strncmp(entry->d_name, ".", 1) != 0)
		{
			if(entry->d_type == DT_DIR)
			{
				//printf("Directory Name: %s\n", entry->d_name);
				memcpy(TargetDate, entry->d_name, 8);
				//printf("TargetDate: %s, BaseDate:%s\n", TargetDate, BaseDate);
				if(strcmp(BaseDate, TargetDate) >= 0)
				{
					DIR *dp;
					struct dirent *ep;
					char p_dir[512];
					char p_buf[512];
					sprintf(p_dir, "%s/%s", LogDir, entry->d_name);
					dp = opendir(p_dir);
					while((ep = readdir(dp)) != NULL)
					{
						sprintf(p_buf, "%s/%s", p_dir, ep->d_name);
						unlink(p_buf);
					}
					closedir(dp);
					rmdir(p_dir);
				}
			}
			else
			{
				printf("No Directory Name: %s\n", entry->d_name);
			}
		}
	}
	closedir(d_fh);
//	OutputDebugString(LogDir);
}

//2012/1/12 USB/SDCARD Mount check/////////////////////
#ifdef STANDALONE
void PlayForm::Check_mount_usb_sdcard()
{
	int i;
	int ret = 0;
	for(i = 0; i < 6; i++)
	{
		ret = gUtilInd.check_usb_dev_attached(i);
		if(ret == 1)
		{
			ret = gUtilInd.check_usb_fs_mounted(i);
			if(ret == 1)
				break;
		}
	}
	if(ret != gGeneral.gnUsb_Status)
	{
		gGeneral.gnUsb_Status = ret;
		SNMP_Send_Status(TVB390_USB_MOUNT_STATUS);
		//printf("USB MOUNT VALUE : %d\n", gGeneral.gnUsb_Status);
	}

	for(i = 0; i < 3; i++)
	{
		ret = gUtilInd.check_sdcard_dev_attached(i);
		if(ret == 1)
		{
			ret = gUtilInd.check_sdcard_fs_mounted(i);
			if(ret == 1)
				break;
		}
	}
	if(ret != gGeneral.gnSdcard_Status)
	{
		gGeneral.gnSdcard_Status = ret;
		SNMP_Send_Status(TVB390_SDCARD_MOUNT_STATUS);
		//printf("SDCARD MOUNT VALUE : %d\n", gGeneral.gnSdcard_Status);
	}
}
#endif
//2010/5/27
void PlayForm::GetPlayListFileIndex(long nBoardNum, char *str)
{
	int cnt, cnt2;
	cnt = gpConfig->gBC[nBoardNum].nFileListIndexCount;
	char temp[256], temp2[8];
	if(cnt > 0)
	{
		gUtility.MyStrCpy(str, 256, (char *)"");
		
		cnt2 = gpConfig->gBC[nBoardNum].nPlayListIndexCount;
		for(int i = 0; i < cnt2 ; i ++)
		{
			gUtility.MyStrCpy(temp, 256, gpConfig->gBC[nBoardNum].szPlayFileList[i]);
			for(int j = 0 ; j < cnt ; j++)
			{
				if(strcmp(temp, gpConfig->gBC[nBoardNum].szFileFileList[j]) == 0)
				{	
					gUtility.MySprintf(temp2, 8, (char *)"%d:", j);
					break;
				}
			}
			gUtility.MyStrCat(str, 256, temp2);
		}
	}
	int  a = strlen(str);
	if(a > 1)
		str[a-1] = '\0';
}

//---- ProcessKey(KEY_UP/KEY_DOWN)
void PlayForm::MoveListUpDown(int nKey)
{
	int	nBoardNum = gGeneral.gnActiveBoard;
	int	i;
	
	if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
		gpConfig->gBC[nBoardNum].bDelayinProgress == true ||
		gpConfig->gBC[nBoardNum].bRecordInProgress == true)
		return;
	
	if (nKey == TPG_KEY_UP)
	{
		if (gpConfig->gBC[nBoardNum].fCurFocus == FILELISTWINDOW)
		{
			i = gpConfig->gBC[nBoardNum].nFileListIndexCount - 1;
	
			if (i < 0)
				return;

			if(gpConfig->gBC[nBoardNum].nFileListIndexCur == 0)
			{
				if(gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
				{
					gpConfig->gBC[nBoardNum].fCurFocus = PLAYLISTWINDOW;
					gpConfig->gBC[nBoardNum].nPlayListIndexCur = gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1;
					UpdatePlayListDisplay();
					return;
				}
			}
			if (gpConfig->gBC[nBoardNum].nFileListIndexCur <= 0)
				gpConfig->gBC[nBoardNum].nFileListIndexCur = gpConfig->gBC[nBoardNum].nFileListIndexCount - 1;
			else
				gpConfig->gBC[nBoardNum].nFileListIndexCur = gpConfig->gBC[nBoardNum].nFileListIndexCur - 1;
			
			UpdateFileListDisplay();
		} else
		{
			i = gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1;

			if (i < 0)
			{
				gpConfig->gBC[nBoardNum].fCurFocus = FILELISTWINDOW;
				return;
			}
			
			if(gpConfig->gBC[nBoardNum].nFileListIndexCur == 0)
			{
				if(gpConfig->gBC[nBoardNum].nFileListIndexCount > 0)
				{
					gpConfig->gBC[nBoardNum].fCurFocus = FILELISTWINDOW;
					gpConfig->gBC[nBoardNum].nFileListIndexCur = gpConfig->gBC[nBoardNum].nFileListIndexCount - 1;
					UpdateFileListDisplay();
					return;
				}
			}
			if (gpConfig->gBC[nBoardNum].nPlayListIndexCur <= 0)
				gpConfig->gBC[nBoardNum].nPlayListIndexCur = gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1;
			else
				gpConfig->gBC[nBoardNum].nPlayListIndexCur = gpConfig->gBC[nBoardNum].nPlayListIndexCur - 1;
			UpdatePlayListDisplay();
		}						
	} else if (nKey == TPG_KEY_DOWN)
	{
		if (gpConfig->gBC[nBoardNum].fCurFocus == FILELISTWINDOW)
		{
			i = gpConfig->gBC[nBoardNum].nFileListIndexCount - 1;
	
			if (i < 0)
				return;

			if(gpConfig->gBC[nBoardNum].nFileListIndexCur == (gpConfig->gBC[nBoardNum].nFileListIndexCount - 1))
			{
				if(gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
				{
					gpConfig->gBC[nBoardNum].fCurFocus = PLAYLISTWINDOW;
					gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
					UpdatePlayListDisplay();
					return;
				}
			}
			gpConfig->gBC[nBoardNum].nFileListIndexCur = gpConfig->gBC[nBoardNum].nFileListIndexCur + 1;
			UpdateFileListDisplay();
			
		} else
		{
			i = gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1;

			if (i < 0)
			{
				gpConfig->gBC[nBoardNum].fCurFocus = FILELISTWINDOW;
				return;
			}
			
			if(gpConfig->gBC[nBoardNum].nFileListIndexCur == (gpConfig->gBC[nBoardNum].nPlayListIndexCount - 1))
			{
				if(gpConfig->gBC[nBoardNum].nFileListIndexCount > 0)
				{
					gpConfig->gBC[nBoardNum].fCurFocus = FILELISTWINDOW;
					gpConfig->gBC[nBoardNum].nFileListIndexCur = 0;
					UpdateFileListDisplay();
					return;
				}
			}
			gpConfig->gBC[nBoardNum].nPlayListIndexCur = gpConfig->gBC[nBoardNum].nPlayListIndexCur + 1;
			UpdatePlayListDisplay();
		}						
	}
		
} 



//----------------------------------------------------------------------------------------------------------------
// 2010/4/14
// wait for 2 minutes
void PlayForm::Wait_End_Of_FirmwareUpdate()
{
    char 	buff[256];
	FILE	*fp;
	int		i;
	char	*s;	
	int		in1;
	char strCmd[30];
	
	//--- check Firmware writing
	gGeneral.gMenuCnt = -1;		
	
	fp = popen("/bin/wrtflash /tmp/FW", "r");
	if (fp == NULL)
	{
		perror("error: ");
		printf("=== Firware Update Failed!!!\n");
		SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
		return;
	}
	Sleep(500);
	
	gGeneral.gMenuCnt = 0;	
	i = 0;	
	while (fgets(buff, 256, fp) != NULL && (i < 240) )
	{	
		s = strtok(buff, "/");
		//printf("===> input=(%s)\n", buff,s);
		in1 = strlen(s);
		strCmd[0] = s[in1-3];
		strCmd[1] = s[in1-2];
		strCmd[2] = s[in1-1];
		strCmd[3] = '\0';
		
		in1 = atoi(strCmd);
		if (in1 > 0 && in1 <= 100)
		{
			printf("  Firmware Updating:  %d%% completed\n", in1);
			gGeneral.gMenuCnt = in1;
		}	
		
		SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
		//Sleep(500);
		i++;
		
		if (gGeneral.gMenuCnt == 100)
			break;
	}
	
	if (gGeneral.gMenuCnt >= 90)
	{
		gGeneral.gMenuCnt = 100;
		SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
		printf("=== Firware Update succeeded. time=%d seconds!. Count=%d\n", i/2, (int)gGeneral.gMenuCnt);
		
		Sleep(1000);
		system("/bin/busybox reboot -f");
	} else
	{
		printf("=== Firware Update failed. time=%d seconds!. Count=%d\n", i/2, (int)gGeneral.gMenuCnt);
	}
	pclose(fp);
}


// huy: adding 
void PlayForm::firmware_update()
{
    char 	buff[256];
	FILE	*fp;
	int		i;
	char	*s;	
	int		in1;
	char strCmd[30];
	
	//--- check Firmware writing
	gGeneral.gMenuCnt = -1;		
	
	fp = popen("/bin/wrtflash /tmp/FW", "r");
	if (fp == NULL)
	{
		perror("error: ");
		printf("=== Firware Update Failed!!!\n");
		SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
		return;
	}

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 12)
		TSPL_SET_BOARD_LED_STATUS_EX(gGeneral.gnActiveBoard, 1, 0);

	Sleep(500);
	
	gGeneral.gMenuCnt = 0;	
	i = 0;	

	int writing_status = 0;  // 0: normal writing -1: error writing 	 

	while (fgets(buff, 256, fp) != NULL)
	{	


		char tmp[10];
		tmp[0] = buff[0];
		tmp[1] = buff[1];
		tmp[2] = buff[2];
		tmp[3] = 0;

		if (strcmp(&tmp[0], "cmd") == 0)
		{
			printf("%s\n", buff);

			strCmd[0] = buff[4];
			strCmd[1] = buff[5];
			strCmd[2] = buff[6];
			strCmd[3] = NULL;
			if (strcmp(strCmd, "end") == 0) 
			{
				writing_status = 0;
				break;
			}else if (strcmp(strCmd, "err") == 0) 
			{
				writing_status = -1;
				break;
			}else if (strcmp(strCmd, "per") == 0)
			{
				char percentage[5];
				percentage[0] =  buff[8];
				percentage[1] =  buff[9];
				percentage[2] =  buff[10];
				percentage[3] =  NULL;
				gGeneral.gMenuCnt = atoi(&percentage[0]);
				SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
			}			
		}else // log message from wrtflash process 
		{
			printf("%s\n", buff);
		}

		
		//Sleep(500);
		i++;
		
//		if (gGeneral.gMenuCnt == 100)
//			break;
	};	

	if ( writing_status == 0)
	{
		gGeneral.gMenuCnt = 100;
		SNMP_Send_Status(TVB390_FIRMWARE_WRITE);
		printf("=== Firware Update succeeded. time=%d seconds!. Count=%d\n", i/2, (int)gGeneral.gMenuCnt);
		
		Sleep(1000);
		system("/bin/busybox reboot -f");
	} else
	{
		printf("=== Firware Update failed. time=%d seconds!. Count=%d\n", i/2, (int)gGeneral.gMenuCnt);
	}
	pclose(fp);
}


int PlayForm::Get_Run_Time_Of_CurrentSet(int nBoardNum)
{
	int	iSec = 0;
	if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
		iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/2048000); //6144.0*7300); //2010/8/18   
	//2010/8/5
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		//printf("gGeneral.gnBitrate : %d, gpConfig->gBC[nBoardNum].gdwPlayRate : %d\n", gGeneral.gnBitrate, gpConfig->gBC[nBoardNum].gdwPlayRate);
		if(gGeneral.gnBitrate > 0)
			iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/gGeneral.gnBitrate);
		else
		{
			if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
				iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/gpConfig->gBC[nBoardNum].gdwPlayRate);
			else
				iSec = -1;
		}
	}
	//2011/4/15 ISDB-S
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
		{
			int i, ts_num;
			long	nBoardNum = gGeneral.gnActiveBoard;
			ts_num = 0;

			//2011/4/4 ISDB-S Bitrate
			__int64 dwSize;
			dwSize = 0;
			int ISDBS_runTime;
			int ISDBS_runTime_old;
			ISDBS_runTime = 0;
			ISDBS_runTime_old = 0;

			for(i = 0 ; i < MAX_TS_COUNT ; i++)
			{
	
				if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[i]) > 0 && 
					gpConfig->gBC[nBoardNum].gnSlotCount_M[i] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[i] <= MAX_SLOT_COUNT)
				{
					ts_num++;
					dwSize = gUtilInd.Get_File_Size_BYTE(gpConfig->gBC[nBoardNum].gszTS_M[i]);
					ISDBS_runTime = (int)(dwSize*8/gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i]);
					if(ISDBS_runTime > ISDBS_runTime_old)
					{
						iSec = ISDBS_runTime;
						ISDBS_runTime_old = ISDBS_runTime;
					}
				}
			}
			if(ts_num == 0)
				iSec = -1;
		}
		else if(gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
		{
			//2010/7/28
			if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
				iSec = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0 * 188.0 / 204.0);
			else
				iSec = -1;
		}
		else
		{
			//2010/7/28
			//if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
			//	iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/gpConfig->gBC[nBoardNum].gdwPlayRate);
			if (gGeneral.gnBitrate > 0)
				iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/gGeneral.gnBitrate);
			else
				iSec = -1;
		}
	}
	else
	{
		//2010/7/28
		if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
			iSec = (long) (gpConfig->gBC[nBoardNum].dwFileSize*8/gpConfig->gBC[nBoardNum].gdwPlayRate);
		else
			iSec = -1;
	}
	//2010/9/29 DEBUG
	//printf("==== runtime : %d ====\n", iSec);
	return iSec;
}

//kslee 2010/4/13 
int PlayForm::SnmpRequestedCheckLN(char *strLN)
{
    long    nBoardNum;
	//char	szOldLN[40];
    int     iOpt;
	int value = 0;
	
	nBoardNum = gGeneral.gnActiveBoard;
	if (strlen(strLN) < 32)
    {
        
		//ToggleModulatorOption(0);
        //kslee 2010/1/5
		//gUtilInd.LogMessage(" ");
		//gUtilInd.LogMessageInt(TLV_FAIL_TO_CHECK_LICENSE);
		
		gGeneral.gnLNOpt = 0;
        return 0;
    }

    iOpt = gWrapDll.Check_Ln_Ex(nBoardNum, strLN);
    if (iOpt <= 0)
    {
        
		//ToggleModulatorOption(0);
		//kslee 2010/1/5
		//gUtilInd.LogMessage(" ");
		gGeneral.gnLNOpt = 0;
		//gUtilInd.LogMessageInt(TLV_FAIL_TO_CHECK_LICENSE);
        return 0;
    }
 //   strOldLN = gcnew String(szOldLN);
	//OldLNInfoFlag = 1;

    //ToggleModulatorOption(iOpt);
    for (int i = 0; i < MAX_MODULATORMODE; i++)
	{
		if(((iOpt >> i) & 0x1) == 1)
		{
			if(i == 9)
			{
				value = value + (3 << i);
			}
			else if(i == 15)
			{
				value = value + (1 << DVB_C2);
			}
			else if(i == 16)
			{
				value = value + (1 << IQ_PLAY);
			}
			else if(i == 17)
			{
				value = value + (1 << ISDB_S);
			}
			else if(i < 9)
			{
				value = value + (1 << i);
			}
			else
			{
				value = value + (1 << (i+1));
			}
		}
	}

	gGeneral.gnLNOpt = value;//iOpt;
    gUtilInd.LogMessage((char *)" ");
	return 1;
}

int PlayForm::SnmpRequestedUpdateLN(char *strLN)
{
    char    szInfo[256];
    char    szSN[40];//, /*szOldLN[40],*/ szLN[40];
    FILE    *hFile = NULL;
    int     iOpt;
    long    nBoardNum;
	nBoardNum = gGeneral.gnActiveBoard;
	gGeneral.gnLNOpt = 0;

	if(strlen(strLN) >= 32)
	{
		iOpt = gWrapDll.Check_Ln_Ex(nBoardNum, strLN);
        if (iOpt > 0)
		{
			gUtility.MyStrCpy(szSN, 40, gWrapDll.Get_AdaptorInfo(nBoardNum,0));  // Serial Number
			gpConfig->gBC[nBoardNum].gnOpenSystem = 0;
			//kslee 2010/4/29
			gGeneral.gndownflag = 1;
			
			//gGeneral.gnLNOpt = iOpt;
			gUtilInd.OpenAndAppendLNData(strLN, szSN, iOpt);
		}
		else
		{
			//gUtilInd.LogMessageInt(TLV_FAIL_TO_CHECK_LICENSE);
			return 0;
		}
	}
	else
	{
		//gUtilInd.LogMessageInt(TLV_FAIL_TO_CHECK_LICENSE);
		return 0;
	}

	return 1;
}


//-----------------------------------------------------------
// Get Mac/Ip/Subnetmask/Gateway address strings
//------------------------------------------------------------
bool is_digit(char dd)
{
	if (dd >= '0' && dd <= '9')
		return true;
	else
		return false;
}

int  get_address_strings()
{
	int     eth_fd, ret;
	struct  ifreq   _req;
	char	mac_address_string[32];
	char 	mac_address[6] = {0, 0, 0, 0, 0, 0};
	struct  sockaddr_in *sin; 

	memset(mac_address_string, 0x00, sizeof(mac_address_string));
	memset(gGeneral.gnIpAddress, 0x00, sizeof(gGeneral.gnIpAddress));
	memset(gGeneral.gnSubnetMask, 0x00, sizeof(gGeneral.gnSubnetMask));
	memset(gGeneral.gnGateway, 0x00, sizeof(gGeneral.gnGateway));

	//----------------------------------------------------
	//--- open socket
	eth_fd = socket(PF_INET, SOCK_STREAM, 0); 
	if (eth_fd < 0)
		return -1;
	
    strcpy(_req.ifr_name, "eth0");

	//----------------------------------------------------
	//--- get mac address
    ret = ioctl(eth_fd, SIOCGIFHWADDR, &_req);
	if (ret == -1)
	{
		printf("[ERROR] get mac address error\n");
		return -1;
	}
	memcpy(mac_address, _req.ifr_hwaddr.sa_data, 6);
	sprintf(mac_address_string, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac_address[0] & 0xFF, mac_address[1] & 0xFF, mac_address[2] & 0xFF,
		mac_address[4] & 0xFF, mac_address[5] & 0xFF, mac_address[6] & 0xFF);

	printf("=== MAC: (%s)\n", mac_address_string);

	//----------------------------------------------------
	//--- get ip address
	ret = ioctl(eth_fd, SIOCGIFADDR, &_req);
 	if (ret == -1)
	{
		 printf("[ERROR] get ip address error\n");
		 return -1;
	}

	sin = (struct sockaddr_in *)&_req.ifr_addr; 
	sprintf(gGeneral.gnIpAddress, "%s", inet_ntoa(sin->sin_addr));
	printf("=== IP: (%s)\n", gGeneral.gnIpAddress);
	
	//----------------------------------------------------
	//--- get subnet mask
	ret = ioctl(eth_fd, SIOCGIFNETMASK, &_req);
 	if (ret == -1)
	{
		 printf("[ERROR] get subnet address error\n");
		 return -1;
	}
	

	sin = (struct sockaddr_in *)&_req.ifr_addr; 
	sprintf(gGeneral.gnSubnetMask, "%s", inet_ntoa(sin->sin_addr));
	printf("=== Mask: (%s)\n", gGeneral.gnSubnetMask);
	
	//----------------------------------------------------
	//---gateway string
	get_gateway_address(gGeneral.gnGateway);
	printf("=== Gateway: (%s)\n", gGeneral.gnGateway);
	return ret;
}


//------------------------------------------------------------
int get_gateway_address(char *strgw)
{
	FILE 	*fp;
	int		i = 0;
	
	char line[255];
	char *p, *q;

	system("netstat -r > gateway.txt");

	fp = fopen("gateway.txt", "rb");
	if (!fp)
		return -1;

	while (fgets(line, 255, fp))
		if (p = strstr(line, "default"))
			break;

	i = 0;

	if (!p)
		return -1;
	
	while (!is_digit(p[i]))
		i++;

	q = p + i;
	p = strstr(q, ".");
	p++;
	p = strstr(p, ".");
	p++;
	p = strstr(p, ".");
	p++;
	i = 0;

	while (is_digit(p[i]))
		i++;

	q[strlen(q) - strlen(p) + i] = 0;
	sprintf(strgw, "%s", q);
	fclose(fp);
	return 0;
}

//2011/1/4 TMCC Setting =======================================================================================================================================================

//=============================================================================================================================================================================
void PlayForm::SetT2MI()
{
	long PLP_COD;
	long nBoardNum = gGeneral.gnActiveBoard;
	int Bitrate = 0;
	int BandWidth = gpConfig->gBC[nBoardNum].gnT2MI_BW;
	if(BandWidth == 0)
		BandWidth = BandWidth + 4;
	else if(BandWidth == 1)
		 BandWidth = BandWidth + 2;
	else
		 BandWidth = BandWidth - 2;
	if((gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM) || gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
	{
		//if((gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM))
		//	printf("======IP======\n");
		//if(gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
		//	printf("======ASI======\n");

		PLP_COD = gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD;
		if(PLP_COD == 1)
			PLP_COD = 2;
		else if(PLP_COD == 2)
			PLP_COD = 3;
		else if(PLP_COD == 3)
			PLP_COD = 1;

		TSPH_SET_T2MI_PARAMS(nBoardNum, 
						BandWidth,
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
		return;
	}
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
							BandWidth,
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
			Bitrate = Bitrate + gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[i];
		}
	}
//	gpConfig->gBC[nBoardNum].gdwPlayRate = Bitrate; 
	OnEnChangeElabplayrate(Bitrate);
//====================================================================================================================

#if 0
	long PlaybackRate, MaxPlaybackRate;
	long PLP_MOD, PLP_COD, PLP_FEC_TYPE;
	long PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE;
	long S1, NUM_DATA_SYMBOLS, PILOT_PATTERN;
	long BW, BWT_EXT, L1_POST_SIZE, HEM, NPD;
	long TLV_BW, TLV_FFT_SIZE;
	long L1_MOD, L1_POST_INFO_SIZE, L1CONF_LEN, L1DYN_CURR_LEN;
	long nBoardNum = gGeneral.gnActiveBoard;
	long TLV_PLP_COD;

	char str[256];

	L1CONF_LEN = 191;
	L1DYN_CURR_LEN = 127;
	//2010/12/28 DVB-T2 MULTI-PLP ==================================================================================
	int plp_cnt = 0;
	int NUM_RF = 1;
	int S2 = 0;
	int NUM_AUX = 0;
    int curRow;
	
	plp_cnt = GRID_PLP->Rows->Count;
	if(plp_cnt < 1)
	{
		Total_Text_PLAYBACKRATE->Text = "n.a.";
		return;
	}

	if(plp_cnt > 1)
	{
		L1CONF_LEN = (35 + (35 * NUM_RF) + ((S2 & 0x01) == 1 ? 34 : 0) + (plp_cnt * 89) + 32 + (NUM_AUX * 32));
		L1DYN_CURR_LEN = (71 + (plp_cnt * 48) + 8 + (NUM_AUX * 48));
	}

	//===============================================================================================================
	L1_POST_INFO_SIZE = L1CONF_LEN + L1DYN_CURR_LEN;

	L1_MOD = gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD;
	GUARD_INTERVAL = gpConfig->gBC[nBoardNum].gnT2MI_GUARD;

	TLV_FFT_SIZE = gpConfig->gBC[nBoardNum].gnT2MI_FFT;
	if(TLV_FFT_SIZE == 0)
		FFT_SIZE = 3;
	else if(TLV_FFT_SIZE == 1)
		FFT_SIZE = 0;
	else if(TLV_FFT_SIZE == 2)
		FFT_SIZE = 2;
	else if(TLV_FFT_SIZE == 3)
	{
		if(GUARD_INTERVAL <= 3)
			FFT_SIZE = 1;
		else
			FFT_SIZE = 6;
	}
	else if(TLV_FFT_SIZE == 4)
		FFT_SIZE = 4;
	else if(TLV_FFT_SIZE == 5)
	{
		if(GUARD_INTERVAL <= 3)
			FFT_SIZE = 5;
		else
			FFT_SIZE = 7;
	}
	
	S1 = 0; //SISO
	NUM_DATA_SYMBOLS = gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL;
	PILOT_PATTERN = gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN;
	TLV_BW = gpConfig->gBC[nBoardNum].gnT2MI_BW;
	if(TLV_BW == 0)	//6MHz
		BW = 2;
	else if(TLV_BW == 1) //7MHz
		BW = 3;
	else if(TLV_BW == 2) //8MHz
		BW = 4;
	else
		BW = 1;			//5MHz

	if(gpConfig->gBC[nBoardNum].gnT2MI_BWT == 1)
		BWT_EXT = 1;
	else
		BWT_EXT = 0;

	NPD = 0; //Null packet deletion

	L1_POST_SIZE = gUtilInd.Calulate_L1_Post_Size(L1_MOD, FFT_SIZE, L1_POST_INFO_SIZE);

    PlaybackRate = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 0, 
        PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE, 
        S1, NUM_DATA_SYMBOLS, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD);
    
    MaxPlaybackRate = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 1, 
        PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE,
        S1, NUM_DATA_SYMBOLS, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD);
    
	double FREQUENCY;
	long NETWORK_ID, T2_SYSTEM_ID, cell_id, NUM_T2_FRAMES, PID;

	FREQUENCY = gpConfig->gBC[nBoardNum].gnRFOutFreq;
	cell_id = gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID;
    NETWORK_ID = gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID;
    T2_SYSTEM_ID = gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID;
    NUM_T2_FRAMES = gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME;
    PID = gpConfig->gBC[nBoardNum].gnT2MI_PID;


    if(S1 == 0)
	{
        if(FFT_SIZE == 3)
		{    //1K
            if( GUARD_INTERVAL == 4 || GUARD_INTERVAL == 0 || GUARD_INTERVAL == 6 || GUARD_INTERVAL == 5)
			{ //1/128, 1/32, 19/256 or 19/128
                //Text_MAXPLAYBACKRATE->Text = "n.a.";
				gUtility.MyStrCpy(str, 256, (char *)"n.a.");
			}
            else
			{
                if( GUARD_INTERVAL == 1)
				{  // 1/16
                    if( PILOT_PATTERN != 3 && PILOT_PATTERN != 4)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
				}
                else if( GUARD_INTERVAL == 2 )
				{  // 1/8
                    if( PILOT_PATTERN != 1 && PILOT_PATTERN != 2)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
				}
                else if( GUARD_INTERVAL == 3)
				{  // 1/4
                    if( PILOT_PATTERN != 0)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
                }
            }
		}    
        else if( FFT_SIZE == 0 || FFT_SIZE == 2)
		{    //2K or 4K
            if( GUARD_INTERVAL == 4 || GUARD_INTERVAL == 6 || GUARD_INTERVAL == 5)
			{ //1/128, 19/256 or 19/128
				gUtility.MyStrCpy(str, 256, (char *)"n.a.");
			}
            else
			{
                if( GUARD_INTERVAL == 0) 
				{  // 1/32
                    if( PILOT_PATTERN != 3 && PILOT_PATTERN != 6) //2010/8/4 
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
				}
                else if( GUARD_INTERVAL == 1)
				{  // 1/16
                    if( PILOT_PATTERN != 3 && PILOT_PATTERN != 4)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
				}
                else if( GUARD_INTERVAL == 2)
				{  // 1/8
                    if( PILOT_PATTERN != 1 && PILOT_PATTERN != 2)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
				}
                else if( GUARD_INTERVAL == 3)
				{  // 1/4
                    if( PILOT_PATTERN != 0)
					{
						gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                    }
                }
            }
		}
        else if( FFT_SIZE == 1 || FFT_SIZE == 6)
		{    //8K
            if(GUARD_INTERVAL == 0)
			{   //1/32
                if (PILOT_PATTERN != 3 && PILOT_PATTERN != 6) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
            else if( GUARD_INTERVAL == 1) 
			{  // 1/16
                if( PILOT_PATTERN != 3 && PILOT_PATTERN != 4 && PILOT_PATTERN != 7) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
            else if( GUARD_INTERVAL == 6)
			{  // 19/256
                if( PILOT_PATTERN != 3 && PILOT_PATTERN != 4 && PILOT_PATTERN != 7) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
            else if (GUARD_INTERVAL == 2)
			{  // 1/8
                if( PILOT_PATTERN != 1 && PILOT_PATTERN != 2 && PILOT_PATTERN != 7) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
            else if( GUARD_INTERVAL == 5)
			{  // 19/128
                if( PILOT_PATTERN != 1 && PILOT_PATTERN != 2 && PILOT_PATTERN != 7) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
            else if (GUARD_INTERVAL == 3) 
			{  // 1/4
                if (PILOT_PATTERN != 0 && PILOT_PATTERN != 7) //2010/8/4
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
            }
			//2010/8/4
            else if (GUARD_INTERVAL == 4)
			{
                if (PILOT_PATTERN != 6) 
				{
					gUtility.MyStrCpy(str, 256, (char *)"n.a.");
                }
			}
        }
    }

	//char str[256];
	//gBaseUtil.GetText_Text(Text_MAXPLAYBACKRATE, str);
	if(strcmp(str, "n.a.") == 0)
	{
        ;
	}
    else
	{
        if( NUM_T2_FRAMES < 2 || NUM_T2_FRAMES > 255)
		{
			;		
		}
        
        if ((FFT_SIZE == 3 && NUM_DATA_SYMBOLS < (16 + 7)) || (FFT_SIZE == 0 && NUM_DATA_SYMBOLS < (8 + 7)) || (FFT_SIZE == 2 && NUM_DATA_SYMBOLS < (4 + 7)) ||
            ((FFT_SIZE == 1 || FFT_SIZE == 6) && NUM_DATA_SYMBOLS < (2 + 7)) || (FFT_SIZE == 4 && NUM_DATA_SYMBOLS < (8 + 7)) || (FFT_SIZE == 2 && NUM_DATA_SYMBOLS < (4 + 7)) ||
            ((FFT_SIZE == 5 || FFT_SIZE == 7) && NUM_DATA_SYMBOLS < (1 + 3)))
		{
			gUtility.MyStrCpy(str, 256, (char *)"n.a.");
			//Text_MAXPLAYBACKRATE->Text = "n.a.";
            //Text_PLAYBACKRATE->Text = "n.a.";
            //gUtilInd.LogMessage ("#DATA SYMBOLS is too small.");
            //Text_NUM_DATA_SYMBOLS->BackColor = System::Drawing::Color::Red;
            return;
		}
	}
    
    //gUtilInd.LogMessage ("");
	//Text_MAXPLAYBACKRATE->Text = MaxPlaybackRate.ToString("###,###,###");
	//Text_PLAYBACKRATE->Text = PlaybackRate.ToString("###,###,###");

	int totalPlaybackRate = 0;
	int msgflag = 0;
  
	for(curRow = 0; curRow < GRID_PLP->Rows->Count; curRow++)
	{
		PLP_MOD = GetGRID_PLP_MOD(curRow);
		TLV_PLP_COD = GetGRID_PLP_COD(curRow);
		PLP_FEC_TYPE = GetGRID_PLP_FEC(curRow);
		PLP_NUM_BLOCKS = GetGRID_PLP_BLOCKS(curRow);
		HEM = GetGRID_PLP_HEM(curRow);
		//2010/6/3
		if(TLV_PLP_COD == 0)
			PLP_COD = 0;
		else if(TLV_PLP_COD == 1)
			PLP_COD = 2;
		else if(TLV_PLP_COD == 2)
			PLP_COD = 3;
		else if(TLV_PLP_COD == 3)
			PLP_COD = 1;
		else if(TLV_PLP_COD == 4)
			PLP_COD = 4;
		else
			PLP_COD = 5;
	    PlaybackRate = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 0, 
		    PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE, 
			S1, NUM_DATA_SYMBOL, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD);
    
	    MaxPlaybackRate = gUtilInd.CalcT2MI_Bitrate(nBoardNum, 1, 
		    PLP_MOD, PLP_COD, PLP_FEC_TYPE, PLP_NUM_BLOCKS, GUARD_INTERVAL, FFT_SIZE,
			S1, NUM_DATA_SYMBOL, PILOT_PATTERN, BW, BWT_EXT, L1_POST_SIZE, HEM, NPD); //2010/12/28 DVB_T2 MULTI-PLP

		
		gUtilInd.LogMessage ("");
		if(PlaybackRate < 1 || PlaybackRate > 72000000 || MaxPlaybackRate < 1 || MaxPlaybackRate > 72000000)
		{
			//if(PlaybackRate < 1 || PlaybackRate > 72000000)
			//	T2MI_INFO->Text = T2MI_INFO->Text + "PLAYBACKRATE shall be less than 72Mbps.\r\n";//"DATA RATE shall be less than 72Mbps.";
			//else
			//	T2MI_INFO->Text = T2MI_INFO->Text + "MAX. PLAYBACKRATE shall be less than 72Mbps.\r\n";
			//GRID_PLP->Rows[curRow]->Cells[PLP_PLPBIT_POS]->Style->BackColor = System::Drawing::Color::Red;
			return;
		}
		else
		{
			if(PlaybackRate > MaxPlaybackRate)
			{
				//T2MI_INFO->Text = T2MI_INFO->Text + "PLAYBACKRATE is greater than MAX. PLAYBACKRATE.\r\n";//"DATA RATE is greater than MAX. RATE.\r\n";
				//GRID_PLP->Rows[curRow]->Cells[PLP_PLPBIT_POS]->Style->BackColor = System::Drawing::Color::Red;
				return;
			}
			else
			{
				totalPlaybackRate = totalPlaybackRate + PlaybackRate;
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[curRow] = PlaybackRate;
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_MAX_Playrate[curRow] = MaxPlaybackRate;
			
			
				if(tempBitrate->Equals("UNKNOWN") == false)
				{
					tempBitrate = tempBitrate->Replace(",", "");
					gBaseUtil.ChangeCharFromString(tempBitrate, str);
					if(atoi(str) <= PlaybackRate)
				//==================================================================================			
						gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = 1;
					else
					{
						gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = 0;
						if(msgflag == 0)
						{
							msgflag = 1;
						}
						//T2MI_INFO->Text = T2MI_INFO->Text + "PLAYBACKRATE is less than ESTIMATED PLAYBACK RATE.\r\n";
						//T2MI_INFO->Text = T2MI_INFO->Text + "it will be played slowly.\r\n";
					}
				}
				else
					gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag = 0;
			}
		}
	}
#endif  
}


void PlayForm::SetDVBC2_parameters()
{
	long PLP_COD;
	long nBoardNum = gGeneral.gnActiveBoard;
	int Bitrate = 0;

	//if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
	//{
			//TSPH_SET_C2MI_PARAMS(
			//		nBoardNum, 
			//		gpConfig->gBC[nBoardNum].gnBandwidth,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_L1,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Guard,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Network,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_System,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_StartFreq,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_NumNoth,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_ReservedTone,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_NotchStart,
			//		(gpConfig->gBC[nBoardNum].gnDVB_C2_NotchWidth + 1),
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_type,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_FecHeader,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_PLP_HEM,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft,
			//		(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Mod + 1),
			//		(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Code + 1),
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Fec,
			//		i,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Blk,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_File_Bitrate,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName,
			//		gpConfig->gBC[nBoardNum].gnDVB_C2_CreateFile
			//		);
	//	return;
	//}
	//2010/12/21 DVB-T2 MULTI-PLP ========================================================================================
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
			Bitrate = Bitrate + gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[i];
		}
	}
//	gpConfig->gBC[nBoardNum].gdwPlayRate = Bitrate; 
	OnEnChangeElabplayrate(Bitrate);
}
#ifdef STANDALONE
void PlayForm::RebootSystem(int opt)
{
	int res;
	if(opt == 1)
	{
		Sleep(1000);
		system("/bin/busybox reboot -f");
	}
	else if(opt == 2)
	{
		DIR	*dp;
		struct dirent *d;
		struct stat sb;
		char	strMsg[256];

		dp = opendir("/sysdb");
		if (dp != NULL)
		{
			while ((d=readdir(dp)) != NULL)
			{
				sprintf(strMsg,"/sysdb/%s", d->d_name);
				//kslee 2010/4/13			
				//if (stat(strMsg, &sb) != -1)
				stat(strMsg, &sb);
				{
					if (S_ISREG(sb.st_mode))
					{
						if(strstr(d->d_name, ".cfg"))
						{
				//printf("FILEPATH ====== :%s\n", strMsg);
							res = remove(strMsg);
							if(res == -1)
							{
								printf("Fail to remove file : %s\n", strMsg);

							}
						}
					}
				}
						
			}
			closedir(dp);
		}
		Sleep(5000);
		system("/bin/busybox reboot -f");
	}

}
#endif

#endif
#ifndef WIN32
//2012/2/29 LINUX Merge ===============================================================

void PlayForm::SendTMCCData()
{
	if(pTsInfo != NULL)
	{
		if(pTsInfo->pid_info[pTsInfo->num_pid_info - 1].PID == 0x1FFF)
			gGeneral.gnPIDCount = pTsInfo->num_pid_info - 1;
		else
			gGeneral.gnPIDCount = pTsInfo->num_pid_info;
	}
	else
		gGeneral.gnPIDCount = 0;
	printf("ASI IN PID count: %d\n", gGeneral.gnPIDCount);
	SNMP_Send_Status(TVB390_TMCC_PID_COUNT);
	SNMP_Send_Status(TVB390_TMCC_BROADCAST);
	SNMP_Send_Status(TVB390_TMCC_MODE);
	SNMP_Send_Status(TVB390_TMCC_GUARD);
	SNMP_Send_Status(TVB390_TMCC_PARTIAL);
	SNMP_Send_Status(TVB390_TMCC_SEG_A);
	SNMP_Send_Status(TVB390_TMCC_MOD_A);
	SNMP_Send_Status(TVB390_TMCC_COD_A);
	SNMP_Send_Status(TVB390_TMCC_TIME_A);
	SNMP_Send_Status(TVB390_TMCC_DATA_A);
	SNMP_Send_Status(TVB390_TMCC_SEG_B);
	SNMP_Send_Status(TVB390_TMCC_MOD_B);
	SNMP_Send_Status(TVB390_TMCC_COD_B);
	SNMP_Send_Status(TVB390_TMCC_TIME_B);
	SNMP_Send_Status(TVB390_TMCC_DATA_B);
	SNMP_Send_Status(TVB390_TMCC_SEG_C);
	SNMP_Send_Status(TVB390_TMCC_MOD_C);
	SNMP_Send_Status(TVB390_TMCC_COD_C);
	SNMP_Send_Status(TVB390_TMCC_TIME_C);
	SNMP_Send_Status(TVB390_TMCC_DATA_C);
}
void PlayForm::OnBtnClicked_Search()
{
	long PLP_COD;
	long GUARD_INTERVAL, FFT_SIZE;
	long S1, NUM_DATA_SYMBOL, PILOT_PATTERN;
	long BW, BWT_EXT, NPD;
	long TLV_BW, TLV_FFT_SIZE;
	long L1_MOD;
	long nBoardNum = gGeneral.gnActiveBoard;

	L1_MOD = gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD;
	GUARD_INTERVAL = gpConfig->gBC[nBoardNum].gnT2MI_GUARD;

	TLV_FFT_SIZE = gpConfig->gBC[nBoardNum].gnT2MI_FFT;
	if(TLV_FFT_SIZE == 0)
		FFT_SIZE = 3;
	else if(TLV_FFT_SIZE == 1)
		FFT_SIZE = 0;
	else if(TLV_FFT_SIZE == 2)
		FFT_SIZE = 2;
	else if(TLV_FFT_SIZE == 3)
	{
		if(GUARD_INTERVAL <= 3)
			FFT_SIZE = 1;
		else
			FFT_SIZE = 6;
	}
	else if(TLV_FFT_SIZE == 4)
		FFT_SIZE = 4;
	else if(TLV_FFT_SIZE == 5)
	{
		if(GUARD_INTERVAL <= 3)
			FFT_SIZE = 5;
		else
			FFT_SIZE = 7;
	}
	
	
	S1 = 0; //SISO
	NUM_DATA_SYMBOL = gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL;		//gBaseUtil.GetText_Text(Text_NUM_DATA_SYMBOLS);
	PILOT_PATTERN = gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN;
	TLV_BW = gpConfig->gBC[nBoardNum].gnT2MI_BW;
	if(TLV_BW == 1)	//6MHz
		BW = 2;
	else if(TLV_BW == 2) //7MHz
		BW = 3;
	else if(TLV_BW == 3) //8MHz
		BW = 4;
	else
		BW = 1;			//5MHz
	if(TLV_BW == 0)
		TLV_BW = TLV_BW + 4;
	else if(TLV_BW == 1)
		TLV_BW = TLV_BW + 2;
	else
		TLV_BW = TLV_BW - 2;

	BWT_EXT = gpConfig->gBC[nBoardNum].gnT2MI_BWT;


	NPD = 0; //Null packet deletion


    
	double FREQUENCY;
	long NETWORK_ID, T2_SYSTEM_ID, cell_id, NUM_T2_FRAMES, PID;
	int i;

	FREQUENCY = gpConfig->gBC[nBoardNum].gnRFOutFreq;
	cell_id = gpConfig->gBC[nBoardNum].gnT2MI_CELL_ID;
    NETWORK_ID = gpConfig->gBC[nBoardNum].gnT2MI_NETWORK_ID;
    T2_SYSTEM_ID = gpConfig->gBC[nBoardNum].gnT2MI_T2_SYSTEM_ID;
	NUM_T2_FRAMES = gpConfig->gBC[nBoardNum].gnT2MI_NUM_T2_FRAME;			//gBaseUtil.GetText_Text(Text_NUM_T2_FRAME);
    PID = gpConfig->gBC[nBoardNum].gnT2MI_PID;
	
	int num_data_symbols = NUM_DATA_SYMBOL;
	int num_blocks[MAX_PLP_TS_COUNT];
	for(i = 0; i < MAX_PLP_TS_COUNT; i++)
	{
		num_blocks[i] = -1;
	}

	for(int curRow = 0; curRow < MAX_PLP_TS_COUNT; curRow++)
	{
		if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[curRow] < 0)
			continue;

		PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[curRow];
		if(PLP_COD == 1)
			PLP_COD = 2;
		else if(PLP_COD == 2)
			PLP_COD = 3;
		else if(PLP_COD == 3)
			PLP_COD = 1;
		TSPH_SET_T2MI_PARAMS(nBoardNum, 
				TLV_BW,
				TLV_FFT_SIZE,
				GUARD_INTERVAL,
				L1_MOD,
				PILOT_PATTERN,
				BWT_EXT,
				FREQUENCY,
				NETWORK_ID,
				T2_SYSTEM_ID,
				cell_id,
				S1,
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[curRow],
				PLP_COD,
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[curRow],
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[curRow],
				NUM_T2_FRAMES,
				NUM_DATA_SYMBOL,
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[curRow],
				PID,
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_Rotation[curRow],
				curRow,
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[curRow],
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[curRow],
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_ORG_Playrate[curRow],
				gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[curRow], 
				gpConfig->gBC[nBoardNum].gnT2MI_PLP_Time_Interleave[curRow]);
		num_blocks[curRow] = gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[curRow];
	}
	int res = TSPH_GET_T2MI_PARAMS(nBoardNum, &num_data_symbols, &num_blocks[0]);
	gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL = num_data_symbols;
	for(i = 0; i < MAX_PLP_TS_COUNT; i++)
	{
		//printf("NUM BLOCKS[%d] : %d, sym : %d\n", i, num_blocks[i], num_data_symbols);
		gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[i] = num_blocks[i];
	}
	SNMP_Send_Status(TVB390_DVB_T2_NUM_DATA_SYMBOL);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_1);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_2);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_3);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_4);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_5);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_6);
	SNMP_Send_Status(TVB390_DVB_T2_PLP_NUM_BLOCK_7);
}
//---------------------------------------------------------------------------
void PlayForm::SetFileListDvbt2_Isdbs(char *strDir)
{
	long	nBoardNum = gGeneral.gnActiveBoard;
	int		index = 0;
	// Set the File list and set the current index of file list
	
	//=========================================
	// test for opendir
	DIR	*dp;
	struct dirent *d;
	struct stat sb;
	char	strMsg[256];

	
	dp = opendir(strDir);
	if (dp != NULL)
	{
		while ((d=readdir(dp)) != NULL)
		{
			sprintf(strMsg,"%s/%s", strDir, d->d_name);
			//printf("FILEPATH ====== :%s\n", strMsg);
			//kslee 2010/4/13			
			//if (stat(strMsg, &sb) != -1)
			stat(strMsg, &sb);
			{
				if (S_ISREG(sb.st_mode))
				{
					if (index < MAX_PLAY_LIST_COUNT)
					{
						if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
						{
							if(strstr(d->d_name, ".t2mi") || strstr(d->d_name, ".T2MI"))
							{
								continue;
							}
							//else if(strstr(d->d_name, ".t2") || strstr(d->d_name, ".T2"))
							//{
							//	strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[index++], d->d_name);
							//	//printf("filename == %s\n", d->d_name);
							//	
							//}
							else if(strstr(d->d_name, ".trp") || strstr(d->d_name, ".ts") || strstr(d->d_name, ".atsc") || strstr(d->d_name, ".dtv") || strstr(d->d_name, ".mpg") || strstr(d->d_name, ".mpeg"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[index++], d->d_name);
								//printf("filename == %s\n", d->d_name);
							}
							else if(strstr(d->d_name, ".TRP") || strstr(d->d_name, ".TS") || strstr(d->d_name, ".ATSC") || strstr(d->d_name, ".DTV") || strstr(d->d_name, ".MPG") || strstr(d->d_name, ".MPEG"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[index++], d->d_name);
								//printf("filename == %s\n", d->d_name);
							}

						}
						else
						{
							//if(gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
								//printf("=====================ATSC_MH===================\n");
							if(strstr(d->d_name, ".t2mi") || strstr(d->d_name, ".T2MI"))
							{
								continue;
							}
							else if(strstr(d->d_name, ".trp") || strstr(d->d_name, ".ts") || strstr(d->d_name, ".atsc") || strstr(d->d_name, ".dtv") || strstr(d->d_name, ".mpg") || strstr(d->d_name, ".mpeg"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[index++], d->d_name);
								//printf("filename == %s\n", d->d_name);
							}
							else if(strstr(d->d_name, ".TRP") || strstr(d->d_name, ".TS") || strstr(d->d_name, ".ATSC") || strstr(d->d_name, ".DTV") || strstr(d->d_name, ".MPG") || strstr(d->d_name, ".MPEG"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[index++], d->d_name);
								//printf("filename == %s\n", d->d_name);
							}
						}
					}
				}
			}
					
		}
		closedir(dp);
	}
	strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileName, "");
	gpConfig->gBC[nBoardNum].gnDvbt2_FileListCount = index;
	qsort((void *)gpConfig->gBC[nBoardNum].szDvbt2_FileList, gpConfig->gBC[nBoardNum].gnDvbt2_FileListCount, 256, directorySort);

	SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_FILE_NAME);
	SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_FILE_COUNT);
}

void PlayForm::GetFileName_Dvbt2_Isdbs(int index)
{
	long	nBoardNum = gGeneral.gnActiveBoard;

	if(index >= 100)
	{
		index = index - 100;
		char strPath[512];
		strcpy(gpConfig->gBC[nBoardNum].szRemux_FileName, gpConfig->gBC[nBoardNum].szDvbt2_FileList[index]);
		sprintf(strPath,"%s/%s", gpConfig->gBC[nBoardNum].szDvbt2_Directory, gpConfig->gBC[nBoardNum].szDvbt2_FileList[index]);

		gpConfig->gBC[nBoardNum].gnDvbt2_bitrate = TSPH_CAL_PLAY_RATE(nBoardNum, strPath, 0);
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
		{
			gGeneral.gnIsdbs_ts_id = TSPH_GET_TS_ID(nBoardNum, strPath);
			SNMP_Send_Status(TVB390_ISDB_S_TS_ID);
		}
		SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_FILE_BITRATE);
	}
	else
	{
		strcpy(gpConfig->gBC[nBoardNum].szDvbt2_FileName, gpConfig->gBC[nBoardNum].szDvbt2_FileList[index]);
		SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_FILE_NAME);
	}
}
void PlayForm::GetPIDInfoLineOne(int index)
{
    char 	buff[256];
	int findflag = 0;
	int PidInfoIndex = 0;
	int i, j;
	if(pTsInfo != NULL)
	{
		for(i = 0; i < pTsInfo->num_pgm_info; i++)
		{
			for(j = 0; j < pTsInfo->pgm_info[i].num_elmt_info; j++)
			{
				if(pTsInfo->pid_info[index].PID == pTsInfo->pgm_info[i].program_map_PID)
				{
					findflag = 1;
					PidInfoIndex = 1;
					break;
				}
				else if(pTsInfo->pid_info[index].PID == pTsInfo->pgm_info[i].elmt_info[j].elementary_PID)
				{
					findflag = 1;
					PidInfoIndex = 2;
					break;
				}
			}
			if(findflag == 1)
				break;
		}
		if(PidInfoIndex == 1)
		{
			gUtility.MySprintf(gGeneral.gszPIDInfo, sizeof(gGeneral.gszPIDInfo), (char *)"%d:%d,PMT,%d,,,%d,%d", index, pTsInfo->pid_info[index].PID,
																pTsInfo->pgm_info[i].program_number, pTsInfo->pid_info[index].bit_rate, pTsInfo->pid_info[index].layer_info);
		}
		else if(PidInfoIndex == 2)
		{
			gUtility.MySprintf(gGeneral.gszPIDInfo, sizeof(gGeneral.gszPIDInfo), (char *)"%d:%d,%d,%d,%d,%s,%d,%d", index, pTsInfo->pid_info[index].PID,pTsInfo->pgm_info[i].program_map_PID,
																pTsInfo->pgm_info[i].program_number, pTsInfo->pgm_info[i].PCR_PID, pTsInfo->pgm_info[i].elmt_info[j].szStreamType, pTsInfo->pid_info[index].bit_rate, pTsInfo->pid_info[index].layer_info);
		}
		else
		{
			gUtility.MySprintf(gGeneral.gszPIDInfo, sizeof(gGeneral.gszPIDInfo), (char *)"%d:%d,%s,,,,%d,%d", index, pTsInfo->pid_info[index].PID,
																pTsInfo->pid_info[index].szPidDesc, pTsInfo->pid_info[index].bit_rate, pTsInfo->pid_info[index].layer_info);
		}

	}
	//gUtility.MySprintf(gGeneral.gszPIDInfo, sizeof(gGeneral.gszPIDInfo), (char *)"%d:%s", index, buff);
	SNMP_Send_Status(TVB390_TMCC_PID_INFO);
}
void PlayForm::Set_TMCC_Remuxing()
{

	int j;
	int nBoardNum =gGeneral.gnActiveBoard;
	//-----------------------------------------
    // Set TMCC Parameters
    j = gWrapDll.Set_Remux_Info(nBoardNum,
            gpConfig->gBC[nBoardNum].tmccInfo.broadcast, (gpConfig->gBC[nBoardNum].tmccInfo.mode + 1), gpConfig->gBC[nBoardNum].tmccInfo.guard, gpConfig->gBC[nBoardNum].tmccInfo.partial,
			gpConfig->gBC[nBoardNum].gdwPlayRate, gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg, gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod, gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod, 
			gpConfig->gBC[nBoardNum].tmccInfo.layerA.time, gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate, gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg, gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod, 
			gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod, gpConfig->gBC[nBoardNum].tmccInfo.layerB.time, gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate, gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg, 
			gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod, gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod, gpConfig->gBC[nBoardNum].tmccInfo.layerC.time, gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.broadcast %d\n",gpConfig->gBC[nBoardNum].tmccInfo.broadcast);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.mode %d\n",gpConfig->gBC[nBoardNum].tmccInfo.mode);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.guard %d\n",gpConfig->gBC[nBoardNum].tmccInfo.guard);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.partial %d\n", gpConfig->gBC[nBoardNum].tmccInfo.partial);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.mod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.cod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.time %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.time);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.bitrate);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.mod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.cod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.time %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.time);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.bitrate);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.mod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.cod);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.time %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.time);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.bitrate);
	if(j < 0)
	{
		//printf("[DEBUG] return Set_TMCC_Remuxing func [%d] \n", j);
		return;
	}

	if(gpConfig->gBC[nBoardNum].tmccInfo.layerA.seg <= 0)
	{
		gpConfig->gBC[nBoardNum].tmccInfo.layerA.count = 0;
		if(gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer == 0)
		{
			gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		}
	}

	if(gpConfig->gBC[nBoardNum].tmccInfo.layerB.seg <= 0)
	{
		gpConfig->gBC[nBoardNum].tmccInfo.layerB.count = 0;
		if(gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer == 1)
		{
			gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		}
	}
	
	if(gpConfig->gBC[nBoardNum].tmccInfo.layerC.seg <= 0)
	{
		gpConfig->gBC[nBoardNum].tmccInfo.layerC.count = 0;
		if(gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer == 2)
		{
			gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer = 3;
		}
	}
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.info);
//#ifdef STANDALONE
//	//2012/6/5 TMCC parameter save. for STANDALONE
//	if((gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T) && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1 && gpConfig->gBC[nBoardNum].tmccInfo.broadcast != -1)
//	{
//		Save_TMCC_Parameters_();
//	}
//#endif


	gWrapDll.Set_Layer_Info(nBoardNum, gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer, gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map,
        gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count, gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info,
        gpConfig->gBC[nBoardNum].tmccInfo.layerA.count, gpConfig->gBC[nBoardNum].tmccInfo.layerA.info,
        gpConfig->gBC[nBoardNum].tmccInfo.layerB.count, gpConfig->gBC[nBoardNum].tmccInfo.layerB.info,
        gpConfig->gBC[nBoardNum].tmccInfo.layerC.count, gpConfig->gBC[nBoardNum].tmccInfo.layerC.info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer %d\n",gpConfig->gBC[nBoardNum].tmccInfo.other_pid_map_to_layer);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map %d\n",gpConfig->gBC[nBoardNum].tmccInfo.multi_pid_map);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layer_total_count);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layer_total_info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.count %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.count);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerA.info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layerA.info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.count %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.count);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerB.info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layerB.info);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.count %d\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.count);
//printf("gpConfig->gBC[nBoardNum].tmccInfo.layerC.info %s\n",gpConfig->gBC[nBoardNum].tmccInfo.layerC.info);
}
void PlayForm::SetMultiTS_Combiner()
{
	int i, j, k, ts_num;
	long	nBoardNum = gGeneral.gnActiveBoard;
	ts_num = 0;
	j = 0;

	//2011/4/4 ISDB-S Bitrate
	__int64 dwSize;
	dwSize = 0;
	int ISDBS_runTime;
	int ISDBS_runTime_old;
	ISDBS_runTime = 0;
	ISDBS_runTime_old = 0;

	for(i = 0 ; i < MAX_TS_COUNT ; i++)
	{
	
		if(gpConfig->gBC[nBoardNum].gnTS_Selected_M[i] == 1 && strlen(gpConfig->gBC[nBoardNum].gszTS_M[i]) > 0 && 
			gpConfig->gBC[nBoardNum].gnSlotCount_M[i] > 0 && gpConfig->gBC[nBoardNum].gnSlotCount_M[i] <= MAX_SLOT_COUNT)
		{
			//printf("ISDB-S ID : %d\n", gpConfig->gBC[nBoardNum].gnTS_ID_M[i]);
			//printf("ISDB-S Selected_M : %d\n", gpConfig->gBC[nBoardNum].gnTS_Selected_M[i]);
			//printf("ISDB-S TS PATH : %s\n", gpConfig->gBC[nBoardNum].gszTS_M[i]);
			//printf("ISDB-S Filerate : %d\n", gpConfig->gBC[nBoardNum].gnTS_Bitrate_M[i]);
			//printf("ISDB-S Constellation : %d\n", gpConfig->gBC[nBoardNum].gnConstellation_M[i]);
			//printf("ISDB-S Coderate : %d\n", gpConfig->gBC[nBoardNum].gnCoderate_M[i]);
			//printf("ISDB-S Slot : %d\n", gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
			TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, gpConfig->gBC[nBoardNum].gszTS_M[i], gpConfig->gBC[nBoardNum].gnConstellation_M[i],
				gpConfig->gBC[nBoardNum].gnCoderate_M[i], gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
			
			ts_num = ts_num + 1;
			k = gUtilInd.CalcDatarate(gpConfig->gBC[nBoardNum].gnConstellation_M[i], gpConfig->gBC[nBoardNum].gnCoderate_M[i],
							gpConfig->gBC[nBoardNum].gnSlotCount_M[i]);
			j = j + k;
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
		}
	}

	if(ts_num == 0)
	{
		TSPH_SET_COMBINER_INFO(nBoardNum, ts_num, (char *)"", -1, -1, 0);
		Display_PlayRate(1);
	}
	else
	{
		Display_PlayRate(j);
	}
}
#endif

//2012/12/21 added
void PlayForm::SetNewFileSubLoopPos()
{
	long nBoardNum = gGeneral.gnActiveBoard;
    double dwStartPos;
    double dwEndPos;
	int		val;
	if(gpConfig->gBC[nBoardNum].gnUseSubLoop == 1 || gpConfig->gBC[nBoardNum].gnUseSubLoop_Command == 1)
	{
        dwStartPos = (double)gpConfig->gBC[nBoardNum].gnStartTimeOffset * (double)(gpConfig->gBC[nBoardNum].gdwPlayRate)/8.0;
		if((__int64)dwStartPos > gpConfig->gBC[nBoardNum].dwFileSize)
		{
            gpConfig->gBC[nBoardNum].gnStartPosChanged = 0;
            gpConfig->gBC[nBoardNum].gnEndPosChanged = 0;
            gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 0;
#ifdef WIN32 
			gWrapDll.TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_RELEASE, 0);
			gpConfig->gBC[nBoardNum].gnUseSubLoop = 0;
#else
			TSPH_SET_CURRENT_OFFSET(nBoardNum, OFFSET_RELEASE, 0);
			gpConfig->gBC[nBoardNum].gnUseSubLoop = 0;
			gpConfig->gBC[nBoardNum].gnUseSubLoop_Command = 0;
#endif
			return;
		}
		gpConfig->gBC[nBoardNum].gnStartPos = (__int64) dwStartPos;
        gpConfig->gBC[nBoardNum].gnCurrentPos = gpConfig->gBC[nBoardNum].gnStartPos;

#ifdef WIN32
		val = (int)(gpConfig->gBC[nBoardNum].gnStartPos*99/gpConfig->gBC[nBoardNum].dwFileSize);
		SetSliderBar_Value(val);
#endif
        
        dwEndPos = (double)gpConfig->gBC[nBoardNum].gnEndTimeOffset * (double)gpConfig->gBC[nBoardNum].gdwPlayRate/8.0;
        
		if((__int64)dwEndPos > gpConfig->gBC[nBoardNum].dwFileSize)
		{
			gpConfig->gBC[nBoardNum].gnEndPos = gpConfig->gBC[nBoardNum].dwFileSize;                           
		}
		else
			gpConfig->gBC[nBoardNum].gnEndPos = (__int64) dwEndPos;                           

		val = (int)(gpConfig->gBC[nBoardNum].gnEndPos*99/gpConfig->gBC[nBoardNum].dwFileSize);
		gpConfig->gBC[nBoardNum].gnSubLoopEndPos = val;
	}
}

#ifdef WIN32
#else
//2012/9/4 new rf level control
void PlayForm::Display_RF_Level_Range(int Set_rf_level_flag)
{
	int nBoardNum = gGeneral.gnActiveBoard;
	double rf_level_min;
	double rf_level_max;
	int MultiLevelOffset = 0;
	double dwRFLevelOffset = 0.;
	char strFilePath[512];

	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		return;
	}
	gpConfig->gBC[nBoardNum].gdRfLevelRange_max = 0;
	gpConfig->gBC[nBoardNum].gdRfLevelRange_min = 0;
	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode,
		&rf_level_min, &rf_level_max, gpConfig->gBC[nBoardNum].gnUseTAT4710);
	//printf("[EDBUG +++] rf_level_min[%f], rf_level_max[%f]\n", rf_level_min, rf_level_max);
	if(gpConfig->gBC[nBoardNum].gdRfLevelRange_max != rf_level_max || gpConfig->gBC[nBoardNum].gdRfLevelRange_min != rf_level_min)
	{
		gpConfig->gBC[nBoardNum].gdRfLevelRange_max = rf_level_max;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_min = rf_level_min;

		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			MultiLevelOffset = GetMultiOptionLevelOffset(gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBoardId, gpConfig->gBC[nBoardNum].gnRFOutFreq);
			if(((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || gpConfig->gBC[nBoardNum].gnBoardId == 16)
			{
				MultiLevelOffset = 0;
			}
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
	{
		gpConfig->gBC[nBoardNum].gdRfLevelRange_max = rf_level_max;
		gpConfig->gBC[nBoardNum].gdRfLevelRange_min = rf_level_min;
		MultiLevelOffset = GetMultiOptionLevelOffset(gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnBoardId, gpConfig->gBC[nBoardNum].gnRFOutFreq);
		if(((gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16) && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		{
			MultiLevelOffset = 0;
		}
	}
	if(Set_rf_level_flag == 1)
	{
		double _val = 0;
		//2012/9/18
		if(gpConfig->gBC[nBoardNum].gnSingleTone == 1 && 
			(((gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0xB || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 0xF) && 
			gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardRev >= 0x6) || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 12 || gpConfig->gBC[gGeneral.gnActiveBoard].gnBoardId == 16))	//2013/5/27 TVB599 0xC
		{
			_val = _val - MultiLevelOffset + dwRFLevelOffset;
		}
		else
		{
			_val = gpConfig->gBC[nBoardNum].gdRfLevelValue - MultiLevelOffset + dwRFLevelOffset;
		}
		OnValueChanged_RFLevel(_val);
	}
}
#endif

int PlayForm::GetMultiOptionLevelOffset(int mod, int BoardID, unsigned long RFfreq)
{
	int TVB593_MULTI_VSB[] = {
		25, 18, 23, 22, 20, 20, 20, 19, 21, 20, 20,18, 19, 17, 17, 19, 19, 20, 20, 21, 19, 20
	};
	int TVB593_MULTI_QAM_B[]= {
		17, 16, 20, 19, 19, 19, 20, 20, 19, 20, 20, 20, 21, 19, 19, 19, 20, 20, 20, 19, 20, 19
	};
	int TVB591S_MULTI_VSB[] = {
		15, 11, 15, 18, 19, 18, 22, 18, 19, 20, 19, 19, 19, 19, 18, 18, 20, 19, 21, 20, 19, 19
	};
	int TVB591S_MULTI_QAM_B[] = {
		18, 16, 18, 20, 17, 19, 19, 18, 19, 19, 17, 18, 19, 19, 20, 19, 18, 20, 19, 19, 18, 20
	};
	//2012/7/4 multi dvb-t
	int TVB593_MULTI_DVB_T[]= {
		11,  7,  9, 10,  8,  7,  6,  7,  6,  6,  6,  6,  5,  6,  6,  7,  8,  6,  6,  6,  7,  6
	};

	int nIndex;
	unsigned long temp;
	nIndex = (int)(RFfreq / 100000000);

	temp = RFfreq % 100000000;
	if(temp == 0 && nIndex > 0)
		nIndex = nIndex - 1;

	if(BoardID == 0xF)
	{
		if(mod == MULTIPLE_VSB)
			return TVB593_MULTI_VSB[nIndex];
		else if(mod == MULTIPLE_QAMB)
			return TVB593_MULTI_QAM_B[nIndex];
		else if(mod == MULTIPLE_DVBT)
			return TVB593_MULTI_DVB_T[nIndex];

	}
	else if(BoardID == 0x16)
	{
		if(mod == MULTIPLE_VSB)
			return TVB591S_MULTI_VSB[nIndex];
		else if(mod == MULTIPLE_QAMB)
			return TVB591S_MULTI_QAM_B[nIndex];
	}

	return 0;

}

#ifdef WIN32
#else
void PlayForm::Set_ItemList_HW()
{
	char temp_info[32];
	int i, j, ii, jj;
	int nSlotNum;
	int nRealandvir_cnt;

	j = 0;
	strcpy(gGeneral.str_board_id_info, "");
	for(i =0 ; i <= MAX_BOARD_COUNT; i++)
	{
		if(gpConfig->nBoardSlotNum[i] >= 0)
		{
			if(gpConfig->gBC[i].gn_IsVirtualSlot == 1)
				continue;

			gpConfig->nBoardRealSlot[j] = i;
			j++;
			if(gpConfig->gBC[i].gnBoardId == _TVB594_BD_ID_)
			{
				if(gpConfig->gBC[i].gnRealandvir_cnt > 1)
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum + 1);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
					for(ii = 1; ii < gpConfig->gBC[i].gnRealandvir_cnt; ii++)
					{
						for(jj = 0; jj <= MAX_BOARD_COUNT; jj++)
						{
							if(jj != i && i == gpConfig->gBC[jj].gn_OwnerSlot && ii == gpConfig->gBC[jj].gn_StreamNum)
							{
								gpConfig->nBoardRealSlot[j] = jj;
								sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[jj].gn_StreamNum + 1);
								gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
								jj++;
								break;
							}
						}
					}
				}
				else
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
				}
			}
			else if((gpConfig->gBC[i].gnBoardId == 0xF || gpConfig->gBC[i].gnBoardId == 16) && 
						(gpConfig->gBC[i].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[i].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[i].gnModulatorMode == MULTIPLE_DVBT))
			{
				if(gpConfig->gBC[i].gnModulatorMode == MULTIPLE_VSB)
				{
					nRealandvir_cnt = TSPL_CNT_MULTI_VSB_RFOUT_EX(i);
				}
				else if(gpConfig->gBC[i].gnModulatorMode == MULTIPLE_DVBT)
				{
					nRealandvir_cnt = TSPL_CNT_MULTI_DVBT_RFOUT_EX(i);
				}
				else 
				{
					nRealandvir_cnt = TSPL_CNT_MULTI_QAM_RFOUT_EX(i);
				}
				if(nRealandvir_cnt > 1)
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum + 1);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
					for(ii = 1; ii < nRealandvir_cnt; ii++)
					{
						for(jj = 0; jj <= MAX_BOARD_COUNT; jj++)
						{
							if(jj != i && i == gpConfig->gBC[jj].gn_OwnerSlot && ii == gpConfig->gBC[jj].gn_StreamNum)
							{
								gpConfig->nBoardRealSlot[j] = jj;
								sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[jj].gn_StreamNum + 1);
								gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
								jj++;
								break;
							}
						}
					}
				}
				else
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
				}
			}
			else if(gpConfig->gBC[i].gnBoardId == 0x16 && (gpConfig->gBC[i].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[i].gnModulatorMode == MULTIPLE_QAMB))
			{
				if(gpConfig->gBC[i].gnModulatorMode == MULTIPLE_VSB)
				{
					nRealandvir_cnt = TSPL_CNT_MULTI_VSB_RFOUT_EX(i);
				}
				else 
				{
					nRealandvir_cnt = TSPL_CNT_MULTI_QAM_RFOUT_EX(i);
				}
				if(nRealandvir_cnt > 1)
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum + 1);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
					for(ii = 1; ii < nRealandvir_cnt; ii++)
					{
						for(jj = 0; jj <= MAX_BOARD_COUNT; jj++)
						{
							if(jj != i && i == gpConfig->gBC[jj].gn_OwnerSlot && ii == gpConfig->gBC[jj].gn_StreamNum)
							{
								gpConfig->nBoardRealSlot[j] = jj;
								sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[jj].gn_StreamNum + 1);
								gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
								jj++;
								break;
							}
						}
					}
				}
				else
				{
					sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum);
					gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
				}
			}
			else
			{
				sprintf(temp_info, "%d,%d,%d:", gpConfig->gBC[i].gnBoardId, i,  gpConfig->gBC[i].gn_StreamNum);
				gUtility.MyStrCat(gGeneral.str_board_id_info,256, temp_info);
			}
		}
	}
	int len = strlen(gGeneral.str_board_id_info); 
    if( len > 0)
	{
		gGeneral.str_board_id_info[len - 1] = '\0';
	}
}
#endif

#ifdef WIN32
/////////////////////////////////////////////////////   2013/3/14 Source //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===========================================================================================================================================
//2011/7/15 DVB-T2 IP
void PlayForm::SetDVBT2_IP_Params()
{
	int nBoardNum = gGeneral.gnActiveBoard;

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T2 && ((gpConfig->gBC[gGeneral.gnActiveBoard].gnUseIPStreaming == 1 &&
		gpConfig->gBC[gGeneral.gnActiveBoard].gnIPStreamingMode == RECV_IP_STREAM) || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
              gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC))
	{
		int PLP_COD = gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_COD;
		if(PLP_COD == 1)
			PLP_COD = 2;
		else if(PLP_COD == 2)
			PLP_COD = 3;
		else if(PLP_COD == 3)
			PLP_COD = 1;
		
		long t2_bandwidth;
		if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 0)
			t2_bandwidth = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 4;
		else if(gpConfig->gBC[nBoardNum].gnIP_T2MI_BW == 1)
			t2_bandwidth = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW + 2;
		else
			t2_bandwidth = gpConfig->gBC[nBoardNum].gnIP_T2MI_BW - 2;

		if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == false && gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false) 
		{
			gWrapDll.TSPH_SET_T2MI_PARAMS(nBoardNum, 
							t2_bandwidth,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_FFT,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_GUARD,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_L1_MOD,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PILOT_PATTERN,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_BWT,
							gpConfig->gBC[nBoardNum].gnRFOutFreq,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_NETWORK_ID,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_T2_SYSTEM_ID,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_CELL_ID,
							0,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_MOD,
							PLP_COD,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_FEC_TYPE,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_HEM,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_T2_FRAME,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_NUM_DATA_SYMBOL,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_NUM_BLOCK,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PID,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Rotation,
							0,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ID,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Playrate,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_ORG_Playrate,
							gpConfig->gBC[nBoardNum].gszIP_T2MI_PLP_FilePath,
							gpConfig->gBC[nBoardNum].gnIP_T2MI_PLP_Time_Interleave);
		}
	}
	else if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_C2 && ((gpConfig->gBC[gGeneral.gnActiveBoard].gnUseIPStreaming == 1 &&
		gpConfig->gBC[gGeneral.gnActiveBoard].gnIPStreamingMode == RECV_IP_STREAM) || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC ||
              gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC))
	{
		if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == false && gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false) 
		{
			gWrapDll.TSPH_SET_C2MI_PARAMS(
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
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_HEM,
				gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_TunePos,
				gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetRight,
				gpConfig->gBC[nBoardNum].gnDVB_C2_Dslice_OffsetLeft,
				(gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Mod + 1),
				(gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Code + 1),
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Fec,
				0,
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_ID,
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_Blk,
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_File_Bitrate,
				gpConfig->gBC[nBoardNum].gnDVB_C2_IP_Plp_FileName,
				gpConfig->gBC[nBoardNum].gnDVB_C2_CreateFile
				);
		}
	}
	else
	{
		UpdateMultiPLPUI(nBoardNum);
	}
}

//2010/12/22 DVB-T2 MULTI-PLP ===============================================================================================================
void PlayForm::UpdateMultiPLPUI(long nBoardNum)
{

	//2011/1/17 FIXED
	if(gpConfig->gBC[nBoardNum].gnModulatorMode != DVB_T2 && gpConfig->gBC[nBoardNum].gnModulatorMode != DVB_C2)
		return;
	String^ strTemp = "";
	int PLP_COD;
	int plpCnt = 0;
	long PlaybackRate = 0;
	int fileari = 1;
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2) //2011/6/22 DVB-C2 MULTI PLP
	{
		if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
		{
			strTemp = "";
			long t2_bandwidth;
			if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 0)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 4;
			else if(gpConfig->gBC[nBoardNum].gnT2MI_BW == 1)
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW + 2;
			else
				t2_bandwidth = gpConfig->gBC[nBoardNum].gnT2MI_BW - 2;
			for(int i = 0; i < MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnT2MI_PLP_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gszT2MI_PLP_FilePath[i]) > 0)
				{
					plpCnt++;
					PLP_COD = gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[i];
					if(PLP_COD == 1)
						PLP_COD = 2;
					else if(PLP_COD == 2)
						PLP_COD = 3;
					else if(PLP_COD == 3)
						PLP_COD = 1;

					//2011/4/13 added
					if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == false && gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false) 
					{
						gWrapDll.TSPH_SET_T2MI_PARAMS(nBoardNum, 
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
					PlaybackRate = PlaybackRate + gpConfig->gBC[nBoardNum].gnT2MI_PLP_Playrate[i];
				}
			}
		}
	}
	else //DVB-C2
	{
		gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 0;
		if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
		{
			gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer = 1;
			strTemp = "";
			for(int i = 0; i < DVB_C2_MAX_PLP_TS_COUNT; i++)
			{
				if(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_ID[i] >= 0 && strlen(gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_FileName[i]) > 0)
				{
					plpCnt++;
					
					gWrapDll.TSPH_SET_C2MI_PARAMS(
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

					PlaybackRate = PlaybackRate + gpConfig->gBC[nBoardNum].gnDVB_C2_Plp_Plp_Bitrate[i];
				}
			}
		}
	}

	if(plpCnt > 0)
	{
		Display_PlayRate(PlaybackRate);
	}
}

void PlayForm::OnChangeModulatorType(int nBoardNum, int tModType)
{
	//2012/4/13 SINGLE TONE
	gpConfig->gBC[nBoardNum].gnSingleTone = 0;
    // store playlist/filelist index
    SaveStreamListInfo(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);

	//2012/7/27 DVB-T2/C2 ASI IN
	if((gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC) && (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S))
	{
		gWrapDll.TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, FILE_SRC);
		gWrapDll.TSPH_START_MONITOR(nBoardNum, 0);
		if(gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
			gWrapDll.TSPL_SET_SDCON_MODE_EX(nBoardNum, 2);
	}

    //----------------------------------------------------------------
    // Change Modulator type. Set global variables
    gWrapDll.Change_Modulator_Type(nBoardNum, tModType);

	//2012/3/28 Multiple vsb, QAM-B
	Set_ItemList_Combo_Adaptor();
}

void PlayForm::UpdateModulatorConfiguration(int nBoardNum)
{
	int index, i;
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gdwAttenVal = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdwAttenVal;
		gpConfig->gBC[nBoardNum].gnRfLevel_Unit = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRfLevel_Unit;
	}
    //--- CheckLoopAdaptation
    gUtilInd.CheckLoopAdaptation(nBoardNum);
    //-----------------------------------------
    // Spectrum Adjust: Set gnSpectrumInverse
    gUtilInd.Adjust_Spectrum(nBoardNum);

	index = 0;
	for (i = 0; i < MAX_MODULATORMODE; i++)
	{
        if (gpConfig->gBC[nBoardNum].gbEnabledType[i] == 1)
        {
            gpConfig->gBC[nBoardNum].giTypeComboMod[index] = i;
            index++;
        }
	}
    gpConfig->gBC[nBoardNum].giNumModulator = index;
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		//2012/6/28 multi dvb-t 
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			gpConfig->gBC[nBoardNum].gnRFOutFreq = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreq + ((6000000 + (1000000 * gpConfig->gBC[nBoardNum].gnBandwidth)) * gpConfig->gBC[nBoardNum].gn_StreamNum);
		}
		else
			gpConfig->gBC[nBoardNum].gnRFOutFreq = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreq + (6000000 * gpConfig->gBC[nBoardNum].gn_StreamNum);
	}

	if(gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
		gWrapDll.TVB59x_SET_Output_TS_Type_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
}

void PlayForm::SetDvbT2_Bandwidth(int nBoardNum, int nBandwidth)
{
	if(nBandwidth == 0)
		gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, DVB_T2, nBandwidth + 4, gpConfig->gBC[nBoardNum].gnRFOutFreq);
	else if(nBandwidth == 1)
		gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, DVB_T2, nBandwidth + 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
	else
		gWrapDll.TVB380_SET_MODULATOR_BANDWIDTH_EX(nBoardNum, DVB_T2, nBandwidth - 2, gpConfig->gBC[nBoardNum].gnRFOutFreq);
}
//---------------------------------------------------------------
void PlayForm::OnCbnSelchangeParam8()
{
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    int     nBoardNum = gGeneral.gnActiveBoard;
    int     iMod = gpConfig->gBC[nBoardNum].gnModulatorMode;	
	int		iCurIx = gBaseUtil.SelectedIndex_Combo(Combo_PARAM8);
    //CMMB
    if(iMod == CMMB)
        return;
	gpConfig->gBC[nBoardNum].gnBoardStatusReset = 3;
	switch (iMod)
	{
		case DTMB:	// carrier number
			if(gpConfig->gBC[nBoardNum].gnBandwidth == iCurIx)
				break;
			ComF1->Enabled = false;
			gpConfig->gBC[nBoardNum].gnBandwidth = iCurIx;
			if(gpConfig->gBC[nBoardNum].gnBandwidth == 2)
				gpConfig->gBC[nBoardNum].gnSymbolRate = DTMB_SYMBOL_8M;
			else if(gpConfig->gBC[nBoardNum].gnBandwidth == 1)
				gpConfig->gBC[nBoardNum].gnSymbolRate = (DTMB_SYMBOL_8M * 7) / 8;
			else if(gpConfig->gBC[nBoardNum].gnBandwidth == 0)
				gpConfig->gBC[nBoardNum].gnSymbolRate = (DTMB_SYMBOL_8M * 6) / 8;
            gWrapDll.TVB380_SET_MODULATOR_SYMRATE_EX(nBoardNum, iMod, gpConfig->gBC[nBoardNum].gnRFOutFreq, gpConfig->gBC[nBoardNum].gnBandwidth);
			Set_Playrate_Symbolrate_On_ParameterChange(nBoardNum);
			ComF1->Enabled = true;
			break;
	}
}
void PlayForm::Control_EachModulator_Parameters(int nBoardNum)
{
	int nModulatorMode = gpConfig->gBC[nBoardNum].gnModulatorMode;
	int nInputSource = gpConfig->gBC[nBoardNum].gnInputSource;
	if(nModulatorMode == ISDB_T_13)
	{
		if(nInputSource == PRBS_DATA_IN)
		{
			gWrapDll.Set_Tmcc_Remuxer(nBoardNum, 1);
		}
		else if(nInputSource == DVB_ASI_IN)
		{
			gWrapDll.Set_Tmcc_Remuxer(nBoardNum, 1);
		}
		else if(nInputSource == FILE_LIST_IN)
		{
			gWrapDll.Set_Tmcc_Remuxer(nBoardNum, 0);
		}
		else
		{
			if(gpConfig->gBC[nBoardNum].gnHaveTMCC == 0)
				gWrapDll.Set_Tmcc_Remuxer(nBoardNum, 1);
			else
				gWrapDll.Set_Tmcc_Remuxer(nBoardNum, gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer + gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting * 2);
		}
	}
}

void PlayForm::Display_DvbT2_ErrorMessage(int error, int nBoardNum)
{
	char str_msg[512];
	char str_tmp[256];
	char str[64];

	gUtility.MyStrCpy(str_tmp, 256, "");
	gUtility.MySprintf(str_msg, 512, "Selected stream has parameters that our modulator does not support. ");

	if((error & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[MISO] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 1) & 0x1) > 0)
	{
		if(gpConfig->gBC[nBoardNum].gsT2mi_info.fft_mode == 4)
			gUtility.MySprintf(str, 64, "[FFT SIZE: 16K] ");
		else
			gUtility.MySprintf(str, 64, "[FFT SIZE: 32K] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 2) & 0x1) > 0)
	{
		//if(gpConfig->gBC[nBoardNum].gsT2mi_info.papr == 1)
		//	gUtility.MySprintf(str, 64, "[PAPR: ACE] ");
		//else if(gpConfig->gBC[nBoardNum].gsT2mi_info.papr == 2)
		//	gUtility.MySprintf(str, 64, "[PAPR: TR] ");
		//else if(gpConfig->gBC[nBoardNum].gsT2mi_info.papr == 3)
		//	gUtility.MySprintf(str, 64, "[PAPR: ACE & TR] ");
		gUtility.MySprintf(str, 64, "[PAPR] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 3) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[FEF] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 4) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[Constellation rotation of 64QAM or 256QAM] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 5) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[Frame Interval] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 6) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[Time Interleaving Length] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 7) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[Time Interleaving Type] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(((error >> 8) & 0x1) > 0)
	{
		gUtility.MySprintf(str, 64, "[PLP Type 2] ");
		gUtility.MyStrCat(str_tmp, 256, str);
	}

	if(strlen(str_tmp) > 0)
	{
		gUtility.MyStrCat(str_msg, 512, str_tmp);
		::MessageBox::Show(gcnew String(str_msg), "TPG0590VC");
		//gUtilInd.LogMessage(str_msg);
	}


}
#else
void PlayForm::SetSnmpFuncCall_AsiIn(int nParam)
{
	int     nBoardNum = gGeneral.gnActiveBoard;
	if(gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC)
		return;

	if(nParam == 2)
	{
		TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, FILE_SRC);
		TSPH_START_DELAY(nBoardNum, 1);
	}
	else if(nParam == 1)
	{
		TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorSource);
		TSPH_START_DELAY(nBoardNum, 0);
		TSPH_START_MONITOR(nBoardNum, 0);
	}

}
void PlayForm::OnClicked_Btn_Stop()
{
	long		nBoardNum = gGeneral.gnActiveBoard;

	if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)    // Stop Playing
    {
	//printf("=======entered OnClicked_Btn_Stop========\n");
        gWrapDll.Stop_Playing(nBoardNum);
		//2011/8/18
		gGeneral.gnValid_PlayStop = 1;
		SetNewFileSubLoopPos();
		gpConfig->gBC[nBoardNum].gnSingleTone = 0;
   } else if (gpConfig->gBC[nBoardNum].bRecordInProgress == true)  // Stop Recording
    {
		//2012/7/24 DVB-T2 ASI
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
		{
			TSPH_START_RECORD(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName, gpConfig->gBC[nBoardNum].gnModulatorSource);
			gpConfig->gBC[nBoardNum].bRecordInProgress = false;
		}
		else
			gWrapDll.Stop_Recording(nBoardNum);
		//2011/8/18
		gGeneral.gnValid_PlayStop = 1;
    }
    SNMP_Send_Status(TVB390_START_PLAYING);
    SNMP_Send_Status(TVB390_START_RECORDING);
	SNMP_Send_Status(TVB390_RUN_TIME);
}
//---------------------------------------------------------------------------
void PlayForm::OnBnClickedComf1()
{
	long		nBoardNum = gGeneral.gnActiveBoard;
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
	//---------------------------------------------------------------------
    //--- if stop --> play, then get playing file
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == false &&
		gpConfig->gBC[nBoardNum].bRecordInProgress == false)
	{
		long		lindex = 0;
		char		str[256];

		//2011/11/17 IQ NEW FILE FORMAT
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY && gpConfig->gBC[nBoardNum].gnSymbolRate < 1)
		{
			return;
		}

		//--------------------------------------------------------------
	    // if no playing list, then get file from filelist
		if (gpConfig->gBC[nBoardNum].nPlayListIndexCount == 0)
		{
			gpConfig->gBC[nBoardNum].nPlayListIndexStart = 0;
			gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
		    gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = 0;
			lindex = gpConfig->gBC[nBoardNum].nFileListIndexCur;
			if (lindex < 0)
			    lindex = 0;

			gUtility.MyStrCpy(str, 256, gpConfig->gBC[nBoardNum].szFileFileList[lindex]);
			gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, str);
		} else
		{
			gpConfig->gBC[nBoardNum].nPlayListIndexStart = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
		    gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
		    lindex = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
	        if (lindex < 0)
				lindex = 0;
 			gUtility.MyStrCpy(str, 256, gpConfig->gBC[nBoardNum].szPlayFileList[lindex]);
 		    gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, str);
	    }
	}

	long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    long    Max_Playrate;

    if (gpConfig->gBC[nBoardNum].bDelayinProgress == true)          // Stop Delaying
    {
        gWrapDll.Stop_Delaying(nBoardNum);
    } else if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)    // Stop Playing
    {
		OnClicked_Pause();
    } else if (gpConfig->gBC[nBoardNum].bRecordInProgress == true)  // Stop Recording
    {
		return;
	} else                                                          // Playing
    {
        //------------------------------------------------------------
        if ((gpConfig->gBC[nBoardNum].gnModulatorSource != FILE_SRC &&
			lModType != TDMB/* && lModType != ISDB_S*/) || gGeneral.gnVLCRunning > 0)	//2010/12/07 ISDB-S
        {

            if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
			{
				gpConfig->gBC[nBoardNum].gnUseIPStreaming = 0;
                gpConfig->gBC[nBoardNum].gnIPStreamingMode = NO_IP_STREAM;
			}

            gWrapDll.SetStreamSourcePort(FILE_SRC);
 			if((gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T))
			{
//#ifndef STANDALONE
				if( gpConfig->gBC[nBoardNum].gnHaveTMCC == 0)
					SetDefaultValue_noTMCC(gpConfig->gBC[nBoardNum].szCurFileName);
//#endif
			}
			SNMP_Send_Status(TVB390_SET_INPUT_SOURCE);
       }

        gGeneral.nConfirmPlaying = 1;

        //------------------------------------------------------------
        // Check Max Playrate
        Max_Playrate = gUtilInd.CalcBurtBitrate(nBoardNum);
		Max_Playrate = Max_Playrate + (long)round(((double)Max_Playrate * 5.0 / 1000000.0));
		
        if (lModType == ISDB_T_13)
            Max_Playrate = 99999999;
		if(gpConfig->gBC[nBoardNum].gInvalidBitrate == 1 && lModType == TDMB)
		{
			return;
		}
		//------------------------------------------------------------
		//2010/7/18 I/Q PLAY/CAPTURE
		if ((gpConfig->gBC[nBoardNum].gInvalidBitrate == 1 ||
            ((lModType != VSB_8 && lModType != ATSC_MH) && Max_Playrate < gpConfig->gBC[nBoardNum].gdwPlayRate) ||
            ((lModType == VSB_8 || lModType == ATSC_MH) && (gpConfig->gBC[nBoardNum].gdwPlayRate > 19392999))) && 
			lModType != DVB_T2 && lModType != IQ_PLAY && lModType != ISDB_S && lModType != DVB_C2)	//2010/12/06
        {
			gGeneral.nConfirmPlaying = 1;
        }

		if(gpConfig->gBC[nBoardNum].gInvalidBitrate == -1 && lModType == CMMB)
		{
			gUtilInd.LogMessage("Invalid stream(CRC32 ERROR). Can not play this stream.");
			return;
		}

        if (gGeneral.nConfirmPlaying == 1)
        {
			gWrapDll.Start_Playing(nBoardNum,0,gpConfig->gBC[gGeneral.gnActiveBoard].szCurFileName);
			gpConfig->gBC[nBoardNum].gnRepeatCount = 0;
			SNMP_Send_Status(TVB390_LOOP_COUNT);	
			
			if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && 
				gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM_REC)
			{
				SetFileListFromDirectory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
			}
        }

    }
	int _Val_;
	if((gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_ || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 12) &&	gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
		if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
			_Val_ = ((gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3) << 2) + (gpConfig->gBC[nBoardNum].gn_StreamNum & 0x3); 
		else
			_Val_ = gpConfig->gBC[nBoardNum].gn_StreamNum;
		TSPH__SEL_TS_of_ASI310OUT(nBoardNum, _Val_);

	}
	if(gpConfig->gBC[nBoardNum].gnBoardId == 12 && gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
		if(lModType == ISDB_T || lModType == ISDB_T_13 || lModType == DVB_T2 || lModType == ATSC_MH || lModType == ISDB_S || lModType == DVB_C2)
		{
			if(gpConfig->gBC[nBoardNum].gnTsOutput_Mode == 1)
			{
				gpConfig->gBC[nBoardNum].gnTsOutput_Mode = 0;
				TVB59x_SET_Output_TS_Type_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
			}
		}
		else
		{
			if(gpConfig->gBC[nBoardNum].gnOutputClockSource == 1 || (gpConfig->gBC[nBoardNum].gdwPlayRate <= 19392000 || gpConfig->gBC[nBoardNum].gdwPlayRate >= 19393000))
			{
				if(gpConfig->gBC[nBoardNum].gnTsOutput_Mode == 1)
				{
					gpConfig->gBC[nBoardNum].gnTsOutput_Mode = 0;
					TVB59x_SET_Output_TS_Type_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
				}
			}
		}
	}
    SNMP_Send_Status(TVB59x_TS_OUTPUT_SEL);
    SNMP_Send_Status(TVB390_START_PLAYING);
    SNMP_Send_Status(TVB390_START_RECORDING);
	SNMP_Send_Status(TVB390_RUN_TIME);
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
		printf("[Debug] Sleep S=====\n");
		Sleep(5000);
		printf("[Debug] Sleep E=====\n");
		TVB59x_SET_Reset_Control_REG_EX(nBoardNum, 8);
	}
}

//----------------------------------------------------------------------------------------------------------------------------
void PlayForm::OnBnClickedComf2()
{
    gWrapDll.ToggleLoopMode(gGeneral.gnActiveBoard);
    SNMP_Send_Status(TVB390_SET_PLAY_MODE);
}
//-----------------------------------------------------------------------------------------------------------------------------
void PlayForm::OnBnClickedComf4()
{
    long    nBoardNum = gGeneral.gnActiveBoard;
    
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
	{
        AddFile2PlayList();
        SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
        SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
        SNMP_Send_Status(TVB390_PLAY_LIST_NAME);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void PlayForm::OnBnClickedComf5()
{
    long		nBoardNum = gGeneral.gnActiveBoard;
	int			index = gpConfig->gBC[nBoardNum].nPlayListIndexCur;

    if (gpConfig->gBC[nBoardNum].bPlayingProgress == false &&
        gpConfig->gBC[nBoardNum].bRecordInProgress == false)
    {
        if(gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0)
		{
            RemovePlayList(index);
        }
        SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
        SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
        SNMP_Send_Status(TVB390_PLAY_LIST_NAME);
    }
}
//---------------------------------------------------------------
void PlayForm::OnBnClickedComf6()
{
    // Recording
    long    nBoardNum = gGeneral.gnActiveBoard;

    //------------------------------------------------------------
    // If Playing and use AV streaming
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
        gpConfig->gBC[nBoardNum].bRecordInProgress == true)
    {
		return;
    }

    //------------------------------------------------------------
    if (gpConfig->gBC[nBoardNum].bDelayinProgress == true)
        gWrapDll.Stop_Delaying(nBoardNum);
    else if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
        gWrapDll.Stop_Playing(nBoardNum);
    else if (gpConfig->gBC[nBoardNum].bRecordInProgress == true)
        gWrapDll.Stop_Recording(nBoardNum);
    else
    {
        if (gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC)
        {
			//kslee 2010/4/6
			if(gGeneral.gnVLCRunning < 0 && gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && gpConfig->gBC[nBoardNum].gnIPStreamingMode == RECV_IP_STREAM)
			{
	            if (gWrapDll.Start_Recording(nBoardNum) == 0)   // Success
		        {
			        char    str1[100];
				    gUtility.MyStrCpy(str1, 100, gUtilInd.szSecTimeToHMSformat(0));
					gUtility.MyStrCpy(str1,  100, gUtilInd.szMsecTimeToHMSdFormat(0));
					gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
					SetFileListFromDirectory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
				}
			}
			else
			{
				gUtilInd.LogMessageInt(TLV_SELECT_INPUT_SOURCE);
				return;
			}
			
        } else
        {
            if (gWrapDll.Start_Recording(nBoardNum) == 0)   // Success
            {
                char    str[100];
                gUtility.MyStrCpy(str, 100, gUtilInd.szSecTimeToHMSformat(0));
                gUtility.MyStrCpy(str,  100, gUtilInd.szMsecTimeToHMSdFormat(0));
                gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
                SetFileListFromDirectory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
            }
        }
    }
    SNMP_Send_Status(TVB390_START_RECORDING);
    SNMP_Send_Status(TVB390_START_PLAYING);
}
//------------------------------------------------------
void PlayForm::OnLbnSelchangeFilelistbox()
{
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
        return;

    gpConfig->gBC[gGeneral.gnActiveBoard].fCurFocus = FILELISTWINDOW;
	UpdateFileListDisplay();
	SNMP_Send_Status(TVB390_SELECT_LIST);
    SNMP_Send_Status(TVB390_FILE_LIST_NAME);
    SNMP_Send_Status(TVB390_FILE_LIST_INDEX);

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == QPSK || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_S2 ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == QAM_A || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T || 
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_H || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == MULTIPLE_DVBT)
	{
		Get_NIT_Delivery_Descriptor_Info();
	}
}
//------------------------------------------------------
void PlayForm::OnLbnSelchangePlaylist()
{
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
        return;

	UpdatePlayListDisplay();
        
    gpConfig->gBC[gGeneral.gnActiveBoard].fCurFocus = PLAYLISTWINDOW;
    SNMP_Send_Status(TVB390_SELECT_LIST);
    SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
	
	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == QPSK || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_S2 ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == QAM_A || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_T ||
		gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_H || gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == MULTIPLE_DVBT)
	{
		Get_NIT_Delivery_Descriptor_Info();
	}
}
//---------------------------------------------------------------

void PlayForm::OnEnChangeElaboutputfrequency(unsigned long dwFreq)
{
	char			strNum[25];
	double			dFreq;
	long nBoardNum = gGeneral.gnActiveBoard;

	if(dwFreq < 10000000 || dwFreq > 2150000000)
		return;

	gWrapDll.Set_RF_Frequency(gGeneral.gnActiveBoard, dwFreq);
	if(gpConfig->gBC[nBoardNum].gnSingleTone == 1)
		TVB380_SET_MODULATOR_SINGLE_TONE_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gnRFOutFreq, 1); 

	if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum))
	{
		Display_RF_Level_Range(1);
	}

	Set_Dac_I_Q_Offset();

	SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_USE_AMP);
	SNMP_Send_Status(TVB390_RF_LEVEL_MIN);
	SNMP_Send_Status(TVB390_RF_LEVEL_MAX);
	SNMP_Send_Status(TVB390_RF);
}
//---------------------------------------------------------------
void PlayForm::OnEnChangeRfOutputLevel(double dwRFLevel)
{
	// Set RF Level
	char	str[100];


	//AGC - RF Level -> Atten/AGC
	gpConfig->gBC[gGeneral.gnActiveBoard].gdwAttenVal = dwRFLevel;
	UpdateAgcUI(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gdwAttenVal, gpConfig->gBC[gGeneral.gnActiveBoard].gnAGC); 
	
    SNMP_Send_Status(TVB390_LEVEL);
	SNMP_Send_Status(TVB390_ATTEN);
	SNMP_Send_Status(TVB390_ATTEN_MIN);
	SNMP_Send_Status(TVB390_ATTEN_MAX);
}
//---------------------------------------------------------------
void PlayForm::OnEnChangeElabplayrate(int lPlayrate)
{
    // Change playrate
    long    nBoardNum = gGeneral.gnActiveBoard;
    if (lPlayrate != gpConfig->gBC[nBoardNum].gdwPlayRate)
    {
        if (lPlayrate < 1)
            gpConfig->gBC[nBoardNum].gdwPlayRate = 1;
        else
            gpConfig->gBC[nBoardNum].gdwPlayRate = lPlayrate;
		gpConfig->gBC[nBoardNum].gdwPlayRate = gWrapDll.Set_Play_Rate_Ex(nBoardNum, gpConfig->gBC[nBoardNum].gdwPlayRate, 
												gpConfig->gBC[nBoardNum].gnOutputClockSource);
		SNMP_Send_Status(TVB390_TS);
    }
}
//---------------------------------------------------------------
void PlayForm::OnEnChangeCnr(double dwCNR)
{
    gWrapDll.Set_CNR(gGeneral.gnActiveBoard, dwCNR);
    SNMP_Send_Status(TVB390_CNR);
}

//---------------------------------------------------------------
void PlayForm::OnEnChangeElaboutputsymrate(long dwSymbolrate)
{
    long    nBoardNum = gGeneral.gnActiveBoard;
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;


	if (dwSymbolrate == gpConfig->gBC[nBoardNum].gnSymbolRate)
	    return;
	//2010/7/18 I/Q PLAY/CAPTURE
	if(lModType == IQ_PLAY)
	{
		gpConfig->gBC[nBoardNum].gdwPlayRate = 32 * dwSymbolrate;
		Display_PlayRate(gpConfig->gBC[nBoardNum].gdwPlayRate);
	}

    if(gWrapDll.Set_Symbolrate(nBoardNum, dwSymbolrate) == FALSE)
		return;

	if (lModType == QAM_A || lModType == QAM_B || lModType == QPSK || lModType == DVB_S2 || lModType == IQ_PLAY || lModType == MULTIPLE_QAMB) //2010/10/5 I/Q PLAY/CAPTURE
    {
		SNMP_Send_Status(TVB390_SYMBOL_RATE);
    }
}

//---------------------------------------------------------------
void PlayForm::OnCbnSelchangeAdaptor(int index)
{
    if (index < 0)
        return;

    ChangeAdaptor(index);

	//2012/9/6 pcr restamp
	if(gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode))
	{
			TVB59x_SET_PCR_STAMP_CNTL_EX(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnPcrReStampingFlag);
	}
	SNMP_All_DataSend();
}

//---------------------------------------------------------------
void PlayForm::OnBnClickedBurstBitrate(long lCheck)
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    long    nBoardNum = gGeneral.gnActiveBoard;
    long    dwRate;

	if (lCheck == gpConfig->gBC[nBoardNum].gnOutputClockSource)
        return;
	gpConfig->gBC[nBoardNum].gnOutputClockSource = lCheck;
    
	//--- Set playrate
	if (lCheck)
    {
        gpConfig->gBC[nBoardNum].gnPrevPlayrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
        dwRate = gUtilInd.CalcBurtBitrate(nBoardNum);
        Display_PlayRate(dwRate);
    } else
    {
		if(gGeneral.gnBitrate > 0)
			Display_PlayRate(gGeneral.gnBitrate);
    }

    //--- Set symbolrate
	if((gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A || gpConfig->gBC[nBoardNum].gnModulatorMode ==QPSK ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2) && gpConfig->gBC[nBoardNum].gnDefaultSymbol == 1)
		gpConfig->gBC[nBoardNum].gnSymbolRate = gUtilInd.CalcSymbolRate(nBoardNum);
	gWrapDll.Set_Burst_Bitrate(nBoardNum, lCheck);
    SNMP_Send_Status(TVB390_TS_MAX);
	SNMP_Send_Status(TVB390_TS);
    SNMP_Send_Status(TVB390_SYMBOL_RATE);
}
//---------------------------------------------------------------

void PlayForm::OnBnClickedCalcSymbol(long lCheck)
{

	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    long            nBoardNum = gGeneral.gnActiveBoard;
	gpConfig->gBC[nBoardNum].gnDefaultSymbol = lCheck;
	if(lCheck)
    {
        gpConfig->gBC[nBoardNum].gnPrevSymbolrate = gpConfig->gBC[nBoardNum].gnSymbolRate;
        long dwRate = gUtilInd.CalcSymbolRate(nBoardNum);
        Display_SymbolRate(dwRate);
    } 
    SNMP_Send_Status(TVB390_SYMBOL_RATE_DEFAULT);
    SNMP_Send_Status(TVB390_SYMBOL_RATE);
}

void PlayForm::OnBnClickedComdir(char *strDirectory)
{
	long			nBoardNum = gGeneral.gnActiveBoard;
    char			str[256];
    char			text[256];
	char            temp[256];
	
	if (gpConfig->gBC[nBoardNum].bRecordInProgress == true ||
        gpConfig->gBC[nBoardNum].bPlayingProgress == true)
	{
        return;
	}

	if (strcmp(gpConfig->gBC[nBoardNum].gszMasterDirectory, strDirectory) != 0)        // differenct direcotry
    {
    	gpConfig->gBC[nBoardNum].nPlayListIndexCount = 0;
    }
    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszMasterDirectory, 256, strDirectory);
    SetFileListFromDirectory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
	

    SNMP_Send_Status(TVB390_FILE_LIST_NAME);
    SNMP_Send_Status(TVB390_PLAY_LIST_NAME);
    SNMP_Send_Status(TVB390_FILE_PATH);
    SNMP_Send_Status(TVB390_FILE_LIST_COUNT);
    SNMP_Send_Status(TVB390_PLAY_LIST_COUNT);
    SNMP_Send_Status(TVB390_FILE_LIST_INDEX);
    SNMP_Send_Status(TVB390_PLAY_LIST_INDEX);
}

//---------------------------------------------------------------
void PlayForm::OnCbnSelchangeIf(long tModulatorIFFreq)
{
	//2011/4/13 added
	if(gpConfig->gBC[gGeneral.gnActiveBoard].bRecordInProgress == true || gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == true) 
		return;

    long    nBoardNum = gGeneral.gnActiveBoard;

    if (tModulatorIFFreq == gpConfig->gBC[nBoardNum].gnIFOutFreq)
        return;

    gpConfig->gBC[nBoardNum].gnIFOutFreq = tModulatorIFFreq;
    gUtilInd.Adjust_Spectrum(nBoardNum);					// set gnSpectrumInverse according to IF, modtype

    gWrapDll.Configure_Modulator(0);
    SNMP_Send_Status(TVB390_IF);
	
}
//---------------------------------------------------------------
void PlayForm::OnCbnSelchangeModulatorType(int tModType)
{
    int     nBoardNum = gGeneral.gnActiveBoard;
    int     i;
    int     oldModulatorMode = gpConfig->gBC[nBoardNum].gnModulatorMode;
    
    //----------------------------------------------------------------
    //--- not selected, then return
    if (tModType < 0)
        return;

    //----------------------------------------------------------------
    //--- same one, then return
    if (tModType == gpConfig->gBC[nBoardNum].gnModulatorMode)
        return;
	
	//2012/4/13 SINGLE TONE
	gpConfig->gBC[nBoardNum].gnSingleTone = 0;

    //----------------------------------------------------------------
    //--- playing or recording, then return and restore old one
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
        gpConfig->gBC[nBoardNum].bRecordInProgress == true)
    {
        gUtilInd.LogMessageInt(TLV_INVALID_WHEN_PLAYING_OR_RECORDING);
        return;
    }

	//2012/4/5
	if(gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
	{
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB || gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			int nVirCnt;
			int RealVirSlot[4];
			TSPH_GetRealAndVirBdMap(nBoardNum, RealVirSlot);
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_VSB)
				nVirCnt = TSPL_CNT_MULTI_VSB_RFOUT_EX(nBoardNum);
			else if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_QAMB)
				nVirCnt = TSPL_CNT_MULTI_QAM_RFOUT_EX(nBoardNum);
			//2012/6/28 multi dvb-t
			else
				nVirCnt = TSPL_CNT_MULTI_DVBT_RFOUT_EX(nBoardNum);
//printf("[DEBUG +++], pos 3\n");	

			if(nVirCnt > 1)
			{
				for(i = 1; i < nVirCnt; i++)
				{
					if(gpConfig->gBC[RealVirSlot[i]].bPlayingProgress == true)
					{
                       gUtilInd.LogMessageInt(TLV_INVALID_WHEN_PLAYING_OR_RECORDING);
                       return;

					}
				}
			}
		}
	}

    //----------------------------------------------------------------
    // store playlist/filelist index
    SaveStreamListInfo(nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode);

	gpConfig->gBC[nBoardNum].nFileListIndexCur = -1;
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *)"");
	//2012/7/27 DVB-T2/C2 ASI IN
	if((gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC || gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC) && (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 ||
		gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 || gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S))
	{
		TSPL_SET_TSIO_DIRECTION_EX(nBoardNum, FILE_SRC);
		TSPH_START_MONITOR(nBoardNum, 0);
	}

	if(gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)
		TSPL_SET_BOARD_LED_STATUS_EX(nBoardNum, 1, 0);

    Sleep(300);
   //----------------------------------------------------------------
    // Change Modulator type. Set global variables
    gWrapDll.Change_Modulator_Type(nBoardNum, tModType);

	Sleep(300);
	if ( gWrapDll.m_Init == -1 )
	{
		gWrapDll.m_Init = 1;
		//gpConfig->gBC[nBoardNum].gnModulatorMode = oldModulatorMode;
		//return;
		tModType = oldModulatorMode;
		gWrapDll.Change_Modulator_Type(nBoardNum, tModType);
    		Sleep(300);
	}
	Set_ItemList_HW();


    UpdateModulatorConfigUI(nBoardNum);
    //--------------------------------------------------------------
    // if playlist is not exsit
    if (gpConfig->gBC[nBoardNum].nPlayListIndexCount <= 0)
    {
		if (gpConfig->gBC[nBoardNum].nFileListIndexCount <= 0)
			gpConfig->gBC[nBoardNum].nFileListIndexCur = -1;
		UpdateFileListDisplay();
    } else
    {
		UpdatePlayListDisplay();
    }
	if(gUtilInd.IsSupported_Mod_PcrRestamp_By_HW(gpConfig->gBC[nBoardNum].gnModulatorMode))
	{
			TVB59x_SET_PCR_STAMP_CNTL_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnPcrReStampingFlag);
	}

	if(gpConfig->gBC[gGeneral.gnActiveBoard].gnModulatorMode == DVB_S2)
	{
		gWrapDll.Set_Symbolrate(gGeneral.gnActiveBoard, gpConfig->gBC[gGeneral.gnActiveBoard].gnSymbolRate);
	}

	SNMP_All_DataSend();
}
//---------------------------------------------------------------------------
//       Add to playlist <= @filelistbox.listindex
void PlayForm::AddFile2PlayList()
{
    long    nBoardNum = gGeneral.gnActiveBoard;
    __int64 DblCurrentFileSize;
    char    str[256];
    int     i;
    int     nIndex;
    char    szFileName[256];
	int		nCount;
	nCount =  gpConfig->gBC[nBoardNum].nFileListIndexCount;
    if (nCount == 0)
    {
        gUtilInd.LogMessageInt(TLV_FILE_LIST_EMPTY);
    } else
    {
				gUtility.MyStrCpy(szFileName, 256, gpConfig->gBC[nBoardNum].szFileFileList[gpConfig->gBC[nBoardNum].nFileListIndexCur]);
                if(gpConfig->gBC[nBoardNum].nPlayListIndexCount < MAX_PLAY_LIST_COUNT)
                {
                    gUtility.MySprintf(str, 256, (char *)"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, szFileName);
                    
                    if (gUtility.Is_File_Exist(str) == true)
                    {
						DblCurrentFileSize = gUtilInd.Get_File_Size_BYTE(str);
                        if (DblCurrentFileSize > 1000)
                        {
							gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szPlayFileList[gpConfig->gBC[nBoardNum].nPlayListIndexCount++], 256,  szFileName);
                            UpdatePlayListDisplay();
                        } else
                        {
                        }
                    } else
                    {
                    }
                }
    }
    gUtilInd.CheckLoopAdaptation(nBoardNum);
}
//---------------------------------------------------------------------------
void PlayForm::RemovePlayList(long nIndex)
{
    int     nIdx;
    int     i;
    long    nBoardNum = gGeneral.gnActiveBoard;
	char	str[100];

	char    temp[MAX_PLAY_LIST_COUNT][256];
	int j;
	for(i = 0; i < MAX_PLAY_LIST_COUNT ; i++)
	{
		gUtility.MyStrCpy(temp[i],  256, (char *)"");
	}

    for(nIdx = gpConfig->gBC[nBoardNum].nPlayListIndexCount ; nIdx >= 0 ; nIdx--)
	{
        if(gpConfig->gBC[nBoardNum].nPlayListIndexCur == nIdx) 
        {
            nIndex = nIdx;
            if(nIndex >= 0 && nIndex < gpConfig->gBC[nBoardNum].nPlayListIndexCount)
			{
				j = 0;
                for (i = 0; i < gpConfig->gBC[nBoardNum].nPlayListIndexCount; i++)
				{
					if(i != nIndex)
					{
						gUtility.MyStrCpy(temp[j],  256, gpConfig->gBC[nBoardNum].szPlayFileList[i]);
						j++;
					}
				}
				gpConfig->gBC[nBoardNum].nPlayListIndexCount--;
				for(i = 0; i < j ; i++)
				{
					gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szPlayFileList[i],  256, temp[i]);
				}

				if(gpConfig->gBC[nBoardNum].nPlayListIndexCur >= gpConfig->gBC[nBoardNum].nPlayListIndexCount)            
					gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
                gpConfig->gBC[nBoardNum].nPlayListIndexDisplay  =  gpConfig->gBC[nBoardNum].nPlayListIndexCur;
            }
            if(gpConfig->gBC[nBoardNum].nPlayListIndexCount < 1)
                gpConfig->gBC[nBoardNum].fCurFocus  = FILELISTWINDOW;
			break;
		}
    }

    UpdatePlayListDisplay();
}		  
//---------------------------------------------------------------------------
void PlayForm::SetFileListFromDirectory(char *strDir)
{
	long	nBoardNum = gGeneral.gnActiveBoard;
	int		index = 0;
	// Set the File list and set the current index of file list
	
	//=========================================
	// test for opendir
	DIR	*dp;
	struct dirent *d;
	struct stat sb;
	char	strMsg[256];

	gpConfig->gBC[nBoardNum].nFileListIndexCount = 0;
	
	dp = opendir(gpConfig->gBC[nBoardNum].gszMasterDirectory);
	if (dp != NULL)
	{
		while ((d=readdir(dp)) != NULL)
		{
			sprintf(strMsg,"%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, d->d_name);
			stat(strMsg, &sb);
			{
				if (S_ISREG(sb.st_mode))
				{
					if (index < MAX_PLAY_LIST_COUNT)
					{
						if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
						{
							if(strstr(d->d_name, ".mmx") || strstr(d->d_name, ".mfs"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
							else if(strstr(d->d_name, ".MMX") || strstr(d->d_name, ".MFS"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}

						}
						else if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
						{
							if(strstr(d->d_name, ".ni") || strstr(d->d_name, ".na") || strstr(d->d_name, ".eti"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
							else if(strstr(d->d_name, ".NI") || strstr(d->d_name, ".NA") || strstr(d->d_name, ".ETI"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
						}
						else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
						{
							if(strstr(d->d_name, ".t2mc") || strstr(d->d_name, ".T2MC"))
							{
								continue;
							}
							else if(strstr(d->d_name, ".t2") || strstr(d->d_name, ".T2") || strstr(d->d_name, ".t2mi") || strstr(d->d_name, ".T2MI"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
								
							}
							else if(strstr(d->d_name, ".trp") || strstr(d->d_name, ".ts") || strstr(d->d_name, ".atsc") || strstr(d->d_name, ".dtv") || strstr(d->d_name, ".mpg") || strstr(d->d_name, ".mpeg"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
							else if(strstr(d->d_name, ".TRP") || strstr(d->d_name, ".TS") || strstr(d->d_name, ".ATSC") || strstr(d->d_name, ".DTV") || strstr(d->d_name, ".MPG") || strstr(d->d_name, ".MPEG"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}

						}
						//2010/11/18 TVB593
						else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
						{
							if(strstr(d->d_name, ".iq") || strstr(d->d_name, ".IQ"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
						}
						else
						{
							if(strstr(d->d_name, ".t2mi") || strstr(d->d_name, ".T2MI"))
							{
								continue;
							}
							else if(strstr(d->d_name, ".trp") || strstr(d->d_name, ".ts") || strstr(d->d_name, ".atsc") || strstr(d->d_name, ".dtv") || strstr(d->d_name, ".mpg") || strstr(d->d_name, ".mpeg"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
							else if(strstr(d->d_name, ".TRP") || strstr(d->d_name, ".TS") || strstr(d->d_name, ".ATSC") || strstr(d->d_name, ".DTV") || strstr(d->d_name, ".MPG") || strstr(d->d_name, ".MPEG"))
							{
								strcpy(gpConfig->gBC[nBoardNum].szFileFileList[index++], d->d_name);
							}
						}
					}
				}
			}
					
		}
		closedir(dp);
	}


	gpConfig->gBC[nBoardNum].nFileListIndexCount = index;
	qsort((void *)gpConfig->gBC[nBoardNum].szFileFileList, gpConfig->gBC[nBoardNum].nFileListIndexCount, 256, directorySort);

	//for(int jj = 0 ; jj < gpConfig->gBC[nBoardNum].nFileListIndexCount ; jj++)
	//{
	//	printf("[%d] : [%s]\n", jj, gpConfig->gBC[nBoardNum].szFileFileList[jj]);
	//}
	if (gpConfig->gBC[nBoardNum].nFileListIndexCur >= gpConfig->gBC[nBoardNum].nFileListIndexCount)
		gpConfig->gBC[nBoardNum].nFileListIndexCur = 0;
}

int directorySort(const void *cmp1, const void *cmp2)
{
	return (strcmp((char *)cmp1, (char *)cmp2));
}
//---------------------------------------------------------------------------
// Called from
//      Display_Init
//      After Configure_Modulator(1)
//          Combo_MODULATOR_TYPEChange()
//          Open_System()
//------------------------------------------------------
// Call following functions
//      gUtilInd.CheckLoopAdaptation()
//      gUtilInd.szSecTimeToHMSformat()
//      *** UpdateRFPowerLevelUI() --> gWrapDll.UpdateRFPowerLevel()
//      gUtilInd.CalcBurtBitrate()
//      gUtilInd.Adjust_Spectrum()
//      *** Display_Modulator_Parameter()
//------------------------------------------------------
// Description
//--- Reset elapsedtimer
//--- Restore Display (Buttons Captions, playrate)
//--- Component Hide: Parameters, bitrate, symbolrate, channel
//--- CheckLoopAdaptation
//--- Display fileinfo
//--- Display subloop
//--- Display and Set RFLevel
//--- Display CNR
//--- if not TDMB/ISDBT, Display MAX checkbox
//--- if IP Streaming & FILE SRC, display IP on ComF3
//--- Display IF
//--- Display ModulatorType
//--- Display Modulator Parameter : Display_Modulator_Parameter()
//--- Display BERT on ComF1

void PlayForm::UpdateModulatorConfigUI(long nBoardNum)
{
    int     i, index;

    //-----------------------------------------------------------------
    //--- List up file/play list :
    SetFileListFromDirectory(gpConfig->gBC[nBoardNum].gszMasterDirectory);
     //-----------------------------------------
    // File Ext (TDMB or Others)
    Update_File_Play_List();
   //-----------------------------------------

	//2012/3/27 TVB593 Multiple VSB/QAM-B
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		gpConfig->gBC[nBoardNum].gdwAttenVal = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gdwAttenVal;
		gpConfig->gBC[nBoardNum].gnRfLevel_Unit = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRfLevel_Unit;
	}


    //--- CheckLoopAdaptation
    gUtilInd.CheckLoopAdaptation(nBoardNum);
    //-----------------------------------------
    // Spectrum Adjust: Set gnSpectrumInverse
    gUtilInd.Adjust_Spectrum(nBoardNum);

    //-----------------------------------------------------------------
    //--- Add Modulator Type : gbEnabledType[]
    //-----------------------------------------------------------------
    index = 0;
	for (i = 0; i < MAX_MODULATORMODE; i++)
    {
        if (gpConfig->gBC[nBoardNum].gbEnabledType[i] == 1)
        {
            gpConfig->gBC[nBoardNum].giTypeComboMod[index] = i;
            index++;
        }
    }
    gpConfig->gBC[nBoardNum].giNumModulator = index;

    //-----------------------------------------------------------------
	if (gpConfig->gBC[gGeneral.gnActiveBoard].nPlayListIndexCount == 0)
	{
		gpConfig->gBC[gGeneral.gnActiveBoard].fCurFocus = FILELISTWINDOW;
	}
	else
	{
		gpConfig->gBC[gGeneral.gnActiveBoard].fCurFocus = PLAYLISTWINDOW;
	}

	long		lindex = -1;
	char		str[256];
	//--------------------------------------------------------------
    // if no playing list, then get file from filelist
	if (gpConfig->gBC[nBoardNum].nPlayListIndexCount == 0)
	{
		gpConfig->gBC[nBoardNum].nPlayListIndexStart = 0;
		gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
	    gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = 0;
		if(gpConfig->gBC[nBoardNum].nFileListIndexCount > 0)
		{
			lindex = gpConfig->gBC[nBoardNum].nFileListIndexCur;
			if (lindex < 0)
				lindex = 0;
			gUtility.MyStrCpy(str, 256, gpConfig->gBC[nBoardNum].szFileFileList[lindex]);
		    //2010/1/8
			if (gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false)
			{
				gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, str);
			}
		}
	} else
	{
		gpConfig->gBC[nBoardNum].nPlayListIndexStart = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
	    gpConfig->gBC[nBoardNum].nPlayListIndexDisplay = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
	    lindex = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
        if (lindex < 0)
			lindex = 0;
		gUtility.MyStrCpy(str, 256, gpConfig->gBC[nBoardNum].szPlayFileList[lindex]);
	    //2010/1/8
		if (gpConfig->gBC[gGeneral.gnActiveBoard].bPlayingProgress == false)
		{
		    gUtility.MySprintf(gpConfig->gBC[nBoardNum].szCurFileName, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, str);
		}
	}
	if(lindex >= 0)
		Display_File_Property(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);       // filename, packetsize, length, bitrate
    
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
	{
		//2012/6/28 multi dvb-t 
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == MULTIPLE_DVBT)
		{
			gpConfig->gBC[nBoardNum].gnRFOutFreq = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreq + ((6000000 + (1000000 * gpConfig->gBC[nBoardNum].gnBandwidth)) * gpConfig->gBC[nBoardNum].gn_StreamNum);
		}
		else
			gpConfig->gBC[nBoardNum].gnRFOutFreq = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnRFOutFreq + (6000000 * gpConfig->gBC[nBoardNum].gn_StreamNum);
	}
    Display_RF_CNR(nBoardNum);
    Display_Modulator_Parameter(nBoardNum);
	OnEnChangeElaboutputfrequency(gpConfig->gBC[nBoardNum].gnRFOutFreq);
	//2012/9/3 new rf level control
	if(gUtilInd.IsAttachedBdTyp_NewRFLevel_Cntl(nBoardNum) == 0)
		OnEnChangeRfOutputLevel(gpConfig->gBC[nBoardNum].gdwAttenVal);
	else
	{
		long iAmp_status;
		if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 0)
		{
			TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX( nBoardNum, gpConfig->gBC[nBoardNum].gnModulatorMode, gpConfig->gBC[nBoardNum].gdRfLevelValue, &iAmp_status, gpConfig->gBC[nBoardNum].gnUseTAT4710);
			gpConfig->gBC[nBoardNum].gnBypassAMP = iAmp_status;
		}
		else
		{
			Display_RF_Level_Range(1);
		}
	}
	//2011/3/30 DVB-T2 MULTI-PLP
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 || gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szDvbt2_Directory, 256, gpConfig->gBC[nBoardNum].gszMasterDirectory);
		for(i = 0 ; i < gpConfig->gBC[nBoardNum].nFileListIndexCount; i++)
		{
			gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szDvbt2_FileList[i], 256, gpConfig->gBC[nBoardNum].szFileFileList[i]);
		}
		SNMP_Send_Status(TVB390_DVB_T2_ISDB_S_DIRECTORY);
	}

	if(gpConfig->gBC[nBoardNum].bPlayingProgress == false && gpConfig->gBC[nBoardNum].bRecordInProgress == false)
	{
		TVB59x_SET_Output_TS_Type_EX(nBoardNum, gpConfig->gBC[nBoardNum].gnTsOutput_Mode);
		SNMP_Send_Status(TVB59x_TS_OUTPUT_SEL);
	}

} 
//---------------------------------------------------------------------------------------------------------
void PlayForm::UpdateFileListDisplay()
{
    char		str[256];
    char		strTemp[256];
    char		text[256];
    long		nBoardNum = gGeneral.gnActiveBoard;
	int			index = gpConfig->gBC[nBoardNum].nFileListIndexCur;
	int			nCount = gpConfig->gBC[nBoardNum].nFileListIndexCount;
    // Limit FileList ListIndex to valid range
    if (nCount < 1)
        return; // nothing to display

    // Adjust Filelist Index
    if (index < 0)        // let the index have valid value
		gpConfig->gBC[nBoardNum].nFileListIndexCur = 0;
    else if (index >= nCount)
		gpConfig->gBC[nBoardNum].nFileListIndexCur = nCount - 1;

	index = gpConfig->gBC[nBoardNum].nFileListIndexCur;

    //-----------------------------------
    gUtility.MySprintf(str, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory , gpConfig->gBC[nBoardNum].szFileFileList[index]);

	Display_File_Property(nBoardNum ,str);     // SetLabPlayrate + ShowFileSize

	if(gpConfig->gBC[nBoardNum].nPlayListIndexCount < 1)
	{
		gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, str);
	}

    //-----------------------------------
}

//---------------------------------------------------------------------------
// CHECKCHECK: PlayList
// Adjust playlist index
void PlayForm::UpdatePlayListDisplay()
{
    //       LCD:update file list display
    char		str[256];
	char		text[100];
    long		nBoardNum = gGeneral.gnActiveBoard;
	int			index;
	int			nCount =  gpConfig->gBC[nBoardNum].nPlayListIndexCount;
    if (nCount < 1)
        return; // nothing to display

    // Adjust Playlist Index
    if(gpConfig->gBC[nBoardNum].nPlayListIndexCur < 0)				// let the index have valid value
		gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
	else if(gpConfig->gBC[nBoardNum].nPlayListIndexCur >= nCount)
		gpConfig->gBC[nBoardNum].nPlayListIndexCur = nCount-1;

	index = gpConfig->gBC[nBoardNum].nPlayListIndexCur;
	gUtility.MySprintf(str, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory , gpConfig->gBC[nBoardNum].szPlayFileList[index]);
	Display_File_Property(nBoardNum ,str);     // SetLabPlayrate + ShowFileSize

	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szCurFileName, 256, str);

    //-----------------------------------
}
//---------------------------------------------------------------------------
void PlayForm::Update_File_Play_List()
{
    int     j, currentPos;
    long    nBoardNum = gGeneral.gnActiveBoard;
    char    szListFileName[256];
    char    szPlayFileName[256];
    char    str[256];
    FILE    *hFile = NULL;
    __int64 dwCurrentFileSize;
	int		nCount =  gpConfig->gBC[nBoardNum].nFileListIndexCount;
    //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // TS source file ext.
#ifdef STANDALONE
	gUtility.MySprintf(szListFileName, 256, (char *) "/sysdb/playlist-%d-%d", gpConfig->gBC[nBoardNum].gnModulatorMode, nBoardNum);
#else
    gUtility.MySprintf(szListFileName, 256, (char *) "%s/playlist-%d-%d", gpConfig->gBC[nBoardNum].gszMasterDirectory, gpConfig->gBC[nBoardNum].gnModulatorMode, nBoardNum);
#endif

    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // Set TS File List Index: Read file list index from playlist-n-n  
    gpConfig->gBC[nBoardNum].nFileListIndexCur = 0;

    if (gUtility.Is_File_Exist(szListFileName) == true)
    {
		hFile = gUtility.MyFopen(hFile, szListFileName, "r");
        if (hFile == NULL)
                return;

        gUtilInd.Get_One_Line_From_File(hFile, szPlayFileName, 256);         // FILE LIST INDEX
        currentPos = atoi(szPlayFileName);                              // FILE LIST INDEX
        fclose(hFile);

        for (j = 0; j < nCount; j++)
        {
            if (j == currentPos)
            {
                gpConfig->gBC[nBoardNum].nFileListIndexCur = j;
                break;
            }
        }
    }

    //--------------------------------------------------------------
    // TS Play List : Set Play List for this board
    gpConfig->gBC[nBoardNum].nPlayListIndexCur = 0;
    gpConfig->gBC[nBoardNum].nPlayListIndexCount = 0;
    if (gUtility.Is_File_Exist(szListFileName) == true)
    {
		hFile = gUtility.MyFopen(hFile, szListFileName, (char *) "r");
        if (hFile == NULL)
            return;
        gUtilInd.Get_One_Line_From_File(hFile, szPlayFileName, 256);         // FILE LIST INDEX
        gUtilInd.Get_One_Line_From_File(hFile, szPlayFileName, 256);         // PLAY LIST INDEX
        currentPos = atoi(szPlayFileName);      // PLAY LIST INDEX
        
        if (gpConfig->gBC[nBoardNum].bPlayingProgress == true)
		{
            currentPos = gpConfig->gBC[nBoardNum].nPlayListIndexDisplay;
		}
        
        j = 0;
        while(1)
        {
            if (gUtilInd.Get_One_Line_From_File(hFile, szPlayFileName, 256) < 0)
                break;
            gUtility.MySprintf(str, 256, (char *) "%s/%s", gpConfig->gBC[nBoardNum].gszMasterDirectory, szPlayFileName);
			if ( (gUtility.Is_File_Exist(str) == true) && (j < MAX_PLAY_LIST_COUNT) )
            {
                dwCurrentFileSize = gUtilInd.Get_File_Size_KB(str);
                if (dwCurrentFileSize > 1)      // 1KB
                {
                    gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].szPlayFileList[j], 256, szPlayFileName);
                    j = j + 1;
                }
            }
        }
        fclose(hFile);
        
        gpConfig->gBC[nBoardNum].nPlayListIndexCount = j;
        for (j = 0; j < gpConfig->gBC[nBoardNum].nPlayListIndexCount; j++)
        {
            if (j == currentPos)
                gpConfig->gBC[nBoardNum].nPlayListIndexCur = j;
        }
    }
}
//---------------------------------------------------------------------------
void PlayForm::Display_RF_CNR(long nBoardNum)
{

	OnCheckedBYPASSAMP(gpConfig->gBC[nBoardNum].gnBypassAMP);
	UpdateAgcUI(nBoardNum, gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gnAGC); 
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// called from
//      UpdateModulatorConfigUI
//------------------------------------------------------
// Call following functions
//      nothing
//------------------------------------------------------
// - Display Parameters
// - set symbolrate, spectrum inversion if default exist
//------------------------------------------------------
//					PARAM1		PARAM2		PARAM3		PARAM4		PARAM5		PARAM6		PARAM7		Check	Command		List
/*
    DVB_T = 0,		BAND		Const		Coderate	Tx			Guard		-		
    VSB_8,			-
    QAM_A,			Const
    QAM_B,			Const		Interleave
    QPSK,			Coderate	Spectrum	RRC Flter
    TDMB,			-
    VSB_16,			-
    DVB_H,			BAND		Const		Coderate	Tx			Guard		-			-			x		O			x
    DVB_S2,			Const		coderate	Pilot		Rolloff		Spectrum
    ISDB_T,			-			-			-			-			-			-			-			O		O			O
    ISDB_T_13,
    DTMB,			Const		Coderate	Interleave	FrameHeader	CarrierNum	FrameHeaerPN Pilot
*/
void PlayForm::Display_Modulator_Parameter(long nBoardNum)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
	char	str[256];
	//2011/12/2 TVB594
	if ((gpConfig->gBC[nBoardNum].gnBoardId >= 45 && gpConfig->gBC[nBoardNum].gnBoardId != _TVB594_BD_ID_) || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 16 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 0x1B)	/* 2012/1/31 TVB591S */ //2011/2/15 added 11(TVB597V2) //2010/10/5 added 0xF
	{
        if (lModType != TDMB && lModType != ISDB_T && lModType != ISDB_T_13)
        {
			//CMMB
			//DVB-T2 2010/3/25
			if(lModType == CMMB || lModType == DVB_T2 || lModType == ATSC_MH || lModType == ISDB_S)	//2010/12/06
			{
				gpConfig->gBC[nBoardNum].gnOutputClockSource = 1;
			}
			//2011/12/1 TVB594
			if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
			{
				gpConfig->gBC[nBoardNum].gnOutputClockSource = gpConfig->gBC[gpConfig->gBC[nBoardNum].gn_OwnerSlot].gnOutputClockSource;
			}
        }
    }
    switch (lModType)
    {
		//----------------------------------------------------------------------------------------------------------------------
		// DVB_T = 0,		BAND		Const		Coderate	Tx			Guard		-			-           x       x      x
		// DVB_H,			BAND		Const		Coderate	Tx			Guard		-			-			x		O  	   x
		case DVB_T:
        case DVB_H:
		case MULTIPLE_DVBT:
			//---------------------------------------------
            if (lModType != DVB_H)
            {
                if (gpConfig->gBC[nBoardNum].gnBandwidth > BW_8MHZ)
                    gpConfig->gBC[nBoardNum].gnBandwidth = BW_8MHZ;
                if (gpConfig->gBC[nBoardNum].gnTxmode > TX_8K)
                    gpConfig->gBC[nBoardNum].gnTxmode = TX_8K;
            }

			//---------------------------------------------
            //--- Coderate/Guard Interval/Constellation
            if (gpConfig->gBC[nBoardNum].gnCodeRate > 4)
                gpConfig->gBC[nBoardNum].gnCodeRate = 4;
            if (gpConfig->gBC[nBoardNum].gnConstellation > CONST_64QAM)
                gpConfig->gBC[nBoardNum].gnConstellation = CONST_64QAM;
            break;

		//----------------------------------------------------------------------------------------------------------------------
        // VSB_8/16:		nothing
		case VSB_8:
        case VSB_16:
		case ATSC_MH:
		case MULTIPLE_VSB:
			gpConfig->gBC[nBoardNum].gnSymbolRate = 4500000;
            if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 16 ||
				gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 0x1B)	/* 2012/1/31 TVB591S */ //2011/2/15 added 11(TVB597V2) //2010/10/5 added 0xF //44,45,47,48,59,60,10(595D),20(590S)
            {
                if (lModType == VSB_16)
                {
                    gpConfig->gBC[nBoardNum].gdwPlayRate = 38785316;
                    Display_PlayRate(gpConfig->gBC[nBoardNum].gdwPlayRate);
                }
            } else if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            else
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = 0;
            break;

		//----------------------------------------------------------------------------------------------------------------------
		// QAM_A,			Const
		// QAM_B,			Const		Interleave
        case QAM_A:
        case QAM_B:
		case MULTIPLE_QAMB:
			if (lModType != QAM_A)
            {
                if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
                    gpConfig->gBC[nBoardNum].gnSymbolRate = 5056941;
                else
                    gpConfig->gBC[nBoardNum].gnSymbolRate = 5360537;
            }
			Display_SymbolRate(gpConfig->gBC[nBoardNum].gnSymbolRate);
            break;

		//----------------------------------------------------------------------------------------------------------------------
		//QPSK,			Coderate	Spectrum	RRC Flter
        case QPSK:
			break;

		//----------------------------------------------------------------------------------------------------------------------
		//DVB_S2,			Const		coderate	Pilot		Rolloff		Spectrum
        case DVB_S2:
			break;

		//----------------------------------------------------------------------------------------------------------------------
		//ISDB_T,			-			-			-			-			-			-			-			O		O    O
		//ISDB_T_13,
        case ISDB_T:
        case ISDB_T_13:
            gpConfig->gBC[nBoardNum].gnSymbolRate = 8126984;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            break;

		//----------------------------------------------------------------------------------------------------------------------
		//TDMB,			-
        case TDMB:
            gpConfig->gBC[nBoardNum].gdwPlayRate = 2433331;
            Display_PlayRate(gpConfig->gBC[nBoardNum].gdwPlayRate);
            gpConfig->gBC[nBoardNum].gnSymbolRate = 2048000;
            break;

		//----------------------------------------------------------------------------------------------------------------------
 		//DTMB,			Const		Coderate	Interleave	FrameHeader	CarrierNum	FrameHeaerPN Pilot
		case DTMB:
			if (gpConfig->gBC[nBoardNum].gnConstellation > CONST_DTMB_64QAM)
                gpConfig->gBC[nBoardNum].gnConstellation = CONST_DTMB_64QAM;
			if (gpConfig->gBC[nBoardNum].gnCodeRate > CONST_DTMB_CODE_7488_6016)
                gpConfig->gBC[nBoardNum].gnCodeRate = CONST_DTMB_CODE_7488_6016;
            if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM_NR ||
                gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_32QAM)
            {
                gpConfig->gBC[nBoardNum].gnCodeRate = CONST_DTMB_CODE_7488_6016;
			}
			if (gpConfig->gBC[nBoardNum].gnQAMInterleave > CONST_DTMB_INTERLEAVE_1)
                gpConfig->gBC[nBoardNum].gnQAMInterleave = CONST_DTMB_INTERLEAVE_1;
			if (gpConfig->gBC[nBoardNum].gnFrameHeader > CONST_DTMB_FRAME_HEADER_MODE_3)
                gpConfig->gBC[nBoardNum].gnFrameHeader = CONST_DTMB_FRAME_HEADER_MODE_3;
			if (gpConfig->gBC[nBoardNum].gnCarrierNumber > CONST_DTMB_CARRIER_NUMBER_1)
                gpConfig->gBC[nBoardNum].gnCarrierNumber = CONST_DTMB_CARRIER_NUMBER_1;

            gpConfig->gBC[nBoardNum].gnSymbolRate = 7560000;
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1;
            break;
		case CMMB:
			//MD_IF
			UpdateCmmbUI(nBoardNum, gpConfig->gBC[nBoardNum].szCurFileName);
			if(gpConfig->gBC[nBoardNum].gnCMMB_Params_Count > 0)
			{
				OnCbnSelchangeParam1(0);
			}
			SNMP_Send_Status(TVB390_CMMB_MDIF);
			SNMP_Send_Status(TVB390_CMMB_CONSTELLATION);
			SNMP_Send_Status(TVB390_CMMB_RSCODING);
			SNMP_Send_Status(TVB390_CMMB_BYTECROSSING);
			SNMP_Send_Status(TVB390_CMMB_LDPC);
			SNMP_Send_Status(TVB390_CMMB_SCRAMBLE);
			SNMP_Send_Status(TVB390_CMMB_TIMESLICE);
			SNMP_Send_Status(TVB390_CMMB_MDIF_ITEM);
			break;
		case DVB_T2:
			break;
		//I/Q PLAY/CAPTURE
		case IQ_PLAY:
			break;
	//2010/12/06 ISDB-S ================================================================================================================================
		case ISDB_S:
			gpConfig->gBC[nBoardNum].gnSymbolRate = 28860000;
			gpConfig->gBC[nBoardNum].gnSpectrumInverse = 1; //???
			if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK)
			{
				gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_1_2;
			}
			else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
			{
				gpConfig->gBC[nBoardNum].gnCodeRate = CONST_ISDBS_CODE_2_3;
			}
			OnCbnSelchangeParam2(gpConfig->gBC[nBoardNum].gnCodeRate);
			break;
	//==================================================================================================================================================
	//2011/2/23 DVB-C2 =============================================================================================================================
		case DVB_C2:
			break;
	//==============================================================================================================================================
	}
}


//---------------------------------------------------------------
void PlayForm::UpdateCmmbUI(long nBoardNum, char *szListFileName)
{
	char szResult[4096];
	unsigned int i, j; 
	
	if(strlen(szListFileName) < 1)
		return;
	//2011/2/14
	for(i = 0 ; i < MAX_CMMB_TIME_SLOT_COUNT; i++)
	{
		for(j = 0; j < MAX_CMMB_PARAM_COUNT; j++)
		{
			gpConfig->gBC[nBoardNum].gnCMMB_Params[i][j] = -1;
		}
	}
	gpConfig->gBC[nBoardNum].gnCMMB_Params_Count = 0;
	//MF_ID
	for(i=0; i<4096;i++)
		szResult[i] = 0;

	TSPH_RUN_MFS_PARSER(nBoardNum, szListFileName, szResult);
	long ii, jj;
	char temp[4096];
	char param[16];
	long index1, index2;
	ii = 0;
	index1 = 0;
	for(i = 0; i < strlen(szResult); i++)
	{
		if(szResult[i] == ',')
		{
			temp[ii] = '\0';
			jj = 0;
			index2 = 0;
			for(j = 0; j < strlen(temp); j++)
			{
				if(temp[j] == ' ')
				{
					param[jj] = '\0';
					long a = atol(param);
					if(a < 0)
					{
						jj = 0;
						index2++;
						continue;
					}
					gpConfig->gBC[nBoardNum].gnCMMB_Params[index1][index2] = atol(param);
					jj = 0;
					index2++;
					continue;
				}
				param[jj++] = temp[j];
			}
			ii = 0;
			index1++;
			continue;
		}
		temp[ii++] = szResult[i];
	}
	gpConfig->gBC[nBoardNum].gnCMMB_Params_Count = index1;
	index1 = 0;

	//kslee 2010/4/16 JAVA TVBManager
	gpConfig->gBC[nBoardNum].gnCmmb_Mdif = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_const = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_rscoding = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_bytecrossing = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_ldpc = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_scramble = -1;
	gpConfig->gBC[nBoardNum].gnCmmb_timeslice = -1;
	char mdif[256];
	char temp_l[16];
	gUtility.MyStrCpy(mdif, 256, (char *)"");
	gUtility.MyStrCpy(temp_l, 16, (char *)"");
	//-------------------------------------------------
	//MF_ID
	for(ii = 0; ii < gpConfig->gBC[nBoardNum].gnCMMB_Params_Count; ii++)
	{
		if(ii < gpConfig->gBC[nBoardNum].gnCMMB_Params_Count -1)
		{
			sprintf(temp_l, "%d:", (int)gpConfig->gBC[nBoardNum].gnCMMB_Params[ii][7]);		
		}
		else
		{
			sprintf(temp_l, "%d", (int)gpConfig->gBC[nBoardNum].gnCMMB_Params[ii][7]);		
		}
		if(ii == 0)
			gUtility.MyStrCpy(mdif, 256, temp_l);
		else
			gUtility.MyStrCat(mdif,256, temp_l);
	}
	gUtility.MyStrCpy(gpConfig->gBC[nBoardNum].gszCmmb_Mdif, 256, mdif);	
}
#endif
