/*
 *  Main authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *
 *  Contributing authors:
 *
 *
 *  Copyright:
 *    Andres Barco, 2008
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

#include "var_base.hh"
#include "builtins.hh"
#include "GeSetVar.hh"
#include "distributor.hh"
#include "branch.hh"


/*
  To access the implemented view selection and value section
  strategies shiped with gecode.
 */
#include "gecode/set/branch.hh"


using namespace Gecode::Set::Branch;

// TODO: Remove this comments
//void gfs_dist_init(void) { }

template<>
class VarBasics<SetView, int> {
public:
  static SetView getView(OZ_Term t) {
    Assert(OZ_isGeSetVar(t));
    return SetView(get_SetVarInfo(t).var());
  }
  static bool assigned(OZ_Term t) {
    /**
      TODO: if a set variable gets determined, it is no longer a GeSetVar
      This behaviour is unespected. So if term is SetValueM return true.
    */
    if(SetValueM::OZ_isSetValueM(t))
      return true;
    
    Assert(OZ_isGeSetVar(t));
    return getView(t).assigned();
  }
  
  static OZ_Term getValue(int v) {
    return OZ_int(v);
  }
  
  static int getValue(OZ_Term v) {
    return OZ_intToC(v);
  }
};

/**
   Variable selection strategies for finite sets
   This strategies is taken from the Gecode ones.
   See http://www.gecode.org/gecode-doc-latest/group__TaskModelSetBranch.html
   for more information
*/
enum SetVarSelection {
  setVarByNone,
  setVarByMinCard,
  setVarByMaxCard,
  setVarByMinUnknown,
  setVarByMaxUnknown
};

/**
   Value selection strategies for finite sets
   This strategies is taken from the Gecode ones.
   See http://www.gecode.org/gecode-doc-latest/group__TaskModelSetBranch.html
   for more information
*/
enum SetValSelection {
  setValMin,
  setValMax
};


#define PP(I,J) I*(setVarByMaxUnknown+1)+J  

#define PPCL(I,J)							\
  case PP(setVar ## I, set ## J):					\
  gfsd = new GeVarDistributor<SetView,int,Gecode::Set::Branch::I,	\
			      Gecode::Set::Branch::J >(bb,vars,n);	\
  break;


OZ_BI_define(gfs_distribute, 3, 1) {
  oz_declareIntIN(0,var_sel);
  oz_declareIntIN(1,val_sel);
  oz_declareNonvarIN(2,vv);

  int n = 0;
  TaggedRef * vars;

  if (oz_isLiteral(vv)) {
    ;
  } else if (oz_isLTupleOrRef(vv)) {
    
    TaggedRef vs = vv;
    
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      //TODO: Tests element v to be a GeSetVar.
      //TestElement(v);
      n++;
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr);
      Assert(!oz_isRef(vs));
      if (oz_isVarOrRef(vs))
	oz_suspendOnPtr(vs_ptr);
    }
    
    if (!oz_isNil(vs))
      oz_typeError(0,"vector of finite sets variables");
    
  } else if (oz_isSRecord(vv)) {
    
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      //TODO: Tests element v to be a GeSetVar.
      //TestElement(v);
      n++;
    }
    
  } else 
    oz_typeError(0,"vector of finite sets variables");
  
  // If there are no variables in the input then return unit
  if (n == 0)
    OZ_RETURN(NameUnit);
  
  // This is inverse order!
  vars = (TaggedRef *) oz_freeListMalloc(sizeof(TaggedRef) * n);
  
  // fill in the vars vector 
  Assert(!oz_isRef(vv));
  if (oz_isLTupleOrRef(vv)) {
    TaggedRef vs = vv;
    for (int i =0; i < n; i++) {
      TaggedRef v = oz_head(vs);
      vars[i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  } else {
      int j = 0;
      for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
	TaggedRef v = tagged2SRecord(vv)->getArg(i);
	vars[j++] = v;
      }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);

  Distributor * gfsd;
  
  switch (PP(var_sel,val_sel)) {
    
    PPCL(ByNone,ValMin);
    PPCL(ByNone,ValMax);

    PPCL(ByMinCard,ValMin);
    PPCL(ByMinCard,ValMax);
    
    PPCL(ByMaxCard,ValMin);
    PPCL(ByMaxCard,ValMax);

    PPCL(ByMinUnknown,ValMin);
    PPCL(ByMinUnknown,ValMax);
    
    PPCL(ByMaxUnknown,ValMin);
    PPCL(ByMaxUnknown,ValMax);

  default:
   Assert(false);
  }
  
  bb->setDistributor(gfsd);
  OZ_RETURN(gfsd->getSync());

}
OZ_BI_end


