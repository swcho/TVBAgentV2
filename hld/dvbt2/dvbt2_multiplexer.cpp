#include <stdio.h>
#include <malloc.h>
#include <string.h>
//#include <assert.h>


#include "tlv_type.h"
#include "tlv_threads.h"

#include "hldplay.h" 
#include "dvbt2_multiplexer.h"

#include "media_interface.h"

#include "input_file.h"

// callback function of t2mi mux 
int get_one_packet(void *owner, int index, char *buff, void *param)
{
	CHldPlayback *play_sys = (CHldPlayback*)param;
	DVBT2Multiplexer *mux = (DVBT2Multiplexer *)play_sys->multiplex_interface;
	int i;

	
	if(play_sys->media_interface[index]->option.media_source_type == PLAY_MEDIA_SOURCE_FILE)
	{
		int bitrate;
		int target_bitrate = mux->dvbt2_param.list_plp[index].UP_BITRATE;
		
		if(play_sys->media_interface[index]->option.media_source_type == PLAY_MEDIA_SOURCE_FILE)
		{
			bitrate = play_sys->media_interface[index]->ioctl(2, IO_IDVBT2_GET_BITRATE);
		}
		else
		{
			bitrate = play_sys->media_interface[index]->option.bitrate;
			//packet_size = 188;	//not use?
		}

		if ( target_bitrate < bitrate)
		{
			int num_bytes;
			num_bytes = play_sys->media_interface[index]->read_one_packet(buff);
			//printf("Debug %s, %d  ts packet: %f, null: %f  \n", __FILE__, __LINE__,accumulator_file, accumulator_null);
			if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
			{
				return -1;
			}						
			
		}else {
			mux->remuxs[index].accumulator_null = mux->remuxs[index].accumulator_null + (target_bitrate - bitrate)*1.0/target_bitrate;
			mux->remuxs[index].accumulator_file = mux->remuxs[index].accumulator_file + bitrate*1.0/target_bitrate;
			// just for testing bitrate
			
			for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
			{
				if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
				{
					break;
				}
				else
				{
					mux->remuxs[index].pcrRestamp[i].uPcrTotalPacket++;
				}
			}

			if (mux->remuxs[index].accumulator_null >= mux->remuxs[index].accumulator_file) 
			{
				memcpy(buff, &mux->stuff_packet[0], 188);
				mux->remuxs[index].accumulator_null = mux->remuxs[index].accumulator_null - 1;
				//printf("Debug ts bitrate: %d, target: %d  \n", bitrate, target_bitrate);
				
			}else // make a ts packet from file
			{			

				for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
				{
					if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
					{
						break;
					}
					else
					{
						mux->remuxs[index].pcrRestamp[i].uPcrOriginalTsPacket++;
					}
				}

				int num_bytes;
				num_bytes = play_sys->media_interface[index]->read_one_packet(buff);
				//printf("Debug ts packet: %f, null: %f  \n", mux->remuxs[index].accumulator_file, mux->remuxs[index].accumulator_null);
				if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
				{
					return -1;
				}		
				mux->remuxs[index].accumulator_file = mux->remuxs[index].accumulator_file  - 1;
				if(buff[1] & 0x80)
				{// TEI: transport error indicator
					return 188;
				}
				if (!((buff[3] & 0x20) == 0x20))
				{// do not have adaptation
					return 188;
				}
				if (buff[4] == 0)
				{// adaptation len = 0
					return 188;
				}
				if ((buff[5] & 0x10) == 0x10)
				{
					int pid = ((buff[1] & 0x1F) << 8) + (buff[2] & 0xFF);
					for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
					{
						if(mux->remuxs[index].pcrRestamp[i].nPid == pid)
						{
							mux->_pcr_correction(&mux->remuxs[index].pcrRestamp[i], (unsigned char *)buff, target_bitrate, bitrate);
							break;
						}
						if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
						{
							mux->remuxs[index].pcrRestamp[i].nPid = pid; 
							break;
						}
					}
				}
			}
		}
	}
	else if(play_sys->media_interface[index]->option.media_source_type == PLAY_MEDIA_SOURCE_IP_BUFFER ||
		play_sys->media_interface[index]->option.media_source_type == PLAY_MEDIA_SOURCE_ASI_310M)
	{
		int bitrate;
		int target_bitrate = mux->dvbt2_param.list_plp[index].UP_BITRATE;
		
		if(play_sys->media_interface[index]->option.media_source_type == PLAY_MEDIA_SOURCE_FILE)
		{
			bitrate = play_sys->media_interface[index]->ioctl(2, IO_IDVBT2_GET_BITRATE);
		}
		else
		{
			bitrate = play_sys->media_interface[index]->option.bitrate;
			//packet_size = 188;	//not use?
		}

		if ( target_bitrate < bitrate)
		{
			int num_bytes;
			num_bytes = play_sys->media_interface[index]->read_one_packet(buff);
			//printf("Debug %s, %d  ts packet: %f, null: %f  \n", __FILE__, __LINE__,accumulator_file, accumulator_null);
			if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
			{
				return -1;
			}						
			
		}else {
			mux->remuxs[index].accumulator_null = mux->remuxs[index].accumulator_null + (target_bitrate - bitrate)*1.0/target_bitrate;
			mux->remuxs[index].accumulator_file = mux->remuxs[index].accumulator_file + bitrate*1.0/target_bitrate;
			// just for testing bitrate
#if 0			
			for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
			{
				if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
				{
					break;
				}
				else
				{
					mux->remuxs[index].pcrRestamp[i].uPcrTotalPacket++;
				}
			}
#endif
			if (mux->remuxs[index].accumulator_null >= mux->remuxs[index].accumulator_file) 
			{
				memcpy(buff, &mux->stuff_packet[0], 188);
				mux->remuxs[index].accumulator_null = mux->remuxs[index].accumulator_null - 1;
				//printf("Debug ts bitrate: %d, target: %d  \n", bitrate, target_bitrate);
				
			}else // make a ts packet from file
			{			
#if 0
				for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
				{
					if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
					{
						break;
					}
					else
					{
						mux->remuxs[index].pcrRestamp[i].uPcrOriginalTsPacket++;
					}
				}
#endif
				int num_bytes;
				num_bytes = play_sys->media_interface[index]->read_one_packet(buff);
				//printf("Debug ts packet: %f, null: %f  \n", mux->remuxs[index].accumulator_file, mux->remuxs[index].accumulator_null);
				if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
				{
					return -1;
				}		
				mux->remuxs[index].accumulator_file = mux->remuxs[index].accumulator_file  - 1;
#if 0
				if(buff[1] & 0x80)
				{// TEI: transport error indicator
					return 188;
				}
				if (!((buff[3] & 0x20) == 0x20))
				{// do not have adaptation
					return 188;
				}
				if (buff[4] == 0)
				{// adaptation len = 0
					return 188;
				}
				if ((buff[5] & 0x10) == 0x10)
				{
					int pid = ((buff[1] & 0x1F) << 8) + (buff[2] & 0xFF);
					for(i = 0; i < DEMUX_MPEG2TS_MAX_PID; i++)
					{
						if(mux->remuxs[index].pcrRestamp[i].nPid == pid)
						{
							mux->_pcr_correction(&mux->remuxs[index].pcrRestamp[i], (unsigned char *)buff, target_bitrate, bitrate);
							break;
						}
						if(mux->remuxs[index].pcrRestamp[i].nPid == -1)
						{
							mux->remuxs[index].pcrRestamp[i].nPid = pid; 
							break;
						}
					}
				}
#endif
			}
		}
	}
	return 188;
}
void DVBT2Multiplexer::_pcr_correction(pcr_info_t *pcrInfo, unsigned char* packet, int out_bitrate, int in_bitrate)
{
	__int64 h1, l1, l2, l3, l4;
	__int64 pcr_base_orig, cur_pcr, i_pcr;
	unsigned short int h, l;
	unsigned short int pcr_ext_orig;
	double n1_b1, n0_b0;
	__int64 clock_reference_base, clock_reference_ext;

    h1 = packet[6];
    l1 = packet[7];
    l2 = packet[8];
    l3 = packet[9];
    l4 = (packet[10] & 0x80);
	pcr_base_orig = (h1 << 25) |	(l1 << 17) | (l2 << 9) | (l3 << 1) | (l4 >> 7);
    h = (unsigned short int)(packet[10] & 0x01);
    l =(unsigned short int) packet[11];
	pcr_ext_orig = (h << 8) | l;

	cur_pcr = (uint64_t)(pcr_base_orig*300) + (uint64_t)pcr_ext_orig;

	n1_b1 = pcrInfo->uPcrTotalPacket*188*8*1.0/out_bitrate;
	n0_b0 = pcrInfo->uPcrOriginalTsPacket*188*8*1.0/in_bitrate;
	pcrInfo->pcrCompensation += (int64_t)(( n1_b1 - n0_b0)*27000000);
	i_pcr = cur_pcr + pcrInfo->pcrCompensation;

	clock_reference_base = i_pcr/300;
	clock_reference_ext = i_pcr%300;

    // set base to pcr value
    packet[6] = (unsigned char)(clock_reference_base >> 25);
    packet[7] = (unsigned char)((clock_reference_base >> 17)  & 0x00000000000000FF);
    packet[8] = (unsigned char)((clock_reference_base >> 9)	& 0x00000000000000FF);
    packet[9] = (unsigned char)((clock_reference_base >> 1)	& 0x00000000000000FF);
    packet[10] = (unsigned char) ((clock_reference_base	& 0x00000000000000FF) << 7);

    // set extension to pcr value
    packet[10] = packet[10] | (unsigned char)(clock_reference_ext >> 8);
    packet[11] = (unsigned char)(clock_reference_ext & 0x00000000000000FF);

	pcrInfo->uPcrTotalPacket = 0;
	pcrInfo->uPcrOriginalTsPacket = 0;
}
// callback function of ts_mux 
int get_t2mi_packet(char *buff, int max, void *param)
{
	
	t2mi_handle_t *handle = (t2mi_handle_t *)param;
	
	struct t2mi_packet_status status;
	t2mi_get_t2_packet(handle, &status, (unsigned char *) buff);
	return status.num_bits/8;
}



