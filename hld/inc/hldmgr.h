//=================================================================	
//	WDM_Drv.h / Device Driver Interface for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef	_TLV_HLD_MGR_DEF_
#define	_TLV_HLD_MGR_DEF_

#if defined(WIN32)
#include	<stdio.h>
#include	<windows.h>
#include	<io.h>
#include	<fcntl.h>
#include	<process.h>
#include	<time.h>
#else
#define		_FILE_OFFSET_BITS	64
#define		_LARGEFILE_SOURCE	1
#include	<stdio.h>
#include	<pthread.h>
#endif

#include	"../../include/logfile.h"
#include	"../LLDWrapper.h"
#include	"../VLCWrapper.h"

#include	"../include/lld_structure.h"

#include	"hldcapture.h"
#include	"hldmonitor.h"
#include	"hldplayback.h"

extern FILE	*fn_Log;
//kslee debug 2011/3/9
//extern FILE *fn_debug;
extern CHld *g_LLDWrapper[];
//vlc1.0.3
#if defined(WIN32)
extern VLCWrapper *g_VLCWrapper;
#else
static VLCWrapper *g_VLCWrapper;
#endif
extern unsigned char	gNullPack[204];

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldMgr	:	public	CHldCap,
	public	CHldMon,
	public	CHldPlay
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

public:
#if defined(WIN32)
#else
	pthread_t	 a_thread[_MAX_INST_CNT_+1];
	void		*thread_result;
#endif

public:
	CHldMgr(void);
	~CHldMgr();

	int HLD_MainTaskStateLoop_Play(int nSlot);
	int HLD_MainTaskStateLoop_Rec(int nSlot);
	int	HLD_MainTaskStateLoop_Monitor(int nSlot);
	int	HLD_StateMainLoop(int nSlot);
#if defined(WIN32)
	static	void	HLDThreadEngine(PVOID param);
#else
	static	void	*HLDThreadEngine(PVOID param);
#endif

};
extern	CHldMgr	_CHldMgr;

class	CBdSysConf
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

	_BD_CONF_CNXT	_bd_cnxt[_MAX_INST_CNT_];

	int	g_MultiBoard;
	int	cnt_phy_bd;
	int	cnt_phy_and_vir_bd;
	char szDateInfo[16];

private:

public:
	CBdSysConf(void);
	~CBdSysConf();

	CHld	*CreateOneCHld(int _id);
	void	DeleteOneCHld(CHld *_hld);
	int AsignSlotNofRealTvbBd(CHld *_hld);
	int AsignSlotNofVirTvbBd(int _r_bd_id_to_copy_from, int _ts_seq_);
	int InitInstalledPhyTvbBd(void);

	_BD_CONF_CNXT	*Cnxt(int _rd_id);
	int GetRealBdCnt_N(void);
	int GetBdVirCnt_N(void);
	int GetBdCnt_N(void);
	int GetRealAndVirBdMap(int _realN, int *_map);
	int GetBdType_N(int _Nth);	//	___ID_BDs___
	char *GetBdName_N(int _Nth);
	int DetectInstalledTvbBd(int _multi_bd);
	int InitOneRealBd(int _bd_id, CHld *_hld);
	int InitOneVirBd(int _r_bd_id, int _v_bd_id, CHld *_hld);
	int ActivateOneRealBd(CHld *_hld, int _init_modulator, int _init_if_freq);

};

#endif

