//=================================================================	
//	decode_cif.c  /  Conversion Utility for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"cif.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

//=================================================================	
// Information and Data of a Frame
struct DATA_FRAME m_Frame;
struct FRAME_INFO m_FrameInfo;
struct SUB_STREAM_INFO m_SubInfo[64]; //  modified on 2010/06/08  struct SUB_STREAM_INFO m_SubInfo[16];

//=================================================================	
// 10 Energy Dispersal
unsigned char m_pPRBS[6912];	// PRBS Sequence
struct DATA_FRAME m_EnerDisp;

//=================================================================	
// 11 Convoutional Coding
struct UEP_PP m_pUepTable[64];
unsigned char m_pMothCode[255];
struct DATA_FRAME m_ConvEnco;

//=================================================================	
// 12 Time Interleaving
int m_pInxBuff = 0;						// Index for Current Buffer
struct DATA **m_pTimeIntBuff;     //  modified on 2010/06/08   struct DATA m_pTimeIntBuff[16][16];		// Buffer for Time Interleaving
struct DATA_FRAME m_TimeInt;					// Time Interleaved Data for Main Service Channel

//=================================================================	
// 13 Common Interleaved Frame
unsigned char *m_pCif = NULL;			// CIF Data

//=================================================================	
// 
int FramePhase = 0;
unsigned char WrBuff[7300];

void MyGenePrbs(unsigned char *PRBS)  // PRBS generator
{
	int i, j;

	bool FSR[9], tmFSR;
	unsigned char tmPRBS = 0;

  
	FSR[0] = true; // Initilize Feedback Shift Register
	FSR[1] = true;
	FSR[2] = true;
	FSR[3] = true;
	FSR[4] = true;

	FSR[5] = true;
	FSR[6] = true;
	FSR[7] = true;
	FSR[8] = true;

	for(i=0; i<6912; i++) {  // Generate PRBS 
		for(j=0; j<8; j++) {
			tmFSR = (FSR[0] != FSR[4]) ? true : false;
			FSR[0] = FSR[1];
			FSR[1] = FSR[2];
			FSR[2] = FSR[3];
			FSR[3] = FSR[4];
			FSR[4] = FSR[5];
			FSR[5] = FSR[6];
			FSR[6] = FSR[7];
			FSR[7] = FSR[8];
			FSR[8] = tmFSR;

			tmPRBS = tmFSR ? (tmPRBS << 1) | 0x01 : (tmPRBS << 1) & 0xfe;
		}

		PRBS[i] = tmPRBS;
	}
}

void MyGeneMoth(unsigned char *m_pMothCode) 
{
	m_pMothCode[0] = 0x00;	
	m_pMothCode[1] = 0x0f;
	m_pMothCode[2] = 0xf6;
	m_pMothCode[3] = 0xf9;
	m_pMothCode[4] = 0x6d;
	m_pMothCode[5] = 0x62;
	m_pMothCode[6] = 0x9b;
	m_pMothCode[7] = 0x94;
	m_pMothCode[8] = 0xdd;
	m_pMothCode[9] = 0xd2;
	m_pMothCode[10] = 0x2b;
	m_pMothCode[11] = 0x24;
	m_pMothCode[12] = 0xb0;
	m_pMothCode[13] = 0xbf;
	m_pMothCode[14] = 0x46;
	m_pMothCode[15] = 0x49;
	m_pMothCode[16] = 0xd2;
	m_pMothCode[17] = 0xdd;
	m_pMothCode[18] = 0x24;
	m_pMothCode[19] = 0x2b;
	m_pMothCode[20] = 0xbf;
	m_pMothCode[21] = 0xb0;
	m_pMothCode[22] = 0x49;
	m_pMothCode[23] = 0x46;
	m_pMothCode[24] = 0x0f;
	m_pMothCode[25] = 0x00;
	m_pMothCode[26] = 0xf9;
	m_pMothCode[27] = 0xf6;
	m_pMothCode[28] = 0x62;
	m_pMothCode[29] = 0x6d;
	m_pMothCode[30] = 0x94;
	m_pMothCode[31] = 0x9b;
	m_pMothCode[32] = 0x29;
	m_pMothCode[33] = 0x26;
	m_pMothCode[34] = 0xdf;
	m_pMothCode[35] = 0xd0;
	m_pMothCode[36] = 0x44;
	m_pMothCode[37] = 0x4b;
	m_pMothCode[38] = 0xb2;
	m_pMothCode[39] = 0xbd;
	m_pMothCode[40] = 0xf4;
	m_pMothCode[41] = 0xfb;
	m_pMothCode[42] = 0x02;
	m_pMothCode[43] = 0x0d;
	m_pMothCode[44] = 0x99;
	m_pMothCode[45] = 0x96;
	m_pMothCode[46] = 0x6f;
	m_pMothCode[47] = 0x60;
	m_pMothCode[48] = 0xfb;
	m_pMothCode[49] = 0xf4;
	m_pMothCode[50] = 0x0d;
	m_pMothCode[51] = 0x02;
	m_pMothCode[52] = 0x96;
	m_pMothCode[53] = 0x99;
	m_pMothCode[54] = 0x60;
	m_pMothCode[55] = 0x6f;
	m_pMothCode[56] = 0x26;
	m_pMothCode[57] = 0x29;
	m_pMothCode[58] = 0xd0;
	m_pMothCode[59] = 0xdf;
	m_pMothCode[60] = 0x4b;
	m_pMothCode[61] = 0x44;
	m_pMothCode[62] = 0xbd;
	m_pMothCode[63] = 0xb2;
	m_pMothCode[64] = 0x9f;
	m_pMothCode[65] = 0x90;
	m_pMothCode[66] = 0x69;
	m_pMothCode[67] = 0x66;
	m_pMothCode[68] = 0xf2;
	m_pMothCode[69] = 0xfd;
	m_pMothCode[70] = 0x04;
	m_pMothCode[71] = 0x0b;
	m_pMothCode[72] = 0x42;
	m_pMothCode[73] = 0x4d;
	m_pMothCode[74] = 0xb4;
	m_pMothCode[75] = 0xbb;
	m_pMothCode[76] = 0x2f;
	m_pMothCode[77] = 0x20;
	m_pMothCode[78] = 0xd9;
	m_pMothCode[79] = 0xd6;
	m_pMothCode[80] = 0x4d;
	m_pMothCode[81] = 0x42;
	m_pMothCode[82] = 0xbb;
	m_pMothCode[83] = 0xb4;
	m_pMothCode[84] = 0x20;
	m_pMothCode[85] = 0x2f;
	m_pMothCode[86] = 0xd6;
	m_pMothCode[87] = 0xd9;
	m_pMothCode[88] = 0x90;
	m_pMothCode[89] = 0x9f;
	m_pMothCode[90] = 0x66;
	m_pMothCode[91] = 0x69;
	m_pMothCode[92] = 0xfd;
	m_pMothCode[93] = 0xf2;
	m_pMothCode[94] = 0x0b;
	m_pMothCode[95] = 0x04;
	m_pMothCode[96] = 0xb6;
	m_pMothCode[97] = 0xb9;
	m_pMothCode[98] = 0x40;
	m_pMothCode[99] = 0x4f;
	m_pMothCode[100] = 0xdb;
	m_pMothCode[101] = 0xd4;
	m_pMothCode[102] = 0x2d;
	m_pMothCode[103] = 0x22;
	m_pMothCode[104] = 0x6b;
	m_pMothCode[105] = 0x64;
	m_pMothCode[106] = 0x9d;
	m_pMothCode[107] = 0x92;
	m_pMothCode[108] = 0x06;
	m_pMothCode[109] = 0x09;
	m_pMothCode[110] = 0xf0;
	m_pMothCode[111] = 0xff;
	m_pMothCode[112] = 0x64;
	m_pMothCode[113] = 0x6b;
	m_pMothCode[114] = 0x92;
	m_pMothCode[115] = 0x9d;
	m_pMothCode[116] = 0x09;
	m_pMothCode[117] = 0x06;
	m_pMothCode[118] = 0xff;
	m_pMothCode[119] = 0xf0;
	m_pMothCode[120] = 0xb9;
	m_pMothCode[121] = 0xb6;
	m_pMothCode[122] = 0x4f;
	m_pMothCode[123] = 0x40;
	m_pMothCode[124] = 0xd4;
	m_pMothCode[125] = 0xdb;
	m_pMothCode[126] = 0x22;
	m_pMothCode[127] = 0x2d;
	m_pMothCode[128] = 0xf0;
	m_pMothCode[129] = 0xff;
	m_pMothCode[130] = 0x06;
	m_pMothCode[131] = 0x09;
	m_pMothCode[132] = 0x9d;
	m_pMothCode[133] = 0x92;
	m_pMothCode[134] = 0x6b;
	m_pMothCode[135] = 0x64;
	m_pMothCode[136] = 0x2d;
	m_pMothCode[137] = 0x22;
	m_pMothCode[138] = 0xdb;
	m_pMothCode[139] = 0xd4;
	m_pMothCode[140] = 0x40;
	m_pMothCode[141] = 0x4f;
	m_pMothCode[142] = 0xb6;
	m_pMothCode[143] = 0xb9;
	m_pMothCode[144] = 0x22;
	m_pMothCode[145] = 0x2d;
	m_pMothCode[146] = 0xd4;
	m_pMothCode[147] = 0xdb;
	m_pMothCode[148] = 0x4f;
	m_pMothCode[149] = 0x40;
	m_pMothCode[150] = 0xb9;
	m_pMothCode[151] = 0xb6;
	m_pMothCode[152] = 0xff;
	m_pMothCode[153] = 0xf0;
	m_pMothCode[154] = 0x09;
	m_pMothCode[155] = 0x06;
	m_pMothCode[156] = 0x92;
	m_pMothCode[157] = 0x9d;
	m_pMothCode[158] = 0x64;
	m_pMothCode[159] = 0x6b;
	m_pMothCode[160] = 0xd9;
	m_pMothCode[161] = 0xd6;
	m_pMothCode[162] = 0x2f;
	m_pMothCode[163] = 0x20;
	m_pMothCode[164] = 0xb4;
	m_pMothCode[165] = 0xbb;
	m_pMothCode[166] = 0x42;
	m_pMothCode[167] = 0x4d;
	m_pMothCode[168] = 0x04;
	m_pMothCode[169] = 0x0b;
	m_pMothCode[170] = 0xf2;
	m_pMothCode[171] = 0xfd;
	m_pMothCode[172] = 0x69;
	m_pMothCode[173] = 0x66;
	m_pMothCode[174] = 0x9f;
	m_pMothCode[175] = 0x90;
	m_pMothCode[176] = 0x0b;
	m_pMothCode[177] = 0x04;
	m_pMothCode[178] = 0xfd;
	m_pMothCode[179] = 0xf2;
	m_pMothCode[180] = 0x66;
	m_pMothCode[181] = 0x69;
	m_pMothCode[182] = 0x90;
	m_pMothCode[183] = 0x9f;
	m_pMothCode[184] = 0xd6;
	m_pMothCode[185] = 0xd9;
	m_pMothCode[186] = 0x20;
	m_pMothCode[187] = 0x2f;
	m_pMothCode[188] = 0xbb;
	m_pMothCode[189] = 0xb4;
	m_pMothCode[190] = 0x4d;
	m_pMothCode[191] = 0x42;
	m_pMothCode[192] = 0x6f;
	m_pMothCode[193] = 0x60;
	m_pMothCode[194] = 0x99;
	m_pMothCode[195] = 0x96;
	m_pMothCode[196] = 0x02;
	m_pMothCode[197] = 0x0d;
	m_pMothCode[198] = 0xf4;
	m_pMothCode[199] = 0xfb;
	m_pMothCode[200] = 0xb2;
	m_pMothCode[201] = 0xbd;
	m_pMothCode[202] = 0x44;
	m_pMothCode[203] = 0x4b;
	m_pMothCode[204] = 0xdf;
	m_pMothCode[205] = 0xd0;
	m_pMothCode[206] = 0x29;
	m_pMothCode[207] = 0x26;
	m_pMothCode[208] = 0xbd;
	m_pMothCode[209] = 0xb2;
	m_pMothCode[210] = 0x4b;
	m_pMothCode[211] = 0x44;
	m_pMothCode[212] = 0xd0;
	m_pMothCode[213] = 0xdf;
	m_pMothCode[214] = 0x26;
	m_pMothCode[215] = 0x29;
	m_pMothCode[216] = 0x60;
	m_pMothCode[217] = 0x6f;
	m_pMothCode[218] = 0x96;
	m_pMothCode[219] = 0x99;
	m_pMothCode[220] = 0x0d;
	m_pMothCode[221] = 0x02;
	m_pMothCode[222] = 0xfb;
	m_pMothCode[223] = 0xf4;
	m_pMothCode[224] = 0x46;
	m_pMothCode[225] = 0x49;
	m_pMothCode[226] = 0xb0;
	m_pMothCode[227] = 0xbf;
	m_pMothCode[228] = 0x2b;
	m_pMothCode[229] = 0x24;
	m_pMothCode[230] = 0xdd;
	m_pMothCode[231] = 0xd2;
	m_pMothCode[232] = 0x9b;
	m_pMothCode[233] = 0x94;
	m_pMothCode[234] = 0x6d;
	m_pMothCode[235] = 0x62;
	m_pMothCode[236] = 0xf6;
	m_pMothCode[237] = 0xf9;
	m_pMothCode[238] = 0x00;
	m_pMothCode[239] = 0x0f;
	m_pMothCode[240] = 0x94;
	m_pMothCode[241] = 0x9b;
	m_pMothCode[242] = 0x62;
	m_pMothCode[243] = 0x6d;
	m_pMothCode[244] = 0xf9;
	m_pMothCode[245] = 0xf6;
	m_pMothCode[246] = 0x0f;
	m_pMothCode[247] = 0x00;
	m_pMothCode[248] = 0x49;
	m_pMothCode[249] = 0x46;
	m_pMothCode[250] = 0xbf;
	m_pMothCode[251] = 0xb0;
	m_pMothCode[252] = 0x24;
	m_pMothCode[253] = 0x2b;
	m_pMothCode[254] = 0xd2;
	m_pMothCode[255] = 0xdd;
}

