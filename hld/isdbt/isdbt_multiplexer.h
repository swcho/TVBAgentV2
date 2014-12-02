// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _ISDBT_Multiplexer_
#define _ISDBT_Multiplexer_

#include "play.h"
#include "corelib.h"
#include "multiplexer.h"



/* Constants */


		
#define TSP_LAYER_NULL 0
#define TSP_LAYER_A 1
#define TSP_LAYER_B 2
#define TSP_LAYER_C 3
#define TSP_LAYER_ACDATA 4
#define TSP_LAYER_IP 8


enum TMCC_INTP
{
	ISDBT_INP_TS188 = 0,
};


enum TMCC_NUM_SEGMENT{
	ISDBT_NUM_SEGMENT_1 = 1, 
	ISDBT_NUM_SEGMENT_2 = 2, 
	ISDBT_NUM_SEGMENT_3 = 3, 
	ISDBT_NUM_SEGMENT_4 = 4, 
	ISDBT_NUM_SEGMENT_5 = 5, 
	ISDBT_NUM_SEGMENT_6 = 6, 
	ISDBT_NUM_SEGMENT_7 = 7, 
	ISDBT_NUM_SEGMENT_8 = 8, 
	ISDBT_NUM_SEGMENT_9 = 9, 
	ISDBT_NUM_SEGMENT_10 = 10, 
	ISDBT_NUM_SEGMENT_11 = 11, 
	ISDBT_NUM_SEGMENT_12 = 12, 
	ISDBT_NUM_SEGMENT_13 = 13, 
	ISDBT_NUM_SEGMENT_RESERVED = 14, 
	ISDBT_NUM_SEGMENT_UNUSED = 15, 
};

// table 3.24
enum TMCC_SYSTEM_INDENTIFIER{
	ISDBT_SYSTEM_BASE = 0,
	ISDBT_sb = 1,
};

enum TMCC_TRANSMISSION_PARAMETER_SWITCHING{
	ISDBT_TPS_NORMAL = 15,
	ISDBT_TPS_FRAME_15 = 14,
	ISDBT_TPS_FRAME_14 = 13,
	ISDBT_TPS_FRAME_13 = 12,
	ISDBT_TPS_FRAME_12 = 11,
	ISDBT_TPS_FRAME_11 = 10,
	ISDBT_TPS_FRAME_10 = 9,
	ISDBT_TPS_FRAME_9 = 8,
	ISDBT_TPS_FRAME_8 = 7,
	ISDBT_TPS_FRAME_7 = 6,
	ISDBT_TPS_FRAME_6 = 5,
	ISDBT_TPS_FRAME_5 = 4,
	ISDBT_TPS_FRAME_4 = 3,
	ISDBT_TPS_FRAME_3 = 2,
	ISDBT_TPS_FRAME_2 = 1,
	ISDBT_TPS_FRAME_1 = 0,

};

enum TMCC_EMERGENCY_ALARM
{
	ISDBT_NO_STARTUP_CONTROL = 0, 
	ISDBT_STARTUP_CONTROL  = 1,

};

enum TMCC_SEGMENT_TYPE
{
	ISDBT_SEGMENT_DIFFERENTIAL = 7,
	ISDBT_SEGMENT_SYNCHRONOUS = 0,
};


// conform to table 3-23
typedef struct  
{
	int modulation;
	int code_rate;
	int interleaving_length;
	int num_segment;
}ISDBT_Transmission_Information;


 

// conform to table 3-22
typedef struct 
{
	TMCC_SYSTEM_INDENTIFIER system_indentification;
	TMCC_TRANSMISSION_PARAMETER_SWITCHING transmission_parameter_switching;
	TMCC_EMERGENCY_ALARM emergency_alarm;
	
	// current frame
	int current_partial_flag;	
	ISDBT_Transmission_Information current_transmission_layerA;
	ISDBT_Transmission_Information current_transmission_layerB;
	ISDBT_Transmission_Information current_transmission_layerC;

	// next frame
	ISDBT_TMCC_PARTIAL_FLAG next_partial_flag;	
	ISDBT_Transmission_Information next_transmission_layerA;
	ISDBT_Transmission_Information next_transmission_layerB;
	ISDBT_Transmission_Information next_transmission_layerC;

	// phase-shift-correction // B107 --> B109
	char phase_shift; // 3 bits
	// 
	short reserve; // 12 bits

}ISDBT_TMCC_INFORMATION;


typedef struct {
	unsigned char sync;  // 8 bit
	unsigned char transport_error_indicator;  // 1 bit
	unsigned char payload_unit_start_indicator;  // 1 bit
	unsigned char transport_priority;       // 1 bit
	unsigned short pid;           // 13 bit
	unsigned char transport_scrambling_ctrl;// 2 bit
	unsigned char adaptation_field;// 2 bit
	unsigned char continuity_counter;// 4 bit
} ISDBT_TSP_Header;

// conform to table 5-10

