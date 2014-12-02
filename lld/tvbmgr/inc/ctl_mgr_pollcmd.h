
#ifndef	_TELEVIEW_CTL_MGR_POLL_CMD_H
#define	_TELEVIEW_CTL_MGR_POLL_CMD_H

#include	<pthread.h>
#include	"ctl_mgr_cntl.h"

class CCtlMgrPollCmd	:	public	CCtlMgrCntl
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

	int	_fb_nim_signal_valid;
	int	_fdf_id_save_inter;


//////////////////////////////////////////////////////////////////////////////////
public:
	void	RestartStateFromFirst(void);

private:
	void	QFlushAndInitState(void);
	void	_ChkHw_BoardSts(void);
	void	_ChkHw_NimSts(void);

public:
	void	_CalcSigStrength(int _lc);

private:
	void	_ApplyHw_DownloadFilter(int _what);

public:
	CCtlMgrPollCmd(void);
	~CCtlMgrPollCmd();

	void	Prt_RcvedMsg(unsigned int _key);
	void	UpdSystemStatus(unsigned int _key);

//////////////////////////////////////////////////////////////////////////////////
	void	___SEL_NIM_ID (void);
	void	POLL___CHK_NIM_SIGNAL(void);
	void	POLL___WAIT_FB_NIM_SIGNAL_STABLE(void);

	void	___CAPTURE_IQ_DATA_for_MONITOR(void);

	void	CopyCalibAndFilterFactoryDta(int _target);
	void	CopyCalibAndFilterUserDta(int _target);


protected:
};



#endif

