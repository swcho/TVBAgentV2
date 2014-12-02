#include <malloc.h>
#include <string.h>
#include <assert.h>


#include "dvbt2_system.h"
#include "utility.h"
#include "dvbt2_bbframe_payload.h"
#include "dvbt2_timestamp_payload.h"
#include "dvbt2_l1_payload.h"
#include "dvbt2_multipleplp.h"


//#define MULTI_PLP_DEBUG



static int _timestamp_size_in_bits()
{
	return 88; // 4 + 4 + 40 + 27 + 13
}

static int Max(int num_plp, double *_mcdf)
{
        int index_max_cdf = 0;
        double max_cdf = _mcdf[index_max_cdf];

        int i;
        for (i = 1; i < num_plp; i++)
        {
                if (max_cdf < _mcdf[i])
                {
                        index_max_cdf = i;
                        max_cdf = _mcdf[index_max_cdf];
                }
        }

        return index_max_cdf;
}


static int dvbt2_CalculateTSBitrate(t2mi_handle_t *handle)
{


    double t2_duration = t2frame_duration(handle->dvbt2_input_param.FFT_SIZE,
                                     handle->dvbt2_input_param.GUARD_INTERVAL,
                                     handle->dvbt2_input_param.NUM_DATA_SYMBOLS,
                                     handle->dvbt2_input_param.BW);

    int total_num_ts_packet = 0;

    // #ts packets for BB payload
    int bb_payload_bytes_total = 0;
    int i, num_ts_packet;
	int plp_index, len_in_bytes;
	int total_bit, ts_bitrate;
	double avg_bitrate;
	int _dfl_max_in_bits, l1_post_info_size_in_bits;

    for (i = 0; i < handle->PLP_NUM_BLOCKS_TOTAL; i++)
    {
           plp_index = handle->bbframe_order[i];
           len_in_bytes = t2mi_bbframe_GetLenInBits(handle->payload_bb[plp_index])/8;
           num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
           total_num_ts_packet += num_ts_packet;

           //bb_payload_bytes_total += len_in_bytes;
    }

    // #ts packets for L1 payload
	//l1_cur_
    len_in_bytes = l1_cur_get_len_in_bits(&handle->l1_cur)/8;
    num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
    total_num_ts_packet += num_ts_packet;
	   

    // #ts packets for timestampe
    len_in_bytes = _timestamp_size_in_bits()/8;
    num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
    total_num_ts_packet += num_ts_packet;
	
    // t2-frame bitrate
    total_bit = total_num_ts_packet*188*8;
    ts_bitrate  = (int)(total_bit*(1000.0/t2_duration));
    handle->dvbt2_input_param.t2_bitrate = ts_bitrate;



    // bitrate of UP (original useful packet) for each plp
    for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
    {
        _dfl_max_in_bits = t2frame_kbch_size(handle->dvbt2_input_param.list_plp[i].PLP_FEC,
                                                 handle->dvbt2_input_param.list_plp[i].PLP_COD);
        _dfl_max_in_bits = _dfl_max_in_bits - 80;

        avg_bitrate = _dfl_max_in_bits*handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS;
        avg_bitrate = avg_bitrate*(1000.0/t2_duration);

        if ( handle->dvbt2_input_param.list_plp[i].PLP_HEM == DVBT2_PLP_MODE_HEM)
        {
                avg_bitrate *= (188./187.);
        }

        handle->dvbt2_input_param.list_plp[i].UP_BITRATE = (int)avg_bitrate;
    }

    // cell related calculation functions
    handle->dvbt2_input_param.l1_pre_cells = t2frame_l1pre_num_cell();
    l1_post_info_size_in_bits = t2frame_l1pos_dynamic_current(handle->dvbt2_input_param.num_plp, 0 /* num of aux */);
    l1_post_info_size_in_bits += t2frame_l1pos_conflen(handle->dvbt2_input_param.num_plp,
                                                       0 /*num of aux */,
                                                       1,
                                                       handle->dvbt2_input_param.FFT_SIZE);
    handle->dvbt2_input_param.l1_pos_cells = TL_Calculate_L1_Post_Size(handle->dvbt2_input_param.L1_MOD,
                                                    handle->dvbt2_input_param.FFT_SIZE,
                                                    l1_post_info_size_in_bits);
    handle->dvbt2_input_param.t2_duration = (int)t2_duration;

	// for compatible with g++ compiler 
	return 0; 

}


static void dvbt2_BBFrameSelection(int num_plp, int *_plp_num_block, int *_bbframe_order)
{
        int i;
        int j;
//        const int max_num_plp = 10;
//        const int max_sum_num_block = 400;
        //ssert(num_plp <= max_num_plp);
        double p[10][400];
        double mcdf[10]; // mcdf: modified cumulative distribution function
		int index_mcdf;
        int sum_plp_num_block = 0;

        for (i = 0; i < num_plp; i++)
        {
                sum_plp_num_block += _plp_num_block[i];
                mcdf[i] = 0;
        }

        //ssert(sum_plp_num_block <= max_sum_num_block);

        // uniform distribution of probability
        for ( i = 0; i < num_plp; i++)
		{
                for (j = 0; j < sum_plp_num_block; j++)
                {
                        p[i][j] = (double)_plp_num_block[i]/sum_plp_num_block;
                }
        }

        // main algorithm
        for (j = 0; j < sum_plp_num_block; j++)
        {
                for (i = 0; i < num_plp; i++)
                {
                        mcdf[i] += p[i][j];
                }
                index_mcdf = Max(num_plp, &mcdf[0]);
                _bbframe_order[j] = index_mcdf;
                //
                mcdf[index_mcdf] = mcdf[index_mcdf] - 1;
        }

}


