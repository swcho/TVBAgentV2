
#ifdef WIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<windows.h>
#include	<winioctl.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<commctrl.h>
#include	<time.h>
#include	<direct.h>

#include	"Ioctl.h"

#else
#define _FILE_OFFSET_BITS 64
#include 	<stdio.h>
#include 	<stdlib.h>
#include	<unistd.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include	<sys/time.h>
#endif

#include	"mainmode.h"
#include	"lld_regs.h"
#include	"lld_log.h"

#ifdef	_DBG_ON_TARGET_TH4435__
#define	_DBG_MON_CONSOLE
#endif

static	int	_waiting_cnt_interval_each_nodes[_CNT_MAX__NODES_of_CNTED_PRINT_]	=
{
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,		//	0 ~ 9
	100, 500, 100, 500, 500, 500, 100, 100, 100, 100,		//	10 ~
	10, 200, 30, 30, 30, 30, 30, 300, 300, 300,		//	20 ~
	40, 40, 40, 40, 200, 30, 30, 100, 200, 100,		//	30 ~
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50,		//	40 ~ 49
};
static	char	*__mod_typ_str__[] =		//	refer to szRegString_KEY[]
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
	"MULTI-QAMB",
	"MULTI-VSB",
	"MULTI-DVBT",
	"Startup",

	"not-defined",
	"not-defined"
	"not-defined",
	"not-defined"
	"not-defined",
	"not-defined"
	"not-defined",
	"not-defined"
};


//////////////////////////////////////////////////////////////////////////////////////
CLldLog::CLldLog(void)
{
	dbg_notice = 1;
	dbg_warning = 1;
	dbg_noisy = 0;
	dbg_fcall = 0;
	dbg_tmr = 0;

	memset((char *)print_cnt_interval, 0, sizeof(int)*_CNT_MAX__NODES_of_CNTED_PRINT_);
	memset((char *)base_msec, 0, sizeof(unsigned long)*_CNT_MAX__NODES_of_TIMER_);

	fbdlog = NULL;

}

