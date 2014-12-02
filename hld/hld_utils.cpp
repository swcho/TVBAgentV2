
#if defined(WIN32)
#include	<Windows.h>
#include	<time.h>
#else
#define _FILE_OFFSET_BITS 64
#include	<sys/time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#endif
#include	<stdio.h>
#include	<math.h>

#include	"../include/hld_structure.h"

#ifdef WIN32
#else
#include	"../include/lld_const.h"
#endif

#include	"hld_utils.h"


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
CHldUtils::CHldUtils(void)
{
	TSSyncLock = FALSE;


#if defined(WIN32)
	GetCurrentDirectory(MAX_PATH, g_szCurDir);
#else
	getcwd(g_szCurDir, MAX_PATH);
#endif

	TL_TestPacketType = NO_BERT_OPERTION;
	TL_fp_14_15 = NULL;
	TL_fp_18_23 = NULL;

//CKIM A 20120817 {
	pTsInfo = NULL;
//CKIM A 20120817 }

}

CHldUtils::~CHldUtils()
{
	if ( TL_fp_14_15 )
	{
		fclose(TL_fp_14_15);
	}
	if ( TL_fp_18_23 )
	{
		fclose(TL_fp_18_23);
	}
}
void	CHldUtils::SetCommonMethod_80(CHldGVar	*__sta__, CHldBdLog	*__hLog__)
{
	__Sta_	=	__sta__;
	__CtlLog_	=	__hLog__;
}

int CHldUtils::CheckExtention(char *szFilePath, char *szExt)
{
	char *pdest;
	char tempFilePath[512];

	strcpy(tempFilePath, szFilePath);
	for(int i = 0; i < strlen(tempFilePath); i++)
	{
		tempFilePath[i] = tolower(tempFilePath[i]);
	}
	
	int result, len, ext_len;

	len = strlen(tempFilePath);
	ext_len = strlen(szExt);
	pdest = strstr(tempFilePath, szExt);
	result = pdest - tempFilePath + 1;

	return result == (len - ext_len + 1) ? 1 : 0;
}

#if defined(WIN32)
unsigned long	CHldUtils::_msec_(void)
{
	unsigned long cur_msec;

	cur_msec = timeGetTime();
	return	cur_msec;
}
unsigned long	CHldUtils::_past_msec_(int _init)
{
	unsigned long cur_msec, _ret;

	if (_init)
	{
		_prev_msec = _msec_();
		return	0;
	}
	cur_msec = _msec_();
	if (cur_msec >= _prev_msec)
	{
		_ret = (cur_msec - _prev_msec);
	}
	else
	{
		_ret = 0;
	}
	_prev_msec = cur_msec;

	return	_ret;
}
#else
long long int	CHldUtils::_msec_(void)
{
	long long int	cur_msec;
	struct	timeval	tv;
	//struct	timezone	tz;

	gettimeofday(&tv, NULL);
	cur_msec = ((long long int)tv.tv_sec*(long long int)1000) + (long long int)(tv.tv_usec/1000);

	return	cur_msec;
}
#endif

//CKIM A 20120625 {
class CPcrDeltaRecorder
{
private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		iMaxNoPcrDelta = 3
	};
#else
	static const int iMaxNoPcrDelta = 3;
#endif
	struct { WORD wPid; __int64 iiPcr1; int ix1; __int64 iiPcr2; int ix2; int iNoPcr2; } PcrDeltaPool[iMaxNoPcrDelta];
	int iNoPcrDelta;

public:
	CPcrDeltaRecorder()
	{
		int i;

		iNoPcrDelta = 0;
		for (i = 0; i < iMaxNoPcrDelta; ++i)
		{
			PcrDeltaPool[i].wPid = 0x1fff;
			PcrDeltaPool[i].iiPcr1 = 0;
			PcrDeltaPool[i].iiPcr2 = 0;
			PcrDeltaPool[i].iNoPcr2 = 0;
		}
	}

	int Record_PCR(WORD wPid, __int64 iiPcr, int ix)
	{
		int i;

		for (i = 0; i < iNoPcrDelta; ++i)
		{
			if (wPid == PcrDeltaPool[i].wPid)
				break;
		}
		if (i < iNoPcrDelta)
		{
			//--- check if round-robin
			if (iiPcr < PcrDeltaPool[i].iiPcr1)
			{
				PcrDeltaPool[i].iiPcr1 = iiPcr;
				PcrDeltaPool[i].ix1 = ix;
				PcrDeltaPool[i].iNoPcr2 = 0;
			} 
			else 
			{
				PcrDeltaPool[i].iiPcr2	= iiPcr;
				PcrDeltaPool[i].ix2 = ix;
				PcrDeltaPool[i].iNoPcr2++;
			}
		}
		else if (iNoPcrDelta < iMaxNoPcrDelta)
		{
			PcrDeltaPool[i].wPid = wPid;
			PcrDeltaPool[i].iiPcr1 = iiPcr;
			PcrDeltaPool[i].ix1 = ix;
			++iNoPcrDelta;
		}
		else
		{
			return 0;
		}

		return PcrDeltaPool[i].iNoPcr2;
	}

	void Get_PCR_Delta(__int64 * piiPcr1, int * pix1, __int64 * piiPcr2, int * pix2)
	{
		int iMaxNoPcr2;
		int iMaxNoPcr2Index;
		int i;

		if (iNoPcrDelta == 0)
		{
			*piiPcr1 = 0;
			*piiPcr2 = 0;

			return;
		}

		iMaxNoPcr2 = 0;
		iMaxNoPcr2Index = 0;
		for (i = 0; i < iNoPcrDelta; ++i)
		{
			if ((PcrDeltaPool[i].iNoPcr2 > iMaxNoPcr2) && (PcrDeltaPool[i].wPid != 0x1FFF))
			{
				iMaxNoPcr2 = PcrDeltaPool[i].iNoPcr2;
				iMaxNoPcr2Index = i;
			}
		}
		*piiPcr1 = PcrDeltaPool[iMaxNoPcr2Index].iiPcr1;
		*pix1 = PcrDeltaPool[iMaxNoPcr2Index].ix1;
		*piiPcr2 = PcrDeltaPool[iMaxNoPcr2Index].iiPcr2;
		*pix2 = PcrDeltaPool[iMaxNoPcr2Index].ix2;
	}
};
//2012/12/14
int CHldUtils::Get_MPE_Packet_PID(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize)
{
	DWORD	dwRemained = dwSize;
	int		index;
	WORD	wPID;
	WORD	w1PID;		// First PCR PID


	//ATSC-M/H
	int MHE_packet_PID = 0, MHE_packet_index = -1, MHE_packet_count = 0, _mh_pid = 0, _mh_ind;

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
			if (!(bData[index + 1] & 0x80))
			{
				wPID = (bData[index+1]&0x1F)*256+bData[index+2];
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
					_mh_ind = index - PKTSIZE*(MHE_packet_count-1);
					_mh_pid = (bData[_mh_ind+1]&0x1F)*256+bData[_mh_ind+2];
					return _mh_pid;
				}
			}
		}
		else
		{
			break;
		}
		index += iTSize;
		dwRemained -= iTSize;
	}
	return _mh_pid;
}
//CKIM A 20120625 }
//------------------------------------------------------------------------
// iTSize = 188,204,208
int CHldUtils::TL_Calculate_Bitrate(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize, int *mhe_pkt_pid, int *mhe_pkt_ind)
{
	DWORD	dwRemained = dwSize;
	int		index;
	WORD	wPID;
	WORD	w1PID;		// First PCR PID
	BYTE	bADExist;
	BYTE	bADLen;

	__int64	iiPCR1 = 0;
	__int64	iiPCR2 = 0;
	int		iNoPkt = 0;
	int		ix1, ix2;
	int		iNoPCR2 = 0;
//CKIM A 20120625 {
	CPcrDeltaRecorder pdrPcrDeltaRecorder;
//CKIM A 20120625 }

#if 0
	//ATSC-M/H
	int MHE_packet_PID = 0, MHE_packet_index = -1, MHE_packet_count = 0, _mh_pid, _mh_ind;
	if (mhe_pkt_pid == NULL)
	{
		mhe_pkt_pid = &_mh_pid;
		mhe_pkt_ind = &_mh_ind;
	}
	*mhe_pkt_pid = 0;
	*mhe_pkt_ind = -1;
#endif

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
//CKIM A 20120625 {
			if (!(bData[index + 1] & 0x80))
			{
//CKIM A 20120625 }
			wPID = (bData[index+1]&0x1F)*256+bData[index+2];
#if 0
			if ( __Sta_->IsModTyp_AtscMH() )	//	why is this code at here?
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
					*mhe_pkt_ind = index - PKTSIZE*(MHE_packet_count-1);
					*mhe_pkt_pid = (bData[*mhe_pkt_ind+1]&0x1F)*256+bData[*mhe_pkt_ind+2];
				}
			}
#endif			
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
//CKIM M 20120625 {
/*
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
*/
					if (pdrPcrDeltaRecorder.Record_PCR(wPID, iiPCR, iNoPkt) > 10)
						break;
//CKIM M 20120625 }
				}
			}
//CKIM A 20120625 {
			}
//CKIM A 20120625 }
		} else  
		{
			// Out of Sync: Stop PCR Checking
			break;
		}
		
		index += iTSize;
		iNoPkt++;
		dwRemained -= iTSize;
	}
	//ATSC-M/H
	//fclose(fp_AtscMH);

	//--- check PUSI for PTS/DTS/ESCR replace
	/* ISDB-T(=9), ISDB-T-13(=10), ISDB-S(=17) */
	int nPacketSize = PKTSIZE;
	if ( __Sta_->IsModTyp_IsdbT_1() || __Sta_->IsModTyp_IsdbT_13())
	{
		nPacketSize = iTSize;//204, 208
	}
	//sskim20081231 - fixed
#ifdef TSPHLD0104_EXPORTS
	nPacketSize = iTSize;//204, 208
#endif

//CKIM A 20120625 {
	pdrPcrDeltaRecorder.Get_PCR_Delta(&iiPCR1, &ix1, &iiPCR2, &ix2);
//CKIM A 20120625 }
	if (iiPCR2 > iiPCR1) 
	{
		double		iDiffTime = (double) iiPCR2-iiPCR1;
		/* ISDB-T(=9) */
		double		iLen	  = (ix2-ix1)*nPacketSize*8;
		double		iBitrate  = iLen*27000000/iDiffTime;

		return (int) iBitrate;
	} else {		// Can't Calculate bitrate
		return -1;
	}

	return 0;
}


int	CHldUtils::SyncPosAssume188(char *buf, int max_len)
{
	int	i_null, i_sync;

	i_sync = -1;
	for (i_null = 0; i_null < 188; i_null++)
	{
		if ((buf[i_null] == 0x47) && (buf[i_null + 188] == 0x47) && (buf[i_null + 188 + 188] == 0x47))
		{
			i_sync = i_null;
			break;
		}
	}
	return	i_sync;
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
int	CHldUtils::TL_SyncLockFunction(char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step)	//	find pkt size. one of 188/192/204/208
{
	int		i, j;
	BOOL	err;

	if (nlen < nlen_srch)
		return -1;

	for (i = 0; i < nlen_srch; i++)
	{
		if (szBuf[i] == 0x47) {

			//-----------------------------------------------
			// check if 188 bytes TS Stream ?
			err = FALSE;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*PKTSIZE + i] != 0x47) {
					err = TRUE;
					break;
				}
			}

			if (err == FALSE) {
				*iSize = PKTSIZE;
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

			//FIXED
			//-----------------------------------------------
			// check if 192 bytes BDAV MPEG-2 TS Stream ?
			err = FALSE;
			for (j = 0; j < nlen_step; j++) {
				if (szBuf[j*192 + i] != 0x47) {
					err = TRUE;
					break;
				}
			}

			if (err == FALSE) {
				*iSize = 192;
				return i;
			}
		}

	}

	return -1;
}
//2011/6/13 ISDB-S ASI IN/OUTPUT ============================================>>
int	CHldUtils::TL_SyncLockFunction_ISDBS(unsigned char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step) 
{
	int		i;
	*iSize = 0;
	if (nlen < nlen_srch)
		return -1;

	for (i = 0; i < nlen_srch; i++)
	{
		if (szBuf[i] == 0x1B)
		{
			if(szBuf[(i + 204)] == 0x95)
			{
				if(szBuf[(i + (204 * 10))] == 0xA3)
				{
					if(szBuf[(i + (204 * 11))] == 0x40) {
						*iSize = 204;
						return i;
					}
				}
			}
		}
	}
	return -1;
}

//----------------------------------------------------------------------------------
//
//	return -1 when no sync is found, else return offset pointer for where sycn is found
//
//----------------------------------------------------------------------------------
int	CHldUtils::TL_SearchSync(int dwDMABufSize)
{
	int	nSyncStartPos = -1;
	int	iTSize = 0;

#ifdef TSPHLD0110_EXPORTS
#if	1
	__CtlLog_->HldPrint("Hld-Bd-Ctl. : not used src-pos : eee ");
#else
	if ((nSyncStartPos = TSPL_GET_SYNC_POSITION(TL_nSyncStatus, 0, 
		(unsigned char *)TL_pdwDMABuffer, dwDMABufSize, dwDMABufSize, 3)) == -1) 
	{
		return -1;
	}
	
	iTSize = TSPL_GET_SYNC_POSITION(TL_nSyncStatus, 1, 
				(unsigned char *)TL_pdwDMABuffer+nSyncStartPos, 
				dwDMABufSize-nSyncStartPos, dwDMABufSize-nSyncStartPos, 4);

	if ( iTSize == -1 )
	{
		return -1;
	}
	else
	{
		return (nSyncStartPos+iTSize);
	}
#endif
#else

	//////////////////////////////////////////////////////////////////////
	//sskim20080724 - BERT
	#if 1
	if ( TL_TestPacketType >= TS_HEAD_184_ALL_0 )
	{
		//fixed - 6.10.05, 20 -> 3, 5630 -> dwDMABufSize
		if ((nSyncStartPos = TL_SyncLockFunction((char *)TL_pdwDMABuffer, dwDMABufSize, &iTSize, dwDMABufSize, 3)) == -1) 
		{
			return -1;
		}
		else 
		{
			TL_gPacketSize = iTSize;
			/*
			iTSize = TL_Calculate_Bitrate((unsigned char *)TL_pdwDMABuffer, 
				dwDMABufSize, nSyncStartPos, TL_gPacketSize);
			if ( iTSize < 100 )
			{
				return -1;
			}
			TL_gBitrate = iTSize;
			*/

			return nSyncStartPos;
		}
	}
	#endif
	//////////////////////////////////////////////////////////////////////

	//fixed - 6.10.05, 20 -> 3, 5630 -> dwDMABufSize
	if ((nSyncStartPos = TL_SyncLockFunction((char *)TL_pdwDMABuffer, 
		dwDMABufSize, &iTSize, dwDMABufSize, 3)) == -1) 
	{
		return -1;
	}
	else 
	{
		return nSyncStartPos;
	}
	

#endif
}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

static	char			szPATSection[1024] = "No PAT";
static	char			szPMTSection[1024] = "No PMT";
static	unsigned char	ParseBuff[MAX_PID_TBL][1024+188];
static	unsigned int	nSIBuff[MAX_PID_TBL];
static	unsigned int	pid_tbl[MAX_PID_TBL];

static	unsigned int	pat_tbl[MAX_PID_SIZE];
static	unsigned int	pmt_tbl[MAX_PID_SIZE][MAX_PID_TBL];
static	unsigned int	pmt_tbl_type[MAX_PID_SIZE][MAX_PID_TBL];
static	unsigned int	pmt_tbl_pcr[MAX_PID_SIZE];
//CKIM A 20120628 {
static int have_pat;
//CKIM A 20120628 }

//
//	initialize buffer pointer
//
//	pid_tbl[0] = 0x00 - PAT
//	pid_tbl[1..15] = PMT_PID[i-1], 0x10,0x20,0x30,0x40,0x50,..0xF0
//
int	CHldUtils::InitParsePID(void)
{
	int	i;
	for (i = 0; i < MAX_PID_TBL; i++)
	{
		nSIBuff[i] = 0;
	}
	return 0;
}
int	CHldUtils::InitPIDTable(void)
{
	int	i,j;
	for (i = 0; i < MAX_PID_TBL; i++)
	{
		nSIBuff[i] = 0;
		pid_tbl[i] = 0x3fff;	// i << 4;	// default pat,pmt pid
	}
	pid_tbl[0] = 0;

	for (i = 0; i < MAX_PID_SIZE; i++)
	{
		pat_tbl[i] = 0xffff;

		pmt_tbl_pcr[i] = 0xffff;
		for (j = 0; j < MAX_PID_TBL; j++)
		{
			pmt_tbl[i][j] = 0xffff;
			pmt_tbl_type[i][j] = 0xffff;
		}
	}
//CKIM A 20120628 {
	have_pat = 0;
//CKIM A 20120628 }

	return 0;
}

//
//	Parsed result
//		num_of_entry + (program_number + pid)*num_of_entry
//		update automatically by checking version number
//
int	CHldUtils::ParsePATsection(unsigned char *dbptr, int sec_len)
{
	int	i,j,k=1;//k==0 is PAT table
	/*
	sprintf(szPATSection,"[PAT] ts_id 0x%02x%02x, ver %i, sec %i/%i \n",
		(int) dbptr[3],	
		(int) dbptr[4],	
		(int) ((dbptr[5] & 0x1f) >> 1),	
		(int) dbptr[6],  
		(int) dbptr[7]); 
	HldPrint(szPATSection);
	*/
	j = 5;
	while ((j < (sec_len-4)))
	{
		i = ((int)(dbptr[j+5] & 0x1f) << 8 ) + (int)(dbptr[j+6]);//PID
		pat_tbl[i] = ((int)(dbptr[j+3]) << 8 ) + (int)(dbptr[j+4]);//PGM
//CKIM M 20120629 {
//		pid_tbl[k++] = i;
		for (k = 1; k < MAX_PID_TBL; ++k)
		{
			if (pid_tbl[k] == i || pid_tbl[k] == 0x3fff)
				break;
		}
		if (k < MAX_PID_TBL)
			pid_tbl[k] = i;
//CKIM M 20120629 }
		/*	
		sprintf(szPATSection,"pgm[0x%02x%02x] PID[0x%02x%02x]  \n",
			(int)(dbptr[j+3]),(int)(dbptr[j+4]),
			(int)(dbptr[j+5] & 0x1f),(int)(dbptr[j+6]));
		HldPrint(szPATSection);
		*/
		j += 4;
	}
	/*
	sprintf(szPATSection,"CRC:%02x%02x%02x%02x\n\n",
		(int)(dbptr[j+3]),(int)(dbptr[j+4]),
		(int)(dbptr[j+5] & 0x1f),(int)(dbptr[j+6]));
	HldPrint(szPATSection);
	*/
//CKIM A 20120628 {
	have_pat = 1;
//CKIM A 20120628 }
	return 0;
}

//
//	Parsed result
//		pcr_pid
//		program_number, (stream_type, pid)[]
//
int	CHldUtils::ParsePMTsection(unsigned char *dbptr, int sec_len)
{
	int	i,j,k,l=0;
	int	desc_len,pcr,type;
	desc_len = (dbptr[10] & 0x0f)*256+dbptr[11];

	i = ((int)dbptr[3] << 8) + (int)dbptr[4];//PGM #
	pcr = ((int)(dbptr[8] & 0x1f) << 8) + (int)dbptr[9];
	/*
	sprintf(szPMTSection,"[PMT] pgm_n 0x%02x%02x, ver %i, sec %i/%i pcr 0x%02x%02x desc_len %i\n",
		(int) dbptr[3],
		(int) dbptr[4],
		(int) ((dbptr[5] & 0x1f) >> 1),	
		(int) dbptr[6],
		(int) dbptr[7],
		(int) (dbptr[8] & 0x1f), (int)dbptr[9],		// PCR_PID
		desc_len); // program info length
	HldPrint(szPMTSection);
	*/
	j = desc_len+12;
	while ((j < (sec_len-4)))
	{
		k = ((int)(dbptr[j+1] & 0x1f) << 8) + (int)(dbptr[j+2]);//PID
		type = (int)(dbptr[j]);
		pmt_tbl[k][l] = i;
		pmt_tbl_type[k][l++] = type;//TYPE
		pmt_tbl_pcr[k] = pcr;//PCR
		
		desc_len = (dbptr[j+3] & 0x0f)*256+dbptr[j+4];
		/*
		sprintf(szPMTSection,"stream_type 0x%x PID[0x%02x%02x] desc %i \n",
			(int)(dbptr[j]),
			(int)(dbptr[j+1] & 0x1f),(int)(dbptr[j+2]),desc_len);
		HldPrint(szPMTSection);
		*/
		j += desc_len+5;
	}
	/*
	sprintf(szPMTSection,"CRC:%02x%02x%02x%02x\n\n",
		(int)(dbptr[j+3]),(int)(dbptr[j+4]),
		(int)(dbptr[j+5] & 0x1f),(int)(dbptr[j+6]));
	HldPrint(szPMTSection);
	*/
	return 0;
}

void CHldUtils::InitNIT_Descriptor_variables()
{
	st_Satellite.i_descriptor_flag = 0; 
	st_Satellite.i_frequeny = 0;
	st_Satellite.i_roll_off = -1;
	st_Satellite.i_modulation_system = -1;
	st_Satellite.i_modulation = -1;
	st_Satellite.i_symbolrate = 0;
	st_Satellite.i_coderate = -1;

	st_Cable.i_descriptor_flag = 0;
	st_Cable.i_frequeny = 0;	
	st_Cable.i_fec_outer = -1;
	st_Cable.i_modulation = -1;
	st_Cable.i_symbolrate = 0;
	st_Cable.i_coderate = -1;
	
	st_terrestrial.i_descriptor_flag = 0;
	st_terrestrial.ui_frequency = 0;
	st_terrestrial.i_bandwith = -1;
	st_terrestrial.i_priority = -1;
	st_terrestrial.i_time_slicing_indicator = -1;
	st_terrestrial.i_mpe_fec_indicator = -1;
	st_terrestrial.i_constellation = -1;
	st_terrestrial.i_hierarchy_information = -1; 
	st_terrestrial.i_code_rate_hp_stream = -1;
	st_terrestrial.i_code_rate_lp_stream = -1;
	st_terrestrial.i_guard_interval = -1;
	st_terrestrial.i_transmission_mode = -1;
}
void CHldUtils::GetSatelliteDescriptorInfo(int *desc_tag, int *freq, int *rolloff, int *mod_sys, 
										   int *mod_typ, int *symbol, int *code)
{
	*desc_tag = st_Satellite.i_descriptor_flag;
	*freq = st_Satellite.i_frequeny;
	*rolloff = st_Satellite.i_roll_off;
	*mod_sys = st_Satellite.i_modulation_system;
	*mod_typ = st_Satellite.i_modulation;
	*symbol = st_Satellite.i_symbolrate;
	*code = st_Satellite.i_coderate;
}
void CHldUtils::GetCableDescriptorInfo(int *desc_tag, int *freq, int *mod_typ, 
									   int *symbol, int *code)
{
	*desc_tag = st_Cable.i_descriptor_flag;
	*freq = st_Cable.i_frequeny;
	*mod_typ = st_Cable.i_modulation;
	*symbol = st_Cable.i_symbolrate;
	*code = st_Cable.i_coderate;

}
void CHldUtils::GetTerrestrialDescriptorInfo(int *desc_tag, unsigned int *freq, int *bw, int *time_slicing, int *mpe_fec, 
								int *constellation, int *code, int *guard, int *txmod)
{
	*desc_tag = st_terrestrial.i_descriptor_flag;
	*freq = st_terrestrial.ui_frequency;
	*bw = st_terrestrial.i_bandwith;
	*constellation = st_terrestrial.i_constellation;
	if(st_terrestrial.i_hierarchy_information == 0)
	{
		*code = st_terrestrial.i_code_rate_hp_stream;
	}
	else
	{
		if(st_terrestrial.i_priority == 1)
			*code = st_terrestrial.i_code_rate_hp_stream;
		else
			*code = st_terrestrial.i_code_rate_lp_stream;
	}
	*time_slicing = st_terrestrial.i_time_slicing_indicator;
	if(st_terrestrial.i_hierarchy_information > 3)
	{
		*time_slicing = *time_slicing + 2;
	}
	*mpe_fec = st_terrestrial.i_mpe_fec_indicator;
	*guard = st_terrestrial.i_guard_interval;
	*txmod = st_terrestrial.i_transmission_mode;
}

