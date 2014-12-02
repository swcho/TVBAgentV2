#include <malloc.h>
#include <string.h>
#include <math.h>

#include "dvbt2_system.h"

#include "dvbt2.h"



// Conform to table 8 (a133 doc)                     

// will be obsoleted
#if 0
int t2mi_LF_length[6][7] = 
                   //  1/128   1/32  1/16  1/256   1/8   19/128   1/4 
{ /* FFT size 32K */ {  68,   66,    64,    64,    60,      60,    -1/*NA*/}, 
  /* FFT size 16K */ {  138,  135,   131,   129,   123,     121,   111}, 
  /* FFT size 08K */ {  276,  270,   262,   259,   247,     242,   223}, 
  /* FFT size 04K */ {  -1,   540,   524,   519,   495,     485,   446}, 
  /* FFT size 02K */ {  -1,   1081,  1049,  1038,  991,     970,   892}, 
  /* FFT size 01K */ {  -1,   -1,    2098,  2076,  1982,    1941,  1784}}; 

#endif

// page 115, table 65 (a122)
// time is in millisecond
double t2mi_elementary_period_t[6] = 
{ /* Bandwidth 1.7 */ (71./131.)/1000., 
  /* Bandwidth 5   */ (7./40.)/1000., 
  /* Bandwidth 6   */ (7./48.)/1000., 
  /* Bandwidth 7   */ (1./8.)/1000., 
  /* Bandwidth 8   */ (7./64.)/1000., 
  /* Bandwidth 10  */ (7./80.)/1000.}; 

// page 115, table 66 (a122)
double t2mi_Tu_base[6] = 
{ /* FFT size 32K */ 32768, 
  /* FFT size 16K */ 16384, 
  /* FFT size 08K */ 8192, 
  /* FFT size 04K */ 4096, 
  /* FFT size 02K */ 2048 , 
  /* FFT size 01K */ 1024}; 



// for 8 Mhz bandwidth
// time in millisecond
double t2mi_Tu[6] = 
{ /* FFT size 32K */ 3.584, 
  /* FFT size 16K */ 1.792, 
  /* FFT size 08K */ 0.896, 
  /* FFT size 04K */ 0.448, 
  /* FFT size 02K */ 0.224, 
  /* FFT size 01K */ 0.112}; 

unsigned char t2mi_np2[6] = 
{ /* FFT size 32K */ 1, 
  /* FFT size 16K */ 1, 
  /* FFT size 08K */ 2, 
  /* FFT size 04K */ 4, 
  /* FFT size 02K */ 8, 
  /* FFT size 01K */ 16}; 

// conform to table 9 (133)
// number of data cell for each P2 symbol
unsigned int t2mi_cp2_cell[6][2] = 
                   //  SISO   MISO
{ /* FFT size 32K */ {  22432,  17612},
  /* FFT size 16K */ {  8944,   8814},
  /* FFT size 08K */ {  4472,   4398},
  /* FFT size 04K */ {  2236,  2198},
  /* FFT size 02K */ {  1118,  1098},
  /* FFT size 01K */ {  558,   546}};

// conform to 6.1.3 (133)
static int t2mi_l1pre_num_cell = 1840;

// number of data cell for each data symbol
int t2mi_cdata_cell_normal[6][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   24886, -1,    26022, -1,   26592,  26836, 26812}, 
  /* FFT size 16K */ {  12418, 12436, 12988, 13002, 13272, 13288, 13416, 13406}, 
  /* FFT size 08K */ {  6208,  6214,  6494,  6498,  6634,  -1,    6698,  6698}, 
  /* FFT size 04K */ {  3084,  3092,  3228,  3234,  3298,  -1,    3328,  -1}, 
  /* FFT size 02K */ {  1522,  1532,  1596,  1602,  1632,  -1,    1646,  -1}, 
  /* FFT size 01K */ {  764,   768,   798,   804,   818,   -1,    -1,    -1}}; 

