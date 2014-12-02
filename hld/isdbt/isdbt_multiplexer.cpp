#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "tlv_type.h"
#include "tlv_threads.h"
#include "input_ts.h"

#include "hldplay.h"
#include "isdbt_multiplexer.h"
//#include "LLDWrapper.h"


static FILE *file_isdbt = NULL;

extern "C"
{

int NUM_TSP[3][4] =
//			1/4    1/8    1/16    1/32
/*mode1*/	{{1280,	1152,	1088,	1056},
/*mode2*/	{2560,	2304,	2176,	2112},
/*mode3*/	{5120,	4608,	4352,	4224}};

int MODE1_LAYERX_NUM_TSP[4][5] =
			// 1/2
/*QPSK*/	{{12,	16,	  18,	20,	  21},
/*QPSK*/	{12,	16,	  18,	20,	  21},
/*16QAM*/	{24,	32,	  36,	40,	  42},
/*64QAM*/	{36,	48,	  54,	60,	  63}};


int Max(int num_plp, double *_mcdf)
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

}



//taken and adapted from libdtv, (c) Rolf Hakenes
// HCRC32 lookup table for polynomial 0x04c11db7
unsigned int ISDBTMultiplexer::crc_table[256] = {
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

unsigned int ISDBTMultiplexer::crc32 (const char *d, int len, unsigned int crc)
{
   register int i;
   const unsigned char *u=(unsigned char*)d; // Saves '& 0xff'

   for (i=0; i<len; i++)
   {
	   crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *(u++))];
   }
   return crc;
}



ISDBTMultiplexer::ISDBTMultiplexer()   // Default constructor
{
	multiplex_option = (void *)&tmcc_param;
	strcpy(&name[0], "muxing, isdb-t 1seg, muxing");

	file_isdbt = NULL;
	//file_isdbt  = fopen("C:/ts/test/tsp_out.ts", "wb");
}


ISDBTMultiplexer::~ISDBTMultiplexer()
{	
	if (file_isdbt != NULL)
		fclose(file_isdbt);			
}

void ISDBTMultiplexer::init()
{
	tsp_buff = (char *)malloc(1024*1024 + 300);
	frame_indicator = 0;
	tsp_index = 0;
	is_iip_transfered = 0;

}

void ISDBTMultiplexer::quit()
{
	free(tsp_buff);
}


int ISDBTMultiplexer::open(CHldPlayback *_play_sys)
{
	play_sys = _play_sys;

	// just support 1-seg 
	tmcc_param.m_Layers[0].m_NumberOfSegments = 1;
	tmcc_param.m_Layers[1].m_NumberOfSegments = 0;
	tmcc_param.m_Layers[2].m_NumberOfSegments = 0;


	int layer_num_tsp[4];	
	int rc =  CalculateParams(&calculated_param);
	if (rc != 0)
		return -1;

	// calculate some information from tmcc .... 
	// note: application have not set this value; 

	tmcc_param.m_Layers[1].m_CodeRate = 2;
	tmcc_param.m_Layers[1].m_TimeInterleave = 2;
	layer_num_tsp[0] = calculated_param.num_TSP - (tmcc_param.m_Layers[0].num_TSP + tmcc_param.m_Layers[1].num_TSP + tmcc_param.m_Layers[2].num_TSP); // NULL + IIP + ... 
	layer_num_tsp[1] = tmcc_param.m_Layers[0].num_TSP;
	layer_num_tsp[2] = tmcc_param.m_Layers[1].num_TSP;
	layer_num_tsp[3] = tmcc_param.m_Layers[2].num_TSP;	
	TSPOrder(4, &layer_num_tsp[0], &tsp_order[0]);

	//play_sys->media_interface[0]->ioctl(2, IO_REQUEST_REMUX_OUT_BITRATE, (int)(tmcc_param.m_Layers[0].bps + tmcc_param.m_Layers[1].bps));

	return 0;	
}



void ISDBTMultiplexer::close(){

//	ts_remux.quit();

}



