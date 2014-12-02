#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <assert.h>



#include "dvbt2_system.h"
#include "dvbt2_l1_payload.h"
#include "utility.h"




int _l1_cur_post_conf_len(T2MI_L1Current_t *p_l1_cur){

	int num_plp = p_l1_cur->l1conf._NUM_PLP;
	int num_aux = p_l1_cur->l1conf._NUM_AUX;
	int num_rf  = p_l1_cur->l1pre._NUM_RF;
	int s2 = p_l1_cur->l1pre._s2;

	int length_in_bits = 35 + 35*num_rf + ((s2 & 0x01) == 1 ? 34 : 0) + 89*num_plp + 32 + 32*num_aux;
	return length_in_bits;

}

int l1_cur_post_dynamic_cur_len(T2MI_L1Current_t *p_l1_cur){

	int num_plp = p_l1_cur->l1conf._NUM_PLP;
	int num_aux = p_l1_cur->l1conf._NUM_AUX;
	int num_rf  = p_l1_cur->l1pre._NUM_RF;

	int length_in_bits = (71 + num_plp*48 + 8 + num_aux*48);
	return length_in_bits;
}

int _l1_post_ext_len(T2MI_L1Current_t *p_l1_cur){
	return 0;
}

int _l1_cur_post_dynamic_to_buff(T2MI_L1Current_t *p_l1_cur, unsigned char *buff)
{
	int i;
        T2MI_L1Current_t l1_current = *p_l1_cur;
        T2MI_L1_Post_Dynamic_t t2mi_post_dynamic = p_l1_cur->l1dyn_curr;

	int num_plp, num_aux;
	int length_in_bits=0; 
	char buff_tmp[5];
	
	num_plp = l1_current.l1conf._NUM_PLP;
	num_aux = l1_current.l1conf._NUM_AUX;

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (t2mi_post_dynamic._FRAM_IDX & 0xFF);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 22bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((t2mi_post_dynamic._SUB_SLICE_INTERVAL >> 14) & 0xFF);
	buff_tmp[1] = (char)((t2mi_post_dynamic._SUB_SLICE_INTERVAL >> 6) & 0xFF);
	buff_tmp[2] = (char)((t2mi_post_dynamic._SUB_SLICE_INTERVAL << 2) & 0xFF);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

	// 22 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((t2mi_post_dynamic._TYPE_2_START >> 14) & 0xFF);
	buff_tmp[1] = (char)((t2mi_post_dynamic._TYPE_2_START >> 6) & 0xFF);
	buff_tmp[2] = (char)((t2mi_post_dynamic._TYPE_2_START << 2) & 0xFF);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (t2mi_post_dynamic._L1_CHANGE_COUNTER);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 3 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (t2mi_post_dynamic._L1_CHANGE_COUNTER) << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (t2mi_post_dynamic._RESERVED_1);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// for each plp
	for (i = 0; i < num_plp; i++)
	{
		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (t2mi_post_dynamic.plp_list[i]._PLP_ID);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		// 22 bits
		memset(&buff_tmp, 0, 5);

		buff_tmp[0] = (char)((t2mi_post_dynamic.plp_list[i]._PLP_START >> 14) & 0xFF);
		buff_tmp[1] = (char)((t2mi_post_dynamic.plp_list[i]._PLP_START >> 6) & 0xFF);
		buff_tmp[2] = (char)((t2mi_post_dynamic.plp_list[i]._PLP_START << 2) & 0xFF);
		
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

		// 10 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (t2mi_post_dynamic.plp_list[i]._PLP_NUM_BLOCKS >> 2) & 0xFF;
		buff_tmp[1] = (t2mi_post_dynamic.plp_list[i]._PLP_NUM_BLOCKS << 6) & 0xFF;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 10);

		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (t2mi_post_dynamic.plp_list[i]._RESERVED_2);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	}

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (t2mi_post_dynamic._RESERVED_3);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// AUX
	// 8 bits
	
	for ( i = 0; i < num_aux; i++)
	{
		// 48 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = 0;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	}

	return length_in_bits;
}

