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
 * Return a SetView from a GeSetVar or SetValue
 * @param set GeSetVar or SetValue
 */
inline
SetView setOrSetView(TaggedRef set){
  if(SetValueM::OZ_isSetValueM(set)){
    SetVar *sv = new SetVar((oz_currentBoard()->getGenericSpace()), SetValueM::tagged2SetVal(set)->getLBValue(), SetValueM::tagged2SetVal(set)->getLBValue());
    SetView vw(*sv);
    delete sv;
    return vw;
  } else if (OZ_isGeSetVar(set)){
    return get_SetView(set);
  }
}

/**
 * \brief Return a SetVarArgs. Space stability is affected as a side effect.
 * @param vaar Array of GeSetVars and/or integers
 */
inline
Gecode::SetVarArgs getSetVarArgs(TaggedRef vaar){
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
	array[i] = setOrSetView(OZ_deref(OZ_head(t)));
      }
      return array;
    } else 
      if(OZ_isTuple(t)) {
	sz=OZ_width(t);
	SetVarArgs array(sz);
	for(int i=0; i<sz; i++) {
	  array[i] = setOrSetView(OZ_getArg(t,i));
	}
	return array;
      } else {
	Assert(OZ_isRecord(t));
	OZ_Term al = OZ_arityList(t);
	sz = OZ_width(t);
	SetVarArgs array(sz);
	for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {
	  array[i] = setOrSetView(OZ_subtree(t,OZ_head(al)));
	}
	return array;
      }  
}

/**
   ############################## New variables from Gecode declare macros ##############################
*/  

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
 * \brief Returns a SetOpType from a integer
 * @param sot is a number between 0 and 3
 */
inline 
SetOpType getSetOpType(TaggedRef sot){
  Assert(OZ_isSetOpType(sot));
  return (SetOpType) OZ_intToC(sot);
}

#endif
