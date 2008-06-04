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


// this will be reflect.unknown

// OZ_BI_define(gfs_reflectUnknown_2,2,0){
//   DeclareGSpace(home);
//   int result;

//   if(OZ_isGeSetVar(OZ_in(0)) && OZ_isList(OZ_in(1))){
//     DeclareGeSetVar(0, __s, home);
//     DeclareGeBoolVar(1, __b, home);

//     try{
//       Gecode::dom(home, __s, Gecode::SRT_SUP, __i, __b);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else{
//     return OZ_typeError(0, "Malformed Propagator");
//   }

//   CHECK_POST(home);
// }OZ_BI_end

// this will be value.make

OZ_BI_define(gfs_valueMake_2,2,0){
  DeclareGSpace(home);
  
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1))){
    DeclareGeIntVar(0, __i, home);
    DeclareGeSetVar(1, __s, home);
    
    try{
      
      printf("__i.min(), __i.max() = %d, %d\n", __i.min(), __i.max());
      //Gecode::SetVar x(home, __i.min(), __i.max(), __i.min(), __i.max());
      //__s.update(home, true, x);
      // ???
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  //TODO: raise a more descriptive error
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
}OZ_BI_end

#endif
