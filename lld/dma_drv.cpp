//=================================================================	
//	Dma_Drv.c : DMA open/close for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================

//=================================================================
#ifdef WIN32

#include	<windows.h>
#include	<winioctl.h>
#include	<stdio.h>

#include	"Ioctl.h"
#include	"wdm_drv.h"
#include	"logfile.h"
#include	"dma_drv.h"
#include	"mainmode.h"

//=================================================================
// Linux
#else
#include 	<stdio.h>
#include 	<time.h>

#include 	"../tsp100.h"
#include 	"../include/logfile.h"
#include 	"wdm_drv.h"
#include 	"dma_drv.h"
#include 	"mainmode.h"
#endif

//TVB590S
#include	"Reg590S.h"

//TVB593
#include	"Reg593.h"

//=================================================================
int		fDMA_Busy = 0;					// indicate DMA progressing prevent board access

/////////////////////////////////////////////////////////////////
CLldDmaDrv::CLldDmaDrv(void)
{

	dwpDMABufLinearAddr = NULL;	// 1Mbyte buffer by DMA
}
CLldDmaDrv::~CLldDmaDrv()
{
}

/*^^***************************************************************************
 * Description : Configure PLX PCI9054 registers
 *				
 * Entry : 
 *
 * Return: 0(fail), 1(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
#ifdef WIN32
int CLldDmaDrv::Set_PCI9054_Reg(HANDLE hDevice, unsigned char nOffset, DWORD dwData)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;

//2014/10/20
//#ifndef WIN32
	LockDmaMutex();
//#endif
	LldPrint_Trace("Set_PCI9054_Reg", nOffset, dwData, (double)0, NULL);
	KRegInf.dwCmdParm1 = nOffset;	// 0~0x3f. from PCI-CNTRL 0x6c
	KRegInf.dwCmdParm2 = dwData;	// 32 bit value

	TLV_DeviceIoControl(hDevice,
			   IOCTL_WRITE_PCI9054_REG,
			   &KRegInf,
			   sizeof(KRegInf),
			   NULL,
			   0,	
			   &dwRet,0);
//2014/10/20
//#ifndef WIN32
	UnlockDmaMutex();
//#endif
	return 1;
}
#else
int CLldDmaDrv::Set_PCI9054_Reg_usb(usb_dev_handle* hDevice, unsigned char nOffset, DWORD dwData)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;

	LldPrint_Trace("Set_PCI9054_Reg_usb", nOffset, dwData, (double)0, NULL);
//2014/10/20
//#ifndef WIN32
	LockDmaMutex();
//#endif
	KRegInf.dwCmdParm1 = nOffset;	// 0~0x3f. from PCI-CNTRL 0x6c
	KRegInf.dwCmdParm2 = dwData;	// 32 bit value

	TLV_DeviceIoControl_usb(hDevice,
			   IOCTL_WRITE_PCI9054_REG,
			   &KRegInf,
			   sizeof(KRegInf),
			   NULL,
			   0,	
			   &dwRet,0);
//2014/10/20
//#ifndef WIN32
	UnlockDmaMutex();
//#endif
	return 1;
}

int CLldDmaDrv::Set_PCI9054_Reg(int hDevice, unsigned char nOffset, DWORD dwData)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;

	LldPrint_Trace("Set_PCI9054_Reg_", nOffset, dwData, (double)0, NULL);
//2014/10/20
//#ifndef WIN32
	LockDmaMutex();
//#endif
	KRegInf.dwCmdParm1 = nOffset;	// 0~0x3f
	KRegInf.dwCmdParm2 = dwData;	// 32 bit value

	TLV_DeviceIoControl(hDevice,
			   IOCTL_WRITE_PCI9054_REG,
			   &KRegInf,
			   sizeof(KRegInf),
			   NULL,
			   0,	
			   &dwRet,0);
//2014/10/20
//#ifndef WIN32
	UnlockDmaMutex();
//#endif
	return 1;
}
#endif

/*^^***************************************************************************
 * Description : Check the DMA done status
 *				
 * Entry : 
 *
 * Return: 0(fail), nonzero(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
int _stdcall CLldDmaDrv::TSPL_GET_DMA_STATUS(void)	//	read the status of pci-controller-reg.
{
	KCMD_ARGS	KCmdInf;
	int			dwStatus;
	DWORD		dwRet;

//	LldPrint_Tmr(1, "tspl-sts-dma : --- chk");
	LldPrint_Trace("TSPL_GET_DMA_STATUS", 0, 0, (double)0, NULL);
	LockDmaMutex();
	fDMA_Busy = 1;


	KCmdInf.dwCmdParm1 = 42;
	if ( IsAttachedBdTyp_UsbTyp() ) 
	{
#ifdef WIN32
		TLV_DeviceIoControl(hDevice,IOCTL_READ_PCI9054_REG,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);
#else
		TLV_DeviceIoControl_usb(hDevice_usb,IOCTL_READ_PCI9054_REG,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);
#endif
	}
	else
	{
		TLV_DeviceIoControl(hDevice,IOCTL_READ_PCI9054_REG,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);
	}
	dwStatus = (DWORD)(KCmdInf.dwCmdParm2 & 0x00000010);
	dwStatus >>= 4;

    fDMA_Busy = 0;
	UnlockDmaMutex();

	if ( IsAttachedBdTyp_NewAddrMap() )
	{
		if ( dwStatus ) 
		{
			if(((IsAttachedBdTyp_599() || IsAttachedBdTyp_598()) && nNumOfLED == 1) || ((IsAttachedBdTyp_598() || IsAttachedBdTyp_599()) && IsAttachedBdTyp__Virtual()))
			{
				TSPL_SET_BOARD_LED_STATUS(4, 3);
				TSPL_SET_BOARD_LED_STATUS(4, 0);
				if(ModulatorResetFlag == 1 && dwStatus == 1)
				{
					AssertReleaseResetSignal((0x1 << 3));
					AssertReleaseResetSignal(0);
					ModulatorResetFlag = 0;
				}

			}
			else
			{
				WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 0x03);
				WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 0x00);
			}
		}
	}
	//if(TSPL_nModulatorType == TVB380_DVBT2_MODE)
	{
		TSPL_T2_parity_errCnt = WDM_Read_TSP(PCI593_REG_MOD_STATUS);
	}
	return dwStatus;
}

// TVB595V1
int _stdcall CLldDmaDrv::TSPL_PUT_DATA(DWORD dwDMASize,DWORD *dest)
{
	int dwStatus = 0;

	//LldPrint_Trace("TSPL_PUT_DATA", dwDMASize, (int)dest, (double)TSPL_nTVB595Board, NULL);
	//LockDmaMutex();
	//fDMA_Busy = 1;

	if (dwpDMABufLinearAddr != NULL)
	{
//2010/10/1 USB_PCI MULTIBOARD
#ifdef WIN32
		if ( CHK_DEV(hDevice) )
#else
		if ( CHK_DEV(hDevice_usb) )
#endif
		{
			if ( IsAttachedBdTyp_UsbTyp() )
			{
				KCMD_ARGS	KFileInf;
				DWORD		dwRet;

				//Fault/Status trigger
				if ( ((TSPL_nAuthorization>>_bits_sht_led_ctl_bd_grp2_) & 0x01) == 1 )
				{
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA3);
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0);
				}
				else
				{
					if((IsAttachedBdTyp_599() && nNumOfLED == 1) || (IsAttachedBdTyp_599() && IsAttachedBdTyp__Virtual()))
					{
						;
					}
					else
					{
						//TVB59V1 - STATUS LED ON
						WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 1 + (0<<1));
						if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
							WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0 + 1 + (0<<1));
					}
				}
				LldPrint_Tmr(0, "tspl-wr-dma : --- starting");

				LockDmaMutex();

				KFileInf.dwCmdParm2 = dwDMASize;
				KFileInf.dwCmdParm1 = (ULONG)dest;
				KFileInf.pdwBuffer = dwpDMABufLinearAddr;	// ~1 Mbytes
//2010/10/1 USB_PCI MULTIBOARD
#ifdef WIN32
				TLV_DeviceIoControl(hDevice,
#else
				TLV_DeviceIoControl_usb(hDevice_usb,
#endif
					IOCTL_FILE_DATA_DOWN, 
					&KFileInf, 
					sizeof(KFileInf),
					NULL, 
					0,
					&dwRet, 
					0);
				if ( KFileInf.dwCmdParm2 != dwDMASize )
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]WRITE BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
				}
				
				UnlockDmaMutex();

				LldPrint_Tmr(0, "tspl-wr-dma : --- end", dwDMASize);

				//Fault/Status trigger
				if ( ((TSPL_nAuthorization>>_bits_sht_led_ctl_bd_grp2_) & 0x01) == 1 )
				{
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA3);
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0);
				}
				else
				{
					if((IsAttachedBdTyp_599() && nNumOfLED == 1) || (IsAttachedBdTyp_599() && IsAttachedBdTyp__Virtual()))
					{
						;
					}
					else
					{
						//TVB59V1 - STATUS LED OFF
						WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 0 + (0<<1));
						if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
							WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0 + 0 + (0<<1));
					}
				}
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]WRITE BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
		}
	}
	else
	{
		dwStatus = -1;
		LldPrint_Error("[LLD]WRITE BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
	}

    //fDMA_Busy = 0;
	//UnlockDmaMutex();

    return dwStatus;
}

void* _stdcall CLldDmaDrv::TSPL_GET_DATA(long dwDMASize)
{
	int		dwStatus = 0;

	LldPrint_Trace("TSPL_GET_DATA", dwDMASize, (int)0, (double)0, NULL);
	//LockDmaMutex();
	//fDMA_Busy = 1;

	if (dwpDMABufLinearAddr != NULL)
	{
//2010/10/1 USB_PCI MULTIBOARD
#ifdef WIN32
		if ( CHK_DEV(hDevice) )
#else
		if ( CHK_DEV(hDevice_usb) )
#endif
		{
			if ( IsAttachedBdTyp_UsbTyp() )
			{
				KCMD_ARGS	KFileInf;
				DWORD		dwRet;

				//Fault/Status trigger
				if ( ((TSPL_nAuthorization>>_bits_sht_led_ctl_bd_grp2_) & 0x01) == 1 )
				{
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA3);
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0);
				}
				else
				{
					if((IsAttachedBdTyp_599() && nNumOfLED == 1) || (IsAttachedBdTyp_599() && IsAttachedBdTyp__Virtual()))
					{
						;
					}
					else
					{
						//TVB59V1 - STATUS LED ON
						WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 1 + (0<<1));
						if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
							WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0 + 1 + (0<<1));
					}
				}
		
				LockDmaMutex();
				
				KFileInf.dwCmdParm2 = dwDMASize;
				KFileInf.dwCmdParm1 = (DWORD)0;//??????
				KFileInf.pdwBuffer = dwpDMABufLinearAddr;	// ~1 Mbytes
//2010/10/1 USB_PCI MULTIBOARD
#ifdef WIN32
				TLV_DeviceIoControl(hDevice,
#else
				TLV_DeviceIoControl_usb(hDevice_usb,
#endif

#ifdef WIN32
					IOCTL_GET_DMA_MEMORY,//==IOCTL_FILE_DATA_UP, 
#else
					IOCTL_FILE_DATA_UP,//IOCTL_GET_DMA_MEMORY,
#endif
					&KFileInf, 
					sizeof(KFileInf),
					NULL, 
					0,
					&dwRet, 
					0);
				//sskim20080416 - fixed
				//if ( KFileInf.dwCmdParm2 != dwDMASize )
				if ( KFileInf.dwCmdParm2 != (unsigned long)dwDMASize )
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]READ BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
				}

				UnlockDmaMutex();

				//Fault/Status trigger
				if ( ((TSPL_nAuthorization>>_bits_sht_led_ctl_bd_grp2_) & 0x01) == 1 )
				{
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA3);
					WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0);
				}
				else
				{
					if((IsAttachedBdTyp_599() && nNumOfLED == 1) || (IsAttachedBdTyp_599() && IsAttachedBdTyp__Virtual()))
					{
						;
					}
					else
					{
						//TVB59V1 - STATUS LED OFF
						WDM_WR2SDRAM_ADDR(TSP_BOARD_LED_STATUS, 0 + (0<<1));
						if ( TSPL_nTVB595Board > __REV_595_v1_0 ) //V1.1 or higher
							WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_COMMAND, 0xA0 + 0 + (0<<1));
					}
				}
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]READ BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
		}
	}
	else
	{
		dwStatus = -1;
		LldPrint_Error("[LLD]READ BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
	}

    //fDMA_Busy = 0;
	//UnlockDmaMutex();

    if (dwStatus == 0)
	{
		return dwpDMABufLinearAddr;
	}
	else
	{
		return NULL;
	}
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
int _stdcall CLldDmaDrv::TSPL_SET_FIFO_CONTROL(int nDMADirection, int nDMASize)
{
	KCMD_ARGS	KCmdInf;
	DWORD 		dwRet;
	LldPrint_Trace("TSPL_SET_FIFO_CONTROL", nDMADirection, (int)nDMASize, (double)0, NULL);

	LockDmaMutex();
	//2010/10/1 USB_PCI MULTIBOARD
#ifdef WIN32
	if ( !CHK_DEV(hDevice) )
	{
//		LldPrint_Error("dma_drv : pos...", 3, 1);
		UnlockDmaMutex();
		return -1;//TLV_NO_DEVICE;
	}

	KCmdInf.dwCmdParm1 = nDMADirection;
	KCmdInf.dwCmdParm2 = (DWORD)nDMASize;
	TLV_DeviceIoControl(hDevice,
		IOCTL_SET_FIFO_CONTROL,
		&KCmdInf,
		sizeof(KCmdInf),
		NULL, 
		0, 
		&dwRet,
		0);
#else
	if ( !CHK_DEV(hDevice_usb) )
	{
//		LldPrint_Error("dma_drv : pos...", 3, 2);
		UnlockDmaMutex();
		return -1;//TLV_NO_DEVICE;
	}

	KCmdInf.dwCmdParm1 = nDMADirection;
	KCmdInf.dwCmdParm2 = (DWORD)nDMASize;
	//2010/10/1 USB_PCI MULTIBOARD
	TLV_DeviceIoControl_usb(hDevice_usb,
		IOCTL_SET_FIFO_CONTROL,
		&KCmdInf,
		sizeof(KCmdInf),
		NULL, 
		0, 
		&dwRet,
		0);
#endif

	UnlockDmaMutex();
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
int _stdcall CLldDmaDrv::TSPL_GET_FIFO_CONTROL(int nDMADirection, int nDMASize)
{
	LldPrint_Trace("TSPL_GET_FIFO_CONTROL", nDMADirection, (int)nDMASize, (double)0, NULL);
#ifdef WIN32
	KCMD_ARGS	KCmdInf;
	unsigned long 		dwRet;
	KCmdInf.dwCmdParm1 = nDMASize;
	KCmdInf.dwCmdParm2 = 0;
	if (hDevice != NULL)
	{
		TLV_DeviceIoControl(hDevice,
			IOCTL_TVB350_EPLD_DOWN,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&dwRet, 0);
	}
	return (int)KCmdInf.dwCmdParm2;

#else
	return 0;
#endif
}

/*^^***************************************************************************
 * Description : Read from the DMA address
 *				
 * Entry : dwDMASize, upto 1Mbytes
 *
 * Return: NULL(fail), DMA address(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
void* _stdcall CLldDmaDrv::TSPL_READ_BLOCK(long dwDMASize)
{
	int	dwStatus = 0;

	LldPrint_Trace("TSPL_READ_BLOCK", dwDMASize, (int)0, (double)0, NULL);
//#ifdef WIN32
	//LockDmaMutex();
	//fDMA_Busy = 1;
//#endif
	
#ifdef WIN32
#else
	DWORD		dwRet;
	KCMD_ARGS	KCmdInf;
	if ( IsAttachedBdTyp_UsbTyp() ) 
	{
		if (dwpDMABufLinearAddr != NULL)
		{
			if ( CHK_DEV(hDevice_usb) )
			{
				// 9054(0x90):CAPTURE DMA MODE:DMA ch0 descriptor pointer
				if (Set_PCI9054_Reg_usb(hDevice_usb, 36, 0x00000009))
				{
					// 9054(0x8c): set DMA transfer size in bytes
					if (Set_PCI9054_Reg_usb(hDevice_usb,35,dwDMASize))
					{
						if (Set_PCI9054_Reg_usb(hDevice_usb, 42, 0x00000003))
						{
							dwpDMABufLinearAddr = (DWORD*)TSPL_GET_DATA(dwDMASize);
					
    						//fDMA_Busy = 0;
							//UnlockDmaMutex();
							return dwpDMABufLinearAddr;
						}
						else
						{
							dwStatus = -1;
							LldPrint_Error("[LLD]READ BLOCK FAILED :: START \n", 0, 0);
						}
					}
					else
					{
						dwStatus = -1;
						LldPrint_Error("[LLD]READ BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
					}
				}
				else
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]READ BLOCK FAILED :: MODE SET\n", 0, 0);
				}
			}
			else
			{
				dwStatus = -1;
				LldPrint_Error("[LLD]READ BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]READ BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
		}

    	//fDMA_Busy = 0;
		//UnlockDmaMutex();

    	if (dwStatus == 0)
		{
			return dwpDMABufLinearAddr;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		if ( !dwpDMABufLinearAddr )
		{
			dwpDMABufLinearAddr = (DWORD*)malloc(dwDMASize);
			if (!dwpDMABufLinearAddr)
			{
				//fDMA_Busy = 0;
				//UnlockDmaMutex();
				return NULL;	
			}
		}
	}
#endif

	if (dwpDMABufLinearAddr != NULL)
	{
		if ( CHK_DEV(hDevice) )
		{
			// 9054(0x90):CAPTURE DMA MODE:DMA ch0 descriptor pointer
			if (Set_PCI9054_Reg(hDevice, 36, 0x00000009))
			{
				// 9054(0x8c): set DMA transfer size in bytes
				if (Set_PCI9054_Reg(hDevice,35,dwDMASize))
				{
					if (Set_PCI9054_Reg(hDevice, 42, 0x00000003))
					{
#ifdef WIN32
						//sskim20071017 - SMP
						#ifdef	TSPLLD0431_EXPORTS
						/*
						SYSTEM_INFO sysInfo; 
						GetSystemInfo(&sysInfo);
						if ( sysInfo.dwNumberOfProcessors > 1 )
						*/
						{
							while ( !TSPL_GET_DMA_STATUS() ) Sleep(10);
						}
						#endif