void ISDBTMultiplexer::TSPOrder(int num_layer, int *layer_num_tsp, int *_tsp_oder)
{
	int i;
	double p[10][252*13];
	double mcdf[10]; // mcdf: modified cumulative distribution function 

	int num_tsp_total = 0;
	for (i = 0; i < num_layer; i++)
	{
		num_tsp_total += layer_num_tsp[i];
		mcdf[i] = 0;
	}

	// uniform distribution of probability
	for ( i = 0; i < num_layer; i++){
		for (int j = 0; j < num_tsp_total; j++)
		{
			p[i][j] = (double)layer_num_tsp[i]/num_tsp_total;
			if (layer_num_tsp[i] == 0)
				p[i][j] = -1; // magic number: ???

		}
	}

	// main algorithm 
	//for (int j = 0; j < num_tsp_total; j++)
	int j = 0;
	while( j< num_tsp_total)
	{
		for (int i = 0; i < num_layer; i++)
		{			
			mcdf[i] += p[i][0];
		}
		int index_mcdf = Max(num_layer, &mcdf[0]);
		if (mcdf[index_mcdf] >= 1)
		{
			_tsp_oder[j] = index_mcdf;
			// 
			mcdf[index_mcdf] = mcdf[index_mcdf] - 1;
			j++;
		}
	}

}



int ISDBTMultiplexer::GetRemuxedTSP(char *buff){

	int  nLayerIdx;
	memset(buff, 0xFF, 204);
	buff[0] = 0x47;
	
	//for (int i = 0; i < OutPars.num_TSP; i++)
	{

	nLayerIdx =tsp_order[tsp_index];
	if (nLayerIdx == 0)
	{
		if (!is_iip_transfered)
		{
			nLayerIdx = TSP_LAYER_IP;
			is_iip_transfered = 1;
		}
		else
			nLayerIdx = TSP_LAYER_NULL;

	}else if (nLayerIdx == 1)
	{
		nLayerIdx = TSP_LAYER_A;
	}else if (nLayerIdx == 2)
	{
		nLayerIdx = TSP_LAYER_B;
	}else if (nLayerIdx == 3)
	{
		nLayerIdx = TSP_LAYER_C;
	}

	if ( nLayerIdx == TSP_LAYER_IP )  
	{
			ISDBT_Information_Packet iip_packet;
			MakeIIPPacket(&iip_packet, 0);
			iip_packet.payload.iip_packet_pointer =  calculated_param.num_TSP - tsp_index -1;
			iip_packet.payload.modulation_configuration_information.tmcc_synchronization_word = frame_indicator;

			ISDBT_GI_Information *gi_information = &iip_packet.payload.modulation_configuration_information.gi_information;
			gi_information->initialization_timing_indicator = 0xF; 
			gi_information->current_mode = tmcc_param.m_Mode;
			gi_information->current_guard_interval = tmcc_param.m_GuardInterval;
			gi_information->next_mode = 0;
			gi_information->next_guard_interval = 0;

			ISDBT_TMCC_INFORMATION *tmcc_pointer = (ISDBT_TMCC_INFORMATION *)&iip_packet.payload.modulation_configuration_information.tmcc_information;			

			IIPPacket2Buff(iip_packet, (char*)buff);
	} else if( nLayerIdx == TSP_LAYER_NULL )  
	{
			//NULL TP
			buff[0] = 0x47;
			buff[1] = 0x1F;
			buff[2] = 0xFF;
			buff[3] = 0x10;
			memset(&buff[4], 0xFF, 188-4);
		
	} 
	else if( nLayerIdx == TSP_LAYER_A /*|| nLayerIdx == TSP_LAYER_B || nLayerIdx == TSP_LAYER_C*/)  
	{

		int rc = play_sys->media_interface[0]->read_one_packet(buff);
		if (rc == -1) // end of file 
			return -1;
	}
	else if( nLayerIdx == TSP_LAYER_B || nLayerIdx == TSP_LAYER_C)  
	{
			//NULL TP
			buff[0] = 0x47;
			buff[1] = 0x1F;
			buff[2] = 0xFF;
			buff[3] = 0x10;
			memset(&buff[4], 0xFF, 188-4);
	}

	// making TMCC information to ts 
	ISDBT_Information isdbt_information; 
	MakeISDBTInformation(&isdbt_information);
	
	if (tsp_index == 0)
		isdbt_information.frame_head_packet_flag = 1;
	else 
		isdbt_information.frame_head_packet_flag = 0;
	isdbt_information.frame_indicator = frame_indicator;
	isdbt_information.layer_indicator = nLayerIdx;
	isdbt_information.tsp_counter = tsp_index;

	ISDBTInformation2Buff(isdbt_information, &buff[188]);

	//set parity
	memset(&buff[196], 0x00, 8);		
	tsp_index = (tsp_index + 1) %calculated_param.num_TSP;
	if (tsp_index == 0)
		is_iip_transfered = 0;

	if (tsp_index == 0)
	{
		frame_indicator = (frame_indicator + 1)%2;	
	}
	
    }
	return 0;
}

