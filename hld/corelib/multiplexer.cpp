#include "corelib.h"
#include "hldplay.h"
#include "multiplexer.h"
#include "hld_fs_rdwr.h"
#include "LLDWrapper.h"
// do we need to include header file here ???
// it is nonsense. Whenever new multiplexer is added, we need to add the corresponding header file, don't we?????
//
#include "dvbt2_multiplexer.h"
#include "dvbc2_multiplexer.h"
#include "isdbt_multiplexer.h"



int Multiplexer::read_n_byte(char *buff, int num_byte)
{
_WAITING_:
	if (play_sys->playback_control == PLAYBACK_CONTROL_END_OUTPUT)
			return INPUT_ERROR_EOF;
	else
	{
		tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
		int num_bytes_buffered = play_sys->cir_multiplexed_data_buffer->ItemSize()*play_sys->cir_multiplexed_data_buffer->NumBufferedItem();
		if (num_bytes_buffered < num_byte)
		{ // not enough data for reading 
//			if((cnt % 2) == 0)
//				play_sys->cir_multiplexed_data_buffer->DebugOutputMsg("Multiplexer::read_n_byte");
			//printf("==== output waiting [need data] %s,  %s, %d \n", __FILE__, __FUNCTION__, __LINE__);
			// signal for reading data 
			tlv_cond_signal(&play_sys->cv_multiplexed_data_in);
			
			// and waiting 
			tlv_cond_wait(&play_sys->cv_multiplexed_data_out, &play_sys->mutex_cir_multiplexed_data);
			
			tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
			goto _WAITING_;	
		}
		int rc = play_sys->cir_multiplexed_data_buffer->DebufferNItem(buff, num_byte);
		//play_sys->cir_multiplexed_data_buffer->DebugOutputMsg("read_n_byte");
		tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
		return rc;
	}
}

int Multiplexer::main_loop(void *param)
{
	int rc;
	int i;

	play_sys = (CHldPlayback *)param;

	init();

_BUFFERRING_:

		tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
		if (play_sys->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
		{
			tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
			goto _END_1_;
		}		
		tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);

		for ( i = 0; i < play_sys->num_media_interface; i++)
		{
			tlv_mutex_lock(&play_sys->thread_media_receiving_data[i].mutex_thread_state);
			if (play_sys->thread_media_receiving_data[i].thread_state == THREAD_STATE_READING_INPUT)
			{
				tlv_mutex_unlock(&play_sys->thread_media_receiving_data[i].mutex_thread_state);
				goto _BUFFERRING_;
			}
			tlv_mutex_unlock(&play_sys->thread_media_receiving_data[i].mutex_thread_state);
		}

		tlv_mutex_lock(&play_sys->thread_producer.mutex_thread_state);
		play_sys->thread_producer.thread_state = THREAD_STATE_READY;
		tlv_mutex_unlock(&play_sys->thread_producer.mutex_thread_state);


	rc = open(play_sys);
	if (rc != 0)
	{
//		printf("assert: canot open  function: %s, line: %d \n", __FUNCTION__, __LINE__);
		goto _END_1_;
	}

	do
	{
		static int end_streaming_is_sent = 0;	

_WAITING_:
		if (play_sys->playback_control != PLAYBACK_CONTROL_END_MULTIPLEXER)
		{
			tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);			
			int num_unbuffered_bytes = play_sys->cir_multiplexed_data_buffer->NumUnbufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
			if(num_unbuffered_bytes < MULTIPLEXER_BUFF_ITEM_SIZE && play_sys->playback_control != PLAYBACK_CONTROL_END_MULTIPLEXER)
			{
				//OutputDebugString("===== Waiting============\n");
				//printf("====  multiplexing waiting [full]%s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
				tlv_cond_wait(&play_sys->cv_multiplexed_data_in, &play_sys->mutex_cir_multiplexed_data);
				
				tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
				//Sleep(10);
				goto _WAITING_;
			}
			tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);

		}

		//printf("====  multiplexing releasing %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);

		if (play_sys->playback_control == PLAYBACK_CONTROL_END_MULTIPLEXER)
				goto _END_;

		// Make a multiplexed data,
		rc = play_sys->multiplex_interface->produce_item(NULL, MULTIPLEXER_BUFF_ITEM_SIZE);
		//printf("Debug %s, %d \n", __FILE__, __LINE__);
		if (rc < 0)
			goto _END_;
			/*
		tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);			
		int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
		//int u_bytes = play_sys->cir_multiplexed_data_buffer->NumUnbufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
		tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
		//printf("multilexer buffer status buffered %d, unbuffered %d ,   %f \n", b_bytes, u_bytes, b_bytes*1.0/(b_bytes + u_bytes));
		*/
		Sleep(20);
	} while (1);

_END_:
	close();

_END_1_:
	quit();		


	tlv_mutex_lock(&play_sys->thread_producer.mutex_thread_state);
	play_sys->thread_producer.thread_state = THREAD_STATE_FINISH;
	tlv_mutex_unlock(&play_sys->thread_producer.mutex_thread_state);


	//printf("[HLD] multiplexer Thread ending .....\n");

	return 0;
}

