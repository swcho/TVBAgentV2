#ifndef _DVBC2_BBFRAME_
#define _DVBC2_BBFRAME_

#include "dvbc2_system.h"
#include "dvbc2_input_processing.h"

#define PLP_MODE_SINGLE_PLP  0
#define PLP_MODE_MULTIPLE_PLP 1

class CTLVMedia;

class CDVBC2_BBFrame_PayLoad{
public: 

	CTLVMedia *media_source; 

	C2MI_BBFrame_Payload s_payload_bb;

	unsigned short length; // 8+8+8+k_bch
	// length of payload BBFrame in bits

	// for supporting BBframe padding	
	unsigned int num_bit_c2_frame;
	unsigned int num_bit_c2_frame_remainder;

	//unsigned int max_dfl_in_bits;
	// maximun length in bits of dfl 
	int plp_hem; 
	// normal mode of hight efficient mode
	int plp_single_multiple_mode; // single PLP or MultiPLP


	unsigned int K_bch;
	int max_dfl_in_bits;

	CDVBC2InputProcessing input_processing; 


public:	

	CDVBC2_BBFrame_PayLoad();
	void init(int _plp_cod, int _plp_fec_type, int _plp_mode, int _num_plp, CTLVMedia *_media_source);

	void SetC2Frame(int num_bits);
	int GetLenInBits();
	int MakeBBPayload(unsigned char frame_idx, unsigned char plp_id, unsigned char _intl_frame_start);

	int BBFrameToBuff(unsigned char *buff);
	int BBFramePayloadToBuff(unsigned char *buff);	

};

#endif
