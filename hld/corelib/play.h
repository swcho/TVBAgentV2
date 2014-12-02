#ifndef _PLAY_H_
#define _PLAY_H_

// TMCC ----------------------------

enum ISDBT_TMCC_BTYPE
{
	ISDBT_BTYPE_TV = 0,
	ISDBT_BTYPE_RAD1,
	ISDBT_BTYPE_RAD3,
	ISDBT_BTYPE_TV_1SEG,
};

enum ISDBT_TMCC_GUARD_INTERVAL
{
	ISDBT_GUARD_1_32 = 0,
	ISDBT_GUARD_1_16,
	ISDBT_GUARD_1_8,
	ISDBT_GUARD_1_4
};


enum ISDBT_TMCC_PARTIAL_FLAG{
	ISDBT_NO_PARTIAL_FLAG = 0,
	ISDBT_PARTIAL_FLAG = 1,
};



enum ISDBT_TMCC_MODULATION
{
	ISDBT_MOD_DQPSK = 0, // 3 bits
	ISDBT_MOD_QPSK,
	ISDBT_MOD_16QAM,
	ISDBT_MOD_64QAM,
	ISDBT_MOD_UNUSE = 7, // un-use

};



enum ISDBT_TMCC_CODERATE
{
	
	ISDBT_RATE_1_2 = 0,
	ISDBT_RATE_2_3,
	ISDBT_RATE_3_4,
	ISDBT_RATE_5_6,
	ISDBT_RATE_7_8,
	ISDBT_RATE_UNUSE = 7,
};


enum ISDBT_TMCC_INTERLEAVING_TIME
{
	//000
	ISDBT_INTERTIME_MODE1_0 = 0,
	ISDBT_INTERTIME_MODE2_0 = 0,
	ISDBT_INTERTIME_MODE3_0 = 0,
	
	// 001
	ISDBT_INTERTIME_MODE1_4 = 1,
	ISDBT_INTERTIME_MODE2_2 = 1,
	ISDBT_INTERTIME_MODE3_1 = 1,
		
	// 010 
	ISDBT_INTERTIME_MODE1_8 = 2,
	ISDBT_INTERTIME_MODE2_4 = 2,
	ISDBT_INTERTIME_MODE3_2 = 2,

	// 011 
	ISDBT_INTERTIME_MODE1_16 = 3,
	ISDBT_INTERTIME_MODE2_8 = 3,
	ISDBT_INTERTIME_MODE3_4 = 3,

	// 100 - 110: RESERVE

	ISDBT_INTERTIME_UNUSE = 7, // un-use


};

/* */
typedef struct
{
	int m_NumberOfSegments;
	int m_modulation;
	int m_CodeRate;
	int m_TimeInterleave;
	int num_TSP;
	int bps;
	double selected_bps; // no sense 
} LayerPars;




// remuxing MPEG2 TS for isdbt
typedef struct 
{
	int layer_a_num_pids;
	int layer_a_pids[50];

	int layer_b_num_pids;
	int layer_b_pids[50];

	int layer_c_num_pids;
	int layer_c_pids[50];

}ISDBT_LAYER_INFORMATION;




typedef struct
{
	int m_BrodcastType;
	int  m_Mode;
	int m_GuardInterval;
	int m_PartialRecev;
	LayerPars m_Layers[3];
} ISDBT_PARAM;


// DVB-C2 interface 

struct DVBC2_PLP{
	char file_path[255];
	unsigned int PLP_ID;

	unsigned int PLP_MOD;
	unsigned int PLP_COD;
	unsigned int PLP_FEC;
	unsigned int PLP_HEM; // normal or hight efficient mode
	unsigned int PLP_TIME_IL_TYPE; // should be 0; modulator do not support time iterleaving 
	unsigned int PLP_TIME_IL_LENGTH; // should be 0; modulator do not support time iterleaving 
	unsigned int PLP_NUM_BLOCKS; // number of FEC Block for corresponding interleaving frame 
	unsigned int PLP_BITRATE; 
	unsigned int PLP_ROTATION; // 0: no rotation; 1: rotation
	unsigned int PLP_TARGET_BITRATE;

};

struct DVBC2_PARAM{
	// general information (l1 signal)
	unsigned int BW;
	unsigned int BW_EXT;
	unsigned int FFT_SIZE;
	unsigned int GUARD_INTERVAL;
	unsigned int FEC_TYPE;
	unsigned int L1_MOD;
	unsigned int L1_COD;
	unsigned int Pilot;
	unsigned int PAPR;
//	unsigned int BANDWIDTH_EXT; // (0 or 1)
	unsigned int L1_REPETITION; // ( 0 or 1)
	unsigned int L1x_LEN; // not use
	unsigned int NETWORK_ID; 
	unsigned int C2_ID; // 
	unsigned int Cell_ID; 
	unsigned int S1; // 0: SISO, 1: MISO
	unsigned int TX_ID_AVAILABILTY; // 0: SISO, 1: MISO
	unsigned int REGEN_FLAG; // 0: SISO, 1: MISO
	unsigned int L1_POST_EXTENSION; // 0: SISO, 1: MISO

	// frame information 
	unsigned int NUM_C2_FRAME; // number of T2 frame per a super frame
	//unsigned int NUM_SUB_SLICES; //
	unsigned int NUM_DATA_SYMBOLS;

	// PLP information
	int num_plp;
	DVBC2_PLP list_plp[10];
	// frequency of output
	unsigned int FREQUENCY;
	int PID;
	// 
	int auto_searching_data_symbol_num_block; 
};

#define MEDIA_TYPE_IP_UDP 1
#define MEDIA_TYPE_FILE 2

#define MULTIPLEX_TYPE_DVBT2 1
#define MULTIPLEX_TYPE_ISDBT_1SEG 2

#define OUTPUT_TYPE_RF  1

class CTLVMedia; 
class Multiplexer;
class Output;


class CHldPlayObserver{
public:
	virtual int Update(int user_state) = 0;
};


#endif
