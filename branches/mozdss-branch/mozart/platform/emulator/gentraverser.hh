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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include <setjmp.h>
#include "base.hh"

#include "stack.hh"
#include "indexing.hh"
#include "tagged.hh"
#include "value.hh"
#include "gname.hh"
#include "codearea.hh"
#include "am.hh"
#include "dictionary.hh"

//
// 3#2 compatibility (alters assertions);
// #define MFORMAT_3h2

//
#define GT_STACKSIZE	4096

#ifdef DEBUG_CHECK
// let's count nodes processed...
#define CrazyDebug(Code) Code
// #define CrazyDebug(Code)
#else
#define CrazyDebug(Code)
#endif

//
#ifdef DEBUG_CHECK
void marshalingErrorHook();
#endif

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
// extended for performance reasons) as well!

//
class Opaque {};
class NodeProcessor;
typedef void (*ProcessNodeProc)(OZ_Term, Opaque*, NodeProcessor*);

//
#ifdef DEBUG_CHECK
//
// The ring buffer keeps all extracted elements;
#define NODEPROCESSOR_RINGBUFFER_ENTRIES	64
// we save address in stack and its content;
#define NODEPROCESSOR_RINGBUFFER_BUFSIZE	64*2
//
class NodeProcessorRingBuffer {
private:
  int cnt;
  int current;
  StackEntry buf[NODEPROCESSOR_RINGBUFFER_BUFSIZE];

  //
public:
  void init() {
    cnt = current = 0;
    for (int i = 0; i < NODEPROCESSOR_RINGBUFFER_BUFSIZE; i++)
      buf[i] = 0;
  }
  NodeProcessorRingBuffer() {
    Assert(NODEPROCESSOR_RINGBUFFER_BUFSIZE % 2 == 0);
    init();
  }

  //
  void save(StackEntry *se, StackEntry e) {
    cnt++;
    current %= NODEPROCESSOR_RINGBUFFER_BUFSIZE;
    buf[current++] = se;
    buf[current++] = e;
  }

  //
  void print(int n) {
    int index = current;	// to be allocated;
    n = max(n, 0);
    n = min(n, cnt);
    n = min(n, NODEPROCESSOR_RINGBUFFER_ENTRIES);
    fprintf(stdout, "Note processor's ring buffer.\n");
    fprintf(stdout, "%d tasks ever recorded, FIFO:\n", cnt);

    //
    while (n--) {
      index -= 2;
      if (index < 0) 
	index = NODEPROCESSOR_RINGBUFFER_BUFSIZE - 2;
      //
      fprintf(stdout, " frame(%p) e=%p\n", buf[index], buf[index+1]);
    }

    //
    fflush(stdout);
  }
};
#endif

//
// 'NodeProcessorStack' keeps OZ_Term"s to be traversed.
// Basically only allocation routines are used from 'Stack';
class NodeProcessorStack : protected Stack {
private:
  DebugCode(NodeProcessorRingBuffer rbuf;);

protected:
  // for 'gCollect()';
  StackEntry *getTop()            { Assert(tos >= array); return (tos); }
  StackEntry *getBottom()         { return (array); }
  void setTop(StackEntry *newTos) { 
    tos = newTos;
    checkConsistency();
  }

public:
  NodeProcessorStack() : Stack(GT_STACKSIZE, Stack_WithMalloc) {}
  DebugCode(virtual) ~NodeProcessorStack() {}
  //
  void mkEmpty(void) {
    DebugCode(rbuf.init());
    tos = array;
  }

  //
  void ensureFree(int n) {
    if (stackEnd <= tos+n)
      resize(n);
    Assert(stackEnd > tos+n);
  }
  StackEntry* extendBy(int n) {
    StackEntry *newTop = tos + n;
    if (stackEnd <= newTop) {
      resize(n);
      newTop = tos + n;
    }
    Assert(stackEnd > newTop);
    return (newTop);
  }
    

  //
#if defined(DEBUG_CHECK)
  virtual void appTCheck(OZ_Term term) {}
#endif

  //
  // we don't use 'push' from Stack because we care about
  // space ourselves;
  void put(OZ_Term term) {
    Assert(!oz_isMark(term) && !oz_isVar(term));
    DebugCode(appTCheck(term));
    checkConsistency();
    *tos++ = ToPointer(term);
  }
  OZ_Term get() {
    checkConsistency();
    Assert(isEmpty() == NO);
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (ToInt32(*(--tos)));
  }
  OZ_Term lookup() {
    checkConsistency();
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (ToInt32(topElem()));
  }

  //
  // For special 'GenTraverser' machinery: handling code areas, etc.
  void putInt(int32 i) {
    checkConsistency();
    *tos++ = ToPointer(i);
  }
  int32 getInt() {
    checkConsistency();
    Assert(isEmpty() == NO);
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (ToInt32(*(--tos)));
  }
  int32 lookupInt() {
    checkConsistency();
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (ToInt32(topElem()));
  }
  void putPtr(void* ptr) {
    checkConsistency();
    *tos++ = ptr;
  }
  void* getPtr() {
    checkConsistency();
    Assert(isEmpty() == NO);
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (*(--tos));
  }
  void* lookupPtr() {
    checkConsistency();
    DebugCode(rbuf.save(tos-1, *(tos-1)));
    return (*(tos-1));
  }

  //
  // And yet we update stack entries:
  StackEntry* putPtrSERef(void *ptr) {
    checkConsistency();
    *tos = ptr;
    return (tos++);
  }
  void updateSEPtr(StackEntry* se, void *ptr) { *se = ptr; }
  //
  void dropEntry() { tos--; }
  void dropEntries(int n) { tos -= n; }
};

//
class NodeProcessor : public NodeProcessorStack {
protected:
  StackEntry* tosNotRunning;	// keeps real 'tos' when suspending;
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
    tosNotRunning = (StackEntry *) 0;
    DebugCode(opaque = (Opaque *) -1);
  }
  ~NodeProcessor() {}

  // adds a new entry to the process stack;
  void add(OZ_Term t) {
    ensureFree(1);
    put(t);
  }

  //
  // If 'suspend()' is called (by 'ProcessNodeProc') then 'start(...)'
  // will return; it can be later resumed by using 'resume()';
  // The term 't' is pushed back into the stack - such that we'll
  // start with it next time;
  void suspend(OZ_Term t) {
    Assert(tosNotRunning == (StackEntry *) 0);
    // FIRST, save the term we'are suspending on:
    put(t);
    // Now, dupe the 'doit()' into believing we're done:
    tosNotRunning = getTop();
    Assert(tosNotRunning != (StackEntry *) 0);
    NodeProcessorStack::mkEmpty();
    DebugCode(opaque = (Opaque *) -1);
  }

  //
  Opaque* getOpaque() { return (opaque); }

  //
  // Define the first node & start the action;
  void start(OZ_Term t, ProcessNodeProc p, Opaque* o) {
    Assert(tosNotRunning == (StackEntry *) 0);
    NodeProcessorStack::mkEmpty();
    put(t);
    proc = p;
    opaque = o;

    //
    doit();

    //
    if (tosNotRunning) {
      // keep 'opaque';
      setTop(tosNotRunning);
      tosNotRunning = (StackEntry *) 0;
    }
    Assert(tosNotRunning == (StackEntry *) 0);
  }

  //
  Bool isFinished() { return (isEmpty()); }

  // see 'suspend()';
  void resume() {
    Assert(tosNotRunning == (StackEntry *) 0);
    doit();
    if (tosNotRunning) {
      setTop(tosNotRunning);
      tosNotRunning = (StackEntry *) 0;
    }
    Assert(tosNotRunning == (StackEntry *) 0);
  }
};

