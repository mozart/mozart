/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *    Ralf Scheidhauer <scheidhr@dfki.de>
 *
 *  Copyright:
 *    Leif Kornstaedt, 1999
 *    Ralf Scheidhauer, 1999
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
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "startup.hh"

bool console = false;

static char *getEmacsHome()
{
  char *ehome = getRegistry("SOFTWARE\\GNU\\Emacs","emacs_dir");
  if (ehome==NULL) {
    panic(1,"Cannot find GNU Emacs.\n");
  }

  char buffer[1000];
  normalizePath(ehome,true);
  sprintf(buffer,"%s/bin/runemacs.exe",ehome);
  if (access(buffer,0)) {
    panic(0,"Emacs binary '%s' does not exist.",buffer);
  }
  return ehome;
}

int WINAPI
WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
        LPSTR /*lpszCmdLine*/, int /*nCmdShow*/)
{
  char buffer[5000];

  initEnv();

  char *emacshome  = getEmacsHome();
  sprintf(buffer,
          "%s/bin/runemacs.exe -L \"%s/share/elisp\" -l oz.elc -f run-oz %s",
          emacshome,ozGetenv("OZHOME"),getCmdLine());

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,
                           0,NULL,NULL,&si,&pi);

  if (ret == FALSE) {
    panic(1,"Cannot run '%s'.",buffer);
  }

  return 0;
}
