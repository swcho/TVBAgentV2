//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#include <stdint.h> 
#include <stdio.h>
#include <sys/time.h>

#include "tlv_type.h"
#include "tlv_threads.h"

///////////////////////////////////////////////////
// mutex

void  tlv_mutex_init  ( tlv_mutex_t *mutex )
{
	int rc;
	pthread_mutexattr_t attr;
	rc= pthread_mutexattr_init(&attr);
	if (rc != 0)
	{
//		printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	if (rc != 0)
	{
//		printf("assert: %s, %d \n", __FILE__, __LINE__);
	}
	rc = pthread_mutex_init(mutex, &attr);
	if (rc != 0)
	{
//		printf("assert: %s, %d \n", __FILE__, __LINE__);
	}
	rc = pthread_mutexattr_destroy(&attr);
	if (rc != 0)
	{
//		printf("assert %s, %d \n", __FILE__, __LINE__);
	}
}

void  tlv_mutex_destroy  ( tlv_mutex_t *mutex )
{
	if(mutex != NULL)
	{
		pthread_mutex_destroy(mutex); 
		mutex = NULL;
	}
}

void  tlv_mutex_lock  ( tlv_mutex_t *mutex )
{
	int rc  = pthread_mutex_lock(mutex);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}

}


void  tlv_mutex_unlock  ( tlv_mutex_t *mutex )
{

	int rc  = pthread_mutex_unlock(mutex);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
}

///////////////////////////////////////////////////
// semaphore 

void tlv_sem_init(tlv_sem_t *_sem, unsigned int _value)
{
	int rc = sem_init(_sem, 0, _value);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
}

}
void tlv_sem_destroy(tlv_sem_t *_sem)
{
	int rc = sem_destroy(_sem);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
}
void tlv_sem_post(tlv_sem_t *_sem)
{
	int rc = sem_post(_sem);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
}
void tlv_sem_wait(tlv_sem_t *_sem)
{
	int rc = sem_wait(_sem);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
}


///////////////////////////////////////////////////
// thread api



