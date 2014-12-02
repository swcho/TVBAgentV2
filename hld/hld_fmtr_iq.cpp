
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<string.h>
#if defined(WIN32)
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include	<stdlib.h>
#include	"../include/lld_const.h"
#endif

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_fmtr_iq.h"

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrIq::CHldFmtrIq(void)
{
	//I/Q PLAY/CAPTURE
	TL_gFPGA_ID = 0;
	TL_gFPGA_VER = 0;
	TL_gFPGA_IQ_Play = 0;
	TL_gFPGA_IQ_Capture = 0;
	TL_gIQ_Play_Capture = 0;
	TL_gIQ_Memory_Based = 0;
	TL_gIQ_Memory_Size = 0;
	gIQBuffer = NULL;
	gIQRead = 0;
	gIQWrite = 0;
	gIQCount = 0;
	gIQBufferSize = 0;
	gFreeDiskSize = 0;

	gBandWidth_Symbolrate = 0;
	TL_gIQ_Capture_Size = 0;
	
}

CHldFmtrIq::~CHldFmtrIq()
{
}
void	CHldFmtrIq::SetCommonMethod_3(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

//2011/11/16 IQ NEW FILE FORMAT
void CHldFmtrIq::SetSymbolrate(long _val)
{
	gBandWidth_Symbolrate = _val;
}
long CHldFmtrIq::GetSymbolrate(void)
{
	long nRet = 0;
	if(__Sta__->IsModTyp_DvbT() || __Sta__->IsModTyp_DvbH())
	{
		if(gBandWidth_Symbolrate == 0)
		{
			nRet = (6 * 8 * 1000000) / 7;
		}
		else if(gBandWidth_Symbolrate == 1)
		{
			nRet = (7 * 8 * 1000000) / 7;
		}
		else if(gBandWidth_Symbolrate == 2)
		{
			nRet = (8 * 8 * 1000000) / 7;
		}
		else if(gBandWidth_Symbolrate == 3)
		{
			nRet = (5 * 8 * 1000000) / 7;
		}
		else	//default
		{
			nRet = (6 * 8 * 1000000) / 7;
		}
	}
	else if(__Sta__->IsModTyp_Vsb())
	{
		nRet = 10762238;
	}
	else if(__Sta__->IsModTyp_Dtmb())
	{
		nRet = 1512000;
	}
	else
	{
		nRet = gBandWidth_Symbolrate * 2;
	}
	return nRet;
}

//---------------------------------------------------------------------------
// Return Free Disk Size in Second
__int64 CHldFmtrIq::GetFreeDiskBytes(char *strFilename)
{
#ifdef WIN32
	ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes, i64FreeBytes;
	char	szDrv[10];
	BOOL	fResult;
	__int64	dblReturn = -1; 

	strncpy(szDrv, strFilename, 3);		// D:/
	szDrv[3] = '\0';
	fResult = GetDiskFreeSpaceEx (szDrv, (PULARGE_INTEGER)&i64FreeBytesToCaller,
										(PULARGE_INTEGER)&i64TotalBytes,
										(PULARGE_INTEGER)&i64FreeBytes);
	if (fResult == false) 
		return dblReturn;



	dblReturn = (__int64) (i64FreeBytes.QuadPart);
    return dblReturn;
#else
	__int64 	i64FreeBytes;
	struct statvfs fiData;
	char fnPath[256];

    strcpy(fnPath, "/");
    if((statvfs(fnPath,&fiData)) < 0 ) 
	{
		 printf("********* Failed to stat %s:\n", fnPath);
		 return 0;		// 10 hours
	} else {
		 i64FreeBytes = (__int64) fiData.f_bsize * fiData.f_bfree;	
	}
    return i64FreeBytes;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////	state
void CHldFmtrIq::SetFlag_IqPlayCap(unsigned int _val)
{
	TL_gIQ_Play_Capture = _val;
}
unsigned int CHldFmtrIq::Flag_IqPlayCap(void)
{
	return	TL_gIQ_Play_Capture;
}
void CHldFmtrIq::Prepare_IQPlayCapBuf_OnRecStart(void)
{
	unsigned long dwPageCount = 0;
	unsigned long dwAllocSize = 0;

#ifdef WIN32
	SYSTEM_INFO sysInfo;
#endif

	long symbolrate;


	if ( TL_gIQ_Play_Capture == 1 )
	{
		symbolrate = GetSymbolrate();
		__FIf__->TL_pbBufferTmp[0] = 0x49;
		__FIf__->TL_pbBufferTmp[1] = 0x51;
		__FIf__->TL_pbBufferTmp[2] = 0x31;
		__FIf__->TL_pbBufferTmp[3] = 0x34;
		__FIf__->TL_pbBufferTmp[4] = (unsigned char)((symbolrate >> 24) & 0xFF);
		__FIf__->TL_pbBufferTmp[5] = (unsigned char)((symbolrate >> 16) & 0xFF);
		__FIf__->TL_pbBufferTmp[6] = (unsigned char)((symbolrate >> 8) & 0xFF);
		__FIf__->TL_pbBufferTmp[7] = (unsigned char)(symbolrate & 0xFF);

		gFreeDiskSize = GetFreeDiskBytes(__FIf__->AP_szfnRecordFile);
		gFreeDiskSize = gFreeDiskSize - (512 * 1024 * 1024);

		long nRet;
#if defined(WIN32)
		WriteFile(__FIf__->AP_hFile, (unsigned char *)__FIf__->TL_pbBufferTmp, 8, (unsigned long*)&nRet, NULL);
#else
		nRet = fwrite((unsigned char *)__FIf__->TL_pbBufferTmp, 1, 8, (FILE*)__FIf__->AP_hFile);
#endif

		if ( gIQBuffer == NULL && TL_gIQ_Memory_Based == 1 )
		{
#ifdef WIN32
			GetSystemInfo(&sysInfo);
			dwPageCount = (DWORD)(TL_gIQ_Memory_Size / sysInfo.dwPageSize);
			dwAllocSize = dwPageCount * sysInfo.dwPageSize;

			gIQBuffer = (BYTE*) VirtualAlloc(NULL, dwAllocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
			dwPageCount = (unsigned long)(TL_gIQ_Memory_Size / getpagesize());
			dwAllocSize = dwPageCount * getpagesize();

			gIQBuffer = (BYTE*)malloc(dwAllocSize);
#endif
			if ( gIQBuffer != NULL )
			{
				gIQBufferSize = dwAllocSize;
			}
			else
			{
				gIQBufferSize = 0;
			}
			__HLog__->HldPrint_1("Hld-Bd-Ctl. I/Q MEMORY PLAY BUFFER SIZE MB", gIQBufferSize/0x100000);
		}

		gIQWrite = gIQRead = gIQCount = 0;
		sprintf(debug_string, "Hld-Bd-Ctl. I/Q MEMORY CAPTURE BUFFER=%d MB\n", gIQBufferSize/0x100000);
		__HLog__->HldPrint( debug_string);

	}
}
void CHldFmtrIq::Free_IQPlayCapBuf_OnMonStart(void)
{
#ifdef WIN32
	if ( gIQBuffer )
	{
		VirtualFree(gIQBuffer, 0, MEM_RELEASE);
		gIQBuffer = NULL;
	}
#else
	if ( gIQBuffer )
	{
		free(gIQBuffer);
		gIQBuffer = NULL;
	}
#endif

	if ( TL_gIQ_Play_Capture == 1 )
	{
		__HLog__->HldPrint_2( "Hld-Bd-Ctl. I/Q MEMORY PLAY/CAPTURE, TSIO ", TL_gIQ_Play_Capture, __Sta__->TL_InputDirection);

		if ( __Sta__->IsAsior310_LoopThru_DtaPathDirection() )
		{
			__FIf__->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_CAPTURE_MODE);
		}
		else
		{
			__FIf__->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_PLAY_MODE);
		}
	}
}
int CHldFmtrIq::Capture_IQData_UntilStopCond__HasEndlessWhile(void)
{
	int		i, nRet;
	unsigned long dwRet = 0;
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

	if ( TL_gIQ_Play_Capture == 1 )
	{
		dwRet = 16*SUB_BANK_MAX_BYTE_SIZE;//SKIP THE FIRST 16MB
		while ( __Sta__->IsTaskState_ContRec() && !__Sta__->ReqedNewAction_User() )
		{
			for (i = 0; (i < __Sta__->iHW_BANK_NUMBER+1) && (__Sta__->IsTaskState_ContRec()) && !__Sta__->ReqedNewAction_User(); i++)
			{
				__FIf__->TL_pdwDMABuffer = (DWORD*)__FIf__->TSPL_READ_BLOCK(nBankBlockSize);

				if ( gIQBuffer != NULL && TL_gIQ_Memory_Based == 1 )
				{
					if ( __FIf__->TL_pdwDMABuffer && gIQCount < gIQBufferSize )
					{
						__FIf__->WaitDmaTrans_CaptureDirection__HasEndlessWhile();
						if ( dwRet <= 0 )
						{
							memcpy(gIQBuffer+gIQWrite, ((unsigned char *)__FIf__->TL_pdwDMABuffer), nBankBlockSize);
							gIQWrite += nBankBlockSize;
							gIQCount += nBankBlockSize;
						}
						else
						{
							dwRet -= nBankBlockSize;
						}

						__FIf__->TL_i64TotalFileSize += nBankBlockSize;
						__FIf__->TL_gTotalSendData += nBankBlockSize;
					}

					if ( gIQCount >= gIQBufferSize )
					{
						while ( gIQCount >= (unsigned long)nBankBlockSize )
						{
#if defined(WIN32)
							WriteFile(__FIf__->AP_hFile, gIQBuffer+gIQRead, nBankBlockSize, (unsigned long*)&nRet, NULL);
#else
							nRet = fwrite(gIQBuffer+gIQRead, 1, nBankBlockSize, (FILE*)__FIf__->AP_hFile);
#endif

							gIQRead += nBankBlockSize;
							gIQCount -= nBankBlockSize;
							//__FIf__->TL_i64TotalFileSize += nRet;
							//__FIf__->TL_gTotalSendData += nRet;
							if(gIQRead >= gFreeDiskSize)
							{
								break;
							}
						}

						sprintf(debug_string, "Hld-Bd-Ctl. I/Q MEMORY CAPTURE BUFFER FLUSHED=%d MB\n", __FIf__->TL_gTotalSendData/0x100000);
						__HLog__->HldPrint( debug_string);

#ifdef WIN32
						CloseHandle(__FIf__->AP_hFile);
#else
						fclose((FILE *)__FIf__->AP_hFile);
#endif
						__FIf__->AP_hFile = INVALID_HANDLE_VALUE;

						__Sta__->SetMainTask_LoopState_(TH_START_MON);
						break;
					}
				}
				else
				{
					__FIf__->WaitDmaTrans_CaptureDirection__HasEndlessWhile();

					//SKIP THE FIRST 16MB
					if ( __FIf__->TL_i64TotalFileSize < dwRet )
					{
						__FIf__->TL_i64TotalFileSize += nBankBlockSize;
					}
					else
					{
#if defined(WIN32)
						WriteFile(__FIf__->AP_hFile, __FIf__->TL_pdwDMABuffer, nBankBlockSize, (unsigned long*)&nRet, NULL);
#else
						nRet = fwrite(__FIf__->TL_pdwDMABuffer, 1, nBankBlockSize, (FILE*)__FIf__->AP_hFile);
#endif
						__FIf__->TL_i64TotalFileSize += nRet;
						__FIf__->TL_gTotalSendData += nRet;
						
						//2011/11/22 IQ NEW FILE FORMAT
						if(TL_gIQ_Capture_Size > 0)
						{
							if(TL_gIQ_Capture_Size <= (unsigned int)(__FIf__->TL_gTotalSendData / (1024 * 1024)))
							{
#ifdef WIN32
								CloseHandle(__FIf__->AP_hFile);
#else	
								fclose((FILE *)__FIf__->AP_hFile);
#endif
								__FIf__->AP_hFile = INVALID_HANDLE_VALUE;
		
								__Sta__->SetMainTask_LoopState_(TH_START_MON);
								return 0;
							}
						}
						//Until 512MB
						if(__FIf__->TL_gTotalSendData >= gFreeDiskSize)
						{
#ifdef WIN32
							CloseHandle(__FIf__->AP_hFile);
#else	
							fclose((FILE *)__FIf__->AP_hFile);
#endif
							__FIf__->AP_hFile = INVALID_HANDLE_VALUE;
		
							__Sta__->SetMainTask_LoopState_(TH_START_MON);
							return 0;
						}
					}
				}
			}
		}

		return 0;
	}

	return 1;
}
int CHldFmtrIq::Play_IQData_UntilStopContPlay__HasEndlessWhile(void)
{
//#ifdef WIN32
	unsigned long dwRet;
//	__HLog__->HldPrint( "=====KSLEE 3======");
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());
	if ( TL_gIQ_Play_Capture == 1 && TL_gIQ_Memory_Based == 1 && gIQBuffer)
	{
		__FIf__->next_file_ready = FALSE;
		while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
		{
			__FIf__->TL_dwBytesRead = 0;
			if(__FIf__->TL_nSubBankIdx == 0)
			{
				__FIf__->TL_dwAddrDestBoardSDRAM = (__FIf__->TL_nIdCurBank == 0? 0 : BANK_SIZE_4);
			}

			if(gIQRead <= ( gIQBufferSize - nBankBlockSize ))
			{
				memcpy(__FIf__->TL_pbBufferTmp, gIQBuffer + gIQRead, nBankBlockSize);
				gIQRead += nBankBlockSize;
			}
			else
			{
				dwRet = gIQBufferSize - gIQRead;

				memcpy(__FIf__->TL_pbBufferTmp, gIQBuffer + gIQRead, dwRet);
				memcpy(__FIf__->TL_pbBufferTmp + dwRet, gIQBuffer, nBankBlockSize - dwRet);
				gIQRead = nBankBlockSize - dwRet;

				/* EOF */
				__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
			}
#if defined(WIN32)				
			memcpy((void *)__FIf__->TL_pdwDMABuffer, __FIf__->TL_pbBufferTmp, nBankBlockSize);
			__FIf__->TSPL_WRITE_BLOCK( NULL, (unsigned long )nBankBlockSize, (unsigned long *)__FIf__->TL_dwAddrDestBoardSDRAM);
#else
			__FIf__->TSPL_WRITE_BLOCK( (DWORD *)__FIf__->TL_pbBufferTmp, (unsigned long )nBankBlockSize, (DWORD *)__FIf__->TL_dwAddrDestBoardSDRAM);
#endif
			while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
			{
				if ( __FIf__->TSPL_GET_DMA_STATUS() )
					break;
				//Sleep(10);
				Sleep(0);
			}
			__FIf__->TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
			__FIf__->TL_CheckBufferStatus(1, __FIf__->GetPlayParam_PlayRate());
			if (++__FIf__->TL_nSubBankIdx == (__Sta__->iHW_BANK_NUMBER+1))
			{
				__FIf__->TL_nIdCurBank = (__FIf__->TL_nIdCurBank + 1) & 0x01;
				__FIf__->TL_nSubBankIdx = 0;
			}
		}
		return 0;
	}
	//I/Q PLAY/CAPTURE - RAID/HDD
//	__HLog__->HldPrint( "=====KSLEE 4======");
	if ( TL_gIQ_Play_Capture == 1 && TL_gIQ_Memory_Based == 0 )
	{
//		__HLog__->HldPrint( "=====KSLEE 5 ======");
#ifdef WIN32
		if ( __FIf__->AP_hFile == INVALID_HANDLE_VALUE)
		{
			__FIf__->AP_hFile = CreateFile(__FIf__->PlayParm.AP_lst.szfn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
			if ( __FIf__->AP_hFile == INVALID_HANDLE_VALUE)
			{
//				__HLog__->HldPrint( "=====KSLEE 6 ======");
				__Sta__->SetMainTask_LoopState_(TH_NONE);
				return 0;
			}

			dwRet = SetFilePointer(__FIf__->AP_hFile, _IQ_HEADER_SIZE_, NULL, FILE_BEGIN);
		}
#else
		if ( __FIf__->AP_hFile == NULL)
		{
			__FIf__->AP_hFile = fopen(__FIf__->PlayParm.AP_lst.szfn, "rb");
			if ( __FIf__->AP_hFile == NULL)
			{
//				__HLog__->HldPrint( "=====KSLEE 6 ======");
				__Sta__->SetMainTask_LoopState_(TH_NONE);
				return 0;
			}

			dwRet = fseeko((FILE *)__FIf__->AP_hFile, _IQ_HEADER_SIZE_, SEEK_SET);
		}

#endif

		/* CHECK FLAG FOR NEXT FILE. IT MUST BE RESET BY APPLICATION. */
		__FIf__->next_file_ready = FALSE;
//		__HLog__->HldPrint( "=====KSLEE======");
		while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
		{
			//time(&ltime0);CHECK_TIME_START

//		__HLog__->HldPrint( "=====KSLEE 2 ======");
			__FIf__->TL_dwBytesRead = 0;
			memset(__FIf__->TL_szBufferPlay, 0x00, nBankBlockSize);
#if defined(WIN32)
			dwRet = ReadFile(__FIf__->AP_hFile, __FIf__->TL_szBufferPlay, nBankBlockSize, &(__FIf__->TL_dwBytesRead), NULL);
#else
			__FIf__->TL_dwBytesRead = fread(__FIf__->TL_szBufferPlay, 1, nBankBlockSize, (FILE *)__FIf__->AP_hFile);
#endif

			/* EOF */
			if (__FIf__->TL_dwBytesRead < (unsigned int)(nBankBlockSize))
			{
				__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
				continue;
			}

			if (__FIf__->TL_nSubBankIdx == 0)
			{
				__FIf__->TL_dwAddrDestBoardSDRAM = (__FIf__->TL_nIdCurBank == 0? 0 : BANK_SIZE_4);
			}

			// DMA START 
#if defined(WIN32)				
			memcpy((void *) __FIf__->TL_pdwDMABuffer, __FIf__->TL_szBufferPlay, nBankBlockSize);
			__FIf__->TSPL_WRITE_BLOCK( NULL, (unsigned long )nBankBlockSize, (unsigned long *)__FIf__->TL_dwAddrDestBoardSDRAM);
#else
			__FIf__->TSPL_WRITE_BLOCK( (DWORD *)__FIf__->TL_szBufferPlay, (unsigned long )nBankBlockSize, (DWORD *)__FIf__->TL_dwAddrDestBoardSDRAM);
#endif
			while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
			{
				if ( __FIf__->TSPL_GET_DMA_STATUS() )
					break;
				//Sleep(10);
				Sleep(0);
			}

			__FIf__->TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
			__FIf__->TL_CheckBufferStatus(1, __FIf__->GetPlayParam_PlayRate());
			if (++__FIf__->TL_nSubBankIdx == (__Sta__->iHW_BANK_NUMBER+1))
			{
				__FIf__->TL_nIdCurBank = (__FIf__->TL_nIdCurBank + 1) & 0x01;
				__FIf__->TL_nSubBankIdx = 0;
			}

		}

		return 0;
	}
//#endif
	return 1;
}

int CHldFmtrIq::Play_IQData_UntilStopCond__HasEndlessWhile(void)
{
	unsigned long dwRet;

	unsigned long dwPageCount = 0;
	unsigned long dwAllocSize = 0;
#ifdef WIN32
	SYSTEM_INFO sysInfo;
#endif
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());

#ifdef WIN32
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	//__HLog__->HldPrint("now GlobalMemoryStatus dwTotalVirtual=%ld, dwAvailVirtual=%ld\n", memStatus.dwTotalVirtual, memStatus.dwAvailVirtual);
#endif
	if ( TL_gIQ_Play_Capture == 1 )
	{
		if ( gIQBuffer == NULL && TL_gIQ_Memory_Based == 1 )
		{
#ifdef WIN32
			__FIf__->AP_hFile = CreateFile(__FIf__->PlayParm.AP_lst.szfn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
			if ( __FIf__->AP_hFile == INVALID_HANDLE_VALUE)
			{
				__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open a PLAY FILE ", __FIf__->PlayParm.AP_lst.szfn);
				__Sta__->SetMainTask_LoopState_(TH_NONE);
				return 1;
			}
			GetSystemInfo(&sysInfo);
			dwPageCount = (DWORD)(TL_gIQ_Memory_Size / sysInfo.dwPageSize);
			dwAllocSize = dwPageCount * sysInfo.dwPageSize;
			__HLog__->HldPrint("VirtualAlloc  bytes, dwPageCount, sysInfo.dwPageSize", dwAllocSize, dwPageCount, sysInfo.dwPageSize);
#else
			__FIf__->AP_hFile = fopen(__FIf__->PlayParm.AP_lst.szfn, "rb");
			if ( __FIf__->AP_hFile == NULL)
			{
				__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open a PLAY FILE ", __FIf__->PlayParm.AP_lst.szfn);
				__Sta__->SetMainTask_LoopState_(TH_NONE);
				return 1;
			}
			dwPageCount = (unsigned long)(TL_gIQ_Memory_Size / getpagesize());
			dwAllocSize = dwPageCount * getpagesize();
			printf("VirtualAlloc  %d bytes, dwPageCount %d, sysInfo.dwPageSize %d , nBankBlockSize %d\n", dwAllocSize, dwPageCount, getpagesize(),nBankBlockSize);
#endif


			if ( gIQBuffer == NULL )
			{
#ifdef WIN32
				gIQBuffer = (BYTE*) VirtualAlloc(NULL, dwAllocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
				gIQBuffer = (unsigned char*) malloc(dwAllocSize);
#endif
				if ( gIQBuffer != NULL )
				{
					gIQBufferSize = dwAllocSize;
				}
				else
				{
					gIQBufferSize = 0;
				}
				__HLog__->HldPrint_1("Hld-Bd-Ctl. I/Q MEMORY PLAY BUFFER SIZE MB", gIQBufferSize/0x100000);
			}
			gIQWrite = gIQRead = gIQCount = 0;

			while ( gIQWrite < gIQBufferSize )
			{
#if defined(WIN32)
				dwRet = ReadFile(__FIf__->AP_hFile, gIQBuffer + gIQWrite, nBankBlockSize, &(__FIf__->TL_dwBytesRead), NULL);
#else
				__FIf__->TL_dwBytesRead = fread(gIQBuffer + gIQWrite, 1, nBankBlockSize, (FILE *)__FIf__->AP_hFile);
#endif

				if ( __FIf__->TL_dwBytesRead != (unsigned int)nBankBlockSize )
					break;

				gIQWrite += __FIf__->TL_dwBytesRead;
				gIQCount += __FIf__->TL_dwBytesRead;
			}
			gIQBufferSize = gIQCount;

#if defined(WIN32)
			if ( __FIf__->AP_hFile != INVALID_HANDLE_VALUE )
			{
				CloseHandle(__FIf__->AP_hFile);
				__FIf__->AP_hFile = INVALID_HANDLE_VALUE;
			}
#else
			if ( __FIf__->AP_hFile != NULL )
			{
				fclose((FILE *)__FIf__->AP_hFile);
				__FIf__->AP_hFile = NULL;
			}
#endif
		}

		sprintf(debug_string, "Hld-Bd-Ctl. I/Q MEMORY PLAY BUFFERED=%d MB\n", gIQCount/0x100000);
		__HLog__->HldPrint( debug_string);
	}

	return	1;
}
int CHldFmtrIq::ChangeStaIq_OnPlayEnd(void)
{
//#ifdef WIN32
	if ( TL_gIQ_Play_Capture == 1 && TL_gIQ_Memory_Based == 1 && gIQBuffer )
	{
		__Sta__->SetMainTask_LoopState_(TH_CONT_PLAY);
		__FIf__->TL_CheckBufferStatus(2, __FIf__->GetPlayParam_PlayRate());

		__FIf__->TL_gFirstRead = 1;
		__FIf__->TL_gNumLoop++;
		////ssert__FIf__->AP_hFile == INVALID_HANDLE_VALUE)
	}
//#endif

	return	1;
}
int CHldFmtrIq::SetFpgaModulatorTyp(long modulator_type, long IF_Frequency)
{
	int	gRet;

	gRet = __FIf__->TVB380_SET_CONFIG(modulator_type, IF_Frequency);
	if(gRet >= 0)
		__Sta__->SetModulatorTyp(modulator_type);
	//I/Q PLAY/CAPTURE
	TL_gFPGA_ID = __FIf__->TSPL_GET_FPGA_INFO(0);
	TL_gFPGA_VER = __FIf__->TSPL_GET_FPGA_INFO(1);
	TL_gFPGA_IQ_Play = __FIf__->TSPL_GET_FPGA_INFO(2);
	TL_gFPGA_IQ_Capture = __FIf__->TSPL_GET_FPGA_INFO(3);
//	__HLog__->HldPrint("Hld-Bd-Ctl. FPGA::ID=0x%04X, VERSION=0x%02X, PLAY=%d, CAPTURE=%d\n", TL_gFPGA_ID, TL_gFPGA_VER, TL_gFPGA_IQ_Play, TL_gFPGA_IQ_Capture);

	return gRet;
}
int CHldFmtrIq::TryAlloc_IqMem(int mem_size)
{
	unsigned char *IQBuffer = NULL;
#ifdef WIN32
	SYSTEM_INFO sysInfo;
#endif
	unsigned long dwPageCount = 0;
	unsigned long dwAllocSize = 0;
#ifdef WIN32
	GetSystemInfo(&sysInfo);
#endif
    
#ifdef WIN32
	dwPageCount = (unsigned long)(mem_size*(1024*1024) / sysInfo.dwPageSize);
	dwAllocSize = dwPageCount * sysInfo.dwPageSize;
//    __HLog__->HldPrint("VirtualAlloc %ld bytes, dwPageCount=%ld, sysInfo.dwPageSize=%ld\n", dwAllocSize, dwPageCount, sysInfo.dwPageSize);
	IQBuffer = (BYTE*) VirtualAlloc(NULL, dwAllocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if ( IQBuffer )
	{
		VirtualFree(IQBuffer, 0, MEM_RELEASE);
		IQBuffer = NULL;

		return dwAllocSize / (1024*1024);
	}
#else
	dwPageCount = (unsigned long)(mem_size*(1024*1024) / getpagesize());
	dwAllocSize = dwPageCount * getpagesize();
	IQBuffer = (BYTE*) malloc(dwAllocSize);
	if ( IQBuffer )
	{
		free(IQBuffer);
		IQBuffer = NULL;

		return dwAllocSize / (1024*1024);
	}
#endif
					
	return 0;
}
int CHldFmtrIq::SetIqMode(int mode, int memory_use, int memory_size, int capture_size) //2011/11/22 IQ NEW FILE FORMAT
{
	TL_gIQ_Play_Capture = mode;
	TL_gIQ_Memory_Based = memory_use;
	TL_gIQ_Memory_Size = memory_size*1024*1024;
	TL_gIQ_Capture_Size = capture_size;

//	if ( !_IQ_SUPPORT_(TL_gFPGA_ID, TL_gFPGA_VER, TL_gFPGA_IQ_Play)
//		&& !_IQ_SUPPORT_(TL_gFPGA_ID, TL_gFPGA_VER, TL_gFPGA_IQ_Capture) )
	if(TL_gFPGA_IQ_Play == 0 && TL_gFPGA_IQ_Capture == 0)
	{
		TL_gIQ_Play_Capture = 0;
		return 0;
	}

	if ( TL_gIQ_Play_Capture == 1 )
	{
		if ( __Sta__->IsAsior310_LoopThru_DtaPathDirection() )
		{
			__FIf__->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_CAPTURE_MODE);
		}
		else
		{
			__FIf__->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_PLAY_MODE);
		}
	}
	else
	{
		__FIf__->TSPL_SET_SDCON_MODE(TSPL_SDCON_IQ_NONE);
	}

	return 0;
}


