//=================================================================	
//	UTIL.c / TVB370/380/390, TES110, TSP104 utility functions
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

//=================================================================	
#if defined(WIN32)
#include	<windows.h>
#include	<winbase.h>
#include	<string.h>
#include	<conio.h>

#if !defined	bool
#define	bool	BOOL
#endif

//=================================================================	
// Linux
#else

#include "tsp100.h"

#ifdef TSP104


#if !defined	FALSE
#define	FALSE	false
#endif
#if !defined	TRUE
#define	TRUE	true
#endif

#if !defined	DWORD
#define	DWORD unsigned int
#endif

#define Sleep(n)	do { usleep(n * 1000); } while (0)

#else
#endif

#include <stdio.h>
#include <termios.h>
struct termios old_t, new_t;
int _kbhit()
{
	char ch;			
	int nr_read;

	nr_read	= read( fileno(stdin), &ch, 1 );
	if ( nr_read == 1 )
	{
		tcsetattr( fileno(stdin), TCSANOW, &old_t );
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif

//TVB590S, TVB593
#include "../include/hld_api.h"
#include	"../include/hld_structure.h"

//------------------------------------------------------------------------
// iTSize = 188,204,208
int Calculate_Bitrate(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize)
{
	DWORD	dwRemained = dwSize;
	int		index;
	WORD	wPID;
	WORD	w1PID;		// First PCR PID
	BYTE	bADExist;
	BYTE	bADLen;

	int nPacketSize = iTSize;
	
	__int64	iiPCR1 = 0;
	__int64	iiPCR2 = 0;
	int		iNoPkt = 0;
	int		ix1, ix2;
	int		iNoPCR2 = 0;

	dwRemained -= iSyncStartPos;
	index = iSyncStartPos;

	//------------------------------------------------------------------------
	//--- Replace Time Stamp 
	//    PCR in Adaption Field
	//    PTS/DTS/ESCR in PES Header

	while (dwRemained > 207)	
	{
		if (bData[index] == 0x47) 
		{
			wPID = (bData[index+1]&0x1F)*256+bData[index+2];

			bADExist = bData[index+3] & 0x20;
			bADLen   = bData[index+4];

			//----------------------------------------------------------------
			//--- Adaptation and PCR Check
			//----------------------------------------------------------------
			if (bADExist && (bADLen >=7) )	// min length for PCR or OPCR 
			{
				if (bData[index+5] & 0x10)	// PCR (or OPCR = 0x08)
				{
					__int64	iiPCR;

					//--- Get PCR Value
					iiPCR = bData[index+6];		
					iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
					iiPCR += bData[index+7] << 17;
					iiPCR += bData[index+8] << 9;
					iiPCR += bData[index+9] << 1;
					iiPCR += (bData[index+10] & 0x80) >> 7;
					iiPCR *= 300;

					iiPCR += (bData[index+10] & 0x01) << 8;
					iiPCR += bData[index+11];

					//--- Check For Bitrate
					if (iiPCR1 == 0)
					{
						iiPCR1	= iiPCR;
						w1PID	= wPID;
						ix1		= iNoPkt;
					} else if (wPID == w1PID)
					{
						//--- check if round-robin
						if (iiPCR < iiPCR1)
						{
							iiPCR1 = iiPCR;
							ix1	   = iNoPkt;
							iNoPCR2 = 0;
						} else {
							iiPCR2	= iiPCR;
							ix2		= iNoPkt;
							iNoPCR2++;
							if (iNoPCR2 > 10)
								break;
						}
					}							
				}
			}

		} else  
		{
			// Out of Sync: Stop PCR Checking
			break;
		}
		
		index += iTSize;
		iNoPkt++;
		dwRemained -= iTSize;
	}
	//--- check PUSI for PTS/DTS/ESCR replace

	nPacketSize = 188;
#ifdef DEF_ISDB_T
	nPacketSize = iTSize;//204, 208
#endif

	if (iiPCR2 > iiPCR1) 
	{
		double		iDiffTime = (double) iiPCR2-iiPCR1;
		/* ISDB-T(=9) */
		//double		iLen	  = (ix2-ix1)*188*8;	
		double		iLen	  = (ix2-ix1)*nPacketSize*8;
		double		iBitrate  = iLen*27000000/iDiffTime;
		//char		text[200];
		
		//wsprintf(text," <<< CAL Bitrate for (%04X): 1stPCR=%d ms, lastPCR=%d ms, iLen=%d, bitrate=%d, #PCR2=%d >>>\n",
		//     w1PID, 
		//	 (int) (iiPCR1/27000),
		//	 (int) (iiPCR2/27000),
		//	 /* ISDB-T(=9) */
		//	 //(ix2-ix1)*188,
		//	 (ix2-ix1)*nPacketSize,
		//	 (int) (iBitrate),
		//	 iNoPCR2);
		// OutputDebugString(text);
		return (int) iBitrate;
	} else {		// Can't Calculate bitrate
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
//
//	Find valid TP start offset and return the offset
//	Return -1 on error, return snc byte offset when TS sync byte is found
//
//
//	Find valid TP start offset and return the offset
//	Return -1 on error
//
int	TSSyncLockFunction(char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step) 
{
	int		i, j;
	bool	err;

	if (nlen < nlen_srch)
		return -1;

	for (i = 0; i < nlen_srch; i++)
	{
		if (szBuf[i] == 0x47) {

			//-----------------------------------------------
			// check if 188 bytes TS Stream ?
			err = FALSE;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*188 + i] != 0x47) {
					err = TRUE;
					break;
				}
			}

			if (err == FALSE) {
				*iSize = 188;
				return i;
			}

			//-----------------------------------------------
			// check if 204 bytes TS Stream ?
			err = FALSE;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*204 + i] != 0x47) {
					err = TRUE;
					break;
				}
			}

			if (err == FALSE) {
				*iSize = 204;
				return i;
			}

			//-----------------------------------------------
			// check if 208 bytes TS Stream ?
			err = FALSE;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*208 + i] != 0x47) {
					err = TRUE;
					break;
				}
			}

			if (err == FALSE) {
				*iSize = 208;
				return i;
			}

		}

	}

	return -1;
}

int AdjustBankOffset(int banknum, int bankoffset, int playrate)
{
	int nNewBankOffset, i;
	double timetowait = 2.0, offset;
	offset = (timetowait * playrate) / (1024. * 8. * (banknum + 1.));

	nNewBankOffset = 1;
	for ( i = 0; i < 10; i++ )
	{
		offset = (((int)offset) >> 1);
		if ( offset < 1. )
			break;
		nNewBankOffset = nNewBankOffset * 2;
	}
	if (nNewBankOffset > 0 && nNewBankOffset < 1024)
		;
	else
		nNewBankOffset = bankoffset;
	return nNewBankOffset;
}

//TVB590S, TVB593
int CheckPllLock(int nBoardID)
{
	long nStatus = 0, nTryCount = 100;
	do
	{
		nStatus = TSPL_GET_AD9775_EX(nBoardID, 0);
		if ( (nStatus >> 1) == 0 )
		{
			nStatus = 0;
		}
		else
		{
			TSPL_WRITE_CONTROL_REG_EX(nBoardID, 0, 0x55100, 1);

			nStatus = 1;
			break;
		}

		Sleep(100);
	} while ( --nTryCount > 0 );

	return nStatus;
}
