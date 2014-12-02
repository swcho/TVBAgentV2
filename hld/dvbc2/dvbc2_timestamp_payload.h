#ifndef _CDVBC2_Timestamp_PayLoad
#define _CDVBC2_Timestamp_PayLoad

class CDVBC2_Timestamp_PayLoad {

public:
	unsigned char length;

	int bw;


	C2MI_TimeStamp timestamp_payload;

public:
	
	CDVBC2_Timestamp_PayLoad();
	void MakeTimestampPayload();
	int TimestampPayloadToBuff( unsigned char *buff);
	int GetLenInBits();
	void init(int bw);
};

#endif
