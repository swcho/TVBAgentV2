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
#include	"GUIDs.h"
#include	"Ioctl.h"
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
#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif

#include	"Reg590S.h"
#include	"Reg593.h"
#include	"Reg497.h"

#include 	"bd_init.h"
#include	"wdm_drv_wrapper.h"

#if defined(WIN32) || defined(TVB595V1)
#ifdef WIN32 
#define TLV_RBF_DOWNLOAD_USB TLV_DeviceIoControl(hDevice, IOCTL_TVB350_EPLD_DOWN, &KFileInf, sizeof(KFileInf), NULL, 0, &dwRet, 0);
#else 
#define TLV_RBF_DOWNLOAD_USB TLV_DeviceIoControl_usb(hDevice_usb, IOCTL_TVB350_EPLD_DOWN, &KFileInf, sizeof(KFileInf), NULL, 0, &dwRet, 0);
#endif
#endif

//=================================================================	
// 
#define CHECK_CONFIG_STATUS_COUNT	1000

//=================================================================	
//
/////////////////////////////////////////////////////////////////
CBdInit::CBdInit(void)	:	CLldBdConf((void *)this)
{
	TSPL_nFilter_type = 0;
}
CBdInit::~CBdInit()
{
}

//=================================================================	
int	CBdInit::Download_RRC_Filter(void)
{
	char *filter_name = "filter_control_I_rrc.dat";
	int filter_control[60], temp_control= 0;

	int i;
	FILE *fp;

	// I filter
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	for(i=0; i < 60; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_RRC_START + 0x800 + (i/12)*0x40+(i%12)*4) >> 2, //in WORD unit
			filter_control[i]);
	}
	fclose(fp);

	// Q filter
	filter_name = "filter_control_Q_rrc.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	for(i=0; i < 60; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_RRC_START + 0xA00 + (i/12)*0x40+(i%12)*4) >> 2, //in WORD unit
			filter_control[i]);
	}
	fclose(fp);
	return 0;
}
int	CBdInit::Download_INTERPOL_Filter(void)
{
	char *filter_name = "filter_control_I_1st_interpol.dat";
	char szfilter_type[100];
	int filter_control[32], temp_control= 0;
	int i;
	FILE *fp;

	TSPL_nFilter_type = 0;

	// 1st
	// I filter
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;
	
	fgets(szfilter_type, 100, fp);
	if ( strstr(szfilter_type, "SYMMETRY=ODD") != NULL )
		TSPL_nFilter_type += 1;
	else if ( strstr(szfilter_type, "SYMMETRY=EVEN") != NULL )
		TSPL_nFilter_type += 2;

	//sskim20070612 - 3stream
	//for(i=0; i < 18; i++)
	for(i=0; i < 20; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_1ST_START + 0x1000 + (i/4)*0x40+(i%4)*4) >> 2, //in WORD unit
			filter_control[i]);
	}
	fclose(fp);

	// Q filter
	filter_name = "filter_control_Q_1st_interpol.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	fgets(szfilter_type, 100, fp);

	//sskim20070612 - 3stream
	//for(i=0; i < 18; i++)
	for(i=0; i < 20; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_1ST_START + 0x1200 + (i/4)*0x40+(i%4)*4) >> 2, //in WORD unit 
			filter_control[i]);
	}
	fclose(fp);

	//sskim20070612 - 3stream
	return 0;
	
	// 2nd
	// I filter
	filter_name = "filter_control_I_2nd_interpol.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	fgets(szfilter_type, 100, fp);
	if ( strstr(szfilter_type, "SYMMETRY=ODD") != NULL )
		TSPL_nFilter_type += 4;
	else if ( strstr(szfilter_type, "SYMMETRY=EVEN") != NULL )
		TSPL_nFilter_type += 8;

	for(i=0; i < 9; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_2ND_START + 0x1800 + (i/3)*0x40+(i%3)*4) >> 2, //in WORD unit 
			filter_control[i]);
	}
	fclose(fp);

	// Q filter
	filter_name = "filter_control_Q_2nd_interpol.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	fgets(szfilter_type, 100, fp);

	for(i=0; i < 9; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_2ND_START + 0x18C0 + (i/3)*0x40+(i%3)*4) >> 2, //in WORD unit 
			filter_control[i]);
	}
	fclose(fp);

	// 3rd
	// I filter
	filter_name = "filter_control_I_3rd_interpol.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	fgets(szfilter_type, 100, fp);
	if ( strstr(szfilter_type, "SYMMETRY=ODD") != NULL )
		TSPL_nFilter_type += 16;
	else if ( strstr(szfilter_type, "SYMMETRY=EVEN") != NULL )
		TSPL_nFilter_type += 32;

	for(i=0; i < 4; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_3RD_START + 0x2000 + (i*0x40)) >> 2, //in WORD unit 
			filter_control[i]);
	}
	fclose(fp);

	// Q filter
	filter_name = "filter_control_Q_3rd_interpol.dat";
#ifdef WIN32
	fopen_s(&fp, filter_name, "r");
#else
	fp = fopen(filter_name, "r");
#endif
	if ( fp == NULL )
		return -1;

	fgets(szfilter_type, 100, fp);

	for(i=0; i < 4; i++)
	{
#ifdef WIN32
		fscanf_s(fp, "%d", &temp_control);
#else
		fscanf(fp, "%d", &temp_control);
#endif
		filter_control[i] = temp_control;
		filter_control[i] &= 0xFFFF;

		WDM_WR2SDRAM_ADDR((TSP_MEM_ADDR_INTERP_3RD_START + 0x2100 + (i*0x40)) >> 2, //in WORD unit 
			filter_control[i]);
	}
	fclose(fp);

	return 0;
}
void CBdInit::WrRrcFilter(void)
{
	unsigned long 	dwStatus;

	if (!IsAttachedBdTyp_594())
	{
		return;
	}

	dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000001);
	dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_RESET_CNTL, 0x00000000);

	Sleep(1);
	