// make t2mi packet header
static void dvbt2_MakeT2MIPacketHeader(t2mi_handle_t *handle, char packet_type,  //T2MI_PACKET_TYPE_BBF, T2MI_PACKET_TYPE_L1CURRENT, T2MI_PACKET_TYPE_AUX
        unsigned char packet_count,
        unsigned char superframe_idx)
{
        handle->t2mi_header.packet_type = packet_type;
        handle->t2mi_header.packet_count = packet_count;
        handle->t2mi_header.superframe_idx = superframe_idx;
        handle->t2mi_header.rfu = 0x0000;


}

// make t2mi packet
static int dvbt2_MakeT2MIPacket(t2mi_handle_t *handle, char packet_type,  //T2MI_PACKET_TYPE_BBF, T2MI_PACKET_TYPE_L1CURRENT, T2MI_PACKET_TYPE_AUX
        unsigned char plp_index,
        unsigned char packet_count,
        unsigned char superframe_idx,
        unsigned char frame_idx,
        unsigned char plp_id,
        unsigned char intl_frame_start)
{
        int rc;

        // make header
        dvbt2_MakeT2MIPacketHeader(handle, packet_type, packet_count, superframe_idx);

        ///////////////////
        // make payload
        switch (packet_type)
        {
        case T2MI_PACKET_TYPE_BBF:
                rc = t2mi_bbframe_MakeBBPayload(handle->payload_bb[plp_index], frame_idx, plp_id, intl_frame_start);
                if (rc < 0)
                {
//			printf("assert: can not make bb payload function: %s, line: %d \n", __FUNCTION__, __LINE__);
                        return -1;
                }
                handle->t2mi_header.payload_len = t2mi_bbframe_GetLenInBits(handle->payload_bb[plp_index]);
                break;

        case T2MI_PACKET_TYPE_L1CURRENT:

                l1_cur_gen(&handle->l1_cur, handle->frame_idx);
                handle->t2mi_header.payload_len = l1_cur_get_len_in_bits(&handle->l1_cur);

                ////ssertt2mi_packet.header.payload_len == 536);
                break;
        case T2MI_PACKET_TYPE_TIMESTAMP:
                timestamp_gen(&handle->timestamp);
                handle->t2mi_header.payload_len = _timestamp_size_in_bits();
                break;

        case T2MI_PACKET_TYPE_AUX:
                break;

        default:
                break;
        }

        return 0;

}

// return length in bits
static int dvbt2_T2MIPacketHeaderToBuff(t2mi_handle_t *handle, unsigned char *buff)
{

        buff[0] = handle->t2mi_header.packet_type;
        buff[1] = handle->t2mi_header.packet_count;
        buff[2] = handle->t2mi_header.superframe_idx << 4;
        buff[2] = buff[2] | (handle->t2mi_header.rfu & 0x0F00 >> 8);
        buff[3] = (handle->t2mi_header.rfu & 0x00FF);
        buff[4] = (handle->t2mi_header.payload_len & 0xFF00) >> 8;
        buff[5] = (handle->t2mi_header.payload_len & 0x00FF);

        return 48;
}



// return length in bits
// function: make a buffer from MultiplePLP class
static int dvbt2_T2MIPacketToBuff(t2mi_handle_t *handle, int _plp_id, unsigned char *buff){

        int packet_type;
        int length_in_bits = 0;
        int length_in_bytes;
		unsigned int crc_32;

        packet_type = handle->t2mi_header.packet_type;


        ////////////////////////////////////////////////////////////////////////
        // make T2MI header
        {
                length_in_bits = dvbt2_T2MIPacketHeaderToBuff(handle, &buff[0]);
        }

        switch(packet_type)
        {
        case T2MI_PACKET_TYPE_BBF:
                {

                        int num_bits_return = t2mi_bbframe_PayloadToBuff(handle->payload_bb[_plp_id], &buff[6]);
                        //ssert(num_bits_return % 8 == 0);
                        length_in_bits += num_bits_return;

                        if (handle->file_bb != NULL)
                                fwrite(buff, 1, length_in_bits/8, handle->file_bb);


                }
                break;
        case T2MI_PACKET_TYPE_L1CURRENT:
                {

                        int num_bits_return = l1_cur_to_buffer( &handle->l1_cur,  &buff[length_in_bits/8]);
                        length_in_bits += num_bits_return;

                        if (handle->file_l1_cur != NULL)
                                fwrite(buff, 1, length_in_bits/8, handle->file_l1_cur);


                }
                break;

        case T2MI_PACKET_TYPE_TIMESTAMP:
                {
                        int num_bits_return = timestamp_to_buffer(&handle->timestamp, &buff[length_in_bits/8]);
                        length_in_bits += num_bits_return;
                }

                if (handle->file_timestamp != NULL)
                        fwrite(buff, 1, length_in_bits/8, handle->file_timestamp);


                break;
        default:
                break;
        }       

        // add 32 CRC-32 bits
        length_in_bytes = length_in_bits/8;
        crc_32 = crc32(&buff[0], length_in_bytes, 0xFFFFFFFF);

        buff[length_in_bytes++] = (unsigned char)(crc_32 >> 24);
        buff[length_in_bytes++] = (unsigned char)(crc_32 >> 16);
        buff[length_in_bytes++] = (unsigned char)(crc_32 >> 8);
        buff[length_in_bytes++] = (unsigned char)crc_32;


        length_in_bits += 32;


        return length_in_bits;
}

