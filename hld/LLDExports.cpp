//*************************************************************************	
//	LLDExports.c / TSPHLD0381.DLL, TSPHLD0390.DLL, TSPHLD0431.DLL
//
//	TVB370/380/390, TSE110 high level DLL
//	Basic skeleton
//
//	version 5.4.0
//	Jun 23, 2006
//	Copyright (C) 2005-2006
//	Teleview Corporation
//	
//	This DLLs are developed to support Teleview TVB370/380/390, TSE110 board.
//
//	Requirement
//*************************************************************************	

#if defined(WIN32)
//IP UDP/RTP
#include	<winsock2.h>

#include	<windows.h>
#include	<stdio.h>
#include	<io.h>
#include	<fcntl.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<stdio.h>
#include 	<string.h>
#include	<fcntl.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include    "hld_type.h"
#include	"../include/lld_const.h"
#endif

//DVB-T2
// huy: add (20110105)
#include "dvbt2_multipleplp.h"

#include	<math.h>

#include	"../include/logfile.h"
#include	"LLDWrapper.h"
#include 	"VLCWrapper.h"

extern int TL_gLastError;

CHld *g_LLDWrapper[]=
{
	NULL,NULL,NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL
};
#define CHECK_DLL_INSTANCE(a)	\
	if ( a < 0 || a > _MAX_INST_CNT_ || g_LLDWrapper[a] == NULL )	\
		return -1;																			\
	if ( g_LLDWrapper[a]->TL_nSupriseRemoval == 1 )							\
		return -1;																			\

static int gRet = 0;
static double gdwRet = 0;

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_AD9775_EX(int nSlot, long reg)
{
	int ret;
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPL_GET_AD9775_EX", 10);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	ret = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_AD9775(reg);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	return ret;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_SET_AD9775_EX(int nSlot, long reg, long data)
{
	int ret;
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPL_SET_AD9775_EX", (int)reg, (int)data);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	ret = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_AD9775(reg, data);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	return ret;
	//return g_LLDWrapper[nSlot]->_FIf->TSPL_SET_AD9775(reg, data);
}

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_BOARD_ID_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_BOARD_ID_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_BOARD_ID();
	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_BOARD_REV_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_BOARD_REV_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_BOARD_REV();
	return gRet;
}

//I/Q PLAY/CAPTURE
extern "C" _declspec(dllexport) int _stdcall TSPL_GET_FPGA_INFO_EX(int nSlot, int info)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_FPGA_INFO_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_FPGA_INFO(info);
	return gRet;
}

//2011/11/22 IQ NEW FILE FORMAT
extern "C" _declspec(dllexport) int _stdcall TSPH_SET_IQ_MODE(int nSlot, int mode, int memory_use, int memory_size, int capture_size)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_IQ_MODE");
	return	g_LLDWrapper[nSlot]->SetIqMode(mode, memory_use, memory_size, capture_size);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_TRY_ALLOC_IQ_MEMORY(int nSlot, int mem_size)
{
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_TRY_ALLOC_IQ_MEMORY");
	return	g_LLDWrapper[nSlot]->TryAlloc_IqMem(mem_size);
}

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_ENCRYPTED_SN_EX(int nSlot, int type, char* sn)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_ENCRYPTED_SN_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_ENCRYPTED_SN(type, sn);
	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_CHECK_LN_EX(int nSlot, char* ln)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_CHECK_LN_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_CHECK_LN(ln);

	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_RESET_SDCON_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_RESET_SDCON_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_RESET_SDCON();
	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TVB380_SET_BOARD_CONFIG_STATUS_EX(int nSlot, long modulator_type, long status)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_BOARD_CONFIG_STATUS_EX", (int)status);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_BOARD_CONFIG_STATUS(modulator_type, status);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_SET_BOARD_LED_STATUS_EX(int nSlot, int status_LED, int fault_LED)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPL_SET_BOARD_LED_STATUS_EX", 11);
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_BOARD_LED_STATUS(status_LED, fault_LED);
	return gRet;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_REG_COMPLETE_EVENT_EX(int nSlot, void* pvoid)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_REG_COMPLETE_EVENT_EX");
	return g_LLDWrapper[nSlot]->_FIf->TSPL_REG_COMPLETE_EVENT(pvoid);
}

extern "C" _declspec(dllexport) int _stdcall TSPL_SET_COMPLETE_EVENT_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_COMPLETE_EVENT_EX");
	return g_LLDWrapper[nSlot]->_FIf->TSPL_SET_COMPLETE_EVENT();
}

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_BOARD_CONFIG_STATUS_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_BOARD_CONFIG_STATUS_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_BOARD_CONFIG_STATUS();
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TSPL_SET_PLAY_RATE_EX(int nSlot, long play_freq_in_herz, long nOutputClockSource)
{
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPL_SET_PLAY_RATE_EX : freq. and out-clk-src", (int)play_freq_in_herz, (int)nOutputClockSource);
	play_freq_in_herz = g_LLDWrapper[nSlot]->PlayRate_IsdbT13(play_freq_in_herz, nOutputClockSource);

	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	gRet = g_LLDWrapper[nSlot]->_FIf->ApplyPlayRate_Calced_or_UserReqed(play_freq_in_herz, nOutputClockSource);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);

	g_LLDWrapper[nSlot]->Set_IpStreaming_TxBitrate(play_freq_in_herz);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_TSIO_DIRECTION_EX(int nSlot, int nDirection)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TSPL_SET_TSIO_DIRECTION_EX", nDirection);
	return	g_LLDWrapper[nSlot]->SetTsio_Direction__HasEndlessWhile(nDirection);
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_GET_TSIO_STATUS_EX(int nSlot, int option)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_TSIO_STATUS_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_TSIO_STATUS(option);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_GET_DMA_REG_INFO_EX(int nSlot, int addr)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_DMA_REG_INFO_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_DMA_REG_INFO(addr);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_DMA_REG_INFO_EX(int nSlot, int addr, int data)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_DMA_REG_INFO_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_DMA_REG_INFO(addr, data);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_DEMUX_CONTROL_TEST_EX(int nSlot, int data)
{
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_DEMUX_CONTROL_TEST_EX");
	if ( g_LLDWrapper[nSlot]->_SysSta->m_nBoardId == 0x2D || g_LLDWrapper[nSlot]->_SysSta->m_nBoardId == 0x2F || g_LLDWrapper[nSlot]->_SysSta->m_nBoardId == 0x3B )
	{
		g_LLDWrapper[nSlot]->TL_nDemuxBlockTest = 1;	
	}
	return g_LLDWrapper[nSlot]->_FIf->TSPL_SET_DEMUX_CONTROL_TEST(data);
}

