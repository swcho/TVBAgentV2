//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************
#ifdef WIN32
#else
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>

#include "tlv_type.h"
#include "corelib.h"
#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "mpeg2ts.h"
#include "media_interface.h"
#include "input_file.h"
#include "tlv_threads.h"

#ifndef _NO_IP_BUFFER_
#include "LLDWrapper.h"
#endif

CTLVFileFake::CTLVFileFake()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, fake for testing purpose");
}

CTLVFileFake::~CTLVFileFake()
{
	
}


void CTLVFileFake::force_close()
{
	is_closed = 1;
}

int CTLVFileFake::read_data(char *buff_unuse, int max_num_byte)
{
	return max_num_byte;
}

int CTLVFileFake::_read_mpeg2ts_data(char *buff)
{
	return 0;
}

void CTLVFileFake::open(void *_path_file)
{
}
void CTLVFileFake::close(void *arg)
{
}

// not yet support 
int CTLVFileFake::read_n_byte(char *des, int num_byte)
{
	return num_byte;
}

int CTLVFileFake::read_one_packet(char *buff)
{
	return 188;
}



CTLVFile::CTLVFile()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, suport (isdb-t 1seg, mux)");
}

CTLVFile::~CTLVFile()
{
	
}


void CTLVFile::force_close()
{
	is_closed = 1;
}

int CTLVFile::read_data(char *buff_unuse, int max_num_byte)
{
	
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
	if (feof(file_handler) != 0)
	{
		fseek(file_handler, 0, SEEK_SET);
		// noooppppppp 
		tlv_message_t message;
		message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
		// in linux 
		
		message.id = PLAYBACK_USER_STATE_START_STREAMING; 
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);

		//printf(" ========================EOF\n");

	}
	// move data to beginging of buffer 
	if (ts_buffer_start > 0)
	{
		memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int syn_pos  = ts_buffer_start;
	int num_added_bytes = 0;
	int packet_size = this->mpeg2ts_statistic.get_packet_size();
	if (mpeg2ts_statistic.num_filter_pids >= 1) 
	{
		while (num_added_bytes + packet_size  <= max_num_byte  && packet_size <= ts_buffer_size)
		{
			if (ts_buffer[syn_pos] != 0x47)
			{
				syn_pos++;
				// notify sync lost 
				ts_buffer_start = syn_pos;
				ts_buffer_size -= 1;
			} else 
			{
				mpeg2ts_header ts_header;
				MPEG2TSStatisticAndFilter::ReadTSHeader((unsigned char *)&ts_buffer[syn_pos], &ts_header);
				if (mpeg2ts_statistic.isFilterPID(ts_header.pid) == 0)
				{
					goto __DISCARD_TS_PACKET;
				}

				// get one ts packet 
				tlv_mutex_lock(mutex_cir_data);
				cir_input_ts_buffer->BufferNItem(&ts_buffer[syn_pos], packet_size);
				tlv_mutex_unlock(mutex_cir_data);
				num_added_bytes += packet_size;

__DISCARD_TS_PACKET:
				syn_pos += packet_size;
				ts_buffer_start = syn_pos;
				ts_buffer_size -= packet_size;
			}
		}

	}else 
	{
		
		// looping for sync code  
		while (ts_buffer_size > 0 && ts_buffer[syn_pos] != 0x47 )
		{ // finding sync position 
			syn_pos++;
			ts_buffer_start = syn_pos;	
			ts_buffer_size = ts_buffer_size - 1;
			printf("warning packet lost %s , buffer_size: %d  char code: %d, syn pos: %d  \n", __FILE__, ts_buffer_size, ts_buffer[syn_pos], syn_pos);
			// notify sync lost 
		} 
		// find run 
		int run = 0;
		while((ts_buffer[syn_pos + run*packet_size] == 0x47) &&  ((run + 1)*packet_size <= ts_buffer_size ))
		{
			if((run + 1)*packet_size > max_num_byte)
				break;
			run++;
		}
		tlv_mutex_lock(mutex_cir_data);
		if (run >  0)
		{
			cir_input_ts_buffer->BufferNItem(&ts_buffer[syn_pos], packet_size*run);
			ts_buffer_start = syn_pos + packet_size*run;	
			ts_buffer_size -= packet_size*run;
		}

		int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
		tlv_mutex_unlock(mutex_cir_data);

		if (num_unbuffered_bytes < packet_size)
			tlv_cond_signal(&cv_cir_out);
		num_added_bytes = run*packet_size;

	}

	return num_added_bytes;
}

