
#ifndef __HLD_MULTI_RFOUT_H__
#define __HLD_MULTI_RFOUT_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"

#define MAX_MULTI_STREAM_CNT 3//4
#define MAX_MULTI_STREAM_BUFFER_SIZE 0x200000

class CHldMultiRfOut
{
private:
	int	my_hld_id;
	void	*my_hld;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];
#if defined(WIN32)
	HANDLE	_dbg_file;
#endif

#if defined(WIN32)
	HANDLE	_multi_ts_mutex;
#else
	pthread_mutex_t	_multi_ts_mutex;
	pthread_mutexattr_t	_multi_ts_mutex_attr;
#endif

public:
	void	*MultiTsMutex(void);
	void	CreateMultiTsMutexForReal(void);
	void	DupMultiTsMutexForVirtual(void *_mutx);
	void	DestroyMultiTsMutex(void);

private:
	void	LockMultiTsMutex(void);
	void	UnlockMultiTsMutex(void);

public:
	CHldMultiRfOut(int _my_id, void *_hld);
	virtual ~CHldMultiRfOut();

	void	SetCommonMethod_9(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);

///////////////////////////////////////////////////////////////////////
	int 	MultiStreamStartMon_594(void);
	void	MultiStreamStartRec_594(void);
	int		MultiStreamContRec_594(void);
	void	MultiStreamStartPlay_594(void);
	int		MultiStreamCpDmaBuf_594(char *_dst, char *_src, int _size);
	int		MultiStreamContPlay_594(void);
	void	MultiStreamSelTs_ofAsi310Out_594(int _ts_n);

	int 	MultiStreamStartMon_593(void);
	void	MultiStreamStartRec_593(void);
	int		MultiStreamContRec_593(void);
	void	MultiStreamStartPlay_593(void);
	int		MultiStreamCpDmaBuf_593(char *_dst, char *_src, int _size);
	int		MultiStreamContPlay_593(void);
	void	MultiStreamSelTs_ofAsi310Out_593(int _ts_n);

	int		MultiStreamStartMon(void);
	void	MultiStreamStartRec(void);
	int		MultiStreamContRec(void);
	void	MultiStreamStartPlay(void);
	int		MultiStreamCpDmaBuf(char *_dst, char *_src, int _size);
	int		MultiStreamContPlay(void);
	void	MultiStreamSelTs_ofAsi310Out(int _ts_n);


};


#endif

