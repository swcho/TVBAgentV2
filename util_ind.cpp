#include "stdafx.h"
#if defined(WIN32)
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include "dos.h"
#include "main.h"
#include "resource.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include <iphlpapi.h>
#include "baseutil.h"
#include "Form_Status.h"
#else		// LINUX
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

//2010/6/22
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <net/if_arp.h>
#include	<sys/time.h>
#include <ctype.h>
#include "variable.h"
#include "hld_api.h"
#include "resource.h"
#include "reg_var.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include "msg_btw_mdls.h"
//---------------------------------------------------------------------------
// For SNMP

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define IFRSIZE   ((int)(size * sizeof (struct ifreq)))

#endif

extern APPLICATION_CONFIG      *gpConfig;
extern TEL_GENERAL_VAR          gGeneral;
extern CWRAP_DLL                gWrapDll;
extern CUTILITY                 gUtility;
extern char						gstrGeneral[256];

HANDLE                          ghMutex = NULL;        // defined in TVB590.cpp
CUTIL_IND                       gUtilInd;

#ifdef WIN32
using namespace System;
using namespace System::Diagnostics;
using namespace System::ComponentModel;
#else

//2012/1/12 USB/SDCARD mount check////////////////////////////////////////////////////////////////////////////
#ifdef STANDALONE
#define	FSRET_NO	0
#define	FSRET_YES	1


#define MAX_CHECK_USB_DEVICE		6
#define MAX_CHECK_SDCARD_DEVICE		3


static	char	*_mount_proc_file		= "/proc/mounts";

// USB Device
static	char	* usb_device_name[MAX_CHECK_USB_DEVICE] =
{
	"/dev/sda",
	"/dev/sdb",
	"/dev/sdc",
	"/dev/sdd",
	"/dev/sde",
	"/dev/sdf",
};
static	char	* usb_mount_name = "/usb";

//SDCard Device
static	char	* sdcard_device_name[MAX_CHECK_SDCARD_DEVICE] =
{
	"/dev/mmcblk0",
	"/dev/mmcblk1",
	"/dev/mmcblk2"
};
static	char	* sdcard_mount_name = "/sdcard";
#endif
////////////////////////////////////////////////////////////////////////////
#endif

//---------------------------------------------------------------------------
CUTIL_IND* gc_util_modulator = NULL;

//---------------------------------------------------------------------------
CUTIL_IND::CUTIL_IND()
{
}

//2012/1/12 USB/SDCARD mount check////////////////////////////////////////////////////////////////////////////
#ifdef STANDALONE
int	CUTIL_IND::chk_existence_string(FILE *_file, char *key_str, char *key_str_opt)
{
	char	rd_text[256];
	static	int	prt_interval1 = 0, prt_interval2 = 0;

	if ((_file == NULL) || (key_str == NULL))
	{
		return	FSRET_NO;
	}

	fseek(_file, 0, SEEK_SET);
	while (1)
	{
		if (fgets(rd_text, 256, _file) != NULL)
		{
			if (strstr(rd_text, key_str) != NULL)	//	match key-str
			{
				if (key_str_opt != NULL)
				{
					if (strstr(rd_text, key_str_opt) != NULL)	//	match key-str-opt
					{
						if (prt_interval1++ > 91)
						{
							prt_interval1 = 0;
//							printf("[fs-mgr] chk-string : [%s][%s]\n", key_str, key_str_opt);
						}
						return	FSRET_YES;
					}
				}
				else	//	match key-str, check only key-str.
				{
					if (prt_interval2++ > 91)
					{
						prt_interval2 = 0;
//						printf("[fs-mgr] chk-string : [%s]\n", key_str);
					}
					return	FSRET_YES;
				}
			}
		}
		else
		{
			break;
		}
	}

	return	FSRET_NO;
}
// USB Device
int	CUTIL_IND::check_usb_dev_attached(int device)
{
	int	ret;

	int	fd;

	ret = FSRET_NO;
	fd = open(usb_device_name[device], O_RDONLY);
	if (fd != -1)
	{
		close(fd);
		ret = FSRET_YES;
	}
	return	ret;
}
int	CUTIL_IND::check_usb_fs_mounted(int device)
{
	int	ret;
	FILE	*mnt_file;

	mnt_file = fopen(_mount_proc_file, "r");
	ret = chk_existence_string(
		mnt_file,
		usb_device_name[device],
		usb_mount_name);
	fclose(mnt_file);

	return	ret;
}
// SDCard Device
int	CUTIL_IND::check_sdcard_dev_attached(int device)
{
	int	ret;

	int	fd;

	ret = FSRET_NO;
	fd = open(sdcard_device_name[device], O_RDONLY);
	if (fd != -1)
	{
		close(fd);
		ret = FSRET_YES;
	}
	return	ret;
}
int	CUTIL_IND::check_sdcard_fs_mounted(int device)
{
	int	ret;
	FILE	*mnt_file;

	mnt_file = fopen(_mount_proc_file, "r");
	ret = chk_existence_string(
		mnt_file,
		sdcard_device_name[device],
		sdcard_mount_name);
	fclose(mnt_file);

	return	ret;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------
// decide gpConfig->gBC[nBoardNum].gnModulatorMode
// 0=DVB-T ~ 11=DTMB (DVBT,VSB,QAMA,QAMB,QPSK,  TDMB,VSB16,DVBH,DVBS2,ISDBT,   ISDBT13,DTMB)
void CUTIL_IND::SyncModulatorMode(long nBoardNum)
{
    int     iType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    int     i;

    //------------------------------------------------------------------------
    // Get Enabled Modulator Type. set gpConfig->gBC[nBoardNum].gbEnabledType[i]
    gWrapDll.Get_Enabled_Modulator_Type(nBoardNum);

    //------------------------------------------------------------------------
    // Set modulator type as enabled type
    if (gpConfig->gBC[nBoardNum].gbEnabledType[iType] == 0)   // Not Enabled
    {
        for (i = 0; i < MAX_MODULATORMODE; i++)
        {
            //iType = (iType + 1) % MAX_MODULATORMODE;
            if (gpConfig->gBC[nBoardNum].gbEnabledType[i] == 1)   // Enabled
            {
                gpConfig->gBC[nBoardNum].gnModulatorMode = i;
                return;
            }

        }
    } else
        return;
    //gpConfig->gBC[nBoardNum].gnModulatorMode = VSB_8;
}

//---------------------------------------------------------------------------
// First part of Form_Load (FormShow)
// - check if no_multi_board file exist
// - check if this program already started
bool CUTIL_IND::Check_Multi_Board()
{
    char    szFile[256];
    FILE    *hFile = NULL;
    DWORD   dwRet;

   //----------------------------------------------------
    // Check if file exist
#ifdef WIN32
    gUtility.MySprintf(szFile, 256, "%s\\no_multi_board", gGeneral.gnStrCurDir);
#else
    gUtility.MySprintf(szFile, 256, (char *) "%s/no_multi_board", gGeneral.gnStrCurDir);
#endif
	hFile = gUtility.MyFopen(hFile, szFile,"r");
    if (hFile)
    {
        gGeneral.gnMultiBoardUsed = false;
        fclose(hFile);
    } else      // not exist
    {
        gGeneral.gnMultiBoardUsed = true;
    }

    //----------------------------------------------------
    // Check if it is already started
#if defined(WIN32)
    ghMutex = CreateMutex(NULL, 0, "MY_TVB0590");
    dwRet = GetLastError();
    if (dwRet == ERROR_ALREADY_EXISTS)
    {
        //----------------------------------------------------------------------------
        //20070309-NO MULTI-BOARD
        //----------------------------------------------------------------------------
        if (gGeneral.gnMultiBoardUsed == true)      
        {
            return false;
        }
    }
#endif

    return true;
}

//---------------------------------------------------------------------------
long CUTIL_IND::AdjustBankOffset(long nBoardNum, long playRate)
{
    long    nBankNumber;
    long    nBankOffset;
    long    lRet;

    nBankNumber = gpConfig->gBC[nBoardNum].gnSubBankNumber;
    nBankOffset = gpConfig->gBC[nBoardNum].gnSubBankOffset;
    lRet = nBankOffset;
    
    if (playRate > 0)
    {
        long    offset;
        long    newbankoffset;
        int     i;

        //--- 2.0 sec wait
        offset = (long) (  (2.0 * playRate)/(1024.0*8.0*(nBankNumber+1.0))  );
     
        //--- Change to 2**n
        newbankoffset = 1;
        for (i = 0; i <= 10; i++)
        {
            offset = offset/2;
            if (offset < 1)
                break;
            newbankoffset = newbankoffset * 2;
        }

        lRet = newbankoffset;
        if (lRet < MIN_BANK_OFFSET)
            lRet = MIN_BANK_OFFSET;

        if (lRet > MAX_BANK_OFFSET)
            lRet = MAX_BANK_OFFSET;
#ifdef STANDALONE
	if ( playRate > 10000000 ) lRet = MAX_BANK_OFFSET;
	else if ( playRate > 6000000 ) lRet = MAX_BANK_OFFSET/2;
	else if ( playRate > 3000000 ) lRet = MAX_BANK_OFFSET/4;
	else if ( playRate > 2000000 ) lRet = MAX_BANK_OFFSET/8;
	else			       lRet = MAX_BANK_OFFSET/16;
#endif
    }

    return lRet;
}

//---------------------------------------------------------------------------
// calculrate burst birate according to parameters and symbolrate
long CUTIL_IND::CalcBurtBitrate(long nBoardNum)
{
    long            lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    double			Bandwidth;
    double          CodeRate;
    double			Constellation;
    double          GuardInterval;
    long            nCalcBurtBitrate = 100000;

    // 423 / 544 * TBandwidth * Tcode_rate * TConstellation * TGuardInterval (bps)
    if (lModType == DVB_T || lModType == DVB_H || lModType == MULTIPLE_DVBT)	//2012/6/28 multi dvb-t
    {
        //--- bandwidth
        if (lModType == DVB_H)
		{
			Bandwidth = (gpConfig->gBC[nBoardNum].gnBandwidth + 5) * 1000000;
		}
        else
            Bandwidth = (gpConfig->gBC[nBoardNum].gnBandwidth + 6) * 1000000;

        //--- Coderate
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            CodeRate = 1.0/2.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            CodeRate = 2.0/3.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            CodeRate = 3.0/4.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            CodeRate = 5.0/6.0;
        else
            CodeRate = 7.0/8.0;

        //--- constellation
        Constellation = gpConfig->gBC[nBoardNum].gnConstellation*2 + 2;

        //--- Guard Interval
        if (gpConfig->gBC[nBoardNum].gnGuardInterval == 0)
            GuardInterval = 4.0/5.0;
        else if (gpConfig->gBC[nBoardNum].gnGuardInterval == 1)
            GuardInterval = 8.0/9.0;
        else if (gpConfig->gBC[nBoardNum].gnGuardInterval == 2)
            GuardInterval = 16.0/17.0;
        else
            GuardInterval = 32.0/33.0;
            
        nCalcBurtBitrate = (long) (423.0/544.0 * Bandwidth * CodeRate * Constellation * GuardInterval);
    //ATSC-M/H kslee 2010/2/3
	} else if (lModType == VSB_8 || lModType == ATSC_MH || lModType == MULTIPLE_VSB)
    {
        nCalcBurtBitrate = 19392999;
    } else if (lModType == QAM_A)
    {
        nCalcBurtBitrate = (long) (((double)gpConfig->gBC[nBoardNum].gnSymbolRate/(204.0/188.0)) * (gpConfig->gBC[nBoardNum].gnQAMMode + 4));
    } else if (lModType == QAM_B || lModType == MULTIPLE_QAMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
            nCalcBurtBitrate = (long) ((double)gpConfig->gBC[nBoardNum].gnSymbolRate*6*(122.0/128.0)*(53760.0/53802.0)*(14.0/15.0));
        else
            nCalcBurtBitrate = (long) ((double)gpConfig->gBC[nBoardNum].gnSymbolRate*8*(122.0/128.0)*(78848.0/78888.0)*(19.0/20.0));
    } else if (lModType == QPSK)
    {
        //--- Coderate
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            CodeRate = 1.0/2.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            CodeRate = 2.0/3.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            CodeRate = 3.0/4.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            CodeRate = 5.0/6.0;
        else
            CodeRate = 7.0/8.0;
            
        nCalcBurtBitrate = (long) ((gpConfig->gBC[nBoardNum].gnSymbolRate*2/(204.0/188.0))*CodeRate);
    } else if (lModType == DVB_S2)
    {
        nCalcBurtBitrate = CalcBurtBitrate_DVB_S2(nBoardNum);
    } else if (lModType == VSB_16)
    {
        nCalcBurtBitrate = 38785316;
    } else if (lModType == TDMB)
    {
        nCalcBurtBitrate = 2433331;
    } else if (lModType == ISDB_T || lModType == ISDB_T_13)
    {
        //vlc1.0.4
		//nCalcBurtBitrate = FIXED_PLAY_RATE_ISDB_T;
		nCalcBurtBitrate = MAX_PLAY_RATE_BOUNDARY_ISDB_T;
    } else if (lModType == DTMB)
    {
        double Const_val, Code_val, Mode_val;

        //--- Constellation
        if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM_NR)
            Const_val = 1.0;
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM)
            Const_val = 2.0;
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_16QAM)
            Const_val = 4.0;
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_32QAM)
            Const_val = 5.0;
        else
            Const_val = 6.0;

        //--- Coderate
        if (gpConfig->gBC[nBoardNum].gnCodeRate == CONST_DTMB_CODE_7488_3008)
            Code_val = 3008.0/ 7488.0;
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == CONST_DTMB_CODE_7488_4512)
            Code_val = 4512.0/ 7488.0;
        else
            Code_val = 6016.0/ 7488.0;
            
        //--- Frame Header
        if (gpConfig->gBC[nBoardNum].gnFrameHeader == CONST_DTMB_FRAME_HEADER_MODE_1)
            Mode_val = 4200.0;
		//2010/8/6
        else if (gpConfig->gBC[nBoardNum].gnFrameHeader == CONST_DTMB_FRAME_HEADER_MODE_2)
            Mode_val = 4375.0;
        else
            Mode_val = 4725.0;

        nCalcBurtBitrate = (long) ((double)gpConfig->gBC[nBoardNum].gnSymbolRate * Const_val * (double)(3744.0/ Mode_val) * Code_val);

    }
	else if(lModType == CMMB)
	{
		if(gpConfig->gBC[nBoardNum].gdwPlayRate > 1)
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
		else
			nCalcBurtBitrate = 5000000;
	}
	else if(lModType == DVB_T2)
	{
		if(gpConfig->gBC[nBoardNum].gdwPlayRate > 1)
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
		else
			nCalcBurtBitrate = 5000000;
	}
	//2010/12/07 ISDB-S ==========================================================================================================================
	else if(lModType == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
		else if(gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
			nCalcBurtBitrate = 52170000;
		else
			nCalcBurtBitrate = CalcDatarate(gpConfig->gBC[nBoardNum].gnConstellation, gpConfig->gBC[nBoardNum].gnCodeRate, MAX_SLOT_COUNT);				
	}
	else if(lModType == IQ_PLAY)
	{
		if(gpConfig->gBC[nBoardNum].gdwPlayRate > 1)
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
		else
			nCalcBurtBitrate = 200000000;
	}
	//============================================================================================================================================
	//2011/2/24 DVB-C2 ===========================================================================================================================
	else if(lModType == DVB_C2)
	{
		if(gpConfig->gBC[nBoardNum].gdwPlayRate > 1)
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
		else
			nCalcBurtBitrate = 5000000;
	}
	else if(lModType == ASI_OUTPUT_MODE)
	{
			nCalcBurtBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
	}
	//============================================================================================================================================
    return nCalcBurtBitrate;
}

//2010/12/07 ISDB-S =================================================================================================================================
long CUTIL_IND::CalcDatarate(long Constellation, int Coderate, int SlotCount)
{
	double SR = 0.0;
	double SE = 0.0;
	double CR = 0.0;
	SR = 28860000.0;

	if(Constellation == CONST_ISDBS_BPSK)
		SE = 1.0;
	else if(Constellation == CONST_ISDBS_QPSK)
		SE = 2.0;
	else if(Constellation == CONST_ISDBS_TC8PSK)
		SE = 3.0;
	else
		SE = 0.0;

	if(Constellation == CONST_ISDBS_QPSK)
	{
		if(Coderate == CONST_ISDBS_CODE_1_2)
			CR = 1.0 / 2.0;
		else if(Coderate == CONST_ISDBS_CODE_2_3)
			CR = 2.0 / 3.0;
		else if(Coderate == CONST_ISDBS_CODE_3_4)
			CR = 3.0 / 4.0;
		else if(Coderate == CONST_ISDBS_CODE_5_6)
			CR = 5.0 / 6.0;
		else if(Coderate == CONST_ISDBS_CODE_7_8)
			CR = 7.0 / 8.0;
		else
			CR = 0.0;
	}
	else if(Constellation == CONST_ISDBS_BPSK)
	{
		if(Coderate < 0)
			CR = 0.0;
		else
			CR = 1.0 / 2.0;
	}
	else if(Constellation == CONST_ISDBS_TC8PSK)
	{
		if(Coderate < 0)
			CR = 0.0;
		else
			CR = 2.0 / 3.0;
	}
	else
		CR = 0.0;

	if(SlotCount <= 0)
		SlotCount = 0;

	double slotrate = (double)SlotCount / (double)MAX_SLOT_COUNT;
	long nCalcDatarate;
#ifdef WIN32
	nCalcDatarate = (long)Math::Round((double)(SR * SE * CR * (double)(188.0 / 208.0) * slotrate));
#else
	nCalcDatarate = (long)round((double)(SR * SE * CR * (double)(188.0 / 208.0) * slotrate));
#endif
	return nCalcDatarate;
	
}
//===================================================================================================================================================

