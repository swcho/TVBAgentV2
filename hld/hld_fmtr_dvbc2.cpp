#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<math.h>

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_fmtr_dvbc2.h"
#include	"dvbc2_multipleplp.h"

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrDvbC2_::CHldFmtrDvbC2_(void)
{
#if defined(WIN32)
	TL_C2MI_hMutex = INVALID_HANDLE_VALUE;
#else
#endif

	TL_C2MI_PumpingThreadDone = 1;
	TL_C2MI_ConvertingThreadDone = 1;
	
}

CHldFmtrDvbC2_::~CHldFmtrDvbC2_()
{
#if defined(WIN32)	
	if ( TL_C2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_C2MI_hMutex);
		TL_C2MI_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutex_destroy(&TL_C2MI_hMutex);
#endif
}
void	CHldFmtrDvbC2_::SetCommonMethod_C2(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void	CHldFmtrDvbC2_::CreateC2miMutex(void)
{
#if defined(WIN32)
	TL_C2MI_hMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_C2MI_hMutexAttr);
	pthread_mutexattr_settype(&TL_C2MI_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_C2MI_hMutex, &TL_C2MI_hMutexAttr);
#endif

}
void	CHldFmtrDvbC2_::DestroyC2miMutex(void)
{
#if defined(WIN32)
	if ( TL_C2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_C2MI_hMutex);
		TL_C2MI_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_C2MI_hMutexAttr);
	pthread_mutex_destroy(&TL_C2MI_hMutex);
#endif

}
void	CHldFmtrDvbC2_::LockC2miMutex(void)
{
#if defined(WIN32)
	if ( TL_C2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_C2MI_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_C2MI_hMutex);
#endif
}
void	CHldFmtrDvbC2_::UnlockC2miMutex(void)
{
#if defined(WIN32)
	if ( TL_C2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_C2MI_hMutex);
	}
#else
	pthread_mutex_unlock(&TL_C2MI_hMutex);
#endif
}
int	CHldFmtrDvbC2_::DummyWaitDvbC2_OnPlayCont__HasEndlessWhile(void)
{
#if	1
	__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "DummyWaitDvbC2_OnPlayCont__HasEndlessWhile : not used");
#else
	int		nBankBlockSize = (iHW_BANK_OFFSET << 10);

	//FIXED - DVB-C2 TS -> C2MI
	if ( IsModTyp_DvbC2() && CheckExtention(PlayParm.AP_lst.szfn, (char*)".c2") == 0 )
	{
		UnlockC2miMutex();

		/* EOF */
		if (TL_dwBytesRead < (unsigned int)nBankBlockSize)
		{
			//FIXED - Bitrate adjustment
			if ( gfRatio > 1 )
			{
				if ( FEOF(AP_hFile) && TL_nNBCount < (unsigned int)nBankBlockSize )
				{
					SetMainTask_LoopState_(TH_END_PLAY);
				}
			}
			else
			{
				SetMainTask_LoopState_(TH_END_PLAY);
			}
		}

		while ( TL_nBufferCnt >= SUB_BANK_MAX_BYTE_SIZE*2 )
		{
			if ( ReqedNewAction_User() ) 
				break;

			Sleep(10);
		}

		return 0;
	}
#endif
	return	1;
}

