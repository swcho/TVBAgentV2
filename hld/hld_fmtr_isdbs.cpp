
#if defined(WIN32)
#include	<Windows.h>
#include	<process.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<pthread.h>
#endif
#include	<stdio.h>

#include	"../include/hld_structure.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"hld_fmtr_isdbs.h"
#include	"LLDWrapper.h"

extern	unsigned char	gNullPack[204];

//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrIsdbS::CHldFmtrIsdbS(int _my_id, void *_hld)
{
	int	i;

	my_hld_id = _my_id;
	my_hld = _hld;

//	TL_sz3rdBufferPlay = (unsigned char *)malloc(SUB_BANK_MAX_BYTE_SIZE*8);
	TL_sz3rdBufferPlay = NULL;

	TL_combiner_ready = 0;
	TL_combiner_TS_count = 0;
	TL_nFrameNum = 0;
	TL_nSlotNum = 0;
	for ( i = 0; i < MAX_TS_COUNT; i++ )
	{
		m_nConst[i] = -1;
		m_nCode[i] = -1;
		m_nSlot[i] = 0;
		memset(m_TS_path[i], 0x00, 1024);
	}
	TL_ISDBS_PumpingThreadDone = 1;

	tmcc_change_flag_old_ = -1;
}

CHldFmtrIsdbS::~CHldFmtrIsdbS()
{
	if ( TL_sz3rdBufferPlay ) free(TL_sz3rdBufferPlay);
}
void	CHldFmtrIsdbS::AllocateCapPlayBufIsdbS(void)
{
	if (TL_sz3rdBufferPlay == NULL)
	{
		TL_sz3rdBufferPlay = (unsigned char *)malloc(SUB_BANK_MAX_BYTE_SIZE*8);
	}
}
void	CHldFmtrIsdbS::FreeCapPlayBufIsdbS(void)
{
	if ( TL_sz3rdBufferPlay )
	{
		free(TL_sz3rdBufferPlay);
	}
	TL_sz3rdBufferPlay = NULL;
}

void	CHldFmtrIsdbS::SetCommonMethod_4(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__,
		CHldFmtrCmmb	*__framalyzer__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
	__Cmmb__	=	__framalyzer__;

	ts_buf_to_process_dta = __FIf__->__Buf_Temp();	//	is not destroyed until the program exit.
}
void CHldFmtrIsdbS::InitIsdbSPlayBufVariables(void)
{
	TL_nWritePos3 = 0;	TL_nReadPos3 = 0;	TL_nBufferCnt3 = 0;
}

void	CHldFmtrIsdbS::CreateIsdbSMutex(void)
{
#if defined(WIN32)
	TL_ISDBS_hMutex = CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutexattr_init(&TL_ISDBS_hMutexAttr);
	pthread_mutexattr_settype(&TL_ISDBS_hMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&TL_ISDBS_hMutex, &TL_ISDBS_hMutexAttr);
#endif

}
void	CHldFmtrIsdbS::DestroyIsdbSMutex(void)
{
#if defined(WIN32)	
	if ( TL_ISDBS_hMutex != INVALID_HANDLE_VALUE )
	{
		CloseHandle(TL_ISDBS_hMutex);
		TL_ISDBS_hMutex = INVALID_HANDLE_VALUE; 
	}
#else
	pthread_mutexattr_destroy(&TL_ISDBS_hMutexAttr);
	pthread_mutex_destroy(&TL_ISDBS_hMutex);
#endif
}
void	CHldFmtrIsdbS::LockIsdbSMutex(void)
{
#if defined(WIN32)
	if ( TL_ISDBS_hMutex != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(TL_ISDBS_hMutex, INFINITE);
	}
#else
	pthread_mutex_lock(&TL_ISDBS_hMutex);
#endif
}
void	CHldFmtrIsdbS::UnlockIsdbSMutex(void)
{
#if defined(WIN32)
	if ( TL_ISDBS_hMutex != INVALID_HANDLE_VALUE )
	{
		ReleaseMutex(TL_ISDBS_hMutex);
	}
#else
	pthread_mutex_unlock(&TL_ISDBS_hMutex);
#endif
}

void CHldFmtrIsdbS::InitIsdbSVariables_OnPlayStart(void)
{
	TL_nFrameNum = 0;
	TL_nSlotNum = 0;
	TL_combiner_ready = 0;
	//fp_dump = fopen("C:\\combined_live.ts", "wb");
	fp_dump = NULL;
	fp_dump2 = NULL;

	tot_cnt_process_dta_frm_layer = 0;
}

