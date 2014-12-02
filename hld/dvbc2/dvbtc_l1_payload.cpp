#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

#include "dvbc2_system.h"
#include "dvbc2_multipleplp.h"
#include "dvbc2_l1_payload.h"
#include "CUtility.h"


extern CDVBC2_MultiplePLP multiple_plp; 


CDVBC2_L1_PayLoad::CDVBC2_L1_PayLoad()
{

	
}
CDVBC2_L1_PayLoad::~CDVBC2_L1_PayLoad(){
}


void CDVBC2_L1_PayLoad::init(DVBC2_PARAM _c2mi_multi_plp)
{
	int i;
	int s2 = _c2mi_multi_plp.FFT_SIZE << 1;
	
	//DEFAULT
	int L1_FEC_TYPE = 0;
	int TYPE_2_START = 0; 
	int TX_ID_AVAILABILITY = 0;
	int REGEN_FLAG = 0;
	int L1_POST_EXTENSION = 0;
	int NUM_RF = 1;
	int CURRENT_RF_IDX = 0;
	int RESERVED = 0;
	int C2_VERSION = 0;//0 == 1.1.1, 1 == 1.2.1
	int SUB_SLICES_PER_FRAME = 1;
	int NUM_AUX = 0;
	int AUX_CONFIG_RFU = 0;
	int RF_IDX = 0;
	int PLP_TYPE = 1;
	int PLP_PAYLOAD_TYPE = 3;//TS
	int FF_FLAG = 0;
	int FIRST_RF_IDX = 0;
	int FIRST_FRAME_IDX = 0;
	int PLP_GROUP_ID = 1;
	//int PLP_ROTATION = 0;
	int FRAME_INTERVAL = 1;
	int IN_BAND_A_FLAG = 0;
	int IN_BAND_B_FLAG = 0;
	int RESERVED_1 = 0;
	int STATIC_FLAG = 0; // 

	int STATIC_PADDING_FLAG = 0;
	int SUB_SLICE_INTERVAL = 0;
	int TYPE_C2_START = 0;
	int L1_CHANGE_COUNTER = 0;
	int START_RF_IDX = 0;
	int PLP_START = 0;
	int RESERVED_2 = 0;
	int RESERVED_3 = 0;
	int L1EXT_LEN = 0;


	// 
	// TODO: dount here

	// top priority of initialization 
	l1_payload.l1conf._NUM_PLP = _c2mi_multi_plp.num_plp;
	l1_payload.l1conf._NUM_AUX = NUM_AUX;
	l1_payload.l1pre._NUM_RF = NUM_RF;
	
	//ssert(l1_payload.l1conf._NUM_PLP == 2);



	// initialize the L1 pre value 
	l1_payload.l1pre._TYPE =  0x00; // TS 
	l1_payload.l1pre._BWT_EXT = _c2mi_multi_plp.BW_EXT; 
	l1_payload.l1pre._S1 = _c2mi_multi_plp.S1; //  using C2_SISO format 
	l1_payload.l1pre._s2 = s2; // 8k, not mixed 
	


	l1_payload.l1pre._L1_REPETITION_FLAG = _c2mi_multi_plp.L1_REPETITION; 
	l1_payload.l1pre._GUARD_INTERVAL = _c2mi_multi_plp.GUARD_INTERVAL;
	l1_payload.l1pre._PAPR = _c2mi_multi_plp.PAPR; // carefully no PAPR reduction is used
	l1_payload.l1pre._L1_MOD = _c2mi_multi_plp.L1_MOD; 	
	l1_payload.l1pre._L1_COD = _c2mi_multi_plp.L1_COD;
	//2011/5/20 DVB-C2 MULTI PLP
	l1_payload.l1pre._L1_FEC_TYPE = _c2mi_multi_plp.FEC_TYPE; //L1_FEC_TYPE;// ?? 



	// make sure that NUM_PLP, NUM_AUX, NUM_RF are initialized first
	{
		int l1_mod = _c2mi_multi_plp.L1_MOD;
		int fft_size = _c2mi_multi_plp.FFT_SIZE;

		int L1_POST_INFO_SIZE = L1PostConfLen()  + L1PostDynamicCurrLen();
		int L1_POST_SIZE =  CUtilities::TL_Calculate_L1_Post_Size(l1_mod, fft_size, L1_POST_INFO_SIZE);
		l1_payload.l1pre._L1_POST_SIZE = L1_POST_SIZE; // 
		l1_payload.l1pre._L1_POST_INFO_SIZE = L1_POST_INFO_SIZE;
	}



	l1_payload.l1pre._PILOT_PATTERN = _c2mi_multi_plp.Pilot;
	l1_payload.l1pre._TX_ID_AVAIBILITY = _c2mi_multi_plp.TX_ID_AVAILABILTY; 

	l1_payload.l1pre._CELL_ID= _c2mi_multi_plp.Cell_ID;
	l1_payload.l1pre._NETWORK_ID = _c2mi_multi_plp.NETWORK_ID;

	l1_payload.l1pre._C2_SYSTEM_ID = _c2mi_multi_plp.C2_ID;
	l1_payload.l1pre._NUM_C2_FRAMES  = _c2mi_multi_plp.NUM_C2_FRAME;
	l1_payload.l1pre._NUM_DATA_SYMBOLS = _c2mi_multi_plp.NUM_DATA_SYMBOLS;

	l1_payload.l1pre._REGEN_FLAG = _c2mi_multi_plp.REGEN_FLAG;
	l1_payload.l1pre._L1_POST_EXTENSION = _c2mi_multi_plp.L1_POST_EXTENSION; /// ???
	l1_payload.l1pre._NUM_RF = NUM_RF;
	l1_payload.l1pre._CURRENT_RF_IDX = CURRENT_RF_IDX;
	l1_payload.l1pre._RESERVED = RESERVED;
	l1_payload.l1pre._CRC_32 = 0; // will be computed later


	// l1conf_len
	{
		l1_payload.l1conf_len = this->L1PostConfLen();
	}

	l1_payload.l1conf._SUB_SLICES_PER_FRAME  = SUB_SLICES_PER_FRAME; //1
	l1_payload.l1conf._NUM_PLP = _c2mi_multi_plp.num_plp;

	l1_payload.l1conf._NUM_AUX = NUM_AUX;
	for (i = 0; i < l1_payload.l1pre._NUM_RF; i++)
	{
		l1_payload.l1conf._RF_list[i]._RF_IDX = i;
		l1_payload.l1conf._RF_list[i]._FREQUENCY = _c2mi_multi_plp.FREQUENCY; 
	}

	if (s2 && 0xF1)
	{
		l1_payload.l1conf._FEF_TYPE = 0;
		l1_payload.l1conf._FEF_INTERVAL = 0; // do no use
		l1_payload.l1conf._FEF_LENGTH = 0;	
	}

//	C2MI_L1_Post_Conf_PLP l1conf[5]; // maximun 5 plp_index
//	assert(num_plp <= 5);
//	l1conf[0].

	for (i = 0; i < _c2mi_multi_plp.num_plp; i++)
	{
		l1_payload.l1conf._plp_list[i]._PLP_ID = _c2mi_multi_plp.list_plp[i].PLP_ID;;

		l1_payload.l1conf._plp_list[i]._PLP_TYPE = PLP_TYPE;
		l1_payload.l1conf._plp_list[i]._PLP_PAYLOAD_TYPE = PLP_PAYLOAD_TYPE;
		l1_payload.l1conf._plp_list[i]._FF_FLAG = FF_FLAG;
		l1_payload.l1conf._plp_list[i]._FIRST_RF_IDX = FIRST_RF_IDX; 
		l1_payload.l1conf._plp_list[i]._FIRST_FRAME_IDX = FIRST_FRAME_IDX;
		l1_payload.l1conf._plp_list[i]._PLP_GROUP_ID = PLP_GROUP_ID;

		l1_payload.l1conf._plp_list[i]._PLP_COD = _c2mi_multi_plp.list_plp[i].PLP_COD; // parameter 
		l1_payload.l1conf._plp_list[i]._PLP_MOD = _c2mi_multi_plp.list_plp[i].PLP_MOD;

		l1_payload.l1conf._plp_list[i]._PLP_ROTATION = _c2mi_multi_plp.list_plp[i].PLP_ROTATION;

		l1_payload.l1conf._plp_list[i]._PLP_FEC_TYPE = _c2mi_multi_plp.list_plp[i].PLP_FEC;
		l1_payload.l1conf._plp_list[i]._PLP_NUM_BLOCKS_MAX = _c2mi_multi_plp.list_plp[i].PLP_NUM_BLOCKS;			

		l1_payload.l1conf._plp_list[i]._FRAME_INTERVAL = FRAME_INTERVAL;
		l1_payload.l1conf._plp_list[i]._TIME_IL_LENGTH = _c2mi_multi_plp.list_plp[i].PLP_TIME_IL_LENGTH;
		l1_payload.l1conf._plp_list[i]._TIME_IL_TYPE = _c2mi_multi_plp.list_plp[i].PLP_TIME_IL_TYPE;
		l1_payload.l1conf._plp_list[i]._IN_BAND_A_FLAG = IN_BAND_A_FLAG;
		l1_payload.l1conf._plp_list[i]._RESERVED_1 = RESERVED_1;
	}


	l1_payload.l1conf.RESERVED_2 = RESERVED_2;

	for ( i = 0; i < NUM_AUX; i++)
	{
		l1_payload.l1conf._aux_list[i]._AUX_RFU = AUX_CONFIG_RFU;
	}

	// l1-post dynamic 
	l1_payload.l1dyn_curr_len = this->L1PostDynamicCurrLen();
	// l1-post dynamic data
	l1_payload.l1dyn_curr._FRAM_IDX = 0; // default value

	l1_payload.l1dyn_curr._SUB_SLICE_INTERVAL = SUB_SLICE_INTERVAL;
	l1_payload.l1dyn_curr._TYPE_2_START = TYPE_2_START;
	l1_payload.l1dyn_curr._L1_CHANGE_COUNTER = L1_CHANGE_COUNTER;
	l1_payload.l1dyn_curr._RESERVED_1 = RESERVED_1;

	for ( i = 0; i < _c2mi_multi_plp.num_plp; i++)
	{
		l1_payload.l1dyn_curr.plp_list[i]._PLP_ID = _c2mi_multi_plp.list_plp[i].PLP_ID;
		
		// setting PLP_START
		{
		if (i == 0)
		{
			PLP_START = 0;
			//PLP_START = 8100*14;
			l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;
		}
		else {
			//PLP_START = 0;
			
			int PLP_MOD_M = l1_payload.l1conf._plp_list[i-1]._PLP_MOD;
			int PLP_FEC_TYPE_M = l1_payload.l1conf._plp_list[i-1]._PLP_FEC_TYPE;
			int PLP_NUM_BLOCKS_PRE = l1_payload.l1dyn_curr.plp_list[i-1]._PLP_NUM_BLOCKS;
			int N_mod;		

			if ( PLP_MOD_M == 0x000 )		N_mod = 2;
			else if ( PLP_MOD_M == 0x001 )	N_mod = 4;
			else if ( PLP_MOD_M == 0x002 )	N_mod = 6;
			else if ( PLP_MOD_M == 0x003 )	N_mod = 8;
			//2011/5/20 DVB-C2 MULTI PLP ==>>
			else if ( PLP_MOD_M == 0x004 )  N_mod = 10;
			else if ( PLP_MOD_M == 0x005 )  N_mod = 12;
			//<<=============================

			int N_ldpc = (PLP_FEC_TYPE_M == 0x001 ?  64800 : 16200);

			PLP_START += ((N_ldpc*(PLP_NUM_BLOCKS_PRE)) / N_mod);
			l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;

		}
		}

		l1_payload.l1dyn_curr.plp_list[i]._PLP_NUM_BLOCKS = _c2mi_multi_plp.list_plp[i].PLP_NUM_BLOCKS;

		l1_payload.l1dyn_curr.plp_list[i]._RESERVED_2 = RESERVED_2;
	}

	l1_payload.l1dyn_curr._RESERVED_3 = RESERVED_3;
	for ( i = 0; i < NUM_AUX; i++)
	{
		l1_payload.l1dyn_curr.aux_list[i].data[0] = 0;
		l1_payload.l1dyn_curr.aux_list[i].data[1] = 0;
		l1_payload.l1dyn_curr.aux_list[i].data[2] = 0;
		l1_payload.l1dyn_curr.aux_list[i].data[3] = 0;
		l1_payload.l1dyn_curr.aux_list[i].data[4] = 0;
		l1_payload.l1dyn_curr.aux_list[i].data[5] = 0;
	}
	// l1ext_len
	l1_payload.l1ext_len = 0;


	length  = PreL1PayloadToBuff((unsigned char *)&pre_buff[0]);

}




