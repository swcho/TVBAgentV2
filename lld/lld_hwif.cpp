
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
#include	<math.h>
#include	<direct.h>
	
#include	"initguid.h"
#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"

#else
#define _FILE_OFFSET_BITS 64
#include 	<stdio.h>
#include 	<stdlib.h>
#include		<unistd.h>
#include		<pthread.h>
#include		<semaphore.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include 	<memory.h>
#include		<math.h>
#include		<string.h>
#include		<stdarg.h>
#include		<errno.h>
#include <ctype.h>

#include 	"../tsp100.h"
#include	"dll_error.h"
#include 	"../include/logfile.h"
#include 	"wdm_drv.h"
#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif

#include		"Reg590S.h"
#include		"Reg593.h"
#include		"Reg497.h"
#include		"lld_hwif.h"

#include		"wdm_drv_wrapper.h"

#ifdef WIN32
#else
static	pthread_mutex_t	_tlv_mutex_initializer2_ = PTHREAD_MUTEX_INITIALIZER;
static	pthread_mutex_t	_tlv_mutex_initializer_ = PTHREAD_MUTEX_INITIALIZER;
#endif

//////////////////////////////////////////////////////////////////
CLldHwIf::CLldHwIf(void)
{
	m_system_pkg_to_support_multi_rfout = 1;

	parent_id__of_virtual_bd_ = -1;
	my_ts_id = 0;

	cnt_my_sub_ts_vsb = 0;
	cnt_my_sub_ts_qam = 0;
	//2012/6/28 multi dvb-t
	cnt_my_sub_ts_dvbt = 0;

	TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_dont_;
	TSPL_nBoardTypeID = 100;
	TSPL_nAuthorization = 0;
	TSPL_nBoardRevision = 0;//0x03 == TVB390/TVB590, 0x04 == TVB590/express
								//0x04 == TVB595(V1.0), 0x05 == TVB595(V1.1)
								//0x01 == TVB590S
	TSPL_nBoardConfig = 0;
	TSPL_nBoardUseAMP = 0;//TVB590V9, 0=no amp, 1=+19dB gain amp, 2=+24dB gain amp
	TSPL_nTVB595Board = 0; //0=TVB380/390/590, 4=TVB595(V1.0), 5=TVB595(V2.0 or higher)
	TSPL_nFPGA_TEST = 0;//FPGA d/w
	TSPL_nBoardRevision_Ext = 0;//TVB595D, 0x95D1, 0x0598 == TVB590S
	TSPL_nMaxBoardCount = _CNT_MAX_BD_INSTALL_AVAILABLE_;
	TSPL_nMaxPCIBoardCount = _CNT_MAX_PCI_BD_INSTALL_AVAILABLE_;
	TSPL_nBoardLocation = -1;//0;
	usb_BdFound = 0;
	TLV_ControlErrorCode = TLV_NO_FAIL;
	TSPL_nModulatorEnabled = 0;
	TSPL_nModulatorType = TVB380_DVBT_MODE;
	TSPL_nBoardSlotNum = -1;//0;
	TSPL_T2_parity_errCnt = 0;
	//I/Q PLAY/CAPTURE
	TSPL_nFPGA_ID = 0x0390;
	TSPL_nFPGA_VER = 0x80;
	TSPL_nFPGA_BLD = 0;
	TSPL_nFPGA_RBF = 0;
	TSPL_nFPGA_IQ_PLAY = 0;
	TSPL_nFPGA_IQ_CAPTURE = 0;
	TSPL_Dac_I_Offset = 0;
	TSPL_Dac_Q_Offset = 0;
	TSPL_HMC833_status = 0;
	//TVB593
	TSPL_nBankOffset = 1024;
	TSPL_nBankCount = 7;
	TSPL_nMemoryCtrl_mode = 0;
	//TVB497
	TSPL_nBoardOption = 7;//b'110=TVO1626, b'111=TVB1624
	TSPL_nUse10MREFClock = 1;//0=On-board 10MHz, 1=External 10MHz

	board_location = 0;
	//2013/5/23 TVB599
	nClockGeneratorModel = 0;
	nNumOfLED = 0;
	gnHmc833_SerialPortMode = -1;
	gnHmc1033_SerialPortMode = -1;
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#ifdef WIN32
	hDevice = NULL;
	h_DmaMutex = NULL;
	h_EpldMutex = NULL;
#else
#if defined(TVB595V1)
	hDevice_usb = NULL;
	verbose = 1;
#endif
	hDevice = -1;
#endif

//////////////////////////////////////////////////////////////////
	InitMutex();

}
CLldHwIf::~CLldHwIf()
{
	DestroyMutex();
}

void	CLldHwIf::DupBdCnxt(void *_in)
{
	_BD_CONF_CNXT	*in_;

	in_ = (_BD_CONF_CNXT *)_in;

	DupBdCnxt_Step1(_in);
	DupBdCnxt_Step2(_in);
	DupBdCnxt_Step3(_in);
	DupBdCnxt_Step4(_in);
	DupBdCnxt_Step5(_in);

	parent_id__of_virtual_bd_ = in_->__id_my_phy__;
	my_ts_id = in_->__id_ts_seq__;

	TSPL_nBoardLocation = in_->__id__;
	TSPL_nBoardSlotNum = TSPL_nBoardLocation;
}
void	CLldHwIf::DupBdCnxt_Step1(void *_in)
{
	_BD_CONF_CNXT	*in_;

	in_ = (_BD_CONF_CNXT *)_in;
	hDevice = in_->_dev_hnd;
	TSPL_nUseRemappedAddress = in_->_bd_typ_small_mem_space;
#ifdef WIN32
#else
	hDevice_usb = (struct usb_dev_handle *)in_->_dev_hnd_usb;
#endif
	TSPL_nBoardTypeID = _BD_ID_undefined__;

	TSPL_nBoardLocation = in_->__id__;
	TSPL_nBoardSlotNum = TSPL_nBoardLocation;
}
void	CLldHwIf::DupBdCnxt_Step2(void *_in)
{
	_BD_CONF_CNXT	*in_;
	in_ = (_BD_CONF_CNXT *)_in;
	TSPL_nBoardTypeID = in_->_bd_typ_id;
}
void	CLldHwIf::DupBdCnxt_Step3(void *_in)
{
	_BD_CONF_CNXT	*in_;
	in_ = (_BD_CONF_CNXT *)_in;
#ifdef WIN32
	h_DmaMutex = in_->_dma_mutex_hnd;
	h_EpldMutex = in_->_epld_mutex_hnd;
#else
	__mutex_dma = in_->_dma_mutex_hnd;
	__mutex_dma_cnt = in_->_dma_mutex_cnt;
#endif
	dwpDMABufLinearAddr = in_->_dma_buf_page_unmapped;

	TSPL_nAuthorization = in_->_bd_authorization;
	TSPL_nBoardRevision = in_->_bd_revision;
	TSPL_nBoardConfig = in_->_bd_config;
	TSPL_nBoardUseAMP = in_->_bd_use_AMP;
	TSPL_nTVB595Board = in_->_bd_revision_tvb595;
	TSPL_nBoardRevision_Ext = in_->_bd_revision_ext;

	TSPL_nBoardOption = in_->_bd_option;
	TSPL_nUseRemappedAddress = in_->_bd_typ_small_mem_space;
}
void	CLldHwIf::DupBdCnxt_Step4(void *_in)
{
	_BD_CONF_CNXT	*in_;
	in_ = (_BD_CONF_CNXT *)_in;

	TSPL_nModulatorEnabled = in_->_modulator_enabled;
	TSPL_nModulatorType = in_->_modulator_typ_running_now;
	TSPL_nUse10MREFClock = in_->_use_10m_ref_clock;
	gChannel_Freq_Offset = in_->_if_freq_offset;

	TSPL_nFPGA_ID = in_->_fpga_id;
	TSPL_nFPGA_VER = in_->_fpga_ver;
	TSPL_nFPGA_BLD = in_->_fpga_bld;
	TSPL_nFPGA_RBF = in_->_fpga_rbf;
	TSPL_nFPGA_IQ_PLAY = in_->_fpga_iq_play;
	TSPL_nFPGA_IQ_CAPTURE = in_->_fpga_iq_capture;

	cnt_my_sub_ts_vsb = in_->__cnt_my_sub_ts_vsb__;
	cnt_my_sub_ts_qam = in_->__cnt_my_sub_ts_qam__;
	//2012/6/28 multi dvb-t
	cnt_my_sub_ts_dvbt = in_->__cnt_my_sub_ts_dvbt__;
	//2013/5/23 TVB599
	nClockGeneratorModel = in_->_fpga_ClockGeneratorModel;
	nNumOfLED = in_->_fpga_NumOfLED;
	gnHmc833_SerialPortMode = in_->_hmc833_SerialPortMode;
	gnHmc1033_SerialPortMode = in_->_hmc1033_SerialPortMode;
	TSPL_Dac_I_Offset = in_->__dac_i_offset;
	TSPL_Dac_Q_Offset = in_->__dac_q_offset;


}
void	CLldHwIf::DupBdCnxt_Step5(void *_in)
{
	_BD_CONF_CNXT	*in_;
	in_ = (_BD_CONF_CNXT *)_in;

	TSPL_nBankOffset = in_->_fpga_sdram_bnk_offset;
	TSPL_nBankCount = in_->_fpga_sdram_bnk_cnt;
}

void	CLldHwIf::InitMutex(void)
{
#ifdef WIN32
#else
	memcpy(&__mutex, &_tlv_mutex_initializer2_, sizeof(pthread_mutex_t));
	pthread_mutex_init(&__mutex, NULL);
#endif
}
void	CLldHwIf::DestroyMutex(void)
{
#ifdef WIN32
#else
	pthread_mutex_destroy(&__mutex);
	memcpy(&__mutex, &_tlv_mutex_initializer2_, sizeof(pthread_mutex_t));
#endif
}
void	CLldHwIf::LockHwMutex(void)
{
#ifdef WIN32
#else
	pthread_mutex_lock(&__mutex);
#endif
}
void	CLldHwIf::UnlockHwMutex(void)
{
#ifdef WIN32
#else
	pthread_mutex_unlock(&__mutex);
#endif
}
#ifndef WIN32
int	CLldHwIf::OpenDmaMutex(void)
{
	__mutex_dma_cnt = 0;
	memcpy(&__mutex_dma, &_tlv_mutex_initializer_, sizeof(pthread_mutex_t));
	pthread_mutex_init(&__mutex_dma, NULL);
	return	1;
}
void	CLldHwIf::CloseDmaMutex(void)
{
	pthread_mutex_destroy(&__mutex_dma);
	memcpy(&__mutex_dma, &_tlv_mutex_initializer_, sizeof(pthread_mutex_t));
	__mutex_dma_cnt = 0;
}
#endif
void	CLldHwIf::LockDmaMutex(void)
{
#if defined(WIN32)
	if(h_DmaMutex != NULL)
		WaitForSingleObject(h_DmaMutex, INFINITE);
#else
//return;
	//__mutex_dma_cnt++;
	//if (__mutex_dma_cnt > 1)
	//{
	//	LldPrint_Error("LINUX-SEMA-TAKE-reentrance",__mutex_dma_cnt,0);
	//}
	pthread_mutex_lock(&__mutex_dma);
#endif
}
void	CLldHwIf::UnlockDmaMutex(void)
{
#if defined(WIN32)
	if(h_DmaMutex != NULL)
		ReleaseMutex(h_DmaMutex);
#else
//return;
	
	//if (__mutex_dma_cnt <= 0)
	//{
	//	LldPrint_Error("LINUX-SEMA-GIVE-multiple",__mutex_dma_cnt,0);
	//	return;
	//}
	//__mutex_dma_cnt--;
	pthread_mutex_unlock(&__mutex_dma);
#endif
}
void	CLldHwIf::LockEpldMutex(void)
{
#if defined(WIN32)
	if(h_EpldMutex != NULL)
		WaitForSingleObject(h_EpldMutex, INFINITE);
#endif
}
void	CLldHwIf::UnlockEpldMutex(void)
{
#if defined(WIN32)
	if(h_EpldMutex != NULL)
		ReleaseMutex(h_EpldMutex);
#endif
}
void	CLldHwIf::WaitMsec(unsigned long _msec)
{
#if defined(WIN32)
	unsigned long	init_msec, cur_msec, past_msec;

	past_msec = 0;
	init_msec = GetTickCount();
	for (cur_msec = init_msec; ; )
	{
		cur_msec = GetTickCount();
		if (cur_msec < init_msec)
		{
//			past_msec = 0xFFFFFFFF - init_msec;
//			past_msec += cur_msec;	//	approximation. greater than given value.
			past_msec = cur_msec;	//	approximation. greater than given value.
		}
		else
		{
			past_msec = (cur_msec - init_msec);
		}
		if (past_msec >= _msec)
		{
			break;
		}
	}
#else
	Sleep(_msec);
#endif
}

