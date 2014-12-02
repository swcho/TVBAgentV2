// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _DVBT2_Multiplexer_
#define _DVBT2_Multiplexer_

#include "dvbt2_multipleplp.h"
#include "play.h"
#include "corelib.h"
#include "multiplexer.h"
#include "ts_mux.h"

#define DEMUX_MPEG2TS_MAX_PID 0x1FFF
#ifdef WIN32
#else
typedef long long __int64;
#endif

typedef struct pcr_info{
	int nPid;
	unsigned int uPcrTotalPacket;
	unsigned int uPcrOriginalTsPacket;
	__int64 pcrCompensation;
} pcr_info_t;


typedef struct ts_remux{
	double accumulator_null;
	double accumulator_file;
	pcr_info_t pcrRestamp[DEMUX_MPEG2TS_MAX_PID];

}ts_remux_t;

// Abstraction of input plug-in
class CHldPlayback;

class DVBT2Multiplexer : public Multiplexer
{
public:

	t2mi_handle_ptr multiple_plp_handle; 
	DVBT2_PARAM_t dvbt2_param;
	// for TS remux 
	ts_remux_t remuxs[10];
	unsigned char stuff_packet[204];

private:
	ts_mux_handle_t ts_mux; 
	


	char *t2mi_buff_tmp;
	char *t2mi_ts_buff;

public:
   DVBT2Multiplexer();
   ~DVBT2Multiplexer();
	//void init();
	void init();
	void quit();
	int open(CHldPlayback *_play_sys);
	int produce_item(char *buff, int size);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	void close();

	int GetT2MIPacket(char **buff, int num_bytes);

	double get_bitrate();

	void _pcr_correction(pcr_info_t *pcrInfo, unsigned char* packet, int out_bitrate, int in_bitrate);	
};


// Read from file directly 
class CTLVMedia; 
class DVBT2Reader : public Multiplexer{

	
private:
	//TS_Reader ts_reader;
	CTLVMedia *media_source;
	char *ts_buff;
public:
	DVBT2Reader();
	~DVBT2Reader();
	void init();
	void quit();
	int open(CHldPlayback *_play_sys);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	int produce_item(char *buff, int size);
	void close();

};


#endif