unsigned long	CHldFmtrIsdbS::_clk_msec_(void)
{
#if defined(WIN32)
	return	GetTickCount();
#endif
	return	0;
}
void CHldFmtrIsdbS::EstimateRuntimeBr(int init, unsigned long cnt_fram_layer)	//	dbg purpose
{
	return;

	unsigned long	cur_tm;
	if (init)
	{
		if (tot_cnt_process_dta_frm_layer <= 0)
		{
			prev_clk_to_calc_runtime_br = _clk_msec_();
			prev_cnt_process_dta_frm_layer = 0;
			elap_tm_to_calc_runtime_br = 0;
			return;
		}
	}
	if (cnt_fram_layer <= 0)
	{
		prev_cnt_process_dta_frm_layer = 0;
		return;
	}
	else
	{
		tot_cnt_process_dta_frm_layer += cnt_fram_layer;	//	(cnt_fram_layer - prev_cnt_process_dta_frm_layer);
//		prev_cnt_process_dta_frm_layer = cnt_fram_layer;
	}
	cur_tm = _clk_msec_();
	elap_tm_to_calc_runtime_br += (cur_tm - prev_clk_to_calc_runtime_br);
	prev_clk_to_calc_runtime_br = cur_tm;

	if (elap_tm_to_calc_runtime_br != 0)
	{
		_runtime_br = (tot_cnt_process_dta_frm_layer*8)/elap_tm_to_calc_runtime_br;
		_runtime_br *= 1000;
		__HLog__->HldPrint_CntWait_2("isdb-s : br ", 22, (int)_runtime_br);
	}
}
FILE *CHldFmtrIsdbS::OpenSource(char *_filen)
{
	if (__Sta__->IPUsing() || __Sta__->IsPlayMod_IsdbS())
	{
		return	NULL;	//	buf interface.
	}
	if (*_filen != 0)	//	file-path is valid.
	{
		return	fopen(_filen, "rb");
	}
	return	NULL;
}
int CHldFmtrIsdbS::RdOnePktIp(unsigned char *buf, int raw_packet_size)
{
    int	i, len, ret_err;
	CHld	*_hld_ = (CHld *)my_hld;

	ret_err = 0;
    for(;;)
	{
		len = _hld_->Get_RcvIpDta(buf, raw_packet_size);
        if (len != raw_packet_size)
       	{
            ret_err = -1;
			goto	__rd_done;
       	}
        if (buf[0] != 0x47)	//	find new sync position. if sync loss.
		{
			for(i = 0;i < MAX_RESYNC_SIZE; i++)
			{
				len = _hld_->Get_RcvIpDta(buf, 1);
				if (len <= 0)
				{
					ret_err = -1;
					goto	__rd_done;
				}
				if (buf[0] == 0x47)
				{
					len = _hld_->Get_RcvIpDta(&buf[1], (raw_packet_size - 1));
					if (len != (raw_packet_size - 1))
					{
						ret_err = -1;
					}
					goto	__rd_done;
				}
			}
            ret_err = -1;
			goto	__rd_done;
        }
		else
		{
            break;
        }
    }

__rd_done:
    return	ret_err;
}
int CHldFmtrIsdbS::RdOnePktAsi(unsigned char *buf, int raw_packet_size)
{
	return	-1;
}
int CHldFmtrIsdbS::RdOnePkt(FILE *pb, unsigned char *buf, int raw_packet_size, int act_ts_id, char *base_tsf)
{
    int	c, i;
	int	len, ret_err, recurcive_retry;

	unsigned char Temp_Buf[208];

	if ( __Sta__->IPUsing())
	{
		return	RdOnePktIp(buf, PKTSIZE);
	}
	if (__Sta__->IsPlayMod_IsdbS())
	{
		return	RdOnePktAsi(buf, raw_packet_size);	//	not support yet
	}

	ret_err = 0;
	recurcive_retry = 0;
    for(;;)
	{
_try_again:
		//2012/9/26
		//len = fread (buf,  1, raw_packet_size, pb);
		len = fread (Temp_Buf,  1, raw_packet_size, pb);
        if (len != raw_packet_size)
       	{
            ret_err = -1;
			goto	__rd_done;
       	}
        if (Temp_Buf[0] != 0x47)	//	find new sync position. if sync loss.
		{
            fseek(pb, -raw_packet_size, SEEK_CUR);
			for(i = 0;i < MAX_RESYNC_SIZE; i++)
			{
				c = fgetc(pb);
				if (c < 0)
				{
					ret_err = -1;
					goto	__rd_done;
				}
				if (c == 0x47)
				{
					fseek(pb, -1, SEEK_CUR);
					goto	_try_again;
				}
			}
            ret_err = -1;
			goto	__rd_done;
        }
		else
		{
			//2012/9/26
			memcpy(buf, Temp_Buf, PKTSIZE);
			ret_err = 0;
            goto	__return;
        }
    }

__rd_done:
	if (ret_err < 0)
	{
		if (recurcive_retry == 1)	//	to retry only one-time.
		{
			goto	__return;
		}
	}
	if ((ret_err < 0) || (feof(pb) != 0))	//	err or eof.
	{
		if (base_tsf != NULL)	//	2011/4/4 ISDB-S Bitrate. what is it???
		{
			if(strcmp(TS_path[act_ts_id], base_tsf) == 0)	//	what is it???
			{
				ret_err = -1;
				goto	__return;
			}
		}
	
		if (feof(pb) != 0)	//	eof
		{
			fseek(pb, 0L, SEEK_SET);	//	wrap around the rd-position. set to 0
			recurcive_retry = 1;
			goto	_try_again;
		}
		else
		{
			//	happened unkown error
		}
	}

__return:
    return	ret_err;
}
int CHldFmtrIsdbS::FillOnePkt_intoProcessBuf(int single_ts, int act_ts_slot)
{
	int	k, nReadBytes;
	//2012/3/27 BERT 187
	int _nBert_type = __FIf__->_VarTstPktTyp();
	int _nBert_pid = __FIf__->_VarTstPktPid();
	unsigned char *buf;

	for ( k = 0; k < nSlot_Valid[act_ts_slot]; k++ )
	{
		if ( AddedNullPacket[act_ts_slot] > 0 )
		{
			nWritePos = nReadPos = nBufferCnt = 0;
			__FIf__->FillOnePacketToQueue(PKTSIZE,
					ts_buf_to_process_dta+off__ts_buf_to_process_dta, &nWritePos, &nReadPos, &nBufferCnt, SUB_BANK_MAX_BYTE_SIZE, gNullPack);
			--AddedNullPacket[act_ts_slot];
		}
		else
		{
			//2012/3/27 BERT 187
			//if((_nBert_type == TS_SYNC_187_ALL_0 || _nBert_type == TS_SYNC_187_ALL_1 ||
			//	_nBert_type == TS_SYNC_187_PRBS_2_15 || _nBert_type == TS_SYNC_187_PRBS_2_23))
			//{
			//	memcpy(ts_buf_to_process_dta + off__ts_buf_to_process_dta, gNullPack, PKTSIZE);
			//	nReadBytes = PKTSIZE;
			//}
			if( _nBert_type >= TS_HEAD_184_ALL_0 && _nBert_type <= TS_SYNC_187_PRBS_2_23 )
			{
				buf = ts_buf_to_process_dta + off__ts_buf_to_process_dta;
				buf[0] = 0x47;
				buf[1] = ((_nBert_pid >> 8) & 0x1F);
				buf[2] = (_nBert_pid & 0xFF);
				buf[3] = 0x10 + __FIf__->TL_Cont_cnt;
				for(int i = 4; i < PKTSIZE; i++)
				{
					buf[i] = 0x00;
				}
				__FIf__->TL_Cont_cnt++;
				if(__FIf__->TL_Cont_cnt > 15)
					__FIf__->TL_Cont_cnt = 0;
				TotalReadPacket[act_ts_slot]++;
				AddedNullPacket[act_ts_slot] = 0;
			}
			else
			{
				nReadBytes = RdOnePkt(fp_mts[act_ts_slot], ts_buf_to_process_dta+off__ts_buf_to_process_dta, nPacket_Size__[act_ts_slot], act_ts_slot, gszISDBS_baseTS);
				if ( nReadBytes < 0 )
				{
					return	-1;
				}

				TotalReadPacket[act_ts_slot]++;
				AddedNullPacket[act_ts_slot] = CalcNullStuffCount(TotalReadPacket[act_ts_slot],  StuffRatio[act_ts_slot]);
			}
		}

		//2011/8/2 BERT
		__FIf__->TL_BERT_SET_DATA((ts_buf_to_process_dta+off__ts_buf_to_process_dta));
		tmcc_TS_ID_per_slot[nSlotNum] = act_ts_slot;
		off__ts_buf_to_process_dta += PKTSIZE;
		add_frame_info(ts_buf_to_process_dta, &off__ts_buf_to_process_dta, &nSlotNum, &nFrameNum, 0, nConst[act_ts_slot], nCode[act_ts_slot]);

		EstimateRuntimeBr(0, PKTSIZE);
	}
	for ( k = 0; k < nSlot_Dummy[act_ts_slot]; k++ )
	{
		memcpy(ts_buf_to_process_dta+off__ts_buf_to_process_dta, gNullPack, PKTSIZE);
		tmcc_TS_ID_per_slot[nSlotNum] = act_ts_slot;
		off__ts_buf_to_process_dta += PKTSIZE;
		add_frame_info(ts_buf_to_process_dta, &off__ts_buf_to_process_dta, &nSlotNum, &nFrameNum, 1, nConst[act_ts_slot], nCode[act_ts_slot]);

		EstimateRuntimeBr(0, PKTSIZE);
	}

	return	0;
}
int CHldFmtrIsdbS::FillTailSuperframe(void)
{
	int	i;
	unsigned char tmcc[48+16];
	int tmcc_emergency_alarm;
	int tmcc_up_link_site_diversity;
	int tmcc_up_link_added;
	int tmcc_ext_flag;

	tmcc_change_indicator = 0; //0~30 for each superfame ?
	tmcc_emergency_alarm = __FIf__->Emergency_Broadcasting;
	tmcc_up_link_site_diversity = 0;
	tmcc_up_link_added = 0;
	tmcc_ext_flag = 0;

	tmcc[0] = ((tmcc_change_indicator&0x1F)<<3) + ((tmcc_tx_mode[0]>>1)&0x07);////////
	tmcc[1] = ((tmcc_tx_mode[0]&0x01)<<7) + ((tmcc_tx_slot[0]&0x3F)<<1) + ((tmcc_tx_mode[1]>>3)&0x01);
	tmcc[2] = ((tmcc_tx_mode[1]&0x07)<<5) + ((tmcc_tx_slot[1]>>1)&0x1F);
	tmcc[3] = ((tmcc_tx_slot[1]&0x01)<<7) + ((tmcc_tx_mode[2]&0x0F)<<3) + ((tmcc_tx_slot[2]>>3)&0x07);
	tmcc[4] = ((tmcc_tx_slot[2]&0x07)<<5) + ((tmcc_tx_mode[3]&0xF)<<1) + ((tmcc_tx_slot[3]>>5)&0x01);
	tmcc[5] = ((tmcc_tx_slot[3]&0x1F)<<3) +
			(tmcc_TS_ID_per_slot[0]&0x07);////////5+40+3
	tmcc[6] = ((tmcc_TS_ID_per_slot[1]&0x07)<<5) + ((tmcc_TS_ID_per_slot[2]&0x07)<<2) + ((tmcc_TS_ID_per_slot[3]>>1)&0x03);
	tmcc[7] = ((tmcc_TS_ID_per_slot[3]&0x01)<<7) + ((tmcc_TS_ID_per_slot[4]&0x07)<<4) + ((tmcc_TS_ID_per_slot[5]&0x07)<<1) + ((tmcc_TS_ID_per_slot[6]>>2)&0x01);
	tmcc[8] = ((tmcc_TS_ID_per_slot[6]&0x03)<<6) + ((tmcc_TS_ID_per_slot[7]&0x07)<<3) + (tmcc_TS_ID_per_slot[8]&0x07);
	tmcc[9] = ((tmcc_TS_ID_per_slot[9]&0x07)<<5) + ((tmcc_TS_ID_per_slot[10]&0x07)<<2) + ((tmcc_TS_ID_per_slot[11]>>1)&0x03);
	tmcc[10] = ((tmcc_TS_ID_per_slot[11]&0x01)<<7) + ((tmcc_TS_ID_per_slot[12]&0x07)<<4) + ((tmcc_TS_ID_per_slot[13]&0x07)<<1) + ((tmcc_TS_ID_per_slot[14]>>2)&0x01);
	tmcc[11] = ((tmcc_TS_ID_per_slot[14]&0x03)<<6) + ((tmcc_TS_ID_per_slot[15]&0x07)<<3) + (tmcc_TS_ID_per_slot[16]&0x07);
	tmcc[12] = ((tmcc_TS_ID_per_slot[17]&0x07)<<5) + ((tmcc_TS_ID_per_slot[18]&0x07)<<2) + ((tmcc_TS_ID_per_slot[19]>>1)&0x03);
	tmcc[13] = ((tmcc_TS_ID_per_slot[19]&0x01)<<7) + ((tmcc_TS_ID_per_slot[20]&0x07)<<4) + ((tmcc_TS_ID_per_slot[21]&0x07)<<1) + ((tmcc_TS_ID_per_slot[22]>>2)&0x01);
	tmcc[14] = ((tmcc_TS_ID_per_slot[22]&0x03)<<6) + ((tmcc_TS_ID_per_slot[23]&0x07)<<3) + (tmcc_TS_ID_per_slot[24]&0x07);
	tmcc[15] = ((tmcc_TS_ID_per_slot[25]&0x07)<<5) + ((tmcc_TS_ID_per_slot[26]&0x07)<<2) + ((tmcc_TS_ID_per_slot[27]>>1)&0x03);
	tmcc[16] = ((tmcc_TS_ID_per_slot[27]&0x01)<<7) + ((tmcc_TS_ID_per_slot[28]&0x07)<<4) + ((tmcc_TS_ID_per_slot[29]&0x07)<<1) + ((tmcc_TS_ID_per_slot[30]>>2)&0x01);
	tmcc[17] = ((tmcc_TS_ID_per_slot[30]&0x03)<<6) + ((tmcc_TS_ID_per_slot[31]&0x07)<<3) + (tmcc_TS_ID_per_slot[32]&0x07);
	tmcc[18] = ((tmcc_TS_ID_per_slot[33]&0x07)<<5) + ((tmcc_TS_ID_per_slot[34]&0x07)<<2) + ((tmcc_TS_ID_per_slot[35]>>1)&0x03);
	tmcc[19] = ((tmcc_TS_ID_per_slot[35]&0x01)<<7) + ((tmcc_TS_ID_per_slot[36]&0x07)<<4) + ((tmcc_TS_ID_per_slot[37]&0x07)<<1) + ((tmcc_TS_ID_per_slot[38]>>2)&0x01);
	tmcc[20] = ((tmcc_TS_ID_per_slot[38]&0x03)<<6) + ((tmcc_TS_ID_per_slot[39]&0x07)<<3) + (tmcc_TS_ID_per_slot[40]&0x07);
	tmcc[21] = ((tmcc_TS_ID_per_slot[41]&0x07)<<5) + ((tmcc_TS_ID_per_slot[42]&0x07)<<2) + ((tmcc_TS_ID_per_slot[43]>>1)&0x03);
	tmcc[22] = ((tmcc_TS_ID_per_slot[43]&0x01)<<7) + ((tmcc_TS_ID_per_slot[44]&0x07)<<4) + ((tmcc_TS_ID_per_slot[45]&0x07)<<1) + ((tmcc_TS_ID_per_slot[46]>>2)&0x01);
	tmcc[23] = ((tmcc_TS_ID_per_slot[46]&0x03)<<6) + ((tmcc_TS_ID_per_slot[47]&0x07)<<3) +
			((tmcc_TS_PID_per_TS_ID[0]>>13)&0x07);////////141 + 3
	tmcc[24] = ((tmcc_TS_PID_per_TS_ID[0]>>5)&0xFF);
	tmcc[25] = ((tmcc_TS_PID_per_TS_ID[0]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[1]>>13)&0x07);
	tmcc[26] = ((tmcc_TS_PID_per_TS_ID[1]>>5)&0xFF);
	tmcc[27] = ((tmcc_TS_PID_per_TS_ID[1]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[2]>>13)&0x07);
	tmcc[28] = ((tmcc_TS_PID_per_TS_ID[2]>>5)&0xFF);
	tmcc[29] = ((tmcc_TS_PID_per_TS_ID[2]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[3]>>13)&0x07);
	tmcc[30] = ((tmcc_TS_PID_per_TS_ID[3]>>5)&0xFF);
	tmcc[31] = ((tmcc_TS_PID_per_TS_ID[3]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[4]>>13)&0x07);
	tmcc[32] = ((tmcc_TS_PID_per_TS_ID[4]>>5)&0xFF);
	tmcc[33] = ((tmcc_TS_PID_per_TS_ID[4]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[5]>>13)&0x07);
	tmcc[34] = ((tmcc_TS_PID_per_TS_ID[5]>>5)&0xFF);
	tmcc[35] = ((tmcc_TS_PID_per_TS_ID[5]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[6]>>13)&0x07);
	tmcc[36] = ((tmcc_TS_PID_per_TS_ID[6]>>5)&0xFF);
	tmcc[37] = ((tmcc_TS_PID_per_TS_ID[6]&0x1F)<<3) + ((tmcc_TS_PID_per_TS_ID[7]>>13)&0x07);
	tmcc[38] = ((tmcc_TS_PID_per_TS_ID[7]>>5)&0xFF);
	tmcc[39] = ((tmcc_TS_PID_per_TS_ID[7]&0x1F)<<3) 
		+ ((tmcc_emergency_alarm&0x01)<<2) + ((tmcc_up_link_site_diversity&0x01)<<1) + ((tmcc_up_link_added>>2)&0x01);
	
	tmcc[40] = ((tmcc_up_link_added&0x03)<<6) + ((tmcc_ext_flag&0x01)<<5) + 0x1F;
	tmcc[41] = 0xFF;
	tmcc[42] = 0xFF;
	tmcc[43] = 0xFF;
	tmcc[44] = 0xFF;
	tmcc[45] = 0xFF;
	tmcc[46] = 0xFF;
	tmcc[47] = 0xFF;
	
	for ( i = 0; i < 16; i++ )
	{
		tmcc[48+i] = 0x00;
	}
	
	for ( i = 0; i < MAX_FRAME_COUNT; i++ )
	{
		if ( i == 0 )
		{
			tail_superframe[i][0] = 0x1B;
			tail_superframe[i][1] = 0x95;
	
			tail_superframe[i][10] = 0xA3;
			tail_superframe[i][11] = 0x40;
		}
		else
		{
			tail_superframe[i][0] = 0x1B;
			tail_superframe[i][1] = 0x95;
	
			tail_superframe[i][10] = 0x5C;
			tail_superframe[i][11] = 0xBF;
		}
	
		tail_superframe[i][2] = tmcc[i*8 + 0];
		tail_superframe[i][3] = tmcc[i*8 + 1];
		tail_superframe[i][4] = tmcc[i*8 + 2];
		tail_superframe[i][5] = tmcc[i*8 + 3];
		tail_superframe[i][6] = tmcc[i*8 + 4];
		tail_superframe[i][7] = tmcc[i*8 + 5];
		tail_superframe[i][8] = tmcc[i*8 + 6];
		tail_superframe[i][9] = tmcc[i*8 + 7];
	}

	return	0;
}
void CHldFmtrIsdbS::GetUserParam_AtCombinerRestart(void)
{
	int	i;

	for ( i = 0; i < MAX_TS_COUNT; i++ )
	{
		sprintf(TS_path[i], "%s", "");
		__nSlot[i] = -1;
		nConst[i] = -1;
		nCode[i] = -1;

		sprintf(TS_path[i], "%s", m_TS_path[i]);
		__nSlot[i] = m_nSlot[i];
		nConst[i] = m_nConst[i];
		nCode[i] = m_nCode[i];
	}
}
void CHldFmtrIsdbS::SwapUserParam_forQpsk_AtCombinerRestart(int single_ts)	//	2011/3/21 FIXED
{
	int	i, j;
	char swapTemp_TsPath[1024];
	int swapTemp_value = 0;

	if(single_ts)
	{
		return;
	}
	sprintf(swapTemp_TsPath, "%s", "");
	for(i = 0; i < MAX_TS_COUNT - 1; i++)
	{
		for(j = i + 1; j < MAX_TS_COUNT; j++)
		{
			if(nConst[i] == QPSK && nConst[j] == QPSK)
			{
				if(nCode[i] < nCode[j])
				{
					//TS PATH
					sprintf(swapTemp_TsPath, "%s", TS_path[i]);
					sprintf(TS_path[i], "%s", TS_path[j]);
					sprintf(TS_path[j], "%s", swapTemp_TsPath);
					//SLOT NUMBER
					swapTemp_value = __nSlot[i];
					__nSlot[i] = __nSlot[j];
					__nSlot[j] = swapTemp_value; 
					//Constellation
					swapTemp_value = nConst[i];
					nConst[i] = nConst[j];
					nConst[j] = swapTemp_value; 
					//CodeRate
					swapTemp_value = nCode[i];
					nCode[i] = nCode[j];
					nCode[j] = swapTemp_value; 
				}
			}
		}
	}
}
void CHldFmtrIsdbS::InitSlotConf_AtCombinerRestart(void)
{
	int	i;
	int nReadBytes, iPacket;
	int _nBert_type = __FIf__->_VarTstPktTyp();
	
	nTC8PSK_TS_Count = 0;
	nQPSK_TS_Count = 0;
	nBPSK_TS_Count = 0;
	
	for ( i = 0; i < MAX_TS_COUNT; i++ )
	{
		fp_mts[i] = NULL;
	
		if ( nConst[i] == BPSK )
		{
			nSlot_Unit[i] = 4;
			nSlot_Valid[i] = 1;
			nSlot_Dummy[i] = 3;
	
			if ( nCode[i] == CODE_1_2  )
			{
				nBPSK_TS[nBPSK_TS_Count] = i;
                if(_nBert_type >=  TS_HEAD_184_ALL_0 && _nBert_type <=  TS_SYNC_187_PRBS_2_23) 
				{
				}
				else
				{
					fp_mts[nBPSK_TS[nBPSK_TS_Count]] = OpenSource(TS_path[i]);
				
					if ( __Sta__->IPUsing() == 0)
					{
						//2012/9/26
						nReadBytes = fread(ts_buf_to_process_dta, 1, SUB_BANK_MAX_BYTE_SIZE, fp_mts[nBPSK_TS[nBPSK_TS_Count]]);
						__FIf__->TL_SyncLockFunction((char*)ts_buf_to_process_dta, nReadBytes, &iPacket, nReadBytes, 3);
						nPacket_Size__[nBPSK_TS[nBPSK_TS_Count]] = iPacket;
						fseek(fp_mts[nBPSK_TS[nBPSK_TS_Count]], 0L, SEEK_SET);
					}
				}
				++nBPSK_TS_Count;
			}
		}
		else if ( nConst[i] == QPSK )
		{
			if ( nCode[i] == CODE_1_2 )
			{
				nSlot_Unit[i] = 2;
				nSlot_Valid[i] = 1;
				nSlot_Dummy[i] = 1;
			}
			else if ( nCode[i] == CODE_2_3 )
			{
				nSlot_Unit[i] = 3;
				nSlot_Valid[i] = 2;
				nSlot_Dummy[i] = 1;
			}
			else if ( nCode[i] == CODE_3_4 )
			{
				nSlot_Unit[i] = 4;
				nSlot_Valid[i] = 3;
				nSlot_Dummy[i] = 1;
			}
			else if ( nCode[i] == CODE_5_6 )
			{
				nSlot_Unit[i] = 6;
				nSlot_Valid[i] = 5;
				nSlot_Dummy[i] = 1;
			}
			else if ( nCode[i] == CODE_7_8 )
			{
				nSlot_Unit[i] = 8;
				nSlot_Valid[i] = 7;
				nSlot_Dummy[i] = 1;
			}
			else
			{
				nSlot_Unit[i] = 0;
				nSlot_Valid[i] = 0;
				nSlot_Dummy[i] = 0;
			}
	
			if ( nCode[i] >= CODE_1_2 && nCode[i] <= CODE_7_8 )
			{
				nQPSK_TS[nQPSK_TS_Count] = i;
                if(_nBert_type >=  TS_HEAD_184_ALL_0 && _nBert_type <=  TS_SYNC_187_PRBS_2_23) 
				{
				}
				else
				{
					fp_mts[nQPSK_TS[nQPSK_TS_Count]] = OpenSource(TS_path[i]);
					
					if ( __Sta__->IPUsing() == 0)
					{
						//2012/9/26
						nReadBytes = fread(ts_buf_to_process_dta, 1, SUB_BANK_MAX_BYTE_SIZE, fp_mts[nQPSK_TS[nQPSK_TS_Count]]);
						__FIf__->TL_SyncLockFunction((char*)ts_buf_to_process_dta, nReadBytes, &iPacket, nReadBytes, 3);
						nPacket_Size__[nQPSK_TS[nQPSK_TS_Count]] = iPacket;
						fseek(fp_mts[nQPSK_TS[nQPSK_TS_Count]], 0L, SEEK_SET);
					}
				}
				++nQPSK_TS_Count;
			}
		}
		else if ( nConst[i] == TC8PSK )
		{
			nSlot_Unit[i] = 1;
			nSlot_Valid[i] = 1;
			nSlot_Dummy[i] = 0;
			
			if ( nCode[i] == CODE_2_3  )
			{
				nTC8PSK_TS[nTC8PSK_TS_Count] = i;
                if(_nBert_type >=  TS_HEAD_184_ALL_0 && _nBert_type <=  TS_SYNC_187_PRBS_2_23) 
				{
				}
				else
				{
					fp_mts[nTC8PSK_TS[nTC8PSK_TS_Count]] = OpenSource(TS_path[i]);
						
					if ( __Sta__->IPUsing() == 0)
					{
						//2012/9/26
						nReadBytes = fread(ts_buf_to_process_dta, 1, SUB_BANK_MAX_BYTE_SIZE, fp_mts[nTC8PSK_TS[nTC8PSK_TS_Count]]);
						__FIf__->TL_SyncLockFunction((char*)ts_buf_to_process_dta, nReadBytes, &iPacket, nReadBytes, 3);
						nPacket_Size__[nTC8PSK_TS[nTC8PSK_TS_Count]] = iPacket;
						fseek(fp_mts[nTC8PSK_TS[nTC8PSK_TS_Count]], 0L, SEEK_SET);
					}
				}
				++nTC8PSK_TS_Count;
			}
		}
		else
		{
			nSlot_Unit[i] = 0;
			nSlot_Valid[i] = 0;
			nSlot_Dummy[i] = 0;
		}
	}

}
void CHldFmtrIsdbS::GetTmccMode_AtCombinerRestart(void)
{
	int	i, kk;

	for ( i = 0; i < 7; i++ )
	{
		tmcc_tx_mode[i] = 0xF;
		tmcc_tx_slot[i] = 0;
	}
	
	//TC8PSK
	kk = 0;
	for ( i = 0; i < nTC8PSK_TS_Count; i++ )
	{
		tmcc_tx_slot[kk] += __nSlot[nTC8PSK_TS[i]];
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x07;
		++kk;
	}
	
	//QPSK - 7/8
	for ( i = 0; i < nQPSK_TS_Count; i++ )
	{
		if ( nCode[nQPSK_TS[i]] == CODE_7_8 )
		{
			tmcc_tx_slot[kk] += __nSlot[nQPSK_TS[i]];
		}
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x06;
		++kk;
	}
	
	//QPSK - 5/6
	for ( i = 0; i < nQPSK_TS_Count; i++ )
	{
		if ( nCode[nQPSK_TS[i]] == CODE_5_6 )
		{
			tmcc_tx_slot[kk] += __nSlot[nQPSK_TS[i]];
		}
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x05;
		++kk;
	}
	
	//QPSK - 3/4
	for ( i = 0; i < nQPSK_TS_Count; i++ )
	{
		if ( nCode[nQPSK_TS[i]] == CODE_3_4 )
		{
			tmcc_tx_slot[kk] += __nSlot[nQPSK_TS[i]];
		}
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x04;
		++kk;
	}
	
	//QPSK - 2/3
	for ( i = 0; i < nQPSK_TS_Count; i++ )
	{
		if ( nCode[nQPSK_TS[i]] == CODE_2_3 )
		{
			tmcc_tx_slot[kk] += __nSlot[nQPSK_TS[i]];
		}
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x03;
		++kk;
	}
	
	//QPSK - 1/2
	for ( i = 0; i < nQPSK_TS_Count; i++ )
	{
		if ( nCode[nQPSK_TS[i]] == CODE_1_2 )
		{
			tmcc_tx_slot[kk] += __nSlot[nQPSK_TS[i]];
		}
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x02;
		++kk;
	}
	
	//BPSK
	for ( i = 0; i < nBPSK_TS_Count; i++ )
	{
		tmcc_tx_slot[kk] += __nSlot[nBPSK_TS[i]];
	}
	
	if ( tmcc_tx_slot[kk] > 0 )
	{
		tmcc_tx_mode[kk] = 0x01;
		++kk;
	}
}
void CHldFmtrIsdbS::GetTsPid_AtCombinerRestart(void)
{
	int	i, j, nReadBytes;
	unsigned short	PID;

	//2012/3/27 BERT 187
	int _nBert_type = __FIf__->_VarTstPktTyp();

	for ( i = 0; i < MAX_TS_COUNT; i++ )
	{
		tmcc_TS_PID_per_TS_ID[i] = 0xFFFF;
	}

	if(_nBert_type >= TS_HEAD_184_ALL_0 && _nBert_type <= TS_SYNC_187_PRBS_2_23)
	{
		return;
	}

	for ( i = 0; i < nTC8PSK_TS_Count; i++ )		//	tc8psk
	{
		for ( j = 0; j < 120000; j++ )
		{
			nReadBytes = RdOnePkt(fp_mts[nTC8PSK_TS[i]], ts_buf_to_process_dta, nPacket_Size__[nTC8PSK_TS[i]], 0, NULL);
			if ( nReadBytes < 0 )
			{
				break;
			}
	
			PID = (((*(ts_buf_to_process_dta+1))&0x1F)<<8) + *(ts_buf_to_process_dta+2);
			if ( PID == 0 )
			{
				tmcc_TS_PID_per_TS_ID[nTC8PSK_TS[i]] = __FIf__->get_tsid(ts_buf_to_process_dta);
				__HLog__->HldPrint_1_s( "TC8PSK, TS, TSID=", TS_path[nTC8PSK_TS[i]]);
				break;
			}
		}
	}
	for ( i = 0; i < nQPSK_TS_Count; i++ )		//	qpsk
	{
		for ( j = 0; j < 120000; j++ )
		{
			nReadBytes = RdOnePkt(fp_mts[nQPSK_TS[i]], ts_buf_to_process_dta, nPacket_Size__[nQPSK_TS[i]], 0, NULL);
			if ( nReadBytes < 0 )
			{
				break;
			}
	
			PID = (((*(ts_buf_to_process_dta+1))&0x1F)<<8) + *(ts_buf_to_process_dta+2);
			if ( PID == 0 )
			{
				tmcc_TS_PID_per_TS_ID[nQPSK_TS[i]] = __FIf__->get_tsid(ts_buf_to_process_dta);
				__HLog__->HldPrint_1_s( "QPSK, TS=, TSID=", TS_path[nQPSK_TS[i]]);
				break;
			}
		}
	}
	for ( i = 0; i < nBPSK_TS_Count; i++ )		//	bpsk
	{
		for ( j = 0; j < 120000; j++ )
		{
			nReadBytes = RdOnePkt(fp_mts[nBPSK_TS[i]], ts_buf_to_process_dta, nPacket_Size__[nBPSK_TS[i]], 0, NULL);
			if ( nReadBytes < 0 )
			{
				break;
			}
	
			PID = (((*(ts_buf_to_process_dta+1))&0x1F)<<8) + *(ts_buf_to_process_dta+2);
			if ( PID == 0 )
			{
				tmcc_TS_PID_per_TS_ID[nBPSK_TS[i]] = __FIf__->get_tsid(ts_buf_to_process_dta);
				__HLog__->HldPrint_1_s( "BPSK, TS=, TSID=", TS_path[nBPSK_TS[i]]);
				break;
			}
		}
	}

}