extern "C" _declspec(dllexport)	unsigned long _stdcall TSPL_READ_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address)
{
	CHECK_DLL_INSTANCE(nSlot);
	unsigned long dwData;
//	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_READ_CONTROL_REG_EX");
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	dwData = g_LLDWrapper[nSlot]->_FIf->TSPL_READ_CONTROL_REG(Is_PCI_Control, address);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
//	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return dwData;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_WRITE_CONTROL_REG_EX(int nSlot, int Is_PCI_Control, unsigned long address, unsigned long dwData)
{
	CHECK_DLL_INSTANCE(nSlot);
//	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPL_WRITE_CONTROL_REG_EX", 12);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_WRITE_CONTROL_REG(Is_PCI_Control, address, dwData);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
//	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	unsigned long _stdcall TSPL_READ_INPUT_STATUS_EX(int nSlot/*, int Is_PCI_Control==0, unsigned long address==0x600042*/)
{
	CHECK_DLL_INSTANCE(nSlot);
//	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_READ_INPUT_STATUS_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->IsAsiInputLocked();	//	_FIf->TSPL_READ_CONTROL_REG(0, 0x600042);
//	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	double _stdcall TSPL_READ_INPUT_TSCOUNT_EX(int nSlot/*, int Is_PCI_Control==0, unsigned long address==0x600044*/)
{
	CHECK_DLL_INSTANCE(nSlot);
	double dwData;
//	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_READ_INPUT_TSCOUNT_EX");
	dwData = (double)g_LLDWrapper[nSlot]->_FIf->BitrateOfAsiInCapedTs();	//	_FIf->TSPL_READ_CONTROL_REG(0, 0x600044));
//	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return dwData;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_SDCON_MODE_EX(int nSlot, int nMode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_SDCON_MODE_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_SDCON_MODE(nMode);
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_RESET_SDCON();
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_STOP_MODE_EX(int nSlot, long stop_mode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_STOP_MODE_EX", (int)stop_mode);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_STOP_MODE(stop_mode);
	
	//2012/4/27
	g_LLDWrapper[nSlot]->_SysSta->SetNullTP_User(stop_mode);
	
	if((g_LLDWrapper[nSlot]->_SysSta->_CntAdditionalVsbRfOut_593_591s() == 0) && 
		(g_LLDWrapper[nSlot]->_SysSta->_CntAdditionalQamRfOut_593_591s() == 0) &&
		(g_LLDWrapper[nSlot]->_SysSta->_CntAdditionalDvbTRfOut_593() == 0)) //2012/6/28 multi dvb-t
	{
		return gRet;
	}

	if(g_LLDWrapper[nSlot]->_SysSta->IsAsior310_LoopThru_DtaPathDirection())
		return gRet;

	if(stop_mode == 1)
		g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_OUTPUT(g_LLDWrapper[nSlot]->_SysSta->TL_gCurrentModType, 0);
	else
		g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_OUTPUT(g_LLDWrapper[nSlot]->_SysSta->TL_gCurrentModType, 1);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_IS_ENABLED_TYPE_EX(int nSlot, long modulator_type)
{
	int ret;
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TVB380_IS_ENABLED_TYPE_EX", 16);
	ret = g_LLDWrapper[nSlot]->_FIf->TVB380_IS_ENABLED_TYPE(modulator_type);
	return ret;
}

extern "C" _declspec(dllexport)	int _stdcall TVB380_SET_CONFIG_EX(int nSlot, long modulator_type, long IF_Frequency)
{
	CHECK_DLL_INSTANCE(nSlot);
	//2010/10/11
	g_LLDWrapper[nSlot]->CHECK_LOCK(0, modulator_type);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_CONFIG_EX");
	gRet = g_LLDWrapper[nSlot]->SetFpgaModulatorTyp(modulator_type, IF_Frequency);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(0);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB380_SET_MODULATOR_FREQ_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB380_SET_MODULATOR_FREQ_EX", (int)symbol_rate_or_bandwidth, output_frequency);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_FREQ(modulator_type, (unsigned long)output_frequency, symbol_rate_or_bandwidth);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_INIT_IP();
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB380_SET_MODULATOR_SYMRATE_EX(int nSlot, long modulator_type, double output_frequency, long symbol_rate_or_bandwidth)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_SYMRATE_EX");
	
	//2011/11/16 IQ NEW FILE FORMAT
	g_LLDWrapper[nSlot]->SetSymbolrate(symbol_rate_or_bandwidth);
	
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_SYMRATE(modulator_type, (unsigned long)output_frequency, symbol_rate_or_bandwidth);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB380_SET_MODULATOR_CODERATE_EX(int nSlot, long modulator_type, long code_rate)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_CODERATE_EX", (int)code_rate);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_CODERATE(modulator_type, code_rate);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_TXMODE_EX(int nSlot, long modulator_type, long tx_mode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_TXMODE_EX", (int)tx_mode);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_TXMODE(modulator_type, tx_mode);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_BANDWIDTH_EX(int nSlot, long modulator_type, long bandwidth, double output_frequency)
{
	CHECK_DLL_INSTANCE(nSlot);

	//TVB593
	if ( modulator_type == 13 )
	{
		g_LLDWrapper[nSlot]->TL_DVB_T2_BW = bandwidth;
	}

	//2011/11/16 IQ NEW FILE FORMAT
	g_LLDWrapper[nSlot]->SetSymbolrate(bandwidth);

	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_BANDWIDTH_EX", (int)bandwidth);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_BANDWIDTH(modulator_type, bandwidth, (unsigned long)output_frequency);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_GUARDINTERVAL_EX(int nSlot, long modulator_type, long guard_interval)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_GUARDINTERVAL_EX", (int)guard_interval);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_GUARDINTERVAL(modulator_type, guard_interval);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_CONSTELLATION_EX(int nSlot, long modulator_type,long constellation)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_CONSTELLATION_EX", (int)constellation);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_CONSTELLATION(modulator_type, constellation);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_INTERLEAVE_EX(int nSlot, long modulator_type, long interleaving)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_INTERLEAVE_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_INTERLEAVE(modulator_type, interleaving);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_IF_FREQ_EX(int nSlot, long modulator_type, long IF_frequency)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_IF_FREQ_EX");
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_IF_FREQ(modulator_type, IF_frequency);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_INIT_IP();
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(int nSlot, long modulator_type, long spectral_inversion)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX", (int)spectral_inversion);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_SPECTRUM_INVERSION(modulator_type, spectral_inversion);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB380_SET_MODULATOR_PRBS_INFO_EX(int nSlot, long modulator_type, long mode, double noise_power)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB380_SET_MODULATOR_PRBS_INFO_EX", (int)mode, noise_power);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_PRBS_INFO(modulator_type, mode, noise_power);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_PRBS_MODE_EX(int nSlot, long modulator_type, long mode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_PRBS_MODE_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_PRBS_MODE(modulator_type, mode);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_PRBS_SCALE_EX(int nSlot, long modulator_type, double noise_power)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_PRBS_SCALE_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_PRBS_SCALE(modulator_type, noise_power);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_ATTEN_VALUE_EX(int nSlot, long modulator_type, double atten_value, long UseTAT4710)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB380_SET_MODULATOR_ATTEN_VALUE_EX", 0, atten_value);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_ATTEN_VALUE(modulator_type, atten_value, UseTAT4710);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_INIT_IP();
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//2012/9/6 Pcr Restamping control
extern "C" _declspec(dllexport)	int _stdcall TVB59x_SET_PCR_STAMP_CNTL_EX(int nSlot, int _val)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX", 0, _val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB59x_SET_PCR_STAMP_CNTL(_val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//2013/5/31 TVB599 TS output type(ASI, 310M)
extern "C" _declspec(dllexport)	int _stdcall TVB59x_SET_Output_TS_Type_EX(int nSlot, int _val)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_SET_Output_TS_Type_EX", 0, _val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB59x_SET_Output_TS_Type(_val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//2013/6/10 TVB599 reset control
extern "C" _declspec(dllexport)	int _stdcall TVB59x_SET_Reset_Control_REG_EX(int nSlot, int _val)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_SET_Reset_Control_REG_EX", 0, _val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB59x_SET_Reset_Control_REG(_val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}
//2013/5/3 ASI bitrate
extern "C" _declspec(dllexport)	int _stdcall TSPL_TVB59x_SET_TsPacket_CNT_Mode_EX(int nSlot, int _val)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TSPL_TVB59x_SET_TsPacket_CNT_Mode_EX", 0, _val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->Func_TVB59x_SET_TsPacket_CNT_Mode(_val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TSPL_TVB59x_Get_Asi_Input_rate_EX(int nSlot, int *delta_packet, int *delta_clock)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TSPL_TVB59x_Get_Asi_Input_rate_EX", 0, 0);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->Func_TVB59x_Get_Asi_Input_rate(delta_packet, delta_clock);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB59x_Modulator_Status_Control_EX(int nSlot, int modulator, int index, int val)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_Modulator_Status_Control_EX", index, val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->Func_TVB59x_Modulator_Status_Control(modulator, index, val);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall TVB59x_Get_Modulator_Status_EX(int nSlot, int index)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_Get_Modulator_Status_EX", 0, index);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->Func_TVB59x_Get_Modulator_Status(index);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//2012/8/29 new rf level control
extern "C" _declspec(dllexport)	int _stdcall	TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX(
			int nSlot, 
			long modulator_type, 
			double rf_level_value, 
			long *AmpFlag, 
			long UseTAT4710)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB59x_SET_MODULATOR_RF_LEVEL_VALUE_EX", 0, rf_level_value);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB59x_SET_MODULATOR_RF_LEVEL_VALUE(modulator_type, rf_level_value, AmpFlag, UseTAT4710);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_INIT_IP();
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//2012/8/29 new rf level control
extern "C" _declspec(dllexport)	int _stdcall	TVB59x_GET_MODULATOR_RF_LEVEL_RANGE_EX(
			int nSlot, 
			long modulator_type, 
			double *rf_level_min, 
			double *rf_level_max, 
			long UseTAT4710)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB59x_GET_MODULATOR_RF_LEVEL_RANGE(modulator_type, rf_level_min, rf_level_max, UseTAT4710);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_AGC_EX(int nSlot, long modulator_type, long agc_on_off, long UseTAT4710)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_AGC_EX");
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_AGC(modulator_type, agc_on_off, UseTAT4710);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_INIT_IP();
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	double _stdcall	TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX(int nSlot, long modulator_type, long info_type)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TVB380_GET_MODULATOR_RF_POWER_LEVEL_EX", 17);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	gdwRet = g_LLDWrapper[nSlot]->_FIf->TVB380_GET_MODULATOR_RF_POWER_LEVEL(modulator_type, info_type);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gdwRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_OPEN_EX(int nSlot, int nInitModulatorType, int nInitIF)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_OPEN_EX");
	return g_LLDWrapper[nSlot]->_FIf->TVB380_OPEN(nInitModulatorType, nInitIF);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPL_GET_DMA_STATUS_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_DMA_STATUS_EX");
	return g_LLDWrapper[nSlot]->_FIf->TSPL_GET_DMA_STATUS();
}

