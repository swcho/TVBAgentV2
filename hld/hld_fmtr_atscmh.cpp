
#if defined(WIN32)
#include	<Windows.h>
#include	<process.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<pthread.h>
#endif
#include	<stdio.h>
#include	<math.h>

#include	"../include/hld_structure.h"
#include	"hld_fmtr_atscmh.h"
#include	"LLDWrapper.h"


//////////////////////////////////////////////////////////////////////////////////////
CHldFmtrAtscMH::CHldFmtrAtscMH(int _my_id, void *_hld)
{
	my_hld_id = _my_id;
	my_hld = _hld;

	m_MHE_packet_PID = 0;
	m_MHE_packet_index = -1;
	m_MHE_TPC_information_validity = 1;

}

CHldFmtrAtscMH::~CHldFmtrAtscMH()
{

}

void	CHldFmtrAtscMH::SetCommonMethod_81(
	CHldGVar	*__sta__,
	CHldFsRdWr	*__fIf__,
	CHldBdLog	*__hLog__)
{
	__Sta__	=	__sta__;
	__FIf__	=	__fIf__;
	__HLog__	=	__hLog__;
}

//////////////////////////////////////////////////////////////////////////////////////	ATSC-M/H
void CHldFmtrAtscMH::InitMHEInfo(int packet_pid)
{
	int mh_mode, mh_pid;
	mh_pid = packet_pid;

	//MHMODE
	mh_mode = ((mh_pid > 0 && mh_pid != 0xF) ? 1 : 0);
	if(m_MHE_TPC_information_validity == 0)
		mh_mode = 0;
	//TVB593
	__FIf__->TSPL_SET_MH_MODE(15, mh_mode);
	__HLog__->HldPrint_1("Hld-Bd-Ctl. ATSC-M/H MODE=", mh_mode<<3);

	//MHPID
	mh_pid = (mh_mode ? mh_pid : 0xF);
	//TVB593
	if(m_MHE_TPC_information_validity == 0)
		mh_pid = 0xF;
	__FIf__->TSPL_SET_MH_PID(15, mh_pid);
	__HLog__->HldPrint_1("Hld-Bd-Ctl. ATSC-M/H PID=", mh_pid);
}

void CHldFmtrAtscMH::InitMHELoopPath(int packet_pid, int tsio_direction)
{
	int current_tsio_direction, direction;
	int mh_mode, mh_pid;
	mh_pid = packet_pid;
	
	mh_mode = ((mh_pid > 0 && mh_pid != 0xF) ? 1 : 0); 
	current_tsio_direction = __FIf__->TSPL_GET_TSIO_DIRECTION();

	//__HLog__->HldPrint("Hld-Bd-Ctl. ATSC-M/H InitMHELoopPath :: tsio_direction=%d, current_tsio_direction=%d, mh_mode=%d \n", tsio_direction, current_tsio_direction, mh_mode);
	
	if ( mh_mode )
	{
		//FILE
		if ( tsio_direction == TSIO_PLAY_WITH_310INPUT )
		{
			direction = TSIO_FILE_LOOP_PLAY;
		}
		else
		{
			direction = tsio_direction;
		}
	}
	else
	{
		direction = tsio_direction;
	}

	if ( current_tsio_direction != direction )
	{
		__FIf__->TSPL_SET_TSIO_DIRECTION(direction);
		__HLog__->HldPrint_1("Hld-Bd-Ctl. ATSC-M/H LOOP PATH=", direction);
	}
}

//ATSC-M/H TPC
#define CHK_ATSC_MH_PARAM if((strlen((char*)tpc_info) + strlen(debug_string)) > 0x2000) {return 0;}
#define CHK_ATSC_MH_PARAM_2(a, b) if((strlen(a) + strlen(b)) >= 2048) {szResult = NULL;free(szBuf);return 0;}
static unsigned char derandom_coeff[20] = { 0xC0, 0x6D, 0x3F, 0x99, 0x38, 0x6A, 0x29, 0x52, 0xB6, 0x64,
	0xD0, 0x45, 0x8A, 0xEB, 0xDE, 0x4B, 0x6B, 0x31, 0x60, 0xC2 };