#if 0
static int _dvbt2_SearchingParamater(t2mi_handle_t *handle, int *NUM_DATA_SYMBOLS, int *PLP_BLOCK )
{
        // compute t2-frame duration
        int fft_size = handle->dvbt2_input_param.FFT_SIZE;
        int guard_interval = handle->dvbt2_input_param.GUARD_INTERVAL;
        int pilot = handle->dvbt2_input_param.Pilot;
        int num_plp = handle->dvbt2_input_param.num_plp;
        int bandwidth = handle->dvbt2_input_param.BW;

        int plp_fec[10];
        int plp_cod[10];
        int plp_mod[10];
        int plp_bitrate[10];
        int i;
        for (i = 0; i < num_plp; i++)
        {
                plp_fec[i] = handle->dvbt2_input_param.list_plp[i].PLP_FEC;
                plp_cod[i] = handle->dvbt2_input_param.list_plp[i].PLP_COD;
                plp_mod[i] = handle->dvbt2_input_param.list_plp[i].PLP_MOD;
                plp_bitrate[i] = handle->dvbt2_input_param.list_plp[i].PLP_BITRATE;

        }


        int rtcd =  dvbt2_SearchingParamater(0, 1, handle->dvbt2_input_param.L1_MOD,
                bandwidth,
                handle->dvbt2_input_param.BW_EXT,
                fft_size,
                guard_interval,
                pilot,
                num_plp,
                &plp_fec[0],
                &plp_cod[0],
                &plp_mod[0],
                &plp_bitrate[0],
                NUM_DATA_SYMBOLS,
                PLP_BLOCK);

        return 0;

}

#endif

static int dvbt2_open(t2mi_handle_t *handle){

        // internal setting
        handle->packet_count = 0;
        handle->superframe_idx = 0;
        handle->plp_index = 0; // suport ovnenly 1 plp_id
        handle->frame_idx = 0;
        handle->packet_index = 0;



        // l1 initialization
        l1_cur_init(&handle->l1_cur, handle->dvbt2_input_param);

#if 0
        // Set number of bit per T2 frame
        {
                int fft_size = dvbt2_input_param.FFT_SIZE;
                int guard_interval = dvbt2_input_param.GUARD_INTERVAL;
                int num_data_symbol = dvbt2_input_param.NUM_DATA_SYMBOLS;
                int bandwidth = dvbt2_input_param.BW;
                double t2_duration = t2frame_duration(fft_size, guard_interval, num_data_symbol, bandwidth);
                for (int i = 0; i < dvbt2_input_param.num_plp; i++){
                        //int dfl_in_bits = payload_bb[i].max_dfl_in_bits;
                        //int num_block = dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS;

                        // todo: target bitrate should include data at P2 symbol
                        // huy: ??
                        //payload_bb[i].input_processing.set_target_bitrate((dfl_in_bits*num_block*1000.0)/t2_duration);
                        //payload_bb[i].input_processing.set_target_bitrate(0);

                }
        }
#endif

       // printf("============== %s %d  \n", __FILE__, __LINE__);
        return 0;
}



static int _get_one_ts_packet(void *owner, char *buff, void *param)
{
    t2mi_handle_t *handle = (t2mi_handle_t *)param;

    // find the index
    int i;
    for (i = 0; i < 10; i++)
	{
		if (handle->payload_bb[i] == owner)
        {
            return handle->callback_func(handle, i, buff, handle->callback_param);

         }
	}
	return 0;
}

// caculate Ctot (number of active cells)
static int _t2frame_num_ctot(t2mi_handle_t *handle)
{
    int c_tot = 0;
    int d_data  = t2frame_data_symbol_num_cell(handle->dvbt2_input_param.FFT_SIZE, handle->dvbt2_input_param.Pilot, handle->dvbt2_input_param.BW_EXT); // number of cell for each data symbol

    int l_p2 = t2frame_p2_num_symbol(handle->dvbt2_input_param.FFT_SIZE);
    int c_p2 = t2frame_p2_symbol_num_cell(handle->dvbt2_input_param.FFT_SIZE, 0 /* SISO mode */);
    int total_np2_cell = c_p2*l_p2;

    if (t2frame_is_closing_symbol(handle->dvbt2_input_param.FFT_SIZE, handle->dvbt2_input_param.GUARD_INTERVAL, handle->dvbt2_input_param.Pilot))
    {
            int l_normal = handle->dvbt2_input_param.NUM_DATA_SYMBOLS -1;
            int total_data_cell = d_data*l_normal;
            total_data_cell += t2frame_data_symbol_closing_frame_num_cell(handle->dvbt2_input_param.FFT_SIZE, handle->dvbt2_input_param.Pilot, handle->dvbt2_input_param.BW_EXT);
            c_tot = total_np2_cell + total_data_cell;
    }
    else
    {
            int total_data_cell = d_data*handle->dvbt2_input_param.NUM_DATA_SYMBOLS;
            c_tot = total_np2_cell + total_data_cell;
    }
    return c_tot;
}


