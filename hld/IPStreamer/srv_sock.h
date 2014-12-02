
#ifndef	TELEVIEW_SRV_SOCK_H
#define	TELEVIEW_SRV_SOCK_H

#if defined(WIN32)
typedef unsigned short u_int16_t;
#endif

class	CSrvSock
{
private:
	int	dbg_notice;
	int	dbg_warning;

	int	daemon_running;
	int	d_sd;
	int	new_sd;

	char	*my_addr_string;
	char	*unicast_addr_string;
	char	*mcast_group_addr_string;
	int		*my_port;

	int	(*RecvFunc)(int srv_sd, void *context);
	void	*data_sess;

	int	Open(void);
	int	Close(void);
	int	Bind(int sd, char *_addr_string);
	void	Listen(int sd, int blog);
	int	UdpWaiting(int sd);
	int	Accept(int sd);
	char	*GetLocalName(int c_sd);
	unsigned int	GetLocalPort(int c_sd);
	char	*GetPeerName(int c_sd);	//	sd of client port connection-established.
	unsigned int	GetPeerPort(int c_sd);	//	sd of client port connection-established.
	static	void	*EntryDaemon(void *context);
	static	void	*EntryRecvFuncSrv(void *context);


public:
	CSrvSock(int	*daemon_port, char	*mcast, char *_s_ip, char *_my_ip);
	~CSrvSock();

	void	AddDataSess(int (*recv_func)(int srv_sd, void *context), void *context);

	void	RunDaemon(void);
	void	*Daemon(void *context);
	void	RecvFuncSrv(int srv_sd);
	unsigned long	PeerAddr(void);
	u_int16_t	MyPort(void);
	void	SetSrvOpt(int sd, char *multi_group_addr, char *_my_addr);
	void	SetSrvOpt_2(int sd, char *multi_group_addr, char *_my_addr);

};

#endif
