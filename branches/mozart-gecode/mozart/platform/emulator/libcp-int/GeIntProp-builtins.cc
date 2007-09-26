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

#ifndef __GEOZ_INT_PROP_BUILTINS_CC__
#define __GEOZ_INT_PROP_BUILTINS_CC__

#include "GeIntProp-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;

OZ_BI_define(int_eq,3,0) 
{
  DeclareGSpace(gs);
  
  DeclareInt(2,consistencyLevel,"The last value must be the consistency level: eqProp");
  
  DeclareGeIntVar(0,v1,gs);
  DeclareGeIntVar(1,v2,gs);
  
  try{
    rel(gs,v1,IRT_EQ,v2,(IntConLevel)consistencyLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(int_rel,4,0) {
  DeclareGSpace(gs);
  DeclareIntConLevel(3,consistencyLevel);
  RelType(1,relType);

  DeclareGeIntVar(0,v1,gs);
  DeclareGeIntVar(2,v2,gs);
  try {
    rel(gs,v1,relType,v2,consistencyLevel);
  } catch (Exception e) {
    RAISE_GE_EXCEPTION(e);
  }  
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(int_dist,2,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTVARARRAY(0,var,sp);   
   DeclareInt(1,conLevel,"The last value must be the consistency level: distProp");
   try {
     distinct(sp,var,(IntConLevel)conLevel);
   } catch(Exception e){
     RAISE_GE_EXCEPTION(e);
   }
   CHECK_POST(sp);
 } OZ_BI_end


 OZ_BI_define(int_dist2,3,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTARGS(0,arguments);
   DECLARE_INTVARARRAY(1,var,sp);
   DeclareInt(2,conLevel,"The last value must be the consistency level: distProp");
   try {
     distinct(sp,arguments,var,(IntConLevel)conLevel);
     unsigned int a;
     CHECK_POST(sp);
   } catch(Exception e){
     RAISE_GE_EXCEPTION(e);
   }
   
 } OZ_BI_end



// //Propagador lineal simple
// //La suma de los elementos del arreglo(x1) debe ser igual a (x3 int o IntVar)
 OZ_BI_define(int_linear,4,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTVARARRAY(0,x1,sp);
   RelType(1,x2);
   DeclareIntConLevel(3,x4);	   
   if(!OZ_isInt(OZ_deref(OZ_in(2)))) {
     try{
       linear(sp,x1,x2,get_IntVar(OZ_in(2)),x4);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   }
   else{
     OZ_declareInt(2,x3);
     try{
       linear(sp,x1,x2,x3,x4);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   } 
   CHECK_POST(sp);       
 } OZ_BI_end
 
// //Propagador un poco linear un poco mas complicado
 OZ_BI_define(int_linear2,5,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTARGS(0,arguments);
   DECLARE_INTVARARRAY(1,array_var,sp);
   RelType(2,rel);
   DeclareIntConLevel(4,con);
   if(!OZ_isInt(OZ_deref(OZ_in(3)))) {
     try{
       linear(sp,arguments,array_var,rel,get_IntVar(OZ_in(3)),con);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   }
   else {
     OZ_declareInt(3,val);
     try{
       linear(sp,arguments,array_var,rel,val,con);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   }
   CHECK_POST(sp);
 } OZ_BI_end
 
OZ_BI_define(int_linearR,5,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(0,array_var,sp);

  RelType(1,relType);
  DeclareIntConLevel(4,conLevel);

  DeclareGeIntVar(2,v2,sp);
  DeclareGeBoolVar(3,v3,sp);

  try{
    linear(sp,array_var,relType,v2,v3,conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
    
} OZ_BI_end


OZ_BI_define(int_linearCR,6,0)
{

  DeclareGSpace(sp);
  DECLARE_INTARGS(0,array_arg);
  DECLARE_INTVARARRAY(1,array_var,sp);
  
  RelType(2,relType);
  DeclareIntConLevel(5,conLevel);
  
  DeclareGeIntVar(3,v3,sp);
  DeclareGeBoolVar(4,v4,sp);

  try{
    linear(sp,array_arg,array_var,relType,v3,v4,conLevel);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end


OZ_BI_define(int_count,5,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(0,arreglo,sp);
  IntVar x1,x3;
  RelType(2,rl);
  DeclareIntConLevel(4,cl);
  
  if(OZ_isGeIntVar(OZ_deref(OZ_in(1))))
    x1 = get_IntVar(OZ_in(1));
  else if (OZ_isInt(OZ_in(1))) {
    OZ_declareInt(1,valTmp);
    IntVar tmp(sp,valTmp,valTmp);
    x1 = tmp;
  }
  else
    RAISE_EXCEPTION("The second argument must be either Int or GeIntVar");
  
  if(OZ_isGeIntVar(OZ_deref(OZ_in(3))))
    x3 = get_IntVar(OZ_in(3));
  else if (OZ_isInt(OZ_in(3))) {
    OZ_declareInt(3,valTmp);
    IntVar tmp(sp,valTmp,valTmp);
    x3 = tmp;
  }
  else
    RAISE_EXCEPTION("The threeth argument must be either Int or GeIntVar");
  try {
    count(sp,arreglo,x1,rl,x3,cl);
  }
  catch(Exception e) {
   RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end

OZ_BI_define(int_mult,3,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v1,sp);
  DeclareGeIntVar(1,v2,sp);
  DeclareGeIntVar(2,v3,sp);
  try{
    mult(sp,v1,v2,v3);
  } catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end


OZ_BI_define(int_Gabs,3,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  OZ_declareInt(2,conLevel);
  try{
    Gecode::abs(sp,v0,v1,(IntConLevel)conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
    
} OZ_BI_end

/*
OZ_BI_define(bool_and,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v1,sp);
  DeclareGeIntVar(1,v2,sp);
  DeclareGeIntVar(2,v3,sp);
  OZ_declareInt(3,ConLevel);
 
  try{
    mult(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end
*/
OZ_BI_define(int_sortedness,3,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(0,a1,sp);
  DECLARE_INTVARARRAY(1,a2,sp);
  OZ_declareInt(2,conLevel);

  try{
    sorted(sp,a1,a2,(IntConLevel)conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end

// Include the propagators wrapper functions generated automatically.
//#include "FDPBuiltins.cc"

#endif
