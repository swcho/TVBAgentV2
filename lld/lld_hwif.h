//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_LLD_HWIF_DEF_
#define	_TLV_LLD_HWIF_DEF_

#ifdef WIN32
#else
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/ioctl.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<semaphore.h>

#include	"../libusb-0.1.12/usb.h"
#include	"../libusb-0.1.12/usbi.h"
#endif

#include		"../include/logfile.h"
#include		"lld_regs.h"
#include		"lld_log.h"

/////////////////////////////////////////////////////////////////
#ifdef WIN32
#else
//	#define	_stdcall
typedef unsigned int DWORD;
typedef unsigned long ULONG;
#endif

/////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
typedef __int64			INT64;
#ifdef	BYPASS_WDM	
#define	TLV_DeviceIoControl(a,b,c,d,e,f,g,h) 	((bool)(1))
#else
#define	TLV_DeviceIoControl 	DeviceIoControl
#endif

#else	// LINUX

typedef long long	INT64;
#ifdef	BYPASS_WDM	
#define	TLV_DeviceIoControl(a,b,c,d,e,f,g,h) 	((bool)(1))
#else
#define	TLV_DeviceIoControl_usb(a,b,c,d,e,f,g,h) DispatchControl(a,(void*)c,(int)b)
#define	TLV_DeviceIoControl 	ioctl
#endif