int CTLVFile::_read_mpeg2ts_data(char *buff)
{
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 

			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);

		}
	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
	return bytes;
}

void CTLVFile::open(void *_path_file)
{

	// print verbose information

	//printf(" ========= plug-in : %s  ============ \n", &name[0]);
	//printf(" file:  %s  ============ \n", _path_file);

	int rc;
	char *buff = (char *) malloc( FILE_READ_DATA_SIZE_MAX*2);
	int buff_index_start = 0;
	int buff_index_last = 0;

	char *path_file = (char *)_path_file;	
	file_handler = fopen(path_file, "rb");
	if(file_handler == NULL)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	is_closed = 0;	

	CCircularBuffer *cir_buffer_tmp = new CCircularBuffer(1, 30*1024*1024);

	// Reading information from input TS, to estimate packet_size, and bitrate 
	// just support constant bitrate ???
	while(thread_handle->thread_state == THREAD_STATE_READING_INPUT)
	{
		if (sys_play->playback_control == PLAYBACK_CONTROL_END_INPUT)
			break;

		if (buff_index_last > 0)
		{
			memcpy(&buff[0], &buff[buff_index_start], buff_index_last - buff_index_start);
			buff_index_last = buff_index_last - buff_index_start;
			buff_index_start = 0;

		}

		int bytes = this->_read_mpeg2ts_data(&buff[buff_index_last]);
		if (bytes == -1)
			break; 
		buff_index_last += bytes;

		// for estimating packet_size
		int packet_size = mpeg2ts_statistic.get_packet_size();
		if ( packet_size < 188)
		{
			int syn_pos = buff_index_start;	
			while (syn_pos < buff_index_last)
			{
				if (buff[syn_pos] == 0x47)
				{
					mpeg2ts_statistic.EstimateTSPacketSize(&buff[syn_pos], buff_index_last - syn_pos, 3);	
					packet_size = mpeg2ts_statistic.get_packet_size();
					if (packet_size >= 188)
						break;
				}
				syn_pos++;
			}

		}


		int syn_pos  = buff_index_start;
		while (syn_pos + packet_size < buff_index_last )
		{
			if (buff[syn_pos] != 0x47)
			{
				syn_pos++;					
			} else {					
				int num_unbuffered_bytes = cir_buffer_tmp->NumUnbufferedItem()*cir_buffer_tmp->ItemSize();
				if (num_unbuffered_bytes < packet_size) // full 
				{					
					// remove the first one 
					char tmp[300];
					cir_buffer_tmp->DebufferNItem(&tmp[0], packet_size);
				}
				// 1. filtering if any 
				if (mpeg2ts_statistic.num_filter_pids >= 1) 
				{
					mpeg2ts_header ts_header;
					MPEG2TSStatisticAndFilter::ReadTSHeader((unsigned char *)&buff[syn_pos], &ts_header);
					if (mpeg2ts_statistic.isFilterPID(ts_header.pid) == 0)
					{
						goto __DISCARD_TS_PACKET_1;
					}
				}
				mpeg2ts_statistic.TSStatistic((unsigned char *)&buff[syn_pos]);
				// get one ts packet 
				tlv_mutex_lock(mutex_cir_data);
				if (buff[syn_pos] != 0x47)
				{
					// observer thread 
					tlv_message_t message;
					message.id = PLAYBACK_LOG_MESSAGE;
					message.param = NULL;
					tlv_mq_send(&sys_play->thread_observer, &message);
				}

				rc = cir_buffer_tmp->BufferNItem(&buff[syn_pos], packet_size);
				tlv_mutex_unlock(mutex_cir_data);
__DISCARD_TS_PACKET_1:
				syn_pos += packet_size;
			}

			buff_index_start = syn_pos;			
			mpeg2ts_statistic.EstimateBitrate();

			int num_buffered_bytes = cir_buffer_tmp->NumBufferedItem()*cir_buffer_tmp->ItemSize();
			int bitrate = (int)(mpeg2ts_statistic.get_bitrate()*188/packet_size);
			if (num_buffered_bytes >= mpeg2ts_statistic.get_bitrate()*4/8 && num_buffered_bytes > 300*1024*10/8 /*waiting for about 10 second */ && bitrate > 0 )
			{			
				//printf("==== %s, %d, %d, \n", __FILE__, __LINE__, bitrate);
				goto _END_;
			}

		}
	}

_END_: // packet size, and bitrate is detected 
	delete cir_buffer_tmp;
	cir_input_ts_buffer = new CCircularBuffer(1, (int)((mpeg2ts_statistic.get_bitrate()*0.5)/8));

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	fseek(file_handler, 0, SEEK_SET);
	free(buff);

	tlv_message_t message;

	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);



}
void CTLVFile::close(void *arg)
{
	delete cir_input_ts_buffer;
	fclose(file_handler);
	free(ts_buffer);
}