static void _init_pos(t2mi_handle_t *handle)
{
    int i, j;
    int _PLP_NUM_BLOCK[10]; // maximun: 10 plp
	int l1_post_info_size_in_bits;

    if (handle->dvbt2_input_param.GUARD_INTERVAL == DVBT2_GUARD_INTERVAL_1_128 ||
        handle->dvbt2_input_param.GUARD_INTERVAL == DVBT2_GUARD_INTERVAL_19_256 ||
        handle->dvbt2_input_param.GUARD_INTERVAL == DVBT2_GUARD_INTERVAL_19_128)
    {
        if (handle->dvbt2_input_param.FFT_SIZE == DVBT2_FFT_SIZE_8K)
            handle->dvbt2_input_param.FFT_SIZE = DVBT2_FFT_SIZE_8K_E;
        else if (handle->dvbt2_input_param.FFT_SIZE == DVBT2_FFT_SIZE_32K)
            handle->dvbt2_input_param.FFT_SIZE = DVBT2_FFT_SIZE_32K_E;
    }

#if 0 
    // re-calculate the number of block for each plp, when auto-searching number of block is applied
    if (handle->dvbt2_input_param.auto_searching_data_symbol_num_block == 1)
    {
            int _num_data_symbol;
            int _plp_num_block[10];

            for ( i = 0; i < handle->dvbt2_input_param.num_plp; i++){
                    //int plp_bitrate = dvbt2_multiplexer->play_sys->media_interface[i]->mpeg2ts_statistic.get_bitrate();
                    int plp_bitrate = handle->dvbt2_input_param.list_plp[i].PLP_BITRATE;

                    if (plp_bitrate <= 0)
                    {
//				printf("assert: can not calculate bitrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
                            return;
                    }

                    handle->dvbt2_input_param.list_plp[i].PLP_BITRATE = plp_bitrate;
            }

            _dvbt2_SearchingParamater(handle, &_num_data_symbol, &_plp_num_block[0]);

            handle->dvbt2_input_param.NUM_DATA_SYMBOLS = _num_data_symbol;

    for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
    {
                    handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS = _plp_num_block[i];
            }
    }    

#endif


    // internal initialization
    handle->PLP_NUM_BLOCKS_TOTAL = 0;
    for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
    {
            handle->PLP_NUM_BLOCKS_TOTAL += handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS;
            _PLP_NUM_BLOCK[i] = handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS;
    }

    // initialize  BBFrame order algorithm
    {
            int num_plp = handle->dvbt2_input_param.num_plp;
            dvbt2_BBFrameSelection(num_plp,  &_PLP_NUM_BLOCK[0], &handle->bbframe_order[0]);

            // initialize the "is_intl_frame_start"
            for (i = 0; i < handle->PLP_NUM_BLOCKS_TOTAL; i++)
            {
                    handle->is_intl_frame_start[i] = 0;
            }
            for (j = 0; j < num_plp; j++)
            {
                    for (i = 0; i < handle->PLP_NUM_BLOCKS_TOTAL; i++)
                    {
                            if (handle->bbframe_order[i] == j)
                            {
                                    handle->is_intl_frame_start[i] = 1;
                                    break;
                            }
                    }
            }

    }

    // payload initialization
    for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
    {
            handle->payload_bb[i] = t2mi_bbframe_init(_get_one_ts_packet, handle);

            t2mi_bbframe_set_HEM(handle->payload_bb[i], handle->dvbt2_input_param.list_plp[i].PLP_HEM);
            t2mi_bbframe_set_dfl(handle->payload_bb[i], t2frame_kbch_size(handle->dvbt2_input_param.list_plp[i].PLP_FEC, handle->dvbt2_input_param.list_plp[i].PLP_COD) - 80 /* bbframe header */ );
            if (handle->dvbt2_input_param.num_plp == 1)
                t2mi_bbframe_set_sis_mode(handle->payload_bb[i], 1);
            else
                t2mi_bbframe_set_sis_mode(handle->payload_bb[i], 0);
    }

    // calculate #cells of a T2-frame, l1_pre + l1_pos, data cells
    handle->dvbt2_input_param.l1_pre_cells = t2frame_l1pre_num_cell();
    //
    l1_post_info_size_in_bits = t2frame_l1pos_dynamic_current(handle->dvbt2_input_param.num_plp, 0 /*num_aux */) +   t2frame_l1pos_conflen(handle->dvbt2_input_param.num_plp, 0, 1, handle->dvbt2_input_param.FFT_SIZE);
    handle->dvbt2_input_param.l1_pos_cells = TL_Calculate_L1_Post_Size(handle->dvbt2_input_param.L1_MOD, handle->dvbt2_input_param.FFT_SIZE, l1_post_info_size_in_bits);
    //
    handle->dvbt2_input_param.c_tot_cells =  _t2frame_num_ctot(handle);

    handle->dvbt2_input_param.plps_cells = 0;
    for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
    {
        int _plp_num_cell = t2frame_fecframe_num_cell(handle->dvbt2_input_param.list_plp[i].PLP_FEC, handle->dvbt2_input_param.list_plp[i].PLP_MOD);
        handle->dvbt2_input_param.plps_cells += _plp_num_cell*handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS;
    }

    handle->dvbt2_input_param.dummy_cells = handle->dvbt2_input_param.c_tot_cells - (handle->dvbt2_input_param.l1_pre_cells + handle->dvbt2_input_param.l1_pos_cells + handle->dvbt2_input_param.plps_cells);

   dvbt2_CalculateTSBitrate(handle);



    // timestamp initialization
    timestamp_init(&handle->timestamp, handle->dvbt2_input_param.BW);

    dvbt2_open(handle);

}











