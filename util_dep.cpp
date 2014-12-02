//-------------------------------------------------
// NO UI COMPONENT RELATED
#include "stdafx.h"
#if defined(WIN32)
#include "dos.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>

#include "main.h"
#include "util_dep.h"
#include "wrapdll.h"
#include "util_ind.h"
#include "PlayForm.h"
#include "baseutil.h"

using namespace TPG0590VC;
using namespace System; 
using namespace Microsoft::Win32;
using namespace System::ComponentModel;
using namespace System::Windows::Forms;
using namespace System::IO;
using namespace System::IO::Ports;
#else
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
//sh4
#include <stdarg.h>

#include "variable.h"
#include "wrapdll.h"
#include "util_dep.h"
#include "util_ind.h"
#include 	"mainutil.h"

#endif

extern APPLICATION_CONFIG      *gpConfig;
extern TEL_GENERAL_VAR          gGeneral;
extern CWRAP_DLL                gWrapDll;
extern CUTIL_IND                gUtilInd;
extern	char			gstrGeneral[256];
#ifndef WIN32
extern  PlayForm		gGui;
#endif
//091217
char recv_msg[256];
//extern  void CallDelegate(char *recv_msg);
CUTILITY				gUtility;

//---------------------------------------------------------------------------
CUTILITY::CUTILITY()
{
}