int _l1_cur_post_conf_to_buff(T2MI_L1Current_t *p_l1_cur, unsigned char *buff ){

        T2MI_L1Current_t l1_current = *p_l1_cur;
        T2MI_L1_Post_Config_t t2mi_post_conf = p_l1_cur->l1conf;

	int length_in_bits=0; 
	int num_rf;
	char buff_tmp[5];
	int i;
	 num_rf = l1_current.l1pre._NUM_RF;


	buff[0] = (t2mi_post_conf._SUB_SLICES_PER_FRAME>>7)&0xFF;
	buff[1] = ((t2mi_post_conf._SUB_SLICES_PER_FRAME&0x7F)<<1) + ((t2mi_post_conf._NUM_PLP>>7)&0x01);
	buff[2] = ((t2mi_post_conf._NUM_PLP&0x7F)<<1) + ((t2mi_post_conf._NUM_AUX>>3)&0x01);
	buff[3] = ((t2mi_post_conf._NUM_AUX&0x07)<<5) + ((t2mi_post_conf._AUX_CONFIG_RFU>>3)&0x1F);
	buff[4] = ((t2mi_post_conf._AUX_CONFIG_RFU&0x07)<<5);
	length_in_bits = 35; 



	for (i = 0; i < num_rf; i++)
	{		
		memset(&buff_tmp, 0, 5);
		
		buff_tmp[0] = (t2mi_post_conf._RF_list[i]._RF_IDX & 0x07) << 5;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);
		

		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((t2mi_post_conf._RF_list[i]._FREQUENCY & 0xFF000000) >> 24);
		buff_tmp[1] = (char)((t2mi_post_conf._RF_list[i]._FREQUENCY & 0x00FF0000) >> 16);
		buff_tmp[2] = (char)((t2mi_post_conf._RF_list[i]._FREQUENCY & 0x0000FF00) >> 8);
		buff_tmp[3] = (char)((t2mi_post_conf._RF_list[i]._FREQUENCY & 0x000000FF));
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);
	}

	

	// S2	
	
	if (l1_current.l1pre._s2 & 0x01) // is xxx1
	{
		// 4 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (t2mi_post_conf._FEF_TYPE) << 4;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);
		

		// 22 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((t2mi_post_conf._FEF_LENGTH >> 14) & 0xFF);
		buff_tmp[1] = (char)((t2mi_post_conf._FEF_LENGTH >> 6) & 0xFF);
		buff_tmp[2] = (char)((t2mi_post_conf._FEF_LENGTH << 2) & 0xFF);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = t2mi_post_conf._FEF_INTERVAL;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	}

	
	for ( i = 0; i < t2mi_post_conf._NUM_PLP; i++)	{
		

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = t2mi_post_conf._plp_list[i]._PLP_ID;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_TYPE & 0x07)<< 5; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);
		
		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_PAYLOAD_TYPE & 0x1F)<< 3; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._FF_FLAG & 0x01)<< 7; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._FIRST_RF_IDX & 0x07)<< 5; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._FIRST_FRAME_IDX & 0xFF); 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_GROUP_ID & 0xFF); 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_COD & 0x07) << 5; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_MOD & 0x07) << 5; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_ROTATION & 0x01) << 7; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_FEC_TYPE & 0x03) << 6; 
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

		memset(&buff_tmp, 0, 3);		
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._PLP_NUM_BLOCKS_MAX >> 2) & 0xFF;
		buff_tmp[1] = (t2mi_post_conf._plp_list[i]._PLP_NUM_BLOCKS_MAX << 6) & 0xFF;				
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 10);
		
		

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._FRAME_INTERVAL & 0xFF);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._TIME_IL_LENGTH & 0xFF);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (t2mi_post_conf._plp_list[i]._TIME_IL_TYPE & 0x01) << 7;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);
		
		memset(&buff_tmp, 0, 3);
                buff_tmp[0] = t2mi_post_conf._plp_list[i]._IN_BAND_FLAG << 7;
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);



		memset(&buff_tmp, 0, 3);
                buff_tmp[0] = (t2mi_post_conf._plp_list[i].v1v2._RESERVED_1 & 0xFF00) >> 8;
                buff_tmp[1] = (t2mi_post_conf._plp_list[i].v1v2._RESERVED_1 & 0x00FF);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 16);
	}
	
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((t2mi_post_conf.RESERVED_2 & 0xFF000000) >> 24);
	buff_tmp[1] = (char)((t2mi_post_conf.RESERVED_2 & 0x00FF0000) >> 16);
	buff_tmp[2] = (char)((t2mi_post_conf.RESERVED_2 & 0x0000FF00) >> 8);
	buff_tmp[3] = (char)((t2mi_post_conf.RESERVED_2 & 0x000000FF));
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);

	for ( i = 0; i < t2mi_post_conf._NUM_AUX; i++)
	{
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((t2mi_post_conf._aux_list[i]._AUX_RFU & 0xFF000000) >> 24);
		buff_tmp[1] = (char)((t2mi_post_conf._aux_list[i]._AUX_RFU & 0x00FF0000) >> 16);
		buff_tmp[2] = (char)((t2mi_post_conf._aux_list[i]._AUX_RFU & 0x0000FF00) >> 8);
		buff_tmp[3] = (char)((t2mi_post_conf._aux_list[i]._AUX_RFU & 0x000000FF) >> 0);
                length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);
	}

	return length_in_bits;
}



