#include "tlv_type.h"
#include "tlv_threads.h"

#include "hldplay.h"

#include "media_interface.h"
#include "input_ip.h"
#include "input_file.h"
#include "input_buffer.h"
#include "input_ts.h"

#include "output.h"
#include "isdbt_multiplexer.h"
#include "dvbt2_multiplexer.h"
#include "log.h"

#include "hldmgr.h"


DVBT2_PARAM dvbt2_param; 
DVBC2_PARAM dvbc2_param; 
ISDBT_PARAM tmcc_param;
ISDBT_LAYER_INFORMATION isdbt_tmcc_layer_info;
char file_name[256];

// this function is to interface with hldmgr.cpp 
extern "C" 
{
	int CHldPlayConcreteObserver::Update(int state)
	{
		switch (state)
		{
			case PLAYBACK_USER_STATE_START_STREAMING:		
				// un-use in linux version 
				//leep(100);
				//pLLD->_FIf->play_buffer_status =  BUFF_STS_BUFFERING;
				//pLLD->_FIf->user_play_buffer_status = BUFF_STS_BUFFERING;
				// it is a kind of trick because of issue which is communication b/t application thread and hld thread 
				// this communication is not synchronized 				
				// un-use in linux version 

				tlv_sleep(100);
				pLLD->_FIf->play_buffer_status =  BUFF_STS_PLAY_STREAM_OUT;
				//pLLD->_FIf->user_play_buffer_status = BUFF_STS_PLAY_STREAM_OUT;

				break;
			case PLAYBACK_USER_STATE_STOP_STREAMING:
				pLLD->_FIf->TL_gNumLoop++;
				pLLD->_FIf->play_buffer_status = BUFF_STS_COMPLETED_A_STREAM_OUT_NOW;
				//pLLD->_FIf->user_play_buffer_status =  BUFF_STS_PLAY_STREAM_OUT;	
				// it is a kind of trick because of issue which is communication b/t application thread and hld thread 
				// this communication is not synchronized 
				tlv_sleep(500);
				break;

			case PLAYBACK_LOG_MESSAGE: 
				//OutputDebugString("Print log message here \n");
				break;
		}

		return 0;
	}

	void playback_receive_request(CHld* plld, int message_type, void *param)
	{	

		int state = plld->playback.playback_state;

		int media_source_type;
		int multiplexing_type;
		int output_type;



		// for debuging 
#if 1 

		static int old_message_type = -1;
		if (old_message_type != message_type)
		{
			old_message_type = message_type;
			//char debug_str[100];
			//sprintf(&debug_str[0], " [playback] received state  %d =======\n", message_type);
			//OutputDebugString(&debug_str[0]);
		}
#endif 

		switch (state)
		{
			case PLAYBACK_STATE_NONE:
				if (message_type == PLAY_MESSAGE_INIT)
				{

					if (plld->_SysSta->IPUsing() == 1)
					{
						media_source_type = PLAY_MEDIA_SOURCE_IP_BUFFER;
					}
					//2012/7/6 DVB-T2 ASI
					else if(plld->_SysSta->IsState_DvbT2LoopThru() == 1 || plld->_SysSta->IsState_DvbC2LoopThru() == 1)
					{
						media_source_type = PLAY_MEDIA_SOURCE_ASI_310M;
					}
					else
					{
						media_source_type = PLAY_MEDIA_SOURCE_FILE;
					}

					if (plld->_SysSta->IsModTyp_IsdbT_1())
					{
						if (plld->_FIf->TmccUsing())
							multiplexing_type =  PLAY_MULTIPLEXER_ISDBT_MUX;
						else if (!plld->_FIf->TmccUsing())
							multiplexing_type = PLAY_MULTIPLEXER_ISDBT_NO_MUX;
						else 
						{
							// observer thread 
							tlv_message_t message;
							message.id = PLAYBACK_LOG_MESSAGE;
							message.param = NULL;
							tlv_mq_send(&plld->playback.thread_observer, &message);
						}
					}
					else if(plld->_SysSta->IsState_DvbT2LoopThru() == 1)
					{
						multiplexing_type = PLAY_DVBT2_MUX;
					}
					else if(plld->_SysSta->IsState_DvbC2LoopThru() == 1)
					{
						multiplexing_type = PLAY_DVBC2_MUX;
					}
					else if (plld->_SysSta->IsModTyp_DvbT2() && plld->_FIf->IsT2miFile() == 0)
					{
						multiplexing_type = PLAY_DVBT2_MUX;
					}else if (plld->_SysSta->IsModTyp_DvbC2() && plld->_FIf->IsC2miFile() == 0)
				    {
						multiplexing_type = PLAY_DVBC2_MUX;
					}else if(plld->_SysSta->IsModTyp_DvbC2())
					{
						multiplexing_type = PLAY_DVBC2_MUXED_FILE;
					} else if (1) 
						multiplexing_type = PLAY_DVBT2_MUXED_FILE;

					output_type = PLAY_OUTPUT_TVBBOARD;
					
					// parameter setting 
					if (plld->_SysSta->IsModTyp_IsdbT_1())
					{
						tmcc_param = plld->tmcc_param;
						isdbt_tmcc_layer_info = plld->isdbt_tmcc_layer_info;
						strcpy(&file_name[0], &plld->_FIf->PlayParm.AP_lst.szfn[0]);

					}else if ((plld->_SysSta->IsModTyp_DvbT2() && plld->_FIf->IsT2miFile() == 0) || plld->_SysSta->IsState_DvbT2LoopThru() == 1 || plld->_SysSta->IPUsing() == 1)
					{

						int BW;
						if ( plld->TL_DVB_T2_BW  == 0 )			BW = 2;//6MHz
						else if ( plld->TL_DVB_T2_BW  == 1 )	BW = 3;//7MHz
						else if ( plld->TL_DVB_T2_BW  == 2 )	BW = 4;//8MHz
						else if ( plld->TL_DVB_T2_BW  == 3 )	BW = 1;//5MHz
						else									BW = 0;//1.7MHz

						/*
						int FFT_SIZE;
						if ( plld->FFT_SIZE == 0 )		FFT_SIZE = 3;//1K
						else if ( plld->FFT_SIZE == 1 ) FFT_SIZE = 0;//2K
						else if ( plld->FFT_SIZE == 2 ) FFT_SIZE = 2;//4K
						else if (plld->FFT_SIZE == 4)
							FFT_SIZE = DVBT2_FFT_SIZE_16K;
						else if (plld->FFT_SIZE == 5)
							FFT_SIZE = DVBT2_FFT_SIZE_32K;
						else
						{
							if ( plld->GUARD_INTERVAL <= 3 )
							{
								FFT_SIZE = 1;
							}
							else
							{
								FFT_SIZE = 6;
							}

						}
						*/



						plld->dvbt2_param.BW = BW;
						plld->dvbt2_param.BW_EXT = plld->BW_EXT;

						plld->dvbt2_param.FFT_SIZE = plld->FFT_SIZE;
						plld->dvbt2_param.GUARD_INTERVAL = plld->GUARD_INTERVAL;

						plld->dvbt2_param.L1_MOD = plld->L1_MOD;
						plld->dvbt2_param.Pilot = plld->PILOT_PATTERN;
						int PAPR = 0;
						plld->dvbt2_param.PAPR = PAPR;
						plld->dvbt2_param.S1 = plld->S1;
						plld->dvbt2_param.L1_REPETITION = 0;
						plld->dvbt2_param.NETWORK_ID = plld->NETWORK_ID;
						plld->dvbt2_param.T2_ID = plld->T2_SYSTEM_ID;
						plld->dvbt2_param.Cell_ID = plld->CELL_ID;
						plld->dvbt2_param.NUM_T2_FRAME = plld->NUM_T2_FRAMES;
						plld->dvbt2_param.NUM_DATA_SYMBOLS = plld->NUM_DATA_SYMBOLS;
						plld->dvbt2_param.FREQUENCY = plld->FREQUENCY;			
						plld->dvbt2_param.num_plp = plld->PLP_COUNT;

						for (int i = 0; i < plld->dvbt2_param.num_plp; i++)
						{	
							strcpy(&plld->dvbt2_param.list_plp[i].file_path[0], plld->PLP_TS_M[i]);
							plld->dvbt2_param.list_plp[i].PLP_ID = plld->PLP_ID_M[i];		
							plld->dvbt2_param.list_plp[i].PLP_COD = plld->PLP_COD_M[i];
							plld->dvbt2_param.list_plp[i].PLP_MOD = plld->PLP_MOD_M[i];
							plld->dvbt2_param.list_plp[i].PLP_FEC = plld->PLP_FEC_TYPE_M[i];
							plld->dvbt2_param.list_plp[i].PLP_HEM = plld->PLP_MODE_M[i];
							plld->dvbt2_param.list_plp[i].UP_BITRATE = plld->PLP_TS_BITRATE_M[i];
							int TIME_IL_TYPE = 0; // T2 frame and interleaving frame is 1:1 corresponding 
							plld->dvbt2_param.list_plp[i].PLP_TIME_IL_TYPE = TIME_IL_TYPE;
							plld->dvbt2_param.list_plp[i].PLP_TIME_IL_LENGTH = plld->PLP_TIME_IL_LENGTH_M[i];

							plld->dvbt2_param.list_plp[i].PLP_NUM_BLOCKS = plld->PLP_NUM_BLOCKS_M[i];
							plld->dvbt2_param.list_plp[i].PLP_ROTATION = plld->PLP_ROTATION_M[i];
						}

						if (plld->_SysSta->IPUsing() == 1 || plld->_SysSta->IsState_DvbT2LoopThru() == 1)	//2012/7/6 DVB-T2 ASI
						{
							plld->dvbt2_param.auto_searching_data_symbol_num_block = 1;
							int ts_input_bitrate[8];
							ts_input_bitrate[0] = plld->TL_CurrentBitrate;
							int rtcd = dvbt2_SearchingParamater(0, 1, plld->L1_MOD,
								BW,
								plld->BW_EXT,
								plld->FFT_SIZE,
								plld->GUARD_INTERVAL,
								plld->PILOT_PATTERN,
								plld->PLP_COUNT,
								&plld->PLP_FEC_TYPE_M[0],
								&plld->PLP_COD_M[0],
								&plld->PLP_MOD_M[0],
								ts_input_bitrate,
								&plld->NUM_DATA_SYMBOLS, 
								&plld->PLP_NUM_BLOCKS_M[0]);

							plld->dvbt2_param.NUM_DATA_SYMBOLS = plld->NUM_DATA_SYMBOLS;

							for (int i = 0; i < plld->dvbt2_param.num_plp; i++)
								plld->dvbt2_param.list_plp[i].PLP_NUM_BLOCKS = plld->PLP_NUM_BLOCKS_M[i];


						}else 
							plld->dvbt2_param.auto_searching_data_symbol_num_block = 0;	

						dvbt2_param = plld->dvbt2_param;


					} else if (plld->_SysSta->IsModTyp_DvbT2() && plld->_FIf->IsT2miFile() == 1)
					{ // t2 file 
						if (plld->_SysSta->IPUsing() == 1)
						{
							plld->dvbt2_param.auto_searching_data_symbol_num_block = 1;
						}else 
							plld->dvbt2_param.auto_searching_data_symbol_num_block = 0;		

						strcpy(&plld->dvbt2_param.list_plp[0].file_path[0], &plld->_FIf->PlayParm.AP_lst.szfn[0]);
						dvbt2_param = plld->dvbt2_param;
					}
					else if ((plld->_SysSta->IsModTyp_DvbC2() && plld->_FIf->IsC2miFile() == 0) || plld->_SysSta->IsState_DvbT2LoopThru() == 1 || plld->_SysSta->IPUsing() == 1)
					{
						int BW_1;
						if ( plld->__C2_BW  == 0 )			BW_1 = 2;//6MHz
						else if ( plld->__C2_BW  == 1 )		BW_1 = 3;//7MHz
						else							 	BW_1 = 4;//8MHz


						plld->dvbc2_param.BW = BW_1;
						plld->dvbc2_param.BW_EXT = plld->__C2_RevTone;

						plld->dvbc2_param.FFT_SIZE = 0;
						plld->dvbc2_param.GUARD_INTERVAL = plld->__C2_Guard;

						plld->dvbc2_param.L1_MOD = 0;
						plld->dvbc2_param.Pilot = 0;
						plld->dvbc2_param.PAPR = 0;
						plld->dvbc2_param.S1 = 0;
						plld->dvbc2_param.L1_REPETITION = 0;
						plld->dvbc2_param.L1_COD = plld->__C2_L1;
						plld->dvbc2_param.FEC_TYPE = ((plld->__C2_Dslice_type & 0x1) << 1) | (plld->__C2_Dslice_FecHeder & 0x1);
						plld->dvbc2_param.L1x_LEN = 0;
						plld->dvbc2_param.TX_ID_AVAILABILTY = (plld->__C2_StartFreq & 0xFF0000) >> 16;
						plld->dvbc2_param.NETWORK_ID = plld->__C2_Network;
						plld->dvbc2_param.C2_ID = plld->__C2_System;
						plld->dvbc2_param.Cell_ID = (plld->__C2_StartFreq & 0xFFFF);
						plld->dvbc2_param.NUM_C2_FRAME = (plld->__C2_NotchStart & 0x3FC0) >> 6;
						plld->dvbc2_param.NUM_DATA_SYMBOLS = ((plld->__C2_NotchStart & 0x3F) << 6) | ((plld->__C2_NotchWidth & 0x1F8) >> 3);			
						plld->dvbc2_param.REGEN_FLAG = plld->__C2_NotchWidth & 0x7;
						plld->dvbc2_param.L1_POST_EXTENSION = plld->__C2_NumNoth;

						plld->dvbc2_param.num_plp = plld->__C2_PLP_Count;

						for (int i = 0; i < plld->dvbc2_param.num_plp; i++)
						{	
							strcpy(&plld->dvbc2_param.list_plp[i].file_path[0], plld->__C2_PLP_TS_M[i]);
							plld->dvbc2_param.list_plp[i].PLP_ID = plld->__C2_PLP_ID_M[i];		
							plld->dvbc2_param.list_plp[i].PLP_COD = plld->__C2_PLP_COD_M[i];
							plld->dvbc2_param.list_plp[i].PLP_MOD = plld->__C2_PLP_MOD_M[i];
							plld->dvbc2_param.list_plp[i].PLP_FEC = plld->__C2_PLP_FEC_M[i];
							plld->dvbc2_param.list_plp[i].PLP_HEM = plld->__C2_PLP_HEM_M[i];
							plld->dvbc2_param.list_plp[i].PLP_BITRATE = plld->__C2_PLP_TS_Bitrate_M[i];
							plld->dvbc2_param.list_plp[i].PLP_TIME_IL_TYPE = 0;
							plld->dvbc2_param.list_plp[i].PLP_TIME_IL_LENGTH = 0;

							plld->dvbc2_param.list_plp[i].PLP_NUM_BLOCKS = plld->__C2_PLP_BLK_M[i];
							plld->dvbc2_param.list_plp[i].PLP_ROTATION = plld->__C2_PLP_HEM_M[i];
						}
						if (plld->_SysSta->IPUsing() == 1 || plld->_SysSta->IsState_DvbC2LoopThru() == 1)
						{
							plld->dvbc2_param.auto_searching_data_symbol_num_block = 1;
						}else 
							plld->dvbc2_param.auto_searching_data_symbol_num_block = 0;	

						dvbc2_param = plld->dvbc2_param;

					} else if (plld->_SysSta->IsModTyp_DvbC2() && plld->_FIf->IsC2miFile() == 1)
					{ // c2 file 
						if (plld->_SysSta->IPUsing() == 1)
						{
							plld->dvbc2_param.auto_searching_data_symbol_num_block = 1;
						}else 
							plld->dvbc2_param.auto_searching_data_symbol_num_block = 0;	
						
						strcpy(&plld->dvbc2_param.list_plp[0].file_path[0], &plld->_FIf->PlayParm.AP_lst.szfn[0]);
						dvbc2_param = plld->dvbc2_param;
					}

					plld->concrete_observer.pLLD = plld;
					plld->playback.set_observer(&plld->concrete_observer);

					int rc = plld->playback.init(plld, media_source_type, multiplexing_type, output_type);
					if (rc != 0)
					{
						// re-init
						plld->playback.quit();
						rc = plld->playback.init(plld, media_source_type, multiplexing_type, output_type);
						if (rc != 0)
						{
							// observer thread 
							tlv_message_t message;
							message.id = PLAYBACK_LOG_MESSAGE;
							message.param = NULL;
							tlv_mq_send(&plld->playback.thread_observer, &message);
						}
						return ;
					}


				}

				break;
			case PLAYBACK_STATE_INIT:
				if (message_type == PLAY_MESSAGE_PLAY)
				{			
					plld->playback.play();
				} else if (message_type == PLAY_MESSAGE_QUIT)
					plld->playback.quit();
				break;
			case PLAYBACK_STATE_PLAY:
				if (message_type == PLAY_MESSAGE_STOP){
					plld->playback.stop();						
				} else if (message_type == PLAY_MESSAGE_PAUSE)
				{
					plld->playback.pause();
				}else if (message_type == PLAY_MESSAGE_SEEK)
				{
					long *seek_pos = (long *)param;
					plld->playback.multiplex_interface->ioctl(PLAY_IO_CONTROL_SEEK, seek_pos);
				}else if (message_type == PLAY_MESSAGE_REWIND)
				{
					long seek_pos = 0;
					plld->playback.multiplex_interface->ioctl(PLAY_IO_CONTROL_SEEK, &seek_pos);
				} else if (message_type == PLAY_MESSAGE_GET_TS_BITRATE)
				{
					long *ts_bitrate = (long *)param;
					*ts_bitrate = (long )plld->playback.media_interface[0]->mpeg2ts_statistic.get_bitrate();
				}
				break;		
			case PLAYBACK_STATE_PAUSE:
				if (message_type == PLAY_MESSAGE_RESUME) {
					plld->playback.resume();
				} else if (message_type == PLAY_MESSAGE_STOP)
				{
					plld->playback.stop();
				}
		}

	}
}