#else
						if ( IsAttachedBdTyp_UsbTyp() ) 
						{
							dwpDMABufLinearAddr = (DWORD*)TSPL_GET_DATA(dwDMASize);
						}
						else
						{
							LockDmaMutex();
#if 1 
							do {
								Sleep(10);
								KCmdInf.dwCmdParm1 = 42;
								TLV_DeviceIoControl(hDevice,IOCTL_READ_PCI9054_REG,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);
								dwStatus = (DWORD)(KCmdInf.dwCmdParm2 & 0x00000010);
								dwStatus >>= 4;
							} while (!dwStatus);
#endif

							KCmdInf.dwCmdParm1 = dwDMASize;
							KCmdInf.pdwBuffer = (DWORD*)dwpDMABufLinearAddr;
							TLV_DeviceIoControl(hDevice,IOCTL_FILE_DATA_UP,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);

							UnlockDmaMutex();
						}
					
    		//			fDMA_Busy = 0;
						//UnlockDmaMutex();
						return dwpDMABufLinearAddr;
#endif
					}
					else
					{
						dwStatus = -1;
						LldPrint_Error("[LLD]READ BLOCK FAILED :: START \n", 0, 0);
					}
				}
				else
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]READ BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
				}
			}
			else
			{
				dwStatus = -1;
				LldPrint_Error("[LLD]READ BLOCK FAILED :: MODE SET\n", 0, 0);
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]READ BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
		}
	}
	else
	{
		dwStatus = -1;
		LldPrint_Error("[LLD]READ BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
	}

#ifdef WIN32
	//Ep(End Point) to Host
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( dwStatus == 0 )
		{
			dwpDMABufLinearAddr = (unsigned long*)TSPL_GET_DATA(dwDMASize);
		}
	}
