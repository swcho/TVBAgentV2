//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#ifdef _LINUX_
#include <sys/time.h>
#include <unistd.h>
#endif 

#include "corelib.h"
#include "hldplay.h"
#include "output.h"
// multiplexers
#include "isdbt_multiplexer.h"
#include "dvbt2_multipleplp.h"
#include "dvbc2_multipleplp.h"

#ifdef WIN32
#else
#include	"../../include/lld_const.h"
#endif


FILE *t2_fp;
int Output::main_loop(void *param)
{
	sys_play = (CHldPlayback *)param;
#if 0	
	t2_fp = fopen("/home/televiewuser/TS/t2test.t2", "wb");
	if(t2_fp == NULL)
	{
		printf("T2 TEst file open error\n");
	}
#else
	t2_fp = NULL;
#endif

_MULTIPLEXING_:
	tlv_mutex_lock(&sys_play->mutex_cir_multiplexed_data);
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT)
	{
		tlv_mutex_unlock(&sys_play->mutex_cir_multiplexed_data);
		goto _END_1_;
	}		
	tlv_mutex_unlock(&sys_play->mutex_cir_multiplexed_data);

	tlv_mutex_lock(&sys_play->thread_producer.mutex_thread_state);
	if (sys_play->thread_producer.thread_state == THREAD_STATE_READING_INPUT)
	{
		tlv_mutex_unlock(&sys_play->thread_producer.mutex_thread_state);
		goto _MULTIPLEXING_;
	}
	
	tlv_mutex_unlock(&sys_play->thread_producer.mutex_thread_state);

	tlv_mutex_lock(&sys_play->thread_consumer.mutex_thread_state);
	sys_play->thread_consumer.thread_state = THREAD_STATE_READY;
	tlv_mutex_unlock(&sys_play->thread_consumer.mutex_thread_state);

	init(sys_play);

	do
	{
		if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT)
				goto _END_;
		consume_item(NULL, MULTIPLEXER_BUFF_ITEM_SIZE);			
		Sleep(20);
	}while (1);

_END_:
	if(t2_fp != NULL)
	{
		fclose(t2_fp);
		t2_fp = NULL;
	}
	quit();

_END_1_: // end because of user...

	//printf("[HLD] output Thread ending .....\n");

	tlv_mutex_lock(&sys_play->thread_consumer.mutex_thread_state);
	sys_play->thread_consumer.thread_state = THREAD_STATE_FINISH;
	tlv_mutex_unlock(&sys_play->thread_consumer.mutex_thread_state);
	return 0;
}


CHldBoard597V2::CHldBoard597V2()
{
	strcpy(&name[0], "output, base for 597V2 board");
}

CHldBoard597V2::~CHldBoard597V2()
{

}

#ifndef _TEST_

#include "hldmgr.h"
int CHldBoard597V2::TSPL_SET_PLAY_RATE(long play_freq, long use_max_play_rate)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_PLAY_RATE(play_freq, use_max_play_rate);

}
int CHldBoard597V2::TSPL_SET_TSIO_DIRECTION(int direction)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_TSIO_DIRECTION(direction);
}
int CHldBoard597V2::TSPL_GET_TSIO_DIRECTION()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_GET_TSIO_DIRECTION();
}
int CHldBoard597V2::TSPL_SET_SDRAM_BANK_CONFIG(int nBankCount)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_SDRAM_BANK_CONFIG(nBankCount);	
}
int CHldBoard597V2::TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankOffset)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(nBankOffset);	
}
int CHldBoard597V2::TSPL_SET_SDRAM_BANK_INFO(int nBankCount, int nBankOffset)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_SDRAM_BANK_INFO(nBankCount, nBankOffset);	
}
int CHldBoard597V2::TSPL_GET_DMA_STATUS()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT || lld->GetFinishAsi())
			return 1;
	return rdwr->TSPL_GET_DMA_STATUS();	
}
void* CHldBoard597V2::TSPL_GET_DMA_ADDR()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_GET_DMA_ADDR();	
}
int CHldBoard597V2::TSPL_SET_SDCON_MODE(int mode)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_SET_SDCON_MODE(mode);
}
#ifdef WIN32
int CHldBoard597V2::TSPL_WRITE_BLOCK(DWORD *srcBuff, unsigned long buffSize, DWORD *des)
#else
int CHldBoard597V2::TSPL_WRITE_BLOCK(unsigned int *srcBuff, unsigned long buffSize, unsigned int *des)
#endif
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT || lld->GetFinishAsi())
			return 0;
