#ifndef _DVBT2_TIMESTAMP_H_
#define _DVBT2_TIMESTAMP_H_

#ifdef __cplusplus
extern "C"
{
#endif

// conform to DVB Document A136r2 @ clause 5.2.5
// T2-MI payload (type T2 timestamp packet)
typedef struct T2MI_TimeStamp{
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

}T2MI_TimeStamp_t;


void timestamp_init(T2MI_TimeStamp_t *p_timestamp, int bw);
void timestamp_gen(T2MI_TimeStamp_t *p_timestamp);
int timestamp_to_buffer(T2MI_TimeStamp_t *p_timestamp, unsigned char *buff);

#ifdef __cplusplus
}
#endif




#endif