#endif
#ifdef WIN32
/////////////////////////////////////////////////////////////////
#define	_CNT_MAX_BD_INSTALL_AVAILABLE_	23		//	refer to _MAX_INST_CNT_
#define	_CNT_MAX_PCI_BD_INSTALL_AVAILABLE_	20	//	refer to _MAX_INST_CNT_
#else
#define	_CNT_MAX_BD_INSTALL_AVAILABLE_	12		//	refer to _MAX_INST_CNT_
#define	_CNT_MAX_PCI_BD_INSTALL_AVAILABLE_	8	//	refer to _MAX_INST_CNT_
#endif
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CLldHwIf	:	public	CLldLog
{
private:
	int m_system_pkg_to_support_multi_rfout;

public:

	int	parent_id__of_virtual_bd_;
	int	my_ts_id;	//	__id_ts_seq__
	int	cnt_my_sub_ts_vsb;
	int	cnt_my_sub_ts_qam;
	//2012/6/28 multi dvb-t
	int	cnt_my_sub_ts_dvbt;
	int	TSPL_nUseRemappedAddress;
	int	TSPL_nBoardTypeID;
	int	TSPL_nAuthorization;
	int	TSPL_nBoardRevision;//0x03 == TVB390/TVB590, 0x04 == TVB590/express
								//0x04 == TVB595(V1.0), 0x05 == TVB595(V1.1)
								//0x01 == TVB590S
	int	TSPL_nBoardConfig;
	int	TSPL_nBoardUseAMP;//TVB590V9, 0=no amp, 1=+19dB gain amp, 2=+24dB gain amp
	int	TSPL_nTVB595Board; //0=TVB380/390/590, 4=TVB595(V1.0), 5=TVB595(V2.0 or higher)
	int TSPL_nFPGA_TEST;//FPGA d/w
	int TSPL_nBoardRevision_Ext;//TVB595D, 0x95D1, 0x0598 == TVB590S
	unsigned int TSPL_nMaxBoardCount;		//	_CNT_MAX_BD_INSTALL_AVAILABLE_
	unsigned int TSPL_nMaxPCIBoardCount;	//	_CNT_MAX_PCI_BD_INSTALL_AVAILABLE_
	int TSPL_nBoardLocation;
	int usb_BdFound;
#if 1
	//2011/6/22 fixed
	int TSPL_nBoardBUSnumber;
#endif
	//TVB593
	int	TLV_ControlErrorCode;
	int TSPL_nModulatorEnabled;
	int TSPL_nModulatorType;	//	TLV_MODULATOR_TYPE
	unsigned int TSPL_nEncryptedSN[2];
	unsigned char TSPL_EncryptedSN[32];
	unsigned char TSPL_ENC_SN[16];
	unsigned char TSPL_EncryptedLN[256];
	unsigned char TSPL_szLocation[256];
	int TSPL_nBoardSlotNum;//0;
	int TSPL_HMC833_status;

	//I/Q PLAY/CAPTURE
	int TSPL_nFPGA_ID;
	int TSPL_nFPGA_VER;
	int TSPL_nFPGA_BLD;
	int TSPL_nFPGA_RBF;
	int TSPL_nFPGA_IQ_PLAY;
	int TSPL_nFPGA_IQ_CAPTURE;

	//EEPROM(ATMAL 25640)
	int TSPL_Dac_I_Offset;
	int TSPL_Dac_Q_Offset;

	//2014/2/18
	int TSPL_T2_x7_Mod_Clock;

	//TVB593
	int TSPL_nBankOffset;
	int TSPL_nBankCount;
	//2013/5/13
	int TSPL_nMemoryCtrl_mode;
	//TVB497
	int TSPL_nBoardOption;//b'110=TVO1626, b'111=TVB1624
	int TSPL_nUse10MREFClock;//0=On-board 10MHz, 1=External 10MHz

	int TSPL_T2_parity_errCnt;

	unsigned long gChannel_Freq_Offset;//Hz
	int board_location;
	//2013/5/23 TVB599
	int nClockGeneratorModel;	//0: AD9852 AST(200MHz), 1: AD9852 ASV(300MHz), 2: Hittite HMC1033 & HMC988 model
	int nNumOfLED;				//0: 3 LED, 1: Single LED(TVB599)

	int gnHmc833_SerialPortMode;	//0: HMC mode, 1:Open mode
	int gnHmc1033_SerialPortMode;	//0: HMC mode, 1:Open mode
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
	DWORD	*dwpDMABufLinearAddr;	// 1Mbyte buffer for recoding by DMA
#ifdef WIN32
	HANDLE	hDevice;
	HANDLE	h_DmaMutex;
	HANDLE	h_EpldMutex;
#else
#if defined(TVB595V1)
	struct usb_dev_handle *hDevice_usb;
	int verbose;
#endif
	pthread_mutex_t   __mutex;
	pthread_mutex_t   __mutex_dma;
	int	__mutex_dma_cnt;
	int hDevice;
#endif

	//2014/2/19 RESET flag
	int ModulatorResetFlag;

public:
	CLldHwIf(void);
	~CLldHwIf();

	void	DupBdCnxt(void *_in);
	void	DupBdCnxt_Step1(void *_in);
	void	DupBdCnxt_Step2(void *_in);
	void	DupBdCnxt_Step3(void *_in);
	void	DupBdCnxt_Step4(void *_in);
	void	DupBdCnxt_Step5(void *_in);
	void	InitMutex(void);
	void	DestroyMutex(void);
	void	LockHwMutex(void);
	void	UnlockHwMutex(void);
	int	OpenDmaMutex(void);
	void	CloseDmaMutex(void);
	void	LockDmaMutex(void);
	void	UnlockDmaMutex(void);
	int	OpenEpldMutex(void);
	void	CloseEpldMutex(void);
	void	LockEpldMutex(void);
	void	UnlockEpldMutex(void);

	void	WaitMsec(unsigned long _msec);
	unsigned long	IoctlRdTvbPciConfReg(unsigned long address);
	void IoctlWrTvbPciConfReg(unsigned long address, unsigned long val);
	unsigned long RemapAddress(unsigned long address);
	unsigned long	WDM_Read_ModCmd_Indirectly(unsigned long cmd_adr);
	unsigned long	WDM_Read_TSP_Indirectly(unsigned long dwMemAddr);
	int	WDM_WR2SDRAM_ADDR(unsigned long dwAddress, unsigned long dwCommand);
	int	WDM_WR2SDRAM(unsigned long dwCommand);
	unsigned long	WDM_Read_TSP(unsigned long dwMemAddr);
	//2013/5/23 TVB599
	int Get_Board_HW_Feature(int num);
	int Check_HMCorOPEN_Mode(int *hmc833__, int *hmc1033__);		//0: HMC833, 1: HMC1033

#ifdef WIN32
#else
	unsigned long	WDM_Read_TSP_USB(unsigned long dwMemAddr);
#endif
	unsigned long	WDM_ReadFromSDRAM(void);
	int 	TSPL_SET_CONFIG_DOWNLOAD(long dwAddress, unsigned long dwData);
	int TSPL_GET_READ_PARAM(void);

 
	int _stdcall TSPL_GET_BOARD_REV(void);
	int _stdcall TSPL_GET_BOARD_ID(void);
	int _stdcall TSPL_GET_AUTHORIZATION(void);
	int _stdcall TSPL_GET_FPGA_INFO(int info);
	int _stdcall TSPL_GET_ENCRYPTED_SN(int type, char* sn);
	int _stdcall TSPL_WRITE_CONTROL_REG(int Is_PCI_Control, unsigned long address, unsigned long dwData);
	unsigned long _stdcall TSPL_READ_CONTROL_REG(int Is_PCI_Control, unsigned long address);
	int _stdcall TSPL_GET_LAST_ERROR(void);
	int _stdcall TSPL_GET_BOARD_CONFIG_STATUS(void);
#if defined(WIN32) || defined(TVB595V1)
	int _stdcall TSPL_SET_BOARD_LED_STATUS(int status_LED, int fault_LED);
	int _stdcall TSPL_REG_EVENT(void* pvoid);
	int _stdcall TSPL_SET_EVENT(void);
	int _stdcall TSPL_REG_COMPLETE_EVENT(void* pvoid);
	int _stdcall TSPL_SET_COMPLETE_EVENT(void);
#endif
#ifdef WIN32
	void usleep(long usec) ;
#else
#endif
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
	void ConfigurePEX8111(void);
#endif
	int CHK_ID( int id, ... );
	void	_stdcall ResetSdram(void);
	void	MakeRbfName(char *_fn, int _mod_typ);
	void	MakeRbfNameOld(char *_fn);

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#ifdef WIN32
#else	//	linux
#if defined(TVB595V1)
	int print_device(struct usb_device *dev, int level);
	void tvb595_usb_init(void);
	struct usb_device* tvb595_usb_ready(void);
	void tvb595_usb_close(usb_dev_handle *hDevice_usb);
	usb_dev_handle* tvb595_usb_open(void);
#else
#endif
#endif

//////////////////////////////////////////////////////////////////
	int IsAttachedBdTyp_AllCase_w_SpecialBd(void);
	int IsAttachedBdTyp_AllCase(void);		//	higher 390v7.0
	int IsAttachedBdTyp_HaveDmxCmtlTest(void);	//	higher 390v8.0
	int IsAttachedBdTyp_NewAmpBypassModeCntl(void);

	int IsAttachedBdTyp_UseAgcFreqTbl(void);
	int IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_1(void);
	int IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_2(void);
	int IsAttachedBdTyp_UseAgcFreqTbl_TblGrp_3_SubGrp(void);

	int IsAttachedBdTyp_WhyThisGrpNeed_13(void);
	int IsAttachedBdTyp_WhyThisGrpNeed_14(void);
	int IsAttachedBdTyp_WhyThisGrpNeed_15(void);

	int IsAttachedBdTyp_DiffCntlMethodAd9852Reg(void);
	int IsAttachedBdTyp_IndirectCntlMethodAd9852Reg(void);

	int IsAttachedBdTyp_UsbTyp(void);
	int IsAttachedBdTyp_UsbTyp_ExceptFor597v2(void);
	int IsAttachedBdTyp_UsbTyp_w_497(void);
	int IsAttachedBdTyp_UseAD9775(void);

	int IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin(void);	//	pll1 and pll2 use rowswin/umc combination
	int IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin__ImproveTdmbClk(void);
	int IsAttachedBdTyp_UseVcoPllN_UmcAndRowswin_WhatIsThis(void);
	int IsAttachedBdTyp_UseVcoPllN_UmcAndUmc(void);	//	pll1 and pll2 use umc/umc combination
	int IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1(void);
	int IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1(void);
	int IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_1_1_1(void);
	int IsAttachedBdTyp_UseVcoPllN_UmcAndUmc_SubCase_Pll2Cntl(void);
	int IsAttachedBdTyp_AD9852Clk_ASx(void);
	int IsAttachedBdTyp_AD9852Clk_ASx_2(void);
	int IsAttachedBdTyp_AD9852Clk_ASx_V(void);

	int IsAttachedBdTyp_ClkGenAdjForDvbS2(void);
	int IsAttachedBdTyp_ClkGenAdjForIsdbT13(void);
	int IsAttachedBdTyp_ClkGenAdjForQamA(void);

	int IsAttachedBdTyp_DacAdjAccordingToBaseBandFreq(void);

	int IsAttachedBdTyp_HaveBlkRstFunc(void);
	int IsAttachedBdTyp_DiffAlteraSizeIndicator(void);

	int IsAttachedBdTyp_380v3_ifrf_scaler(void);
	int IsAttachedBdTyp_380v3_upconverter(void);
	int IsAttachedBdTyp_390v6_Lowcost(void);
	int IsAttachedBdTyp_390v7_IF_only(void);
	int IsAttachedBdTyp_390v7(void);
	int IsAttachedBdTyp_390v8(void);
	int IsAttachedBdTyp_590v9(void);
	int IsAttachedBdTyp_590v10(void);
	int IsAttachedBdTyp_593(void);
	int IsAttachedBdTyp_595v2(void);
	int IsAttachedBdTyp_595v3(void);
	int IsAttachedBdTyp_595v4(void);
	int IsAttachedBdTyp_597(void);
	int IsAttachedBdTyp_597v2(void);
	int IsAttachedBdTyp_597_or_597v2(void);
	int IsAttachedBdTyp_497(void);
	int IsAttachedBdTyp_591(void);
//2012/1/31 TVB591S
	int IsAttachedBdTyp_591S(void);
	//2013/1/4 TVB499
	int IsAttachedBdTyp_499(void);
	//2013/5/27 TVB599
	int IsAttachedBdTyp_599(void);
	//2013/9/5 TVB598
	int IsAttachedBdTyp_598(void);
	//2013/11/22 TVB599/598 V2
	int IsAttachedBdTyp_599_598_ver2(void);

	int IsAttachedBdTyp_594(void);
	int IsAttachedBdTyp__Virtual(void);
	int IsAttachedBdTyp_SupportMultiRfOut(void);
	int IsAttachedBdTyp_SupportMultiRfOut_593_591s(void);
	int CntAdditionalVsbRfOut_593_591s(void);
	int CntAdditionalQamRfOut_593_591s(void);
	int IsAttachedBdTyp_593_or_497(void);
	int IsAttachedBdTyp_497_or_597v2(void);
	int IsAttachedBdTyp_590v10_or_590s(void);
	int IsAttachedBdTyp_590s_SmallAddrSpace(void);
	//2012/6/27 multi dvb-t
	int CntAdditionalDvbTRfOut_593(void);

	int IsAttachedBdTyp_NewAddrMap(void);
	int IsAttachedBdTyp_AssignNewMaxPlayrateCntlBits(void);
	int IsAttachedBdTyp_DdsIncNewEquation(void);

	int IsAttachedBdTyp_UseIndirectRegAccess(void);

	int IsAttachedBdTyp_WhyThisGrpNeed_4(void);
	int IsAttachedBdTyp_AttenTyp_1(void);
	int IsAttachedBdTyp_AttenTyp_2(void);
	int IsAttachedBdTyp_AttenTyp_3(void);
	int IsAttachedBdTyp_AttenTyp_4(void);

	int IsAttachedBdTyp_AttenTyp_AssignNewBitsPos(void);
	int IsAttachedBdTyp_595_Vx(void);
	int IsAttachedBdTyp_597_497(void);
	int IsAttachedBdTyp_590S_Ver2(void);
	//2012/4/12 SINGLE TONE
	int IsAttachedBdTyp_SingleTone(void);

	//2012/7/9 HMC833
	int IsAttachedBdTyp_591S_V2(void);
	int IsAttachedBdTyp_UseVcoPllN_Hmc833(void);
	//2012/8/27
	int IsAttachedBdTyp_NewRFLevel_Cntl(void);
	//2012/9/18 TVB593/597A V3
	int IsAttachedBdTyp_TVB593_597A_V3(void);
	//2013/3/22 TVB593 V4
	int IsAttachedBdTyp_TVB593_V4(void);
	//2013/5/23	TVB599
	int IsAttachedBdTyp_SupportBoardFeature(void);
	int IsAttachedBdTyp_SupportFastDownload(void);
	int IsAttachedBdTyp_597A_V4(void);

	//2013/7/17 Eeprom R/W
	int IsAttachedBdTyp_SupportEepromRW(void);
	int Read_Eeprom_Information(int *_dac_i_offset, int *_dac_q_offset, char *_license_number);
	void Write_LN_Eeprom(char *szLN);
};


#ifdef __cplusplus
}
#endif

#ifdef WIN32
#else

typedef usb_dev_handle*  WDU_DEVICE_HANDLE;
extern "C"	ULONG DispatchControl(WDU_DEVICE_HANDLE hDevice, void* SystemBuffer, int ControlCode);
extern "C"	void StartDevice(WDU_DEVICE_HANDLE hDevice);

extern	struct usb_dev_handle *hDevice_usb; //hDevice ==> hDevice_usb
extern "C"	void tvb595_usb_init(void);

#endif

#endif