void MyUepTableGene(struct UEP_PP *UepTable)
{
	// Audio bit rate = 32
	UepTable[0].L1 = 3;		UepTable[0].L2 = 4;		UepTable[0].L3 = 17;	UepTable[0].L4 = 0;
	UepTable[0].PI1 = 5;	UepTable[0].PI2 = 3;	UepTable[0].PI3 = 2;	UepTable[0].PI4 = 0;
	UepTable[0].PAD_BIT = 0;

	UepTable[1].L1 = 3;		UepTable[1].L2 = 3;		UepTable[1].L3 = 18;	UepTable[1].L4 = 0;
	UepTable[1].PI1 = 11;	UepTable[1].PI2 = 6;	UepTable[1].PI3 = 5;	UepTable[1].PI4 = 0;
	UepTable[1].PAD_BIT = 0;

	UepTable[2].L1 = 3;		UepTable[2].L2 = 4;		UepTable[2].L3 = 14;	UepTable[2].L4 = 3;
	UepTable[2].PI1 = 15;	UepTable[2].PI2 = 9;	UepTable[2].PI3 = 6;	UepTable[2].PI4 = 8;
	UepTable[2].PAD_BIT = 0;

	UepTable[3].L1 = 3;		UepTable[3].L2 = 4;		UepTable[3].L3 = 14;	UepTable[3].L4 = 3;
	UepTable[3].PI1 = 22;	UepTable[3].PI2 = 23;	UepTable[3].PI3 = 8;	UepTable[3].PI4 = 13;
	UepTable[3].PAD_BIT = 0;

	UepTable[4].L1 = 3;		UepTable[4].L2 = 5;		UepTable[4].L3 = 13;	UepTable[4].L4 = 3;
	UepTable[4].PI1 = 24;	UepTable[4].PI2 = 17;	UepTable[4].PI3 = 12;	UepTable[4].PI4 = 17;
	UepTable[4].PAD_BIT = 4;

	// Audio bit rate = 64
	UepTable[5].L1 = 4;		UepTable[5].L2 = 3;		UepTable[5].L3 = 26;	UepTable[5].L4 = 3;
	UepTable[5].PI1 = 5;	UepTable[5].PI2 = 4;	UepTable[5].PI3 = 2;	UepTable[5].PI4 = 3;
	UepTable[5].PAD_BIT = 0;

	UepTable[6].L1 = 3;		UepTable[6].L2 = 4;		UepTable[6].L3 = 26;	UepTable[6].L4 = 3;
	UepTable[6].PI1 = 9;	UepTable[6].PI2 = 6;	UepTable[6].PI3 = 4;	UepTable[6].PI4 = 6;
	UepTable[6].PAD_BIT = 0;

	UepTable[7].L1 = 3;		UepTable[7].L2 = 4;		UepTable[7].L3 = 26;	UepTable[7].L4 = 3;
	UepTable[7].PI1 = 15;	UepTable[7].PI2 = 10;	UepTable[7].PI3 = 6;	UepTable[7].PI4 = 9;
	UepTable[7].PAD_BIT = 4;

	UepTable[8].L1 = 3;		UepTable[8].L2 = 4;		UepTable[8].L3 = 26;	UepTable[8].L4 = 3;
	UepTable[8].PI1 = 24;	UepTable[8].PI2 = 14;	UepTable[8].PI3 = 8;	UepTable[8].PI4 = 15;
	UepTable[8].PAD_BIT = 0;

	UepTable[9].L1 = 3;		UepTable[9].L2 = 5;		UepTable[9].L3 = 25;	UepTable[9].L4 = 3;
	UepTable[9].PI1 = 24;	UepTable[9].PI2 = 18;	UepTable[9].PI3 = 13;	UepTable[9].PI4 = 18;
	UepTable[9].PAD_BIT = 0;

	// Audio bit rate = 56
	UepTable[10].L1 = 6;	UepTable[10].L2 = 10;	UepTable[10].L3 = 23;	UepTable[10].L4 = 3;
	UepTable[10].PI1 = 5;	UepTable[10].PI2 = 4;	UepTable[10].PI3 = 2;	UepTable[10].PI4 = 3;
	UepTable[10].PAD_BIT = 0;

	UepTable[11].L1 = 6;	UepTable[11].L2 = 10;	UepTable[11].L3 = 23;	UepTable[11].L4 = 3;
	UepTable[11].PI1 = 9;	UepTable[11].PI2 = 6;	UepTable[11].PI3 = 4;	UepTable[11].PI4 = 5;
	UepTable[11].PAD_BIT = 0;

	UepTable[12].L1 = 6;	UepTable[12].L2 = 12;	UepTable[12].L3 = 23;	UepTable[12].L4 = 3;
	UepTable[12].PI1 = 23;	UepTable[12].PI2 = 13;	UepTable[12].PI3 = 8;	UepTable[12].PI4 = 13;
	UepTable[12].PAD_BIT = 0;

	UepTable[13].L1 = 6;	UepTable[13].L2 = 10;	UepTable[13].L3 = 14;	UepTable[13].L4 = 3;
	UepTable[13].PI1 = 15;	UepTable[13].PI2 = 9;	UepTable[13].PI3 = 6;	UepTable[13].PI4 = 8;
	UepTable[13].PAD_BIT = 8;

	// Audio bit rate = 64
	UepTable[14].L1 = 6;	UepTable[14].L2 = 9;	UepTable[14].L3 = 31;	UepTable[14].L4 = 2;
	UepTable[14].PI1 = 5;	UepTable[14].PI2 = 3;	UepTable[14].PI3 = 2;	UepTable[14].PI4 = 3;
	UepTable[14].PAD_BIT = 0;

	UepTable[15].L1 = 6;	UepTable[15].L2 = 9;	UepTable[15].L3 = 33;	UepTable[15].L4 = 0;
	UepTable[15].PI1 = 11;	UepTable[15].PI2 = 6;	UepTable[15].PI3 = 5;	UepTable[15].PI4 = 0;
	UepTable[15].PAD_BIT = 0;

	UepTable[16].L1 = 6;	UepTable[16].L2 = 12;	UepTable[16].L3 = 27;	UepTable[16].L4 = 3;
	UepTable[16].PI1 = 16;	UepTable[16].PI2 = 8;	UepTable[16].PI3 = 6;	UepTable[16].PI4 = 9;
	UepTable[16].PAD_BIT = 0;

	UepTable[17].L1 = 6;	UepTable[17].L2 = 10;	UepTable[17].L3 = 29;	UepTable[17].L4 = 3;
	UepTable[17].PI1 = 23;	UepTable[17].PI2 = 13;	UepTable[17].PI3 = 8;	UepTable[17].PI4 = 13;
	UepTable[17].PAD_BIT = 8;

	UepTable[18].L1 = 6;	UepTable[18].L2 = 11;	UepTable[18].L3 = 28;	UepTable[18].L4 = 3;
	UepTable[18].PI1 = 24;	UepTable[18].PI2 = 18;	UepTable[18].PI3 = 12;	UepTable[18].PI4 = 18;
	UepTable[18].PAD_BIT = 4;

	// Audio bit rate = 80
	UepTable[19].L1 = 6;	UepTable[19].L2 = 10;	UepTable[19].L3 = 41;	UepTable[19].L4 = 3;
	UepTable[19].PI1 = 6;	UepTable[19].PI2 = 3;	UepTable[19].PI3 = 2;	UepTable[19].PI4 = 3;
	UepTable[19].PAD_BIT = 0;

	UepTable[20].L1 = 6;	UepTable[20].L2 = 10;	UepTable[20].L3 = 41;	UepTable[20].L4 = 3;
	UepTable[20].PI1 = 11;	UepTable[20].PI2 = 6;	UepTable[20].PI3 = 5;	UepTable[20].PI4 = 6;
	UepTable[20].PAD_BIT = 0;

	UepTable[21].L1 = 6;	UepTable[21].L2 = 11;	UepTable[21].L3 = 40;	UepTable[21].L4 = 3;
	UepTable[21].PI1 = 16;	UepTable[21].PI2 = 8;	UepTable[21].PI3 = 6;	UepTable[21].PI4 = 7;
	UepTable[21].PAD_BIT = 0;

	UepTable[22].L1 = 6;	UepTable[22].L2 = 10;	UepTable[22].L3 = 41;	UepTable[22].L4 = 3;
	UepTable[22].PI1 = 23;	UepTable[22].PI2 = 13;	UepTable[22].PI3 = 8;	UepTable[22].PI4 = 13;
	UepTable[22].PAD_BIT = 8;

	UepTable[23].L1 = 6;	UepTable[23].L2 = 10;	UepTable[23].L3 = 41;	UepTable[23].L4 = 3;
	UepTable[23].PI1 = 24;	UepTable[23].PI2 = 17;	UepTable[23].PI3 = 12;	UepTable[23].PI4 = 18;
	UepTable[23].PAD_BIT = 4;

	// Audio bit rate = 96
	UepTable[24].L1 = 7;	UepTable[24].L2 = 9;	UepTable[24].L3 = 53;	UepTable[24].L4 = 3;
	UepTable[24].PI1 = 5;	UepTable[24].PI2 = 4;	UepTable[24].PI3 = 2;	UepTable[24].PI4 = 4;
	UepTable[24].PAD_BIT = 0;

	UepTable[25].L1 = 7;	UepTable[25].L2 = 10;	UepTable[25].L3 = 52;	UepTable[25].L4 = 3;
	UepTable[25].PI1 = 9;	UepTable[25].PI2 = 6;	UepTable[25].PI3 = 4;	UepTable[25].PI4 = 6;
	UepTable[25].PAD_BIT = 0;

	UepTable[26].L1 = 6;	UepTable[26].L2 = 12;	UepTable[26].L3 = 51;	UepTable[26].L4 = 3;
	UepTable[26].PI1 = 16;	UepTable[26].PI2 = 9;	UepTable[26].PI3 = 6;	UepTable[26].PI4 = 10;
	UepTable[26].PAD_BIT = 4;

	UepTable[27].L1 = 6;	UepTable[27].L2 = 10;	UepTable[27].L3 = 53;	UepTable[27].L4 = 3;
	UepTable[27].PI1 = 22;	UepTable[27].PI2 = 12;	UepTable[27].PI3 = 9;	UepTable[27].PI4 = 12;
	UepTable[27].PAD_BIT = 0;

	UepTable[28].L1 = 6;		UepTable[28].L2 = 13;	UepTable[28].L3 = 50;	UepTable[28].L4 = 3;
	UepTable[28].PI1 = 24;	UepTable[28].PI2 = 18;	UepTable[28].PI3 = 13;	UepTable[28].PI4 = 19;
	UepTable[28].PAD_BIT = 0;

	// Audio bit rate = 112
	UepTable[29].L1 = 14;	UepTable[29].L2 = 17;	UepTable[29].L3 = 50;	UepTable[29].L4 = 3;
	UepTable[29].PI1 = 5;	UepTable[29].PI2 = 4;	UepTable[29].PI3 = 2;	UepTable[29].PI4 = 5;
	UepTable[29].PAD_BIT = 0;

	UepTable[30].L1 = 11;	UepTable[30].L2 = 21;	UepTable[30].L3 = 49;	UepTable[30].L4 = 3;
	UepTable[30].PI1 = 9;	UepTable[30].PI2 = 6;	UepTable[30].PI3 = 4;	UepTable[30].PI4 = 8;
	UepTable[30].PAD_BIT = 0;

	UepTable[31].L1 = 11;	UepTable[31].L2 = 23;	UepTable[31].L3 = 47;	UepTable[31].L4 = 3;
	UepTable[31].PI1 = 16;	UepTable[31].PI2 = 8;	UepTable[31].PI3 = 6;	UepTable[31].PI4 = 9;
	UepTable[31].PAD_BIT = 0;

	UepTable[32].L1 = 11;	UepTable[32].L2 = 21;	UepTable[32].L3 = 49;	UepTable[32].L4 = 3;
	UepTable[32].PI1 = 23;	UepTable[32].PI2 = 12;	UepTable[32].PI3 = 9;	UepTable[32].PI4 = 14;
	UepTable[32].PAD_BIT = 4;
	
	// Audio bit rate = 128
	UepTable[33].L1 = 12;	UepTable[33].L2 = 19;	UepTable[33].L3 = 62;	UepTable[33].L4 = 3;
	UepTable[33].PI1 = 5;	UepTable[33].PI2 = 3;	UepTable[33].PI3 = 2;	UepTable[33].PI4 = 4;
	UepTable[33].PAD_BIT = 0;

	UepTable[34].L1 = 11;	UepTable[34].L2 = 21;	UepTable[34].L3 = 61;	UepTable[34].L4 = 3;
	UepTable[34].PI1 = 11;	UepTable[34].PI2 = 6;	UepTable[34].PI3 = 5;	UepTable[34].PI4 = 7;
	UepTable[34].PAD_BIT = 0;

	UepTable[35].L1 = 11;	UepTable[35].L2 = 22;	UepTable[35].L3 = 60;	UepTable[35].L4 = 3;
	UepTable[35].PI1 = 16;	UepTable[35].PI2 = 9;	UepTable[35].PI3 = 6;	UepTable[35].PI4 = 10;
	UepTable[35].PAD_BIT = 4;

	UepTable[36].L1 = 11;	UepTable[36].L2 = 21;	UepTable[36].L3 = 61;	UepTable[36].L4 = 3;
	UepTable[36].PI1 = 22;	UepTable[36].PI2 = 12;	UepTable[36].PI3 = 9;	UepTable[36].PI4 = 14;
	UepTable[36].PAD_BIT = 0;

	UepTable[37].L1 = 11;	UepTable[37].L2 = 20;	UepTable[37].L3 = 62;	UepTable[37].L4 = 3;
	UepTable[37].PI1 = 24;	UepTable[37].PI2 = 17;	UepTable[37].PI3 = 13;	UepTable[37].PI4 = 19;
	UepTable[37].PAD_BIT = 8;

	// Audio bit rate = 160
	UepTable[38].L1 = 11;	UepTable[38].L2 = 19;	UepTable[38].L3 = 87;	UepTable[38].L4 = 3;
	UepTable[38].PI1 = 5;	UepTable[38].PI2 = 4;	UepTable[38].PI3 = 2;	UepTable[38].PI4 = 4;
	UepTable[38].PAD_BIT = 0;

	UepTable[39].L1 = 11;	UepTable[39].L2 = 23;	UepTable[39].L3 = 83;	UepTable[39].L4 = 3;
	UepTable[39].PI1 = 11;	UepTable[39].PI2 = 6;	UepTable[39].PI3 = 5;	UepTable[39].PI4 = 9;
	UepTable[39].PAD_BIT = 0;

	UepTable[40].L1 = 11;	UepTable[40].L2 = 24;	UepTable[40].L3 = 82;	UepTable[40].L4 = 3;
	UepTable[40].PI1 = 16;	UepTable[40].PI2 = 8;	UepTable[40].PI3 = 6;	UepTable[40].PI4 = 11;
	UepTable[40].PAD_BIT = 4;

	UepTable[41].L1 = 11;	UepTable[41].L2 = 21;	UepTable[41].L3 = 85;	UepTable[41].L4 = 3;
	UepTable[41].PI1 = 22;	UepTable[41].PI2 = 11;	UepTable[41].PI3 = 9;	UepTable[41].PI4 = 13;
	UepTable[41].PAD_BIT = 0;

	UepTable[42].L1 = 11;	UepTable[42].L2 = 22;	UepTable[42].L3 = 84;	UepTable[42].L4 = 3;
	UepTable[42].PI1 = 24;	UepTable[42].PI2 = 18;	UepTable[42].PI3 = 12;	UepTable[42].PI4 = 19;
	UepTable[42].PAD_BIT = 0;

	// Audio bit rate = 192
	UepTable[43].L1 = 11;	UepTable[43].L2 = 20;	UepTable[43].L3 = 110;	UepTable[43].L4 = 3;
	UepTable[43].PI1 = 6;	UepTable[43].PI2 = 4;	UepTable[43].PI3 = 2;	UepTable[43].PI4 = 5;
	UepTable[43].PAD_BIT = 0;

	UepTable[44].L1 = 11;	UepTable[44].L2 = 22;	UepTable[44].L3 = 108;	UepTable[44].L4 = 3;
	UepTable[44].PI1 = 10;	UepTable[44].PI2 = 6;	UepTable[44].PI3 = 4;	UepTable[44].PI4 = 9;
	UepTable[44].PAD_BIT = 0;

	UepTable[45].L1 = 11;	UepTable[45].L2 = 24;	UepTable[45].L3 = 106;	UepTable[45].L4 = 3;
	UepTable[45].PI1 = 16;	UepTable[45].PI2 = 10;	UepTable[45].PI3 = 6;	UepTable[45].PI4 = 11;
	UepTable[45].PAD_BIT = 0;

	UepTable[46].L1 = 11;	UepTable[46].L2 = 20;	UepTable[46].L3 = 110;	UepTable[46].L4 = 3;
	UepTable[46].PI1 = 22;	UepTable[46].PI2 = 13;	UepTable[46].PI3 = 9;	UepTable[46].PI4 = 13;
	UepTable[46].PAD_BIT = 8;

	UepTable[47].L1 = 11;	UepTable[47].L2 = 21;	UepTable[47].L3 = 109;	UepTable[47].L4 = 3;
	UepTable[47].PI1 = 24;	UepTable[47].PI2 = 20;	UepTable[47].PI3 = 13;	UepTable[47].PI4 = 24;
	UepTable[47].PAD_BIT = 0;

	// Audio bit rate = 224
	UepTable[48].L1 = 12;	UepTable[48].L2 = 22;	UepTable[48].L3 = 131;	UepTable[48].L4 = 3;
	UepTable[48].PI1 = 8;	UepTable[48].PI2 = 6;	UepTable[48].PI3 = 2;	UepTable[48].PI4 = 6;
	UepTable[48].PAD_BIT = 4;

	UepTable[49].L1 = 12;	UepTable[49].L2 = 26;	UepTable[49].L3 = 127;	UepTable[49].L4 = 3;
	UepTable[49].PI1 = 12;	UepTable[49].PI2 = 8;	UepTable[49].PI3 = 4;	UepTable[49].PI4 = 11;
	UepTable[49].PAD_BIT = 0;

	UepTable[50].L1 = 11;	UepTable[50].L2 = 20;	UepTable[50].L3 = 134;	UepTable[50].L4 = 3;
	UepTable[50].PI1 = 16;	UepTable[50].PI2 = 10;	UepTable[50].PI3 = 7;	UepTable[50].PI4 = 9;
	UepTable[50].PAD_BIT = 0;

	UepTable[51].L1 = 11;	UepTable[51].L2 = 22;	UepTable[51].L3 = 132;	UepTable[51].L4 = 3;
	UepTable[51].PI1 = 24;	UepTable[51].PI2 = 16;	UepTable[51].PI3 = 10;	UepTable[51].PI4 = 15;
	UepTable[51].PAD_BIT = 0;

	UepTable[52].L1 = 11;	UepTable[52].L2 = 24;	UepTable[52].L3 = 130;	UepTable[52].L4 = 3;
	UepTable[52].PI1 = 24;	UepTable[52].PI2 = 20;	UepTable[52].PI3 = 12;	UepTable[52].PI4 = 20;
	UepTable[52].PAD_BIT = 4;

	// Audio bit rate = 256
	UepTable[53].L1 = 11;	UepTable[53].L2 = 24;	UepTable[53].L3 = 154;	UepTable[53].L4 = 3;
	UepTable[53].PI1 = 6;	UepTable[53].PI2 = 5;	UepTable[53].PI3 = 2;	UepTable[53].PI4 = 5;
	UepTable[53].PAD_BIT = 0;

	UepTable[54].L1 = 11;	UepTable[54].L2 = 24;	UepTable[54].L3 = 154;	UepTable[54].L4 = 3;
	UepTable[54].PI1 = 12;	UepTable[54].PI2 = 9;	UepTable[54].PI3 = 5;	UepTable[54].PI4 = 10;
	UepTable[54].PAD_BIT = 4;

	UepTable[55].L1 = 11;	UepTable[55].L2 = 27;	UepTable[55].L3 = 151;	UepTable[55].L4 = 3;
	UepTable[55].PI1 = 16;	UepTable[55].PI2 = 10;	UepTable[55].PI3 = 7;	UepTable[55].PI4 = 10;
	UepTable[55].PAD_BIT = 0;

	UepTable[56].L1 = 11;	UepTable[56].L2 = 22;	UepTable[56].L3 = 156;	UepTable[56].L4 = 3;
	UepTable[56].PI1 = 24;	UepTable[56].PI2 = 14;	UepTable[56].PI3 = 10;	UepTable[56].PI4 = 13;
	UepTable[56].PAD_BIT = 8;

	UepTable[57].L1 = 11;	UepTable[57].L2 = 26;	UepTable[57].L3 = 152;	UepTable[57].L4 = 3;
	UepTable[57].PI1 = 24;	UepTable[57].PI2 = 19;	UepTable[57].PI3 = 14;	UepTable[57].PI4 = 18;
	UepTable[57].PAD_BIT = 4;

	// Audio bit rate = 320
	UepTable[58].L1 = 11;	UepTable[58].L2 = 26;	UepTable[58].L3 = 200;	UepTable[58].L4 = 3;
	UepTable[58].PI1 = 8;	UepTable[58].PI2 = 5;	UepTable[58].PI3 = 2;	UepTable[58].PI4 = 6;
	UepTable[58].PAD_BIT = 4;

	UepTable[59].L1 = 11;	UepTable[59].L2 = 25;	UepTable[59].L3 = 201;	UepTable[59].L4 = 3;
	UepTable[59].PI1 = 13;	UepTable[59].PI2 = 9;	UepTable[59].PI3 = 5;	UepTable[59].PI4 = 10;
	UepTable[59].PAD_BIT = 8;

	UepTable[60].L1 = 11;	UepTable[60].L2 = 26;	UepTable[60].L3 = 200;	UepTable[60].L4 = 3;
	UepTable[60].PI1 = 24;	UepTable[60].PI2 = 17;	UepTable[60].PI3 = 9;	UepTable[60].PI4 = 17;
	UepTable[60].PAD_BIT = 0;

	// Audio bit rate = 384
	UepTable[61].L1 = 11;	UepTable[61].L2 = 27;	UepTable[61].L3 = 247;	UepTable[61].L4 = 3;
	UepTable[61].PI1 = 8;	UepTable[61].PI2 = 6;	UepTable[61].PI3 = 2;	UepTable[61].PI4 = 7;
	UepTable[61].PAD_BIT = 0;

	UepTable[62].L1 = 11;	UepTable[62].L2 = 24;	UepTable[62].L3 = 250;	UepTable[62].L4 = 3;
	UepTable[62].PI1 = 16;	UepTable[62].PI2 = 9;	UepTable[62].PI3 = 7;	UepTable[62].PI4 = 10;
	UepTable[62].PAD_BIT = 4;

	UepTable[63].L1 = 12;	UepTable[63].L2 = 28;	UepTable[63].L3 = 245;	UepTable[63].L4 = 3;
	UepTable[63].PI1 = 24;	UepTable[63].PI2 = 20;	UepTable[63].PI3 = 14;	UepTable[63].PI4 = 23;
	UepTable[63].PAD_BIT = 0;
}