unsigned long CLldHwIf::IoctlRdTvbPciConfReg(unsigned long address)
{
	unsigned long	_ret;
	KCMD_ARGS		KCmdInf;

	KCmdInf.dwCmdParm2 = 0xc8;
	LockDmaMutex();
#ifdef WIN32
	KCmdInf.dwCmdParm1 = address;
	TLV_DeviceIoControl(hDevice, IOCTL_READ_TVBPCI_CONF_REG, &KCmdInf, sizeof(KCMD_ARGS), &KCmdInf, sizeof(KCMD_ARGS), &_ret, 0);
//	fprintf(fn_Ctl, "...[fffff] : [0x%x], [0x%x]\n", _ret, GetLastError());
#endif
	UnlockDmaMutex();
	return	KCmdInf.dwCmdParm2;
}
void CLldHwIf::IoctlWrTvbPciConfReg(unsigned long address, unsigned long val)
{
	unsigned long	_ret;
	KCMD_ARGS		KCmdInf;
	LockDmaMutex();

#ifdef WIN32
	KCmdInf.dwCmdParm1 = address;
	KCmdInf.dwCmdParm2 = val;
	TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TVBPCI_CONF_REG, &KCmdInf, sizeof(KCMD_ARGS), NULL, 0, &_ret, 0);
//	fprintf(fn_Ctl, "...[fffff] : [0x%x], [0x%x]\n", _ret, GetLastError());
#endif
	UnlockDmaMutex();
}
unsigned long CLldHwIf::RemapAddress(unsigned long address)
{
	unsigned long dwNewAddress = address;

//	LldPrint_Trace("RemapAddress", address, TSPL_nUseRemappedAddress, 0, NULL);

	unsigned long tmp = 0;

	//TVB590S
	int ii, kk;

	//TVB597A
	if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_597_ )
	{
		tmp = ((address>>12) & 0xF); 
		if ( tmp >= 1 && tmp <= 3 )
		{
			dwNewAddress = ((address>>20 & 0xF) << 16) + (address & 0xFFFF);
		}
		else
		{
			dwNewAddress = ((address>>16 & 0xFF) << 12) + (address & 0xFFF);
		}
	}
	//TVB590S
	else if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_590s_ )
	{
		DO_PCI590S_ADDRESS_NON_MAPPING(ii,kk,dwNewAddress,dwNewAddress)
	}
	//TVB593
	else if ( TSPL_nUseRemappedAddress == _ADDR_REMAP_METHOD_593_ )
	{
		DO_PCI593_ADDRESS_NON_MAPPING(ii,kk,dwNewAddress,dwNewAddress);
	}

	return dwNewAddress;
}

unsigned long	CLldHwIf::WDM_Read_ModCmd_Indirectly(unsigned long cmd_adr)
{
	KCMD_ARGS	KCmdInf;
	unsigned long 		dwRet;

#if defined(WIN32)
#else
	return	0;
#endif
	if (!CHK_DEV(hDevice))
	{
		return	0;
	}
	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_FPGA_WRITE_TEST;
	KCmdInf.dwCmdParm2 = cmd_adr;
	LockDmaMutex();

	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
	Sleep(1);
	TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);

	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_READ_PARAMS;
	KCmdInf.dwCmdParm2 = 0;
	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
	TLV_DeviceIoControl(hDevice, IOCTL_READ_FROM_MEMORY, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	UnlockDmaMutex();

	return (unsigned long)KCmdInf.dwCmdParm2;
}

//2013/5/23 TVB599
int CLldHwIf::Get_Board_HW_Feature(int num)
{
	unsigned long nVal;
	int nRet = 0;
	nVal = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);

	switch(num)
	{
	case 0: //AD9852 REF
		nRet = (nVal & 0x3);
	case 1:	//AD9852 Model
		nRet = (nVal >> 2) & 0x3;
		break;
	case 2:	//USB RESET SUPPORT
		nRet = (nVal >> 4) & 0x1;
		break;
	case 3:	//Fast download Enable
		nRet = (nVal >> 5) & 0x1;
		break;
	case 4:	//Num of LED
		if(IsAttachedBdTyp_599())
			nRet = (nVal >> 6) & 0x3;
		else if(IsAttachedBdTyp_598())
			nRet = (nVal >> 7) & 0x3;
		break;
	case 5:	//System Clock
		if(IsAttachedBdTyp_599())
			nRet = (nVal >> 8) & 0x1; 
		break;
	case 6:	//Ready Control
		nRet = (nVal >> 9) & 0x1; 
		break;
	}

	return nRet;
}


//TVB593
/*^^***************************************************************************
 * Description : Access(Read) F/W of TVB593
 *				
 * Entry : dwMemAddr
 *
 * Return: value of the address, 0(fail)
 *
 * Notes :  
 *
 **************************************************************************^^*/
unsigned long	CLldHwIf::WDM_Read_TSP_Indirectly(unsigned long dwMemAddr)
{
	KCMD_ARGS	KCmdInf;
	unsigned long 		dwRet;
	KCmdInf.dwCmdParm1 = dwMemAddr;
	KCmdInf.dwCmdParm2 = 0;
	LockDmaMutex();

//	LldPrint_Trace("WDM_Read_TSP_Indirectly", dwMemAddr, 0, 0, NULL);
	if ( CHK_DEV(hDevice) )
	{
		KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_FPGA_WRITE_TEST;
		KCmdInf.dwCmdParm2 = dwMemAddr;
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		KCmdInf.dwCmdParm2 = RemapAddress(KCmdInf.dwCmdParm2);
		TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);
		Sleep(1);

		KCmdInf.dwCmdParm1 = RemapAddress(PCI593_REG_IQ_PLAY_CAPTURE_FPGA_READ);
		KCmdInf.dwCmdParm2 = 0;
		TLV_DeviceIoControl(hDevice, IOCTL_READ_FROM_MEMORY, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	}
#if defined(WIN32)
#else
	else if (CHK_DEV(hDevice_usb))
	{
		KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_FPGA_WRITE_TEST;
		KCmdInf.dwCmdParm2 = dwMemAddr;
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		KCmdInf.dwCmdParm2 = RemapAddress(KCmdInf.dwCmdParm2);
		TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);
		Sleep(1);

		KCmdInf.dwCmdParm1 = RemapAddress(PCI593_REG_IQ_PLAY_CAPTURE_FPGA_READ);
		KCmdInf.dwCmdParm2 = 0;
		TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_READ_FROM_MEMORY, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	}
#endif
	LldPrint_Trace("WDM_Read_TSP_Indirectly", dwMemAddr, KCmdInf.dwCmdParm2, 0, NULL);
	UnlockDmaMutex();
	return (unsigned long)KCmdInf.dwCmdParm2;
}

/*^^***************************************************************************
 * Description : Access(Write) F/W
 *				
 * Entry : dwAddress, dwCommand
 *
 * Return: 0(fail), 1(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CLldHwIf::WDM_WR2SDRAM_ADDR(unsigned long dwAddress, unsigned long dwCommand)
{
	KCMD_ARGS		KCmdInf;
	unsigned long 			dwRet;

	LldPrint_Trace("WDM_WR2SDRAM_ADDR", dwAddress, dwCommand, 0, NULL);

	KCmdInf.dwCmdParm1 = dwAddress;
	KCmdInf.dwCmdParm2 = dwCommand;
	
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
	{
		LockDmaMutex();
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

		TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);
		UnlockDmaMutex();
		return 1;
	}
	else
		return 0;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
		{
			LockDmaMutex();
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);
			UnlockDmaMutex();
			return 1;
		}
	}
	else
	{
		if ( CHK_DEV(hDevice) )
		{
			LockDmaMutex();
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);

			UnlockDmaMutex();
			return 1;
		}
	}
	return 0;
#endif
}

/*^^***************************************************************************
 * Description : Access(Write) F/W, TSP_MEM_ADDR_COMMAND(0x400.000)
 *				
 * Entry : dwCommand
 *
 * Return: 0(fail), 1(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	CLldHwIf::WDM_WR2SDRAM(unsigned long dwCommand)
{
	KCMD_ARGS		KCmdInf;
	unsigned long 			dwRet;

	LldPrint_Trace("WDM_WR2SDRAM", dwCommand, 0, 0, NULL);
	KCmdInf.dwCmdParm1 = TSP_MEM_ADDR_COMMAND;
	KCmdInf.dwCmdParm2 = dwCommand;

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
	{
		LockDmaMutex();
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);
		UnlockDmaMutex();
		return 1;
	}
	else
		return 0;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
		{
			LockDmaMutex();
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);

			UnlockDmaMutex();
			return 1;
		}
	}
	else
	{
		if ( CHK_DEV(hDevice) )
		{
			LockDmaMutex();
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_TO_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   NULL, 0, &dwRet,0);

			UnlockDmaMutex();
			return 1;
		}
	}
	return 0;
#endif
}

/*^^***************************************************************************
 * Description : Access(Read) F/W
 *				
 * Entry : dwMemAddr
 *
 * Return: value of the address, 0(fail)
 *
 * Notes :  
 *
 **************************************************************************^^*/
