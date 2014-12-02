#include <malloc.h>
#include <string.h>
#include <math.h>
#include "dvbc2_system.h"


// Conform to table 8 (a133 doc)                     
int c2mi_LF_length[6][7] = 
                   //  1/128   1/32  1/16  1/256   1/8   19/128   1/4 
{ /* FFT size 32K */ {  68,   66,    64,    64,    60,      60,    -1/*NA*/}, 
  /* FFT size 16K */ {  138,  135,   131,   129,   123,     121,   111}, 
  /* FFT size 08K */ {  276,  270,   262,   259,   247,     242,   223}, 
  /* FFT size 04K */ {  -1,   540,   524,   519,   495,     485,   446}, 
  /* FFT size 02K */ {  -1,   1081,  1049,  1038,  991,     970,   892}, 
  /* FFT size 01K */ {  -1,   -1,    2098,  2076,  1982,    1941,  1784}}; 

// page 115, table 65 (a122)
// time is in millisecond
double c2mi_elementary_period_t[6] = 
{ /* Bandwidth 1.7 */ (71./131.)/1000., 
  /* Bandwidth 5   */ (7./40.)/1000., 
  /* Bandwidth 6   */ (7./48.)/1000., 
  /* Bandwidth 7   */ (1./8.)/1000., 
  /* Bandwidth 8   */ (7./64.)/1000., 
  /* Bandwidth 10  */ (7./80.)/1000.}; 

// page 115, table 66 (a122)
double c2mi_Tu_base[6] = 
{ /* FFT size 32K */ 32768, 
  /* FFT size 16K */ 16384, 
  /* FFT size 08K */ 8192, 
  /* FFT size 04K */ 4096, 
  /* FFT size 02K */ 2048 , 
  /* FFT size 01K */ 1024}; 



// for 8 Mhz bandwidth
// time in millisecond
double c2mi_Tu[6] = 
{ /* FFT size 32K */ 3.584, 
  /* FFT size 16K */ 1.792, 
  /* FFT size 08K */ 0.896, 
  /* FFT size 04K */ 0.448, 
  /* FFT size 02K */ 0.224, 
  /* FFT size 01K */ 0.112}; 

unsigned char c2mi_np2[6] = 
{ /* FFT size 32K */ 1, 
  /* FFT size 16K */ 1, 
  /* FFT size 08K */ 2, 
  /* FFT size 04K */ 4, 
  /* FFT size 02K */ 8, 
  /* FFT size 01K */ 16}; 

// conform to table 9 (133)
// number of data cell for each P2 symbol
unsigned int c2mi_cp2_cell[6][2] = 
                   //  SISO   MISO
{ /* FFT size 32K */ {  22432,  17612},
  /* FFT size 16K */ {  8944,   8814},
  /* FFT size 08K */ {  4472,   4398},
  /* FFT size 04K */ {  2236,  2198},
  /* FFT size 02K */ {  1118,  1098},
  /* FFT size 01K */ {  558,   546}};

// number of data cell for each data symbol
unsigned int c2mi_cdata_cell_normal[6][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   24886, -1,    26022, -1,   26592,  26836, 26812}, 
  /* FFT size 16K */ {  12418, 12436, 12988, 13002, 13272, 13288, 13416, 13406}, 
  /* FFT size 08K */ {  6208,  6214,  6494,  6498,  6634,  -1,    6698,  6698}, 
  /* FFT size 04K */ {  3084,  3092,  3228,  3234,  3298,  -1,    3328,  -1}, 
  /* FFT size 02K */ {  1522,  1532,  1596,  1602,  1632,  -1,    1646,  -1}, 
  /* FFT size 01K */ {  764,   768,   798,   804,   818,   -1,    -1,    -1}}; 

unsigned int c2mi_cdata_cell_extended[3][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   25412, -1,    26572, -1,   27152,  27404, 27376}, 
  /* FFT size 16K */ {  12678, 12698, 13262, 13276, 13552, 13568, 13698, 13688}, 
  /* FFT size 08K */ {  6296,  6298,  6584,  6588,  6728,  -1,    6788,  6788}}; 



// conform to table 11(section 6.2, 122)
// number of data cell for each FECFRAME
//2011/5/20 DVB-C2 MULTI PLP
//unsigned int c2mi_fecframe_cells[2][4] = 
unsigned int c2mi_fecframe_cells[2][6] = 
                    // QAM256 QAM64  QAM16  QPSK    1024QAM  4096QAM
{ /* 64,800 (64K) */ {  8100,  10800, 16200, 32400,  6480,   5400}, 
/*   16,200 (16K) */ {  2025,  2700,  4050,  8100,   1620,   1350}}; 



///////////////////////////////////////////////////////////////////////////////
// refer to 8.3.1 (122)

