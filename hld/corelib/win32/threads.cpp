//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#include <Windows.h> 

#include "tlv_type.h"
#include "tlv_threads.h"

///////////////////////////////////////////////////
// mutex

void  tlv_mutex_init  ( tlv_mutex_t *mutex )
{

	*mutex =  CreateMutex(
		NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);
	//ssert(*mutex != NULL);
	
}

void  tlv_mutex_destroy  ( tlv_mutex_t *mutex )
{
	if(*mutex != NULL)
	{
		CloseHandle(*mutex);
		*mutex = NULL;
	}
}

void  tlv_mutex_lock  ( tlv_mutex_t *mutex )
{
	WaitForSingleObject(*mutex, INFINITE);
	//EnterCriticalSection(mutex);
}


void  tlv_mutex_unlock  ( tlv_mutex_t *mutex )
{
	ReleaseMutex(*mutex);
	//LeaveCriticalSection(mutex);
}

///////////////////////////////////////////////////
// semaphore 

void tlv_sem_init(tlv_sem_t *_sem, unsigned int _value)
{
		*_sem = CreateSemaphore(NULL,           // default security attributes
        _value,  // initial count
        _value ,  // maximum count
        NULL);          // unnamed semaphore
	//ssert(*_sem != NULL);

}


void tlv_sem_destroy(tlv_sem_t *_sem)
{
	CloseHandle(*_sem);
}


void tlv_sem_post(tlv_sem_t *_sem)
{
	ReleaseSemaphore(*_sem,  // handle to semaphore
                        1,            // increase count by one
                        NULL);     

}
void tlv_sem_wait(tlv_sem_t *_sem)
{
	WaitForSingleObject(*_sem, INFINITE);
}

///////////////////////////////////////////////////
// thread api


void tlv_thread_create(tlv_thread_t *_thread)
{
	_thread->thread_state = THREAD_STATE_READING_INPUT;
	tlv_mutex_init(&_thread->mutex_thread_state);
	_thread->handle = CreateThread( NULL,       // default security attributes
                     0,          // default stack size
                     (LPTHREAD_START_ROUTINE) _thread->thread_entry, 
                     _thread->param,       // 
                     0,          // default creation flags
                     &_thread->id); // receive thread identifier
	//ssert(_thread->handle != NULL);


	// note: we have not destroy this object 

}

///////////////////////////////////////////////////
// message queue 

void tlv_mq_init(tlv_thread_t *_thread)
{
	// for message queue
	// maximun: 1000 message in queue 
	_thread->message_queue = new CCircularBuffer(20, 10000);
	//ssert(_thread->message_queue != NULL);

	_thread->message_queue_event = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset
        FALSE,              // initial state is nonsignaled
        NULL  // object name
        ); 

	tlv_mutex_init(&_thread->message_queue_mutex);
}

void tlv_mq_destroy(tlv_thread_t *_thread)
{
	tlv_mutex_destroy(&_thread->message_queue_mutex);	
	if(_thread->message_queue_event != NULL)
		CloseHandle(_thread->message_queue_event);
	delete _thread->message_queue;
}