unsigned long	CLldHwIf::WDM_Read_TSP(unsigned long dwMemAddr)
{
	KCMD_ARGS	KCmdInf;
	unsigned long 		dwRet;
	KCmdInf.dwCmdParm1 = dwMemAddr;
	KCmdInf.dwCmdParm2 = 0;

	LldPrint_Trace("WDM_Read_TSP", dwMemAddr, 0, 0, NULL);
//2010/10/1 PCI/USB MULTIBOARD
	LockDmaMutex();
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
	{
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_FROM_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   &KCmdInf,
				   sizeof(KCmdInf),	
				   &dwRet, 0);
	}
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
		{
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_READ_FROM_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   &KCmdInf,
				   sizeof(KCmdInf),	
				   &dwRet, 0);
		}
	}
	else
	{
		if ( CHK_DEV(hDevice) )
		{
			KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

			TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_FROM_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   &KCmdInf,
				   sizeof(KCmdInf),	
				   &dwRet, 0);
		}
	}
#endif
UnlockDmaMutex();
	return (unsigned long)KCmdInf.dwCmdParm2;
}
//2010/9/30 PCI/USB MULTIBOARD
#ifdef WIN32
#else
unsigned long	CLldHwIf::WDM_Read_TSP_USB(unsigned long dwMemAddr)
{
	KCMD_ARGS	KCmdInf;
	unsigned long 		dwRet;
	LockDmaMutex();
	
	KCmdInf.dwCmdParm1 = dwMemAddr;
	KCmdInf.dwCmdParm2 = 0;

	LldPrint_Trace("WDM_Read_TSP_USB", dwMemAddr, 0, 0, NULL);
	if ( CHK_DEV(hDevice_usb) )
	{
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

		TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_READ_FROM_MEMORY,
				   &KCmdInf,
				   sizeof(KCmdInf),
				   &KCmdInf,
				   sizeof(KCmdInf),	
				   &dwRet, 0);
	}
	
	UnlockDmaMutex();
	return (unsigned long)KCmdInf.dwCmdParm2;
}
#endif

/*^^***************************************************************************
 * Description : Access(Read) F/W, TSP_MEM_ADDR_STATUS(0x600.000)
 *				
 * Entry : 
 *
 * Return: value of the address
 *
 * Notes :  
 *
 **************************************************************************^^*/
unsigned long	CLldHwIf::WDM_ReadFromSDRAM(void)
{
	LldPrint_Trace("WDM_ReadFromSDRAM", 0, 0, 0, NULL);
	return WDM_Read_TSP(TSP_MEM_ADDR_STATUS);
}

/*^^***************************************************************************
 * Description : Access(Write) F/W
 *				
 * Entry : dwAddress, dwData
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int 	CLldHwIf::TSPL_SET_CONFIG_DOWNLOAD(long dwAddress, unsigned long dwData)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;
	int			iRet = 0;

//	DBG_PLATFORM_BRANCH();
	LldPrint_Trace("TSPL_SET_CONFIG_DOWNLOAD", dwAddress, dwData, (double)0, NULL);
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
		return TLV_NO_DEVICE;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
			return TLV_NO_DEVICE;
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
			return TLV_NO_DEVICE;
	}
#endif
	LockDmaMutex();
	KCmdInf.dwCmdParm1 = dwAddress;
	KCmdInf.dwCmdParm2 = (DWORD) (dwData);
	KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	iRet = TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY, &KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet, 0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		iRet = TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_WRITE_TO_MEMORY, &KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet, 0);
	}
	else
	{
		iRet = TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY, &KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet, 0);
	}

#endif
	UnlockDmaMutex();
#ifdef WIN32
	return (iRet ? 0:-1);
#else
	return 0;
#endif
}

/*^^***************************************************************************
 * Description : Read the current modulator parameters
 *				
 * Entry : 
 *
 * Return:
 *
 * Notes :  
 *
 **************************************************************************^^*/
int CLldHwIf::TSPL_GET_READ_PARAM(void)
{
	LldPrint_Trace("TSPL_GET_READ_PARAM", 0, 0, (double)0, NULL);
	return WDM_Read_TSP(TSP_MEM_ADDR_READ_PARAMS);
}




/*^^***************************************************************************
 * Description : Get the board revision
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_GET_BOARD_REV(void)
{
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_GET_BOARD_REV", 0, 0);
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
		return TSPL_nBoardRevision;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
			return TSPL_nBoardRevision;
	}
	else
	{
		if ( CHK_DEV(hDevice) )
			return TSPL_nBoardRevision;
	}
#endif
	return -1;
}

/*^^***************************************************************************
 * Description : Get the board Id
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_GET_BOARD_ID(void)
{
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_GET_BOARD_ID", 0, 0);
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
		return TSPL_nBoardTypeID;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
			return TSPL_nBoardTypeID;
	}
	else
	{
		if ( CHK_DEV(hDevice) )
			return TSPL_nBoardTypeID;
	}
#endif
	return -1;
}

/*^^***************************************************************************
 * Description : Get the enabled modulator type information
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_GET_AUTHORIZATION(void)
{
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_GET_AUTHORIZATION", 0, 0);
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
		return TSPL_nAuthorization;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
			return TSPL_nAuthorization;
	}
	else
	{
		if ( CHK_DEV(hDevice) )
			return TSPL_nAuthorization;
	}
#endif
	return -1;
}

/*^^***************************************************************************
 * Description : Get the installed board's FPGA ID
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :
 *
 **************************************************************************^^*/
//I/Q PLAY/CAPTURE
int _stdcall CLldHwIf::TSPL_GET_FPGA_INFO(int info)
{
	DWORD nRet=0;
	//2012/4/12 SINGLE TONE
	int singleTone;
	//2012/9/11	PCR RESTAMP
	int pcrRestamp;
	int rbf_revision_cntl;
	int ts_packet_cntl;
	int fpga_id;
	int major_version_num;
	int build_num;
	//2013/10/1 T2 parity error 
	int evenParity;
	LldPrint_Trace("TSPL_GET_FPGA_INFO", info, 0, (double)0, NULL);
	if ( CHK_DEV(hDevice) )
	{
		nRet = WDM_Read_TSP(TSP_MEM_ADDR_Chip_STATUS_REG);
		TSPL_nFPGA_ID = ((nRet>>16)&0xFFFF);
		TSPL_nFPGA_VER = ((nRet>>8)&0xFF);

		nRet = WDM_Read_TSP(TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_CAP);
		TSPL_nFPGA_IQ_PLAY = ((nRet>>1) & 0x01);
		TSPL_nFPGA_IQ_CAPTURE = (nRet & 0x01);
		//2012/4/12 SINGLE TONE
		singleTone = ((nRet>>3) & 0x01);	
		//2012/9/11	PCR RESTAMP
		pcrRestamp = ((nRet>>4) & 0x01);

		rbf_revision_cntl = ((nRet>>7) & 0x01);
		ts_packet_cntl = ((nRet>>6) & 0x01);
		//2013/10/1 T2 even parity
		evenParity = ((nRet>>16) & 0x01);

		TSPL_T2_x7_Mod_Clock = ((nRet>>17) & 0x1);

//		LldPrint_1("[LLD] capability T2_x7_Mod_Clock::::::", TSPL_T2_x7_Mod_Clock);
	
		nRet = WDM_Read_TSP(TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_BLD);
		if(rbf_revision_cntl == 1)
		{
			fpga_id = ((nRet >> 16) & 0xFFFF);
			major_version_num = ((nRet >> 12) & 0xF);
			build_num = (nRet & 0xFFF); 
		}
		else
		{
			TSPL_nFPGA_BLD = nRet & 0xFF;
			TSPL_nFPGA_RBF = (nRet >> 24) & 0xFF;
			fpga_id = TSPL_nFPGA_ID;
			build_num = TSPL_nFPGA_BLD;
			major_version_num = TSPL_nFPGA_RBF;
		}

		if ( info == 0 )	//fpga id
			return fpga_id;
		else if ( info == 1 )
			return TSPL_nFPGA_VER;
		else if ( info == 2 )
			return TSPL_nFPGA_IQ_PLAY;
		else if ( info == 3 )
			return TSPL_nFPGA_IQ_CAPTURE;
		else if ( info == 4 )	//major version number
			return major_version_num;
		else if ( info == 5 )	//build number
			return build_num;
		//2012/4/12 SINGLE TONE
		else if ( info == 6)
			return singleTone;
		//2012/9/11 PCR RESTAMP
		else if ( info == 7)
			return pcrRestamp;
		else if ( info == 8)
			return ts_packet_cntl;
		else if ( info == 9)
			return rbf_revision_cntl;
		else if (info == 12)
			return evenParity;
#if 1
		//2013/3/25 DAC I OFFSET
		else if ( info == 10)
			return TSPL_Dac_I_Offset;
		else if ( info == 11)
			return TSPL_Dac_Q_Offset;
#endif
	}
	return -1;
}

