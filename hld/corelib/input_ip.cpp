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

#include "tlv_type.h"
#include "corelib.h"
#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "mpeg2ts.h"
#include "input_ip.h"
#include "log.h"

#include "tlv_threads.h"

#define gaussian_width 50
double gaussian_values[gaussian_width];// = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//double gaussian_values[20] = {0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1, 1, 1, 1, 1, 1, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
//double gaussian_values[20] = {0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1, 1, 1, 1, 1.15, 1.1, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40};
//double gaussian_values[20] = {0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1, 1, 1, 1, 1.15, 1.1, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40};


CTLVIPServer::CTLVIPServer()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, receiving data from a ip source\n"); 

	for (int i = 0; i < gaussian_width/2; i++)
	{
		double step = 0.9 + 0.005*i;
		if ( step < 1.0)
		{
			gaussian_values[i] = step;
			gaussian_values[gaussian_width - i - 1] = 2 - step;
		}
		else
		{
			gaussian_values[i] = 1;
			gaussian_values[gaussian_width - i - 1] = 1;
		}
		//printf("Debug  [%d] %0.3f  [%d] %0.3f \n", i, gaussian_values[i], gaussian_width -i - 1, gaussian_values[gaussian_width - i - 1]);
	}
	
		
}

CTLVIPServer::~CTLVIPServer()
{

}

int CTLVIPServer::get_bitrate()
{
	return bitrate;
}

void CTLVIPServer::set_bitrate(int _bitrate)
{
	bitrate = _bitrate;
}

