
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#ifdef WIN32
#else
#include	<stdlib.h>
#include	<string.h>
#endif
#include	<math.h>

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_adapt.h"

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
CHldAdapt::CHldAdapt(void)
{
//////////////////////////////////////////////////////////////////////////////////////
	TL_sz2ndBufferPlay = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);

//////////////////////////////////////////////////////////////////////////////////////
	int	i;

	gfRatio = 1.0;
	gTotalReadPacket = 0;
	giTargetBitrate = 1;
	giAddedNullPacket = 0;
	giFirstPCRNullPacket = 0;

	TL_gRestamping = 0;
	TL_gRestampLog = 0;
	TL_giiTimeOffset	= 0;

	TL_gContiuityCounter = 0;
	TL_gDateTimeOffset = 0;

	m_nTimeDiff = m_nTime1 = m_nTime2 = (time_t)0;
	m_nCount = (int*)malloc(sizeof(int)*MAX_PID_SIZE);
	m_nDiscontinuity = (DWORD*)malloc(sizeof(DWORD)*MAX_PID_SIZE);
	m_nDiscontinuityIndicator = (DWORD*)malloc(sizeof(DWORD)*MAX_PID_SIZE);
	for (i = 0; i < MAX_PID_SIZE; i++ )
	{
		m_nDiscontinuity[i] = 1;
		m_nCount[i] = -1;
		m_nDiscontinuityIndicator[i] = 0;
	}

	//6.9.16 - fixed, EIT/MGT
	m_nPos_EIT = 0;
	m_nPos_MGT = 0;
	m_nPos_STT = 0;
	m_nEIT_Packet_Count = 0;
	m_nTimeDiff_STT = 0;
	m_nEIT_Packet_PID_Count = 0;
	m_nEIT_Adaptaion = 0;
	//fixed
	m_nEIT_adlength = 0;
	//6.9.16 - fixed, DVB/EIT
	m_nDVB_EIT_Adaptaion = 0;
	//FIXED - TOT/CRC
	m_nEIT_packet_length = PKTSIZE;

	TL_ErrLost = TL_ErrBits = TL_ErrBytes= 0;
	TL_ErrLostPacket = TL_ErrBitsPacket = TL_ErrBytesPacket = 0;
	TL_ErrBitsCount = TL_ErrBytesCount = 0;


}

