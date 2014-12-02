
#ifndef	TELEVIEW_SRV_RCVER_H
#define	TELEVIEW_SRV_RCVER_H

#if defined(WIN32)
//#include <winsock2.h>
#include	<Windows.h>

#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	"include/pthread.h"
#include	"include/semaphore.h"
#include	<errno.h>

typedef int socklen_t;
#else
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<semaphore.h>
#include	<errno.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<linux/unistd.h>
#include	<linux/sysctl.h>
#include	<linux/if.h>
#endif

#include	"srv_sock.h"
#include	"srv_rcver_utils.h"


///////////////////////////////////////////
class	CSrvRcver	:	public	CSrvSock,	public	CSrvRcverUtils
{
private:
	int	dbg_notice;
	int	dbg_warning;

	struct	sockaddr_in	peer_addr_in;
	socklen_t	peer_addrinlen;
	char		*peer_addr_string;
	int		if_monitor_interval;
	int		rcv_tot;

	static	int	TsRecvFuncEntry(int srv_sd, void *context);
	void	PreProcessUdpRcvSts(int rcv_len);
	void	SMyAddr(int srv_sd);
	int		IfMonitor(void);
	int		ProcessUdpRcvRslt(int rcv_len);
	int		InitRecvLvl1(void);
	void	TermRecvLvl1(void);
	void	TermRecvLvl2(void);
	int		TsRecvViaUdp(int srv_sd);
	int		TsReadUdpPool(char *buf, int max_size, unsigned int buf_seek);
	unsigned int	ByteSizeInBufUdpPool(void);

public:
	CSrvRcver(int *udp_port, char *mcast_addr, char *src_addr, char *my_addr);
	~CSrvRcver();

	int	TsRecvFunc(int srv_sd);
	int	TsRead(char *buf, int max_size, unsigned int buf_seek);
	void	ConnectReq(void);
	void	DisconnectReq(void);
	int	IsConnectionReqed(void);
	int	IsConnectionConnected(void);
	unsigned int	ByteSizeInBuf(void);
	void	FlushBuf(int pkt_cnt);
	int	IsTsLocked(void);
	void	tst_freader(char *buf, int size);
	void	tst_writer(char *buf, int size);

};

class	CIpApi
{
private:
	int	dbg_notice;
	int	dbg_warning;

////////////////////////////////////////////////////////
	CSrvRcver	*m_Rcver;

	char	my_ip_dot_notation_buf[32];
	char	mcast_dot_notation_buf[32];
	char	_req_src_dot_notation_buf[32];
	unsigned long	__req_mcast_ip__;
	unsigned long	__req_src_ip__;
	int				__udp_port__;
	int				__udp_port_fec_col_offset__;
	int				__fec_is_inactivate__;

public:
	CIpApi(void);
	~CIpApi();

	void close_ip_streaming_srv(void);
	int run_ip_streaming_srv(
		unsigned long	my_ip,
		unsigned long	src_ip,
		unsigned long	mcast_ip,
		int udp_port,
		int fec_udp_offset,
		int fec_inact);
	int is_ip_streaming_srv(void);
	int get_ip_streaming_buf(char *_buf, unsigned int max_size, unsigned int buf_seek);
	char* get_src_ip_string(void);
	unsigned int bytes_ip_recv_buffer(void);
	unsigned int bitrate_ip_recv_buffer(void);
	unsigned int bitrate188_ip_recv_buffer(void);
	void flush_ip_recv_buffer(int pkt_cnt);
	int is_ip_ts_locked(void);
	unsigned int get_eth_pkt_lost_cnt(void);

	//2014/01/09 IP Sampling time
	void SetDelayTime_Millisecond(int milliSec);
};

#endif