//---------------------------------------------------------------------------
// Read String and convert to integer
#ifdef WIN32
int CUTILITY::Get_Registry_Integer_Value(RegistryKey^ Reg, char *strKey, char *strName, int iDefaultValue)
{
	int         iRet;
	String^		sStr;
	char		cStr[256];
	sStr = Reg->GetValue(gcnew String(strName), iDefaultValue)->ToString();
	gBaseUtil.ChangeCharFromString(sStr, cStr);
	if (gUtilInd.IsNumericTel(cStr) == true)
		gUtility.MySscanfOne(cStr, "%d", &iRet);
#else
int CUTILITY::Get_Registry_Integer_Value(RegistryKey Reg, char *strKey, char *strName, int iDefaultValue)
{
	int         iRet = iDefaultValue;
	char		strValue[256];
	int			iValid = 0;

	//---No File, then return default
	if (Reg == NULL)
		return iRet;

	//---read a line, check strName
	iValid = Get_Line_And_Matched_Value(Reg, strName, strValue); 
	if (iValid <= 0)
		return iRet;
	
	//--- change to integer
	iRet = atoi(strValue);
#endif
	return iRet;
}
#ifdef WIN32
unsigned long CUTILITY::Get_Registry_UnsignedLong_Value(RegistryKey^ Reg, char *strKey, char *strName, int iDefaultValue)
{

	double  iRet;
	String^		sStr;
	char		cStr[256];
	unsigned long ulRet;
	sStr = Reg->GetValue(gcnew String(strName), iDefaultValue)->ToString();
	gBaseUtil.ChangeCharFromString(sStr, cStr);
	if (gUtilInd.IsNumericTel(cStr) == true)
	{
		iRet = double::Parse(sStr);
		ulRet = (unsigned long)iRet; 
	}
	return ulRet;
}
#else
unsigned long CUTILITY::Get_Registry_UnsignedLong_Value(RegistryKey Reg, char *strKey, char *strName, int iDefaultValue)
{
	unsigned long	iRet = (unsigned long)iDefaultValue;
	char		strValue[256];
	int		iValid = 0;
	int			i;

	if(Reg == NULL)
		return iRet;

	iValid = Get_Line_And_Matched_Value(Reg, strName, strValue);
	if(iValid <= 0)
		return iRet;

	int len = strlen(strValue) / 2;
	unsigned long tmp1, tmp2;
	char temp1[10];
	char temp2[10];
	for(i = 0; i < len; i++)
	{
		temp1[i] = strValue[i];
	}
	temp1[i] = '\0';
	tmp1 = atoi(temp1);
	for(i = 0; i < (strlen(strValue) - len); i++)
	{
		tmp1 = tmp1 * 10;
	}
	for(i = len; i < strlen(strValue); i++)
	{
		temp2[i - len] = strValue[i];
	}
	temp2[i - len] = '\0';
	tmp2 = atoi(temp2);
	iRet = tmp1 + tmp2;
	return iRet;
}
#endif

//---------------------------------------------------------------------------
// Read String and convert to integer
#ifdef WIN32
double CUTILITY::Get_Registry_Float_Value(RegistryKey^ Reg, char *strKey, char *strName, double dblDefaultValue)
{
	double       dblRet;
	String^		sStr;
	char		cStr[256];
	sStr = Reg->GetValue(gcnew String(strName), dblDefaultValue)->ToString();
	gBaseUtil.ChangeCharFromString(sStr, cStr);
	if (gUtilInd.IsNumericTel(cStr) == true)
		dblRet = DOUBLE::Parse(sStr);
		//gUtility.MySscanfOneFloat(cStr, "%f", &dblRet);
#else
double CUTILITY::Get_Registry_Float_Value(RegistryKey Reg, char *strKey, char *strName, double dblDefaultValue)
{
	float       dblRet = (float)dblDefaultValue;
	char		strValue[256];
	int			iValid = 0;
	//---No File, then return default
	if (Reg == NULL)
		return dblRet;
	
	//---read a line, check strName
	iValid = Get_Line_And_Matched_Value(Reg, strName, strValue); 
	if (iValid <= 0)
		return dblRet;
		
	//--- change to integer
	gUtility.MySscanfOneFloat(strValue, (char *) "%f", &dblRet);
#endif
	return dblRet;

}

//---------------------------------------------------------------------------
#ifdef WIN32
char *CUTILITY::Get_Registry_String_Value(RegistryKey^ Reg, char *strKey, char *strName, char *strDefault)
{
	String^		sStr;
	sStr = Reg->GetValue(gcnew String(strName), gcnew String(strDefault))->ToString();
	gBaseUtil.ChangeCharFromString(sStr, gstrGeneral);
#else
char *CUTILITY::Get_Registry_String_Value(RegistryKey Reg, char *strKey, char *strName, char *strDefault)
{
	int			iValid = 0;
	strcpy(gstrGeneral, strDefault);

	//---No File, then return default
	if (Reg == NULL)
		return gstrGeneral;
	
	//---read a line, check strName
	iValid = Get_Line_And_Matched_Value(Reg, strName, gstrGeneral); 
	if (iValid <= 0)
	{
		strcpy(gstrGeneral, strDefault);
		return gstrGeneral;
	}
#endif
	return gstrGeneral;
}

//---------------------------------------------------------------------------
#ifdef WIN32
void CUTILITY::Set_Registry_Integer_Value(RegistryKey^ Reg, char *strKey, char *strName, long iValue)
{
	Reg->SetValue(gcnew String(strName), iValue, RegistryValueKind::String);
#else
void CUTILITY::Set_Registry_Integer_Value(RegistryKey Reg, char *strKey, char *strName, int iValue)
{
	if (Reg == NULL)
		return;

	fprintf(Reg, "%s=%d\n", strName, iValue);
#endif
}
//kslee 2010/1/20
#ifdef WIN32
void CUTILITY::Set_Registry_UnsignedLong_Value(RegistryKey^ Reg, char *strKey, char *strName, unsigned long iValue)
{
	Reg->SetValue(gcnew String(strName), iValue, RegistryValueKind::String);
#else
void CUTILITY::Set_Registry_UnsignedLong_Value(RegistryKey Reg, char *strKey, char *strName, unsigned long iValue)
{
	if (Reg == NULL)
		return;
	fprintf(Reg, (char *)"%s=%u\n", strName, (unsigned int)iValue);
#endif
}

//---------------------------------------------------------------------------
#ifdef WIN32
void CUTILITY::Set_Registry_Float_Value(RegistryKey^ Reg, char *strKey, char *strName, double dblValue)
{
	Reg->SetValue(gcnew String(strName), dblValue, RegistryValueKind::String);
#else
void CUTILITY::Set_Registry_Float_Value(RegistryKey Reg, char *strKey, char *strName, double dblValue)
{
	if (Reg == NULL)
		return;

	fprintf(Reg, "%s=%f\n", strName, dblValue);
#endif
}

//---------------------------------------------------------------------------
#ifdef WIN32
void CUTILITY::Set_Registry_String_Value(RegistryKey^ Reg, char *strKey, char *strName, char *strValue)
{
	Reg->SetValue(gcnew String(strName), gcnew String(strValue), RegistryValueKind::String);
#else
void CUTILITY::Set_Registry_String_Value(RegistryKey Reg, char *strKey, char *strName, char *strValue)
{
	if (Reg == NULL)
		return;

	fprintf(Reg, "%s=%s\n", strName, strValue);
#endif
}

//---------------------------------------------------------------------------
#ifdef WIN32
void CUTILITY::Delete_Registry_Key(RegistryKey^ Reg, char *strKey)
{
	Reg->DeleteSubKeyTree(gcnew String(strKey));
#else
void CUTILITY::Delete_Registry_Key(RegistryKey Reg, char *strKey)
{
#endif
}

//---------------------------------------------------------------------------
// Get n-th Command argument: CHECKCHECK
char    *CUTILITY::Get_Main_Parameter(int ith)
{
	gUtility.MyStrCpy(gstrGeneral,256, "aaa"); 
    return gstrGeneral;
}

//---------------------------------------------------------------------------
// Divide Directory, File
// Make From Root to Last Directories
// Example: C:\AA\BB\CC --> make AA, make AA\BB, and make AA\BB\CC     B
// strDirectory: directory name including drive letter (C:\AA\BB...)
void CUTILITY::Create_Directory(char *strDirectory)
{
    char        str[256];
    int         i;
    int         iLen = (int) strlen(strDirectory);
#ifdef WIN32
    for (i = 1; i < iLen; i++)
    {
		if (strDirectory[i] == '\\')
		{
			gUtility.MyStrCpy(str, 256, strDirectory);
			str[i] = '\0';
			CreateDirectory(str, NULL);
        }
    }
    CreateDirectory(strDirectory, NULL);
#else
	mode_t		a_mode = 0777;
	
    for (i = 1; i < iLen; i++)
    {
		if (strDirectory[i] == '/')
		{
			gUtility.MyStrCpy(str, 256, strDirectory);
			str[i] = '\0';
			mkdir(str, a_mode);
        }
    }
    mkdir(strDirectory, a_mode);
#endif
}

//---------------------------------------------------------------------------
// Test if we can create a file on this directory
bool CUTILITY::Is_Directory_Exist(char *strDirectory)
{
	FILE	*hFile = NULL;
	char	strTempFile[256];

	if (strlen(strDirectory) <= 3)	// maybe c:/
		return true;
#ifdef WIN32
	gUtility.MySprintf(strTempFile, 256, "%s\\TEST", strDirectory);
#else
	gUtility.MySprintf(strTempFile, 256, (char *) "%s/TEST", strDirectory);
#endif
	hFile = gUtility.MyFopen(hFile, strTempFile, "wb");
	if (hFile == NULL)
		return false;

	fclose(hFile);
	Delete_File_TEL(strTempFile);
	return true;
}

//---------------------------------------------------------------------------
bool CUTILITY::Is_File_Exist(char *strFile)
{
	if (strlen(strFile) <= 0)
		return false;

	FILE	*hFile = NULL;
	hFile = gUtility.MyFopen(hFile, strFile, "rb");
	if (hFile == NULL)
		return false;

	fclose(hFile);
	return true;
}

//---------------------------------------------------------------------------
void CUTILITY::Delete_File_TEL(char *strFile)
{
#ifdef WIN32
    DeleteFile(strFile);
#else
	struct stat tmpStat;

	if ( Stat(strFile, tmpStat) == -1)
		return;
	
	Delete(strFile, tmpStat);
#endif
}

//---------------------------------------------------------------------------
bool CUTILITY::Copy_File_TEL(char *strSource, char * strDest, bool bFailIfExist)
{
#ifdef WIN32
    BOOL    bRet;
    bRet = CopyFile(strSource, strDest, bFailIfExist);        // 3rd parameter: bFailIfExist.
#else
    bool    bRet;
	struct stat sStat;
    bRet = CopyRegularFile(strSource, strDest, sStat);
#endif
	if (bRet == 0)
		return false;
	else
		return true;
}

//---------------------------------------------------------------------------
bool CUTILITY::Move_File_TEL(char *strSource, char * strDest)
{
#ifdef WIN32
    BOOL    bRet;
    bRet = MoveFile(strSource, strDest);
	if (bRet == 0)
#else
	if (rename(strSource, strDest) == -1)
#endif
		return false;
	else
		return true;
}

//---------------------------------------------------------------------------
char *CUTILITY::LoadResString(int inum)
{
    
    gUtility.MyStrCpy(gstrGeneral, 256, "TEST");      
    return gstrGeneral;
}

//---------------------------------------------------------------------------
void CUTILITY::Send_NanoSolTech(long nBoardNum)
{
#ifdef WIN32
	FormCollection^ forms;
	forms = Application::OpenForms;
	String^ szData;
	
	for(int i=0; i < forms->Count; i++)
	{
		if(forms[i]->Name == "PlayForm")
		{
			try
			{

				if(safe_cast<PlayForm^>(forms[i])->serialPort1->IsOpen == true)
				{
					safe_cast<PlayForm^>(forms[i])->serialPort1->Close();
				}
				safe_cast<PlayForm^>(forms[i])->serialPort1->PortName = "COM" + gpConfig->gBC[nBoardNum].gnCommPort_NanoSolTech.ToString();
				safe_cast<PlayForm^>(forms[i])->serialPort1->Open();
				szData = "$FRE," + ((double)gpConfig->gBC[nBoardNum].gnRFOutFreq / (double)1000000).ToString() + "\r\n";
				safe_cast<PlayForm^>(forms[i])->serialPort1->Write(szData);
				Sleep(200);
	
				szData = "$LEVEL," + 
						 (gpConfig->gBC[nBoardNum].gRFPowerLevel_NanoSolTech - gpConfig->gBC[nBoardNum].gRFPowerLevelOffset).ToString() +
						 "\r\n";
				safe_cast<PlayForm^>(forms[i])->serialPort1->Write(szData);
				Sleep(200);
	
				safe_cast<PlayForm^>(forms[i])->serialPort1->Close();
				gUtilInd.LogMessage("");
				break;
			}
			catch(IOException^ e)
			{
				gUtilInd.LogMessageInt(TLV_RS232C_TX_ERROR);
			}
		}
	}
	#endif
	// COMPORT Open:
    //      Port#, 9600,N,8,1
    // Send "$FRE, MHz LF CR"
    // Sleep(200)
    // Send "LEVEL, dBm LF CR"      // LF=13,CR=10
    // COMPORT Close:
}

//---------------------------------------------------------------------------
#ifdef WIN32
//2010/12/14
void CUTILITY::DisplayMessage(String^ strMessage)
#else
void CUTILITY::DisplayMessage(char *strMessage)
#endif
{
#ifdef WIN32
	MessageBox::Show(strMessage, "TVB0590", MessageBoxButtons::OK);
#else
	if (strMessage != NULL)
  		printf("%s\n", strMessage);
#endif
}

//---------------------------------------------------------------------------
// Return Free Disk Size in Second
__int64 CUTILITY::GetFreeDiskInSecondFormat(char *strFilename, long dwPlayRate)
{
#ifdef WIN32
	ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes, i64FreeBytes;
	char	szDrv[10];
	BOOL	fResult;
	__int64	dblReturn = 100000; 

	if (dwPlayRate == 0)
			return dblReturn;
	strncpy_s(szDrv, 10, strFilename, 3);		// D:/
	fResult = GetDiskFreeSpaceEx (szDrv, (PULARGE_INTEGER)&i64FreeBytesToCaller,
										(PULARGE_INTEGER)&i64TotalBytes,
										(PULARGE_INTEGER)&i64FreeBytes);
	if (fResult == false) 
		return dblReturn;



	dblReturn = (__int64) (i64FreeBytes.QuadPart*8/dwPlayRate);

    return dblReturn;
#else
	__int64 	i64FreeBytes;
	__int64		dblReturn = 36000; 
	struct statvfs fiData;
	char fnPath[128];

    strcpy(fnPath, "/");
    if((statvfs(fnPath,&fiData)) < 0 ) 
	{
		 printf("********* Failed to stat %s:\n", fnPath);
		 return dblReturn;		// 10 hours
	} else {
		 i64FreeBytes = (__int64) fiData.f_bsize * fiData.f_bfree;	
		 if (dwPlayRate == 0)
		 	dwPlayRate = 19392658;

		dblReturn = i64FreeBytes * 8 / dwPlayRate;
	}

    return dblReturn;
#endif

}

//---------------------------------------------------------------------------
// Return Free Disk Size in Second
__int64 CUTILITY::GetFreeDiskBytes(char *strFilename)
{
#ifdef WIN32
	ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes, i64FreeBytes;
	char	szDrv[10];
	BOOL	fResult;
	__int64	dblReturn = -1; 

	strncpy_s(szDrv, 10, strFilename, 3);		// D:/
	fResult = GetDiskFreeSpaceEx (szDrv, (PULARGE_INTEGER)&i64FreeBytesToCaller,
										(PULARGE_INTEGER)&i64TotalBytes,
										(PULARGE_INTEGER)&i64FreeBytes);
	if (fResult == false) 
		return dblReturn;



	dblReturn = (__int64) (i64FreeBytes.QuadPart);

    return dblReturn;
#else
	__int64 	i64FreeBytes;
	__int64		dblReturn = -1; 
	struct statvfs fiData;
	char fnPath[128];

    strcpy(fnPath, "/");
    if((statvfs(fnPath,&fiData)) < 0 ) 
	{
		 printf("********* Failed to stat %s:\n", fnPath);
	} else {
		 i64FreeBytes = (__int64) fiData.f_bsize * fiData.f_bfree;	
		 dblReturn = i64FreeBytes;
	}

    return dblReturn;
#endif

}

//--------------------------------------------------------------
void CUTILITY::SNMP_DataArrival(char *strMsg)
{
#ifdef WIN32

	//TPG0590VC::PlayForm fmain;
	//fmain.SNMP_DataArrival(strMsg);
    //gMainDlg->SNMP_DataArrival(strMsg);
	//09/12/18
	//CallDelegate(strMsg);
	FormCollection^ forms;
	forms = Application::OpenForms;

	memset(recv_msg, 0, sizeof(recv_msg));
	MyStrCpy(recv_msg, sizeof(recv_msg), strMsg);

	for(int i=0; i < forms->Count; i++)
	{
		if(forms[i]->Name == "PlayForm")
		{
			//String^ str = gcnew String(strMsg);
			//
			//array<Object^>^myStringArray = {str};
			safe_cast<PlayForm^>(forms[i])->Invoke(safe_cast<PlayForm^>(forms[i])->CallDelegate);
			//delete str;
			break;
		}
	}
#else
	 gGui.SNMP_DataArrival(strMsg);
#endif
}

//--------------------------------------------------------------
void CUTILITY::MySprintf(char *strDest, int iSize, char *strFormat, ...)
{
#ifdef WIN32
	vsnprintf_s(strDest, iSize,  _TRUNCATE, strFormat, (LPSTR)(&strFormat+1));
#else
	va_list ap;
	va_start(ap, strFormat);
	vsnprintf(strDest, iSize, strFormat, ap);
	va_end(ap);	
#endif
}
//--------------------------------------------------------------
void CUTILITY::MyStrCpy(char *strDest, int iSize, char *strSource)
{
#ifdef WIN32
	strcpy_s(strDest, iSize, strSource);
#else
	strcpy(strDest, strSource);
#endif
}

//--------------------------------------------------------------
void CUTILITY::MyStrCat(char *strDest, int iSize, char *strSource)
{
#ifdef WIN32
	strcat_s(strDest, iSize, strSource);
#else
	strcat(strDest, strSource);
#endif
}

//--------------------------------------------------------------
FILE *CUTILITY::MyFopen(FILE *hFile, char *strFile, char *strMod)
{
#ifdef WIN32
	errno_t		err;

	err = fopen_s(&hFile, strFile, strMod);
#else
	hFile = fopen(strFile, strMod);
#endif
	return hFile;
}

//--------------------------------------------------------------
void CUTILITY::MySscanfOne(char *strDest, char *strFormat, int *iVal)
{
#ifdef WIN32
	sscanf_s(strDest, strFormat, iVal);
#else
	sscanf(strDest, strFormat, iVal);
#endif
}

//--------------------------------------------------------------
void CUTILITY::MySscanfOneFloat(char *strDest, char *strFormat, float *fVal)
{
#ifdef WIN32
	sscanf_s(strDest, strFormat, fVal);
#else
	sscanf(strDest, strFormat, fVal);
#endif
}

//--------------------------------------------------------------
void CUTILITY::Display_Confirm_Dialog()
{
		gGeneral.nConfirmPlaying = 0;
}

#ifndef WIN32
//2012/2/29 LINUX Merge ===============================================================
void CUTILITY::Get_T2MI_PARAMETERS_200(int nBoardNum, char *T2MIdata)
{
	int i,j, loop, cnt;

	for(i = 0; i < 17 ; i++)
	{
		MyStrCpy(gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[i], 256, (char *)"");
	}
	loop = 4096;
	cnt = 0;
	j = 0;
	for(i = 0 ; i < loop ; i++)
	{
		if(T2MIdata[i] == ' ')
			gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j][(i % 250)] = '*';
		else if(T2MIdata[i] == '\r')
			gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j][(i % 250)] = '[';
		else if(T2MIdata[i] == '\n')
			gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j][(i % 250)] = ']';
		else
			gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j][(i % 250)] = T2MIdata[i];
		
		cnt++;
		if((cnt % 250) == 0)
		{
			gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j++][(i % 250) + 1] = '\0'; 
		}

	}
	gpConfig->gBC[nBoardNum].gsz_T2MI_Parameter[j][(i % 250)] = '\0';
}
void CUTILITY::Get_RemuxInfo(int nBoardNum, char *RemuxInfo)
{
	int i,j, loop, cnt;

	loop = strlen(RemuxInfo);
	for(i = 0; i < 9 ; i++)
	{
		MyStrCpy(gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[i], 256, (char *)"");
	}
	cnt = 0;
	j = 0;
	for(i = 0 ; i < loop ; i++)
	{
		if(RemuxInfo[i] == ' ')
			gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j][(i % 250)] = '*';
		else if(RemuxInfo[i] == '\r')
			gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j][(i % 250)] = '[';
		else if(RemuxInfo[i] == '\n')
			gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j][(i % 250)] = ']';
		else
			gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j][(i % 250)] = RemuxInfo[i];
		
		cnt++;
		if((cnt % 250) == 0)
		{
			gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j++][(i % 250) + 1] = '\0'; 
		}

	}
	gpConfig->gBC[nBoardNum].gsz_Tmcc_RemuxInfo[j][(i % 250)] = '\0';
}