CHldAdapt::~CHldAdapt()
{
	if ( TL_sz2ndBufferPlay ) free(TL_sz2ndBufferPlay);

	if ( m_nDiscontinuityIndicator )	free(m_nDiscontinuityIndicator);
	if ( m_nDiscontinuity ) free(m_nDiscontinuity);
	if ( m_nCount ) 		free(m_nCount);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#define CHK_PACKET_INCR(a,b)														\
			if ( ((a + b) - m_nEIT_Packet_List[i]) >= PKTSIZE )						\
			{																		\
				k = ((a + b) - m_nEIT_Packet_List[i]) - PKTSIZE;					\
																					\
				while ( k > (PKTSIZE - (4+adlength)) )								\
				{																	\
					k -= (PKTSIZE - (4+adlength));									\
					if ( ++i >= m_nEIT_Packet_Count )								\
					{																\
						break;														\
					}																\
				}																	\
																					\
				if ( ++i >= m_nEIT_Packet_Count )									\
				{																	\
					break;															\
				}																	\
																					\
				a = m_nEIT_Packet_List[i];											\
				a += (4+adlength);													\
				CHK_BUF_OVERFLOW(a);												\
																					\
				a += k;																\
				CHK_BUF_OVERFLOW(a);												\
			}																		\
			else																	\
			{																		\
				a += b;																\
				CHK_BUF_OVERFLOW(a);												\
			}

#define CHK_BUF_OVERFLOW(a)						\
		if ( a >= MAX_BUFFER_BYTE_SIZE )		\
		{										\
			a -= MAX_BUFFER_BYTE_SIZE;			\
		}										

void CHldAdapt::SetFmtrDependentVar_(int *_b_pkt_size, __int64 *_tot_snd_dta, int* _b_bitrate)
{
	_PktSize	=	_b_pkt_size;	//	TL_gPacketSize;
	_TotSndDta	=	_tot_snd_dta;	//	TL_gTotalSendData;
	_bitrate = _b_bitrate;
}
void	CHldAdapt::SetCommonMethod_20(CHldGVar *__sta__, CHldBdLog *__hLog__)
{
	_AHLog	=	__hLog__;
	_ASta	=	__sta__;
}
void	CHldAdapt::InitLoopRestamp_Param(void)
{
	/* restamping function off */
	TL_gRestamping = 0;

	TL_gContiuityCounter = 0;
	TL_gDateTimeOffset = 0;
}
void	CHldAdapt::InitLoopPcrRestamp_Param(void)
{
	//---- Restamping and Start offset
	TL_gRestamping = 0;

	TL_gContiuityCounter = 0;
	TL_gDateTimeOffset = 0;
	if (!_ASta->IsModTyp_IsdbS())
	{
		TL_gPCR_Restamping = 0;
	}
}
void	CHldAdapt::SetRestamp_Flag(int _pcrstamp, int _stamp)
{
	TL_gPCR_Restamping = _pcrstamp;
	TL_gRestamping = _stamp;
}
void CHldAdapt::InitAdaptVariables_OnPlayStart(void)
{
	int	i;

	for ( i = 0; i < MAX_PID_SIZE; i++ )
	{
		m_nDiscontinuity[i] = 0;
		m_nCount[i] = -1;
		m_nDiscontinuityIndicator[i] = 0;
	}
	m_nTimeDiff = m_nTime1 = m_nTime2 = (time_t)0;
	time(&m_nTime1);

	//6.9.16 - fixed, EIT/MGT
	m_nPos_EIT = 0;
	m_nPos_MGT = 0;
	m_nPos_STT = 0;
	m_nEIT_Packet_Count = 0;
	m_nTimeDiff_STT = 0;
	m_nEIT_Packet_PID_Count = 0;
	m_nEIT_Adaptaion = 0;
	//fixed
	m_nEIT_adlength = 0;

	//6.9.16 - fixed, DVB/EIT
	m_nDVB_EIT_Adaptaion = 0;

}
void	CHldAdapt::SetTimeDiff_at_LoopEnd(void)
{
	if ( IsUsrReqed_DateTime() )
	{
		time(&m_nTime2);
		m_nTimeDiff = (m_nTime2 - m_nTime1);
	}
}
void	CHldAdapt::SetAnchorTime(void)
{
	if ( m_nTime1 == 0 )
	{
		time(&m_nTime1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
void CHldAdapt::CheckAndModifyTimeStamp(BYTE *bData, DWORD	dwSize)
{
	long	dwRemained = (long) dwSize;
	//char	text[200];
	DWORD	i;
	int		index,tindex;
 
	WORD	wPID;
	BYTE	bStreamId;
	WORD	wPesLen;
	BYTE	bPesFlag;
	BYTE	bPesHLen;

	BYTE	bADExist;
	BYTE	bADLen;

	giiFirstPCR = 0;
	giiLastPCR	= 0;
	giNoPacket  = 0;

	//------------------------------------------------------------------------
	//--- Find First 0x47 Sync
	for (i = 0; i < dwSize-*_PktSize*3;i++)	// use gpacketSize instead of 188. support 204/208 byte packet. 2003.8.22
	{
		if (bData[i] == 0x47 && bData[i+*_PktSize] == 0x47)		// Found 0x47 Sync
			break;
	}
	
	if (i >= (dwSize-*_PktSize*3))		// Not Found
		return;

	dwRemained -= i;
	index = (int) i;

	//------------------------------------------------------------------------
	//--- Replace Time Stamp 
	//    PCR in Adaption Field
	//    PTS/DTS/ESCR in PES Header

	while (dwRemained > 20)					// because we sent 1024 bytes 
	{
		if (bData[index] == 0x47) 
		{
			wPID = (bData[index+1]&0x1F)*256+bData[index+2];

			bADExist = bData[index+3] & 0x20;
			bADLen   = bData[index+4];

			//----------------------------------------------------------------
			//--- Adaptation and PCR Check
			//----------------------------------------------------------------
			if (bADExist) 
			{
				if (bADLen >= 7)		// min length for PCR or OPCR
				{
					if (bData[index+5] & 0x10)	// PCR (or OPCR = 0x08)
					{
						Replace_PCR(&bData[index+6], TL_giiTimeOffset, wPID);
					}
				} 
			}

			if ( TL_gNumLoop == 0 ) 
			{
				index += *_PktSize;
				giNoPacket++;
				dwRemained -= *_PktSize;

				continue;
			}

			//----------------------------------------------------------------
			//--- PES Header Check : PTS/DTS/ESCR Possible
			//----------------------------------------------------------------
			if (bData[index+1] & 0x40)		// PUSI : Packet Unit Start Indicator
			{
				if (bADExist)
					bADLen += 5;		// (Index of PES Header Start)
				else
					bADLen = 4;

				//--- find PES Header
				if (bData[index+bADLen] == 0x00 && 
					bData[index+bADLen+1] == 0x00 &&
					bData[index+bADLen+2] == 0x01)
				{
					bStreamId = bData[index+bADLen+3];
					wPesLen = bData[index+bADLen+4]*256 + bData[index+bADLen+5];
					bPesFlag = bData[index+bADLen+7];
					bPesHLen = bData[index+bADLen+8];

					//--- Found PES Header
					//wsprintf(text,"=== PID=%X, %d-th packet. PUSI=1, AD=%02X ==>SID=%02X, PesLen=%d, PesFlag=%02X, PesHLen=%d (%s %s %s)\n",
					//			wPID, giNoPacket, bADExist, 
					//			bStreamId, wPesLen, bPesFlag, bPesHLen,
					//			bPesFlag & 0x80 ? "PTS" : " ",
					//			bPesFlag & 0x40 ? "DTS" : " ",
					//			bPesFlag & 0x20 ? "ESCR" : " ");
					//OutputDebugString(text);

					tindex = index+bADLen+9;

					//--- PTS Exist
					if (bPesFlag & 0x80) 
					{
						Replace_PTS_DTS(&bData[tindex], TL_giiTimeOffset, wPID, 1);
						tindex += 5;				// add PTS length
					} 

					//--- DTS Exist
					if (bPesFlag & 0x40)
					{
						Replace_PTS_DTS(&bData[tindex], TL_giiTimeOffset, wPID, 0);
						tindex += 5;				// add DTS length
					}

					//--- ESCR Exist
					if (bPesFlag & 0x20)
					{
						Replace_ESCR(&bData[tindex], TL_giiTimeOffset, wPID);
					}

				} else
				{
					// Not PES 
				}
			}
		} 
		else  
		{
			break;
		}
		 
		index += *_PktSize;
		giNoPacket++;
		dwRemained -= *_PktSize;
	}
	//--- check PUSI for PTS/DTS/ESCR replace

	if (giiLastPCR > giiFirstPCR) 
	{
		double		iDiffTime = (double) giiLastPCR-giiFirstPCR;
		double		iLen	  = (gixLast-gixFirst)*PKTSIZE*8;	
		double		iBitrate  = iLen*27000000/iDiffTime;

		//wsprintf(text," <<< Bitrate for (%04X): 1stPCR=%d ms, lastPCR=%d ms, iLen=%d, bitrate=%d >>>\n",
		//     gwFirstPID, 
		//	 (int) (giiFirstPCR/27000),
		//	 (int) (giiLastPCR/27000),
		//	 (gixLast-gixFirst)*188,
		//	 (int) (iBitrate));
		//OutputDebugString(text);
	}
}
int CHldAdapt::CheckAndModifyTimeStamp(BYTE *bData)
{
	//char	text[200];
	DWORD	i=0;
	int		index,tindex;
 
	WORD	wPID;
	BYTE	bStreamId;
	WORD	wPesLen;
	BYTE	bPesFlag;
	BYTE	bPesHLen;

	BYTE	bADExist;
	BYTE	bADLen;

	giiFirstPCR = 0;
	giiLastPCR	= 0;
	giNoPacket  = 0;

	index = (int) i;

	//------------------------------------------------------------------------
	//--- Replace Time Stamp 
	//    PCR in Adaption Field
	//    PTS/DTS/ESCR in PES Header
	if (bData[index] == 0x47) 
	{
		wPID = (bData[index+1]&0x1F)*256+bData[index+2];

		bADExist = bData[index+3] & 0x20;
		bADLen   = bData[index+4];


		if ( TL_gNumLoop == 0 ) 
		{
			index += *_PktSize;
			giNoPacket++;
			TL_nAdatationFlag = 0;
			TL_nRemainder  = 0;
			return 0;
		}
		else
		{
			if(TL_nAdatationFlag == 0)
			{
				if(TL_nAdaptationPos > TL_nRemainder)
				{
					index += *_PktSize;
					giNoPacket++;
					TL_nRemainder = TL_nRemainder + *_PktSize;
					return 0;
				}
				else
				{
					double dblTime;// to sec
					double dblBitrate = (*_bitrate) / 8.0;// to bytes
					if (dblBitrate != 0) {	
						if ((*_PktSize) !=0)
							dblTime = ((*_TotSndDta)*188/(*_PktSize))/dblBitrate;
						else
							dblTime = (*_TotSndDta)/dblBitrate;
						TL_giiTimeOffset = (__int64) (27000000*dblTime);
					}
					/* 204/208 Packet */
					if ( (*_PktSize) == 204 || (*_PktSize) == 208 )
					{
						if (dblBitrate != 0) 
						{	
							dblTime = (*_TotSndDta)/dblBitrate;
							TL_giiTimeOffset = (__int64) (27000000*dblTime);
						}
					}
					TL_nAdatationFlag = 1;
					TL_nRemainder = 0;
					TL_nAdaptationPos = 0;
				}
			}
			else
			{
				if(TL_nAdaptationPos != 0)
				{
					if(TL_nAdaptationPos <= TL_nRemainder)
					{
						double dblTime;// to sec
						double dblBitrate = (*_bitrate) / 8.0;// to bytes
						if (dblBitrate != 0) {	
							if ((*_PktSize) !=0)
								dblTime = ((*_TotSndDta)*188/(*_PktSize))/dblBitrate;
							else
								dblTime = (*_TotSndDta)/dblBitrate;
							TL_giiTimeOffset = (__int64) (27000000*dblTime);
						}
						/* 204/208 Packet */
						if ( (*_PktSize) == 204 || (*_PktSize) == 208 )
						{
							if (dblBitrate != 0) 
							{	
								dblTime = (*_TotSndDta)/dblBitrate;
								TL_giiTimeOffset = (__int64) (27000000*dblTime);
							}
						}
						TL_nAdatationFlag = 1;
						TL_nRemainder = 0;
						TL_nAdaptationPos = 0;
					}
					else
					{
						TL_nRemainder = TL_nRemainder + *_PktSize;
					}
				}
			}
		}
		//----------------------------------------------------------------
		//--- Adaptation and PCR Check
		//----------------------------------------------------------------
		if (bADExist) 
		{
			if (bADLen >= 7)		// min length for PCR or OPCR
			{
				if (bData[index+5] & 0x10)	// PCR (or OPCR = 0x08)
				{
					Replace_PCR(&bData[index+6], TL_giiTimeOffset, wPID);
				}
			}
		}

		//----------------------------------------------------------------
		//--- PES Header Check : PTS/DTS/ESCR Possible
		//----------------------------------------------------------------
		if (bData[index+1] & 0x40)		// PUSI : Packet Unit Start Indicator
		{
			if (bADExist)
				bADLen += 5;		// (Index of PES Header Start)
			else
				bADLen = 4;

			//--- find PES Header
			if (bData[index+bADLen] == 0x00 && 
				bData[index+bADLen+1] == 0x00 &&
				bData[index+bADLen+2] == 0x01)
			{
				bStreamId = bData[index+bADLen+3];
				wPesLen = bData[index+bADLen+4]*256 + bData[index+bADLen+5];
				bPesFlag = bData[index+bADLen+7];
				bPesHLen = bData[index+bADLen+8];

				//--- Found PES Header
				/*
				wsprintf(text,"=== PID=%X, %d-th packet. PUSI=1, AD=%02X ==>SID=%02X, PesLen=%d, PesFlag=%02X, PesHLen=%d (%s %s %s)\n",
							wPID, giNoPacket, bADExist, 
							bStreamId, wPesLen, bPesFlag, bPesHLen,
							bPesFlag & 0x80 ? "PTS" : " ",
							bPesFlag & 0x40 ? "DTS" : " ",
							bPesFlag & 0x20 ? "ESCR" : " ");
				OutputDebugString(text);
				*/

				tindex = index+bADLen+9;

				//--- PTS Exist
				if (bPesFlag & 0x80) 
				{
					Replace_PTS_DTS(&bData[tindex], TL_giiTimeOffset, wPID, 1);
					tindex += 5;				// add PTS length
				} 

				//--- DTS Exist
				if (bPesFlag & 0x40)
				{
					Replace_PTS_DTS(&bData[tindex], TL_giiTimeOffset, wPID, 0);
					tindex += 5;				// add DTS length
				}

				//--- ESCR Exist
				if (bPesFlag & 0x20)
				{
					Replace_ESCR(&bData[tindex], TL_giiTimeOffset, wPID);
				}

			} else
			{
				// Not PES 
			}
		}
	} 
	else  
	{
		return 0;
	}
	giNoPacket++;

	if (giiLastPCR > giiFirstPCR) 
	{
		double		iDiffTime = (double) giiLastPCR-giiFirstPCR;
		double		iLen	  = (gixLast-gixFirst)*PKTSIZE*8;	
		double		iBitrate  = iLen*27000000/iDiffTime;

		/*
		wsprintf(text," <<< Bitrate for (%04X): 1stPCR=%d ms, lastPCR=%d ms, iLen=%d, bitrate=%d >>>\n",
		     gwFirstPID, 
			 (int) (giiFirstPCR/27000),
			 (int) (giiLastPCR/27000),
			 (gixLast-gixFirst)*188,
			 (int) (iBitrate));
		OutputDebugString(text);
		*/
	}

	return 0;
}

//------------------------------------------------------------------------
// PTS/DTS in PES Header 
//--- PTS (33bit) : xxxxooox oooooooo ooooooox oooooooo ooooooox
// 0) Base 32~30 
// 1) Base 29~22
// 2) Base 21~15  Reserved 1
// 3) Base 14~07
// 4) Base 06~0   Reserved  1
void CHldAdapt::Replace_PTS_DTS(BYTE *bData, __int64 iiOffset, WORD wPID, int iPTS)
{
	__int64 iiPTS;
	//char	text[100];
	
	//--- Get PTS or DTS Value
	iiPTS  = bData[0] & 0x0E;				// bit 32 ~ 30 
	iiPTS <<= 29;
	iiPTS |= (bData[1]) << 22;			// bit 29 ~ 22
	iiPTS |= (bData[2] & 0xFE) << 14;		// bit 21 ~ 15
	iiPTS |= (bData[3]) << 7;				// bit 14 ~ 7
	iiPTS |= (bData[4] & 0xFE) >> 1;		// bit  6 ~ 0

	//--- Print Original PTS/DTS
	//wsprintf(text,"(%s) PID=%X, %dth PKT. %d msec (%d sec)\n", 
	//				iPTS ? "PTS" : "DTS",
	//				wPID, 
	//				giNoPacket,
	//				(int) (iiPTS / 90),
	//				(int) (iiPTS / 90000));
	//OutputDebugString(text);

	//--- Add PCR Offset
	iiPTS += iiOffset/300;

	//--- Back to Adaptation Field
	bData[4] = (BYTE) ( ((iiPTS & 0x7F) << 1) | 0x01);
	bData[3] = (BYTE) ( (iiPTS & 0x7F80) >> 7);
	bData[2] = (BYTE) ( ((iiPTS & 0x3F8000) >> 14) | 0x01);
	bData[1] = (BYTE) ( (iiPTS & 0x3FC00000) >> 22);
#if defined(WIN32)
	bData[0] = (BYTE) ( ((iiPTS & 0x1C0000000) >> 29) | (bData[0] & 0xF1) );
#else
	bData[0] = (BYTE) ( ((iiPTS & 0x1C0000000LL) >> 29) | (bData[0] & 0xF1) );
#endif

	//--- Print Modified PTS/DTS
	iiPTS  = bData[0] & 0x0E;				// bit 32 ~ 30 
	iiPTS <<= 29;
	iiPTS |= (bData[1]) << 22;				// bit 29 ~ 22
	iiPTS |= (bData[2] & 0xFE) << 14;		// bit 21 ~ 15
	iiPTS |= (bData[3]) << 7;				// bit 14 ~ 7
	iiPTS |= (bData[4] & 0xFE) >> 1;		// bit  6 ~ 0
	//wsprintf(text,"(M%s) PID=%X, %dth PKT. %d msec (%d sec)\n", 
	//				iPTS ? "PTS" : "DTS",
	//				wPID, 
	//				giNoPacket,
	//				(int) (iiPTS / 90), 
	//				(int) (iiPTS / 90000));
	//OutputDebugString(text);
}

//------------------------------------------------------------------------
// PCR in Adaption Field
// 33bit Base + 6bit Reserved + 9bit Extension
// 0) Base 32~25
// 1) Base 24~17
// 2) Base 16~09
// 3) Base 08~01
// 4) Base 00 + Reserved + Ext 08
// 5) Ext  07~00
void CHldAdapt::Replace_PCR(BYTE *bData, __int64 iiOffset, WORD wPID)
{
	__int64	iiPCR;
	int		iPCR;
	char	text[100];

	__int64	iimPCR;		// Modified PCR
	int		iExtPCR;
	
	static	int		iOldPCR = 0;
	
	//--- Get PCR Value
	iiPCR = bData[0];		
	iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
	iiPCR += bData[1] << 17;
	iiPCR += bData[2] << 9;
	iiPCR += bData[3] << 1;
	iiPCR += (bData[4] & 0x80) >> 7;
	iiPCR *= 300;

	iiPCR += (bData[4] & 0x01) << 8;
	iiPCR += bData[5];

	//if ( TL_gPCR_Restamping == 1 && giiFirstPCR == 0)
	if ( TL_gPCR_Restamping == 1 && giFirstPCRNullPacket == 0) //???
	{
		giiFirstPCR = iiPCR;
		gwFirstPID = wPID;
		gixFirst   = giNoPacket;

		giFirstPCRNullPacket   = giAddedNullPacket;
	}

	//--- Print Original PCR
	iPCR = (int) (iiPCR/27000);
	//wsprintf(text,"(PCR) PID=%X, %dth PKT. %d msec (%d sec)\n", wPID, giNoPacket, (unsigned int)iPCR, iPCR/1000);
	//OutputDebugString(text);
	if ( TL_gNumLoop == 0 && TL_gRestampLog == 1)
	{
//		_FHLog->HldPrint(text);
		return;
	}
	
	//--- Check For Bitrate
	if (giiFirstPCR == 0)
	{
		giiFirstPCR = iiPCR;
		gwFirstPID = wPID;
		gixFirst   = giNoPacket;
	} 
	else if (wPID == gwFirstPID)
	{
		giiLastPCR = iiPCR;
		gixLast   = giNoPacket;
	}							

	//--- Add PCR Offset
	iiPCR += iiOffset;

	//--- Back to Adaptation Field
	iExtPCR = (int) (iiPCR % 300);
	iimPCR  = iiPCR/300;

	bData[5]  = (BYTE) (iExtPCR & 0xFF);
	bData[4]  = (BYTE) ((iExtPCR & 0x0100) >> 8);
	bData[4] |= (BYTE) ((iimPCR & 0x01) << 7) | 0x7E;

	bData[3]  = (BYTE) ( (iimPCR & 0x01FE) >> 1);
	bData[2]  = (BYTE) ( (iimPCR & 0x01FE00) >> 9);
	bData[1]  = (BYTE) ( (iimPCR & 0x01FE0000) >> 17);
#if defined(WIN32)
	bData[0]  = (BYTE) ( (iimPCR & 0x01FE000000) >> 25);
#else
	bData[0]  = (BYTE) ( (iimPCR & 0x01FE000000LL) >> 25);
#endif

	//--- Print Modified PCR
	iiPCR = bData[0];		
	iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
	iiPCR += bData[1] << 17;
	iiPCR += bData[2] << 9;
	iiPCR += bData[3] << 1;
	iiPCR += (bData[4] & 0x80) >> 7;
	iiPCR *= 300;

	iiPCR += (bData[4] & 0x01) << 8;
	iiPCR += bData[5];

	if ( TL_gRestampLog == 1 )
	{
#if defined(WIN32)
		wsprintf(text,"PID=%X, iOld=%d, iNew=%d, Offset=%d\n", (unsigned int)wPID, (int)iPCR, (int) (iiPCR/27000), (int) (iiOffset/27000));
#else
		sprintf(text,"PID=%X, iOld=%d, iNew=%d, Offset=%d\n", (unsigned int)wPID, (int)iPCR, (int) (iiPCR/27000), (int) (iiOffset/27000));
#endif
//		_FHLog->HldPrint(text);
	}

	iPCR = (int) (iiPCR/27000);
	
	if (iPCR > (iOldPCR + 100))
	{
		//OutputDebugString("***************** TOO DISTANCE\n");
	}
	//wsprintf(text,"iOld=%d, iNew=%d, Diff=%d, Offset=%d\n", iOldPCR, iPCR, iPCR-iOldPCR, (int) (iiOffset/27000));
	//OutputDebugString(text);
	
	iOldPCR = iPCR; 

	//wsprintf(text,"(M PCR) PID=%X, %dth PKT. %d msec (%d sec)\n", 
	//				          wPID, 
	//						  giNoPacket,
	//						  iPCR,
	//						  iPCR/1000);
	//OutputDebugString(text);
}

//------------------------------------------------------------------------
// ESCR in PES Header
// 33bit Base + 9bit Extension + 6bit Reserved
//--- ESCR (33+9) : xxoooxoo oooooooo oooooxoo oooooooo oooooxoo ooooooox
// 0) Base 32~28 + Reserved 3
// 1) Base 27~20
// 2) Base 19~13 + Reserved 1
// 3) Base 12~05
// 4) Base 04~00 + Ext 08~07 + Reserved 1
// 5) Ext  06~00 + Reserved 1
void CHldAdapt::Replace_ESCR(BYTE *bData, __int64 iiOffset, WORD wPID)
{
	__int64 iiESCR;
	BYTE	bTemp;
	//char	text[100];

	__int64	iimESCR;		// Modified PCR
	int		iExtESCR;

	//--- Get ESCR Value
	iiESCR  = ( (bData[0]&0x38) >> 1) | (bData[0]&0x03);			// bit 32 ~ 28 
	iiESCR <<= 28;
	iiESCR |= (bData[1]) << 20;										// bit 27 ~ 20
	bTemp = ( (bData[2] & 0xF8) >> 1) | (bData[2] & 0x03);
	iiESCR |= bTemp << 13;											// bit 19 ~ 13
	iiESCR |= (bData[3]) << 5;										// bit 12 ~ 5
	iiESCR |= (bData[4] & 0xF8) >> 3;								// bit  4 ~ 0

	iiESCR *= 300;
	iiESCR += (bData[4] & 0x03) << 7;								// Extended bit 8,7
	iiESCR += (bData[5] & 0xFE) >> 1;								// Extended bit 6~0

	//--- Print Original PCR
	//wsprintf(text,"     ESCR = %d msec (%d sec)\n", (int) (iiESCR / 27000), (int) (iiESCR / 27000000));
	//OutputDebugString(text);

	//--- Add PCR Offset
	iiESCR += iiOffset;

	//--- Back to Adaptation Field
	iExtESCR = (int) (iiESCR % 300);
	iimESCR  = iiESCR/300;

	bData[5]  = (BYTE) ((iExtESCR & 0x7F) << 1) | 0x01;
	bData[4]  = (BYTE) ((iExtESCR & 0x0180) >> 7) | 0x04;
	bData[4] |= (BYTE) ((iimESCR & 0x1F) << 3);

	bData[3]  = (BYTE) ( (iimESCR & 0x1FE0) >> 5);				// bit 12~ 5
	bData[2]  = (BYTE) ( (iimESCR & 0x6000) >> 13) | 0x04;		// bit 14,13
	bData[2] |= (BYTE) ( (iimESCR & 0x0F8000) >> 12);			// bit 19~15
	bData[1]  = (BYTE) ( (iimESCR & 0x0FF00000) >> 20);			// bit 27~20
	bData[0]  = (BYTE) ( (iimESCR & 0x30000000) >> 28) | 0x04;	// bit 29,28
#if defined(WIN32)
	bData[0] |= (BYTE) ( (iimESCR & 0x1C0000000) >> 27);
#else
	bData[0] |= (BYTE) ( (iimESCR & 0x1C0000000LL) >> 27);
#endif

	//--- Get ESCR Value
	iiESCR  = ( (bData[0]&0x38) >> 1) | (bData[0]&0x03);			// bit 32 ~ 28 
	iiESCR <<= 28;
	iiESCR |= (bData[1]) << 20;										// bit 27 ~ 20
	bTemp = ( (bData[2] & 0xF8) >> 1) | (bData[2] & 0x03);
	iiESCR |= bTemp << 13;											// bit 19 ~ 13
	iiESCR |= (bData[3]) << 5;										// bit 12 ~ 5
	iiESCR |= (bData[4] & 0xF8) >> 3;								// bit  4 ~ 0

	iiESCR *= 300;
	iiESCR += (bData[4] & 0x03) << 7;								// Extended bit 8,7
	iiESCR += (bData[5] & 0xFE) >> 1;								// Extended bit 6~0

	//--- Print Modified ESCR
	//wsprintf(text,"     MESCR = %d msec (%d sec)\n", (int) (iiESCR / 27000), (int) (iiESCR / 27000000));
	//OutputDebugString(text);
}
void CHldAdapt::BSTARTClick(int target_bitrate, int source_bitrate)
{
    gTotalReadPacket = 0;
    giiFirstPCR = 0;
    giAddedNullPacket = 0;
    giFirstPCRNullPacket = 0;

    //--- Set Ratio
    giTargetBitrate = target_bitrate;
	if ( source_bitrate > 0 )
	{
		gfRatio = giTargetBitrate/(source_bitrate*1.0);

		//FIXME
		if ( gfRatio < 1 )
		{
			gfRatio = 1.;
		}
	}
	else
	{
		gfRatio = 1.;
	}
	//_FHLog->HldPrint( "target_bitrate=%d, source_bitrate=%d, gfRatio=%f\n", target_bitrate, source_bitrate, gfRatio);

	gStuffRatio[0] = (int) (gfRatio*10)  - 10;
	gStuffRatio[1] = (int) (gfRatio*100) - ((int) (gfRatio*10)) * 10;
	gStuffRatio[2] = (int) (gfRatio*1000) - ((int) (gfRatio*100)) * 10;
	gStuffRatio[3] = (int) (gfRatio*10000) - ((int) (gfRatio*1000)) * 10;
	gStuffRatio[4] = (int) (gfRatio*100000) - ((int) (gfRatio*10000)) * 10;
	gStuffRatio[5] = (int) (gfRatio*1000000) - ((int) (gfRatio*100000)) * 10;
	gStuffRatio[6] = (int) (gfRatio*10000000) - ((int) (gfRatio*1000000)) * 10;
	gStuffRatio[7] = (int) (gfRatio*100000000) - ((int) (gfRatio*10000000)) * 10;
	gStuffRatio[8] = (int) (gfRatio*1000000000) - ((int) (gfRatio*100000000)) * 10;
	//gStuffRatio[9] = (int) (gfRatio*10000000000) - ((int) (gfRatio*1000000000)) * 10;
}
void CHldAdapt::BSTARTClick(float fRatio, int *pStuffRatio, int *pTotalReadPacket, int *pAddedNullPacket)
{
    *pTotalReadPacket = 0;
    *pAddedNullPacket = 0;

	*(pStuffRatio+0) = (int) ((fRatio)*10)  - 10;
	*(pStuffRatio+1) = (int) ((fRatio)*100) - ((int) ((fRatio)*10)) * 10;
	*(pStuffRatio+2) = (int) ((fRatio)*1000) - ((int) ((fRatio)*100)) * 10;
	*(pStuffRatio+3) = (int) ((fRatio)*10000) - ((int) ((fRatio)*1000)) * 10;
	*(pStuffRatio+4) = (int) ((fRatio)*100000) - ((int) ((fRatio)*10000)) * 10;
	*(pStuffRatio+5) = (int) ((fRatio)*1000000) - ((int) ((fRatio)*100000)) * 10;
	*(pStuffRatio+6) = (int) ((fRatio)*10000000) - ((int) ((fRatio)*1000000)) * 10;
	*(pStuffRatio+7) = (int) ((fRatio)*100000000) - ((int) ((fRatio)*10000000)) * 10;
	*(pStuffRatio+8) = (int) ((fRatio)*1000000000) - ((int) ((fRatio)*100000000)) * 10;
}
//////////////////////////////////////////////////////////////////////////////////////
#define MK_12BIT(X, Y)		(unsigned short)(((unsigned short)(X & 0x0f) << 8) | Y)
#define MK_13BIT(X, Y)		(unsigned short)(((unsigned short)(X & 0x1f) << 8) | Y)
#define MK_14BIT(X, Y)		(unsigned short)(((unsigned short)(X & 0x3f) << 8) | Y)
#define MK_16BIT(X, Y)		(unsigned short)(((unsigned short)X << 8) | Y)
#define ADAP_MASK			0x20000000
#define DATA_MASK			0x10000000
#define PAYLOAD_START_MASK	0x00004000
#define CHK_ADAP_MASK(a)	(a&ADAP_MASK)
#define CHK_DATA_MASK(a)	(a&DATA_MASK)
#define CHK_PAYLOAD_START_MASK(a)	(a&PAYLOAD_START_MASK)

unsigned short CHldAdapt::get_tsid(unsigned char* buf)
{
	unsigned char m_pPesBuf[4096];
	int m_nPos = 0;
	unsigned char *pData = buf;

	DWORD hdata = *(DWORD *)pData, adlength = 0, length=PKTSIZE-4;
	if(CHK_ADAP_MASK(hdata))
	{
		adlength = (unsigned long)(*(pData+4)) + 1;
	}

	if(CHK_PAYLOAD_START_MASK(hdata))
	{
	}

	if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
	{
		memcpy(m_pPesBuf+m_nPos, pData+adlength+4, length-adlength);
		m_nPos += (length-adlength);

		BYTE pointer  = m_pPesBuf[0]+1;
		BYTE table_id = m_pPesBuf[pointer];

		if ( table_id == 0x00 )
		{
			BYTE* pData1 = m_pPesBuf + pointer;
			DWORD section_length = MK_12BIT(pData1[1], pData1[2]);
			DWORD transport_stream_id = MK_16BIT(pData1[3], pData1[4]);
			
			return (unsigned short)transport_stream_id;
		}
	}

	return 0xFFFF;
}


inline DWORD CHldAdapt::GetPidFromHeader(DWORD data)
{
	// xxxx xxxx 1111 1111 xxx1 1111 xxxx xxxx
	DWORD pid =  (data & 0x00001f00) |((data & 0x00ff0000)>>16);
	return pid;
}

DWORD CHldAdapt::DecodeAdaptationField(const BYTE *pData, DWORD pid)
{
	DWORD size = *pData;
	if(size != 0)
	{
		m_nDiscontinuityIndicator[pid] = ((pData[1] >> 7) & 0x01);
		m_nDiscontinuity[pid] = m_nDiscontinuityIndicator[pid];
	}
	return size;
}

BOOL CHldAdapt::CheckContinuity(BYTE *pData)
{
	DWORD data = *(DWORD *)pData, pid;
	pid = GetPidFromHeader(data);
	if ( pid < 0 || pid >= MAX_PID_SIZE )
		return 0;

	// Get Continuity counter
	DWORD count = (data>>24) & 0x0000000f;
	if ( m_nCount[pid] == - 1 )
	{
		m_nCount[pid] = count;
		m_nDiscontinuity[pid] = 0;
		return 0;
	}

	//if(m_nDiscontinuity[pid]) 
	//{
	//	m_nCount[pid] = count;
	//	m_nDiscontinuity[pid] = 0;
	//	return 0;
	//}

	// The continuity_counter shall no be incremented when the adaptation_field_control 
	// of the packet equal '00' or '10'
	if((CHK_DATA_MASK(data) == 0) || m_nDiscontinuityIndicator[pid] == 1)
	{
		//_FHLog->HldPrint(".");
		//return 0;
		m_nDiscontinuityIndicator[pid] = 0;
		m_nDiscontinuity[pid] = 0;
		if(m_nCount[pid] != count)
		{
			// occur discontinuity...
			pData[3] = (pData[3] & 0xF0) + (unsigned char)m_nCount[pid];
			return 0;
		}
	}
	else
	{
		// calculate the expected continuity count value
		DWORD nExpectedCount = (m_nCount[pid]+1)&0xf;
		if(nExpectedCount != count)
		{
			// occur discontinuity...
			pData[3] = (pData[3] & 0xF0) + (unsigned char)nExpectedCount;
			m_nCount[pid] = nExpectedCount;
			return 1;
		}
	}


	// A continuity_counter discontinuity is indicated by the use of the discontinuity_indicator
	// in any Transport Stream packet. When the discontinuity state is true in any Transport Stream 
	// packet of a PID not designated as a PCR_PID, the continuity_counter in that packet may be
	// discontinuous with respect to the previous Transport Stream packet of the same PID.
	//if(m_nDiscontinuityIndicator[pid] == 1)
	//{
	//	m_nCount[pid] = count;
	//	m_nDiscontinuity[pid] = 0;
	//	m_nDiscontinuityIndicator[pid] = 0;
	//	return 0;
	//}
	
	// calculate the expected continuity count value
	//DWORD nExpectedCount = (m_nCount[pid]+1)&0xf;

	//TEST - CC
#if 0
	if ( TL_gNumLoop == 3 && m_nCC[0] != 100 && m_nCC[0] != 101 )
	{
			m_nCC[0] = 100;
	}
	else if ( TL_gNumLoop == 3 && m_nCC[0] == 100 )
	{
		sprintf(debug_string, "TL_gNumLoop == 3::pid = %d, cc = %d\n", pid, count);
		OutputDebugString(debug_string);

		if ( count >= 0xF )
		{
			pData[3] = (pData[3] & 0xF0);
		}
		else
		{
			pData[3] = (pData[3] & 0xF0) + (unsigned char)(count+1);
		}

		m_nCC[0] = 101;
		m_nCC_Pid[0] = pid;
	}
	else if ( TL_gNumLoop == 5 && m_nCC[1] != 200 && m_nCC[1] != 201 && m_nCC_Pid[0] != pid )
	{
		m_nCC[1] = 200;
	}
	else if ( TL_gNumLoop == 5 && m_nCC[1] == 200 && m_nCC_Pid[0] != pid )
	{
		sprintf(debug_string, "TL_gNumLoop == 5::pid = %d, cc = %d\n", pid, count);
		OutputDebugString(debug_string);
		
		if ( count >= 0xF )
		{
			pData[3] = (pData[3] & 0xF0);
		}
		else
		{
			pData[3] = (pData[3] & 0xF0) + (unsigned char)(count+1);
		}

		m_nCC[1] = 201;
		m_nCC_Pid[1] = pid;
	}
	else if ( TL_gNumLoop == 7 && m_nCC[2] != 300 && m_nCC[2] != 301 && m_nCC_Pid[0] != pid && m_nCC_Pid[1] != pid )
	{
		m_nCC[2] = 300;
	}
	else if ( TL_gNumLoop == 7 && m_nCC[2] == 300 && m_nCC_Pid[0] != pid && m_nCC_Pid[1] != pid )
	{
		sprintf(debug_string, "TL_gNumLoop == 7::pid = %d, cc = %d\n", pid, count);
		OutputDebugString(debug_string);

		if ( count >= 0xF )
		{
			pData[3] = (pData[3] & 0xF0);
		}
		else
		{
			pData[3] = (pData[3] & 0xF0) + (unsigned char)(count+1);
		}

		m_nCC[2] = 301;
		m_nCC_Pid[2] = pid;
	}
#endif

	//if(nExpectedCount != count)
	//{
	//	// occur discontinuity...
	//	pData[3] = (pData[3] & 0xF0) + (unsigned char)nExpectedCount;
	//	m_nCount[pid] = nExpectedCount;
	//	return 1;
	//}
	//m_nCount[pid] = count;
	return 0;
}

int CHldAdapt::CheckAndModifyContinuityCounter(BYTE *pData)
{
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid == 0x1FFF )
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}
	else
	{
		m_nDiscontinuityIndicator[pid] = 0;
	}
	
	return CheckContinuity(pData);
}

//6.9.16 - fixed, DVB/EIT
#define UTC_OFFSET 315964800

int CHldAdapt::CheckDateTimeOffset(BYTE *pData)
{
	BYTE pointer  = pData[0]+1;
	BYTE table_id = pData[pointer];

	//FIXED - TOT in TDT
	int tot_in_TDT = 0;

	//TDT, TOT
	if ( table_id != 0x70 && table_id != 0x73 )
	{
		return 0;
	}
	
	//section_syntax_indicator
	int section_syntax_indicator = (pData[pointer+1]&0x80)>>7;
	if ( section_syntax_indicator != 0 )
	{
		return 0;
	}

	//section_length
	int section_length = MK_12BIT(pData[pointer+1], pData[pointer+2]);
	if ( section_length > 4090 )
	{
		return 0;
	}

	//FIXED - TOT in TDT
	if ( table_id == 0x70 )
	{
		if ( pData[pointer+3+section_length] && pData[pointer+3+section_length] == 0x73 )
		{
			tot_in_TDT = 1;
		}
	}
TOT_IN_TDT:

	// Get the date/time information
	unsigned int K,W;
	int MJD = MK_16BIT(pData[pointer+3], pData[pointer+4]);
	double mjd = double(MJD);
	int Year  = int((MJD-15078.2)/365.25);
	int Month = int((MJD-14956.1-int(Year*365.25))/30.6001);
	int Date  = int(MJD - 14956-int(Year*365.25) - int(Month*30.6001));
	if(Month == 14 || Month == 15)	K = 1;
	else K=0;
	Year = Year+K+1900;
	Month = Month-1-K*12;
	int WD = int((MJD+2)%7)+1;
	W  = int((MJD/7)-2144.64);
	int WY = int((W*28/1461)-0.0079);
	int WN = W - int((WY*1461/28)+0.41);

	int Hour, Minute, Sec;	//kslee 2010/3/10
	Hour = Minute = Sec = 0;
	
	Hour = ((pData[pointer+5] & 0xF0) >> 4) * 10 + (pData[pointer+5] & 0x0F);
	Minute = ((pData[pointer+6] & 0xF0) >> 4) * 10 + (pData[pointer+6] & 0x0F);
	Sec = ((pData[pointer+7] & 0xF0) >> 4) * 10 + (pData[pointer+7] & 0x0F);
	/*
	sprintf(debug_string, "%d, %d, %d...%d\n", Hour, Minute, Sec, GetTickCount()/1000);
	_FHLog->HldPrint(debug_string);
	//OutputDebugString(debug_string);
	*/

	// Adjust the current date/time
	struct tm current, next;
	time_t now;

	current.tm_hour = Hour;
	current.tm_isdst = 0;
	current.tm_mday = Date;
	current.tm_min = Minute;
	current.tm_mon = Month-1;//0~11
	current.tm_sec = Sec;
	current.tm_wday = WD-1;//0~6
	//current.tm_yday
	current.tm_year = Year-1900;

	now = mktime(&current);
	//6.9.16 - fixed, DVB/EIT
	/*
	if ( now != -1 )
	{
		now += m_nTimeDiff;//seconds
		next = *localtime(&now);
	}
	else
	{
		if ( TL_gDateTimeOffset != 2 )
			return 0;
	}

	//using the current time
	if ( TL_gDateTimeOffset == 2 )
	{
		time(&now);
		next = *localtime(&now);
	}
	*/
	if ( now == -1 )
		return 0;

	if ( TL_gDateTimeOffset == 2 )
	{
		if ( m_nTimeDiff_STT == 0 || m_nLoopAdaptaionOption == 1 )
		{
			m_nTimeDiff_STT = now;
			time(&now);
			m_nTimeDiff_STT = now - m_nTimeDiff_STT;
		}
		else
		{
			time(&now);
		}
	}
	//TDT/TOT - USER DATE/TIME
	else if ( TL_gDateTimeOffset == 3 )
	{
		sscanf(TL_gUserDate, "%d-%d-%d", (int*)&Year, (int*)&Month, (int*)&Date);
		sscanf(TL_gUserTime, "%d-%d-%d", (int*)&Hour, (int*)&Minute, (int*)&Sec);

		current.tm_hour = Hour;
		current.tm_isdst = 0;
		current.tm_mday = Date;
		current.tm_min = Minute;
		current.tm_mon = Month-1;//0~11
		current.tm_sec = Sec;
		current.tm_year = Year-1900;

		if ( m_nTimeDiff_STT == 0 || m_nLoopAdaptaionOption == 1 )
		{
			m_nTimeDiff_STT = now;
			
			now = mktime(&current);
			m_nTimeDiff_STT = now - m_nTimeDiff_STT;
		}
		else
		{
			now = mktime(&current);
			//time(&now);

			//fixed
			time_t now1;
			time(&now1);
			m_nTime2 = now1;
			now += (m_nTime2 - m_nTime1);
		}
	}
	else if ( TL_gDateTimeOffset == 1 )
	{
		now += m_nTimeDiff;//seconds
	}
	next = *localtime(&now);
	
	// Update the current date/time
	Year = next.tm_year;
	Month = next.tm_mon + 1;
	Date = next.tm_mday;
	int L = 0;
	if ( Month == 1 || Month == 2 )
	{
		L=1;
	}
	MJD = 14956 + Date + int((Year-L)*365.25) + int((Month+1+L*12)*30.6001);
	pData[pointer+3] = (MJD >> 8) & 0x00FF;
	pData[pointer+4] = MJD & 0x00FF;
	pData[pointer+5] = ((int)(next.tm_hour/10)<<4) + (next.tm_hour%10);
	pData[pointer+6] = ((int)(next.tm_min/10)<<4) + (next.tm_min%10);
	pData[pointer+7] = ((int)(next.tm_sec/10)<<4) + (next.tm_sec%10);
	/*
	sprintf(debug_string, "%d:%d:%d\n",next.tm_hour,next.tm_min,next.tm_sec);
	_FHLog->HldPrint(debug_string);
	*/

	//FIXED - TOT in TDT
	if ( tot_in_TDT == 1 )
	{
		tot_in_TDT = 0;
		pointer = pointer+3+section_length;
		table_id = pData[pointer];

		goto TOT_IN_TDT;
	}

	//6.9.16 - fixed, DVB/EIT
	BYTE *pData1, *pData2;
	DWORD descriptors_length = 0, descriptor_length = 0;
	int len1=0, len2=0, i, j, id;
	if ( table_id == 0x73 )
	{
		descriptors_length = MK_12BIT(pData[pointer+8], pData[pointer+9]);	
		len1 = descriptors_length;

		pData1 = &pData[pointer+10];
		i = 0;
		while ( len1 > 0 )
		{
			id = pData1[i+0];
			descriptor_length = pData1[i+1];
			len2 = descriptor_length;

			pData2 = &pData1[i+2];
			j = 0;
			while ( len2 > 0 )
			{
				//local time offset
				if ( id == 0x58 )
				{
					MJD = MK_16BIT(pData2[j+6], pData2[j+7]);
					mjd = double(MJD);
					Year  = int((MJD-15078.2)/365.25);
					Month = int((MJD-14956.1-int(Year*365.25))/30.6001);
					Date  = int(MJD - 14956-int(Year*365.25) - int(Month*30.6001));
					if(Month == 14 || Month == 15)	K = 1;
					else K=0;
					Year = Year+K+1900;
					Month = Month-1-K*12;
					WD = int((MJD+2)%7)+1;
					W  = int((MJD/7)-2144.64);
					WY = int((W*28/1461)-0.0079);
					WN = W - int((WY*1461/28)+0.41);

					Hour = ((pData2[j+8] & 0xF0) >> 4) * 10 + (pData2[j+8] & 0x0F);
					Minute = ((pData2[j+9] & 0xF0) >> 4) * 10 + (pData2[j+9] & 0x0F);
					Sec = ((pData2[j+10] & 0xF0) >> 4) * 10 + (pData2[j+10] & 0x0F);
					/*
					sprintf(debug_string, "desc...%d/%d/%d,%d:%d:%d...%d\n", Year, Month, Date, Hour, Minute, Sec, GetTickCount()/1000);
					_FHLog->HldPrint(debug_string);
					OutputDebugString(debug_string);
					*/
					
					// Adjust the current date/time
					current.tm_hour = Hour;
					current.tm_isdst = 0;
					current.tm_mday = Date;
					current.tm_min = Minute;
					current.tm_mon = Month-1;//0~11
					current.tm_sec = Sec;
					current.tm_wday = WD-1;//0~6
					//current.tm_yday
					current.tm_year = Year-1900;

					now = mktime(&current);
					if ( now == -1 )
						break;

					//TDT/TOT - USER DATE/TIME
					if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
					{
						now += m_nTimeDiff_STT;
					}
					else if ( TL_gDateTimeOffset == 1 )
					{
						now += m_nTimeDiff;//seconds
					}
					next = *localtime(&now);
					
					// Update the current date/time
					Year = next.tm_year;
					Month = next.tm_mon + 1;
					Date = next.tm_mday;
					int L = 0;
					if ( Month == 1 || Month == 2 )
					{
						L=1;
					}
					MJD = 14956 + Date + int((Year-L)*365.25) + int((Month+1+L*12)*30.6001);
					pData2[j+6] = (MJD >> 8) & 0x00FF;
					pData2[j+7] = MJD & 0x00FF;
					pData2[j+8] = ((int)(next.tm_hour/10)<<4) + (next.tm_hour%10);
					pData2[j+9] = ((int)(next.tm_min/10)<<4) + (next.tm_min%10);
					pData2[j+10] = ((int)(next.tm_sec/10)<<4) + (next.tm_sec%10);
				}
				len2 -= 13;
				j += 13;
			}
			len1 -= (j+2);
			i += descriptor_length;
		}

		//FIXED - TOT/CRC
		MakeCRC(pData+pointer+10+descriptors_length, pData+pointer, 10+descriptors_length);
	}

	return 0;
}

//6.9.16 - fixed, EIT/MGT
//#define UTC_OFFSET 315964800
struct EIT_FILTER
{
    unsigned short      length_in_seconds_0     : 4;
    unsigned short      ETM_location            : 2;
    unsigned short      reserved                : 2;    // --- 1 BYTE
};

int CHldAdapt::CheckMasterGuideLoop(BYTE *pData, int adlength)
{
	BYTE* pData1 = pData, *pData2, *pData3;
	BYTE pointer  = pData1[0]+1;
	BYTE table_id = pData1[pointer];

	if ( table_id != 0xC7 )
	{
		return 0;
	}
	pData2 = pData1 + pointer;
	
	DWORD section_length = MK_12BIT(pData2[1], pData2[2]);
	if ( section_length < 14 || section_length > 4093 )
	{
		return 0;
	}

	unsigned short table_type;
	unsigned short table_type_PID;
	unsigned short table_type_version_number;
	DWORD table_type_descriptors_length;
	DWORD master_guide_loop = MK_16BIT(pData2[9],pData2[10]);
	DWORD i, j;
	int k;
	for ( j = 11,  i = 0; j < section_length && i < master_guide_loop; i++ )
	{
		pData3 = pData2+j;

		table_type = MK_16BIT(pData3[0],pData3[1]);
		table_type_PID = MK_13BIT(pData3[2],pData3[3]);
		table_type_version_number = (unsigned char)(pData3[4] & 0x1F);
		table_type_descriptors_length = MK_12BIT(pData3[9],pData3[10]);

		if ( table_type >= 0x0100 && table_type <= 0x17F )
		{
			for ( k = 0; k < 256 && k < m_nEIT_Packet_PID_Count; k++ )
			{
				if ( m_nEIT_Packet_PID[k] == table_type_PID )
					break;
			}
			if ( k == m_nEIT_Packet_PID_Count )
			{
				m_nEIT_Packet_PID[m_nEIT_Packet_PID_Count] = table_type_PID;
				++m_nEIT_Packet_PID_Count;
			}
		}
		j = j + 11 + table_type_descriptors_length;
	}

	return 0;
}

int CHldAdapt::CheckAndModifyMasterGuideLoop(BYTE *pData)
{
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid != 0x1FFB )
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}

	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		if ( m_nPos_MGT > 0 )
		{
			CheckMasterGuideLoop(m_pPesBuf_MGT, 0);
			m_nPos_MGT = 0;
		}
	}

	if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
	{
		memcpy(m_pPesBuf_MGT+m_nPos_MGT, pData+adlength+4, length-adlength);
		m_nPos_MGT += (length-adlength);
	}
	
	return 0;
}

