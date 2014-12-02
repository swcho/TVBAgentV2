//CKIM A 20120805 { 
#ifndef	__HLD_TYPE2_H__
#define	__HLD_TYPE2_H__

#include "../include/hld_const.h"

struct _TSPH_TS_NIT_INFO
{
	long network_PID;
	long NIT_Flag;
		// Set TSPH_TS_NIT_FLAG_CABLE_DELIVERY_SYSTEM_DESC_IN_TS Bit if cable_delivery_system_descriptor Exists in NIT
		// Set TSPH_TS_NIT_FLAG_SATELLITE_DELIVERY_SYSTEM_DESC_IN_TS Bit if satellite_delivery_system_descriptor Exists in NIT
		// Set TSPH_TS_NIT_FLAG_S2_SATELLITE_DELIVERY_SYSTEM_DESC_IN_TS Bit if s2_satellite_delivery_system_descriptor Exists in NIT
		// Set TSPH_TS_NIT_FLAG_TERRESTRIAL_DELIVERY_SYSTEM_DESC_IN_TS Bit if terrestrial_delivery_system_descriptor Exists in NIT
};

struct _TSPH_TS_PGM_ELMT_INFO
{
	char szStreamType[TSPH_TS_STREAM_TYPE_STR_MAX_SIZE];
		// long stream_type;
		// Formatted as "stream_type_string(stream_type)"
	long elementary_PID;
	long bit_rate;
};

struct _TSPH_TS_PGM_INFO
{
	long program_map_PID;
	long program_number;
	long PCR_PID;
	long bit_rate;
	long num_elmt_info;
	struct _TSPH_TS_PGM_ELMT_INFO *elmt_info;
};

struct _TSPH_TS_PID_INFO
{
	long PID;
		// Includes PID 0x1FFF of Null Packet
	char szPidDesc[TSPH_TS_PID_DESC_STR_MAX_SIZE];
		// "PAT" if PID is PAT PID (PID == 0x0000)
		// "NIT" if PID is NIT PID (PID == 0x0010)
		// "PMT" if PID is PMT PID
		// "Program - 0xN" if PID is elementary_PID of Program Whose program_number == N in Hexa
	long bit_rate;
	long layer_info;
		// Initialized to 0x1
	long scrambled;
};

struct _TSPH_TS_INFO
{
	long packet_size;
	long packet_count;
	long TEI_packet_count;
	long Flags;
		// Set TSPH_TS_FLAG_PAT_IN_TS Bit if PAT Exits in TS
		// Set TSPH_TS_FLAG_PMT_IN_TS Bit if PMT Exits in TS
		// Set TSPH_TS_FLAG_NIT_IN_TS Bit if NIT Exits in TS
		// Set TSPH_TS_FLAG_ALL_PMT_IN_TS Bit if All PMT w.r.t PAT Exits in TS
		// Set TSPH_TS_FLAG_PCR_IN_TS Bit if Delta PCR Available from TS
		// Set TSPH_TS_FLAG_OUT_OF_SYNC_IN_TS Bit if Out of Sync Exists in TS
	struct _TSPH_TS_NIT_INFO *nit_info;
		// NULL if Not Set TSPH_TS_FLAG_NIT_IN_TS Bit in Flag
	long num_pgm_info;
	struct _TSPH_TS_PGM_INFO *pgm_info;
		// NULL if num_pgm_info == 0
	long num_pid_info;
	struct _TSPH_TS_PID_INFO *pid_info;
		// NULL if num_pid_info == 0
};
typedef struct _T2MI_Plp_Param
{
	char id;
	char type;
	char mod;
	char cod;
	char fec;
	int blk;
	char hem;
	char rot;
	char interleave_length;
	int bitrate;
} T2MI_Plp_Param;

typedef struct _T2MI_Parsing_Info
{
	char bw;
	char bwt;
	char fft_mode;
	char guard_interval;
	char l1_modulation;
	char pilot_pattern;
	unsigned int frequency;
	int network_id;
	int t2_system_id;
	int cell_id;
	int pid;
	int papr;
	char superframe;
	int data_symbols;
	char t2_version;
	char l1_post_scrambled;
	char t2_lite_mode;
	char fef_enable;
	int fef_length;
	int fef_interval;
	int error_flag;
	T2MI_Plp_Param sPlp_Info[8];
} T2MI_Parsing_Info;

#endif	// __HLD_TYPE2_H__
//CKIM A 20120805 }