extern "C" _declspec(dllexport)	int	_stdcall	TVB380_SET_MODULATOR_DVBH_EX(int nSlot, long modulator_type, long tx_mode, long in_depth_interleave, long time_slice, long mpe_fec, long cell_id)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_DVBH_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_DVBH(modulator_type, tx_mode, in_depth_interleave, time_slice, mpe_fec, cell_id);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPL_GET_LAST_ERROR_EX(int nSlot)
{
	//TVB593
	/*
	CHECK_DLL_INSTANCE(nSlot);
	return g_LLDWrapper[nSlot]->TSPL_GET_LAST_ERROR();
	*/
	return TL_gLastError;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_PILOT_EX(int nSlot, long modulator_type, long  pilot_on_off)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_PILOT_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_PILOT(modulator_type, pilot_on_off);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(int nSlot, long modulator_type, long  roll_off_factor)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_ROLL_OFF_FACTOR(modulator_type, roll_off_factor);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_BERT_MEASURE_EX(int nSlot, long modulator_type, long packet_type, long bert_pid)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_BERT_MEASURE_EX", (int)packet_type);
	return	g_LLDWrapper[nSlot]->SetMidulator_Bert_Measure(modulator_type, packet_type, bert_pid);
}

extern "C" _declspec(dllexport)	double _stdcall	TVB380_GET_MODULATOR_BERT_RESULT_EX(int nSlot, long modulator_type)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_GET_MODULATOR_BERT_RESULT_EX");
	return g_LLDWrapper[nSlot]->GetBer();
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_DTMB_EX(int nSlot, long modulator_type, long constellation, long code_rate, long interleaver, long frame_header, long carrier_number, long frame_header_pn, long pilot_insertion)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB380_SET_MODULATOR_DTMB_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_DTMB(modulator_type, constellation, code_rate, interleaver, frame_header, carrier_number, frame_header_pn, pilot_insertion);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

extern "C" _declspec(dllexport)	int	_stdcall TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(int nSlot, long modulator_type, long sdram_clock)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TVB380_SET_MODULATOR_SDRAM_CLOCK_EX", (int)sdram_clock);
	return g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_SDRAM_CLOCK(modulator_type,sdram_clock);
}

