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
  if (ozhome==NULL) {
    OzPanic(1,"Oz home not found.\nDid you run setup?");
  }

  char *ebin;
  if (stricmp(progname,"oz.exe")!=0) {
    ebin = "unused";
  } else {
    char *ehome  = getRegistry("EMACSHOME");
    if (ehome==NULL) {
      OzPanic(1,"Emacs home not found.\nDid you run setup?");
    }
    sprintf(buffer,"%s/bin/runemacs.exe",ehome);
    ebin = strdup(buffer);
  }

  ozSetenv("OZPLATFORM",ozplatform);
  ozSetenv("OZHOME",ozhome);

  if (getenv("OZPATH") == NULL) {
    sprintf(buffer,"%s;%s/lib;%s/platform/%s;%s/demo",
            ozhome,ozhome,ozhome,ozplatform,ozhome);
    ozSetenv("OZPATH",buffer);
  }

  sprintf(buffer,"%s/platform/%s;%s",ozhome,ozplatform,getenv("PATH"));
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
    sprintf(buffer,"%s -l \"%s/lib/elisp/oz.elc\" -f run-oz",ebin,ozhome);
  } else if (stricmp(progname,"ozdemo.exe")==0) {
    sprintf(buffer,"%s/platform/%s/ozemulator -f %s/demo/rundemo",
            ozhome,ozplatform,ozhome);
  } else if (stricmp(progname,"ozengine.exe")==0) {
    char *rest = splitFirstArg(lpszCmdLine);
    char *url = lpszCmdLine;
    sprintf(buffer,"%s/platform/%s/ozemulator -u %s -a %s",
            ozhome,ozplatform,url,rest);
    //    console = DETACHED_PROCESS;
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
