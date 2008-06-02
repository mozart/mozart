/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
 *    Andres Felipe Barco, 2008
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
 
 
#ifndef __GFS_SET_DECLARE_MACROS_HH__
#define __GFS_SET_DECLARE_MACROS_HH__

#include "../GeVar.hh"
#include "GeSetVar.hh"
#include "builtins.hh"
#include "../libcp-int/GeIntVar.hh"
#include "../libcp-int/IntVarMacros.hh"


/**
   ############################## Variable declaration macros ##############################
*/

/**
 * \brief Macros for variable declaration inside propagators posting  built-ins. Space stability is affected as a side effect.
 * @param p argument at position p in the OZ_in
 * @param v the new SetlVar Variable 
 * @param sp the space where the variable bellows to
 */
#define DeclareGeSetVar(p,v,sp)           \
  declareInTerm(p,v##x);            \
  SetVar v;               \
  {                 \
    if(SetValueM::OZ_isSetValueM(v##x)) {       \
      SetVar _tmp( (sp) ,SetValueM::tagged2SetVal(v##x)->getLBValue(),SetValueM::tagged2SetVal(v##x)->getLBValue()); \
      v=_tmp;               \
    }                 \
    else if(OZ_isGeSetVar(v##x)) {          \
      v = get_SetVar(OZ_in(p));           \
    }                 \
    else return OZ_typeError(p,"SetVar or SetValue");     \
  }
  
/**
 * \brief Declares a GeBoolVar inside a var array. Space stability is affected as a side effect. 
 * @param val argument at position p in the OZ_in
 * @param ar the array of SetVars
 * @param i the index of this new variable in the array \a ar
 * @param sp the space where the variable bellows to
 */
#define DeclareGeSetVarVA(val,ar,i,sp)          \
  {  declareTerm(val,x);            \
    if(SetValueM::OZ_isSetValueM(val)) {        \
      Gecode::SetVar v(sp,SetValueM::tagged2SetVal(x)->getLBValue(),SetValueM::tagged2SetVal(x)->getLBValue()); \
      ar[i] = v;              \
    }                 \
    else if(OZ_isGeSetVar(val)) {         \
      ar[i]=get_SetVar(val);            \
    }                 \
  }

/**
 * \brief Declares a Array of SetlVars
 * @param tIn the array of values
 * @param array the new array to declare
 * @param sp the space of this array
 */
#define DECLARE_SETVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,SetVarArgs,DeclareGeSetVarVA)


/**
   ############################## New variables from Gecode declare macros ##############################
*/  

/**
 * \brief Declares a Gecode::SetRelType
 * @param arg An integer defining the SetRelType
 * @param var the variable name of the SetRelType
 */
#define DeclareSetRelType(arg,var)          \
  SetRelType var;             \
  {                 \
    OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected relation type") ;  \
    var = (SetRelType)__vv;           \
  }
  
/**
 * \brief Declares a Gecode::SetOpType form a integer value.
 * @param arg An integer refering to a SetOpType
 * @param var the name of the set operation
 */
#define DeclareSetOpType(arg,var)         \
  SetOpType var;              \
  {                 \
    OZ_declareInt(arg,op);            \
    switch(op) {              \
    case 0: var = SOT_UNION; break;         \
    case 1: var = SOT_DUNION; break;          \
    case 2: var = SOT_INTER; break;         \
    case 3: var = SOT_MINUS; break;         \
    default: return OZ_typeError(arg,"Expecting atom with a set operation: Union, Disjoint union, Intersection, Difference"); \
    }}  


#endif
