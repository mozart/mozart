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

#include "base.hh"
#include "debug.hh"
#include "codearea.hh"
#include "builtins.hh"
#include "am.hh"
#include "board.hh"
#include "thr_class.hh"

#include <string.h>
#include <signal.h>
#include <setjmp.h>

OZ_BI_define(BIgetGlobals,1,1)
{
  oz_declareNonvarIN(0, proc);
  if (oz_isConst(proc) && tagged2Const(proc)->getType() == Co_Abstraction) {
    OZ_RETURN(((Abstraction *) tagged2Const(proc))->DBGgetGlobals());
  } else if (oz_isProcedure(proc)) {
    OZ_RETURN(OZ_atom("globals"));
  } else {
    oz_typeError(0,"User-defined Procedure");
  }
} OZ_BI_end

OZ_BI_define(BIgetDebugStream,0,1)
{
  OZ_RETURN(am.getDebugStreamTail());
} OZ_BI_end

OZ_BI_define(BIthreadUnleash,2,0)
{
  oz_declareThread(0,thread);
  OZ_declareInt(1,frameId);

  if (!thread->isDead())
    thread->getTaskStackRef()->unleash(frameId);

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetStepFlag,2,0)
{
  oz_declareThread(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setStep();
  else if (OZ_isFalse(yesno))
    thread->unsetStep();
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIsetTraceFlag,2,0)
{
  oz_declareThread(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setTrace();
  else if (OZ_isFalse(yesno))
    thread->unsetTrace();
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

// ------------------

OZ_BI_define(BIbreakpointAt, 3,1)
{
  OZ_declareTerm(0,file)
  OZ_declareInt(1,line);
  OZ_declareTerm(2,what);

  DEREF(file,_1);

  DbgInfo *info = allDbgInfos;
  Bool    ok    = NO;
  char *inFile  = strdup(toC(file));
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
          line = -line;
        CodeArea::writeInt(OZ_int(line),info->PC+2);
      }
    }
    info = info->next;
  }
  OZ_RETURN(ok? OZ_true(): OZ_false());
} OZ_BI_end

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
  if (!a->getPred()) {     // incomplete abstraction
    OZ_RETURN(NameUnit);
  }
  ProgramCounter PC = a->getPred()->getPC();
  ProgramCounter definitionPC = CodeArea::definitionStart(PC);
  if (definitionPC != NOCODE) {
    XReg reg;
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
  OZ_RETURN_INT(th->getID() & THREAD_ID_MASK);
} OZ_BI_end

OZ_BI_define(BIsetThreadID,2,0)
{
  oz_declareThread(0,th);
  oz_declareIntIN(1,id);

  th->setID(id | (1 << THREAD_ID_SIZE));
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIparentThreadID,1,1)
{
  oz_declareThreadIN(0,th);

  OZ_RETURN_INT((th->getID() >> THREAD_ID_SIZE) & THREAD_ID_MASK);
} OZ_BI_end

OZ_BI_define(BIthreadSetRaiseOnBlock,2,0)
{
  oz_declareThread(0,thread);
  oz_declareNonvarIN(1,yesno);

  if (OZ_isTrue(yesno))
    thread->setNoBlock();
  else if (OZ_isFalse(yesno))
    thread->unsetNoBlock();
  else
    oz_typeError(1,"Bool");
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIthreadGetRaiseOnBlock,1,1)
{
  oz_declareThread(0,thread);

  OZ_RETURN(oz_bool(thread->isNoBlock()));
} OZ_BI_end


OZ_BI_define(BIthreadTaskStack,3,1)
{
  oz_declareThreadIN(0,thread);
  oz_declareIntIN(1,depth);
  oz_declareNonvarIN(2,verbose);

  if (thread->isDead())
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
  oz_declareThread(0,thread);
  oz_declareIntIN(1,frameId);

  if (thread->isDead())
    OZ_RETURN(NameUnit);

  TaskStack *taskstack = thread->getTaskStackRef();
  OZ_RETURN(taskstack->getFrameVariables(frameId));
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDebug-if.cc"

#endif
