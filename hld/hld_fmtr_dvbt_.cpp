
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>
#include	<math.h>

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_fmtr_dvbt_.h"
#ifdef WIN32
#include "DVBT2_MultiplePLP.h"
#else
#include	"dvbt2_multipleplp.h"
#endif

//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation), 5(Frame interval), 6(Time interleaving length), 7(Time interleaving type)
#define E_SISO_ 0
#define E_FFT_SIZE_ 1
#define E_PAPR_POS 2
#define E_FEF_ 3
#define E_ROTATION_ 4
#define E_FRAME_INTERVAL_ 5
#define E_INTERLEAVING_LENGTH_ 6
#define E_INTERLEAVING_TYPE_ 7
#define E_PLP_TYPE_ 8
#define E_CRC_ 9
#define E_T2MIP_LENGTH_ 10

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrDvbT_::CHldFmtrDvbT_(void)
{
#if defined(WIN32)
	TL_T2MI_hMutex = INVALID_HANDLE_VALUE;
#else
#endif

	TL_T2MI_PumpingThreadDone = 1;
	TL_T2MI_ConvertingThreadDone = 1;
	
}

CHldFmtrDvbT_::~CHldFmtrDvbT_()
{
#if defined(WIN32)	
	if ( TL_T2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_T2MI_hMutex);
		TL_T2MI_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutex_destroy(&TL_T2MI_hMutex);
#endif
}
void	CHldFmtrDvbT_::SetCommonMethod_5(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void	CHldFmtrDvbT_::CreateT2miMutex(void)
{
#if defined(WIN32)
	TL_T2MI_hMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_T2MI_hMutexAttr);
	pthread_mutexattr_settype(&TL_T2MI_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_T2MI_hMutex, &TL_T2MI_hMutexAttr);
#endif

}
void	CHldFmtrDvbT_::DestroyT2miMutex(void)
{
#if defined(WIN32)
	if ( TL_T2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_T2MI_hMutex);
		TL_T2MI_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_T2MI_hMutexAttr);
	pthread_mutex_destroy(&TL_T2MI_hMutex);
#endif

}
void	CHldFmtrDvbT_::LockT2miMutex(void)
{
#if defined(WIN32)
	if ( TL_T2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_T2MI_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_T2MI_hMutex);
#endif
}
void	CHldFmtrDvbT_::UnlockT2miMutex(void)
{
#if defined(WIN32)
	if ( TL_T2MI_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_T2MI_hMutex);
	}
#else
	pthread_mutex_unlock(&TL_T2MI_hMutex);
#endif
}

//DVB-T2 - Multi PLP 4096->8192->65526
#define CHK_DVB_T2_PARAM if((strlen(szResult) + strlen(debug_string)) >= 65526) {szResult = NULL;free(fid_t2mi);free(Packet);free(szBuf);return 0;}
#define GET_DVB_T2_PARA(a,b) a = 0;	for ( kkk = 0; kkk < b; kkk++ )	{if ( kk < 0 ) {k += 1; kk = 7;} a += (((Packet[k]>>kk)&0x01)<<(b-kkk-1)); kk -= 1;}
#ifdef WIN32
double CHldFmtrDvbT_::RunT2MiParser(char *szFile, T2MI_Parsing_Info *szResult)
#else
double CHldFmtrDvbT_::RunT2MiParser(char *szFile, char *szResult)
#endif
{
#if	0
	__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "RunT2MiParser : not used");
#else
		//
		FILE *fp;
		unsigned char *szBuf;
		int sync_pos = -1;
		int ts_packet_len = 0;
		int index, numread;
		
		//2012/12/17 T2MI stream error flag
		int error_flag = 0;
#ifndef WIN32
		memset(szResult, 0x00, 65526);
#endif
		if ( __FIf__->IsT2miFile(szFile) == 0 )
		{
			//DVB-T2 - Multi PLP 4096->65526
			return 0;
		}
	
		BYTE *bData;
		int CntByte = 1, CntFrame = 0, CntTsPacket = 0;
		int t2mi_len = 0, read_len = 0, LenStuff, LenPacket, count1, count2, count3, t2mi_total_len = 0, find_Pusi = 0, t2mi_analysis_L1_current=0, t2mi_analysis_timestamp=0, i, k, kk, kkk;
		unsigned char Sync, Pusi, Afe;
		unsigned char *fid_t2mi;
		unsigned char *RdData;
		unsigned char Header[6], Crc32[4], *Packet;
	
		int INTL_FRAME_START, NORMAL_MODE, NULL_PACKET_DELETION = 0;
	
		int FFT_SIZE = 0;
		int L1CONF_LEN = 0, L1DYN_CURR_LEN = 0, L1EXT_LEN = 0;
		int TYPE, BWT_EXT, S1, S2, L1_REPETITION_FLAG, GUARD_INTERVAL, PAPR, L1_MOD, L1_COD, L1_FEC_TYPE, L1_POST_SIZE, L1_POST_INFO_SIZE;
		int TX_ID_AVAILABILITY, CELL_ID, NETWORK_ID, T2_SYSTEM_ID, PID_;
		int PILOT_PATTERN, MUM_T2_FRAMES, NUM_DATA_SYMBOLS, REGEN_FLAG, L1_POST_EXTENTION, NUM_RF, CURRENT_RF_IDX, RESERVED;
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
		int TYPE_T2_START;
		int L1_CHANGE_COUNTER;
		int START_RF_IDX;
		int PLP_START;
		int PLP_NUM_BLOCKS;
		int RESERVED_3;
		int AUX_PRIVATE_DYN;
	int PLP_ROTATION_M[MAX_PLP_COUNT];
	int PLP_ID_M[MAX_PLP_COUNT];
	int PLP_MOD_M[MAX_PLP_COUNT];
	int PLP_COD_M[MAX_PLP_COUNT];
	int PLP_FEC_TYPE_M[MAX_PLP_COUNT];
	int PLP_MODE_M[MAX_PLP_COUNT]; //high(2) or normal(1) efficiency mode, 0(not specified if T2_VERSION == "0000")
	int PLP_NUM_BLOCKS_M[MAX_PLP_COUNT];
	int PLP_BITRATE_M[MAX_PLP_COUNT];
	int PLP_TS_BITRATE_M[MAX_PLP_COUNT];
	char PLP_TS_M[MAX_PLP_COUNT][1024];
		
		int bw;
		double avg_bitrate = 0;
		char szInfo[64];
	
		fp = fopen(szFile, "rb");
		if ( !fp )
		{
			return -1;
		}
		
		szBuf = (unsigned char *)malloc((MAX_BUFFER_BYTE_SIZE * 4));
		if ( !szBuf )
		{
			fclose(fp);
			return -2;
		}
	
		fid_t2mi = (unsigned char*)malloc((MAX_BUFFER_BYTE_SIZE * 4));
		if ( !fid_t2mi )
		{
#ifndef WIN32
			szResult = NULL;
#endif
			free(szBuf);
			return 0;
		}
	
		Packet = (unsigned char*)malloc((MAX_BUFFER_BYTE_SIZE * 4));
		if ( !Packet )
		{
#ifndef WIN32
			szResult = NULL;
#endif
			free(fid_t2mi);
			free(szBuf);
			return 0;
		}
		int retryCnt = 0;
		numread = fread(szBuf, 1, (MAX_BUFFER_BYTE_SIZE * 4), fp);
		fclose(fp);
