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

#ifdef __GNUC__
#pragma interface
#endif


/*
 * - if you ever change this check POPTASK, ie. in other words:
 *   keep your hands off this definition if you don't
 *   fully understand the macro POPTASK
 * - there are 4 tag bits  */
enum ContFlag {
  C_CONT       = 0,  // a continuation without X registers
  C_XCONT      = 1,  // a continuation with    X registers
  C_NERVOUS    = 2,  // check for stability & distribution after solved
  C_CFUNC_CONT = 3,  // a continuation  to call a c-function
  C_DEBUG_CONT = 4,  // a continuation for debugging
  C_CALL_CONT  = 5,  // an application
  C_COMP_MODE  = 6,  // switch to seq/par mode
  C_SOLVE      = 7,  // the SOLVE combinator
  C_LOCAL      = 8,  // a local computation space
  C_EXCEPT_HANDLER = 9 
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

  TaskStack(int s): Stack(s,freeListMalloc) { push(emptyTaskStackEntry); }
  ~TaskStack()               { error("~TaskStack called"); }

  virtual void deallocate(StackEntry *p, int n);
  virtual StackEntry *reallocate(StackEntry *p, int oldsize, int newsize);
  void dispose () { deallocate(array,getMaxSize()); }
  virtual void resize(int newSize);

  void printDebug(ProgramCounter pc, Bool verbose, int depth = 10000);
  
  Bool isEmpty(TaskStackEntry t) { return (t == emptyTaskStackEntry); }
  Bool isEmpty()                 { return isEmpty(*(tos-1)); }

  void makeEmpty()
  {
    tos = array;
    push(emptyTaskStackEntry);
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
  void gcRecurse();

  // for debugging
  TaggedRef TaskStack::DBGmakeList();

  void pushCall(Chunk *pred, RefsArray  x, int i)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(3);

    push(i>0 ? copyRefsArray(x, i) : NULL, NO);
    push(pred, NO);
    push(ToPointer(C_CALL_CONT), NO);
  }

  void pushExceptionHandler(Chunk *pred)
  {
    ensureFree(2);
    push(pred, NO);
    push(ToPointer(C_EXCEPT_HANDLER), NO);
  }

  void pushNervous() { push(ToPointer(C_NERVOUS)); }
  void pushSolve()   { push(ToPointer(C_SOLVE)); }
  void pushLocal()   { push(ToPointer(C_LOCAL)); }

  void pushCFunCont(OZ_CFun f, Suspension* s,
		    RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(4);

    Assert(MemChunks::areRegsInHeap(x, i));
    push(i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL, NO);
    push(s, NO);
    push((TaskStackEntry)f,NO);
    push(ToPointer(C_CFUNC_CONT), NO);
  }
  
  
  void pushCont(ProgramCounter pc,
		RefsArray y,RefsArray g,RefsArray x,int i, Bool copy)
  {
    Assert(!isFreedRefsArray(y));
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    /* cache top of stack in register since gcc does not do it */
    TaskStackEntry *newTop = ensureFree(3);

    Assert(MemChunks::areRegsInHeap(x,i));
    Assert(!y || MemChunks::areRegsInHeap(y,getRefsArraySize(y)));
    Assert(!g || MemChunks::areRegsInHeap(g,getRefsArraySize(g)));
	       
    if (i > 0) { *newTop++ = copy ? copyRefsArray(x,i) : x; }
    *newTop     = g;
    *(newTop+1) = y; 
    *(newTop+2) = ToPointer(makeTaggedPC(i>0 ? C_XCONT : C_CONT, pc));
    
    tos = newTop + 3;
  }

  void pushDebug(OzDebug *deb)
  {
    push(deb);
    push(ToPointer(C_DEBUG_CONT));
  }

  static TaskStackEntry makeCompMode(int mode) {
    return ToPointer((mode<<4) | C_COMP_MODE);
  }
  static int getCompMode(TaskStackEntry e) {
    return (ToInt32(e)>>4);
  }
  void pushCompMode(int mode)
  {
    push(makeCompMode(mode));
  }

  int getSeqSize();
  void copySeq(TaskStack *newStack,int size);
  Bool discardLocalTasks();
  Chunk *findExceptionHandler();

private:

  void gcInit()                     { tos = stackEnd-1; }

  void gcQueue(TaskStackEntry elem) { *(tos--) = elem; }

  void gcEnd()
  {
    TaskStackEntry *saveTop = tos+1;
    makeEmpty();
    while(saveTop < stackEnd) {
      push(*saveTop,NO);
      saveTop++;
    }
  }
};

int frameSize(ContFlag);


#endif
