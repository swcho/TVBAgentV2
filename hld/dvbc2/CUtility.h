#ifndef _Utility_
#define _Utility_

//CRC
//taken and adapted from libdtv, (c) Rolf Hakenes
class HCRC32 
{
public:
	HCRC32(const char *d, int len, unsigned int CRCvalue=0xFFFFFFFF);
   int isValid() { return crc32(data, length, value) == 0; }
   static int isValid(const char *d, int len, unsigned int CRCvalue=0xFFFFFFFF) { return crc32(d, len, CRCvalue) == 0; }
   static unsigned int crc32(const char *d, int len, unsigned int CRCvalue);
protected:
   static unsigned int crc_table[256];

   const char *data;
   int length;
   unsigned int value;
};


//HCRC8
class HCRC8 {
public:
	void init_crc8();
	void crc8(unsigned char *crc, unsigned char m);

public:
	static unsigned char crc8_table[256];
	int made_table;
};



class CUtilities 
{

public:
	CUtilities();
	static int MakeCRC(unsigned char *Target, const unsigned char *Data, int Length);

//	static int BBFrameHeaderToBuff(BBFrameHeader bbframe_header, char *buff);	

	
	


	static void BitConcat(unsigned char *des, int syncd, int num_bits);
	static int AddBitToBuffer(unsigned char *des, int des_length_in_bit, unsigned char *src, int src_length_in_bit);
	static int TL_Calculate_L1_Post_Size(int L1_MOD, int FFT_SIZE, int L1_POST_INFO_SIZE);


	// return the length of buffer 

//	void static T2MI_BBFrameToBuff(T2MI_BBFrame_Payload t2mi_bbframe, char *buff);
//	void static T2MI_L1CurrentToBuff(T2MI_L1Current t2mi_l1, char *buff);
//	void static T2MI_TimeStampToBuff(T2MI_TimeStamp t2mi_timestamp, char *buff);

//	void static T2MI_PacketToBuff(T2MI_Packet t2mi_packet, char *buff);	

};


#endif
