/*
 * winMain.c --
 *
 *      Main entry point for wish and other Tk-based applications.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef cdecl
#define cdecl __cdecl
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <string.h>

#include "tk.h"

static void cdecl WishPanic(char *x,...);
static unsigned __stdcall readerThread(void *arg);
static int cdecl asyncHandler(ClientData cd, Tcl_Interp *i, int code);

#define xxDEBUG

#ifdef DEBUG
#define DebugCode(Code) Code
#else
#define DebugCode(Code)
#endif

DebugCode(FILE *dbgout = NULL; FILE *dbgin = NULL;)

FILE *outstream;

/*
 * Global variables used by the main program:
 */

static Tcl_Interp *interp;      /* Interpreter for this application. */
static char argv0[255];         /* Buffer used to hold argv0. */

/*
 * Command-line options:
 */

static char *fileName = NULL;

static Tk_ArgvInfo argTable[] = {
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
        (char *) NULL}
};

int
PutsCmd(clientData, inter, argc, argv)
    ClientData clientData;              /* ConsoleInfo pointer. */
    Tcl_Interp *inter;                  /* Current interpreter. */
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Argument strings. */
{
    FILE *f;
    int i, newline;
    char *fileId;

    i = 1;
    newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
      newline = 0;
      i++;
    }

    if ((i < (argc-3)) || (i >= argc)) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                " ?-nonewline? ?fileId? string\"", (char *) NULL);
        return TCL_ERROR;
    }

    if (i == (argc-1)) {
        fileId = "stdout";
    } else {
        fileId = argv[i];
        i++;
    }

    f = outstream;

    clearerr(f);
    fputs(argv[i], f);
    if (newline) {
      fputc('\n', f);
    }
    fflush(f);

    DebugCode(fprintf(dbgout,"********puts(%d):\n%s\n",inter,argv[i]); fflush(dbgout));

    if (ferror(f)) {
      WishPanic("Connection to engine lost");
      ExitProcess(1);
      return TCL_ERROR;
    }
    return TCL_OK;
}



typedef struct {
  char *cmd;
  int cmdlen;
  DWORD toplevelThread;
  Tcl_AsyncHandler ash;
} ReaderInfo;


/* THE TWO FOLLOWING FUNCTIONS HAVE BEEN COPIED FROM EMULATOR */


unsigned __stdcall watchEmulatorThread(void *arg)
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
  int pid;
  HANDLE handle;
  unsigned thrid;

  if (GetEnvironmentVariable("OZPPID",buf,sizeof(buf)) == 0) {
    WishPanic("getenv failed");
  }

  pid = atoi(buf);
  handle = OpenProcess(SYNCHRONIZE, 0, pid);
  if (handle==0) {
    WishPanic("OpenProcess(%d) failed",pid);
  } else {
    CreateThread(0,10000,watchEmulatorThread,handle,0,&thrid);
    //_beginthreadex(0,0,watchEmulatorThread,handle,0,&thrid);
  }
}



CRITICAL_SECTION lock;