extern "C" TLV_ThreadEntry(observer_thread, (void *arg))
{
	CHldPlayback *_play = (CHldPlayback *)arg;
	_play->ObserverThread(NULL);
	return NULL;
}


extern "C" TLV_ThreadEntry(multiplexer_thread, (void *arg)) 
{
	CHldPlayback *_play = (CHldPlayback *)arg;
	//printf("multiplexer thread is running .... \n"); 
	_play->multiplex_interface->main_loop(_play);
	return NULL;
}

extern "C" TLV_ThreadEntry(output_thread, (void *arg))
{
	CHldPlayback *_play = (CHldPlayback *)arg;
	//printf("output thread is running .... \n"); 
	_play->output_interface->main_loop(_play);
	return NULL;
}


extern "C" TLV_ThreadEntry(thread_media_receving_data_entry, (void *arg))
{
	CTLVMedia *meida_source = (CTLVMedia *)arg;	
	//printf("input thread is running .... \n"); 
	meida_source->media_receving_data_loop(NULL);
	return NULL;
}



/////////////////////////////////////////////////////////////////
CHldPlayback::CHldPlayback(void)
{


	playback_state = PLAYBACK_STATE_NONE;
	playback_control = PLAYBACK_CONTROL_NONE;

}
CHldPlayback::~CHldPlayback()
{

}