#endif

//#ifdef WIN32
//    fDMA_Busy = 0;
//	UnlockDmaMutex();
//#endif

    if (dwStatus == 0)
	{
		return dwpDMABufLinearAddr;
	}
	else
	{
		return NULL;
	}
}

//PCIe - Memory copy
void	_stdcall CLldDmaDrv::TSPL_WRITE_BLOCK_TEST(DWORD *pdwSrcBuff, unsigned long dwBuffSize, DWORD *dest)
{
	KCMD_ARGS	KFileInf;
	DWORD		dwRet;

	LldPrint_Trace("TSPL_WRITE_BLOCK_TEST", dwBuffSize, (int)0, (double)0, NULL);
	if ( !CHK_DEV(hDevice) )
	{
//		LldPrint_Error("dma_drv : pos...", 3, 3);
		return;
	}
	WDM_WR2SDRAM(CARD_WR_START);

//2012/12/28
#ifndef WIN32
//LockDmaMutex();
#endif
	KFileInf.dwCmdParm2 = (dwBuffSize << 2);	// ~256Kx32 bitWords
	KFileInf.dwCmdParm1 = (ULONG)dest;
	KFileInf.pdwBuffer = pdwSrcBuff;	// ~1 Mbytes
	
	TLV_DeviceIoControl(hDevice, IOCTL_FILE_DATA_DOWN, &KFileInf, sizeof(KFileInf),NULL, 0,&dwRet,0);
//2012/12/28
#ifndef WIN32
//UnlockDmaMutex();
#endif

	WDM_WR2SDRAM(CARD_WR_END);

	return;
}

