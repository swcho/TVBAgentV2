//=================================================================	
//	cif.c  /  Conversion Utility for TVB390-TDMB
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2007
//=================================================================	

#ifndef __CIF_H__
#define __CIF_H__

//=================================================================	
#ifdef WIN32

#include	<windows.h>

#ifndef bool
#define bool BOOL
#endif
#ifndef true
#define true TRUE
#endif
#ifndef false
#define false FALSE
#endif

#ifdef __cplusplus
extern "C" {
#endif
//=================================================================	
// Linux
#else

#define max(a, b)  (((a) > (b)) ? (a) : (b))

#ifndef bool
#define bool int
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#endif

//=================================================================	
// Data structure
typedef	struct FRAME_INFO{
		int FCT;
		int FICF;
		int NST;
		int FP;
		int MID;
		int FL;
		int FICL;
} _FRAME_INFO;

typedef	struct SUB_STREAM_INFO{
	int SCID;
	int SAD;
	int TPL;
	int STL;
} _SUB_STREAM_INFO;
	
typedef	struct DATA{
	unsigned char *Data;
	int Length;
} _DATA;

typedef	struct DATA_FRAME{
	struct DATA Fic;
	struct DATA Msc[64]; 	// modified on 2010/06/08 struct DATA Msc[16]; 
} _FRAME;

typedef	struct UEP_PP{
	int L1;
	int L2;
	int L3;
	int L4;
	int PI1;
	int PI2;
	int PI3;
	int PI4;
	int PAD_BIT;
} _UEP_PP;

//=================================================================	
// Information and Data of a Frame
extern struct DATA_FRAME m_Frame;
extern struct FRAME_INFO m_FrameInfo;
extern struct SUB_STREAM_INFO m_SubInfo[64]; //extern struct SUB_STREAM_INFO m_SubInfo[16]; modified on 2010/06/08

//=================================================================	
// 10 Energy Dispersal
extern unsigned char m_pPRBS[6912];	// PRBS Sequence
extern struct DATA_FRAME m_EnerDisp;

//=================================================================	
// 11 Convoutional Coding
extern struct UEP_PP m_pUepTable[64];
extern unsigned char m_pMothCode[255];
extern struct DATA_FRAME m_ConvEnco;

//=================================================================	
// 12 Time Interleaving
extern int m_pInxBuff;						// Index for Current Buffer
extern struct DATA **m_pTimeIntBuff;  // modified on 2010/06/08 extern struct DATA m_pTimeIntBuff[16][16]; 		// Buffer for Time Interleaving
extern struct DATA_FRAME m_TimeInt;					// Time Interleaved Data for Main Service Channel

//=================================================================	
// 13 Common Interleaved Frame
extern unsigned char *m_pCif;			// CIF Data

//=================================================================	
// 
extern int FramePhase;;
extern unsigned char WrBuff[7300];

//=================================================================	
// 
void MyGenePrbs(unsigned char *PRBS);  // PRBS generator
void MyGeneMoth(unsigned char *m_pMothCode);
void MyUepTableGene(struct UEP_PP *UepTable);
int MyInitFunc();
void MyFreeFunc();
bool MyDoReadETI(struct DATA_FRAME *m_Frame, unsigned char *pReadData);
bool MyDoCRC(unsigned char *pReadData);
void MyDoEnerDisp(struct DATA_FRAME *pEnerDisp, struct DATA_FRAME Frame);
void MyPuncProc(unsigned int PuncIndx, unsigned char *PuncVal, unsigned char *MothVal);
int MyGetUepIndex(int STL, int ProtLevel);
void MyDoConvEnco(struct DATA_FRAME *pConvEnco, struct DATA_FRAME EnerDisp);
void MyDoTimeInterleav(struct DATA_FRAME *pTimeInt, struct DATA_FRAME ConvEnco);
void MyDoCif(unsigned char *pCIF, struct DATA_FRAME TimeInt);
#ifdef WIN32
#ifdef __cplusplus
}
#endif
#endif
#endif //__CIF_H__

