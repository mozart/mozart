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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

class PropFuture: public GenCVariable {
public:
  PropFuture() : GenCVariable(OZ_VAR_FUTURE) {}
  virtual OZ_Return unifyV(TaggedRef* vPtr,TaggedRef t,ByteCode* scp);
  virtual OZ_Return validV(TaggedRef* /* vPtr */, TaggedRef /* val */) {
    return TRUE;
  }
  virtual GenCVariable* gcV() { return new PropFuture(*this); }
  virtual void gcRecurseV()   {}
  virtual void addSuspV(Suspension, TaggedRef*, int);
  virtual void disposeV(void) {
    freeListDispose(this, sizeof(PropFuture));
  }
  virtual void printStreamV(ostream &out,int depth = 10);
  virtual void printLongStreamV(ostream &out,int depth = 10,
				int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
  virtual OZ_Term inspectV();
};

#endif /* __BYNEED__HH__ */
