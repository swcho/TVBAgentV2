#include <stdio.h>
#include <malloc.h>
#include <string.h>


#include "tlv_type.h"
#include "tlv_threads.h"

#include "hldplay.h" 
#include "dvbc2_multiplexer.h"

#include "media_interface.h"

DVBC2Multiplexer::DVBC2Multiplexer()
{

	strcpy(&name[0], "multiplexer, dvbt2, muxing");
	// default value 
	dvbc2_param.BW = DVBC2_BW_8;
//	dvbt2_param.
	multiplex_option = (void *)&dvbc2_param;
	ts_maker.c2mi_get  = this;
}

DVBC2Multiplexer::~DVBC2Multiplexer()
{
}

int DVBC2Multiplexer::GetC2MIPacket(char **buff_pointer, int num_byte)
{
	c2mi_packet_status status;
	
	char buff_tmp[10*1024];

/* testing, multi-level copy 
	int rc = multiple_plp.GetNextC2MIPacket(&status, (unsigned char*)&buff_tmp[0]);
	memcpy(c2mi_buff_tmp, &buff_tmp[0], status.num_bits/8);
	memcpy(&buff_tmp[0], c2mi_buff_tmp, status.num_bits/8);
*/		

	int rc = multiple_plp.GetNextC2MIPacket(&status, (unsigned char*)c2mi_buff_tmp);
	//static FILE *file_h  = fopen("c:/ts/c2mi.data", "wb");
	//fwrite(c2mi_buff_tmp, 1, status.num_bits/8, file_h);

	if (rc < 0)
	{
		return 0;
	}
	*buff_pointer = c2mi_buff_tmp;
	return status.num_bits/8; 
}


void DVBC2Multiplexer::init()
{
	c2mi_buff_tmp = (char *)malloc(512*1024);
	c2mi_ts_buff = (char *)malloc(1024*1024 + 500);

}

void DVBC2Multiplexer::quit()
{
	free(c2mi_buff_tmp);
	free(c2mi_ts_buff);
}

int DVBC2Multiplexer::open( CHldPlayback *_play_sys )
{
	play_sys = _play_sys;	
	multiple_plp.init(dvbc2_param, this);
	int rc = multiple_plp.open();
	if (rc != 0)
	{
		//printf("====== %s, %d  \n", __FILE__, __LINE__);
		multiple_plp.quit();
		return -1;
	}

	int PLP_NUM_BLOCKS = multiple_plp.PLP_NUM_BLOCKS_TOTAL;	
	ts_maker.init(dvbc2_param.PID, 188);
	
	return 0;

}

void DVBC2Multiplexer::close()
{
	multiple_plp.close();
	multiple_plp.quit();
}

int DVBC2Multiplexer::produce_item( char *buff, int size )
{
	
	int buff_size = 0;

	while(buff_size + 188 <= size/*1MB*/)
	{
		int rtcd = ts_maker.MakeTsPacket((char *)&c2mi_ts_buff[buff_size]);
		if (rtcd == -1)
		{
			return -1; 
		}else
		{
			buff_size += 188;
		}
		
		if (play_sys->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
			return -1;
	}

	////////////////////////////////////////////////////////////////////
	// add to multiplexed buffer 
	tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
	play_sys->cir_multiplexed_data_buffer->BufferNItem((char *) &c2mi_ts_buff[0], buff_size);
	int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
	tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
	if (b_bytes >= 1024*1024)
		tlv_cond_signal(&play_sys->cv_multiplexed_data_out);

	return buff_size;
}


void DVBC2Multiplexer::seek( int percentage )
{

}

int DVBC2Multiplexer::ioctl(int message_type, void *param)
{
	return 0;
}


double DVBC2Multiplexer::get_bitrate()
{
	return multiple_plp.CalculateTSBitrate();
}

// -----------------------------------

DVBC2Reader::DVBC2Reader()
{
	strcpy(&name[0], "multiplexer, dvbc2, no muxing");
}

DVBC2Reader::~DVBC2Reader()
{

}

void DVBC2Reader::init()
{



}

void DVBC2Reader::quit()
{
}

int DVBC2Reader::open(CHldPlayback *_play_sys)
{
	ts_buff = (char *)malloc(1024*1024 + 300);
	play_sys = _play_sys;
	media_source = play_sys->media_interface[0];
	return 0;
	
}



void DVBC2Reader::close(){
	free(ts_buff);
}


int DVBC2Reader::produce_item( char *buff, int size )  
{
	
	int buff_size = 0; 
	while(buff_size + 512*1024 <= size /*1MB*/)
	{
		int rc = media_source->read_n_byte(&ts_buff[buff_size], 512*1024);
		if (rc == INPUT_ERROR_EOF || rc == INPUT_ERROR_SOCKET_CLOSE)
			return -1; // eof
		else {
			buff_size += rc;
		}

	}

	////////////////////////////////////////////////////////////////////
	// add to multiplexed buffer 
	tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
	play_sys->cir_multiplexed_data_buffer->BufferNItem((char *)&ts_buff[0], buff_size);
	int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
	tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
	if (b_bytes >= 1024*1024)
		tlv_cond_signal(&play_sys->cv_multiplexed_data_out);
	
	return buff_size;

}

void DVBC2Reader::seek(int percentage)
{
}

int DVBC2Reader::ioctl( int message_type, void *param )
{
	switch (message_type)
	{
		case PLAY_IO_CONTROL_SEEK:
			long *seek_pos = (long *)param;
			break;
	}
	return 0;
}