/*^^***************************************************************************
 * Description : Get the encrypted serial number of Chip
 *				
 * Entry : high(1=hgih 32 bits, 0=low 32 bits)
 *
 * Return: 
 *
 * Notes : 
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_GET_ENCRYPTED_SN(int type, char* sn)
{
	// S/N
	LldPrint_Trace("TSPL_GET_ENCRYPTED_SN", type, 0, (double)0, NULL);
	if ( type == 0 )
		strcpy(sn, (char*)&TSPL_EncryptedSN[0]);

	// L/N
	else
		strcpy(sn, (char*)&TSPL_EncryptedLN[0]);

	return 0;
}

/*^^***************************************************************************
 * Description : 
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes : 
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_WRITE_CONTROL_REG(int Is_PCI_Control, unsigned long address, unsigned long dwData)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;

	LldPrint_Trace("TSPL_WRITE_CONTROL_REG", Is_PCI_Control, address, (double)dwData, NULL);
//#ifdef WIN32
	//LockDmaMutex();
	//if ( fDMA_Busy == 1 )
	//{
	//	UnlockDmaMutex();
	//	return 0;
	//}
//#endif

	if( Is_PCI_Control == 3)
	{
		LockDmaMutex();
		KRegInf.dwCmdParm1 = address & 0xFFFF;	
		KRegInf.dwCmdParm2 = dwData & 0xFFFF;
#ifndef WIN32
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_WRITE_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
		}
		else
		{
#endif
			TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				   &KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
		}
#endif
		UnlockDmaMutex();
		return 1;
	}

	KRegInf.dwCmdParm1 = address;	
	KRegInf.dwCmdParm2 = dwData;

#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
	//TVB590S - AD9775 access
	if ( Is_PCI_Control == 2 )
	{
		//TVB590S, TVB593, TVB497
		if ( IsAttachedBdTyp_UseAD9775() )
		{

			if ( address == 0xFF )
			{
				___TSPL_SET_AD9775(-1, dwData);
			}
			else
			{
				___TSPL_SET_AD9775(address, dwData);
			}
		}
	}
	else
#endif

	if ( Is_PCI_Control )
	{
		LockDmaMutex();
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
		TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_PCI9054_REG,
				   &KRegInf,
				   sizeof(KRegInf),
				   NULL,
				   0,	
				   &dwRet,0);
#else
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_WRITE_PCI9054_REG,
				   &KRegInf,
				   sizeof(KRegInf),
				   NULL,
				   0,	
				   &dwRet,0);
		}
		else
		{
			TLV_DeviceIoControl(hDevice,
				   IOCTL_WRITE_PCI9054_REG,
				   &KRegInf,
				   sizeof(KRegInf),
				   NULL,
				   0,	
				   &dwRet,0);
		}
#endif
		UnlockDmaMutex();
	}
	else
	{
		WDM_WR2SDRAM_ADDR(address, dwData);
	}
//#ifdef WIN32
	//UnlockDmaMutex();
//#endif
	return 1;
}

unsigned long _stdcall CLldHwIf::TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;
	unsigned long			dwStatus;

	LldPrint_Trace("TSPL_READ_CONTROL_REG", Is_PCI_Control, address, (double)0, NULL);
//#ifdef WIN32
	//LockDmaMutex();
	//if ( fDMA_Busy == 1 )
	//{
	//	UnlockDmaMutex();
	//	return 0;
	//}
//#endif
	KRegInf.dwCmdParm1 = address;	// 0~0x3f. from PCI-CNTRL 0x6c

	if( Is_PCI_Control == 3)
	{
		LockDmaMutex();
#ifndef WIN32
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			TLV_DeviceIoControl_usb(hDevice_usb,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				   &KRegInf,
				   sizeof(KRegInf),	
				   &dwRet,0);
		}
		else
		{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				   &KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
		}
#endif
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);

		UnlockDmaMutex();
		return dwStatus;
	}


#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
	//TVB590S - AD9775 access
	if ( Is_PCI_Control == 2 )
	{
		//TVB590S, TVB593,TVB497
		if ( IsAttachedBdTyp_UseAD9775() )
		{

			dwStatus = ___TSPL_GET_AD9775(address);
		}
	}
	else
#endif

	if ( Is_PCI_Control )
	{
		LockDmaMutex();
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
		TLV_DeviceIoControl(hDevice,
				IOCTL_READ_PCI9054_REG,
				&KRegInf,
				sizeof(KRegInf),
				&KRegInf,
				sizeof(KRegInf),	
				&dwRet, 0);
#else
		if ( IsAttachedBdTyp_UsbTyp() )
		{
			TLV_DeviceIoControl_usb(hDevice_usb,
				IOCTL_READ_PCI9054_REG,
				&KRegInf,
				sizeof(KRegInf),
				&KRegInf,
				sizeof(KRegInf),	
				&dwRet, 0);
		}
		else
		{
			TLV_DeviceIoControl(hDevice,
				IOCTL_READ_PCI9054_REG,
				&KRegInf,
				sizeof(KRegInf),
				&KRegInf,
				sizeof(KRegInf),	
				&dwRet, 0);
		}
#endif
		UnlockDmaMutex();
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);
	}
	else
	{
		dwStatus = WDM_Read_TSP(address);
	}
//#ifdef WIN32
	//UnlockDmaMutex();
//#endif

	return dwStatus;
}

int _stdcall CLldHwIf::TSPL_GET_LAST_ERROR(void)
{
//	LldPrint_Trace("TSPL_READ_CONTROL_REG", Is_PCI_Control, address, (double)0, NULL);
	return TLV_ControlErrorCode;
}

//TVB590V9
/*^^***************************************************************************
 * Description : Get the installed board's status - RF/IF AMP is used(1 or 2) or not(0)
 *				
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB590, TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_GET_BOARD_CONFIG_STATUS(void)
{
//2010/10/1 PCI/USB MULTIBOARD
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_GET_BOARD_CONFIG_STATUS", 0, 0);
#ifdef WIN32
	if ( CHK_DEV(hDevice) )
		return TSPL_nBoardUseAMP;
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( CHK_DEV(hDevice_usb) )
			return TSPL_nBoardUseAMP;
	}
	else
	{
		if ( CHK_DEV(hDevice) )
			return TSPL_nBoardUseAMP;
	}
#endif
	return 0;
}

#if defined(WIN32) || defined(TVB595V1)
/*^^***************************************************************************
 * Description : Set the installed board's LED status 
 *				status_LED==0(LED off), 1==(LED on), fault_LED==0(LED off), 1(LED on) 
  * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_SET_BOARD_LED_STATUS(int status_LED, int fault_LED)
{
	int	_ret;

	LldPrint_Trace("TSPL_SET_BOARD_LED_STATUS", status_LED, fault_LED, (double)0, NULL);

	if(((IsAttachedBdTyp_599() || IsAttachedBdTyp_598()) && nNumOfLED == 1) || ((IsAttachedBdTyp_598() || IsAttachedBdTyp_599()) && IsAttachedBdTyp__Virtual()))
	{
		int data_;
		switch(status_LED)
		{
		case 1:	//App_SW_Ready 0x40200 bit3
			data_ = (fault_LED & 0x1) << 3;
			break;
		case 2:	//Host_LED_Ctrl_On	0x40200 bit4
			data_ = ((fault_LED & 0x1) << 4) + 8;
			break;
		case 3: //Host_LED_Ctrl
			data_ = ((fault_LED & 0x3) + (0x3 << 3) + (0x2 << 5));//((fault_LED & 0x3) << 5) + (0x3 << 3);
			break;
		case 4: //Fault, Status LED control, disable Host control
			data_ = ((fault_LED & 0x3) + (0x1 << 3) + (0x2 << 5));
			break;
		}
		
		_ret = WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, data_);
		return _ret;
	}

	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		_ret = TSPL_WRITE_CONTROL_REG(0, PCI593_REG_LED_CONTROL, status_LED + (fault_LED<<1));
		return	_ret;
	}

	if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
		TSPL_WRITE_CONTROL_REG(0, TSP_MEM_ADDR_COMMAND, 0xA0 + status_LED + (fault_LED<<1));

	_ret = TSPL_WRITE_CONTROL_REG(0, TSP_BOARD_LED_STATUS, status_LED + (fault_LED<<1));
	return	_ret;
}

/*^^***************************************************************************
 * Description :
 *
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_REG_EVENT(void* pvoid)
{
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_REG_EVENT", 0, 0);
#ifdef WIN32
	unsigned long junk;
	HANDLE* pEvent = (HANDLE*)pvoid;
	LockDmaMutex();

//	*pEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!TLV_DeviceIoControl(hDevice, 
		IOCTL_REGISTER_EVENT, 
		pEvent, 
		sizeof(*pEvent), 
		NULL, 
		0, 
		&junk, 
		NULL))
	{
		LldPrint_1("[LLD]Error in IOCTL_REGISTER_EVENT call", GetLastError());
//		CloseHandle(*pEvent);
	}
	UnlockDmaMutex();
#endif
	return 0;
}

/*^^***************************************************************************
 * Description :
 *
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_SET_EVENT(void)
{
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_SET_EVENT", 0, 0);
#ifdef WIN32
	unsigned long junk;

	LockDmaMutex();
	if (!TLV_DeviceIoControl(hDevice,
		IOCTL_SIGNAL_EVENT, 
		NULL, 
		0, 
		NULL, 
		0, 
		&junk, 
		NULL))
		LldPrint_1("[LLD]Error in IOCTL_SIGNAL_EVENT call", GetLastError());
	UnlockDmaMutex();
#endif
	return 0;
}

/*^^***************************************************************************
 * Description :
 *
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_REG_COMPLETE_EVENT(void* pvoid)
{
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_REG_COMPLETE_EVENT", 0, 0);
#ifdef WIN32
	unsigned long junk;
	HANDLE* pEvent = (HANDLE*)pvoid;

	LockDmaMutex();
//	*pEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!TLV_DeviceIoControl(hDevice, 
		IOCTL_REGISTER_COMPLETE_EVENT, 
		pEvent, 
		sizeof(*pEvent), 
		NULL, 
		0, 
		&junk, 
		NULL))
	{
		LldPrint_1("[LLD]Error in IOCTL_REGISTER_COMPLETE_EVENT call", GetLastError());
//		CloseHandle(*pEvent);
	}
	UnlockDmaMutex();
#endif
	return 0;
}

/*^^***************************************************************************
 * Description :
 *
 * Entry : 
 *
 * Return: 
 *
 * Notes :  only for TVB595
 *
 **************************************************************************^^*/
int _stdcall CLldHwIf::TSPL_SET_COMPLETE_EVENT(void)
{
	DBG_PLATFORM_BRANCH();
	LldPrint_FCall("TSPL_SET_COMPLETE_EVENT", 0, 0);
#ifdef WIN32
	unsigned long junk;
	LockDmaMutex();

	if (!TLV_DeviceIoControl(hDevice,
		IOCTL_SIGNAL_COMPLETE_EVENT, 
		NULL, 
		0, 
		NULL, 
		0, 
		&junk, 
		NULL))
		LldPrint_1("[LLD]Error in IOCTL_SIGNAL_COMPLETE_EVENT call", GetLastError());
	UnlockDmaMutex();
#endif
	return 0;
}
#endif

//=================================================================	
#ifdef WIN32
void CLldHwIf::usleep(long usec)
{ 
#if	1
	LARGE_INTEGER lFrequency; 
	LARGE_INTEGER lEndTime; 
	LARGE_INTEGER lCurTime; 

	QueryPerformanceFrequency (&lFrequency); 
	if (lFrequency.QuadPart) 
	{ 
		QueryPerformanceCounter (&lEndTime); 
		lEndTime.QuadPart += (LONGLONG) usec * lFrequency.QuadPart / 1000000; 
		do 
		{ 
			QueryPerformanceCounter (&lCurTime); 
			Sleep(0); 
		} while (lCurTime.QuadPart < lEndTime.QuadPart); 
	} 
#else
	if (usec >= 1000)
	{
		SleepEx(usec/1000,FALSE);
	}
	else
	{
		SleepEx(1,FALSE);
	}
#endif
} 
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
//PCIe - PEX8111
#include	"PlxApi.h"
void CLldHwIf::ConfigurePEX8111(void)
{
    unsigned char	DeviceNum;
    unsigned short	ChipType;
    unsigned char	Revision;
    unsigned long	rc;
    unsigned short	Offset;
    unsigned long	Value, Offset32;
    PLX_DEVICE_KEY	Key;
    PLX_DEVICE_OBJECT	Device;
	
	LldPrint_Trace("ConfigurePEX8111", 0, 0, (double)0, NULL);
    // Find PEX811 devices
    DeviceNum = 0;
    do
    {
        // Set key structure to ignore all fields
        memset( &Key, PCI_FIELD_IGNORE, sizeof(PLX_DEVICE_KEY) );

        // Search for next device
        rc = PlxPci_DeviceFind(
                &Key,
                DeviceNum
                );

        // Check if all devices have been found
        if (rc != ApiSuccess)
        {
            break;
        }

        // Check PEX8111's device ID/vendor ID
		if ( Key.DeviceId == 0x8111 && Key.VendorId == 0x10B5 )
		{
			// Select device
			rc = PlxPci_DeviceOpen(
					&Key,
					&Device
					);

			if (rc != ApiSuccess)
			{
				continue;
			}

    
			/*********************************************
			*           Call an API function
			********************************************/
			rc = PlxPci_ChipTypeGet(
					&Device,
					&ChipType,
					&Revision
					);

			if (rc != ApiSuccess)
			{
			}
			else
			{
			}

			//{0x00c, "BIST | Header Type | Prim Latency Timer | Cacheline Size"},
			Offset = 0x00c;
			Value =	PlxPci_PciRegisterReadFast(&Device, Offset, NULL);
			//16DWords
			Value = 0x10;
			PlxPci_PciRegisterWriteFast(&Device, Offset, Value);

			//{0x048, "Device-specific Control"},
			Offset = 0x048;
			Value =	PlxPci_PciRegisterReadFast(&Device, Offset, NULL);
			//Blind Prefetch Enabled
			Value |= 0x01;
			PlxPci_PciRegisterWriteFast(&Device, Offset, Value);

			//{0x068, "PCIe Cap: Device Status | Device Control"},
			Offset = 0x068;
			Value =	PlxPci_PciRegisterReadFast(&Device, Offset, NULL);
			//Maximum Read Request Size = 2048 bytes
			Value = (((Value>>15) & 0x01) << 15) + (0x04 << 12) + (Value & 0x7FF);
			PlxPci_PciRegisterWriteFast(&Device, Offset, Value);

			//{0x100c, "PCI Control"},
			Offset32 = 0x100c;
			Value = PlxPci_PlxRegisterRead(&Device, Offset32, (enum _PLX_STATUS *)&rc);
			//Programmed Prefetch = 2048 bytes
			Value = (((Value>>30) & 0x03) << 30) + (0x06 << 27) + (Value & 0x3FFFFFF);
			PlxPci_PlxRegisterWrite(&Device, Offset32, Value);
	

			/*********************************************
			*              Close Device
			********************************************/
			rc = PlxPci_DeviceClose(
					&Device
					);

			if (rc != ApiSuccess)
			{
			}
			else
			{
			}
		}
        
        // Increment to next device
        DeviceNum++;
    }
    while (1);
}
#endif

