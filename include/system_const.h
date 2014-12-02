
#ifndef	__SYS_CONST_H__
#define	__SYS_CONST_H__

static char 	szRegString_ROOT_KEY[] = "Software\\VB and VBA Program Settings\\";
static char 	*szRegString_PARENT_KEY[] = {(char*)"TPG0370", (char*)"TPG0380", (char*)"TPG0390", (char*)"TPG0590", (char*)""};
static char 	*szRegString_KEY[] =
{
	(char*)"DVB-T",
	(char*)"VSB",
	(char*)"QAM-A",
	(char*)"QAM-B",
	(char*)"QPSK",
	(char*)"TDMB",	//	5
	(char*)"16VSB",
	(char*)"DVB-H",
	(char*)"DVB-S2",
	(char*)"ISDB-T",	//	9
	(char*)"ISDB-T-13",
	(char*)"DTMB",
	(char*)"CMMB",
	(char*)"DVB-T2",
	(char*)"RESERVED-O",
	(char*)"ATSC-MH",	//	15
	(char*)"IQ-PLAY",
	(char*)"ISDB-S",	//	17
	(char*)"DVB-C2",
	(char*)"Startup",
	(char*)""
};
static char 	*szRegStr_RfOutScale = (char*)"TVB380RFOutScale";
static char 	*szRegStr_RfOutStep = (char*)"TVB380RFOutStep";
static char 	*szRegStr_Rfshft = (char*)"TVB380RFShift";
static char 	*szRegStr_SysClkAdj = (char*)"TVB380SystemClkAdjust";
static char 	*szRegStr_ModMode = (char*)"TVB380ModulationMode";
static char 	*szRegStr_IFFreq = (char*)"TVB380IFOutFreq";
static char 	*szRegStr_LoopAdapt = (char*)"TVB380LoopAdaptationOption";

#if defined(WIN32)
static char 	*szEvt0 = (char*)"HLD_DEVICE_EVENT#0";
static char 	*szEvt1 = (char*)"HLD_DEVICE_EVENT#1";
#endif

#endif

