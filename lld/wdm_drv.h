//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_WDM_DRV_DEF_
#define	_TLV_WDM_DRV_DEF_

#ifdef WIN32
#else
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/ioctl.h>
#include	"../libusb-0.1.12/usb.h"
#include	"../libusb-0.1.12/usbi.h"
#endif
#include	"lld_regs.h"
#include	"lld_hwif.h"

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================	
//LOW GAIN, AD9852ASQ/AD9852AST, UMC CONFIG
#define ROWSWIN_REG_VAL0		0x34		//0x34		//0x410
#define ROWSWIN_REG_VAL1_36M	0xE41		//0x100E41	//0x111D01
#define ROWSWIN_REG_VAL1_44M	0xE21		//0x100E21	//0x111A81
#define ROWSWIN_REG_VAL2		0x92		//0x92		//0x92
//sskim20081126 - fixed
#define ROWSWIN_REG_VAL0_DTMB		0x208
#define ROWSWIN_REG_VAL1_36M_DTMB	0x8E81
#define ROWSWIN_REG_VAL1_44M_DTMB	0x8D41
#define ROWSWIN_REG_VAL2_DTMB		0x92

#define ROWSWIN_FREQ_36M		912000000	//Hz
#define ROWSWIN_FREQ_44M		904000000	//Hz

#define	UMC_START_FREQ			1000		//MHz
#define	UMC_STOP_FREQ			2000		//MHz
#define	UMC_STEP_FREQ			100			//KHz
#define	UMC_REF_FREQ			10			//MHz

//950~2150MHz support
#define	IF_OUT_36MHZ			36000000	//HZ
#define	IF_OUT_44MHZ			44000000	//HZ
//FIXED - ROWSWIN offset, 52M->48M
#define	RF_MIN					48000000	//HZ
//FIXED - ROWSWIN offset, 52M->48M
#define	RF_MIN_DVB_S			48000000//950000000	//HZ
#define	RF_MAX					2150000000UL//HZ
#define	RF_1GHZ					1000000000UL//HZ
#define	RF_2GHZ					2000000000UL//HZ
#define	RF_948MHZ				948000000	//HZ

//ISDB-T 143KHz SHIFT
#define RF_143KHZ				143000		//HZ

#ifdef WIN32
//	#define	_MAX_PATH	1024
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CWdmDrv	:	public	CLldHwIf
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:
#ifdef WIN32
	TCHAR	szCurDir[_MAX_PATH];
#endif

public:
	CWdmDrv(void);
	~CWdmDrv();

	int IsModulatorTSLock(void);
	int WDM_Check_Board_Status(int nCount, int simple_test);
	int WDM_Check_PCI_Status(int nCount, int simple_test);
	int 	_stdcall TSPL_RESET_SDCON(void);
	void 	TSPL_RESET_SDCON_backwardcompatible(int comp_mod);
	int TVB594_RESET(int change_mod_param);
	int Cpld_RW_Test();


};


#ifdef __cplusplus
}
#endif

#endif

