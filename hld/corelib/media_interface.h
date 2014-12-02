//************************************************************************* //	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _MEDIA_INTERFACE_
#define _MEDIA_INTERFACE_

#include <stdarg.h>

#include "tlv_type.h"
#include "tlv_threads.h"
#include "corelib.h"
#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "mpeg2ts.h"




// note: Source is later replaced by Input :w
#define INPUT_ERROR_EOF -1
#define INPUT_ERROR_BUFFERING -2
#define INPUT_ERROR_SOCKET_CLOSE -3

#define UDP_DATA_SIZE_MAX 16*1024 // ?? what is the maximun size 
#define BUFFER_TS_DATA_SIZE_MAX 60*1024
#define FILE_READ_DATA_SIZE_MAX 1024*1024 // ?? what is the maximun size 



class CHldPlayback;

struct TLVPLAY_MEDIA_OPTION{

	int num_filter_pids;
	int filter_pids[500];


	int bitrate; // usage in dvb-t2 when playing from ip source 
	int media_source_type;
	char ip_address[100]; // ip address, unicast address, multicast address 
	int udp_port;
	


	// networking or file option 
	char networking_or_file[256]; 
};


class CTLVMedia{

public:
	TLVPLAY_MEDIA_OPTION option;
	char name[256];

public:
	// a playing system 
	CHldPlayback *sys_play; 
	// settings for initializing media
	// ex: file_path for file, IP address, port, for opening IP 
	void *param_for_open; 
	// ?? thread handle of input 
	// todo: design thread scheme 
	tlv_thread_t *thread_handle; 
	// for buffering the receiving ts 
	// the size is about 0.5 or 1 second 
	CCircularBuffer *cir_input_ts_buffer;
	// for mutual exclusive control of cir_input_ts_buffer
	// 
	tlv_mutex_t *mutex_cir_data;
	tlv_cond_t cv_cir_in;
	tlv_cond_t cv_cir_out;	

	MPEG2TSStatisticAndFilter mpeg2ts_statistic; 

public: 
	void init(CHldPlayback *_sys_play);
	void quit();
	virtual void open(void *arg) = 0;
	// > 0: ok
	// <=0: erro
	virtual int read_one_packet(char *buff) = 0;

	// read n byte in max,
	// return 0: reading NOT succesfully 
	// num_byte: reading successfully 
	virtual int read_n_byte(char *des, int num_byte ) = 0; 

	virtual void close(void *) = 0;
	// reutrn >= 0: normal
	// return -1: need to end thread
	virtual int _read_mpeg2ts_data(char *buff) = 0;	
	virtual int read_data(char *buff, int max_num_byte) = 0;
	virtual void force_close() = 0;

	virtual int ioctl(int count, ... );

	int media_receving_data_loop(void *param);

public:

};

class CDVBT2_interface: public CTLVMedia
{
public:
	int bitrate; 
public:
	virtual int get_bitrate() = 0;
	virtual void set_bitrate(int _bitrate) = 0;
};

class CDVBC2_interface: public CTLVMedia
{
public:
	int bitrate; 
public:
	virtual int get_bitrate() = 0;
	virtual void set_bitrate(int _bitrate) = 0;
};
#endif


