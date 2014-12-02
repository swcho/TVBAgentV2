//=================================================================	
//	WDM_Drv.c / Device Open/Close for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

//=================================================================	
#ifdef WIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<windows.h>
#include	<winioctl.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<commctrl.h>
#include	<time.h>
#include	<setupapi.h>
#include	<memory.h>
// TVB595V1
#include	<math.h>
	
#include	"initguid.h"
#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"

//=================================================================	
// Linux
#else
#define _FILE_OFFSET_BITS 64
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include 	<memory.h>
#include	<math.h>
#include	<string.h>
#include	<stdarg.h>
	
#include 	"../tsp100.h"
#include	"dll_error.h"
#include 	"../include/logfile.h"
#include 	"wdm_drv.h"
#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif

//TVB590S
#include	"Reg590S.h"

//TVB593
#include	"Reg593.h"

//TVB497
#include	"Reg497.h"

#include	"wdm_drv_wrapper.h"
#include	"../include/system_const.h"

//=================================================================	
// 
#define CHECK_CONFIG_STATUS_COUNT	1000

/////////////////////////////////////////////////////////////////
CWdmDrv::CWdmDrv(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 1;

}
CWdmDrv::~CWdmDrv()
{
}
int CWdmDrv::IsModulatorTSLock(void)
{
	unsigned long	_ret;

	if ( IsAttachedBdTyp_WhyThisGrpNeed_15() )
	{
		_ret = WDM_Read_TSP(TSP_MEM_ADDR_READ_PARAMS);
		if (((_ret >> 3) & 0x1) || ((_ret >> 4) & 0x1))
		{
			return	1;
		}
		return	0;
	}
	return	1;
}

int CWdmDrv::Cpld_RW_Test()
{
	unsigned int CPLD_W_TEST_ADDR = 0x40a00;
	unsigned int CPLD_R_TEST_ADDR = 0x7a000;
	unsigned int ret;
	int i;
for(int j = 0; j < 100; j++)
{
	WDM_WR2SDRAM_ADDR(CPLD_W_TEST_ADDR, 0xFFFFFFFF);
	//Sleep(10);
	ret = WDM_Read_TSP(CPLD_R_TEST_ADDR);
	if ( ret != 0xFFFFFFFF )
	{
			LldPrint_Error("CPLD read/write test error", 0xFFFFFFFF, ret);
			//OutputDebugString("CPLD ERROR\n");
	}
	WDM_WR2SDRAM_ADDR(CPLD_W_TEST_ADDR, 0x0);
	//Sleep(10);
	ret = WDM_Read_TSP(CPLD_R_TEST_ADDR);
	if ( ret != 0x0 )
	{
			LldPrint_Error("CPLD read/write test error", 0x0, ret);
	}
	WDM_WR2SDRAM_ADDR(CPLD_W_TEST_ADDR, 0xAAAAAAAA);
	//Sleep(10);
	ret = WDM_Read_TSP(CPLD_R_TEST_ADDR);
	if ( ret != 0xAAAAAAAA )
	{
			LldPrint_Error("CPLD read/write test error", 0xAAAAAAAA, ret);
	}
	WDM_WR2SDRAM_ADDR(CPLD_W_TEST_ADDR, 0x55555555);
	//Sleep(10);
	ret = WDM_Read_TSP(CPLD_R_TEST_ADDR);
	if ( ret != 0x55555555 )
	{
			LldPrint_Error("CPLD read/write test error", 0x55555555, ret);
	}
	for(i = 0; i < 32; i++)
	{
			WDM_WR2SDRAM_ADDR(CPLD_W_TEST_ADDR, 0x01 << i);
			//Sleep(10);
			ret = WDM_Read_TSP(CPLD_R_TEST_ADDR);
			if ( ret != (unsigned int)(0x01 << i) )
			{
				LldPrint_Error("CPLD read/write test error", 0x01 << i, ret);
			}
	}
}
	return 0;
}
/*^^***************************************************************************
 * Description : Access(Test) F/W
 *				
 * Entry : nCount, simple_test
 *
 * Return: -7(fail), 0(success)
 *
 * Notes : 
 *
 **************************************************************************^^*/
