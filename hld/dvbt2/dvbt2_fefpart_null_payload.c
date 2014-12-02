#include	<string.h>

#include "dvbt2_fefpart_null_payload.h"
#include "utility.h"


void FEFpartNull_init(T2MI_FEFpart_Null_t *p_fef_null, int _val_s1, int _val_s2)
{

    p_fef_null->rfu = 0;
	p_fef_null->s1_field = _val_s1;
	p_fef_null->s2_field = _val_s2;
}




void FEFpartNull_gen(T2MI_FEFpart_Null_t *p_fef_null, unsigned short fef_idx)
{
	p_fef_null->fef_idx = fef_idx;
}

int FEFpartNull_to_buffer(T2MI_FEFpart_Null_t *p_fef_null, unsigned char *buff)
{
	unsigned int length_in_bits = 0; 
	char buff_tmp[5];

	// 8 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_fef_null->fef_idx;
    length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 9 bits rfu
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = 0;
	buff_tmp[1] = 0;
    length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 9);

	// 3 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_fef_null->s1_field << 5;
    length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

	//4 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_fef_null->s2_field << 4;
    length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);
	return length_in_bits;
}

//timestamp_decode