unsigned char TPCOFFSET[NUM_OF_PCCCDATA][2] = {{16,157},{14,157},{12,157},{10,157},{8,157},{6,157},{4,157},{2,157},{53,17},
	{51,17},{49,17},{47,17},{45,17},{43,17},{41,17},{39,17},{37,17},{35,17},{33,17},{31,17},{29,17},{27,17},{25,17},{23,17} };

int CHldFmtrAtscMH::TL_DecodeTPCInfo(
	unsigned char buf[],
	unsigned char *subframenum,
	unsigned char *slotnum,
	unsigned char *paradeid, 
	unsigned char *fic_version,
	unsigned char *tpc_protocol_version, 
	unsigned char *tpc_info)
{
	FILE *fp = NULL;
	char szInfo[64];

	int	i,j,k;
	unsigned char pbyte[NUM_OF_TPC];
	unsigned char currstate[NUM_PCCC_COMP];
	unsigned char nextstate,tmp,uin,c1c0;
	unsigned char TPC[NUM_OF_TPC];
	unsigned char UE[NUM_PCCC_COMP];

//	unsigned char subframenum,slotnum,paradeid;
	unsigned char current_starting_group_number;
	unsigned char current_number_of_groups_minus_1;
	unsigned char current_rs_frame_mode;
	unsigned char current_rs_code_mode_primary;
	unsigned char current_rs_code_mode_secondary;
	unsigned char current_sccc_block_mode;
	unsigned char current_sccc_outer_code_mode_a;
	unsigned char current_sccc_outer_code_mode_b;
	unsigned char current_sccc_outer_code_mode_c;
	unsigned char current_sccc_outer_code_mode_d;
	unsigned char current_TNoG;
//	unsigned char fic_version;
	unsigned char parade_continuity_counter;
//	unsigned char tpc_protocol_version;

	// 1. even component decoder 
	// j==index % 6 
	// buf[0] : EVEN#2
	// buf[1] : EVEN#3
	// buf[2] : EVEN#4
	// buf[3] : EVEN#5
	// buf[4] : EVEN#0
	// buf[5] : EVEN#1

	// b7 b6 b5 b4 b3 b2 b1 b0
	// C1 C0 C1 C0 C1 C0 C1 C0

	// init PCCC decoder buffer
	for(i=0;i<NUM_OF_TPC;i++)
		pbyte[i] = 0;

	// reset state machine
	for(i=0;i<NUM_PCCC_COMP;i++)
		currstate[i] = 0;

	for(k=0;k<NUM_OF_DECODERLOOP;k++){
	
		// init UE buffer before new 6 data decoding
		UE[0] = 0;
		UE[1] = 0;
		UE[2] = 0;
		UE[3] = 0;
		UE[4] = 0;
		UE[5] = 0;


		for(j=0;j<NUM_PCCC_COMP;j++){	// number of trellis coder
			
			tmp = 0;
			for(i=3;i>=0;i--){
	
				c1c0 = (buf[(k*NUM_PCCC_COMP)+j] >> (i<<1)) & 0x3;		// 2 bit , C1,C0 
	
//				printf("C1C0 :%x\n",c1c0);
	
				if (currstate[j] == 0) {
					if (c1c0 == 0){ 
						nextstate = 0; uin = 0;
					}
					else if (c1c0 == 2){ 
						nextstate = 2; uin = 1;
					}
					else{
						//printf("decoding error @ i:%d, j:%d, currstate:%d, c1c0:%d\n",j,i,currstate,c1c0);
						return TREL_ERROR_NUM;	
					}
				}
				else if (currstate[j] == 1){
					if (c1c0 == 1){ 
						nextstate = 2; uin = 0;
					}
					else if (c1c0 == 3){ 
						nextstate = 0; uin = 1;
					}
					else{
						//printf("decoding error @ i:%d, j:%d, currstate:%d, c1c0:%d\n",j,i,currstate,c1c0);
						return TREL_ERROR_NUM;	
					}
				}
				else if (currstate[j] == 2){
					if (c1c0 == 2){ 
						nextstate = 1; uin = 0;
					}
					else if (c1c0 == 0){ 
						nextstate = 3; uin = 1;
					}
					else{
						//printf("decoding error @ i:%d, j:%d, currstate:%d, c1c0:%d\n",j,i,currstate,c1c0);
						return TREL_ERROR_NUM;	
					}
				}
				else { // currstate == 3
					if (c1c0 == 3){ 
						nextstate = 3; uin = 0;
					}
					else if (c1c0 == 1){ 
						nextstate = 1; uin = 1;
					}
					else{
						//printf("decoding error @ i:%d, j:%d, currstate:%d, c1c0:%d\n",j,i,currstate,c1c0);
						return TREL_ERROR_NUM;	
					}
				}

				// state update, tmp value update
				tmp = tmp | (uin<<i);	// temporal even component decoder output
#ifdef DEBUG_PRINT
				//printf("decoding ok @ i:%d, j:%d, currstate:%d, c1c0:%d, nextstate:%d, uin:%d, tmp:%x \n",j,i,currstate[j],c1c0,nextstate, uin,tmp);
#endif
				currstate[j] = nextstate;
			} // i-loop

			UE[(j+2)%6] = tmp;

	  	} // j-loop


#ifdef DEBUG_PRINT
		fprintf(fp, "Ueven[0-5]:%X %X %X %X %X %X \n", UE[0],UE[1],UE[2],UE[3],UE[4],UE[5]);
#endif


		// 2. merge pByte0,pByte1,pByte2 from decoding result

		//                              B3   B2   B1   B0
		//								-------------------
		// j=4, output byte 8 , UE[0] 	b0.7 b0.1 b1.3 b2.5
		// j=5, output byte 10, UE[1] 	b0.6 b0.0 b1.2 b2.4
		// j=0, output byte 0 , UE[2] 	b0.5 b1.7 b1.1 b2.3
		// j=1, output byte 2 , UE[3] 	b0.4 b1.6 b1.0 b2.2
		// j=2, output byte 4 , UE[4] 	b0.3 b1.5 b2.7 b2.1
		// j=3, output byte 6 , UE[5] 	b0.2 b1.4 b2.6 b2.0


		pbyte[k*3+0] = (((UE[0]>>3)&1)<<7) | (((UE[1]>>3)&1)<<6) | (((UE[2]>>3)&1)<<5) | (((UE[3]>>3)&1)<<4) | 
					(((UE[4]>>3)&1)<<3) | (((UE[5]>>3)&1)<<2) | (((UE[0]>>2)&1)<<1) | (((UE[1]>>2)&1)<<0); 

		pbyte[k*3+1] = (((UE[2]>>2)&1)<<7) | (((UE[3]>>2)&1)<<6) | (((UE[4]>>2)&1)<<5) | (((UE[5]>>2)&1)<<4) | 
					(((UE[0]>>1)&1)<<3) | (((UE[1]>>1)&1)<<2) | (((UE[2]>>1)&1)<<1) | (((UE[3]>>1)&1)<<0); 

		pbyte[k*3+2] = (((UE[4]>>1)&1)<<7) | (((UE[5]>>1)&1)<<6) | (((UE[0]>>0)&1)<<5) | (((UE[1]>>0)&1)<<4) | 
					(((UE[2]>>0)&1)<<3) | (((UE[3]>>0)&1)<<2) | (((UE[4]>>0)&1)<<1) | (((UE[5]>>0)&1)<<0); 
	 		

	}	// k-loop
	
	if ( fp )
	{
		fprintf(fp, "After Even component decoder, pByte =  ");
		for (i=0;i<NUM_OF_TPC;i++)
			fprintf(fp, "%02X ", pbyte[i]);

		fprintf(fp, "\n");
	}

	// 3. Byte derandomizer
	for (i=0;i<NUM_OF_TPC;i++)
		TPC[i]= pbyte[i] ^ derandom_coeff[i];

#ifdef DEBUG_PRINT
	if ( fp )
	{
		for (i=0;i<NUM_OF_TPC;i++)
			fprintf(fp, "TPC[%d] : %02X xor %02X = %02X \n", i,pbyte[i], derandom_coeff[i],TPC[i]);

		for (i=0;i<NUM_OF_TPC;i++)
			fprintf(stderr, "TPC[%d] : %02X xor %02X = %02X \n", i,pbyte[i], derandom_coeff[i],TPC[i]);
	}
#endif

	// 4. analyze TPC data
	// subframe_number = 3bit
	// slot_number = 4bit
	// parade_id = 7bit
	*subframenum = (TPC[0] >> 5) & 0x7;
	*slotnum = (TPC[0] >> 1) & 0xf;
	*paradeid = ((TPC[0] & 0x1)<<6) | ((TPC[1]>>2) & 0x3f);

	if ( tpc_info )
	{
		//sprintf(debug_string, "sub_frame_number=%d\r\n", *subframenum);
		//CHK_ATSC_MH_PARAM
		//strcat((char*)tpc_info, debug_string);
		
		if ( *slotnum == 0 /*|| *slotnum == 15*/ )
		{
			sprintf(debug_string, "Slot number= %d\r\n", *slotnum);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);
		}
		
		sprintf(debug_string, "Parade ID= %d\r\n", *paradeid);
		CHK_ATSC_MH_PARAM
		strcat((char*)tpc_info, debug_string);
		/*
		if (*subframenum <= 1)
		{
			sprintf(debug_string, "Current M/H Frame : \r\n");
		}
		else
		{
			sprintf(debug_string, "Next M/H Frame : \r\n");
		}
		CHK_ATSC_MH_PARAM
		strcat((char*)tpc_info, debug_string);
		*/
	}

	//if (*subframenum <= 1)
	if (1)
	{
		current_starting_group_number  = (((TPC[1] & 0x3) << 2) | ((TPC[2] >> 6) & 0x3));
		current_number_of_groups_minus_1 = ((TPC[2]>>3) & 7);
		current_rs_frame_mode = ((TPC[3]>>6) & 0x3);
		current_rs_code_mode_primary = ((TPC[3]>>4) & 0x3);
		current_rs_code_mode_secondary = ((TPC[3]>>2) & 0x3);
		current_sccc_block_mode =  ((TPC[3]>>0) & 0x3);
		current_sccc_outer_code_mode_a = ((TPC[4]>>6) & 0x3);
		current_sccc_outer_code_mode_b = ((TPC[4]>>4) & 0x3);
		current_sccc_outer_code_mode_c = ((TPC[4]>>2) & 0x3);
		current_sccc_outer_code_mode_d =  ((TPC[4]>>0) & 0x3);

		current_TNoG  = (TPC[6] >> 2) & 0x1f;
		
		if ( tpc_info )
		{
			sprintf(debug_string, "TNoG= %d\r\n", current_TNoG);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			sprintf(debug_string, "Starting group number= %d\r\n", current_starting_group_number);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_rs_frame_mode == 0x00 )		sprintf(szInfo, "%s", "Single RS frame");
			else if ( current_rs_frame_mode == 0x01 )	sprintf(szInfo, "%s", "Dual RS frame");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "RS frame mode= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_rs_code_mode_primary == 0x00 )			sprintf(szInfo, "%s", "211, 187");
			else if ( current_rs_code_mode_primary == 0x01 )	sprintf(szInfo, "%s", "223, 187");
			else if ( current_rs_code_mode_primary == 0x02 )	sprintf(szInfo, "%s", "235, 187");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "RS code mode primary= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_rs_code_mode_secondary == 0x00 )		sprintf(szInfo, "%s", "211, 187");
			else if ( current_rs_code_mode_secondary == 0x01 )	sprintf(szInfo, "%s", "223, 187");
			else if ( current_rs_code_mode_secondary == 0x02 )	sprintf(szInfo, "%s", "235, 187");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "RS code mode secondary= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_sccc_block_mode == 0x00 )				sprintf(szInfo, "%s", "Separate");
			else if ( current_sccc_block_mode == 0x01 )			sprintf(szInfo, "%s", "Combined");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "SCCC block mode= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_sccc_outer_code_mode_a == 0x00 )		sprintf(szInfo, "%s", "Half");
			else if ( current_sccc_outer_code_mode_a == 0x01 )	sprintf(szInfo, "%s", "Quarter");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "SCCC outer code mode A= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_sccc_outer_code_mode_b == 0x00 )		sprintf(szInfo, "%s", "Half");
			else if ( current_sccc_outer_code_mode_b == 0x01 )	sprintf(szInfo, "%s", "Quarter");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "SCCC outer code mode B= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_sccc_outer_code_mode_c == 0x00 )		sprintf(szInfo, "%s", "Half");
			else if ( current_sccc_outer_code_mode_c == 0x01 )	sprintf(szInfo, "%s", "Quarter");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "SCCC outer code mode C= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);

			memset(szInfo, 0x00, 64);
			if ( current_sccc_outer_code_mode_d == 0x00 )		sprintf(szInfo, "%s", "Half");
			else if ( current_sccc_outer_code_mode_d == 0x01 )	sprintf(szInfo, "%s", "Quarter");
			else sprintf(szInfo, "%s", "unknown");
			sprintf(debug_string, "SCCC outer code mode D= %s\r\n", szInfo);
			CHK_ATSC_MH_PARAM
			strcat((char*)tpc_info, debug_string);
		}
	}

	*fic_version = (TPC[5] >> 3) & 0x1f;
	parade_continuity_counter = ((TPC[5]&0x7)<<1) | ((TPC[6]>>7)&0x1);
	*tpc_protocol_version = (TPC[9] & 0x1f);

	if ( tpc_info )
	{
		//sprintf(debug_string, "FIC version=%d\r\n", *fic_version);
		//CHK_ATSC_MH_PARAM
		//strcat((char*)tpc_info, debug_string);

		//sprintf(debug_string, "parade_continuity_counter=%d\r\n", parade_continuity_counter);
		//CHK_ATSC_MH_PARAM
		//strcat((char*)tpc_info, debug_string);

		//sprintf(debug_string, "TPC Protocol version=%d\r\n", *tpc_protocol_version);
		//CHK_ATSC_MH_PARAM
		//strcat((char*)tpc_info, debug_string);

		strcat((char*)tpc_info, "\t");
	}
	
	// 5. save result to file
	/*
	if ( fp )
	{
		fprintf(fp, "TPC[0-10] : ");
		for (i=0;i<10;i++)
			fprintf(fp, "%02X ", TPC[i]);
		fprintf(fp,"\n");
		fprintf(fp, "subframe %d slotnum %d paraid %d\n", subframenum, slotnum, paradeid);
		if (*subframenum <= 1) {
			fprintf(fp, "current_starting_group_number = %d \n", current_starting_group_number);
			fprintf(fp, "current_number_of_groups = %d \n", current_number_of_groups_minus_1+1);
			fprintf(fp, "current_rs_frame_mode = %d \n", current_rs_frame_mode);
			fprintf(fp, "current_rs_code_mode_primary = %d \n", current_rs_code_mode_primary);
			fprintf(fp, "current_rs_code_mode_secondary = %d \n", current_rs_code_mode_secondary);
			fprintf(fp, "current_sccc_block_mode = %d \n", current_sccc_block_mode);
			fprintf(fp, "current_TNoG = %d \n", current_TNoG);
		}
		fprintf(fp, "fic_version = %d \n", fic_version);
		fprintf(fp, "parade_continuity_counter = %d \n", parade_continuity_counter);
		fprintf(fp, "tpc_protocol_version = %d \n", tpc_protocol_version);	

		// 6. show result to the screen 
		fprintf(stderr, "TPC[0-10] : ");
		for (i=0;i<10;i++)
			fprintf(stderr, "%02X ", TPC[i]);
		fprintf(stderr,"\n");
		fprintf(stderr, "subframe %d slotnum %d paraid %d\n", subframenum, slotnum, paradeid);
		if (*subframenum <= 1) {
			fprintf(stderr, "current_starting_group_number = %d \n", current_starting_group_number);
			fprintf(stderr, "current_number_of_groups = %d \n", current_number_of_groups_minus_1+1);
			fprintf(stderr, "current_rs_frame_mode = %d \n", current_rs_frame_mode);
			fprintf(stderr, "current_rs_code_mode_primary = %d \n", current_rs_code_mode_primary);
			fprintf(stderr, "current_rs_code_mode_secondary = %d \n", current_rs_code_mode_secondary);
			fprintf(stderr, "current_sccc_block_mode = %d \n", current_sccc_block_mode);
			fprintf(stderr, "current_TNoG = %d \n", current_TNoG);
		}
		fprintf(stderr, "fic_version = %d \n", fic_version);
		fprintf(stderr, "parade_continuity_counter = %d \n", parade_continuity_counter);
		fprintf(stderr, "tpc_protocol_version = %d \n", tpc_protocol_version);
	}
	*/

	return 0;//NO_ERROR;
}

