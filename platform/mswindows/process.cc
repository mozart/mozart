/*
 *  Authors:
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Ralf Scheidhauer, 1999
 *    Leif Kornstaedt, 1999
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 */

#include <windows.h>
#include <string.h>

#include "startup.hh"

static DWORD WINAPI readerThread(void *arg)
{
  HANDLE hRead = (HANDLE) arg;
  DWORD ret;
  char buffer[1024];
  while(1) {
    if (ReadFile(hRead,buffer,sizeof(buffer) - 1,&ret,0) == FALSE)
      break;
    buffer[ret] = 0;
    MessageBox(NULL,buffer,"Mozart Output",
               MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  }
  return 0;
}

int createProcess(char *cmdline)
{
  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  HANDLE hRead,hWrite;
  {
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa,sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (CreatePipe(&hRead,&hWrite,&sa,0) == FALSE) {
      panic(true,"Could not create pipe.\n");
    }
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    // Is it OK to pass the same handle twice?
    si.hStdOutput = hWrite;
    si.hStdError  = hWrite;
  }

  // Why make the pi.hProcess handle inheritable?
  SECURITY_ATTRIBUTES sa;
  ZeroMemory(&sa,sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;
  PROCESS_INFORMATION pi;
  DWORD ret = CreateProcess(NULL,cmdline,&sa,NULL,TRUE,
                            DETACHED_PROCESS | CREATE_SUSPENDED,
                            NULL,NULL,&si,&pi);

  if (ret == FALSE) {
    panic(true,"Cannot run '%s'.\n",cmdline);
  }
  CloseHandle(pi.hThread);

  DWORD thrid;
  CreateThread(0,10000,&readerThread,hRead,0,&thrid);
  ResumeThread(pi.hThread);

  WaitForSingleObject(pi.hProcess,INFINITE);

  DWORD code;
  ret = GetExitCodeProcess(pi.hProcess,&code);
  CloseHandle(pi.hProcess);
  if (ret != FALSE && code != 0)
    return code;
  else
    return 0;
}