CLldLog::~CLldLog()
{
	CloseF_LldPrint();
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned long	CLldLog::_sys_msec_(void)
{
#if defined(WIN32)
	return	GetTickCount();
#else
	return	0;
#endif
}
unsigned long	CLldLog::_dbg_msec_(int _tmr_id)
{
	unsigned long	_ret;

#if defined(WIN32)
	unsigned long	cur_msec;

	if (_tmr_id >= _CNT_MAX__NODES_of_TIMER_)
	{
		return	0;
	}

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

	return	_ret;
}
void	CLldLog::OpenF_LldPrint(int __bd_loc__, char *szSubDir)
{
#ifdef	WIN32
	_mkdir("TPG.LOG");
	sprintf(__buf, "TPG.LOG/%s", szSubDir);
	_mkdir(__buf);
#else
#ifdef _DBG_MON_CONSOLE
	fbdlog = stderr;
	return;
#endif
	mkdir("TPG.LOG", 0777);
	sprintf(__buf, "TPG.LOG/%s", szSubDir);
	mkdir(__buf, 0777);
#endif
	sprintf(__buf, "TPG.LOG/%s/lld_bd_%d.log", szSubDir, __bd_loc__);

#ifdef WIN32
	fopen_s(&fbdlog, __buf,"w");
#else
	fbdlog = fopen(__buf,"w");
#endif
}
void	CLldLog::CloseF_LldPrint(void)
{
#ifdef _DBG_MON_CONSOLE
	fbdlog = NULL;
	return;
#endif
	if (fbdlog == NULL)
	{
		return;
	}
	fclose(fbdlog);
	fbdlog = NULL;
}
int	CLldLog::ChkPrtValidity(void)
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
int	CLldLog::ChkPrtValidity_Err(void)
{
	if (dbg_warning != 1)
	{
		return	0;
	}
	if (fbdlog == NULL)
	{
		return	0;
	}
	return	1;
}
int	CLldLog::ChkPrtValidity_Noisy(void)
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
int	CLldLog::ChkPrtValidity_FCall(void)
{
	if (dbg_fcall != 1)
	{
		return	0;
	}
	if (fbdlog == NULL)
	{
		return	0;
	}
	return	1;
}
int	CLldLog::ChkPrtValidity_Tmr(void)
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

void	CLldLog::LldPrint(char *str, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_lul(char *str, long para1, unsigned long para2, long para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%u].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_ldl(char *str, long para1, double para2, long para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%f].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_ldll(char *str, long para1, double para2, long para3, long para4)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%f].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3, para4);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_ddi(char *str, double para1, double para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%f].[%f].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_ddd(char *str, double para1, double para2, double para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%f].[%f].[%f]\n", (str == NULL) ? "null" : str, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_4(char *str, int para1, int para2, int para3, double para4)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d].[%f]\n", (str == NULL) ? "null" : str, para1, para2, para3, para4);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint(char *str, char *str2, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%s].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint(char *str, char *str2, char *str3, int para1, int para2, int para3)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%s].[%s].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2, (str3 == NULL) ? "null" : str3, para1, para2, para3);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_5(char *str, int para1, int para2, int para3, int para4, int para5)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3, para4, para5);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_6(char *str, int para1, int para2, int para3, int para4, int para5, int para6)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3, para4, para5, para6);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_8(char *str, int para1, int para2, int para3, int para4, int para5, int para6, int para7, int para8)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d].[%d].[%d].[%d].[%d].[%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2, para3, para4, para5, para6, para7, para8);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_2(char *str, int para1, int para2)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_1(char *str, int para1)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [%d]\n", (str == NULL) ? "null" : str, para1);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_1x(char *str, int para1)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : [0x%x]\n", (str == NULL) ? "null" : str, para1);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_1x_nochk_prtoption(char *str, int para1)
{
	if (fbdlog == NULL)	return;
	sprintf(__buf, "%s : [0x%x]\n", (str == NULL) ? "null" : str, para1);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_x_CIC(char *str, char para1, int para2, char para3)
{
return;

//	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s : off=[0x%x].dta=[0x%x].ext=[0x%x]\n", str, para1 & 0xff, para2, para3 & 0xff);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint(char *str)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s\n", (str == NULL) ? "null" : str);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_s_s(char *str, char *str2)
{
	if (ChkPrtValidity() == 0)	return;
	sprintf(__buf, "%s %s\n", (str == NULL) ? "null" : str, (str2 == NULL) ? "null" : str2);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_CntWait(char *str, unsigned int print_point_ind)
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
void	CLldLog::LldPrint_CntWait_2(char *str, unsigned int print_point_ind, int _para)
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
void	CLldLog::LldPrint_CntWait_3(char *str, unsigned int print_point_ind, int _para, int _para2)
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
void	CLldLog::LldPrint_FCall(char *str, int para1, int para2)
{
	if (ChkPrtValidity_FCall() == 0)	return;
	sprintf(__buf, "[lld-func-call] : %s : [%d].[%d]\n", (str == NULL) ? "null" : str, para1, para2);
	fprintf(fbdlog, __buf);
}

void	CLldLog::LldPrint_Error(char *str, int para1, int para2)
{
	if (ChkPrtValidity_Err() == 0)	return;
	sprintf(__buf, "[lld-ERROR] : %s : [%x].[%x]\n", (str == NULL) ? "null" : str, para1, para2);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_Trace(char *str, int para1, int para2, double para3, char *str2)
{
	if (ChkPrtValidity_Noisy() == 0)	return;
	sprintf(__buf, "[lld-trace] : %s : [%d].[%d].[%f] : [%s]\n", (str == NULL) ? "null" : str, para1, para2, para3, (str2 == NULL) ? "null" : str2);
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_Tmr(int _tmr, char *str)
{
	if (ChkPrtValidity_Tmr() == 0)	return;
	sprintf(__buf, "[tmr-%d] : %u msec ... %s ::: sys-tm. %u msec\n", _tmr, _dbg_msec_(_tmr), (str == NULL) ? "null" : str, _sys_msec_());
	fprintf(fbdlog, __buf);
}
void	CLldLog::LldPrint_Tmr(int _tmr, char *str, int _i1)
{
	if (ChkPrtValidity_Tmr() == 0)	return;
	sprintf(__buf, "[tmr-%d] : %u msec ... %s ... [%d] ::: sys-tm. %u msec\n", _tmr, _dbg_msec_(_tmr), (str == NULL) ? "null" : str, _i1, _sys_msec_());
	fprintf(fbdlog, __buf);
}

void	CLldLog::DBG_PLATFORM_BRANCH(void)
{
	if (ChkPrtValidity_Noisy() == 0)	return;
#ifdef WIN32
	fprintf(fbdlog, "++++++[lld-platform] : WIN32\n");
#else
	fprintf(fbdlog, "++++++[lld-platform] : LINUX\n");
#endif
}
void	CLldLog::DBG_PRINT_MOD_TYP(int typ, int en)
{
	if (ChkPrtValidity() == 0)	return;

	if (en < 0)
	{
		if ((typ < 0) || (typ >= TLV_MODULATOR_TYPE_MAX))
		{
			fprintf(fbdlog, "\n:::[lld-sta] : MOD-UNDEFINED : [%d]\n", typ);
		}
		else
		{
			fprintf(fbdlog, "\n:::[lld-sta] : MOD-DISABLED : [%s]\n", __mod_typ_str__[typ]);
		}
	}
	else
	{
		if ((typ < 0) || (typ >= TLV_MODULATOR_TYPE_MAX))
		{
			fprintf(fbdlog, "\n:::[lld-sta] : MOD-UNDEFINED : [%d]\n", typ);
		}
		else
		{
			fprintf(fbdlog, "\n:::[lld-sta] : MOD-ENABLED : [%s]\n", __mod_typ_str__[typ]);
		}
	}
}
void	CLldLog::DBG_PRINT_BD_CONF(int _slot,
		int TSPL_nBoardTypeID,
		int TSPL_nUseRemappedAddress,
		int TSPL_nBoardRevision,
		int TSPL_nBoardUseAMP,
		int TSPL_nTVB595Board,
		int TSPL_nBoardRevision_Ext,
		int TSPL_nBoardLocation)
{
//	if (ChkPrtValidity() == 0)	return;

	fprintf(fbdlog, "\n");
	fprintf(fbdlog, "...[bd-conf] : req-id=[d'%d]\n", _slot);
	switch(TSPL_nBoardTypeID)
	{
	case	_BD_ID_380__:			fprintf(fbdlog, "...[bd-conf] : bd-typ=[380]\n");		break;
	case	_BD_ID_380v2__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[380v2]\n");		break;
	case	_BD_ID_380v3_ifrf_scaler__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[380v3-ifrfscaler]\n");		break;
	case	_BD_ID_380v3_upconverter__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[380v3-upconverter]\n");		break;
	case	_BD_ID_390v6__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v6]\n");		break;
	case	_BD_ID_390v6_Lowcost__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390-lowcost]\n");		break;
	case	_BD_ID_390v7__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v7]\n");		break;
	case	_BD_ID_390v8__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v8]\n");		break;
	case	_BD_ID_390v7_IF_only__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v7-ifonly]\n");		break;
	case	_BD_ID_590v9_x__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v9.x]\n");		break;
	case	_BD_ID_590v10_x__:	fprintf(fbdlog, "...[bd-conf] : bd-typ=[390v10.x]\n");		break;
	case	_BD_ID_595v2__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[595v2]\n");		break;
	case	_BD_ID_595Bv3__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[395bv3]\n");		break;
	case	_BD_ID_590s__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[590s]\n");		break;
	case	_BD_ID_593__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[593]\n");		break;
	case	_BD_ID_597__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[597]\n");		break;
	case	_BD_ID_597v2__:	fprintf(fbdlog, "...[bd-conf] : bd-typ=[597v2]\n");		break;
	case	_BD_ID_497__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[497]\n");		break;
	case	_BD_ID_499__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[499]\n");		break;
	case	_BD_ID_591__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[591]\n");		break;
	case	_BD_ID_594__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[594]\n");		break;
	case	_BD_ID_599__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[599]\n");		break;
	case	_BD_ID_598__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[598]\n");		break;
	//2012/1/31 TVB591S
	case	_BD_ID_591S__:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[591s]\n");		break;
	default:		fprintf(fbdlog, "...[bd-conf] : bd-typ=[unknown].[0x%x]\n", TSPL_nBoardTypeID);		break;
	}
	switch(TSPL_nUseRemappedAddress)
	{
	case	_ADDR_REMAP_METHOD_dont_:	fprintf(fbdlog, "...[bd-conf] : mem-map-typ=[base]\n");		break;
	case	_ADDR_REMAP_METHOD_597_:	fprintf(fbdlog, "...[bd-conf] : mem-map-typ=[597]\n");		break;
	case	_ADDR_REMAP_METHOD_590s_:	fprintf(fbdlog, "...[bd-conf] : mem-map-typ=[590s]\n");		break;
	case	_ADDR_REMAP_METHOD_593_:	fprintf(fbdlog, "...[bd-conf] : mem-map-typ=[599/593/597v2/591/591s]\n");		break;
	}
	fprintf(fbdlog, "...[bd-conf] : bd-rev=[0x%x]\n", TSPL_nBoardRevision);
	fprintf(fbdlog, "...[bd-conf] : amp-typ=[0x%x]\n", TSPL_nBoardUseAMP);
	fprintf(fbdlog, "...[bd-conf] : led-typ=[0x%x]\n", TSPL_nTVB595Board);
	fprintf(fbdlog, "...[bd-conf] : rev-ext=[0x%x]\n", TSPL_nBoardRevision_Ext);
	fprintf(fbdlog, "...[bd-conf] : bd-loc=[d'%d]\n", TSPL_nBoardLocation);
	fprintf(fbdlog, "\n");

}
void	CLldLog::DBG_PRINT_FPGA_CONF(int TSPL_nFPGA_ID, int TSPL_nFPGA_VER, int TSPL_nFPGA_BLD, int TSPL_nFPGA_RBF, int TSPL_nFPGA_IQ_PLAY, int TSPL_nFPGA_IQ_CAPTURE)
{
	if (ChkPrtValidity() == 0)	return;

	fprintf(fbdlog, "\n");
	fprintf(fbdlog, "...[fpga-conf] : fpga-id=[0x%x]\n", TSPL_nFPGA_ID);
	fprintf(fbdlog, "...[fpga-conf] : fpga-ver=[0x%x]\n", TSPL_nFPGA_VER);
	fprintf(fbdlog, "...[fpga-conf] : fpga-bld=[0x%x]\n", TSPL_nFPGA_BLD);
	fprintf(fbdlog, "...[fpga-conf] : fpga-rbf#=[0x%x]\n", TSPL_nFPGA_RBF);
	fprintf(fbdlog, "...[fpga-conf] : fpga-iq-play=[0x%x]\n", TSPL_nFPGA_IQ_PLAY);
	fprintf(fbdlog, "...[fpga-conf] : fpga-iq-capt=[0x%x]\n", TSPL_nFPGA_IQ_CAPTURE);
	fprintf(fbdlog, "\n");
}
void	CLldLog::DBG_PRINT_INIT_CONF(
	int TSPL_nModulatorType,
	int TSPL_nModulatorEnabled,
	int TSPL_nBoardOption,
	int TSPL_nUse10MREFClock,
	int TSPL_nBankOffset,
	int TSPL_nBankCount)
{
	if (ChkPrtValidity() == 0)	return;

	fprintf(fbdlog, "\n");
	fprintf(fbdlog, "...[init-conf] : mod-typ=[0x%x]\n", TSPL_nModulatorType);
	fprintf(fbdlog, "...[init-conf] : mod-en=[0x%x]\n", TSPL_nModulatorEnabled);
	fprintf(fbdlog, "...[init-conf] : bd-opt=[0x%x]\n", TSPL_nBoardOption);
	fprintf(fbdlog, "...[init-conf] : 10m-ref=[0x%x]\n", TSPL_nUse10MREFClock);
	fprintf(fbdlog, "...[init-conf] : bnk-off=[0x%x]\n", TSPL_nBankOffset);
	fprintf(fbdlog, "...[init-conf] : bnk-cnt=[0x%x]\n", TSPL_nBankCount);
	fprintf(fbdlog, "\n");
}
void	CLldLog::DBG_PRINT_RECONF_MOD(long modulator_type, long IF_Frequency)
{
	if (ChkPrtValidity() == 0)	return;

	fprintf(fbdlog, "\n");
	fprintf(fbdlog, "...[RECONF-MOD-TYPE] : mod-typ=[0x%x]...if=[d'%d]", modulator_type, IF_Frequency);
	fprintf(fbdlog, "\n");
}
#ifdef WIN32
void	CLldLog::DBG_PRINT_DRV_MSG_LOG(HANDLE	_hnd)
{
#if	0
	KCMD_ARGS		KCmdInf;
	unsigned long 	dwRet;
	char	*_str;
	int	_ret, i;

	_str = (char *)malloc(512);
	KCmdInf.pdwBuffer = (unsigned long *)_str;
	for (i = 0; i < 100; i++)
	{
		_ret = DeviceIoControl(_hnd, IOCTL_GET_DRV_MSG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
		if (_ret != 0)
		{
			sprintf(__buf, "%s : 0x%x\n", KCmdInf.pdwBuffer, KCmdInf.dwCmdParm2);
			fprintf(fbdlog, __buf);
		}
	}
	free(_str);
#endif

#if	0
	for (i = 0; i < 0x4a; i++)
	{
		KCmdInf.dwCmdParm1 = i;
		DeviceIoControl(_hnd, IOCTL_READ_PCI9054_CONF_REG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
		sprintf(__buf, "CONF-0x%x : 0x%x\n", i, KCmdInf.dwCmdParm2);
		fprintf(fbdlog, __buf);
	}
#endif

#if	0
	KCmdInf.dwCmdParm1 = 0x00/4;	//	4bytes interface
	DeviceIoControl(_hnd, IOCTL_READ_PCI9054_REG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	sprintf(__buf, "LAS0RR : 0x%x\n", KCmdInf.dwCmdParm2);
	fprintf(fbdlog, __buf);

	KCmdInf.dwCmdParm1 = 0x04/4;	//	4bytes interface
	DeviceIoControl(_hnd, IOCTL_READ_PCI9054_REG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	sprintf(__buf, "LAS0BA : 0x%x\n", KCmdInf.dwCmdParm2);
	fprintf(fbdlog, __buf);

	KCmdInf.dwCmdParm1 = 0xf0/4;	//	4bytes interface
	DeviceIoControl(_hnd, IOCTL_READ_PCI9054_REG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	sprintf(__buf, "LAS1RR : 0x%x\n", KCmdInf.dwCmdParm2);
	fprintf(fbdlog, __buf);

	KCmdInf.dwCmdParm1 = 0xf4/4;	//	4bytes interface
	DeviceIoControl(_hnd, IOCTL_READ_PCI9054_REG, &KCmdInf, sizeof(KCmdInf), &KCmdInf, sizeof(KCmdInf), &dwRet, 0);
	sprintf(__buf, "LAS1BA : 0x%x\n", KCmdInf.dwCmdParm2);
	fprintf(fbdlog, __buf);
#endif

}
#endif