// not yet support 
int CTLVFile::read_n_byte(char *des, int num_byte)
{
	int num_bytes_buffered;

	if (is_closed)
		return INPUT_ERROR_EOF;

_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();		
		if (num_bytes_buffered < num_byte)
		{ // not enough data for reading 

			// signal for reading data 
			tlv_cond_signal(&cv_cir_in);

			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(des, num_byte);  
		tlv_mutex_unlock(mutex_cir_data);
		return rc;
	}

}

int CTLVFile::read_one_packet(char *buff)
{
	if (is_closed)
	{
		return INPUT_ERROR_EOF;
	}

_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
	{
		return INPUT_ERROR_EOF;
	}
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();		
		if (num_bytes_buffered < mpeg2ts_statistic.get_packet_size())
		{ // not enough data for reading 

			// signal for reading data 
			tlv_cond_signal(&cv_cir_in);

			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, mpeg2ts_statistic.get_packet_size());
		//sys_play->plld->_FIf->CheckAdaptation((unsigned char *)buff, mpeg2ts_statistic.get_packet_size());
		tlv_mutex_unlock(mutex_cir_data);
		return rc;
	}
}

CTLVFileDVBT2Mul::CTLVFileDVBT2Mul()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, support (dvb-t2, muxing )");
	//debug_fp = fopen("/home/televiewuser/TS/dvbT2TestRaw.ts", "wb");
	debug_fp = NULL;
}

CTLVFileDVBT2Mul::~CTLVFileDVBT2Mul()
{
	if(debug_fp != NULL)
		fclose(debug_fp);
	free(tempT2buf);
}