//2010/5/24
void CUTILITY::Get_ATSC_MH_PARAMETERS_256(int nBoardNum, char *ATSC_MH_data)
{
	int i,j, loop, cnt;

	loop = strlen(ATSC_MH_data);
	for(i = 0; i < 9 ; i++)
	{
		MyStrCpy(gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[i], 256, (char *)"");
	}
	cnt = 0;
	j = 0;
	for(i = 0 ; i < loop ; i++)
	{
		if(ATSC_MH_data[i] == ' ')
			gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j][(i % 250)] = '*';
		else if(ATSC_MH_data[i] == '\r')
			gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j][(i % 250)] = '[';
		else if(ATSC_MH_data[i] == '\n')
			gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j][(i % 250)] = ']';
		else
			gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j][(i % 250)] = ATSC_MH_data[i];
		
		cnt++;
		if((cnt % 250) == 0)
		{
			gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j++][(i % 250) + 1] = '\0'; 
		}

	}
	gpConfig->gBC[nBoardNum].gsz_ATSC_MH_Parameter[j][(i % 250)] = '\0';
}
//=====================================================================================

//==========================================================================
//--------------------------------------------------------------
int CUTILITY::Stat (const char *a_name, struct stat& a_stat, bool a_supportSymbolLink) const
{
	int result = a_supportSymbolLink 
		? stat(a_name, &a_stat) : lstat(a_name, &a_stat);
	return result == -1 ? errno : 0;
}

