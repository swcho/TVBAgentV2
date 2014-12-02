//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************
// 20110608: should have a method to protect internal buff of CircularBuffer object

#ifndef _CORE_LIB_H_
#define _CORE_LIB_H_

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#if defined(WIN32)
#include <conio.h>
#else
#endif
#include <string.h>
#include <math.h>
#include "play.h"


// Media source just support MPEG-2 TS format



class CCircularBuffer {
private:

	char *cir_buff;
	// size in bytes
	int cir_buff_size;
	// point to start position
	int cir_buff_start;
	// point to end position
	int cir_buff_end;

	int item_size;
	int num_item;

public:
	CCircularBuffer(int item_size, int num_item);
	~CCircularBuffer();

	////////////////////////////////////////
	// circular buffer's operations

	// do buffering
	// return 0: ok
	// return -1: fail (full)
	int BufferItem(char *item);
	int BufferNItem(char *item, int _num_item);
	int BufferNItem_ASI(char *item, int _num_item);

	// debuffering
	// return 0: ok
	// return -1: fail (empty)
	int DebufferItem(char *item);
	int DebufferNItem(char *item, int _num_item);
	int ReferDebufferedItem(char *buff, int _num_item);
	
	//2012/8/8 
	int DebufferNItem_sync(char *item, int _num_item);
	

	int NumBufferedItem();
	int NumUnbufferedItem();


	// return number of item
	int ItemSize();
	int NumItem();
	int IsFull();
	int IsEmpty();
	void Clear();

	int DebugOutputMsg(char *funcName);
};

/*

typedef struct {
	unsigned int id; // 4 bytes
	int bitrate;  // 4 bytes
	int size;  // 4 bytes 
} tlv_com_t;


// message queue
extern "C" void tlv_com_init(tlv_thread_t *_thread);
extern "C" void tlv_com_send(tlv_thread_t * _thread, tlv_com_t *message);
extern "C" void tlv_com_receive(tlv_thread_t * _thread, tlv_com_t *message);
extern "C" void tlv_com_destroy(tlv_thread_t *_thread);

*/




extern "C" {
	unsigned long getBits (unsigned char *buf, int byte_offset, int startbit, int bitlen);
	uint64_t getBits48 (unsigned char *buf, int byte_offset, int startbit, int bitlen);

	// for ISDBT
	void isdbt_set_layer_information1(long other_pid_map_to_layer,
						long multi_pid_map,
						long total_count, char* total_pid_info,
						long a_pid_count, char* a_pid_info,
						long b_pid_count, char* b_pid_info,
						long c_pid_count, char* c_pid_info,
						ISDBT_LAYER_INFORMATION *layer_information);

}


#endif