void CTLVFileDVBT2Mul::open(void *_path_file)
{

	// print verbose information

	//printf(" ========= plug-in : %s  ============ \n", &name[0]);
	//printf(" file:  %s  ============ \n", _path_file);

	int rc;
	char *buff = (char *) malloc( FILE_READ_DATA_SIZE_MAX*2);
	int buff_index_start = 0;
	int buff_index_last = 0;

	char *path_file = (char *)_path_file;	
	file_handler = fopen(path_file, "rb");
	if(file_handler == NULL)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	is_closed = 0;	

	// Reading information from input TS, to estimate packet_size, and bitrate 
	// just support constant bitrate ???
	while(thread_handle->thread_state == THREAD_STATE_READING_INPUT)
	{
		if (sys_play->playback_control == PLAYBACK_CONTROL_END_INPUT)
			break;

		if (buff_index_last > 0)
		{
			memcpy(&buff[0], &buff[buff_index_start], buff_index_last - buff_index_start);
			buff_index_last = buff_index_last - buff_index_start;
			buff_index_start = 0;

		}

		int bytes = this->_read_mpeg2ts_data(&buff[buff_index_last]);
		if (bytes == -1)
			break; 
		buff_index_last += bytes;

		// for estimating packet_size
		int packet_size  = -1;
		if ( packet_size < 188)
		{
			int syn_pos = buff_index_start;	
			while (syn_pos < buff_index_last)
			{
				if (buff[syn_pos] == 0x47)
				{
					mpeg2ts_statistic.EstimateTSPacketSize(&buff[syn_pos], buff_index_last - syn_pos, 3);	
					packet_size = mpeg2ts_statistic.get_packet_size();
					if (packet_size >= 188)
						goto _END_;
				}
				syn_pos++;
			}

		}

		
	}

_END_: // packet size

	cir_input_ts_buffer = new CCircularBuffer(1, (in_bitrate/8)/2);
	free(buff);

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	tempT2buf =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	fseek(file_handler, 0, SEEK_SET);
	

	tlv_message_t message;

	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);



}


int CTLVFileDVBT2Mul::read_data(char *buff_unuse, int max_num_byte)
{
	
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
	if (feof(file_handler) != 0)
	{
		fseek(file_handler, 0, SEEK_SET);

	}
	// move data to beginning of buffer 
	if (ts_buffer_start > 0)
	{
		if((ts_buffer_start + ts_buffer_size) >= (FILE_READ_DATA_SIZE_MAX * 2))
printf("memory over\n");
		memcpy(&tempT2buf[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		memcpy(&ts_buffer[0], &tempT2buf[0], ts_buffer_size);
		//memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int syn_pos  = ts_buffer_start;
	int num_added_bytes = 0;
	int packet_size = this->mpeg2ts_statistic.get_packet_size();
		
		// looping for sync code  
		//2012/5/31
		//while (ts_buffer_size > 0 && ts_buffer[syn_pos] != 0x47 )
		while(ts_buffer_size > 0)
		{ 
			// finding sync position 
			if( ts_buffer[syn_pos] == 0x47 && ts_buffer[syn_pos + packet_size] == 0x47 && ts_buffer[syn_pos + packet_size * 2] == 0x47)
				break;
			syn_pos++;
			ts_buffer_start = syn_pos;	
			ts_buffer_size = ts_buffer_size - 1;
			printf("warning packet lost %s , buffer_size: %d  char code: %d, syn pos: %d  \n", __FILE__, ts_buffer_size, ts_buffer[syn_pos], syn_pos);
			// notify sync lost 
		} 
		// find run 
		int run = 0;
		while((ts_buffer[syn_pos + run*packet_size] == 0x47) &&  ((run + 1)*packet_size <= ts_buffer_size ))
		{
			if((run + 1)*packet_size > max_num_byte)
				break;
			run++;
		}
		tlv_mutex_lock(mutex_cir_data);
		if (run >  0)
		{
//printf("[DVBT2 DEBUG] syn_pos [%d], run [%d], packet_size [%d]\n", syn_pos, run,packet_size);
			cir_input_ts_buffer->BufferNItem(&ts_buffer[syn_pos], packet_size*run);
			if(debug_fp != NULL)
				fwrite(&ts_buffer[syn_pos], 1, packet_size*run, debug_fp);

			ts_buffer_start = syn_pos + packet_size*run;	
			ts_buffer_size -= packet_size*run;
		}

		int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
		tlv_mutex_unlock(mutex_cir_data);

		if (num_unbuffered_bytes < packet_size)
			tlv_cond_signal(&cv_cir_out);
		num_added_bytes = run*packet_size;

	return num_added_bytes;
}


int CTLVFileDVBT2Mul::ioctl(int count, ... )
{

	va_list list_member; 	
	va_start(list_member, count);
	int request = va_arg(list_member, int);
	
	switch (request)
	{
	case IO_IDVBT2_SET_BITRATE:
		in_bitrate = va_arg(list_member, int);
		break;
	case IO_IDVBT2_GET_BITRATE:
		return in_bitrate;
	case IO_IDVBT2_SET_PACKETSIZE:
		//packet_size = va_arg(list_member, int);
		break;
	case IO_IDVBT2_GET_PACKETSIZE:
		return mpeg2ts_statistic.get_packet_size();
	default:
		break;
	}
	return 0;
	
}




int CTLVFileDVBT2Mul::_read_mpeg2ts_data(char *buff)
{
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
		}
	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX, file_handler);		
	return bytes;
}


CTLVFileDVBT2::CTLVFileDVBT2()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, support (dvb-t2, no muxing (just play-out .t2 file))");
}

