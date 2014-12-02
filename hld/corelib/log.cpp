#include <stdio.h>

#if defined(WIN32)
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#endif


#include "log.h"


tlv_log *tlv_log::get_instance()
{
	static tlv_log *_log = 0;
	if (_log == 0)
	{
		_log = new tlv_log();
	}
	return _log;
}

tlv_log::tlv_log()
{
#ifdef WIN32_CONSOLE

	long lStdHandle;
	
	// allocate a console for this app
	AllocConsole();

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle,  O_TEXT);

	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );


	// redirect unbuffered STDIN to the console
	long lStdHandle_in;
	lStdHandle_in = (long)GetStdHandle(STD_INPUT_HANDLE);

	lStdHandle_in = _open_osfhandle(lStdHandle_in, _O_TEXT);

	fp = _fdopen( lStdHandle_in, "r" );

	*stdin = *fp;

	setvbuf( stdin, NULL, _IONBF, 0 );



#endif
}
tlv_log::~tlv_log()
{
#ifdef WIN32_CONSOLE
	fclose(fp);
//	_close(hConHandle);	
#endif	
}

void tlv_log::init()
{

}


void tlv_log::quit()
{
}

void tlv_log::print(char *log_mgs)
{

#if defined(WIN32)
#ifdef WIN32_CONSOLE
	printf(log_mgs);
#else
	OutputDebugString(log_mgs);
#endif
#else // linux
	printf(log_mgs);
#endif
}