int CHldFmtrAtscMH::TL_GetStartPos(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize, int from_slot_0, unsigned char *tpc_info)
{
	DWORD	dwRemained = dwSize;
	int		index;
	WORD	wPID;
	int		iNoPkt = 0;

	int MHE_packet_PID = 0, MHE_packet_index = -1, MHE_packet_count = 0, i, slotnum_0_found = 0, slotnum_15_found = 0;
	m_MHE_packet_PID = 0;
	m_MHE_packet_index = -1;
	m_MHE_FIC_version = -1;
	m_MHE_TPC_protocol_version = -1;

	dwRemained -= iSyncStartPos;
	index = iSyncStartPos;
	
	while (dwRemained > 207)	
	{
		if (bData[index] == 0x47) 
		{
			wPID = (bData[index+1]&0x1F)*256+bData[index+2];

			if ( __Sta__->IsModTyp_AtscMH()/*ATSC-M/H*/ )
			{
				if ( MHE_packet_PID == -1 )
				{
					MHE_packet_PID = wPID;
				}

				if ( MHE_packet_PID != wPID )
				{
					MHE_packet_PID = wPID;
					MHE_packet_count = 1;
				}
				else
				{
					++MHE_packet_count;
				}
				MHE_packet_PID = wPID;
				
				if ( MHE_packet_count == 118 && (MHE_packet_PID != 0x1FFF)) 
				{
					m_MHE_packet_index = index - iTSize*(MHE_packet_count-1);
					m_MHE_packet_PID = (bData[m_MHE_packet_index+1]&0x1F)*256+bData[m_MHE_packet_index+2];

					unsigned char buf2[NUM_OF_PCCCDATA], subframenum, slotnum, paradeid, fic_version, tpc_protocol_version;
					unsigned char* buf1 = &bData[m_MHE_packet_index];
					for(i=0;i<NUM_OF_PCCCDATA;i++)
					{
						buf2[i] = buf1[((PKTSIZE*TPCOFFSET[i][0])+TPCOFFSET[i][1])];
					}

					if ( tpc_info == NULL )
					{
						TL_DecodeTPCInfo(buf2, &subframenum, &slotnum, &paradeid, &fic_version, &tpc_protocol_version, tpc_info);
						m_MHE_FIC_version = fic_version;
						m_MHE_TPC_protocol_version = tpc_protocol_version;

						if ( from_slot_0 == 1 )
						{
							if ( slotnum == 0 )
							{
								return m_MHE_packet_index;
							}
						}
						else
						{
							if ( slotnum == 0 && m_MHE_packet_index > (iTSize*(38+156)) )
							{
								return (m_MHE_packet_index - (iTSize*(38+156)));
							}
						}
					}
					else
					{
						if ( slotnum_0_found == 1 && slotnum_15_found == 1 )
						{
						}
						else
						{
							TL_DecodeTPCInfo(buf2, &subframenum, &slotnum, &paradeid, &fic_version, &tpc_protocol_version, tpc_info);
							m_MHE_FIC_version = fic_version;
							m_MHE_TPC_protocol_version = tpc_protocol_version;
						}

						if ( slotnum == 0 )
						{
							slotnum_0_found = 1;

							char str[] = "Slot number= 0\r\n";
							char *pdest;
							int result, len;

							len = strlen(str);
							pdest = strstr((char*)tpc_info, str);
							result = pdest - str + 1;

							if ( result > 0 )
							{
								strcpy((char*)tpc_info, pdest+len);
							}
						}
						else if ( slotnum_0_found == 1 && slotnum == 15 )
						{
							slotnum_15_found = 1;
						}
					}
				}
			}
		} 
		else  
		{
			break;
		}
		
		index += iTSize;
		iNoPkt++;
		dwRemained -= iTSize;
	}

	return 0;
}

