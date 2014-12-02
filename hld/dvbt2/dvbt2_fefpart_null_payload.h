#ifndef _DVBT2_FEFPART_NULL_
#define  _DVBT2_FEFPART_NULL_

#ifdef __cplusplus
extern "C"
{
#endif

// conform to DVB Document TS 102 773 (V1.3.1) @ clause 5.2.9
// T2-MI payload (type T2 FEF part: Null packet)
typedef struct T2MI_FEFpart_Null{
	unsigned char fef_idx; // 8bits
	unsigned char rfu;  // 9 bits 
	unsigned char s1_field; // 3 bits
	unsigned char s2_field;// 4 bits
}T2MI_FEFpart_Null_t;


void FEFpartNull_init(T2MI_FEFpart_Null_t *p_fef_null, int _val_s1, int _val_s2);
void FEFpartNull_gen(T2MI_FEFpart_Null_t *p_fef_null, unsigned short fef_idx);
int FEFpartNull_to_buffer(T2MI_FEFpart_Null_t *p_fef_null, unsigned char *buff);

#ifdef __cplusplus
}
#endif




#endif