void ISDBTMultiplexer::seek(int percentage)
{
}

int ISDBTMultiplexer::ioctl(int message_type, void *param)
{
	switch (message_type)
	{
	case  PLAY_IO_CONTROL_SEEK:
		{
		long *seek_pos = (long *)param;
		break;
		}

	case  ISDBT_IO_SET_LAYER_INFOR:
		{		
		break;
		}
	}
	return 0;
}



void ISDBTMultiplexer::MakeISDBTInformation(ISDBT_Information *isdbt_information)
{
	isdbt_information->tmcc_identifier = 2;
	isdbt_information->reserver = 1;
	isdbt_information->buffer_reset_control_flag = 0;
	isdbt_information->switch_flag_for_emergency_broadcasting = 0;
	isdbt_information->initialization_timing_head_packet_flag = 0; // or 0
	//isdbt_information->frame_head_packet_flag = is_head;
	//isdbt_information->frame_indicator 
	//isdbt_information->layer_indicator 
	isdbt_information->count_down_index = 0xF;
	isdbt_information->ac_data_invalid_flags = 1;
	isdbt_information->ac_data_effective_bytes = 3;
	//isdbt_information->tsp_counter;


}

int ISDBTMultiplexer::ISDBTInformation2Buff(ISDBT_Information isdbt_information, char *buff){

	int num_bits = 0;
	int value; 

	// 2 bit
	value = isdbt_information.tmcc_identifier;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 2);
	// 1
	value = isdbt_information.reserver;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1
	value = isdbt_information.buffer_reset_control_flag;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1
	value = isdbt_information.switch_flag_for_emergency_broadcasting;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1
	value = isdbt_information.initialization_timing_head_packet_flag;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1
	value = isdbt_information.frame_head_packet_flag;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1
	value = isdbt_information.frame_indicator;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 4
	value = isdbt_information.layer_indicator;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 4);
	// 4
	value = isdbt_information.count_down_index;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 4);
	// 1
	value = isdbt_information.ac_data_invalid_flags;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 2
	value = isdbt_information.ac_data_effective_bytes;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 2);
	// 13
	value = isdbt_information.tsp_counter;
	value = value << 3;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 0, 5);

	// 32 bits
	value = 0xFFFFFFFF;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 3, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 2, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 0, 8);
	//num_bits += 32;

	//ssertnum_bits == 8*8);
	return 8*8;
}
void ISDBTMultiplexer::MakeIIPPacket(ISDBT_Information_Packet *iip, int countinuity_counter)
{

	iip->header.sync = 0x47;
	iip->header.transport_error_indicator = 0;
	iip->header.payload_unit_start_indicator = 1;
	iip->header.transport_priority = 0;
	iip->header.pid = 0x1FFF; // PID of terretrial digital television broadcasting 
	iip->header.transport_scrambling_ctrl = 0; 
	iip->header.adaptation_field = 1;
	iip->header.continuity_counter = countinuity_counter;

	// set later
	//iip->payload.iip_packet_pointer = 
	iip->payload.modulation_configuration_information.tmcc_synchronization_word = 0; 

	iip->payload.modulation_configuration_information.ac_data_effective_position = 0x1; // AC data is multiplexed in dummy 
	iip->payload.modulation_configuration_information.reserved = 0X3;  // 2 bit
	// set later
	//iip->payload.modulation_configuration_information.gi_information

	ISDBT_TMCC_SIGNAL tmcc_signal;
	tmcc_signal.ref = 0; //  not clear ??? 
	tmcc_signal.synchronization_signal = 1; // should be assigned to frame number 
	tmcc_signal.system_type = ISDBT_SEGMENT_SYNCHRONOUS; // ?? for which segment??? 
	tmcc_signal.tmcc_information.system_indentification = ISDBT_SYSTEM_BASE; //??
	tmcc_signal.tmcc_information.transmission_parameter_switching = ISDBT_TPS_NORMAL; // ??
	tmcc_signal.tmcc_information.emergency_alarm = ISDBT_NO_STARTUP_CONTROL; //??

	// huy: ??? 
	tmcc_signal.tmcc_information.current_partial_flag = tmcc_param.m_PartialRecev;//
	
	// layer A
	tmcc_signal.tmcc_information.current_transmission_layerA.modulation = tmcc_param.m_Layers[0].m_modulation;// ISDBT_MOD_QPSK; 
	tmcc_signal.tmcc_information.current_transmission_layerA.code_rate = tmcc_param.m_Layers[0].m_CodeRate;//ISDBT_RATE_1_2;
	tmcc_signal.tmcc_information.current_transmission_layerA.interleaving_length = tmcc_param.m_Layers[0].m_TimeInterleave;// ISDBT_INTERTIME_MODE3_2;
	tmcc_signal.tmcc_information.current_transmission_layerA.num_segment = ISDBT_NUM_SEGMENT_1; // 1 segment 

	// layer B
	tmcc_signal.tmcc_information.current_transmission_layerB.modulation = tmcc_param.m_Layers[1].m_modulation; 
	tmcc_signal.tmcc_information.current_transmission_layerB.code_rate = tmcc_param.m_Layers[1].m_CodeRate;
	tmcc_signal.tmcc_information.current_transmission_layerB.interleaving_length = tmcc_param.m_Layers[1].m_TimeInterleave;
	tmcc_signal.tmcc_information.current_transmission_layerB.num_segment = ISDBT_NUM_SEGMENT_12; // 12 segment 

	// layer C
	tmcc_signal.tmcc_information.current_transmission_layerC.modulation = ISDBT_MOD_UNUSE; 
	tmcc_signal.tmcc_information.current_transmission_layerC.code_rate = ISDBT_RATE_UNUSE;
	tmcc_signal.tmcc_information.current_transmission_layerC.interleaving_length = ISDBT_INTERTIME_UNUSE;
	tmcc_signal.tmcc_information.current_transmission_layerC.num_segment = ISDBT_NUM_SEGMENT_UNUSED; 


	tmcc_signal.tmcc_information.next_partial_flag = ISDBT_NO_PARTIAL_FLAG; // must be yes 
	// layer A
	tmcc_signal.tmcc_information.next_transmission_layerA.modulation = ISDBT_MOD_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerA.code_rate = ISDBT_RATE_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerA.interleaving_length = ISDBT_INTERTIME_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerA.num_segment = ISDBT_NUM_SEGMENT_UNUSED;

	// layer B
	tmcc_signal.tmcc_information.next_transmission_layerB.modulation = ISDBT_MOD_UNUSE; 
	tmcc_signal.tmcc_information.next_transmission_layerB.code_rate = ISDBT_RATE_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerB.interleaving_length = ISDBT_INTERTIME_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerB.num_segment = ISDBT_NUM_SEGMENT_UNUSED; // 12 segment 

	// layer C
	tmcc_signal.tmcc_information.next_transmission_layerC.modulation = ISDBT_MOD_UNUSE; 
	tmcc_signal.tmcc_information.next_transmission_layerC.code_rate = ISDBT_RATE_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerC.interleaving_length = ISDBT_INTERTIME_UNUSE;
	tmcc_signal.tmcc_information.next_transmission_layerC.num_segment = ISDBT_NUM_SEGMENT_UNUSED; 
	tmcc_signal.tmcc_information.phase_shift = 7; 
	tmcc_signal.tmcc_information.reserve = 0xFFF;


	iip->payload.modulation_configuration_information.tmcc_information = tmcc_signal.tmcc_information;
	iip->payload.modulation_configuration_information.reserved_future_use = 0x3FF; // 10 bit
	iip->payload.modulation_configuration_information.CRC_32 = 0; // recompute when make buffer 

	iip->payload.iip_branch_number = 0;  // first IIP packet 
	iip->payload.last_iip_branch_number = 0; // network synchronization information is not overlapped 
	iip->payload.network_synchronization_information_length = 0; // unuse network information 

	iip->payload.network_synchronization_information = NULL; // will be filled when buffering 
	iip->payload.stuffing_byte = NULL;  // will be filled when buffering 
}