void CHldFmtrIsdbS::CalcStuffRatio(int _ts_slot)
{
	int	nReadBytes, nRet;
	double	dwRet;

	//2012/3/27 BERT 187
	int _nBert_type = __FIf__->_VarTstPktTyp();

	if (__Sta__->IPUsing() || __Sta__->IsPlayMod_IsdbS())
	{
		if ( __FIf__->GetBitRate() > 100 )
		{
			fRatio[_ts_slot] = (float)((double)__FIf__->GetPlayParam_PlayRate() / (double)__FIf__->GetBitRate());
			if ( fRatio[_ts_slot] < 1 )
			{
				fRatio[_ts_slot] = 1;
			}
			__FIf__->BSTARTClick(fRatio[_ts_slot], StuffRatio[_ts_slot], &AddedNullPacket[_ts_slot], &AddedNullPacket[_ts_slot]);
		}
	}
	else
	{
		//2012/3/27 BERT 187
		if((_nBert_type == TS_SYNC_187_ALL_0 || _nBert_type == TS_SYNC_187_ALL_1 ||
			_nBert_type == TS_SYNC_187_PRBS_2_15 || _nBert_type == TS_SYNC_187_PRBS_2_23))
		{
			fRatio[_ts_slot] = 1.;
			__FIf__->BSTARTClick(fRatio[_ts_slot], StuffRatio[_ts_slot], &AddedNullPacket[_ts_slot], &AddedNullPacket[_ts_slot]);
			return;
		}
		nReadBytes = fread(ts_buf_to_process_dta, 1, SUB_BANK_MAX_BYTE_SIZE, fp_mts[_ts_slot]);
		nRet = __FIf__->TL_SyncLockFunction((char*)ts_buf_to_process_dta, nReadBytes, (int*)&dwRet, nReadBytes, 3);
		
		fRatio[_ts_slot] = 1.;
		if ( nRet >= 0 )
		{
			__FIf__->SetBitRate(CalcPlayRate_SpecifiedFile(TS_path[_ts_slot], 0));
			if ( __FIf__->GetBitRate() > 100 )
			{
				fRatio[_ts_slot] = (float)((double)__FIf__->GetPlayParam_PlayRate() / (double)__FIf__->GetBitRate());
				if ( fRatio[_ts_slot] < 1 )
				{
					fRatio[_ts_slot] = 1;
				}
				__FIf__->BSTARTClick(fRatio[_ts_slot], StuffRatio[_ts_slot], &AddedNullPacket[_ts_slot], &AddedNullPacket[_ts_slot]);
			}
		}
		fseek(fp_mts[_ts_slot], 0L, SEEK_SET);
	}
}
void CHldFmtrIsdbS::CalcStuff_forMTs_AtCombinerRestart(void)
{
	int	i;
	//2012/3/27 BERT 187
	int _nBert_type = __FIf__->_VarTstPktTyp();

	if(_nBert_type >= TS_HEAD_184_ALL_0 && _nBert_type <= TS_SYNC_187_PRBS_2_23)
	{
		return;
	}


	for ( i = 0; i < nTC8PSK_TS_Count; i++ )		//	tc8psk
	{
		if ( nConst[nTC8PSK_TS[i]] == 0 )
		{
			SE_mts = 1.;
		}
		else if ( nConst[nTC8PSK_TS[i]] == 1 )
		{
			SE_mts = 2.;
		}
		else
		{
			SE_mts = 3.;
		}
				
		if ( nConst[nTC8PSK_TS[i]] == 1 )
		{
			if ( nCode[nTC8PSK_TS[i]] == 0 )
			{
			  CR_mts = 1. / 2.;
			}
			else if ( nCode[nTC8PSK_TS[i]] == 1 )
			{
				CR_mts = 2. / 3.;
			}
			else if ( nCode[nTC8PSK_TS[i]] == 2 )
			{
				CR_mts = 3. / 4.;
			}
			else if ( nCode[nTC8PSK_TS[i]] == 3 )
			{
				CR_mts = 5. / 6.;
			}
			else
			{
				CR_mts = 7. / 8.;
			}
		}
		else if ( nConst[nTC8PSK_TS[i]] == 0 )
		{
			CR_mts = 1. / 2.;
		}
		else
		{
			CR_mts = 2. / 3.;
		}
	
		__FIf__->SetPlayParam_PlayRate((long)(SR_mts * SE_mts * CR_mts * (188. / 208.)));
		__FIf__->SetPlayParam_PlayRate(__FIf__->GetPlayParam_PlayRate() * ((double)__nSlot[nTC8PSK_TS[i]] / MAX_SLOT_COUNT));
		//PlayParm.AP_lst.nPlayRate = SR_mts*2.;

		CalcStuffRatio(nTC8PSK_TS[i]);
	}
	for ( i = 0; i < nQPSK_TS_Count; i++ )		//	qpsk
	{
		if ( nConst[nQPSK_TS[i]] == 0 )
		{
			SE_mts = 1.;
		}
		else if ( nConst[nQPSK_TS[i]] == 1 )
		{
			SE_mts = 2.;
		}
		else
		{
			SE_mts = 3.;
		}
				
		if ( nConst[nQPSK_TS[i]] == 1 )
		{
			if ( nCode[nQPSK_TS[i]] == 0 )
			{
			  CR_mts = 1. / 2.;
			}
			else if ( nCode[nQPSK_TS[i]] == 1 )
			{
				CR_mts = 2. / 3.;
			}
			else if ( nCode[nQPSK_TS[i]] == 2 )
			{
				CR_mts = 3. / 4.;
			}
			else if ( nCode[nQPSK_TS[i]] == 3 )
			{
				CR_mts = 5. / 6.;
			}
			else
			{
				CR_mts = 7. / 8.;
			}
		}
		else if ( nConst[nQPSK_TS[i]] == 0 )
		{
			CR_mts = 1. / 2.;
		}
		else
		{
			CR_mts = 2. / 3.;
		}
	
		__FIf__->SetPlayParam_PlayRate((long)(SR_mts * SE_mts * CR_mts * (188. / 208.)));
		__FIf__->SetPlayParam_PlayRate(__FIf__->GetPlayParam_PlayRate() * ((double)__nSlot[nQPSK_TS[i]] / MAX_SLOT_COUNT));
		//PlayParm.AP_lst.nPlayRate = SR_mts*2.;
	
		CalcStuffRatio(nQPSK_TS[i]);
	}
	for ( i = 0; i < nBPSK_TS_Count; i++ )		//	bpsk
	{
		if ( nConst[nBPSK_TS[i]] == 0 )
		{
			SE_mts = 1.;
		}
		else if ( nConst[nBPSK_TS[i]] == 1 )
		{
			SE_mts = 2.;
		}
		else
		{
			SE_mts = 3.;
		}
				
		if ( nConst[nBPSK_TS[i]] == 1 )
		{
			if ( nCode[nBPSK_TS[i]] == 0 )
			{
				CR_mts = 1. / 2.;
			}
			else if ( nCode[nBPSK_TS[i]] == 1 )
			{
				CR_mts = 2. / 3.;
			}
			else if ( nCode[nBPSK_TS[i]] == 2 )
			{
				CR_mts = 3. / 4.;
			}
			else if ( nCode[nBPSK_TS[i]] == 3 )
			{
				CR_mts = 5. / 6.;
			}
			else
			{
				CR_mts = 7. / 8.;
			}
		}
		else if ( nConst[nBPSK_TS[i]] == 0 )
		{
			CR_mts = 1. / 2.;
		}
		else
		{
			CR_mts = 2. / 3.;
		}
	
		__FIf__->SetPlayParam_PlayRate((long)(SR_mts * SE_mts * CR_mts * (188. / 208.)));
		__FIf__->SetPlayParam_PlayRate(__FIf__->GetPlayParam_PlayRate() * ((double)__nSlot[nBPSK_TS[i]] / MAX_SLOT_COUNT));
		//PlayParm.AP_lst.nPlayRate = SR_mts*2.;
	
		CalcStuffRatio(nBPSK_TS[i]);
	}
}
void CHldFmtrIsdbS::add_frame_info(unsigned char* buf, int *cur_kk, int *cur_nSlotNum, int *cur_nFrameNum, int is_dummy_slot, int modulation, int code_rate)
{
	int info = 0x00;
	if ( modulation == TC8PSK ) 
	{
		info = 7;
	}
	else if ( modulation == BPSK ) 
	{
		info = 1;
	}
	else
	{
		if ( code_rate == CODE_1_2 )
		{
			info = 2;
		}
		else if ( code_rate == CODE_2_3 )
		{
			info = 3;
		}
		else if ( code_rate == CODE_3_4 )
		{
			info = 4;
		}
		else if ( code_rate == CODE_5_6 )
		{
			info = 5;
		}
		else
		{
			info = 6;
		}
	}
	info += (is_dummy_slot << 4);

	memset(buf+(*cur_kk), 0, 16);
	*(buf+(*cur_kk)+12) = 0xB8;
	*(buf+(*cur_kk)+13) = info;//0x00;
	*(buf+(*cur_kk)+14) = (unsigned char)(*cur_nFrameNum);
	*(buf+(*cur_kk)+15) = (unsigned char)(*cur_nSlotNum);
	(*cur_kk) += 16;

	++(*cur_nSlotNum);
	if ( (*cur_nSlotNum) >= MAX_SLOT_COUNT )
	{
		(*cur_nSlotNum) = 0;
	}
	
	if ( (*cur_nSlotNum) == 0 )
	{
		++(*cur_nFrameNum);
		if ( (*cur_nFrameNum) >= MAX_FRAME_COUNT )
		{
			(*cur_nFrameNum) = 0;
		}
	}
}

