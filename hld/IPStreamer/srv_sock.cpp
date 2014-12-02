
#if defined(WIN32)
//#include	<Windows.h>
#pragma comment (lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<fcntl.h>
#include	<memory.h>
#include	"include/pthread.h"
#include	<time.h>

#include	<errno.h>

#include "tlvtype.h"
typedef int socklen_t;

#else
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<memory.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<pthread.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<time.h>
#include	<sys/time.h>
#include	<sys/shm.h>
#include	<signal.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<linux/unistd.h>
#include	<linux/sysctl.h>

#include	<errno.h>
#include "tlvtype.h"
#endif

extern	int	errno;

#include	"srv_sock.h"

#define	_USING_DATAGRAM_PORT_

static	char	*hostaddr_lo = "127.0.0.1";
static	int	backlog = 5;

////////////////////////////////////////////////////////	SERVER
CSrvSock::CSrvSock(int	*daemon_port, char	*mcast, char *_s_ip, char *_my_ip)
{
	dbg_notice = 0;
	dbg_warning = 0;

	daemon_running = 0;

	my_addr_string = _my_ip;
	unicast_addr_string = _s_ip;
	mcast_group_addr_string = mcast;
	my_port = daemon_port;
//	DBG_PRINT("[Sk] create-daemon-sock [%d]\n", *my_port);

	RecvFunc = NULL;
	data_sess = NULL;

	d_sd = -1;

}
CSrvSock::~CSrvSock()
{
	daemon_running = 0;

}
void	CSrvSock::AddDataSess(int (*recv_func)(int srv_sd, void *context), void *context)
{
	RecvFunc = recv_func;
	data_sess = context;
}
int	CSrvSock::Open(void)
{
#ifdef	_USING_DATAGRAM_PORT_
	d_sd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	d_sd = socket(AF_INET, SOCK_STREAM, 0);		// tcp socket
#endif
	if (d_sd < 0)
	{
//		DBG_PRINT_WARING( "[Sk] daemon sock open error [%d], [%s.%d]\n", errno, hostaddr_lo, *my_port);
	}

	return	d_sd;
}
int	CSrvSock::Close(void)
{
	if (d_sd < 0)
	{
		return	d_sd;
	}

#if defined(WIN32)
	closesocket(d_sd);
#else
	close(d_sd);
#endif
	d_sd = -1;

	return	d_sd;
}
int	CSrvSock::Bind(int sd, char *_addr_string)
{
	int	ret;
	struct	sockaddr_in	my_addr_in;
	socklen_t	my_addrinlen;

	memset(&my_addr_in, 0, sizeof(my_addr_in));

#ifdef	_USING_DATAGRAM_PORT_
	ret = 0;
	my_addr_in.sin_family = AF_INET;
	my_addr_in.sin_port = htons((u_int16_t)*my_port);

//	my_addr_in.sin_addr.s_addr = inet_addr(_addr_string);
	my_addr_in.sin_addr.s_addr = INADDR_ANY;
	my_addrinlen = sizeof(my_addr_in);

	ret = bind(sd, (struct sockaddr *)&my_addr_in, my_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] bind error [%d]\n", errno);
	}

#else
	ret = 0;
	my_addr_in.sin_family = AF_INET;
	my_addr_in.sin_port = htons((u_int16_t)*my_port);

//	my_addr_in.sin_addr.s_addr = inet_addr(_addr_string);
	my_addr_in.sin_addr.s_addr = INADDR_ANY;
	my_addrinlen = sizeof(my_addr_in);

	ret = bind(sd, (struct sockaddr *)&my_addr_in, my_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] bind error [%d]\n", errno);
	}
