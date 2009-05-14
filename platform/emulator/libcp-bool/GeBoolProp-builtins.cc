/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
 *    Andres Felipe Barco <anfelbar@univalle.edu.co>
 *    Victor Rivera Zuniga <varivera@javerianacali.edu.co>
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

#ifndef __GEOZ_BOOL_PROP_BUILTINS_CC__
#define __GEOZ_BOOL_PROP_BUILTINS_CC__

#include "BoolVarMacros.hh"


using namespace Gecode;
using namespace Gecode::Int;

OZ_BI_define(bool_rel_BV_BT_BV_BV,4,0)
{
  DeclareGSpace(sp);
  BoolVar *v1 = boolOrBoolVar(OZ_in(0));
  DeclareBoolOpType(1,bt);
  BoolVar *v2 = boolOrBoolVar(OZ_in(2));
  BoolVar *v3 = boolOrBoolVar(OZ_in(3));
 
  try{
    rel(sp,*v1,bt,*v2,*v3);
    delete v1, v2, v3;
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  
  CHECK_POST(sp);

} OZ_BI_end

OZ_BI_define(bool_not,2,0)
{
  DeclareGSpace(sp);
  BoolVar *v1 = boolOrBoolVar(OZ_in(0));
  BoolVar *v2 = boolOrBoolVar(OZ_in(1));
  try{
    rel(sp,*v1,IRT_NQ,*v2);
    delete v1, v2;
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end


#ifndef MODULES_LINK_STATIC
#include "../modGBDP-if.cc"
#endif

#endif
