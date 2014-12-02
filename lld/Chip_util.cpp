//=================================================================	
//	Chip_util.c / Chip License Chip Access Utility for TVB370/380/390/590(E,S)/595/597A
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
#include	<windows.h>

#include	"logfile.h"
#include	"wdm_drv.h"
#include	"mainmode.h"

//=================================================================	
// Linux
#else

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <memory.h>
	
#include "../tsp100.h"
#include "../include/logfile.h"
#include "wdm_drv.h"
#include	"mainmode.h"
#endif
#include 	"Chip_util.h"

//=================================================================	
extern int 	TSPL_SET_CONFIG_DOWNLOAD(long addr, unsigned long data);
extern int	WDM_WR2SDRAM_ADDR(unsigned long dwAddress, unsigned long dwCommand);
//=================================================================	
#ifdef WIN32
extern "C" int _stdcall TVB380_IS_ENABLED_TYPE(long modulator_type);
//=================================================================	
// Linux
#else
extern int TVB380_IS_ENABLED_TYPE(long modulator_type);
#endif

//	extern	unsigned long	WDM_Read_TSP(unsigned long dwMemAddr);

/////////////////////////////////////////////////////////////////
CChipUtil::CChipUtil(void)
{
	cnt_multi_rfout_vsb_support	= 1;
	cnt_multi_rfout_qam_support = 1;
	//2012/6/28 multi dvb-t
	cnt_multi_rfout_dvbt_support = 1;
	//2014/3/26 multi dvb-s2
	cnt_multi_rfout_dvbs2_support = 1;
}
CChipUtil::~CChipUtil()
{
}

