//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_HLD_CAPTURE_DEF_
#define	_TLV_HLD_CAPTURE_DEF_

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldCap
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:
	CHldCap(void);
	~CHldCap();

	int HLD_TH_START_REC(int nSlot);
	int HLD_TH_CONT_REC(int nSlot);

};


#endif

