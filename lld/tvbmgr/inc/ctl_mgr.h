
#ifndef	_TELEVIEW_CTL_MGR_H
#define	_TELEVIEW_CTL_MGR_H

#if defined(WIN32)
#else

#include	<pthread.h>
#include	"tvb_msg.h"

//	#define	__USE_TIMER_MSG__

#define	TLV_YES		1
#define	TLV_NO		0
#define	TLV_OK		1
#define	TLV_ZERO	0
#define	TLV_ONE		1
#define	TLV_TWO		2
#define	TLV_ERR		-1

#define	_DFLT_CMD_TIMEOUT__	15000000	//	5-sec
typedef	struct	_mgr_cntl_req_sts
{
	long		_request;
	long		_cmd;
	int		_ret_val;
	double	_ret_double;
	void		*_ret_void;

	long		_arg_int1;
	long		_arg_int2;
	long		_arg_int3;
	long		_arg_int4;
	long		_arg_int5;
	long		_arg_int6;
	long		_arg_int7;
	long		_arg_int8;
	long		_arg_int9;
	double	_arg_double;
}	_MGR_CNTL_REQ_STS;

class CCtlMgr
{
private:

	int	pause_system_polling;
	_MGR_CNTL_REQ_STS	__req_sts_cnxt;
	_MGR_CNTL_REQ_STS	__req_ctl_cnxt;

////////////////////////////////////
	void	Prt_RcvedMsg (unsigned int _cmd);
	static	void	*EntryCtlManager (void *context);
	void	PollWTimeout(void);
	void	wait_until(long *_done_flag, long _timeout);
	void	InitOnManager1st (void);

public:
	CCtlMgr(void);
	~CCtlMgr();

	int	ReqMgrCntl_HwStsApi(long _cmd,
		long _arg_int1,
		long _arg_int2,
		long _arg_int3,
		long _arg_int4,
		long _arg_int5,
		long _arg_int6,
		long _arg_int7,
		long _arg_int8,
		long _arg_int9,
		long wait_msec);
	void	*ReqMgrCntl_HwStsApiPtr(long _cmd,
		long _arg_int1,
		long _arg_int2,
		long _arg_int3,
		long _arg_int4,
		long _arg_int5,
		long _arg_int6,
		long _arg_int7,
		long _arg_int8,
		long _arg_int9,
		long wait_msec);
	double	ReqMgrCntl_HwStsApiDouble(long _cmd,
		long _arg_int1,
		long _arg_int2,
		long _arg_int3,
		long _arg_int4,
		long _arg_int5,
		long _arg_int6,
		long _arg_int7,
		long _arg_int8,
		long _arg_int9,
		long wait_msec);
	void	ExecMgrCntl_HwStsApi(void);
	int	ReqMgrCntl_HwCtlApi(long _cmd,
		long _arg_int1,
		long _arg_int2,
		long _arg_int3,
		long _arg_int4,
		long _arg_int5,
		long _arg_int6,
		long _arg_int7,
		long _arg_int8,
		long _arg_int9,
		double _arg_dbl,
		long wait_msec);
	void	ExecMgrCntl_HwCtlApi(void);
	void	_tvbxxx_msg_snd(unsigned long tvb_msg_key,
			long _arg_int1,
			long _arg_int2,
			long _arg_int3,
			long _arg_int4,
			long _arg_int5,
			long _arg_int6,
			long _arg_int7,
			long _arg_int8,
			long _arg_int9,
			double _arg_dbl);
	void	MainCtlManager (void *userPara);

protected:
};
//2011/4/12
//int	_tvbxxx_create_ctl_mgr (void);
int	_tvbxxx_create_ctl_mgr (int nSlot);
int	_tvbxxx_msg_snd_get_sts(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec);
void	*_tvbxxx_msg_snd_get_sts_ptr(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec);
double	_tvbxxx_msg_snd_get_sts_double(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	long wait_msec);
int	_tvbxxx_msg_snd_set_ctl(long _key,
	long _arg_int1,
	long _arg_int2,
	long _arg_int3,
	long _arg_int4,
	long _arg_int5,
	long _arg_int6,
	long _arg_int7,
	long _arg_int8,
	long _arg_int9,
	double _arg_dbl);

#endif
#endif