//
// An object of the 'GenTraverser' class traverses the node graph. The
// user of this class is supposed to create a subclass that specifies
// what to do with each type of nodes. Traversing starts with
// 'traverse(OZ_Term t, Opaque *o)', where 't' is a root node.  The
// traverser then calls 'processXXX(root)', depending on the type of
// root node XXX.  The processXXX methods are divided into two
// categories, those that return void (these are always leaves) and
// boolean ones which return TRUE to indicate a leaf, and FALSE
// otherwise.  For instance, if 'processRecord()' returns FALSE, then
// the traverser proceeds with traversing the subtrees, i.e. will call
// processXXX for each argument. The traverser works in depth-first
// manner (it is based on 'NodeProcessor').
//
// Co-references and cycles are to be handled by a particular
// marshaler itself (now). The pickler, for instance, relies on
// "patches" produced by the preceeding "resource excavation". This
// way, a hash table dealing with co-references is used only when
// doing resource excavation.
//
// Note that the idea is that you can easily create subclasses for
// marshaling, export control, etc.
// 
// suspend/resume are inherited. 
//

//
#define MAKETRAVERSERTASK(task)  makeTaggedMarkIntNOTEST((int32) task)
//
inline
int32 getTraverserTaskArg(OZ_Term taggedTraverserTask)
{
  Assert(oz_isMark(taggedTraverserTask));
  return tagged2UnmarkedInt(taggedTraverserTask);
}
//
const OZ_Term taggedBATask   = MAKETRAVERSERTASK(0);
const OZ_Term taggedSyncTask = MAKETRAVERSERTASK(1);
const OZ_Term taggedContTask = MAKETRAVERSERTASK(2);

//
// A user can declare a binary area which will be processed with
// 'TraverserBinaryAreaProcessor' supplied (see also the comments for
// GenTraverser::traverseBinary()');
typedef Bool
(*TraverserBinaryAreaProcessor)(GenTraverser *m, GTAbstractEntity *arg);

//
// GT_ExtensionSusp is fixed for extensions (see bytedata.cc).
#define GT_ExtensionSusp	0
//
#define GT_AE_SectionSizeBits   8
#define GT_AE_SectionSize       (1<<GT_AE_SectionSizeBits)
//
#define GT_AE_GenericBase       (1<<GT_AE_SectionSizeBits)
#define GT_AE_PicklerBase       (2<<GT_AE_SectionSizeBits)
#define GT_AE_DPMarshalerBase   (3<<GT_AE_SectionSizeBits)
#define GT_AE_AuxBase           (4<<GT_AE_SectionSizeBits)

//
// For both the traverser and the builder, 'GTAbstractEntity'
// represents arguments for various "processors" that need GC defined;
class GTAbstractEntity {
public:
  virtual int getType() = 0;
  virtual void gc() = 0;
};

//
// A continuation function for a suspension. It is useful e.g. for
// suspending the marshaler in the middle of a term. (See also the
// comments for GenTraverser::suspend(TraverserContProcessor proc,
// GTAbstractEntity *arg)');
typedef void (*TraverserContProcessor)(GenTraverser *m,
				       GTAbstractEntity *arg);

//
// An object of the class can be used for more than one traversing;
class GenTraverser : protected NodeProcessor {
private:
  CrazyDebug(int debugNODES;);
  // 'doit()'s are templated from 'gentraverserLoop.cc' (see also
  // comments before "process" methods;
  void doit() { Assert(0); }

protected:
  // special treatment: no value is available to suspend on!
  void suspendSync() {
    Assert(tosNotRunning == (StackEntry *) 0);
    putInt(taggedSyncTask);
    //
    tosNotRunning = getTop();
    Assert(tosNotRunning != (StackEntry *) 0);
    NodeProcessorStack::mkEmpty();
    DebugCode(opaque = (Opaque *) -1);
  }

  //
  // When the builder receives a value from the stream, it either just
  // stores it somewhere, or, alternatively, passes it to some
  // emulator function. While in the first case it is sufficient just
  // to know the top node of the value, in the second one the whole
  // value must be built up completely. That is, a task that handles
  // such a value must be postponed until the value is completely
  // built up. The marshaler supports the builder by putting marks in
  // the stream saying that a value of interest is complete at this
  // point. These marks and tasks that handle complete values match
  // with other, that is, when the builder fetches a mark from the
  // stream, it executes a task from the stack;
  void putSync() {
    putInt(taggedSyncTask);
  }

  //
  CrazyDebug(void incDebugNODES() { debugNODES++; });
  CrazyDebug(void decDebugNODES() { debugNODES--; });

  //
public:
  //
  // 'reset()' returns the traverser to the original state.
  // This method may be used if marshaling failed;
  void reset() {
    CrazyDebug(debugNODES = 0;); 
    Assert(proc == (ProcessNodeProc) -1); // not used;
    Assert(tosNotRunning == (StackEntry *) 0);
    DebugCode(opaque = (Opaque *) -1);
    NodeProcessorStack::mkEmpty();
  }

  //
  GenTraverser() {
    DebugCode(proc = (ProcessNodeProc) -1;); // not used;
    reset();
  }
  ~GenTraverser() {}

  //
  // Steps through the stack and gCollect's everything there;
  void gCollect();

  //
  // 'suspend(OZ_Term t)' is inherited from NodeProcessor;
  //
  // However, treating binary areas (BA) is different: the binary area
  // is just not finished (so, there is no other value to suspend
  // with), but we still have to suspend:
  void suspendBA() {
    Assert(tosNotRunning == (StackEntry *) 0);
    tosNotRunning = getTop();
    Assert(tosNotRunning != (StackEntry *) 0);
    NodeProcessorStack::mkEmpty();
    DebugCode(opaque = (Opaque *) -1);
  }

  //
  // It is also possible to suspend with an abstract continuation
  // (AC): the traverser proceeds then by application of a
  // user-specified function;
  void suspendAC(TraverserContProcessor proc, GTAbstractEntity *arg) {
    Assert(tosNotRunning == (StackEntry *) 0);
    ensureFree(3);
    putPtr((void *) proc);
    putPtr(arg);
    putInt(taggedContTask);
    //
    tosNotRunning = getTop();
    Assert(tosNotRunning != (StackEntry *) 0);
    NodeProcessorStack::mkEmpty();
    DebugCode(opaque = (Opaque *) -1);
  }

  //
  void resume(Opaque *o) { Assert(0); }

  //
  // The caller does not always know whether the traversing process
  // has been finished or not. So,
  Bool isFinished() {
    Assert(tosNotRunning == (StackEntry *) 0);
    return (isEmpty());
  }

  //
  // Sometimes it is desirable to marshal a number of values with
  // detecting equal nodes within all of them... This is useful when
  // e.g. marshaling a (PERDIO) message, 'cause we know the message is
  // unmarshaled atomically, so the term table keeps containing values
  // across "unmarshalTerm' (whatever it is called)
  //
  // In this case one should start with 'prepareTraversing()' and
  // specify values to be marshaled with 'traverseOne()', and finish
  // with 'finishTraversing()':
  void prepareTraversing(Opaque *o) {
    Assert(proc == (ProcessNodeProc) -1); // not used;
    Assert(opaque == (Opaque *) -1); // otherwise that's recursive;
    Assert(tosNotRunning == (StackEntry *) 0);
    Assert(o != (Opaque *) -1);	     // not allowed (limitation);
    opaque = o;
  }
  // weak 'reset()': the stack is supposed to be empty;
  void finishTraversing() {
    Assert(isEmpty());
    Assert(proc == (ProcessNodeProc) -1); // not used;
    Assert(tosNotRunning == (StackEntry *) 0);
    DebugCode(opaque = (Opaque *) -1);
  }

  //
  Opaque* getOpaque() {
    Assert(opaque != (Opaque *) -1);
    return (opaque);
  }

  //
  // 'process*' methods may refuse to marshal anything, but specify
  // a new term to be marshaled instead:
  void replaceOzValue(OZ_Term term) {
    ensureFree(1);
    put(term);
  }