/*^^***************************************************************************
 * Description : Copy 1Mbyte to the DMA address
 *				
 * Entry : dwDMASize, upto 1Mbytes
 *
 * Return: -1(fail), 0(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
#ifdef WIN32
int _stdcall CLldDmaDrv::TSPL_WRITE_BLOCK(DWORD dwBuffSize,DWORD *dest)
#else
int CLldDmaDrv::TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, ULONG dwBuffSize, DWORD *dest)
#endif
{
	int dwStatus = 0;

	//LldPrint_Trace("TSPL_WRITE_BLOCK call", (int)dwBuffSize, (int)dest, (double)0, NULL);
//#ifdef WIN32
	//LockDmaMutex();
	//fDMA_Busy = 1;
//#endif

#ifdef WIN32
#else
	int dwRet;
	KCMD_ARGS	KCmdInf;
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		//2010/10/1 PCI/USB MULTIBOARD
		if (dwpDMABufLinearAddr != NULL)
		{
			if ( CHK_DEV(hDevice_usb) )
			{
				// 9054(0x90): PLAY mode:DMA ch0 descriptor pointer
				if (Set_PCI9054_Reg_usb(hDevice_usb, 36, 0x00000001))
				{
					// 9054(0x8c): set DMA transfer size in bytes
					if (Set_PCI9054_Reg_usb(hDevice_usb, 35, dwBuffSize))
					{
						if (Set_PCI9054_Reg_usb(hDevice_usb, 42, 0x00000003))
						{
							if  (dwpDMABufLinearAddr == NULL )
							{
								//fDMA_Busy = 0;
								//UnlockDmaMutex();
								LldPrint_Error("dma_drv : pos...", 3, 4);
								return -1;
							}
				
							memcpy(dwpDMABufLinearAddr, pdwSrcBuff, dwBuffSize);
							TSPL_PUT_DATA(dwBuffSize, dest);
							dwStatus = 0;
						}
						else
						{
							dwStatus = -1;
						 	LldPrint_Error("[LLD]WRITE BLOCK FAILED :: START \n", 0, 0);
				       	}
					}
					else
					{
						dwStatus = -1;
						LldPrint_Error("[LLD]WRITE BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
					}
				}
				else
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]WRITE BLOCK FAILED :: MODE SET\n", 0, 0);
				}
			}
			else
			{
				dwStatus = -1;
				LldPrint_Error("[LLD]WRITE BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]WRITE BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
		}

		//fDMA_Busy = 0;
		//UnlockDmaMutex();

	    return dwStatus;
	}
	else
	{
		if ( !dwpDMABufLinearAddr )
		{
			//2011/4/20 TEST
			//dwpDMABufLinearAddr = (DWORD*)malloc((dwBuffSize<<2));
			dwpDMABufLinearAddr = (DWORD*)malloc((dwBuffSize));
			if (!dwpDMABufLinearAddr)
			{
				//fDMA_Busy = 0;
				//UnlockDmaMutex();
			}
			//2011/4/20 
			return 0;	
		}
		if(!pdwSrcBuff)
			return 0;

		LockDmaMutex();
		//2011/4/20 TEST
		//KCmdInf.dwCmdParm1 = (dwBuffSize<<2);
		KCmdInf.dwCmdParm1 = (dwBuffSize);
		KCmdInf.pdwBuffer = (DWORD*)pdwSrcBuff;
		TLV_DeviceIoControl(hDevice,IOCTL_FILE_DATA_DOWN,&KCmdInf,sizeof(KCmdInf),NULL,0,&dwRet,0);
		UnlockDmaMutex();
	}
#endif

	if (dwpDMABufLinearAddr != NULL)
	{
		if ( CHK_DEV(hDevice) )
		{
			// 9054(0x90): PLAY mode:DMA ch0 descriptor pointer
			if (Set_PCI9054_Reg(hDevice, 36, 0x00000001))
			{
				// 9054(0x8c): set DMA transfer size in bytes
				if (Set_PCI9054_Reg(hDevice, 35, dwBuffSize))
				{
					if (Set_PCI9054_Reg(hDevice, 42, 0x00000003))
					{
#ifdef WIN32
						//sskim20071017 - SMP
						#ifdef	TSPLLD0431_EXPORTS
						/*
						SYSTEM_INFO sysInfo; 
						GetSystemInfo(&sysInfo);
						if ( sysInfo.dwNumberOfProcessors > 1 )
						*/
						{
							while ( !TSPL_GET_DMA_STATUS() )
							{
								Sleep(10);
							}
						}
						#endif

