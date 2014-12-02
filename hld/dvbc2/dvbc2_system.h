#ifndef _DVBC2_SYSTEM
#define _DVBC2_SYSTEM 


#define TLV_BW_1_7 xx
#define TLV_BW_5 1
#define TLV_BW_6 2
#define TLV_BW_7 3
#define TLV_BW_8 4
#define TLV_BW_10 xxx

// support loop & no loop mode
#define DVBC2_PLAY_NO_LOOP 0
#define DVBC2_PLAY_LOOP 1

// 
#define DVBC2_PLP_SCHEDULE_STATIC 0 


#define DVBC2_MULTI_PLP_STATE_NO_EXIST 0
#define DVBC2_MULTI_PLP_STATE_INIT 1
#define DVBC2_MULTI_PLP_STATE_OPEN 2
#define DVBC2_MULTI_PLP_STATE_CLOSE 3





#define PLAYBACK_TS_PACKET_SIZE_MAX 300
// conform to (A122, page 26) 
#define DVBC2_PLP_MODE_NORMAL 0  // normal mode 
#define DVBC2_PLP_MODE_HEM 1  // high efficient mode 

//////////////////////////////////////////////////////////
// Frequencies information
#define C2MI_FREQ_NUM_RF 1 // number of RF
#define C2MI_FREQ_RF_IDX_STAR 0 //


// BBFRAME

#define C2_BBFRAME_K_BCH_MAX  (58192 + 100) 
#define C2_BBFRAME_USER_DATA_SIZE_MAX  (C2_BBFRAME_K_BCH_MAX - 1)/8 + 1

// C2MI Packet 
#define C2MI_PACKET_HEADER_SIZE 48 // conform to A136R2
#define C2MI_PACKET_TYPE_BBF 0x00 // baseband frame
#define C2MI_PACKET_TYPE_AUX 0x01 // auxiliary I/O data
#define C2MI_PACKET_TYPE_L1CURRENT 0X10 // L1 current data
#define C2MI_PACKET_TYPE_TIMESTAMP 0x20 // timestamp

#define C2MI_FRAME_IDX_MAX 0xFF
#define C2MI_SUPERFRAME_IDX_MAX 0xFF



// L1 pre signall

/*
#define DVBC2_S1_C2_SISO 0 // 000
#define DVBC2_S1_C2_MISO 1 // 001

#define DVBC2_S1 0 // 000: C2_SISO preamble 
//#define DVBC2_S2 2 // 0010: FFT: 8k; not mix

#define DVBC2_REPETITION_FLAG 0 // no repetition for next frame

#define DVBC2_TYPE_TS 0X00 //TS ONLY 
#define DVBC2_TYPE_GSE_GFP 0X01 // GSE and/or GFPS and/or GCS
#define DVBC2_TYPE_TS_GSE 0X02  // TS and (GSE and/or GFPS and/or GCS)
#define DVBC2_TYPE_SELECTED  DVBC2_TYPE_TS
*/
#define DVBC2_BW_5 1
#define DVBC2_BW_6 2
#define DVBC2_BW_7 3
#define DVBC2_BW_8 4

#define DVBC2_BWT_NORMAL 0 // (0)
#define DVBC2_BWT_EXT 1 // (1)



#define DVBC2_GUARD_INTERVAL_1_32   0  // (000)
#define DVBC2_GUARD_INTERVAL_1_16   1  // (001)
#define DVBC2_GUARD_INTERVAL_1_8    2  // (010)
#define DVBC2_GUARD_INTERVAL_1_4    3  // (011)
#define DVBC2_GUARD_INTERVAL_1_128  4  // (100)
#define DVBC2_GUARD_INTERVAL_19_128 5  // (101)
#define DVBC2_GUARD_INTERVAL_19_256 6  // (110)
#define DVBC2_GUARD_INTERVAL_RESERVER 7  // (111)
#define DVBC2_GUARD_INTERVAL_SELECTED   // 

