/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
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

#ifndef __EXTENSIONHH
#define __EXTENSIONHH

#include "value.hh"

/*===================================================================
 * Extension
 *=================================================================== */

int oz_newUniqueId();

class SituatedExtension: public ConstTermWithHome {
public:
  SituatedExtension();
  virtual ~SituatedExtension() {}
  SituatedExtension(Board *bb) : ConstTermWithHome(bb,Co_SituatedExtension) {}
  virtual int getTypeV() { return 0; }
  virtual SituatedExtension *gcV() = 0;
  virtual void          gcRecurseV() = 0;
  virtual void          printStreamV(ostream &out,int depth = 10);
  virtual void          printLongStreamV(ostream &out,int depth = 10,
					 int offset = 0);
  virtual OZ_Term       inspectV();
  virtual OZ_Term       getFeatureV(OZ_Term fea) { return 0; }
};

inline
int oz_isSituatedExtension(OZ_Term t) {
  return oz_isConst(t) && tagged2Const(t)->getType()==Co_SituatedExtension;
}

inline
SituatedExtension *tagged2SituatedExtension(OZ_Term t) {
  return (SituatedExtension *) tagged2Const(t);
}

class ConstExtension: public ConstTerm {
public:
  virtual ~ConstExtension() {}
  ConstExtension() : ConstTerm(Co_ConstExtension) {}
  virtual int getTypeV() { return 0; }
  virtual ConstExtension *gcV() = 0;
  virtual void          gcRecurseV() = 0;
  virtual void          printStreamV(ostream &out,int depth = 10);
  virtual void          printLongStreamV(ostream &out,int depth = 10,
					 int offset = 0);
  virtual OZ_Term       inspectV();
  virtual OZ_Term       getFeatureV(OZ_Term fea) { return 0; }
};

inline
int oz_isConstExtension(OZ_Term t) {
  return oz_isConst(t) && tagged2Const(t)->getType()==Co_ConstExtension;
}

inline
ConstExtension *tagged2ConstExtension(OZ_Term t) {
  return (ConstExtension *) tagged2Const(t);
}

#endif