int CHldAdapt::CheckSystemTimeTable(BYTE *pData, int adlength)
{
	BYTE pointer  = pData[0]+1;
	BYTE table_id = pData[pointer];

	if ( table_id != 0xCD )
	{
		return 0;
	}
	
	DWORD section_length = MK_12BIT(pData[pointer + 1], pData[pointer + 2]);
	if ( section_length < 17 || section_length > 1021 )
	{
		return 0;
	}

	unsigned int system_time = 
		((unsigned int)pData[pointer + 9] << 24) 
		| ((unsigned int)pData[pointer + 10] << 16)
		| ((unsigned int)pData[pointer + 11] << 8) 
		| (unsigned int)pData[pointer + 12];
	unsigned char GPS_UTC_offset = pData[pointer + 13];
	unsigned short daylight_savings = MK_16BIT(pData[pointer + 14], pData[pointer + 15]);

	DWORD i;
	for ( i = 16; i < (section_length-4);)
	{
		i = i + pData[(pointer + i) + 1 ] + 2;
	}

	time_t now, next;
	now = system_time;
	next = now + m_nTimeDiff;

	//using the current time
	if ( TL_gDateTimeOffset == 2 )
	{
		time(&now);
		next = now - UTC_OFFSET;
		//6.9.16 - fixed, DVB/EIT
		if ( m_nTimeDiff_STT == 0 || m_nLoopAdaptaionOption == 1 )
		{
			m_nTimeDiff_STT = next - system_time;
		}
	}
	//TDT/TOT - USER DATE/TIME
	else if ( TL_gDateTimeOffset == 3 )
	{
		int Year, Month, Day, Hour, Minute, Sec;
		sscanf(TL_gUserDate, "%d-%d-%d", &Year, &Month, &Day);
		sscanf(TL_gUserTime, "%d-%d-%d", &Hour, &Minute, &Sec);

		struct tm current;
		current.tm_hour = Hour;
		current.tm_isdst = 0;
		current.tm_mday = Day;
		current.tm_min = Minute;
		current.tm_mon = Month-1;//0~11
		current.tm_sec = Sec;
		current.tm_year = Year-1900;

		now = mktime(&current);
		next = now - UTC_OFFSET;
		if ( m_nTimeDiff_STT == 0 || m_nLoopAdaptaionOption == 1 )
		{
			m_nTimeDiff_STT = next - system_time;
		}
	}

	int h0 = (int)(next>>24 & 0xFF);
	int h1 = (int)(next>>16 & 0xFF);
	int h2 = (int)(next>>8 & 0xFF);
	int h3 = (int)(next>>0 & 0xFF);	
	pData[pointer + 9] = h0;
	pData[pointer + 10] = h1;
	pData[pointer + 11] = h2;
	pData[pointer + 12] = h3;

	MakeCRC(pData+pointer+i, pData+pointer, i);

	return 0;
}