__RETRY__:	
		if(find_Pusi == 1)
		{
			fp = fopen(szFile, "rb");
			if ( !fp || retryCnt > 2)
			{
				free(Packet);
				free(fid_t2mi);
				free(szBuf);
				return -1;
			}
			fseek(fp, ((MAX_BUFFER_BYTE_SIZE * 4) * (retryCnt + 1)), SEEK_SET);
			numread = fread(szBuf, 1, (MAX_BUFFER_BYTE_SIZE * 4), fp);
			retryCnt++;
			find_Pusi = 0;
			t2mi_total_len = 0;
			t2mi_analysis_L1_current = 0;
			t2mi_analysis_timestamp = 0;
		}
		sync_pos = __FIf__->TL_SyncLockFunction((char*)szBuf, numread, &ts_packet_len, numread, 3);
		if ( sync_pos == -1 )
		{
#ifndef WIN32
			szResult = NULL;
#endif
			free(Packet);
			free(fid_t2mi);
			free(szBuf);
			return 0;
		}
	
		index = 0;
		bData = szBuf+sync_pos;
	unsigned char pid_up;
	unsigned char pid_down;
	unsigned char pid_up_rem;
	unsigned char pid_down_rem;
	
		//1600 -> 3200 -> 5577 -> 16731 -> (numread/ts_packet_len)
		for ( CntTsPacket = 0; CntTsPacket < (int)(numread/ts_packet_len); CntTsPacket++ )
		{
			Sync = bData[index];
			if ( Sync != 0x47 ) 
			{
			//	OutputDebugString("======Sync..faild\n");
			}
			pid_up = (bData[index+1] & 0x1F);
			pid_down = bData[index + 2];

			if((pid_up == 0x1F && pid_down == 0xFF)/* || (pid_up == 0x0 && (pid_down >= 0x0 && pid_down <= 0x1F))*/)
			{
				index += ts_packet_len;
				continue;
			}

			if(find_Pusi == 1)
			{
				if(pid_up_rem != pid_up || pid_down_rem != pid_down)
				{
					index += ts_packet_len;
					continue;
				}
			}
			else
			{
				pid_up_rem = pid_up;
				pid_down_rem = pid_down;
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
					if(read_len > 0)
					{
						if(find_Pusi == 0)
						{
							int startpos;
							startpos = RdData[LenStuff+1];
							LenStuff = LenStuff + startpos;
							read_len = read_len - startpos;
						}
						memcpy(fid_t2mi + t2mi_total_len, &RdData[LenStuff+2], read_len);
					}

				}
				else
				{
					if(find_Pusi == 0)
					{
						LenStuff = RdData[0];
					}
					else
					{
						LenStuff = 0;
					}
					read_len = 184 - LenStuff - 1;
					memcpy(fid_t2mi + t2mi_total_len, &RdData[LenStuff + 1], read_len);
				}
				find_Pusi = 1;
			}
			else
			{
				if ( find_Pusi == 1 )
				{
					if ( (Afe & 0x20) )
					{
						LenStuff = RdData[0];
						read_len = 184-(LenStuff+1);
						memcpy(fid_t2mi + t2mi_total_len, &RdData[LenStuff+1], read_len);
					}
					else
					{
						read_len = 184;
						memcpy(fid_t2mi + t2mi_total_len, &RdData[0], read_len);
					}
				}
			}
			if ( find_Pusi == 1 )
			{
				t2mi_total_len += read_len;
				++CntTsPacket;
			}
	
			index += ts_packet_len;
		}
		
		t2mi_len = 0;
		error_flag = 0;
		int t2mi_start_pos = 0;
		while(1)
		{
			count1 = 6;
			if ( t2mi_total_len - t2mi_len < count1 )
			{
				break;
			}
			memcpy(Header, fid_t2mi+t2mi_len, count1);
			t2mi_len += count1;
			
			LenPacket = (int)ceil((256*Header[4] + Header[5]) / 8.);
			count2 = LenPacket;
			if ( t2mi_total_len - t2mi_len < count2 || count2 > SUB_BANK_MAX_BYTE_SIZE )
			{
				break;
			}
			memcpy(Packet, fid_t2mi+t2mi_len, count2);
			t2mi_len += count2;
	
			count3 = 4;
			if ( t2mi_total_len - t2mi_len < count3 )
			{
				break;
			}
			memcpy(Crc32, fid_t2mi+t2mi_len, count3);
			unsigned int crc = CRC32::crc32((const char *)(fid_t2mi + t2mi_start_pos), (LenPacket + count1), 0xFFFFFFFF);
			
			t2mi_len += count3;
	
			if ( t2mi_len >= t2mi_total_len || count2 != LenPacket)
			{
				break;
			}
			if((Crc32[0] != ((crc >> 24) & 0xFF)) || (Crc32[1] != ((crc >> 16) & 0xFF)) || (Crc32[2] != ((crc >> 8) & 0xFF)) || (Crc32[3] != (crc & 0xFF)))
			{
				error_flag = error_flag | (0x1 << E_CRC_);
				goto __RETRY__;
			}

			t2mi_start_pos = t2mi_len;

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
	
				//DVB-T2 - Multi PLP
				PLP_MODE_M[PLP_ID] = (NORMAL_MODE==0 ? 2 : 1);
			}
			else if ( Header[0] == 16 && t2mi_analysis_L1_current == 0 )
			{
				t2mi_analysis_L1_current = 1;
#ifdef WIN32
				//PID
				(*szResult).pid = ((pid_up & 0x1F) << 8) + (pid_down & 0xFF);
#else
				PID_ = ((pid_up & 0x1F) << 8) + (pid_down & 0xFF);
#endif

				//L1PRE
				TYPE = Packet[2];
#ifndef WIN32
				memset(szInfo, 0x00, 64);
				if ( TYPE == 0x00 ) 		sprintf(szInfo, "%s", "TS only");
				else if ( TYPE == 0x01 )	sprintf(szInfo, "%s", "Generic stream");
				else if ( TYPE == 0x02 )	sprintf(szInfo, "%s", "TS and Generic");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "Type= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif	
				BWT_EXT = (Packet[3]>>7) & 0x01;
#ifdef WIN32
				(*szResult).bwt = (char)BWT_EXT;
#else
				memset(szInfo, 0x00, 64);
				if ( BWT_EXT == 0x00 )		sprintf(szInfo, "%s", "Normal carrier mode");
				else if ( BWT_EXT == 0x01 ) sprintf(szInfo, "%s", "Extended carrier mode");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "Band width extension= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif				
				S1 = (Packet[3]>>4) & 0x07;
#ifdef WIN32
				//2013/11/07 T2 Lite
				(*szResult).t2_lite_mode = S1;
#endif
				//2012/12/17 bit 0(SISO)
				if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
				{
					if(S1 != 0)
						error_flag = error_flag | (0x1 << E_SISO_);
				}
#ifndef WIN32
				memset(szInfo, 0x00, 64);
				if ( S1 == 0x000 )		sprintf(szInfo, "%s", "T2_SISO");
				else if ( S1 == 0x001 ) sprintf(szInfo, "%s", "T2_MISO");
				else if ( S1 == 0x002 ) sprintf(szInfo, "%s", "Non-T2");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "S1= %d, %s\r\n", S1, szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif	
				S2 = Packet[3]&0x0F;
#ifndef WIN32
				sprintf(debug_string, "S2= %d\r\n", S2);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif	
				FFT_SIZE = (S2>>1);
				//2013/11/07 T2 Lite
				if(S1 == 3 || S1 == 4)
				{
					if(FFT_SIZE == 3)
						FFT_SIZE = 4;
				}
				if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
				{
					//2012/12/17 bit 0(SISO), 1(FFT_SIZE)
					if(FFT_SIZE == 4 || FFT_SIZE == 0x005 || FFT_SIZE == 0x007)
						error_flag = error_flag | (0x1 << E_FFT_SIZE_);
				}
#ifdef WIN32
				if(FFT_SIZE == 0x000)
					(*szResult).fft_mode = 1;
				else if(FFT_SIZE == 0x001 || FFT_SIZE == 0x006)
					(*szResult).fft_mode = 3;
				else if(FFT_SIZE == 0x002)
					(*szResult).fft_mode = 2;
				else if(FFT_SIZE == 0x003)
					(*szResult).fft_mode = 0;
				else if(FFT_SIZE == 0x004)
					(*szResult).fft_mode = 4;
				else if(FFT_SIZE == 0x005 || FFT_SIZE == 0x007)
					(*szResult).fft_mode = 5;
#else
				memset(szInfo, 0x00, 64);
				if ( FFT_SIZE == 0x000 )		sprintf(szInfo, "FFT size= %s", "2K, any gaurd interval");
				else if ( FFT_SIZE == 0x001 )	sprintf(szInfo, "FFT size= %s", "8K, 1/32, 1/16, 1/8 or 1/4");
				else if ( FFT_SIZE == 0x002 )	sprintf(szInfo, "FFT size= %s", "4K, any gaurd interval");
				else if ( FFT_SIZE == 0x003 )	sprintf(szInfo, "FFT size= %s", "1K, any gaurd interval");
				else if ( FFT_SIZE == 0x004 )	sprintf(szInfo, "FFT size= %s", "16K, any gaurd interval");
				else if ( FFT_SIZE == 0x005 )	sprintf(szInfo, "FFT size= %s", "32K, 1/32, 1/16, 1/8 or 1/4");
				else if ( FFT_SIZE == 0x006 )	sprintf(szInfo, "FFT size= %s", "8K, any gaurd interval");
				else if ( FFT_SIZE == 0x007 )	sprintf(szInfo, "FFT size= %s", "32K, 1/128, 19/256 or 19/128");
				else sprintf(szInfo, "FFT size= %s", "unknown");
				sprintf(debug_string, "%s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
				
				L1_REPETITION_FLAG = (Packet[4]>>7) & 0x01;
#ifndef WIN32
				sprintf(debug_string, "L1 repetition flag= %d\r\n", L1_REPETITION_FLAG);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif				
				GUARD_INTERVAL = ((Packet[4]>>4)&0x07);
#ifdef WIN32
				(*szResult).guard_interval = GUARD_INTERVAL;
#else
				memset(szInfo, 0x00, 64);
				if ( GUARD_INTERVAL == 0x000 )		sprintf(szInfo, "%s", "1/32");
				else if ( GUARD_INTERVAL == 0x001 ) sprintf(szInfo, "%s", "1/16");
				else if ( GUARD_INTERVAL == 0x002 ) sprintf(szInfo, "%s", "1/8");
				else if ( GUARD_INTERVAL == 0x003 ) sprintf(szInfo, "%s", "1/4");
				else if ( GUARD_INTERVAL == 0x004 ) sprintf(szInfo, "%s", "1/128");
				else if ( GUARD_INTERVAL == 0x005 ) sprintf(szInfo, "%s", "19/128");
				else if ( GUARD_INTERVAL == 0x006 ) sprintf(szInfo, "%s", "19/256");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "Guard interval= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
				
				PAPR = (Packet[4]&0x0F);
#ifdef WIN32
				(*szResult).papr = PAPR;
#endif
				//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR)
				if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
				{
					if(PAPR != 0)
						error_flag = error_flag | (0x1 << E_PAPR_POS);
				}
#ifndef WIN32
				memset(szInfo, 0x00, 64);
				if ( PAPR == 0x000 )		sprintf(szInfo, "%s", "None");
				else if ( PAPR == 0x001 )	sprintf(szInfo, "%s", "ACE used");
				else if ( PAPR == 0x002 )	sprintf(szInfo, "%s", "TR used");
				else if ( PAPR == 0x003 )	sprintf(szInfo, "%s", "Both ACE and TR used");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "PAPR= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				L1_MOD = ((Packet[5]>>4)&0x0F);
#ifdef WIN32
				(*szResult).l1_modulation = L1_MOD;
#else
				memset(szInfo, 0x00, 64);
				if ( L1_MOD == 0x000 )		sprintf(szInfo, "%s", "BPSK");
				else if ( L1_MOD == 0x001 ) sprintf(szInfo, "%s", "QPSK");
				else if ( L1_MOD == 0x002 ) sprintf(szInfo, "%s", "16QAM");
				else if ( L1_MOD == 0x003 ) sprintf(szInfo, "%s", "64QAM");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "L1 modulation= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				L1_COD = ((Packet[5]>>2)&0x03);
#ifndef WIN32
				memset(szInfo, 0x00, 64);
				if ( L1_COD == 0x000 )		sprintf(szInfo, "%s", "1/2");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "L1 code rate= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				L1_FEC_TYPE = (Packet[5]&0x03);
#ifndef WIN32
				memset(szInfo, 0x00, 64);
				if ( L1_FEC_TYPE == 0x000 ) 	sprintf(szInfo, "%s", "LDPC 16K");
				else sprintf(szInfo, "%s", "unknown");
				sprintf(debug_string, "L1 FEC type= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				L1_POST_SIZE = (Packet[6]<<10) + (Packet[7]<<2) + ((Packet[8]>>6)&0x03);
#ifndef WIN32
				sprintf(debug_string, "L1 post size= %dbits\r\n", L1_POST_SIZE);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
				
				//L1_POST_INFO_SIZE == L1CONF_LEN + L1DYN_CONF_LEN + L1EXT_LEN
				L1_POST_INFO_SIZE = ((Packet[8]&0x3F)<<12) + (Packet[9]<<4) + ((Packet[10]>>4)&0x0F);
#ifndef WIN32
				sprintf(debug_string, "L1 post info size= %dbits\r\n", L1_POST_INFO_SIZE);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				PILOT_PATTERN = (Packet[10]&0x0F);
#ifdef WIN32
				(*szResult).pilot_pattern = PILOT_PATTERN;
#else
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
				sprintf(debug_string, "Pilot pattern= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				TX_ID_AVAILABILITY = Packet[11];
#ifndef WIN32
				sprintf(debug_string, "TX ID availability= %d\r\n", TX_ID_AVAILABILITY);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				CELL_ID = (Packet[12]<<8) + Packet[13];
#ifdef WIN32
				(*szResult).cell_id = CELL_ID;
#else
				sprintf(debug_string, "Cell ID= %d\r\n", CELL_ID);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				NETWORK_ID = (Packet[14]<<8) + Packet[15];
#ifdef WIN32
				(*szResult).network_id = NETWORK_ID;
#else
				sprintf(debug_string, "Network ID= %d\r\n", NETWORK_ID);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				T2_SYSTEM_ID = (Packet[16]<<8) + Packet[17];
#ifdef WIN32
				(*szResult).t2_system_id = T2_SYSTEM_ID;
#else
				sprintf(debug_string, "T2 system ID= %d\r\n", T2_SYSTEM_ID);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				MUM_T2_FRAMES = Packet[18];
#ifdef WIN32
				(*szResult).superframe = MUM_T2_FRAMES;
#else
				sprintf(debug_string, "T2 frames= %d\r\n", MUM_T2_FRAMES);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				NUM_DATA_SYMBOLS = (Packet[19]<<4) + ((Packet[20]>>4)&0x0F);
#ifdef WIN32
				(*szResult).data_symbols = NUM_DATA_SYMBOLS;
#else
				sprintf(debug_string, "Data symbols= %d\r\n", NUM_DATA_SYMBOLS);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				REGEN_FLAG = ((Packet[20]>>1)&0x07);
#ifndef WIN32
				sprintf(debug_string, "Regen flag= %d\r\n", REGEN_FLAG);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				L1_POST_EXTENTION = (Packet[20]&0x01);
#ifndef WIN32
				sprintf(debug_string, "L1 post extention= %d\r\n", L1_POST_EXTENTION);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				NUM_RF = ((Packet[21]>>5)&0x07);
#ifndef WIN32
				sprintf(debug_string, "Number of RF= %d\r\n", NUM_RF);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				CURRENT_RF_IDX = ((Packet[21]>>2)&0x07);
#ifndef WIN32
				sprintf(debug_string, "Current RF index= %d\r\n", CURRENT_RF_IDX);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
				
				RESERVED = ((Packet[21]&0x03)<<8) + Packet[22];
#ifndef WIN32
				sprintf(debug_string, "Reserved= %d\r\n", RESERVED);
				strcat(szResult, debug_string);
#else
				(*szResult).t2_version = ((RESERVED >> 6) & 0xF);
				(*szResult).l1_post_scrambled = ((RESERVED >> 5) & 0x1);
#endif
				
				//L1CONF_LEN
				L1CONF_LEN = (Packet[23]<<8) + Packet[24];
#ifndef WIN32
				sprintf(debug_string, "L1conf len= %dbits\r\n", L1CONF_LEN);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				//L1CONF
				SUB_SLICES_PER_FRAME = (Packet[25]<<7) + ((Packet[26]>>1)&0x7F);
#ifndef WIN32
				sprintf(debug_string, "Sub slices per frame= %d\r\n", SUB_SLICES_PER_FRAME);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				NUM_PLP = ((Packet[26]&0x01)<<7) + ((Packet[27]>>1)&0x7F);
#ifndef WIN32
				sprintf(debug_string, "Number of PLP= %d\r\n", NUM_PLP);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				//DVB-T2 - Multi PLP
				PLP_COUNT = NUM_PLP;
	
				NUM_AUX = ((Packet[27]&0x01)<<3) + ((Packet[28]>>5)&0x07);
#ifndef WIN32
				sprintf(debug_string, "Number of AUX= %d\r\n", NUM_AUX);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				AUX_CONFIG_RFU = ((Packet[28]&0x1F)<<3) + ((Packet[29]>>5)&0x07);
#ifndef WIN32
				sprintf(debug_string, "AUX config rfu= %d\r\n", AUX_CONFIG_RFU);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				////////
				k = 29;
				kk = 4;
	
				//35bits * NUM_RF
				for ( i = 0; i < NUM_RF; i++ )
				{
#ifndef WIN32
					sprintf(debug_string, "----------\r\n");
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					//3bits
					GET_DVB_T2_PARA(RF_IDX,3)
#ifndef WIN32
					sprintf(debug_string, "RF index= %d\r\n", RF_IDX);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					//32bits
					GET_DVB_T2_PARA(FREQUENCY,32)
#ifndef WIN32
					sprintf(debug_string, "Frequency= %d Hz\r\n", FREQUENCY);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#else
					if(i == 0)
					{
						(*szResult).frequency = FREQUENCY;
					}
#endif
				}
	
				//34bits
#ifdef WIN32
				(*szResult).fef_enable = 0;
#endif
				if ( (S2 & 0x01) )
				{
#ifdef WIN32
					(*szResult).fef_enable = 1;
#endif
					//4bits
					GET_DVB_T2_PARA(FEF_TYPE,4)
#ifndef WIN32
					sprintf(debug_string, "FEF type= %d\r\n", FEF_TYPE);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					//22bits
					GET_DVB_T2_PARA(FEF_LENGTH,22)
#ifndef WIN32
					sprintf(debug_string, "FEF length= %d\r\n", FEF_LENGTH);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#else
					(*szResult).fef_length = FEF_LENGTH;
#endif
					//8bits
					GET_DVB_T2_PARA(FEF_INTERVAL,8)
#ifndef WIN32
					sprintf(debug_string, "FEF interval= %d\r\n", FEF_INTERVAL);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#else
					(*szResult).fef_interval = FEF_INTERVAL;
#endif
						//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF)
					if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
					{
						error_flag = error_flag | (0x1 << E_FEF_);
					}
				}
				
				//89bits * NUM_PLP
#ifdef WIN32
				for ( i = 0; i < 8/*NUM_PLP*/; i++ )
				{
					if(i >= NUM_PLP)
					{
						(*szResult).sPlp_Info[i].id = -1;
						continue;
					}
#else
				for ( i = 0; i < NUM_PLP; i++ )
				{
					sprintf(debug_string, "----------\r\n");
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif	
					GET_DVB_T2_PARA(PLP_ID,8)
#ifdef WIN32
					(*szResult).sPlp_Info[i].id = PLP_ID;
#else
					sprintf(debug_string, "PLP ID= %d\r\n", PLP_ID);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_TYPE,3)
#ifdef WIN32
					(*szResult).sPlp_Info[i].type = PLP_TYPE;
					if(PLP_TYPE == 2)
						error_flag = error_flag | (0x1 << E_PLP_TYPE_);
#else
					memset(szInfo, 0x00, 64);
					if ( PLP_TYPE == 0x000 )		sprintf(szInfo, "%s", "Common PLP");
					else if ( PLP_TYPE == 0x001 )	sprintf(szInfo, "%s", "Data PLP Type 1");
					else if ( PLP_TYPE == 0x002 )	sprintf(szInfo, "%s", "Data PLP Type 2");
					else sprintf(szInfo, "%s", "unknown");
					sprintf(debug_string, "PLP type= %s\r\n", szInfo);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_PAYLOAD_TYPE,5)
#ifndef WIN32
					memset(szInfo, 0x00, 64);
					if ( PLP_PAYLOAD_TYPE == 0x000 )		sprintf(szInfo, "%s", "GPFS");
					else if ( PLP_PAYLOAD_TYPE == 0x001 )	sprintf(szInfo, "%s", "GCS");
					else if ( PLP_PAYLOAD_TYPE == 0x002 )	sprintf(szInfo, "%s", "GSE");
					else if ( PLP_PAYLOAD_TYPE == 0x003 )	sprintf(szInfo, "%s", "TS");
					else sprintf(szInfo, "%s", "unknown");
					sprintf(debug_string, "PLP payload type= %s\r\n", szInfo);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(FF_FLAG,1)
#ifndef WIN32
					sprintf(debug_string, "FF flag= %d\r\n", FF_FLAG);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(FIRST_RF_IDX,3)
#ifndef WIN32
					sprintf(debug_string, "First RF index= %d\r\n", FIRST_RF_IDX);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
					
					GET_DVB_T2_PARA(FIRST_FRAME_IDX,8)
#ifndef WIN32
					sprintf(debug_string, "First frame index= %d\r\n", FIRST_FRAME_IDX);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_GROUP_ID,8)
#ifndef WIN32
					sprintf(debug_string, "PLP group ID= %d\r\n", PLP_GROUP_ID);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_COD,3)
#ifdef WIN32
					if ( PLP_COD == 0x000 ) 		(*szResult).sPlp_Info[i].cod = 0;
					else if ( PLP_COD == 0x001 )	(*szResult).sPlp_Info[i].cod = 3;
					else if ( PLP_COD == 0x002 )	(*szResult).sPlp_Info[i].cod = 1;
					else if ( PLP_COD == 0x003 )	(*szResult).sPlp_Info[i].cod = 2;
					else if ( PLP_COD == 0x004 )	(*szResult).sPlp_Info[i].cod = 4;
					else if ( PLP_COD == 0x005 )	(*szResult).sPlp_Info[i].cod = 5;
					else if ( PLP_COD == 0x006 )	(*szResult).sPlp_Info[i].cod = 6;
					else if ( PLP_COD == 0x007 )	(*szResult).sPlp_Info[i].cod = 7;
					else							(*szResult).sPlp_Info[i].cod = -1;
#else
					memset(szInfo, 0x00, 64);
					if ( PLP_COD == 0x000 ) 		sprintf(szInfo, "%s", "1/2");
					else if ( PLP_COD == 0x001 )	sprintf(szInfo, "%s", "3/5");
					else if ( PLP_COD == 0x002 )	sprintf(szInfo, "%s", "2/3");
					else if ( PLP_COD == 0x003 )	sprintf(szInfo, "%s", "3/4");
					else if ( PLP_COD == 0x004 )	sprintf(szInfo, "%s", "4/5");
					else if ( PLP_COD == 0x005 )	sprintf(szInfo, "%s", "5/6");
					else sprintf(szInfo, "%s", "unknown");
					sprintf(debug_string, "PLP code rate= %s\r\n", szInfo);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_MOD,3)
#ifdef WIN32
					(*szResult).sPlp_Info[i].mod = PLP_MOD;
#else
					memset(szInfo, 0x00, 64);
					if ( PLP_MOD == 0x000 ) 		sprintf(szInfo, "%s", "QPSK");
					else if ( PLP_MOD == 0x001 )	sprintf(szInfo, "%s", "16QAM");
					else if ( PLP_MOD == 0x002 )	sprintf(szInfo, "%s", "64QAM");
					else if ( PLP_MOD == 0x003 )	sprintf(szInfo, "%s", "256QAM");
					else sprintf(szInfo, "%s", "unknown");
					sprintf(debug_string, "PLP modulation= %s\r\n", szInfo);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_ROTATION,1)
