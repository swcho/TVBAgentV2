//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mpeg2ts.h"


// MPEG2TS Statistic 

MPEG2TSStatisticAndFilter::MPEG2TSStatisticAndFilter(){



}

MPEG2TSStatisticAndFilter::~MPEG2TSStatisticAndFilter(){
}

void MPEG2TSStatisticAndFilter::init()
{
	int	i;	
	packet_size = 0; // not initialization 
	bitrate = 0;

	num_filter_pids = -1;

	ts_statistic.num_pmt = 0;
	ts_statistic.num_pat = 0;
	ts_statistic.num_pid = 0; 
	ts_statistic.packet_count = 0;
	
	for (i = 0; i < 10; i++)
	{
		ts_statistic.pmt_table[i].program_number = -1;
		ts_statistic.pmt_table[i].pcr_pid = -1;
	}
	for (i = 0; i < 500; i++)
	{
		ts_statistic.pid_table[i].current_pcr_idx = 0;
		ts_statistic.pid_table[i].pcr_gap_count = 0;
		ts_statistic.pid_table[i].is_pcr = 0; 
		ts_statistic.pid_table[i].packet_count = 0;
		ts_statistic.pid_table[i].num_pcr = 10; // magic number

		for (int j = 0; j < ts_statistic.pid_table[i].num_pcr; j++)
		{
			ts_statistic.pid_table[i].discontinunity[j] = 1;
			ts_statistic.pid_table[i].pcr_value[j] = 0;
			ts_statistic.pid_table[i].packet_index[j] = 0;			
			
		}
	}

	

}



void MPEG2TSStatisticAndFilter::quit(){


}



int	MPEG2TSStatisticAndFilter::EstimateTSPacketSize(char *ts_buff, int ts_buff_len, int credit) 
{
		
	int	is_packet_size_detected;
	int packet_size[4] = {188, 192, 204, 208};

	for (int j = 0; j < 4; j++)
	{
		is_packet_size_detected = 1;
		for (int i = 0; i < credit; i++) {
			if ((i + 1)*packet_size[j] > ts_buff_len)
			{
				is_packet_size_detected = 0;
				break;
			}
			if (ts_buff[i*packet_size[j]] != 0x47) {
				is_packet_size_detected = 0;
				break;
			}
		}
		if (is_packet_size_detected == 1) {
			this->packet_size = packet_size[j];
			return packet_size[j];
		}
	}

	return 0;

}

int MPEG2TSStatisticAndFilter::get_packet_size()
{

	return packet_size;
}



void MPEG2TSStatisticAndFilter::ReadTSHeader( unsigned char *buff, mpeg2ts_header *ts_header )
{
	unsigned char  tmp; 
	tmp = buff[1];
	ts_header->transport_error_indicator = (tmp & 0x80) >> 7;
	ts_header->payload_unit_start_indicator = (tmp & 0x40) >> 6;


	short pidh = buff[1] & 0x1F; 
	short pidl = buff[2];
	ts_header->pid = (pidh << 8) | pidl;
	tmp = buff[3];
	ts_header->adaptation_field_ctl = (tmp >> 4) & (0x03);

	if (ts_header->adaptation_field_ctl == 2 || ts_header->adaptation_field_ctl == 3)
	{
		ts_header->adaptation_field.adaptation_field_length = buff[4];
	}

	tmp = buff[3];
	ts_header->continuity_counter = (tmp & 0x0F);



}

void MPEG2TSStatisticAndFilter::ReadPayloadPAT( unsigned char *buff, mpeg2ts_program_association_section *pat )
{
	short int h;
	short int l;
	h = buff[1] & 0x0F;
	l = buff[2];
	short unsigned int section_length = (h << 8) | l;
	short unsigned int program_info_length = section_length - 5 - 4 /*CRC32*/;
	unsigned char *buff_program_info = buff + 8;

	
	int buff_pro_index = 0;
	pat->num_section = 0;
	while (program_info_length >= buff_pro_index + 4)
	{
		h = buff_program_info[buff_pro_index];
		buff_pro_index++;
		l = buff_program_info[buff_pro_index];
		buff_pro_index++;

		pat->section_info[pat->num_section].program_number = (h << 8 ) | l;		
		h = buff_program_info[buff_pro_index] & 0x1F;
		buff_pro_index++;
		l = buff_program_info[buff_pro_index];
		buff_pro_index++;
		pat->section_info[pat->num_section].network_prog_map_pid = (h << 8) | l;

		pat->num_section++;
	}


}

void MPEG2TSStatisticAndFilter::ReadPayloadPMT( unsigned char *buff, mpeg2ts_program_map_section *pmt )
{
	
	short int h = buff[3];
	short int l = buff[4];
	pmt->program_num = (h << 8) | l;

	h = buff[8] & 0x1F; 
	l = buff[9];
	pmt->PCR_ID = (h << 8) | l;


}