// return -1
// return duration in millisecond 
// not available 
#if 0
double c2frame_duration(int fft_size, double gif, int NUM_DATA_SYMBOLS, int bandwidth){
	
	double Tp1 = 0.224;
	double gif_value; 

	int guard_interval_index = 0;
	int fft_size_index = 0; 
	int bandwidth_index = 0;


	if (bandwidth == TLV_BW_5)
	{
		bandwidth_index = 1;
	}else if (bandwidth == TLV_BW_6)
	{
		bandwidth_index = 2;
	}else if (bandwidth == TLV_BW_7)
	{
		bandwidth_index = 3;
	}else if (bandwidth == TLV_BW_8)
	{
		bandwidth_index = 4;
	}else
		bandwidth_index = 4;


	if (fft_size == DVBC2_FFT_SIZE_32K || fft_size == DVBC2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBC2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBC2_FFT_SIZE_8K || fft_size == DVBC2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBC2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBC2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBC2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}



	if (gif == DVBC2_GUARD_INTERVAL_1_128)
	{
		guard_interval_index = 0;
		gif_value = 1.0/128;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_32)
	{
		guard_interval_index = 1;
		gif_value = 1.0/32;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_16)
	{
		guard_interval_index = 2;
		gif_value = 1.0/16;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_19_256)
	{
		guard_interval_index = 3;
		gif_value = 19.0/256;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_8)
	{
		gif_value = 1.0/8;
		guard_interval_index = 4;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_19_128)
	{
		gif_value = 19.0/128;
		guard_interval_index = 5;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_4)
	{
		gif_value = 1.0/4;
		guard_interval_index = 6;
	}

	int Lf_max = c2mi_LF_length[fft_size_index][guard_interval_index];
	int Np2 = c2mi_np2[fft_size_index];
	if (NUM_DATA_SYMBOLS + Np2 > Lf_max )
		return - 1; // number of data symbol must be less

	int Lf = Np2 + NUM_DATA_SYMBOLS;

	//double Tu = c2mi_Tu[fft_size_index];
	double Tu = c2mi_Tu_base[fft_size_index]*c2mi_elementary_period_t[bandwidth_index];

	

	return Lf*(Tu*(1.0 + gif_value)) + Tp1;
}
#endif

bool c2frame_check_valid_NUM_DATA_SYMBOL(int fft_size_index, double gif, int NUM_DATA_SYMBOLS){

	int num_data_symbol_min = 0;
	int Np2 = c2mi_np2[fft_size_index];

	if (fft_size_index == DVBC2_FFT_SIZE_32K)
		num_data_symbol_min = 3;
	else 
		num_data_symbol_min = 7;

	int guard_interval_index;
	if (gif == DVBC2_GUARD_INTERVAL_1_128)
		guard_interval_index = 0;
	else if (gif == DVBC2_GUARD_INTERVAL_1_32)
		guard_interval_index = 1;
	else if (gif == DVBC2_GUARD_INTERVAL_1_16)
		guard_interval_index = 2;
	else if (gif == DVBC2_GUARD_INTERVAL_19_256)
		guard_interval_index = 3;
	else if (gif == DVBC2_GUARD_INTERVAL_1_8)
		guard_interval_index = 4;
	else if (gif == DVBC2_GUARD_INTERVAL_19_128)
		guard_interval_index = 5;
	else if (gif == DVBC2_GUARD_INTERVAL_1_4)
		guard_interval_index = 6;

	int Lf_max = c2mi_LF_length[fft_size_index][guard_interval_index];

	if ((NUM_DATA_SYMBOLS + Np2 > Lf_max) || (NUM_DATA_SYMBOLS < num_data_symbol_min))
		return false;
	
	return true;
}