/*
#define DVBC2_PAPR_NO   0x0 // 
#define DVBC2_PAPR_ACE   0x1 // 
#define DVBC2_PAPR_TR   0x2 // 
#define DVBC2_PAPR_ACE_TR   0x3 // 
#define DVBC2_PAPR_SELECTED DVBC2_PAPR_NO
*/

#define DVBC2_MOD_BPSK 0x0
#define DVBC2_MOD_QPSK 0x1
#define DVBC2_MOD_QAM16 0x2
#define DVBC2_MOD_QAM64 0x3

/*
#define DVBC2_COD_1_2 0 // (00)
#define DVBC2_COD_SELECTED DVBC2_COD_1_2

#define DVBC2_FEC_TYPE_LDPC16K 0 // (00)
#define DVBC2_FEC_TYPE_SELECTED DVBC2_FEC_TYPE_LDPC16K
*/

#define DVBC2_PILOT_PATTERN_PP1 0X0 
#define DVBC2_PILOT_PATTERN_PP2 0X1 
#define DVBC2_PILOT_PATTERN_PP3 0X2 
#define DVBC2_PILOT_PATTERN_PP4 0X3 
#define DVBC2_PILOT_PATTERN_PP5 0X4 
#define DVBC2_PILOT_PATTERN_PP6 0X5 
#define DVBC2_PILOT_PATTERN_PP7 0X6 
#define DVBC2_PILOT_PATTERN_PP8 0X7 
#define DVBC2_PILOT_PATTERN_SELECTED 

/*
#define DVBC2_NUM_C2_FRAMES 2 
#define DVBC2_NUM_DATA_SYMBOLS 9 

#define DVBC2_REGEN_FLAG 0 
#define DVBC2_POST_EXTENSION 0 // not use post signal extension 

#define DVBC2_NUM_RF C2MI_FREQ_NUM_RF // only one RF
#define DVBC2_CURRENT_RF_IDX 0 // do not support TFS

*/


#define DVBC2_PLP_TYPE_COMMON 0 // (000) 
#define DVBC2_PLP_TYPE_DATATYPE1 1 // (001) 
#define DVBC2_PLP_TYPE_DATATYPE2 2 // (010) 

#define DVBC2_PLP_PAYLOAD_TYPE_GFPS 0 // (00000)
#define DVBC2_PLP_PAYLOAD_TYPE_GCS 1 // (00000)
#define DVBC2_PLP_PAYLOAD_TYPE_GSE 2 // (00000)
#define DVBC2_PLP_PAYLOAD_TYPE_TS 3 // (00000)

#define DVBC2_PLP_COD_1_2 0 // 000
#define DVBC2_PLP_COD_3_5 1 // 001
#define DVBC2_PLP_COD_2_3 2 // 010
#define DVBC2_PLP_COD_3_4 3 // 011
#define DVBC2_PLP_COD_4_5 4 // 100
#define DVBC2_PLP_COD_5_6 5 // 101

#define DVBC2_PLP_MOD_QPSK 0 // 000
#define DVBC2_PLP_MOD_16QAM 1 // 000
#define DVBC2_PLP_MOD_64QAM 2 // 000
#define DVBC2_PLP_MOD_256QAM 3 // 000
//2011/5/20 DVB-C2 MULTI PLP ==>>
#define DVBC2_PLP_MOD_1024QAM 4 // 000
#define DVBC2_PLP_MOD_4096QAM 5 // 000
//<<=============================

#define DVBC2_PLP_FEC_TYPE_16K_LDPC 0 // 00
#define DVBC2_PLP_FEC_TYPE_64K_LDPC 1 // 01

#define DVBC2_FFT_SIZE_32K   5    // 0101
#define DVBC2_FFT_SIZE_32K_E 7  //0111
#define DVBC2_FFT_SIZE_16K   4  // 0100
#define DVBC2_FFT_SIZE_8K    1  // 0001
#define DVBC2_FFT_SIZE_8K_E  6  // 0110
#define DVBC2_FFT_SIZE_4K    2  // 0010  
#define DVBC2_FFT_SIZE_2K    0  // 0000  
#define DVBC2_FFT_SIZE_1K    3  // 0011



