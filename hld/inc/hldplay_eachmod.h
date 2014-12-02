//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_HLD_PLAY_EACH_MODE_DEF_
#define	_TLV_HLD_PLAY_EACH_MODE_DEF_


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldPlayEachMod
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:
	CHldPlayEachMod(void);
	~CHldPlayEachMod();


};

#endif