FILE *CHldFmtrIsdbS::FopenCombinedFile_AtReplaceRestart(int *_pkt_size)
{
	FILE	*fp = NULL;
	int	iPacketSize, nReadBytes;
	fp = fopen((char *)__FIf__->GetPlayParam_FName(), "rb"); //	have only one-ts in case of conbined-ts
	if(fp == NULL)
	{
		__Sta__->SetMainTask_LoopState_(TH_NONE);	//	???
		return	NULL;
	}

	fseek(fp, 0L, SEEK_SET);
	iPacketSize = Isdbs_FramedTS_GetPacketSize(fp);
	nReadBytes = read_superframe(fp, ts_buf_cmbnd_rslt, iPacketSize);
	fseek(fp, 0L, SEEK_SET);
	if ( nReadBytes < 0 || feof(fp) != 0 )
	{
		if ( feof(fp) != 0 )
		{
			fseek(fp, 0L, SEEK_SET);
			nReadBytes = read_superframe(fp, ts_buf_cmbnd_rslt, iPacketSize);
			if ( nReadBytes < 0 )
			{
				return	NULL;
			}
		}
		else
		{
			return	NULL;
		}
	}
	fseek(fp, 0L, SEEK_SET);
	*_pkt_size = iPacketSize;

	return	fp;
}
void	CHldFmtrIsdbS::ParseTmcc_ofCombinedFile(int _pkt_size, unsigned char *_ts)
{
	int	i;
    unsigned char	tmcc[48+16];

	for(i = 0; i < MAX_FRAME_COUNT; i++)
	{
		tmcc[i * MAX_FRAME_COUNT] = _ts[_pkt_size * (2 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 1] = _ts[_pkt_size * (3 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 2] = _ts[_pkt_size * (4 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 3] = _ts[_pkt_size * (5 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 4] = _ts[_pkt_size * (6 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 5] = _ts[_pkt_size * (7 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 6] = _ts[_pkt_size * (8 + (48 * i))];
		tmcc[i * MAX_FRAME_COUNT + 7] = _ts[_pkt_size * (9 + (48 * i))];
	}

	tmcc_change_flag_ = (tmcc[0] >> 3) & 0x1F;
	tmcc_replace_mode[0] = ((tmcc[0] << 1) & 0xE) + ((tmcc[1] >> 7) & 0x1);
	tmcc_replace_mode_slot[0] = (tmcc[1] >> 1) & 0x3F;
	tmcc_replace_mode[1] = ((tmcc[1] << 3) & 0x8) + ((tmcc[2] >> 5) & 0x7);
	tmcc_replace_mode_slot[1] = ((tmcc[2] << 1) & 0x3E) + ((tmcc[3] >> 7) & 0x1);
	tmcc_replace_mode[2] = ((tmcc[3] >> 3) & 0xF);
	tmcc_replace_mode_slot[2] = ((tmcc[3] << 3) & 0x38) + ((tmcc[4] >> 5) & 0x7);
	tmcc_replace_mode[3] = ((tmcc[4] >> 1) & 0xF);
	tmcc_replace_mode_slot[3] = ((tmcc[4] << 5) & 0x20) + ((tmcc[5] >> 3) & 0x3F);

}
void	CHldFmtrIsdbS::GetReplacedModeAndActInfo(void)
{
	int i, j, index;

	index = 0;
	tmcc_change_flag_old_ = tmcc_change_flag_;
	//TC8PSK
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x7)
		{
			for(j = 0; j < tmcc_replace_mode_slot[i]; j++)
			{
				replaced_mode_and_act_info[index] = 0x07;
				index++;
			}
			break;
		}
	}
	
	//QPSK(7/8)
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x6)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 8) == 7)
				{
					replaced_mode_and_act_info[index] = 0x16;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x06;
				}
				index++;
			}
			break;
		}
	}
	
	//QPSK(5/6)
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x5)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 6) == 5)
				{
					replaced_mode_and_act_info[index] = 0x15;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x05;
				}
				index++;
			}
			break;
		}
	}
	
	//QPSK(3/4)
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x4)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 4) == 3)
				{
					replaced_mode_and_act_info[index] = 0x14;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x04;
				}
				index++;
			}
			break;
		}
	}
	
	//QPSK(2/3)
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x3)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 3) == 2)
				{
					replaced_mode_and_act_info[index] = 0x13;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x03;
				}
				index++;
			}
			break;
		}
	}
	
	//QPSK(1/2)
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x2)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 2) == 1)
				{
					replaced_mode_and_act_info[index] = 0x12;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x02;
				}
				index++;
			}
			break;
		}
	}
	
	//BPSK
	for(i = 0; i < 4; i++)
	{
		if(tmcc_replace_mode[i] == 0x1)
		{
			for(j = 0 ; j < tmcc_replace_mode_slot[i]; j++)
			{
				if((j % 4) == 0)
				{
					replaced_mode_and_act_info[index] = 0x01;
				}
				else
				{
					replaced_mode_and_act_info[index] = 0x11;
				}
				index++;
			}
			break;
		}
	}

}