///////////////////////////////////////////////////////////////	Download filters
	Download_RRC_Filter();
	Download_INTERPOL_Filter();
	dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_FILTER_TYPE_CNTL, TSPL_nFilter_type);
}
/*^^***************************************************************************
 * Description : Access(Download) F/W
 *				
 * Entry : hDevice, szPathRBF, rbf_name
 *
 * Return: non zero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
#ifdef WIN32
int CBdInit::Write_EPLD(HANDLE hDevice, TCHAR *szPathRBF, TCHAR *rbf_name)
{
	KALTERA_RBF		KAlteraRbf;
	unsigned long 			dwRet;
	unsigned long 			dwStatus;
	int		nAltera_file_size;
	int		hf_RFF_File;
	KCMD_ARGS	KRegInf;

//	FILE		*hf_RFF_File;
	char	szCurrentRBF[_MAX_PATH];
	int FastDN_Flag = 0;
	

//	LldPrint_FCall(rbf_name, 0, 0);

	LldPrint("[LLD]===FPGA DOWNLOAD");

	if (hDevice == NULL)
	{
		LldPrint_Error("bd-init : pos...", 4, 1);
		return TLV_NO_DRIVER;
	}
	//FastDN_Flag = 0;
	if(!IsAttachedBdTyp_SupportFastDownload())
	{
		FastDN_Flag = 0;
	}
	else
	{
		dwStatus = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);
		if(((dwStatus >> 5) & 0x1) == 1)
			FastDN_Flag = 1;
	}
	if(FastDN_Flag == 1)
	{
		KRegInf.dwCmdParm1 = 0;	// 0~0x3f. from PCI-CNTRL 0x6c
		KRegInf.dwCmdParm2 = 0;	// 32 bit value

		TLV_DeviceIoControl(hDevice,
			   IOCTL_GET_FAST_DOWN,
			   &KRegInf,
			   sizeof(KRegInf),
			   &KRegInf,
			   sizeof(KRegInf),
			   &dwRet,0);
		if(KRegInf.dwCmdParm2 != 1 && IsAttachedBdTyp_UsbTyp_w_497())
			FastDN_Flag = 0;
	}
__RETRY_RBF_DOWNLOAD_:	
	if ( IsAttachedBdTyp_UsbTyp_w_497() && FastDN_Flag == 0)
	{
		//return TLV_NO_DRIVER;
		return Write_EPLD_USB(hDevice, szPathRBF, rbf_name);
	}
	
	//GetCurrentDirectory(_MAX_PATH, szCurDir);	// Save Current directory
	//SetCurrentDirectory(szPathRBF);
 
	sprintf(szCurrentRBF, "%s\\%s",szPathRBF,rbf_name);
#if	1
	hf_RFF_File = _open(szCurrentRBF, _O_BINARY);
	nAltera_file_size = (int) _filelengthi64(hf_RFF_File);	// Get file size
	//SetCurrentDirectory(szCurDir);	// Restore Current Directory
	if (hf_RFF_File == -1)
#else
	hf_RFF_File = fopen(szCurrentRBF, "rb");
	fseek(hf_RFF_File, 0L, SEEK_END);
	nAltera_file_size = (int) ftell(hf_RFF_File);	// Get file size
	fseek(hf_RFF_File, 0L, SEEK_SET);
	if (hf_RFF_File == NULL)
#endif
	{	
		TLV_ControlErrorCode = TLV_FAIL_TO_FIND_FW_FILE;

		// Fail to open Altera Configuration File
		LldPrint_Error("bd-init : pos...", 4, 2);
		return TLV_NO_RBF;
	}
	else if (nAltera_file_size > MAX_RBF_BUFF_SIZE)
	{
		LldPrint_Error("bd-init : pos...", 4, 3);
		return TLV_NO_RBF;
	}

#if	1
	if(FastDN_Flag == 1)
	{
		if(_read(hf_RFF_File, snRBF_buff, nAltera_file_size) != nAltera_file_size)	// Corrupted Altera Configuration file size
		{
			_close(hf_RFF_File);
			LldPrint_Error("bd-init : pos...", 4, 304);
			return TLV_ALTERA_FILE_READ_ERR;
		}
	}
	else
	{
		if(_read(hf_RFF_File, szRBF_buff, nAltera_file_size) != nAltera_file_size)	// Corrupted Altera Configuration file size
		{
			_close(hf_RFF_File);
			LldPrint_Error("bd-init : pos...", 4, 4);
			return TLV_ALTERA_FILE_READ_ERR;
		}
	}
	_close(hf_RFF_File);
#else
	if(fread(szRBF_buff, 1, nAltera_file_size, hf_RFF_File) != nAltera_file_size)	// Corrupted Altera Configuration file size
	{
		fclose(hf_RFF_File);
		LldPrint_Error("bd-init : pos...", 4, 4);
		return TLV_ALTERA_FILE_READ_ERR;
	}
	fclose(hf_RFF_File);
#endif
	//
	//	ConfigureEPLD(szRBF_buff[])
	//
	unsigned long dwStatus2;
#if 1
	if(FastDN_Flag == 1)
	{
		int _retry;
		if(IsAttachedBdTyp_497_or_597v2())
		{
			TSPL_SET_DMA_REG_INFO__(0x6, 0x42032643);
			dwStatus = WDM_WR2SDRAM_ADDR(PCI497_REG_CONTROL_REG, 0x00000000); 
		}
		else //if(IsAttachedBdTyp_497_or_597v2() == 0)
		{
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);	//0x18 Local Address Space 0
			dwStatus = dwStatus & 0xF6FFFF7F;			// bit 7, 24, 27 set 0
			TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
			dwStatus = 0;

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
			if(((dwStatus >> 27) & 0x1) != 0 || ((dwStatus >> 24) & 0x1) != 0 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus & 0xF6FFFF7F;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
					if(((dwStatus >> 27) & 0x1) == 0 && ((dwStatus >> 24) & 0x1) == 0 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 400);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#if 1

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);	//0x80 DMA register
			dwStatus = dwStatus & 0xFFFFEE7F;			// bit 7, 8, 12 set 0
			TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
			dwStatus = 0;

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
			if(((dwStatus >> 12) & 0x1) != 0 || ((dwStatus >> 8) & 0x1) != 0 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus & 0xFFFFEE7F;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
					if(((dwStatus >> 12) & 0x1) == 0 && ((dwStatus >> 8) & 0x1) == 0 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 401);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#endif


		}
		dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x3); //assert nconfig
		Sleep(1);
		if (dwStatus)
		{
			dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
			if(((dwStatus2 >> 3) & 0x3) == 0)
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x1);
			else
			{
				dwStatus = 0;
				//for 8 check Max 800ns delay
				for(int jj = 0; jj < 8; jj++)
				{
					dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
					if(((dwStatus2 >> 3) & 0x3) == 0)
					{
						dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x1);
						break;
					}
				}
				if(dwStatus == 0)
				{
					LldPrint_Error("bd-init : pos...", 4, 300);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
		}
		else 
		{
			LldPrint_Error("bd-init : pos...", 4, 106);
			return TLV_FAIL_TO_CONFIRE_EPLD;
		}
		Sleep(1);	// wait 100 msec for /status release
		dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);
		// nStatus check "1" retry 8
		if(((dwStatus2 >> 3) & 0x3) != 1)
		{
			dwStatus = 0;
			for(int jjj = 0; jjj < 8; jjj++)
			{
				dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
				if(((dwStatus2 >> 3) & 0x3) == 1)
				{
					dwStatus = 1;
					break;
				}
			}
			if(dwStatus == 0)
			{
				LldPrint_Error("bd-init : pos...", 4, 301);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		if(FastDownloadRbf(nAltera_file_size) == -1)
		{
			LldPrint_Error("bd-init : pos...", 4, 404);
			return TLV_FAIL_TO_CONFIRE_EPLD;
		}
#if 1
		if(IsAttachedBdTyp_497_or_597v2() == 0)
		{

#if 0
			KRegInf.dwCmdParm1 = 0;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_EXPRESS_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Express 0x0 ,", (int)KRegInf.dwCmdParm2);
			KRegInf.dwCmdParm1 = 0;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_LOCAL_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Local 0x0 ,", (int)KRegInf.dwCmdParm2);

			KRegInf.dwCmdParm1 = 0x11;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_EXPRESS_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Express 0x44 ,", (int)KRegInf.dwCmdParm2);
			KRegInf.dwCmdParm1 = 0x11;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_LOCAL_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Local 0x44 ,", (int)KRegInf.dwCmdParm2);
#endif
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);	//0x18 Local Address Space 0
			dwStatus = dwStatus | 0x9000000;			// bit 7, 24 set 0
			TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
			dwStatus = 0;
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
			if(((dwStatus >> 27) & 0x1) != 1 || ((dwStatus >> 24) & 0x1) != 1 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus | 0x9000000;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
					if(((dwStatus >> 24) & 0x1) == 1 && ((dwStatus >> 24) & 0x1) == 1 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 402);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);	//0x80 DMA register
			dwStatus = dwStatus | 0x1180;			// bit 7, 8, 12 set 1
			TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
			dwStatus = 0;
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
			if(((dwStatus >> 12) & 0x1) != 1 || ((dwStatus >> 8) & 0x1) != 1 || ((dwStatus >> 7) & 0x1) != 1)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus | 0x1180;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
					if(((dwStatus >> 12) & 0x1) == 1 && ((dwStatus >> 8) & 0x1) == 1 && ((dwStatus >> 7) & 0x1) == 1)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 403);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
		}
#endif
		WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x0);
		while(1)
		{
			Sleep(1);
			dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
			Sleep(1);
			if(((dwStatus >> 7) & 0x1) == 0)
				break;
			else
			{
				WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x0);
			}
		}
	}
	else
#else
	dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0); //assert nconfig
#endif
	{
		//TVB590S
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			dwStatus = WDM_WR2SDRAM_ADDR(PCI590S_REG_FPGA_CONFIG, PCI590S_CARD_CFG_START);
			Sleep(1);
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM_ADDR(PCI590S_REG_FPGA_CONFIG, PCI590S_CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 5);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		else if ( IsAttachedBdTyp_NewAddrMap() )
		{
			dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI593_CARD_CFG_START);
			Sleep(1);
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI593_CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 6);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		else
		{
			dwStatus = WDM_WR2SDRAM(CARD_CFG_START);

			Sleep(1);	// wait 1 msec
			
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM(CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 7);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		Sleep(100);	// wait 100 msec for /status release
		LldPrint_s_s("Downloading... : ",szCurrentRBF);
		//	Downloading... please Wait...
//2012/12/28
#if 1
		KAlteraRbf.pBuffer = szRBF_buff;
		KAlteraRbf.Size = (unsigned long) nAltera_file_size;
		if ( IsAttachedBdTyp_DiffAlteraSizeIndicator() || IsAttachedBdTyp_499())
		{
			//KAlteraRbf.Size += (1 << 31);
			KAlteraRbf.Size += 0x80000000;
		}
		dwStatus =	TLV_DeviceIoControl(hDevice,
					IOCTL_TSP100_EPLD_DOWN,
					&KAlteraRbf,
					sizeof(KAlteraRbf),
					NULL, 0,	&dwRet,0);
#else
		if(DownloadRbf_Test(nAltera_file_size) < 0)
			OutputDebugString("=============RBF DOWN FAIL=============\n");
#endif
//2012/12/28
		/* DEMUX BLOCK TEST : normal state */
		//if ( IsAttachedBdTyp_HaveDmxCmtlTest() )
		//{
			//sskim20080925 - TEST
			//Sleep(10);
		//	dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_DEMUX_CNTL_TEST, 0x00000000);
			//sskim20080925 - TEST
			//Sleep(10);
		//}
	}
#if 1
	Sleep(20);
	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
	{
		while(1)
		{
			dwStatus = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);
#if 1
			Sleep(1);
			if( ( ((dwStatus >> 3) & 0x1) == 0 || ((dwStatus >> 1) & 0x1) == 1 ) ) // Status or crc error
			{
				if(FastDN_Flag == 1)
				{
					TSPL_SET_DMA_REG_INFO__(0x6, 0x43032643);
					OutputDebugString("CRC ERROR====\n");
					if(((dwStatus >> 1) & 0x1) == 1 )
						LldPrint_Error("==== Fail fast download (CRC ERROR)===", 4, (dwStatus & 0xFFFFFFF));
					else
						LldPrint_Error("==== Fail fast download (NSTATUS 0)===", 4, (dwStatus & 0xFFFFFFF));
					FastDN_Flag = 0;
					Sleep(10);
					goto __RETRY_RBF_DOWNLOAD_;
				}
				else
				{
					LldPrint_Error("==== Fail RBF download ===", 4, (dwStatus & 0xFFFFFFF));
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#endif
			if(((dwStatus >> 4) & 0x1))
			{
				Sleep(1);
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI591S_CARD_DEV_OE);
				break;
			}
		}
		if(IsAttachedBdTyp_497_or_597v2() && FastDN_Flag == 1)
		{
			TSPL_SET_DMA_REG_INFO__(0x6, 0x43032643);
		}
	}
#endif
	//MessageBox(NULL,"Downloading...Finished", "TPG0590VC", MB_OK);
	LldPrint("Downloading...Finished");
	return 0;
}

//=================================================================	
// Linux
#else
int CBdInit::Write_EPLD(char *szPathRBF, char *rbf_name)
{
	KALTERA_RBF		KAlteraRbf;
	unsigned long 			dwRet;
	unsigned long 			dwStatus, dwStatus2;
	int		nAltera_file_size;
	FILE *hf_RFF_File;
	int FastDN_Flag = 0;
	KCMD_ARGS	KRegInf;

//	LldPrint_FCall(rbf_name, 0, 0);
	if(hDevice == NULL && hDevice_usb == NULL)
	{
		LldPrint_Error("bd-init : pos...", 4, 1);
		return TLV_NO_DRIVER;
	}
	if(!IsAttachedBdTyp_SupportFastDownload())
	{
		FastDN_Flag = 0;
	}
	else
	{
		dwStatus = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);
		if(((dwStatus >> 5) & 0x1) == 1)
			FastDN_Flag = 1;
	}

__RETRY_RBF_DOWNLOAD_:	
	if ( IsAttachedBdTyp_UsbTyp_w_497() && FastDN_Flag == 0)
	{
#if defined(TVB595V1)
		return Write_EPLD_USB(szPathRBF, rbf_name);
#endif
	}

	if (!(hf_RFF_File = fopen(rbf_name, "r"))) {
		LldPrint_Error("bd-init : pos...", 4, 9);
		return TLV_NO_RBF;
	}

	fseek(hf_RFF_File, 0L, SEEK_END);
	nAltera_file_size = (int) ftell(hf_RFF_File);	// Get file size

	if (nAltera_file_size > MAX_RBF_BUFF_SIZE)
	{
		LldPrint_Error("bd-init : pos...", 4, 10);
		return TLV_NO_RBF;
	}

	fseek(hf_RFF_File, 0L, SEEK_SET);
	if(FastDN_Flag == 1)
	{
		if(fread(snRBF_buff, 1, nAltera_file_size, hf_RFF_File) != nAltera_file_size) {
			// Corrupted Altera Configuration file size
			fclose(hf_RFF_File);
			LldPrint_Error("bd-init : pos...", 4, 11);
			return TLV_ALTERA_FILE_READ_ERR;
		}
	}
	else
	{
		if(fread(szRBF_buff, 1, nAltera_file_size, hf_RFF_File) != nAltera_file_size) {
			// Corrupted Altera Configuration file size
			fclose(hf_RFF_File);
			LldPrint_Error("bd-init : pos...", 4, 11);
			return TLV_ALTERA_FILE_READ_ERR;
		}
	}
	fclose(hf_RFF_File);