int CChipUtil::set_sync_modulator_type(int modulator_type)
{
	int	nDVB_T, n8VSB, nQAM_A, nQAM_B, nQPSK, nTDMB, n16VSB;
	int find_modulator_type = modulator_type;
	int	nDVB_H;
	int nDVB_S2, nISDB_T;
	int	nDTMB;
	int nCMMB;
	int nATSC_MH;
	int nDVB_T2, nDVB_C2;
	int nIQ_Play;
	//ISDB-S
	int nISDB_S;
	//2012/4/9
	int nMultiVsb;
	int nMultiQam;
	//2012/6/27 multi dvb-t
	int nMultiDvbT;

	nDVB_T = TVB380_IS_ENABLED_TYPE(TVB380_DVBT_MODE);
	n8VSB = TVB380_IS_ENABLED_TYPE(TVB380_VSB8_MODE);
	nQAM_A = TVB380_IS_ENABLED_TYPE(TVB380_QAMA_MODE);
	nQAM_B = TVB380_IS_ENABLED_TYPE(TVB380_QAMB_MODE);
	nQPSK = TVB380_IS_ENABLED_TYPE(TVB380_QPSK_MODE);
	nTDMB = TVB380_IS_ENABLED_TYPE(TVB380_TDMB_MODE);
	n16VSB = TVB380_IS_ENABLED_TYPE(TVB380_VSB16_MODE);
	nDVB_H = TVB380_IS_ENABLED_TYPE(TVB380_DVBH_MODE);
	nDVB_S2 = TVB380_IS_ENABLED_TYPE(TVB380_DVBS2_MODE);
	nISDB_T = TVB380_IS_ENABLED_TYPE(TVB380_ISDBT_MODE);
	nDTMB = TVB380_IS_ENABLED_TYPE(TVB380_DTMB_MODE);
	nCMMB = TVB380_IS_ENABLED_TYPE(TVB380_CMMB_MODE);
	nATSC_MH = TVB380_IS_ENABLED_TYPE(TVB380_ATSC_MH_MODE);
	nDVB_T2 = TVB380_IS_ENABLED_TYPE(TVB380_DVBT2_MODE);
	nDVB_C2 = TVB380_IS_ENABLED_TYPE(TVB380_DVBC2_MODE);
	nIQ_Play = TVB380_IS_ENABLED_TYPE(TVB380_IQ_PLAY_MODE);
	nISDB_S = TVB380_IS_ENABLED_TYPE(TVB380_ISDBS_MODE);
	nMultiVsb = TVB380_IS_ENABLED_TYPE(TVB380_MULTI_VSB_MODE);
	nMultiQam = TVB380_IS_ENABLED_TYPE(TVB380_MULTI_QAMB_MODE);
	//2012/6/27 multi dvb-t
	nMultiDvbT = TVB380_IS_ENABLED_TYPE(TVB380_MULTI_DVBT_MODE);

	if ( ((modulator_type == TVB380_DVBT_MODE && nDVB_T != 1)
		|| (modulator_type == TVB380_VSB8_MODE && n8VSB != 1)
		|| (modulator_type == TVB380_QAMA_MODE && nQAM_A != 1)
		|| (modulator_type == TVB380_QAMB_MODE && nQAM_B != 1)
		|| (modulator_type == TVB380_QPSK_MODE && nQPSK != 1)
		|| (modulator_type == TVB380_TDMB_MODE && nTDMB != 1))
		|| (modulator_type == TVB380_VSB16_MODE && n16VSB != 1)
		|| (modulator_type == TVB380_DVBH_MODE && nDVB_H != 1)
		|| (modulator_type == TVB380_DVBS2_MODE && nDVB_S2 != 1)
		|| (modulator_type == TVB380_ISDBT_MODE && nISDB_T != 1)
		|| (modulator_type == TVB380_DTMB_MODE && nDTMB != 1)
		|| (modulator_type == TVB380_CMMB_MODE && nCMMB != 1)
		|| (modulator_type == TVB380_ATSC_MH_MODE && nATSC_MH != 1)
		|| (modulator_type == TVB380_DVBT2_MODE && nDVB_T2 != 1)
		|| (modulator_type == TVB380_DVBC2_MODE && nDVB_C2 != 1)
		|| (modulator_type == TVB380_IQ_PLAY_MODE && nIQ_Play != 1)
		|| (modulator_type == TVB380_ISDBS_MODE && nISDB_S != 1)
		|| (modulator_type == TVB380_MULTI_VSB_MODE && nMultiVsb != 1)
		|| (modulator_type == TVB380_MULTI_QAMB_MODE && nMultiQam != 1)
		|| (modulator_type == TVB380_MULTI_DVBT_MODE && nMultiDvbT != 1) /* 2012/6/27 multi dvb-t */
		|| modulator_type == -1 )
	{
		if ( nDVB_T == 1 )			find_modulator_type = TVB380_DVBT_MODE;
		else if ( n8VSB == 1 )		find_modulator_type = TVB380_VSB8_MODE;
		else if ( nQAM_A == 1 ) 	find_modulator_type = TVB380_QAMA_MODE;
		else if ( nQAM_B == 1 ) 	find_modulator_type = TVB380_QAMB_MODE;
		else if ( nQPSK == 1 )		find_modulator_type = TVB380_QPSK_MODE;
		else if ( nTDMB == 1 )		find_modulator_type = TVB380_TDMB_MODE;
		else if ( n16VSB == 1 )		find_modulator_type = TVB380_VSB16_MODE;
		else if ( nDVB_H == 1 )		find_modulator_type = TVB380_DVBH_MODE;
		else if ( nDVB_S2 == 1 )	find_modulator_type = TVB380_DVBS2_MODE;
		else if ( nISDB_T == 1 )	find_modulator_type = TVB380_ISDBT_MODE;
		else if ( nDTMB == 1 )		find_modulator_type = TVB380_DTMB_MODE;
		else if ( nCMMB == 1 )		find_modulator_type = TVB380_CMMB_MODE;
		else if ( nATSC_MH == 1 )	find_modulator_type = TVB380_ATSC_MH_MODE;
		else if ( nDVB_T2 == 1 )	find_modulator_type = TVB380_DVBT2_MODE;
		else if ( nDVB_C2 == 1 )	find_modulator_type = TVB380_DVBC2_MODE;
		else if ( nIQ_Play == 1 )	find_modulator_type = TVB380_IQ_PLAY_MODE;
		//ISDB-S
		else if ( nISDB_S == 1 )	find_modulator_type = TVB380_ISDBS_MODE;
		else if ( nMultiVsb == 1 )	find_modulator_type = TVB380_MULTI_VSB_MODE;
		else if ( nMultiQam == 1 )	find_modulator_type = TVB380_MULTI_QAMB_MODE;
		//2012/6/27 multi dvb-t
		else if ( nMultiDvbT == 1 )	find_modulator_type = TVB380_MULTI_DVBT_MODE;
		else
		{
			find_modulator_type = -1;
			LldPrint_1("FAIL:Can not find modulator type", (int)modulator_type);

			//TVB593
			TLV_ControlErrorCode = TLV_FAIL_TO_FIND_MOD_TYPE;
		}
	}

	DBG_PRINT_MOD_TYP(modulator_type, find_modulator_type);

	return find_modulator_type;
}

