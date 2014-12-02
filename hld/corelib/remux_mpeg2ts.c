#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "remux_mpeg2ts.h"


static FILE *file = NULL;

static cir_buf_t* cir_buf_init(int _item_size, int _num_item)
{
	//file = fopen("C:/ts/test/ts_remux_out.ts", "wb");
	cir_buf_t *_cir_buf = (cir_buf_t *)malloc(sizeof(cir_buf_t));
	_cir_buf->item_size = _item_size;
	_cir_buf->num_item = _num_item;
	_cir_buf->cir_buff_size = (_num_item + 1)*_item_size;
	_cir_buf->cir_buff = (char *) malloc(_cir_buf->cir_buff_size);
	_cir_buf->cir_buff_start = 0;
	_cir_buf->cir_buff_end = 0;

	return _cir_buf;
	
}

static void cir_buf_quit(cir_buf_t* _handle)
{
	if (file != NULL)
		fclose(file);
	if (_handle->cir_buff != NULL)
	{	
//		while(1);
		free(_handle->cir_buff);		
	}	
	free(_handle);
}


static int cir_buf_is_full(cir_buf_t* _handle)
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if ((_handle->cir_buff_end + _handle->item_size) % _handle->cir_buff_size != _handle->cir_buff_start)
		return 0;
	else
		return 1;

}

static int cir_buf_is_empty(cir_buf_t* _handle)
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if (_handle->cir_buff_end != _handle->cir_buff_start)
		return 0; // not empty
	else
		return 1; // empty


}


static int cir_buf_buffer_item(cir_buf_t* _handle, char *item)
{
	if (_handle->cir_buff == NULL)
		return -1;
	
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if ((_handle->cir_buff_end + _handle->item_size) % _handle->cir_buff_size != _handle->cir_buff_start){
		// not full 
		memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, _handle->item_size);	
		_handle->cir_buff_end = (_handle->cir_buff_end + _handle->item_size)% _handle->cir_buff_size;
		return 0;
	}else {
		return -1;
	}

}

static int cir_buf_num_buffered_item(cir_buf_t* _handle)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (_handle->cir_buff_start <= _handle->cir_buff_end)
		return (_handle->cir_buff_end - _handle->cir_buff_start)/_handle->item_size;
	else
		return (_handle->cir_buff_size - (_handle->cir_buff_start - _handle->cir_buff_end))/_handle->item_size;

}


static int cir_buf_num_unbuffered_item(cir_buf_t* _handle)
{
	if (_handle->cir_buff == NULL)
		return -1;

	int num_buffered_item = cir_buf_num_buffered_item(_handle);
	return _handle->num_item - num_buffered_item;
}


static int cir_buf_buffer_n_item(cir_buf_t* _handle, char *item, int _num_item)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (cir_buf_is_full(_handle) == 1)
		return 0;

	// get number of item which is buffered
	int num_item_buffered = cir_buf_num_buffered_item(_handle);
	// num of item unbuffered
	int num_item_unbuffered = _handle->num_item - num_item_buffered;
	// num of item for buffering
	int num_item_xxx = (num_item_unbuffered < _num_item ? num_item_unbuffered: _num_item);

	// not good naming ????
	int end_item_index = _handle->cir_buff_end/_handle->item_size; 
	int gap_end_and_last_item = _handle->num_item + 1 - end_item_index;
	if (num_item_xxx <= gap_end_and_last_item)
	{
			memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, num_item_xxx*_handle->item_size);
			_handle->cir_buff_end = (_handle->cir_buff_end + num_item_xxx*_handle->item_size) % _handle->cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, gap_end_and_last_item);
		_handle->cir_buff_end = (_handle->cir_buff_end + gap_end_and_last_item*_handle->item_size) % _handle->cir_buff_size;
		assert(_handle->cir_buff_end == 0); // pointer_end is wrapped around

		memcpy(&_handle->cir_buff[_handle->cir_buff_end], item + gap_end_and_last_item*_handle->item_size, (num_item_xxx - gap_end_and_last_item)*_handle->item_size);
		_handle->cir_buff_end = (_handle->cir_buff_end + (num_item_xxx - gap_end_and_last_item)*_handle->item_size) % _handle->cir_buff_size;
		return num_item_xxx;
	}


}



static int cir_buf_debuffer_item(cir_buf_t* _handle, char *item)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (_handle->cir_buff_end != _handle->cir_buff_start) {
		memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], _handle->item_size);
        _handle->cir_buff_start = (_handle->cir_buff_start + _handle->item_size) % _handle->cir_buff_size;
        return 0;
	}