void CHldUtils::ParseDescriptor(unsigned char *dbptr, int descTag)
{
	char debugBuf[256];
	int index = 0;

	switch(descTag)
	{
	case 0x40:	//network_name_descriptor
		break;
	case 0x41:	//service_list_descriptor
		break;
	case 0x42:	//stuffing_descriptor
		break;
	case 0x43:	//satellite_delivery_system_descriptor
		st_Satellite.i_descriptor_flag = 1;
		st_Satellite.i_frequeny = (unsigned int)(dbptr[index] << 24) + (unsigned int)(dbptr[index+1] << 16) +
							(unsigned int)(dbptr[index+2] << 8) + (unsigned int)dbptr[index+3];
		st_Satellite.i_roll_off = (int)((dbptr[index+6] & 0x18)>>3);
		st_Satellite.i_modulation_system = (int)((dbptr[index+6] & 0x4)>>2);
		st_Satellite.i_modulation = (int)((dbptr[index+6] & 0x3));
		st_Satellite.i_symbolrate = (int)(dbptr[index+7] << 20) + (int)(dbptr[index+8] << 12) +
							(int)(dbptr[index+9] << 4) + (int)((dbptr[index+10] & 0xf0) >> 4);
		st_Satellite.i_coderate = (int)(dbptr[index+10] & 0x0f);
//		sprintf(debugBuf, "FREQ : 0x%x, ROLL OFF : %d, ModSys : %d, ModType : %d, SymbolRate : 0x%x, fec_inner : %d\n",
//				st_Satellite.i_frequeny, st_Satellite.i_roll_off, st_Satellite.i_modulation_system, st_Satellite.i_modulation, st_Satellite.i_symbolrate, st_Satellite.i_coderate);
//		OutputDebugString(debugBuf);
		break;
	case 0x44:	//cable_delivery_system_descriptor
		st_Cable.i_descriptor_flag = 1;
		st_Cable.i_frequeny = (unsigned int)(dbptr[index] << 24) + (unsigned int)(dbptr[index+1] << 16) +
							(unsigned int)(dbptr[index+2] << 8) + (unsigned int)dbptr[index+3];
		st_Cable.i_fec_outer = (int)((dbptr[index+7] & 0x4));
		st_Cable.i_modulation = (int)((dbptr[index+8]));
		st_Cable.i_symbolrate = (int)(dbptr[index+9] << 20) + (int)(dbptr[index+10] << 12) +
							(int)(dbptr[index+11] << 4) + (int)((dbptr[index+12] & 0xf0) >> 4);
		st_Cable.i_coderate = (int)(dbptr[index+12] & 0x0f);
//		sprintf(debugBuf, "FREQ : 0x%x, FEC Outer : %d, ModType : %d, SymbolRate : 0x%x, fec_inner : %d\n",
//				st_Cable.i_frequeny, st_Cable.i_fec_outer, st_Cable.i_modulation, st_Cable.i_symbolrate, st_Cable.i_coderate);
//		OutputDebugString(debugBuf);
		break;
	case 0x4A:	//linkage_descriptor
		break;
	case 0x5A:	//terrestrial_delivery_system_descriptor
		st_terrestrial.i_descriptor_flag = 1;
		st_terrestrial.ui_frequency = (unsigned int)(dbptr[index] << 24) + (unsigned int)(dbptr[index+1] << 16) +
							(unsigned int)(dbptr[index+2] << 8) + (unsigned int)dbptr[index+3];
		st_terrestrial.i_bandwith = (int)((dbptr[index+4] & 0xE0) >> 5);
		st_terrestrial.i_priority = (int)((dbptr[index+4] & 0x10) >> 4);
		st_terrestrial.i_time_slicing_indicator = (int)((dbptr[index+4] & 0x08) >> 3);
		st_terrestrial.i_mpe_fec_indicator = (int)((dbptr[index+4] & 0x04) >> 2);
		st_terrestrial.i_constellation = (int)((dbptr[index+5] & 0xC0) >> 6);
		st_terrestrial.i_hierarchy_information = (int)((dbptr[index+5] & 0x38) >> 3);
		st_terrestrial.i_code_rate_hp_stream = (int)((dbptr[index+5] & 0x03));
		st_terrestrial.i_code_rate_lp_stream = (int)((dbptr[index+6] & 0xE0) >> 5);
		st_terrestrial.i_guard_interval = (int)((dbptr[index+6] & 0x18) >> 3);
		st_terrestrial.i_transmission_mode = (int)((dbptr[index+6] & 0x06) >> 1);
//		sprintf(debugBuf, "FREQ : 0x%x, Bandwidth : %d, priority : %d, timeslicing : %d, mpefec : %d, constellation : %d, hierarchy_information : %d, code_rate-HP_stream : %d, code_rate-LP_stream : %d, guardinterval : %d, transmission_mode : %d\n",
//				st_terrestrial.ui_frequency, st_terrestrial.i_bandwith, st_terrestrial.i_priority, st_terrestrial.i_time_slicing_indicator, st_terrestrial.i_mpe_fec_indicator, st_terrestrial.i_constellation, 
//				st_terrestrial.i_hierarchy_information, st_terrestrial.i_code_rate_hp_stream, st_terrestrial.i_code_rate_lp_stream, st_terrestrial.i_guard_interval, st_terrestrial.i_transmission_mode);
//		OutputDebugString(debugBuf);
		break;
	case 0x5B:	//multilingual_network_name_descriptor
		break;
	case 0x5F:	//private_data_specifier_descriptor
		break;
	case 0x62:	//frequency_list_descriptor
		break;
	case 0x6C:	//cell_list_descriptor
		break;
	case 0x6D:	//cell_frequency_link_descriptor
		break;
	case 0x73:	//default_authority_descriptor
		break;
	case 0x77:	//time_slice_fec_identifier_descriptor
		break;
	case 0x79:	//S2_satellite_delivery_system_descriptor
		break;
	case 0x7D:	//XAIT location descriptor
		break;
	case 0x7F:	//extension descriptor
		break;
	default:
		break;
	}
}

int	CHldUtils::ParseNITsection(unsigned char *dbptr, int sec_len)
{
	int network_id;
	int netDescLength;
	int descTag;
	int descLength = 0;
	int totDescLength = 0;
	int totloopLength = 0;
	int tsLoopLength;
	int ts_id;
	int org_net_id;
	int transportDescLength;
	int index = 0;
	char debugBuf[256];
	unsigned char DescBuf[256];

	index = 1;
	network_id = ((int)((dbptr[index] & 0x0f) << 8)) + (int)dbptr[index + 1];
	index = 8;
	netDescLength = ((int)((dbptr[index] & 0x0f) << 8)) + (int)dbptr[index + 1];
//	sprintf(debugBuf, "sec_len : %d\nFIND NIT!!!\nNETWORK ID : 0x%x, network descriptors length : %d\n", sec_len, 
//		network_id, netDescLength);
//	OutputDebugString(debugBuf);
	index = 10;
	//network descriptor
	for(;;)
	{
		
		descTag = (int)dbptr[index];
		descLength = (int)dbptr[index + 1];
		index = index + 2;
		if((index + descLength) > sec_len)
			return 0;
//		sprintf(debugBuf, "descTag : 0x%x, descLength : %d\n", descTag, descLength); 
//		OutputDebugString(debugBuf);
		memcpy(DescBuf, &dbptr[index], descLength);
		ParseDescriptor(DescBuf, descTag);

		totDescLength = totDescLength + (descLength + 2);
		index = index + descLength;
		if(netDescLength <= totDescLength)
			break;
	}
	//transport stream loop length
	tsLoopLength = ((int)((dbptr[index] & 0x0f) << 8)) + (int)dbptr[index + 1];
//	sprintf(debugBuf, "transport stream loop length : %d\n", tsLoopLength); 
//	OutputDebugString(debugBuf);
	index = index + 2;
	for(;;)
	{
		ts_id = ((int)(dbptr[index] << 8)) + (int)dbptr[index + 1];
		index = index + 2;
		org_net_id = ((int)(dbptr[index] << 8)) + (int)dbptr[index + 1];
		index = index + 2;
		transportDescLength = ((int)((dbptr[index] & 0x0f) << 8)) + (int)dbptr[index + 1];
		index = index + 2;
		if((index + transportDescLength) > sec_len)
			return 0;
//		sprintf(debugBuf, "ts_id : 0x%x, org_net_id : 0x%x, transportDescLength : %d\n", ts_id, org_net_id, transportDescLength); 
//		OutputDebugString(debugBuf);
		totDescLength = 0;
		for(;;)
		{
			descTag = (int)dbptr[index];
			descLength = (int)dbptr[index + 1];
			index = index + 2;
			if((index + descLength) > sec_len)
				return 0;
//			sprintf(debugBuf, "TS descTag : 0x%x, descLength : %d\n", descTag, descLength); 
//			OutputDebugString(debugBuf);
			
			memcpy(DescBuf, &dbptr[index], descLength);
			ParseDescriptor(DescBuf, descTag);

			totDescLength = totDescLength + (descLength + 2);
			index = index + descLength;
			if(transportDescLength <= totDescLength)
				break;
		}
		totloopLength = totloopLength + totDescLength;
		if(tsLoopLength <= totloopLength)
			break;
	}


	return 0;
}

