/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __FUTURE__HH__
#define __FUTURE__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "value.hh"

class Future: public GenCVariable {
private:
  OZ_Term function;

  void kick(TaggedRef *);
public:
  Future(Board *bb) : GenCVariable(OZ_VAR_FUTURE,bb), function(0) {}
  Future(OZ_Term function,Board *bb)
    : GenCVariable(OZ_VAR_FUTURE,bb), function(function) {}
  OZ_Return unify(TaggedRef* vPtr,TaggedRef t,ByteCode* scp);
  OZ_Return valid(TaggedRef /* val */) {
    return TRUE;
  }
  GenCVariable* gc() { return new Future(*this); }
  void gcRecurse()   {
    if (function) {
      OZ_collectHeapTerm(function,function);
    }
  }
  void addSusp(TaggedRef*, Suspension, int);
  void dispose(void) {
    freeListDispose(this, sizeof(Future));
  }
  void printStream(ostream &out,int depth = 10);
  void printLongStream(ostream &out,int depth = 10,
                                int offset = 0) {
    printStream(out,depth); out << endl;
  }
  OZ_Term inspect();
};

#endif /* __BYNEED__HH__ */
