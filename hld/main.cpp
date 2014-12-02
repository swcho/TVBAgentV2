//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "main.h"
#include "tsanal.h"
#include "about.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;

TS_REMUX_CONFIG     gConfig;
CTSAnal             gTA;        // TS Analyze Class

BYTE                *gInputBuffer;
BYTE                *gOutputBuffer;
int                 gInputSize;
int                 gOutputSize;
int                 gEOF = 0;

float	            gfRatio;
int 	            gStuffRatio[10];		// for 10,100,1000,   10K, 100K, 1M
DWORD               gTotalReadPacket = 0;
int                 giTargetBitrate = 1;

BYTE	gNullPack[204] = {0x47,0x1F, 0xFF, 0x10,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00,				// 10
			0x00,0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,0x00};				// 200

__int64     giiFirstPCR = 0;
int         giAddedNullPacket = 0;
int         giFirstPCRNullPacket = 0;       // PCR처음 나왔을때 Null Packet added된 수 

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
    : TForm(Owner)
{
    m_hInputFile = INVALID_HANDLE_VALUE;
    m_hOutputFile = INVALID_HANDLE_VALUE;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::FormCreate(TObject *Sender)
{
    // Create
    gInputBuffer = new BYTE[1024*1024]; // 1MB
    gOutputBuffer = new BYTE[204*10000];    // 2MB
    gInputSize = 0;
    gOutputSize = 0;
    ReadConfigurationFile();
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::FormShow(TObject *Sender)
{
    // Show
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    BSTOPClick(Sender);
    
    WriteConfigurationFile();
    // Close
    delete gInputBuffer;
    delete gOutputBuffer;
}

//---------------------------------------------------------------------------
// File Open
void __fastcall TFormMain::BFILEClick(TObject *Sender)
{
    // File Open
    FILE    *hFile = NULL;
    int     index;
    char    strCurDir[256];
    char    text[100];

    GetCurrentDirectory(256, strCurDir);

    //----------------------------------------------
    // File Open
    OpenFileDlg->FileName = "*.*";
    OpenFileDlg->InitialDir = gConfig.strLastReadFilename; "D:\\TestTS\\MERETA"; //strCurDir;
    if (OpenFileDlg->Execute() == false)
    {
        return;
    }

    //----------------------------------------------
    if (OpenFileDlg->FileName == "") {
        ShowMessage("You should select a TS file");
        return;
	}

    //------------------------------------------------------
    // Check if file exist
    //------------------------------------------------------
    hFile = fopen(OpenFileDlg->FileName.c_str(), "rb");
    if (hFile == NULL)
    {
        return;
    }
    fclose(hFile);

    strcpy(gConfig.nfc.strFilename, OpenFileDlg->FileName.c_str());

    //------------------------------------------------------
    // Get File Information: FileSize, Packet Size, Bitrate
    //------------------------------------------------------
    gTA.Get_TransportStream_Information(gConfig.nfc.strFilename, (long *) &gConfig.nfc.finfo);

    //--- Display File Information
    SFILENAME->Caption = gConfig.nfc.strFilename;
    wsprintf(text,"%d.%03d MB (dur=%02d:%02d)",
            gConfig.nfc.finfo.iFileSizeInKBytes/1000,
            gConfig.nfc.finfo.iFileSizeInKBytes % 1000,
            gConfig.nfc.finfo.iFileSizeInSec/60,
            gConfig.nfc.finfo.iFileSizeInSec%60);
    LFILESIZE->Caption = text;
    LBITRATE->Caption = gConfig.nfc.finfo.iBitrate;
    LPKTSIZE->Caption = gConfig.nfc.finfo.iPacketSize;
    wsprintf(text,"%s.out", gConfig.nfc.strFilename);
    EOUTFILE->Text = text;
    EOUTBITRATE->Text = gConfig.nfc.finfo.iBitrate;
    giTargetBitrate = atoi(EOUTBITRATE->Text.c_str());
    sprintf(text,"Ratio=%.6f", giTargetBitrate/(gConfig.nfc.finfo.iBitrate*1.0));
    LRATIO->Caption = text;
    /*
    Display_File_Information1(index);

    //------------------------------------------------------
    // Analysis File: Find VPID --> Find First IDR, Find Last IDR
    //------------------------------------------------------

    //------------------------------------------------------
    //--- File Create
    if (gConfig.nfc.hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(gConfig.nfc.hFile);
        gConfig.nfc.hFile = INVALID_HANDLE_VALUE;
    }

    gConfig.nfc.hFile = ::CreateFile(gConfig.nfc.strFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (gConfig.nfc.hFile == INVALID_HANDLE_VALUE)
	    return;

    //------------------------------------------------------
    // Start Analyze from Finding PAT/PMT/VIDEO PID
    ListSIStatus->Clear();
    gTA.Before_Analyze();       // Variable Reset
    gConfig.iPhase = 1;
    TimerAnalysis->Enabled = true;


    //--- Save Last Read Filename
    strcpy(gConfig.strLastReadFilename,gConfig.nfc.strFilename);

    char text[256];
    wsprintf(text,"--- Start to analyze %s", gConfig.nfc.strFilename);
    FormMain->ListSIStatus->Items->Add(text);
    */

    //-------------------------------------------------------
    BFILE->Enabled = true;
    BSTART->Enabled = true;
    BSTOP->Enabled = false;

}

//---------------------------------------------------------------------------
void __fastcall TFormMain::EOUTBITRATEChange(TObject *Sender)
{
    if (BSTART->Enabled == false)
        return;
        
    char    text[100];
    giTargetBitrate = atoi(EOUTBITRATE->Text.c_str());
    sprintf(text,"Ratio=%.6f", giTargetBitrate/(gConfig.nfc.finfo.iBitrate*1.0));
    LRATIO->Caption = text;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::BSTARTClick(TObject *Sender)
{
    char    text[100];
    //-------------------------------------------------------
    // Start to insert Null packet    
    gInputSize = 0;
    gOutputSize = 0;
    gEOF = 0;
    gConfig.iiTotalRead = 0;
    gConfig.iiTotalWrite = 0;
    gTotalReadPacket = 0;

    //--- for PCR restamping
    giiFirstPCR = 0;
    giAddedNullPacket = 0;
    giFirstPCRNullPacket = 0;       // PCR처음 나왔을때 Null Packet added된 수

    //-------------------------------------------------------
    //--- Set Ratio
    giTargetBitrate = atoi(EOUTBITRATE->Text.c_str());
    gfRatio = giTargetBitrate/(gConfig.nfc.finfo.iBitrate*1.0);
    sprintf(text,"Ratio=%.6f", atoi(EOUTBITRATE->Text.c_str())/(gConfig.nfc.finfo.iBitrate*1.0));
    LRATIO->Caption = text;

	gStuffRatio[0] = (int) (gfRatio*10)  - 10;
	gStuffRatio[1] = (int) (gfRatio*100) - ((int) (gfRatio*10)) * 10;
	gStuffRatio[2] = (int) (gfRatio*1000) - ((int) (gfRatio*100)) * 10;
	gStuffRatio[3] = (int) (gfRatio*10000) - ((int) (gfRatio*1000)) * 10;
	gStuffRatio[4] = (int) (gfRatio*100000) - ((int) (gfRatio*10000)) * 10;
	gStuffRatio[5] = (int) (gfRatio*1000000) - ((int) (gfRatio*100000)) * 10;
	gStuffRatio[6] = (int) (gfRatio*10000000) - ((int) (gfRatio*1000000)) * 10;
	gStuffRatio[7] = (int) (gfRatio*100000000) - ((int) (gfRatio*10000000)) * 10;
	gStuffRatio[8] = (int) (gfRatio*1000000000) - ((int) (gfRatio*100000000)) * 10;
	gStuffRatio[9] = (int) (gfRatio*10000000000) - ((int) (gfRatio*1000000000)) * 10;

    //-------------------------------------------------------
    //--- Open Files
    if (m_hInputFile != INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hInputFile);
        m_hInputFile = INVALID_HANDLE_VALUE;
    }
    if (m_hOutputFile != INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hOutputFile);
        m_hOutputFile = INVALID_HANDLE_VALUE;
    }

    m_hInputFile = CreateFile(gConfig.nfc.strFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,0, NULL);
    m_hOutputFile = CreateFile(EOUTFILE->Text.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);

    if (m_hInputFile == INVALID_HANDLE_VALUE || m_hOutputFile == INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hInputFile);
        m_hInputFile = INVALID_HANDLE_VALUE;
		CloseHandle(m_hOutputFile);
        m_hOutputFile = INVALID_HANDLE_VALUE;
        return;
    }

    //-------------------------------------------------------
    //--- Start Progress
    TimerConvert->Enabled = true;

    //-------------------------------------------------------
    BFILE->Enabled = false;
    BSTART->Enabled = false;
    BSTOP->Enabled = true;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::BSTOPClick(TObject *Sender)
{
    //-------------------------------------------------------
    // STOP Converting
    TimerConvert->Enabled = false;

    //-------------------------------------------------------
    //--- Close Files
    if (m_hInputFile != INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hInputFile);
        m_hInputFile = INVALID_HANDLE_VALUE;
    }
    if (m_hOutputFile != INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hOutputFile);
        m_hOutputFile = INVALID_HANDLE_VALUE;
    }

    //-------------------------------------------------------
    BFILE->Enabled = true;
    BSTART->Enabled = true;
    BSTOP->Enabled = false;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::TimerConvertTimer(TObject *Sender)
{
    char    text[100];
    int     iRet;
    int     i;

    for (i = 0; i < 100; i++)
    {
        iRet = ReadFromQueue(m_hInputFile, m_hOutputFile);
        wsprintf(text,"WriteSize=%d.%03d MB",
                (int) (gConfig.iiTotalWrite/1000000),
                (int) (gConfig.iiTotalWrite/1000) % 1000);
        LSTATUS->Caption = text;
        PROGRESS1->Position = (int) (gConfig.iiTotalRead*100/gConfig.nfc.finfo.iiFileSize);

        if (iRet < 0)
        {
            BSTOPClick(Sender);
            wsprintf(text,"WriteSize=%d.%03d MB. Completed",
                (int) (gConfig.iiTotalWrite/1000000),
                (int) (gConfig.iiTotalWrite/1000) % 1000);
            LSTATUS->Caption = text;
            return;
        }
    }
}

