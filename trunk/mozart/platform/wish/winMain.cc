/* 
 * winMain.c --
 *
 *	Main entry point for wish and other Tk-based applications.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <string.h>
#include <ctype.h>

#include "tk.h"

#ifndef cdecl
#define cdecl __cdecl
#endif

static void cdecl WishPanic(char *x,...);
static DWORD __stdcall readerThread(void *arg);
static int cdecl asyncHandler(ClientData cd, Tcl_Interp *i, int code);

#define xxDEBUG

#ifdef DEBUG
#define DebugCode(Code) Code
#else
#define DebugCode(Code)
#endif

DebugCode(FILE *dbgout = NULL; FILE *dbgin = NULL;)


/*
 * Global variables used by the main program:
 */

static Tcl_Interp *interp;	/* Interpreter for this application. */
static char argv0[255];		/* Buffer used to hold argv0. */

int outstream = -1;

void sendToEngine(char *s)
{
  int ret;
  int len = strlen(s);
  while(1) {
    ret = send(outstream, s, len, 0);
    if (ret < 0) {
      WishPanic("send failed");
  }
    if (ret=len)
      return;

    len -= ret;
    s += ret;
  }
}

int
PutsCmd(ClientData clientData, Tcl_Interp *inter, int argc, char **argv)
{
    int i = 1;
    int newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
      newline = 0;
      i++;
    }

    if ((i < (argc-3)) || (i >= argc)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?-nonewline? ?fileId? string\"", (char *) NULL);
	return TCL_ERROR;
    }

    if (i != (argc-1)) {
      i++;
    }

    sendToEngine(argv[i]);
    if (newline) {
      sendToEngine("\n");
    }

    DebugCode(fprintf(dbgout,"********puts(%d):\n%s\n",inter,argv[i]); fflush(dbgout));
    return TCL_OK;
}



typedef struct {
  char *cmd;
  int cmdlen;
  DWORD toplevelThread;
  Tcl_AsyncHandler ash;
  int fd;
} ReaderInfo;


/* THE TWO FOLLOWING FUNCTIONS HAVE BEEN COPIED FROM EMULATOR */


DWORD __stdcall watchEmulatorThread(void *arg)
{
  HANDLE handle = (HANDLE) arg;
  DWORD ret = WaitForSingleObject(handle,INFINITE);
  if (ret != WAIT_OBJECT_0) {
    WishPanic("WaitForSingleObject(0x%x) failed: %d (error=%d)",
	      handle,ret,GetLastError());
    ExitThread(0);
  }
  ExitProcess(0);
  return 1;
}

/* there are no process groups under Win32
 * so Emulator hands its pid via envvar OZPPID to emulator
 * it then creates a thread watching whether the Emulator is still living
 * and terminates otherwise
 */
void watchParent()
{
  char buf[100];

  if (GetEnvironmentVariable("OZPPID",buf,sizeof(buf)) == 0) {
    WishPanic("getenv failed");
  }

  int pid = atoi(buf);
  HANDLE handle = OpenProcess(SYNCHRONIZE, 0, pid);
  if (handle==0) {
    WishPanic("OpenProcess(%d) failed",pid);
  } else {
    DWORD thrid;
    CreateThread(0,10000,watchEmulatorThread,handle,0,&thrid);
  }
}



