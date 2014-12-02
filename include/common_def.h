#ifndef __INCLUDE_COMMON_DEF_
#define __INCLUDE_COMMON_DEF_


#define	TSPL_SDCON_PLAY_MODE		0
#define	TSPL_SDCON_CAPTURE_MODE		1
#define	TSPL_SDCON_DELAY_MODE		2
#define	TSPL_SDCON_LOOP_THRU_MODE	3

//I/Q PALY/CAPTURE
#define	TSPL_SDCON_IQ_PLAY_MODE		4
#define	TSPL_SDCON_IQ_CAPTURE_MODE	5
#define	TSPL_SDCON_IQ_NONE			6

enum	TSIO_CUR_STATE__
{
	TSIO_PLAY_WITH_310INPUT	=	0,
	TSIO_PLAY_WITH_ASIINPUT	=	1,
	TSIO_310_LOOPTHRU	=		2,
	TSIO_ASI_LOOPTHRU	=		3,
//	ATSC-MH
	TSIO_310_CAPTURE_PLAY	=	4,
	TSIO_ASI_CAPTURE_PLAY	=	5,
	TSIO_FILE_LOOP_PLAY		=	6,
};
typedef	enum
{
	_BD_ID_undefined__				=	0x0,

	_BD_ID_380__							=	0x26,
	_BD_ID_380v2__						=	0x27,
	_BD_ID_380v3_ifrf_scaler__		=	0x28,
	_BD_ID_380v3_upconverter__		=	0x29,

	_BD_ID_390v6__						=	0x2A,
	_BD_ID_390v6_Lowcost__				=	0x2B,
	_BD_ID_390v7__						=	0x2C,
	_BD_ID_390v8__						=	0x2D,
	_BD_ID_390v7_IF_only__				=	0x2E,

	_BD_ID_590v9_x__						=	0x2F,	//	590
	_BD_ID_590v10_x__					=	0x30,	//	590b/c/e

	_BD_ID_595v2__						=	0x3B,
	_BD_ID_595Bv3__						=	0x3C,
	_BD_ID_595C__		 					=	0x3D,

	_BD_ID_590s__							=	0x14,
	_BD_ID_593__							=	0x0F,

	_BD_ID_597__							=	0x0A,
	_BD_ID_597v2__						=	0x0B,
	_BD_ID_497__							=	0x19,

	_BD_ID_V4__ 							=	0x3D,	//	tvb595v4

	_BD_ID_591__							=	0x15,

	_BD_ID_594__							=	0x50,
	//2012/1/31 TVB591S
	_BD_ID_591S__							=	0x16,
	//2013/1/4 TVB499
	_BD_ID_499__							=	0x1B,
	//2013/5/23 TVB599
	_BD_ID_599__							=	0x0C,
	//2013/9/5 TVB598
	_BD_ID_598__							= 0x10,

}	___ID_BDs___;

/////////////////////////////////////////////////////////////////
#define	__SLOT_ALLOC_FROM__	0
#ifdef WIN32
#define	__CNT_MAX_BD_INS_AVAILABLE_	23		//	refer to _MAX_INST_CNT_
#else
#define	__CNT_MAX_BD_INS_AVAILABLE_	12		//	refer to _MAX_INST_CNT_
#endif
#define	__MAX_NAME_STR_BD_LOCATION_	256	//	refer to __MAX_NAME_STR_BD_LOCATION__

#define	SUB_BANK_MAX_BYTE_SIZE		0x100000					/* 1 MBytes */
#define	HW_BANK_NUMBER				7							/* Use 1 bank, 0~7 for 1MB - 8MB */
#define	MAX_BUFFER_BYTE_SIZE		SUB_BANK_MAX_BYTE_SIZE*3	/* */

#define G704_FRAME_SIZE				32
#define	G704_BLOCK_SIZE				(G704_FRAME_SIZE << 3)
#define G704_SYNC					0x1B
#define G704_SYNC_MASK				0x7F
#define	BCNT(x)						((x >> 5) & 0x7)
#define	INCB(x)						((x + 1) % 8)
#define	TS1							1
#define	TS1_N						(TS1 + G704_BLOCK_SIZE)

#endif