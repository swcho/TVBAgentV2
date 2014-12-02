//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#ifndef	_TLV_OUTPUT_
#define _TLV_OUTPUT_

#include "corelib.h"

#define BANK_SIZE_4	0x200000	// 8 Mbyte is 2M x 32bit 


class CHldPlayback;
class Output{
	public:
		CHldPlayback *sys_play;
		char name[256];
	public:
		// memory allocation
		// initialization of target hardward
		virtual void init(CHldPlayback *sys_play) = 0;
		virtual void quit() = 0;	
		// 
		virtual int consume_item(char *buff, int size) = 0;	

		int main_loop(void *param);
};



// using abstract base class as a method for ......

class CHldFileWriter : public Output{

	private:
		char file_path[256];
		FILE *file_handler;
		char *tmp_buff;
	public:
		CHldFileWriter();
		~CHldFileWriter();

		void init(CHldPlayback *core_broadcast);
		int consume_item(char *buff, int size);
		void quit();
		void WriteFileSize(int size_);
};


class CHldBoard597V2: public Output{

	public:

		int hw_group_index;
		int hw_bank_index;

		int hw_bank_number; 
		int hw_bank_offset;

		void *dma_buffer;

	public:
		CHldBoard597V2();
		~CHldBoard597V2();

		void init(CHldPlayback *core_broadcast);
		int consume_item(char *buff, int size);
		void quit();
#ifndef _TEST_
		// for interfacing with HLD 
		int TSPL_SET_PLAY_RATE(long play_freq, long use_max_play_rate);
		int TSPL_SET_TSIO_DIRECTION(int direction);
		int TSPL_GET_TSIO_DIRECTION();
		int TSPL_SET_SDRAM_BANK_CONFIG(int nBankCount);
		int TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankOffset);
		int TSPL_SET_SDRAM_BANK_INFO(int nBankCount, int nBankOffset);
		int TSPL_GET_DMA_STATUS();
		void* TSPL_GET_DMA_ADDR();
		int TSPL_SET_SDCON_MODE(int nMode);
#ifdef WIN32
		int TSPL_WRITE_BLOCK(DWORD *srcBuff, unsigned long buffSize, DWORD *des);
#else
		int TSPL_WRITE_BLOCK(unsigned int *srcBuff, unsigned long buffSize, unsigned int *des);
#endif
		int TSPL_GET_CUR_BANK_GROUP();
		
		//2012/7/5 DM ========================================================================
		void call_SetHwDmaDiection_Play(void);
		void call_SetHwFifoCntl_();
		int	call_WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile();
		//====================================================================================
#endif
};
// do not write to board 
class OutputNULL: public Output
{

	public:
		OutputNULL();
		~OutputNULL();
		void init(CHldPlayback *core_broadcast);
		int consume_item(char *buff, int size);
		void quit();
};

class CHldBoardDVBT2: public CHldBoard597V2
{

	public:
		CHldBoardDVBT2();
		~CHldBoardDVBT2();

		void init(CHldPlayback *core_broadcast);
};

class CHldBoardISDBT: public CHldBoard597V2
{

	public:
		CHldBoardISDBT();
		~CHldBoardISDBT();

		void init(CHldPlayback *core_broadcast);
};


// for playing muxed file, using tmcc in file 
// this is nonsense class, it will be delete when multiplexer interface is re-designed 
class CHldBoardISDBTUsingTMCC: public CHldBoard597V2{


	public:
		CHldBoardISDBTUsingTMCC();
		~CHldBoardISDBTUsingTMCC();

		void init(CHldPlayback *core_broadcast);
};

class CHldBoardDVBC2: public CHldBoard597V2
{

	public:
		CHldBoardDVBC2();
		~CHldBoardDVBC2();

		void init(CHldPlayback *core_broadcast);
};

#endif
