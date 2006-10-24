/*
 *  Author:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 * 
 *  Copyright:
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

bool console = true;

int main(int argc, char **argv)
{
  char *cmdline = makeCmdLine(true);

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEOFFFEEDBACK|STARTF_USESTDHANDLES;
  SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE),
		       HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  PROCESS_INFORMATION pi;
  if (!CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)) {
    panic(true,"Cannot run '%s'.\n",cmdline);
  }
  CloseHandle(pi.hThread);

  if (WaitForSingleObject(pi.hProcess,INFINITE) == WAIT_FAILED) {
    panic(true,"Wait for subprocess failed.\n");
  }

  DWORD code;
  if (!GetExitCodeProcess(pi.hProcess,&code)) {
    panic(true,"Could not get process exit code.\n");
  }
  CloseHandle(pi.hProcess);

  return code;
}