//DVB-C2 - Multi PLP 4096->8192->65526
#define CHK_DVB_C2_PARAM if((strlen(szResult) + strlen(debug_string)) >= 65526) {szResult = NULL;free(fid_c2mi);free(Packet);free(szBuf);return 0;}
#define GET_DVB_C2_PARA(a,b) a = 0;	for ( kkk = 0; kkk < b; kkk++ )	{if ( kk < 0 ) {k += 1; kk = 7;} a += (((Packet[k]>>kk)&0x01)<<(b-kkk-1)); kk -= 1;}
double CHldFmtrDvbC2_::RunC2MiParser(char *szFile, char *szResult)
{
	FILE *fp;
	unsigned char *szBuf;
	int	sync_pos = -1;
	int	ts_packet_len = 0;
	int	index, numread;

	if ( __FIf__->IsC2miFile(szFile) == 0 )
	{
		memset(szResult, 0x00, 65526);
		return 0;
	}

	BYTE *bData;
	int CntByte = 1, CntFrame = 0, CntTsPacket = 0;
	int c2mi_len = 0, read_len = 0, LenStuff, LenPacket, count1, count2, count3, c2mi_total_len = 0, find_Pusi = 0, c2mi_analysis_L1_current=0, c2mi_analysis_timestamp=0, i, k, kk, kkk;
	unsigned char Sync, Pusi, Afe;
	unsigned char *fid_c2mi;
	unsigned char *RdData;
	unsigned char Header[6], Crc32[4], *Packet;

	int INTL_FRAME_START, NORMAL_MODE, NULL_PACKET_DELETION = 0;

	int FFT_SIZE = 0;
	int L1CONF_LEN = 0, L1DYN_CURR_LEN = 0, L1EXT_LEN = 0;
	int TYPE, BWT_EXT, S1, S2, L1_REPETITION_FLAG, GUARD_INTERVAL, PAPR, L1_MOD, L1_COD, L1_FEC_TYPE, L1_POST_SIZE, L1_POST_INFO_SIZE;
	int TX_ID_AVAILABILITY, CELL_ID, NETWORK_ID, C2_SYSTEM_ID;
	int PILOT_PATTERN, MUM_C2_FRAMES, NUM_DATA_SYMBOLS, REGEN_FLAG, L1_POST_EXTENTION, NUM_RF, CURRENT_RF_IDX, RESERVED;
	int SUB_SLICES_PER_FRAME, NUM_PLP, NUM_AUX, AUX_CONFIG_RFU, AUX_STREAM_TAG, AUX_PRIVATE_CONF;
	int RF_IDX;
	unsigned int FREQUENCY;
	int FEF_TYPE, FEF_LENGTH, FEF_INTERVAL;
	int PLP_ID, PLP_TYPE, PLP_PAYLOAD_TYPE, FF_FLAG, FIRST_RF_IDX, FIRST_FRAME_IDX, PLP_GROUP_ID, PLP_COD, PLP_MOD, PLP_ROTATION, PLP_FEC_TYPE, PLP_NUM_BLOCKS_MAX, FRAME_INTERVAL, TIME_IL_LENGTH, TIME_IL_TYPE, RESERVED_1;
	int RESERVED_2;
	int IN_BAND_A_FLAG, IN_BAND_B_FLAG, PLP_MODE, STATIC_FLAG, STATIC_PADDING_FLAG;

	////// Dynamic L1-post signaling
	int FRAME_IDX;
	int SUB_SLICE_INTERVAL;
	int TYPE_C2_START;
	int L1_CHANGE_COUNTER;
	int START_RF_IDX;
	int PLP_START;
	int PLP_NUM_BLOCKS;
	int RESERVED_3;
	int AUX_PRIVATE_DYN;
	
	int bw;
	double avg_bitrate = 0;
	char szInfo[64];

	fp = fopen(szFile, "rb");
	if ( !fp )
	{
		return -1;
	}
	
	szBuf = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE);
	if ( !szBuf )
	{
		fclose(fp);
		return -2;
	}

	numread = fread(szBuf, 1, MAX_BUFFER_BYTE_SIZE, fp);
	fclose(fp);

	sync_pos = __FIf__->TL_SyncLockFunction((char*)szBuf, numread, &ts_packet_len, numread, 3);
	if ( sync_pos == -1 )
	{
		szResult = NULL;
		free(szBuf);
		return 0;
	}

	fid_c2mi = (unsigned char*)malloc(MAX_BUFFER_BYTE_SIZE);
	if ( !fid_c2mi )
	{
		szResult = NULL;
		free(szBuf);
		return 0;
	}

	Packet = (unsigned char*)malloc(MAX_BUFFER_BYTE_SIZE);
	if ( !Packet )
	{
		szResult = NULL;
		free(fid_c2mi);
		free(szBuf);
		return 0;
	}
	
	index = 0;
	bData = szBuf+sync_pos;

	//1600 -> 3200 -> 5577 -> 16731 -> (numread/ts_packet_len)
	for ( CntTsPacket = 0; CntTsPacket < (int)(numread/ts_packet_len); CntTsPacket++ )
	{
		Sync = bData[index];
		if ( Sync != 0x47 ) 
		{
		//	OutputDebugString("======Sync..faild\n");
		}
		Pusi = bData[index+1];
		Afe = bData[index+3];
		RdData = &bData[index+4];
		if ( (Pusi & 0x40) )
		{
			if ( (Afe & 0x20) )
			{
				LenStuff = RdData[0];
				read_len = 184-(LenStuff+2);
				//kslee 2010/4/23
				if(read_len < 1)
					break;
				memcpy(fid_c2mi + c2mi_total_len, &RdData[LenStuff+2], read_len);
			}
			else
			{
				read_len = 184-1;
				memcpy(fid_c2mi + c2mi_total_len, &RdData[1], read_len);

				if ( RdData[0] == 0x00 )
				{
					find_Pusi = 1;
				}
			}
		}
		else
		{
			if ( find_Pusi == 1 )
			{
				if ( (Afe & 0x20) )
				{
					LenStuff = RdData[0];
					read_len = 184-(LenStuff+1);
					memcpy(fid_c2mi + c2mi_total_len, &RdData[LenStuff+1], read_len);
				}
				else
				{
					read_len = 184;
					memcpy(fid_c2mi + c2mi_total_len, &RdData[0], read_len);
				}
			}
		}
		if ( find_Pusi == 1 )
		{
			c2mi_total_len += read_len;
			++CntTsPacket;
		}

		index += ts_packet_len;
	}
	
	c2mi_len = 0;
	while(1)
	{
		count1 = 6;
		if ( c2mi_total_len - c2mi_len < count1 )
		{
			break;
		}
		memcpy(Header, fid_c2mi+c2mi_len, count1);
		c2mi_len += count1;
		
		LenPacket = (int)ceil((256*Header[4] + Header[5]) / 8.);
		count2 = LenPacket;
		if ( c2mi_total_len - c2mi_len < count2 || count2 > SUB_BANK_MAX_BYTE_SIZE )
		{
			break;
		}
		memcpy(Packet, fid_c2mi+c2mi_len, count2);
		c2mi_len += count2;

		count3 = 4;
		if ( c2mi_total_len - c2mi_len < count3 )
		{
			break;
		}
		memcpy(Crc32, fid_c2mi+c2mi_len, count3);
		c2mi_len += count3;

		if ( c2mi_len >= c2mi_total_len || count1 != 6 || count2 != LenPacket || count3 != 4 )
		{
			break;
		}

		if ( Header[0] == 0 )
		{
			FRAME_IDX = Packet[0];
			PLP_ID = Packet[1];
			INTL_FRAME_START = ((Packet[2]>>7)&0x01);

			i = (Packet[10]<<8) + Packet[11];
			i /= 8;//bytes

			if ( Packet[9] == 0x47 )
			{
				NORMAL_MODE = 1;
			}
			else
			{
				NORMAL_MODE = 0;//High Efficiency Mode
			}

		}
		else if ( Header[0] == 16 && c2mi_analysis_L1_current == 0 )
		{
			c2mi_analysis_L1_current = 1;

			//L1PRE
			TYPE = Packet[2];
			memset(szInfo, 0x00, 64);
			if ( TYPE == 0x00 )			sprintf(szInfo, "%s", "TS only");
			else if ( TYPE == 0x01 )	sprintf(szInfo, "%s", "Generic stream");
			else if ( TYPE == 0x02 )	sprintf(szInfo, "%s", "TS and Generic");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "TYPE=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			BWT_EXT = (Packet[3]>>7) & 0x01;
			memset(szInfo, 0x00, 64);
			if ( BWT_EXT == 0x00 )		sprintf(szInfo, "%s", "off");
			else if ( BWT_EXT == 0x01 )	sprintf(szInfo, "%s", "on");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "RESERVED TONE=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);
			
			S1 = (Packet[3]>>4) & 0x07;
			memset(szInfo, 0x00, 64);
			if ( S1 == 0x000 )		sprintf(szInfo, "%s", "C2_SISO");
			else if ( S1 == 0x001 )	sprintf(szInfo, "%s", "C2_MISO");
			else if ( S1 == 0x002 )	sprintf(szInfo, "%s", "Non-C2");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "S1=%d, %s\r\n", S1, szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			S2 = Packet[3]&0x0F;
			sprintf(debug_string, "S2=%d\r\n", S2);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			FFT_SIZE = (S2>>1);
			memset(szInfo, 0x00, 64);
			if ( FFT_SIZE == 0x000 )		sprintf(szInfo, "FFT Size=%s", "2K, any gaurd interval");
			else if ( FFT_SIZE == 0x001 )	sprintf(szInfo, "FFT Size=%s", "8K, 1/32, 1/16, 1/8 or 1/4");
			else if ( FFT_SIZE == 0x002 )	sprintf(szInfo, "FFT Size=%s", "4K, any gaurd interval");
			else if ( FFT_SIZE == 0x003 )	sprintf(szInfo, "FFT Size=%s", "1K, any gaurd interval");
			else if ( FFT_SIZE == 0x004 )	sprintf(szInfo, "FFT Size=%s", "16K, any gaurd interval");
			else if ( FFT_SIZE == 0x005 )	sprintf(szInfo, "FFT Size=%s", "32K, 1/32, 1/16, 1/8 or 1/4");
			else if ( FFT_SIZE == 0x006 )	sprintf(szInfo, "FFT Size=%s", "8K, any gaurd interval");
			else if ( FFT_SIZE == 0x007 )	sprintf(szInfo, "FFT Size=%s", "32K, 1/128, 19/256 or 19/128");
			else sprintf(szInfo, "FFT Size=%s", "unknown");
			sprintf(debug_string, "%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);
			
			L1_REPETITION_FLAG = (Packet[4]>>7) & 0x01;
			sprintf(debug_string, "L1_REPETITION_FLAG=%d\r\n", L1_REPETITION_FLAG);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);
			
			GUARD_INTERVAL = ((Packet[4]>>4)&0x07);
			memset(szInfo, 0x00, 64);
			if ( GUARD_INTERVAL == 0x000 )		sprintf(szInfo, "%s", "1/128");
			else if ( GUARD_INTERVAL == 0x001 )	sprintf(szInfo, "%s", "1/64");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "GUARD_INTERVAL=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);
			
			PAPR = (Packet[4]&0x0F);
			memset(szInfo, 0x00, 64);
			if ( PAPR == 0x000 )		sprintf(szInfo, "%s", "None");
			else if ( PAPR == 0x001 )	sprintf(szInfo, "%s", "ACE used");
			else if ( PAPR == 0x002 )	sprintf(szInfo, "%s", "TR used");
			else if ( PAPR == 0x003 )	sprintf(szInfo, "%s", "Both ACE and TR used");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "PAPR=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			L1_MOD = ((Packet[5]>>4)&0x0F);
			memset(szInfo, 0x00, 64);
			if ( L1_MOD == 0x000 )		sprintf(szInfo, "%s", "BPSK");
			else if ( L1_MOD == 0x001 )	sprintf(szInfo, "%s", "QPSK");
			else if ( L1_MOD == 0x002 )	sprintf(szInfo, "%s", "16QAM");
			else if ( L1_MOD == 0x003 )	sprintf(szInfo, "%s", "64QAM");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "L1_MOD=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			L1_COD = ((Packet[5]>>2)&0x03);
			memset(szInfo, 0x00, 64);
			if ( L1_COD == 0x000 )		sprintf(szInfo, "%s", "None");
			else if ( L1_COD == 0x001 )		sprintf(szInfo, "%s", "best");
			else if ( L1_COD == 0x002 )		sprintf(szInfo, "%s", "4symbols");
			else if ( L1_COD == 0x003 )		sprintf(szInfo, "%s", "8symbols");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "L1 TI MODE=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			L1_FEC_TYPE = (Packet[5]&0x03);
			memset(szInfo, 0x00, 64);
			if ( L1_FEC_TYPE == 0x000 )		sprintf(szInfo, "[DSlice] TYPE=%s, FEC HEADER TYPE=%s", "type1", "Robust");
			else if ( L1_FEC_TYPE == 0x001 )		sprintf(szInfo, "[DSlice] TYPE=%s, FEC HEADER TYPE=%s", "type1", "HEM");
			else if ( L1_FEC_TYPE == 0x002 )		sprintf(szInfo, "[DSlice] TYPE=%s, FEC HEADER TYPE=%s", "type2", "Robust");
			else if ( L1_FEC_TYPE == 0x003 )		sprintf(szInfo, "[DSlice] TYPE=%s, FEC HEADER TYPE=%s", "type2", "HEM");
			else sprintf(szInfo, "[DSlice] TYPE=%s, FEC HEADER TYPE=%s", "unknown", "unknown");
			sprintf(debug_string, "%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			L1_POST_SIZE = (Packet[6]<<10) + (Packet[7]<<2) + ((Packet[8]>>6)&0x03);
			sprintf(debug_string, "L1_POST_SIZE=%dbits\r\n", L1_POST_SIZE);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);
			
			//L1_POST_INFO_SIZE == L1CONF_LEN + L1DYN_CONF_LEN + L1EXT_LEN
			L1_POST_INFO_SIZE = ((Packet[8]&0x3F)<<12) + (Packet[9]<<4) + ((Packet[10]>>4)&0x0F);
			sprintf(debug_string, "L1_POST_INFO_SIZE=%dbits\r\n", L1_POST_INFO_SIZE);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			PILOT_PATTERN = (Packet[10]&0x0F);
			memset(szInfo, 0x00, 64);
			if ( PILOT_PATTERN == 0x000 )		sprintf(szInfo, "%s", "PP1");
			else if ( PILOT_PATTERN == 0x001 )	sprintf(szInfo, "%s", "PP2");
			else if ( PILOT_PATTERN == 0x002 )	sprintf(szInfo, "%s", "PP3");
			else if ( PILOT_PATTERN == 0x003 )	sprintf(szInfo, "%s", "PP4");
			else if ( PILOT_PATTERN == 0x004 )	sprintf(szInfo, "%s", "PP5");
			else if ( PILOT_PATTERN == 0x005 )	sprintf(szInfo, "%s", "PP6");
			else if ( PILOT_PATTERN == 0x006 )	sprintf(szInfo, "%s", "PP7");
			else if ( PILOT_PATTERN == 0x007 )	sprintf(szInfo, "%s", "PP8");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "PILOT_PATTERN=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			TX_ID_AVAILABILITY = Packet[11];
			sprintf(debug_string, "TX_ID_AVAILABILITY=%d\r\n", TX_ID_AVAILABILITY);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			CELL_ID = (Packet[12]<<8) + Packet[13];
			sprintf(debug_string, "START FREQ=%d\r\n", ((TX_ID_AVAILABILITY<<16) + CELL_ID));
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			NETWORK_ID = (Packet[14]<<8) + Packet[15];
			sprintf(debug_string, "NETWORK_ID=%d\r\n", NETWORK_ID);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			C2_SYSTEM_ID = (Packet[16]<<8) + Packet[17];
			sprintf(debug_string, "C2_SYSTEM_ID=%d\r\n", C2_SYSTEM_ID);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			MUM_C2_FRAMES = Packet[18];
			sprintf(debug_string, "MUM_C2_FRAMES=%d\r\n", MUM_C2_FRAMES);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			NUM_DATA_SYMBOLS = (Packet[19]<<4) + ((Packet[20]>>4)&0x0F);
			sprintf(debug_string, "NOTCH START=%d\r\n", ((MUM_C2_FRAMES<<6) + (NUM_DATA_SYMBOLS>>6)));
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			REGEN_FLAG = ((Packet[20]>>1)&0x07);
			sprintf(debug_string, "NOTCH WIDTH=%d\r\n", (((NUM_DATA_SYMBOLS & 0x3F)<<3) + REGEN_FLAG));
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			L1_POST_EXTENTION = (Packet[20]&0x01);
			if ( L1_POST_EXTENTION == 0x000 )	sprintf(szInfo, "%s", "off");
			else if ( L1_POST_EXTENTION == 0x001 )	sprintf(szInfo, "%s", "on");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "NUM NOTCH=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			NUM_RF = ((Packet[21]>>5)&0x07);
			sprintf(debug_string, "NUM_RF=%d\r\n", NUM_RF);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			CURRENT_RF_IDX = ((Packet[21]>>2)&0x07);
			sprintf(debug_string, "CURRENT_RF_IDX=%d\r\n", CURRENT_RF_IDX);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);
			
			RESERVED = ((Packet[21]&0x03)<<8) + Packet[22];
			sprintf(debug_string, "RESERVED=%d\r\n", RESERVED);
			//strcat(szResult, debug_string);
			
			//L1CONF_LEN
			L1CONF_LEN = (Packet[23]<<8) + Packet[24];
			sprintf(debug_string, "L1CONF_LEN=%dbits\r\n", L1CONF_LEN);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			//L1CONF
			SUB_SLICES_PER_FRAME = (Packet[25]<<7) + ((Packet[26]>>1)&0x7F);
			sprintf(debug_string, "SUB_SLICES_PER_FRAME=%d\r\n", SUB_SLICES_PER_FRAME);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			NUM_PLP = ((Packet[26]&0x01)<<7) + ((Packet[27]>>1)&0x7F);
			sprintf(debug_string, "NUM_PLP=%d\r\n", NUM_PLP);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			//DVB-C2 - Multi PLP
			__C2_PLP_Count = NUM_PLP;

			NUM_AUX = ((Packet[27]&0x01)<<3) + ((Packet[28]>>5)&0x07);
			sprintf(debug_string, "NUM_AUX=%d\r\n", NUM_AUX);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			AUX_CONFIG_RFU = ((Packet[28]&0x1F)<<3) + ((Packet[29]>>5)&0x07);
			sprintf(debug_string, "AUX_CONFIG_RFU=%d\r\n", AUX_CONFIG_RFU);
			CHK_DVB_C2_PARAM
			//strcat(szResult, debug_string);

			////////
			k = 29;
			kk = 4;

			//35bits * NUM_RF
			for ( i = 0; i < NUM_RF; i++ )
			{
				sprintf(debug_string, "----------\r\n");
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				//3bits
				GET_DVB_C2_PARA(RF_IDX,3)
				sprintf(debug_string, "RF_IDX=%d\r\n", RF_IDX);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				//32bits
				GET_DVB_C2_PARA(FREQUENCY,32)
				sprintf(debug_string, "FREQUENCY=%d Hz\r\n", FREQUENCY);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);
			}

			//34bits
			if ( (S2 & 0x01) )
			{
				//4bits
				GET_DVB_C2_PARA(FEF_TYPE,4)
				sprintf(debug_string, "FEF_TYPE=%d\r\n", FEF_TYPE);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				//22bits
				GET_DVB_C2_PARA(FEF_LENGTH,22)
				sprintf(debug_string, "FEF_LENGTH=%d\r\n", FEF_LENGTH);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);
				
				//8bits
				GET_DVB_C2_PARA(FEF_INTERVAL,8)
				sprintf(debug_string, "FEF_INTERVAL=%d\r\n", FEF_INTERVAL);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);
			}

			//89bits * NUM_PLP
			for ( i = 0; i < NUM_PLP; i++ )
			{
				sprintf(debug_string, "----------\r\n");
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_ID,8)
				sprintf(debug_string, "PLP_ID=%d\r\n", PLP_ID);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_TYPE,3)
				memset(szInfo, 0x00, 64);
				if ( PLP_TYPE == 0x000 )		sprintf(szInfo, "%s", "Common PLP");
				else if ( PLP_TYPE == 0x001 )	sprintf(szInfo, "%s", "Data PLP Type 1");
				else if ( PLP_TYPE == 0x002 )	sprintf(szInfo, "%s", "Data PLP Type 2");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PLP_TYPE=%s\r\n", szInfo);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_PAYLOAD_TYPE,5)
				memset(szInfo, 0x00, 64);
				if ( PLP_PAYLOAD_TYPE == 0x000 )		sprintf(szInfo, "%s", "GPFS");
				else if ( PLP_PAYLOAD_TYPE == 0x001 )	sprintf(szInfo, "%s", "GCS");
				else if ( PLP_PAYLOAD_TYPE == 0x002 )	sprintf(szInfo, "%s", "GSE");
				else if ( PLP_PAYLOAD_TYPE == 0x003 )	sprintf(szInfo, "%s", "TS");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PLP_PAYLOAD_TYPE=%s\r\n", szInfo);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(FF_FLAG,1)
				sprintf(debug_string, "FF_FLAG=%d\r\n", FF_FLAG);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(FIRST_RF_IDX,3)
				sprintf(debug_string, "FIRST_RF_IDX=%d\r\n", FIRST_RF_IDX);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);
				
				GET_DVB_C2_PARA(FIRST_FRAME_IDX,8)
				sprintf(debug_string, "FIRST_FRAME_IDX=%d\r\n", FIRST_FRAME_IDX);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_GROUP_ID,8)
				sprintf(debug_string, "PLP_GROUP_ID=%d\r\n", PLP_GROUP_ID);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_COD,3)
				memset(szInfo, 0x00, 64);
				if ( PLP_COD == 0x001 )			sprintf(szInfo, "%s", "2/3");
				else if ( PLP_COD == 0x002 )	sprintf(szInfo, "%s", "3/4");
				else if ( PLP_COD == 0x003 )	sprintf(szInfo, "%s", "4/5");
				else if ( PLP_COD == 0x004 )	sprintf(szInfo, "%s", "5/6");
				else if ( PLP_COD == 0x005 )	sprintf(szInfo, "%s", "8/9(16K LDPC) or 9/10(64K LDPC)");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PLP_COD=%s\r\n", szInfo);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_MOD,3)
				memset(szInfo, 0x00, 64);
				if		( PLP_MOD == 0x001 )	sprintf(szInfo, "%s", "16QAM");
				else if ( PLP_MOD == 0x002 )	sprintf(szInfo, "%s", "64QAM");
				else if ( PLP_MOD == 0x003 )	sprintf(szInfo, "%s", "256QAM");
				else if ( PLP_MOD == 0x004 )	sprintf(szInfo, "%s", "1024QAM");
				else if ( PLP_MOD == 0x005 )	sprintf(szInfo, "%s", "4096QAM");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PLP_MOD=%s\r\n", szInfo);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_ROTATION,1)
				sprintf(debug_string, "PLP_HEM=%d\r\n", PLP_ROTATION);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_FEC_TYPE,2)
				memset(szInfo, 0x00, 64);
				if ( PLP_FEC_TYPE == 0x000 )		sprintf(szInfo, "%s", "16K LDPC");
				else if ( PLP_FEC_TYPE == 0x001 )	sprintf(szInfo, "%s", "64K LDPC");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PLP_FEC_TYPE=%s\r\n", szInfo);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_NUM_BLOCKS_MAX,10)
				sprintf(debug_string, "PLP_NUM_BLOCKS_MAX=%d\r\n", PLP_NUM_BLOCKS_MAX);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(FRAME_INTERVAL,8)
				sprintf(debug_string, "FRAME_INTERVAL=%d\r\n", FRAME_INTERVAL);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(TIME_IL_LENGTH,8)
				sprintf(debug_string, "TIME_IL_LENGTH=%d\r\n", TIME_IL_LENGTH);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(TIME_IL_TYPE,1)
				sprintf(debug_string, "TIME_IL_TYPE=%d\r\n", TIME_IL_TYPE);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(IN_BAND_A_FLAG,1)
				sprintf(debug_string, "IN_BAND_A_FLAG=%d\r\n", IN_BAND_A_FLAG);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(IN_BAND_B_FLAG,1)
				sprintf(debug_string, "IN_BAND_B_FLAG=%d\r\n", IN_BAND_B_FLAG);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(RESERVED_1,11)
				sprintf(debug_string, "RESERVED_1=%d\r\n", RESERVED_1);
				CHK_DVB_C2_PARAM
				//strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_MODE,2)
				sprintf(debug_string, "PLP_MODE=%d\r\n", PLP_MODE);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(STATIC_FLAG,1)
				sprintf(debug_string, "STATIC_FLAG=%d\r\n", STATIC_FLAG);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(STATIC_PADDING_FLAG,1)
				sprintf(debug_string, "STATIC_PADDING_FLAG=%d\r\n", STATIC_PADDING_FLAG);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				//DVB-C2 - Multi PLP
				__C2_PLP_ID_M[i] = PLP_ID;
				__C2_PLP_COD_M[i] = PLP_COD;
				__C2_PLP_FEC_M[i] = PLP_FEC_TYPE;
				__C2_PLP_HEM_M[i] = PLP_ROTATION;
				__C2_PLP_MOD_M[i] = PLP_MOD;
			}

			GET_DVB_C2_PARA(RESERVED_2,32)
			sprintf(debug_string, "RESERVED_2=%d\r\n", RESERVED_2);
			strcat(szResult, debug_string);

			//32bits * NUM_AUX
			for ( i = 0; i < NUM_AUX; i++ )
			{
				sprintf(debug_string, "----------\r\n");
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(AUX_STREAM_TAG,4)
				sprintf(debug_string, "AUX_STREAM_TAG=%d\r\n", AUX_STREAM_TAG);
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(AUX_PRIVATE_CONF,28)
				sprintf(debug_string, "AUX_PRIVATE_CONF=%d\r\n", AUX_PRIVATE_CONF);
				strcat(szResult, debug_string);
			}

			//L1DYN_CURR_LEN
			L1DYN_CURR_LEN = (Packet[25+(int)ceil(L1CONF_LEN/8.)] << 8) + Packet[25+(int)ceil(L1CONF_LEN/8.)+1];
			sprintf(debug_string, "L1DYN_CURR_LEN=%dbits\r\n", L1DYN_CURR_LEN);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			//L1DYN_CURR
			k = 25+(int)ceil(L1CONF_LEN/8.)+1;
			//k += 1;
			//kk = 0;
			kk = -1;
			
			GET_DVB_C2_PARA(FRAME_IDX,8)
			sprintf(debug_string, "FRAME_IDX=%d\r\n", FRAME_IDX);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			GET_DVB_C2_PARA(SUB_SLICE_INTERVAL,22)
			sprintf(debug_string, "SUB_SLICE_INTERVAL=%d\r\n", SUB_SLICE_INTERVAL);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			GET_DVB_C2_PARA(TYPE_C2_START,22)
			sprintf(debug_string, "TYPE_C2_START=%d\r\n", TYPE_C2_START);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			GET_DVB_C2_PARA(L1_CHANGE_COUNTER,8)
			sprintf(debug_string, "L1_CHANGE_COUNTER=%d\r\n", L1_CHANGE_COUNTER);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			GET_DVB_C2_PARA(START_RF_IDX,3)
			sprintf(debug_string, "START_RF_IDX=%d\r\n", START_RF_IDX);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			GET_DVB_C2_PARA(RESERVED_1,8)
			sprintf(debug_string, "RESERVED_1=%d\r\n", RESERVED_1);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			for ( i = 0; i <  NUM_PLP; i++ )
			{
				sprintf(debug_string, "----------\r\n");
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_ID,8)
				sprintf(debug_string, "PLP_ID=%d\r\n", PLP_ID);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_START,22)
				sprintf(debug_string, "PLP_START=%d\r\n", PLP_START);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(PLP_NUM_BLOCKS,10)
				sprintf(debug_string, "PLP_NUM_BLOCKS=%d\r\n", PLP_NUM_BLOCKS);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				GET_DVB_C2_PARA(RESERVED_2,8)
				sprintf(debug_string, "RESERVED_2=%d\r\n", RESERVED_2);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);

				//DVB-C2 - Multi PLP
				__C2_PLP_BLK_M[i] = PLP_NUM_BLOCKS;
			}

			GET_DVB_C2_PARA(RESERVED_3,8)
			sprintf(debug_string, "RESERVED_3=%d\r\n", RESERVED_3);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);

			for ( i = 0; i < NUM_AUX; i++ )
			{
				GET_DVB_C2_PARA(AUX_PRIVATE_DYN,48)
				sprintf(debug_string, "AUX_PRIVATE_DYN=%d\r\n", AUX_PRIVATE_DYN);
				CHK_DVB_C2_PARAM
				strcat(szResult, debug_string);
			}

			//L1EXT_LEN = 16bits
			//L1EXT ...(8 * ceil(L1EXT_LEN/8])
			//CRC-32
			//L1 Padding
		}
		else if ( Header[0] == 32 && c2mi_analysis_timestamp == 0 )
		{
			c2mi_analysis_timestamp = 1;

			memset(szInfo, 0x00, 64);

			bw = (Packet[0]&0x0F);
			if ( bw == 0x2 )
			{
				//subseconds *= (1./48.);
				sprintf(szInfo, "6MHz");
			}
			else if ( bw == 0x3 )
			{
				//subseconds *= (1./56.);
				sprintf(szInfo, "7MHz");
			}
			else if ( bw == 0x4 )
			{
				//subseconds *= (1./64.);
				sprintf(szInfo, "8MHz");
			}
			else
			{
				sprintf(szInfo, "unknown");
			}

			sprintf(debug_string, "BAND WIDTH=%s\r\n", szInfo);
			CHK_DVB_C2_PARAM
			strcat(szResult, debug_string);
		}
		else
		{
		}
	}

	if ( fid_c2mi )
	{
		free(fid_c2mi);
	}

	if ( Packet )
	{
		free(Packet);
	}

	if ( szBuf )
	{
		free(szBuf);
	}
