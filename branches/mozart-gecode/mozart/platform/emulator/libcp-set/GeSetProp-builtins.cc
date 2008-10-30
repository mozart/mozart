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
    
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  DeclareGeSetVar(2,v3,gs);
  
  try{
    rel(gs,v1,SOT_MINUS,v2,SRT_EQ,v3);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(set_inter,3,0) 
{
  DeclareGSpace(gs);  
    
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  DeclareGeSetVar(2,v3,gs);
  
  try{
    rel(gs,v1,SOT_INTER,v2,SRT_EQ,v3);
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
  DeclareGeSetVar(1,v1,gs);
  
  try{
    rel(gs,SOT_INTER,var,v1);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_union,3,0) 
{
  DeclareGSpace(gs);  
    
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  DeclareGeSetVar(2,v3,gs);
  
  try{
    rel(gs,v1,SOT_UNION,v2,SRT_EQ,v3);
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
  DeclareGeSetVar(1,v1,gs);
  
  try{
    rel(gs,SOT_UNION,var,v1);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_subS,2,0) 
{
  DeclareGSpace(gs);  
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  
  try{
    rel(gs,v1,SRT_SUB,v2);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end


OZ_BI_define(set_disjoint,2,0) 
{
  DeclareGSpace(gs);  
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  
  try{
    rel(gs,v1,SRT_DISJ,v2);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(gs);
} OZ_BI_end

OZ_BI_define(set_distinct,2,0) 
{
  DeclareGSpace(gs);  
  DeclareGeSetVar(0,v1,gs);
  DeclareGeSetVar(1,v2,gs);
  
  try{
    rel(gs,v1,SRT_NQ,v2);
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
    DeclareGeSetVar(0, fsvar, gs);
    Set::SetView sv = fsvar.var();
  
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
    DeclareGeSetVar(0, fsvar, gs);
    Set::SetView sv = fsvar.var();
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


#ifndef MODULES_LINK_STATIC
#include "../modGFSP-if.cc"
#endif

#endif
