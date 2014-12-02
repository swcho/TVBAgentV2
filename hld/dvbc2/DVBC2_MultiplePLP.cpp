#include <malloc.h>
#include <string.h>

#include "dvbc2_system.h"
#include "CUtility.h"
#include "dvbc2_bbframe_payload.h"
#include "dvbc2_timestamp_payload.h"
#include "dvbc2_l1_payload.h"
#include "dvbc2_input_processing.h"
#include "dvbc2_multipleplp.h"
#include "media_interface.h"
#include "dvbc2_multiplexer.h"
#include "hldplay.h"

#define C2_BANDWIDTH_142 142
#define C2_BANDWIDTH_284 284

#define C2_VALUE_8BIT       255
#define C2_VALUE_9BIT       511
#define C2_VALUE_13BIT     8191
#define C2_VALUE_14BIT    16383

#define C2_GUARD_INDEX_0      0
#define C2_GUARD_INDEX_1      1

#define C2_PLP_CODE_2_3	   0
#define C2_PLP_CODE_3_4	   1
#define C2_PLP_CODE_4_5	   2
#define C2_PLP_CODE_5_6	   3
#define C2_PLP_CODE_8_9	   4

#define C2_PLP_FEC_16K		   0
#define C2_PLP_FEC_64K        1

#define C2_PLP_MOD_16QAM	   0
#define C2_PLP_MOD_64QAM	   1
#define C2_PLP_MOD_256QAM	   2
#define C2_PLP_MOD_1024QAM	   3
#define C2_PLP_MOD_4096QAM	   4

#define C2_FEC_HEADER_TYPE_0   0
#define C2_FEC_HEADER_TYPE_1   1

#define C2_L1TI_MODE_NONE		0
#define C2_L1TI_MODE_BEST_FIT	1
#define C2_L1TI_MODE_4SYMBOLS	2
#define C2_L1TI_MODE_8SYMBOLS	3

#define C2_BANDWIDTH_6M		0
#define C2_BANDWIDTH_7M		1
#define C2_BANDWIDTH_8M		2

#define C2_ARRAY_COUNT_1    4
#define C2_ARRAY_COUNT_0    3408


//#define MULTI_PLP_DEBUG


CDVBC2_MultiplePLP::CDVBC2_MultiplePLP()   // Default constructor
{
	state = DVBC2_MULTI_PLP_STATE_NO_EXIST;

	dvbc2_input_param.L1_COD = 0; // 1/2, default value 

	
}


CDVBC2_MultiplePLP::~CDVBC2_MultiplePLP()
{	


	
}

void CDVBC2_MultiplePLP::init(DVBC2_PARAM _c2mi_multiple_plp, DVBC2Multiplexer *_dvbc2_multiplexer)
{

	//printf("===== %s %d \n", __FILE__, __LINE__);
	dvbc2_multiplexer = _dvbc2_multiplexer;
	state = DVBC2_MULTI_PLP_STATE_INIT;
	dvbc2_input_param = _c2mi_multiple_plp;	
	// tricks: l1_code is always 0 
	// value 
	dvbc2_input_param.L1_COD = 0;

	
	// payload initization	
	for (int i = 0; i < dvbc2_input_param.num_plp; i++)
	{
		int PLP_COD = dvbc2_input_param.list_plp[i].PLP_COD;
		int PLP_FEC_TYPE  = dvbc2_input_param.list_plp[i].PLP_FEC;
		int PLP_MODE = dvbc2_input_param.list_plp[i].PLP_HEM;
		payload_bb[i].init(PLP_COD, PLP_FEC_TYPE, PLP_MODE, dvbc2_input_param.num_plp, dvbc2_multiplexer->play_sys->media_interface[i]);
	}

	
	// timestamp initialization
	payload_timestamp.init(dvbc2_input_param.BW);
	//printf("===== %s %d \n", __FILE__, __LINE__);
}

void CDVBC2_MultiplePLP::quit()
{
	if (state == DVBC2_MULTI_PLP_STATE_CLOSE)
		state = DVBC2_MULTI_PLP_STATE_NO_EXIST;

}


