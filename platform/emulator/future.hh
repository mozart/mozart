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
  Future(OZ_Term function=0)
    : GenCVariable(OZ_VAR_FUTURE), function(function) {}
  virtual OZ_Return unifyV(TaggedRef* vPtr,TaggedRef t,ByteCode* scp);
  virtual OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef /* val */) {
    return TRUE;
  }
  virtual GenCVariable* gcV() { return new Future(*this); }
  virtual void gcRecurseV()   {
    if (function) {
      OZ_collectHeapTerm(function,function);
    }
  }
  virtual void addSuspV(Suspension, TaggedRef*, int);
  virtual void disposeV(void) {
    freeListDispose(this, sizeof(Future));
  }
  virtual void printStreamV(ostream &out,int depth = 10);
  virtual void printLongStreamV(ostream &out,int depth = 10,
				int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
  virtual OZ_Term inspectV();
};

#endif /* __BYNEED__HH__ */
