//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _INPUT_IP_
#define _INPUT_IP_

#include "corelib.h"

class CTLVIPServer: public CDVBT2_interface
{

public:

	int received_ts_size; // # of bytes

	tlv_socket_t socket_id;
	char *ts_buffer;
	int ts_buffer_start;
	int ts_buffer_size;
	char null_packet[204];


private:
	// blabla
	int start_time;
	int end_time; 
public: 
	CTLVIPServer();
	~CTLVIPServer();
	void open(void *file_path);
	int read_one_packet(char *buff);
	void close(void *);	
	int read_n_byte(char *des, int num_byte);

	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();

	int get_bitrate();
	void set_bitrate(int bitrate);
};

#endif