int CDVBC2_L1_PayLoad::L1PostConfLen(){

	int num_plp = l1_payload.l1conf._NUM_PLP;
	int num_aux = l1_payload.l1conf._NUM_AUX;
	int num_rf  = l1_payload.l1pre._NUM_RF;
	int s2 = l1_payload.l1pre._s2;

	int length_in_bits = 35 + 35*num_rf + ((s2 & 0x01) == 1 ? 34 : 0) + 89*num_plp + 32 + 32*num_aux;
	return length_in_bits;

}

int CDVBC2_L1_PayLoad::L1PostDynamicCurrLen(){

	int num_plp = l1_payload.l1conf._NUM_PLP;
	int num_aux = l1_payload.l1conf._NUM_AUX;
	int num_rf  = l1_payload.l1pre._NUM_RF;

	int length_in_bits = (71 + num_plp*48 + 8 + num_aux*48);
	return length_in_bits;
}

int CDVBC2_L1_PayLoad::L1PostExtLen(){
	return 0;
}

int CDVBC2_L1_PayLoad::GetLenInBits()
{
	int num_plp = l1_payload.l1conf._NUM_PLP;
	int num_aux = l1_payload.l1conf._NUM_AUX;
	int num_rf = l1_payload.l1pre._NUM_RF;

	int l1conf_len_in_byte = (L1PostConfLen() - 1)/8 + 1;
	int l1dyn_cur_len_in_byte = (L1PostDynamicCurrLen() - 1)/8 + 1;
	int l1ext_len_in_byte = 0; ///(L1PostExtLent() - 1)/8 + 1;

	return 8 /* fram_idx */ + 8 /* rfu */ + 168 /* l1pre */ + 16 /* l1conf_len */  + l1conf_len_in_byte*8 /* l1conf */ + 16  /* l1dyn_current_len */+ l1dyn_cur_len_in_byte*8 /*l1dyn_current*/ + 16 /* l1ext_len */ + l1ext_len_in_byte*8 /* l1ext */;	
}

