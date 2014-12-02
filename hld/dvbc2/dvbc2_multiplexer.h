// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _DVBC2_Multiplexer_
#define _DVBC2_Multiplexer_

#include "dvbc2_multipleplp.h"
#include "play.h"
#include "corelib.h"
#include "multiplexer.h"

#include "dvbc2_input_processing.h"

// Abstraction of input plug-in
class CHldPlayback;


class DVBC2Multiplexer : public Multiplexer, public C2MI_TS_Writer_Interface
{
public:
	CDVBC2_MultiplePLP multiple_plp; 
	DVBC2_PARAM dvbc2_param;
private:
	C2MI_TS_Writer ts_maker; 
	char *c2mi_buff_tmp;
	char *c2mi_ts_buff;

public:
   DVBC2Multiplexer();
   ~DVBC2Multiplexer();
	//void init();
	void init();
	void quit();
	int open(CHldPlayback *_play_sys);
	int produce_item(char *buff, int size);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	void close();

	int GetC2MIPacket(char **buff, int num_bytes);

	double get_bitrate();
};


// Read from file directly 
class CTLVMedia; 
class DVBC2Reader : public Multiplexer{

private:
	//TS_Reader ts_reader;
	CTLVMedia *media_source;
	char *ts_buff;
public:
	DVBC2Reader();
	~DVBC2Reader();
	void init();
	void quit();
	int open(CHldPlayback *_play_sys);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	int produce_item(char *buff, int size);
	void close();

};


#endif
