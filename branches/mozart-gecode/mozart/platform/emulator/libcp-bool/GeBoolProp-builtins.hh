/*
 *  Main authors:
 *     Alejandro Arbelaez: <aarbelaez@puj.edu.co>
 *
 *
 *  Contributing authors:
 *
 *  Copyright:
 *     Alejandro Arbelaez, 2006
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

#ifndef __GEOZ_BOOL_PROP_BUILTINS_HH__
#define __GEOZ_BOOL_PROP_BUILTINS_HH__



#include "GeBoolVar-builtins.hh"
#include "../libcp-int/GeIntVar.hh"
#include "gecode/int.hh"

										

// array: the resulting IntArgs
// s: the space
// t: possition in OZ_in
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



#define DECLARE_INTARGS(array,tIn)			\
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

/*
  Some propagators are "run" on posting (e.g. <) and no wait 
  for a status method call. The following macro is intended for
  early failure detection and checks if the space sp becomes failed
  after propoagator posting.
*/
#define GZ_RETURN(sp)                  \
  if(sp->failed())                     \
return FAILED;                     \
else                                 \
if(sp->isStable())                 \
sp->makeUnstable();              \
return PROCEED;

#endif
