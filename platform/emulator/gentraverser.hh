/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __GENTRAVERSER_H
#define __GENTRAVERSER_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"

#include "stack.hh"
#include "hashtbl.hh"
#include "tagged.hh"
#include "value.hh"
#include "am.hh"
#include "dictionary.hh"
#include "gname.hh"

#define CrazyDebug(Code) 
// let's count nodes processed...
// #define CrazyDebug(Code) Code

//
// The class NodeProcessor allows to build a stack-based traverser on top
// of it. An object of this class keeps track of nodes in the (may be
// rational) tree that are still to be processed. Traversing begins with
// 'start(root, proc, opaque)' where 'root' is a root node, 'proc' is a
// procedure that processes nodes in the tree, and 'opaque' is an opaque
// for the node processor chunk of data that is needed for processing
// nodes in the tree. 'proc' performs whatever it wants on tree nodes;
// for non-leaf nodes (e.g. records) it can apply
// 'nodeProcessor->add(node)' for subtrees in whatever order. This causes
// the node processor to visit subtrees: it will call 'proc(node)' for
// nodes in the reverse order that you added them.
//
// Traversing can be also suspended and resumed, and the queue of nodes
// being processed can be emptied.
//
// Note that the GC could be built using this class (though, slightly
// extended for perfromance reasons) as well!

//	   
class Opaque {};
class NodeProcessor;
typedef void (*ProcessNodeProc)(OZ_Term, Opaque*, NodeProcessor*);

//
// 'NodeProcessorStack' keeps OZ_Term"s to be traversed.
// Basically only allocation routines are used from 'Stack';
class NodeProcessorStack : protected Stack {
public:
  NodeProcessorStack() : Stack(1024, Stack_WithMalloc) {}
  ~NodeProcessorStack() {}

  //
  void ensureFree(int n) {
    if (stackEnd <= tos+n)
      resize(n);
  }

  //
  // we don't use 'push' from Stack because we care about
  // space ourselves;
  void put(OZ_Term term) {
    checkConsistency();
    *tos++ = ToPointer(term);
  }
  OZ_Term get() {
    checkConsistency();
    Assert(isEmpty() == NO);
    return (ToInt32(*(--tos)));
  }
  OZ_Term lookup() {
    checkConsistency();
    return (ToInt32(topElem()));
  }
};

//
class NodeProcessor : public NodeProcessorStack {
protected:
  Bool keepRunning;
  ProcessNodeProc proc;
  Opaque* opaque;

  //
protected:
  // actual processor;
  void doit() {
    while (!isEmpty()) {
      OZ_Term term = get();
      (*proc)(term, opaque, this);
    }
  }

  //
public:
  NodeProcessor() {
    DebugCode(opaque = (Opaque *) -1);
  }
  ~NodeProcessor() {}

  //
  // Define the first node & start the action;
  void start(OZ_Term t, ProcessNodeProc p, Opaque* o) {
    clear();
    put(t);
    proc = p;
    opaque = o;
    //
    keepRunning = OK;
    doit();
  }

  //
  // If 'suspend()' is called (by 'ProcessNodeProc') then 'start(...)'
  // will return; it can be later resumed by using 'resume()';
  void suspend() { keepRunning = NO; }
  void add(OZ_Term t) {
    ensureFree(1);
    put(t);
  } // adds a new entry to the process stack;

  //
  void resume() { doit(); } // see 'suspend()';
  void clear() { mkEmpty(); } // deletes all entries in process queue;

  //
  Opaque* getOpaque() { return (opaque); }
};

//
// 'IndexTable' is basically former 'RefTrail' from 'marshaler.hh'
// (done by Ralf?). Since the later one should go away, i've copied it
// here (this allows for a fair perfomance comparision);
//
// However, it takes now 'OZ_Term's as keys!
class IndexTable : public HashTable {
  int rtcounter;

  //
public:
  IndexTable() : HashTable(HT_INTKEY,2000), rtcounter(0) {}

  //
  int remember(OZ_Term l) {
    Assert(find(l)==-1);
    htAdd((intlong) l, ToPointer(rtcounter++));
    return rtcounter-1;
  }
  int find(OZ_Term l) {
    void *ret = htFind((intlong) l);
    return (ret==htEmpty) ? -1 : (int)ToInt32(ret);
  }

  //
  DebugCode(Bool isEmptyIT() { return rtcounter==0; })
  void unwind() {
    rtcounter -= getSize();
    mkEmpty();
    Assert(isEmptyIT());
  }
};