void InitMyCifBuf()
{
	int	i;
	
	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_Frame.Msc[i].Data = NULL;
	}
	m_Frame.Fic.Data = NULL;

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_EnerDisp.Msc[i].Data = NULL;
	}
	m_EnerDisp.Fic.Data = NULL;
	
	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++)
	{
		m_ConvEnco.Msc[i].Data = NULL;
	}
	m_ConvEnco.Fic.Data = NULL;

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_TimeInt.Msc[i].Data = NULL;
	}
	m_TimeInt.Fic.Data = NULL;

	m_pTimeIntBuff = NULL;
	m_pCif = NULL;
}

void MyFreeFunc()  // Free all allocated memory
{
	int i, j;
	
	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		if ( m_Frame.Msc[i].Data )
		{
			free(m_Frame.Msc[i].Data);
			m_Frame.Msc[i].Data = NULL;
		}
	}
	if ( m_Frame.Fic.Data )
	{
		free(m_Frame.Fic.Data);
		m_Frame.Fic.Data = NULL;
	}

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		if ( m_EnerDisp.Msc[i].Data )
		{
			free(m_EnerDisp.Msc[i].Data);
			m_EnerDisp.Msc[i].Data = NULL;
		}
	}
	if ( m_EnerDisp.Fic.Data )
	{
		free(m_EnerDisp.Fic.Data);
		m_EnerDisp.Fic.Data = NULL;
	}
	
	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++)
	{
		if ( m_ConvEnco.Msc[i].Data )
		{
			free(m_ConvEnco.Msc[i].Data);
			m_ConvEnco.Msc[i].Data = NULL;
		}
	}
	if ( m_ConvEnco.Fic.Data )
	{
		free(m_ConvEnco.Fic.Data);
		m_ConvEnco.Fic.Data = NULL;
	}

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		if ( m_TimeInt.Msc[i].Data )
		{
			free(m_TimeInt.Msc[i].Data);
			m_TimeInt.Msc[i].Data = NULL;
		}
	}

	
	for(i=0; i<16; i++) 
	{
		for(j=0; j<64; j++)   //  modified on 2010/06/08  for(j=0; j<16; j++) 
		{
			if ( m_pTimeIntBuff && m_pTimeIntBuff[i][j].Data ) //sskim
			{
				free(m_pTimeIntBuff[i][j].Data);
				m_pTimeIntBuff[i][j].Data = NULL;
			}
		}
	}
	

	if ( m_pCif )
	{
		free(m_pCif);
		m_pCif = NULL;
	}
}