void CDVBC2_L1_PayLoad::MakeL1Payload(int frame_idx){
	l1_payload.l1dyn_curr._FRAM_IDX = frame_idx;
}

int CDVBC2_L1_PayLoad::L1PostDynamicToBuff( unsigned char *buff)
{
	int i;
	C2MI_L1Current l1_current = this->l1_payload;
	C2MI_L1_Post_Dynamic c2mi_post_dynamic = this->l1_payload.l1dyn_curr;

	int num_plp, num_aux;
	int length_in_bits=0; 
	char buff_tmp[5];
	
	num_plp = l1_current.l1conf._NUM_PLP;
	num_aux = l1_current.l1conf._NUM_AUX;

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (c2mi_post_dynamic._FRAM_IDX & 0xFF);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 22bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((c2mi_post_dynamic._SUB_SLICE_INTERVAL >> 14) & 0xFF);
	buff_tmp[1] = (char)((c2mi_post_dynamic._SUB_SLICE_INTERVAL >> 6) & 0xFF);
	buff_tmp[2] = (char)((c2mi_post_dynamic._SUB_SLICE_INTERVAL << 2) & 0xFF);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

	// 22 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((c2mi_post_dynamic._TYPE_2_START >> 14) & 0xFF);
	buff_tmp[1] = (char)((c2mi_post_dynamic._TYPE_2_START >> 6) & 0xFF);
	buff_tmp[2] = (char)((c2mi_post_dynamic._TYPE_2_START << 2) & 0xFF);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (c2mi_post_dynamic._L1_CHANGE_COUNTER);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 3 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (c2mi_post_dynamic._L1_CHANGE_COUNTER) << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (c2mi_post_dynamic._RESERVED_1);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// for each plp
	for (i = 0; i < num_plp; i++)
	{
		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (c2mi_post_dynamic.plp_list[i]._PLP_ID);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		// 22 bits
		memset(&buff_tmp, 0, 5);

		buff_tmp[0] = (char)((c2mi_post_dynamic.plp_list[i]._PLP_START >> 14) & 0xFF);
		buff_tmp[1] = (char)((c2mi_post_dynamic.plp_list[i]._PLP_START >> 6) & 0xFF);
		buff_tmp[2] = (char)((c2mi_post_dynamic.plp_list[i]._PLP_START << 2) & 0xFF);
		
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

		// 10 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (c2mi_post_dynamic.plp_list[i]._PLP_NUM_BLOCKS >> 2) & 0xFF;
		buff_tmp[1] = (c2mi_post_dynamic.plp_list[i]._PLP_NUM_BLOCKS << 6) & 0xFF;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 10);

		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (c2mi_post_dynamic.plp_list[i]._RESERVED_2);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	}

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (c2mi_post_dynamic._RESERVED_3);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// AUX
	// 8 bits
	
	for ( i = 0; i < num_aux; i++)
	{
		// 48 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = 0;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	}

	return length_in_bits;
}

