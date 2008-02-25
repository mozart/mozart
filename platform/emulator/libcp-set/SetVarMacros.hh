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
 
 
#ifndef __GFS_SET_DECLARE_MACROS_HH__
#define __GFS_SET_DECLARE_MACROS_HH__

#include "../GeVar.hh"
#include "GeSetVar.hh"
#include "builtins.hh"
#include "../libcp-int/GeIntVar.hh"



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
	* @param array the new array to declareInTerm
	* @param sp the space of this array
*/
#define DECLARE_BOOLVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,BoolVarArgs,DeclareGeBoolVarVA)

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param arg Position at OZ_in array
 */
#define	DECLARE_INT_SET(ds,val,arg)				           \
  IntSet *tmp##val;                                           \
  if(OZ_isNil(OZ_in(arg))){                                                \
    tmp##val = new Gecode::IntSet(IntSet::empty);                             \
  }else{                                                                   \
  OZ_declareDetTerm(arg,val);					           \
  OZ_Term val##arg = (OZ_isCons(val) ? val : OZ_cons(val, OZ_nil()));	   \
  int length##val = OZ_length(val##arg);				   \
  int _pairs##val[length##val][2];					   \
  for (int i = 0; OZ_isCons(val##arg); val##arg=OZ_tail(val##arg), i++) {		\
      OZ_Term _val = OZ_head(val##arg);					   \
      if (OZ_isInt(_val)) {					           \
        _pairs##val[i][0] = OZ_intToC(_val);			           \
        _pairs##val[i][1] = OZ_intToC(_val);			           \
      }								           \
      else if (OZ_isTuple(_val)) {				           \
        _pairs##val[i][0] = OZ_intToC(OZ_getArg(_val,0));		   \
        _pairs##val[i][1] = OZ_intToC(OZ_getArg(_val,1));		   \
      }								           \
      else {							           \
        OZ_typeError(arg,"Int domain");		                           \
      }								           \
    }								           \
    tmp##val = new Gecode::IntSet(_pairs##val, length##val);               \
  }                                                                        \
  Gecode::IntSet ds(*tmp##val);                                            \


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
	* \brief Declare a new integer, the same of DeclareInt but with only two arguments in his input
	* @param arg value of the integer
	* @param var the variable declared as integer
*/
#define DeclareInt2(arg,var) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,"The value muts be a number")
	
/**
	* \brief Declare a new integer
	* @param arg value of the integer
	* @param var the variable declared as integer
*/
#define DeclareInt(arg,var,msg) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,msg)

/**
	* \brief Macros for variable declaration inside propagators posting	built-ins. Space stability is affected as a side effect.
	* @param p argument at position p in the OZ_in
	* @param v the new SetlVar Variable 
	* @param sp the space where the variable bellows to
*/
#define DeclareGeSetVar(p,v,sp) \
  declareInTerm(p,v##x); \
  SetVar v; \
  { \
  	if(SetValueM::OZ_isSetValueM(v##x)) { \
    	SetVar _tmp( (sp) ,SetValueM::tagged2SetVal(v##x)->getLBValue(),SetValueM::tagged2SetVal(v##x)->getLBValue()); \
    	v=_tmp; \
  	}\
  	else if(OZ_isGeSetVar(v##x)) { \
    	v = get_SetVar(OZ_in(p)); \
  	}\
  	else return OZ_typeError(p,"SetVar or SetValue"); \
  }

/**
	* \brief Declares a GeBoolVar inside a var array. Space stability is affected as a side effect. 
	* @param val argument at position p in the OZ_in
	* @param ar the array of SetVars
	* @param i the index of this new variable in the array \a ar
	* @param sp the space where the variable bellows to
*/
#define DeclareGeSetVarVA(val,ar,i,sp)				\
{  declareTerm(val,x);							\
  if(SetValueM::OZ_isSetValueM(val)) {						\
       Gecode::SetVar v(sp,SetValueM::tagged2SetVal(x)->getLBValue(),SetValueM::tagged2SetVal(x)->getLBValue());\
       ar[i] = v;				\
  }								\
  else if(OZ_isGeSetVar(val)) {					\
          ar[i]=get_SetVar(val);					\
  }								\
}

/**
	* \brief Declares a Array of SetlVars
	* @param tIn the array of values
	* @param array the new array to declare
	* @param sp the space of this array
*/
#define DECLARE_SETVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,SetVarArgs,DeclareGeSetVarVA)

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
  * \brief Declares a GeIntVar Array. Space stability is affected as a side effect.
  * @param tIn Values of the variables in the new array
  * @param array Array of variables
  * @param sp Space where the array is declared
*/
#define DECLARE_INTVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,IntVarArgs,DeclareGeIntVarVA)

/**
	* \brief Declares a Gecode::SetRelType
	* @param arg An integer defining the SetRelType
	* @param var the variable name of the SetRelType
*/
#define DeclareSetRelType(arg,var) \
	SetRelType var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected relation type") ;\
		var = (SetRelType)__vv;\
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
  
/**
	* \brief Declares a Gecode::SetOpType form a integer value.
	* @param arg An integer refering to a SetOpType
	* @param var the name of the set operation
*/
#define DeclareSetOpType(arg,var)\
	SetOpType var;\
	{\
		OZ_declareInt(arg,op);\
		switch(op) {\
		case 0: var = SOT_UNION; break;\
		case 1: var = SOT_DUNION; break;\
		case 2: var = SOT_INTER; break;\
		case 3: var = SOT_MINUS; break;\
		default: return OZ_typeError(arg,"Expecting atom with a set operation: Union, Disjoint union, Intersection, Difference");\
	}}	


#endif