int CDVBC2_MultiplePLP::open(){

	int i;
	if (state == DVBC2_MULTI_PLP_STATE_INIT || state == DVBC2_MULTI_PLP_STATE_CLOSE)
	state = DVBC2_MULTI_PLP_STATE_OPEN;

	// internal setting
	packet_count = 0; 
	superframe_idx = 0;
	plp_index = 0; // suport only 1 plp_id
	frame_idx = 0;	
	packet_index = 0;
	
#if 1
	if (dvbc2_input_param.auto_searching_data_symbol_num_block == 1)
	{	
		int _plp_num_block;

		int plp_bitrate = payload_bb[0].media_source->mpeg2ts_statistic.get_bitrate();

		if (plp_bitrate <= 0)
		{
//				printf("assert: can not calculate bitrate function: %s, line: %d \n", __FUNCTION__, __LINE__);
			return -1;
		}

		dvbc2_input_param.list_plp[0].PLP_BITRATE = plp_bitrate;

		SearchingParamater(&_plp_num_block);

		dvbc2_input_param.list_plp[0].PLP_NUM_BLOCKS = _plp_num_block;	

	}
#endif
	// print log 
	//
	//printf(" --- dvbt2 setting ----- \n");
	//printf("# of data symbol : %d \n",  dvbt2_input_param.NUM_DATA_SYMBOLS);
	//for (i = 0; i < dvbt2_input_param.num_plp; i++)
		//printf("# of data block @ plp [%d]  : %d \n", i,  dvbt2_input_param.list_plp[i].PLP_NUM_BLOCKS);
	


	PLP_NUM_BLOCKS_TOTAL = 0;

	int _PLP_NUM_BLOCK[10]; // maximun: 10 plp 	

	// internal initialization 
	for (i = 0; i < dvbc2_input_param.num_plp; i++)
	{
		PLP_NUM_BLOCKS_TOTAL += dvbc2_input_param.list_plp[i].PLP_NUM_BLOCKS;
		_PLP_NUM_BLOCK[i] = dvbc2_input_param.list_plp[i].PLP_NUM_BLOCKS;
	}

	

	// initialize  BBFrame order algorithm 
	{
		int num_plp = dvbc2_input_param.num_plp;
		CDVBC2_MultiplePLP::BBFrameSelection(num_plp,  &_PLP_NUM_BLOCK[0], &this->bbframe_order[0]);

		// initialize the "is_intl_frame_start"
		for (i = 0; i < PLP_NUM_BLOCKS_TOTAL; i++)
		{
			is_intl_frame_start[i] = 0;
		}
		for (int j = 0; j < num_plp; j++)
		{
			for (i = 0; i < PLP_NUM_BLOCKS_TOTAL; i++)
			{
				if (bbframe_order[i] == j)
				{
					is_intl_frame_start[i] = 1;
					break;
				}
			}	
		}

	}

	// l1 initialization	
	payload_l1.init(dvbc2_input_param);

	// Set number of bit per C2 frame 
	{
		int fft_size = dvbc2_input_param.FFT_SIZE;
		int guard_interval = dvbc2_input_param.GUARD_INTERVAL;
		int num_data_symbol = 448;
		int l1_ti_mode = dvbc2_input_param.L1_COD;
		double c2_duration = c2frame_duration(l1_ti_mode, guard_interval, num_data_symbol);	
		for (int i = 0; i < dvbc2_input_param.num_plp; i++){	
			int plp_bitrate = dvbc2_input_param.list_plp[i].PLP_BITRATE;
			int plp_bit_per_c2 = int((c2_duration/1000.0)*plp_bitrate);			
			payload_bb[i].SetC2Frame(int(plp_bit_per_c2/8)*8);		
			// seting target bitrate for each PLP
			int dfl_in_bits = payload_bb[i].max_dfl_in_bits;
			int num_block = dvbc2_input_param.list_plp[i].PLP_NUM_BLOCKS;
			payload_bb[i].input_processing.set_target_bitrate((dfl_in_bits*num_block*1000.0)/c2_duration);
		}
	}
	
	//printf("============== %s %d  \n", __FILE__, __LINE__);
	return 0;
}

void CDVBC2_MultiplePLP::close(){

	if (state == DVBC2_MULTI_PLP_STATE_OPEN )
	{
		state = DVBC2_MULTI_PLP_STATE_OPEN;
	}
}