int	CHldFmtrAtscMH::GetAtscMh_PktInfo(char *szFile, int iType)
{
	//ASI/310 Loop-thru
	if ( iType == 1 )
	{
		return m_MHE_packet_PID;
	}

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
	
	int iTSize, syncStart, mh_pid = -1; 
	unsigned long dwReadByte;
	unsigned char *szBuffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE);
	if ( !szBuffer )
	{
#if defined(WIN32)	
		CloseHandle(hFile);
#else
		fclose(hFile);
#endif
		return -1;
	}
	
#if defined(WIN32)
	ReadFile(hFile, szBuffer, SUB_BANK_MAX_BYTE_SIZE, &dwReadByte, NULL);
#else
	dwReadByte = fread(szBuffer, 1, SUB_BANK_MAX_BYTE_SIZE, hFile);
#endif
	if (dwReadByte <= 0)
	{
		return -1;
	}

	syncStart = __FIf__->TL_SyncLockFunction((char*)szBuffer, dwReadByte, &iTSize, dwReadByte, 3);
	if (syncStart != -1)
	{
		m_MHE_packet_PID = __FIf__->Get_MPE_Packet_PID(szBuffer, dwReadByte, syncStart, iTSize);
		mh_pid = m_MHE_packet_PID;
	}
	
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

	return mh_pid;
}

