/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *	   Andres Felipe Barco (anfelbar@univalle.edu.co)
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


/**
	* \brief Declares a Gecode::BoolOpType form a integer value.
	* @param arg An integer refering to a BoolOpType
	* @param var the name of the bool operation
*/
#define DeclareBoolOpType(arg,var)\
	BoolOpType var;\
	{\
		OZ_declareInt(arg,op);\
		switch(op) {\
		case 0: var = BOT_AND; break;\
		case 1: var = BOT_OR; break;\
		case 2: var = BOT_IMP; break;\
		case 3: var = BOT_EQV; break;\
		case 4: var = BOT_XOR; break;\
		default: return OZ_typeError(arg,"Expecting atom with a lgical operation: and, or, imp, eqv, xor");\
	}}


/**
	* \brief Declares a variable form OZ_Term This declaration does not affect space stability so it can not be used in propagator's built ins.
	* @param p argument at position p in the OZ_in
	* @param v the new BoolVar Variable 
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
	* \brief Macros for variable declaration inside propagators posting built-ins. Space stability is affected as a side effect.
	* @param p argument at position p in the OZ_in
	* @param v the new BoolVar Variable 
	* @param sp the space where the variable bellows to
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
	* \brief Declares a GeBoolVar inside a var array. Space stability is affected as a side effect. 
	* @param val argument at position p in the OZ_in
	* @param ar the array of BoolVars
	* @param i the index of this new variable in the array \a ar
	* @param sp the space where the variable bellows to
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

/**
	* \brief Declares a new boolean value
	* @param p the position in OZ_in
	* @param v the name of this new boolean
*/
#define DeclareBool(p, v) \
bool v;\
{\
declareInTerm(p,v##x);\
if (!OZ_isBool(v##x))\
	   return OZ_typeError(p,"atom");		\
  v = OZ_isTrue(v##x) ? true : false; \
}

/**
	* \brief Declares a Array of BoolVars
	* @param tIn the array of values
	* @param array the new array to declare
	* @param sp the space of this array
*/
#define DECLARE_BOOLVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,BoolVarArgs,DeclareGeBoolVarVA)


/**
	* \brief Declares a Gecode::IntConLevel
	* @param arg A integer defining the IntConLevel
	* @param var the variable name of the IntConLevel
*/
#define DeclareIntConLevel(arg,var) \
	IntConLevel var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected consistency level"); \
		var = (IntConLevel)__vv;\
	}
	
/**
	* \brief Declares a Gecode::IntRelType
	* @param arg An integer defining the IntRelType
	* @param var the variable name of the IntRelType
*/
#define DeclareIntRelType(arg,var) \
	IntRelType var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected relation type") ;\
		var = (IntRelType)__vv;\
	}

/**
	* \brief Declares a Gecode::IntConLevel
	* @param arg A integer defining the IntConLevel
	* @param var the variable name of the IntConLevel
*/
#define DeclareIntConLevel(arg,var) \
	IntConLevel var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected consistency level"); \
		var = (IntConLevel)__vv;\
	}

/**
	* \brief Declares a Gecode::PropKind
	* @param arg An integer defining the PropKind
	* @param var the variable name of the PropKind
*/
#define DeclarePropKind(arg,var) \
	PropKind var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected propagator kind"); \
		var = (PropKind)__vv;\
	}

/**
	* \brief Declare a new integer, the same of DeclareInt but with only two arguments in his input
	* @param arg value of the integer
	* @param var the variable declared as integer
*/
#define DeclareInt2(arg,var) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,"The value muts be a number")
  

/**
	* \brief Turn a OZ type value in a C/C++ type value
 	* @param arg the value to be transformed
 	* @param type the type of the variable to be transformed
 	* @param var the name of variable
 	* @param check check wheter the value is of the corresponding type
 	* @param conv funcion to turn the values
*/
#define OZ_TOC(arg,type,var,check,conv,msg) \
	type var;                                                      \
	{\
		if (!check(OZ_in(arg)))                                 \
			return OZ_typeError(arg,msg);                     \
		var=conv(OZ_in(arg));\
	}

/**
  * \brief Declares a GeIntVar Array. Space stability is affected as a side effect.
  * @param tIn Values of the variables in the new array
  * @param array Array of variables
  * @param sp Space where the array is declared
*/
#define DECLARE_INTVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,IntVarArgs,DeclareGeIntVarVA)

/**
	* \brief Declares a Gecode::Int:IntArgs from a literal, list, tuple or record of int values.
 	* @param tIn possition in OZ_in
 	* @param array the resulting IntArgs
*/
#define DECLARE_INTARGS(tIn,array)			\
IntArgs array(0);					\
{							\
  int sz;						\
  /*OZ_Term t = OZ_deref(OZ_in(tIn));*/			\
  OZ_declareTerm(tIn,t);                                \
  if(OZ_isLiteral(t)) {					\
    sz = 0;						\
    IntArgs _array(sz);					\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    IntArgs _array(sz);					\
    /*t = OZ_hallocOzTerms(sz);*/			\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t)) 		\
      _array[i++] = OZ_intToC(OZ_head(t));		\
    /*t[i++] = OZ_head(t);*/				\
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    IntArgs _array(sz);					\
    /*t = OZ_hallocOzTerms(sz);*/			\
    for(int i=0; i < sz; i++) {				\
      OZ_Term _tmp = OZ_getArg(t,i);			\
      _array[i] = OZ_intToC(_tmp);			\
      /*v[i] = OZ_getArg(t,i);*/			\
    }							\
    array=_array;					\
  }							\
  else {                                                \
    assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    IntArgs _array(sz);					\
    for(int i = 0; OZ_isCons(al); al=OZ_tail(al))	\
      _array[i++]=OZ_intToC(OZ_subtree(t,OZ_head(al)));	\
    array=_array;                                       \
  }                                                     \
}

/**
	* \brief Declares a GeIntVar inside a var array. Space stability is affected as a side effect.
	* @param val integer value of the new variable
	* @param ar array of variables where the new variable is posted
	* @param i position in the array assigned to the variable
	* @param sp Space where the array belows to
*/
#define DeclareGeIntVarVA(val,ar,i,sp)				\
{  declareTerm(val,x);							\
  if(OZ_isInt(val)) {						\
    int domain=OZ_intToC(val);					\
	Gecode::IntVar v(sp,domain,domain);\
	ar[i] = v;				\
  }								\
  else if(OZ_isGeIntVar(val)) {					\
          ar[i]=get_IntVar(val);					\
  }								\
}

/**
	* \brief Macros for variable declaration inside propagators posting built-ins. 
	* Space stability is affected as a side effect.
	* @param p The position in OZ
	* @param v Name of the new variable
	* @param sp The space where the variable is declared.
*/
#define DeclareGeIntVar(p,v,sp)\
  declareInTerm(p,v##x);\
  IntVar v;\
  {\
  if(OZ_isInt(v##x)) {\
    OZ_declareInt(p,domain);\
    IntVar _tmp( (sp) ,domain,domain);\
    v=_tmp;\
  }\
  else if(OZ_isGeIntVar(v##x)) {\
    v = get_IntVar(OZ_in(p));\
  }\
  else return OZ_typeError(p,"IntVar or Int");\
  }

#endif
