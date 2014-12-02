#include	<string.h>

#include "dvbc2_system.h"
#include "dvbc2_timestamp_payload.h"
#include "CUtility.h"


CDVBC2_Timestamp_PayLoad::CDVBC2_Timestamp_PayLoad(){
//	C2MI_BW = 2;//0==1.7MHz, 1==5MHz, 2==6MHz, 3==7MHz, 4==8MHz, 5==10MHz						 
	length = 88; // 4 + 4 + 40 + 27 + 13
						 
}

int CDVBC2_Timestamp_PayLoad::GetLenInBits()
{
	return length;
}

void CDVBC2_Timestamp_PayLoad::init(int _bw){
	bw = _bw;
}

void CDVBC2_Timestamp_PayLoad::MakeTimestampPayload(){

	timestamp_payload.rfu = 0xFF;
	timestamp_payload.bw =  bw; // bandwidth
	timestamp_payload.second_since_2000[1] = 0xFF;
	timestamp_payload.second_since_2000[2] = 0xFF;
	timestamp_payload.second_since_2000[3] = 0xFF;
	timestamp_payload.second_since_2000[4] = 0xFF;
	timestamp_payload.second_since_2000[5] = 0xFF;
	timestamp_payload.subseconds[1] = 0xFF;
	timestamp_payload.subseconds[2] = 0xFF;
	timestamp_payload.subseconds[3] = 0xFF;
	timestamp_payload.subseconds[4] = 0xFF;
	timestamp_payload.utco[1] = 0xFF;
	timestamp_payload.utco[2] = 0xFF;		

}

						 


int CDVBC2_Timestamp_PayLoad::TimestampPayloadToBuff( unsigned char *buff)
{
	unsigned int length_in_bits = 0; 
	char buff_tmp[5];
	CDVBC2_Timestamp_PayLoad _timestamp_payload = *this;
	C2MI_TimeStamp timestamp_payload = _timestamp_payload.timestamp_payload;

	// 4 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = timestamp_payload.rfu << 4;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);

	// 4 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = timestamp_payload.bw << 4;
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 4);

	// 40 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = timestamp_payload.second_since_2000[0];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.second_since_2000[1];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.second_since_2000[2];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.second_since_2000[3];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.second_since_2000[4];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);

	// 27 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = timestamp_payload.subseconds[0];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.subseconds[1];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.subseconds[2];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.subseconds[3];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 3);

	// 13 bits
	memset(&buff_tmp, 0, 5);
	buff_tmp[0] = timestamp_payload.utco[0];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 8);
	buff_tmp[0] = timestamp_payload.utco[1];
	length_in_bits = CUtilities::AddBitToBuffer(buff, length_in_bits, (unsigned char *)&buff_tmp[0], 5);

	//ssert(length_in_bits == (4 + 4 + 40 + 27 + 13));
	return length_in_bits;
}
