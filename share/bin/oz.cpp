#include "misc.cpp"

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
  } else if (stricmp(progname,"ozsa.exe")==0) {
    sprintf(buffer,"%s/platform/%s/ozemulator -E -quiet -f %s",
	    ozhome,ozplatform,lpszCmdLine);
    // console = CREATE_NEW_CONSOLE;
  } else {
    OzPanic(1,"Unknown invocation: %s", progname);
  }

  PROCESS_INFORMATION pinf;
  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,
			   console,NULL,NULL,&si,&pinf);
  
  if (ret!=TRUE) {
    OzPanic(1,"Cannot start Oz.\nError = %d.\nDid you run setup?",errno);
  }
  return 0;
}