  //	
protected:
  //
  // 'process' methods are to be defined by every subclass
  // (i.e. marshaler's instance). Note: they are not virtual in
  // current implementation, so every subclass defines also its 'doit'
  // - based on the template from 'gentraverserLoop'. All this is for
  // efficiency reasons (that is, to eliminate virtual methods as
  // such);
  //
  // Note that co-references are discovered not among all nodes, but
  // only among: literals, var"s, ltuples, srecords, and all oz
  // const"s;
  //
  // OZ_Term"s are dereferenced;
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  // OzConst"s;
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *resConst);
  // anything else:
  void processNoGood(OZ_Term resTerm);
  //
  void processVar(OZ_Term v, OZ_Term *vRef);

  //
  // These methods return TRUE if the node to be considered a leaf;
  // (Note that we might want to go through a repetition, don't we?)
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  // composite OzConst"s;
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  //
  // 'processAbstraction' also issues 'traverseBinary';
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);

  //
  // One can think of 'sync' as an of a pseudo term: it simplifies
  // greately the builder's work;
  void processSync();

  //
public:
  // 
  // The 'traverseBinary' method is the only artifact due to the
  // iterative nature of marshaling. Consider marshaling of a code
  // area: it contains Oz values in it. Recursive marshaler just
  // marshals those values "in place", where they occur. Iterative
  // marshaler can either (a) have the traverser knowing where Oz
  // values are, so the job is done similar to Oz records, (b) declare
  // them using 'GenTraverser::traverseOzValue()' (see below). The first
  // approach requires more knowledge from the traverser and in
  // general two scanning phases, yet the traverser's stack can be as
  // large as there are Oz values in the code area. The second
  // approach fixes first two problems but still suffers from the last
  // one. That problem is solved by making binary areas split into
  // pieces; the first one is declared using 'traverseBinary'.
  // Marshaling a binary area is finished when 'proc' returns
  // TRUE. 'proc' must take care of descriptor (e.g. deallocate it);
  void traverseBinary(TraverserBinaryAreaProcessor proc,
		      GTAbstractEntity *binaryDescriptor) {
    Assert(binaryDescriptor);	// '0' is used by the traverser itself;
    ensureFree(3);
    putPtr((void *) proc);
    putPtr(binaryDescriptor);
    putInt(taggedBATask);
  }
  //
  // Hopefully, this abstraction is sufficient for marshaling
  // arbitrary stuff.

  //
  // 'TraverserBinaryAreaProcessor' declare Oz values to be marshaled
  // via 'traverseOzValue()'. Unmarshaler must declare them to be
  // unmarshaled in the same order (but in the stream they appear in
  // reverse order);
  void traverseOzValue(OZ_Term term) {
    ensureFree(2);
    putSync();		// will appear after the list;
    put(term);
  }

  //
  // When a binary area is small (exactly speaking, when we can first
  // marshal the area completely, and after that Oz values contained
  // in it), it's legal to use 'GenTraverser' as 'NodeProcessor', thus
  // to call 'traverseOzValue()' on its own;

  //
  DebugCode(void vHook() {})

protected:
  //
  //
  // There is also a possible optimization for tail-recursive (well,
  // sort of) processing of lists: if a traverser sees that a cons
  // cell is followed by another cell, its 'car' and 'cdr' (that is a
  // list) are treated together. If the traverser discovers a
  // repetition on either car or cdr side, it will fall back to
  // "normal" processing of subtrees;
  //
  // kost@ : TODO: i'm not yet sure that this is worth the result...
  //         Ralf, do you have an idea about that?
  //  Bool processSmallIntList(OZ_Term siTerm, OZ_Term ltuple);
  //  Bool processLiteralList(OZ_Term litTerm, OZ_Term ltuple);
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
// OK, the following task types emerged (keep the 'builderTaskNames'
// array consistent!):
enum BuilderTaskType {
  //
  // Mostly, the only thing to be done with a term is to place it at a
  // given location (including the topmost task):
  BT_spointer = 0,		// (keep this value!!!)
  // A variation of this is to do also a next task:
  BT_spointer_iterate,
  // When constructing subtree in a bottom-up fashion, the node is to
  // be remebered (it proceeds iteratively further). Note that this
  // task is never popped: it follows some '_iterate' task:
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
  // respectively. 'takeRecordArity' issues 'makeRecordSync' what
  // constructs the record and issues 'recordArg's. The thing is
  // called "sync" since it's applied when a sync mark arrives (the
  // gentraverser makes sure that it comes after the arity list).
  // Thus, 'makeRecordSync' does the construction job.
  BT_takeRecordLabel,
  BT_takeRecordLabelMemo,
  BT_takeRecordArity,
  BT_takeRecordArityMemo,

  BT_makeRecordSync,
  BT_makeRecordMemoSync,
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
  BT_fsetvalueSync,
  BT_fsetvalueMemoSync,
  // 
  BT_chunk,
  BT_chunkMemo,
  //
  BT_classFeatures,
  //
  BT_takeObjectLock,
  BT_takeObjectLockMemo,
  BT_takeObjectState,
  BT_takeObjectStateMemo,
  BT_makeObject,
  BT_makeObjectMemo,
  //
  BT_procFile,
  BT_procFileMemo,
  BT_proc,
  BT_procMemo,
  BT_closureElem,
  BT_closureElem_iterate,

  //
  // Abstract entity;
  BT_abstractEntity,

  //
  // Dealing with binary areas (e.g. code areas);
  BT_binary,
  // 'binary_getValue' records the top node of a value, and
  // 'binary_getValueSync' processes it (on a sync that follows it);
  BT_binary_getValue,
  BT_binary_getValueSync,
  // do arbitrary task at the point (see also 'schedGenAction').
  // 'intermediate' is called so because it gets its job done in front
  // of another task: when 'buildValue' is applied, 'intermediate'
  // tasks are done (they cannot modify stack, apply 'buildValue',
  // modify the value 'buildValue' has got, etc.), and then iterate to
  // a "real" task;
  BT_binary_doGenAction_intermediate,

  //
  // Sometimes we have to ignore the sync mark:
  BT_nop,

  //
  BT_NOTASK
};

//
static const int bsFrameSize = 3;
typedef StackEntry BTFrame;
// ... and stack entries are actually 'void*' (see stack.hh);
//

//
#ifdef DEBUG_CHECK
#define BUILDER_RINGBUFFER_FRAME        (bsFrameSize+1)
#define BUILDER_RINGBUFFER_ENTRIES	64
#define BUILDER_RINGBUFFER_BUFSIZE	64*(bsFrameSize+1)
// A ring buffer for the builder: it keeps track of recently processed
// tasks.

//
// 
static char* builderTaskNames[] = {
  "spointer",
  "spointer_iterate",
  "buildValue",
  "makeTuple",
  "makeTupleMemo",
  "takeRecordLabel",
  "takeRecordLabelMemo",
  "takeRecordArity",
  "takeRecordArityMemo",
  "makeRecord(sync)",
  "makeRecordMemo(sync)",
  "recordArg",
  "recordArg_iterate",
  "dictKey",
  "dictVal",
  "fsetvalue",
  "fsetvalueMemo",
  "fsetvalue(sync)",
  "fsetvalueMemo(sync)",
  "chunk",
  "chunkMemo",
  "classFeatures",
  "takeObjectLock",
  "takeobjectLockMemo",
  "takeobjectState",
  "takeobjectStateMemo",
  "makeObject",
  "makeObjectMemo",
  "procFile",
  "procFileMemo",
  "proc",
  "procMemo",
  "closureElem",
  "closureElem_iterate",
  "abstractEntity",
  "binary",
  "binary_getValue",
  "binary_getValue(sync)",
  "binary_doGenAction(intermediate)",
  "(nop)"
};