// return number of bytes in buff
int ISDBTMultiplexer::IIPPacket2Buff(ISDBT_Information_Packet iip, char *buff)
{
	int num_bits = 0;
	int value; 

	value = iip.header.sync;
	// 8 bit
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 8);
	// 1 bit
	value = iip.header.transport_error_indicator;
	value = value << 7;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);

	// 1 bit
	value = iip.header.payload_unit_start_indicator;
	value = value << 7;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 1 bit
	value = iip.header.transport_priority;
	value = value << 7;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 1);
	// 13 bits ( 2 bytes)
	value = iip.header.pid;
	value = value << 3;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 0, 5);
	// 2 bits
	value = iip.header.transport_scrambling_ctrl;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 2);

	// 2 bits
	value = iip.header.adaptation_field;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 2);
	// 4 bits
	value = iip.header.continuity_counter;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 4);

	// 16 bits
	value = iip.payload.iip_packet_pointer;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 0, 8);

	//////////////////////////////////////////////////////////////////////////
	// modulation control configuration

	// 1 bit
	value = iip.payload.modulation_configuration_information.tmcc_synchronization_word;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 1);
	//1 bit
	value = iip.payload.modulation_configuration_information.ac_data_effective_position;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 1);
	//2 bit
	value = iip.payload.modulation_configuration_information.reserved;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);

	//4 bit
	value = iip.payload.modulation_configuration_information.gi_information.initialization_timing_indicator;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);
	//2 bit
	value = iip.payload.modulation_configuration_information.gi_information.current_mode;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);
	//2 bit
	value = iip.payload.modulation_configuration_information.gi_information.current_guard_interval;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);
	//2 bit
	value = iip.payload.modulation_configuration_information.gi_information.next_mode;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);
	//2 bit
	value = iip.payload.modulation_configuration_information.gi_information.next_guard_interval;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);
	// tmcc information 
	//2 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.system_indentification;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.transmission_parameter_switching;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);
	//1 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.emergency_alarm;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 1);
	//1 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_partial_flag;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 1);

	// tmcc current layerA
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerA.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerA.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerA.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerA.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// tmcc current layerB
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerB.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerB.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerB.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerB.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// tmcc current layerC
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerC.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerC.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerC.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.current_transmission_layerC.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// 1 bit 
	value = iip.payload.modulation_configuration_information.tmcc_information.next_partial_flag;
	value = value << 7;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 1);

	// tmcc next layer A
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerA.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerA.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerA.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerA.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// tmcc next layer B
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerB.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerB.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerB.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerB.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// tmcc next layer C
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerC.modulation;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerC.code_rate;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerC.interleaving_length;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	//4 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.next_transmission_layerC.num_segment;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);
	// 3 bit
	value = iip.payload.modulation_configuration_information.tmcc_information.phase_shift;
	value = value << 5;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 3);
	// 12 bits
	value = iip.payload.modulation_configuration_information.tmcc_information.reserve;
	value = value << 4;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 4);

	// 10 bits
	value = iip.payload.modulation_configuration_information.reserved_future_use;
	value = value << 6;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value , 2);

	// 32 bits
	int index = num_bits/8 - 16 ;
	iip.payload.modulation_configuration_information.CRC_32 = crc32(&buff[index], 16, 0xFFFFFFFF);
	//value = iip.payload.modulation_configuration_information.CRC_32;	
	value = 0;
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 3, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 2, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 1, 8);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value + 0, 8);

	// 8 bits
	value = iip.payload.iip_branch_number;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 8);
	// 8 bits
	value = iip.payload.last_iip_branch_number;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 8);
	// 8 bits
	value = iip.payload.network_synchronization_information_length;	
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&value, 8);
	// 159 stuffing bytes
	char stuffing_bytes[160];
	memset(&stuffing_bytes[0], 0xFF, 159);
	num_bits = AddBitToBuffer(buff, num_bits, (unsigned char*)&stuffing_bytes[0], 159*8);

	//ssertnum_bits == 188*8);
	return 188;
}


