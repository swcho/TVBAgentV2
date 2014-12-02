/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapper.cpp
    Desc.:  VLCWrapper Implementation.

    Author: Alex Skoruppa
    Date:   08/10/2009
    eM@il:  alex.skoruppa@googlemail.com

    VLCWrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
     
    VLCWrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
     
    You should have received a copy of the GNU General Public License
    along with VLCWrapper.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/
#include "VLCWrapper.h"
#include "VLCWrapperImpl.h"

#if defined(WIN32)
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>

#define KEY_NUM	5128

pthread_t	g_thread, g_thread1;
#endif

VLCWrapper::VLCWrapper(void)
{
    m_pImpl = new VLCWrapperImpl();

	//TEST - VIDEO WINDOW MOVE/RESIZE
	m_nVideo_X = 0;
	m_nVideo_Y = 0;
	m_nVideo_W = 0;
	m_nVideo_H = 0;

	m_nCurrentProgram = -1;
	g_VLC_Control = NULL;
	g_SharedBuffer = NULL;	
#if defined(WIN32)
	g_hMap = INVALID_HANDLE_VALUE;
	g_hMap1 = INVALID_HANDLE_VALUE;
	g_hMutex = INVALID_HANDLE_VALUE;
	g_hEvent = INVALID_HANDLE_VALUE;
#else
	g_hMutex = -1;
	g_hEvent.IsOk = 0;
#endif
	g_VLC_PumpingThreadDone = 1;
	g_PumpingWait = 1;
	g_VLC_ThreadDone = 1;
	g_VLC_Bufferring = 1;
	g_VLC_File = INVALID_HANDLE_VALUE;

#if defined(WIN32)
	GetCurrentDirectory(MAX_PATH, g_szCurDir);
#else
	getcwd(g_szCurDir, MAX_PATH);
#endif
}

VLCWrapper::~VLCWrapper(void)
{
	if ( m_pImpl )
	{
		delete m_pImpl;
		m_pImpl = NULL;
	}
}

void VLCWrapper::SetOutputWindow(void* pHwnd)
{
	//TEST
	if ( !m_pImpl )
		return;

    m_pImpl->SetOutputWindow(pHwnd);
}

void VLCWrapper::SetExceptionHandler(VLCExceptionHandler eh)
{
    m_pImpl->SetExceptionHandler(eh);
}

void VLCWrapper::SetEventHandler(VLCEventHandler evt)
{
    m_pImpl->SetEventHandler(evt);
}

void VLCWrapper::Play()
{
    m_pImpl->Play();
}

void VLCWrapper::Pause()
{
    m_pImpl->Pause();
}

void VLCWrapper::Stop()
{
	//TEST - VIDEO WINDOW MOVE/RESIZE
	int nRet;
	do
	{
		nRet = m_pImpl->GetPlayerState();
		if ( nRet == libvlc_Playing || nRet == libvlc_Paused || nRet == libvlc_Buffering )
		{
			break;
		}
		Sleep(100);
	} while (1);
	m_pImpl->DestroyWindow();
	m_nVideo_X = 0;
	m_nVideo_Y = 0;
	m_nVideo_W = 0;
	m_nVideo_H = 0;

    m_pImpl->Stop();
}

int64_t VLCWrapper::GetLength()
{
    return m_pImpl->GetLength();
}

int64_t VLCWrapper::GetTime()
{
    return m_pImpl->GetTime();
}

void VLCWrapper::SetTime(int64_t llNewTime)
{
    m_pImpl->SetTime(llNewTime);
}

void VLCWrapper::Mute(bool bMute)
{
    m_pImpl->Mute(bMute);
}

bool VLCWrapper::GetMute()
{
    return m_pImpl->GetMute();
}

int  VLCWrapper::GetVolume()
{
    return m_pImpl->GetVolume();
}

void VLCWrapper::SetVolume(int iVolume)
{
    m_pImpl->SetVolume(iVolume);
}

void VLCWrapper::OpenMedia(const char* pszMediaPathName)
{
    m_pImpl->OpenMedia(pszMediaPathName);
}

////////////////////////////////////////////////////////
int VLCWrapper::AddOption(char* pszOption)
{
    return m_pImpl->AddOption(pszOption);
}

float VLCWrapper::GetInputBitrate()
{
	return m_pImpl->GetInputBitrate();
}

