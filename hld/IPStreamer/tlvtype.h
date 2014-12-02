#ifndef	_TLVTYPE_H
#define	_TLVTYPE_H
////////////////////////
#define TLV_YES		1
#define TLV_NO		0
#define TLV_OK		1
#define TLV_ZERO	0
#define TLV_ONE		1
#define TLV_TWO		2
#define TLV_THREE		3
#define TLV_ERR		-1

///////////////////////
//#if defined(WIN32)
//#define MIN_BYTES_TO_CALC_BITRATE 0x8000
//#else
#define MIN_BYTES_TO_CALC_BITRATE 0x100000
//#endif
#define TLV_MIN_BITRATE 0x100000

//////////////////////
#if defined(WIN32)
extern void usleep(long usec);
#endif

#endif
