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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "debug.hh"
#endif

#include "debug.hh"
#include "codearea.hh"
#include "builtins.hh"
#include "thread.hh"
#include "am.hh"
#include "board.hh"

#include <string.h>
#include <signal.h>
#include <setjmp.h>

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
         oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),
              oz_cons(OZ_pairA("file",getTaggedArg(PC+1)),
                   oz_cons(OZ_pairAI("line",iline < 0? -iline: iline),
                        oz_cons(OZ_pairA("column",getTaggedArg(PC+3)),
                             oz_cons(OZ_pairA("origin",
                                           OZ_atom("OzDebug::toRecord")),
                                  oz_cons(OZ_pairAI("PC",(int)PC),
                                       oz_cons(OZ_pairA("kind",
                                                     getTaggedArg(PC+4)),
                                            pairlist))))))));

  return OZ_recordInit(OZ_atom(label), pairlist);
}

TaggedRef OzDebug::getFrameVariables() {
  return CodeArea::getFrameVariables(PC,Y,CAP);
}

// ------------------ debug stream messages ---------------------------

void debugStreamBreakpoint(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("breakpoint"), pairlist));
}

void debugStreamBlocked(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("blocked"), pairlist));
}

void debugStreamReady(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("ready"), pairlist));
}

void debugStreamTerm(Thread *thread) {
  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("term"), pairlist));
}

void debugStreamException(Thread *thread, TaggedRef exc) {
  oz_currentThread()->setStop(OK);

  TaggedRef pairlist =
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),
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
    oz_cons(OZ_pairA("thr",makeTaggedConst(thread)),oz_nil());
  am.debugStreamMessage(OZ_recordInit(OZ_atom("update"), pairlist));
}

// ------------------ Debugging Builtins ---------------------------

OZ_BI_define(BIgetDebugStream,0,1)
{
  OZ_RETURN(am.getDebugStreamTail());
} OZ_BI_end