int	CHldFmtrAtscMH::RunAtscMhParser(char *szFile, char *szResult)
{
	FILE *fp;
	unsigned char *szBuf;
	int iTSize, SyncPos, StartPos;
	int numread, i, parade_id[127];
	char *pdest, str[32];
	char seps[] = "\t", *token;
	char szTemp[8192];
	int result, len;
	
	//2010/6/9
	memcpy(szTemp,"", 8192);
	
	fp = fopen(szFile, "rb");
	if ( !fp )
	{
		return -1;
	}
	//kslee 2010/6/7
	for(i = 0; i < 127 ; i++)
		parade_id[i] = 0;
	
	szBuf = (unsigned char *)malloc(MAX_BUFFER_BYTE_SIZE*sizeof(unsigned char));
	if ( !szBuf )
	{
		fclose(fp);
		return -2;
	}
	numread = fread(szBuf, 1, MAX_BUFFER_BYTE_SIZE, fp);
	fclose(fp);

	SyncPos = __FIf__->TL_SyncLockFunction((char*)szBuf, numread, &iTSize, numread, 3);
	if ( SyncPos >= 0 )
	{
		//kslee 2010/6/7
		//StartPos = TL_GetStartPos(szBuf, numread, SyncPos, iTSize, 0, (unsigned char *)szResult);

		StartPos = TL_GetStartPos(szBuf, numread, SyncPos, iTSize, 0, (unsigned char *)szTemp);
		if ( szTemp && strlen(szTemp) > 1 )
		{
			//kslee 2010/6/7			
			//memcpy(szBuf, szResult, strlen(szResult));
			//memset(szResult, 0x00, strlen(szResult));
			memcpy(szBuf, szTemp, strlen(szTemp));

			token = strtok((char*)szBuf, seps );
			while( token != NULL )
			{
				for ( i = 0; i < 127; i++ )
				{
					sprintf(str, "Parade ID= %d\r\n", i);
		
					len = strlen(str);
					pdest = strstr((char*)token, str);
					//2010/6/7					
					if(pdest == NULL)
					{
						continue;
					}					
					result = strlen(pdest) - strlen(str) + 1;
					
					if ( result > 0 && parade_id[i] != 1 )
					{
						parade_id[i] = 1;
						//2010/6/4
						CHK_ATSC_MH_PARAM_2(szResult, pdest)
						strcat(szResult, pdest);
						//2010/6/4
						if((strlen(szResult) + 2) >= 2048)
						{
							free(szBuf);
							return 0;
						}
						strcat(szResult, "\r\n");
						break;
					}
				}

				token = strtok( NULL, seps );
			}

			sprintf(debug_string, "FIC version= %d\r\n", m_MHE_FIC_version);
			//2010/6/4
			CHK_ATSC_MH_PARAM_2(szResult, debug_string)
			strcat(szResult, debug_string);

			sprintf(debug_string, "TPC Protocol version= %d",  m_MHE_TPC_protocol_version);
			//2010/6/4
			CHK_ATSC_MH_PARAM_2(szResult, debug_string)
			strcat(szResult, debug_string);
			m_MHE_TPC_information_validity = 1;
		}
		else
		{
			sprintf(debug_string, "!!!No TPC information found,\r\nInvalid ATSC M/H stream!!!\r\n");
			strcat(szResult, debug_string);
			m_MHE_TPC_information_validity = 0;
			free(szBuf);
			return 1;
		}
	}

	free(szBuf);

	return 0;
}




