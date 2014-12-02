#ifndef _REG_VAR_H_
#define _REG_VAR_H_
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class CREG_VAR
{
private:

public:
    CREG_VAR();

public:
    //-----------------------------------------------------------
    void    modCommandLine(long nActiveBoardNum);
#ifndef WIN32
	int 	Get_StartUp_Value_From_Registry(int iType, int nSlot);	//2010/9/28 MULTIBOARD
#endif
    void    RestoreVariables(long nBoardNum);
    void    SaveVariables(long nBoardNum);
    void    SaveNewModulatorMode(long nBoardNum, long lModType);
    void    CheckGlobalVarValidity(long nBoardNum);
    void    Restore_Factory_Default();

	//2012/3/23
	void	GetInitSystemValue(int nBoardNum, char *BoardLocation, int nBoard_ID);

	//2012/10/22 system configuration
#ifdef WIN32
	void	GetSystemConfiguration();
	void	SetSystemConfiguration();
#endif
};

extern CREG_VAR* gc_reg_var;

#endif	//_REG_VAR_H_