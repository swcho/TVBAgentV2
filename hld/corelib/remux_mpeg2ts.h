#ifndef  _REMUX_MPEG2TS_
#define _REMUX_MPEG2TS_

#include <stdio.h>
#include <stdlib.h>


#define REMUX_EOF -1

#ifdef _MSC_VER
//typedef __int64 int64_t;
//typedef unsigned __int64 uint64_t;

#else
#include <stdint.h>
#endif




// cir_buff in C implementation 

typedef struct cir_buff_s{
	char *cir_buff;
	// size in bytes
	int cir_buff_size;
	// point to start position
	int cir_buff_start;
	// point to end position
	int cir_buff_end;

	int item_size;
	// number of item
	// number of actual item for buffering data: num_item + 1
	int num_item; 
}cir_buf_t;

// do buffering
// return 0: ok
// return -1: fail (full)
static cir_buf_t* cir_buf_init(int item_size, int num_item);
static void cir_buf_quit(cir_buf_t* _handle);
static int cir_buf_is_full(cir_buf_t* _handle);
static int cir_buf_is_empty(cir_buf_t* _handle);
static int cir_buf_num_buffered_item(cir_buf_t* _handle);
static int cir_buf_num_unbuffered_item(cir_buf_t* _handle);


static int cir_buf_buffer_item(cir_buf_t* _handle, char *item);
static int cir_buf_buffer_n_item(cir_buf_t* _handle, char *item, int _num_item);

// debuffering
// return 0: ok
// return -1: fail (empty)
static int cir_buf_debuffer_item(cir_buf_t* _handle, char *item);
static int cir_buf_debuffer_n_item(cir_buf_t* _handle, char *item, int _num_item);



typedef struct ts_header_s
{
	unsigned char sync;  // 8 bit
	unsigned char transport_error_indicator;  // 1 bit
	unsigned char payload_unit_start_indicator;  // 1 bit
	unsigned char transport_priority;       // 1 bit
	unsigned short pid;           // 13 bit
	unsigned char scrambling_ctrl;// 2 bit
	unsigned char adaptation_field_ctl;// 2 bit
	unsigned char continuity_counter;// 4 bit

}ts_header_t;

typedef	struct ts_adaptation{
	char adaptation_field_len; // 8 bits
	char discontinuity_indicator;  // 1 bits
	char random_ac_indicator; // 1 bit
	// 1 bit 
	// 1 bit
	char PCR_flag; // 1 bit
	char OPCR_flag; // 1 bit
	char splicing_point_flag; // 1 bit
	char transport_private_data_flag; // 1bit
	char adaptation_field_ext_flag; // 1 bit

	uint64_t clock_reference_base; // 33 bit
	char reserved; // 6 bit
	short int clock_reference_ext; // 9 bit

} ts_adaptation_t;






#define DEMUX_MPEG2TS_MAX_PID 0x1FFF

typedef void * handle; 

// return -1: REMUX_EOF 
// return > 0: number of ts packet (188 byte) in buff 
typedef int (* mpeg2ts_remux_callback)(handle mpeg2ts, int number_packet_max, void *cb_data);

typedef struct {
	int out_bitrate;
	int num_pids;
	int pid_list[DEMUX_MPEG2TS_MAX_PID];
	mpeg2ts_remux_callback callback_get_data; 
	void *callback_data;

	
	char *tmp_ts_buffer; 
	cir_buf_t *ts_packets; 
	cir_buf_t *ts_multiplexed_packets; 

	int64_t sw_pcr; 
	int64_t pcr_188; // number of clock for each packet (188)
	int64_t pcr0;
	int64_t pcr1;
	// number of packets bw pcr0 and  pcr1 
	unsigned int pcr0_pcr1_num_packets; 

}remux_mpeg2ts;


handle remux_mpeg2ts_init(mpeg2ts_remux_callback callback, void *cb_data);
int remux_mpeg2ts_add_pid(handle _handle, int pid);
int remux_mpeg2ts_set_out_bitrate(handle _handle, int bitrate);
int remux_mpeg2ts_get_ts_packet(handle _handle, char *buf);
int remux_mpeg2ts_quit(handle _handle);

int remux_mpeg2ts_push_ts_packet(handle _handle, char *buff, int num_packet);

#endif




