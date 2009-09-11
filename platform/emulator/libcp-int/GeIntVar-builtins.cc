/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
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


#ifndef __GEOZ_INT_VAR_BUILTINS_CC__
#define __GEOZ_INT_VAR_BUILTINS_CC__

#include "IntVarMacros.hh"

using namespace Gecode;
using namespace Gecode::Int;

//*****************************************************************************

#define GEOZ_FD_DESCR_SYNTAX						\
  "The syntax of a " "description of a finite domain" " is:\n"				\
  "   set_descr   ::= simpl_descr | compl(simpl_descr)\n"		\
  "   simpl_descr ::= range_descr | nil | [range_descr+]\n"		\
  "   range_descr ::= integer | integer#integer\n"			\
  "   integer     ::= { Limits::Int::min ,..., Limits::Int::max}"

//*****************************************************************************

/**
 * \brief Binds a new Mozart variable to Gecode constraint variable.
 *  This is used to declare a finite domain variable or assign a domain
 *  to a already created finite domain variable.
 *  @param v is the new Mozart variable
 *  @param dom is the domain of \a v. 
 *  If dom is the maximal allowed domain [Limits::min, Limits::max], 
 *  then this create a new Finite domain variable. Else, narrow the
 *  domain of a variable already created.
 */
OZ_Return tellNewIntVar(OZ_Term v, const IntSet& dom){
  DEREF(v, vptr);

  Assert(!oz_isRef(v));
  if (oz_isFree(v)) {
    GenericSpace *sp = oz_currentBoard()->getGenericSpace();
    IntVar *x = new IntVar(sp,dom);
    GeIntVar *nv = new GeIntVar(sp->getVarsSize());
    OzVariable * ov   = extVar2Var(nv);
    OZ_Term * tcv = newTaggedVar(ov);
    int index        = sp->newVar(static_cast<VarImpBase*>(x->var()), makeTaggedRef(tcv));
        
    if (oz_onToplevel())
      oz_currentBoard()->getGenericSpace()->makeUnstable();
    
    postValReflector<IntView,IntVarImp>(sp,index);
    
    if (oz_isLocalVariable(v)) {
      if (!oz_isOptVar(v)) {
	oz_checkSuspensionListProp(tagged2Var(v));
      }
      bindLocalVar(vptr, tcv);
    } else {
      bindGlobalVar(vptr, tcv);
    }
    
    delete x;
    return PROCEED;

  } else if(OZ_isGeIntVar(v)) {
    GenericSpace *sp = oz_currentBoard()->getGenericSpace();
    IntView view = get_IntView(v);
    try {
      Gecode::dom(sp, view, dom);
    } catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    
    if (oz_onToplevel())
      oz_currentBoard()->getGenericSpace()->makeUnstable();

    return PROCEED;
  } else {
    return PROCEED;
  }
}

OZ_BI_define(BINewIntVar,2,0){
  //1) First case when the domain of the variable is a specific domain
  if(OZ_isIntSet(OZ_in(0))){
    IntSet is = getIntSet(OZ_in(0));
    return tellNewIntVar(OZ_in(1), is);
  }

  //2) Second case when the domain of the variable is a complement of a specific domain
  if(OZ_label(OZ_in(0)) == AtomCompl){
    IntSet is = getIntSet(OZ_getArg(OZ_in(0),0));

    GenericSpace *sp = oz_currentBoard()->getGenericSpace();
    IntVar *x = new IntVar(sp,is);
    ViewRanges<IntView> xvr(*x);
    IntVar *xcompl = new IntVar(sp, Gecode::Int::Limits::min, Gecode::Int::Limits::max);
    IntView xcv(*xcompl);
    xcv.minus_r(sp, xvr);
    ViewRanges<IntView> xcomplvr(xcv);
    IntSet is2(xcomplvr);
    
    delete x, xcompl;
    return tellNewIntVar(OZ_in(1), is2);
  }
  //3) Third case when 1 and 2 fails!
  OZ_typeError(0,GEOZ_FD_DESCR_SYNTAX);

} OZ_BI_end

OZ_BI_define(BIDeclIntVar,1,0){
  
  IntSet is(Int::Limits::min, Int::Limits::max);
  return tellNewIntVar(OZ_in(0), is);

} OZ_BI_end