t2mi_handle_ptr t2mi_init(read_one_packet_t _callback_func, void *_callback_params)
{

    t2mi_handle_t *handle = (t2mi_handle_t *)malloc(sizeof(t2mi_handle_t));
    int i;

    handle->callback_func = _callback_func;
    handle->callback_param = _callback_params;


#if 1
    // set default values
    // general parameters
    handle->dvbt2_input_param.BW = DVBT2_BW_8;
    handle->dvbt2_input_param.BW_EXT = DVBT2_BWT_NORMAL;
    handle->dvbt2_input_param.FFT_SIZE = DVBT2_FFT_SIZE_8K;
    handle->dvbt2_input_param.GUARD_INTERVAL = DVBT2_GUARD_INTERVAL_1_16;
    handle->dvbt2_input_param.L1_MOD = DVBT2_MOD_QAM64;
    // todo: think of removing this parameter
    handle->dvbt2_input_param.L1_COD = 0; // code-rate: 1/2
    handle->dvbt2_input_param.Pilot = DVBT2_PILOT_PATTERN_PP5;
    handle->dvbt2_input_param.PAPR = 0; // no PART
    handle->dvbt2_input_param.FREQUENCY = 474000000;
    handle->dvbt2_input_param.NETWORK_ID = 12421;
    handle->dvbt2_input_param.T2_ID = 32769;
    handle->dvbt2_input_param.Cell_ID = 0;
    handle->dvbt2_input_param.L1_REPETITION = 0;

    // T2-frame
    handle->dvbt2_input_param.NUM_T2_FRAME = 2;
    handle->dvbt2_input_param.NUM_DATA_SYMBOLS = 150;

    // PLP setting
    handle->dvbt2_input_param.num_plp = 1;

    for (i = 0; i < 8 /* magic number */; i++)
    {
        handle->dvbt2_input_param.list_plp[i].PLP_ID = i;
        handle->dvbt2_input_param.list_plp[i].PLP_TYPE = DVBT2_PLP_TYPE_DATATYPE1;
        handle->dvbt2_input_param.list_plp[i].PLP_MOD = DVBT2_PLP_MOD_256QAM;
        handle->dvbt2_input_param.list_plp[i].PLP_COD = DVBT2_PLP_COD_5_6;
        handle->dvbt2_input_param.list_plp[i].PLP_ROTATION = DVBT2_PLP_ROT_ON;
        handle->dvbt2_input_param.list_plp[i].PLP_FEC = DVBT2_PLP_FEC_TYPE_64K_LDPC;
        handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS = 123;
        handle->dvbt2_input_param.list_plp[i].PLP_HEM = DVBT2_PLP_MODE_HEM;
        handle->dvbt2_input_param.list_plp[i].PLP_TIME_IL_TYPE = 0; // No Time interleaver
        handle->dvbt2_input_param.list_plp[i].PLP_TIME_IL_LENGTH = 0;
        handle->dvbt2_input_param.list_plp[i].ISSY = DVBT2_PLP_ISSY_OFF;
    }

#else
    // set default values
    // general parameters
    handle->dvbt2_input_param.BW = DVBT2_BW_8;
    handle->dvbt2_input_param.BW_EXT = DVBT2_BWT_NORMAL;
    handle->dvbt2_input_param.FFT_SIZE = DVBT2_FFT_SIZE_8K_E;
    handle->dvbt2_input_param.GUARD_INTERVAL = DVBT2_GUARD_INTERVAL_1_128;
    handle->dvbt2_input_param.L1_MOD = DVBT2_MOD_QAM64;
    // todo: think of removing this parameter
    handle->dvbt2_input_param.L1_COD = 0; // code-rate: 1/2
    handle->dvbt2_input_param.Pilot = DVBT2_PILOT_PATTERN_PP7;
    handle->dvbt2_input_param.PAPR = 0; // no PART
    handle->dvbt2_input_param.FREQUENCY = 474000000;
    handle->dvbt2_input_param.NETWORK_ID = 12421;
    handle->dvbt2_input_param.T2_ID = 32769;
    handle->dvbt2_input_param.Cell_ID = 0;
    handle->dvbt2_input_param.L1_REPETITION = 0;

    // T2-frame
    handle->dvbt2_input_param.NUM_T2_FRAME = 2;    
    handle->dvbt2_input_param.NUM_DATA_SYMBOLS = 59;

    // PLP setting
    handle->dvbt2_input_param.num_plp = 1;

    for (i = 0; i < 8 /* magic number */; i++)
    {
        handle->dvbt2_input_param.list_plp[i].PLP_ID = i;
        handle->dvbt2_input_param.list_plp[i].PLP_TYPE = DVBT2_PLP_TYPE_DATATYPE1;
        handle->dvbt2_input_param.list_plp[i].PLP_MOD = DVBT2_PLP_MOD_256QAM;
        handle->dvbt2_input_param.list_plp[i].PLP_COD = DVBT2_PLP_COD_5_6;
        handle->dvbt2_input_param.list_plp[i].PLP_ROTATION = DVBT2_PLP_ROT_ON;
        handle->dvbt2_input_param.list_plp[i].PLP_FEC = DVBT2_PLP_FEC_TYPE_64K_LDPC;
        handle->dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS = 48;
        handle->dvbt2_input_param.list_plp[i].PLP_HEM = DVBT2_PLP_MODE_HEM;
        handle->dvbt2_input_param.list_plp[i].PLP_TIME_IL_TYPE = 0; // No Time interleaver
        handle->dvbt2_input_param.list_plp[i].PLP_TIME_IL_LENGTH = 0;
        handle->dvbt2_input_param.list_plp[i].ISSY = DVBT2_PLP_ISSY_OFF;
    }

#endif

    _init_pos(handle);

    handle->file_bb = NULL;
    handle->file_l1_cur = NULL;
    handle->file_timestamp = NULL;
    handle->file_muxed_t2 = NULL;

//    handle->file_bb = fopen("/media/win37/bb.t2mi", "wb");
//    handle->file_l1_cur = fopen("/media/win37/l1.t2mi", "wb");
//    handle->file_timestamp =  fopen("/media/win37/time.t2mi", "wb");
//    handle->file_muxed_t2 = fopen("/media/win37/t2mi_only.t2mi", "wb");

    return (t2mi_handle_ptr)handle;
}



