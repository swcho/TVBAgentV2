//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include "corelib.h"
#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "mpeg2ts.h"
#include "input_buffer.h"
#include "tlv_type.h"

#ifdef WIN32
#else
#include	"../../include/lld_const.h"
#endif

#ifndef _NO_IP_BUFFER_
#include "LLDWrapper.h"
#endif

CTLVBuffer::CTLVBuffer()
{
	option.num_filter_pids = 0;
	strcpy(&name[0], "input, receiving data from  IP \n");
}
CTLVBuffer::~CTLVBuffer()
{

}

int CTLVBuffer::read_data(char *buff_tmp, int max_num_byte)
{

	int bytes;
	char buff[BUFFER_TS_DATA_SIZE_MAX + 100]; 

#ifndef _NO_IP_BUFFER_
	if (!sys_play->plld->IsThere_EnoughIpDta() && is_start)
	{
		is_start = 0;
		return 0;
	}

	sys_play->plld->LockIpMutex_Cntrl();
	if (sys_play->plld->_SysSta->IPUsing() == 0)
	{
		sys_play->plld->UnlockIpMutex_Cntrl();
		return -1; 
	}
	int num_byte_need_read = (BUFFER_TS_DATA_SIZE_MAX > max_num_byte? max_num_byte: UDP_DATA_SIZE_MAX);
	bytes = sys_play->plld->Get_RcvIpDta((unsigned char*)&buff[0], num_byte_need_read);	
	sys_play->plld->UnlockIpMutex_Cntrl();
	
	if (bytes > 0)
	{

		tlv_mutex_lock(mutex_cir_data);
		cir_input_ts_buffer->BufferNItem(&buff[0], bytes);
		int num_buffered_bytes = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();		

		// signal 
		tlv_cond_signal(&cv_cir_out);

		// and waiting 
		tlv_cond_wait(&cv_cir_in, mutex_cir_data);
		tlv_mutex_unlock(mutex_cir_data);		

		return bytes;

		
	}else
	{
		//printf("%s === read bytes: (want: %d, return: %d) \n", __FILE__, num_byte_need_read, bytes);
	}
#endif
	return bytes;
}

int CTLVBuffer::_read_mpeg2ts_data(char *buff)
{
//#if defined(WIN32)
#if 0
	int bytes;
_START_:

	if (!sys_play->plld->IsThere_EnoughIpDta() && is_start)
	{
		is_start = 0;
		return 0;
	}

	sys_play->plld->LockIpMutex_Cntrl();
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_INPUT)
	{
		sys_play->plld->UnlockIpMutex_Cntrl();
		return -1; 
	}
	if (sys_play->plld->_SysSta->IPUsing() == 0)
	{
		sys_play->plld->UnlockIpMutex_Cntrl();
		return -1; 
	}
	bytes = sys_play->plld->Get_RcvIpDta((unsigned char*)buff, 1024*3);	
	sys_play->plld->UnlockIpMutex_Cntrl();
	if (bytes == 0)
	{
		// un-use 
		//leep(10);
		goto _START_;
	}

	return bytes;
#endif
	return 0;
}

void CTLVBuffer::open(void *_path_file)
{

	mpeg2ts_statistic.packet_size = 188;
	mpeg2ts_statistic.bitrate = sys_play->media_interface[0]->option.bitrate;

	cir_input_ts_buffer = new CCircularBuffer(1, (int)(mpeg2ts_statistic.bitrate*0.5)/8);
	is_start = 1;

	tlv_message_t message;
	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);

	//printf("==================input-buffer plug-in, =============\n");
	//printf("bitrate : %f \n", mpeg2ts_statistic.get_bitrate());
	//printf("=====================================================\n");


}
void CTLVBuffer::close(void *arg)
{
	delete cir_input_ts_buffer;
}

// not yet support 
int CTLVBuffer::read_n_byte(char *des, int num_byte)
{

	return 0; 

}

int CTLVBuffer::read_one_packet(char *buff)
{
_WAITING_:
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < mpeg2ts_statistic.get_packet_size())
		{
			// signal
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem(buff, mpeg2ts_statistic.get_packet_size());
		tlv_mutex_unlock(mutex_cir_data);
		return rc;
	}
}

void CTLVBuffer::force_close()
{

}

//2012/7/6 DVB-T2 ASI =============================================================================================================================================================
CTS_IN_Buffer::CTS_IN_Buffer()
{
	option.num_filter_pids = 0;
	sync_pos = -1;
	read_data_size = 0;
	strcpy(&name[0], "input, receiving data from  IP \n");
}
CTS_IN_Buffer::~CTS_IN_Buffer()
{
	sync_pos = -1;
	read_data_size = 0;
}

