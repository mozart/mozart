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

// TODO: Remove the call to this function from GeIntVar
void gfs_dist_init(void) { }

template<>
class VarBasics<SetView, int> {
public:
  static SetView getView(OZ_Term t, bool affect_stability = false) {
    Assert(OZ_isGeSetVar(t));
    if (!affect_stability)
      return SetView(get_SetVarInfo(t).var());
    return SetView(get_SetVar(t).var());
  }
  static bool assigned(OZ_Term t) {
    //Assert(OZ_isGeSetVar(t));
    if(!OZ_isGeSetVar(t)){
      //printf("no es gesetvar...\n");fflush(stdout);
      return true;
    }
    return getView(t).assigned();
  }
  
  static OZ_Term getValue(int v) {
    return OZ_int(v);
  }
  
  static int getValue(OZ_Term v) {
    return OZ_intToC(v);
  }
};

//var selection strategies
enum SetVarSelection {
  setVarByNone,
  setVarByMinCard,
  setVarByMaxCard,
  setVarByMinUnknown,
  setVarByMaxUnknown
};

//val selection strategies
enum SetValSelection {
  setValMin,
  setValMax
};

/*#define iVarByNone        0
#define iVarByMinCard     1
#define iVarByMaxCard     2
#define iVarByMinUnknown  3
#define iVarByMaxUnknown  4

#define iValMin           0
#define iValMax           1
*/

#define PP(I,J) I*(setVarByMaxUnknown+1)+J  

#define PPCL(I,J)					\
  case PP(setVar ## I, set ## J):				\
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
      //TestElement(v);
      n++;
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr);
      Assert(!oz_isRef(vs));
      if (oz_isVarOrRef(vs))
	oz_suspendOnPtr(vs_ptr);
    }
    
    if (!oz_isNil(vs))
      oz_typeError(0,"vector of finite domains");
    
  } else if (oz_isSRecord(vv)) {
    
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      //TestElement(v);
      n++;
    }
    
  } else 
    oz_typeError(0,"vector of finite sets");
  
  

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
    
    /*PPCL(BySizeMin,ValMin);
    PPCL(BySizeMin,ValMax);
    PPCL(BySizeMin,ValMed);
    PPCL(BySizeMin,ValSplitMin);
    PPCL(BySizeMin,ValSplitMax);
    
    PPCL(ByMinMin,ValMin);
    PPCL(ByMinMin,ValMax);
    PPCL(ByMinMin,ValMed);
    PPCL(ByMinMin,ValSplitMin);
    PPCL(ByMinMin,ValSplitMax);
    
    PPCL(ByMaxMax,ValMin);
    PPCL(ByMaxMax,ValMax);
    PPCL(ByMaxMax,ValMed);
    PPCL(ByMaxMax,ValSplitMin);
    PPCL(ByMaxMax,ValSplitMax);
    
    /*PPCL(Width,Min);
    PPCL(Width,Max);
    PPCL(Width,Mid);
    PPCL(Width,SplitMin);
    PPCL(Width,SplitMax);
    
    PPCL(NbProp,Min);
    PPCL(NbProp,Max);
    PPCL(NbProp,Mid);
    PPCL(NbProp,SplitMin);
    PPCL(NbProp,SplitMax);
    */
  default:
   Assert(false);
  }
  
  bb->setDistributor(gfsd);
  OZ_RETURN(gfsd->getSync());

}
OZ_BI_end


