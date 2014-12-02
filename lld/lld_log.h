
#ifndef __LLD_BD_LOG_H__
#define __LLD_BD_LOG_H__

#if defined(WIN32)
#include	<Windows.h>
#include	<io.h>
#endif
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>

#define	_CNT_MAX__NODES_of_TIMER_		10
#define	_CNT_MAX__NODES_of_CNTED_PRINT_	50
class CLldLog
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;
	int	dbg_fcall;
	int	dbg_tmr;

	int	print_cnt_interval[_CNT_MAX__NODES_of_CNTED_PRINT_];
	char	__buf[2048];

	FILE	*fbdlog;

	unsigned long base_msec[_CNT_MAX__NODES_of_TIMER_];

	unsigned long	_sys_msec_(void);
	unsigned long	_dbg_msec_(int _tmr_id);


public:
	CLldLog(void);
	virtual ~CLldLog();

	void	OpenF_LldPrint(int __bd_loc__, char *szSubDir);
	void	CloseF_LldPrint(void);

	int		ChkPrtValidity(void);
	int		ChkPrtValidity_Err(void);
	int		ChkPrtValidity_Noisy(void);
	int		ChkPrtValidity_FCall(void);
	int		ChkPrtValidity_Tmr(void);

	void	LldPrint(char *str, int para1, int para2, int para3);
	void	LldPrint_lul(char *str, long para1, unsigned long para2, long para3);
	void	LldPrint_ldl(char *str, long para1, double para2, long para3);
	void	LldPrint_ldll(char *str, long para1, double para2, long para3, long para4);
	void	LldPrint_ddi(char *str, double para1, double para2, int para3);
	void	LldPrint_ddd(char *str, double para1, double para2, double para3);
	void	LldPrint_4(char *str, int para1, int para2, int para3, double para4);
	void	LldPrint(char *str, char *str2, int para1, int para2, int para3);
	void	LldPrint(char *str, char *str2, char *str3, int para1, int para2, int para3);
	void	LldPrint_5(char *str, int para1, int para2, int para3, int para4, int para5);
	void	LldPrint_6(char *str, int para1, int para2, int para3, int para4, int para5, int para6);
	void	LldPrint_8(char *str, int para1, int para2, int para3, int para4, int para5, int para6, int para7, int para8);
	void	LldPrint_2(char *str, int para1, int para2);
	void	LldPrint_1(char *str, int para1);
	void	LldPrint_1x(char *str, int para1);
	void	LldPrint_1x_nochk_prtoption(char *str, int para1);
	void	LldPrint_x_CIC(char *str, char para1, int para2, char para3);
	void	LldPrint(char *str);
	void	LldPrint_s_s(char *str, char *str2);
	void	LldPrint_CntWait(char *str, unsigned int print_point_ind);
	void	LldPrint_CntWait_2(char *str, unsigned int print_point_ind, int _para);
	void	LldPrint_CntWait_3(char *str, unsigned int print_point_ind, int _para, int _para2);
	void	LldPrint_FCall(char *str, int para1, int para2);

	void	LldPrint_Error(char *str, int para1, int para2);

	void	LldPrint_Trace(char *str, int para1, int para2, double para3, char *str2);
	void	LldPrint_Tmr(int _tmr, char *str);
	void	LldPrint_Tmr(int _tmr, char *str, int _i1);

	void	DBG_PLATFORM_BRANCH(void);
	void	DBG_PRINT_MOD_TYP(int typ, int en);
	void	DBG_PRINT_BD_CONF(int _slot,
			int TSPL_nBoardTypeID,
			int TSPL_nUseRemappedAddress,
			int TSPL_nBoardRevision,
			int TSPL_nBoardUseAMP,
			int TSPL_nTVB595Board,
			int TSPL_nBoardRevision_Ext,
			int TSPL_nBoardLocation);
	void	DBG_PRINT_FPGA_CONF(int TSPL_nFPGA_ID, int TSPL_nFPGA_VER, int TSPL_nFPGA_BLD, int TSPL_nFPGA_RBF, int TSPL_nFPGA_IQ_PLAY, int TSPL_nFPGA_IQ_CAPTURE);
	void	DBG_PRINT_INIT_CONF(
		int TSPL_nModulatorType,
		int TSPL_nModulatorEnabled,
		int TSPL_nBoardOption,
		int TSPL_nUse10MREFClock,
		int TSPL_nBankOffset,
		int TSPL_nBankCount);
	void	DBG_PRINT_RECONF_MOD(long modulator_type, long IF_Frequency);
#ifdef WIN32
	void	DBG_PRINT_DRV_MSG_LOG(HANDLE	_hnd);
#endif

};


#endif