//---------------------------------------------------------------------------
long CUTIL_IND::CalcBurtBitrate_DVB_S2(long nBoardNum)
{
    double        Coderate;
    double        dwBitsPerSymbol;
    long          nCalcBurtBitrate = 100000;
    double        Kbch;
    double        Nbch;
    double        NumOfSLOT;
    double        PLFRAMING_effiency;

    if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
        {
            Coderate = 1.0/4.0;
            Kbch = 16008;
            Nbch = 16200;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
        {
            Coderate = 1.0/3.0;
            Kbch = 21408;
            Nbch = 21600;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
        {
            Coderate = 2.0/5.0;
            Kbch = 25728;
            Nbch = 25920;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
        {
            Coderate = 1.0/2.0;
            Kbch = 32208;
            Nbch = 32400;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
        {
            Coderate = 3.0/5.0;
            Kbch = 38688;
            Nbch = 38880;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 5)
        {
            Coderate = 2.0/3.0;
            Kbch = 43040;
            Nbch = 43200;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 6)
        {
            Coderate = 3.0/4.0;
            Kbch = 48408;
            Nbch = 48600;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 7)
        {
            Coderate = 4.0/5.0;
            Kbch = 51648;
            Nbch = 51840;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 8)
        {
            Coderate = 5.0/6.0;
            Kbch = 53840;
            Nbch = 54000;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 9)
        {
            Coderate = 8.0/9.0;
            Kbch = 57472;
            Nbch = 57600;
        } else
        {
            Coderate = 9.0/10.0;
            Kbch = 58192;
            Nbch = 58320;
        }
    } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
        {
            Coderate = 3.0/5.0;
            Kbch = 38688;
            Nbch = 38880;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
        {
            Coderate = 2.0/3.0;
            Kbch = 43040;
            Nbch = 43200;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
        {
            Coderate = 3.0/4.0;
            Kbch = 48408;
            Nbch = 48600;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
        {
            Coderate = 5.0/6.0;
            Kbch = 53840;
            Nbch = 54000;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
        {
            Coderate = 8.0/9.0;
            Kbch = 57472;
            Nbch = 57600;
        } else
        {
            Coderate = 9.0/10.0;
            Kbch = 58192;
            Nbch = 58320;
        }
    } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
        {
            Coderate = 2.0/3.0;
            Kbch = 43040;
            Nbch = 43200;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
        {
            Coderate = 3.0/4.0;
            Kbch = 48408;
            Nbch = 48600;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
        {
            Coderate = 4.0/5.0;
            Kbch = 51648;
            Nbch = 51840;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
        {
            Coderate = 5.0/6.0;
            Kbch = 53840;
            Nbch = 54000;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
        {
            Coderate = 8.0/9.0;
            Kbch = 57472;
            Nbch = 57600;
        } else
        {
            Coderate = 9.0/10.0;
            Kbch = 58192;
            Nbch = 58320;
        }
    } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
        {
            Coderate = 3.0/4.0;
            Kbch = 48408;
            Nbch = 48600;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
        {
            Coderate = 4.0/5.0;
            Kbch = 51648;
            Nbch = 51840;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
        {
            Coderate = 5.0/6.0;
            Kbch = 53840;
            Nbch = 54000;
        } else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
        {
            Coderate = 8.0/9.0;
            Kbch = 57472;
            Nbch = 57600;
        } else
        {
            Coderate = 9.0/10.0;
            Kbch = 58192;
            Nbch = 58320;
        }
    }

    dwBitsPerSymbol = 2.0 + gpConfig->gBC[nBoardNum].gnConstellation;    //2(QPSK), 3(8PSK), 4(16APSK), 5(32APSK)
        
    if (dwBitsPerSymbol == 2.0)
        NumOfSLOT = 360;
    else if (dwBitsPerSymbol == 3.0)
        NumOfSLOT = 240;
    else if (dwBitsPerSymbol == 4.0)
        NumOfSLOT = 180;
    else
        NumOfSLOT = 144;
                
    if (gpConfig->gBC[nBoardNum].gnPilot == 0)
        PLFRAMING_effiency = (90.0*NumOfSLOT)/(90.0*(NumOfSLOT + 1));
    else
        PLFRAMING_effiency = (90.0*NumOfSLOT)/((90.0*(NumOfSLOT + 1) + (36.0* (int)((NumOfSLOT-1)/16.0) )));

    nCalcBurtBitrate = (long) ((double)gpConfig->gBC[nBoardNum].gnSymbolRate*dwBitsPerSymbol*Coderate*PLFRAMING_effiency*((Kbch - 80.0)/ Nbch) );
    return nCalcBurtBitrate;
}


//---------------------------------------------------------------------------
// Calculate Symbolrate
// Check Symbolrate  (LabOutputSymRate_Change)
// Set gnSymbolRate
long CUTIL_IND::CalcSymbolRate(long nBoardNum)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;
    long    nSymRate=100;
    long    iBitrate;
	double	dwSymRate = 0;

    if (lModType == QAM_A)
    {
        // Sym = bitrate*(204/188)*(1/Mode), Mode=4,5,6,7,8 for 16,32,64,128,256QAM
        
		//nSymRate = (long) (((double)gpConfig->gBC[nBoardNum].gdwPlayRate * (204.0 / 188.0) / (double)(gpConfig->gBC[nBoardNum].gnQAMMode + 4)));
        dwSymRate = (((double)gpConfig->gBC[nBoardNum].gdwPlayRate * (204.0 / 188.0) / (double)(gpConfig->gBC[nBoardNum].gnQAMMode + 4)));
		nSymRate = (long)dwSymRate;
		if((nSymRate % 1000) > 0)
			nSymRate = (nSymRate / 1000) + 1;
		else
			nSymRate = (nSymRate / 1000);

		nSymRate = nSymRate * 1000;
    } else if (lModType == QAM_B || lModType == MULTIPLE_QAMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode == 0)
            nSymRate = 5056941;
        else
            nSymRate = 5360537;
    } else if (lModType == QPSK)
    {
        //Sym = (bitrate/2)*(204/188)*(1/Mode), Mode=1/2,2/3,3/4,5/6,7/8
        iBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
        
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
		if((nSymRate % 1000) > 0)
			nSymRate = (nSymRate / 1000) + 1;
		else
			nSymRate = (nSymRate / 1000);

        nSymRate = nSymRate * 1000;
    } else if (lModType == DVB_S2)
    {
        nSymRate = CalcSymbolRate_DVBS2(nBoardNum);
    }

    //gpConfig->gBC[nBoardNum].gnSymbolRate = nSymRate;
    return nSymRate;
}

//---------------------------------------------------------------------------
long CUTIL_IND::CalcSymbolRate_DVBS2(long nBoardNum)
{
    double PLFRAMING_effiency, Kbch, Nbch, NumOfSLOT;
    double dblCoderate;
    double dblSymbolrate, dblBitPerSym;
    long    iBitrate;
    long    nSymRate;

    switch (gpConfig->gBC[nBoardNum].gnConstellation)
    {
        case CONST_DVB_S2_QPSK: // qpsk
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            {
                dblCoderate = 1.0/4.0;
                Kbch = 16008.0;
                Nbch = 16200.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            {
                dblCoderate = 1.0/3.0;
                Kbch = 21408.0;
                Nbch = 21600.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            {
                dblCoderate = 2.0/5.0;
                Kbch = 25728.0;
                Nbch = 25920.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            {
                dblCoderate = 1.0/2.0;
                Kbch = 32208.0;
                Nbch = 32400.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
            {
                dblCoderate = 3.0/5.0;
                Kbch = 38688.0;
                Nbch = 38880.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 5)
            {
                dblCoderate = 2.0/3.0;
                Kbch = 43040.0;
                Nbch = 43200.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 6)
            {
                dblCoderate = 3.0/4.0;
                Kbch = 48408.0;
                Nbch = 48600.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 7)
            {
                dblCoderate = 4.0/5.0;
                Kbch = 51648.0;
                Nbch = 51840.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 8)
            {
                dblCoderate = 5.0/6.0;
                Kbch = 53840.0;
                Nbch = 54000.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 9)
            {
                dblCoderate = 8.0/9.0;
                Kbch = 57472.0;
                Nbch = 57600.0;
            }
            else
            {
                dblCoderate = 9.0/10.0;
                Kbch = 58192.0;
                Nbch = 58320.0;
            }

            NumOfSLOT = 360.0;
            break;
        }
        case CONST_DVB_S2_8PSK: //
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            {
                dblCoderate = 3.0/5.0;
                Kbch = 38688.0;
                Nbch = 38880.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            {
                dblCoderate = 2.0/3.0;
                Kbch = 43040.0;
                Nbch = 43200.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            {
                dblCoderate = 3.0/4.0;
                Kbch = 48408.0;
                Nbch = 48600.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            {
                dblCoderate = 5.0/6.0;
                Kbch = 53840.0;
                Nbch = 54000.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
            {
                dblCoderate = 8.0/9.0;
                Kbch = 57472.0;
                Nbch = 57600.0;
            }
            else
            {
                dblCoderate = 9.0/10.0;
                Kbch = 58192.0;
                Nbch = 58320.0;
            }

            NumOfSLOT = 240.0;
            break;
        }
        case CONST_DVB_S2_16APSK:
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            {
                dblCoderate = 2.0/3.0;
                Kbch = 43040.0;
                Nbch = 43200.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            {
                dblCoderate = 3.0/4.0;
                Kbch = 48408.0;
                Nbch = 48600.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            {
                dblCoderate = 4.0/5.0;
                Kbch = 51648.0;
                Nbch = 51840.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            {
                dblCoderate = 5.0/6.0;
                Kbch = 53840.0;
                Nbch = 54000.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
            {
                dblCoderate = 8.0/9.0;
                Kbch = 57472.0;
                Nbch = 57600.0;
            }
            else
            {
                dblCoderate = 9.0/10.0;
                Kbch = 58192.0;
                Nbch = 58320.0;
            }

            NumOfSLOT = 180.0;
            break;
        }
        case CONST_DVB_S2_32APSK:
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            {
                dblCoderate = 3.0/4.0;
                Kbch = 48408.0;
                Nbch = 48600.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            {
                dblCoderate = 4.0/5.0;
                Kbch = 51648.0;
                Nbch = 51840.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            {
                dblCoderate = 5.0/6.0;
                Kbch = 53840.0;
                Nbch = 54000.0;
            }
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            {
                dblCoderate = 8.0/9.0;
                Kbch = 57472.0;
                Nbch = 57600.0;
            }
            else
            {
                dblCoderate = 9.0/10.0;
                Kbch = 58192.0;
                Nbch = 58320.0;
            }

            NumOfSLOT = 144.0;
            break;
        }
    }

    if (gpConfig->gBC[nBoardNum].gnPilot == 0)   // Off
        PLFRAMING_effiency =  (90.0*NumOfSLOT)/(90.0*(NumOfSLOT+1));
    else
        PLFRAMING_effiency =  (90.0*NumOfSLOT)/( 90.0*(NumOfSLOT+1) + (36.0* (int)((NumOfSLOT-1.0)/16.0) ) );

    dblBitPerSym = gpConfig->gBC[nBoardNum].gnConstellation + 2.0;
    iBitrate = gpConfig->gBC[nBoardNum].gdwPlayRate;
    dblSymbolrate = iBitrate/(dblBitPerSym*dblCoderate*PLFRAMING_effiency*((Kbch-80.0)/Nbch));

    nSymRate = (long) dblSymbolrate;
	
	if((nSymRate % 1000) > 0)
		nSymRate = (nSymRate / 1000) + 1;
	else
		nSymRate = (nSymRate / 1000);

	nSymRate = nSymRate * 1000;

    return nSymRate;
}

//---------------------------------------------------------------------------
// Decide Spectrum inversion according to ModulatorType/RF Frequency
// called From Reg_Var::RestoreVariables
//             WrapDll::Set_RF_Frequency
//             Main::UpdateModulatorConfigUI
void CUTIL_IND::Adjust_Spectrum(long nBoardNum)
{
    long    lModType = gpConfig->gBC[nBoardNum].gnModulatorMode;

	//2011/11/28 TVB594
	if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
		return;

    //-----------------------------------------
    // Spectrum
    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
    {
        
		
		if(lModType == VSB_8 || lModType == MULTIPLE_VSB) 
		{
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20/* || gpConfig->gBC[nBoardNum].gnBoardId == 0xF || 
				gpConfig->gBC[nBoardNum].gnBoardId == 11 || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 0x16*/)
			{
                if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
                {
                    if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
                    else
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                } else
                {
                    if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
                    else
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                }
			}
		}
		//ATSC-M/H kslee 2010/2/3
		else if (lModType == VSB_16 || lModType == ATSC_MH)
        {
			if(gpConfig->gBC[nBoardNum].gnBoardId == 20)
			{
                if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
                {
                    if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
                    else
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                } else
                {
                    if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
                    else
                        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                }
			}
        }
        else if (lModType == TDMB)
        {
            if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
            {
                if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
                    gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                else
                    gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
            } else
            {
                if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1044000000)
                    gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
                else
                    gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
            }
        }
        else if (lModType == ISDB_T || lModType == ISDB_T_13 || lModType == DTMB)
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
#ifndef WIN32
		else if (lModType == DVB_T || lModType == DVB_H || lModType == QAM_A || lModType == QAM_B)		// yuricho. sometimes DVB-H/QAM_B spectrum changed to 1.
            gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
#endif
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 43)
    {
        gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
        // Disable SpctrumInverse combo box
    } else if (gpConfig->gBC[nBoardNum].gnBoardId == 41 || gpConfig->gBC[nBoardNum].gnBoardId == 42)
    {
        if (lModType == TDMB)
        {
            if (gpConfig->gBC[nBoardNum].gnRFOutFreq >= 1036000000)
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_NORMAL;
            else
                gpConfig->gBC[nBoardNum].gnSpectrumInverse = SPECTRUM_INVERSE;
        }
    }
}

//---------------------------------------------------------------------------
// Called From
//      Start Playing
//      Stop Playing
//      Stop Recording
// gpConfig->gBC[nBoardNum].szCurFileName should have the filename
void CUTIL_IND::Start_Stop_Playing(long nBoardNum, char *strfilename)
{
}

//---------------------------------------------------------------------------
// Called From
//      F3 press: Change to next input source
void CUTIL_IND::Set_Next_Input_Source(long nBoardNum)
{

	gWrapDll.SetStreamSourcePort(gpConfig->gBC[nBoardNum].gnModulatorSource);
}

//---------------------------------------------------------------------------
// Called From
//   lSource : FILE_SRC --> DVBASI_SRC --> SMPTE310M_SRC --> FILE_SRC
void CUTIL_IND::Set_Input_Source(long nBoardNum, long lSource)
{
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
        gpConfig->gBC[nBoardNum].bRecordInProgress == true ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
        return;     // can't change during playback/recording/TDMB

    //-----------------------------------------------------------------------
    // Set to FILE SRC <-- SMPTE310M_SRC
    if (lSource == FILE_SRC)
        gpConfig->gBC[nBoardNum].gnModulatorSource = SMPTE310M_SRC;
    else if (lSource == DVBASI_SRC)
        gpConfig->gBC[nBoardNum].gnModulatorSource = FILE_SRC;
    else
        gpConfig->gBC[nBoardNum].gnModulatorSource = DVBASI_SRC;

    Set_Next_Input_Source(nBoardNum);
}

//---------------------------------------------------------------------------
// playlist > 0  && (Restamping=1 ||gnDateTimeOffset != 0)
// ==> LOOP ADAPTION INVALID
void CUTIL_IND::CheckLoopAdaptation(long nBoardNum)
{
    if (gpConfig->gBC[nBoardNum].nPlayListIndexCount > 0 &&
        (gpConfig->gBC[nBoardNum].gnRestamping == 1 || gpConfig->gBC[nBoardNum].gnDateTimeOffset != 0) )
    {
        LogMessageInt(TLV_INVALID_LOOP_ADAPT);
    } else
        LogMessage("");
}

//---------------------------------------------------------------------------
void CUTIL_IND::LogMessage(char *strMessage)
{
    gUtility.MyStrCpy(gGeneral.szStatusMessage, 256, strMessage);
}

//---------------------------------------------------------------------------
void CUTIL_IND::LogMessageInt(int nCheck)
{
	char str[256];

#if defined(WIN32)
	if(gGeneral.gMultiLang == 1)
	{
		LoadString(GetModuleHandle(NULL), (1000 + nCheck), str, sizeof(str));
		LogMessage(str);
		return;
	}

#endif

	if (nCheck == TLV_NO_DRIVER)
        LogMessage("Fail to open device driver");
    else if (nCheck == TLV_UNKNOWN_ERR)
        LogMessage("Unknown error");
    else if (nCheck == TLV_NO_DEVICE)
        LogMessage("Device not found");
    else if (nCheck == TLV_NO_RBF)
        LogMessage("Fail:No EPLD file");
    else if (nCheck == TLV_FILE_READ_ERR)
        LogMessage("Corrupted EPLD file");
    else if (nCheck == TLV_DRV_ERR)
        LogMessage("Invalid EPLD status");
    else if (nCheck == TLV_DOWNLOAD_ERR)
        LogMessage("Fail to configure EPLD");
    else if (nCheck == TLV_DMAM_ALLOC_ERR)
        LogMessage("DMA memory alloc error");
    else if (nCheck == TLV_NO_TS_SYNC_ERR)
        LogMessage("No TS sync found");
    else if (nCheck == TLV_TOO_BIG_FILE_TO_WRITE_ERR)
        LogMessage("Too big file to write");
    else if (nCheck == TLV_FAIL_TO_START_PLAY_THREAD)
        LogMessage("Fail to start Play Thread");
    else if (nCheck == TLV_FAIL_TO_START_RECORD_THREAD)
        LogMessage("Fail to start Recorder");
    else if (nCheck == TLV_FAIL_TO_STOP_DRV)
       LogMessage("Fail to stop driver");
    else if (nCheck == TLV_NO_DRV_FOR_SET_SYS)
        LogMessage("No driver for SetMode");
    else if (nCheck == TLV_ALTERA_FILE_READ_ERR)
        LogMessage("EPLD config file corrupted");
    else if (nCheck == TLV_DEBUG_ERR)
        LogMessage("Debug error");
    else if (nCheck == TLV_FAIL_TO_CLOSE_TSP_DRV)
        LogMessage("Fail to close device driver");
    else if (nCheck == TLV_FAIL_TO_CREATE_LOG_FILE)
        LogMessage("Fail to create log file");
    else if (nCheck == TLV_PLAY_FILE_SIZE_IS_ZERO)
        LogMessage("File size is zero");
    else if (nCheck == TLV_FAIL_TO_CREATE_RECORD_FILE)
        LogMessage("Fail to create file");

    else if (nCheck == TLV_NO_FILE_TO_RECORD)
        LogMessage("No file to record");
    else if (nCheck == TLV_INVALID_ARGUMENT)
        LogMessage("Invalid argument to DLL");
    else if (nCheck == TLV_RS232C_TX_ERROR)
        LogMessage("RS232C transmitter failed. Check the selected COM port is available.");
    else if (nCheck == TLV_FAIL_TO_CONFIGURE_EPLD)
        LogMessage("Fail to configure EPLD");
    else if (nCheck == TLV_FAIL_TO_STOP_PLAY)
        LogMessage("Fail to stop playing");
    else if (nCheck == TLV_FAIL_TO_START_MONITOR_THREAD)
        LogMessage("Fail to start monitor");
    else if (nCheck == TLV_EXIT_SYSTEM)
        LogMessage("Exit main program (Debug)");
    else if (nCheck == TLV_FAIL_TO_START_IP_STREAMING)
        LogMessage("Fail to IP streaming. Check IP Reception configuration");
    else if (nCheck == TLV_INVALID_DTMB_PARAMS)
        LogMessage("It is unavailable parameter combination.");
    else if (nCheck == TLV_INVALID_LOOP_ADAPT)
        LogMessage("Restamping on Loop is unavailable if Play list is not empty.");

    else if (nCheck == TLV_INVALID_WHEN_PLAYING_OR_RECORDING)
        LogMessage("Can't be selected. Please, stop playing or recording.");
    else if (nCheck == TLV_INVALID_FRONT_KEY_INPUT)
        LogMessage("Invalid front key input.");
    else if (nCheck == TLV_FAIL_TO_SET_RF_LEVEL)
        LogMessage("Amplitude is invalid");
    else if (nCheck == TLV_FAIL_TO_SET_CNR)
        LogMessage("C/N is invalid.");
    else if (nCheck == TLV_FAIL_TO_CHECK_LICENSE)
        LogMessage("Fail to confirm modulator option. Check authorization key.");
    else if (nCheck == TLV_FAIL_TO_GET_FILE_SIZE)
        LogMessage("Fail to get file size.");
    else if (nCheck == TLV_FAIL_TO_SHOW_FILE_INFO)
        LogMessage("Fail to show file information.");
    else if (nCheck == TLV_READ_ONLY_FILE)
        LogMessage("It's read-only file.");
    else if (nCheck == TLV_FILE_NOT_FOUND)
        LogMessage("File  not found.");
    else if (nCheck == TLV_FAIL_TO_REMOVE_FILE)
        LogMessage("Fail to remove file.");

    else if (nCheck == TLV_FAIL_TO_REFESH_PLAY_LIST)
        LogMessage("Fail to refresh Play list.");
    else if (nCheck == TLV_FILE_LIST_EMPTY)
        LogMessage("File list is empty.");
    else if (nCheck == TLV_PLAY_LIST_FULL)
        LogMessage("Play list is full.");
    else if (nCheck == TLV_FAILT_TO_ADD_TO_PLAY_LIST)
        LogMessage("Fail to add to Play list.");
    else if (nCheck == TLV_EXCEED_LAYER_CAPACITY)
        LogMessage("Selected bitrate exceeds layer capacity.");
    else if (nCheck == TLV_INVALID_TMCC_PARAMETER)
        LogMessage("Invalid TMCC parameters. Check the total segments count.");
    else if (nCheck == TLV_LAYER_A_1_SEGMENT)
        LogMessage("Layer A must be 1 segment.");
    else if (nCheck == TLV_PARTIAL_RECEPTION_CHECK)
        LogMessage("Partial reception must be checked.");
    else if (nCheck == TLV_ONLY_LAYER_A_1_SEGMENT)
        LogMessage("Only Layer A must be 1 segment.");
    else if (nCheck == TLV_TOO_MANY_TASKS)
        LogMessage("Too many TASKs. Delete some TASKs.");

    else if (nCheck == TLV_INVALID_WAKE_UP_FORMAT)
        LogMessage("Wake-up/Exit must be hour:min.(00:00~23:59)" );
    else if (nCheck == TLV_INVALID_DURATION_TIME)
        LogMessage("Duration must be 1~min.");
    else if (nCheck == TLV_ASI_UNLOCKED)
        LogMessage("[DVB ASI] TS is not being input.");
    else if (nCheck == TLV_310M_UNLOCKED)
        LogMessage("[SMPTE 310M] TS is not being input.");
    else if (nCheck == TLV_FIFO_FULL)
        LogMessage("FIFO full detected.");
    else if (nCheck == TLV_FIFO_EMPTY)
        LogMessage("FIFO empty detected.");
    else if (nCheck == TLV_MODULATOR_TX_DISABLED)
        LogMessage("Modulator TX disabled.");
    else if (nCheck == TLV_9857_PLL_UNLOCKED)
        LogMessage("DAC(AD9775) PLL unlocked.");
    else if (nCheck == TLV_MODULATOR_UNLOCKED)
        LogMessage("Modulator TS sync. unlocked.");
    else if (nCheck == TLV_PLAY_FIFO_FULL)
        LogMessage("Play FIFO full detected.");

    else if (nCheck == TLV_FAIL_TO_DETECT_BOARD)
        LogMessage("Board is not available or application is already running.");
    else if (nCheck == TLV_FAIL_TO_BACKUP_LICENSE_DATA)
        LogMessage("Fail to backup authorization key.");
    else if (nCheck == TLV_FAIL_TO_UPDATE_LICENSE_DATA)
        LogMessage("Fail to apply authorization key.");
    else if (nCheck == TLV_REMOVE_FILE_CONFIRMED)
        LogMessage("File removed.");
    else if (nCheck == TLV_TMCC_UPDATED)
        LogMessage("TMCC parameters applied.");
    else if (nCheck == TLV_SELECT_A_WEEKDAYS)
        LogMessage("Select a weekdays.");
    else if (nCheck == TLV_SELECT_A_DAY_OF_MONTH)
        LogMessage("Select a Day of month.");
    else if (nCheck == TLV_INPUT_BITRATE)
        LogMessage("Input bitrate ");
    else if (nCheck == TLV_RECORDING)
        LogMessage("Recording ");
    else if (nCheck == TLV_RECODRING_STOPPED)
        LogMessage("Recording stopped.");

    else if (nCheck == TLV_PLAYING_STOPPED)
        LogMessage("Playing stopped.");
    else if (nCheck == TLV_DELAYING_STOPPED)
        LogMessage("Delaying stopped.");
    else if (nCheck == TLV_BUFERING)
        LogMessage("Buffering...");
    else if (nCheck == TLV_PLAYING)
        LogMessage("Playing...");
    else if (nCheck == TLV_WAITING)
        LogMessage("Wait...");
    else if (nCheck == TLV_INPUT_SOURCE_CHANGED)
        LogMessage("Input source changed.");
    else if (nCheck == TLV_ENTER_DELETE)
        LogMessage("Enter ""DELETE LIST"" or ""DELETE"" to delete from FILE LIST.");
    else if (nCheck == TLV_SELECT_INPUT_SOURCE)
        LogMessage("Select Input source");
    else if (nCheck == TLV_REMOVE_FILE_ABORTED)
        LogMessage("File remove aborted.");
    else if (nCheck == TLV_MOUSE_OPERATION_REQUIRED)
        LogMessage("Mouse operation is required.");

    else if (nCheck == TLV_TS_RATE_OUT_OF_RANGE)
        LogMessage("Playback rate is greater than maximum value.");
    else if (nCheck == TLV_LOOP_PLAY_MODE)
        LogMessage("Loop play mode");
    else if (nCheck == TLV_ONCE_PLAY_MODE)
        LogMessage("Once play mode");
    else if (nCheck == TLV_IP_RECV_OUTPUT_RATE)
        LogMessage("Before IP receiving, set playback rate as the expected IP input rate.");
    else if (nCheck == TLV_UPDATE_LICENSE_DATA)
        LogMessage("Authorization key applied.");
    else if (nCheck == TLV_OUT_OF_RF_RANGE)
        LogMessage("RF frequency is invalid." );
    else if (nCheck == TLV_SYMBOL_RATE_OUT_OF_RANGE)		// 20090812
        LogMessage("Symbol rate is invalid." );
    else
        LogMessage("Unknown error");
}

//---------------------------------------------------------------------------
// Set dwMsecCounter, dwLastTimeInMsec, dwStartTime
void CUTIL_IND::ResetElapsedtimeCounter(long nBoardNum, long dwStartOffseMsec)
{
    gpConfig->gBC[nBoardNum].dwMsecCounter = 0;
    gpConfig->gBC[nBoardNum].dwLastTimeInMsec = gpConfig->gBC[nBoardNum].dwStartTime;

    if (gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
    {
        __int64 dwPos;

        if (gpConfig->gBC[nBoardNum].gnCurrentPosChanged == 1)
            dwPos = gpConfig->gBC[nBoardNum].gnCurrentPos;
        else if (gpConfig->gBC[nBoardNum].gnStartPosChanged == 1)
            dwPos = gpConfig->gBC[nBoardNum].gnStartPos;
        else
            dwPos = dwStartOffseMsec;

        dwStartOffseMsec =(long) (( ((double)((double)dwPos/(double)gpConfig->gBC[nBoardNum].gdwPlayRate) * 8.0)) * 10.0);

		//kslee 2010/3/18
		//PCR RESTAMP
		if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
		{
			if(gGeneral.gnBitrate > 0)
			{
				dwStartOffseMsec = ((long)((double)dwPos / (double)gGeneral.gnBitrate * 8.0)) * 10;
			}
		}
		//2012/5/15
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
		{
			dwStartOffseMsec = (long)((double)dwStartOffseMsec / 6144.0 * 7300.0);
		}
    }

    gpConfig->gBC[nBoardNum].dwStartTime = GetCurrentTimeinMsec(nBoardNum) - dwStartOffseMsec;
}

//---------------------------------------------------------------------------
// return -1: TDMB mode
//        -2: File not exist
//        -3: Can't calculrate
// Description: set gnBitrate, gnPacketSize
#ifdef WIN32
int CUTIL_IND::SetLabPlayRateCalc(char *szFileName)      // Set  gnBitrate, gnPacketSize
{
    int     nBoardNum = gGeneral.gnActiveBoard;
    long    lRet;

    //--- TDMB --> return
    //if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    //    return -1;

    //--- File not exist --> return
    if (!gUtility.Is_File_Exist(szFileName))
        return -3;
	
	//DVB-T2 kslee 2010/4/20
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 1);     // packet size
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
			gGeneral.gnBitrate = -1;
			gGeneral.gnPacketSize = -1;
			return FILE_INVALID_FORMAT;
		}
		gGeneral.gnPacketSize = lRet;
		//2010/11/12 4097 -> 65526
		//char szResult[65526];
		//gUtility.MyStrCpy(szResult, 65526, "");
	
		if(gpConfig->gBC[nBoardNum].gnInputSource == FILE_SINGLE_IN || gpConfig->gBC[nBoardNum].gnInputSource == FILE_LIST_IN)
		{
			lRet = gWrapDll.TSPH_RUN_T2MI_PARSER(nBoardNum, szFileName, &(gpConfig->gBC[nBoardNum].gsT2mi_info));
			if(lRet <= 0)
			{
				gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
				gGeneral.gnBitrate = -1;
				return FILE_INVALID_BITRATE;
			}
			gGeneral.gnBitrate = lRet;
		}
		else if(gpConfig->gBC[nBoardNum].gnInputSource == REMUX_FILE_IN)
		{
			lRet = gWrapDll.TSPH_CAL_PLAY_RATE(nBoardNum, szFileName, 0);
			if(lRet <= 0)
			{
				gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
				gGeneral.gnBitrate = -1;
				return FILE_INVALID_BITRATE;
			}
			gGeneral.gnBitrate = lRet;
		}
	}
	//I/Q PLAY/CAPTURE
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{	
		//2011/11/16 IQ File 
		FILE *fp = NULL;
		unsigned char iq_data_info[8];
		fp = fopen(szFileName, "rb");
		if(fp != NULL)
		{
			fread(iq_data_info, 1, 8, fp);
			if(iq_data_info[0] == 'I' && iq_data_info[1] == 'Q' && iq_data_info[2] == '1' && iq_data_info[3] == '4')
			{
				gpConfig->gBC[nBoardNum].gnSymbolRate = (unsigned long)((iq_data_info[4] << 24) + (iq_data_info[5] << 16) + (iq_data_info[6] << 8) + iq_data_info[7]);
			}
			else
			{
				fclose(fp);
				gpConfig->gBC[nBoardNum].gnSymbolRate = 0;
				gGeneral.gnBitrate = -1;
				gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
				return FILE_INVALID_FORMAT;
			}
			fclose(fp);
		}
		gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
		lRet = 32 * gpConfig->gBC[nBoardNum].gnSymbolRate;
		gGeneral.gnBitrate = lRet;
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
	{
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate
		gpConfig->gBC[nBoardNum].gnETI_Format = lRet;
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
			gGeneral.gnBitrate = -1;
			lRet = FILE_INVALID_FORMAT;
			LogMessage("Invalid ETI format.");
		}
		else
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
			gGeneral.gnBitrate = 2433331;
		}
		return lRet;
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate
		gGeneral.gnPacketSize = -1;
		if(lRet == FILE_INVALID_FORMAT)
		{
			LogMessage("Invalid stream(CRC32 ERROR). Please check Multiplex Frame Header.");
			gpConfig->gBC[nBoardNum].gInvalidBitrate = -1;
			gGeneral.gnBitrate = -1;
		}
		else
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
			gGeneral.gnBitrate = lRet;
		}
		return lRet;
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		gpConfig->gBC[nBoardNum].gnCombinedTS = 0;
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 1);     // packet size
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
			gGeneral.gnBitrate = -1;
			gGeneral.gnPacketSize = -1;
			return FILE_INVALID_FORMAT;
		}
		gGeneral.gnPacketSize = lRet;
		if(gGeneral.gnPacketSize == 204 || gGeneral.gnPacketSize == 188)
		{
			if(gWrapDll.TSPH_IS_COMBINED_TS(nBoardNum, szFileName) == 0)
			{
				gpConfig->gBC[nBoardNum].gnCombinedTS = 1;
				lRet = gWrapDll.TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(nBoardNum, szFileName);
				if(lRet > 0)
				{
					gGeneral.gnBitrate = 52170000;
					gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
				}
				else
				{
					gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
					gGeneral.gnBitrate = -1;
					return FILE_INVALID_FORMAT;
				}
			}
			else
			{
				lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate
				if(lRet == -1)
				{
					gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
					gGeneral.gnBitrate = -1;
					return FILE_INVALID_BITRATE;
				}
				gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
				gGeneral.gnBitrate = lRet;
			}
		}
		else
		{
			lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate
			if(lRet == -1)
			{
				gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
				gGeneral.gnBitrate = -1;
				return FILE_INVALID_BITRATE;
			}
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
			gGeneral.gnBitrate = lRet;
		}
	}
	else
	{
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 1);     // packet size
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
			gGeneral.gnBitrate = -1;
			gGeneral.gnPacketSize = -1;
			return FILE_INVALID_FORMAT;
		}
		gGeneral.gnPacketSize = lRet;
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
			gGeneral.gnBitrate = -1;
			return FILE_INVALID_BITRATE;
		}
		gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
		gGeneral.gnBitrate = lRet;
	}
	return 0;
}
#else
//---------------------------------------------------------------------------
// return -1: TDMB mode
//        -2: File not exist
//        -3: Can't calculrate
// Description: set gnBitrate, gnPacketSize
int CUTIL_IND::SetLabPlayRateCalc(char *szFileName)      // Set  gnBitrate, gnPacketSize
{
    int     nBoardNum = gGeneral.gnActiveBoard;
    long    lRet;

	//2010/6/9 DVB-T2
	long	T2MI_playrate;
	T2MI_playrate = -1;

    //--- TDMB --> return
    //if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    //    return -1;

    //--- File not exist --> return
    if (!gUtility.Is_File_Exist(szFileName))
        return -2;
	
	//DVB-T2 kslee 2010/4/20
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		//2010/11/12 4097 -> 65526
		char szResult[65526];
		gUtility.MyStrCpy(szResult, 65526, "");
		lRet = TSPH_RUN_T2MI_PARSER(nBoardNum, szFileName, szResult);
		T2MI_playrate = lRet;
		if(lRet <= 1)
			lRet = TSPH_CAL_PLAY_RATE(nBoardNum, szFileName, 0);
	}
	//I/Q PLAY/CAPTURE
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{	
		//2011/11/16 IQ File 
		FILE *fp = NULL;
		unsigned char iq_data_info[8];
		fp = fopen(szFileName, "rb");
		if(fp != NULL)
		{
			fread(iq_data_info, 1, 8, fp);
			if(iq_data_info[0] == 'I' && iq_data_info[1] == 'Q' && iq_data_info[2] == '1' && iq_data_info[3] == '4')
			{
				gpConfig->gBC[nBoardNum].gnSymbolRate = (unsigned long)((iq_data_info[4] << 24) + (iq_data_info[5] << 16) + (iq_data_info[6] << 8) + iq_data_info[7]);
			}
			else
			{
				fclose(fp);
				gpConfig->gBC[nBoardNum].gnSymbolRate = 0;
				return -2;
			}
			fclose(fp);
		}
		lRet = 32 * gpConfig->gBC[nBoardNum].gnSymbolRate;
	}
	else
		lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 0);  // playrrate

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
	{
		if(lRet == -1)
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;
		}
		else
		{
			gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
		}
		return lRet;
	}
    
	if (lRet == -1)
    {
        if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T ||
            gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13 ||
			gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
            ;
        else
            gpConfig->gBC[nBoardNum].gInvalidBitrate = 1;

        //return -3;
    }
	//2012/3/19 CMMB CRC ERROR
	else if(lRet == -2 && gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{

		LogMessage("Invalid stream(CRC32 ERROR). Please check Multiplex Frame Header.");
		gpConfig->gBC[nBoardNum].gInvalidBitrate = -1;
	}
	else
    {
        gpConfig->gBC[nBoardNum].gInvalidBitrate = 0;
        //2010/6/9
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
		{
			gGeneral.gnBitrate = lRet;
			gGeneral.gnT2MI_playrate = T2MI_playrate;
		}else
			gGeneral.gnBitrate = lRet;

        lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 1);     // packet size
        gGeneral.gnPacketSize = lRet;
        return 1;
    }
	lRet = gWrapDll.Get_Playrate(nBoardNum, szFileName, 1);     // packet size
	if(lRet == -1)
	{
		return -3;
	}
	else
	{
        gGeneral.gnPacketSize = lRet;
        return 2;
	}
}
#endif
//---------------------------------------------------------------------------
// set  gGeneral.gnElapsedTime
//      (gpConfig->gBC[nBoardNum].gnCurrentPosChanged)
char *CUTIL_IND::szElapsedTimeinHMSdFormat(long nBoardNum)
{
    //       Elapsed time = CurrentTime - StartTime in [00:00:00.0] format string
    long    dwCurTime;
    double  dwEndTime;
    double  dwRunTime;
    int     nSliderPos;

#ifdef WIN32
    gUtility.MyStrCpy(gstrGeneral, 256, "");	// strElapsedTimeinHMSdFormat
#else 
    gUtility.MyStrCpy(gstrGeneral, 256, (char *) "");	// strElapsedTimeinHMSdFormat
//2012/5/14
#ifdef STANDALONE
	if(gpConfig->gBC[nBoardNum].bPlayingProgress == true || gpConfig->gBC[nBoardNum].bRecordInProgress == true)
	{
		if(gGeneral.gnSetSystemClock == 1)
		{
			gpConfig->gBC[nBoardNum].dwStartTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum) - (gGeneral.gnSnmpElapsedTime * 10);
			gGeneral.gnSetSystemClock = 0;
		}
	}
#endif

#endif
    //----------------------------------------------------------------
    dwCurTime = gUtilInd.GetCurrentTimeinMsec(nBoardNum) - gpConfig->gBC[nBoardNum].dwStartTime;
    gGeneral.gnElapsedTime = dwCurTime / 10;    // sec
    //----------------------------------------------------------------

#ifdef WIN32

	//if(gpConfig->gBC[nBoardNum].gnBertPacketType >= TS_HEAD_184_ALL_0 && gpConfig->gBC[nBoardNum].gnBertPacketType <= TS_SYNC_187_PRBS_2_23)
	//{
	//	gUtility.MyStrCpy(gstrGeneral,256, szMsecTimeToHMSdFormat(dwCurTime));
	//    return gstrGeneral;
	//}

	if(gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 && (gpConfig->gBC[nBoardNum].bPlayingProgress == true || gpConfig->gBC[nBoardNum].bRecordInProgress == true))
	{
		gUtility.MyStrCpy(gstrGeneral,256, szMsecTimeToHMSdFormat(dwCurTime));
	    return gstrGeneral;
	}
#else 
	//2010/9/29 ActiveBoard
	if(nBoardNum == gGeneral.gnActiveBoard)
	{
		gGeneral.gnSnmpElapsedTime = gGeneral.gnElapsedTime;
	}

#endif
    if (gpConfig->gBC[nBoardNum].gdwPlayRate >= 1)
    {
        if (gpConfig->gBC[nBoardNum].gnCurrentPosChanged == 1)
        {
            gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
            gpConfig->gBC[nBoardNum].gnCurrentPosChanged = 0;
            return gstrGeneral;
        }
        else if (gpConfig->gBC[nBoardNum].gnStartPosChanged == 1 &&
                 gpConfig->gBC[nBoardNum].gnEndPosChanged == 1)
        {
            dwEndTime = ((double)(gpConfig->gBC[nBoardNum].gnEndPos / (double)(gpConfig->gBC[nBoardNum].gdwPlayRate) * 8.0)) * 10.0;
            
			//kslee 2010/3/18
			//PCR RESTAMP	2010/5/31 DVB-T2
			if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
			{
				if(gGeneral.gnBitrate > 0)
				{
					dwEndTime = (long)((double)gpConfig->gBC[nBoardNum].gnEndPos / (double)gGeneral.gnBitrate * 8.0) * 10;
				}
			}
			
	 		//2012/5/15
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
			{
				dwEndTime = dwEndTime * 7300.0 / 6144;
			}

			if (dwCurTime > dwEndTime)
            {
                gUtilInd.ResetElapsedtimeCounter(nBoardNum, 0);
                return gstrGeneral;
            }
        }
    }
    
    //----------------------------------------------------------------
    if (gpConfig->gBC[nBoardNum].dwFileSize > 0 &&
        gpConfig->gBC[nBoardNum].gdwPlayRate > 0)
    {
		//2011/4/7 ISDB-S Bitrate
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
		{
			if(gGeneral.gnPacketSize == 204)
				dwRunTime = (long)((double)(gpConfig->gBC[nBoardNum].dwFileSize / 52170000.0) * 8.0 *188.0 / 204.0);
			else
				dwRunTime = (long)((double)(gpConfig->gBC[nBoardNum].dwFileSize / 52170000.0) * 8.0);
		}
		else
			dwRunTime = (long)((double)(gpConfig->gBC[nBoardNum].dwFileSize / (double)(gpConfig->gBC[nBoardNum].gdwPlayRate)) * 8.0);
		
		//kslee 2010/3/18
		//PCR RESTAMP
		if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
		{
			if(gGeneral.gnBitrate > 0)
			{
				//2011/4/7 ISDB-S Bitrate
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
				{
					if(gGeneral.gnPacketSize == 204)
						dwRunTime = ((long)((double)gpConfig->gBC[nBoardNum].dwFileSize / 52170000.0 * 8.0 * 188.0 / 204.0));
					else
						dwRunTime = ((long)((double)gpConfig->gBC[nBoardNum].dwFileSize / 52170000.0 * 8.0));
				}
				else
					dwRunTime = ((long)((double)gpConfig->gBC[nBoardNum].dwFileSize / (double)gGeneral.gnBitrate * 8.0));
			}
		}
		//2010/12/06 ISDB-S =================================================
		else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && (gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1 /*|| gpConfig->gBC[nBoardNum].gnCombinedTS == 1*/))
		{
			dwRunTime = (long)((double)(gpConfig->gBC[nBoardNum].dwFileSize / (double)(gpConfig->gBC[nBoardNum].gnISDBS_BaseBitrate)) * 8.0);
	        if ( (dwCurTime / 10) >= dwRunTime)
		        return gstrGeneral;
		}
		//2011/1/03 DVB-T2 MULTI-PLP ============================================================================
		else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
			;
		//=======================================================================================================
		//2011/5/23 DVB-C2 MULTI PLP
		else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2 && gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
			;
		else
		{
	        if ( (dwCurTime / 10) >= dwRunTime)
		        return gstrGeneral;
		}
		//===================================================================
        if (dwRunTime <= 0)
			nSliderPos = 0;
        else
            nSliderPos = (int) ( ( (dwCurTime/10.0)/ dwRunTime) * 99.0);

        if (gpConfig->gBC[nBoardNum].gnCurrentPosScrolled == 0)
        {
			if(gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
		        gpConfig->gBC[nBoardNum].gnCurrentPos = (__int64) ((__int64)((double)gpConfig->gBC[nBoardNum].dwFileSize / 7300.0 * 6144.0) * nSliderPos / 99.0);
			else
	            gpConfig->gBC[nBoardNum].gnCurrentPos = (__int64) (gpConfig->gBC[nBoardNum].dwFileSize * nSliderPos / 99.0);
            //  set Slider in FormMain::Display_LabTimer
        }
    }

    gUtility.MyStrCpy(gstrGeneral,256, szMsecTimeToHMSdFormat(dwCurTime));
    return gstrGeneral;
}

//---------------------------------------------------------------------------
char *CUTIL_IND::szMsecTimeToHMSdFormat(long dwMSecTime)
{
    //       Convert msecond unit time value to [00:00:00.0] format string
    long dwMsecond, dwSecond, dwMinute, dwHour;

    if (dwMSecTime < 0)
           dwMSecTime = dwMSecTime + 864000;   // add 24 hour

    dwMsecond = dwMSecTime % 10;     // have remainder
    dwSecond = dwMSecTime / 10;
    dwSecond = dwSecond  % 60;
    dwMinute = dwMSecTime / 600;
    dwMinute = dwMinute % 60;
    dwHour = dwMSecTime / 36000;
    dwHour = dwHour % 24;

    gUtility.MySprintf(gstrGeneral, 256, "%02d:%02d:%02d.%d", dwHour, dwMinute, dwSecond, dwMsecond);
    return gstrGeneral;
}

//---------------------------------------------------------------------------
// Convert Second unit time value to [00:00:00] format string
char *CUTIL_IND::szSecTimeToHMSformat(unsigned long dwSecTime)
{
    unsigned long    dwSecond, dwMinute, dwHour;

    if (dwSecTime > 356400)         //more that 99 hour
    {
        gUtility.MyStrCpy(gstrGeneral, 256, "99:99:99");
    }
    else
    {
        if (dwSecTime < 0)
            dwSecTime = dwSecTime + 86400l;  //   ' add 24 hour

        dwSecond = dwSecTime  % 60;
        dwMinute = dwSecTime / 60;
        dwMinute = dwMinute % 60;
        dwHour = dwSecTime / 3600;
        //dwHour = dwHour / 24;

        gUtility.MySprintf(gstrGeneral, 256, "%02d:%02d:%02d", dwHour, dwMinute, dwSecond);
    }
    return gstrGeneral;
}

//---------------------------------------------------------------------------
// flow:
//      1) create at.bat file and write at.exe > atlist
//      2) execute at.bat
//      3) open atlist file
//      4) read line and set gszScheduledTask[],gnScheduledTask
void CUTIL_IND::GetScheduledTask()
{
    char    DosCmd[256];
    char    TempPath[256];
    char    File_Data[256];
    char    ScheduledInfo[1024];
    char    ScheduledEntry[256];
    char    TaskInfo[256];
    char    str[256];
    char    WeekDays[20], WakeupTime[20], ExitTime[20], Duration[20];
    FILE    *hFile = NULL, *hFile1 = NULL;
    long    lReturn; 
    int     DayOfMonth, nScheduleType, n, nID=0;

    //-------------------------------------------------------
    //---- write "at.exe > atlist in at.bat file
    gUtility.MySprintf(DosCmd,  256, "at.exe > atlist");
    gUtility.MyStrCpy(TempPath, 256, "at.bat");
	hFile = gUtility.MyFopen(hFile, TempPath,"w");
    if (hFile == NULL)
        return;

    fwrite(DosCmd, 1, strlen(DosCmd), hFile);
    fclose(hFile);
    hFile = NULL;
	

#if defined(WIN32)

	WinExec(TempPath, SW_HIDE);

    Sleep(1000);
#else
	return;
#endif

	
    //-------------------------------------------------------
    gUtility.MyStrCpy(TempPath, 256, "atlist");
	hFile = gUtility.MyFopen(hFile, TempPath,"r");
    if (hFile == NULL)
        return;

    gpConfig->gnScheduledTask = 0;
    lReturn = Get_One_Line_From_File(hFile, File_Data, 256);       // skip header line
	//2010/11/29
    //String^ strt;
	
	while (1)        // Not EOF
    {
        lReturn = Get_One_Line_From_File(hFile, File_Data, 256);
        if (lReturn < 0)
            break;
	
        if (gpConfig->gnScheduledTask < MAX_SCHEDULE_COUNT)         // 5
        {
			//gBaseUtil.ChangeStringFromChar(&strt, File_Data);
			//2010/11/29
			//System::Windows::Forms::MessageBox::Show(strt);
            lReturn = Get_At_Bat_File(File_Data, TempPath);       // Get mmddhhnnss_at.bat file name to TempPath. If not exist return < 0
            if (lReturn > 0)                                        // TempPath=CurDir\\mmddhhnnss_at.bat
            {
                str[0] = File_Data[7];
                str[1] = File_Data[8];
                str[2] = File_Data[9];
                str[3] = '\0';
                nID = atoi(str);
                gUtility.MySprintf(TaskInfo, 256, "%d   ", nID);

                if (gUtility.Is_File_Exist(TempPath) == true)
                {
					hFile1 = gUtility.MyFopen(hFile1, TempPath,"r");
                    while (1)
                    {
                        lReturn = Get_One_Line_From_File(hFile1, ScheduledInfo, 1024);
                        if (lReturn < 0)
                            break;

                        //-----------------------------------------------------
                        //--- Check Schedule TYPE
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem SCHEDULE ");
                        if (strncmp(ScheduledInfo, ScheduledEntry, strlen(ScheduledEntry)) == 0)
                        {
                            nScheduleType = ScheduledInfo[13] - '0';
                            if (nScheduleType == 0)
                                gUtility.MyStrCat(TaskInfo, 256, "DAILY ");
                            else if (nScheduleType == 1)
                                gUtility.MyStrCat(TaskInfo, 256, "WEEKLY ");
                            else if (nScheduleType == 2)
                                gUtility.MyStrCat(TaskInfo, 256, "MONTHLY ");
                            else
                                gUtility.MyStrCat(TaskInfo, 256, "Unknown ");
                        }

                        //-----------------------------------------------------
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem WAKE-UP ");
                        if (strncmp(ScheduledInfo, ScheduledEntry,  strlen(ScheduledEntry)) == 0)
                        {
                            n = (int) (strlen(ScheduledInfo) - strlen(ScheduledEntry));
                            memcpy(WakeupTime, &ScheduledInfo[strlen(ScheduledEntry)], n);
                            WakeupTime[n] = '\0';
                            gUtility.MyStrCat(TaskInfo, 256, WakeupTime);
                            gUtility.MyStrCat(TaskInfo, 256, " ");
                        }
                                                
                        //-----------------------------------------------------
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem EXIT ");
                        if (strncmp(ScheduledInfo, ScheduledEntry,  strlen(ScheduledEntry)) == 0)
                        {
                            n = (int) (strlen(ScheduledInfo) - strlen(ScheduledEntry));
                            memcpy(ExitTime, &ScheduledInfo[strlen(ScheduledEntry)], n);
                            ExitTime[n] = '\0';
                            gUtility.MyStrCat(TaskInfo, 256, ExitTime);
                            gUtility.MyStrCat(TaskInfo, 256, " ");
                        }

                        //-----------------------------------------------------
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem DURATION ");
                        if (strncmp(ScheduledInfo, ScheduledEntry,  strlen(ScheduledEntry)) == 0)
                        {
                            n = (int) (strlen(ScheduledInfo) - strlen(ScheduledEntry) );
                            memcpy(Duration, &ScheduledInfo[strlen(ScheduledEntry)], n);
                            Duration[n] = '\0';
                            if (strlen(Duration) > 0 && atoi(Duration) > 0)
                            {
                                gUtility.MyStrCat(TaskInfo, 256, Duration);
                                gUtility.MyStrCat(TaskInfo, 256, "min. ");
                            }
                        }

                        //-----------------------------------------------------
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem DAYOFMONTH ");
                        if (strncmp(ScheduledInfo, ScheduledEntry,  strlen(ScheduledEntry)) == 0)
                        {
                            n = (int) (strlen(ScheduledInfo) - strlen(ScheduledEntry));
                            memcpy(str, &ScheduledInfo[strlen(ScheduledEntry)], n);
                            str[n] = '\0';
                            DayOfMonth = atoi(str);
                            if (DayOfMonth > 0)
                            {
                                gUtility.MySprintf(str, 256, "%d", DayOfMonth);
                                gUtility.MyStrCat(TaskInfo,  256, str);

                                if (DayOfMonth == 1)
                                    gUtility.MyStrCat(TaskInfo,  256, "st day ");
                                else if (DayOfMonth == 2)
                                    gUtility.MyStrCat(TaskInfo,  256, "nd day ");
                                else if (DayOfMonth == 3)
                                    gUtility.MyStrCat(TaskInfo,  256, "rd day ");
                                else
                                    gUtility.MyStrCat(TaskInfo,  256, "th day ");
                            }
                        }

                        //-----------------------------------------------------
                        gUtility.MyStrCpy(ScheduledEntry, 256, "rem WEEKDAYS ");
                        if (strncmp(ScheduledInfo, ScheduledEntry,  strlen(ScheduledEntry)) == 0)
                        {
                            n = (int) (strlen(ScheduledInfo) - strlen(ScheduledEntry));
                            memcpy(WeekDays, &ScheduledInfo[strlen(ScheduledEntry)], n);
                            WeekDays[n] = '\0';
                            if (strlen(WeekDays) > 0)
                            {
                                gUtility.MyStrCat(TaskInfo,  256, WeekDays);
                                gUtility.MyStrCat(TaskInfo,  256, " ");
                            }
                        }                        
                    }   // end of while
                    fclose(hFile1);     //mmddhhmmss_at.bat
                    hFile1 = NULL;
					//2010/11/29
					//System::Windows::Forms::MessageBox::Show("END hFile1");

                    gUtility.MyStrCpy(gpConfig->gszScheduledTask[gpConfig->gnScheduledTask], 1024, TaskInfo);
                    gpConfig->gnScheduledTask = gpConfig->gnScheduledTask + 1;
                }   // end of if
            }
        }   //  if (gpConfig->gnScheduledTask < MAX_SCHEDULE_COUNT)         // 5
    }   // end of while
    
    fclose(hFile);      // atlist
	hFile = NULL;
	//2010/11/29
	//System::Windows::Forms::MessageBox::Show("END hFile");
}

//---------------------------------------------------------------------------
// Get mmddhhnnss_at.bat file name to TempPath. If not exist return < 0
long CUTIL_IND::Get_At_Bat_File(char *strLine, char *TempPath)
{
    long    lReturn = -1;
    char    *pdest;
    int     iPos;
    char    str[100];

    if (strlen(strLine) < 17)
        return -1;

    //--- find _at.bat position
    pdest = strstr(strLine, "_at.bat");
    if (pdest == NULL)
        return -1;          // not found

    iPos = (int) (pdest - strLine + 1);     // iPos = postion of _at.bat
    memcpy(str, &strLine[iPos - 11], 17);       // mmddhhmmss_at.bat
    str[17] = '\0';

#ifdef WIN32
    gUtility.MySprintf(TempPath, 256, "%s\\%s", gGeneral.gnStrCurDir, str);
#else
    gUtility.MySprintf(TempPath, 256, (char *)"%s/%s", gGeneral.gnStrCurDir, str);
#endif

    return 1;
}

//------------------------------------------------------------------------------
// Get One Line and Return converted numeric
// return -1 if EOF or no data
void CUTIL_IND::Get_TS_Parameters(char *input_string, unsigned short *pPID, char *pSI, unsigned long *pPGMNUM, unsigned short *pPCR, char *pDesc, int *pBitrate)
{
    char    str[40];
    int     i,k;
    int     index;
    int     iLen = (int) (strlen(input_string));

    //----------------------------------------------------------------
    //--- initialize
    *pPID = 0;
    gUtility.MyStrCpy(pSI,20, "");
    *pPGMNUM = 0;
    *pPCR = 0;
    gUtility.MyStrCpy(pDesc,20, "");
    *pBitrate = 0;

    //----------------------------------------------------------------
    if (iLen < 6)
        return;

    k = 0;
    index = 0;
    for (i = 0; i < iLen; i++)
    {
        if (input_string[i] == ',')
        {
            str[k] = '\0';
            if (index == 0)
                *pPID = atoi(str);
            else if (index == 1)
                gUtility.MyStrCpy(pSI, 40,  str);
            else if (index == 2)
                *pPGMNUM = atoi(str);
            else if (index == 3)
                *pPCR = atoi(str);
            else if (index == 4)
                gUtility.MyStrCpy(pDesc, 40, str);
            else if (index == 5)
                *pBitrate = atoi(str);

            k = 0;
            index++;
            if (index >= 6)
                return;
                
            continue;
        }

       str[k++] = input_string[i];
    }

    return;
}

//------------------------------------------------------------------------------
// Get One Line and Return converted numeric
void CUTIL_IND::Get_Two_Parameters(char *input_string, long *pChNum, long *pFreq)
{
    char    str[20];
    int     i,k;
    int     index;
    int     iLen = (int) (strlen(input_string));

    //----------------------------------------------------------------
    //--- initialize
    *pChNum = 0;
    *pFreq = 0;

    //----------------------------------------------------------------
    if (iLen < 3)
        return;

    k = 0;
    index = 0;
    for (i = 0; i < iLen; i++)
    {
        if (input_string[i] == ',')
        {
            str[k] = '\0';
            if (index == 0)
                *pChNum = atoi(str);
            else if (index == 1)
                *pFreq = atoi(str);
            k = 0;
            index++;
            if (index >= 2)
                return;
                
            continue;
        }

       str[k++] = input_string[i];
    }
    str[k] = '\0';
    *pFreq = atoi(str);
    return;
}

//---------------------------------------------------------------------------
char *CUTIL_IND::MakeCommandArgs(long nBoardNum, long nRunType, char *TargetFile)
{
    char    str[200], str1[200], str2[200], str3[200], str4[200], str5[200];
    char    str6[200], str7[200], str8[200], str9[200], str10[200], str11[200];
	//2010/6/10
	char	str12[200], str13[200], str14[200], str15[200];
    char    RepeatMode[20], ModType[20], ModulatorSource[20], IFvalue[20];
    char    CodeRate[20], Constellation[20], Bandwidth[20], Txmode[20];
    char    GuardInterval[20], QAMInterleave[20], SpectrumInverse[20], PRBSmode[20];
    char    mpe_fec[20], time_slice[20], in_depth[20], Pilot[20];
    char    RollOffFactor[20], StopMode[20], restamping[20], CalcPlayRate[20];
    char    UseAVDecoding[20], UseIPStreaming[20], BypassAMP[20], AutoRun[20];
    char    Max_Playrate[20], FrameHeader[20], CarrierNumber[20], FrameHeaderPN[20];
    char    PilotInsertion[20], continuity[20], timeoffset[20], userdate[20], usertime[20];
	//2010/12/14 
	char	UseTMCC[20], MultiTS[20], EmergencyAlarm[20];

    static  char    strArg[4096];//strArg[2048];

    gUtility.MyStrCpy(strArg, 4096/*2048*/, "");
    
    //--------------------------------------------------------
    // play mode
    if (gpConfig->gBC[nBoardNum].gbRepeatMode == true)
        gUtility.MyStrCpy(RepeatMode, 20, "loop");
    else
        gUtility.MyStrCpy(RepeatMode, 20, "once");
	if (gpConfig->gBC[nBoardNum].gnModulatorSource != FILE_SRC)
		nRunType = 1;
    // Input source
    if (gpConfig->gBC[nBoardNum].gnModulatorSource == FILE_SRC)
        gUtility.MyStrCpy(ModulatorSource, 20, "FILE");
    else if (gpConfig->gBC[nBoardNum].gnModulatorSource == SMPTE310M_SRC)
        gUtility.MyStrCpy(ModulatorSource, 20, "SMPTE-310M");
    else if (gpConfig->gBC[nBoardNum].gnModulatorSource == DVBASI_SRC)
        gUtility.MyStrCpy(ModulatorSource, 20, "DVB-ASI");
    else
        gUtility.MyStrCpy(ModulatorSource, 20, "FILE");

    // modulator type
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T)
        gUtility.MyStrCpy(ModType, 20, "dvb-t");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_8)
        gUtility.MyStrCpy(ModType, 20, "8vsb");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
        gUtility.MyStrCpy(ModType, 20, "qam-ac");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
        gUtility.MyStrCpy(ModType, 20, "qam-b");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK)
        gUtility.MyStrCpy(ModType, 20, "qpsk");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
        gUtility.MyStrCpy(ModType, 20, "tdmb");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == VSB_16)
        gUtility.MyStrCpy(ModType, 20, "16vsb");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
        gUtility.MyStrCpy(ModType, 20, "dvb-h");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
        gUtility.MyStrCpy(ModType, 20, "dvb-s2");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T)
        gUtility.MyStrCpy(ModType, 20, "isdb-t");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_T_13)
        gUtility.MyStrCpy(ModType, 20, "isdb-t-13");
    else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
        gUtility.MyStrCpy(ModType, 20, "dtmb");
    //CMMB
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
        gUtility.MyStrCpy(ModType, 20, "cmmb");
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ATSC_MH)
        gUtility.MyStrCpy(ModType, 20, "atsc-mh");
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
        gUtility.MyStrCpy(ModType, 20, "dvb-t2");
	//2010/12/14 ISDB-S
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
		gUtility.MyStrCpy(ModType, 20, "isdb-s");
    else
        gUtility.MyStrCpy(ModType, 20, "qam-a");

    // IF
    if (gpConfig->gBC[nBoardNum].gnIFOutFreq == IF_OUT_36MHZ)
        gUtility.MyStrCpy(IFvalue, 20, "36000000");
    else
        gUtility.MyStrCpy(IFvalue, 20, "44000000");

    // Code rate
    gUtility.MyStrCpy(CodeRate, 20, "");
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
            gUtility.MyStrCpy(CodeRate, 20, "1/2");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
            gUtility.MyStrCpy(CodeRate, 20, "2/3");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
            gUtility.MyStrCpy(CodeRate, 20, "3/4");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
            gUtility.MyStrCpy(CodeRate, 20, "5/6");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
            gUtility.MyStrCpy(CodeRate, 20, "7/8");
        else
            gUtility.MyStrCpy(CodeRate, 20, "1/2");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
                gUtility.MyStrCpy(CodeRate, 20, "1/4");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
                gUtility.MyStrCpy(CodeRate, 20, "1/3");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
                gUtility.MyStrCpy(CodeRate, 20, "2/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
                gUtility.MyStrCpy(CodeRate, 20, "1/2");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
                gUtility.MyStrCpy(CodeRate, 20, "3/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 5)
                gUtility.MyStrCpy(CodeRate, 20, "2/3");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 6)
                gUtility.MyStrCpy(CodeRate, 20, "3/4");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 7)
                gUtility.MyStrCpy(CodeRate, 20, "4/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 8)
                gUtility.MyStrCpy(CodeRate, 20, "5/6");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 9)
                gUtility.MyStrCpy(CodeRate, 20, "8/9");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 10)
                gUtility.MyStrCpy(CodeRate, 20, "9/10");
            else
                gUtility.MyStrCpy(CodeRate, 20, "1/4");
        } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
                gUtility.MyStrCpy(CodeRate, 20, "3/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
                gUtility.MyStrCpy(CodeRate, 20, "2/3");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
                gUtility.MyStrCpy(CodeRate, 20, "3/4");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
                gUtility.MyStrCpy(CodeRate, 20, "5/6");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
                gUtility.MyStrCpy(CodeRate, 20, "8/9");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 5)
                gUtility.MyStrCpy(CodeRate, 20, "9/10");
            else
                gUtility.MyStrCpy(CodeRate, 20, "3/5");
        } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
                gUtility.MyStrCpy(CodeRate, 20, "2/3");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
                gUtility.MyStrCpy(CodeRate, 20, "3/4");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
                gUtility.MyStrCpy(CodeRate, 20, "4/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
                gUtility.MyStrCpy(CodeRate, 20, "5/6");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
                gUtility.MyStrCpy(CodeRate, 20, "8/9");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 5)
                gUtility.MyStrCpy(CodeRate, 20, "9/10");
            else
                gUtility.MyStrCpy(CodeRate, 20, "2/3");
        } else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
        {
            if (gpConfig->gBC[nBoardNum].gnCodeRate == 0)
                gUtility.MyStrCpy(CodeRate, 20, "3/4");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 1)
                gUtility.MyStrCpy(CodeRate, 20, "4/5");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 2)
                gUtility.MyStrCpy(CodeRate, 20, "5/6");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 3)
                gUtility.MyStrCpy(CodeRate, 20, "8/9");
            else if (gpConfig->gBC[nBoardNum].gnCodeRate == 4)
                gUtility.MyStrCpy(CodeRate, 20, "9/10");
            else
                gUtility.MyStrCpy(CodeRate, 20, "3/4");
        }
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnCodeRate == CONST_DTMB_CODE_7488_3008)
            gUtility.MyStrCpy(CodeRate, 20, "0.4");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == CONST_DTMB_CODE_7488_4512)
            gUtility.MyStrCpy(CodeRate, 20, "0.6");
        else if (gpConfig->gBC[nBoardNum].gnCodeRate == CONST_DTMB_CODE_7488_4512)
            gUtility.MyStrCpy(CodeRate, 20, "0.8");
        else
            gUtility.MyStrCpy(CodeRate, 20, "0.4");
    }
	//2010/12/14 ISDB-S ========================================================================
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnCodeRate == CONST_ISDBS_CODE_1_2)
			gUtility.MyStrCpy(CodeRate, 20, "1/2");
		else if(gpConfig->gBC[nBoardNum].gnCodeRate == CONST_ISDBS_CODE_2_3)
			gUtility.MyStrCpy(CodeRate, 20, "2/3");
		else if(gpConfig->gBC[nBoardNum].gnCodeRate == CONST_ISDBS_CODE_3_4)
			gUtility.MyStrCpy(CodeRate, 20, "3/4");
		else if(gpConfig->gBC[nBoardNum].gnCodeRate == CONST_ISDBS_CODE_5_6)
			gUtility.MyStrCpy(CodeRate, 20, "5/6");
		else if(gpConfig->gBC[nBoardNum].gnCodeRate == CONST_ISDBS_CODE_7_8)
			gUtility.MyStrCpy(CodeRate, 20, "7/8");
		else
			gUtility.MyStrCpy(CodeRate, 20, "1/2");
	}
	//===========================================================================================

    if (strlen(CodeRate) != 0)
    {
        gUtility.MyStrCpy(str, 200, CodeRate);
        gUtility.MySprintf(CodeRate, 20, "-coderate %s", str);
    } else
        gUtility.MyStrCpy(CodeRate, 20, " ");

    // constellation
    gUtility.MyStrCpy(Constellation, 20, "");
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T ||
        gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_QPSK)
            gUtility.MyStrCpy(Constellation,  20, "QPSK");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_16QAM)
            gUtility.MyStrCpy(Constellation,  20, "16QAM");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_64QAM)
            gUtility.MyStrCpy(Constellation,  20, "64QAM");
        else
            gUtility.MyStrCpy(Constellation,  20, "QPSK");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_A)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_A_16)
            gUtility.MyStrCpy(Constellation,  20, "16QAM");
        else if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_A_32)
            gUtility.MyStrCpy(Constellation,  20, "32QAM");
        else if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_A_64)
            gUtility.MyStrCpy(Constellation,  20, "64QAM");
        else if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_A_128)
            gUtility.MyStrCpy(Constellation,  20, "128QAM");
        else if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_A_256)
            gUtility.MyStrCpy(Constellation,  20, "256QAM");
        else
            gUtility.MyStrCpy(Constellation,  20, "16QAM");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_B_64)
            gUtility.MyStrCpy(Constellation,  20, "64QAM");
        else if (gpConfig->gBC[nBoardNum].gnQAMMode == CONST_QAM_B_256)
            gUtility.MyStrCpy(Constellation,  20, "256QAM");
        else
            gUtility.MyStrCpy(Constellation,  20, "64QAM");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_QPSK)
            gUtility.MyStrCpy(Constellation,  20, "QPSK");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_8PSK)
            gUtility.MyStrCpy(Constellation,  20, "8PSK");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_16APSK)
            gUtility.MyStrCpy(Constellation,  20, "16APSK");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DVB_S2_32APSK)
            gUtility.MyStrCpy(Constellation,  20, "32APSK");
        else
            gUtility.MyStrCpy(Constellation,  20, "QPSK");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM_NR)
            gUtility.MyStrCpy(Constellation,  20, "4QAM-NR");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_4QAM)
            gUtility.MyStrCpy(Constellation,  20, "4QAM");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_16QAM)
            gUtility.MyStrCpy(Constellation,  20, "16QAM");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_32QAM)
            gUtility.MyStrCpy(Constellation,  20, "32QAM");
        else if (gpConfig->gBC[nBoardNum].gnConstellation == CONST_DTMB_64QAM)
            gUtility.MyStrCpy(Constellation,  20, "64QAM");
        else
            gUtility.MyStrCpy(Constellation,  20, "4QAM-NR");
    }
    //2010/12/14 ISDB-S==================================================================================================================
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S)
	{
		if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_BPSK)
			gUtility.MyStrCpy(Constellation,  20, "BPSK");
		else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_QPSK)
			gUtility.MyStrCpy(Constellation,  20, "QPSK");
		else if(gpConfig->gBC[nBoardNum].gnConstellation == CONST_ISDBS_TC8PSK)
			gUtility.MyStrCpy(Constellation,  20, "TC8PSK");
		else
			gUtility.MyStrCpy(Constellation,  20, "BPSK");
	}
	//===================================================================================================================================

    if (strlen(Constellation) != 0)
    {
        gUtility.MyStrCpy(str, 200, Constellation);
        gUtility.MySprintf(Constellation, 20, "-const %s", str);
    } else
        gUtility.MyStrCpy(Constellation, 20, " ");

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		// band widtrh
		if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_1Dot7MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "1.7MHz");
		else if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_5MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "5MHz");
		if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_6MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "6MHz");
		else if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBT2_BW_7MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "7MHz");
		else
			gUtility.MyStrCpy(Bandwidth, 20, "8MHz");
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_H)
	{
		// band widtrh
		if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBH_BW_5MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "5MHz");
		else if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBH_BW_6MHZ )
			gUtility.MyStrCpy(Bandwidth, 20, "6MHz");
		else if (gpConfig->gBC[nBoardNum].gnBandwidth == DVBH_BW_7MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "7MHz");
		else
			gUtility.MyStrCpy(Bandwidth, 20, "8MHz");
	}
	else
	{
		// band widtrh
		if (gpConfig->gBC[nBoardNum].gnBandwidth == BW_6MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "6MHz");
		else if (gpConfig->gBC[nBoardNum].gnBandwidth == BW_7MHZ)
			gUtility.MyStrCpy(Bandwidth, 20, "7MHz");
		else
			gUtility.MyStrCpy(Bandwidth, 20, "8MHz");
	}
    // tx mode
    if (gpConfig->gBC[nBoardNum].gnTxmode == TX_2K)
        gUtility.MyStrCpy(Txmode,20,  "2K");
    else if (gpConfig->gBC[nBoardNum].gnTxmode == TX_8K)
        gUtility.MyStrCpy(Txmode, 20, "8K");
    else if (gpConfig->gBC[nBoardNum].gnTxmode == TX_4K)
        gUtility.MyStrCpy(Txmode, 20, "4K");
    else
        gUtility.MyStrCpy(Txmode, 20, "2K");
    
    // Guard Interval
    if (gpConfig->gBC[nBoardNum].gnGuardInterval == GI_1_OF_4)
        gUtility.MyStrCpy(GuardInterval, 20, "1/4");
    else if (gpConfig->gBC[nBoardNum].gnGuardInterval == GI_1_OF_8)
        gUtility.MyStrCpy(GuardInterval, 20, "1/8");
    else if (gpConfig->gBC[nBoardNum].gnGuardInterval == GI_1_OF_16)
        gUtility.MyStrCpy(GuardInterval, 20, "1/16");
    else if (gpConfig->gBC[nBoardNum].gnGuardInterval == GI_1_OF_32)
        gUtility.MyStrCpy(GuardInterval, 20, "1/32");
    else
        gUtility.MyStrCpy(GuardInterval, 20, "1/4");

    // QAM interleaving
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == QAM_B)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_1_)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-1");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_64_2)
            gUtility.MyStrCpy(QAMInterleave, 20, "64-2");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_32_4)
            gUtility.MyStrCpy(QAMInterleave,20,  "32-4");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_16_8)
            gUtility.MyStrCpy(QAMInterleave, 20, "16-8");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_8_16)
            gUtility.MyStrCpy(QAMInterleave, 20, "8-16");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_1)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-1");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_2)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-2");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_3)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-3");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_4)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-4");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_5)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-5");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_6)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-6");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_7)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-7");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == INTERLEAVE_128_8)
            gUtility.MyStrCpy(QAMInterleave, 20, "128-8");
        else
            gUtility.MyStrCpy(GuardInterval, 20, "128-1");
    } else if (gpConfig->gBC[nBoardNum].gnModulatorMode == DTMB)
    {
        if (gpConfig->gBC[nBoardNum].gnQAMInterleave == CONST_DTMB_INTERLEAVE_0)
            gUtility.MyStrCpy(QAMInterleave, 20, "52/240");
        else if (gpConfig->gBC[nBoardNum].gnQAMInterleave == CONST_DTMB_INTERLEAVE_1)
            gUtility.MyStrCpy(QAMInterleave, 20, "52/720");
        else
            gUtility.MyStrCpy(QAMInterleave, 20, "52/240");
    } else
	{
            gUtility.MyStrCpy(QAMInterleave, 20, "");
	}

    if (strlen(QAMInterleave) != 0)
    {
        gUtility.MyStrCpy(str, 200, QAMInterleave);
        gUtility.MySprintf(QAMInterleave, 20, "-interleaving %s", str);
    } else
        gUtility.MyStrCpy(QAMInterleave, 20, " ");


    // sskim20081010 - DTMB
    if (gpConfig->gBC[nBoardNum].gnFrameHeader == CONST_DTMB_FRAME_HEADER_MODE_1)
        gUtility.MyStrCpy(FrameHeader, 20, "PN420");
    else if (gpConfig->gBC[nBoardNum].gnFrameHeader == CONST_DTMB_FRAME_HEADER_MODE_2)
        gUtility.MyStrCpy(FrameHeader, 20, "PN595");
    else if (gpConfig->gBC[nBoardNum].gnFrameHeader == CONST_DTMB_FRAME_HEADER_MODE_3)
        gUtility.MyStrCpy(FrameHeader, 20, "PN945");
    else
        gUtility.MyStrCpy(FrameHeader, 20, "PN420");

    if (gpConfig->gBC[nBoardNum].gnCarrierNumber == CONST_DTMB_CARRIER_NUMBER_0)
        gUtility.MyStrCpy(CarrierNumber, 20, "1(ADTB-T)");
    else if (gpConfig->gBC[nBoardNum].gnCarrierNumber == CONST_DTMB_CARRIER_NUMBER_1)
        gUtility.MyStrCpy(CarrierNumber, 20, "1(DTMB)");
    else
        gUtility.MyStrCpy(CarrierNumber, 20, "1(ADTB-T)");

    if (gpConfig->gBC[nBoardNum].gnFrameHeaderPN == CONST_DTMB_FRAME_HEADER_PN_FIXED)
        gUtility.MyStrCpy(FrameHeaderPN, 20, "fixed");
    else
        gUtility.MyStrCpy(FrameHeaderPN, 20, "rotated");

    if (gpConfig->gBC[nBoardNum].gnPilotInsertion == CONST_DTMB_PILOT_INSERTION_OFF)
        gUtility.MyStrCpy(PilotInsertion, 20, "off");
    else
        gUtility.MyStrCpy(PilotInsertion, 20, "on");

    if (gpConfig->gBC[nBoardNum].gnSpectrumInverse == SPECTRUM_INVERSE)
        gUtility.MyStrCpy(SpectrumInverse, 20, "inverse");
    else
        gUtility.MyStrCpy(SpectrumInverse, 20, "normal");

    // PRBS mode
    if (gpConfig->gBC[nBoardNum].gnPRBSmode == PRBS_MODE_NONE)
        gUtility.MyStrCpy(PRBSmode, 20, "none");
    else if (gpConfig->gBC[nBoardNum].gnPRBSmode == PRBS_MODE_2_EXP_7_1)
        gUtility.MyStrCpy(PRBSmode, 20, "7");
    else if (gpConfig->gBC[nBoardNum].gnPRBSmode == PRBS_MODE_2_EXP_10_1)
        gUtility.MyStrCpy(PRBSmode, 20, "10");
    else if (gpConfig->gBC[nBoardNum].gnPRBSmode == PRBS_MODE_2_EXP_15_1)
        gUtility.MyStrCpy(PRBSmode, 20, "15");
    else if (gpConfig->gBC[nBoardNum].gnPRBSmode == PRBS_MODE_2_EXP_23_1)
        gUtility.MyStrCpy(PRBSmode, 20, "23");
    else
        gUtility.MyStrCpy(PRBSmode, 20, "none");
    
    // mpe-fec
    if (gpConfig->gBC[nBoardNum].gnMPE_FEC == 1)
        gUtility.MyStrCpy(mpe_fec, 20, "on");
    else
        gUtility.MyStrCpy(mpe_fec, 20, "off");

    // time slice
    if (gpConfig->gBC[nBoardNum].gnTime_Slice == 1)
        gUtility.MyStrCpy(time_slice, 20,  "on");
    else
        gUtility.MyStrCpy(time_slice, 20, "off");

    // in-depth interleaving
    if (gpConfig->gBC[nBoardNum].gnIn_Depth == 1)
        gUtility.MyStrCpy(in_depth, 20, "on");
    else
        gUtility.MyStrCpy(in_depth, 20, "off");

    // pilot
    if (gpConfig->gBC[nBoardNum].gnPilot == 1)
        gUtility.MyStrCpy(Pilot, 20, "on");
    else
        gUtility.MyStrCpy(Pilot, 20, "off");

    // roll-off factor
    if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_020)
        gUtility.MyStrCpy(RollOffFactor, 20, "0.20");
    else if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_025)
        gUtility.MyStrCpy(RollOffFactor, 20, "0.25");
    else if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_035)
        gUtility.MyStrCpy(RollOffFactor, 20, "0.35");
    else if (gpConfig->gBC[nBoardNum].gnRollOffFactor == ROLL_OFF_FACTOR_NONE)
        gUtility.MyStrCpy(RollOffFactor, 20, "none");
    else
        gUtility.MyStrCpy(RollOffFactor, 20, "0.20");
    
    // null TP on stop
    if (gpConfig->gBC[nBoardNum].gnStopMode == 1)
        gUtility.MyStrCpy(StopMode, 20, "on");
    else
        gUtility.MyStrCpy(StopMode, 20, "off");

    // restamping
    if (gpConfig->gBC[nBoardNum].gnRestamping == 1)
        gUtility.MyStrCpy(restamping, 20, "on");
    else
        gUtility.MyStrCpy(restamping, 20, "off");

    // LOOP_ADAPT
    if (gpConfig->gBC[nBoardNum].gnContinuity == 1)
        gUtility.MyStrCpy(continuity, 20, "on");
    else
        gUtility.MyStrCpy(continuity, 20, "off");

    if (gpConfig->gBC[nBoardNum].gnDateTimeOffset == 1)
        gUtility.MyStrCpy(timeoffset, 20, "on");
    else if (gpConfig->gBC[nBoardNum].gnDateTimeOffset == 2)
        gUtility.MyStrCpy(timeoffset, 20, "on(current time)");
    else if(gpConfig->gBC[nBoardNum].gnDateTimeOffset == 3)
		gUtility.MyStrCpy(timeoffset, 20, "on(user time)");
	else
        gUtility.MyStrCpy(timeoffset, 20, "off");
	//TDT/TOT - USER DATE/TIME
	//DATE
	gUtility.MyStrCpy(userdate, sizeof(userdate), gpConfig->gBC[nBoardNum].gnDateTimeOffset_Date);
	//TIME
	gUtility.MyStrCpy(usertime, sizeof(usertime), gpConfig->gBC[nBoardNum].gnDateTimeOffset_Time);


    // reset playrate
    if (gpConfig->gBC[nBoardNum].gnCalcPlayRate == 1)
        gUtility.MyStrCpy(CalcPlayRate, 20, "on");
    else
        gUtility.MyStrCpy(CalcPlayRate, 20, "off");
    
    // a/v decoding
    if (gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1)
        gUtility.MyStrCpy(UseAVDecoding, 20, "on");
    else
        gUtility.MyStrCpy(UseAVDecoding, 20, "off");

    // IP streaming
    if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1)
        gUtility.MyStrCpy(UseIPStreaming, 20, "on");
    else
        gUtility.MyStrCpy(UseIPStreaming, 20, "off");

    // TVB595V1
    if (gpConfig->gBC[nBoardNum].gnBypassAMP == 1)
        gUtility.MyStrCpy(BypassAMP, 20, "on");
    else
        gUtility.MyStrCpy(BypassAMP, 20, "off");

    // auto run type
    if (nRunType == 0)
        gUtility.MyStrCpy(AutoRun, 20, "play");
    else if (nRunType == 1)
        gUtility.MyStrCpy(AutoRun, 20, "record");
    else
        gUtility.MyStrCpy(AutoRun, 20, "monitor");
    
    if (gpConfig->gBC[nBoardNum].gnOutputClockSource == 1)
        gUtility.MyStrCpy(Max_Playrate, 20, "on");
    else
        gUtility.MyStrCpy(Max_Playrate, 20, "off");

	//2010/12/14 =====================================================================================================================
	//TMCC use
	if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 0)
		gUtility.MyStrCpy(UseTMCC, 20, "on");
	else
		gUtility.MyStrCpy(UseTMCC, 20, "off");

	//Multi-TS Combiner
	if(gpConfig->gBC[nBoardNum].gnUseTMCCRemuxer == 1)
		gUtility.MyStrCpy(MultiTS, 20, "on");
	else
		gUtility.MyStrCpy(MultiTS, 20, "off");
	
	//Emergency-alarm broadcasting
	if(gpConfig->gBC[nBoardNum].gnEmergencyBroadcasting == 1)
		gUtility.MyStrCpy(EmergencyAlarm, 20, "on");
	else
		gUtility.MyStrCpy(EmergencyAlarm, 20, "off");
	//================================================================================================================================


	gUtility.MySprintf(str, 200, " -slot %d -folder %s\\ %s -mode %s -input-source %s -playrate %d", nBoardNum, 
						gpConfig->gBC[nBoardNum].gszMasterDirectory, TargetFile, RepeatMode, ModulatorSource, gpConfig->gBC[nBoardNum].gdwPlayRate);
	gUtility.MySprintf(str1, 200, " -type %s -rf %d -if %s -symbol %d %s", ModType, gpConfig->gBC[nBoardNum].gnRFOutFreq,
						IFvalue, gpConfig->gBC[nBoardNum].gnSymbolRate, CodeRate);
	gUtility.MySprintf(str2, 200, " %s -bw %s -tx %s -guard %s", Constellation, Bandwidth, Txmode, GuardInterval);
	gUtility.MySprintf(str3, 200, " %s -frame-header %s -carrier-number %s", QAMInterleave, FrameHeader, CarrierNumber);
	gUtility.MySprintf(str4, 200, " -frame-header-pn %s -pilot-insert %s -spectrum %s -noise-mode %s", FrameHeaderPN, 
						PilotInsertion, SpectrumInverse, PRBSmode);
	gUtility.MySprintf(str5, 200, " -noise-power %.1f -atten %.1f -ext-atten %.1f", gpConfig->gBC[nBoardNum].gnPRBSscale, 
						gpConfig->gBC[nBoardNum].gdwAttenVal, gpConfig->gBC[nBoardNum].gdwAttenExtVal);
	gUtility.MySprintf(str6, 200, " -mpe-fec %s -time-slice %s -in-depth %s -cell-id %d", mpe_fec, time_slice, in_depth, 
						gpConfig->gBC[nBoardNum].gnCell_Id);
	gUtility.MySprintf(str7, 200, " -use-tmcc %s -multi-ts %s -emergency-alarm %s", UseTMCC, MultiTS, EmergencyAlarm);
	gUtility.MySprintf(str8, 200, " -pilot %s -roll-off %s", Pilot, RollOffFactor);
    //2010/6/10
	gUtility.MySprintf(str9, 200, " -l1-mod %d -t2mi-guard %d -t2mi-fft %d -t2mi-plp-mod %d", gpConfig->gBC[nBoardNum].gnT2MI_L1_MOD, gpConfig->gBC[nBoardNum].gnT2MI_GUARD, gpConfig->gBC[nBoardNum].gnT2MI_FFT, gpConfig->gBC[nBoardNum].gnT2MI_PLP_MOD[0]);
    gUtility.MySprintf(str10, 200, " -t2mi-plp-cod %d -t2mi-plp-fec-type %d -t2mi_plp_num_block %d", gpConfig->gBC[nBoardNum].gnT2MI_PLP_COD[0], gpConfig->gBC[nBoardNum].gnT2MI_PLP_FEC_TYPE[0], gpConfig->gBC[nBoardNum].gnT2MI_PLP_NUM_BLOCK[0]);
    gUtility.MySprintf(str11, 200, " -t2mi-num-data-symbol %d -t2mi-pilot-pattern %d -t2mi-bw %d", gpConfig->gBC[nBoardNum].gnT2MI_NUM_DATA_SYMBOL, gpConfig->gBC[nBoardNum].gnT2MI_PILOT_PATTERN, gpConfig->gBC[nBoardNum].gnT2MI_BW);
    gUtility.MySprintf(str12, 200, " -t2mi-bwt %d -t2mi-hem %d -t2mi-adjustment-flag %d", gpConfig->gBC[nBoardNum].gnT2MI_BWT, gpConfig->gBC[nBoardNum].gnT2MI_PLP_HEM[0], gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag);

	
	gUtility.MySprintf(str13, 200, " -max-playrate %s -null-tp %s -restamping %s -continuity %s", Max_Playrate, StopMode, restamping, continuity);
    gUtility.MySprintf(str14, 200, " -timeoffset %s -user-date %s -user-time %s -reset-playrate %s ", timeoffset, userdate, usertime, CalcPlayRate);
    gUtility.MySprintf(str15, 200, " -av-decoding %s -ip-streaming %s -bypass-amp %s -auto-run %s ", UseAVDecoding, UseIPStreaming, BypassAMP, AutoRun);

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
	    gUtility.MySprintf(strArg,4096, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", str,str1,str2,str3,str4,str5,str6,str7,str8,str9,str10,str11,str12,str13,str14,str15);
	}
	else
	{
	    gUtility.MySprintf(strArg,4096, "%s%s%s%s%s%s%s%s%s%s%s%s", str,str1,str2,str3,str4,str5,str6,str7,str8,str13,str14,str15);
	}
    return strArg;
}

