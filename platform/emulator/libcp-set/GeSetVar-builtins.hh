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
#include "SetValue.hh"
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

/**
	Macros for variable declaration inside propagators posting
	built-ins. Space stability is affected as a side effect.
*/
#define DeclareGeSetVar(p,v,sp)\
  declareInTerm(p,v##x);\
  SetVar v;\
  {\
  if(SetValueM::OZ_isSetValueM(v##x)) {\    
    SetVar _tmp( (sp) ,SetValueM::tagged2SetVal(v##x)->getLBValue(),SetValueM::tagged2SetVal(v##x)->getLBValue());\
    v=_tmp;\
  }\
  else if(OZ_isGeSetVar(v##x)) {\
    v = get_SetVar(OZ_in(p));\
  }\
  else return OZ_typeError(p,"SetVar or SetValue");\
  }

/**
	Declares a GeSetVar inside a var array. Space stability is affected as a side effect. 
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

#define DECLARE_SETVARARGS(tIn,array,sp) DECLARE_VARARGS(tIn,array,sp,SetVarArgs,DeclareGeSetVarVA)

#endif