int APIENTRY
WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpszCmdLine;
    int nCmdShow;
{
    char **argv, **argvlist, *args, *p, *ozhome;
    int argc, size, i, code;
    char buf[300];
    size_t length;

    DebugCode(
              dbgin  = fopen("c:\\tmp\\wishdbgin","w");
              dbgout = fopen("c:\\tmp\\wishdbgout","w");
              if (dbgin == NULL || dbgout == NULL)
              WishPanic("cannot open dbgin/dbgout"));

    Tcl_SetPanicProc(WishPanic);

    InitializeCriticalSection(&lock);

    TkWinXInit(hInstance);

    /*
     * Increase the application queue size from default value of 8.
     * At the default value, cross application SendMessage of WM_KILLFOCUS
     * will fail because the handler will not be able to do a PostMessage!
     * This is only needed for Windows 3.x, since NT dynamically expands
     * the queue.
     */
    SetMessageQueue(64);

    interp = Tcl_CreateInterp();

    /* set TCL_LIBRARY and TK_LIBRARY */
    ozhome = getenv("OZHOME");
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
    size++;                     /* Leave space for final NULL pointer. */
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
    argc = i;

    /*
     * Parse command-line arguments.  A leading "-file" argument is
     * ignored (a historical relic from the distant past).  If the
     * next argument doesn't start with a "-" then strip it off and
     * use it as the name of a script file to process.  Also check
     * for other standard arguments, such as "-geometry", anywhere
     * in the argument list.
     */

    GetModuleFileName(NULL, argv0, 255);

    if (argc > 0) {
      length = strlen(argv[0]);
      if ((length >= 2) && (strncmp(argv[0], "-file", length) == 0)) {
        argc--;
        argv++;
      }
    }
    if ((argc > 0) && (argv[0][0] != '-')) {
      fileName = argv[0];
      argc--;
      argv++;
    }

    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, argv, argTable,
            TK_ARGV_DONT_SKIP_FIRST_ARG) != TCL_OK) {
      WishPanic("%s\n", interp->result);
    }
    if (fileName != NULL) {
      p = fileName;
    } else {
      p = argv0;
    }

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".    Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    args = Tcl_Merge(argc, argv);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv0,
            TCL_GLOBAL_ONLY);

    Tcl_SetVar2(interp, "env", "DISPLAY", "localhost:0", TCL_GLOBAL_ONLY);

    Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
        WishPanic("Tcl_AppInit failed: %s\n", interp->result);
    }

    Tcl_ResetResult(interp);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    ckfree((char *)argvlist);

    Tcl_CreateCommand(interp, "puts", PutsCmd,  (ClientData) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin),O_BINARY);

    outstream = fdopen(fileno(stdout),"wb");

    /* mm: do not show the main window */
    code = Tcl_GlobalEval(interp, "wm withdraw . ");
    if (code != TCL_OK) {
      fprintf(outstream, "w %s\n.\n", interp->result);
      fflush(outstream); /* added mm */
    }

    {
      ReaderInfo *info = (ReaderInfo*) malloc(sizeof(ReaderInfo));
      unsigned tid;
      unsigned long thread;

      info->ash            = Tcl_AsyncCreate(asyncHandler,(ClientData)info);
      info->toplevelThread = GetCurrentThreadId();
      info->cmd            = NULL;
      info->cmdlen         = -1;

      thread = (unsigned long) CreateThread(NULL,0,readerThread,info,0,&tid);
      if (thread==0) {
        fprintf(outstream, "w reader thread creation failed\n.\n");
        fflush(outstream); /* added mm */
        exit(1);
      }
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




int
Tcl_AppInit(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
  if (Tcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }
  if (Tk_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }

  return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * WishPanic --
 *
 *      Display a message and exit.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Exits the program.
 *
 *----------------------------------------------------------------------
 */


void cdecl
WishPanic TCL_VARARGS_DEF(char *,arg1)
{
    va_list argList;
    char buf[1024];
    char *format;

    format = TCL_VARARGS_START(char *,arg1,argList);
    vsprintf(buf, format, argList);

    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, buf, "Fatal Error in Wish",
            MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
    ExitProcess(1);
}


void cdecl idleProc(ClientData cd)
{
  ReaderInfo *ri = (ReaderInfo*) cd;
  int code;

  EnterCriticalSection(&lock);

  code = Tcl_GlobalEval(interp, ri->cmd);

  if (code != TCL_OK) {
    DebugCode(fprintf(dbgin,"### Error(%d):  %s\n", code,interp->result);
              fflush(dbgin));
    fprintf(outstream,"w --- %s", ri->cmd);
    fprintf(outstream,"---  %s\n---\n.\n", interp->result);
    fflush(outstream); /* by mm */
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

static unsigned __stdcall readerThread(void *arg)
{
  ReaderInfo *ri = (ReaderInfo *)arg;
  int count,i;

  int bufSize  = 64*1024;
  char *buffer = (char*) malloc(bufSize+1);
  int used = 0;

  while(1) {

    if (used>=bufSize) {
      bufSize *= 2;
      buffer = realloc(buffer,bufSize+1);
      if (buffer==0)
        WishPanic("realloc of buffer failed");
    }
    if (ReadFile(GetStdHandle(STD_INPUT_HANDLE),buffer+used,
                 bufSize-used,&count,0)==FALSE) {
      WishPanic("Connection to engine lost");
    }

    DebugCode(fprintf(dbgin,"\n### read done: %d\n",count); fflush(dbgin));

    used += count;
    buffer[used] = 0;
    if ((buffer[used-1] != '\n') && (buffer[used-1] != ';') ||
        !Tcl_CommandComplete(buffer) ||
        used >=2 && buffer[used-2] == '\\')
      continue;

    EnterCriticalSection(&lock);
    if (ri->cmd==NULL) {
      ri->cmdlen = used+1;
      ri->cmd = malloc(ri->cmdlen);
      memcpy(ri->cmd,buffer,used+1);
      Tcl_AsyncMark(ri->ash);
    } else {
      char *oldcmd = ri->cmd;
      int oldlen   = ri->cmdlen;
      ri->cmdlen   = oldlen+used;
      ri->cmd      = malloc(ri->cmdlen);
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
