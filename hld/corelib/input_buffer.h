//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _INPUT_BUFFER_
#define _INPUT_BUFFER_


class CTLVBuffer: public CTLVMedia
{
	int is_start;
public: 
	
	CTLVBuffer();
	~CTLVBuffer();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *tmp);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};

//2012/7/6 DVB-T2 ASI
class CTS_IN_Buffer: public CTLVMedia
{
	int is_start;
	int read_data_size;
	int sync_pos;
	int packet_size;
	FILE *fp;
public: 

	CTS_IN_Buffer();
	~CTS_IN_Buffer();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *tmp);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
	int	call_IsFpgaCapBuf_Underrun_toMinLevel(void);
	void call_SetHwFifoCntl_();
	void call_SetHwDmaDiection_LoopThru(void);
};


#endif
