#ifdef WIN32
#else
#include <string.h> 
#endif
#include	"hld_bd_log.h"

static	int	_waiting_cnt_interval_each_nodes[_CNT_MAX__NODES_of_CNTED_PRINT_]	=
{
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,		//	0 ~ 9
	100, 500, 100, 500, 500, 500, 100, 100, 100, 100,		//	10 ~
	10, 200, 1000, 30, 30, 30, 30, 300, 300, 300,		//	20 ~
	40, 40, 40, 40, 200, 30, 30, 100, 200, 100,		//	30 ~
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50,		//	40 ~ 49
};
static	char	*_internal_task_string_[]	=
{
	"TH_NONE",
	"TH_START_MON",
	"TH_CONT_MON",
	"TH_START_REC",
	"TH_CONT_REC",
	"TH_STOP_REC",
	"TH_START_PLAY",
	"TH_CONT_PLAY",
	"TH_END_PLAY",
	"TH_START_DELAY",
	"TH_CONT_DELAY"
};
static	char	*_modtr_typ_string_[] =		//	refer to szRegString_KEY[]
{
	"DVB-T",
	"VSB",
	"QAM-A",
	"QAM-B",
	"DVB-S",
	"TDMB",	//	5
	"16VSB",
	"DVB-H",
	"DVB-S2",
	"ISDB-T",	//	9
	"ISDB-T-13",
	"DTMB",
	"CMMB",
	"DVB-T2",
	"RESERVED-O",
	"ATSC-MH",	//	15
	"IQ-PLAY",
	"ISDB-S",	//	17
	"DVB-C2",
	"Startup",

	"not-defined",
	"not-defined"
};
static	char	*_tsio_string_[] =		//	TSIO_CUR_STATE__
{
	"TSIO_PLAY_WITH_310INPUT or TSIO_TS_ASIINPUT",
	"TSIO_PLAY_WITH_ASIINPUT or TSIO_TS_310INPUT",
	"TSIO_310_LOOPTHRU or TSIO_TS_DEFAULTNIM",
	"TSIO_ASI_LOOPTHRU",
	"TSIO_310_CAPTURE_PLAY or TSIO_TS_ASIINPUT_SMB",
	"TSIO_ASI_CAPTURE_PLAY",
	"TSIO_FILE_LOOP_PLAY",

	"not-defined",
	"not-defined"
};
static	char	*_vlc_sta_string_[] =		//	VLC_OPERATION
{
	"VLC_RUN_NO_READY",
	"VLC_SEND_IP_STREAM",
	"VLC_RECV_IP_STREAM",
	"VLC_RECV_IP_STREAM_REC",
	"VLC_RUN_FILE_DISPLAY",
	"VLC_RUN_ASI_DISPLAY",
	"VLC_SEND_IP_STREAM_REC",
	"VLC_OPERATION_MAX",

	"not-defined",
	"not-defined"
};


//////////////////////////////////////////////////////////////////////////////////////
CHldBdLog::CHldBdLog(void)
{
	dbg_notice = 0;
	dbg_warning = 0;
	dbg_noisy = 0;
	dbg_tmr = 0;

	memset((char *)print_cnt_interval, 0, sizeof(int)*_CNT_MAX__NODES_of_CNTED_PRINT_);
#if defined(WIN32)
	memset((char *)base_msec, 0, sizeof(unsigned long)*_CNT_MAX__NODES_of_TIMER_);
#else
	memset((char *)base_msec, 0, sizeof(long long int)*_CNT_MAX__NODES_of_TIMER_);
#endif

	fbdlog = NULL;

//	OpenF_HldPrint();
}