int VLCWrapper::GetProgramInfo(int *pi)
{
	return m_pImpl->GetProgramInfo(pi);
}

int VLCWrapper::SetCurrentProgram(int current_pgm)
{
	return m_pImpl->SetCurrentProgram(current_pgm);
}

int VLCWrapper::isPlaying()
{
	return m_pImpl->isPlaying();
}

//TEST - VIDEO WINDOW MOVE/RESIZE
int VLCWrapper::MoveResizeWindow(int x, int y, int w, int h)
{
	return m_pImpl->MoveResizeWindow(x, y, w, h);
}

int VLCWrapper::ShowWindow(int nShow)
{
	return m_pImpl->ShowWindow(nShow);
}

int VLCWrapper::IsWindowVisible(void)
{
	return m_pImpl->IsWindowVisible();
}

int VLCWrapper::DestroyWindow(void)
{
	return m_pImpl->DestroyWindow();
}

int VLCWrapper::GetPlayerState(void)
{
	return m_pImpl->GetPlayerState();
}

VLCWrapper *g_VLCWrapper = NULL;
/* APIs for shared memory buffer/control */
int VLCWrapper::_Get_VLC_CONTROL()
{
#if defined(WIN32)

#ifdef	TSPHLD_VLC
	g_hMap1 = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(struct VLC_CONTROL), "VLC_SHARED_CONTROL" );
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
#endif
	{
		g_hMap1 =  OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "VLC_SHARED_CONTROL");
	}
	if ( g_hMap1 == INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	g_VLC_Control = (struct VLC_CONTROL *)MapViewOfFile(g_hMap1, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if ( g_VLC_Control == NULL )
	{
		CloseHandle(g_hMap1);
		return -2;
	}

#else

#ifdef	TSPHLD_VLC	
#endif
	shared_memory::status rc;
	char str[32];
	sprintf(str, "%s.odb", "VLC_SHARED_CONTROL");
	rc = g_hMap1.open(str, "VLC_SHARED_CONTROL", sizeof(struct VLC_CONTROL));
	if ( rc != shared_memory::ok ) 
	{ 
		/*
		shmem.get_error_text(rc, buf, sizeof buf);
		fprintf(stderr, "Failed to open file: %s\n", buf);
		return EXIT_FAILURE;
		*/
		return -1;
	} 
	else 
	{ 
		exclusive_lock x_lock(g_hMap1);
		g_VLC_Control = (struct VLC_CONTROL*)g_hMap1.get_root_object();
		if (g_VLC_Control == NULL)
		{ 
			g_VLC_Control =  (struct VLC_CONTROL*)malloc(sizeof(struct VLC_CONTROL));
			g_hMap1.set_root_object((void*)g_VLC_Control);
		}
	}

#endif

	return 0;
}

int VLCWrapper::_Close_VLC_CONTROL()
{
#if defined(WIN32)
	if ( g_VLC_Control )
	{
		UnmapViewOfFile(g_VLC_Control);
		g_VLC_Control = NULL;
	}

	if ( g_hMap1 != INVALID_HANDLE_VALUE )
	{
		CloseHandle(g_hMap1);
		g_hMap1 = NULL;
	}
#else
	if ( g_VLC_Control )
	{
		free(g_VLC_Control);
		g_VLC_Control = NULL;
	}
	g_hMap1.close();
#endif

	return 0;
}

int VLCWrapper::_Get_VLC_BUFFER(int nBufferSize)
{
#if defined(WIN32)

#ifdef	TSPHLD_VLC
	g_hMap = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, nBufferSize, "VLC_SHARED_BUFFER" );
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
#endif
	{
		g_hMap =  OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "VLC_SHARED_BUFFER");
	}
	if ( g_hMap == INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	g_SharedBuffer = (char*)MapViewOfFile(g_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if ( g_SharedBuffer == NULL )
	{
		CloseHandle(g_hMap);
		return -2;
	}

#else

#ifdef	TSPHLD_VLC	
#endif
	shared_memory::status rc;
	char str[32];
	sprintf(str, "%s.odb", "VLC_SHARED_BUFFER");
	rc = g_hMap.open(str, "VLC_SHARED_BUFFER", nBufferSize);
	if ( rc != shared_memory::ok ) 
	{ 
		/*
		shmem.get_error_text(rc, buf, sizeof buf);
		fprintf(stderr, "Failed to open file: %s\n", buf);
		return EXIT_FAILURE;
		*/
		return -1;
	} 
	else 
	{ 
		exclusive_lock x_lock(g_hMap);
		g_SharedBuffer = (char*)g_hMap.get_root_object();
		if (g_SharedBuffer == NULL)
		{ 
			g_SharedBuffer =  (char*)malloc(nBufferSize);
			g_hMap.set_root_object((void*)g_SharedBuffer);
		}
	}

#endif

	return 0;
}

int VLCWrapper::_Close_VLC_BUFFER()
{
#if defined(WIN32)

	if ( g_SharedBuffer )
	{
		UnmapViewOfFile(g_SharedBuffer);
		g_SharedBuffer = NULL;
	}

	if ( g_hMap != INVALID_HANDLE_VALUE )
	{
		CloseHandle(g_hMap);
		g_hMap = NULL;
	}
#else
	if ( g_SharedBuffer )
	{
		free(g_SharedBuffer);
		g_SharedBuffer = NULL;
	}	
	g_hMap.close();
#endif

	return 0;
}


#ifdef WIN32
void HLDPumpingThread(PVOID param)
#else
void* HLDPumpingThread(PVOID param)
#endif
{
	CHld *pLLD = (CHld *)param;
	if ( !pLLD )
	{
#ifdef WIN32
	return;
#else
	return 0;
#endif
	}

	g_VLCWrapper->g_VLC_PumpingThreadDone = 0;
	while (g_VLCWrapper->g_VLC_PumpingThreadDone == 0)
	{
		if (pLLD->_SysSta->IsTaskState_StopCond_VlcPumping())
		{
			g_VLCWrapper->g_VLC_PumpingThreadDone = 1;
			break;
		}

		Sleep(10);
		if ( g_VLCWrapper->g_PumpingWait == 1 && pLLD->_SysSta->Vlc_SendIpStreaming() )
		{
			continue;
		}
#if defined(WIN32)
		__try
		{
#endif
			if ( pLLD->_SysSta->Vlc_SendIpStreaming() )
			{
				g_VLCWrapper->VLC_Send_Stream(pLLD);
			}
			else
			{
				g_VLCWrapper->VLC_Recv_Stream(pLLD);
			}
#if defined(WIN32)
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
#endif

		g_VLCWrapper->g_PumpingWait = 1;
	}

	//OutputDebugString("HLDPumpingThread :: exit \n");
	
#ifdef WIN32
	return;
#else
	return 0;
#endif	
}

#ifdef WIN32
void HLDThreadEngineForVLC(PVOID param)
#else
void* HLDThreadEngineForVLC(PVOID param)
#endif
{
	CHld *pLLD = (CHld *)param;
	if ( !pLLD )
	{
#ifdef WIN32
	return;
#else
	return 0;
#endif
	}

	pLLD->_HLog->HldPrint("Hld-Vlc. Launch Vlc Task");

	g_VLCWrapper->g_VLC_ThreadDone = 0;

	/* Create a dummy file for IP/RECV/REC */
	char szDummyFilepath[MAX_PATH];
	//sprintf(szDummyFilepath, "%s\\%s_%d", pLLD->g_szCurDir, VLC_DUMMY_PATH);
	sprintf(szDummyFilepath, "%s\\%s", pLLD->_FIf->g_szCurDir, VLC_DUMMY_PATH);
	FILE *hFile = fopen(szDummyFilepath, "r");
	if ( hFile )
	{
		fclose(hFile);
	}
	else
	{
		hFile = fopen(szDummyFilepath, "w+");
		if ( hFile )
		{
			fputs("Don't remove!!!", hFile);
			fclose(hFile);
		}
	}

	/* Create event/mutex */
#if defined(WIN32)
	g_VLCWrapper->g_hEvent = CreateEvent(NULL, TRUE, FALSE, "VLC_SHARED_EVENT");
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		g_VLCWrapper->g_hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "VLC_SHARED_EVENT");
	}

	g_VLCWrapper->g_hMutex = CreateMutex(NULL, FALSE, "VLC_SHARED_MUTEX");
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		g_VLCWrapper->g_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "VLC_SHARED_MUTEX");
	}
