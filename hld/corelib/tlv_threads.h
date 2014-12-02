//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

#ifndef _TLV_THREAD_
#define _TLV_THREAD_

// mutex api 
extern "C"  void  tlv_mutex_init  ( tlv_mutex_t * );
extern "C"  void  tlv_mutex_destroy  ( tlv_mutex_t * );
extern "C"  void  tlv_mutex_lock  ( tlv_mutex_t * );
extern "C"  void  tlv_mutex_unlock  ( tlv_mutex_t * );


// semaphore api 
extern "C" void tlv_sem_init(tlv_sem_t *_sem, unsigned int _value);
extern "C" void tlv_sem_destroy(tlv_sem_t *_sem);
extern "C" void tlv_sem_post(tlv_sem_t *_sem);
extern "C" void tlv_sem_wait(tlv_sem_t *_sem);


// thread api 
extern "C" void tlv_thread_create(tlv_thread_t *_thread);

// message queue
extern "C" void tlv_mq_init(tlv_thread_t *_thread);
extern "C" void tlv_mq_send(tlv_thread_t * _thread, tlv_message_t *message);
// return 0: receive an message successfully 
//        -1: internal error 
extern "C" int tlv_mq_receive(tlv_thread_t * _thread, tlv_message_t *message);
extern "C" void tlv_mq_destroy(tlv_thread_t *_thread);


// condition variable 
extern "C" int tlv_cond_init(tlv_cond_t *cv, tlv_condattr_t *attr);
extern "C" int tlv_cond_destroy(tlv_cond_t *cv);
extern "C" int tlv_cond_wait(tlv_cond_t *cv, tlv_mutex_t *mutex);
extern "C" int tlv_cond_signal(tlv_cond_t *cond);


// timmer
extern "C" int tlv_gettime_ms();

#endif