int t2mi_quit(t2mi_handle_ptr _handle)
{
     int i;
     t2mi_handle_t *handle = (t2mi_handle_t *)_handle;

    if (handle == NULL)
        return -1;


        for (i = 0; i < handle->dvbt2_input_param.num_plp; i++)
            t2mi_bbframe_quit(&handle->payload_bb[i]);


        if (handle->file_bb != NULL)
             fclose(handle->file_bb);

        if (handle->file_l1_cur != NULL)
             fclose(handle->file_l1_cur);

        if (handle->file_timestamp != NULL)
             fclose(handle->file_timestamp);

        if (handle->file_muxed_t2 != NULL)
            fclose(handle->file_muxed_t2);

        free(handle);
        handle = NULL;
	return 0;
}

void t2mi_get_param(t2mi_handle_ptr _handle, struct DVBT2_PARAM *param)
{
    t2mi_handle_t *handle = (t2mi_handle_t *)_handle;
    *param = handle->dvbt2_input_param;
}

int t2mi_set_param(t2mi_handle_ptr _handle, struct DVBT2_PARAM *param)
{

    t2mi_handle_t *handle = (t2mi_handle_t *)_handle;
    handle->dvbt2_input_param = *param;
    //tv_printf("-Set values to T2MI\n");
    _init_pos(handle);
    *param = handle->dvbt2_input_param;
    return 0;
}

int t2mi_set_callback_param(t2mi_handle_ptr _handle, void *_callback_param)
{
    t2mi_handle_t *handle = (t2mi_handle_t *)_handle;
    handle->callback_param = _callback_param;

	return 0;
}

int t2mi_get_t2_packet(t2mi_handle_ptr _handle, struct t2mi_packet_status *status, unsigned char *buff)
{
    int plp_id = 0;
    int intl_frame_start = 0;
	int rc;
    t2mi_handle_t *handle = (t2mi_handle_t *)_handle;

    if (handle->packet_index < handle->PLP_NUM_BLOCKS_TOTAL)
    {
            handle->plp_index = handle->bbframe_order[handle->packet_index];
            intl_frame_start = handle->is_intl_frame_start[handle->packet_index];
            plp_id = handle->dvbt2_input_param.list_plp[handle->plp_index].PLP_ID;
            rc= dvbt2_MakeT2MIPacket(handle, T2MI_PACKET_TYPE_BBF, handle->plp_index, handle->packet_count, handle->superframe_idx, handle->frame_idx, plp_id, intl_frame_start);
            if (rc < 0)
                    return -1;

            status->packet_type = T2MI_PACKET_TYPE_BBF;
            goto END_PACKET;
    }else if (handle->packet_index == handle->PLP_NUM_BLOCKS_TOTAL)
    { // timestamp

            //printf("debug %s, %d \n", __FILE__, __LINE__);

            dvbt2_MakeT2MIPacket(handle, T2MI_PACKET_TYPE_TIMESTAMP, handle->plp_index, handle->packet_count, handle->superframe_idx, handle->frame_idx, plp_id, intl_frame_start);
            status->packet_type = T2MI_PACKET_TYPE_TIMESTAMP;
            goto END_PACKET;
    }else
    { // l1
            //printf("debug %s, %d \n", __FILE__, __LINE__);
            dvbt2_MakeT2MIPacket(handle, T2MI_PACKET_TYPE_L1CURRENT, handle->plp_index, handle->packet_count, handle->superframe_idx, handle->frame_idx, plp_id, intl_frame_start);
            status->packet_type = T2MI_PACKET_TYPE_L1CURRENT;
            // goto end_frame
    }

    //END_FRAME:
    // frame_idx is incremented by each "T2MI packet transition"
    handle->frame_idx++;
    handle->frame_idx = (handle->frame_idx >= handle->dvbt2_input_param.NUM_T2_FRAME ? 0: handle->frame_idx);
    // superframe_idx is incremented by each T2MI_FRAME_IDX_MAX
    handle->superframe_idx = (handle->frame_idx == 0 ? handle->superframe_idx + 1: handle->superframe_idx);
    handle->superframe_idx = (handle->frame_idx ==  T2MI_FRAME_IDX_MAX ? 0 : handle->superframe_idx);

    // check state of all bb payload


END_PACKET:

    status->packet_index = handle->packet_index;
    status->num_bits = dvbt2_T2MIPacketToBuff(handle, handle->plp_index, buff);
    handle->packet_count++;
    handle->packet_count = (handle->packet_count > 0xFF ? 0: handle->packet_count);
    handle->packet_index++;
    handle->packet_index = (handle->packet_index ==  handle->PLP_NUM_BLOCKS_TOTAL + 2 ? 0 : handle->packet_index);

    // write
    if (handle->file_muxed_t2 != NULL)
        fwrite(buff, 1, status->num_bits/8, handle->file_muxed_t2);

    return 0;

}