#else
	g_VLCWrapper->g_hEvent.open("VLC_SHARED_EVENT");
	semp_init(&g_VLCWrapper->g_hMutex, (char*)"VLC_SHARED_MUTEX", 0);
#endif

	/* Prepare parameter */
	int nCount = 0;
	struct VLC_CONTROL stVLC;
	stVLC.nOption = 0;
	stVLC.nVLC_Oper = VLC_RUN_NO_READY;
	stVLC.nSize = G_BUFFER_SIZE;
	stVLC.nCount = 0;
	stVLC.nRead = 0;
	stVLC.nWrite = 0;
	stVLC.nBufferedCount = 0x100000;
		
	/* Create the shared parameter */
	g_VLCWrapper->_Get_VLC_CONTROL();
	if ( !g_VLCWrapper->g_VLC_Control )
		goto _ERROR_VLC_MAIN_;

	memcpy(g_VLCWrapper->g_VLC_Control, &stVLC, sizeof(struct VLC_CONTROL));

	/* Create the shared buffer */
	g_VLCWrapper->_Get_VLC_BUFFER(g_VLCWrapper->g_VLC_Control->nSize);	
	
	while ( !pLLD->_SysSta->ReqedNewAction_User() )
	{
		Sleep(10);
	}
	//OutputDebugString("HLDThreadEngineForVLC :: while :: exit \n");

