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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
  SimpleVar(Board *bb) : GenCVariable(OZ_VAR_SIMPLE,bb) {}

  OZ_Return bind(TaggedRef* vPtr, TaggedRef t, ByteCode* scp);
  OZ_Return unify(TaggedRef* vPtr, TaggedRef t, ByteCode* scp);

  OZ_Return valid(TaggedRef /* val */) { return OK; }
  GenCVariable* gc() { return new SimpleVar(*this); }
  void gcRecurse() {}

  void dispose(void) { freeListDispose(this, sizeof(SimpleVar)); }

  void printStream(ostream &out,int depth = 10) {
    out << "<simple>";
  }
  void printLongStream(ostream &out,int depth = 10,
                        int offset = 0) {
    printStream(out,depth); out << endl;
  }
};


inline
Bool isSimpleVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_SIMPLE);
}

inline
SimpleVar *tagged2SimpleVar(TaggedRef t) {
  Assert(isSimpleVar(t));
  return (SimpleVar *) tagged2CVar(t);
}

#endif /* __SIMPLEVAR__H__ */
