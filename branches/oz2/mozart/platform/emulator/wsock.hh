/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: $Author$
  Last modified: $Date$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

/* "windows.h" defines some constants, that are also used in Oz,
 * so this file MUST BE INCLUDED BEFORE ANY OTHER FILE
 */


#ifndef __WSOCK_H__
#define __WSOCK_H__

#ifdef WINDOWS

#define NOMINMAX
#define Bool WinBool
#define min winmin
#define max winmax

#include <windows.h>

#undef winmin
#undef winmax

#undef FAILED /* used in oz.h as well */
#undef Bool

#ifdef GNUWIN32

/* The headers of gnu win32 are incomplete: */

/* do not redefine FD_* macros, have to use win32 versions */
#define _POSIX_SOURCE  
#include <sys/types.h>
#undef _POSIX_SOURCE


#include <sys/times.h>
#include <fcntl.h>

typedef HINSTANCE HMODULE;
#define CreateEvent  CreateEventA
extern "C" {

typedef void *PVOID;
#include "winsock.h"

#ifdef OLDCYGGNUS

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

  BOOL WINAPI SetStdHandle(DWORD nStdHandle, HANDLE hHandle);
#define GetModuleHandle  GetModuleHandleA
  HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);
  BOOL WINAPI FreeLibrary(HINSTANCE hLibModule);
  FARPROC WINAPI GetProcAddress(HINSTANCE hModule,LPCSTR lpProcName);

typedef struct _OSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];       // Maintenance string for PSS usage
} OSVERSIONINFO, *POSVERSIONINFO, *LPOSVERSIONINFO;

BOOL WINAPI GetVersionExW(LPOSVERSIONINFO lpVersionInformation);
#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2

#endif

  inline void _endthreadex( unsigned __retval )
  {
    /* empty for now */
  }

#define _beginthreadex(security, stack_size,fun,args,initflag,thrdaddr) \
  CreateThread(security,stack_size,fun,args,initflag,thrdaddr);

}


#endif

#endif

#endif