int CChipUtil::check_Chip(void)
{
	DWORD Chip=0;
	Chip = WDM_Read_TSP(TSP_MEM_ADDR_Chip_STATUS_REG);
	LldPrint_1x("[LLD]===LICENSE, Status", (unsigned int)Chip);

	LldPrint_Trace("check_Chip", 0, 0, (double)0, NULL);
	//I/Q PLAY/CAPTURE
	TSPL_nFPGA_ID = ((Chip>>16)&0xFFFF);
	TSPL_nFPGA_VER = ((Chip>>8)&0xFF);

	//I/Q PLAY/CAPTURE
	DWORD nRet;
	nRet = WDM_Read_TSP(TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_CAP);
	TSPL_nFPGA_IQ_PLAY = ((nRet>>1) & 0x01);
	TSPL_nFPGA_IQ_CAPTURE = (nRet & 0x01);

	nRet = WDM_Read_TSP(TSP_MEM_ADDR_IQ_PLAY_CAPTURE_FPGA_BLD);
	TSPL_nFPGA_BLD = nRet & 0xFF;
	TSPL_nFPGA_RBF = (nRet >> 24) & 0xFF;

	DBG_PRINT_FPGA_CONF(TSPL_nFPGA_ID, TSPL_nFPGA_VER, TSPL_nFPGA_BLD, TSPL_nFPGA_RBF, TSPL_nFPGA_IQ_PLAY, TSPL_nFPGA_IQ_CAPTURE);

	return (Chip & 0x03);
}

int CChipUtil::read_Chip_SN(DWORD addr)
{
	DWORD SN=0;

	LldPrint_Trace("check_Chip", addr, 0, (double)0, NULL);
	SN = WDM_Read_TSP(addr);
	
	return SN;
}

int CChipUtil::read_Chip_encrypted_SN(DWORD addr)
{
	DWORD SN=0;
	LldPrint_FCall("read_Chip_encrypted_SN", addr, 0);
	SN = WDM_Read_TSP(addr);

	return SN;
}

int CChipUtil::write_encrypted_SN(DWORD SN, DWORD addr)
{
	LldPrint_FCall("write_encrypted_SN", addr, SN);
	if ( WDM_WR2SDRAM_ADDR(addr, SN) == 0 )
	{
		LldPrint_Error("sn-ln : pos...", 7, 1);
		return  -1;
	}
	else
		return 0;
}

int CChipUtil::read_modulator_permission()
{
	DWORD permission=0;
	permission = WDM_Read_TSP(TSP_MEM_ADDR_MOD_PERMISSION_REG);
	LldPrint_FCall("read_modulator_permission", permission, 0);
	LldPrint_1x("[LLD]===LICENSE, Permission", (int)permission);

	return permission;
}

