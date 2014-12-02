#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cir_buf.h"

static FILE *file = NULL;

cir_buf_t* cir_buf_init(int _item_size, int _num_item)
{
	//file = fopen("C:/ts/test/ts_remux_out.ts", "wb");
	cir_buf_t *_cir_buf = (cir_buf_t *)malloc(sizeof(cir_buf_t));
	_cir_buf->item_size = _item_size;
	_cir_buf->num_item = _num_item;
	_cir_buf->cir_buff_size = (_num_item + 1)*_item_size;
	_cir_buf->cir_buff = (char *) malloc(_cir_buf->cir_buff_size);
	_cir_buf->cir_buff_start = 0;
	_cir_buf->cir_buff_end = 0;

	return _cir_buf;
	
}

void cir_buf_quit(cir_buf_t* _handle)
{
	if (file != NULL)
		fclose(file);
	if (_handle->cir_buff != NULL)
	{	
//		while(1);
		free(_handle->cir_buff);		
	}	
	free(_handle);
}


int cir_buf_is_full(cir_buf_t* _handle)
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if ((_handle->cir_buff_end + _handle->item_size) % _handle->cir_buff_size != _handle->cir_buff_start)
		return 0;
	else
		return 1;

}

int cir_buf_is_empty(cir_buf_t* _handle)
{
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if (_handle->cir_buff_end != _handle->cir_buff_start)
		return 0; // not empty
	else
		return 1; // empty


}


int cir_buf_buffer_item(cir_buf_t* _handle, char *item)
{
	if (_handle->cir_buff == NULL)
		return -1;
	
	//Use modulo as a trick to wrap around the end of the buffer back to the beginning
	if ((_handle->cir_buff_end + _handle->item_size) % _handle->cir_buff_size != _handle->cir_buff_start){
		// not full 
		memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, _handle->item_size);	
		_handle->cir_buff_end = (_handle->cir_buff_end + _handle->item_size)% _handle->cir_buff_size;
		return 0;
	}else {
		return -1;
	}

}

int cir_buf_num_buffered_item(cir_buf_t* _handle)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (_handle->cir_buff_start <= _handle->cir_buff_end)
		return (_handle->cir_buff_end - _handle->cir_buff_start)/_handle->item_size;
	else
		return (_handle->cir_buff_size - (_handle->cir_buff_start - _handle->cir_buff_end))/_handle->item_size;

}


int cir_buf_num_unbuffered_item(cir_buf_t* _handle)
{
	int num_buffered_item;
	if (_handle->cir_buff == NULL)
		return -1;

	num_buffered_item = cir_buf_num_buffered_item(_handle);
	return _handle->num_item - num_buffered_item;
}


int cir_buf_buffer_n_item(cir_buf_t* _handle, char *item, int _num_item)
{
	int num_item_buffered;
	int num_item_unbuffered, gap_end_and_last_item;
	int num_item_xxx, end_item_index;
	if (_handle->cir_buff == NULL)
		return -1;

	if (cir_buf_is_full(_handle) == 1)
		return 0;

	// get number of item which is buffered
	num_item_buffered = cir_buf_num_buffered_item(_handle);
	// num of item unbuffered
	num_item_unbuffered = _handle->num_item - num_item_buffered;
	// num of item for buffering
	num_item_xxx = (num_item_unbuffered < _num_item ? num_item_unbuffered: _num_item);

	// not good naming ????
	end_item_index = _handle->cir_buff_end/_handle->item_size; 
    gap_end_and_last_item = _handle->num_item /* last index */ - end_item_index + 1;
	if (num_item_xxx <= gap_end_and_last_item)
	{
			memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, num_item_xxx*_handle->item_size);
			_handle->cir_buff_end = (_handle->cir_buff_end + num_item_xxx*_handle->item_size) % _handle->cir_buff_size;
			return num_item_xxx;
	} else {
                memcpy(&_handle->cir_buff[_handle->cir_buff_end], item, gap_end_and_last_item*_handle->item_size);
		_handle->cir_buff_end = (_handle->cir_buff_end + gap_end_and_last_item*_handle->item_size) % _handle->cir_buff_size;
		assert(_handle->cir_buff_end == 0); // pointer_end is wrapped around

		memcpy(&_handle->cir_buff[_handle->cir_buff_end], item + gap_end_and_last_item*_handle->item_size, (num_item_xxx - gap_end_and_last_item)*_handle->item_size);
		_handle->cir_buff_end = (_handle->cir_buff_end + (num_item_xxx - gap_end_and_last_item)*_handle->item_size) % _handle->cir_buff_size;
		return num_item_xxx;
	}


}



int cir_buf_debuffer_item(cir_buf_t* _handle, char *item)
{
	if (_handle->cir_buff == NULL)
		return -1;

	if (_handle->cir_buff_end != _handle->cir_buff_start) {
		memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], _handle->item_size);
        _handle->cir_buff_start = (_handle->cir_buff_start + _handle->item_size) % _handle->cir_buff_size;
        return 0;
	}
//otherwise, the buffer is empty; return an error code
	return -1;


}
int cir_buf_debuffer_n_item(cir_buf_t* _handle, char *item, int _num_item)
{
	int num_item_buffered, num_item_xxx, gap_start_and_last_item;
	if (_handle->cir_buff == NULL)
		return -1;

	if (cir_buf_is_empty(_handle) == 1)
		return 0;

	// get number of item which is buffered
	num_item_buffered = cir_buf_num_buffered_item(_handle);
	// num of item for debufferting
	num_item_xxx = (num_item_buffered < _num_item ? num_item_buffered: _num_item);

	// not good naming ????
	gap_start_and_last_item = _handle->num_item + 1 - _handle->cir_buff_start/_handle->item_size;

	if (num_item_xxx <= gap_start_and_last_item)
	{
			memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], num_item_xxx*_handle->item_size);
			_handle->cir_buff_start = (_handle->cir_buff_start + num_item_xxx*_handle->item_size) % _handle->cir_buff_size;
			return num_item_xxx;
	} else {
		memcpy(item, &_handle->cir_buff[_handle->cir_buff_start], gap_start_and_last_item);
		_handle->cir_buff_start = (_handle->cir_buff_start + gap_start_and_last_item*_handle->item_size) % _handle->cir_buff_size;
		//ssert(cir_buff_start == 0); // pointer_end is wrapped around

		memcpy(item + gap_start_and_last_item*_handle->item_size, &_handle->cir_buff[_handle->cir_buff_start], (num_item_xxx - gap_start_and_last_item)*_handle->item_size);
		_handle->cir_buff_start = (_handle->cir_buff_start + (num_item_xxx - gap_start_and_last_item)*_handle->item_size) % _handle->cir_buff_size;
		return num_item_xxx;
	}



}

