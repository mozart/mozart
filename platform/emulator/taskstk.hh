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


/*
 * - if you ever change this then check POPTASK,
 *  i.e. in other words: keep your hands off this definition
 *  if you don't fully understand the macro POPTASK
 * - there are 4 tag bits
 */
enum ContFlag {
  C_CONT           = 0, // a continuation without X registers
  C_XCONT          = 1, // a continuation with    X registers
  C_CFUNC_CONT     = 2, // a continuation  to call a c-function
  C_DEBUG_CONT     = 3, // a continuation for debugging
  C_CALL_CONT      = 4, // an application
  C_SETFINAL       = 5, //
  C_ACTOR          = 6, // an actor task
  C_XXXX2          = 7, //
  C_XXXX3          = 9, //
  C_SET_OOREGS     = 9, // set am.ooRegisters
  C_LTQ            = 10,// local thread queue
  C_CATCH          = 11 // exception handler
};



/* for C_CONT and C_XCONT we combine
 * the ContFlag with the PC
 */

typedef TaggedRef TaggedPC;

inline
TaggedPC makeTaggedPC(ContFlag f, ProgramCounter pc)
{
  return makeTaggedRef((TypeOfTerm)f,pc);
}

inline
ProgramCounter getPC(ContFlag f,TaggedPC tpc)
{
  return (ProgramCounter) tagValueOf((TypeOfTerm)f,tpc);
}

inline
ContFlag getContFlag(TaggedPC tpc)
{
  return (ContFlag) tagTypeOf(tpc);
}

typedef StackEntry TaskStackEntry;


/* The bottom of the TaskStack contains a special element: emptyTaskStackEntry
 * this allows faster check for emptyness
 */


/* if you ever change this check POPTASK */
const TaskStackEntry emptyTaskStackEntry = (TaskStackEntry)(unsigned int32)-1;

class TaskStack: public Stack {
public:
  USEFREELISTMEMORY;

  void makeEmpty()
  {
    mkEmpty();
    push(emptyTaskStackEntry,NO);
  }

  TaskStack(int s): Stack(s,Stack_WithFreelist) { makeEmpty(); }
  ~TaskStack()                  { error("~TaskStack called"); }

  void printTaskStack(ProgramCounter pc = NOCODE,
                      Bool verbose = NO, int depth = 10000);
  TaggedRef dbgGetTaskStack(ProgramCounter pc = NOCODE,
                       int depth = 10000);

  Bool isEmpty(TaskStackEntry t) { return (t == emptyTaskStackEntry); }
  Bool isEmpty()                 { return isEmpty(*(tos-1)); }

  void checkMax();
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

  void shift(int len)
  {
    ensureFree(len);

    for(TaskStackEntry *help = tos; help >= array; help--) {
      *(help+len) = *help;
    }

    tos += len;
  }

# define TaskStackPop(tos) (*(tos))

  TaskStackEntry *getTop()            { return tos; }
  void setTop(TaskStackEntry *newTos) { tos = newTos; }

  void gc(TaskStack *newstack);

  TaggedRef findCatch(TaggedRef &traceback);

  void pushSetFinal()
  {
    push(ToPointer(C_SETFINAL));
  }

  void pushCatch(TaggedRef hdl)
  {
    ensureFree(2);
    push(ToPointer(hdl),NO);
    push(ToPointer(C_CATCH),NO);
  }

  void pushCall(TaggedRef pred, RefsArray  x, int i)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(3);

    push(i>0 ? copyRefsArray(x, i) : NULL, NO);
    push(ToPointer(pred), NO);
    push(ToPointer(C_CALL_CONT), NO);
  }

  void pushCFunCont(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(3);

    Assert(MemChunks::areRegsInHeap(x, i));
    push(i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL, NO);
    push((TaskStackEntry)f,NO);
    push(ToPointer(C_CFUNC_CONT), NO);
  }

  void pushCont(ProgramCounter pc,RefsArray y,RefsArray g,RefsArray x,int i)
  {
    Assert(!isFreedRefsArray(y));
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    /* cache top of stack in register since gcc does not do it */
    TaskStackEntry *newTop = ensureFree(4);

    Assert(MemChunks::areRegsInHeap(x,i));
    Assert(!y || MemChunks::areRegsInHeap(y,getRefsArraySize(y)));
    Assert(!g || MemChunks::areRegsInHeap(g,getRefsArraySize(g)));

    if (i > 0) { *newTop++ = x; }
    *newTop++   = g;
    *newTop++   = y;
    *newTop++   = ToPointer(makeTaggedPC(i>0 ? C_XCONT : C_CONT, pc));

    tos = newTop;
  }

  void pushCont(Continuation *cont) {
    pushCont(cont->getPC(),cont->getY(),cont->getG(),
             cont->getX(),cont->getXSize());
  }

  void pushPair(void *p, ContFlag cf)
  {
    TaskStackEntry *newTop = ensureFree(2);
    *newTop = p;
    *(newTop+1) = ToPointer(cf);
    tos = newTop+2;
  }

  void pushLTQ(SolveActor * sa)
  {
    TaskStackEntry *newTop = ensureFree(2);
    *newTop++ = ToPointer((int) sa);
    *newTop++ = ToPointer(C_LTQ);
    tos = newTop;
  }

  void pushDebug(OzDebug *deb)   { pushPair(deb,C_DEBUG_CONT); }
  void pushOORegs(int32 regs)    { pushPair(ToPointer(regs),C_SET_OOREGS); }
  void pushActor(Actor *aa)      { pushPair(aa,C_ACTOR); }

  static int frameSize(ContFlag);
  int tasks();
};



#endif