//---------------------------------------------------------------------------
// called from Form_LICENSE_INPUT
// Write LN data to license.dat
#ifdef WIN32
void CUTIL_IND::OpenAndAppendLNData(char *ln_data)
#else
void CUTIL_IND::OpenAndAppendLNData(char *ln_data, char *sn_data, int opt)
#endif
#ifdef WIN32 
{
    FILE    *hFile = NULL;
    char    strPath[256];

    gUtility.MySprintf(strPath, 256, "%s\\license.dat", gGeneral.gnStrCurDir);
	hFile = gUtility.MyFopen(hFile, strPath,"a");
    if (hFile == NULL)
        return;

    fwrite(ln_data, 1, strlen(ln_data), hFile);
    fwrite("\n", 1, 1, hFile);

    fclose(hFile);
}

#else 
{
    FILE    *hFile = NULL;
    char    strPath[256];

 #ifdef WIN32 
    gUtility.MySprintf(strPath, 256, "%s\\license.dat", gGeneral.gnStrCurDir);
#else
#ifdef STANDALONE
    gUtility.MySprintf(strPath, 256, (char *) "/sysdb/license.dat");
#else
	//2010/9/28
	FILE	*hFile2 = NULL;
	char	strPathBak[256];
    sprintf(strPath,  (char *) "%s/license.dat", gGeneral.gnStrCurDir);
	//2010/9/28
	sprintf(strPathBak, (char *)"%s/license.bak", gGeneral.gnStrCurDir);
	hFile2 = gUtility.MyFopen(hFile2, strPathBak, (char *)"r");
	if(hFile2 != NULL)
	{
		fclose(hFile2);
		remove(strPathBak);
	}
	int res = rename(strPath, strPathBak);
	if(res < 0)
	{
		printf("Fail to make License file 1!\n");
		return;
	}
	hFile2 = gUtility.MyFopen(hFile2, strPathBak, (char *)"r");
	if(hFile2 == NULL)
	{
		printf("Fail to make License file 2!\n");
		return;
	}
	fseek(hFile2, 0, SEEK_SET);
#endif
#endif
#ifdef STANDALONE
	hFile = gUtility.MyFopen(hFile, strPath,(char *) "w");
#else
	hFile = gUtility.MyFopen(hFile, strPath,(char *) "a");
#endif
	if (hFile == NULL)
        return;
#ifndef STANDALONE
#ifndef WIN32
	char	strLine[256];
	char	strTempName[256];
	char	strTempValue[256];
	int		writeflag = 0;
	memset(strLine, 0x00, sizeof(strLine));
	sprintf(strTempValue,  (char *) "%s                'SN:%s, OPTION:%d\n", ln_data, sn_data, opt);
	while (fgets(strLine, 256, hFile2) != NULL)
	{
		for (int i = 0; i < strlen(strLine); i++)
		{
			if (strLine[i] == 'S')
			{
				strncpy(strTempName, strLine + i + 3, 16);
				strTempName[16] = '\0';
				if (strcmp(strTempName, sn_data) == 0)
				{
					writeflag = 1;
				    fwrite(strTempValue, 1, strlen(strTempValue), hFile);

				}
				else
				{
				    fwrite(strLine, 1, strlen(strLine), hFile);
				}
				break;	// break for loop. goto net line
			}
		}
		memset(strLine, 0x00, sizeof(strLine));
	}
	if(writeflag == 0)
	{
	    fwrite(strTempValue, 1, strlen(strTempValue), hFile);
	}
	fclose(hFile2);
#else
    fwrite(ln_data, 1, strlen(ln_data), hFile);
    fwrite("\n", 1, 1, hFile);
#endif
//TVB597LAN
#else
    fwrite(ln_data, 1, strlen(ln_data), hFile);
    fwrite("\n", 1, 1, hFile);
#endif
    fclose(hFile);
}
#endif

