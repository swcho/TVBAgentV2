//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

//#include <Windows.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "corelib.h"


/* some utilities
 * will be make another file in the future
 */

//#define MULTI_PLP_DEBUG
#define T2MI_PACKET_SIZE_MAX 3*1024*1024
#define PLAYBACK_TS_PACKET_SIZE_MAX 300


#define TS_SYNCD_BUFF_SIZE_MAX 300




CCircularBuffer::CCircularBuffer(int _item_size, int _num_item)
{
	item_size = _item_size;
	num_item = _num_item + 1;
	cir_buff_size = (num_item)*_item_size;
	cir_buff = (char *) malloc(cir_buff_size);
	cir_buff_start = 0;
	cir_buff_end = 0;
	//ssert(cir_buff != NULL);

}

CCircularBuffer::~CCircularBuffer()
{
	free(cir_buff);
}

void CCircularBuffer::Clear(){
	cir_buff_start = 0;
	cir_buff_end = 0;
	memset(cir_buff, 0, cir_buff_size);
}


int CCircularBuffer::BufferItem(char *item){

        //Use modulo as a trick to wrap around the end of the buffer back to the beginning
        if ((cir_buff_end + item_size) % cir_buff_size != cir_buff_start){
        	memcpy(&cir_buff[cir_buff_end], item, item_size);

			cir_buff_end = (cir_buff_end + item_size)% cir_buff_size;
			return 0;
		}else {
			return -1;
		}
        //otherwise, the buffer is full; don't do anything. you might want to
        //return an error code to notify the writing process that the buffer is full.
}

int CCircularBuffer::NumBufferedItem()
{
	if (cir_buff_start <= cir_buff_end)
		return (cir_buff_end - cir_buff_start)/item_size;
	else
		return (cir_buff_size - (cir_buff_start - cir_buff_end))/item_size;

}

int CCircularBuffer::DebugOutputMsg(char *funcName)
{
	//printf("%s : start[%d], end[%d], size[%d], nitem[%d]\n",funcName, cir_buff_start,cir_buff_end,cir_buff_size,num_item);
	return 0;
}
int CCircularBuffer::NumUnbufferedItem()
{
	int num_buffered_item = this->NumBufferedItem();
	return num_item - 1 - num_buffered_item;
}

// return: number of buffered item which is referred
int CCircularBuffer::ReferDebufferedItem(char *buff, int _num_item){

	if (IsEmpty() == 1)
		return 0;

	int cir_buff_start_old = cir_buff_start;
	int ref_buffered_item = this->DebufferNItem(buff, _num_item);

	cir_buff_start = cir_buff_start_old;
	return ref_buffered_item;
}


// check if buffer is full
// return: 1: full
//         0: not full
int CCircularBuffer::IsFull()
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
		if ((cir_buff_end + item_size) % cir_buff_size != cir_buff_start)
			return 0;
		else
			return 1;
	}

// check if buffer is full
// return: 1: full
//         0: not full
int CCircularBuffer::IsEmpty()
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
		if (cir_buff_end != cir_buff_start)
			return 0; // not empty
		else
			return 1; // empty
	}




// buffer n Items
// parameters:
// _num_item: number of item for adding
// _item: point to the begging of buffer item

// return number of item is buffered
// return: 0, buffer is full

int CCircularBuffer::BufferNItem(char *item, int _num_item){

	if (IsFull() == 1)
		return 0;

	// get number of item which is buffered
	int num_item_buffered = this->NumBufferedItem();
	// num of item unbuffered
	int num_item_unbuffered = num_item - num_item_buffered - 1;
	// num of item for buffering
	int num_item_xxx = (num_item_unbuffered < _num_item ? num_item_unbuffered: _num_item);

	// not good naming ????
	int gap_end_and_last_item = num_item - cir_buff_end/item_size;
	if (num_item_xxx <= gap_end_and_last_item)
	{
			memcpy(&cir_buff[cir_buff_end], item, num_item_xxx*item_size);
			cir_buff_end = (cir_buff_end + num_item_xxx*item_size) % cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(&cir_buff[cir_buff_end], item, gap_end_and_last_item);
		long dbg_offset;
		cir_buff_end = (cir_buff_end + gap_end_and_last_item*item_size) % cir_buff_size;
		//ssert(cir_buff_end == 0); // pointer_end is wrapped around
		dbg_offset = gap_end_and_last_item * item_size;
		memcpy(&cir_buff[cir_buff_end], item + dbg_offset, (num_item_xxx - gap_end_and_last_item)*item_size);
		cir_buff_end = (cir_buff_end + (num_item_xxx - gap_end_and_last_item)*item_size) % cir_buff_size;
		return num_item_xxx;
	}
}