// shift left 1 bit from second bytes
void ISDBTMultiplexer::BitConcat(char *des, int syncd, int num_bits){

	
	int length_in_bytes = (num_bits - 1)/8 + 1;	
	char *shift;
	char first_byte = des[0];
	des[0] = 0x00;
	for (int i = 0; i < syncd; i++)
	{
		shift = &des[0];
		while (shift < &des[0] +(length_in_bytes + 1)) {
			*shift = (*(shift)&0x7F)<<1 | (*(shift+1)&0x80)>>7;
			shift++;	
		}
		//printf("first bytes %X \n", des[0]);
	}

	first_byte = ((first_byte >> syncd) << syncd); // cleaning the last syncd bits
	// clean the first (8-syncd) bytes of des[0];
//	des[0] = ((des[0] << (8-syncd)) >> (8-syncd));
	des[0] = des[0] | first_byte;
}

// add bits to a buffer 
// return total numbe of bits in des
int ISDBTMultiplexer::AddBitToBuffer(char *des, int des_length_in_bit, unsigned char *src, int src_length_in_bit){	

	if (src_length_in_bit > 0)
	{
	
		int des_index_in_byte= (des_length_in_bit - 1)/8 + 1;
		// note: when des_length_in_bit = 0, des_index_in_byte = 1

		int des_length_remainer = des_length_in_bit%8;
		int src_length_in_byte = (src_length_in_bit - 1)/8 + 1;
		int syncd; 

		//strncpy((char*)&des[des_index_in_byte], (const char *)src, src_length_in_byte);
		for (int i = 0; i < src_length_in_byte; i++)
			des[des_index_in_byte + i] = src[i];

		if (des_length_remainer == 0)
		{		
			if (des_length_in_bit == 0)
			{ // special case			
				syncd = 8;
			}else{
				syncd = 0;			
			}
			
		}else{
			syncd = 8 - des_length_remainer;
		}

		BitConcat(&des[des_index_in_byte - 1], syncd, src_length_in_bit);
	}

	return des_length_in_bit + src_length_in_bit;
}




