#ifndef _DVBT2_L1_PAYLOAD_H_
#define _DVBT2_L1_PAYLOAD_H_

#include "dvbt2.h"
#include "dvbt2_system.h"

#ifdef __cplusplus
extern "C"
{
#endif



//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.2
typedef struct T2MI_L1_Presignalling {
	
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

	unsigned short _T2_SYSTEM_ID; // 16 BITS

	unsigned char _NUM_T2_FRAMES; // 8 bits

	unsigned short _NUM_DATA_SYMBOLS; // 12 bits
	unsigned char _REGEN_FLAG; // 3 bits
	unsigned char _L1_POST_EXTENSION; // 1 bit

	unsigned char _NUM_RF; // 3 bits
	unsigned char _CURRENT_RF_IDX; // 3 bits

        union {
            unsigned short _RESERVED; // 10 bit
            struct {
                unsigned char reserved : 6;    // 6 bit
                unsigned char t2_version : 4;  // 4 bit
            }v2;
        } v1v2;

}T2MI_L1_Presignalling_t;


//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1

typedef struct T2MI_L1_Post_Conf_RF{

	unsigned char _RF_IDX; // 3 bits
	unsigned int _FREQUENCY; // 32 bits
}T2MI_L1_Post_Conf_RF_t;


//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1
typedef struct T2MI_L1_Post_Conf_S2{

	unsigned char _FEF_TYPE; // 4 bits 
	unsigned int _FEF_LENGTH;  // 22 bits 
	unsigned char _FEF_INTERVAL;  // 8 bits

}T2MI_L1_Post_Conf_S2_t;

//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1
typedef struct T2MI_L1_Post_Conf_PLP{
	

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
        unsigned char _IN_BAND_FLAG;  // 1 bit
        union V1V2_ {
            unsigned short _RESERVED_1; // 16 bit
            struct {
                unsigned char static_padding_flag : 1;
                unsigned char static_flag : 1;
                unsigned char plp_mode : 2;
                unsigned short RESERVED_1 : 11; // 11 bit
                unsigned char in_band_b_flag : 1;
            }v2;
        }v1v2;


}T2MI_L1_Post_Conf_PLP_t;

typedef struct T2MI_L1_Post_AUX{
	unsigned int _AUX_RFU;
}T2MI_L1_Post_AUX_t;

//  conform to DVB Document ETSI EN 302 755 (2009-9) @ clause 7.2.3.1

typedef struct T2MI_L1_Post_Config {

	unsigned short _SUB_SLICES_PER_FRAME; // 15 bits
	unsigned char _NUM_PLP; // 8 bits
	unsigned char _NUM_AUX; // 4 bits
	unsigned char _AUX_CONFIG_RFU; // 8 bits

        // fixme: magic number 10
        T2MI_L1_Post_Conf_RF_t _RF_list[10];  // maximum 10

	// S2 == xxx1 
	unsigned char _FEF_TYPE;   // 4 bits
	unsigned int _FEF_LENGTH; // 22 bits
	unsigned char _FEF_INTERVAL; // 8 bits

        T2MI_L1_Post_Conf_PLP_t _plp_list[10];  // maximum 10

        unsigned int RESERVED_2;  // 32 bits

        T2MI_L1_Post_AUX_t _aux_list[10];	 // maximun 10
}T2MI_L1_Post_Config_t;

typedef struct T2MI_L1_Post_Dynamic_PLP{

	unsigned char _PLP_ID; // 8 bits
	unsigned int _PLP_START; // 22 bits
	unsigned short _PLP_NUM_BLOCKS; // 10 bits
	unsigned char _RESERVED_2;  // 8 bits

}T2MI_L1_Post_Dynamic_PLP_t;

typedef struct T2MI_L1_Post_Dynamic_AUX{
	char data[6];
}T2MI_L1_Post_Dynamic_AUX_t;

typedef struct T2MI_L1_Post_Dynamic{



	unsigned char _FRAM_IDX; // 8 bits

	unsigned int _SUB_SLICE_INTERVAL; // 22 bit
	unsigned int _TYPE_2_START; // 22 bit
	unsigned char _L1_CHANGE_COUNTER; // 8 bits
	unsigned char _START_RF_IDX; // 3 bits

	unsigned char _RESERVED_1;   // 8 bits

        T2MI_L1_Post_Dynamic_PLP_t plp_list[10]; // 48*NUM_PLP bit

	unsigned char _RESERVED_3;  // 8 bits
        T2MI_L1_Post_Dynamic_AUX_t aux_list[10];


}T2MI_L1_Post_Dynamic_t;




// conform to DVB Document A136r2 @ clause 5.2.3
// T2-MI payload (type L1-Current packet)
typedef struct T2MI_L1Current{
	unsigned char frame_idx; // 8bits
	// frame index of T2-frame 

        unsigned char rfu;  // 8 bits
	//char *l1_current_data; // point to the current data

	// L1-current data (168 bits)
        T2MI_L1_Presignalling_t l1pre; // l1 presignalling
	
	// l1conf_len (16 bits)
	unsigned short l1conf_len; 

	// l1conf 
        T2MI_L1_Post_Config_t l1conf;

	// l1conf_curr_len (16 bits)
	unsigned short l1dyn_curr_len;  // 16 bit
	// 
        T2MI_L1_Post_Dynamic_t  l1dyn_curr;

	// l1ext_curr_len (16 bits)
	unsigned short l1ext_len;  // 16 bit

	char l1ext[8]; // not use 

	// to improve performance, we do not need to make buffer from l1_payload structure 
	// we .... 
	// note: check standard again to know what is exactly size ???? 
        char pre_buff[10240];
	int length;

	
}T2MI_L1Current_t;

void l1_cur_init(T2MI_L1Current_t *p_l1_current, DVBT2_PARAM_t t2mi_multi_plp);
void l1_cur_gen(T2MI_L1Current_t *p_l1_current, int frame_idx);
int l1_cur_get_len_in_bits(T2MI_L1Current_t *p_l1_cur);
int l1_cur_to_buffer(T2MI_L1Current_t *p_l1_cur, unsigned char *buff);


#ifdef __cplusplus

}
#endif




#endif
