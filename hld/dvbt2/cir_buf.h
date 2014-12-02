#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef _CIR_BUF_H_
#define _CIR_BUF_H_ 
// cir_buff in C implementation 

typedef struct cir_buff_s{
	char *cir_buff;
	// size in bytes
	int cir_buff_size;
	// point to start position
	int cir_buff_start;
	// point to end position
	int cir_buff_end;

	int item_size;
	// number of item
	// number of actual item for buffering data: num_item + 1
	int num_item; 
}cir_buf_t;

cir_buf_t* cir_buf_init(int _item_size, int _num_item);
void cir_buf_quit(cir_buf_t* _handle);
int cir_buf_is_full(cir_buf_t* _handle);
int cir_buf_is_empty(cir_buf_t* _handle);
int cir_buf_buffer_item(cir_buf_t* _handle, char *item);
int cir_buf_num_buffered_item(cir_buf_t* _handle);
int cir_buf_num_unbuffered_item(cir_buf_t* _handle);
int cir_buf_buffer_n_item(cir_buf_t* _handle, char *item, int _num_item);
int cir_buf_debuffer_item(cir_buf_t* _handle, char *item);
int cir_buf_debuffer_n_item(cir_buf_t* _handle, char *item, int _num_item);


#ifdef __cplusplus
}
#endif


#endif