void tlv_mq_send(tlv_thread_t * _thread, tlv_message_t *message)
{
	unsigned char buff[100];
	if (_thread->message_queue->IsFull())
	{
		printf("message queue is full %s, %d \n", __FILE__, __LINE__);
		//asert(0);
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

	

	tlv_mutex_lock(&_thread->message_queue_mutex);
	_thread->message_queue->BufferItem((char *)&buff[0]);	
	tlv_mutex_unlock(&_thread->message_queue_mutex);

	// set event
	int rc = SetEvent(_thread->message_queue_event);
	//ssert(rc != 0);

}

int tlv_mq_receive(tlv_thread_t * _thread, tlv_message_t *message)
{
	unsigned char buff[100];
	int is_empty; 

	tlv_mutex_lock(&_thread->message_queue_mutex);
	is_empty = _thread->message_queue->IsEmpty();
	tlv_mutex_unlock(&_thread->message_queue_mutex);

	if (is_empty)
	{
		//printf("Debug waiting  %s, %d \n", __FILE__, __LINE__);
		ResetEvent(_thread->message_queue_event);
		WaitForSingleObject(_thread->message_queue_event, INFINITE);
	}


	tlv_mutex_lock(&_thread->message_queue_mutex);
	int rc = _thread->message_queue->DebufferItem((char *)&buff[0]);
	tlv_mutex_unlock(&_thread->message_queue_mutex);
	if (rc < 0)
	{
		printf("[internal] error %s, %d \n", __FILE__, __LINE__);
		return -1;
	}


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


int tlv_cond_init(tlv_cond_t *cv, tlv_condattr_t *attr)
{
	cv->waiters_count = 0;
	tlv_mutex_init(&cv->waiters_count_lock);


// Create an auto-reset event.
	tlv_cond_t tmp;
	cv->events_[tmp.SIGNAL] = CreateEvent (NULL,  // no security
                                     FALSE, // auto-reset event
                                     FALSE, // non-signaled initially
                                     NULL); // unnamed

  // Create a manual-reset event.
  cv->events_[tmp.BROADCAST] = CreateEvent (NULL,  // no security
                                        TRUE,  // manual-reset
                                        FALSE, // non-signaled initially
                                        NULL); // unnamed

  return 0;
}
int tlv_cond_destroy(tlv_cond_t *cv)
{
	tlv_mutex_destroy(&cv->waiters_count_lock);
	return 0;
}
int tlv_cond_wait(tlv_cond_t *cv, tlv_mutex_t *external_mutex)
{
 // Avoid race conditions.
  tlv_mutex_lock (&cv->waiters_count_lock);
  cv->waiters_count++;
  tlv_mutex_unlock(&cv->waiters_count_lock);


  // It's ok to release the <external_mutex> here since Win32
  // manual-reset events maintain state when used with
  // <SetEvent>.  This avoids the "lost wakeup" bug...
  //LeaveCriticalSection((LPCRITICAL_SECTION)external_mutex);
  tlv_mutex_unlock(external_mutex);

  // Wait for either event to become signaled due to <pthread_cond_signal>
  // being called or <pthread_cond_broadcast> being called.
  int result = WaitForMultipleObjects (2, cv->events_, FALSE, INFINITE);

  tlv_cond_t tmp; 

  tlv_mutex_lock (&cv->waiters_count_lock);
  cv->waiters_count--;
  int last_waiter =
    result == WAIT_OBJECT_0 + tmp.BROADCAST 
    && cv->waiters_count == 0;
  tlv_mutex_unlock (&cv->waiters_count_lock);

  // Some thread called <pthread_cond_broadcast>.
  if (last_waiter)
    // We're the last waiter to be notified or to stop waiting, so
    // reset the manual event. 
    ResetEvent (cv->events_[tmp.BROADCAST]); 

  // Reacquire the <external_mutex>.
  //EnterCriticalSection ((LPCRITICAL_SECTION)external_mutex);
  tlv_mutex_lock(external_mutex);

	return 0;
}
	

int tlv_cond_signal(tlv_cond_t *cv)
{
	 tlv_mutex_lock (&cv->waiters_count_lock);
	 int have_waiters = cv->waiters_count > 0;
	 tlv_mutex_unlock (&cv->waiters_count_lock);
	 if (have_waiters)
		 SetEvent (cv->events_[cv->SIGNAL]);

	return 0;
}

// timer
int tlv_gettime_ms()
{
	SYSTEMTIME system_time;
	GetLocalTime(&system_time);

	// for evaluation performance of making t2mi
	long hour, minute, seconds, milisecond, useconds;

	hour = system_time.wHour*60*60*1000;
	minute = system_time.wMinute*60*1000;
	seconds = system_time.wSecond*1000;
	milisecond = system_time.wMilliseconds;


	return (int)(hour + minute + seconds + milisecond);
}



