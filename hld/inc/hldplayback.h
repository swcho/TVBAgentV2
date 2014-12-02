//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_HLD_PLAYBACK_DEF_
#define	_TLV_HLD_PLAYBACK_DEF_

#include "hldplay_eachmod.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldPlay
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:
	CHldPlay(void);
	~CHldPlay();

	int HLD_TH_START_PLAY(int nSlot, int IP_RESET_ONLY);
	int HLD_TH_CONT_PLAY(int nSlot);
	int HLD_TH_END_PLAY(int nSlot);

};

#endif

