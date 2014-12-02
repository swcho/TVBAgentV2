#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>

typedef enum {
	log_type_fatal,  // system down, should call "exit" function 
	log_type_error,  // out-off system need to handle this error. For example, can not reading data from file during playing, hardware device is power-off during playing, etc.
	log_type_warning, // errors wich the system can handle. For example, sync code (0x47) is not detected correctly
	log_type_notice,  // send the system status such as, current settings of system, start playing, stop playing, looping, etc. 
	log_type_debug
}LogType;

typedef struct {
	LogType type;
	int code; // 
	char msg[256]; 
} LOG;

class tlv_log
{
private:
	FILE *fp;
	int hConHandle;

public:
	tlv_log();
	~tlv_log();
	void init();
	void quit();
public:
	static tlv_log *get_instance();
	static void print(char *log_mgs);
};

#endif