//=========================================================================
// File Read해서 Stuffing 채운후에 Queue에 넣는다.
// Queue에 원하는 양의 data가 있으면 추출해서 return
// === OneSeg Stream일 경우에는 queue가 찰때까지 기다림.
//     약 1.5초이내에 원하는 data가 안들어 오면 End Of Stream이라 간주.
//---------------------------------------------------------------------------
// 1. Get TS Packet from input
// 2. Write TS Packet to output
// 3. Write NULL TS Packet to output
int TFormMain::ReadFromQueue(HANDLE hHandle, HANDLE hOutHandle)
{
	int		nByte = 0;
	int		indexPkt;
    int     i,j,k;
    DWORD   dwSizeWritten;

    gOutputSize = 0;
    //-------------------------------------------------------
    // Read Data
    if (gInputSize < gConfig.nfc.finfo.iPacketSize)
    {
		ReadFile(hHandle, &gInputBuffer[gInputSize],  gConfig.nfc.finfo.iPacketSize*1000,  (unsigned long *) &nByte, NULL);		// 18K
        if (nByte < gConfig.nfc.finfo.iPacketSize*1000)
        {
            gEOF++;       // Now Reached EOF. gEOF가 2d이상이면 종료.
            if (gEOF > 1)
                return -1;   // End Of Processing
        }
        gInputSize += nByte;
        gConfig.iiTotalRead += nByte;
    }

	//---------------------------------------------------------------------------
	while (gInputSize >= gConfig.nfc.finfo.iPacketSize)
	{
		//-------------------------------------------------
		// Adjust Sync. sync를 찾아서 File Position을 옮긴다음에 다시 시작한다.
		if (gInputBuffer[0] != 0x47)		// not Synced
		{
            //--- find next 0x47
			for (i = 0; i < gConfig.nfc.finfo.iPacketSize; i++)
            {
                if (gInputBuffer[i] == 0x47)
                {
                    memcpy(gInputBuffer, &gInputBuffer[i], gInputSize - i);
                    gInputSize -= i;
                    break;
                }
            }

            if (i>= gConfig.nfc.finfo.iPacketSize)      // not found이면
            {
                memcpy(gInputBuffer, &gInputBuffer[i], gInputSize - i);
                gInputSize -= i;
            }
            continue;   // while부터 다시 시작 한다.
		}

		//-------------------------------------------------
		// Packet수만큼 반복하면서 Queue에 data를 넣는다.
        // 지금부터는 n packet만큼은 sync맞는다고 생각함.
		indexPkt = gInputSize/gConfig.nfc.finfo.iPacketSize;      // 보통 1000이다.

		for (i = 0; i < indexPkt; i++)
		{
	        WORD	wPID;		
        	BYTE	bADExist;
	        BYTE	bADLen;
            BYTE    *bData = &gInputBuffer[i*gConfig.nfc.finfo.iPacketSize];

            //-------------------------------------------------------------------------------------
            // Check Adaptation/PCR check
            if (bData[0] == 0x47)
            {
			    wPID = (bData[1]&0x1F)*256+bData[2];
			    bADExist = bData[3] & 0x20;
			    bADLen   = bData[4];

			    //----------------------------------------------------------------
			    //--- Adaptation and PCR Check
			    //----------------------------------------------------------------
			    if (bADExist)
			    {
				    if (bADLen >= 7)		// min length for PCR or OPCR
				    {
					    if (bData[5] & 0x10)	// PCR (or OPCR = 0x08)
					    {
						    Replace_PCR(&bData[6]);
					    }
				    }
			    }
            }
            
			//-------------------------------------------------------------------------------------
            // 원본 one packet을 write한다.
            memcpy(&gOutputBuffer[gOutputSize], &gInputBuffer[i*gConfig.nfc.finfo.iPacketSize], gConfig.nfc.finfo.iPacketSize);
            gOutputSize += gConfig.nfc.finfo.iPacketSize;
            gTotalReadPacket++;
            gInputSize -= gConfig.nfc.finfo.iPacketSize;
            giAddedNullPacket++;

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 1 단위
			if (gStuffRatio[0] >= 10)		// 2배 이상 이면
			{
                //--------------------------------------------------
                // 원본 packet하나에 n개의 null packet 넣음.
				for (j=0; j < gStuffRatio[0]/10;j++)        // null packet을 왕창 넣음.
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
				}

                //--------------------------------------------------
                // 원본 packet 여러개 지나면  n개의 null packet 넣음.
				if (gTotalReadPacket % 10 == 0)
				{
					k = gStuffRatio[0] % 10;
					for (j=0; j < k;j++)
                    {
                        memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                        gOutputSize += gConfig.nfc.finfo.iPacketSize;
                        giAddedNullPacket++;
                    }
				}
			} else									// 2배 미만이면
			{
				if (gTotalReadPacket % 10 == 0)
				{
					for (j=0; j < gStuffRatio[0];j++)
                    {
                        memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                        gOutputSize += gConfig.nfc.finfo.iPacketSize;
                        giAddedNullPacket++;
                    }
				}
			}

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.1 단위
			if (gTotalReadPacket % 100 == 0)
			{
				for (j=0; j < gStuffRatio[1];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}
            
			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.01 단위
			if (gTotalReadPacket % 1000 == 0)
			{
				for (j=0; j < gStuffRatio[2];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
               }
			}

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.001 단위
			if (gTotalReadPacket % 10000 == 0)
			{
				for (j=0; j < gStuffRatio[3];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.0001 단위
			if (gTotalReadPacket % 100000 == 0)
			{
				for (j=0; j < gStuffRatio[4];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}
			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.00001 단위
			if (gTotalReadPacket % 1000000 == 0)
			{
				for (j=0; j < gStuffRatio[5];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.000001 단위
			if (gTotalReadPacket % 10000000 == 0)
			{
				for (j=0; j < gStuffRatio[6];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}

			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.0000001 단위
			if (gTotalReadPacket % 100000000 == 0)
			{
				for (j=0; j < gStuffRatio[7];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}
			//-------------------------------------------------------------------------------------
			// Insert Stuffing : 0.00000001 단위
			if (gTotalReadPacket % 1000000000 == 0)
			{
				for (j=0; j < gStuffRatio[8];j++)
                {
                    memcpy(&gOutputBuffer[gOutputSize], gNullPack, gConfig.nfc.finfo.iPacketSize);
                    gOutputSize += gConfig.nfc.finfo.iPacketSize;
                    giAddedNullPacket++;
                }
			}
		}
	}

    //--- Write Data
    WriteFile(hOutHandle, gOutputBuffer, gOutputSize, &dwSizeWritten, NULL);
    gConfig.iiTotalWrite += gOutputSize;
    gOutputSize = 0;

    return 1;
}

//---------------------------------------------------------------------------
//------------------------------------------------------------------------
// PCR in Adaption Field
// 33bit Base + 6bit Reserved + 9bit Extension
// 0) Base 32~25
// 1) Base 24~17
// 2) Base 16~09
// 3) Base 08~01
// 4) Base 00 + Reserved + Ext 08
// 5) Ext  07~00
void TFormMain::Replace_PCR(BYTE *bData)
{
	__int64	iiPCR;
	int		iPCR;
	char	text[100];

	__int64	iimPCR;		// Modified PCR
    __int64 iiOffset;
	int		iExtPCR;
	
	static	int		iOldPCR = 0;

    //--------------------------------------------------------------------------
	//--- Get PCR Value
	iiPCR = bData[0];		
	iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
	iiPCR += bData[1] << 17;
	iiPCR += bData[2] << 9;
	iiPCR += bData[3] << 1;
	iiPCR += (bData[4] & 0x80) >> 7;
	iiPCR *= 300;

	iiPCR += (bData[4] & 0x01) << 8;
	iiPCR += bData[5];

    //--------------------------------------------------------------------------
	//--- Print Original PCR
	iPCR = (int) (iiPCR/27);
	wsprintf(text,"(Original-PCR) : PCR=%d usec . Added NullPacket=%d\n", iPCR, giAddedNullPacket);
    OutputDebugString(text);

    //--------------------------------------------------------------------------
	//--- Check For Bitrate
	if (giiFirstPCR == 0)
	{
		giiFirstPCR = iiPCR;
		giFirstPCRNullPacket   = giAddedNullPacket;
	}

    //--------------------------------------------------------------------------
    // Calculate Add해야할 PCR Offset
    //
    double  dblTime;
    int     iNumPkt = giAddedNullPacket - giFirstPCRNullPacket; //in unit packet
    iNumPkt = iNumPkt * 188 * 8;        // in unit byte

    dblTime = iNumPkt*1.0 / giTargetBitrate;
    iiOffset = (__int64) (27000000*dblTime);

    //--------------------------------------------------------------------------
	//--- Add PCR Offset
	iiPCR = giiFirstPCR + iiOffset;

    //--------------------------------------------------------------------------
	//--- Back to Adaptation Field
	iExtPCR = (int) (iiPCR % 300);
	iimPCR  = iiPCR/300;

	bData[5]  = (BYTE) (iExtPCR & 0xFF);
	bData[4]  = (BYTE) ((iExtPCR & 0x0100) >> 8);
	bData[4] |= (BYTE) ((iimPCR & 0x01) << 7) | 0x7E;

	bData[3]  = (BYTE) ( (iimPCR & 0x01FE) >> 1);
	bData[2]  = (BYTE) ( (iimPCR & 0x01FE00) >> 9);
	bData[1]  = (BYTE) ( (iimPCR & 0x01FE0000) >> 17);
	bData[0]  = (BYTE) ( (iimPCR & 0x01FE000000) >> 25);

	iPCR = (int) (iiPCR/27);
	wsprintf(text,"(Modified-PCR) : pcr=%d usec\n", iPCR);
    OutputDebugString(text);
    //---------------------------------------------------------
    /**********************************************************
	//--- Print Modified PCR
	iiPCR = bData[0];
	iiPCR <<= 25;					// if Directly shift, then signed/unsigned problem
	iiPCR += bData[1] << 17;
	iiPCR += bData[2] << 9;
	iiPCR += bData[3] << 1;
	iiPCR += (bData[4] & 0x80) >> 7;
	iiPCR *= 300;

	iiPCR += (bData[4] & 0x01) << 8;
	iiPCR += bData[5];
	iPCR = (int) (iiPCR/27000);
	if (iPCR > (iOldPCR + 100))
	{
		;// OutputDebugString("***************** TOO DISTANCE\n");
	}
	//wsprintf(text,"iOld=%d, iNew=%d, Diff=%d, Offset=%d\n", iOldPCR, iPCR, iPCR-iOldPCR, (int) (iiOffset/27000));
	//OutputDebugString(text);

	iOldPCR = iPCR;

	wsprintf(text,"(M PCR) PID=%X, %dth PKT. %d msec (%d sec)\n",
					          wPID,
							  giNoPacket,
							  iPCR,
							  iPCR/1000);
	//OutputDebugString(text);
    **********************************************************/
}

//---------------------------------------------------------------------------

void __fastcall TFormMain::BOKClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ILOGOClick(TObject *Sender)
{
    // About Box
    AboutBox->ShowModal();  
}
//---------------------------------------------------------------------------