int CHldAdapt::CheckAndModifySystemTimeTable(BYTE *pData)
{
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid != 0x1FFB )
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}
	
#if 1
	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
		{
			//CheckSystemTimeTable(pData+adlength+4);
			CheckSystemTimeTable(pData+adlength+4, 0);
		}
	}

#else
	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		if ( m_nPos_STT > 0 )
		{
			CheckSystemTimeTable(m_pPesBuf_STT, 0);
			m_nPos_STT = 0;
		}
	}

	if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
	{
		memcpy(m_pPesBuf_STT+m_nPos_STT, pData+adlength+4, length-adlength);
		m_nPos_STT += (length-adlength);
	}
#endif

	return 0;
}

int CHldAdapt::CheckDateTimeOffset_EIT(BYTE *pData, int adlength)
{
	BYTE pointer  = pData[0]+1;
	BYTE table_id = pData[pointer];

	if ( table_id != 0xCB )
	{
		return 0;
	}

	BYTE* pData1 = pData, *pData2, *pData3;
	pData2 = pData1 + pointer;

	DWORD section_length = MK_12BIT(pData2[1], pData2[2]);
	if ( section_length < 11 || section_length > 4093 )
	{
		return 0;
	}

	//fixed
#if defined(WIN32)
	__try
	{
#else
#endif
	unsigned short event_id;
	DWORD start_time;
	DWORD length_in_seconds = 0, length_in_seconds_total = 0;
	DWORD title_length;
	DWORD descriptors_length;
	DWORD num_events_in_section = pData2[9];
	EIT_FILTER *fltr;
	//fixed
	//DWORD k,crc_start=0;
	int k,crc_start=0;
	int i,j;

	//MakeCRC
	for (j = 10, i = 0; i < (int)num_events_in_section; i++)
	{
		pData3 = pData2+j;

		start_time = ((unsigned int)pData3[2] << 24) | ((unsigned int)pData3[3] << 16)
			| ((unsigned int)pData3[4] << 8) | (unsigned int)pData3[5];

		//TDT/TOT - USER DATE/TIME
		if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
		{
			start_time += m_nTimeDiff_STT;
		}
		else if ( TL_gDateTimeOffset == 1 )
		{
			start_time += m_nTimeDiff;
		}
		pData3[2] = (unsigned char)((start_time>>24) & 0xFF);
		pData3[3] = (unsigned char)((start_time>>16) & 0xFF);
		pData3[4] = (unsigned char)((start_time>>8) & 0xFF);
		pData3[5] = (unsigned char)((start_time>>0) & 0xFF);

		title_length = pData3[9];
		descriptors_length = MK_12BIT(pData3[title_length + 10], pData3[title_length + 11]);

		j = j + descriptors_length + title_length + 12;
	}
	if ( num_events_in_section >  0 )
	{
		//FIXED - TOT/CRC
		/*
		MakeCRC(pData2+j, pData2, j);
		crc_start = j;
		*/
		crc_start = pointer+j;
		MakeCRC(pData+pointer+j, pData+pointer, j);
	}

	//Update start time
	//fixed
	//DWORD nPos = 0, nPos0, nPos1, nPos2, nPos3;
	int nPos = 0, nPos0, nPos1, nPos2, nPos3;
	for ( i = 0, j = 0; i < m_nEIT_Packet_Count; i++ )
	{
		nPos = m_nEIT_Packet_List[i];
		nPos += (4+adlength);
		CHK_BUF_OVERFLOW(nPos);
		if ( i == 0 )
		{
			nPos += (TL_sz2ndBufferPlay[nPos] + 1);
			CHK_BUF_OVERFLOW(nPos);
			table_id = TL_sz2ndBufferPlay[nPos];

			nPos += 9;
			CHK_BUF_OVERFLOW(nPos);
			num_events_in_section = TL_sz2ndBufferPlay[nPos];

			nPos += 1;
			CHK_BUF_OVERFLOW(nPos);
		}

		for ( ; j < (int)num_events_in_section; j++ )
		{
			event_id = ((unsigned short)(TL_sz2ndBufferPlay[nPos] & 0x3f) << 8);
			CHK_PACKET_INCR(nPos, 1)
			event_id = (unsigned short)(event_id | TL_sz2ndBufferPlay[nPos]);
			CHK_PACKET_INCR(nPos, 1)

			nPos0 = nPos;
			start_time = ((unsigned int)TL_sz2ndBufferPlay[nPos] << 24);
			CHK_PACKET_INCR(nPos, 1)

			nPos1 = nPos;
			start_time |= ((unsigned int)TL_sz2ndBufferPlay[nPos] << 16);
			CHK_PACKET_INCR(nPos, 1)
			
			nPos2 = nPos;
			start_time |= ((unsigned int)TL_sz2ndBufferPlay[nPos] << 8);
			CHK_PACKET_INCR(nPos, 1)

			nPos3 = nPos;
			start_time |= ((unsigned int)TL_sz2ndBufferPlay[nPos] << 0);
			CHK_PACKET_INCR(nPos, 1)

			//TDT/TOT - USER DATE/TIME
			if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
			{
				start_time += m_nTimeDiff_STT;
			}
			else if ( TL_gDateTimeOffset == 1 )
			{
				start_time += m_nTimeDiff;
			}
			TL_sz2ndBufferPlay[nPos0] = (unsigned char)((start_time>>24) & 0xFF);
			TL_sz2ndBufferPlay[nPos1] = (unsigned char)((start_time>>16) & 0xFF);
			TL_sz2ndBufferPlay[nPos2] = (unsigned char)((start_time>>8) & 0xFF);
			TL_sz2ndBufferPlay[nPos3] = (unsigned char)((start_time>>0) & 0xFF);

			fltr = (EIT_FILTER *)&TL_sz2ndBufferPlay[nPos];
			CHK_PACKET_INCR(nPos, 1)

			length_in_seconds = ((unsigned int)fltr->length_in_seconds_0 << 16);
			length_in_seconds |= ((unsigned int)TL_sz2ndBufferPlay[nPos] << 8);
			CHK_PACKET_INCR(nPos, 1)
			length_in_seconds |= ((unsigned int)TL_sz2ndBufferPlay[nPos]);
			CHK_PACKET_INCR(nPos, 1)

			title_length = TL_sz2ndBufferPlay[nPos];
			CHK_PACKET_INCR(nPos, 1)
			CHK_PACKET_INCR(nPos, (int)title_length)

			descriptors_length = ((unsigned short)(TL_sz2ndBufferPlay[nPos] & 0x0f) << 8);
			CHK_PACKET_INCR(nPos, 1)
			descriptors_length = (unsigned short)(descriptors_length | TL_sz2ndBufferPlay[nPos]);
			CHK_PACKET_INCR(nPos, 1)
			CHK_PACKET_INCR(nPos, (int)descriptors_length)
		}
	}

	//CRC
	if ( num_events_in_section >  0 )
	{
		//FIXED - TOT/CRC pData2->pData
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+1];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+2];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+3];
	}