//////////////////////////////////////////////////////////////////////////////////////
void CHldFmtrIsdbS::TL_ConvertToCombinedTS__HasEndlessWhile(int single_ts)	//	TL_sz3rdBufferPlay
{
	unsigned short PID;
	int i, j, k;
	int nWrittenBytes, nCount, nProgress;
	CHld	*_hld_ = (CHld *)my_hld;

	single_ts = 1;
	if ( __FIf__->TmccUsing())
	{
		single_ts = 0;
	}
	if (__Sta__->IPUsing() || __Sta__->IsPlayMod_IsdbS())
	{
		single_ts = 1;
	}

	nFrameNum = TL_nFrameNum;//0;
	nSlotNum = TL_nSlotNum;//0;

	nWritePos = 0;
	nReadPos = 0;
	nBufferCnt = 0;
	for (i = 0; i < MAX_TS_COUNT; i++)
	{
		AddedNullPacket[i] = 0;
		TotalReadPacket[i] = 0;
	}
	SR_mts = 28860000.;
	ts_buf_superframe = __FIf__->__BufPlay_Has_RstDtaAllConversion();

	if ( TL_combiner_ready != 1 )
	{
		TL_combiner_ready = 1;

		GetUserParam_AtCombinerRestart();
		SwapUserParam_forQpsk_AtCombinerRestart(single_ts);

		InitSlotConf_AtCombinerRestart();	//	determine the slot configuration. and fp_mts are opend at here.
		GetTmccMode_AtCombinerRestart();	//	tmcc tx mode. it's the one of the tmcc-contents.

		GetTsPid_AtCombinerRestart();		//	parse pid from each-ts-file.
		
		for ( i = 0; i < MAX_TS_COUNT; i++ )
		{
			if ( fp_mts[i] != NULL )
			{
				fseek(fp_mts[i], 0L, SEEK_SET);
			}
		}
		CalcStuff_forMTs_AtCombinerRestart();		//	calc stuffing ratio to match the output symbol.
	}
	else
	{
		for ( i = 0; i < MAX_TS_COUNT; i++ )
		{
			if ( fp_mts[i] != NULL )
			{
				fp_mts[i] = NULL;	//	????
			}
		}
	}

	//////////////////////////////////////
	nProgress = 0;
	off__ts_buf_superframe = 0;
	do
	{
		if ( TL_ISDBS_PumpingThreadDone == 1 )
		{
			break;
		}

		off__ts_buf_to_process_dta = 0;
		EstimateRuntimeBr(1, (unsigned long)off__ts_buf_to_process_dta);
		for ( i = 0; i < nTC8PSK_TS_Count; i++ )
		{
			nCount = __nSlot[nTC8PSK_TS[i]] / nSlot_Unit[nTC8PSK_TS[i]];
			
			for ( j = 0; j < nCount; j++ )
			{
				if (FillOnePkt_intoProcessBuf(single_ts, nTC8PSK_TS[i]) < 0)
				{
					if(__Sta__->IPUsing() == 1)
					{
						goto _WAIT_IP_DATA_;
					}
					goto _END_OF_FILE_;
				}
			}

			memcpy(ts_buf_superframe+off__ts_buf_superframe, ts_buf_to_process_dta, off__ts_buf_to_process_dta);
			off__ts_buf_superframe += off__ts_buf_to_process_dta;
			off__ts_buf_to_process_dta = 0;
		}

		off__ts_buf_to_process_dta = 0;
		EstimateRuntimeBr(1, (unsigned long)off__ts_buf_to_process_dta);
		for ( i = 0; i < nQPSK_TS_Count; i++ )
		{
			nCount = __nSlot[nQPSK_TS[i]] / nSlot_Unit[nQPSK_TS[i]];
			
			for ( j = 0; j < nCount; j++ )
			{
				if (FillOnePkt_intoProcessBuf(single_ts, nQPSK_TS[i]) < 0)
				{
					if(__Sta__->IPUsing() == 1)
					{
						goto _WAIT_IP_DATA_;
					}
					goto _END_OF_FILE_;
				}
			}

			memcpy(ts_buf_superframe+off__ts_buf_superframe, ts_buf_to_process_dta, off__ts_buf_to_process_dta);
			off__ts_buf_superframe += off__ts_buf_to_process_dta;
			off__ts_buf_to_process_dta = 0;
		}

		off__ts_buf_to_process_dta = 0;
		EstimateRuntimeBr(1, (unsigned long)off__ts_buf_to_process_dta);
		for ( i = 0; i < nBPSK_TS_Count; i++ )
		{
			nCount = __nSlot[nBPSK_TS[i]] / nSlot_Unit[nBPSK_TS[i]];
			
			for ( j = 0; j < nCount; j++ )
			{
				if (FillOnePkt_intoProcessBuf(single_ts, nBPSK_TS[i]) < 0)
				{
					if(__Sta__->IPUsing() == 1)
					{
						goto _WAIT_IP_DATA_;
					}
					goto _END_OF_FILE_;
				}
			}

			memcpy(ts_buf_superframe+off__ts_buf_superframe, ts_buf_to_process_dta, off__ts_buf_to_process_dta);
			off__ts_buf_superframe += off__ts_buf_to_process_dta;
			off__ts_buf_to_process_dta = 0;
		}

		if ( off__ts_buf_superframe <= 0 )
		{
			Sleep(10);
			continue;
		}

		//////////////////////////////////////
		FillTailSuperframe();

		///////////////////////////////////////////////
		if ( nFrameNum == 0 )
		{
			for ( i = 0; i < MAX_FRAME_COUNT; i++ )
			{
				for ( j = 0; j < 12; j++ )
				{
					ts_buf_superframe[204*(MAX_SLOT_COUNT*i+j)] = tail_superframe[i][j];
				}
			}
			LockIsdbSMutex();
			if (TL_sz3rdBufferPlay == NULL)
			{
				UnlockIsdbSMutex();
				goto	_END_OF_FILE_;
			}
			__FIf__->FillOnePacketToQueue(off__ts_buf_superframe, TL_sz3rdBufferPlay, (int*)&TL_nWritePos3, (int*)&TL_nReadPos3, (int*)&TL_nBufferCnt3, SUB_BANK_MAX_BYTE_SIZE*8, ts_buf_superframe);
			UnlockIsdbSMutex();

			while ( (int)TL_nBufferCnt3 >= SUB_BANK_MAX_BYTE_SIZE*8 - off__ts_buf_superframe )
			{
				if ( TL_ISDBS_PumpingThreadDone == 1 )
				{
					break;
				}
				Sleep(10);
			}

			if ( fp_dump )
			{
				nWrittenBytes = fwrite(ts_buf_superframe, 1, off__ts_buf_superframe, fp_dump);
			}

			off__ts_buf_superframe = 0;
		}

		TL_nFrameNum = nFrameNum;
		TL_nSlotNum = nSlotNum;
		if(__Sta__->IPUsing() == 1)
		{

			if((_hld_->CntBytesInIpBuf() / 1024) < 15 * 1024)
				Sleep(10);

		}

_WAIT_IP_DATA_:
		;
	} while (1);

_END_OF_FILE_:
	
	/////////////////////////////////////
	for ( i = 0; i < MAX_TS_COUNT; i++ )
	{
		if ( fp_mts[i] != NULL )
		{
			fclose(fp_mts[i]);
		}
	}
	__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
}