CTLVFileDVBT2::~CTLVFileDVBT2()
{

}


void CTLVFileDVBT2::force_close()
{
	is_closed = 1;
}

int CTLVFileDVBT2::read_data(char *buff, int max_num_byte)
{	
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 

			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);

		}
	// move data to beginging of buffer 
	if (ts_buffer_start > 0)
	{
		memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int num_byte = (max_num_byte > ts_buffer_size? ts_buffer_size: max_num_byte);
	tlv_mutex_lock(mutex_cir_data);
	if (num_byte >  0)
	{
		cir_input_ts_buffer->BufferNItem(&ts_buffer[0], num_byte); 
		ts_buffer_start = num_byte;	
		ts_buffer_size -= num_byte;
	}

	int num_buffered_bytes = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();
	int packet_size = mpeg2ts_statistic.get_packet_size();
	int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
	tlv_mutex_unlock(mutex_cir_data);
	if (num_unbuffered_bytes < packet_size)
	{
		tlv_cond_signal(&cv_cir_out);
	}


	return num_byte;

}
int CTLVFileDVBT2::_read_mpeg2ts_data(char *buff)
{

	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 
			// sleep api is to pause the current process for a mount of time 
			//leep(100);
			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
		}

	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX, file_handler);		

	// 
	return bytes;
}
void CTLVFileDVBT2::open(void *_path_file)
{
	char *path_file = (char *)_path_file;	
	file_handler = fopen(path_file, "rb");
	if (file_handler == NULL)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}
	is_closed = 0;	

	mpeg2ts_statistic.bitrate = 24000000; 
	mpeg2ts_statistic.packet_size = 188;

	int buffering_size = (int)mpeg2ts_statistic.get_bitrate()*0.5/8;

	cir_input_ts_buffer = new CCircularBuffer(1, buffering_size);

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	// send start playing status to user
	tlv_message_t message;
	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);

}
void CTLVFileDVBT2::close(void *arg)
{
	delete cir_input_ts_buffer;
	fclose(file_handler);
	free(ts_buffer);
}

int CTLVFileDVBT2::read_n_byte(char *des, int num_byte)
{
	int num_bytes_buffered = 0;
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		int packet_size = mpeg2ts_statistic.get_packet_size();
		tlv_mutex_lock(mutex_cir_data);
		num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < packet_size) 
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			//printf("====  multiplexing waiting [need data]%s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
			// waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			//Sleep(10);
			goto _WAITING_;	
		}
		int num_byte_read = (num_bytes_buffered < num_byte ? num_bytes_buffered: num_byte);

		int rc = cir_input_ts_buffer->DebufferNItem(des, num_byte_read);
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}
	
}
int CTLVFileDVBT2::read_one_packet(char *buff)
{
	int packet_size = mpeg2ts_statistic.get_packet_size();
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < packet_size) 
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, packet_size);
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}

}




CTLVFileISDBT::CTLVFileISDBT()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, support (isdb-t 1seg, no mux)");
}

CTLVFileISDBT::~CTLVFileISDBT()
{

}


void CTLVFileISDBT::force_close()
{
	is_closed = 1;
}

