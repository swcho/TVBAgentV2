#ifndef	__INCLUDE_LLD_CONST_	
#define	__INCLUDE_LLD_CONST_

#ifdef WIN32
#else
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#ifndef false
#define false 	0
#endif

#ifndef FALSE
#define FALSE 	0
#endif

#ifndef true
#define true 	1
#endif

#ifndef TRUE
#define TRUE 	1
#endif

#define	_stdcall


//
// LLD Error Code Definition
//

#define Sleep(n)	do { usleep(n * 1000); } while (0)

#if defined(TVB595V1)
#include "../libusb-0.1.12/usb.h"
#include "../libusb-0.1.12/usbi.h"

//	extern struct usb_dev_handle *hDevice_usb;
#else

//	extern int hDevice;
#endif

//2010/6/22
#if defined(WIN32)
#define IP_STREAM 1
#endif
#define TSPHLD_VLC 1
#define TSPHLD0390_EXPORTS 1

#endif


//LLD DEMO
#define MAJOR_BANK_WORD_SIZE		0x200000					/* 8Mbytes is 2M x 32bit */
#define	SUB_BANK_NUMBER				(HW_BANK_NUMBER+1)
#define SUB_BANK_OFFSET_SIZE		0x400						/* max 0x400 */
#define	SUB_BANK_BYTE_SIZE			0x100000					/* 1 MBytes */
#define	SCNT(x)						((x >> 3) & 0x3)
#define TS1_NN						(TS1_N + G704_BLOCK_SIZE)
#ifdef WIN32
#define	_MAX_INST_CNT_	23
#else
#define	_MAX_INST_CNT_	12
#endif


#endif
