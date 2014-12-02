#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ts_mux.h"
#include "ts_mux_sys.h"


#include "utility.h"






static void AdaptaionField(M2TS_AdaptationField *adaptation_field, int size){
	adaptation_field->adaptation_field_length = size;
}

static void M2tsHeaderToBuff(char *buff, M2TS_Header m2ts_header){

	int length_in_bits = 0;
	char buff_tmp[2];

	// 8 bits 
	buff_tmp[0] = m2ts_header.sync;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 1 bit
	buff_tmp[0] = m2ts_header.transport_error_indicator << 7;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 1 bit
	buff_tmp[0] = m2ts_header.payload_unit_start_indicator << 7;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 1 bit
	buff_tmp[0] = m2ts_header.transport_priority << 7;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

	// 13 bit
	buff_tmp[0] = m2ts_header.pid >> 5;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = m2ts_header.pid | 0X1F;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

	// 2 bit
	buff_tmp[0] = m2ts_header.scrambling_ctrl << 6;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

	// 2 bit
	buff_tmp[0] = m2ts_header.adaptation_field << 6;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

	// 4 bit
	buff_tmp[0] = m2ts_header.continuity_counter << 4;
        length_in_bits = AddBitToBuffer((unsigned char *)buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);
}




ts_mux_handle_t ts_mux_init(f_get_payload_data_t callback_func, void *callback_param)
{

    ts_mux_t *handle = (ts_mux_t *)malloc(sizeof(ts_mux_t));
	
    handle->callback_func = callback_func;
    handle->callback_param = callback_param;

    handle->pid = 4096;
    handle->packet_size = 188;

    handle->t2mi_buffer = (char *)malloc(1024*1024);
    handle->t2mi_index_start = 0;
    handle->t2mi_index_last = 0;

    // syncd buffer
    handle->pointer_field = 0;

    handle->continuity_counter = 0;

    return (ts_mux_handle_t)handle;
}


int ts_mux_quit(ts_mux_handle_t* handle)
{
        ts_mux_t *p_ts_mux = (ts_mux_t *)*handle;
        free(p_ts_mux->t2mi_buffer);
	free(*handle);
	*handle = NULL;

	return 0;
}


int ts_mux_set_pid(ts_mux_handle_t handle, int _pid){
        ts_mux_t *p_ts_mux = (ts_mux_t *)handle;
        p_ts_mux->pid = _pid;
	return 0;
}

int ts_mux_set_packet_size(ts_mux_handle_t handle, int packet_size)
{
        ts_mux_t *p_ts_mux = (ts_mux_t *)handle;
        p_ts_mux->packet_size = packet_size;
	return 0;
}