//
// An object of the 'GenTraverser' class traverses the node graph. The
// user of this class is supposed to create a subclass that specifies
// what to do with each type of node. Traversing starts with
// 'traverse(OZ_Term t, Opaque *o)', where 't' is a root node.  The
// traverser then calls 'processXXX(root)', depending on the type of
// root node XXX.  The processXXX virtual methods are divided into two
// categories, those that return void (these are always leaves) and
// boolean ones which return TRUE to indicate a leaf, and FALSE
// otherwise.  For instance, if 'processRecord()' returns FALSE, then
// the traverser proceeds with traversing the subtrees, i.e. will call
// processXXX for each argument. The traverser works in depth-first
// manner (it is based on 'NodeProcessor').
//
// For handling of cycles and co-references, the method 'remember()' can
// be used inside 'processXXX()': it returns an integer uniquely
// identifying the remembered node. Thereafter if the node is reached
// again the traverser does not call 'processXXX(OZ_Term)' but rather
// 'processRepetition(OZ_Term, int)'. The last method also returns a
// Bool, indicating if traversal should continue or not. For example,
// let's assume that 'remember(f(X Y))' returns '1'. Then later
// 'processRepetition(1)' is called upon reaching a repetition (pointer
// equality). Usually it would return 'TRUE' and f(X Y) would not be
// traversed again. [Possibly you might want to traverse the same thing
// twice, in which case you return FALSE].
// 
// Note that the idea is that the idea is that you can easily create
// subclasses for marshaling, export control, etc.
// 
// suspend/resume are inherited. 
//

//
// An object of the class can be used for more than one traversing;
class GenTraverser : private NodeProcessor, protected IndexTable {
private:
  DebugCode(int debugNODES;)
  CrazyDebug(int debugNODES;)
  void doit();			// actual processor;

  //
public:
  GenTraverser() {}
  virtual ~GenTraverser() {}

  //
  // 'reset()' returns the traverser to the original state;
  void reset() {
    DebugCode(debugNODES = 0;);
    CrazyDebug(debugNODES = 0;);
    suspend();
    clear();
    unwind();
  }

  // For efficiency reasons 'GenTraverser' has its own 'doit' - not
  // the one from 'NodeProcessor'. Because of that, 'resume()' is 
  // overloaded as well (but with the same meaning);
  void traverse(OZ_Term t, Opaque* o) {
    reset();
    //
    ensureFree(1);
    put(t);
    DebugCode(proc = (ProcessNodeProc) -1;); // not used;
    opaque = o;
    //
    keepRunning = OK;
    doit();
    DebugCode(fprintf(stdout, " --- %d nodes.\n", debugNODES););
    DebugCode(fflush(stdout););
    CrazyDebug(fprintf(stdout, " --- %d nodes.\n", debugNODES););
    CrazyDebug(fflush(stdout););
  }

  //
  void resume() { doit(); }

  //	
protected:
  //
  Opaque* getOpaque() { return (opaque); }

  //
  // OZ_Term"s are dereferenced;
  virtual void processSmallInt(OZ_Term siTerm) = 0;
  virtual void processFloat(OZ_Term floatTerm) = 0;
  virtual void processLiteral(OZ_Term litTerm) = 0;
  virtual void processExtension(OZ_Term extensionTerm) = 0;
  // OzConst"s;
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst) = 0;
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst) = 0;
  virtual void processObject(OZ_Term objTerm, ConstTerm *objConst) = 0;
  // 'Tertiary' OzConst"s;
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert) = 0;
  virtual void processCell(OZ_Term cellTerm, Tertiary *cellTert) = 0;
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert) = 0;
  virtual void processResource(OZ_Term resTerm, Tertiary *resTert) = 0;
  // anything else:
  virtual void processNoGood(OZ_Term resTerm, Bool trail) = 0;
  //
  virtual void processUVar(OZ_Term *uvarTerm) = 0;
  virtual void processCVar(OZ_Term *cvarTerm) = 0;

  //
  // These methods return TRUE if the node to be considered a leaf;
  // (Note that we might want to go through a repetition, don't we?)
  virtual Bool processRepetition(OZ_Term term, int repNumber) = 0;
  //
  virtual Bool processLTuple(OZ_Term ltupleTerm) = 0;
  virtual Bool processSRecord(OZ_Term srecordTerm) = 0;
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm) = 0;
  // composite OzConst"s;
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst) = 0;
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst) = 0;
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst) = 0;
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst) = 0;
  // Instructions in a code area (e.g. in an abstraction) are considered
  // to be subtrees. Those, in turn, can be leafs (without OZ_Term"s) 
  // or non-leafs (containing OZ_Term"s). 
  // virtual 

  //
  // There is also an optimization for tail-recursive (well, sort of)
  // processing of lists: if a traverser sees that a cons cell is
  // followed by another cell, its 'car' and 'cdr' (that is a list)
  // are treated together. If the traverser discovers a repetition
  // on either car or cdr side, it will fall back to "normal"
  // processing of subtrees;
  //
  // kost@ : TODO: i'm not yet sure that this is worth the result...
  //         Ralf, do you have an idea about that?
