#ifndef _DVBT2_BBFRAME_
#define _DVBT2_BBFRAME_


#include "dvbt2_system.h"

#ifdef __cplusplus
extern "C"
{
#endif



// define a function pointer
typedef int (*get_one_ts_packet_t)(void *owner, char *buff, void *param);


typedef struct t2mi_payload_bbframe_s
{
    get_one_ts_packet_t callback_func;
    void *callback_param;

    struct DVBT2_PARAM * p_dvbt2_input_param;
    int plp_index;

    unsigned int K_bch;
    int dfl;
    struct T2MI_BBFrame_Payload s_payload_bb;

    char ts_syncd_buff[300];
    int syncd;
    // for mod adaptation (NM or HEM)
    // normal mode (NM)
    unsigned char preceding_up_crc8;

}t2mi_payload_bbframe_t;



// APIs

t2mi_payload_bbframe_t *t2mi_bbframe_init(get_one_ts_packet_t _callback_func, void *_callback_param);
void t2mi_bbframe_quit(t2mi_payload_bbframe_t **handle);
int t2mi_bbframe_set_dfl(t2mi_payload_bbframe_t *handle, int dfl);
int t2mi_bbframe_set_sis_mode(t2mi_payload_bbframe_t *handle, int is_sis);
int t2mi_bbframe_set_HEM(t2mi_payload_bbframe_t *handle, int HEM);
int t2mi_bbframe_GetLenInBits(t2mi_payload_bbframe_t *handle);
int t2mi_bbframe_MakeBBPayload(t2mi_payload_bbframe_t *handle, unsigned char frame_idx, unsigned char plp_id,  unsigned char _intl_frame_start);
int t2mi_bbframe_PayloadToBuff(t2mi_payload_bbframe_t *handle, unsigned char *buff);





#ifdef __cplusplus
}
#endif

#endif