//otherwise, the buffer is empty; return an error code
	return -1;


}
static int cir_buf_debuffer_n_item(cir_buf_t* _handle, char *item, int _num_item)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (cir_buf_is_empty(_handle) == 1)
		return 0;

	// get number of item which is buffered
	int num_item_buffered = cir_buf_num_buffered_item(_handle);
	// num of item for debufferting
	int num_item_xxx = (num_item_buffered < _num_item ? num_item_buffered: _num_item);

	// not good naming ????
	int gap_start_and_last_item = _handle->num_item + 1 - _handle->cir_buff_start/_handle->item_size;

	if (num_item_xxx <= gap_start_and_last_item)
	{
			memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], num_item_xxx*_handle->item_size);
			_handle->cir_buff_start = (_handle->cir_buff_start + num_item_xxx*_handle->item_size) % _handle->cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], gap_start_and_last_item);
		_handle->cir_buff_start = (_handle->cir_buff_start + gap_start_and_last_item*_handle->item_size) % _handle->cir_buff_size;
		//ssert(cir_buff_start == 0); // pointer_end is wrapped around

		memcpy(item + gap_start_and_last_item*_handle->item_size, &_handle->cir_buff[_handle->cir_buff_start], (num_item_xxx - gap_start_and_last_item)*_handle->item_size);
		_handle->cir_buff_start = (_handle->cir_buff_start + (num_item_xxx - gap_start_and_last_item)*_handle->item_size) % _handle->cir_buff_size;
		return num_item_xxx;
	}



}


static void _mpeg2ts_get_ts_header(unsigned char *buff, ts_header_t *ts_header )
{
	unsigned char  tmp; 
	tmp = buff[1];
	ts_header->transport_error_indicator = (tmp & 0x80) >> 7;
	ts_header->payload_unit_start_indicator = (tmp & 0x40) >> 6;


	unsigned short pidh = buff[1] & 0x1F; 
	unsigned short pidl = buff[2];
	ts_header->pid = (pidh << 8) | pidl;
	tmp = buff[3];
	ts_header->adaptation_field_ctl = (tmp >> 4) & (0x03);

	tmp = buff[3];
	ts_header->continuity_counter = (tmp & 0x0F);
}



static void _mpeg2ts_get_adaptation( unsigned char *packet, ts_adaptation_t *adaptation )
{
	 
	adaptation->adaptation_field_len= packet[4];
	if (adaptation->adaptation_field_len == 0)
		return;
	unsigned char tmp; 
	tmp = packet[5];
	adaptation->discontinuity_indicator = (tmp & 0x80) >> 8;
	adaptation->PCR_flag = (tmp & 0x10) >> 4;

	if (adaptation->PCR_flag == 1)
	{
		uint64_t h4, h3, h2, h1, l1, l2, l3, l4;

		h1 = packet[6];
		l1 = packet[7];
		l2 = packet[8];
		l3 = packet[9];
		l4 = packet[10] & 0x80;
		adaptation->clock_reference_base = (h1 << 25) |  (l1 << 17) | (l2 << 9) | (l3 << 1) | (l4 >> 7);

		short int h, l;
		h = packet[10] & 0x01;
		l = packet[11];
		adaptation->clock_reference_ext = (h << 8) | l;
	}
}



handle remux_mpeg2ts_init(mpeg2ts_remux_callback callback, void *cb_data)
{

	remux_mpeg2ts *remux = (remux_mpeg2ts *)malloc(sizeof(remux_mpeg2ts));
	remux->callback_get_data = callback;
	remux->callback_data = cb_data;
	remux->num_pids = 0;
	remux->out_bitrate = -1;
	remux->pcr_188 = -1;
	remux->pcr0 = -1;
	remux->pcr1 = -1;	
	remux->sw_pcr = 0;
	remux->ts_packets = cir_buf_init(188, 10240);
	remux->ts_multiplexed_packets = cir_buf_init(188, 10240);
	remux->tmp_ts_buffer = (char*) malloc(188*10240);

//	remux->debug_num_decoded_packet = 0;

	return remux;

}


int remux_mpeg2ts_quit(handle _handle)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)malloc(sizeof(remux_mpeg2ts));
	free(remux->tmp_ts_buffer);
	cir_buf_quit(remux->ts_packets);
	cir_buf_quit(remux->ts_multiplexed_packets);	
	free(_handle);
	return 0;
}

	
int remux_mpeg2ts_add_pid(handle _handle, int pid)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)_handle;
	remux->pid_list[remux->num_pids] = pid;
	remux->num_pids++;
	return 0;
}

static int is_need_pids(handle _handle, int pid)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)_handle;

	for (int i = 0; i < remux->num_pids; i++)
		if (pid == remux->pid_list[i])
			return 1;	

	return 0;
}

int remux_mpeg2ts_set_out_bitrate(handle _handle, int bitrate)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)_handle;
	remux->out_bitrate = bitrate;
	remux->pcr_188 = (int64_t) ((188*8*1.0/bitrate)*27000000);

	return 0;
}

int remux_mpeg2ts_get_ts_packet(handle _handle, char *buf)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)_handle;
	
_TRY_:
	if (cir_buf_num_buffered_item(remux->ts_multiplexed_packets) > 0)
	{
		cir_buf_debuffer_item(remux->ts_multiplexed_packets, buf);
		if (file != NULL)
			fwrite(buf, 1, 188, file);
		return 0;
	} else
	{
		int rc = remux->callback_get_data(_handle, cir_buf_num_unbuffered_item(remux->ts_packets), remux->callback_data);
		if (rc == REMUX_EOF)
			return REMUX_EOF;
		goto _TRY_;		
	}
	return 0;
}

