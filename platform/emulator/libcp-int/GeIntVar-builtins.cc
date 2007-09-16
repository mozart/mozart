/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
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

#include "GeIntVar-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;


/** 
 * \brief Creates a new IntVar variable 
 * 
 * @param 0 The space
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_intvar,1,1)
{
  DECLARE_INT_SET(0,dom);   // the domain of the IntVar
  OZ_RETURN(new_GeIntVar(dom));
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
 * \brief Returns the minimum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The minimum of the domain 
 */
OZ_BI_define(intvar_getMin,1,1)
{
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(v.min());
}
OZ_BI_end

/** 
 * \brief Returns the maximum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The maximum of the domain 
 */
OZ_BI_define(intvar_getMax,1,1)
{
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(v.max());
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
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(v.size());
}
OZ_BI_end

/**
 *\brief Return the oz domain \a OZ_in(0)
 * @param 0 A reference to the variable
 */
/* I have to think in a better way to do this method*/
OZ_BI_define(int_dom,1,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    RAISE_EXCEPTION("The variable must be a GeIntVar");
  IntVar Tmp = get_IntVar(OZ_in(0));
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
 * \brief Return the oz domain of \a OZ_in(0) in a ordered list of integers
 * @param 0 A reference to the variable
 * @param 1 List that represent the oz domain of the first parameter
 */
OZ_BI_define(int_domList,1,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    RAISE_EXCEPTION("The variables must be a GeIntVar");
  IntVar Tmp = get_IntVar(OZ_in(0));
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
 * \brief Return the next integer that \a OZ_in(1) in the GeIntVar \a OZ_in(0)
 * @param 0 A reference to the variable
 * @param 1 integer
 */

OZ_BI_define(int_nextLarger,2,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    RAISE_EXCEPTION("The variables must be a GeIntVar");
  int Val = OZ_intToC(OZ_in(1));
  IntVar Tmp = get_IntVar(OZ_in(0));
  IntVarRanges TmpRange(Tmp);

  for(;TmpRange(); ++TmpRange) {
    if(TmpRange.min() <= Val && TmpRange.max() > Val)
      OZ_RETURN_INT(Val+1);
    if(TmpRange.min() > Val)
      OZ_RETURN_INT(TmpRange.min());
  }
  RAISE_EXCEPTION("The domain does not have a next larger value input");

} 
OZ_BI_end

/**
 * \brief Return the small integer that \a OZ_in(1) in the GeIntVar \a OZ_in(0)
 * @param 0 A reference to the variable
 * @param 1 integer
 */

OZ_BI_define(int_nextSmaller,2,1)
{
  if(!OZ_isGeIntVar(OZ_deref(OZ_in(0))))
    RAISE_EXCEPTION("The variables must be a GeIntVar");
  int Val = OZ_intToC(OZ_in(1));
  IntVar Tmp = get_IntVar(OZ_in(0));
  IntVarRanges TmpRange(Tmp);
  int Min = Gecode::Limits::Int::int_max;
  if(Tmp.min() >= Val)
    RAISE_EXCEPTION("Input value is smaller that domain of input variable");
  for(;TmpRange(); ++TmpRange) {
    if(TmpRange.min() >= Val)
      OZ_RETURN_INT(Min);
    if(TmpRange.min() < Val && TmpRange.max() >= Val)
      OZ_RETURN_INT(Val-1);
    if(TmpRange.max() < Val)
      Min = TmpRange.max();
  }
  
  RAISE_EXCEPTION("Unexpected error please communicate this bug to autors");
} 
OZ_BI_end


/** 
 * \brief Returns the median of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The median
 */
OZ_BI_define(intvar_getMed,1,1)
{
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(v.med());
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
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(v.width());
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
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(IntView(v).regret_min());
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
  DeclareGeIntVar1(0,v);
  OZ_RETURN_INT(IntView(v).regret_max());
}
OZ_BI_end


#endif
