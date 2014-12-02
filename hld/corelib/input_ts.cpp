//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "tlv_type.h"
#include "corelib.h"
#include "hldplay.h"
#include "media_interface.h"
#include "input_ts.h"
#include "tlv_threads.h"





static int _mpeg2ts_detect_packet_size(char *ts_buff, int size, int credit)
{
	int	is_packet_size_detected;
	int packet_size[4] = {188, 192, 204, 208};
	int j;
	for (j = 0; j < 4; j++)
	{
		int i;
		is_packet_size_detected = 1;		
		for (i = 0; i < credit; i++)
		{
			if ((i + 1)*packet_size[j] > size || ts_buff[i*packet_size[j]] != 0x47)
			{
				is_packet_size_detected = 0;
				break;
			}
		}

		if (is_packet_size_detected == 1)
			return packet_size[j];
	}

	return -1; // can not detect packet size

}



static int _find_syn_pos(char *buff, int size)
{
	int i = 0;
	int packet_size = 188;
	while(i + 3*204< size)
	{
		packet_size = 188;
		if (*(buff + i) == 0x47 && *(buff + packet_size + i) == 0x47)
		{
			return i;			
		}

		packet_size = 196;
		if (*(buff + i) == 0x47 && *(buff + packet_size + i) == 0x47)
		{
			return i;
		}

		packet_size = 204;
		if (*(buff + i) == 0x47 && *(buff + packet_size + i) == 0x47)
		{
			return i;
		}

		i++;
		
	}
	return  -1;
}



int callback_function(handle _m2_handle, int max_packet_num, void *cb_data)
{	
	MTSRemux *mux = (MTSRemux *)cb_data;

	// read data 
	static int i = 0;
	int syn_pos;
	int rc;
	int read_size;
	int num_push_bytes;

	//printf("[Debug] %d: index: %d, max size: %d \n", __LINE__, i++, max_size);

_READ_:
	read_size = fread(mux->ts_packet + mux->buf_end, 1, 1024*1024 - mux->buf_end, mux->file_handler);
	if (read_size == 0)
	{ 
		fseek(mux->file_handler, 0, SEEK_SET);
		read_size = fread(mux->ts_packet + mux->buf_end, 1, 1024*1024 - mux->buf_end, mux->file_handler);

	if ((mux->sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		{
			fseek(mux->file_handler, 0, SEEK_SET);

			tlv_sleep(2*1000); // wait for playing all multiplexed data
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&mux->sys_play->thread_observer, &message);
			// in linux 

			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&mux->sys_play->thread_observer, &message);
		}


		//return REMUX_EOF; // eof
	}
	mux->buf_end = mux->buf_end + read_size;
	
	syn_pos = _find_syn_pos(&mux->ts_packet[mux->buf_start], mux->buf_end);
	if (syn_pos >= 0) {
		mux->buf_start = syn_pos;
	}else
	{
		mux->buf_start = 0;
		mux->buf_end = 0;
		goto _READ_;
	}

	// if packet size is not detected,
	if (mux->packet_size < 188)
	{
		mux->packet_size = _mpeg2ts_detect_packet_size(mux->ts_packet + mux->buf_start, mux->buf_end - mux->buf_start + 1, 3);
		if (mux->packet_size < 188)
			mux->packet_size = 188; //default
	}

	if (mux->packet_size == 188)
	{
		// find run
		int run = 0;
		do
		{
			if (mux->ts_packet[mux->buf_start + run*mux->packet_size] != 0x47)
			{
				printf("[Warning] Syn lost \n");
				break;
			}
			run++;
		}while ((run + 1) < max_packet_num && (run + 1)*mux->packet_size < mux->buf_end - mux->buf_start + 1);

		if (run > 0)
		{
			rc = remux_mpeg2ts_push_ts_packet(_m2_handle, &mux->ts_packet[mux->buf_start] , run);
			mux->buf_start += 188*rc;
		}

	}else
	{
		char *buf_tmp = (char *)malloc(max_packet_num*188 + 100);
		int run = 0;
		do
		{
			if (mux->ts_packet[mux->buf_start] != 0x47)
			{
				printf("[Warning] Syn lost \n");
				break;
			}
			memcpy(buf_tmp + run*188, mux->ts_packet + mux->buf_start, 188);
			mux->buf_start += mux->packet_size;
			run++;
		}while ((run + 1) < max_packet_num && mux->packet_size < mux->buf_end - mux->buf_start + 1);

		if (run > 0)
		{
			rc = remux_mpeg2ts_push_ts_packet(_m2_handle, buf_tmp , run);
		}

		free(buf_tmp);

	}
	
	if (mux->buf_start != 0)
	{
		memcpy(mux->ts_packet, mux->ts_packet + mux->buf_start, mux->buf_end - mux->buf_start + 1);
		mux->buf_end = mux->buf_end - mux->buf_start;
		mux->buf_start = 0;
	}

	return 0;
}





