#ifndef _tsp100_header_h
#define _tsp100_header_h

//	
//	TSP100.h / TSP100DLL.DLL
//
//	TSP100 main header file
//
//	Copyright (C) 2000-20001
//	Teleview Corporation
//	
//
//	Revisioin history
//	17-Apr-2000	1.0.0	CC	creation
//

#define	_x86_

//
//	Error messages to upper API
//
#define TLV_NO_ERR			0
#define	TLV_UNKNOWN_ERR			-1
#define TLV_NO_DRIVER			-2
#define TLV_NO_DEVICE			-3
#define TLV_NO_RBF			-4
#define TLV_FILE_READ_ERR		-5
#define TLV_DRV_ERR			-6
#define TLV_DOWNLOAD_ERR		-7
#define TLV_DMAM_ALLOC_ERR		-8
#define	TLV_NO_TS_SYNC_ERR		-11
#define	TLV_TOO_BIG_FILE_TO_WRITE_ERR	-12
#define	TLV_FAIL_TO_START_PLAY_THREAD	 -13
#define	TLV_FAIL_TO_START_RECORD_THREAD	-14
#define	TLV_FAIL_TO_STOP_DRV		-15
#define	TLV_NO_DRV_FOR_SET_MODE		-16
#define	TLV_FAIL_TO_CLOSE_TSP_DRV	-17
#define	TLV_ALTERA_FILE_READ_ERR	-18
#define	TLV_FAIL_TO_CREATE_LOG_FILE	-19
#define	TLV_FAIL_TO_CREATE_RECORD_FILE	-20
#define	TLV_INVALID_ARGUMENT		-21
#define	TLV_FAIL_TO_STOP_PLAY		-22
#define	TLV_FAIL_TO_CONFIRE_EPLD	-23
#define	TLV_RS232C_TX_ERROR		-24
#define	TLV_FAIL_TO_START_MONITOR_THREAD	-25
#define	TLV_EXIT_SYSTEM			-99
#define	TLV_DEBUG_ERR			-100

#endif
