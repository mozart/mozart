/*
 *  Authors:
 *    Kostja Popov <kost@sics.se>
 *
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov, 2001
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

#ifndef __MARSHALERPATCH_H
#define __MARSHALERPATCH_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "var_ext.hh"

//
// The 'OzValuePatch' abstraction.
//
// The "virtual snapshot" business (see libdp/dpMarshaler.hh) requires
// this abstraction: exported/marshaled variables are represented this
// way. Theoretically, it could be also used for masquerading cycles
// and co-references in front of the second phase marshaling
// (i.e. real marshaling, as the first phase figures out resources
// (pickling) or exports variables (distribution)). However, this is
// progressively inefficient for suspendable marshaling as the number
// co-reference increases.
//
// OzValuePatch serves as a base class for actual patches. Observe
// that installation/deinstallation of patches does not depend on
// their particular types (but marshaling thereof of course does);
//

//
#if defined(DEBUG_CHECK)
typedef enum {
  OVP_uninstalled,
  OVP_installed,
  OVP_disposed
} OVPStatus;
#endif

//
class OzValuePatch : public ExtVar {
  friend void installOVP(OzValuePatch *ovp);
  friend void deinstallOVP(OzValuePatch *ovp);
  friend OZ_Term gcStartOVP(OzValuePatch *ovp);
  friend OzValuePatch* gcFinishOVP(OZ_Term first);

  //
private:
  OZ_Term loc;                  // where to patch;
  OZ_Term val;                  // actual (patched) value - when installed;
  // OzValuePatch"es are either linked with naked pointers (for speed
  // during installation/deinstallation), or they constitute one single
  // compound OZ_Term (for GC). A gc link is either a tagged
  // (OzValuePatch) variable - if the patch wasn't installed by
  // 'gcStart()', or a ref to a patched location - if the patch was
  // installed by 'gcStart()'.
  union {
    OzValuePatch *ovp;
    OZ_Term gc;
  } next;                       // .. OzValuePatch;
  DebugCode(OVPStatus status;);

  //
public:
  OzValuePatch(OZ_Term locIn, OzValuePatch *nIn)
    : ExtVar(oz_currentBoard()), loc(locIn) {
    next.ovp = nIn;
    Assert(oz_isRef((OZ_Term) locIn));
    DebugCode(val = (OZ_Term) 0;);
    DebugCode(status = OVP_uninstalled;);
  }
  virtual ~OzValuePatch() { Assert(0); }

  //
  void disposeOVP() {
    Assert(status == OVP_uninstalled);
    Assert(val == (OZ_Term) 0);
    Assert(next.ovp == (OzValuePatch *) 0); // must be taken care of already;
    DebugCode(loc = val = (OZ_Term) -1;);
    DebugCode(status = OVP_disposed;);
  }
  // gcRecurse"ing OzValuePatch"es include at least:
  void gcRecurseOVP() {
    // At the location we have either an ordinary variable (which can
    // be garbage otherwise, BTW), or an OzValuePatch [variable] (not
    // necessarily this one!);
    DebugCode(OZ_Term v = *(tagged2Ref(loc)););
    // (should also check whether the marked value is a var;)
    Assert(oz_isMark(v) || (!oz_isRef(v) && oz_isVarOrRef(v)));
    oz_gCollectTerm(loc, loc);
    if (val)      oz_gCollectTerm(val, val);
    if (next.gc)  oz_gCollectTerm(next.gc, next.gc);
  }

  //
  // ExtVar's heritage, mostly of no use here;
  //
  // each patch type has its own size:
  virtual void          disposeV() = 0;
  // each patch type has its own type:
  virtual ExtVarType    getIdV() = 0;
  // also type-specific;
  virtual ExtVar*       gCollectV() = 0;
  // (usually just 'gcRecurseOVP()');
  virtual void          gCollectRecurseV() = 0;
  virtual ExtVar*       sCloneV() { Assert(0); return ((ExtVar *) 0); }
  virtual void          sCloneRecurseV() { Assert(0); }

  //
  virtual OZ_Return     unifyV(TaggedRef *lPtr, TaggedRef *rPtr) {
    Assert(0); return (FAILED);
  }
  virtual OZ_Return     bindV(TaggedRef *lPtr, TaggedRef valIn) {
    Assert(0); return (FAILED);
  }
  virtual Bool          validV(TaggedRef) { Assert(0); return (TRUE); }
  virtual OZ_Term       statusV() { Assert(0); return ((OZ_Term) 0); }
  virtual VarStatus     checkStatusV() {
    Assert(0); return (EVAR_STATUS_UNKNOWN);
  }
  virtual OZ_Return addSuspV(TaggedRef *, Suspendable *susp) {
    Assert(0); return (FAILED);
  }
  virtual int getSuspListLengthV() {
    Assert(0); return (0);
  }
  virtual OZ_Return forceBindV(TaggedRef *p, TaggedRef v) {
    Assert(0); return (FAILED);
  }

  //
  // virtual void printStreamV(ostream &out, int depth);
  // virtual void printLongStreamV(ostream &out, int depth, int offset);
  // void print(void);
  // void printLong(void);

  //
  OZ_Term getPatchedValue() {
    Assert(status == OVP_installed);
    Assert(val != (OZ_Term) 0);
    return (val);
  }
  OzValuePatch *getNext() { return (next.ovp); }

  //
#if defined(DEBUG_CHECK)
  void dropNext() { next.ovp = (OzValuePatch *) 0; }
#endif
};

//
// non-recursive;
inline
void installOVP(OzValuePatch *ovp)
{
  do {
    Assert(ovp->status == OVP_uninstalled);
    Assert(ovp->val == (OZ_Term) 0);
    OZ_Term *vp = tagged2Ref(ovp->loc);
    ovp->val = *vp;             // saving the patched value;
    *vp = oz_makeExtVar(ovp);   // patching;
    DebugCode(ovp->status = OVP_installed;);
    ovp = ovp->next.ovp;
  } while(ovp);
}

//
inline
void deinstallOVP(OzValuePatch *ovp)
{
  do {
    Assert(ovp->status == OVP_installed);
    Assert(ovp->val != (OZ_Term) 0);
    OZ_Term *vp = tagged2Ref(ovp->loc);
    // *usually* reverting the location back. BUT it does not have
    // to: the patch may update that location to whatever it sees
    // fit;
    *vp = ovp->val;
    // must be reset for 'gcStartOVP()'/'gcFinishOVP()':
    ovp->val = (OZ_Term) 0;
    DebugCode(ovp->status = OVP_uninstalled;);
    ovp = ovp->next.ovp;
  } while(ovp);
}

//
// 'start'/'finish' procedures make sure that the locations and the
// patches themselves are kept. 'gcStart()' returns the Oz value GCing
// of the patch should start with, and 'gcFinish()' performs a reverse
// conversion;
OZ_Term gcStartOVP(OzValuePatch *ovp);
OzValuePatch* gcFinishOVP(OZ_Term first);

//
inline
void deleteOzValuePatch(OzValuePatch *ovp)
{
  while (ovp) {
    OzValuePatch *novp = ovp->getNext();
    DebugCode(ovp->dropNext(););
    ovp->disposeV();
    ovp = novp;
  }
}

/*
//
// A stub that keeps the location of an Oz value during GC
// It should be only visible to the GC.
//
class GCStubVar : public ExtVar {
private:
  OZ_Term val;                  // that has been overwritten by;

  //
public:
  GCStubVar(OZ_Term valIn) : ExtVar(oz_rootBoard()), val(valIn) {}
  virtual ~GCStubVar() { Assert(0); }

  //
  OZ_Term getValue() { return (val); }

  //
  virtual ExtVarType    getIdV() { return (OZ_EVAR_GCSTUB); }
  // It does not need to be saved itself: will become garbage at the
  // end of GC!
  virtual OzVariable*   gCollectV() { return (this); }
  virtual void          gCollectRecurseV() { oz_gCollectTerm(val, val); }
  virtual OzVariable*   sCloneV() { Assert(0); return ((GCStubVar *) 0); }
  virtual void          sCloneRecurseV() { Assert(0); }

  //
  virtual OZ_Return     unifyV(TaggedRef *lPtr, TaggedRef *rPtr) {
    Assert(0); return (FAILED);
  }
  virtual OZ_Return     bindV(TaggedRef *lPtr, TaggedRef valIn) {
    Assert(0); return (FAILED);
  }

  //
  virtual Bool          validV(TaggedRef) {
    Assert(0); return (TRUE);
  }
  virtual OZ_Term       statusV() {
    Assert(0); return ((OZ_Term) 0);
  }
  virtual VarStatus     checkStatusV() {
    Assert(0); return (EVAR_STATUS_UNKNOWN);
  }
  virtual void          disposeV() {
    Assert(isEmptySuspList());
    oz_freeListDispose(this, sizeof(GCStubVar));
  }

  //
  virtual OZ_Return addSuspV(TaggedRef *, Suspendable *susp) {
    Assert(0); return (FAILED);
  }
  virtual int getSuspListLengthV() {
    Assert(0); return (0);
  }

  //
  // virtual void printStreamV(ostream &out, int depth);
  // virtual void printLongStreamV(ostream &out, int depth, int offset);
  // void print(void);
  // void printLong(void);

  //
  virtual OZ_Return forceBindV(TaggedRef *p, TaggedRef v) {
    Assert(0); return (FAILED);
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
*/

#endif