#if defined(WIN32)
	rdwr->TSPL_WRITE_BLOCK(NULL, buffSize, des);
	return 0;
#else
	if(t2_fp != NULL)
	{
		fwrite(srcBuff, 1, buffSize, t2_fp);
	}
	return rdwr->TSPL_WRITE_BLOCK(srcBuff, buffSize, 0);
#endif
}

//2012/7/5 DM ==============================================
void CHldBoard597V2::call_SetHwDmaDiection_Play()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT || lld->GetFinishAsi())
			return;

	rdwr->SetHwDmaDiection_Play();
}
void CHldBoard597V2::call_SetHwFifoCntl_()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT || lld->GetFinishAsi())
			return;
	rdwr->SetHwFifoCntl_(0, rdwr->SdramSubBankSize());
}
int CHldBoard597V2::call_WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT || lld->GetFinishAsi())
			return 0;
	return rdwr->WaitConsumePlayBuf_toMaxLevel_DvbT2(200);
}

//===============================================

int CHldBoard597V2::TSPL_GET_CUR_BANK_GROUP()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	return rdwr->TSPL_GET_CUR_BANK_GROUP();	
}
#endif


void CHldBoard597V2::init( CHldPlayback *_sys_play )
{
	sys_play = _sys_play;

// todo: now, it is just support max playing 

	double bitrate = 11111; // _dvbt2_mutilpexer->get_bitrate();
	long clock_src = 1; // using maximun bitrate

	hw_bank_index = 0;
	hw_bank_number = 7;
	hw_bank_offset = 1024;

#if 0
	if ( TSPL_SET_SDRAM_BANK_INFO(hw_bank_number, hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank_info  function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( TSPL_SET_SDRAM_BANK_CONFIG(hw_bank_number) == -1 )
	{		
//		printf("assert: canot set bank config: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank offset function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return; 
	}

	//printf(" ===== set bitrate %f \n", bitrate);
	if ( TSPL_SET_PLAY_RATE(bitrate, clock_src) == -1 )
	{
		// observer thread 
//		printf("assert: canot set play rate function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_TSIO_DIRECTION(0) == -1 )
	{
//		printf("assert: canot set io direction function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
//		printf("assert: canot set mode function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
#endif
	dma_buffer = TSPL_GET_DMA_ADDR();
	hw_group_index = TSPL_GET_CUR_BANK_GROUP();
	if (hw_group_index == 0xffff)
	{
//		printf("assert: hw_group_index is wrong function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
}


void CHldBoard597V2::quit()
{
}


int CHldBoard597V2::consume_item( char *buff, int size )
{

	int nBankBlockSize = hw_bank_offset << 10; // multiple with 1024
	//if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() == 1)
	//{
	//	nBankBlockSize = nBankBlockSize / 2;
	//}
	sys_play->multiplex_interface->read_n_byte((char *)dma_buffer, nBankBlockSize);
#if 0
#if 0 // LINUX 
	struct timeval time;
	gettimeofday(&time, NULL);

	// for evaluation performance of making t2mi
	static int num_bit_read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static long duration_miliseconds[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static int index_tmp = 0;
	static struct timeval end = time;
	struct timeval start;
	long mtime, seconds, useconds;

	start = end; // get the old time
	end = time;

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	duration_miliseconds[index_tmp] = seconds*1000 + useconds/1000;
	num_bit_read[index_tmp] = size*8;
#else // window 

	SYSTEMTIME system_time;
	GetLocalTime(&system_time);

	// for evaluation performance of making t2mi
	static int num_bit_read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static long duration_miliseconds[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static int index_tmp = 0;
	static SYSTEMTIME end = system_time;
	SYSTEMTIME start;
	long hour, minute, seconds, milisecond, useconds;
	char str_tmp[128];
	start = end;
	end = system_time;

	hour = (end.wHour - start.wHour)*60*60*1000;
	minute = (end.wMinute - start.wMinute)*60*1000;
	seconds = (end.wSecond - start.wSecond)*1000;
	milisecond = end.wMilliseconds - start.wMilliseconds;


	duration_miliseconds[index_tmp] =  hour + minute + seconds + milisecond;
	num_bit_read[index_tmp] = (size - 2)*8;

#endif 
	sprintf(str_tmp, "[HLD] [Consumer] Time %d ========\n", duration_miliseconds[index_tmp]);
	OutputDebugString(str_tmp);
	index_tmp = ++index_tmp%10;
	long bitrate;
	long sum_miliseconds = 0;
	int sum_bits_read = 0;

	for (int i = 0; i < 10; i++)
	{
		sum_miliseconds = sum_miliseconds + duration_miliseconds[i];
		sum_bits_read = sum_bits_read + num_bit_read[i];
	}

	//printf("[HLD] [Consumer] bitrate %fK ========\n", ((sum_bits_read*1.0)/sum_miliseconds)*1000/1024);
#endif


/*

	if (item_state == PLAY_ITEM_STATE_EOF)
	{
		tlv_message_t message;
		message.id = PLAYBACK_USER_STATE_STOP_STREAMING;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
		// notify STOP_STREAMMING 
		return size;
	}

	if (item_count == 0)
	{
		tlv_message_t message;
		message.id = PLAYBACK_USER_STATE_START_STREAMING;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
		// notify START_STREAMMING 
	}

*/

#if 1
	int mutexLock = 0;
	if(sys_play->plld->_SysSta->IsAttachedTvbTyp_599() || sys_play->plld->_SysSta->IsAttachedTvbTyp_598())
	{
		if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() || sys_play->plld->_SysSta->IsState_DvbC2LoopThru())
		{
			//sys_play->plld->_FIf->LockFileMutex();
			//2012/7/5
			if( call_WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile() == 0)
			{
				//sys_play->plld->_FIf->UnlockFileMutex();
				goto _END_;
			}
		
			sys_play->plld->_FIf->LockFileMutex();
			mutexLock = 1;
			call_SetHwFifoCntl_();
			call_SetHwDmaDiection_Play();
		}
		if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() == 0 && sys_play->plld->_SysSta->IsState_DvbC2LoopThru() == 0)
		{
			int dwAddrDestBoardSDRAM = (hw_group_index*BANK_SIZE_4) +  hw_bank_index*(SUB_BANK_MAX_BYTE_SIZE >> 2);  
			TSPL_WRITE_BLOCK( (unsigned int *)dma_buffer, (unsigned long )nBankBlockSize, (unsigned int *)dwAddrDestBoardSDRAM);
		}
		else
		{
			TSPL_WRITE_BLOCK( (unsigned int *)dma_buffer, (unsigned long )nBankBlockSize, (unsigned int *)0);
		}

		while (TSPL_GET_DMA_STATUS() == 0) 
		{
             	Sleep(10);
			if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT)
			{
				//if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() || sys_play->plld->_SysSta->IsState_DvbC2LoopThru())
				if(mutexLock == 1)
				{
					sys_play->plld->_FIf->UnlockFileMutex();
					mutexLock = 0;
				}
				goto _END_;
			}
		}; // wait until the end of DMA process
		//if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() || sys_play->plld->_SysSta->IsState_DvbC2LoopThru())
		if(mutexLock == 1)
		{
			sys_play->plld->_FIf->UnlockFileMutex();
		}
	}
	else
	{
		if(sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbT2LoopThru() || sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbC2LoopThru())
		{
			//sys_play->plld->_FIf->LockFileMutex();
			//2012/7/5
			if( call_WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile() == 0)
			{
				//sys_play->plld->_FIf->UnlockFileMutex();
				goto _END_;
			}
		
			sys_play->plld->_FIf->LockFileMutex();
			mutexLock = 1;
			call_SetHwFifoCntl_();
			call_SetHwDmaDiection_Play();
		}
		if(sys_play->plld->_SysSta->IsState_DvbT2LoopThru() == 0 && sys_play->plld->_SysSta->IsState_DvbC2LoopThru() == 0)
		{
			int dwAddrDestBoardSDRAM = (hw_group_index*BANK_SIZE_4) +  hw_bank_index*(SUB_BANK_MAX_BYTE_SIZE >> 2);  
			TSPL_WRITE_BLOCK( (unsigned int *)dma_buffer, (unsigned long )nBankBlockSize, (unsigned int*)dwAddrDestBoardSDRAM);
		}
		else
		{
			TSPL_WRITE_BLOCK( (unsigned int *)dma_buffer, (unsigned long )nBankBlockSize, (unsigned int *)0);
		}

		while (TSPL_GET_DMA_STATUS() == 0) {
		
             	Sleep(10);
			if (sys_play->playback_control == PLAYBACK_CONTROL_END_OUTPUT)
			{
				//if(sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbT2LoopThru() || sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbC2LoopThru())
				if(mutexLock == 1)
				{
					sys_play->plld->_FIf->UnlockFileMutex();
					mutexLock = 0;
				}
				goto _END_;
			}
		}; // wait until the end of DMA process
		//if(sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbT2LoopThru() || sys_play->plld->_SysSta->IsAttachedTvbTyp_SupportDvbC2LoopThru())
		if(mutexLock == 1)
		{
			sys_play->plld->_FIf->UnlockFileMutex();
		}
	}
#endif


	hw_bank_index++;
	hw_bank_index = (hw_bank_index == (hw_bank_number + 1) ? 0: hw_bank_index);
	if (hw_bank_index == 0)
	{
		hw_group_index = (hw_group_index + 1) & 0x01;
	}

_END_:
	return size;
}


OutputNULL::OutputNULL()
{
	strcpy(&name[0], "output, null (do nothing), useful for evaluating performance .... ");
}

OutputNULL::~OutputNULL()
{

}

void OutputNULL::init( CHldPlayback *_sys_play )
{

}

void OutputNULL::quit()
{

}

int OutputNULL::consume_item( char *buff, int size )
{

	static char tmp_buff[1024*1024 + 10]; 
	int rc = this->sys_play->multiplex_interface->read_n_byte(&tmp_buff[0], size);
#if 1

#if defined(WIN32) // LINUX 
	
	SYSTEMTIME system_time;
	GetLocalTime(&system_time);

	// for evaluation performance of making t2mi
	static int num_bit_read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static long duration_miliseconds[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static int index_tmp = 0;
	static SYSTEMTIME end = system_time;
	SYSTEMTIME start;
	long hour, minute, seconds, milisecond, useconds;

	start = end;
	end = system_time;

	hour = (end.wHour - start.wHour)*60*60*1000;
	minute = (end.wMinute - start.wMinute)*60*1000;
	seconds = (end.wSecond - start.wSecond)*1000;
	milisecond = end.wMilliseconds - start.wMilliseconds;


	duration_miliseconds[index_tmp] =  hour + minute + seconds + milisecond;
	num_bit_read[index_tmp] = rc*8;

#else // window 
	struct timeval time;
	gettimeofday(&time, NULL);

	// for evaluation performance of making t2mi
	static int num_bit_read[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static long duration_miliseconds[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static int index_tmp = 0;
	static struct timeval end = time;
	struct timeval start;
	long mtime, seconds, useconds;

	start = end; // get the old time
	end = time;

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	duration_miliseconds[index_tmp] = seconds*1000 + useconds/1000;
	num_bit_read[index_tmp] = rc*8;

#endif 
	index_tmp = ++index_tmp%10;
	long bitrate;
	long sum_miliseconds = 0;
	int sum_bits_read = 0;

	for (int i = 0; i < 10; i++)
	{
		sum_miliseconds = sum_miliseconds + duration_miliseconds[i];
		sum_bits_read = sum_bits_read + num_bit_read[i];
	}

	//printf("[output]  bitrate %fK ========\n", ((sum_bits_read*1.0)/sum_miliseconds)*1000/1024);
#endif
	return size;
}
/////////////////////////////////////////////////

CHldBoardDVBT2::CHldBoardDVBT2()
{
	strcpy(&name[0], "output, dvbt2 (t2mi packet) ");
}

CHldBoardDVBT2::~CHldBoardDVBT2()
{

}

void CHldBoardDVBT2::init( CHldPlayback *_sys_play )
{
	sys_play = _sys_play;


/*
	DVBT2Multiplexer *_dvbt2_mutilpexer = (DVBT2Multiplexer *)sys_play->multiplex_interface;
	double bitrate1 = _dvbt2_mutilpexer->get_bitrate();
	printf("===== multiplexer -bitrate: %f \n", bitrate1);
*/	
	double bitrate = 11111; // _dvbt2_mutilpexer->get_bitrate();
	long clock_src = 1; // using maximun bitrate

	hw_bank_index = 0;
	hw_bank_number = 7;
	hw_bank_offset = (sys_play->plld->_FIf->SdramSubBankSize() >> 10);


#if 0
	if ( TSPL_SET_SDRAM_BANK_INFO(hw_bank_number, hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank info function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( TSPL_SET_SDRAM_BANK_CONFIG(hw_bank_number) == -1 )
	{		
//		printf("assert: canot set bank config function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank offset function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}

	//printf(" ===== set bitrate %f \n", bitrate);
	if ( TSPL_SET_PLAY_RATE(bitrate, clock_src) == -1 )
	{
				// observer thread 
//		printf("assert: canot set bitrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	// ???: this sentence makes sense 
	if ( TSPL_SET_TSIO_DIRECTION(0) == -1 )
	{
//		printf("assert: canot set io direction function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
//		printf("assert: canot set mode function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
#endif
	dma_buffer = TSPL_GET_DMA_ADDR();
	hw_group_index = TSPL_GET_CUR_BANK_GROUP();
	if (hw_group_index == 0xffff)
	{
//		printf("assert: hw_group_index is wrong, function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
}



CHldFileWriter::CHldFileWriter(void)
{
	strcpy(&name[0], "output, file) ");

}

CHldFileWriter::~CHldFileWriter(void)
{

}

void CHldFileWriter::WriteFileSize(int size_)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	rdwr->TL_i64TotalFileSize = rdwr->TL_i64TotalFileSize + size_;
}
void CHldFileWriter::init(CHldPlayback *core_broadcast)
{

	tmp_buff = (char *)malloc(1024*1024 + 10); 

	//printf(" output_file plugin, init \n");

	strcpy(&file_path[0], &core_broadcast->file_path_for_writing[0]);
	file_handler = fopen(&file_path[0], "wb");
	if (file_handler == NULL)
	{
//		printf("assert: canot open a file function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}

}

void CHldFileWriter::quit()
{
	//printf("output_file plugin, quit \n");
	int rtcd = fclose(file_handler);
	if (rtcd != 0)
	{
		// observer thread 
		tlv_message_t message;
		message.id = PLAYBACK_LOG_MESSAGE;
		message.param = NULL;
		tlv_mq_send(&sys_play->thread_observer, &message);
	}

	free(tmp_buff);
}

int CHldFileWriter::consume_item(char *buff, int size)
{
	int rc = sys_play->multiplex_interface->read_n_byte(&tmp_buff[0], size);
	fwrite(&tmp_buff[0], 1, size, file_handler);
	WriteFileSize(size);
	return size;

}


CHldBoardISDBT::CHldBoardISDBT(void)
{
	strcpy(&name[0], "output, isdb-t 1seg, muxing ");
}

CHldBoardISDBT::~CHldBoardISDBT(void)
{

}


void CHldBoardISDBT::init(CHldPlayback *_sys_play)
{
	sys_play = _sys_play;

	
	ISDBTMultiplexer *_isdbt_mutilpexer = (ISDBTMultiplexer *)sys_play->multiplex_interface;
	double bitrate = _isdbt_mutilpexer->CalculateBitrate();
	long clock_src = 0; // using maximun bitrate


	hw_bank_index = 0;
	hw_bank_number = 7;
	hw_bank_offset = 1024;
	
#if 0
	if ( TSPL_SET_SDRAM_BANK_INFO(hw_bank_number, hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank info function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( TSPL_SET_SDRAM_BANK_CONFIG(hw_bank_number) == -1 )
	{		
//		printf("assert: canot set bank config function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank offset function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}


	//printf(" ===== set bitrate %f \n", bitrate);
	if ( TSPL_SET_PLAY_RATE(bitrate, clock_src) == -1 )
	{
//		printf("assert: canot set playrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	// ???: this sentence makes sense 
	if ( TSPL_SET_TSIO_DIRECTION(TSPL_GET_TSIO_DIRECTION()) == -1 )
	{
//		printf("assert: canot set io direction function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
//		printf("assert: canot set sdcon function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
#endif
	dma_buffer = TSPL_GET_DMA_ADDR();
	hw_group_index = TSPL_GET_CUR_BANK_GROUP();
	if (hw_group_index == 0xffff)
	{
//		printf("assert: hw_group_index is wrong  function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
}



CHldBoardISDBTUsingTMCC::CHldBoardISDBTUsingTMCC(void)
{
	strcpy(&name[0], "output, isdb-t 1seg, no muxing ");
}

CHldBoardISDBTUsingTMCC::~CHldBoardISDBTUsingTMCC(void)
{

}


void CHldBoardISDBTUsingTMCC::init(CHldPlayback *_sys_play)
{

	sys_play = _sys_play;

	
	double bitrate = 0;
	long clock_src = 0; // using maximun bitrate


	hw_bank_index = 0;
	hw_bank_number = 7;
	hw_bank_offset = 1024;
	

#if 0
	if ( TSPL_SET_SDRAM_BANK_INFO(hw_bank_number, hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank info function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( TSPL_SET_SDRAM_BANK_CONFIG(hw_bank_number) == -1 )
	{		
//		printf("assert: canot set bank config function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank offset function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}


	//printf(" ===== set bitrate %f \n", bitrate);
	if ( TSPL_SET_PLAY_RATE(bitrate, clock_src) == -1 )
	{
//		printf("assert: canot set playrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	// ???: this sentence makes sense 
	if ( TSPL_SET_TSIO_DIRECTION(TSPL_GET_TSIO_DIRECTION()) == -1 )
	{
//		printf("assert: canot set tsio direction function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
//		printf("assert: canot set sdcod mode function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
#endif
	dma_buffer = TSPL_GET_DMA_ADDR();
	hw_group_index = TSPL_GET_CUR_BANK_GROUP();
	if (hw_group_index == 0xffff)
	{
//		printf("assert: hw_group_index is wrong, function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
}


CHldBoardDVBC2::CHldBoardDVBC2()
{
	strcpy(&name[0], "output, dvbc2 (c2mi packet) ");
}

CHldBoardDVBC2::~CHldBoardDVBC2()
{

}

void CHldBoardDVBC2::init( CHldPlayback *_sys_play )
{
	sys_play = _sys_play;


/*
	DVBT2Multiplexer *_dvbt2_mutilpexer = (DVBT2Multiplexer *)sys_play->multiplex_interface;
	double bitrate1 = _dvbt2_mutilpexer->get_bitrate();
	printf("===== multiplexer -bitrate: %f \n", bitrate1);
*/	
	double bitrate = 11111; // _dvbt2_mutilpexer->get_bitrate();
	long clock_src = 1; // using maximun bitrate

	hw_bank_index = 0;
	hw_bank_number = 7;
	hw_bank_offset = 1024;


#if 0
	if ( TSPL_SET_SDRAM_BANK_INFO(hw_bank_number, hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank info function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( TSPL_SET_SDRAM_BANK_CONFIG(hw_bank_number) == -1 )
	{		
//		printf("assert: canot set bank config function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(hw_bank_offset) == -1 )
	{		
//		printf("assert: canot set bank offset function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}

	//printf(" ===== set bitrate %f \n", bitrate);
	if ( TSPL_SET_PLAY_RATE(bitrate, clock_src) == -1 )
	{
				// observer thread 
//		printf("assert: canot set bitrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	// ???: this sentence makes sense 
	if ( TSPL_SET_TSIO_DIRECTION(0) == -1 )
	{
//		printf("assert: canot set io direction function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	if ( TSPL_SET_SDCON_MODE(TSPL_SDCON_PLAY_MODE) == -1 )
	{
//		printf("assert: canot set mode function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
#endif
	dma_buffer = TSPL_GET_DMA_ADDR();
	hw_group_index = TSPL_GET_CUR_BANK_GROUP();
	if (hw_group_index == 0xffff)
	{
//		printf("assert: hw_group_index is wrong, function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return;
	}
	
}

