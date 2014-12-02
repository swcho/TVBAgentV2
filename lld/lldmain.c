//=================================================================	
//	LldMain.c : Low level device manager for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================

//=================================================================
#ifdef WIN32
#include	<windows.h>
#include	<process.h>
#include	<stdio.h>

//#include	"wdm_drv.h"
//#include	"logfile.h"
//#include	"dma_drv.h"
//#include	"dll_error.h"
//#include	"mainmode.h"
//
//#include	"Reg590S.h"
//
//#include	"wdm_drv_wrapper.h"

//=================================================================
// Linux
#else
#define _FILE_OFFSET_BITS 64
#include 	<stdio.h>
#include 	<stdlib.h>
#include	<stdarg.h>
//#include 	"../tsp100.h"
//#include 	"../include/logfile.h"

//#include 	"mainmode.h"
//#include	"Reg590S.h"

//#include	"wdm_drv_wrapper.h"

#endif

extern	void	Close_System(void);
#ifdef WIN32
/*^^***************************************************************************
 * Description : DLL entry point
 *				
 * Entry :
 *
 * Return:
 *
 * Notes :  
 *
 **************************************************************************^^*/
_declspec(dllexport) int _stdcall DllMain(HANDLE hinstDll, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason) 
	{
	case DLL_PROCESS_ATTACH:
//		atexit(Close_System);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	default:
		break;
	}
	return(1);
}
#endif

