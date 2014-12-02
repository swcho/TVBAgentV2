/******************************************************************************
******************************************************************************/
#include	<stdio.h>
#include	<time.h>
#include	<math.h>
#if defined(WIN32)
#include	<windows.h>
#include	<winbase.h>
#include	<string.h>
#include	<conio.h>
#else
#endif

//HLD Application
#include    "../include/hld_api.h"
#include	"../include/common_def.h"
#include	"../include/hld_const.h"

extern int AdjustBankOffset(int banknum, int bankoffset, int playrate);
extern int CheckPllLock(int nBoardID);//TVB590S, TVB593

//
//	Demonstrate to play one MPEG stream file
//
int	main(int argc, char *argv[])
{
	int		Modulator_Type = 8;//DVB_S2
	long    Playrate;
	int     Board_Id=-1;
	int     Repeat;
	int		Coding_Rate;
	int		Constellation;
	int		Pilot;
	long	RF_Freq, IF_Freq = 36000000;
	long 	Symbol_Rate;
	int		nSpectrum_Inversion = 0;

	long	Max_Playrate;
	double	dwCoderate;

	int		dwBufferSize;
	char	line[512];
	char	fileName[512];
	float 	Atten = 0.;
	int 	nPRBS_mode = 0;
	float 	nNoise_power = 0;
	int 	nPort = -1;//-1 : Play, 0 : Capture from 310M, 1 : Capture from ASI

	int		nRoll_off = 3;//none
	double	dwBitsPerSymbol = 0.;
	double	NumOfSLOT = 360.;
	double	PLFRAMING_effiency = 0.;
	double	Kbch, Nbch;
	
	int		nBoardTypeID;
	int 	nOptionalOutputAmp;
	int		nBypassAmp=0;//TVB595V1, 0 : Use AMP., 1 : Bypass AMP.

	int		nBankOffset;
	int		Restamping;//PCR, DTS/PTS
	int		ContinuityCounter;
	int		DateTimeOffset;//TDT/TOT

	//2012/4/18
	int DetectedBoardCnt;

	int i = 0;
	int cnt = 14;
#ifdef TVB390V8
	cnt += 3;
#endif
	cnt += 3;
	//2012/4/18
	DetectedBoardCnt = TSPH_ConfTvbSytem();

	if ( argc > 1 ) 
	{
		if ( argc < cnt )
		{
		#ifdef WIN32
			printf("Usage:hld_mod_dvb_s2.exe");
		#else
			printf("Usage:mod_dvb_s2");
		#endif
		
#ifdef TVB390V8
		#ifdef  WIN32
			printf("\nboard slot#(1,2,3,...,(TVB595:23,..,20))");
		#else
			printf("\nboard id(1,2,3,...)");
		#endif
			printf("\ninput_source(File : 0, 310M : 1, ASI : 2)");
#endif
			printf("\nfile_name");
			printf("\nrepeat_mode");
			printf("\nplayrate");
			printf("\nconstellation(QPSK:0, 8PSK:1)");
			printf("\ncode rate(1/4:0, 1/3:1, 2/5:2, 1/2:3, 3/5:4, 2/3:5, 3/4:6, 4/5:7, 5/6:8, 8/9:9, 9/10:10)");
			printf("\npilot(Off:0, On:1)");
			printf("\nsymbol rate(1000000~45000000, Symbol/sec)");
			printf("\nRF(Hz)");
			printf("\nIF(36000000 or 44000000 Hz)");
			printf("\nattenuation(0~31.5, +/-0.1 dB, default:17dB)"); 
			printf("\nnoise_mode(none : 0, 2^7-1 : 1, 2^10-1 : 2, 2^15-1 : 3, 2^23-1 : 4)"); 
			//printf("\nnoise_power(-70.0 ~ 20.0, +/-0.1 dB)");
			printf("\nC/N(0.0 ~ 50.0, +/-0.1 dB), noise_mode must be 1~4");
			printf("\nroll-off factor(0.20 : 0, 0.25 : 1, 0.35 : 2, none : 3)");
			printf("\nuse AMP. only for TVB595, TVB590V9.2 or higher(yes : 0, no : 1)");
			printf("\nloop adaptation(PCR, PTS/DTS)(no : 0, yes : 1)");
			printf("\n(continuity counter)(no : 0, yes : 1)");
			printf("\n(TDT/TOT)(no : 0, yes : 1, use current time : 2)");
			return 0;
		}

#ifdef TVB390V8
		i = 1;
		Board_Id = atoi(argv[i]);
		nPort = atoi(argv[++i]);
		nPort -= 1;
#endif	
		sprintf(fileName, "%s", argv[++i]);
		Repeat = atoi(argv[++i]);
		Playrate = atol(argv[++i]);
		Constellation = atoi(argv[++i]);
		Coding_Rate = atoi(argv[++i]);
		Pilot = atoi(argv[++i]);
		Symbol_Rate = atol(argv[++i]);
		RF_Freq = atol(argv[++i]);
		IF_Freq = atol(argv[++i]);
		Atten = (float)atof(argv[++i]);
		nPRBS_mode = atoi(argv[++i]);
		nNoise_power = (float)atof(argv[++i]);
		nRoll_off = atoi(argv[++i]);
		nBypassAmp = atoi(argv[++i]);
		Restamping = atoi(argv[++i]);;
		ContinuityCounter = atoi(argv[++i]);
		DateTimeOffset = atoi(argv[++i]);
	}
	else
	{

		printf("---------------------------------------------------\n");
#ifdef TVB390V8
		//2012/4/18
		printf("=== Installed Board List ===\n");
		for(i = 0; i < DetectedBoardCnt; i++)
		{
			printf("%d : %s\n", i, TSPH_GetBdName_N(i));
		}

		printf("Select board (0 ~ %d) : ", (DetectedBoardCnt - 1));
//	#if defined(WIN32)
//		printf("Board Slot#(1,2,3,...,(TVB595:23,..,20)) : ");
//	#else
//		printf("Board ID(1,2,3,...) : ");
//	#endif
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &Board_Id);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Board_Id = 0;
		}
		if ( Board_Id < 0 )
		{
			printf("Invalid board ID.\n");
			return 0;
		}
		
		printf("Input source(File : 0, 310M : 1, ASI : 2) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &nPort);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			nPort = 0;
		}
		if ( nPort < 0 || nPort > 2 )
		{
			printf("Invalid input source.\n");
			return 0;
		}
		nPort -= 1;
