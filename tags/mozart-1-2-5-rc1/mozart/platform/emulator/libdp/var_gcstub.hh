/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Konstantin Popov (2000)
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

#ifndef __VAR_GCSTUB_HH
#define __VAR_GCSTUB_HH

#if defined(INTERFACE)
#pragma interface
#endif

#include "dpBase.hh"
#include "var_ext.hh"

//
// The stub that keeps the location of a maybe already bound variable.
// It should be only visible to the GC. With some twist it could be
// replaced with ordinary simple variables though, but (a) that twist
// (exactly speaking, 'MsgTermSnapshotImpl::gc()' should then keep &
// collect those overwritten values, and also the temporary variables
// should be marked such that multiple virtual snapshots will not
// overwrite the same location multiple times) has to be done, and (b)
// debugging is much easier with this explicit solution.
//
class GCStubVar : public ExtVar {
private:
  OZ_Term val;

  //
public:
  GCStubVar(OZ_Term valIn) : ExtVar(oz_rootBoard()), val(valIn) {}
  virtual ~GCStubVar() { Assert(0); }

  //
  OZ_Term getValue() { return (val); }

  //
  virtual ExtVarType    getIdV() { return (OZ_EVAR_GCSTUB); }
  virtual ExtVar*       gCollectV() { return new GCStubVar(*this); }
  virtual void          gCollectRecurseV() { oz_gCollectTerm(val, val); }
  virtual ExtVar*       sCloneV() {
    Assert(0);
    return ((GCStubVar *) 0);
  }
  virtual void          sCloneRecurseV() { Assert(0); }

  //
  virtual OZ_Return     unifyV(TaggedRef *lPtr, TaggedRef *rPtr) {
    Assert(0);
    return (FAILED);
  }
  virtual OZ_Return     bindV(TaggedRef *lPtr, TaggedRef valIn) {
    Assert(0);
    return (FAILED);
  }

  //    
  virtual Bool          validV(TaggedRef) {
    Assert(0);
    return (TRUE);
  }
  virtual OZ_Term       statusV() {
    Assert(0);
    return ((OZ_Term) 0);
  }
  virtual VarStatus     checkStatusV() {
    Assert(0);
    return (EVAR_STATUS_UNKNOWN);
  }
  virtual void          disposeV() {
    Assert(extVar2Var(this)->isEmptySuspList());
    freeListDispose(sizeof(GCStubVar));
  }

  //
  virtual OZ_Return addSuspV(TaggedRef *, Suspendable *susp) {
    Assert(0);
    return (FAILED);
  }
  virtual int getSuspListLengthV() {
    Assert(0);
    return (0);
  }

  //
  // virtual void printStreamV(ostream &out, int depth);
  // virtual void printLongStreamV(ostream &out, int depth, int offset);
  // void print(void);
  // void printLong(void);

  //
  virtual OZ_Return forceBindV(TaggedRef *p, TaggedRef v) {
    Assert(0);
    return (FAILED);
  }
};

//
inline
Bool oz_isGCStubVar(TaggedRef v) {
  return (oz_isExtVar(v) && oz_getExtVar(v)->getIdV() == OZ_EVAR_GCSTUB);
}

//
inline
GCStubVar* oz_getGCStubVar(TaggedRef v) {
  Assert(oz_isGCStubVar(v));
  return ((GCStubVar *) oz_getExtVar(v));
}
inline
GCStubVar* getGCStubVar(TaggedRef *tPtr) {
  return (oz_getGCStubVar(*tPtr));
}

#endif // __VAR_GCSTUB_HH
