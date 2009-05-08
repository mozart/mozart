/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
 *    Andres Felipe Barco <anfelbar@univalle.edu.co>
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
  BoolOpType bt = getBoolOpType(OZ_in(1));
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

OZ_BI_define(bool_not,3,0)
{
  DeclareGSpace(sp);
  BoolVar *v1 = boolOrBoolVar(OZ_in(0));
  BoolVar *v2 = boolOrBoolVar(OZ_in(1));
  OZ_declareInt(2,ConLevel); 
  try{
    rel(sp,*v1,IRT_NQ,*v2,(IntConLevel)ConLevel);
    delete v1, v2;
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end


OZ_BI_define(bool_rel,4,0) {
  DeclareGSpace(gs);

  int consistencyLevel,relType; 
  if(!OZ_isInt(OZ_in(3))) {
    return OZ_typeError(3,"The value must be a consistency level");
  }
  consistencyLevel=OZ_intToC(OZ_in(3));
  
  if(!OZ_isInt(OZ_in(1))) {
    return OZ_typeError(1,"Argument has to be a Relation Type");
  }
  relType=OZ_intToC(OZ_in(1));
  BoolVar *v1 = boolOrBoolVar(OZ_in(0));
  BoolVar *v2 = boolOrBoolVar(OZ_in(2));

  /*
    try {
    rel(gs,*v1,(IntRelType)relType,*v2,(IntConLevel)consistencyLevel);
    } catch (Exception e) {
    RAISE_GE_EXCEPTION(e);
    } */
  
  delete v1, v2;
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(bool_linear,4,0)
{
  DeclareGSpace(sp);
  OZ_declareInt(1,x2);
  OZ_declareInt(3,x4);	   
  /*
    if(!OZ_isInt(OZ_deref(OZ_in(2)))) {
    try{
    linear(sp,x1,(IntRelType)x2,get_IntVar(OZ_in(2)),(IntConLevel)x4);
    } catch(Exception e){
    RAISE_GE_EXCEPTION(e);
    }
    }
    else{
    OZ_declareInt(2,x3);
    try{
    linear(sp,x1,(IntRelType)x2,x3,(IntConLevel)x4);
    } catch(Exception e){
    RAISE_GE_EXCEPTION(e);
    }
    }
  */
  CHECK_POST(sp);       
} OZ_BI_end

#ifndef MODULES_LINK_STATIC
#include "../modGBDP-if.cc"
#endif

#endif