int t2mi_cdata_cell_extended[3][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   25412, -1,    26572, -1,   27152,  27404, 27376}, 
  /* FFT size 16K */ {  12678, 12698, 13262, 13276, 13552, 13568, 13698, 13688}, 
  /* FFT size 08K */ {  6296,  6298,  6584,  6588,  6728,  -1,    6788,  6788}}; 


int t2mi_N_FC_normal[6][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   22720, -1,    24992, -1,    26128,  -1,    -1}, 
  /* FFT size 16K */ {  9088, 11360, 11360, 12496, 12496, 13064, 13064, -1}, 
  /* FFT size 08K */ {  4544, 5680,  5980,  6248,  6248,  -1,    6532,  -1}, 
  /* FFT size 04K */ {  2272, 2840,  2840,  3124,  3124,  -1,    3266,  -1}, 
  /* FFT size 02K */ {  1136, 1420,  1420,  1562,  1562,  -1,    1632,  -1}, 
  /* FFT size 01K */ {  568,  710,   710,   780,   780,   -1,    -1,    -1}}; 

int t2mi_N_FC_extended[3][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   23200, -1,    25520, -1,    26680, -1,    -1}, 
  /* FFT size 16K */ {  9280, 11600, 11600, 12760, 12760, 13340, 13340, -1}, 
  /* FFT size 08K */ {  4608, 5760,  5760,  6336,  6336,  -1,    6624,  -1}}; 


int t2mi_C_FC_normal[6][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   20952, -1,     22649, -1,   23603,  -1,    -1}, 
  /* FFT size 16K */ {  6437, 10476,  7845,  11324, 8709,  11801, 11170, -1}, 
  /* FFT size 08K */ {  3218,  5238,  3922,  5662,  4354,  -1,    5585,  -1}, 
  /* FFT size 04K */ {  1609,  2619,  1961,  2831,  2177,  -1,    2792,  -1}, 
  /* FFT size 02K */ {  804,   1309,  980,   1415,  1088,  -1,    1396,  -1}, 
  /* FFT size 01K */ {  402,   654,   490,   707,   544,   -1,    -1,    -1}}; 

int t2mi_C_FC_extended[3][8]=
                   //  PP1    PP2    PP3    PP4   PP5    PP6     PP7    PP8
{ /* FFT size 32K */ {  -1,   21395, -1,    23172, -1,    24102, -1,    -1}, 
  /* FFT size 16K */ {  6573, 10697, 8011,  11563, 8893,  12051, 11406, -1}, 
  /* FFT size 08K */ {  3264, 5312,  3978,  5742,  4416,  -1,    5664,  -1}}; 




unsigned int t2mi_closing_frame_2_pilot[6][7]=
                                  //  1/128         1/32                                1/16                    19/256            1/8                        19/128                    1/4 
{ /* FFT size 32K */ {  0,          1 << 6               ,          1 << 4                ,  1 << 4                ,  1 << 2              ,  1 << 2               , 0         }, 
  /* FFT size 16K */ {  0,          (1<<6) | (1<<7),          (1<<4) | (1<<5), (1<<4) | (1<<5) , (1<<2) | (1<<3), (1<<2) | (1<<3), 1 << 1}, 
  /* FFT size 08K */ {  0,          (1<<7)               ,          (1<<4) | (1<<5), (1<<4) | (1<<5) , (1<<2) | (1<<3), (1<<2) | (1<<3), 1 << 1}, 
  /* FFT size 04K */ {  0,          (1<<7)               ,          (1<<4) | (1<<5),  0                         , (1<<2) | (1<<3),  0                       , 1 << 1}, 
  /* FFT size 02K */ {  0,          (1<<7)               ,          (1<<4) | (1<<5),  0                         , (1<<2) | (1<<3),  0                       , 1 << 1}, 
  /* FFT size 01K */ {  0,          0                         ,          (1<<4) | (1<<5),  0                         , (1<<2) | (1<<3),  0                       , 1 << 1}}; 