int ts_mux_get_a_packet(ts_mux_handle_t handle, char *ts_packet)
{

	int rc; 
	int ts_packet_index = 0;
    int i;
	int t2mi_packet_remainder;
	M2TS_Header m2ts_header;
	M2TS_AdaptationField m2ts_adaptaion;
	int payload_t2mi_len;
	int payload_t2mi_len_max;

    ts_mux_t *p_ts_mux = (ts_mux_t *)handle;

    if ((p_ts_mux->t2mi_index_last - p_ts_mux->t2mi_index_start) == 0)
	{
        rc = p_ts_mux->callback_func(p_ts_mux->t2mi_buffer, p_ts_mux->t2mi_index_last - p_ts_mux->t2mi_index_start, p_ts_mux->callback_param);

		if (rc == TS_MUX_EOF)
			return TS_MUX_EOF;

                p_ts_mux->t2mi_index_start = 0;
                p_ts_mux->t2mi_index_last = rc;
		// new packet is added 
                p_ts_mux->t2mi_packet_start = 1;
	}
	
	//printf("Debug %s, %d \n", __FILE__, __LINE__);

/*
        if ((p_ts_mux->t2mi_index_last - p_ts_mux->t2mi_index_start) == 0)
	{
		return -1;
	}
*/
    t2mi_packet_remainder= p_ts_mux->t2mi_index_last - p_ts_mux->t2mi_index_start;

	

	////////////////////////// TS header ///////////////////////

	m2ts_header.sync = 0x47;
	m2ts_header.transport_error_indicator = 0;
        if (p_ts_mux->t2mi_packet_start == 1)
	{
		m2ts_header.payload_unit_start_indicator = 1;
	}
	else{

		m2ts_header.payload_unit_start_indicator = 0;
	}
	m2ts_header.transport_priority = 0;
        m2ts_header.pid = p_ts_mux->pid;
	m2ts_header.scrambling_ctrl = 0;

	if (m2ts_header.payload_unit_start_indicator == 1) 
	{

                if (t2mi_packet_remainder >= p_ts_mux->packet_size /* 188 */ - 4  - 1){ // one byte for pointer field
			m2ts_header.adaptation_field = 1; //01b: no adaptation, payload only
		}else 
		{
			m2ts_header.adaptation_field = 3; // 11b: adaptation followed by payload
		}
	}else {
                if (t2mi_packet_remainder >= p_ts_mux->packet_size /* 188 */ - 4){
			m2ts_header.adaptation_field = 1; //01b: no adaptation, payload only 			
		}else 
		{
			m2ts_header.adaptation_field = 3; // 11b: adaptation followed by payload
		}
	}
        m2ts_header.continuity_counter = p_ts_mux->continuity_counter;


        M2tsHeaderToBuff(ts_packet, m2ts_header);
	ts_packet_index = 4;


	////////////////////////// adaptaion field if any ///////////////////////
	if (m2ts_header.adaptation_field == 3 || m2ts_header.adaptation_field == 2 /* 10 */)
	{
		if (m2ts_header.payload_unit_start_indicator == 1) {		
			int adaptaion_len = 183 - t2mi_packet_remainder - 1/* pointer field */; 
			AdaptaionField(&m2ts_adaptaion, adaptaion_len);
		}else { // 
			int adaptaion_len = 183 - t2mi_packet_remainder - 0/* do not have pointer field */; 
			AdaptaionField(&m2ts_adaptaion, adaptaion_len);
		}

		ts_packet_index = 4;
                ts_packet[ts_packet_index] = m2ts_adaptaion.adaptation_field_length;
		ts_packet_index++;

                for (i = 0; i < m2ts_adaptaion.adaptation_field_length; i++)
		{
                        ts_packet[ts_packet_index] = 0x00;
			ts_packet_index++;
		}

	}

	///////////////////// payload: pointer field if any ///////////////////////////
	if (m2ts_header.payload_unit_start_indicator == 1)
	{
                ts_packet[ts_packet_index] = p_ts_mux->pointer_field;;
		ts_packet_index++;

	}

	///////////////////// payload: pointer field if any ///////////////////////////

	if (m2ts_header.adaptation_field == 3 || m2ts_header.adaptation_field == 1)
	{
		if (m2ts_header.payload_unit_start_indicator == 1){
			if (m2ts_header.adaptation_field == 3)
			{
				payload_t2mi_len = t2mi_packet_remainder;
				payload_t2mi_len_max = 184 - 1 - 1; //
			}
			else 
			{
				payload_t2mi_len = 184 - 1;
				payload_t2mi_len_max = 184 - 1;				
			}

		}else if(m2ts_header.payload_unit_start_indicator == 0)
		{

			if (m2ts_header.adaptation_field == 3)
			{
				payload_t2mi_len = t2mi_packet_remainder;
				payload_t2mi_len_max = 184 - 1;
			}
			else 
			{
				payload_t2mi_len = 184;
				payload_t2mi_len_max = 184;
			}
		}

		//

                p_ts_mux->t2mi_packet_start = 0;

                memcpy(&ts_packet[ts_packet_index], p_ts_mux->t2mi_buffer + p_ts_mux->t2mi_index_start, payload_t2mi_len);
                p_ts_mux->t2mi_index_start += payload_t2mi_len;
		ts_packet_index += payload_t2mi_len;
	}

        p_ts_mux->continuity_counter++;
        p_ts_mux->continuity_counter = (p_ts_mux->continuity_counter > 0xF? 0: p_ts_mux->continuity_counter);
	
	return 0;

}


