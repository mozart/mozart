/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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



OZ_BI_define(BIdistHandlerInstall,2,0){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc      = OZ_in(1);

  initDP();
  NONVAR(c0, c);

  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;

  return DistHandlerInstall(condStruct,proc);
}OZ_BI_end

OZ_BI_define(BIdistHandlerDeInstall,2,0){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc      = OZ_in(1);

  NONVAR(c0, c);

  TaggedRef thread;

  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;

  return DistHandlerDeInstall(condStruct,proc);
}OZ_BI_end

OZ_BI_define(BIgetEntityCond,2,0){
  OZ_Term e0 = OZ_in(0);
  OZ_Term v0 = OZ_in(1);

  initDP();
  EntityCond ec;
  DEREF(e0,vs_ptr,vs_tag);
  if(isVariableTag(vs_tag)){
    VarKind vk=classifyVar(vs_ptr);
    if((vk==VAR_KINDED) || (vk==VAR_FREE) || (vk==VAR_FUTURE))
      ec=ENTITY_NORMAL;
    else ec=varGetEntityCond(vs_ptr);}

  NONVAR(e0, e);
  if(!oz_isConst(e)) return IncorrectFaultSpecification;

  Tertiary *tert = tagged2Tert(e);
  if(!isWatcherEligible(tert)) return IncorrectFaultSpecification;
  ec = getEntityCond(tert);

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
