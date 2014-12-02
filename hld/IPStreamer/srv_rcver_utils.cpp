
#define	_FILE_OFFSET_BITS	64
#define	_LARGEFILE_SOURCE

#if defined(WIN32)
#pragma comment (lib, "winmm.lib")

#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<fcntl.h>
//#include <unistd.h>
#include	<memory.h>
//#include	<sys/types.h>
//#include	<sys/stat.h>
#include	"include/pthread.h"
//#include	<sys/ipc.h>
//#include	<sys/msg.h>
#include	<time.h>
//#include	<sys/time.h>
//#include	<sys/shm.h>
//#include	<sys/ioctl.h>
//#include	<signal.h>
//#include	<sys/socket.h>
//#include	<netinet/in.h>
//#include	<arpa/inet.h>
//#include	<linux/unistd.h>
//#include	<linux/sysctl.h>
//#include	<linux/if.h>

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
#include "../include/hld_const.h"
extern	int	errno;

#include	"srv_rcver.h"

////////////////////////////////////////////////////////
static	pthread_mutex_t	_rcv_mutex_initializer_ = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////

//20140107 IP TEST
//FILE *fp_ip = NULL;
//===============

void	CSrvRcverUtils::__memcpy__(char *dest, char *src, int size)
{
	int	_loop;
	unsigned int	*_src, *_dest;

	_src = (unsigned int *)src;
	_dest = (unsigned int *)dest;
	for (_loop = 0; _loop < size; _loop += 4)
	{
		*_dest = *_src;
		_dest++;
		_src++;
	}
}

#if defined(WIN32)
int CSrvRcverUtils::recvfromtimeout(int fd, char *buf, size_t len, int flags, struct sockaddr *from, socklen_t fromlen, struct timeval tv)
{
    fd_set mySet;

    int n;

    FD_ZERO(&mySet);
    FD_SET(fd, &mySet);

    n = select(fd+1, &mySet, NULL, NULL, &tv);
    if (n == 0) return -2;
    if (n == -1) return -1;

    return recvfrom(fd, buf, len, flags, from, &fromlen);
}

unsigned int	CSrvRcverUtils::_msec_(void)
{
	unsigned int cur_msec;
	cur_msec = timeGetTime();
	return	cur_msec;
}
#else
long long int	CSrvRcverUtils::_msec_(void)
{
	long long int	cur_msec;
	struct	timeval	tv;
	//struct	timezone	tz;

	gettimeofday(&tv, NULL);
	cur_msec = ((long long int)tv.tv_sec*(long long int)1000) + (long long int)(tv.tv_usec/1000);

	return	cur_msec;
}
#endif

////////////////////////////////////////////////////////
CSrvRcverUtils::CSrvRcverUtils(void)
{
	dbg_notice = 0;
	dbg_warning = 0;
	dbg_noisy = 1;

	mtu_size_hard = ETHERNET_MTU_MAX;
	mtu_size_pkt = (MAX_UDP_SIZE_ON_ETHERNET + RTP_HDR_SIZE);
	cnt_rcv_b_pool = MAX_CNT_RCV_BUF_POOL;
	one_pkt_r_size = MAX_UDP_SIZE_ON_ETHERNET;	//	_dflt_mtu_real_data_size();

	_rd_i__ = 0;
	_wr_i__ = 0;
	rd_pool_pkt_data = NULL;
	rd_pool_seek = 0;
	rd_pool_tot_r_size = 0;

	_alloc_recv_pool_ = NULL;
	pkt_pool[0] = NULL;
//	AllocRcvBuf();

	memcpy(&_mutex, &_rcv_mutex_initializer_, sizeof(pthread_mutex_t));
	pthread_mutex_init(&_mutex, NULL);

	is_ts_locked = TLV_NO;

	connection_requested = TLV_YES;	//	this class create with connect-request
	connection_connected = TLV_NO;


	_S_BYTES_TO_CAL_PLAYRATE_ = S_BYTES_TO_CAL_PLAYRATE;
	src_protocol_type_is_rtp = TLV_YES;

//20140107 IP TEST
	//if(	fp_ip == NULL)
	//	fp_ip = fopen("2023IP_test.txt", "w");
//===============
}
CSrvRcverUtils::~CSrvRcverUtils()
{
//20140107 IP TEST
	//if(fp_ip != NULL)
	//	fclose(fp_ip);
//===============
}
void	CSrvRcverUtils::Mutex_Lock(void)
{
	pthread_mutex_lock(&_mutex);
}
void	CSrvRcverUtils::Mutex_Unlock(void)
{
	pthread_mutex_unlock(&_mutex);
}