int CWdmDrv::WDM_Check_Board_Status(int nCount, int simple_test)
{
	unsigned int ret, i, j=nCount;
	
	LldPrint_FCall("WDM_Check_Board_Status", nCount, simple_test);
#if TVB597A_STANDALONE
	return TLV_NO_ERR;
#endif

	Sleep(100);
	do
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, 0xFFFFFFFF);
		ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
		if ( ret != 0xFFFFFFFF )
		{
			//ret = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);
			LldPrint_Error("wdm_drv : pos... test-dnload-rbf", 0xFFFFFFFF, ret);
//			LldPrint_Error("wdm_drv : conf-sts", 2, ret);
			return TLV_DOWNLOAD_ERR;
		}
		Sleep(10);
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, 0x00000000);
		ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
		if ( ret != 0x00000000 )
		{
			LldPrint_Error("wdm_drv : pos... test-dnload-rbf", 222, ret);
			return TLV_DOWNLOAD_ERR;
		}
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, 0x55555555);
		ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
		if ( ret != 0x55555555 )
		{
			LldPrint_Error("wdm_drv : pos... test-dnload-rbf", 0x55555555, ret);
			return TLV_DOWNLOAD_ERR;
		}
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, 0xAAAAAAAA);
		ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
		if ( ret != 0xAAAAAAAA )
		{
			LldPrint_Error("wdm_drv : pos... test-dnload-rbf", 0xAAAAAAAA, ret);
			return TLV_DOWNLOAD_ERR;
		}
		if ( simple_test == 1 )
		{
			return TLV_NO_ERR;
		}

		for ( i = 0; i < 32; i++ )
		{
			WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, 0x01 << i);
			ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
			if ( ret != (unsigned int)(0x01 << i) )
			{
				LldPrint_Error("wdm_drv : pos... test-dnload-rbf", 0x01 << i, ret);
				return TLV_DOWNLOAD_ERR;
			}
		}

		for ( i = 0; i < 32; i++ )
		{
			WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FPGA_WRITE_TEST, ~(0x01 << i));
			ret = WDM_Read_TSP(TSP_MEM_ADDR_FPGA_READ_TEST);
			if ( ret != (unsigned int)(~(0x01 << i)) )
			{
				LldPrint_Error("wdm_drv : pos... test-dnload-rbf", ~(0x01 << i), ret);
				return TLV_DOWNLOAD_ERR;
			}
		}
	} while ( j-- );
	LldPrint("Succeed to configure F/W");
	return TLV_NO_ERR;
}

/*^^***************************************************************************
 * Description : Access(Test) PCI
 *				
 * Entry : nCount, simple_test
 *
 * Return: -7(fail), 0(success)
 *
 * Notes : 
 *
 **************************************************************************^^*/
int CWdmDrv::WDM_Check_PCI_Status(int nCount, int simple_test)
{
	unsigned int ret, i, j=nCount;

	LldPrint_Trace("WDM_Check_PCI_Status", nCount, simple_test, 0, NULL);
	do
	{
		TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, 0x00000000);
		ret = TSPL_GET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_READ_TEST);
		if ( ret != 0x00000000 )
		{
			LldPrint_Error("wdm_drv : pos...", 2, 7);
			return TLV_DOWNLOAD_ERR;
		}
		TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, 0x55555555);
		ret = TSPL_GET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_READ_TEST);
		if ( ret != 0x55555555 )
		{
			LldPrint_Error("wdm_drv : pos...", 2, 8);
			return TLV_DOWNLOAD_ERR;
		}
		TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, 0xAAAAAAAA);
		ret = TSPL_GET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_READ_TEST);
		if ( ret != 0xAAAAAAAA )
		{
			LldPrint_Error("wdm_drv : pos...", 2, 9);
			return TLV_DOWNLOAD_ERR;
		}
		if ( simple_test == 1 )
		{
			return TLV_NO_ERR;
		}

		for ( i = 0; i < 32; i++ )
		{
			TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, 0x01 << i);
			ret = TSPL_GET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_READ_TEST);
			if ( ret != (unsigned int)(0x01 << i) )
			{
				LldPrint_Error("wdm_drv : pos...", 2, 10);
				return TLV_DOWNLOAD_ERR;
			}
		}

		for ( i = 0; i < 32; i++ )
		{
			TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, ~(0x01 << i));
			ret = TSPL_GET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_READ_TEST);
			if ( ret != (unsigned int)(~(0x01 << i)) )
			{
				LldPrint_Error("wdm_drv : pos...", 2, 11);
				return TLV_DOWNLOAD_ERR;
			}
		}
	} while ( j-- );

	TSPL_SET_DMA_REG_INFO__(TSP_MEM_ADDR_PCI_WRITE_TEST, 0x00000000);

	LldPrint("[LLD]Succeed to read PCI register");
	LldPrint("[LLD]Read PCI9054 Configuration Registers.");
	for(i=0 ; i < 0x3f; i++)
	{
		if(i == 0x0 || i == 0x7 || i == 0x3C || i == 0x3D)
		{
			ret = TSPL_GET_DMA_REG_INFO__(i);
		}
	}

	return TLV_NO_ERR;
}