/*^^***************************************************************************
 * Description : Check valid board ID
 *				
 * Entry : device handle
 *
 * Return: 
 *
 * Notes :  TSPL_nBoardTypeID == 
 *			TVB380V4==0x29, TVB390V6(No demand mode)==0x2A,
 *			TVB370V6(VSB Only, No demand mode)==0x2B
 *			TVB390V7(No demand mode, Chip)==0x2C, TVB390V8,TVB590==0x2D
 *			TVB370V7(IF Only, No demand mode, DS2410)==0x2E
 *			TVB595V1,2==0x3B
 *			TVB590V9.2==0x2F
 *			TVB590V10.0==0x30
 *			TVB595V3==0x3C
 *
 **************************************************************************^^*/
//	extern int	TSPL_nBoardTypeID;
int CLldHwIf::CHK_ID( int id, ... )
{
	int i = id;
	va_list marker;

//	LldPrint_Trace("CHK_ID", 0, 0, (double)0, NULL);
	va_start( marker, id );     /* Initialize variable arguments. */
	//sskim20080229
	//while( i != -1 )
	while(i >= 0)
	{
		i = va_arg( marker, int);
		//fixed - ubuntu
		if ( i < _BD_ID_597__ || i > 0xFF )
		{
			break;
		}

		if ( TSPL_nBoardTypeID == i )
		{
			va_end( marker );
			return 1;
		}
	}
	va_end( marker );              /* Reset variable arguments.      */

	return 0;
}
void	_stdcall CLldHwIf::ResetSdram(void)
{
	KCMD_ARGS	KCmdInf;
	unsigned long		dwRet;

	if (!IsAttachedBdTyp_NewAddrMap() && !IsAttachedBdTyp_590s_SmallAddrSpace())
	{
		return;
	}

	LldPrint_FCall("ResetSdram", (int)hDevice, 0);
	if ( CHK_DEV(hDevice) )
	{
		LockDmaMutex();
		KCmdInf.dwCmdParm1 = _ADR_597v2_FPGA_RESET_CTRL;
		KCmdInf.dwCmdParm2 = (0x1 << 6);
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);

		KCmdInf.dwCmdParm1 = _ADR_597v2_FPGA_RESET_CTRL;
		KCmdInf.dwCmdParm2 = 0;
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		TLV_DeviceIoControl(hDevice, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);
		UnlockDmaMutex();
	}
#ifdef	WIN32
#else
	else if (CHK_DEV(hDevice_usb))
	{
		LockDmaMutex();
		KCmdInf.dwCmdParm1 = _ADR_597v2_FPGA_RESET_CTRL;
		KCmdInf.dwCmdParm2 = (0x1 << 6);
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);

		KCmdInf.dwCmdParm1 = _ADR_597v2_FPGA_RESET_CTRL;
		KCmdInf.dwCmdParm2 = 0;
		KCmdInf.dwCmdParm1 = RemapAddress(KCmdInf.dwCmdParm1);
		TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_WRITE_TO_MEMORY,&KCmdInf, sizeof(KCmdInf), NULL, 0, &dwRet,0);
		UnlockDmaMutex();
	}
#endif
}
void	CLldHwIf::MakeRbfName(char *_fn, int _mod_typ)
{
#ifdef	_DBG_ON_TARGET_TH4435__
	switch(TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB599A_DVBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB599A_ATSC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB599A_DVBC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB599A_J83B.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"rbf/TVB599A_DVBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB599A_TDMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB16_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB599A_DVBH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBS2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB599A_ISDBT1SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_13_MODE:		sprintf(_fn,"rbf/TVB599A_ISDBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB599A_DTMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_CMMB_MODE:			sprintf(_fn,"rbf/TVB599A_CMMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBT2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBT2.rbf", (int)('A'+_mod_typ));	break;	//			sprintf(_fn,"rbf/TVB597_%c_DVBT2_SinglePLP.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBC2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBC2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_RESERVED_O_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ATSC_MH_MODE:		sprintf(_fn,"rbf/TVB599A_ATSCMH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_IQ_PLAY_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"rbf/TVB599A_ISDBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TSIO_MODE:			sprintf(_fn,"rbf/TVB599A_TSIO.rbf");	break;
		default:
			TSPL_nModulatorType = TVB380_TSIO_MODE;
			sprintf(_fn,"rbf/TVB599A_TSIO.rbf");	break;
		}
		break;
	case	_BD_ID_597v2__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_ATSC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_J83B.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_TDMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB16_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBS2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_ISDBT1SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_13_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_ISDBT13SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DTMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_CMMB_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_CMMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBT2_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBT2_MultiPLP.rbf", (int)('A'+_mod_typ));	break;	//			sprintf(_fn,"rbf/TVB597_%c_DVBT2_SinglePLP.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBC2_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_DVBC2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_RESERVED_O_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ATSC_MH_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_ATSCMH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_IQ_PLAY_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"/root/rbf/TVB597_%c_ISDBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TSIO_MODE:			sprintf(_fn,"/root/rbf/TVB597_TSIO.rbf");	break;
		default:
			sprintf(_fn,"/root/rbf/TVB597_TSIO.rbf");	break;
		}
		break;
	default:
		TSPL_nModulatorType = TVB380_TSIO_MODE;
		sprintf(_fn,"/root/rbf/tvb%i%c.rbf", TSPL_nBoardTypeID, (int)('a'+_mod_typ));
		break;
	}
