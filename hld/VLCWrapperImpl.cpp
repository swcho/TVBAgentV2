/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapperImpl.cpp
    Desc.:  VLCWrapperImpl Implementation.

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
#include "VLCWrapperImpl.h"
#include <stdio.h>

static void ProcessVLCException(libvlc_exception_t* ex, VLCExceptionHandler eh)
{
    if(libvlc_exception_raised(ex) && eh)
    {
		char szErrorMsg[1024]="";
        sprintf(szErrorMsg, "%s", libvlc_exception_get_message(ex));
        libvlc_exception_clear(ex);
		eh(szErrorMsg);
    }
}

VLCWrapperImpl::VLCWrapperImpl(void)
:	m_pVLCInstance(0),
	m_pMediaPlayer(0),
	m_pMedia(0),
    m_pEvtManager(0),
    m_EH(0),
    m_EvtH(0)
{
	const char * const vlc_args[] = {
		"-I", "dummy",     // No special interface
		"--ignore-config", // Don't use VLC's config
		"--plugins-cache",
		"--no-reset-plugins-cache",
		"--plugin-path=./plugins",
#if defined(WIN32)
		"--one-instance", 
#endif
		"--ts-es-id-pid",
		"--video-on-top",
		"--vout-event=3",
		"--no-video-title-show",
		"--embedded-video",
		"--video-x=0",
		"--video-y=0",
		"--width=1", 
		"--height=1",
		"--scale=1.0",
		"--no-video-deco",
		"--video-title=VLC/VIDEO",
		"--loop",
		};

    // init the exception object.
	libvlc_exception_init (&m_VLCex);

	// init vlc modules, should be done only once
	m_pVLCInstance = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);

    // Create a media player playing environement
    m_pMediaPlayer = libvlc_media_player_new(m_pVLCInstance, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);

    // Create an event manager for the player for handling e.g. time change events
    m_pEvtManager=libvlc_media_player_event_manager(m_pMediaPlayer, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

VLCWrapperImpl::~VLCWrapperImpl(void)
{
    // Free the media_player
    libvlc_media_player_release (m_pMediaPlayer);
	libvlc_release (m_pVLCInstance);
}

void VLCWrapperImpl::SetOutputWindow(void* pHwnd)
{
    // Set the output window    
	libvlc_media_player_set_hwnd(m_pMediaPlayer, pHwnd, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::SetExceptionHandler(VLCExceptionHandler eh)
{
    m_EH=eh;
}

void VLCWrapperImpl::SetEventHandler(VLCEventHandler evt)
{
    m_EvtH=evt;
    libvlc_event_attach(m_pEvtManager,        
                        libvlc_MediaPlayerTimeChanged,
                        m_EvtH,
                        NULL,//void *user_data,
                        &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::Play()
{
	// play the media_player
    libvlc_media_player_play (m_pMediaPlayer, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::Pause()
{
	// Pause playing
    libvlc_media_player_pause (m_pMediaPlayer, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::Stop()
{
	// Stop playing
    libvlc_media_player_stop (m_pMediaPlayer, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);

	//sskim20100115
	if ( m_pMedia )
	{
		libvlc_media_release (m_pMedia);
		m_pMedia = 0;
	}
}

int64_t VLCWrapperImpl::GetLength()
{
    int64_t iLength=libvlc_media_player_get_length(m_pMediaPlayer, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
    return iLength;
}

int64_t VLCWrapperImpl::GetTime()
{
    int64_t iTime=libvlc_media_player_get_time(m_pMediaPlayer, &m_VLCex);    
    ProcessVLCException(&m_VLCex, m_EH);
    return iTime;
}

void VLCWrapperImpl::SetTime(int64_t llNewTime)
{
    libvlc_media_player_set_time(m_pMediaPlayer,(libvlc_time_t)llNewTime, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::Mute(bool bMute)
{
    libvlc_audio_set_mute(m_pVLCInstance, bMute, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

bool VLCWrapperImpl::GetMute()
{
    bool bMuteState=!!libvlc_audio_get_mute(m_pVLCInstance, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
    return bMuteState;
}

int VLCWrapperImpl::GetVolume()
{
    int iVolume=libvlc_audio_get_volume(m_pVLCInstance, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
    return iVolume;
}

void VLCWrapperImpl::SetVolume(int iVolume)
{
    libvlc_audio_set_volume(m_pVLCInstance, iVolume, &m_VLCex);
    ProcessVLCException(&m_VLCex, m_EH);
}

void VLCWrapperImpl::OpenMedia(const char* pszMediaPathName)
{
	// Load a new item
	m_pMedia = libvlc_media_new (m_pVLCInstance, pszMediaPathName, &m_VLCex);
    libvlc_media_player_set_media (m_pMediaPlayer, m_pMedia, &m_VLCex);
	ProcessVLCException(&m_VLCex, m_EH);
}

////////////////////////////////////////////////////////
int VLCWrapperImpl::AddOption(char* pszOption)
{
	libvlc_media_add_option(m_pMedia, (const char*)pszOption, &m_VLCex);
	ProcessVLCException(&m_VLCex, m_EH);
	return 0;
}

float VLCWrapperImpl::GetInputBitrate()
{
    float fInputBitrate=libvlc_media_player_get_input_bitrate(m_pMediaPlayer, &m_VLCex);    
    ProcessVLCException(&m_VLCex, m_EH);
    return fInputBitrate;
}

int VLCWrapperImpl::GetProgramInfo(int *pi)
{
	int nRet = libvlc_media_player_get_program_info(m_pMediaPlayer, &m_VLCex, pi);
	ProcessVLCException(&m_VLCex, m_EH);
	return nRet;
}

int VLCWrapperImpl::SetCurrentProgram(int current_pgm)
{
	int nRet = libvlc_media_player_set_current_program(m_pMediaPlayer, &m_VLCex, current_pgm);
	ProcessVLCException(&m_VLCex, m_EH);
	return nRet;
}

int VLCWrapperImpl::isPlaying()
{
	int nRet = 0;
	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_is_playing(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);
	}
	return nRet;
}

//TEST - VIDEO WINDOW MOVE/RESIZE
int VLCWrapperImpl::MoveResizeWindow(int x, int y, int w, int h)
{
	int nRet = 0;

	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_has_vout(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);
		if ( nRet )
		{
			libvlc_video_move_resize(m_pMediaPlayer, x, y, w, h, &m_VLCex);
			ProcessVLCException(&m_VLCex, m_EH);
		}
	}
	return nRet;
}

int VLCWrapperImpl::ShowWindow(int nShow)
{
	int nRet = 0;

	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_has_vout(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);

		if ( nRet )
		{
			libvlc_video_show_window(m_pMediaPlayer, nShow, &m_VLCex);
			ProcessVLCException(&m_VLCex, m_EH);
		}
	}

	return nRet;
}

int VLCWrapperImpl::IsWindowVisible(void)
{
	int nRet = 0;

	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_has_vout(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);

		if ( nRet )
		{
			nRet = libvlc_video_is_window_visible(m_pMediaPlayer, &m_VLCex);
			ProcessVLCException(&m_VLCex, m_EH);
		}
	}

	return nRet;
}

int VLCWrapperImpl::DestroyWindow(void)
{
	int nRet = 0;

	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_has_vout(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);
		if ( nRet )
		{
			libvlc_video_destroy_window(m_pMediaPlayer, &m_VLCex);
			ProcessVLCException(&m_VLCex, m_EH);
		}
	}
	return nRet;
}

int VLCWrapperImpl::GetPlayerState(void)
{
	int nRet = 0;
	if ( m_pMediaPlayer )
	{
		nRet = libvlc_media_player_get_state(m_pMediaPlayer, &m_VLCex);
		ProcessVLCException(&m_VLCex, m_EH);
	}
	return nRet;
}
