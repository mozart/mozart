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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include <windows.h>
#include <ddeml.h>
#include <process.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "version.h"

#ifndef X_OK
#define X_OK 0
#endif

const char *ozplatform = "win32-i486";

char *reg_path = "SOFTWARE\\Mozart Consortium\\Mozart\\" OZVERSION;

void panic(int system, char *format, ...)
{
  va_list argList;
  char buf[1024];

  va_start(argList, format);
  vsprintf(buf, format, argList);

  if (system) {
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL);
    strcat(buf, (LPTSTR) lpMsgBuf);
    LocalFree(lpMsgBuf);
  }

#ifdef OZENGINE
  fprintf(stderr,"Fatal Error: %s\n",buf);
#else
  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buf, "Mozart Fatal Error",
             MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#endif

  ExitProcess(1);
}

void normalizePath(char *path)
{
  for(char *aux=path; *aux; aux++) {
    if (*aux == '\\') {
      *aux = '/';
    }
  }
}

void ozSetenv(const char *var, const char *value)
{
  if (SetEnvironmentVariable(var,value) == FALSE) {
    panic(1,"Adding %s=%s to environment failed.\n",var,value);
  }
}

char *getRegistry(char *path, char *var)
{
  DWORD value_type;
  DWORD buf_size = MAX_PATH;
  char buf[MAX_PATH];
  int rc = 0;

  HKEY hk;
  if (RegOpenKey(HKEY_LOCAL_MACHINE, path, &hk) != ERROR_SUCCESS)
    return NULL;

  if (RegQueryValueEx(hk,
                      var,
                      0,
                      &value_type,
                      (LPBYTE)buf,
                      &buf_size) == ERROR_SUCCESS)
    {
      rc = 1;
    }

  RegCloseKey(hk);

  return rc==1 && value_type == REG_SZ? strdup(buf): NULL;
}

char *getRegistry(char *var)
{
  return getRegistry(reg_path,var);
}

char *getParent(char *path, int levelsup)
{
  char *ret = strdup(path);
  for(int i=0; i<levelsup; i++) {
    char *aux = strrchr(ret,'\\');
    if (aux==NULL) {
      return NULL;
    }
    *aux = '\0';
  }
  return ret;
}
char *getOzHome(char *path, int depth)
{
  char *ret = getenv("OZHOME");
  if (ret==NULL) {
    ret = getParent(path,depth);
    if (ret == NULL) {
      panic(0,"Cannot determine Mozart installation directory.\n"
            "Try setting OZHOME environment variable.");
    }
  }
  normalizePath(ret);
  return ret;
}

char *getEmacsHome()
{
  char *ehome = getRegistry("SOFTWARE\\GNU\\Emacs","emacs_dir");
  if (ehome==NULL) {
    panic(1,"Cannot find GNU Emacs.\n");
  }

  char buffer[1000];
  normalizePath(ehome);
  sprintf(buffer,"%s/bin/runemacs.exe",ehome);
  if (access(buffer,X_OK)) {
    panic(0,"Emacs binary '%s' does not exist.",buffer);
  }
  return ehome;
}
