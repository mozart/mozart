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

/*
 * NEW_STACK is an experimental feature.
 * If defined the thread stack is represented as linked list
 * (Michael)
 *
 * usage 'ozmkmf -DNEW_STACK'
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
  C_SET_ABSTR_Ptr      ,
  C_LTQ_Ptr            , // local thread queue
  C_CATCH_Ptr          , // exception handler
  C_EMPTY_STACK        ;


#ifdef NEW_STACK
class TaskStack;

class Frame {
public:
  friend class TaskStack;
private:
  Frame *next;
  ProgramCounter pc;
  void *y;
  void *g;

public:
  USEFREELISTMEMORY;
  Frame() {}
  Frame(ProgramCounter pc,void *y, void *g, Frame *next)
    : next(next), pc(pc), y(y), g(g) {}

  ProgramCounter getPC() const { return pc; }
  Frame *getNext() const       { return next; }
  RefsArray getY() const       { return (RefsArray) y; }
  RefsArray getG() const       { return (RefsArray) g; }

  Frame *dispose() {
    Frame *ret = getNext();
    freeListDispose(this,sizeof(Frame));
    return ret;
  }
  void disposeAll() {
    for (Frame *f=this; f; f = f->dispose()) ;
  }

  int count() const {
    int i=0;
    for (const Frame *f=this; f; f=f->getNext()) {
      i++;
    }
    return i;
  }
};

inline
int isEmpty(Frame *f) { return f->getPC()==C_EMPTY_STACK; }

class TaskStack {
private:
  Frame *tos;

public:
  USEFREELISTMEMORY;

  ~TaskStack() { Assert(0); }

  Frame *getTop() { return tos; }
  void setTop(Frame *t) { tos=t; }

  void pushFrame(ProgramCounter pc,void *y, void *g)
  {
    Frame *fr = new Frame();
    fr->next=tos;
    tos=fr;
    fr->pc=pc;
    fr->y=y;
    fr->g=g;
    // mm2 CountMax(maxStackDepth,(tos-array)*sizeof(StackEntry));
  }

  Bool checkFrame(ProgramCounter pc)
  {
    return tos->getPC()==pc;
  }

  void discardFrame(ProgramCounter pc)
  {
    Assert(pc==NOCODE || checkFrame(pc));
    tos = tos->dispose();
  }

  void pushEmpty() {
    Assert(!tos);
    pushFrame(C_EMPTY_STACK,0,0);
  }

  TaskStack() {
    tos=0;
    pushEmpty();
  }

  void makeEmpty()
  {
    if (tos) {
      tos->disposeAll();
    }
    init();
  }
  void init()
  {
    tos=0;
    pushEmpty();
  }
  Bool isEmpty() { return ::isEmpty(tos); }

  void printTaskStack(ProgramCounter pc = NOCODE,
                      Bool verbose = NO, int depth = 1000);
  TaggedRef dbgGetTaskStack(ProgramCounter pc = NOCODE, int depth = 1000);
  TaggedRef dbgFrameVariables(int frameId);

  void gc(TaskStack *);

  Bool findCatch(TaggedRef *traceBack=0, Bool verbose=0);

  void pushCFun(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    Assert(MemChunks::areRegsInHeap(x, i));

    pushFrame(C_CFUNC_CONT_Ptr,(void*)f,
              i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL);
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

#define GetFrameNoDecl(tos,pc,y,g)              \
{                                               \
  pc   = tos->getPC();                          \
  y    = tos->getY();                           \
  g    = tos->getG();                           \
                                                \
  tos  = tos->getNext();                        \
}

#define PopFrameNoDecl(ts,pc,y,g)               \
{                                               \
  TaskStack *__ts = ts;                         \
  Frame *__top = __ts->getTop();                \
  __ts->setTop(__top->dispose());               \
  pc   = __top->getPC();                        \
  y    = __top->getY();                         \
  g    = __top->getG();                         \
}

#endif

#ifndef NEW_STACK
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

  void printTaskStack(ProgramCounter pc = NOCODE,
                      Bool verbose = NO, int depth = 1000);
  TaggedRef dbgGetTaskStack(ProgramCounter pc = NOCODE, int depth = 1000);
  TaggedRef dbgFrameVariables(int frameId);

  Bool isEmpty() { return ::isEmpty(tos); }

  void checkMax(int n);

  Frame *getTop()            { return tos; }
  void setTop(Frame *newTos) { tos = newTos; }

  void gc(TaskStack *newstack);

  Bool findCatch(TaggedRef *traceBack=0, Bool verbose=0);

  void pushCFun(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    Assert(MemChunks::areRegsInHeap(x, i));

    pushFrame(C_CFUNC_CONT_Ptr,(void*)f,
              i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL);
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
  void pushAbstr(PrTabEntry  *a) { pushFrame(C_SET_ABSTR_Ptr,a,NULL); }
  void pushActor(Actor *aa)      { pushFrame(C_ACTOR_Ptr,aa,0); }

  int tasks();
};

#define GetFrameNoDecl(top,pc,y,g)              \
{                                               \
  pc   = (ProgramCounter) *(top-1);             \
  y    = (RefsArray) *(top-2);                  \
  g    = (RefsArray) *(top-3);                  \
  top -= frameSz;                               \
}

#define PopFrameNoDecl(ts,pc,y,g)               \
{                                               \
  TaskStack *__ts = ts;                         \
  Frame *__top = __ts->getTop();                \
  GetFrameNoDecl(__top,pc,y,g)                  \
  __ts->setTop(__top);                          \
}

#endif

#define PopFrame(ts,pc,y,g)                     \
    ProgramCounter pc;                          \
    RefsArray y,g;                              \
    PopFrameNoDecl(ts,pc,y,g)

#define GetFrame(top,pc,y,g)                    \
    ProgramCounter pc;                          \
    RefsArray y,g;                              \
    GetFrameNoDecl(top,pc,y,g)

#endif