int	CSrvRcverUtils::PreparedRcvBuf(void)
{
	if (pkt_pool[0] == NULL)
	{
		return	TLV_NO;
	}
	return	TLV_YES;
}
void	CSrvRcverUtils::AllocRcvBuf(void)
{
	int	ind;

	if (pkt_pool[0] != NULL)
	{
		return;
	}
	printf( "[ip-ts] rcv-alloc-rcv-ip-pool : [%d]..[%d]\n", cnt_rcv_b_pool, mtu_size_hard*cnt_rcv_b_pool);

	if (_alloc_recv_pool_ == NULL)
	{
		_alloc_recv_pool_ = (char *)malloc(mtu_size_hard*cnt_rcv_b_pool);
	}
	if (_alloc_recv_pool_ != NULL)
	{
		for (ind = 0; ind < cnt_rcv_b_pool; ind++)
		{
			pkt_raw_size[ind] = 0;
			pkt_pool[ind] = (char *)&_alloc_recv_pool_[ind*mtu_size_hard];
		}
	}
	else
	{
		for (ind = 0; ind < cnt_rcv_b_pool; ind++)
		{
			pkt_raw_size[ind] = 0;
			pkt_pool[ind] = (char *)malloc(mtu_size_hard);
		}
	}
}
void	CSrvRcverUtils::FreeRcvBuf(void)
{
	int	ind;

	if ((_alloc_recv_pool_ == NULL) && (pkt_pool[0] == NULL))
	{
		return;
	}

	if (_alloc_recv_pool_ != NULL)
	{
		free(_alloc_recv_pool_);
	}
	else
	{
		for (ind = 0; ind < cnt_rcv_b_pool; ind++)
		{
			free(pkt_pool[ind]);
		}
	}
	_alloc_recv_pool_ = pkt_pool[0] = NULL;
}
unsigned int	CSrvRcverUtils::BitRatePartialTs(void)
{
	return	rcv_bit_in_rate_188;
}
unsigned int	CSrvRcverUtils::BitRateFullTs(void)
{
	return	rcv_bit_in_rate;
}

void CSrvRcverUtils::setIP_delayTime(int millisec)
{
	delayTime_milli = millisec;
}