int MyInitFunc()
{
	int i, j, k;
	
	////////////////////////
	/* init data struct/variables */
	m_Frame.Fic.Data = (unsigned char *) malloc(128);
	if ( !m_Frame.Fic.Data )
		goto error_0;

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_Frame.Msc[i].Data = (unsigned char *) malloc(5472);
		if ( !m_Frame.Msc[i].Data )
			goto error_0;
	}

	// Output Data of Energy Dispersal
	m_EnerDisp.Fic.Data = (unsigned char *) malloc(128);
	if ( !m_EnerDisp.Fic.Data )
		goto error_0;

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_EnerDisp.Msc[i].Data = (unsigned char *) malloc(5472);
		if ( !m_EnerDisp.Msc[i].Data )
			goto error_0;
	}

	// Output Data of Convolutional Coding
	m_ConvEnco.Fic.Data = (unsigned char *) malloc(384);
	if ( !m_ConvEnco.Fic.Data )
		goto error_0;

	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++)
	{
		//sskim20061117 - bug fix
		m_ConvEnco.Msc[i].Data = (unsigned char *) malloc(20736);
		if ( !m_ConvEnco.Msc[i].Data )
			goto error_0;
	}

	// Output Data of Time Interleaving 
	for(i=0; i<64; i++) //  modified on 2010/06/08  for(i=0; i<16; i++) 
	{
		m_TimeInt.Msc[i].Data = (unsigned char *) malloc(20736);
		if ( !m_TimeInt.Msc[i].Data )
			goto error_0;
	}

	// Buffer for Time Interleaving	
	/* //  modified on 2010/06/08
	for(i=0; i<16; i++) {
		for(j=0; j<16; j++) {
			m_pTimeIntBuff[i][j].Data = (unsigned char *) malloc(20736);
			if ( !m_pTimeIntBuff[i][j].Data )
				goto error_0;
		}
	}*/	
	m_pTimeIntBuff = (struct DATA **) malloc(16*sizeof(struct DATA*));
	for(i=0; i<16; i++) {
		m_pTimeIntBuff[i] = (struct DATA *) malloc(64*sizeof(struct DATA));
		
		for(j=0; j<64; j++) {
			m_pTimeIntBuff[i][j].Data = (unsigned char *) malloc(20736);
		}
	}

	m_pCif = (unsigned char *) malloc(6912);
	if ( !m_pCif )
		goto error_0;

	FramePhase = 0;

	////////////////////////
	MyGenePrbs(m_pPRBS);		// Funciton for generating PRBS vector
	MyGeneMoth(m_pMothCode);		
	MyUepTableGene(m_pUepTable);	// Function for generating UEP table

	// Initialize Buffer for Time Interleaving
	m_pInxBuff = 0;  // Initialize Index for Buffer
	for(i=0; i<16; i++) {
		for(j=0; j<64; j++) {//  modified on 2010/06/08  for(j=0; j<16; j++) {
			m_pTimeIntBuff[i][j].Length = 0;  

			for(k=0; k<20736; k++) {
				m_pTimeIntBuff[i][j].Data[k] = 0;
			}
		}
	}

	return 0;

