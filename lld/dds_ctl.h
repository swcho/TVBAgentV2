//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_DDS_CTL_H_
#define	__TLV_DDS_CTL_H_

#include 	"Chip_util.h"
#include	"dac_ctl.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
#else
typedef long long __int64;
#endif
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CDdsCtl	:	public	CDacCtl
{
private:

//2011/5/4 AD9852 MAX CLK
#ifdef WIN32
	int ad9852_Max_clk;
#endif

	void RdRegistry_FreqStep(int modulation_mode, int	*_step, unsigned long *_sht);
	void RdRegistry_SymClkAdj(int modulation_mode, int *_adj);
	void CalcClkCntlCnst_(int modulation_mode, int _en_adj, int _sym_freq, int *_cntl, double *_cnst);
	int CalcOutFreq_(int modulation_mode, int *_sym_freq, double _cnst, __int64 *_out_freq);

	int CntlTrf178_NewBd_Case_1(unsigned long dwfreq, int step_freq);		//	593/597v2 or 590s higher rev3
	int CntlTrf178_NewBd_Case_2(unsigned long dwfreq, int step_freq);	//	tvb590s rev2s
	int CntlTrf178_NewBd_Case_3(unsigned long dwfreq, int step_freq);	//	tvb590s rev1
	int CntlTrf178_OldBd_Case_1(int modulation_mode, unsigned long dwfreq, int step_freq);

	void CntlAD9852SymClk_NewBd_IndirectAccess(__int64 outfreq_long, int nCLK_CONTROL);
	void CntlAD9852SymClk_NewBd_DirectAccess(__int64 outfreq_long, int nCLK_CONTROL);
	int CntlAD9852SymClk_OldBd_DirectAccess(__int64 outfreq_long, int nCLK_CONTROL);
	int TAD142_SET_AD9852(unsigned char	offset, DWORD data32, unsigned char bExternal);
	//2012/7/9 HMC833
	int Cntl_HMC833(unsigned long dwfreq, int step_freq);

public:
	CDdsCtl(void);
	~CDdsCtl();

	int Config_TRF178_PLL2(int R_Reg, int N_Reg, int F_Reg);
	int Reset_TRF178(int nBoardId, int nModulatorType, int nIF);

	int CntlClkGen_RFout_Carrier(int modulation_mode, unsigned long dwfreq);	//2150MHz support - int -> unsigned long
	int CntlClkGen_AD9852_Symclock(int modulation_mode, int symbol_freq_or_bandwidth);
	int _stdcall TSPL_SET_SYMBOL_CLOCK(long modulator_type, long symbol_clock);
	//2013/5/22 TVB599
	int CntlClkGen_HMC_1033_988_Symclock(int modulation_mode, int symbol_freq_or_bandwidth);
	int Set_HMC988_register(int nDivideRatio);
	int Set_HMC1033_register(int nRF_Divider, int nInteger, int nFractional);
//2011/5/4 AD9852 MAX CLK
#ifdef WIN32
	int _stdcall TSPL_SET_AD9852_MAX_CLOCK(long value);
#endif

};

#ifdef __cplusplus
}
#endif



#endif