#else
	switch(TSPL_nBoardTypeID)
	{
	case	_BD_ID_598__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB598_DVBT.rbf");	break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB598_ATSC.rbf");	break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB598_DVBC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB598_J83B.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"rbf/TVB598_DVBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB598_TDMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB16_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB598_DVBH.rbf");	break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"rbf/TVB598_DVBS2.rbf");	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB598_ISDBT1SEG.rbf");	break;
		case	TVB380_ISDBT_13_MODE:		sprintf(_fn,"rbf/TVB598_ISDBT.rbf");	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB598_DTMB.rbf");	break;
		case	TVB380_CMMB_MODE:			sprintf(_fn,"rbf/TVB598_CMMB.rbf");	break;
		case	TVB380_DVBT2_MODE:			sprintf(_fn,"rbf/TVB598_DVBT2.rbf");	break;	//			sprintf(_fn,"rbf/TVB597_%c_DVBT2_SinglePLP.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBC2_MODE:			sprintf(_fn,"rbf/TVB598_DVBC2.rbf");	break;
		case	TVB380_RESERVED_O_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ATSC_MH_MODE:		sprintf(_fn,"rbf/TVB598_ATSCMH.rbf");	break;
		case	TVB380_IQ_PLAY_MODE:
			sprintf(_fn,"rbf/TVB598_IQPLAY.rbf");	break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"rbf/TVB598_ISDBS.rbf");	break;
		case	TVB380_TSIO_MODE:			sprintf(_fn,"rbf/TVB598_TSIO.rbf");	break;
		case	TVB380_MULTI_QAMB_MODE:			sprintf(_fn,"rbf/TVB598_J83B_MULTI.rbf");	break;
		case	TVB380_MULTI_VSB_MODE:			sprintf(_fn,"rbf/TVB598_ATSC_MULTI.rbf");	break;
		case	TVB380_MULTI_DVBT_MODE:			sprintf(_fn,"rbf/TVB598_DVBT_MULTI.rbf");	break;
		default:
			TSPL_nModulatorType = TVB380_TSIO_MODE;
			sprintf(_fn,"rbf/TVB599A_TSIO.rbf");	break;
		}
		break;
		
	case	_BD_ID_599__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB599A_DVBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB599A_ATSC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB599A_DVBC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB599A_J83B.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"rbf/TVB599A_DVBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB599A_TDMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB16_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB599A_DVBH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBS2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB599A_ISDBT1SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_13_MODE:		sprintf(_fn,"rbf/TVB599A_ISDBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB599A_DTMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_CMMB_MODE:			sprintf(_fn,"rbf/TVB599A_CMMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBT2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBT2.rbf", (int)('A'+_mod_typ));	break;	//			sprintf(_fn,"rbf/TVB597_%c_DVBT2_SinglePLP.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBC2_MODE:			sprintf(_fn,"rbf/TVB599A_DVBC2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_RESERVED_O_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ATSC_MH_MODE:		sprintf(_fn,"rbf/TVB599A_ATSCMH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_IQ_PLAY_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"rbf/TVB599A_ISDBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TSIO_MODE:			sprintf(_fn,"rbf/TVB599A_TSIO.rbf");	break;
		default:
			TSPL_nModulatorType = TVB380_TSIO_MODE;
			sprintf(_fn,"rbf/TVB599A_TSIO.rbf");	break;
		}
		break;
		
	case	_BD_ID_597v2__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBT.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB597_%c_ATSC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBC.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB597_%c_J83B.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB597_%c_TDMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_VSB16_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBS2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB597_%c_ISDBT1SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_ISDBT_13_MODE:			sprintf(_fn,"rbf/TVB597_%c_ISDBT13SEG.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB597_%c_DTMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_CMMB_MODE:			sprintf(_fn,"rbf/TVB597_%c_CMMB.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBT2_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBT2_MultiPLP.rbf", (int)('A'+_mod_typ));	break;	//			sprintf(_fn,"rbf/TVB597_%c_DVBT2_SinglePLP.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_DVBC2_MODE:			sprintf(_fn,"rbf/TVB597_%c_DVBC2.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_RESERVED_O_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ATSC_MH_MODE:			sprintf(_fn,"rbf/TVB597_%c_ATSCMH.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_IQ_PLAY_MODE:
			sprintf(_fn,"rbf/unknown.rbf");	break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"rbf/TVB597_%c_ISDBS.rbf", (int)('A'+_mod_typ));	break;
		case	TVB380_TSIO_MODE:			sprintf(_fn,"rbf/TVB597_TSIO.rbf");	break;
		default:
			TSPL_nModulatorType = TVB380_TSIO_MODE;
			sprintf(_fn,"rbf/TVB597_TSIO.rbf");	break;
		}
		break;

	case	_BD_ID_591__:
		if (TSPL_nBoardRevision < __REV_2_0_of_bd_591v2)
		{
			switch(_mod_typ)
			{
			case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB591_A_DVBT.rbf"); break;
			case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB591_B_ATSC.rbf"); break;
			case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB591_C_DVBC.rbf"); break;
			case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB591_D_J83B.rbf"); break;
			case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB591_F_TDMB.rbf"); break;
			case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB591_H_DVBH.rbf"); break;
			case	TVB380_IQ_PLAY_MODE:			sprintf(_fn,"rbf/TVB591_Q_IQPLAY.rbf"); break;
			default:
				sprintf(_fn,"rbf/unknown.rbf"); break;
			}
		}
		else
		{
			switch(_mod_typ)
			{
			case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB591v2_A_DVBT.rbf"); break;
			case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB591v2_B_ATSC.rbf"); break;
			case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB591v2_C_DVBC.rbf"); break;
			case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB591v2_D_J83B.rbf"); break;
			case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB591v2_F_TDMB.rbf"); break;
			case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB591v2_H_DVBH.rbf"); break;
			case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB591v2_J_ISDBT_1SEG.rbf");	break;
			case	TVB380_ISDBT_13_MODE:		sprintf(_fn,"rbf/TVB591v2_K_ISDBT_13SEG.rbf");	break;
			case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB591v2_L_DTMB.rbf");	break;
			case	TVB380_ATSC_MH_MODE:		sprintf(_fn,"rbf/TVB591v2_P_ATSCMH.rbf");	break;
			case	TVB380_IQ_PLAY_MODE:		sprintf(_fn,"rbf/TVB591v2_Q_IQPLAY.rbf"); break;
			default:
				sprintf(_fn,"rbf/unknown.rbf"); break;
			}
		}
		break;
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		switch(_mod_typ)
		{
		case	TVB380_DVBT_MODE:			sprintf(_fn,"rbf/TVB591S_A_DVBT.rbf"); break;
		case	TVB380_VSB8_MODE:			sprintf(_fn,"rbf/TVB591S_B_ATSC.rbf"); break;
		case	TVB380_QAMA_MODE:			sprintf(_fn,"rbf/TVB591S_C_DVBC.rbf"); break;
		case	TVB380_QAMB_MODE:			sprintf(_fn,"rbf/TVB591S_D_J83B.rbf"); break;
		case	TVB380_QPSK_MODE:			sprintf(_fn,"rbf/TVB591S_E_DVBS.rbf");	break;
		case	TVB380_TDMB_MODE:			sprintf(_fn,"rbf/TVB591S_F_TDMB.rbf"); break;
		case	TVB380_DVBH_MODE:			sprintf(_fn,"rbf/TVB591S_H_DVBH.rbf"); break;
		case	TVB380_DVBS2_MODE:			sprintf(_fn,"rbf/TVB591S_I_DVBS2.rbf");	break;
		case	TVB380_ISDBT_MODE:			sprintf(_fn,"rbf/TVB591S_J_ISDBT_1SEG.rbf");	break;
		case	TVB380_ISDBT_13_MODE:		sprintf(_fn,"rbf/TVB591S_K_ISDBT_DM.rbf");	break;
		case	TVB380_DTMB_MODE:			sprintf(_fn,"rbf/TVB591S_L_DTMB.rbf");	break;
		case	TVB380_ATSC_MH_MODE:		sprintf(_fn,"rbf/TVB591S_P_ATSCMH.rbf");	break;
		case	TVB380_IQ_PLAY_MODE:		sprintf(_fn,"rbf/TVB591S_Q_IQPLAY.rbf"); break;
		case	TVB380_ISDBS_MODE:			sprintf(_fn,"rbf/TVB591S_R_ISDBS.rbf");	break;
		case	TVB380_MULTI_QAMB_MODE:		sprintf(_fn,"rbf/TVB591S_T_J83B_Quad.rbf");	break;
		case	TVB380_MULTI_VSB_MODE:		sprintf(_fn,"rbf/TVB591S_U_ATSC_Quad.rbf");	break;
		default:
			sprintf(_fn,"rbf/unknown.rbf"); break;
		}
		break;
	default:
		if(TSPL_nBoardTypeID == _BD_ID_593__ && _mod_typ == TVB380_TSIO_MODE)
		{
			sprintf(_fn,"rbf/TVB593_TSIO.rbf");
		}
		else
			sprintf(_fn,"rbf/tvb%i%c.rbf", TSPL_nBoardTypeID, (int)('a'+_mod_typ));
		break;
	}
