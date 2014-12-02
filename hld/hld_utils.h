
#ifndef __HLD_UTILS_H__
#define __HLD_UTILS_H__

#if defined(WIN32)
#else
#define _FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#endif

#include	"local_def.h"

#include	"hld_lldcaller.h"
#include	"hld_gvar.h"
#include	"hld_bd_log.h"


#if defined(WIN32)
#define GetVLFilePointer(hFile, lpPositionHigh) (*lpPositionHigh = 0, SetFilePointer(hFile, 0, lpPositionHigh, FILE_CURRENT))
#else
#endif

//2012/2/14 NIT
//Satellite delivery system descriptor DVB-S/S2
struct SATELLITE_Delivery_Descriptor_
{
	int i_descriptor_flag;
	int i_frequeny;			//32bit, 4bit BCD, the decimal point occurs after the third character (e.g. 011.75725 GHz)
	int i_orbital_position;	//16bit, not used
	int i_west_east_flag;	//1bit, not used
	int i_polarization;		//2bit, not used
	int i_roll_off;			//2bit, 0 : 0.35, 1 : 0.25, 2 : 0.20
	int i_modulation_system;//1bit, 0 : DVB-S, 1 : DVB-S2	
	int i_modulation;		//2bit, 0 : not defined, 1 : QPSK , 2 : 8PSK , 3 : 16-QAM , 00100 ~ 11111 : reserved
	int i_symbolrate;		//28bit, 4bit BCD, Msymbol/s, the decimal point occurs after the third character (e.g. 027.4500).
	int i_coderate;			//4bit, 0: not defined, 1 : 1/2 , 2 : 2/3 , 3 : 3/4 , 4 : 5/6 , 5 : 7/8 , 6 : 8/9 , 7 : 3/5, 8 : 4/5, 9 : 9/10, 15 : No conv , 10 ~ 14 : reserved
};
//Cable delivery system descriptor DVB-C
struct CABLE_Delivery_Descriptor_
{
	int i_descriptor_flag;
	int i_frequeny;		//32bit, 4bit BCD, the decimal point occurs after the fourth character (e.g. 0312.0000 MHz)
	int i_fec_outer;	//4bit, not used.
	int i_modulation;	//8bit, 0 : not defined, 1 : 16QAM , 2 : 32QAM , 3 : 64QAM , 4 : 128QAM , 5: 256QAM , 0x06 ~ 0xFF : reserved
	int i_symbolrate;	//28bit, 4bit BCD, Msymbol/s, the decimal point occurs after the third character (e.g. 027.4500).
	int i_coderate;		//4bit, 0: not defined, 1 : 1/2 , 2 : 2/3 , 3 : 3/4 , 4 : 5/6 , 5 : 7/8 , 6 : 8/9 , 7 : 3/5, 8 : 4/5, 9 : 9/10, 15 : No conv , 10 ~ 14 : reserved
};
//Terrestrial delivery system descriptor DVB-T/H
struct TERRESTRIAL_Delivery_Descriptor_
{
	int i_descriptor_flag;
	unsigned int ui_frequency;
	int i_bandwith;
	int i_priority;
	int i_time_slicing_indicator;
	int i_mpe_fec_indicator;
	int i_constellation;
	int i_hierarchy_information;
	int i_code_rate_hp_stream;
	int i_code_rate_lp_stream;
	int i_guard_interval;
	int i_transmission_mode;
};

//=================================================================

class CHldUtils	:	public	Hlldcaller
{
private:
#if defined(WIN32)
	unsigned long	_prev_msec;
#else
	long long int	_prev_msec;
#endif

	//2012/2/15 NIT
	struct SATELLITE_Delivery_Descriptor_	st_Satellite;
	struct CABLE_Delivery_Descriptor_		st_Cable;
	struct TERRESTRIAL_Delivery_Descriptor_ st_terrestrial;

public:
	char g_szCurDir[MAX_PATH];

	int TL_TestPacketType;
	int TL_TestPacketPid;
	FILE *TL_fp_14_15;
	FILE *TL_fp_18_23;

									
	int 	TSSyncLock;		//	cap-dta sync lock. sync is ok if TRUE.

	
	int g_FrameDrop;
	int g_PacketCount;
	int	TL_nSyncStatus;
	int TL_gPacketSize;	//	one of 188/192/204/208

public:
	CHldUtils();
	virtual ~CHldUtils();

public:
	void	SetCommonMethod_80(CHldGVar	*__sta__, CHldBdLog	*__hLog__);
	int CheckExtention(char *szSource, char *szCharSet);
#if defined(WIN32)
	unsigned long	_msec_(void);
	unsigned long	_past_msec_(int _init);
#else
	long long int	_msec_(void);
#endif

