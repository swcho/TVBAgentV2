//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _INPUT_TS_
#define _INPUT_TS_

#include "media_interface.h"

#include "remux_mpeg2ts.h"

#define IO_REQUEST_REMUX_OUT_BITRATE 1
#define IO_REQUEST_ADD_PID_FOR_TS_REMUX 2

class MTSRemux: public CTLVMedia
{
public:
	FILE *file_handler; 
	char *ts_packet; 
	int buf_end;
	int buf_start;
	int packet_size;

	handle mpeg2ts_remux_handle; 


	
	int is_closed;

public: 
	MTSRemux();
	~MTSRemux();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
	int ioctl(int count, ... );
};

#endif
