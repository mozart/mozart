/* -*- C++ -*-
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Konstantin Popov, 2000
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

#ifndef __DPMARSHALER_HH
#define __DPMARSHALER_HH

#include "base.hh"
#include "dpBase.hh"
#include "msgType.hh"
#include "byteBuffer.hh"
#include "perdio.hh"
#include "table.hh"
#include "marshalerBase.hh"
#include "gname.hh"

#ifdef INTERFACE  
#pragma interface
#endif

//
// GenTraverser's abstract entities. They are used right now for
// handling suspensions of the (dp)marshaler.
//
// GT_ExtensionSusp is fixed for extensions (see gentraverser.hh).
// generic (marshaler):
#define GT_DPMCodeAreaDesc	(GT_AE_DPMarshalerBase + 0)
#define GT_LiteralSusp		(GT_AE_DPMarshalerBase + 1)
// unmarshaler:
#define GT_DPBCodeAreaDesc	(GT_AE_DPMarshalerBase + 2)
#define GT_AtomSusp		(GT_AE_DPMarshalerBase + 3)
#define GT_UniqueNameSusp	(GT_AE_DPMarshalerBase + 4)
#define GT_CopyableNameSusp	(GT_AE_DPMarshalerBase + 5)
#define GT_NameSusp		(GT_AE_DPMarshalerBase + 6)

//
// Abstract continuation for literals' 'suspendAC':
class DPMarshalerLitSusp : public GTAbstractEntity, 
			   public CppObjMemory {
private:
  OZ_Term lt;
  int totalSize;
  int index;

  //
public:
  DPMarshalerLitSusp(OZ_Term ltIn, int tsIn)
    : lt(ltIn), totalSize(tsIn), index(0) {
    Assert(oz_isLiteral(ltIn));
    Assert(totalSize >= 0);
  }
  virtual ~DPMarshalerLitSusp() {
    DebugCode(lt = (OZ_Term) -1);
    DebugCode(totalSize = index = -1);
  }

  //
  int getTotalSize() { return (totalSize); }
  int getCurrentSize() { return (totalSize - index); }
  const char* getRemainingString() {
    Assert(index <= totalSize);
    Literal *lit = tagged2Literal(lt);
    const char *pn = lit->getPrintName();
    return (pn + index);
  }
  void incIndex(int inc) {
    index += inc;
    Assert(index < totalSize);
  }

  //
  virtual int getType() { return (GT_LiteralSusp); }
  virtual void gc() {
    Assert(lt);
    oz_gCollectTerm(lt, lt);
  }
};

//
// Code area descriptors for suspendable marshaler are extended with
// support for split instructions (e.g. 'match')
class DPMarshalerCodeAreaDescriptor : public MarshalerCodeAreaDescriptor {
private:
  // index in the hash table we're suspended on;
  int htIndex;
  // number of hash table entries that have already been processed;
  int htNDone;
#if defined(DEBUG_CHECK)
  // number of remaining entries *before* suspension;
  int htREntries;
  // The hash table we're wrestling with;
  IHashTable *htable;
#endif

public:
  // Note: htNDone must be initialized (to zero);
  DPMarshalerCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn)
    : MarshalerCodeAreaDescriptor(startIn, endIn), htNDone(0) {
    DebugCode(htIndex = -1);
  }
  virtual ~DPMarshalerCodeAreaDescriptor() {
    DebugCode(start = end = current = (ProgramCounter) -1;);
    DebugCode(htIndex = htNDone = -1;);
    DebugCode(htREntries = -1;);
    DebugCode(htable = (IHashTable *) -1;);
  }

  //
  virtual int getType() { return (GT_DPMCodeAreaDesc); }
  // there are no problems with code areas: the corresponding
  // abstraction are collected themselves;
  virtual void gc() { }

  //
  int getHTIndex() { return (htIndex); }
  void setHTIndex(int htIndexIn) { htIndex = htIndexIn; }
  int getHTNDone() { return (htNDone); }
  void setHTNDone(int htNDoneIn) { htNDone = htNDoneIn; }
#if defined(DEBUG_CHECK)
  int getHTREntries() { return (htREntries); }
  void setHTREntries(int htREntriesIn) { htREntries = htREntriesIn; }
  IHashTable* getHTable() { return (htable); }
  void setHTable(IHashTable *htableIn) { htable = htableIn; }
#endif
};

//
class DPBuilderCodeAreaDescriptor : public BuilderCodeAreaDescriptor {
private:
  // number of hash table entries that have already been processed;
  int htNDone;

public:
  // Note: htNDone must be initialized (to zero);
  DPBuilderCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn,
			      CodeArea *codeIn)
    : BuilderCodeAreaDescriptor(startIn, endIn, codeIn), htNDone(0) {}
  virtual ~DPBuilderCodeAreaDescriptor() {
    DebugCode(start = end = current = (ProgramCounter) -1;);
    DebugCode(code = (CodeArea *) -1;);
    DebugCode(htNDone = -1;);
  }

  //
  int getHTNDone() { return (htNDone); }
  void setHTNDone(int htNDoneIn) { htNDone = htNDoneIn; }

  //
  virtual int getType() { return (GT_DPBCodeAreaDesc); }
  virtual void gc();
};

//
Bool dpMarshalHashTableRef(GenTraverser *gt,
			   DPMarshalerCodeAreaDescriptor *desc,
			   int start, IHashTable *table,
			   ByteBuffer *bs);

#ifdef USE_FAST_UNMARSHALER   
ProgramCounter
dpUnmarshalHashTableRef(Builder *b,
			ProgramCounter pc, MarshalerBuffer *bs,
			DPBuilderCodeAreaDescriptor *desc, Bool &suspend);
#else
ProgramCounter
dpUnmarshalHashTableRefRobust(Builder *b,
			      ProgramCounter pc, MarshalerBuffer *bs,
			      DPBuilderCodeAreaDescriptor *desc,
			      Bool &suspend,
			      int *error);
#endif


//
class VariableExcavator;

//
// Deferred (or lazy) and concurrent (or suspendable) marshaling
// require marshaling a snapshot of a value as it existed at the
// moment of message generation by the protocol layer. This is because
// messages are considered sent at the moment of their generation, so
// any subsequent activity of the (centralized) engine does not
// influence their content.
//
// Conceptually, such a snapshot can be either real or virtual. Real
// snapshot can be achieved by copying the datastructure, which is
// considered to be unacceptable. Another way to obtain effectively a
// real snapshot is to prohibit modification of the original data
// structure, which in the current implementation model is effectively
// prohibited by possible starvation of bindings of variables
// contained in the messages being sent out concurrently to the
// binder. Just imagine two threads that keep sending out messages
// referencing a variable: due to interleaving of marshaling, at every
// point in time there could be at least one marshaler that marshals a
// data structure referencing that variable. So, the variable will
// never be bound. QED.
// 
// In our framework there are problems with a virtual snapshot too.
// Virtual snapshot means that the emulator is free to bind variables
// as it goes, while some internal marshaler logic recovers the
// original structure of the value.  Marshaling a virtual snapshot
// means that for each variable that occured in the value before
// marshaling has begun, the variable itself and eventually its value
// are recognized to be the same and treated as the original variable.
// Speaking in terms of nodes in a value, variable nodes and value
// nodes that substitute them must be recognized to be the same.  In
// the current model, variable locations are used to identify them,
// and locations could be also used to identify values as well.
// Unfortunately, the garbage collector eliminates the location of a
// bound variable by shortening reference chains to that variable.
// Alternatively, if values are identified by values themselves
// (that's how it is done, and how it should be done since values can
// be replicated), bound variables cannot be uniquely identified due
// to sharing of values.
//
// Despite these problems, our implementation utilizes the "take a
// virtual snapshot by catching locations of bound variables".
// Elimination of locations of bound variable by the GC is fixed by
// additional pre-GC and post-GC phases. Pre-GC phase reverts a
// binding of a variable to a fresh variable of a special kind. As a
// result, the location of the former variable is kept. The special
// variable references the overwritten value, so it is not lost after
// the GC. Post-GC phase restores the value back.
// 
// More technically, sending a message proceeds as follows.
//
// When a message is generated, it is scanned for variables. Locations
// of all variables are remembered. Local variables are globalized.
// One credit is taken from manager variables, so that they cannot be
// accidently localized back and disappear. Copies of tagged
// references to variable managers are remembered; these are used
// later for marshaling. Proxies can disappear, so they are exported
// right away. An exported variable proxy looks like a variable proxy,
// but its marshaling is just the conversion from its internal
// representation into the marshaled one. Exported variable proxies
// are only visible to its marshaler. Note that they must be marshaled
// exactly once. Tagged references to exported variable proxies are
// remembered and used later for marshaling.
//
// When a marshaling step begins, locations of variables are reverted
// to saved copies of variable managers and exported variable proxies.
// Note that is done regardless whether variables has been bound or
// not. At the end of a step, original tagged references are restored
// back.
//
// When marshaling finishes, credits of variable managers are returned
// back, and exported variable proxies are destroyed.
//
// There is an important optimization (cf. Per): variables that are
// sent in a message to a site can be "auto-registered" for that site.
// Auto-registration means that the site is entered into the variable
// manager's proxy list already at the message sending; the "register"
// message from the site is then omitted. This results in faster
// propagation of a variable binding.
//
// This optimization could be optimized even further by delaying
// globalization of local variables. However, the globalization cannot
// be delayed until e.g. marshaling. As an example consider a message
// M0 that references a variable. If its globalization is postponed,
// then it can be concurrently globalized & marshaled by some other
// thread/marshaler for a message M-, so the receiving site obtains a
// proxy of the variable. Now, should be globalization postponed
// beyond binding, the M0 will contain already a variable's value. The
// receiving site will observe then both the variable proxy due to M-
// and its value due to M0. Due to FIFO of message delivery, the
// message Mvb with a binding of the variable is delivered after M0.
// Should a network failure occur before Mvb's delivery, the receiving
// site has a hidden failure point associated with the variable proxy.
//
// The problem does not exist if globalization cannot be postponed
// beyond thread preemption either due to I/O or another thread.
// However, that still covers the interesting case of concurrent
// producer thread and a thread that sends produced values over the
// network.
//

#if defined(DEBUG_CHECK)
class MsgTermSnapshot;
class MsgTermSnapshotImpl;
#endif

//
class DPMarshaler : public GenTraverser {
  friend class VariableExcavator;
private:
  // Support for lazy protocols: when 'doToplevel' is set to TRUE, a
  // complete representation of the first object (etc.?) is generated,
  // as opposed to its stub.
  //
  // Note that 'DPMarshaler' is not specialized (as opposed to the
  // 'GenTraverser's spirit, when every single variation of a
  // marshaling activity has its own class). This is because marshaler
  // objects are large and, hence, cannot be re-created all the time.
  // Instead, a pool of standard 'DPMarshaler's is kept.
  Bool doToplevel;
#if defined(DEBUG_CHECK)
  OZ_Term mValue;
  MsgTermSnapshotImpl *mts;
#endif

  //
public:
  DPMarshaler() : doToplevel(FALSE) {}

  //
  virtual ~DPMarshaler() {}
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual Bool processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processVar(OZ_Term cv, OZ_Term *varTerm);
  virtual void processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  virtual void processSync();

  //
  // Support for lazy protocols: next marshaling will generate a full
  // representation of a top-level object (as opposed to its stub);
  void genFullToplevel() { doToplevel = TRUE; }
  DebugCode(Bool isFullToplevel() { return (doToplevel); })

  //
#if defined(DEBUG_CHECK)
  void traverse(OZ_Term t) {
    mValue = t;
    GenTraverser::traverse(t);
  }
  void prepareTraversing(Opaque *o, MsgTermSnapshot *mtsIn) {
    mts = (MsgTermSnapshotImpl *) mtsIn;
    GenTraverser::prepareTraversing(o);
  }
  virtual void appTCheck(OZ_Term term);
#endif

  //
private:
  Bool marshalObjectStub(OZ_Term objTerm, ConstTerm *objConst);
  Bool marshalFullObject(OZ_Term objTerm, ConstTerm *objConst);
};

//
// Extract variables from a term into a list (former '::digOutVars()'
// business;)
class VariableExcavator : public GenTraverser {
private:
  Bool doToplevel;
  OZ_Term vars;

  //
private:
  void addVar(OZ_Term v) { vars = oz_cons(v, vars); }

  //
public:
  virtual ~VariableExcavator() {}
  void init(Bool full) { vars = oz_nil(); doToplevel = full; }

  //
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual Bool processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processVar(OZ_Term cv, OZ_Term *varTerm);
  virtual void processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  virtual void processSync();

  //
  // This guy is capable of re-mapping the DPMarshaler's task
  // "processor"s into corresponding VariableExcavator's ones. So,
  // it's not a generic routine - sadly but true;
  // 
  // Not in use since we take a snapshot of a value ahead of
  // marshaling now;
  //   void copyStack(DPMarshaler *dpm);

  //
  OZ_Term getVars()      { return (vars); }
};


//
// Blocking factor for binary areas: how many Oz values a binary area
// may contain (in fact, modulo a constant factor: code area"s, for
// instance, count instruction fields with Oz values but not values
// themselves);
const int ozValuesBADP = 1024;
// const int ozValuesBADP = 1;

//
extern VariableExcavator ve;

//
inline
OZ_Term extractVars(OZ_Term in, Bool full)
{
  ve.init(full);
  ve.prepareTraversing((Opaque *) 0);
  ve.traverse(in);
  ve.finishTraversing();
  return (ve.getVars());
}

//
// Not in use since we take a snapshot of a value ahead of
// marshaling now;
//
//  static inline 
//  OZ_Term extractVars(DPMarshaler *dpm)
//  {
//    ve.init();
//    ve.prepareTraversing((Opaque *) 0);
//    ve.copyStack(dpm);
//    ve.traverse();
//    ve.finishTraversing();
//    return (ve.getVars());
//  }

//
// That's the corresponding 'CodeAreaProcessor':
Bool dpMarshalCode(GenTraverser *m, GTAbstractEntity *arg);
// 'traverseCode' is shared with the centralized system (logically,
// yeah?)
Bool traverseCode(GenTraverser *m, GTAbstractEntity *arg);


//
// Virtual snapshot business. 

//
class SntVarLocation : public CppObjMemory {
private:
  OZ_Term loc;
  // A copy of the original OZ_Term for managers, or a fresh "var"
  // tagged pointer to a dedicated "exported" proxy;
  OZ_Term var;
  OZ_Term aVal;			// actual value;
  //
  SntVarLocation *next;

public:
  SntVarLocation(OZ_Term locIn, OZ_Term varIn, SntVarLocation *nextIn)
    : loc(locIn), var(varIn), next(nextIn) {
    Assert(oz_isRef(locIn));
    DebugCode(aVal = (OZ_Term) -1;);
  }

  //
  OZ_Term getLoc() { Assert(oz_isRef(loc)); return (loc); }
  OZ_Term getVar() { return (var); }
  //
  OZ_Term &getLocRef() { return (loc); }
  OZ_Term &getVarRef() { return (var); }
  void gcSetLoc(OZ_Term locIn) { loc = locIn; }
  //
  void saveValue(OZ_Term value) { aVal = value; }
  OZ_Term getSavedValue() { return (aVal); }
  //
  SntVarLocation* getNextLoc() { return (next); }
};  

//
#define MTS_NONE	0x0
#define MTS_SET		0x1	// snapshot is currently actualized;

//
// MsgTermSnapshot"s are opaque for the container itself, but have to
// be kept here just 'cause the snapshot is taken way before ever
// begins;
class MsgTermSnapshot {};

//
class MsgTermSnapshotImpl : public MsgTermSnapshot,
			    public CppObjMemory {
private:
  DebugCode(int flags;);
  SntVarLocation *locs;

  //
public:
  MsgTermSnapshotImpl(SntVarLocation *l) 
    : locs(l) {  DebugCode(flags = MTS_NONE;); }

  //
  SntVarLocation* getLocs() { return (locs); }

  //
  void install()   {
    Assert(!(flags&MTS_SET));
    SntVarLocation* l = locs;
    while (l) {
      OZ_Term vr = l->getLoc();
      OZ_Term *vp = tagged2Ref(vr);
      OZ_Term v = l->getVar();
      l->saveValue(*vp);
      *vp = v;
      l = l->getNextLoc();
    }
    DebugCode(flags |= MTS_SET);
  }

  //
  void deinstall() {
    Assert(flags&MTS_SET);
    SntVarLocation* l = locs;
    while (l) {
      OZ_Term vr = l->getLoc();
      OZ_Term *vp = tagged2Ref(vr);
      OZ_Term val = l->getSavedValue();
      DebugCode(l->saveValue((OZ_Term) -1););
      *vp = val;
      l = l->getNextLoc();
    }
    DebugCode(flags &= ~MTS_SET);
  }

  //
  // 'start'/'finish' methods make sure that locations are kept by
  // storing there temporary "GCStubVar"s.
  //
  // 'gc()' collects hidden values (manager variables that are already
  // bound locally and "exported proxies", to be precise).
  void gcStart();
  void gc();
  void gcFinish();

  //
#if defined(DEBUG_CHECK)
  void checkVar(OZ_Term t);
  int getSize() {
    SntVarLocation* l = locs;
    int cnt = 0;
    while (l) {
      cnt++;
      l = l->getNextLoc();
    }
    return (cnt);
  }
#endif
};

//
// auxilary service functions;
static inline
void dpMarshalerStartBatch(ByteBuffer *bs, DPMarshaler *dpm,
			   MsgTermSnapshot *mtsIn)
{
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  mts->install();
}

//
static inline
DPMarshaler* dpMarshalerFinishBatch(ByteBuffer *bs, DPMarshaler *dpm,
				    MsgTermSnapshot *mtsIn)
{
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  mts->deinstall();
  if (dpm->isFinished()) {
    dpm->finishTraversing();
    marshalDIF(bs, DIF_EOF);
    return ((DPMarshaler *) 0);
  } else {
    return (dpm);
  }
}

//
SntVarLocation* takeSntVarLocsOutline(OZ_Term vars, DSite *dest);
void deleteSntVarLocsOutline(SntVarLocation *locs);

//
// Interface procedures;
//

//
inline
MsgTermSnapshot* takeTermSnapshot(OZ_Term t, DSite *dest, Bool full) {
  OZ_Term vars = extractVars(t, full);
  SntVarLocation *svl;
  //
  svl = oz_isNil(vars) ?
    (SntVarLocation *) 0 : takeSntVarLocsOutline(vars, dest);
  return (new MsgTermSnapshotImpl(svl));
}

//
inline
void deleteTermSnapshot(MsgTermSnapshot *mtsIn) {
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  SntVarLocation *locs = mts->getLocs();
  //
  if (locs) deleteSntVarLocsOutline(locs);
  delete mts;
}

//
inline 
void gcTermSnapshot(MsgTermSnapshot *mtsIn) {
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  mts->gc();
}

//
inline
void mtsStartGC(MsgTermSnapshot *mtsIn) {
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  mts->gcStart();
}
inline
void mtsFinishStartGC(MsgTermSnapshot *mtsIn) {
  MsgTermSnapshotImpl *mts = (MsgTermSnapshotImpl *) mtsIn;
  mts->gcFinish();
}

///
// 'dpMarshalTerm' with three arguments is called when a first frame
// is marshaled, and with two arguments - for subsequent frames;
inline
DPMarshaler* dpMarshalTerm(ByteBuffer *bs, DPMarshaler *dpm,
			   OZ_Term term, MsgTermSnapshot *mts)
{
  Assert(dpm->isFinished());
  Assert(mts);
  //
  dpm->prepareTraversing((Opaque *) bs DebugArg(mts));
  //
  dpMarshalerStartBatch(bs, dpm, mts);
  //
  dpm->traverse(term);
  //
  return (dpMarshalerFinishBatch(bs, dpm, mts));
}

//
inline
DPMarshaler* dpMarshalTerm(ByteBuffer *bs, DPMarshaler *dpm,
			   MsgTermSnapshot *mts)
{
  Assert(!dpm->isFinished());
  Assert(mts);
  //
  dpMarshalerStartBatch(bs, dpm, mts);
  //
  dpm->resume((Opaque *) bs);
  //
  return (dpMarshalerFinishBatch(bs, dpm, mts));
}

//
// Unmarshaling;
//

//
class DPUnmarshalerLitSusp : public GTAbstractEntity, 
			     public CppObjMemory {
private:
  int refTag;
  int nameSize;
  char *printname;
  int pnSize;

  //
public:
  DPUnmarshalerLitSusp(int refTagIn, int nameSizeIn,
		       char *printnameIn, int pnSizeIn)
    : refTag(refTagIn), nameSize(nameSizeIn),
      printname(printnameIn), pnSize(pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerLitSusp() {
    delete printname;
    DebugCode(printname = (char *) 0);
    DebugCode(refTag = nameSize = pnSize = -1);
  }

  //
  virtual int getType() = 0;
  virtual void gc() = 0;

  //
  int getRefTag() { return (refTag); }
  int getNameSize() { return (nameSize); }
  char *getPrintname() { return (printname); }
  int getPNSize() { return (pnSize); }

  //
  void appendPrintname(int size, char *app) {
    char *npn = new char[pnSize + size + 1];
    strncpy(npn, printname, pnSize);
    strncpy(npn+pnSize, app, size);
    pnSize += size;
    npn[pnSize] = (char) 0;
    delete printname;
    printname = npn;
  }
};

//
class DPUnmarshalerAtomSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerAtomSusp(int refTagIn, int nameSizeIn,
			char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerAtomSusp() {}

  //
  virtual int getType() { return (GT_AtomSusp); }
  virtual void gc() {}
};

//
class DPUnmarshalerUniqueNameSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerUniqueNameSusp(int refTagIn, int nameSizeIn,
			      char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerUniqueNameSusp() {}

  //
  virtual int getType() { return (GT_UniqueNameSusp); }
  virtual void gc() {}
};

//
class DPUnmarshalerCopyableNameSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerCopyableNameSusp(int refTagIn, int nameSizeIn,
				char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerCopyableNameSusp() {}

  //
  virtual int getType() { return (GT_CopyableNameSusp); }
  virtual void gc() {}
};


//
class DPUnmarshalerNameSusp : public DPUnmarshalerLitSusp {
private:
  OZ_Term value;
  GName *gname;

  //
public:
  DPUnmarshalerNameSusp(int refTagIn, int nameSizeIn, OZ_Term valueIn,
			GName *gnameIn, char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn),
      value(valueIn), gname(gnameIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerNameSusp() {}

  //
  virtual int getType() { return (GT_NameSusp); }
  virtual void gc() {
    if (gname) {
      Assert(!value);
      gCollectGName(gname);
    } else {
      Assert(value);
      oz_gCollectTerm(value, value);
    }
  }

  //
  OZ_Term getValue() { return (value); }
  GName *getGName() { return (gname); }
};


//
OZ_Term dpUnmarshalTerm(ByteBuffer *bs, Builder *dpb);

//
inline
void dpUnmarshalerStartBatch(Builder *b)
{
  b->prepareBuild();
}
inline
void dpUnmarshalerFinishBatch(Builder *b) {}

//
class SendRecvCounter;
extern SendRecvCounter mess_counter[];

//
void marshalDSite(MarshalerBuffer *, DSite *);
void marshalVarObject(ByteBuffer *bs, int BTI, GName *gnobj, GName *gnclass);

// '-1' means the action is suspended;

// var.cc
OZ_Term
#ifdef USE_FAST_UNMARSHALER   
unmarshalBorrow(MarshalerBuffer *bs, OB_Entry *&ob, int &bi);
#else
unmarshalBorrowRobust(MarshalerBuffer *bs,
		      OB_Entry *&ob, int &bi, int *error);
#endif

void marshalBorrowHead(MarshalerBuffer *bs, MarshalTag tag, int bi);
void saveMarshalBorrowHead(int bi, DSite* &ms, int &oti,
			   Credit &c);
void marshalBorrowHeadSaved(MarshalerBuffer *bs, MarshalTag tag, DSite *ms,
			    int oti, Credit c);
void discardBorrowHeadSaved(DSite *ms, int oti,
			    Credit credit);
void marshalToOwner(MarshalerBuffer *bs, int bi);
void saveMarshalToOwner(int bi, int &oti,
			Credit &c);
void marshalToOwnerSaved(MarshalerBuffer *bs,Credit c,
			 int oti);
inline
void discardToOwnerSaved(DSite *ms, int oti,Credit c) {
  discardBorrowHeadSaved(ms, oti, c);
}
void marshalOwnHead(MarshalerBuffer *bs, int tag, int i);
void saveMarshalOwnHead(int oti, Credit &c);
void marshalOwnHeadSaved(MarshalerBuffer *bs, int tag, int oti, Credit c);
void discardOwnHeadSaved(int oti, Credit c);

//
void marshalTertiary(ByteBuffer *bs, Tertiary *t, MarshalTag tag);
#ifdef USE_FAST_UNMARSHALER   
OZ_Term unmarshalTertiary(MarshalerBuffer *bs, MarshalTag tag);
OZ_Term unmarshalOwner(MarshalerBuffer *bs, MarshalTag mt);
#else
OZ_Term unmarshalTertiaryRobust(MarshalerBuffer *bs,
				MarshalTag tag, int *error);
OZ_Term unmarshalOwnerRobust(MarshalerBuffer *bs,
			     MarshalTag mt, int *error);
#endif
void marshalObject(MarshalerBuffer *bs, ConstTerm* t);
void marshalSPP(MarshalerBuffer *bs, TaggedRef entity, Bool trail);


//
// The maximal sizes for the marshal* routines are defined as follows:
// aux:
#define MCreditMaxSize MNumberMaxSize 
#define MBaseSiteMaxSize (3*MNumberMaxSize + MShortMaxSize)
#define MDSiteMaxSize (DIFMaxSize + MBaseSiteMaxSize)
#define MBorrowHeadMaxSize (2*DIFMaxSize + 2*MDSiteMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MOwnHeadMaxSize (2*DIFMaxSize + MDSiteMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MObjectVarMaxSize (MBorrowHeadMaxSize + 2*MGNameMaxSize)
// now:
#define MTertiaryMaxSize MBorrowHeadMaxSize
#define MDistVarMaxSize MObjectVarMaxSize
#define MDistObjectMaxSize MObjectVarMaxSize
#define MDistSPPMaxSize MOwnHeadMaxSize

//
// Top-level management of marshalers (to be used by transport
// objects). 

//
#define MUEmpty          0x0
#define MUMarshalerBusy  0x1
#define MUBuilderBusy    0x2
//
typedef struct {
  int flags;
  DPMarshaler *m;
  Builder *b;
} MU;				// marshaler unit

//
class DPMarshalers {
private:
  MU *mus;
  int musNum;

  //
protected:
  DPMarshalers() : mus((MU *) 0), musNum(0) {}
  ~DPMarshalers() { dpAllocateMarshalers(0); }

  //
  // before any marshalers can be obtained, they must be allocated:
  void dpAllocateMarshalers(int numof);
  int getMNum() { return (musNum); }

  // 
  DPMarshaler* dpGetMarshaler();
  Builder* dpGetUnmarshaler();
  void dpReturnMarshaler(DPMarshaler* dpm);
  void dpReturnUnmarshaler(Builder* dpb);
};

//
extern DPMarshaler **dpms;
extern Builder **dpbs;

#endif