//  virtual Bool processSmallIntList(OZ_Term siTerm, OZ_Term ltuple) = 0;
//  virtual Bool processLiteralList(OZ_Term litTerm, OZ_Term ltuple) = 0;
};


// 
// The class builder builds a node graph. It does the reverse of the
// traverser. Builder assembles a value mostly top-down, except for
// special cases like an arity list of a record is constructed before
// the record itself is. Builder takes primitive values and
// instructions where and how compound values are placed.  The user of
// the class is supposed to parse the input stream on his own, and
// call corresponding 'build' methods. For (all) primitive values the
// 'buildValue()' method is called. There are methods that correspond
// to 'DIF_XXX' of compound structures, which eventually supply a
// value (in general - not immediately, but in this case the s-pointer
// must be preserved until the structure is really built up);
//
// Internally, Builder contains a stack of tasks, which describe what
// to do with values appearing from the stream. These tasks are
// necessary for dealing with compound structures. In general, a task
// consists out of a type tag (which, on certain platforms, can be
// used for threaded code) and two arguments. For the simplest task,
// "spointer", one of the arguments is an 's-pointer' aka in WAM (and
// another is ignored). The three-field task frame format is motivated
// by 'recordArg' task: it places a record subtree given the record
// and its feature name;

//
// OK, the following task types emerged:
enum BuilderTaskType {
  //
  // Mostly, the only thing to be done with a term is to place it at a
  // given location (including the topmost task):
  BT_spointer = 0,		// (keep this value!!!)
  // A variation of this is to do also a next task:
  BT_spointer_iterate,
  // When constructing subtree in a bottom-up fashion, the node is to
  // be remebered (it proceeds iteratively further). Note that this
  // task is never popped on it follows some '_iterate' task:
  BT_buildValue,

  //
  // 'buildTuple' puts 'makeTuple' (and a special frame with the
  // arity, and maybe - for 'makeTupleMemo' - with storing index).
  // 'makeTuple' creates the tuple and puts 'n' 'tuple' tasks, each of
  // whose contains a corresponding s-pointer;
  BT_makeTuple,
  BT_makeTupleMemo,

  //
  // Records are processed like this: first, 'takeRecordLabel' and
  // 'takeRecordArity' subsequently accumulate a label and arity list
  // respectively. 'takeRecordArity' issues 'makeRecord_intermediate'
  // what constructs the record and issues 'recordArg's. The thing is
  // called "intermediate" since it's applied when a first subtree
  // arrives; thus, it does the construction job and immediately
  // proceeds to the actual topmost task (which is 'recordArg'). Note
  // that we cannot handle structures that do not have proper subtree
  // while have special ones necessary for its construction. Better to
  // say, in this case a dummy subtree task ('DIF_EOT') is necessary.
  BT_takeRecordLabel,
  BT_takeRecordLabelMemo,
  BT_takeRecordArity,
  BT_takeRecordArityMemo,
  BT_makeRecord_intermediate,
  BT_makeRecordMemo_intermediate,
  //
  BT_recordArg,
  BT_recordArg_iterate,

  //
  // Dictionaries are yet more complicated. 'DIF_DICT/n' creates an
  // empty dictionary and 'n' 'BT_dictKey' tasks. 'dictKey' refers the
  // dictionary and issues a 'dictVal' task. 'dictVal' refers both the
  // dictionary and the key and puts a value into the dictionary;
  BT_dictKey,
  BT_dictVal,

  //
  BT_fsetvalue,
  BT_fsetvalueMemo,
  // 
  BT_chunk,
  BT_chunkMemo,
  BT_class,
  BT_classMemo,
  //
  BT_proc,
  BT_procMemo,
  BT_closureElem,
  BT_closureElem_iterate,

  //
  BT_NOTASK
};

