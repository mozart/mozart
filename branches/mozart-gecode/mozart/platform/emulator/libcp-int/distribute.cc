/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Victor Alfonso Rivera <varivera@puj.edu.co>
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>                                                                                                       
 *
 *  Copyright:
 *    Raphael Collet, 2008
 *    Gustavo Gutierrez, 2008
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
#include "GeIntVar.hh"
#include "distributor.hh"
#include "branch.hh"


/*
  To access the implemented view selection and value section
  strategies shiped with gecode.
 */
#include "gecode/int/branch.hh"


using namespace Gecode::Int::Branch;

// TODO: Remove the call to this function from GeIntVar
void gfd_dist_init(void) { }

template<>
class VarBasics<IntView, int> {
public:
  static IntView getView(OZ_Term t) {
    Assert(OZ_isGeIntVar(t));
    return IntView(get_IntVarInfo(t).var());
  }
  static bool assigned(OZ_Term t) {
    if (OZ_isInt(t)) {
      return true;
    }
    Assert(OZ_isGeIntVar(t));
    return getView(t).assigned();
  }
  static OZ_Term getValue(int v) {
    return OZ_int(v);
  }
  static int getValue(OZ_Term v) {
    return OZ_intToC(v);
  }
};


#define iVarByNone       0
#define iVarBySizeMin    1
#define iVarByMinMin     2
#define iVarByMaxMax     3
#define iVarByVarNbProp  4
#define iVarByVarWidth   5

#define iValMin          0
#define iValMed          1
#define iValMax          2
#define iValSplitMin     3
#define iValSplitMax     4

#define PP(I,J) I*(iVarByVarWidth+1)+J  

#define PPCL(I,J)					\
  case PP(iVar ## I, i ## J):				\
  gfdd = new GeVarDistributor<IntView,int,I<IntView>,	\
			    J<IntView> >(bb,vars,n);	\
  break;

OZ_BI_define(gfd_distribute, 3, 1) {
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
    oz_typeError(0,"vector of finite domains");
  
  

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
    /*    int i = n;
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      vars[i] = v;
      vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
      }*/
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

  Distributor * gfdd;
  
  switch (PP(var_sel,val_sel)) {
    
    PPCL(ByNone,ValMin);
    PPCL(ByNone,ValMax);
    PPCL(ByNone,ValMed);
    PPCL(ByNone,ValSplitMin);
    PPCL(ByNone,ValSplitMax);
    
    PPCL(BySizeMin,ValMin);
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
  
  bb->setDistributor(gfdd);
  OZ_RETURN(gfdd->getSync());

}
OZ_BI_end

  
OZ_BI_define(gfd_assign, 1, 1) {
  oz_declareNonvarIN(0,vv);

  int n = 0;
  TaggedRef * vars;

  // Assume vv is a tuple (list) of gfd variables
  Assert(oz_isTuple(vv));
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
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  GeVarAssignment<IntView,int,ByNone<IntView>,ValMin<IntView> > * gfda =
    new GeVarAssignment<IntView,int,ByNone<IntView>,ValMin<IntView> >(bb,vars,n);

   bb->setDistributor(gfda);
  
  OZ_RETURN(gfda->getSync()); 
}
OZ_BI_end
