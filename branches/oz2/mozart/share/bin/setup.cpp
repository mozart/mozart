#include "misc.cpp"

HDDEDATA CALLBACK 
DdeCallback (UINT uType, UINT uFmt, HCONV hconv,
	     HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
	     DWORD dwData1, DWORD dwData2)
{
  return ((HDDEDATA) NULL);
}

inline void DdeCommand(char *str, HCONV HConversation)
{
  DdeClientTransaction ((unsigned char*)str, strlen (str)+1, HConversation, (HSZ)NULL,
			CF_TEXT, XTYP_EXECUTE, 30000, NULL);
}



int PASCAL
WinMain(HANDLE hInstance, HANDLE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
  char buffer[5000];

  /*
   * Emacs
   */
  char *ehome = getRegistry("SOFTWARE\\GNU\\Emacs","emacs_dir");
  if (ehome==NULL) {
    OzPanic(1,"Cannot find Emacs: did you correctly install GNU Emacs?");
  }

  normalizePath(ehome);
  sprintf(buffer,"%s/bin/runemacs.exe",ehome);
  char *ebin = strdup(buffer);
  
  if (access(ebin,X_OK)) {
    OzPanic(1,"Emacs binary '%s' does not exist.",ebin);
  }

  GetModuleFileName(NULL, buffer, sizeof(buffer));
  char *ozhome = getOzHome(buffer);
  //  ozhome = "F:\\oz-devel";
  normalizePath(ozhome);

  sprintf(buffer,"%s/platform/%s/ozemulator.exe",ozhome,ozplatform);
  if (access(buffer,X_OK)) {
    OzPanic(1,"Oz installation incorrect.\n"
	      "Cannot find '%s'!",buffer);
  }

  sprintf(buffer,
	  "Setting up Oz version %s under directory:\n"
	  "\t %s",
	  OZVERSION,ozhome);
  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buffer, "DFKI Oz Installation",
	     MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  
  DWORD idDde = 0;
  HCONV HConversation;
  HSZ ProgMan;
  DdeInitialize (&idDde, (PFNCALLBACK)DdeCallback, APPCMD_CLIENTONLY, 0);
  
  ProgMan = DdeCreateStringHandle (idDde, "PROGMAN", CP_WINANSI);
  
  if (HConversation = DdeConnect (idDde, ProgMan, ProgMan, NULL))
    {
      DdeCommand ( "[DeleteGroup (DFKI Oz)]", HConversation);
      DdeCommand ( "[CreateGroup (DFKI Oz)]", HConversation);
      DdeCommand ("[ReplaceItem (DFKI Oz)]", HConversation);
      sprintf(buffer, "[AddItem (%s\\platform\\%s\\tcldoc\\tcl76.hlp, Tcl_Tk Manual)]", ozhome,ozplatform);
      DdeCommand(buffer, HConversation);
      sprintf(buffer, "[AddItem (%s\\oz.exe, DFKI Oz)]", ozhome);
      DdeCommand(buffer, HConversation);
      //      sprintf(buffer, "[AddItem (%s\\ozdemo.exe, Oz Demo)]", ozhome);
      //      DdeCommand(buffer, HConversation);
      sprintf(buffer, "[AddItem (%s\\setup.exe, Oz Setup)]", ozhome);
      DdeCommand(buffer, HConversation);

      DdeDisconnect (HConversation);
    }
  
  DdeFreeStringHandle (idDde, ProgMan);
  
  DdeUninitialize (idDde);
  
  setRegistry("OZHOME",ozhome);
  setRegistry("EMACSHOME",ehome);
  Sleep(2000);
  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, 
	     "Finished installation of DFKI Oz.",
	     "DFKI Oz Installation",
	     MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);

  return 0;
}