#ifdef WIN32
					(*szResult).sPlp_Info[i].rot = PLP_ROTATION;
#endif
					sprintf(debug_string, "PLP rotation= %d\r\n", PLP_ROTATION);
					if(PLP_ROTATION == 1)
					{
						if(PLP_MOD == 2 || PLP_MOD == 3)
						{	//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation)
							if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
							{
								error_flag = error_flag | (0x1 << E_ROTATION_);
							}
						}

					}
#ifndef WIN32
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_FEC_TYPE,2)
#ifdef WIN32
					(*szResult).sPlp_Info[i].fec = PLP_FEC_TYPE;
#else
					memset(szInfo, 0x00, 64);
					if ( PLP_FEC_TYPE == 0x000 )		sprintf(szInfo, "%s", "16K LDPC");
					else if ( PLP_FEC_TYPE == 0x001 )	sprintf(szInfo, "%s", "64K LDPC");
					else sprintf(szInfo, "%s", "unknown");
					sprintf(debug_string, "PLP FEC type= %s\r\n", szInfo);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_NUM_BLOCKS_MAX,10)
#ifdef WIN32
					(*szResult).sPlp_Info[i].blk = PLP_NUM_BLOCKS_MAX;
#else
					sprintf(debug_string, "PLP maximum number of blocks= %d\r\n", PLP_NUM_BLOCKS_MAX);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(FRAME_INTERVAL,8)
