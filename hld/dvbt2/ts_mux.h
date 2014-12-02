#ifndef _TS_MUX_H_
#define _TS_MUX_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TS_MUX_EOF -1


// callback function to get data
typedef int (* f_get_payload_data_t)(char *buff, int max, void *param);

// handle
typedef void * ts_mux_handle_t; 

// APIs
ts_mux_handle_t ts_mux_init(f_get_payload_data_t callback_func, void *callback_param);
int ts_mux_quit(ts_mux_handle_t* handle);
int ts_mux_set_pid(ts_mux_handle_t handle, int _pid);
int ts_mux_set_packet_size(ts_mux_handle_t handle, int packet_size);
int ts_mux_get_a_packet(ts_mux_handle_t handle, char *buff);



#ifdef __cplusplus
}
#endif

#endif
