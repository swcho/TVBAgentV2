#ifndef _DVBT2_SYSTEM
#define _DVBT2_SYSTEM 

#include <stdio.h>
#include <string.h>
//#include <strings.h>


#include "dvbt2_timestamp_payload.h"
#include "dvbt2_l1_payload.h"

#ifdef __cplusplus
extern "C"
{
#endif




// conform to DVB Document ETSI EN 302 755  5.1.7
typedef struct BBFrameHeader { // 80 bits
   unsigned short matype;   // 16 bits
   //  ts, gfps, gcs, gse, etc. 
   union upl_issymsb_u
   {
        unsigned short upl;
         unsigned short issy2msb;
   }upl_issy2msb;

//   unsigned short upl;   // 16 bits
   // packet length
   unsigned short dfl;   // 16 bits
   // data field length 
  union sync_issy_u{     // 8 bit
      unsigned char sync;
      unsigned char issy;
  } sync_issy;

   // sync byte of (ts, gfps, gcs, etc)
   unsigned short syncd; // 16 bits 

  // this 8-bit encode CRC-8 and mode
   unsigned char crc8_mode;   // 8 bits

   // normal mode: 0
   // high efficient mode: 1
   unsigned char HEM;
}BBFrameHeader_t;




// conform to DVB Document A136r2 @ clause 5.2.1
// T2-MI payload (type BBFrame packet)
typedef struct T2MI_BBFrame_Payload{
	unsigned char frame_idx; // 8bits
	// frame index of first T2 frame 
	// note: do not support interleaving frame
	// this value is set to xxxxx
	unsigned char plp_id;    // 8 bits
	// signal plp_id in which the Baseband frame is to be carried in the DVB-T2 signal

	unsigned char intl_frame_start; // 1bit
	// alway set to 1(2). do not support interleaving frame 

        // future use
	unsigned char rfu;  // 7 bits 
        BBFrameHeader_t bb_header;	// 80 bits

}T2MI_BBFrame_Payload_t;

// conform to DVB Document A136r2 @ clause 5.2.2
// T2-MI payload (type I/Q data packet)
struct T2MI_IQData{
	unsigned char frame_idx; // 8bits
	unsigned char aux_id;    // 4 bits
	unsigned short rfu;  // 12 bits 

	char *aux_stream_data; // point to aux_stream_data
};



// conform to DVB Document A136r2 @ clause 5.1
// T2-MI packet header

struct T2MI_Packet_Header{
	unsigned char packet_type; // 8 bits
	// baseband frame, L1 current, etc
	unsigned char packet_count; // 8 bits
	// incremented by one for each T2-MI packet sent
	unsigned char superframe_idx; // 4 bits
	// shall be constant for all T2-MI packets that carry data preparing to one T2 superframe
	// incremented for each subsequent super frame
	unsigned short rfu; // 12 bits
	// set to 0 (for future usage)
	unsigned short payload_len; // 16 bits
	// 
};

// conform to DVB Document A136r2 @ clause 5.1
// T2-MI packet

struct T2MI_Packet {
    // header
/*

    unsigned char type;
    unsigned char count;
    unsigned char sup_index; // supper frame index
    unsigned char rfu; // 3 bit
    unsigned char stream_id;
    unsigned short payload_len;
*/

    struct T2MI_Packet_Header header;
	union _Payload{
            struct T2MI_BBFrame_Payload bb_payload;
            struct T2MI_TimeStamp timestamp_payload;
            T2MI_L1Current_t l1_payload;
	} payload;
};

typedef struct T2MI_Packet T2MI_Packet_t;



#ifdef __cplusplus
}
#endif


#endif 
