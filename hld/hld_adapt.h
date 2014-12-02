
#ifndef __HLD_PCR_ADAP_H__
#define __HLD_PCR_ADAP_H__

#if defined(WIN32)
#include	"io.h"
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#endif
#include	"fcntl.h"
#include	"sys/types.h"
#include	"sys/stat.h"
#include	<time.h>

#include	"local_def.h"
#include	"../include/hld_const.h"

#include	"hld_gvar.h"
#include	"hld_bd_log.h"

class CHldAdapt
{
private:
	CHldBdLog	*_AHLog;
	CHldGVar	*_ASta;

	int				*_PktSize;	//	TL_gPacketSize;
	__int64		*_TotSndDta;	//	TL_gTotalSendData;

	//2014/1/6 fixed adaptation
	int				*_bitrate;

	__int64 giiFirstPCR;
	__int64 giiLastPCR;
	WORD	gwFirstPID;
	int 	giNoPacket;
	int 	gixFirst;
	int 	gixLast;

	time_t	m_nTimeDiff, m_nTime1, m_nTime2;
	DWORD	*m_nDiscontinuityIndicator;
	DWORD	*m_nDiscontinuity;
	int 	*m_nCount;

	int TL_gContiuityCounter;
	int TL_gDateTimeOffset;
	//TDT/TOT - USER DATE/TIM
	char TL_gUserDate[32]; 
	char TL_gUserTime[32]; 

	unsigned char m_pPesBuf_EIT[4096];
	unsigned char m_pPesBuf_MGT[4096];
	unsigned char m_pPesBuf_STT[4096];
	int m_nPos_EIT;
	int m_nPos_MGT;
	int m_nPos_STT;
	int m_nEIT_Packet_List[384];
	int m_nEIT_Packet_Count;
	time_t	m_nTimeDiff_STT;
	DWORD m_nEIT_Packet_PID[256];
	int m_nEIT_Packet_PID_Count;
	int m_nEIT_Adaptaion;
	//fixed
	int m_nEIT_adlength;
	//6.9.16 - fixed, DVB/EIT
	int m_nDVB_EIT_Adaptaion;
//	int m_nLoopAdaptaionOption;
	//FIXED - TOT/CRC
	int m_nEIT_packet_length;

	long TL_ErrLost, TL_ErrBits, TL_ErrBytes;
	long TL_ErrBitsCount, TL_ErrBytesCount;
	long TL_ErrLostPacket, TL_ErrBitsPacket, TL_ErrBytesPacket;

public:
	int TL_gNumLoop;

	unsigned char	*TL_sz2ndBufferPlay;	//	204 byte buffer pool for G.704 LI format and CIF for TDMB
	unsigned int	TL_nWritePos;
	unsigned int	TL_nReadPos;
	unsigned int	TL_nBufferCnt;

	//2014/1/3 fixed Adaptation
	unsigned int TL_nAdaptationPos;
	int TL_nAdatationFlag;
	unsigned int TL_nRemainder;

	int 			TL_gPCR_Restamping;
	double			gfRatio;
	int 			gStuffRatio[10];// for 10,100,1000,   10K, 100K, 1M
	unsigned int	gTotalReadPacket;
	int 			giTargetBitrate;
	__int64 		giAddedNullPacket;
	__int64 		giFirstPCRNullPacket;

	int TL_gRestamping;	//	flag for replace pcr/pts/dts
	int TL_gRestampLog;
	__int64 TL_giiTimeOffset;