void MPEG2TSStatisticAndFilter::TSStatistic( unsigned char *buff_ts_packet)
{

	mpeg2ts_program_association_section pat;
	mpeg2ts_header ts_header;

	ReadTSHeader(buff_ts_packet, &ts_header);

	
	unsigned char *payload_buff; 
	payload_buff = buff_ts_packet + 4;

	if (IsPIDAddedToPIDTable(ts_header.pid) != 0)
	{
		ts_statistic.pid_table[ts_statistic.num_pid].pid = ts_header.pid;
		ts_statistic.num_pid++;
	}
	// update PID information
	for (int i = 0; i < ts_statistic.num_pid; i++)
	{
		if (ts_statistic.pid_table[i].pid == ts_header.pid)
		{
			if (ts_statistic.pid_table[i].is_pcr == 1) {
				ts_statistic.pid_table[i].pcr_gap_count++;
			}
			ts_statistic.pid_table[i].packet_count++;
			ts_statistic.packet_count++;
			break;
		}
	}
	// update PCR information 
	if (ts_header.adaptation_field_ctl == 2 || ts_header.adaptation_field_ctl == 3)
	{
		mpeg2ts_pcr pcr; 
		ReadTSPCR(payload_buff, &pcr);
		if (pcr.PCR_flag == 1)
		{
			for (int i = 0; i < ts_statistic.num_pid; i++)
			{
				if (ts_header.pid == ts_statistic.pid_table[i].pid)
				{
					int index = ts_statistic.pid_table[i].current_pcr_idx;
					ts_statistic.pid_table[i].is_pcr = 1; 

					
					ts_statistic.pid_table[i].pcr_value[index] = pcr.clock_reference_base*300 + pcr.clock_reference_ext;					
					ts_statistic.pid_table[i].discontinunity[index] = pcr.discontinuity_indicator;
					ts_statistic.pid_table[i].pcr_gap_count_array[index] = ts_statistic.pid_table[i].pcr_gap_count;
					ts_statistic.pid_table[i].pcr_gap_count = 0;
					
					ts_statistic.pid_table[i].packet_index[index] = ts_statistic.packet_count;

					// increment xxxx by one 
					ts_statistic.pid_table[i].current_pcr_idx++;
					ts_statistic.pid_table[i].current_pcr_idx = ts_statistic.pid_table[i].current_pcr_idx%ts_statistic.pid_table[i].num_pcr;

					
					break;
				}
			}
		}
	}

	if (ts_header.payload_unit_start_indicator == 1)
	{
		payload_buff += 1;
	}

	if (ts_header.adaptation_field_ctl == 2 || ts_header.adaptation_field_ctl == 3)
	{
		 payload_buff = payload_buff  + 1 +  ts_header.adaptation_field.adaptation_field_length;
	}

	if (ts_header.pid == 0) 
	{
		char table_id = payload_buff[0];
		if (table_id == 0)
		{
			static int is_PAT_detected = 0;
			if (is_PAT_detected == 0) 
			{
				ReadPayloadPAT(payload_buff, &pat); // PAT 	
				for (int i = 0; i < pat.num_section; i++)
				{
					if (pat.section_info[i].program_number != 0) // not network pid 
					{
						if (IsPIDAddedToPATTable(pat.section_info[i].program_number) == 0)
						{

						}else
						{
							ts_statistic.pat_table[ts_statistic.num_pat].program_number = pat.section_info[i].program_number;
							ts_statistic.pat_table[ts_statistic.num_pat].program_map_id = pat.section_info[i].network_prog_map_pid;
							ts_statistic.num_pat++;
						}
					}
				}		
				is_PAT_detected = 1;
			}
		}
		else //if (table_id == 2)
		{
		//	//ssert0);
			
		}
		
	}else if (PMTTableIsPCRPID(ts_header.pid) == 0)
	{
		// is PCRID		
	} else if (PATTableIsProgramMapPID(ts_header.pid) == 0)
	{
		// get program map and adding 		
		mpeg2ts_program_map_section pmt;
		ReadPayloadPMT(payload_buff, &pmt); // PMT
		if (IsPIDAddedToPMTTable(pmt.program_num) != 0)
		{
			ts_statistic.pmt_table[ts_statistic.num_pmt].pcr_pid = pmt.PCR_ID;
			ts_statistic.pmt_table[ts_statistic.num_pmt].program_number = pmt.program_num;
			ts_statistic.num_pmt++;
		}
	}


}

int MPEG2TSStatisticAndFilter::IsPIDAddedToPATTable( int _pid )
{
	for (int i = 0; i < ts_statistic.num_pat; i++)
	{
		if (ts_statistic.pat_table[i].program_map_id == _pid)
			return 0;
	}
	return -1;

}

int MPEG2TSStatisticAndFilter::IsPIDAddedToPMTTable( int _pid )
{
	for (int i = 0; i < ts_statistic.num_pmt; i++)
	{
		if (ts_statistic.pmt_table[i].pcr_pid == _pid)
			return 0;
	}
	return -1;

}