//
//	SI decode
//
int	CHldUtils::si_decode(unsigned char *dbptr, int nSizeBuff)	// pointer to section start
{
	unsigned int	table_id;
	unsigned int	sec_len;

	if (nSizeBuff > 4)
	{
		//2012/5/23 PES data
		if(dbptr[0] == 0x00 && dbptr[1] == 0x00 && dbptr[2] == 0x01)
			return -1;

		table_id = *dbptr;
		sec_len = ((dbptr[1] & 0x0f) << 8) + dbptr[2];

		if ((sec_len > 1023)/* || (nSizeBuff < (int)sec_len)*/)	// corrupted PSI table
			return -1;

		switch (table_id) 
		{
			case 0x00:
				ParsePATsection(dbptr, sec_len);
				return 0;
				break;
			case 0x02: 
				ParsePMTsection(dbptr, sec_len);
				return 2;
				break;
			//2012/5/8 
			case 0x40:
			//case 0x41:
				ParseNITsection(dbptr, sec_len);
				return 0x40;
				//
				// add table parsing more
				//
			default:
				/*HldPrint(  "\n[XXX:%i:%i]",sec_len,nSizeBuff);*/
				break;
		}
		return -1;
	}
	return -1;
}
int	CHldUtils::RunTs_Parser(int nSlot, char *szFile, int default_bitrate, int nitFlag)
{
	FILE	*file_stream;
	int		syncStart, iTSize;
	size_t	readByte;
	BYTE	*AP_szBuffer;
	int nTryCount = MAX_REPEAT_TIME;
	HANDLE hFile;

	//2012/5/8 NIT
	int buf_pos;

#if defined(WIN32)
	hFile = CreateFile(szFile,	GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if ( hFile == INVALID_HANDLE_VALUE)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to open ");
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
	hFile = fopen(szFile, "rb");
	if ( hFile == NULL)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to open ");
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
	
	//-----------------------------------------------------------------------
	// Memory allocation
	AP_szBuffer = (unsigned char*)malloc(SUB_BANK_MAX_BYTE_SIZE);

	if (AP_szBuffer == NULL)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL alloc-memory");
		return	-1;
	}

	//-----------------------------------------------------------------------
	// Open and Read target file
	file_stream = fopen(szFile, "rb");
	if (file_stream == NULL)
	{
		__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to open FILE ");
		free (AP_szBuffer);
		return	-1;
	}

//CKIM D 20120706 {
/*
#if defined(WIN32)
	fseek(file_stream, dwSizeLow/2, SEEK_SET);
#else
	fseeko((FILE*)file_stream, dwSizeLow/2, SEEK_SET);
#endif
*/
//CKIM D 20120706 }
	//-----------------------------------------------------------------------
	// Create a PID information file to save

	char szDBpath[MAX_PATH];
	FILE *fp;
	int maxPid_searchRange;
	if(nitFlag == 0)
	{
#if defined(WIN32)
		sprintf(szDBpath, "%s\\%s_%d", g_szCurDir, PID_DB_PATH, nSlot);
#else
#ifdef STANDALONE
		sprintf(szDBpath, "/ramfs/%s_%d", PID_DB_PATH, nSlot);
#else
		sprintf(szDBpath, "%s/%s_%d", g_szCurDir, PID_DB_PATH, nSlot);
#endif
#endif
		fp = fopen(szDBpath, "wt");
		if (fp == NULL)
		{
			__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to open FILE ");
			free (AP_szBuffer);
			return	-1;
		}
//CKIM M 20120627 {
//		maxPid_searchRange = MAX_PID_SEARCH_RANGE;
		maxPid_searchRange = SUB_BANK_MAX_BYTE_SIZE * 32;
//CKIM M 20120627 }
	}
	else
	{
		fp = NULL;
		maxPid_searchRange = MAX_PID_SEARCH_RANGE;
	}
	//2012/5/9 NIT
	InitNIT_Descriptor_variables();

	long search_pos=0, cur_pos = 0, start_pos = 0;
//CKIM A 20120706 {
#define TS_SYNC_CHECK_LOOKAHAED_LEN	(208 * 3 + 1)
	int AP_szBuffer_offset = 0;
//CKIM A 20120706 }
	do
	{
#if defined(WIN32)
		search_pos = ftell(file_stream);
#else
		search_pos = ftello((FILE *)file_stream);
#endif
		//read stream data
//CKIM M 20120706 {
//		readByte = fread (AP_szBuffer,  1, SUB_BANK_MAX_BYTE_SIZE, file_stream);
//		if (readByte <= 0)
		readByte = fread (AP_szBuffer + AP_szBuffer_offset,  1, SUB_BANK_MAX_BYTE_SIZE - AP_szBuffer_offset, file_stream);
		if (AP_szBuffer_offset + readByte <= TS_SYNC_CHECK_LOOKAHAED_LEN)
//CKIM M 20120706 }
		{
			break;
		}

		//get a valid sync. position and packet size
//CKIM M 20120706 {
//		syncStart = TL_SyncLockFunction((char*)AP_szBuffer, readByte, &iTSize, readByte, 3);
		syncStart = TL_SyncLockFunction((char*)AP_szBuffer, AP_szBuffer_offset + readByte, &iTSize, AP_szBuffer_offset + readByte - TS_SYNC_CHECK_LOOKAHAED_LEN, 3);
//CKIM M 20120706 }
		if (syncStart != -1)
		{
//CKIM A 20120706 }
			int out_of_sync = 0;
//CKIM A 20120706 }
			{
//CKIM D 20120706 {
/*
#if defined(WIN32)
				fseek(file_stream, search_pos+syncStart, SEEK_SET);
#else
				fseeko((FILE *)file_stream, search_pos+syncStart, SEEK_SET);
#endif
*/
//CKIM D 2120706 }
				//6.9.14
				//start_pos = ftell(fp);
									
				unsigned char	skipbyte, ptr_field;
				int	buf_id, payload_unit_start_ind, Ret;
				unsigned int	nPid;
				int	nPsi_size;
				int	i,j,k,l;

				InitParsePID();
				InitPIDTable();

				//iterate until it finds two PCRs to estimate the bitrate */
				int pcr_pid, pid, nb_packets, nb_pcrs;
				__int64 pcrs[MAX_PID_SEARCH], pcr_h, pcr_offset=0;
				unsigned char packet[TS_PACKET_SIZE+20], *bData;
				char	text[100];

				int pcr_incr;
				int bit_rate;
				int count_packets = 0;
				//LAYER INFO ADDED - 1=A, 2=B, 3=C, only for ISDB-T
				int layer_info[MAX_PID_SIZE];
				
				int pid_packet_count[MAX_PID_SEARCH][MAX_PID_SIZE], pid_nb_packets[MAX_PID_SIZE];
				for ( i = 0; i < MAX_PID_SEARCH; i++ )
				{
					//kslee
					//memset(&pid_packet_count[i], 0x00000000, MAX_PID_SIZE);
					for(j = 0; j < MAX_PID_SIZE; j++)
					{
						pid_packet_count[i][j] = 0x0;
					}
					pcrs[i] = 0;
				}
				//kslee
				//memset(pid_nb_packets, 0x00000000, MAX_PID_SIZE);
				for(i = 0; i < MAX_PID_SIZE; i++)
				{
					pid_nb_packets[i] = 0x0;
//CKIM A 20120706 {	// need an initialization value for a pid referenced but not appeared 
					layer_info[i] = 0;
//CKIM A 20120706 }
				}	

				pcr_pid = -1;
				nb_pcrs = 0;
				nb_packets = 0;
				
				//2012/5/8 NIT
//CKIM M 20120706 {
/*
				buf_pos = 0;
				readByte = fread(AP_szBuffer,  1, SUB_BANK_MAX_BYTE_SIZE, file_stream);
				if (readByte <= 0)
				{
					break;
				}
				cur_pos = cur_pos + readByte;
*/
				buf_pos = syncStart;
				cur_pos = cur_pos + readByte;
				readByte += AP_szBuffer_offset;
//CKIM M 20120706 }
				
				for(;;) 
				{
#if defined(WIN32)
//					cur_pos = ftell(file_stream);
#else
//					cur_pos = ftello((FILE *)file_stream);
#endif
					//cur_pos -= start_pos;

//					if ( feof( file_stream ) 
//						|| cur_pos > (MAX_PID_SEARCH_RANGE+search_pos) 
//						|| nb_pcrs >= MAX_PID_SEARCH ) 
					if(cur_pos > maxPid_searchRange || nb_pcrs >= MAX_PID_SEARCH)
					{
							break;
					}
//					ret = read_packet(file_stream, packet, iTSize);
//					if (ret < 0)
//						break;
					//2012/5/8 NIT
//CKIM M 20120629 {
//					if((buf_pos + iTSize) > SUB_BANK_MAX_BYTE_SIZE)
					if((buf_pos + iTSize) > readByte)
//CKIM M 20120629 }
					{
						if(feof( file_stream ))
						break;

//CKIM M 20120630 {
//						int buf_pos_tmp = SUB_BANK_MAX_BYTE_SIZE - buf_pos;
						int buf_pos_tmp = readByte - buf_pos;
//CKIM M 20120630 }
//CKIM A 20120706 {
						if (buf_pos_tmp > 0)
						{
							if (*(AP_szBuffer + buf_pos) != 0x47)
							{
								memcpy(AP_szBuffer, AP_szBuffer + buf_pos, buf_pos_tmp);
								AP_szBuffer_offset = buf_pos_tmp;
								out_of_sync = 1;
								break;
							}							
						}
//CKIM A 20120706 }
						memcpy(packet, AP_szBuffer + buf_pos, buf_pos_tmp);
						readByte = fread(AP_szBuffer,  1, SUB_BANK_MAX_BYTE_SIZE, file_stream);
//CKIM A 20120706 {
						if (readByte < iTSize - buf_pos_tmp)
							break;
						if (buf_pos_tmp == 0)
						{
							if (*AP_szBuffer != 0x47)
							{
								AP_szBuffer_offset = 0;
								out_of_sync = 1;
								break;
							}							
						}
//CKIM A 20120706 }
						memcpy(packet+buf_pos_tmp, AP_szBuffer, iTSize - buf_pos_tmp);
						buf_pos = (iTSize - buf_pos_tmp);
						cur_pos = cur_pos + readByte; 
					}
					else
					{
//CKIM A 20120706 {
						if (*(AP_szBuffer + buf_pos) != 0x47)
						{
							int i;
							AP_szBuffer_offset = readByte - buf_pos;
							for (i = 0; i < AP_szBuffer_offset; ++i)
							{
								AP_szBuffer[i] = AP_szBuffer[i + buf_pos];
							}
							out_of_sync = 1;
							break;
						}
//CKIM A 20120706 }
						memcpy(packet, AP_szBuffer + buf_pos, iTSize);
						buf_pos = buf_pos + iTSize;
					}

//CKIM A 20120627 {
					if (packet[1] & 0x80)
					{
						++nb_packets;
						continue;
					}
//CKIM A 20120627 }
					pid = (packet[1]&0x1F)*256+packet[2];
					
					//LAYER INFO ADDED
					layer_info[pid] = 1;
					if ( iTSize >= 204 )
					{
						if ( pid >= 0 && pid < MAX_PID_SIZE )
						{
							layer_info[pid] = (packet[189] >> 4) & 0x0F;
						}
					}
					
					//PCR searching....
					if ((pcr_pid == -1 || pcr_pid == pid))
					{
						if ((packet[3] & 0x20) && 
							packet[4] >= 7 && 
							(packet[5] & 0x10)) 
						{
							bData = &packet[6];

							//--- Get PCR Value
							pcr_h = bData[0];		
							pcr_h <<= 25;
							pcr_h += bData[1] << 17;
							pcr_h += bData[2] << 9;
							pcr_h += bData[3] << 1;
							pcr_h += (bData[4] & 0x80) >> 7;
							pcr_h *= 300;

							pcr_h += (bData[4] & 0x01) << 8;
							pcr_h += bData[5];

							pcr_pid = pid;
							//packet_count[nb_pcrs] = nb_packets;

							for ( i = 0; i < MAX_PID_SIZE; i++ )
							{
								pid_packet_count[nb_pcrs][i] = pid_nb_packets[i];
							}

							//pcrs[nb_pcrs] = pcr_h;
							if ( nb_pcrs > 0 && pcr_h < pcrs[nb_pcrs-1] )
							{
								if ( pcr_h + pcr_offset < pcrs[nb_pcrs-1] )
								{
									pcr_offset = pcrs[nb_pcrs-1] - pcr_h;
								}
							}
							pcrs[nb_pcrs] = pcr_h + pcr_offset;

							nb_pcrs++;
							if (nb_pcrs >= MAX_PID_SEARCH)
							{
								//break;
								nb_pcrs = MAX_PID_SEARCH-1;
							}
						}
						else
						{
							for ( i = 0; i < MAX_PID_SIZE; i++ )
							{
								pid_packet_count[nb_pcrs][i] = pid_nb_packets[i];
							}
						}
					}
					nb_packets++;

					//PIDs count store
					if ( pid >= 0 && pid < MAX_PID_SIZE )
					{
						//fixed
						if ((packet[1] & 0x80) == 0)
						{
							++pid_nb_packets[pid];
						}
					}

					//PAT/PMT parsing....
					if ((packet[1] & 0x80) == 0)
					{
						payload_unit_start_ind = ((packet[1] & 0x40) == 0x40);	
						nPid = packet[1] & 0x1f;
						nPid = (nPid << 8) + packet[2];	// PID[7..0]
						buf_id = 255;
						// compare with pid table
						for (i = 0; i < MAX_PID_TBL; i++)
						{
							if (nPid == pid_tbl[i])
							{
								buf_id = i;
								break;
							}
						}
//CKIM A 20120628 {
						if (!have_pat &&  buf_id == 255 && nPid != 0x1fff && (packet[1] & 0x40))
						{
							int pointer_field_offset;
							int table_id_offset;

							pointer_field_offset = 4;
							if (packet[3] & 0x20)
							{
								pointer_field_offset += 1 + packet[4]; 
							}
							table_id_offset = pointer_field_offset + 1 + packet[pointer_field_offset];
							if (packet[table_id_offset] == 0x02 && (packet[table_id_offset + 1] & 0xc0) == 0x80)
							{
								for (i = 0; i < MAX_PID_TBL; ++i)
								{
									if (pid_tbl[i] == 0x3fff)
										break;
								}
								if (i < MAX_PID_TBL)
								{
									pat_tbl[nPid] = (packet[table_id_offset + 3] << 8) + packet[table_id_offset + 4];
									pid_tbl[i] = nPid;
									buf_id = i;
								}

							}
						}
//CKIM A 20120628 }

						// pid match found
						if (buf_id != 255)		
						{
							if (payload_unit_start_ind)
							{	// decode previous fetch
								
								if (nSIBuff[buf_id])
									Ret = si_decode(ParseBuff[buf_id],nSIBuff[buf_id]);
								nSIBuff[buf_id] = 0;
								//2012/5/9 NIT
								if(nitFlag == 1)
								{
									if(Ret == 0)
									{
										for (i = 0; i < MAX_PID_TBL; i++)
										{
											//find NIT
											if (0x10 == pid_tbl[i])
											{
												break;
											}
										}
										//not found NIT
										if(i == MAX_PID_TBL)
										{
											fclose (file_stream);
											free (AP_szBuffer);
											return 0;
										}
									}
									if(st_Satellite.i_descriptor_flag == 1 || st_Cable.i_descriptor_flag == 1 || st_terrestrial.i_descriptor_flag == 1)
									{
										fclose (file_stream);
										free (AP_szBuffer);
										return 0;
									}
								}
							}
							if (packet[3] & 0x20)	
							{	// skip adaptation field 
								skipbyte = packet[4] + 1;
							}
							else
							{
								skipbyte = 0;
							}
							
							// pointer field for PSI
//CKIM A 20120706 {
							if (!payload_unit_start_ind)
								ptr_field = skipbyte+4;
							else
//CKIM A 20120706 }
							ptr_field = packet[4+skipbyte]+skipbyte+5;
							if (ptr_field < 187)
							{
//CKIM M 20120706 {
//								nPsi_size = ((packet[ptr_field+1] & 0x0f) << 8) + packet[ptr_field+2];
								if (nSIBuff[buf_id] > 0)
									nPsi_size = ((ParseBuff[buf_id][1] & 0x0f) << 8) + ParseBuff[buf_id][2] + 3 - nSIBuff[buf_id];
								else
									nPsi_size = ((packet[ptr_field+1] & 0x0f) << 8) + packet[ptr_field+2] + 3;
//CKIM M 20120706 }
								if (nPsi_size > (188-ptr_field))
								{
									nPsi_size = 188 - ptr_field;
								}
								//2012/5/23
								if((nSIBuff[buf_id] + nPsi_size) <= (1024 + 188))
								{
									memcpy(&ParseBuff[buf_id][nSIBuff[buf_id]],(packet+ ptr_field),nPsi_size);
									nSIBuff[buf_id] = (nSIBuff[buf_id] + nPsi_size);
								}
							}
							//2012/5/23
							if(nPid == 0x10 && payload_unit_start_ind == 1)
							{
								if (nSIBuff[buf_id])
									Ret = si_decode(ParseBuff[buf_id],nSIBuff[buf_id]);
							}
						}
					}
				}
//CKIM A 20120706 {
				if (!out_of_sync)
				{
//CKIM A 20120706 }
				if(nitFlag == 0)
				{
					//only one or no PCR packet found
					if ( nb_pcrs <= 1 )
					{
						count_packets = 0;
						for ( i = 0; i < MAX_PID_SIZE; i++ )
						{
							if ( pid_packet_count[0][i] > 0 )
								count_packets += pid_packet_count[0][i];
						}
					}

					//Logging...
					for ( i = 0; i < MAX_PID_SIZE; i++ )
					{
						//bitrate for each PIDs
						if ( nb_pcrs <= 1 )
						{
							if ( pid_packet_count[0][i] <= 0 )
//CKIM M 20120706 {
//							continue;
						{
							for (l = 0; l < MAX_PID_TBL; l++)
							{
								if (pmt_tbl[i][l] != 0xffff)
								{
									break;
								}
							}
							if (l == MAX_PID_TBL)
								continue;
						}
//CKIM M 20120706 }

//CKIM A 20120710 {
						if (nb_pcrs == 0)
							bit_rate = -1;
						else
//CKIM A 20120710 }
							if ( count_packets > 0 )
								bit_rate = (int)(((double)pid_packet_count[0][i] / (double)count_packets) *  (double)default_bitrate);
						}
						else
						{
							if ( nb_pcrs < MAX_PID_SEARCH-1 )
							{
								nb_pcrs = nb_pcrs-1;
							}

							int packet_count = pid_packet_count[nb_pcrs][i] - pid_packet_count[0][i];
							if ( packet_count <= 0 )
							{
//CKIM A 20120706 {
							for (l = 0; l < MAX_PID_TBL; l++)
							{
								if (pmt_tbl[i][l] != 0xffff)
								{
									bit_rate = 0;
									break;
								}
							}
							if (l == MAX_PID_TBL)
//CKIM A 20120706 }
								continue;
							}
//CKIM A 20120706 {
						else
						{
//CKIM A 20120706 }
							
							pcr_incr = (int)((pcrs[nb_pcrs] - pcrs[0]) / packet_count);
							if ( pcr_incr <= 0 ) 
								bit_rate = 0;
							else
								bit_rate = (int)((188 * 8) * 27e6 / pcr_incr);
//CKIM A 20120706 {
						}
//CKIM A 20120706 }
						}

						//PAT/PMT...
						if ((fp != NULL))
						{
							int is_pgm = 0;
							int is_pmt = 0;
							int is_pcr = 0;
							char tmp[32];
							int stream_type = 0;
							for (k = 0; k < MAX_PID_SIZE; k++)
							{
								if ( pat_tbl[k] != 0xffff )//PGM
								{
									for (j = 0; j < MAX_PID_SIZE; j++)
									{
										for (l = 0; l < MAX_PID_TBL; l++)
										{
											if ( pmt_tbl[j][l] == 0xffff )//PGM
												continue;
											
											if ( pmt_tbl[j][l] == pat_tbl[k] )
											{
												if ( j == i )
												{
													stream_type = pmt_tbl_type[j][l];
													if ( stream_type == 0x01 )	sprintf(tmp, "%s", "video");
													else if ( stream_type == 0x02 )	sprintf(tmp, "%s", "mpeg-2 video");
													else if ( stream_type == 0x03 )	sprintf(tmp, "%s", "mpeg-1 audio");
													else if ( stream_type == 0x04 )	sprintf(tmp, "%s", "mpeg-2 audio");
													else if ( stream_type == 0x05 )	sprintf(tmp, "%s", "private data");
													else if ( stream_type == 0x06 )	sprintf(tmp, "%s", "private data");
													else if ( stream_type == 0x0A )	sprintf(tmp, "%s", "13818-6");
													else if ( stream_type == 0x0B )	sprintf(tmp, "%s", "13818-6");
													else if ( stream_type == 0x0C )	sprintf(tmp, "%s", "13818-6");
													else if ( stream_type == 0x0D )	sprintf(tmp, "%s", "13818-6");
													else if ( stream_type == 0x0E )	sprintf(tmp, "%s", "13818-1");
													else if ( stream_type == 0x0F )	sprintf(tmp, "%s", "AAC audio");//"13818-7");
													else if ( stream_type == 0x10 )	sprintf(tmp, "%s", "14496-2");
													else if ( stream_type == 0x11 )	sprintf(tmp, "%s", "mpeg-4 audio");//"14496-3");
													else if ( stream_type == 0x12 )	sprintf(tmp, "%s", "14496-1");
													else if ( stream_type == 0x13 )	sprintf(tmp, "%s", "14496-1");
													else if ( stream_type == 0x14 )	sprintf(tmp, "%s", "13818-6");
													else if ( stream_type >= 0x15 && stream_type <= 0x19)
																					sprintf(tmp, "%s", "meta data");
													else if ( stream_type == 0x1B )	sprintf(tmp, "%s", "AVC/H.264 video");
													else if ( stream_type == 0x7F )	sprintf(tmp, "%s", "IPMP stream");
													else if ( stream_type == 0x81 )	sprintf(tmp, "%s", "Dolby-AC3 audio");
													else							sprintf(tmp, "%s", "");

													//LAYER INFO ADDED
													sprintf(text, "%d,%d,%d,%d,%s(%d),%d,%d\n", 
														i,							//PID
														k,							//PMT
														pat_tbl[k],					//PGM
														pmt_tbl_pcr[j],				//PCR
														tmp,pmt_tbl_type[j][l],		//STREAM TYPE 
														bit_rate,					//BITRATE
														layer_info[i]);			//LAYER INFO, only 204 TS		
													fputs(text, fp);

													is_pgm = 1;
													break;
												}
											}
										}
									}
								}
								if (is_pgm)
								{
									break;
								}
							}
							if (!is_pgm)
							{
								for (k = 0; k < MAX_PID_SIZE; k++)
								{
									if ( pat_tbl[k] > 0 && pat_tbl[k] != 0xffff )
									{
										if ( k == i )
										{
											//LAYER INFO ADDED
											sprintf(text, "%d,PMT,%d,,,%d,%d\n", i, pat_tbl[k], bit_rate, layer_info[i]);
											is_pmt = 1;
											break;
										}
									}
								}
								if (!is_pmt)
								{
									//LAYER INFO ADDED
									if ( i == 0x0 )			sprintf(text, "%d,PAT,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1 )	sprintf(text, "%d,CAT,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x2 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x3 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x10 )	sprintf(text, "%d,NIT,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x11 )	sprintf(text, "%d,SDT,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x12 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x13 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x14 )	sprintf(text, "%d,TDT/TOT,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x15 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x16 )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1C )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1D )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1E )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1F )	sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else if ( i == 0x1FFF )	sprintf(text, "%d,NULL,,,,%d,%d\n", i, bit_rate, layer_info[i]);
									else
									{
										is_pcr = 0;
										for (k = 0; k < MAX_PID_SIZE; k++)
										{
											if (pmt_tbl_pcr[k] != (unsigned int)0xffff && pmt_tbl_pcr[k] == (unsigned int)i)
											{
												//LAYER INFO ADDED
												sprintf(text, "%d,PCR,,,,%d,%d\n", i, bit_rate, layer_info[i]);
												is_pcr = 1;
												break;
											}
										}
										if ( !is_pcr )
										{
											//LAYER INFO ADDED
											sprintf(text, "%d,,,,,%d,%d\n", i, bit_rate, layer_info[i]);
										}
									}
								}

//CKIM M 20120710 {
								//if( i != 0x1FFF && bit_rate > 0 )
							if( i != 0x1FFF && (bit_rate > 0 || nb_pcrs == 0) )
//CKIM M 20120710 }
								{
									fputs(text, fp);
								}
								//fflush(fp);
							}
						}
					}
				}
//CKIM A 20120706 {
				}
//CKIM A 20120706 }
			}

//CKIM A 20120706 {
			if (!out_of_sync)
			{
//CKIM A 20120706 }
			fclose (file_stream);
			free (AP_szBuffer);

			if(fp != NULL)
			{
				fflush(fp);
				fclose(fp);
			}
			
			return 0;
//CKIM A 20120706 {
		}
//CKIM A 20120706 }
		}
//CKIM A 20120706 {
		else
		{
			AP_szBuffer_offset = TS_SYNC_CHECK_LOOKAHAED_LEN;
		}
//CKIM A 20120706 }

	} while ((--nTryCount > 0) && !feof(file_stream));

	__CtlLog_->HldPrint("Hld-Bd-Ctl. FAIL to parse PAT/PMT and calc. bitrate for each PID");

	fclose (file_stream);
	free (AP_szBuffer);

	if(fp != NULL)
	{
		fflush(fp);
		fclose(fp);
	}

	return -1;
}


//------------------------------------------------------------------------
//ISDB-S
/* XXX: try to find a better synchro over several packets (use get_packet_size() ?) */
int CHldUtils::mpegts_resync(FILE *pb)
{
    int c, i;

    for(i = 0;i < MAX_RESYNC_SIZE; i++)	{
        c = fgetc(pb);
        if (c < 0)
            return -1;
        if (c == 0x47) {
			fseek(pb, -1, SEEK_CUR);
            return 0;
        }
    }
    /* no sync found */
    return -1;
}
/* return -1 if error or EOF. Return 0 if OK. */
int CHldUtils::read_packet(FILE *pb, unsigned char *buf, int raw_packet_size)
{
	//LAYER INFO ADDED - TS_PACKET_SIZE -> raw_packet_size
	int skip, len;
    for(;;) {
		len = fread (buf,  1, raw_packet_size/*TS_PACKET_SIZE*/, pb);
        if (len != raw_packet_size/*TS_PACKET_SIZE*/)
            return -1;
        /* check paquet sync byte */
        if (buf[0] != 0x47) 
		{
            /* find a new packet start */
            fseek(pb, -raw_packet_size/*TS_PACKET_SIZE*/, SEEK_CUR);
            if (mpegts_resync(pb) < 0)
                return -1;
            else
                continue;
        } else {
            skip = raw_packet_size - raw_packet_size/*TS_PACKET_SIZE*/;
            if (skip > 0)
				fseek(pb, skip, SEEK_CUR);
            break;
        }
    }
    return 0;
}

int CHldUtils::FEOF(HANDLE hFile)
{
#ifdef WIN32
	DWORD dwOffsetLow, dwOffsetHigh=0;
	dwOffsetLow = GetVLFilePointer(hFile, (long*)&dwOffsetHigh);

	DWORD dwSizeLow, dwSizeHigh=0;
	dwSizeLow = GetFileSize (hFile, &dwSizeHigh); 

	//return (dwOffsetHigh == 0xFFFFFFFF && dwOffsetLow == 0xFFFFFFFF);
	if ( (dwOffsetHigh == 0xFFFFFFFF && dwOffsetLow == 0xFFFFFFFF) || (dwSizeHigh == 0 && dwSizeLow <= dwOffsetLow) || (dwSizeHigh != 0 && dwSizeHigh <= dwOffsetHigh) )
		return 1;
	else
		return 0;
#else
	return feof((FILE*)hFile);
#endif
}
//////////////////////////////////////////////////////////////////////////////////////
//extern char	g_szCurDir[MAX_PATH];
int	CHldUtils::TL_BERT_SET_DATA(unsigned char *pBuf)	//	the data generated here may processed in func TL_ProcessCaptureSubBlock() at counter party.
{
	if ( TL_TestPacketType < TS_HEAD_184_ALL_0 )
	{
		return -1;
	}

	unsigned char *buf=NULL;
	FILE *fp=NULL;
	int nRet, nPlayloadSize=0;
	char szDataPath[MAX_PATH];
	if ( !TL_fp_14_15 )
	{
#ifdef WIN32
		sprintf(szDataPath, "%s\\%s", g_szCurDir, BERT_PRBS_14_15_TYPE);
#else
#ifdef STANDALONE
		sprintf(szDataPath, "/root/rbf/%s", BERT_PRBS_14_15_TYPE);
#else
		sprintf(szDataPath, "%s/%s", g_szCurDir, BERT_PRBS_14_15_TYPE);
#endif
#endif
		TL_fp_14_15 = fopen(szDataPath, "rb");
	}
	if ( !TL_fp_18_23 )
	{
#ifdef WIN32
		sprintf(szDataPath, "%s\\%s", g_szCurDir, BERT_PRBS_18_23_TYPE);
#else
#ifdef STANDALONE
		sprintf(szDataPath, "/root/rbf/%s", BERT_PRBS_18_23_TYPE);
#else
		sprintf(szDataPath, "%s/%s", g_szCurDir, BERT_PRBS_18_23_TYPE);
#endif
#endif
		TL_fp_18_23 = fopen(szDataPath, "rb");
	}

	buf = pBuf+4;	//	skip sync/pid/adap-field
	nPlayloadSize = PKTSIZE-4;
	if ( (TL_TestPacketType >= TS_SYNC_187_ALL_0 && TL_TestPacketType <= TS_SYNC_187_PRBS_2_23) )	//	in case of sync error
	{
		buf = pBuf+1;	//	skip sync
		nPlayloadSize = PKTSIZE-1;
	}

	if ( TL_TestPacketType == TS_HEAD_184_PRBS_2_15 || TL_TestPacketType == TS_SYNC_187_PRBS_2_15 || TL_TestPacketType == TS_STUFFING_184_PRBS_2_15 )	//	in case of prbs
	{
		fp = TL_fp_14_15;
	}
	else if ( TL_TestPacketType == TS_HEAD_184_PRBS_2_23 || TL_TestPacketType == TS_SYNC_187_PRBS_2_23 || TL_TestPacketType == TS_STUFFING_184_PRBS_2_23 )	//	in case of prbs
	{
		fp = TL_fp_18_23;
	}
				
	switch (TL_TestPacketType)
	{
	case TS_HEAD_184_ALL_0:
	case TS_SYNC_187_ALL_0:
	case TS_STUFFING_184_ALL_0:
		if ( TL_TestPacketType == TS_STUFFING_184_ALL_0 )
		{
			if ( pBuf[0] != 0x47 || pBuf[1] != 0x1F || pBuf[2] != 0xFF )	//	NOT null pkt
				break;	//	keep normal pkt
		}
		else if ( TL_TestPacketType == TS_SYNC_187_ALL_0 )
		{
			pBuf[0] = 0x47;
		}
		memset(buf, 0x00, nPlayloadSize);	//	fill 0 pkt payload.

		break;

	case TS_HEAD_184_ALL_1:
	case TS_SYNC_187_ALL_1:
	case TS_STUFFING_184_ALL_1:
		if ( TL_TestPacketType == TS_STUFFING_184_ALL_1 )
		{
			if ( pBuf[0] != 0x47 || pBuf[1] != 0x1F || pBuf[2] != 0xFF )	//	NOT null pkt
				break;	//	keep normal pkt
		}
		else if ( TL_TestPacketType == TS_SYNC_187_ALL_1 )
		{
			pBuf[0] = 0x47;
		}
		memset(buf, 0xFF, nPlayloadSize);	//	fill ff pkt payload.

		break;

	case TS_HEAD_184_PRBS_2_15:
	case TS_HEAD_184_PRBS_2_23:
	case TS_SYNC_187_PRBS_2_15:
	case TS_SYNC_187_PRBS_2_23:
	case TS_STUFFING_184_PRBS_2_15:
	case TS_STUFFING_184_PRBS_2_23:
		if (!fp) 
		{
			break;
		}

		if ( TL_TestPacketType == TS_STUFFING_184_PRBS_2_15 || TL_TestPacketType == TS_STUFFING_184_PRBS_2_23 )
		{
			if ( pBuf[0] != 0x47 || pBuf[1] != 0x1F || pBuf[2] != 0xFF )	//	NOT null pkt
				break;	//	keep normal pkt
		}
		else if ( TL_TestPacketType == TS_SYNC_187_PRBS_2_15 || TL_TestPacketType == TS_SYNC_187_PRBS_2_23 )
		{
			pBuf[0] = 0x47;
		}
		nRet = fread(buf, 1, nPlayloadSize, fp );	//	replace given pattern
		if( nRet != nPlayloadSize )
		{
			fseek(fp, 0, SEEK_SET);
			nRet = fread(buf+nRet, 1, nPlayloadSize-nRet, fp );
		}
		break;
	
	default:
		break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
int	*CHldUtils::_Ptr_VarTstPktTyp(void)
{
	return	&TL_TestPacketType;
}
int	CHldUtils::_VarTstPktTyp(void)
{
	return	TL_TestPacketType;
}


//////////////////////////////////////////////////////////////////////////////////////
int	*CHldUtils::_Ptr_VarTstPktPid(void)
{
	return	&TL_TestPacketPid;
}
int	CHldUtils::_VarTstPktPid(void)
{
	return	TL_TestPacketPid;
}
//////////////////////////////////////////////////////////////////////////////////////




//CKIM A 20120828 {
#define MIN_TS_PACKET_HEADER_LEN						4
#define MAX_TS_PACKET_HEADER_ADAPTATION_FIELD_LENGTH	183
#define MAX_TS_PACKET_PAYLOAD_LEN						184
#define MAX_TS_PACKET_TRAILER_LEN						20
#define MIN_TS_PACKET_LEN								(MIN_TS_PACKET_HEADER_LEN + MAX_TS_PACKET_PAYLOAD_LEN)
#define MAX_TS_PACKET_LEN								(MIN_TS_PACKET_LEN + MAX_TS_PACKET_TRAILER_LEN) 
#define NUM_OF_CONSECUTIVE_TS_PACKETS_IN_SYNC			3
#define TS_SYNC_CHECK_LOOKAHEAD_LEN						(MAX_TS_PACKET_LEN * (NUM_OF_CONSECUTIVE_TS_PACKETS_IN_SYNC - 1) + 1)
#define NUM_OF_PIDS										8192
#define MIN_PSI_SI_SECTION_HEADER_LEN					3
#define MAX_PSI_SI_SECTION_HEADER_LEN					(MIN_PSI_SI_SECTION_HEADER_LEN + 5)
#define PSI_SI_SECTION_CRC_32_LEN						4						
#define MAX_PSI_SI_LAST_SECTION_NUMBER					255


class CPsiSiSectionNumberBitVector
{
private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		MAX_SECTION_NUMBER_BIT_VECTOR_LEN = (MAX_PSI_SI_LAST_SECTION_NUMBER + 1) / 8
	};
#else
	static const int MAX_SECTION_NUMBER_BIT_VECTOR_LEN = (MAX_PSI_SI_LAST_SECTION_NUMBER + 1) / 8;
#endif

	int last_section_number;
	BYTE section_number_bit_vector[MAX_SECTION_NUMBER_BIT_VECTOR_LEN];
	int num_of_sections;

public:
	void Construct(void)
	{
		last_section_number = -1;
		num_of_sections = 0;
	}

	CPsiSiSectionNumberBitVector()
	{
		Construct();
	}

	void Destruct(void)
	{

	}

	~CPsiSiSectionNumberBitVector()
	{
		Destruct();
	}

	int SetLastSectionNumber(int section_number)
	{
		if (section_number < 0 || section_number > MAX_PSI_SI_LAST_SECTION_NUMBER)
{
			return -1;
		}

		last_section_number = section_number;
		memset(section_number_bit_vector, 0, last_section_number / 8 + 1);
		num_of_sections = 0;

	return 0;
}

	int Set(int section_number)
	{
		if (section_number < 0 || section_number > last_section_number)
		{
			return -1;
		}

		if (!(section_number_bit_vector[section_number / 8] & (1 << (section_number % 8))))
		{
			section_number_bit_vector[section_number / 8] |= 1 << (section_number % 8);
			++num_of_sections;
		}

		return 0;
	}

	int IsSet(int section_number)
	{
		if (section_number < 0 || section_number > last_section_number)
		{
			return 0;
		}
		if (section_number_bit_vector[section_number / 8] & (1 << (section_number % 8)))
		{
			return 1;
		}
		else
{
	return 0;
}
	}

	int IsAllSet(void)
	{
		if (num_of_sections == last_section_number + 1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
};

class CPsiSiDescriptorHandler
{
public:
	virtual int Parse(BYTE *data) = 0;
};

class CPsiSiSatelliteDeliverySystemDescriptorHandler: public CPsiSiDescriptorHandler
{
private:
	int parsed;
	unsigned int frequency;
	unsigned int orbital_position;
	unsigned int west_east_flag;
	unsigned int polarization;
	unsigned int roll_off;
	unsigned int modulation_system;
	unsigned int modulation_type;
	unsigned int symbol_rate;
	unsigned int fec_inner;

public:
	int Init(void)
	{
		parsed = 0;

		return 0;
	}

	CPsiSiSatelliteDeliverySystemDescriptorHandler()
	{
		Init();
	}

	int Parse(BYTE *data)
	{
		// frequency			32-bit	4-byte
		// orbital_position		16-bit	2-byte
		// west_east_flag		1-bit
		// polarization			2-bit
		// roll_off				2-bit
		// modulation_system	1-bit
		// modulation_type		2-bit	1-byte
		// symbol_rate			28-bit
		// FEC_inner			4-bit	4-byte

		if (data == NULL)
		{
			return -1;
		}
		if (data[0] != 0x43 || data[1] != 11)
		{
			return -2;
		}

		frequency = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + data[5];
		orbital_position = (data[6] << 8) + data[7];
		west_east_flag = (data[8] & 0x80) >> 7;
		polarization = (data[8] & 0x60) >> 5;
		roll_off = (data[8] & 0x18) >> 3;
		modulation_system = (data[8] & 0x04) >> 2;
		modulation_type = data[8] & 0x03;
		symbol_rate = (data[9] << 20) + (data[10] << 12) + (data[11] << 4) + ((data[12] & 0xf0) >> 4);
		fec_inner = data[12] & 0x0f;
		parsed = 1;

		return 0;
	}

	int GetFrequency(int *p_frequency)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_frequency == NULL)
		{
			return -2;
		}

		*p_frequency = frequency;		

		return 0;
	}

	int GetOrbitalPosition(int *p_orbital_position)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_orbital_position == NULL)
		{
			return -2;
		}

		*p_orbital_position = orbital_position;		

		return 0;
	}

	int GetWestEastFlag(int *p_west_east_flag)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_west_east_flag == NULL)
		{
			return -2;
		}

		*p_west_east_flag = west_east_flag;		

		return 0;
	}

	int GetPolarization(int *p_polarization)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_polarization == NULL)
		{
			return -2;
		}

		*p_polarization = polarization;		

		return 0;
	}

	int GetRollOff(int *p_roll_off)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_roll_off == NULL)
		{
			return -2;
		}

		*p_roll_off = roll_off;		

		return 0;
	}

	int GetModulationSystem(int *p_modulation_system)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_modulation_system == NULL)
		{
			return -2;
		}

		*p_modulation_system = modulation_system;		

		return 0;
	}

	int GetModulationType(int *p_modulation_type)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_modulation_type == NULL)
		{
			return -2;
		}

		*p_modulation_type = modulation_type;		

		return 0;
	}

	int GetSymbolRate(int *p_symbol_rate)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_symbol_rate == NULL)
		{
			return -2;
		}

		*p_symbol_rate = symbol_rate;		

		return 0;
	}

	int GetFecInner(int *p_fec_inner)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_fec_inner == NULL)
		{
			return -2;
		}

		*p_fec_inner = fec_inner;		

		return 0;
	}
};