//
class BuilderRingBuffer {
private:
  int cnt;
  int current;
  StackEntry buf[BUILDER_RINGBUFFER_BUFSIZE];

  //
public:
  void init() {
    cnt = current = 0;
    for (int i = 0; i < BUILDER_RINGBUFFER_BUFSIZE; i++)
      buf[i] = 0;
  }
  BuilderRingBuffer() {
    Assert(BUILDER_RINGBUFFER_BUFSIZE % BUILDER_RINGBUFFER_FRAME == 0);
    init();
  }

  //
  void save(StackEntry *se,
	    BuilderTaskType type, StackEntry e1, StackEntry e2) {
    cnt++;
    current %= BUILDER_RINGBUFFER_BUFSIZE;
    buf[current++] = se;
    buf[current++] = ToPointer(type);
    buf[current++] = e1;
    buf[current++] = e2;
  }

  //
  BuilderTaskType getLastType() {
    return ((BuilderTaskType) int(buf[current-4]));
  }

  //
  void print(int n) {
    int index = current;	// to be allocated;
    n = max(n, 0);
    n = min(n, cnt);
    n = min(n, BUILDER_RINGBUFFER_ENTRIES);
    fprintf(stdout, "Builder's ring buffer (%d tasks ever recorded):\n",
	    cnt);

    //
    while (n--) {
      index -= BUILDER_RINGBUFFER_FRAME;
      if (index < 0) 
	index = BUILDER_RINGBUFFER_BUFSIZE - BUILDER_RINGBUFFER_FRAME;
      //
      int maybetask = (int32) buf[index+1];
      if (maybetask >= 0 && maybetask < BT_NOTASK) {
	char *name = builderTaskNames[maybetask];
	fprintf(stdout, " frame(%p) e0=%p (%s?), e1=%p, e2=%p\n",
		buf[index], buf[index+1], name, buf[index+2], buf[index+3]);
      } else {
	fprintf(stdout, " frame(%p) e0=%p, e1=%p, e2=%p\n",
		buf[index], buf[index+1], buf[index+2], buf[index+3]);
      }
    }

    //
    fflush(stdout);
  }
};
#endif

//
#define ReplaceBTTask(frame,type)			\
{							\
  *(frame-1) = ToPointer(type);				\
}
#define ReplaceBTTask1stArg(frame,type,uintArg)		\
{							\
  *(frame-1) = ToPointer(type);				\
  *(frame-2) = ToPointer(uintArg);			\
}
#define ReplaceBTTask1stPtrOnly(frame,ptr)		\
{							\
  *(frame-2) = ptr;					\
}
#define ReplaceBTTask1Ptr(frame,type,ptr)		\
{							\
  *(frame-1) = ToPointer(type);				\
  *(frame-2) = ptr;					\
}
#define ReplaceBTTaskPtrArg(frame,type,ptrArg,uintArg)	\
{							\
  *(frame-1) = ToPointer(type);				\
  *(frame-2) = ptrArg;					\
  *(frame-3) = ToPointer(uintArg);			\
}
#define ReplaceBTTask2ndArg(frame,type,uintArg)		\
{							\
  *(frame-1) = ToPointer(type);				\
  *(frame-3) = ToPointer(uintArg);			\
}
//
#define ReplaceBTFrame1stArg(frame,arg)			\
{							\
  *(frame-1) = ToPointer(arg);				\
}
#define ReplaceBTFrame2ndPtr(frame,ptr)			\
{							\
  *(frame-2) = ptr;					\
}
#define ReplaceBTFrame2ndArg(frame,arg)			\
{							\
  *(frame-2) = ToPointer(arg);				\
}
#define ReplaceBTFrame3rdArg(frame,arg)			\
{							\
  *(frame-3) = ToPointer(arg);				\
}

//
// Separated 'GetBTFrame'/'getType'/'getArg'/'discardBTFrame'/...
// Proceed with care...
#define GetBTFrame(frame)				\
  BTFrame *frame = getTop();

#define GetBTTaskType(frame, type)			\
  BuilderTaskType type =			       	\
    (BuilderTaskType) ToInt32(*(frame-1));		\
  DebugCode(ringbuf.save(frame, type,			\
            *(frame-2), *(frame-3)););
#define GetBTTaskTypeNoDecl(frame, type)		\
  type = (BuilderTaskType) ToInt32(*(frame-1));		\
  DebugCode(ringbuf.save(frame, type,			\
            *(frame-2), *(frame-3)););

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
#define GetBTTaskArg1Ref(frame, ATYPE)			\
  ((ATYPE &) *(frame-2))
#define GetBTTaskArg2Ref(frame, ATYPE)			\
  ((ATYPE &) *(frame-3))


#define DiscardBTFrame(frame)				\
  frame = frame - bsFrameSize;
#define DiscardBT2Frames(frame)				\
  frame = frame - bsFrameSize*2;

// Special: lookup the type of the next frame...
#define GetBTNextTaskType(frame, type)			\
  BuilderTaskType type = (BuilderTaskType)		\
    ToInt32(*(frame - bsFrameSize - 1));		\
  DebugCode(ringbuf.save(frame-bsFrameSize, type,	\
            *(frame - bsFrameSize - 2),			\
            *(frame - bsFrameSize - 3)););
#define GetBTNextTaskArg1(frame, ATYPE, arg)		\
  ATYPE arg = (ATYPE) ToInt32(*(frame - bsFrameSize - 2));
#define GetBTNextTaskPtr1(frame, PTYPE, ptr)		\
  PTYPE ptr = (PTYPE) *(frame - bsFrameSize - 2);

//
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
#define GetNextBTFramePtr1(frame, PTYPE, arg)		\
  PTYPE arg = (PTYPE) *(frame - bsFrameSize - 1);

#define EnsureBTSpace(frame,n)				\
  frame = ensureFree(frame, n * bsFrameSize);
#define EnsureBTSpace1Frame(frame)			\
  frame = ensureFree(frame, bsFrameSize);
#define SetBTFrame(frame)				\
  setTop(frame);

#define PutBTTask(frame,type)				\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTTaskPtr(frame,type,ptr)			\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ptr;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTTaskArg(frame,type,arg)			\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ToPointer(arg);				\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTTask2Args(frame,type,arg1,arg2)		\
{							\
  DebugCode(*(frame) = ToPointer(arg2););		\
  *(frame+1) = ToPointer(arg1);				\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTTask2Ptrs(frame,type,ptr1,ptr2)		\
{							\
  *(frame) = ptr2;					\
  *(frame+1) = ptr1;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTTaskPtrArg(frame,type,ptr,arg)		\
{							\
  *(frame) = ToPointer(arg);				\
  *(frame+1) = ptr;					\
  *(frame+2) = ToPointer(type);				\
  frame = frame + bsFrameSize;				\
}

//
#define PutBTEmptyFrame(frame)				\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  DebugCode(*(frame+2) = ToPointer(0xffffffff););	\
  frame = frame + bsFrameSize;				\
}
#define PutBTFramePtr(frame,ptr)			\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ptr;					\
  frame = frame + bsFrameSize;				\
}
#define PutBTFrameArg(frame,arg)			\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  DebugCode(*(frame+1) = ToPointer(0xffffffff););	\
  *(frame+2) = ToPointer(arg);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTFrame2Ptrs(frame,ptr1,ptr2)		\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ptr2;					\
  *(frame+2) = ptr1;					\
  frame = frame + bsFrameSize;				\
}
#define PutBTFramePtrArg(frame,ptr,arg)			\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ToPointer(arg);				\
  *(frame+2) = ptr;					\
  frame = frame + bsFrameSize;				\
}
#define PutBTFrame2PtrsArg(frame,ptr1,ptr2,arg)		\
{							\
  *(frame) = ToPointer(arg);				\
  *(frame+1) = ptr2;					\
  *(frame+2) = ptr1;					\
  frame = frame + bsFrameSize;				\
}
#define PutBTFrame2Args(frame,arg1,arg2)		\
{							\
  DebugCode(*(frame) = ToPointer(0xffffffff););		\
  *(frame+1) = ToPointer(arg2);				\
  *(frame+2) = ToPointer(arg1);				\
  frame = frame + bsFrameSize;				\
}
#define PutBTFrame3Args(frame,arg1,arg2,arg3)		\
{							\
  *(frame) = ToPointer(arg3);				\
  *(frame+1) = ToPointer(arg2);				\
  *(frame+2) = ToPointer(arg1);				\
  frame = frame + bsFrameSize;				\
}

