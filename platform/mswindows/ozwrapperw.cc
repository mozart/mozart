/*
 *  Authors:
 *    Ralf Scheidhauer (scheidhr@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

int PASCAL
WinMain(HANDLE /*hInstance*/, HANDLE /*hPrevInstance*/,
	LPSTR lpszCmdLine, int /*nCmdShow*/)
{
  char buffer[5000];
  sprintf(buffer,"ozenginew.exe ");
  int len = strlen(buffer);
  GetModuleFileName(NULL, buffer+len, sizeof(buffer)-len);

  strcat(buffer," ");
  strcat(buffer,lpszCmdLine);

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pinf;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,DETACHED_PROCESS,
			   NULL,NULL,&si,&pinf);
  if (ret!=TRUE) {
    OzPanic(1,"Cannot run '%s' Oz.\nError = %d.\nDid you run setup?",buffer,errno);
    exit(1);
  }
  exit(0);
}