int CTLVIPServer::read_data(char *buff_delete, int max_num_byte)
{


	// move data to beginging of buffer 
	if (ts_buffer_start > 0)
	{
		if(sizeof(long) > 4)
		{
			char *temp_buf = (char *)malloc(ts_buffer_size + 1);
			if(temp_buf != NULL)
			{
				memcpy(&temp_buf[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
				memcpy(&ts_buffer[0], &temp_buf[0], ts_buffer_size);
				free(temp_buf);
			}
			else
				memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		}
		else
			memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < UDP_DATA_SIZE_MAX) // ambigous condition, todo: make it simple 
	{
		//int bytes = udp_server_receive(socket_id, &ts_buffer[ts_buffer_size]);
		int bytes =  rtp_server_receive(socket_id, &ts_buffer[ts_buffer_size]);
//		printf("Debug %s, %d \n", __FILE__, __LINE__);
		if (bytes <= 0) 
			return 0;
		ts_buffer_size += bytes;
	}
	int syn_pos  = ts_buffer_start;
	int packet_size = this->mpeg2ts_statistic.get_packet_size();

	// looping for sync code  
	while (ts_buffer_size > 0 && ts_buffer[syn_pos] != 0x47 )
	{ // finding sync position 
		syn_pos++;
		ts_buffer_start = syn_pos;	
		ts_buffer_size = ts_buffer_size - 1;
	} 
	// find run 
	int run = 0;
	while((ts_buffer[syn_pos + run*packet_size] == 0x47) &&  ((run + 1)*packet_size <= ts_buffer_size ))
	{
		if((run + 1)*packet_size > max_num_byte)
		{
			//printf("[Warning] ....... over-flow  \n");
			break;
		}
		run++;
	}
	if (run >  0)
	{
		tlv_mutex_lock(mutex_cir_data);
		cir_input_ts_buffer->BufferNItem(&ts_buffer[syn_pos], packet_size*run);		
		tlv_mutex_unlock(mutex_cir_data);
		ts_buffer_start = syn_pos + packet_size*run;	
		ts_buffer_size -= packet_size*run;

		received_ts_size += packet_size*run;
	}

	
	end_time = tlv_gettime_ms();
	 if (end_time - start_time > 3000)
	{
		start_time = end_time;
		cir_input_ts_buffer->Clear();
		//printf("Warning: Do not enough data in 3 seconds \n");

	} else if (received_ts_size > (int)mpeg2ts_statistic.bitrate/8/20 /* ~50 milisecond */) 
	{
		if (start_time != end_time)
		{
			int num_byte_buffered = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();
			int num_byte_buffer = cir_input_ts_buffer->NumItem()*cir_input_ts_buffer->ItemSize();
			int gau_index = num_byte_buffered*gaussian_width/num_byte_buffer;

			static unsigned int id_count = 0;

			tlv_message_t message;
			message.id = id_count;
			id_count = (id_count + 1)%255;
			
			message.bitrate = (received_ts_size*8/(end_time - start_time))*1000;
			message.bitrate = (int)(message.bitrate*gaussian_values[gau_index]);
			message.data_size_in_byte = ((received_ts_size/packet_size)*packet_size);
			tlv_mq_send(&sys_play->thread_producer, &message);
			

			received_ts_size -= message.data_size_in_byte;
			start_time = end_time;
		}	
	} 
	

	return run*packet_size;

}

int CTLVIPServer::_read_mpeg2ts_data(char *buff)
{
	int bytes = udp_server_receive(socket_id, buff);
	return bytes;
}


void CTLVIPServer::open(void *p)
{
	int rc;
	int buff_index_start = 0;
	int buff_index_last = 0;

	received_ts_size = 0;

	rc = udp_server_open(&socket_id, option.udp_port);
	if (rc != 0)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}	

	mpeg2ts_statistic.packet_size = 188;
	mpeg2ts_statistic.bitrate = sys_play->media_interface[0]->option.bitrate;
	if (mpeg2ts_statistic.bitrate < 1024*1024 || mpeg2ts_statistic.bitrate > 50*1025*1024 /* check the maximun value again */ ) 
	{
		mpeg2ts_statistic.bitrate = 19*1024*1024; 
	}


	int buffer_size  = 2*(mpeg2ts_statistic.bitrate/8); // ~2 sec of maximun bitrate, 40Mbps 	
	cir_input_ts_buffer = new CCircularBuffer(1, buffer_size);




	ts_buffer =(char *)malloc(UDP_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;


	memset(&null_packet[0], 0, 204);
	null_packet[0] = 0x47;
	null_packet[1] = 0x1F;
	null_packet[2] = 0xFF;
	null_packet[3] = 0x10;

	


	
	
	start_time = tlv_gettime_ms();
	end_time = start_time;

	// read data untill we can estimate bitrate 	
/*
	while(mpeg2ts_statistic.bitrate == 0 && sys_play->playback_control != PLAYBACK_CONTROL_END_INPUT)
	{
		int num_unbuffered_bytes = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumUnbufferedItem();
		if (num_unbuffered_bytes < 188*3)
		{
			char buff[188*3 + 10];
			cir_input_ts_buffer->DebufferNItem(&buff[0], 188*3);
			num_unbuffered_bytes = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumUnbufferedItem();
		}
		read_data(NULL, num_unbuffered_bytes);
	}
*/
	

	//printf(" === IP source: Open \n");
	//printf(" bitrate = %f \n", mpeg2ts_statistic.bitrate);
	//printf(" packet size = %d \n \n", mpeg2ts_statistic.packet_size);
	

}
int CTLVIPServer::read_n_byte(char *des, int num_byte) 
{
	return 0;
}
int CTLVIPServer::read_one_packet(char *buff)
{
	if (socket_id == 0)
		return INPUT_ERROR_SOCKET_CLOSE;

	tlv_mutex_lock(mutex_cir_data);
	int rc = cir_input_ts_buffer->DebufferNItem(buff, mpeg2ts_statistic.get_packet_size());
	tlv_mutex_unlock(mutex_cir_data);

	tlv_cond_signal(&cv_cir_in);

	return 188;	
}

void CTLVIPServer::force_close()
{
	int rc = udp_server_close(socket_id);
	if (rc != 0)
	{
		printf("erro \n");		
	}

	socket_id = 0;
}

void CTLVIPServer::close(void *arg)
{
	delete cir_input_ts_buffer;

	free(ts_buffer);
}