int CHldPlayback::init(CHld *_plld, int media_source_type, int multiplexing_type, int output_type)
{	

	this->plld = _plld;
	int i = 0;

	if (playback_state != PLAYBACK_STATE_NONE)
		return -1;


	// initialize default value 

	play_mode = PLAY_MODE_REPEAT; // no loop, just repeat
	num_media_interface = 1;
	if (multiplexing_type == PLAY_DVBT2_MUX)
		num_media_interface = dvbt2_param.num_plp;

	if (multiplexing_type == PLAY_DVBC2_MUX)
		num_media_interface = dvbc2_param.num_plp;

	//////////////////////////////
	// select plug-in
	switch (media_source_type)
	{
		case PLAY_MEDIA_SOURCE_IP_BUFFER:
			media_interface[0] = new CTLVBuffer();		
			break;
		//2012/7/6 DVB-T2 ASI
		case PLAY_MEDIA_SOURCE_ASI_310M:
			media_interface[0] = new CTS_IN_Buffer();		
			break;
		case PLAY_MEDIA_SOURCE_IP_RTP:
			//printf("Debug %s, %d source udp \n", __FILE__, __LINE__);
			media_interface[0] = new CTLVIPServer();				
			break;		
		case PLAY_MEDIA_SOURCE_FILE:
			int i;
			for (i = 0; i < num_media_interface; i++)
			{
				// nopppppppppppp
				if (multiplexing_type == PLAY_DVBT2_MUXED_FILE) 
				{
					media_interface[i] = new CTLVFileDVBT2();
				}else if (multiplexing_type == PLAY_DVBT2_MUX)
				{
					media_interface[i] = new CTLVFileDVBT2Mul();
				}
				else if (multiplexing_type == PLAY_DVBC2_MUXED_FILE) 
				{
					media_interface[i] = new CTLVFileDVBC2();
				}else if (multiplexing_type == PLAY_DVBC2_MUX)
				{
					media_interface[i] = new CTLVFileDVBC2Mul();
				}
				else if (multiplexing_type == PLAY_MULTIPLEXER_ISDBT_NO_MUX)
				{
					media_interface[i] = new CTLVFileISDBT();
				}
				else
					media_interface[i] = new MTSRemux();						
			}

			break;
		default:
			media_interface[0] = new CTLVIPServer();		
	}

	//printf("===== %s   %d \n", __FILE__, __LINE__);
	//////////////////////////////
	// setting MEDIA

	if(multiplexing_type == PLAY_MULTIPLEXER_ISDBT_MUX)
	{
		media_interface[0]->option.num_filter_pids = isdbt_tmcc_layer_info.layer_a_num_pids;	
		for (i = 0; i < media_interface[0]->option.num_filter_pids; i++)
		{
			media_interface[0]->ioctl(2, IO_REQUEST_ADD_PID_FOR_TS_REMUX, isdbt_tmcc_layer_info.layer_a_pids[i]);
		}		

			// just support 1-seg 
		tmcc_param.m_Layers[0].m_NumberOfSegments = 1;
		tmcc_param.m_Layers[1].m_NumberOfSegments = 12;
		tmcc_param.m_Layers[2].m_NumberOfSegments = 0;
		int layer_num_tsp[4];	
		CalculatedParams  calculated_param;
		ISDBTMultiplexer::CalculateParams(&calculated_param, &tmcc_param);

		media_interface[0]->ioctl(2, IO_REQUEST_REMUX_OUT_BITRATE, (int)(tmcc_param.m_Layers[0].bps));


		//isdbt_tmcc_layer_info.

		strcpy(&media_interface[0]->option.networking_or_file[0], &file_name[0] );
		num_media_interface = 1;

	}else if (multiplexing_type == PLAY_MULTIPLEXER_ISDBT_NO_MUX)
	{
		strcpy(&media_interface[0]->option.networking_or_file[0], &file_name[0]);
		num_media_interface = 1;
	}
	else if (multiplexing_type== PLAY_DVBT2_MUX)
	{ // DVB-T2
		for (i = 0; i < dvbt2_param.num_plp; i++)
		{
			strcpy(&media_interface[i]->option.networking_or_file[0], dvbt2_param.list_plp[i].file_path);			
		}
		
		if (media_source_type == PLAY_MEDIA_SOURCE_IP_BUFFER || media_source_type == PLAY_MEDIA_SOURCE_IP_RTP || media_source_type == PLAY_MEDIA_SOURCE_ASI_310M)//2012/7/6 DVB-T2 ASI
		{
			media_interface[0]->option.bitrate = plld->TL_CurrentBitrate;		
			media_interface[0]->option.udp_port = plld->TL_rx_udp_port;
			
		}
		num_media_interface = dvbt2_param.num_plp;
		for (i = 0; i < num_media_interface; i++)
			media_interface[i]->ioctl(2, IO_IDVBT2_SET_BITRATE, plld->PLP_TS_BITRATE_M[i]);

	}else if (multiplexing_type == PLAY_DVBT2_MUXED_FILE)
	{
		strcpy(&media_interface[0]->option.networking_or_file[0], dvbt2_param.list_plp[0].file_path);
		num_media_interface = 1;
	}
	else if (multiplexing_type== PLAY_DVBC2_MUX)
	{ // DVB-C2
		for (i = 0; i < dvbc2_param.num_plp; i++)
			strcpy(&media_interface[i]->option.networking_or_file[0], dvbc2_param.list_plp[i].file_path);

		if (media_source_type == PLAY_MEDIA_SOURCE_IP_BUFFER || media_source_type == PLAY_MEDIA_SOURCE_IP_RTP || media_source_type == PLAY_MEDIA_SOURCE_ASI_310M)
		{
			media_interface[0]->option.bitrate = plld->TL_CurrentBitrate;		
			media_interface[0]->option.udp_port = plld->TL_rx_udp_port;
			
		}

		num_media_interface = dvbc2_param.num_plp;
		for (i = 0; i < num_media_interface; i++)
			media_interface[i]->ioctl(2, IO_IDVBT2_SET_BITRATE, plld->__C2_PLP_TS_Bitrate_M[i]);
	}else if (multiplexing_type == PLAY_DVBC2_MUXED_FILE)
	{
		strcpy(&media_interface[0]->option.networking_or_file[0], dvbc2_param.list_plp[0].file_path);
		num_media_interface = 1;
	}

	for (i = 0; i < num_media_interface; i++)
		media_interface[i]->option.media_source_type = media_source_type;
	//printf("===== %s   %d \n", __FILE__, __LINE__); 

	/////////////////////////////////
	// multiplexer option 
	switch (multiplexing_type)
	{
		case PLAY_MULTIPLEXER_ISDBT_MUX:
			multiplex_interface = new ISDBTMultiplexer();		
			break;
		case PLAY_MULTIPLEXER_ISDBT_NO_MUX:
			multiplex_interface = new ISDBT_Reader();
			break;
		case PLAY_DVBT2_MUX:		
			multiplex_interface = new DVBT2Multiplexer();
			break;
		case PLAY_DVBT2_MUXED_FILE: 
			multiplex_interface = new DVBT2Reader();
			break;
		case PLAY_DVBC2_MUX:		
			multiplex_interface = new DVBC2Multiplexer();
			break;
		case PLAY_DVBC2_MUXED_FILE: 
			multiplex_interface = new DVBC2Reader();
			break;

		default:
			return -1; // do not support
	}
	// set option for multiplexer 
	if (multiplexing_type == PLAY_DVBT2_MUX)
	{
		DVBT2_PARAM *tmp = (DVBT2_PARAM *)multiplex_interface->multiplex_option;
		*tmp = dvbt2_param;
	} else if (multiplexing_type == PLAY_MULTIPLEXER_ISDBT_MUX)
	{
		ISDBT_PARAM *tmp = (ISDBT_PARAM *)multiplex_interface->multiplex_option;
		*tmp = tmcc_param;
	} else if (multiplexing_type == PLAY_DVBC2_MUX)
	{
		DVBC2_PARAM *tmp = (DVBC2_PARAM *)multiplex_interface->multiplex_option;
		*tmp = dvbc2_param;
	} else
		multiplex_interface->multiplex_option = NULL;




	//printf("===== %s   %d \n", __FILE__, __LINE__);

	///////////////////////////////////////
	// output option 	

	// manual test
	//output_type = PLAY_OUTPUT_FILE;
	//sprintf(&file_name[0], "c:\\ts\\asiinTest.t2");

	if (output_type == PLAY_OUTPUT_TVBBOARD)
	{
		switch (multiplexing_type)
		{
			case PLAY_MULTIPLEXER_ISDBT_MUX:
				output_interface =  new CHldBoardISDBT();
				break;
			case PLAY_MULTIPLEXER_ISDBT_NO_MUX:
				output_interface = new CHldBoardISDBTUsingTMCC();
				break;
			case PLAY_DVBT2_MUX:
			case PLAY_DVBT2_MUXED_FILE:
				output_interface = new CHldBoardDVBT2();
				break;			
			case PLAY_DVBC2_MUX:
			case PLAY_DVBC2_MUXED_FILE:
				output_interface = new CHldBoardDVBC2();
				break;			
			default:
				return -1; // do not support
		}
	}else if (output_type == PLAY_OUTPUT_FILE)
	{
		sprintf(&this->file_path_for_writing[0], &file_name[0]);	
		output_interface = new CHldFileWriter();
	}else if (output_type == PLAY_OUTPUT_NULL)
	{
		output_interface = new OutputNULL();
	}else
	{
//		printf("assert: canot set output type  function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return -1;
	}

	// for making a file 

	// verbose 
	//printf(" ========== selected plug-in ===================\n", &multiplex_interface->name[0]);
	//for ( i = 0; i < this->num_media_interface; i++)
		//printf("input plug-in: %s \n", &media_interface[i]->name[0]);
	//printf("multiplexing plug-in: %s \n", &multiplex_interface->name[0]);
	//printf("output plug-in: %s \n", &output_interface->name[0]);
	//printf(" ===============================================\n", &multiplex_interface->name[0]);

	cir_multiplexed_data_buffer = new CCircularBuffer(1, MULTIPLEXER_BUFF_SIZE);
	for ( i = 0; i < this->num_media_interface; i++)
	{
		tlv_mutex_init(&mutex_cir_input_ts[i]);
	}


	tlv_mutex_init(&playback_end_mutex);
	tlv_mutex_init(&mutex_cir_multiplexed_data);
	tlv_cond_init(&cv_multiplexed_data_in, NULL);
	tlv_cond_init(&cv_multiplexed_data_out, NULL);

	playback_state = PLAYBACK_STATE_INIT;	

	//printf("===== %s   %d \n", __FILE__, __LINE__); 

	tlv_log::get_instance()->init();
	
	return 0;
}

