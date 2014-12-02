//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_INTERP_H_
#define	__TLV_INTERP_H_

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CUtilInterp
{
private:
	int	dbg_notice;
	int	dbg_warning;
	int	dbg_noisy;


public:
	CUtilInterp(void);
	~CUtilInterp();

	void Cubic_Interp(double *x, double *y, int count, double *result, int result_len);
	void DoInterp(int target, int *sample, int sample_count, int interp_len, double *result, int result_len, int interpolation_type);

};

#ifdef __cplusplus
}
#endif



#endif