int CDVBC2_L1_PayLoad::L1PostConfToBuff( unsigned char *buff ){

	C2MI_L1Current l1_current = this->l1_payload;
	C2MI_L1_Post_Config c2mi_post_conf = this->l1_payload.l1conf;

	int length_in_bits=0; 
	int num_rf;
	char buff_tmp[5];
	int i;
	 num_rf = l1_current.l1pre._NUM_RF;


	buff[0] = (c2mi_post_conf._SUB_SLICES_PER_FRAME>>7)&0xFF;
	buff[1] = ((c2mi_post_conf._SUB_SLICES_PER_FRAME&0x7F)<<1) + ((c2mi_post_conf._NUM_PLP>>7)&0x01);
	buff[2] = ((c2mi_post_conf._NUM_PLP&0x7F)<<1) + ((c2mi_post_conf._NUM_AUX>>3)&0x01);
	buff[3] = ((c2mi_post_conf._NUM_AUX&0x07)<<5) + ((c2mi_post_conf._AUX_CONFIG_RFU>>3)&0x1F);
	buff[4] = ((c2mi_post_conf._AUX_CONFIG_RFU&0x07)<<5);
	length_in_bits = 35; 



	for (i = 0; i < num_rf; i++)
	{		
		memset(&buff_tmp, 0, 5);
		
		buff_tmp[0] = (c2mi_post_conf._RF_list[i]._RF_IDX & 0x07) << 5;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);
		

		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((c2mi_post_conf._RF_list[i]._FREQUENCY & 0xFF000000) >> 24);
		buff_tmp[1] = (char)((c2mi_post_conf._RF_list[i]._FREQUENCY & 0x00FF0000) >> 16);
		buff_tmp[2] = (char)((c2mi_post_conf._RF_list[i]._FREQUENCY & 0x0000FF00) >> 8);
		buff_tmp[3] = (char)((c2mi_post_conf._RF_list[i]._FREQUENCY & 0x000000FF));
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);
	}

	

	// S2	
	
	if (l1_current.l1pre._s2 & 0x01) // is xxx1
	{
		// 4 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (c2mi_post_conf._FEF_TYPE) << 4;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);
		

		// 22 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((c2mi_post_conf._FEF_LENGTH >> 14) & 0xFF);
		buff_tmp[1] = (char)((c2mi_post_conf._FEF_LENGTH >> 6) & 0xFF);
		buff_tmp[1] = (char)((c2mi_post_conf._FEF_LENGTH << 2) & 0xFF);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 22);

		// 8 bits
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = c2mi_post_conf._FEF_INTERVAL;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	}

	
	for ( i = 0; i < c2mi_post_conf._NUM_PLP; i++)	{
		

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = c2mi_post_conf._plp_list[i]._PLP_ID;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_TYPE & 0x07)<< 5; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);
		
		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_PAYLOAD_TYPE & 0x1F)<< 3; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._FF_FLAG & 0x01)<< 7; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._FIRST_RF_IDX & 0x07)<< 5; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._FIRST_FRAME_IDX & 0xFF); 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_GROUP_ID & 0xFF); 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_COD & 0x07) << 5; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_MOD & 0x07) << 5; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_ROTATION & 0x01) << 7; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_FEC_TYPE & 0x03) << 6; 
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 2);

		memset(&buff_tmp, 0, 3);		
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._PLP_NUM_BLOCKS_MAX >> 2) & 0xFF;
		buff_tmp[1] = (c2mi_post_conf._plp_list[i]._PLP_NUM_BLOCKS_MAX << 6) & 0xFF;				
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 10);
		
		

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._FRAME_INTERVAL & 0xFF);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._TIME_IL_LENGTH & 0xFF);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._TIME_IL_TYPE & 0x01) << 7;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);
		
		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = c2mi_post_conf._plp_list[i]._IN_BAND_A_FLAG << 7;
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 1);



		memset(&buff_tmp, 0, 3);
		buff_tmp[0] = (c2mi_post_conf._plp_list[i]._RESERVED_1 & 0xFF00) >> 8;
		buff_tmp[1] = (c2mi_post_conf._plp_list[i]._RESERVED_1 & 0x00FF);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 16);
	}
	
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (char)((c2mi_post_conf.RESERVED_2 & 0xFF000000) >> 24);
	buff_tmp[1] = (char)((c2mi_post_conf.RESERVED_2 & 0x00FF0000) >> 16);
	buff_tmp[2] = (char)((c2mi_post_conf.RESERVED_2 & 0x0000FF00) >> 8);
	buff_tmp[3] = (char)((c2mi_post_conf.RESERVED_2 & 0x000000FF));
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);

	for ( i = 0; i < c2mi_post_conf._NUM_AUX; i++)
	{
		memset(&buff_tmp, 0, 5);
		buff_tmp[0] = (char)((c2mi_post_conf._aux_list[i]._AUX_RFU & 0xFF000000) >> 24);
		buff_tmp[1] = (char)((c2mi_post_conf._aux_list[i]._AUX_RFU & 0x00FF0000) >> 16);
		buff_tmp[2] = (char)((c2mi_post_conf._aux_list[i]._AUX_RFU & 0x0000FF00) >> 8);
		buff_tmp[3] = (char)((c2mi_post_conf._aux_list[i]._AUX_RFU & 0x000000FF) >> 0);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 32);
	}

	return length_in_bits;
}