//---------------------------------------------------------------------------
// return Frequency in Hz
unsigned long CUTIL_IND::Get_VSB_Frequency(long bCATV, long chNum)
{
    unsigned long    lFreq = 0;
    
    // === Check Channel Range
    if (bCATV == 2)         //STD. CABLE(2~125)
    {
        if (chNum < 2 || chNum > 125)
            return lFreq;

        if (chNum <= 4)
            lFreq = 57000000 + (chNum - 2) * 6000000;
        else if (chNum <= 6)
            lFreq = 79000000 + (chNum - 5) * 6000000;
        else if (chNum <= 13)
            lFreq = 177000000 + (chNum - 7) * 6000000;
        else if (chNum <= 22)
            lFreq = 123000000 + (chNum - 14) * 6000000;
        else if (chNum <= 94)
            lFreq = 219000000 + (chNum - 23) * 6000000;
        else if (chNum <= 99)
            lFreq = 93000000 + (chNum - 95) * 6000000;
        else if (chNum <= 125)
            lFreq = 651000000 + (chNum - 100) * 6000000;
    } else if (bCATV == 3)  // HRC. CABLE(1~125)
    {
        if (chNum < 1 || chNum > 125)
            return lFreq;

        if (chNum == 1)
            lFreq = 73750000;
        else if (chNum >= 2 && chNum <= 4)
            lFreq = 57000000 + (chNum - 2) * 6000000 - 1250000;
        else if (chNum == 5)
            lFreq = 79750000;
        else if (chNum == 6)
            lFreq = 85750000;
        else if (chNum <= 13)
            lFreq = 177000000 + (chNum - 7) * 6000000 - 1250000;
        else if (chNum <= 22)
            lFreq = 123000000 + (chNum - 14) * 6000000 - 1250000;
        else if (chNum <= 94)
            lFreq = 219000000 + (chNum - 23) * 6000000 - 1250000;
        else if (chNum <= 99)
            lFreq = 93000000 + (chNum - 95) * 6000000 - 1250000;
        else if (chNum <= 125)
            lFreq = 651000000 + (chNum - 100) * 6000000 - 1250000;
    } else if (bCATV == 4)  // IRC. CABLE(1~125)
    {
        if (chNum < 1 || chNum > 125)
            return lFreq;
        
        if (chNum == 1)
            lFreq = 75000000;
        else if (chNum <= 4)
            lFreq = 57000000 + (chNum - 2) * 6000000;
        else if (chNum == 5)
            lFreq = 81000000;
        else if (chNum == 6)
            lFreq = 87000000;
        else if (chNum <= 13)
            lFreq = 177000000 + (chNum - 7) * 6000000;
        else if (chNum <= 22)
            lFreq = 123000000 + (chNum - 14) * 6000000;
        else if (chNum <= 94)
            lFreq = 219000000 + (chNum - 23) * 6000000;
        else if (chNum <= 99)
            lFreq = 93000000 + (chNum - 95) * 6000000;
        else if (chNum <= 125)
            lFreq = 651000000 + (chNum - 100) * 6000000;
	}
    else    // BROADCAST
	{
		if(chNum < 2 || chNum > 69)
            return lFreq;

        if(chNum <= 4)
            lFreq = 57000000 + (chNum - 2) * 6000000;
        else if(chNum <= 6)
            lFreq = 79000000 + (chNum - 5) * 6000000;
        else if(chNum <= 13)
            lFreq = 177000000 + (chNum - 7) * 6000000;
        else if(chNum <= 69)
            lFreq = 473000000 + (chNum - 14) * 6000000;
	}
    return lFreq;
}

