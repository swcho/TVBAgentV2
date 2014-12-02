//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _DATA_READER_FROM_IP_
#define _DATA_READER_FROM_IP_


#include "tlv_type.h"
#include "tlv_threads.h"
#include "media_interface.h"
#include "corelib.h"

extern "C"
{
	int udp_server_open(tlv_socket_t *socket_id, int udp_port);
	// return number of received bytes 
	int udp_server_receive(tlv_socket_t sd, char *buff);
	int rtp_server_receive(tlv_socket_t sd, char *buff);
	int udp_server_close(tlv_socket_t sd);

}

///////////////////////////////////////////
typedef	struct	_RTP_HEADER	//	rfc3550
{
	unsigned char	v2_p1_x1_cc4; // 2 bit, 1 bit, 1 bit, 4 bit 
	unsigned char	m1_pt7;  // 1 bit, 7 bit
	unsigned char	sn_msb8; // 8 bit
	unsigned char	sn_lsb8; // 8 bit

	unsigned char	tm_s_31_24;// 8 bit 
	unsigned char	tm_s_23_16;// 8 bit 
	unsigned char	tm_s_15_8; // 8 bit 
	unsigned char	tm_s_7_0;  // 8 bit 

	unsigned int	ssrc_31_0; // 32 bit 
//	unsigned int	csrc_31_0;	//	optional
}RTP_HEADER;

#endif

