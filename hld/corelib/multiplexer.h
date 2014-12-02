//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#ifndef	_TLV_MULTIPLEXER_
#define _TLV_MULTIPLEXER_

#include "corelib.h"

class CHldPlayback;

class Multiplexer{
public:
	CHldPlayback *play_sys;

public:
	void *multiplex_option; // DVBT2_PARAM, ISDBT_PARAM	
	char name[256];
	
public:
	virtual void init() = 0;
	virtual int open(CHldPlayback *core_broadcast) = 0;
	virtual void seek(int percentage) = 0;	
	virtual int ioctl(int message_type, void *param) = 0;
	// return -1: error 
	// return size: ok 
	virtual int produce_item(char *buff, int size) = 0;

	// des: read num_byte from multiplexer cir buffer 
	// this function is call in output's thread context 
	//
	// return number of byte is read, 
	//
	int read_n_byte(char *buff, int num_byte);


	virtual void close() = 0;
	virtual void quit() = 0;

	int main_loop(void *param);
};



#endif
