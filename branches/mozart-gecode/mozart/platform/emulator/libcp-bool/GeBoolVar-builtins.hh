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

#define DeclareGeBoolVar2(p,v,sp)					\
  IntVar v;                                                             \
  if(OZ_isInt(OZ_in(p))) {						\
    OZ_declareInt(p,domain);						\
    IntVar _tmp(sp,domain,domain);					\
    v=_tmp;								\
  }									\
  else if(OZ_isGeBoolVar(OZ_in(p))) {					\
    v = get_BoolVar(OZ_in(p));						\   
  }									\
  else RAISE_EXCEPTION("The variables must be either GeIntVar or int");

/*
  This macro declares a variable without comprising space stability.
  It must be used from functions that no require further propagation.
*/
#define DeclareGeBoolVar1(p,v)					\
  BoolVar v;							\
  { TaggedRef x = OZ_in(p);					\
    DEREF(x,x_ptr);						\
    Assert(!oz_isRef(x));					\
    if (oz_isFree(x)) {						\
      oz_suspendOn(makeTaggedRef(x_ptr));			\
    }								\
    if (OZ_isInt(x)) {						\
      OZ_declareInt(p,domain);					\
      v.init(oz_currentBoard()->getGenericSpace(),		\
		  domain, domain);				\
    }								\
    else if(OZ_isGeBoolVar(x)) {					\
      v = get_BoolVarInfo(x);					\
    } else							\
      RAISE_EXCEPTION("Type error: Expected IntVar");		\
  }

#define DeclareGeBoolVar(p,v,sp)					        \
  {  TaggedRef x = OZ_in(p);						\
    DEREF(x,x_ptr);							\
    Assert(!oz_isRef(x));						\
    if (oz_isFree(x)) {							\
      oz_suspendOn(makeTaggedRef(x_ptr));				\
    }}									\
  BoolVar v;								\
  if(OZ_isInt(OZ_in(p))) {						\
    OZ_declareInt(p,domain);						\
    v.init(sp,domain,domain);					\
  }									\
  else if(OZ_isGeBoolVar(OZ_in(p))) {					\
    v = get_BoolVar(OZ_in(p));						\
  }									\
  else RAISE_EXCEPTION("The variables must be either GeBoolVar or int");


#define DeclareGeBoolVarT2(val,ar,i)				\
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
  else if(OZ_isGeBoolVar(val)) {				\
      ar[i]=static_cast<Gecode::BoolVar>(get_BoolVar(val));	\
  }								\
}

#define DECLARE_BOOLVARARRAY(sp,array,tIn)  		\
BoolVarArray array;					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::BoolVarArray _array((Gecode::Space*)sp,sz);	\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::BoolVarArray _array((Gecode::Space*)sp,sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeBoolVarT2(OZ_deref(OZ_head(t)),_array,i);\
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::BoolVarArray _array((Gecode::Space*)sp,sz);	\
    for(int i=0;i<sz;i++) {				\
      DeclareGeBoolVarT2(OZ_getArg(t,i),_array,i);	\
    }							\
    array=_array;                                      \
  }							\
  else {						\
    assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::BoolVarArray _array((Gecode::Space*)sp,sz); \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeBoolVarT2(OZ_subtree(t,OZ_head(al)),_array,i);\
    }							 \
    array=_array;                                       \
    }		                                         \
}

#endif