_ERROR_VLC_MAIN_:

	/* Close event/mutex */
#if defined(WIN32)
	if ( g_VLCWrapper->g_hEvent != INVALID_HANDLE_VALUE )
	{
		SetEvent(g_VLCWrapper->g_hEvent);
		CloseHandle(g_VLCWrapper->g_hEvent);
	}
	
	if ( g_VLCWrapper->g_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(g_VLCWrapper->g_hMutex);
		CloseHandle(g_VLCWrapper->g_hMutex);
	}
#else
	if ( g_VLCWrapper->g_hEvent.IsOk )
	{
		g_VLCWrapper->g_hEvent.signal();
		g_VLCWrapper->g_hEvent.close();
	}
	
	if ( g_VLCWrapper->g_hMutex >= 0 )
	{
		semp_post(&g_VLCWrapper->g_hMutex);
		semp_destroy(&g_VLCWrapper->g_hMutex);
	}
#endif

	/* Close the shared bufer/paramter */
	g_VLCWrapper->_Close_VLC_BUFFER();
	g_VLCWrapper->_Close_VLC_CONTROL();

	g_VLCWrapper->g_VLC_ThreadDone = 1;
	g_VLCWrapper->g_VLC_PumpingThreadDone = 1;
	g_VLCWrapper->g_VLC_Bufferring = 0;

	pLLD->_HLog->HldPrint("Hld-Vlc. Term Vlc Task");

	//OutputDebugString("HLDThreadEngineForVLC :: exit \n");
#ifdef WIN32
	return;
#else
	return 0;
#endif
}

int VLCWrapper::VLC_Run(CHld *pLLD)
{
	int nRet = 0;

	if ( !pLLD )
	{
		return -1;
	}

	if ( !pLLD->_SysSta->Vlc_HasBeenActivated() )
	{
		return -1;
	}

	//not ready to start VLC thread
	while ( g_VLC_ThreadDone != 1 )
	{
		Sleep(100);
	}

#if defined(WIN32)	
	SetCurrentDirectory(g_szCurDir);
	if ( _beginthread(HLDThreadEngineForVLC, 0, (PVOID)pLLD) == -1 ) 
	{
		pLLD->_HLog->HldPrint("Hld-Vlc. FAIL to create thread(VLC)");
	}
#else
	chdir(g_szCurDir);
	if ( pthread_create(&g_thread, NULL, HLDThreadEngineForVLC, (PVOID)pLLD) != 0 )
	{
		pLLD->_HLog->HldPrint("Hld-Vlc. FAIL to create thread(VLC)");
	}
#endif

	//not ready to access VLC play list
	while ( VLC_Ready() != 0 )
	{
		Sleep(100);
	}

	if ( pLLD->_SysSta->Vlc_SendIpStreaming() )
	{
		nRet = VLC_Start(pLLD->_SysSta->_VlcSta(), (char*)VLC_DUMMY_PATH, pLLD->g_VLC_Str, (char*)"");
		memcpy(VLC_Current_Target, pLLD->_FIf->PlayParm.AP_lst.szfn, MAX_PATH);
	}
	else if ( pLLD->_SysSta->Vlc_RecvIpStreaming() )
	{
		nRet = VLC_Start(pLLD->_SysSta->_VlcSta(), (char*)"", pLLD->g_VLC_Str, (char*)"");
	}
	else if ( pLLD->_SysSta->Vlc_RecvIpStreamingRec() )
	{
		pLLD->_FIf->TL_i64TotalFileSize	= 0;
		nRet = VLC_Start(pLLD->_SysSta->_VlcSta(), (char*)"", pLLD->g_VLC_Str, pLLD->_FIf->AP_szfnRecordFile);
	}

#if defined(WIN32)
	if ( _beginthread(HLDPumpingThread, 0, (PVOID)pLLD) == -1 ) 
	{
		pLLD->_HLog->HldPrint("Hld-Vlc. FAIL to create pumping thread(VLC)");
	}
#else
	if ( pthread_create(&g_thread1, NULL, HLDPumpingThread, (PVOID)pLLD) != 0 )
	{
		pLLD->_HLog->HldPrint("Hld-Vlc. FAIL to create pumping thread(VLC)");
	}
#endif
	Sleep(100);

	return nRet;
}

