/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapper.h
    Desc.:  An simple C++-interface to libvlc.

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
#ifndef __VLCWRAPPER_H__
#define __VLCWRAPPER_H__

//#ifdef	TSPHLD_VLC
#if defined(WIN32)
#include	<windows.h>
#include	<stdio.h>
#include	<process.h>
#else
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/mman.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<semaphore.h>
#include	<fcntl.h>
#include	<pthread.h>
#include	"shmem.h"
#endif

#include	"LLDWrapper.h"
#include	"../include/hld_const.h"

#define WAIT_PUMPING_SIGNAL	10000
#define WAIT_EVENT_SIGNAL	5000
#define VLC_DUMMY_PATH		"dummy"
#define G_BUFFER_SIZE		(32*0x100000*2)	//Mbytes

struct VLC_CONTROL
{
	int nVLC_Oper;
	
	char szURL[1024];
	char szMRL[1024];
	
	//option
	int nOption;//max = 15	
	char szOption[15][64];
	
	//buffer
	int nSize;
	int nRead;
	int nWrite;
	int nCount;
	int nBufferedCount;

	//state
	float fInputBitrate;
	float fAverageInputBitrate;
	float fDemuxBitrate;
	float fAverageDemuxBitrate;

	//multi-program count/id
	int nPrograms;	//Max 64
	int nProgramId[2+64];//65th = to be selected program
	int nCurrentProgram;	
};
//#endif
#ifdef WIN32
typedef __int64                         int64_t;                      ///< For old MS Compilers.
#endif
typedef struct libvlc_event_t           VLCEvent;                     ///< A vlc event.
typedef void (*VLCExceptionHandler)     (const char*);                ///< Exception handler callback.
typedef void (*VLCEventHandler)         (const VLCEvent *, void *);   ///< Event handler callback.

class VLCWrapperImpl;

class VLCWrapper
{
private:
	char debug_string[100];

    VLCWrapperImpl* m_pImpl; ///< VLCWrapper's private Implementation

public:
	VLCWrapper(void);  ///< Ctor.
	~VLCWrapper(void); ///< Dtor.    

    /** Set window for media output.
    *   @param [in] pHwnd window, on Windows a HWND handle. */
    void SetOutputWindow(void* pHwnd);

    /** Register an exception handler for libvlc-exceptions.
    *   @param [in] eh The exception handler. */
    void SetExceptionHandler(VLCExceptionHandler eh);

    /** Register an event handler for libvlc-events.
    *   @param [in] evt The event handler. */
    void SetEventHandler(VLCEventHandler evt);

    /** Open a media file.
    *   @param [in] pszMediaPathName PathName of the media file. */
    void OpenMedia(const char* pszMediaPathName);

    /** Start playback. */
    void Play();

    /** Pause playback. */
    void Pause();
    
    /** Stop playback. */
    void Stop();

    /** Get length of media in milliseconds. Call this in the event handler,
    *   otherwise the result is not reliable!!!
    *   @return The media length in milliseconds. */
    int64_t GetLength();

    /** Get actual position of media in milliseconds. Call this in the event handler,
    *   otherwise the result is not reliable!!!
    *   @return The media position in milliseconds. */
    int64_t GetTime();

    /** Set new position of media in milliseconds.
    *   @param [in] llNewTime The new media position in milliseconds. */
    void SetTime(int64_t llNewTime);

    /** Mutes the audio output of playback.
    *   @param [in] bMute True or false. */
    void Mute(bool bMute=true);

    /** Get mute state of playback.
    *   @return True or false. */
    bool GetMute();

    /** Returns the actual audio volume.
    *   @return The actual audio volume. */
    int  GetVolume();

    /** Set the actual audio volume.
    *   @param [in] iVolume New volume level. */
    void SetVolume(int iVolume);    

	////////////////////////////////////////////////////////
	int m_nProgramInfo[2+64];
	int m_nCurrentProgram;
	//TEST - VIDEO WINDOW MOVE/RESIZE
	int m_nVideo_X;
	int m_nVideo_Y;
	int m_nVideo_W;
	int m_nVideo_H;

	int AddOption(char* pszOption);
	float GetInputBitrate();
	int GetProgramInfo(int*);
	int SetCurrentProgram(int);
	int isPlaying();
	//TEST - VIDEO WINDOW MOVE/RESIZE
	int MoveResizeWindow(int x, int y, int w, int h);
	int ShowWindow(int nShow);
	int IsWindowVisible(void);
	int DestroyWindow(void);
	int GetPlayerState(void);

	/* APIs for shared memory buffer/control */
	/* Shared Memory to comm. with VLC */
	struct VLC_CONTROL *g_VLC_Control;
	char *g_SharedBuffer;	
#if defined WIN32
	HANDLE g_hMap;
	HANDLE g_hMap1;
	HANDLE g_hMutex;
	HANDLE g_hEvent;
#else
	shared_memory g_hMap;
	shared_memory g_hMap1;
	semp_t	g_hMutex;
	event	g_hEvent;
#endif

	int	g_VLC_PumpingThreadDone;//0=Ready, 1=Done
	int	g_PumpingWait;//0=Pumping Start, 1=Pumping Done
	int	g_VLC_ThreadDone;//0=Ready, 1=Done
	int	g_VLC_Bufferring;
	HANDLE	g_VLC_File;
	char g_szCurDir[MAX_PATH];
	char VLC_Current_Target[MAX_PATH];
	
	int _Get_VLC_CONTROL();
	int _Close_VLC_CONTROL();
	int _Get_VLC_BUFFER(int nBufferSize);
	int _Close_VLC_BUFFER();

	int VLC_Send_Stream(CHld* pLLD);
	int VLC_Recv_Stream(CHld* pLLD);
	int VLC_Run(CHld* pLLD);
	int VLC_Restart(CHld* pLLD);
	int VLC_Stop(void);
	int VLC_Ready(void);
	int VLC_Start(int nVLC_Oper, char* szURL, char* szMRL, char* szRecordFilePath);
	int VLC_Set_Program(CHld* pLLD);
	int VLC_Read_Buffered_Count(CHld* pLLD);
};

#endif // __VLCWRAPPER_H__
