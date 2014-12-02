//=================================================================	
//
//	Copyright (C) 2011
//	Teleview Corporation
//
//	Author : 
//  	HuyLe, huy@teleview.com
//=================================================================	

// history:
// 20110414: create for Window version 

#ifndef	_TLV_PLAYBACK_
#define	_TLV_PLAYBACK_


#include <stdio.h>

#include "tlv_type.h"
#include "corelib.h"
#include "media_interface.h"
#include "data_reader_from_ip.h"

class CHld;


//////////////////////////////// define some utilities ////////////////////////////////////////////////

// state of play 
#define PLAYBACK_STATE_NONE  1000 
#define PLAYBACK_STATE_INIT 0
#define PLAYBACK_STATE_PLAY 1
#define PLAYBACK_STATE_PAUSE 2


// for controling ending..... , 
// this declaration is used as communication b/t hldplayer & input & mul & output
// once communication method is developed, those will be obsoleted 
#define PLAYBACK_CONTROL_NONE 0
#define PLAYBACK_CONTROL_END_INPUT 1
#define PLAYBACK_CONTROL_END_MULTIPLEXER 2
#define PLAYBACK_CONTROL_END_OUTPUT 2
#define PLAYBACK_CONTROL_PAUSE 3



// report some states to users
// 
#define PLAYBACK_USER_STATE_EXIT 0
#define PLAYBACK_USER_STATE_START_STREAMING 2
#define PLAYBACK_USER_STATE_STOP_STREAMING 3

// for print out log 
#define PLAYBACK_LOG_MESSAGE 100

// multiplexer types
#define PLAY_MULTIPLEXER_ISDBT_MUX  0
#define PLAY_MULTIPLEXER_ISDBT_NO_MUX  1
#define PLAY_DVBT2_MUX 3
#define PLAY_DVBT2_MUXED_FILE 4 
//2011/6/28 DVB-C2 MULTI PLP 
#define PLAY_DVBC2_MUX 5
#define PLAY_DVBC2_MUXED_FILE 6

// Media source 
#define PLAY_MEDIA_SOURCE_IP_BUFFER 1 
#define PLAY_MEDIA_SOURCE_FILE 2 
#define PLAY_MEDIA_SOURCE_FAKE 3 
#define PLAY_MEDIA_SOURCE_IP_RTP 4 
//2012/7/6 DVB-T2 ASI
#define PLAY_MEDIA_SOURCE_ASI_310M 5 

// output  
#define PLAY_OUTPUT_TVBBOARD 1 
#define PLAY_OUTPUT_FILE 2 
#define PLAY_OUTPUT_NULL 3 

// play mode 
#define PLAY_MODE_REPEAT 1 // 0001: repeat current item
#define PLAY_MODE_LOOP  2 // 0010 : loop current playlist

// message is sent to play 
#define PLAY_MESSAGE_SEEK 1
#define PLAY_MESSAGE_PLAY  2
#define PLAY_MESSAGE_STOP  3
#define PLAY_MESSAGE_REWIND  4
#define PLAY_MESSAGE_INIT  5
#define PLAY_MESSAGE_QUIT  6
#define PLAY_MESSAGE_GET_TS_BITRATE  7
#define PLAY_MESSAGE_PAUSE  8
#define PLAY_MESSAGE_RESUME 9



// packet type in multiplexer buffer 
#define  PLAY_ITEM_STATE_NORMAL 0 
#define  PLAY_ITEM_STATE_EOF 1


// IO control 
#define PLAY_IO_CONTROL_SEEK 1
// ISDB-T 
#define ISDBT_IO 1000
#define ISDBT_IO_SET_LAYER_INFOR ISDBT_IO + 1

// number of byte for producing or consuming
#define MULTIPLEXER_BUFF_SIZE 8*1024*1024
#define MULTIPLEXER_BUFF_ITEM_SIZE 1024*1024
#define MULTIPLEXER_BUFF_ITEM_NUM 4



class CHldPlayConcreteObserver: public CHldPlayObserver{

public:
	CHld *pLLD;
	int Update(int state);
};



class ConcreteObserverTest: public CHldPlayObserver{

public:
	int Update(int state);
};


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CHldPlayback
{

public:

	int play_mode; 
	int num_media_interface;
	char file_path_for_writing[256];

	// for interfacing with HLD	
	CHld *plld;
	
	CTLVMedia *media_interface[10]; 

	Multiplexer *multiplex_interface;

	Output *output_interface;


	CHldPlayObserver *observer;

public:

//	CHld *plld;

	int playback_state;
	int playback_control;



	tlv_thread_t thread_producer;
	tlv_thread_t thread_consumer;
	tlv_thread_t thread_observer; 
	tlv_thread_t thread_media_receiving_data[10];

	
	CCircularBuffer  *cir_multiplexed_data_buffer;
	tlv_mutex_t mutex_cir_multiplexed_data;	
	tlv_cond_t cv_multiplexed_data_in;
	tlv_cond_t cv_multiplexed_data_out;

	CCircularBuffer *cir_input_ts_buffer[10]; 
	tlv_mutex_t mutex_cir_input_ts[10];


	// solution for producer-consumer problem 
	tlv_mutex_t playback_end_mutex;	
	
public:
	CHldPlayback(void);
	~CHldPlayback();


	void ObserverThread(void *arg);

	int init(CHld *_plld, int media_source, int multipelxing_type, int output_type);
	int quit();
	int stop();
	int play();
	int pause();
	int resume();
	void set_observer(CHldPlayObserver *_observer);
	
};

extern "C" {
	void playback_receive_request(CHld* plld1, int message_type, void *param);
	void playback_cmd_interface(int message_type, void *param);
}


#endif