//--------------------------------------------------------------
int CUTILITY::Delete (const char *a_name, const struct stat& a_stat)
{
	if (S_ISDIR(a_stat.st_mode))
		RemoveDir(a_name);
	else
		Unlink(a_name);
}

//--------------------------------------------------------------
int CUTILITY::Delete (const char *a_name)
{
	struct stat tmpStat;
 	if ( Stat(a_name, tmpStat) == -1 )
 		return errno;
 	
	return Delete(a_name, tmpStat);
}

//--------------------------------------------------------------
int CUTILITY::RemoveDir (const char *a_name)
{
	return rmdir(a_name) == -1 ? errno : 0;
}

//--------------------------------------------------------------
int CUTILITY::Unlink (const char *a_name)
{
	int result = 0;
	int fd = open(a_name, O_WRONLY);
	if ( fd < 0 )
	{
		return errno;
	}

	result = LockRegularFile(fd, false);
	if ( result )
	{
		close(fd);
		return result;
	}

	result = unlink(a_name);
	close(fd);
	return result == -1 ? errno : 0;
}

//--------------------------------------------------------------
int CUTILITY::LockRegularFile (int a_fd, bool a_isReadOnly)
{
	struct flock ldata;
	ldata.l_type = (a_isReadOnly ? F_RDLCK : F_WRLCK);
	ldata.l_whence = SEEK_SET;
	ldata.l_start = 0;
	ldata.l_len = 0;
	
	int result = fcntl(a_fd, F_SETLK, &ldata);
	return result == -1 ? errno : 0;
}

