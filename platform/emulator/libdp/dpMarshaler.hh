/* -*- C++ -*-
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
 *    Boriss Mejias <bmc@info.ucl.ac.be>
 *    Raphael Collet <raph@info.ucl.ac.be>
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
#include "byteBuffer.hh"
#include "marshalerBase.hh"
#include "gname.hh"
#include "glue_entities.hh"

#ifdef INTERFACE  
#pragma interface
#endif

//
// Define if you want to take the snapshot before first marshaling
// step:
// #define EVAL_EAGER_SNAPSHOT

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
  DPMarshalerCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn,
				AddressHashTableO1Reset *lITin)
    : MarshalerCodeAreaDescriptor(startIn, endIn, lITin), htNDone(0) {
    DebugCode(htIndex = -1);
  }
  virtual ~DPMarshalerCodeAreaDescriptor() {
    DebugCode(start = end = current = (ProgramCounter) -1;);
    DebugCode(lIT = (AddressHashTableO1Reset *) -1;);
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

ProgramCounter
dpUnmarshalHashTableRef(Builder *b,
			ProgramCounter pc, MarshalerBuffer *bs,
			DPBuilderCodeAreaDescriptor *desc, Bool &suspend);


//
// Deferred (or lazy) and concurrent (or suspendable) marshaling
// require marshaling a snapshot of a value as it existed at the
// moment of message generation by the protocol layer. This is because
// messages are considered sent at the moment of their generation (or,
// as it is allowed by Oz - a little bit later, e.g. when first
// marshaling step has begun), so any subsequent activity of the
// (centralized) engine does not influence their content.
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
// Globalization of local variables can be (and actually IS in the
// current system) delayed until (first phase) marshaling, and that is
// transparent to the receiving site since it cannot distinguish
// between points of message generations and message marshaling.
//

//
#define ValuesITInitSize	2048
#define LocationsITInitSize	256

//
// There are two marshalers, for the first and seconds stages.  Both
// are inherited from the 'DPMarshaler' class (which does not contain
// the marshaling code).

//
class DPMarshaler : public GenTraverser {
  friend class VSnapshotBuilder;
protected:
  MarshalerDict *vIT;
  AddressHashTableO1Reset *lIT;
  OzValuePatch *expVars;
  OZ_Term gcExpVars;		// "OZ_Term" version of 'expVars';

  //
  // Support for lazy protocols: when 'doToplevel' is set to TRUE, a
  // complete representation of the first object (etc.?) is generated,
  // as opposed to its stub.
  Bool doToplevel;

  // Defines wich of the proxy-marshal interfaces to se. 
  Bool pushTerm;  
  //
public:
  DPMarshaler() : doToplevel(FALSE), pushTerm(FALSE) {
    lIT = new AddressHashTableO1Reset(LocationsITInitSize);
    vIT = new MarshalerDict(ValuesITInitSize);
    expVars = (OzValuePatch *) 0;
    gcExpVars = (OZ_Term) 0;
  }
  ~DPMarshaler() {
    delete lIT;
    delete vIT;
    DebugCode(expVars = (OzValuePatch *) -1;);
    DebugCode(lIT = (AddressHashTableO1Reset *) -1;);
    DebugCode(vIT = (MarshalerDict *) -1;);
    DebugCode(gcExpVars = (OZ_Term) -1;);
  }

  //
  void reset() {
    GenTraverser::reset();
    lIT->mkEmpty();
    vIT->mkEmpty();
    if (expVars) {
      deleteOzValuePatch(expVars);
      expVars = (OzValuePatch *) 0;
    }
    Assert(gcExpVars == (OZ_Term) 0);
  }

  //
  void gcStart() {
    Assert(gcExpVars == (OZ_Term) 0);
    if (expVars) {
      gcExpVars = gcStartOVP(expVars);
      DebugCode(expVars = (OzValuePatch *) 0;);
      Assert(gcExpVars != (OZ_Term) 0);
    }
  }
  void gcFinish() {
    Assert(expVars == (OzValuePatch *) 0);
    if (gcExpVars) {
      expVars = gcFinishOVP(gcExpVars);
      gcExpVars = (OZ_Term) 0;
      Assert(expVars != (OzValuePatch *) 0);
    }
  }

  //
  void gCollect() {
    Assert(expVars == (OzValuePatch *) 0);
    GenTraverser::gCollect();
    vIT->gCollect();
    oz_gCollectTerm(gcExpVars, gcExpVars);
  }

  //
  OzValuePatch* getExpVars() { return (expVars); }
  void setExpVars(OzValuePatch *p) { expVars = p; }

  //
  // Support for lazy protocols: next marshaling will generate a full
  // representation of a top-level object (as opposed to its stub);
  void genFullToplevel() { doToplevel = TRUE; }
  Bool isFullToplevel() { return (doToplevel); }

  void pushContents(){ pushTerm = TRUE;}
  Bool isPushContents(){ return pushTerm; }
  //
  MarshalerDict *getVIT() { return (vIT); }
};

//
class DPMarshaler1stP : public DPMarshaler {
public:
  DPMarshaler1stP() { Assert(0); }

  //
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *unusConst);
  void processNoGood(OZ_Term resTerm);
  Bool processVar(OZ_Term cv, OZ_Term *varTerm);
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  void processSync();

  //
  void doit();
  //
  void traverse(OZ_Term t);
  void resume(Opaque *o);
  void resume();

  //
private:
  Bool marshalObjectStub(OZ_Term objTerm, ConstTerm *objConst);
  Bool marshalFullObject(OZ_Term objTerm, ConstTerm *objConst);
};

//
#define	TRAVERSERCLASS	DPMarshaler1stP
#include "gentraverserLoop.hh"
#undef	TRAVERSERCLASS

//
class DPMarshaler2ndP : public DPMarshaler {
public:
  DPMarshaler2ndP() { Assert(0); }

  //
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *unusConst);
  void processNoGood(OZ_Term resTerm);
  Bool processVar(OZ_Term cv, OZ_Term *varTerm);
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  void processSync();

  //
  void doit();
  //
  void traverse(OZ_Term t);
  void resume(Opaque *o);
  void resume();

  //
  DebugCode(void vHook();)

  //
private:
  Bool marshalObjectStub(OZ_Term objTerm, ConstTerm *objConst);
  Bool marshalFullObject(OZ_Term objTerm, ConstTerm *objConst);
};

//
#define	TRAVERSERCLASS	DPMarshaler2ndP 
#include "gentraverserLoop.hh"
#undef	TRAVERSERCLASS

// 
// Support for "virtual snapshot"s: patches that
// . keep locations of variables
// . hold internal (memory) representation of "exported" variables until
//   their external (network) representations are sent out;

//
// MarshaledVarPatch holds a location of a *marshaled* variable, of
// whichever type (but should not be marshaled itself: 'processVar()'
// methods should recognize coreferences, and marshal them
// accordingly).  It can be seen as an optimization of
// DistributedVarPatch.

class MarshaledVarPatch : public OzValuePatch {
public:
  MarshaledVarPatch(OZ_Term loc, OzValuePatch *next)
    : OzValuePatch(loc, next) {}
  ~MarshaledVarPatch() { Assert(0); }

  //
  virtual void disposeV() { 
    disposeOVP();
    oz_freeListDispose(extVar2Var(this), extVarSizeof(MarshaledVarPatch));
  }
  //
  virtual ExtVarType getIdV(void) { return (OZ_EVAR_MARSHALEDVARPATCH); }
  //
  virtual OzValuePatch* gCollectV() {
    return (new MarshaledVarPatch(*this));
  }
  virtual void gCollectRecurseV() { gcRecurseOVP(); }
};

//
// DistributedVarPatch holds a location of a variable that has not
// been marshaled yet when the snapshot took place.  The mediator of
// the patched variable is assigned to the patch itself.  This
// mediator contains all the necessary information for marshaling.

class DistributedVarPatch : public OzValuePatch {
public:
  DistributedVarPatch(OZ_Term loc, OzValuePatch *next)
    : OzValuePatch(loc, next)
  {
    OzVariable *ov = tagged2Var(oz_deref(loc));
    Assert(ov->hasMediator());
    extVar2Var(this)->setMediator(ov->getMediator());
  }
  virtual ~DistributedVarPatch() { Assert(0); }

  //
  virtual void disposeV() {
    disposeOVP();
    oz_freeListDispose(extVar2Var(this), extVarSizeof(DistributedVarPatch));
  }
  //
  virtual ExtVarType getIdV(void) { return (OZ_EVAR_DISTRIBUTEDVARPATCH); }
  //
  virtual OzValuePatch* gCollectV() {
    return (new DistributedVarPatch(*this));
  }
  virtual void gCollectRecurseV() { gcRecurseOVP(); }
};

inline
Bool oz_isDistributedVarPatch(OZ_Term v)
{
  return (oz_isExtVar(v) &&
	  (oz_getExtVar(v)->getIdV() == OZ_EVAR_DISTRIBUTEDVARPATCH));
}

inline
DistributedVarPatch *oz_getDistributedVarPatch(OZ_Term v)
{
  Assert(oz_isDistributedVarPatch(v));
  return ((DistributedVarPatch *) oz_getExtVar(v));
}

//
// Construct virtual snapshot;
class VSnapshotBuilder : public GenTraverser {
private:
  MarshalerDict *vIT;		// shared with the dpMarshaler;
  OzValuePatch *expVars;
  //
  Bool doToplevel;

  //
private:
  // This guy is re-mapping the DPMarshaler's task "processor"s into
  // corresponding internal ones.
  void copyStack(DPMarshaler *dpm);

  //
public:
  VSnapshotBuilder() {
    DebugCode(vIT = (MarshalerDict *) -1;);
    DebugCode(expVars = (OzValuePatch *) -1;);
    DebugCode(doToplevel = (Bool) -1;);
  }

  //
  void init(DPMarshaler *dpm) {
    vIT = dpm->getVIT();
    doToplevel = dpm->isFullToplevel();
    expVars = dpm->getExpVars();
    copyStack(dpm);
  }
#if defined(EVAL_EAGER_SNAPSHOT)
  void initEager(DPMarshaler *dpm, OZ_Term t) {
    dest = destIn;
    vIT = dpm->getVIT();
    doToplevel = dpm->isFullToplevel();
    expVars = (OzValuePatch *) 0;
    put(t);
  }
#endif

  void reset() {
    GenTraverser::reset();
    DebugCode(vIT = (MarshalerDict *) -1;);
    DebugCode(doToplevel = (Bool) -1;);
    DebugCode(doToplevel = (Bool) -1;);
  }
  ~VSnapshotBuilder() { reset(); }

  //
  OzValuePatch *takeExpVars () { return (expVars); }

  //
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *unusConst);
  void processNoGood(OZ_Term resTerm);
  Bool processVar(OZ_Term cv, OZ_Term *varTerm);
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  void processSync();

  //
  void doit();			// actual processor;
  //
  void traverse(OZ_Term t);
  void resume(Opaque *o);
  void resume();
};

//
#define	TRAVERSERCLASS	VSnapshotBuilder
#include "gentraverserLoop.hh"
#undef	TRAVERSERCLASS

//
// Blocking factor for binary areas: how many Oz values a binary area
// may contain (in fact, modulo a constant factor: code area"s, for
// instance, count instruction fields with Oz values but not values
// themselves);
const int ozValuesBADP = 1024;
// const int ozValuesBADP = 1;

//
extern VSnapshotBuilder vsb;

//
// That's the corresponding 'CodeAreaProcessor':
Bool dpMarshalCode(GenTraverser *m, GTAbstractEntity *arg);
// 'traverseCode' is shared with the centralized system (logically,
// yeah?)
Bool traverseCode(GenTraverser *m, GTAbstractEntity *arg);

//
// Interface procedures;
//

//
// 'dpMarshalTerm' with four arguments is called when a first frame
// is marshaled, and with two arguments - for subsequent frames;
inline
DPMarshaler* dpMarshalTerm(OZ_Term term, ByteBuffer *bs, DPMarshaler *dpmIn)
{
  Assert(dpmIn->isFinished());
#if defined(EVAL_EAGER_SNAPSHOT)
  DPMarshaler2ndP *dpm = (DPMarshaler2ndP *) dpmIn;
  OzValuePatch *expVars;
#else
  DPMarshaler1stP *dpm = (DPMarshaler1stP *) dpmIn;
#endif

  //
#if defined(EVAL_EAGER_SNAPSHOT)
  vsb.initEager(dpm, term);
  DebugCode(vsb.prepareTraversing((Opaque *) bs));
  vsb.resume();
  vsb.finishTraversing();
  dpm->setExpVars(vsb.takeExpVars());
#endif

  //
  dpm->prepareTraversing((Opaque *) bs);
#if defined(EVAL_EAGER_SNAPSHOT)
  // Note that the patch can grow, but newly elements do not need to
  // be deinstalled:
  expVars = dpm->getExpVars();
  if (expVars) installOVP(expVars);
#endif
  dpm->traverse(term);
#if defined(EVAL_EAGER_SNAPSHOT)
  if (expVars) deinstallOVP(expVars);
#endif

  //
  if (dpm->isFinished()) {
    dpm->finishTraversing();
    marshalDIF(bs, DIF_EOF);
    return ((DPMarshaler *) 0);
  } else {
#if !defined(EVAL_EAGER_SNAPSHOT)
    // Now, take the snapshot:
    vsb.init(dpm);	// copy the marshaler's stack, among all;
    DebugCode(vsb.prepareTraversing((Opaque *) bs));
    vsb.resume();		// .. off the just copied stack;
    vsb.finishTraversing();
    dpm->setExpVars(vsb.takeExpVars());
#endif
    //
    return (dpm);
  }
}

//
inline
DPMarshaler* dpMarshalContTerm(ByteBuffer *bs, DPMarshaler *dpmIn)
{
  Assert(!dpmIn->isFinished());
  DPMarshaler2ndP *dpm = (DPMarshaler2ndP *) dpmIn;
  OzValuePatch *expVars;

  //
  expVars = dpm->getExpVars();
  if (expVars) installOVP(expVars);
  dpm->resume((Opaque *) bs);
  if (expVars) deinstallOVP(expVars);

  //
  if (dpm->isFinished()) {
    dpm->finishTraversing();
    marshalDIF(bs, DIF_EOF);
    return ((DPMarshaler *) 0);
  } else {
    return (dpm);
  }
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

// '-1' means the action is suspended;

//
// var.cc
void marshalObject(MarshalerBuffer *bs, ConstTerm* t);

//
// The maximal sizes for the marshal* routines are defined as follows:
// aux:
// kost@ : determining the size of a marshaled credit is broken right now:
//         marshaling of only up to MMaxNumOfCreditRRs RR"s is allowed.
#define MMaxNumOfCreditRRs 5
#define MCreditRRSize (3*MNumberMaxSize)
#define MCreditMaxSize							\
  (DIFMaxSize + MNumberMaxSize + MMaxNumOfCreditRRs*MCreditRRSize)
#define MCreditToOwnerMaxSize						\
  (DIFMaxSize + 2*MNumberMaxSize + MMaxNumOfCreditRRs*MCreditRRSize)
#define MBaseSiteMaxSize (3*MNumberMaxSize + MShortMaxSize)
#define MDSiteMaxSize (DIFMaxSize + MBaseSiteMaxSize)
#define MBorrowHeadMaxSize						\
  (DIFMaxSize + MDSiteMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MOwnHeadMaxSize							\
  (2*DIFMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MToOwnerMaxSize MCreditToOwnerMaxSize
#define MRefConsInfoMaxSize (MDSiteMaxSize + MCreditMaxSize) 

// raph: maximal size for marshaling distributed entities.  The
// DssProxyMaxSize is only speculative, and might be underevaluated.
// When marshaling, the DSS assumes there is enough space available in
// the provided buffers.  Increase DssProxyMaxSize if you encounter
// DSS errors when marshaling.
#define MDssProxyMaxSize  172
#define MMediatorMaxSize  MDssProxyMaxSize + 1   // (proxy + glue tag)

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
public:
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
extern DPMarshalers *DPM_Repository;

#endif
