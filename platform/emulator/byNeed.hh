/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
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

#ifndef __BYNEED__HH__
#define __BYNEED__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "value.hh"

class ByNeedVariable: public GenCVariable {
private:
  OZ_Term function;
public:
  ByNeedVariable(); // mm2: fake compiler
  ByNeedVariable(OZ_Term fun) : GenCVariable(OZ_VAR_BYNEED),function(fun){}
  OZ_Term getFunction() { return function; }
  void kick(TaggedRef *);

  virtual OZ_Return unifyV(TaggedRef* vPtr,TaggedRef t,ByteCode* scp);
  virtual OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef /* val */) {
    return TRUE;
  }
  virtual GenCVariable* gcV() { return new ByNeedVariable(*this); }
  virtual void gcRecurseV() {
    if (function!=0) {
      OZ_collectHeapTerm(function,function);
    }
  }
  virtual void addSuspV(Suspension, TaggedRef*, int);
  virtual void disposeV(void) {
    freeListDispose(this, sizeof(ByNeedVariable));
  }
  virtual void printStreamV(ostream &out,int depth = 10);
  virtual void printLongStreamV(ostream &out,int depth = 10,
                                int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
  virtual OZ_Term inspectV();
};


inline
Bool isByNeedVariable(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_BYNEED);
}

inline
ByNeedVariable *tagged2ByNeedVariable(TaggedRef t) {
  Assert(isByNeedVariable(t));
  return (ByNeedVariable *) tagged2CVar(t);
}

#endif /* __BYNEED__HH__ */
