
#ifndef	_TLV_BD_INIT_DEF_
#define	_TLV_BD_INIT_DEF_

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
#include 	"bd_lld_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_RBF_BUFF_SIZE	0x800000	//2013/1/4 TVB499 0x200000
#define  MAX_DMA_SIZE				0x100000
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CBdInit	:	public	CLldDmaDrv, public	CLldBdConf
{
private:

	unsigned char	szRBF_buff[MAX_RBF_BUFF_SIZE];
	unsigned int	snRBF_buff[MAX_RBF_BUFF_SIZE];
	int	TSPL_nFilter_type;

/////////////////////////////////////////////////////////////////

public:

private:
	int	Download_RRC_Filter(void);
	int	Download_INTERPOL_Filter(void);
	void	WrRrcFilter(void);

public:
	CBdInit(void);
	~CBdInit();

#ifdef WIN32
	int Write_EPLD(HANDLE hDevice, TCHAR *szPathRBF, TCHAR *rbf_name);
#else
	int Write_EPLD(char *szPathRBF, char *rbf_name);
#endif
	int	_stdcall TVB380_open(int nInitModulatorType, int nInitIF);

#if defined(WIN32) || defined(TVB595V1)
#ifdef WIN32
	int Write_EPLD_USB(HANDLE hDevice, TCHAR *szPathRBF, TCHAR *rbf_name);
#else
	int Write_EPLD_USB(char *szPathRBF, char *rbf_name);
#endif
#endif
	int	DownloadRbf(int nAltera_file_size);
	//2012/1/18 Fast download
	int	FastDownloadRbf(int nAltera_file_size);

	//2013/7/24 Test old board rbf down
	int	DownloadRbf_Test(int nAltera_file_size);
/////////////////////////////////////////////////////////////////
	int	_stdcall TVBxxx_DETECT_BD(int _multi_bd);
	int	_stdcall TVBxxx_INIT_BD(int _my_num, void *_my_cxt);
	int	_stdcall TVBxxx_GET_CNXT_ALL_BD(void *_cxt);
	int	_stdcall TVBxxx_GET_CNXT_MINE(void *_cxt);
	int	_stdcall TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *_from_cnxt);
	int	_stdcall TVBxxx_CLOSE_BD(void);

/////////////////////////////////////////////////////////////////
	int _stdcall TVB380_SET_CONFIG(long modulator_type, long IF_Frequency);

};


#ifdef __cplusplus
}
#endif

#endif

