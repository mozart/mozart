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

#pragma aux (cdecl) Tcl_AppendResult;
#pragma aux (cdecl) Tcl_CreateCommand;
#pragma aux (cdecl) Tcl_PosixError;
#pragma aux (cdecl) Tcl_Merge;
#pragma aux (cdecl) Tcl_SetVar;
#pragma aux (cdecl) Tcl_CreateInterp;
#pragma aux (cdecl) Tcl_EvalFile;
#pragma aux (cdecl) Tcl_GetVar;
#pragma aux (cdecl) Tcl_TildeSubst;
#pragma aux (cdecl) Tcl_DStringFree;
#pragma aux (cdecl) Tcl_Eval;
#pragma aux (cdecl) Tcl_Init;
#pragma aux (cdecl) Tcl_SetPanicProc;
#pragma aux (cdecl) Tcl_SetVar2;
#pragma aux (cdecl) Tk_ParseArgv;
#pragma aux (cdecl) Tcl_DStringInit;
#pragma aux (cdecl) Tcl_ResetResult;
#pragma aux (cdecl) Tcl_VarEval;
#pragma aux (cdecl) Tk_GetNumMainWindows;
#pragma aux (cdecl) Tk_Init;
#pragma aux (cdecl) TkWinXInit;
#pragma aux (cdecl) Tcl_DStringAppend;
#pragma aux (cdecl) Tcl_CommandComplete;
#pragma aux (cdecl) Tcl_AsyncCreate;
#pragma aux (cdecl) Tcl_AsyncMark;
#pragma aux (cdecl) PutsCmd;
#pragma aux (cdecl) asyncHandler;

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

#define Tk_MainLoop xx
#include "tk.h"
#undef Tk_MainLoop

static void WishPanic(char *x,...);
static unsigned __stdcall readerThread();
static int asyncHandler(ClientData *cd, Tcl_Interp *i, int code);

extern void __cdecl Tk_MainLoop();

FILE *outstream, *dbgout;


/*
 * Global variables used by the main program:
 */

HINSTANCE appInstance;		/* Application instance handle. */
int initShowState;		/* Initial window startup state. */

static Tcl_Interp *interp;	/* Interpreter for this application. */
static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */
static char defaultDisplay[] = "localhost:0";
static char argv0[255];		/* Buffer used to hold argv0. */

/*
 * Command-line options:
 */

static int synchronize = 0;
static char *fileName = NULL;
static char *display = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &display,
	"Display to use"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
	"Initial geometry for window"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
	"Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
	(char *) NULL}
};

int
PutsCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* ConsoleInfo pointer. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    FILE *f;
    static FILE *dbgout = NULL;
    int i, newline;
    char *fileId;

    if (dbgout==NULL) {
      dbgout = fopen("/tmp/dbgout","w");
    }
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
    fprintf(dbgout,"printed: %s\n",argv[i]); fflush(dbgout);
    if (ferror(f)) {
      Tcl_AppendResult(interp, "error writing \"", fileId,
		       "\": ", Tcl_PosixError(interp), (char *) NULL);
      return TCL_ERROR;
    }
    return TCL_OK;
}



typedef struct {
  char *cmd;
  HANDLE event;
  DWORD toplevelThread;
  Tcl_AsyncHandler ash;
} ReaderInfo;

/*
 *----------------------------------------------------------------------
 *
 * WinMain --
 *
 *	Main entry point from Windows.
 *
 * Results:
 *	Returns false if initialization fails, otherwise it never
 *	returns. 
 *
 * Side effects:
 *	Just about anything, since from here we call arbitrary Tcl code.
 *
 *----------------------------------------------------------------------
 */


