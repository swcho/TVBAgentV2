//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_Chip__UTIL_H_
#define	__TLV_Chip__UTIL_H_

#include		"sio_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CChipUtil	:	public	CSioDrv
{
private:
	int	cnt_multi_rfout_vsb_support;
	int	cnt_multi_rfout_qam_support;
	//2012/6/28 multi dvb-t
	int	cnt_multi_rfout_dvbt_support;
	//2014/3/26 multi dvb-s2
	int	cnt_multi_rfout_dvbs2_support;

public:
	CChipUtil(void);
	~CChipUtil();

	int set_sync_modulator_type(int modulator_type);
	int check_Chip(void);
	int read_Chip_SN(DWORD addr);
	int read_Chip_encrypted_SN(DWORD addr);
	int write_encrypted_SN(DWORD SN, DWORD addr);
	int read_modulator_permission();
	int download_license_data();
	unsigned long DtaSnCtrlNewPath(void);
	int read_encrypted_sn();
#ifdef WIN32
	int read_modulator_option();
#else
	int read_modulator_option();
#endif
	int read_modulator_option2(char *szLN);
#ifdef WIN32
	int _stdcall TSPL_CHECK_LN(char* ln);
#else
	int TSPL_CHECK_LN(char* ln);
#endif
	int	TSPL_CNT_MULTI_VSB_RFOUT(void);
	int	TSPL_CNT_MULTI_QAM_RFOUT(void);
	//2012/6/28 multi dvb-t
	int	TSPL_CNT_MULTI_DVBT_RFOUT(void);
	//2014/3/26 multi dvb-s2
	int	TSPL_CNT_MULTI_DVBS2_RFOUT(void);

};

#ifdef __cplusplus
}
#endif



#endif