int CHldPlayback::quit()
{
	int i;
	

	if (playback_state != PLAYBACK_STATE_INIT)
		return -1;


	tlv_mutex_destroy(&playback_end_mutex);	
	tlv_mutex_destroy(&mutex_cir_multiplexed_data);
	tlv_cond_destroy(&cv_multiplexed_data_in);
	tlv_cond_destroy(&cv_multiplexed_data_out);



	delete cir_multiplexed_data_buffer;

	for ( i = 0; i < this->num_media_interface; i++)
	{
		tlv_mutex_destroy(&mutex_cir_input_ts[i]);
	}

	delete output_interface;
	delete multiplex_interface;
	for ( i = 0; i < num_media_interface; i++)
		delete media_interface[i];

	playback_state = PLAYBACK_STATE_NONE;

	tlv_log::get_instance()->quit();

	return 0;
}

// return 
// 	0: normal 
// 	-1: error 

int CHldPlayback::play()
{
	int i;
	if (playback_state != PLAYBACK_STATE_INIT)
		return -1;

	playback_control = PLAYBACK_CONTROL_NONE;
	cir_multiplexed_data_buffer->Clear();		

	thread_observer.thread_entry = observer_thread; 
	thread_observer.param = this;
	// oberser thread is a message queue thread 
	tlv_mq_init(&thread_observer);
	tlv_thread_create(&thread_observer);

	for (i = 0; i < num_media_interface; i++)
	{
		media_interface[i]->sys_play = this;
		media_interface[i]->mutex_cir_data = &mutex_cir_input_ts[i];
		media_interface[i]->cir_input_ts_buffer = cir_input_ts_buffer[i];
		media_interface[i]->thread_handle = &thread_media_receiving_data[i];

		thread_media_receiving_data[i].thread_entry = thread_media_receving_data_entry;
		thread_media_receiving_data[i].param = media_interface[i];

		tlv_thread_create(&thread_media_receiving_data[i]);
	}

	// create producer and consumer thread, and observer

	thread_producer.thread_entry = multiplexer_thread; 
	thread_producer.param = this;
	// producer have a message queue
	tlv_mq_init(&thread_producer);
	tlv_thread_create(&thread_producer);

	thread_consumer.thread_entry = output_thread; 
	thread_consumer.param = this;
	tlv_thread_create(&thread_consumer);


	playback_state = PLAYBACK_STATE_PLAY;

	return 0;
}