void tlv_thread_create(tlv_thread_t *_thread)
{
	_thread->thread_state = THREAD_STATE_READING_INPUT;
	tlv_mutex_init(&_thread->mutex_thread_state);
	int rcd = pthread_create(&_thread->thread, NULL, _thread->thread_entry, _thread->param);
	if (rcd != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	

	// note: we have not destroy this object 

	
}


///////////////////////////////////////////////////
// message queue 

void tlv_mq_init(tlv_thread_t *_thread)
{
	// size of each message is 10 bytes ( 1 -- 2 byte: ID (short data type), 7 byte: blabla) 
	// and 1000 message
	// 
	_thread->message_queue = new CCircularBuffer(20, 10000);
	// initialize mutex 
	tlv_mutex_init(&_thread->message_mutex);
	// initialize condition variable 
	tlv_cond_init(&_thread->message_cond, NULL);
}

void tlv_mq_destroy(tlv_thread_t *_thread)
{
	tlv_mutex_destroy(&_thread->message_mutex);	
	tlv_cond_destroy(&_thread->message_cond);
	delete _thread->message_queue;
}

void tlv_mq_send(tlv_thread_t * _thread, tlv_message_t *message)
{
	unsigned char buff[100];
	if (_thread->message_queue->IsFull())
	{
		//printf("assert: message queue is full %s, %d \n", __FILE__, __LINE__);
	}

	// message id
	unsigned char int_h2 = (message->id & 0xFF000000) >> 24;
	unsigned char int_h1 = (message->id & 0x00FF0000) >> 16;
	unsigned char int_l1 = (message->id & 0x0000FF00) >> 8;
	unsigned char int_l2 = (message->id & 0x000000FF);
	buff[0] =  int_h2;
	buff[1] =  int_h1;
	buff[2] =  int_l1;
	buff[3] =  int_l2;

		// data 
	// bitrate 
	int_h2 = (message->bitrate & 0xFF000000) >> 24;
	int_h1 = (message->bitrate & 0x00FF0000) >> 16;
	int_l1 = (message->bitrate & 0x0000FF00) >> 8;
	int_l2 = (message->bitrate & 0x000000FF);
	buff[4] =  int_h2;
	buff[5] =  int_h1;
	buff[6] =  int_l1;
	buff[7] =  int_l2;

	// data size 
	int_h2 = (message->data_size_in_byte & 0xFF000000) >> 24;
	int_h1 = (message->data_size_in_byte & 0x00FF0000) >> 16;
	int_l1 = (message->data_size_in_byte & 0x0000FF00) >> 8;
	int_l2 = (message->data_size_in_byte & 0x000000FF);
	buff[8] =  int_h2;
	buff[9] =  int_h1;
	buff[10] =  int_l1;
	buff[11] =  int_l2;
		
	tlv_mutex_lock(&_thread->message_mutex);

	tlv_cond_signal(&_thread->message_cond);
	_thread->message_queue->BufferItem((char *)&buff[0]);

	tlv_mutex_unlock(&_thread->message_mutex);

//	_thread->message_queu 
}
int tlv_mq_receive(tlv_thread_t * _thread, tlv_message_t *message)
{
//	printf("message queue: receving \n");

	char buff[100];
	tlv_mutex_lock(&_thread->message_mutex);
	if (_thread->message_queue->IsEmpty())
	{
		tlv_cond_wait(&_thread->message_cond, &_thread->message_mutex);
	}
	int rc = _thread->message_queue->DebufferItem(&buff[0]);
	if(rc < 0)
	{
		return -1;
	}
	tlv_mutex_unlock(&_thread->message_mutex);

	unsigned int int_h2 = buff[0]; 
	unsigned int int_h1 = buff[1]; 
	unsigned int int_l1 = buff[2];
	unsigned int int_l2 = buff[3];
	message->id = ((int_h2 << 24) | (int_h1 << 16) | (int_l1 << 8) | (int_l2));

	int_h2 = buff[4]; 
	int_h1 = buff[5]; 
	int_l1 = buff[6];
	int_l2 = buff[7];
	message->bitrate = ((int_h2 << 24) | (int_h1 << 16) | (int_l1 << 8) | (int_l2));

	int_h2 = buff[8]; 
	int_h1 = buff[9]; 
	int_l1 = buff[10];
	int_l2 = buff[11];
	message->data_size_in_byte = ((int_h2 << 24) | (int_h1 << 16) | (int_l1 << 8) | (int_l2));
	return 0;

}

///////////////////////////////////////////////////
// condition variable 

// huy: note attr_null is not used at all 
// it is just use to conform with pthread specification 
int tlv_cond_init(tlv_cond_t *_cond, tlv_condattr_t *attr_null)
{
	pthread_condattr_t attr; 
	int rc = pthread_condattr_init(&attr);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	rc = pthread_cond_init(_cond, &attr);
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	rc = pthread_condattr_destroy(&attr); 
	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	return 0;
}

int tlv_cond_destroy(tlv_cond_t *_cond)
{
	int rc;
	rc = pthread_cond_destroy(_cond);

	if (rc != 0)
	{
		//printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	return 0;
}

int tlv_cond_wait(tlv_cond_t *_cond, tlv_mutex_t *_mutex)
{
	int rc; 
	rc = pthread_cond_wait(_cond, _mutex);
	if (rc != 0)
	{
		printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	return 0;
}
int tlv_cond_signal(tlv_cond_t *_cond)
{
	int rc;
	rc = pthread_cond_signal(_cond);
	if (rc != 0)
	{
		printf("assert %s, %d \n", __FILE__, __LINE__);
	}
	return 0;
}

// timer
extern "C" int tlv_gettime_ms()
{
	struct timeval time;
	gettimeofday(&time, NULL);

	return int(time.tv_sec*1000 + time.tv_usec/1000);
}