#define CopyBTFrame(oframe,nframe)			\
{							\
  *(--nframe) = *(--oframe);				\
  *(--nframe) = *(--oframe);				\
  *(--nframe) = *(--oframe);				\
}

#define NextBTFrame(frame)  frame = frame - bsFrameSize;
#define NextBT2Frames(frame)  frame = frame - bsFrameSize*2;
#define NextBT3Frames(frame)  frame = frame - bsFrameSize*3;
#define NextBT4Frames(frame)  frame = frame - bsFrameSize*4;

//
class BuilderStack : protected Stack {
public:
  BuilderStack() : Stack(GT_STACKSIZE*bsFrameSize, Stack_WithMalloc) {}
  ~BuilderStack() {}

  //
  StackEntry *getTop()            { Assert(tos >= array); return (tos); }
  StackEntry *getBottom()         { return (array); }
  void setTop(StackEntry *newTos) { 
    tos = newTos;
    checkConsistency();
  }
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
  void putTask(BuilderTaskType type, void* ptr1, void* ptr2) {
    StackEntry *newTop = Stack::ensureFree(bsFrameSize);
    *(newTop) = ptr2;
    *(newTop+1) = ptr1;
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

  //
#ifdef DEBUG_CHECK
  void print(int n) {
    StackEntry *se = getTop();
    n = max(n, 0);
    fprintf(stdout, "Builder's stack:\n");

    //
    while (n-- && se > array) {
      StackEntry sse = se;
      StackEntry e0 = *(--se);
      StackEntry e1 = *(--se);
      StackEntry e2 = *(--se);
      int maybetask = (int32) e0;
      if (maybetask >= 0 && maybetask < BT_NOTASK) {
	char *name = builderTaskNames[maybetask];
	fprintf(stdout, " frame(%p) e0=%p (%s?), e1=%p, e2=%p\n",
		sse, e0, name, e1, e2);
      } else {
	fprintf(stdout, " frame(%p) e0=%p, e1=%p, e2=%p\n",
		sse, e0, e1, e2);
      }
    }    

    //
    fflush(stdout);
  }
#endif  
};

//
// Tables that keep re-occurring values: OZ_Term"s and (C heap)
// locations;
//

//
#define BuilderRTInitSize 100

//
// Location tables are filled sequentially, starting from slot 0;
template<class T> 
class BuilderLocTable {
  T *array;
  int size;
  DebugCode(int lastIndex;);

  //
public:
  // invariant: when a slot is not in use, it's -1;
  void mkEmptyTable(void) {
#if defined(DEBUG_CHECK)
    for (int i = lastIndex+1; i--; ) 
      array[i] = (T) -1;
    lastIndex = -1;
#endif
  }
  BuilderLocTable() {
    size  = BuilderRTInitSize;
    array = new T[size];
    //
    DebugCode(lastIndex = size-1;);
    // is supposed to be reset before use;
  }
  ~BuilderLocTable() {
    delete [] array;
    DebugCode(array = (T *) 0;);
    DebugCode(size = -1;);
    DebugCode(lastIndex = -1;);
  }

  //
private:
  void resize(int newIndex) {
    int oldsize = size;
    T *oldarray = array;

    //
    Assert(size < newIndex+1);
    size = newIndex*2;
    array = new T[size];
    for (int i = oldsize; i--; )
      array[i] = oldarray[i];
#if defined(DEBUG_CHECK)
    for (int j = oldsize; j < size; j++)
      array[j] = (T) -1;
#endif
    delete [] oldarray;
  }

  //
public:

  //
  T get(int i) {
    Assert(i >= 0 && i <= lastIndex);
    return (array[i]);
  }

  //
  void set(T val, int pos) {
#if !defined(MFORMAT_3h2)
    // .. in 3#2, both values and locations were kept in the same
    // table, so there could be gaps:
    Assert(pos == lastIndex+1);
#endif
    if (pos >= size) 
      resize(pos);
    Assert(array[pos] == (T) -1);
    Assert(pos < size);
    array[pos] = val;
    DebugCode(lastIndex = pos;);
  }
};

//
// Unfortunately, the traverser assignes indices not in the order of
// traversal (and, thus, the order in which values appear from the
// stream), but in the order nodes are found in the marshaler
// dictionary for the first time. This means that there are unused
// gaps between occupied slots. Occupied slots are now linked, so the
// GC can find them.
//
// Additionally, the slot 0 is reserved by the dictionary;

//
typedef struct {
  // 'n' must occupy the first slot for 'BuilderRefTable::getNextRef()';
  OZ_Term n;			// node;
  int     p;			// previous index;
} BuilderRefTableNode;

//
class BuilderRefTable {
  BuilderRefTableNode *array;
  int size;
  int lastIndex;

  //
public:
  // invariant: when a slot is not in use, it's -1;
  void mkEmptyTable(void) {
#if !defined(MFORMAT_3h2)
    Assert(array[0].n == (OZ_Term) 0);
#endif
#if defined(DEBUG_CHECK)
    int i = lastIndex;
    while (i) {
      int j = array[i].p;
      array[i].n = (OZ_Term) -1;
      array[i].p = -1;
      i = j;
    }
#endif
    lastIndex = 0;		// slot 0 is not used;
  }

  BuilderRefTable() {
    size  = BuilderRTInitSize;
    array = new BuilderRefTableNode[size];
    // 'n' must occupy the first slot for 'BuilderRefTable::getNextRef()':
    Assert(((void *) &(array[0])) == ((void *) &(array[0].n)));
    //
    array[0].n = (OZ_Term) 0;
    array[0].p = -1;
#if defined(DEBUG_CHECK)
    for (int i = 1; i < size; i++) {
      array[i].n = (OZ_Term) -1;
      array[i].p = -1;
    }
    lastIndex = 0;
#endif
    // is supposed to be reset before use;
  }
  ~BuilderRefTable() {
    delete [] array;
    DebugCode(array = (BuilderRefTableNode *) 0;);
    DebugCode(size = -1;);
    DebugCode(lastIndex = -1;);
  }

  //
private:
  void resize(int newIndex) {
    int oldsize = size;
    BuilderRefTableNode *oldarray = array;

    //
    Assert(size < newIndex+1);
    size = newIndex*2;
    array = new BuilderRefTableNode[size];
    for (int i = oldsize; i--; )
      array[i] = oldarray[i];
#if defined(DEBUG_CHECK)
    for (int j = oldsize; j < size; j++) {
      array[j].n = (OZ_Term) -1;
      array[j].p = -1;
    }
#endif
    delete [] oldarray;
  }

  //
public:
  // GC;
  OZ_Term* getFirstRef() {
    return (lastIndex ? &(array[lastIndex].n) : ((OZ_Term *) 0));
  }
  OZ_Term* getNextRef(OZ_Term* in) {
    BuilderRefTableNode *prev = (BuilderRefTableNode *) ((void *) in);
    int index = prev->p;
    return (index ? &(array[index].n) : ((OZ_Term *) 0));
  }

