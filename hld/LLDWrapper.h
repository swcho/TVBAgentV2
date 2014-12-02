
#ifndef __LLDWRAPPER_H__
#define __LLDWRAPPER_H__

#include	"../include/logfile.h"

#include	<time.h>

#if defined(WIN32)
#else
#include	<pthread.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<stdarg.h>
#endif

#include	"hld_bd_log.h"
#include	"local_def.h"
#include	"hld_fmtr_ip.h"
#include	"hld_fmtr_loopthru.h"
#include	"dvbt2_multiplexer.h"
#include	"dvbc2_multiplexer.h"
#include	"hldplay.h"
#include	"play.h"



class CHld :		public	CHldFmtrIp,	public	CHldFmtrLoopThru
{
private:
	char	debug_string[100];

public:
	int	my_hld_id;	//	same # as _CBdSysConf->_bd_cnxt[__id__].__id__
	CHldGVar	*_SysSta;
	CHldFsRdWr	*_FIf;
	CHldBdLog	*_HLog;
	CHldFmtrCmmb	*__Cmmb;	//	frame analyzer

	int 	TL_nDemuxBlockTest;


//	DVB-C2
//	int __C2_BW;
//	int __C2_L1;
//	int __C2_Guard;
//	int __C2_Network;
//	int __C2_System;
//	int __C2_StartFreq;
//	int __C2_NumNoth;
//	int __C2_RevTone;
//	int __C2_NotchStart;
//	int __C2_NotchWidth;
//	int __C2_Dslice_type;
//	int __C2_Dslice_FecHeder;
//	int __C2_Dslice_BBHeder;
//	int __C2_Dslice_TonePos;
//	int __C2_Dslice_OffRight;
//	int __C2_Dslice_OffLeft;
//	int __C2_Plp_Mod;
//	int __C2_Plp_Code;
//	int __C2_Plp_Fec;

// huy: 
// if use TMCC
	CHldPlayback playback;
	CHldPlayConcreteObserver concrete_observer;

	ISDBT_PARAM tmcc_param; // adding for interface with ISDB-T module 
	DVBT2_PARAM dvbt2_param;
	ISDBT_LAYER_INFORMATION isdbt_tmcc_layer_info;

	DVBC2_PARAM dvbc2_param;
public:
	CHld(int _my_id);
	virtual ~CHld();

public:

	int AtStartingPos_RcvIp_or_RdFile_OnPlayCont__HasEndlessWhile(void);
	int ProcessDta_1stLoop_RcvIp_or_RdFile_Dta_OnPlayCont(void);
	int ProcessDta_2ndLoop_RcvIp_or_RdFile_Dta_OnPlayCont(void);
	int AtStartingPos_FmtrDta_RcvIp_or_RdFile_OnPlayCont(void);
	int FillPlayBuf_by_Dta_of_RdFile_OnPlayCont__HasEndlessWhile(int *_red_thisturn);
	int UpdPlayBufWIndex_SendIpQ_OnPlayCont(int _red_thisturn);
	int AdjLoopStartingPos_OnPlayEnd(void);

	int CurOffsetChanged_by_User_OnPlayCont(void);
	int SectionRepeatChanged_by_User_OnPlayCont(void);
	int FirstFRead_OnPlayCont(void);
	int GetPlay_BufSts(void);
	void	WaitAppCntl_NxtFileReady_OnPlayEnd__HasEndlessWhile(void);	//	non sense
	int SetTsio_Direction__HasEndlessWhile(int nDirection);


	/* TVB390V8 Demux Block Test Mode */
	int		TL_ProcessDemuxBlockCapture(HANDLE hFile, int NewModeRequest);
	int		TL_ProcessDemuxBlockPlay(HANDLE hFile, int NewModeRequest);

	/* TEST/DEBUGGING */
	void TL_TestTSPattern(char* szFileName);

	//CRC8
	int TL_Calculate_L1_Post_Size(int L1_MOD, int FFT_SIZE, int L1_POST_INFO_SIZE);


	//------------------------------------------------------------------------

};


#endif //__LLDWRAPPER_H__