int VLCWrapper::VLC_Restart(CHld *pLLD)
{
	if ( !pLLD )
	{
		return false;
	}

	pLLD->_HLog->HldPrint("Hld-Vlc. Restart");

	if ( !pLLD->_SysSta->Vlc_HasBeenActivated() )
	{
		return false;
	}

	char szVLC_Str[MAX_PATH], tmp[MAX_PATH];
	memcpy(szVLC_Str, pLLD->g_VLC_Str, MAX_PATH);

	if ( pLLD->g_IP_ProgramChanged != -1 )
	{
		sprintf(tmp, "%s program=%d", szVLC_Str, pLLD->g_IP_ProgramChanged);
		memcpy(pLLD->g_VLC_Str, tmp, MAX_PATH);
	}

	VLC_Stop();
	VLC_Run(pLLD);

	memcpy(pLLD->g_VLC_Str, szVLC_Str, MAX_PATH);

	return true;
}

int VLCWrapper::VLC_Stop(void)
{
	if ( !g_VLC_Control || !g_SharedBuffer )
		return -1;

	g_VLC_ThreadDone = 1;
#if defined(WIN32)
	if ( g_VLC_File != INVALID_HANDLE_VALUE )
	{
		CloseHandle(g_VLC_File);
		g_VLC_File = INVALID_HANDLE_VALUE;
	}
#else
	if ( g_VLC_File != NULL )
	{
		fclose((FILE*)g_VLC_File);
		g_VLC_File = NULL;
	}
#endif
	Stop();

	return 0;
}

int VLCWrapper::VLC_Ready(void)
{
	if ( !g_VLC_Control || !g_SharedBuffer )
		return -1;

	return 0;
}

