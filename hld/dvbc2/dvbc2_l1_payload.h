#ifndef _CDVBC2_L1_PayLoad
#define _CDVBC2_L1_PayLoad

#include "play.h"
#include "dvbc2_system.h"

class CDVBC2_L1_PayLoad {

private:

	// to improve performance, we do not need to make buffer from l1_payload structure 
	// we .... 
	// note: check standard again to know what is exactly size ???? 
	char pre_buff[1024]; 
	C2MI_L1Current l1_payload;
	
	int length;

public:
	CDVBC2_L1_PayLoad();
	~CDVBC2_L1_PayLoad();

	void init(DVBC2_PARAM _c2mi_multi_plp);
	int getNUM_PLP();
	int getNUM_C2_FRAME();

	void MakeL1Payload(int frame_idx);

	int GetLenInBits();
	int L1PayloadToBuff(unsigned char *buff);
	int PreL1PayloadToBuff(unsigned char *buff);
	int L1PREPayloadToBuff(unsigned char* buff);
	int L1PostConfToBuff(unsigned char *buff );
	int L1PostDynamicToBuff(unsigned char *buff);


	int L1PostConfLen();
	int L1PostDynamicCurrLen();
	int L1PostExtLen();


};

#endif