//--------------------------------------------------------------
int CUTILITY::UnlockRegularFile (int a_fd)
{
	struct flock ldata;
	ldata.l_type = F_UNLCK;
	ldata.l_whence = SEEK_SET;
	ldata.l_start = 0;
	ldata.l_len = 0;
	
	int result = fcntl(a_fd, F_SETLK, &ldata);
	return result == -1 ? errno : 0;
}

//--------------------------------------------------------------
int CUTILITY::CopyRegularFile (const char *a_source, const char *a_target, const struct stat& a_sourceStat)
{
	int copyCount = 0;

	int source = open(a_source, O_RDONLY);
	if ( source == -1 )
	{
		return errno;
	}

	int result = LockRegularFile(source, true);
	if ( result != 0 )
	{
		return EBUSY;
	}

	int target = creat(a_target, a_sourceStat.st_mode);
	if ( target == -1 )
	{
		close(source);
		return EBUSY;
	}

	result = LockRegularFile(target, false);
	if ( result != 0 )
	{
		close(source);
		close(target);
		return result;
	}

	char buf[64*1024];
	int bytesRead = 0;
	int bytesWritten = 0;
	off_t bytesTotal = 0;
	
	while( true )
	{
		if ( (bytesRead = read(source, buf, 64*1024)) <= 0 )
		{
			break;
		}
		
		if ( (bytesWritten = write(target, buf, bytesRead)) <= 0 )
		{
			break;
		}

		bytesTotal += bytesWritten;
		if ( ++copyCount % 64 == 0 )
		{
			copyCount = 0;
			fdatasync(target);
			sleep(50);
		}
	}

	close(source);
	close(target);

	if ( a_sourceStat.st_size == bytesTotal )
	{
		sleep(100);
		return 0;
	}

	return 1;
}