int CChipUtil::download_license_data()
{
	int nRet = 0;
	unsigned long enc_sn_part;

	LldPrint_FCall("download-license-data", 0, 0);

	//23(TSPL_ENC_SN[0]) 0f 65 81(REG3)     e9 57 89 e6    13 ba e5 53    ad e4 3e 34(TSPL_ENC_SN[15])(REG0)
	enc_sn_part = (TSPL_ENC_SN[12] << 24) + (TSPL_ENC_SN[13] << 16) + (TSPL_ENC_SN[14] << 8) + TSPL_ENC_SN[15];
	if ( write_encrypted_SN(enc_sn_part, TSP_MEM_ADDR_LN_DATA_REG0) == -1 )
	{
		nRet = -5;
		LldPrint_Error("sn-ln : pos...", 7, 2);
		return nRet;
	}

	enc_sn_part = (TSPL_ENC_SN[8] << 24) + (TSPL_ENC_SN[9] << 16) + (TSPL_ENC_SN[10] << 8) + TSPL_ENC_SN[11];
	if ( write_encrypted_SN(enc_sn_part, TSP_MEM_ADDR_LN_DATA_REG1) == -1 )
	{
		nRet = -5;
		LldPrint_Error("sn-ln : pos...", 7, 3);
		return nRet;
	}

	enc_sn_part = (TSPL_ENC_SN[4] << 24) + (TSPL_ENC_SN[5] << 16) + (TSPL_ENC_SN[6] << 8) + TSPL_ENC_SN[7];
	if ( write_encrypted_SN(enc_sn_part, TSP_MEM_ADDR_LN_DATA_REG2) == -1 )
	{
		nRet = -5;
		LldPrint_Error("sn-ln : pos...", 7, 4);
		return nRet;
	}

	enc_sn_part = (TSPL_ENC_SN[0] << 24) + (TSPL_ENC_SN[1] << 16) + (TSPL_ENC_SN[2] << 8) + TSPL_ENC_SN[3];
	if ( write_encrypted_SN(enc_sn_part, TSP_MEM_ADDR_LN_DATA_REG3) == -1 )
	{
		nRet = -5;
		LldPrint_Error("sn-ln : pos...", 7, 5);
		return nRet;
	}
	
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_LN_DATA_CNTL, 0))
	{
		nRet = -6;
		LldPrint_Error("sn-ln : pos...", 7, 6);
		return nRet;
	}	
	Sleep(1);
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_LN_DATA_CNTL, 1))
	{
		nRet = -6;
		LldPrint_Error("sn-ln : pos...", 7, 7);
		return nRet;
	}	
	Sleep(1);
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_LN_DATA_CNTL, 0))
	{
		nRet = -6;
		LldPrint_Error("sn-ln : pos...", 7, 8);
		return nRet;
	}	
	
	return nRet;
}
unsigned long CChipUtil::DtaSnCtrlNewPath(void)
{
	unsigned long	ret;

	ret = (0x1 << 1);
	switch(TSPL_nBoardTypeID)
	{
	case _BD_ID_599__:
	case _BD_ID_598__:
		ret &= ~(0x1 << 1);
		break;
	case	_BD_ID_597v2__:
	case	_BD_ID_590v10_x__:
		if (TSPL_nBoardRevision >= __REV_5_0__sel_new_path_Chip__)
		{
			ret &= ~(0x1 << 1);
		}
		break;

	case	_BD_ID_593__:
	case	_BD_ID_590s__:
		if (TSPL_nBoardRevision >= __REV_4_0__sel_new_path_Chip__)
		{
			ret &= ~(0x1 << 1);
		}
		break;
	}
	return	ret;
}
int CChipUtil::read_encrypted_sn()
{
	int nRet = 0;

	LldPrint("[LLD]===LICENSE READ S/N");
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_Chip_SN_CNTL, 0 | DtaSnCtrlNewPath()))
	{
		nRet = -2;
		LldPrint_Error("sn-ln : pos...", 7, 9);
		return nRet;
	}	
	Sleep(1);
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_Chip_SN_CNTL, 1 | DtaSnCtrlNewPath()))
	{
		nRet = -2;
		LldPrint_Error("sn-ln : pos...", 7, 10);
		return nRet;
	}	
	Sleep(1);
	if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ADDR_Chip_SN_CNTL, 0 | DtaSnCtrlNewPath()))
	{
		nRet = -2;
		LldPrint_Error("sn-ln : pos...", 7, 11);
		return nRet;
	}	
	Sleep(150);

	nRet = check_Chip();
	if ( nRet != 0x03 )
	{
		if ( (nRet & 0x01) == 0 )
		{
			nRet = -3;
			LldPrint_Error("Fail to detect the precense of license chip!!!", 0, 0);
		}
		else if ( ((nRet>>1) & 0x01) == 0 )
		{
			nRet = -4;
			LldPrint_Error("CRC error!!!", 0, 0);
		}
		return nRet;
	}

	TSPL_nEncryptedSN[1] = read_Chip_encrypted_SN(TSP_MEM_ADDR_SN_DATA_REG1);
	TSPL_nEncryptedSN[0] = read_Chip_encrypted_SN(TSP_MEM_ADDR_SN_DATA_REG0);

	sprintf((char*)TSPL_EncryptedSN, "%08X%08X",TSPL_nEncryptedSN[1], TSPL_nEncryptedSN[0]);
	LldPrint("============================================================");
	LldPrint_s_s("EncryptedSN :", (char *)TSPL_EncryptedSN);
	LldPrint("============================================================");

	return nRet;
}
int CChipUtil::read_modulator_option2(char *szLN)
{
	//unsigned long enc_sn_part;
	//unsigned char buf[128];
	char *cp, ch;
	int by = 0, err = 0, src_len = 0, i;
	int nRet = 0;	
	DBG_PLATFORM_BRANCH();
	TSPL_EncryptedLN[0] = '\0';
	nRet = read_encrypted_sn();
	if ( nRet < 0 )
	{
		LldPrint("Fail to read the enc. sn...");
		return nRet;
	}
	cp = (char*)szLN;
	i = 0;
	while(i < 32 && *cp)
	{     
		ch = toupper(*cp++);
		if(ch >= '0' && ch <= '9')
			by = (by << 4) + ch - '0';
		else if(ch >= 'A' && ch <= 'F')
			by = (by << 4) + ch - 'A' + 10;
		else                    // error if not hexadecimal
		{
			break;
		}
		// store a byte for each pair of hexadecimal digits
		if(i++ & 1)
			TSPL_ENC_SN[i / 2 - 1] = by & 0xff;
	}
	src_len = i / 2;
	if ( src_len < 16 ) 
		return -1;
	nRet = download_license_data();
	if ( nRet < 0 )
	{
		LldPrint("Fail to download license data to FPGA...");
		return nRet;
	}
	Sleep(1);
	
	nRet = read_modulator_permission();
	TSPL_nModulatorEnabled = nRet;
	LldPrint_1x("Enabled Modulator Type", nRet);
	
	if ( nRet != 0 )
	{
		strncpy((char*)&TSPL_EncryptedLN[0], (char*)szLN, 32);
	}
	LldPrint_2("read_modulator_option", nRet, 0);
	nRet &= 0x00FFFFFF;	//	2bits-multi-dvbt|2bits-multi-vsb|2bits-multi-qam|isdbs|iqplay|dvbc2|...|dvb-t
	if (IsAttachedBdTyp_SupportMultiRfOut())
	{
		if (IsAttachedBdTyp_594())	//	special case. 594 is diff from the other bd.
		{
			cnt_multi_rfout_vsb_support = (nRet >> 16) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 17) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 18) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 19) & 0x1;
			cnt_multi_rfout_qam_support = 0;
			nRet &= 0x0000FFFF;
		}