int ISDBTMultiplexer::CalculateParams(CalculatedParams* pOutPars)
{
	int i;
	LayerPars*  pLayer;
	int guard_interval_index = 0; 
	int mode_index = 0; 
	
	
	if ( tmcc_param.m_GuardInterval == ISDBT_GUARD_1_4)
		guard_interval_index = 0;
	else if ( tmcc_param.m_GuardInterval == ISDBT_GUARD_1_8)
		guard_interval_index = 1;
	else if ( tmcc_param.m_GuardInterval == ISDBT_GUARD_1_16)
		guard_interval_index = 2;
	else if ( tmcc_param.m_GuardInterval == ISDBT_GUARD_1_32)
		guard_interval_index = 3;
	else 
		return -1;

	if (tmcc_param.m_Mode == 1)
		mode_index = 0;
	else if (tmcc_param.m_Mode == 2)
		mode_index = 1;
	else if (tmcc_param.m_Mode == 3)
		mode_index = 2;
	else 
		return -1;

	pOutPars->num_TSP = NUM_TSP[mode_index][guard_interval_index];
	

	for ( i=0; i<3; i++) 
	{
		int modulation_index;
		int code_rate_index;
		pLayer = &(tmcc_param.m_Layers[i]);

		tmcc_param.m_Layers[i].num_TSP = 0;
		if (pLayer->m_NumberOfSegments != 0) 
		{
			if ( pLayer->m_modulation  == ISDBT_MOD_DQPSK)
				modulation_index = 0;
			else if ( pLayer->m_modulation  == ISDBT_MOD_QPSK)
				modulation_index = 1;
			else if ( pLayer->m_modulation  == ISDBT_MOD_16QAM)
				modulation_index = 2;
			else if ( pLayer->m_modulation  == ISDBT_MOD_64QAM)
				modulation_index = 3;
			else 
			{
//				printf("assert modulation is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
				printf("assign default: ISDBT_MOD_64QAM at layer: %d \n", i);
				pLayer->m_modulation = ISDBT_MOD_64QAM;		
				modulation_index = 3;
			}

			if (pLayer->m_CodeRate == ISDBT_RATE_1_2)
				code_rate_index = 0;
			else if (pLayer->m_CodeRate == ISDBT_RATE_2_3)
				code_rate_index = 1;
			else if (pLayer->m_CodeRate == ISDBT_RATE_3_4)
				code_rate_index = 2;
			else if (pLayer->m_CodeRate == ISDBT_RATE_5_6)
				code_rate_index = 3;
			else if (pLayer->m_CodeRate == ISDBT_RATE_7_8)
				code_rate_index = 4;
			else {
//				printf("assert code rate is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
				printf("assign default: ISDBT_RATE_7_8 at layer: %d \n", i);
				pLayer->m_CodeRate = ISDBT_RATE_7_8;
				code_rate_index = 4;
			}
			tmcc_param.m_Layers[i].num_TSP = MODE1_LAYERX_NUM_TSP[modulation_index][code_rate_index];
			tmcc_param.m_Layers[i].num_TSP *= (int)pow((double)2, tmcc_param.m_Mode-1);
			tmcc_param.m_Layers[i].num_TSP *= pLayer->m_NumberOfSegments;

		}
	}

	double frame_length=0.;//msec.
	switch ( tmcc_param.m_Mode )
	{
	case 1:
		if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 53.0145;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 54.621;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 57.834;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 64.26;
		}
		else 
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 64.26;
		}
		break;
	case 2:
		if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 106.029;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 109.242;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 115.668;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 128.52;
		} else
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 128.52;
		}
		break;
	case 3:
		if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 212.058;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 218.484;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 231.336;
		}
		else if (tmcc_param.m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 257.04;
		} else
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 257.04;
		}
		break;
	default :
		break;
	}
	pOutPars->m_FrameLength = frame_length;

	
	for (i = 0; i<3; i++) 
	{
		pLayer = &(tmcc_param.m_Layers[i]);
		tmcc_param.m_Layers[i].bps = (int)(tmcc_param.m_Layers[i].num_TSP * 188 * 8 * (1/frame_length) * 1000);
	}
	

	// Symbol Delay
	pOutPars->m_DelayOfSymbol = 0;

	return 0;
}




