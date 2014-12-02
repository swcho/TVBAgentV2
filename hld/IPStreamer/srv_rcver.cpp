
#define	_FILE_OFFSET_BITS	64
#define	_LARGEFILE_SOURCE

#if defined(WIN32)
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

#include	"tlvtype.h"
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
#include	<sys/ioctl.h>
#include	<signal.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<linux/unistd.h>
#include	<linux/sysctl.h>
#include	<linux/if.h>

#include	<errno.h>
#include	"tlvtype.h"
#endif

extern	int	errno;

#include	"srv_rcver.h"

static	char	peer_dot_notation_buf[32]	=	{0};

////////////////////////////////////////////////////////
static	inline	void	__memcpy(char *dest, char *src, int size)
{
	int	_loop, _loop_size;
	unsigned int	*_src, *_dest;

	if (((unsigned long)dest & 0x3) || ((unsigned long)src & 0x3))
	{
		memcpy(dest, src, size);
		return;
	}

	_src = (unsigned int *)src;
	_dest = (unsigned int *)dest;
	_loop_size = (size & ~0x3);
	for (_loop = 0; _loop < _loop_size; _loop += 4)
	{
		*_dest++ = *_src++;
	}
	if (_loop < size)
	{
		src = (char *)_src;
		dest = (char *)_dest;
		_loop_size = size - _loop;
		for (_loop = 0; _loop < _loop_size; _loop++)
		{
			*dest++ = *src++;
		}
	}
}

////////////////////////////////////////////////////////	SERVER
CSrvRcver::CSrvRcver(int *udp_port, char *mcast_addr, char *src_addr, char *my_addr)
	:	CSrvSock(udp_port, mcast_addr, src_addr, my_addr)
{
	dbg_notice = 0;
	dbg_warning = 0;

	memset(peer_dot_notation_buf, 0, sizeof(peer_dot_notation_buf));
	if_monitor_interval = 0;
	rcv_tot = 0;

//
	AddDataSess(TsRecvFuncEntry, this);
	RunDaemon();
}
CSrvRcver::~CSrvRcver()
{
}

