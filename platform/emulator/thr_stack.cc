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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "taskstk.hh"
#endif

#include "taskstk.hh"
#include "runtime.hh"

int TaskStack::tasks()
{
  /* we do not count the empty task */
  return (tos-array)/frameSz - 1;
}

void TaskStack::checkMax(int n)
{
  int maxSize = getMaxSize();
  if (maxSize >= ozconf.stackMaxSize && ozconf.stackMaxSize!=-1) {
    int newMaxSize = (maxSize*3)/2;

loop:
    printf("\n*** Task stack maxsize exceeded. Increase from %d to %d? (y/n/b/u/?) ",
	   ozconf.stackMaxSize,newMaxSize);
    prefixError();
    fflush(stdout);
    char buf[1000];
    if (osfgets(buf,1000,stdin) == 0) {
      perror("\nofsgets");
      am.exitOz(1);
    }
    switch (buf[0]) {
    case 'n':
      am.exitOz(1);
    case 'u':
      ozconf.stackMaxSize = -1;
      break;
    case 'y':
      ozconf.stackMaxSize = newMaxSize;
      break;

    case '?':
    case 'h':
      printf("\nOptions:\n");
      printf(  "=======\n");
      printf("y = yes, increase stack maxsize\n");
      printf("n = no (will exit Oz)\n");
      printf("b = print a stack dump\n");
      printf("u = unlimited stack size (no further questions)\n");

      goto loop;

    case 'b':
      printTaskStack(ozconf.errorThreadDepth);
      goto loop;
    default:
      goto loop;
    }
  }

  resize(n);
}

static Bool isUninterestingTask(ProgramCounter PC) {
  return
    PC == C_XCONT_Ptr ||
    PC == C_CALL_CONT_Ptr ||
    PC == C_SET_SELF_Ptr ||
    PC == C_SET_ABSTR_Ptr ||
    PC == C_LTQ_Ptr ||
    PC == C_ACTOR_Ptr ||
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
      TaggedRef args = nil();
      if (X) {
	for (int i = getRefsArraySize(X) - 1; i >= 0; i--)
	  args = cons(X[i],args);
      }

      TaggedRef pairlist =
	cons(OZ_pairA("args",args),
	     cons(OZ_pairA("kind",AtomDebugCall),
		  cons(OZ_pairA("thr",makeTaggedConst(thread)),
		       cons(OZ_pairAI("time",CodeArea::findTimeStamp(PC)),
			    cons(OZ_pairA("origin",OZ_atom("builtinFrame")),
				 nil())))));
      if (frameId != -1)
	pairlist = cons(OZ_pairAI("frameID",frameId),pairlist);
      BuiltinTabEntry *biTabEntry = builtinTab.getEntry((void *) Y);
      Assert(biTabEntry != NULL);
      pairlist = cons(OZ_pairA("data",makeTaggedConst(biTabEntry)),pairlist);

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
    // this builtin frame:
    frame = lastframe;
    return makeTaggedNULL();
  } else {
    frame = lastframe;
    return CodeArea::dbgGetDef(PC,definitionPC,verbose);
  }
}

Bool TaskStack::findCatch(ProgramCounter PC, TaggedRef *out, Bool verbose) 
{
  Assert(this);

  if (out) {
    *out = nil();
    if (PC != NOCODE) {
      Frame *auxframe = getTop();
      GetFrame(auxframe,auxPC,auxY,auxG);
      while (isUninterestingTask(auxPC))
	GetFrameNoDecl(auxframe,auxPC,auxY,auxG);
      if (auxPC != C_DEBUG_CONT_Ptr) {
	ProgramCounter definitionPC = CodeArea::definitionStart(PC);
	if (definitionPC != NOCODE) {
	  TaggedRef frameRec = CodeArea::dbgGetDef(PC,definitionPC);
	  if (frameRec != makeTaggedNULL())
	    *out = cons(frameRec,*out);
	}
      }
    }
  }

  while (!isEmpty()) {
    if (out) {
      Frame *frame = getTop();
      TaggedRef frameRec = frameToRecord(frame,am.currentThread(),verbose);
      if (frameRec != makeTaggedNULL())
	*out = cons(frameRec,*out);
    }

    PopFrame(this,PC,Y,G);
    if (PC==C_CATCH_Ptr) {
      if (out) *out = reverseC(*out);
      return TRUE;
    } else if (PC==C_DEBUG_CONT_Ptr) {
      OzDebug *dbg = (OzDebug *) Y;
      dbg->dispose();
    } else if (PC==C_ACTOR_Ptr) {
      Actor *ac = (Actor *) Y;
      ac->discardActor();
      am.currentBoard()->decSuspCount();
    } else if (PC==C_LOCK_Ptr) { 
      OzLock *lck = (OzLock *) Y;
      switch(lck->getTertType()){
      case Te_Local: ((LockLocal*)lck)->unlock();break;
      case Te_Frame: ((LockFrame*)lck)->unlock(am.currentThread());break;
      case Te_Manager: ((LockManager*)lck)->unlock(am.currentThread());break;
      case Te_Proxy: error("lock proxy unlocking\n");break;}
    } else if (PC==C_SET_SELF_Ptr) { 
      Object *newSelf = (Object*)Y;
      am.setSelf(newSelf);
    } else if (PC==C_SET_ABSTR_Ptr) { 
      ozstat.leaveCall((PrTabEntry*)Y);
    }
  }
  if (out) *out = reverseC(*out);
  return FALSE;
}

// for debugging:
void printStack()
{
  am.currentThread()->getTaskStackRef()
    ->printTaskStack(ozconf.errorThreadDepth);
}

void TaskStack::printTaskStack(int depth)
{
  Assert(this);
  if (isEmpty()) {
    message("\tEMPTY\n");
    message("\n");
    return;
  }

  Frame *auxtos = getTop();
  while (auxtos != NULL && (depth == -1 || depth > 0)) {
    GetFrame(auxtos,PC,Y,G);
    if (PC==C_EMPTY_STACK) {
      message("\n");
      return;
    }
    CodeArea::printDef(PC);
    if (depth != -1)
      depth--;
  }
  if (depth == 0)
    message("\t...\n");
  message("\n");
}

TaggedRef TaskStack::getTaskStack(Thread *tt, Bool verbose, int depth) {
  Assert(this);

  TaggedRef out = nil();
  Frame *auxtos = getTop();
  while (auxtos != NULL && (depth > 0 || depth == -1)) {
    TaggedRef frameRec = frameToRecord(auxtos,tt,verbose);
    if (frameRec != makeTaggedNULL()) {
      out = cons(frameRec,out);
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
  if (PC != C_DEBUG_CONT_Ptr)   // inconsistency detected
    return NameUnit;
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
  if (n!=m) {
    if (m>n) printf("#######################\n");
    printf("TaskStack: checkLiveness(%p): unused X detected: %d of %d\n",
	   auxPC,m,n);
  }
}
#endif
