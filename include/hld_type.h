
#ifndef	__HLD_TYPE_H__
#define	__HLD_TYPE_H__

#include	"../include/hld_const.h"

#if defined(WIN32)
#else

#define HINSTANCE void*
#define HANDLE void*
#define WINAPI 

#define _declspec(dllexport)
#define _stdcall 

typedef void VOID;
typedef VOID* PVOID;
typedef VOID* LPVOID;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef int INT;
typedef INT* PINT;
typedef long LONG;
typedef unsigned long ULONG;
typedef long WORD;
typedef unsigned int DWORD;
typedef DWORD* PDWORD;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef CHAR* PCHAR;
typedef UCHAR* PUCHAR;
typedef double DOUBLE;
typedef int BOOL;
typedef char* LPCTSTR;
typedef long long INT64;
typedef long long __int64;
typedef unsigned int UINT;

#endif

typedef void *(WINAPI* FTYPE_PVOID_VOID)(void);
typedef INT   (WINAPI* FTYPE_INT_VOID)(void);
typedef void  (WINAPI* FTYPE_VOID_PDWORD_DWORD_PDWORD)(PDWORD,DWORD,PDWORD);
typedef INT   (WINAPI* FTYPE_INT_LONG)(LONG);
typedef INT   (WINAPI* FTYPE_INT_LONG_LONG)(LONG,LONG);
typedef INT   (WINAPI* FTYPE_INT_INT)(INT);
typedef DWORD (WINAPI* FTYPE_DWORD_VOID)(void);
typedef LONG  (WINAPI* FTYPE_LONG_LONG)(LONG);
typedef LONG  (WINAPI* FTYPE_LONG_VOID)(void);
typedef INT   (WINAPI* FTYPE_INT_INT_INT)(INT,INT);
typedef INT   (WINAPI* FTYPE_INT_BYTE_PBYTE_INT_PINT)(BYTE,PBYTE,INT,PINT);
typedef INT   (WINAPI* FTYPE_INT_BYTE_INT_PBYTE_INT_PINT)(BYTE,INT,PBYTE,INT,PINT);
typedef INT   (WINAPI* FTYPE_INT_INT_INT_INT_INT)(INT,INT,INT,INT);
typedef INT   (WINAPI* FTYPE_INT_INT_INT_INT_LONG_LONG)(INT,INT,INT,LONG,LONG);
typedef INT   (WINAPI* FTYPE_INT_INT_LONG_LONG)(INT,LONG,LONG);
typedef INT   (WINAPI* FTYPE_INT_PCHAR)(PCHAR);
typedef INT   (WINAPI* FTYPE_INT_DWORD_PDWORD) (DWORD, PDWORD);
typedef void *(WINAPI* FTYPE_PVOID_LONG)(LONG);
typedef INT   (WINAPI* FTYPE_INT_LONG_LONG_LONG)(LONG,LONG,LONG);
typedef INT   (WINAPI* FTYPE_INT_LONG_LONG_LONG_LONG)(LONG,LONG,LONG,LONG);
typedef INT   (WINAPI* FTYPE_INT_LONG_DOUBLE)(LONG,double);
typedef INT   (WINAPI* FTYPE_INT_INT_INT_INT_INT_INT)(INT,INT,INT,INT,INT);
typedef INT   (WINAPI* FTYPE_INT_INT_INT_INT_INT_INT_INT)(INT,INT,INT,INT,INT,INT);
typedef INT   (WINAPI* FTYPE_INT_INT_INT_PUCHAR_INT_INT_INT)(INT,INT,PUCHAR,INT,INT,INT);
typedef INT   (WINAPI* FTYPE_INT_PUCHAR_PUCHAR)(PUCHAR,PUCHAR);
typedef INT   (WINAPI* FTYPE_INT_PUCHAR_PUCHAR_PINT_PINT_PINT_INT)(PUCHAR,PUCHAR,PINT,PINT,PINT,INT);
typedef INT   (WINAPI* FTYPE_INT_PUCHAR_PUCHAR_INT)(PUCHAR,PUCHAR,INT);
typedef INT   (WINAPI* FTYPE_INT_INT_DWORD_DWORD)(INT,DWORD,DWORD);
typedef INT   (WINAPI* FTYPE_INT_LONG_LONG_DOUBLE)(LONG,LONG,double);
typedef INT   (WINAPI* FTYPE_ULONG_INT_ULONG)(INT,LONG);
typedef INT	(WINAPI* FTYPE_INT_LONG_ULONG)(LONG, ULONG);
typedef INT	(WINAPI* FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG)(LONG,LONG,LONG,LONG,LONG,LONG);
typedef DOUBLE   (WINAPI* FTYPE_DOUBLE_LONG_LONG)(LONG,LONG);
typedef DOUBLE   (WINAPI* FTYPE_DOUBLE_LONG_DOUBLE)(LONG,DOUBLE);
typedef INT	(WINAPI* FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG)(LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG);
typedef DOUBLE	(WINAPI* FTYPE_DOUBLE_VOID)(void);
typedef UCHAR	(WINAPI* FTYPE_UCHAR_INT)(INT);
typedef INT		(WINAPI* FTYPE_INT_PVOID)(PVOID);
typedef PCHAR	(WINAPI* FTYPE_PCHAR_INT_PINT_PINT_PINT)(INT,PINT,PINT,PINT);
typedef INT		(WINAPI* FTYPE_INT_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG_LONG)(LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG,LONG);
typedef INT		(WINAPI* FTYPE_INT_LONG_LONG_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR_LONG_PCHAR)(LONG,LONG,LONG,PCHAR,LONG,PCHAR,LONG,PCHAR,LONG,PCHAR);
typedef LONG	(WINAPI* FTYPE_LONG_LONG_PLONG)(LONG, LONG*);
typedef INT		(WINAPI* FTYPE_INT_INT_PCHAR)(INT, PCHAR);
typedef INT	  (WINAPI* FTYPE_INT_LONG_ULONG_LONG)(LONG,ULONG,LONG);
typedef INT	  (WINAPI* FTYPE_INT_LONG_LONG_ULONG)(LONG,LONG,ULONG);
typedef INT	  (WINAPI* FTYPE_INT_PUCHAR_INT)(PUCHAR,INT);
typedef INT	  (WINAPI* FTYPE_INT_PDWORD_ULONG_PDWORD)(PDWORD,ULONG,PDWORD);
typedef INT	  (WINAPI* FTYPE_INT_PCHAR_ULONG_ULONG_INT_INT_INT_INT_INT)(PCHAR,ULONG,ULONG,INT,INT,INT,INT,INT);
typedef void  (WINAPI* FTYPE_VOID_VOID)(void);
typedef INT	  (WINAPI* FTYPE_INT_PCAHR_INT_INT)(PCHAR,INT,INT);
typedef INT	  (WINAPI* FTYPE_INT_ULONG_ULONG_ULONG_INT_INT_INT)(ULONG,ULONG,ULONG,INT,INT,INT);
typedef INT	  (WINAPI* FTYPE_INT_PCHAR_UINT_UINT)(PCHAR,UINT,UINT);
typedef INT	  (WINAPI* FTYPE_VOID_INT)(INT);
//2011/6/29 added UseTAT4710
typedef INT   (WINAPI* FTYPE_INT_LONG_DOUBLE_LONG)(LONG,double,LONG);
typedef DOUBLE   (WINAPI* FTYPE_DOUBLE_LONG_DOUBLE_LONG)(LONG,double,LONG);		//2011/6/29 added UseTAT4710
typedef INT 	(WINAPI* FTYPE_INT_INT_INT_PVOID)(int,int,void *);
typedef void *(WINAPI* FTYPE_PVOID_INT)(int);
typedef INT 	(WINAPI* FTYPE_INT_PCHAR_INT)(char *, int);
typedef INT		(WINAPI* FTYPE_INT_INT_PVOID)(int, PVOID);
//2012/8/29 new rf control
typedef INT		(WINAPI* FTYPE_INT_LONG_DOUBLE_PLONG_LONG)(long, double, long *, long);
typedef INT		(WINAPI* FTYPE_INT_LONG_PDOUBLE_PDOUBLE_LONG)(long, double *, double *, long);
typedef INT		(WINAPI* FTYPE_INT_PINT_PINT)(int *, int *);


#endif
