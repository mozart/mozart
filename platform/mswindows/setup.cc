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

#ifdef __MINGW32__

#define     APPCMD_CLIENTONLY            0x00000010L

extern "C" WINBOOL WINAPI DdeFreeStringHandle(DWORD idInst, HSZ hsz);

extern "C" HDDEDATA WINAPI DdeClientTransaction(LPBYTE pData, DWORD cbData,
        HCONV hConv, HSZ hszItem, UINT wFmt, UINT wType,
        DWORD dwTimeout, LPDWORD pdwResult);


#endif


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
  char *ehome = getEmacsHome();

  GetModuleFileName(NULL, buffer, sizeof(buffer));
  char *ozhome = getOzHome(buffer,1);

  sprintf(buffer,"%s/platform/%s/emulator.exe",ozhome,ozplatform);
  if (access(buffer,X_OK)) {
    OzPanic(1,"Oz installation incorrect.\n"
	      "Cannot find '%s'!",buffer);
  }

  int quiet = strncmp(lpszCmdLine,"-q",2)==0 ? 1 : 0;

  if (!quiet) {
    sprintf(buffer,
	    "Setting up Oz version %s under directory:\n"
	    "\t %s",
	    OZVERSION,ozhome);
    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, buffer, "Mozart Installation",
	       MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  }

  DWORD idDde = 0;
  HCONV HConversation;
  HSZ ProgMan;
  DdeInitialize (&idDde, (PFNCALLBACK)DdeCallback, APPCMD_CLIENTONLY, 0);
  
  ProgMan = DdeCreateStringHandle (idDde, "PROGMAN", CP_WINANSI);
  
  if (HConversation = DdeConnect (idDde, ProgMan, ProgMan, NULL)) {
    DdeCommand ( "[DeleteGroup (Mozart 3)]", HConversation);
    DdeCommand ( "[CreateGroup (Mozart 3)]", HConversation);
    DdeCommand ( "[ReplaceItem (Mozart 3)]", HConversation);
    
    sprintf(buffer, 
	    "[AddItem (%s\\platform\\%s\\tcldoc\\tcl80.hlp, Tcl_Tk Manual)]", 
	    ozhome,ozplatform);
    DdeCommand(buffer, HConversation);

    sprintf(buffer,
	    "[AddItem (%s\\oz.exe, Mozart, %s\\oz.ico)]", 
	    ozhome, ozhome);
    DdeCommand(buffer, HConversation);
    
    sprintf(buffer, 
	    "[AddItem (%s\\setup.exe, Mozart Setup, %s\\setup.ico)]",
	    ozhome, ozhome);
    DdeCommand(buffer, HConversation);
    
    DdeDisconnect (HConversation);
  }
  
  DdeFreeStringHandle (idDde, ProgMan);
  
  DdeUninitialize (idDde);
  
  Sleep(1000);

  if (!quiet) {
    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, 
	       "Finished installation of Mozart.",
	       "Mozart Installation",
	       MB_ICONINFORMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  }

  return 0;
}