//////////////////////////////////////////////////////////////////
int	CSrvRcver::TsRecvFuncEntry(int srv_sd, void *context)
{
	CSrvRcver	*_rcver;

	_rcver = (CSrvRcver *)context;
	return	_rcver->TsRecvFunc(srv_sd);
}
//////////////////////////////////////////////////////////////////////////////
void	CSrvRcver::PreProcessUdpRcvSts(int rcv_len)
{
	rcv_tot += rcv_len;
	if ((rcv_tot/ByteRateFullTs()) >= NETWORK_CHCK__SEC)
	{
		peer_addr_string = inet_ntoa(peer_addr_in.sin_addr);
		if (peer_addr_string != NULL)
		{
			strcpy(peer_dot_notation_buf, peer_addr_string);
//			DBG_PRINT( "[ip-ts] rcv :::::: [%s]..[%d]\n", peer_addr_string, rcv_len);
		}
		rcv_tot = 0;
	}
}
//////////////////////////////////////////////////////////////////////////////
void	CSrvRcver::SMyAddr(int srv_sd)
{
	int	i_val, ret;

#if defined(WIN32)
	i_val = 1;
	ret = ioctlsocket(srv_sd, FIONBIO, (unsigned long*)&i_val);	//	non-blocking
#else
	i_val = fcntl(srv_sd, F_GETFL, 0);
	fcntl(srv_sd, F_SETFL, ((i_val != -1) ? i_val : 0) | O_NONBLOCK);
#endif

	peer_addr_string = NULL;
	memset(&peer_addr_in, 0, sizeof(peer_addr_in));

	peer_addr_in.sin_family = AF_INET;
	peer_addr_in.sin_addr.s_addr = PeerAddr();
	peer_addr_in.sin_port = htons(MyPort());

	peer_addrinlen = sizeof(peer_addr_in);

}
//////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::IfMonitor(void)
{
	static	int		if_prt_interval = 0;

	if_monitor_interval++;
	if (if_monitor_interval > 10000)
	{
		if_monitor_interval = 0;
#if defined(WIN32)
		if (0)
#else
		//if (_is_pri_netdev_up() == TLV_NO)
		if (0)
#endif
		{
			return	TLV_ERR;
		}
		else
		{
			if (if_prt_interval++ > 10)
			{
				if_prt_interval = 0;
//				DBG_PRINT("[ip-ts] rcv : [%s] ... if-UP\n", _pri_netdev_string());
			}
		}
	}

	return	TLV_OK;
}
//////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::InitRecvLvl1(void)
{
	AllocRcvBuf();
	connection_connected = TLV_YES;
	if (PreparedRcvBuf() == TLV_NO)
	{
		return	TLV_ERR;
	}

	rcv_tot = 0;
	cnt_rd_buf_consume = 0;

	FlushBuf(TLV_ZERO);
	InitInputPara();

	return	TLV_OK;
}
void	CSrvRcver::TermRecvLvl1(void)
{
	is_ts_locked = TLV_NO;
	AddDataSess(TsRecvFuncEntry, NULL);
	connection_connected = TLV_NO;
	FreeRcvBuf();
}
void	CSrvRcver::TermRecvLvl2(void)
{
	is_ts_locked = TLV_NO;
	AddDataSess(TsRecvFuncEntry, this);
	connection_connected = TLV_NO;
	FreeRcvBuf();
}
//////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::ProcessUdpRcvRslt(int rcv_len)
{
	static	int	ts_lock_monitor = 0;

	if (rcv_len == -1)
	{
		cnt_rd_buf_consume = 0;
		if(errno == EAGAIN)	//	no data in udp buf.
		{
#if defined(WIN32)
			Sleep(10);
#else
			usleep(10*1000);
#endif
			ts_lock_monitor += 10;
			if (ts_lock_monitor > PKT_RECV_TIMEOUT__TS_UNLOCK)
			{
				printf("[ip-ts] :::::::::::: unlocked ip-ts\n");
				ts_lock_monitor = 0;
				FlushBuf(TLV_ZERO);
				InitInputPara();
			}
			return	TLV_TWO;
		}
		else
		{
			peer_addr_string = inet_ntoa(peer_addr_in.sin_addr);
			if (peer_addr_string != NULL)
			{
//				DBG_PRINT_WARING( "[ip-ts] rcv-???? : [%s]..[%d]..[%d]\n", peer_addr_string, errno, _wr_i__);
			}
			else
			{
//				DBG_PRINT_WARING( "[Sk] rcv-???? : [%d]..[%d]\n", errno, _wr_i__);
			}
			return	TLV_ERR;
		}
	}
	else if (rcv_len == 0)
	{
//		DBG_PRINT("[ip-ts] rcv-buffer-error : [%d]\n", errno);
		return	TLV_ERR;
	}
	ts_lock_monitor = 0;

	return	TLV_OK;
}
////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::TsRecvViaUdp(int srv_sd)
{
	int	rcv_len, ret;

	rcv_len = recvfrom(
				srv_sd,
				pkt_pool[_wr_i__],
				mtu_size_pkt,
				0,
				(struct sockaddr *)&peer_addr_in,
				&peer_addrinlen);	//	need to check in-addr.

#if defined(WIN32)
	if ( rcv_len == -1 )
	{
		errno = EAGAIN;
	}
#endif

	if (IfMonitor() == TLV_ERR)
	{
		return	TLV_ERR;
	}

	ret = ProcessUdpRcvRslt(rcv_len);
	switch(ret)
	{
	case	TLV_TWO:
		return	TLV_TWO;
	case	TLV_ERR:
		return	TLV_ERR;

	case	TLV_OK:
	default:
		break;
	}

	is_ts_locked = TLV_YES;

	if (cnt_rd_buf_consume > 0)
	{
		cnt_rd_buf_consume--;
		return	TLV_TWO;
	}

//////////////////////////////////////////////////////////////////////	process data
	Mutex_Lock();
	PrePeocessRtp(_wr_i__, rcv_len);
	PreProcessUdpRcvSts(rcv_len);
	Mutex_Unlock();

	return	TLV_OK;
}
int	CSrvRcver::TsRecvFunc(int srv_sd)
{
	int	ret;

	ret = InitRecvLvl1();
	if (ret == TLV_ERR)
	{
		goto	_exit_point_2;
	}
//	DBG_PRINT("[ip-ts] ::: launch rtp recv session\n");

	SMyAddr(srv_sd);

	while(1)
	{
_recv_again:
		if (connection_requested == TLV_NO)
		{
			break;
		}

		ret = TsRecvViaUdp(srv_sd);
		switch(ret)
		{
		case	TLV_TWO:
#if defined(WIN32)
			if ( cnt_rd_buf_consume <= 0 )
			{
				Sleep(1);
			}
#endif
			goto	_recv_again;
		case	TLV_ERR:
			goto	_exit_point;

		case	TLV_OK:
		default:
			break;
		}
	}

	TermRecvLvl1();
	return	TLV_OK;

_exit_point:
	TermRecvLvl2();
	return	TLV_ERR;

_exit_point_2:
	TermRecvLvl1();
	return	TLV_ERR;
}
//////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::TsReadUdpPool(char *buf, int max_size, unsigned int buf_seek)
{
	static	int	under_prt_inter = 0, wl_prt_inter = 0;
	int	copyed_size, real_size;
	char	*src_pkt;

	if (_diff_node_(_rd_i__,_wr_i__) > (cnt_rcv_b_pool*70/100))
	{
//		DBG_PRINT("[ip-ts] wl : [%d/%d]\n", _diff_node_(_rd_i__,_wr_i__), cnt_rcv_b_pool);
	}
	if (wl_prt_inter++ > 3000)
	{
/*
		DBG_PRINT("[ip-ts] --- rcv-buf-water-level : [%d/%d]..[%u]\n",
			_diff_node_(_rd_i__,_wr_i__),
			cnt_rcv_b_pool,
			(unsigned int)_diff_node_(_rd_i__,_wr_i__)*one_pkt_r_size);
*/
		wl_prt_inter = 0;
	}

	Mutex_Lock();
	copyed_size = buf_seek;
//	DBG_PRINT("[ip-ts] v- : [%d/%d]\n", _diff_node_(_rd_i__,_wr_i__), cnt_rcv_b_pool);
	while(1)
	{
		if (_wr_i__ == _rd_i__)
		{
			under_prt_inter++;
			if (under_prt_inter > 1000)
			{
//				DBG_PRINT("[ip-ts] rcv-buffer-underflow : [%d]\n", _rd_i__);
				under_prt_inter = 0;
			}
			break;
		}

		real_size = DecapRtp(&src_pkt, max_size - copyed_size);
		if (real_size > 0)
		{
//			memcpy(&buf[copyed_size], src_pkt, real_size);
			__memcpy(&buf[copyed_size], src_pkt, real_size);
			copyed_size += real_size;
		}
		else
		{
			//	discard
		}

		if (copyed_size >= max_size)	//	fill complete
		{
//			DBG_PRINT( "[ip-ts] read [%d][%d][%d][%d]\n", max_size, buf_seek,copyed_size, _rd_i__);
			break;
		}
	}
	Mutex_Unlock();

	if (copyed_size >= max_size)
	{
		CalcTsRateByContent(buf, copyed_size);
	}

	return	copyed_size;
}
//////////////////////////////////////////////////////////////////////////////
int	CSrvRcver::TsRead(char *buf, int max_size, unsigned int buf_seek)
{
	int	copyed_size;

	if (connection_connected != TLV_YES)
	{
		return	0;
	}

	copyed_size = TsReadUdpPool(buf, max_size, buf_seek);
	return	copyed_size;
}
//////////////////////////////////////////////////////////////////////////////
void	CSrvRcver::ConnectReq(void)
{
	connection_requested = TLV_YES;
	AddDataSess(TsRecvFuncEntry, this);
}
void	CSrvRcver::DisconnectReq(void)
{
	connection_requested = TLV_NO;
	memset(peer_dot_notation_buf, 0, sizeof(peer_dot_notation_buf));
}
int	CSrvRcver::IsConnectionReqed(void)
{
	return	connection_requested;
}
int	CSrvRcver::IsConnectionConnected(void)
{
	return	connection_connected;
}
unsigned int	CSrvRcver::ByteSizeInBufUdpPool(void)
{
	static	int	bsize_prt = 0;

	if (bsize_prt++ > 5000)
	{
		bsize_prt = 0;
//		DBG_PRINT( "[ip-ts] byte-size-in-ip-rcv-buf : [%d].[%u]\n", _diff_node_(_rd_i__,_wr_i__), one_pkt_r_size/188);
	}
	return	((unsigned int)_diff_node_(_rd_i__,_wr_i__)*one_pkt_r_size);
}
unsigned int	CSrvRcver::ByteSizeInBuf(void)
{
	return	ByteSizeInBufUdpPool();
}
void	CSrvRcver::FlushBuf(int pkt_cnt)
{
	FlushBufUdpPool(pkt_cnt);
}
int	CSrvRcver::IsTsLocked(void)
{
	return	is_ts_locked;
}
void	CSrvRcver::tst_freader(char *buf, int size)
{
	static	FILE	*f_str = NULL;

	if (f_str == NULL)
	{
		f_str = fopen("/mnt/TL_4M.trp", "r");
	}
	if (f_str != NULL)
	{
		if (fread(buf, 1, size, f_str) != size)
		{
			fclose(f_str);
			f_str = NULL;
		}
	}
}
void	CSrvRcver::tst_writer(char *buf, int size)
{
	static	FILE	*f_str = NULL;

	if (f_str == NULL)
	{
		f_str = fopen("/mnt/tlv400s-ip-rcv.trp", "w+");
	}
	if (f_str != NULL)
	{
		if (fwrite(buf, 1, size, f_str) != size)
		{
			fclose(f_str);
			f_str = NULL;
		}
		else
		{
#if defined(WIN32)
#else
			fdatasync(fileno(f_str));
#endif
		}
	}
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CIpApi::CIpApi(void)
{
	dbg_notice = 0;
	dbg_warning = 0;

	m_Rcver	= NULL;
	__req_mcast_ip__ = 0;
	__req_src_ip__ = 0;
	__udp_port__ = 1234;
	__udp_port_fec_col_offset__ = 2;
	__fec_is_inactivate__ = TLV_ZERO;

}
CIpApi::~CIpApi()
{
}
void CIpApi::close_ip_streaming_srv(void)
{
	if (m_Rcver != NULL)
	{
		m_Rcver->DisconnectReq();
		while(1)
		{
			if (m_Rcver->IsConnectionConnected() == TLV_NO)
			{
				break;
			}
#if defined(WIN32)
			Sleep(100);
#else
			usleep(100000);
#endif
		}
	}
}

int CIpApi::run_ip_streaming_srv(
	unsigned long	my_ip,
	unsigned long	src_ip,
	unsigned long	mcast_ip,
	int	udp_port,
	int	fec_udp_offset,
	int	fec_inact)
{
	struct in_addr	in_addr;
	char	*ip_dot;
	int		conf_upded;

//////////////////////////////////////////////////////////////////////////
	conf_upded = TLV_NO;
	if ((__req_mcast_ip__ != mcast_ip) ||
		(__req_src_ip__ != src_ip) ||
		(__udp_port__ != udp_port) ||
		(__udp_port_fec_col_offset__ != fec_udp_offset) ||
		(__fec_is_inactivate__ != fec_inact))
	{
		conf_upded = TLV_YES;
	}
	__req_mcast_ip__ = mcast_ip;
	__req_src_ip__ = src_ip;
	__udp_port__ = udp_port;
	__udp_port_fec_col_offset__ = fec_udp_offset;
	__fec_is_inactivate__ = fec_inact;

//////////////////////////////////////////////////////////////////////////
	if (m_Rcver != NULL)
	{
		if (m_Rcver->IsConnectionReqed())	//	in progress.
		{
			if (conf_upded == TLV_YES)
			{
				close_ip_streaming_srv();
				goto	_reconnect;
			}
			return 0;
		}
	}

//////////////////////////////////////////////////////////////////////////
_reconnect:
	if (__req_mcast_ip__ == 0)	//	mcast not specified, we are not mcast domain.
	{
		memset(mcast_dot_notation_buf, 0, sizeof(mcast_dot_notation_buf));
	}
	else	//	in case of mcast service.
	{
		__req_src_ip__ = 0;

		in_addr.s_addr = __req_mcast_ip__;
		ip_dot = inet_ntoa(in_addr);
		strcpy(mcast_dot_notation_buf, ip_dot);
	}

//////////////////////////////////////////////////////////////////////////
	in_addr.s_addr = my_ip;
	ip_dot = inet_ntoa(in_addr);
	strcpy(my_ip_dot_notation_buf, ip_dot);

//////////////////////////////////////////////////////////////////////////
	if (__req_src_ip__ == 0)	//	src-ip not specified, we are running in inaddr-any.
	{
		memset(_req_src_dot_notation_buf, 0, sizeof(_req_src_dot_notation_buf));
	}
	else	//	unicast
	{
		in_addr.s_addr = __req_src_ip__;
		ip_dot = inet_ntoa(in_addr);
		strcpy(_req_src_dot_notation_buf, ip_dot);
	}

//////////////////////////////////////////////////////////////////////////
	if (m_Rcver == NULL)
	{
		m_Rcver	=	new	CSrvRcver(
								&__udp_port__,
								mcast_dot_notation_buf,
								_req_src_dot_notation_buf,
								my_ip_dot_notation_buf);
	}
	else
	{
		m_Rcver->ConnectReq();
	}

	return	1;
}

int CIpApi::is_ip_streaming_srv(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->IsConnectionReqed();
	}
	return TLV_NO;
}

