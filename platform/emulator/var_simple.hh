/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
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

#ifndef __SIMPLEVAR__H__
#define __SIMPLEVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"

class SimpleVar: public GenCVariable {
private:
  // OZ_Term future;
public:
  SimpleVar(Board *bb) : GenCVariable(SimpleVarType,bb) {}

  OZ_Return unifyV(TaggedRef* vPtr, TaggedRef t, ByteCode* scp);

  Bool isKindedV() { return NO; }
  OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef /* val */) { return OK; }
  OZ_Return hasFeatureV(TaggedRef, TaggedRef *) { return SUSPEND; }

  GenCVariable* gcV() { return new SimpleVar(*this); }
  void gcRecurseV() {}

  void addSuspV(Suspension susp, TaggedRef*, int unstable)
  {
    addSuspSVar(susp, unstable);
  }
  void disposeV(void) { freeListDispose(this, sizeof(SimpleVar)); }
  int getSuspListLengthV() { return getSuspListLengthS(); }

  void printStreamV(ostream &out,int depth = 10) {
    out << "<simple>";
  }
  void printLongStreamV(ostream &out,int depth = 10,
                        int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
};


inline
Bool isSimpleVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == SimpleVarType);
}

inline
SimpleVar *tagged2SimpleVar(TaggedRef t) {
  Assert(isSimpleVar(t));
  return (SimpleVar *) tagged2CVar(t);
}



#endif /* __SIMPLEVAR__H__ */