// for adaptation with output (ts)

struct c2mi_packet_status{
	int packet_type;
	int num_bits;
	int packet_index;
};



// conform to ETSI EN 302 755 V1.1.1 (2009-09)
struct C2MI_BBFrameHeader { // 80 bits 
   unsigned short matype;   // 16 bits
   //  ts, gfps, gcs, gse, etc. 
   unsigned short upl;   // 16 bits
   // packet length
   unsigned short dfl;   // 16 bits
   // data field length 
   unsigned char sync;   // 8 bits
   // sync byte of (ts, gfps, gcs, etc)
   unsigned short syncd; // 16 bits 
   // unknow 
   unsigned char crc8_mode;   // 8 bits
   // unclear (normal mode, high efficiency mode)
};   

// conform to ETSI EN 302 755 V1.1.1 (2009-09) @ clause 5.1.7 
struct C2MI_BBFrame{
	C2MI_BBFrameHeader header;	// 80 bits
	char *data_unuse;
};




// conform to DVB Document A136r2 @ clause 5.2.1
// C2-MI payload (type BBFrame packet)
struct C2MI_BBFrame_Payload{
	unsigned char frame_idx; // 8bits
	// frame index of first C2 frame 
	// note: do not support interleaving frame
	// this value is set to xxxxx
	unsigned char plp_id;    // 8 bits
	// signal plp_id in which the Baseband frame is to be carried in the DVB-C2 signal

	unsigned char intl_frame_start; // 1bit
	// alway set to 1(2). do not support interleaving frame 

	unsigned char rfu;  // 7 bits 
	// future use
	C2MI_BBFrame bbframe; // kbch bits
};

// conform to DVB Document A136r2 @ clause 5.2.2
// C2-MI payload (type I/Q data packet)
struct C2MI_IQData{
	unsigned char frame_idx; // 8bits
	unsigned char aux_id;    // 4 bits
	unsigned short rfu;  // 12 bits 

	char *aux_stream_data; // point to aux_stream_data
};



//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.2
struct C2MI_L1_Presignalling {
	
	unsigned char _TYPE;  // 8 bits 

	unsigned char _BWT_EXT; // 1 bits
	unsigned char _S1; // 3 bits
	unsigned char _s2; // 4 bits

	unsigned char _L1_REPETITION_FLAG; // 1 bit
	unsigned char _GUARD_INTERVAL; // 3 bits
	unsigned char _PAPR; // 4 bits

	unsigned char _L1_MOD; // 4 bits
	unsigned char _L1_COD; // 2 bits
	unsigned char _L1_FEC_TYPE; // 2 bits

	unsigned int _L1_POST_SIZE; // 18 bits
	unsigned int _L1_POST_INFO_SIZE; // 18 bits
	unsigned char _PILOT_PATTERN; // 4 bits

	unsigned char _TX_ID_AVAIBILITY; // 8 bits

	unsigned short _CELL_ID; // 16 bits

	unsigned short _NETWORK_ID; // 16 bits

	unsigned short _C2_SYSTEM_ID; // 16 BITS

	unsigned char _NUM_C2_FRAMES; // 8 bits

	unsigned short _NUM_DATA_SYMBOLS; // 12 bits
	unsigned char _REGEN_FLAG; // 3 bits
	unsigned char _L1_POST_EXTENSION; // 1 bit

	unsigned char _NUM_RF; // 3 bits
	unsigned char _CURRENT_RF_IDX; // 3 bits
	unsigned short _RESERVED; // 10 bits

	unsigned int _CRC_32; // 32 bits

};


//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1

struct C2MI_L1_Post_Conf_RF{
	//unsigned _int8 _RF_IDX : 3; // 3 bits
	//unsigned _int32 _FREQUENCY : 32;

	unsigned char _RF_IDX; // 3 bits
	unsigned int _FREQUENCY; // 32 bits
};


//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1
struct C2MI_L1_Post_Conf_S2{
	//unsigned _int8 _FEF_TYPE: 4;
	//unsigned _int32 _FEF_LENGTH : 22; 
	//unsigned _int8 _FEF_INTERVAL : 8;

