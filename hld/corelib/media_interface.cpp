#include <stdio.h>
#include <stdlib.h>

#include "corelib.h"
#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "media_interface.h"
#include "log.h"


void CTLVMedia::init(CHldPlayback *_sys_play){
	
	//sys_play = _sys_play;	
	
	mpeg2ts_statistic.init();
	mpeg2ts_statistic.set_selected_pids(&option.filter_pids[0], option.num_filter_pids);		
	
}

void CTLVMedia::quit()
{
	mpeg2ts_statistic.quit();
	
}

int CTLVMedia::media_receving_data_loop(void *param)
{
	int num_buffered_bytes;
	int rc;


	tlv_cond_init(&cv_cir_in, NULL);
	tlv_cond_init(&cv_cir_out, NULL);

	init(NULL);
	open(option.networking_or_file); 
	int packet_size = mpeg2ts_statistic.get_packet_size();
	
	
	tlv_mutex_lock(&thread_handle->mutex_thread_state);
	if (thread_handle->thread_state == THREAD_STATE_READING_INPUT)
	{
			thread_handle->thread_state = THREAD_STATE_READY;
	}
	tlv_mutex_unlock(&thread_handle->mutex_thread_state);


	while(1)
	{
_WAITING_:
		int num_unbuffered_bytes = 0;
		if (sys_play->playback_control != PLAYBACK_CONTROL_END_INPUT)
		{

			// wait because of data buffer is full
			tlv_mutex_lock(mutex_cir_data);	
			num_unbuffered_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
			if(num_unbuffered_bytes < packet_size && sys_play->playback_control != PLAYBACK_CONTROL_END_INPUT)
			{
				// signal 
				tlv_cond_signal(&cv_cir_out);				
				//printf("====  input waiting [full] %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
				
				// and waiting 
				tlv_cond_wait(&cv_cir_in, mutex_cir_data);
				
				tlv_mutex_unlock(mutex_cir_data);
				goto _WAITING_;
			}
			tlv_mutex_unlock(mutex_cir_data);	


			// wait because of system is in PAUSE state
			if(sys_play->playback_control == PLAYBACK_CONTROL_PAUSE)
			{
				// It is natural but not robust !!!!
				tlv_sleep(100);
				goto _WAITING_;
			}
		}

//		printf("====  input releasing  %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__);
		if (sys_play->playback_control == PLAYBACK_CONTROL_END_INPUT)
				goto _END_;
		
		read_data(NULL, num_unbuffered_bytes); 

/*
		tlv_mutex_lock(mutex_cir_data);			
		int u_bytes = cir_input_ts_buffer->NumUnbufferedItem()*cir_input_ts_buffer->ItemSize();
		int b_bytes = cir_input_ts_buffer->NumBufferedItem()*cir_input_ts_buffer->ItemSize();
		tlv_mutex_unlock(mutex_cir_data);
		printf("input buffered %d, unbuffered %d ,   %f \n", b_bytes, u_bytes, b_bytes*1.0/(b_bytes + u_bytes));
*/		

	}

_END_:
	
	close(NULL);
	quit();

	tlv_cond_destroy(&cv_cir_in);
	tlv_cond_destroy(&cv_cir_out);
	//printf(" Media is ending \n");

	tlv_mutex_lock(&thread_handle->mutex_thread_state);

	thread_handle->thread_state = THREAD_STATE_FINISH;
	tlv_mutex_unlock(&thread_handle->mutex_thread_state);
	return 0;

}


int CTLVMedia::ioctl(int count, ... )
{

	return 0;
}