int CDVBC2_L1_PayLoad::L1PREPayloadToBuff( unsigned char* buff)
{
	int length_in_bits=0; 
	unsigned char temp_buff[5];
	C2MI_L1_Presignalling c2mi_pre = this->l1_payload.l1pre;



	// 8 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._TYPE;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._BWT_EXT << 7;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._S1 << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 4 bit
	memset(&temp_buff[0], 0, 5);	
	temp_buff[0]  = c2mi_pre._s2 << 4;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._L1_REPETITION_FLAG << 7;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._GUARD_INTERVAL << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 4 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._PAPR << 4;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 4 bits
	c2mi_pre._L1_MOD = 3;
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._L1_MOD << 4;
	temp_buff[0] = 0x30;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 2 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._L1_COD << 6;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 2);

	// 2 bit

	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._L1_FEC_TYPE << 6;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 2);

	
	// 18 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = (char)(((c2mi_pre._L1_POST_SIZE >> 10) & 0x000000FF));
	temp_buff[1]  = (char)(((c2mi_pre._L1_POST_SIZE >> 2) & 0x000000FF));
	temp_buff[2]  = (char)(((c2mi_pre._L1_POST_SIZE << 6) & 0x000000FF));
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 18);

	// 18 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = (char)(((c2mi_pre._L1_POST_INFO_SIZE >> 10) & 0x000000FF));
	temp_buff[1]  = (char)(((c2mi_pre._L1_POST_INFO_SIZE >> 2) & 0xFF));
	temp_buff[2]  = (char)(((c2mi_pre._L1_POST_INFO_SIZE << 6) & 0xFF));
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 18);

	//ssert(t2mi_pre._L1_POST_INFO_SIZE != 0);

	// 4 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._PILOT_PATTERN << 4;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 4);

	// 8 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._TX_ID_AVAIBILITY;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._CELL_ID >> 8;
	temp_buff[1]  = c2mi_pre._CELL_ID & 0x00FF;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._NETWORK_ID >> 8;
	temp_buff[1]  = c2mi_pre._NETWORK_ID & 0x00FF;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 16 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._C2_SYSTEM_ID >> 8;
	temp_buff[1]  = c2mi_pre._C2_SYSTEM_ID & 0x00FF;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 16);

	// 8
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._NUM_C2_FRAMES;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 8);

	// 12
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._NUM_DATA_SYMBOLS >> 4;
	temp_buff[1]  = (c2mi_pre._NUM_DATA_SYMBOLS << 4) & 0x00FF;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 12);

	// 3 bits
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._REGEN_FLAG << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 1 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._L1_POST_EXTENSION << 7;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 1);

	// 3 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._NUM_RF << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 3 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._CURRENT_RF_IDX << 5;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 3);

	// 10 bit
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._RESERVED >> 2;
	temp_buff[1]  = c2mi_pre._RESERVED << 6;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 10);

	// 32 bit