int APIENTRY
WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpszCmdLine;
    int nCmdShow;
{
    char **argv, **argvlist, *args, *p;
    int argc, size, i, code;
    char buf[20];
    size_t length;

    appInstance = hInstance;
    initShowState = nCmdShow;

    Tcl_SetPanicProc(WishPanic);

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
    if (geometry != NULL) {
	Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display == NULL) {
	display = defaultDisplay;
    }
    Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = (fileName == NULL) ? 1 : 0;
    Tcl_SetVar(interp, "tcl_interactive", (tty ? "1" : "0"), TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
	WishPanic("Tcl_AppInit failed: %s\n", interp->result);
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	if (code != TCL_OK) {
	    p = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	    if ((p == NULL) || (*p == '\0')) {
		p = interp->result;
	    }
	    WishPanic("%s\n", p);
	}
	tty = 0;
    }
    Tcl_DStringInit(&command);
    Tcl_ResetResult(interp);

    /*
     * Set the geometry of the main window, if requested.  If the "geometry"
     * variable has gone away, this means that the application doesn't want
     * us to set the geometry after all.
     */

    if (geometry != NULL) {
	p = Tcl_GetVar(interp, "geometry", TCL_GLOBAL_ONLY);
	if (p != NULL) {
	    code = Tcl_VarEval(interp, "wm geometry . ", p, (char *) NULL);
	    if (code != TCL_OK) {
		WishPanic("%s\n", interp->result);
	    }
	}
    }

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    ckfree((char *)argvlist);

    Tcl_CreateCommand(interp, "puts", PutsCmd,  (ClientData) NULL,
		      (Tcl_CmdDeleteProc *) NULL);

    outstream = fdopen(_hdopen((int)GetStdHandle(STD_OUTPUT_HANDLE),
			       O_WRONLY|O_BINARY),
		       "wb");
    
    setmode(fileno(stdout),O_BINARY);
    {
      ReaderInfo *info = (ReaderInfo*) malloc(sizeof(ReaderInfo));
      Tcl_AsyncHandler ash = Tcl_AsyncCreate(asyncHandler,(ClientData)info);
      unsigned tid;
      unsigned long thread;

      info->event = CreateEvent(NULL, FALSE, FALSE, NULL);
      info->toplevelThread = GetCurrentThreadId();
      info->cmd=NULL;
      info->ash=ash;

      dbgout = fopen("/tmp/out","w");
      thread =_beginthreadex(NULL,0,readerThread,info,0,&tid);
      if (thread==0) {
	fprintf(outstream, "w reader thread creation failed\n.\n");
	fflush(outstream); /* added mm */
	exit(1);
      }
    }

    /* mm: do not show the main window */
    code = Tcl_Eval(interp, "wm withdraw . ");
    if (code != TCL_OK) {
      fprintf(outstream, "w %s\n.\n", interp->result);
      fflush(outstream); /* added mm */
    }

    Tk_MainLoop();

    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */
    /*      TerminateThread(thread,0);
	    Tcl_Eval(interp, "exit");*/
    fprintf(dbgout,"after eval exit\n"); fflush(dbgout);
    /*    exit(0);*/
    return 0;
}




/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    Tk_Window main;

    main = Tk_MainWindow(interp);

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

void
WishPanic(char *arg,...)
{
  va_list argList;
  char buf[1024];
  char *format;

  va_start(argList,format);

  vsprintf(buf, format, argList);
  
  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, buf, "Fatal Error in Wish",
	     MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
  exit(1);
}

static int asyncHandler(ClientData *cd, Tcl_Interp *i, int co)
{
  ReaderInfo *ri = (ReaderInfo*) cd;
  int code;

  fprintf(dbgout,"asyncHandler called: %s\n", ri->cmd); fflush(dbgout);
  code = Tcl_Eval(interp, ri->cmd);
  fprintf(dbgout,"after eval\n"); fflush(dbgout);
  if (*interp->result != 0) {
    if (code != TCL_OK) {
      fprintf(dbgout,"Error:  %s\n", interp->result);
      fflush(dbgout); /* by mm */
      fprintf(outstream,"w --- %s", ri->cmd);
      fprintf(outstream,"---  %s\n---\n.\n", interp->result);
      fflush(outstream); /* by mm */
    }
  }
  SetEvent(ri->event);
  return TCL_OK;
}


static unsigned __stdcall readerThread(void *arg)
{
#define BUFFER_SIZE 4000
  ReaderInfo *ri = (ReaderInfo *)arg;
  char input[BUFFER_SIZE+1];
  static int gotPartial = 0;
  char *cmd;
  BOOL bol;
  int count;
  /*Tcl_Interp *interp = Tcl_CreateInterp();*/
  int fdin = _hdopen((int)GetStdHandle(STD_INPUT_HANDLE),O_RDONLY|O_BINARY);
    
#define TclRead read

start:
  fprintf(dbgout,"before read(0x%x)\n",ri); fflush(dbgout);

  count = read(fdin, input, BUFFER_SIZE);

  /*  fprintf(dbgout,"after read(%d)\n",count); fflush(dbgout);*/

  if (count <= 0) {
    /*    fprintf(dbgout,"readerThread exit\n",count); fflush(dbgout);*/
    /*    fprintf(outstream, "s stop\n");
    fflush(outstream);*/ /* added mm */
    return 0;
  }
  fprintf(dbgout,"before append\n"); fflush(dbgout);
  cmd = Tcl_DStringAppend(&command, input, count);
  fprintf(dbgout,"after append\n"); fflush(dbgout);
  if (count != 0) {
    if ((input[count-1] != '\n') && (input[count-1] != ';')) {
      gotPartial = 1;
      goto prompt;
    }
    if (!Tcl_CommandComplete(cmd)) {
      gotPartial = 1;
      goto prompt;
    }
  }
  gotPartial = 0;
  
  ri->cmd = cmd;
  ResetEvent(ri->event);
  Tcl_AsyncMark(ri->ash);
  bol = PostThreadMessage(ri->toplevelThread,WM_NULL,NULL,NULL);
  fprintf(dbgout,"before wait: %s,bol=%d,err=%d\n",
	  cmd,bol,GetLastError()); fflush(dbgout);
  if (WaitForSingleObject(ri->event, INFINITE) != WAIT_OBJECT_0)
    return 0;
  fprintf(dbgout,"after wait\n"); fflush(dbgout);

  Tcl_DStringFree(&command);
prompt:
  goto start;
}



