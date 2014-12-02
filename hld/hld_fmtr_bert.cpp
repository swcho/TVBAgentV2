
#if defined(WIN32)
#include	<Windows.h>
#else
#define _FILE_OFFSET_BITS 64
#endif
#include	<stdio.h>

#ifdef WIN32
#else
#include	<stdlib.h>
#include	"../include/lld_const.h"
#endif

#include	"../include/common_def.h"
#include	"../include/hld_structure.h"
#include	"hld_fmtr_bert.h"

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrBert::CHldFmtrBert(void)
{
	TL_pbPRBSBuffer_14_15 = NULL;
	TL_pbPRBSBuffer_18_23 = NULL;
	TL_ErrorCount = 0;
	TL_TotalCount = 0;
	TL_BER = 0.0;
	TL_ErrorInsert = 0;
	TL_ErrorInsert2 = 0;

	//fp1 = fopen("c:\\work\\bert_1.dat", "w");
	fp1 = NULL;
	//fp2 = fopen("c:\\ts\\bert_2.dat", "wb");
	fp2 = NULL;


}

CHldFmtrBert::~CHldFmtrBert()
{
	if ( TL_pbPRBSBuffer_14_15 )
	{
		free(TL_pbPRBSBuffer_14_15);
	}

	if ( TL_pbPRBSBuffer_18_23 )
	{
		free(TL_pbPRBSBuffer_18_23);
	}
}
void	CHldFmtrBert::SetCommonMethod_2(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

void	CHldFmtrBert::InitBertCariable_OnRecStart(void)
{
#if 1
	if ( __FIf__->_VarTstPktTyp() >= TS_HEAD_184_ALL_0 )
	{
		TL_ErrorCount = 0;
		TL_TotalCount = 0;
		TL_BER = 0.0;

		__FIf__->InitPlayBufVariables_OnRec();
	}
#else
#endif
}
void	CHldFmtrBert::InitBertVariable_OnPlayStart(void)
{
	int	i;

	if ( __FIf__->_VarTstPktTyp() >= TS_HEAD_184_ALL_0 )
	{
		TL_ErrorCount = 0;
		TL_TotalCount = 0;
		TL_BER = 0.0;

		//FIXED - BERT
		for ( i = 0; i < PRBS_15; i++ )
		{
			p_15[i]=seed_15[i];
		}

		for ( i = 0; i < PRBS_23; i++ )
		{
			p_23[i]=seed_23[i];
		}
		p_15_init = 0;
		p_23_init = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//FIXED - BERT
void CHldFmtrBert::TL_Make_PRBS_2_15(unsigned char *prbs_15, unsigned char *p_15, int payload_size)
{
	//unsigned char seed_15[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	//unsigned char p_15[15];
	unsigned char np_15[PRBS_15];
	//unsigned char prbs_15[PKTSIZE*8];

	//fp = fopen("c:\\prbs_14_15.dat", "wb");
	//fp = fopen("c:\\bert_14_15.dat", "wb");

	int i, j, nLen = PRBS_15;
	for ( i = 0; i < payload_size*8; i++ )
	{
		if ( p_15_init == 0 )
		{
			for ( j = 0; j < nLen; j++ )
			{
				p_15[j]=seed_15[j];
			}
			p_15_init = 1;
		}
		else
		{
			for ( j = 0; j < (nLen-1); j++ )
			{
				np_15[j+1] = p_15[j];
			}
			np_15[0] = p_15[14]^p_15[13];

			for ( j = 0; j < nLen; j++ )
			{
				p_15[j]=np_15[j];
			}
		}
		prbs_15[i] = !p_15[0];
	}

	for ( i = 0; i < payload_size; i++ )
	{
		prbs_15[i] = (prbs_15[i*8+0]<<7) + (prbs_15[i*8+1]<<6) + (prbs_15[i*8+2]<<5) + (prbs_15[i*8+3]<<4) + 
			(prbs_15[i*8+4]<<3) + (prbs_15[i*8+5]<<2) + (prbs_15[i*8+6]<<1) + prbs_15[i*8+7];
	}

	//fwrite(prbs_15, 1, nPlayloadSize*8, fp);
	//fwrite(prbs_15, 1, nPlayloadSize, fp);
	//fclose(fp);
}

void CHldFmtrBert::TL_Make_PRBS_2_23(unsigned char *prbs_23, unsigned char *p_23, int payload_size)
{
	//unsigned char seed_23[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}; 
	//unsigned char p_23[23];
	unsigned char np_23[PRBS_23];
	//unsigned char prbs_23[PKTSIZE*8];

	//fp = fopen("c:\\prbs_14_15.dat", "wb");
	//fp = fopen("c:\\bert_14_15.dat", "wb");

	int i, j, nLen = PRBS_23;
	for ( i = 0; i < payload_size*8; i++ )
	{
		if ( p_23_init == 0 )
		{
			for ( j = 0; j < nLen; j++ )
			{
				p_23[j]=seed_23[j];
			}
			p_23_init = 1;
		}
		else
		{
			for ( j = 0; j < nLen-1; j++ )
			{
				np_23[j+1] = p_23[j];
			}
			np_23[0] = p_23[22]^p_23[17];

			for ( j = 0; j < nLen; j++ )
			{
				p_23[j]=np_23[j];
			}
		}
		prbs_23[i] = !p_23[0];
	}

	for ( i = 0; i < payload_size; i++ )
	{
		prbs_23[i] = (prbs_23[i*8+0]<<7) + (prbs_23[i*8+1]<<6) + (prbs_23[i*8+2]<<5) + (prbs_23[i*8+3]<<4) + 
			(prbs_23[i*8+4]<<3) + (prbs_23[i*8+5]<<2) + (prbs_23[i*8+6]<<1) + prbs_23[i*8+7];
	}

	//fwrite(prbs_15, 1, nPlayloadSize*8, fp);
	//fwrite(prbs_15, 1, nPlayloadSize, fp);
	//fclose(fp);
}

int	CHldFmtrBert::Set_ErrInjection_Param(long error_lost, long error_lost_packet,
	long error_bits, long error_bits_packet, long error_bits_count,
	long error_bytes, long error_bytes_packet, long error_bytes_count)
{
	return	__FIf__->__Set_ErrInjection_Param(error_lost, error_lost_packet,
		error_bits, error_bits_packet, error_bits_count,
		error_bytes, error_bytes_packet, error_bytes_count);
}
int CHldFmtrBert::SetMidulator_Bert_Measure(long modulator_type, long packet_type, long bert_pid)
{
	int		*_tst_pkt_typ;
	int		*_tst_pkt_pid;
	_tst_pkt_typ = __FIf__->_Ptr_VarTstPktTyp();
	_tst_pkt_pid = __FIf__->_Ptr_VarTstPktPid();

	//MPEG-TS
	if ( packet_type >= TS_HEAD_184_ALL_0 )
	{
		//TDMB
		if ( modulator_type == 5 )
		{
			*_tst_pkt_typ = NO_BERT_OPERTION;
		}
		else
		{
			*_tst_pkt_typ = packet_type;
		}
		*_tst_pkt_pid = bert_pid;
		return 0;
	}
	//SERIEAL
	else
	{
		*_tst_pkt_typ = packet_type;
		if ( modulator_type == 0	//DVB-T
			|| modulator_type == 1	//8VSB
			|| modulator_type == 2	//QAM-A/C
			|| modulator_type == 3	//QAM-B
			|| modulator_type == 4	//QPSK
			|| modulator_type == 7	//DVB-H
			|| modulator_type == 15	//ATSC-MH //TVB593
			|| modulator_type == 19	//Multi_QAMB //TVB593
			|| modulator_type == 20	//Multi_VSB //TVB593
			|| modulator_type == 21	//Multi_DVBT //TVB593
			)
		{
			return __FIf__->TVB380_SET_MODULATOR_BERT_MEASURE(modulator_type, packet_type);
		}
		else
		{
			return -1;
		}
	}
}
double	CHldFmtrBert::GetBer(void)
{
	return	TL_BER;
}

//----------------------------------------------------------------------------------
int	CHldFmtrBert::RdPrbsDta(int _pkt_typ)
{
	int		i = 0, j = 0;
	char	szDataPath[MAX_PATH];
	unsigned char	**_target_buf;
	FILE	**_src_f;
	char	*_src_fn;
	int		_rd_siz;

	switch(_pkt_typ)
	{
	case	TS_HEAD_184_PRBS_2_15:
	case	TS_SYNC_187_PRBS_2_15:
	case	TS_STUFFING_184_PRBS_2_15:
		_src_f = &__FIf__->TL_fp_14_15;
		_src_fn = BERT_PRBS_14_15_TYPE;
		_target_buf = &TL_pbPRBSBuffer_14_15;
		_rd_siz = BERT_PRBS_14_15_SIZE;
		break;

	case	TS_HEAD_184_PRBS_2_23:
	case	TS_SYNC_187_PRBS_2_23:
	case	TS_STUFFING_184_PRBS_2_23:
		_src_f = &__FIf__->TL_fp_18_23;
		_src_fn = BERT_PRBS_18_23_TYPE;
		_target_buf = &TL_pbPRBSBuffer_18_23;
		_rd_siz = BERT_PRBS_18_23_SIZE;
		break;

	default:
		return	0;
	}

	if ( !*_src_f )
	{
		__FIf__->Create_AFileName_at_CurDir(szDataPath, _src_fn);
		*_src_f = __FIf__->_FOpen(szDataPath, "rb");
		if ( *_src_f )
		{
			__FIf__->_FSeek(*_src_f, 0L, SEEK_END);
			j = __FIf__->_FTell(*_src_f);
			__FIf__->_FSeek(*_src_f, 0L, SEEK_SET);

			*_target_buf = (unsigned char*)malloc(_rd_siz);
			i = __FIf__->_FRead(*_src_f, (char *)*_target_buf, j);
		}
	}

	return	i;
}
//----------------------------------------------------------------------------------
//	Record and Analyze captured stream
//	Use 1 Mbyte - for recording or monitoring
//
int	CHldFmtrBert::TL_ProcessCaptureSubBlock(void)
{
	unsigned long	dwSizeWritten = 0;
	int		nSyncStartPos;
	DWORD	RestByte;
	int		*_tst_pkt_typ;

	_tst_pkt_typ = __FIf__->_Ptr_VarTstPktTyp();

//////////////////////////////////////////////////////////////	Add TS analyzer here
	nSyncStartPos = 0;
	if(__FIf__->TSSyncLock == FALSE) 
	{
		if ((nSyncStartPos = __FIf__->TL_SearchSync(__FIf__->SdramSubBankSize())) != -1)
		{
			__FIf__->TSSyncLock = TRUE;
			__HLog__->HldPrint_2("Hld-Bd-Ctl. : cap-dta-sync-locked. ", nSyncStartPos, (__FIf__->SdramSubBankSize()) - nSyncStartPos);
		}
	}

	RestByte = (__FIf__->SdramSubBankSize()) - nSyncStartPos;

//////////////////////////////////////////////////////////////	Skip recording for Analyze/Monitor only mode
	if(__FIf__->TSSyncLock) 
	{
		dwSizeWritten = RestByte;

#ifdef TSPHLD0110_EXPORTS
//
#else
//////////////////////////////////////////////////////////////////////	BERT
#if 1
		//char tmp[32];
		//FILE *fp=fopen("C:\\error.txt", "at");
		if ( *_tst_pkt_typ >= TS_HEAD_184_ALL_0 )
		{
			int nPlayloadSize, i, nTestSize=0;
			unsigned char *buf = ((unsigned char *)__FIf__->__Buf_DmaPtr_Transfer_Bidirectional()) + nSyncStartPos;	//	cap-dta are here.

//			SKIP THE FIRST 16MB
			if ( __FIf__->BytesCapFile() < (__FIf__->SdramSubBankSize())*MAX_BANK_NUMBER*2 )
			{
				dwSizeWritten = RestByte;
				__FIf__->TL_i64TotalFileSize += dwSizeWritten;
			}
			else
			{
				int j = 0;
				i = RdPrbsDta(*_tst_pkt_typ);

				//PAYLOAD SIZE
				if ( *_tst_pkt_typ == TS_SYNC_187_ALL_0
					|| *_tst_pkt_typ == TS_SYNC_187_ALL_1
					|| *_tst_pkt_typ == TS_SYNC_187_PRBS_2_15
					|| *_tst_pkt_typ == TS_SYNC_187_PRBS_2_23 )
				{
					nPlayloadSize = 187;
				}
				else
				{
					nPlayloadSize = 184;
				}

				__FIf__->FillPlayBuf_from_GivenBuf_w_ChkBoundary(buf, dwSizeWritten);	//	cpy to play-buf the dta of dma-buf.

//////////////////////////////////////////////////////////////////////	Read Buffer and check bit error
				while ( __FIf__->TL_nBCount > (unsigned int)(__FIf__->TL_gPacketSize*10) )
				{
					__FIf__->FillRsltBuf_from_PlayBuf_w_ChkBoundary(__FIf__->TL_gPacketSize);
					if ( __FIf__->TL_pbBuffer[0] != 0x47 )
					{
						__FIf__->SetNewRdAlignPos_of_PlayBuf(1);
						continue;
					}
					else
					{
						__FIf__->FillTemp2Buf_from_PlayBufSkipPktSize_w_ChkBoundary();
						if ( __FIf__->Temp2Buf_IsNot_Sync_Aligned() )
						{
							__FIf__->SetNewRdAlignPos_of_PlayBuf(1);
							continue;
						}
					}
					__FIf__->SetNewRdAlignPos_of_PlayBuf(__FIf__->TL_gPacketSize);

					if ( __FIf__->TL_pbBuffer[0] != 0x47 )
					{
					//	MessageBox(NULL, "TEST", "TEST", MB_OK);
					}

					//sskim20080725 - BERT FIXED
					if ( *_tst_pkt_typ >= TS_STUFFING_184_ALL_0 )	//	TL_BERT_SET_DATA()
					{
						if ( __FIf__->TL_pbBuffer[0] != 0x47 || __FIf__->TL_pbBuffer[1] != 0x1F || __FIf__->TL_pbBuffer[2] != 0xFF )
						{
							TL_TotalCount += __FIf__->TL_gPacketSize;
							continue;
						}
					}
					
					buf = __FIf__->TL_pbBuffer + 4;	//	skip sync/pid/adap-field
					nPlayloadSize = 184;
					if ( *_tst_pkt_typ >= TS_SYNC_187_ALL_0 && *_tst_pkt_typ <= TS_SYNC_187_PRBS_2_23 )	//	in case of sync error
					{
						buf = __FIf__->TL_pbBuffer + 1;	//	skip sync
						nPlayloadSize = 187;
					}

					switch (*_tst_pkt_typ)
					{
					case TS_HEAD_184_ALL_0:
					case TS_SYNC_187_ALL_0:
					case TS_STUFFING_184_ALL_0:
						for ( i = 0; i < nPlayloadSize; i++ )
						{
							if ( buf[i] != 0 )
							{
								if ( (buf[i] & 0x01) != 0 )			++TL_ErrorCount;
								if ( ((buf[i]>>1) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>2) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>3) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>4) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>5) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>6) & 0x01) != 0 )	++TL_ErrorCount;
								if ( ((buf[i]>>7) & 0x01) != 0 )	++TL_ErrorCount;
							}
						}
						break;

					case TS_HEAD_184_ALL_1:
					case TS_SYNC_187_ALL_1:
					case TS_STUFFING_184_ALL_1:
						for ( i = 0; i < nPlayloadSize; i++ )
						{
							if ( buf[i] != 1 )
							{
								if ( (buf[i] & 0x01) != 1 )			++TL_ErrorCount;
								if ( ((buf[i]>>1) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>2) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>3) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>4) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>5) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>6) & 0x01) != 1 )	++TL_ErrorCount;
								if ( ((buf[i]>>7) & 0x01) != 1 )	++TL_ErrorCount;
							}
						}
						break;

					case TS_HEAD_184_PRBS_2_15:
					case TS_SYNC_187_PRBS_2_15:
					case TS_STUFFING_184_PRBS_2_15:
					case TS_HEAD_184_PRBS_2_23:
					case TS_STUFFING_184_PRBS_2_23:
					case TS_SYNC_187_PRBS_2_23:
					{
						if ( *_tst_pkt_typ == TS_HEAD_184_PRBS_2_15
							|| *_tst_pkt_typ == TS_SYNC_187_PRBS_2_15
							|| *_tst_pkt_typ == TS_STUFFING_184_PRBS_2_15 )
						{
							nTestSize = BERT_PRBS_14_15_SIZE;
							TL_pbPRBSBuffer = TL_pbPRBSBuffer_14_15;
						}
						else
						{
							nTestSize = BERT_PRBS_18_23_SIZE;
							TL_pbPRBSBuffer = TL_pbPRBSBuffer_18_23;
						}

						if ( __FIf__->TL_nFindSync == -1 )
						{
							if ( TL_pbPRBSBuffer == NULL )
							{
								break;
							}

							for ( j = 0; j < nTestSize; j++ )
							{
								if ( j == nTestSize - 2 )
								{
									if ( TL_pbPRBSBuffer[j] == buf[0] 
										&& TL_pbPRBSBuffer[j+1] == buf[1]
										&& TL_pbPRBSBuffer[0] == buf[2] )
									{
										__FIf__->TL_nFindSync = j;
										break;
									}
								}
								else if ( j == nTestSize - 1 )
								{
									if ( TL_pbPRBSBuffer[j] == buf[0] 
										&& TL_pbPRBSBuffer[0] == buf[1]
										&& TL_pbPRBSBuffer[1] == buf[2] )
									{
										__FIf__->TL_nFindSync = j;
										break;
									}
								}
								else if ( TL_pbPRBSBuffer[j] == buf[0] 
									&& TL_pbPRBSBuffer[j+1] == buf[1]
									&& TL_pbPRBSBuffer[j+2] == buf[2] )
								{
									__FIf__->TL_nFindSync = j;
									break;
								}
							}
						}

						if ( __FIf__->TL_nFindSync != -1 )
						{
							for ( i = 0; i < nPlayloadSize; i++ )	//	to compare
							{
								if ( TL_pbPRBSBuffer[__FIf__->TL_nFindSync] != buf[i] )
								{
									if ( (buf[i] & 0x01) != (TL_pbPRBSBuffer[__FIf__->TL_nFindSync] & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>1) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>1) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>2) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>2) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>3) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>3) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>4) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>4) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>5) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>5) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>6) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>6) & 0x01) )	++TL_ErrorCount;
									if ( ((buf[i]>>7) & 0x01) != ((TL_pbPRBSBuffer[__FIf__->TL_nFindSync]>>7) & 0x01) )	++TL_ErrorCount;
								}
								__FIf__->TL_nFindSync = (__FIf__->TL_nFindSync >= nTestSize-1) ? 0 : __FIf__->TL_nFindSync+1;
							}
						}
					}
					break;

					default:
						break;
					}

					TL_TotalCount += __FIf__->TL_gPacketSize;
					if ( TL_TotalCount >= BERT_DATA_SIZE )
					{
						__FIf__->TL_i64TotalFileSize += TL_TotalCount;
						TL_BER = (double)TL_ErrorCount / (double)(BERT_DATA_SIZE*8);

						if ( __FIf__->TL_nFindSync >= 0 )
						{
							//sprintf(tmp, "TL_pbPRBSBuffer[%d]=%02X,error=%d\n", __FIf__->TL_nFindSync, TL_pbPRBSBuffer[__FIf__->TL_nFindSync], TL_ErrorCount);
							//OutputDebugString(tmp);
						}
						if ( TL_ErrorCount > 0 )
						{
							__FIf__->TL_nFindSync = -1;
						}

						TL_TotalCount = 0;
						TL_ErrorCount = 0;
						//break;
					}
				}
				//fclose(fp);
			}
		}