#if 1
	if(FastDN_Flag == 1)
	{
		int _retry;
		if(IsAttachedBdTyp_497_or_597v2())
		{
			TSPL_SET_DMA_REG_INFO__(0x6, 0x42032643);
			dwStatus = WDM_WR2SDRAM_ADDR(PCI497_REG_CONTROL_REG, 0x00000000); 
		}
		else //if(IsAttachedBdTyp_497_or_597v2() == 0)
		{
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);	//0x18 Local Address Space 0
			dwStatus = dwStatus & 0xF6FFFF7F;			// bit 7, 24, 27 set 0
			TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
			dwStatus = 0;

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
			if(((dwStatus >> 27) & 0x1) != 0 || ((dwStatus >> 24) & 0x1) != 0 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus & 0xF6FFFF7F;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
					if(((dwStatus >> 27) & 0x1) == 0 && ((dwStatus >> 24) & 0x1) == 0 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 400);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#if 1

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);	//0x80 DMA register
			dwStatus = dwStatus & 0xFFFFEE7F;			// bit 7, 8, 12 set 0
			TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
			dwStatus = 0;

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
			if(((dwStatus >> 12) & 0x1) != 0 || ((dwStatus >> 8) & 0x1) != 0 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus & 0xFFFFEE7F;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
					if(((dwStatus >> 12) & 0x1) == 0 && ((dwStatus >> 8) & 0x1) == 0 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 401);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#endif


		}
		dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x3); //assert nconfig
		Sleep(1);
		if (dwStatus)
		{
			dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
			if(((dwStatus2 >> 3) & 0x3) == 0)
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x1);
			else
			{
				dwStatus = 0;
				//for 8 check Max 800ns delay
				for(int jj = 0; jj < 8; jj++)
				{
					dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
					if(((dwStatus2 >> 3) & 0x3) == 0)
					{
						dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x1);
						break;
					}
				}
				if(dwStatus == 0)
				{
					LldPrint_Error("bd-init : pos...", 4, 300);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
		}
		else 
		{
			LldPrint_Error("bd-init : pos...", 4, 106);
			return TLV_FAIL_TO_CONFIRE_EPLD;
		}
		Sleep(1);	// wait 100 msec for /status release
		dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);
		// nStatus check "1" retry 8
		if(((dwStatus2 >> 3) & 0x3) != 1)
		{
			dwStatus = 0;
			for(int jjj = 0; jjj < 8; jjj++)
			{
				dwStatus2 = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);	//read status
				if(((dwStatus2 >> 3) & 0x3) == 1)
				{
					dwStatus = 1;
					break;
				}
			}
			if(dwStatus == 0)
			{
				LldPrint_Error("bd-init : pos...", 4, 301);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		if(FastDownloadRbf(nAltera_file_size) == -1)
		{
			LldPrint_Error("bd-init : pos...", 4, 404);
			return TLV_FAIL_TO_CONFIRE_EPLD;
		}
#if 1
		if(IsAttachedBdTyp_497_or_597v2() == 0)
		{

#if 0
			KRegInf.dwCmdParm1 = 0;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_EXPRESS_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Express 0x0 ,", (int)KRegInf.dwCmdParm2);
			KRegInf.dwCmdParm1 = 0;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_LOCAL_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Local 0x0 ,", (int)KRegInf.dwCmdParm2);

			KRegInf.dwCmdParm1 = 0x11;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_EXPRESS_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Express 0x44 ,", (int)KRegInf.dwCmdParm2);
			KRegInf.dwCmdParm1 = 0x11;	// 0~0x3f. from PCI-CNTRL 0x6c
			KRegInf.dwCmdParm2 = 0;	// 32 bit value
			TLV_DeviceIoControl(hDevice,
					IOCTL_READ_PEX8111_PCI_LOCAL_REG,
					&KRegInf,
					sizeof(KRegInf),
					&KRegInf,
					sizeof(KRegInf),
					&dwRet,0);
			LldPrint_1x("[LLD]===PEX8111 PCI-Local 0x44 ,", (int)KRegInf.dwCmdParm2);
#endif
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);	//0x18 Local Address Space 0
			dwStatus = dwStatus | 0x9000000;			// bit 7, 24 set 0
			TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
			dwStatus = 0;
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
			if(((dwStatus >> 27) & 0x1) != 1 || ((dwStatus >> 24) & 0x1) != 1 || ((dwStatus >> 7) & 0x1) != 0)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus | 0x9000000;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x6, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x6);
					if(((dwStatus >> 24) & 0x1) == 1 && ((dwStatus >> 24) & 0x1) == 1 && ((dwStatus >> 7) & 0x1) == 0)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 402);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}

			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);	//0x80 DMA register
			dwStatus = dwStatus | 0x1180;			// bit 7, 8, 12 set 1
			TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
			dwStatus = 0;
			dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
			if(((dwStatus >> 12) & 0x1) != 1 || ((dwStatus >> 8) & 0x1) != 1 || ((dwStatus >> 7) & 0x1) != 1)
			{
				for(_retry = 0; _retry < 9; _retry++)
				{
					dwStatus = dwStatus | 0x1180;			// bit 7, 24 set 0
					TSPL_SET_DMA_REG_INFO__(0x20, dwStatus);
					dwStatus = 0;
					dwStatus = TSPL_GET_DMA_REG_INFO__(0x20);
					if(((dwStatus >> 12) & 0x1) == 1 && ((dwStatus >> 8) & 0x1) == 1 && ((dwStatus >> 7) & 0x1) == 1)
						break;
				}
				if(_retry >= 9)
				{
					LldPrint_Error("bd-init : pos...", 4, 403);
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
		}
#endif
		WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x0);
		while(1)
		{
			Sleep(1);
			dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
			Sleep(1);
			if(((dwStatus >> 7) & 0x1) == 0)
				break;
			else
			{
				WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0x0);
			}
		}
	}
	else
#else
	dwStatus = WDM_WR2SDRAM_ADDR(PCI593_FAST_DOWNLOAD_CTRL, 0); //assert nconfig
#endif
	{
		//TVB590S
		if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
		{
			dwStatus = WDM_WR2SDRAM_ADDR(PCI590S_REG_FPGA_CONFIG, PCI590S_CARD_CFG_START);
			Sleep(1);
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM_ADDR(PCI590S_REG_FPGA_CONFIG, PCI590S_CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 5);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		else if ( IsAttachedBdTyp_NewAddrMap() )
		{
			dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI593_CARD_CFG_START);
			Sleep(10);
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI593_CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 6);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		else
		{
			dwStatus = WDM_WR2SDRAM(CARD_CFG_START);

			Sleep(10);	// wait 1 msec
			
			if (dwStatus)
			{
				dwStatus = WDM_WR2SDRAM(CARD_CFG_END);
			}
			else 
			{
				LldPrint_Error("bd-init : pos...", 4, 7);
				return TLV_FAIL_TO_CONFIRE_EPLD;
			}
		}
		Sleep(100);	// wait 100 msec for /status release
		//LldPrint_s_s("Downloading... : ",szCurrentRBF);
		//	Downloading... please Wait...
//2012/12/28
#if 1
		KAlteraRbf.pBuffer = szRBF_buff;
		KAlteraRbf.Size = (unsigned long) nAltera_file_size;
		if ( IsAttachedBdTyp_DiffAlteraSizeIndicator() || IsAttachedBdTyp_499())
		{
			//KAlteraRbf.Size += (1 << 31);
			KAlteraRbf.Size += 0x80000000;
		}
		dwStatus =	TLV_DeviceIoControl(hDevice,
					IOCTL_TSP100_EPLD_DOWN,
					&KAlteraRbf,
					sizeof(KAlteraRbf),
					NULL, 0,	&dwRet,0);
#else
		if(DownloadRbf_Test(nAltera_file_size) < 0)
			OutputDebugString("=============RBF DOWN FAIL=============\n");
#endif
//2012/12/28
		/* DEMUX BLOCK TEST : normal state */
		//if ( IsAttachedBdTyp_HaveDmxCmtlTest() )
		//{
			//sskim20080925 - TEST
			//Sleep(10);
		//	dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_DEMUX_CNTL_TEST, 0x00000000);
			//sskim20080925 - TEST
			//Sleep(10);
		//}
	}
#if 1
	Sleep(20);
	//2012/7/9 HMC833
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
	{
		while(1)
		{
			dwStatus = WDM_Read_TSP(PCI593_REG_CONFIGURE_STATUS_READ);
#if 1
printf("END Status: 0x%X\n", dwStatus);
			Sleep(1);
			if( ( ((dwStatus >> 3) & 0x1) == 0 || ((dwStatus >> 1) & 0x1) == 1 ) ) // Status or crc error
			{
				if(FastDN_Flag == 1)
				{
					TSPL_SET_DMA_REG_INFO__(0x6, 0x43032643);
					//OutputDebugString("CRC ERROR====\n");
					if(((dwStatus >> 1) & 0x1) == 1 )
						LldPrint_Error("==== Fail fast download (CRC ERROR)===", 4, (dwStatus & 0xFFFFFFF));
					else
						LldPrint_Error("==== Fail fast download (NSTATUS 0)===", 4, (dwStatus & 0xFFFFFFF));
					FastDN_Flag = 0;
					Sleep(10);
					goto __RETRY_RBF_DOWNLOAD_;
				}
				else
				{
					LldPrint_Error("==== Fail RBF download ===", 4, (dwStatus & 0xFFFFFFF));
					return TLV_FAIL_TO_CONFIRE_EPLD;
				}
			}
#endif
			if(((dwStatus >> 4) & 0x1))
			{
				Sleep(1);
				dwStatus = WDM_WR2SDRAM_ADDR(PCI593_REG_FPGA_CONFIG, PCI591S_CARD_DEV_OE);
				break;
			}
		}
		if(IsAttachedBdTyp_497_or_597v2() && FastDN_Flag == 1)
		{
			TSPL_SET_DMA_REG_INFO__(0x6, 0x43032643);
		}
	}
#endif
	//MessageBox(NULL,"Downloading...Finished", "TPG0590VC", MB_OK);
	LldPrint("Downloading...Finished");
	return 0;
}

#endif

