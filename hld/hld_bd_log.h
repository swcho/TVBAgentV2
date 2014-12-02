
#ifndef __HLD_BD_LOG_H__
#define __HLD_BD_LOG_H__

#include	<stdio.h>
#include	<fcntl.h>
#if defined(WIN32)
#include	<Windows.h>
#include	<io.h>
#else
#include  <time.h>
#include  <sys/time.h>
#include  <unistd.h>
#endif

#define	_CNT_MAX__NODES_of_TIMER_		10
#define	_CNT_MAX__NODES_of_CNTED_PRINT_	50
class CHldBdLog
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;
	int	dbg_tmr;

	int	print_cnt_interval[_CNT_MAX__NODES_of_CNTED_PRINT_];
	char	__buf[2048];

	FILE	*fbdlog;

#if defined(WIN32)
	unsigned long base_msec[_CNT_MAX__NODES_of_TIMER_];
#else
	long long int base_msec[_CNT_MAX__NODES_of_TIMER_];
#endif

public:

public:
	CHldBdLog(void);
	virtual ~CHldBdLog();

	unsigned long	_dbg_msec_(int _tmr_id);
	void	OpenF_HldPrint(int __bd_loc__, char *subFolder);
	void	CloseF_HldPrint(void);

	int 	ChkPrtValidity(void);
	int		ChkPrtValidity_Noisy(void);
	int		ChkPrtValidity_Tmr(void);

	void	HldPrint_VlcSta(int _from, int _to);
	void	HldPrint_Tsio(int _typ);
	void	HldPrint_ModtrTyp(int _typ);
	void	HldPrint_InternalState(int _from, int _to);

	void	HldPrint_1_s(char *str, char *str2);
	void	HldPrint(char *str, int para1, int para2, int para3);
	void	HldPrint(char *str, char *str2, int para1, int para2, int para3);
	void	HldPrint(char *str, char *str2, char *str3, int para1, int para2, int para3);
	void	HldPrint_2(char *str, int para1, int para2);
	void	HldPrint_2_d(char *str, int para1, double para2);
	void	HldPrint_1(char *str, int para1);
	void	HldPrint(char *str);

	void	HldPrint_CntWait(char *str, unsigned int print_point_ind);
	void	HldPrint_CntWait_2(char *str, unsigned int print_point_ind, int _para);
	void	HldPrint_CntWait_3(char *str, unsigned int print_point_ind, int _para, int _para2);
	void	HldPrint_WrHw(int _typ, int where_from);
	void	HldPrint_RdHw_LoopThruMode(int _typ);
	void	HldPrint_WrHw_LoopThruMode(int _typ);
	void	HldPrint_FillPlayBuf_IpDta(int _typ);
	void	HldPrint_FillPlayBuf_FileDta(int _typ);
	void	HldPrint_FindSync_PlayCond(int _typ, int _sync);
	void	HldPrint_IpBuf_Monitor(int _typ, float _f1, float _f2, float _f3, int _i2, int _i3, int _i4, int _i5);
	void	HldPrint_IpBuf_Monitor_2(int _typ, int _i0, int _i1, int _i2, int _i3, int _i4, int _i5);

	void	HldPrint_Tmr(int _tmr, char *str);
	void	HldPrint_Tmr(int _tmr, char *str, int _i1);

};


#endif

