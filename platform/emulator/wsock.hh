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


#ifdef WINDOWS

#define NOMINMAX
#define Bool WinBool

#include <windows.h>
#undef FAILED /* used in oz.h as well */

#undef Bool

#ifdef GNUWIN32

#include <sys/times.h>
#include <fcntl.h>

typedef HINSTANCE HMODULE;
#define CreateEvent  CreateEventA
extern "C" {
  int WINAPI closesocket(int s);
  int WINAPI send(int s, const char *buf, int len, int flags);
  int WINAPI recv(int s, char * buf, int len, int flags);
  
  int WINAPI WSAGetLastError(void);
  int WINAPI socket(int af, int type, int protocol);

  BOOL WINAPI SetStdHandle(DWORD nStdHandle, HANDLE hHandle);
#define timeGetTime() 0
#define GetModuleHandle  GetModuleHandleA
  HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);
  BOOL WINAPI FreeLibrary(HINSTANCE hLibModule);
  FARPROC WINAPI GetProcAddress(HINSTANCE hModule,LPCSTR lpProcName);

  inline void _endthreadex( unsigned __retval )
  {
    /* empty for now */
  }

#define _beginthreadex(security, stack_size,fun,args,initflag,thrdaddr) \
  CreateThread(security,stack_size,(void(*)(void*))(fun),args,initflag,thrdaddr);

}

#endif

#endif