/*
	memset(&temp_buff[0], 0, 5);
	temp_buff[0]  = c2mi_pre._CRC_32 >> 24;
	temp_buff[1]  = c2mi_pre._CRC_32 >> 16;
	temp_buff[2]  = c2mi_pre._CRC_32 >> 8;
	temp_buff[3]  = c2mi_pre._CRC_32 & 0x000000FF;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, &temp_buff[0], 32);
*/
	//ssert(length_in_bits == 168);		
	return length_in_bits; 

}

int CDVBC2_L1_PayLoad::L1PayloadToBuff( unsigned char *buff){

	int l1_cur_frame_idx_pos = 2 /*frame indx + rfu */ + 21 /* l1 header + l1 current signal(fixed) */ + 2 /* l1_payload.l1conf_len */ + ((l1_payload.l1conf_len - 1)/8 + 1) + 2 /*l1_payload.l1dyn_curr_len */; 

	 pre_buff[l1_cur_frame_idx_pos] = l1_payload.l1dyn_curr._FRAM_IDX;
	// pre_buff[51] = (char)l1_payload.l1dyn_curr._FRAM_IDX;
	 int length_in_bytes = (length - 1)/8 + 1;
	 memcpy(buff, &pre_buff[0], (length + 7)/8);
	return length; 
	
}