//---------------------------------------------------------------------------
// return Frequency in Hz
unsigned long CUTIL_IND::Get_OFDM_Frequency(long bCATV, long codeNum, long chNum)
{
    unsigned long lFreq = 0;
    long nBandwidth = 6;        // MHz

    // EUROPE:0, AUSTRALIA:61, GERMANY:49, FRANCE:33, CHINA:86, ITALY:39
    switch (codeNum)
    {
        case 0:
        case 33:
        case 39:
        case 49:
        case 61:
            if (bCATV == 1)
            {
                nBandwidth = 7;
                if (chNum >= 1 && chNum <= 3)
                    lFreq = 50500000 + (chNum - 1) * (nBandwidth) * 1000000;
                else if (chNum >= 17 && chNum <= 24)
                    lFreq = 177500000 + (chNum - 17) * (nBandwidth) * 1000000;
                else if (chNum >= 4 && chNum <= 6)
                    lFreq = 71500000 + (chNum - 4) * (nBandwidth) * 1000000;
                else if (chNum >= 7 && chNum <= 16)
                    lFreq = 107500000 + (chNum - 7) * (nBandwidth) * 1000000;
                else if (chNum >= 25 && chNum <= 34)
                    lFreq = 233500000 + (chNum - 25) * (nBandwidth) * 1000000;
                else if (chNum >= 35 && chNum <= 55)
                {
                    nBandwidth = 8;
                    lFreq = 306000000 + (chNum - 35) * (nBandwidth) * 1000000;
                }
            } else
            {
                if (codeNum == 0 || codeNum == 49)      //GERMANY
                {
                    if (chNum >= 5 && chNum <= 12)
                    {
                        nBandwidth = 7;
                        lFreq = 177500000 + (chNum - 5) * (nBandwidth) * 1000000;
                    }  else if (chNum >= 21 && chNum <= 69)
                    {
                        nBandwidth = 8;
                        lFreq = 474000000 + (chNum - 21) * (nBandwidth) * 1000000;
                    }
                } else if (codeNum == 33)               //FRANCE
                {
                    nBandwidth = 8;
                    if (chNum >= 0 && chNum <= 2)       // A,B,C
                        lFreq = 45000000 + chNum * (nBandwidth) * 1000000;
                    else if (chNum == 3)                // C1
                        lFreq = 57750000;
                    else if (chNum >= 4 && chNum <= 9)   // 1~6
                        lFreq = 178750000 + (chNum - 4) * (nBandwidth) * 1000000;
                    else if (chNum >= 21 && chNum <= 69)
                        lFreq = 474000000 + (chNum - 21) * (nBandwidth) * 1000000;
                } else if (codeNum == 39)               //ITALY
                {
                    if (chNum >= 14 && chNum <= 20)     // D,E,F,G,H,H1,H2
                    {
                        nBandwidth = 7;

                        if (chNum == 14)
                            lFreq = 177500000;
                        else if (chNum == 15)
                            lFreq = 186000000;
                        else if (chNum == 16)
                            lFreq = 194500000;
                        else if (chNum == 17)
                            lFreq = 203500000;
                        else if (chNum == 18)
                            lFreq = 212500000;
                        else if (chNum == 19)
                            lFreq = 219500000;
                        else if (chNum == 20)
                            lFreq = 226500000;
                    }
                    else if (chNum >= 21 && chNum <= 69)
                    {
                        nBandwidth = 8;
                        lFreq = 474000000 + (chNum - 21) * (nBandwidth) * 1000000;
                    }
                } else if (codeNum == 61)               //AUSTRALIA
                {
                    nBandwidth = 7;
                    if (chNum >= 6 && chNum <= 8)
                    {
                        lFreq = 135500000 + chNum * (nBandwidth) * 1000000;
                        if (chNum == 8)
                            lFreq = lFreq + 125000;
                    }
                    else if (chNum == 9)
                        lFreq = 142500000 + (chNum - 1) * (nBandwidth) * 1000000;
                    else if (chNum >= 10 && chNum <= 12)
                        lFreq = 142500000 + chNum * (nBandwidth) * 1000000;
					else if (chNum == 13)       // 9A
                        lFreq = 142500000 + 9 * nBandwidth * 1000000;
                    else if (chNum >= 28 && chNum <= 69)
					{
                        lFreq = 333500000 + chNum * (nBandwidth) * 1000000;
						if(chNum == 29)
							lFreq = lFreq + 125000;
					}
				}
            }
            break;

        case 86:        // CHINA
            nBandwidth = 8;
            if (bCATV == 1)
            {
                // chNum : 1~37
                if (chNum >= 1 && chNum <= 7)
                    lFreq = 115000000 + (chNum - 1) * (nBandwidth) * 1000000;
                else if (chNum >= 8)
                    lFreq = 227000000 + (chNum - 8) * (nBandwidth) * 1000000;
            } else
            {
                // chNum : 1~68
                if (chNum >= 1 && chNum <= 5)
                    lFreq = 52500000 + (chNum - 1) * (nBandwidth) * 1000000;
                else if(chNum >= 6 && chNum <= 12)
                    lFreq = 17100000 + (chNum - 6) * (nBandwidth) * 1000000;
                else if(chNum >= 13 && chNum <= 24)
                    lFreq = 474000000 + (chNum - 13) * (nBandwidth) * 1000000;
                else if(chNum >= 25)
                    lFreq = 610000000 + (chNum - 25) * (nBandwidth) * 1000000;
            }
            break;
    }
    return lFreq;
}

//---------------------------------------------------------------------------
// return Frequency in Hz
unsigned long CUTIL_IND::Get_TDMB_Frequency(long channelType, long chNum)
{
    unsigned long    lFreq = 0;

    if (channelType == 13)      //    ' Korea
    {
        if (chNum == 0)         //      '7A
            lFreq = 175280000;
        else if (chNum == 1)    //      '7B
            lFreq = 177008000;
        else if (chNum == 2)    //      '7c
            lFreq = 178736000;
        else if (chNum == 3)    //      '8A
            lFreq = 181280000;
        else if (chNum == 4)    //      '8B
            lFreq = 183008000;
        else if (chNum == 5)    //      '8C
            lFreq = 184736000;
        else if (chNum == 6)    //      '9A
            lFreq = 187280000;
        else if (chNum == 7)    //      '9B
            lFreq = 189008000;
        else if (chNum == 8)    //      '9C
            lFreq = 190736000;
        else if (chNum == 9)    //      '10A
            lFreq = 193280000;
        else if (chNum == 10)    //      '10B
            lFreq = 195008000;
        else if (chNum == 11)    //      '10C
            lFreq = 196736000;
        else if (chNum == 12)    //      '11A
            lFreq = 199280000;
        else if (chNum == 13)    //      '11B
            lFreq = 201008000;
        else if (chNum == 14)    //      '11C
            lFreq = 202736000;
        else if (chNum == 15)    //      '12A
            lFreq = 205280000;
        else if (chNum == 16)    //      '12B
            lFreq = 207008000;
        else if (chNum == 17)    //      '12C
            lFreq = 208736000;
        else if (chNum == 18)    //      '13A
            lFreq = 211280000;
        else if (chNum == 19)    //      '13B
            lFreq = 213008000;
        else if (chNum == 20)    //      '13C
            lFreq = 214736000;
    } else if (channelType == 12)       //  ' Canada, 1~23
    {
        chNum = chNum + 1;
        if (chNum >= 1 && chNum <= 23)
            lFreq = 1452816000 + (chNum - 1) * 1744000;
    } else if (channelType == 11)       //  ' L-Band
    {
        if (chNum >= 0 && chNum <= 22)
            lFreq = 1452960000 + chNum * 1712000;
    } else if (channelType == 10)       //  ' Band III
    {
        if (chNum >= 0 && chNum <= 3)
            lFreq = 174928000 + chNum * 1712000;
        else if (chNum >= 4 && chNum <= 7)
            lFreq = 181936000 + (chNum - 4) * 1712000;
        else if (chNum >= 8 && chNum <= 11)
            lFreq = 188928000 + (chNum - 8) * 1712000;
        else if (chNum >= 12 && chNum <= 15)
            lFreq = 195936000 + (chNum - 12) * 1712000;
        else if (chNum >= 16 && chNum <= 19)
            lFreq = 202928000 + (chNum - 16) * 1712000;
        else if (chNum >= 20 && chNum <= 23)
            lFreq = 209936000 + (chNum - 20) * 1712000;
        else if (chNum >= 24 && chNum <= 27)
            lFreq = 216928000 + (chNum - 24) * 1712000;
        else if (chNum >= 28 && chNum <= 31)
            lFreq = 223936000 + (chNum - 28) * 1712000;
        else if (chNum >= 32 && chNum <= 34)
            lFreq = 230784000 + (chNum - 32) * 1712000;
        else if (chNum >= 35 && chNum <= 37)
            lFreq = 235776000 + (chNum - 35) * 1712000;
    }
    return lFreq;
}

//---------------------------------------------------------------------------
// return Frequency in Hz
unsigned long CUTIL_IND::Get_ISDBT_Frequency(long codeNum, long chNum)
{
    unsigned long    lFreq = 0;

    switch (codeNum)
    {
        case 81:    // ' JAPAN
            if (chNum < 0 || chNum > 49)    // 13~62
                return lFreq;

            lFreq = 473143000 + chNum * 6000000;
            break;

        case 55:    // ' BRAZIL
            if (chNum >= 0 && chNum <= 6)   // 7 ~ 13
                lFreq = 177143000 + chNum * 6000000;
            if (chNum >= 7 && chNum <= 62)
                lFreq = 473143000 + (chNum - 7) * 6000000;
            break;
   }
   return lFreq;
}

//---------------------------------------------------------------------------
// return Frequency in Hz
unsigned long CUTIL_IND::Get_DTMB_Frequency(long chNum)
{
    unsigned long    lFreq = 0;

    if (chNum < 0 || chNum > 43)    // 13~56
        return lFreq;

    if (chNum >= 0 && chNum <= 11)
        lFreq = 474000000 + chNum * 8000000;
    else
        lFreq = 610000000 + (chNum - 12) * 8000000;

    return lFreq;
}
//2011/11/22 added ISDB-S CHANNEL
unsigned long CUTIL_IND::Get_ISDBS_Frequency(long chNum)
{
    unsigned long    lFreq = 0;

    if (chNum < 0 || chNum > 23)    // 13~56
        return lFreq;

    if (chNum >= 0 && chNum <= 11)
        lFreq = 1049480000 + chNum * 38360000;
    else
        lFreq = 1613000000 + (chNum - 12) * 40000000;

    return lFreq;
}


//---------------------------------------------------------------------------
void CUTIL_IND::Toggle_Video_Size(long nBoardNum)
{
    if (gpConfig->gBC[nBoardNum].bPlayingProgress == true ||
        gpConfig->gBC[nBoardNum].bRecordInProgress == true)
    {
        if (gpConfig->gBC[nBoardNum].gnUseIPStreaming == 1 ||
            gpConfig->gBC[nBoardNum].gnUseAVDecoding == 1)
        {
            if (gpConfig->gBC[nBoardNum].gnVideoWidth == VLC_VIDEO_WIDTH_S)
            {
                gpConfig->gBC[nBoardNum].gnVideoWidth = VLC_VIDEO_WIDTH_M;
                gpConfig->gBC[nBoardNum].gnVideoHeight = VLC_VIDEO_HEIGHT_M;
            } else if (gpConfig->gBC[nBoardNum].gnVideoWidth == VLC_VIDEO_WIDTH_M)
            {
                gpConfig->gBC[nBoardNum].gnVideoWidth = VLC_VIDEO_WIDTH_L-120;
                gpConfig->gBC[nBoardNum].gnVideoHeight = VLC_VIDEO_HEIGHT_L-117;
            } else
            {
                gpConfig->gBC[nBoardNum].gnVideoWidth = VLC_VIDEO_WIDTH_S;
                gpConfig->gBC[nBoardNum].gnVideoHeight = VLC_VIDEO_HEIGHT_S;
            }
        }
    }
}

//---------------------------------------------------------------------------
char *CUTIL_IND::Get_BERT_String(long nBoardNum)
{
    char    str[100];

    gUtility.MyStrCpy(str, 100, "");
    if (gpConfig->gBC[nBoardNum].gnBertPacketType != NO_BERT_OPERTION)
    {
        if (gpConfig->gBC[nBoardNum].gnBertPacketType == SERIAL_PRBS_2_15)
            gUtility.MyStrCpy(str,100, "SERIAL/2^15-1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == SERIAL_PRBS_2_23)
            gUtility.MyStrCpy(str,100, "SERIAL/2^23-1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_HEAD_184_ALL_0)
            gUtility.MyStrCpy(str,100, "TS/184/0");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_HEAD_184_ALL_1)
            gUtility.MyStrCpy(str,100, "TS/184/1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_HEAD_184_PRBS_2_15)
            gUtility.MyStrCpy(str,100, "TS/184/2^15-1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_HEAD_184_PRBS_2_23)
            gUtility.MyStrCpy(str,100, "TS/184/2^23-1");

        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_ALL_0)
            gUtility.MyStrCpy(str,100, "TS/187/0");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_ALL_1)
            gUtility.MyStrCpy(str,100, "TS/187/1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_PRBS_2_15)
            gUtility.MyStrCpy(str,100, "TS/187/2^15-1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_SYNC_187_PRBS_2_23)
            gUtility.MyStrCpy(str,100, "TS/187/2^23-1");

        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_STUFFING_184_ALL_0)
            gUtility.MyStrCpy(str,100, "TS(NULL)/184/0");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_STUFFING_184_ALL_1)
            gUtility.MyStrCpy(str,100, "TS(NULL)/184/1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_STUFFING_184_PRBS_2_15)
            gUtility.MyStrCpy(str,100, "TS(NULL)/184/2^15-1");
        else if (gpConfig->gBC[nBoardNum].gnBertPacketType == TS_STUFFING_184_PRBS_2_23)
            gUtility.MyStrCpy(str,100, "TS(NULL)/184/2^23-1");
    }

	gUtility.MyStrCpy(gstrGeneral, 256, str);
    return gstrGeneral;
}


//------------------------------------------------------------------------------
// return file name only 

void CUTIL_IND::Get_Filename_From_FullPath(char *strFullPath, char *strDirectory, char *strFile)
{
    int  i;
    int  iLen = (int) (strlen(strFullPath));

    gUtility.MyStrCpy(strDirectory, 256, strFullPath);
    for (i = iLen-1; i >= 0; i--)
    {
        if (strFullPath[i] == '\\')
            break;
    }

#if defined(WIN32)
    strncpy_s(strFile, 256, strFullPath+i+1, (iLen-i));
#else
	strncpy(strFile, strFullPath+i+1, (iLen-i));
#endif
    strFile[iLen-i] = '\0';

    if (i > 2)      // C:\xxx...
        strDirectory[i] = '\0';
    else            
        strDirectory[i+1] = '\0';
}

//---------------------------------------------------------------------------
// return commaed string
char *CUTIL_IND::Get_Commaed_String(int iDigit)
{
    int     iTemp;

    if (iDigit < 1000)
        gUtility.MySprintf(gstrGeneral,  256, "%d", iDigit);
    else if (iDigit < 1000000)
        gUtility.MySprintf(gstrGeneral, 256, "%d,%03d", iDigit/1000, iDigit % 1000);
    else
    {
        iTemp = iDigit/1000;
        gUtility.MySprintf(gstrGeneral, 256, "%d,%03d,%03d", iTemp/1000, iTemp % 1000, iDigit % 1000);
    }
    return gstrGeneral;
}

//---------------------------------------------------------------------------
// set gpConfig->gBC[].dwFileSize
//     gGeneral.gnRunTime
// ShowFileSize = GetFileSizeInSec + Display_LabFileSize
void CUTIL_IND::GetFileSizeInSec(char *szFileName, long nBoardNum)
{
#ifdef WIN32
    HANDLE  hFile;
    LARGE_INTEGER	uliSize;

    gpConfig->gBC[nBoardNum].dwFileSize = 0;

    hFile = CreateFile(szFileName,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    GetFileSizeEx(hFile, &uliSize);
	gpConfig->gBC[nBoardNum].dwFileSize = (__int64) uliSize.QuadPart;

#else 
	gpConfig->gBC[nBoardNum].dwFileSize = Get_File_Size_BYTE(szFileName);
#endif
    if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    {
	    gpConfig->gBC[nBoardNum].dwFileSize = (__int64) ((gpConfig->gBC[nBoardNum].dwFileSize/6144.0)*7300);
    }

    if (gpConfig->gBC[nBoardNum].gdwPlayRate >= 1)
    {
		//2011/4/7 ISDB-S Bitrate
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
		{
			if(gGeneral.gnPacketSize == 204)
#ifdef WIN32
				gpConfig->gBC[nBoardNum].gnPlaybackTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0 * 188.0 / 204.0);
#else
				gGeneral.gnRunTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0 * 188.0 / 204.0);
#endif
			else
#ifdef WIN32
				gpConfig->gBC[nBoardNum].gnPlaybackTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0);
#else
				gGeneral.gnRunTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0);
#endif
		}
		else
#ifdef WIN32
	        gpConfig->gBC[nBoardNum].gnPlaybackTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/(double)gpConfig->gBC[nBoardNum].gdwPlayRate);
#else
	        gGeneral.gnRunTime = (long) ((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/(double)gpConfig->gBC[nBoardNum].gdwPlayRate);
#endif

		//kslee 2010/3/18
		//PCR RESTAMP
		if(gpConfig->gBC[nBoardNum].gnPCR_Restamping == 1 || gpConfig->gBC[nBoardNum].gnBitrate_Adjustment_Flag == 1)
		{
			if(gGeneral.gnBitrate > 0)
			{
				//2011/4/7 ISDB-S Bitrate
				if(gpConfig->gBC[nBoardNum].gnModulatorMode == ISDB_S && gpConfig->gBC[nBoardNum].gnCombinedTS == 1)
				{
					if(gGeneral.gnPacketSize == 204)
#ifdef WIN32
						gpConfig->gBC[nBoardNum].gnPlaybackTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0 * 188.0 / 204.0);
#else
						gGeneral.gnRunTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0 * 188.0 / 204.0);
#endif
					else
#ifdef WIN32
						gpConfig->gBC[nBoardNum].gnPlaybackTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0);
#else
						gGeneral.gnRunTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/52170000.0);
#endif
				}
				else
#ifdef WIN32
					gpConfig->gBC[nBoardNum].gnPlaybackTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/(double)gGeneral.gnBitrate);
#else
					gGeneral.gnRunTime = (long)((double)gpConfig->gBC[nBoardNum].dwFileSize*8.0/(double)gGeneral.gnBitrate);
#endif
			}
		}
    }
#ifdef WIN32
    CloseHandle(hFile);
#else
#endif
}

//---------------------------------------------------------------------------
long CUTIL_IND::GetCurrentTimeinMsec(long nBoardNum)
{
    // 0~24 hour in msec count, return 0~863990
    long    dwCurHour, dwCurMin, dwCurSec, dwLastTime;
    long    lRet = 0;

    if (gpConfig->gBC[nBoardNum].dwMsecCounter == 0)
    {
        time_t      tCur;
        struct tm   newtime;

        tCur = time(NULL);
#if defined(WIN32)
		localtime_s(&newtime, &tCur);
#else
		localtime_r(&tCur, &newtime);
#endif

        dwCurHour = newtime.tm_hour;
        dwCurMin = newtime.tm_min;
        dwCurSec = newtime.tm_sec;
        
        dwLastTime = (dwCurHour * 36000) + (dwCurMin * 600) + (dwCurSec * 10);
        if (dwLastTime < 0 || dwLastTime > 863990)
            dwLastTime = 0;

        gpConfig->gBC[nBoardNum].dwLastTimeInMsec = dwLastTime;    //0~863990
        
        if (gpConfig->gBC[nBoardNum].dwLastTimeInMsec < gpConfig->gBC[nBoardNum].dwStartTime)
            gpConfig->gBC[nBoardNum].dwLastTimeInMsec = gpConfig->gBC[nBoardNum].dwLastTimeInMsec + 863990;
    }

    lRet = gpConfig->gBC[nBoardNum].dwLastTimeInMsec + gpConfig->gBC[nBoardNum].dwMsecCounter;
#ifdef WIN32
    gpConfig->gBC[nBoardNum].dwMsecCounter = (gpConfig->gBC[nBoardNum].dwMsecCounter + 1) % 10;
#else
    gpConfig->gBC[nBoardNum].dwMsecCounter = (gpConfig->gBC[nBoardNum].dwMsecCounter + 5) % 10;		// depend on MainTimer Sleep()
#endif

    return lRet;
}