#ifdef WIN32
		else if (IsAttachedBdTyp_593())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
			case	3:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
				break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
				cnt_multi_rfout_qam_support = 2;
				break;
			case	2:
				cnt_multi_rfout_qam_support = 3;
				break;
			case	3:
				cnt_multi_rfout_qam_support = 4;
				break;
			}
			//2012/6/28 multi dvb-t
			cnt_multi_rfout_dvbt_support = 1;
			switch((nRet >> 22) & 0x3)
			{
			case	0:
				cnt_multi_rfout_dvbt_support = 1;
				break;
			case	1:
			case	2:
			case	3:
				cnt_multi_rfout_dvbt_support = 2;
				break;
			}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			
			//nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if (IsAttachedBdTyp_598())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
			case	3:
				cnt_multi_rfout_vsb_support = 4;
				break;
			break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
				cnt_multi_rfout_qam_support = 2;
				break;
			case	2:
				cnt_multi_rfout_qam_support = 3;
				break;
			case	3:
				cnt_multi_rfout_qam_support = 4;
				break;
			}
			//2012/6/28 multi dvb-t
			cnt_multi_rfout_dvbt_support = 1;
			switch((nRet >> 22) & 0x3)
			{
			case	0:
				cnt_multi_rfout_dvbt_support = 1;
				break;
			case	1:
				cnt_multi_rfout_dvbt_support = 2;
				break;
			case	2:
				cnt_multi_rfout_dvbt_support = 3;
				break;
			case	3:
				cnt_multi_rfout_dvbt_support = 4;
				break;
			}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			//nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if (IsAttachedBdTyp_591S())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
				break;
			case	3:
				cnt_multi_rfout_vsb_support = 4;
				break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
			case	2:
			case	3:
				cnt_multi_rfout_qam_support = 2;
				break;
			}
			nRet &= 0x003FFFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if(IsAttachedBdTyp_597v2() || IsAttachedBdTyp_599())
		{
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
		}
#else
		nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