//
static const int bsFrameSize = 3;
typedef StackEntry BTFrame;
// ... and stack entries are actually 'void*' (see stack.hh);
//
#define ReplaceTask1stArg(frame,type,uintArg)	\
{						\
  *(frame-1) = ToPointer(type);			\
  *(frame-2) = ToPointer(uintArg);		\
}
#define ReplaceTask1Ptr(frame,type,ptr)		\
{						\
  *(frame-1) = ToPointer(type);			\
  *(frame-2) = ptr;				\
}
#define ReplaceTaskPtrArg(frame,type,ptrArg,uintArg)	\
{							\
  *(frame-1) = ToPointer(type);				\
  *(frame-2) = ptrArg;					\
  *(frame-3) = ToPointer(uintArg);			\
}
#define ReplaceTask2ndArg(frame,type,uintArg)	\
{						\
  *(frame-1) = ToPointer(type);			\
  *(frame-3) = ToPointer(uintArg);		\
}

//
// Separated 'GetBTFrame'/'getType'/'getArg'/'discardFrame'/...
// Proceed with care...
#define GetBTFrame(frame)				\
  BTFrame *frame = getTop();

#define GetBTTaskType(frame, type)			\
  BuilderTaskType type = (BuilderTaskType) ToInt32(*(frame-1));
#define GetBTTaskTypeNoDecl(frame, type)		\
  type = (BuilderTaskType) ToInt32(*(frame-1));

#define GetBTTaskArg1(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame-2));
#define GetBTTaskPtr1(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame-2);
#define GetBTTaskArg1NoDecl(frame, ATYPE, arg)		\
  arg = (ATYPE) ToInt32(*(frame-2));
#define GetBTTaskPtr1NoDecl(frame, PTYPE, ptr)		\
  ptr = (PTYPE) *(frame-2);
#define GetBTTaskArg2(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame-3));
#define GetBTTaskPtr2(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame-3);

#define DiscardBTFrame(frame)				\
  frame = frame - bsFrameSize;
#define DiscardBT2Frames(frame)				\
  frame = frame - bsFrameSize - bsFrameSize;

// Special: lookup the type of the next frame...
#define GetNextTaskType(frame, type)			\
  BuilderTaskType type = (BuilderTaskType) ToInt32(*(frame - bsFrameSize - 1));
#define GetNextTaskArg1(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame - bsFrameSize - 2));
#define GetNextTaskPtr1(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame - bsFrameSize - 2);

#define GetBTFrameArg1(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame-1));
#define GetBTFramePtr1(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame-1);
#define GetBTFrameArg2(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame-2));
#define GetBTFramePtr2(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame-2);
#define GetBTFrameArg3(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame-3));
#define GetBTFramePtr3(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame-3);

#define GetNextBTFrameArg1(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame - bsFrameSize - 1));

#define EnsureBTSpace(frame,n)				\
  frame = ensureFree(frame, n * bsFrameSize);
#define EnsureBTSpace1Frame(frame)			\
  frame = ensureFree(frame, bsFrameSize);
#define SetBTFrame(frame)				\
  setTop(frame);

#define PutBTTaskPtr(frame,type,ptr1)			\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ptr1;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;
#define PutBTTask(frame,type)				\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;
#define PutBTTask2Args(frame,type,arg1,arg2)		\
  *(frame) = ToPointer(arg2);				\
  *(frame+1) = arg1;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;
#define PutBTTaskPtrArg(frame,type,ptr,arg)		\
  *(frame) = ToPointer(arg);				\
  *(frame+1) = ptr;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;
#define PutBTEmptyFrame(frame)				\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  DebugCode(*(frame+2) = ToPointer(0xffffffff););	\
  frame = frame + bsFrameSize;
#define PutBTFramePtr(frame,ptr)			\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ptr;					\
  frame = frame + bsFrameSize;
#define PutBTFrameArg(frame,arg)			\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ToPointer(arg);				\
  frame = frame + bsFrameSize;
#define PutBTFrame3Args(frame,arg1,arg2,arg3)		\
  *(frame) = ToPointer(arg3);				\
  *(frame+1) = ToPointer(arg2);				\
  *(frame+2) = ToPointer(arg1);				\
  frame = frame + bsFrameSize;

//
class BuilderStack : protected Stack {
public:
  BuilderStack() : Stack(1024, Stack_WithMalloc) {}
  ~BuilderStack() {}