#endif
		if ( nPort < 0 )
			printf("File name to play : ");
		else
			printf("File name to record : ");
		fgets(fileName, sizeof(fileName), stdin);
		/* Removing the '\n' character from the end */
		dwBufferSize = strlen(fileName)-1;
		fileName[dwBufferSize] = '\0';
		if ( strlen(fileName) == 0 )
		{
			printf("Invalid file name.\n");
			return 0;
		}

		/* Play Mode Paramters */
		if ( nPort < 0 )
		{
			printf("Repeat mode(Once:0, Loop:1) : ");
			fgets(line, sizeof(line), stdin);
			sscanf(line, "%d", &Repeat);
			dwBufferSize = strlen(line)-1;
			line[dwBufferSize] = '\0';
			if ( strlen(line) == 0 )
			{
				Repeat = 1;
			}
			if ( Repeat < 0 )
			{
				printf("Invalid repeat mode.\n");
				return 0;
			}
			
			printf("Play rate(bps) : ");
			fgets(line, sizeof(line), stdin);
			sscanf(line, "%ld", &Playrate);
			dwBufferSize = strlen(line)-1;
			line[dwBufferSize] = '\0';
			if ( strlen(line) == 0 )
			{
				Playrate = 19392658;
			}
			if ( Playrate < 0 || Playrate > 90000000 )
			{
				printf("Invalid play rate.\n");
				return 0;
			}
		}	
		/* Record Mode Paramters */
		else
		{
		}

		/* Modulator Paramters */
		printf("Constellation(QPSK:0, 8PSK:1, 16APSK:2, 32APSK:3) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &Constellation);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Constellation = 0;
		}
		if ( Constellation < 0 || Constellation  > 3 )
		{
			printf("Invalid constellation.\n");
			return 0;
		}

		printf("Code rate(1/4:0, 1/3:1, 2/5:2, 1/2:3, 3/5:4, 2/3:5, 3/4:6, 4/5:7, 5/6:8, 8/9:9, 9/10:10) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &Coding_Rate);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Coding_Rate = 0;
		}
		if ( Coding_Rate < 0 || Coding_Rate  > 10 )
		{
			printf("Invalid code rate.\n");
			return 0;
		}

		printf("Pilot(Off:0, On:1) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &Pilot);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Pilot = 0;
		}
		if ( Pilot < 0 || Pilot  > 1 )
		{
			printf("Invalid pilot.\n");
			return 0;
		}

		printf("Symbol rate(Sps)(1000000 ~ 45000000) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%ld", &Symbol_Rate);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Symbol_Rate = 1000000;
		}
		if ( Symbol_Rate < 1000000	|| Symbol_Rate > 45000000 )
		{
			printf("Invalid symbol rate.\n");
			return 0;
		}

		printf("RF freq.(Hz) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%ld", &RF_Freq);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			RF_Freq = 1000000000;
		}
		if ( RF_Freq < 0 )
		{
			printf("Invalid RF frequency.\n");
			return 0;
		}
		
		printf("IF freq.(Hz)(36000000 or 44000000): ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%ld", &IF_Freq);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			IF_Freq = 36000000;
		}
		if ( IF_Freq != 36000000 && IF_Freq != 44000000 )
		{
			printf("Invalid IF frequency.\n");
			return 0;
		}

		printf("Attenuation(dB)(0.0~31.5, +/-0.1) : (default:17.0dB)");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%f", &Atten);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Atten = 17.0;
		}
		if ( Atten < 0 || Atten > 31.5)
		{
			printf("Invalid attenuation value.\n");
			return 0;
		}

		printf("Noise mode(none : 0, 2^7-1 : 1, 2^10-1 : 2, 2^15-1 : 3, 2^23-1 : 4) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &nPRBS_mode);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			nPRBS_mode = 0;
		}
		if ( nPRBS_mode < 0 || nPRBS_mode > 4 )
		{
			printf("Invalid noise mode.\n");
			return 0;
		}

		//printf("Noise power(dB)(-70.0 ~ 20.0, +/-0.1) : ");
		printf("C/N(dB)(0.0 ~ 50.0, +/-0.1, Noise mode must be 1~4) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%f", &nNoise_power);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			//nNoise_power = -70.0;
			nNoise_power = 50.0;
		}
		//if ( nNoise_power < -70.0 || nNoise_power > 20.0 )
		if ( nNoise_power < 0.0 || nNoise_power > 50.0 )
		{
			//printf("Invalid noise power.\n");
			printf("Invalid C/N.\n");
			return 0;
		}

		printf("Roll-off factor(0.20 : 0, 0.25 : 1, 0.35 : 2, none : 3) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &nRoll_off);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			nRoll_off = 3;
		}
		if ( nRoll_off < 0 || nRoll_off > 3 )
		{
			printf("Invalid Roll-off factor.\n");
			return 0;
		}

		printf("Use AMP. only for TVB595, TVB590V9.2 or higher(yes : 0, no : 1) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &nBypassAmp);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			nBypassAmp = 0;
		}
		if ( nBypassAmp < 0 || nBypassAmp > 1 )
		{
			printf("Invalid parameter.\n");
			return 0;
		}

		printf("Loop adaptation(PCR, PTS/DTS)(no : 0, yes : 1) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &Restamping);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			Restamping = 0;
		}
		if ( Restamping < 0 || Restamping > 1 )
		{
			printf("Invalid parameter.\n");
			return 0;
		}

		printf("(Continuity Counter)(no : 0, yes : 1) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &ContinuityCounter);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			ContinuityCounter = 0;
		}
		if ( ContinuityCounter < 0 || ContinuityCounter > 1 )
		{
			printf("Invalid parameter.\n");
			return 0;
		}

		printf("(TDT/TOT)(no : 0, yes : 1, use current time : 2) : ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &DateTimeOffset);
		dwBufferSize = strlen(line)-1;
		line[dwBufferSize] = '\0';
		if ( strlen(line) == 0 )
		{
			DateTimeOffset = 0;
		}
		if ( DateTimeOffset < 0 || DateTimeOffset > 2 )
		{
			printf("Invalid parameter.\n");
			return 0;
		}

		printf("---------------------------------------------------\n");
	}
	//2012/4/18
	for(i = 0; i < DetectedBoardCnt; i++)
	{
		if(i == Board_Id)
		{
			TSPH_InitOneRealBd(Board_Id);
			Sleep(10);
			TSPH_ActivateOneBd(Board_Id, Modulator_Type, 36000000);
			Sleep(10);
		}
		else
		{
			TSPH_CloseOneRealBd(i);
		}
	}

	/* check play rate is valid */
	if (Coding_Rate == 0)
	{
        dwCoderate = 1. / 4.;
        Kbch = 16008.;
        Nbch = 16200.;
	}
    else if (Coding_Rate == 1)
	{
        dwCoderate = 1. / 3.;
        Kbch = 21408.;
        Nbch = 21600.;
	}
    else if (Coding_Rate == 2)
	{
        dwCoderate = 2. / 5.;
        Kbch = 25728.;
        Nbch = 25920.;
	}
    else if (Coding_Rate == 3)
	{
        dwCoderate = 1. / 2.;
        Kbch = 32208.;
        Nbch = 32400.;
	}
    else if (Coding_Rate == 4)
	{
        dwCoderate = 3. / 5.;
        Kbch = 38688.;
        Nbch = 38880.;
	}
    else if (Coding_Rate == 5)
	{
        dwCoderate = 2. / 3.;
        Kbch = 43040.;
        Nbch = 43200.;
	}
    else if (Coding_Rate == 6)
	{
        dwCoderate = 3. / 4.;
        Kbch = 48408.;
        Nbch = 48600.;
	}
    else if (Coding_Rate == 7)
	{
        dwCoderate = 4. / 5.;
        Kbch = 51648.;
        Nbch = 51840.;
	}
    else if (Coding_Rate == 8)
	{
        dwCoderate = 5. / 6.;
        Kbch = 53840.;
        Nbch = 54000.;
	}
    else if (Coding_Rate == 9)
	{
        dwCoderate = 8. / 9.;
        Kbch = 57472.;
        Nbch = 57600.;
	}
    else
	{
        dwCoderate = 9. / 10.;
        Kbch = 58192.;
        Nbch = 58320.;
    }

	dwBitsPerSymbol = 2. + Constellation;
    if (dwBitsPerSymbol == 2.)
        NumOfSLOT = 360.;
	else if (dwBitsPerSymbol == 3.)
        NumOfSLOT = 240.;
    else if (dwBitsPerSymbol == 4.)
        NumOfSLOT = 180.;
    else
        NumOfSLOT = 144.;
                        
    if ( Pilot == 0 ) //Off
        PLFRAMING_effiency = (90. * NumOfSLOT) / (90. * (NumOfSLOT + 1));
    else
        PLFRAMING_effiency = (90. * NumOfSLOT) / ((90. * (NumOfSLOT + 1) + (36. * floor((NumOfSLOT - 1.) / 16.))));
                        
    Max_Playrate = (long)(Symbol_Rate * dwBitsPerSymbol * dwCoderate * PLFRAMING_effiency * ((Kbch - 80.) / Nbch));

	printf("!!!!Maximum Play Rate(bps) is [%ld]\n", Max_Playrate);
	if (Playrate > Max_Playrate)
	{
		return 0;
	}

	/* F/W downlaod */
