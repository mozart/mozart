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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "var_obj.hh"
#endif

#include "var_obj.hh"
#include "dpMarshaler.hh"
#include "dpBase.hh"
#include "gname.hh"
#include "unify.hh"
#include "fail.hh"



void ObjectVar::marshal(MsgBuffer *bs)
{
  PD((MARSHAL,"var objectproxy"));
  int done=checkCycleOutLine(getObject(),bs);
  if (!done) {
    GName *classgn =  isObjectClassAvail()
      ? globalizeConst(getClass(),bs) : getGNameClass();
    marshalObjectImpl(getObject(),bs,classgn);
  }
}

/* --- ObjectProxis --- */

// mm2: deep as future!
OZ_Return ObjectVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  am.addSuspendVarList(lPtr);
  return SUSPEND;
}

// mm2: deep as future!
OZ_Return ObjectVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  return oz_var_bind(tagged2CVar(*rPtr),rPtr,makeTaggedRef(lPtr));
}

// extern
TaggedRef newObjectProxy(Object *o, GName *gnobj,
			 GName *gnclass, TaggedRef clas)
{
  ObjectVar *pvar;
  if (gnclass) {
    pvar = new ObjectVar(oz_currentBoard(),o,gnclass);
  } else {
    pvar = new ObjectVar(oz_currentBoard(),o,
			 tagged2ObjectClass(oz_deref(clas)));
  }
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  addGName(gnobj, val);
  return val;
}


static
void sendRequest(MessageType mt,BorrowEntry *be)
{
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  switch (mt) {
  case M_GET_OBJECT:
    marshal_M_GET_OBJECT(bs,na->index,myDSite);
    break;
  case M_GET_OBJECTANDCLASS:
    marshal_M_GET_OBJECTANDCLASS(bs,na->index,myDSite);
    break;
  default:
    Assert(0);
  }
  SendTo(na->site,bs,mt,na->site,na->index);
}

OZ_Return ObjectVar::addSuspV(TaggedRef * v, Suspension susp, int unstable)
{
  if(!errorIgnore()){
    //    if(failurePreemption()) return BI_REPLACECALL; mm3
  }
  addSuspSVar(susp, unstable);
  if (!requested) {
    requested = 1;
    MessageType mt = M_GET_OBJECT; 
    if (isObjectClassNotAvail() &&
	oz_findGName(getGNameClass())==0) {
      mt = M_GET_OBJECTANDCLASS;
    }
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());      
    sendRequest(mt,be);
  }
  return SUSPEND;
}


void ObjectVar::gcRecurseV(void)
{
  BT->getBorrow(getObject()->getIndex())->gcPO();
  obj = getObject()->gcObject();
  if (isObjectClassAvail()) {
    u.aclass = u.aclass->gcClass();}
}

void ObjectVar::disposeV()
{
  disposeS();
  if (isObjectClassNotAvail()) {
    deleteGName(u.gnameClass);
  }
  freeListDispose(this,sizeof(ObjectVar));
}


OZ_Term ObjectVar::statusV()
{
  SRecord *t = SRecord::newSRecord(AtomDet, 1);
  t->setArg(0, AtomObject);
  return makeTaggedSRecord(t);
}

VarStatus ObjectVar::checkStatusV()
{
  return EVAR_STATUS_DET;
}


void ObjectVar::sendObject(DSite* sd, int si, ObjectFields& of,
			   BorrowEntry *be)
{
  Object *o = getObject();
  Assert(o);
  GName *gnobj = o->getGName1();
  Assert(gnobj);
  gnobj->setValue(makeTaggedConst(o));
            
  fillInObject(&of,o);
  ObjectClass *cl;
  if (isObjectClassAvail()) {
    cl=getClass();
  } else {
    cl=tagged2ObjectClass(oz_deref(oz_findGName(getGNameClass())));
  }
  o->setClass(cl);
  oz_bindLocalVar(this,be->getPtr(),makeTaggedConst(o));
  be->changeToRef();
  BT->maybeFreeBorrowEntry(o->getIndex());
  o->localize();
  maybeHandOver(info,makeTaggedConst(o));
}

void ObjectVar::sendObjectAndClass(ObjectFields& of, BorrowEntry *be)
{
  Object *o = getObject();
  Assert(o);
  GName *gnobj = o->getGName1();
  Assert(gnobj);
  gnobj->setValue(makeTaggedConst(o));
      
  fillInObjectAndClass(&of,o);
  oz_bindLocalVar(this,be->getPtr(),makeTaggedConst(o));
  be->changeToRef();
  BT->maybeFreeBorrowEntry(o->getIndex());
  o->localize(); 
  maybeHandOver(info,makeTaggedConst(o));
}

// failure stuff

Bool ObjectVar::failurePreemption(){
  Bool hit=FALSE;
  Assert(info!=NULL);
  EntityCond oldC=info->getSummaryWatchCond();  
  if(varFailurePreemption(getTaggedRef(),info,hit)){
    EntityCond newC=info->getSummaryWatchCond();
    varAdjustPOForFailure(getObject()->getIndex(),oldC,newC);}
  return hit;
}

void ObjectVar::addEntityCond(EntityCond ec){
  Bool hit=FALSE;
  if(info==NULL) info=new EntityInfo();
  if(!info->addEntityCond(ec)) return;
  if(isInjectorCondition(ec)){
    wakeAll();
    return;}
  info->dealWithWatchers(getTaggedRef(),ec);
}

void ObjectVar::subEntityCond(EntityCond ec){
  Assert(info!=NULL);
  info->subEntityCond(ec);
}
  
void ObjectVar::probeFault(int pr){
  if(pr==PROBE_PERM){
    if(requested){
      addEntityCond(PERM_ME|PERM_BLOCKED);
      return;}
    addEntityCond(PERM_ME);
    return;}
  if(pr==PROBE_TEMP){
    if(requested){
      addEntityCond(TEMP_ME|TEMP_BLOCKED);
      return;}
    addEntityCond(TEMP_ME);    
    return;}
  Assert(pr==PROBE_OK);
  if(requested){
      subEntityCond(TEMP_ME|TEMP_BLOCKED);
      return;}
  subEntityCond(TEMP_ME);
}
  
Bool ObjectVar::errorIgnore(){
  if(info==NULL) return TRUE;
  if(info->getEntityCond()==ENTITY_NORMAL) return TRUE;
  return FALSE;}

void ObjectVar::wakeAll(){ // mm3
  oz_checkSuspensionList(this,pc_all);
}

void ObjectVar::newWatcher(Bool b){
  if(b){
    wakeAll();
    return;}
  info->dealWithWatchers(getTaggedRef(),info->getEntityCond());
}

TaggedRef ObjectVar::getTaggedRef(){
  return borrowTable->getBorrow(getObject()->getIndex())->getRef();}
