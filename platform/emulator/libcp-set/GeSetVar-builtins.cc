/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
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


#ifndef __GEOZ_SET_VAR_BUILTINS_CC__
#define __GEOZ_SET_VAR_BUILTINS_CC__

#include "SetVarMacros.hh"

using namespace Gecode;
using namespace Gecode::Set;

/** 
 * \brief Creates a new SetVar variable with bounds
 * 
 * @param 0 Domain description
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_bounds,2,1)
{
  DECLARE_INT_SET3(dom1, val1, 0);   // the glb of the SetVar
  DECLARE_INT_SET3(dom2, val2, 1);   // the lub of the SetVar
  OZ_RETURN(new_GeSetVar(dom1,dom2));
}
OZ_BI_end


/** 
 * \brief Creates a new SetVar variable with the complement of an old one
 * 
 * @param 0 The old variable
 * @param 1 The complement of the old variable as a new one
 */
OZ_BI_define(new_comp,1,1)
{
  OZ_RETURN(new_GeSetVarComp(OZ_in(0)));
}
OZ_BI_end

/** 
 * \brief Creates a new SetVar variable with the difference of two variables
 * 
 * @param 0 The old variable number 1,  V1
 * @param 1 The old variable number 2,  V2
 * @param 2 Variable = V1\V2
 */
OZ_BI_define(new_complin,2,1)
{  
  OZ_RETURN(new_GeSetVarComplIn(OZ_in(0),OZ_in(1)));
}
OZ_BI_end


/** 
 * \brief Test whether \a OZ_in(0) is a GeSetVar
 * 
 */
OZ_BI_define(var_is,1,1)
{
  OZ_RETURN_BOOL(OZ_isGeSetVar(OZ_in(0)));
}
OZ_BI_end


/** 
 * \brief Update greatest lower bound to contain a value
 * 
 */
OZ_BI_define(inc_val,2,0)
{
  DeclareInt(0,incValue,"The parameter has to be a value");
  return inc_GeSetVarVal(OZ_in(1),incValue);
  
}
OZ_BI_end

/** 
 * \brief Restrict least upper bound to not contain a value.
 * 
 */
OZ_BI_define(exc_val,2,0)
{
  DeclareInt(0,incValue,"The parameter has to be a value");
  return exc_GeSetVarVal(OZ_in(1),incValue);
  
}
OZ_BI_end

/** 
 * \brief 
 * 
 */
OZ_BI_define(is_In,3,1)
{
  DeclareInt(0,value,"The parameter has to be a value");
  return isIn_GeSetVar(OZ_in(1),value,OZ_in(2));  
}
OZ_BI_end



/** 
 * \brief Returns the Max number that gecode can representate
 * 
 * @param 0 Max integer in c++
 */

OZ_BI_define(set_sup,0,1)
{
  OZ_RETURN_INT(Set::Limits::max);
} 
OZ_BI_end

/** 
 * \brief Returns the Min number that gecode can representate
 * 
 * @param 0 Min integer in c++
 */

OZ_BI_define(set_inf,0,1)
{
  OZ_RETURN_INT(Set::Limits::min);
} 
OZ_BI_end

OZ_BI_define(gfs_unknownSize,1,1){
  DeclareGSpace(home);
  int num = 0;
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);  
    try{
      Gecode::Set::SetView sv(__s);
      num = sv.unknownSize();
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else if(SetValueM::OZ_isSetValueM(OZ_in(0))){
    //the setvar is now determined, return 0.
    OZ_RETURN_INT(num);
  }
  //TODO: raise a more descriptive error
  else{
    return OZ_typeError(0, "Malformed Propagator, Variable must be a finite set");
  }
  OZ_RETURN_INT(num);
}OZ_BI_end

 //this has to be tested!
OZ_BI_define(gfs_ValueToString, 1,1)
{
  oz_declareNonvarIN(0,in);

  //if (oz_isFSetValue(in)) {
    char *s = OZ_toC(in,100,100);
    OZ_RETURN_STRING(s);
    //}
    //oz_typeError(0,"FSetValue");
} OZ_BI_end

OZ_BI_define(gfs_ValueIs, 1,1)
{
  OZ_Term term = OZ_in(0);
  DEREF(term, term_ptr);
  Assert(!oz_isRef(term));
  if (oz_isVarOrRef(term))
    oz_suspendOnPtr(term_ptr);
  
  OZ_RETURN(oz_bool(SetValueM::OZ_isSetValueM(term)));
  
} OZ_BI_end

