/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand
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

#include "base.hh"
#include "dpBase.hh"

#include "fail.hh"
#include "perdio.hh"

#include "builtins.hh"
#include "var.hh"

OZ_BI_define(BIdistHandlerInstall,2,1){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc0      = OZ_in(1);  
  
  initDP();
  NONVAR(c0, c);
  NONVAR(proc0,proc);
  
  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;
  
  Bool suc;
  OZ_Return ret=DistHandlerInstall(condStruct,proc,suc);
  if(ret!=PROCEED) return ret;
  OZ_RETURN(oz_bool(suc));
}OZ_BI_end

OZ_BI_define(BIdistHandlerDeInstall,2,1){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc0      = OZ_in(1);  
  
  initDP();
  NONVAR(c0, c);
  NONVAR(proc0,proc);

  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;

  Bool suc;
  OZ_Return ret=DistHandlerDeInstall(condStruct,proc,suc);
  if(ret!=PROCEED) return ret;
  OZ_RETURN(oz_bool(suc));
}OZ_BI_end

OZ_BI_define(BIgetEntityCond,2,0){
  OZ_Term e0 = OZ_in(0);
  OZ_Term v0 = OZ_in(1);

  initDP();  
  EntityCond ec=ENTITY_NORMAL;

  DEREF(e0,vs_ptr);
  Assert(!oz_isRef(e0));
  if(oz_isVarOrRef(e0)){
    VarKind vk=classifyVar(vs_ptr);             
    if((vk!=VAR_KINDED) && (vk!=VAR_FREE) && (vk!=VAR_FUTURE)) 
      ec=varGetEntityCond(vs_ptr);}
  else{
    NONVAR(e0, e);
    if(oz_isConst(e)){
      if(isWatcherEligible(e)){
	ec = getEntityCond((Tertiary *) tagged2Const(e));}}}

  if(ec!= ENTITY_NORMAL){
    OZ_RETURN(listifyWatcherCond(ec));}

  OZ_RETURN(oz_cons(AtomNormal,oz_nil()));
}OZ_BI_end


/*
 * The builtin table
 */
#ifndef MODULES_LINK_STATIC

#include "modFault-if.cc"

#endif