//	if (TVB380_SET_CONFIG_EX(Board_Id, Modulator_Type, IF_Freq) < 0)
//	{
//		printf("!!!! Board initialization failed. 'RBF' sub-directory must be at the working directory.\n");
//		return 0;
//	}

	/* check the modulator permission */
	if ( TVB380_IS_ENABLED_TYPE_EX(Board_Id, Modulator_Type) == 0 )
	{
		printf("!!!! Modulator is not  enabled. Check the board installation or the license information(or license.dat).\n");
		return 0;
	}

	/* read the board ID */
	nBoardTypeID = TSPL_GET_BOARD_ID_EX(Board_Id);
	
	/* TVB590V9.2, TVB590V10, TVB595V2, TVB595V3, TVB595V4, TVB597A, TVB590S, TVB593 */
	/* read the board's optional amplifier information */
	if ( nBoardTypeID == 0x2F
		|| nBoardTypeID == 0x30 
		|| nBoardTypeID == 0x3B
		|| nBoardTypeID == 0x3C	
		|| nBoardTypeID == 0x3D
		|| nBoardTypeID == 0x0A
		|| nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C
		|| nBoardTypeID == 0x16 ) //TVB591S 
	{
		if ( nBypassAmp == 0 )
			printf("Output level option is 0dBm. The default atten(17dBm) may be required.\n");
	}
	else
	{
		nOptionalOutputAmp = TSPL_GET_BOARD_CONFIG_STATUS_EX(Board_Id);
		if ( nOptionalOutputAmp == 1 )
			printf("Output level option is -6dBm. The default atten(14dBm) may be required.\n");
		else if ( nOptionalOutputAmp == 2 )
			printf("Output level option is 0dBm. The default atten(17dBm) may be required.\n");
	}
	
	TSPL_RESET_SDCON_EX(Board_Id);
	Sleep(100);

	/* set modulator parameters */
	TVB380_SET_MODULATOR_IF_FREQ_EX(Board_Id, Modulator_Type, IF_Freq);
	TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(Board_Id, Modulator_Type, nSpectrum_Inversion);
	TVB380_SET_MODULATOR_FREQ_EX(Board_Id, Modulator_Type, RF_Freq, Symbol_Rate);
	TVB380_SET_MODULATOR_SYMRATE_EX(Board_Id, Modulator_Type, RF_Freq, Symbol_Rate);
	TVB380_SET_MODULATOR_CODERATE_EX(Board_Id, Modulator_Type, Coding_Rate);
	TVB380_SET_MODULATOR_CONSTELLATION_EX(Board_Id, Modulator_Type, Constellation);

	//TVB390V7, V8 or higher
	if ( nBoardTypeID >= 0x2C 
		|| nBoardTypeID == 0x0A
		|| nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C
		|| nBoardTypeID == 0x16 ) //TVB591S 
	{
		TVB380_SET_MODULATOR_ATTEN_VALUE_EX(Board_Id, Modulator_Type, Atten, 0);
		TVB380_SET_MODULATOR_PRBS_INFO_EX(Board_Id, Modulator_Type, nPRBS_mode, nNoise_power);

		TVB380_SET_MODULATOR_PILOT_EX(Board_Id, Modulator_Type, (Pilot+1)%2);
		TVB380_SET_MODULATOR_ROLL_OFF_FACTOR_EX(Board_Id, Modulator_Type, nRoll_off);
	}

	/* TVB590V9.2, TVB590V10, TVB595V2, TVB595V3, TVB595V4, TVB597A, TVB590S, TVB593 */
	if ( nBoardTypeID == 0x2F
		|| nBoardTypeID == 0x30 
		|| nBoardTypeID == 0x3B
		|| nBoardTypeID == 0x3C	
		|| nBoardTypeID == 0x3D
		|| nBoardTypeID == 0x0A
		|| nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C
		|| nBoardTypeID == 0x16 ) //TVB591S 
	{
		TVB380_SET_BOARD_CONFIG_STATUS_EX(Board_Id, Modulator_Type, nBypassAmp);
	}

	/* TVB590V10, TVB595V3, TVB595V4, TVB597A, TVB590S, TVB593 */
	/* Set SDRAM clock mode, 0=USE SYSTEM CLOCK, 1=USE SYMBOL CLOCK */
	if ( nBoardTypeID == 0x30 
		|| nBoardTypeID == 0x3C	
		|| nBoardTypeID == 0x3D
		|| nBoardTypeID == 0x0A
		|| nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C
		|| nBoardTypeID == 0x16 ) //TVB591S 
	{
		TVB380_SET_MODULATOR_SDRAM_CLOCK_EX(Board_Id, Modulator_Type, 0);
	}

	/* TVB590S, TVB593,  called after TVB380_SET_MODULATOR_SYMRATE */
	if ( nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x16
		|| nBoardTypeID == 0x0C) //TVB591S 
	{
		TVB380_SET_MODULATOR_SPECTRUM_INVERSION_EX(Board_Id, Modulator_Type, nSpectrum_Inversion);
	}

    /* set board configuration for playing */
	nBankOffset = AdjustBankOffset(HW_BANK_NUMBER, SUB_BANK_OFFSET_SIZE, Playrate);
	/* record */
	if ( nPort == 0 || nPort == 1 )
	{
		nBankOffset = SUB_BANK_OFFSET_SIZE;
	}

	printf("Adjusted bank offset=%d\n", nBankOffset);
	//if ( TSPH_SET_SDRAM_BANK_INFO(Board_Id, HW_BANK_NUMBER, SUB_BANK_OFFSET_SIZE) == -1 )
	if ( TSPH_SET_SDRAM_BANK_INFO(Board_Id, HW_BANK_NUMBER, nBankOffset) == -1 )
		printf("FAIL: can not set SDRAM BANK info, unstable board status\n");

	if (TSPL_RESET_SDCON_EX(Board_Id) == -1)
		printf("FAIL: fail to reset SDCON, REBOOT the system\n");

	if (TSPL_SET_SDCON_MODE_EX(Board_Id, TSPL_SDCON_LOOP_THRU_MODE) == -1)
		printf("FAIL: can not set SDCON mode, unstable board status\n");	