int CDVBC2_MultiplePLP::GetNextC2MIPacket(c2mi_packet_status *status, unsigned char *buff){

	if (state == DVBC2_MULTI_PLP_STATE_OPEN)
	{

		int plp_id = 0; 
		int intl_frame_start = 0;

		if (packet_index < PLP_NUM_BLOCKS_TOTAL)
		{	
			plp_index = bbframe_order[packet_index];
			intl_frame_start = is_intl_frame_start[packet_index];
			plp_id = dvbc2_input_param.list_plp[plp_index].PLP_ID;
			int rc= MakeC2MIPacket(C2MI_PACKET_TYPE_BBF, plp_index, packet_count, superframe_idx, frame_idx, plp_id, intl_frame_start);
			if (rc < 0)
				return -1;

			status->packet_type = C2MI_PACKET_TYPE_BBF;
			goto END_PACKET;
		}else if (packet_index == PLP_NUM_BLOCKS_TOTAL)
		{ // timestamp

			MakeC2MIPacket(C2MI_PACKET_TYPE_TIMESTAMP, plp_index, packet_count, superframe_idx, frame_idx, plp_id, intl_frame_start);
			status->packet_type = C2MI_PACKET_TYPE_TIMESTAMP;
			goto END_PACKET;
		}else 
		{ // l1
			MakeC2MIPacket(C2MI_PACKET_TYPE_L1CURRENT, plp_index, packet_count, superframe_idx, frame_idx, plp_id, intl_frame_start);
			status->packet_type = C2MI_PACKET_TYPE_L1CURRENT;
			// goto end_frame
		}

//END_FRAME:
		// frame_idx is incremented by each "C2MI packet transition"	
		frame_idx++;
		frame_idx = (frame_idx >= dvbc2_input_param.NUM_C2_FRAME ? 0: frame_idx);
		// superframe_idx is incremented by each C2MI_FRAME_IDX_MAX					
		superframe_idx = (frame_idx == 0 ? superframe_idx + 1: superframe_idx);
		superframe_idx = (frame_idx ==  C2MI_FRAME_IDX_MAX ? 0 : superframe_idx);

		// check state of all bb payload


END_PACKET:

		status->packet_index = packet_index;
		status->num_bits = this->C2MIPacketToBuff(plp_index, buff);
		packet_count++;
		packet_count = (packet_count > 0xFF ? 0: packet_count);
		packet_index++;
		packet_index = (packet_index ==  PLP_NUM_BLOCKS_TOTAL + 2 ? 0 : packet_index);


		return 0;
	} 

	return 0;

}



int CDVBC2_MultiplePLP::GetL1PreLenInBits()
{
	return 0;
}

// todo: make this method as static

