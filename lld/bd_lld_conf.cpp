
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
	
#include	"initguid.h"
#include	"GUIDs.h"
#include	"Ioctl.h"
#include	"logfile.h"
#include	"dll_error.h"
#include	"mainmode.h"
#include	"dma_drv.h"

#else	// Linux
#define _FILE_OFFSET_BITS 64
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<semaphore.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<memory.h>
#include	<math.h>
#include	<string.h>
#include	<stdarg.h>
	
#include 	"../tsp100.h"
#include	"dll_error.h"
#include 	"../include/logfile.h"
#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif

#include	"Reg590S.h"
#include	"Reg593.h"
#include	"Reg497.h"

#include 	"bd_init.h"
#include	"wdm_drv_wrapper.h"
/////////////////////////////////////////////////////////////////
#ifdef WIN32
#ifdef __cplusplus
extern "C"
{
#endif
#endif
extern void	InitMyCifBuf();
extern void	MyFreeFunc();
#ifdef WIN32
#ifdef __cplusplus
}
#endif
#endif
//////////////////////////////////////////////////////////////////
#ifdef WIN32
#else
//static	pthread_mutex_t	_tlv_mutex_initializer_ = PTHREAD_MUTEX_INITIALIZER;
#endif
static	char	*lux_pci_dev_name = "/dev/tvb380v4";
static	char	*fn_multi_bd_indicator	=	".\\info_bd_using.txt";
static	char	*fn_multi_bd_indicator_bak	=	".\\info_bd_using.bak";
static	char	*add_str_bd_indicator	=	":.rUnNInG";

