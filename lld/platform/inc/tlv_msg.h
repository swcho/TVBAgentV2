
#ifndef	_TELEVIEW_TLV_MSG_H
#define	_TELEVIEW_TLV_MSG_H

#include	<pthread.h>

/////////////////////////////////////////////////////////////////////
typedef	enum
{
	_USE_QUEUE_MSG_LEN	=	32,	//	64,	//	32,
	__MAX_MSG_SIZE__		=	(64*1), //	have to larger than _USE_QUEUE_MSG_LEN
}	ENUM_MSG_Q_ATTR;

/////////////////////////////////////////////////////////////////////
typedef	enum
{
	__MSG_CORE__		=	0xc1,
	__MSG_DATA__		=	0xc2,
	__MSG_TIMER__		=	0xccd1,
	__MSG_TIMEOUT__ =	0x1012,
	__MSG_400e_MNU__	=	0x201e,
	__MSG_FNT_WR__	=	0x201f,
}	ENUM_TLV_MSG_KEY;

/////////////////////////////////////////////////////////////////////
struct tlv_msgbuf
{
	long mtype;     // message type,
	unsigned int mtext[__MAX_MSG_SIZE__/4];  // message data
};

////////////////////////////////////////////
class	CMsg
{
private:
	int		dbg_notice;
	int		dbg_warning;

	int		m_running;
	int		m_msg_id;
	int		m_msg_key;
	int		m_msg_queued;
	int		m_activated;

	int		m_msg_size;
	pthread_t	task_id;

	struct  tlv_msgbuf	r_buf;

	static	void	*EntryTimerTask(void *context);

public:
	CMsg(int msg_key, int size);
	~CMsg();

	void	ActTmMsg(int msg_type);
	int	Rcv(int msg_type, int *data, int size);
	int	RcvNoWait(int msg_type, int *data, int size);
	int	RcvNoWait2(int msg_type, int *data, int size);
	void	Snd(int msg_type, int *data, int size);
	void	MsgFlush(int m_type);
	void	*Timer(void *context);
};

#endif