OZ_BI_define(gfs_unknown,1,1){
  DeclareGSpace(home);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);  

    int cont1 = 0;
    Gecode::SetVarUnknownRanges ranges1(__s);
    for(;ranges1(); ++ranges1, cont1++);
    TaggedRef array[cont1];

    Gecode::SetVarUnknownRanges ranges2(__s);
    int cont2 = 0;

    while(ranges2()){
      int min = ranges2.min();
      int max = ranges2.max();

      if(min == max){
	array[cont2] = OZ_int(min); 
      }
      else{
	array[cont2] = OZ_mkTupleC("#",2,
				  OZ_int(min),
				  OZ_int(max));
      }
      ++ranges2;
      cont2++;
    }
    OZ_RETURN(OZ_toList(cont2, array));  
  } else {
    /**anfelbar@: this is not what we want.
    if the variables is not a setvar, raise error!
    but, whenever a setvar, intvar or boolvar is assigned,
    it is no more GeSetVar, GeIntVar or GeBoolVar, this is bad!*/
    OZ_RETURN(oz_nil());
  }
}OZ_BI_end

OZ_BI_define(gfs_unknownList,1,1){
  DeclareGSpace(home);

  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);

    Gecode::Set::SetView sv(__s);
    int n = sv.unknownSize();
    int cont=0;
    TaggedRef array[n];

    try{
      Gecode::SetVarUnknownValues values(__s);
      while(values()){
	//FIXME: makeTaggedSmallInt() is bad here, it causes an assertion due to overflow in mozart int limits
	//but with OZ_int() the builtin blocks and it never returns :(
	array[cont] = OZ_int(values.val());
	++values;
	cont++;
      }
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    OZ_RETURN(OZ_toList(n, array));
  }
  //TODO: raise a more descriptive error
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
}OZ_BI_end

OZ_BI_define(gfs_getGlb,1,1){
  DeclareGSpace(home);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);

    int cont1 = 0;
    Gecode::SetVarGlbRanges ranges1(__s);
    for(;ranges1(); ++ranges1, cont1++);
    TaggedRef array[cont1];

    Gecode::SetVarGlbRanges ranges2(__s);
    int cont2 = 0;

    while(ranges2()){
      int min = ranges2.min();
      int max = ranges2.max();

      if(min == max){
	array[cont2] = OZ_int(min); 
      }
      else{
	array[cont2] = OZ_mkTupleC("#",2,
				   OZ_int(min),
				   OZ_int(max));
      }
      ++ranges2;
      cont2++;
    }
    OZ_RETURN(OZ_toList(cont2, array));
  }
}OZ_BI_end

OZ_BI_define(gfs_getLub,1,1){
  DeclareGSpace(home);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);

    int cont1 = 0;
    Gecode::SetVarLubRanges ranges1(__s);
    for(;ranges1(); ++ranges1, cont1++);
    TaggedRef array[cont1];

    Gecode::SetVarLubRanges ranges2(__s);
    int cont2 = 0;

    while(ranges2()){
      int min = ranges2.min();
      int max = ranges2.max();

      if(min == max){
	array[cont2] = OZ_int(min); 
      }
      else{
	array[cont2] = OZ_mkTupleC("#",2,
				   OZ_int(min),
				   OZ_int(max));
      }
      ++ranges2;
      cont2++;
    }
    OZ_RETURN(OZ_toList(cont2, array));
  }
}OZ_BI_end

OZ_BI_define(gfs_getGlbList,1,1){
  DeclareGSpace(home);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);
    
    int cont1 = 0;
    Gecode::SetVarGlbValues values1(__s);
    for(;values1(); ++values1, cont1++);
    TaggedRef array[cont1];

    Gecode::SetVarGlbValues values2(__s);
    int cont2 = 0;

    while(values2()){
      array[cont2] = OZ_int(values2.val()); 
      ++values2;
      cont2++;
    }
    OZ_RETURN(OZ_toList(cont2, array));
  }
}OZ_BI_end

OZ_BI_define(gfs_getLubList,1,1){
  DeclareGSpace(home);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    DeclareGeSetVar(0, __s, home);
    
    int cont1 = 0;
    Gecode::SetVarLubValues values1(__s);
    for(;values1(); ++values1, cont1++);
    TaggedRef array[cont1];

    Gecode::SetVarLubValues values2(__s);
    int cont2 = 0;

    while(values2()){
      array[cont2] = OZ_int(values2.val()); 
      ++values2;
      cont2++;
    }
    OZ_RETURN(OZ_toList(cont2, array));
  }
}OZ_BI_end

#endif
