/*
 * main.c --
 *
 *      This file contains the main program for "ozwish", a windowing
 *      shell based on Tk and Tcl.  It also provides a template that
 *      can be used as the basis for main programs for other Tk
 *      applications.
 *
 * OZWISH: specials:
 *      - errors are prefixed by 'w '
 *      - after errors: fflush stdout
 *      - before exit: output 's stop'
 *      - no initial MainWindow visible (withdrawn)
 *      - load .ozwishrc
 *
 * Copyright (c) 1990-1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <stdio.h>
#include <tcl.h>
#include <tk.h>

/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkConfig.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 */

extern void             exit _ANSI_ARGS_((int status));
extern int              isatty _ANSI_ARGS_((int fd));
extern int              read _ANSI_ARGS_((int fd, char *buf, size_t size));
extern char *           strrchr _ANSI_ARGS_((CONST char *string, int c));

/*
 * Global variables used by the main program:
 */

static Tk_Window mainWindow;    /* The main window for the application.  If
                                 * NULL then the application no longer
                                 * exists. */
static Tcl_Interp *interp;      /* Interpreter for this application. */
static Tcl_DString command;     /* Used to assemble lines of terminal input
                                 * into Tcl commands. */
static int tty;                 /* Non-zero means standard input is a
                                 * terminal-like device.  Zero means it's
                                 * a file. */
static char errorExitCmd[] = "exit 1";

/*
 * Command-line options:
 */

static int synchronize = 0;
static char *fileName = NULL;
static char *name = NULL;
static char *display = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-file", TK_ARGV_STRING, (char *) NULL, (char *) &fileName,
        "File from which to read commands"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
        "Initial geometry for window"},
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &display,
        "Display to use"},
    {"-name", TK_ARGV_STRING, (char *) NULL, (char *) &name,
        "Name to use for application"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
        "Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
        (char *) NULL}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static void             Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
static void             StdinProc _ANSI_ARGS_((ClientData clientData,
                            int mask));

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *      Main program for Wish.
 *
 * Results:
 *      None. This procedure never returns (it exits the process when
 *      it's done
 *
 * Side effects:
 *      This procedure initializes the wish world and then starts
 *      interpreting commands;  almost anything could happen, depending
 *      on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

int
main(argc, argv)
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Array of argument strings. */
{
    char *args, *p, *msg;
    char buf[20];
    int code;

    interp = Tcl_CreateInterp();

    /*
     * Parse command-line arguments.
     */

    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, argv, argTable, 0)
            != TCL_OK) {
        fprintf(stdout, "w %s\n.\n", interp->result);
        fflush(stdout); /* added mm */
        exit(1);
    }
    if (name == NULL) {
        if (fileName != NULL) {
            p = fileName;
        } else {
            p = argv[0];
        }
        name = strrchr(p, '/');
        if (name != NULL) {
            name++;
        } else {
            name = p;
        }
    }

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display != NULL) {
        Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);
    }

    /*
     * Initialize the Tk application.
     */

#ifdef RS
    mainWindow = Tk_CreateMainWindow(interp, display, name, "Tk");
    if (mainWindow == NULL) {
        fprintf(stdout, "w %s\n.\n", interp->result);
        fprintf(stdout, "s stop\n");
        fflush(stdout); /* added mm */
        exit(1);
    }

    if (synchronize) {
        XSynchronize(Tk_Display(mainWindow), True);
    }
    Tk_GeometryRequest(mainWindow, 200, 200);