  //
  StackEntry *getTop()            { return tos; }
  void setTop(StackEntry *newTos) { tos = newTos; }
  void clear() { tos = array; }
  //
  StackEntry *ensureFree(StackEntry *frame, int n)
  {
    if (stackEnd <= frame + n) {
      setTop(frame);
      resize(n);
      frame = tos;
    }
    return (frame);
  }

  //
  void putTask(BuilderTaskType type, void* ptr1, uint32 arg2) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    *(newTop) = ToPointer(arg2);
    *(newTop+1) = ptr1;
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
  void putTask(BuilderTaskType type, uint32 arg1, uint32 arg2) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    *(newTop) = ToPointer(arg2);
    *(newTop+1) = ToPointer(arg1);
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
  void putTask(BuilderTaskType type, void* ptr) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    DebugCode(*(newTop) = ToPointer(0xffffffff););
    *(newTop+1) = ptr;
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
  void putTask(BuilderTaskType type, uint32 arg) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    DebugCode(*(newTop) = ToPointer(0xffffffff););
    *(newTop+1) = ToPointer(arg);
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
  void putTask2ndArg(BuilderTaskType type, uint32 arg) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    *(newTop) = ToPointer(arg);
    DebugCode(*(newTop+1) = ToPointer(0xffffffff););
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
  void putTask(BuilderTaskType type) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    DebugCode(*(newTop) = ToPointer(0xffffffff););
    DebugCode(*(newTop+1) = ToPointer(0xffffffff););
    *(newTop+2) = ToPointer(type);
    setTop(newTop + bsFrameSize);
  }
};

//
// That's also a piece of history (former 'RefTable');
class TermTable {
  OZ_Term *array;
  int size;
public:
  TermTable() {
    size     = 100;
    array    = new OZ_Term[size];
  }

  //
  OZ_Term get(int i) {
    return (i>=size) ? makeTaggedNULL() : array[i];
  }
  void set(OZ_Term val, int pos) {
    Assert(pos >= 0);
    if (pos>=size) 
      resize(pos);
    array[pos] = val;
  }

  void resize(int newsize) {
    int oldsize = size;
    OZ_Term  *oldarray = array;
    while(size <= newsize) {
      size = (size*3)/2;
    }
    array = new OZ_Term[size];
    for (int i=0; i<oldsize; i++) {
      array[i] = oldarray[i];
    }
    delete oldarray;
  }
};


//
class Builder : private BuilderStack, public TermTable {
private:
  DebugCode(int debugNODES;)
  CrazyDebug(int debugNODES;)
  OZ_Term result;		// used as a "container";
  OZ_Term blackhole;		// ... for discarding stuff;

  //
private:
  void buildValueOutline(OZ_Term value, BTFrame *frame,
			 BuilderTaskType type);

  //
public:
  Builder() : result((OZ_Term) 0), blackhole((OZ_Term) 0) {}
  ~Builder() {}

  //
  // begin building:
  void build() {
    DebugCode(debugNODES = 0;);
    CrazyDebug(debugNODES = 0;);
    putTask(BT_spointer, &result);
  }
  // returns '0' if inconsistent:
  OZ_Term finish() {
    if (isEmpty()) {
      DebugCode(fprintf(stdout, " --- %d nodes.\n", debugNODES););
      DebugCode(fflush(stdout););
      CrazyDebug(fprintf(stdout, " --- %d nodes.\n", debugNODES););
      CrazyDebug(fflush(stdout););
      return (result);
    } else {
      clear();			// do it eagerly - presumably seldom;
      return ((OZ_Term) 0);
    }
  }
  void stop() { clear(); }

  //
  // 'buildValue' is the main actor: it pops tasks;
  void buildValue(OZ_Term value) {
    DebugCode(debugNODES++;);
    CrazyDebug(debugNODES++;);
    GetBTFrame(frame);
    GetBTTaskType(frame, type);
    if (type == BT_spointer) {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      DiscardBTFrame(frame);
      SetBTFrame(frame);
      *spointer = value;
    } else {
      buildValueOutline(value, frame, type);
    }
  }
  void buildValueRemeber(OZ_Term value, int n) {
    buildValue(value);
    set(value, n);
  }

  //
  void buildRepetition(int n) {
    OZ_Term value = get(n);
    buildValue(value);
  }