error_0:
	MyFreeFunc();
	return -1;
}

bool MyDoReadETI(struct DATA_FRAME *m_Frame, unsigned char *pReadData)
{
	int i;
	int tmInx;

//	char str[100];
	// Read Infomation
	m_FrameInfo.FCT = ((unsigned int)pReadData[4]) > 249 ? 249 : (unsigned int)pReadData[4];
	m_FrameInfo.FICF = (unsigned int)(pReadData[5] >> 7);
	m_FrameInfo.NST = (unsigned int)(pReadData[5] & 0x7f);  //  modified on 2010/06/08 m_FrameInfo.NST = ((unsigned int)(pReadData[5] & 0x7f)) > 16 ? 16 : (unsigned int)(pReadData[5] & 0x7f);
	m_FrameInfo.FP = (unsigned int)(pReadData[6] >> 5);
	m_FrameInfo.MID = (unsigned int)((pReadData[6] >> 3) & 0x03);
	m_FrameInfo.FL = (unsigned int)(pReadData[6] & 0x03) * 256 + (unsigned int)pReadData[7];
	
//	sprintf(str, "DAB MODE : %d\n", m_FrameInfo.MID);
//	OutputDebugString(str);

	if (m_FrameInfo.NST > 64)
		m_FrameInfo.NST = 64;

	if(m_FrameInfo.FICF == 0)		// size of fast information channel
		m_FrameInfo.FICL = 0;
	else if(m_FrameInfo.MID == 3) 
		m_FrameInfo.FICL = 32;
	else
		m_FrameInfo.FICL = 24;

	for(i=0; i<m_FrameInfo.NST; i++) {
		m_SubInfo[i].SCID = (unsigned int)(pReadData[4*i+8] >> 2);
		m_SubInfo[i].SAD = (unsigned int)(pReadData[4*i+8] & 0x03)*256+(unsigned int)pReadData[4*i+9];
		m_SubInfo[i].TPL = (unsigned int)(pReadData[4*i+10] >> 2);
		m_SubInfo[i].STL = (unsigned int)(pReadData[4*i+10] & 0x03)*256+(unsigned int)pReadData[4*i+11];
	}

	
	if(MyDoCRC(pReadData) == 1) {
		// Read Data
		tmInx = m_FrameInfo.NST * 4 + 12;
		m_Frame->Fic.Length = m_FrameInfo.FICL * 4;	// Fast Information Channel
		memcpy(m_Frame->Fic.Data, &pReadData[tmInx],m_Frame->Fic.Length);
		tmInx += m_Frame->Fic.Length;
#if 0
		int index = 0;
		while(1)
		{
			int FIG_Length = m_Frame->Fic.Data[index] & 0x1F;
			
			int FIG_Type = ((m_Frame->Fic.Data[index] >> 5) & 0x7);
			index++;
	sprintf(str, "FIG_Type : %d, FIG_Length : %d, index : %d\n", FIG_Type, FIG_Length, index);
//	OutputDebugString(str);
			if(FIG_Length <= 0)
				continue;
			if(FIG_Type == 0)
			{
				int fig_data_OE = ((m_Frame->Fic.Data[index] >> 6) & 0x1);
				int fig_data_extension = (m_Frame->Fic.Data[index] & 0x1F);
				if(fig_data_extension == 21)
				{
					index = index + 2;
					int Flist_Lenth = m_Frame->Fic.Data[index] & 0x1F;
	sprintf(str, "FIG_Length : %d, Flist_Lenth : %d, fig_data_extension : %x\n", FIG_Length, Flist_Lenth, fig_data_extension);
	OutputDebugString(str);
					int total_flist_lenth = 0;
					while(1)
					{
						index = index + 3;
						total_flist_lenth = total_flist_lenth + 3;
						int R_M = ((m_Frame->Fic.Data[index] >> 4) & 0xF);
						int Continuity_flg = ((m_Frame->Fic.Data[index] >> 3) & 0x1);
						int list_length_freq = (m_Frame->Fic.Data[index] & 0x7);
						int freq_cnt;
						int frequency;
						int jj;
						if( R_M == 0 || R_M == 1)
						{
							freq_cnt = list_length_freq / 3;
							int Control_field;
							for(jj = 0; jj < freq_cnt; jj++)
							{
								index++;
								total_flist_lenth++;
								Control_field = ((m_Frame->Fic.Data[index] >> 3) & 0x1F);
								frequency = (m_Frame->Fic.Data[index] & 0x7);
								index++;
								total_flist_lenth++;
								frequency = (frequency << 8) + m_Frame->Fic.Data[index];
								index++;
								total_flist_lenth++;
								frequency = (frequency << 8) + m_Frame->Fic.Data[index];
	sprintf(str, "R_M : %x, Control_field : %x, frequency : %d\n", R_M, Control_field, frequency * 16);
//	OutputDebugString(str);
							}
						}
						else if(R_M == 8 || R_M == 9)
						{
							freq_cnt = list_length_freq;
							for(jj = 0; jj < freq_cnt; jj++)
							{
								index++;
								total_flist_lenth++;
								frequency = m_Frame->Fic.Data[index];
	sprintf(str, "R_M : %x, frequency : %d\n", R_M, (87500 + (frequency * 100)));
//	OutputDebugString(str);
							}

						}
						else if(R_M == 10)
						{
							freq_cnt = list_length_freq;
							for(jj = 0; jj < freq_cnt; jj++)
							{
								index++;
								total_flist_lenth++;
								frequency = m_Frame->Fic.Data[index];
								if(frequency < 16)
	sprintf(str, "R_M : %x, frequency : %d\n", R_M, (144 + (frequency * 9)));
								else
	sprintf(str, "R_M : %x, frequency : %d\n", R_M, (387 + (frequency * 9)));

//	OutputDebugString(str);
							}
						}
						else if(R_M == 12)
						{
							freq_cnt = list_length_freq / 2;
							for(jj = 0; jj < freq_cnt; jj++)
							{
								index++;
								total_flist_lenth++;
								frequency = m_Frame->Fic.Data[index];
								index++;
								total_flist_lenth++;
								frequency = (frequency * 256) + m_Frame->Fic.Data[index];
	sprintf(str, "R_M : %x, frequency : %d\n", R_M, (frequency * 5));

	OutputDebugString(str);
							}
						}
						else
						{
							index = index + list_length_freq;
							total_flist_lenth = total_flist_lenth + list_length_freq;
						}
						if(Flist_Lenth <= total_flist_lenth)
							break;
					}
					
				}
				else
				{
					index = index + FIG_Length;
				}
			}
			else
				index = index + FIG_Length;
			if(index >= (m_Frame->Fic.Length - 1))
				break;
		}
		OutputDebugString("END ============\n");	
#endif
		for(i=0; i<m_FrameInfo.NST; i++) { // Main Service Channel
			m_Frame->Msc[i].Length = m_SubInfo[i].STL * 8;
			memcpy(m_Frame->Msc[i].Data, &pReadData[tmInx], m_Frame->Msc[i].Length);
			tmInx += m_Frame->Msc[i].Length;
		}

		return true;
	}
	else
		return false;

}

bool MyDoCRC(unsigned char *pReadData)
{
	int i, j;
	int numCheck;
	bool CRCdata[16], tmCRCdata;
	unsigned char CRC[2];

	numCheck = 4 + 4 * m_FrameInfo.NST + 2;
	
	for(j=0; j<16; j++) { // Initialize
		CRCdata[j] = true;
	}

	for(i=0; i<numCheck; i++) {
		for(j=7; j>=0; j--) {
			tmCRCdata = CRCdata[15] ^ ( ((pReadData[i+4] >> j) & 0x01) == 0x01 ? true : false);
			CRCdata[15] = CRCdata[14];
			CRCdata[14] = CRCdata[13];
			CRCdata[13] = CRCdata[12];
			CRCdata[12] = CRCdata[11] ^ tmCRCdata;

			CRCdata[11] = CRCdata[10];
			CRCdata[10] = CRCdata[9];
			CRCdata[9] = CRCdata[8];
			CRCdata[8] = CRCdata[7];
			CRCdata[7] = CRCdata[6];
			CRCdata[6] = CRCdata[5];
			CRCdata[5] = CRCdata[4] ^ tmCRCdata;
			CRCdata[4] = CRCdata[3];
			CRCdata[3] = CRCdata[2];
			CRCdata[2] = CRCdata[1];
			CRCdata[1] = CRCdata[0];
			CRCdata[0] = tmCRCdata;
		}
	}

	CRC[0] = 0x00;
	for(j=0; j<8; j++) {
		CRC[0] += (unsigned char)((CRCdata[j+8] == false) ? pow((double)2, j) : 0);
	}

	CRC[1] = 0x00;
	for(j=0; j<8; j++) {
		CRC[1] += (unsigned char)((CRCdata[j] == false) ? pow((double)2, j) : 0);
	}

	if((CRC[0] == pReadData[numCheck+4]) && (CRC[1] == pReadData[numCheck+5])) 
		return true;
	else
		return false;

}

void MyDoEnerDisp(struct DATA_FRAME *pEnerDisp, struct DATA_FRAME Frame)
{
	int i, j;

	pEnerDisp->Fic.Length = Frame.Fic.Length;
	for(i=0; i<pEnerDisp->Fic.Length; i++) {	// Energy dispersal of Fast Information Channel
		pEnerDisp->Fic.Data[i] = Frame.Fic.Data[i] ^ m_pPRBS[i];
	}

	for(i=0; i<m_FrameInfo.NST; i++) { // Energy dispersal of Main Service Channel
		pEnerDisp->Msc[i].Length = Frame.Msc[i].Length;

		for(j=0; j<pEnerDisp->Msc[i].Length; j++) { // Energy dispersal of Sub-Stream
			pEnerDisp->Msc[i].Data[j] = Frame.Msc[i].Data[j] ^ m_pPRBS[j];
		}
	}
}