DVBT2Multiplexer::DVBT2Multiplexer()
{

	strcpy(&name[0], "multiplexer, dvbt2, muxing");
	// default value 
	dvbt2_param.BW = DVBT2_BW_8;
//	dvbt2_param.
	multiplex_option = (void *)&dvbt2_param;
}

DVBT2Multiplexer::~DVBT2Multiplexer()
{
}

int DVBT2Multiplexer::GetT2MIPacket(char **buff_pointer, int num_byte)
{
	t2mi_packet_status status;
	
	char buff_tmp[10*1024];

	*buff_pointer = t2mi_buff_tmp;
	return status.num_bits/8; 
}


void DVBT2Multiplexer::init()
{
	t2mi_buff_tmp = (char *)malloc(512*1024);
	t2mi_ts_buff = (char *)malloc(1024*1024 + 500);

	for (int i= 0; i < 10; i++)
	{	
		remuxs[i].accumulator_null = 0;
		remuxs[i].accumulator_file = 0;
		for(int j = 0;	j < DEMUX_MPEG2TS_MAX_PID; j++)
		{
			remuxs[i].pcrRestamp[j].pcrCompensation = 0;
			remuxs[i].pcrRestamp[j].uPcrOriginalTsPacket = 0;
			remuxs[i].pcrRestamp[j].uPcrTotalPacket = 0;
			remuxs[i].pcrRestamp[j].nPid = -1;
		}
	}

	memset(&stuff_packet[0], 0, 204);
	stuff_packet[0] = 0x47;
	stuff_packet[1] = 0x1F;
	stuff_packet[2] = 0xFF;
	stuff_packet[3] = 0x10;




}

