/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __TASKSTACKH
#define __TASKSTACKH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "ozconfig.hh"
#include "stack.hh"
#include "tagged.hh"
#include "refsarray.hh"
#include "value.hh"

extern ProgramCounter
  C_XCONT_Ptr          , // a continuation with    X registers
  C_DEBUG_CONT_Ptr     , // a continuation for debugging
  C_CALL_CONT_Ptr      , // an application
  C_LOCK_Ptr           , // 
  C_SET_SELF_Ptr       , // set am.ooRegisters
  C_SET_ABSTR_Ptr      ,
  C_CATCH_Ptr          , // exception handler
  C_EMPTY_STACK        ;


typedef StackEntry Frame;

inline
int isEmpty(Frame *tos) {
  return *(tos-1)==C_EMPTY_STACK;
}

#define frameSz TASKFRAMESIZE

extern unsigned invoc_counter;

class TaskStack: public Stack {
private:

  StackEntry *ensureFree(int n) {
    StackEntry *ret=tos;
    if (stackEnd <= ret+n) {
      resize(n);
      ret=tos;
    }
    return ret;
  }

  int suggestNewSize() {
      int used = getUsed();
      return max(ozconf.stackMinSize,
		 min(used * 2, (getMaxSize() + used + 1) >> 1));
  }

public:
  USEFREELISTMEMORY;
  NO_DEFAULT_CONSTRUCTORS(TaskStack);

  int getFrameId(Frame *frame) {
    return frame - array;
  }

  int getFrameId() {
    return getFrameId(tos);
  }


  void restoreFrame() { 
    tos += frameSz; Assert(tos<stackEnd); 
  }
  Bool checkFrame(ProgramCounter pc) { 
    return (ProgramCounter)*(tos-1)==pc;
  }
  void discardFrame(ProgramCounter pc) {
    Assert(pc==NOCODE || checkFrame(pc));
    tos -= frameSz; 
  }

  void printTaskStack(int depth);
  TaggedRef getTaskStack(Thread *tt, Bool verbose, int depth);
  TaggedRef getFrameVariables(int frameId);
  void unleash(int frameId);

  Bool isEmpty() { return ::isEmpty(tos); }

  Frame *getTop()            { return tos; }
  void setTop(Frame *newTos) { tos = newTos; }

  TaskStack * gCollect(void);
  TaskStack * sClone(void);

  TaggedRef frameToRecord(Frame *&frame, Thread *thread, Bool verbose);
  TaggedRef findAbstrRecord(void);

  Bool findCatch(Thread *thr, 
		 ProgramCounter PC=NOCODE, RefsArray * Y=NULL,
		 Abstraction *G=NULL,
		 TaggedRef *traceBack=0, Bool verbose=NO);


  void checkLiveness(RefsArray * X);

  // kost@ : Note that 'cap' is stored now as a naked pointer. 
  // The GC has to derive its type based on the task type;
  void pushFrame(ProgramCounter pc, void *y, void *cap) {
    Frame *newTop = ensureFree(frameSz);
    *(newTop)   = cap;
    *(newTop+1) = y; 
    *(newTop+2) = pc;
    tos = newTop+frameSz;
  }

  void pushEmpty() {
    pushFrame(C_EMPTY_STACK, 0, 0);
  }

  void init() {
    mkEmpty();
    pushEmpty();
  }

  void pushCont(ProgramCounter pc, RefsArray *y, Abstraction *cap) {
#ifdef DEBUG_MEM
    Assert(!y || MemChunks::areRegsInHeap(y->getArgsRef(), y->getLen()));
#endif
    Assert(cap);
    pushFrame(pc, y, cap);
  }

  void pushCall(TaggedRef pred, RefsArray * x) {
    pushFrame(C_CALL_CONT_Ptr, (void *) pred, x);
#ifdef DEBUG_MEM
    Assert(!x || MemChunks::areRegsInHeap(x->getArgsRef(), x->getLen()));
#endif
  }
  void pushX(int i) {
    Assert(i >= 0);
    if (i > 0) {
      RefsArray * x = RefsArray::make(XREGS, i);
#ifdef DEBUG_LIVENESS
      checkLiveness(x);
#endif
#ifdef DEBUG_MEM
    Assert(MemChunks::areRegsInHeap(x->getArgsRef(), x->getLen()));
#endif
      pushFrame(C_XCONT_Ptr, x, 0);
    }
  }

  void pushLock(OzLock *lck)     { 
    Assert(lck);
    pushFrame(C_LOCK_Ptr, 0, lck); 
  }
  void pushCatch()               { 
    pushFrame(C_CATCH_Ptr, 0, 0);
  }
  void discardCatch()            { 
    discardFrame(C_CATCH_Ptr); 
  }
  void pushDebug(OzDebug *dbg, Atom *dothis) { 
    pushFrame(C_DEBUG_CONT_Ptr, dbg, dothis); 
  }
  void pushSelf(Object *o) { 
    pushFrame(C_SET_SELF_Ptr, 0, o);
  }
  void pushAbstr(PrTabEntry  *a) { 
    pushFrame(C_SET_ABSTR_Ptr, a, ToPointer(invoc_counter++)); 
  }
  int tasks();

  TaskStack(int s): Stack(s, Stack_WithFreelist) { pushEmpty(); }
  TaskStack(TaskStack * f) : Stack(f->suggestNewSize(),Stack_WithFreelist) {
    int used = f->getUsed();
    memcpy(array, f->array, used * sizeof(Frame));
    tos += used;
  }

};

#define GetFrameNoDecl(top,pc,y,cap)		\
{						\
  pc   = (ProgramCounter) *(top-1);		\
  y    = (RefsArray *)    *(top-2);		\
  cap  = (Abstraction *)  *(top-3);		\
  top -= frameSz;				\
}

#define ReplaceFrame(frame,pc,y,cap)            \
{                                               \
  *(frame+2) = (void *) pc;                     \
  *(frame+1) = (void *) y;                      \
  *frame     = (void *) cap;                    \
}

#define PopFrameNoDecl(ts,pc,y,cap)		\
{						\
  TaskStack *__ts = ts;				\
  Frame *__top = __ts->getTop();		\
  GetFrameNoDecl(__top,pc,y,cap)		\
  __ts->setTop(__top);				\
}

#define PopFrame(ts,pc,y,cap)			\
    ProgramCounter pc;				\
    RefsArray * y;				\
    Abstraction *cap;				\
    PopFrameNoDecl(ts,pc,y,cap)

#define GetFrame(top,pc,y,cap)			\
    ProgramCounter pc;				\
    RefsArray * y;				\
    Abstraction *cap;				\
    GetFrameNoDecl(top,pc,y,cap)

#endif

