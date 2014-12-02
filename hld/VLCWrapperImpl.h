/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapperImpl.h
    Desc.:  Private Implementation of VLCWrapper.

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
#ifndef __VLCWRAPPERIMPL_H__
#define __VLCWRAPPERIMPL_H__

#include "VLCWrapper.h"
#include <vlc/vlc.h>

class VLCWrapperImpl
{
    libvlc_instance_t*       m_pVLCInstance;        ///< The VLC instance.
	libvlc_media_player_t*   m_pMediaPlayer;        ///< The VLC media player object.
	libvlc_media_t*          m_pMedia;              ///< The media played by the media player.
    libvlc_event_manager_t*  m_pEvtManager;         ///< The event manger for the loaded media file.
    libvlc_exception_t       m_VLCex;               ///< A VLC exection object.
    VLCExceptionHandler      m_EH;                  ///< A handler for VLC exceptions, basically a callback which takes the ex message...
    VLCEventHandler          m_EvtH;                ///< An event handler for the media player.    
	
public:
	VLCWrapperImpl(void);
	~VLCWrapperImpl(void);
    void SetOutputWindow(void* pHwnd);
    void SetExceptionHandler(VLCExceptionHandler eh);
    void SetEventHandler(VLCEventHandler evt);    
    void Play();
    void Pause();
    void Stop();
    int64_t GetLength();
    int64_t GetTime();
    void SetTime(int64_t llNewTime);
    void Mute(bool bMute=true);
    bool GetMute();
    int  GetVolume();
    void SetVolume(int iVolume);
    void OpenMedia(const char* pszMediaPathName);

	////////////////////////////////////////////////////////
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
};

#endif // __VLCWRAPPERIMPL_H__