/*^^***************************************************************************
 * Description : Configuration of the loaded device
 *				
 * Entry : nInitModulatorType, nInitIF(36000000 or 44000000)
 *
 * Return: non zero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int	_stdcall CBdInit::TVB380_open(int nInitModulatorType, int nInitIF)
{
	unsigned long 	dwStatus;
	char			szfnRbf[80];
	int				reg_data = -1;
	long			scale_factor = 0;
	int i_offset = 0;
	int q_offset = 0;
    char		szLicenseNum[64];
	int nFlag_readLN_from_Eeprom = -1;

	if(IsAttachedBdTyp_SupportEepromRW())
	{
		nFlag_readLN_from_Eeprom = Read_Eeprom_Information(&i_offset, &q_offset, szLicenseNum);
	}


	LldPrint_Trace("TVB380_open", nInitModulatorType, nInitIF, (double)0, NULL);
	if (IsAttachedBdTyp_594())
	{
		nInitModulatorType = TVB380_VSB8_MODE;	//	support vsb only.
	}

	if (IsAttachedBdTyp__Virtual())
	{
		TLV_ControlErrorCode = TLV_NO_FAIL;
		return TLV_NO_ERR;
	}
	if ( nInitModulatorType < TVB380_DVBT_MODE || nInitModulatorType >= TLV_MODULATOR_TYPE_MAX)
	{
		nInitModulatorType = TVB380_DVBT_MODE;
	}

	LldPrint_2("[LLD]===OPEN, Type, IF",nInitModulatorType, nInitIF);
	
	TSPL_nModulatorType = nInitModulatorType;
	gChannel_Freq_Offset = nInitIF;

//////////////////////////////////////////////////////////////
	if (IsAttachedBdTyp_390v6_Lowcost())	//	TVB370V6(VSB Only), No demand mode
	{
		TSPL_nModulatorType = TVB380_VSB8_MODE;
	}

//////////////////////////////////////////////////////////////	Confirm the selected Modulator Type is valid
	if ( IsAttachedBdTyp_AllCase() )
	{
	}
	else
	{
		TSPL_nModulatorType = ___set_sync_modulator_type(TSPL_nModulatorType);
		if ( TSPL_nModulatorType < TVB380_DVBT_MODE || TSPL_nModulatorType >= TLV_MODULATOR_TYPE_MAX)
		{
			LldPrint_Error("bd-init : pos...", 4, 18);
			return TLV_DOWNLOAD_ERR;
		}
	}

//////////////////////////////////////////////////////////////	Load the RBF for Modulator Type
__DOWNLOAD_RBF__ :

	dwStatus = WDM_Read_TSP(TSP_MEM_ADDR_MODULATOR_CONFIG);
	if ( ((dwStatus >> 7) & 0x01) == 1 && (int)((dwStatus >> 12) & 0xF) == TSPL_nModulatorType )
	{
	}
	
//=================================================================	
#ifdef WIN32

	GetCurrentDirectory(_MAX_PATH, szCurDir);// Save Current directory

	MakeRbfName(szfnRbf, TSPL_nModulatorType);
	time_t ltime0, ltime1;
	time(&ltime0);
	//LldPrint_2("Cpld_RW_Test()", 10, 1);
	//Cpld_RW_Test();
	if (Write_EPLD(hDevice, szCurDir, szfnRbf))
	{
		LldPrint_s_s("Fail to configure F/W with the file :",szfnRbf);
		//TVB593
		if ( TLV_ControlErrorCode != TLV_FAIL_TO_FIND_FW_FILE )
		{
			TLV_ControlErrorCode = TLV_FAIL_TO_DOWNLOAD_FW;
		}

		LldPrint_Error("bd-init : pos...", 4, 19);
		return TLV_DOWNLOAD_ERR;
	}
	//LldPrint_2("Cpld_RW_Test()", 10, 2);
	//Cpld_RW_Test();
	time(&ltime1);
	LldPrint("[HLD]Write_EPLD ");
	
	//////////////////////////////////////////////////////////////
	//	Check the RBF download is complete
	//
	//time(&ltime0);
	if ( IsAttachedBdTyp_AllCase_w_SpecialBd() )
	{
		dwStatus = WDM_Check_Board_Status(0, 0);
		if ( dwStatus == TLV_NO_ERR )
		{
		}
		else
		{

			if ( IsAttachedBdTyp_UsbTyp() )
			{
				LldPrint_s_s("Fail to configure F/W with the file :",szfnRbf);
				LldPrint("Try to download again...");
				if (Write_EPLD(hDevice, szCurDir, szfnRbf))
				{
					LldPrint_Error("bd-init : pos...", 4, 20);
					return -1;
				}
				if ( WDM_Check_Board_Status(0, 1) == TLV_NO_ERR )
					;
				else
				{
					LldPrint_Error("bd-init : pos...", 4, 21);
					TLV_ControlErrorCode = TLV_FAIL_TO_TEST_FW;
					return TLV_DOWNLOAD_ERR;
				}
			}
			else
			{
				LldPrint_Error("bd-init : pos...", 4, 22);
				TLV_ControlErrorCode = TLV_FAIL_TO_TEST_FW;
				return TLV_DOWNLOAD_ERR;
			}
		}
		ResetSdram();
	}

//=================================================================		//Linux
#else

	MakeRbfName(szfnRbf, TSPL_nModulatorType);
	if (Write_EPLD(NULL, szfnRbf))
	{
		LldPrint_Error("bd-init : pos...", 4, 23);
		return TLV_DOWNLOAD_ERR;
	}

	//////////////////////////////////////////////////////////////
	//	Check the RBF download is complete
	//
	if ( IsAttachedBdTyp_AllCase_w_SpecialBd() )
	{
		dwStatus = WDM_Check_Board_Status(0, 0);
		if ( dwStatus == TLV_NO_ERR )
		{
		}
		else
		{
			if ( IsAttachedBdTyp_UsbTyp() )
			{
				LldPrint_s_s("[LLD] Fail to configure F/W with the file :",szfnRbf);
				LldPrint("Try to download again...");
				if (Write_EPLD(NULL, szfnRbf))
				{
					LldPrint_Error("bd-init : pos...", 4, 24);
					TLV_ControlErrorCode = TLV_FAIL_TO_TEST_FW;
					return TLV_DOWNLOAD_ERR;
				}
				if ( WDM_Check_Board_Status(0, 1) == TLV_NO_ERR )
					;
				else
				{
					LldPrint_Error("bd-init : pos...", 4, 25);
					TLV_ControlErrorCode = TLV_FAIL_TO_TEST_FW;
					return TLV_DOWNLOAD_ERR;
				}
			}
			else
			{
				LldPrint_Error("bd-init : pos...", 4, 26);
				TLV_ControlErrorCode = TLV_FAIL_TO_TEST_FW;
				return TLV_DOWNLOAD_ERR;
			}
		}
		ResetSdram();
	}
	
#endif

//__SKIP__DOWNLOAD_RBF__:
	
	// Set default subbank size to 8 Mbyte SUB bank
	SetSDRAMBankConfig(3);

	WrRrcFilter();

	//////////////////////////////////////////////////////////////
	//	Get the enabled modulator types
	if ( IsAttachedBdTyp_AllCase() )
	{
		/* Read the license data from "./license.dat"
		Read the modulator option to be enabled
		*/
		if(nFlag_readLN_from_Eeprom == 0)
		{
			int nRet;
			nRet = read_modulator_option2(szLicenseNum);
			if(nRet < 0)
			{
				TSPL_nModulatorEnabled = ___read_modulator_option();
			}
			else
			{
				TSPL_nModulatorEnabled = nRet;
			}
		}
		else
			TSPL_nModulatorEnabled = ___read_modulator_option();
#if TVB597A_STANDALONE
		TSPL_nModulatorEnabled = 0xFFFFFFFF;
#endif
		//2013/5/27 SINGLE LED Control
		if((IsAttachedBdTyp_599() || IsAttachedBdTyp_598()) && nNumOfLED == 1)
		{
			TSPL_SET_BOARD_LED_STATUS(1, 1);
			TSPL_SET_BOARD_LED_STATUS(2, 1);
			TSPL_SET_BOARD_LED_STATUS(3, 2);
		}
		if ( TSPL_nModulatorEnabled > 0 )
		{
			reg_data = TSPL_nModulatorType;
			TSPL_nModulatorType = ___set_sync_modulator_type(TSPL_nModulatorType);
			if ( TSPL_nModulatorType >= TVB380_DVBT_MODE )
			{
				if ( reg_data != TSPL_nModulatorType )
				{
					goto __DOWNLOAD_RBF__;
				}
			}
		}
		else
		{
			if ( IsAttachedBdTyp_UsbTyp() )	//TVB59V1 - FAULT LED ON
			{
				if(!IsAttachedBdTyp_599() || nNumOfLED == 0)
				{
					WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 0 + (1<<1));
					if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
					{
						WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0 + 0 + (1<<1));
					}
				}
			}

			TLV_ControlErrorCode = TLV_FAIL_TO_CONFIRM_LN;
		}
	}

	//////////////////////////////////////////////////////////////
	//	PNP-1500-P22 : IF
	//
	reg_data = gChannel_Freq_Offset;
	dwStatus = Reset_TRF178(TSPL_nBoardTypeID, TSPL_nModulatorType, reg_data/* IF */);
	if ( dwStatus == -1 )
	{
		LldPrint_Error("bd-init : pos...", 4, 27);
		return TLV_DOWNLOAD_ERR;
	}
	LldPrint(" ");

	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		WDM_WR2SDRAM_ADDR(PCI590S_REG_MODULATION_OPTION, TSPL_nModulatorType & 0x1F);
	}
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		WDM_WR2SDRAM_ADDR(PCI593_REG_MODULATION_OPTION, TSPL_nModulatorType & 0xFF);
	}
	else
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, (0x80+TSPL_nModulatorType)&0xFF);
	}

	//TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAD9775() )
	{
		Default_Configure_AD9775(TSPL_nModulatorType, 0, 0);
	}
	if( IsAttachedBdTyp_499())
	{
		;
	}

	//TVB593, TVB497
	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		if ( TSPL_nModulatorType == TVB380_ISDBT_MODE )
		{
			scale_factor = 0xE00;
		}
		else if ( TSPL_nModulatorType == TVB380_ISDBT_13_MODE )
		{
			scale_factor = 0x800;
		}
		else if (TSPL_nModulatorType == TVB380_DVBT2_MODE )
		{
			scale_factor = 0x500;
		}
		//2011/6/3 DVB-C2
		else if(TSPL_nModulatorType == TVB380_DVBC2_MODE)
		{
			scale_factor = 0x600;
		}
		//ISDB-S
		else if ( TSPL_nModulatorType == TVB380_ISDBS_MODE )
		{
			scale_factor = 0x400;
		}

		if(IsAttachedBdTyp_591())
		{
			scale_factor = 0;
			if( TSPL_nModulatorType == TVB380_VSB8_MODE )
			{
				scale_factor = 0x350;
			}
			else if( TSPL_nModulatorType == TVB380_QAMB_MODE )
			{
				scale_factor = 0x260;
			}
			else if( TSPL_nModulatorType == TVB380_QAMA_MODE )
			{
				scale_factor = 0x260;	//initialize default value.
			}
			else if( TSPL_nModulatorType == TVB380_TDMB_MODE )
			{
				scale_factor = 0x390;
			}
			else if( TSPL_nModulatorType == TVB380_DVBT_MODE || TSPL_nModulatorType == TVB380_DVBH_MODE )
			{
				scale_factor = 0x350;
			}
			else if ( TSPL_nModulatorType == TVB380_ISDBT_MODE )
			{
				scale_factor = 0xE00;
			}
			else if ( TSPL_nModulatorType == TVB380_ISDBT_13_MODE )
			{
				scale_factor = 0x800;
			}

		}

		if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_SCALE_FACTOR, scale_factor) )
		{
			LldPrint_Error("bd-init : pos...", 4, 28);
			return -1;
		}
	}

	//ISDB-S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		if ( TSPL_nModulatorType == TVB380_ISDBS_MODE )
		{
			scale_factor = 0x400;
		}

		if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_SCALE_FACTOR, scale_factor) )
		{
			LldPrint_Error("bd-init : pos...", 4, 29);
			return -1;
		}
	}

	//TVB497
	if ( IsAttachedBdTyp_497() || IsAttachedBdTyp_499())
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
		LldPrint_1("[LLD]TVB497 MODULATOR PARAM/PCI593_REG_SELECT_EXT_OSC=", (unsigned int)dwStatus);

		TSPL_nUse10MREFClock = (((dwStatus>>27)&0x01)<<27);
		LldPrint_1("[LLD]TVB497 10MHz REF. CLOCK=", (unsigned int)TSPL_nUse10MREFClock);
	}
	DBG_PRINT_INIT_CONF(TSPL_nModulatorType, TSPL_nModulatorEnabled, TSPL_nBoardOption, TSPL_nUse10MREFClock, TSPL_nBankOffset, TSPL_nBankCount);

	//2012/8/22 TVB593/597A V3
	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		dwStatus = WDM_Read_TSP(_ADR_597v2_FPGA_FPGA_AD9852_INFO);
		if((dwStatus >> 4) & 0x1)
		{
			TSPL_SET_CONFIG_DOWNLOAD(_ADR_597v2_WATCHDOG_CONTROL, 0x0);
		}
	}

	//2012/9/21 ISDB-T Frequency skip
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833() && (TSPL_nModulatorType == TVB380_ISDBT_13_MODE || TSPL_nModulatorType == TVB380_ISDBT_MODE))
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ISDBT_MODULATOR_RESET);
		dwStatus = (((dwStatus >> 5) & 0x7FFFFFF) << 5) + 0x10 + (dwStatus & 0xF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ISDBT_MODULATOR_RESET, dwStatus))
		{
			UnlockEpldMutex();
			LldPrint_Error("bd-init : pos...", 4, 52);
			return -1;
		}
	}