// conform to table 11(section 6.2, 122)
// number of data cell for each FECFRAME
unsigned int t2mi_fecframe_cells[2][4] = 
                    // QAM256 QAM64  QAM16  QPSK    
{ /* 64,800 (64K) */ {  8100,  10800, 16200, 32400}, 
/*   16,200 (16K) */ {  2025,  2700,  4050,  8100 }}; 



///////////////////////////////////////////////////////////////////////////////
// refer to 8.3.1 (122)

// return -1
// return duration in millisecond 
// not available 
double t2frame_duration(int fft_size, double gif, int NUM_DATA_SYMBOLS, int bandwidth){
	
	double Tp1 = 0.224;
	double gif_value; 
	int Np2;
	int Lf;
	double Tu;

	int guard_interval_index = 0;
	int fft_size_index = 0; 
	int bandwidth_index = 0;

        if (bandwidth == DVBT2_BW_1_7)
        {
                bandwidth_index = 0;
        }else if (bandwidth == DVBT2_BW_5)
	{
		bandwidth_index = 1;
        }else if (bandwidth == DVBT2_BW_6)
	{
		bandwidth_index = 2;
        }else if (bandwidth == DVBT2_BW_7)
	{
		bandwidth_index = 3;
        }else if (bandwidth == DVBT2_BW_8)
	{
		bandwidth_index = 4;
	}else
		bandwidth_index = 4;


	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}



	if (gif == DVBT2_GUARD_INTERVAL_1_128)
	{
		guard_interval_index = 0;
		gif_value = 1.0/128;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_32)
	{
		guard_interval_index = 1;
		gif_value = 1.0/32;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_16)
	{
		guard_interval_index = 2;
		gif_value = 1.0/16;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_256)
	{
		guard_interval_index = 3;
		gif_value = 19.0/256;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_8)
	{
		gif_value = 1.0/8;
		guard_interval_index = 4;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_128)
	{
		gif_value = 19.0/128;
		guard_interval_index = 5;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_4)
	{
		gif_value = 1.0/4;
		guard_interval_index = 6;
	}

        /*  will be obsoleted
	int Lf_max = t2mi_LF_length[fft_size_index][guard_interval_index];
        */

	Np2 = t2mi_np2[fft_size_index];

/* will be obsoleted
	if (NUM_DATA_SYMBOLS + Np2 > Lf_max )
		return - 1; // number of data symbol must be less

*/
	Lf = Np2 + NUM_DATA_SYMBOLS;

	//double Tu = t2mi_Tu[fft_size_index];
	Tu = t2mi_Tu_base[fft_size_index]*t2mi_elementary_period_t[bandwidth_index];
	Tp1 = 2048 * t2mi_elementary_period_t[bandwidth_index];
	

	return Lf*(Tu*(1.0 + gif_value)) + Tp1;
}

int t2frame_l1pre_num_cell()
{	
	return t2mi_l1pre_num_cell;
}

// will be revised
// the maximun number of data symbol is subjected to constraint that maximun of a T-2 frame is 250us
#if 0
int t2frame_check_valid_NUM_DATA_SYMBOL(int fft_size_index, double gif, int NUM_DATA_SYMBOLS)
{


	int num_data_symbol_min = 0;
	int Np2 = t2mi_np2[fft_size_index];

	if (fft_size_index == DVBT2_FFT_SIZE_32K)
		num_data_symbol_min = 3;
	else 
		num_data_symbol_min = 7;

	int guard_interval_index = 0;
	if (gif == DVBT2_GUARD_INTERVAL_1_128)
		guard_interval_index = 0;
	else if (gif == DVBT2_GUARD_INTERVAL_1_32)
		guard_interval_index = 1;
	else if (gif == DVBT2_GUARD_INTERVAL_1_16)
		guard_interval_index = 2;
	else if (gif == DVBT2_GUARD_INTERVAL_19_256)
		guard_interval_index = 3;
	else if (gif == DVBT2_GUARD_INTERVAL_1_8)
		guard_interval_index = 4;
	else if (gif == DVBT2_GUARD_INTERVAL_19_128)
		guard_interval_index = 5;
	else if (gif == DVBT2_GUARD_INTERVAL_1_4)
		guard_interval_index = 6;

	int Lf_max = t2mi_LF_length[fft_size_index][guard_interval_index];

	if ((NUM_DATA_SYMBOLS + Np2 > Lf_max) || (NUM_DATA_SYMBOLS < num_data_symbol_min))
		return 0;
	
	return 1;
}
#endif



