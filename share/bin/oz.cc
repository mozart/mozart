#define NOGDI
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>


void
OzPanic(char *format,...)
{
  va_list argList;
  char buf[1024];

  va_start(argList, format);
  vsprintf(buf, format, argList);

  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buf, "Fatal Error in Oz",
             MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  ExitProcess(1);
}


char *getOzHome(char *path)
{
  char *ret = strdup(path);
  for(int i=0; i<3; i++) {
    char *aux = strrchr(ret,'\\');
    if (aux==NULL) {
      OzPanic("Cannot determine Oz installation directory.\nTry setting OZHOME environment variable.");
    }
    *aux = '\0';
  }
  return ret;
}

void ozSetenv(const char *var, const char *value)
{
  if (SetEnvironmentVariable(var,value) == FALSE) {
    OzPanic("setenv %s=%s failed",var,value);
  }
}

int PASCAL
WinMain(HANDLE hInstance, HANDLE hPrevInstance,
        LPSTR lpszCmdLine, int nCmdShow)
{
  char buffer[5000];
  char *ozhome = getenv("OZHOME");

  if (ozhome == NULL) {
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    ozhome = getOzHome(buffer);
    ozSetenv("OZHOME",ozhome);
  }

  const char *ozplatform = "win32-i486";
  ozSetenv("OZPLATFORM",ozplatform);

  if (getenv("OZPATH") == NULL) {
    sprintf(buffer,"%s;%s/lib;%s/platform/%s;%s/demo",
            ozhome,ozhome,ozhome,ozplatform,ozhome);
    ozSetenv("OZPATH",buffer);
  }

  sprintf(buffer,"%s/platform/%s;%s",
          ozhome,ozplatform,getenv("PATH"));
  ozSetenv("PATH",buffer);

  sprintf(buffer,"%s/ozwish/lib/tcl",ozhome);
  ozSetenv("TCL_LIBRARY",buffer);
  sprintf(buffer,"%s/ozwish/lib/tk",ozhome);
  ozSetenv("TK_LIBRARY",buffer);

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;

  PROCESS_INFORMATION pinf;

  sprintf(buffer,"%s/platform/%s/ozcompiler",ozhome,ozplatform);

  char *emacs_dir = "c:/emacs-19.30";

  sprintf(buffer,"%s/lisp",emacs_dir);
  ozSetenv("EMACSLOADPATH",buffer);

  sprintf(buffer,"%s/etc",emacs_dir);
  ozSetenv("EMACSDATA",buffer);

  sprintf(buffer,"%s/bin",emacs_dir);
  ozSetenv("EMACSPATH",buffer);

  sprintf(buffer,"%s/lock",emacs_dir);
  ozSetenv("EMACSLOCKDIR",buffer);

  sprintf(buffer,"%s/info",emacs_dir);
  ozSetenv("INFOPATH",buffer);

  sprintf(buffer,"%s/etc",emacs_dir);
  ozSetenv("EMACSDOC","%s/etc");

  ozSetenv("EMACSDOTERM","CMD");

  char *ozemacs = "c:/emacs-19.30/bin/emacs.exe";
  sprintf(buffer,"%s -l %s/lib/elisp/oz.elc -f run-oz",
          ozemacs,ozhome);

  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,
                           DETACHED_PROCESS,NULL,NULL,&si,&pinf);

  if (ret!=TRUE) {
    OzPanic("Cannot start Oz.\nError = %d.",errno);
  }
  return 0;
}