int _l1_cur_pre_to_buff(T2MI_L1Current_t *p_l1_cur, unsigned char* buff)
{
	int length_in_bits=0; 
	unsigned char temp_buff[5];
        T2MI_L1_Presignalling_t t2mi_pre = p_l1_cur->l1pre;



	// 8 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._TYPE;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._BWT_EXT << 7;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._S1 << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 4 bit
	memset(&temp_buff[0], 0, 5);	
	temp_buff[0]  = t2mi_pre._s2 << 4;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._L1_REPETITION_FLAG << 7;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._GUARD_INTERVAL << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 4 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._PAPR << 4;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 4 bits
        // t2mi_pre._L1_MOD = 3; // why assume it is 64 QAM, should comment
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._L1_MOD << 4;
        // temp_buff[0] = 0x30; //
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 2 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._L1_COD << 6;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 2);

	// 2 bit

	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._L1_FEC_TYPE << 6;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 2);

	
	// 18 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = (char)(((t2mi_pre._L1_POST_SIZE >> 10) & 0x000000FF));
	temp_buff[1]  = (char)(((t2mi_pre._L1_POST_SIZE >> 2) & 0x000000FF));
	temp_buff[2]  = (char)(((t2mi_pre._L1_POST_SIZE << 6) & 0x000000FF));
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 18);

	// 18 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = (char)(((t2mi_pre._L1_POST_INFO_SIZE >> 10) & 0x000000FF));
	temp_buff[1]  = (char)(((t2mi_pre._L1_POST_INFO_SIZE >> 2) & 0xFF));
	temp_buff[2]  = (char)(((t2mi_pre._L1_POST_INFO_SIZE << 6) & 0xFF));
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 18);

	//ssert(t2mi_pre._L1_POST_INFO_SIZE != 0);

	// 4 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._PILOT_PATTERN << 4;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 8 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._TX_ID_AVAIBILITY;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._CELL_ID >> 8;
	temp_buff[1]  = t2mi_pre._CELL_ID & 0x00FF;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._NETWORK_ID >> 8;
	temp_buff[1]  = t2mi_pre._NETWORK_ID & 0x00FF;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._T2_SYSTEM_ID >> 8;
	temp_buff[1]  = t2mi_pre._T2_SYSTEM_ID & 0x00FF;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 8
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._NUM_T2_FRAMES;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 12
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._NUM_DATA_SYMBOLS >> 4;
	temp_buff[1]  = (t2mi_pre._NUM_DATA_SYMBOLS << 4) & 0x00FF;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 12);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._REGEN_FLAG << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._L1_POST_EXTENSION << 7;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._NUM_RF << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

        // 3 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = t2mi_pre._CURRENT_RF_IDX << 5;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

        // 10 bit
        memset(&temp_buff[0], 0, 1);
        temp_buff[0]  = t2mi_pre.v1v2._RESERVED >> 2;
        temp_buff[1]  = t2mi_pre.v1v2._RESERVED & 0x3;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 10);

        assert(length_in_bits == 168);
	return length_in_bits; 

}


