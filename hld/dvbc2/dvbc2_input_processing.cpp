#ifdef WIN32
#include <Windows.h>
#else
#endif

#include <string.h>

#include "hldplay.h"
#include "media_interface.h"
#include "dvbc2_input_processing.h"
#include "CUtility.h"
#include "dvbc2_bbframe_payload.h"
#include "input_file.h"


CDVBC2InputProcessing::CDVBC2InputProcessing(){


}

CDVBC2InputProcessing::~CDVBC2InputProcessing(){
}

void CDVBC2InputProcessing::init(int _mode, CDVBC2_BBFrame_PayLoad *_payload_p)
{
	payload_p = _payload_p;
	mode = _mode;	
	// syncd buffer
	memset(&ts_syncd_buff[0], 0, 300);
	syncd = 0;

	// 
	preceding_up_crc8 = 0;	
	memset(&stuff_packet[0], 0, 204);
	stuff_packet[0] = 0x47;
	stuff_packet[1] = 0x1F;
	stuff_packet[2] = 0xFF;
	stuff_packet[3] = 0x10;
	
	accumulator_null = 0;
	accumulator_file = 0;

	mpeg2ts_null_packet_addition.init();

	bitrate = 0;
	num_packet = 0;
	num_packet_have_null = 0;


}

int CDVBC2InputProcessing::read_one_packet(char *buff)
{

	if (payload_p->media_source->option.media_source_type == PLAY_MEDIA_SOURCE_FILE || payload_p->media_source->option.media_source_type == PLAY_MEDIA_SOURCE_IP_BUFFER ||
		payload_p->media_source->option.media_source_type == PLAY_MEDIA_SOURCE_ASI_310M)
	{
		int bitrate;
		int packet_size;
		if(payload_p->media_source->option.media_source_type == PLAY_MEDIA_SOURCE_FILE)
		{
			bitrate = payload_p->media_source->ioctl(2, IO_IDVBT2_GET_BITRATE);
			//packet_size = payload_p->media_source->ioctl(2, IO_IDVBT2_GET_PACKETSIZE); //not use?
		}
		else
		{
			bitrate = payload_p->media_source->option.bitrate;
			//packet_size = 188;	//not use?
		}
		if (target_bitrate < bitrate)
		{
				int num_bytes;
				num_bytes = payload_p->media_source->read_one_packet(buff);
				//printf("Debug %s, %d  ts packet: %f, null: %f  \n", __FILE__, __LINE__,accumulator_file, accumulator_null);
				if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
				{
					return -1;
				}						

		}else {
		accumulator_null = accumulator_null + (target_bitrate - bitrate)*1.0/target_bitrate;
		accumulator_file = accumulator_file + bitrate*1.0/target_bitrate;
		// just for testing bitrate
	
		if (accumulator_null >= accumulator_file) 
		{
			memcpy(buff, &stuff_packet[0], 188);
			accumulator_null = accumulator_null - 1;

		}else // make a ts packet from file
		{

			int num_bytes;
			num_bytes = payload_p->media_source->read_one_packet(buff);
			//printf("Debug %s, %d  ts packet: %f, null: %f  \n", __FILE__, __LINE__,accumulator_file, accumulator_null);
			if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
			{
				return -1;
			}		

			accumulator_file = accumulator_file  - 1;
		}
		}
	


	}else //PLAY_MEDIA_SOURCE_IP_XXX
	{
	

		// if there is no available packets
		// waiting 
	
		int packet_size = payload_p->media_source->mpeg2ts_statistic.get_packet_size();
		if (num_packet_have_null == 0)
		{

			//printf(" Debug null %d/%d \n", num_packet, num_packet_have_null);  
			tlv_message_t message;		
			tlv_mq_receive(&payload_p->media_source->sys_play->thread_producer, &message);

			bitrate = message.bitrate;
			num_packet = message.data_size_in_byte/packet_size;

			num_packet_have_null = int((target_bitrate*1.0/bitrate)*num_packet);

			//printf("receiving bitrate %d  num byte: %d  id: %d  (%d / %d )\n", message.bitrate, message.data_size_in_byte, message.id, num_packet, num_packet_have_null);	
			//printf(" null / file (ip) = %f/ %f \n", accumulator_null, accumulator_file);  		
			
			int num_byte_buffered = payload_p->media_source->cir_input_ts_buffer->NumBufferedItem()*payload_p->media_source->cir_input_ts_buffer->ItemSize();
			int num_byte_buffer = payload_p->media_source->cir_input_ts_buffer->NumItem()*payload_p->media_source->cir_input_ts_buffer->ItemSize();
//			printf("bitrate: %d bufferred:  %d/%d \n", message.bitrate, num_byte_buffered, num_byte_buffer);



			accumulator_null = 0;
			accumulator_file = 0;
		}

		if (payload_p->media_source->sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
			return -1;


	//printf("Debug %s, %d \n", __FILE__, __LINE__);
		accumulator_null = accumulator_null + (target_bitrate - bitrate)*1.0/target_bitrate;
		accumulator_file = accumulator_file + bitrate*1.0/target_bitrate;
		// just for testing bitrate
	
		if (accumulator_null >= accumulator_file || num_packet == 0) // ugly condition 
		{
			memcpy(buff, &stuff_packet[0], 188);
			accumulator_null = accumulator_null - 1;

		}else // make a ts packet from file
		{

			int num_bytes;
			num_bytes = payload_p->media_source->read_one_packet(buff);
			//printf("Debug %s, %d  ts packet: %f, null: %f  \n", __FILE__, __LINE__,accumulator_file, accumulator_null);
			if (num_bytes == INPUT_ERROR_EOF || num_bytes == INPUT_ERROR_SOCKET_CLOSE)
			{
				return -1;
			}
			num_packet--;
			accumulator_file = accumulator_file  - 1;
		}
	}

	num_packet_have_null--;
	

	int packet_size_adaptation; 
	if (mode == DVBC2_PLP_MODE_NORMAL)
	{
		packet_size_adaptation = 188; 

		// replace sync by the preceeding_up_crc8
		buff[0] = preceding_up_crc8;
		char crc8 = 0;
		int i; 
		for (i = 1; i <= 187; i++)
		{
			unsigned char index = crc8 ^ buff[i];
			crc8 =  HCRC8::crc8_table[index];
		}
		preceding_up_crc8 = crc8;


	}else if (mode == DVBC2_PLP_MODE_HEM)
	{
		packet_size_adaptation = 187;
		if(sizeof(long) > 4)
		{
			char tmpbuf[188];
			memcpy(&tmpbuf[0], &buff[1], packet_size_adaptation);
			memcpy(&buff[0], &tmpbuf[0], packet_size_adaptation);
		}
		else
			memcpy(&buff[0], &buff[1], packet_size_adaptation);

	}	
	
	return packet_size_adaptation; 
	
}


// return number of read bytes
// note: num_bytes must be over size of a packet after adaptation
//       to ensure that one ts packet can not allocated to more than 2 BBframe

int CDVBC2InputProcessing::read_n_bytes(char *buff, int num_bytes, int *_syncd)
{

	int buff_index = 0;
	int syncd_buff_size_in_byte = syncd/8;
	int syncd_buff_index = 0;
	// read data in syncd_buff if any
	{		
		for (int i = 0; i < syncd_buff_size_in_byte && num_bytes >= syncd_buff_size_in_byte; i++)
		{
			buff[buff_index] = ts_syncd_buff[i];
			buff_index++;
		}
		// ts_syncd buffer is empty now
		*_syncd = syncd;
		syncd = 0;
	}

	int num_byte_remainder = num_bytes - syncd_buff_size_in_byte;

	if (num_byte_remainder > 0)
	{

		int AUPL = 0; // adaptation user packet len
		if (mode == DVBC2_PLP_MODE_NORMAL)
			AUPL = 188;
		else if (mode == DVBC2_PLP_MODE_HEM)
			AUPL = 187;

		int num_byte_div = num_byte_remainder/AUPL;
		int num_byte_mod = num_byte_remainder%AUPL;

		// read ts from file 
		char tmp_buff[PLAYBACK_TS_PACKET_SIZE_MAX];
		for (int i = 0; i < num_byte_div; i++)
		{
			int return_size = read_one_packet(&buff[buff_index]);			
			if (return_size < 0)
			{
			  return -1; ;		
			}
			buff_index += return_size;
		}

		syncd_buff_index = 0;
		if (num_byte_mod > 0){
			int return_size = read_one_packet(&tmp_buff[0]);
			if (return_size < 0)
			{
				
				return -1;		
			}

			for (int i = 0; i < return_size; i++){
				if (i < num_byte_mod)
				{
					buff[buff_index] =  tmp_buff[i];
					buff_index++;
				}else
				{
					ts_syncd_buff[syncd_buff_index] = tmp_buff[i];
					syncd_buff_index++;				
				}
			}
			syncd = syncd_buff_index*8;
		}
	}
	//ssert(num_bytes == buff_index);

	return buff_index;
}

// input 
// ts_packet_buff: 
// 
// return: number of byte after adaptation
//         -1 (ts packet does not start with 0x47



void CDVBC2InputProcessing::set_target_bitrate( unsigned int _target_bitrate )
{
	int bitrate = payload_p->media_source->mpeg2ts_statistic.get_bitrate();

	if (_target_bitrate > bitrate)
		target_bitrate = _target_bitrate;
	else 
		target_bitrate = bitrate;
}






////////////////////////////////////////////////////////////////////////////////////////////////////
////////// ts writer 

C2MI_TS_Writer::C2MI_TS_Writer()
{
	c2mi_get =NULL;
	c2mi_buffer =NULL;
	c2mi_index_start = 0;
	c2mi_index_last = 0;

	// syncd buffer
	pointer_field = 0;
}

C2MI_TS_Writer::~C2MI_TS_Writer()
{
}


void C2MI_TS_Writer::init( unsigned int _pid, unsigned int _packet_size )
{
	pid = _pid;
	packet_size = _packet_size;
}



int C2MI_TS_Writer::MakeTsPacket( char *ts_packet)
{

	 
	if (c2mi_get == NULL)
		return -1;

	int rc; 
	if ((c2mi_index_last - c2mi_index_start) == 0)
	{
		rc = c2mi_get->GetC2MIPacket(&c2mi_buffer, c2mi_index_last - c2mi_index_start);
		c2mi_index_start = 0; 
		c2mi_index_last = rc; 
		// new packet is added 
		c2mi_packet_start = 1; 
	}
	

	if ((c2mi_index_last - c2mi_index_start) == 0)
	{
		return -1;
	}
	int c2mi_packet_remainder= c2mi_index_last - c2mi_index_start;

	static int continuity_counter = 0;
	int ts_packet_index = 0;

	////////////////////////// TS header ///////////////////////
	C2MI_M2TS_Header m2ts_header;
	C2MI_M2TS_AdaptationField m2ts_adaptaion;
	

	m2ts_header.sync = 0x47;
	m2ts_header.transport_error_indicator = 0;
	if (c2mi_packet_start == 1)
	{
		m2ts_header.payload_unit_start_indicator = 1;
	}
	else{

		m2ts_header.payload_unit_start_indicator = 0;
	}
	m2ts_header.transport_priority = 0;
	m2ts_header.pid = pid;
	m2ts_header.scrambling_ctrl = 0;

	if (m2ts_header.payload_unit_start_indicator == 1) 
	{

		if (c2mi_packet_remainder >= packet_size /* 188 */ - 4  - 1){ // one byte for pointer field 
			m2ts_header.adaptation_field = 1; //01b: no adaptation, payload only
		}else 
		{
			m2ts_header.adaptation_field = 3; // 11b: adaptation followed by payload
		}
	}else {
		if (c2mi_packet_remainder >= packet_size /* 188 */ - 4){
			m2ts_header.adaptation_field = 1; //01b: no adaptation, payload only 			
		}else 
		{
			m2ts_header.adaptation_field = 3; // 11b: adaptation followed by payload
		}
	}
	m2ts_header.continuity_counter = continuity_counter;


	C2MI_TS_Writer::M2tsHeaderToBuff(ts_packet, m2ts_header);
	ts_packet_index = 4;


	////////////////////////// adaptaion field if any ///////////////////////
	if (m2ts_header.adaptation_field == 3 || m2ts_header.adaptation_field == 2 /* 10 */)
	{
		if (m2ts_header.payload_unit_start_indicator == 1) {		
			int adaptaion_len = 183 - c2mi_packet_remainder - 1/* pointer field */; 
			AdaptaionField(&m2ts_adaptaion, adaptaion_len);
		}else { // 
			int adaptaion_len = 183 - c2mi_packet_remainder - 0/* do not have pointer field */; 
			AdaptaionField(&m2ts_adaptaion, adaptaion_len);
		}

		ts_packet_index = 4;
		ts_packet[ts_packet_index] = m2ts_adaptaion.adaptation_field_length;
		ts_packet_index++;

		for (int i = 0; i < m2ts_adaptaion.adaptation_field_length; i++)
		{
			ts_packet[ts_packet_index] = 0x00;
			ts_packet_index++;
		}

	}

	///////////////////// payload: pointer field if any ///////////////////////////
	if (m2ts_header.payload_unit_start_indicator == 1)
	{
		ts_packet[ts_packet_index] = pointer_field;;
		ts_packet_index++;

	}

	///////////////////// payload: pointer field if any ///////////////////////////
	int payload_c2mi_len;
	int payload_c2mi_len_max;

	if (m2ts_header.adaptation_field == 3 || m2ts_header.adaptation_field == 1)
	{
		if (m2ts_header.payload_unit_start_indicator == 1){
			if (m2ts_header.adaptation_field == 3)
			{
				payload_c2mi_len = c2mi_packet_remainder;
				payload_c2mi_len_max = 184 - 1 - 1; //
			}
			else 
			{
				payload_c2mi_len = 184 - 1;
				payload_c2mi_len_max = 184 - 1;				
			}

		}else if(m2ts_header.payload_unit_start_indicator == 0)
		{

			if (m2ts_header.adaptation_field == 3)
			{
				payload_c2mi_len = c2mi_packet_remainder;
				payload_c2mi_len_max = 184 - 1;
			}
			else 
			{
				payload_c2mi_len = 184;
				payload_c2mi_len_max = 184;
			}
		}

		//
		
		//if (payload_c2mi_len_max > c2mi_packet_remainder)
		if (0)
		{
			return -1;
		}
		c2mi_packet_start = 0; 

		memcpy(&ts_packet[ts_packet_index], c2mi_buffer + c2mi_index_start, payload_c2mi_len);
		c2mi_index_start += payload_c2mi_len;
		ts_packet_index += payload_c2mi_len;
	}


	//ssert(ts_packet_index == packet_size);

	continuity_counter++;
	continuity_counter = (continuity_counter > 0xF? 0: continuity_counter);
	
	return 0;


}

void C2MI_TS_Writer::AdaptaionField(C2MI_M2TS_AdaptationField *adaptation_field, int size){
	adaptation_field->adaptation_field_length = size;
}

void C2MI_TS_Writer::M2tsHeaderToBuff(char *buff, C2MI_M2TS_Header m2ts_header){
	
	int length_in_bits = 0;
	char buff_tmp[2];

	// 8 bits 
	buff_tmp[0] = m2ts_header.sync;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 1 bit
	buff_tmp[0] = m2ts_header.transport_error_indicator << 7;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 1 bit
	buff_tmp[0] = m2ts_header.payload_unit_start_indicator << 7;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 1 bit
	buff_tmp[0] = m2ts_header.transport_priority << 7;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 13 bit
	buff_tmp[0] = m2ts_header.pid >> 5;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = m2ts_header.pid | 0X1F;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

	// 2 bit
	buff_tmp[0] = m2ts_header.scrambling_ctrl << 6;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

	// 2 bit
	buff_tmp[0] = m2ts_header.adaptation_field << 6;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

	// 4 bit
	buff_tmp[0] = m2ts_header.continuity_counter << 4;
	length_in_bits = CUtilities::AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);
}
