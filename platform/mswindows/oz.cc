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

#include "misc.cc"

#if !defined(OZENGINE) && !defined(OZENGINEW)
#define OZSCRIPT
#endif


char *getEmulator(char *ozhome)
{
  char buffer[5000];
  char *ozemulator = getenv("OZEMULATOR");
  if (ozemulator == NULL) {
    sprintf(buffer,"%s/platform/%s/emulator.exe",ozhome,ozplatform);
    ozemulator = buffer;
  }
  return strdup(ozemulator);
}


#ifdef OZENGINEW

#define bufsz 1000

unsigned __stdcall readerThread(void *arg)
{
  HANDLE hd = (HANDLE)arg;
  unsigned int ret;
  char buf[bufsz];
  while(1) {
    if (ReadFile(hd,buf,bufsz-1,&ret,0)==FALSE)
      return 0;
    buf[ret]=0;
    //MessageBeep(MB_ICONINFORMATION);
    MessageBox(NULL, buf, "Mozart Emulator says:",
               MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
    Sleep(2000);
  }
  return 1;
}
#endif


#ifdef OZENGINE
int main(int argc, char **argv)
#else
int PASCAL
WinMain(HANDLE /*hInstance*/, HANDLE /*hPrevInstance*/,
        LPSTR lpszCmdLine, int /*nCmdShow*/)
#endif
{
  char buffer[5000];

  GetModuleFileName(NULL, buffer, sizeof(buffer));
  char *progname = getProgname(buffer);
  char *ozhome   = getOzHome(buffer,2);

  ozSetenv("OZPLATFORM",ozplatform);
  ozSetenv("OZHOME",ozhome);

  char *ozpath = getenv("OZPATH");
  if (ozpath == NULL) {
    ozpath = ".";
  }
  sprintf(buffer,"%s;%s/share", ozpath,ozhome);
  ozSetenv("OZPATH",buffer);

  sprintf(buffer,"%s/bin;%s/platform/%s;%s",
          ozhome,ozhome,ozplatform,getenv("PATH"));
  ozSetenv("PATH",buffer);

  int console = 0;

#ifdef OZSCRIPT
  char *emacshome  = getEmacsHome();
  if (emacshome==NULL) {
    OzPanic(1,"Emacs home not found.\nDid you install Emacs?");
  }
  sprintf(buffer,"%s/bin/runemacs.exe -L \"%s/share/elisp\" -l oz.elc -f run-oz",
          emacshome,ozhome);
#endif
#if defined(OZENGINE) || defined(OZENGINEW)
#ifdef OZENGINEW
  int argc    = _argc;
  char **argv = _argv;
  console = DETACHED_PROCESS;
#endif
  if (argc < 2) {
    OzPanic(1,"usage: ozengine url <args>\n");
  }
  char *ozemulator = getEmulator(ozhome);
  char *url = argv[1];
  sprintf(buffer,"%s -u \"%s\" -- ", ozemulator,url);
  for (int i=2; i<argc; i++) {
    strcat(buffer," \"");
    strcat(buffer,argv[i]);
    strcat(buffer,"\"");
  }
#endif

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEOFFFEEDBACK;

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;

#ifdef OZENGINEW
  HANDLE rh,wh;
  if (!CreatePipe(&rh,&wh,&sa,0)) {
    OzPanic(1,"CreatePipe failed");
  }
  si.dwFlags |= STARTF_USESTDHANDLES;
  si.hStdOutput = wh;
  si.hStdError  = wh;
#endif

  PROCESS_INFORMATION pinf;
  BOOL ret = CreateProcess(NULL,buffer,&sa,NULL,TRUE,
                           console,NULL,NULL,&si,&pinf);

  if (ret!=TRUE) {
    OzPanic(1,"Cannot run '%s' Oz.\nError = %d."
              "Did you run setup?",buffer,errno);
  }


#ifdef OZENGINEW
  unsigned thrid;
  CreateThread(0,10000,&readerThread,rh,0,&thrid);
#endif

  WaitForSingleObject(pinf.hProcess,INFINITE);

#ifdef OZENGINE
  fprintf(stdout,"\n");
  fflush(stdout);
#endif

  return 0;
}
