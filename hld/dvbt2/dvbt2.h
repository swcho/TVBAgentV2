//	Teleview Corporation
// huy@teleview.com

#ifndef DVBT2_H_
#define DVBT2_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#ifdef __cplusplus
extern "C"
{
#endif

    typedef void *t2mi_handle_ptr;

    
// DVB-T2 interface

typedef struct DVBT2_PLP{
        char file_path[255];
        int file_bitrate;

        unsigned int PLP_ID;
        unsigned int group_id;
        unsigned int type;

        unsigned int PLP_TYPE;
        unsigned int PLP_MOD;
        unsigned int PLP_COD;
        unsigned int PLP_FEC;
        unsigned int PLP_HEM; // normal or hight efficient mode
        unsigned int PLP_NUM_BLOCKS; // number of FEC Block for corresponding interleaving frame
            unsigned int UP_BITRATE;
        unsigned int PLP_ROTATION; // 0: no rotation; 1: rotation
        unsigned char ISSY;
        unsigned char PLP_TIME_IL_TYPE;
        unsigned char PLP_TIME_IL_LENGTH;
        // 0: no using ISSY (default)
        // 1: short
        // 2: long

}DVBT2_PLP_t;

typedef struct DVBT2_PARAM{
        // general information
        unsigned int BW;
        unsigned int BW_EXT;
        unsigned int FFT_SIZE;
        unsigned int GUARD_INTERVAL;
//	unsigned int FEC_TYPE;
        unsigned int L1_MOD;
        unsigned int L1_COD;
        unsigned int Pilot;
        unsigned int PAPR;
//	unsigned int BANDWIDTH_EXT; // (0 or 1)
        unsigned int L1_REPETITION; // ( 0 or 1)
        unsigned int L1x_LEN; // not use
        unsigned int NETWORK_ID;
        unsigned int T2_ID; //
        unsigned int Cell_ID;
        unsigned int S1; // 0: SISO, 1: MISO

        // frame information
        unsigned int NUM_T2_FRAME;
        unsigned int NUM_DATA_SYMBOLS;
        unsigned int t2_bitrate;
        unsigned int l1_pre_cells;
        unsigned int l1_pos_cells;
        unsigned int c_tot_cells; // number of active cell
        unsigned int plps_cells; // number of cells are allocated to plps
        unsigned int dummy_cells;
        //unsgned int
        unsigned int t2_duration; // duration in milisecond

        // PLP information
        int num_plp;
        DVBT2_PLP_t list_plp[10];
        // frequency of output
        unsigned int FREQUENCY;
        int PID;


        // for FEF frame
        char fef_type;
        int fef_length;
        unsigned char fef_interval;


        //
        int auto_searching_data_symbol_num_block;
}DVBT2_PARAM_t;

// for adaptation with output (ts)
struct t2mi_packet_status{
        int packet_type;
        int num_bits;
        int packet_index;
};




    /**
    \ingroup typedef
    */

    // callback function
    typedef int (*read_one_packet_t)(void *owner, int child_index, char *buff, void *_param );




    //dvbt2_mux_t* dvbt2_init(struct DVBT2_PARAM _t2mi_multiple_plp);
    /*!
    \brief initialization
    \param _callback_func
    \param _callback_params
    \return a handler
    \ingroup api
    */

    t2mi_handle_ptr t2mi_init(read_one_packet_t _callback_func, void *_callback_params);

    /*!
    \brief quit
    \param handle
    \return none
    \ingroup api
    */
    int t2mi_quit(t2mi_handle_ptr handle);

    /*!
    \brief get current parameters
    \param handle
    \param param
    \return none
    \ingroup api
    */
    void t2mi_get_param(t2mi_handle_ptr handle, struct DVBT2_PARAM *param);

    /*!
    \brief set parameters
    \param handle
    \param param
    \return none
    \ingroup api
    */
    int t2mi_set_param(t2mi_handle_ptr handle, struct DVBT2_PARAM *param);

    /*!
    \brief set callback param
    \param handle
    \param param
    \return none
    \ingroup api
    */

    int t2mi_set_callback_param(t2mi_handle_ptr handle, void *_callback_param);

    /*!
    \brief get a t2mi packet
    \param handle
    \param status
    \param buff
    \return none
    \ingroup api
    */
    int t2mi_get_t2_packet(t2mi_handle_ptr handle, struct t2mi_packet_status *status, unsigned char *buff);


    int dvbt2_SearchingParamater( int num_aux, int num_rf, int modulation,
            int bandwidth, int bw_ext, int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int *plp_bitrate, int *num_data_symbol, int *plp_num_block);




#ifdef _WIN32

#define strcasecmp _stricmp

#else
#include <strings.h>
#endif






// support loop & no loop mode
#define DVBT2_PLAY_NO_LOOP 0
#define DVBT2_PLAY_LOOP 1




#define MULTI_PLP_STATE_NO_EXIST 0
#define MULTI_PLP_STATE_INIT 1
#define MULTI_PLP_STATE_OPEN 2
#define MULTI_PLP_STATE_CLOSE 3





#define PLAYBACK_TS_PACKET_SIZE_MAX 300
// conform to (A122, page 26)
#define DVBT2_PLP_MODE_NORMAL 0  // normal mode
#define DVBT2_PLP_MODE_HEM 1  // high efficient mode


#define DVBT2_PLP_ISSY_OFF 0
#define DVBT2_PLP_ISSY_ON 1



//////////////////////////////////////////////////////////
// Frequencies information
#define T2MI_FREQ_NUM_RF 1 // number of RF
#define T2MI_FREQ_RF_IDX_STAR 0 //


// BBFRAME

#define BBFRAME_K_BCH_MAX  (53840 + 100 /* alignment */)
#define BBFRAME_USER_DATA_SIZE_MAX  (BBFRAME_K_BCH_MAX - 1)/8 + 1

// T2MI Packet
#define T2MI_PACKET_HEADER_SIZE 48 // conform to A136R2
#define T2MI_PACKET_TYPE_BBF 0x00 // baseband frame
#define T2MI_PACKET_TYPE_AUX 0x01 // auxiliary I/O data
#define T2MI_PACKET_TYPE_L1CURRENT 0X10 // L1 current data
#define T2MI_PACKET_TYPE_TIMESTAMP 0x20 // timestamp

#define T2MI_FRAME_IDX_MAX 0xFF
#define T2MI_SUPERFRAME_IDX_MAX 0xFF

// see ETSI TS 102 773 V1.3.1
#define DVBT2_BW_1_7 0
#define DVBT2_BW_5 1
#define DVBT2_BW_6 2
#define DVBT2_BW_7 3
#define DVBT2_BW_8 4

#define DVBT2_BWT_NORMAL 0 // (0)
#define DVBT2_BWT_EXT 1 // (1)



#define DVBT2_GUARD_INTERVAL_1_32   0  // (000)
#define DVBT2_GUARD_INTERVAL_1_16   1  // (001)
#define DVBT2_GUARD_INTERVAL_1_8    2  // (010)
#define DVBT2_GUARD_INTERVAL_1_4    3  // (011)
#define DVBT2_GUARD_INTERVAL_1_128  4  // (100)
#define DVBT2_GUARD_INTERVAL_19_128 5  // (101)
#define DVBT2_GUARD_INTERVAL_19_256 6  // (110)
#define DVBT2_GUARD_INTERVAL_RESERVER 7  // (111)
//#define DVBT2_GUARD_INTERVAL_SELECTED   //

/*
#define DVBT2_PAPR_NO   0x0 //
#define DVBT2_PAPR_ACE   0x1 //
#define DVBT2_PAPR_TR   0x2 //
#define DVBT2_PAPR_ACE_TR   0x3 //
#define DVBT2_PAPR_SELECTED DVBT2_PAPR_NO
*/

#define DVBT2_MOD_BPSK 0x0
#define DVBT2_MOD_QPSK 0x1
#define DVBT2_MOD_QAM16 0x2
#define DVBT2_MOD_QAM64 0x3

/*
#define DVBT2_COD_1_2 0 // (00)
#define DVBT2_COD_SELECTED DVBT2_COD_1_2

#define DVBT2_FEC_TYPE_LDPC16K 0 // (00)
#define DVBT2_FEC_TYPE_SELECTED DVBT2_FEC_TYPE_LDPC16K
*/

#define DVBT2_PILOT_PATTERN_PP1 0X0
#define DVBT2_PILOT_PATTERN_PP2 0X1
#define DVBT2_PILOT_PATTERN_PP3 0X2
#define DVBT2_PILOT_PATTERN_PP4 0X3
#define DVBT2_PILOT_PATTERN_PP5 0X4
#define DVBT2_PILOT_PATTERN_PP6 0X5
#define DVBT2_PILOT_PATTERN_PP7 0X6
#define DVBT2_PILOT_PATTERN_PP8 0X7
//#define DVBT2_PILOT_PATTERN_SELECTED


#define DVBT2_PLP_TYPE_COMMON 0 // (000)
#define DVBT2_PLP_TYPE_DATATYPE1 1 // (001)
#define DVBT2_PLP_TYPE_DATATYPE2 2 // (010)

#define DVBT2_PLP_PAYLOAD_TYPE_GFPS 0 // (00000)
#define DVBT2_PLP_PAYLOAD_TYPE_GCS 1 // (00000)
#define DVBT2_PLP_PAYLOAD_TYPE_GSE 2 // (00000)
#define DVBT2_PLP_PAYLOAD_TYPE_TS 3 // (00000)

#define DVBT2_PLP_COD_1_2 0 // 000
#define DVBT2_PLP_COD_3_5 1 // 001
#define DVBT2_PLP_COD_2_3 2 // 010
#define DVBT2_PLP_COD_3_4 3 // 011
#define DVBT2_PLP_COD_4_5 4 // 100
#define DVBT2_PLP_COD_5_6 5 // 101

#define DVBT2_PLP_MOD_QPSK 0 // 000
#define DVBT2_PLP_MOD_16QAM 1 // 000
#define DVBT2_PLP_MOD_64QAM 2 // 000
#define DVBT2_PLP_MOD_256QAM 3 // 000

#define DVBT2_PLP_FEC_TYPE_16K_LDPC 0 // 00
#define DVBT2_PLP_FEC_TYPE_64K_LDPC 1 // 01

#define DVBT2_FFT_SIZE_32K   5    // 0101
#define DVBT2_FFT_SIZE_32K_E 7  //0111
#define DVBT2_FFT_SIZE_16K   4  // 0100
#define DVBT2_FFT_SIZE_8K    1  // 0001
#define DVBT2_FFT_SIZE_8K_E  6  // 0110
#define DVBT2_FFT_SIZE_4K    2  // 0010
#define DVBT2_FFT_SIZE_2K    0  // 0000
#define DVBT2_FFT_SIZE_1K    3  // 0011

#define DVBT2_PLP_ROT_OFF 0
#define DVBT2_PLP_ROT_ON 1


#define DVBT2_BW_1_7 0
#define DVBT2_BW_5 1
#define DVBT2_BW_6 2
#define DVBT2_BW_7 3
#define DVBT2_BW_8 4

static char dvbt2_bandwidth_validate(char *text)
{

    if (!strcasecmp(text, "8") ||
        !strcasecmp(text, "7") ||
        !strcasecmp(text, "6") ||
        !strcasecmp(text, "5") ||
        !strcasecmp(text, "1.7") )
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_bandwidth_to_num(char *text)
{

    if (!strcasecmp(text, "8"))
        return DVBT2_BW_8;
    else if (!strcasecmp(text, "7"))
        return DVBT2_BW_7;
    else if (!strcasecmp(text, "6"))
        return DVBT2_BW_6;
    else if (!strcasecmp(text, "5"))
        return DVBT2_BW_5;
    else if (!strcasecmp(text, "1.7"))
        return DVBT2_BW_1_7;
    else{
        printf("[Warning] default bandwidth: 8 \n");
        return DVBT2_BW_8;    }
}


static int dvbt2_conv_bandwidth_to_text(unsigned char bw, char *text)
{
    if (bw == DVBT2_BW_8)
    {
        strcpy(text, "8");
        return 0;
    }else if (bw == DVBT2_BW_7)
    {
        strcpy(text, "7");
        return 0;
    }else if (bw == DVBT2_BW_6)
    {
        strcpy(text, "6");
        return 0;
    }else if (bw == DVBT2_BW_5)
    {
        strcpy(text, "5");
        return 0;
    }else if (bw == DVBT2_BW_1_7)
    {
        strcpy(text, "1.7");
        return 0;
    }else{
        strcpy(text, "unknown");
        return -1;
    }


}


static char dvbt2_fft_validate(char *fft)
{
    if (!strcasecmp(fft, "32k") ||
        !strcasecmp(fft, "16k") ||
        !strcasecmp(fft, "8k") ||
        !strcasecmp(fft, "4k") ||
        !strcasecmp(fft, "2k") ||
        !strcasecmp(fft, "1k") )
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_fft_to_num(char *fft)
{
    if (!strcasecmp(fft, "32k"))
        return DVBT2_FFT_SIZE_32K;
    else if (!strcasecmp(fft, "16k"))
        return DVBT2_FFT_SIZE_16K;
    else if (!strcasecmp(fft, "8k"))
        return DVBT2_FFT_SIZE_8K;
    else if (!strcasecmp(fft, "4k"))
        return DVBT2_FFT_SIZE_4K;
    else if (!strcasecmp(fft, "2k"))
        return DVBT2_FFT_SIZE_2K;
    else if (!strcasecmp(fft, "1k"))
        return DVBT2_FFT_SIZE_1K;
    else{
        printf("[Warning] set default fft size: 8K \n");
        return DVBT2_FFT_SIZE_8K;

    }
}

static int dvbt2_conv_fft_to_text(unsigned char fft_size, char *text)
{
    if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)
    {
        strcpy(text, "32K");
        return 0;
    }else if (fft_size == DVBT2_FFT_SIZE_16K)
    {
        strcpy(text, "16K");
        return 0;
    }else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
    {
        strcpy(text, "8K");
        return 0;
    }else if (fft_size == DVBT2_FFT_SIZE_4K)
    {
        strcpy(text, "4K");
        return 0;
    }
    else if (fft_size == DVBT2_FFT_SIZE_2K)
    {
        strcpy(text, "2K");
        return 0;
    }
    else if (fft_size == DVBT2_FFT_SIZE_1K)
    {
        strcpy(text, "1K");
        return 0;
    }
    else{
        strcpy(text, "unknown");
        return -1;
    }
}

static char dvbt2_guard_validate(char *gi)
{
    if (!strcasecmp(gi, "1/128") ||
        !strcasecmp(gi, "1/32") ||
        !strcasecmp(gi, "1/16") ||
        !strcasecmp(gi, "1/8") ||
        !strcasecmp(gi, "1/4") ||
        !strcasecmp(gi, "19/128") ||
        !strcasecmp(gi, "19/256"))
        return 1;
    else
        return -1;
}

static unsigned char dvbt2_conv_guard_to_num(char *gi)
{
    if (!strcasecmp(gi, "1/128"))
        return DVBT2_GUARD_INTERVAL_1_128;
    else if (!strcasecmp(gi, "1/32"))
        return DVBT2_GUARD_INTERVAL_1_32;
    else if (!strcasecmp(gi, "1/16"))
        return DVBT2_GUARD_INTERVAL_1_16;
    else if (!strcasecmp(gi, "1/8"))
        return DVBT2_GUARD_INTERVAL_1_8;
    else if (!strcasecmp(gi, "1/4"))
        return DVBT2_GUARD_INTERVAL_1_4;
    else if (!strcasecmp(gi, "19/128"))
        return DVBT2_GUARD_INTERVAL_19_128;
    else if (!strcasecmp(gi, "19/256"))
        return DVBT2_GUARD_INTERVAL_19_256;
    else
    {
        printf("[Warning] set default guard interval: 1/128 \n");
        return DVBT2_GUARD_INTERVAL_1_128;
    }
}

static int dvbt2_conv_guard_to_text(unsigned char gi, char *text)
{

    if (gi == DVBT2_GUARD_INTERVAL_1_128)
    {
        strcpy(text, "1/128");
        return 0;
    }else if (gi == DVBT2_GUARD_INTERVAL_1_32)
    {
        strcpy(text, "1/32");
        return 0;
    }
    else if (gi == DVBT2_GUARD_INTERVAL_1_16)
    {
        strcpy(text, "1/16");
        return 0;
    }else if (gi == DVBT2_GUARD_INTERVAL_1_8)
    {
        strcpy(text, "1/8");
        return 0;
    }else if (gi == DVBT2_GUARD_INTERVAL_1_4)
    {
        strcpy(text, "1/4");
        return 0;
    }else if (gi == DVBT2_GUARD_INTERVAL_19_128)
    {
        strcpy(text, "19/128");
        return 0;
    }else if (gi == DVBT2_GUARD_INTERVAL_19_256)
    {
        strcpy(text, "19/256");
        return 0;
    }  else
    {
        strcpy(text, "unknown");
        return -1;
    }
}

static unsigned char dvbt2_conv_bwext_to_num(char *text)
{
    if (!strcasecmp(text, "on"))
        return DVBT2_BWT_EXT;
    else if (!strcasecmp(text, "off"))
        return DVBT2_BWT_NORMAL;
    else
    {
        printf("[Warning] set default bandwidth extension: normal \n");
        return DVBT2_BWT_NORMAL;
    }
}

static int dvbt2_conv_bwext_to_text(unsigned char bwext, char  *text)
{
    if (bwext == DVBT2_BWT_EXT)
    {
        strcpy(text, "on");
        return 0;
    } else if (bwext == DVBT2_BWT_NORMAL)
    {
        strcpy(text, "off");
        return 0;
    }
    else
    {
        strcpy(text, "unknown");
        return -1;
    }
}

static char dvbt2_pilot_validate(char *text)
{
    if (!strcasecmp(text, "PP1") ||
        !strcasecmp(text, "PP2") ||
        !strcasecmp(text, "PP3") ||
        !strcasecmp(text, "PP4") ||
        !strcasecmp(text, "PP5") ||
        !strcasecmp(text, "PP6") ||
        !strcasecmp(text, "PP7") ||
        !strcasecmp(text, "PP8") )
        return 1;
    else
        return -1;

}

static unsigned char dvbt2_conv_pilot_to_num(char *text)
{
    if (!strcasecmp(text, "PP1"))
        return DVBT2_PILOT_PATTERN_PP1;
    else if (!strcasecmp(text, "PP2"))
        return DVBT2_PILOT_PATTERN_PP2;
    else if (!strcasecmp(text, "PP3"))
        return DVBT2_PILOT_PATTERN_PP3;
    else if (!strcasecmp(text, "PP4"))
        return DVBT2_PILOT_PATTERN_PP4;
    else if (!strcasecmp(text, "PP5"))
        return DVBT2_PILOT_PATTERN_PP5;
    else if (!strcasecmp(text, "PP6"))
        return DVBT2_PILOT_PATTERN_PP6;
    else if (!strcasecmp(text, "PP7"))
        return DVBT2_PILOT_PATTERN_PP7;
    else if (!strcasecmp(text, "PP8"))
        return DVBT2_PILOT_PATTERN_PP8;
    else
    {
        printf("[Warning] set default pillot: PP7 \n");
        return DVBT2_PILOT_PATTERN_PP7;
    }
}

static int dvbt2_conv_pilot_to_text(unsigned char pilot, char *text)
{
    if (pilot == DVBT2_PILOT_PATTERN_PP1)
    {
        strcpy(text, "PP1");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP2)
    {
        strcpy(text, "PP2");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP3)
    {
        strcpy(text, "PP3");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP4)
    {
        strcpy(text, "PP4");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP5)
    {
        strcpy(text, "PP5");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP6)
    {
        strcpy(text, "PP6");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP7)
    {
        strcpy(text, "PP7");
        return 0;
    }
    else if (pilot == DVBT2_PILOT_PATTERN_PP8)
    {
        strcpy(text, "PP8");
        return 0;
    }
    else
    {
        strcpy(text, "unknown");
        return -1;
    }
}

static char dvbt2_l1modulation_validate(char *text)
{
    if (!strcasecmp(text, "BPSK") ||
        !strcasecmp(text, "QPSK") ||
        !strcasecmp(text, "16QAM") || !strcasecmp(text, "QAM16") ||
        !strcasecmp(text, "64QAM") || !strcasecmp(text, "QAM64"))
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_l1modulation_to_num(char *text)
{
    if (!strcasecmp(text, "BPSK"))
        return DVBT2_MOD_BPSK;
    else if (!strcasecmp(text, "QPSK"))
        return DVBT2_MOD_QPSK;
    else if (!strcasecmp(text, "16QAM") || !strcasecmp(text, "QAM16"))
        return DVBT2_MOD_QAM16;
    else if (!strcasecmp(text, "64QAM") || !strcasecmp(text, "QAM64"))
        return DVBT2_MOD_QAM64;
    else {
        printf("[Warning]l1 modulation default: QPSK \n");
        return DVBT2_MOD_QPSK;
    }
}

static int dvbt2_conv_l1modulation_to_text(unsigned char l1_mod, char *text)
{
    if (l1_mod == DVBT2_MOD_BPSK)
    {
        strcpy(text, "BPSK");
        return 0;
    }
    else if (l1_mod == DVBT2_MOD_QPSK)
    {
        strcpy(text, "QPSK");
        return 0;
    }
    else if (l1_mod == DVBT2_MOD_QAM16)
    {
        strcpy(text, "QAM16");
        return 0;
    }
    else if (l1_mod == DVBT2_MOD_QAM64)
    {
        strcpy(text, "QAM64");
        return 0;
    }
    else {
        strcpy(text, "unknown");
        return -1;
    }
}

static char dvbt2_plp_mod_validate(char *text)
{
    if (!strcasecmp(text, "QPSK") ||
        !strcasecmp(text, "16QAM") || !strcasecmp(text, "QAM16") ||
        !strcasecmp(text, "64QAM") || !strcasecmp(text, "QAM64") ||
        !strcasecmp(text, "256QAM") || !strcasecmp(text, "QAM256") )
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_plp_mod_to_num(char *text)
{
    if (!strcasecmp(text, "QPSK"))
        return DVBT2_PLP_MOD_QPSK;
    else if (!strcasecmp(text, "16QAM") || !strcasecmp(text, "QAM16"))
        return DVBT2_PLP_MOD_16QAM;
    else if (!strcasecmp(text, "64QAM") || !strcasecmp(text, "QAM64"))
        return DVBT2_PLP_MOD_64QAM;
    else if (!strcasecmp(text, "256QAM") || !strcasecmp(text, "QAM256"))
        return DVBT2_PLP_MOD_256QAM;
    else {
        printf("[Warning]l1 modulation default: 64QAM \n");
        return DVBT2_PLP_MOD_64QAM;
    }

}

static int dvbt2_conv_plp_mod_to_text(unsigned char plp_mod, char *text)
{
    if (plp_mod == DVBT2_PLP_MOD_QPSK)
    {
        //strc
        strcpy(text, "QPSK");
        return 0;
    }
    else if (plp_mod == DVBT2_PLP_MOD_16QAM)
    {
        strcpy(text, "16QAM");
        return 0;
    }
    else if (plp_mod == DVBT2_PLP_MOD_64QAM)
    {
        strcpy(text, "64QAM");
        return 0;
    }
    else if ( plp_mod ==  DVBT2_PLP_MOD_256QAM)
    {
        strcpy(text, "256QAM");
        return 0;
    }
    else {
        strcpy(text, "unknown");
        return 0;
    }

}

static char dvbt2_plp_coderate_validate(char *text)
{
    if (!strcasecmp(text, "1/2") ||
        !strcasecmp(text, "2/3") ||
        !strcasecmp(text, "3/4") ||
        !strcasecmp(text, "3/5") ||
        !strcasecmp(text, "4/5") ||
        !strcasecmp(text, "5/6") )
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_plp_coderate_to_num(char *text)
{
    if (!strcasecmp(text, "1/2"))
        return DVBT2_PLP_COD_1_2;
    else if (!strcasecmp(text, "2/3"))
        return DVBT2_PLP_COD_2_3;
    else if (!strcasecmp(text, "3/4"))
        return DVBT2_PLP_COD_3_4;
    else if (!strcasecmp(text, "3/5"))
        return DVBT2_PLP_COD_3_5;
    else if (!strcasecmp(text, "4/5"))
        return DVBT2_PLP_COD_4_5;
    else if (!strcasecmp(text, "5/6"))
        return DVBT2_PLP_COD_5_6;
    else
    {
        printf("[Warning] coderate default: 3/4 \n");
        return DVBT2_PLP_COD_3_4;
    }
}


static int dvbt2_conv_plp_coderate_to_text(unsigned char plp_code, char *text)
{
    if (plp_code == DVBT2_PLP_COD_1_2)
    {
         strcpy(text, "1/2");
         return 0;
    }
    else if (plp_code == DVBT2_PLP_COD_2_3)
    {
        strcpy(text, "2/3");
        return 0;

    }
    else if (plp_code == DVBT2_PLP_COD_3_4)
    {
        strcpy(text, "3/4");
        return 0;
    }
    else if (plp_code == DVBT2_PLP_COD_3_5)
    {
        strcpy(text, "3/5");
        return 0;
    }
    else if (plp_code == DVBT2_PLP_COD_4_5)
    {
        strcpy(text, "4/5");
        return 0;
    }
    else if (plp_code == DVBT2_PLP_COD_5_6)
    {
        strcpy(text, "5/6");
        return 0;
    }
    else
    {
        printf("[Warning] coderate default: 3/4 \n");
        strcpy(text, "3/4");
        return 0;
    }
}

static char dvbt2_plp_fec_validate(char *text)
{
    if (!strcasecmp(text, "16200") ||
        !strcasecmp(text, "64800"))
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_plp_fec_to_num(char *text)
{
    if (!strcasecmp(text, "16200"))
        return DVBT2_PLP_FEC_TYPE_16K_LDPC;
    else if (!strcasecmp(text, "64800"))
        return DVBT2_PLP_FEC_TYPE_64K_LDPC;
    else
    {
        printf("[Warning] fec default: 64K \n");
        return DVBT2_PLP_FEC_TYPE_64K_LDPC;
    }
}

static int dvbt2_conv_plp_fec_to_text(unsigned char plp_fec, char *text)
{
    if (plp_fec == DVBT2_PLP_FEC_TYPE_16K_LDPC)
    {
        strcpy(text, "16200");
        return 0;
    }
    else if (plp_fec == DVBT2_PLP_FEC_TYPE_64K_LDPC)
    {
        strcpy(text, "64800");
        return 0;
    }
    else
    {
        printf("[Warning] fec default: 64K \n");
        strcpy(text, "64800");
        return 0;
    }
}

static char dvbt2_plp_rot_validate(char *text)
{
    if (!strcasecmp(text, "on") ||
        !strcasecmp(text, "off"))
        return 1;
    else
        return -1;
}


static unsigned char dvbt2_conv_plp_rot_to_num(char *text)
{
    if (!strcasecmp(text, "on"))
        return DVBT2_PLP_ROT_ON;
    else if (!strcasecmp(text, "off"))
        return DVBT2_PLP_ROT_OFF;
    else
    {
        printf("[Warning] rotation default: off \n");
        return DVBT2_PLP_ROT_OFF;
    }
}

static int dvbt2_conv_plp_rot_to_text(unsigned char plp_rot,  char *text)
{
    if (plp_rot == DVBT2_PLP_ROT_ON)
    {
        strcpy(text, "on");
        return 0;
    } else if (plp_rot == DVBT2_PLP_ROT_OFF)
    {
        strcpy(text, "off");
        return 0;
    }
    else
    {
        strcpy(text, "off");
        return -1;
    }
}

static char dvbt2_plp_hem_validate(char *text)
{
    if (!strcasecmp(text, "on") ||
        !strcasecmp(text, "off"))
        return 1;
    else
        return -1;
}



static unsigned char dvbt2_conv_plp_hem_to_num(char *text)
{
    if (!strcasecmp(text, "on"))
        return DVBT2_PLP_MODE_HEM;
    else if (!strcasecmp(text, "off"))
        return DVBT2_PLP_MODE_NORMAL;
    else
    {
        printf("[Warning] high efficient mode default: off \n");
        return DVBT2_PLP_MODE_HEM;
    }
}


static int dvbt2_conv_plp_hem_to_text(unsigned char plp_hem, char *text)
{
    if (plp_hem == DVBT2_PLP_MODE_HEM) // (!strcasecmp(text, "on"))
    {
        strcpy(text, "on");
        return 0;
    }
    else if (plp_hem == DVBT2_PLP_MODE_NORMAL)
    {
        strcpy(text, "off");
        return 0;
    }
    else
    {
        printf("[Warning] high efficient mode default: off \n");
        strcpy(text, "off");
        return -1;
    }
}


static unsigned char dvbt2_conv_plp_issy_to_num(char *text)
{
    if (!strcasecmp(text, "on"))
        return DVBT2_PLP_ISSY_ON;
    else if (!strcasecmp(text, "off"))
        return DVBT2_PLP_ISSY_OFF;
    else
    {
        //printf("[Warning] ISSY default: off \n");
        return DVBT2_PLP_ISSY_OFF;
    }
}

static int dvbt2_conv_plp_issy_to_text(unsigned char plp_issy, char *text)
{
    if (plp_issy == DVBT2_PLP_ISSY_ON)
    {
        strcpy(text, "on");
        return 0;
    }
    else if (plp_issy == DVBT2_PLP_ISSY_OFF)
    {
        strcpy(text, "off");
        return 0;
    }
    else
    {
        strcpy(text, "unknown");
        return -1;
    }
}



int t2frame_check_valid_NUM_DATA_SYMBOL(int fft_size_index, double gif, int NUM_DATA_SYMBOLS);
double t2frame_duration(int fft_size, double gif, int NUM_DATA_SYMBOLS, int bandwidth);
int t2frame_p2_symbol_num_cell(int fft_size, int mode);
int t2frame_p2_num_symbol(int fft_size);
int t2frame_fecframe_num_cell(int fec_type, int modulation);

// obsolete 
#if 0
int t2frame_Lf(int fft_size, int gif);
#endif 

int t2frame_num_data_symbol_min(int fft_size, int gif);
int t2frame_data_symbol_num_cell(int fft_size, int pilot, int bw_ext);
int t2frame_data_symbol_closing_frame_num_cell(int fft_size, int pilot, int bw_ext);
int t2frame_kbch_size(int plp_fec_type, int plp_cod );

//2012/1/3
//int t2frame_l1_signal_cell(int fft_size, int siso_miso);
int t2frame_l1pre_num_cell();
int t2frame_l1pos_dynamic_current(int num_plp, int num_aux);
int t2frame_l1pos_conflen(int num_plp, int num_aux, int num_rf, int fft_size);
int t2frame_is_closing_symbol(int fft_size, int gi, int pilot);

//2012/1/3 (kslee added)
int t2frame_l1_signal_cell(int fft_size, int siso_miso);


int TL_Calculate_L1_Post_Size(int L1_MOD, int FFT_SIZE, int L1_POST_INFO_SIZE);
int t2frame_num_ctot(int fft_size, int pilot, int bw_ext, int guard_interval, int num_data_symbol);



// log
static int tv_printf(char *format, ...)
{
//#ifndef _WIN32	
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
//#endif
	return 0;
}


static int tv_dvbt2_print_param(DVBT2_PARAM_t param)
{
	// print log
	//
	char text[256];
	int i;
	// general
	tv_printf("-- general\n");
	dvbt2_conv_fft_to_text(param.FFT_SIZE, text);
	tv_printf("---- FFT size: %s\n", text);
	dvbt2_conv_guard_to_text(param.GUARD_INTERVAL, text);
	tv_printf("---- Guard Interval: %s\n", text);
	sprintf(text, "%d", param.NUM_DATA_SYMBOLS);
	tv_printf("---- Number of data symbol: %s\n", text);
	
	sprintf(text, "%d", param.NUM_T2_FRAME);
	tv_printf("---- Number of T2 frame: %s\n", text);
	
	dvbt2_conv_bandwidth_to_text(param.BW, text);
	tv_printf("---- Bandwidth : %s Mhz\n", text);
	
	dvbt2_conv_bwext_to_text(param.BW_EXT, text);
	tv_printf("---- Bandwidth Extension: %s\n", text);
	
	dvbt2_conv_pilot_to_text(param.Pilot, text);
	tv_printf("---- Pilot: %s\n", text);
	
	dvbt2_conv_l1modulation_to_text(param.L1_MOD, text);
	tv_printf("---- L1 Modulation: %s\n", text);
	
	printf("---- l1 pre + l1 post: %d \n", param.l1_pre_cells + param.l1_pos_cells);        
	printf("---- Total cells: %d \n", param.c_tot_cells);
	printf("---- Cells allocated to plps: %d \n", param.plps_cells);
	printf("---- Duration in ms: %d \n", param.t2_duration);
	
	printf("---- Bitrate: %d (bps) \n", param.t2_bitrate);
	
	tv_printf("-- PLP inforamation \n");
	for (i = 0; i < param.num_plp; i++)
	{
		tv_printf("---- PLP [%d] \n", i);

                tv_printf("---- ID: %d \n", param.list_plp[i].PLP_ID);
                tv_printf("---- TYPE: %d \n", param.list_plp[i].PLP_TYPE);

		dvbt2_conv_plp_mod_to_text(param.list_plp[i].PLP_MOD, text);
		tv_printf("---- Modulation: %s \n", text);
		
		dvbt2_conv_plp_coderate_to_text(param.list_plp[i].PLP_COD, text);
		tv_printf("---- CodeRate: %s \n", text);
		
		dvbt2_conv_plp_fec_to_text(param.list_plp[i].PLP_FEC, text);
		tv_printf("---- FEC: %s \n", text);
		
		dvbt2_conv_plp_rot_to_text(param.list_plp[i].PLP_ROTATION, text);
		tv_printf("---- Rotation: %s \n", text);
		
		tv_printf("---- # of data block: %d \n", param.list_plp[i].PLP_NUM_BLOCKS);
		
		tv_printf("---- Time IL Type: %d \n", param.list_plp[i].PLP_TIME_IL_TYPE);
		tv_printf("---- Time IL LENGTH: %d \n", param.list_plp[i].PLP_TIME_IL_LENGTH);
		
		dvbt2_conv_plp_hem_to_text(param.list_plp[i].PLP_HEM, text);
		tv_printf("---- Hight Efficient Mode: %s \n", text);
		
		dvbt2_conv_plp_issy_to_text(param.list_plp[i].ISSY, text);
		tv_printf("---- ISSY: %s \n", text);
		
	}
	
	tv_printf("\n\n");
	
	return 0;
	
}





#ifdef __cplusplus
}
#endif

#endif // DVBT2_H
