//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _INPUT_FILE_
#define _INPUT_FILE_

#include "media_interface.h"


class CTLVFileFake: public CTLVMedia
{
public:
	int is_closed;

public: 
	CTLVFileFake();
	~CTLVFileFake();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};


class CTLVFile: public CTLVMedia
{
public:

	char *ts_buffer; 
	int ts_buffer_start;
	int ts_buffer_size;
public:
	FILE *file_handler; 
	int is_closed;

public: 
	CTLVFile();
	~CTLVFile();
	virtual void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};

// for multiplexing 
// its existing is to handle EOF event of each PLP
// 

#define IO_IDVBT2_SET_BITRATE 0
#define IO_IDVBT2_GET_BITRATE 1
#define IO_IDVBT2_SET_PACKETSIZE 2
#define IO_IDVBT2_GET_PACKETSIZE 3

class CTLVFileDVBT2Mul: public CTLVFile
{
private:
	FILE *debug_fp;
	int in_bitrate;
	char *tempT2buf;
public:
	CTLVFileDVBT2Mul();
	~CTLVFileDVBT2Mul();
public: 
	void open(void *file_path);
	int read_data(char *buff_unuse, int max_num_byte);
	int _read_mpeg2ts_data(char *buff);
	int ioctl(int count, ... );

};

// for reading t2 file 
class CTLVFileDVBT2: public CTLVMedia
{
	FILE *file_handler; 
	int is_closed;

public:

	char *ts_buffer; 
	int ts_buffer_start;
	int ts_buffer_size;

public: 
	CTLVFileDVBT2();
	~CTLVFileDVBT2();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};



class CTLVFileISDBT: public CTLVMedia
{
	FILE *file_handler; 
	int is_closed;

	char *ts_buffer; 
	int ts_buffer_start;
	int ts_buffer_size;

public: 
	CTLVFileISDBT();
	~CTLVFileISDBT();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};


// for multiplexing 
// its existing is to handle EOF event of each PLP
// 
class CTLVFileDVBC2Mul: public CTLVFile
{
private:
	int in_bitrate;
public:
	CTLVFileDVBC2Mul();
	~CTLVFileDVBC2Mul();
public: 
	void open(void *file_path);
	int read_data(char *buff_unuse, int max_num_byte);
	int _read_mpeg2ts_data(char *buff);
	int ioctl(int count, ... );

};

// for reading c2 file 
class CTLVFileDVBC2: public CTLVMedia
{
	FILE *file_handler; 
	int is_closed;

public:

	char *ts_buffer; 
	int ts_buffer_start;
	int ts_buffer_size;

public: 
	CTLVFileDVBC2();
	~CTLVFileDVBC2();
	void open(void *file_path);
	int read_one_packet(char *buff);
	int read_n_byte(char *buff, int num_byte);
	void close(void *);
	int _read_mpeg2ts_data(char *buff);
	int read_data(char *buff, int max_num_byte);
	void force_close();
};





#endif
