//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#ifndef _TLV_MPEG2_TS_
#define _TLV_MPEG2_TS_
#include "corelib.h"
#include <stdint.h>

#define MPEG2TS_MAX_PID 1000 // ?? magic number 


typedef struct
{
	unsigned char adaptation_field_length; // 8 bit
	unsigned char discontinuity_indicator; // 1 bit
	unsigned char random_access_indicator; // 1 bit
	unsigned char priority_indicator;     // 1 bit

	unsigned char PCR_flag;             // 5 bits
	uint64_t PCR_base, PCR_ext;

	unsigned int OPCR_flag;
	uint64_t OPCR_base, OPCR_ext;

	unsigned int splicing_point_flag;
	unsigned int transport_private_data_flag;
	unsigned int adaptation_field_extension_flag;

} mpeg2ts_adaptation_field;


// TS analyzer 
typedef struct
{
	unsigned char sync;  // 8 bit
	unsigned char transport_error_indicator;  // 1 bit
	unsigned char payload_unit_start_indicator;  // 1 bit
	unsigned char transport_priority;       // 1 bit
	unsigned short pid;           // 13 bit
	unsigned char scrambling_ctrl;// 2 bit
	unsigned char adaptation_field_ctl;// 2 bit
	unsigned char continuity_counter;// 4 bit

	mpeg2ts_adaptation_field adaptation_field;

}mpeg2ts_header;


typedef struct {
	char table_id;  // 8 bit
	char section_syntax_indicator; // 1 bit
	char tmp; // 1 bit
	char reserved; // 1 bit 
	short int section_length; // 12 bit
	short int transport_stream_id; // 16 bit
	char reserved1; // 2 bit
	char version_num; // 5 bit
	char current_next_indicator; // 1 bit
	char section_number; // 8 bit
	char last_section_num; // 8 bit;

	
	struct new_datatype {
		short int program_number;  // 16 bit
		char reserved; // 3 bit
		short int network_prog_map_pid; // 13 bit

	}  section_info[20];  

	int CRC32; // 32 bits


	// do not have standard 
	int num_section; 

} mpeg2ts_program_association_section;


	//not in standard 
typedef	struct{
	char adaptation_field_len; // 8 bits
	char discontinuity_indicator;  // 1 bits
	char random_ac_indicator; // 1 bit
	// 1 bit 
	// 1 bit
	char PCR_flag; // 1 bit
	char OPCR_flag; // 1 bit
	char splicing_point_flag; // 1 bit
	char transport_private_data_flag; // 1bit
	char adaptation_field_ext_flag; // 1 bit

	uint64_t clock_reference_base; // 33 bit
	char reserved; // 6 bit
	short int clock_reference_ext; // 9 bit

} mpeg2ts_pcr;

typedef struct {	
	int program_number;
	int pcr_pid; 	

}PMTTable;

typedef struct {
	int program_number;
	int program_map_id;
}PATTable; 


typedef struct {
	int pid;
	// counting the gap b/t two TSpackets, with pid, which all have PCR information
	int pcr_gap_count;
	// if have PCR 
	int is_pcr; // 0: no PCR, 1: have PCR 

	// data for estimating bitrate of this PID 
	int num_pcr;	
	unsigned int pcr_gap_count_array[10];
	uint64_t pcr_value[10];	
	unsigned char discontinunity[10]; 
	int current_pcr_idx;

	// index of packet  in TS stream 
	uint64_t packet_index[10];

	uint64_t packet_count; 
	
}PIDTable;

typedef struct {
	uint64_t packet_count; 

	PMTTable pmt_table[10];
	int num_pmt;
	PATTable pat_table[10];
	int num_pat;
	PIDTable pid_table[5000];
	int num_pid; 

}MPEG2TS;



typedef struct {
	char table_id;  // 8 bit
	char section_syntax_indicator; // 1 bit
	char tmp; // 1 bit
	char reserved; // 1 bit 
	short int section_length; // 12 bit
	short int program_num; // 16 bit
	char reserved1; // 2 bit
	char version_num; // 5 bit
	char current_next_indicator; // 1 bit
	char section_number; // 8 bit
	char last_section_num; // 8 bit;} program_map_section;
	char reserved2; // 3 bit
	short int PCR_ID; // 13 bit
	char reserved3; // 4 bit
	short int program_info_length; // 12 bit

}mpeg2ts_program_map_section;

class MPEG2TSStatisticAndFilter
{

public:	
	int packet_size; // ts packet size
	double bitrate;

	MPEG2TS ts_statistic; 
public:
	int filter_pids[MPEG2TS_MAX_PID];
	int num_filter_pids; 
	// return 0: selected
	// return -1: not selected
	int isFilterPID(int _pid);

	void set_selected_pids(int _pid_list[], int _num_pid);
	double get_selected_bitrate();	
public:	
	MPEG2TSStatisticAndFilter();
	~MPEG2TSStatisticAndFilter();	
	void init();
	void quit();
	///////////////////////////////////////

public:
	int get_packet_size();
	double get_bitrate();
public:
	int EstimateTSPacketSize(char *ts_buff, int ts_buff_len, int credit);
	void EstimateBitrate();
	
	////////////////////////////////////////////////////////////////
public:
	int IsPIDAddedToPATTable(int _pid);
	int IsPIDAddedToPMTTable(int _pid);
	int IsPIDAddedToPIDTable(int _pid);

	int PMTTableIsPCRPID(int _pcr_pid);
	int PATTableIsProgramMapPID(int _pid);

	void TSStatistic(unsigned char *ts_packet);
	// buff include 0x47
	static void ReadTSHeader(unsigned char *buff, mpeg2ts_header *ts_header);
	// buff: payload of TS packet
	void ReadPayloadPAT(unsigned char *buff, mpeg2ts_program_association_section *pat);
	void ReadPayloadPMT(unsigned char *buff, mpeg2ts_program_map_section *pmt);
	void ReadTSPCR(unsigned char *buff, mpeg2ts_pcr *pcr);
};



#endif
