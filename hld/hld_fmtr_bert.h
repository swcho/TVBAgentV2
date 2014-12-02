
#ifndef __HLD_FMTER_BERT_H__
#define __HLD_FMTER_BERT_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#endif

#include	"hld_ctl_hwbuf.h"
#include	"hld_fs_rdwr.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"


class CHldFmtrBert
{
private:
	CHldGVar	*__Sta__;
	CHldFsRdWr	*__FIf__;
	CHldBdLog	*__HLog__;


	//FIXED - BERT
	int p_15_init;
	int p_23_init;
	unsigned char p_15[15];
	unsigned char p_23[23];

	unsigned char	*TL_pbPRBSBuffer_14_15;
	unsigned char	*TL_pbPRBSBuffer_18_23;
	unsigned char	*TL_pbPRBSBuffer;
//	int TL_TestPacketType;
//	FILE *TL_fp_14_15;
//	FILE *TL_fp_18_23;
	int TL_ErrorCount;
	int TL_TotalCount;
	double TL_BER;
	int TL_ErrorInsert;
	int TL_ErrorInsert2;

	FILE *fp1, *fp2;


public:
	CHldFmtrBert(void);
	virtual ~CHldFmtrBert();

	void	SetCommonMethod_2(
		CHldGVar	*__sta__,
		CHldFsRdWr	*__fIf__,
		CHldBdLog	*__hLog__);
	void	InitBertCariable_OnRecStart(void);
	void	InitBertVariable_OnPlayStart(void);
	void TL_Make_PRBS_2_15(unsigned char *prbs_15, unsigned char *p_15, int payload_size);
	void TL_Make_PRBS_2_23(unsigned char *prbs_23, unsigned char *p_23, int payload_size);
	int Set_ErrInjection_Param(long error_lost, long error_lost_packet,
		long error_bits, long error_bits_packet, long error_bits_count,
		long error_bytes, long error_bytes_packet, long error_bytes_count);
	int SetMidulator_Bert_Measure(long modulator_type, long packet_type, long bert_pid);

	int	RdPrbsDta(int _pkt_typ);
	int 	TL_ProcessCaptureSubBlock(void);
	double	GetBer(void);

	//2012/3/16 BERT 187
	int Play_BERT187_EmptyFile_UntilStopContPlay__HasEndlessWhile(void);
};


#endif

