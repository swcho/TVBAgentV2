#ifndef _DVBT2_MultiplePLP
#define _DVBT2_MultiplePLP
#include <stdio.h>
#include <stdlib.h>


#include "dvbt2.h"

#include "dvbt2_bbframe_payload.h"
#include "dvbt2_timestamp_payload.h"
#include "dvbt2_l1_payload.h"
#include "dvbt2_system.h"



#ifdef __cplusplus
extern "C"
{
#endif


/**
\ingroup typedef 
*/
// data struct
typedef struct t2mi_handle_s{

    struct DVBT2_PARAM dvbt2_input_param;
    struct T2MI_Packet_Header t2mi_header;

    // packet handle
    t2mi_payload_bbframe_t *payload_bb[10];
    struct T2MI_L1Current l1_cur;
    struct T2MI_TimeStamp timestamp;

    // interface
    read_one_packet_t callback_func;
    void *callback_param;


    // algorithm for ...
	int PLP_NUM_BLOCKS_TOTAL;
    // todo: maximun value?
    int bbframe_order[1000];
    int is_intl_frame_start[1000];


	// handling the T2MI packet status
	unsigned short packet_count; 
	unsigned short superframe_idx;
	unsigned short plp_index; // support only 1 plp_id
	unsigned short frame_idx;	
	int packet_index;


    // just for debugging
    FILE *file_bb;
    FILE *file_l1_cur;
    FILE *file_timestamp;
    FILE *file_muxed_t2;



}t2mi_handle_t;


#ifdef __cplusplus
}
#endif

#endif