class CPsiSiCableDeliverySystemDescriptorHandler: public CPsiSiDescriptorHandler
{
private:
	int parsed;
	unsigned int frequency;
	unsigned int fec_outer;
	unsigned int modulation;
	unsigned int symbol_rate;
	unsigned int fec_inner;

public:
	int Init(void)
	{
		parsed = 0;

		return 0;
	}

	CPsiSiCableDeliverySystemDescriptorHandler()
	{
		Init();
	}

	int Parse(BYTE *data)
	{
		// frequency			32-bit	4-byte
		// reserved_future_use	12-bit
		// FEC_outer			4-bit	2-byte
		// modulation			8-bit	1-byte
		// symbol_rate			28-bit
		// FEC_inner			4-bit	4-byte

		if (data == NULL)
		{
			return -1;
		}
		if (data[0] != 0x44 || data[1] != 11)
		{
			return -2;
		}
		
		frequency = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + data[5];
		fec_outer = data[7] & 0x0f;
		modulation = data[8];
		symbol_rate = (data[9] << 20) + (data[10] << 12) + (data[11] << 4) + ((data[12] & 0xf0) >> 4);
		fec_inner = data[12] & 0x0f;
		parsed = 1;

		return 0;
	}

	int GetFrequency(int *p_frequency)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_frequency == NULL)
		{
			return -2;
		}

		*p_frequency = frequency;		

		return 0;
	}

	int GetFecOuter(int *p_fec_outer)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_fec_outer == NULL)
		{
			return -2;
		}

		*p_fec_outer = fec_outer;

		return 0;
	}

	int GetModulation(int *p_modulation)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_modulation == NULL)
		{
			return -2;
		}

		*p_modulation = modulation;

		return 0;
	}

	int GetSymbolRate(int *p_symbol_rate)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_symbol_rate == NULL)
		{
			return -2;
		}

		*p_symbol_rate = symbol_rate;

		return 0;
	}

	int GetFecInner(int *p_fec_inner)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_fec_inner == NULL)
		{
			return -2;
		}

		*p_fec_inner = fec_inner;

		return 0;
	}
};


class CPsiSiTerrestrialDeliverySystemDescriptorHandler: public CPsiSiDescriptorHandler
{
private:
	int parsed;
	unsigned int centre_frequency;
	unsigned int bandwidth;
	unsigned int priority;
	unsigned int time_slicing_indicator;
	unsigned int mpe_fec_indicator;
	unsigned int constellation;
	unsigned int hierarchy_information;
	unsigned int code_rate_hp_stream;
	unsigned int code_rate_lp_stream;
	unsigned int guard_interval;
	unsigned int transmission_mode;
	unsigned int other_frequency_flag;

public:
	int Init(void)
	{
		parsed = 0;

		return 0;
	}

	CPsiSiTerrestrialDeliverySystemDescriptorHandler()
	{
		Init();
	}

	int Parse(BYTE *data)
	{
		// centre_frequency			32-bit	4-byte
		// bandwidth				3-bit
		// priority					1-bit
		// Time_Slicing_indicator	1-bit
		// MPE-FEC_indicator		1-bit
		// reserved_future_use		2-bit	1-byte
		// constellation			2-bit
		// hierarchy_information	3-bit
		// code_rate-HP_stream		3-bit	1-byte
		// code_rate-LP_stream		3-bit
		// guard_interval			2-bit
		// transmission_mode		2-bit
		// other_frequency_flag		1-bit	1-byte
		// reserved_future_use		32-bit	4-byte

		if (data == NULL)
		{
			return -1;
		}
		if (data[0] != 0x5a || data[1] != 11)
		{
			return -2;
		}

		centre_frequency = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + data[5];
		bandwidth = (data[6] & 0xe0) >> 5;
		priority = (data[6] & 0x10) >> 4;
		time_slicing_indicator = (data[6] & 0x80) >> 3;
		mpe_fec_indicator = (data[6] & 0x40) >> 2;
		constellation = (data[7] & 0xc0) >> 6;
		hierarchy_information = (data[7] & 0x38) >> 3;
		code_rate_hp_stream = data[7] & 0x07;
		code_rate_lp_stream = (data[8] & 0xe0) >> 5;
		guard_interval = (data[8] & 0x18) >> 3;
		transmission_mode = (data[8] & 0x06) >> 1;
		other_frequency_flag = data[8] & 0x01;
		parsed = 1;

		return 0;
	}

	int GetCentreFrequency(int *p_centre_frequency)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_centre_frequency == NULL)
		{
			return -2;
		}

		*p_centre_frequency = centre_frequency;

		return 0;
	}

	int GetBandwidth(int *p_bandwidth)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_bandwidth == NULL)
		{
			return -2;
		}

		*p_bandwidth = bandwidth;

		return 0;
	}

	int GetPriority(int *p_priority)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_priority == NULL)
		{
			return -2;
		}

		*p_priority = priority;

		return 0;
	}

	int GetTimeSlicingIndicator(int *p_time_slicing_indicator)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_time_slicing_indicator == NULL)
		{
			return -2;
		}

		*p_time_slicing_indicator = time_slicing_indicator;

		return 0;
	}

	int GetMpeFecIndicator(int *p_mpe_fec_indicator)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_mpe_fec_indicator == NULL)
		{
			return -2;
		}

		*p_mpe_fec_indicator = mpe_fec_indicator;

		return 0;
	}

	int GetConstellation(int *p_constellation)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_constellation == NULL)
		{
			return -2;
		}

		*p_constellation = constellation;

		return 0;
	}

	int GetHierarchyInformation(int *p_hierarchy_information)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_hierarchy_information == NULL)
		{
			return -2;
		}

		*p_hierarchy_information = hierarchy_information;

		return 0;
	}

	int GetCodeRateHpStream(int *p_code_rate_hp_stream)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_code_rate_hp_stream == NULL)
		{
			return -2;
		}

		*p_code_rate_hp_stream = code_rate_hp_stream;

		return 0;
	}

	int GetCodeRateLpStream(int *p_code_rate_lp_stream)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_code_rate_lp_stream == NULL)
		{
			return -2;
		}

		*p_code_rate_lp_stream = code_rate_lp_stream;

		return 0;
	}

	int GetGuardInterval(int *p_guard_interval)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_guard_interval == NULL)
		{
			return -2;
		}

		*p_guard_interval = guard_interval;

		return 0;
	}

	int GetTransmissionMode(int *p_transmission_mode)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_transmission_mode == NULL)
		{
			return -2;
		}

		*p_transmission_mode = transmission_mode;

		return 0;
	}

	int GetOtherFrequencyFlag(int *p_other_frequency_flag)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_other_frequency_flag == NULL)
		{
			return -2;
		}

		*p_other_frequency_flag = other_frequency_flag;

		return 0;
	}
};

class CPsiSiS2SatelliteDeliverySystemDescriptorHandler: public CPsiSiDescriptorHandler
{
private:
	int parsed;
	unsigned int scrambling_sequence_selector;
	unsigned int multiple_input_stream_flag;
	unsigned int backwards_compatibility_indicator;
	unsigned int scrambling_sequence_index;
	unsigned int input_stream_identifier;

public:
	int Init(void)
	{
		parsed = 0;

		return 0;
	}

	CPsiSiS2SatelliteDeliverySystemDescriptorHandler()
	{
		Init();
	}

	int Parse(BYTE *data)
	{
		// scrambling_sequence_selector				1-bit
		// multiple_input_stream_flag				1-bit
		// backwards_compatibility_indicator		1-bit
		// reserved_future_use						5-bit	1-byte
		// if (scrambling_sequence_selector == 1) {
			// Reserved								6-bit
			// scrambling_sequence_index			18-bit	3-byte
		// }
		// if (multiple_input_stream_flag == 1) {
			// input_stream_identifier				8-bit	1-byte
		// }

		if (data == NULL)
		{
			return -1;
		}
		if (data[0] != 0x79 || data[1] == 0 || !(data[1] == 1 && (data[2] & 0xc0) == 0x00 || data[1] == 4 && (data[2] & 0xc0) == 0x80 || 
			data[1] == 2 && (data[2] & 0xc0) == 0x40 || data[1] == 5 && (data[2] & 0xc0) == 0xc0))
		{
			return -2;
		}

		scrambling_sequence_selector = (data[2] & 0x80) >> 7;
		multiple_input_stream_flag = (data[2] & 040) >> 6;
		backwards_compatibility_indicator = (data[2] & 0x20) >> 5;
		if (data[1] == 4)
		{
			scrambling_sequence_index = ((data[3] & 0x03) << 16) + (data[4] << 8) + data[5];
		}
		else if (data[1] == 2)
		{
			input_stream_identifier = data[3];
		}		
		else if (data[1] == 5)
		{
			scrambling_sequence_index = ((data[3] & 0x03) << 16) + (data[4] << 8) + data[5];
			input_stream_identifier = data[6];
		}
		else	// data[1] = 1
		{
			// does nothing
		}
		parsed = 1;

		return 0;
	}

	int GetScramblingSequenceSelector(int *p_scrambling_sequence_selector)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_scrambling_sequence_selector == NULL)
		{
			return -2;
		}

		*p_scrambling_sequence_selector = scrambling_sequence_selector;

		return 0;
	}

	int GetMultipleInputStreamFlag(int *p_multiple_input_stream_flag)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_multiple_input_stream_flag == NULL)
		{
			return -2;
		}

		*p_multiple_input_stream_flag = multiple_input_stream_flag;

		return 0;
	}

	int GetBackwardsCompatibilityIndicator(int *p_backwards_compatibility_indicator)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_backwards_compatibility_indicator == NULL)
		{
			return -2;
		}

		*p_backwards_compatibility_indicator = backwards_compatibility_indicator;

		return 0;
	}

	int GetScramblingSequenceIndex(int *p_scrambling_sequence_index)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_scrambling_sequence_index == NULL)
		{
			return -2;
		}

		*p_scrambling_sequence_index = scrambling_sequence_index;

		return 0;
	}

	int GetInputStreamIdentifier(int *p_input_stream_identifier)
	{
		if (!parsed)
		{
			return -1;
		}
		if (p_input_stream_identifier == NULL)
		{
			return -2;
		}

		*p_input_stream_identifier = input_stream_identifier;

		return 0;
	}
};


class CPsiSiDescriptorHandlerRegistry
{
private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		MAX_NUM_OF_PSI_SI_DESCRIPTOR_HANDLERS = 256
	};
#else
	static const int MAX_NUM_OF_PSI_SI_DESCRIPTOR_HANDLERS = 256;
#endif

	int num_of_psi_si_descriptor_handlers;
	CPsiSiDescriptorHandler *psi_si_descriptor_handlers[MAX_NUM_OF_PSI_SI_DESCRIPTOR_HANDLERS];

public:
	int Init(void)
	{
		int i;

		for (i = 0; i < MAX_NUM_OF_PSI_SI_DESCRIPTOR_HANDLERS; ++i)
		{
			psi_si_descriptor_handlers[i] = NULL;
		}
		num_of_psi_si_descriptor_handlers = 0;

		return 0;
	}

	void Construct(void)
	{
		Init();
	}

	CPsiSiDescriptorHandlerRegistry()
	{
		Construct();
	}

	int RegisterHandler(int descriptor_tag, CPsiSiDescriptorHandler *pPsiSiDescriptorHandler)
	{
		if (pPsiSiDescriptorHandler == NULL)
		{
			return -1;
		}

		if (psi_si_descriptor_handlers[descriptor_tag] == NULL)
		{
			++num_of_psi_si_descriptor_handlers;
		}
		psi_si_descriptor_handlers[descriptor_tag] = pPsiSiDescriptorHandler;

		return 0;
	}

	int UnregisterHandler(int descriptor_tag)
	{
		if (psi_si_descriptor_handlers[descriptor_tag] != NULL)
		{
			psi_si_descriptor_handlers[descriptor_tag] == NULL;
			--num_of_psi_si_descriptor_handlers;
		}

		return 0;
	}

	CPsiSiDescriptorHandler *GetHandler(int descriptor_tag)
	{
		return psi_si_descriptor_handlers[descriptor_tag];
	}
};

class CPsiSiDescriptorPool
{
private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		MAX_NUM_OF_PSI_SI_DESCRIPTORS = 256
	};
#else
	static const int MAX_NUM_OF_PSI_SI_DESCRIPTORS = 256;
#endif

	int num_of_psi_si_descriptors;
	BYTE *psi_si_descriptors[MAX_NUM_OF_PSI_SI_DESCRIPTORS];

public:
	void Construct(void)
	{
		int i;

		for (i = 0; i < MAX_NUM_OF_PSI_SI_DESCRIPTORS; ++i)
		{
			psi_si_descriptors[i] = NULL;
		}
		num_of_psi_si_descriptors = 0;
	}

	void Destruct(void)
	{
		int i;

		if (num_of_psi_si_descriptors > 0)
		{
			for (i = 0; num_of_psi_si_descriptors > 0 && i < MAX_NUM_OF_PSI_SI_DESCRIPTORS; ++i)
			{
				if (psi_si_descriptors[i] != NULL)
				{
					free(psi_si_descriptors[i]);
					psi_si_descriptors[i] = NULL;
					--num_of_psi_si_descriptors;
				}
			}
		}
	}

	CPsiSiDescriptorPool()
	{
		Construct();
	}

	~CPsiSiDescriptorPool()
	{
		Destruct();
	}	

	int Add(BYTE *descriptor)
	{
		int descriptor_tag;
		int descriptor_len;

		if (descriptor == NULL)
		{
			return -1;
		}

		descriptor_tag = descriptor[0];
		if (descriptor_tag < 0 || descriptor_tag > 255)
		{
			return -2;
		}

		descriptor_len = 2 + descriptor[1];
		if (psi_si_descriptors[descriptor_tag] != NULL)
		{
			free(psi_si_descriptors[descriptor_tag]);
			psi_si_descriptors[descriptor_tag] = NULL;
			--num_of_psi_si_descriptors;
		}
		if ((psi_si_descriptors[descriptor_tag] = (BYTE *)malloc(descriptor_len * sizeof(BYTE))) == NULL)
		{
			return -3;
		}
		memcpy(psi_si_descriptors[descriptor_tag], descriptor, descriptor_len);
		++num_of_psi_si_descriptors;

		return 0;
	}

	int Get(int descriptor_tag, BYTE **p_descriptor)
	{
		if (descriptor_tag < 0 || descriptor_tag > 255)
		{
			return -1;
		}

		if (*p_descriptor == NULL)
		{
			return -2;
		}

		if (psi_si_descriptors[descriptor_tag] == NULL)
		{
			return -3;
		}

		*p_descriptor = psi_si_descriptors[descriptor_tag];

		return 0;
	}
};

class CPsiSiTableHandler
{
public:
	virtual int PreprocessSection(BYTE *data, int data_len) = 0;
	virtual int ParseSection(BYTE *data, int data_len) = 0;
};

class CPsiSiPatHandler: public CPsiSiTableHandler
{
private:
	class CPsiSiProgramAssociationMap
	{
	private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
		enum
		{
			NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INIT = 16,
			NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INCR = 4
		};
#else
		static const int NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INIT = 16;
		static const int NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INCR = 4;
#endif

		int max_num_of_program_associations;
		int num_of_program_associations;
		struct _PSI_SI_PROGRAM_ASSOCIATION
		{
			int program_number;
			int pid;
		} *program_association_map;
		int max_num_of_pids;
		int *pids;

		int Locate(int program_number)
		{
			int index;
			int i;

			index = -num_of_program_associations - 1;
			for (i = 0; i < num_of_program_associations; ++i)
			{
				if (program_association_map[i].program_number == program_number)
				{
					index = i;
					break;
				}
				else if (program_association_map[i].program_number > program_number)
				{
					index = -i - 1;
					break;
				}
			}

			return index;
		}

		int IncreaseStorage(void)
		{
			struct _PSI_SI_PROGRAM_ASSOCIATION *increased_program_association_map;

			if ((increased_program_association_map = (struct _PSI_SI_PROGRAM_ASSOCIATION *)realloc(program_association_map, 
				(max_num_of_program_associations + NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INCR) * 
				sizeof(struct _PSI_SI_PROGRAM_ASSOCIATION))) == NULL)
			{
				return -1;
			}

			program_association_map = increased_program_association_map;
			max_num_of_program_associations += NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INCR;

			return 0;
		}

	public:
		int Init(void)
		{
			num_of_program_associations = 0;

			return 0;
		}

		void Construct(void)
		{
			if ((program_association_map = (struct _PSI_SI_PROGRAM_ASSOCIATION *)malloc(
				NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INIT * sizeof(struct _PSI_SI_PROGRAM_ASSOCIATION))) == NULL)
			{
				max_num_of_program_associations = 0;
			}
			else
			{
				max_num_of_program_associations = NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_INIT;
			}
			pids = NULL;
			max_num_of_pids = 0;

			Init();
		}

		CPsiSiProgramAssociationMap()
		{
			Construct();
		}

		void Destruct()
		{
			if (program_association_map != NULL)
			{
				free(program_association_map);
				program_association_map = NULL;
			}
			if (pids != NULL)
			{
				free(pids);
				pids = NULL;
			}
		}

		~CPsiSiProgramAssociationMap()
		{
			Destruct();
		}

		int Add(int program_number, int pid)
		{
			int index;
			int i;

			index = Locate(program_number);
			if (index >= 0)
			{
				program_association_map[index].pid = pid;
			}
			else
			{
				if (num_of_program_associations >= max_num_of_program_associations)
				{
					if (IncreaseStorage() < 0)
					{
						return -1;
					}
				}
				index = -index - 1;
				for (i = num_of_program_associations;i > index; --i)
				{
					program_association_map[i] = program_association_map[i - 1];
				}
				program_association_map[index].program_number = program_number;
				program_association_map[index].pid = pid;
				
				++num_of_program_associations;
			}

			return 0;
		}

		int GetPids(int *p_num_of_pids, int **p_pids)
		{
			int num_of_pids;
			int found;
			int i;
			int j;
			int k;

			if (max_num_of_pids < num_of_program_associations)
			{
				if (pids != NULL)
				{
					free(pids);
					pids = NULL;
					max_num_of_pids = 0;
				}
				if ((pids = (int *)malloc(num_of_program_associations * sizeof(int))) == NULL)
				{
					return -1;
				}
				max_num_of_pids = num_of_program_associations;
			}

			num_of_pids = 0;
			for (i = 0; i < num_of_program_associations; ++i)
			{
				found = 0;
				for (j = 0; j < num_of_pids; ++j)
				{
					if (pids[j] == program_association_map[i].pid)
					{
						found = 1;
						break;
					}
					else if (pids[j] > program_association_map[i].pid)
					{
						break;
					}
				}
				if (!found)
				{
					for (k = num_of_pids; k > j; --k)
					{
						pids[k] = pids[k - 1];
					}
					pids[j] = program_association_map[i].pid;
					++num_of_pids;
				}
			}

			*p_num_of_pids = num_of_pids;
			*p_pids = pids;

			return 0;
		}

		int GetPid(int program_number, int *p_pid)
		{
			int index;

			index = Locate(program_number);
			if (index >= 0)
			{
				*p_pid = program_association_map[index].pid;
				return 0;
			}
			
			return -1;
		}
	};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INIT = 1,
		NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INCR = 1
	};
#else
	static const int NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INIT = 1;
	static const int NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INCR = 1;
#endif

	int max_num_of_program_association_subtables;
	int num_of_program_association_subtables;
	int last_transport_stream_id;
	struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE
	{
		int transport_stream_id;
		int version_number;
		int last_section_number;
		CPsiSiSectionNumberBitVector SectionParsedFlags;
		CPsiSiProgramAssociationMap ProgramAssociationMap;
	} *program_association_subtables;
	int (*update_callback)(int);

	int Locate(int transport_stream_id)
	{
		int index;
		int i;

		index = -num_of_program_association_subtables - 1;
		for (i = 0; i < num_of_program_association_subtables; ++i)
		{
			if (program_association_subtables[i].transport_stream_id == transport_stream_id)
			{
				index = i;
				break;
			}
			else if (program_association_subtables[i].transport_stream_id > transport_stream_id)
			{
				index = -i - 1;
				break;
			}
		}

		return index;
	}

	int IncreaseStorage(void)
	{
		struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *increased_program_association_subtables;
		int i;

		if ((increased_program_association_subtables = (struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *)realloc(program_association_subtables, 
			(max_num_of_program_association_subtables + NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INCR) * 
			sizeof(struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE))) == NULL)
		{
			return -1;
		}

		program_association_subtables = increased_program_association_subtables;
		for (i = max_num_of_program_association_subtables; i < max_num_of_program_association_subtables + 
			NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INCR; ++i)
		{
			program_association_subtables[i].SectionParsedFlags.Construct();
			program_association_subtables[i].ProgramAssociationMap.Construct();
		}
		max_num_of_program_association_subtables += NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INCR;

		return 0;
	}

	struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *AddSubtable(int transport_stream_id, int version_number, int last_section_number)
	{
		int index;
		int i;

		if ((index = Locate(transport_stream_id)) >= 0)
		{
			if (program_association_subtables[index].version_number != version_number || 
				program_association_subtables[index].last_section_number != last_section_number)
			{
				program_association_subtables[index].version_number = version_number;
				program_association_subtables[index].last_section_number = last_section_number;
				program_association_subtables[index].SectionParsedFlags.SetLastSectionNumber(last_section_number);
				program_association_subtables[index].ProgramAssociationMap.Init();
			}
		}
		else
		{
			if (num_of_program_association_subtables >= max_num_of_program_association_subtables)
			{
				if (IncreaseStorage() < 0)
				{
					return NULL;
				}
			}
			index = -index - 1;
			program_association_subtables[num_of_program_association_subtables].ProgramAssociationMap.Destruct();
			for (i = num_of_program_association_subtables;i > index; --i)
			{
				program_association_subtables[i] = program_association_subtables[i - 1];
			}
			program_association_subtables[index].transport_stream_id = transport_stream_id;
			program_association_subtables[index].version_number = version_number;
			program_association_subtables[index].last_section_number = last_section_number;
			program_association_subtables[index].SectionParsedFlags.SetLastSectionNumber(last_section_number);
			program_association_subtables[index].ProgramAssociationMap.Construct();

			++num_of_program_association_subtables;			
		}

		return &program_association_subtables[index];
	}

	struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *GetSubtable(int transport_stream_id)
	{
		int index;

		if ((index = Locate(transport_stream_id)) < 0)
		{
			return NULL;
		}
		else
		{
			return &program_association_subtables[index];
		}
	}