////////////////////////////////////////////////////////////
	_MyCnxt()->_modulator_enabled = TSPL_nModulatorEnabled;
	_MyCnxt()->_modulator_typ_running_now = TSPL_nModulatorType;
	_MyCnxt()->_use_10m_ref_clock = TSPL_nUse10MREFClock;
	_MyCnxt()->_if_freq_offset = gChannel_Freq_Offset;

	_MyCnxt()->_fpga_id = TSPL_GET_FPGA_INFO(0);
	_MyCnxt()->_fpga_ver = TSPL_GET_FPGA_INFO(1);
	_MyCnxt()->_fpga_iq_play = TSPL_GET_FPGA_INFO(2);
	_MyCnxt()->_fpga_iq_capture = TSPL_GET_FPGA_INFO(3);
	_MyCnxt()->_fpga_rbf = TSPL_GET_FPGA_INFO(4);
	_MyCnxt()->_fpga_bld = TSPL_GET_FPGA_INFO(5);

	_MyCnxt()->__cnt_my_sub_ts_vsb__ = TSPL_CNT_MULTI_VSB_RFOUT();
	_MyCnxt()->__cnt_my_sub_ts_qam__ = TSPL_CNT_MULTI_QAM_RFOUT();
	//2012/6/28 multi dvb-t
	_MyCnxt()->__cnt_my_sub_ts_dvbt__ = TSPL_CNT_MULTI_DVBT_RFOUT();
	//2013/7/17
	_MyCnxt()->__dac_i_offset = i_offset;
	_MyCnxt()->__dac_q_offset = q_offset;

	if(IsAttachedBdTyp_SupportBoardFeature())
	{
		_MyCnxt()->_fpga_ClockGeneratorModel = Get_Board_HW_Feature(1);
		_MyCnxt()->_fpga_UsbResetSupport = Get_Board_HW_Feature(2);
		_MyCnxt()->_fpga_NumOfLED = Get_Board_HW_Feature(4);
	}
	else
	{
		_MyCnxt()->_fpga_ClockGeneratorModel = 0;
		_MyCnxt()->_fpga_UsbResetSupport = 0;
		_MyCnxt()->_fpga_NumOfLED = 0;
	}
	
	//2013/6/13
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
	{
		Check_HMCorOPEN_Mode(&_MyCnxt()->_hmc833_SerialPortMode, &_MyCnxt()->_hmc1033_SerialPortMode);	
	}
	else
	{
		_MyCnxt()->_hmc833_SerialPortMode = -1;
		_MyCnxt()->_hmc1033_SerialPortMode = -1;
	}

	DupBdCnxt_Step4((void *)_MyCnxt());

	return TLV_NO_ERR;
}

