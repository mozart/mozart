#include <windows.h>
#include <ddeml.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>

#include "version.h"

#ifndef X_OK
#define X_OK 0
#endif

void
OzPanic(int quit, char *format,...)
{
  va_list argList;
  char buf[1024];

  va_start(argList, format);
  vsprintf(buf, format, argList);

  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buf, "Fatal Error in Oz",
             MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  if (quit)
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

char *getOzHome(char *path)
{
  char *ret = getParent(path,1);
  if (ret == NULL) {
    OzPanic(1,"Cannot determine Oz installation directory.\nTry setting OZHOME environment variable.");
  }
  return ret;
}

char *getProgname(char *path)
{
  char *aux = path+strlen(path)-1;
  while(*aux != '\\' && *aux!='/') {
    if (aux == path)
      return strdup(path);
    aux--;
  }
  return strdup(aux+1);
}

void ozSetenv(const char *var, const char *value)
{
  if (SetEnvironmentVariable(var,value) == FALSE) {
    OzPanic(1,"setenv %s=%s failed",var,value);
  }
}


char *reg_path = "SOFTWARE\\DFKI\\Oz\\" OZVERSION;

char *getRegistry(char *path, char *var)
{
  DWORD value_type = REG_SZ;
  DWORD buf_size = MAX_PATH;
  char buf[MAX_PATH];
  int rc = 0;

  HKEY hk;
  if (RegOpenKey(HKEY_LOCAL_MACHINE, path, &hk) != ERROR_SUCCESS)
    goto end;

  if (RegQueryValueEx(hk,
                      var,
                      0,
                      &value_type,
                      (LPBYTE)buf,
                      &buf_size) == ERROR_SUCCESS)
    {
      rc = 1;
    }

 end:
  RegCloseKey(hk);

  return rc==1 ? strdup(buf) : NULL;
}

char *getRegistry(char *var)
{
  return getRegistry(reg_path,var);
}

int setRegistry(char *var, const char *value)
{
  HKEY hk;

  int ret = RegCreateKey(HKEY_LOCAL_MACHINE, reg_path, &hk);
  if (ret != ERROR_SUCCESS)
    return 0;

  if (RegSetValueEx(hk,
                    var,
                    0,
                    REG_SZ,
                    (CONST BYTE *) value,
                    strlen(value)+1) == ERROR_SUCCESS) {
    RegCloseKey(hk);
    return 1;
  }

  RegCloseKey(hk);
  return 0;
}

const char *ozplatform = "win32-i486";