int CDVBC2_MultiplePLP::SearchingParamater(int *PLP_BLOCK )
{
	int startFreq = ((dvbc2_input_param.TX_ID_AVAILABILTY & 0xFF) << 16) + (dvbc2_input_param.Cell_ID & 0xFFFF);
	int guard_interval = dvbc2_input_param.GUARD_INTERVAL;	
	int reservedTone = dvbc2_input_param.BW_EXT;
	int numNotch = dvbc2_input_param.L1_POST_EXTENSION;
	int notchStart = ((dvbc2_input_param.NUM_C2_FRAME & 0xFF) << 6) + ((dvbc2_input_param.NUM_DATA_SYMBOLS >> 6) & 0x3F);
	int notchWidth = ((dvbc2_input_param.NUM_DATA_SYMBOLS & 0xFF) << 3) + (dvbc2_input_param.REGEN_FLAG & 0x7);
	int dsliceFecHeader = (dvbc2_input_param.FEC_TYPE & 0x1);

	int plp_fec;
	int plp_mod;
	int plp_bitrate;
	plp_fec = dvbc2_input_param.list_plp[0].PLP_FEC;
	plp_mod = dvbc2_input_param.list_plp[0].PLP_MOD - 1;
	plp_bitrate = dvbc2_input_param.list_plp[0].PLP_BITRATE;
	

	*PLP_BLOCK =  CDVBC2_MultiplePLP::SearchingParamater(
		startFreq, 
		guard_interval, 
		reservedTone, 
		numNotch, 
		notchStart,  
		notchWidth, 
		dsliceFecHeader,
		plp_fec, 
		plp_mod,
		plp_bitrate);
	
	return 0;

}
int CDVBC2_MultiplePLP::CalcC2MI_Ctot(long nStartFreq, long nGuardInterval, long nReservedTones, long nNumNotch, long nNotchStart, long nNotchWidth)
{
	long Dx, Dy;
	long K_N_min, K_N_max;
	long i, j;
	long C2Frame[C2_ARRAY_COUNT_1][C2_ARRAY_COUNT_0];
	long Symbols[C2_ARRAY_COUNT_1]; 
	long Cell_tot;
	


	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		Symbols[i] = 0;
		for(j = 0; j < C2_ARRAY_COUNT_0; j++)
		{
			C2Frame[i][j] = 0;
		}
	}

	long Locs_Of_Cont_Pilots[] = {96, 216, 306, 390, 450, 486, 780, 804, 924, 1026, 1224, 1422, 1554, 1620, 1680, 1902, 1956, 2016, 2142,
								  2220, 2310, 2424, 2466, 2736, 3048, 3126, 3156, 3228, 3294, 3366};

	long Locs_Of_Rsvd_Tones[] = {161, 243, 296, 405, 493, 584, 697, 741, 821, 934, 1021, 1160, 1215, 1312, 1417, 1462, 1591, 1693, 1729, 1845, 1910,
								 1982, 2127, 2170, 2339, 2365, 2499, 2529, 2639, 2745, 2864, 2950, 2992, 3119, 3235, 3255, 3559, 3620, 3754, 3835, 
								 3943, 3975, 4061, 4210, 4270, 4371, 4417, 4502, 4640, 4677, 4822, 4904, 5026, 5113, 5173, 5271, 5317, 5426, 5492, 
								 5583, 5740, 5757, 5839, 5935, 6033, 6146, 6212, 6369, 6454, 6557, 6597, 6711, 6983, 7047, 7173, 7202, 7310, 7421,
								 7451, 7579, 7666, 7785, 7831, 7981, 8060, 8128, 8251, 8326, 8369, 8445, 8569, 8638, 8761, 8873, 8923, 9017, 9104,
								 9239, 9283, 9368, 9500, 9586, 9683, 9782, 9794, 9908, 9989, 10123, 10327, 10442, 10535, 10658, 10739, 10803, 10925,
								 11006, 11060, 11198, 11225, 11326, 11474, 11554, 11663, 11723, 11810, 11902, 11987, 12027, 12117, 12261, 12320, 12419,
								 12532, 12646, 12676, 12808, 12915, 12941, 13067, 13113, 13246, 13360, 13426, 13520, 13811, 13862, 13936, 14073, 14102,
								 14206, 14305, 14408, 14527, 14555, 14650, 14755, 14816, 14951, 15031, 15107, 15226, 15326, 15392, 15484, 15553, 15623,
								 15734, 15872, 15943, 16043, 16087, 16201, 16299, 16355, 16444, 16514, 16635, 16723, 16802, 16912, 17150, 17285, 17387,
								 17488, 17533, 17603, 17708, 17793, 17932, 18026, 18081, 18159, 18285, 18356, 18395, 18532, 18644, 18697, 18761, 18874,
								 18937, 19107, 19119, 19251, 19379, 19414, 19522, 19619, 19691, 19748, 19875, 19935, 20065, 20109, 20261, 20315, 20559,
								 20703, 20737, 20876, 20950, 21069, 21106, 21231, 21323, 21379, 21494, 21611, 21680, 21796, 21805, 21958, 22027, 22091,
								 22167, 22324, 22347, 22459, 22551, 22691, 22761, 22822, 22951, 22981, 23089, 23216, 23290, 23402, 23453, 23529, 23668,
								 23743, 24019, 24057, 24214, 24249, 24335, 24445, 24554, 24619, 24704, 24761, 24847, 24947, 25089, 25205, 25274, 25352,
								 25474, 25537, 25612, 25711, 25748, 25874, 25984, 26078, 26155, 26237, 26324, 26378, 26545, 26623, 26720, 26774, 26855,
								 26953, 27021, 27123};
	if(nGuardInterval == C2_GUARD_INDEX_0) //1/128
	{
		Dx = 24;
		Dy = 4;
	}
	else if(nGuardInterval == C2_GUARD_INDEX_1) // 1/64
	{
		Dx = 12;
		Dy = 4;
	}
	else
	{
		Dx = 0;
		Dy = 0;
	}

 	K_N_min = nNotchStart * Dx + nStartFreq + 1;
	K_N_max = (nNotchStart + nNotchWidth) * Dx + nStartFreq - 1;

	//Ctot
	for(i = 0; i < C2_ARRAY_COUNT_1 ; i++)
	{
		for(j = nStartFreq; j < (nStartFreq + C2_ARRAY_COUNT_0) ; j++)
		{
			//Check Scatter Pilots
			if((j % (Dx * Dy)) == (Dx * (i % Dy)))
			{
				C2Frame[i][(j - nStartFreq)] = 1;
			}

			//Check Continual Pilots
			for(int aa = 0 ; aa < 30 ; aa++)
			{
				if((j % C2_ARRAY_COUNT_0) == Locs_Of_Cont_Pilots[aa])
				{
					C2Frame[i][(j - nStartFreq)] = 2;
				}
			}

			//Check Reserved Tones
			if(nReservedTones == 1)
			{
				for(int bb = 0; bb < 288; bb++)
				{
					if(((j % (8 * 3408)) - (Dx * (i % Dy))) == Locs_Of_Rsvd_Tones[bb])
					{
						C2Frame[i][(j - nStartFreq)] = 3;
					}
				}
			}

			//Check_Edge Pilots
			if(nNumNotch == 0)
			{
				if( j == nStartFreq )
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}
			else
			{
				if( j == nStartFreq || j == (K_N_min - 1) || j == (K_N_max + 1))
				{
					C2Frame[i][(j - nStartFreq)] = 5;
				}
			}

			//Check Notches
			if(nNumNotch == 1 && j >= K_N_min && j <= K_N_max)
			{
				C2Frame[i][(j - nStartFreq)] = 4;
			}

			if(C2Frame[i][j - nStartFreq] == 0)
				Symbols[i]++;
		}
	}
	Cell_tot = (Symbols[0] + Symbols[1] + Symbols[2] + Symbols[3]) * 112;

	return Cell_tot;
}
int CDVBC2_MultiplePLP::CalcC2MI_plpCells(long nPlpFecType, long nPlpMod, long nFecHeaderType)
{
	long PlpCells;
	long Ncells;
	long FECFrameHeader;

	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 4050;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 2700;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 2025;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 1620;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 1350;
		}
		else
		{
			Ncells = 0;
		}
	}
	else //64K
	{
		if(nPlpMod == C2_PLP_MOD_16QAM)
		{
			Ncells = 16200;
		}
		else if(nPlpMod == C2_PLP_MOD_64QAM) 
		{
			Ncells = 10800;
		}
		else if(nPlpMod == C2_PLP_MOD_256QAM) 
		{
			Ncells = 8100;
		}
		else if(nPlpMod == C2_PLP_MOD_1024QAM) 
		{
			Ncells = 6480;
		}
		else if(nPlpMod == C2_PLP_MOD_4096QAM) 
		{
			Ncells = 5400;
		}
		else
		{
			Ncells = 0;
		}
	}

	if(nFecHeaderType == C2_FEC_HEADER_TYPE_0)
		FECFrameHeader = 32;
	else
		FECFrameHeader = 16;


	PlpCells = (Ncells + FECFrameHeader);
	return PlpCells;
}