/** 
 * \brief Creates a new IntVar variable 
 * 
 * @param 0 The space
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_intvar,1,1)
{
  OZ_declareDetTerm(0,arg);
  //DECLARE_INT_SET(arg,dom);
  //IntSet dom = getIntSet(OZ_in(0));
  if (OZ_isGeIntVar(OZ_in(1))) {
    GenericSpace *sp = oz_currentBoard()->getGenericSpace(); 
    IntSet dom = getIntSet(OZ_in(0));
    IntVar right(sp,dom);
    IntView vl = get_IntView(OZ_in(1));
    IntView vr(right);
    ViewRanges<IntView> vvr(vr);
    if (vl.inter_r(sp,vvr)==ME_GEN_FAILED ) {
      return FAILED;
    }
    OZ_RETURN(OZ_in(1));
  } else {
    if(OZ_label(arg) == AtomCompl) {
      //DECLARE_INT_SET(OZ_getArg(arg,0), dom);
      IntSet dom = getIntSet(OZ_getArg(OZ_in(0),0));
      OZ_RETURN(new_GeIntVarCompl(dom));
    }
    //DECLARE_INT_SET(arg,dom);   // the domain of the IntVar
    IntSet dom = getIntSet(OZ_in(0));
    OZ_RETURN(new_GeIntVar(dom));
  }
}
OZ_BI_end

/** 
 * \brief Test whether \a OZ_in(0) is a GeIntVar
 * 
 */
OZ_BI_define(intvar_is,1,1)
{
  OZ_RETURN_BOOL(OZ_isGeIntVar(OZ_in(0)));
}
OZ_BI_end


/** 
 * \brief Returns the Max number that gecode can representate
 * 
 * @param 0 Max integer in c++
 */

OZ_BI_define(int_sup,0,1)
{
  OZ_RETURN_INT(Int::Limits::max);
} 
OZ_BI_end

/** 
 * \brief Returns the Min number that gecode can representate
 * 
 * @param 0 Min integer in c++
 */

OZ_BI_define(int_inf,0,1)
{
  OZ_RETURN_INT(Int::Limits::min);
} 
OZ_BI_end


/** 
 * \brief Returns the minimum element in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The minimum of the domain 
 */
OZ_BI_define(intvar_getMin,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.min());
}
OZ_BI_end

/** 
 * \brief Returns the maximum element in the domain
 * 
 * @param intvar_getMax 
 * @param 0 A reference to the variable 
 * @param 1 The maximum of the domain 
 */
OZ_BI_define(intvar_getMax,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.max());
}
OZ_BI_end

/** 
 * \brief Returns the size of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The domain size
 */
OZ_BI_define(intvar_getSize,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.size());
}
OZ_BI_end

/**
 *\brief Returns the oz domain \a OZ_in(0)
 * @param 0 A reference to the variable
 */
/* I have to think in a better way to do this method*/
OZ_BI_define(int_dom,1,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    OZ_typeError(0,"IntVar");
  IntView Tmp = get_IntView(OZ_in(0));
  
  IntVarRanges TmpRange(Tmp);
  int TotalRangs = 0;
  
  for(;TmpRange();++TmpRange,TotalRangs++);
  
  OZ_Term TmpArray[TotalRangs];
  IntVarRanges TmpRange1(Tmp);

  for(int i=0;TmpRange1();++TmpRange1,i++)
    TmpArray[i] = OZ_mkTupleC("#",2,OZ_int(TmpRange1.min()),OZ_int(TmpRange1.max()));

  if(TotalRangs==1) OZ_RETURN(TmpArray[0]);
  OZ_Term DomList = OZ_toList(TotalRangs,TmpArray);
  
  OZ_RETURN(DomList);
}
OZ_BI_end


/**
 * \brief Returns the oz domain of \a OZ_in(0) in a ordered list of integers
 * @param 0 A reference to the variable
 * @param 1 List that represent the oz domain of the first parameter
 */