int remux_mpeg2ts_push_ts_packet(handle _handle, char *buff, int num_packet)
{
	remux_mpeg2ts *remux = (remux_mpeg2ts *)_handle;
	for (int i = 0; i < num_packet; i++)
	{
		//remux->debug_num_decoded_packet++;
		// check syn
		ts_header_t header; 
		_mpeg2ts_get_ts_header((unsigned char *)(buff + i*188), &header);

		if (header.transport_error_indicator == 1)
			continue;

		if (!is_need_pids(_handle, header.pid))
		{
			continue;
		}		
		//printf("[Debug] num decoded packet: %lld \n", remux->debug_num_decoded_packet);
		//if (remux->debug_num_decoded_packet == 5612)
		//	int tmp = 0; 

		cir_buf_buffer_item(remux->ts_packets, buff + i*188);
		remux->pcr0_pcr1_num_packets++;


		if ((header.adaptation_field_ctl == 0 || header.adaptation_field_ctl == 1))
			continue;

		ts_adaptation_t _adaptation; 
		_mpeg2ts_get_adaptation((unsigned char *)(buff + i*188), &_adaptation);
		if (_adaptation.adaptation_field_len == 0)
			continue;

		if (_adaptation.PCR_flag)
		{
			remux->pcr1 =  _adaptation.clock_reference_base*300 + _adaptation.clock_reference_ext;
			if (remux->pcr1 > remux->pcr0 && remux->pcr0 != -1 /* not first found PCR */)
			{
				// do bitrate adaptation 
				int64_t diff_pcr_value = remux->pcr1 - remux->pcr0;
				int64_t pcr_step = diff_pcr_value/remux->pcr0_pcr1_num_packets;

				double second = diff_pcr_value*1.0/27000000;
				int bitrate = (int) ((remux->pcr0_pcr1_num_packets*1.0*188*8)/second);
				//printf("[Debug] pid: %d, packetgap: %d, second: %f, diff pcr: %lld, bitrate: %d \n", header.pid, remux->pcr0_pcr1_num_packets, second, diff_pcr_value, bitrate);
				//printf("[Debug] pcr0: %lld, pcr1: %lld , num decoded packet: %lld \n", remux->pcr0, remux->pcr1, remux->debug_num_decoded_packet);
				//printf("[Debug] out-bitrate:  %d \n", remux->out_bitrate);


				char ts_packet_null[204]; 
				ts_packet_null[0] = 0x47;
				ts_packet_null[1] = 0x1F;
				ts_packet_null[2] = 0xFF;
				ts_packet_null[3] = 0x10;
				memset(&ts_packet_null[4], 0xFF, 188-4);
				
				if (remux->sw_pcr == 0)
					remux->sw_pcr = remux->pcr0;

				int num_added_null_packet = 0;
				int num_total_packet = cir_buf_num_buffered_item(remux->ts_packets); 

				while (cir_buf_num_buffered_item(remux->ts_packets) > 0)				
				{
					int64_t packet_pcr =  remux->pcr0 + ((int64_t)remux->pcr0_pcr1_num_packets - (int64_t)cir_buf_num_buffered_item(remux->ts_packets))*pcr_step;
//					printf("[Debug] %s, %d, %lld, %lld, pcr0: %lld, pcr1: %lld, 188_step: %lld, pcr_step: %lld, pcr_gap: %lld \n", __FILE__, __LINE__, packet_pcr, remux->sw_pcr, remux->pcr0, remux->pcr1, remux->pcr_188, pcr_step, (int64_t)remux->pcr0_pcr1_num_packets - (int64_t)cir_buf_num_buffered_item(remux->ts_packets));  

//					assert(0);
					if (remux->sw_pcr + remux->pcr_188 < packet_pcr )
					{
					//	printf("[Debug] %s, %d \n", __FILE__, __LINE__);
						// get null packet 	
						cir_buf_buffer_item(remux->ts_multiplexed_packets, &ts_packet_null[0]);
						num_added_null_packet++;
					}else
					{
						char ts_packet_tmp[204];
						cir_buf_debuffer_item(remux->ts_packets, &ts_packet_tmp[0]);
						cir_buf_buffer_item(remux->ts_multiplexed_packets, &ts_packet_tmp[0]);
//						printf("[Debug] %s, %d \n", __FILE__, __LINE__);
					}

					// update sw_pcr 
					remux->sw_pcr += remux->pcr_188; 

				}
				//printf("[Debug] num added packet: %d \n", num_added_null_packet);
			}else if (remux->pcr0 != -1)
			{
				//printf("[Debug] update pcr :%lld \n", remux->pcr1);
				remux->sw_pcr = remux->pcr1;
			}


			remux->pcr0 = remux->pcr1;
			remux->pcr0_pcr1_num_packets = 0;

		}
		
		

	}

	return num_packet;


}