void MyPuncProc(unsigned int PuncIndx, unsigned char *PuncVal, unsigned char *MothVal)
{
	PuncVal[0] = 0;
	PuncVal[1] = 0;
	PuncVal[2] = 0;
	PuncVal[3] = 0;

	switch(PuncIndx) {
	case 1:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x08) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0x80) >> 3) | (MothVal[1] & 0x08);
		PuncVal[0] = PuncVal[0] | ((MothVal[2] & 0x80) >> 5) | ((MothVal[2] & 0x08) >> 2) | ((MothVal[3] & 0x80) >> 7);
		PuncVal[1] = (MothVal[3] & 0x08) << 4;
		break;
	case 2:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x08) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0x80) >> 3) | (MothVal[1] & 0x08);
		PuncVal[0] = PuncVal[0] | ((MothVal[2] & 0xc0) >> 5) | ((MothVal[2] & 0x08) >> 3);
		PuncVal[1] = (MothVal[3] & 0x80) | ((MothVal[3] & 0x08) << 3);
		break;
	case 3:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x08) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 3) | ((MothVal[1] & 0x08) >> 1);
		PuncVal[0] = PuncVal[0] | ((MothVal[2] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[2] & 0x08) << 4) | ((MothVal[3] & 0x80) >> 1) | ((MothVal[3] & 0x08) << 2);
		break;
	case 4:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x08) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 3) | ((MothVal[1] & 0x08) >> 1);
		PuncVal[0] = PuncVal[0] | ((MothVal[2] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[2] & 0x08) << 4) | ((MothVal[3] & 0xc0) >> 1) | ((MothVal[3] & 0x08) << 1);
		break;
	case 5:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x0c) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 4) | ((MothVal[1] & 0x08) >> 2) | ((MothVal[2] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[2] & 0x40) << 1) | ((MothVal[2] & 0x08) << 3) | ((MothVal[3] & 0xc0) >> 2);
		PuncVal[1] = PuncVal[1] | (MothVal[3] & 0x08);
		break;
	case 6:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x0c) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 4) | ((MothVal[1] & 0x08) >> 2) | ((MothVal[2] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[2] & 0x40) << 1) | ((MothVal[2] & 0x0c) << 3) | ((MothVal[3] & 0xc0) >> 3);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0x08) >> 1);
		break;
	case 7:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x0c) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 4) | ((MothVal[1] & 0x0c) >> 2);
		PuncVal[1] = (MothVal[2] & 0xc0) | ((MothVal[2] & 0x0c) << 2);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0xc0) >> 4) | ((MothVal[3] & 0x08) >> 2);
		break;
	case 8:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x0c) << 2);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 4) | ((MothVal[1] & 0x0c) >> 2);
		PuncVal[1] = (MothVal[2] & 0xc0) | ((MothVal[2] & 0x0c) << 2);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0xc0) >> 4) | ((MothVal[3] & 0x0c) >> 2);
		break;
	case 9:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0c) << 1);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 5) | ((MothVal[1] & 0x08) >> 3);
		PuncVal[1] = ((MothVal[1] & 0x04) << 5) | ((MothVal[2] & 0xc0) >> 1) | ((MothVal[2] & 0x0c) << 1);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0xc0) >> 5) | ((MothVal[3] & 0x08) >> 3);
		PuncVal[2] = ((MothVal[3] & 0x04) << 5);
		break;
	case 10:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0c) << 1);
		PuncVal[0] = PuncVal[0] | ((MothVal[1] & 0xc0) >> 5) | ((MothVal[1] & 0x08) >> 3);
		PuncVal[1] = ((MothVal[1] & 0x04) << 5) | ((MothVal[2] & 0xe0) >> 1) | (MothVal[2] & 0x0c);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0xc0) >> 6);
		PuncVal[2] = ((MothVal[3] & 0x0c) << 4);
		break;
	case 11:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0c) << 1) | ((MothVal[1] & 0xe0) >> 5);
		PuncVal[1] = ((MothVal[1] & 0x0c) << 4) | ((MothVal[2] & 0xe0) >> 2) | ((MothVal[2] & 0x0c) >> 1);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0x80) >> 7);
		PuncVal[2] = ((MothVal[3] & 0x40) << 1) | ((MothVal[3] & 0x0c) << 3);
		break;
	case 12:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0c) << 1) | ((MothVal[1] & 0xe0) >> 5);
		PuncVal[1] = ((MothVal[1] & 0x0c) << 4) | ((MothVal[2] & 0xe0) >> 2) | ((MothVal[2] & 0x0c) >> 1);
		PuncVal[1] = PuncVal[1] | ((MothVal[3] & 0x80) >> 7);
		PuncVal[2] = ((MothVal[3] & 0x60) << 1) | ((MothVal[3] & 0x0c) << 2);
		break;
	case 13:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0e) << 1) | ((MothVal[1] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[1] & 0x20) << 2) | ((MothVal[1] & 0x0c) << 3) | ((MothVal[2] & 0xe0) >> 3);
		PuncVal[1] = PuncVal[1] | ((MothVal[2] & 0x0c) >> 2);
		PuncVal[2] = (MothVal[3] & 0xe0) | ((MothVal[3] & 0x0c) << 1);
		break;
	case 14:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0e) << 1) | ((MothVal[1] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[1] & 0x20) << 2) | ((MothVal[1] & 0x0c) << 3) | ((MothVal[2] & 0xe0) >> 3);
		PuncVal[1] = PuncVal[1] | ((MothVal[2] & 0x0c) >> 2);
		PuncVal[2] = ((MothVal[2] & 0x02) << 6) | ((MothVal[3] & 0xe0) >> 1) | (MothVal[3] & 0x0c);
		break;
	case 15:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0e) << 1) | ((MothVal[1] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[1] & 0x20) << 2) | ((MothVal[1] & 0x0e) << 3) | ((MothVal[2] & 0xe0) >> 4);
		PuncVal[1] = PuncVal[1] | ((MothVal[2] & 0x08) >> 3);
		PuncVal[2] = ((MothVal[2] & 0x06) << 5) | ((MothVal[3] & 0xe0) >> 2) | ((MothVal[3] & 0x0c) >> 1);
		break;
	case 16:
		PuncVal[0] = (MothVal[0] & 0xe0) | ((MothVal[0] & 0x0e) << 1) | ((MothVal[1] & 0xc0) >> 6);
		PuncVal[1] = ((MothVal[1] & 0x20) << 2) | ((MothVal[1] & 0x0e) << 3) | ((MothVal[2] & 0xe0) >> 4);
		PuncVal[1] = PuncVal[1] | ((MothVal[2] & 0x08) >> 3);
		PuncVal[2] = ((MothVal[2] & 0x06) << 5) | ((MothVal[3] & 0xe0) >> 2) | ((MothVal[3] & 0x0e) >> 1);
		break;
	case 17:
		PuncVal[0] = (MothVal[0] & 0xfe) | ((MothVal[1] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[1] & 0x60) << 1) | ((MothVal[1] & 0x0e) << 2) | ((MothVal[2] & 0xe0) >> 5);
		PuncVal[2] = ((MothVal[2] & 0x0e) << 4) | ((MothVal[3] & 0xe0) >> 3) | ((MothVal[3] & 0x0c) >> 2);
		PuncVal[3] = ((MothVal[3] & 0x02) << 6);
		break;
	case 18:
		PuncVal[0] = (MothVal[0] & 0xfe) | ((MothVal[1] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[1] & 0x60) << 1) | ((MothVal[1] & 0x0e) << 2) | ((MothVal[2] & 0xe0) >> 5);
		PuncVal[2] = ((MothVal[2] & 0x1e) << 3) | ((MothVal[3] & 0xe0) >> 4) | ((MothVal[3] & 0x08) >> 3);
		PuncVal[3] = ((MothVal[3] & 0x06) << 5);
		break;
	case 19:
		PuncVal[0] = (MothVal[0] & 0xfe) | ((MothVal[1] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[1] & 0x7e) << 1) | ((MothVal[2] & 0xc0) >> 6);
		PuncVal[2] = ((MothVal[2] & 0x3e) << 2) | ((MothVal[3] & 0xe0) >> 5);
		PuncVal[3] = ((MothVal[3] & 0x0e) << 4);
		break;
	case 20:
		PuncVal[0] = (MothVal[0] & 0xfe) | ((MothVal[1] & 0x80) >> 7);
		PuncVal[1] = ((MothVal[1] & 0x7e) << 1) | ((MothVal[2] & 0xc0) >> 6);
		PuncVal[2] = ((MothVal[2] & 0x3e) << 2) | ((MothVal[3] & 0xe0) >> 5);
		PuncVal[3] = ((MothVal[3] & 0x1e) << 3);
		break;
	case 21:
		PuncVal[0] = MothVal[0];
		PuncVal[1] = (MothVal[1] & 0xfe) | ((MothVal[2] & 0x80) >> 7);
		PuncVal[2] = ((MothVal[2] & 0x7e) << 1) | ((MothVal[3] & 0xc0) >> 6);
		PuncVal[3] = ((MothVal[3] & 0x3e) << 2);
		break;
	case 22:
		PuncVal[0] = MothVal[0];
		PuncVal[1] = (MothVal[1] & 0xfe) | ((MothVal[2] & 0x80) >> 7);
		PuncVal[2] = ((MothVal[2] & 0x7f) << 1) | ((MothVal[3] & 0x80) >> 7);
		PuncVal[3] = ((MothVal[3] & 0x7e) << 1);
		break;
	case 23:
		PuncVal[0] = MothVal[0];
		PuncVal[1] = MothVal[1];
		PuncVal[2] = MothVal[2];
		PuncVal[3] = (MothVal[3] & 0xfe);
		break;
	case 24:
		PuncVal[0] = MothVal[0];
		PuncVal[1] = MothVal[1];
		PuncVal[2] = MothVal[2];
		PuncVal[3] = MothVal[3];
		break;

	case 0:
		PuncVal[0] = (MothVal[0] & 0xc0) | ((MothVal[0] & 0x0c) << 2) | ((MothVal[1] & 0xc0) >> 4) | ((MothVal[1] & 0x0c) >> 2);
		PuncVal[1] = (MothVal[2] & 0xc0) | ((MothVal[2] & 0x0c) << 2) | ((MothVal[3] & 0xc0) >> 4) | ((MothVal[3] & 0x0c) >> 2);
		break;
	default:
		PuncVal[0] = 0x00;
		PuncVal[1] = 0x00;
		PuncVal[2] = 0x00;
		PuncVal[3] = 0x00;
	}
}