CHldBdLog::~CHldBdLog()
{
	CloseF_HldPrint();
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned long	CHldBdLog::_dbg_msec_(int _tmr_id)
{
	unsigned long	_ret;

	if (_tmr_id >= _CNT_MAX__NODES_of_TIMER_)
	{
		return	0;
	}

#if defined(WIN32)
	unsigned long	cur_msec;

	cur_msec = GetTickCount();	//	timeGetTime();
	if (cur_msec >= base_msec[_tmr_id])
	{
		_ret = (cur_msec - base_msec[_tmr_id]);
	}
	else
	{
		_ret = 0;
	}
	base_msec[_tmr_id] = cur_msec;

#else
	long long int	cur_msec;
	struct	timeval	tv;

	gettimeofday(&tv, NULL);
	cur_msec = ((long long int)tv.tv_sec*(long long int)1000) + (long long int)(tv.tv_usec/1000);

	if (cur_msec >= base_msec[_tmr_id])
	{
		_ret = (cur_msec - base_msec[_tmr_id]);
	}
	else
	{
		_ret = 0;
	}
	base_msec[_tmr_id] = cur_msec;
#endif

	return	(unsigned long)_ret;
}
void	CHldBdLog::OpenF_HldPrint(int __bd_loc__, char *subFolder)
{
	sprintf(__buf, "TPG.LOG/%s/Hld___%d.log", subFolder, __bd_loc__);
	fbdlog = fopen(__buf,"w");
}
void	CHldBdLog::CloseF_HldPrint(void)
{
	if (fbdlog == NULL)
	{
		return;
	}
	fclose(fbdlog);
	fbdlog = NULL;
}
int	CHldBdLog::ChkPrtValidity(void)
{
	if (dbg_notice != 1)
	{
		return	0;
	}
	if (fbdlog == NULL)
	{
		return	0;
	}
	return	1;
}
int	CHldBdLog::ChkPrtValidity_Noisy(void)
{
	if (dbg_noisy != 1)
	{
		return	0;
	}
	if (fbdlog == NULL)
	{
		return	0;
	}
	return	1;
}
int	CHldBdLog::ChkPrtValidity_Tmr(void)
{
	if (dbg_tmr != 1)
	{
		return	0;
	}
	if (fbdlog == NULL)
	{
		return	0;
	}
	return	1;
}

void	CHldBdLog::HldPrint_VlcSta(int _from, int _to)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "Hld-Vlc. ++.. vlc-sta : From-To : [%s].[%s]\n", _vlc_sta_string_[_from], _vlc_sta_string_[_to]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_Tsio(int _typ)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "Hld-Bd-Ctl. ++.. Set-Tsio-Direction : [%s]\n", _tsio_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_ModtrTyp(int _typ)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "Hld-Bd-Ctl. ++.. Set-Modtr-Typ : [%s]\n", _modtr_typ_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_InternalState(int _from, int _to)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "Hld-Mgr. ++.. State of Task : From-To : [%s].[%s]\n", _internal_task_string_[_from], _internal_task_string_[_to]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_1_s(char *str, char *str2)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%s]\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint(char *str, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint(char *str, char *str2, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%s].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint(char *str, char *str2, char *str3, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%s].[%s].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2, (str3 == NULL) ? "null" : str3, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_2(char *str, int para1, int para2)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_2_d(char *str, int para1, double para2)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%lld]\n", (str == NULL) ? "null" : str, para1, para2);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_1(char *str, int para1)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d]\n", (str == NULL) ? "null" : str, para1);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint(char *str)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s\n", (str == NULL) ? "null" : str);
	fprintf(fbdlog, __buf);