#if 1
//	double N_mod, frame_closing_symbol, N_P2, C_P2, L_data, C_data, C_FC, D_L1, C_tot, N_ldpc, K_ldpc, K_bch, N_FEC_FRAME, R_bits, GuardInterval, IFFT_SIZE, T, T_F, LDPC_code;
//	double max_bitrate;
	long K_bch, LF;
	double T, TF;
	double PlpBitrate;
	long N_fec_frame, GI;
	double M;


	//DVB-C2 - Multi PLP
	for ( i = 0; i < __C2_PLP_Count; i++ )
	{
		N_fec_frame = __C2_PLP_BLK_M[i];
		
		if(__C2_PLP_HEM_M[i] == 0)
			M = 1.0;
		else
			M = 188.0 / 187.0;

		if(GUARD_INTERVAL == 0)
			GI = 32;
		else
			GI = 64;

		// T
		if(bw == 2)
			T = 7.0 / 1000000.0 / 48.0;
		else if(bw == 3)
			T = 7.0 / 1000000.0 / 56.0;
		else
			T = 7.0 / 1000000.0 / 64.0;

		//LF
		if(L1_COD == 0 || L1_COD == 1)
			LF = 449;
		else if(L1_COD == 2)
			LF = 452;
		else
			LF = 456;

		//LDPC_code, K_bch, K_ldpc
		if(__C2_PLP_FEC_M[i] == 0) //16K
		{
			if(__C2_PLP_COD_M[i] == 1)
			{
			    K_bch = 10632;
			}
			else if(__C2_PLP_COD_M[i] == 2)
			{
			    K_bch = 11712;
			}
			else if(__C2_PLP_COD_M[i] == 3)
			{
				K_bch = 12432;
			}
			else if(__C2_PLP_COD_M[i] == 4)
			{
				K_bch = 13152;
			}
			else if(__C2_PLP_COD_M[i] == 5)
			{
				K_bch = 14232;
			}
			else
			{
			    K_bch = 0;
			}
		}
		else
		{
			if(__C2_PLP_COD_M[i] == 1)
			{
		        K_bch = 43040;
			}
			else if(__C2_PLP_COD_M[i] == 2)
			{
				K_bch = 48408;
			}
			else if(__C2_PLP_COD_M[i] == 3)
			{
				K_bch = 51648;
			}
			else if(__C2_PLP_COD_M[i] == 4)
			{
				K_bch = 53840;
			}
			else if(__C2_PLP_COD_M[i] == 5)
			{
				K_bch = 58192;
			}
			else
			{
			    K_bch = 0;
			}
		}
		//TF
		TF = (double)LF * (double)(4096.0 + GI) * T;
		PlpBitrate = ((double)N_fec_frame * (double)(K_bch - 80) * M) / TF;
	

		//fixed
		__FIf__->TL_gBitrate = (int)PlpBitrate;

		//DVB-C2 - Multi PLP
		__C2_PLP_TS_Bitrate_M[i] = (int)PlpBitrate;
	}

	//DVB-C2 - Multi PLP
	avg_bitrate = 0;
	for ( i = 0; i < __C2_PLP_Count; i++ )
	{
		avg_bitrate += __C2_PLP_TS_Bitrate_M[i];
	}

