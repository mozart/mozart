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
  publishPid();

  char *cmdline = makeCmdLine(true);

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  BOOL ret = CreateProcess(NULL,cmdline,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
  if (ret == FALSE) {
    panic(true,"Cannot run '%s'.\n",cmdline);
  }
  WaitForSingleObject(pi.hProcess,INFINITE);

  DWORD code;
  if (GetExitCodeProcess(pi.hProcess,&code) != FALSE)
    return code;
  else
    return 0;
}