#ifndef WIN32
					sprintf(debug_string, "Frame interval= %d\r\n", FRAME_INTERVAL);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
					if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
					{
						if(FRAME_INTERVAL != 1)
							//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation), 5(Frame interval)
							error_flag = error_flag | (0x1 << E_FRAME_INTERVAL_);
					}
	
					GET_DVB_T2_PARA(TIME_IL_LENGTH,8)
#ifdef WIN32
					(*szResult).sPlp_Info[i].interleave_length = TIME_IL_LENGTH;
#else
					sprintf(debug_string, "Time IL length= %d\r\n", TIME_IL_LENGTH);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
					if(!(__Sta__->IsAttachedTvbTyp_599() || __Sta__->IsAttachedTvbTyp_598()))
					{
						if(TIME_IL_LENGTH != 0)
							//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation), 5(Frame interval), 6(Time interleaving length),
							error_flag = error_flag | (0x1 << E_INTERLEAVING_LENGTH_);
					}	
	
					GET_DVB_T2_PARA(TIME_IL_TYPE,1)
#ifndef WIN32
					sprintf(debug_string, "Time IL type= %d\r\n", TIME_IL_TYPE);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
					if(TIME_IL_TYPE != 0)
						//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation), 5(Frame interval), 6(Time interleaving length), 7(Time interleaving type)
						error_flag = error_flag | (0x1 << E_INTERLEAVING_TYPE_);
					GET_DVB_T2_PARA(IN_BAND_A_FLAG,1)