int t2frame_data_symbol_num_cell(int fft_size, int pilot, int bw_ext){
	int pilot_index;
	int fft_size_index; 

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;		
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	pilot_index = pilot;
	if (bw_ext)
		return t2mi_cdata_cell_extended[fft_size_index][pilot_index];
	else
	    return t2mi_cdata_cell_normal[fft_size_index][pilot_index];
}

int t2frame_data_symbol_closing_frame_num_cell(int fft_size, int pilot, int bw_ext){
	int pilot_index;
	int fft_size_index; 

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;		
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	pilot_index = pilot;
	if (bw_ext)
		return t2mi_C_FC_extended[fft_size_index][pilot_index];
	else
	    return t2mi_C_FC_normal[fft_size_index][pilot_index];
}




int t2frame_p2_symbol_num_cell(int fft_size, int s1){
	int mode_index;
	int fft_size_index; 

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	mode_index = s1;
	return t2mi_cp2_cell[fft_size_index][mode_index];

}


int t2frame_p2_num_symbol(int fft_size){
	
	int fft_size_index; 

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	
	return t2mi_np2[fft_size_index];

}



int t2frame_fecframe_num_cell(int fec_type, int modulation){
	
	int modulation_index;
	int fec_type_index; 
	
	if (fec_type == DVBT2_PLP_FEC_TYPE_64K_LDPC)
	{
		fec_type_index = 0;
	}else if(fec_type == DVBT2_PLP_FEC_TYPE_16K_LDPC){
		fec_type_index = 1;
	}

	if (modulation == DVBT2_PLP_MOD_256QAM){
		modulation_index = 0;
	}else if(modulation == DVBT2_PLP_MOD_64QAM){
		modulation_index = 1;
	}else if (modulation == DVBT2_PLP_MOD_16QAM){
		modulation_index = 2;
	}else if (modulation == DVBT2_PLP_MOD_QPSK){
		modulation_index = 3;
	}

	return t2mi_fecframe_cells[fec_type_index][modulation_index];
}

// this function will be obsoleted in next version

#if 0
int t2frame_Lf(int fft_size, int gif)
{
    
	int fft_size_index = 0; 
	int guard_interval_index = 0;

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}


	if (gif == DVBT2_GUARD_INTERVAL_1_128)
	{
		guard_interval_index = 0;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_32)
	{
		guard_interval_index = 1;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_16)
	{
		guard_interval_index = 2;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_256)
	{
		guard_interval_index = 3;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_8)
	{
		guard_interval_index = 4;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_128)
	{
		guard_interval_index = 5;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_4)
	{
		guard_interval_index = 6;
	}

	return t2mi_LF_length[fft_size_index][guard_interval_index];

        return 0;
}
#endif



