#ifndef __GEOZ_SPACE_BUILTINS_HH__
#define __GEOZ_SPACE_BUILTINS_HH__

#include "var_ext.hh"
//#include "mozart.h"
#include "misc.hh"

#include "GeSpace.hh"
/**
   \def OZ_declareGeSpace(ARG,VAR)
   \brief Declares \a VAR to be the GeSpace referenced by <code>OZ_in(ARG)</code>
*/ 
//#define OZ_declareGeSpace(ARG,VAR) \
//OZ_declareType(ARG,VAR,GeSpace*,"gspace",OZ_isGeSpace,OZ_GeSpaceToC)
//#define OZ_declareGenericSpace(arg,sp)\


/**
   \def CHECK_SPACE(s)
   \brief Test if \a s points to something different from \a NULL
   \attention This macro can be called by the <code>DECLARE_SPACE</code> macro only.
*/ 
#define CHECK_SPACE(s)  \
	if (!s) OZ_RETURN(OZ_raiseC("InvalidSpace",0))

/**
   \def DECLARE_SPACE(s, arg)
   \brief Declares \a s to be the GeSpace referenced by <code>OZ_in(arg)</code>
   \attention The order of the parameters should be swaped to be compliant with the Mozart standar
*/
#define DECLARE_SPACE(s, arg)  \
        GenericSpace *s = extVar2Var(arg)->getBoardInternal->getGenericSpace();   \
	CHECK_SPACE(s) 

	
#define DeclareGSpace(sp) \
	GenericSpace *sp = oz_currentBoard()->getGenericSpace()

	
/*
  Some propagators are "run" on posting (e.g. <) and no wait 
  for a status method call. The following macro is intended for
  early failure detection and checks if the space sp becomes failed
  after propoagator posting.
*/
#define CHECK_POST(sp)                  \
  if(sp->failed())                     \
return FAILED;                     \
else                                 \
if(sp->isStable())                 \
sp->makeUnstable();              \
return PROCEED;

/* 
	Some useful macros to declare VarArgs from OZ_terms (list, records, tuples)
	type must be IntVarArgs or BoolVarargs. op is the name of a function, macro to
	declare one variable of the corresponding array type.
*/

#define DECLARE_VARARGS(tIn,array,sp,type,opDecl)  		\
int __x##tIn = 0; \
{ \
	OZ_Term t = OZ_deref(OZ_in(tIn));                     \
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
}
 
#endif