	unsigned char _FEF_TYPE; // 4 bits 
	unsigned int _FEF_LENGTH;  // 22 bits 
	unsigned char _FEF_INTERVAL;  // 8 bits

};
//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1
struct C2MI_L1_Post_Conf_PLP{
	

	unsigned char _PLP_ID; // 8 bits 

	unsigned char _PLP_TYPE;  // 3 bits
	unsigned char _PLP_PAYLOAD_TYPE; // 5 bits

	unsigned char _FF_FLAG; // 1 bit
	unsigned char _FIRST_RF_IDX; // 3 bit
	unsigned char _FIRST_FRAME_IDX; // 8 bit

	unsigned char _PLP_GROUP_ID; // 8 bit

	unsigned char _PLP_COD;  // 3 bit
	unsigned char _PLP_MOD; // 3 bit
	unsigned char _PLP_ROTATION; // 1 bit
	unsigned char _PLP_FEC_TYPE; // 2 bits
	unsigned short _PLP_NUM_BLOCKS_MAX; // 10 bits
	unsigned char _FRAME_INTERVAL;  // 8 bits
	unsigned char _TIME_IL_LENGTH; // 8 bits
	unsigned char _TIME_IL_TYPE;  // 1 bit
	unsigned char _IN_BAND_A_FLAG;  // 1 bit
	unsigned char _IN_BAND_B_FLAG; // 1 bit	
	unsigned short _RESERVED_1; // (temporary use 16 bit) --->  11 bits (version 1.1.2)
	unsigned short _PLP_MODE; // 2 BIT
	unsigned short _STATIC_FLAG;
	unsigned short _STATIC_PADDING_FLAG;

};

struct C2MI_L1_Post_AUX{
	unsigned int _AUX_RFU;
};

//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1

struct C2MI_L1_Post_Config {

	unsigned short _SUB_SLICES_PER_FRAME; // 15 bits
	unsigned char _NUM_PLP; // 8 bits
	unsigned char _NUM_AUX; // 4 bits
	unsigned char _AUX_CONFIG_RFU; // 8 bits

	C2MI_L1_Post_Conf_RF _RF_list[10];  // maximum 10

	// S2 == xxx1 
	unsigned char _FEF_TYPE;   // 4 bits
	unsigned int _FEF_LENGTH; // 22 bits
	unsigned char _FEF_INTERVAL; // 8 bits

	C2MI_L1_Post_Conf_PLP _plp_list[10];  // maximum 10

	unsigned int RESERVED_2;  // 32 bits

	C2MI_L1_Post_AUX _aux_list[10];	 // maximun 10
};

struct C2MI_L1_Post_Dynamic_PLP{
	//unsigned _int8 _PLP_ID : 8;
	//unsigned _int32 _PLP_START : 22;
	//unsigned _int16 _PLP_NUM_BLOCKS : 10;
	//unsigned _int8 _RESERVED_2 : 8;

	unsigned char _PLP_ID; // 8 bits
	unsigned int _PLP_START; // 22 bits
	unsigned short _PLP_NUM_BLOCKS; // 10 bits
	unsigned char _RESERVED_2;  // 8 bits

};

struct C2MI_L1_Post_Dynamic_AUX{
	char data[6];
};
struct C2MI_L1_Post_Dynamic{

	//unsigned char _FRAM_IDX; // 8 bits

	//unsigned int _SUB_SLICE_INTERVAL; // 22 bit
	//unsigned int _TYPE_2_START; // 22 bit
	//unsigned char _L1_CHANGE_COUNTER; // 8 bits
	//unsigned char _START_RF_IDX; // 3 bits

	//unsigned char _RESERVED_1;   // 8 bits

	//C2MI_L1_Post_Dynamic_PLP *plp_list;

	//unsigned char _RESERVED_2 : 8;
	//C2MI_L1_Post_AUX *aux_list;



	unsigned char _FRAM_IDX; // 8 bits