OZ_BI_define(int_domList,1,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    OZ_typeError(0,"IntVar");
  IntView Tmp = get_IntView(OZ_in(0));
  IntVarRanges TmpRange(Tmp);
  OZ_Term TmpArray[Tmp.size()];
  int i = 0;
  for(;TmpRange();++TmpRange)
    for(int j=TmpRange.min();j<=TmpRange.max();j++,i++)
      TmpArray[i] = OZ_int(j);
  OZ_Term DomList = OZ_toList(Tmp.size(),TmpArray);
  OZ_RETURN(DomList);
}
OZ_BI_end

/**
 * \brief Returns the next integer that \a OZ_in(1) in the GeIntVar \a OZ_in(0)
 * @param 0 A reference to the variable
 * @param 1 integer
 */

OZ_BI_define(int_nextLarger,2,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    OZ_typeError(0,"IntVar");
  int Val = OZ_intToC(OZ_in(1));
  IntView Tmp = get_IntView(OZ_in(0));
  IntVarRanges TmpRange(Tmp);
  
  for(;TmpRange(); ++TmpRange) {
    if(TmpRange.min() <= Val && TmpRange.max() > Val)
      OZ_RETURN_INT(Val+1);
    if(TmpRange.min() > Val)
      OZ_RETURN_INT(TmpRange.min());
  }
  return OZ_typeError(0,"The domain does not have a next larger value input");

} 
OZ_BI_end

/**
 * \brief Returns the small integer that \a OZ_in(1) in the GeIntVar \a OZ_in(0)
 * @param 0 A reference to the variable
 * @param 1 integer
 */

OZ_BI_define(int_nextSmaller,2,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    OZ_typeError(0,"IntVar");
  int Val = OZ_intToC(OZ_in(1));
  IntView Tmp = get_IntView(OZ_in(0));
  IntVarRanges TmpRange(Tmp);
  int Min = Gecode::Int::Limits::max;
  if(Tmp.min() >= Val){
    return OZ_typeError(0,"Input value is smaller that domain of input variable");
  }
  
  for(;TmpRange(); ++TmpRange) {
    if(TmpRange.min() >= Val)
      OZ_RETURN_INT(Min);
    if(TmpRange.min() < Val && TmpRange.max() >= Val)
      OZ_RETURN_INT(Val-1);
    if(TmpRange.max() < Val)
      Min = TmpRange.max();
  }
  Assert(false);
} 
OZ_BI_end


/** 
 * \brief Returns the median of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The median
 */
OZ_BI_define(intvar_getMid,1,1)
{
  OZ_getINDeref(0, var, varptr);
  
  if (oz_isSmallInt(var)) {
    OZ_RETURN(var);
  } else if (OZ_isGeIntVar(var)) {
    IntView view = get_IntView(OZ_in(0));
    int mid = (view.min() + view.max()) / 2;
    IntVarValues values(view);
    
    if (view.in(mid)) {
      OZ_RETURN_INT(mid);
    }
    
    while (values()) {
      int cur = values.val();
      int left;
      
      if (cur < mid) {
	left = cur;
	++values;
      } else {
	OZ_RETURN_INT(((cur - mid) >= (mid - left)) ? left : cur);
      }
    }
  } else if (oz_isNonKinded(var)) {
    oz_suspendOnPtr(varptr);
  } else {
    return OZ_typeError(0, "finite domain or integer value");
  }
}
OZ_BI_end


/** 
 * \brief Returns the width of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The width
 */
OZ_BI_define(intvar_getWidth,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.width());
}
OZ_BI_end

/** 
 * \brief Returns the Regret Min of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The Regret Min
 */
OZ_BI_define(intvar_getRegretMin,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.regret_min());
}
OZ_BI_end

/** 
 * \brief Returns the Regret Max of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The Regret Max
 */
OZ_BI_define(intvar_getRegretMax,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  OZ_RETURN_INT(view.regret_max());
}
OZ_BI_end

/** 
 * \brief Returns the number of propagators associated with this variable
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) Number of associated propagators
 */
OZ_BI_define(intvar_propSusp,1,1)
{
  IntView view = intOrIntView(OZ_in(0));
  GeVarBase *gv = get_GeVar(OZ_in(0));
  OZ_RETURN_INT(view.degree()-gv->varprops());
}
OZ_BI_end

/**
 * \brief the same that FD.watch.min in mozart
 * @param 0 A reference to the variable
 * @param 1 A reference to the variable (BoolVar)
 * @param 2 Integer
 */

#endif