int _l1_cur_pre_l1_to_buff(T2MI_L1Current_t *p_l1_cur){

	int length_in_bits = 0;	
    unsigned char  buff_tmp[5];
    char tmp[180]; // maximun 168 + 4 (CRC-32) bytes
	int size_in_bits;
    T2MI_L1Current_t l1_payload = *p_l1_cur;

	// frame index : 8 bits
	memset(&buff_tmp, 0, 5);
	// note: ?? frame_idx is not initialzed 
	buff_tmp[0] = l1_payload.frame_idx; // higher bit
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// rfu : 8 bits
	memset(&buff_tmp, 0, 5);
        buff_tmp[0] = l1_payload.rfu;
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	////
	// l1 pre

	{
                memset(&tmp[0], 0, 180);
                size_in_bits = _l1_cur_pre_to_buff(p_l1_cur, (unsigned char*)&tmp[0]);
                length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&tmp[0], size_in_bits);
	}

	///
	// l1conf_len: 16 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1conf_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1conf_len) & 0xFF; // lower bit
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&buff_tmp[0], 16);
	
	///
	// l1conf
	{
	int l1conf_length_in_bytes = (l1_payload.l1conf_len - 1)/8 + 1;
	int l1conf_length_in_bits;
	char l1conf_buff[72*10]; //maximun of 10 PLP ;
	memset(&l1conf_buff[0], 0, l1conf_length_in_bytes + 2);
	l1conf_length_in_bits = _l1_cur_post_conf_to_buff(p_l1_cur, (unsigned char *)&l1conf_buff[0]);
	//ssert(l1conf_length_in_bits == l1_payload.l1conf_len);		
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&l1conf_buff[0], l1conf_length_in_bits);
	// round......
	length_in_bits = 8*((length_in_bits - 1)/8 + 1);


	}
	
	
	///
	// l1dyn_curr_len: 16 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1dyn_curr_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1dyn_curr_len) & 0xFF; // lower bit
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&buff_tmp[0], 16);

	///
	// l1dyn_curr
	{
	int l1dyn_length_in_bytes = (l1_payload.l1dyn_curr_len - 1)/8 + 1;
	int l1dyn_length_in_bits;
        char l1dyn_buff[15*100];
	memset(&l1dyn_buff[0], 0, l1dyn_length_in_bytes + 2);
	l1dyn_length_in_bits = _l1_cur_post_dynamic_to_buff(p_l1_cur, (unsigned char *)&l1dyn_buff[0]);
	//ssert(l1dyn_length_in_bits == l1_payload.l1dyn_curr_len);	
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&l1dyn_buff[0], l1dyn_length_in_bits);
	// round 
	length_in_bits = 8*((length_in_bits - 1)/8 + 1);	
	}

	///
	// l1EXT_len: 16 bits
	{
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1ext_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1ext_len) & 0xFF; // lower bit
        length_in_bits = AddBitToBuffer((unsigned char *)&p_l1_cur->pre_buff[0], length_in_bits, (unsigned char *)&buff_tmp[0], 16);

	// l1ext buffer
	// do not use
	/*
	int l1ext_length_in_bytes = (l1_payload.l1ext_len - 1)/8 + 1;
	int l1ext_length_in_bits = l1_payload.l1ext_len;
//	char *l1ext_buff = (char*) malloc(l1ext_length_in_bytes + 2);
	//ssertl1ext_buff != NULL);
	memset(l1ext_buff, 0, l1ext_length_in_bytes + 2);
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&l1ext_buff[0], l1ext_length_in_bits);
	*/
	}

	////ssertlength_in_bits == 552);
	

	return length_in_bits;	
}