#endif
	return avg_bitrate;
}
int	CHldFmtrDvbC2_::SetC2MiParam(
	int C2_BW, int C2_L1, int C2_Guard, int C2_Network, int C2_System,
	int C2_StartFreq, int C2_NumNoth, int C2_RevTone, int C2_NotchStart, int C2_NotchWidth,
	int C2_Dslice_type, int C2_Dslice_FecHeder, int C2_Dslice_BBHeder, int C2_Dslice_TonePos,
	int C2_Dslice_OffRight, int C2_Dslice_OffLeft, int C2_Plp_Mod, int C2_Plp_Code, int C2_Plp_Fec,
	int C2_Plp_Count, int C2_Plp_ID, int C2_Plp_Blk, int C2_Plp_TS_Bitrate, char *C2_Plp_TS,
	int C2_createFile )
{
	__C2_BW = C2_BW;
	__C2_L1 = C2_L1;
	__C2_Guard = C2_Guard;
	__C2_Network = C2_Network;
	__C2_System = C2_System;
	__C2_StartFreq = C2_StartFreq;
	__C2_NumNoth = C2_NumNoth;
	__C2_RevTone = C2_RevTone;
	__C2_NotchStart = C2_NotchStart;
	__C2_NotchWidth = C2_NotchWidth;
	__C2_Dslice_TonePos = C2_Dslice_TonePos;
	__C2_Dslice_OffRight = C2_Dslice_OffRight;
	__C2_Dslice_OffLeft = C2_Dslice_OffLeft;
	__C2_Dslice_type = C2_Dslice_type;
	__C2_Dslice_FecHeder = C2_Dslice_FecHeder;

	__C2_CreateFile = C2_createFile;

	if(C2_Plp_Count == 0)
	{
		__C2_PLP_Count = 0;
	}
	__C2_PLP_ID_M[C2_Plp_Count] = C2_Plp_ID;
	__C2_PLP_MOD_M[C2_Plp_Count] = C2_Plp_Mod;
	__C2_PLP_COD_M[C2_Plp_Count] = C2_Plp_Code;
	__C2_PLP_FEC_M[C2_Plp_Count] = C2_Plp_Fec;
	__C2_PLP_BLK_M[C2_Plp_Count] = C2_Plp_Blk;
	__C2_PLP_HEM_M[C2_Plp_Count] = C2_Dslice_BBHeder;
	__C2_PLP_TS_Bitrate_M[C2_Plp_Count] = C2_Plp_TS_Bitrate;
	sprintf(__C2_PLP_TS_M[C2_Plp_Count], "%s", C2_Plp_TS);
	++__C2_PLP_Count;

	return 0;
}