#ifdef TVB390V8
	if (TSPL_SET_TSIO_DIRECTION_EX(Board_Id, TSIO_PLAY_WITH_ASIINPUT) == -1)
		printf("FAIL: can not set TSIO direction, unstable board status\n");	
#endif

	/* TVB595V2, TVB595V3, TVB595V4, TVB597A */
	if ( nBoardTypeID == 0x3B
		|| nBoardTypeID == 0x3C	
		|| nBoardTypeID == 0x3D
		|| nBoardTypeID == 0x0A
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C)	//2011/6/21 TVB597A V2
	{
		TSPL_SET_FIFO_CONTROL_EX(Board_Id, 
			(nPort == 0 || nPort == 1) ? 1/* RECORD */ : 0 /* PLAY */,
			nBankOffset<<10/* DMA BLOCK SIZE*/);
	}

	/* TVB590S, TVB593 */
	/* Check PLL lock status */
	if ( nBoardTypeID == 0x14 
		|| nBoardTypeID == 0x0F
		|| nBoardTypeID == 0x10
		|| nBoardTypeID == 0x0B
		|| nBoardTypeID == 0x0C
		|| nBoardTypeID == 0x16 ) //TVB591S 
	{
		if ( !CheckPllLock(Board_Id) )
		{
			printf("ERROR: Invalid PLL lock status.\n");
			return 0;
		}
	}

	TSPH_START_MONITOR(Board_Id, 0);
	Sleep(500);
	do 
	{
		//Record
		if ( nPort == 0 || nPort == 1 )
		{
			TSPH_START_RECORD(Board_Id, fileName, nPort+1);
		}
		//Play
		else
		{
			TSPH_SET_LOOP_ADAPTATION(Board_Id, Restamping, ContinuityCounter, DateTimeOffset, "", "");

			TSPH_START_PLAY(Board_Id, fileName, Playrate, 0, 0, Repeat);
			if ( Repeat < 1 )
			{
				printf("PRESS another key to end\n");
				while(!_kbhit())
				{
					if ( TSPH_GET_CURRENT_THREAD_STATE(Board_Id) == 8/*TH_END_PLAY*/ )
						break;
					Sleep(10);
				}
				break;
			}
		}

		printf("PRESS another key to end\n");
		i = _getch();	// wait to finish job

	} while ((i == 'c') || (i == 'C'));

	TSPH_START_MONITOR(Board_Id, 0);
	Sleep(1000);

	return 0;
}
//------------- ends here -------------------