int CCircularBuffer::BufferNItem_ASI(char *item, int _num_item){

	if (IsFull() == 1)
		return 0;


	// get number of item which is buffered
	int num_item_buffered = this->NumBufferedItem();
	// num of item unbuffered
	int num_item_unbuffered = num_item - num_item_buffered - 1;
	// num of item for buffering
	int num_item_xxx = (num_item_unbuffered < _num_item ? num_item_unbuffered: _num_item);

	// not good naming ????
	int gap_end_and_last_item = num_item - cir_buff_end/item_size;
	if (num_item_xxx <= gap_end_and_last_item)
	{
			memcpy(&cir_buff[cir_buff_end], item, num_item_xxx*item_size);
			cir_buff_end = (cir_buff_end + num_item_xxx*item_size) % cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(&cir_buff[cir_buff_end], item, gap_end_and_last_item);
		cir_buff_end = (cir_buff_end + gap_end_and_last_item*item_size) % cir_buff_size;
		//ssert(cir_buff_end == 0); // pointer_end is wrapped around

		memcpy(&cir_buff[cir_buff_end], item + gap_end_and_last_item*item_size, (num_item_xxx - gap_end_and_last_item)*item_size);
		cir_buff_end = (cir_buff_end + (num_item_xxx - gap_end_and_last_item)*item_size) % cir_buff_size;
		return num_item_xxx;
	}
}


// debuffer n Items
// parameters:
// _num_item: number of item for adding
// _item: point to the begging of buffer item

// return number of item is debuffered
// return: 0, buffer is empty

int CCircularBuffer::DebufferNItem(char *item, int _num_item){

	if (IsEmpty() == 1)
		return 0;

	// get number of item which is buffered
	int num_item_buffered = this->NumBufferedItem();
	// num of item for debufferting
	int num_item_xxx = (num_item_buffered < _num_item ? num_item_buffered: _num_item);

	// not good naming ????
	int gap_start_and_last_item = num_item - cir_buff_start/item_size;

	if (num_item_xxx <= gap_start_and_last_item)
	{
			memcpy(item, &cir_buff[cir_buff_start], num_item_xxx*item_size);
			cir_buff_start = (cir_buff_start + num_item_xxx*item_size) % cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(item, &cir_buff[cir_buff_start], gap_start_and_last_item);
		cir_buff_start = (cir_buff_start + gap_start_and_last_item*item_size) % cir_buff_size;
		//ssert(cir_buff_start == 0); // pointer_end is wrapped around
		long dbg_offset = gap_start_and_last_item * item_size;
		memcpy(item + dbg_offset, &cir_buff[cir_buff_start], (num_item_xxx - gap_start_and_last_item)*item_size);
		cir_buff_start = (cir_buff_start + (num_item_xxx - gap_start_and_last_item)*item_size) % cir_buff_size;
		return num_item_xxx;
	}
}

//2012/8/8
int CCircularBuffer::DebufferNItem_sync(char *item, int _num_item){

	if (IsEmpty() == 1)
		return 0;

	// get number of item which is buffered
	int num_item_buffered = this->NumBufferedItem();
	// num of item for debufferting
	int num_item_xxx = (num_item_buffered < _num_item ? num_item_buffered: _num_item);
#if 1
	if(cir_buff[cir_buff_start] != 0x47)
	{
		char tmp;
		DebufferItem(&tmp);
		//printf("DVB_T2 sync error\n");
		return -1;
#if 0
		int next_sync;
		//printf("DVB_T2 sync error\n");
		while(1)
		{
			cir_buff_start++;
			if(cir_buff_start > cir_buff_size - 1)
			{
				cir_buff_start = 0;
				next_sync = cir_buff_start + _num_item;
			}
			else
			{
				next_sync = cir_buff_start + _num_item;
				if(next_sync > cir_buff_size - 1)
				{
					next_sync = next_sync - cir_buff_size;
				}
			}

			if(cir_buff[cir_buff_start] == 0x47 && cir_buff[next_sync] == 0x47 && cir_buff[next_sync + _num_item] == 0x47)
			{
				//printf("DVB_T2 find sync\n");
				//printf("cir_buff_end[%d], cir_buff_start[%d], num_item_xxx[%d], cir_buff_size[%d]\n", cir_buff_end, cir_buff_start, num_item_xxx, cir_buff_size);
				*debugflag = 1;
				break;
			}
		}
#endif
	}
#endif	
	// not good naming ????
	int gap_start_and_last_item = num_item - cir_buff_start/item_size;

	if (num_item_xxx <= gap_start_and_last_item)
	{
			memcpy(item, &cir_buff[cir_buff_start], num_item_xxx*item_size);
			cir_buff_start = (cir_buff_start + num_item_xxx*item_size) % cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(item, &cir_buff[cir_buff_start], gap_start_and_last_item);
		cir_buff_start = (cir_buff_start + gap_start_and_last_item*item_size) % cir_buff_size;
		//ssert(cir_buff_start == 0); // pointer_end is wrapped around

		memcpy(item + gap_start_and_last_item*item_size, &cir_buff[cir_buff_start], (num_item_xxx - gap_start_and_last_item)*item_size);
		cir_buff_start = (cir_buff_start + (num_item_xxx - gap_start_and_last_item)*item_size) % cir_buff_size;
		return num_item_xxx;
	}
}


int CCircularBuffer::DebufferItem( char *item)
{

	if (cir_buff_end != cir_buff_start) {
		memcpy(item, &cir_buff[cir_buff_start], item_size);
        cir_buff_start = (cir_buff_start + item_size) % cir_buff_size;
        return 0;
	}
//otherwise, the buffer is empty; return an error code
	return -1;
}

// return size in bytes for an item
int CCircularBuffer::ItemSize(){
	return item_size;
}

int CCircularBuffer::NumItem(){
	return num_item - 1;
}