public:
	int Init(void)
	{
		num_of_program_association_subtables = 0;
		last_transport_stream_id = -1;
		update_callback = NULL;
		
		return 0;
	}

	CPsiSiPatHandler()
	{
		int i;

		if ((program_association_subtables = (struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *)malloc(
			NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INIT * sizeof(struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE))) == NULL)
		{
			max_num_of_program_association_subtables = 0;
		}
		else
		{
			max_num_of_program_association_subtables = NUM_OF_PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE_INIT;
			for (i = 0; i < max_num_of_program_association_subtables; ++i)
			{
				program_association_subtables[i].SectionParsedFlags.Construct();
				program_association_subtables[i].ProgramAssociationMap.Construct();
			}
		}

		Init();
	}

	~CPsiSiPatHandler()
	{
		int i;

		for (i = 0; i < max_num_of_program_association_subtables; ++i)
		{
			program_association_subtables[i].SectionParsedFlags.Destruct();
			program_association_subtables[i].ProgramAssociationMap.Destruct();
		}
		if (program_association_subtables != NULL)
		{
			free(program_association_subtables);
			program_association_subtables = NULL;
		}
	}

	int PreprocessSection(BYTE *data, int data_len)
	{
		int section_length;
		struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *ProgramAssociationSubtable;

		if (data_len < MAX_PSI_SI_SECTION_HEADER_LEN)
		{
			return 0;
		}
		if ((data[1] & 0xc0) != 0x80)	// skips unless section_syntax_indicator '0' == '10'
		{
			return -1;
		}
		if (!(data[5] & 0x01))	// skips unless current_next_indicator == '1'
		{
			return -2;
		}
		if (data[6] > data[7])	// skips unless section_number <= last_section_number
		{
			return -3;
		}
		if ((section_length = ((data[1] & 0x0f) << 8) + data[2]) > 1021)
		{
			return -4;
		}
		if ((ProgramAssociationSubtable = GetSubtable((data[3] << 8) + data[4])) != NULL &&
			((data[5] & 0x3e) >> 1) == ProgramAssociationSubtable->version_number && 
			data[7] == ProgramAssociationSubtable->last_section_number && 
			ProgramAssociationSubtable->SectionParsedFlags.IsSet(data[6]))	// skips unless version_number of current PAT changed
		{
			return -5;
		}
		
		return 0;
	}

	int ParseSection(BYTE *data, int data_len)
	{
		int transport_stream_id;
		struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *ProgramAssociationSubtable;
		int offset;

		transport_stream_id = (data[3] << 8) + data[4];
		if ((ProgramAssociationSubtable = AddSubtable(transport_stream_id, (data[5] & 0x3e) >> 1, data[7])) == NULL)
		{
			return -1;
		}

		for (offset = MAX_PSI_SI_SECTION_HEADER_LEN; offset < data_len - PSI_SI_SECTION_CRC_32_LEN; offset += 4)
		{
			if (ProgramAssociationSubtable->ProgramAssociationMap.Add((data[offset] << 8) + data[offset + 1],
				((data[offset + 2] & 0x1f) << 8) + data[offset + 3]) < 0)
			{
				return -2;
			}
		}

		ProgramAssociationSubtable->SectionParsedFlags.Set(data[6]);
		if (ProgramAssociationSubtable->SectionParsedFlags.IsAllSet())
		{
			last_transport_stream_id = transport_stream_id;
			if (update_callback != NULL)
			{
				update_callback(transport_stream_id);
			}
		}
		
		return 0;
	}

	int SetUpdateCallback(int (*callback)(int))
	{
		update_callback = callback;
		
		return 0;
	}

	int GetPmtPids(int transport_stream_id, int *p_num_of_pmt_pids, int **p_pmt_pids)
	{
		struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *ProgramAssociationSubtable;

		if (transport_stream_id < 0)
		{
			transport_stream_id = last_transport_stream_id;
		}		
		if ((ProgramAssociationSubtable = GetSubtable(transport_stream_id)) == NULL)
		{
			return -1;
		}

		return ProgramAssociationSubtable->ProgramAssociationMap.GetPids(p_num_of_pmt_pids, p_pmt_pids);
	}

	int GetPid(int transport_stream_id, int program_number, int *p_pid)
	{
		struct _PSI_SI_PROGRAM_ASSOCIATION_SUBTABLE *ProgramAssociationSubtable;

		if (transport_stream_id < 0)
		{
			transport_stream_id = last_transport_stream_id;
		}
		if ((ProgramAssociationSubtable = GetSubtable(transport_stream_id)) == NULL)
		{
			return -1;
		}

		return ProgramAssociationSubtable->ProgramAssociationMap.GetPid(program_number, p_pid);
	}

	int GetTransportStreamId(int *p_transport_stream_id)
	{
		if (last_transport_stream_id < 0)
		{
			return -1;
		}

		*p_transport_stream_id = last_transport_stream_id;

		return 0;
	}
};


class CPsiSiPmtHandler: public CPsiSiTableHandler
{
private:
	class CPsiSiProgramElementMap
	{
	private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
		enum
		{
			NUM_OF_PSI_SI_PROGRAM_ELEMENT_INIT = 16,
			NUM_OF_PSI_SI_PROGRAM_ELEMENT_INCR = 4
		};
#else
		static const int NUM_OF_PSI_SI_PROGRAM_ELEMENT_INIT = 16;
		static const int NUM_OF_PSI_SI_PROGRAM_ELEMENT_INCR = 4;
#endif

		int max_num_of_program_elements;
		int num_of_program_elements;
		struct _PSI_SI_PROGRAM_ELEMENT
		{
			int elementary_pid;
			int stream_type;
		} *program_element_map;
		int max_num_of_elementary_pids;
		int *elementary_pids;

		int Locate(int elementary_pid)
		{
			int index;
			int i;

			index = -num_of_program_elements - 1;
			for (i = 0; i < num_of_program_elements; ++i)
			{
				if (program_element_map[i].elementary_pid == elementary_pid)
				{
					index = i;
					break;
				}
				else if (program_element_map[i].elementary_pid > elementary_pid)
				{
					index = -i - 1;
					break;
				}
			}

			return index;
		}

		int IncreaseStorage(void)
		{
			struct _PSI_SI_PROGRAM_ELEMENT *increased_program_element_map;

			if ((increased_program_element_map = (struct _PSI_SI_PROGRAM_ELEMENT *)realloc(program_element_map, 
				(max_num_of_program_elements + NUM_OF_PSI_SI_PROGRAM_ELEMENT_INCR) * 
				sizeof(struct _PSI_SI_PROGRAM_ELEMENT))) == NULL)
			{
				return -1;
			}

			program_element_map = increased_program_element_map;
			max_num_of_program_elements += NUM_OF_PSI_SI_PROGRAM_ELEMENT_INCR;

			return 0;
		}

	public:
		int Init(void)
		{
			num_of_program_elements = 0;

			return 0;
		}

		void Construct(void)
		{
			if ((program_element_map = (struct _PSI_SI_PROGRAM_ELEMENT *)malloc(
				NUM_OF_PSI_SI_PROGRAM_ELEMENT_INIT * sizeof(struct _PSI_SI_PROGRAM_ELEMENT))) == NULL)
			{
				max_num_of_program_elements = 0;
			}
			else
			{
				max_num_of_program_elements = NUM_OF_PSI_SI_PROGRAM_ELEMENT_INIT;
			}
			elementary_pids = NULL;
			max_num_of_elementary_pids = 0;

			Init();
		}

		CPsiSiProgramElementMap()
		{
			Construct();
		}

		void Destruct(void)
		{
			if (program_element_map != NULL)
			{
				free(program_element_map);
				program_element_map = NULL;
			}
			if (elementary_pids != NULL)
			{
				free(elementary_pids);
				elementary_pids = NULL;
			}
		}

		~CPsiSiProgramElementMap()
		{
			Destruct();
		}

		int Add(int elementary_pid, int stream_type)
		{
			int index;
			int i;

			index = Locate(elementary_pid);
			if (index >= 0)
			{
				program_element_map[index].stream_type = stream_type;
			}
			else
			{
				if (num_of_program_elements >= max_num_of_program_elements)
				{
					if (IncreaseStorage() < 0)
					{
						return -1;
					}
				}
				index = -index - 1;
				for (i = num_of_program_elements;i > index; --i)
				{
					program_element_map[i] = program_element_map[i - 1];
				}
				program_element_map[index].elementary_pid = elementary_pid;
				program_element_map[index].stream_type = stream_type;
				
				++num_of_program_elements;
			}

			return 0;
		}

		int GetElementaryPids(int *p_num_of_elementary_pids, int **p_elementary_pids)
		{
			int i;

			if (max_num_of_elementary_pids < num_of_program_elements)
			{
				if (elementary_pids != NULL)
				{
					free(elementary_pids);
					elementary_pids = NULL;
					max_num_of_elementary_pids = 0;
				}
				if ((elementary_pids = (int *)malloc(num_of_program_elements * sizeof(int))) == NULL)
				{
					return -1;
				}
				max_num_of_elementary_pids = num_of_program_elements;
			}

			for (i = 0; i < num_of_program_elements; ++i)
			{
				elementary_pids[i] = program_element_map[i].elementary_pid;
			}
			*p_num_of_elementary_pids = num_of_program_elements;
			*p_elementary_pids = elementary_pids;

			return 0;

		}

		int GetStreamType(int elementary_pid, int *p_stream_type)
		{
			int index;
	
			index = Locate(elementary_pid);
			if (index >= 0)
			{
				*p_stream_type = program_element_map[index].stream_type;
				return 0;
			}

			return -1;
		}
	};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INIT = 16,
		NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INCR = 4
	};
#else
	static const int NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INIT = 16;
	static const int NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INCR = 4;
#endif

	int max_num_of_program_map_subtables;
	int num_of_program_map_subtables;
	struct _PSI_SI_PROGRAM_MAP_SUBTABLE
	{
		int program_number;
		int version_number;
		int pcr_pid;
		CPsiSiProgramElementMap ProgramElementMap;
	} *program_map_subtables;
	int max_num_of_program_numbers;
	int *program_numbers;

	int Locate(int program_number)
	{
		int index;
		int i;

		index = -num_of_program_map_subtables - 1;
		for (i = 0; i < num_of_program_map_subtables; ++i)
		{
			if (program_map_subtables[i].program_number == program_number)
			{
				index = i;
				break;
			}
			else if (program_map_subtables[i].program_number > program_number)
			{
				index = -i - 1;
				break;
			}
		}

		return index;
	}

	int IncreaseStorage(void)
	{
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *increased_program_map_subtables;
		int i;

		if ((increased_program_map_subtables = (struct _PSI_SI_PROGRAM_MAP_SUBTABLE *)realloc(program_map_subtables, 
			(max_num_of_program_map_subtables + NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INCR) * 
			sizeof(struct _PSI_SI_PROGRAM_MAP_SUBTABLE))) == NULL)
		{
			return -1;
		}

		program_map_subtables = increased_program_map_subtables;
		for (i = max_num_of_program_map_subtables; i < max_num_of_program_map_subtables + NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INCR; ++i)
		{
			program_map_subtables[i].ProgramElementMap.Construct();
		}
		max_num_of_program_map_subtables += NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INCR;

		return 0;
	}

	struct _PSI_SI_PROGRAM_MAP_SUBTABLE *AddSubtable(int program_number, int version_number)
	{
		int index;
		int i;

		if ((index = Locate(program_number)) >= 0)
		{
			if (program_map_subtables[index].version_number != version_number) 
			{
				program_map_subtables[index].version_number = version_number;
				program_map_subtables[index].ProgramElementMap.Init();
			}
		}
		else
		{
			if (num_of_program_map_subtables >= max_num_of_program_map_subtables)
			{
				if (IncreaseStorage() < 0)
				{
					return NULL;
				}
			}
			index = -index - 1;
			program_map_subtables[num_of_program_map_subtables].ProgramElementMap.Destruct();
			for (i = num_of_program_map_subtables;i > index; --i)
			{
				program_map_subtables[i] = program_map_subtables[i - 1];
			}
			program_map_subtables[index].program_number = program_number;
			program_map_subtables[index].version_number = version_number;
			program_map_subtables[index].ProgramElementMap.Construct();

			++num_of_program_map_subtables;			
		}

		return &program_map_subtables[index];
	}

	struct _PSI_SI_PROGRAM_MAP_SUBTABLE *GetSubtable(int program_number)
	{
		int index;

		if ((index = Locate(program_number)) < 0)
		{
			return NULL;
		}
		else
		{
			return &program_map_subtables[index];
		}
	}

public:
	int Init(void)
	{
		num_of_program_map_subtables = 0;
		
		return 0;
	}

	CPsiSiPmtHandler()
	{
		int i;

		if ((program_map_subtables = (struct _PSI_SI_PROGRAM_MAP_SUBTABLE *)malloc(
			NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INIT * sizeof(struct _PSI_SI_PROGRAM_MAP_SUBTABLE))) == NULL)
		{
			max_num_of_program_map_subtables = 0;
		}
		else
		{
			max_num_of_program_map_subtables = NUM_OF_PSI_SI_PROGRAM_MAP_SUBTABLE_INIT;
		}
		for (i = 0; i < max_num_of_program_map_subtables; ++i)
		{
			program_map_subtables[i].ProgramElementMap.Construct();
		}
		program_numbers = NULL;
		max_num_of_program_numbers = 0;

		Init();		
	}

	~CPsiSiPmtHandler()
	{
		int i;

		for (i = 0; i < max_num_of_program_map_subtables; ++i)
		{
			program_map_subtables[i].ProgramElementMap.Destruct();
		}
		if (program_map_subtables != NULL)
		{
			free(program_map_subtables);
			program_map_subtables = NULL;
		}
		if (program_numbers != NULL)
		{
			free(program_numbers);
			program_numbers = NULL;
		}
	}

	int PreprocessSection(BYTE *data, int data_len)
	{
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *ProgramMapSubtable;

		if (data_len < MAX_PSI_SI_SECTION_HEADER_LEN)
		{
			return 0;
		}
		if ((data[1] & 0xc0) != 0x80)	// skips unless section_syntax_indicator '0' == '10'
		{
			return -1;
		}
		if (!(data[5] & 0x01))	// skips unless current_next_indicator == '1'
		{
			return -2;
		}
		if (data[6] != 0 || data[7] != 0)	// skips unless section_number == last_section_number == 0
		{
			return -3;
		}
		if ((ProgramMapSubtable = GetSubtable((data[3] << 8) + data[4])) != NULL &&
			((data[5] & 0x3e) >> 1) == ProgramMapSubtable->version_number)	// skips unless version_number of current PMT changed
		{
			return -5;
		}
		
		return 0;
	}

	int ParseSection(BYTE *data, int data_len)
	{
		int program_number;
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *ProgramMapSubtable;
		int offset;

		program_number = (data[3] << 8) + data[4];
		if ((ProgramMapSubtable = AddSubtable(program_number, (data[5] & 0x3e) >> 1)) == NULL)
		{
			return -1;
		}

		offset = MAX_PSI_SI_SECTION_HEADER_LEN;
		ProgramMapSubtable->pcr_pid = ((data[offset] & 0x1f) << 8) + data[offset + 1];
		offset += 2 + 2 + ((data[offset + 2] & 0xf) << 8) + data[offset + 3];
		for ( ; offset <= data_len - PSI_SI_SECTION_CRC_32_LEN - 3 - 2; )
		{
			if (ProgramMapSubtable->ProgramElementMap.Add(((data[offset + 1] & 0x1f) << 8) + data[offset + 2], data[offset]) < 0)
			{
				return -2;
			}
			offset += 3 + 2 + ((data[offset + 3] & 0x0f) << 8) + data[offset + 4];
		}

		return 0;
	}

	int GetProgramNumbers(int *p_num_of_program_numbers, int **p_program_numbers)
	{
		int i;

		if (max_num_of_program_numbers < num_of_program_map_subtables)
		{
			if (program_numbers != NULL)
			{
				free(program_numbers);
				program_numbers = NULL;
			}	
			max_num_of_program_numbers = 0;
			if ((program_numbers = (int *)malloc(num_of_program_map_subtables * sizeof(int))) == NULL)
			{
				return -1;
			}
			max_num_of_program_numbers = num_of_program_map_subtables;
		}

		for (i = 0; i < num_of_program_map_subtables; ++i)
		{
			program_numbers[i] = program_map_subtables[i].program_number;
		}
		*p_num_of_program_numbers = num_of_program_map_subtables;
		*p_program_numbers = program_numbers;

		return 0;
	}

	int GetPcrPid(int program_number, int *p_pcr_pid)
	{
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *ProgramMapSubtable;

		if ((ProgramMapSubtable = GetSubtable(program_number)) == NULL)
		{
			return -1;
		}

		*p_pcr_pid = ProgramMapSubtable->pcr_pid;

		return 0;
	}

	int GetElementaryPids(int program_number, int *p_num_of_elementary_pids, int **p_elementary_pids)
	{
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *ProgramMapSubtable;

		if ((ProgramMapSubtable = GetSubtable(program_number)) == NULL)
		{
			return -1;
		}

		return ProgramMapSubtable->ProgramElementMap.GetElementaryPids(p_num_of_elementary_pids, p_elementary_pids);
	}

	int GetStreamType(int program_number, int elementary_pid, int *p_stream_type)
	{
		struct _PSI_SI_PROGRAM_MAP_SUBTABLE *ProgramMapSubtable;

		if ((ProgramMapSubtable = GetSubtable(program_number)) == NULL)
		{
			return -1;
		}

		return ProgramMapSubtable->ProgramElementMap.GetStreamType(elementary_pid, p_stream_type);
	}
};

class CPsiSiNitHandler: public CPsiSiTableHandler
{
private:
	class CPsiSiTransportStreamMap
	{
	private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		NUM_OF_PSI_SI_TRANSPORT_STREAM_INIT = 16,
		NUM_OF_PSI_SI_TRANSPORT_STREAM_INCR = 4
	};
#else
		static const int NUM_OF_PSI_SI_TRANSPORT_STREAM_INIT = 16;
		static const int NUM_OF_PSI_SI_TRANSPORT_STREAM_INCR = 4;
#endif

		int max_num_of_transport_streams;
		int num_of_transport_streams;
		struct _PSI_SI_TRANSPORT_STREAM
		{
			int transport_stream_id;
			int original_network_id;
			CPsiSiDescriptorPool *pTransportDescriptorPool;
		} *transport_stream_map;

		int Locate(int transport_stream_id)
		{
			int index;
			int i;

			index = -num_of_transport_streams - 1;
			for (i = 0; i < num_of_transport_streams; ++i)
			{
				if (transport_stream_map[i].transport_stream_id == transport_stream_id)
				{
					index = i;
					break;
				}
				else if (transport_stream_map[i].transport_stream_id > transport_stream_id)
				{
					index = -i - 1;
					break;
				}
			}

			return index;
		}

		int IncreaseStorage(void)
		{
			struct _PSI_SI_TRANSPORT_STREAM *increased_transport_stream_map;

			if ((increased_transport_stream_map = (struct _PSI_SI_TRANSPORT_STREAM *)realloc(transport_stream_map, 
				(max_num_of_transport_streams + NUM_OF_PSI_SI_TRANSPORT_STREAM_INCR) * 
				sizeof(struct _PSI_SI_TRANSPORT_STREAM))) == NULL)
			{
				return -1;
			}

			transport_stream_map = increased_transport_stream_map;
			max_num_of_transport_streams += NUM_OF_PSI_SI_TRANSPORT_STREAM_INCR;

			return 0;
		}

	public:
		int Init(void)
		{
			int i;

			for (i = 0; i < num_of_transport_streams; ++i)
			{
				if (transport_stream_map[i].pTransportDescriptorPool != NULL)
				{
					delete transport_stream_map[i].pTransportDescriptorPool;
				}
			}
			num_of_transport_streams = 0;

			return 0;
		}

		void Construct(void)
		{
			int i;
			
			if ((transport_stream_map = (struct _PSI_SI_TRANSPORT_STREAM *)malloc(
				NUM_OF_PSI_SI_TRANSPORT_STREAM_INIT * sizeof(struct _PSI_SI_TRANSPORT_STREAM))) == NULL)
			{
				max_num_of_transport_streams = 0;
			}
			else
			{
				max_num_of_transport_streams = NUM_OF_PSI_SI_TRANSPORT_STREAM_INIT;
			}
			num_of_transport_streams = 0;
		}

		CPsiSiTransportStreamMap()
		{
			Construct();
		}

		void Destruct()
		{
			int i;

			for (i = 0; i < num_of_transport_streams; ++i)
			{
				if (transport_stream_map[i].pTransportDescriptorPool != NULL)
				{
					delete transport_stream_map[i].pTransportDescriptorPool;
				}
			}
			if (transport_stream_map != NULL)
			{
				free(transport_stream_map);
				transport_stream_map = NULL;
			}
		}

		~CPsiSiTransportStreamMap()
		{
			Destruct();
		}

		int Add(int transport_stream_id, int original_network_id, CPsiSiDescriptorPool *pTransportDescriptorPool)
		{
			int index;
			int i;

			index = Locate(transport_stream_id);
			if (index >= 0)
			{
				transport_stream_map[index].original_network_id = original_network_id;
				if (transport_stream_map[index].pTransportDescriptorPool != NULL)
				{
					delete transport_stream_map[index].pTransportDescriptorPool;
				}
				transport_stream_map[index].pTransportDescriptorPool = pTransportDescriptorPool;
			}
			else
			{
				if (num_of_transport_streams >= max_num_of_transport_streams)
				{
					if (IncreaseStorage() < 0)
					{
						return -1;
					}
				}
				index = -index - 1;
				for (i = num_of_transport_streams;i > index; --i)
				{
					transport_stream_map[i] = transport_stream_map[i - 1];
				}
				transport_stream_map[index].transport_stream_id = transport_stream_id;
				transport_stream_map[index].original_network_id = original_network_id;
				transport_stream_map[index].pTransportDescriptorPool = pTransportDescriptorPool;
				
				++num_of_transport_streams;
			}

			return 0;
		}