typedef struct  
{
	unsigned char initialization_timing_indicator; // 4 bit
	unsigned char current_mode; // 2 bit
	unsigned char current_guard_interval; // 2 bits
	unsigned char next_mode; // 2 bit
	unsigned char next_guard_interval; // 2 bit
}ISDBT_GI_Information;

typedef struct  
{

	unsigned char tmcc_synchronization_word; // 1 bits
	unsigned char ac_data_effective_position; // 1 bits
	unsigned char reserved; // 2 bits

	ISDBT_GI_Information gi_information;  // 12 bits

	ISDBT_TMCC_INFORMATION tmcc_information; // 102 bits

	unsigned short reserved_future_use; // 10 bits

	unsigned int CRC_32;  // 32 bit

}ISDBT_TS_Modulation_Configuration_Information;


// table 5-8 
typedef struct
{
	unsigned short iip_packet_pointer; // 16 bits
	// modulation control configuration 
	ISDBT_TS_Modulation_Configuration_Information modulation_configuration_information; // 160 bits
	
	unsigned char iip_branch_number; // 8 bits
	unsigned char last_iip_branch_number; // 8 bits
	unsigned char network_synchronization_information_length; // 8 bits
	unsigned char *network_synchronization_information;
	unsigned char *stuffing_byte;
}ISDBT_TSP_Payload;  // 200 bytes


typedef struct 
{
	ISDBT_TSP_Header header;
	ISDBT_TSP_Payload payload;

}ISDBT_Information_Packet;

// table 5-6, ARIB STD-B31
typedef struct {
	char tmcc_identifier; // 2 bit
	char reserver; // 1 bit
	char buffer_reset_control_flag; // 1 bit
	char switch_flag_for_emergency_broadcasting; // 1 bit
	char initialization_timing_head_packet_flag; // 1 bit
	char frame_head_packet_flag; // 1 bit
	char frame_indicator; // 1 bit
	char layer_indicator; // 4 bit
	char count_down_index; // 4 bit
	char ac_data_invalid_flags; // 1 bit 
	char ac_data_effective_bytes; // 2 bit
	short tsp_counter; // 13 bits
	int stuff_or_ac; // 32 bits

}ISDBT_Information;


// conform to table 3-22
typedef struct 
{
	char ref; // 1 bit 
	short synchronization_signal; // 16 bit, number is assigned as frame number 
	TMCC_SEGMENT_TYPE system_type; // 3 BITS
	ISDBT_TMCC_INFORMATION tmcc_information; // 120 bits 
	char parity[11]; // 81 bits
}ISDBT_TMCC_SIGNAL;



typedef struct
{
	int num_TSP;	
	int m_DelayOfSymbol;	
	//double m_Bps;
//	LayerPars m_Layers[3];
	double m_FrameLength;
} CalculatedParams;



// Abstraction of input plug-in
class CHldPlayback;
class CTLVMedia;

// Read from file directly 
class ISDBT_Reader : public Multiplexer{

private:
	//TS_Reader ts_reader;
	CTLVMedia *media_source;
	char *ts_buff;


public:
	ISDBT_Reader();
	~ISDBT_Reader();
	void init();
	void quit();
	//void open(char *file_path);
	//void open(char *file_path, ISDBT_PARAM _tmcc_param, ISDBT_PLAY_MODE _mode);
	int open(CHldPlayback *_play_sys);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	int produce_item(char *buff, int size);
	void close();


};


// plug-in for ISDB-T multiplexer 
class ISDBTMultiplexer : public Multiplexer
{

public:
	int frame_indicator;
	int tsp_index; 
	int is_iip_transfered; 


	// parameters for multiplexing....
	ISDBT_PARAM tmcc_param;	
private:
	// order of tsp packet
	int tsp_order[10000];
	CalculatedParams  calculated_param;
	char *tsp_buff;

public:
   ISDBTMultiplexer();
   ~ISDBTMultiplexer();
	//void init();
	void init();
	void quit();
	int open(CHldPlayback *_play_sys);
	int produce_item(char *buff, int size);
	void seek(int percentage);
	int ioctl(int message_type, void *param);
	void close();

	int CalculateBitrate();

	// for calculating CRC32
	unsigned int crc32 (const char *d, int len, unsigned int crc);
	static unsigned int crc_table[256];


private:
	int GetRemuxedTSP(char *buff);
	void TSPOrder(int num_layer, int *layer_num_tsp, int *_tsp_oder);
	int IIPPacket2Buff(ISDBT_Information_Packet iip, char *buff);
	void MakeIIPPacket(ISDBT_Information_Packet *iip, int countinuity_counter);

	void MakeISDBTInformation(ISDBT_Information *isdbt_information);
	int ISDBTInformation2Buff(ISDBT_Information isdbt_information, char *buff);

	void BitConcat(char *des, int syncd, int num_bits);
	int AddBitToBuffer(char *des, int des_length_in_bit, unsigned char *src, int src_length_in_bit);
	int CalculateParams(CalculatedParams* pOutPars);

public:
	// temp 
	static int CalculateParams(CalculatedParams* pOutPars, ISDBT_PARAM *tmcc_param);


};

#endif
