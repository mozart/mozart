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

#ifndef __GEOZ_SETVAR_BUILTINS_HH__
#define __GEOZ_SETVAR_BUILTINS_HH__

#include "value.hh"
#include "GeSetVar.hh"
#include "GeSpace-builtins.hh"
#include "builtins.hh"

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param arg Position at OZ_in array
 */
#define	DECLARE_INT_SET(ds,val,arg)				           \
  IntSet *tmp##val;                                                        \
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


#define OZ_TOC(arg,type,var,check,conv,msg) \
	type var;                                                      \
	{\
		if (!check(OZ_in(arg)))                                 \
			return OZ_typeError(arg,msg);                     \
		var=conv(OZ_in(arg));\
	}


#define DeclareInt(arg,var,msg) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,msg)
/*
#define DeclareGSpace(sp) GenericSpace *sp = oz_currentBoard()->getGenericSpace();

#define DeclareGeIntVar2(p,v,sp)					\
  IntVar v;                                                             \
  if(OZ_isInt(OZ_in(p))) {						\
    OZ_declareInt(p,domain);						\
    IntVar _tmp(sp,domain,domain);					\
    v=_tmp;								\
  }									\
  else if(OZ_isGeIntVar(OZ_in(p))) {					\
    v = get_IntVar(OZ_in(p));						\   
  }									\
  else RAISE_EXCEPTION("The variables must be either GeIntVar or int");


#define DeclareGeIntVar1(p,v)					\
  IntVar v;							\
  { TaggedRef x = OZ_in(p);					\
    DEREF(x,x_ptr);						\
    Assert(!oz_isRef(x));					\
    if (oz_isFree(x)) {						\
      oz_suspendOn(makeTaggedRef(x_ptr));			\
    }								\
    if (OZ_isInt(x)) {						\
      OZ_declareInt(p,domain);					\
      IntVar _tmp(oz_currentBoard()->getGenericSpace(),		\
		  domain, domain);				\
      v=_tmp;							\
    }								\
    else if(OZ_isGeIntVar(x)) {					\
      v = get_IntVarInfo(x);					\
    } else							\
      RAISE_EXCEPTION("Type error: Expected IntVar");		\
  }

#define DeclareGeIntVar(p,v,sp)					        \
  {  TaggedRef x = OZ_in(p);						\
    DEREF(x,x_ptr);							\
    Assert(!oz_isRef(x));						\
    if (oz_isFree(x)) {							\
      oz_suspendOn(makeTaggedRef(x_ptr));				\
    }}									\
  IntVar v;								\
  if(OZ_isInt(OZ_in(p))) {						\
    OZ_declareInt(p,domain);						\
    IntVar _tmp(sp,domain,domain);					\
    v=_tmp;								\
  }									\
  else if(OZ_isGeIntVar(OZ_in(p))) {					\
    v = get_IntVar(OZ_in(p));						\
  }									\
  else RAISE_EXCEPTION("The variables must be either GeIntVar or int");


#define DeclareGeIntVarT2(val,ar,i)				\
{  TaggedRef x = val;						\
  DEREF(x,x_ptr);						\
  Assert(!oz_isRef(x));						\
  if (oz_isFree(x)) {						\
    oz_suspendOn(makeTaggedRef(x_ptr));				\
  }								\
  if(OZ_isInt(val)) {						\
    int domain=OZ_intToC(val);					\
    ar[i].init(sp,domain,domain);				\
  }								\
  else if(OZ_isGeIntVar(val)) {					\
          ar[i]=get_IntVar(val);					\
  }								\
}

#define DECLARE_INTVARARRAY(sp,array,tIn)  		\
IntVarArray array;					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
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
*/
#endif