		CPsiSiDescriptorPool *GetTransportDescriptorPool(int transport_stream_id)
		{
			int index;
			int i;

			index = Locate(transport_stream_id);
			if (index < 0)
			{
				return NULL;			
			}
			else
			{
				return transport_stream_map[index].pTransportDescriptorPool;
			}
		}
	};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INIT = 1,
		NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INCR = 1
	};
#else
	static const int NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INIT = 1;
	static const int NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INCR = 1;
#endif

	int max_num_of_network_information_subtables;
	int num_of_network_information_subtables;
	struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE
	{
		int table_id;
		int network_id;
		int version_number;
		int last_section_number;
		CPsiSiSectionNumberBitVector SectionParsedFlags;
		CPsiSiTransportStreamMap TransportStreamMap;
	} *network_information_subtables;
	CPsiSiDescriptorHandlerRegistry TransportDescriptorHandlerRegistry;
	int last_actual_network_id;

	int Locate(int table_id, int network_id)
	{
		int index;
		int i;

		index = -num_of_network_information_subtables - 1;
		for (i = 0; i < num_of_network_information_subtables; ++i)
		{
			if (network_information_subtables[i].table_id == table_id && 
				network_information_subtables[i].network_id == network_id)
			{
				index = i;
				break;
			}
			else if (network_information_subtables[i].table_id > table_id || 
				network_information_subtables[i].table_id == table_id && network_information_subtables[i].network_id > network_id)
			{
				index = -i - 1;
				break;
			}
		}

		return index;
	}

	int IncreaseStorage(void)
	{
		struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *increased_network_information_subtables;
		int i;

		if ((increased_network_information_subtables = (struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *)realloc(network_information_subtables, 
			(max_num_of_network_information_subtables + NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INCR) * 
			sizeof(struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE))) == NULL)
		{
			return -1;
		}

		network_information_subtables = increased_network_information_subtables;
		for (i = max_num_of_network_information_subtables; i < max_num_of_network_information_subtables + 
			NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INCR; ++i)
		{
			network_information_subtables[i].SectionParsedFlags.Construct();
			network_information_subtables[i].TransportStreamMap.Construct();
		}
		max_num_of_network_information_subtables += NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INCR;

		return 0;
	}


	struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *AddSubtable(int table_id, int network_id, int version_number, int last_section_number)
	{
		int index;
		int i;

		if ((index = Locate(table_id, network_id)) >= 0)
		{
			if (network_information_subtables[index].version_number != version_number ||
				network_information_subtables[index].last_section_number != last_section_number)
			{
				network_information_subtables[index].version_number = version_number;
				network_information_subtables[index].last_section_number = last_section_number;
				network_information_subtables[index].SectionParsedFlags.SetLastSectionNumber(last_section_number);
				network_information_subtables[index].TransportStreamMap.Init();
			}
		}
		else
		{
			if (num_of_network_information_subtables >= max_num_of_network_information_subtables)
			{
				if (IncreaseStorage() < 0)
				{
					return NULL;
				}
			}
			index = -index - 1;
			network_information_subtables[num_of_network_information_subtables].TransportStreamMap.Destruct();
			for (i = num_of_network_information_subtables;i > index; --i)
			{
				network_information_subtables[i] = network_information_subtables[i - 1];
			}
			network_information_subtables[index].table_id = table_id;
			network_information_subtables[index].network_id = network_id;
			network_information_subtables[index].version_number = version_number;
			network_information_subtables[index].last_section_number = last_section_number;
			network_information_subtables[index].SectionParsedFlags.SetLastSectionNumber(last_section_number);
			network_information_subtables[index].TransportStreamMap.Construct();

			++num_of_network_information_subtables;			
		}

		return &network_information_subtables[index];
	}

	struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *GetSubtable(int table_id, int network_id)
	{
		int index;

		if ((index = Locate(table_id, network_id)) < 0)
		{
		return NULL;
	}
		else
		{
			return &network_information_subtables[index];
		}
	}

public:
	int Init(void)
	{
		int i;

		if (num_of_network_information_subtables > 0)
		{
			for (i = 0; i < num_of_network_information_subtables; ++i)
			{
				network_information_subtables[i].TransportStreamMap.Init();
			}
			num_of_network_information_subtables = 0;
		}
		TransportDescriptorHandlerRegistry.Init();
		last_actual_network_id = -1;
		
		return 0;		
	}

	CPsiSiNitHandler()
	{
		int i;

		if ((network_information_subtables = (struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *)malloc(
			NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INIT * sizeof(struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE))) == NULL)
		{
			max_num_of_network_information_subtables = 0;
		}
		else
		{
			max_num_of_network_information_subtables = NUM_OF_PSI_SI_NETWORK_INFORMATION_SUBTABLE_INIT;
			for (i = 0; i < max_num_of_network_information_subtables; ++i)
			{
				network_information_subtables[i].SectionParsedFlags.Construct();
				network_information_subtables[i].TransportStreamMap.Construct();
			}
		}
		
		Init();
	}

	~CPsiSiNitHandler()
	{
		int i;
		
		for (i = 0; i < max_num_of_network_information_subtables; ++i)
		{
			network_information_subtables[i].SectionParsedFlags.Destruct();
			network_information_subtables[i].TransportStreamMap.Destruct();
		}
		if (network_information_subtables != NULL)
		{
			free(network_information_subtables);
			network_information_subtables = NULL;
		}
	}

	int RegisterTransportDescriptorHandler(int descriptor_tag, CPsiSiDescriptorHandler *pTransportDescriptorHandler)
	{
		return TransportDescriptorHandlerRegistry.RegisterHandler(descriptor_tag, pTransportDescriptorHandler);
	}

	int PreprocessSection(BYTE *data, int data_len)
	{
		int section_length;
		struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *NetworkInformationSubtable;

		if (data_len < MAX_PSI_SI_SECTION_HEADER_LEN)
		{
			return 0;
		}
		if (!(data[1] & 0x80))	// skips unless section_syntax_indicator == '1'
		{
			return -1;
		}
		if (!(data[5] & 0x01))	// skips unless current_next_indicator == '1'
		{
			return -2;
		}
		if (data[6] > data[7])	// skips unless section_number <= last_section_number
		{
			return -3;
		}
		if ((section_length = ((data[1] & 0x0f) << 8) + data[2]) > 1021)
		{
			return -4;
		}
		if ((NetworkInformationSubtable = GetSubtable(data[0], (data[3] << 8) + data[4])) != NULL &&
			((data[5] & 0x3e) >> 1) == NetworkInformationSubtable->version_number && 
			data[7] == NetworkInformationSubtable->last_section_number && 
			NetworkInformationSubtable->SectionParsedFlags.IsSet(data[6]))	// skips unless version_number of current NIT changed
		{
			return -5;
		}
		
		return 0;
	}

	int ParseSection(BYTE *data, int data_len)
	{
		int table_id;
		int network_id;
		struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *NetworkInformationSubtable;
		int offset;
		int transport_stream_id;
		int original_network_id;
		int transport_stream_loop_end_offset;
		int transport_descriptors_length;
		int transport_descriptors_end_offset;
		CPsiSiDescriptorPool *pTransportDescriptorPool;
		CPsiSiDescriptorHandler *pTransportDescriptorHandler;
		BYTE *descriptor;

		table_id = data[0];
		network_id = (data[3] << 8) + data[4];
		if ((NetworkInformationSubtable = AddSubtable(table_id, network_id, (data[5] & 0x3e) >> 1, data[7])) == NULL)
		{
			return -1;
		}

		data_len -= PSI_SI_SECTION_CRC_32_LEN;
		offset = MAX_PSI_SI_SECTION_HEADER_LEN;

		// reserved_future_use 4-bits
		// network_descriptors_length 12-bits 2-bytes
		offset += 2 + ((data[offset] & 0xf) << 8) + data[offset + 1];
		if (offset > data_len - 2)
		{
			return -2;
		}

		// reserved_future_use 4-bits
		// transport_stream_loop_length 12-bits 2-bytes
		transport_stream_loop_end_offset = offset + 2 + ((data[offset] & 0xf) << 8) + data[offset + 1];
		if (transport_stream_loop_end_offset != data_len)
		{
			return -3;
		}
		offset += 2;

		for ( ; offset <= data_len - 6; )
		{
			// transport_stream_id 16-bits 2-bytes
			transport_stream_id = (data[offset] << 8) + data[offset + 1];
			offset += 2;
			// original_network_id 16-bits 2-bytes
			original_network_id = (data[offset] << 8) + data[offset + 1];
			offset += 2;
			// reserved_future_use 4-bits
			// transport_descriptors_length 12-bits 2-bytes
			transport_descriptors_length = ((data[offset] & 0x0f) << 8) + data[offset + 1];
			offset += 2;
			if (transport_descriptors_length > 0)
			{
				transport_descriptors_end_offset = offset + transport_descriptors_length;
				if (transport_descriptors_end_offset > data_len)
				{
					return -4;
				}

				if ((pTransportDescriptorPool = new CPsiSiDescriptorPool) == NULL)
				{
					return -5;
				}

				for ( ; offset < transport_descriptors_end_offset; offset += 1 + 1 + data[offset + 1])
				{
					if ((pTransportDescriptorHandler = TransportDescriptorHandlerRegistry.GetHandler(data[offset])) != NULL)
					{
						descriptor = data + offset;
						if (pTransportDescriptorHandler->Parse(descriptor) < 0)
						{
							return -6;
						}

						if (pTransportDescriptorPool->Add(descriptor) < 0)
						{
							return -7;
						}
					}
				}
				if (offset != transport_descriptors_end_offset)
				{
					return -8;
				}
			}
			else
			{
				pTransportDescriptorPool = NULL;
			}
			if (NetworkInformationSubtable->TransportStreamMap.Add(transport_stream_id, original_network_id, pTransportDescriptorPool) < 0)
			{
				if (pTransportDescriptorPool != NULL)
				{
					delete pTransportDescriptorPool;
				}

				return -9;
			}
		}
		if (offset != data_len)
		{
			return -10;
		}

		NetworkInformationSubtable->SectionParsedFlags.Set(data[6]);
		if (table_id == 0x40 && NetworkInformationSubtable->SectionParsedFlags.IsAllSet())
		{
			last_actual_network_id = network_id;
		}

		return 0;
	}

	int GetActualNetworkId(int *p_network_id)
	{
		if (last_actual_network_id < 0)
		{
			return -1;
		}

		*p_network_id = last_actual_network_id;

		return 0;
	}

	int GetTransportDescriptor(int table_id, int network_id, int transport_stream_id, int descriptor_tag, BYTE **p_descriptor)
	{
		struct _PSI_SI_NETWORK_INFORMATION_SUBTABLE *NetworkInformationSubtable;
		CPsiSiDescriptorPool *pTransportDescriptorPool;

		if (table_id == 0x40 && network_id < 0)
		{
			if (last_actual_network_id < 0)
			{
				return -1;
			}

			network_id = last_actual_network_id;
		}
		if ((NetworkInformationSubtable = GetSubtable(table_id, network_id)) == NULL)
		{
			return -2;
		}

		if ((pTransportDescriptorPool = NetworkInformationSubtable->TransportStreamMap.GetTransportDescriptorPool(transport_stream_id)) == 
			NULL)
		{
			return -3;
		}

		if (pTransportDescriptorPool->Get(descriptor_tag, p_descriptor) < 0)
		{
			return -4;
		}

		return 0;
	}
};

class CPsiSiTableHandlerRegistry
{
private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INIT = 8,
		NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INCR = 2
	};
#else
	static const int NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INIT = 8;
	static const int NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INCR = 2;
#endif

	int max_num_of_psi_si_table_handler_map_entry;
	int num_of_psi_si_table_handler_map_entry;
	struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY
	{
		int pid;
		int table_id;
		CPsiSiTableHandler *handler;
	} *psi_si_table_handler_map;

	int LocateHandler(int pid, int table_id)
	{
		int i;

		for (i = 0; i < num_of_psi_si_table_handler_map_entry; ++i)
		{
			if (psi_si_table_handler_map[i].pid == pid && psi_si_table_handler_map[i].table_id == table_id)
			{
				return i;
			}
		}

		return -1;
	}

	int IncreaseStorage(void)
	{
		struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY *increased_psi_si_table_handler_map;
		int i;

		if ((increased_psi_si_table_handler_map = (struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY *)realloc(psi_si_table_handler_map, 
			(max_num_of_psi_si_table_handler_map_entry + NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INCR) * 
			sizeof(struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY))) == NULL)
		{
			return -1;
		}

		psi_si_table_handler_map = increased_psi_si_table_handler_map;
		max_num_of_psi_si_table_handler_map_entry += NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INCR;
		for (i = num_of_psi_si_table_handler_map_entry; i < max_num_of_psi_si_table_handler_map_entry; ++i)
		{
			psi_si_table_handler_map[i].pid = -1;
		}

		return 0;
	}

public:
	int Init(void)
	{
		int i;

		num_of_psi_si_table_handler_map_entry = 0;
		for (i = 0; i < max_num_of_psi_si_table_handler_map_entry; ++i)
		{
			psi_si_table_handler_map[i].pid = -1;
		}

		return 0;
	}

	CPsiSiTableHandlerRegistry()
	{
		if ((psi_si_table_handler_map = (struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY *)malloc(NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INIT * 
			sizeof(struct _PSI_SI_TABLE_HANDLER_MAP_ENTRY))) == NULL)
		{
			max_num_of_psi_si_table_handler_map_entry = 0;
		}
		else
		{
			max_num_of_psi_si_table_handler_map_entry = NUM_OF_PSI_SI_TABLE_HANDLER_MAP_ENTRY_INIT;
		}
		Init();
	}

	~CPsiSiTableHandlerRegistry()
	{
		if (psi_si_table_handler_map != NULL)
		{
			free(psi_si_table_handler_map);
			psi_si_table_handler_map = NULL;
		}
	}

	int RegisterHandler(int pid, int table_id, CPsiSiTableHandler *handler)
	{
		if (LocateHandler(pid, table_id) >= 0)
		{
			return -1;
		}

		if (num_of_psi_si_table_handler_map_entry >= max_num_of_psi_si_table_handler_map_entry)
		{
			if (IncreaseStorage() < 0)
			{
				return -2;
			}
		}

		psi_si_table_handler_map[num_of_psi_si_table_handler_map_entry].pid = pid;
		psi_si_table_handler_map[num_of_psi_si_table_handler_map_entry].table_id = table_id;
		psi_si_table_handler_map[num_of_psi_si_table_handler_map_entry++].handler = handler;

		return 0;
	}

	int UnRegisterHandler(int pid, int table_id)
	{
		int index;
		int i;
		
		if ((index = LocateHandler(pid, table_id)) < 0)
		{
			return -1;
		}

		for (i = index + 1; i < num_of_psi_si_table_handler_map_entry; ++i)
		{
			psi_si_table_handler_map[i - 1] = psi_si_table_handler_map[i];
		}
		psi_si_table_handler_map[num_of_psi_si_table_handler_map_entry-- - 1].pid = -1;

		return 0;
	}

	CPsiSiTableHandler *GetHandler(int pid, int table_id)
	{
		int index;

		if ((index = LocateHandler(pid, table_id)) < 0)
		{
			return NULL;
		}

		return psi_si_table_handler_map[index].handler;
	}
};

class CPsiSiSectionBufPool
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
public:
	enum
	{
		NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INIT = 16,
		NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INCR = 4,
		MAX_PSI_SI_SECTION_LEN = 1024,
		PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_BUF_ACTIVE = 0x01,
		PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_SECTION_HEADER_CHECKED = 0x02
	};
private:
#else
private:
	static const int NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INIT = 16;
	static const int NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INCR = 4;
	static const int MAX_PSI_SI_SECTION_LEN = 1024;
	static const unsigned int PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_BUF_ACTIVE = 0x01;
	static const unsigned int PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_SECTION_HEADER_CHECKED = 0x02;
#endif

	int max_num_of_psi_si_section_buf_map_entry;
	int num_of_psi_si_section_buf_map_entry;
	struct _PSI_SI_SECTION_BUF_MAP_ENTRY
	{
		int pid;
		int continuity_counter;
		int table_id;
		int section_length;
		BYTE buf[MAX_PSI_SI_SECTION_LEN];
		int buf_len;
		unsigned int flags;
	} *psi_si_section_buf_map;
	int pid_to_psi_si_section_buf_map_entry[NUM_OF_PIDS];

	int IncreaseStorage(void)
	{
		struct _PSI_SI_SECTION_BUF_MAP_ENTRY *increased_psi_si_section_buf_map;
		int i;

		if ((increased_psi_si_section_buf_map = (struct _PSI_SI_SECTION_BUF_MAP_ENTRY *)realloc(psi_si_section_buf_map, 
			(max_num_of_psi_si_section_buf_map_entry + NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INCR) * 
			sizeof(struct _PSI_SI_SECTION_BUF_MAP_ENTRY))) == NULL)
		{
			return -1;
		}

		psi_si_section_buf_map = increased_psi_si_section_buf_map;
		max_num_of_psi_si_section_buf_map_entry += NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INCR;
		for (i = num_of_psi_si_section_buf_map_entry; i < max_num_of_psi_si_section_buf_map_entry; ++i)
		{
			psi_si_section_buf_map[i].pid = -1;
		}

		return 0;
	}

public:
	void Init(void)
	{
		int i;

		num_of_psi_si_section_buf_map_entry = 0;
		for (i = 0; i < max_num_of_psi_si_section_buf_map_entry; ++i)
		{
			psi_si_section_buf_map[i].pid = -1;
		}
		for (i = 0; i < NUM_OF_PIDS; ++i)
		{
			pid_to_psi_si_section_buf_map_entry[i] = -1;
		}
	}

	CPsiSiSectionBufPool()
	{
		psi_si_section_buf_map = (struct _PSI_SI_SECTION_BUF_MAP_ENTRY *)malloc(NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INIT * 
			sizeof(struct _PSI_SI_SECTION_BUF_MAP_ENTRY));
		if (psi_si_section_buf_map == NULL)
		{
			max_num_of_psi_si_section_buf_map_entry = 0;
		}
		else
		{
			max_num_of_psi_si_section_buf_map_entry = NUM_OF_PSI_SI_SECTION_BUF_MAP_ENTRY_INIT;
		}
		Init();
	}

	~CPsiSiSectionBufPool()
	{
		if (psi_si_section_buf_map != NULL)
		{
			free(psi_si_section_buf_map);
			psi_si_section_buf_map = NULL;
		}
	}

	int AllocateBuf(int pid)
	{
		int index;
		int i;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) >= 0)
		{
			return -1;
		}

		if (num_of_psi_si_section_buf_map_entry >= max_num_of_psi_si_section_buf_map_entry)
		{
			if (IncreaseStorage() < 0)
			{
				return -2;
			}
			index = num_of_psi_si_section_buf_map_entry;
		}
		else
		{
			for (i = max_num_of_psi_si_section_buf_map_entry - 1; i >= 0; --i)
			{
				if (psi_si_section_buf_map[i].pid == -1)
				{
					index = i;
					break;
				}
			}
			if (i < 0)
			{
				return -3;
			}
		}
		psi_si_section_buf_map[index].pid = pid;
		psi_si_section_buf_map[index].flags = 0;
		pid_to_psi_si_section_buf_map_entry[pid] = index;
		++num_of_psi_si_section_buf_map_entry;

		return 0;
	}

	int ReleaseBuf(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) < 0)
		{
			return -1;
		}

		psi_si_section_buf_map[index].pid = -1;
		pid_to_psi_si_section_buf_map_entry[pid] = -1;
		--num_of_psi_si_section_buf_map_entry;

		return 0;
	}

	int IsBufAllocated(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return 0;
		}

		return 1;
	}

	int FillBuf(int pid, int continuity_counter, BYTE *data, int data_len)
	{
		int index;
		int section_length;
		int copy_data_len;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return -1;
		}

		if (data_len >= MIN_PSI_SI_SECTION_HEADER_LEN)
		{
			section_length =  ((data[1] & 0x0f) << 8) + data[2];
			if (section_length > MAX_PSI_SI_SECTION_LEN - MIN_PSI_SI_SECTION_HEADER_LEN)
			{
				psi_si_section_buf_map[index].flags = 0;
				return -2;
			}

			psi_si_section_buf_map[index].section_length = section_length;
			copy_data_len = MIN_PSI_SI_SECTION_HEADER_LEN + section_length;
			if (copy_data_len > data_len)
			{
				copy_data_len = data_len;
			}
		}
		else
		{
			psi_si_section_buf_map[index].section_length = -1;
			copy_data_len = data_len;
		}
		memcpy(psi_si_section_buf_map[index].buf, data, copy_data_len);
		psi_si_section_buf_map[index].buf_len = copy_data_len;

		psi_si_section_buf_map[index].pid = pid;
		psi_si_section_buf_map[index].continuity_counter = continuity_counter;
		psi_si_section_buf_map[index].flags = PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_BUF_ACTIVE;

		return 0;
	}

	int IsBufActive(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return 0;
		}
		if (psi_si_section_buf_map[index].flags & PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_BUF_ACTIVE)
		{
			return 1;
		}

		return 0;
	}

	int AppendToBuf(int pid, int continuity_counter, BYTE *data, int data_len)
	{
		int index;
		int copy_data_len;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return -1;
		}
		if (!(psi_si_section_buf_map[index].flags & PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_BUF_ACTIVE))
		{
			return -2;
		}
		if (((psi_si_section_buf_map[index].continuity_counter + 1) % 16) != continuity_counter)
		{
			return -3;
		}

		if (psi_si_section_buf_map[index].buf_len >= MIN_PSI_SI_SECTION_HEADER_LEN)
		{
			copy_data_len = MIN_PSI_SI_SECTION_HEADER_LEN + psi_si_section_buf_map[index].section_length - 
				psi_si_section_buf_map[index].buf_len;
			if (copy_data_len > data_len)
			{
				copy_data_len = data_len;
			}
		}
		else
		{
			copy_data_len = MAX_PSI_SI_SECTION_LEN - psi_si_section_buf_map[index].buf_len;
			if (copy_data_len > data_len)
			{
				copy_data_len = data_len;
			}
		}
		memcpy(psi_si_section_buf_map[index].buf + psi_si_section_buf_map[index].buf_len, data, copy_data_len);
		psi_si_section_buf_map[index].buf_len += copy_data_len;
		psi_si_section_buf_map[index].continuity_counter = continuity_counter;
		if (psi_si_section_buf_map[index].section_length < 0 && psi_si_section_buf_map[index].buf_len >= MIN_PSI_SI_SECTION_HEADER_LEN)
		{
			psi_si_section_buf_map[index].section_length = ((psi_si_section_buf_map[index].buf[1] & 0x0f) << 8) + 
				psi_si_section_buf_map[index].buf[2];
			if (psi_si_section_buf_map[index].section_length > MAX_PSI_SI_SECTION_LEN - MIN_PSI_SI_SECTION_HEADER_LEN)
			{
				psi_si_section_buf_map[index].flags = 0;
				return -4;
			}
		}

		return 0;
	}

	int IsSectionComplete(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return 0;
		}
		if (psi_si_section_buf_map[index].buf_len >= MIN_PSI_SI_SECTION_HEADER_LEN && 
			psi_si_section_buf_map[index].buf_len >= MIN_PSI_SI_SECTION_HEADER_LEN + psi_si_section_buf_map[index].section_length)
		{
			return 1;
		}

		return 0;
	}

	int GetBuf(int pid, BYTE **p_data, int *p_data_len)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return -1;
		}		

		*p_data = psi_si_section_buf_map[index].buf;
		*p_data_len = psi_si_section_buf_map[index].buf_len;

		return 0;
	}

	int IsSectionHeaderChecked(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return 0;
		}
		if (psi_si_section_buf_map[index].flags & PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_SECTION_HEADER_CHECKED)
		{
			return 1;
		}

		return 0;
	}

	int IsSectionHeaderComplete(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return 0;
		}
		if (psi_si_section_buf_map[index].buf_len >= MAX_PSI_SI_SECTION_HEADER_LEN)
		{
			return 1;
		}

		return 0;
	}

	int DeactivateBuf(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return -1;
		}
		psi_si_section_buf_map[index].flags = 0;

		return 0;
	}

	int SetSectionHeaderChecked(int pid)
	{
		int index;

		if ((index = pid_to_psi_si_section_buf_map_entry[pid]) == -1)
		{
			return -1;
		}
		psi_si_section_buf_map[index].flags |= PSI_SI_SECTION_BUF_MAP_ENTRY_FLAG_SECTION_HEADER_CHECKED;

		return 0;
	}
};