int ISDBTMultiplexer::CalculateParams(CalculatedParams* pOutPars, ISDBT_PARAM *tmcc_param)
{
	int i;
	LayerPars*  pLayer;
	int guard_interval_index = 0; 
	int mode_index = 0; 
	
	
	if ( tmcc_param->m_GuardInterval == ISDBT_GUARD_1_4)
		guard_interval_index = 0;
	else if ( tmcc_param->m_GuardInterval == ISDBT_GUARD_1_8)
		guard_interval_index = 1;
	else if ( tmcc_param->m_GuardInterval == ISDBT_GUARD_1_16)
		guard_interval_index = 2;
	else if ( tmcc_param->m_GuardInterval == ISDBT_GUARD_1_32)
		guard_interval_index = 3;
	else 
		return -1;

	if (tmcc_param->m_Mode == 1)
		mode_index = 0;
	else if (tmcc_param->m_Mode == 2)
		mode_index = 1;
	else if (tmcc_param->m_Mode == 3)
		mode_index = 2;
	else 
		return -1;

	pOutPars->num_TSP = NUM_TSP[mode_index][guard_interval_index];
	

	for ( i=0; i<3; i++) 
	{
		int modulation_index;
		int code_rate_index;
		pLayer = &(tmcc_param->m_Layers[i]);

		tmcc_param->m_Layers[i].num_TSP = 0;
		if (pLayer->m_NumberOfSegments != 0) 
		{
			if ( pLayer->m_modulation  == ISDBT_MOD_DQPSK)
				modulation_index = 0;
			else if ( pLayer->m_modulation  == ISDBT_MOD_QPSK)
				modulation_index = 1;
			else if ( pLayer->m_modulation  == ISDBT_MOD_16QAM)
				modulation_index = 2;
			else if ( pLayer->m_modulation  == ISDBT_MOD_64QAM)
				modulation_index = 3;
			else 
			{
//				printf("assert modulation is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
				printf("assign default: ISDBT_MOD_64QAM at layer: %d \n", i);
				pLayer->m_modulation = ISDBT_MOD_64QAM;		
				modulation_index = 3;
			}

			if (pLayer->m_CodeRate == ISDBT_RATE_1_2)
				code_rate_index = 0;
			else if (pLayer->m_CodeRate == ISDBT_RATE_2_3)
				code_rate_index = 1;
			else if (pLayer->m_CodeRate == ISDBT_RATE_3_4)
				code_rate_index = 2;
			else if (pLayer->m_CodeRate == ISDBT_RATE_5_6)
				code_rate_index = 3;
			else if (pLayer->m_CodeRate == ISDBT_RATE_7_8)
				code_rate_index = 4;
			else {
//				printf("assert code rate is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
				printf("assign default: ISDBT_RATE_7_8 at layer: %d \n", i);
				pLayer->m_CodeRate = ISDBT_RATE_7_8;
				code_rate_index = 4;
			}
			tmcc_param->m_Layers[i].num_TSP = MODE1_LAYERX_NUM_TSP[modulation_index][code_rate_index];
			tmcc_param->m_Layers[i].num_TSP *= (int)pow((double)2, tmcc_param->m_Mode-1);
			tmcc_param->m_Layers[i].num_TSP *= pLayer->m_NumberOfSegments;

		}
	}

	double frame_length=0.;//msec.
	switch ( tmcc_param->m_Mode )
	{
	case 1:
		if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 53.0145;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 54.621;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 57.834;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 64.26;
		}
		else 
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 64.26;
		}
		break;
	case 2:
		if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 106.029;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 109.242;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 115.668;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 128.52;
		} else
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 128.52;
		}
		break;
	case 3:
		if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_32)
		{
			frame_length = 212.058;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_16)
		{
			frame_length = 218.484;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_8)
		{
			frame_length = 231.336;
		}
		else if (tmcc_param->m_GuardInterval == ISDBT_GUARD_1_4)
		{
			frame_length = 257.04;
		} else
		{
//			printf("assert guard interval is not initialized  file: %s, function: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__);
			printf("assign default: ISDBT_GUARD_1_4 \n");
			pLayer->m_CodeRate = ISDBT_GUARD_1_4;
			frame_length = 257.04;
		}
		break;
	default :
		break;
	}
	pOutPars->m_FrameLength = frame_length;

	
	for (i = 0; i<3; i++) 
	{
		pLayer = &(tmcc_param->m_Layers[i]);
		tmcc_param->m_Layers[i].bps = (int)(tmcc_param->m_Layers[i].num_TSP * 188 * 8 * (1/frame_length) * 1000);
	}
	

	// Symbol Delay
	pOutPars->m_DelayOfSymbol = 0;
	
	return 0;
}




