#include <stdio.h>
#include <stdlib.h>

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned char       byte;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL             *LPBOOL;
typedef BYTE            *PBYTE;
typedef BYTE             *LPBYTE;
typedef int             *PINT;
typedef int              *LPINT;
typedef WORD            *PWORD;
typedef WORD             *LPWORD;
typedef long             *LPLONG;
typedef DWORD           *PDWORD;
typedef DWORD            *LPDWORD;
typedef void             *LPVOID;

typedef char TCHAR;
typedef TCHAR* LPTSTR;
typedef const TCHAR* LPCTSTR;
typedef void* HANDLE;

#define wsprintf sprintf

#ifndef NULL
	#define NULL 0L
#endif
#define FALSE 0
#define TRUE 1

#define TEXT(c) (c)

#define ERROR_INVALID_DATA 101
#define ERROR_NOT_ENOUGH_MEMORY 102
#define ERROR_INVALID_PASSWORD 103

static void SetLastError(DWORD dwErrCode)
{
	(dwErrCode);
}

static void FatalError(int errCode, LPCTSTR msg)
{
	fprintf(stderr, msg);
	exit(errCode);
}