#if defined(WIN32) || defined(TVB595V1)
/*^^***************************************************************************
 * Description : Access(Download) F/W thru. USB
 *				
 * Entry : hDevice, szPathRBF, rbf_name
 *
 * Return: non zero(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
//=================================================================	
#ifdef WIN32
int CBdInit::Write_EPLD_USB(HANDLE hDevice, TCHAR *szPathRBF, TCHAR *rbf_name)
{
	int			nAltera_file_size;
	int			hf_RFF_File;
//	FILE			*hf_RFF_File;
	char			szCurrentRBF[_MAX_PATH];

	DBG_PLATFORM_BRANCH();
	LldPrint("[LLD]===FPGA DOWNLOAD USB");

	if (hDevice == NULL)
	{
		LldPrint_Error("bd-init : pos...", 4, 33);
		return TLV_NO_DRIVER;
	}

	//GetCurrentDirectory(_MAX_PATH, szCurDir);	// Save Current directory
	//SetCurrentDirectory(szPathRBF);
 
	sprintf(szCurrentRBF, "%s\\%s",szPathRBF,rbf_name);

#if	1
	hf_RFF_File = _open(szCurrentRBF, _O_BINARY);
	nAltera_file_size = (int) _filelengthi64(hf_RFF_File);	// Get file size
	if (hf_RFF_File == -1)
#else
	hf_RFF_File = fopen(szCurrentRBF, "rb");
	fseek(hf_RFF_File, 0L, SEEK_END);
	nAltera_file_size = (int) ftell(hf_RFF_File);	// Get file size
	fseek(hf_RFF_File, 0L, SEEK_SET);
	if (hf_RFF_File == NULL)
#endif
	{	
		//TVB593
		TLV_ControlErrorCode = TLV_FAIL_TO_FIND_FW_FILE;

		// Fail to open Altera Configuration File
		LldPrint_Error("bd-init : pos...", 4, 34);
		return TLV_NO_RBF;
	}
	else if (nAltera_file_size > MAX_RBF_BUFF_SIZE)
	{
		LldPrint_Error("bd-init : pos...", 4, 35);
		return TLV_NO_RBF;
	}
#if	1
	if(_read(hf_RFF_File, szRBF_buff, nAltera_file_size) != nAltera_file_size)	// Corrupted Altera Configuration file size
	{
		_close(hf_RFF_File);
		LldPrint_Error("bd-init : pos...", 4, 36);
		return TLV_ALTERA_FILE_READ_ERR;
	}
	_close(hf_RFF_File);
#else
	if(fread(szRBF_buff, 1, nAltera_file_size, hf_RFF_File) != nAltera_file_size)	// Corrupted Altera Configuration file size
	{
		fclose(hf_RFF_File);
		LldPrint_Error("bd-init : pos...", 4, 36);
		return TLV_ALTERA_FILE_READ_ERR;
	}
	fclose(hf_RFF_File);
#endif

	//	Downloading... please Wait...
	if (hDevice == NULL)
	{
		LldPrint_Error("bd-init : pos...", 4, 37);
		return -1;
	}
	
	return	DownloadRbf(nAltera_file_size);
}
#else	//Linux
int CBdInit::Write_EPLD_USB(char *szPathRBF, char *rbf_name)
{
	int			nAltera_file_size;
	FILE*		hf_RFF_File;

	DBG_PLATFORM_BRANCH();
	LldPrint_FCall(rbf_name, 0, 0);
	if ( !CHK_DEV(hDevice_usb) )
	{
		LldPrint_Error("bd-init : pos...", 4, 38);
		return TLV_NO_DRIVER;
	}

	if (!(hf_RFF_File = fopen(rbf_name, "r")))
	{
		//TVB593
		TLV_ControlErrorCode = TLV_FAIL_TO_FIND_FW_FILE;

		LldPrint_Error("bd-init : pos...", 4, 39);
		LldPrint_s_s("[LLD] No RBF file :", rbf_name);
		return TLV_NO_RBF;
	}

	fseek(hf_RFF_File, 0L, SEEK_END);
	nAltera_file_size = (int) ftell(hf_RFF_File);	// Get file size

	if (nAltera_file_size > MAX_RBF_BUFF_SIZE)
	{
		LldPrint_Error("bd-init : pos...", 4, 40);
		return TLV_NO_RBF;
	}

	fseek(hf_RFF_File, 0L, SEEK_SET);

	if(fread(szRBF_buff, 1, nAltera_file_size, hf_RFF_File) != nAltera_file_size)
	{
		// Corrupted Altera Configuration file size
		fclose(hf_RFF_File);
		LldPrint_Error("bd-init : pos...", 4, 41);
		return TLV_ALTERA_FILE_READ_ERR;
	}
	fclose(hf_RFF_File);

	//	Downloading... please Wait...
	if ( !CHK_DEV(hDevice_usb) )
	{
		LldPrint_Error("bd-init : pos...", 4, 42);
		return -1;
	}

	return	DownloadRbf(nAltera_file_size);
}
#endif
#endif
int	CBdInit::DownloadRbf_Test(int nAltera_file_size)
{
	//unsigned long localAddress = TSPL_GET_DMA_REG_INFO__(0x22);
	static unsigned char	szbuffer[MAX_DMA_SIZE];
	int i,j,k;
//	unsigned long 	dwRet;
//	unsigned long 	dwStatus;
	unsigned char buf;
//	KCMD_ARGS	KFileInf;
	unsigned long maskdata=0, serialdata;
	int use_2m_space = 0;
	unsigned long dmaAddress = 0x400000;

	if ( IsAttachedBdTyp_DiffAlteraSizeIndicator() || IsAttachedBdTyp_499())
	{
		use_2m_space = 1;
		dmaAddress = 0x40000;
	}

	k = 0;
	//TSPL_SET_DMA_REG_INFO__(0x22, (dmaAddress * 4));
	for (i = 0; i < nAltera_file_size; i++)
	{
		buf = *(szRBF_buff + i);

		for (j = 0; j < 8; j++)
		{	
			maskdata = (unsigned long)((buf & 0x01));

			//if ( k == MAX_DMA_SIZE )
			//{
			//	memcpy(dwpDMABufLinearAddr, szbuffer, MAX_DMA_SIZE);
			//	if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
			//	{
			//		// 9054(0x8c): set DMA transfer size in bytes
			//		if (TSPL_SET_DMA_REG_INFO__(35, 0x100000))
			//		{
			//			if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
			//			{
			//				while ( !TSPL_GET_DMA_STATUS() ) Sleep(10);
			//			}
			//			else
			//			{
			//				return -1;
			//			}
	
			//		}
			//		else
			//			return -1;
			//	}
			//	else
			//		return -1;
			//	k = 0;
			//	memset(szbuffer, 0, MAX_DMA_SIZE);
			//}
			if(use_2m_space == 1)
				serialdata = ((0x00000000) | maskdata);
			else
				serialdata = ((0x000000c0) | maskdata);
			//memcpy(szbuffer+k, &serialdata, sizeof(unsigned long));
			WDM_WR2SDRAM_ADDR(dmaAddress, serialdata);
			//k += sizeof(unsigned long);

			if(use_2m_space == 1)
				serialdata = ((0x00000002) | maskdata);
			else
				serialdata = ((0x000000c4) | maskdata);
			WDM_WR2SDRAM_ADDR(dmaAddress, serialdata);
			//memcpy(szbuffer+k, &serialdata, sizeof(unsigned long));
			//k += sizeof(unsigned long);

			buf >>= 1; // LSB first
		} //j-for loop			
	} //i-for loop

	//if(k > 0)
	//{
	//	memcpy(dwpDMABufLinearAddr, szbuffer, MAX_DMA_SIZE);
	//	if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
	//	{
	//		// 9054(0x8c): set DMA transfer size in bytes
	//		if (TSPL_SET_DMA_REG_INFO__(35, 0x100000))
	//		{
	//			if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
	//			{
	//				while ( !TSPL_GET_DMA_STATUS() ) Sleep(10);
	//			}
	//			else
	//			{
	//				return -1;
	//			}
	//
	//		}
	//		else
	//			return -1;
	//	}
	//	else
	//		return -1;
	//}
	//TSPL_SET_DMA_REG_INFO__(0x22, localAddress);
	return 0;
}
//2013/1/18 Fast Download
int	CBdInit::FastDownloadRbf(int nAltera_file_size)
{
	unsigned long Data = 0;
	KCMD_ARGS	KFileInf;
	static unsigned int szbuffer[MAX_RBF_BUFF_SIZE];
	unsigned long bufferSize;
	unsigned long 	dwRet;
	int p_buf_pos;
	if(IsAttachedBdTyp_497_or_597v2())
	{
#if 1
		p_buf_pos = 0;
		
		if((nAltera_file_size % 0x400) > 0)
		{
			bufferSize = ((nAltera_file_size / 0x400) + 1) * 0x400;
		}
		else
		{
			bufferSize = nAltera_file_size;
		}
		while(1)
		{
			memset((char *)szbuffer, 0, MAX_RBF_BUFF_SIZE);
			if(p_buf_pos + 0x100000 < nAltera_file_size)
			{
				memcpy(szbuffer, (snRBF_buff + (p_buf_pos / 4)), 0x100000);
				KFileInf.pdwBuffer = (DWORD *)szbuffer;
				KFileInf.dwCmdParm1 = PCI593_FAST_DOWNLOAD_DATA;
				KFileInf.dwCmdParm2 = 0x100000;
				TLV_RBF_DOWNLOAD_USB
				p_buf_pos += 0x100000;
			}
			else if(p_buf_pos + 0x100000 == nAltera_file_size)
			{
				memcpy(szbuffer, (snRBF_buff + (p_buf_pos / 4)), 0x100000);
				KFileInf.pdwBuffer = (DWORD *)szbuffer;
				KFileInf.dwCmdParm1 = PCI593_FAST_DOWNLOAD_DATA;
				KFileInf.dwCmdParm2 = 0x100000;
				TLV_RBF_DOWNLOAD_USB
				//p_buf_pos += 0x100000;
				break;
			}
			else
			{
				memcpy(szbuffer, (snRBF_buff + (p_buf_pos / 4)), (((nAltera_file_size / 4) + 1) * 4));
				KFileInf.pdwBuffer = (DWORD *)szbuffer;
				KFileInf.dwCmdParm1 = PCI593_FAST_DOWNLOAD_DATA;
				KFileInf.dwCmdParm2 = (bufferSize - p_buf_pos);
				TLV_RBF_DOWNLOAD_USB
				break;
			}

		}
#else
		memset((char *)szbuffer, 0, MAX_RBF_BUFF_SIZE);
#if 1
		if((nAltera_file_size % 0x400) > 0)
		{
			bufferSize = ((nAltera_file_size / 0x400) + 1) * 0x400;
		}
		else
		{
			bufferSize = nAltera_file_size;
		}
#else
		bufferSize = ((nAltera_file_size / 4) + 4) * 4;
#endif
		p_buf_pos = 0;
		nLoopCnt = nAltera_file_size / 4;
		nRemainder = nAltera_file_size - (nLoopCnt * 4);
	
		if(nRemainder > 0)
			nLoopCnt++;
		LockDmaMutex();
#if 1
		for (i = 0; i < nLoopCnt; i++)
		{
			Data = *(snRBF_buff + i);
			if(p_buf_pos == 0x100000)
			{
				sprintf(tmp2, "[0x%x][0x%x][0x%x][0x%x]\n", szbuffer[0],szbuffer[0x80],szbuffer[0x100],szbuffer[0xffb60]);
				OutputDebugString(tmp2);
				KFileInf.pdwBuffer = szbuffer;
				KFileInf.dwCmdParm1 = PCI593_FAST_DOWNLOAD_DATA;
				KFileInf.dwCmdParm2 = 0x100000;
				TLV_RBF_DOWNLOAD_USB
				memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
				p_buf_pos = 0;
				memcpy(szbuffer+p_buf_pos, &Data, sizeof(unsigned int));
				p_buf_pos += sizeof(unsigned int);
				bufferSize = bufferSize - 0x100000;
			}
			else
			{
				memcpy(szbuffer+p_buf_pos, &Data, sizeof(unsigned int));
				p_buf_pos += sizeof(unsigned int);
			}
		}
#else
		memcpy((char *)szbuffer, snRBF_buff, nAltera_file_size + 1);
#endif
		KFileInf.pdwBuffer = szbuffer;
		KFileInf.dwCmdParm1 = PCI593_FAST_DOWNLOAD_DATA;
		KFileInf.dwCmdParm2 = bufferSize;
		sprintf(tmp2, "[0x%x][0x%x][0x%x][0x%x]\n", szbuffer[0],szbuffer[4],szbuffer[8],szbuffer[12]);
		OutputDebugString(tmp2);
		if(p_buf_pos > 0)
		{
			TLV_RBF_DOWNLOAD_USB
		}
		UnlockDmaMutex();
#endif
	}
	else
	{
		p_buf_pos = 0;
		unsigned int localAddress = TSPL_GET_DMA_REG_INFO__(0x22);
		TSPL_SET_DMA_REG_INFO__(0x22, (PCI593_FAST_DOWNLOAD_DATA * 4));
#if 0
		memset((char *)dwpDMABufLinearAddr, 0, 0x800000);
		bufferSize = nAltera_file_size / 4;
		nAltera_file_size = (bufferSize + 1) * 4;

		memcpy(dwpDMABufLinearAddr, snRBF_buff, nAltera_file_size);
		if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
		{
			// 9054(0x8c): set DMA transfer size in bytes
			if (TSPL_SET_DMA_REG_INFO__(35, nAltera_file_size))
			{
				if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
				{
					while ( !TSPL_GET_DMA_STATUS() )
					{
						Sleep(10);
					}
				}
				else
				{
					return -1;
				}
			}
			else
				return -1;
		}
		else
			return -1;
#else
		while(1)
		{
			memset((char *)dwpDMABufLinearAddr, 0, 0x100000);
			if((p_buf_pos + 0x100000) < nAltera_file_size)
			{
				memcpy(dwpDMABufLinearAddr, (snRBF_buff + (p_buf_pos / 4)), 0x100000);
#ifdef WIN32		
#else
				KFileInf.pdwBuffer = (DWORD *)dwpDMABufLinearAddr;
				KFileInf.dwCmdParm1 = 0x100000;
				TLV_DeviceIoControl(hDevice, IOCTL_FILE_DATA_DOWN, &KFileInf, sizeof(KFileInf), NULL, 0, &dwRet, 0);
#endif
				if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
				{
					// 9054(0x8c): set DMA transfer size in bytes
					if (TSPL_SET_DMA_REG_INFO__(35, 0x100000))
					{
						if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
						{
							p_buf_pos = p_buf_pos + 0x100000;
							while ( !TSPL_GET_DMA_STATUS() )
							{
								Sleep(10);
							}
						}
						else
						{
							return -1;
						}
					}
					else
						return -1;
				}
				else
					return -1;
			}
			else if((p_buf_pos + 0x100000) == nAltera_file_size)
			{
				memcpy(dwpDMABufLinearAddr, (snRBF_buff + (p_buf_pos / 4)), 0x100000);
#ifdef WIN32		
#else
				KFileInf.pdwBuffer = (DWORD *)dwpDMABufLinearAddr;
				KFileInf.dwCmdParm1 = 0x100000;
				TLV_DeviceIoControl(hDevice, IOCTL_FILE_DATA_DOWN, &KFileInf, sizeof(KFileInf), NULL, 0, &dwRet, 0);
#endif
				if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
				{
					// 9054(0x8c): set DMA transfer size in bytes
					if (TSPL_SET_DMA_REG_INFO__(35, 0x100000))
					{
						if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
						{
							p_buf_pos = p_buf_pos + 0x100000;
							while ( !TSPL_GET_DMA_STATUS() ) Sleep(10);
							
						}
						else
						{
							return -1;
						}
	
					}
					else
						return -1;
				}
				else
					return -1;
				break;
			}
			else
			{
				
				bufferSize = (nAltera_file_size - p_buf_pos);
				if((bufferSize % 4) != 0)
				{
					bufferSize = ((bufferSize / 4) + 1) * 4;
				}
				memcpy(dwpDMABufLinearAddr, (snRBF_buff + (p_buf_pos / 4)), bufferSize);
#ifdef WIN32		
#else
				KFileInf.pdwBuffer = (DWORD *)dwpDMABufLinearAddr;
				KFileInf.dwCmdParm1 = bufferSize;
				TLV_DeviceIoControl(hDevice, IOCTL_FILE_DATA_DOWN, &KFileInf, sizeof(KFileInf), NULL, 0, &dwRet, 0);
#endif
				if (TSPL_SET_DMA_REG_INFO__(36, 0x00000001))
				{
					// 9054(0x8c): set DMA transfer size in bytes
					if (TSPL_SET_DMA_REG_INFO__(35, bufferSize))
					{
						if (TSPL_SET_DMA_REG_INFO__(42, 0x00000003))
						{
							p_buf_pos = p_buf_pos + bufferSize;
							while ( !TSPL_GET_DMA_STATUS() ) Sleep(10);
						}
						else
						{
							return -1;
						}
					}
					else
						return -1;
				}
				else
					return -1;
				break;
			}
		}
#endif
		TSPL_SET_DMA_REG_INFO__(0x22, localAddress);
	}	
	return 0;
}

int	CBdInit::DownloadRbf(int nAltera_file_size)	//	ConfigureEPLD(szRBF_buff[])
{
	static	unsigned char	szbuffer[MAX_RBF_BUFF_SIZE];
	KCMD_ARGS	KFileInf;
	int i,j,k, total;
	unsigned long 	dwRet;
	unsigned long 	dwStatus;
	unsigned char buf;
	unsigned long maskdata=0, serialdata;

	KFileInf.pdwBuffer = (DWORD*)szbuffer;
	KFileInf.dwCmdParm1 = TSP_MEM_ADDR_COMMAND;
	KFileInf.dwCmdParm2 = MAX_RBF_BUFF_SIZE/8;	//1MB	//2013/1/4 2 --> 8
	
	k = 0;
	memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
	
#if TVB597A_STANDALONE
	TSPL_nFPGA_TEST = 1;
#else
//	TSPL_nFPGA_TEST = 1;
#endif
	
	//FPGA d/w
	if ( IsAttachedBdTyp_597_497() || IsAttachedBdTyp_499() )
	{
		KFileInf.dwCmdParm1 = RemapAddress(KFileInf.dwCmdParm1);
	
		if ( IsAttachedBdTyp_497_or_597v2() || IsAttachedBdTyp_499() )
		{
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF 
					|| (dwStatus>>3 & 0x01) != 1 )
				{
					LldPrint_Error("bd-init : pos...", 4, 43);
					LldPrint_1x("STEP1:CONFIG_STATUS :", (unsigned int)dwStatus);
					return -1;
				}
			}
		
			//set DMA configuration on
			dwStatus = WDM_WR2SDRAM_ADDR(PCI497_REG_CONTROL_REG, 0x00000001);
		
			//assert nCONFIG
			dwStatus = WDM_WR2SDRAM(0x000000C4);
			usleep(100);
		
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF 
					|| ((dwStatus>>3 & 0x01) != 0 || (dwStatus>>4 & 0x01) != 0) )
				{
					LldPrint_1x("STEP3:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_497;
				}
			}
		
			//negate nCONFIG
			dwStatus = WDM_WR2SDRAM(0x000000C0);
			usleep(100);
		
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				LldPrint_1x("CONF-STS before sending FPGA DATA", (int)dwStatus);
				if ( dwStatus == 0xFFFFFFFF || ((dwStatus>>3 & 0x01) != 1 || (dwStatus>>4 & 0x01) != 0) )
				{
					LldPrint_1x("STEP5:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_497;
				}
				usleep(100);
			}
		}
		else
		{
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF 
					|| ((dwStatus>>4 & 0x01) != 1/*|| (dwStatus>>5 & 0x01) != 1*/) )
				{
					LldPrint_Error("bd-init : pos...", 4, 44);
					return -1;
				}
			}
		
			//set DMA configuration on
			dwStatus = WDM_WR2SDRAM(0x000000E1);
		
			//assert nCONFIG
			dwStatus = WDM_WR2SDRAM(0x000000C4);
			usleep(100);
		
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF 
					|| ((dwStatus>>4 & 0x01) != 0 || (dwStatus>>5 & 0x01) != 0) )
				{
					LldPrint_1x("STEP3:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_;
				}
			}
		
			//negate nCONFIG
			dwStatus = WDM_WR2SDRAM(0x000000C0);
			usleep(100);
		
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF 
					|| ((dwStatus>>4 & 0x01) != 1 || (dwStatus>>5 & 0x01) != 0) )
				{
					LldPrint_1x("STEP5:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_;
				}
				usleep(100);
			}
		}

		for (i = 0; i < nAltera_file_size; i++)
		{
			buf = *(szRBF_buff + i);

			for (j = 0; j < 8; j++)
			{	
				maskdata = (unsigned long)((buf & 0x01));

				if ( k == (int)KFileInf.dwCmdParm2 )
				{
					TLV_RBF_DOWNLOAD_USB

					k = 0;
					memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
				}

				serialdata = ((0x00000000) | maskdata);
				memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
				k += sizeof(unsigned int);

				if ( k == (int)KFileInf.dwCmdParm2 )
				{
					TLV_RBF_DOWNLOAD_USB

					k = 0;
					memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
				}

				serialdata = ((0x00000002) | maskdata);
				memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
				k += sizeof(unsigned int);

				buf >>= 1; // LSB first
			} //j-for loop			
		} //i-for loop

		if ( k > 0 )
		{
			TLV_RBF_DOWNLOAD_USB
		}
		Sleep(100);
	
		if ( IsAttachedBdTyp_497_or_597v2() || IsAttachedBdTyp_499() )
		{
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				LldPrint_1("STEP-A:CONFIG_STATUS :", (unsigned int)dwStatus);
				if ( dwStatus == 0xFFFFFFFF || ((dwStatus>>3 & 0x01) != 1 || (dwStatus>>4 & 0x01) != 1) )
				{
					LldPrint_1("STEP8:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_497;
				}
			}

			//set DMA configuration off
_DMA_CONFIGURATION_OFF_497:
			dwStatus = WDM_WR2SDRAM_ADDR(PCI497_REG_CONTROL_REG, 0x00000000);
		}
		else
		{
			if ( TSPL_nFPGA_TEST == 0 )
			{
				dwStatus = WDM_Read_TSP(TSP_BOARD_CONFIG_STATUS);
				if ( dwStatus == 0xFFFFFFFF || ((dwStatus>>4 & 0x01) != 1 || (dwStatus>>5 & 0x01) != 1) )
				{
					LldPrint_1("STEP8:CONFIG_STATUS :", (unsigned int)dwStatus);
					goto _DMA_CONFIGURATION_OFF_;
				}
			}

			//set DMA configuration off
_DMA_CONFIGURATION_OFF_:
			dwStatus = WDM_WR2SDRAM(0x000000E0);
		}

		usleep(100);
	}
	else
	{
		//set DMA configuration on
		dwStatus = WDM_WR2SDRAM(0x000000E1);

		serialdata = 0x000000C0;
		memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
		k += sizeof(unsigned int);

		serialdata = 0x000000C4;

		//Dummy
		total = (int)(ceil((nAltera_file_size*8*2) / (double)KFileInf.dwCmdParm2) * KFileInf.dwCmdParm2) - ((nAltera_file_size*8*2));
		for ( i = 0 ; i < total-1; i++ )
		{
			memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
			k += sizeof(unsigned int);

			if ( k == (int)KFileInf.dwCmdParm2 )
			{
				TLV_RBF_DOWNLOAD_USB

				k = 0;
				memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
			}
		}

		serialdata = 0x000000C0;
		memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
		k += sizeof(unsigned int);

		if ( k == (int)KFileInf.dwCmdParm2 )
		{
			TLV_RBF_DOWNLOAD_USB

			k = 0;
			memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
		}

		for (i = 0; i < nAltera_file_size; i++)
		{
			buf = *(szRBF_buff + i);

			for (j = 0; j < 8; j++)
			{	
				maskdata = (unsigned long)((buf & 0x01));

				if ( k == (int)KFileInf.dwCmdParm2 )
				{
					TLV_RBF_DOWNLOAD_USB

					k = 0;
					memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
				}

				serialdata = (0x000000C0 | maskdata);
				memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
				k += sizeof(unsigned int);

				if ( k == (int)KFileInf.dwCmdParm2 )
				{
					TLV_RBF_DOWNLOAD_USB

					k = 0;
					memset(szbuffer, 0, MAX_RBF_BUFF_SIZE);
				}

				serialdata = (0x000000C2 | maskdata);
				memcpy(szbuffer+k, &serialdata, sizeof(unsigned int));
				k += sizeof(unsigned int);

				buf >>= 1; // LSB first
			} //j-for loop			
		} //i-for loop

		//set DMA configuration off
		dwStatus = WDM_WR2SDRAM(0x000000E0);
	}

	/* DEMUX BLOCK TEST : normal state */ //TVB593, TVB497
	if ( IsAttachedBdTyp_HaveDmxCmtlTest() )
	{
		//sskim20080925 - TEST
		//Sleep(10);
		dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_DEMUX_CNTL_TEST, 0x00000000);
		//sskim20080925 - TEST
		//Sleep(10);
	}
	LldPrint_2("rbf-download finished : usb", 0, 0);

	return 0;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int	_stdcall CBdInit::TVBxxx_DETECT_BD(int _multi_bd)
{
	return	TVBxxx_DETECT_BD_(_multi_bd);
}
int	_stdcall CBdInit::TVBxxx_INIT_BD(int _my_num, void *_my_cxt)
{
	int	ret;
	_BD_CONF_CNXT	_dev_cnxt;
	memcpy(&_dev_cnxt, _my_cxt, sizeof(_BD_CONF_CNXT));
	OpenF_LldPrint(_my_num, _dev_cnxt.szLog_SubFolder);
	ret = TVBxxx_INIT_BD_(_my_num, _my_cxt);
	return	ret;
}
int	_stdcall CBdInit::TVBxxx_GET_CNXT_ALL_BD(void *_cxt)
	{
	return	TVBxxx_RdAllRealBdCnxt(_cxt);
	}