/*^^***************************************************************************
 * Description : Reset EPLD
 *				
 * Entry :
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int 	_stdcall CWdmDrv::TSPL_RESET_SDCON(void)
{
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LockEpldMutex();
	LldPrint_FCall("TSPL_RESET_SDCON", 0, 0);
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_ASSERT);
		WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_RELEASE);
	}
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_ASSERT);
		WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_RELEASE);
	}
	else
	{
		WDM_WR2SDRAM(CARD_RESET_BOARD);
	}
	UnlockEpldMutex();
	return 0;
}
void 	CWdmDrv::TSPL_RESET_SDCON_backwardcompatible(int comp_mod)
{
	if (IsAttachedBdTyp__Virtual())
	{
		return;
	}
	LldPrint_FCall("TSPL_RESET_SDCON_backwardcompatible", comp_mod, 0);
	switch(comp_mod)
	{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	initial reset
	case	1:
	case	2:
		//TVB590S - ????, cancel CARD_RESET_BOARD?
		if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_590s_ )
		{
			WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_ASSERT);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_RELEASE);
		}
		//TVB593
		else if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_593_ )
		{
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_ASSERT);
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_RELEASE);
		}
		else if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_597_ )
		{
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_ASSERT);
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_RELEASE);
			WDM_WR2SDRAM(CARD_RESET_BOARD);
		}
		else
		{
			WDM_WR2SDRAM(CARD_RESET_BOARD);
		}
		break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case	3:
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_ASSERT);
			WDM_WR2SDRAM_ADDR(PCI590S_REG_RESET_COMMAND, PCI590S_CARD_RESET_ALL_RELEASE);
		}
		else if ( IsAttachedBdTyp_NewAddrMap() )
		{
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_ASSERT);
			WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_RELEASE);
		}
		else
		{
			WDM_WR2SDRAM(CARD_RESET_BOARD);
		}
		break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	reset modulator only
	case	4:
		WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_MOD_ASSERT);
		WDM_WR2SDRAM_ADDR(PCI593_REG_RESET_COMMAND, PCI593_CARD_RESET_ALL_RELEASE);
		break;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	usleep(5000);
}
int CWdmDrv::TVB594_RESET(int change_mod_param)
{
	if (!IsAttachedBdTyp_594())
	{
		return 0;
	}
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}
	LldPrint_FCall("TVB594_RESET", change_mod_param, 0);

	/*
	if ( !change_mod_param )
	{
		//AD9857_RESET
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<2));
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);
	}

	//MOD#_RESET
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<4));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<8));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<12));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<16));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	//MOD#_RA_RESET
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<5));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<9));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<13));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, (0x00000001<<17));
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);
	*/

	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0xFFFFFFFE);
//	LPRINT(fn_Log,"[TVB594]TVB594_RESET ::address=0x%08X, ::data=0x%08X\n", TSP_MEM_ADDR_RESET_CNTL, 0xFFFFFFFE);
	WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);
//	LPRINT(fn_Log,"[TVB594]TVB594_RESET ::address=0x%08X, ::data=0x%08X\n", TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	return 0;
}