#else
						if ( IsAttachedBdTyp_UsbTyp() ) 
						{
							if  (dwpDMABufLinearAddr == NULL )
							{
								//fDMA_Busy = 0;
								//UnlockDmaMutex();
								LldPrint_Error("dma_drv : pos...", 3, 5);
								return -1;
							}
				
							memcpy(dwpDMABufLinearAddr, pdwSrcBuff, dwBuffSize);
							TSPL_PUT_DATA(dwBuffSize, dest);
						}
						else
						{
//2012/12/28
#if 0
LockDmaMutex();
							do {
								Sleep(10);
								KCmdInf.dwCmdParm1 = 42;
								TLV_DeviceIoControl(hDevice,IOCTL_READ_PCI9054_REG,&KCmdInf,sizeof(KCmdInf),&KCmdInf,sizeof(KCmdInf),&dwRet, 0);
								dwStatus = (DWORD)(KCmdInf.dwCmdParm2 & 0x00000010);
								dwStatus >>= 4;
							} while (!dwStatus);
//2012/12/28
UnlockDmaMutex();
#endif
						}
						dwStatus = 0;
#endif
					}
					else
					{
						dwStatus = -1;
					 	LldPrint_Error("[LLD]WRITE BLOCK FAILED :: START \n", 0, 0);
			       	}
				}
				else
				{
					dwStatus = -1;
					LldPrint_Error("[LLD]WRITE BLOCK FAILED :: TRANSFER SIZE\n", 0, 0);
				}
			}
			else
			{
				dwStatus = -1;
				LldPrint_Error("[LLD]WRITE BLOCK FAILED :: MODE SET\n", 0, 0);
			}
		}
		else
		{
			dwStatus = -1;
			LldPrint_Error("[LLD]WRITE BLOCK FAILED :: LOAD DEVICE\n", 0, 0);
		}
	}
	else
	{
		dwStatus = -1;
		LldPrint_Error("[LLD]WRITE BLOCK FAILED :: INVALID DMA ADDRESS\n", 0, 0);
	}