#ifndef WIN32
					sprintf(debug_string, "In band A flag= %d\r\n", IN_BAND_A_FLAG);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(IN_BAND_B_FLAG,1)
#ifndef WIN32
					sprintf(debug_string, "In band B flag= %d\r\n", IN_BAND_B_FLAG);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(RESERVED_1,11)
#ifndef WIN32
					sprintf(debug_string, "Reserved 1= %d\r\n", RESERVED_1);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_MODE,2)
#ifndef WIN32
					sprintf(debug_string, "PLP mode= %d\r\n", PLP_MODE);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(STATIC_FLAG,1)
#ifndef WIN32
					sprintf(debug_string, "Static flag= %d\r\n", STATIC_FLAG);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(STATIC_PADDING_FLAG,1)
#if 0
					sprintf(debug_string, "Static padding flag= %d\r\n", STATIC_PADDING_FLAG);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					//DVB-T2 - Multi PLP
					PLP_ID_M[i] = PLP_ID;
					PLP_COD_M[i] = PLP_COD;
					PLP_FEC_TYPE_M[i] = PLP_FEC_TYPE;
					PLP_ROTATION_M[i] = PLP_ROTATION;
					PLP_MOD_M[i] = PLP_MOD;
					if ( PLP_MODE != 0 )
					{
						PLP_MODE_M[i] = PLP_MODE;
					}
				}
	
				GET_DVB_T2_PARA(RESERVED_2,32)
#ifndef WIN32
				sprintf(debug_string, "Reserved 2= %d\r\n", RESERVED_2);
				strcat(szResult, debug_string);
#endif
	
				//32bits * NUM_AUX
				for ( i = 0; i < NUM_AUX; i++ )
				{
#ifndef WIN32
					sprintf(debug_string, "----------\r\n");
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(AUX_STREAM_TAG,4)
#ifndef WIN32
					sprintf(debug_string, "AUX stream tag= %d\r\n", AUX_STREAM_TAG);
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(AUX_PRIVATE_CONF,28)
#ifndef WIN32
					sprintf(debug_string, "AUX private conf= %d2\r\n", AUX_PRIVATE_CONF);
					strcat(szResult, debug_string);
#endif
				}
	
				//L1DYN_CURR_LEN
				L1DYN_CURR_LEN = (Packet[25+(int)ceil(L1CONF_LEN/8.)] << 8) + Packet[25+(int)ceil(L1CONF_LEN/8.)+1];
#ifndef WIN32
				sprintf(debug_string, "L1DYN CURR LEN= %dbits\r\n", L1DYN_CURR_LEN);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				//L1DYN_CURR
				k = 25+(int)ceil(L1CONF_LEN/8.)+1;
				//k += 1;
				//kk = 0;
				kk = -1;
				
				GET_DVB_T2_PARA(FRAME_IDX,8)
#ifndef WIN32
				sprintf(debug_string, "Frame index= %d\r\n", FRAME_IDX);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				GET_DVB_T2_PARA(SUB_SLICE_INTERVAL,22)
#ifndef WIN32
				sprintf(debug_string, "Sub slice interval= %d\r\n", SUB_SLICE_INTERVAL);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				GET_DVB_T2_PARA(TYPE_T2_START,22)
#ifndef WIN32
				sprintf(debug_string, "Type t2 start= %d\r\n", TYPE_T2_START);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				GET_DVB_T2_PARA(L1_CHANGE_COUNTER,8)
#ifndef WIN32
				sprintf(debug_string, "L1 change counter= %d\r\n", L1_CHANGE_COUNTER);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				GET_DVB_T2_PARA(START_RF_IDX,3)
#ifndef WIN32
				sprintf(debug_string, "Start RF index= %d\r\n", START_RF_IDX);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				GET_DVB_T2_PARA(RESERVED_1,8)
#ifndef WIN32
				sprintf(debug_string, "Reserved 1= %d\r\n", RESERVED_1);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				for ( i = 0; i <  NUM_PLP; i++ )
				{
#ifndef WIN32
					sprintf(debug_string, "----------\r\n");
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_ID,8)
#ifndef WIN32
					sprintf(debug_string, "PLP ID= %d\r\n", PLP_ID);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_START,22)
#ifndef WIN32
					sprintf(debug_string, "PLP start= %d\r\n", PLP_START);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(PLP_NUM_BLOCKS,10)
#ifndef WIN32
					sprintf(debug_string, "PLP number of blocks= %d\r\n", PLP_NUM_BLOCKS);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					GET_DVB_T2_PARA(RESERVED_2,8)
#ifndef WIN32
					sprintf(debug_string, "Reserved 2= %d\r\n", RESERVED_2);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
	
					//DVB-T2 - Multi PLP
					PLP_NUM_BLOCKS_M[i] = PLP_NUM_BLOCKS;
				}
	
				GET_DVB_T2_PARA(RESERVED_3,8)
#ifndef WIN32
				sprintf(debug_string, "Reserved 3= %d\r\n", RESERVED_3);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
	
				for ( i = 0; i < NUM_AUX; i++ )
				{
					GET_DVB_T2_PARA(AUX_PRIVATE_DYN,48)
#ifndef WIN32
					sprintf(debug_string, "AUX private DYN= %d\r\n", AUX_PRIVATE_DYN);
					CHK_DVB_T2_PARAM
					strcat(szResult, debug_string);
#endif
				}
	
				//L1EXT_LEN = 16bits
				//L1EXT ...(8 * ceil(L1EXT_LEN/8])
				//CRC-32
				//L1 Padding
			}
			else if ( Header[0] == 32 && t2mi_analysis_timestamp == 0 )
			{
				t2mi_analysis_timestamp = 1;
	
				memset(szInfo, 0x00, 64);
	
				bw = (Packet[0]&0x0F);
#ifdef WIN32
				(*szResult).bw = bw;
#else
				if ( bw == 0x0 ) 
				{
					//subseconds *= (1./131.);
					sprintf(szInfo, "1.7MHz");
				}
				else if ( bw == 0x1 )
				{
					//subseconds *= (1./40.);
					sprintf(szInfo, "5MHz");
				}
				else if ( bw == 0x2 )
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
				else if ( bw == 0x5 )
				{
					//subseconds *= (1./80.);
					sprintf(szInfo, "10MHz");
				}
				else
				{
					sprintf(szInfo, "unknown");
				}
	
				sprintf(debug_string, "Band width= %s\r\n", szInfo);
				CHK_DVB_T2_PARAM
				strcat(szResult, debug_string);
#endif
			}
			else
			{
			}
		}
	
		if ( fid_t2mi )
		{
			free(fid_t2mi);
		}
	
		if ( Packet )
		{
			free(Packet);
		}
	
		if ( szBuf )
		{
			free(szBuf);
		}
#ifdef WIN32
		(*szResult).error_flag = error_flag;
#else		
		//2012/12/17 bit 0(SISO), 1(FFT_SIZE), 2(PAPR), 3(FEF), 4(Rotation), 5(Frame interval), 6(Time interleaving length), 7(Time interleaving type)
		if(error_flag > 0)
		{
			char err_msg[65526];
			int err_indx = 1;
			memset(err_msg, 0x00, 65526);
			sprintf(err_msg, "This stream is not supported!\r\nThis stream has parameters that our modulator does not support.\r\nThe parameters are as following.\r\n");
			for(int e_i = 0; e_i < 32; e_i++)
			{
				if(((error_flag >> e_i) & 0x1))
				{
					switch(e_i)
					{
					case 0:	//SISO
						sprintf(debug_string, "%d. The value of S1 field should be SISO.\r\n", err_indx);
						break;
					case 1: //FFT SIZE
						sprintf(debug_string, "%d. The value of FFT size should be 1K, 2K, 4K or 8K.\r\n", err_indx);
						break;
					case 2:	//PAPR
						sprintf(debug_string, "%d. The value of PAPR should be None.\r\n", err_indx);
						break;
					case 3: //FEF
						sprintf(debug_string, "%d. FEF parts not support.\r\n", err_indx);
						break;
					case 4:	//Rotation
						sprintf(debug_string, "%d. Rotation is supported in PLP modulation [QPSK, 16QAM]\r\n", err_indx);
						break;
					case 5:	//Frame interval
						sprintf(debug_string, "%d. The value of frame interval should be 1.\r\n", err_indx);
						break;
					case 6:	//Time interleaving length
						sprintf(debug_string, "%d. The value of time interleaving length should be 0.\r\n", err_indx);
						break;
					case 7:	//Time interleaving Type
						sprintf(debug_string, "%d. The value of time interleaving type should be 0.\r\n", err_indx);
						break;
					}
					strcat(err_msg, debug_string);
					err_indx++;
				}

			}
			if(strlen(szResult) < (65526 - 1024))
			{
				strcat(err_msg, "\r\n[T2MI Stream Information]\r\n");
				strcat(err_msg, szResult);
				strcpy(szResult, err_msg);
				//sprintf(szResult, "%s\r\n%s", err_msg, szResult);
			}
		}
