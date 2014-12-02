
#ifndef	TELEVIEW_FEC_PROTO_H
#define	TELEVIEW_FEC_PROTO_H

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
//#include	<linux/if_ether.h>
//#include	<linux/ip.h>
//#include	<linux/udp.h>
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
#include	<linux/if_ether.h>
#include	<linux/ip.h>
#include	<linux/udp.h>
#endif

///////////////////////////////////////////
#define	__CONSTANT_TMSTAMP_AND_SSRC__

//#define	__DATAGRAM_CONTENT_UDP__
#define	__DATAGRAM_CONTENT_RTP__

#define	__CLI_SND_VIA_UDP_IP__
//	#define	__CLI_SND_ETH_DIRECT__

///////////////////////////////////////////
typedef	struct	__rtp_hdr	//	rfc3550
{
	unsigned char	v2_p1_x1_cc4;
	unsigned char	m1_pt7;
	unsigned char	sn_msb8;
	unsigned char	sn_lsb8;

	unsigned char	tm_s_31_24;
	unsigned char	tm_s_23_16;
	unsigned char	tm_s_15_8;
	unsigned char	tm_s_7_0;

	unsigned int	ssrc_31_0;
//	unsigned int	csrc_31_0;	//	optional
}	__RTP_HDR;
typedef	struct	__fec_hdr	//	draft interleaving scheme-02
{
	unsigned char	snb_low_msb8;
	unsigned char	snb_low_lsb8;
	unsigned char	l_rcvr_msb8;
	unsigned char	l_rcvr_lsb8;

	unsigned char	e1_pt_rcvr7;
	unsigned char	mask_23_16;
	unsigned char	mask_15_8;
	unsigned char	mask_7_0;

	unsigned char	tm_s_rcvr31_24;
	unsigned char	tm_s_rcvr23_16;
	unsigned char	tm_s_rcvr15_8;
	unsigned char	tm_s_rcvr7_0;

	unsigned char	n1_d1_t3_i3;
	unsigned char	off;
	unsigned char	na;
	unsigned char	snb_ext;
}	__FEC_HDR;

///////////////////////////////////////////
#define	MAX_CNT_snd_RTP_C_OFF		16
#define	MAX_CNT_snd_RTP_C_NA			10
#define	MAX_CNT_snd_RTP_L_OFF		1	//	not support yet
#define	MAX_CNT_snd_RTP_L_NA			1
#define	MIM_CNT_snd_RTP_LC			1
#define	MAX_I_snd_SEQ_NUM			0xffff

#define	RTP_HDR_SIZE					sizeof(__RTP_HDR)
#define	RTP_FEC_HDR_SIZE				sizeof(__FEC_HDR)

#define	_URB_TX_CMD_CND				4
#define	_DUMMY_LEN_TO_ALIGN			2


#endif