//---------------------------------------------------------------------------
// Return Filesize in KB
__int64 CUTIL_IND::Get_File_Size_KB(char *strFilename)
{
#if defined(WIN32)
    HANDLE          hFile;
    LARGE_INTEGER	uliSize;
    //kslee 2010/4/2 
	//long            lSizeKB;
	__int64			lSizeKB;
    hFile = CreateFile( strFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	GetFileSizeEx(hFile, &uliSize);
	lSizeKB = (int) (uliSize.QuadPart/1000);
	CloseHandle(hFile);
#else
	FILE	*hFile;
	__int64	lSize;		//kslee long -> __int64
	__int64	lSizeKB;	//kslee long -> __int64

	hFile = fopen(strFilename,"rb");
	if (hFile == NULL)
		return -1;

	fseeko(hFile, 0, SEEK_END);
	lSize = ftello(hFile); //kslee 2010/4/8 ftell -> ftello

	lSizeKB = (__int64) (lSize/1000);
	fclose(hFile);
#endif
    return lSizeKB;
}

//---------------------------------------------------------------------------
// Return Filesize in BYTE
__int64 CUTIL_IND::Get_File_Size_BYTE(char *strFilename)
{
#if defined(WIN32)
    HANDLE          hFile;
    LARGE_INTEGER	uliSize;
    __int64         lSizeBYTE;

    hFile = CreateFile( strFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	GetFileSizeEx(hFile, &uliSize);
	lSizeBYTE = (__int64) (uliSize.QuadPart);
	CloseHandle(hFile);
#else
	FILE	*hFile;
	__int64 lSize;
    __int64         lSizeBYTE;

	hFile = fopen(strFilename,"rb");
	if (hFile == NULL)
		return -1;

	fseeko(hFile, 0, SEEK_END);
	lSize = ftello(hFile);
	lSizeBYTE = (__int64) lSize;
	fclose(hFile);
#endif
    return lSizeBYTE;
}

//---------------------------------------------------------------------------
// strTime = hh:mm:ss
long CUTIL_IND::Get_Sec_From_HHMMSS(char *strTime)
{
    long    lRet = 0;
    int     iLen = (int) (strlen(strTime));
    char    str[10];
    int     iH, iM, iS;

    if (iLen != 8)
        return lRet;

    str[0] = strTime[0];
    str[1] = strTime[1];
    str[2] = '\0';
    iH = atoi(str);

    str[0] = strTime[3];
    str[1] = strTime[4];
    str[2] = '\0';
    iM = atoi(str);

    str[0] = strTime[6];
    str[1] = strTime[7];
    str[2] = '\0';
    iS = atoi(str);

    lRet = iH * 60 * 60 + iM * 60 + iS;
    return lRet;
}

//---------------------------------------------------------------------------
long CUTIL_IND::Get_Current_SecTime()
{
    time_t      tCur;
#ifdef WIN32
    struct tm   nt;
    long        lSec = 0;

    tCur = time(NULL);
    localtime_s(&nt, &tCur);
    lSec = nt.tm_hour*3600 + nt.tm_min*60 + nt.tm_sec;
#else
	struct	timeval	tv;
    long lSec = 0;
	//struct	timezone	tz;

	gettimeofday(&tv, NULL);
	lSec = ((long long int)tv.tv_sec*(long long int)1000) + (long long int)(tv.tv_usec/1000);
#endif

    return lSec;
}

//---------------------------------------------------------------------------
// strDate = mm/dd/yyyy
// return MM DD YYYY
long CUTIL_IND::Get_MDY_From_MMDDYYYY(char *strDate, long *pMM, long *pDD, long *pYY)
{
    int     iLen = (int) (strlen(strDate));
    char    str[10];

    *pMM = 0;
    *pDD = 0;
    *pYY = 0;
    
    if (iLen != 10)
        return -1;

    str[0] = strDate[0];
    str[1] = strDate[1];
    str[2] = '\0';
    *pMM = atoi(str);

    str[0] = strDate[3];
    str[1] = strDate[4];
    str[2] = '\0';
    *pDD = atoi(str);

    str[0] = strDate[6];
    str[1] = strDate[7];
    str[2] = strDate[8];
    str[3] = strDate[9];
    str[4] = '\0';
    *pYY = atoi(str);

    return 1;
}

//------------------------------------------------------------------------------
// Get One Line and Return converted numeric
// return -1 if EOF or no data
long CUTIL_IND::Get_One_Line_From_File(FILE *hFile, char *strLine, int iSize)
{
    int    ch;
    int    ix = 0;
    long    lRet = -1;
    int     i;

    gUtility.MyStrCpy(strLine, iSize, "");

    if (hFile == NULL)
        return -1;

    //--- get one line
    ch = fgetc(hFile);

    //--- delete preceding LF/CCR
    while (ch == 10 || ch == 13)
    {
        ch = fgetc(hFile);
        if (feof(hFile))
            return -1;
    }

    for (i = 0; ((i < 256 ) && feof(hFile) == 0 ); i++)
    {
        strLine[ix++] = (char) ch;
        if (ch == 10 || ch == 13)       // LF or CR
        {   ix--;   // exlude LF/CR
            break;
        }
        ch = fgetc(hFile);
    }

    strLine[ix] = '\0';
    
	//2010/12/23 FIXED
	//lRet = atoi(strLine);
	lRet = 1;
    if (strlen(strLine) <= 0)   // EOF
        return -1;

    return lRet;
}

#ifdef WIN32
//2011/2/17 check 2byte characters
bool CUTIL_IND::Is_Correct_Extension(System::String ^extName)
{
    long        nBoardNum = gGeneral.gnActiveBoard;
	String^ strExt = extName->ToLower();

	if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
		if (strExt->StartsWith(".mfs"))
            return true;
        if (strExt->StartsWith(".mmx"))
            return true;
 	}
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    {
        if (strExt->StartsWith(".ni"))
            return true;
        if (strExt->StartsWith(".na"))
            return true;
        if (strExt->StartsWith(".eti"))
            return true;
    }
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
		//2011/2/19
		if(strExt->StartsWith(".t2mc"))
			return false;

        //2010/5/28
		//2010/12/23 DVB-T2 MULTI-PLP
		if (strExt->StartsWith(".trp"))
            return true;
        if (strExt->StartsWith(".ts"))
            return true;
        if (strExt->StartsWith(".atsc"))
            return true;
        if (strExt->StartsWith(".dtv"))
            return true;
        if (strExt->StartsWith(".mpg"))
            return true;
        if (strExt->StartsWith(".mpeg"))
            return true;

        if (strExt->StartsWith(".t2"))
            return true;
        if (strExt->StartsWith(".t2mi"))
            return true;
	}
	//2011/6/1 DVB-C2 MULTI PLP
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_C2)
	{
		if(strExt->StartsWith(".c2mi"))
			return false;

		if (strExt->StartsWith(".trp"))
            return true;
        if (strExt->StartsWith(".ts"))
            return true;
        if (strExt->StartsWith(".atsc"))
            return true;
        if (strExt->StartsWith(".dtv"))
            return true;
        if (strExt->StartsWith(".mpg"))
            return true;
        if (strExt->StartsWith(".mpeg"))
            return true;

        if (strExt->StartsWith(".c2"))
            return true;
	}
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{
		if (strExt->StartsWith(".iq"))
            return true;
	}
	//----------------
	else
    {
        if (strExt->StartsWith(".trp"))
            return true;
        if (strExt->StartsWith(".ts"))
            return true;
        if (strExt->StartsWith(".atsc"))
            return true;
        if (strExt->StartsWith(".dtv"))
            return true;
        if (strExt->StartsWith(".mpg"))
            return true;
        if (strExt->StartsWith(".mpeg"))
            return true;
		if(gpConfig->gBC[nBoardNum].gnModulatorMode == QPSK || gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_S2)
		{	if (strExt->StartsWith(".rec"))
				return true;
		}
    }

    return false;
}
#endif
//------------------------------------------------------------------------------
//        PlayForm.CommonOpen.Filter = "*.trp;*.ts;*.atsc;*.dtv;*.mpg;*.mpeg"
//        PlayForm.CommonOpen.Filter = "*.ni;*.na;*.eti"
bool CUTIL_IND::Is_Correct_Extension(char *strFile)
{
    int     i;
    int     iLen = (int) (strlen(strFile));
    char    strExt[100];
    char    str[100];
    long        nBoardNum = gGeneral.gnActiveBoard;
	//kslee 2010/1/12
	char	strExt2[100];

	if (iLen < 3)
		return false;

    for (i = iLen-1; i >= 0; i--)
    {
        if (strFile[i] == '.')
            break;
    }

	if (i < 0)			// no extension
		return false;

#if defined(WIN32)
    strncpy_s(strExt2, 100, strFile+i+1, (iLen-i-1));
#else
	strncpy(strExt2, strFile+i+1, (iLen-i-1));
#endif
    strExt2[iLen-i-1] = '\0';

	//kslee 2010/1/12
	for(i = 0; i < (int)strlen(strExt2); i++)
	{
		if((strExt2[i] >='a' && strExt2[i] <='z') || (strExt2[i] >='A' && strExt2[i] <='Z'))
		{
			strExt[i] = strExt2[i];
		}
		//kslee 2010/4/27
		else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2 && i == 1)
		{
			strExt[i] = strExt2[i];
		}
		else
		{
			if(i == 2)
			{
				strExt[i] = '\0';
				gUtility.MyStrCpy(str, 100, Convert_To_LowerCase(strExt));
				if(strcmp(strExt,"ts") == 0)
					strExt[i++] = '_';
			}
			break;
		}
	}
	strExt[i] =	'\0';

    gUtility.MyStrCpy(str, 100, Convert_To_LowerCase(strExt));
    if(gpConfig->gBC[nBoardNum].gnModulatorMode == CMMB)
	{
        if (strcmp(strExt,"mfs") == 0)
            return true;
        if (strcmp(strExt,"mmx") == 0)
            return true;
 	}
	else if (gpConfig->gBC[nBoardNum].gnModulatorMode == TDMB)
    {
        if (strcmp(strExt,"ni") == 0)
            return true;
        if (strcmp(strExt,"na") == 0)
            return true;
        if (strcmp(strExt,"eti") == 0)
            return true;
    }
	//kslee 2010/4/27
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == DVB_T2)
	{
        //2010/5/28
		//2010/12/23 DVB-T2 MULTI-PLP
		if (strcmp(strExt,"trp") == 0)
            return true;
        if (strcmp(strExt,"ts") == 0)
            return true;
        if (strcmp(strExt,"atsc") == 0)
            return true;
        if (strcmp(strExt,"dtv") == 0)
            return true;
        if (strcmp(strExt,"mpg") == 0)
            return true;
        if (strcmp(strExt,"mpeg") == 0)
            return true;

        if (strcmp(strExt,"t2") == 0)
            return true;
        if (strcmp(strExt,"t2mi") == 0)
            return true;
	}
	//2010/7/18
	else if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
	{
		if (strcmp(strExt,"iq") == 0)
            return true;
	}
	//----------------
	else
    {
        if (strcmp(strExt,"trp") == 0)
            return true;
        if (strcmp(strExt,"ts") == 0)
            return true;
        if (strcmp(strExt,"atsc") == 0)
            return true;
        if (strcmp(strExt,"dtv") == 0)
            return true;
        if (strcmp(strExt,"mpg") == 0)
            return true;
        if (strcmp(strExt,"mpeg") == 0)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// alloc gpConfig, reset gpConfig/gGeneral, GetCurrentDirectory, CheckMultiBoard, SNMP Init
bool CUTIL_IND::Initialize_Application()
{

    gpConfig = (APPLICATION_CONFIG *) new APPLICATION_CONFIG;
    memset(gpConfig, 0x00, sizeof(APPLICATION_CONFIG));          // about 65K
    memset(&gGeneral, 0x00, sizeof(TEL_GENERAL_VAR));
#ifdef WIN32
    GetCurrentDirectory(256, gGeneral.gnStrCurDir);
#else
	getcwd(gGeneral.gnStrCurDir, 256);
	DebugOutEx((char *) "Initialize_Application: curdir=%s", gGeneral.gnStrCurDir);
#endif
	gGeneral.gMenuIndex = -1;

#ifdef WIN32
	if (Check_Multi_Board() == false)
        return false;
#else
	gGeneral.gnMultiBoardUsed = true;
#endif

    return true;
}

//------------------------------------------------------------------------------
// ABC --> abc
// A=65, a=97
// A --> a ==> A + 32

char *CUTIL_IND::Convert_To_LowerCase(char *str)
{
	int	iLen = (int) (strlen(str));
	int	i;

	for (i = 0; i < iLen; i++)
	{
		if (str[i] >= 65 && str[i] <= 90)	// A~Z
			str[i] = str[i] + 32;
	}

	return str;
}

//****************************************************************************
bool	CUTIL_IND::IsNumericTel(char *szVal)
{
	int	iLen = (int) strlen(szVal);
	int	i;
	//2011/2/17
	int dotCnt = 0;
	int minusCnt = 0;

	if (iLen <= 0)
		return false;

	for (i = 0; i < iLen; i++)
	{
		if (szVal[i] < '0' || szVal[i] > '9')
			//kslee
			//return false;
		{
			if((szVal[i] == '.' || szVal[i] == ',') && i != 0 && dotCnt == 0)
			{
				dotCnt++;
				continue;
			}
			else if(szVal[i] == '-' && i == 0 && minusCnt == 0)
			{
				minusCnt++;
				continue;
			}
			else
				return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
// return send bytes
long CUTIL_IND::Get_nKeyVal(int nKey)
{
	long	nKeyVal = -1;

	nKeyVal = -1;
	if (nKey == TPG_KEY_NUM_0)
		nKeyVal = 0;
	else if (nKey == TPG_KEY_NUM_1)
		nKeyVal = 1;
	else if (nKey == TPG_KEY_NUM_2)
		nKeyVal = 2;
	else if (nKey == TPG_KEY_NUM_3)
		nKeyVal = 3;
	else if (nKey == TPG_KEY_NUM_4)
		nKeyVal = 4;
	else if (nKey == TPG_KEY_NUM_5)
		nKeyVal = 5;
	else if (nKey == TPG_KEY_NUM_6)
		nKeyVal = 6;
	else if (nKey == TPG_KEY_NUM_7)
		nKeyVal = 7;
	else if (nKey == TPG_KEY_NUM_8)
		nKeyVal = 8;
	else if (nKey == TPG_KEY_NUM_9)
		nKeyVal = 9;
  
	return nKeyVal;
}

/****************************************************************************
 * SNMP Processing
 ****************************************************************************/
//------------------------------------------------------------------------------
// - Initialize Winsock
// - Create In/Out Socket
// - Bind Socket
// Sending Socket : Remote Port = 8240
// Receiving Socket: Local Port = 8001
HANDLE	ghThread = NULL;


void CUTIL_IND::SNMP_Init()
{
	WORD	    wVersionRequested	= WINSOCK_VERSION;
	WSADATA	    wsaData;
	sockaddr_in sin;
    long        nBoardNum = gGeneral.gnActiveBoard;
    int         iRet;

    gGeneral.gnRecevingSocket = INVALID_SOCKET;
    gGeneral.gnSendingSocket = INVALID_SOCKET;

#ifdef WIN32
    //---------------------------------------
	//// Initialize WinSock.dll
    //---------------------------------------
	if( WSAStartup(wVersionRequested, &wsaData) )
	{
		gUtility.DisplayMessage("WSAStartup() fail");
		return;
	}

    //---------------------------------------
	//// Check WinSock Version
	if( wsaData.wVersion != wVersionRequested )
	{
		gUtility.DisplayMessage("WinSock version not supported");
		return;
	}

    //---------------------------------------
	// Create Sockets
    //---------------------------------------
    gGeneral.gnSendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGeneral.gnSendingSocket == INVALID_SOCKET)
    {
		gUtility.DisplayMessage("Fail to create new socket for sending.");
        return;
    }

    gGeneral.gnRecevingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGeneral.gnRecevingSocket == INVALID_SOCKET)
    {
		gUtility.DisplayMessage("Fail to create new socket for receiving.");
        return;
    }
	int option;
	option = 1;
	setsockopt(gGeneral.gnRecevingSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option)); 

    //---------------------------------------
	// Bind Sockets
    //---------------------------------------
	sin.sin_family              = AF_INET;
    sin.sin_addr.S_un.S_addr    = inet_addr(gpConfig->gBC[nBoardNum].gszSnmpRemoteHost);            // 127.0.0.1
	sin.sin_port                = htons(gpConfig->gBC[nBoardNum].gnSnmpLocalPort);                  // 8001: SNMP

    //-----------------------------------------
	// Binding
    iRet = bind(gGeneral.gnRecevingSocket, (sockaddr *)&sin, sizeof(sockaddr_in));
	if (iRet != 0)
	{
		gUtility.DisplayMessage("Fail to bind socket.");
    }

    //----------------------------------------
    // Create Receiving Thread
    DWORD   m_ThreadId;
	ghThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) SNMP_ThreadProc, (LPVOID)this, 0, &m_ThreadId);
#else
	MsgGet_AppToSnmpAgent();
	MsgGet_AppFromSnmpAgent(gUtility.SNMP_DataArrival);
#endif
}

//---------------------------------------------------------------------------
#ifdef WIN32
DWORD CUTIL_IND::SNMP_ThreadProc()
#else
void *CUTIL_IND::SNMP_ThreadProc(void *arg)
#endif
{
    int     iNumByte;
    char    strBuffer[256];

    OutputDebugString("**** SNMP_ThreadProc Started");
	for(int i=0;i < sizeof(strBuffer); i++)
	{
		strBuffer[i] = ' ';
	}
#ifdef WIN32 
    //---------------------------------------------
    // Infinit Loop
	while(gGeneral.gnRecevingSocket != INVALID_SOCKET)
	{
	    iNumByte = recv(gGeneral.gnRecevingSocket,strBuffer, 256, 0);
        if (iNumByte > 0)
        {
            strBuffer[iNumByte] = '\0';
            OutputDebugString(strBuffer);
            gUtility.SNMP_DataArrival(strBuffer);
			memset(strBuffer,0,sizeof(strBuffer));
        } else
        {
        }

        Sleep(1);
    }
#endif

    OutputDebugString("**** SNMP_ThreadProc Ended");
	return 0;
}

//------------------------------------------------------------------------------
void CUTIL_IND::SNMP_Term()
{
#ifdef WIN32
    if (gGeneral.gnRecevingSocket != INVALID_SOCKET)
        closesocket(gGeneral.gnRecevingSocket);
    gGeneral.gnRecevingSocket = INVALID_SOCKET;

    if (gGeneral.gnSendingSocket != INVALID_SOCKET)
        closesocket(gGeneral.gnSendingSocket);
    gGeneral.gnSendingSocket = INVALID_SOCKET;

    Sleep(10);
	WSACleanup();
#else
	Kill_AppMibTask();
#endif
}

//------------------------------------------------------------------------------
// return send bytes
long CUTIL_IND::SNMP_DataSend(char *szSnmpMsg)
{
    int         iLen = (int) (strlen(szSnmpMsg));
    long        lRet = 0;
    long        nBoardNum = gGeneral.gnActiveBoard;
	sockaddr_in sin;

#ifdef WIN32 
    if (iLen <= 0)
        return lRet;

    if (gGeneral.gnSendingSocket == INVALID_SOCKET)
        return lRet;

	sin.sin_family              = AF_INET;
    sin.sin_addr.S_un.S_addr    = inet_addr(gpConfig->gBC[nBoardNum].gszSnmpRemoteHost);
	sin.sin_port                = htons(gpConfig->gBC[nBoardNum].gnSnmpRemotePort);                  // 8240: SNMP
    lRet = sendto(gGeneral.gnSendingSocket, szSnmpMsg, iLen+1, 0,  (sockaddr *) &sin, sizeof(sockaddr_in));

#else
	lRet = SndMsg_AppToSnmpAgent();
#endif
    return lRet;
}

void CUTIL_IND::GetIpTable()
{
#ifdef WIN32
	// Before calling AddIPAddress we use GetIpAddrTable to get
	// an adapter to which we can add the IP.
	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;
	DWORD dwRetVal;
	DWORD IPCnt, j;
	struct in_addr IPAddr;
	char *strIPAddr;
	char strIp[64];
	pIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof( MIB_IPADDRTABLE) );

	// Make an initial call to GetIpAddrTable to get the
	// necessary size into the dwSize variable
	if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		free( pIPAddrTable);
		pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
	}

	// Make a second call to GetIpAddrTable to get the
	// actual data we want
	if ( (dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) == NO_ERROR ) { 
		IPCnt = pIPAddrTable->dwNumEntries;
		for(int i = 0 ; i <= MAX_BOARD_COUNT ; i++)
		{
			if(gpConfig->gBC[i].gnBoardStatus == 1)
			{
				if(IPCnt == 1)
				{
					IPAddr.S_un.S_addr = pIPAddrTable->table[IPCnt - 1].dwAddr;
					strIPAddr = inet_ntoa(IPAddr);
					gUtility.MyStrCpy(gpConfig->gBC[i].gszIP_RxLocalIP, 64, strIPAddr);
				}
				else
				{
					for(j = 0 ; j < IPCnt ; j++)
					{
						IPAddr.S_un.S_addr = pIPAddrTable->table[j].dwAddr;
						strIPAddr = inet_ntoa(IPAddr);
						gUtility.MyStrCpy(strIp, 64, strIPAddr);
						if(strcmp(strIp, "127.0.0.1") != 0)
						{
							gUtility.MyStrCpy(gpConfig->gBC[i].gszIP_RxLocalIP, 64, strIPAddr);
							break;
						}
					}
				}
			}
		}
	}
	free( pIPAddrTable);
#else
    struct ifreq *ifr;
    struct sockaddr_in *sin;
    struct sockaddr *sa;

	char *strIPAddr;
	char strIp[64];
	int i;

    struct ifconf ifcfg;
    int fd;
    int n;
    int numreqs = 30;
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ifcfg, 0, sizeof(ifcfg));
    ifcfg.ifc_buf = NULL;
    ifcfg.ifc_len = sizeof(struct ifreq) * numreqs;
    ifcfg.ifc_buf = (char *)malloc(ifcfg.ifc_len);

    for(;;)
    {
        ifcfg.ifc_len = sizeof(struct ifreq) * numreqs;
        ifcfg.ifc_buf = (char *)realloc(ifcfg.ifc_buf, ifcfg.ifc_len);
        if (ioctl(fd, SIOCGIFCONF, &ifcfg) < 0)
        {
            perror("SIOCGIFCONF ");
			return;
        }
        break;
    }


    ifr = ifcfg.ifc_req;
    for (n = 0; n < ifcfg.ifc_len; n+= sizeof(struct ifreq))
    {
        sin = (struct sockaddr_in *)&ifr->ifr_addr;
		strIPAddr = inet_ntoa(sin->sin_addr);
		gUtility.MyStrCpy(strIp, 64, strIPAddr);
		if(strcmp(strIp, "127.0.0.1") != 0)
		{
			break;
		}
		
        ifr++;
    }
	for(int i = 0 ; i <= MAX_BOARD_COUNT ; i++)
	{
		if(gpConfig->gBC[i].gnBoardStatus == 1)
		{
			gUtility.MyStrCpy(gpConfig->gBC[i].gszIP_RxLocalIP, 64, strIp);
		}
	}
	close(fd);
	free(ifcfg.ifc_buf);
#endif
}

