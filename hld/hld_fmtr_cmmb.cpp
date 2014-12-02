
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
#include	"../include/lld_const.h"
#endif
#include	"../include/hld_structure.h"
#include	"hld_fmtr_cmmb.h"

#define MFS_INFO_PATH		"mfs_info"

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrCmmb::CHldFmtrCmmb(void)
{
	//fp = fopen("C:\\CMMB_CMCT_INFO.txt", "w");
	fp = NULL;
}

CHldFmtrCmmb::~CHldFmtrCmmb()
{
}
void	CHldFmtrCmmb::SetCommonMethod_6(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void CHldFmtrCmmb::InitCmmbVariables_OnPlayStart(void)
{
	m_nRemaindedBlockSize = 0;
	m_nStartOfMultiplexFrame0 = 0;
	m_nStartOfMultiplexFrame1 = 0;
	
}
int CHldFmtrCmmb::ChkCmmbPlayFile_OnPlayCont(void)
{
#if defined(WIN32)	
	if (__FIf__->AP_hFile == INVALID_HANDLE_VALUE)
	{
		__FIf__->ChkAndOpenPlayFile_OnPlayCont();
		return 0;
	}

#else
	if ( (FILE*)__FIf__->AP_hFile == NULL )
	{
		__FIf__->ChkAndOpenPlayFile_OnPlayCont();
		return 0;
	}
#endif

	return	1;
}
int CHldFmtrCmmb::PlaybackCmmbFile_OnPlayCont__HasEndlessWhile(void)
{
	int		nBankBlockSize = __FIf__->SdramSubBankSize();

	if ( __Sta__->IsModTyp_Cmmb() )
	{
		__FIf__->next_file_ready = FALSE;
		if (ChkCmmbPlayFile_OnPlayCont() == 0)
		{
			return 0;
		}
		if (__FIf__->Fseek_Current_Offset_UserReq() == 0)
		{
		}
		else
		{
			__FIf__->Fseek_SectionRepeat_UserReq();
		}

		__FIf__->TL_dwBytesRead = 0;
		__FIf__->MapTvbBdDestAddr_fromBnkId();

		__FIf__->Fread_into_PlayBuf_StartingPos();
		__FIf__->IsEof();

		__FIf__->FillDmaSrcDta_PlayDirection(__FIf__->TL_szBufferPlay, nBankBlockSize);
		__FIf__->StartDmaTransfer_Play(nBankBlockSize);

		__FIf__->IncBankAddrHwDest_Sdram(__FIf__->GetPlayParam_PlayRate());

//_exit_eof:
		return 0;
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
void CHldFmtrCmmb::TL_FindStartOfMultiplexFrame(void)	//	TL_sz2ndBufferPlay
{
#if	1
	__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "TL_FindStartOfMultiplexFrame : not used");
#else
	int nBankBlockSize = (__FIf__->SdramSubBankSize());
	int i;
	unsigned long lTemp = 0;

	unsigned char *szBuf, szCRC[4];
	int x = m_nStartOfMultiplexFrame1;//0;
	int LengthOfMultiplexFrameHeader = 0;
	int IndicatorOfMultiplexFrame = -1;
	int NumberOfMultiplexSubFrame = 0;
	int StartingOfMultiplexFrame = -1;
	int ValidCRC = 1;

	while ( TL_nBCount > 0 )
	{
		if (TL_nRIndex <= (unsigned int)(MAX_BUFFER_BYTE_SIZE - nBankBlockSize))
		{
			memcpy(TL_pbBufferTmp+m_nRemaindedBlockSize, TL_szBufferPlay + TL_nRIndex, nBankBlockSize);
		}
		else 
		{
			lTemp = MAX_BUFFER_BYTE_SIZE-TL_nRIndex;

			memcpy(TL_pbBufferTmp+m_nRemaindedBlockSize, TL_szBufferPlay + TL_nRIndex, lTemp);
			memcpy(TL_pbBufferTmp+m_nRemaindedBlockSize+lTemp, TL_szBufferPlay, nBankBlockSize - lTemp);
		}

		TL_nRIndex += nBankBlockSize;
		if (TL_nRIndex >= MAX_BUFFER_BYTE_SIZE)
			TL_nRIndex -= MAX_BUFFER_BYTE_SIZE;
		
		TL_nBCount -= nBankBlockSize;

#if 1
		szBuf = TL_pbBufferTmp;
		while ( x < nBankBlockSize+m_nRemaindedBlockSize - 3 )
		{
			///////////////
			ValidCRC = MultipexFrameHeader(szBuf, nBankBlockSize+m_nRemaindedBlockSize, 
				&x, &IndicatorOfMultiplexFrame, &NumberOfMultiplexSubFrame, &LengthOfMultiplexFrameHeader, &StartingOfMultiplexFrame);
			//sprintf(debug_string, "x=%d, MultipexFrameHeader::indi=%d,numOfsubframe=%d,len=%d,StartingOfMultiplexFrame=%d\n", 
			//	x, IndicatorOfMultiplexFrame, NumberOfMultiplexSubFrame, LengthOfMultiplexFrameHeader,StartingOfMultiplexFrame);
			//OutputDebugString(debug_string);
			if ( NumberOfMultiplexSubFrame == 1 )
			{
				//sprintf(debug_string, "LengthOfSubFrame=%d\n", LengthOfSubFrame[0]);
				//OutputDebugString(debug_string);
			}
			if ( ValidCRC != 1 )//Invalid CRC or not found of StartingOfMultiplexFrame
			{
				m_nRemaindedBlockSize = nBankBlockSize+m_nRemaindedBlockSize - StartingOfMultiplexFrame;
				memcpy(TL_pbBufferTmp, TL_pbBufferTmp + StartingOfMultiplexFrame, m_nRemaindedBlockSize);
				m_nStartOfMultiplexFrame1 = 0;
				break;
			}
			
			for ( i = 0; i < NumberOfMultiplexSubFrame; i++ )
			{
				x += LengthOfSubFrame[i];
			}

			if ( x >= nBankBlockSize+m_nRemaindedBlockSize )
			{
				m_nRemaindedBlockSize = nBankBlockSize+m_nRemaindedBlockSize - StartingOfMultiplexFrame;
				memcpy(TL_pbBufferTmp, TL_pbBufferTmp + StartingOfMultiplexFrame, m_nRemaindedBlockSize);
				m_nStartOfMultiplexFrame1 = 0;
				break;
			}

			///////////////
			m_nStartOfMultiplexFrame0 = StartingOfMultiplexFrame;

			while ( x < nBankBlockSize+m_nRemaindedBlockSize - 3 )
			{
				lTemp = szBuf[x]*(256^3);
				lTemp +=  szBuf[x+1]*(256^2);
				lTemp +=  szBuf[x+2]*256;
				lTemp +=  szBuf[x+3];
				if ( lTemp == 1 )
				{
					break;
				}
				++x;
			}

			if ( x == nBankBlockSize+m_nRemaindedBlockSize - 3 )//not found of StartingOfMultiplexFrame
			{
				m_nRemaindedBlockSize = nBankBlockSize+m_nRemaindedBlockSize - StartingOfMultiplexFrame;
				memcpy(TL_pbBufferTmp, TL_pbBufferTmp + StartingOfMultiplexFrame, m_nRemaindedBlockSize);
				m_nStartOfMultiplexFrame1 = 0;
				break;
			}

			m_nStartOfMultiplexFrame1 = x;

			i = 0;
			TL_pbBuffer[i] = 0x00;
			TL_pbBuffer[++i] = 0x00;
			TL_pbBuffer[++i] = 0x00;
			TL_pbBuffer[++i] = 0x01;
			TL_pbBuffer[++i] = 0x0F;
			TL_pbBuffer[++i] = 0x08;
			//Indicator of multiplex frme : b'000000
			TL_pbBuffer[++i] = 0x40;
			TL_pbBuffer[++i] = 0x9d;
			TL_pbBuffer[++i] = 0xFC;
			TL_pbBuffer[++i] = 0xE0;
			TL_pbBuffer[++i] = 0x0F;
			TL_pbBuffer[++i] = 0x01;
			TL_pbBuffer[++i] = 0x00;
			TL_pbBuffer[++i] = 0x00;
			TL_pbBuffer[++i] = 0x15;

			MakeCRC(szCRC, TL_pbBuffer, i+1);
			TL_pbBuffer[++i] = szCRC[0];
			TL_pbBuffer[++i] = szCRC[1];
			TL_pbBuffer[++i] = szCRC[2];
			TL_pbBuffer[++i] = szCRC[3];
			
			lTemp = i+1;
			TL_pbBuffer[++i] = 0x02;
			TL_pbBuffer[++i] = 0x14;
			TL_pbBuffer[++i] = 0xC0;
			//number of multiplex frame in a broadcasting channel frame
			TL_pbBuffer[++i] = 0x02;

			//RS=240/240
			TL_pbBuffer[++i] = 0x00;
			//Byte crossing=Mode1, LDPC=1/2, Modulation=BPSK
			TL_pbBuffer[++i] = 0x40;
			//number of time slot=1
			TL_pbBuffer[++i] = 0x01;
			TL_pbBuffer[++i] = 0x00;
			//number of multiplex subframe
			TL_pbBuffer[++i] = 0x01;
			TL_pbBuffer[++i] = 0x10;
			TL_pbBuffer[++i] = 0xFF;
			TL_pbBuffer[++i] = 0xFF;

			//RS=176/240
			TL_pbBuffer[++i] = 0x07;
			//Byte crossing=Mode1, LDPC=1/2, Modulation=16QAM
			TL_pbBuffer[++i] = 0x48;
			//number of time slot=2
			TL_pbBuffer[++i] = 0x02;
			TL_pbBuffer[++i] = 0x04;
			TL_pbBuffer[++i] = 0x08;
			//number of multiplex subframe
			TL_pbBuffer[++i] = 0x01;
			TL_pbBuffer[++i] = 0x10;
			TL_pbBuffer[++i] = 0x00;
			TL_pbBuffer[++i] = 0xFE;

			MakeCRC(szCRC, TL_pbBuffer+lTemp, i+1-lTemp);
			TL_pbBuffer[++i] = szCRC[0];
			TL_pbBuffer[++i] = szCRC[1];
			TL_pbBuffer[++i] = szCRC[2];
			TL_pbBuffer[++i] = szCRC[3];

#if 0
			++i;
			if (TL_nWritePos <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - i)))
			{
				memcpy(TL_sz2ndBufferPlay + TL_nWritePos, TL_pbBuffer, i);
				TL_nWritePos += i;
			}
			else 
			{
				lTemp = MAX_BUFFER_BYTE_SIZE-TL_nWritePos;

				memcpy(TL_sz2ndBufferPlay + TL_nWritePos, TL_pbBuffer, lTemp);
				memcpy(TL_sz2ndBufferPlay, TL_pbBuffer + lTemp, i - lTemp);

				TL_nWritePos += i;
				if (TL_nWritePos >= MAX_BUFFER_BYTE_SIZE)
					TL_nWritePos -= MAX_BUFFER_BYTE_SIZE;
			}
			TL_nBufferCnt += i;
#endif

			///////////////
			i = m_nStartOfMultiplexFrame1 - m_nStartOfMultiplexFrame0;
			if (TL_nWritePos <= (unsigned int)((MAX_BUFFER_BYTE_SIZE - i)))
			{
				memcpy(TL_sz2ndBufferPlay + TL_nWritePos, TL_pbBufferTmp + m_nStartOfMultiplexFrame0, i);
				TL_nWritePos += i;
			}
			else 
			{
				lTemp = MAX_BUFFER_BYTE_SIZE-TL_nWritePos;

				memcpy(TL_sz2ndBufferPlay + TL_nWritePos, TL_pbBufferTmp + m_nStartOfMultiplexFrame0, lTemp);
				memcpy(TL_sz2ndBufferPlay, TL_pbBufferTmp + m_nStartOfMultiplexFrame0 + lTemp, i - lTemp);

				TL_nWritePos += i;
				if (TL_nWritePos >= MAX_BUFFER_BYTE_SIZE)
					TL_nWritePos -= MAX_BUFFER_BYTE_SIZE;
			}
			TL_nBufferCnt += i;
			
			//sprintf(debug_string, "m_nStartOfMultiplexFrame %d, %d\n", m_nStartOfMultiplexFrame0, m_nStartOfMultiplexFrame1);
			//OutputDebugString(debug_string);

			if ( IndicatorOfMultiplexFrame < 0 )
			{
				continue;
			}
		}
#endif

	}
#endif
}
void CHldFmtrCmmb::StreamAnalyzer(unsigned char* szBuf, int nlen, long* bitrate, int* pNumberOfMultiplexFrame)
{
#if 1
	int i, j, x = 0;
	unsigned long tmp;

	int LengthOfMultiplexFrameHeader = 0;
	int IndicatorOfMultiplexFrame = -1;
	int NumberOfMultiplexSubFrame = 0;
	int StartingOfMultiplexFrame = -1;
	//int LengthOfSubFrame[0x0F];
	int CRC_32 = -1;

	int TableIdentifier = -1;
	int NumberOfMultiplexFrame = -1;
	//int Parameters[0x3F][6];

//2012/2/10 TEST
//	int MultiplexFrameCnt = 0;
	//fixed - nlen -> nlen-3
	while ( x < nlen-3 )
	{
//2012/2/10 TEST
//		MultiplexFrameCnt++;

		CRC_32 = MultipexFrameHeader(szBuf, nlen, &x, 
			&IndicatorOfMultiplexFrame, 
			&NumberOfMultiplexSubFrame, 
			&LengthOfMultiplexFrameHeader,
			&StartingOfMultiplexFrame);
		//2012/3/19
		if(CRC_32 == 0)
		{
			*bitrate = -2;
			return;
		}
		
		//sprintf(debug_string, "x=%d, MultipexFrameHeader::indi=%d,numOfsubframe=%d,len=%d,StartingOfMultiplexFrame=%d\n", 
		//	x, IndicatorOfMultiplexFrame, NumberOfMultiplexSubFrame, LengthOfMultiplexFrameHeader,StartingOfMultiplexFrame);
		//OutputDebugString(debug_string);

		//Control Information
		if ( IndicatorOfMultiplexFrame == 0 )
		{
			//TEST
			/*
			HldPrint("\n");
			tmp = x;
			for ( i = 0; i < NumberOfMultiplexSubFrame; i++ )
			{
				for ( j = 0; j < LengthOfSubFrame[i]; j++ )
				{
					HldPrint("%02x ", szBuf[tmp+j]);
				}
				HldPrint("\n");
				tmp += LengthOfSubFrame[i];
			}
			*/
			
			for ( i = 0; i < NumberOfMultiplexSubFrame; i++ )
			{
				//Table Identifier
				TableIdentifier = szBuf[x];
				x += 1;

				//CMCT
				if ( TableIdentifier == 2 )
				{
					MultiplexConfiguration(szBuf, nlen, &x,	&NumberOfMultiplexFrame);
				}
				else
				{
					x += (LengthOfSubFrame[i] - 1);
				}
			}
		}
		//Service Data
		else
		{
			for ( i = 0; i < NumberOfMultiplexSubFrame; i++ )
			{
			//	sprintf(debug_string, "LengthOfSubFrame[%d]=%d\n", i, LengthOfSubFrame[i]);
			//	OutputDebugString(debug_string);
			
				x += LengthOfSubFrame[i];
			}
		}

		//fixed
		if ( IndicatorOfMultiplexFrame < 0 )
		{
//2012/2/10 TEST
//			MultiplexFrameCnt--;
			continue;
		}

		tmp = 0;
		for ( j = 0; j < NumberOfMultiplexSubFrame; j++ )
		{
			tmp += LengthOfSubFrame[j];
		}
		Parameters[IndicatorOfMultiplexFrame][6] = LengthOfMultiplexFrameHeader + tmp + 4;
		Parameters[IndicatorOfMultiplexFrame][7] = IndicatorOfMultiplexFrame;

		//sprintf(debug_string, "IndicatorOfMultiplex[%d], NumberOfMultiplexFrame[%d]\n", IndicatorOfMultiplexFrame, NumberOfMultiplexFrame);
		//OutputDebugString(debug_string);
		if ( IndicatorOfMultiplexFrame == (NumberOfMultiplexFrame - 1) ) 
//2012/2/10 TEST
//		if ( MultiplexFrameCnt == NumberOfMultiplexFrame)
		{
			*pNumberOfMultiplexFrame = NumberOfMultiplexFrame;
			*bitrate = 0;
			for ( j = 0; j < NumberOfMultiplexFrame; j++ )
			{
				//orginal
				//*bitrate += Parameters[j][6];//???
				if(j == Parameters[j][7])
					*bitrate += Parameters[j][6];
			}
			*bitrate *= 8;
			
			//sskim20091005
//2012/2/10 TEST
//			MultiplexFrameCnt = 0;
			break;
		}
	}

#else
#endif
}