int ISDBTMultiplexer::CalculateBitrate(){
	
	int num_TSP = this->calculated_param.num_TSP;
	double frame_length_in_sec = calculated_param.m_FrameLength*1.0/1000;

	return (int)(204*8*num_TSP*1.0/frame_length_in_sec);
}


int ISDBTMultiplexer::produce_item( char *buff_tmp_tmp, int max_size )
{
	
	int buff_size = 0;
	

	while(buff_size + 204 < max_size  /*1MB*/)
	{
		// size of a TSP must be 204 bytes
		int rc = GetRemuxedTSP(&tsp_buff[buff_size]);
#ifdef WIN32
		if (file_isdbt != NULL)
		{
			fwrite(&tsp_buff[buff_size], 1, 204, file_isdbt);
			flushall();
		}
#endif
		if (rc != 0)
		{				
			return -1;
		}
		buff_size += 204;

		
	}
	

	////////////////////////////////////////////////////////////////////
	// add to multiplexed buffer 
	tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
	play_sys->cir_multiplexed_data_buffer->BufferNItem((char *) &tsp_buff[0], buff_size);
	//tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);		

	//tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);			
	int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
	tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
	if (b_bytes >= 1024*1024)
		tlv_cond_signal(&play_sys->cv_multiplexed_data_out);
	return buff_size;

}



// -----------------------------------

ISDBT_Reader::ISDBT_Reader()
{
	strcpy(&name[0], "isdb-t, 1seg, reader");
}

ISDBT_Reader::~ISDBT_Reader()
{

}
void ISDBT_Reader::init()
{
		ts_buff = (char *)malloc(1024*1024 + 300);
}

void ISDBT_Reader::quit()
{
	free(ts_buff);
}

int ISDBT_Reader::open(CHldPlayback *_play_sys)
{

	play_sys = _play_sys;
	media_source = play_sys->media_interface[0];
	return 0;
	
}



void ISDBT_Reader::close(){
}


int ISDBT_Reader::produce_item( char *buff_tmp, int size ) 
{
// step: produce item 
	int buff_size = 0; 
	while(buff_size + 512*1024 <= size /*1MB*/)
	{
		int rc = media_source->read_n_byte(&ts_buff[buff_size], 512*1024);
		if (rc == INPUT_ERROR_EOF || rc == INPUT_ERROR_SOCKET_CLOSE)
		{
			return -1; 
		}
		else {
			buff_size += rc;
		}

	}

// step: add to buffer  
	////////////////////////////////////////////////////////////////////
	// add to multiplexed buffer 
	tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);
	play_sys->cir_multiplexed_data_buffer->BufferNItem((char *)&ts_buff[0], buff_size);
	//tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);		

// step: notify   
	//tlv_mutex_lock(&play_sys->mutex_cir_multiplexed_data);			
	int b_bytes = play_sys->cir_multiplexed_data_buffer->NumBufferedItem()*play_sys->cir_multiplexed_data_buffer->ItemSize();
	tlv_mutex_unlock(&play_sys->mutex_cir_multiplexed_data);
	if (b_bytes >= 1024*1024)
		tlv_cond_signal(&play_sys->cv_multiplexed_data_out);
	
	return buff_size;

}

void ISDBT_Reader::seek(int percentage)
{
}

int ISDBT_Reader::ioctl( int message_type, void *param )
{
	switch (message_type)
	{
		case PLAY_IO_CONTROL_SEEK:
			long *seek_pos = (long *)param;
			break;
	}
	return 0;
}