//--------------------------------------------------------------------------
extern "C" _declspec(dllexport) int _stdcall TSPH_SET_SDRAM_BANK_INFO(int nSlot, int nBankCount, int nBankOffset)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPH_SET_SDRAM_BANK_INFO : bnk-cnt and bnk-off ...", nBankCount, nBankOffset);
	return	g_LLDWrapper[nSlot]->_FIf->SetSdram_BankInfo(nBankCount, nBankOffset);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_SET_FILENAME_NEXT_PLAY(int nSlot, char *szFile)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_FILENAME_NEXT_PLAY");
	return	g_LLDWrapper[nSlot]->_FIf->SetFileName_NxtPlay(szFile);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_START_PLAY(int nSlot, char *szfnNewPlayFile, LONG PlayFreq, double DblStartOffset/*always 0*/, long nUseMaxPlayrate, long fRepeatMode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_START_PLAY : file, platrate, playratemax, repeatmode", szfnNewPlayFile, (int)PlayFreq, (int)nUseMaxPlayrate, (int)fRepeatMode);
	//2011/11/01 added PAUSE
	if(g_LLDWrapper[nSlot]->_SysSta->IsModTyp_Tdmb())
		g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_OUTPUT(g_LLDWrapper[nSlot]->_SysSta->TL_gCurrentModType, 1);
	else
		g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_OUTPUT(g_LLDWrapper[nSlot]->_SysSta->TL_gCurrentModType, 0);
	return	g_LLDWrapper[nSlot]->Req_Sta_byUser_StartPlay(szfnNewPlayFile, PlayFreq, DblStartOffset, nUseMaxPlayrate, fRepeatMode);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_GET_PLAY_BUFFER_STATUS(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPH_GET_PLAY_BUFFER_STATUS", 13);
	return	g_LLDWrapper[nSlot]->GetPlay_BufSts();
}
extern "C" _declspec(dllexport)	int	_stdcall    TSPH_START_RECORD(int nSlot, char* szNewRecordFile, int nPort)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_START_RECORD");
	return	g_LLDWrapper[nSlot]->Req_Sta_byUser_StartRevord(szNewRecordFile, nPort);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_CAL_PLAY_RATE(int nSlot, char *szFile, int iType)
{
	CHECK_DLL_INSTANCE(nSlot);
	int	_prate;
	if(g_LLDWrapper[nSlot]->_SysSta->IsModTyp_Tdmb())
	{
		_prate = g_LLDWrapper[nSlot]->Verify_ETI_Format(szFile);
	}
	else
		_prate = g_LLDWrapper[nSlot]->CalcPlayRate_SpecifiedFile(szFile, iType);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPH_CAL_PLAY_RATE : calc-typ. and result", iType, _prate);
	return	_prate;
}

extern "C" _declspec(dllexport)	int	_stdcall   TSPH_START_MONITOR(int nSlot, int nPort)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_START_MONITOR");
	return	g_LLDWrapper[nSlot]->Req_Sta_byUser_StartMonitor(nPort);
}

extern "C" _declspec(dllexport)	int	_stdcall    TSPH_START_DELAY(int nSlot, int nPort)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_START_DELAY");
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_OUTPUT(g_LLDWrapper[nSlot]->_SysSta->TL_gCurrentModType, nPort);
	return g_LLDWrapper[nSlot]->Req_Sta_byUser_StartDelay(nPort);
}

extern "C" _declspec(dllexport)	int	_stdcall    	TSPH_SET_REPEAT_PLAY_MODE(int nSlot, int fRepeatMode)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_REPEAT_PLAY_MODE");
	return	g_LLDWrapper[nSlot]->_FIf->SetRepeat_PlayMode(fRepeatMode);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_GET_CURRENT_THREAD_STATE(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPH_GET_CURRENT_THREAD_STATE", 14);
	return g_LLDWrapper[nSlot]->_SysSta->GetCurTaskSta();
}

extern "C" _declspec(dllexport) double _stdcall TSPH_GET_CURRENT_RECORD_POINT(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_CURRENT_RECORD_POINT");
	double dwRecordedSize;
	if(g_LLDWrapper[nSlot]->_SysSta->IsModTyp_DvbT2() || g_LLDWrapper[nSlot]->_SysSta->IsModTyp_DvbC2())
		dwRecordedSize = (double)(g_LLDWrapper[nSlot]->_FIf->BytesCapFile());
	else
		dwRecordedSize = (double)(g_LLDWrapper[nSlot]->_FIf->GetCur_Rec_Pos());	//SKIP THE FIRST 16MB
	return (dwRecordedSize <= 0 ? 0. : dwRecordedSize);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_SET_MODULATOR_TYPE(int nSlot, long type)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(0, type);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_MODULATOR_TYPE");
	g_LLDWrapper[nSlot]->_SysSta->SetModulatorTyp(type);
	g_LLDWrapper[nSlot]->_SysSta->SetFlag_DtaPathDirection(0);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(0);

	return 0;
}

extern "C" _declspec(dllexport) int _stdcall TSPH_SET_CURRENT_OFFSET(int nSlot, int nOffsetType, double dwOffset)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TSPH_SET_CURRENT_OFFSET : typ and offset ... ", nOffsetType, dwOffset);
	return	g_LLDWrapper[nSlot]->_FIf->SetCurrentOffset(nOffsetType, dwOffset);
}

#ifdef	TSPHLD_VLC
extern "C" _declspec(dllexport) int _stdcall TSPH_START_IP_STREAMING(
	int nSlot, char *szFilePath, long nPlayrate, double dStartOffset/*always 0*/,
	long nOption, long nRepeatMode, long nExtendedMode/*Operation mode thru. IP*/,
	char *szExtendedInfo/*Source or Target thru. IP*/)
{
	int nSlotNum = nSlot;
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_START_IP_STREAMING");
	g_LLDWrapper[nSlot]->Start_IpStreaming_OnUserReq(szFilePath, nPlayrate, dStartOffset, nOption, nRepeatMode, nExtendedMode, szExtendedInfo);
	return	g_LLDWrapper[nSlot]->Start_IpStreaming_OnUserReq_ExtCntl(szFilePath, nPlayrate, dStartOffset, nOption, nRepeatMode, nExtendedMode, szExtendedInfo);

}

extern "C" _declspec(dllexport) long _stdcall TSPH_READ_IP_STREAMING_INPUT_STATUS(int nSlot, long nStatus)
{
	CHECK_DLL_INSTANCE(nSlot);
	long	_ret;
	_ret = g_LLDWrapper[nSlot]->StsIpRx_OnUserReq(nStatus);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait_3("App-Call : TSPH_READ_IP_STREAMING_INPUT_STATUS", 28, nStatus, _ret);
	return	_ret;
}

