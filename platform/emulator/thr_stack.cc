/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thr_stack.hh"
#endif

#include "thr_stack.hh"
#include "os.hh"
#include "value.hh"
#include "am.hh"
#include "codearea.hh"
#include "board.hh"
#include "debug.hh"
#include "dpInterface.hh"
#include "boot-manager.hh"

unsigned invoc_counter = 1000;

int TaskStack::tasks()
{
  /* we do not count the empty task */
  return (tos-array)/frameSz - 1;
}

static Bool isUninterestingTask(ProgramCounter PC) {
  return
    PC == C_XCONT_Ptr ||
    PC == C_CALL_CONT_Ptr ||
    PC == C_SET_SELF_Ptr ||
    PC == C_SET_ABSTR_Ptr ||
    PC == C_CATCH_Ptr;
}


TaggedRef TaskStack::frameToRecord(Frame *&frame, Thread *thread, Bool verbose)
{
  int frameId = verbose? -1: getFrameId(frame);
  GetFrame(frame,PC,Y,G);

  if (PC == C_EMPTY_STACK) {
    frame = NULL;
    return makeTaggedNULL();
  }

  if (PC == C_DEBUG_CONT_Ptr) {
    OzDebug *dbg = (OzDebug *) Y;
    Atom * dothis = (Atom *) (int) G;
    return dbg->toRecord((dothis == DBG_EXIT_ATOM)? AtomExit: AtomEntry,
                         thread,frameId);
  }

  if (PC == C_CATCH_Ptr) {
    GetFrame(frame,auxPC,auxY,auxG);   // ignore the handler continuation
    return makeTaggedNULL();
  }

  ProgramCounter definitionPC = CodeArea::definitionStart(PC);
  if (definitionPC == NOCODE)
    return makeTaggedNULL();

  Frame *auxframe = frame, *lastframe = frame;
  GetFrame(auxframe,auxPC,auxY,auxG);
  while (isUninterestingTask(auxPC)) {
    lastframe = auxframe;
    GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
  }
  if (auxPC == C_EMPTY_STACK) {
    frame = NULL;
    return makeTaggedNULL();
  } else if (auxPC == C_LOCK_Ptr) {
    // the continuation we found belongs to this lock
    // (note that we may lose information when executing
    // code without debug information, but this saves us from
    // stack corruptions in Ozcar)
    frame = auxframe;
    return makeTaggedNULL();
  } else if (auxPC == C_DEBUG_CONT_Ptr) {
    // the OzDebug in the next stack frame has more to tell than
    // this vanilla frame:
    frame = lastframe;
    return makeTaggedNULL();
  } else {
    frame = lastframe;
    return CodeArea::dbgGetDef(PC,definitionPC,frameId,Y,G);
  }
}

void splitfname(const char * fname, const char * &dirname, const char * &basename);

TaggedRef TaskStack::findAbstrRecord(void)
{
  Frame * frame = getTop();
  PrTabEntry * abstr = NULL;
  OZ_Term return_value = NameUnit;

  while (1) {
    GetFrame(frame,PC,Y,G);
    //
    if (PC == C_EMPTY_STACK) {
      frame = NULL;
      return return_value;
    }
    if (PC == C_DEBUG_CONT_Ptr) {
      OzDebug *dbg = (OzDebug *) Y;
      abstr = ((Abstraction *) tagged2Const(dbg->CAP))->getPred();
    }
    //
    if (PC == C_SET_ABSTR_Ptr && abstr != NULL &&
        strcmp(abstr->getPrintName(), "")) {
      unsigned invoc_counter = (unsigned) ToInt32(G);
      //
      if (return_value == NameUnit) {
        // retrieve information for enclosing procedure
        const char * fname = OZ_atomToC(abstr->getFile());
        const char * dirname, * basename;
        //
        splitfname(fname, dirname, basename);
        //
        return_value =
          OZ_record(AtomPropInvoc,
                    OZ_cons(AtomName,
                            OZ_cons(AtomCallerInvoc,
                                    OZ_cons(AtomFile,
                                            OZ_cons(AtomLine,
                                                    OZ_cons(AtomColumn,
                                                            OZ_cons(AtomPath,
                                                                    OZ_cons(AtomInvoc,
                                                                            oz_nil()))))))));
        //
        OZ_putSubtree(return_value, AtomName, abstr->getName());
        OZ_putSubtree(return_value, AtomPath, OZ_atom(dirname));
        OZ_putSubtree(return_value, AtomFile, OZ_atom(basename));
        OZ_putSubtree(return_value, AtomLine, OZ_int(abstr->getLine()));
        OZ_putSubtree(return_value, AtomColumn, OZ_int(abstr->getColumn()));
        OZ_putSubtree(return_value, AtomInvoc, OZ_int(invoc_counter));
        OZ_putSubtree(return_value, AtomCallerInvoc, NameUnit);
      } else {
        // retrieve information for procedure calling enclosing procedure
        OZ_putSubtree(return_value, AtomCallerInvoc, OZ_int(invoc_counter));
        return return_value;
      }
    }
  }
}