#endif


#if 1
		double N_mod, frame_closing_symbol, N_P2, C_P2, L_data, C_data, C_FC, D_L1, C_tot, N_ldpc, K_ldpc, K_bch, N_FEC_FRAME, R_bits, GuardInterval, IFFT_SIZE, T, T_F, LDPC_code;
		double max_bitrate;
	
		//DVB-T2 - Multi PLP
		for ( i = 0; i < PLP_COUNT; i++ )
		{
				PLP_MOD = PLP_MOD_M[i];
				PLP_COD = PLP_COD_M[i];
				PLP_FEC_TYPE = PLP_FEC_TYPE_M[i];
				//high(2) or normal(1) efficiency mode, 0(not specified if T2_VERSION == "0000")
				if ( PLP_MODE_M[i] != 0 )//???
				{
					NORMAL_MODE = (PLP_MODE_M[i] == 2 ? 0 : 1); 
#ifdef WIN32
					(*szResult).sPlp_Info[i].hem = (NORMAL_MODE == 0 ? 1 : 0);
#endif
				}

				PLP_NUM_BLOCKS = PLP_NUM_BLOCKS_M[i];
	
		N_mod = 2;//QPSK, bits
		if ( PLP_MOD == 0x000 ) 		N_mod = 2;
		else if ( PLP_MOD == 0x001 )	N_mod = 4;
		else if ( PLP_MOD == 0x002 )	N_mod = 6;
		else if ( PLP_MOD == 0x003 )	N_mod = 8;
	
		frame_closing_symbol = 1;
		if ( GUARD_INTERVAL == 0x004 )//==1/128
		{
			frame_closing_symbol = 0;
		}
	
		N_P2 = 16;//==FFT 1k
		if ( FFT_SIZE == 3 )						N_P2 = 16;
		else if ( FFT_SIZE == 0 )					N_P2 = 8;
		else if ( FFT_SIZE == 2 )					N_P2 = 4;
		else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	N_P2 = 2;
		else if ( FFT_SIZE == 4 )					N_P2 = 1;
		else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	N_P2 = 1;
	
		C_P2 = 558;//==FFT 1k, SISO
		if ( S1 == 0x000 ) //==SISO
		{
			if ( FFT_SIZE == 3 )						C_P2 = 558;
			else if ( FFT_SIZE == 0 )					C_P2 = 1118;
			else if ( FFT_SIZE == 2 )					C_P2 = 2236;
			else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	C_P2 = 4472;
			else if ( FFT_SIZE == 4 )					C_P2 = 8944;
			else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	C_P2 = 22432;
		}
		else if ( S1 == 0x001 ) //==MISO
		{
			if ( FFT_SIZE == 3 )						C_P2 = 546;
			else if ( FFT_SIZE == 0 )					C_P2 = 1098;
			else if ( FFT_SIZE == 2 )					C_P2 = 2198;
			else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	C_P2 = 4398;
			else if ( FFT_SIZE == 4 )					C_P2 = 8814;
			else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	C_P2 = 17612;
		}
	
		L_data = NUM_DATA_SYMBOLS;
	
		C_data = 764;//FFT 1k, PP1
		if ( FFT_SIZE == 3 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_data = 764;
			else if ( PILOT_PATTERN == 0x001 )	C_data = 768;
			else if ( PILOT_PATTERN == 0x002 )	C_data = 798;
			else if ( PILOT_PATTERN == 0x003 )	C_data = 804;
			else if ( PILOT_PATTERN == 0x004 )	C_data = 818;
			else if ( PILOT_PATTERN == 0x005 )	C_data = 0;//never used
			else if ( PILOT_PATTERN == 0x006 )	C_data = 0;
			else if ( PILOT_PATTERN == 0x007 )	C_data = 0;
		}
		else if ( FFT_SIZE == 0 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_data = 1522;
			else if ( PILOT_PATTERN == 0x001 )	C_data = 1532;
			else if ( PILOT_PATTERN == 0x002 )	C_data = 1596;
			else if ( PILOT_PATTERN == 0x003 )	C_data = 1602;
			else if ( PILOT_PATTERN == 0x004 )	C_data = 1632;
			else if ( PILOT_PATTERN == 0x005 )	C_data = 0;
			else if ( PILOT_PATTERN == 0x006 )	C_data = 1646;
			else if ( PILOT_PATTERN == 0x007 )	C_data = 0;
		}
		else if ( FFT_SIZE == 2 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_data = 3084;
			else if ( PILOT_PATTERN == 0x001 )	C_data = 3092;
			else if ( PILOT_PATTERN == 0x002 )	C_data = 3228;
			else if ( PILOT_PATTERN == 0x003 )	C_data = 3234;
			else if ( PILOT_PATTERN == 0x004 )	C_data = 3298;
			else if ( PILOT_PATTERN == 0x005 )	C_data = 0;
			else if ( PILOT_PATTERN == 0x006 )	C_data = 3328;
			else if ( PILOT_PATTERN == 0x007 )	C_data = 0;
		}
		else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )//8k
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 6208;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 6124;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 6494;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 6498;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 6634;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 6698;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 6698;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 6296;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 6298;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 6584;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 6588;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 6728;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 6788;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 6788;
			}
		}
		else if ( FFT_SIZE == 4 )
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 12418;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 12436;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 12988;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 13002;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 13272;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 13288;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 13416;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 13406;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 12678;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 12698;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 13262;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 13276;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 13552;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 13568;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 13698;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 13688;
			}
		}
		else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 0;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 24886;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 26022;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 26592;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 26836;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 26812;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_data = 0;
				else if ( PILOT_PATTERN == 0x001 )	C_data = 25412;
				else if ( PILOT_PATTERN == 0x002 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x003 )	C_data = 26572;
				else if ( PILOT_PATTERN == 0x004 )	C_data = 0;
				else if ( PILOT_PATTERN == 0x005 )	C_data = 27152;
				else if ( PILOT_PATTERN == 0x006 )	C_data = 27404;
				else if ( PILOT_PATTERN == 0x007 )	C_data = 27376;
			}
		}
	
		C_FC = 402;//FFT 1k, PP1
		if ( FFT_SIZE == 3 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_FC = 402;
			else if ( PILOT_PATTERN == 0x001 )	C_FC = 654;
			else if ( PILOT_PATTERN == 0x002 )	C_FC = 490;
			else if ( PILOT_PATTERN == 0x003 )	C_FC = 707;
			else if ( PILOT_PATTERN == 0x004 )	C_FC = 544;
			else if ( PILOT_PATTERN == 0x005 )	C_FC = 0;//never used
			else if ( PILOT_PATTERN == 0x006 )	C_FC = 0;
			else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
		}
		else if ( FFT_SIZE == 0 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_FC = 804;
			else if ( PILOT_PATTERN == 0x001 )	C_FC = 1309;
			else if ( PILOT_PATTERN == 0x002 )	C_FC = 960;
			else if ( PILOT_PATTERN == 0x003 )	C_FC = 1415;
			else if ( PILOT_PATTERN == 0x004 )	C_FC = 1088;
			else if ( PILOT_PATTERN == 0x005 )	C_FC = 0;
			else if ( PILOT_PATTERN == 0x006 )	C_FC = 1396;
			else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
		}
		else if ( FFT_SIZE == 2 )
		{
			if ( PILOT_PATTERN == 0x000 )		C_FC = 1609;
			else if ( PILOT_PATTERN == 0x001 )	C_FC = 2619;
			else if ( PILOT_PATTERN == 0x002 )	C_FC = 1961;
			else if ( PILOT_PATTERN == 0x003 )	C_FC = 2381;
			else if ( PILOT_PATTERN == 0x004 )	C_FC = 2177;
			else if ( PILOT_PATTERN == 0x005 )	C_FC = 0;
			else if ( PILOT_PATTERN == 0x006 )	C_FC = 2792;
			else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
		}
		else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )//8k
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 3218;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 5283;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 3922;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 5662;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 4353;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 5585;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 3264;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 5312;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 3978;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 5742;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 4416;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 5664;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
		}
		else if ( FFT_SIZE == 4 )
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 6437;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 10476;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 7845;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 11324;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 8709;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 11801;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 11170;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 6573;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 10697;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 8011;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 11563;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 8893;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 12051;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 11406;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
		}
		else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )
		{
			if ( BWT_EXT == 0x00 )
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 0;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 20952;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 22649;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 23603;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
			else
			{
				if ( PILOT_PATTERN == 0x000 )		C_FC = 0;
				else if ( PILOT_PATTERN == 0x001 )	C_FC = 21395;
				else if ( PILOT_PATTERN == 0x002 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x003 )	C_FC = 23127;
				else if ( PILOT_PATTERN == 0x004 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x005 )	C_FC = 24102;
				else if ( PILOT_PATTERN == 0x006 )	C_FC = 0;
				else if ( PILOT_PATTERN == 0x007 )	C_FC = 0;
			}
		}
	
		D_L1 = 1840 + L1_POST_SIZE;
	
		if ( frame_closing_symbol )
		{
			C_tot = N_P2 * C_P2 + (L_data - 1) * C_data + C_FC - D_L1;
		}
		else
		{
			C_tot = N_P2 * C_P2 + L_data * C_data - D_L1;
		}
	
		N_ldpc = (PLP_FEC_TYPE == 0x001 ?  64800 : 16200);
	
		K_ldpc = 32400;
		if ( PLP_FEC_TYPE == 0x001 )
		{
			if ( PLP_COD == 0x000 ) 		K_ldpc = 32400;
			else if ( PLP_COD == 0x001 )	K_ldpc = 38880;
			else if ( PLP_COD == 0x002 )	K_ldpc = 43200;
			else if ( PLP_COD == 0x003 )	K_ldpc = 48600;
			else if ( PLP_COD == 0x004 )	K_ldpc = 51840;
			else if ( PLP_COD == 0x005 )	K_ldpc = 54000;
		}
		else
		{
			if ( PLP_COD == 0x000 ) 		K_ldpc = 7200;
			else if ( PLP_COD == 0x001 )	K_ldpc = 9720;
			else if ( PLP_COD == 0x002 )	K_ldpc = 10800;
			else if ( PLP_COD == 0x003 )	K_ldpc = 11880;
			else if ( PLP_COD == 0x004 )	K_ldpc = 12600;
			else if ( PLP_COD == 0x005 )	K_ldpc = 13320;
			else if ( PLP_COD == 0x006 )	K_ldpc = 5400;
			else if ( PLP_COD == 0x007 )	K_ldpc = 6480;
		}
	
		K_bch = 32208;//normal FECFRAME, 1/2
		if ( PLP_FEC_TYPE == 0x001 )
		{
			if ( PLP_COD == 0x000 ) 		K_bch = 32208;
			else if ( PLP_COD == 0x001 )	K_bch = 38688;
			else if ( PLP_COD == 0x002 )	K_bch = 43040;
			else if ( PLP_COD == 0x003 )	K_bch = 48408;
			else if ( PLP_COD == 0x004 )	K_bch = 51648;
			else if ( PLP_COD == 0x005 )	K_bch = 53840;
		}
		else
		{
			if ( PLP_COD == 0x000 ) 		K_bch = 7032;
			else if ( PLP_COD == 0x001 )	K_bch = 9552;
			else if ( PLP_COD == 0x002 )	K_bch = 10632;
			else if ( PLP_COD == 0x003 )	K_bch = 11712;
			else if ( PLP_COD == 0x004 )	K_bch = 12432;
			else if ( PLP_COD == 0x005 )	K_bch = 13152;
			else if ( PLP_COD == 0x006 )	K_bch = 5232;
			else if ( PLP_COD == 0x007 )	K_bch = 6312;
		}
		N_FEC_FRAME = PLP_NUM_BLOCKS; //???
		R_bits = (N_ldpc - K_bch + 80) * N_FEC_FRAME;
	
		GuardInterval = 1/32.;
		if ( GUARD_INTERVAL == 0x000 )		GuardInterval = 1./32.;
		else if ( GUARD_INTERVAL == 0x001 ) GuardInterval = 1./16.;
		else if ( GUARD_INTERVAL == 0x002 ) GuardInterval = 1./8.;
		else if ( GUARD_INTERVAL == 0x003 ) GuardInterval = 1./4.;
		else if ( GUARD_INTERVAL == 0x004 ) GuardInterval = 1./128.;
		else if ( GUARD_INTERVAL == 0x005 ) GuardInterval = 19./128.;
		else if ( GUARD_INTERVAL == 0x006 ) GuardInterval = 19./256.;
	
		IFFT_SIZE = 1.;//1K ???
		if ( FFT_SIZE == 3 )						IFFT_SIZE = 1;
		else if ( FFT_SIZE == 0 )					IFFT_SIZE = 2.;
		else if ( FFT_SIZE == 2 )					IFFT_SIZE = 4.;
		else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	IFFT_SIZE = 8.;
		else if ( FFT_SIZE == 4 )					IFFT_SIZE = 16.;
		else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	IFFT_SIZE = 32.;
		IFFT_SIZE *= 1024;
	
		T = 71./131.;
		if ( bw == 0x0 )		T = 71./131.;
		else if ( bw == 0x1 )	T = 7./40.;
		else if ( bw == 0x2 )	T = 7./48.;
		else if ( bw == 0x3 )	T = 1./8.;
		else if ( bw == 0x4 )	T = 7./64.;
		else if ( bw == 0x5 )	T = 7./80.;
	
		//by user selection
		if ( TL_DVB_T2_BW	== 0x0 )		T = 7./48.;//6MHz
		else if ( TL_DVB_T2_BW  == 0x1 )	T = 1./8.;//7MHz
		else if ( TL_DVB_T2_BW  == 0x2 )	T = 7./64.;//8MHz
		else if ( TL_DVB_T2_BW  == 0x3 )	T = 7./40.;//5MHz
		else if ( TL_DVB_T2_BW  == 0x4 )	T = 71./131.;//1.7MHz
		else if ( TL_DVB_T2_BW  == 0x5 )	T = 7./80.;//10MHz
		T *= (1/1000000.);//sec.
	
		T_F = (2048. + (1. + GuardInterval) * IFFT_SIZE * (N_P2 + L_data)) * T;
		
		max_bitrate = (N_mod * C_tot - R_bits) / T_F;
	
		if ( PLP_FEC_TYPE == 0x001 )
		{
			if ( PLP_COD == 0x000 ) 		LDPC_code = 1./2.;
			else if ( PLP_COD == 0x001 )	LDPC_code = 3./5.;
			else if ( PLP_COD == 0x002 )	LDPC_code = 2./3.;
			else if ( PLP_COD == 0x003 )	LDPC_code = 3./4.;
			else if ( PLP_COD == 0x004 )	LDPC_code = 4./5.;
			else if ( PLP_COD == 0x005 )	LDPC_code = 5./6.;
		}
		else
		{
			if ( PLP_COD == 0x000 ) 		LDPC_code = 4./9.;
			else if ( PLP_COD == 0x001 )	LDPC_code = 3./5.;
			else if ( PLP_COD == 0x002 )	LDPC_code = 2./3.;
			else if ( PLP_COD == 0x003 )	LDPC_code = 11./15.;
			else if ( PLP_COD == 0x004 )	LDPC_code = 7./9.;
			else if ( PLP_COD == 0x005 )	LDPC_code = 37./45.;
			else if ( PLP_COD == 0x006 )	LDPC_code = 1./3.;
			else if ( PLP_COD == 0x007 )	LDPC_code = 2./5.;
		}
	
		avg_bitrate = (N_ldpc * N_FEC_FRAME * LDPC_code * (K_bch / K_ldpc) * ((K_bch - 80.) / K_bch));
		avg_bitrate /= T_F;
		if ( NORMAL_MODE == 0 )
		{
			avg_bitrate *= (188./187.);
		}
		if ( NULL_PACKET_DELETION == 1 )
		{
			avg_bitrate *= (188./189.);
		}
	
		if (S2 & 0x01)
		{
			double Tfef = T*FEF_LENGTH;
			int Nfef = MUM_T2_FRAMES/FEF_INTERVAL;
			double Tsf = T_F*MUM_T2_FRAMES + Tfef*Nfef;
			double ratio = MUM_T2_FRAMES*T_F/Tsf;
			avg_bitrate *= ratio;
		}


		//fixed
		__FIf__->TL_gBitrate = (int)avg_bitrate;
	
		//DVB-T2 - Multi PLP
		PLP_BITRATE_M[i] = (int)avg_bitrate;
