#include "misc.cpp"

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


#ifdef CONSOLEAPP
int main(int argc, char **argv)
#else
int PASCAL
WinMain(HANDLE hInstance, HANDLE hPrevInstance,
        LPSTR lpszCmdLine, int nCmdShow)
#endif
{
  char buffer[5000];

  GetModuleFileName(NULL, buffer, sizeof(buffer));
  char *progname = getProgname(buffer);

  char *ozhome = getRegistry("OZHOME");
  char *ehome  = getRegistry("EMACSHOME");

  if (ehome==NULL || ozhome==NULL) {
    OzPanic(1,"Installation incorrect.\nDid you run setup?");
  }

  sprintf(buffer,"%s/bin/runemacs.exe",ehome);
  char *ebin = strdup(buffer);

  ozSetenv("OZPLATFORM",ozplatform);
  ozSetenv("OZHOME",ozhome);

  if (getenv("OZPATH") == NULL) {
    sprintf(buffer,"%s;%s/lib;%s/platform/%s;%s/demo",
            ozhome,ozhome,ozhome,ozplatform,ozhome);
    ozSetenv("OZPATH",buffer);
  }

  sprintf(buffer,"%s/platform/%s;%s",
          ozhome,ozplatform,getenv("PATH"));
  ozSetenv("PATH",buffer);


  /*
   * TCL/TK
   */
  sprintf(buffer,"%s/lib/wish/tcl",ozhome);
  ozSetenv("TCL_LIBRARY",buffer);
  sprintf(buffer,"%s/lib/wish/tk",ozhome);
  ozSetenv("TK_LIBRARY",buffer);


  STARTUPINFO si;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;

  int console = 0;
  if (stricmp(progname,"oz.exe")==0) {
    sprintf(buffer,"%s -l %s/lib/elisp/oz.elc -f run-oz",
            ebin,ozhome);
  } else if (stricmp(progname,"ozemacs.exe")==0) {
    sprintf(buffer,"%s",ebin);
  } else if (stricmp(progname,"ozdemo.exe")==0) {
    sprintf(buffer,"%s/platform/%s/ozemulator -f %s/demo/rundemo",
            ozhome,ozplatform,ozhome);
#ifdef CONSOLEAPP
  } else if (stricmp(progname,"ozsac.exe")==0) {
    if (argc!=3) {
      fprintf(stderr,"usage: ozsac infile outfile\n");
      exit(1);
    }
    sprintf(buffer,"%s/platform/%s/ozcompiler +p /dev/null +c %s +dn %s",
            ozhome,ozplatform,argv[1],argv[2]);
#else
  } else if (stricmp(progname,"ozsa.exe")==0) {
    char *rest = splitFirstArg(lpszCmdLine);
    sprintf(buffer,"%s/platform/%s/ozemulator -E -quiet -f %s %s %s",
            ozhome,ozplatform,lpszCmdLine,((*rest)==0) ? "" : "-a", rest);
    // console = CREATE_NEW_CONSOLE;
#endif
  } else {
    OzPanic(1,"Unknown invocation: %s", progname);
  }

  PROCESS_INFORMATION pinf;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,
                           console,NULL,NULL,&si,&pinf);

  if (ret!=TRUE) {
    OzPanic(1,"Cannot start Oz.\nError = %d.\nDid you run setup?",errno);
  }
#ifdef CONSOLEAPP
  WaitForSingleObject(pinf.hProcess,INFINITE);
  fprintf(stdout,"\n");
  fflush(stdout);
#endif
  return 0;
}