  //
  OZ_Term get(int i) {
    Assert(i >= 0);		// also zeroth slot;
    Assert(array[i].n != (OZ_Term) -1);   // occupied;
    return (array[i].n);
  }

  //
  void set(OZ_Term val, int pos) {
#if !defined(MFORMAT_3h2)
    // .. in 3#2, the position 0 could be used as well;
    Assert(pos > 0);
#endif
    if (pos >= size) 
      resize(pos);
#if !defined(MFORMAT_3h2)
    Assert(array[pos].n == (OZ_Term) -1);
#else
    Assert(pos == 0 || array[pos].n == (OZ_Term) -1);
#endif
    Assert(pos < size);
    array[pos].n = val;
    array[pos].p = lastIndex;
    lastIndex = pos;
  }

  //
  void occupy(int pos){
#if !defined(MFORMAT_3h2)
    Assert(pos > 0);
#endif
    if (pos >= size) 
      resize(pos);
#if !defined(MFORMAT_3h2)
    Assert(array[pos].n == (OZ_Term) -1);
#else
    Assert(pos == 0 || array[pos].n == (OZ_Term) -1);
#endif
    Assert(pos < size);
    array[pos].n = (OZ_Term) 0;
    array[pos].p = lastIndex;
    lastIndex = pos;
  }

  //
#if defined(DEBUG_CHECK)
  void resetFilled(int pos) {
    Assert(array[pos].n != (OZ_Term) 0 && array[pos].n != (OZ_Term) -1);
    array[pos].n = (OZ_Term) 0;
  }
#endif

  //
  void fill(OZ_Term val, int pos) {
    // any *occupied* slot;
#if !defined(MFORMAT_3h2)
    Assert(pos > 0);
#else
    Assert(pos >= 0);
#endif
    Assert(array[pos].n == (OZ_Term) 0);
    array[pos].n = val;
  }
};

//
typedef int BuilderOpaqueBA;
typedef void (*OzValueProcessor)(GTAbstractEntity *arg, OZ_Term value);
typedef void (*BuilderGenAction)(void *arg);

//
class Builder : private BuilderStack {
private:
  CrazyDebug(int debugNODES;);
  OZ_Term result;		// used as a "container";
  OZ_Term blackhole;		// .. for discarding stuff;

  // Reference tables, for OZ_Term"s and locations.
  // In the current implementation, where value construction is still
  // not strictly top-down, builder still has to access the values'
  // ref table. More specifically, values the construction of which is
  // delayed are constructed by the builder as a side-effect of some
  // 'buildValue()', so they have to be recorded in the ref table by
  // the builder too.
  BuilderRefTable *valueRT;
  // Locations' ref table is here just for coding convenience;
  BuilderLocTable<void *> *locationRT;
  DebugCode(BuilderRingBuffer ringbuf;);

private:
  CrazyDebug(void incDebugNODES() { debugNODES++; });
  CrazyDebug(void decDebugNODES() { debugNODES--; });

  //
  void buildValueOutline(OZ_Term value, BTFrame *frame,
			 BuilderTaskType type);

  //
  // Handling binary areas involves pushing a task into the stack (the
  // task represents the binary area to be processed, and is used by
  // 'fillBinary'). However, an Oz entry that declares a binary
  // area(s) is not built yet, thus there is a task for that
  // entry. Obviously, the entry task should stay atop, and the binary
  // area task should go beneath it:
  // 
  // 'BTFrame* liftTask(int sz)' lifts the topmost stack (and if that
  // task iterates, all of its successors) by 'sz' frames, and returns
  // the bottom frame that has been freed:
  BTFrame* liftTask(int sz);
  //
  // ... 'sync' tasks add problems here: in the case when the whole
  // binary-area-containing entity is sync'ed, the sync will appear
  // after the code area, thus, the binary task is buried and must be
  // explicitly searched for:
  BTFrame* findBinary(BTFrame *frame);

  //
  DebugCode(void dbgWrap(););

  //
public:
  Builder()
    : result((OZ_Term) 0), blackhole((OZ_Term) 0) {
    valueRT = new BuilderRefTable;
    locationRT = new BuilderLocTable<void *>;
  }
  ~Builder() {}

  // Index tables:
  OZ_Term getTerm(int i) { return (valueRT->get(i)); }
  void setTerm(OZ_Term t, int i) { valueRT->set(t, i); }
  void occupyTerm(int i) { valueRT->occupy(i); }
  // Filling a previously occupied slot can be either safe - if it is
  // known that the slot is not accessed, and a non-safe case - if it
  // can be referenced by 'buildValueRef()' (see also its comment);
  void fillTermSafe(OZ_Term t, int i) { valueRT->fill(t, i); }
  void fillTerm(OZ_Term t, int i) {
    OZ_Term sc = valueRT->get(i);
    if (sc) {
      Assert(oz_isRef(sc));
      OZ_Term *vp = tagged2Ref(sc);
      DebugCode(OZ_Term v = *vp;);
      Assert(!oz_isRef(v) && oz_isVar(v));
      *vp = t;		// just overwrite it;
      DebugCode(valueRT->resetFilled(i););
    } 
    fillTermSafe(t, i);
  }

  //
  void *getLocation(int i) { return (locationRT->get(i)); }
  void setLocation(void *l, int i) { locationRT->set(l, i); }

  //
  // Called once for the whole batch:
  void prepareBuild() {
    // necessary for the GC: it has to distinguish between garbage and
    // a (partially instantiated) term in this cell;
    result = (OZ_Term) 0;
    valueRT->mkEmptyTable();	// ditto (allows GC);
    locationRT->mkEmptyTable();
    CrazyDebug(debugNODES = 0;);
    DebugCode(ringbuf.init(););
    putTask(BT_spointer, &result);
  }

  //
  // returns '0' if inconsistent:
  OZ_Term finish() {
    if (isEmpty()) {
      // CrazyDebug(fprintf(stdout, " --- %d nodes.\n", debugNODES););
      // CrazyDebug(fflush(stdout););
      OZ_Term r = result;
      DebugCode(result = (OZ_Term) 0xfefefea1);
      return (r);
    } else {
      // may be empty binary task(s)?
      while (!isEmpty()) {
	GetBTFrame(frame);
	GetBTTaskType(frame, type);
	GetBTTaskPtr1(frame, void*, bp);
	//
	if (type == BT_binary && bp == (void *) 0) {
	  DiscardBTFrame(frame);
	  SetBTFrame(frame);
	} else {
	  break;
	}
      }

      //     
      if (isEmpty()) {
	// CrazyDebug(fprintf(stdout, " --- %d nodes.\n", debugNODES););
	// CrazyDebug(fflush(stdout););
	OZ_Term r = result;
	DebugCode(result = (OZ_Term) 0xfefefea1);
	return (r);
      } else {
	mkEmpty();		// do it eagerly - presumably seldom;
	DebugCode(result = (OZ_Term) 0xfefefea1);
	return ((OZ_Term) 0);
      }
    }
  }

  //
  // Bringing the builder into a GC-compliant state;
  void suspend() {}
  //
  // Garbage collection: traverse through the stack and collect all
  // the Oz terms found there. In order to make it work, holes in
  // partially constructed values are closed with dedicated values
  // (see the implementation);
  void gCollect();

  //
  // "build" methods - used by whatever unmarshaler;
  void buildTuple(int arity) {
    putTask(BT_makeTuple, arity);
  }
  void buildTupleRemember(int arity, int n) {
    occupyTerm(n);
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
    occupyTerm(n);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTFrameArg(frame, n);
    PutBTTask(frame, BT_takeRecordLabelMemo);
    SetBTFrame(frame);
  }

  //
  void buildFSETValue() {
    putTask(BT_fsetvalue);
  }
  void buildFSETValueRemember(int n) {
    // FSet"s cannot be recursive. More precisely, construction of an
    // Oz value needed for 'makeFSetValue()' cannot depend on that
    // FSetValue itself (even though 'makeFSetValue()' will not use
    // it, of course).
    occupyTerm(n);
    putTask(BT_fsetvalueMemo, n);
  }