void CHldFmtrIsdbS::TL_ReplaceCombinedTS__HasEndlessWhile()		//	TL_sz3rdBufferPlay
{
	FILE *fp = NULL;
	int i, j;
	int nReadBytes;
	//2011/6/20
	int iPacketSize;
	unsigned char ts_buf_188[204 * 48 * 8];

	tmcc_change_flag_old_ = -1;

	ts_buf_cmbnd_rslt = __FIf__->__BufPlay_Has_RstDtaAllConversion();

	if ( TL_combiner_ready != 1 )
	{
		TL_combiner_ready = 1;
		fp = FopenCombinedFile_AtReplaceRestart(&iPacketSize);
		if (fp == NULL)
		{
			goto	_END_OF_FILE_;
		}

		ParseTmcc_ofCombinedFile(iPacketSize, ts_buf_cmbnd_rslt);
		GetReplacedModeAndActInfo();
	}
	else
	{
		if ( fp != NULL )
		{
			fp = NULL;
		}
	}


	while(1)
	{
		if ( TL_ISDBS_PumpingThreadDone == 1 )
		{
			break;
		}
		nReadBytes = read_superframe(fp, ts_buf_cmbnd_rslt, iPacketSize);
		if ( nReadBytes < 0 || feof(fp) != 0 )
		{
			if((int)TL_nBufferCnt3 < SUB_BANK_MAX_BYTE_SIZE * 8 - (204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT))
			{
				goto _END_OF_FILE_;
			}
		}
#if 1
		ParseTmcc_ofCombinedFile(iPacketSize, ts_buf_cmbnd_rslt);
		if(tmcc_change_flag_ != tmcc_change_flag_old_)
		{
			GetReplacedModeAndActInfo();
		}
#endif
		if(iPacketSize == 188)
		{
			int index_188 = 0;
			int index_204 = 0;
			for(i = 0 ; i < MAX_FRAME_COUNT; i++)
			{
				for(j = 0 ; j < MAX_SLOT_COUNT ; j++)
				{
					memcpy(ts_buf_188 + index_204, ts_buf_cmbnd_rslt + index_188, iPacketSize);
					ts_buf_188[0 + 204 * ((48 * i) + j) + 200] = 0xB8;
					ts_buf_188[0 + 204 * ((48 * i) + j) + 201] = replaced_mode_and_act_info[j];
					ts_buf_188[0 + 204 * ((48 * i) + j) + 202] = (unsigned char)i;
					ts_buf_188[0 + 204 * ((48 * i) + j) + 203] = (unsigned char)j;
					index_188 = 188 * ((48 * i) + j);
					index_204 = 204 * ((48 * i) + j);
				}
			}
			LockIsdbSMutex();
			if (TL_sz3rdBufferPlay == NULL)
			{
				UnlockIsdbSMutex();
				goto	_END_OF_FILE_;
			}
			__FIf__->FillOnePacketToQueue((204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT),
					TL_sz3rdBufferPlay, (int*)&TL_nWritePos3, (int*)&TL_nReadPos3, (int*)&TL_nBufferCnt3, SUB_BANK_MAX_BYTE_SIZE*8, ts_buf_188);
			UnlockIsdbSMutex();
		}
		else
		{
			for(i = 0 ; i < MAX_FRAME_COUNT; i++)
			{
				for(j = 0 ; j < MAX_SLOT_COUNT ; j++)
				{
					ts_buf_cmbnd_rslt[0 + 204 * ((48 * i) + j) + 200] = 0xB8;
					ts_buf_cmbnd_rslt[0 + 204 * ((48 * i) + j) + 201] = replaced_mode_and_act_info[j];
					ts_buf_cmbnd_rslt[0 + 204 * ((48 * i) + j) + 202] = (unsigned char)i;
					ts_buf_cmbnd_rslt[0 + 204 * ((48 * i) + j) + 203] = (unsigned char)j;
				}
			}
			LockIsdbSMutex();
			if (TL_sz3rdBufferPlay == NULL)
			{
				UnlockIsdbSMutex();
				goto	_END_OF_FILE_;
			}
			__FIf__->FillOnePacketToQueue((204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT),
					TL_sz3rdBufferPlay, (int*)&TL_nWritePos3, (int*)&TL_nReadPos3, (int*)&TL_nBufferCnt3, SUB_BANK_MAX_BYTE_SIZE*8, ts_buf_cmbnd_rslt);
			UnlockIsdbSMutex();
		}

		if ( fp_dump )
		{
			if(iPacketSize == 188)
			{
				fwrite(ts_buf_188, 1, 188 * MAX_SLOT_COUNT * MAX_FRAME_COUNT, fp_dump);
			}else
			{
				fwrite(ts_buf_cmbnd_rslt, 1, 204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT, fp_dump);
			}
		}
		while ( (int)TL_nBufferCnt3 >= SUB_BANK_MAX_BYTE_SIZE*8 - (204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT))
		{
			if ( TL_ISDBS_PumpingThreadDone == 1 )
			{
				break;
			}
			Sleep(10);
		}
	}

_END_OF_FILE_:
	
	/////////////////////////////////////
	if ( fp != NULL )
	{
		fclose(fp);
		TL_combiner_ready = 0;
		__Sta__->SetMainTask_LoopState_(TH_END_PLAY);
	}
}
void CHldFmtrIsdbS::TL_ReplaceCombinedTS_IN_ASI__HasEndlessWhile(int nPacketSize, DWORD* dwBYTEsRead)
{

	int i, j;
	int nSyncPos = -1, nSearchStep = 0, nNextIndex, nPacketLen;
	long lTemp = 0;
	unsigned int dwBytesToRead = nPacketSize*48*8*2;

	tmcc_change_flag_old_ = -1;

	if (  __FIf__->TL_nBCount <= dwBytesToRead ) 
		return;

_FIND_204_SYNC_SEARCH_:
	// find 204/208 sync.
	if ( __FIf__->TL_nFindSync == 1 )
	{
		do {
			__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(__FIf__->TL_pbBuffer, dwBytesToRead);
			nSyncPos = __FIf__->TL_SyncLockFunction_ISDBS((unsigned char*)__FIf__->TL_pbBuffer, dwBytesToRead, &nPacketLen, dwBytesToRead, 3);
			if ( nSyncPos >= 0 )
			{
				__FIf__->SetNewRdAlignPos_of_PlayBuf(nSyncPos);
				break;
			}
			__FIf__->SetNewRdAlignPos_of_PlayBuf(dwBytesToRead/2);

		} while ( __FIf__->TL_nBCount > dwBytesToRead );

		if ( nSyncPos < 0 ) 
		{
			return;
		}

		__FIf__->TL_nFindSync = 0;
	}

	// fill the sync buffer
	while ( __FIf__->TL_nBCount > dwBytesToRead )
	{
		//FIXED - Bitrate adjustment
		if ( __FIf__->TL_nBufferCnt > MAX_BUFFER_BYTE_SIZE - __FIf__->gfRatio * nPacketSize )
		{
			break;
		}
		__FIf__->FillGivenBuf_from_PlayBuf_w_ChkBoundary(__FIf__->TL_pbBuffer, nPacketSize * 48 * 8);

		if(__FIf__->TL_pbBuffer[0] != 0x1B)
		{
			__FIf__->TL_nFindSync = 1;
			goto _FIND_204_SYNC_SEARCH_;
		}
		__FIf__->SetNewRdAlignPos_of_PlayBuf(nPacketSize * 48 * 8);

		ParseTmcc_ofCombinedFile(nPacketSize, __FIf__->TL_pbBuffer);
		if(tmcc_change_flag_ != tmcc_change_flag_old_)
		{
			GetReplacedModeAndActInfo();
		}
		for(i = 0 ; i < MAX_FRAME_COUNT; i++)
		{
			for(j = 0 ; j < MAX_SLOT_COUNT ; j++)
			{
				__FIf__->TL_pbBuffer[0 + 204 * ((48 * i) + j) + 200] = 0xB8;
				__FIf__->TL_pbBuffer[0 + 204 * ((48 * i) + j) + 201] = replaced_mode_and_act_info[j];
				__FIf__->TL_pbBuffer[0 + 204 * ((48 * i) + j) + 202] = (unsigned char)i;
				__FIf__->TL_pbBuffer[0 + 204 * ((48 * i) + j) + 203] = (unsigned char)j;
			}
		}

//////////////////////////////////////////////////////////////////////
		__FIf__->FillOnePacketToQueue((204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT),
				__FIf__->TL_sz2ndBufferPlay, (int*)&__FIf__->TL_nWritePos, (int*)&__FIf__->TL_nReadPos, (int*)&__FIf__->TL_nBufferCnt,
				MAX_BUFFER_BYTE_SIZE, __FIf__->TL_pbBuffer);

		nNextIndex = __FIf__->TL_nRIndex + (204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT);
		if ( nNextIndex >= MAX_BUFFER_BYTE_SIZE )
		{
			nNextIndex -= MAX_BUFFER_BYTE_SIZE;
		}

		if (*(__FIf__->TL_szBufferPlay + nNextIndex) != 0x1B )
		{
			__FIf__->TL_nFindSync = 1;
			__FIf__->TL_nWritePos = 0;
			__FIf__->TL_nReadPos = 0;
			__FIf__->TL_nBufferCnt = 0;
			goto _FIND_204_SYNC_SEARCH_;
		}
	}
}

//2011/2/11 ISBD-S CombinedTS
#if 1
/* return -1 if error or EOF. Return 0 if OK. */
int CHldFmtrIsdbS::read_superframe(FILE *pb, unsigned char *buf, int raw_packet_size)
{
	int skip, len;
	int readDataSize = raw_packet_size * 48 * 8;
	for(;;)
	{
		len = fread(buf, 1, readDataSize, pb);
        if(len < readDataSize)
			return -1;
		//check superframe
		if(buf[0] == 0x1B && buf[raw_packet_size] == 0x95 && buf[raw_packet_size * 10] == 0xA3 && buf[raw_packet_size * 11] == 0x40)
		{
			break;
		}
		else
		{
			int	i;
			//find a new superframe
			for(i = 1; i < len ; i++)
			{
				if( i >= (len - (raw_packet_size * 11))) 
				{
					skip = len - i;
					fseek(pb, -skip, SEEK_CUR);
					break;
				}

				if(buf[i] == 0x1B && buf[i + raw_packet_size] == 0x95 && buf[i + raw_packet_size * 10] == 0xA3 && buf[i + raw_packet_size * 11] == 0x40)
				{

					skip = len - i;
					fseek(pb, -skip, SEEK_CUR);
					//2011/6/16 Fixed.
					len = fread(buf, 1, readDataSize, pb);
					if(len < readDataSize)
						return -1;
					return 0;
				}
			}
		}
	}
	return 0;
}

