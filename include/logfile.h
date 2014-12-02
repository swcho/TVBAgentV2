
#ifndef	__LOG_FILE_DEF_H_
#define	__LOG_FILE_DEF_H_

//
//	Log file declaration
//
//	June 14 2001
//	Teleview Corporation
//

#ifdef WIN32

#ifdef	_DEBUG
#define	LPRINT			if (fn_Log != NULL) fprintf
#else
#define	LPRINT
#endif


#else	//	LINUX

#define	_DEBUG

extern	FILE	*fn_Log;
//	#define LOG_IN_STDERR
#define LLD_DEBUG

#ifdef LLD_DEBUG
#define LPRINT  fprintf
#else
#define	LPRINT
#endif
#endif

#endif //__LOG_FILE_DEF_H_ 

