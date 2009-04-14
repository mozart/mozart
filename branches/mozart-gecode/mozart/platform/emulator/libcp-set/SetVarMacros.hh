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
 * \brief Macros for variable declaration inside propagators posting builtins. Space stability is affected as a side effect.
 * @param p argument at position p in the OZ_in
 * @param v the new SetVar Variable 
 * @param sp the space where the variable belongs to
 */
#define DeclareGeSetVar(p,v,sp)						\
  declareInTerm(p,v##x);						\
  SetVar v;								\
  {									\
    if(SetValueM::OZ_isSetValueM(v##x)) {				\
      SetVar _tmp((sp), SetValueM::tagged2SetVal(v##x)->getLBValue(), SetValueM::tagged2SetVal(v##x)->getLBValue()); \
      v=_tmp;								\
    }									\
    else if(OZ_isGeSetVar(v##x)) {					\
      v = get_SetVar(OZ_in(p));						\
    }									\
    else return OZ_typeError(p,"SetVar or SetValue");			\
  }

/**
 * Return a SetVar pointer from a GeSetVar or SetValue
 * @param set GeSetVar or SetValue
 */
inline
SetVar * setOrSetVar(TaggedRef set){
  if(SetValueM::OZ_isSetValueM(set)){
    return new SetVar((oz_currentBoard()->getGenericSpace()), SetValueM::tagged2SetVal(set)->getLBValue(), SetValueM::tagged2SetVal(set)->getLBValue());
  } else if (OZ_isGeSetVar(set)){
    return get_SetVarPtr(set);
  }
}
  
/**
 * \brief Declares a GeSetVar inside a var array. Space stability is affected as a side effect. 
 * @param val argument at position p in the OZ_in
 * @param ar the array of SetVars
 * @param i the index of this new variable in the array \a ar
 * @param sp the space where the variable bellows to
 */
#define DeclareGeSetVarVA(val,ar,i,sp)					\
  {  declareTerm(val,x);						\
    if(SetValueM::OZ_isSetValueM(val)) {				\
      Gecode::SetVar v(sp,SetValueM::tagged2SetVal(x)->getLBValue(),SetValueM::tagged2SetVal(x)->getLBValue()); \
      ar[i] = v;							\
    }									\
    else if(OZ_isGeSetVar(val)) {					\
      ar[i]=get_SetVar(val);						\
    }									\
  }

/**
 * \brief Declares a Array of SetVars
 * @param tIn the array of values
 * @param array the new array to declare
 * @param sp the space of this array
 */
#define DECLARE_SETVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,SetVarArgs,DeclareGeSetVarVA)

/**
 * Returns a SetVarArgs from a List, tuple or
 * record. The structure only allows GeSetVar or SetValues.
 * @param vaar is a vector of GeSetVar or SetValues
 */
inline
SetVarArgs getSetVarArgs(TaggedRef vaar){
  int sz;
  TaggedRef t = vaar;

  Assert(OZ_isSetVarArgs(vaar));

  if(OZ_isLiteral(OZ_deref(t))) {
    sz=0;
    SetVarArgs array(sz);
    return array;
  } else
    if(OZ_isCons(t)) {
      sz = OZ_length(t);
      SetVarArgs array(sz);
      for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){
	SetVar *sv = setOrSetVar(OZ_deref(OZ_head(t)));
	array[i] = *sv;
	delete sv;
      }
      return array;
    } else 
      if(OZ_isTuple(t)) {
	sz=OZ_width(t);
	SetVarArgs array(sz);
	for(int i=0; i<sz; i++) {
	  SetVar *sv = setOrSetVar(OZ_getArg(t,i));
	  array[i] = *sv;
	  delete sv;
	}
	return array;
      } else {
	Assert(OZ_isRecord(t));
	OZ_Term al = OZ_arityList(t);
	sz = OZ_width(t);
	SetVarArgs array(sz);
	for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {
	  SetVar *sv = setOrSetVar(OZ_subtree(t,OZ_head(al)));
	  array[i] = *sv;
	  delete sv;
	}
	return array;
      }  
}


/**
   ############################## New variables from Gecode declare macros ##############################
*/  

/**
 * \brief Declares a Gecode::SetRelType
 * @param arg An integer defining the SetRelType
 * @param var the variable name of the SetRelType
 */
#define DeclareSetRelType(arg,var)					\
  SetRelType var;							\
  {									\
    OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected relation type") ;	\
    var = (SetRelType)__vv;						\
  }

/**
 * \brief Returns a SetRelType from a integer
 * @param srt is a number between 0 and 5
 */
inline
SetRelType getSetRelType(TaggedRef srt){
  Assert(OZ_isSetRelType(srt));
  return (SetRelType) OZ_intToC(srt);
}
  
/**
 * \brief Declares a Gecode::SetOpType form a integer value.
 * @param arg An integer refering to a SetOpType
 * @param var the name of the set operation
 */
#define DeclareSetOpType(arg,var)					\
  SetOpType var;							\
  {									\
    OZ_declareInt(arg,op);						\
    switch(op) {							\
    case 0: var = SOT_UNION; break;					\
    case 1: var = SOT_DUNION; break;					\
    case 2: var = SOT_INTER; break;					\
    case 3: var = SOT_MINUS; break;					\
    default: return OZ_typeError(arg,"Expecting atom with a set operation: Union, Disjoint union, Intersection, Difference"); \
    }}	


/**
 * \brief Returns a SetOpType from a integer
 * @param sot is a number between 0 and 3
 */
inline 
SetOpType getSetOpType(TaggedRef sot){
  Assert(OZ_isSetOpType(sot));
  return (SetOpType) OZ_intToC(sot);
}

#endif