#if defined(WIN32)
	//fixed
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
#else
#endif

	return 0;
}

int CHldAdapt::CheckAndModifyDateTimeOffset_EIT(BYTE *pData, DWORD* pids, int count)
{
	if ( m_nEIT_Adaptaion == 1 )
	{
		CheckDateTimeOffset_EIT(m_pPesBuf_EIT, m_nEIT_adlength);
		m_nPos_EIT = 0;
		m_nEIT_Packet_Count = 0;
		m_nEIT_Adaptaion = 0;
		//fixed
		m_nEIT_adlength = 0;
	}
	
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE )
	{
		return 0;
	}
	int i;
	for ( i = 0; i < count; i++ )
	{
		if ( pid == pids[i] )
			break;
	}
	if ( i == count )
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}

	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		m_nPos_EIT = 0;
		m_nEIT_Packet_Count = 0;
		m_nEIT_Adaptaion = 0;
		m_nEIT_adlength = 0;
	}

	if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
	{
		memcpy(m_pPesBuf_EIT+m_nPos_EIT, pData+adlength+4, length-adlength);
		m_nPos_EIT += (length-adlength);
		//fixed
		m_nEIT_adlength = adlength;

		m_nEIT_Packet_List[m_nEIT_Packet_Count] = TL_nWritePos;//of TL_sz2ndBufferPlay
		++m_nEIT_Packet_Count;
		
		BYTE pointer  = m_pPesBuf_EIT[0]+1;
		BYTE table_id = m_pPesBuf_EIT[pointer];
		if ( table_id == 0xCB )
		{
			BYTE* pData1 = m_pPesBuf_EIT + pointer;

			DWORD section_length = MK_12BIT(pData1[1], pData1[2]);
			if ( section_length >= 11 && section_length <= 4093 )
			{
				//FIXED - TOT/CRC PKTSIZE -> PKTSIZE-(4+adlength) -> +pointer
				int n = (int)(ceil((4+adlength+pointer+3+section_length)/(double)(PKTSIZE-(4+adlength))));
				if ( m_nEIT_Packet_Count == n )
				{
					m_nEIT_Adaptaion = 1;
				}
			}
		}
	}
	
	return 0;
}