int	_stdcall CBdInit::TVBxxx_GET_CNXT_MINE(void *_cxt)
	{
	return	TVBxxx_RD_BD_CNXT(_cxt);
	}
int	_stdcall CBdInit::TVBxxx_DUP_BD(int from_slot_num, int to_slot_num, void *_from_cnxt)
	{
	_BD_CONF_CNXT	_dev_cnxt;
	memcpy(&_dev_cnxt, _from_cnxt, sizeof(_BD_CONF_CNXT));
	OpenF_LldPrint(to_slot_num, _dev_cnxt.szLog_SubFolder);
	TVBxxx_DUP_BD_(from_slot_num, to_slot_num, _from_cnxt);
	DupBdCnxt(_from_cnxt);
	return	0;
		}
int	_stdcall CBdInit::TVBxxx_CLOSE_BD(void)
		{
	TVBxxx_CLOSE_BD_();
			CloseF_LldPrint();
	return 0;
}

/*^^***************************************************************************
 * Description : Initialize the selected modulator type
 *				
 * Entry : modulator_type, IF_Frequency(36000000 or 44000000)
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int _stdcall CBdInit::TVB380_SET_CONFIG(long modulator_type, long IF_Frequency)
{
	char	szfnRbf[80];
	int 	nRet = 0;
	//TVB593
	long	scale_factor = 0;
	//TVB497
	unsigned long dwStatus;

	if (IsAttachedBdTyp_594())
	{
		modulator_type = TVB380_VSB8_MODE;	//	support vsb only.
	}
	if (IsAttachedBdTyp__Virtual())
	{
		return 0;
	}

	LockEpldMutex();
	DBG_PRINT_RECONF_MOD(modulator_type, IF_Frequency);

	gChannel_Freq_Offset = IF_Frequency;
	LldPrint_2("TVB380_SET_CONFIG, Type, IF", (int)modulator_type, (int)IF_Frequency);

	nRet = WDM_Read_TSP(TSP_MEM_ADDR_MODULATOR_CONFIG);
	if ( ((nRet >> 7) & 0x01) == 1 && ((nRet >> 12) & 0xF) == modulator_type )
	{
	}
	
	MakeRbfName(szfnRbf, modulator_type);

//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
	{
		LldPrint_Error("FAIL of device driver. Check the device handle is loaded properly", 0, 0);
		UnlockEpldMutex();
		return -1;
	}
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( !CHK_DEV(hDevice_usb) )
		{
			LldPrint_Error("FAIL of device driver. Check the device handle is loaded properly", 0, 0);
			UnlockEpldMutex();
			return -1;
		}
	}
	else
	{
		if ( !CHK_DEV(hDevice) )
		{
			LldPrint_Error("FAIL of device driver. Check the device handle is loaded properly", 0, 0);
			UnlockEpldMutex();
			return -1;
		}
	}
#endif
#ifdef WIN32
	nRet = Write_EPLD(hDevice, szCurDir, szfnRbf);
#else
	nRet = Write_EPLD(NULL, szfnRbf);
#endif
	if ( nRet )
	{
		LldPrint_Error("bd-init : pos...", 4, 46);
		LldPrint_s_s("FAIL to configure F/W with the file ", szfnRbf);
		UnlockEpldMutex();
		return -1;
	}
	
	//////////////////////////////////////////////////////////////
	//	Check the RBF download is complete
	//TVB593, TVB497
	if ( IsAttachedBdTyp_AllCase_w_SpecialBd() )
	{
		if ( WDM_Check_Board_Status(0, 1) == TLV_NO_ERR )
		{
		}
		else
		{
			if ( IsAttachedBdTyp_UsbTyp() )
			{
				LldPrint_s_s("FAIL to configure F/W with the file ", szfnRbf);
				LldPrint("Try to download again...");
				//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
				if ( !CHK_DEV(hDevice) )
#else
				if ( !CHK_DEV(hDevice_usb) )
#endif			
				{
					LldPrint_Error("FAIL of device driver. Check the device handle is loaded properly", 0, 0);
					UnlockEpldMutex();
					return -1;
				}
#ifdef WIN32
				nRet = Write_EPLD(hDevice, szCurDir, szfnRbf);
#else
				nRet = Write_EPLD(NULL, szfnRbf);
#endif
				if ( nRet )
				{
					LldPrint_Error("bd-init : pos...", 4, 47);
					UnlockEpldMutex();
					return -1;
				}
				if ( WDM_Check_Board_Status(0, 1) == TLV_NO_ERR )
					;
				else
				{
					LldPrint_Error("bd-init : pos...", 4, 48);
					UnlockEpldMutex();
					return -1;
				}
			}
			else
			{
				LldPrint_Error("bd-init : pos...", 4, 49);
				UnlockEpldMutex();
				return -1;
			}
		}
	}

	WrRrcFilter();

	//sskim20080526 - TEST
//__SKIP__DOWNLOAD_RBF__:
	int i_offset = 0;
	int q_offset = 0;
    char		szLicenseNum[64];
	int nFlag_readLN_from_Eeprom = -1;

	if(IsAttachedBdTyp_SupportEepromRW())
	{
		nFlag_readLN_from_Eeprom = Read_Eeprom_Information(&i_offset, &q_offset, szLicenseNum);
	}

	//////////////////////////////////////////////////////////////
	//	Get the enabled modulator types
	//TVB593, TVB497
	if ( IsAttachedBdTyp_AllCase() )
	{
		if(nFlag_readLN_from_Eeprom == 0)
		{
			int nRet;
			nRet = read_modulator_option2(szLicenseNum);
			if(nRet < 0)
			{
				TSPL_nModulatorEnabled = read_modulator_option();
			}
			else
			{
				TSPL_nModulatorEnabled = nRet;
			}
		}
		else
			TSPL_nModulatorEnabled = read_modulator_option();
		if ( TSPL_nModulatorEnabled > 0 )
		{
			TSPL_nModulatorType = modulator_type;
		}
		else
		{
			TLV_ControlErrorCode = TLV_FAIL_TO_CONFIRM_LN;
		}
	}

	//////////////////////////////////////////////////////////////
	//	PNP-1500-P22
	//
	nRet = Reset_TRF178(TSPL_nBoardTypeID, modulator_type, gChannel_Freq_Offset/* IF */);
	if ( nRet == -1 )
	{
		LldPrint("Fail to reset TRF178");
	}
	else
	{
	}

	//TVB590S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		WDM_WR2SDRAM_ADDR(PCI590S_REG_MODULATION_OPTION, modulator_type & 0x1F);
	}
	//TVB593, TVB497
	else if ( IsAttachedBdTyp_NewAddrMap() )
	{
		WDM_WR2SDRAM_ADDR(PCI593_REG_MODULATION_OPTION, modulator_type & 0xFF);
	}
	else
	{
		WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, (0x80+modulator_type)&0xFF);
	}

	//TVB590S, TVB593, TVB497
	if ( IsAttachedBdTyp_UseAD9775() )
	{
		Default_Configure_AD9775(modulator_type, 0, 0);
	}

	//TVB593, TVB497
	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		if ( modulator_type == TVB380_ISDBT_MODE )
		{
			scale_factor = 0xE00;
		}
		else if ( modulator_type == TVB380_ISDBT_13_MODE )
		{
			scale_factor = 0x800;
		}
		else if (TSPL_nModulatorType == TVB380_DVBT2_MODE )
		{
			scale_factor = 0x500;
		}
		//2011/6/3 DVB-C2
		else if(TSPL_nModulatorType == TVB380_DVBC2_MODE)
		{
			scale_factor = 0x600;
		}
		else if ( modulator_type == TVB380_ISDBS_MODE )
		{
			scale_factor = 0x400;
		}

		if(IsAttachedBdTyp_591())
		{
			scale_factor = 0;
			if( modulator_type == TVB380_VSB8_MODE )
			{
				scale_factor = 0x350;
			}
			else if( modulator_type == TVB380_QAMB_MODE )
			{
				scale_factor = 0x260;
			}
			else if( modulator_type == TVB380_QAMA_MODE )
			{
				scale_factor = 0x260;	//initialize default value.
			}
			else if( modulator_type == TVB380_TDMB_MODE )
			{
				scale_factor = 0x390;
			}
			else if( modulator_type == TVB380_DVBT_MODE || modulator_type == TVB380_DVBH_MODE )
			{
				scale_factor = 0x350;
			}
			else if ( TSPL_nModulatorType == TVB380_ISDBT_MODE )
			{
				scale_factor = 0xE00;
			}
			else if ( TSPL_nModulatorType == TVB380_ISDBT_13_MODE )
			{
				scale_factor = 0x800;
			}

		}


		if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI593_REG_SCALE_FACTOR, scale_factor) )
		{
			LldPrint_Error("bd-init : pos...", 4, 50);
			UnlockEpldMutex();
			return -1;
		}
	}

	//ISDB-S
	if ( IsAttachedBdTyp_590s_SmallAddrSpace() )
	{
		if ( modulator_type == TVB380_ISDBS_MODE )
		{
			scale_factor = 0x400;
		}

		if ( scale_factor != 0 && TSPL_SET_CONFIG_DOWNLOAD(PCI590S_REG_SCALE_FACTOR, scale_factor) )
		{
			LldPrint_Error("bd-init : pos...", 4, 51);
			UnlockEpldMutex();
			return -1;
		}
	}

	if ( IsAttachedBdTyp_497() || IsAttachedBdTyp_499() )
	{
		dwStatus = WDM_Read_TSP_Indirectly(PCI593_REG_SELECT_EXT_OSC);
		LldPrint_1("[LLD]TVB497 MODULATOR PARAM/PCI593_REG_SELECT_EXT_OSC=", (unsigned int)dwStatus);

		TSPL_nUse10MREFClock = (((dwStatus>>27)&0x01)<<27);
		LldPrint_1("[LLD]TVB497 10MHz REF. CLOCK=", (unsigned int)TSPL_nUse10MREFClock);
	}

	ResetModBlkAfterChangingHwDacPara(3);
	
	//2012/9/7 new rf level control
	if(IsAttachedBdTyp_NewRFLevel_Cntl())
	{
		gBypass_AMP = 0;
	}
	//2012/9/18 
	gSingleTone = 0;

	//2012/9/21 ISDB-T Frequency skip
	if(IsAttachedBdTyp_UseVcoPllN_Hmc833() && (modulator_type == TVB380_ISDBT_13_MODE || modulator_type == TVB380_ISDBT_MODE))
	{
		dwStatus = WDM_Read_TSP_Indirectly(TSP_MEM_ISDBT_MODULATOR_RESET);
		dwStatus = (((dwStatus >> 5) & 0x7FFFFFF) << 5) + 0x10 + (dwStatus & 0xF);
		if(TSPL_SET_CONFIG_DOWNLOAD(TSP_MEM_ISDBT_MODULATOR_RESET, dwStatus))
		{
			UnlockEpldMutex();
			LldPrint_Error("bd-init : pos...", 4, 52);
			return -1;
		}
	}

	if(IsAttachedBdTyp_UseVcoPllN_Hmc833())
	{
		int hmc833_serial, hmc1033_serial;
		Check_HMCorOPEN_Mode(&hmc833_serial, &hmc1033_serial);
		gnHmc833_SerialPortMode = hmc833_serial;
		gnHmc1033_SerialPortMode = hmc1033_serial;
	}

	UnlockEpldMutex();
	return 0;
}