//	fflush(fbdlog);
}
void	CHldBdLog::HldPrint_CntWait(char *str, unsigned int print_point_ind)
{
	if (ChkPrtValidity() == 0)	return;
	if (print_point_ind >= _CNT_MAX__NODES_of_CNTED_PRINT_)	return;

	print_cnt_interval[print_point_ind]++;
	if (print_cnt_interval[print_point_ind] < _waiting_cnt_interval_each_nodes[print_point_ind])
	{
		return;
	}
	print_cnt_interval[print_point_ind] = 0;
	sprintf(__buf, "%s\n", (str == NULL) ? "null" : str);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_CntWait_2(char *str, unsigned int print_point_ind, int _para)
{
	if (ChkPrtValidity() == 0)	return;
	if (print_point_ind >= _CNT_MAX__NODES_of_CNTED_PRINT_)	return;

	print_cnt_interval[print_point_ind]++;
	if (print_cnt_interval[print_point_ind] < _waiting_cnt_interval_each_nodes[print_point_ind])
	{
		return;
	}
	print_cnt_interval[print_point_ind] = 0;
	sprintf(__buf, "%s : [%d]\n", (str == NULL) ? "null" : str, _para);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_CntWait_3(char *str, unsigned int print_point_ind, int _para, int _para2)
{
	if (ChkPrtValidity() == 0)	return;
	if (print_point_ind >= _CNT_MAX__NODES_of_CNTED_PRINT_)	return;

	print_cnt_interval[print_point_ind]++;
	if (print_cnt_interval[print_point_ind] < _waiting_cnt_interval_each_nodes[print_point_ind])
	{
		return;
	}
	print_cnt_interval[print_point_ind] = 0;
	sprintf(__buf, "%s : [%d].[%d]\n", (str == NULL) ? "null" : str, _para, _para2);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_WrHw(int _typ, int where_from)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[39]++;
	if (print_cnt_interval[39] < _waiting_cnt_interval_each_nodes[39])
	{
		return;
	}
	print_cnt_interval[39] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Wr-Dta-to-Hw : [%s]...[%d]\n", _modtr_typ_string_[_typ], where_from);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_RdHw_LoopThruMode(int _typ)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[38]++;
	if (print_cnt_interval[38] < _waiting_cnt_interval_each_nodes[38])
	{
		return;
	}
	print_cnt_interval[38] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Rd-Dta-Hw and Wr-File and Fill Play-Buf : loopthru-mode : [%s]\n", _modtr_typ_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_WrHw_LoopThruMode(int _typ)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[37]++;
	if (print_cnt_interval[37] < _waiting_cnt_interval_each_nodes[37])
	{
		return;
	}
	print_cnt_interval[37] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Wr-Dta-to-Hw : loopthru-mode : [%s]\n", _modtr_typ_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_FillPlayBuf_IpDta(int _typ)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[36]++;
	if (print_cnt_interval[36] < _waiting_cnt_interval_each_nodes[36])
	{
		return;
	}
	print_cnt_interval[36] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Fill-Ip-Dta into Play-Buf : is to be witten to Hw : [%s]\n", _modtr_typ_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_FillPlayBuf_FileDta(int _typ)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[35]++;
	if (print_cnt_interval[35] < _waiting_cnt_interval_each_nodes[35])
	{
		return;
	}
	print_cnt_interval[35] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Fill-File-Dta into Play-Buf : is to be witten to Hw : [%s]\n", _modtr_typ_string_[_typ]);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_FindSync_PlayCond(int _typ, int _sync)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[34]++;
	if (print_cnt_interval[34] < _waiting_cnt_interval_each_nodes[34])
	{
		return;
	}
	print_cnt_interval[34] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Find Sync Play-Cond : [%s]...[%d]\n", _modtr_typ_string_[_typ], _sync);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_IpBuf_Monitor(int _typ, float _f1, float _f2, float _f3, int _i2, int _i3, int _i4, int _i5)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[20]++;
	if (print_cnt_interval[20] < _waiting_cnt_interval_each_nodes[20])
	{
		return;
	}
	print_cnt_interval[20] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Ip-Buf-Mon : [%s] : i=[%f]..o=[%f]..v=[%f]..p=[%d].c=[%d/%d].i=[%d]\n", _modtr_typ_string_[_typ], _f1, _f2, _f3, _i2, _i3, _i4, _i5);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_IpBuf_Monitor_2(int _typ, int _i0, int _i1, int _i2, int _i3, int _i4, int _i5)
{
	if (ChkPrtValidity_Noisy() == 0)	return;

	print_cnt_interval[20]++;
	if (print_cnt_interval[20] < _waiting_cnt_interval_each_nodes[20])
	{
		return;
	}
	print_cnt_interval[20] = 0;

	sprintf(__buf, "Hld-Bd-Ctl. ++.. Ip-Buf-Mon : [%s] : dta=[%d].R=[%d]..iip=[%d].wh=[%d].ins=[%d].rmv=[%d]\n", _modtr_typ_string_[_typ], _i0, _i1, _i2, _i3, _i4, _i5);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_Tmr(int _tmr, char *str)
{
	if (ChkPrtValidity_Tmr() == 0)	return;
	sprintf(__buf, "[tmr-%d] : %u msec ... %s\n", _tmr, _dbg_msec_(_tmr), (str == NULL) ? "null" : str);
	fprintf(fbdlog, __buf);
}
void	CHldBdLog::HldPrint_Tmr(int _tmr, char *str, int _i1)
{
	if (ChkPrtValidity_Tmr() == 0)	return;
	sprintf(__buf, "[tmr-%d] : %u msec ... %s ... [%d]\n", _tmr, _dbg_msec_(_tmr), (str == NULL) ? "null" : str, _i1);
	fprintf(fbdlog, __buf);
}