Bool TaskStack::findCatch(Thread *thr,
                          ProgramCounter PC,
                          RefsArray *Y, Abstraction *G,
                          TaggedRef *out,
                          Bool verbose)
{
  Assert(this);

  if (out) {
    *out = oz_nil();
    if (PC != NOCODE) {
      Frame *auxframe = getTop();
      GetFrame(auxframe,auxPC,auxY,auxG);
      while (isUninterestingTask(auxPC))
        GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
      if (auxPC != C_DEBUG_CONT_Ptr) {
        ProgramCounter definitionPC = CodeArea::definitionStart(PC);
        if (definitionPC != NOCODE) {
          TaggedRef frameRec =
            CodeArea::dbgGetDef(PC,definitionPC,-1,Y,G);
          if (frameRec != makeTaggedNULL())
            *out = oz_cons(frameRec,*out);
        }
      }
    }
  }

  OzObject * foundSelf = (OzObject *) NULL;

  while (!isEmpty()) {
    if (out) {
      Frame *frame = getTop();
      TaggedRef frameRec = frameToRecord(frame,thr,verbose);
      if (frameRec != makeTaggedNULL())
        *out = oz_cons(frameRec,*out);
    }

    PopFrame(this,PC,Y,G);
    if (PC==C_CATCH_Ptr) {
      if (out) *out = reverseC(*out);

      // If there was a set self, push it back!
      if (foundSelf)
        pushSelf(foundSelf);

      return TRUE;
    } else if (PC==C_DEBUG_CONT_Ptr) {
      OzDebug *dbg = (OzDebug *) Y;
      dbg->dispose();
    } else if (PC==C_LOCK_Ptr) {
      OzLock *lck = (OzLock *) G;
      lockRelease(lck);
    } else if (PC==C_SET_SELF_Ptr) {
      foundSelf = (OzObject*) G;
    } else if (PC==C_SET_ABSTR_Ptr) {
      ozstat.leaveCall((PrTabEntry*)Y);
    }
  }

  if (out) *out = reverseC(*out);
  return FALSE;
}

#ifdef DEBUG_PRINT
// for debugging:
void ozd_printStack()
{
  oz_currentThread()->printLong();
}

#endif

#ifdef DEBUG_CHECK

void TaskStack::printTaskStack(int depth)
{
  Assert(this);
  if (isEmpty()) {
    fprintf(stderr,"*** EMPTY\n");
    fprintf(stderr,"***\n");
    fflush(stderr);
    return;
  }

  Frame *auxtos = getTop();
  while (auxtos != NULL && (depth == -1 || depth > 0)) {
    GetFrame(auxtos,PC,Y,G);
    if (PC==C_EMPTY_STACK) {
      fprintf(stderr,"***\n");
      fflush(stderr);
      return;
    }
    fprintf(stderr,"*** PC: %p Y: %p, G: %p\n",PC,Y,G);
    CodeArea::printDef(PC,stderr);
    if (depth != -1)
      depth--;
  }
  if (depth == 0)
    fprintf(stderr,"*** ...\n");
  fprintf(stderr,"***\n");
  fflush(stderr);
}

#endif

TaggedRef TaskStack::getTaskStack(Thread *tt, Bool verbose, int depth) {
  Assert(this);

  TaggedRef out = oz_nil();
  Frame *auxtos = getTop();
  while (auxtos != NULL && (depth > 0 || depth == -1)) {
    TaggedRef frameRec = frameToRecord(auxtos,tt,verbose);
    if (frameRec != makeTaggedNULL()) {
      out = oz_cons(frameRec,out);
      if (depth != -1)
        depth--;
    }
  }

  return reverseC(out);
}

TaggedRef TaskStack::getFrameVariables(int frameId) {
  Assert(this);

  if (frameId < 0 || frameId % frameSz != 0)   // incorrect frame ID
    return NameUnit;
  Frame *frame = array + frameId;
  if (frame > tos)   // the frame does not exist any longer
    return NameUnit;
  GetFrame(frame,PC,Y,G);
  if (PC != C_DEBUG_CONT_Ptr)
    return CodeArea::getFrameVariables(PC,Y,G);
  OzDebug *dbg = (OzDebug *) Y;
  return dbg->getFrameVariables();
}

void TaskStack::unleash(int frameId) {
  Assert(this);

  Atom * dothislater = DBG_NOSTEP_ATOM;
  Frame *auxtos = getTop();
  while (auxtos != NULL) {
    if (getFrameId(auxtos) <= frameId)
      dothislater = DBG_STEP_ATOM;
    GetFrame(auxtos,PC,Y,G);
    if (PC == C_DEBUG_CONT_Ptr) {
      Atom * dothis = (Atom *) (int) G;
      if (dothis != DBG_EXIT_ATOM)
        ReplaceFrame(auxtos,PC,Y,dothislater);
    } else if (PC == C_EMPTY_STACK)
      return;
  }
}

#ifdef DEBUG_LIVENESS
// set unused values in X to zero
void TaskStack::checkLiveness(RefsArray * X) {
  PopFrame(this,auxPC,auxY,auxG);
  pushFrame(auxPC,auxY,auxG);
  int n=getRefsArraySize(X);
  int m=CodeArea::livenessX(auxPC,X,n);
  if (m>n) {
    printf("TaskStack: checkLiveness(%p): unused X detected: %d of %d\n",
           auxPC,m,n);
  }
}
#endif