  //
  // New chunks/classes/procedures are built with 'buildXXX()' while
  // those that are already imported are placed with 'knownXXX()'.
  // Note that one can't do 'buildValue()' instead of 'knownXXX()'
  // because the later one discards also the unused terms!
  void buildChunk(GName *gname) {
    Assert(gname);
    putTask(BT_chunk, gname);
  }
  void buildChunkRemember(GName *gname, int n) {
    Assert(gname);
    // ditto, chunks cannot be recursive either (see also
    // 'buildFSETValueRemember()');
    occupyTerm(n);
    putTask(BT_chunkMemo, gname, n);
  }

  //
  void buildClass(GName *gname, int flags) {
    Assert(gname);

    //      
    OzClass *cl = new OzClass(makeTaggedNULL(), 
				      makeTaggedNULL(),
				      makeTaggedNULL(), 
				      makeTaggedNULL(), NO, NO,
				      am.currentBoard());
    cl->setGName(gname);
    gname->gcMaybeOff();	// if not in the table right now;;
    OZ_Term classTerm = makeTaggedConst(cl);
    // Note: no gname"s are assigned globally until the construction
    // of the class is *completely* finished;

    //
    putTask(BT_classFeatures, cl, flags);
  }

  //
  void buildClassRemember(GName *gname, int flags, int n) {
    Assert(gname);

    //      
    OzClass *cl = new OzClass(makeTaggedNULL(), 
				      makeTaggedNULL(),
				      makeTaggedNULL(), 
				      makeTaggedNULL(), NO, NO,
				      am.currentBoard());
    cl->setGName(gname);
    gname->gcMaybeOff();
    OZ_Term classTerm = makeTaggedConst(cl);
    //
    setTerm(classTerm, n);

    //
    putTask(BT_classFeatures, cl, flags);
  }