#endif
	}
	else
	{
		nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
	}

	return nRet;
}
/*
ret =	> 0 : Modulator types are enabled
		0	: All modulator types are disabled
		-1	: Fail to open a license file
		-2	: Fail to reset FSM
		-3	: Fail to detect the precense of a license chip 
		-4	: CRC error
		-5	: Fail to write LN0,1,2,3
		-6	: Fail to load LN

*/
//=================================================================	
#ifdef WIN32
int CChipUtil::read_modulator_option()
{
	//unsigned long enc_sn_part;
	unsigned char buf[128];
	char *cp, ch;
	int by = 0, err = 0, src_len = 0, i;
	int nRet = 0;	
	char	szCurrentLN[_MAX_PATH];
	FILE* fp;

	DBG_PLATFORM_BRANCH();
//	LldPrint_Trace("read_modulator_option", 0, 0, (double)0, NULL);
	sprintf(szCurrentLN, "%s\\%s", szCurDir, "./license.dat");
	LldPrint_s_s("[LLD]===LICENSE READ data file :", szCurrentLN);
#ifdef WIN32
	fopen_s(&fp, szCurrentLN, "rt");
#else
	fp = fopen(szCurrentLN, "rt");
#endif
	if ( fp == NULL )
	{
		nRet = -1;
		LldPrint("Fail to open the license data file!!!");

//=================================================================	
//Linux
#else

int CChipUtil::read_modulator_option()
{
	//unsigned long enc_sn_part;
	unsigned char buf[128];
	char *cp, ch;
	int by = 0, err = 0, src_len = 0, i;
	int nRet = 0;	

#ifdef	_DBG_ON_TARGET_TH4435__
	FILE* fp = fopen("/sysdb/license.dat", "rt");
#else
	FILE* fp = fopen("./license.dat", "rt");
#endif
	DBG_PLATFORM_BRANCH();
	if ( fp == NULL )
	{
		nRet = -1;
		LldPrint_Error("Fail to open the license data file!!!", 0, 0);

#endif

		/* read the enc. sn */
		read_encrypted_sn();

		return nRet;
	}
	TSPL_EncryptedLN[0] = '\0';
#ifdef WIN32
	nRet = read_encrypted_sn();
	if ( nRet < 0 )
	{
		LldPrint("Fail to read the enc. sn...");
		return nRet;
	}
	nRet = 0;
	while( !feof(fp) )
	{
		cp = NULL;
		fgets((char*)buf, 128, fp);
		cp = strstr((char*)buf, (char*)TSPL_EncryptedSN);
		if(cp != NULL)
		{
			cp = (char*)buf;
			i = 0;
			while(i < 32 && *cp)
			{     
				ch = toupper(*cp++);
				if(ch >= '0' && ch <= '9')
					by = (by << 4) + ch - '0';
				else if(ch >= 'A' && ch <= 'F')
					by = (by << 4) + ch - 'A' + 10;
				else                    // error if not hexadecimal
				{
					break;
				}
				// store a byte for each pair of hexadecimal digits
				if(i++ & 1)
					TSPL_ENC_SN[i / 2 - 1] = by & 0xff;
			}
			src_len = i / 2;
			if ( src_len < 16 ) 
				continue;
			nRet = download_license_data();
			if ( nRet < 0 )
			{
				LldPrint("Fail to download license data to FPGA...");
				fclose(fp);
				return nRet;
			}
			Sleep(1);
	
			nRet = read_modulator_permission();
			TSPL_nModulatorEnabled = nRet;
			LldPrint_1x("Enabled Modulator Type", nRet);
	
			if ( nRet != 0 )
			{
				strncpy((char*)&TSPL_EncryptedLN[0], (char*)buf, 32);
				if(IsAttachedBdTyp_SupportEepromRW())
					Write_LN_Eeprom((char *)TSPL_EncryptedLN);
				break;
			}
		}
	}
	fclose(fp);
#else
	while( !feof(fp) )
	{
		/* read license data */
		fgets((char*)buf, 128, fp);
		cp = (char*)buf;
		i = 0;
		while(i < 32 && *cp)
		{     
			ch = toupper(*cp++);
			if(ch >= '0' && ch <= '9')
				by = (by << 4) + ch - '0';
			else if(ch >= 'A' && ch <= 'F')
				by = (by << 4) + ch - 'A' + 10;
			else                    // error if not hexadecimal
			{
				break;
			}

			// store a byte for each pair of hexadecimal digits
			if(i++ & 1)
				TSPL_ENC_SN[i / 2 - 1] = by & 0xff;

			
		}

		/* read the enc. sn */
		nRet = read_encrypted_sn();
		if ( nRet < 0 )
		{
			LldPrint("Fail to read the enc. sn...");
			break;
		}

		src_len = i / 2;
		if ( src_len < 16 ) 
			continue;
	
		/* download license data to FPGA*/
		nRet = download_license_data();
		if ( nRet < 0 )
		{
			LldPrint("Fail to download license data to FPGA...");
			break;
		}
		Sleep(1);

		nRet = read_modulator_permission();
		TSPL_nModulatorEnabled = nRet;
		LldPrint_1x("Enabled Modulator Type", nRet);

		if ( nRet != 0 )
		{
//			nRet |= 0x04;//QAM_A enabled

			strncpy((char*)&TSPL_EncryptedLN[0], (char*)buf, 32);
			break;
		}
	}
	fclose(fp);
#endif
	LldPrint_2("read_modulator_option", nRet, 0);
	nRet &= 0x00FFFFFF;	//	2bits-multi-dvbt|2bits-multi-vsb|2bits-multi-qam|isdbs|iqplay|dvbc2|...|dvb-t
	if (IsAttachedBdTyp_SupportMultiRfOut())
	{
		if (IsAttachedBdTyp_594())	//	special case. 594 is diff from the other bd.
		{
			cnt_multi_rfout_vsb_support = (nRet >> 16) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 17) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 18) & 0x1;
			cnt_multi_rfout_vsb_support += (nRet >> 19) & 0x1;
			cnt_multi_rfout_qam_support = 0;
		nRet &= 0x0000FFFF;
		}
#ifdef WIN32
		else if (IsAttachedBdTyp_593())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
			case	3:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
				break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
				cnt_multi_rfout_qam_support = 2;
				break;
			case	2:
				cnt_multi_rfout_qam_support = 3;
				break;
			case	3:
				cnt_multi_rfout_qam_support = 4;
				break;
			}
			//2012/6/28 multi dvb-t
			cnt_multi_rfout_dvbt_support = 1;
			switch((nRet >> 22) & 0x3)
			{
			case	0:
				cnt_multi_rfout_dvbt_support = 1;
				break;
			case	1:
			case	2:
			case	3:
				cnt_multi_rfout_dvbt_support = 2;
				break;
			}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			//nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if (IsAttachedBdTyp_598())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
				break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
				cnt_multi_rfout_qam_support = 2;
				break;
			case	2:
				cnt_multi_rfout_qam_support = 3;
				break;
			case	3:
				cnt_multi_rfout_qam_support = 4;
				break;
			}
			//2012/6/28 multi dvb-t
			cnt_multi_rfout_dvbt_support = 1;
			switch((nRet >> 22) & 0x3)
			{
			case	0:
				cnt_multi_rfout_dvbt_support = 1;
				break;
			case	1:
				cnt_multi_rfout_dvbt_support = 2;
				break;
			case	2:
				cnt_multi_rfout_dvbt_support = 3;
				break;
			case	3:
				cnt_multi_rfout_dvbt_support = 4;
				break;
			}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			//nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if (IsAttachedBdTyp_591S())
		{
			cnt_multi_rfout_vsb_support = 1;
			switch((nRet >> 20) & 0x3)
			{
			case	0:
				cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
				break;
			case	1:
				cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
				break;
			case	2:
				cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
				break;
			case	3:
				cnt_multi_rfout_vsb_support = 4;
				break;
			}

			cnt_multi_rfout_qam_support = 1;
			switch((nRet >> 18) & 0x3)
			{
			case	0:
				cnt_multi_rfout_qam_support = 1;
				break;
			case	1:
			case	2:
			case	3:
				cnt_multi_rfout_qam_support = 2;
				break;
			}
			nRet &= 0x003FFFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
		}
		else if(IsAttachedBdTyp_597v2() || IsAttachedBdTyp_599())
		{
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
		}
