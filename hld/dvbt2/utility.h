#ifndef _UTILITY_H_
#define _UTILITY_H_

#ifdef __cplusplus
extern "C"
{
#endif



// CRC-8
unsigned char crc8(unsigned char *buff, int len, unsigned char init);
// CRC-32
unsigned int crc32 (const unsigned char *buf, int len, unsigned int init);


// Some functions for bit manipulation
int AddBitToBuffer(unsigned char *des, int des_length_in_bit, unsigned char *src, int src_length_in_bit);


#ifdef __cplusplus
}
#endif


#endif
