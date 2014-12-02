
#ifndef	TELEVIEW_SRV_RCVER_UTILS_H
#define	TELEVIEW_SRV_RCVER_UTILS_H

#if defined(WIN32)
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
//#include	<unistd.h>
#include	"include/pthread.h"
#include	"include/semaphore.h"
#include	<errno.h>
//#include	<sys/socket.h>
//#include	<netinet/in.h>
//#include	<arpa/inet.h>
//#include	<linux/unistd.h>
//#include	<linux/sysctl.h>
//#include	<linux/if.h>
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

//#include	"stb_sysconf.h"
#include	"srv_sock.h"
#include	"fec_proto.h"

#define	____SEQ_CHK___

///////////////////////////////////////////
#define	MAX_CNT_RCV_BUF_POOL			8500	//	9800
#define	FLUSH_PKT_CNT_IN_OVERRUN	(MAX_CNT_RCV_BUF_POOL*5/100)


#define	REF_IP_INPUT_BYTE_RATE		(20000000/8)		//	0x300000	//	0x400000

#define	NETWORK_CHCK__SEC			5
#define	UDP_BUFF_CHCK__SEC		1
#define	RECV_RATE_CALC_SEC			1	//	2	//	3
#define	S_BYTES_TO_CAL_PLAYRATE		20000000	//	10 Mbytes
#define	PKT_RECV_TIMEOUT__TS_UNLOCK	3000	//	2000
#define	RECV_RATE_CALC_AVG_CNT	3	//	10

////////////////////////////////////////////////////////
#define	_UDP_INDICATOR_1st_BYTE_	0x47
#define	_diff_node_(r,w)	((r > w) ? ((MAX_CNT_RCV_BUF_POOL - r) + w) : (w - r))

///////////////////////////////////////////
class	CSrvRcverUtils
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;

	int				mtu_size_hard;
	unsigned int	rcv_tot_to_calc_byterate_by_content;
	unsigned int	_S_BYTES_TO_CAL_PLAYRATE_;
#if defined(WIN32)
	unsigned int	prev_msec_to_calc_bitrate;
#else
	long long int	prev_msec_to_calc_bitrate;
#endif
	unsigned int	rcv_tot_to_calc_rcving_rate;
	unsigned int	rd_pool_tot_r_size;
	char			*rd_pool_pkt_data;
	char			*rd_pool_r_start;
	unsigned int	rd_pool_seek;
	unsigned int	rd_pool_cpyed;
	unsigned int	one_pkt_size_w_hdr;
	int				pkt_raw_size[MAX_CNT_RCV_BUF_POOL];
	int				src_protocol_type_is_rtp;
	unsigned int	_sequence_number;
	unsigned int	_cnt_lost_pkts;

	//2014/01/08 IP improve
	unsigned int timestamp_old;
#ifdef WIN32
	__int64 sumTimestamp100;
	__int64 sumDataSize100;
#else
	long long sumTimestamp100;
	long long sumDataSize100;
#endif
	int sampling;
	int samplingFlag;
	int samplingCnt;
	//2014/01/09 IP Delay Time
	int	delayTime_milli;
	////===================
#if defined(WIN32)
	unsigned int	prev_msec_to_calc_188rate;
#else
	long long int	prev_msec_to_calc_188rate;
#endif
	unsigned int	rcv_tot_cnt_188_pkt;
	pthread_mutex_t   _mutex;

	unsigned int	rcv_byte_in_rate;
	unsigned int	rcv_bit_in_rate;
	unsigned int	rcv_bit_in_rate_188;
	unsigned int	rcv_rate_avg_cnt;
	unsigned int	rcv_rate_avg_cnt_188;

	char	*_alloc_recv_pool_;
public:
	int				mtu_size_pkt;
	int				cnt_rcv_b_pool;
	int				connection_requested;
	int				connection_connected;
	int				cnt_rd_buf_consume;

	int				is_ts_locked;
	unsigned int	one_pkt_r_size;

	char	*pkt_pool[MAX_CNT_RCV_BUF_POOL];
	int		_rd_i__;
	int		_wr_i__;
	
private:
#if defined(WIN32)
	unsigned int _msec_(void);
#else
	long long int	_msec_(void);
#endif
	unsigned int	CalcTsRecvingRate(unsigned int _new_r_size);

public:
	CSrvRcverUtils(void);
	~CSrvRcverUtils();

	void	__memcpy__(char *dest, char *src, int size);
	void	Mutex_Lock(void);
	void	Mutex_Unlock(void);

	int		PreparedRcvBuf(void);
	void	AllocRcvBuf(void);
	void	FreeRcvBuf(void);
	unsigned int	BitRatePartialTs(void);
	unsigned int	BitRateFullTs(void);
	unsigned int	ByteRateFullTs(void);
	unsigned int	CalcTsRateByContent(char *rd_buf, int rd_len);
	void	InitInputPara2(void);
	void	InitInputPara(void);
	int		DecapRtp(char **ret_buf, int max_size);
	int		PrePeocessRtpDirectEth(int rcv_buf_ind, int _rcv_l);
	int		PrePeocessRtp(int rcv_buf_ind, int _rcv_l);
	void	FlushBufUdpPool(int pkt_cnt);
	unsigned int	CntLostPkts(void);

	//2014/01/09 IP Delay Time
	void setIP_delayTime(int millisec);
#if defined(WIN32)
	char debug_string[512];
	int recvfromtimeout(int fd, char *buf, size_t len, int flags, struct sockaddr *from, socklen_t fromlen, struct timeval tv);
#endif
};

#endif
