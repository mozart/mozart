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
#include <stdlib.h>
#include <string.h>

#include "startup.hh"

bool console = true;

typedef void (*MAIN)(int, char **);

static DWORD __stdcall watchParentThread(void *arg)
{
  HANDLE handle = (HANDLE) arg;
  if (WaitForSingleObject(handle,INFINITE) == WAIT_FAILED) {
    panic(true,"Wait for parent process failed.\n");
  }
  ExitProcess(0);
  return 0;
}

static void watchParent()
{
  char buf[100];
  if (GetEnvironmentVariable("OZPPID",buf,sizeof(buf)) == 0)
    return;

  //--** this should really store an inherited handle instead of a pid
  int pid = atoi(buf);
  HANDLE handle = OpenProcess(SYNCHRONIZE,0,pid);
  if (!handle) {
    panic(true,"Could not access open parent process.\n");
  }

  DWORD thrid;
  HANDLE hThread = CreateThread(0,0,watchParentThread,handle,0,&thrid);
  CloseHandle(hThread);
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    panic(false,"Usage: ozengine <url> <args>");
  }

  initEnv();
  watchParent();
  // sameh@, dragan@ and kost@: first watch the parent, if any,
  // *then* set the OZPPID. In other words, DO NOT MOVE IT UP!
  publishPid();

  char *ozemulator = ozGetenv("OZEMULATOR");
  if (ozemulator == NULL) {
    ozemulator = "emulator.dll";
  }

  int new_argc = argc + 2;
  char **new_argv = new char *[new_argc];
  new_argv[0] = strdup(ozemulator);
  new_argv[1] = "-u";
  new_argv[2] = argv[1];
  new_argv[3] = "--";
  for (int i = 2; i < argc; i++)
    new_argv[i + 2] = argv[i];

  HINSTANCE hEmulator = LoadLibrary(new_argv[0]);
  if (hEmulator == NULL) {
    panic(true,"Could not link %s.\n",new_argv[0]);
  }
  MAIN OZ_main = (MAIN) GetProcAddress(hEmulator, "OZ_main");
  if (OZ_main == NULL) {
    panic(true,"Could not find function OZ_main in %s.\n",new_argv[0]);
  }
  OZ_main(new_argc, new_argv);
  FreeLibrary(hEmulator);

  return 0;
}