int dvbt2_GetL1PreLenInBits()
{
        return 0;
}





//int CDVBT2_MultiplePLP::SearchingParamater(int bandwidth, int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int *plp_bitrate, int *num_data_symbol, int *plp_num_block)
int dvbt2_SearchingParamater( int num_aux, int num_rf, int modulation,
        int bandwidth, int bw_ext, int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int *plp_bitrate, int *num_data_symbol, int *plp_num_block)
{


        int _fft_size = fft_size;
        int _guard_interval = guard_interval;
        int _pilot = pilot;
        int _num_plp = num_plp;
// the function is obsoleted 
#if 0
        int lf = t2frame_Lf(_fft_size, _guard_interval);
#endif

        int d_data  = t2frame_data_symbol_num_cell(_fft_size, _pilot, bw_ext); // number of cell for each data symbol
        //int s1

        int l_p2 = t2frame_p2_num_symbol(fft_size);
        int c_p2 = t2frame_p2_symbol_num_cell(fft_size, 0 /* SISO mode */);

        //int l_data =  lf - l_p2;

        int total_np2_cell = c_p2*l_p2;
        //int t2_frame_cell = total_np2_cell + l_data*d_data;

        int _num_data_symbol = t2frame_num_data_symbol_min(fft_size, guard_interval);

        int l1_post_info_size_in_bits = t2frame_l1pos_dynamic_current(num_plp, num_aux) +   t2frame_l1pos_conflen(num_plp, num_aux, num_rf, fft_size);

        int l1_pos_num_cell = TL_Calculate_L1_Post_Size(modulation, fft_size, l1_post_info_size_in_bits);
		int is_no_found = 1; 
		double _t2_duration;
		int _t2_frame_plp_total_data_cell;
        int i;
		int _plp_num_cell;
		int num_estimated_c_tot;

        do {

				if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)
				{
					_num_data_symbol++; // refer to 8.3.1, 302755 version 1.2.1
					//char tmp[100];
					//sprintf(tmp, "Num data symbol: %d \n", _num_data_symbol);
					//OutputDebugString(tmp);
				}
				
                _t2_duration = t2frame_duration(_fft_size, _guard_interval, _num_data_symbol, bandwidth);
				if (_t2_duration > 250)
					break;
                if (_t2_duration != -1.0)
                {


                        int c_tot = 0;
                        if (t2frame_is_closing_symbol(_fft_size, _guard_interval, _pilot))
                        {
                                int l_normal = _num_data_symbol -1;
                                int total_data_cell = d_data*l_normal;
                                total_data_cell += t2frame_data_symbol_closing_frame_num_cell(_fft_size, _pilot, bw_ext);
                                c_tot = total_np2_cell + total_data_cell;
                        }
                        else
                        {
                                int total_data_cell = d_data*_num_data_symbol;
                                c_tot = total_np2_cell + total_data_cell;
                        }

                        _t2_frame_plp_total_data_cell = 0;

                        for (i = 0; i < _num_plp; i++)
                        {
                                int _plp_bitrate = *(plp_bitrate + i);
                                int _plp_fec_type  = *(plp_fec_type + i);
                                int _plp_cod  = *(plp_cod + i);
                                int _plp_modulation = *(plp_mod + i);

                                int _plp_bit_per_t2 = (int)((_t2_duration/1000.0)*_plp_bitrate);
                                int _dfl_max_in_bits = t2frame_kbch_size(_plp_fec_type, _plp_cod) - 80;

                                int _plp_num_block = (_plp_bit_per_t2 - 1)/_dfl_max_in_bits + 2 /*tricky 2*/;
                                *(plp_num_block + i) = _plp_num_block;

                                _plp_num_cell = t2frame_fecframe_num_cell(_plp_fec_type, _plp_modulation);
                                _t2_frame_plp_total_data_cell = _t2_frame_plp_total_data_cell + _plp_num_cell*_plp_num_block;
                        }

                        num_estimated_c_tot = _t2_frame_plp_total_data_cell + t2frame_l1pre_num_cell() +  l1_pos_num_cell;

                        if ( num_estimated_c_tot  < c_tot  )
						{
								is_no_found = 0;
                                break;
						}
                }
				// look for next valid number of data symbol 
                _num_data_symbol++;

        }while (1);


        if (is_no_found == 0)
        {
                *num_data_symbol = _num_data_symbol;
                // search successfully and guarantee the bitrate
                return 0;
        }
        else{
                int i;
                for (i = 0; i < _num_plp; i++)
                {
                        *(plp_bitrate + i) = (int)(*(plp_bitrate + i)*0.9);
                }
                dvbt2_SearchingParamater(num_aux, num_rf, modulation, bandwidth, bw_ext, fft_size, guard_interval, pilot, num_plp, plp_fec_type, plp_cod, plp_mod, plp_bitrate, num_data_symbol, plp_num_block);

                return 0;
        }

}