int CHldFmtrCmmb::MultipexFrameHeader(unsigned char* szBuf, int len, int *pos,
							   int *IndicatorOfMultiplexFrame, 
							   int *NumberOfMultiplexSubFrame, 
							   int *LengthOfMultiplexFrameHeader,
							   int *StartingOfMultiplexFrame)
{
	int i=*pos, x=0;
	unsigned long tmp;
	//int StartingOfMultiplexFrame = -1;
	int IndicatorOfNextFrameParameter = -1;
	int TotalLengthOfSubFrame = 0;
	int CRC_32 = -1;

	//Start of Multiplex frame
	*StartingOfMultiplexFrame = -1;
	//for ( i = 0; i < len-3; i++ )
	for ( ; i < len-3; i++ )
	{
		tmp = szBuf[i]*(256^3);
		tmp +=  szBuf[i+1]*(256^2);
		tmp +=  szBuf[i+2]*256;
		tmp +=  szBuf[i+3];
		
		if ( tmp == 1 )
		{
			*StartingOfMultiplexFrame = i;
			//sprintf(debug_string, "StartingOfMultiplexFrame=%d\n", *StartingOfMultiplexFrame);
			//OutputDebugString(debug_string);
			break;
		}
	}

	if ( *StartingOfMultiplexFrame < 0 )
	{
		//fixed - 0 -> -1
		*IndicatorOfMultiplexFrame = -1;
		*NumberOfMultiplexSubFrame = 0;
		*LengthOfMultiplexFrameHeader = 0;
		*StartingOfMultiplexFrame = -1;
		
		*pos = i;
		return -1;
	}
	x = *StartingOfMultiplexFrame;
	x += 4;
	
	//Length Of Multiplex Frame Header
	*LengthOfMultiplexFrameHeader = szBuf[x];
	x += 2;

	//TEST
	/*
	HldPrint("\n");
	for(i = *StartingOfMultiplexFrame; i < *LengthOfMultiplexFrameHeader+4; i++)
	{
		HldPrint("%02x ", szBuf[i]);
	}
	HldPrint("\n");
	*/

	//Indicator of Multiplex Frame
	*IndicatorOfMultiplexFrame = szBuf[x] & 0x3F;
	x += 1;

	//Indicator of Next Frame Parameter
    IndicatorOfNextFrameParameter = (szBuf[x]>>5) & 0x01;
	x += 4;

	//Number Of Multiplex Sub-Frame
	*NumberOfMultiplexSubFrame = szBuf[x] & 0x0F;
	x += 1;
	
	//(Total)Length of Sub-Frame
	TotalLengthOfSubFrame = 0;
	for ( i = 0; i < *NumberOfMultiplexSubFrame; i++ )
	{
		tmp =  szBuf[x]*(256^2);
		tmp +=  szBuf[x+1]*256;
		tmp +=  szBuf[x+2];	
		TotalLengthOfSubFrame += tmp;

		x += 3;

		LengthOfSubFrame[i] = tmp;//???
	}
	
	//TEST
	unsigned char szCRC[4];
	__FIf__->MakeCRC(szCRC, szBuf+(*StartingOfMultiplexFrame), x - (*StartingOfMultiplexFrame));

	//CRC_32
	tmp = szBuf[x]*(256^3);
	tmp +=  szBuf[x+1]*(256^2);
	tmp +=  szBuf[x+2]*256;
	tmp +=  szBuf[x+3];
	CRC_32 = tmp;
	
	if ( szBuf[x] != szCRC[0] || szBuf[x+1] != szCRC[1] || szBuf[x+2] != szCRC[2] || szBuf[x+3] != szCRC[3] )
	{
		CRC_32 = 0;
	}

	x += 4;

	*pos = x;


	return (CRC_32 == 0 ? 0 : 1);
}
void CHldFmtrCmmb::MultiplexConfiguration(unsigned char* szBuf, int len, int *pos, int *NumberOfMultiplexFrame)
{
	unsigned char szBuf1;
	int j, x = *pos, M2;
	unsigned long tmp;

	int FrequencyNo = -1;
	int MultiplexConfigUpdateSequenceNo = -1;
	int MultiplexFrameIdentifer = -1;
	int RsCodingRate = -1;
	int ByteCrossingMode = -1;
	int LdpcEncodingRate = -1;
	int ModulationMethod = -1;
	int ScrambleMethod = -1;
	int NumberOfTimeSlot = 0;
	int NumberOfMultiplexSubframe = 0;
	
	int CRC_32 = -1;

	//2012/3/19 TEST
	int __strlen = 0;

	//Frequency No.
	FrequencyNo = szBuf[x];
	x += 1;
	
	if(fp != NULL)
	{
		sprintf(debug_string, "\n\n========CMCT Start=========\n");
		__strlen = strlen(debug_string);
		fwrite(debug_string,1,__strlen, fp);
	}
	//Multiplex Config Update Sequence No.
	MultiplexConfigUpdateSequenceNo = (szBuf[x]>>4) & 0x0F;
	x += 1;

	//Number Of Multiplex Frame
	*NumberOfMultiplexFrame = szBuf[x] & 0x3F;
	x += 1;
	if(fp != NULL)
	{
		sprintf(debug_string, "NumberOfMultiplexFrame : %d\n", *NumberOfMultiplexFrame);
		__strlen = strlen(debug_string);
		fwrite(debug_string,1,__strlen, fp);
	}

	for ( j = 0; j < *NumberOfMultiplexFrame; j++ )
	{
		//Multiplex Frame Identifer
		MultiplexFrameIdentifer = (szBuf[x]>>2) & 0x3F;
		if(fp != NULL)
		{
			sprintf(debug_string, "MultiplexFrameIdentifer : %d\n", MultiplexFrameIdentifer);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//RS Coding Rate
		RsCodingRate = szBuf[x] & 0x03;
		x += 1;
		if(fp != NULL)
		{
			sprintf(debug_string, "RsCodingRate : %d\n", RsCodingRate);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//Byte Crossing Mode
		ByteCrossingMode = (szBuf[x]>>6) & 0x03;
		if(fp != NULL)
		{
			sprintf(debug_string, "ByteCrossingMode : %d\n", ByteCrossingMode);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//Ldpc Encoding Rate
		LdpcEncodingRate = (szBuf[x]>>4) & 0x03;
		if(fp != NULL)
		{
			sprintf(debug_string, "LdpcEncodingRate : %d\n", LdpcEncodingRate);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//Modulation Method
		ModulationMethod = (szBuf[x]>>2) & 0x03;
		szBuf1 = szBuf[x];
		x += 1;
		if(fp != NULL)
		{
			sprintf(debug_string, "ModulationMethod : %d\n", ModulationMethod);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//Scramble Method
		ScrambleMethod = ((szBuf1 & 0x01) << 2) + ((szBuf[x]>>6) & 0x03);
		if(fp != NULL)
		{
			sprintf(debug_string, "ScrambleMethod : %d\n", ScrambleMethod);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		//Number Of Time Slot
		NumberOfTimeSlot = szBuf[x] & 0x3F;
		x += 1;
		if(fp != NULL)
		{
			sprintf(debug_string, "NumberOfTimeSlot : %d\n", NumberOfTimeSlot);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}
		///*
		for ( M2 = 0; M2 < NumberOfTimeSlot; M2++ )
		{
			if(fp != NULL)
			{
				sprintf(debug_string, "Time Slot No. : %d\n", ((szBuf[x + M2] & 0xFC) >> 2));
				__strlen = strlen(debug_string);
				fwrite(debug_string,1,__strlen, fp);
			}
			//TimeSlotNo[M2] = szBuf[x] & 0x3F;
			//x += 1;

			//sprintf(debug_string, "Slot # = %d\n", (szBuf[x+M2]>>2) & 0x3F);
			//OutputDebugString(debug_string);
		}
		//*/
		x += NumberOfTimeSlot;

		//Number Of Multiplex Sub-frame
		NumberOfMultiplexSubframe = szBuf[x] & 0x0F;
		x += 1;
		if(fp != NULL)
		{
			sprintf(debug_string, "NumberOfMultiplexSubframe : %d\n", NumberOfMultiplexSubframe);
			__strlen = strlen(debug_string);
			fwrite(debug_string,1,__strlen, fp);
		}

		for ( M2 = 0; M2 < NumberOfMultiplexSubframe; M2++ )
		{
			if(fp != NULL)
			{
				sprintf(debug_string, "MultiplexSubframe No. : %d\n", (szBuf[x] & 0x0F));
				__strlen = strlen(debug_string);
				fwrite(debug_string,1,__strlen, fp);
			}
			//Multiplex Sub-frame No.
			//MultiplexSubframeNo[M2] = szBuf[x] & 0x0F;
			x += 1;

			tmp =  szBuf[x]*256;
			tmp +=  szBuf[x+1];
			//ServiceIdentifier[M2] = tmp;
			if(fp != NULL)
			{
				sprintf(debug_string, "ServiceIdentifier : 0x%x\n", tmp);
				__strlen = strlen(debug_string);
				fwrite(debug_string,1,__strlen, fp);
			}

			x += 2;
		}

		Parameters[j][0] = RsCodingRate;
		Parameters[j][1] = ByteCrossingMode;
		Parameters[j][2] = LdpcEncodingRate;
		Parameters[j][3] = ModulationMethod;
		Parameters[j][4] = ScrambleMethod;
		Parameters[j][5] = NumberOfTimeSlot;

		double bitrate = 9216*15.;
		if ( RsCodingRate == 0 ) bitrate *= (240./240.);
		else if ( RsCodingRate == 1 ) bitrate *= (224./240.);
		else if ( RsCodingRate == 2 ) bitrate *= (192./240.);
		else if ( RsCodingRate == 3 ) bitrate *= (176./240.);

		if ( LdpcEncodingRate == 0 ) bitrate *= (1./2.);
		else if ( LdpcEncodingRate == 1 ) bitrate *= (3./4.);

		if ( ModulationMethod == 0 ) bitrate *= 1.;
		else if ( ModulationMethod == 1 ) bitrate *= 2.;
		else if ( ModulationMethod == 2 ) bitrate *= 4.;

		bitrate *= NumberOfTimeSlot;

		//sprintf(debug_string, "MultiplexConfiguration(%d)::time=%d,Subframe#=%d,bitrate=%f\n", j, NumberOfTimeSlot, NumberOfMultiplexSubframe, bitrate);
		//OutputDebugString(debug_string);
	}

	//CRC_32
	tmp = szBuf[x]*(256^3);
	tmp +=  szBuf[x+1]*(256^2);
	tmp +=  szBuf[x+2]*256;
	tmp +=  szBuf[x+3];
	CRC_32 = tmp;
	x += 4;

	*pos = x;
}

int	CHldFmtrCmmb::RunMfs_CmmbParser(char *szFile, char *szResult)
{
	FILE *fp;
	unsigned char *szBuf;
	long bitrate;
	int NumberOfMultiplexFrame, j;
	int numread;
	
	fp = fopen(szFile, "rb");
	if ( !fp )
	{
		return -1;
	}
	
	szBuf = (unsigned char *)malloc(SUB_BANK_MAX_BYTE_SIZE*sizeof(unsigned char));
	if ( !szBuf )
	{
		fclose(fp);
		return -2;
	}
	numread = fread(szBuf, 1, SUB_BANK_MAX_BYTE_SIZE, fp);
	fclose(fp);

	StreamAnalyzer(szBuf, numread, &bitrate, &NumberOfMultiplexFrame);
	free(szBuf);
//2010/10/8
//#ifdef WIN32
//#else
	if(NumberOfMultiplexFrame > 4096)
		return 0;
//#endif
	for ( j = 0; j < NumberOfMultiplexFrame; j++ )
	{
//2012/2/10 TEST
//		if(j != Parameters[j][7])
//			continue;
		sprintf(debug_string, "%d %d %d %d %d %d %d %d ", 
			Parameters[j][0], 
			Parameters[j][1], 
			Parameters[j][2], 
			Parameters[j][3], 
			Parameters[j][4], 
			Parameters[j][5],
			Parameters[j][6],
			Parameters[j][7]);
		strcat(szResult, debug_string);
		{
			strcat(szResult, ",");
		}
	}

	return 0;
}