#ifdef WIN32
	//Host to Ep(End Point)
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		if ( dwStatus == 0 )
		{
			TSPL_PUT_DATA(dwBuffSize, dest);
		}
	}
#endif

//#ifdef WIN32
    //fDMA_Busy = 0;
	//UnlockDmaMutex();
//#endif
	LldPrint_Trace("TSPL_WRITE_BLOCK return", (int)dwStatus, (int)fDMA_Busy, (double)0, NULL);
    return dwStatus;
}

/*^^***************************************************************************
 * Description : Get the allocated DMA address
 *				
 * Entry : 
 *
 * Return: NULL(fail), DMA address(success)
 *
 * Notes :  
 *
 **************************************************************************^^*/
void* _stdcall CLldDmaDrv::TSPL_GET_DMA_ADDR(void)
{
	LldPrint_Trace("TSPL_GET_DMA_ADDR", 0, (int)0, (double)0, NULL);
	return dwpDMABufLinearAddr;
}

/*^^***************************************************************************
 * Description : Get the DMA register information
 *				
 * Entry : nOffset
 *
 * Return: regster value
 *
 * Notes :  
 *
 **************************************************************************^^*/
int _stdcall CLldDmaDrv::TSPL_GET_DMA_REG_INFO(int nOffset)
{
	KCMD_ARGS	KCmdInf;
	int			dwStatus;
	DWORD		dwRet;

	LldPrint_Trace("TSPL_GET_DMA_REG_INFO", nOffset, (int)0, (double)0, NULL);
	LockDmaMutex();
	KCmdInf.dwCmdParm1 = nOffset;
//2010/10/1 PCI/USB MULTIBOARD
#ifdef WIN32
	TLV_DeviceIoControl(hDevice,
			IOCTL_READ_PCI9054_REG,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&dwRet, 0);
#else
	if ( IsAttachedBdTyp_UsbTyp() )
	{
		TLV_DeviceIoControl_usb(hDevice_usb,
			IOCTL_READ_PCI9054_REG,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&dwRet, 0);
	}
	else
	{
		TLV_DeviceIoControl(hDevice,
			IOCTL_READ_PCI9054_REG,
			&KCmdInf,
			sizeof(KCmdInf),
			&KCmdInf,
			sizeof(KCmdInf),	
			&dwRet, 0);
	}
#endif
	dwStatus = (DWORD)(KCmdInf.dwCmdParm2);

	UnlockDmaMutex();
	LldPrint_Trace("TSPL_GET_DMA_REG_INFO", nOffset, dwStatus, 0, NULL);

	return dwStatus;
}

