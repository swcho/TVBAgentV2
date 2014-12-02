#include <string.h>
#include <malloc.h>

#include "dvbt2_bbframe_payload.h"
#include "utility.h"

#define MATYPE_MASK_TSGS 0xC000
#define MATYPE_MASK_SISMIS 0x2000
#define MATYPE_MASK_CCMACM 0x1000
#define MATYPE_MASK_ISSYI 0x0800
#define MATYPE_MASK_NPD 0x0400
#define MATYPE_MASK_EXT 0x0300




static void re_init(t2mi_payload_bbframe_t *handle)
{
    if ( handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_NORMAL) // Normal Mode
    {
        handle->s_payload_bb.bb_header.upl_issy2msb.upl = 188*8;
    }
    else			// High Efficiency Mode
    {
        handle->s_payload_bb.bb_header.upl_issy2msb.issy2msb = 0;
    }

    if ( handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_NORMAL ) // normal mode
    {
            handle->s_payload_bb.bb_header.sync_issy.sync = 0x47;
    }
    else // hew mode
    {
            handle->s_payload_bb.bb_header.sync_issy.sync = 0x00;
    }
}



static int packet_adatation(t2mi_payload_bbframe_t *handle, char *buff)
{
        int 	packet_size_adaptation;
		unsigned char crc_value;
		char tmpbuf[188];
        if (handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_NORMAL)
        {
                packet_size_adaptation = 188;

                // replace sync by the preceeding_up_crc8
                buff[0] = handle->preceding_up_crc8;
                crc_value = 0;
                crc_value = crc8((unsigned char *)&buff[1], 187, crc_value);
                handle->preceding_up_crc8 = crc_value;


        }else if (handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_HEM)
        {
                packet_size_adaptation = 187;
				if(sizeof(long) > 4)
				{
					memcpy(&tmpbuf[0], &buff[1], packet_size_adaptation);
					memcpy(&buff[0], &tmpbuf[0], packet_size_adaptation);
				}
				else
					memcpy(&buff[0], &buff[1], packet_size_adaptation);

        }

        return packet_size_adaptation;
}






static int get_UP_in_bytes(t2mi_payload_bbframe_t *handle, char *buff, int num_bytes, int *_syncd)
{

        int buff_index = 0;
		int num_byte_remainder;
        int syncd_buff_size_in_byte = handle->syncd/8;
        int syncd_buff_index = 0;
		char tmp_buff[PLAYBACK_TS_PACKET_SIZE_MAX];
		int num_byte_div, num_byte_mod, i;
		int AUPL;
        // read data in syncd_buff if any
        {
            int i;
            for (i = 0; i < syncd_buff_size_in_byte && num_bytes >= syncd_buff_size_in_byte; i++)
            {
                    buff[buff_index] = handle->ts_syncd_buff[i];
                    buff_index++;
            }
            // ts_syncd buffer is empty now
            *_syncd = handle->syncd;
            handle->syncd = 0;
        }

        num_byte_remainder = num_bytes - syncd_buff_size_in_byte;

        if (num_byte_remainder > 0)
        {

                AUPL = 0; // adaptation user packet len
                if (handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_NORMAL)
                        AUPL = 188;
                else if (handle->s_payload_bb.bb_header.HEM == DVBT2_PLP_MODE_HEM)
                        AUPL = 187;

                num_byte_div = num_byte_remainder/AUPL;
                num_byte_mod = num_byte_remainder%AUPL;

                // read ts from file
                
                for (i = 0; i < num_byte_div; i++)
                {
                        int return_size = handle->callback_func(handle, &buff[buff_index], handle->callback_param);
                        if (return_size < 0)
                        {
                          return -1; ;
                        }
                        return_size = packet_adatation(handle, &buff[buff_index]);
                        if (return_size < 0)
                        {
                          return -1; ;
                        }
                        buff_index += return_size;
                }

                syncd_buff_index = 0;
                if (num_byte_mod > 0)
                {
                        int return_size = handle->callback_func(handle, &tmp_buff[0], handle->callback_param);
                        if (return_size < 0)
                                return -1;
                        return_size = packet_adatation(handle, &tmp_buff[0]);
                        if (return_size < 0)
                                return -1;


                        for (i = 0; i < return_size; i++){
                                if (i < num_byte_mod)
                                {
                                        buff[buff_index] =  tmp_buff[i];
                                        buff_index++;
                                }else
                                {
                                        handle->ts_syncd_buff[syncd_buff_index] = tmp_buff[i];
                                        syncd_buff_index++;
                                }
                        }
                        handle->syncd = syncd_buff_index*8;
                }
        }

        return buff_index;
}