#endif
#if 1
//2011/6/16  ISDB-S Framed TS 188 packet Support
int CHldFmtrIsdbS::Isdbs_FramedTS_GetPacketSize(FILE *pb)
{
	unsigned char buf[204 * 48 * 8];
	int skip, len, len2;
	int ipacket = 204;
	int readDataSize = SUB_BANK_MAX_BYTE_SIZE;
	int j;
	int retry = 0;
	unsigned char *ts_buf_ = __FIf__->__Buf_Temp();

	for(;;)
	{
		len = fread(ts_buf_, 1, SUB_BANK_MAX_BYTE_SIZE, pb);
        if(len < readDataSize)
			return -1;
		for(int i = 0; i < len; i++)
		{
			if(ts_buf_[i] == 0x1B)
			{
				skip = len - i;
				fseek(pb, -skip, SEEK_CUR);
				len2 = fread(buf, 1, 204 * 48 * 8, pb);
				if(len2 < 204 * 48 * 8)
					return -1;
				for(j = 0; j < 8; j++)
				{
					if(buf[204 * 48 * j] != 0x1B || buf[204 + 204 * 48 * j] != 0x95)
						break;
				}
				if(j >= 8)
					return 204;

				for(j = 0; j < 8; j++)
				{
					if(buf[188 * 48 * j] != 0x1B || buf[188 + 188 * 48 * j] != 0x95)
						break;
				}
				if(j >= 8)
					return 188;
				fseek(pb, skip, SEEK_CUR);
				fseek(pb, -len2, SEEK_CUR);
			}
		}
		retry++;
		if(retry > 1)
			break;
	}
	return -1;
}
#endif
int CHldFmtrIsdbS::CalcNullStuffCount(unsigned int totalReadPacket,  int *stuffRatio)
{
	int sutff_count = 0;

	if (stuffRatio[0] >= 10)
	{
		sutff_count = (stuffRatio[0]/10);

		if (totalReadPacket % 10 == 0)
		{
			sutff_count += (stuffRatio[0] % 10);
		}
	} 
	else
	{
		if (totalReadPacket % 10 == 0)
		{
			sutff_count = stuffRatio[0];
		}
	}

	if (totalReadPacket % 100 == 0)
	{
		sutff_count += stuffRatio[1];
	}
    
	if (totalReadPacket % 1000 == 0)
	{
		sutff_count += stuffRatio[2];
	}

	if (totalReadPacket % 10000 == 0)
	{
		sutff_count += stuffRatio[3];	
	}

	if (totalReadPacket % 100000 == 0)
	{
		sutff_count += stuffRatio[4];	
	}
	
	if (totalReadPacket % 1000000 == 0)
	{
		sutff_count += stuffRatio[5];	
	}

	if (totalReadPacket % 10000000 == 0)
	{
		sutff_count += stuffRatio[6];	
	}

	if (totalReadPacket % 100000000 == 0)
	{
		sutff_count += stuffRatio[7];	
	}

	if (totalReadPacket % 1000000000 == 0)
	{
		sutff_count += stuffRatio[8];	
	}

    return sutff_count;
}
int CHldFmtrIsdbS::SetIsCombinedTs_IsdbS(char* ts_path)
{
	size_t	readByte;
	BYTE	*AP_szBuffer;
	int nTryCount = MAX_REPEAT_TIME;
	HANDLE hFile;
	
#if defined(WIN32)
	hFile = CreateFile(ts_path,	GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if ( hFile == INVALID_HANDLE_VALUE)
	{
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open ", ts_path);
		return -1;
	}
	unsigned long dwSizeLow, dwSizeHigh, dwError;
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	if ( dwSizeLow == 0xFFFFFFFF && (dwError = GetLastError()) != NO_ERROR )
	{
		CloseHandle(hFile);
		return -1;
	}
	CloseHandle(hFile);
#else
	hFile = fopen(ts_path, "rb");
	if ( hFile == NULL)
	{
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open ", ts_path);
		return -1;
	}
	unsigned long dwError;
	off_t dwSizeLow, dwSizeHigh;
	fseeko((FILE*)hFile, 0L, SEEK_END);
	dwSizeLow = ftello((FILE*)hFile);
	fseeko((FILE*)hFile, 0L, SEEK_SET);
	if ( dwSizeLow <= 0 )
	{
		fclose((FILE*)hFile);
		return -1;
	}
	fclose((FILE*)hFile);
#endif
	dwSizeLow = (DWORD)((dwSizeLow & 0xFFFFFFFF)/1024)*1024;
	FILE *fp = fopen(ts_path, "rb");
    int start_pos = -1;
	int cur_pos = -1;
	int next_pos = -1;
	int W1_pos = -1;
	int W2_pos = -1;
	int i;

	if ( fp == NULL )
	{
		return -1;
	}
	//2011/6/20
	int iPacketSize;

	fseek(fp, 0L, SEEK_SET);
	
	iPacketSize = Isdbs_FramedTS_GetPacketSize(fp);
	
	if(iPacketSize == -1)
	{
		fclose((FILE*)fp);
		return -1;
	}

	fseek(fp, dwSizeLow/2, SEEK_SET);
	//-----------------------------------------------------------------------
	// Memory allocation
	AP_szBuffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE);
	if (AP_szBuffer == NULL)
	{
		__HLog__->HldPrint("[HLD:TSPH_IS_COMBINED_TS] FAIL alloc-memory");
		fclose((FILE*)fp);
		return	-1;
	}

	//read stream data
	readByte = fread(AP_szBuffer,  1, SUB_BANK_MAX_BYTE_SIZE, fp);
	i = 0;
	while(readByte > i)
	{
		if(W1_pos < 0)
		{
			if(AP_szBuffer[i] == 0x1B && AP_szBuffer[i+iPacketSize] == 0x95)
			{
				if(AP_szBuffer[i + (iPacketSize * 10)] == 0xA3 && AP_szBuffer[i + (iPacketSize * 11)] == 0x40)
				{
					__HLog__->HldPrint("[HLD:TSPH_IS_COMBINED_TS] Find Frame sync and Post frame sync.");
					W1_pos = i;
				}
			}
		}
		if(W1_pos >= 0)
		{
			
			W1_pos = W1_pos + (iPacketSize * 48 * 8);

			if(readByte <= W1_pos)
			{
				break;
			}
			if(AP_szBuffer[W1_pos] == 0x1B && AP_szBuffer[W1_pos + iPacketSize] == 0x95)
			{
				if(AP_szBuffer[W1_pos + (iPacketSize * 10)] == 0xA3 && AP_szBuffer[W1_pos + (iPacketSize * 11)] == 0x40)
				{
					i = W1_pos;
				}
				else
				{
					free(AP_szBuffer);
					fclose((FILE*)fp);
					return -1;
				}
			}
			else
			{
				free(AP_szBuffer);
				fclose((FILE*)fp);
				return -1;
			}
		}
		else
		{
			i++;
		}
	}
	free(AP_szBuffer);
	fclose((FILE*)fp);
	if(W1_pos < 0)
		return -1;
	nCombinedTS = 1;
	return 0;
}
int	CHldFmtrIsdbS::CalcPlayRate_SpecifiedFile(char *szFile, int iType)
{
	nCombinedTS = 0;
#ifdef TSPHLD0110_EXPORTS
	if ( iType == 0 ) return 2048000;
	if ( iType == 1 ) return -1;
	return 0;
	
#else

	//2012/2/13
#if defined(WIN32)
	if ( (__Sta__->IsModTyp_QamA() || __Sta__->IsModTyp_Qpsk() || __Sta__->IsModTyp_DvbS2() ||
		__Sta__->IsModTyp_DvbT() || __Sta__->IsModTyp_DvbH()) && iType == 0)
	{
		//Func_NIT_Init(szFile, &st_Satellite, &st_Cable);
//CKIM TEST M {
		//__FIf__->RunTs_Parser(-1, szFile, 19392658, 1);
		struct _TSPH_TS_INFO *dummy;
		__FIf__->RunTs_Parser(szFile, 19392658, 1, &dummy);
//CKIM TEST M }
	}
#endif
//	Func_PSI_init(szFile);
	//char debug_string[256];
	//sprintf(debug_string, "[Result] flag %d, frequency 0x%x, symbolrate 0x%x, modulation 0x%x, coderate 0x%x  \n", 
	//	st_Satellite.i_descriptor_flag, st_Satellite.i_frequeny, st_Satellite.i_symbolrate, st_Satellite.i_modulation, st_Satellite.i_coderate);
	//OutputDebugString(debug_string);

#if defined(WIN32)
	HANDLE hFile;
	hFile = CreateFile(szFile,	GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if ( hFile == INVALID_HANDLE_VALUE)
	{
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open ", szFile);
		return -1;
	}
#else
	FILE *hFile;
	hFile = fopen(szFile, "rb");
	if ( hFile == NULL)
	{
		__HLog__->HldPrint_1_s("Hld-Bd-Ctl. FAIL to open ", szFile);
		return -1;
	}
#endif

#if defined(WIN32)
	unsigned long dwSizeLow, dwSizeHigh, dwError;

	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	if ( dwSizeLow == 0xFFFFFFFF && (dwError = GetLastError()) != NO_ERROR )
	{
		CloseHandle(hFile);
		return -1;
	}
#else
	unsigned long dwError;
	off_t dwSizeLow, dwSizeHigh;

	fseeko(hFile, 0L, SEEK_END);
	dwSizeLow = ftello(hFile);
	fseeko(hFile, 0L, SEEK_SET);
#endif
	
	unsigned long dwReadByte;
//CKIM M 20120625 {
//	unsigned char *szBuffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE);
	unsigned char *szBuffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE * 4);
//CKIM M 20120625 }
	if ( !szBuffer )
	{
#if defined(WIN32)	
		CloseHandle(hFile);
#else
		fclose(hFile);
#endif
		return -1;
	}
	
	long transRate = -1;
	int	syncStart, iTSize;
	int i;
	int nTryCount = 10;
	long	nBitrate[3], nMax=-1, nMin=99999999, nCount=0;
	nBitrate[0] = nBitrate[1] = nBitrate[2] = 0;
	dwSizeLow = (DWORD)((dwSizeLow & 0xFFFFFFFF)/1024)*1024;
#if defined(WIN32)
//	SetFilePointer(hFile, dwSizeLow/2, NULL, FILE_BEGIN);

	//CMMB
	if ( __Sta__->IsModTyp_Cmmb() )
	{
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	}

	//DVB-T2 ???
	if ( __Sta__->IsModTyp_DvbT2() )
	{
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	}


#else
	fseeko(hFile, (dwSizeLow/2), SEEK_SET);

	//CMMB
	if ( __Sta__->IsModTyp_Cmmb() )
	{
		fseeko(hFile, 0L, SEEK_SET);
	}

#endif
	
	do
	{
#if defined(WIN32)
//CKIM M 20120625 {
//		ReadFile(hFile, szBuffer, SUB_BANK_MAX_BYTE_SIZE, &dwReadByte, NULL);
		ReadFile(hFile, szBuffer, SUB_BANK_MAX_BYTE_SIZE * 4, &dwReadByte, NULL);
//CKIM M 20120625 }
#else
//CKIM M 20120625 {
//		dwReadByte = fread(szBuffer, 1, SUB_BANK_MAX_BYTE_SIZE, hFile);
		dwReadByte = fread(szBuffer, 1, SUB_BANK_MAX_BYTE_SIZE * 4, hFile);
//CKIM M 20120625 }
#endif
		if (dwReadByte <= 0)
		{
			break;
		}

		//CMMB
		if ( __Sta__->IsModTyp_Cmmb() )
		{
			if (iType == 0) 
			{
				__Cmmb__->StreamAnalyzer(szBuffer, dwReadByte, &transRate, &i);
			}
			else
			{
				transRate = -1;
			}
			break;
		}
		else
		{
			syncStart = __FIf__->TL_SyncLockFunction((char*)szBuffer, dwReadByte, &iTSize, dwReadByte, 3);
		}

		if (syncStart != -1)
		{
			// bitrate
			if (iType == 0) 
			{
				transRate = __FIf__->TL_Calculate_Bitrate(szBuffer, dwReadByte, syncStart, iTSize, NULL, NULL);
				if (transRate < 100)
				{
					transRate = -1;
					continue;
				}

				nBitrate[nCount] = transRate;
				if ( nCount == 0 )
				{
					nMin = nMax = transRate;
				}
				else
				{
					if ( transRate > nMax )
					{
						nMax = transRate;
					}
					else if ( transRate < nMin )
					{
						nMin = transRate;
					}
				}

				if ( ++nCount < 3 ) 
				{
//CKIM M 20120625 {
//					if ( dwSizeLow >= SUB_BANK_MAX_BYTE_SIZE*4 )
					if ( dwSizeLow >= SUB_BANK_MAX_BYTE_SIZE*4 * 4 )
//CKIM M 20120625 }
					{
						transRate = -1;
						continue;
					}
				}
				else
				{
					for ( i = 0; i < 3; i++ )
					{
						if ( nBitrate[i] == nMin )
							;
						else if ( nBitrate[i] == nMax )
							;
						else
						{
							transRate = nBitrate[i];
							break;
						}
					}

					break;
				}
			}
			// packet size
			else 
			{
				transRate = iTSize;
			}

			// check it's valid ????
			if (transRate < 100)
			{
				transRate = -1;
				continue;
			}
			break;
		}
	} while (--nTryCount > 0);

	if ( szBuffer )
	{
		free(szBuffer);
	}
#if defined(WIN32)
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFile);
	}
#else
	if ( hFile != NULL )
	{
		fclose((FILE*)hFile);
	}
#endif

	if (iType == 0)
	{
		if ( nCount > 0 && nTryCount < 2 )
		{
			transRate = nBitrate[nCount-1];
		}
		
		__FIf__->SetBitRate(transRate);
	}

	return transRate;//packet size or bps

#endif
}