int CHldAdapt::CheckAndModifyDateTimeOffset(BYTE *pData)
{
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid != 0x14 ) //TDT, TOT
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}
	else
	{
		m_nDiscontinuityIndicator[pid] = 0;
	}
	
	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
		{
			//using the current time
			//if ( m_nTimeDiff > 0 )
			{
				CheckDateTimeOffset(pData+adlength+4);
			}
		}
	}

	return 0;
}

//6.9.16 - fixed, DVB/EIT
int CHldAdapt::CheckDateTimeOffset_DVB_EIT(BYTE *pData, int adlength)
{
	BYTE pointer  = pData[0]+1;
	BYTE table_id = pData[pointer];

	//FIXED - TOT/CRC
	//if ( table_id != 0x4E && table_id != 0x4F && table_id != 0x50 && table_id != 0x6F )
	if ( table_id != 0x4E && table_id != 0x4F && !((table_id >= 0x50 && table_id <= 0x5F) || (table_id >= 0x60 && table_id <= 0x6F)) )
	{
		return 0;
	}

	BYTE* pData1 = pData, *pData2, *pData3;
	pData2 = pData1 + pointer;

	DWORD section_length = MK_12BIT(pData2[1], pData2[2]);
	if ( section_length < 11 || section_length > 4093 )
	{
		return 0;
	}

	//fixed
#if defined(WIN32)	
	__try
	{
#else
#endif
	unsigned short event_id;
	DWORD length_in_seconds = 0, length_in_seconds_total = 0;
	DWORD descriptors_length;
	DWORD num_events_in_section = 0;
	int i, j, k, len1=section_length-11, crc_start=0;

	unsigned int K,W;
	double mjd;
	int MJD, Year, Month, Date, WD, WY, WN, L;
	unsigned char Hour, Minute, Sec;
	struct tm current, next;
	time_t now;

	//MakeCRC
	j = 14;
//	OutputDebugString("\n");
	while(len1 > 4)
	{
		pData3 = pData2+j;

		event_id = MK_16BIT(pData3[0], pData3[1]);

		// Get the date/time information
		MJD = MK_16BIT(pData3[2], pData3[3]);
		mjd = double(MJD);
		Year  = int((MJD-15078.2)/365.25);
		Month = int((MJD-14956.1-int(Year*365.25))/30.6001);
		Date  = int(MJD - 14956-int(Year*365.25) - int(Month*30.6001));
		if(Month == 14 || Month == 15)	K = 1;
		else K=0;
		Year = Year+K+1900;
		Month = Month-1-K*12;
		WD = int((MJD+2)%7)+1;
		W  = int((MJD/7)-2144.64);
		WY = int((W*28/1461)-0.0079);
		WN = W - int((WY*1461/28)+0.41);
		Hour = Minute = Sec = 0;
		Hour = ((pData3[4] & 0xF0) >> 4) * 10 + (pData3[4] & 0x0F);
		Minute = ((pData3[5] & 0xF0) >> 4) * 10 + (pData3[5] & 0x0F);
		Sec = ((pData3[6] & 0xF0) >> 4) * 10 + (pData3[6] & 0x0F);
		/*
		sprintf(debug_string, "event_id=%X..%d/%d/%d,%d:%d:%d...%d\n", event_id, Year, Month, Date, Hour, Minute, Sec, GetTickCount()/1000);
		_FHLog->HldPrint(debug_string);
		OutputDebugString(debug_string);
		*/
		
		// Adjust the current date/time
		current.tm_hour = Hour;
		current.tm_isdst = 0;
		current.tm_mday = Date;
		current.tm_min = Minute;
		current.tm_mon = Month-1;//0~11
		current.tm_sec = Sec;
		current.tm_wday = WD-1;//0~6
		//current.tm_yday
		current.tm_year = Year-1900;

		now = mktime(&current);
		if ( now != -1 )
		{
			//TDT/TOT - USER DATE/TIME
			if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
			{
				now += m_nTimeDiff_STT;
			}
			else if ( TL_gDateTimeOffset == 1 )
			{
				now += m_nTimeDiff;
			}

			next = *localtime(&now);
		}
		
		// Update the current date/time
		Year = next.tm_year;
		Month = next.tm_mon + 1;
		Date = next.tm_mday;
		L = 0;
		if ( Month == 1 || Month == 2 )
		{
			L=1;
		}
		MJD = 14956 + Date + int((Year-L)*365.25) + int((Month+1+L*12)*30.6001);
		pData3[2] = (MJD >> 8) & 0x00FF;
		pData3[3] = MJD & 0x00FF;
		pData3[4] = ((int)(next.tm_hour/10)<<4) + (next.tm_hour%10);
		pData3[5] = ((int)(next.tm_min/10)<<4) + (next.tm_min%10);
		pData3[6] = ((int)(next.tm_sec/10)<<4) + (next.tm_sec%10);
		/*
		sprintf(debug_string, "%d/%d/%d,%d:%d:%d...%d\n", Year, Month, Date, Hour, Minute, Sec, GetTickCount()/1000);
		_FHLog->HldPrint(debug_string);
		OutputDebugString(debug_string);
		*/

		descriptors_length = MK_12BIT(pData3[10], pData3[11]);
		len1 -= (12+descriptors_length);
		j += 12;
		/*
		int len2 = descriptors_length, X;
		while ( len2 > 0 )
		{
			X = ((int)pData2[j+1]) + 2;
			j += X;
			len2 -= X;
		}
		*/
		j += descriptors_length;

		++num_events_in_section;
	}
	if ( num_events_in_section >  0 )
	{
		//FIXED - TOT/CRC
		/*
		MakeCRC(pData2+j, pData2, j);
		crc_start = j;
		*/
		crc_start = pointer+j;
		MakeCRC(pData+pointer+j, pData+pointer, j);
	}

	//Update start time
	int nPos = 0, nPos0, nPos1, nPos2, nPos3, nPos4;
	for ( i = 0, j = 0; i < m_nEIT_Packet_Count; i++ )
	{
		nPos = m_nEIT_Packet_List[i];
		nPos += (4+adlength);
		CHK_BUF_OVERFLOW(nPos);
		if ( i == 0 )
		{
			nPos += (TL_sz2ndBufferPlay[nPos] + 1);
			CHK_BUF_OVERFLOW(nPos);
			table_id = TL_sz2ndBufferPlay[nPos];

			nPos += 14;
			CHK_BUF_OVERFLOW(nPos);
		}

		for ( ; j < (int)num_events_in_section; j++ )
		{
			event_id = ((unsigned short)(TL_sz2ndBufferPlay[nPos] & 0xff) << 8);
			CHK_PACKET_INCR(nPos, 1)
			event_id = (unsigned short)(event_id | TL_sz2ndBufferPlay[nPos]);
			CHK_PACKET_INCR(nPos, 1)

			nPos0 = nPos;
			MJD = (TL_sz2ndBufferPlay[nPos] << 8);
			CHK_PACKET_INCR(nPos, 1)

			nPos1 = nPos;
			MJD |= (TL_sz2ndBufferPlay[nPos] << 0);
			CHK_PACKET_INCR(nPos, 1)
			
			nPos2 = nPos;
			Hour = ((TL_sz2ndBufferPlay[nPos] & 0xF0) >> 4) * 10 + (TL_sz2ndBufferPlay[nPos] & 0x0F);
			CHK_PACKET_INCR(nPos, 1)

			nPos3 = nPos;
			Minute = ((TL_sz2ndBufferPlay[nPos] & 0xF0) >> 4) * 10 + (TL_sz2ndBufferPlay[nPos] & 0x0F);
			CHK_PACKET_INCR(nPos, 1)

			nPos4 = nPos;
			Sec = ((TL_sz2ndBufferPlay[nPos] & 0xF0) >> 4) * 10 + (TL_sz2ndBufferPlay[nPos] & 0x0F);
			CHK_PACKET_INCR(nPos, 1)

			// Get the date/time information
			mjd = double(MJD);
			Year  = int((MJD-15078.2)/365.25);
			Month = int((MJD-14956.1-int(Year*365.25))/30.6001);
			Date  = int(MJD - 14956-int(Year*365.25) - int(Month*30.6001));
			if(Month == 14 || Month == 15)	K = 1;
			else K=0;
			Year = Year+K+1900;
			Month = Month-1-K*12;
			WD = int((MJD+2)%7)+1;
			W  = int((MJD/7)-2144.64);
			WY = int((W*28/1461)-0.0079);
			WN = W - int((WY*1461/28)+0.41);

			// Adjust the current date/time
			current.tm_hour = Hour;
			current.tm_isdst = 0;
			current.tm_mday = Date;
			current.tm_min = Minute;
			current.tm_mon = Month-1;//0~11
			current.tm_sec = Sec;
			current.tm_wday = WD-1;//0~6
			//current.tm_yday
			current.tm_year = Year-1900;

			now = mktime(&current);
			if ( now != -1 )
			{
				//TDT/TOT - USER DATE/TIME
				if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
				{
					now += m_nTimeDiff_STT;
				}
				else if ( TL_gDateTimeOffset == 1 )
				{
					now += m_nTimeDiff;
				}

				next = *localtime(&now);
			}
			
			// Update the current date/time
			Year = next.tm_year;
			Month = next.tm_mon + 1;
			Date = next.tm_mday;
			L = 0;
			if ( Month == 1 || Month == 2 )
			{
				L=1;
			}
			MJD = 14956 + Date + int((Year-L)*365.25) + int((Month+1+L*12)*30.6001);
			TL_sz2ndBufferPlay[nPos0] = (MJD >> 8) & 0x00FF;
			TL_sz2ndBufferPlay[nPos1] = MJD & 0x00FF;
			TL_sz2ndBufferPlay[nPos2] = ((int)(next.tm_hour/10)<<4) + (next.tm_hour%10);
			TL_sz2ndBufferPlay[nPos3] = ((int)(next.tm_min/10)<<4) + (next.tm_min%10);
			TL_sz2ndBufferPlay[nPos4] = ((int)(next.tm_sec/10)<<4) + (next.tm_sec%10);
			/*
			sprintf(debug_string, "%d/%d/%d,%d:%d:%d...%d\n", Year, Month, Date, Hour, Minute, Sec, GetTickCount()/1000);
			_FHLog->HldPrint(ebug_string);
			OutputDebugString(debug_string);
			*/
			CHK_PACKET_INCR(nPos, 3)
			descriptors_length = ((unsigned short)(TL_sz2ndBufferPlay[nPos] & 0x0f) << 8);
			CHK_PACKET_INCR(nPos, 1)
			descriptors_length = (unsigned short)(descriptors_length | TL_sz2ndBufferPlay[nPos]);
			CHK_PACKET_INCR(nPos, 1)
			CHK_PACKET_INCR(nPos, (int)descriptors_length)
		}
	}

	//CRC
	if ( num_events_in_section >  0 )
	{
		//FIXED - TOT/CRC pData2->pData
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+1];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+2];
		nPos += 1;
		CHK_BUF_OVERFLOW(nPos);
		TL_sz2ndBufferPlay[nPos] = pData[crc_start+3];
	}