	int m_nLoopAdaptaionOption;

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
private:
	DWORD GetPidFromHeader(DWORD data);
	DWORD DecodeAdaptationField(const BYTE *pData, DWORD pid);
	BOOL CheckContinuity(BYTE *pData);
	int CheckAndModifyContinuityCounter(BYTE *pData);
	int CheckDateTimeOffset(BYTE *pData);
	int CheckMasterGuideLoop(BYTE *pData, int adlength);
	int CheckAndModifyMasterGuideLoop(BYTE *pData);
	int CheckSystemTimeTable(BYTE *pData, int adlength);
	int CheckAndModifySystemTimeTable(BYTE *pData);
	int CheckDateTimeOffset_EIT(BYTE *pData, int adlength);
	int CheckAndModifyDateTimeOffset_EIT(BYTE *pData, DWORD* pids, int count);
	int CheckAndModifyDateTimeOffset(BYTE *pData);
	int CheckDateTimeOffset_DVB_EIT(BYTE *pData, int adlength);
	int CheckAndModifyDateTimeOffset_DVB_EIT(BYTE *pData, DWORD* pids, int count);
	int CheckNetworkInformation(BYTE *pData);
	int CheckAndModifyNetworkInformation(BYTE *pData);
	int CheckAndModifyTimeStamp(BYTE *bData);
	void Replace_PTS_DTS(BYTE *bData, __int64 iiOffset, WORD wPID, int iPTS);
	void Replace_PCR(BYTE *bData, __int64 iiOffset, WORD wPID);
	void Replace_ESCR(BYTE *bData, __int64 iiOffset, WORD wPID);

public:
	CHldAdapt(void);
	virtual ~CHldAdapt();

	void	SetFmtrDependentVar_(int *_b_pkt_size, __int64 *_tot_snd_dta, int* _b_bitrate);
	void	SetCommonMethod_20(CHldGVar *__sta__, CHldBdLog *__hLog__);

	void	InitLoopRestamp_Param(void);
	void	InitLoopPcrRestamp_Param(void);
	void	SetRestamp_Flag(int _pcrstamp, int _stamp);
	void	InitAdaptVariables_OnPlayStart(void);
	void	SetTimeDiff_at_LoopEnd(void);
	void	SetAnchorTime(void);

	void	CheckAndModifyTimeStamp(BYTE *bData, DWORD	dwSize);
	void	BSTARTClick(int target_bitrate, int source_bitrate);
	void	BSTARTClick(float fRatio, int *pStuffRatio, int *pTotalReadPacket, int *pAddedNullPacket);

	unsigned short get_tsid(unsigned char* buf);
	void	CheckAdaptation(unsigned char *pData, int Length);
	int		SetLoop_AdaptParam(long pcr_restamping, long continuity_conunter, long tdt_tot, char* user_date, char* user_time);
	int		__Set_ErrInjection_Param(long error_lost, long error_lost_packet,
		long error_bits, long error_bits_packet, long error_bits_count,
		long error_bytes, long error_bytes_packet, long error_bytes_count);
	int IsUsrReqed_DateTime_or_Restamp(void);
	int IsUsrReqed_DateTime(void);
	int IsUsrReqed_Restamp(void);

	int MakeCRC(unsigned char *Target, const unsigned char *Data, int Length);
	int MakeCRC8(unsigned char *Target, const unsigned char *Data, int Length);

	//2014/1/3 fixed Adaptation
	void AdaptationOneReadBlock(unsigned char* buf, unsigned int bufWritePos, int rdLength, int packetSize);

};

//CRC
//taken and adapted from libdtv, (c) Rolf Hakenes
class CRC32 {
public:
	CRC32(const char *d, int len, unsigned int CRCvalue=0xFFFFFFFF);
   bool isValid() { return crc32(data, length, value) == 0; }
   static bool isValid(const char *d, int len, unsigned int CRCvalue=0xFFFFFFFF) { return crc32(d, len, CRCvalue) == 0; }
   static unsigned int crc32(const char *d, int len, unsigned int CRCvalue);
protected:
   static unsigned int crc_table[256];

   const char *data;
   int length;
   unsigned int value;
};


//CRC8
class CRC8 {
public:
	void init_crc8();
	void crc8(unsigned char *crc, unsigned char m);

protected:
	static unsigned char crc8_table[256];
	int made_table;
};

#endif

