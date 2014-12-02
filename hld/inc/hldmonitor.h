//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_HLD_MONITOR_DEF_
#define	_TLV_HLD_MONITOR_DEF_


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldMon
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;
public:
	CHldMon(void);
	~CHldMon();

	int HLD_LauchTask_CaptureAndPlay(int nSlot);
	int HLD_TH_START_MON(int nSlot);
	int HLD_TH_CONT_MON(int nSlot);

	//2012/7/10 DVB-T2 ASI
	int HLD_LauchTask_Capture(int nSlot);


};


#endif

