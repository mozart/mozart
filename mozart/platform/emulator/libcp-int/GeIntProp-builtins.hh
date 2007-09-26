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

#ifndef __GEOZ_INT_PROP_BUILTINS_HH__
#define __GEOZ_INT_PROP_BUILTINS_HH__



#include "GeIntVar-builtins.hh"
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

#define OZ_TOC(arg,type,var,check,conv,msg) \
	type var;                                                      \
	{\
		if (!check(OZ_in(arg)))                                 \
			RAISE_EXCEPTION(msg);                     \
		var=conv(OZ_in(arg));\
	}

#define DeclareInt(arg,var,msg) \
	OZ_TOC(arg,int,var,OZ_isInt,OZ_intToC,msg)

#define ConLevel(arg,var) \
	IntConLevel var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected consistency level"); \
		var = (IntConLevel)__vv;\
	}
#endif

#define RelType(arg,var) \
	IntRelType var;\
	{\
		OZ_TOC(arg,int,__vv,OZ_isInt,OZ_intToC,"Expected relation type") ;\
		var = (IntRelType)__vv;\
	}

