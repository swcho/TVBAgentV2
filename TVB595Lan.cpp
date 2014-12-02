/******************************************************************************
******************************************************************************/
#include 	<unistd.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>


#if defined(WIN32)
#include	<windows.h>
#include	<winbase.h>
#include	<conio.h>
#else
#include 	<termios.h>
#include 	<unistd.h>
#include 	<dirent.h>			// for directory file
#include 	<linux/fs.h> 		// BLKGETSIZE, BLKSSZGET
#include 	<sys/ioctl.h> 
#include    <sys/statvfs.h>
#include	"variable.h"

#include "reg_var.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include "mainutil.h"
// huy: comment
#include "wrapdll.h"
#include "hld_api.h"

extern struct termios old_t, new_t;
extern int _kbhit();
extern APPLICATION_CONFIG      *gpConfig;
extern CWRAP_DLL                gWrapDll;
extern TEL_GENERAL_VAR         gGeneral;
extern CREG_VAR					gRegVar;

#define Sleep(n)	do { usleep(n * 1000); } while (0)
#endif

PlayForm		gGui;
  
void DebugOutEx(char *szFormat, ...)
{
	/* Not supported
	  char sz[2048];
	  char szlast[2048];
	  strcpy(szlast,"[590LINUX]");
	  vsprintf(sz, szFormat, (char *)(&szFormat+1));
	  strcat(sz,"\r\n");
	  strcat(szlast, sz);
	  printf(szlast);
	  */
}


//===========================================================================
// tvb595lan main function
//===========================================================================
int	main(int argc, char *argv[])
{

	char	ic;
	int		nBoardId;
	char	strMsg[256], strSN[64];
	int		iType = 0;
	
	printf("===========================================\n");
	printf("argc = %d\n", argc);
	printf("PRESS 'e' key to end the program\n");

	//=========================================
	// test for diskspace

	//=========================================
	// - Initialize
	gGui.Initialize();
	
	// - thread create (main timer)
	// - 

	nBoardId = gGeneral.gnActiveBoard;
	if (nBoardId < 0)
		nBoardId = 0;
	printf("  gnActiveBoard = %d\n", (int)gGeneral.gnActiveBoard);
	//2014/10/29 All board auto start
	for(int nBoardNum = 0; nBoardNum < gGeneral.gnRealBd_Cnt; nBoardNum++)
	{
		if(nBoardNum == nBoardId)
		{
			gGui.Set_Auto_Play();
		}
		else
		{
			gRegVar.RestoreVariables(nBoardNum);
			if(gpConfig->gBC[nBoardNum].gnAuto_Play == 1)
			{
				gpConfig->gnCurSlotIndex = nBoardNum;
				gGui.OnCbnSelchangeAdaptor(nBoardNum);
				gGui.SNMP_Send_Status(TVB390_RUN_TIME); 
				gGui.Set_Auto_Play();
			}
		}
	}
	if(nBoardId != gGeneral.gnActiveBoard)
	{
		gpConfig->gnCurSlotIndex = nBoardId;
		gGui.OnCbnSelchangeAdaptor(nBoardId);
	}

#ifdef STANDALONE	
	//  this bock of source code is temporary solution for making unique, nochange MAC address only 
	//  ------------------------------------------------------------------------------------------
//	char strSN[10];
	/// 
        TSPL_GET_ENCRYPTED_SN_EX(gGeneral.gnActiveBoard, 0, &strSN[0]);
	printf("[TVBAgent] serie number %s \n", &strSN[0]);

	//	the value of strSN is the unique board-ID. do you make eth-addr file here.
	// open file	
	char *eth0_file_path = "/root/network/eth0addr";
	struct stat st; 
	if(stat(eth0_file_path, &st) == 0)
		printf("[TVBAgent] eth0addr exists \n");
	else{	
		FILE *eth0_fh = fopen(eth0_file_path, "w+");
		if (eth0_fh == NULL)
		{
			printf("[TVBAgeng] can not create eth0addr file \n");
		}else
		{
			char eth0_mac_string[50];
			sprintf(&eth0_mac_string[0], "%.2X:%.2X:%C%C:%C%C:%C%C:%C%C", 0, 0, strSN[4], strSN[5],strSN[6], strSN[7], strSN[8], strSN[9], strSN[10], strSN[11]);
			eth0_mac_string[18] = NULL;

			printf("[TVBAgent] Debug: MAC address in string:\n");
			for (int i = 0; i < 17; i++){
				printf("%c", eth0_mac_string[i]);
			}
			int rt_size = fwrite(&eth0_mac_string[0], 17, 1, eth0_fh);
			if (rt_size == 0 )
			{
				printf("[TVBAgeng] can not write data to file  \n");
				fclose(eth0_fh);
				remove(eth0_file_path);
			} else
				fclose(eth0_fh);
		}


	}
	//  ------------------------------------------------------------------------------------------
	//
#endif
	ic = getchar();
	
	while(1)
	{
		if (ic == 'e')
			break;
#ifndef STANDALONE
		printf("=== INPUT = %c\n", ic);

		//====================================================
		//=== for Test Code
		if (ic == 'p')		// Play
		{
			gpConfig->gBC[nBoardId].gdwPlayRate = 19392658;
			gWrapDll.Start_Playing(nBoardId, 0, (char *) "/ts/KBS2.trp");		
		} else if (ic == 's')
		{
			gWrapDll.Stop_Playing(nBoardId);
		} else if (ic == 'f')
		{
			sprintf(strMsg, "%d 490000000", TVB390_RF);
			gGui.SNMP_DataArrival(strMsg);
		} else if (ic == 'g')
		{
			sprintf(strMsg, "%d 480000000", TVB390_RF);
			gGui.SNMP_DataArrival(strMsg);
		} else if (ic == 'h')		// firmware change
		{
				sprintf(strMsg, "%d %d", TVB390_MODULATOR_TYPE, (iType++ % 12));
				gGui.SNMP_DataArrival(strMsg);
		}
		//====================================================
		ic = getchar();
#else
		Sleep(1000000000);
#endif
	}

	
	gGui.Terminate();
	printf("End of Main\n");
	return 0;
}
//------------- ends here -------------------