#endif
	LldPrint_2(_fn, 0, 0);
}
void	CLldHwIf::MakeRbfNameOld(char *_fn)
{
	sprintf(_fn,"rbf/sdc%i%c.rbf", TSPL_nBoardTypeID,'a');
	LldPrint_2(_fn, 0, 0);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#ifdef WIN32
#else
#if defined(TVB595V1)
int CLldHwIf::print_device(struct usb_device *dev, int level)
{
  usb_dev_handle *udev;
  char description[256];
  char string[256];
  int ret, i;

  udev = usb_open(dev);
  if (udev) {
    if (dev->descriptor.iManufacturer) {
      ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
      if (ret > 0)
        snprintf(description, sizeof(description), "%s - ", string);
      else
        snprintf(description, sizeof(description), "%04X - ",
                 dev->descriptor.idVendor);
    } else
      snprintf(description, sizeof(description), "%04X - ",
               dev->descriptor.idVendor);

    if (dev->descriptor.iProduct) {
      ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
      if (ret > 0)
        snprintf(description + strlen(description), sizeof(description) -
                 strlen(description), "%s", string);
      else
        snprintf(description + strlen(description), sizeof(description) -
                 strlen(description), "%04X", dev->descriptor.idProduct);
    } else
      snprintf(description + strlen(description), sizeof(description) -
               strlen(description), "%04X", dev->descriptor.idProduct);

  } else
    snprintf(description, sizeof(description), "%04X - %04X",
             dev->descriptor.idVendor, dev->descriptor.idProduct);

  printf("%.*sDev #%d: %s\n", level * 2, "                    ", dev->devnum,   description);

  if (udev && verbose) {
    if (dev->descriptor.iSerialNumber) {
      ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
      if (ret > 0)
        printf("%.*s  - Serial Number: %s\n", level * 2, "                    ", string);
    }
  }

  if (udev)
    usb_close(udev);

  if (verbose) {
    if (!dev->config) {
      printf("  Couldn't retrieve descriptors\n");
      return 0;
    }

    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
      ;//print_configuration(&dev->config[i]);
  } else {
    for (i = 0; i < dev->num_children; i++)
      print_device(dev->children[i], level + 1);
  }

  return 0;
}

void CLldHwIf::tvb595_usb_init(void)
{
	usb_init();
	printf("[usb] : usb-init\n");
}

struct usb_device* CLldHwIf::tvb595_usb_ready(void)
{
	int retval = 0;
	struct usb_bus *bus = NULL;
	struct usb_device *dev = NULL;

	usb_find_busses();
	usb_find_devices();

#if defined(WIN32)
#else
	char name[64];
	FILE *fp;
	struct usb_device *tmp_dev = NULL;
	int	bd_location;
#endif
	for (bus = usb_busses; bus; bus = bus->next) 
	{
		if (bus->root_dev && !verbose)
			print_device(bus->root_dev, 0);
		else 
		{
			for (dev = bus->devices; dev; dev = dev->next)
			{
				print_device(dev, 0);

				if ( dev->descriptor.idVendor == VENDOR_ID &&
					(dev->descriptor.idProduct == TVB595_ID ||
					dev->descriptor.idProduct == __ID_BDs_MODEL_597v2__ ||
					dev->descriptor.idProduct == TVB597A_ID ||
					dev->descriptor.idProduct == TVB499_ID ||
					dev->descriptor.idProduct == TVB497_ID) )
				{
					//2010/9/29 TEST MultiBoard
					sprintf(name, "./teleview_usbdevice_%04X_%04X_%d_%s", dev->descriptor.idVendor, dev->descriptor.idProduct, dev->devnum, dev->bus->dirname);	//2010/10/20 FIXED
					
					fp = fopen(name, "r" );
					if ( fp == NULL )
					{
						fp = fopen(name, "w");
						fclose(fp);
					}
					else
					{
						continue;
					}
					//TVB597A
					if ( dev->descriptor.idProduct == TVB597A_ID )
					{
						TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_597_;
					}
					//TVB497
					else if ( dev->descriptor.idProduct == TVB497_ID )
					{
						TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_593_;
					}
					else if ( dev->descriptor.idProduct == TVB499_ID )
					{
						TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_593_;
					}
					else if ( dev->descriptor.idProduct == __ID_BDs_MODEL_597v2__ )
					{
						TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_597v2_;
					}
					else
					{
						TSPL_nUseRemappedAddress = _ADDR_REMAP_METHOD_dont_;
					}
					//2010/10/13
					bd_location = dev->devnum;//atoi(dev->bus->dirname) * 256 + dev->devnum; //dev->devnum;	//2010/11/10	
					___set_board_location(bd_location);
					retval = 1;//device found
					break;
				}
			}
		}
		//20081210 - added
		if ( retval == 1 )//device found
			break;
	}
#if defined(WIN32)
#else
	for (bus = usb_busses; bus; bus = bus->next) 
	{
		for (tmp_dev = bus->devices; tmp_dev; tmp_dev = tmp_dev->next)
		{
			if ( tmp_dev->descriptor.idVendor == 0x9445 &&	//	VENDOR_ID
				(tmp_dev->descriptor.idProduct == TVB595_ID ||
				tmp_dev->descriptor.idProduct == __ID_BDs_MODEL_597v2__ ||
				tmp_dev->descriptor.idProduct == TVB597A_ID ||
				tmp_dev->descriptor.idProduct == TVB499_ID ||
				tmp_dev->descriptor.idProduct == TVB497_ID) )
			{
				sprintf(name, "./%04X_%04X_%d", tmp_dev->descriptor.idVendor, tmp_dev->descriptor.idProduct, tmp_dev->devnum);
				fp = fopen(name, "rt+" );
				if ( fp != NULL )
				{
					fgets(name, 32, fp);
					if ( atoi(name) == 1 )
					{
						fseek(fp, 0L, SEEK_SET);
						fputs("0", fp);
					}
					fclose(fp);
				}
			}
		}
	}
#endif	
	if ( retval == 0 )
	{
//		printf("No device found. Check the device connection.!!!\n");
		return NULL;
	}
	LldPrint_2("[usb] : checking tvb-usb-ready : [ok]", 0, 0);
	return dev;
}

void CLldHwIf::tvb595_usb_close(usb_dev_handle *hDevice_usb)
{
	if ( !hDevice_usb ) return;

	usb_close(hDevice_usb);
	LldPrint_FCall("[usb] : tvb-usb-close : [ok]", 0, 0);
}

usb_dev_handle* CLldHwIf::tvb595_usb_open(void)
{
	struct usb_device *dev = NULL;

	dev = tvb595_usb_ready();
	if ( !dev )
	{
		return NULL;
	}
	LldPrint_FCall("[usb] : tvb-usb-open", 0, 0);
	return usb_open(dev);
}
#endif
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int CLldHwIf::IsAttachedBdTyp_AllCase_w_SpecialBd(void)
{
	if (IsAttachedBdTyp_AllCase())
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_380v3_upconverter__:
	case	_BD_ID_390v6__:
	case	_BD_ID_390v6_Lowcost__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_AllCase(void)	//	higher 390v7.0
{
	if (IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis())
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case _BD_ID_599__: 
	case _BD_ID_598__: 
	case	_BD_ID_390v7_IF_only__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_HaveDmxCmtlTest(void)	//	higher 390v8.0
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_NewAmpBypassModeCntl(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_499__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	if (TSPL_nTVB595Board > __REV_595_v1_0)	//	???
	{
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseAgcFreqTbl(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		return	1;
	}
	if (TSPL_nTVB595Board > __REV_595_v1_0)	//	???
	{
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_TVB593_597A_V3(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
		if (TSPL_nBoardRevision >= __REV_6_0_of_bd_593v3_597v3)
		{
			return	1;
		}
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_597A_V4(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597v2__:
		if (TSPL_nBoardRevision >= __REV_9_0_of_bd_59x)
		{
			return	1;
		}
	}
	return	0;
}


int CLldHwIf::IsAttachedBdTyp_TVB593_V4(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
		if (TSPL_nBoardRevision >= __REV_8_0_of_bd_593v4)
		{
			return	1;
		}
	}
	return	0;
}


int CLldHwIf::IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV_390_590__)
		{
			return	1;
		}
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2(void)
{
	return	IsAttachedBdTyp_NewAddrMap();
}
int CLldHwIf::IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin(void)
{
	if (IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk())
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_380v3_upconverter__:
	case	_BD_ID_390v6__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_390v7_IF_only__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_ClkGenAdjForDvbS2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_ClkGenAdjForIsdbT13(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_ClkGenAdjForQamA(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_591__:
		if (TSPL_nBoardRevision >= __REV_2_0_of_bd_591v2)
		{
			return	0;
		}
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_DacAdjAccordingToBaseBandFreq(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_WhyThisGrpNeed_13(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_WhyThisGrpNeed_14(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_380v3_upconverter__:
	case	_BD_ID_390v6__:
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_390v7_IF_only__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_WhyThisGrpNeed_15(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_DiffCntlMethodAd9852Reg(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
	case	_BD_ID_590s__:
	//case	_BD_ID_593__:
	case	_BD_ID_497__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_IndirectCntlMethodAd9852Reg(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597v2__:
	case	_BD_ID_593__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_UsbTyp(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UsbTyp_ExceptFor597v2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_SupportFastDownload(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	case	_BD_ID_597v2__:
		if ( TSPL_nBoardRevision >= 0x8)
		{
			return	1;
		}
	case	_BD_ID_591S__:
		if ( TSPL_nBoardRevision >= 0x4)
		{
			return	1;
		}
	case	_BD_ID_593__:
		if ( TSPL_nBoardRevision >= 0xA)
		{
			return	1;
		}
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_SupportEepromRW(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
	case	_BD_ID_598__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591S__:
	case	_BD_ID_593__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UsbTyp_w_497(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_497__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseAD9775(void)	//	dac is ad9775
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_HaveBlkRstFunc(void)
{
	if (IsAttachedBdTyp_UseAD9775())
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_DiffAlteraSizeIndicator(void)
{
	if (IsAttachedBdTyp_UseAD9775())
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndUmc(void)	//	590s/593/597v2 or 497 with tvo1626
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case _BD_ID_591S__:
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_497__:
		if (TSPL_nBoardOption == 6)
		{
			return	1;
		}
		break;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1(void)	//	593/497/597v2 or 590s higher rev3
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case _BD_ID_591S__:
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV_390_590__)
		{
			return	1;
		}
		break;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1(void)	//	593/497/597v2
{
	return	IsAttachedBdTyp_NewAddrMap();
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1_1(void)	//	497/597v2 or 593 higher rev2
{
	if ( TSPL_nBoardRevision >= __REV_2_0__of_bd_593__)
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl(void)	//	593/497/597v2 or 593 higher rev2
{
	if (TSPL_nBoardRevision >= __REV_2_0__of_bd_593__)
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case _BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AD9852Clk_ASx(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
	case	_BD_ID_390v8__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597v2__:
		if (TSPL_nBoardRevision == __REV_590_express || TSPL_nBoardRevision == __REV_595_v1_1)
		{
			return	1;
		}
		break;
	case	_BD_ID_594__:
	case	_BD_ID_591__:	//	???
	case	_BD_ID_591S__:	//	???
		return	1;
	}
	if ((TSPL_nBoardRevision_Ext == TVB597A_REV_EXT) || (TSPL_nBoardRevision_Ext == TVB590S_REV_EXT)	|| (TSPL_nBoardRevision_Ext == TVB593_REV_EXT))
		//0x03(rev.) == TVB390/TVB590, 0x04(rev.) == TVB590/express
		//0x04(rev.) == TVB595(V1.0), 0x05(rev.) == TVB595(V1.1)
		//0x95D1 == TVB595D(V1.0)
		//0x90D1 == TVB590S(V1.0)
	{
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AD9852Clk_ASx_2(void)	//	???
{
	if ((TSPL_nBoardRevision == __REV_590_express) || (TSPL_nBoardRevision == __REV_595_v1_1))
	{
		return	1;
	}
	if ((TSPL_nBoardRevision_Ext == TVB597A_REV_EXT) || (TSPL_nBoardRevision_Ext == TVB590S_REV_EXT) || (TSPL_nBoardRevision_Ext == TVB593_REV_EXT))
		//0x03 == TVB390/TVB590, 0x04 == TVB590/express
		//0x04 == TVB595(V1.0), 0x05 == TVB595(V1.1)
		//0x95D1 == TVB595D(V1.0)
		//0x90D1 == TVB590S(V1.0)
	{
		return	1;
	}
	if(TSPL_nBoardTypeID == _BD_ID_591__ || TSPL_nBoardTypeID == _BD_ID_591S__)
	{
		return 1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AD9852Clk_ASx_V(void)	//	597v2 rev2.1, 593 rev2.2, 590s rev2.2
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597v2__:
		if (TSPL_nBoardRevision >= __REV_597A_v2_1)
		{
			return	1;
		}
		break;
	case	_BD_ID_593__:
		if (TSPL_nBoardRevision >= __REV5_593_v2_2)
		{
			return	1;
		}
		break;
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV5_593_v2_2)
		{
			return	1;
		}
		break;

	case	_BD_ID_591__:
		if (TSPL_nBoardRevision >= __REV_2_0_of_bd_591v2)
		{
			return	1;
		}
		break;
	//2012/1/31 TVB591S
	case _BD_ID_591S__:
		return 1;
		break;
	}

	return	0;
}

//2012/8/27 new rf level control
int CLldHwIf::IsAttachedBdTyp_NewRFLevel_Cntl(void)	//	597A v2, 593, 590s v3, 591, 591s
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV_390_590__)
		{
			return	1;
		}
		break;
	case	_BD_ID_597v2__:
	case	_BD_ID_593__:
	case	_BD_ID_591__:
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}

	return	0;
}

int CLldHwIf::IsAttachedBdTyp_380v3_ifrf_scaler(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_380v3_ifrf_scaler__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_380v3_upconverter(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_380v3_upconverter__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_390v6_Lowcost(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v6_Lowcost__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_390v7_IF_only(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7_IF_only__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_390v7(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v7__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_390v8(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v8__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_590v9(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590v9_x__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_590v10(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590v10_x__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_593(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_595v2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595v2__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_595v3(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595Bv3__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_595v4(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	0x3D:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_597(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_597v2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597v2__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_599(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_598(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_599_598_ver2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		if (TSPL_nBoardRevision >= __REV_2_0_of_bd_591Sv2)	//Revision 2
			return	1;
	}
	return	0;
	
}


//2013/1/4 TVB499
int CLldHwIf::IsAttachedBdTyp_499(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_499__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_597_or_597v2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_497(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_497__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_591(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591__:
		return	1;
	}
	return	0;
}
//2012/1/31 TVB591S
int CLldHwIf::IsAttachedBdTyp_591S(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}

//2012/7/9 HMC833 ===========================================================================================
int CLldHwIf::IsAttachedBdTyp_591S_V2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591S__:
		if(TSPL_nBoardRevision >= __REV_2_0_of_bd_591Sv2)
			return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseVcoPllN_Hmc833(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return 1;
		break;
	//2012/8/22 TVB593/597A V3
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
		if(TSPL_nBoardRevision >= __REV_6_0_of_bd_593v3_597v3)
			return 1;
		break;
	case	_BD_ID_591S__:
		if(TSPL_nBoardRevision >= __REV_2_0_of_bd_591Sv2)
			return	1;
		break;
	}
	return	0;
}
//===========================================================================================================


int CLldHwIf::IsAttachedBdTyp_594(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp__Virtual(void)	//	common func. for 594/593/591s
{
	if (parent_id__of_virtual_bd_ >= 0)
	{
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_SupportMultiRfOut(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_594__:
	case	_BD_ID_598__:
			return	1;
		}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_SupportMultiRfOut_593_591s(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::CntAdditionalVsbRfOut_593_591s(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TSPL_nModulatorType != TVB380_MULTI_VSB_MODE)
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;

/*
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
		if (cnt_my_sub_ts_vsb <= 1)			return	0;	//	real only
		if (TSPL_nModulatorType != TVB380_VSB8_MODE)			return	0;
		return	cnt_my_sub_ts_vsb;
	}
	return	0;
*/
}
int CLldHwIf::CntAdditionalQamRfOut_593_591s(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TSPL_nModulatorType != TVB380_MULTI_QAMB_MODE)
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;

/*
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
		if (cnt_my_sub_ts_qam <= 1)			return	0;	//	real only
		if (TSPL_nModulatorType != TVB380_QAMB_MODE)			return	0;
		return	cnt_my_sub_ts_qam;
	}
	return	0;
*/
}
int CLldHwIf::CntAdditionalDvbTRfOut_593(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_598__:
		break;
	default:
		return	0;
	}
	if (TSPL_nModulatorType != TVB380_MULTI_DVBT_MODE)
		return	0;
	if (m_system_pkg_to_support_multi_rfout)
		return	1;
	return	0;

/*
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_591S__:
		if (cnt_my_sub_ts_qam <= 1)			return	0;	//	real only
		if (TSPL_nModulatorType != TVB380_QAMB_MODE)			return	0;
		return	cnt_my_sub_ts_qam;
	}
	return	0;
*/
}

int CLldHwIf::IsAttachedBdTyp_593_or_497(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_497__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_497_or_597v2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_590v10_or_590s(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590v10_x__:
	case	_BD_ID_590s__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_590s_SmallAddrSpace(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_NewAddrMap(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	//2013/1/4 added
	case	_BD_ID_499__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	//2013/5/27 TVB599
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_499__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_DdsIncNewEquation(void)
{
	//TVB590S //TVB593
	if ((TSPL_nBoardRevision >= __REV_390_590__) ||
		(TSPL_nBoardRevision_Ext == TVB590S_REV_EXT) || (TSPL_nBoardRevision_Ext == TVB593_REV_EXT))
	{
		return	1;
	}
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_591__:
	case _BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_UseIndirectRegAccess(void)
{
	return	IsAttachedBdTyp_NewAddrMap();
}
//2012/4/12 SINGLE TONE
int CLldHwIf::IsAttachedBdTyp_SingleTone(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_WhyThisGrpNeed_4(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_390v8__:
	case	_BD_ID_594__:
	case	_BD_ID_590v9_x__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AttenTyp_1(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AttenTyp_2(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_594__:
	case	_BD_ID_590v10_x__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
	case	_BD_ID_597__:
	case	_BD_ID_590s__:
	case	_BD_ID_593__:
	case	_BD_ID_497__:
	case	_BD_ID_597v2__:
	case	_BD_ID_591__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_AttenTyp_3(void)
{
	return	IsAttachedBdTyp_UseAD9775();
}
int CLldHwIf::IsAttachedBdTyp_AttenTyp_4(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_593__:
	case	_BD_ID_597v2__:
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:
	case	_BD_ID_599__:
	case	_BD_ID_598__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_AttenTyp_AssignNewBitsPos(void)	//	refer to TRF178_ATTN_ADDR_CONTROL_REG
{
	return	IsAttachedBdTyp_NewAddrMap();
}

int CLldHwIf::IsAttachedBdTyp_595_Vx(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_595v2__:
	case	_BD_ID_595Bv3__:
	case	0x3D:
		return	1;
	}
	return	0;
}
int CLldHwIf::IsAttachedBdTyp_597_497(void)
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_597__:
	case	_BD_ID_597v2__:
	case	_BD_ID_497__:
	case	_BD_ID_599__:
		return	1;
	}
	return	0;
}

int CLldHwIf::IsAttachedBdTyp_590S_Ver2(void)	//	590s rev2
{
	switch (TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision == __REV_2_0__of_bd_590s__)
		{
			return	1;
		}
		break;
	}
	return	0;
}

//2013/5/23 TVB599
int CLldHwIf::IsAttachedBdTyp_SupportBoardFeature(void)
{
	switch(TSPL_nBoardTypeID)
	{
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV_5_0__sel_new_path_Chip__)
		{
			return	1;
		}
		break;
	}
	if(IsAttachedBdTyp_NewAddrMap())
		return 1;
	return 0;
}

int CLldHwIf::Check_HMCorOPEN_Mode(int *hmc833__, int *hmc1033__)
{
	unsigned int nStatus;
	unsigned int reg_data;
	*hmc833__ = -1;
	*hmc1033__ = -1;
	nStatus = (1 << 27);
	reg_data = (int)WDM_Read_TSP_Indirectly(TRF178_ATTN_ADDR_CONTROL_REG);
	reg_data = (((nStatus >> 8) & 0xFFFFFF) << 8) + (reg_data&0xFF);
	if(TSPL_SET_CONFIG_DOWNLOAD(TRF178_ATTN_ADDR_CONTROL_REG, reg_data))
	{
		LldPrint_Error("HMC833 error", 0, reg_data);
		return 0;
	}
	Sleep(10);
	if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, 0x80000000))
	{
		LldPrint_Error("Function Check_HMCorOPEN_Mode : pos...", 1, 1);
	}
	Sleep(10);
	nStatus = WDM_Read_TSP(FPGA_HMC833_READ_DATA_REG2);
	if(((nStatus & 0x1FFFFFE) >> 1) == 0xA7975)
	{
		LldPrint_1x("[LLD] HMC833 HMC Mode===", nStatus);
		//OutputDebugString("===HMC833 HMC Mode===\n");
		*hmc833__ = 0;
	}
	if(*hmc833__ == -1)
	{
		if( TSPL_SET_CONFIG_DOWNLOAD(TRF178_PLL2_ADDR_CONTROL_REG, 0x00000000))
		{
			LldPrint_Error("Function Check_HMCorOPEN_Mode : pos...", 1, 2);
		}
		Sleep(10);
		nStatus = WDM_Read_TSP(FPGA_HMC833_READ_DATA_REG2);
		if(((nStatus >> 8) & 0xFFFFFF) == 0xA7975)
		{
			LldPrint_1x("[LLD] HMC833 OPEN Mode===", nStatus);
			//OutputDebugString("===HMC833 Open Mode===\n");
			*hmc833__ = 1;
		}
		else
		{
			LldPrint_1x("[LLD] Default HMC833 HMC Mode===", nStatus);
			*hmc833__ = 0;
		}
	}

	if( TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, 0x80000000))
	{
		LldPrint_Error("Function Check_HMCorOPEN_Mode : pos...", 1, 1);
	}
	Sleep(10);
	nStatus = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
	if(((nStatus & 0x1FFFFFE) >> 1) == 0xA7975)
	{
		LldPrint_1x("[LLD] HMC1033 HMC Mode===", nStatus);
		//OutputDebugString("===HMC1033 HMC Mode===\n");
		*hmc1033__ = 0;
	}
	if(*hmc1033__ == -1)
	{
		if( TSPL_SET_CONFIG_DOWNLOAD(FPGA_HMC1033_CTRL_REG, 0x00000000))
		{
			LldPrint_Error("Function Check_HMCorOPEN_Mode : pos...", 1, 2);
		}
		Sleep(10);
		nStatus = WDM_Read_TSP(FPGA_HMC1033_READ_DATA_REG);
		if(((nStatus >> 8) & 0xFFFFFF) == 0xA7975)
		{
			LldPrint_1x("[LLD] HMC1033 OPEN Mode===", nStatus);
			//OutputDebugString("===HMC1033 Open Mode===\n");
			*hmc1033__ = 1;
		}
		else
		{
			LldPrint_1x("[LLD] Undefine HMC1033 HMC Mode===", nStatus);
			*hmc1033__ = 0;
		}
	}
	return 0;
}

int CLldHwIf::Read_Eeprom_Information(int *_dac_i_offset, int *_dac_q_offset, char *_license_number)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;
	unsigned long			dwStatus;
	const int Dac_Offset_Base_Addr = 0x300;
	const int License_Base_Addr = 0x400;
	const int Step_2Bytes = 0x2;
	unsigned long nCheckSum = 0;
	char str_tmp[8];
	char str_debug[256];

	LockDmaMutex();
	//Read Magic word.
	KRegInf.dwCmdParm1 = Dac_Offset_Base_Addr;

#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
	TLV_DeviceIoControl(hDevice,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
	dwStatus = (DWORD)(KRegInf.dwCmdParm2);

	if(dwStatus == 0xDAC0)
	{
		nCheckSum = dwStatus;
		//Read DAC I Offset
		KRegInf.dwCmdParm1 = Dac_Offset_Base_Addr + Step_2Bytes;
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);
		nCheckSum = nCheckSum + dwStatus;
		if((dwStatus & 0x1000) > 0)
		{
			*_dac_i_offset = ((dwStatus & 0xFFF) * -1);
		}
		else
		{
			*_dac_i_offset = dwStatus;
		}
		
		//Read DAC Q Offset
		KRegInf.dwCmdParm1 = Dac_Offset_Base_Addr + (Step_2Bytes * 2);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);
		nCheckSum = nCheckSum + dwStatus;
		if((dwStatus & 0x1000) > 0)
		{
			*_dac_q_offset = ((dwStatus & 0xFFF) * -1);
		}
		else
		{
			*_dac_q_offset = dwStatus;
		}
		
		//Read Check Sum
		KRegInf.dwCmdParm1 = Dac_Offset_Base_Addr + (Step_2Bytes * 3);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);
		KRegInf.dwCmdParm1 = Dac_Offset_Base_Addr + (Step_2Bytes * 4);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
		dwStatus = ((dwStatus & 0xFFFF) << 16) + (DWORD)(KRegInf.dwCmdParm2 & 0xFFFF);
		if(nCheckSum != dwStatus)
		{
			*_dac_i_offset = 0;
			*_dac_q_offset = 0;
		}
	}
	else
	{
		*_dac_i_offset = 0;
		*_dac_q_offset = 0;
	}
	//sprintf(str_debug, "I offset: %d, Q offset: %d\n",  *_dac_i_offset, *_dac_q_offset);
	//OutputDebugString(str_debug);

	//Read Magic word.
	KRegInf.dwCmdParm1 = License_Base_Addr;

#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
	TLV_DeviceIoControl(hDevice,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
	dwStatus = (DWORD)(KRegInf.dwCmdParm2);
	if(dwStatus != 0xA720)
	{
		UnlockDmaMutex();
		return -1;
	}
	nCheckSum = dwStatus;
#ifdef WIN32
	strcpy_s(_license_number, 64, "");
#else
	strcpy(_license_number, "");
#endif
	for(int i = 1; i <= 8; i++)
	{
		KRegInf.dwCmdParm1 = License_Base_Addr + (Step_2Bytes * i);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
		TLV_DeviceIoControl(hDevice,
				   IOCTL_READ_EEPROM_AT25640,
				   &KRegInf,
				   sizeof(KRegInf),
				&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
		dwStatus = (DWORD)(KRegInf.dwCmdParm2);
#ifdef WIN32
		sprintf_s(str_tmp, 8, "%04X", dwStatus);
		strcat_s(_license_number, 64, str_tmp);
#else
		sprintf(str_tmp,"%04X", dwStatus);
		strcat(_license_number, str_tmp);
#endif
		nCheckSum = nCheckSum + dwStatus;
	}
	//Read Check Sum
	KRegInf.dwCmdParm1 = License_Base_Addr + (Step_2Bytes * 9);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
	TLV_DeviceIoControl(hDevice,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
	dwStatus = (DWORD)(KRegInf.dwCmdParm2);
	KRegInf.dwCmdParm1 = License_Base_Addr + (Step_2Bytes * 10);
#ifndef WIN32
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),	
			   &dwRet,0);
	}
	else
	{
#endif
	TLV_DeviceIoControl(hDevice,
			   IOCTL_READ_EEPROM_AT25640,
			   &KRegInf,
			   sizeof(KRegInf),
			&KRegInf, sizeof(KRegInf), &dwRet,0);
#ifndef WIN32
	}
#endif
	dwStatus = ((dwStatus & 0xFFFF) << 16) + (DWORD)(KRegInf.dwCmdParm2 & 0xFFFF);
	if(nCheckSum != dwStatus)
	{
		UnlockDmaMutex();
		return -1;
	}
	//sprintf(str_debug, "Authorization Key: %s\n",  _license_number);
	//OutputDebugString(str_debug);
	UnlockDmaMutex();

	return 0;

}

void CLldHwIf::Write_LN_Eeprom(char *szLN)
{
	unsigned long address;
	unsigned long data;
	int checkSum = 0;
	char ch;
    int _val;
	address = 0x400;
	data = 0xA720;
	TSPL_WRITE_CONTROL_REG(3, address, data);
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
			TSPL_WRITE_CONTROL_REG(3, address, data);
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
	TSPL_WRITE_CONTROL_REG(3, address, data);
	Sleep(100);
	address = 0x414;
	data = (unsigned long)(checkSum & 0xFFFF);
	TSPL_WRITE_CONTROL_REG(3, address, data);
}