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
  C_JOB            = 5, // job marker
  C_LOCAL          = 6, // a local computation space
  C_EXCEPT_HANDLER = 7, // 
  C_SET_CAA        = 8, // supply the emulator with the CAA pointer;
  C_SET_SELF       = 9  // set am.cachedSelf
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

  TaskStack(int s): Stack(s,freeListMalloc) { makeEmpty(); }
  ~TaskStack()               { error("~TaskStack called"); }

  virtual void deallocate(StackEntry *p, int n);
  virtual StackEntry *reallocate(StackEntry *p, int oldsize, int newsize);
  void dispose () { deallocate(array,getMaxSize()); }
  virtual void resize(int newSize);

  void printTaskStack(ProgramCounter pc = NOCODE,
		      Bool verbose = NO, int depth = 10000);
  
  Bool isEmpty(TaskStackEntry t) { return (t == emptyTaskStackEntry); }
  Bool isEmpty()                 { return isEmpty(*(tos-1)); }

  void makeEmpty()
  {
    mkEmpty();
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

  void pushCall(TaggedRef pred, RefsArray  x, int i)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(3);

    push(i>0 ? copyRefsArray(x, i) : NULL, NO);
    push(ToPointer(pred), NO);
    push(ToPointer(C_CALL_CONT), NO);
  }

  void pushLocal()   { push(ToPointer(C_LOCAL)); }

  void pushCFunCont(OZ_CFun f, RefsArray  x, int i, Bool copy)
  {
    DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));

    ensureFree(3);

    Assert(MemChunks::areRegsInHeap(x, i));
    push(i>0 ? (copy ? copyRefsArray(x, i) : x) : NULL, NO);
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
    *newTop++   = g;
    *newTop++   = y; 
    *newTop++   = ToPointer(makeTaggedPC(i>0 ? C_XCONT : C_CONT, pc));
    
    tos = newTop;
  }

  void pushCont(Continuation *cont) {
    pushCont(cont->getPC(),cont->getY(),cont->getG(),
	     cont->getX(),cont->getXSize(),NO);
  }

  void pushPair(void *p, ContFlag cf)
  {
    TaskStackEntry *newTop = ensureFree(2);
    *newTop = p;
    *(newTop+1) = ToPointer(cf);
    tos = newTop+2;
  }

  void pushDebug(OzDebug *deb)   { pushPair(deb,C_DEBUG_CONT); }
  void pushSetCaa (AskActor *aa) { pushPair(aa,C_SET_CAA); }
  void pushSelf(Object *obj)     { pushPair(obj,C_SET_SELF); }
  void pushExceptionHandler(TaggedRef pred) { 
    pushPair(ToPointer(pred),C_EXCEPT_HANDLER);
  }


  static TaskStackEntry makeJobEntry(Bool hasJob)
  {
    return (TaskStackEntry) ToPointer(makeTaggedRef((TypeOfTerm)C_JOB,hasJob));
  }

  static Bool getJobFlagFromEntry(TaskStackEntry e)
  {
    return (Bool) tagValueOf((TypeOfTerm)C_JOB,(TaggedRef) ToInt32(e));
  }
  void pushJob(Bool hasJobs)
  {
    push(makeJobEntry(hasJobs));
  }

  int getSeqSize();
  DebugCode (int hasJobDebug ();)
  void copySeq(TaskStack *newStack,int size);
  static int frameSize(ContFlag);

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



#endif
