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

#ifndef __GEOZ_BOOL_PROP_BUILTINS_CC__
#define __GEOZ_BOOL_PROP_BUILTINS_CC__

#include "GeBoolProp-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;



OZ_BI_define(bool_not,3,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(2,ConLevel); 
  try{
    bool_not(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),(IntConLevel)ConLevel);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end



OZ_BI_define(bool_eq,3,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(2,ConLevel); 
  try{
    bool_eq(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),(IntConLevel)ConLevel);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(bool_and,4,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(3,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(2)))
      bool_and(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),OZ_intToC(OZ_in(2)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(2,v3,sp);
      bool_and(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(bool_and_arr,3,0)
{
  DeclareGSpace(sp);  
  DECLARE_BOOLVARARRAY(sp,v12,0);
  BoolVarArgs v1(v12);                             
  OZ_declareInt(2,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(1)))
      bool_and(sp,v1,OZ_intToC(OZ_in(1)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(1,v2,sp);
      bool_and(sp,v1,static_cast<BoolVar>(v2),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end




OZ_BI_define(bool_or,4,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(3,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(2)))
      bool_or(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),OZ_intToC(OZ_in(2)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(2,v3,sp);
      bool_or(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(bool_or_arr,3,0)
{
  DeclareGSpace(sp);  
  DECLARE_BOOLVARARRAY(sp,v12,0);
  BoolVarArgs v1(v12);                             
  OZ_declareInt(2,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(1)))
      bool_or(sp,v1,OZ_intToC(OZ_in(1)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(1,v2,sp);
      bool_or(sp,v1,static_cast<BoolVar>(v2),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end




OZ_BI_define(bool_imp,4,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(3,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(2)))
      bool_imp(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),OZ_intToC(OZ_in(2)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(2,v3,sp);
      bool_imp(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(bool_eqv,4,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(3,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(2)))
      bool_eqv(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),OZ_intToC(OZ_in(2)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(2,v3,sp);
      bool_eqv(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end


OZ_BI_define(bool_xor,4,0)
{
  DeclareGSpace(sp);
  DeclareGeBoolVar(0,v1,sp);
  DeclareGeBoolVar(1,v2,sp);    
  OZ_declareInt(3,ConLevel);
 
  try{
    if(OZ_isInt(OZ_in(2)))
      bool_xor(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),OZ_intToC(OZ_in(2)),(IntConLevel)ConLevel);
    else{
      DeclareGeBoolVar(2,v3,sp);
      bool_xor(sp,static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),static_cast<BoolVar>(v3),(IntConLevel)ConLevel);
    }
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  GZ_RETURN(sp);

} OZ_BI_end

OZ_BI_define(bool_rel,4,0) {
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
  DeclareGeBoolVar(0,v1,gs);
  DeclareGeBoolVar(2,v2,gs);

  try {
    rel(gs,v1,(IntRelType)relType,v2,(IntConLevel)consistencyLevel);
  } catch (Exception e) {
    RAISE_GE_EXCEPTION(e);
  }  
  GZ_RETURN(gs);
} OZ_BI_end

OZ_BI_define(bool_linear,4,0)
{
  DeclareGSpace(sp);
  DECLARE_BOOLVARARRAY(sp,x1,0);
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


#endif