//************************************
// Method:    t2frame_kbch_size
// FullName:  t2frame_kbch_size
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int plp_fec_type
// Parameter: int plp_cod
//************************************
int t2frame_kbch_size( int plp_fec_type, int plp_cod )
{

	int K_bch;
	if ( plp_fec_type == 0x001 )
	{
		if ( plp_cod == 0x000 )			K_bch = 32208;
		else if ( plp_cod == 0x001 )	K_bch = 38688;
		else if ( plp_cod == 0x002 )	K_bch = 43040;
		else if ( plp_cod == 0x003 )	K_bch = 48408;
		else if ( plp_cod == 0x004 )	K_bch = 51648;
		else if ( plp_cod == 0x005 )	K_bch = 53840;
	}
	else
	{
		if ( plp_cod == 0x000 )			K_bch = 7032;
		else if ( plp_cod == 0x001 )	K_bch = 9552;
		else if ( plp_cod == 0x002 )	K_bch = 10632;
		else if ( plp_cod == 0x003 )	K_bch = 11712;
		else if ( plp_cod == 0x004 )	K_bch = 12432;
		else if ( plp_cod == 0x005 )	K_bch = 13152;
	}

	return K_bch;

}

int t2frame_num_data_symbol_min( int fft_size, int gif )
{
	int fft_size_index; 
	int addition_min; 

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
		addition_min = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
		addition_min = 7;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
		addition_min = 7;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
		addition_min = 7;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
		addition_min = 7;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
		addition_min = 7;
	}

	return (t2mi_np2[fft_size_index] + addition_min);

}


int t2frame_l1pos_dynamic_current(int num_plp, int num_aux)
{

	int length_in_bits = (71 + num_plp*48 + 8 /*reserver */ + num_aux*48);
	return length_in_bits;
}


int t2frame_l1pos_conflen(int num_plp, int num_aux, int num_rf, int fft_size)
{

	int s2 = fft_size << 1;
	int length_in_bits = 35 + 35*num_rf + ((s2 & 0x01) == 1 ? 34 : 0) + 89*num_plp + 32 + 32*num_aux;
	return length_in_bits;

}





/*

// KSLee added 

//2012/1/3
int t2frame_l1_signal_cell(int fft_size, int siso_miso)
{
	int fft_size_index = 0;
	int l1_signal_cell = 0;
	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;		
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}

	l1_signal_cell = t2mi_np2[fft_size_index] * t2mi_cp2_cell[fft_size_index][siso_miso];
	return l1_signal_cell;
}


*/
int t2frame_is_closing_symbol(int fft_size, int gif, int pilot)
{
	int fft_size_index = 0; 
	int guard_interval_index = 0;
	int _pilot;

	if (fft_size == DVBT2_FFT_SIZE_32K || fft_size == DVBT2_FFT_SIZE_32K_E)	
	{
		fft_size_index = 0;
	} else if (fft_size == DVBT2_FFT_SIZE_16K)
	{
		fft_size_index = 1;
	} else if (fft_size == DVBT2_FFT_SIZE_8K || fft_size == DVBT2_FFT_SIZE_8K_E)
	{
		fft_size_index = 2;
	} else if (fft_size == DVBT2_FFT_SIZE_4K) {
		fft_size_index = 3;
	} else if (fft_size == DVBT2_FFT_SIZE_2K){
		fft_size_index = 4;
	} else if (fft_size == DVBT2_FFT_SIZE_1K) {
		fft_size_index = 5;
	}


	if (gif == DVBT2_GUARD_INTERVAL_1_128)
	{
		guard_interval_index = 0;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_32)
	{
		guard_interval_index = 1;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_16)
	{
		guard_interval_index = 2;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_256)
	{
		guard_interval_index = 3;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_8)
	{
		guard_interval_index = 4;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_19_128)
	{
		guard_interval_index = 5;
	}
	else if (gif == DVBT2_GUARD_INTERVAL_1_4)
	{
		guard_interval_index = 6;
	}

	_pilot = t2mi_closing_frame_2_pilot[fft_size_index][guard_interval_index];
	if (((1 << (pilot + 1)) & _pilot))
		return 1;
	return 0;

}