#if 0 // have not support

int dvbt2_ParameterValidation(dvbt2_mux_t *handle, int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int num_data_symbol, int *plp_num_block)
{
        return 0;
#if 0
#define DVBT2_VALID_PARAMETER 0x00
#define DVBT2_INVALID_FFT_SIZE 0x01
#define DVBT2_INVALID_GUAR_INTERVAL 0x02
#define DVBT2_INVALID_NUM_DATA_SYMBOL 0x04
#define DVBT2_INVALID_PLP_MOD 0x08
#define DVBT2_INVALID_PLP_COD 0x10
#define DVBT2_INVALID_PLP_FEC 0x20
#define DVBT2_INVALID_PLP_NUM_BLOCK 0x40


        int _fft_size = fft_size;
        int _guard_interval = guard_interval;
        int _pilot = pilot;
        int _num_plp = num_plp;
        int _num_data_symbol = num_data_symbol;

//	int _num_data_symbol_max = t2frame_num_data_symbol_max(_fft_size, _guard_interval);
        //int bandwidth = dvbt2_input_param.BW;
        double _t2_duration = 0; //t2frame_duration(_fft_size, _guard_interval, _num_data_symbol, bandwidth);

        if (_t2_duration == -1.0)
        {
                // wrong fft_size, _guard_interval, num_data_symbol
                return DVBT2_INVALID_FFT_SIZE | DVBT2_INVALID_GUAR_INTERVAL | DVBT2_INVALID_NUM_DATA_SYMBOL;
        }else
        {
                int _data_symbol_num_cell  = t2frame_data_symbol_num_cell(_fft_size, _pilot);
                int _t2_frame_data_cell = t2frame_l1_signal_cell(_fft_size, 0) + _data_symbol_num_cell*_num_data_symbol;
                int _t2_frame_plp_total_data_cell = 0;
                int l1_post_info_size_in_bits = t2frame_l1pos_dynamic_current(num_plp, num_aux) +   t2frame_l1pos_conflen(num_plp, num_aux, num_rf, fft_size);

                int l1_pos_num_cell = TL_Calculate_L1_Post_Size(modulation, fft_size, l1_post_info_size_in_bits);

                for (int i = 0; i < _num_plp; i++)
                {
                        int _plp_fec_type  = *(plp_fec_type + i);
                        //int _plp_cod  = *(plp_cod + i);
                        int _plp_modulation = *(plp_mod + i);
                        int _plp_num_block = *(plp_num_block + i);

                        int _plp_num_cell = t2frame_fecframe_num_cell(_plp_fec_type, _plp_modulation);
                        _t2_frame_plp_total_data_cell = _t2_frame_plp_total_data_cell + _plp_num_cell*_plp_num_block;
                }

                        if (_t2_frame_plp_total_data_cell + l1_pos_num_cell +  t2frame_l1pre_num_cell() < _t2_frame_data_cell)
                        return DVBT2_VALID_PARAMETER;
                else
                        return DVBT2_INVALID_NUM_DATA_SYMBOL | DVBT2_INVALID_PLP_MOD | DVBT2_INVALID_PLP_COD | DVBT2_INVALID_PLP_FEC | DVBT2_INVALID_PLP_NUM_BLOCK;
        }
#endif
}

#endif




int dvbt2_ValidateBandWidth(t2mi_handle_t *handle, int fft_size){

        // step 1: compute number of data cell for Cdata
        //         and number of data cell of remaining P2 (after allocating cell to L1-post signal)


        // step 2: compute number of cell need for all PLP



#if 0
        int pilot_pattern = payload_l1.l1_payload.l1pre._PILOT_PATTERN;
        int max_data_cell = t2frame_max_num_datacell(fft_size, pilot_pattern);



        for (int i = 0; i < payload_l1.l1_payload.l1conf._NUM_PLP; i++)
        {
                l1_payload.l1dyn_curr.plp_list[i]._PLP_ID = _t2mi_multi_plp.list_plp[i].PLP_ID;

                // setting PLP_START
                {
                if (i == 0)
                {
                        PLP_START = 0;
                        //PLP_START = 8100*14;
                        l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;
                }
                else {
                        //PLP_START = 0;

                        int PLP_MOD_M = l1_payload.l1conf._plp_list[i-1]._PLP_MOD;
                        int PLP_FEC_TYPE_M = l1_payload.l1conf._plp_list[i-1]._PLP_FEC_TYPE;
                        int PLP_NUM_BLOCKS_PRE = l1_payload.l1dyn_curr.plp_list[i-1]._PLP_NUM_BLOCKS;
                        int N_mod;

                        if ( PLP_MOD_M == 0x000 )		N_mod = 2;
                        else if ( PLP_MOD_M == 0x001 )	N_mod = 4;
                        else if ( PLP_MOD_M == 0x002 )	N_mod = 6;
                        else if ( PLP_MOD_M == 0x003 )	N_mod = 8;

                        int N_ldpc = (PLP_FEC_TYPE_M == 0x001 ?  64800 : 16200);

                        PLP_START += ((N_ldpc*(PLP_NUM_BLOCKS_PRE)) / N_mod);
                        l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;

                }
#endif

        return 1;

}

