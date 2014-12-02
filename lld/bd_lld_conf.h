
#ifndef	_TLV_BD_LLD_CONF_H_
#define	_TLV_BD_LLD_CONF_H_

#ifdef WIN32
#else
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/ioctl.h>

#include "../libusb-0.1.12/usb.h"
#include "../libusb-0.1.12/usbi.h"
#endif

#include	"lld_regs.h"
#include	"dma_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CLldBdConf
{
private:
	int	my_id;
	void	*my_lld;
	_BD_CONF_CNXT	_dev_cnxt[__CNT_MAX_BD_INS_AVAILABLE_];
	int	verbose;
	int	multi_bd_service;
	int	detected_bd_cnt;

private:
#ifdef WIN32
	int	OpenDmaMutex(void);
	void	CloseDmaMutex(void);
#endif
	int	OpenEpldMutex(void);
	void	CloseEpldMutex(void);
	int	FreeRunningDevFromBdIndicatorFile(char *_dev_loc);
	int	UsingThisDev(char *_dev_loc);
	int	BuildDevName(char *dev_loc, int bus_num, int slot_num, char *_rslt);

	_BD_CONF_CNXT	*MyCnxt(void);
	int	IsTeleviewUsbDev(void *_dev);
	int	GetADevLoc_LnxUsb(
		void	*_dev,
		char	*_dev_loc,
		int	*_mem_if_method,
		int	*_597_v1_or_v2);
	int	GetADevLoc_LnxUsbGetADevLoc_LnxUsb(void *_dev, char *_dev_loc, int *_mem_if_method);
#ifdef WIN32
	int	GetADevicePlugIn_WinPciUsb(
		HANDLE	*_hnd,
		GUID	*_guid,
		unsigned long	_instance,
		char	*_dev_loc,
		int	*_mem_if_method,
		int	*_597_v1_or_v2);	//	using common func for pci and usb
#else
	int	GetADevicePlugIn_LnxPci(
		int *_hnd,
		unsigned long _instance,
		char *_dev_loc,
		int *_mem_if_method,
		int *_597_v1_or_v2);
#endif
	int	GetAllDevicePlugIn_LnxUsb(int from_instance);
	void	CloseDev_LnxUsb(void *_hdev);

	int	DetectBdAndPrepareHandle(int _multi_bd);
	int	InitATvbBd(int _my_num, void *_my_cnxt);
	int	InitATvbBd_WinLnx_PciUsb(int _r_d_scr, int _r_d_bd_typ, int _r_d_conf);
	int	CloseATvbBd(void);
	int	CloseATvbDev(void);

public:
	CLldBdConf(void *_my_lld);
	~CLldBdConf();

	int	TVBxxx_RdAllRealBdCnxt(void *_cnxt);
	int	TVBxxx_DETECT_BD_(int _multi_bd);
	int	TVBxxx_INIT_BD_(int _my_num, void *_my_cnxt);
	int	TVBxxx_RD_BD_CNXT(void *_cnxt);
	int	TVBxxx_DUP_BD_(int from_slot_num, int to_slot_num, void *_from_cnxt);
	int	TVBxxx_CLOSE_BD_(void);
	_BD_CONF_CNXT	*_MyCnxt(void);

};


#ifdef __cplusplus
}
#endif

#endif