unsigned int	CSrvRcverUtils::ByteRateFullTs(void)
{
	return	rcv_byte_in_rate;
}
unsigned int	CSrvRcverUtils::CalcTsRateByContent(char *rd_buf, int rd_len)
{
//	#define	___CONTENT_B_RATE_FOR_TEST__
#ifdef	___CONTENT_B_RATE_FOR_TEST__
	int		bitrate;

	rcv_tot_to_calc_byterate_by_content += rd_len;
	if (rcv_tot_to_calc_byterate_by_content > _S_BYTES_TO_CAL_PLAYRATE_)
	{
		if (rd_len >= MIN_BYTES_TO_CALC_BITRATE)
		{
//			bitrate = _FileUtil->_Bitrate2(rd_buf, (size_t)rd_len);
			bitrate = TLV_MIN_BITRATE;
			if (bitrate > TLV_MIN_BITRATE)
			{
				rcv_bit_in_rate = bitrate;
				rcv_byte_in_rate = bitrate/8 + 1;
				rcv_tot_to_calc_byterate_by_content = 0;
				_S_BYTES_TO_CAL_PLAYRATE_ = rcv_byte_in_rate*20;
				DBG_PRINT( "[ip-ts] rcv-ts-rate : [%u:%d]..[%d]\n", rcv_byte_in_rate, bitrate, rd_len);
			}
			else
			{
_got_from_inrate:
				rcv_tot_to_calc_byterate_by_content = 0;
				_S_BYTES_TO_CAL_PLAYRATE_ = rcv_byte_in_rate*20;
//				printf( "[ip-ts] rcv-bytes-rate : error [%u:%d]..[%d]\n", rcv_byte_in_rate, bitrate, rd_len);
			}
		}
		else
		{
			goto	_got_from_inrate;
		}
	}
	return	rcv_byte_in_rate;
#endif

	return	rcv_byte_in_rate;	//	not use this func. refer to CalcTsRecvingRate()
}
unsigned int	CSrvRcverUtils::CalcTsRecvingRate(unsigned int _new_r_size)
	//	call if we have received new data. no time dependancy.
{
	static	int		prt_interval = 0;
#if defined(WIN32)
	unsigned int	cur_msec, elap_sec, elap_msec;
#else
	long long int	cur_msec, elap_sec, elap_msec;
#endif
	int				new_pkt_cnt, new_pkt_cnt_188;
	unsigned int	v_avg;

////////////////////////////////////////////////////////////////////////////////////
	if (_new_r_size == TLV_ZERO)	//	refer to __USE_DMA_ETH_DIRECT_MAPPING_POOL__
	{
		InitInputPara2();
		return	_new_r_size;
	}

////////////////////////////////////////////////////////////////////////////////////
	new_pkt_cnt = 0;

////////////////////////////////////////////////////////////////////////////////////
	cur_msec = _msec_();
	if (prev_msec_to_calc_bitrate <= 0)
	{
		rcv_tot_to_calc_rcving_rate = 0;
		prev_msec_to_calc_bitrate = cur_msec;
	}
	else
	{
		elap_msec = cur_msec - prev_msec_to_calc_bitrate;
		elap_sec = elap_msec/100;	//	unit in 10-msec. means 1 is 100-msec
#ifdef	_USE_ADAPTATION_ALG_FOR_CALCTING_BR_
		if (elap_sec >= (RECV_RATE_CALC_SEC*10))
#else
		if (elap_sec >= (RECV_RATE_CALC_SEC*5))	//	500-msec
#endif
		{
////////////////////////////////////////////////////////////////////////////////////
			if (_new_r_size == TLV_ZERO)	//	refer to __USE_DMA_ETH_DIRECT_MAPPING_POOL__
			{
//				OutputDebugString("_new_r_size == TLV_ZERO\n");
				new_pkt_cnt = 1; //	get total cnt and clear prev. cnt-value.
				rcv_tot_to_calc_rcving_rate += new_pkt_cnt*one_pkt_r_size;
			}
			else
			{
//				OutputDebugString("_new_r_size != TLV_ZERO\n");
				rcv_tot_to_calc_rcving_rate += _new_r_size;
			}

////////////////////////////////////////////////////////////////////////////////////
			rcv_bit_in_rate = (rcv_tot_to_calc_rcving_rate*8)/(unsigned int)elap_sec;
			rcv_bit_in_rate *= 10;	//	unit in sec
//			DBG_PRINT("[ip-ts] rcv-ts-tot-bitrate=[%u]\n", rcv_bit_in_rate);
			//sprintf(debug_string, "[ip-ts] rcv-ts-tot-bitrate=[%u], rcv_tot_to_calc_rcving_rate=%d, elap_sec=%d\n", rcv_bit_in_rate, rcv_tot_to_calc_rcving_rate, elap_sec);
			//OutputDebugString(debug_string);

			rcv_byte_in_rate = rcv_tot_to_calc_rcving_rate/(unsigned int)elap_sec;
			rcv_byte_in_rate *= 10;	//	byte-rate unit in sec
			rcv_byte_in_rate += 1;
			rcv_tot_to_calc_rcving_rate = 0;
			prev_msec_to_calc_bitrate = cur_msec;
			if (prt_interval++ > 5)
			{
				prt_interval = 0;
//				DBG_PRINT( "[ip-ts] ............ rcv-in-rate : [%u:%u]..[%lld]\n", rcv_byte_in_rate, rcv_byte_in_rate*8, elap_sec);
				//sprintf(debug_string, "[ip-ts] ............ rcv-in-rate : [%u:%u]..[%lld]\n", rcv_byte_in_rate, rcv_byte_in_rate*8, elap_sec);
				//OutputDebugString(debug_string);
			}
		}
		else rcv_tot_to_calc_rcving_rate += _new_r_size;//fixed ???

		v_avg = rcv_byte_in_rate - (rcv_byte_in_rate/rcv_rate_avg_cnt);
		v_avg = v_avg + (rcv_byte_in_rate/rcv_rate_avg_cnt);
		rcv_byte_in_rate = v_avg;
		v_avg = rcv_bit_in_rate - (rcv_bit_in_rate/rcv_rate_avg_cnt);
		v_avg = v_avg + (rcv_bit_in_rate/rcv_rate_avg_cnt);
		if (rcv_rate_avg_cnt < RECV_RATE_CALC_AVG_CNT)
		{
			rcv_rate_avg_cnt++;
		}
	}

////////////////////////////////////////////////////////////////////////////////////
	if (prev_msec_to_calc_188rate <= 0)
	{
		rcv_tot_cnt_188_pkt = 0;
		prev_msec_to_calc_188rate = cur_msec;
	}
	else
	{
		elap_msec = cur_msec - prev_msec_to_calc_188rate;
		elap_sec = elap_msec/100;	//	unit in 10-msec. means 1 is 100-msec
#ifdef	_USE_ADAPTATION_ALG_FOR_CALCTING_BR_
		if (elap_sec >= (RECV_RATE_CALC_SEC*10))
#else
		if (elap_sec >= (RECV_RATE_CALC_SEC*5))	//	500-msec
#endif
		{
////////////////////////////////////////////////////////////////////////////////////
			if (_new_r_size == TLV_ZERO)	//	refer to __USE_DMA_ETH_DIRECT_MAPPING_POOL__
			{
				new_pkt_cnt_188 = 1; //	get partial cnt and clear prev. cnt-value.
				rcv_tot_cnt_188_pkt += new_pkt_cnt_188;
			}
			else
			{
				rcv_tot_cnt_188_pkt += _new_r_size;
			}

////////////////////////////////////////////////////////////////////////////////////
			rcv_bit_in_rate_188 = (rcv_tot_cnt_188_pkt*188*8)/(unsigned int)elap_sec;
			rcv_bit_in_rate_188 *= 10;	//	unit in sec

			rcv_tot_cnt_188_pkt = 0;
			prev_msec_to_calc_188rate = cur_msec;
		}
		else rcv_tot_cnt_188_pkt += _new_r_size;//fixed ???

		rcv_bit_in_rate = v_avg;
		v_avg = rcv_bit_in_rate_188 - (rcv_bit_in_rate_188/rcv_rate_avg_cnt_188);
		v_avg = v_avg + (rcv_bit_in_rate_188/rcv_rate_avg_cnt_188);
		rcv_bit_in_rate_188 = v_avg;
		if (rcv_rate_avg_cnt_188 < RECV_RATE_CALC_AVG_CNT)
		{
			rcv_rate_avg_cnt_188++;
		}
	}

////////////////////////////////////////////////////////////////////////////////////
	return	new_pkt_cnt;
}
void	CSrvRcverUtils::InitInputPara2(void)
{
	rcv_tot_to_calc_byterate_by_content = _S_BYTES_TO_CAL_PLAYRATE_;

#ifdef	_USE_ADAPTATION_ALG_FOR_CALCTING_BR_
	rcv_bit_in_rate = rcv_bit_in_rate_188 = (REF_IP_INPUT_BYTE_RATE*8);
#else
	rcv_bit_in_rate = rcv_bit_in_rate_188 = 0;
#endif
	rcv_byte_in_rate = REF_IP_INPUT_BYTE_RATE;
	rcv_rate_avg_cnt = rcv_rate_avg_cnt_188 = 1;	//	donot replace 0.

	prev_msec_to_calc_bitrate = 0;
	rcv_tot_to_calc_rcving_rate = 0;

	prev_msec_to_calc_188rate = 0;
	rcv_tot_cnt_188_pkt = 0;
}
void	CSrvRcverUtils::InitInputPara(void)
{
	InitInputPara2();
	is_ts_locked = TLV_NO;
	_sequence_number = 0xffffffff;
	_cnt_lost_pkts = 0;
	//2014/01/08 IP improve
	timestamp_old = 0;
	sumTimestamp100 = 0;
	sumDataSize100 = 0;
	sampling = 0;
	////===================
}
int	CSrvRcverUtils::DecapRtp(char **ret_buf, int max_size)
{
	char	*src_seek;
	unsigned long	cp_avil_size;
	int		r_i;

////////////////////////////////////////////////////////////////////////////////////
	if (rd_pool_tot_r_size > 0)
	{
		goto	_process_remainder;
	}

////////////////////////////////////////////////////////////////////////////////////
	if (pkt_raw_size[_rd_i__] <= RTP_HDR_SIZE)
	{
//		printf("[ip-ts] rtp-pkt-size is wrong-- : [%d]\n", _rd_i__);
		goto	_broken_pool;
	}
	else if (pkt_raw_size[_rd_i__] <= mtu_size_pkt)
	{
		r_i = _rd_i__;
		rd_pool_pkt_data = &pkt_pool[r_i][0];
	}
	else
	{
		printf("[ip-ts] rtp-pkt-size is wrong-- : [%d]\n", _rd_i__);
		goto	_broken_pool;
	}

	if (rd_pool_pkt_data == NULL)
	{
		printf("[ip-ts] rtp-pkt-size is wrong--- : [%d]\n", _rd_i__);
		goto	_broken_pool;
	}

////////////////////////////////////////////////////////////////////////////////////
	switch(src_protocol_type_is_rtp)
	{
	case	TLV_NO:
		rd_pool_r_start = &rd_pool_pkt_data[0];
		rd_pool_tot_r_size = pkt_raw_size[r_i];
		rd_pool_seek = rd_pool_cpyed = 0;
		break;

	default:
		rd_pool_r_start = &rd_pool_pkt_data[RTP_HDR_SIZE];
		rd_pool_tot_r_size = pkt_raw_size[r_i] - RTP_HDR_SIZE;
		rd_pool_seek = rd_pool_cpyed = 0;
		break;
	}

	if (rd_pool_r_start == NULL)
	{
		printf("[ip-ts] rtp-pkt-size is wrong---- : [%d]\n", _rd_i__);
		goto	_broken_pool;
	}

////////////////////////////////////////////////////////////////////////////////////
_process_remainder:
	if (rd_pool_tot_r_size <= TLV_ZERO)
	{
		goto	_broken_pool;
	}

	src_seek = rd_pool_r_start + rd_pool_seek;
	cp_avil_size = ((unsigned long)rd_pool_r_start + rd_pool_tot_r_size) - (unsigned long)src_seek;
	if (cp_avil_size > one_pkt_size_w_hdr)
	{
		goto	_broken_pool;
	}

	if (cp_avil_size > max_size)	//	no room to copy all data of src-buf.
	{
		cp_avil_size = max_size;
	}
	rd_pool_seek += cp_avil_size;
	rd_pool_cpyed += cp_avil_size;

	*ret_buf = src_seek;

	if (rd_pool_cpyed < rd_pool_tot_r_size)
	{
		//	has more data to copy at next time.
	}
	else
	{
		_rd_i__ = (_rd_i__ >= (cnt_rcv_b_pool - 1)) ? 0 : (_rd_i__ + 1);
		rd_pool_tot_r_size = 0;
	}
	return	cp_avil_size;

_broken_pool:
	rd_pool_tot_r_size = 0;
	rd_pool_seek = 0;
	rd_pool_pkt_data = NULL;
	rd_pool_r_start = NULL;
	_rd_i__ = (_rd_i__ >= (cnt_rcv_b_pool - 1)) ? 0 : (_rd_i__ + 1);
	return	TLV_ZERO;
}
int	CSrvRcverUtils::PrePeocessRtpDirectEth(int rcv_buf_ind, int _rcv_l)
	//	call if we have received new data. no time dependancy.
{
	int	new_pkt_cnt;

	new_pkt_cnt = CalcTsRecvingRate(TLV_ZERO);
	return	(new_pkt_cnt*one_pkt_r_size);
}
int	CSrvRcverUtils::PrePeocessRtp(int rcv_buf_ind, int _rcv_l)
{
	static	int	overf_prt_interval = 0;
	unsigned int	_r_size_this_turn;
//20140107 IP TEST
	unsigned int timestamp;
//	char payloadType;
	int bitrate;
#ifdef WIN32
	__int64 delta_timestamp;
#else
	long long delta_timestamp;
#endif
#if 0
	static int startTime;
	int currentTime;
#ifdef WIN32
	SYSTEMTIME systemTime;
#else
	struct timeval systemTime;
#endif
#endif
//===============
////////////////////////////////////////////////////////////////////////////////////
	_r_size_this_turn = TLV_ZERO;

////////////////////////////////////////////////////////////////////////////////////
	if (_diff_node_(_rd_i__,_wr_i__) >= (cnt_rcv_b_pool - 2))
	{
		if (overf_prt_interval++ > 100)
		{
			overf_prt_interval = 0;
			printf("[ip-ts] ... rcv-buffer-overflow : [%d:%d].[%d]\n", _rd_i__, _wr_i__, cnt_rcv_b_pool);
		}
		cnt_rd_buf_consume = cnt_rcv_b_pool*50/100;
	}

////////////////////////////////////////////////////////////////////////////////////
	if(pkt_pool[rcv_buf_ind][0] == _UDP_INDICATOR_1st_BYTE_)
	{
		src_protocol_type_is_rtp = TLV_NO;
	}
	else
	{
		src_protocol_type_is_rtp = TLV_YES;
	}

	pkt_raw_size[rcv_buf_ind] = _rcv_l;
	switch(src_protocol_type_is_rtp)
	{
	case	TLV_NO:
		if (_rcv_l <= mtu_size_pkt)
		{
			one_pkt_r_size = _rcv_l;
			one_pkt_size_w_hdr = one_pkt_r_size;
			_r_size_this_turn += one_pkt_r_size;
		}
		else
		{
//			printf("[ip-ts] udp-pkt-size is wrong : [%d]\n", _rcv_l);
			pkt_raw_size[rcv_buf_ind] = 0;
		}
		break;

	default:	//	rtp
		if (_rcv_l <= RTP_HDR_SIZE)
		{
//			printf("[ip-ts] rtp-pkt-size is wrong : [%d]\n", _rcv_l);
			pkt_raw_size[rcv_buf_ind] = 0;
		}
		else if (_rcv_l <= mtu_size_pkt)
		{
			one_pkt_r_size = _rcv_l - RTP_HDR_SIZE;
			one_pkt_size_w_hdr = one_pkt_r_size + RTP_HDR_SIZE;
			_r_size_this_turn += one_pkt_r_size;

////////////////////////////////////////////////////////////////////////////////////
#ifdef	____SEQ_CHK___	//	seq-chk
			unsigned int	_seq;
			_seq = ((pkt_pool[rcv_buf_ind][2] & 0xff) << 8);
			_seq |= ((pkt_pool[rcv_buf_ind][3] & 0xff) << 0);
			if (_sequence_number == 0xffffffff)
			{
			}
			else
			{
				if (_seq == 0)
				{
					if (_sequence_number != 0xffff)
					{
						_cnt_lost_pkts++;
					}
				}
				else
				{
					if (_seq != (_sequence_number + 1))
					{
						_cnt_lost_pkts++;
					}
				}
			}
			_sequence_number = _seq;

			timestamp = ((pkt_pool[rcv_buf_ind][4] & 0xff) << 24) + ((pkt_pool[rcv_buf_ind][5] & 0xff) << 16) + ((pkt_pool[rcv_buf_ind][6] & 0xff) << 8) + ((pkt_pool[rcv_buf_ind][7] & 0xff) << 0);
			if(timestamp_old == 0)
			{
				timestamp_old = timestamp;
				samplingCnt = delayTime_milli;
				sampling = 0;
				samplingFlag = 1;
#if 0
				samplingCnt = 0;
#ifdef WIN32
				GetLocalTime(&systemTime);
				startTime = (systemTime.wHour * 60 * 60 * 1000) + (systemTime.wMinute * 60 * 1000) + (systemTime.wSecond * 1000) + systemTime.wMilliseconds;
#else
				gettimeofday(&systemTime);
				startTime = (systemTime.tv_sec * 1000) + (systemTime.tv_usec / 1000);
#endif
#endif
			}
			else
			{
				if(timestamp_old != timestamp)
				{
////20140107 IP TEST
//	if(fp_ip != NULL)
//	{
//		char str_testIp[256];
//		char payloadType;
//		payloadType = ((pkt_pool[rcv_buf_ind][1] & 0x7f) << 0);
//		sprintf(str_testIp, "one_pkt_r_size[%d], payloadType[%d], _sequence_number[%d], timestamp[%u]\n",one_pkt_r_size, payloadType, _sequence_number, delta_timestamp);
//		fwrite(str_testIp, 1, strlen(str_testIp),fp_ip);
//	}
#if 0
					if(samplingFlag == 1)
					{
						sumDataSize100 = sumDataSize100 + one_pkt_r_size;
						sumTimestamp100 = sumTimestamp100 + delta_timestamp;
						samplingCnt++;
#ifdef WIN32
						GetLocalTime(&systemTime);
						currentTime = (systemTime.wHour * 60 * 60 * 1000) + (systemTime.wMinute * 60 * 1000) + (systemTime.wSecond * 1000) + systemTime.wMilliseconds;
#else
						gettimeofday(&systemTime);
						currentTime = (systemTime.tv_sec * 1000) + (systemTime.tv_usec / 1000);
#endif
						if(startTime < currentTime)
						{
							if((currentTime - startTime) >= delayTime_milli)
							{
								bitrate = (int)( (__int64)((__int64)(sumDataSize100) * (__int64)720000) / sumTimestamp100 ); 
								rcv_bit_in_rate = bitrate;
								samplingFlag = 0;
							}
						}
						else
						{
							if((startTime - currentTime) >= delayTime_milli)
							{
								bitrate = (int)( (__int64)((__int64)(sumDataSize100) * (__int64)720000) / sumTimestamp100 ); 
								rcv_bit_in_rate = bitrate;
								samplingFlag = 0;
							}
						}
						//char str_testIp[256];
						//sprintf(str_testIp, "delayTime_milli[%d], samplingCnt[%d], timestamp[%u]\n",delayTime_milli, samplingCnt, delta_timestamp);
						//OutputDebugString(str_testIp);
					}
					else
#else
					
#endif
					delta_timestamp = timestamp - timestamp_old;
					if(timestamp < timestamp_old)
					{
						//timestamp_old = timestamp;
						//delta_timestamp = delta_timestamp + 4294967295;
					}
					else
					{
						if(sampling != samplingCnt)
						{
							sampling++;
							sumDataSize100 = sumDataSize100 + one_pkt_r_size;
							sumTimestamp100 = sumTimestamp100 + delta_timestamp;
							if(samplingFlag == 1)
							{
#ifdef WIN32
								bitrate = (int)( (__int64)((__int64)(sumDataSize100) * (__int64)720000) / sumTimestamp100 );
#else
								bitrate = (int)( (long long)((long long)(sumDataSize100) * (long long)720000) / sumTimestamp100 );
#endif
								samplingCnt = bitrate / 10000;
								if(samplingCnt > 1000)
									samplingCnt = 1000;
								samplingFlag = 0;
							}
						}
						else
						{
#ifdef WIN32
							bitrate = (int)( (__int64)((__int64)(sumDataSize100) * (__int64)720000) / sumTimestamp100 ); 
#else
							bitrate = (int)( (long long)((long long)(sumDataSize100) * (long long)720000) / sumTimestamp100 ); 
#endif
							rcv_bit_in_rate = bitrate;
							sampling = 1;
							sumDataSize100 = one_pkt_r_size;
							sumTimestamp100 = delta_timestamp;
						}
					}
					timestamp_old = timestamp;
				}
			}

			//2014/01/08 IP bitrate
			if (cnt_rd_buf_consume > 0)
			{
				return	0;
			}

			////////////////////////////////////////////////////////////////////////////////////
			_wr_i__ = (_wr_i__ >= (cnt_rcv_b_pool - 1)) ? 0 : (_wr_i__ + 1);

			
			
			return	_sequence_number;

#endif
		}
		else
		{
//			printf("[ip-ts] rtp-pkt-size is wrong : [%d]\n", _rcv_l);
			pkt_raw_size[rcv_buf_ind] = 0;
		}

		break;
	}

////////////////////////////////////////////////////////////////////////////////////
	if (_r_size_this_turn != TLV_ZERO)
	{
		CalcTsRecvingRate(_r_size_this_turn);
	}

	if (cnt_rd_buf_consume > 0)
	{
		return	0;
	}

////////////////////////////////////////////////////////////////////////////////////
	if (_rcv_l <= RTP_HDR_SIZE)
	{
//		printf("[ip-ts] rtp-pkt-size is wrong- : [%d]\n", _rcv_l);
	}
	else if (_rcv_l <= mtu_size_pkt)
	{
		_wr_i__ = (_wr_i__ >= (cnt_rcv_b_pool - 1)) ? 0 : (_wr_i__ + 1);
	}
	else
	{
//		printf("[ip-ts] rtp-pkt-size is wrong- : [%d]\n", _rcv_l);
	}

	return	_sequence_number;
}
void	CSrvRcverUtils::FlushBufUdpPool(int pkt_cnt)
{
	int	ind;
	static	unsigned int	overrun_prt = 0;

	Mutex_Lock();
	if (pkt_cnt == TLV_ZERO)	//	all flush
	{
		_rd_i__ = 0;
		_wr_i__ = 0;
		rd_pool_tot_r_size = 0;
		rd_pool_seek = 0;
		rd_pool_pkt_data = NULL;
	}
	else
	{
		for (ind = 0; ind < pkt_cnt; ind++)
		{
			if (_wr_i__ == _rd_i__)
			{
				break;
			}
			_rd_i__ = (_rd_i__ >= (cnt_rcv_b_pool - 1)) ? 0 : (_rd_i__ + 1);
		}
		rd_pool_tot_r_size = 0;
		rd_pool_seek = 0;
		rd_pool_pkt_data = NULL;

		overrun_prt++;
		if ((overrun_prt%100) == 0)
		{
			printf( "[ip-ts] flush-ip-rcv-buf : [%d/%d]...[%u]\n", ind, pkt_cnt, overrun_prt);
		}
	}
	Mutex_Unlock();
}
unsigned int	CSrvRcverUtils::CntLostPkts(void)
{
	return	_cnt_lost_pkts;
}


