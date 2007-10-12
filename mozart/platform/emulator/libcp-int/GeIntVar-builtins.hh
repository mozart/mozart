/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
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

#ifndef __GEOZ_INTVAR_BUILTINS_HH__
#define __GEOZ_INTVAR_BUILTINS_HH__

#include "value.hh"
#include "GeIntVar.hh"
#include "GeSpace-builtins.hh"
#include "builtins.hh"
// think: just one include
#include "../libcp-bool/GeBoolVar-builtins.hh"
#include "../libcp-bool/GeBoolProp-builtins.hh"

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param _t Mozart domain specification
 */
#define	DECLARE_INT_SET(_t,ds)					\
  OZ_Term l = (OZ_isCons(_t) ? _t : OZ_cons(_t, OZ_nil()));	\
  int length = OZ_length(l);					\
  int _pairs[length][2];					\
								\
  for (int i = 0; OZ_isCons(l); l=OZ_tail(l), i++) {		\
    OZ_Term _val = OZ_head(l);					\
    if (OZ_isInt(_val)) {					\
      _pairs[i][0] = OZ_intToC(_val);				\
      _pairs[i][1] = OZ_intToC(_val);				\
    }								\
    else if (OZ_isTuple(_val)) {				\
      _pairs[i][0] = OZ_intToC(OZ_getArg(_val,0));		\
      _pairs[i][1] = OZ_intToC(OZ_getArg(_val,1));		\
    }								\
    else {							\
      return OZ_typeError(0,"domain type unknown");		\
    }								\
  }								\
  Gecode::IntSet ds(_pairs, length);


/**
	Declares a veriable form OZ_Term argument at position p in the input to be an IntVar 
	variable. This declaration does not affect space stability so it can not be used in 
	propagator's built ins.
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
	Macros for variable declaration inside propagators posting
	built-ins. Space stability is affected as a side effect.
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
	Declares a GeIntVar inside a var array. Space stability is affected as a side effect. 
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
/*
#define DeclareBool(p, v) \
bool v;\
declareInTerm(p,v##x);\
if (!OZ_isBool(v##x))\
	   return OZ_typeError(p,"atom");		\
  v = OZ_isTrue(v##x) ? true : false; \
}
*/
#define DECLARE_INTVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,IntVarArgs,DeclareGeIntVarVA)

inline
Gecode::DFA::Transition TransitionS(TaggedRef tm) {
  Gecode::DFA::Transition t;
  t.i_state = OZ_getArg(tm,0);
  t.o_state = OZ_getArg(tm,1);
  t.symbol  = OZ_getArg(tm,2);
  return t;
}

/*
inline
Gecode::Support::DynamicArray<Gecode::DFA::Transition> TransitionL(TaggedRef tr) {
//Gecode::DFA::Transition TransitionL(TaggedRef tr) {
//void TransitionL(TaggedRef tr) {
  int size = OZ_width(tr);
  Gecode::DFA::Transition tl[size];
  Gecode::Support::DynamicArray<Gecode::DFA::Transition> tl2(size);
  OZ_Term al = OZ_arityList(tr);
  for(int i=0; OZ_isCons(al); al=OZ_tail(al)) {
    tl[i] = TransitionS(OZ_subtree(tr,OZ_head(al)));
  }
  return tl2;
  } */
// tr new Transition array
// tl Mozart Descripcion of the DFA (TaggedRef)
#define DeclareDFA(tr, tl)				\
  Gecode::DFA::Transition tl[OZ_width(tr)];		\
  {							\
    OZ_Term al = OZ_arityList(tr);			\
    for(int i=0; OZ_isCons(al); al=OZ_tail(al)) {	\
      tl[i] = TransitionS(OZ_subtree(tr, OZ_head(al))); \
    }							\
  }


#endif