int CTS_IN_Buffer::read_data(char *buff_tmp, int max_num_byte)
{

	int bytes;

	char buff[BUFFER_TS_DATA_SIZE_MAX + 100]; 

	sys_play->plld->_FIf->LockBufferMutex();
	int num_byte_need_read = (BUFFER_TS_DATA_SIZE_MAX > max_num_byte? max_num_byte: UDP_DATA_SIZE_MAX);
	bytes = sys_play->plld->GetAsiInputData((PVOID)sys_play->plld, (unsigned char*)&buff[0], num_byte_need_read); 
	sys_play->plld->_FIf->UnlockBufferMutex();
	if (bytes > 0)
	{
		tlv_mutex_lock(mutex_cir_data);
		cir_input_ts_buffer->BufferNItem_ASI(&buff[0], bytes);
		int num_buffered_bytes = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();		
		// signal 
		tlv_cond_signal(&cv_cir_out);
		// and waiting 
		tlv_cond_wait(&cv_cir_in, mutex_cir_data);
		tlv_mutex_unlock(mutex_cir_data);
	}
	else
	{
		Sleep(10);
		//Sleep(10);
	}
	return bytes;
}

int CTS_IN_Buffer::_read_mpeg2ts_data(char *buff)
{
	return 0;
}

void CTS_IN_Buffer::open(void *_path_file)
{

	mpeg2ts_statistic.packet_size = sys_play->plld->g_CurrentPacketSize;
	mpeg2ts_statistic.bitrate = sys_play->media_interface[0]->option.bitrate;

	//cir_input_ts_buffer = new CCircularBuffer(1, (int)(mpeg2ts_statistic.bitrate*0.5)/8);
	cir_input_ts_buffer = new CCircularBuffer(1, (int)((mpeg2ts_statistic.bitrate)/8) * 4);
	is_start = 1;

	tlv_message_t message;
	message.id = PLAYBACK_USER_STATE_START_STREAMING;
	message.param = NULL;
	tlv_mq_send(&sys_play->thread_observer, &message);

	//printf("==================input-buffer plug-in, =============\n");
	//printf("bitrate : %f \n", mpeg2ts_statistic.get_bitrate());
	//printf("=====================================================\n");


}
void CTS_IN_Buffer::close(void *arg)
{
	delete cir_input_ts_buffer;
}

// not yet support 
int CTS_IN_Buffer::read_n_byte(char *des, int num_byte)
{

	return 0; 

}

int CTS_IN_Buffer::read_one_packet(char *buff)
{

//	int cnt = 0;
_WAITING_:
	
	if (sys_play->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(mutex_cir_data);
		int num_bytes_buffered = cir_input_ts_buffer->ItemSize()*cir_input_ts_buffer->NumBufferedItem();
		if (num_bytes_buffered < mpeg2ts_statistic.get_packet_size())
		{
//			cnt++;
//			if((cnt % 2) == 0) 
//				cir_input_ts_buffer->DebugOutputMsg("CTS_IN_Buffer::read_one_packet");
			// signal
			tlv_cond_signal(&cv_cir_in);
			// and waiting 
			tlv_cond_wait(&cv_cir_out, mutex_cir_data);
			tlv_mutex_unlock(mutex_cir_data);
			goto _WAITING_;	
		}
		int rc = cir_input_ts_buffer->DebufferNItem_sync(buff, mpeg2ts_statistic.get_packet_size());
		tlv_mutex_unlock(mutex_cir_data);
		if(rc == 0)
			goto _WAITING_;
		if(rc == -1)
			goto _WAITING_;
			
		return rc;
	}
}

void CTS_IN_Buffer::force_close()
{

}
void CTS_IN_Buffer::call_SetHwDmaDiection_LoopThru()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	rdwr->SetHwDmaDiection_LoopThru();
}
void CTS_IN_Buffer::call_SetHwFifoCntl_()
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	rdwr->SetHwFifoCntl_(1, rdwr->SdramSubBankSize());
}
int	CTS_IN_Buffer::call_IsFpgaCapBuf_Underrun_toMinLevel(void)
{
	CHld *lld = sys_play->plld;
	CHldFsRdWr *rdwr = lld->_FIf;
	rdwr->RdFpgaCapPlay_BufLevel(200);
	return rdwr->IsFpgaCapBuf_Underrun_toMinLevel();
}
//=================================================================================================================================================================================