int VLCWrapper::VLC_Start(int nVLC_Oper, char* szURL, char* szMRL, char* szRecordFilePath)
{
	int nRet = 0;

	if ( !g_VLC_Control || !g_SharedBuffer )
		return -1;

	g_VLC_Bufferring = 1;

	memset(g_SharedBuffer, 0, g_VLC_Control->nSize);
	g_VLC_Control->nRead =	g_VLC_Control->nWrite =	g_VLC_Control->nCount = 0;
	g_VLC_Control->nVLC_Oper = nVLC_Oper;
	g_VLC_Control->nBufferedCount = 0x100000;
	sprintf(g_VLC_Control->szURL, "%s", szURL);
	sprintf(g_VLC_Control->szMRL, "%s", szMRL);

	if ( nVLC_Oper == VLC_SEND_IP_STREAM )
	{
		OpenMedia(szURL);
		
		char seps[] = " \t\r\n", *token;
		token = strtok( szMRL, seps );
		while( token != NULL )
		{
			AddOption(token);
			token = strtok( NULL, seps );
		}

		Play();
	}	
	else if ( nVLC_Oper == VLC_RECV_IP_STREAM || nVLC_Oper == VLC_RECV_IP_STREAM_REC )
	{
		OpenMedia(szMRL);
		Play();

		if ( nVLC_Oper == VLC_RECV_IP_STREAM_REC && szRecordFilePath )
		{
#if defined(WIN32)
			if ( g_VLC_File != INVALID_HANDLE_VALUE )
			{
				CloseHandle(g_VLC_File);
				g_VLC_File = INVALID_HANDLE_VALUE;
			}

			g_VLC_File = CreateFile(szRecordFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
#else
			if ( g_VLC_File != NULL )
			{
				fclose((FILE*)g_VLC_File);
				g_VLC_File = NULL;
			}

			g_VLC_File = fopen(szRecordFilePath, "wb");
#endif
		}
	}

	return nRet;
}

int VLCWrapper::VLC_Recv_Stream(CHld* pLLD)
{
	int dwRet, dwSizeWritten;
	int	nBankBlockSize = (pLLD->_FIf->SdramSubBankSize());

	if ( !pLLD || !g_VLC_Control || !g_SharedBuffer ) 
	{
		return -1;
	}

	if ( !pLLD->_SysSta->Vlc_Recv_or_RecvRec() )
	{
		return -1;
	}

	//Receive IP stream
#if defined(WIN32)
	if ( g_hEvent != INVALID_HANDLE_VALUE )
	{
		if ( WaitForSingleObject(g_hEvent, WAIT_EVENT_SIGNAL) == WAIT_TIMEOUT )
		{
			return 0;
		}
	}
#else
	if ( g_hEvent.IsOk )
	{
		if ( g_hEvent.wait(WAIT_EVENT_SIGNAL) != 1 )
		{
			return 0;
		}
	}
#endif
	

	pLLD->_FIf->TL_dwBytesRead = 0;
	
	if ( g_VLC_Control->nCount >= nBankBlockSize )
	{
#if defined(WIN32)
		if ( g_hMutex != INVALID_HANDLE_VALUE )
		{
			WaitForSingleObject(g_hMutex, INFINITE);
		}
#else
		if ( g_hMutex >= 0 )
		{
			semp_wait(&g_hMutex);
		}
#endif

		sprintf(debug_string, "R:%d, W:%d, C:%d, B:%d\n", g_VLC_Control->nRead, g_VLC_Control->nWrite,g_VLC_Control->nCount, g_VLC_Control->nBufferedCount);
//		sprintf(pLLD->debug_string, "R:%d, W:%d, C:%d, B:%d\n", g_VLC_Control->nRead, g_VLC_Control->nWrite,g_VLC_Control->nCount, g_VLC_Control->nBufferedCount);
		//OutputDebugString(pLLD->debug_string);

		if ( g_VLC_Control->nSize - g_VLC_Control->nRead >= nBankBlockSize )
		{
			//Record
#if defined(WIN32)
			if ( g_VLC_File != INVALID_HANDLE_VALUE )
			{
				WriteFile(g_VLC_File, g_SharedBuffer + g_VLC_Control->nRead, (DWORD)nBankBlockSize, (DWORD*)&dwSizeWritten, NULL);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#else
			if ( g_VLC_File != NULL )
			{
				dwSizeWritten = fwrite(g_SharedBuffer + g_VLC_Control->nRead, 1, (DWORD)nBankBlockSize, (FILE *)g_VLC_File);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#endif
			
			memcpy((char *)pLLD->_FIf->TL_szBufferPlay + pLLD->_FIf->TL_nWIndex, g_SharedBuffer + g_VLC_Control->nRead, nBankBlockSize);

			g_VLC_Control->nRead += nBankBlockSize;
			if ( g_VLC_Control->nRead >= g_VLC_Control->nSize )
				g_VLC_Control->nRead -= g_VLC_Control->nSize;
		}
		else
		{
			dwRet = g_VLC_Control->nSize - g_VLC_Control->nRead;

			//Record
#if defined(WIN32)
			if ( g_VLC_File != INVALID_HANDLE_VALUE )
			{
				WriteFile(g_VLC_File, g_SharedBuffer + g_VLC_Control->nRead, (DWORD)dwRet, (DWORD*)&dwSizeWritten, NULL);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#else
			if ( g_VLC_File != NULL )
			{
				dwSizeWritten = fwrite(g_SharedBuffer + g_VLC_Control->nRead, 1, (DWORD)nBankBlockSize, (FILE *)g_VLC_File);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#endif
			memcpy((char *)pLLD->_FIf->TL_szBufferPlay + pLLD->_FIf->TL_nWIndex, g_SharedBuffer + g_VLC_Control->nRead, dwRet);

			//Record
#if defined(WIN32)
			if ( g_VLC_File != INVALID_HANDLE_VALUE )
			{
				WriteFile(g_VLC_File, g_SharedBuffer, (DWORD)(nBankBlockSize - dwRet), (DWORD*)&dwSizeWritten, NULL);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#else
			if ( g_VLC_File != NULL )
			{
				dwSizeWritten = fwrite(g_SharedBuffer, 1, (DWORD)(nBankBlockSize - dwRet), (FILE *)g_VLC_File);
				pLLD->_FIf->TL_i64TotalFileSize += dwSizeWritten;
			}
#endif
			memcpy((char *)pLLD->_FIf->TL_szBufferPlay + pLLD->_FIf->TL_nWIndex + dwRet, g_SharedBuffer, nBankBlockSize - dwRet);

			g_VLC_Control->nRead = (nBankBlockSize - dwRet);
		}
		g_VLC_Control->nCount -= nBankBlockSize;
		if ( g_VLC_Control->nCount < 0 )
			g_VLC_Control->nCount = 0;

#if defined(WIN32)
		if ( g_hMutex != INVALID_HANDLE_VALUE )
		{
			ReleaseMutex(g_hMutex);
		}
#else
		if ( g_hMutex >= 0 )
		{
			semp_post(&g_hMutex);
		}
#endif

		if ( pLLD->_FIf->TL_gFirstRead )
		{
			pLLD->_FIf->TL_gFirstRead = 0;
		}
		pLLD->_FIf->TL_dwBytesRead = nBankBlockSize;
	}
	
	//get input bitrate
	pLLD->g_IP_InputBitrate = (int)(GetInputBitrate() * 8 * 1000000);

	//get program information
	int pi[2+64], i, j=0;
	int nProgramCount = 0;
	GetProgramInfo(pi);
	nProgramCount = pi[0];
	if ( nProgramCount > 0 )
	{
		for ( i = 0; i < nProgramCount; i++ )
		{
			if ( pi[i+2] == 0 ) continue;

			pLLD->g_IP_ProgramId[i] =  pi[i+2];
			//sprintf(pLLD->debug_string, "t:%d, c:%d, cc:%d\n", pLLD->g_IP_ProgramCount, pLLD->g_IP_ProgramId[i], g_VLC_Control->nCurrentProgram);
			//OutputDebugString(pLLD->debug_string);
			++j;
		}
		
		if ( j == nProgramCount )
		{
			pLLD->g_IP_ProgramCount = j;
			pLLD->g_IP_CurrentProgram = pi[1];
		}
	}

	return 0;
}

int VLCWrapper::VLC_Send_Stream(CHld* pLLD)
{
	int	nBankBlockSize = (pLLD->_FIf->SdramSubBankSize());

	if ( !pLLD || !g_VLC_Control || !g_SharedBuffer ) 
	{
		return 0;
	}

	//Send IP stream
	if ( !pLLD->_SysSta->Vlc_SendIpStreaming() )
		return 0;

	//sprintf(pLLD->debug_string, "R:%d, W:%d, C:%d, B:%d\n", g_VLC_Control->nRead, g_VLC_Control->nWrite,g_VLC_Control->nCount, g_VLC_Control->nBufferedCount);
	//OutputDebugString(pLLD->debug_string);
	
	//vlc1.0.3
	/*
	//sskim20071115 - delay VLC start time...
	if ( g_hEvent != INVALID_HANDLE_VALUE && pLLD->TL_gTotalSendData < (nBankBlockSize*(pLLD->iHW_BANK_NUMBER+1))*2 )
	{
		ResetEvent(g_hEvent);
	}
	else
	*/
#if defined(WIN32)
	if ( g_hEvent != INVALID_HANDLE_VALUE )
	{
		if ( g_VLC_Control->nCount >= g_VLC_Control->nBufferedCount )
		{
			if ( g_VLC_Bufferring == 1 && g_VLC_Control->nCount < SUB_BANK_MAX_BYTE_SIZE*MAX_BANK_NUMBER )
			{
				ResetEvent(g_hEvent);
			}
			else
			{
				g_VLC_Bufferring = 0;
				SetEvent(g_hEvent);
			}
		}
		else
		{
			g_VLC_Bufferring = 1;
			ResetEvent(g_hEvent);
		}
	}

	if ( g_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(g_hMutex, INFINITE);
	}
#else
	if ( g_hEvent.IsOk )
	{
		if ( g_VLC_Control->nCount >= g_VLC_Control->nBufferedCount )
		{
			if ( g_VLC_Bufferring == 1 && g_VLC_Control->nCount < SUB_BANK_MAX_BYTE_SIZE*MAX_BANK_NUMBER )
			{
				g_hEvent.reset();
			}
			else
			{
				g_VLC_Bufferring = 0;
				g_hEvent.signal();
			}
		}
		else
		{
			g_VLC_Bufferring = 1;
			g_hEvent.reset();
		}
	}

	if ( g_hMutex >= 0 )
	{
		//semp_wait(&g_hMutex);
		semp_trywait(&g_hMutex);
	}
#endif

#if defined(WIN32)
	memcpy(g_SharedBuffer + g_VLC_Control->nWrite, (char *)pLLD->_FIf->TL_pdwDMABuffer, nBankBlockSize);
#else
	memcpy(g_SharedBuffer + g_VLC_Control->nWrite, (char *)pLLD->_FIf->TL_pbBuffer, nBankBlockSize);
#endif

	g_VLC_Control->nWrite += nBankBlockSize;
	if ( g_VLC_Control->nWrite >= g_VLC_Control->nSize )
		g_VLC_Control->nWrite = 0;
	g_VLC_Control->nCount += nBankBlockSize;
	if ( g_VLC_Control->nWrite == 0 )
	{
		g_VLC_Control->nCount = g_VLC_Control->nSize - g_VLC_Control->nRead;
	}

#if defined(WIN32)
	if ( g_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(g_hMutex);
	}
#else
	if ( g_hMutex >= 0 )
	{
		semp_post(&g_hMutex);
	}
#endif

	//get input bitrate
	pLLD->g_IP_InputBitrate = (int)(GetInputBitrate() * 8 * 1000000);

	//get program information
	int pi[2+64], i, j=0;
	int nProgramCount = 0;
	GetProgramInfo(pi);
	nProgramCount = pi[0];
	if ( nProgramCount > 0 )
	{
		for ( i = 0; i < nProgramCount; i++ )
		{
			if ( pi[i+2] == 0 ) continue;

			pLLD->g_IP_ProgramId[i] =  pi[i+2];
			//sprintf(pLLD->debug_string, "t:%d, c:%d, cc:%d\n", pLLD->g_IP_ProgramCount, pLLD->g_IP_ProgramId[i], g_VLC_Control->nCurrentProgram);
			//OutputDebugString(pLLD->debug_string);
			++j;
		}
		
		if ( j == nProgramCount )
		{
			pLLD->g_IP_ProgramCount = j;
			pLLD->g_IP_CurrentProgram = pi[1];
		}
	}

	return 0;
}

int VLCWrapper::VLC_Set_Program(CHld* pLLD)
{
	if ( !pLLD || !g_VLC_Control || !g_SharedBuffer ) 
	{
		return 0;
	}
	pLLD->_HLog->HldPrint("Hld-Vlc. Set Program");

	SetCurrentProgram(pLLD->g_IP_ProgramChanged);
	return 0;
}

int VLCWrapper::VLC_Read_Buffered_Count(CHld* pLLD)
{
	if ( !pLLD || !g_VLC_Control || !g_SharedBuffer ) 
	{
		return -1;
	}

	//Send IP stream
	if ( !pLLD->_SysSta->Vlc_SendIpStreaming() )
		return -1;

	int nCount = -1;

#if defined(WIN32)
	if ( g_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(g_hMutex, INFINITE);
	}
#else
	if ( g_hMutex >= 0 )
	{
		//semp_wait(&g_hMutex);
		semp_trywait(&g_hMutex);
	}
#endif

	nCount = g_VLC_Control->nCount;

#if defined(WIN32)
	if ( g_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(g_hMutex);
	}
#else
	if ( g_hMutex >= 0 )
	{
		semp_post(&g_hMutex);
	}
#endif

	//sprintf(pLLD->debug_string, "R:%d, W:%d, C:%d, B:%d\n", g_VLC_Control->nRead, g_VLC_Control->nWrite,g_VLC_Control->nCount, g_VLC_Control->nBufferedCount);
	//OutputDebugString(pLLD->debug_string);
	return nCount;
}

