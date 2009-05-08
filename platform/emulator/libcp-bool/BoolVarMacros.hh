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
 * \brief Return a BoolVarArgs. Space stability is affected as a side effect.
 * @param vaar Array of GeBoolVars and/or integers (0#1)
 */
inline
Gecode::BoolVarArgs getBoolVarArgs(TaggedRef vaar){
  int sz;
  TaggedRef t = vaar;

  Assert(OZ_isBoolVarArgs(vaar));

  if(OZ_isLiteral(OZ_deref(t))) {
    sz=0;
    BoolVarArgs array(sz);
    return array;
  } else
    if(OZ_isCons(t)) {
      sz = OZ_length(t);
      BoolVarArgs array(sz);
      for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){
	BoolVar *iv = boolOrBoolVar(OZ_deref(OZ_head(t)));
	array[i] = *iv;
	delete iv;
      }
      return array;
    } else 
      if(OZ_isTuple(t)) {
	sz=OZ_width(t);
	BoolVarArgs array(sz);
	for(int i=0; i<sz; i++) {
	  BoolVar *iv = boolOrBoolVar(OZ_getArg(t,i));
	  array[i] = *iv;
	  delete iv;
	}
	return array;
      } else {
	Assert(OZ_isRecord(t));
	OZ_Term al = OZ_arityList(t);
	sz = OZ_width(t);
	BoolVarArgs array(sz);
	for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {
	  BoolVar *iv = boolOrBoolVar(OZ_subtree(t,OZ_head(al)));
	  array[i] = *iv;
	  delete iv;
	}
	return array;
      }  
}
  
  
/**
   ############################## New variables from Gecode declare macros ##############################
*/  
  
/**
 * \brief Declares a Gecode::BoolOpType form a integer value.
 * @param arg An integer refering to a BoolOpType
 * @param var the name of the bool operation
 */
inline
Gecode::BoolOpType getBoolOpType(TaggedRef arg){
  Assert(OZ_isBoolOpType(arg));
  int op = OZ_intToC(arg);
  switch(op) {
  case 0: return BOT_AND;
  case 1: return BOT_OR;
  case 2: return BOT_IMP;
  case 3: return BOT_EQV; 
  case 4: return BOT_XOR;
  }
} 
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
