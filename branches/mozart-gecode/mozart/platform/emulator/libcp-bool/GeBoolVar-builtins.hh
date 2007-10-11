/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
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

#ifndef __GEOZ_BOOLVAR_BUILTINS_HH__
#define __GEOZ_BOOLVAR_BUILTINS_HH__

#include "value.hh"
#include "GeBoolVar.hh"
#include "GeSpace-builtins.hh"
#include "builtins.hh"


/**
	Declares a veriable form OZ_Term argument at position p in the input to be an IntVar 
	variable. This declaration does not affect space stability so it can not be used in 
	propagator's built ins.
*/
#define DeclareGeBoolVar1(p,v)					\
  BoolVar v;							\
  {\
	declareInTerm(p,v##x);\
    if (OZ_isInt(v##x)) {						\
      OZ_declareInt(p,domain);					\
	  if (domain < 0 || domain > 1) {return OZ_typeError(p,"Cannot create a BoolVar form integers different from 0 or 1");}\
      BoolVar _tmp(oz_currentBoard()->getGenericSpace(),		\
		  domain, domain);				\
      v=_tmp;							\
    }								\
    else if(OZ_isGeBoolVar(v##x)) {					\
      v = get_BoolVarInfo(v##x);					\
    } else							\
      return OZ_typeError(p,"BoolVar");		\
  }
  
  /**
	Macros for variable declaration inside propagators posting
	built-ins. Space stability is affected as a side effect.
*/
#define DeclareGeBoolVar(p,v,sp)\
  declareInTerm(p,v##x);\
  BoolVar v;\
  {\
  if(OZ_isInt(v##x)) {\
    OZ_declareInt(p,domain);\
	 if (domain < 0 || domain > 1) {return OZ_typeError(p,"Cannot create a BoolVar form integers different from 0 or 1");}\
    BoolVar _tmp( (sp) ,domain,domain);\
    v=_tmp;\
  }\
  else if(OZ_isGeBoolVar(v##x)) {\
    v = get_BoolVar(OZ_in(p));\
  }\
  else return OZ_typeError(p,"BoolVar or Int");\
  }

/**
	Declares a GeBoolVar inside a var array. Space stability is affected as a side effect. 
*/
// TODO: Improve error reporting 0 s not the argument position
#define DeclareGeBoolVarVA(val,ar,i,sp)				\
{  declareTerm(val,x);							\
  if(OZ_isInt(val)) {						\
    int domain=OZ_intToC(val);					\
	 if (domain < 0 || domain > 1) {return OZ_typeError(0,"Cannot create a BoolVar form integers different from 0 or 1");}\
	Gecode::BoolVar v(sp,domain,domain);\
	ar[i] = v;				\
  }								\
  else if(OZ_isGeBoolVar(val)) {					\
          ar[i]=get_BoolVar(val);					\
  }								\
}

#define DeclareBool(p, v) \
bool v;\
declareInTerm(p,v##x);\
if (!OZ_isBool(v##x))\
	   return OZ_typeError(p,"atom");		\
  v = OZ_isTrue(v##x) ? true : false; \
}

#define DECLARE_BOOLVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,BoolVarArgs,DeclareGeBoolVarVA)


#endif
