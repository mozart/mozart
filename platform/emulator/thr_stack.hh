/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Stack of tasks in threads
  ------------------------------------------------------------------------
*/

#ifndef __TASKSTACKH
#define __TASKSTACKH

#ifdef INTERFACE
#pragma interface
#endif


extern ProgramCounter
  C_XCONT_Ptr          , // a continuation with    X registers
  C_CFUNC_CONT_Ptr     , // a continuation  to call a c-function
  C_DEBUG_CONT_Ptr     , // a continuation for debugging
  C_CALL_CONT_Ptr      , // an application
  C_LOCK_Ptr           , //
  C_ACTOR_Ptr          , // an actor task
  C_SET_SELF_Ptr       , // set am.ooRegisters
  C_LTQ_Ptr            , // local thread queue
  C_CATCH_Ptr          , // exception handler
  C_EMPTY_STACK        ;

typedef StackEntry TaskStackEntry;

#define frameSz TASKFRAMESIZE

class TaskStack: public Stack {
private:

  StackEntry *ensureFree(int n)
  {
    StackEntry *ret = tos;
    if (stackEnd <= tos+n) {
      checkMax();
      resize(n);
      ret = tos;
    }
    return ret;
  }

public:
  void pushFrame(ProgramCounter pc,void *y, void *g)
  {
    TaskStackEntry *newTop = ensureFree(frameSz);
    *(newTop)   = g;
    *(newTop+1) = y;
    *(newTop+2) = pc;

    tos = newTop+frameSz;
    CountMax(maxStackDepth,(tos-array)*sizeof(StackEntry));
  }


  USEFREELISTMEMORY;

  int suggestNewSize() {
      int used = getUsed();
      return max(ozconf.stackMinSize,
                 min(used * 2, (getMaxSize() + used + 1) >> 1));
  }

  void restoreFrame() { tos += frameSz; Assert(tos<stackEnd); }
  Bool checkFrame(ProgramCounter pc)
  {
    return (ProgramCounter)*(tos-1)==pc;
  }
  void discardFrame(ProgramCounter pc)
  {
    Assert(pc==NOCODE || checkFrame(pc));
    tos -= frameSz;
  }

  void makeEmpty()
  {
    mkEmpty();
    pushFrame(C_EMPTY_STACK,0,0);
  }


  TaskStack(int s): Stack(s,Stack_WithFreelist) { makeEmpty(); }
  ~TaskStack() { Assert(0); }

  void printTaskStack(ProgramCounter pc = NOCODE,
                      Bool verbose = NO, int depth = 10000);
  TaggedRef dbgGetTaskStack(ProgramCounter pc = NOCODE, int depth = 10000,
                            TaskStackEntry *top = NULL);
  TaggedRef dbgFrameVariables(int frameId);

  Bool isEmpty() { return *(tos-1)==C_EMPTY_STACK; }

  void checkMax();

  TaskStackEntry *getTop()            { return tos; }
  void setTop(TaskStackEntry *newTos) { tos = newTos; }

  void gc(TaskStack *newstack);

  Bool findCatch();
  TaggedRef reflect(TaskStackEntry *from=0,TaskStackEntry *to=0,
                    ProgramCounter pc=NOCODE);


  void pushCFunCont(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    Assert(MemChunks::areRegsInHeap(x, i));

    pushFrame(C_CFUNC_CONT_Ptr,(void*)f,
              i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL);
  }

  void pushCont(Continuation *cont)
  {
    pushCont(cont->getPC(),cont->getY(),cont->getG(),cont->getX());
  }

  void pushCont(ProgramCounter pc,RefsArray y,RefsArray g,RefsArray x)
  {
    Assert(!isFreedRefsArray(y));

    Assert(!x || MemChunks::areRegsInHeap(x,getRefsArraySize(x)));
    Assert(!y || MemChunks::areRegsInHeap(y,getRefsArraySize(y)));
    Assert(!g || MemChunks::areRegsInHeap(g,getRefsArraySize(g)));

    pushFrame(pc,y,g);
    if (x)
      pushFrame(C_XCONT_Ptr,x,0);
  }

  void pushCall(TaggedRef pred, RefsArray  x, int i)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    pushFrame(C_CALL_CONT_Ptr, ToPointer(pred), i>0 ? copyRefsArray(x, i) : NULL);
  }

  void pushLTQ(SolveActor *sa)   { pushFrame(C_LTQ_Ptr,sa,0); }
  void pushLock(OzLock *lck)     { pushFrame(C_LOCK_Ptr,lck,0); }
  void pushCatch()               { pushFrame(C_CATCH_Ptr,0,0); }
  void pushDebug(OzDebug *deb)   { pushFrame(C_DEBUG_CONT_Ptr,deb,0); }
  void pushSelf(Object *o)       { pushFrame(C_SET_SELF_Ptr,o,NULL); }
  void pushActor(Actor *aa)      { pushFrame(C_ACTOR_Ptr,aa,0); }

  int tasks();
};



#define PopFrameNoDecl(top,pc,y,g)              \
    pc   = (ProgramCounter) *(top-1);           \
    y    = (RefsArray) *(top-2);                \
    g    = (RefsArray) *(top-3);                \
    top -= frameSz;

#define PopFrame(top,pc,y,g)                    \
    ProgramCounter pc;                          \
    RefsArray y,g;                              \
    PopFrameNoDecl(top,pc,y,g)


#endif