#if defined(WIN32)
	//fixed
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
#else
#endif
	return 0;
}

int CHldAdapt::CheckAndModifyDateTimeOffset_DVB_EIT(BYTE *pData, DWORD* pids, int count)
{
	//6.9.16 - fixed, DVB/EIT
	if ( m_nDVB_EIT_Adaptaion == 1 )
	{
		CheckDateTimeOffset_DVB_EIT(m_pPesBuf_EIT, m_nEIT_adlength);
		m_nPos_EIT = 0;
		m_nEIT_Packet_Count = 0;
		m_nEIT_Adaptaion = 0;
		//fixed
		m_nEIT_adlength = 0;
		//6.9.16 - fixed, DVB/EIT
		m_nDVB_EIT_Adaptaion = 0;
	}
	
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid != 0x12 )
	{
		return 0;
	}
	/*
	int i;
	for ( i = 0; i < count; i++ )
	{
		if ( pid == pids[i] )
			break;
	}
	if ( i == count )
	{
		return 0;
	}
	*/

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}

	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		m_nPos_EIT = 0;
		m_nEIT_Packet_Count = 0;
		m_nEIT_Adaptaion = 0;
		m_nEIT_adlength = 0;
		//6.9.16 - fixed, DVB/EIT
		m_nDVB_EIT_Adaptaion = 0;
	}

	if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
	{
		memcpy(m_pPesBuf_EIT+m_nPos_EIT, pData+adlength+4, length-adlength);
		m_nPos_EIT += (length-adlength);
		//fixed
		m_nEIT_adlength = adlength;

		m_nEIT_Packet_List[m_nEIT_Packet_Count] = TL_nWritePos;//of TL_sz2ndBufferPlay
		++m_nEIT_Packet_Count;
		
		BYTE pointer  = m_pPesBuf_EIT[0]+1;
		BYTE table_id = m_pPesBuf_EIT[pointer];

		//FIXED - TOT/CRC
		//if ( table_id == 0x4E || table_id == 0x4F || table_id == 0x50 || table_id == 0x6F )
		if ( table_id == 0x4E || table_id == 0x4F || ((table_id >= 0x50 && table_id <= 0x5F) || (table_id >= 0x60 && table_id <= 0x6F)) )
		{
			BYTE* pData1 = m_pPesBuf_EIT + pointer;

			DWORD section_length = MK_12BIT(pData1[1], pData1[2]);
			if ( section_length >= 11 && section_length <= 4093 )
			{
				//FIXED - TOT/CRC PKTSIZE -> PKTSIZE-(4+adlength) -> +pointer
				int n = (int)(ceil((4+adlength+pointer+3+section_length)/(double)(PKTSIZE-(4+adlength))));
				if ( m_nEIT_Packet_Count == n )
				{
					//6.9.16 - fixed, DVB/EIT
					m_nDVB_EIT_Adaptaion = 1;
				}
			}
		}
	}
	
	return 0;
}

DWORD swap32(const DWORD val)
{
	DWORD retVal;
#if defined(WIN32)	
	__asm
	{
		mov		eax, val
		bswap	eax
		mov		retVal, eax;
	}
#else
#endif
	return retVal;
}