int MyGetUepIndex(int STL, int ProtLevel)
{
	int Index;

	switch(STL) {
	case 12:
		switch(ProtLevel) {
		case 5: Index = 0; break;
		case 4: Index = 1; break;
		case 3: Index = 2; break;
		case 2: Index = 3; break;
		case 1: Index = 4; break; 
		} break;
	case 18:
		switch(ProtLevel) {
		case 5: Index = 5; break;
		case 4: Index = 6; break;
		case 3: Index = 7; break;
		case 2: Index = 8; break;
		case 1: Index = 9; break; 
		} break;
	case 21:
		switch(ProtLevel) {
		case 5: Index = 10; break;
		case 4: Index = 11; break;
		case 3: Index = 12; break;
		case 2: Index = 13; break;
		} break;
	case 24:
		switch(ProtLevel) {
		case 5: Index = 14; break;
		case 4: Index = 15; break;
		case 3: Index = 16; break;
		case 2: Index = 17; break;
		case 1: Index = 18; break; 
		} break;
	case 30:
		switch(ProtLevel) {
		case 5: Index = 19; break;
		case 4: Index = 20; break;
		case 3: Index = 21; break;
		case 2: Index = 22; break;
		case 1: Index = 23; break; 
		} break;
	case 36:
		switch(ProtLevel) {
		case 5: Index = 24; break;
		case 4: Index = 25; break;
		case 3: Index = 26; break;
		case 2: Index = 27; break;
		case 1: Index = 28; break; 
		} break;
	case 42:
		switch(ProtLevel) {
		case 5: Index = 29; break;
		case 4: Index = 30; break;
		case 3: Index = 31; break;
		case 2: Index = 32; break;
		} break;
	case 48:
		switch(ProtLevel) {
		case 5: Index = 33; break;
		case 4: Index = 34; break;
		case 3: Index = 35; break;
		case 2: Index = 36; break;
		case 1: Index = 37; break; 
		} break;
	case 60:
		switch(ProtLevel) {
		case 5: Index = 38; break;
		case 4: Index = 39; break;
		case 3: Index = 40; break;
		case 2: Index = 41; break;
		case 1: Index = 42; break; 
		} break;
	case 72:
		switch(ProtLevel) {
		case 5: Index = 43; break;
		case 4: Index = 44; break;
		case 3: Index = 45; break;
		case 2: Index = 46; break;
		case 1: Index = 47; break; 
		} break;
	case 84:
		switch(ProtLevel) {
		case 5: Index = 48; break;
		case 4: Index = 49; break;
		case 3: Index = 50; break;
		case 2: Index = 51; break;
		case 1: Index = 52; break; 
		} break;
	case 96:
		switch(ProtLevel) {
		case 5: Index = 53; break;
		case 4: Index = 54; break;
		case 3: Index = 55; break;
		case 2: Index = 56; break;
		case 1: Index = 57; break; 
		} break;			
	case 120:
		switch(ProtLevel) {
		case 5: Index = 58; break;
		case 4: Index = 59; break;
		case 3: Index = 60; break;
		} break;
	case 144:
		switch(ProtLevel) {
		case 5: Index = 61; break;
		case 3: Index = 62; break;
		case 1: Index = 63; break; 
		} break;			
	}
	
	return Index;

}

void MyDoConvEnco(struct DATA_FRAME *pConvEnco, struct DATA_FRAME EnerDisp)
{
	int i, j, k, InxTm;
	int IndxConvData=0, PosBit, PosBitTm;
	int NumByte, NumBit;
	unsigned char StatusReg=0x00;
	unsigned char MothVal[4], PuncVal[4], BuffVal;

	int ProtProfIdx;

	int DataRateN;
	int Leng1, Leng2, ProtInx1, ProtInx2;



	// Convolutional Coding for Fast Information Channel
	StatusReg = 0x00;
	PosBit = 0; BuffVal = 0x00;

	if(m_FrameInfo.MID == 3)  {  // Transmission Mode III

		m_ConvEnco.Fic.Length = 384;
		for(i=0; i<116; i++) {  // Puncturing Index PI = 16
			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] >> 6);
			MothVal[0] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x30) >> 4);
			MothVal[1] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x0c) >> 2);
			MothVal[2] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] & 0x03);
			MothVal[3] = m_pMothCode[StatusReg];

			MyPuncProc(16, PuncVal, MothVal);

			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[0];
			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[1];
			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[2];
		}

		for(i=116; i<128; i++) {  // Puncturing Index PI = 15
			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] >> 6);
			MothVal[0] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x30) >> 4);
			MothVal[1] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x0c) >> 2);
			MothVal[2] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] & 0x03);
			MothVal[3] = m_pMothCode[StatusReg];

			MyPuncProc(15, PuncVal, MothVal);

			PosBitTm = 8-PosBit;
			pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] >> PosBit) | BuffVal;
			pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] << PosBitTm) | (PuncVal[1] >> PosBit);
			if(PosBit == 0) {
				BuffVal = PuncVal[2];
				PosBit = 7;
			}
			else {
				pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[1] << PosBitTm) | (PuncVal[2] >> PosBit);
				BuffVal = (PuncVal[2] << PosBitTm);
				PosBit = PosBit - 1;
			}
		}
	}
	else {	// Transmission Mode I, II, IV
		m_ConvEnco.Fic.Length = 288;
		for(i=0; i<84; i++) {  // Puncturing Index PI = 16
			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] >> 6);
			MothVal[0] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x30) >> 4);
			MothVal[1] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x0c) >> 2);
			MothVal[2] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] & 0x03);
			MothVal[3] = m_pMothCode[StatusReg];

			MyPuncProc(16, PuncVal, MothVal);
			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[0];
			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[1];
			pConvEnco->Fic.Data[IndxConvData++] = PuncVal[2];
		}



		for(i=84; i<96; i++) {  // Punctureing Index PI = 15
			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] >> 6);
			MothVal[0] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x30) >> 4);
			MothVal[1] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | ((EnerDisp.Fic.Data[i] & 0x0c) >> 2);
			MothVal[2] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2) | (EnerDisp.Fic.Data[i] & 0x03);
			MothVal[3] = m_pMothCode[StatusReg];

			MyPuncProc(15, PuncVal, MothVal);

			PosBitTm = 8-PosBit;
			pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] >> PosBit) | BuffVal;
			pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] << PosBitTm) | (PuncVal[1] >> PosBit);
			if(PosBit == 0) {
				BuffVal = PuncVal[2];
				PosBit = 7;
			}
			else {
				pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[1] << PosBitTm) | (PuncVal[2] >> PosBit);
				BuffVal = (PuncVal[2] << PosBitTm);
				PosBit = PosBit - 1;
			}
		}
	}

	StatusReg = (StatusReg << 2);
	MothVal[0] = m_pMothCode[StatusReg];

	StatusReg = (StatusReg << 2);
	MothVal[1] = m_pMothCode[StatusReg];

	StatusReg = (StatusReg << 2);
	MothVal[2] = m_pMothCode[StatusReg];

	StatusReg = (StatusReg << 2);
	MothVal[3] = m_pMothCode[StatusReg];

	MyPuncProc(0, PuncVal, MothVal);   // Puncturing Vector V_PI

	pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] >> 4) | BuffVal;
	pConvEnco->Fic.Data[IndxConvData++] = (PuncVal[0] << 4) | (PuncVal[1] >> 4);


	// Convolutional Coding for Main Service Channel
	for(j=0; j<m_FrameInfo.NST; j++) {
		PosBit = 0; BuffVal=0x00;
		IndxConvData = 0; InxTm = 0;
		StatusReg = 0x00;

///////////////////////////////////////////////////////////////////////////////////////////////////
//		Unequal Error Protection(UEP) coding
///////////////////////////////////////////////////////////////////////////////////////////////////
		if((m_SubInfo[j].TPL & 0x20) == 0x00) {
			ProtProfIdx =  MyGetUepIndex(m_SubInfo[j].STL, (m_SubInfo[j].TPL & 0x07) + 1);

			// L1 blocks sahll be punctured accoding to the puncturing index PI1
			for(i = 0; i < m_pUepTable[ProtProfIdx].L1 * 4; i++) {		
				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
				MothVal[2] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
				MothVal[3] = m_pMothCode[StatusReg];

				MyPuncProc(m_pUepTable[ProtProfIdx].PI1, PuncVal, MothVal);  // Puncturing

				NumBit = PosBit + (m_pUepTable[ProtProfIdx].PI1 % 8);
				PosBitTm = 8 - PosBit;
				NumByte = (m_pUepTable[ProtProfIdx].PI1 + 8 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}
			}

			// L2 blocks sahll be punctured accoding to the puncturing index PI2
			for(i = 0; i < m_pUepTable[ProtProfIdx].L2 * 4; i++) {	
				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
				MothVal[2] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
				MothVal[3] = m_pMothCode[StatusReg];

				MyPuncProc(m_pUepTable[ProtProfIdx].PI2, PuncVal, MothVal);  // Puncturing

				NumBit = PosBit + (m_pUepTable[ProtProfIdx].PI2 % 8);
				PosBitTm = 8 - PosBit;
				NumByte = (m_pUepTable[ProtProfIdx].PI2 + 8 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}
			}

			// L3 blocks sahll be punctured accoding to the puncturing index PI3
			for(i = 0; i < m_pUepTable[ProtProfIdx].L3 * 4; i++) {	
				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
				MothVal[2] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
				MothVal[3] = m_pMothCode[StatusReg];

				MyPuncProc(m_pUepTable[ProtProfIdx].PI3, PuncVal, MothVal);  // Puncturing

				NumBit = PosBit + (m_pUepTable[ProtProfIdx].PI3 % 8);
				PosBitTm = 8 - PosBit;
				NumByte = (m_pUepTable[ProtProfIdx].PI3 + 8 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}
			}

			// L4 blocks sahll be punctured accoding to the puncturing index PI4
			for(i = 0; i < m_pUepTable[ProtProfIdx].L4 * 4; i++) {	
				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
				MothVal[2] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
				MothVal[3] = m_pMothCode[StatusReg];

				MyPuncProc(m_pUepTable[ProtProfIdx].PI4, PuncVal, MothVal);  // Puncturing

				NumBit = PosBit + (m_pUepTable[ProtProfIdx].PI4 % 8);
				PosBitTm = 8 - PosBit;
				NumByte = (m_pUepTable[ProtProfIdx].PI4 + 8 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}
			}

			// The last 24 bits of the serial mother codeword shall be punctured 
			StatusReg = (StatusReg << 2);
			MothVal[0] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2);
			MothVal[1] = m_pMothCode[StatusReg];

			StatusReg = (StatusReg << 2);
			MothVal[2] = m_pMothCode[StatusReg];

			MothVal[3] = 0x00;

			MyPuncProc(0, PuncVal, MothVal);   // Puncturing Vector V_PI

			NumBit = PosBit + 4;
			PosBitTm = 8 - PosBit;
			NumByte = (12 + PosBit) / 8;

			if(NumBit < 8) {
				for(k=0; k<NumByte; k++) {
					pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
					BuffVal = PuncVal[k] << PosBitTm;
				}
				BuffVal = BuffVal | (PuncVal[k] >> PosBit);
				PosBit = NumBit % 8;
			}
			else {
				for(k=0; k<NumByte; k++) {
					pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
					BuffVal = PuncVal[k] << PosBitTm;
				}
				PosBit = NumBit % 8;
			}

			if(PosBit != 0) {
				pConvEnco->Msc[j].Data[IndxConvData++] = BuffVal;	
			}

			pConvEnco->Msc[j].Length = IndxConvData;
		}