int c2frame_data_symbol_num_cell(int fft_size, int pilot){
	int pilot_index;
	int fft_size_index; 

	int is_fft_extend = 0;

	if (fft_size == DVBC2_FFT_SIZE_32K_E || fft_size == DVBC2_FFT_SIZE_8K_E /* where is 16K extended */ )
	{
		is_fft_extend = 1;
	}
	if (fft_size == DVBC2_FFT_SIZE_32K || fft_size == DVBC2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;		
	} else if (fft_size == DVBC2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBC2_FFT_SIZE_8K || fft_size == DVBC2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBC2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBC2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBC2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	pilot_index = pilot;
	if (is_fft_extend)
		return c2mi_cdata_cell_extended[fft_size_index][pilot_index];
	else
	    return c2mi_cdata_cell_normal[fft_size_index][pilot_index];
}

int c2frame_p2_symbol_num_cell(int fft_size, int s1){
	int mode_index;
	int fft_size_index; 

	if (fft_size == DVBC2_FFT_SIZE_32K || fft_size == DVBC2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBC2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBC2_FFT_SIZE_8K || fft_size == DVBC2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBC2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBC2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBC2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	mode_index = s1;
	return c2mi_cp2_cell[fft_size_index][mode_index];

}

int c2frame_fecframe_num_cell(int fec_type, int modulation){
	
	int modulation_index;
	int fec_type_index; 
	
	if (fec_type == DVBC2_PLP_FEC_TYPE_64K_LDPC)
	{
		fec_type_index = 0;
	}else if(fec_type == DVBC2_PLP_FEC_TYPE_16K_LDPC){
		fec_type_index = 1;
	}

	if (modulation == DVBC2_PLP_MOD_256QAM){
		modulation_index = 0;
	}else if(modulation == DVBC2_PLP_MOD_64QAM){
		modulation_index = 1;
	}else if (modulation == DVBC2_PLP_MOD_16QAM){
		modulation_index = 2;
	}else if (modulation == DVBC2_PLP_MOD_QPSK){
		modulation_index = 3;
	}
	//2011/5/20 DVB-C2 MULTI-PLP ==>>
	else if (modulation == DVBC2_PLP_MOD_1024QAM){
		modulation_index = 4;
	}else if (modulation == DVBC2_PLP_MOD_4096QAM){
		modulation_index = 5;
	}
	//<<==

	return c2mi_fecframe_cells[fec_type_index][modulation_index];
}

int c2frame_num_data_symbol_max(int fft_size, int gif)
{
	int fft_size_index = 0; 
	int guard_interval_index = 0;

	if (fft_size == DVBC2_FFT_SIZE_32K || fft_size == DVBC2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBC2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBC2_FFT_SIZE_8K || fft_size == DVBC2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBC2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBC2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBC2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}


	if (gif == DVBC2_GUARD_INTERVAL_1_128)
	{
		guard_interval_index = 0;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_32)
	{
		guard_interval_index = 1;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_16)
	{
		guard_interval_index = 2;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_19_256)
	{
		guard_interval_index = 3;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_8)
	{
		guard_interval_index = 4;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_19_128)
	{
		guard_interval_index = 5;
	}
	else if (gif == DVBC2_GUARD_INTERVAL_1_4)
	{
		guard_interval_index = 6;
	}

	return c2mi_LF_length[fft_size_index][guard_interval_index];

}

//************************************
// Method:    c2frame_kbch_size
// FullName:  c2frame_kbch_size
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int plp_fec_type
// Parameter: int plp_cod
//************************************
int c2frame_kbch_size( int plp_fec_type, int plp_cod )
{

	int K_bch;
	/* BCH Uncoded Block */
	if ( plp_fec_type == 0x001 )
	{
		if ( plp_cod == 0x000 )			K_bch = 32208;
		else if ( plp_cod == 0x001 )	K_bch = 43040;
		else if ( plp_cod == 0x002 )	K_bch = 48408;
		else if ( plp_cod == 0x003 )	K_bch = 51648;
		else if ( plp_cod == 0x004 )	K_bch = 53840;
		else if ( plp_cod == 0x005 )	K_bch = 58192;
	}
	else
	{
		if ( plp_cod == 0x000 )			K_bch = 7032;
		else if ( plp_cod == 0x001 )	K_bch = 10632;
		else if ( plp_cod == 0x002 )	K_bch = 11712;
		else if ( plp_cod == 0x003 )	K_bch = 12432;
		else if ( plp_cod == 0x004 )	K_bch = 13152;
		else if ( plp_cod == 0x005 )	K_bch = 14232;
	}

	return K_bch;

}

int c2frame_num_data_symbol_min( int fft_size, int gif )
{
	int fft_size_index; 
	int guard_interval_index;
	int addition_min; 

	if (fft_size == DVBC2_FFT_SIZE_32K || fft_size == DVBC2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
		addition_min = 3;
	} else if (fft_size == DVBC2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
		addition_min = 7;
	} else if (fft_size == DVBC2_FFT_SIZE_8K || fft_size == DVBC2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
		addition_min = 7;
	} else if (fft_size == DVBC2_FFT_SIZE_4K) {
		fft_size_index = 3;
		addition_min = 7;
	} else if (fft_size == DVBC2_FFT_SIZE_2K){
		fft_size_index = 4;
		addition_min = 7;
	} else if (fft_size == DVBC2_FFT_SIZE_1K) {
		fft_size_index = 5;
		addition_min = 7;
	}

	return c2mi_np2[fft_size_index] + addition_min;

}


//2011/5/20 DVB-C2 MULTI PLP ===================================================>>

double c2frame_duration(int l1_ti_mode, double gif, int NUM_DATA_SYMBOLS){
	
	double gif_value; 
	double Tf;
	int Lp, Ldata;

	int guard_interval_index;

	if (gif == 0)
	{
		gif_value = 1.0/128.0;
	}
	else if (gif == 1)
	{
		gif_value = 1.0/64.0;
	}

	if(l1_ti_mode == 0 || l1_ti_mode == 1)
		Lp = 1;
	else if(l1_ti_mode == 2)
		Lp = 4;
	else
		Lp = 8;

  
	double Tu = 0.448;
	Ldata = NUM_DATA_SYMBOLS;

	//return 2.3; //42.0*3.584*(1.0 + 1.0/128) + 0.224;
	Tf = (double)(Lp + Ldata) * (Tu * (1.0 + gif_value)); 
	return Tf;
}
//<<==============================================================================

