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
 
#ifndef __GBD_BOOL_DECLARE_MACROS_HH__
#define __GBD_BOOL_DECLARE_MACROS_HH__

#include "value.hh"
#include "../GeVar.hh"
#include "GeBoolVar.hh"
#include "builtins.hh"
#include "../libcp-int/GeIntVar.hh"
#include "../libcp-int/IntVarMacros.hh"


/**
   ############################## Variable declaration macros ##############################
*/

/**
 * \brief Macros for variable declaration inside propagators posting built-ins. Space stability is affected as a side effect.
 * @param p The new variable.
 * @param s The space where the variable is declared.
 */
inline
BoolVar* declareBV(OZ_Term p, GenericSpace *s) {
  BoolVar *v = NULL;
  if (OZ_isInt(p)) {
    int val = OZ_intToC(p);
    if (val == 0 || val == 1) {
      v = new BoolVar(s,val,val);
    }
  } else if (OZ_isGeBoolVar(p)) {
    v = &get_BoolVar(p);
  } 
  return v;
} 

/**
 * \brief Macros for variable declaration inside propagators posting built-ins. 
 * Space stability is affected as a side effect.
 * @param p The position in OZ_in
 * @param v Name of the new variable
 * @param sp The space where the variable is declared.
 */
#define DeclareGeBoolVar(p,v,sp)					\
  declareInTerm(p,v##x);						\
  BoolVar v;								\
  { BoolVar *vp = declareBV(v##x,sp);					\
    if (vp == NULL) return OZ_typeError(p,"Error while declaring a BoolVar, expected BoolVar or Int between 0 and 1"); \
    v = *vp;								\
  }


/**
 * \brief Return a BoolVar from a integer or a GeBoolVar
 * @param value integer or GeBoolVar representing a BoolVar
 */
inline
BoolVar * boolOrBoolVar(TaggedRef value){
  if(OZ_isInt(value)){
    return new BoolVar(oz_currentBoard()->getGenericSpace(), OZ_intToC(value), OZ_intToC(value));
  }else {
    return get_BoolVarPtr(value);
  }
}

/**
 * \brief Declares a GeBoolVar inside a var array. Space stability is affected as a side effect.
 * @param val integer value of the new variable
 * @param ar array of variables where the new variable is posted
 * @param i position in the array assigned to the variable
 * @param sp Space where the array belongs to
 */
// TODO: Improve error reporting 0 s not the argument position
#define DeclareGeBoolVarVA(val,ar,i,sp)					\
  {  declareTerm(val,x);						\
    if(OZ_isInt(val)) {							\
      int domain=OZ_intToC(val);					\
      if (domain < 0 || domain > 1) {					\
	return OZ_typeError(0,"Cannot create a BoolVar form integers different from 0 or 1"); \
      }									\
      Gecode::BoolVar v(sp,domain,domain);				\
      ar[i] = v;							\
    }									\
    else if(OZ_isGeBoolVar(val)) {					\
      ar[i]=get_BoolVar(val);						\
    }									\
  }

/**
 * \brief Declares a GeBoolVarArgs. Space stability is affected as a side effect.
 * @param tIn Values of the variables in the new array
 * @param array Array of variables
 * @param sp Space where the array is declared
 */
#define DECLARE_BOOLVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,BoolVarArgs,DeclareGeBoolVarVA)

/**
 * \brief Declares a variable form OZ_Term This declaration does not affect space stability so it can not be used in propagator's built ins.
 * @param p argument at position p in the OZ_in
 * @param v the new BoolVar Variable 
 */
#define DeclareGeBoolVar1(p,v)						\
  BoolVar v;								\
  {									\
    declareInTerm(p,v##x);						\
    if (OZ_isInt(v##x)) {						\
      OZ_declareInt(p,domain);						\
      if (domain < 0 || domain > 1) {return OZ_typeError(p,"Cannot create a BoolVar form integers different from 0 or 1");} \
      BoolVar _tmp(oz_currentBoard()->getGenericSpace(),		\
		   domain, domain);					\
      v=_tmp;								\
    }									\
    else if(OZ_isGeBoolVar(v##x)) {					\
      v = get_BoolVarInfo(v##x);					\
    } else								\
      return OZ_typeError(p,"BoolVar");					\
  }
  
  
/**
   ############################## New variables from Gecode declare macros ##############################
*/  
  
/**
 * \brief Declares a Gecode::BoolOpType form a integer value.
 * @param arg An integer refering to a BoolOpType
 * @param var the name of the bool operation
 */
#define DeclareBoolOpType(arg,var)					\
  BoolOpType var;							\
  {									\
    OZ_declareInt(arg,op);						\
    switch(op) {							\
    case 0: var = BOT_AND; break;					\
    case 1: var = BOT_OR; break;					\
    case 2: var = BOT_IMP; break;					\
    case 3: var = BOT_EQV; break;					\
    case 4: var = BOT_XOR; break;					\
    default: return OZ_typeError(arg,"Expecting atom with a logical operation: and, or, imp, eqv, xor");	\
    }}
  
/**
   ############################## Miscelanious declare macros ##############################
*/

/**
 * \brief Declares a boolean variable
 * @param p the boolean value
 * @param var the name of the variable
 */
#define DeclareBool(p, v)			\
  bool v;					\
  {						\
    declareInTerm(p,v##x);			\
    if (!OZ_isBool(v##x))			\
      return OZ_typeError(p,"atom");		\
    v = OZ_isTrue(v##x) ? true : false;		\
  }

#endif