//----------------------------------------------------------------------
// read a line 
// check strName
// get strValue
// return 1 if len(strValue) > 0
// return 0
int CUTILITY::Get_Line_And_Matched_Value(RegistryKey Reg, char *strName, char *strValue)
{
	int 	iRet = 0;
	char	strLine[256];
	size_t	len = 256;
	ssize_t	read;
	int		i,j;
	char	strTempName[256];
	char	strTempValue[256];
	int		index = 0;
	
	if (Reg == NULL)
		return iRet;
	
	fseek(Reg, 0, SEEK_SET);
	memset(strLine, 0x00, sizeof(strLine));

	Get_A_Line(Reg, strLine);
	while (strlen(strLine) > 2)
	{
		//DebugOutEx("------ Get Line: (%s)", strLine);
		for (i = 0; i < strlen(strLine); i++)
		{
			if (strLine[i] == '=')
			{
				strncpy(strTempName, strLine, i);
				strTempName[i] = '\0';
				//DebugOutEx("   Title=(%s)i=%d, Taget=(%s)", strTempName, i, strName);
				if (strcmp(strTempName, strName) == 0)
				{
					index = 0;
					for (j = i+1; j < strlen(strLine); j++)
					{
						if (strLine[j] != '\n')
							strValue[index++] = strLine[j];
					}
					strValue[index]= '\0';
					
					DebugOutEx((char *) "****** Get Value: (%s) --> (%s)", strTempName, strValue);
					if (strlen(strValue) > 0)
						return 1;
					else 
						return 0;
				}
				break;	// break for loop. goto net line
			}
		}
		memset(strLine, 0x00, sizeof(strLine));
		Get_A_Line(Reg, strLine);
	}


	return 0;
}


int CUTILITY::Get_A_Line(FILE *fp, char *strDest)
{
	int		i = 0;
	char	strLine[256];

	memset(strLine, 0x00, 256);
	
	fgets(strLine, 150, fp);
	for (i = 0; i < strlen(strLine); i++)
		if (strLine[i] == 0x0A || strLine[i] == 0x0D)
		{
			strLine[i] = '\0';
			break;
		}

	strcpy(strDest, strLine);
	//DebugOutEx("------ Get Line: i=%d, (%s)(%s)", i, strLine,strDest);
	return 1;
}
#endif