long CUTIL_IND::CalcT2MI_Bitrate(long nBoardNum, long nCalcMaxBitrate, long PLP_MOD, long PLP_COD, long PLP_FEC_TYPE,
							double PLP_NUM_BLOCKS, long GUARD_INTERVAL, long FFT_SIZE, long S1, long NUM_DATA_SYMBOLS,
							long PILOT_PATTERN, long BW, long BWT_EXT, long L1_POST_SIZE, long HEM, long NPD)
{
	double N_mod, frame_closing_symbol, N_P2, C_P2;
	double L_data, C_data, C_FC, D_L1, C_tot;
	double GuardInterval, IFFT_SIZE, t, T_F, LDPC_code;
	double max_bitrate;
	double N_ldpc, K_ldpc, K_bch, N_FEC_FRAME, R_bits;

	double ref = 0;

	N_mod = 2.0; //QPSK, bits
	if(PLP_MOD == 0)
		N_mod = 2.0;
	else if(PLP_MOD == 1)
		N_mod = 4.0;
	else if(PLP_MOD == 2)
		N_mod = 6.0;
	else if(PLP_MOD == 3)
		N_mod = 8.0;

	frame_closing_symbol = 1;
	if(GUARD_INTERVAL == 4)
		frame_closing_symbol = 0.0;

	N_P2 = 16.0; //FFT 1k
	if(FFT_SIZE == 3)
		N_P2 = 16.0;
	else if(FFT_SIZE == 0)
		N_P2 = 8.0;
	else if(FFT_SIZE == 2)
		N_P2 = 4.0;
	else if(FFT_SIZE == 1 || FFT_SIZE == 6)
		N_P2 = 2.0;
	else if(FFT_SIZE == 4)
		N_P2 = 1.0;
	else if(FFT_SIZE == 5 || FFT_SIZE == 7)
		N_P2 = 1.0;

	C_P2 = 558.0; //FFT 1k, SISO
	if(S1 == 0)		//SISO
	{
		if(FFT_SIZE == 3)
			C_P2 = 558.0;
        else if(FFT_SIZE == 0)
            C_P2 = 1118.0;
        else if (FFT_SIZE == 2)
            C_P2 = 2236.0;
        else if (FFT_SIZE == 1 || FFT_SIZE == 6)
            C_P2 = 4472.0;
        else if (FFT_SIZE == 4)
            C_P2 = 8944.0;
        else if (FFT_SIZE == 5 || FFT_SIZE == 7)
            C_P2 = 22432.0;
	}
    else if (S1 == 1)   //==MISO
	{
		if (FFT_SIZE == 3)
            C_P2 = 546.0;
        else if (FFT_SIZE == 0)
            C_P2 = 1098.0;
        else if (FFT_SIZE == 2)
            C_P2 = 2198.0;
        else if (FFT_SIZE == 1 || FFT_SIZE == 6)
            C_P2 = 4398.0;
        else if (FFT_SIZE == 4)
            C_P2 = 8814.0;
        else if (FFT_SIZE == 5 || FFT_SIZE == 7)
            C_P2 = 17612.0;
	}
    L_data = NUM_DATA_SYMBOLS;

    C_data = 764.0; //FFT 1k, PP1
    if (FFT_SIZE == 3) 
	{
		if (PILOT_PATTERN == 0) 
			C_data = 764.0;
        else if (PILOT_PATTERN == 1) 
			C_data = 768.0;
        else if (PILOT_PATTERN == 2) 
	        C_data = 798.0;
        else if (PILOT_PATTERN == 3) 
		    C_data = 804.0;
        else if (PILOT_PATTERN == 4) 
			C_data = 818.0;
        else if (PILOT_PATTERN == 5) 
			C_data = 0.0;   //never used
        else if (PILOT_PATTERN == 6) 
			C_data = 0.0;
        else if (PILOT_PATTERN == 7) 
			C_data = 0.0;
	}     
    else if (FFT_SIZE == 0) 
	{
		if (PILOT_PATTERN == 0) 
			C_data = 1522.0;
        else if (PILOT_PATTERN == 1) 
			C_data = 1532.0;
        else if (PILOT_PATTERN == 2) 
			C_data = 1596.0;
        else if (PILOT_PATTERN == 3) 
			C_data = 1602.0;
        else if (PILOT_PATTERN == 4) 
			C_data = 1632.0;
        else if (PILOT_PATTERN == 5) 
			C_data = 0.0;
        else if (PILOT_PATTERN == 6) 
			C_data = 1646.0;
        else if (PILOT_PATTERN == 7) 
			C_data = 0.0;
	}        
    else if (FFT_SIZE == 2) 
	{
		if (PILOT_PATTERN == 0) 
	        C_data = 3084.0;
        else if (PILOT_PATTERN == 1) 
		    C_data = 3092.0;
        else if (PILOT_PATTERN == 2) 
			C_data = 3228.0;
        else if (PILOT_PATTERN == 3) 
			C_data = 3234.0;
        else if (PILOT_PATTERN == 4) 
			C_data = 3298.0;
        else if (PILOT_PATTERN == 5) 
			C_data = 0.0;
        else if (PILOT_PATTERN == 6) 
	        C_data = 3328.0;
        else if (PILOT_PATTERN == 7) 
		    C_data = 0.0;
	}
    else if (FFT_SIZE == 1 || FFT_SIZE == 6)    //8k
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 6208.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 6124.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 6494.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 6498.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 6634.0;
            else if (PILOT_PATTERN == 5) 
		        C_data = 0.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 6698.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 6698.0;
		}
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 6296.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 6298.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 6584.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 6588.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 6728.0;
	        else if (PILOT_PATTERN == 5) 
		        C_data = 0.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 6788.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 6788.0;
		}
	}
    else if (FFT_SIZE == 4) 
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 12418.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 12436.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 12988.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 13002.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 13272.0;
            else if (PILOT_PATTERN == 5) 
		        C_data = 13288.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 13416.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 13406.0;
		}
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 12678.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 12698.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 13262.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 13276.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 13552.0;
            else if (PILOT_PATTERN == 5) 
		        C_data = 13568.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 13698.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 13688.0;
		}
	}
    else if (FFT_SIZE == 5 || FFT_SIZE == 7) 
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 0.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 24886.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 0.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 26022.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 0.0;
            else if (PILOT_PATTERN == 5) 
		        C_data = 26592.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 26836.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 26812.0;
		} 
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_data = 0.0;
            else if (PILOT_PATTERN == 1) 
		        C_data = 25412.0;
            else if (PILOT_PATTERN == 2) 
			    C_data = 0.0;
            else if (PILOT_PATTERN == 3) 
				C_data = 26572.0;
            else if (PILOT_PATTERN == 4) 
	            C_data = 0.0;
            else if (PILOT_PATTERN == 5) 
		        C_data = 27152.0;
            else if (PILOT_PATTERN == 6) 
			    C_data = 27404.0;
            else if (PILOT_PATTERN == 7) 
				C_data = 27376.0;
		}
        
	}

    C_FC = 402.0; //FFT 1k, PP1
    if (FFT_SIZE == 3) 
	{
		if (PILOT_PATTERN == 0) 
	        C_FC = 402.0;
        else if (PILOT_PATTERN == 1) 
		    C_FC = 654.0;
        else if (PILOT_PATTERN == 2) 
			C_FC = 490.0;
        else if (PILOT_PATTERN == 3) 
			C_FC = 707.0;
        else if (PILOT_PATTERN == 4) 
	        C_FC = 544.0;
        else if (PILOT_PATTERN == 5) 
		    C_FC = 0.0;    //never used
        else if (PILOT_PATTERN == 6) 
			C_FC = 0.0;
        else if (PILOT_PATTERN == 7) 
			C_FC = 0.0;
	}
    else if (FFT_SIZE == 0) 
	{
		if (PILOT_PATTERN == 0) 
	        C_FC = 804.0;
        else if (PILOT_PATTERN == 1) 
		    C_FC = 1309.0;
        else if (PILOT_PATTERN == 2) 
			C_FC = 960.0;
        else if (PILOT_PATTERN == 3) 
	        C_FC = 1415.0;
        else if (PILOT_PATTERN == 4) 
		    C_FC = 1088.0;
        else if (PILOT_PATTERN == 5) 
			C_FC = 0.0;
        else if (PILOT_PATTERN == 6) 
	        C_FC = 1396.0;
        else if (PILOT_PATTERN == 7) 
		    C_FC = 0.0;
	}
    else if (FFT_SIZE == 2) 
	{
		if (PILOT_PATTERN == 0) 
	        C_FC = 1609.0;
        else if (PILOT_PATTERN == 1) 
		    C_FC = 2619.0;
        else if (PILOT_PATTERN == 2) 
			C_FC = 1961.0;
        else if (PILOT_PATTERN == 3) 
	        C_FC = 2381.0;
        else if (PILOT_PATTERN == 4) 
		    C_FC = 2177.0;
        else if (PILOT_PATTERN == 5) 
			C_FC = 0.0;
        else if (PILOT_PATTERN == 6) 
	        C_FC = 2792.0;
        else if (PILOT_PATTERN == 7) 
		    C_FC = 0.0;
	}
    else if (FFT_SIZE == 1 || FFT_SIZE == 6)     //8k
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 3218.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 5283.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 3922.0;
            else if (PILOT_PATTERN == 3) 
	            C_FC = 5662.0;
            else if (PILOT_PATTERN == 4) 
		        C_FC = 4353.0;
            else if (PILOT_PATTERN == 5) 
			    C_FC = 0.0;
            else if (PILOT_PATTERN == 6) 
				C_FC = 5585.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 3264.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 5312.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 3978.0;
            else if (PILOT_PATTERN == 3) 
				C_FC = 5742.0;
            else if (PILOT_PATTERN == 4) 
	            C_FC = 4416.0;
            else if (PILOT_PATTERN == 5) 
		        C_FC = 0.0;
            else if (PILOT_PATTERN == 6) 
			    C_FC = 5664.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
	}
    else if (FFT_SIZE == 4) 
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 6437.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 10476.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 7845.0;
            else if (PILOT_PATTERN == 3) 
				C_FC = 11324.0;
            else if (PILOT_PATTERN == 4) 
	            C_FC = 8709.0;
            else if (PILOT_PATTERN == 5) 
		        C_FC = 11801.0;
            else if (PILOT_PATTERN == 6) 
			    C_FC = 11170.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 6573.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 10697.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 8011.0;
            else if (PILOT_PATTERN == 3) 
				C_FC = 11563.0;
            else if (PILOT_PATTERN == 4) 
	            C_FC = 8893.0;
            else if (PILOT_PATTERN == 5) 
		        C_FC = 12051.0;
            else if (PILOT_PATTERN == 6) 
			    C_FC = 11406.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
	}
    else if (FFT_SIZE == 5 || FFT_SIZE == 7) 
	{
		if (BWT_EXT == 0) 
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 0.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 20952.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 0.0;
            else if (PILOT_PATTERN == 3) 
				C_FC = 22649.0;
            else if (PILOT_PATTERN == 4) 
	            C_FC = 0.0;
            else if (PILOT_PATTERN == 5) 
		        C_FC = 23603.0;
            else if (PILOT_PATTERN == 6) 
			    C_FC = 0.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
        else
		{
			if (PILOT_PATTERN == 0) 
	            C_FC = 0.0;
            else if (PILOT_PATTERN == 1) 
		        C_FC = 21395.0;
            else if (PILOT_PATTERN == 2) 
			    C_FC = 0.0;
            else if (PILOT_PATTERN == 3) 
				C_FC = 23127.0;
            else if (PILOT_PATTERN == 4) 
	            C_FC = 0.0;
            else if (PILOT_PATTERN == 5) 
		        C_FC = 24102.0;
            else if (PILOT_PATTERN == 6) 
			    C_FC = 0.0;
            else if (PILOT_PATTERN == 7) 
				C_FC = 0.0;
		}
	}
    D_L1 = 1840.0 + L1_POST_SIZE;

    if (frame_closing_symbol) 
        C_tot = N_P2 * C_P2 + (L_data - 1.0) * C_data + C_FC - D_L1;
    else
        C_tot = N_P2 * C_P2 + L_data * C_data - D_L1;
    

    if (PLP_FEC_TYPE == 1) 
        N_ldpc = 64800.0;
    else
        N_ldpc = 16200.0;
    

    K_ldpc = 32400.0;
    if (PLP_FEC_TYPE == 1) 
	{
		if (PLP_COD == 0) 
	        K_ldpc = 32400.0;
        else if (PLP_COD == 1) 
		    K_ldpc = 38880.0;
        else if (PLP_COD == 2) 
			K_ldpc = 43200.0;
        else if (PLP_COD == 3) 
	        K_ldpc = 48600.0;
        else if (PLP_COD == 4) 
		    K_ldpc = 51840.0;
        else if (PLP_COD == 5) 
			K_ldpc = 54000.0;
	}   
    else
	{
		if (PLP_COD == 0) 
	        K_ldpc = 7200.0;
        else if (PLP_COD == 1) 
		    K_ldpc = 9720.0;
        else if (PLP_COD == 2) 
			K_ldpc = 10800.0;
        else if (PLP_COD == 3) 
			K_ldpc = 11880.0;
        else if (PLP_COD == 4) 
			K_ldpc = 12600.0;
        else if (PLP_COD == 5) 
			K_ldpc = 13320.0;
	}        
    

    K_bch = 32208; //normal FECFRAME, 1/2
    if (PLP_FEC_TYPE == 1) 
	{
		if (PLP_COD == 0) 
	        K_bch = 32208.0;
        else if (PLP_COD == 1) 
		    K_bch = 38688.0;
        else if (PLP_COD == 2) 
			K_bch = 43040.0;
        else if (PLP_COD == 3) 
			K_bch = 48408.0;
        else if (PLP_COD == 4) 
	        K_bch = 51648.0;
        else if (PLP_COD == 5) 
		    K_bch = 53840.0;
	}
    else
	{
		if (PLP_COD == 0) 
	        K_bch = 7032.0;
        else if (PLP_COD == 1) 
		    K_bch = 9552.0;
        else if (PLP_COD == 2) 
			K_bch = 10632.0;
        else if (PLP_COD == 3) 
	        K_bch = 11712.0;
        else if (PLP_COD == 4) 
		    K_bch = 12432.0;
        else if (PLP_COD == 5) 
			K_bch = 13152.0;
	}
    
    N_FEC_FRAME = PLP_NUM_BLOCKS;  //???
    R_bits = (N_ldpc - K_bch + 80.0) * N_FEC_FRAME;

    GuardInterval = 1.0 / 32.0;
    if (GUARD_INTERVAL == 0) 
		GuardInterval = 1.0 / 32.0;
    else if (GUARD_INTERVAL == 1) 
		GuardInterval = 1.0 / 16.0;
    else if (GUARD_INTERVAL == 2) 
		GuardInterval = 1.0 / 8.0;
    else if (GUARD_INTERVAL == 3) 
		GuardInterval = 1.0 / 4.0;
    else if (GUARD_INTERVAL == 4) 
		GuardInterval = 1.0 / 128.0;
    else if (GUARD_INTERVAL == 5) 
		GuardInterval = 19.0 / 128.0;
    else if (GUARD_INTERVAL == 6) 
		GuardInterval = 19.0 / 256.0;
    

    IFFT_SIZE = 1.0; //1K ???
    if (FFT_SIZE == 3) 
		IFFT_SIZE = 1.0;
    else if (FFT_SIZE == 0) 
		IFFT_SIZE = 2.0;
    else if (FFT_SIZE == 2) 
		IFFT_SIZE = 4.0;
    else if (FFT_SIZE == 1 || FFT_SIZE == 6) 
		IFFT_SIZE = 8.0;
    else if (FFT_SIZE == 4)
		IFFT_SIZE = 16.0;
    else if (FFT_SIZE == 5 || FFT_SIZE == 7) 
		IFFT_SIZE = 32.0;
    
    IFFT_SIZE = IFFT_SIZE * 1024.0;

    t = 71.0 / 131.0;
    if (BW == 0) 
		t = 71.0 / 131.0;
    else if (BW == 1) 
		t = 7.0 / 40.0;
    else if (BW == 2) 
		t = 7.0 / 48.0;
    else if (BW == 3) 
		t = 1.0 / 8.0;
    else if (BW == 4) 
		t = 7.0 / 64.0;
    else if (BW == 5) 
		t = 7.0 / 80.0;
    

    t = t * (1.0 / 1000000.0); //sec.

    T_F = (2048.0 + (1.0 + GuardInterval) * IFFT_SIZE * (N_P2 + L_data)) * t;
    
    max_bitrate = (N_mod * C_tot - R_bits) / T_F;
	double min = -2147483647;
	double max = 2147483647;
    if (nCalcMaxBitrate == 1) 
	{
		if(max_bitrate <= min)
		{
			max_bitrate = min;
		}
		else if(max_bitrate > max)
		{
			max_bitrate = max;
		}
		return (long)(max_bitrate);
	}        
    
    
    if (PLP_FEC_TYPE == 1) 
	{
		if (PLP_COD == 0) 
			LDPC_code = 1.0 / 2.0;
        else if (PLP_COD == 1) 
			LDPC_code = 3.0 / 5.0;
        else if (PLP_COD == 2) 
			LDPC_code = 2.0 / 3.0;
        else if (PLP_COD == 3) 
			LDPC_code = 3.0 / 4.0;
        else if (PLP_COD == 4) 
			LDPC_code = 4.0 / 5.0;
        else if (PLP_COD == 5) 
			LDPC_code = 5.0 / 6.0;
	}
    else
	{
		if (PLP_COD == 0) 
	        LDPC_code = 4.0 / 9.0;
	    else if (PLP_COD == 1) 
		    LDPC_code = 3.0 / 5.0;
        else if (PLP_COD == 2) 
			LDPC_code = 2.0 / 3.0;
        else if (PLP_COD == 3) 
			LDPC_code = 11.0 / 15.0;
        else if (PLP_COD == 4) 
			LDPC_code = 7.0 / 9.0;
        else if (PLP_COD == 5) 
			LDPC_code = 37.0 / 45.0;
	}
    ref = (N_ldpc * N_FEC_FRAME * LDPC_code * (K_bch / K_ldpc) * ((K_bch - 80.0) / K_bch));
	ref = (ref / T_F);
	if (HEM == 1) 
		ref = (ref * (188.0 / 187.0));
   
	if (NPD == 1) 
		ref = (ref * (188.0 / 189.0));
	
	if(ref <= min)
	{
		ref = min;
	}
	else if(ref > max)
	{
		ref = max;
	}
   return (long)ref;
}

//-------------------------------------------------------------------------------------------------------------
long CUTIL_IND::Calulate_L1_Post_Size(long L1_MOD, long FFT_SIZE, long L1_POST_INFO_SIZE)
{
	double K_post_ex_pad, N_post_FEC_Block, K_L1_PADDING, K_post;
	double K_sig, N_L1_mult, N_punc_temp, N_bch_parity;
	double R_elf_16k_LDPC_1_2, N_post_temp, N_post, N_MOD_per_Block, N_MOD_Total;
	double N_ldpc, K_bch, N_mod, N_P2;

    N_ldpc = 16200.0;
    K_bch = 7032.0;
    N_mod = 1.0;
    N_P2 = 16.0;

	if(L1_MOD == 0)
		N_mod = 1.0;
	else if(L1_MOD == 1)
		N_mod = 2.0;
	else if(L1_MOD == 2)
		N_mod = 4.0;
	else if(L1_MOD == 3)
		N_mod = 6.0;

	if(FFT_SIZE == 3)
		N_P2 = 16.0;
	else if(FFT_SIZE == 0)
		N_P2 = 8.0;
	else if(FFT_SIZE == 2)
		N_P2 = 4.0;
	else if(FFT_SIZE == 1 || FFT_SIZE == 6)
		N_P2 = 2.0;
	else if(FFT_SIZE == 4)
		N_P2 = 1.0;
	else if(FFT_SIZE == 5 || FFT_SIZE == 7)
		N_P2 = 1.0;

	K_post_ex_pad = L1_POST_INFO_SIZE + 32.0;

#ifdef WIN32
	N_post_FEC_Block = Math::Round(K_post_ex_pad / K_bch + 0.5);
	K_L1_PADDING = Math::Round(K_post_ex_pad / N_post_FEC_Block + 0.5) * N_post_FEC_Block - K_post_ex_pad;
#else
	N_post_FEC_Block = round(K_post_ex_pad / K_bch + 0.5);
	K_L1_PADDING = round(K_post_ex_pad / N_post_FEC_Block + 0.5) * N_post_FEC_Block - K_post_ex_pad;
#endif
	K_post = (int)(K_post_ex_pad + K_L1_PADDING);
	K_sig = (int)(K_post / N_post_FEC_Block);
	
	if(N_P2 == 1)
		N_L1_mult = 2.0 * N_mod;
	else
		N_L1_mult = N_P2 * N_mod;

	N_punc_temp = (int)((int)(6.0 / 5.0) * (K_bch - K_sig));
	if(N_punc_temp < (N_L1_mult - 1.0))
		N_punc_temp = (N_L1_mult - 1.0);

	R_elf_16k_LDPC_1_2 = 4.0 / 9.0;

	N_bch_parity = 168.0;
	N_post_temp = (int)(K_sig + N_bch_parity + N_ldpc * (1.0 - R_elf_16k_LDPC_1_2) - N_punc_temp);
#ifdef WIN32
	if(N_P2 == 1)
		N_post = (int)(Math::Round(N_post_temp / (2.0 * N_mod) + 0.5) * (2.0 * N_mod));
	else
		N_post = (int)(Math::Round(N_post_temp / (N_mod * N_P2) + 0.5) * (N_mod * N_P2));
#else
	if(N_P2 == 1)
		N_post = (int)(round(N_post_temp / (2.0 * N_mod) + 0.5) * (2.0 * N_mod));

	else
		N_post = (int)(round(N_post_temp / (N_mod * N_P2) + 0.5) * (N_mod * N_P2));
#endif
	N_MOD_per_Block = (int)(N_post / N_mod);
	N_MOD_Total = (int)(N_MOD_per_Block * N_post_FEC_Block);

	return (long)N_MOD_Total;
}

//2011/5/31 DVB-C2 MULTI PLP
long CUTIL_IND::CalcC2MI_PLPBitrate(long nBandWidth, long nL1TIMode, long nGuardInterval, long nPlpCod, long nPlpFecType, long nPlpNumBlk, long nHem)
{
	long K_bch, LF;
	double T, TF;
	double PlpBitrate;
	long N_fec_frame, GI;
	double M;

	N_fec_frame = nPlpNumBlk;
	
	if(nHem == 0)
		M = 1.0;
	else
		M = 188.0 / 187.0;

	if(nGuardInterval == C2_GUARD_INDEX_0)
		GI = 32;
	else
		GI = 64;

	// T
	if(nBandWidth == C2_BANDWIDTH_6M)
		T = 7.0 / 1000000.0 / 48.0;
	else if(nBandWidth == C2_BANDWIDTH_7M)
		T = 7.0 / 1000000.0 / 56.0;
	else
		T = 7.0 / 1000000.0 / 64.0;

	//LF
	if(nL1TIMode == C2_L1TI_MODE_NONE || nL1TIMode == C2_L1TI_MODE_BEST_FIT)
		LF = 449;
	else if(nL1TIMode == C2_L1TI_MODE_4SYMBOLS)
		LF = 452;
	else
		LF = 456;

	//LDPC_code, K_bch, K_ldpc
	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
		    K_bch = 10632;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
		    K_bch = 11712;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			K_bch = 12432;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			K_bch = 13152;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
		{
			K_bch = 14232;
		}
		else
		{
		    K_bch = 0;
		}
	}
	else
	{
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
	        K_bch = 43040;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
			K_bch = 48408;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			K_bch = 51648;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			K_bch = 53840;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
		{
			K_bch = 58192;
		}
		else
		{
		    K_bch = 0;
		}
	}
	//TF
	TF = (double)LF * (double)(4096.0 + GI) * T;
	PlpBitrate = ((double)N_fec_frame * (double)(K_bch - 80) * M) / TF;
	return (long)PlpBitrate;
}