OZ_BI_define(BIthreadUnleash,2,0)
{
  oz_declareThreadIN(0,thread);
  OZ_declareIntIN(1,frameId);

  if (!thread->isDeadThread() && thread->hasStack())
    thread->getTaskStackRef()->unleash(frameId);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetStepFlag,2,0)
{
  oz_declareThreadIN(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setStep(OK);
  else if (OZ_isFalse(yesno))
    thread->setStep(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetTraceFlag,2,0)
{
  oz_declareThreadIN(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setTrace(OK);
  else if (OZ_isFalse(yesno))
    thread->setTrace(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIcheckStopped,1,1)
{
  oz_declareThreadIN(0,thread);
  OZ_RETURN(thread->getStop() ? NameTrue : NameFalse);
} OZ_BI_end

// ------------------

OZ_BI_define(BIbreakpointAt, 3,1)
{
  OZ_declareIN    (0,file)
  OZ_declareIntIN (1,line);
  OZ_declareIN    (2,what);

  DEREF(file,_1,_2);

  DbgInfo *info = allDbgInfos;
  Bool    ok    = NO;
  char *inFile  = ozstrdup(toC(file));
  char *fullFile, *stripFile;

  while (info) {
    if (line == info->line) {
      fullFile  = toC(info->file);
      stripFile = strrchr(fullFile,'/');
      // the strstr approach might be somewhat too fuzzy...
      if (strstr(inFile,stripFile?stripFile+1:fullFile)) {
        ok = OK;
        // the PC+2 in the next lines is due to the format of the
        // DEBUGENTRY instruction:
        if (OZ_isTrue(what))
          CodeArea::writeTagged(OZ_int(-line),info->PC+2);
        else
          CodeArea::writeTagged(OZ_int( line),info->PC+2);
      }
    }
    info = info->next;
  }
  OZ_RETURN(ok? OZ_true(): OZ_false());
} OZ_BI_end

void execBreakpoint(Thread *t) {
  if (!t->getTrace() || !t->getStep()) {
    t->setTrace(OK);
    t->setStep(OK);
    debugStreamBreakpoint(t);
  }
}

OZ_BI_define(BIbreakpoint, 0,0)
{
  if (am.debugmode() && oz_onToplevel())
    execBreakpoint(oz_currentThread());
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIprocedureCoord, 1,1)
{
  oz_declareNonvarIN(0,proc);
  if (!oz_isProcedure(proc)) {
    oz_typeError(0,"Procedure");
  }
  if (oz_isBuiltin(proc)) {
    oz_typeError(0,"Procedure (no builtin)");
  }
  Abstraction *a=tagged2Abstraction(proc);
  ProgramCounter PC = a->getPred()->getPC();
  ProgramCounter definitionPC = CodeArea::definitionStart(PC);
  if (definitionPC != NOCODE) {
    Reg reg;
    int next,line,colum;
    TaggedRef file, predName;
    CodeArea::getDefinitionArgs(definitionPC,reg,next,file,line,colum,predName);
    TaggedRef pairlist =
      oz_cons(OZ_pairA("file",file),
           oz_cons(OZ_pairAI("line",line),
                oz_cons(OZ_pairAI("column",colum),
                     oz_cons(OZ_pairAI("PC",ToInt32(definitionPC)),oz_nil()))));
    OZ_RETURN(OZ_recordInit(OZ_atom("def"), pairlist));
  } else   // should never happen
    OZ_RETURN(NameUnit);
} OZ_BI_end


OZ_BI_define(BIthreadID,1,1)
{
  oz_declareThreadIN(0,th);

  if (th->isProxy())
    return oz_raise(E_ERROR,E_SYSTEM,"threadID Proxy not impl",0);

  OZ_RETURN_INT(th->getID() & THREAD_ID_MASK);
} OZ_BI_end

OZ_BI_define(BIsetThreadID,2,0)
{
  oz_declareThreadIN(0,th);
  oz_declareIntIN(1,id);

  if (th->isProxy())
    return oz_raise(E_ERROR,E_SYSTEM,"setThreadID Proxy not impl",0);

  th->setID(id | (1 << THREAD_ID_SIZE));
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIparentThreadID,1,1)
{
  oz_declareThreadIN(0,th);

  if (th->isProxy())
    return oz_raise(E_ERROR,E_SYSTEM,"parentThreadID Proxy not impl",0);

  OZ_RETURN_INT((th->getID() >> THREAD_ID_SIZE) & THREAD_ID_MASK);
} OZ_BI_end

OZ_BI_define(BIthreadSetRaiseOnBlock,2,0)
{
  oz_declareThreadIN(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setNoBlock(OK);
  else if (OZ_isFalse(yesno))
    thread->setNoBlock(NO);
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadGetRaiseOnBlock,1,1)
{
  oz_declareThreadIN(0,thread);

  OZ_RETURN(thread->getNoBlock()? NameTrue: NameFalse);
} OZ_BI_end


OZ_BI_define(BIthreadTaskStack,3,1)
{
  oz_declareThreadIN(0,thread);
  oz_declareIntIN(1,depth);
  oz_declareNonvarIN(2,verbose);

  if (thread->isDeadThread() || !thread->hasStack())
    OZ_RETURN(oz_nil());

  Bool doverbose;
  if (OZ_isTrue(verbose))
    doverbose = OK;
  else if (OZ_isFalse(verbose))
    doverbose = NO;
  else
    oz_typeError(2,"Bool");

  TaskStack *taskstack = thread->getTaskStackRef();
  OZ_RETURN(taskstack->getTaskStack(thread,doverbose,depth));
} OZ_BI_end

OZ_BI_define(BIthreadFrameVariables,2,1)
{
  oz_declareThreadIN(0,thread);
  oz_declareIntIN(1,frameId);

  if (thread->isDeadThread() || !thread->hasStack())
    OZ_RETURN(NameUnit);

  TaskStack *taskstack = thread->getTaskStackRef();
  OZ_RETURN(taskstack->getFrameVariables(frameId));
} OZ_BI_end


OZ_BI_define(BIthreadLocation,1,1)
{
  oz_declareThreadIN(0,thread);

  if (thread->isDeadThread()) {
    OZ_RETURN(oz_nil());
  } else {
    OZ_RETURN(oz_getLocation(GETBOARD(thread)));
  }
} OZ_BI_end

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDebug-if.cc"

#endif
