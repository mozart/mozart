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

#ifndef __TASKSTACKH
#define __TASKSTACKH

#ifdef INTERFACE
#pragma interface
#endif

#include "types.hh"
#include "stack.hh"
#include "tagged.hh"

extern ProgramCounter
  C_XCONT_Ptr          , // a continuation with    X registers
  C_CFUNC_CONT_Ptr     , // a continuation  to call a c-function
  C_DEBUG_CONT_Ptr     , // a continuation for debugging
  C_CALL_CONT_Ptr      , // an application
  C_LOCK_Ptr           , // 
  C_SET_SELF_Ptr       , // set am.ooRegisters
  C_SET_ABSTR_Ptr      ,
  C_LTQ_Ptr            , // local thread queue
  C_CATCH_Ptr          , // exception handler
  C_ACTOR_Ptr          , // actor
  C_EMPTY_STACK        ;


typedef StackEntry Frame;

inline
int isEmpty(Frame *tos) {
  return *(tos-1)==C_EMPTY_STACK;
}

#define frameSz TASKFRAMESIZE

class TaskStack: public Stack {
private:

  StackEntry *ensureFree(int n)
  {
    StackEntry *ret=tos;
    if (stackEnd <= ret+n) {
      checkMax(n);
      ret=tos;
    }
    return ret;
  }

public:
  void pushFrame(ProgramCounter pc,void *y, void *g)
  {
    Frame *newTop = ensureFree(frameSz);
    *(newTop)   = g;
    *(newTop+1) = y; 
    *(newTop+2) = pc;
    
    tos = newTop+frameSz;
    CountMax(maxStackDepth,(tos-array)*sizeof(StackEntry));
  }

  int getFrameId(Frame *frame) {
    return frame - array;
  }

  int getFrameId() {
    return getFrameId(tos);
  }

  USEFREELISTMEMORY;

  int suggestNewSize() {
      int used = getUsed();
      return max(ozconf.stackMinSize,
		 min(used * 2, (getMaxSize() + used + 1) >> 1));
  }

  void restoreFrame() { tos += frameSz; Assert(tos<stackEnd); }
  Bool checkFrame(ProgramCounter pc) { 
    return (ProgramCounter)*(tos-1)==pc;
  }
  void discardFrame(ProgramCounter pc) {
    Assert(pc==NOCODE || checkFrame(pc));
    tos -= frameSz; 
  }

  void pushEmpty() {
    pushFrame(C_EMPTY_STACK,0,0);
  }

  void makeEmpty() {
    mkEmpty();
    pushEmpty();
  }
  void init() {
    mkEmpty();
    pushEmpty();
  }

  TaskStack(int s): Stack(s,Stack_WithFreelist) { pushEmpty(); }
  ~TaskStack() { Assert(0); }

  void printTaskStack(int depth);
  TaggedRef getTaskStack(Thread *tt, Bool verbose, int depth);
  TaggedRef getFrameVariables(int frameId);
  void unleash(int frameId);

  Bool isEmpty() { return ::isEmpty(tos); }

  void checkMax(int n);

  Frame *getTop()            { return tos; }
  void setTop(Frame *newTos) { tos = newTos; }

  void gc(TaskStack *newstack);

  TaggedRef frameToRecord(Frame *&frame, Thread *thread, Bool verbose);

  Bool findCatch(ProgramCounter PC=NOCODE, TaggedRef *traceBack=0,
		 Bool verbose=NO);


  void checkLiveness(RefsArray X);

  void pushX(RefsArray X) {
#ifdef DEBUG_LIVENESS
    checkLiveness(X);
#endif
    Assert(X);
    pushFrame(C_XCONT_Ptr,X,0);
  }

  void pushX(RefsArray X, int i) {
    if (i>0) {
      RefsArray x=copyRefsArray(X,i);
      Assert(MemChunks::areRegsInHeap(x,getRefsArraySize(x)));
      pushX(x);
    }
  }

  void pushCFun(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    Assert(MemChunks::areRegsInHeap(x, i));

    pushFrame(C_CFUNC_CONT_Ptr,(void*)f,
	      i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL);
  }
  
  void pushCont(ProgramCounter pc,RefsArray y,RefsArray g)
  {
    Assert(!isFreedRefsArray(y));

    Assert(!y || MemChunks::areRegsInHeap(y,getRefsArraySize(y)));
    Assert(!g || MemChunks::areRegsInHeap(g,getRefsArraySize(g)));
	       
    pushFrame(pc,y,g);
  }

  void pushCall(TaggedRef pred, RefsArray  x, int i)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    pushFrame(C_CALL_CONT_Ptr, ToPointer(pred), i>0 ? copyRefsArray(x, i) : NULL);
  }

  void pushCallNoCopy(TaggedRef pred, RefsArray  x)
  {
    pushFrame(C_CALL_CONT_Ptr, ToPointer(pred), x);
  }

  void pushLTQ(SolveActor *sa)   { pushFrame(C_LTQ_Ptr,sa,0); }
  void pushLock(OzLock *lck)     { pushFrame(C_LOCK_Ptr,lck,0); }
  void pushCatch()               { pushFrame(C_CATCH_Ptr,0,0); }
  void discardCatch()            { discardFrame(C_CATCH_Ptr); }
  void pushDebug(OzDebug *dbg, OzDebugDoit dothis)
				 { pushFrame(C_DEBUG_CONT_Ptr,dbg,
					     (void *)dothis); }
  void pushSelf(Object *o)       { pushFrame(C_SET_SELF_Ptr,o,NULL); }
  void pushAbstr(PrTabEntry  *a) { pushFrame(C_SET_ABSTR_Ptr,a,NULL); }
  void pushActor(Actor *ac, ProgramCounter PC)
				 { pushFrame(C_ACTOR_Ptr,ac,PC); }
  void discardActor()            { discardFrame(C_ACTOR_Ptr); }

  int tasks();
};

#define GetFrameNoDecl(top,pc,y,g)		\
{						\
  pc   = (ProgramCounter) *(top-1);		\
  y    = (RefsArray) *(top-2);			\
  g    = (RefsArray) *(top-3);			\
  top -= frameSz;				\
}

#define ReplaceFrame(frame,pc,y,g)              \
{                                               \
  *(frame+2) = (void *) pc;                     \
  *(frame+1) = (void *) y;                      \
  *frame     = (void *) g;                      \
}

#define PopFrameNoDecl(ts,pc,y,g)		\
{						\
  TaskStack *__ts = ts;				\
  Frame *__top = __ts->getTop();		\
  GetFrameNoDecl(__top,pc,y,g)			\
  __ts->setTop(__top);				\
}

#endif

#define PopFrame(ts,pc,y,g)			\
    ProgramCounter pc;				\
    RefsArray y,g;				\
    PopFrameNoDecl(ts,pc,y,g)

#define GetFrame(top,pc,y,g)			\
    ProgramCounter pc;				\
    RefsArray y,g;				\
    GetFrameNoDecl(top,pc,y,g)