#else
		nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
#endif
	}
	else
	{
		nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t
	}

	return nRet;
}

#ifdef WIN32
int _stdcall CChipUtil::TSPL_CHECK_LN(char* ln)
#else
int CChipUtil::TSPL_CHECK_LN(char* ln)
#endif
{
	//unsigned long enc_sn_part;
	unsigned char buf[128];
	char *cp, ch;
	int by = 0, err = 0, src_len = 0, i;
	int nRet = 0;	

	LldPrint_Trace("TSPL_CHECK_LN", 0, 0, (double)0, NULL);
	DBG_PLATFORM_BRANCH();
#if TVB597A_STANDALONE
	return 0xFFFFFFFF;
#endif
	
	TSPL_EncryptedLN[0] = '\0';
	{
		/* read license data */
		strcpy((char*)buf, ln);
			
		cp = (char*)buf;
		i = 0;
		while(i < 32 && *cp)
		{     
			ch = toupper(*cp++);
			if(ch >= '0' && ch <= '9')
				by = (by << 4) + ch - '0';
			else if(ch >= 'A' && ch <= 'F')
				by = (by << 4) + ch - 'A' + 10;
			else                    // error if not hexadecimal
			{
				break;
			}

			// store a byte for each pair of hexadecimal digits
			if(i++ & 1)
				TSPL_ENC_SN[i / 2 - 1] = by & 0xff;

			
		}
		src_len = i / 2;
		if ( src_len < 16 ) 
		{
			LldPrint_Error("sn-ln : pos...", 7, 12);
			return -1;
		}

		/* read the enc. sn */
		nRet = read_encrypted_sn();
		if ( nRet < 0 )
		{
			LldPrint_Error("sn-ln : pos...", 7, 13);
			return nRet;
		}
		
		/* download license data to FPGA*/
		nRet = download_license_data();
		if ( nRet < 0 )
		{
			LldPrint_Error("sn-ln : pos...", 7, 14);
			return nRet;
		}

		Sleep(1);

		nRet = read_modulator_permission();
		TSPL_nModulatorEnabled = nRet;

		if (IsAttachedBdTyp_SupportMultiRfOut())
		{
			if (IsAttachedBdTyp_594())	//	special case. 594 is diff from the other bd.
			{
				cnt_multi_rfout_vsb_support = (nRet >> 16) & 0x1;
				cnt_multi_rfout_vsb_support += (nRet >> 17) & 0x1;
				cnt_multi_rfout_vsb_support += (nRet >> 18) & 0x1;
				cnt_multi_rfout_vsb_support += (nRet >> 19) & 0x1;
				cnt_multi_rfout_qam_support = 0;
			}
#ifdef WIN32
			else if (IsAttachedBdTyp_593())
			{
				cnt_multi_rfout_vsb_support = 1;
				switch((nRet >> 20) & 0x3)
				{
				case	0:
					cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
					break;
				case	1:
					cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
					break;
				case	2:
				case	3:
					cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
					break;
				}

				cnt_multi_rfout_qam_support = 1;
				switch((nRet >> 18) & 0x3)
				{
				case	0:
					cnt_multi_rfout_qam_support = 1;
					break;
				case	1:
					cnt_multi_rfout_qam_support = 2;
					break;
				case	2:
					cnt_multi_rfout_qam_support = 3;
					break;
				case	3:
					cnt_multi_rfout_qam_support = 4;
					break;
				}
				//2012/6/28 multi dvb-t
				cnt_multi_rfout_dvbt_support = 1;
				switch((nRet >> 22) & 0x3)
				{
				case	0:
					cnt_multi_rfout_dvbt_support = 1;
					break;
				case	1:
				case	2:
				case	3:
					cnt_multi_rfout_dvbt_support = 2;
					break;
				}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			}
			else if (IsAttachedBdTyp_598())
			{
				cnt_multi_rfout_vsb_support = 1;
				switch((nRet >> 20) & 0x3)
				{
				case	0:
					cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
					break;
				case	1:
					cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
					break;
				case	2:
					cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
					break;
				case	3:
					cnt_multi_rfout_vsb_support = 4;	//	means that support two virtual-rf-out.
					break;
				}

				cnt_multi_rfout_qam_support = 1;
				switch((nRet >> 18) & 0x3)
				{
				case	0:
					cnt_multi_rfout_qam_support = 1;
					break;
				case	1:
					cnt_multi_rfout_qam_support = 2;
					break;
				case	2:
					cnt_multi_rfout_qam_support = 3;
					break;
				case	3:
					cnt_multi_rfout_qam_support = 4;
					break;
				}
				//2012/6/28 multi dvb-t
				cnt_multi_rfout_dvbt_support = 1;
				switch((nRet >> 22) & 0x3)
				{
				case	0:
					cnt_multi_rfout_dvbt_support = 1;
					break;
				case	1:
					cnt_multi_rfout_dvbt_support = 2;
					break;
				case	2:
					cnt_multi_rfout_dvbt_support = 3;
					break;
				case	3:
					cnt_multi_rfout_dvbt_support = 4;
					break;
				}
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			}
			else if (IsAttachedBdTyp_591S())
			{
				cnt_multi_rfout_vsb_support = 1;
				switch((nRet >> 20) & 0x3)
				{
				case	0:
					cnt_multi_rfout_vsb_support = 1;	//	means that support one rf-out(real-rf).
					break;
				case	1:
					cnt_multi_rfout_vsb_support = 2;	//	means that support one virtual-rf-out.
					break;
				case	2:
					cnt_multi_rfout_vsb_support = 3;	//	means that support two virtual-rf-out.
					break;
				case	3:
					cnt_multi_rfout_vsb_support = 4;
					break;
				}

				cnt_multi_rfout_qam_support = 1;
				switch((nRet >> 18) & 0x3)
				{
				case	0:
					cnt_multi_rfout_qam_support = 1;
					break;
				case	1:
				case	2:
				case	3:
					cnt_multi_rfout_qam_support = 2;
					break;
				}
			}
			else if(IsAttachedBdTyp_597v2() || IsAttachedBdTyp_599())
			{
				if((nRet >> 8) & 0x1)
					cnt_multi_rfout_dvbs2_support	= 4;
			}
#else
		nRet &= 0x0003FFFF; //	isdbs|iqplay|dvbc2|...|dvb-t

#endif
		}

		LldPrint_1x("Enabled Modulator Type", nRet);

		if ( nRet != 0 )
		{
//			nRet |= 0x04;//QAM_A enabled

			strncpy((char*)&TSPL_EncryptedLN[0], (char*)buf, 32);
		}
	}

	if (IsAttachedBdTyp_594())	//	special case. 594 is diff from the other bd.
	{
		nRet &= 0x0000FFFF;
	}

	return nRet;
}
int CChipUtil::TSPL_CNT_MULTI_VSB_RFOUT(void)
{
	return	cnt_multi_rfout_vsb_support;
}
int CChipUtil::TSPL_CNT_MULTI_QAM_RFOUT(void)
{
	return	cnt_multi_rfout_qam_support;
}
//2012/6/28 multi dvb-t
int CChipUtil::TSPL_CNT_MULTI_DVBT_RFOUT(void)
{
	return	cnt_multi_rfout_dvbt_support;
}

//2014/3/26 multi dvb-s2
int CChipUtil::TSPL_CNT_MULTI_DVBS2_RFOUT(void)
{
	return	cnt_multi_rfout_dvbs2_support;
}