void CHldAdapt::CheckAdaptation(unsigned char *pData, int Length)
{
	//FIXED - TOT/CRC
	m_nEIT_packet_length = Length;

	if ( _ASta->IsModTyp_Tdmb() ) //TDMB
	{
		return;
	}

	if (TL_gRestamping == 1)
	{	
		CheckAndModifyTimeStamp(pData);
	}

	if ( TL_gContiuityCounter == 1 )
	{
		CheckAndModifyContinuityCounter(pData);
	}

	//TDT/TOT - USER DATE/TIME
	if ( TL_gDateTimeOffset == 1 || TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
	{
		//6.9.16 - fixed, EIT/MGT
		CheckAndModifySystemTimeTable(pData);
		//6.9.16 - fixed, DVB/EIT
		if ( TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 || m_nLoopAdaptaionOption == 1 )
		{
			CheckAndModifyMasterGuideLoop(pData);
			CheckAndModifyDateTimeOffset_EIT(pData, m_nEIT_Packet_PID, m_nEIT_Packet_PID_Count);

			//6.9.16 - fixed, DVB/EIT
			CheckAndModifyDateTimeOffset_DVB_EIT(pData, m_nEIT_Packet_PID, m_nEIT_Packet_PID_Count);
		}

		CheckAndModifyDateTimeOffset(pData);
	}

	*_TotSndDta += Length;

	//2012/9/4 kslee
	//BYTE *pBuf = pData + 4;
	BYTE *pBuf = pData;
	DWORD hdata = *(DWORD *)pBuf;
	int i=0, j=0, k=0;
	if ( TL_ErrLost == 1 && TL_ErrLostPacket > 0 )	//	requested pkt lost
	{
		if ( (*_TotSndDta / Length) % TL_ErrLostPacket == 1 )
		{
			//2012/9/4 kslee
			//pid = GetPidFromHeader(hdata);
			//if ( pid >= 0 && pid < MAX_PID_SIZE && CHK_ADAP_MASK(hdata) && *pBuf != 0 )
			{
				pBuf[1] |= 0x80;	//	replace ts hdr.
			}
		}
	}

	if ( TL_ErrBits == 1 && TL_ErrBitsPacket > 0 )	//	requested bit lost
	{
		if ( ((*_TotSndDta / Length) % TL_ErrBitsPacket) == 1 )
		{
			for (i = 187; i >= 4; i-- )
			{
				for (j = 0; j < 8; j++)
				{
					if ( (pData[i] >> j) & 0x01 ) 
						pData[i] &= ~(1 << j);		//	bit complement
					else
						pData[i] |= ~(1 << j);		//	bit complement

					if ( ++k >= TL_ErrBitsCount )
						break;
				}

				if ( k >= TL_ErrBitsCount )
					break;
			}
		}
	}

	if ( TL_ErrBytes == 1 && TL_ErrBytesPacket > 0 )	//	requested byte lost
	{
		k=0;
		if ( ((*_TotSndDta / Length) % TL_ErrBytesPacket) == 1 )
		{
			for (i = 187; i >= 4; i-- )
			{
				pData[i] = ~pData[i];

				if ( ++k >= TL_ErrBytesCount )
					break;
			}
		}
	}
}


int CHldAdapt::CheckNetworkInformation(BYTE *pData)
{
	BYTE pointer  = pData[0]+1;
	BYTE table_id = pData[pointer];
	
	//NIT
	if ( table_id != 0x40 /*&& table_id != 0x41*/ )
	{
		return 0;
	}
	
	//section_syntax_indicator
	int section_syntax_indicator = (pData[pointer+1]&0x80)>>7;
	if ( section_syntax_indicator != 1 )
	{
		return 0;
	}

	//section_length
	int section_length = MK_12BIT(pData[pointer+1], pData[pointer+2]);
	if ( section_length > 1024 )
	{
		return 0;
	}

	//section_number
	int section_number = pData[pointer+6];

	//last_section_number
	int last_section_number = pData[pointer+7];

	int network_descriptors_length = MK_12BIT(pData[pointer+8], pData[pointer+9]);
	//...decode network descriptors in here

	unsigned int pos = pointer+(10+network_descriptors_length);
	int transport_stream_loop_length = MK_12BIT(pData[pos], pData[pos+1]);
	int size = 0;
	for ( ; size < transport_stream_loop_length; )
	{
		int transport_stream_id = MK_16BIT(pData[pos+2], pData[pos+3]);
		int original_network_id = MK_16BIT(pData[pos+4], pData[pos+5]);

		int transport_descriptors_length = MK_12BIT(pData[pos+6], pData[pos+7]);
		unsigned int pos2 = pos+8;
		unsigned short descriptor_tag;
		unsigned short descriptor_length;
		int size2 = 0;
		for ( ; size2 < transport_descriptors_length; )
		{
			descriptor_tag = pData[pos2];
			descriptor_length = pData[pos2+1];

			//DESCRIPTOR_SATELLITE_DELIVERY_SYSTEM
			if ( descriptor_tag == 0x43 )
			{
			}
			//DESCRIPTOR_CABLE_DELIVERY_SYSTEM
			if ( descriptor_tag == 0x44 )
			{
			}
			//DESCRIPTOR_TERRESTRIAL_DELIVERY_SYSTEM
			if ( descriptor_tag == 0x5A )
			{
				float	centre_frequency;
				unsigned short		bandwidth;
				unsigned short		constellation;
				unsigned short		hierachy_information;
				unsigned short		code_rate_HP_stream;
				unsigned short		code_rate_LP_stream;
				unsigned short		guard_interval;
				unsigned short		transmission_mode;
				unsigned short		other_frequency_flag;

				centre_frequency = float(swap32(*(DWORD *)(pData+pos2+2))*10.0);
				bandwidth		= (pData[pos2+6]&0xE0)>>5;
				constellation	= (pData[pos2+7]&0xC0)>>6;
				hierachy_information	= (pData[pos2+7]&0x38)>>3;
				code_rate_HP_stream		= (pData[pos2+7]&0x07);
				code_rate_LP_stream		= (pData[pos2+8]&0xE0)>>5;
				guard_interval			= (pData[pos2+8]&0x18)>>3;
				transmission_mode		= (pData[pos2+8]&0x06)>>1;
				other_frequency_flag	= (pData[pos2+8]&0x01);
//				_FHLog->HldPrint("centre_frequency=%f\n", centre_frequency);
			}

			pos2 += descriptor_length;
			size2 += descriptor_length;
		}

		pos += transport_descriptors_length+6;
		size += transport_descriptors_length+6;
	}
	
	return 0;
}
int CHldAdapt::CheckAndModifyNetworkInformation(BYTE *pData)
{
	DWORD hdata = *(DWORD *)pData, pid, adlength = 0, length=PKTSIZE-4;
	pid = GetPidFromHeader(hdata);
	if ( pid < 0 || pid >= MAX_PID_SIZE || pid != 0x10 ) //NIT
	{
		return 0;
	}

	if(CHK_ADAP_MASK(hdata))
	{
		adlength = DecodeAdaptationField(pData+4, pid) +1;
	}
	
	/*
	if(CheckContinuity(data) == TRUE) 
		return;

	if(m_bWrite[pid] == FALSE) return;
	*/

	if(CHK_PAYLOAD_START_MASK(hdata))
	{
		if(CHK_DATA_MASK(hdata) && length - adlength > 0) 
		{
			CheckNetworkInformation(pData+adlength+4);
		}
	}

	return 0;
}



//TDT/TOT - USER DATE/TIME
int	CHldAdapt::SetLoop_AdaptParam(long pcr_restamping, long continuity_conunter, long tdt_tot, char* user_date, char* user_time)
{
	TL_gRestamping = pcr_restamping & 0x01;
	TL_gContiuityCounter = continuity_conunter & 0x01;
	TL_gDateTimeOffset = tdt_tot & 0x03;
	sprintf(TL_gUserDate, "%s", user_date);
	sprintf(TL_gUserTime, "%s", user_time);

	TL_gPCR_Restamping = (pcr_restamping>>1) & 0x01; 

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
int	CHldAdapt::__Set_ErrInjection_Param(long error_lost, long error_lost_packet,
	long error_bits, long error_bits_packet, long error_bits_count,
	long error_bytes, long error_bytes_packet, long error_bytes_count)
{
	TL_ErrLost = error_lost;
	TL_ErrBits = error_bits;
	TL_ErrBytes = error_bytes;
	TL_ErrBitsCount = error_bits_count;
	TL_ErrBytesCount = error_bytes_count;
	TL_ErrLostPacket = error_lost_packet;
	TL_ErrBitsPacket = error_bits_packet;
	TL_ErrBytesPacket = error_bytes_packet;
	return 0;
}
int	CHldAdapt::IsUsrReqed_DateTime_or_Restamp(void)
{
	if ((TL_gRestamping == 1 || TL_gDateTimeOffset == 1 || TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3))
	{
		return	1;
	}
	return	0;
}
int	CHldAdapt::IsUsrReqed_DateTime(void)
{
	if ( TL_gDateTimeOffset == 1 || TL_gDateTimeOffset == 2 || TL_gDateTimeOffset == 3 )
	{
		return	1;
	}
	return	0;
}
int	CHldAdapt::IsUsrReqed_Restamp(void)
{
	if ( TL_gRestamping == 1 )
	{
		return	1;
	}
	return	0;
}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
int CHldAdapt::MakeCRC(unsigned char *Target, const unsigned char *Data, int Length)
{
  int crc = CRC32::crc32((const char *)Data, Length, 0xFFFFFFFF);
  int i = 0;
  Target[i++] = crc >> 24;
  Target[i++] = crc >> 16;
  Target[i++] = crc >> 8;
  Target[i++] = crc;
  //__BHwHLog__->HldPrint("new...CRC=0x%x, 0x%x, 0x%x, 0x%x\n\n", Target[0], Target[1], Target[2], Target[3]);
  return i;
}

//taken and adapted from libdtv, (c) Rolf Hakenes
// CRC32 lookup table for polynomial 0x04c11db7
unsigned int CRC32::crc_table[256] = {
   0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
   0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
   0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
   0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
   0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
   0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
   0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
   0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
   0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
   0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
   0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
   0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
   0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
   0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
   0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
   0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
   0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
   0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
   0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
   0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
   0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
   0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
   0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
   0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
   0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
   0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
   0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
   0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
   0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
   0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
   0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
   0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
   0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
   0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
   0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
   0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
   0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
   0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
   0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
   0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
   0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
   0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
   0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

unsigned int CRC32::crc32 (const char *d, int len, unsigned int crc)
{
   register int i;
   const unsigned char *u=(unsigned char*)d; // Saves '& 0xff'

   for (i=0; i<len; i++)
      crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *u++)];

   return crc;
}

CRC32::CRC32(const char *d, int len, unsigned int CRCvalue) {
   data=d;
   length=len;
   value=CRCvalue;
}

#define GP  0x1D5   /* x^8 + x^7 + x^6 + x^4 + x^2 + 1 */
#define DI  0xD5
//static unsigned char crc8_table[256];     /* 8-bit table */
//static int made_table=0;

//CRC8
unsigned char CRC8::crc8_table[256] = {
	0x00, 0xd5, 0x7f, 0xaa, 0xfe, 0x2b, 0x81, 
	0x54, 0x29, 0xfc, 0x56, 0x83, 0xd7, 
	0x02, 0xa8, 0x7d, 0x52, 0x87, 0x2d, 
	0xf8, 0xac, 0x79, 0xd3, 0x06, 0x7b, 
	0xae, 0x04, 0xd1, 0x85, 0x50, 0xfa, 
	0x2f, 0xa4, 0x71, 0xdb, 0x0e, 0x5a, 
	0x8f, 0x25, 0xf0, 0x8d, 0x58, 0xf2, 
	0x27, 0x73, 0xa6, 0x0c, 0xd9, 0xf6, 
	0x23, 0x89, 0x5c, 0x08, 0xdd, 0x77, 
	0xa2, 0xdf, 0x0a, 0xa0, 0x75, 0x21, 
	0xf4, 0x5e, 0x8b, 0x9d, 0x48, 0xe2, 
	0x37, 0x63, 0xb6, 0x1c, 0xc9, 0xb4, 
	0x61, 0xcb, 0x1e, 0x4a, 0x9f, 0x35, 
	0xe0, 0xcf, 0x1a, 0xb0, 0x65, 0x31, 
	0xe4, 0x4e, 0x9b, 0xe6, 0x33, 0x99, 
	0x4c, 0x18, 0xcd, 0x67, 0xb2, 0x39, 
	0xec, 0x46, 0x93, 0xc7, 0x12, 0xb8, 
	0x6d, 0x10, 0xc5, 0x6f, 0xba, 0xee, 
	0x3b, 0x91, 0x44, 0x6b, 0xbe, 0x14, 
	0xc1, 0x95, 0x40, 0xea, 0x3f, 0x42, 
	0x97, 0x3d, 0xe8, 0xbc, 0x69, 0xc3, 
	0x16, 0xef, 0x3a, 0x90, 0x45, 0x11, 
	0xc4, 0x6e, 0xbb, 0xc6, 0x13, 0xb9, 
	0x6c, 0x38, 0xed, 0x47, 0x92, 0xbd, 
	0x68, 0xc2, 0x17, 0x43, 0x96, 0x3c, 
	0xe9, 0x94, 0x41, 0xeb, 0x3e, 0x6a, 
	0xbf, 0x15, 0xc0, 0x4b, 0x9e, 0x34, 
	0xe1, 0xb5, 0x60, 0xca, 0x1f, 0x62, 
	0xb7, 0x1d, 0xc8, 0x9c, 0x49, 0xe3, 
	0x36, 0x19, 0xcc, 0x66, 0xb3, 0xe7, 
	0x32, 0x98, 0x4d, 0x30, 0xe5, 0x4f, 
	0x9a, 0xce, 0x1b, 0xb1, 0x64, 0x72, 
	0xa7, 0x0d, 0xd8, 0x8c, 0x59, 0xf3, 
	0x26, 0x5b, 0x8e, 0x24, 0xf1, 0xa5, 
	0x70, 0xda, 0x0f, 0x20, 0xf5, 0x5f, 
	0x8a, 0xde, 0x0b, 0xa1, 0x74, 0x09, 
	0xdc, 0x76, 0xa3, 0xf7, 0x22, 0x88, 
	0x5d, 0xd6, 0x03, 0xa9, 0x7c, 0x28, 
	0xfd, 0x57, 0x82, 0xff, 0x2a, 0x80, 
	0x55, 0x01, 0xd4, 0x7e, 0xab, 0x84, 
	0x51, 0xfb, 0x2e, 0x7a, 0xaf, 0x05, 
	0xd0, 0xad, 0x78, 0xd2, 0x07, 0x53, 
	0x86, 0x2c, 0xf9};

int CHldAdapt::MakeCRC8(unsigned char *Target, const unsigned char *Data, int Length)
{
	int i;

	CRC8 crc_8;
//	crc_8.init_crc8();
	 
	*Target = 0;//Data[0];
	for ( i = 0; i < Length; i++ )
	{
		crc_8.crc8(Target, Data[i]);
	}
  
	//__BHwHLog__->HldPrint("new...CRC=0x%x, 0x%x, 0x%x, 0x%x\n\n", Target[0], Target[1], Target[2], Target[3]);
	return i;
}

/*
* Should be called before any other crc function.  
*/
void CRC8::init_crc8()
{
	int i,j;
	unsigned char crc;
	
	if (made_table != 1)
	{
		for (i=0; i<256; i++)
		{
			crc = i;
			for (j=0; j<8; j++)
				crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
			crc8_table[i] = crc & 0xFF;
		}
		made_table=1;
	}
}

/*
* For a byte array whose accumulated crc value is stored in *crc, computes
* resultant crc obtained by appending m to the byte array
*/
void CRC8::crc8(unsigned char *crc, unsigned char m)
{
//	if (!made_table)
//		init_crc8();
	*crc = crc8_table[(*crc) ^ m];
	*crc &= 0xFF;
}




