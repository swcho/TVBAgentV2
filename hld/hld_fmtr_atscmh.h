
#ifndef __HLD_FMTER__ATSCMH_H__
#define __HLD_FMTER__ATSCMH_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"

#if defined(WIN32)
#else
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef long long __int64;
#endif


class CHldFmtrAtscMH
{
private:
	int	my_hld_id;
	void	*my_hld;

	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;

	char debug_string[100];

public:
	int m_MHE_packet_PID;
	int m_MHE_packet_index;
	int m_MHE_FIC_version;
	int m_MHE_TPC_protocol_version;
	int m_MHE_TPC_information_validity;

public:
	CHldFmtrAtscMH(int _my_id, void *_hld);
	virtual ~CHldFmtrAtscMH();


	void	SetCommonMethod_81(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);

	void InitMHEInfo(int packet_pid);
	void InitMHELoopPath(int packet_pid, int tsio_direction);
	int TL_DecodeTPCInfo(
		unsigned char buf[],
		unsigned char *subframenum,
		unsigned char *slotnum,
		unsigned char *paradeid, 
		unsigned char *fic_version,
		unsigned char *tpc_protocol_version, 
		unsigned char *tpc_info);
	int TL_GetStartPos(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize, int from_slot_0, unsigned char *tpc_info);
	int	GetAtscMh_PktInfo(char *szFile, int iType);
	int	RunAtscMhParser(char *szFile, char *szResult);



};


#endif