//2011/5/9 DVB-C2
long CUTIL_IND::CalcC2MI_Bitrate(long nBandWidth, long nL1TIMode, long nStartFreq, long nGuardInterval, long nDsliceType, long nFecHeaderType, long nPlpFecType, 
							long nPlpMod, long nPlpCod, long nHem, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth)
{
	long Dx, Dy;
	long K_N_min, K_N_max;
	long FECFrameHeader;
	long N_ldpc, Ncells, n_mod;
    long K_bch, K_ldpc, LF;
	double LDPC_code, T, TF;
	long i, j;
	long C2Frame[C2_ARRAY_COUNT_1][C2_ARRAY_COUNT_0];
	long Symbols[C2_ARRAY_COUNT_1]; 
	long Cell_tot;
	double N_FEC_FRAME;
	double PayloadBitTemp, MaxBitRateTemp;
	long MaxBitRate;
	


	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		Symbols[i] = 0;
		for(j = 0; j < C2_ARRAY_COUNT_0; j++)
		{
			C2Frame[i][j] = 0;
		}
	}

	long Locs_Of_Cont_Pilots[] = {96, 216, 306, 390, 450, 486, 780, 804, 924, 1026, 1224, 1422, 1554, 1620, 1680, 1902, 1956, 2016, 2142,
								  2220, 2310, 2424, 2466, 2736, 3048, 3126, 3156, 3228, 3294, 3366};

	long Locs_Of_Rsvd_Tones[] = {161, 243, 296, 405, 493, 584, 697, 741, 821, 934, 1021, 1160, 1215, 1312, 1417, 1462, 1591, 1693, 1729, 1845, 1910,
								 1982, 2127, 2170, 2339, 2365, 2499, 2529, 2639, 2745, 2864, 2950, 2992, 3119, 3235, 3255, 3559, 3620, 3754, 3835, 
								 3943, 3975, 4061, 4210, 4270, 4371, 4417, 4502, 4640, 4677, 4822, 4904, 5026, 5113, 5173, 5271, 5317, 5426, 5492, 
								 5583, 5740, 5757, 5839, 5935, 6033, 6146, 6212, 6369, 6454, 6557, 6597, 6711, 6983, 7047, 7173, 7202, 7310, 7421,
								 7451, 7579, 7666, 7785, 7831, 7981, 8060, 8128, 8251, 8326, 8369, 8445, 8569, 8638, 8761, 8873, 8923, 9017, 9104,
								 9239, 9283, 9368, 9500, 9586, 9683, 9782, 9794, 9908, 9989, 10123, 10327, 10442, 10535, 10658, 10739, 10803, 10925,
								 11006, 11060, 11198, 11225, 11326, 11474, 11554, 11663, 11723, 11810, 11902, 11987, 12027, 12117, 12261, 12320, 12419,
								 12532, 12646, 12676, 12808, 12915, 12941, 13067, 13113, 13246, 13360, 13426, 13520, 13811, 13862, 13936, 14073, 14102,
								 14206, 14305, 14408, 14527, 14555, 14650, 14755, 14816, 14951, 15031, 15107, 15226, 15326, 15392, 15484, 15553, 15623,
								 15734, 15872, 15943, 16043, 16087, 16201, 16299, 16355, 16444, 16514, 16635, 16723, 16802, 16912, 17150, 17285, 17387,
								 17488, 17533, 17603, 17708, 17793, 17932, 18026, 18081, 18159, 18285, 18356, 18395, 18532, 18644, 18697, 18761, 18874,
								 18937, 19107, 19119, 19251, 19379, 19414, 19522, 19619, 19691, 19748, 19875, 19935, 20065, 20109, 20261, 20315, 20559,
								 20703, 20737, 20876, 20950, 21069, 21106, 21231, 21323, 21379, 21494, 21611, 21680, 21796, 21805, 21958, 22027, 22091,
								 22167, 22324, 22347, 22459, 22551, 22691, 22761, 22822, 22951, 22981, 23089, 23216, 23290, 23402, 23453, 23529, 23668,
								 23743, 24019, 24057, 24214, 24249, 24335, 24445, 24554, 24619, 24704, 24761, 24847, 24947, 25089, 25205, 25274, 25352,
								 25474, 25537, 25612, 25711, 25748, 25874, 25984, 26078, 26155, 26237, 26324, 26378, 26545, 26623, 26720, 26774, 26855,
								 26953, 27021, 27123};
	if(nGuardInterval == C2_GUARD_INDEX_0) //1/128
	{
		Dx = 24;
		Dy = 4;
	}
	else if(nGuardInterval == C2_GUARD_INDEX_1) // 1/64
	{
		Dx = 12;
		Dy = 4;
	}
	else
	{
		Dx = 0;
		Dy = 0;
	}

 	K_N_min = nNotchStart * Dx + nStartFreq + 1;
	K_N_max = (nNotchStart + nNotchWidth) * Dx + nStartFreq - 1;

	//FEC Frame Header
	if(nFecHeaderType == C2_FEC_HEADER_TYPE_0)
		FECFrameHeader = 32;
	else
		FECFrameHeader = 16;

	//N_ldpc, Ncells, n_mod
	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		N_ldpc = 16200;
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 4050;
			n_mod = 4;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 2700;
			n_mod = 6;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 2025;
			n_mod = 8;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 1620;
			n_mod = 10;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 1350;
			n_mod = 12;
		}
		else
		{
			Ncells = 0;
			n_mod = 0;
		}
	}
	else //64K
	{
		N_ldpc = 64800;
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 16200;
			n_mod = 4;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 10800;
			n_mod = 6;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 8100;
			n_mod = 8;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 6480;
			n_mod = 10;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 5400;
			n_mod = 12;
		}
		else
		{
			Ncells = 0;
			n_mod = 0;
		}
	}

	//LDPC_code, K_bch, K_ldpc
	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
	        LDPC_code = 2.0 / 3.0;
		    K_bch = 10632;
			K_ldpc = 10800;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
			//2011/5/24 DVB-C2 MULTI PLP
	        //LDPC_code = 3.0 / 4.0;
			LDPC_code = 11.0 / 15.0;
		    K_bch = 11712;
			K_ldpc = 11880;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			//2011/5/24 DVB-C2 MULTI PLP
			//LDPC_code = 4.0 / 5.0;
			LDPC_code = 7.0 / 9.0;
			K_bch = 12432;
			K_ldpc = 12600;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			//2011/5/24 DVB-C2 MULTI PLP
			//LDPC_code = 5.0 / 6.0;
			LDPC_code = 37.0 / 45.0;
			K_bch = 13152;
			K_ldpc = 13320;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
		{
			LDPC_code = 8.0 / 9.0;
			K_bch = 14232;
			K_ldpc = 14400;
		}
		else
		{
	        LDPC_code = 0;
		    K_bch = 0;
			K_ldpc = 0;
		}
	}
	else
	{
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
	        LDPC_code = 2.0 / 3.0;
	        K_bch = 43040;
		    K_ldpc = 43200;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
	        LDPC_code = 3.0 / 4.0;
			K_bch = 48408;
			K_ldpc = 48600;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			LDPC_code = 4.0 / 5.0;
			K_bch = 51648;
			K_ldpc = 51840;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			LDPC_code = 5.0 / 6.0;
			K_bch = 53840;
			K_ldpc = 54000;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
		{
			LDPC_code = 9.0 / 10.0;
			K_bch = 58192;
			K_ldpc = 58320;
		}
		else
		{
	        LDPC_code = 0;
		    K_bch = 0;
			K_ldpc = 0;
		}
	}

	//LF
	if(nL1TIMode == C2_L1TI_MODE_NONE || nL1TIMode == C2_L1TI_MODE_BEST_FIT)
		LF = 449;
	else if(nL1TIMode == C2_L1TI_MODE_4SYMBOLS)
		LF = 452;
	else
		LF = 456;

	// T
	if(nBandWidth == C2_BANDWIDTH_6M)
		T = 7.0 / 1000000.0 / 48.0;
	else if(nBandWidth == C2_BANDWIDTH_7M)
		T = 7.0 / 1000000.0 / 56.0;
	else
		T = 7.0 / 1000000.0 / 64.0;

	
	//TF
	if(nGuardInterval == C2_GUARD_INDEX_0)
		TF = (double)LF * (4096.0 + 32.0) * T;
	else
		TF = (double)LF * (4096.0 + 64.0) * T;

	//Ctot
	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		for(j = nStartFreq; j < (nStartFreq + C2_ARRAY_COUNT_0) ; j++)
		{
			//Check Scatter Pilots
			if((j % (Dx * Dy)) == (Dx * (i % Dy)))
			{
				C2Frame[i][(j - nStartFreq)] = 1;
			}

			//Check Continual Pilots
			for(int aa = 0 ; aa < 30 ; aa++)
			{
				if((j % C2_ARRAY_COUNT_0) == Locs_Of_Cont_Pilots[aa])
				{
					C2Frame[i][(j - nStartFreq)] = 2;
				}
			}

			//Check Reserved Tones
			if(nReservedTones == 1)
			{
				for(int bb = 0; bb < 288; bb++)
				{
					if(((j % (8 * 3408)) - (Dx * (i % Dy))) == Locs_Of_Rsvd_Tones[bb])
					{
						C2Frame[i][(j - nStartFreq)] = 3;
					}
				}
			}

			//Check_Edge Pilots
			if(nNumNotch == 0)
			{
				if( j == nStartFreq )
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}
			else
			{
				if( j == nStartFreq || j == (K_N_min - 1) || j == (K_N_max + 1))
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}

			//Check Notches
			if(nNumNotch == 1 && j >= K_N_min && j <= K_N_max)
			{
				C2Frame[i][(j - nStartFreq)] = 4;
			}

			if(C2Frame[i][j - nStartFreq] == 0)
				Symbols[i]++;
		}
	}
	Cell_tot = (Symbols[0] + Symbols[1] + Symbols[2] + Symbols[3]) * 112;

	//N_FEC_FRAME
	if(nDsliceType == 0)
	{
		N_FEC_FRAME = (double)Cell_tot / (double)Ncells;
	}
	else
	{
		N_FEC_FRAME = (double)Cell_tot / (double)(Ncells + FECFrameHeader);
	}

	//PayloadBitTemp
	PayloadBitTemp = LDPC_code * (double)((double)K_bch / (double)K_ldpc) * (((double)K_bch -80.0) / (double)K_bch);

	//MaxBitRateTemp
	if(nDsliceType == 0)
	{
		if((Cell_tot % Ncells) == 0)
		{
			MaxBitRateTemp = (double)((double)N_ldpc * N_FEC_FRAME * PayloadBitTemp) / TF;
		}
		else
		{
			MaxBitRateTemp = (double)((double)((double)N_ldpc * N_FEC_FRAME * PayloadBitTemp) + (double)((double)((double)(double)Cell_tot - 
				                      (double)(N_FEC_FRAME * (double)Ncells)) * (double)n_mod * PayloadBitTemp)) / TF;
		}
	}
	else
	{
		if((Cell_tot % (Ncells + FECFrameHeader)) == 0)
		{
			MaxBitRateTemp = (double)((double)N_ldpc * N_FEC_FRAME * PayloadBitTemp) / TF;
		}
		else
		{
			MaxBitRateTemp = (double)((double)((double)N_ldpc * N_FEC_FRAME * PayloadBitTemp) + (double)((double)((double)(double)Cell_tot - (double)(N_FEC_FRAME * 
				                       (double)(Ncells + FECFrameHeader))) * (double)((double)Ncells / (double)(Ncells + FECFrameHeader)) * (double)n_mod * PayloadBitTemp)) / TF;
		}
	}
	
	//MaxBitRate
#ifdef WIN32
	if(nHem == 0)
		MaxBitRate = (long)Math::Round(MaxBitRateTemp);
	else
		MaxBitRate = (long)Math::Round(MaxBitRateTemp * (double)(188.0 / 187.0));
#else
	if(nHem == 0)
		MaxBitRate = (long)round(MaxBitRateTemp);
	else
		MaxBitRate = (long)round(MaxBitRateTemp * (double)(188.0 / 187.0));
#endif
	return MaxBitRate;
}
//2011/5/30 DAC FREQ RANGE
int CUTIL_IND::getFreqIndex()
{
	long nBoardNum = gGeneral.gnActiveBoard;
    long nFreq_MHz;
	nFreq_MHz = gpConfig->gBC[nBoardNum].gnRFOutFreq / 1000000;
	int index = 0;
	if(gpConfig->gBC[nBoardNum].gnBoardId == 12)
	{
		index = 0;
		while(1)
		{
			if(gDacValue_TVB599[index][0] == -1)
			{
				if(nFreq_MHz <= 0)
					return 0;
				else
					return 66;
			}

			if(gDacValue_TVB599[index][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB599[index][1])
			{
				return index;
			}
			index++;
		}
	}
	else if(gpConfig->gBC[nBoardNum].gnBoardId == 16)
	{
		index = 0;
		while(1)
		{
			if(gDacValue_TVB599[index][0] == -1)
			{
				if(nFreq_MHz <= 0)
					return 0;
				else
					return 66;
			}

			if(gDacValue_TVB599[index][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB599[index][1])
			{
				return index;
			}
			index++;
		}
	}
	else if((gpConfig->gBC[nBoardNum].gnBoardId == 0x16 && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xF && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6) || 
		(gpConfig->gBC[nBoardNum].gnBoardId == 0xB && gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6))
	{
		index = 0;
		while(1)
		{
			if(gDacValue_TVB591S_V_2_x[index][0] == -1)
			{
				if(nFreq_MHz <= 0)
					return 0;
				else
					return 48;
			}

			if(gDacValue_TVB591S_V_2_x[index][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB591S_V_2_x[index][1])
			{
				return index;
			}
			index++;
		}
	}
	else
	{
		if(gDacValue_TVB590S_V_3_x[0][0] <= nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[0][1])
			return 0;
		else if(gDacValue_TVB590S_V_3_x[1][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[1][1])
			return 1;
		else if(gDacValue_TVB590S_V_3_x[2][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[2][1])
			return 2;
		else if(gDacValue_TVB590S_V_3_x[3][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[3][1])
			return 3;
		else if(gDacValue_TVB590S_V_3_x[4][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[4][1])
			return 4;
		else if(gDacValue_TVB590S_V_3_x[5][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[5][1])
			return 5;
		else if(gDacValue_TVB590S_V_3_x[6][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[6][1])
			return 6;
		else if(gDacValue_TVB590S_V_3_x[7][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[7][1])
			return 7;
		else if(gDacValue_TVB590S_V_3_x[8][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[8][1])
			return 8;
		else if(gDacValue_TVB590S_V_3_x[9][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[9][1])
			return 9;
		else if(gDacValue_TVB590S_V_3_x[10][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[10][1])
			return 10;
		else if(gDacValue_TVB590S_V_3_x[11][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[11][1])
			return 11;
		else if(gDacValue_TVB590S_V_3_x[12][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[12][1])
			return 12;
		else if(gDacValue_TVB590S_V_3_x[13][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[13][1])
			return 13;
		else if(gDacValue_TVB590S_V_3_x[14][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[14][1])
			return 14;
		else if(gDacValue_TVB590S_V_3_x[15][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[15][1])
			return 15;
		else if(gDacValue_TVB590S_V_3_x[16][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[16][1])
			return 16;
		else if(gDacValue_TVB590S_V_3_x[17][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[17][1])
			return 17;
		else if(gDacValue_TVB590S_V_3_x[18][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[18][1])
			return 18;
		else if(gDacValue_TVB590S_V_3_x[19][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[19][1])
			return 19;
		else if(gDacValue_TVB590S_V_3_x[20][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[20][1])
			return 20;
		else if(gDacValue_TVB590S_V_3_x[21][0] < nFreq_MHz && nFreq_MHz <= gDacValue_TVB590S_V_3_x[21][1])
			return 21;
		else 
			return -1;
	}
}

//2011/6/21 DVB-C2
long CUTIL_IND::CalcC2MI_Ctot(long nStartFreq, long nGuardInterval, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth)
{
	long Dx, Dy;
	long K_N_min, K_N_max;
	long i, j;
	long C2Frame[C2_ARRAY_COUNT_1][C2_ARRAY_COUNT_0];
	long Symbols[C2_ARRAY_COUNT_1]; 
	long Cell_tot;
	


	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		Symbols[i] = 0;
		for(j = 0; j < C2_ARRAY_COUNT_0; j++)
		{
			C2Frame[i][j] = 0;
		}
	}

	long Locs_Of_Cont_Pilots[] = {96, 216, 306, 390, 450, 486, 780, 804, 924, 1026, 1224, 1422, 1554, 1620, 1680, 1902, 1956, 2016, 2142,
								  2220, 2310, 2424, 2466, 2736, 3048, 3126, 3156, 3228, 3294, 3366};

	long Locs_Of_Rsvd_Tones[] = {161, 243, 296, 405, 493, 584, 697, 741, 821, 934, 1021, 1160, 1215, 1312, 1417, 1462, 1591, 1693, 1729, 1845, 1910,
								 1982, 2127, 2170, 2339, 2365, 2499, 2529, 2639, 2745, 2864, 2950, 2992, 3119, 3235, 3255, 3559, 3620, 3754, 3835, 
								 3943, 3975, 4061, 4210, 4270, 4371, 4417, 4502, 4640, 4677, 4822, 4904, 5026, 5113, 5173, 5271, 5317, 5426, 5492, 
								 5583, 5740, 5757, 5839, 5935, 6033, 6146, 6212, 6369, 6454, 6557, 6597, 6711, 6983, 7047, 7173, 7202, 7310, 7421,
								 7451, 7579, 7666, 7785, 7831, 7981, 8060, 8128, 8251, 8326, 8369, 8445, 8569, 8638, 8761, 8873, 8923, 9017, 9104,
								 9239, 9283, 9368, 9500, 9586, 9683, 9782, 9794, 9908, 9989, 10123, 10327, 10442, 10535, 10658, 10739, 10803, 10925,
								 11006, 11060, 11198, 11225, 11326, 11474, 11554, 11663, 11723, 11810, 11902, 11987, 12027, 12117, 12261, 12320, 12419,
								 12532, 12646, 12676, 12808, 12915, 12941, 13067, 13113, 13246, 13360, 13426, 13520, 13811, 13862, 13936, 14073, 14102,
								 14206, 14305, 14408, 14527, 14555, 14650, 14755, 14816, 14951, 15031, 15107, 15226, 15326, 15392, 15484, 15553, 15623,
								 15734, 15872, 15943, 16043, 16087, 16201, 16299, 16355, 16444, 16514, 16635, 16723, 16802, 16912, 17150, 17285, 17387,
								 17488, 17533, 17603, 17708, 17793, 17932, 18026, 18081, 18159, 18285, 18356, 18395, 18532, 18644, 18697, 18761, 18874,
								 18937, 19107, 19119, 19251, 19379, 19414, 19522, 19619, 19691, 19748, 19875, 19935, 20065, 20109, 20261, 20315, 20559,
								 20703, 20737, 20876, 20950, 21069, 21106, 21231, 21323, 21379, 21494, 21611, 21680, 21796, 21805, 21958, 22027, 22091,
								 22167, 22324, 22347, 22459, 22551, 22691, 22761, 22822, 22951, 22981, 23089, 23216, 23290, 23402, 23453, 23529, 23668,
								 23743, 24019, 24057, 24214, 24249, 24335, 24445, 24554, 24619, 24704, 24761, 24847, 24947, 25089, 25205, 25274, 25352,
								 25474, 25537, 25612, 25711, 25748, 25874, 25984, 26078, 26155, 26237, 26324, 26378, 26545, 26623, 26720, 26774, 26855,
								 26953, 27021, 27123};
	if(nGuardInterval == C2_GUARD_INDEX_0) //1/128
	{
		Dx = 24;
		Dy = 4;
	}
	else if(nGuardInterval == C2_GUARD_INDEX_1) // 1/64
	{
		Dx = 12;
		Dy = 4;
	}
	else
	{
		Dx = 0;
		Dy = 0;
	}

 	K_N_min = nNotchStart * Dx + nStartFreq + 1;
	K_N_max = (nNotchStart + nNotchWidth) * Dx + nStartFreq - 1;

	//Ctot
	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		for(j = nStartFreq; j < (nStartFreq + C2_ARRAY_COUNT_0) ; j++)
		{
			//Check Scatter Pilots
			if((j % (Dx * Dy)) == (Dx * (i % Dy)))
			{
				C2Frame[i][(j - nStartFreq)] = 1;
			}

			//Check Continual Pilots
			for(int aa = 0 ; aa < 30 ; aa++)
			{
				if((j % C2_ARRAY_COUNT_0) == Locs_Of_Cont_Pilots[aa])
				{
					C2Frame[i][(j - nStartFreq)] = 2;
				}
			}

			//Check Reserved Tones
			if(nReservedTones == 1)
			{
				for(int bb = 0; bb < 288; bb++)
				{
					if(((j % (8 * 3408)) - (Dx * (i % Dy))) == Locs_Of_Rsvd_Tones[bb])
					{
						C2Frame[i][(j - nStartFreq)] = 3;
					}
				}
			}

			//Check_Edge Pilots
			if(nNumNotch == 0)
			{
				if( j == nStartFreq )
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}
			else
			{
				if( j == nStartFreq || j == (K_N_min - 1) || j == (K_N_max + 1))
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}

			//Check Notches
			if(nNumNotch == 1 && j >= K_N_min && j <= K_N_max)
			{
				C2Frame[i][(j - nStartFreq)] = 4;
			}

			if(C2Frame[i][j - nStartFreq] == 0)
				Symbols[i]++;
		}
	}
	Cell_tot = (Symbols[0] + Symbols[1] + Symbols[2] + Symbols[3]) * 112;

	return Cell_tot;
}

//2011/6/21 DVB-C2
long CUTIL_IND::CalcC2MI_plpCells(long nPlpFecType, long nPlpMod, long nFecHeaderType, long nPlpNumBlk)
{
	long PlpCells;
	long Ncells;
	long FECFrameHeader;

	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 4050;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 2700;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 2025;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 1620;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 1350;
		}
		else
		{
			Ncells = 0;
		}
	}
	else //64K
	{
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 16200;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 10800;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 8100;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 6480;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 5400;
		}
		else
		{
			Ncells = 0;
		}
	}

	if(nFecHeaderType == C2_FEC_HEADER_TYPE_0)
		FECFrameHeader = 32;
	else
		FECFrameHeader = 16;


	PlpCells = (Ncells + FECFrameHeader) * nPlpNumBlk;
	return PlpCells;
}

//2012/5/9
int	CUTIL_IND::ModulatorTypeValidity(int board_id, int board_rev, int modulator)
{
	switch(board_id)
	{
	case 43:	//TVB370
	case 46:
	case 80:
		return VSB_8;
	case 38:
	case 39:
	case 40:
		if(modulator > QPSK)
		{
			return VSB_8;
		}
		break;
	case 41:
	case 42:
		if(modulator > TDMB)
		{
			return VSB_8;
		}
		break;
	case 44:
		if(modulator > DVB_S2)
		{
			return VSB_8;
		}
		break;
	case 47:
		if(modulator > ISDB_T_13)
		{
			return VSB_8;
		}
		break;
	case 45:
	case 59:
	case 60:
	case 61:
		if(modulator > DTMB)
		{
			return VSB_8;
		}
		break;
	case 48:
		if(modulator == CMMB || modulator == DVB_T2 || modulator == ATSC_MH || modulator == ISDB_S || 
			modulator == DVB_C2 || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
		{
			return VSB_8;
		}
		break;
	case 20:
		if(modulator == ISDB_T || modulator == ISDB_T_13 || modulator == DTMB || modulator == CMMB || modulator == DVB_T2 || 
			modulator == ATSC_MH || modulator == DVB_C2 || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
		{
			return VSB_8;
		}
		break;
	case 10:
		if(modulator == IQ_PLAY || modulator == ISDB_S || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
		{
			return VSB_8;
		}
		break;
	case 11:
	case 12:
		if(modulator == IQ_PLAY || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
		{
			return ASI_OUTPUT_MODE;
		}
		break;
	case 16:
		if(modulator == IQ_PLAY)
		{
			return ASI_OUTPUT_MODE;
		}
		break;
	case 21:
		if(board_rev < 3)
		{
			if(modulator == QPSK || modulator == DVB_S2 || modulator == ISDB_T || modulator == ISDB_T_13 || modulator == DTMB || modulator == CMMB || modulator == DVB_T2 || 
				modulator == ATSC_MH || modulator == ISDB_S || modulator == DVB_C2 || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
			{
				return VSB_8;
			}
		}
		else
		{
			if(modulator == QPSK || modulator == DVB_S2 || modulator == CMMB || modulator == DVB_T2 || 
				modulator == ISDB_S || modulator == DVB_C2 || modulator == MULTIPLE_VSB || modulator == MULTIPLE_QAMB || modulator == MULTIPLE_DVBT)
			{
				return VSB_8;
			}
		}
		break;
	case 22:
		if(modulator == CMMB || modulator == DVB_T2 || modulator == DVB_C2 || modulator == MULTIPLE_DVBT)
		{
			return VSB_8;
		}
		break;
	}
	return modulator;
}
//2012/9/6 pcr restamp
int	CUTIL_IND::IsSupported_Mod_PcrRestamp_By_HW(int nMod)
{
	switch(nMod)
	{
	case DVB_T:
	case VSB_8:
	case QAM_A:
	case QAM_B:
	case QPSK:
	case DVB_H:
	case DVB_S2:
	case DTMB:
#ifdef WIN32
		if(gWrapDll.TSPL_GET_FPGA_INFO_EX(gGeneral.gnActiveBoard, 7) == 1)
			return 1;
#else
        if(TSPL_GET_FPGA_INFO_EX(gGeneral.gnActiveBoard, 7) == 1)
            return 1;
#endif
		break;
	}
	return 0;
}

int CUTIL_IND::IsAttachedBdTyp_NewRFLevel_Cntl(int nBoardNum)
{
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		return 0;
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
		if(gpConfig->gBC[nBoardNum].gnBoardRev >= 3)
			return 1;
		break;
	case 11:
	case 15:
	case 21:
	case 22:
	case 27:
	case 12:		//2013/5/27 TVB599 0xC
	case 16:		//2013/9/5 TVB598
		return 1;
	}
	return 0;
}

int CUTIL_IND::IsEnabled_DacOffset(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
	case 11:
	case 15:
//	case 21:
	case 22:
	case 27:
	case 12:		//2013/5/27 TVB599 0xC
	case 16:		//TVB598
		return 1;
		break;
	}
	return 0;
}

//2012/9/5 added ToolStripMenu
int	CUTIL_IND::IsEnabled_CN(int nBoardNum)
{
	if(gpConfig->gBC[nBoardNum].gnModulatorMode == IQ_PLAY)
		return 0;

	if(gpConfig->gBC[nBoardNum].gnBoardId == _TVB594_BD_ID_)
		return 0;

    if (gpConfig->gBC[nBoardNum].gnBoardId >= 44 || gpConfig->gBC[nBoardNum].gnBoardId == 10 || gpConfig->gBC[nBoardNum].gnBoardId == 20 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0xF || gpConfig->gBC[nBoardNum].gnBoardId == 0x15 || gpConfig->gBC[nBoardNum].gnBoardId == 11 ||
		gpConfig->gBC[nBoardNum].gnBoardId == 0x16 || gpConfig->gBC[nBoardNum].gnBoardId == 27 || gpConfig->gBC[nBoardNum].gnBoardId == 12 || gpConfig->gBC[nBoardNum].gnBoardId == 16)	//2013/5/27 TVB599 0xC
	{
		return 1;
	}

	return 0;

}
int	CUTIL_IND::IsVisible_IF_(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
	case 11:
	case 15:
	case 21:
	case 22:
	case 27:
	case 43:
	case 12:		//2013/5/27 TVB599 0xC
	case 16:
		return 0;
	case _TVB594_BD_ID_:
		if(gpConfig->gBC[nBoardNum].gn_IsVirtualSlot == 1)
			return 0;
	}
	return 1;
}

int CUTIL_IND::IsSupportDvbT_Cell_ID(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 48:
	case 10:
	case 20:
	case 21:
	case 22:
	case 15:
	case 11:
	case 12:		//2013/5/27 TVB599 0xC
	case 16:
		return 1;
	}
	return 0;
}
int CUTIL_IND::IsSupportSpectrumInversion(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
		return 0;
	}
	return 1;
}
int CUTIL_IND::IsVisible_RRC_Filter(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
	case 21:
	case 22:
	case 15:
	case 11:
	case 27:
	case 12:		//2013/5/27 TVB599 0xC
	case 16:
		return 0;
	}
	return 1;
}
int CUTIL_IND::IsSupportDvbS2_RollOff_None(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 20:
	case 22:
	case 27:
		return 0;
	}
	return 1;
}

int CUTIL_IND::IsSupportDvbT2_TimeIL_Length(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 27:				//499
	case 12:
	case 16:
		return 1;
	}
	return 0;
}
int CUTIL_IND::IsSupportHMC833(int nBoardNum)
{
	switch(gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case 12:	
	case 16:	
		return 1;
		break;
	case 11:
	case 15:
		if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x6)
			return 1;
		break;
	case 22:
		if(gpConfig->gBC[nBoardNum].gnBoardRev >= 0x2)
			return 1;
		break;
	}
	return 0;
}
int CUTIL_IND::IsAttachedBdTyp_SupportEepromRW(int nBoardNum)
{
	switch (gpConfig->gBC[nBoardNum].gnBoardId)
	{
	case	12:
	case	16:
	case	11:
	case	22:
	case	15:
		return	1;
	}
	return	0;
}
void CUTIL_IND::Write_LN_Eeprom(int nBoardNum, char *szLN)
{
	unsigned long address;
	unsigned long data;
	int checkSum = 0;
	char ch;
    int _val;
	address = 0x400;
	data = 0xA720;
#ifdef WIN32
	gWrapDll.TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#else
	TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#endif
	checkSum = data;
	Sleep(100);
	data = 0;
	for(int i = 0; i < 32; i++)
	{
		ch = toupper(szLN[i]);
		if(ch >= '0' && ch <= '9')
			_val = ch - '0';
		else if(ch >= 'A' && ch <= 'F')
			_val = ch - 'A' + 10;

		if( ((i + 1) % 4)  == 0)
		{
			data = ((data & 0xFFFF) << 4) + (_val & 0xF);
			address = 0x400 + (2 * ((i + 1) / 4));
			checkSum = checkSum + (data & 0xFFFF);
#ifdef WIN32
			gWrapDll.TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#else
			TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#endif
			Sleep(100);
			data = 0;
		}
		else
		{
			data = ((data & 0xFFFF) << 4) + (_val & 0xF);
		}
	}
	address = 0x412;
	data = (unsigned long)((checkSum >> 16) & 0xFFFF);
#ifdef WIN32
	gWrapDll.TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#else
	TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#endif
	Sleep(100);
	address = 0x414;
	data = (unsigned long)(checkSum & 0xFFFF);
#ifdef WIN32
	gWrapDll.TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#else
	TSPL_WRITE_CONTROL_REG_EX(nBoardNum, 3, address, data);
#endif
}