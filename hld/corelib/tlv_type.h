//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#ifndef _TLV_TYPE_
#define _TLV_TYPE_

#ifdef WIN32
#include <windows.h>
#else // LINUX
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#endif
#include "corelib.h"

#define THREAD_STATE_READING_INPUT 0
#define THREAD_STATE_READY 1
#define THREAD_STATE_FINISH 2



#ifdef WIN32
#define TLV_ThreadEntry(name, args ) \
                        HANDLE name args

#define TLV_PThreadEntry(name, args ) \
                        HANDLE (*name) args

#else

#define TLV_ThreadEntry(name, args ) \
                        void* name args

#define TLV_PThreadEntry(name, args ) \
                        void* (*name) args

#endif


// 
#ifdef WIN32

// mutex 
typedef HANDLE tlv_mutex_t;
//typedef CRITICAL_SECTION tlv_mutex_t; 
// semaphore 
typedef HANDLE tlv_sem_t;

// cond variable 
typedef struct
{
	// Count of the number of waiters.
	u_int waiters_count;
	//CRITICAL_SECTION waiters_count_lock;
	tlv_mutex_t waiters_count_lock;

	enum {
		SIGNAL = 0,
		BROADCAST = 1,
		MAX_EVENTS = 2
	};
  // Signal and broadcast event HANDLEs.
  HANDLE events_[MAX_EVENTS];

} tlv_cond_t;

typedef void tlv_condattr_t;



typedef struct {
	unsigned int id; // 4 bytes

	// temporal solution for data exchange among threads 
	// try to employ technique for exchanging data on Linux an window flatform.... 
	int bitrate;   // 4 bytes
	int data_size_in_byte; 	// 4 bytes 

	WPARAM param; // data for exchange b/t threads 

} tlv_message_t;


typedef struct {
	DWORD id;
	HANDLE handle;
	void *param;

	tlv_mutex_t mutex_thread_state;
	int thread_state; 

	TLV_PThreadEntry(thread_entry, (void *args));

	// for create a message queue associated with thread 
	CCircularBuffer * message_queue;
	tlv_mutex_t message_queue_mutex;
	HANDLE message_queue_event; 


}tlv_thread_t;

#else // UNIX 
// mutex 
typedef pthread_mutex_t tlv_mutex_t;
// semaphore 
typedef sem_t tlv_sem_t;

// condition variable
typedef pthread_cond_t tlv_cond_t; 
typedef pthread_condattr_t tlv_condattr_t;

typedef struct {
	unsigned int id;
	
	// temporal solution for data exchange among threads 
	// try to employ technique for exchanging data on Linux an window flatform.... 
	int bitrate;   // 4 bytes
	int data_size_in_byte; 	// 4 bytes 

	void* param; // data for exchange b/t threads
} tlv_message_t;

typedef struct {
	pthread_t thread;
	void *param;
	TLV_PThreadEntry(thread_entry, (void *args));

	tlv_mutex_t mutex_thread_state;
	int thread_state; 
	
	// object 
	CCircularBuffer * message_queue;
	tlv_mutex_t message_mutex;
	tlv_cond_t message_cond; 

}tlv_thread_t;

#endif


// socket's definition 
#ifdef WIN32
typedef SOCKET tlv_socket_t ;
#else
typedef int tlv_socket_t;
#endif



// define some OS-independent function
#ifdef WIN32
#define tlv_sleep(millisecond) Sleep(millisecond)
#else
#define tlv_sleep(millisecond)  usleep(millisecond*1000)
#endif






#endif