void DVBT2Multiplexer::quit()
{
	free(t2mi_buff_tmp);
	free(t2mi_ts_buff);
	
}

int DVBT2Multiplexer::open( CHldPlayback *_play_sys )
{
	play_sys = _play_sys;	
	int i;

	DVBT2_PARAM_t t2_param; 
	multiple_plp_handle = t2mi_init(get_one_packet, play_sys);
	// get and set parameters
	t2mi_get_param(multiple_plp_handle, &t2_param);



#if 1 
	// transfer param from
    // general
    t2_param.BW = dvbt2_param.BW;
    t2_param.FFT_SIZE = dvbt2_param.FFT_SIZE;
    t2_param.GUARD_INTERVAL = dvbt2_param.GUARD_INTERVAL;
    t2_param.L1_MOD = dvbt2_param.L1_MOD;
    t2_param.Pilot = dvbt2_param.Pilot;
    t2_param.BW_EXT = dvbt2_param.BW_EXT;
	
    // frame
    t2_param.NUM_T2_FRAME = dvbt2_param.NUM_T2_FRAME;
    t2_param.NUM_DATA_SYMBOLS = dvbt2_param.NUM_DATA_SYMBOLS;
	t2_param.auto_searching_data_symbol_num_block = 0;

	
    // PLP
    t2_param.num_plp = dvbt2_param.num_plp;
	for (i = 0; i < t2_param.num_plp; i++)
	{
		t2_param.list_plp[i].PLP_ID = dvbt2_param.list_plp[i].PLP_ID;
		t2_param.list_plp[i].PLP_MOD = dvbt2_param.list_plp[i].PLP_MOD;
		t2_param.list_plp[i].PLP_COD = dvbt2_param.list_plp[i].PLP_COD;
		t2_param.list_plp[i].PLP_FEC = dvbt2_param.list_plp[i].PLP_FEC;
		t2_param.list_plp[i].PLP_ROTATION = dvbt2_param.list_plp[i].PLP_ROTATION;
		t2_param.list_plp[i].PLP_NUM_BLOCKS = dvbt2_param.list_plp[i].PLP_NUM_BLOCKS;
		t2_param.list_plp[i].PLP_TIME_IL_TYPE = dvbt2_param.list_plp[i].PLP_TIME_IL_TYPE;
		t2_param.list_plp[i].PLP_TIME_IL_LENGTH = dvbt2_param.list_plp[i].PLP_TIME_IL_LENGTH;
		t2_param.list_plp[i].PLP_HEM = dvbt2_param.list_plp[i].PLP_HEM;
		t2_param.list_plp[i].ISSY = dvbt2_param.list_plp[i].ISSY;    
	}
    t2_param.FREQUENCY = dvbt2_param.FREQUENCY;


#else // test

#define TEST_CASE 74

#if (TEST_CASE == 74)

	t2_param.BW = DVBT2_BW_8;
    t2_param.BW_EXT = DVBT2_BWT_NORMAL;
    t2_param.Cell_ID = 0;
    t2_param.FFT_SIZE = DVBT2_FFT_SIZE_32K;
    t2_param.FREQUENCY = 474000000;
    t2_param.GUARD_INTERVAL = DVBT2_GUARD_INTERVAL_19_128;
    t2_param.L1_MOD = DVBT2_MOD_QPSK;
    t2_param.L1_REPETITION = 0;
    t2_param.L1x_LEN = 0; // recompute
    t2_param.NETWORK_ID = 12421;
	
    t2_param.PAPR = 0;
    t2_param.PID = 4096;
    t2_param.Pilot = DVBT2_PILOT_PATTERN_PP2;
    t2_param.S1 = 0; // ???
    t2_param.T2_ID = 32796;
    t2_param.auto_searching_data_symbol_num_block = 0;
	
	
    // frame configuration
    t2_param.NUM_DATA_SYMBOLS = 59;
    t2_param.NUM_T2_FRAME = 2;
	
	
    t2_param.num_plp = 1;
    t2_param.list_plp[0].PLP_MOD = DVBT2_PLP_MOD_QPSK;
    t2_param.list_plp[0].PLP_COD = DVBT2_PLP_COD_2_3;
    t2_param.list_plp[0].PLP_FEC = DVBT2_PLP_FEC_TYPE_64K_LDPC;
    t2_param.list_plp[0].PLP_HEM = DVBT2_PLP_MODE_HEM;
    t2_param.list_plp[0].PLP_ROTATION = DVBT2_PLP_ROT_ON;
    t2_param.list_plp[0].PLP_ID = 0;
    t2_param.list_plp[0].PLP_NUM_BLOCKS = 45;
	t2_param.list_plp[0].TIME_IL_LENGTH = 3;
    t2_param.list_plp[0].PLP_BITRATE = 10350404;

#elif (TEST_CASE == DTG0076)
	// 
	
	t2_param.BW = DVBT2_BW_8;
    t2_param.BW_EXT = DVBT2_BWT_NORMAL;
    t2_param.Cell_ID = 0;
    t2_param.FFT_SIZE = DVBT2_FFT_SIZE_32K;
    t2_param.FREQUENCY = 474000000;
    t2_param.GUARD_INTERVAL = DVBT2_GUARD_INTERVAL_1_8;
    t2_param.L1_MOD = DVBT2_MOD_QAM64;
    t2_param.L1_REPETITION = 0;
    t2_param.L1x_LEN = 0; // recompute
    t2_param.NETWORK_ID = 12421;
	
    t2_param.PAPR = 0;
    t2_param.PID = 4096;
    t2_param.Pilot = DVBT2_PILOT_PATTERN_PP2;
    t2_param.S1 = 0; // ???
    t2_param.T2_ID = 32796;
    t2_param.auto_searching_data_symbol_num_block = 0;
	
	
    // frame configuration
    t2_param.NUM_DATA_SYMBOLS = 59;
    t2_param.NUM_T2_FRAME = 2;
	
	
    t2_param.num_plp = 1;
    t2_param.list_plp[0].PLP_MOD = DVBT2_PLP_MOD_256QAM;
    t2_param.list_plp[0].PLP_COD = DVBT2_PLP_COD_3_5;
    t2_param.list_plp[0].PLP_FEC = DVBT2_PLP_FEC_TYPE_64K_LDPC;
    t2_param.list_plp[0].PLP_HEM = DVBT2_PLP_MODE_NORMAL;
    t2_param.list_plp[0].PLP_ROTATION = DVBT2_PLP_ROT_ON;
    t2_param.list_plp[0].PLP_ID = 0;
    t2_param.list_plp[0].PLP_NUM_BLOCKS = 183;
	t2_param.list_plp[0].TIME_IL_LENGTH = 3;
    t2_param.list_plp[0].PLP_BITRATE = 10350404;


#endif
	
#endif
	
	t2mi_set_param(multiple_plp_handle, &t2_param);
//	tv_dvbt2_print_param(t2_param);

	dvbt2_param = t2_param;

	// ts_mux
	ts_mux = ts_mux_init(get_t2mi_packet, multiple_plp_handle);
	return 0;

}