CRITICAL_SECTION lock;

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    char **argv, **argvlist, *args, *p;
    int size, i;

    DebugCode(
	      dbgin  = fopen("c:\\tmp\\wishdbgin","w");
	      dbgout = fopen("c:\\tmp\\wishdbgout","w");
	      if (dbgin == NULL || dbgout == NULL)
	      WishPanic("cannot open dbgin/dbgout"));

    Tcl_SetPanicProc(WishPanic);

    InitializeCriticalSection(&lock);

    interp = Tcl_CreateInterp();

    /* set TCL_LIBRARY and TK_LIBRARY */
    char *ozhome = getenv("OZHOME");
    if (ozhome == NULL) {
      WishPanic("OZHOME not set\n");
    }

    /*
     * First get an upper bound on the size of the argv array by counting the
     * number of whitespace characters in the string.
     */

    for (size = 1, p = lpszCmdLine; *p != '\0'; p++) {
      if (isspace(*p)) {
	size++;
      }
    }
    size++;			/* Leave space for final NULL pointer. */
    argvlist = (char **) ckalloc((unsigned) (size * sizeof(char *)));
    argv = argvlist;

    /*
     * Split the command line into words, and store pointers to the start of
     * each word into the argv array.  Skips leading whitespace on each word.
     */

    for (i = 0, p = lpszCmdLine; *p != '\0'; i++) {
	while (isspace(*p)) {
	    p++;
	}
	if (*p == '\0') {
	    break;
	}
	argv[i] = p;
	while (*p != '\0' && !isspace(*p)) {
	    p++;
	}
	if (*p != '\0') {
	    *p = '\0';
	    p++;
	}
    }
    argv[i] = NULL;
    int argc = i;

    /*
     * Parse command-line arguments.  A leading "-file" argument is
     * ignored (a historical relic from the distant past).  If the
     * next argument doesn't start with a "-" then strip it off and
     * use it as the name of a script file to process.  Also check
     * for other standard arguments, such as "-geometry", anywhere
     * in the argument list.
     */

    GetModuleFileName(NULL, argv0, 255);

    Tcl_SetVar2(interp, "env", "DISPLAY", "localhost:0", TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */
    if (Tcl_Init(interp) == TCL_ERROR ||
	Tk_Init(interp) == TCL_ERROR) {
      WishPanic("Tcl_Init failed: %s\n", interp->result);
    }

    Tcl_ResetResult(interp);

    ckfree((char *)argvlist);

    Tcl_CreateCommand(interp, "puts", (Tcl_CmdProc*) PutsCmd,  (ClientData) NULL,
		      (Tcl_CmdDeleteProc *) NULL);

    if (argc!=1) {
      WishPanic("argc!=1 (%d)\nUsage: tk.exe port\n", argc);
    }

    WSADATA wsa_data;
    WORD req_version = MAKEWORD(1,1);
    (void) WSAStartup(req_version, &wsa_data);
    
    char hostname[1000];
    int aux = gethostname(hostname,sizeof(hostname));
    if (aux != 0) {
      WishPanic("gethostname failed: %d\n",GetLastError());	
    }
    
    struct hostent *hostaddr = gethostbyname(hostname);
    if (hostaddr==NULL) {
      WishPanic("gethostbyname(%s) failed: %d\n",hostname,WSAGetLastError());	
    }
    
    struct in_addr tmp;
    memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(tmp));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr=tmp.s_addr;
    addr.sin_port = htons(atoi(argv[0]));

    outstream = socket(PF_INET,SOCK_STREAM,0);
    if(outstream <0 || connect(outstream,(struct sockaddr *) &addr,sizeof(addr))!=0) {
      WishPanic("socket creation failed: %d, %d, %d\n", 
		outstream, WSAGetLastError(),addr.sin_port);
    }
    
    close(0);
    close(1);
    close(2);

    Tcl_Channel chin  = Tcl_MakeFileChannel((HANDLE)outstream,TCL_READABLE|TCL_WRITABLE);
    Tcl_SetStdChannel(chin,TCL_STDIN);
    Tcl_SetStdChannel(chin,TCL_STDOUT);
    Tcl_SetStdChannel(chin,TCL_STDERR);

    /* mm: do not show the main window */
    int code = Tcl_GlobalEval(interp, "wm withdraw . ");
    if (code != TCL_OK) {
      char buf[1000];
      sprintf(buf,"w %s\n.\n", interp->result);
      sendToEngine(buf);
    }

    ReaderInfo *info = (ReaderInfo*) malloc(sizeof(ReaderInfo));
    info->ash            = Tcl_AsyncCreate(asyncHandler,(ClientData)info);
    info->toplevelThread = GetCurrentThreadId();
    info->cmd            = NULL;
    info->cmdlen         = -1;
    info->fd             = outstream;
    
    DWORD tid;
    if (CreateThread(NULL,0,readerThread,info,0,&tid)==0){
      sendToEngine("w reader thread creation failed\n.\n");
      exit(1);
    }

    watchParent();
    Tk_MainLoop();

    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */
    /*      TerminateThread(thread,0);
	    Tcl_Eval(interp, "exit");*/
    /*    exit(0);*/
    ExitProcess(0);
    return 0;
}