int CDVBC2_L1_PayLoad::PreL1PayloadToBuff( unsigned char *buff){

	int length_in_bits = 0;	
	char buff_tmp[5];
	C2MI_L1Current l1_payload = this->l1_payload;

	// frame index : 8 bits
	memset(&buff_tmp, 0, 5);
	// note: ?? frame_idx is not initialzed 
	buff_tmp[0] = l1_payload.frame_idx; // higher bit
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// rfu : 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = l1_payload.rfu; // higher bit
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	////
	// l1 pre

	{
		char tmp[170]; // maximun 168 bytes
		memset(&tmp[0], 0, 170);
		int size_in_bits;
		size_in_bits = L1PREPayloadToBuff((unsigned char*)&tmp[0]);
		//ssert(size_in_bits == 168);
		length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&tmp[0], size_in_bits);
	}

	///
	// l1conf_len: 16 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1conf_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1conf_len) & 0xFF; // lower bit
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 16);
	
	///
	// l1conf
	{
	int l1conf_length_in_bytes = (l1_payload.l1conf_len - 1)/8 + 1;
	int l1conf_length_in_bits;
	char l1conf_buff[72*10]; //maximun of 10 PLP ;
	memset(&l1conf_buff[0], 0, l1conf_length_in_bytes + 2);
	l1conf_length_in_bits = L1PostConfToBuff((unsigned char *)&l1conf_buff[0]);
	//ssert(l1conf_length_in_bits == l1_payload.l1conf_len);		
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&l1conf_buff[0], l1conf_length_in_bits);		
	// round......
	length_in_bits = 8*((length_in_bits - 1)/8 + 1);


	}
	
	
	///
	// l1dyn_curr_len: 16 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1dyn_curr_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1dyn_curr_len) & 0xFF; // lower bit
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 16);

	///
	// l1dyn_curr
	{
	int l1dyn_length_in_bytes = (l1_payload.l1dyn_curr_len - 1)/8 + 1;
	int l1dyn_length_in_bits;
	if (l1dyn_length_in_bytes > 15*10) 
	{
//		printf("assert: %s line: %d \n", __FUNCTION__, __LINE__);
	}
	char l1dyn_buff[15*10]; 
	memset(&l1dyn_buff[0], 0, l1dyn_length_in_bytes + 2);
	l1dyn_length_in_bits = L1PostDynamicToBuff((unsigned char *)&l1dyn_buff[0]);
	//ssert(l1dyn_length_in_bits == l1_payload.l1dyn_curr_len);	
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&l1dyn_buff[0], l1dyn_length_in_bits);	
	// round 
	length_in_bits = 8*((length_in_bits - 1)/8 + 1);	
	}

	///
	// l1EXT_len: 16 bits
	{
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = (l1_payload.l1ext_len >> 8) & 0xFF; // higher bit
	buff_tmp[1] = (l1_payload.l1ext_len) & 0xFF; // lower bit
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 16);

	// l1ext buffer
	// do not use
	/*
	int l1ext_length_in_bytes = (l1_payload.l1ext_len - 1)/8 + 1;
	int l1ext_length_in_bits = l1_payload.l1ext_len;
//	char *l1ext_buff = (char*) malloc(l1ext_length_in_bytes + 2);
	//ssertl1ext_buff != NULL);
	memset(l1ext_buff, 0, l1ext_length_in_bytes + 2);
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&l1ext_buff[0], l1ext_length_in_bits);	
	*/
	}

	////ssertlength_in_bits == 552);
	

	return length_in_bits;	
}
