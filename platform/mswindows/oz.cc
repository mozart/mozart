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

char *splitFirstArg(char *s)
{
  while (*s && (*s)!=' ') {
    s++;
  }
  
  if (*s==0)
    return s;
  *s=0;
  return s+1;
}


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

#ifdef OZENGINE
  const int depth = 2;
#else
  const int depth = 2;
#endif
  char *ozhome   = getOzHome(buffer,depth);

  ozSetenv("OZPLATFORM",ozplatform);
  ozSetenv("OZHOME",ozhome);

  {
    char *ozpath = getenv("OZPATH");
    if (ozpath == NULL) {
      ozpath = ".";
    }
    sprintf(buffer,"%s;%s/share", ozpath,ozhome);
    ozSetenv("OZPATH",buffer);
  }

  sprintf(buffer,"%s/bin;%s/platform/%s;%s",ozhome,ozhome,ozplatform,getenv("PATH"));
  ozSetenv("PATH",buffer);

  int console = 0;
  if (stricmp(progname,"oz.exe")==0) {
    char *emacshome  = getEmacsHome();
    if (emacshome==NULL) {
      OzPanic(1,"Emacs home not found.\nDid you install Emacs?");
    }
    sprintf(buffer,"%s/bin/runemacs.exe -L \"%s/share/elisp\" -l oz.elc -f run-oz",
	    emacshome,ozhome);
  } else if (stricmp(progname,"ozengine.exe")==0) {
    /*
      char *rest = splitFirstArg(lpszCmdLine);
      char *url = lpszCmdLine;
      sprintf(buffer,"%s -u \"%s\" -- %s", ozemulator,url,rest);
      */
#ifdef OZENGINE
    if (argc < 2) {
      fprintf(stderr,"usage: ozengine url <args>\n");
      exit(1);
    }
    char *ozemulator = getenv("OZEMULATOR");
    if (ozemulator == NULL) {
      sprintf(buffer,"%s/platform/%s/emulator.exe",ozhome,ozplatform);
      ozemulator = strdup(buffer);
    }

    char *url = argv[1];
    sprintf(buffer,"%s -u \"%s\" -- ", ozemulator,url);
    for (int i=2; i<argc; i++) {
      strcat(buffer," ");
      strcat(buffer,argv[i]);
    }
#endif
    //    console = DETACHED_PROCESS;
  } else {
    OzPanic(1,"Unknown invocation: %s", progname);
  } 

  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pinf;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,console,NULL,NULL,&si,&pinf);
  if (ret!=TRUE) {
    OzPanic(1,"Cannot run '%s' Oz.\nError = %d.\nDid you run setup?",buffer,errno);
  }
#ifdef OZENGINE
  WaitForSingleObject(pinf.hProcess,INFINITE);
  fprintf(stdout,"\n");
  fflush(stdout);
#endif
  return 0;
}