/*^^***************************************************************************
 * Description : reset the DMA register information
 *				
 * Entry : nOffset, dwData
 *
 * Return: 1
 *
 * Notes :  
 *
 **************************************************************************^^*/
int _stdcall CLldDmaDrv::TSPL_SET_DMA_REG_INFO(unsigned char nOffset, DWORD dwData)
{
	KCMD_ARGS	KRegInf;
	DWORD 		dwRet;

	LldPrint_Trace("TSPL_SET_DMA_REG_INFO", nOffset, (int)dwData, (double)0, NULL);
	LockDmaMutex();
	KRegInf.dwCmdParm1 = nOffset;	// 0~0x3f
	KRegInf.dwCmdParm2 = dwData;	// 32 bit value

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
	return 1;
}

/*^^***************************************************************************
 * Description : Demux block test API only for internal usage
 *				
 * Entry : nState
 *
 * Return: 0
 *
 * Notes :  
 *
 **************************************************************************^^*/
int CLldDmaDrv::TSPL_SET_DEMUX_CONTROL_TEST(int nState)
{
	/*
	bit 3 : 0(normal mode) 
			1:(test mode)
	bit 2 : 0(capture, normal data input) 
			1:(capture, FPGA data generation)
	bit 1-0 : 00(play, check-sum at atPLX9054)
				01(play, check-sum before writing to SDRAM)
				10(play, check-sum after reading from SDRAM)
				11(play, check-sum before output to ASI or 310M port)
	*/			
	int	dwStatus;
	int	dwControl = 0;

	LldPrint_Trace("TSPL_SET_DEMUX_CONTROL_TEST", nState, (int)0, (double)0, NULL);
	//TVB593, TVB497
	if ( IsAttachedBdTyp_HaveDmxCmtlTest() )
	{
		dwStatus = WDM_WR2SDRAM_ADDR(TSP_MEM_ADDR_DEMUX_CNTL_TEST, nState);
		LldPrint_2("[LLD]DEMUX CONTROL TEST, Return, State", dwStatus, nState);

		Sleep(12);
		TSPL_RESET_SDCON_backwardcompatible(3);
	}

	return 0;
}