void l1_cur_init(T2MI_L1Current_t *p_l1_current, DVBT2_PARAM_t _t2mi_multi_plp)
{

	int i;
	int s2 = _t2mi_multi_plp.FFT_SIZE << 1;
	
	int AUX_CONFIG_RFU = 0;
	int PLP_START = 0;

        p_l1_current->rfu = 0;

	p_l1_current->l1conf._NUM_PLP = _t2mi_multi_plp.num_plp;
        p_l1_current->l1conf._NUM_AUX = 0;
        p_l1_current->l1pre._NUM_RF = 1;

	
	//ssert(p_l1_current->l1conf._NUM_PLP == 2);



	// initialize the L1 pre value 
	p_l1_current->l1pre._TYPE =  0x00; // TS 
	p_l1_current->l1pre._BWT_EXT = _t2mi_multi_plp.BW_EXT; 
	p_l1_current->l1pre._S1 = 0; // _t2mi_multi_plp.S1; //  using T2_SISO format 
	p_l1_current->l1pre._s2 = s2; // 8k, not mixed 
	


	p_l1_current->l1pre._L1_REPETITION_FLAG = _t2mi_multi_plp.L1_REPETITION; 
	p_l1_current->l1pre._GUARD_INTERVAL = _t2mi_multi_plp.GUARD_INTERVAL;
	p_l1_current->l1pre._PAPR = _t2mi_multi_plp.PAPR; // carefully no PAPR reduction is used
	p_l1_current->l1pre._L1_MOD = _t2mi_multi_plp.L1_MOD; 	
	p_l1_current->l1pre._L1_COD = _t2mi_multi_plp.L1_COD;
        p_l1_current->l1pre._L1_FEC_TYPE = 0;  // 16K



	// make sure that NUM_PLP, NUM_AUX, NUM_RF are initialized first
	{
		int l1_mod = _t2mi_multi_plp.L1_MOD;
		int fft_size = _t2mi_multi_plp.FFT_SIZE;

		int L1_POST_INFO_SIZE = _l1_cur_post_conf_len(p_l1_current)  + l1_cur_post_dynamic_cur_len(p_l1_current);
                int L1_POST_SIZE =  TL_Calculate_L1_Post_Size(l1_mod, fft_size, L1_POST_INFO_SIZE);
		p_l1_current->l1pre._L1_POST_SIZE = L1_POST_SIZE; // 
		p_l1_current->l1pre._L1_POST_INFO_SIZE = L1_POST_INFO_SIZE;
	}



	p_l1_current->l1pre._PILOT_PATTERN = _t2mi_multi_plp.Pilot;
        p_l1_current->l1pre._TX_ID_AVAIBILITY = 0;

	p_l1_current->l1pre._CELL_ID= _t2mi_multi_plp.Cell_ID;
	p_l1_current->l1pre._NETWORK_ID = _t2mi_multi_plp.NETWORK_ID;

	p_l1_current->l1pre._T2_SYSTEM_ID = _t2mi_multi_plp.T2_ID;
	p_l1_current->l1pre._NUM_T2_FRAMES  = _t2mi_multi_plp.NUM_T2_FRAME;
	p_l1_current->l1pre._NUM_DATA_SYMBOLS = _t2mi_multi_plp.NUM_DATA_SYMBOLS;

        p_l1_current->l1pre._REGEN_FLAG = 0;
        p_l1_current->l1pre._L1_POST_EXTENSION = 0; /// no exetnsion
        p_l1_current->l1pre._NUM_RF = 1;
        p_l1_current->l1pre._CURRENT_RF_IDX = 0;

        // currently, just support the version 1.1.1
        if (1) // version 1.1.1
        {
            p_l1_current->l1pre.v1v2._RESERVED = 0;

        }else
        {
           p_l1_current->l1pre.v1v2.v2.t2_version = 0;
           p_l1_current->l1pre.v1v2.v2.reserved= 0;
        }

	// l1conf_len
	{
		p_l1_current->l1conf_len = _l1_cur_post_conf_len(p_l1_current);
	}

        p_l1_current->l1conf._SUB_SLICES_PER_FRAME  = 1; // not type 2 PLP
	p_l1_current->l1conf._NUM_PLP = _t2mi_multi_plp.num_plp;
        p_l1_current->l1conf._AUX_CONFIG_RFU = 0;

        p_l1_current->l1conf._NUM_AUX = 0;
	for (i = 0; i < p_l1_current->l1pre._NUM_RF; i++)
	{
		p_l1_current->l1conf._RF_list[i]._RF_IDX = i;
		p_l1_current->l1conf._RF_list[i]._FREQUENCY = _t2mi_multi_plp.FREQUENCY; 
	}

        if (s2 & 0x01)
	{
		p_l1_current->l1conf._FEF_TYPE = 0;
		p_l1_current->l1conf._FEF_INTERVAL = 0; // do no use
		p_l1_current->l1conf._FEF_LENGTH = 0;	
	}


	for (i = 0; i < _t2mi_multi_plp.num_plp; i++)
	{
		p_l1_current->l1conf._plp_list[i]._PLP_ID = _t2mi_multi_plp.list_plp[i].PLP_ID;;

                p_l1_current->l1conf._plp_list[i]._PLP_TYPE = _t2mi_multi_plp.list_plp[i].PLP_TYPE;
                p_l1_current->l1conf._plp_list[i]._PLP_PAYLOAD_TYPE = 3; // TS
                p_l1_current->l1conf._plp_list[i]._FF_FLAG = 0;
                p_l1_current->l1conf._plp_list[i]._FIRST_RF_IDX = 0;
                p_l1_current->l1conf._plp_list[i]._FIRST_FRAME_IDX = 0;
                p_l1_current->l1conf._plp_list[i]._PLP_GROUP_ID = 1;

		p_l1_current->l1conf._plp_list[i]._PLP_COD = _t2mi_multi_plp.list_plp[i].PLP_COD; // parameter 
		p_l1_current->l1conf._plp_list[i]._PLP_MOD = _t2mi_multi_plp.list_plp[i].PLP_MOD;

		p_l1_current->l1conf._plp_list[i]._PLP_ROTATION = _t2mi_multi_plp.list_plp[i].PLP_ROTATION;

		p_l1_current->l1conf._plp_list[i]._PLP_FEC_TYPE = _t2mi_multi_plp.list_plp[i].PLP_FEC;
		p_l1_current->l1conf._plp_list[i]._PLP_NUM_BLOCKS_MAX = _t2mi_multi_plp.list_plp[i].PLP_NUM_BLOCKS;			

                p_l1_current->l1conf._plp_list[i]._FRAME_INTERVAL = 1; // PLP appear on each T2-Frame
                p_l1_current->l1conf._plp_list[i]._TIME_IL_TYPE = 0;   //_t2mi_multi_plp.list_plp[i].PLP_TIME_IL_TYPE;
                p_l1_current->l1conf._plp_list[i]._TIME_IL_LENGTH = _t2mi_multi_plp.list_plp[i].PLP_TIME_IL_LENGTH;
                p_l1_current->l1conf._plp_list[i]._IN_BAND_FLAG = 0;
                if (p_l1_current->l1pre.v1v2.v2.t2_version == 0) // version 1.1.1
                {
                   p_l1_current->l1conf._plp_list[i].v1v2._RESERVED_1 = 0;
                }else
                    assert(0); // not support
	}


        p_l1_current->l1conf.RESERVED_2 = 0;

        for ( i = 0; i < p_l1_current->l1conf._NUM_AUX; i++)
	{
		p_l1_current->l1conf._aux_list[i]._AUX_RFU = AUX_CONFIG_RFU;
	}

	// l1-post dynamic 
	p_l1_current->l1dyn_curr_len = l1_cur_post_dynamic_cur_len(p_l1_current);
	// l1-post dynamic data
	p_l1_current->l1dyn_curr._FRAM_IDX = 0; // default value

        p_l1_current->l1dyn_curr._SUB_SLICE_INTERVAL = 0;
        p_l1_current->l1dyn_curr._TYPE_2_START = 0;
        p_l1_current->l1dyn_curr._L1_CHANGE_COUNTER = 0;
        p_l1_current->l1dyn_curr._START_RF_IDX = 0;

        p_l1_current->l1dyn_curr._RESERVED_1 = 0;

	for ( i = 0; i < _t2mi_multi_plp.num_plp; i++)
	{
		p_l1_current->l1dyn_curr.plp_list[i]._PLP_ID = _t2mi_multi_plp.list_plp[i].PLP_ID;
		
		// setting PLP_START
		{
		if (i == 0)
		{
                        p_l1_current->l1dyn_curr.plp_list[i]._PLP_START = 0;
		}
		else {
			//PLP_START = 0;
                        int pre_PLP_START = p_l1_current->l1dyn_curr.plp_list[i-1]._PLP_START;
			
			int PLP_MOD_M = p_l1_current->l1conf._plp_list[i-1]._PLP_MOD;
			int PLP_FEC_TYPE_M = p_l1_current->l1conf._plp_list[i-1]._PLP_FEC_TYPE;
			int PLP_NUM_BLOCKS_PRE = p_l1_current->l1dyn_curr.plp_list[i-1]._PLP_NUM_BLOCKS;
			
			PLP_START = t2frame_fecframe_num_cell(PLP_FEC_TYPE_M, PLP_MOD_M);
			PLP_START = PLP_START*PLP_NUM_BLOCKS_PRE;

                        p_l1_current->l1dyn_curr.plp_list[i]._PLP_START = PLP_START + pre_PLP_START;

		}
		}

		p_l1_current->l1dyn_curr.plp_list[i]._PLP_NUM_BLOCKS = _t2mi_multi_plp.list_plp[i].PLP_NUM_BLOCKS;

                p_l1_current->l1dyn_curr.plp_list[i]._RESERVED_2 = 0;
	}

        p_l1_current->l1dyn_curr._RESERVED_3 = 0;
        for ( i = 0; i < p_l1_current->l1conf._NUM_AUX; i++)
	{
		p_l1_current->l1dyn_curr.aux_list[i].data[0] = 0;
		p_l1_current->l1dyn_curr.aux_list[i].data[1] = 0;
		p_l1_current->l1dyn_curr.aux_list[i].data[2] = 0;
		p_l1_current->l1dyn_curr.aux_list[i].data[3] = 0;
		p_l1_current->l1dyn_curr.aux_list[i].data[4] = 0;
		p_l1_current->l1dyn_curr.aux_list[i].data[5] = 0;
	}
	// l1ext_len
	p_l1_current->l1ext_len = 0;

	p_l1_current->length = _l1_cur_pre_l1_to_buff(p_l1_current);

}

