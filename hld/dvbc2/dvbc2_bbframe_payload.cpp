#include	<string.h>

#include "dvbc2_system.h"
#include "dvbc2_bbframe_payload.h"
#include "dvbc2_input_processing.h"
#include "dvbc2_multipleplp.h"
#include "CUtility.h"
#include "malloc.h"


// usage:

// init --> sett2frame --> makepayload --> quit
//             ^              |
//             |--------------|


CDVBC2_BBFrame_PayLoad::CDVBC2_BBFrame_PayLoad()
{


}

void CDVBC2_BBFrame_PayLoad::init(int _plp_cod, int _plp_fec_type, int _plp_mode, int _num_plp, CTLVMedia *_media_source)
{	
	media_source = _media_source;

	
	plp_hem = _plp_mode;

	if (_num_plp == 1)
		plp_single_multiple_mode = PLP_MODE_SINGLE_PLP;
	else // > 1
		plp_single_multiple_mode = PLP_MODE_MULTIPLE_PLP;
	
	/* BCH Uncoded Block */
	if ( _plp_fec_type == 0x001 )
	{
		if ( _plp_cod == 0x000 )			K_bch = 32208;
		else if ( _plp_cod == 0x001 )	K_bch = 43040;
		else if ( _plp_cod == 0x002 )	K_bch = 48408;
		else if ( _plp_cod == 0x003 )	K_bch = 51648;
		else if ( _plp_cod == 0x004 )	K_bch = 53840;
		else if ( _plp_cod == 0x005 )	K_bch = 58192;
	}
	else
	{
		if ( _plp_cod == 0x000 )			K_bch = 7032;
		else if ( _plp_cod == 0x001 )	K_bch = 10632;
		else if ( _plp_cod == 0x002 )	K_bch = 11712;
		else if ( _plp_cod == 0x003 )	K_bch = 12432;
		else if ( _plp_cod == 0x004 )	K_bch = 13152;
		else if ( _plp_cod == 0x005 )	K_bch = 14232;
	}


	max_dfl_in_bits = K_bch - 80;

	// init bbframe header 
	C2MI_BBFrameHeader *bb_frame_header = &(this->s_payload_bb.bbframe.header);
	if (plp_single_multiple_mode == PLP_MODE_SINGLE_PLP)
	{
		bb_frame_header->matype =  0xF000;
	}else if(plp_single_multiple_mode == PLP_MODE_MULTIPLE_PLP){
		bb_frame_header->matype =  0xD000; //TS: 11, Multi: 0, CCM: 1, other 00000
	}
	// TS, Single input stream, all bitstream use same modulation and coding

	if ( plp_hem == DVBC2_PLP_MODE_NORMAL) // Normal Mode
	{
		bb_frame_header->upl = 188*8; 
	}
	else			// High Efficiency Mode
	{
		bb_frame_header->upl = 187*8;
	}

	if ( plp_hem == DVBC2_PLP_MODE_NORMAL ) // normal mode
	{
		bb_frame_header->sync = 0x47;
	}
	else // hew mode
	{
		bb_frame_header->sync = 0x00;
	}


	
	input_processing.init(plp_hem, this);
}

void CDVBC2_BBFrame_PayLoad::SetC2Frame(int num_bits)
{

	num_bit_c2_frame = num_bits;
	
}

int CDVBC2_BBFrame_PayLoad::GetLenInBits(){
	length = 8 + 8 + 8 + K_bch;
	return this->length;
}

int CDVBC2_BBFrame_PayLoad::MakeBBPayload(unsigned char frame_idx, unsigned char plp_id,  unsigned char _intl_frame_start)
{

	// make payload header
	s_payload_bb.frame_idx = frame_idx;
	s_payload_bb.plp_id = plp_id;
	s_payload_bb.intl_frame_start = _intl_frame_start; 
	s_payload_bb.rfu = 0x00;

	return 0;
}

// return length in bits
int CDVBC2_BBFrame_PayLoad::BBFramePayloadToBuff(unsigned char *buff)
{
	int length_in_bits;

	// frame index 
	buff[0] = s_payload_bb.frame_idx;
	buff[1] = s_payload_bb.plp_id;
	buff[2] = s_payload_bb.intl_frame_start << 7;
	buff[2] = (buff[2] & 0x80) | (s_payload_bb.rfu & ~0x80);


	// bbframe 

	length_in_bits = BBFrameToBuff(&buff[3]);	
	
	return length_in_bits + 8 + 8 + 8; 
}


// return length in bits
int CDVBC2_BBFrame_PayLoad::BBFrameToBuff(unsigned char *buff){
	
	unsigned int length_in_bits = 0; 
	C2MI_BBFrame bb_frame  = this->s_payload_bb.bbframe;

	unsigned int data_field_size_in_bytes = max_dfl_in_bits/8;
	int syncd; 
	int return_size = input_processing.read_n_bytes((char *)&buff[10], data_field_size_in_bytes, &syncd);
	if (return_size < 0)
	{
		return -1; //  
	}

	bb_frame.header.dfl = return_size*8;
	bb_frame.header.syncd = syncd;

	//////////////////////////////////////////
	// bbframe_header ---> buff (80 bits)

	{
	buff[0] = (bb_frame.header.matype >> 8) & 0xFF;	
	buff[1] = (bb_frame.header.matype & 0x00FF) & 0xFF;	

	buff[2] = (bb_frame.header.upl >> 8) & 0xFF;	
	buff[3] = (bb_frame.header.upl & 0x00FF) & 0xFF;	

	buff[4] = (bb_frame.header.dfl >> 8) & 0xFF;	
	buff[5] = (bb_frame.header.dfl & 0x00FF) & 0xFF;	

	buff[6] = (bb_frame.header.sync) & 0xFF;	

	buff[7] = (bb_frame.header.syncd >> 8) & 0xFF;	
	buff[8] = (bb_frame.header.syncd  & 0x00FF) & 0xFF;	

	// 8 bits
	int i; 
	char crc8 = 0;
	for (i = 0; i <=8; i++)
	{
		unsigned char index = buff[i] ^ crc8;
		crc8 = HCRC8::crc8_table[index];
	}
	buff[9] = crc8 ^ plp_hem; // normal mode

	length_in_bits = 80;

	}

	length_in_bits += return_size*8;
	return length_in_bits;
}