	unsigned int _SUB_SLICE_INTERVAL; // 22 bit
	unsigned int _TYPE_2_START; // 22 bit
	unsigned char _L1_CHANGE_COUNTER; // 8 bits
	unsigned char _START_RF_IDX; // 3 bits

	unsigned char _RESERVED_1;   // 8 bits

	C2MI_L1_Post_Dynamic_PLP plp_list[10]; // 48*NUM_PLP bit

	unsigned char _RESERVED_3;  // 8 bits
	C2MI_L1_Post_Dynamic_AUX aux_list[10];


};




// conform to DVB Document A136r2 @ clause 5.2.3
// C2-MI payload (type L1-Current packet)
struct C2MI_L1Current{
	unsigned char frame_idx; // 8bits
	// frame index of C2-frame 

	unsigned char rfu;  // 12 bits 
	//char *l1_current_data; // point to the current data

	// L1-current data (168 bits)
	C2MI_L1_Presignalling l1pre; // l1 presignalling 
	
	// l1conf_len (16 bits)
	unsigned short l1conf_len; 

	// l1conf 
	C2MI_L1_Post_Config l1conf;

	// l1conf_curr_len (16 bits)
	unsigned short l1dyn_curr_len;  // 16 bit
	// 
	C2MI_L1_Post_Dynamic  l1dyn_curr;

	// l1ext_curr_len (16 bits)
	unsigned short l1ext_len;  // 16 bit

	char l1ext[8]; // not use 
};


// conform to DVB Document A136r2 @ clause 5.2.5
// C2-MI payload (type C2 timestamp packet)
struct C2MI_TimeStamp{
	unsigned char rfu; // 4bits
	// bit reserved. set to 0 
	unsigned char bw;  // 4 bits 
	//indicate the system bandwidth: unclear

	unsigned char second_since_2000[5]; // 40 bits
	// 
	unsigned char subseconds[4];// 27 bits
	//
	unsigned char utco[2]; // 13 bits
	// unclear

};


// conform to DVB Document A136r2 @ clause 5.1
// C2-MI packet header

struct C2MI_Packet_Header{
	unsigned char packet_type; // 8 bits
	// baseband frame, L1 current, etc
	unsigned char packet_count; // 8 bits
	// incremented by one for each C2-MI packet sent
	unsigned char superframe_idx; // 4 bits
	// shall be constant for all C2-MI packets that carry data preparing to one C2 superframe
	// incremented for each subsequent super frame
	unsigned short rfu; // 12 bits
	// set to 0 (for future usage)
	unsigned short payload_len; // 16 bits
	// 
};

// conform to DVB Document A136r2 @ clause 5.1
// C2-MI packet

struct C2MI_Packet {
//	int packet_type;
	//C2MI_PACKET_TYPE_BBF;
	//C2MI_PACKET_TYPE_L1CURRENT;
	//C2MI_PACKET_TYPE_AUX;

	C2MI_Packet_Header header;	
	union _Payload{
		C2MI_BBFrame_Payload bb_payload;
		C2MI_TimeStamp timestamp_payload;
		C2MI_L1Current l1_payload;
	} payload;
};




bool c2frame_check_valid_NUM_DATA_SYMBOL(int fft_size_index, double gif, int NUM_DATA_SYMBOLS);
//double c2frame_duration(int fft_size, double gif, int NUM_DATA_SYMBOLS, int bandwidth);
int c2frame_p2_symbol_num_cell(int fft_size, int mode);
int c2frame_fecframe_num_cell(int fec_type, int modulation);
int c2frame_num_data_symbol_max(int fft_size, int gif);
int c2frame_num_data_symbol_min(int fft_size, int gif);
int c2frame_data_symbol_num_cell(int fft_size, int pilot);
int c2frame_kbch_size(int plp_fec_type, int plp_cod );
//2011/5/20 DVB-C2 MULTI PLP ============================================>>
double c2frame_duration(int l1_ti_mode, double gif, int NUM_DATA_SYMBOLS);
//<<=======================================================================

#endif 