extern "C" _declspec(dllexport) long _stdcall TSPH_SET_PROGRAM(int nSlot, long nIndex)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_PROGRAM");
	return	g_LLDWrapper[nSlot]->SetIpProg_OnUserReq(nIndex);
}

extern "C" _declspec(dllexport) long _stdcall TSPH_GET_PROGRAM(int nSlot, long nIndex)
{
	CHECK_DLL_INSTANCE(nSlot);
	long	_ret;
	_ret = g_LLDWrapper[nSlot]->GetIpProg_OnUserReq(nIndex);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait_3("App-Call : TSPH_GET_PROGRAM", 27, nIndex, _ret);
	return	_ret;
}

extern "C" _declspec(dllexport) int _stdcall TSPH_SHOW_VIDEO_WINDOW(int nSlot, int nShowWindow)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SHOW_VIDEO_WINDOW");
	return	g_LLDWrapper[nSlot]->VlcShowVid_OnUserReq(nShowWindow);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_MOVE_VIDEO_WINDOW(int nSlot, int nX, int nY, int nW, int nH)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_MOVE_VIDEO_WINDOW");
	return	g_LLDWrapper[nSlot]->VlcMoveVid_OnUserReq(nX, nY, nW, nH);
}

extern "C" _declspec(dllexport) int _stdcall TSPH_IS_VIDEO_WINDOW_VISIBLE(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	int	_ret;
	_ret = g_LLDWrapper[nSlot]->IsVlcVidVisible_OnUserReq();
	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait_2("App-Call : TSPH_IS_VIDEO_WINDOW_VISIBLE", 29, _ret);
	return	_ret;
}

extern "C" _declspec(dllexport) int _stdcall TSPH_EXIT_PROCESS(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_EXIT_PROCESS");
#if defined(WIN32)
	::ExitProcess(1);
#else
#endif
	return 0;	
}

#endif

extern "C" _declspec(dllexport) int _stdcall	TSPL_SET_FIFO_CONTROL_EX(int nSlot, long dma_direction, long dma_size)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_FIFO_CONTROL_EX");
	return g_LLDWrapper[nSlot]->_FIf->TSPL_SET_FIFO_CONTROL(dma_direction, dma_size);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_GET_REMUX_INFO(int nSlot, char *szFile, int layer_index)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_REMUX_INFO");
	return	g_LLDWrapper[nSlot]->GetRmx_Info(szFile, layer_index);
}

extern "C" _declspec(dllexport)	int	_stdcall TSPH_SET_REMUX_INFO(int nSlot, long btype, long mode, long guard_interval, long partial_reception, long bitrate,
													long a_segments, long a_modulation, long a_code_rate, long a_time_interleave, long a_bitrate,
													long b_segments, long b_modulation, long b_code_rate, long b_time_interleave, long b_bitrate,
													long c_segments, long c_modulation, long c_code_rate, long c_time_interleave, long c_bitrate)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_REMUX_INFO");
	layer *pLayer;
	int NumOfSegs = a_segments + b_segments + c_segments;
	if ( (btype == 0 && NumOfSegs != 13)
		|| (btype == 1 && NumOfSegs != 1)
		|| (btype == 2 && NumOfSegs != 3) 
		|| (btype == 3 && NumOfSegs != 1) )
	{
		return -1;
	}
	if ( btype == 0 && partial_reception == 1 && a_segments != 1 )
	{
		return -2;
	}
	if ( btype == 3 && (partial_reception != 1 || a_segments != 1) )
	{
		return -3;
	}
		
	g_LLDWrapper[nSlot]->tmcc_param.m_BrodcastType = btype;
	g_LLDWrapper[nSlot]->tmcc_param.m_GuardInterval = guard_interval;
	g_LLDWrapper[nSlot]->tmcc_param.m_Mode = mode;
	g_LLDWrapper[nSlot]->tmcc_param.m_PartialRecev = partial_reception;

	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[0].m_NumberOfSegments = a_segments;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[0].m_modulation = a_modulation;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[0].m_CodeRate = a_code_rate;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[0].m_TimeInterleave = a_time_interleave;

	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[1].m_NumberOfSegments = b_segments;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[1].m_modulation = b_modulation;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[1].m_CodeRate = b_code_rate;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[1].m_TimeInterleave = b_time_interleave;

	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[2].m_NumberOfSegments = c_segments;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[2].m_modulation = c_modulation;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[2].m_CodeRate = c_code_rate;
	g_LLDWrapper[nSlot]->tmcc_param.m_Layers[2].m_TimeInterleave = c_time_interleave;


	return	g_LLDWrapper[nSlot]->SetRmx_Info(btype, mode, guard_interval, partial_reception, bitrate,
		a_segments, a_modulation, a_code_rate, a_time_interleave, a_bitrate,
		b_segments, b_modulation, b_code_rate, b_time_interleave, b_bitrate,
		c_segments, c_modulation, c_code_rate, c_time_interleave, c_bitrate);
}
//2011/6/20 ISDB-T 1/13seg bug fixed
extern "C" _declspec(dllexport)	int	_stdcall TSPH_CLEAR_REMUX_INFO(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_CLEAR_REMUX_INFO");
	g_LLDWrapper[nSlot]->CLEAR_REMUX_INFO();
	return 0;
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_GET_REMUX_DATARATE(int nSlot, long layer_index)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_REMUX_DATARATE");
	return	g_LLDWrapper[nSlot]->GetRmxDtaRate(layer_index);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_SET_TMCC_REMUXER(int nSlot, long use_tmcc_remuxer)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_TMCC_REMUXER");
	return	g_LLDWrapper[nSlot]->_FIf->SetTmccRmx_Mode(use_tmcc_remuxer);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_SET_LAYER_INFO(int nSlot,
									long other_pid_map_to_layer,
									long multi_pid_map,
									long total_pid_count, char* total_pid_info,
									long a_pid_count, char* a_pid_info,
									long b_pid_count, char* b_pid_info,
									long c_pid_count, char* c_pid_info)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_LAYER_INFO");
	if (!g_LLDWrapper[nSlot]->_SysSta->is_new_playback())
	{
		g_LLDWrapper[nSlot]->TMCC_SET_LAYER_INFO(other_pid_map_to_layer, multi_pid_map, 
			total_pid_count, total_pid_info,
			a_pid_count, a_pid_info,
			b_pid_count, b_pid_info,
			c_pid_count, c_pid_info);
		return 0;
	}


	// why the pids list is in a string format? what is it benifit??? 
	// why it is not an array ???????
		
	for (int i = 1; i < strlen(total_pid_info); i++)
	{
		if (a_pid_info[i] == ',')
			a_pid_info[i] = ' ';
	}
	g_LLDWrapper[nSlot]->isdbt_tmcc_layer_info.layer_a_num_pids = a_pid_count;
	int *layer_a_pids = g_LLDWrapper[nSlot]->isdbt_tmcc_layer_info.layer_a_pids;

	// eck eck, what happend if num of pids larger than 15 ???
	sscanf(a_pid_info, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
		&layer_a_pids[0],
		&layer_a_pids[1], 
		&layer_a_pids[2], 
		&layer_a_pids[3], 
		&layer_a_pids[4], 
		&layer_a_pids[5], 
		&layer_a_pids[6], 
		&layer_a_pids[7], 
		&layer_a_pids[8], 
		&layer_a_pids[9],
		&layer_a_pids[10],
		&layer_a_pids[11],
		&layer_a_pids[12],
		&layer_a_pids[13],
		&layer_a_pids[14]);

	return 0;
}