#endif

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".  Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    args = Tcl_Merge(argc-1, argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv[0],
            TCL_GLOBAL_ONLY);
    if (geometry != NULL) {
        Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive",
            ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
        fprintf(stdout, "w Tcl_AppInit failed: %s\n.\n", interp->result);
        fflush(stdout); /* added mm */
    }

    /*
     * Set the geometry of the main window, if requested.
     */

    if (geometry != NULL) {
        code = Tcl_VarEval(interp, "wm geometry . ", geometry, (char *) NULL);
        if (code != TCL_OK) {
            fprintf(stdout, "w %s\n.\n", interp->result);
            fflush(stdout); /* added mm */
        }
    }

    /* mm: do not show the main window */
    code = Tcl_Eval(interp, "wm withdraw . ");
    if (code != TCL_OK) {
      fprintf(stdout, "w %s\n.\n", interp->result);
      fflush(stdout); /* added mm */
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
        code = Tcl_VarEval(interp, "source ", fileName, (char *) NULL);
        if (code != TCL_OK) {
            goto error;
        }
        tty = 0;
    } else {
        /*
         * Commands will come from standard input, so set up an event
         * handler for standard input.  If the input device is aEvaluate the
         * .rc file, if one has been specified, set up an event handler
         * for standard input, and print a prompt if the input
         * device is a terminal.
         */

        Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
        if (tty) {
            Prompt(interp, 0);
        }
    }
    fflush(stdout);
    Tcl_DStringInit(&command);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    Tk_MainLoop();

    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */

    Tcl_Eval(interp, "exit");
    exit(1);

error:
    msg = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    if (msg == NULL) {
        msg = interp->result;
    }
    fprintf(stdout, "w %s\n.\n", msg);
    fflush(stdout);  /* added mm */
    Tcl_Eval(interp, errorExitCmd);
    fprintf(stdout, "s stop\n");
    fflush(stdout);  /* added mm */
    return 1;                   /* Needed only to prevent compiler warnings. */
}

/*
 *----------------------------------------------------------------------
 *
 * StdinProc --
 *
 *      This procedure is invoked by the event dispatcher whenever
 *      standard input becomes readable.  It grabs the next line of
 *      input characters, adds them to a command being assembled, and
 *      executes the command if it's complete.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Could be almost arbitrary, depending on the command that's
 *      typed.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
static void
StdinProc(clientData, mask)
    ClientData clientData;              /* Not used. */
    int mask;                           /* Not used. */
{
#define BUFFER_SIZE 4000
    char input[BUFFER_SIZE+1];
    static int gotPartial = 0;
    char *cmd;
    int code, count;

    count = read(fileno(stdin), input, BUFFER_SIZE);
    if (count <= 0) {
        if (!gotPartial) {
            if (tty) {
              Tcl_Eval(interp, "exit");
              fprintf(stdout, "s stop\n");
              fflush(stdout); /* added mm */
              exit(1);
            } else {
                Tk_DeleteFileHandler(0);
            }
            return;
        } else {
            count = 0;
        }
    }
    cmd = Tcl_DStringAppend(&command, input, count);
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

    /*
     * Disable the stdin file handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tk_CreateFileHandler(0, 0, StdinProc, (ClientData) 0);
    code = Tcl_Eval(interp, cmd);
    Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
    if (*interp->result != 0) {
        if ((code != TCL_OK) || (tty)) {
          fprintf(stdout,"w --- %s", cmd);
          fprintf(stdout,"---  %s\n---\n.\n", interp->result);
          fflush(stdout); /* by mm */
        }
    }
    Tcl_DStringFree(&command);

    /*
     * Output a prompt.
     */

    prompt:
    if (tty) {
        Prompt(interp, gotPartial);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Prompt --
 *
 *      Issue a prompt on standard output, or invoke a script
 *      to issue the prompt.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      A prompt gets output, and a Tcl script may be evaluated
 *      in interp.
 *
 *----------------------------------------------------------------------
 */

static void
Prompt(interp, partial)
    Tcl_Interp *interp;                 /* Interpreter to use for prompting. */
    int partial;                        /* Non-zero means there already
                                         * exists a partial command, so use
                                         * the secondary prompt. */
{
    char *promptCmd;
    int code;

    promptCmd = Tcl_GetVar(interp,
        partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    if (promptCmd == NULL) {
        defaultPrompt:
        if (!partial) {
            fputs("% ", stdout);
        }
    } else {
        code = Tcl_Eval(interp, promptCmd);
        if (code != TCL_OK) {
            Tcl_AddErrorInfo(interp,
                    "\n    (script that generates prompt)");
            fprintf(stdout, "w %s\n.\n", interp->result);
            fflush(stdout); /* added mm */
            goto defaultPrompt;
        }
    }
    fflush(stdout);
}
