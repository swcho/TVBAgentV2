#include	<string.h>

#include "dvbt2_timestamp_payload.h"
#include "utility.h"


void timestamp_init(T2MI_TimeStamp_t *p_timestamp, int bw)
{
	p_timestamp->bw = bw;
        p_timestamp->rfu = 0;
}




void timestamp_gen(T2MI_TimeStamp_t *p_timestamp)
{


        p_timestamp->second_since_2000[0] = 0xFF;
        p_timestamp->second_since_2000[1] = 0xFF;
        p_timestamp->second_since_2000[2] = 0xFF;
        p_timestamp->second_since_2000[3] = 0xFF;
        p_timestamp->second_since_2000[4] = 0xFF;
        p_timestamp->subseconds[0] = 0xFF;
        p_timestamp->subseconds[1] = 0xFF;
        p_timestamp->subseconds[2] = 0xFF;
        p_timestamp->subseconds[3] = 0xFF;
        p_timestamp->utco[0] = 0xFF;
        p_timestamp->utco[1] = 0xFF;

}

int timestamp_to_buffer(T2MI_TimeStamp_t *p_timestamp, unsigned char *buff)
{
	unsigned int length_in_bits = 0; 
	char buff_tmp[5];

	// 4 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_timestamp->rfu << 4;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);

	// 4 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_timestamp->bw << 4;
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);

	// 40 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_timestamp->second_since_2000[0];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->second_since_2000[1];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->second_since_2000[2];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->second_since_2000[3];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->second_since_2000[4];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 27 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_timestamp->subseconds[0];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->subseconds[1];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->subseconds[2];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->subseconds[3];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

	// 13 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = p_timestamp->utco[0];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = p_timestamp->utco[1];
        length_in_bits = AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

	//ssert(length_in_bits == (4 + 4 + 40 + 27 + 13));
	return length_in_bits;
}

//timestamp_decode