int CIpApi::get_ip_streaming_buf(char *_buf, unsigned int max_size, unsigned int buf_seek)
{
	if (m_Rcver != NULL)
	{
		if (buf_seek >= max_size)
		{
			return	max_size;
		}
		return	m_Rcver->TsRead(_buf, max_size, buf_seek);
	}
	return	TLV_ZERO;
}


char* CIpApi::get_src_ip_string(void)
{
	return	peer_dot_notation_buf;
}

unsigned int CIpApi::bytes_ip_recv_buffer(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->ByteSizeInBuf();
	}
	return	TLV_ZERO;
}
unsigned int CIpApi::bitrate_ip_recv_buffer(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->BitRateFullTs();
	}
	return	(REF_IP_INPUT_BYTE_RATE*8);
}

//2014/01/09 IP delayTime
void CIpApi::SetDelayTime_Millisecond(int milliSec)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->setIP_delayTime(milliSec);
	}
}
unsigned int CIpApi::bitrate188_ip_recv_buffer(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->BitRatePartialTs();
	}
	return	TLV_ZERO;
}

void CIpApi::flush_ip_recv_buffer(int pkt_cnt)
{
	if (m_Rcver != NULL)
	{
		m_Rcver->FlushBuf(pkt_cnt);
	}
}

int CIpApi::is_ip_ts_locked(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->IsTsLocked();
	}
	return	TLV_NO;
}

unsigned int CIpApi::get_eth_pkt_lost_cnt(void)
{
	if (m_Rcver != NULL)
	{
		return	m_Rcver->CntLostPkts();
	}
	return	0;
}


