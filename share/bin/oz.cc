#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>

BOOL getFileName(char *fname)
{
  static char 	filterList[] = "Emacs binary (emacs.exe)" \
    "\0" \
    "emacs.exe" \
    "\0\0";
    
  fname[ 0 ] = 0;

  OPENFILENAME	of;
  memset( &of, 0, sizeof( OPENFILENAME ) );
  of.lStructSize = sizeof( OPENFILENAME );
  of.hwndOwner = NULL;
  of.lpstrFilter = (LPSTR) filterList;
  of.lpstrDefExt = "";
  of.nFilterIndex = 1L;
  of.lpstrFile = fname;
  of.nMaxFile = _MAX_PATH;
  of.lpstrTitle = "Where is the emacs binary?";
  of.Flags = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;
  return GetOpenFileName( &of );
}

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
char *getEmacsHome(char *path)
{
  char buf[MAX_PATH];
  
  MessageBox(NULL, 
	     "When you start Oz for the first time,\nyou have to specify where the Emacs binary resides.\n", 
	     "Cannot find Emacs",
	     MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);

  BOOL ret = getFileName(buf);
  if (ret == FALSE)
    return NULL;
  return getParent(buf,2);
}

char *getOzHome(char *path)
{
  char *ret = getParent(path,3);
  if (ret == NULL) {
    OzPanic(1,"Cannot determine Oz installation directory.\nTry setting OZHOME environment variable.");
  }
  return ret;
}

void ozSetenv(const char *var, const char *value)
{
  if (SetEnvironmentVariable(var,value) == FALSE) {
    OzPanic(1,"setenv %s=%s failed",var,value);
  }
}


/* Todo: version should not be wired. */
char *reg_path = "SOFTWARE\\DFKI\\Oz\\1.9.9";

char *getRegistry(char *var)
{
  DWORD value_type = REG_SZ;
  DWORD buf_size = MAX_PATH;
  char buf[MAX_PATH];
  int rc = 0;

  HKEY hk;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,
		 reg_path,
		 &hk) != ERROR_SUCCESS)
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

int setRegistry(char *var, const char *value)
{
  HKEY hk;

  int ret = RegCreateKey(HKEY_LOCAL_MACHINE,
			 "SOFTWARE\\DFKI\\Oz\\1.9.9",
			 &hk);
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

int PASCAL
WinMain(HANDLE hInstance, HANDLE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
  char buffer[5000];
  char *ozhome = getenv("OZHOME");

  if (ozhome == NULL) {
    ozhome = getRegistry("OZHOME");
  }

  if (ozhome == NULL) {
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    ozhome = getOzHome(buffer);
    ozSetenv("OZHOME",ozhome);
    setRegistry("OZHOME",ozhome);
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

  char *ehome = getenv("EMACSHOME");
  if (ehome==NULL && (ehome=getRegistry("EMACSHOME"))==NULL) {
  getehome:
    ehome = getEmacsHome(buffer);
    if (ehome == NULL)
      OzPanic(1,"Cannot find Emacs.");
    ozSetenv("EMACSHOME",ehome);
    setRegistry("EMACSHOME",ehome);
  }

  sprintf(buffer,"%s/bin/emacs.exe",ehome);
  char *ebin = strdup(buffer);
  
  if (access(ebin,X_OK)) {
    OzPanic(0,"Emacs binary '%s' does not exist.",ebin);
    goto getehome;
  }
  sprintf(buffer,"%s/lisp",ehome);
  ozSetenv("EMACSLOADPATH",buffer);

  sprintf(buffer,"%s/etc",ehome);
  ozSetenv("EMACSDATA",buffer);

  sprintf(buffer,"%s/bin",ehome);
  ozSetenv("EMACSPATH",buffer);

  sprintf(buffer,"%s/lock",ehome);
  ozSetenv("EMACSLOCKDIR",buffer);

  sprintf(buffer,"%s/info",ehome);
  ozSetenv("INFOPATH",buffer);

  sprintf(buffer,"%s/etc",ehome);
  ozSetenv("EMACSDOC","%s/etc");

  ozSetenv("EMACSDOTERM","CMD");

  sprintf(buffer,"%s -l %s/lib/elisp/oz.elc -f run-oz",
	  ebin,ozhome);

  BOOL ret = CreateProcess(NULL,buffer,NULL,NULL,TRUE,
			   DETACHED_PROCESS,NULL,NULL,&si,&pinf);
  
  if (ret!=TRUE) {
    OzPanic(1,"Cannot start Oz.\nError = %d.",errno);
  }
  return 0;
}