int CHldPlayback::pause()
{
	if (playback_state != PLAYBACK_STATE_PLAY)
		return -1;

	//printf("pause .... \n");
	playback_state = PLAYBACK_STATE_PAUSE;

	playback_control =  PLAYBACK_CONTROL_PAUSE;
	return 0;
}

int CHldPlayback::resume()
{
	if (playback_state != PLAYBACK_STATE_PAUSE)
		return -1;

	//printf("resume .... \n");
	playback_control =  PLAYBACK_CONTROL_NONE;
	playback_state = PLAYBACK_STATE_PLAY;

	return 0;
}



int CHldPlayback::stop()
{
	int i;

	if (playback_state != PLAYBACK_STATE_PLAY)
		return -1;

	// observer thread 
_observer_:

	tlv_message_t message;
	message.id = PLAYBACK_USER_STATE_EXIT;
	message.param = NULL;
	tlv_mq_send(&thread_observer, &message);

	if (thread_observer.thread_state != THREAD_STATE_FINISH)
	{
		tlv_sleep(20);
		goto _observer_;
	}


	playback_control = PLAYBACK_CONTROL_END_OUTPUT;
_output_: 
	tlv_cond_signal(&cv_multiplexed_data_out);
	// consumer  
	if (thread_consumer.thread_state != THREAD_STATE_FINISH)
	{
		goto _output_;
	}
	
	playback_control = PLAYBACK_CONTROL_END_MULTIPLEXER;

_multiplexer_:	
	message.id = PLAYBACK_CONTROL_END_MULTIPLEXER;
	message.param = NULL;
	tlv_mq_send(&thread_producer, &message);
	
	tlv_cond_signal(&cv_multiplexed_data_in);
	for ( i = 0; i < num_media_interface; i++)
	{
		tlv_cond_signal(&media_interface[i]->cv_cir_out);
	}	
	// multiplexer
	if (thread_producer.thread_state != THREAD_STATE_FINISH)
	{
		tlv_sleep(20);
		goto _multiplexer_;
	}

	playback_control = PLAYBACK_CONTROL_END_INPUT;

	for ( i = 0; i < num_media_interface; i++)
	{
		media_interface[i]->force_close();
	}

	for ( i = 0; i < num_media_interface; i++)
	{
		tlv_cond_signal(&media_interface[i]->cv_cir_in);
	}
_input_:
	for ( i = 0; i < num_media_interface; i++)
	{
		if (thread_media_receiving_data[i].thread_state != THREAD_STATE_FINISH)
			goto _input_;
	}

	//printf("playback is ending ... \n");
	tlv_mq_destroy(&thread_observer);
	tlv_mq_destroy(&thread_producer);


	playback_state = PLAYBACK_STATE_INIT;
	plld->SetFinishAsi(0);
	return 0;
}

void CHldPlayback::ObserverThread(void *param)
{
	while (1)
	{
		tlv_message_t message;		
		int rc = tlv_mq_receive(&thread_observer, &message);

		switch (message.id)
		{
			case PLAYBACK_USER_STATE_START_STREAMING:
				observer->Update(PLAYBACK_USER_STATE_START_STREAMING);
				break;
			case PLAYBACK_USER_STATE_STOP_STREAMING:
				observer->Update(PLAYBACK_USER_STATE_STOP_STREAMING);			
				break;
			case PLAYBACK_LOG_MESSAGE:
				observer->Update(PLAYBACK_LOG_MESSAGE);			
				break;
			case PLAYBACK_USER_STATE_EXIT:
				goto _END_;

		}

	}

_END_:
	thread_observer.thread_state = THREAD_STATE_FINISH;
	//printf("[HLD] ObserverThread ending .....\n");

}

void CHldPlayback::set_observer( CHldPlayObserver *_observer )
{
	observer = _observer;

}