int CHldFmtrIsdbS::CalcPlayRate_IsdbS_SpecifiedFile(char* ts_path)
{
	FILE *fp = NULL;
	int i, index;
    unsigned char tmcc[48+16];
	unsigned char tmcc_trans_mode[4];
	unsigned char tmcc_trans_mode_slot[4];
	int nReadBytes;
	double bitrate = 0;
	unsigned char ts_buf__[204 * MAX_SLOT_COUNT * MAX_FRAME_COUNT];
    const double SR = 28860000;
	double SE, CR;
	int totalSlot;
	int retry = 0;
	//2011/6/20
	int iPacketSize;

	fp = fopen(ts_path, "rb");

	fseek(fp, 0L, SEEK_SET);
	
	iPacketSize = Isdbs_FramedTS_GetPacketSize(fp);

	fseek(fp, 0L, SEEK_SET);
	while(1)
	{
		bitrate = 0;
		nReadBytes = read_superframe(fp, ts_buf__, iPacketSize);
		if ( nReadBytes < 0 || feof(fp) != 0)
		{
			break;
		}
		for(i = 0; i < MAX_FRAME_COUNT; i++)
		{
			tmcc[i * MAX_FRAME_COUNT] = ts_buf__[iPacketSize * (2 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 1] = ts_buf__[iPacketSize * (3 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 2] = ts_buf__[iPacketSize * (4 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 3] = ts_buf__[iPacketSize * (5 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 4] = ts_buf__[iPacketSize * (6 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 5] = ts_buf__[iPacketSize * (7 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 6] = ts_buf__[iPacketSize * (8 + (48 * i))];
			tmcc[i * MAX_FRAME_COUNT + 7] = ts_buf__[iPacketSize * (9 + (48 * i))];
		}

		tmcc_trans_mode[0] = ((tmcc[0] << 1) & 0xE) + ((tmcc[1] >> 7) & 0x1);
		tmcc_trans_mode_slot[0] = (tmcc[1] >> 1) & 0x3F;
		tmcc_trans_mode[1] = ((tmcc[1] << 3) & 0x8) + ((tmcc[2] >> 5) & 0x7);
		tmcc_trans_mode_slot[1] = ((tmcc[2] << 1) & 0x3E) + ((tmcc[3] >> 7) & 0x1);
		tmcc_trans_mode[2] = ((tmcc[3] >> 3) & 0xF);
		tmcc_trans_mode_slot[2] = ((tmcc[3] << 3) & 0x38) + ((tmcc[4] >> 5) & 0x7);
		tmcc_trans_mode[3] = ((tmcc[4] >> 1) & 0xF);
        tmcc_trans_mode_slot[3] = ((tmcc[4] << 5) & 0x20) + ((tmcc[5] >> 3) & 0x3F);
		
		index = 0;
		//TC8PSK
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x7)
			{
				SE = 3.0;
				CR = 2.0 / 3.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		//QPSK(7/8)
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x6)
			{
				SE = 2.0;
				CR = 7.0 / 8.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		//QPSK(5/6)
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x5)
			{
				SE = 2.0;
				CR = 5.0 / 6.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		//QPSK(3/4)
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x4)
			{
				SE = 2.0;
				CR = 3.0 / 4.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		//QPSK(2/3)
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x3)
			{
				SE = 2.0;
				CR = 2.0 / 3.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		//QPSK(1/2)
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x2)
			{
				SE = 2.0;
				CR = 1.0 / 2.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}
	
		//BPSK
		for(i = 0; i < 4; i++)
		{
			if(tmcc_trans_mode[i] == 0x1)
			{
				SE = 1.0;
				CR = 1.0 / 2.0;
				bitrate = bitrate + ((SR * SE * CR * 188.0 / 208.0) * tmcc_trans_mode_slot[i] / 48.0);
				break;
			}
		}

		totalSlot = tmcc_trans_mode_slot[0] + tmcc_trans_mode_slot[1] + tmcc_trans_mode_slot[2] + tmcc_trans_mode_slot[3];
		if(totalSlot == 48)
		{
			break;
		}
		if(retry++ > 9)
		{
			bitrate = 0;
			break;
		}
	}
	fclose((FILE*)fp);
	return (int)bitrate;

}
int	CHldFmtrIsdbS::SetCombinerInfo_IsdbS(int ts_count, char *ts_path, long modulation, long code_rate, long slot_count)
{
	int i;
	if ( ts_count == 0 )
	{
		for ( i = 0; i < MAX_TS_COUNT; i++ )
		{
			sprintf((char*)m_TS_path[i], "%s", "");
			m_nConst[i] = -1;
			m_nCode[i] = -1;
			m_nSlot[i] = -1;
		}

		TL_combiner_TS_count = 0;
	}

	if ( modulation < 0 || code_rate < 0 || slot_count == 0 )
	{
		return -1;
	}

	/* BPSK */
	if ( modulation == 0 )
	{
		code_rate = 0;
	}
	/* TC8PSK */
	else if ( modulation == 2 )
	{
		code_rate = 1;
	}

	sprintf((char*)m_TS_path[ts_count], "%s", ts_path);
	m_nConst[ts_count] = modulation;
	m_nCode[ts_count] = code_rate;
	m_nSlot[ts_count] = slot_count;
	TL_combiner_TS_count = ts_count;
	++(TL_combiner_TS_count);

	return 0;
}
int	CHldFmtrIsdbS::SetBaseTs_IsdbS(char *ts_path)
{
	sprintf((char*)gszISDBS_baseTS, "%s", ts_path);

	return 0;
}


void CHldFmtrIsdbS::TL_WRITE_BLOCK(void)	//	isdbS and t2mi.
{
	int nBankBlockSize = (__FIf__->SdramSubBankSize());
	int nBufferCount;

	nBufferCount = TL_nBufferCnt3;
//	nBankBlockSize = 1024 * 1024;

	if (__FIf__->SetDmaPtr_HostSide() == 0)
	{
		return;
	}

	if ( nBufferCount >= nBankBlockSize)	//	threr is enough data
	{
		__FIf__->next_file_ready = FALSE;	//	why?
		__FIf__->MapTvbBdDestAddr_fromBnkId();
		if ( __Sta__->IsModTyp_IsdbS() )
		{
			LockIsdbSMutex();

			__FIf__->FillDmaSrcDta_PlayDirection(TL_sz3rdBufferPlay + TL_nReadPos3, nBankBlockSize);
			TL_nReadPos3 += nBankBlockSize;
			TL_nBufferCnt3 -= nBankBlockSize;
			if (TL_nReadPos3 >= (SUB_BANK_MAX_BYTE_SIZE*8))
				TL_nReadPos3 -= (SUB_BANK_MAX_BYTE_SIZE*8);

			UnlockIsdbSMutex();

			if(__Sta__->IsAttachedTvbTyp_SupportIsdbSLoopThru())
			{
				if(__FIf__->WaitConsumePlayBuf_toMaxLevel_HasEndlessWhile(200) == 0)
					return;
			}
		}
		__FIf__->StartDmaTransfer(nBankBlockSize);
		__FIf__->IncBankAddrHwDest_Sdram(__FIf__->GetPlayParam_PlayRate());
	}
	else
	{
		Sleep(10);	//	wait because this func is called by task-endless.
	}

	return;
}
int CHldFmtrIsdbS::IsdbS_ContFilePlayback(void)
{
	if ( __Sta__->IsModTyp_IsdbS() )
	{
		if(nCombinedTS == 1 && __Sta__->IPUsing() == 0)
		{
			TL_ReplaceCombinedTS__HasEndlessWhile();	//	not return until end of playback
		}
		else
		{
			TL_ConvertToCombinedTS__HasEndlessWhile(0);	//	not return until end of playback
		}
		return 0;
	}

	return	1;
}

//////////////////////////////////////////////////////////////////////////////////////
void	CHldFmtrIsdbS::LaunchIsdbS_WrTask(void)	//	task to write isbdS data to HW
{
	CHld	*_hld_ = (CHld *)my_hld;

	if ( __Sta__->IsModTyp_IsdbS() )
	{
		AllocateCapPlayBufIsdbS();
		//2011/8/24
		if(__Sta__->IsAttachedTvbTyp_SupportIsdbSLoopThru())
		{
			__FIf__->SetHwDmaDiection_Play();
			__FIf__->TSPL_RESET_SDCON();
			__FIf__->SetHwFifoCntl_(0, __FIf__->SdramSubBankSize());
		}
#if defined(WIN32)
		_beginthread(_hld_->HLD_ISDBS_PumpingThread, 0, (PVOID)_hld_);
#else
		pthread_create(&TL_ISDBS_thread, NULL, _hld_->HLD_ISDBS_PumpingThread, (PVOID)_hld_);
#endif
	}
}
#ifdef WIN32
void CHldFmtrIsdbS::HLD_ISDBS_PumpingThread(PVOID param)
#else
void* CHldFmtrIsdbS::HLD_ISDBS_PumpingThread(PVOID param)
#endif
{
	CHld	*_hld_;

	_hld_ = (CHld *)param;
	if ( !_hld_ )
	{
#ifdef WIN32
		return;
#else
		return 0;
#endif
	}

	_hld_->CreateIsdbSMutex();

	_hld_->TL_ISDBS_PumpingThreadDone = 0;

	while ( _hld_->TL_nBufferCnt3 < SUB_BANK_MAX_BYTE_SIZE )
	{
		if ( _hld_->_SysSta->ReqedNewAction_User() )
		{
			_hld_->TL_ISDBS_PumpingThreadDone = 1;
			goto	__exit;
		}

		Sleep(10);
	}

	do
	{
		if(_hld_->_SysSta->ReqedStartDelay_User())
		{
			Sleep(10);
			continue;
		}
		_hld_->TL_WRITE_BLOCK();		//	wr TL_sz3rdBufferPlay
	}
	while (!_hld_->_SysSta->ReqedNewAction_User());

__exit:
	_hld_->TL_ISDBS_PumpingThreadDone = 1;

	_hld_->LockIsdbSMutex();
	_hld_->FreeCapPlayBufIsdbS();
	_hld_->UnlockIsdbSMutex();
	_hld_->DestroyIsdbSMutex();

#ifdef WIN32
	return;
#else
	return 0;
#endif	
}

//2012/2/15 NIT
void CHldFmtrIsdbS::Get_NIT_Satellite_Info(int *descriptor_flag, int *freq, int *rolloff, int *modulation_system, int *modulation, int *symbolrate, int *coderate)
{
	__FIf__->GetSatelliteDescriptorInfo(descriptor_flag, freq, rolloff, modulation_system, modulation, symbolrate, coderate);
}
void CHldFmtrIsdbS::Get_NIT_Cable_Info(int *descriptor_flag, int *freq, int *modulation, int *symbolrate, int *coderate)
{
	__FIf__->GetCableDescriptorInfo(descriptor_flag, freq, modulation, symbolrate, coderate);
}
void CHldFmtrIsdbS::Get_NIT_Terrestrial_Info(int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
											 int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod)
{
	__FIf__->GetTerrestrialDescriptorInfo(descriptor_flag, freq, bw, time_slicing, mpe_fec, 
								constellation, coderate, guard, txmod);
}
//CKIM A 20120827 {
void CHldFmtrIsdbS::Get_NIT_Satellite_Info2(int *descriptor_flag, int *freq, int *rolloff, int *modulation_system, int *modulation, int *symbolrate, int *coderate)
{
	__FIf__->GetSatelliteDescriptorInfo2(descriptor_flag, freq, rolloff, modulation_system, modulation, symbolrate, coderate);
}
void CHldFmtrIsdbS::Get_NIT_Cable_Info2(int *descriptor_flag, int *freq, int *modulation, int *symbolrate, int *coderate)
{
	__FIf__->GetCableDescriptorInfo2(descriptor_flag, freq, modulation, symbolrate, coderate);
}
void CHldFmtrIsdbS::Get_NIT_Terrestrial_Info2(int *descriptor_flag, unsigned int *freq, int *bw, int *time_slicing, 
											 int *mpe_fec, int *constellation, int *coderate, int *guard, int *txmod)
{
	__FIf__->GetTerrestrialDescriptorInfo2(descriptor_flag, freq, bw, time_slicing, mpe_fec, 
								constellation, coderate, guard, txmod);
}
//CKIM A 20120827 }