extern "C" _declspec(dllexport)	int	_stdcall TSPH_SET_RX_IP_STREAMING_INFO(int nSlot, char* src_ip, char* rx_multicast_ip, long rx_udp_port, char* local_ip, long fec_udp_off, long fec_inact)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_1("App-Call : TSPH_SET_RX_IP_STREAMING_INFO", rx_udp_port);
	return	g_LLDWrapper[nSlot]->SetRx_IpStreamingInfo(src_ip, rx_multicast_ip, rx_udp_port, local_ip, fec_udp_off, fec_inact);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_SET_ERROR_INJECTION(int nSlot, long error_lost, long error_lost_packet,
																		 long error_bits, long error_bits_packet, long error_bits_count,
																		 long error_bytes, long error_bytes_packet, long error_bytes_count)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_ERROR_INJECTION");
	return	g_LLDWrapper[nSlot]->Set_ErrInjection_Param(error_lost, error_lost_packet, error_bits, error_bits_packet, error_bits_count, error_bytes, error_bytes_packet, error_bytes_count);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPL_GET_AUTHORIZATION_EX(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_GET_AUTHORIZATION_EX");
	return g_LLDWrapper[nSlot]->_FIf->TSPL_GET_AUTHORIZATION();
}

//CMMB
#define MFS_INFO_PATH		"mfs_info"
extern "C" _declspec(dllexport)	int	_stdcall	TSPH_RUN_MFS_PARSER(int nSlot, char *szFile, char *szResult)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_RUN_MFS_PARSER");
	return	g_LLDWrapper[nSlot]->__Cmmb->RunMfs_CmmbParser(szFile, szResult);
}

//TDT/TOT - USER DATE/TIME
extern "C" _declspec(dllexport)	int	_stdcall	TSPH_SET_LOOP_ADAPTATION(int nSlot, long pcr_restamping, long continuity_conunter, long tdt_tot, char* user_date, char* user_time)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_LOOP_ADAPTATION", user_date, user_time, pcr_restamping, continuity_conunter, tdt_tot);
	return	g_LLDWrapper[nSlot]->_FIf->SetLoop_AdaptParam(pcr_restamping, continuity_conunter, tdt_tot, user_date, user_time);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_GET_LOOP_COUNT(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->_HLog->HldPrint_CntWait("App-Call : TSPH_GET_LOOP_COUNT", 15);
	return g_LLDWrapper[nSlot]->_FIf->TL_gNumLoop;
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_GET_MHE_PACKET_INFO(int nSlot, char *szFile, int iType)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_MHE_PACKET_INFO");
	return	g_LLDWrapper[nSlot]->GetAtscMh_PktInfo(szFile, iType);
}

extern "C" _declspec(dllexport)	int	_stdcall	TSPH_RUN_ATSC_MH_PARSER(int nSlot, char *szFile, char *szResult)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_RUN_ATSC_MH_PARSER");
	return	g_LLDWrapper[nSlot]->RunAtscMhParser(szFile, szResult);
}
#ifdef WIN32
extern "C" _declspec(dllexport)	int	_stdcall TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, T2MI_Parsing_Info *szResult)
#else
extern "C" _declspec(dllexport)	int	_stdcall TSPH_RUN_T2MI_PARSER(int nSlot, char *szFile, char *szResult)
#endif
{
	CHECK_DLL_INSTANCE(nSlot);

	double avg_bitrate = 0;
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_RUN_T2MI_PARSER");
	avg_bitrate = g_LLDWrapper[nSlot]->RunT2MiParser(szFile, szResult);

	return (int)avg_bitrate;
}
extern "C" _declspec(dllexport)	int	_stdcall TSPH_RUN_C2MI_PARSER(int nSlot, char *szFile, char *szResult)
{
	CHECK_DLL_INSTANCE(nSlot);
	double avg_bitrate = 0;
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_RUN_C2MI_PARSER");
	avg_bitrate = g_LLDWrapper[nSlot]->RunC2MiParser(szFile, szResult);

	return (int)avg_bitrate;
}
extern "C" _declspec(dllexport)	int	_stdcall TSPH_SET_T2MI_PARAMS(int nSlot, 
	int BW, int FFT_SIZE, int GUARD_INTERVAL, int L1_MOD, int PILOT_PATTERN, int BW_EXT, double FREQUENCY,
	int NETWORK_ID, int T2_SYSTEM_ID, int CELL_ID, int S1, int PLP_MOD, int PLP_COD, int PLP_FEC_TYPE, int HEM, 
	int NUM_T2_FRAMES, int NUM_DATA_SYMBOLS, int PLP_NUM_BLOCKS, int PID
	//DVB-T2 - Multi PLP MUX
#ifdef _MULTI_PLP_MUX_0
	, int PLP_ROTATION, int PLP_COUNT, int PLP_ID, int PLP_BITRATE, int PLP_TS_BITRATE, char *PLP_TS, int TIME_IL_LENGTH
#endif
	)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_T2MI_PARAMS");
	g_LLDWrapper[nSlot]->SetT2MiParam(BW, FFT_SIZE, GUARD_INTERVAL, L1_MOD, PILOT_PATTERN, BW_EXT, FREQUENCY,
		NETWORK_ID, T2_SYSTEM_ID, CELL_ID, S1, PLP_MOD, PLP_COD, PLP_FEC_TYPE, HEM, NUM_T2_FRAMES, NUM_DATA_SYMBOLS, PLP_NUM_BLOCKS, PID
	//DVB-T2 - Multi PLP MUX
#ifdef _MULTI_PLP_MUX_0
		, PLP_ROTATION, PLP_COUNT, PLP_ID, PLP_BITRATE, PLP_TS_BITRATE, PLP_TS, TIME_IL_LENGTH
#endif
	);
	return 0;
}

