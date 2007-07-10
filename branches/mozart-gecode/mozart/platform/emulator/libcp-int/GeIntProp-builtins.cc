/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
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

#ifndef __GEOZ_INT_PROP_BUILTINS_CC__
#define __GEOZ_INT_PROP_BUILTINS_CC__

#include "GeIntProp-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;

OZ_BI_define(int_eq,3,0) 
{
  DeclareGSpace(gs);
  int consistencyLevel, relType;
  
  if(!OZ_isInt(OZ_in(2))) RAISE_EXCEPTION("The last variable must be the consistency level: ");
  consistencyLevel = OZ_intToC(OZ_in(2));
  
  DeclareGeIntVar(0,v1,gs);
  DeclareGeIntVar(1,v2,gs);
  
  try{
    eq(gs,v1,v2,(IntConLevel)consistencyLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(gs);
} OZ_BI_end

OZ_BI_define(int_rel,4,0) {
  DeclareGSpace(gs);

  int consistencyLevel,relType; 
  if(!OZ_isInt(OZ_in(3))) {
    RAISE_EXCEPTION("The last value must be the consistency level: relProp");
  }
  consistencyLevel=OZ_intToC(OZ_in(3));
  
  if(!OZ_isInt(OZ_in(1))) {
    RAISE_EXCEPTION("The second argument has to be a Relation Type: relProp");
  }
  relType=OZ_intToC(OZ_in(1));
  DeclareGeIntVar(0,v1,gs);
  DeclareGeIntVar(2,v2,gs);
  try {
    rel(gs,v1,(IntRelType)relType,v2,(IntConLevel)consistencyLevel);
  } catch (Exception e) {
    RAISE_GE_EXCEPTION(e);
  }  
  GZ_RETURN(gs);
} OZ_BI_end


OZ_BI_define(int_dist,2,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTVARARRAY(sp,var,0);   
   OZ_declareInt(1,conlevel);
   try {
     distinct(sp,var,(IntConLevel)conlevel);
     unsigned int a;
     GZ_RETURN(sp);
   } catch(Exception e){
     RAISE_GE_EXCEPTION(e);
   }
   
 } OZ_BI_end


 OZ_BI_define(int_dist2,3,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTARGS(arguments,0);
   DECLARE_INTVARARRAY(sp,var,1);
   OZ_declareInt(2,conlevel);
   try {
     distinct(sp,arguments,var,(IntConLevel)conlevel);
     unsigned int a;
     GZ_RETURN(sp);
   } catch(Exception e){
     RAISE_GE_EXCEPTION(e);
   }
   
 } OZ_BI_end



// //Propagador lineal simple
// //La suma de los elementos del arreglo(x1) debe ser igual a (x3 int o IntVar)
 OZ_BI_define(int_linear,4,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTVARARRAY(sp,x1,0);
   OZ_declareInt(1,x2);
   OZ_declareInt(3,x4);	   
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
   GZ_RETURN(sp);       
 } OZ_BI_end
 
// //Propagador un poco linear un poco mas complicado
 OZ_BI_define(int_linear2,5,0)
 {
   DeclareGSpace(sp);
   DECLARE_INTARGS(arguments,0);
   DECLARE_INTVARARRAY(sp,array_var,1);
   OZ_declareInt(2,rel);
   OZ_declareInt(4,con);

   if(!OZ_isInt(OZ_deref(OZ_in(3)))) {
     try{
       linear(sp,arguments,array_var,(IntRelType)rel,get_IntVar(OZ_in(3)),(IntConLevel)con);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   }
   else {
     OZ_declareInt(3,val);
     try{
       linear(sp,arguments,array_var,(IntRelType)rel,val,(IntConLevel)con);
     } catch(Exception e){
       RAISE_GE_EXCEPTION(e);
     }
   }
   GZ_RETURN(sp);
 } OZ_BI_end
 
OZ_BI_define(int_linearR,5,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(sp,array_var,0);

  OZ_declareInt(1,relType);
  OZ_declareInt(4,conLevel);

  DeclareGeIntVar(2,v2,sp);
  DeclareGeIntVar(3,v3,sp);

  try{
    linear(sp,array_var,(IntRelType)relType,v2,static_cast<BoolVar>(v3),(IntConLevel)conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);
    
} OZ_BI_end


OZ_BI_define(int_linearCR,6,0)
{

  DeclareGSpace(sp);
  DECLARE_INTARGS(array_arg,0);
  DECLARE_INTVARARRAY(sp,array_var,1);
  
  OZ_declareInt(2,relType);
  OZ_declareInt(5,conLevel);
  
  DeclareGeIntVar(3,v3,sp);
  DeclareGeIntVar(4,v4,sp);

  try{
    linear(sp,array_arg,array_var,(IntRelType)relType,v3,static_cast<BoolVar>(v4),(IntConLevel)conLevel);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(int_count,5,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(sp,arreglo,0);
  IntVar x1,x3;
  OZ_declareInt(2,rl);
  OZ_declareInt(4,cl);
  
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
    count(sp,arreglo,x1,(IntRelType)rl,x3,(IntConLevel)cl);
  }
  catch(Exception e) {
   RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

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
  GZ_RETURN(sp);
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
  GZ_RETURN(sp);
    
} OZ_BI_end

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
  GZ_RETURN(sp);

} OZ_BI_end

OZ_BI_define(int_sortedness,3,0)
{
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(sp,a1,0);
  DECLARE_INTVARARRAY(sp,a2,1);
  OZ_declareInt(2,conLevel);

  try{
    sortedness(sp,a1,a2,(IntConLevel)conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);
} OZ_BI_end

#endif
