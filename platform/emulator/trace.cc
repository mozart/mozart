/*
 *  Authors:
 *    Michael Mehl <mehl@dfki.de>
 *
 *  Copyright:
 *    Michael Mehl (1998)
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "trace.hh"
#endif

#include "trace.hh"
#include "codearea.hh"
#include "builtins.hh"
#include "board.hh"
#include "os.hh"

#include <string.h>
#include <signal.h>
#include <setjmp.h>

/*----------------------------------------------------------------------
 * the machine level debugger starts here
 */

#ifdef DEBUG_TRACE

#define MaxLine 100

static Bool mode=NO;

void ozd_tracerOn()
{
  mode = OK;
}

void ozd_tracerOff()
{
  mode = NO;
}

Bool ozd_trace(const char *info, ProgramCounter PC,RefsArray Y,
	       Abstraction *CAP)
{
  static char command[MaxLine];
  static int skip=0;

  if (!mode) {
    return OK;
  }
  if (PC != NOCODE) {
    displayCode(PC, 1);
  } else {
    printf("%s: ",info);
    oz_currentBoard()->print();
  }

  if (PC != NOCODE && skip > 0) {
    skip--;
    return OK;
  }
  while (1) {
    printf("\nBREAK");
    if (am.isSetSFlag()) {
      printf("[S:0x%p=",oz_currentBoard());
      if (am.isSetSFlag(ThreadSwitch)) {
	printf("P");
      }
      printf("]");
    }
    printf("> ");
    fflush(stdout);
    if (osfgets(command,MaxLine,stdin) == (char *) NULL) {
      printf("read no input\n");
      sprintf(command,"e\n");
    } else if (feof(stdin)) {
      clearerr(stdin);
      printf("exit\n");
      sprintf(command,"e\n");
    }
    if (command[0] == '\n') {
      sprintf(command,"s\n");
    }
    char *c = &command[1];
    while (*c != '\n') c++;
    *c = '\0';

    switch (command[0]) {
    case 'a':
      printAtomTab();
      break;
    case 'c':
      {
	sscanf(&command[1],"%d",&skip);
	if (skip==0) mode = NO;
	return OK;
      }
    case 'd':
      if (PC != NOCODE) {
	displayDef(PC,0);
      }
      break;
    case 'e':
      printf("*** Leaving Oz\n");
      am.exitOz(0);
    case 'f':
      return NO;
    case 'p':
      oz_currentBoard()->printLong();
      break;
    case 's':
      mode = OK;
      return OK;
    case 't':
      oz_currentThread()->printLong();
      break;
    case 'A':
      ozd_printAM();
      break;
    case 'B':
      ozd_printBoards();
      break;
    case 'D':
      {
	static ProgramCounter from=0;
	static int size=0;
	sscanf(&command[1],"%p %d",&from,&size);
	if (size == 0)
	  size = 10;
	displayCode(from,size);
      }
      break;
    case 'M':
      {
	ProgramCounter from=0;
	int size=0;
	sscanf(&command[1],"%p %d",&from,&size);
	printf("%p:",from);
	for (int i = 0; i < 20; i++)
	  printf(" %d",getWord(from+i));
	printf("\n%p:\n",from+20);
      }
      break;
    case 'T':
      am.threadsPool.printThreads();
      break;

    case '?':
      {
// CC does not like strings with CR
	static char *help[]
	  = {"\nOnline Help:",
	     "^D      = e",
	     "RET     = s",
	     "a       print atom tab",
	     "b       print builtin tab",
	     "c       continue (debug mode off)",
	     "d       display current definition",
	     "e       exit oz",
	     "f       fail",
	     "p       print current board (long)",
	     "s       continue with full debug mode",
	     "t       print current taskstack",
	     "A print class AM",
	     "D %x %d Display code",
	     "M %x %d view Memory <from> <len>",
	     "T       print class Thread",
	     "?       this help",
	     "\nin emulate mode additionally",
	     "d %d    display current line(s) code",
	     "x %d    display X register",
	     "X %d    display X register (long)",
	     "        for y,g also",
	     NULL};
	for (char **s = help; *s; s++) {
	  printf("%s\n",*s);
	}
	break;
      }
    default:
      if (PC != NOCODE) {
	switch (command[0]) {
	case 'd':
	  {
	    int size=0;
	    sscanf(&command[1],"%d",&size);
	    displayCode(PC, size ? size : 10);
	  }
	  break;
	case 'g':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf("G[%d] = ", numb);
	    fflush(stdout);
	    if (CAP) oz_print(CAP->getG(numb));
	    printf ("\n");
	  }
	  break;
	case 'G':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ( "G[%d]:\n", numb);
	    if (CAP) ozd_printLong(CAP->getG(numb));
	    printf ( "\n");
	  }
	  break;
	case 'x':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("X[%d] = ", numb);
	    fflush(stdout);
	    oz_print(XREGS[numb]);
	    printf ("\n");
	  }
	  break;
	case 'X':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("X[%d]:\n", numb);
	    ozd_printLong(XREGS[numb]);
	  }
	  break;
	case 'y':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("Y[%d] = ", numb);
	    fflush(stdout);
	    if (Y) oz_print(Y[numb]);
	    printf ("\n");
	  }
	  break;
	case 'Y':
	  {
	    int numb=0;
	    sscanf(&command[1],"%d",&numb);
	    printf ("Y[%d]:\n", numb);
	    if (Y) ozd_printLong(Y[numb]);
	  }
	  break;
	default:
	  printf("unknown emulate command '%s'\n",command);
	  printf("help with '?'\n");
	  break;
	}
      } else {
	printf("unknown command '%s'\n",command);
	printf("help with '?'\n");
      }
      break;
    }
  }
}

#endif