#else
#endif

//////////////////////////////////////////////////////////////////////
		if (!__FIf__->IsInvalidFileHandle())
		{	
			if ( __FIf__->TL_i64TotalFileSize < (__FIf__->SdramSubBankSize())*MAX_BANK_NUMBER*2 )	//	skip for BERT chk.
			{
				dwSizeWritten = RestByte;
				__FIf__->TL_i64TotalFileSize += dwSizeWritten;

				if ( __FIf__->TL_i64TotalFileSize >= (__FIf__->SdramSubBankSize())*MAX_BANK_NUMBER*2 )
				{
					__FIf__->TSSyncLock = FALSE;
				}
			}
			else
			{
				__FIf__->_FWrite_(__FIf__->AP_hFile,
										((unsigned char *)__FIf__->DmaPtr_HostSide()) + nSyncStartPos,
										RestByte,
										&dwSizeWritten);

				__FIf__->TL_i64TotalFileSize += dwSizeWritten;
				__FIf__->TL_gTotalSendData += dwSizeWritten;	//IP streaming
			}
		}
#endif
	}

	return (dwSizeWritten != (unsigned int)(__FIf__->SdramSubBankSize())?-1:0);
}
//2012/3/16 BERT 187
#if 1 
int CHldFmtrBert::Play_BERT187_EmptyFile_UntilStopContPlay__HasEndlessWhile(void)
{
	int		nBankBlockSize = (__FIf__->SdramSubBankSize());
    int		_buf_rd_pos = 0;
	int     _buf_wr_pos = 0;
    int		_BertDataSize = 0;
	int		_tmp_val;
	static unsigned char buf[204];
	int nBertType = __FIf__->_VarTstPktTyp();
	int nBertPid = __FIf__->_VarTstPktPid();
	while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
	{
		_BertDataSize = 0;
		while(_BertDataSize < nBankBlockSize)
		{
			memset(buf, 0, 204);
			if(nBertType >= TS_HEAD_184_ALL_0 && nBertType <= TS_HEAD_184_PRBS_2_23)
			{
				buf[0] = 0x47;
				buf[1] = ((nBertPid >> 8) & 0x1F);
				buf[2] = (nBertPid & 0xFF);
				buf[3] = 0x10 + __FIf__->TL_Cont_cnt;
				__FIf__->TL_Cont_cnt++;
				if(__FIf__->TL_Cont_cnt > 15)
					__FIf__->TL_Cont_cnt = 0;
			}

			__FIf__->TL_BERT_SET_DATA(&buf[0]);
			if(_buf_wr_pos + 188 <= MAX_BUFFER_BYTE_SIZE)
			{
				memcpy(__FIf__->TL_szBufferPlay + _buf_wr_pos, &buf[0], 188);
				_buf_wr_pos = _buf_wr_pos + 188;
			}
			else
			{
				_tmp_val = MAX_BUFFER_BYTE_SIZE - _buf_wr_pos;
				memcpy(__FIf__->TL_szBufferPlay + _buf_wr_pos, &buf[0], _tmp_val);
				memcpy(__FIf__->TL_szBufferPlay, &buf[_tmp_val], 188 - _tmp_val);
				_buf_wr_pos = _buf_wr_pos + 188 - MAX_BUFFER_BYTE_SIZE;
			}
			if(fp1 != NULL)
			{
				fwrite(buf, 1, 188, fp1);
			}
			_BertDataSize = _BertDataSize + 188;

		}
		if (__FIf__->TL_nSubBankIdx == 0)
		{
			__FIf__->TL_dwAddrDestBoardSDRAM = (__FIf__->TL_nIdCurBank == 0? 0 : BANK_SIZE_4);
		}
			// DMA START 
#if defined(WIN32)				
		if(_buf_rd_pos + nBankBlockSize <= MAX_BUFFER_BYTE_SIZE)
		{
			memcpy((void *) __FIf__->TL_pdwDMABuffer, __FIf__->TL_szBufferPlay + _buf_rd_pos, nBankBlockSize);
			_buf_rd_pos = _buf_rd_pos + nBankBlockSize;
		}
		else
		{
			_tmp_val = MAX_BUFFER_BYTE_SIZE - _buf_rd_pos;
			memcpy(__FIf__->TL_pdwDMABuffer, __FIf__->TL_szBufferPlay + _buf_rd_pos, _tmp_val);
			memcpy(__FIf__->TL_pdwDMABuffer + _tmp_val, __FIf__->TL_szBufferPlay, nBankBlockSize - _tmp_val);
			_buf_rd_pos = _buf_rd_pos + nBankBlockSize - MAX_BUFFER_BYTE_SIZE;
		}

		if(fp2 != NULL)
		{
			fwrite(__FIf__->TL_pdwDMABuffer, 1, nBankBlockSize, fp2);
		}

		__FIf__->TSPL_WRITE_BLOCK( NULL, (unsigned long )nBankBlockSize, (unsigned long *)__FIf__->TL_dwAddrDestBoardSDRAM);
#else

		if(_buf_rd_pos + nBankBlockSize <= MAX_BUFFER_BYTE_SIZE)
		{
			memcpy((void *) __FIf__->TL_pbBuffer, __FIf__->TL_szBufferPlay + _buf_rd_pos, nBankBlockSize);
			_buf_rd_pos = _buf_rd_pos + nBankBlockSize;
		}
		else
		{
			_tmp_val = MAX_BUFFER_BYTE_SIZE - _buf_rd_pos;
			memcpy(__FIf__->TL_pbBuffer, __FIf__->TL_szBufferPlay + _buf_rd_pos, _tmp_val);
			memcpy(__FIf__->TL_pbBuffer + _tmp_val, __FIf__->TL_szBufferPlay, nBankBlockSize - _tmp_val);
			_buf_rd_pos = _buf_rd_pos + nBankBlockSize - MAX_BUFFER_BYTE_SIZE;
		}

		__FIf__->TSPL_WRITE_BLOCK( (DWORD *)__FIf__->TL_pbBuffer, (unsigned long )nBankBlockSize, (DWORD *)__FIf__->TL_dwAddrDestBoardSDRAM);
#endif
		while ( __Sta__->IsTaskState_ContPlay() && !__Sta__->ReqedNewAction_User() )
		{
			if ( __FIf__->TSPL_GET_DMA_STATUS() )
				break;
			//Sleep(10);
			Sleep(0);
		}
		__FIf__->TL_dwAddrDestBoardSDRAM += (SUB_BANK_MAX_BYTE_SIZE >> 2);
		__FIf__->TL_CheckBufferStatus(1, __FIf__->GetPlayParam_PlayRate());
		if (++__FIf__->TL_nSubBankIdx == (__Sta__->iHW_BANK_NUMBER+1))
		{
			__FIf__->TL_nIdCurBank = (__FIf__->TL_nIdCurBank + 1) & 0x01;
			__FIf__->TL_nSubBankIdx = 0;
		}
	}
	if(fp1 != NULL)
		fclose(fp1);
	if(fp2 != NULL)
		fclose(fp2);
	return 1;
}
#endif