int CDVBC2_MultiplePLP::SearchingParamater(int startFreq, int guard_interval, int reservedTone, int numNotch, int notchStart, int notchWidth,
										   int dsliceFecHeader, int plp_fec_type, int plp_mod, int plp_bitrate)
{

	int bandwidth = dvbc2_input_param.BW - 2;
	int L1Ti = dvbc2_input_param.L1_COD;
	int plpcod = dvbc2_input_param.list_plp[0].PLP_COD - 1;
	int hem = dvbc2_input_param.list_plp[0].PLP_HEM;
	int Maximum_Cells;
	int plp_Cell;
	int numblk = 100;
	int bitrate;
	int i;
	Maximum_Cells = CalcC2MI_Ctot(startFreq, guard_interval, reservedTone, numNotch, notchStart, notchWidth);
	plp_Cell = CalcC2MI_plpCells(plp_fec_type, plp_mod, dsliceFecHeader);
	
	bitrate = CalcC2MI_PLPBitrate(bandwidth, L1Ti, guard_interval, plpcod, plp_fec_type, numblk, hem);
	if(Maximum_Cells >= (plp_Cell * numblk))
	{
		if(bitrate == plp_bitrate)
		{
			return numblk;
		}
		else if(bitrate < plp_bitrate) //bitrate : calculated bitrate, plp_bitrate : file bitrate
		{
			for(i = 100 ; i < 1024; i++)
			{
				bitrate = CalcC2MI_PLPBitrate(bandwidth, L1Ti, guard_interval, plpcod, plp_fec_type, i, hem);
				if((Maximum_Cells >= (plp_Cell * i)) && (bitrate >= plp_bitrate))
					return i;
				else if(Maximum_Cells < (plp_Cell * i))
				{
					return (i - 1);
				}
			}
		}
		else
		{
			for(i = 100 ; i > 1; i--)
			{
				bitrate = CalcC2MI_PLPBitrate(bandwidth, L1Ti, guard_interval, plpcod, plp_fec_type, i, hem);
				if(bitrate == plp_bitrate)
					return i;
				else if(bitrate < plp_bitrate)
				{
					return (i + 1);
				}
			}
		}

	}
	else
	{
		for(i = 100 ; i > 1; i--)
		{
			if(Maximum_Cells >= (plp_Cell * i))
			{
				bitrate = CalcC2MI_PLPBitrate(bandwidth, L1Ti, guard_interval, plpcod, plp_fec_type, i, hem);
				if(bitrate == plp_bitrate)
					return i;
				else if(bitrate < plp_bitrate)
				{
					return (i + 1);
				}
			}
		}
	}
	return 0;
}