#endif

	return	ret;
}
void	CSrvSock::Listen(int sd, int blog)
{

#ifdef	_USING_DATAGRAM_PORT_
#else
	int	ret;
	ret = listen(sd, blog);
	if(ret)
	{
		DBG_PRINT_WARING( "[Sk] listen error [%d]\n", errno);
	}
#endif
}
int	CSrvSock::UdpWaiting(int sd)
{
	int	ret;

	if ((data_sess == NULL) || (RecvFunc == NULL))
	{
		return	TLV_NO;
	}

	ret = RecvFunc(sd, data_sess);
	if (data_sess == NULL)
	{
		ret = TLV_NO;	//	session disconnected
	}
	else
	{
		ret = TLV_YES;	//	disconnected by network peoblem
	}

	return	ret;
}
int	CSrvSock::Accept(int sd)
{
	pthread_t		task_id;
	int	srv_sd;
	struct	sockaddr	peer_addr;
	socklen_t	peer_addrlen;
	CSrvSock	*m_data_service;

	peer_addrlen = sizeof(peer_addr);

	srv_sd = -1;
	srv_sd = accept(sd, &peer_addr, &peer_addrlen);

	if (srv_sd == -1)
	{
//		DBG_PRINT_WARING( "[Sk] sock accept error [%d]\n", errno);
	}

	m_data_service = new CSrvSock(my_port, NULL, NULL, NULL);
	m_data_service->AddDataSess(RecvFunc, this);
	m_data_service->new_sd = srv_sd;
	if(pthread_create(&task_id, NULL, EntryRecvFuncSrv, (void *)m_data_service) != 0)
	{
//		DBG_PRINT_WARING( "[Sk] FATAL : CreateTask : d'%d\n", errno);
	}

	return	srv_sd;
}
void	CSrvSock::SetSrvOpt(int sd, char *multi_group_addr, char *_my_addr)
{
	int	i_val, ret;

	i_val = 1;
#if defined(WIN32)
	ret = setsockopt(d_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&i_val, sizeof(i_val));
#else
	struct	ip_mreq	multi_membership;
	setsockopt(d_sd, SOL_SOCKET, SO_REUSEADDR, (void *)&i_val, sizeof(i_val));
#endif

#if defined(WIN32)
	//setsockopt(...SO_RCVBUF...) should be called after calling bind
#else
	i_val = 0x40000;
	setsockopt(d_sd, SOL_SOCKET, SO_RCVBUF, (void *)&i_val, sizeof(i_val));
#endif

#if defined(WIN32)
	//setsockopt(...IP_ADD_MEMBERSHIP...) should be called after calling bind
#else
	if ((multi_group_addr != NULL) && (multi_group_addr[0] != 0))
	{
		printf( "[Sk] ip-add-membership into multicast group : [%s]\n", multi_group_addr);

		memset(&multi_membership, 0, sizeof(struct ip_mreq));
		multi_membership.imr_multiaddr.s_addr = inet_addr(multi_group_addr);
		multi_membership.imr_interface.s_addr = inet_addr(_my_addr);	//	INADDR_ANY;
		setsockopt(d_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multi_membership, sizeof(multi_membership));
	}
#endif
}
void	CSrvSock::SetSrvOpt_2(int sd, char *multi_group_addr, char *_my_addr)
{
#if defined(WIN32)
	int i_val, ret;
	struct	ip_mreq multi_membership;

	i_val = 0x100000;
	ret = setsockopt( d_sd, SOL_SOCKET, SO_RCVBUF, (char *)&i_val, sizeof( i_val ) );

	if ((mcast_group_addr_string != NULL) && (mcast_group_addr_string[0] != 0))
	{
		//printf( "[Sk] ip-add-membership into multicast group : [%s]\n", mcast_group_addr_string);

		memset(&multi_membership, 0, sizeof(struct ip_mreq));
		multi_membership.imr_multiaddr.s_addr = inet_addr(mcast_group_addr_string);
		multi_membership.imr_interface.s_addr = inet_addr(my_addr_string);	//	INADDR_ANY;
		i_val = setsockopt(d_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multi_membership, sizeof(multi_membership));
		if ( i_val == SOCKET_ERROR )
		{
//			DBG_PRINT_WARING( "[Sk] ip-add-membership into multicast group=%d\n", i_val);
		}
	}
#endif
}
char	*CSrvSock::GetLocalName(int c_sd)
{
	int	ret;
	char	*local_string;
	struct	sockaddr_in	local_addr_in;
	socklen_t	local_addrinlen;

	local_string = NULL;
	local_addrinlen = sizeof(local_addr_in);

	ret = getsockname(c_sd, (struct sockaddr*)&local_addr_in, &local_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] get localname error [%d]\n", errno);
	}
	else
	{
		local_string = inet_ntoa(local_addr_in.sin_addr);
//		DBG_PRINT("[Sk] local-name : [%s][0x%x]\n", local_string, local_addr_in.sin_addr);
	}
  
	return local_string;
}
unsigned int	CSrvSock::GetLocalPort(int c_sd)
{
	int	ret;
	unsigned int	local_addr;
	struct	sockaddr_in	local_addr_in;
	socklen_t	local_addrinlen;

	local_addr = 0;
	local_addrinlen = sizeof(local_addr_in);

	ret = getsockname(c_sd, (struct sockaddr*)&local_addr_in, &local_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] get localname error [%d]\n", errno);
	}
	else
	{
		local_addr = ntohs(local_addr_in.sin_port);
	}
  
	return local_addr;
}
char	*CSrvSock::GetPeerName(int c_sd)	//	sd of client port connection-established.
{
	int	ret;
	char	*peer_string;
	struct	sockaddr_in	peer_addr_in;
	socklen_t	peer_addrinlen;

	peer_string = NULL;
	peer_addrinlen = sizeof(peer_addr_in);

	ret = getpeername(c_sd, (struct sockaddr*)&peer_addr_in, &peer_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] get peername error [%d]\n", errno);
	}
	else
	{
		peer_string = inet_ntoa(peer_addr_in.sin_addr);
//		DBG_PRINT("[Sk] peer-name : [%s][0x%x]\n", peer_string, peer_addr_in.sin_addr);
	}
  
	return peer_string;
}
unsigned int	CSrvSock::GetPeerPort(int c_sd)	//	sd of client port connection-established.
{
	int	ret;
	unsigned int	peer_addr;
	struct	sockaddr_in	peer_addr_in;
	socklen_t	peer_addrinlen;

	peer_addr = 0;
	peer_addrinlen = sizeof(peer_addr_in);

	ret = getpeername(c_sd, (struct sockaddr*)&peer_addr_in, &peer_addrinlen);
	if(ret)
	{
//		DBG_PRINT_WARING( "[Sk] get peername error [%d]\n", errno);
	}
	else
	{
		peer_addr = ntohs(peer_addr_in.sin_port);
	}
  
	return peer_addr;
}
unsigned long	CSrvSock::PeerAddr(void)
{
	if ((unicast_addr_string != 0) && (unicast_addr_string[0] != 0))
	{
		return	inet_addr(unicast_addr_string);
	}
	return	INADDR_ANY;
}
u_int16_t	CSrvSock::MyPort(void)
{
	return	(u_int16_t)*my_port;
}
//////////////////////////////////////////////////////////////////
void	CSrvSock::RunDaemon(void)
{
	pthread_t	task_id;
	int _policy;
	struct sched_param _sche_param;

//	DBG_PRINT("[Sk] sock-run-daemon [%d]\n", *my_port);
	if (daemon_running != 0)
	{
		return;
	}

	if(pthread_create(&task_id, NULL, EntryDaemon, (void *)this) != 0)
	{
//		DBG_PRINT_WARING("[Sk] FATAL : CreateTask : d'%d\n", errno);
	}
	if(pthread_getschedparam(task_id, &_policy, &_sche_param) == 0)
	{
		_policy = SCHED_FIFO;
		_sche_param.sched_priority += 1;	//	10;
//		DBG_PRINT_WARING( "Priority : d'%d\n", _sche_param.sched_priority);
		pthread_setschedparam(task_id, _policy, &_sche_param);
	}
}
/////////////////////////////////////////////////
void	*CSrvSock::EntryDaemon(void *context)
{
	CSrvSock	*c_sock;

	c_sock = (CSrvSock *)context;
	c_sock->Daemon(context);

	return	NULL;
}
void	*CSrvSock::Daemon(void *context)
{
	int	is_sess_active;

#if defined(WIN32)
	WSADATA wsaData;
	int nErrorStatus;
	nErrorStatus = WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

_open_again:
	while(1)
	{
#if defined(WIN32)
		Sleep(1000);
#else
		usleep(1000000);
#endif
		if (data_sess != NULL)	//	request session.
		{
			break;
		}
	}

	d_sd = Open();
	if (d_sd < 0)
	{
		goto	_open_again;
	}
	SetSrvOpt(d_sd, mcast_group_addr_string, my_addr_string);

_bind_again:
	if (Bind(d_sd, my_addr_string) == -1)
	{
#if defined(WIN32)
		Sleep(1000);
#else
		usleep(1000000);
#endif
		goto	_bind_again;
	}
	SetSrvOpt_2(d_sd, mcast_group_addr_string, my_addr_string);

	Listen(d_sd, backlog);

	daemon_running = 1;
	while(daemon_running)
	{
#ifdef	_USING_DATAGRAM_PORT_
		is_sess_active = UdpWaiting(d_sd);
		Close();
#if defined(WIN32)
		Sleep(1000);
#else
		usleep(1000000);
#endif

		goto	_open_again;
//		goto	_bind_again;
#else
		Accept(d_sd);
#endif
	}
	Close();

#if defined(WIN32)
	WSACleanup();
#endif

	return	0;
}

void	*CSrvSock::EntryRecvFuncSrv(void *context)
{
	CSrvSock	*c_sock;

	c_sock = (CSrvSock *)context;
	c_sock->RecvFuncSrv(c_sock->new_sd);

	return	NULL;
}
void	CSrvSock::RecvFuncSrv(int srv_sd)
{
#ifdef	_USING_DATAGRAM_PORT_
#else
//	fcntl(srv_sd, F_SETFL, O_NONBLOCK);
//	DBG_PRINT("[Sk] server established data-con [%d]\n", srv_sd);
	while(1)
	{
		usleep(1000000);
		printf( "[Sk] rcv-???? [%d]\n", srv_sd);
/*
		rcv_len = recv(srv_sd, rd_buf, sizeof(_API_PACKET), MSG_NOSIGNAL);
		if (rcv_len == -1)
		{
			if(errno == EAGAIN)
			{
				usleep(1000000);
				DBG_PRINT_WARING( "[Sk] rcv-????\n");
			}
			else
			{
				goto	_exit_point;
			}
		}
		else if (rcv_len == 0)	//	disconnected
		{
			goto	_exit_point;
		}
*/
	}
#endif
}