int l1_cur_get_len_in_bits(T2MI_L1Current_t *p_l1_cur)
{
	int num_plp = p_l1_cur->l1conf._NUM_PLP;
	int num_aux = p_l1_cur->l1conf._NUM_AUX;
	int num_rf = p_l1_cur->l1pre._NUM_RF;

	int l1conf_len_in_byte = (_l1_cur_post_conf_len(p_l1_cur) - 1)/8 + 1;
	int l1dyn_cur_len_in_byte = (l1_cur_post_dynamic_cur_len(p_l1_cur) - 1)/8 + 1;
	int l1ext_len_in_byte = 0; ///(L1PostExtLent() - 1)/8 + 1;

	return 8 /* fram_idx */ + 8 /* rfu */ + 168 /* l1pre */ + 16 /* l1conf_len */  + l1conf_len_in_byte*8 /* l1conf */ + 16  /* l1dyn_current_len */+ l1dyn_cur_len_in_byte*8 /*l1dyn_current*/ + 16 /* l1ext_len */ + l1ext_len_in_byte*8 /* l1ext */;	
}


void l1_cur_gen(T2MI_L1Current_t *p_l1_cur, int frame_idx){
    p_l1_cur->frame_idx = frame_idx;
    p_l1_cur->l1dyn_curr._FRAM_IDX = frame_idx;
}


int l1_cur_to_buffer(T2MI_L1Current_t *p_l1_cur, unsigned char *buff){

	int length_in_bytes;

	int l1_cur_frame_idx_pos = 2 /*frame indx + rfu */ + 21 /* l1 header + l1 current signal(fixed) */ + 2 /* l1_payload.l1conf_len */ + ((p_l1_cur->l1conf_len - 1)/8 + 1) + 2 /*l1_payload.l1dyn_curr_len */; 

    p_l1_cur->pre_buff[0] = p_l1_cur->frame_idx;

	 p_l1_cur->pre_buff[l1_cur_frame_idx_pos] = p_l1_cur->l1dyn_curr._FRAM_IDX;
	// pre_buff[51] = (char)l1_payload.l1dyn_curr._FRAM_IDX;
	 length_in_bytes = (p_l1_cur->length - 1)/8 + 1;
	 memcpy(buff, &p_l1_cur->pre_buff[0], (p_l1_cur->length + 7)/8);
	return p_l1_cur->length; 
	
}