/*
 *----------------------------------------------------------------------
 *
 * WishPanic --
 *
 *	Display a message and exit.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Exits the program.
 *
 *----------------------------------------------------------------------
 */


void cdecl
WishPanic TCL_VARARGS_DEF(char *,arg1)
{
  va_list argList;
  char *format = TCL_VARARGS_START(char *,arg1,argList);
  char buf[1024];
  vsprintf(buf, format, argList);
  
  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buf, "Fatal Error in Wish",
	     MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  ExitProcess(1);
}


void cdecl idleProc(ClientData cd)
{
  ReaderInfo *ri = (ReaderInfo*) cd;

  EnterCriticalSection(&lock);

  int code = Tcl_GlobalEval(interp, ri->cmd);

  if (code != TCL_OK) {
    char buf[1000];
    DebugCode(fprintf(dbgin,"### Error(%d):  %s\n", code,interp->result);
	      fflush(dbgin));
    sprintf(buf,"w --- %s---  %s\n---\n.\n", ri->cmd,interp->result);
    sendToEngine(buf);
  }

  free(ri->cmd);
  ri->cmd    = 0;
  ri->cmdlen = -1;
  LeaveCriticalSection(&lock);
}


int cdecl asyncHandler(ClientData cd, Tcl_Interp *i, int code)
{
  Tcl_DoWhenIdle(idleProc,cd);
  return code;
}


static DWORD __stdcall readerThread(void *arg)
{
  ReaderInfo *ri = (ReaderInfo *)arg;
  int count,i;

  int bufSize  = 64*1024;
  char *buffer = (char*) malloc(bufSize+1);
  int used = 0;

  while(1) {

    if (used>=bufSize) {
      bufSize *= 2;
      buffer = (char *) realloc(buffer,bufSize+1);
      if (buffer==0)
	WishPanic("realloc of buffer failed");	
    }
    if ((count = recv(ri->fd,buffer+used,bufSize-used,0))<0) {
      WishPanic("Connection to engine lost: %d, %d, %d", 
		count, ri->fd,WSAGetLastError());
    }

    used += count;
    buffer[used] = 0;

    DebugCode(fprintf(dbgin,"\n### xxread done: %d\n%s\n",count,buffer); fflush(dbgin));

    if ((buffer[used-1] != '\n') && (buffer[used-1] != ';') ||
	!Tcl_CommandComplete(buffer) ||
	used >=2 && buffer[used-2] == '\\')
      continue;
  
    EnterCriticalSection(&lock);
    if (ri->cmd==NULL) {
      ri->cmdlen = used+1;
      ri->cmd = (char*) malloc(ri->cmdlen);
      memcpy(ri->cmd,buffer,used+1);
      Tcl_AsyncMark(ri->ash);
    } else {
      char *oldcmd = ri->cmd;
      int oldlen   = ri->cmdlen;
      ri->cmdlen   = oldlen+used;
      ri->cmd      = (char *) malloc(ri->cmdlen);
      memcpy(ri->cmd,oldcmd,oldlen);
      free(oldcmd);
      memcpy(ri->cmd+oldlen-1,buffer,used+1);
    }
    LeaveCriticalSection(&lock);

    used = 0;

    /* wake up toplevel */
    PostThreadMessage(ri->toplevelThread,WM_NULL,0,0);
  }
  
  return 0;
}