MTSRemux::MTSRemux()
{
	strcpy(&name[0], "input, support (isdb-t ,mux)");
	mpeg2ts_remux_handle = remux_mpeg2ts_init(callback_function, this);

}

MTSRemux::~MTSRemux()
{
	remux_mpeg2ts_quit(mpeg2ts_remux_handle);
}


void MTSRemux::force_close()
{
	is_closed = 1;
}

int MTSRemux::read_data(char *buff, int max_num_byte)
{
	// get data ... 
	int size = 0;
	char ts_tmp[300];


	while (size + 188 <= max_num_byte)
	{
		int rc = remux_mpeg2ts_get_ts_packet(mpeg2ts_remux_handle, ts_tmp);
	
		tlv_mutex_lock(mutex_cir_data);
		cir_input_ts_buffer->BufferNItem(ts_tmp, 188);
		tlv_mutex_unlock(mutex_cir_data);		

		size += 188;
		if (rc == REMUX_EOF)
			return -1;
	}
	tlv_cond_signal(&cv_cir_out);
	return 0;
}


// ?? useless function
int MTSRemux::_read_mpeg2ts_data(char *buff)
{
	return 0; // 
	
}

void MTSRemux::open(void *_path_file)
{
	char *path_file = (char *)_path_file;
	tlv_message_t message;
	file_handler = fopen(path_file, "rb");
	assert(file_handler != NULL);

	ts_packet = (char *)malloc(2*1024*1024);
	buf_end = 0;
	buf_start = 0;

	packet_size = -1;

	if(file_handler == NULL)
	{
		// observer thread 
		//tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	is_closed = 0;	
	
	cir_input_ts_buffer = new CCircularBuffer(1, (300000/8)/4 /* 0.25 sec */ );

	// notify to out-side 
	message.id = PLAYBACK_USER_STATE_START_STREAMING; 
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);
}


void MTSRemux::close(void *arg)
{
	free(ts_packet);
	delete cir_input_ts_buffer;
	fclose(file_handler);

	
}

int MTSRemux::read_n_byte(char *des, int num_byte)
{
	return -1; // not support 
}
int MTSRemux::read_one_packet(char *buff)
{

#if 0
	int rc = fread(buff, 1, 188, file_handler_test);
	if (rc < 188)
	{
		fseek(file_handler_test, 0, SEEK_SET);
		fread(buff, 1, 188, file_handler_test);
	}

	return 188;

#else

	if (is_closed)
		return INPUT_ERROR_EOF;
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < 188)
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			
			tlv_mutex_unlock(mutex_cir_data);
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, 188);
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}


#endif
}


int MTSRemux::ioctl(int count, ... )
{
	va_list list_member; 	
	va_start(list_member, count);
	int request = va_arg(list_member, int);

	switch (request)
	{
	case IO_REQUEST_REMUX_OUT_BITRATE:
		{
		int out_bitrate; 
		out_bitrate = va_arg(list_member, int);
		remux_mpeg2ts_set_out_bitrate(mpeg2ts_remux_handle, out_bitrate);
		}
		break;
	case IO_REQUEST_ADD_PID_FOR_TS_REMUX:
		{
		int pid; 
		pid = va_arg(list_member, int);
		remux_mpeg2ts_add_pid(mpeg2ts_remux_handle, pid);
		}
		break;

	default:
		break;
	}

	return 0;

}