extern "C" _declspec(dllexport)	int	_stdcall TSPH_GET_T2MI_PARAMS(int nSlot, int *num_data_symbol, int *plp_num_block){
	
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_T2MI_PARAMS");
	int rtcd = g_LLDWrapper[nSlot]->GetT2MiParam(num_data_symbol, plp_num_block);

	return rtcd;
}
extern "C" _declspec(dllexport)	int	_stdcall TSPH_SET_C2MI_PARAMS(
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
	int C2_createFile 
	)
{
	CHECK_DLL_INSTANCE(nSlot);

	//C2_createFile = 1;
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_C2MI_PARAMS");
	g_LLDWrapper[nSlot]->SetC2MiParam(C2_BW, C2_L1, C2_Guard, C2_Network, C2_System,
		C2_StartFreq, C2_NumNoth, C2_RevTone, C2_NotchStart, C2_NotchWidth, C2_Dslice_type,
		C2_Dslice_FecHeder, C2_Dslice_BBHeder, C2_Dslice_TonePos, C2_Dslice_OffRight,
		C2_Dslice_OffLeft, C2_Plp_Mod, C2_Plp_Code, C2_Plp_Fec, C2_Plp_Count, C2_Plp_ID,
		C2_Plp_Blk, C2_Plp_TS_Bitrate, C2_Plp_TS, C2_createFile
	);
	return 0;

}

//2011/5/4 AD9852 MAX CLK
#ifdef WIN32
extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_AD9852_MAX_CLOCK_EX(int nSlot, long value)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_AD9852_MAX_CLOCK_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_AD9852_MAX_CLOCK(value);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}
#endif

//TVB593
extern "C" _declspec(dllexport)	int _stdcall TSPL_SET_MAX_PLAYRATE_EX(int nSlot, long modulator_type, long use_max_playrate)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPL_SET_MAX_PLAYRATE_EX");
	gRet = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_MAX_PLAYRATE(modulator_type, use_max_playrate);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}

//ISDB-S
extern "C" _declspec(dllexport)	int	_stdcall TSPH_SET_COMBINER_INFO(int nSlot, int ts_count, char *ts_path, long modulation, long code_rate, long slot_count)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_COMBINER_INFO");
	return	g_LLDWrapper[nSlot]->SetCombinerInfo_IsdbS(ts_count, ts_path, modulation, code_rate, slot_count);
}

extern "C" _declspec(dllexport)	int	_stdcall TSPH_GET_TS_ID(int nSlot, char* szFile)
{
	CHECK_DLL_INSTANCE(nSlot);
	
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_TS_ID");
	int ts_id = -1, j, nReadBytes;
	unsigned char ts_buf[PKTSIZE];
	unsigned short PID;
	FILE *fp = fopen(szFile, "rb");

	if ( fp == NULL )
	{
		return ts_id;
	}

	for ( j = 0; j < 120000; j++ )
	{
		nReadBytes = g_LLDWrapper[nSlot]->_FIf->read_packet(fp, ts_buf, PKTSIZE);
		if ( nReadBytes < 0  )
		{
			break;
		}

		PID = (((*(ts_buf+1))&0x1F)<<8) + *(ts_buf+2);
		if ( PID == 0 )
		{
			ts_id = g_LLDWrapper[nSlot]->_FIf->get_tsid(ts_buf);
			break;
		}
	}

	fclose(fp);

	return ts_id;
}

//2011/3/18 ISDB-S
extern "C" _declspec(dllexport) int _stdcall TSPH_ISDBS_CALC_COMBINED_TS_BITRATE(int nSlot, char* ts_path)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_ISDBS_CALC_COMBINED_TS_BITRATE");
	return	g_LLDWrapper[nSlot]->CalcPlayRate_IsdbS_SpecifiedFile(ts_path);
}
//2011/2/8 ISDB-S
extern "C" _declspec(dllexport) int _stdcall TSPH_IS_COMBINED_TS(int nSlot, char* ts_path)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_IS_COMBINED_TS");
	return	g_LLDWrapper[nSlot]->SetIsCombinedTs_IsdbS(ts_path);
}

extern "C" _declspec(dllexport)	int _stdcall TVB597F_SET_FLASH_EX(int nSlot, char *szFile)
{
#if TVB597A_STANDALONE
	CHECK_DLL_INSTANCE(nSlot);

	g_LLDWrapper[nSlot]->CHECK_LOCK(0, modulator_type);

	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TVB597F_SET_FLASH_EX");
	g_LLDWrapper[nSlot]->_FIf->TSPL_SET_DMA_DIRECTION(g_LLDWrapper[nSlot]->TL_gCurrentModType, (1<<2));
	g_LLDWrapper[nSlot]->_FIf->TVB597F_SET_FLASH(szFile);

	g_LLDWrapper[nSlot]->CHECK_UNLOCK(0);
#endif
	return 0;
}

//2011/4/4 ISDB-S Bitrate
extern "C" _declspec(dllexport) int _stdcall TSPH_SET_ISDBS_BASE_TS(int nSlot, char* ts_path)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_ISDBS_BASE_TS");
	return	g_LLDWrapper[nSlot]->SetBaseTs_IsdbS(ts_path);
}
extern "C" _declspec(dllexport) int _stdcall TSPH_IP_RECV_STATUS(int nSlot, unsigned int *_buf_bytes_avable, unsigned int *_cnt_lost_pkt)
{
	CHECK_DLL_INSTANCE(nSlot);
//	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_IP_RECV_STATUS");
	*_buf_bytes_avable = g_LLDWrapper[nSlot]->CntBytesInIpBuf();
	*_cnt_lost_pkt = g_LLDWrapper[nSlot]->CntIpLostPkts();
	return	1;
}

extern "C" _declspec(dllexport) int _stdcall TSPH_IS_LOOPTHRU_ISDBT13_188(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	//return	g_LLDWrapper[nSlot]->IsLoopthru188_IsdbT13();
	return g_LLDWrapper[nSlot]->Get_LoopThru_PacketSize();
}
extern "C" _declspec(dllexport) int _stdcall TSPH_IS_LOOPTHRU_INBUF_FULL_ISDBT13_188(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	return	g_LLDWrapper[nSlot]->IsLoopthru188InputFullness_IsdbT13();
}
//2012/8/23
extern "C" _declspec(dllexport) int _stdcall TSPH_PAUSE_LOOPTHRU_ISDBT13_Parser(int nSlot, struct _TSPH_TS_INFO **ppTsInfo)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->PauseLoopthru188_IsdbT13(ppTsInfo);
	return	1;
}