int CTLVFileISDBT::read_data(char *buff, int max_num_byte)
{

	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 
			// sleep api is to pause the current process for a mount of time 
			//leep(100);
			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);

		}
	if (ts_buffer_start > 0)
	{
		memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int num_added_bytes = (ts_buffer_size > max_num_byte? max_num_byte: ts_buffer_size);

	tlv_mutex_lock(mutex_cir_data);
	cir_input_ts_buffer->BufferNItem(&ts_buffer[0], num_added_bytes);
	ts_buffer_size -= num_added_bytes;
	ts_buffer_start += num_added_bytes;

	int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
	tlv_mutex_unlock(mutex_cir_data);
	int packet_size = mpeg2ts_statistic.get_packet_size(); 
	if (num_unbuffered_bytes < packet_size)
	{
		tlv_cond_signal(&cv_cir_out);
	}
	return num_added_bytes;
}
int CTLVFileISDBT::_read_mpeg2ts_data(char *buff)
{
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 
			// sleep api is to pause the current process for a mount of time 
			//leep(100);
			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);

		}
	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX, file_handler);	
	return bytes;
}

void CTLVFileISDBT::open(void *_path_file)
{
	char *path_file = (char *)_path_file;
	tlv_message_t message;
	file_handler = fopen(path_file, "rb");
	if(file_handler == NULL)
	{
		// observer thread 
		//tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	is_closed = 0;	

	mpeg2ts_statistic.bitrate = 32000000; // default
	mpeg2ts_statistic.packet_size = 204;

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	cir_input_ts_buffer = new CCircularBuffer(1, (int)(mpeg2ts_statistic.get_bitrate()*0.5/8));

	// notify to out-side 
	message.id = PLAYBACK_USER_STATE_START_STREAMING; 
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);
}


void CTLVFileISDBT::close(void *arg)
{
	delete cir_input_ts_buffer;
	fclose(file_handler);
	free(ts_buffer);
}

int CTLVFileISDBT::read_n_byte(char *des, int num_byte)
{
	int num_bytes_buffered = 0;

	if (is_closed)
		return INPUT_ERROR_EOF;
	int packet_size;
	packet_size = mpeg2ts_statistic.get_packet_size();
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < packet_size)
		{ // not enough data for reading 

			// signal for reading data 
			tlv_cond_signal(&cv_cir_in);

			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int num_byte_read = (num_bytes_buffered < num_byte ? num_bytes_buffered: num_byte);

		int rc = cir_input_ts_buffer->DebufferNItem(des, num_byte_read);  
		tlv_mutex_unlock(mutex_cir_data);
		return rc;
	}
}
int CTLVFileISDBT::read_one_packet(char *buff)
{

	if (is_closed)
		return INPUT_ERROR_EOF;
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < mpeg2ts_statistic.get_packet_size())
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, mpeg2ts_statistic.get_packet_size());
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}
}


CTLVFileDVBC2Mul::CTLVFileDVBC2Mul()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, support (dvb-c2, muxing )");
}

CTLVFileDVBC2Mul::~CTLVFileDVBC2Mul()
{

}
void CTLVFileDVBC2Mul::open(void *_path_file)
{

	// print verbose information

	//printf(" ========= plug-in : %s  ============ \n", &name[0]);
	//printf(" file:  %s  ============ \n", _path_file);

	int rc;
	char *buff = (char *) malloc( FILE_READ_DATA_SIZE_MAX*2);
	int buff_index_start = 0;
	int buff_index_last = 0;

	char *path_file = (char *)_path_file;	
	file_handler = fopen(path_file, "rb");
	if(file_handler == NULL)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	is_closed = 0;	

	// Reading information from input TS, to estimate packet_size, and bitrate 
	// just support constant bitrate ???
	while(thread_handle->thread_state == THREAD_STATE_READING_INPUT)
	{
		if (sys_play->playback_control == PLAYBACK_CONTROL_END_INPUT)
			break;

		if (buff_index_last > 0)
		{
			memcpy(&buff[0], &buff[buff_index_start], buff_index_last - buff_index_start);
			buff_index_last = buff_index_last - buff_index_start;
			buff_index_start = 0;

		}

		int bytes = this->_read_mpeg2ts_data(&buff[buff_index_last]);
		if (bytes == -1)
			break; 
		buff_index_last += bytes;

		// for estimating packet_size
		int packet_size  = -1;
		if ( packet_size < 188)
		{
			int syn_pos = buff_index_start;	
			while (syn_pos < buff_index_last)
			{
				if (buff[syn_pos] == 0x47)
				{
					mpeg2ts_statistic.EstimateTSPacketSize(&buff[syn_pos], buff_index_last - syn_pos, 3);	
					packet_size = mpeg2ts_statistic.get_packet_size();
					if (packet_size >= 188)
						goto _END_;
				}
				syn_pos++;
			}

		}

		
	}

_END_: // packet size

	cir_input_ts_buffer = new CCircularBuffer(1, (in_bitrate/8)/2);
	free(buff);

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	fseek(file_handler, 0, SEEK_SET);
	

	tlv_message_t message;

	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);



}



