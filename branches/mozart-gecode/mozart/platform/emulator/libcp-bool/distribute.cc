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
#include "GeBoolVar.hh"
#include "distributor.hh"
#include "branch.hh"


/*
  To access the implemented view selection and value section
  strategies shiped with gecode.
 */
#include "gecode/int/branch.hh"


using namespace Gecode::Int::Branch;

template<>
class VarBasics<BoolView, int> {
public:
  static BoolView getView(OZ_Term t) {
    Assert(OZ_isGeBoolVar(t));
    BoolVar *bv = get_BoolVarInfoPtr(t);
    BoolView vi(*bv);
    delete bv;
    return vi;
  }
  static bool assigned(OZ_Term t) {
    if (OZ_isInt(t)) {
      return true;
    }
    Assert(OZ_isGeBoolVar(t));
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
   Variable selection strategies for boolean domains
   This strategies is taken from the Gecode ones.
   See http://www.gecode.org/gecode-doc-latest/group__TaskModelIntBranch.html
   for more information
*/
enum BoolVarSelection {
  bVarByNone,
  bVarByDegreeMinNoTies,
  bVarByDegreeMaxNoTies
};

/**
   Value selection strategies for boolean domains
   This strategies is taken from the Gecode ones.
   See http://www.gecode.org/gecode-doc-latest/group__TaskModelIntBranch.html
   for more information
*/
enum BoolValSelection {
  bValMin,
  bValMax
};


/**
   This Macro test whether element v is an 
   undetermined GeBoolVar or a determined one.
   If v is varOrRef then suspend, otherwise raise error.
*/
#define TestGeBoolVar(v)						\
  {									\
    DEREF(v, v_ptr);							\
    Assert(!oz_isRef(v));						\
    if (OZ_isGeBoolVar(v)) {						\
      n++;								\
    } else if (OZ_isInt(v)) {						\
      ;									\
    } else if (oz_isVarOrRef(v)) {					\
      oz_suspendOnPtr(v_ptr);						\
    } else {								\
      oz_typeError(0,"vector of booleans domains");			\
    }									\
  }



#define PP(I,J) I*(bVarByDegreeMaxNoTies+1)+J  

#define PPCL(I,J)					\
  case PP(bVar ## I, b ## J):				\
  gbdd = new GeVarDistributor<BoolView,int,I<BoolView>,		     \
			      J<BoolView> >(bb,vars,n); \
  break;

OZ_BI_define(gbd_distribute, 3, 1) {
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
      TestGeBoolVar(v);
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr);
      Assert(!oz_isRef(vs));
      if (oz_isVarOrRef(vs))
	oz_suspendOnPtr(vs_ptr);
    }
    
    if (!oz_isNil(vs))
      oz_typeError(0,"vector of boolean domains");
    
  } else if (oz_isSRecord(vv)) {
    
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      TestGeBoolVar(v);
    }
    
  } else 
    oz_typeError(0,"vector of boolean domains");
  
  

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

  Distributor * gbdd;
  
  switch (PP(var_sel,val_sel)) {
    
    PPCL(ByNone,ValMin);
    PPCL(ByNone,ValMax);
        
    PPCL(ByDegreeMaxNoTies,ValMin);
    PPCL(ByDegreeMaxNoTies,ValMax);
        
    PPCL(ByDegreeMinNoTies,ValMin);
    PPCL(ByDegreeMinNoTies,ValMax);
    
  default:
    Assert(false);
  }
  
  bb->setDistributor(gbdd);
  OZ_RETURN(gbdd->getSync());

}
OZ_BI_end
