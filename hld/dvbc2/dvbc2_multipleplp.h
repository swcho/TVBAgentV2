// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef _DVBC2_MultiplePLP
#define _DVBC2_MultiplePLP

#include "dvbc2_bbframe_payload.h"
#include "dvbc2_timestamp_payload.h"
#include "dvbc2_l1_payload.h"
#include "dvbc2_input_processing.h"
#include "dvbc2_system.h"

class  DVBC2Multiplexer;

class CDVBC2_MultiplePLP
{
public: 

	DVBC2Multiplexer *dvbc2_multiplexer;
	DVBC2_PARAM dvbc2_input_param;
	// property 
	C2MI_Packet_Header c2mi_header;

public:
	CDVBC2_BBFrame_PayLoad payload_bb[10];
	CDVBC2_L1_PayLoad payload_l1;
	CDVBC2_Timestamp_PayLoad payload_timestamp;
		

	//******************************************************

	int PLP_NUM_BLOCKS_TOTAL;
	int bbframe_order[1024];		//2011/7/11 
	int is_intl_frame_start[1024];	//2011/7/11

	//********************************************* 
	// handling the C2MI packet status
	unsigned short packet_count; 
	unsigned short superframe_idx;
	unsigned short plp_index; // support only 1 plp_id
	unsigned short frame_idx;	
	int packet_index;


	// 
	int state;

public:
   CDVBC2_MultiplePLP();
   ~CDVBC2_MultiplePLP();
	//void init();
	void init(DVBC2_PARAM _c2mi_multiple_plp, DVBC2Multiplexer *_dvbc2_multiplexer);
	void quit();
	// return: 0, normal
	// return -1, error, maybe paramer setting 
	int open();
	void close();
	

	// get next BBframe	and return len in bits 
	int GetNextC2MIPacket(c2mi_packet_status *status, unsigned char *buff);	
	int C2MIPacketToBuff(int _plp_id, unsigned char *buff);

	// searching parameter
	//************************************
	// Method:    SearchingParamater
	// FullName:  CDVBC2_MultiplePLP::SearchingParamater
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: int * NUM_DATA_SYMBOLS
	// Parameter: int * * PLP_BLOCK
	//************************************
	int SearchingParamater(int *PLP_BLOCK );
	int SearchingParamater(int startFreq, int guard_interval, int reservedTone, int numNotch, int notchStart, int notchWidth,
										   int dsliceFecHeader, int plp_fec_type, int plp_mod, int plp_bitrate);
	static int ParameterValidation(int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int num_data_symbol, int *plp_num_block);
	int CalculateTSBitrate();
	


	int ValidateBandWidth(int fft_size);
	int ValidatePLP_NUM_BLOCKS();
	int GetL1PreLenInBits();
	int getL2PostLenInBits();


private:
	static void BBFrameSelection(int num_plp, int *_plp_num_block, int *_bbframe_order);
	static int Max(int num_plp, double *_mcdf);

	// make c2mi packet header 
	void MakeC2MIPacketHeader(char packet_type,  //C2MI_PACKET_TYPE_BBF, C2MI_PACKET_TYPE_L1CURRENT, C2MI_PACKET_TYPE_AUX
		unsigned char packet_count, 
		unsigned char superframe_idx);	

	// make c2mi packet
	int MakeC2MIPacket(char packet_type,  //C2MI_PACKET_TYPE_BBF, C2MI_PACKET_TYPE_L1CURRENT, C2MI_PACKET_TYPE_AUX
		unsigned char plp_index,										
		unsigned char packet_count, 
		unsigned char superframe_idx,
		unsigned char frame_idx,
		unsigned char plp_id,
		unsigned char is_intl_frame_start);

	int GetNextC2MIPacket_V1(c2mi_packet_status *status, unsigned char *buff);

	int C2MIPacketHeaderToBuff(unsigned char *buff);
	
	int CalcC2MI_Ctot(long nStartFreq, long nGuardInterval, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth);
	int CalcC2MI_plpCells(long nPlpFecType, long nPlpMod, long nFecHeaderType);
	int CalcC2MI_PLPBitrate(long nBandWidth, long nL1TIMode, long nGuardInterval, long nPlpCod, long nPlpFecType, long nPlpNumBlk, long nHem);
};

#endif