extern "C" _declspec(dllexport) int _stdcall TSPH_BUF_CLEAR_LOOPTHRU_ISDBT13_188(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->Buf_Clear_Loopthru188_IsdbT13();
	return	1;
}
extern "C" _declspec(dllexport) int _stdcall	TSPH_GET_NIT_SATELLITE_INFO(
																			int nSlot, 
																			int *descriptor_flag,
																			int *freq,
																			int *rolloff,
																			int *modulation_system,
																			int *modulation,
																			int *symbolrate,
																			int *coderate)
{
#ifdef WIN32
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_NIT_SATELLITE_INFO");
//CKIM TEST M {
//	g_LLDWrapper[nSlot]->Get_NIT_Satellite_Info(descriptor_flag, freq, rolloff, modulation_system, modulation, symbolrate, coderate);
	g_LLDWrapper[nSlot]->Get_NIT_Satellite_Info2(descriptor_flag, freq, rolloff, modulation_system, modulation, symbolrate, coderate);
//CKIM TEST M }
#else
	*descriptor_flag = 0;
	*freq = 0;
	*modulation = 0;
	*symbolrate = 0;
	*coderate = 0;
#endif
	return 0;
}

extern "C" _declspec(dllexport) int _stdcall	TSPH_GET_NIT_CABLE_INFO(
																			int nSlot, 
																			int *descriptor_flag,
																			int *freq,
																			int *modulation,
																			int *symbolrate,
																			int *coderate)
{
#ifdef WIN32
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_NIT_SATELLITE_INFO");
//CKIM TEST M {
//	g_LLDWrapper[nSlot]->Get_NIT_Cable_Info(descriptor_flag, freq, modulation, symbolrate, coderate);
	g_LLDWrapper[nSlot]->Get_NIT_Cable_Info2(descriptor_flag, freq, modulation, symbolrate, coderate);
//CKIM TEST M }
#else
	*descriptor_flag = 0;
	*freq = 0;
	*modulation = 0;
	*symbolrate = 0;
	*coderate = 0;
#endif
	return 0;
}

extern "C" _declspec(dllexport) int _stdcall	TSPH_GET_NIT_TERRESTRIAL_INFO(int nSlot, int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
										int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod)
{
#ifdef WIN32
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_GET_NIT_TERRESTRIAL_INFO");
//CKIM TEST M {
//	g_LLDWrapper[nSlot]->Get_NIT_Terrestrial_Info(descriptor_flag, freq, bw, time_slicing, mpe_fec, 
	g_LLDWrapper[nSlot]->Get_NIT_Terrestrial_Info2(descriptor_flag, freq, bw, time_slicing, mpe_fec, 
//CKIM TEST M }
								constellation, coderate, guard, txmod);

#else
#endif
	return 0;
}


//2012/4/12 SINGLE TONE
extern "C" _declspec(dllexport)	int _stdcall	TVB380_SET_MODULATOR_SINGLE_TONE_EX(int nSlot, long modulator_type, unsigned long freq, long singleTone)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->CHECK_LOCK(2, 0);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2_d("App-Call : TVB380_SET_MODULATOR_SINGLE_TONE_EX", modulator_type, singleTone);
	gRet = g_LLDWrapper[nSlot]->_FIf->TVB380_SET_MODULATOR_SINGLE_TONE(modulator_type, freq, singleTone);
	g_LLDWrapper[nSlot]->CHECK_UNLOCK(2);
	return gRet;
}
//CKIM A 20120805 {


extern "C" _declspec(dllexport)	int _stdcall	TSPH_RUN_TS_PARSER2(int nSlot, char *szFile, int default_bitrate, struct _TSPH_TS_INFO **ppTsInfo)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_RUN_TS_PARSER2");
//CKIM TEST {
//	return	g_LLDWrapper[nSlot]->_FIf->RunTs_Parser(nSlot, szFile, default_bitrate, 0);
	return	g_LLDWrapper[nSlot]->_FIf->RunTs_Parser(szFile, default_bitrate, 0, ppTsInfo);
//CKIM TEST }
}


extern "C" _declspec(dllexport)	int _stdcall	TSPH_FREE_TS_PARSER_MEMORY(int nSlot)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_FIf->FreeTsParserMemory();
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_FREE_TS_PARSER_MEMORY");

	return 0;
}


extern "C" _declspec(dllexport)	int _stdcall	TSPH_GET_PMT_INFO(int nSlot, int *pNumPgmInfo, struct _TSPH_TS_PGM_INFO **ppPgmInfo)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_FIf->GetPmtInfo(pNumPgmInfo, ppPgmInfo);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPH_GET_PMT_INFO", *pNumPgmInfo, 0);

	return 0;
}


extern "C" _declspec(dllexport)	int _stdcall	TSPH_GET_PID_INFO(int nSlot, int *pNumPidInfo, struct _TSPH_TS_PID_INFO **ppPidInfo)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_FIf->GetPidInfo(pNumPidInfo, ppPidInfo);
	g_LLDWrapper[nSlot]->_HLog->HldPrint_2("App-Call : TSPH_GET_PID_INFO", *pNumPidInfo, 0);

	return 0;
}
//CKIM A 20120805 }

extern "C" _declspec(dllexport) int _stdcall TSPL_GET_AD9787_EX(int nSlot, long reg)
{
	int ret;
	CHECK_DLL_INSTANCE(nSlot);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	ret = g_LLDWrapper[nSlot]->_FIf->TSPL_GET_AD9787(reg);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	return ret;
}

extern "C" _declspec(dllexport) int _stdcall TSPL_SET_AD9787_EX(int nSlot, long reg, long data)
{
	int ret;
	CHECK_DLL_INSTANCE(nSlot);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->LockFileMutex();
#endif
	ret = g_LLDWrapper[nSlot]->_FIf->TSPL_SET_AD9787(reg, data);
#ifndef WIN32
	g_LLDWrapper[nSlot]->_FIf->UnlockFileMutex();
#endif
	return ret;
}

//2013/1/23 Output T2MI Stream
extern "C" _declspec(dllexport) int _stdcall TSPH_SET_T2MI_STREAM_GENERATION(int nSlot, int Output_T2mi, char* ts_path)
{
	CHECK_DLL_INSTANCE(nSlot);
	g_LLDWrapper[nSlot]->_HLog->HldPrint("App-Call : TSPH_SET_T2MI_STREAM_GENERATION");
	g_LLDWrapper[nSlot]->_SysSta->SetT2MI_Stream_Generation(Output_T2mi);
	g_LLDWrapper[nSlot]->_SysSta->SetT2MI_OutputFileName(ts_path);
	return 0;
}
