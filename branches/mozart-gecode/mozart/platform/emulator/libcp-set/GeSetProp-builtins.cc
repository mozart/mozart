/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
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

#ifndef __GEOZ_SET_PROP_BUILTINS_CC__
#define __GEOZ_SET_PROP_BUILTINS_CC__

#include "SetVarMacros.hh"

using namespace Gecode;
using namespace Gecode::Set;

OZ_BI_define(set_diff,3,0) 
{
  DeclareGSpace(gs);  
    
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));
  SetVar *v3 = setOrSetVar(OZ_in(2));
  
  try{
    rel(gs,*v1,SOT_MINUS,*v2,SRT_EQ,*v3);
    delete v1, v2, v3;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(set_inter,3,0) 
{
  DeclareGSpace(gs);  
    
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));
  SetVar *v3 = setOrSetVar(OZ_in(2));
  
  try{
    rel(gs,*v1,SOT_INTER,*v2,SRT_EQ,*v3);
    delete v1, v2, v3;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_interN,2,0) 
{
  DeclareGSpace(gs);  
  DECLARE_SETVARARGS(0,var,gs); 
  SetVar *v1 = setOrSetVar(OZ_in(1));

  try{
    rel(gs,SOT_INTER,var,*v1);
    delete v1;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_union,3,0) 
{
  DeclareGSpace(gs);  
    
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));
  SetVar *v3 = setOrSetVar(OZ_in(2));
  
  try{
    rel(gs,*v1,SOT_UNION,*v2,SRT_EQ,*v3);
    delete v1, v2, v3;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_unionN,2,0) 
{
  DeclareGSpace(gs);  
  DECLARE_SETVARARGS(0,var,gs); 
  SetVar *v1 = setOrSetVar(OZ_in(1));
  
  try{
    rel(gs,SOT_UNION,var,*v1);
    delete v1;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_subS,2,0) 
{
  DeclareGSpace(gs);  
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));
  
  try{
    rel(gs,*v1,SRT_SUB,*v2);
    delete v1, v2;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_disjoint,2,0) 
{
  DeclareGSpace(gs);  
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));

  try{
    rel(gs,*v1,SRT_DISJ,*v2);
    delete v1, v2;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(set_distinct,2,0) 
{
  DeclareGSpace(gs);  
  SetVar *v1 = setOrSetVar(OZ_in(0));
  SetVar *v2 = setOrSetVar(OZ_in(1));

  try{
    rel(gs,*v1,SRT_NQ,*v2);
    delete v1, v2;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


  /**
     Monitoring builtins.
  */

#include "SetGeMozProp.hh"

void MonitorIn(Gecode::Space *gs, Gecode::Set::SetView sv, OZ_Term stream) {
  if(gs->failed()) return;
  (void) new(gs) GeMonitorIn(gs, sv, stream);
}

void MonitorOut(Gecode::Space *gs, Gecode::Set::SetView sv, OZ_Term stream) {
  if(gs->failed()) return;
  (void) new(gs) GeMonitorOut(gs, sv, stream);
}

OZ_BI_define(gfs_monitorIn, 2, 0)
{
  DeclareGSpace(gs);
  
  if(OZ_isGeSetVar(OZ_in(0))){
    SetVar *fsvar = setOrSetVar(OZ_in(0));
    Set::SetView sv(*fsvar);
    delete fsvar;
  
    try{
      MonitorIn(gs, sv, OZ_in(1));
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    }
    CHECK_POST(gs);
  } else {
    return OZ_typeError(0, "expected finite set domain");
  }
}
OZ_BI_end

OZ_BI_define(gfs_monitorOut, 2, 0)
{
  DeclareGSpace(gs);
  if(OZ_isGeSetVar(OZ_in(0))){
    SetVar *fsvar = setOrSetVar(OZ_in(0));
    Set::SetView sv(*fsvar);
    delete fsvar;
    
    try{
      MonitorOut(gs, sv, OZ_in(1));
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    }
    CHECK_POST(gs);
  } else {
    return OZ_typeError(0, "expected finite set domain");
  }
}
OZ_BI_end

/**
   int.minN & int.maxN builtins.
*/

void IntMinN(Gecode::Space *gs, Gecode::Set::SetView sv, ViewArray<Gecode::Int::IntView> iv) {
  if(gs->failed()) return;
  (void) new(gs) IntMinNElement(gs, sv, iv);
}


OZ_BI_define(gfs_intMinN, 2, 0)
{
  DeclareGSpace(gs);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    SetVar *fsvar = setOrSetVar(OZ_in(0));
    Set::SetView sv(*fsvar);
    delete fsvar;
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(1));

    if(sv.cardMax() < iva.size()){
      //return OZ_typeError(1, "SetVar, 
                          //maximal cardinality less than array size");
      return OZ_raiseErrorC("GFS.int.minN",1,
			    OZ_atom("SetVar, maximal cardinality less than array size"));
    }


    //Gecode::IntVarArgs iva2(iva.size());
    /** first post all IntVar's to be pair-wise disctinct 
     Should be sorted in ascendig order, but i do not find the right propagator**/
    try{
      Gecode::distinct(gs, iva);
      //Gecode::sorted(gs, iva2, iva);
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    } 
    
    /** second get View and post intMinN propagator **/
    Gecode::ViewArray<Gecode::Int::IntView> va(gs,iva);
    try{
      IntMinN(gs, sv, va);
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    }
    CHECK_POST(gs);
  } else {
    return OZ_typeError(0, "expected finite set domain");
  }
}
OZ_BI_end


void IntMaxN(Gecode::Space *gs, Gecode::Set::SetView sv, ViewArray<Gecode::Int::IntView> iv) {
  if(gs->failed()) return;
  (void) new(gs) IntMaxNElement(gs, sv, iv);
}

OZ_BI_define(gfs_intMaxN, 2, 0)
{
  DeclareGSpace(gs);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    SetVar *fsvar = setOrSetVar(OZ_in(0));
    Set::SetView sv(*fsvar);
    delete fsvar;
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(1));

    if(sv.cardMax() < iva.size()){
      //return OZ_typeError(1, "SetVar, 
                          //maximal cardinality less than array size");
      return OZ_raiseErrorC("GFS.int.minN",1,
			    OZ_atom("SetVar, maximal cardinality less than array size"));
    }


    //Gecode::IntVarArgs iva2(iva.size());
    /** first post all IntVar's to be pair-wise disctinct 
     Should be sorted in ascendig order, but i do not find the right propagator**/
    try{
      Gecode::distinct(gs, iva);
      //Gecode::sorted(gs, iva2, iva);
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    } 
    
    /** second get View and post intMinN propagator **/
    Gecode::ViewArray<Gecode::Int::IntView> va(gs,iva);
    try{
      IntMaxN(gs, sv, va);
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    }
    CHECK_POST(gs);
  } else {
    return OZ_typeError(0, "expected finite set domain");
  }
}
OZ_BI_end

/**
   Reified IsIn builtin.
*/

void IsInReified(Gecode::Space *gs, Gecode::Set::SetView sv, SingletonView sin, BoolView bv){
  if(gs->failed()) return;
  (void) new(gs) ReifiedIsIn(gs, sin, sv, bv);
}


OZ_BI_define(gfs_reifiedIsIn, 3, 0)
{
  DeclareGSpace(gs);
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    SetVar *fsvar = setOrSetVar(OZ_in(1));
    BoolVar *bv = boolOrBoolVar(OZ_in(2));
    IntVar *vaa = intOrIntVar(OZ_in(0));
    IntView iv(*vaa);
    Gecode::Set::SingletonView sin(iv);
    delete vaa;
    try{
      IsInReified(gs, fsvar->var(), sin, bv->var());
      delete fsvar, bv;
    }
    catch(Exception e) {
      RAISE_GE_EXCEPTION(e);
    }
    CHECK_POST(gs);
  } else {
    return OZ_typeError(0, "expected finite set domain");
  }
}
OZ_BI_end


#ifndef MODULES_LINK_STATIC
#include "../modGFSP-if.cc"
#endif

#endif
