//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_SIO__DRV_DEF_
#define	_TLV_SIO__DRV_DEF_

#ifdef WIN32
#else
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/ioctl.h>

#include "../libusb-0.1.12/usb.h"
#include "../libusb-0.1.12/usbi.h"
#endif

#include 	"wdm_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CSioDrv	:	public	CWdmDrv
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:

public:
	CSioDrv(void);
	~CSioDrv();

	int _stdcall TSPL_PUT_CHAR(long dwCommand);
	DWORD _stdcall TSPL_GET_CHAR(void);


};


#ifdef __cplusplus
}
#endif

#endif

