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

//
OZ_BI_define(BIhwInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  

  //  
  initDP();

  NONVAR(c0, c);
  NONVAR(e0, e);
  TaggedRef label;
  SRecord  *condStruct;
  if(oz_isSRecord(c)){
    condStruct = tagged2SRecord(c);
    label = condStruct->getLabel();}
  else
    return oz_raise(E_ERROR,E_SYSTEM,"???? is not a Srecord",0);
  
  Tertiary *entity = tagged2Tert(e);
  
  TaggedRef type = condStruct->getLabel();
  if(type == AtomHandler)
    return HandlerInstall(entity,condStruct,proc);
  if(type == AtomWatcher)
    return WatcherInstall(entity,condStruct,proc);
  return oz_raise(E_ERROR,E_SYSTEM,"label must be either handler or watcher",0);
}OZ_BI_end


OZ_BI_define(BIhwDeInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  

  //  
  initDP();

  NONVAR(c0, c);
  NONVAR(e0, e);
  TaggedRef label;
  SRecord  *condStruct;
  if(oz_isSRecord(c)){
    condStruct = tagged2SRecord(c);
    label = condStruct->getLabel();}
  else
    oz_raise(E_ERROR,E_SYSTEM,"???? is not a Srecord",0);
  
  Tertiary *entity = tagged2Tert(e);
  TaggedRef type = condStruct->getLabel();
  if(type == AtomHandler)
    return HandlerDeInstall(entity,condStruct,proc);
  if(type == AtomWatcher)
    return WatcherDeInstall(entity,condStruct,proc);
  oz_raise(E_ERROR,E_SYSTEM,"label must be either handler or watcher",0);
  return PROCEED;
}OZ_BI_end

OZ_BI_define(BIgetEntityCond,1,1)
{
  OZ_Term e = OZ_in(0);
  NONVAR(e, entity);
  Tertiary *tert = tagged2Tert(entity);

  //  
  initDP();

  EntityCond ec = getEntityCond(tert);
  if(ec == ENTITY_NORMAL)
    OZ_RETURN(oz_cons(AtomEntityNormal,oz_nil()));
  OZ_RETURN(listifyWatcherCond(ec));
}OZ_BI_end

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modFault-if.cc"

#endif