void DVBT2Multiplexer::close()
{
	ts_mux_quit(&ts_mux);

	t2mi_quit(multiple_plp_handle);

}

int DVBT2Multiplexer::produce_item( char *buff_tmtm, int size )
{
	
	// huy: comment 
	int buff_size = 0;

	while(buff_size + 188 <= size/*1MB*/)
	{
		int rtcd = ts_mux_get_a_packet(ts_mux, (char *)&t2mi_ts_buff[buff_size]);
		if (rtcd == TS_MUX_EOF)
		{
			return -1; 
		}else
		{
			buff_size += 188;
		}
		
		if (play_sys->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
			return -1;
	}

	//printf("Debug %s, %d \n", __FILE__, __LINE__);
	////////////////////////////////////////////////////////////////////
	// add to multiplexed buffer 
	tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
	play_sys->cir_multiplexed_data_buffer->BufferNItem((char *) &t2mi_ts_buff[0], buff_size);
	int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
	tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
	if (b_bytes >= 1024*1024)
		tlv_cond_signal(&play_sys->cv_multiplexed_data_out);

	return buff_size;

}


void DVBT2Multiplexer::seek( int percentage )
{

}

int DVBT2Multiplexer::ioctl(int message_type, void *param)
{
	return 0;
}


double DVBT2Multiplexer::get_bitrate()
{
	// huy: comment 
	//return multiple_plp.CalculateTSBitrate();
	return 0;
}

// -----------------------------------

DVBT2Reader::DVBT2Reader()
{
	strcpy(&name[0], "multiplexer, dvbt2, no muxing");
}

DVBT2Reader::~DVBT2Reader()
{

}

void DVBT2Reader::init()
{



}

void DVBT2Reader::quit()
{
}

int DVBT2Reader::open(CHldPlayback *_play_sys)
{
	ts_buff = (char *)malloc(1024*1024 + 300);
	play_sys = _play_sys;
	media_source = play_sys->media_interface[0];
	return 0;
	
}



void DVBT2Reader::close(){
	free(ts_buff);
}


int DVBT2Reader::produce_item( char *buff, int size )  
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

void DVBT2Reader::seek(int percentage)
{
}

int DVBT2Reader::ioctl( int message_type, void *param )
{
	switch (message_type)
	{
		case PLAY_IO_CONTROL_SEEK:
			long *seek_pos = (long *)param;
			break;
	}
	return 0;
}

