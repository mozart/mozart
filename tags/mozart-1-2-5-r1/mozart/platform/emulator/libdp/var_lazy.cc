/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Per Brand (perbrand@sics.se)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Copyright:
 *    Michael Mehl (1997,1998)
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

#if defined(INTERFACE)
#pragma implementation "var_lazy.hh"
#endif

#include "var_lazy.hh"
#include "dpMarshaler.hh"
#include "dpBase.hh"
#include "gname.hh"
#include "unify.hh"
#include "fail.hh"


//
void LazyVar::marshal(ByteBuffer *bs)
{
  Assert(0);
}

// mm2: deep as future!
// kost@ : a masterpiece comment...
// cs: not bad either...
OZ_Return LazyVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  // PER-LOOK
  // kost@ : found anything?
  return oz_addSuspendVarList(lPtr);
}

// mm2: deep as future!
OZ_Return LazyVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  return oz_var_bind(tagged2Var(*rPtr),rPtr,makeTaggedRef(lPtr));
}

OZ_Return LazyVar::addSuspV(TaggedRef * v, Suspendable * susp)
{
  if(!errorIgnore()){
    if(failurePreemption()) return BI_REPLACEBICALL;}

  extVar2Var(this)->addSuspSVar(susp);
  if (!requested) {
    requested = 1;
    sendRequest();
  }
  return SUSPEND;
}

void LazyVar::gCollectRecurseV(void)
{
  Assert(0);
}

void LazyVar::disposeV()
{
  Assert(0);
}

OZ_Term LazyVar::statusV()
{
  SRecord *t = SRecord::newSRecord(AtomDet, 1);
  t->setArg(0, AtomObject);
  return makeTaggedSRecord(t);
}

VarStatus LazyVar::checkStatusV()
{
  return EVAR_STATUS_DET;
}

// failure stuff
Bool LazyVar::failurePreemption(){
  Bool hit=FALSE;
  Assert(info!=NULL);
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
  EntityCond oldC=info->getSummaryWatchCond();  
  if(varFailurePreemption(getTaggedRef(),info,hit,AtomObjectFetch)) {
    EntityCond newC=info->getSummaryWatchCond();
    varAdjustPOForFailure(index, oldC, newC);
  }
  return hit;
}

void LazyVar::addEntityCond(EntityCond ec){
  if(info==NULL) info=new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  wakeAll();
  info->dealWithWatchers(getTaggedRef(),ec);
}

void LazyVar::subEntityCond(EntityCond ec){
  Assert(info!=NULL);
  info->subEntityCond(ec);
}

void LazyVar::probeFault(int pr){
  if(pr==PROBE_PERM){
    addEntityCond(PERM_FAIL);
    return;}
  if(pr==PROBE_TEMP){
    addEntityCond(TEMP_FAIL);    
    return;}
  Assert(pr==PROBE_OK);
  subEntityCond(TEMP_FAIL);
}
  
Bool LazyVar::errorIgnore(){
  if(info==NULL) return TRUE;
  if(info->getEntityCond()==ENTITY_NORMAL) return TRUE;
  return FALSE;}

void LazyVar::wakeAll(){ // mm3 // kost@ who is that???
  OzVariable*p=extVar2Var(this);
  oz_checkSuspensionList(p,pc_all);
}

void LazyVar::newWatcher(Bool b){
  if(b){
    wakeAll();
    return;}
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
}

TaggedRef LazyVar::getTaggedRef() {
  return borrowTable->getBorrow(index)->getRef();
}