///////////////////////////////////////////////////////////////////////////////////////////////
//		Equal Error Protection(EEP) Coding
///////////////////////////////////////////////////////////////////////////////////////////////
		else { 

			// Bit Rates in Multiples of 8 kbit/s
			if((m_SubInfo[j].TPL & 0x1c) == 0x00) { 
				DataRateN = m_SubInfo[j].STL / 3;

				switch(m_SubInfo[j].TPL & 0x03) {		// Setting L1, L3, PI1 and PI2
					case 0x00: 
						Leng1 = 6 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 24;	ProtInx2 = 23;
						break;
					case 0x01: 
						if(DataRateN == 1) {
							Leng1 = 5;		Leng2 = 1;
							ProtInx1 = 13;	ProtInx2 = 12;
						}
						else {
							Leng1 = 2 * DataRateN - 3;		Leng2 = 4 * DataRateN + 3;
							ProtInx1 = 14;	ProtInx2 = 13;
						}
						break;
					case 0x02: 
						Leng1 = 6 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 8;	ProtInx2 = 7;
						break;
					case 0x03: 
						Leng1 = 4 * DataRateN - 3;	Leng2 = 2 * DataRateN + 3;
						ProtInx1 = 3;	ProtInx2 = 2;
						break;
				}

				// The first L1 blocks shall be punctured according to the puncturing index PI1
				for(i=0; i<Leng1 * 4; i++) {
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
					MothVal[0] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
					MothVal[1] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
					MothVal[2] = m_pMothCode[StatusReg];
	
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
					MothVal[3] = m_pMothCode[StatusReg];

					MyPuncProc(ProtInx1, PuncVal, MothVal);  // Puncturing

					NumBit = PosBit + (ProtInx1 % 8);
					NumByte = (ProtInx1 + 8 + PosBit) / 8;
					PosBitTm = 8 - PosBit;

					if(NumBit < 8) {
						for(k=0; k<NumByte; k++) {
							m_ConvEnco.Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						BuffVal = BuffVal | (PuncVal[k] >> PosBit);
						PosBit = NumBit % 8;
					}
					else {
						for(k=0; k<NumByte; k++) {
							m_ConvEnco.Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						PosBit = NumBit % 8;
					}
				}

				// The first L2 blocks shall be punctured according to the puncturing index PI2
				for(i=0; i<Leng2 * 4; i++) {
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
					MothVal[0] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
					MothVal[1] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
					MothVal[2] = m_pMothCode[StatusReg];
	
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
					MothVal[3] = m_pMothCode[StatusReg];

					MyPuncProc(ProtInx2, PuncVal, MothVal);  // Puncturing


					NumBit = PosBit + (ProtInx2 % 8);
					NumByte = (ProtInx2 + 8 + PosBit) / 8;
					PosBitTm = 8 - PosBit;

					if(NumBit < 8) {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						BuffVal = BuffVal | (PuncVal[k] >> PosBit);
						PosBit = NumBit % 8;
					}
					else {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						PosBit = NumBit % 8;
					}

				}
				// The last 24 bits of the serial mother codeword shall be punctured 
				StatusReg = (StatusReg << 2);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2);
				MothVal[2] = m_pMothCode[StatusReg];

				MothVal[3] = 0x00;

				MyPuncProc(0, PuncVal, MothVal);   // Puncturing Vector V_PI

				NumBit = PosBit + 4;
				PosBitTm = 8 - PosBit;
				NumByte = (12 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}

				pConvEnco->Msc[j].Length = IndxConvData;
			}

			// Bit Rates in Multiples of 32 kbit/s
			else {
				DataRateN = m_SubInfo[j].STL / 12;
				switch(m_SubInfo[j].TPL & 0x03) {		// Setting L1, L3, PI1 and PI2
					case 0x00: 
						Leng1 = 24 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 10;	ProtInx2 = 9;
						break;
					case 0x01: 
						Leng1 = 24 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 6;	ProtInx2 = 5;
						break;
					case 0x02: 
						Leng1 = 24 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 4;	ProtInx2 = 3;
						break;
					case 0x03: 
						Leng1 = 24 * DataRateN - 3;	Leng2 = 3;
						ProtInx1 = 2;	ProtInx2 = 1;
						break;
				}

				// The first L1 blocks shall be punctured according to the puncturing index PI1
				for(i=0; i<Leng1 * 4; i++) {
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
					MothVal[0] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
					MothVal[1] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
					MothVal[2] = m_pMothCode[StatusReg];
	
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
					MothVal[3] = m_pMothCode[StatusReg];

					MyPuncProc(ProtInx1, PuncVal, MothVal);  // Puncturing

					NumBit = PosBit + (ProtInx1 % 8);
					NumByte = (ProtInx1 + 8 + PosBit) / 8;
					PosBitTm = 8 - PosBit;

					if(NumBit < 8) {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						BuffVal = BuffVal | (PuncVal[k] >> PosBit);
						PosBit = NumBit % 8;
					}
					else {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						PosBit = NumBit % 8;
					}
				}

				// The first L2 blocks shall be punctured according to the puncturing index PI2
				for(i=0; i<Leng2 * 4; i++) {
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm] >> 6);
					MothVal[0] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x30) >> 4);
					MothVal[1] = m_pMothCode[StatusReg];

					StatusReg = (StatusReg << 2) | ((EnerDisp.Msc[j].Data[InxTm] & 0x0c) >> 2);
					MothVal[2] = m_pMothCode[StatusReg];
	
					StatusReg = (StatusReg << 2) |  (EnerDisp.Msc[j].Data[InxTm++] & 0x03);
					MothVal[3] = m_pMothCode[StatusReg];

					MyPuncProc(ProtInx2, PuncVal, MothVal);  // Puncturing

					NumBit = PosBit + (ProtInx2 % 8);
					NumByte = (ProtInx2 + 8 + PosBit) / 8;
					PosBitTm = 8 - PosBit;

					if(NumBit < 8) {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						BuffVal = BuffVal | (PuncVal[k] >> PosBit);
						PosBit = NumBit % 8;
					}
					else {
						for(k=0; k<NumByte; k++) {
							pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
							BuffVal = PuncVal[k] << PosBitTm;
						}
						PosBit = NumBit % 8;
					}
				}

				// The last 24 bits of the serial mother codeword shall be punctured 
				StatusReg = (StatusReg << 2);
				MothVal[0] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2);
				MothVal[1] = m_pMothCode[StatusReg];

				StatusReg = (StatusReg << 2);
				MothVal[2] = m_pMothCode[StatusReg];

				MothVal[3] = 0x00;

				MyPuncProc(0, PuncVal, MothVal);   // Puncturing Vector V_PI

				NumBit = PosBit + 4;
				PosBitTm = 8 - PosBit;
				NumByte = (12 + PosBit) / 8;

				if(NumBit < 8) {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					BuffVal = BuffVal | (PuncVal[k] >> PosBit);
					PosBit = NumBit % 8;
				}
				else {
					for(k=0; k<NumByte; k++) {
						pConvEnco->Msc[j].Data[IndxConvData++] = (PuncVal[k] >> PosBit) | BuffVal;
						BuffVal = PuncVal[k] << PosBitTm;
					}
					PosBit = NumBit % 8;
				}

				pConvEnco->Msc[j].Length = IndxConvData;
			}			
		}
	}
}

void MyDoTimeInterleav(struct DATA_FRAME *pTimeInt, struct DATA_FRAME ConvEnco)
{
	int i, j, jTm;
	int LenTimeInt;
	

	for(i=0; i<m_FrameInfo.NST; i++) { // Time Interleaving of Main Service Channel
		memcpy(m_pTimeIntBuff[m_pInxBuff][i].Data, ConvEnco.Msc[i].Data, ConvEnco.Msc[i].Length);  // Buffer¿¡ ÀúÀå
		m_pTimeIntBuff[m_pInxBuff][i].Length = ConvEnco.Msc[i].Length;


		LenTimeInt =  (m_pInxBuff != 15) ? 
			max(m_pTimeIntBuff[m_pInxBuff][i].Length, m_pTimeIntBuff[m_pInxBuff+1][i].Length) :
			max(m_pTimeIntBuff[m_pInxBuff][i].Length, m_pTimeIntBuff[0][i].Length) ;

		
		for(j=0; j<LenTimeInt; j+=2) {
			pTimeInt->Msc[i].Data[j] = 
				(m_pTimeIntBuff[m_pInxBuff][i].Data[j] & 0x80) | 
				(m_pTimeIntBuff[(m_pInxBuff+8 ) % 16][i].Data[j] & 0x40) |
				(m_pTimeIntBuff[(m_pInxBuff+12) % 16][i].Data[j] & 0x20) |
				(m_pTimeIntBuff[(m_pInxBuff+4 ) % 16][i].Data[j] & 0x10) |
				(m_pTimeIntBuff[(m_pInxBuff+14) % 16][i].Data[j] & 0x08) |
				(m_pTimeIntBuff[(m_pInxBuff+6 ) % 16][i].Data[j] & 0x04) |
				(m_pTimeIntBuff[(m_pInxBuff+10) % 16][i].Data[j] & 0x02) |
				(m_pTimeIntBuff[(m_pInxBuff+2 ) % 16][i].Data[j] & 0x01);

			jTm = j + 1;
			pTimeInt->Msc[i].Data[jTm] = 
			    (m_pTimeIntBuff[(m_pInxBuff+15) % 16][i].Data[jTm] & 0x80) | 
				(m_pTimeIntBuff[(m_pInxBuff+7 ) % 16][i].Data[jTm] & 0x40) |
				(m_pTimeIntBuff[(m_pInxBuff+11) % 16][i].Data[jTm] & 0x20) |
				(m_pTimeIntBuff[(m_pInxBuff+3 ) % 16][i].Data[jTm] & 0x10) |
				(m_pTimeIntBuff[(m_pInxBuff+13) % 16][i].Data[jTm] & 0x08) |
				(m_pTimeIntBuff[(m_pInxBuff+5 ) % 16][i].Data[jTm] & 0x04) |
				(m_pTimeIntBuff[(m_pInxBuff+9 ) % 16][i].Data[jTm] & 0x02) |
				(m_pTimeIntBuff[(m_pInxBuff+1 ) % 16][i].Data[jTm] & 0x01);
			}
		pTimeInt->Msc[i].Length = LenTimeInt;  // Setting the Length of Time Interleaved Data
	}
	m_pInxBuff = (++m_pInxBuff) % 16;

}

void MyDoCif(unsigned char *pCIF, struct DATA_FRAME TimeInt)
{
	int i;

	memcpy(pCIF, m_pPRBS, 6912);
	for(i=0; i<m_FrameInfo.NST; i++) 
		memcpy(&pCIF[m_SubInfo[i].SAD * 8], TimeInt.Msc[i].Data, TimeInt.Msc[i].Length);
}