class CTsBitrateCalculator
{
private:
	class CPidToPcrSamplePool
	{
	private:
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
		enum
		{
			MAX_NUM_OF_PID_TO_PCR_SAMPLE_MAP_ENTRIES = 4,
			SUFFICIENT_NUM_OF_PCRS = 16
		};
#else
		static const int MAX_NUM_OF_PID_TO_PCR_SAMPLE_MAP_ENTRIES = 4;
		static const int SUFFICIENT_NUM_OF_PCRS = 16;
#endif

		int num_of_pid_to_pcr_sample_map_entries;
		struct _PID_TO_PCR_SAMPLE_MAP_ENTRY
		{
			int pid;
			__int64 i64_pcr1;
			__int64 i64_pcr2;
			int ts_packet_index1;
			int ts_packet_index2;
			int num_of_pcrs;
		} pid_to_pcr_sample_map[MAX_NUM_OF_PID_TO_PCR_SAMPLE_MAP_ENTRIES];
		int delta_index;
		int delta_index_unique;

		int Locate(int pid)
		{
			int i;

			for (i = 0; i < num_of_pid_to_pcr_sample_map_entries; ++i)
			{
				if (pid_to_pcr_sample_map[i].pid == pid)
				{
					return i;
				}
			}

			return -1;	
		}

		int GetBestDelta(__int64 *p_i64_delta_pcr, int *p_delta_ts_packet_index)
		{
			int max_num_of_pcrs;
			int max_num_of_pcrs_index;
			int i;

			max_num_of_pcrs = 1;
			max_num_of_pcrs_index = -1;
			for (i = 0; i < num_of_pid_to_pcr_sample_map_entries; ++i)
			{
				if (pid_to_pcr_sample_map[i].num_of_pcrs > max_num_of_pcrs)
				{
					max_num_of_pcrs = pid_to_pcr_sample_map[i].num_of_pcrs;
					max_num_of_pcrs_index = i;
				}
			}
			if (max_num_of_pcrs_index < 0)
			{
				return -1;
			}

			*p_i64_delta_pcr = pid_to_pcr_sample_map[max_num_of_pcrs_index].i64_pcr2 - 
				pid_to_pcr_sample_map[max_num_of_pcrs_index].i64_pcr1;
			*p_delta_ts_packet_index = pid_to_pcr_sample_map[max_num_of_pcrs_index].ts_packet_index2 -
				pid_to_pcr_sample_map[max_num_of_pcrs_index].ts_packet_index1;

			return 0;
		}

	public:
		int Init(void)
		{
			num_of_pid_to_pcr_sample_map_entries = 0;
			delta_index = -1;

			return 0;
		}

		CPidToPcrSamplePool()
		{
			Init();
		}

		int IsSampled(int pid)
		{
			if (num_of_pid_to_pcr_sample_map_entries < MAX_NUM_OF_PID_TO_PCR_SAMPLE_MAP_ENTRIES || Locate(pid) >= 0)
			{
				return 1;
			}

			return 0;
		}

		void AddSample(int pid, __int64 i64_pcr, int ts_packet_index)
		{
			int index;

			if ((index = Locate(pid)) >= 0)
			{
				if (pid_to_pcr_sample_map[index].num_of_pcrs >= SUFFICIENT_NUM_OF_PCRS)
				{
					return;
				}

				if ((pid_to_pcr_sample_map[index].num_of_pcrs == 1 && pid_to_pcr_sample_map[index].i64_pcr1 > i64_pcr) ||
					(pid_to_pcr_sample_map[index].num_of_pcrs > 1 && pid_to_pcr_sample_map[index].i64_pcr2 > i64_pcr))	
					// Restarts if PCR Wrap-Around			
				{
						pid_to_pcr_sample_map[index].i64_pcr1 = i64_pcr;
						pid_to_pcr_sample_map[index].ts_packet_index1 = ts_packet_index;
						pid_to_pcr_sample_map[index].num_of_pcrs = 1;
				}
				else
				{
					pid_to_pcr_sample_map[index].i64_pcr2 = i64_pcr;
					pid_to_pcr_sample_map[index].ts_packet_index2 = ts_packet_index;
					++pid_to_pcr_sample_map[index].num_of_pcrs;
					if (pid_to_pcr_sample_map[index].num_of_pcrs >= SUFFICIENT_NUM_OF_PCRS)
					{
						if (delta_index < 0)
						{
							delta_index = index;
							delta_index_unique = 1;
						}
						else if (delta_index != index)
						{
							delta_index_unique = 0;
						}
					}
				}
			}
			else if (num_of_pid_to_pcr_sample_map_entries < MAX_NUM_OF_PID_TO_PCR_SAMPLE_MAP_ENTRIES)
			{
				pid_to_pcr_sample_map[num_of_pid_to_pcr_sample_map_entries].pid = pid;
				pid_to_pcr_sample_map[num_of_pid_to_pcr_sample_map_entries].i64_pcr1 = i64_pcr;
				pid_to_pcr_sample_map[num_of_pid_to_pcr_sample_map_entries].ts_packet_index1 = ts_packet_index;
				pid_to_pcr_sample_map[num_of_pid_to_pcr_sample_map_entries].num_of_pcrs = 1;
				++num_of_pid_to_pcr_sample_map_entries;
			}
		}

		int IsDeltaReady(void)
		{
			return delta_index >= 0;
		}

		int IsEmpty(void)
		{
			return num_of_pid_to_pcr_sample_map_entries == 0;
		}

		int GetDelta(__int64 *p_i64_delta_pcr, int *p_delta_ts_packet_index)
		{
			int i;

			if (delta_index >= 0 && delta_index_unique)
			{
				*p_i64_delta_pcr = pid_to_pcr_sample_map[delta_index].i64_pcr2 - pid_to_pcr_sample_map[delta_index].i64_pcr1;
				*p_delta_ts_packet_index = pid_to_pcr_sample_map[delta_index].ts_packet_index2 - 
					pid_to_pcr_sample_map[delta_index].ts_packet_index1;
			}
			else
			{
				return GetBestDelta(p_i64_delta_pcr, p_delta_ts_packet_index);
			}
			
			return 0;
		}

		int IsMinimalDeltaReady(void)
		{
			int i;

			if (delta_index >= 0)
			{
				return 1;
			}

			for (i = 0; i < num_of_pid_to_pcr_sample_map_entries; ++i)
			{
				if (pid_to_pcr_sample_map[i].num_of_pcrs > 1)
				{
					return 1;
				}
			}

			return 0;
		}
	};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum
	{
		MAX_PID_PLUS_1 = 0x1fff + 1
	};
#else
	static const int MAX_PID_PLUS_1 = 0x1fff + 1;
#endif
	
	int ts_bitrate;
	int default_ts_bitrate;
	int ts_bitrate_ready;
	int ts_packet_count;
	struct _PID_INFO
	{
		int packet_count;
	} pid_infos[MAX_PID_PLUS_1];
	CPidToPcrSamplePool PidToPcrSamplePool;

public:
	int Init(void)
	{
		int i;

		ts_bitrate = -1;
		default_ts_bitrate = -1;
		ts_bitrate_ready = 0;
		ts_packet_count = 0;
		for (i = 0; i < MAX_PID_PLUS_1; ++i)
		{
			pid_infos[i].packet_count = 0;
		}
		PidToPcrSamplePool.Init();

		return 0;
	}

	void SetDefaultTsBitrate(int ts_bitrate)
	{
		default_ts_bitrate = ts_bitrate;
	}

	int ProcessTsPacket(BYTE *data, int data_len)
	{
		int pid;	
		int adaptation_field_control;
		int adaptation_field_length;
		__int64 i64_pcr;

		if (data[1] & 0x80)
		{
			return 0;
		}

		++ts_packet_count;
		pid = ((data[1] & 0x1f) << 8) + data[2];
		++pid_infos[pid].packet_count;
		if(pid == 0x1FFF)
		{
			return 0;
		}
		adaptation_field_control = (data[3] & 0x30) >> 4;
		if (adaptation_field_control == 0x02 || adaptation_field_control == 0x03)
		{
			adaptation_field_length = data[4];
			if (adaptation_field_length > 6 && (data[5] & 0x10))
			{
				if (PidToPcrSamplePool.IsSampled(pid))
				{
					i64_pcr = data[6];		
					i64_pcr <<= 25;
					i64_pcr += data[7] << 17;
					i64_pcr += data[8] << 9;
					i64_pcr += data[9] << 1;
#if 0
					i64_pcr += (data[10] & 0x80) >> 7;
#else
					if (data[10] & 0x80)	
					{
						++i64_pcr;
					}
#endif
					i64_pcr *= 300;
#if 0
					i64_pcr += (data[10] & 0x01) << 8;
#else
					if (data[10] & 0x01)
					{
						i64_pcr += 0x100;
					}
#endif
					i64_pcr += data[11];

					PidToPcrSamplePool.AddSample(pid, i64_pcr, ts_packet_count);
					if (ts_bitrate_ready = PidToPcrSamplePool.IsDeltaReady())
					{
						UpdateTsBitrate();
					}
				}
			}
		}

		return 0;
	}

	int IsBitrateReady(void)
	{
		return ts_bitrate_ready;
	}

	void UpdateTsBitrate(void)
	{
		__int64 i64_pcr_delta;
		int ts_packet_index_delta;

		if (PidToPcrSamplePool.IsEmpty())
		{
			ts_bitrate = -1;
		}
		else if (PidToPcrSamplePool.GetDelta(&i64_pcr_delta, &ts_packet_index_delta) < 0)
		{
			ts_bitrate = default_ts_bitrate;
		}
		else	
		{
			ts_bitrate = 188 * 8 * ts_packet_index_delta * 27e6 / i64_pcr_delta;
		}
	}

	int GetTsBitrate(int *p_bitrate)
	{
		*p_bitrate = ts_bitrate;

		return 0;
	}

	int GetBitrate(int pid, int *p_bitrate)
	{
		if (pid < 0 || pid >= MAX_PID_PLUS_1)
		{
			return -1;
		}

		if (ts_bitrate >= 0)
		{
			*p_bitrate = (double)pid_infos[pid].packet_count / ts_packet_count * ts_bitrate;
		}
		else
		{
			*p_bitrate = -1;	// NO PCR in TS
		}

		return 0;
	}

	int IsPcrDeltaAvaliable(void)
	{
		return PidToPcrSamplePool.IsMinimalDeltaReady();
	}
};


CPsiSiTableHandlerRegistry PsiSiTableHandlerRegistry;
CPsiSiSectionBufPool PsiSiSectionBufPool;
CPsiSiPatHandler PsiSiPatHandler;
CPsiSiPmtHandler PsiSiPmtHandler;
CPsiSiSatelliteDeliverySystemDescriptorHandler PsiSiSatelliteDeliverySystemDescriptorHandler;
CPsiSiCableDeliverySystemDescriptorHandler PsiSiCableDeliverySystemDescriptorHandler;
CPsiSiTerrestrialDeliverySystemDescriptorHandler PsiSiTerrestrialDeliverySystemDescriptorHandler;
CPsiSiS2SatelliteDeliverySystemDescriptorHandler PsiSiS2SatelliteDeliverySystemDescriptorHandler;
CPsiSiNitHandler PsiSiNitHandler;
CTsBitrateCalculator TsBitrateCalculator;


int CHldUtils::RunTs_Parser(char *szFile, int default_bitrate, int nitFlag, struct _TSPH_TS_INFO **ppTsInfo)
{
#define _MAX_DATA_LEN	(32 * 1024 * 1024)
	
	FILE *fp;
	int data_len;
	BYTE *data;
	int read_data_len;

	fp = fopen(szFile, "rb");
	if( fp == NULL)
		return 0;
	data_len = _MAX_DATA_LEN;
	data = (BYTE *)malloc(data_len);
	if(data == NULL)
	{
		fclose(fp);
		return 0;
	}
	read_data_len = fread(data, 1, data_len, fp);
	fclose(fp);
	RunTs_Parser(data, read_data_len, default_bitrate, nitFlag, ppTsInfo);
	free(data);
	data = NULL;
	return 0;
}


int psi_si_pat_update_callback(int transport_stream_id)
{
	int num_of_pmt_pids;
	int *pmt_pids;
	int i;
	
	if (PsiSiPatHandler.GetPmtPids(transport_stream_id, &num_of_pmt_pids, &pmt_pids) < 0)
	{
		return -1;
	}

	for (i = 0; i < num_of_pmt_pids; ++i)
	{
		PsiSiTableHandlerRegistry.RegisterHandler(pmt_pids[i], 0x02, &PsiSiPmtHandler);
		PsiSiSectionBufPool.AllocateBuf(pmt_pids[i]);
	}

	return 0;
}


int CHldUtils::init_psi_si_table_parsing_context(void)
{
	int i;

	FreeTsParserMemory();

	if (working_ts_info.nit_only)
	{
		PsiSiPatHandler.Init();
		PsiSiNitHandler.Init();

		PsiSiNitHandler.RegisterTransportDescriptorHandler(0x43, &PsiSiSatelliteDeliverySystemDescriptorHandler);
		PsiSiNitHandler.RegisterTransportDescriptorHandler(0x44, &PsiSiCableDeliverySystemDescriptorHandler);
		PsiSiNitHandler.RegisterTransportDescriptorHandler(0x5A, &PsiSiTerrestrialDeliverySystemDescriptorHandler);
		PsiSiNitHandler.RegisterTransportDescriptorHandler(0x79, &PsiSiS2SatelliteDeliverySystemDescriptorHandler);

		PsiSiTableHandlerRegistry.Init();
		PsiSiTableHandlerRegistry.RegisterHandler(0x0000, 0x00, &PsiSiPatHandler);
		PsiSiTableHandlerRegistry.RegisterHandler(0x0010, 0x40, &PsiSiNitHandler);

		PsiSiSectionBufPool.Init();
		PsiSiSectionBufPool.AllocateBuf(0x0000);
		PsiSiSectionBufPool.AllocateBuf(0x0010);

		return 0;
	}

	working_ts_info.packet_count = 0;
	working_ts_info.tei_packet_count = 0;
	working_ts_info.out_of_sync = 0;
	for (i = 0; i < 8192; ++i)
	{
		working_ts_info.working_pid_infos[i].packet_count = 0;
		working_ts_info.working_pid_infos[i].scrambled = 0;
		working_ts_info.working_pid_infos[i].layer_info = 1;
		working_ts_info.working_pid_infos[i].program_number = -1;
	}

	PsiSiPatHandler.Init();
	PsiSiPmtHandler.Init();
	PsiSiNitHandler.Init();
	PsiSiPatHandler.SetUpdateCallback(psi_si_pat_update_callback);
	PsiSiNitHandler.RegisterTransportDescriptorHandler(0x43, &PsiSiSatelliteDeliverySystemDescriptorHandler);
	PsiSiNitHandler.RegisterTransportDescriptorHandler(0x44, &PsiSiCableDeliverySystemDescriptorHandler);
	PsiSiNitHandler.RegisterTransportDescriptorHandler(0x5A, &PsiSiTerrestrialDeliverySystemDescriptorHandler);
	PsiSiNitHandler.RegisterTransportDescriptorHandler(0x79, &PsiSiS2SatelliteDeliverySystemDescriptorHandler);

	PsiSiTableHandlerRegistry.Init();
	PsiSiTableHandlerRegistry.RegisterHandler(0x0000, 0x00, &PsiSiPatHandler);
	PsiSiTableHandlerRegistry.RegisterHandler(0x0010, 0x40, &PsiSiNitHandler);

	PsiSiSectionBufPool.Init();
	PsiSiSectionBufPool.AllocateBuf(0x0000);
	PsiSiSectionBufPool.AllocateBuf(0x0010);

	TsBitrateCalculator.Init();

	return 0;
}


void CHldUtils::log_ts_packet_info(BYTE *ts_packet, int ts_packet_len)
{
	int pid;

	++working_ts_info.packet_count;
	if (ts_packet[1] & 0x80)	// Checks if TEI Set
	{
		++working_ts_info.tei_packet_count;
	}
	else
	{
		pid = ((ts_packet[1] & 0x1f) << 8) + ts_packet[2];
		++working_ts_info.working_pid_infos[pid].packet_count;
		if (ts_packet_len >= 204)
		{
			working_ts_info.working_pid_infos[pid].layer_info = (ts_packet[189] >> 4) & 0x0f;
		}
		if (pid != 0x1fff)	// Unless Null Packet
		{
			working_ts_info.working_pid_infos[pid].scrambled = !((ts_packet[3] & 0xc0) == 00);;
		}
	}
}


int CHldUtils::process_ts_packet(BYTE *ts_packet, int ts_packet_len)
{
	int pid;
	int adaptation_field_control;
	int adaptation_field_length;
	int ts_header_len;
	int continuity_counter;
	BYTE * ts_packet_payload;
	int ts_packet_payload_len;
	int pointer_field;
	int table_id;
	int section_length;
	BYTE *psi_si_section;
	int psi_si_section_len;
	CPsiSiTableHandler *pPsiSiTableHandler;

	pid = ((ts_packet[1] & 0x1f) << 8) + ts_packet[2];
	if (!PsiSiSectionBufPool.IsBufAllocated(pid))
	{
		return 0;
	}

	adaptation_field_control = (ts_packet[3] & 0x30) >> 4;
	if (adaptation_field_control == 0x02)	// Adaptation_field only, no payload
	{
		return 0;
	}

	continuity_counter = ts_packet[3] & 0x0f;
	if (adaptation_field_control == 0x03)
	{
		adaptation_field_length = ts_packet[4];
		if (adaptation_field_length >= MAX_TS_PACKET_HEADER_ADAPTATION_FIELD_LENGTH)
		{
			return -1;
		}
		ts_header_len = MIN_TS_PACKET_HEADER_LEN + 1 + adaptation_field_length; 
	}
	else
	{
		ts_header_len = MIN_TS_PACKET_HEADER_LEN;
	}
	ts_packet_payload = ts_packet + ts_header_len;
	ts_packet_payload_len = MIN_TS_PACKET_LEN - ts_header_len;

	if (ts_packet[1] & 0x40)	// payload_unit_start_indicator == 1
	{
		pointer_field = *ts_packet_payload;
		if (ts_packet_payload_len <= 1 + pointer_field)
		{
			return -2;
		}

		if (pointer_field != 0)
		{
			if (PsiSiSectionBufPool.IsBufActive(pid))
			{
				if (PsiSiSectionBufPool.AppendToBuf(pid, continuity_counter, ts_packet_payload + 1, pointer_field) == 0)
				{
					if (PsiSiSectionBufPool.IsSectionComplete(pid))
					{
						PsiSiSectionBufPool.GetBuf(pid, &psi_si_section, &psi_si_section_len);
						table_id = *psi_si_section;
						if ((pPsiSiTableHandler = PsiSiTableHandlerRegistry.GetHandler(pid, table_id)) != NULL)
						{
							pPsiSiTableHandler->ParseSection(psi_si_section, psi_si_section_len);
						}
					}
				}
			}
		}

		PsiSiSectionBufPool.DeactivateBuf(pid);
		ts_packet_payload += 1 + pointer_field;
		ts_packet_payload_len -= 1 + pointer_field;
		do
		{
			table_id = *ts_packet_payload;
			if (ts_packet_payload_len < MIN_TS_PACKET_HEADER_LEN)
			{
				if (PsiSiTableHandlerRegistry.GetHandler(pid, table_id) != NULL)
				{
					PsiSiSectionBufPool.FillBuf(pid, continuity_counter, ts_packet_payload, ts_packet_payload_len);
				}
				ts_packet_payload_len = 0;
			}
			else
			{
				section_length = ((ts_packet_payload[1] & 0x0f) << 8) + ts_packet_payload[2];
				psi_si_section_len = MIN_PSI_SI_SECTION_HEADER_LEN + section_length;
				if (psi_si_section_len > ts_packet_payload_len)
				{
					if (PsiSiTableHandlerRegistry.GetHandler(pid, table_id) != NULL)
					{
						PsiSiSectionBufPool.FillBuf(pid, continuity_counter, ts_packet_payload, ts_packet_payload_len);
					}
					ts_packet_payload_len = 0;
				}
				else
				{			
					if ((pPsiSiTableHandler = PsiSiTableHandlerRegistry.GetHandler(pid, table_id)) != NULL)
					{
						if (pPsiSiTableHandler->PreprocessSection(ts_packet_payload, psi_si_section_len) == 0)
						{
							pPsiSiTableHandler->ParseSection(ts_packet_payload, psi_si_section_len);
						}
					}
					PsiSiSectionBufPool.DeactivateBuf(pid);
					ts_packet_payload += psi_si_section_len;
					ts_packet_payload_len -= psi_si_section_len;
				}
			}
		} while (ts_packet_payload_len > 0);
	}
	else if (PsiSiSectionBufPool.IsBufActive(pid))	// payload_unit_start_indicator == 0
	{
		if (PsiSiSectionBufPool.AppendToBuf(pid, continuity_counter, ts_packet_payload, ts_packet_payload_len) == 0)
		{
			if (PsiSiSectionBufPool.IsSectionComplete(pid))
			{
				PsiSiSectionBufPool.GetBuf(pid, &psi_si_section, &psi_si_section_len);
				table_id = *psi_si_section;
				if ((pPsiSiTableHandler = PsiSiTableHandlerRegistry.GetHandler(pid, table_id)) != NULL)
				{
					pPsiSiTableHandler->ParseSection(psi_si_section, psi_si_section_len);
				}
				PsiSiSectionBufPool.DeactivateBuf(pid);
			}
			else
			{
				if (!PsiSiSectionBufPool.IsSectionHeaderChecked(pid) && PsiSiSectionBufPool.IsSectionHeaderComplete(pid))
				{
					PsiSiSectionBufPool.GetBuf(pid, &psi_si_section, &psi_si_section_len);
					table_id = *psi_si_section;
					if ((pPsiSiTableHandler = PsiSiTableHandlerRegistry.GetHandler(pid, table_id)) != NULL)
					{
						if (pPsiSiTableHandler->PreprocessSection(psi_si_section, psi_si_section_len) == 0)
						{
							PsiSiSectionBufPool.SetSectionHeaderChecked(pid);
						}
						else
						{
							PsiSiSectionBufPool.DeactivateBuf(pid);
						}
					}
					else
					{
						PsiSiSectionBufPool.DeactivateBuf(pid);
					}
				}
			}
		}
	}

	return 0;
}


int CHldUtils::is_ts_info_ready(void)
{
	if (TsBitrateCalculator.IsBitrateReady())
	{
		//return 1;
	}

	return 0;
}


