/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997,1998)
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

#ifndef __VAR_LAZY__HH__
#define __VAR_LAZY__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "dpBase.hh"
#include "var_ext.hh"
#include "var.hh"

class LazyVar : public ExtVar {
protected:
  OB_TIndex index;		// borrow index;
  short requested;		// flag - whether in transition;
  GName *gname;			// how it is known;
  EntityInfo* info;		// failure;

public:
  LazyVar(Board *bb, OB_TIndex indexIn, GName *gIn)
    : index(indexIn), gname(gIn), ExtVar(bb), requested(0), info(0)
  {}

  //
  EntityInfo* getInfo(){return info;}
  void setInfo(EntityInfo* ei){info=ei;}
  OB_TIndex getIndex(){ return (index); }
  void setIndex(OB_TIndex indexIn) { index = indexIn; }

  //
  virtual ExtVarType getIdV() { return (OZ_EVAR_LAZY); }
  virtual OZ_Term statusV();
  virtual VarStatus checkStatusV();
  virtual OZ_Return addSuspV(TaggedRef *v, Suspendable * susp);
  virtual LazyType getLazyType() = 0;
  virtual void sendRequest() = 0;
  virtual Bool validV(TaggedRef v) { return (TRUE); }
  virtual ExtVar* gCollectV() { Assert(0); return NULL; }
  virtual ExtVar* sCloneV() { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void);
  virtual void sCloneRecurseV(void) { Assert(0); }
  virtual void printStreamV(ostream &out,int depth = 10) {
    out << "<dist:lazy>";
  }
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  virtual OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
  virtual void disposeV(void);

  //
public:
  virtual void marshal(ByteBuffer *bs, Bool hasIndex);

  //
  GName *getGName() { return (gname); }

  // failure
  void probeFault(int);
  void addEntityCond(EntityCond);
  void subEntityCond(EntityCond);
  Bool errorIgnore();
  void wakeAll();
  Bool failurePreemption();
  void newWatcher(Bool);

  //
  TaggedRef getTaggedRef();
};

//
inline
Bool oz_isLazyVar(TaggedRef v) {
  return (oz_isExtVar(v) && oz_getExtVar(v)->getIdV() == OZ_EVAR_LAZY);
}

//
inline
LazyVar* oz_getLazyVar(TaggedRef v) {
  Assert(oz_isLazyVar(v));
  return ((LazyVar *) oz_getExtVar(v));
}
inline
LazyVar* getLazyVar(TaggedRef *tPtr) {
  Assert(classifyVar(tPtr) == VAR_LAZY);
  return (oz_getLazyVar(*tPtr));
}

#endif