	int Get_MPE_Packet_PID(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize);
	int TL_Calculate_Bitrate(BYTE *bData, DWORD dwSize, int iSyncStartPos, int iTSize, int *mhe_pkt_pid, int *mhe_pkt_ind);
	int SyncPosAssume188(char * buf, int max_len);
	int	TL_SyncLockFunction(char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step);
	//2011/8/5 ISDB-S ASI
	int	TL_SyncLockFunction_ISDBS(unsigned char * szBuf, int nlen, int *iSize, int nlen_srch, int nlen_step);
	
	int		TL_SearchSync(int dwDMABufSize);

	int InitParsePID(void);
	int InitPIDTable(void);
	int ParsePATsection(unsigned char *dbptr, int sec_len);
	int ParsePMTsection(unsigned char *dbptr, int sec_len);
//2012/5/8 NIT
	int ParseNITsection(unsigned char *dbptr, int sec_len);
	int si_decode(unsigned char *dbptr, int nSizeBuff);	// pointer to section start
	int RunTs_Parser(int nSlot, char *szFile, int default_bitrate, int nitFlag);

	int mpegts_resync(/*HANDLE pb*/FILE *pb);
	int read_packet(/*HANDLE pb*/FILE *pb, unsigned char *buf, int raw_packet_size);
	int FEOF(HANDLE hFile);
	int	TL_BERT_SET_DATA(unsigned char *pBuf);

	int	*_Ptr_VarTstPktTyp(void);
	int _VarTstPktTyp(void);

	//2012/9/28 bert fixed
	int	*_Ptr_VarTstPktPid(void);
	int _VarTstPktPid(void);

	//2012/5/9 NIT
	void ParseDescriptor(unsigned char *dbptr, int descTag);
	void InitNIT_Descriptor_variables();
	void GetSatelliteDescriptorInfo(int *desc_tag, int *freq, int *rolloff, int *mod_sys, int *mod_typ, int *symbol, int *code);
	void GetCableDescriptorInfo(int *desc_tag, int *freq, int *mod_typ, int *symbol, int *code);
	void GetTerrestrialDescriptorInfo(int *desc_tag, unsigned int *freq, int *bw, int *time_slicing, int *mpe_fec, 
									int *constellation, int *code, int *guard, int *txmod);

//CKIM A 20120828 {
private:
	struct _TSPH_TS_INFO *pTsInfo;
	struct _WORKING_TS_INFO
	{
		int nit_only;
		int packet_size;
		int packet_count;
		int tei_packet_count;
		int out_of_sync;
		struct _WORKING_PID_INFO
		{
			int packet_count;
			int scrambled;
			int layer_info;
			int program_number;
		} working_pid_infos[8192];
		int bitrate;
	} working_ts_info;

	int init_psi_si_table_parsing_context(void);
	void log_ts_packet_info(BYTE *ts_packet, int ts_packet_size);
	int process_ts_packet(BYTE *ts_packet, int ts_packet_size);
	int is_ts_info_ready(void);
	void conv_psi_si_stream_type_to_str(int stream_type, char *stream_type_str);
	void fill_psi_si_pid_desc(int pid, char *pid_desc);
	int output_ts_info_to_file(void);
	int pack_ts_info(struct _TSPH_TS_INFO **ppTsInfo);

public:
	int RunTs_Parser(char *szFile, int default_bitrate, int nitFlag, struct _TSPH_TS_INFO **ppTsInfo);
	int RunTs_Parser(BYTE *bData, DWORD dwSize, int default_bitrate, int nitFlag, struct _TSPH_TS_INFO ** ppTsInfo);
	int FreeTsParserMemory(void);
	int GetPmtInfo(int *pNumPgmInfo, struct _TSPH_TS_PGM_INFO **ppPgmInfo);
	int GetPidInfo(int *pNumPidInfo, struct _TSPH_TS_PID_INFO **ppPidInfo);
	void GetSatelliteDescriptorInfo2(int *desc_tag, int *freq, int *rolloff, int *mod_sys, int *mod_typ, int *symbol, int *code);
	void GetCableDescriptorInfo2(int *desc_tag, int *freq, int *mod_typ, int *symbol, int *code);
	void GetTerrestrialDescriptorInfo2(int *desc_tag, unsigned int *freq, int *bw, int *time_slicing, int *mpe_fec, 
		int *constellation, int *code, int *guard, int *txmod);
//CKIM A 20120814 }
};

#endif //__LLDWRAPPER_H__