// calculate the l1_pos size in cells
int TL_Calculate_L1_Post_Size(int L1_MOD, int FFT_SIZE, int L1_POST_INFO_SIZE)
{
double K_post_ex_pad, N_post_FEC_Block, K_L1_PADDING, K_post, K_sig, N_L1_mult, N_punc_temp, N_bch_parity, R_elf_16k_LDPC_1_2, N_post_temp, N_post, N_MOD_per_Block, N_MOD_Total;

        double N_ldpc = 16200;
        double K_bch = 7032;
		double N_P2;
        double N_mod = 1;
        if ( L1_MOD == 0x000 )		N_mod = 1;
        else if ( L1_MOD == 0x001 )	N_mod = 2;
        else if ( L1_MOD == 0x002 )	N_mod = 4;
        else if ( L1_MOD == 0x003 )	N_mod = 6;

        N_P2 = 16;//==FFT 1k
        if ( FFT_SIZE == 3 )						N_P2 = 16;
        else if ( FFT_SIZE == 0 )					N_P2 = 8;
        else if ( FFT_SIZE == 2 )					N_P2 = 4;
        else if ( FFT_SIZE == 1 || FFT_SIZE == 6 )	N_P2 = 2;
        else if ( FFT_SIZE == 4 )					N_P2 = 1;
        else if ( FFT_SIZE == 5 || FFT_SIZE == 7 )	N_P2 = 1;

        K_post_ex_pad = L1_POST_INFO_SIZE + 32;
        N_post_FEC_Block = (int)ceil(K_post_ex_pad / K_bch);
        K_L1_PADDING = (int)ceil(K_post_ex_pad / N_post_FEC_Block) * N_post_FEC_Block - K_post_ex_pad;
        K_post = (int)(K_post_ex_pad + K_L1_PADDING);
        K_sig  = (int)(K_post / N_post_FEC_Block);
        if (N_P2 == 1)
        {
                N_L1_mult = 2 * N_mod;
        }
        else
        {
                N_L1_mult = N_P2 * N_mod;
        }

        N_punc_temp  = (int)floor((6./5.) * (K_bch - K_sig));
        if ( N_punc_temp < (N_L1_mult - 1) )
        {
                N_punc_temp = (N_L1_mult - 1);
        }

        R_elf_16k_LDPC_1_2 = 4./9.;
        N_bch_parity = 168.;
        N_post_temp = (int)(K_sig + N_bch_parity + N_ldpc * (1 - R_elf_16k_LDPC_1_2) - N_punc_temp);
        if (N_P2 == 1)
        {
                N_post = (int)ceil(N_post_temp / (2*N_mod)) * (2*N_mod);
        }
        else
        {
                N_post = (int)ceil(N_post_temp / (N_mod * N_P2)) * (N_mod * N_P2);
        }

        N_MOD_per_Block =  (int)(N_post / N_mod);
        N_MOD_Total = (int)(N_MOD_per_Block * N_post_FEC_Block);

        return (int)N_MOD_Total;
}


// Number of active cells of a T2-frame

int t2frame_num_ctot(int fft_size, int pilot, int bw_ext, int guard_interval, int num_data_symbol)
{
    int c_tot = 0;
    int d_data  = t2frame_data_symbol_num_cell(fft_size, pilot, bw_ext); // number of cell for each data symbol

    int l_p2 = t2frame_p2_num_symbol(fft_size);
    int c_p2 = t2frame_p2_symbol_num_cell(fft_size, 0 /* SISO mode */);
    int total_np2_cell = c_p2*l_p2;

    if (t2frame_is_closing_symbol(fft_size, guard_interval, pilot))
    {
            int l_normal = num_data_symbol -1;
            int total_data_cell = d_data*l_normal;
            total_data_cell += t2frame_data_symbol_closing_frame_num_cell(fft_size, pilot, bw_ext);
            c_tot = total_np2_cell + total_data_cell;
    }
    else
    {
            int total_data_cell = num_data_symbol;
            c_tot = total_np2_cell + total_data_cell;
    }
    return c_tot;
}







