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


#ifndef __GEOZ_BOOL_VAR_BUILTINS_CC__
#define __GEOZ_BOOL_VAR_BUILTINS_CC__

#include "GeBoolVar-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;


/** 
 * \brief Creates a new IntVar variable 
 * 
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_boolvar,1,1)
{
  int l, u;
  OZ_declareDetTerm(0,dom);
  
  if (OZ_isTuple(dom)) {
	OZ_Term lower = OZ_getArg(dom,0);
	OZ_Term upper = OZ_getArg(dom,1);
  
	if (!OZ_isInt(lower) || !OZ_isInt(upper))
		return OZ_typeError(0,"Only integer values are allowed in bool var creation.");
	l = OZ_intToC(lower);
	u = OZ_intToC(upper);
  
	// Only 0/1 values are allowed when declaring a bool var.
	if (l > 1 || l < 0 || u > 1 || u < 0) 
		return OZ_typeError(0,"Invalid value to create a bool var.");
  } else if (OZ_isInt(dom)) {
	l = OZ_intToC(dom);
	if (l > 1 || l < 0) 
		return OZ_typeError(0,"Invalid value to create a bool var.");
	u = l;
  } else {
	return OZ_typeError(0, "Unknown domain specification for a boolean variable.");
  }
  
  OZ_RETURN(new_GeBoolVar(l,u));
}
OZ_BI_end

/** 
 * \brief Test whether \a OZ_in(0) is a GeBoolVar
 * 
 */
OZ_BI_define(boolvar_is,1,1)
{
  OZ_RETURN_BOOL(OZ_isGeBoolVar(OZ_in(0)));
}
OZ_BI_end


/** 
 * \brief Returns the size of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The domain size
 */
OZ_BI_define(boolvar_getSize,1,1)
{
  DeclareGeBoolVar1(0,v);
  OZ_RETURN_INT(v.size());
}
OZ_BI_end

/** 
 * \brief Returns true if the variable has only one domain element and this is 0.
 * 
 * @param intvar_getZero
 * @param 0 A reference to the variable 
 * @param 1 The minimum of the domain 
 */
OZ_BI_define(boolvar_getZero,1,1)
{
  DeclareGeBoolVar1(0,v);
  OZ_RETURN_BOOL(v.zero());
}
OZ_BI_end

/** 
 * \brief Returns the maximum elemen in the domain
 * 
 * @param intvar_getOne
 * @param 0 A reference to the variable 
 * @param 1 The maximum of the domain 
 */
OZ_BI_define(boolvar_getOne,1,1)
{
  DeclareGeBoolVar1(0,v);
  OZ_RETURN_BOOL(v.one());
}
OZ_BI_end



#endif
