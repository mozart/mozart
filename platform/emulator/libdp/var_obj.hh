/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#ifndef __VAR_OBJ__HH__
#define __VAR_OBJ__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_lazy.hh"

class ObjectVar : public LazyVar {
protected:
  TaggedRef aclass;		// either class or its lazy proxy;

public:
  ObjectVar(Board *bb, OB_TIndex indexIn, GName *gobjIn, OZ_Term cl)
    : LazyVar(bb, indexIn, gobjIn)
  {
    Assert(cl);
    aclass = cl;
    // check whether 'cl' is fine:
    DebugCode(GName *_cl = getGNameClass());
  }

  virtual LazyType getLazyType();
  // 'sendRequest' defines what is to be done for a particular lazy
  // var type:
  virtual void sendRequest();
  // New (extended) format;
  virtual OzVariable * gCollectV() { return new ObjectVar(*this); }
  virtual void gCollectRecurseV(void);

  virtual void disposeV(void);

  Bool isObjectClassAvail(void) {
    OZ_Term cl = oz_deref(aclass);
    if (oz_isConst(cl)) {
      DebugCode(ConstTerm *ct = tagged2Const(cl));
      Assert(ct->getType() == Co_Class);
      return (OK);
    }
    Assert(!oz_isRef(cl));
    if (oz_isVar(cl)) {
      OzVariable *var = tagged2Var(cl);
      Assert(var->getType() == OZ_VAR_EXT);
      ExtVar *evar = (ExtVar *) var;
      Assert(evar->getIdV() == OZ_EVAR_LAZY);
      LazyVar *lvar = (LazyVar *) evar;
      Assert(lvar->getLazyType() == LT_CLASS);
      return (NO);
    }
    Assert(0);
    return (NO);
  }

  GName *getGNameClass() {
    OZ_Term cl = oz_deref(aclass);
    if (oz_isConst(cl)) {
      ConstTerm *ct = tagged2Const(cl);
      Assert(ct->getType() == Co_Class);
      return (((ObjectClass *) ct)->getGName());
    }
    Assert(!oz_isRef(cl));
    if (oz_isVar(cl)) {
      OzVariable *var = tagged2Var(cl);
      Assert(var->getType() == OZ_VAR_EXT);
      ExtVar *evar = (ExtVar *) var;
      Assert(evar->getIdV() == OZ_EVAR_LAZY);
      LazyVar *lvar = (LazyVar *) evar;
      Assert(lvar->getLazyType() == LT_CLASS);
      return (((ObjectVar *) lvar)->getGName());
    }
    Assert(0);
    return ((GName *) 0);
  }

  OZ_Term getClass() { 
    Assert(isObjectClassAvail()); 
    Assert(aclass);
    return (aclass); 
  }

  OZ_Term getClassProxy() { 
    Assert(!isObjectClassAvail()); 
    Assert(aclass);
    return (aclass); 
  }

public:
  virtual void marshal(ByteBuffer *, Bool hasCoRefsIndex);
  //
  void transfer(Object *o, BorrowEntry *be);
};

//
TaggedRef newObjectProxy(OB_TIndex bi, GName *gnobj, TaggedRef clas);

#endif