int CTLVFileDVBC2Mul::read_data(char *buff_unuse, int max_num_byte)
{
	
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
	if (feof(file_handler) != 0)
	{
		fseek(file_handler, 0, SEEK_SET);

	}
	// move data to beginging of buffer 
	if (ts_buffer_start > 0)
	{
		memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int syn_pos  = ts_buffer_start;
	int num_added_bytes = 0;
	int packet_size = this->mpeg2ts_statistic.get_packet_size();
		// looping for sync code  
		//2012/5/31
		//while (ts_buffer_size > 0 && ts_buffer[syn_pos] != 0x47 )
		while(ts_buffer_size > 0)
		{ 
			// finding sync position 
			if( ts_buffer[syn_pos] == 0x47 && ts_buffer[syn_pos + packet_size] == 0x47 && ts_buffer[syn_pos + packet_size * 2] == 0x47)
				break;
			syn_pos++;
			ts_buffer_start = syn_pos;	
			ts_buffer_size = ts_buffer_size - 1;
			//printf("warning packet lost %s , buffer_size: %d  char code: %d, syn pos: %d  \n", __FILE__, ts_buffer_size, ts_buffer[syn_pos], syn_pos);
			// notify sync lost 
		} 
		// find run 
		int run = 0;
		while((ts_buffer[syn_pos + run*packet_size] == 0x47) &&  ((run + 1)*packet_size <= ts_buffer_size ))
		{
			if((run + 1)*packet_size > max_num_byte)
				break;
			run++;
		}
		tlv_mutex_lock(mutex_cir_data);
		if (run >  0)
		{
			cir_input_ts_buffer->BufferNItem(&ts_buffer[syn_pos], packet_size*run);

			ts_buffer_start = syn_pos + packet_size*run;	
			ts_buffer_size -= packet_size*run;
		}

		int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
		tlv_mutex_unlock(mutex_cir_data);

		if (num_unbuffered_bytes < packet_size)
			tlv_cond_signal(&cv_cir_out);
		num_added_bytes = run*packet_size;

	return num_added_bytes;
}
int CTLVFileDVBC2Mul::ioctl(int count, ... )
{

	va_list list_member; 	
	va_start(list_member, count);
	int request = va_arg(list_member, int);
	
	switch (request)
	{
	case IO_IDVBT2_SET_BITRATE:
		in_bitrate = va_arg(list_member, int);
		break;
	case IO_IDVBT2_GET_BITRATE:
		return in_bitrate;
	case IO_IDVBT2_SET_PACKETSIZE:
		//packet_size = va_arg(list_member, int);
		break;
	case IO_IDVBT2_GET_PACKETSIZE:
		return mpeg2ts_statistic.get_packet_size();
	default:
		break;
	}
	return 0;
	
}

int CTLVFileDVBC2Mul::_read_mpeg2ts_data(char *buff)
{
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
		}
	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX, file_handler);		
	return bytes;
}


CTLVFileDVBC2::CTLVFileDVBC2()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, support (dvb-c2, no muxing (just play-out .c2 file))");
}

CTLVFileDVBC2::~CTLVFileDVBC2()
{

}


