/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __LAZYVAR__H__
#define __LAZYVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "value.hh"

class GenLazyVariable: public GenCVariable {
private:
  OZ_Term function;
  OZ_Term result;
public:
  GenLazyVariable() : GenCVariable(LazyVariable) {}
  GenLazyVariable(OZ_Term fun,OZ_Term res)
    : GenCVariable(LazyVariable),function(fun),result(res){}
  OZ_Term getFunction() { return function; }
  void kickLazy();

  OZ_Return unifyV(TaggedRef* vPtr,TaggedRef v,TaggedRef *tPtr,TaggedRef t,
                   ByteCode* scp);
  OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef /* val */) { return TRUE; }
  OZ_Return hasFeatureV(TaggedRef, TaggedRef *) { return SUSPEND; }
  GenCVariable* gcV() { return new GenLazyVariable(*this); }
  void gcRecurseV() {
    if (function!=0) {
      OZ_collectHeapTerm(function,function);
      OZ_collectHeapTerm(result,result);
    }
  }
  void addSuspV(Suspension, TaggedRef*, int);
  Bool isKindedV() { return false; }
  void disposeV(void) { freeListDispose(this, sizeof(GenLazyVariable)); }
  int getSuspListLengthV() { return getSuspList()->length()-1; }
  void printV() {}
  void printLongV() {}
};


inline
Bool isLazyVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == LazyVariable);
}

inline
GenLazyVariable *tagged2LazyVar(TaggedRef t) {
  Assert(isLazyVar(t));
  return (GenLazyVariable *) tagged2CVar(t);
}



#endif /* __LAZYVAR__H__ */