static  int BBFrameToBuff(t2mi_payload_bbframe_t *handle, unsigned char *buff){

        unsigned int length_in_bits = 0;
        unsigned char crc_value = 0;

        // data field is the same as max_dfl...
        unsigned int data_field_size_in_bytes = handle->s_payload_bb.bb_header.dfl/8;
        int syncd;
        int return_size =  get_UP_in_bytes(handle, (char *)&buff[10], data_field_size_in_bytes, &syncd);


        if (return_size < 0)
        {
                return -1; //
        }

        handle->s_payload_bb.bb_header.dfl = return_size*8;
        handle->s_payload_bb.bb_header.syncd = syncd;

        //////////////////////////////////////////
        // bbframe_header ---> buff (80 bits)

        {
        buff[0] = (handle->s_payload_bb.bb_header.matype >> 8) & 0xFF;
        buff[1] = (handle->s_payload_bb.bb_header.matype & 0x00FF) & 0xFF;

        buff[2] = (handle->s_payload_bb.bb_header.upl_issy2msb.upl >> 8) & 0xFF;
        buff[3] = (handle->s_payload_bb.bb_header.upl_issy2msb.upl & 0x00FF) & 0xFF;

        buff[4] = (handle->s_payload_bb.bb_header.dfl >> 8) & 0xFF;
        buff[5] = (handle->s_payload_bb.bb_header.dfl & 0x00FF) & 0xFF;

        buff[6] = (handle->s_payload_bb.bb_header.sync_issy.sync) & 0xFF;

        buff[7] = (handle->s_payload_bb.bb_header.syncd >> 8) & 0xFF;
        buff[8] = (handle->s_payload_bb.bb_header.syncd  & 0x00FF) & 0xFF;

        // 8 bits


        crc_value = crc8(buff, 9, crc_value);
        buff[9] = crc_value ^ handle->s_payload_bb.bb_header.HEM;

        length_in_bits = 80;

        }

        length_in_bits += return_size*8;
        return length_in_bits;
}



t2mi_payload_bbframe_t *t2mi_bbframe_init(get_one_ts_packet_t _callback_func, void *_callback_param)
{
		struct BBFrameHeader *bb_frame_header;
        t2mi_payload_bbframe_t *handle = (t2mi_payload_bbframe_t*)malloc(sizeof(t2mi_payload_bbframe_t));

        // initialize default value
        handle->callback_func = _callback_func;
        handle->callback_param = _callback_param;


        // syncd buffer
        memset(&handle->ts_syncd_buff[0], 0, 300);
        handle->syncd = 0;

        handle->preceding_up_crc8 = 0;



        // init bbframe header


        bb_frame_header = &(handle->s_payload_bb.bb_header);

        // default value:
        bb_frame_header->matype  = 0xF000; //TS | SinglePLP | CCM | ISSYT (inactive) | NPD (inactive) | not use
        bb_frame_header->HEM = DVBT2_PLP_MODE_NORMAL; // normal mode
        bb_frame_header->dfl = 53840; // FEC: 64K, code-rate: 5/6

        // infer-values
        re_init(handle);

        return handle;
}

void t2mi_bbframe_quit(t2mi_payload_bbframe_t **handle)
{
    free(*handle);
    *handle = NULL;
}

int t2mi_bbframe_set_dfl(t2mi_payload_bbframe_t *handle, int dfl)
{
    handle->s_payload_bb.bb_header.dfl = dfl;
    re_init(handle);
    return 0;
}

int t2mi_bbframe_set_sis_mode(t2mi_payload_bbframe_t *handle, int is_sis)
{
    if (is_sis)
    {
         handle->s_payload_bb.bb_header.matype =  handle->s_payload_bb.bb_header.matype | MATYPE_MASK_SISMIS; // enable SIS/MIS field
    }else
    {
        handle->s_payload_bb.bb_header.matype =  handle->s_payload_bb.bb_header.matype & ~MATYPE_MASK_SISMIS; // clear SIS/MIS field
    }
    re_init(handle);

    return 0;
}

int t2mi_bbframe_set_HEM(t2mi_payload_bbframe_t *handle, int HEM)
{
    handle->s_payload_bb.bb_header.HEM = HEM;
    re_init(handle);
    return 0;
}



int t2mi_bbframe_GetLenInBits(t2mi_payload_bbframe_t *handle){
        return 8 + 8 + 8 /*T2Mi header */ + 80 /* BBframe header */ + handle->s_payload_bb.bb_header.dfl + 0 /* padding */;
}

int t2mi_bbframe_MakeBBPayload(t2mi_payload_bbframe_t *handle, unsigned char frame_idx, unsigned char plp_id,  unsigned char _intl_frame_start)
{

        // make payload header
        handle->s_payload_bb.frame_idx = frame_idx;
        handle->s_payload_bb.plp_id = plp_id;
        handle->s_payload_bb.intl_frame_start = _intl_frame_start;
        handle->s_payload_bb.rfu = 0x00;

        return 0;
}


// return length in bits
int t2mi_bbframe_PayloadToBuff(t2mi_payload_bbframe_t *handle, unsigned char *buff)
{
        int length_in_bits;

        // frame index
        buff[0] = handle->s_payload_bb.frame_idx;
        buff[1] = handle->s_payload_bb.plp_id;
        buff[2] = handle->s_payload_bb.intl_frame_start << 7;
        buff[2] = (buff[2] & 0x80) | (handle->s_payload_bb.rfu & ~0x80);


        // bbframe
        length_in_bits = BBFrameToBuff(handle, &buff[3]);
        return length_in_bits + 8 + 8 + 8;
}










