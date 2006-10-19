/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *     Raphael Collet <raph@info.ucl.ac.be>
 *
 *  Contributing authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GEOZ_INTVAR_BUILTINS_HH__
#define __GEOZ_INTVAR_BUILTINS_HH__

#include "value.hh"
#include "GeIntVar.hh"
#include "GeSpace-builtins.hh"

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param arg Position at OZ_in array
 */
#define	DECLARE_INT_SET(ds,arg)					\
  OZ_declareDetTerm(arg,_t);					\
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
      RAISE_EXCEPTION("Error, domain type unknown");		\
    }								\
  }								\
  Gecode::IntSet ds(_pairs, length);


/** 
 * Declares an array from an Oz list
 * 
 * @param array The declared array
 * @param t Position at the input array
 */
#define DECLARE_INTVARARRAY1(array,t)						\
  OZ_Term _termTmp = OZ_deref(OZ_in(t));					\
  int __length=1;								\
  __length=OZ_length(_termTmp);							\
  IntVarArray array;								\
  GenericSpace *sp;								\
  if(__length != -1 ){								\
    /*The next loop  will be replaced by OZ_currentGenericSpace*/		\
    while((OZ_isCons(_termTmp))&&!OZ_isGeIntVar(OZ_deref(OZ_head(_termTmp))))	\
      _termTmp = OZ_tail(_termTmp);						\
    if(!OZ_isCons(_termTmp))							\
      RAISE_EXCEPTION("you must have at least one GeIntVar");			\
    sp = extVar2Var(get_GeIntVar(OZ_deref(OZ_head(_termTmp))))->getBoardInternal()->getGenericSpace();				\
    IntVarArray __tmp((Gecode::Space*)sp,__length);				\
    _termTmp = OZ_deref(OZ_in(t));						\
    for(int i = 0; OZ_isCons(_termTmp); _termTmp=OZ_tail(_termTmp),i++)		\
      {										\
  	OZ_Term val = OZ_deref(OZ_head(_termTmp));				\
        if(OZ_isInt(val)){							\
	  int domain=OZ_intToC(val);						\
	  __tmp[i].init(sp,domain,domain);					\
        }									\
	else if(sp!=extVar2Var(get_GeIntVar(val))->getBoardInternal()->getGenericSpace()){						\
	  RAISE_EXCEPTION("The variables are in different Spaces");		\
	}									\
	else if(OZ_isGeIntVar(OZ_deref(OZ_head(_termTmp)))){			\
	  __tmp[i]=get_IntVar(val);						\
	  }else{								\
	  RAISE_EXCEPTION("The variables have to be either GeIntVar or Int");	\
	}									\
      }										\
    array = __tmp;								\
  }

#define DeclareGSpace(sp) GenericSpace *sp = oz_currentBoard()->getGenericSpace();
#define get_Space(t) extVar2Var(get_GeIntVar(t))->getBoardInternal()->getGenericSpace()

#define DeclareGeIntVar(p,v,sp)					        \
  IntVar v;                                                             \
  if(OZ_isInt(OZ_in(p))) {						\
    OZ_declareInt(p,domain);						\
    IntVar _tmp(sp,domain,domain);					\
    v=_tmp;								\
  }									\
  else if(OZ_isGeIntVar(OZ_in(p))) {					\
    v = get_IntVar(OZ_in(p));						\
    /*if(sp != get_Space(OZ_in(p)))					\
      RAISE_EXCEPTION("The variables are in differents spaces");*/	\
  }									\
  else RAISE_EXCEPTION("The variables must be either GeIntVar or int");


#define DeclareGeIntVarT2(val,ar,i)					\
{									\
  /*OZ_Term val = OZ_deref(OZ_head(t));*/				\
  if(OZ_isInt(val)) {							\
    int domain=OZ_intToC(val);						\
    ar[i].init(sp,domain,domain);					\
  }									\
  /*else if(sp!= get_Space(val)){}					\
    RAISE_EXCEPTION("The variables are not in the same space");  */     	\
  if(OZ_isGeIntVar(val)) 						\
      ar[i]=get_IntVar(val);						\
  /*_array[i++] = _tmp;*/						\
}

#define DECLARE_INTVARARRAY(sp,array,tIn)  		\
/*DeclareGSpace(sp);*/                                  \
IntVarArray array;					\
{							\
  int sz;						\
  /*OZ_declareTerm(tInt,t);*/				\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    IntVarArray _array((Gecode::Space*)sp,sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    IntVarArray _array((Gecode::Space*)sp,sz);		\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeIntVarT2(OZ_deref(OZ_head(t)),_array,i); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    IntVarArray _array((Gecode::Space*)sp,sz);		\
    for(int i=0;i<sz;i++) {				\
      DeclareGeIntVarT2(OZ_getArg(t,i),_array,i);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    IntVarArray _array((Gecode::Space*)sp,sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeIntVarT2(OZ_subtree(t,OZ_head(al)),_array,i);\
    }							\
    array=_array;                                       \
    }							\
}


#endif
