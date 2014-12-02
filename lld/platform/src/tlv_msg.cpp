
//
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<memory.h>
#include	<sys/stat.h>
#include	<pthread.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<time.h>
#include	<sys/time.h>
#include	<errno.h>

#include	"tlv_msg.h"

extern	int	errno;

////////////////////////////////////////////
CMsg::CMsg(int msg_key, int size)
{
	dbg_notice = dbg_warning = 0;

	m_activated = 0;
	m_msg_queued = 0;

	m_msg_size = size;
	m_msg_key = msg_key;
	m_msg_id = msgget((key_t)msg_key, IPC_CREAT | 0x1ff);
	if(m_msg_id < 0)
	{
		printf("[msg] : WARNNING : create-msgq err : d'%d\n", errno);
	}

	if (msg_key != __MSG_400e_MNU__)	//	__MSG_400e_MNU__ recv msg from RcvNoWait2() always.
	{
		if(pthread_create(&task_id, NULL, EntryTimerTask, (void *)this) != 0)
		{
			printf("[msg] : WARNNING : create-Task err : d'%d\n", errno);
		}
	}
}
CMsg::~CMsg()
{
	m_running = 0;
	msgctl(m_msg_id, IPC_RMID, NULL);
}

void	*CMsg::EntryTimerTask(void *context)
{
	CMsg	*p_msg;

	p_msg = (CMsg *)context;
	p_msg->Timer(context);

	return	NULL;
}
////////////////////////////////
void	*CMsg::Timer(void *context)
{
	int		_data[__MAX_MSG_SIZE__/4];

//	printf("[msg] >>>>>>>>>>>>>>>>>>>>> launch-timer ::: [0x%x]\n", m_msg_key);
	while(m_activated == 0)
	{
		usleep(100*1000);
	}

	m_running = 1;
	while(m_running)
	{
		usleep(100*1000);
		_data[0] = __MSG_TIMER__;
		if (m_msg_queued <= 0)
		{
			m_msg_queued++;
			Snd(m_msg_key, _data, m_msg_size);
		}
	}
	_exit(0);
}
void	CMsg::ActTmMsg(int msg_type)
{
	m_activated = 1;
}

int	CMsg::Rcv(int msg_type, int *data, int size)
{
	ssize_t	ret_val;
	int		msg_type_to_rd;

//	printf("[msg] rcv : [d'%d][0x%x:%x]\n", size, msg_type, *data);

	msg_type_to_rd = msg_type;
#if	1
	ret_val = msgrcv(m_msg_id, &r_buf, size, msg_type_to_rd, MSG_NOERROR | IPC_NOWAIT);
	if (ret_val < 0)	//	maybe, there is no msg in queue
	{
		m_msg_queued = 0;
		ret_val = msgrcv(m_msg_id, &r_buf, size, msg_type_to_rd, MSG_NOERROR);
	}
#else
	ret_val = msgrcv(m_msg_id, &r_buf, size, msg_type_to_rd, MSG_NOERROR);
#endif

	if (ret_val < 0)
	{
		printf("[msg] WARNNING ::::::::::::: rcv-err : [%d]\n", errno);
		return	__MSG_TIMEOUT__;
	}
	else if (ret_val != size)
	{
		printf("[msg] WARNNING ::::::::::::: rcv-len-err\n");
		return	__MSG_TIMEOUT__;
	}
	else	//	exactlly read success
	{
		if (r_buf.mtext[0] == __MSG_TIMER__)
		{
			m_msg_queued--;
			return	__MSG_TIMEOUT__;
		}
		memcpy((char *)data, r_buf.mtext, size);
	}
//	printf("[msg] rcv-done : [d'%d][0x%x:%x]\n", size, msg_type, *data);

	return	ret_val;
}
int	CMsg::RcvNoWait(int msg_type, int *data, int size)
{
	ssize_t	ret_val;

//	msg_type = 0;
	ret_val = msgrcv(m_msg_id, &r_buf, size, msg_type, MSG_NOERROR | IPC_NOWAIT);

	return	ret_val;
}
int	CMsg::RcvNoWait2(int msg_type, int *data, int size)
{
	ssize_t	ret_val;

	ret_val = msgrcv(m_msg_id, &r_buf, size, msg_type, MSG_NOERROR | IPC_NOWAIT);

	if (ret_val < 0)
	{
		return	__MSG_TIMEOUT__;
	}
	else if (ret_val != size)
	{
//		printf("[msg] WARNNING ::::::::::::: rcv-no-wait-len-err\n");
		return	__MSG_TIMEOUT__;
	}
	else	//	exactlly read success
	{
		if (r_buf.mtext[0] == __MSG_TIMER__)
		{
			return	__MSG_TIMEOUT__;
		}
		memcpy((char *)data, r_buf.mtext, size);
	}

	return	ret_val;
}


////////////////////////////////////////////
void	CMsg::Snd(int msg_type, int *data, int size)
{
	int	ret_val;
	struct  tlv_msgbuf	s_buf;

//	printf("[msg] snd : [d'%d][0x%x:%x]\n", size, msg_type, *data);

	s_buf.mtype = msg_type;
	memcpy(s_buf.mtext, (char *)data, size);

	ret_val = msgsnd(m_msg_id, &s_buf, size, IPC_NOWAIT);
	if (ret_val != 0)
	{
		printf("[msg] snd-msgq err : d'%d, 0x%x\n", errno, msg_type);
	}
}
void	CMsg::MsgFlush(int m_type)
{
	int	ind_flush, ret_val;

	m_type = 0;
	for (ind_flush = 0; ind_flush < 300; ind_flush++)
	{
		ret_val = RcvNoWait(m_type, (int *)&r_buf, m_msg_size);
		if (ret_val <= 0)
		{
			m_msg_queued = 0;
			break;	//	flush complete
		}
	}
	printf("[msg] flush : [0x%x:%x:%x]\n", m_msg_id, ind_flush, m_msg_queued);
}


