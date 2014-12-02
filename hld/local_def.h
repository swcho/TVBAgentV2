
#ifndef __LOCAL_DEF_H__
#define __LOCAL_DEF_H__

#if defined(WIN32)
#else
#define MAX_PATH 1024
#endif

//--------------------------------------------------------------------------
#if defined(WIN32)
#define	_MAX_INST_CNT_	23
#define	_MAX_IND_PCI_BD_	19		//	_CNT_MAX_PCI_BD_INSTALL_AVAILABLE_
#else
#define	_MAX_INST_CNT_	12 //==MAX_TSP_DEVICE in tsp100.h
#define	_MAX_IND_PCI_BD_	8		//	_CNT_MAX_PCI_BD_INSTALL_AVAILABLE_
#endif

#ifdef	SUB_BANK_BYTE_SIZE
#undef	SUB_BANK_BYTE_SIZE
#endif
#define	SUB_BANK_BYTE_SIZE	(iHW_BANK_OFFSET << 10)

//--------------------------------------------------------------------------
//--- Constant and Types
#define	SUB_BANK_NUM			(iHW_BANK_NUMBER+1)		// 8 : MODIFIED BY SWPARK 
#define	MAX_BANK_NUMBER			8
#define BANK_SIZE_4				0x200000				// 8 Mbyte is 2M x 32bit 
#define	DISK_RW_UNIT_SIZE		SUB_BANK_BYTE_SIZE		// 0x10000(1Mbyte) MODIFIED BY SWPARK
#define	SUB_BANK_MAX_BYTE_SIZE	0x100000				// 1 MBYTE
#define	DMA_TRANS_UNIT_CAP_PLAY	(SUB_BANK_MAX_BYTE_SIZE/2)	//	(SUB_BANK_MAX_BYTE_SIZE/1)
#define	PLAY_BUF_MAX_LVL_when_LOOPTHTU	(SUB_BANK_MAX_BYTE_SIZE*2)
#define	PLAY_BUF_MAX_LVL_when_LOOPTHTU_3M	(SUB_BANK_MAX_BYTE_SIZE * 3)// + (SUB_BANK_MAX_BYTE_SIZE / 2))

#define DVBT2_LOOPTHRU_ENABLE_BUFFER (SUB_BANK_MAX_BYTE_SIZE*4)
#define DVBT2_LOOPTHRU_LEVEL (SUB_BANK_MAX_BYTE_SIZE*2)

#define	NEED_FILL_TO_STREAM_OUT					(SUB_BANK_NUM * 2 - 1)
#define	BUFF_STS_BUFFERING						0
#define	BUFF_STS_PLAY_STREAM_OUT				1
#define	BUFF_STS_THERE_IS_LAST_STREAM_TO_OUT	2
#define	BUFF_STS_COMPLETED_A_STREAM_OUT_NOW		3

//DVB-T2 - Multi PLP
#define MAX_PLP_COUNT 256

//--------------------------------------------------------------------------

typedef struct tTsPlayList
{
	char	szfn[MAX_PATH];
	long	nPlayRate;
	long	nOutputClockSource;
} _TSPLAYLIST;

typedef	struct	tPlayParm
{
	struct	tTsPlayList	AP_lst;
	int		AP_fRepeat;	// Set by API
} _PLAYPARAM;

typedef	struct	_buf_queue_config
{
	int	stream_progressing;
	int	stream_pos_1st;
	int	stream_pos_last;
} _BUF_QUEUE_CONFIG;

#endif