int MPEG2TSStatisticAndFilter::IsPIDAddedToPIDTable( int _pid )
{
	for (int i = 0; i < ts_statistic.num_pid; i++)
	{
		if (ts_statistic.pid_table[i].pid == _pid)
			return 0;
	}
	return -1;

}


int MPEG2TSStatisticAndFilter::PMTTableIsPCRPID(int _pcr_pid)
{
	for (int i = 0; i < ts_statistic.num_pmt; i++)
	{
		if (ts_statistic.pmt_table[i].pcr_pid == _pcr_pid)
			return 0;
	}
	return -1;

}

int MPEG2TSStatisticAndFilter::PATTableIsProgramMapPID( int _program_map_pid )
{
	for (int i = 0; i < ts_statistic.num_pat; i++)
	{
		if (ts_statistic.pat_table[i].program_map_id == _program_map_pid)
			return 0;
	}
	return -1;

}

void MPEG2TSStatisticAndFilter::ReadTSPCR( unsigned char *buff, mpeg2ts_pcr *pcr )
{
	 
	pcr->adaptation_field_len= buff[0];
	unsigned char tmp; 
	tmp = buff[1];
	pcr->discontinuity_indicator = (tmp & 0x80) >> 8;
	pcr->PCR_flag = (tmp & 0x10) >> 4;

	if (pcr->PCR_flag == 1)
	{
		uint64_t h4, h3, h2, h1, l1, l2, l3, l4;

		h1 = buff[2];
		l1 = buff[3];
		l2 = buff[4];
		l3 = buff[5];
		l4 = buff[6] & 0x80;
		pcr->clock_reference_base = (h1 << 25) |  (l1 << 17) | (l2 << 9) | (l3 << 1) | (l4 >> 7);

		short int h, l;
		h = buff[6] & 0x01;
		l = buff[7];
		pcr->clock_reference_ext = (h << 8) | l;
	}
}

void MPEG2TSStatisticAndFilter::EstimateBitrate()
{
	unsigned char ts_packet_buff[300];	

	for (int i = 0; i < ts_statistic.num_pid; i++)
	{
		int lastest_update_index = (ts_statistic.pid_table[i].current_pcr_idx + ts_statistic.pid_table[i].num_pcr - 1)%ts_statistic.pid_table[i].num_pcr;					
			
		if (ts_statistic.pid_table[i].discontinunity[lastest_update_index] == 0)
		if (ts_statistic.pid_table[i].is_pcr == 1)
		{// if not discontinuty 				
			int previous_latest_update_index = (lastest_update_index + ts_statistic.pid_table[i].num_pcr - 1)% ts_statistic.pid_table[i].num_pcr;
			int64_t diff_pcr_value = ts_statistic.pid_table[i].pcr_value[lastest_update_index] - ts_statistic.pid_table[i].pcr_value[previous_latest_update_index];			
			if (ts_statistic.pid_table[i].pcr_gap_count_array[lastest_update_index] > 0)
			{// eliminate the case that there is only one PCR in TS 
				if (diff_pcr_value > 0)
				{
					bitrate = (double)((ts_statistic.pid_table[i].packet_index[lastest_update_index] - ts_statistic.pid_table[i].packet_index[previous_latest_update_index])*1.0*packet_size*8*27000000)/(diff_pcr_value);
				}
				break;
			}
		}

	}
}


double MPEG2TSStatisticAndFilter::get_bitrate()
{
	return bitrate;
}


void MPEG2TSStatisticAndFilter::set_selected_pids( int _pid_list[], int _num_pid )
{
	for (int i = 0; i < _num_pid; i++)
	{
		filter_pids[i] = _pid_list[i];
	}
	num_filter_pids = _num_pid;
}



double  MPEG2TSStatisticAndFilter::get_selected_bitrate()
{

	// selected bitrate 
	double selected_bitrate = 0;
	for (int i = 0; i < ts_statistic.num_pid; i++)
	{	
		if (isFilterPID(ts_statistic.pid_table[i].pid) == 0)
		{
			double estimated_bitrate = 0;
			estimated_bitrate = (ts_statistic.pid_table[i].packet_count*1.0/ts_statistic.packet_count);
			estimated_bitrate = estimated_bitrate*get_bitrate();
			selected_bitrate += estimated_bitrate;		
		}
	}

#if 0
	char tmp[100];
	sprintf(&tmp[0], "mux-bitrate %f === selected-bitrate %f \n", get_bitrate(), selected_bitrate);
	printf(&tmp[0]);
#endif 

	return selected_bitrate;


}

int MPEG2TSStatisticAndFilter::isFilterPID( int _pid )
{
	for (int i = 0; i < this->num_filter_pids; i++)
	{
		if (filter_pids[i] == _pid)
			return 1;		
	}

	return 0;

}




