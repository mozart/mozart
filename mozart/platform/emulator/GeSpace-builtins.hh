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

#endif
