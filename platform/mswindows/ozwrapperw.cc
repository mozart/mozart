/*
 *  Authors:
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Ralf Scheidhauer and Leif Kornstaedt, 1999
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */


/*
 * windowed version of ozwrapper. 
 * Does something like
 *     "exec ozenginew $0 $@"
 */

#include <windows.h>
#include <process.h>
#include "misc.cc"

#define ISSPACE(a)	(a == ' ' || a == '\t')

int WINAPI
WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpszCmdLine*/, int /*nCmdShow*/)
{
  char buffer[5000];
  sprintf(buffer,"ozenginew.exe \"");
  int len = strlen(buffer);
  GetModuleFileName(NULL, buffer+len, sizeof(buffer)-len);
  strcat(buffer,"\" ");

  //
  // We don't use the command line we got as argument since
  // this is not correctly preprocessed by mingw32.
  //

  char *lpszCmdLine = GetCommandLine();
  // Skip whitespace and application name
  if (lpszCmdLine) {
    while (ISSPACE(*lpszCmdLine)) {
      lpszCmdLine++;
    }

    int isQuoted = 0;
    while (*lpszCmdLine != '\0' && (!ISSPACE(*lpszCmdLine) || isQuoted)) {
      if (*lpszCmdLine == '\"')
	isQuoted = !isQuoted;
      lpszCmdLine++;
    }
    strcat(buffer,lpszCmdLine);
  }

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,DETACHED_PROCESS,
			   NULL,NULL,&si,&pi);
  if (ret == FALSE) {
    panic(1,"Cannot run '%s'.\n",buffer);
  }

  return 0;
}
