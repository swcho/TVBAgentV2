#ifndef _CDVBC2_TS
#define _CDVBC2_TS

#include <stdio.h>
#include <stdint.h>

#include "corelib.h"
#include "dvbc2_system.h"
#include "data_reader_from_ip.h"
#include "media_interface.h"




class CDVBC2_BBFrame_PayLoad;
class CDVBC2InputProcessing {
public:
	CDVBC2_BBFrame_PayLoad *payload_p;
private:
	char ts_syncd_buff[300];
	int syncd; 
	// for mod adaptation (NM or HEM)
	// normal mode (NM)
	int mode; 
	unsigned char preceding_up_crc8;


private:
	// null packet addition 
	int bitrate;
	int num_packet; 
	int num_packet_have_null;

	int target_bitrate;
	double accumulator_null;
	double accumulator_file;
	unsigned char stuff_packet[204];

	MPEG2TSStatisticAndFilter mpeg2ts_null_packet_addition; 

public:
	CDVBC2InputProcessing();
	~CDVBC2InputProcessing();

public:
	
	void init(int _mode, CDVBC2_BBFrame_PayLoad *_payload_p);
public: 

	// ts's operations
	// read one ts packet from cir_buff
	int read_one_packet(char *buff);
	// read n bytes from cir_buff
	int read_n_bytes(char *buff, int num_bytes, int *_syncd);
	// calculate the bitrate based on the current data in cir_buff
	// return -1: if it is not computed
	void set_target_bitrate(unsigned int _target_bitrate);

};


// ---------------------------- TS WRITER part -------------------------------------------------

#define CIR_BUFF_SIZE_WRITER 1024*1024
typedef struct
{
	unsigned char sync;  // 8 bit
	unsigned char transport_error_indicator;  // 1 bit
	unsigned char payload_unit_start_indicator;  // 1 bit
	unsigned char transport_priority;       // 1 bit
	unsigned short pid;           // 13 bit
	unsigned char scrambling_ctrl;// 2 bit
	unsigned char adaptation_field;// 2 bit
	unsigned char continuity_counter;// 4 bit
}C2MI_M2TS_Header;


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

} C2MI_M2TS_AdaptationField;


//
class C2MI_TS_Writer_Interface{
public: 
	//this function is called to acquire more t2mi data 
	// return number of byte in buff 
	// num_byte: num of bytes available in buff 
	virtual int GetC2MIPacket(char **buff, int num_bytes) = 0;

};


class C2MI_TS_Writer{

public:
	C2MI_TS_Writer();
	~C2MI_TS_Writer();

	C2MI_TS_Writer_Interface *c2mi_get; 
	char *c2mi_buffer;
	int c2mi_index_start; 
	int c2mi_index_last; 

	// ts property section
private:
	unsigned int pid;
	unsigned int packet_size;

private:



	//////////////////////////////////////////////
	// C2MI
	// indicate the number of bytes from first byte of cir_buff to first byte of a new (first) C2MI packet
	int pointer_field; 
	int c2mi_packet_start; 

public:
	void init(unsigned int _pid, unsigned int _packet_size);

	// return: 
	// 0: normal
	// -1: not enough data for making a TS packet
	int MakeTsPacket(char *ts_packet);

	// adaptaion field
	void AdaptaionField(C2MI_M2TS_AdaptationField *adaptation_field, int size);

	// m2ts_header to buffer
	static void M2tsHeaderToBuff(char *buff, C2MI_M2TS_Header m2ts_header);



};
#endif
