#ifndef _VISUAL_UTIL_H_
#define _VISUAL_UTIL_H_
#include <stdio.h>
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#if defined(WIN32)
using namespace System; 
using namespace Microsoft::Win32;
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "variable.h"		// LINUX
#endif

class CUTILITY
{
private:

public:
    CUTILITY();
    //--- Registry control value
#if defined(WIN32)
    int     Get_Registry_Integer_Value(RegistryKey^ Reg, char *strKey, char *strName, int iDefaultValue);
    double   Get_Registry_Float_Value(RegistryKey^ Reg, char *strKey, char *strName, double dblDefaultValue);
    char    *Get_Registry_String_Value(RegistryKey^ Reg, char *strKey, char *strName, char *strDefault);
    void    Set_Registry_Integer_Value(RegistryKey^ Reg, char *strKey, char *strName, long iValue);
    void    Set_Registry_Float_Value(RegistryKey^ Reg, char *strKey, char *strName, double dblValue);
    void    Set_Registry_String_Value(RegistryKey^ Reg, char *strKey, char *strName, char *strValue);
    void    Delete_Registry_Key(RegistryKey^ Reg, char *strKey);
	//void	Delete_All_Registry_Key(HKEY hkey, char *strKey);
    char    *Get_Main_Parameter(int ith);
	//kslee 2010/1/20
	void	Set_Registry_UnsignedLong_Value(RegistryKey^ Reg, char *strKey, char *strName, unsigned long iValue);
	//kslee 2010/3/31
    unsigned long Get_Registry_UnsignedLong_Value(RegistryKey^ Reg, char *strKey, char *strName, int iDefaultValue);
#else
	int 	Get_Registry_Integer_Value(RegistryKey Reg, char *strKey, char *strName, int iDefaultValue);
	double	Get_Registry_Float_Value(RegistryKey Reg, char *strKey, char *strName, double dblDefaultValue);
	char	*Get_Registry_String_Value(RegistryKey Reg, char *strKey, char *strName, char *strDefault);
	void	Set_Registry_Integer_Value(RegistryKey Reg, char *strKey, char *strName, int iValue);
	void	Set_Registry_Float_Value(RegistryKey Reg, char *strKey, char *strName, double dblValue);
	void	Set_Registry_String_Value(RegistryKey Reg, char *strKey, char *strName, char *strValue);
	void	Delete_Registry_Key(RegistryKey Reg, char *strKey);
	//kslee 2010/3/31
	unsigned long Get_Registry_UnsignedLong_Value(RegistryKey Reg, char *strKey, char *strName, int iDefaultValue);
	//kslee 2010/5/3
	void Set_Registry_UnsignedLong_Value(RegistryKey Reg, char *strKey, char *strName, unsigned long iValue);
	char    *Get_Main_Parameter(int ith);
#endif    

    void    Create_Directory(char *strDirectory);
    bool    Is_Directory_Exist(char *strDirectory);
    bool    Is_File_Exist(char *strFile);
    void    Delete_File_TEL(char *strFile);
    bool    Copy_File_TEL(char *strSource, char * strDest, bool bFailIfExist);
    bool    Move_File_TEL(char *strSource, char * strDest);

    //--- General Utility
    char 	*LoadResString(int inum);
    void    Send_NanoSolTech(long nBoardNum);
#ifdef WIN32    
	void    DisplayMessage(String^ strMessage);
#else
	void    DisplayMessage(char *strMessage);
#endif
    __int64 GetFreeDiskInSecondFormat(char *strFilename, long dwPlayRate);
	__int64 GetFreeDiskBytes(char *strFilename);
#ifdef WIN32
	void    SNMP_DataArrival(char *strMsg);
#else
	static void    SNMP_DataArrival(char *strMsg);
#endif
	void	MySprintf(char *strDest, int iSize, char *strFormat, ...);
	void	MyStrCpy(char *strDest, int iSize, char *strSource);
	void	MyStrCat(char *strDest, int iSize, char *strSource);
	FILE	*MyFopen(FILE *hFile, char *strFile, char *strMod);
	void	MySscanfOne(char *strDest, char *strFormat, int *iVal);
	void	MySscanfOneFloat(char *strDest, char *strFormat, float *fVal);
	void	Display_Confirm_Dialog();
#ifndef WIN32
	void Get_T2MI_PARAMETERS_200(int nBoardNum, char *T2MIdata);
	void Get_RemuxInfo(int nBoardNum, char *RemuxInfo);
	void Get_ATSC_MH_PARAMETERS_256(int nBoardNum, char *ATSC_MH_data);


	int Stat (const char *a_name, struct stat& a_stat, bool a_supportSymbolLink = false) const;
	int Delete (const char *a_name, const struct stat& a_stat);
	int Delete (const char *a_name);
	int RemoveDir (const char *a_name);
	int Unlink (const char *a_name);
	int LockRegularFile (int a_fd, bool a_isReadOnly);
	int UnlockRegularFile (int a_fd);
	int CopyRegularFile (const char *a_source, const char *a_target, const struct stat& a_sourceStat);
	
	int Get_Line_And_Matched_Value(RegistryKey Reg, char *strName, char *strValue);
	int Get_A_Line(FILE *fp, char *strDest);
#endif
};

extern CUTILITY* gc_utility;

#endif	//_VISUAL_UTIL_H_
 