/////////////////////////////////////////////////////////////////
CLldBdConf::CLldBdConf(void *_my_lld)
{
	my_id = 0;
	my_lld = _my_lld;
	memset(&_dev_cnxt[0], 0, sizeof(_BD_CONF_CNXT)*__CNT_MAX_BD_INS_AVAILABLE_);
	verbose = 1;
	for (int ind = 0; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		_dev_cnxt[ind].__id__ = -1;
		_dev_cnxt[ind].__id_my_phy__ = -1;
#ifdef WIN32
#else
		_dev_cnxt[ind]._dev_hnd = -1;
#endif
		_dev_cnxt[ind]._fpga_sdram_bnk_offset = 1024;
		_dev_cnxt[ind]._fpga_sdram_bnk_cnt = 7;
	}
	multi_bd_service = 1;
	detected_bd_cnt = 0;
}
CLldBdConf::~CLldBdConf()
{
}
_BD_CONF_CNXT	*CLldBdConf::MyCnxt(void)
{
	return	&_dev_cnxt[my_id];
}
#ifdef WIN32
int	CLldBdConf::OpenDmaMutex(void)
{
	SECURITY_ATTRIBUTES	s_attr;
	
	s_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	s_attr.lpSecurityDescriptor = NULL;
	s_attr.bInheritHandle = TRUE;
	_dev_cnxt[my_id]._dma_mutex_hnd = CreateMutex(&s_attr, 0, NULL);
	if (_dev_cnxt[my_id]._dma_mutex_hnd == NULL)
	{
		return	0;
	}
	return	1;
}
void	CLldBdConf::CloseDmaMutex(void)
{
	if(_dev_cnxt[my_id]._dma_mutex_hnd != NULL)
	{
		CloseHandle(_dev_cnxt[my_id]._dma_mutex_hnd);
	}
	_dev_cnxt[my_id]._dma_mutex_hnd = NULL;
}
#endif
int	CLldBdConf::OpenEpldMutex(void)
{
#ifdef WIN32
	SECURITY_ATTRIBUTES	s_attr;
	
	s_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	s_attr.lpSecurityDescriptor = NULL;
	s_attr.bInheritHandle = TRUE;
	_dev_cnxt[my_id]._epld_mutex_hnd = CreateMutex(&s_attr, 0, NULL);
	if (_dev_cnxt[my_id]._epld_mutex_hnd == NULL)
	{
		return	0;
	}
#endif
	return	1;
}
void	CLldBdConf::CloseEpldMutex(void)
{
#ifdef WIN32
	if(_dev_cnxt[my_id]._epld_mutex_hnd != NULL)
	{
		CloseHandle(_dev_cnxt[my_id]._epld_mutex_hnd);
	}
	_dev_cnxt[my_id]._epld_mutex_hnd = NULL;
#endif
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int	CLldBdConf::FreeRunningDevFromBdIndicatorFile(char *_dev_loc)
{
#if	0
	FILE	*_f = NULL, *_fw = NULL;
	char	_rd_buf[256], _wr_buf[256], *_add_pos;

	if (strcmp(_dev_loc, "remove.all") == 0)
	{
		remove(fn_multi_bd_indicator);
		return	0;
	}

	_f = fopen(fn_multi_bd_indicator, "r");
	if (_f == NULL) return	0;
	_fw = fopen(fn_multi_bd_indicator_bak, "w");
	if (_fw == NULL)
	{
		fclose(_f);
		return	0;
	}
	while(fgets(_rd_buf, 256, _f) != NULL)
	{
		_add_pos = strstr(_rd_buf, add_str_bd_indicator);
		if (_add_pos == NULL)		goto	_err_close_;
		*_add_pos = 0;
		if (strcmp(_rd_buf, _dev_loc) == 0)
		{
			continue;
		}
		sprintf(_wr_buf, "%s%s\n", _rd_buf, add_str_bd_indicator);
		fputs(_wr_buf, _fw);
	}
	fclose(_f);
	fclose(_fw);
	remove(fn_multi_bd_indicator);
	rename(fn_multi_bd_indicator_bak, fn_multi_bd_indicator);
	return	0;

_err_close_:
	fclose(_f);
	fclose(_fw);
#endif
	return	0;
}
int	CLldBdConf::UsingThisDev(char *_dev_loc)
{
#ifdef WIN32
	if (multi_bd_service)			return	0;
	if (_dev_loc[0] == 0)		return	1;
	if (detected_bd_cnt >= 1)	return	1;

	detected_bd_cnt++;
	return	0;
#else
	return	0;
#endif
#if	0
	FILE	*_fr = NULL, *_fw = NULL;
	char	_rd_buf[256], _wr_buf[256], found_dev, *_add_pos;

	if (multi_bd_service)			return	0;
	if (_dev_loc[0] == 0)		return	1;
	if (detected_bd_cnt >= 1)	return	1;

	_fr = fopen(fn_multi_bd_indicator, "r");
	if (_fr == NULL)	return	0;

	found_dev = 0;
	while(fgets(_rd_buf, 256, _fr) != NULL)
	{
		_add_pos = strstr(_rd_buf, add_str_bd_indicator);
		if (_add_pos == NULL)	goto	_assume_new_bd;
		*_add_pos = 0;
		if (strcmp(_rd_buf, _dev_loc) == 0)
		{
			found_dev = 1;
			break;
		}
	}
_assume_new_bd:
	fclose(_fr);
	if (found_dev)
	{
		return	1;
	}
	sprintf(_wr_buf, "%s%s\n", _dev_loc, add_str_bd_indicator);
	_fw = fopen(fn_multi_bd_indicator, "a");
	if (_fw == NULL)
		return	0;
	fputs(_wr_buf, _fw);
	fclose(_fw);
	detected_bd_cnt++;
	return	0;
#endif
}
int	CLldBdConf::BuildDevName(char *dev_loc, int bus_num, int slot_num, char *_rslt)
{
	sprintf(_rslt, "%s-%d-%d", dev_loc, bus_num, slot_num);
	return	1;
}

/////////////////////////////////////////////////////////////////
int	CLldBdConf::DetectBdAndPrepareHandle(int _multi_bd)
{
	int	ind, bd_seq_n, ret;
#ifdef WIN32	//	win32

	multi_bd_service = _multi_bd;
	bd_seq_n = __SLOT_ALLOC_FROM__;
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0391_GUID, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0594_GUID, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}

	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0595_GUID, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0595_GUID_1, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0595_GUID_2, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_WinPciUsb(&_dev_cnxt[bd_seq_n]._dev_hnd, (LPGUID)&TVB0595_GUID_3, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = NULL;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}

#else	//	lnx

	multi_bd_service = _multi_bd;
	bd_seq_n = __SLOT_ALLOC_FROM__;
	for (ind = __SLOT_ALLOC_FROM__; ind < __CNT_MAX_BD_INS_AVAILABLE_; ind++)
	{
		ret = GetADevicePlugIn_LnxPci(&_dev_cnxt[bd_seq_n]._dev_hnd, ind,
			_dev_cnxt[bd_seq_n]._location_name_string, &_dev_cnxt[bd_seq_n]._bd_typ_small_mem_space,
			&_dev_cnxt[bd_seq_n]._bd_typ_597_v1_v2_);
		if (ret <= 0)
		{
			_dev_cnxt[bd_seq_n]._dev_hnd = -1;
			continue;
		}
		_dev_cnxt[bd_seq_n].__iam_real__ = 1;
		bd_seq_n++;
	}
	bd_seq_n += GetAllDevicePlugIn_LnxUsb(bd_seq_n);
#endif
	return	bd_seq_n;
}
int	CLldBdConf::InitATvbBd(int _my_num, void *_my_cnxt)
{
	CBdInit	*lld;
	_BD_CONF_CNXT	*my_cxt;
	int	reg_dta_secur, reg_dta_bd_typ, reg_dta_conf;
	unsigned long 	ret, sts;
	KCMD_ARGS		KCmdInf;

	my_id = _my_num;
	memcpy(MyCnxt(), _my_cnxt, sizeof(_BD_CONF_CNXT));
	my_cxt = MyCnxt();

	lld = (CBdInit *)my_lld;
	lld->DupBdCnxt_Step1((void *)MyCnxt());

	InitMyCifBuf();

	if (my_cxt->_dev_hnd_usb != NULL)		//	lnx. usb type
	{
#ifdef WIN32
#else
		StartDevice((usb_dev_handle *)my_cxt->_dev_hnd_usb);

////////////////////////////////////////////////////////////////
		KCmdInf.dwCmdParm1 = 0x1C;	// 0x70 in byte address
		TLV_DeviceIoControl_usb((usb_dev_handle *)my_cxt->_dev_hnd_usb,
			IOCTL_READ_PCI9054_REG,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&ret, 0);
		sts = (unsigned long)(KCmdInf.dwCmdParm2);
		lld->LldPrint_1("[LLD]PCI9054REG:[0x70]", (int)sts);

////////////////////////////////////////////////////////////////
		reg_dta_secur = lld->WDM_Read_TSP_USB(TSP_BOARD_SECURITY);
		reg_dta_bd_typ = lld->WDM_Read_TSP_USB(TSP_BOARD_ID_ADDR);
		reg_dta_conf = lld->WDM_Read_TSP_USB(TSP_BOARD_CONFIG_STATUS);
#endif
	}
	else	//	win or lnx. pci typ
	{
		KCmdInf.dwCmdParm1 = 0x1C;		//	Test to read PCI Register. 0x70 in byte address
		TLV_DeviceIoControl(my_cxt->_dev_hnd,
			IOCTL_READ_PCI9054_REG,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&ret, 0);
		sts = (unsigned long)(KCmdInf.dwCmdParm2);
		lld->LldPrint_1("[LLD]PCI9054REG:[0x70]", (int)sts);

////////////////////////////////////////////////////////////////
		reg_dta_secur = lld->WDM_Read_TSP(TSP_BOARD_SECURITY);
		reg_dta_bd_typ = lld->WDM_Read_TSP(TSP_BOARD_ID_ADDR);
		reg_dta_conf = lld->WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
	}
	if (InitATvbBd_WinLnx_PciUsb(reg_dta_secur, reg_dta_bd_typ, reg_dta_conf) < 0)
	{
		CloseATvbDev();
		return	-1;
	}
	
////////////////////////////////////////////////////////////////	test interface
	ret = lld->WDM_Check_PCI_Status(0, 0);
	if (ret != TLV_NO_ERR)
	{
		lld->LldPrint_Error("[LLD]Fail to read PCI register", my_id, 0);
		CloseATvbDev();
		return	-1;
	}

	lld->DBG_PRINT_BD_CONF(my_id,
			my_cxt->_bd_typ_id,
			my_cxt->_bd_typ_small_mem_space,
			my_cxt->_bd_revision,
			my_cxt->_bd_use_AMP,
			my_cxt->_bd_revision_tvb595,
			my_cxt->_bd_revision_ext,
			my_cxt->__id__);

	return	TLV_NO_ERR;
}
int	CLldBdConf::InitATvbBd_WinLnx_PciUsb(int _r_d_scr, int _r_d_bd_typ, int _r_d_conf)
{
	DMA_ALLOC_ARGS  dma_alloc_args;
	_BD_CONF_CNXT	*my_cxt;
	unsigned long 	ret;
	CBdInit	*lld;
	int		reg_dta;

	reg_dta = -1;
	my_cxt = MyCnxt();
	lld = (CBdInit *)my_lld;

////////////////////////////////////////////////////////////////	Read board security
	my_cxt->_bd_authorization = _r_d_scr;
	my_cxt->_bd_authorization &= 0xFFFF;//Fault/Status trigger, 0xFFF -> 0xFFFF

////////////////////////////////////////////////////////////////	Read board ID and revision
	my_cxt->_bd_typ_id = _r_d_bd_typ;
	reg_dta = my_cxt->_bd_typ_id;
	lld->LldPrint_1x("[LLD]XBOARDID: found ",my_cxt->_bd_typ_id);
	my_cxt->_bd_revision = (my_cxt->_bd_typ_id >> 13) & 0x0f;
	my_cxt->_bd_typ_id = my_cxt->_bd_typ_id & 0xff;	// use 8 bit only

////////////////////////////////////////////////////////////////
	lld->DupBdCnxt_Step2((void *)MyCnxt());

////////////////////////////////////////////////////////////////	b'd specific configuration.
	if  (lld->IsAttachedBdTyp_595_Vx())
	{
		my_cxt->_bd_revision_tvb595 = my_cxt->_bd_revision;
	}
	else if  (lld->IsAttachedBdTyp_597() )
	{
		my_cxt->_bd_revision_ext = (reg_dta >> 16) & 0xFFFF;
		my_cxt->_bd_revision_tvb595 = __REV_595_v1_1;
		my_cxt->_bd_typ_small_mem_space = _ADDR_REMAP_METHOD_597_;
	}
	else if  (lld->IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_revision_ext = TVB590S_REV_EXT;
	}
	else if  (lld->IsAttachedBdTyp_593() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_revision_ext = TVB593_REV_EXT;
	}
	else if  (lld->IsAttachedBdTyp_597v2() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_revision_ext = _TVB597_REV_EXT;
		my_cxt->_bd_revision_tvb595 = __REV_595_v1_1;
		my_cxt->_bd_typ_small_mem_space = _ADDR_REMAP_METHOD_597v2_;
	}
	else if  (lld->IsAttachedBdTyp_599() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_revision_ext = _TVB597_REV_EXT;
		my_cxt->_bd_revision_tvb595 = __REV_595_v1_1;
		my_cxt->_bd_typ_small_mem_space = _ADDR_REMAP_METHOD_597v2_;
	}
	else if  (lld->IsAttachedBdTyp_598() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_revision_ext = TVB593_REV_EXT;
		my_cxt->_bd_typ_small_mem_space = _ADDR_REMAP_METHOD_597v2_;
	}
	else if  (lld->IsAttachedBdTyp_497() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
		my_cxt->_bd_option = (reg_dta >> 16) & 0x07;
	}
	else if  (lld->IsAttachedBdTyp_591() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
	}
	else if  (lld->IsAttachedBdTyp_594() )
	{
//		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
	}
	else if  (lld->IsAttachedBdTyp_591S() )
	{
		my_cxt->_bd_revision = (reg_dta >> 8) & 0xFF;
	}
	else	//	590c, ...
	{
	}

#ifdef WIN32	//	win32
	my_cxt->_bd_authorization &= 0xFFFF;//Fault/Status trigger, 0xFFF -> 0xFFFF
#else	//	lnx
	my_cxt->_bd_authorization &= 0xFFF;
#endif	//	win32/lnx

////////////////////////////////////////////////////////////////	TVB590V9
	my_cxt->_bd_config = _r_d_conf;
	my_cxt->_bd_use_AMP = ((my_cxt->_bd_config >> 6) & 0x03);

////////////////////////////////////////////////////////////////	init dma
////////////////////////////////////////////////////////////////	init dma
#ifdef WIN32
	if (OpenDmaMutex() == 0)
#else
	if (lld->OpenDmaMutex() == 0)
#endif
	{
		lld->LldPrint_Error("dma_drv : pos...", 3, 6);
		return	-1;
	}
	fDMA_Busy = 0;	//	what is this?
	OpenEpldMutex();

	my_cxt->_dma_buf_page_unmapped = NULL; //	1Mbyte buffer for DMA
	if (lld->IsAttachedBdTyp_UsbTyp())
	{
		my_cxt->_dma_buf_page_unmapped = (DWORD*)malloc(0x100000);
	}
	else
	{
#ifdef WIN32	//	win32
		TLV_DeviceIoControl(my_cxt->_dev_hnd,IOCTL_GET_DMA_MEMORY,
			NULL,0,&dma_alloc_args,sizeof(dma_alloc_args),&ret,0);
		my_cxt->_dma_buf_page_unmapped = dma_alloc_args.pdwLinearAddr;
#else
		TLV_DeviceIoControl(my_cxt->_dev_hnd,IOCTL_GET_DMA_MEMORY,
			&dma_alloc_args,sizeof(dma_alloc_args),&dma_alloc_args,sizeof(dma_alloc_args), &ret, 0);
		my_cxt->_dma_buf_page_unmapped = (DWORD*)malloc(0x100000);
#endif
	}
	if (lld->IsAttachedBdTyp_HaveDmxCmtlTest())	//	Demand Mode DMA //TVB593 0x0F //TVB497 _BD_ID_497__
	{
		lld->TSPL_SET_DMA_REG_INFO(0x20, 0x19C3);// 0x80 in byte address
//		DMAMODE_BUS32BIT | DMAMODE_ENA_READY | DMAMODE_ENA_LOC_BURST | 
//		DMAMODE_ENA_BTERM | DMAMODE_ENA_DEMAND_MODE | DMAMODE_CONST_LOC_ADDR);
		ret = lld->TSPL_GET_DMA_REG_INFO(0x20);
		lld->LldPrint_1("PCI9054REG:[0x80] = ",(int)ret);
	}

////////////////////////////////////////////////////////////////
	if (lld->IsAttachedBdTyp_UsbTyp())
	{
	}
	else
	{
#ifdef WIN32	//	win32
#if defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)	//	defined(TSPLLD0381_EXPORTS) || defined(TVB380) || defined(TVB380V4) || defined(TVB370V6)
		lld->ConfigurePEX8111();	//PCIe - PEX8111
#endif
#else	//	lnx
		lld->ConfigurePEX8111();	//PCIe - PEX8111
#endif
	}

////////////////////////////////////////////////////////////////
	lld->DupBdCnxt_Step3((void *)MyCnxt());

	return	TLV_NO_ERR;
}
int	CLldBdConf::CloseATvbBd(void)
{
	CBdInit	*lld;
	_BD_CONF_CNXT	*my_cxt;

	my_cxt = MyCnxt();
	lld = (CBdInit *)my_lld;

	FreeRunningDevFromBdIndicatorFile(my_cxt->_location_name_string);

	if (lld->IsAttachedBdTyp__Virtual())
	{
//		CloseF_LldPrint();
		return 0;
	}
	if((lld->IsAttachedBdTyp_599() || lld->IsAttachedBdTyp_598()) && lld->nNumOfLED == 1)
		lld->TSPL_SET_BOARD_LED_STATUS(1, 0);
	if(lld->gEnableTAT4710 == 1 && lld->ScanTAT4710())
	{
		lld->SetTAT4710(0);
		//lld->CloseTAT4710();
	}
	lld->TSPL_RESET_SDCON();

	MyFreeFunc();

	if (my_cxt->_dma_buf_page_unmapped != NULL )
	{
		if (lld->IsAttachedBdTyp_UsbTyp())
		{
			//free(my_cxt->_dma_buf_page_unmapped);
		}
		//	why dont you free malloc space in case of lnx-pci?
		my_cxt->_dma_buf_page_unmapped = NULL;
	}
#ifdef WIN32
	CloseDmaMutex();
#else
	lld->CloseDmaMutex();
#endif
	CloseEpldMutex();

//	CloseF_LldPrint();
	CloseATvbDev();

	return	TLV_NO_ERR;
}
int	CLldBdConf::CloseATvbDev(void)
{ 
	_BD_CONF_CNXT	*my_cxt;
	int	ret;
	CBdInit *lld;

	my_cxt = MyCnxt();
	lld = (CBdInit *)my_lld;

	if (lld->IsAttachedBdTyp__Virtual())
	{
		return	0;
	}
#ifdef WIN32
	if (my_cxt->_dev_hnd != NULL) 
	{
		TLV_DeviceIoControl(my_cxt->_dev_hnd, IOCTL_END_OF_APPLICATION, NULL, 0,NULL, 0,(unsigned long *)&ret,0);
		ret = CloseHandle(my_cxt->_dev_hnd);
		my_cxt->_dev_hnd = NULL;
	}
#else
	if (lld->IsAttachedBdTyp_UsbTyp())
	{
		CloseDev_LnxUsb(my_cxt->_dev_hnd_usb);
	}
	else
	{
		if (my_cxt->_dev_hnd >= 0)
		{
			close(my_cxt->_dev_hnd);
			my_cxt->_dev_hnd = -1;
		}
	}
#endif
	return	0;
}
#ifdef WIN32
int	CLldBdConf::GetADevicePlugIn_WinPciUsb(
	HANDLE	*_hnd,
	GUID	*_guid,
	unsigned long	_instance,
	char	*_dev_loc,
	int	*_mem_if_method,
	int	*_597_v1_or_v2)
{
	HDEVINFO	_hnd_info_dev;
	SP_INTERFACE_DEVICE_DATA			if_info_dta;
	PSP_INTERFACE_DEVICE_DETAIL_DATA	if_info_dta_detail;
	SP_DEVINFO_DATA						dev_info_data = {sizeof(SP_DEVINFO_DATA)};
	unsigned long			requred_length;
	unsigned long			predicted_len;
	int	bus_loc, slot_num;
	char	dev_loc[__MAX_NAME_STR_BD_LOCATION_];

	*_hnd = NULL;
	if_info_dta_detail = NULL;
		_hnd_info_dev = SetupDiGetClassDevs(_guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
		if (_hnd_info_dev == NULL)
		{
		return	-1;
	}
    
	if_info_dta.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
	if (SetupDiEnumDeviceInterfaces(_hnd_info_dev, 0, _guid, _instance, &if_info_dta)) 
	{
		requred_length = 0;
		SetupDiGetInterfaceDeviceDetail(_hnd_info_dev, &if_info_dta, NULL, 0, &requred_length, &dev_info_data);

		predicted_len = requred_length;
		if_info_dta_detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc (predicted_len);
		if_info_dta_detail->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);
		if (!SetupDiGetInterfaceDeviceDetail(
			_hnd_info_dev, &if_info_dta, if_info_dta_detail, predicted_len, &requred_length, NULL))
		{
//			LldPrint("Error in SetupDiGetInterfaceDeviceDetail");
			goto	_ret_err;
		}
    }
    else if (GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		goto	_ret_err;
    }
	if (if_info_dta_detail == NULL)	//	device not found
	{
		goto	_ret_err;
	}
	dev_loc[0] = 0;
	if (SetupDiGetDeviceRegistryProperty(
		_hnd_info_dev, &dev_info_data, SPDRP_LOCATION_INFORMATION, 0, (PBYTE)dev_loc, __MAX_NAME_STR_BD_LOCATION_, 0))
	{
//		goto	_ret_err;
	}
	slot_num = 0;
	if (SetupDiGetDeviceRegistryProperty(_hnd_info_dev, &dev_info_data, SPDRP_UI_NUMBER, NULL, (PBYTE)&slot_num, sizeof(slot_num), NULL))
	{
//		LldPrint_1("Board UI Number", TSPL_nBoardLocation);
	}
	bus_loc = 0;
	if (SetupDiGetDeviceRegistryProperty(_hnd_info_dev, &dev_info_data, SPDRP_BUSNUMBER, NULL, (PBYTE)&bus_loc, sizeof(bus_loc), NULL))
	{
//		LldPrint_1("Board BUS Number", TSPL_nBoardBUSnumber);
	}
	//2012/4/5
	if(strstr(if_info_dta_detail->DevicePath, "pid_597F") ||
		strstr(if_info_dta_detail->DevicePath, "pid_0597") ||
		strstr(if_info_dta_detail->DevicePath, "pid_4971") ||
		strstr(if_info_dta_detail->DevicePath, "pid_4991"))
	{
		char *pdest, *psrc, tmp[256];
		int pos;

		psrc = if_info_dta_detail->DevicePath;
		pdest = strstr(psrc, "#{");
		pos = pdest - psrc + 1;
		if ( pdest )
		{
			memset(tmp, 0, 256);
			strncpy(tmp, psrc, pos-1);
			pdest = strrchr(tmp, '&');
			if ( pdest )
			{
				pos = pdest - tmp + 1;
				slot_num = atoi(tmp+pos) % 4;
				if(slot_num == 0)
				{
					slot_num = 4;
				}
			}
		}
	}

	BuildDevName(dev_loc, bus_loc, slot_num, _dev_loc);

	*_597_v1_or_v2 = 0;
	*_mem_if_method = _ADDR_REMAP_METHOD_dont_;
//////////////////////////////////////////////////////	check bd-typ 595/597/497
	if (strstr(if_info_dta_detail->DevicePath, "pid_597F"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_597v2_;
		*_597_v1_or_v2 = 1;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "pid_0597"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_597_;
		*_597_v1_or_v2 = 1;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "pid_4971"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_497_;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "pid_4991"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_593_;
	}

//////////////////////////////////////////////////////	check pci bd-typ
	if (strstr(if_info_dta_detail->DevicePath, "dev_0598"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_590s_;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "dev_0593"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_593_;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "dev_4971"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_497_;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "dev_591A"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_591_;
	}
	else if (strstr(if_info_dta_detail->DevicePath, "dev_4991"))
	{
		*_mem_if_method = _ADDR_REMAP_METHOD_593_;
	}

//////////////////////////////////////////////////////
	switch(*_mem_if_method)
	{
	case	_ADDR_REMAP_METHOD_597_:	//	why???
		*_hnd = CreateFile(if_info_dta_detail->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_OFFLINE | FILE_FLAG_WRITE_THROUGH, NULL);
		break;
	default:
		*_hnd = CreateFile(if_info_dta_detail->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		break;
	}
	if (*_hnd == ((HANDLE)-1))
	{
		goto	_ret_err;
	}
	if (UsingThisDev(_dev_loc))
	{
		CloseHandle(*_hnd);
		*_hnd = NULL;
		*_597_v1_or_v2 = 0;
		*_mem_if_method = _ADDR_REMAP_METHOD_dont_;
		if (if_info_dta_detail != NULL) free(if_info_dta_detail);
		SetupDiDestroyDeviceInfoList(_hnd_info_dev);
		_dev_loc[0] = 0;
		return	0;
	}

	if (if_info_dta_detail != NULL)	free(if_info_dta_detail);
	SetupDiDestroyDeviceInfoList(_hnd_info_dev);
	return	1;

_ret_err:
	if (if_info_dta_detail != NULL)	free(if_info_dta_detail);
	SetupDiDestroyDeviceInfoList(_hnd_info_dev);

	return	-1;
}
#else
int	CLldBdConf::GetADevicePlugIn_LnxPci(
	int *_hnd,
	unsigned long _instance,
	char *_dev_loc,
	int *_mem_if_method,
	int *_597_v1_or_v2)
{
	KCMD_ARGS	KCmdInf;
	char		dev_name[32];
	unsigned long 	ret, red_sts;

	sprintf(dev_name,  "%s-%d", lux_pci_dev_name, _instance);
	if (UsingThisDev(dev_name))
	{
		_dev_loc[0] = 0;
		return	0;
	}
    if ((*_hnd = open(dev_name, O_RDONLY)) < 0)	//	why O_RDONLY?
    {
//		LldPrint_Error("[LLD] TSP-LLD TSP Driver : Open Error", nSlot, 0);
		return	-1;
    }

	KCmdInf.dwCmdParm1 = 0x00;	//	DEV_ID, 0x01=VEN_ID, 0x02=BUS, 0x03=SLOT, 0x04=FUNCTION
	ioctl(*_hnd, IOCTL_TEST_TSP, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &ret, 0);
	red_sts = (unsigned long)(KCmdInf.dwCmdParm2);
	*_597_v1_or_v2 = 0;
	switch(red_sts)
	{
	case	TVB590S_ID:
		*_mem_if_method = _ADDR_REMAP_METHOD_590s_;
		break;
	case	TVB593_ID:
		*_mem_if_method = _ADDR_REMAP_METHOD_593_;
		break;
	case	TVB597A_ID:
		*_mem_if_method = _ADDR_REMAP_METHOD_597_;
		*_597_v1_or_v2 = 1;
		break;
	case	__ID_BDs_MODEL_597v2__:
		*_mem_if_method = _ADDR_REMAP_METHOD_597v2_;
		*_597_v1_or_v2 = 1;
		break;
	default:
		*_mem_if_method = _ADDR_REMAP_METHOD_dont_;
		break;
	}

	KCmdInf.dwCmdParm1 = 0x01;
	ioctl(*_hnd, IOCTL_TEST_TSP, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &ret, 0);
	red_sts = (unsigned long)(KCmdInf.dwCmdParm2);

	KCmdInf.dwCmdParm1 = 0x02;
	ioctl(*_hnd, IOCTL_TEST_TSP, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &ret, 0);
	red_sts = (unsigned long)(KCmdInf.dwCmdParm2);

	KCmdInf.dwCmdParm1 = 0x03;
	ioctl(*_hnd, IOCTL_TEST_TSP, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &ret, 0);
	red_sts = (unsigned long)(KCmdInf.dwCmdParm2);

	KCmdInf.dwCmdParm1 = 0x04;
	ioctl(*_hnd, IOCTL_TEST_TSP, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &ret, 0);
	red_sts = (unsigned long)(KCmdInf.dwCmdParm2);

	strcpy(_dev_loc, dev_name);
	return	1;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////
void	CLldBdConf::CloseDev_LnxUsb(void *_hdev)
{
#ifdef WIN32
#else
	usb_dev_handle	*dev_hnd_usb;

	if (!_hdev) return;
	dev_hnd_usb = (usb_dev_handle *)_hdev;

	usb_close(dev_hnd_usb);
#endif
}
int	CLldBdConf::IsTeleviewUsbDev(void *_dev)
{
#ifdef WIN32
#else
	struct usb_device *dev;

	dev = (struct usb_device *)_dev;
	if (dev->descriptor.idVendor == VENDOR_ID &&
			(dev->descriptor.idProduct == TVB595_ID ||
			dev->descriptor.idProduct == __ID_BDs_MODEL_597v2__ ||
			dev->descriptor.idProduct == TVB597A_ID || 
			dev->descriptor.idProduct == TVB499_ID ||
			dev->descriptor.idProduct == TVB497_ID))
	{
		return	1;
	}
#endif
	return	0;
}
int	CLldBdConf::GetADevLoc_LnxUsb(
	void	*_dev,
	char	*_dev_loc,
	int	*_mem_if_method,
	int	*_597_v1_or_v2)
{
#ifdef WIN32
#else
	struct usb_device *dev;

	dev = (struct usb_device *)_dev;
	if (IsTeleviewUsbDev(dev))
	{
		*_597_v1_or_v2 = 0;
		sprintf(_dev_loc, "teleview_usb_%04X_%04X_%d_%s",
			dev->descriptor.idVendor, dev->descriptor.idProduct, dev->devnum, dev->bus->dirname);
	
		if ( dev->descriptor.idProduct == TVB597A_ID )
		{
			*_mem_if_method = _ADDR_REMAP_METHOD_597_;
			*_597_v1_or_v2 = 1;
		}
		else if ( dev->descriptor.idProduct == TVB497_ID )
		{
			*_mem_if_method = _ADDR_REMAP_METHOD_593_;
		}
		else if ( dev->descriptor.idProduct == __ID_BDs_MODEL_597v2__ )
		{
			*_mem_if_method = _ADDR_REMAP_METHOD_597v2_;
			*_597_v1_or_v2 = 1;
		}
		else if ( dev->descriptor.idProduct == TVB499_ID )
		{
			*_mem_if_method = _ADDR_REMAP_METHOD_593_;
		}
		else
		{
			*_mem_if_method = _ADDR_REMAP_METHOD_dont_;
		}
		return	1;
	}
#endif
	return	0;
}

int	CLldBdConf::GetAllDevicePlugIn_LnxUsb(int from_instance)
{
#ifdef WIN32
	return	0;
#else
	int	retval = 0;
	struct usb_bus	*bus = NULL;
	struct usb_device	*dev = NULL;
	usb_dev_handle	*dev_hnd_usb;
	int	instance;

	usb_init();	//	get the usb bus/device file path.

	usb_find_busses();	//	open bus file and make linked list of buses.
	usb_find_devices();	//	make linked list of devices.

	instance = from_instance;
	for (bus = usb_busses; bus; bus = bus->next)
	{
		if (bus->root_dev && !verbose)
		{
//			print_device(bus->root_dev, 0);
		}
		else 
		{
			for (dev = bus->devices; dev; dev = dev->next)
			{
//				print_device(dev, 0);
				if (GetADevLoc_LnxUsb((void *)dev, _dev_cnxt[instance]._location_name_string,
						&_dev_cnxt[instance]._bd_typ_small_mem_space,
						&_dev_cnxt[instance]._bd_typ_597_v1_v2_))
				{
					if (UsingThisDev(_dev_cnxt[instance]._location_name_string))
					{
						_dev_cnxt[instance]._location_name_string[0] = 0;
						continue;
					}
					dev_hnd_usb = usb_open(dev);
					if (dev_hnd_usb != NULL)
					{
						_dev_cnxt[instance]._dev_hnd_usb = (void *)dev_hnd_usb;
						_dev_cnxt[instance].__iam_real__ = 1;
						instance++;
					}
				}
			}
		}
	}
	return	(instance - from_instance);
#endif
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int	CLldBdConf::TVBxxx_RdAllRealBdCnxt(void *_cnxt)
{
	memcpy(_cnxt, &_dev_cnxt[0], sizeof(_BD_CONF_CNXT)*__CNT_MAX_BD_INS_AVAILABLE_);
	return	sizeof(_BD_CONF_CNXT)*__CNT_MAX_BD_INS_AVAILABLE_;
}
int	CLldBdConf::TVBxxx_DETECT_BD_(int _multi_bd)
{
	return	DetectBdAndPrepareHandle(_multi_bd);
}
int	CLldBdConf::TVBxxx_INIT_BD_(int _my_num, void *_my_cnxt)
{
	return	InitATvbBd(_my_num, _my_cnxt);
}
int	CLldBdConf::TVBxxx_RD_BD_CNXT(void *_cnxt)
{
	memcpy(_cnxt, MyCnxt(), sizeof(_BD_CONF_CNXT));
	return	sizeof(_BD_CONF_CNXT);
}
int	CLldBdConf::TVBxxx_DUP_BD_(int from_slot_num, int to_slot_num, void *_from_cnxt)
{
	memcpy(&_dev_cnxt[to_slot_num], _from_cnxt, sizeof(_BD_CONF_CNXT));
	my_id = to_slot_num;
	return	sizeof(_BD_CONF_CNXT);
}
int	CLldBdConf::TVBxxx_CLOSE_BD_(void)
{
	return	CloseATvbBd();
}
_BD_CONF_CNXT	*CLldBdConf::_MyCnxt(void)
{
	return	MyCnxt();
}

