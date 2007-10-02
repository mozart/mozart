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
#include "../libcp-bool/GeBoolVar-builtins.hh"

/** 
 * \brief Declares a Gecode::Int::IntSet from an Oz domain description
 * 
 * @param ds The domain description int terms of list and tuples
 * @param arg Position at OZ_in array
 */
#define	DECLARE_INT_SET(arg,ds)					\
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

/*
  This macro declares a variable without comprising space stability.
  It must be used from functions that no require further propagation.
*/
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
    IntVar _tmp( (sp) ,domain,domain);					\
    v=_tmp;								\
  }									\
  else if(OZ_isGeIntVar(OZ_in(p))) {					\
    v = get_IntVar(OZ_in(p));						\
  }									\
  else RAISE_EXCEPTION("The variables must be either GeIntVar or int");


#define DeclareGeIntVarT2(val,ar,i,sp)				\
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

#define DeclareGeIntVarT3(val,ar,i,sp)				\
{  TaggedRef x = val;						\
  DEREF(x,x_ptr);						\
  Assert(!oz_isRef(x));						\
  if (oz_isFree(x)) {						\
    oz_suspendOn(makeTaggedRef(x_ptr));				\
  }								\
  if(OZ_isInt(val)) {						\
    int domain=OZ_intToC(val);					\
	Gecode::IntVar v(sp,domain,domain);\
	ar[i] = v;				\
  }								\
  else if(OZ_isGeIntVar(val)) {					\
          ar[i]=get_IntVar(val);					\
  }								\
}

#define DeclareBool(p, v) \
bool v;\
{  TaggedRef x = OZ_in(p);						\
  DEREF(x,x_ptr);						\
  Assert(!oz_isRef(x));						\
  if (oz_isFree(x)) {						\
    oz_suspendOn(makeTaggedRef(x_ptr));				\
  }\
  if (!OZ_isBool(v))\
	   RAISE_EXCEPTION("Error, expected atom");	\
  v = OZ_isTrue(x) ? true : false; \
}


#define DECLARE_INTVARARRAY(tIn,array,sp)  		\
IntVarArray array;					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::IntVarArray _array((Gecode::Space*) sp ,sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::IntVarArray _array((Gecode::Space*) sp ,sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeIntVarT2(OZ_deref(OZ_head(t)),_array,i,sp); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::IntVarArray _array((Gecode::Space*) sp ,sz);	\
    for(int i=0;i<sz;i++) {				\
      DeclareGeIntVarT2(OZ_getArg(t,i),_array,i,sp);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::IntVarArray _array((Gecode::Space*) sp ,sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeIntVarT2(OZ_subtree(t,OZ_head(al)),_array,i,sp);\
    }							\
    array=_array;                                       \
    }							\
}

#define DECLARE_INTVARARGS(tIn,array,sp)  		\
int __x##tIn = 0; \
{ \
	OZ_Term t = OZ_deref(OZ_in(tIn));                     \
	__x##tIn =  OZ_isLiteral(t) ? 0 : OZ_isCons(t) ? OZ_length(t) : OZ_width(t); \
} \
IntVarArgs array(__x##tIn);					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::IntVarArgs _array(sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::IntVarArgs _array(sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeIntVarT3(OZ_deref(OZ_head(t)),_array,i,sp); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::IntVarArgs _array(sz);	\
    for(int i=0;i<sz;i++) {				\
      DeclareGeIntVarT3(OZ_getArg(t,i),_array,i,sp);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    Assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::IntVarArgs _array(sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeIntVarT3(OZ_subtree(t,OZ_head(al)),_array,i,sp);\
    }							\
    array=_array;                                       \
    }							\
}

#define DECLARE_BOOLVARARGS(tIn,array,sp)  		\
int __x##tIn = 0; \
{ \
	OZ_Term t = OZ_deref(OZ_in(tIn));                     \
	__x##tIn =  OZ_isLiteral(t) ? 0 : OZ_isCons(t) ? OZ_length(t) : OZ_width(t); \
} \
BoolVarArgs array(__x##tIn);					\
{							\
  int sz;						\
  OZ_Term t = OZ_deref(OZ_in(tIn));                     \
  if(OZ_isLiteral(t)) {					\
    sz=0;						\
    Gecode::BoolVarArgs _array(sz);		\
    array=_array;					\
  }							\
  else if(OZ_isCons(t)) {				\
    sz = OZ_length(t);					\
    Gecode::BoolVarArgs _array(sz);	\
    for(int i=0; OZ_isCons(t); t=OZ_tail(t),i++){	\
      DeclareGeIntVarT3(OZ_deref(OZ_head(t)),_array,i,sp); \
    }                                                   \
    array=_array;					\
  }							\
  else if(OZ_isTuple(t)) {				\
    sz=OZ_width(t);					\
    Gecode::BoolVarArgs _array(sz);	\
    for(int i=0;i<sz;i++) {				\
      DeclareGeIntVarT3(OZ_getArg(t,i),_array,i,sp);	\
    }							\
    array=_array;                                       \
  }							\
  else {						\
    Assert(OZ_isRecord(t));				\
    OZ_Term al = OZ_arityList(t);			\
    sz = OZ_width(t);					\
    Gecode::BoolVarArgs _array(sz);          \
    for(int i=0; OZ_isCons(al); al=OZ_tail(al),i++) {	\
      DeclareGeIntVarT3(OZ_subtree(t,OZ_head(al)),_array,i,sp);\
    }							\
    array=_array;                                       \
    }							\
}

#endif
