/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *    Benjamin Lorenz <lorenz@ps.uni-sb.de>
 *    Michael Mehl <mehl@dfki.de>
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *
 *  Copyright:
 *    Benjamin Lorenz <lorenz@ps.uni-sb.de>
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
#pragma implementation "debug.hh"
#endif

#include "debug.hh"
#include "codearea.hh"
#include "am.hh"

/*
#include "builtins.hh"
#include "thread.hh"
#include "board.hh"

#include <string.h>
#include <signal.h>
#include <setjmp.h>
*/

Bool pbDebug=FALSE;

TaggedRef OzDebug::toRecord(const char *label, Thread *thread, int frameId) {
  TaggedRef pairlist = oz_nil();
  if (data != makeTaggedNULL()) {
    pairlist = oz_cons(OZ_pairA("data",data),pairlist);
  }
  if (arguments != (RefsArray) NULL) {
    TaggedRef arglist = oz_nil();
    for(int i = getRefsArraySize(arguments) - 2; i >= 0; i--) {
      if (arguments[i] == makeTaggedNULL())
	arguments[i] = OZ_newVariable();
      arglist = oz_cons(arguments[i],arglist);
    }
    pairlist = oz_cons(OZ_pairA("args",arglist),pairlist);
  }
  if (frameId == -1) {
    pairlist = oz_cons(OZ_pairA("vars",getFrameVariables()),pairlist);
  } else {
    pairlist = oz_cons(OZ_pairAI("frameID",frameId),pairlist);
  }
  int iline = smallIntValue(getNumberArg(PC+2));
  pairlist =
    oz_cons(OZ_pairAI("time",CodeArea::findTimeStamp(PC)),
	 oz_cons(OZ_pairA("thr",oz_thread(thread)),
	      oz_cons(OZ_pairA("file",getTaggedArg(PC+1)),
		   oz_cons(OZ_pairAI("line",iline < 0? -iline: iline),
			oz_cons(OZ_pairA("column",getTaggedArg(PC+3)),
			     oz_cons(OZ_pairA("origin",
					   OZ_atom("debugFrame")),
				  oz_cons(OZ_pairAI("PC",(int)PC),
				       oz_cons(OZ_pairA("kind",
						     getTaggedArg(PC+4)),
					    pairlist))))))));

  return OZ_recordInit(OZ_atom((OZ_CONST char*)label), pairlist);
}

TaggedRef OzDebug::getFrameVariables() {
  return CodeArea::getFrameVariables(PC,Y,CAP);
}

// ------------------ debug stream messages ---------------------------

void debugStreamBreakpoint(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("breakpoint"), pairlist));
}

void debugStreamBlocked(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("blocked"), pairlist));
}

void debugStreamReady(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("ready"), pairlist));
}

void debugStreamTerm(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("term"), pairlist));
}

void debugStreamException(Thread *thread, TaggedRef exc) {
  oz_currentThread()->setStop(OK);

  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),
	 oz_cons(OZ_pairA("exc",exc),oz_nil()));
  am.debugStreamMessage(OZ_recordInit(OZ_atom("exception"), pairlist));
}

void debugStreamEntry(OzDebug *dbg, int frameId) {
  oz_currentThread()->setStop(OK);
  am.debugStreamMessage(dbg->toRecord("entry",oz_currentThread(),frameId));
}

void debugStreamExit(OzDebug *dbg, int frameId) {
  oz_currentThread()->setStep(OK);
  oz_currentThread()->setStop(OK);
  am.debugStreamMessage(dbg->toRecord("exit",oz_currentThread(),frameId));
}

void debugStreamUpdate(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",oz_thread(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("update"), pairlist));
}

void execBreakpoint(Thread *t) {
  if (!t->getTrace() || !t->getStep()) {
    t->setTrace(OK);
    t->setStep(OK);
    debugStreamBreakpoint(t);
  }
}
