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

 
 
#ifndef __GFD_INT_DECLARE_MACROS_HH__
#define __GFD_INT_DECLARE_MACROS_HH__

#include "GeIntVar.hh"
#include "GeSpace-builtins.hh"
#include "builtins.hh"
#include "value.hh"
#include "../GeVar.hh"


/**
############################## Variable declaration macros ##############################
*/

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
 * \brief Declares a variable form OZ_Term argument at position p in the
 * input to be an IntVar variable. This declaration does not affect space 
 * stability so it can not be used in 
 * propagator's built ins.
 * @param p The position in OZ
 * @param v Name of the new variable
 */
#define DeclareGeIntVar1(p,v)					\
  IntVar v;							\
  {\
	 declareInTerm(p,v##x);\
    if (OZ_isInt(v##x)) {						\
      OZ_declareInt(p,domain);					\
      IntVar _tmp(oz_currentBoard()->getGenericSpace(),		\
		  domain, domain);				\
      v=_tmp;							\
    }								\
    else if(OZ_isGeIntVar(v##x)) {					\
      v = get_IntVarInfo(v##x);					\
    } else							\
      return OZ_typeError(p,"IntVar");		\
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
 * \brief Macro to declare VarArgs from OZ_terms (list, records, tuples)
 * @param tIn values
 * @param array the array of type @a type
 * @param sp the space of declaration
 * @param type the type of variable, must be IntVarArgs or BoolVarargs.
 * @param opDecl is the name of a function, macro to declare one variable 
 * of the corresponding array type.
 */

/*#define DECLARE_VARARGS(tIn,array,sp,type,opDecl)  		\
int __x##tIn = 0; \
{ \
	declareInTerm(tIn,t);\
	__x##tIn =  OZ_isLiteral(t) ? 0 : OZ_isCons(t) ? OZ_length(t) : OZ_width(t); \
} \
type array(__x##tIn);					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::type _array(sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::type _array(sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      opDecl(OZ_deref(OZ_head(t)),_array,i,sp); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::type _array(sz);	\
    for(int i=0;i<sz;i++) {				\
      opDecl(OZ_getArg(t,i),_array,i,sp);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    Assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::type _array(sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      opDecl(OZ_subtree(t,OZ_head(al)),_array,i,sp);\
    }							\
    array=_array;                                       \
    }							\
}*/
  
/**
  * \brief Declares a GeIntVar Array. Space stability is affected as a side effect.
  * @param tIn Values of the variables in the new array
  * @param array Array of variables
  * @param sp Space where the array is declared
*/
#define DECLARE_INTVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,IntVarArgs,DeclareGeIntVarVA)

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
{\
declareInTerm(p,v##x);\
if (!OZ_isBool(v##x))\
	   return OZ_typeError(p,"atom");		\
  v = OZ_isTrue(v##x) ? true : false; \
}

#define DECLARE_BOOLVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,BoolVarArgs,DeclareGeBoolVarVA)
 
 
/**
############################## Domain declaration ##############################
*/

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param _t Mozart domain specification
 */
#define	DECLARE_INT_SET(_t,ds)					\
  OZ_Term _l = (OZ_isCons(_t) ? _t : OZ_cons(_t, OZ_nil()));	\
  int _length = OZ_length(_l);					\
  int _pairs[_length][2];					\
  for (int i = 0; OZ_isCons(_l); _l=OZ_tail(_l), i++) {		\
    OZ_Term _val = OZ_head(_l);					\
    if (OZ_isInt(_val)) {					\
      _pairs[i][0] = OZ_intToC(_val);				\
      _pairs[i][1] = OZ_intToC(_val);				\
    }								\
    else if (OZ_isTuple(_val)) {				\
      _pairs[i][0] = OZ_intToC(OZ_getArg(_val,0));		\
      _pairs[i][1] = OZ_intToC(OZ_getArg(_val,1));		\
    }							\
    else {							\
      return OZ_typeError(0,"malformed domain description");	\
    }								\
  }								\
  Gecode::IntSet ds(_pairs, _length);
  

#define DECLARE_INT_SET2(_p,_v) DECLARE_INT_SET(OZ_in(_p),_v)
/** 
 * \brief Declares a Gecode::Int::IntSetArgs from an Oz domain description, no working yet.
 * 
 * @param ds The domain description int terms of list and tuples
 * @param _t Mozart domain specification
 */
#define	DECLARE_INT_SET_ARGS(_t,ds)					\
  OZ_Term l = (OZ_isCons(_t) ? _t : OZ_cons(_t, OZ_nil()));	\
  int length = OZ_length(l);					\
  int _pairs[length][2];					\
	std::vector<int> ar;\
								\
  for (int i = 0; OZ_isCons(l); l=OZ_tail(l), i++) {		\
    OZ_Term _val = OZ_head(l);					\
    if (OZ_isIntSet(_val)) {					\
      _pairs[i][0] = OZ_intToC(OZ_getArg(_val,0)); \
      _pairs[i][1] = OZ_intToC(OZ_getArg(_val,1));				\
			ar[i] = OZ_intToC(OZ_getArg(_val,0)); \
    }								\
    else {							\
      return OZ_typeError(0,"domain type unknown");		\
    }								\
  }								\
  Gecode::PrimArgArray<IntSet> ds(length);



/**
	* \brief Declares a Gecode::Int:IntArgs from a list of int values.
 	* @param array the resulting IntArgs
	* @param t possition in OZ_in
*/
#define DECLARE_INTARGS1(array,t)						\
	OZ_declareTerm(t,_termTmp_args);							\
	int __length_args=1;									\
	__length_args=OZ_length(_termTmp_args);							\
	IntArgs array(__length_args);								\
	if( __length_args != -1 ){								\
		for(int i = 0; OZ_isCons(_termTmp_args); _termTmp_args=OZ_tail(_termTmp_args),i++)	\
		{									\
			OZ_Term val = OZ_head(_termTmp_args);				\
			if(true){							\
				array[i]=OZ_intToC(val);					\
			}								\
			else{								\
				std::cout<<"Error,  Variable type is not GDF"<<std::endl;	\
			}								\
		}									\
	}										\


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
	* \brief Declare a Gecode::TupleSet from a domain description.
	* @param _t tuple with IntArgs as values
	* @param ds domain specification
*/
#define DeclareTupleSet(_t, ds)\
  OZ_Term l = (OZ_isCons(_t) ? _t : OZ_cons(_t, OZ_nil()));	\
	Gecode::TupleSet aa;\
	int ta = OZ_length(l);\
	IntArgs array(ta);							\
  for (int i = 0; OZ_isCons(l); l=OZ_tail(l), i++) {		\
    OZ_Term _val = OZ_head(l);					\
    if (OZ_isIntArgs(_val)) {					\
      aa.add((IntArgs)_val);\
    }								\
    else {							\
      return OZ_typeError(0,"domain type unknown");		\
    }								\
  }\
  Gecode::TupleSet ds(aa);


/**
############################## New variables from Gecode declare macros ##############################
*/

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
	* \brief Declares a Gecode::DFA::Transition for DFA
	* @param val the name of the new transition
	* @param tr tuple with the values of transition
*/
#define TransitionS(val, tr)			\
  { Gecode::DFA::Transition _t;			\
    _t.i_state = OZ_intToC(OZ_getArg(tr,0));	\
    _t.symbol  = OZ_intToC(OZ_getArg(tr,1));	\
    _t.o_state = OZ_intToC(OZ_getArg(tr,2));	\
    val = _t;					\
  }
	
/**
	* \brief Declares a Gecode::DFA with the argument pos
	* @param pos the position in the OZ_in
	* @param dfa the name of the new declared DFA
*/
#define DeclareDFA(pos, dfa)						\
  OZ_Term _t = OZ_in(pos);						\
  OZ_Term _inputl = OZ_arityList(_t);					\
  OZ_Term _inputs = OZ_subtree(_t, OZ_head(_inputl));			\
  int _istate     = OZ_intToC(_inputs);					\
  OZ_Term _tl    = OZ_subtree(_t, OZ_head(OZ_tail(_inputl)));		\
  Gecode::DFA::Transition _trans[OZ_length(_tl)];			\
  for(int i=0; OZ_isCons(_tl); _tl=OZ_tail(_tl)) {			\
    TransitionS(_trans[i++], OZ_head(_tl));				\
  }									\
  OZ_Term _fstates = OZ_subtree(_t, OZ_head(OZ_tail(OZ_tail(_inputl)))); \
  _fstates = oz_deref(_fstates);					\
  int _fl[OZ_length(_fstates)];						\
  for(int i=0; OZ_isCons(_fstates); _fstates=OZ_tail(_fstates)) {	\
    _fl[i++] = OZ_intToC(OZ_head(_fstates));				\
  }									\
  Gecode::DFA dfa(_istate, _trans, _fl);	


/**
############################## Micelanious declare macros ##############################
*/

/**
	* \brief Declares a IntAssignType
	* @param arg the position in the OZ_in
	* @param var the name of the new IntAssignType
*/
#define DeclareIntAssignType(arg,var)\
	IntAssign var;\
	{\
		OZ_declareInt(arg,op);\
		switch(op) {\
		case 0: var = INT_ASSIGN_MIN; break;\
		case 1: var = INT_ASSIGN_MED; break;\
		case 2: var = INT_ASSIGN_MAX; break;\
		default: return OZ_typeError(arg,"Expecting atom with an integer assign type: min, med, max");\
	}}	
	
/**
	* \brief Declare a new integer
	* @param arg value of the integer
	* @param var the variable declared as integer
*/
#define DeclareInt(arg,var,msg) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,msg)
	
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
	* \brief This macro is used to wait for the variable if it is not defined yet (e.g. a free var).
	* @param trm is or will be bind to a constraint variable. 
	* @param varName the name of the variable
*/
/*#define declareTerm(trm,varName) \
	TaggedRef varName = (trm);\
	{\
		DEREF(varName,varName_ptr);\
		Assert(!oz_isRef(varName));\
		if (oz_isFree(varName)) {\
			oz_suspendOn(makeTaggedRef(varName_ptr));\
		}}

/**
	* \brief Waits until variable at position pos is defined and then declares it as an OZ_Term
	* @param pos position in OZ_in
	* @param varName the name of the new OZ_Term
*/
//#define declareInTerm(pos,varName) declareTerm(OZ_in(pos),varName)


#endif