#ifdef WIN32
		(*szResult).sPlp_Info[i].bitrate = (int)avg_bitrate;
#endif
		}
#endif
		
		//Set PID
#ifdef WIN32
		__FIf__->TSPL_SET_MH_PID(13, (*szResult).pid);
#else
		__FIf__->TSPL_SET_MH_PID(13, PID_);
#endif

		//DVB-T2 - Multi PLP
		avg_bitrate = 0;
		for ( i = 0; i < PLP_COUNT; i++ )
		{
			avg_bitrate += PLP_BITRATE_M[i];
		}
	
	return	avg_bitrate;
#endif
	return	0;
}

static unsigned char _tlv_fft_to_t2_fft(unsigned char tlv_fft)
{
	if (tlv_fft == 0)
		return DVBT2_FFT_SIZE_1K;
	else if (tlv_fft == 1)
		return DVBT2_FFT_SIZE_2K;
	else if (tlv_fft == 2)
		return DVBT2_FFT_SIZE_4K;
	else if (tlv_fft == 4)
		return DVBT2_FFT_SIZE_16K;
	else if (tlv_fft == 5)
		return DVBT2_FFT_SIZE_32K;
	else 
		return DVBT2_FFT_SIZE_8K;
}

int	CHldFmtrDvbT_::SetT2MiParam(
	int _BW, int _FFT_SIZE, int _GUARD_INTERVAL, int _L1_MOD, int _PILOT_PATTERN, int _BW_EXT, double _FREQUENCY,
	int _NETWORK_ID, int _T2_SYSTEM_ID, int _CELL_ID, int _S1, int _PLP_MOD, int _PLP_COD, int _PLP_FEC_TYPE, int _HEM, 
	int _NUM_T2_FRAMES, int _NUM_DATA_SYMBOLS, int _PLP_NUM_BLOCKS, int _PID
	//DVB-T2 - Multi PLP MUX
#ifdef _MULTI_PLP_MUX_0
	, int _PLP_ROTATION, int _PLP_COUNT, int _PLP_ID, int _PLP_BITRATE, int _PLP_TS_BITRATE, char *_PLP_TS, int _PLP_TI_LENGTH
#endif
	)
{
#if	0
	__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "SetT2MiParam : not used");