  //
  void buildList() {
    LTuple *l = new LTuple();
    buildValue(makeTaggedLTuple(l));
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTTaskPtr(frame, BT_spointer, l->getRefTail());
    PutBTTaskPtr(frame, BT_spointer, l->getRefHead());
    SetBTFrame(frame);
  }
  void buildListRemember(int n) {
    LTuple *l = new LTuple();
    OZ_Term list = makeTaggedLTuple(l);
    buildValue(list);
    set(list, n);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTTaskPtr(frame, BT_spointer, l->getRefTail());
    PutBTTaskPtr(frame, BT_spointer, l->getRefHead());
    SetBTFrame(frame);
  }

  //
  void buildTuple(int arity) {
    putTask(BT_makeTuple, arity);
  }
  void buildTupleRemember(int arity, int n) {
    putTask(BT_makeTupleMemo, arity, n);
  }

  //
  // Tasks's frame will be extended for label and arity;
  void buildRecord() {
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTEmptyFrame(frame);
    PutBTTask(frame, BT_takeRecordLabel);
    SetBTFrame(frame);
  }
  void buildRecordRemember(int n) {
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTFrameArg(frame, n);
    PutBTTask(frame, BT_takeRecordLabelMemo);
    SetBTFrame(frame);
  }

  //
  void buildDictionary(int size) {
    OzDictionary *aux = new OzDictionary(am.currentBoard(), size);
    aux->markSafe();
    //
    buildValue(makeTaggedConst(aux));
    //
    GetBTFrame(frame);
    EnsureBTSpace(frame, size);
    while(size-- > 0) {
      PutBTTaskPtr(frame, BT_dictKey, aux);
    }
    SetBTFrame(frame);
  }

  //
  void buildDictionaryRemember(int size, int n) {
    OzDictionary *aux = new OzDictionary(am.currentBoard(),size);
    aux->markSafe();
    //
    OZ_Term dict = makeTaggedConst(aux);
    buildValue(dict);
    set(dict, n);
    //
    GetBTFrame(frame);
    EnsureBTSpace(frame, size);
    while(size-- > 0) {
      PutBTTaskPtr(frame, BT_dictKey, aux);
    }
    SetBTFrame(frame);
  }

  //
  void buildFSETValue() {
    putTask(BT_fsetvalue);
  }
  void buildFSETValueRemember(int n) {
    putTask(BT_fsetvalueMemo, n);
  }

  //
  // New chunks/classes/procedures are built with 'buildXXX()' while
  // those that are already imported are placed with 'knownXXX()'.
  // Note that one cann't do 'buildValue()' instead of 'knownXXX()'
  // because the later one discards also the unused terms!
  void buildChunk(GName *gname) {
    Assert(gname);
    putTask(BT_chunk, gname);
  }
  void buildChunkRemember(GName *gname, int n) {
    Assert(gname);
    putTask(BT_chunkMemo, gname, n);
  }
  void knownChunk(OZ_Term chunkTerm) {
    buildValue(chunkTerm);
    putTask(BT_spointer, &blackhole);
  }

  //
  void buildClass(GName *gname, int flags) {
    Assert(gname);
    putTask(BT_class, gname, flags);
  }
  void buildClassRemember(GName *gname, int flags, int n) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 1);
    PutBTFrameArg(frame,n)
    putTask(BT_classMemo, gname, flags);
  }
  void knownClass(OZ_Term classTerm) {
    buildValue(classTerm);
    putTask(BT_spointer, &blackhole);
  }

  //
  // Procedures are "more" interesting...
  void buildProc(GName *gname,
		 int arity, int gsize, int maxX, ProgramCounter pc) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 3);
    PutBTFramePtr(frame, pc);
    PutBTFrame3Args(frame, arity, gsize, maxX);
    PutBTTaskPtr(frame, BT_proc, gname);
    SetBTFrame(frame);
  }
  void buildProcRemember(GName *gname,
			 int arity, int gsize, int maxX, ProgramCounter pc,
			 int n) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 3);
    PutBTFramePtr(frame, pc);
    PutBTFrame3Args(frame, arity, gsize, maxX);
    PutBTTaskPtr(frame, BT_procMemo, gname);
    SetBTFrame(frame);
  }
  void knownProc(OZ_Term procTerm) {
    buildValue(procTerm);

    Abstraction *pp = (Abstraction *) tagged2Const(procTerm);
    Assert(isAbstraction(pp));
    int gsize = pp->getPred()->getGSize();
    GetBTFrame(frame);
    EnsureBTSpace(frame, gsize+1); // 'name' as well;
    for (int i = 0; i < gsize + 1; i++) {
      PutBTTaskPtr(frame, BT_spointer, &blackhole);
    }
    SetBTFrame(frame);
  }
};

#endif