void CTLVFileDVBC2::force_close()
{
	is_closed = 1;
}

int CTLVFileDVBC2::read_data(char *buff, int max_num_byte)
{	
	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 

			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);

		}
	// move data to beginging of buffer 
	if (ts_buffer_start > 0)
	{
		memcpy(&ts_buffer[0], &ts_buffer[ts_buffer_start], ts_buffer_size);
		ts_buffer_start = 0;
	}
	if (ts_buffer_size < FILE_READ_DATA_SIZE_MAX)
	{
		int bytes = fread(&ts_buffer[ts_buffer_size], 1, FILE_READ_DATA_SIZE_MAX , file_handler);		
		ts_buffer_size += bytes;
	}
	int num_byte = (max_num_byte > ts_buffer_size? ts_buffer_size: max_num_byte);
	tlv_mutex_lock(mutex_cir_data);
	if (num_byte >  0)
	{
		cir_input_ts_buffer->BufferNItem(&ts_buffer[0], num_byte); 
		ts_buffer_start = num_byte;	
		ts_buffer_size -= num_byte;
	}
	int num_buffered_bytes = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();
	int packet_size = mpeg2ts_statistic.get_packet_size();
	int num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
	tlv_mutex_unlock(mutex_cir_data);
	if (num_unbuffered_bytes < packet_size)
	{
		tlv_cond_signal(&cv_cir_out);
	}
	return num_byte;

}
int CTLVFileDVBC2::_read_mpeg2ts_data(char *buff)
{

	if ((sys_play->play_mode & PLAY_MODE_REPEAT) == PLAY_MODE_REPEAT)
		if (feof(file_handler))
		{
			fseek(file_handler, 0, SEEK_SET);
			// noooppppppp 
			tlv_message_t message;
			message.id = PLAYBACK_USER_STATE_STOP_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
			// in linux 
			// sleep api is to pause the current process for a mount of time 
			//leep(100);
			message.id = PLAYBACK_USER_STATE_START_STREAMING; 
			message.param = NULL;
			tlv_mq_send(&sys_play->thread_observer, &message);
		}

	int bytes = fread(buff, 1, FILE_READ_DATA_SIZE_MAX, file_handler);		

	// 
	return bytes;
}
void CTLVFileDVBC2::open(void *_path_file)
{
	char *path_file = (char *)_path_file;	
	file_handler = fopen(path_file, "rb");
	if (file_handler == NULL)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}
	is_closed = 0;	

	mpeg2ts_statistic.bitrate = 24000000; 
	mpeg2ts_statistic.packet_size = 188;

	int buffering_size = (int)mpeg2ts_statistic.get_bitrate()*0.5/8;

	cir_input_ts_buffer = new CCircularBuffer(1, buffering_size);

	ts_buffer =(char *)malloc(FILE_READ_DATA_SIZE_MAX*2);
	ts_buffer_start = 0;
	ts_buffer_size = 0;

	// send start playing status to user
	tlv_message_t message;
	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);

}
void CTLVFileDVBC2::close(void *arg)
{
	delete cir_input_ts_buffer;
	fclose(file_handler);
	free(ts_buffer);
}

int CTLVFileDVBC2::read_n_byte(char *des, int num_byte)
{
	int num_bytes_buffered = 0;
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		int packet_size = mpeg2ts_statistic.get_packet_size();
		tlv_mutex_lock(mutex_cir_data);
		num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < packet_size) 
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			//printf("====  multiplexing waiting [need data]%s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
			// waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int num_byte_read = (num_bytes_buffered < num_byte ? num_bytes_buffered: num_byte);

		//printf("==== multipelxing releasing %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
		int rc = cir_input_ts_buffer->DebufferNItem(des, num_byte_read);
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}
}
int CTLVFileDVBC2::read_one_packet(char *buff)
{
	int packet_size = mpeg2ts_statistic.get_packet_size();
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < packet_size) 
		{
			// signal 
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, packet_size);
		tlv_mutex_unlock(mutex_cir_data);
		return rc;	
	}
}