#else
		TL_DVB_T2_BW = _BW;
		FFT_SIZE = _tlv_fft_to_t2_fft(_FFT_SIZE);
		GUARD_INTERVAL = _GUARD_INTERVAL;
		L1_MOD = _L1_MOD;
		PILOT_PATTERN = _PILOT_PATTERN;
		BW_EXT = _BW_EXT;
		FREQUENCY = (unsigned long)_FREQUENCY;
		NETWORK_ID = _NETWORK_ID;
		T2_SYSTEM_ID = _T2_SYSTEM_ID;
		CELL_ID = _CELL_ID;
		S1 = _S1;
		PLP_MOD = _PLP_MOD;
		PLP_COD = _PLP_COD;
		PLP_FEC_TYPE = _PLP_FEC_TYPE;
		HEM = _HEM;
		NUM_T2_FRAMES = _NUM_T2_FRAMES;
		NUM_DATA_SYMBOLS = _NUM_DATA_SYMBOLS;
		PLP_NUM_BLOCKS = _PLP_NUM_BLOCKS;
		PID = _PID;
		//Set PID
		__FIf__->TSPL_SET_MH_PID(13, _PID);
	
		//DVB-T2 - Multi PLP MUX
#ifdef _MULTI_PLP_MUX_0
		if ( _PLP_COUNT == 0 )
		{
			PLP_COUNT = 0;
		}
		PLP_ID_M[PLP_COUNT] = _PLP_ID;
		PLP_ROTATION_M[PLP_COUNT] = _PLP_ROTATION;
		PLP_MOD_M[PLP_COUNT] = _PLP_MOD;
		PLP_COD_M[PLP_COUNT] = _PLP_COD;
		PLP_FEC_TYPE_M[PLP_COUNT] = _PLP_FEC_TYPE;
		PLP_MODE_M[PLP_COUNT] = _HEM;
		PLP_NUM_BLOCKS_M[PLP_COUNT] = _PLP_NUM_BLOCKS;
		PLP_BITRATE_M[PLP_COUNT] = _PLP_BITRATE;
		PLP_TS_BITRATE_M[PLP_COUNT] = _PLP_TS_BITRATE;
		sprintf(PLP_TS_M[PLP_COUNT], "%s", _PLP_TS);
		PLP_TIME_IL_LENGTH_M[PLP_COUNT] = _PLP_TI_LENGTH;

		++PLP_COUNT;
#endif
	
	/*
		//TEST
		PLP_COUNT = 1;
		PLP_ID_M[0] = 0;
		PLP_MOD_M[0] = 0;
		PLP_COD_M[0] = 2;
		PLP_FEC_TYPE_M[0] = 0;
		PLP_MODE_M[0] = 1;
		PLP_NUM_BLOCKS_M[0] = 7;
		PLP_BITRATE_M[0] = 4272423;
		PLP_TS_BITRATE_M[0] = 4040345;
		//sprintf(PLP_TS_M[0], "%s", "C:\\ts\\Dvb_T2_Stream\\DVD_4M.ts");
		sprintf(PLP_TS_M[0], "%s", "C:\\ts\\Dvb_T2_Stream\\dump_plp_hem_plp0.ts");
		
#if 1
		PLP_COUNT = 2;
		PLP_ID_M[1] = 1;
		PLP_MOD_M[1] = 3;
		PLP_COD_M[1] = 5;
		PLP_FEC_TYPE_M[1] = 1;
		PLP_MODE_M[1] = 1;
		PLP_NUM_BLOCKS_M[1] = 8;
		PLP_BITRATE_M[1] = 24876583;
		PLP_TS_BITRATE_M[1] = 24128371;
		//sprintf(PLP_TS_M[1], "%s", "C:\\ts\\Dvb_T2_Stream\\bbcall_tennis.trp");
		sprintf(PLP_TS_M[1], "%s", "C:\\ts\\Dvb_T2_Stream\\dump_plp_hem_plp1.ts");
#endif
	*/

	return	0;
#endif
	return	0;
}
int	CHldFmtrDvbT_::GetT2MiParam(int *num_data_symbol, int *plp_num_block)
{
#if	0
	__HLog__->HldPrint_1_s("++++++++---->>>> Donot use this : ", "GetT2MiParam : not used");
#else
	/*
	int _FFT_SIZE;
	if ( FFT_SIZE == 0 )		_FFT_SIZE = 3;//1K
	else if ( FFT_SIZE == 1 ) _FFT_SIZE = 0;//2K
	else if ( FFT_SIZE == 2 ) _FFT_SIZE = 2;//4K
	else if (FFT_SIZE == 4)
		_FFT_SIZE = DVBT2_FFT_SIZE_16K;
	else if (FFT_SIZE == 5)
		_FFT_SIZE = DVBT2_FFT_SIZE_32K;
	else //8K
	{
		if ( GUARD_INTERVAL <= 3 )
		{
			_FFT_SIZE = 1;
		}
		else
		{
			_FFT_SIZE = 6;
		}
	}
*/
	//2011/7/13 added
	int BW;
	if ( TL_DVB_T2_BW  == 0 )		BW = 2;//6MHz
	else if ( TL_DVB_T2_BW  == 1 )	BW = 3;//7MHz
	else if ( TL_DVB_T2_BW  == 2 )	BW = 4;//8MHz
	else if ( TL_DVB_T2_BW  == 3 )	BW = 1;//5MHz
	else if ( TL_DVB_T2_BW  == 4 )	BW = 0;//1.7MHz
	else							BW = 5;//10MHz

	// huy: modify 201301
	int rtcd = 0;
	 rtcd = dvbt2_SearchingParamater(0, 1, L1_MOD,
		BW,
		BW_EXT,
		FFT_SIZE,
		GUARD_INTERVAL,
		PILOT_PATTERN,
		PLP_COUNT,
		&PLP_FEC_TYPE_M[0],
		&PLP_COD_M[0],
		&PLP_MOD_M[0],
		&PLP_TS_BITRATE_M[0],
		num_data_symbol, 
		plp_num_block);

	return rtcd;
#endif
	return	0;
}



