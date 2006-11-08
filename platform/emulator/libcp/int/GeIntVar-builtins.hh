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
    checkGlobalVar(OZ_in(p));                                           \
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
  else if(OZ_isGeIntVar(val)) { 						\
      checkGlobalVar(val);                                              \
      ar[i]=get_IntVar(val);						\
  /*_array[i++] = _tmp;*/ \
  } \
      }

#define DECLARE_INTVARARRAY(sp,array,tIn)  		\
/*DeclareGSpace(sp);*/                                  \
IntVarArray array;					\
{							\
  int sz;						\
  /*OZ_declareTerm(tInt,t);*/				\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  /*OZ_Term t = tIn; */   \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::IntVarArray _array((Gecode::Space*)sp,sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::IntVarArray _array((Gecode::Space*)sp,sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeIntVarT2(OZ_deref(OZ_head(t)),_array,i); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::IntVarArray _array((Gecode::Space*)sp,sz);	\
    for(int i=0;i<sz;i++) {				\
      DeclareGeIntVarT2(OZ_getArg(t,i),_array,i);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::IntVarArray _array((Gecode::Space*)sp,sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeIntVarT2(OZ_subtree(t,OZ_head(al)),_array,i);\
    }							\
    array=_array;                                       \
    }							\
}


/*void DECLARE_INTVARARRAY_PRO(GenericSpace *sp, Gecode::IntVarArray &array, OZ_Term tIn);


#define DECLARE_INTVARARRAY(sp,array,tIn) \
 Gecode::IntVarArray array; \
 DECLARE_INTVARARRAY_PRO(sp,array,oz_deref(OZ_in(tIn)));
*/

#endif