  //
  void buildObject(GName *gname) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTFramePtr(frame, gname);
    PutBTTask(frame, BT_takeObjectLock);
    SetBTFrame(frame);
  }
  void buildObjectRemember(GName *gname, int n) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTFramePtrArg(frame, gname, n);
    PutBTTask(frame, BT_takeObjectLockMemo);
    SetBTFrame(frame);
    // ditto, objects cannot be recursive either (see also
    // 'buildFSETValueRemember()');
    occupyTerm(n);
  }

  //
  // Procedures are "more" interesting... they are done in two phases,
  // of which the second one deals with the code area and can contain
  // a number of sub-steps. First, the name of the procedure arrives,
  // which will match the 'proc' task. This task creates the procedure
  // but its 'pc' field, and yet pushes two tasks (in this order) - a
  // 'procSecondary' that will update procedure's 'pc', and a 'binary'
  // task, that provides for filling the code area. The later one
  // matches the 'DIF_CODEAREA' from the stream.
  void buildProc(GName *gname, int arity,
		 int gsize, int maxX, int line, int column,
		 ProgramCounter pc) {
    Assert(gname);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 4);
    PutBTFrame2Args(frame, arity, gsize);
    PutBTFrame3Args(frame, maxX, line, column);
    PutBTFrame2Ptrs(frame, gname, pc);
    PutBTTask(frame, BT_procFile);
    SetBTFrame(frame);
  }
  void buildProcRemember(GName *gname, int arity,
			 int gsize, int maxX, int line, int column,
			 ProgramCounter pc, int memoIndex) {
    Assert(gname);
    occupyTerm(memoIndex);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 4);
    PutBTFrame2Args(frame, arity, gsize);
    PutBTFrame3Args(frame, maxX, line, column);
    PutBTFrame2PtrsArg(frame, gname, pc, memoIndex);
    PutBTTask(frame, BT_procFileMemo);
    SetBTFrame(frame);
  }

  //
  // Abstract entity: it is not interpreted by the builder, but handed
  // out to the user by means of 'buildAbstractEntity' (when e.g. a
  // corresponding 'DIF' appears in the stream). This is useful for
  // e.g. unmarshaling terms that span more than one fragment in the
  // stream;
  void getAbstractEntity(GTAbstractEntity *bae) {
    Assert(bae);
    putTask(BT_abstractEntity, bae);
  }
  //
  GTAbstractEntity *buildAbstractEntity() {
    GetBTFrame(frame);
    GetBTTaskPtr1(frame, GTAbstractEntity*, bae);
    DiscardBTFrame(frame);
    SetBTFrame(frame);
    return (bae);
  }

  //
  // An Oz value can contain a binary area that can contain references
  // to (other) Oz values and that can be split into pieces. The
  // unmarshaler can declare such an area using 'buildBinary'.
  // 'binaryAreaDesc' holds the state of unmarshaler relevant for that
  // binary area. The descriptor cannot contain by now any Oz terms
  // (they are handled when a builder is GCed);
  void buildBinary(GTAbstractEntity *binaryAreaDesc) {
    // Zero arguments are not allowed since they are used internally;
    Assert(binaryAreaDesc);
    // Observe that lifting a task can have a consequence: if there
    // is a 'sync' mark in the stream for the whole entity, then 
    // 'fillBinary' will have to search for the 'BT_binary' task;
    BTFrame *hole = liftTask(1);
    PutBTTaskPtr(hole, BT_binary, binaryAreaDesc);
    // no 'SetBTFrame' since the thing is buried;
  }

  //
  // When a fragment of marshaled binary area begins in the stream the
  // unmarshaler is supposed to know its type (e.g. using a dedicated
  // 'DIF' header, aka 'DIF_CODEAREA' for code areas) while the
  // abstract argument for its processing is supplied by the builder
  // using 'fillBinary()':
  GTAbstractEntity* fillBinary(BuilderOpaqueBA &opaque) {
    CrazyDebug(incDebugNODES(););
    GetBTFrame(frame);
    GTAbstractEntity *bp;

    //
  repeat:
    GetBTTaskType(frame, type);
    if (type != BT_binary) {
      // This means that there is either a 'sync' or an
      // '_intermediate' task which covers the binary task. A 'sync'
      // task corresponds to a 'sync' token in the stream, and
      // '_intermediate' tasks are transparent...  Thus,
      Assert(type == BT_makeRecordSync || 
	     type == BT_makeRecordMemoSync ||
	     type == BT_fsetvalueSync ||
	     type == BT_fsetvalueMemoSync ||
	     type == BT_binary_getValueSync ||
	     type == BT_binary_doGenAction_intermediate);
      //
      frame = findBinary(frame);
      GetBTTaskPtr1NoDecl(frame, GTAbstractEntity*, bp);
      Assert(bp);
    } else {
      Assert(type == BT_binary);

      //
      GetBTTaskPtr1NoDecl(frame, GTAbstractEntity*, bp);
      if (!bp) {
	// finished binary area - discard it & try again;
	DiscardBTFrame(frame);
	SetBTFrame(frame);
	goto repeat;
      }
    }

    //
    Assert(bp);
    opaque = (BuilderOpaqueBA) (ToInt32(frame) - ToInt32(getBottom()));
    return (bp);
  }

  //
  // Binary areas can contain references to Oz values that will appear
  // later in the stream. The builder can be instructed to process
  // them using 'getOzValue()' method.
  //
  // Note that 'proc' will be applied when a value from the stream is
  // built up completely.
  //
  // 'proc' must take of 'arg' by itself. For the builder it is really
  // an opaque value. 'arg' cannot refer by now any heap-based
  // objects, aka Oz values (the builder does not handle them during
  // GC));
  //
  void getOzValue(OzValueProcessor proc, GTAbstractEntity *arg) {
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTFrame2Ptrs(frame, (void *) proc, arg);
    PutBTTask(frame, BT_binary_getValue);
    SetBTFrame(frame);
  }

  //
  // In "discard" mode, a value that will appear (in the stream) is
  // not needed (and we don't need any special processing for it).  In
  // the end, if the value is not needed but nevertheless must be
  // processed, everyone is free to define its own processor;
  void discardOzValue() {
    putTask(BT_nop);
    putTask(BT_spointer, &blackhole);
  }

  //
  // Sometimes it is also needed to execute a task just at a
  // particular point during building. For instance, it is necessary
  // to copy an Oz value from a 'DEBUGENTRY' instruction into the
  // 'CodeArea' object itself.
  // 
  // Note: the action is executed just before the last
  // known-but-not-yet-built (that is, there is a task for it in the
  // stack) Oz value will be built. This implies that e.g. all the
  // tasks generated after that (say, we find a 'DEBUGENTRY'
  // instruction in the stream that contains two Oz values and which,
  // thus, will create two tasks) will be processed before it.  The
  // 'arg' cannot refer by now any heap-based entities, aka Oz values
  // (they won't be handled by the garbage collector).
  void schedGenAction(BuilderGenAction proc, void *arg) {
    putTask(BT_binary_doGenAction_intermediate, (void *) proc, arg);
  }

  // 
  // User can interrupt filling the area with 'suspendFillBinary()'.
  // This is needed when a term representation(s) will appear next in
  // the stream (of terms previously declared with 'getOzValue()'):
  void suspendFillBinary(BuilderOpaqueBA opaque) {
    // That's a NOP: the task remains in place;
    DebugCode(BTFrame* frame = (BTFrame *) (ToInt32(getBottom()) + opaque););
    DebugCode(GetBTTaskType(frame, type););
    Assert(type == BT_binary);
  }

  //
  // This guy either disposes the (top-level) 'fillArea' task or
  // invalidates it, so that 'buildValueOutline()' will get rid of it;
  void finishFillBinary(BuilderOpaqueBA opaque) {
    // trust user...
    BTFrame* frame = (BTFrame *) (ToInt32(getBottom()) + opaque);
    GetBTFrame(realBTFrame);
    //
    DebugCode(GetBTTaskType(frame, type););
    Assert(type == BT_binary);

    //
    if (realBTFrame == frame) {
      // able to get rid of it now;
      DiscardBTFrame(frame);
      SetBTFrame(frame);
    } else {
      ReplaceBTTask1stPtrOnly(frame, 0);
    }
  }

  //
  // 'buildValue' is the main actor: it pops tasks;
  void buildValue(OZ_Term value) {
    CrazyDebug(incDebugNODES(););
    GetBTFrame(frame);
    GetBTTaskType(frame, type);
    if (type == BT_spointer) {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      DiscardBTFrame(frame);
      SetBTFrame(frame);
      Assert(value);
      *spointer = value;
    } else {
      buildValueOutline(value, frame, type);
    }
  }

  //
  void buildValueRemember(OZ_Term value, int n) {
    buildValue(value);
    setTerm(value, n);
  }

  //
  // Handle also the case when a value is not yet constructed, so a
  // variable is temporarily substituted for it (see also
  // 'buildFSETValueRemember()');
  void buildValueRef(int n) {
    OZ_Term v = getTerm(n);
    if (v) {
      buildValue(v);
    } else {
      OZ_Term vt = oz_newVariable(oz_rootBoard());
      setTerm(vt, n);		// a reference to, of course;
      buildValue(vt);
    }
  }

  //
  // Process 'intermediate' tasks: nowadays it is abstracted away,
  // since we have to be eager with processing them 'cause of
  // cycles!
  void processSync() {
    GetBTFrame(frame);
    GetBTTaskType(frame, type);
    buildValueOutline(0, frame, type); // there is no value;
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
    setTerm(list, n);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 2);
    PutBTTaskPtr(frame, BT_spointer, l->getRefTail());
    PutBTTaskPtr(frame, BT_spointer, l->getRefHead());
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
    setTerm(dict, n);
    //
    GetBTFrame(frame);
    EnsureBTSpace(frame, size);
    while(size-- > 0) {
      PutBTTaskPtr(frame, BT_dictKey, aux);
    }
    SetBTFrame(frame);
  }

  //
  void buildArray(int low, int high) {
    OzArray *aux = new OzArray(am.currentBoard(),low,high,oz_int(0));
    buildValue(makeTaggedConst(aux));
    GetBTFrame(frame);
    int width = aux->getWidth();
    EnsureBTSpace(frame, width);
    while (width-- > 0) {
      PutBTTaskPtr(frame, BT_spointer, &aux->getRef()[width]);
    }
    SetBTFrame(frame);
  }
  void buildArrayRemember(int low, int high, int n) {
    OzArray *aux = new OzArray(am.currentBoard(),low,high,oz_int(0));
    OZ_Term array = makeTaggedConst(aux);
    buildValue(array);
    setTerm(array, n);
    GetBTFrame(frame);
    int width = aux->getWidth();
    EnsureBTSpace(frame, width);
    while (width-- > 0) {
      PutBTTaskPtr(frame, BT_spointer, &aux->getRef()[width]);
    }
    SetBTFrame(frame);
  }

  //
  void buildClonedCell() {
    OzCell *c = new OzCell(oz_currentBoard(), oz_int(0));
    buildValue(makeTaggedConst(c));
    GetBTFrame(frame);
    EnsureBTSpace(frame, 1);
    PutBTTaskPtr(frame, BT_spointer, c->getRef());
    SetBTFrame(frame);
  }
  void buildClonedCellRemember(int n) {
    OzCell *c = new OzCell(oz_currentBoard(), oz_int(0));
    OZ_Term cell = makeTaggedConst(c);
    buildValue(cell);
    setTerm(cell, n);
    GetBTFrame(frame);
    EnsureBTSpace(frame, 1);
    PutBTTaskPtr(frame, BT_spointer, c->getRef());
    SetBTFrame(frame);
  }

  //
  void knownChunk(OZ_Term chunkTerm) {
    buildValue(chunkTerm);
    putTask(BT_spointer, &blackhole);
  }

  //  
  void knownClass(OZ_Term classTerm) {
    buildValue(classTerm);
    putTask(BT_spointer, &blackhole); // class features;
  }

  //  
  void knownObject(OZ_Term objTerm) {
    buildValue(objTerm);
    putTask(BT_spointer, &blackhole); // lock;
    putTask(BT_spointer, &blackhole); // state;
    putTask(BT_spointer, &blackhole); // free record;
  }

  //
  // There is a need for 'knownProc' since a user is not supposed to
  // understand the structure of closure's (Oz) terms that are to be
  // skipped;
  void knownProc(OZ_Term procTerm) {
    buildValue(procTerm);
    //
    Abstraction *pp = (Abstraction *) tagged2Const(procTerm);
    Assert(isAbstraction(pp));
    int gsize = pp->getPred()->getGSize();
    //
    GetBTFrame(frame);
    EnsureBTSpace(frame, gsize+2); // 'name' and 'file' as well;
    for (int i = 0; i < gsize; i++) {
    PutBTTaskPtr(frame, BT_spointer, &blackhole);
    }
    PutBTTaskPtr(frame, BT_spointer, &blackhole); // name;
    PutBTTaskPtr(frame, BT_spointer, &blackhole); // file;
    SetBTFrame(frame);
  }
  void knownProcRemember(OZ_Term procTerm, int memoIndex) {
    setTerm(procTerm, memoIndex);
    knownProc(procTerm);
  }

  //
#ifdef DEBUG_CHECK
  void prRB(int n = BUILDER_RINGBUFFER_ENTRIES) { ringbuf.print(n); }
  void prST(int n = 100000) { BuilderStack::print(n); }
  void pr() { 
    prST();
    prRB();
  }
#endif  
};

#endif
