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

TaggedRef OzDebug::toRecord(const char *label, Thread *thread, int frameId) {
  TaggedRef pairlist = oz_nil();
  if (data != makeTaggedNULL()) {
    pairlist = oz_cons(OZ_pairA("data",data),pairlist);
  }
  if (arity >= 0) {
    TaggedRef arglist = oz_nil();
    for(int i = arity; i--; ) {
      if (arguments[i] == NameVoidRegister)
        arguments[i] = OZ_newVariable();
      arglist = oz_cons(arguments[i], arglist);
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

#define DBG_MESSAGE(MSG) \
  OZ_MAKE_RECORD_S(MSG,1,{"thr"},{oz_thread(thread)},r); \
  am.debugStreamMessage(r);

void debugStreamBreakpoint(Thread *thread) {
  DBG_MESSAGE("breakpoint");
}

void debugStreamBlocked(Thread *thread) {
  DBG_MESSAGE("blocked");
}

void debugStreamReady(Thread *thread) {
  DBG_MESSAGE("ready");
}

void debugStreamTerm(Thread *thread) {
  DBG_MESSAGE("term");
}

void debugStreamException(Thread *thread, TaggedRef exc) {
  oz_currentThread()->setStop();

  OZ_MAKE_RECORD_S("exception",2,
                   {"thr" OZ_COMMA "exc" },
                   {oz_thread(thread) OZ_COMMA exc},r);
  am.debugStreamMessage(r);
}

void debugStreamEntry(OzDebug *dbg, int frameId) {
  oz_currentThread()->setStop();
  am.debugStreamMessage(dbg->toRecord("entry",oz_currentThread(),frameId));
}

void debugStreamExit(OzDebug *dbg, int frameId) {
  oz_currentThread()->setStep();
  oz_currentThread()->setStop();
  am.debugStreamMessage(dbg->toRecord("exit",oz_currentThread(),frameId));
}

void debugStreamUpdate(Thread *thread) {
  DBG_MESSAGE("update");
}

void execBreakpoint(Thread *t) {
  if (!t->isTrace() || !t->isStep()) {
    t->setTrace();
    t->setStep();
    debugStreamBreakpoint(t);
  }
}