int CDVBC2_MultiplePLP::CalcC2MI_PLPBitrate(long nBandWidth, long nL1TIMode, long nGuardInterval, long nPlpCod, long nPlpFecType, long nPlpNumBlk, long nHem)
{
	long K_bch, LF;
	double T, TF;
	double PlpBitrate;
	long N_fec_frame, GI;
	double M;

	N_fec_frame = nPlpNumBlk;
	
	if(nHem == 0)
		M = 1.0;
	else
		M = 188.0 / 187.0;

	if(nGuardInterval == C2_GUARD_INDEX_0)
		GI = 32;
	else
		GI = 64;

	// T
	if(nBandWidth == C2_BANDWIDTH_6M)
		T = 7.0 / 1000000.0 / 48.0;
	else if(nBandWidth == C2_BANDWIDTH_7M)
		T = 7.0 / 1000000.0 / 56.0;
	else
		T = 7.0 / 1000000.0 / 64.0;

	//LF
	if(nL1TIMode == C2_L1TI_MODE_NONE || nL1TIMode == C2_L1TI_MODE_BEST_FIT)
		LF = 449;
	else if(nL1TIMode == C2_L1TI_MODE_4SYMBOLS)
		LF = 452;
	else
		LF = 456;

	//LDPC_code, K_bch, K_ldpc
	if(nPlpFecType == C2_PLP_FEC_16K) //16K
	{
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
		    K_bch = 10632;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
		    K_bch = 11712;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			K_bch = 12432;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			K_bch = 13152;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
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
		if(nPlpCod == C2_PLP_CODE_2_3)
		{
	        K_bch = 43040;
		}
		else if(nPlpCod == C2_PLP_CODE_3_4)
		{
			K_bch = 48408;
		}
		else if(nPlpCod == C2_PLP_CODE_4_5)
		{
			K_bch = 51648;
		}
		else if(nPlpCod == C2_PLP_CODE_5_6)
		{
			K_bch = 53840;
		}
		else if(nPlpCod == C2_PLP_CODE_8_9)
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
	return (long)PlpBitrate;
}


int CDVBC2_MultiplePLP::ParameterValidation(int fft_size, int guard_interval, int pilot, int num_plp, int *plp_fec_type, int *plp_cod, int *plp_mod, int num_data_symbol, int *plp_num_block)
{

#define DVBC2_VALID_PARAMETER 0x00
#define DVBC2_INVALID_FFT_SIZE 0x01
#define DVBC2_INVALID_GUAR_INTERVAL 0x02
#define DVBC2_INVALID_NUM_DATA_SYMBOL 0x04
#define DVBC2_INVALID_PLP_MOD 0x08
#define DVBC2_INVALID_PLP_COD 0x10
#define DVBC2_INVALID_PLP_FEC 0x20
#define DVBC2_INVALID_PLP_NUM_BLOCK 0x40


	int _fft_size = fft_size;
	int _guard_interval = guard_interval;
	int _pilot = pilot;
	int _num_plp = num_plp;
	int _num_data_symbol = num_data_symbol; 

//	int _num_data_symbol_max = c2frame_num_data_symbol_max(_fft_size, _guard_interval);
	//int bandwidth = multiple_plp_interface.BW;
	double _c2_duration = 0; //c2frame_duration(_fft_size, _guard_interval, _num_data_symbol, bandwidth);

	if (_c2_duration == -1.0)
	{
		// wrong fft_size, _guard_interval, num_data_symbol
		return DVBC2_INVALID_FFT_SIZE | DVBC2_INVALID_GUAR_INTERVAL | DVBC2_INVALID_NUM_DATA_SYMBOL; 
	}else 
	{
		int _data_symbol_num_cell  = c2frame_data_symbol_num_cell(_fft_size, _pilot);
		int _c2_frame_data_cell = _data_symbol_num_cell*_num_data_symbol;
		int _c2_frame_plp_total_data_cell = 0;		

		for (int i = 0; i < _num_plp; i++)
		{	
			int _plp_fec_type  = *(plp_fec_type + i);
			//int _plp_cod  = *(plp_cod + i);
			int _plp_modulation = *(plp_mod + i);	
			int _plp_num_block = *(plp_num_block + i);		

			int _plp_num_cell = c2frame_fecframe_num_cell(_plp_fec_type, _plp_modulation);
			_c2_frame_plp_total_data_cell = _c2_frame_plp_total_data_cell + _plp_num_cell*_plp_num_block;
		}

		if (_c2_frame_plp_total_data_cell < _c2_frame_data_cell)
			return DVBC2_VALID_PARAMETER;
		else 
			return DVBC2_INVALID_NUM_DATA_SYMBOL | DVBC2_INVALID_PLP_MOD | DVBC2_INVALID_PLP_COD | DVBC2_INVALID_PLP_FEC | DVBC2_INVALID_PLP_NUM_BLOCK;
	}			
}

int CDVBC2_MultiplePLP::CalculateTSBitrate()
{

	
	int total_num_ts_packet = 0;
	
	// #ts packets for BB payload
	int bb_payload_bytes_total = 0; 
	for (int i = 0; i < PLP_NUM_BLOCKS_TOTAL; i++)
	{
		int plp_index = bbframe_order[packet_index];
		int len_in_bytes = this->payload_bb[plp_index].GetLenInBits()/8;
		int num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
		total_num_ts_packet += num_ts_packet;

		//bb_payload_bytes_total += len_in_bytes;
	}
//	int num_ts_packet = (((bb_payload_bytes_total + 1) - 1)/144 + 1);
//	total_num_ts_packet += num_ts_packet;

	// #ts packets for L1 payload
	{
	int len_in_bytes = this->payload_l1.GetLenInBits()/8;
	int num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
	total_num_ts_packet += num_ts_packet;
	}

	// #ts packets for timestampe
	{
	int len_in_bytes = this->payload_timestamp.GetLenInBits()/8;
	int num_ts_packet = (((len_in_bytes + 1) - 1)/144 + 1);
	total_num_ts_packet += num_ts_packet;
	}

	int fft_size = dvbc2_input_param.FFT_SIZE;
	int guard_interval = dvbc2_input_param.GUARD_INTERVAL;
	int num_data_symbol = 448;
	int l1_ti_mode = dvbc2_input_param.L1_COD;
	double c2_duration = c2frame_duration(l1_ti_mode, guard_interval, num_data_symbol);	

	int total_bit = total_num_ts_packet*188*8;
	int ts_bitrate  = (int)(total_bit*(1000.0/c2_duration));
	return ts_bitrate;
}


int CDVBC2_MultiplePLP::ValidateBandWidth(int fft_size){
	
	// step 1: compute number of data cell for Cdata
	//         and number of data cell of remaining P2 (after allocating cell to L1-post signal)


	// step 2: compute number of cell need for all PLP 



#if 0
	int pilot_pattern = payload_l1.l1_payload.l1pre._PILOT_PATTERN;
	int max_data_cell = c2frame_max_num_datacell(fft_size, pilot_pattern);



	for (int i = 0; i < payload_l1.l1_payload.l1conf._NUM_PLP; i++)
	{
		l1_payload.l1dyn_curr.plp_list[i]._PLP_ID = _c2mi_multi_plp.list_plp[i].PLP_ID;
		
		// setting PLP_START
		{
		if (i == 0)
		{
			PLP_START = 0;
			//PLP_START = 8100*14;
			l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;
		}
		else {
			//PLP_START = 0;
			
			int PLP_MOD_M = l1_payload.l1conf._plp_list[i-1]._PLP_MOD;
			int PLP_FEC_TYPE_M = l1_payload.l1conf._plp_list[i-1]._PLP_FEC_TYPE;
			int PLP_NUM_BLOCKS_PRE = l1_payload.l1dyn_curr.plp_list[i-1]._PLP_NUM_BLOCKS;
			int N_mod;		

			if ( PLP_MOD_M == 0x000 )		N_mod = 2;
			else if ( PLP_MOD_M == 0x001 )	N_mod = 4;
			else if ( PLP_MOD_M == 0x002 )	N_mod = 6;
			else if ( PLP_MOD_M == 0x003 )	N_mod = 8;

			int N_ldpc = (PLP_FEC_TYPE_M == 0x001 ?  64800 : 16200);

			PLP_START += ((N_ldpc*(PLP_NUM_BLOCKS_PRE)) / N_mod);
			l1_payload.l1dyn_curr.plp_list[i]._PLP_START = PLP_START;

		}
#endif

	return 1;

}


void CDVBC2_MultiplePLP::BBFrameSelection(int num_plp, int *_plp_num_block, int *_bbframe_order)
{
	int i;
	const int max_num_plp = 10;
	const int max_sum_num_block = 1023;
	double p[max_num_plp][max_sum_num_block];
	double mcdf[max_num_plp]; // mcdf: modified cumulative distribution function 

	int sum_plp_num_block = 0;
	for (i = 0; i < num_plp; i++)
	{
		sum_plp_num_block += _plp_num_block[i];
		mcdf[i] = 0;
	}


	// uniform distribution of probability
	for ( i = 0; i < num_plp; i++){
		for (int j = 0; j < sum_plp_num_block; j++)
		{
			p[i][j] = (double)_plp_num_block[i]/sum_plp_num_block;
		}
	}

	// main algorithm 
	for (int j = 0; j < sum_plp_num_block; j++)
	{
		for (int i = 0; i < num_plp; i++)
		{
			mcdf[i] += p[i][j];
		}
		int index_mcdf = Max(num_plp, &mcdf[0]);
		_bbframe_order[j] = index_mcdf;
		// 
		mcdf[index_mcdf] = mcdf[index_mcdf] - 1;
	}

}

int CDVBC2_MultiplePLP::Max(int num_plp, double *_mcdf)
{
	int index_max_cdf = 0;
	double max_cdf = _mcdf[index_max_cdf];

	for (int i = 1; i < num_plp; i++)
	{
		if (max_cdf < _mcdf[i])
		{
			index_max_cdf = i;
			max_cdf = _mcdf[index_max_cdf];
		}
	}

	return index_max_cdf;	
}



// make c2mi packet header 
void CDVBC2_MultiplePLP::MakeC2MIPacketHeader(char packet_type,  //C2MI_PACKET_TYPE_BBF, C2MI_PACKET_TYPE_L1CURRENT, C2MI_PACKET_TYPE_AUX
	unsigned char packet_count, 
	unsigned char superframe_idx)
{		
	
	this->c2mi_header.packet_type = packet_type;
	this->c2mi_header.packet_count = packet_count;
	this->c2mi_header.superframe_idx = superframe_idx;
	this->c2mi_header.rfu = 0x0000;
	

}

// make c2mi packet
int CDVBC2_MultiplePLP::MakeC2MIPacket(char packet_type,  //C2MI_PACKET_TYPE_BBF, C2MI_PACKET_TYPE_L1CURRENT, C2MI_PACKET_TYPE_AUX
	unsigned char plp_index,								
	unsigned char packet_count, 
	unsigned char superframe_idx,
	unsigned char frame_idx,
	unsigned char plp_id,
	unsigned char intl_frame_start)
{

	// make header
	MakeC2MIPacketHeader(packet_type, packet_count, superframe_idx);

	///////////////////
	// make payload
	int rc; 
	switch (packet_type)
	{
	case C2MI_PACKET_TYPE_BBF:			
		rc = this->payload_bb[plp_index].MakeBBPayload(frame_idx, plp_id, intl_frame_start);
		if (rc < 0)
		{
//			printf("assert: can not make bb payload function: %s, line: %d \n", __FUNCTION__, __LINE__);
			return -1;
		}
		c2mi_header.payload_len = this->payload_bb[plp_index].GetLenInBits();
		break;

	case C2MI_PACKET_TYPE_L1CURRENT:

		this->payload_l1.MakeL1Payload(frame_idx);
		c2mi_header.payload_len = this->payload_l1.GetLenInBits();	

		//assert(c2mi_packet.header.payload_len == 536);
		break;
	case C2MI_PACKET_TYPE_TIMESTAMP: 
		this->payload_timestamp.MakeTimestampPayload();
		c2mi_header.payload_len = this->payload_timestamp.GetLenInBits();
		break;

	case C2MI_PACKET_TYPE_AUX:
		break;

	default:
		break;
	}

	return 0;

}




// return length in bits 
// function: make a buffer from MultiplePLP class
int CDVBC2_MultiplePLP::C2MIPacketToBuff(int _plp_id, unsigned char *buff){

	int packet_type;
	int length_in_bits = 0;
	int length_in_bytes;
	
	packet_type = c2mi_header.packet_type;
	
	
	////////////////////////////////////////////////////////////////////////
	// make C2MI header
	{		
		length_in_bits = this->C2MIPacketHeaderToBuff(&buff[0]);
	}		

	switch(packet_type)
	{
	case C2MI_PACKET_TYPE_BBF:
		{

			int num_bits_return = this->payload_bb[_plp_id].BBFramePayloadToBuff(&buff[6]);		
			//ssert(num_bits_return % 8 == 0);
			length_in_bits += num_bits_return; 
		}		
		break;
	case C2MI_PACKET_TYPE_L1CURRENT:
		{			

			int num_bits_return =this->payload_l1.L1PayloadToBuff( &buff[length_in_bits/8]);  
			length_in_bits += num_bits_return; 
		}
		break;

	case C2MI_PACKET_TYPE_TIMESTAMP:
		{
			int num_bits_return = this->payload_timestamp.TimestampPayloadToBuff(&buff[length_in_bits/8]);
			length_in_bits += num_bits_return; 
		}

		break;
	default:
		break;
	}

	// add 32 CRC-32 bits
	length_in_bytes = length_in_bits/8;	
	CUtilities::MakeCRC(&buff[length_in_bits/8], &buff[0], length_in_bytes);
	length_in_bits += 32; 

	return length_in_bits;
}


// return length in bits
int CDVBC2_MultiplePLP::C2MIPacketHeaderToBuff(unsigned char *buff)
{
	
	buff[0] = c2mi_header.packet_type;
	buff[1] = c2mi_header.packet_count;
	buff[2] = c2mi_header.superframe_idx << 4; 
	buff[2] = buff[2] | (c2mi_header.rfu & 0x0F00 >> 8); 
	buff[3] = (c2mi_header.rfu & 0x00FF);
	buff[4] = (c2mi_header.payload_len & 0xFF00) >> 8;
	buff[5] = (c2mi_header.payload_len & 0x00FF);
	
	return 48;
}

