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

unsigned invoc_counter = 0;

int TaskStack::tasks()
{
  /* we do not count the empty task */
  return (tos-array)/frameSz - 1;
}

void TaskStack::pushCall(TaggedRef pred, TaggedRef arg0, TaggedRef arg1, 
			 TaggedRef arg2, TaggedRef arg3, TaggedRef arg4)
{
  static RefsArray a = NULL;
  if (a==NULL)
    a = allocateStaticRefsArray(5);
  int argno = 0;
  if (arg0) argno++;
  if (arg1) argno++;
  if (arg2) argno++;
  if (arg3) argno++;
  if (arg4) argno++;
  a[0] = arg0;
  a[1] = arg1;
  a[2] = arg2;
  a[3] = arg3;
  a[4] = arg4;
  pushCall(pred,a,argno);
}


void TaskStack::checkMax(int n)
{
  int maxSize = getMaxSize();

  if (maxSize >= ozconf.stackMaxSize && ozconf.stackMaxSize!=-1) {
    
    int newMaxSize = (maxSize*3)/2;
    
    if (ozconf.runningUnderEmacs) {
      printf("\n*** Task stack maxsize exceeded. Increasing from %d to %d.\n",
	     ozconf.stackMaxSize,newMaxSize);
      prefixError();
      fflush(stdout);
    }

    ozconf.stackMaxSize = newMaxSize;
  }

  resize(n);
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
    OzDebugDoit dothis = (OzDebugDoit) (int) G;
    return dbg->toRecord((dothis == DBG_EXIT)?"exit":"entry",thread,frameId);
  }

  if (PC == C_CATCH_Ptr) {
    GetFrame(frame,auxPC,auxY,auxG);   // ignore the handler continuation
    return makeTaggedNULL();
  }

  if (PC == C_CFUNC_CONT_Ptr) {
    // Alas, this frame is not pushed for builtins called as
    // inline-builtins (inlineFun, inlineRel), so if the examined
    // code was not executed with debug information and in debugmode,
    // it will not be shown on the stack.
    Frame *auxframe = frame, *lastframe;
    GetFrame(auxframe,auxPC,auxY,auxG);
    while (isUninterestingTask(auxPC))
      GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
    if (auxPC == C_EMPTY_STACK) {
      frame = NULL;
      return makeTaggedNULL();
    }
    // now we also ignore the frame with the next normal continuation,
    // to see whether it is followed by a debug frame:
    lastframe = auxframe;
    GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
    while (isUninterestingTask(auxPC)) {
      lastframe = auxframe;
      GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
    }
    if (auxPC == C_EMPTY_STACK) {
      frame = NULL;
      return makeTaggedNULL();
    } else if (auxPC == C_DEBUG_CONT_Ptr) {
      // the OzDebug in the next stack frame has more to tell than
      // this builtin-application frame:
      frame = lastframe;
      return makeTaggedNULL();
    } else {
      frame = auxframe;

      RefsArray X = (RefsArray) G;
      TaggedRef args = oz_nil();
      if (X) {
	for (int i = getRefsArraySize(X) - 1; i >= 0; i--)
	  args = oz_cons(X[i],args);
      }

      TaggedRef pairlist =
	oz_cons(OZ_pairA("args",args),
	     oz_cons(OZ_pairA("kind",OZ_atom("call")),
		  oz_cons(OZ_pairA("thr",oz_thread(thread)),
		       oz_cons(OZ_pairAI("time",CodeArea::findTimeStamp(PC)),
			    oz_cons(OZ_pairA("origin",OZ_atom("builtinFrame")),
				 oz_nil())))));
      if (frameId != -1)
	pairlist = oz_cons(OZ_pairAI("frameID",frameId),pairlist);

      pairlist = oz_cons(OZ_pairA("data", makeTaggedConst(cfunc2Builtin((void *) Y))),
			 pairlist);

      return OZ_recordInit(OZ_atom("entry"), pairlist);
    }
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

void splitfname(const char * fname, char * &dirname, char * &basename);

TaggedRef TaskStack::findAbstrRecord(void)
{
  Frame * frame = getTop();
  PrTabEntry * abstr = NULL;

  while (1) {
    GetFrame(frame,PC,Y,G);
    
    if (PC == C_EMPTY_STACK) {
      frame = NULL;
      return NameUnit;
    }
    
    if (PC == C_DEBUG_CONT_Ptr) {
      OzDebug *dbg = (OzDebug *) Y;
      abstr = dbg->CAP->getPred();
    }

    if (PC == C_SET_ABSTR_Ptr && abstr != NULL) {
      unsigned invoc_counter = (unsigned) G;

      
      const char * fname = OZ_atomToC(abstr->getFile());
      char * dirname, * basename;
      
      splitfname(fname, dirname, basename);

      OZ_Term prop_loc = 
	OZ_record(AtomPropInvoc, 
		  OZ_cons(AtomName, 
			  OZ_cons(AtomFile, 
				  OZ_cons(AtomLine,
					  OZ_cons(AtomColumn,
						  OZ_cons(AtomPath,
							  OZ_cons(AtomInvoc,
								  OZ_nil())))))));
      OZ_putSubtree(prop_loc, AtomName, abstr->getName());
      OZ_putSubtree(prop_loc, AtomPath, OZ_atom(dirname));
      OZ_putSubtree(prop_loc, AtomFile, OZ_atom(basename));
      OZ_putSubtree(prop_loc, AtomLine, OZ_int(abstr->getLine()));
      OZ_putSubtree(prop_loc, AtomColumn, OZ_int(abstr->getColumn()));
      OZ_putSubtree(prop_loc, AtomInvoc, OZ_int(invoc_counter));
      
      return prop_loc;
    }
  }
}

Bool TaskStack::findCatch(Thread *thr, ProgramCounter PC, 
			  RefsArray Y, Abstraction *G,
			  TaggedRef *out, Bool verbose) 
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
      return TRUE;
    } else if (PC==C_DEBUG_CONT_Ptr) {
      OzDebug *dbg = (OzDebug *) Y;
      dbg->dispose();
    } else if (PC==C_LOCK_Ptr) { 
      OzLock *lck = (OzLock *) Y;
      switch(lck->getTertType()){
      case Te_Local: 
	if (((LockLocal*)lck)->hasLock(thr))
	  ((LockLocal*)lck)->unlock();
	break;
      case Te_Frame: 
	if (((LockFrameEmul*)lck)->hasLock(thr))
	  ((LockFrameEmul*)lck)->unlock(thr);
	break;
      case Te_Manager: 
	if (((LockManagerEmul*)lck)->hasLock(thr))
	  ((LockManagerEmul*)lck)->unlock(thr);
	break;
      case Te_Proxy: OZ_error("lock proxy unlocking\n");break;}
    } else if (PC==C_SET_SELF_Ptr) { 
      Object *newSelf = (Object*)Y;
      thr->setSelf(newSelf);
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

  OzDebugDoit dothislater = DBG_NOSTEP;
  Frame *auxtos = getTop();
  while (auxtos != NULL) {
    if (getFrameId(auxtos) <= frameId)
      dothislater = DBG_STEP;
    GetFrame(auxtos,PC,Y,G);
    if (PC == C_DEBUG_CONT_Ptr) {
      OzDebugDoit dothis = (OzDebugDoit) (int) G;
      if (dothis != DBG_EXIT)
	ReplaceFrame(auxtos,PC,Y,dothislater);
    } else if (PC == C_EMPTY_STACK)
      return;
  }
}

#ifdef DEBUG_LIVENESS
// set unused values in X to zero
void TaskStack::checkLiveness(RefsArray X) {
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
