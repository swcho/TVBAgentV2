#ifndef _TS_MUX_SYS_H_
#define _TS_MUX_SYS_H_

#include <stdio.h>
#include <stdint.h>


// ---------------------------- TS WRITER part -------------------------------------------------

#define CIR_BUFF_SIZE_WRITER 1024*1024
typedef struct
{
	unsigned char sync;  // 8 bit
	unsigned char transport_error_indicator;  // 1 bit
	unsigned char payload_unit_start_indicator;  // 1 bit
	unsigned char transport_priority;       // 1 bit
	unsigned short pid;           // 13 bit
	unsigned char scrambling_ctrl;// 2 bit
	unsigned char adaptation_field;// 2 bit
	unsigned char continuity_counter;// 4 bit
}M2TS_Header;


typedef struct
{
	unsigned char adaptation_field_length; // 8 bit
	unsigned char discontinuity_indicator; // 1 bit
	unsigned char random_access_indicator; // 1 bit
	unsigned char priority_indicator;     // 1 bit

	unsigned char PCR_flag;             // 5 bits
 	uint64_t PCR_base, PCR_ext;

	unsigned int OPCR_flag;
	uint64_t OPCR_base, OPCR_ext;

	unsigned int splicing_point_flag;
	unsigned int transport_private_data_flag;
	unsigned int adaptation_field_extension_flag;

} M2TS_AdaptationField;


typedef struct ts_mux_s{

	f_get_payload_data_t callback_func;
	void *callback_param;

	char *t2mi_buffer;
	int t2mi_index_start; 
	int t2mi_index_last; 
	unsigned int pid;
	unsigned int packet_size;

	//////////////////////////////////////////////
	// T2MI
	// indicate the number of bytes from first byte of cir_buff to first byte of a new (first) T2MI packet
	int pointer_field; 
	int t2mi_packet_start; 

	int continuity_counter; 


}ts_mux_t;



#endif

