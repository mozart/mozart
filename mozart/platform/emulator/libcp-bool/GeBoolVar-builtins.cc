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
 * @param 0 The space
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_boolvar,1,1)
{
  DECLARE_BOOL_SET(dom,0);   // the domain of the BoolVar
  OZ_RETURN(new_GeBoolVar(dom));
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
 * \brief Returns the minimum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The minimum of the domain 
 */
OZ_BI_define(boolvar_getMin,1,1)
{
  DeclareGeBoolVar1(0,v);
  OZ_RETURN_INT(v.min());
}
OZ_BI_end

/** 
 * \brief Returns the maximum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The maximum of the domain 
 */
OZ_BI_define(boolvar_getMax,1,1)
{
  DeclareGeBoolVar1(0,v);
  OZ_RETURN_INT(v.max());
}
OZ_BI_end



#endif