void CHldUtils::conv_psi_si_stream_type_to_str(int stream_type, char *stream_type_str)
{
	if (stream_type == 0x01)
	{
		sprintf(stream_type_str, "video(%d)", stream_type);
	}
	else if (stream_type == 0x02)
	{
		sprintf(stream_type_str, "mpeg-2 video(%d)", stream_type);
	}
	else if (stream_type == 0x03)
	{
		sprintf(stream_type_str, "mpeg-1 audio(%d)", stream_type);
	}
	else if (stream_type == 0x04)
	{
		sprintf(stream_type_str, "mpeg-2 audio(%d)", stream_type);
	}
	else if (stream_type == 0x05)
	{
		sprintf(stream_type_str, "private data(%d)", stream_type);
	}
	else if (stream_type == 0x06)	
	{
		sprintf(stream_type_str, "private data(%d)", stream_type);
	}
	else if (stream_type == 0x0A)	
	{
		sprintf(stream_type_str, "13818-6(%d)", stream_type);
	}
	else if (stream_type == 0x0B)	
	{
		sprintf(stream_type_str, "13818-6(%d)", stream_type);
	}
	else if (stream_type == 0x0C)
	{
		sprintf(stream_type_str, "13818-6(%d)", stream_type);
	}
	else if (stream_type == 0x0D)	
	{
		sprintf(stream_type_str, "13818-6(%d)", stream_type);
	}
	else if (stream_type == 0x0E)
	{
		sprintf(stream_type_str, "13818-1(%d)", stream_type);
	}
	else if (stream_type == 0x0F)
	{
		sprintf(stream_type_str, "AAC audio(%d)", stream_type);	// "13818-7"
	}
	else if (stream_type == 0x10)
	{
		sprintf(stream_type_str, "14496-2(%d)", stream_type);
	}
	else if (stream_type == 0x11)	
	{
		sprintf(stream_type_str, "mpeg-4 audio(%d)", stream_type);	// "14496-3"
	}
	else if (stream_type == 0x12)
	{
		sprintf(stream_type_str, "14496-1(%d)", stream_type);
	}
	else if (stream_type == 0x13)	
	{
		sprintf(stream_type_str, "14496-1(%d)", stream_type);
	}
	else if (stream_type == 0x14)	
	{
		sprintf(stream_type_str, "13818-6(%d)", stream_type);
	}
	else if (stream_type >= 0x15 && stream_type <= 0x19)
	{
		sprintf(stream_type_str, "meta data(%d)", stream_type);
	}
	else if (stream_type == 0x1B)
	{
		sprintf(stream_type_str, "AVC/H.264 video(%d)", stream_type);
	}
	else if (stream_type == 0x7F)
	{
		sprintf(stream_type_str, "IPMP stream(%d)", stream_type);
	}
	else if (stream_type == 0x81)
	{
		sprintf(stream_type_str, "Dolby-AC3 audio(%d)", stream_type);
	}
	else
	{
		sprintf(stream_type_str, "");
	}
}


void CHldUtils::fill_psi_si_pid_desc(int pid, char *pid_desc)
{
	if (working_ts_info.working_pid_infos[pid].program_number >= 0)
	{
		sprintf(pid_desc, "Program - 0x%X", working_ts_info.working_pid_infos[pid].program_number);
	}
	else if (working_ts_info.working_pid_infos[pid].program_number == -2 && pid != 0x0010)
	{
		sprintf(pid_desc, "PMT");
	}
	else if (pid == 0x0000)
	{
		sprintf(pid_desc, "PAT");
	}
	else if (pid == 0x0001)
	{
		sprintf(pid_desc, "CAT");
	}
	else if (pid == 0x0010)
	{
		sprintf(pid_desc, "NIT");
	}
	else if (pid == 0x0011)
	{
		sprintf(pid_desc, "SDT");
	}
	else if (pid == 0x0014)
	{
		sprintf(pid_desc, "TDT/TOT");
	}
	else if (pid == 0x1FFF)
	{
		sprintf(pid_desc, "null packet");
	}
	else
	{
		sprintf(pid_desc, "");
	}
}


int CHldUtils::output_ts_info_to_file(void)
{
	char szDBpath[MAX_PATH];
	int nSlot;
	FILE *fp;
	int found;
	int i;
	int j;
	int k;

	nSlot = 9;
#if defined(WIN32)
	sprintf(szDBpath, "%s\\%s_%d", g_szCurDir, PID_DB_PATH, nSlot);
#else
#ifdef STANDALONE
	sprintf(szDBpath, "/ramfs/%s_%d", PID_DB_PATH, nSlot);
#else
	sprintf(szDBpath, "%s/%s_%d", g_szCurDir, PID_DB_PATH, nSlot);
#endif
#endif
	fp = fopen(szDBpath, "wt");
	if (fp == NULL)
	{
		return -1;
	}

	for (i = 0; i < pTsInfo->num_pid_info; ++i)
	{
		found = 0;
		for (j = 0; j < pTsInfo->num_pgm_info; ++j)
		{
			if (pTsInfo->pgm_info[j].program_map_PID == pTsInfo->pid_info[i].PID)
			{
				found = 2;
				break;
			}
			for (k = 0; k < pTsInfo->pgm_info[j].num_elmt_info; ++k)
			{
				if (pTsInfo->pgm_info[j].elmt_info[k].elementary_PID == pTsInfo->pid_info[i].PID)
				{
					found = 1;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}
		if (found == 2)
		{
			fprintf(fp, "%d,%s,%d,,,%d,%d\n", 
				pTsInfo->pid_info[i].PID, 
				pTsInfo->pid_info[i].szPidDesc,
				pTsInfo->pgm_info[j].program_number,
				pTsInfo->pid_info[i].bit_rate,
				pTsInfo->pid_info[i].layer_info
				);
		}
		else if (found == 1)
		{
			fprintf(fp, "%d,%d,%d,%d,%s,%d,%d\n", 
				pTsInfo->pid_info[i].PID, 
				pTsInfo->pgm_info[j].program_map_PID,
				pTsInfo->pgm_info[j].program_number,
				pTsInfo->pgm_info[j].PCR_PID,
				pTsInfo->pgm_info[j].elmt_info[k].szStreamType,
				pTsInfo->pid_info[i].bit_rate,
				pTsInfo->pid_info[i].layer_info
				);
		}
		else
		{
			fprintf(fp, "%d,%s,,,,%d,%d\n", 
				pTsInfo->pid_info[i].PID, 
				pTsInfo->pid_info[i].szPidDesc,
				pTsInfo->pid_info[i].bit_rate,
				pTsInfo->pid_info[i].layer_info
				);
		}
	}

	fclose(fp);

	return 0;
}


int CHldUtils::pack_ts_info(struct _TSPH_TS_INFO **ppTsInfo)
{
	int actual_network_id;
	int transport_stream_id;
	BYTE *descriptor;
	int num_of_pmt_pids;
	int *pmt_pids;
	int num_of_programs;
	int *program_numbers;
	int num_of_elements;
	int *elementary_pids;
	int stream_type;
	int i;
	int j;
	int num_of_pids;

	if ((pTsInfo = (struct _TSPH_TS_INFO *)malloc(sizeof(struct _TSPH_TS_INFO))) == NULL)
	{
		return -1;
	}

	pTsInfo->Flags = 0;
	if (PsiSiPatHandler.GetTransportStreamId(&transport_stream_id) == 0)
	{
		pTsInfo->Flags |= TSPH_TS_FLAG_PAT_IN_TS;
	}
	else
	{
		transport_stream_id = -1;
	}

	if (PsiSiNitHandler.GetActualNetworkId(&actual_network_id) == 0)
	{
		if ((pTsInfo->nit_info = (struct _TSPH_TS_NIT_INFO *)malloc(sizeof(struct _TSPH_TS_NIT_INFO))) == NULL)
		{
			free(pTsInfo);
			pTsInfo = NULL;

			return -2;
		}

		pTsInfo->nit_info->network_PID = 0x0010;
		pTsInfo->nit_info->NIT_Flag = 0;
		if (transport_stream_id >= 0)
		{
			// satellite_delivery_system_descriptor
			if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x43, &descriptor) == 0)
			{
				pTsInfo->nit_info->NIT_Flag |= TSPH_TS_NIT_FLAG_SATELLITE_DELIVERY_SYSTEM_DESC_IN_TS;
			}
			// cable_delivery_system_descriptor
			if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x44, &descriptor) == 0)
			{
				pTsInfo->nit_info->NIT_Flag |= TSPH_TS_NIT_FLAG_CABLE_DELIVERY_SYSTEM_DESC_IN_TS;
			}
			// terrestrial_delivery_system_descriptor
			if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x5A, &descriptor) == 0)
			{
				pTsInfo->nit_info->NIT_Flag |= TSPH_TS_NIT_FLAG_TERRESTRIAL_DELIVERY_SYSTEM_DESC_IN_TS;
			}
			// S2_satellite_delivery_system_descriptor
			if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x79, &descriptor) == 0)
			{
				pTsInfo->nit_info->NIT_Flag |= TSPH_TS_NIT_FLAG_S2_SATELLITE_DELIVERY_SYSTEM_DESC_IN_TS;
			}
		}
		pTsInfo->Flags |= TSPH_TS_FLAG_NIT_IN_TS;
	}
	else
	{
		pTsInfo->nit_info = NULL;
	}
	if (working_ts_info.nit_only)
	{
		pTsInfo->num_pgm_info = 0;
		pTsInfo->pgm_info = NULL;
		pTsInfo->num_pid_info = 0;
		pTsInfo->pid_info = NULL;
		*ppTsInfo = pTsInfo;

		return 0;
	}

	TsBitrateCalculator.UpdateTsBitrate();

	PsiSiPmtHandler.GetProgramNumbers(&num_of_programs, &program_numbers);
	pTsInfo->num_pgm_info = num_of_programs;
	if (num_of_programs > 0)
	{
		if ((pTsInfo->pgm_info = (struct _TSPH_TS_PGM_INFO *)malloc(num_of_programs * sizeof(struct _TSPH_TS_PGM_INFO))) == NULL)
		{
			if (pTsInfo->nit_info != NULL)
			{
				free(pTsInfo->nit_info);
				pTsInfo->nit_info = NULL;
			}
			free(pTsInfo);
			pTsInfo = NULL;

			return -3;
		}
		for (i = 0; i < num_of_programs; ++i)
		{
			PsiSiPatHandler.GetPid(-1, program_numbers[i], (int *)&pTsInfo->pgm_info[i].program_map_PID);
			pTsInfo->pgm_info[i].program_number = program_numbers[i];
			PsiSiPmtHandler.GetPcrPid(program_numbers[i], (int *)&pTsInfo->pgm_info[i].PCR_PID);
			pTsInfo->pgm_info[i].bit_rate = 0;
			PsiSiPmtHandler.GetElementaryPids(program_numbers[i], &num_of_elements, &elementary_pids);
			pTsInfo->pgm_info[i].num_elmt_info = num_of_elements;
			if (num_of_elements > 0)
			{
				if ((pTsInfo->pgm_info[i].elmt_info = (struct _TSPH_TS_PGM_ELMT_INFO *)malloc(num_of_elements * 
					sizeof(struct _TSPH_TS_PGM_ELMT_INFO))) == NULL)
				{
					for (j = i - 1; j >= 0; --j)
					{
						free(pTsInfo->pgm_info[j].elmt_info);
						pTsInfo->pgm_info[j].elmt_info = NULL;
					}
					free(pTsInfo->pgm_info);
					pTsInfo->pgm_info = NULL;
					if (pTsInfo->nit_info != NULL)
					{
						free(pTsInfo->nit_info);
						pTsInfo->nit_info = NULL;
					}
					free(pTsInfo);
					pTsInfo = NULL;

					return -4;
				}
				
				for (j = 0; j < num_of_elements; ++j)
				{
					pTsInfo->pgm_info[i].elmt_info[j].elementary_PID = elementary_pids[j];
					PsiSiPmtHandler.GetStreamType(program_numbers[i], elementary_pids[j], &stream_type);
					conv_psi_si_stream_type_to_str(stream_type, pTsInfo->pgm_info[i].elmt_info[j].szStreamType);
					TsBitrateCalculator.GetBitrate(elementary_pids[j], (int *)&pTsInfo->pgm_info[i].elmt_info[j].bit_rate);
					pTsInfo->pgm_info[i].bit_rate += pTsInfo->pgm_info[i].elmt_info[j].bit_rate;
					
					working_ts_info.working_pid_infos[elementary_pids[j]].program_number = program_numbers[i];
				}
			}
			else
			{
				pTsInfo->pgm_info[i].elmt_info = NULL;
			}
		}
		pTsInfo->Flags |= TSPH_TS_FLAG_PMT_IN_TS;
	}
	else
	{
		pTsInfo->pgm_info = NULL;
	}
	
	PsiSiPatHandler.GetPmtPids(-1, &num_of_pmt_pids, &pmt_pids);
	for (i = 0; i < num_of_pmt_pids; ++i)
	{
		working_ts_info.working_pid_infos[pmt_pids[i]].program_number = -2;
	}
	if (num_of_programs > 0 && num_of_programs == num_of_pmt_pids)
	{
		pTsInfo->Flags |= TSPH_TS_FLAG_ALL_PMT_IN_TS;
	}

	num_of_pids = 0;
	for (i = 0; i < 8192; ++i)
	{
		if (working_ts_info.working_pid_infos[i].packet_count > 0 || working_ts_info.working_pid_infos[i].program_number >= 0 ||
			 working_ts_info.working_pid_infos[i].program_number == -2)
		{
			++num_of_pids;
		}
	}
	pTsInfo->num_pid_info = num_of_pids;
	if (num_of_pids > 0)
	{
		if ((pTsInfo->pid_info = (struct _TSPH_TS_PID_INFO *)malloc(num_of_pids * sizeof(struct _TSPH_TS_PID_INFO))) == NULL)
		{
			if (num_of_programs > 0)
			{
				for (i = 0; i < num_of_programs; ++i)
				{
					free(pTsInfo->pgm_info[j].elmt_info);
					pTsInfo->pgm_info[j].elmt_info = NULL;
				}
				free(pTsInfo->pgm_info);
				pTsInfo->pgm_info = NULL;
			}
			if (pTsInfo->nit_info != NULL)
			{
				free(pTsInfo->nit_info);
				pTsInfo->nit_info = NULL;
			}
			free(pTsInfo);
			pTsInfo = NULL;

			return -5;
		}
		j = 0;
		for (i = 0; i < 8192; ++i)
		{
			if (working_ts_info.working_pid_infos[i].packet_count > 0 || working_ts_info.working_pid_infos[i].program_number >= 0 || 
				working_ts_info.working_pid_infos[i].program_number == -2)
			{
				pTsInfo->pid_info[j].PID = i;
				fill_psi_si_pid_desc(i, pTsInfo->pid_info[j].szPidDesc);
				TsBitrateCalculator.GetBitrate(i, (int *)&pTsInfo->pid_info[j].bit_rate);
				pTsInfo->pid_info[j].layer_info = working_ts_info.working_pid_infos[i].layer_info;
				pTsInfo->pid_info[j].scrambled = working_ts_info.working_pid_infos[i].scrambled;
				++j;
			}
		}
	}
	else
	{
		pTsInfo->pid_info = NULL;
	}

	pTsInfo->packet_size = working_ts_info.packet_size;
	pTsInfo->packet_count = working_ts_info.packet_count;
	pTsInfo->TEI_packet_count = working_ts_info.tei_packet_count;
	if (TsBitrateCalculator.IsPcrDeltaAvaliable())
	{
		pTsInfo->Flags |= TSPH_TS_FLAG_PCR_IN_TS;
	}
	if (working_ts_info.out_of_sync)
	{
		pTsInfo->Flags |= TSPH_TS_FLAG_OUT_OF_SYNC_IN_TS;
	}

	//output_ts_info_to_file();
	*ppTsInfo = pTsInfo;

	return 0;
}


int CHldUtils::RunTs_Parser(BYTE *bData, DWORD dwSize, int default_bitrate, int nitFlag, struct _TSPH_TS_INFO **ppTsInfo)
{
	BYTE *cur_ts_packet;
	int b_ts_info_ready;
	int sync_offset;
	int ts_packet_size;
	DWORD remaining_size;

	working_ts_info.nit_only = nitFlag;
	cur_ts_packet = bData;
	b_ts_info_ready = 0;
	TsBitrateCalculator.SetDefaultTsBitrate(default_bitrate);
	do
	{
		init_psi_si_table_parsing_context();
		remaining_size = dwSize - (cur_ts_packet - bData);
		sync_offset = TL_SyncLockFunction((char *)cur_ts_packet, remaining_size, &ts_packet_size, 
			remaining_size - TS_SYNC_CHECK_LOOKAHEAD_LEN, NUM_OF_CONSECUTIVE_TS_PACKETS_IN_SYNC);
		if (sync_offset < 0)
		{
			break;
		}
		working_ts_info.packet_size = ts_packet_size;
		for (cur_ts_packet += sync_offset; cur_ts_packet + ts_packet_size <= bData + dwSize; cur_ts_packet += ts_packet_size)
		{
			if (cur_ts_packet[0] != 0x47)
			{
				break;
			}
			if (!working_ts_info.nit_only)
			{
				log_ts_packet_info(cur_ts_packet, ts_packet_size);
				TsBitrateCalculator.ProcessTsPacket(cur_ts_packet, ts_packet_size);
			}
			if (!(cur_ts_packet[1] & 0x80) && ((cur_ts_packet[1] & 0x1f) << 8) + cur_ts_packet[2] != 0x1FFF)	
				// Unless TEI Set or Null Packet
 			{
				process_ts_packet(cur_ts_packet, ts_packet_size);
			}
			if (is_ts_info_ready())
			{
				b_ts_info_ready = 1;
				break;
			}
		}
	} while (cur_ts_packet + ts_packet_size <= bData + dwSize && !b_ts_info_ready);
	
	return pack_ts_info(ppTsInfo);
}


int CHldUtils::FreeTsParserMemory(void)
{
	if (pTsInfo != NULL)
	{
		if (pTsInfo->nit_info != NULL)
		{
			free(pTsInfo->nit_info);
			pTsInfo->nit_info = NULL;
		}
		if (pTsInfo->pgm_info != NULL)
		{
			free(pTsInfo->pgm_info);
			pTsInfo->pgm_info = NULL;
		}
		if (pTsInfo->pid_info != NULL)
		{
			free(pTsInfo->pid_info);
			pTsInfo->pid_info = NULL;
		}
		free(pTsInfo);

		pTsInfo = NULL;
	}

	return 0;
}


int CHldUtils::GetPmtInfo(int *pNumPgmInfo, struct _TSPH_TS_PGM_INFO **ppPgmInfo)
{
	if (pTsInfo != NULL)
	{
		*pNumPgmInfo = pTsInfo->num_pgm_info;
		*ppPgmInfo = pTsInfo->pgm_info;
	}
	else
	{
		*pNumPgmInfo = 0;
		*ppPgmInfo = NULL;
	}

	return 0;
}


int CHldUtils::GetPidInfo(int *pNumPidInfo, struct _TSPH_TS_PID_INFO **ppPidInfo)
{
	if (pTsInfo != NULL)
	{
		*pNumPidInfo = pTsInfo->num_pid_info;
		*ppPidInfo = pTsInfo->pid_info;
	}
	else
	{
		*pNumPidInfo = 0;
		*ppPidInfo = NULL;
	}

	return 0;
}

void CHldUtils::GetSatelliteDescriptorInfo2(int *desc_tag, int *freq, int *rolloff, int *mod_sys, int *mod_typ, int *symbol, int *code)
{
	int actual_network_id;
	int transport_stream_id;
	BYTE *descriptor;

	if (pTsInfo != NULL && pTsInfo->Flags & TSPH_TS_FLAG_NIT_IN_TS && 
		pTsInfo->nit_info->NIT_Flag & TSPH_TS_NIT_FLAG_SATELLITE_DELIVERY_SYSTEM_DESC_IN_TS)
	{
		if (PsiSiNitHandler.GetActualNetworkId(&actual_network_id) < 0)
		{
			return;
		}
		if (PsiSiPatHandler.GetTransportStreamId(&transport_stream_id) < 0)
		{
			return;
		}
		if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x43, &descriptor) < 0)
		{
			return;
		}
		if (PsiSiSatelliteDeliverySystemDescriptorHandler.Parse(descriptor) < 0)
		{
			return;
		}

		*desc_tag = 1;
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetFrequency(freq);
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetRollOff(rolloff);
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetModulationSystem(mod_sys);
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetModulationType(mod_typ);
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetSymbolRate(symbol);
		PsiSiSatelliteDeliverySystemDescriptorHandler.GetFecInner(code);
	}
	else
	{
		*desc_tag = 0;
	}
}

void CHldUtils::GetCableDescriptorInfo2(int *desc_tag, int *freq, int *mod_typ, int *symbol, int *code)
{
	int actual_network_id;
	int transport_stream_id;
	BYTE *descriptor;

	if (pTsInfo != NULL && pTsInfo->Flags & TSPH_TS_FLAG_NIT_IN_TS && 
		pTsInfo->nit_info->NIT_Flag & TSPH_TS_NIT_FLAG_CABLE_DELIVERY_SYSTEM_DESC_IN_TS)
	{
		if (PsiSiNitHandler.GetActualNetworkId(&actual_network_id) < 0)
		{
			return;
		}
		if (PsiSiPatHandler.GetTransportStreamId(&transport_stream_id) < 0)
		{
			return;
		}
		if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x44, &descriptor) < 0)
		{
			return;
		}
		if (PsiSiCableDeliverySystemDescriptorHandler.Parse(descriptor) < 0)
		{
			return;
		}

		*desc_tag = 1;
		PsiSiCableDeliverySystemDescriptorHandler.GetFrequency(freq);
		PsiSiCableDeliverySystemDescriptorHandler.GetModulation(mod_typ);
		PsiSiCableDeliverySystemDescriptorHandler.GetSymbolRate(symbol);
		PsiSiCableDeliverySystemDescriptorHandler.GetFecInner(code);
	}
	else
	{
		*desc_tag = 0;
	}
}

void CHldUtils::GetTerrestrialDescriptorInfo2(int *desc_tag, unsigned int *freq, int *bw, int *time_slicing, int *mpe_fec, 
	int *constellation, int *code, int *guard, int *txmod)
{
	int actual_network_id;
	int transport_stream_id;
	BYTE *descriptor;
	int priority;
	int hierarchy_information;

	if (pTsInfo != NULL && pTsInfo->Flags & TSPH_TS_FLAG_NIT_IN_TS && 
		pTsInfo->nit_info->NIT_Flag & TSPH_TS_NIT_FLAG_TERRESTRIAL_DELIVERY_SYSTEM_DESC_IN_TS)
	{
		if (PsiSiNitHandler.GetActualNetworkId(&actual_network_id) < 0)
		{
			return;
		}
		if (PsiSiPatHandler.GetTransportStreamId(&transport_stream_id) < 0)
		{
			return;
		}
		if (PsiSiNitHandler.GetTransportDescriptor(0x40, actual_network_id, transport_stream_id, 0x5A, &descriptor) < 0)
		{
			return;
		}
		if (PsiSiTerrestrialDeliverySystemDescriptorHandler.Parse(descriptor) < 0)
		{
			return;
		}

		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetPriority(&priority);
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetHierarchyInformation(&hierarchy_information);

		*desc_tag = 1;
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetCentreFrequency((int *)freq);
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetBandwidth(bw);
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetTimeSlicingIndicator(time_slicing);
		if (hierarchy_information > 3)
		{
			*time_slicing += 3;
		}
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetMpeFecIndicator(mpe_fec);
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetConstellation(constellation);
		if (hierarchy_information == 0)
		{
			PsiSiTerrestrialDeliverySystemDescriptorHandler.GetCodeRateHpStream(code);
		}
		else
		{
			if (priority == 1)
			{
				PsiSiTerrestrialDeliverySystemDescriptorHandler.GetCodeRateHpStream(code);
			}
			else
			{
				PsiSiTerrestrialDeliverySystemDescriptorHandler.GetCodeRateLpStream(code);
			}
		}
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetGuardInterval(guard);
		PsiSiTerrestrialDeliverySystemDescriptorHandler.GetTransmissionMode(txmod);
	}
	else
	{
		*desc_tag = 0;
	}
}
//CKIM A 20120828 }
