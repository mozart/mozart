/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
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
#pragma implementation "comm.hh"
#pragma implementation "perdio.hh"
#endif

#include "base.hh"
#include "dpBase.hh"
#include "dpInterface.hh"

#include "perdio.hh"

#include "thr_int.hh"
#include "var.hh"
#include "controlvar.hh"

#include "var_obj.hh"
#include "chain.hh"
#include "state.hh"
#include "fail.hh"
#include "interFault.hh"
#include "protocolCredit.hh"
#include "port.hh"
#include "dpResource.hh"
#include "protocolState.hh"
#include "protocolFail.hh"
#include "dpMarshaler.hh"
#include "flowControl.hh"
#include "ozconfig.hh"

#include "os.hh"

// from builtins.cc
void doPortSend(PortWithStream *port,TaggedRef val,Board*);

/* *********************************************************************/
/*   global variables                                                  */
/* *********************************************************************/

MsgBufferManager* msgBufferManager = new MsgBufferManager();

/* *********************************************************************/
/*   init;                                                             */
/* *********************************************************************/
//

//
// Interface method, BTW PER-LOOK is this necessary
Bool isPerdioInitializedImpl()
{
  return (perdioInitialized);
}

//
OZ_Term GateStream;

//
static void initGateStream()
{
  //
  // The gate is implemented as a Port reciding at location 0 in
  // the ownertable. The gateStream is keept alive, the Connection 
  // library will fetch it later.
  // The port is made persistent so it should not disapear.
  //
  GateStream = oz_newVariable();
  OZ_protect(&GateStream);
  {
    Tertiary *t=(Tertiary*)new PortWithStream(oz_currentBoard(),GateStream);
    globalizeTert(t);
    int ind = t->getIndex();
    Assert(ind==0);
    OwnerEntry* oe=OT->getOwner(ind);
    oe->makePersistent();
  }
}

//
static void initDPCore();

void initDP()
{
  //
  if (perdioInitialized)
    return;
  perdioInitialized = OK;

#ifdef DEBUG_CHECK
//   fprintf(stderr, "Waiting 10 secs... hook up (pid %d)!\n", osgetpid());
//   fflush(stderr);
//   sleep(10);
#endif

  //
  initDPCore();

}

/* *********************************************************************/
/*   Utility routines                                      */
/* *********************************************************************/

// PER-LOOK simplify
void SendTo(DSite* toS,MsgBuffer *bs,MessageType mt,DSite* sS,int sI)
{
#ifdef PERDIOLOGHIGH
  printf("sendingperdio: to:%s type:%s site:%s index:%d\n",
	 toS->stringRep(),mess_names[mt],sS->stringRep(),sI);
#endif
  int ret=toS->sendTo(bs,mt,sS,sI);
  if(ozconf.perdioMinimal){
    OZ_Term nogoods = bs->getNoGoods();
    if (!oz_eq(oz_nil(),nogoods)) {
      OZ_warning("send message '%s' contains nogoods: %s",
		 mess_names[mt],toC(nogoods));
    }}
  if(ret==ACCEPTED) return;
  if(ret==PERM_NOT_SENT){
    toS->communicationProblem(mt,sS,sI,COMM_FAULT_PERM_NOT_SENT,
			      (FaultInfo) bs);
    msgBufferManager->dumpMsgBuffer(bs);
  }
}


//
// mm2: should be OZ_unifyInThread???
void SiteUnify(TaggedRef val1,TaggedRef val2)
{
  TaggedRef aux1 = val1; DEREF(aux1,_1,_2);
  TaggedRef aux2 = val2; DEREF(aux2,_3,_4);
  
    if (oz_isUVar(aux1) || oz_isUVar(aux2)) {
      // cannot fail --> do it in current thread
    OZ_unify(val1,val2); // mm2: should be bind?
    return;
    }
  
  Assert(oz_onToplevel());
  Thread *th=oz_newThread(DEFAULT_PRIORITY);
#ifdef PERDIO_DEBUG
  PD((SITE_OP,"SITE_OP: site unify called %d %d",val1, val2));

  Assert(MemChunks::isInHeap(val1) && MemChunks::isInHeap(val1));
#endif
  th->pushCall(BI_Unify,val1,val2);
}

DSite* getSiteFromTertiaryProxy(Tertiary* t){
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  Assert(be!=NULL);
  return be->getNetAddress()->site;}

/* ******************************************************************* */
/*   pendThread          */
/* ******************************************************************* */

void pendThreadRemoveFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Assert(tmp->thread==NULL);
  *pt=tmp->next;  
  tmp->dispose();}

void pendThreadAddToEnd(PendThread **pt,TaggedRef o, 
				    TaggedRef n, ExKind e){
  while(*pt!=NULL){pt= &((*pt)->next);}
  ControlVarNew(controlvar,oz_rootBoard());
  *pt=new PendThread(oz_currentThread(),NULL,o,n,controlvar,e);
  suspendOnControlVar2();
}

void pendThreadAddDummyToEnd(PendThread **pt){
  while(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(NULL,NULL,DUMMY);
}

void pendThreadAddToEnd(PendThread **pt){
  while(*pt!=NULL){pt= &((*pt)->next);}
  ControlVarNew(controlvar,oz_rootBoard());  
  *pt=new PendThread(oz_currentThread(),NULL,controlvar,NOEX);
  suspendOnControlVar2();
}

void pendThreadAddMoveToEnd(PendThread **pt){
  while(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(NULL,NULL,MOVEEX);
}

void pendThreadAddRAToEnd(PendThread **pt,DSite *s1, DSite *s2,int index){
  while(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(NULL,NULL,(TaggedRef)s1,(TaggedRef)s2,
		     (TaggedRef)index,REMOTEACCESS);
}

/* ******************************************************************* */
/*   SECTION 18::  garbage-collection  DMM + some BASIC                */
/* ******************************************************************* */

/* OBS: ---------- interface to gc.cc ----------*/

void gcBorrowTableUnusedFramesImpl() { 
  if (isPerdioInitializedImpl())
    borrowTable->gcBorrowTableUnusedFrames();
}
void gcFrameToProxyImpl() {
  if (isPerdioInitializedImpl())
    borrowTable->gcFrameToProxy();

  Assert(OT->notGCMarked());
  Assert(BT->notGCMarked());
}

void gcProxyRecurseImpl(Tertiary *t) {
  int i = t->getIndex();
  BorrowEntry *be=BT->getBorrow(i);
  if(be->isGCMarked()){
    PD((GC,"borrow already marked:%d",i));
    return;}
  be->makeGCMark();
  if(be->isTertiary()) {
    be->updateTertiaryGC(t);}
  PD((GC,"relocate borrow :%d old:%x new:%x",i,be,t));
  return;}

void gcManagerRecurseImpl(Tertiary *t) {
  Assert(!t->isFrame());
  int i = t->getIndex();
  OwnerEntry *oe=OT->getOwner(i);
  if(oe->isGCMarked()){
    PD((GC,"owner already marked:%d",i));
    return;
  }
  PD((GC,"relocate owner:%d old%x new %x",i,oe,t));
  oe->gcPO(t);
}

void gCollectPendThread(PendThread **pt){
  PendThread *tmp;
  while(*pt!=NULL){
    tmp=new PendThread(SuspToThread((*pt)->thread->gCollectSuspendable()),(*pt)->next);
    tmp->exKind = (*pt)->exKind;
    if(tmp->exKind==REMOTEACCESS) {    
      tmp->nw = (*pt)->nw; 
      tmp->old = (*pt)->old; 
      tmp->controlvar = (*pt)->controlvar;
      ((DSite* )tmp->old)->makeGCMarkSite();      
      ((DSite* )tmp->nw) ->makeGCMarkSite();}
    else{
      oz_gCollectTerm((*pt)->old,tmp->old);
      oz_gCollectTerm((*pt)->nw,tmp->nw);
      oz_gCollectTerm((*pt)->controlvar,tmp->controlvar);}
    *pt=tmp;
    pt=&(tmp->next);}
}

/*--------------------*/

void gcPerdioRootsImpl()
{
  if (isPerdioInitializedImpl()) {
    OT->gcOwnerTableRoots();
    BT->gcBorrowTableRoots();
    gcGlobalWatcher();
    flowControler->gcEntries();
    gcDeferEvents();
  }
}

void gcPerdioFinalImpl()
{
  if (isPerdioInitializedImpl()) {
    BT->gcBorrowTableFinal();
    OT->gcOwnerTableFinal(); 
    RHT->gcResourceTable();
    gcDSiteTable();
  }
  gcTwins();
  Assert(OT->notGCMarked());
  Assert(BT->notGCMarked());
}

Bool isTertiaryPending(Tertiary* t){
  switch(t->getType()){
  case Co_Lock:
    if(t->getTertType()==Te_Proxy) return NO;
    Assert(t->getTertType()==Te_Frame);
    if(getLockSecFromTert(t)->getPending()==NULL) return NO;
    return OK;
  case Co_Cell:
    if(t->getTertType()==Te_Proxy) return NO;
    Assert(t->getTertType()==Te_Frame);
    if(getCellSecFromTert(t)->getPending()==NULL) return NO;
    return OK;
  case Co_Port: // ERIK-LOOK
    Assert(t->getTertType()==Te_Proxy);
    return ((PortProxy*)t)->pending != NULL;
  case Co_Resource:
    return NO;
  default:
    Assert(0);}
  return NO;
}
    
/* *********************************************************************/
/*   globalization                                       */
/* *********************************************************************/

void cellifyObject(Object* o){
  RecOrCell state = o->getState();
  if(stateIsCell(state)) return;
  SRecord *r = getRecord(state);
  Assert(r!=NULL);
  Tertiary *cell = (Tertiary *) tagged2Const(OZ_newCell(makeTaggedSRecord(r)));
  o->setState(cell);
  transferWatchers(o);
}

void globalizeTert(Tertiary *t)
{ 
  Assert(t->isLocal());

  switch(t->getType()) {
  case Co_Cell:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING cell index:%d",manI));
      oe_manager->mkTertiary(t);
      globalizeCell((CellLocal*)t,manI);
      return;
    }
  case Co_Lock:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING lock index:%d",manI));
      oe_manager->mkTertiary(t);
      globalizeLock((LockLocal*)t,manI);
      return;
    }
  case Co_Object:
    {
      cellifyObject((Object*) t);
      break;
    }
  case Co_Port:
    break;
  default:
    Assert(0);
  }

  t->setTertType(Te_Manager);
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"GLOBALIZING port/object index:%d",i));
  if(t->getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",t,((Object *)t)->getClass()));}
  oe->mkTertiary(t);
  t->setIndex(i);
  if(t->getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",t,((Object *)t)->getClass()));}
}


/**********************************************************************/
/*   Localizing                 should be more localize */
/**********************************************************************/

void Object::localize(){
  setTertType(Te_Local);
  setBoard(oz_currentBoard());
}

void localizeCell(Tertiary*t){
   CellFrame *cf=(CellFrame *)t;
   TaggedRef tr=cf->getCellSec()->getContents();    
   t->setTertType(Te_Local);
   t->setBoard(am.currentBoard());
   CellLocal *cl=(CellLocal*) t;
   cl->setValue(tr);
   return;}

void localizeLock(Tertiary*t){
  LockFrame *lf=(LockFrame *)t;
  Thread *th=lf->getLockSec()->getLocker();
  t->setTertType(Te_Local);
  t->setBoard(am.currentBoard());
  LockLocal *ll=(LockLocal*) t;
  ll->convertToLocal(th,lf->getLockSec()->getPending());
  return;}

void localizePort(Tertiary*t){
  t->setTertType(Te_Local);
  t->setBoard(am.currentBoard());
  return;}

Bool localizeTertiary(Tertiary*t){
  Assert(t->getTertType()==Te_Manager);
  switch(t->getType()){
  case Co_Lock:
    localizeLock(t);
    return OK;
  case Co_Cell:
    localizeCell(t);
    return OK;
  case Co_Port:
    localizePort(t);
    return OK;
  case Co_Object:
    ((Object*)t)->localize();
    return OK;
  default:
    Assert(0);
    return NO;
  }
}
    

/**********************************************************************/
/*  Main Receive                                      */
/**********************************************************************/

OwnerEntry* maybeReceiveAtOwner(DSite* mS,int OTI){
  if(mS==myDSite){
    OwnerEntry *oe=OT->getOwner(OTI);
    Assert(!oe->isFree());
    if(!oe->isPersistent())
      oe->receiveCredit(OTI);
    return oe;}
  return NULL;
}

inline OwnerEntry* receiveAtOwner(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  if(!oe->isPersistent())
    oe->receiveCredit(OTI);
  return oe;
}

inline OwnerEntry* receiveAtOwnerNoCredit(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  return oe;
}

BorrowEntry* receiveAtBorrow(DSite* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  be->receiveCredit();
  return be;
}

inline BorrowEntry* receiveAtBorrowNoCredit(DSite* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  return be;
}

inline BorrowEntry* maybeReceiveAtBorrow(DSite* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  if(be==NULL){
    sendCreditBack(na.site,na.index,1);}
  else {
    be->receiveCredit();}
  return be;
}

void msgReceived(MsgBuffer* bs)
{
  Assert(oz_onToplevel());
  Assert(creditSiteIn==NULL);
  Assert(creditSiteOut==NULL);
  MessageType mt = (MessageType) unmarshalHeader(bs);

  // this is a necessary check - you should never receive
  // a message from a site that you think is PERM or TEMP
  // this can happen - though it is very rare
  // for virtual sites we do not know, for now, the sending site so
  // we can do nothing
  DSite *ds=bs->getSite(); 
  if(ds!=NULL && ds->siteStatus()!=SITE_OK){
    return;}

  PD((MSG_RECEIVED,"msg type %d",mt));
  switch (mt) {
  case M_PORT_SEND:   
    {
      int portIndex;
      OZ_Term t;
      unmarshal_M_PORT_SEND(bs,portIndex,t);
      OwnerEntry *oe=receiveAtOwner(portIndex);
      Assert(oe);
      PortManager *pm=(PortManager*)(oe->getTertiary());
      Assert(pm->checkTertiary(Co_Port,Te_Manager));
      
      doPortSend(pm,t,NULL);

      break;
      }
  case M_ASK_FOR_CREDIT:
    {
      int na_index;
      DSite* rsite;
      unmarshal_M_ASK_FOR_CREDIT(bs,na_index,rsite);
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT index:%d site:%s",
	  na_index,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(na_index);
      Credit c= oe->giveMoreCredit();
      MsgBuffer *bs=msgBufferManager->getMsgBuffer(rsite);  
      marshal_M_BORROW_CREDIT(bs,myDSite,na_index,c);
      SendTo(rsite,bs,M_BORROW_CREDIT,myDSite,na_index);
      break;
    }

  case M_OWNER_CREDIT: 
    {
      int index;
      Credit c;
      unmarshal_M_OWNER_CREDIT(bs,index,c);
      PD((MSG_RECEIVED,"OWNER_CREDIT index:%d credit:%d",index,c));
      receiveAtOwnerNoCredit(index)->returnCreditOwner(c,index);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
      int index;
      Credit c;
      DSite* s;
      unmarshal_M_OWNER_SEC_CREDIT(bs,s,index,c);
      PD((MSG_RECEIVED,"OWNER_SEC_CREDIT site:%s index:%d credit:%d",
	  s->stringrep(),index,c));    
      receiveAtBorrowNoCredit(s,index)->addSecondaryCredit(c,myDSite);
      creditSiteIn = NULL;
      break;
    }

  case M_BORROW_CREDIT:  
    {
      int si;
      Credit c;
      DSite* sd;
      unmarshal_M_BORROW_CREDIT(bs,sd,si,c);
      PD((MSG_RECEIVED,"BORROW_CREDIT site:%s index:%d credit:%d",
	  sd->stringrep(),si,c));
      receiveAtBorrowNoCredit(sd,si)->addPrimaryCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_REGISTER(bs,OTI,rsite);
      PD((MSG_RECEIVED,"REGISTER index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(OTI);
      if (oe->isVar()) {
	(GET_VAR(oe,Manager))->registerSite(rsite);
      } else {
	sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());
      }
      break;
    }

  case M_DEREGISTER:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_REGISTER(bs,OTI,rsite);
      PD((MSG_RECEIVED,"REGISTER index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(OTI);
      if (oe->isVar()) {
	(GET_VAR(oe,Manager))->deregisterSite(rsite);
      } else {
	if(USE_ALT_VAR_PROTOCOL){
	  recDeregister(OT->getOwner(OTI)->getRef(),rsite);}
	}
      break;
    }

  case M_GET_OBJECT:
  case M_GET_OBJECTANDCLASS:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_GET_OBJECT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_GET_OBJECT(ANDCLASS) index:%d site:%s",
	  OTI,rsite->stringrep()));
      //      OwnerEntry *oe=receiveAtOwner(OTI);
      OwnerEntry *oe=OT->getOwner(OTI);
      Tertiary *t = oe->getTertiary();
      Assert(isObject(t));
      PD((SPECIAL,"object get %x %x",t,((Object *)t)->getClass()));
      sendObject(rsite,(Object *)t, mt==M_GET_OBJECTANDCLASS);
      break;
    }	
  case M_SEND_OBJECT:
    {
      ObjectFields of;
      DSite* sd;
      int si;
      unmarshal_M_SEND_OBJECT(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECT site:%s index:%d",sd->stringrep(),si));
      BorrowEntry *be=receiveAtBorrow(sd,si);
      Assert(be->isVar()); // check for duplicate object requests

      GET_VAR(be,Object)->sendObject(sd,si,of,be);
      break;
    }

  case M_SEND_OBJECTANDCLASS:
    {
      ObjectFields of;
      DSite* sd;
      int si;
      unmarshal_M_SEND_OBJECTANDCLASS(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECTANDCLASS site:%s index:%d",
	  sd->stringrep(),si));
      BorrowEntry *be=receiveAtBorrow(sd,si);
      Assert(be->isVar());// check for duplicate object requests
      
      GET_VAR(be,Object)->sendObjectAndClass(of,be);
      break;
    }

  case M_REDIRECT:
    {
      DSite* sd;
      int si;
      TaggedRef val;
      unmarshal_M_REDIRECT(bs,sd,si,val);
      PD((MSG_RECEIVED,"M_REDIRECT site:%s index:%d val%s",
	  sd->stringrep(),si,toC(val)));
      BorrowEntry* be=maybeReceiveAtBorrow(sd,si);
      if (!be) { // if not found, then forget the redirect message
	PD((WEIRD,"REDIRECT: no borrow entry found"));
	break;
      }
      Assert(be->isVar());
      ProxyVar *pv=GET_VAR(be,Proxy);
      pv->redirect(be->getPtr(),val,be);
      break;
    }

  case M_SURRENDER:
    {
      int OTI;
      DSite* rsite;
      TaggedRef v;
      unmarshal_M_SURRENDER(bs,OTI,rsite,v);
      PD((MSG_RECEIVED,"M_SURRENDER index:%d site:%s val%s",
	  OTI,rsite->stringrep(),toC(v)));
      OwnerEntry *oe = receiveAtOwner(OTI);

      if (oe->isVar()) {
	PD((PD_VAR,"SURRENDER do it"));
	GET_VAR(oe,Manager)->surrender(oe->getPtr(),v);
      } else {
	PD((PD_VAR,"SURRENDER discard"));
	PD((WEIRD,"SURRENDER discard"));
	// ignore redirect: NOTE: v is handled by the usual garbage collection
      }
      break;
    }

  case M_GETSTATUS:
    {
      DSite* site;
      int OTI;
      unmarshal_M_GETSTATUS(bs,site,OTI);
      PD((MSG_RECEIVED,"M_GETSTATUS index:%d",OTI));
      OwnerEntry *oe = receiveAtOwner(OTI);

      if(oe->isVar()){
	varGetStatus(site,OTI,oz_status(oe->getValue()));}
      break;
    }

  case M_SENDSTATUS:
    {
      DSite* site;
      int OTI;
      TaggedRef status;
      unmarshal_M_SENDSTATUS(bs,site,OTI,status);
      PD((MSG_RECEIVED,"M_SENDSTATUS site:%s index:%d status:%d",
      	  site->stringrep(),OTI,status));
      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=BT->find(&na);
      if(be==NULL){
	PD((WEIRD,"receive M_SENDSTATUS after gc"));
	sendCreditBack(site,OTI,1);
	break;}
      be->receiveCredit();
      Assert(be->isVar());
      (GET_VAR(be,Proxy))->receiveStatus(status);      
      break;
    }

  case M_ACKNOWLEDGE:
    {
      DSite* sd;
      int si;
      unmarshal_M_ACKNOWLEDGE(bs,sd,si);
      PD((MSG_RECEIVED,"M_ACKNOWLEDGE site:%s index:%d",sd->stringrep(),si));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->receiveCredit();

      Assert(be->isVar());
      GET_VAR(be,Proxy)->acknowledge(be->getPtr(), be);

      break;
    }
  case M_CELL_LOCK_GET:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_CELL_LOCK_GET(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_GET index:%d site:%s",OTI,rsite->stringrep()));
      cellLockReceiveGet(receiveAtOwner(OTI),rsite);
      break;
    }
   case M_CELL_CONTENTS:
    {
      DSite* rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CONTENTS(bs,rsite,OTI,val);
      PD((MSG_RECEIVED,"M_CELL_CONTENTS index:%d site:%s val:%s",
	  OTI,rsite->stringrep(),toC(val)));

      OwnerEntry* oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
	cellReceiveContentsManager(oe,val,OTI);
	break;}

      cellReceiveContentsFrame(receiveAtBorrow(rsite,OTI),val,rsite,OTI);
      break;
    }
  case M_CELL_READ:
    {
      int OTI;
      DSite* fS;
      unmarshal_M_CELL_READ(bs,OTI,fS);
      PD((MSG_RECEIVED,"M_CELL_READ"));
      cellReceiveRead(receiveAtOwner(OTI),fS); 
      break;
    }
  case M_CELL_REMOTEREAD:      
    {
      int OTI;
      DSite* fS,*mS;
      unmarshal_M_CELL_REMOTEREAD(bs,mS,OTI,fS);
      PD((MSG_RECEIVED,"CELL_REMOTEREAD %s",fS->stringrep()));
      cellReceiveRemoteRead(receiveAtBorrow(mS,OTI),mS,OTI,fS); 
      break;
    }
  case M_CELL_READANS:
    {
      int index;
      DSite*mS;
      TaggedRef val;
      unmarshal_M_CELL_READANS(bs,mS,index,val);
      PD((MSG_RECEIVED,"CELL_READANS"));
      OwnerEntry *oe=maybeReceiveAtOwner(mS,index);
      if(oe==NULL){
	cellReceiveReadAns(receiveAtBorrow(mS,index)->getTertiary(),val);
	break;}
      cellReceiveReadAns(oe->getTertiary(),val); 
      break;
   }   
  case M_CELL_LOCK_FORWARD:
    {
      DSite* site,*rsite;
      int OTI;
      unmarshal_M_CELL_LOCK_FORWARD(bs,site,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_FORWARD index:%d site:%s rsite:%s",
	  OTI,site->stringrep(),rsite->stringrep()));

      cellLockReceiveForward(receiveAtBorrow(site,OTI),rsite,site,OTI);
      break;
    }
  case M_CELL_LOCK_DUMP:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_CELL_LOCK_DUMP(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_DUMP index:%d site:%s",
	  OTI,rsite->stringrep()));
      cellLockReceiveDump(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_CELL_CANTPUT:
    {
      DSite* rsite, *ssite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CANTPUT(bs, OTI, rsite, val, ssite);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep(),toC(val)));
      cellReceiveCantPut(receiveAtOwner(OTI),val,OTI,ssite,rsite);
      break;
    }  
  case M_LOCK_TOKEN:
    {
      DSite* rsite;
      int OTI;
      unmarshal_M_LOCK_TOKEN(bs,rsite,OTI);
      PD((MSG_RECEIVED,"M_LOCK_TOKEN index:%d site:%s",
	  OTI,rsite->stringrep()));
      OwnerEntry *oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
	lockReceiveTokenManager(oe,OTI);
	break;}
      lockReceiveTokenFrame(receiveAtBorrow(rsite,OTI),rsite,OTI);
      break;
    }
  case M_CHAIN_ACK:
    {
      int OTI;
      DSite* rsite;
      unmarshal_M_CHAIN_ACK(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CHAIN_ACK index:%d site:%s",
	  OTI,rsite->stringrep()));
      chainReceiveAck(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_LOCK_CANTPUT:
    {
      DSite* rsite, *ssite;
      int OTI;
      unmarshal_M_LOCK_CANTPUT(bs, OTI, rsite, ssite);
      PD((MSG_RECEIVED,"M_LOCK_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep()));
      lockReceiveCantPut(receiveAtOwner(OTI),OTI,ssite,rsite);
      break;
    }
  case M_CHAIN_QUESTION:
   {
      DSite* site,*deadS;
      int OTI;
      unmarshal_M_CHAIN_QUESTION(bs,OTI,site,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_QUESTION index:%d site:%s",
	  OTI,site->stringrep()));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      chainReceiveQuestion(be,site,OTI,deadS);
      break;
   }
  case M_CHAIN_ANSWER:
    {
      DSite* rsite,*deadS;
      int OTI;
      int ans;
      unmarshal_M_CHAIN_ANSWER(bs,OTI,rsite,ans,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_ANSWER index:%d site:%s val:%d",
	  OTI,rsite->stringrep(),ans));
      chainReceiveAnswer(receiveAtOwner(OTI),rsite,ans,deadS);
      break;
    }  

  case M_TELL_ERROR:
    {
      DSite* site;
      int OTI;
      int ec,flag;
      unmarshal_M_TELL_ERROR(bs,site,OTI,ec,flag);
      PD((MSG_RECEIVED,"M_TELL_ERROR index:%d site:%s ec:%d",
	  OTI,site->stringrep(),ec));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      receiveTellError(be,ec,flag);
      break;
    }

  case M_ASK_ERROR:
    {
      int OTI;
      int ec;
      DSite* toS;
      unmarshal_M_ASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_ASK_ERROR index:%d ec:%d toS:%s",
	  OTI,ec,toS->stringrep()));
      receiveAskError(receiveAtOwner(OTI),toS,ec);
      break; 
    }
  case M_UNASK_ERROR:
    {
      int OTI;
      int ec;
      DSite* toS;
      unmarshal_M_UNASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_UNASK_ERROR index:%d ec:%d toS:%s",
	  OTI,ec,toS->stringrep()));
      receiveUnAskError(receiveAtOwner(OTI),toS,ec);
      break; 
    }
  case M_SEND_PING:
    {
      // the information received is not of any interest.
      int unused;
      DSite* fromS;
      unmarshal_M_SEND_PING(bs,fromS,unused);
      break;
    }
  default:
    OZ_error("siteReceive: unknown message %d\n",mt);
    break;
  }

  Assert(creditSiteIn==NULL);
  Assert(creditSiteOut==NULL);
}


/**********************************************************************/
/*   communication problem                             */
/**********************************************************************/

// ERIK-LOOK 

inline void returnSendCredit(DSite* s,int OTI){
  if(s==myDSite){
    OT->getOwner(OTI)->receiveCredit(OTI);
    return;}
  sendCreditBack(s,OTI,1);}

enum CommCase{
    USUAL_OWNER_CASE,
    USUAL_BORROW_CASE
  };

#define ResetCP(buf,mt) {				\
  buf->unmarshalReset();				\
  buf->unmarshalBegin();	 			\
  MessageType mt1=unmarshalHeader(buf);			\
  Assert(mt1==mt);					\
}

void DSite::communicationProblem(MessageType mt, DSite* storeSite,
				 int storeIndex, FaultCode fc, 
				 FaultInfo fi) {
  int OTI;
  DSite* s1;
  TaggedRef tr;
  CommCase flag;

  switch(mt){
  case M_CELL_CONTENTS:{
    if(fc == COMM_FAULT_PERM_NOT_SENT){
      ResetCP(((MsgBuffer*)fi),M_CELL_CONTENTS);
      unmarshal_M_CELL_CONTENTS((MsgBuffer*)fi,s1,OTI,tr);
      Assert(s1==storeSite);
      Assert(OTI=storeIndex);
      returnSendCredit(s1,OTI);	  
      cellSendContentsFailure(tr,this,storeSite,OTI);
      return;}
    flag=USUAL_BORROW_CASE;
    break;}

  case M_LOCK_TOKEN:{
    if(fc == COMM_FAULT_PERM_NOT_SENT){
      ResetCP(((MsgBuffer*)fi),M_LOCK_TOKEN);
      unmarshal_M_LOCK_TOKEN((MsgBuffer*)fi,s1,OTI);
      Assert(s1==storeSite);
      Assert(OTI=storeIndex);
      returnSendCredit(s1,OTI);
      lockSendTokenFailure(this,storeSite,OTI);
      return;}
    return;}
  
  default:
    return;
  }
}  
    
/**********************************************************************/
/*   Initialization                                      */
/**********************************************************************/

//
OZ_BI_proto(BIstartTmp);
OZ_BI_proto(BIdefer);
OZ_BI_proto(BIfailureDefault);

void initDPCore()
{
  // link interface...
  isPerdioInitialized = isPerdioInitializedImpl;
  portSend = portSendImpl;
  cellDoExchange = cellDoExchangeImpl;
  objectExchange = objectExchangeImpl;
  cellDoAccess = cellDoAccessImpl;
  cellAtAccess = cellAtAccessImpl;
  cellAtExchange = cellAtExchangeImpl;
  cellAssignExchange = cellAssignExchangeImpl;
  lockLockProxy = lockLockProxyImpl;
  lockLockManagerOutline = lockLockManagerOutlineImpl;
  unlockLockManagerOutline = unlockLockManagerOutlineImpl;
  lockLockFrameOutline = lockLockFrameOutlineImpl;
  unlockLockFrameOutline = unlockLockFrameOutlineImpl;
  marshalTertiary = marshalTertiaryImpl;
#ifdef USE_FAST_UNMARSHALER   
  unmarshalTertiary = unmarshalTertiaryImpl;
  unmarshalOwner = unmarshalOwnerImpl;
  unmarshalVar = unmarshalVarImpl;
#else
  unmarshalTertiaryRobust = unmarshalTertiaryRobustImpl;
  unmarshalOwnerRobust = unmarshalOwnerRobustImpl;
  unmarshalVarRobust = unmarshalVarRobustImpl;
#endif
  marshalVariable = marshalVariableImpl;
  triggerVariable = triggerVariableImpl;
  marshalObject = marshalObjectImpl;
  marshalSPP = marshalSPPImpl;
  gCollectProxyRecurse = gcProxyRecurseImpl;
  gCollectManagerRecurse = gcManagerRecurseImpl;
  gCollectDistResource = gcDistResourceImpl;
  gCollectDistCellRecurse = gcDistCellRecurseImpl;
  gCollectDistLockRecurse = gcDistLockRecurseImpl;
  gCollectDistPortRecurse = gcDistPortRecurseImpl;
  gCollectBorrowTableUnusedFrames = gcBorrowTableUnusedFramesImpl;
  gCollectFrameToProxy = gcFrameToProxyImpl;
  gCollectPerdioFinal = gcPerdioFinalImpl;
  gCollectPerdioRoots = gcPerdioRootsImpl;
  gCollectEntityInfo = gcEntityInfoImpl;
  dpExit = dpExitImpl;
#ifdef DEBUG_CHECK
  maybeDebugBufferGet = maybeDebugBufferGetImpl;
  maybeDebugBufferPut = maybeDebugBufferPutImpl;
#endif
  distHandlerInstall = distHandlerInstallImpl;
  distHandlerDeInstall = distHandlerDeInstallImpl;
  //
  DV = new DebugVector();

  initNetwork();

  creditSiteIn = NULL;
  creditSiteOut = NULL;
  borrowTable      = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  ownerTable       = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  resourceTable    = new ResourceHashTable(RESOURCE_HASH_TABLE_DEFAULT_SIZE);
  flowControler    = new FlowControler();
  msgBufferManager = new MsgBufferManager();

#ifndef DENYS_EVENTS
  if(!am.registerTask((void*)flowControler, FlowControlCheck, FlowControlExecute))
    OZ_error("Unable to register FlowControl task");
#endif
   
  BI_defer = makeTaggedConst(new Builtin("", "defer", 0, 0, BIdefer, OK));
  globalWatcher = NULL; 
  DeferdEvents = NULL;
  usedTwins = NULL;

  BI_startTmp  = makeTaggedConst(new Builtin("", "startTmp",
					     2, 0, BIstartTmp, OK));


  if(ozconf.perdioSeifHandler)
    installGlobalWatcher(PERM_FAIL|TEMP_FAIL,
	          makeTaggedConst(new Builtin("", "failureDefault",
					       3, 0, BIfailureDefault, OK)),
	 WATCHER_PERSISTENT|WATCHER_SITE_BASED|WATCHER_INJECTOR);
  
  Assert(sizeof(BorrowCreditExtension)<=sizeof(Construct_3));
  Assert(sizeof(OwnerCreditExtension)<=sizeof(Construct_3));
  Assert(sizeof(Chain)==sizeof(Construct_4));
  Assert(sizeof(ChainElem)==sizeof(Construct_3));
  Assert(sizeof(InformElem)==sizeof(Construct_3));
  Assert(sizeof(CellProxy)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellFrame));
  Assert(sizeof(CellManagerEmul)==sizeof(CellManager));
  Assert(sizeof(CellManager)==sizeof(CellLocal));
  Assert(sizeof(LockProxy)==sizeof(LockFrame));
  Assert(sizeof(LockManager)==sizeof(LockLocal));
  Assert(sizeof(LockManager)==sizeof(LockFrame));
  Assert(sizeof(LockManagerEmul)==sizeof(LockManager));
  Assert(sizeof(LockSecEmul)==sizeof(LockSec));
  Assert(sizeof(CellSecEmul)==sizeof(CellSec));
  Assert(sizeof(PortManager)==sizeof(PortLocal));
  Assert(sizeof(PortProxy)<=SIZEOFPORTPROXY);
  initGateStream();
  dealWithDeferredWatchers();
}

/**********************************************************************/
/*   MISC                                                */
/**********************************************************************/

void sendPing(DSite* s){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(s);  
  marshal_M_SEND_PING(bs,myDSite,42);
  SendTo(s,bs,M_SEND_PING,myDSite,0);
}

void marshalDSite(DSite* s,MsgBuffer *buf){
  s->marshalDSite(buf);
}

DSite* getSiteFromBTI(int i){
  return BT->getBorrow(i)->getNetAddress()->site;}

OwnerEntry *getOwnerEntryFromOTI(int i){
  return OT->getOwner(i);}

Tertiary* getTertiaryFromOTI(int i){
  return OT->getOwner(i)->getTertiary();}

/*
 * The builtin table: no builtins, just a fake
 */

#ifndef MODULES_LINK_STATIC

#ifdef DENYS_EVENTS
OZ_BI_proto(BIdp_task_tmpDown);
OZ_BI_proto(BIdp_task_myDown);
OZ_BI_proto(BIdp_task_probe);
OZ_BI_proto(BIdp_task_flowControl);
#endif

extern "C"
{
  OZ_C_proc_interface * mod_int_DPB(void)
  {
    static OZ_C_proc_interface i_table[] = {
#ifdef DENYS_EVENTS
      {"task.tmpDown",0,1,BIdp_task_tmpDown},
      {"task.myDown",0,1,BIdp_task_myDown},
      {"task.probe",0,1,BIdp_task_probe},
      {"task.flowControl",0,1,BIdp_task_flowControl},
#endif
      {0,0,0,0}
    };

    return i_table;
  } /* mod_int_DPB(void) */
} /* extern "C" */

#endif  

/**********************************************************************/
/*   Exported for gates                                   */
/**********************************************************************/

OZ_Term getGatePort(DSite* sd){
  int si=0; /* Gates are always located at position 0 */
  if(sd==myDSite){
    OwnerEntry* oe=OT->getOwner(si);
    Assert(oe->isPersistent());
    return  oe->getValue();}
  NetAddress na = NetAddress(sd,si); 
  BorrowEntry *b = borrowTable->find(&na);
  if (b==NULL) {
    int bi=borrowTable->newBorrow( PERSISTENT_CRED,sd,si);
    b=borrowTable->getBorrow(bi);
    PortProxy *pp = new PortProxy(bi);
    b->mkTertiary(pp);
    b->makePersistent();
    if(sd->siteStatus()!=SITE_OK){
      if(sd->siteStatus()==SITE_PERM){
	deferProxyTertProbeFault(pp,PROBE_PERM);}
      else{
	Assert(sd->siteStatus()==SITE_TEMP);
	deferProxyTertProbeFault(pp,PROBE_TEMP);}}      
    return b->getValue();}
  Assert(b->isPersistent());
  return b->getValue();}


void dpExitWithTimer(unsigned int timeUntilClose) {
  //  printf("Close started at %s\n", myDSite->stringrep());
  //printf("tiden:%d\n", timeUntilClose);
  //printf("tiden:%d\n", ozconf.closetime);

  if (!isPerdioInitialized())
    return;

  int proxiesLeft = 1, connectionsLeft;
  
  unsigned int timeToSleep;

  oz_rootBoard()->install();
  osSetAlarmTimer(0);

  if((int) timeUntilClose > 0)
    BT->closeFrameToProxy(timeUntilClose);
  while ((int) timeUntilClose > 0 && proxiesLeft) {
    //    printf("times left %d\n", timeUntilClose);
    //    printf("proxies left %d\n", proxiesLeft);
    unsigned long idle_start = osTotalTime();
    proxiesLeft = BT->closeProxyToFree(timeUntilClose);
    osUnblockSignals();
    timeToSleep = 50;
    osBlockSelect(timeToSleep);
    osBlockSignals(NO);
    timeUntilClose -= (osTotalTime() - idle_start);
    oz_io_handle();
  }
  //  printf("times left %d\n", timeUntilClose);
  //  printf("proxies left %d\n", proxiesLeft);

  (*virtualSitesExit)();

  if((int) timeUntilClose > 0)
    connectionsLeft =  startNiceClose();
  while ((int) timeUntilClose > 0 && connectionsLeft) {
    //    printf("times left %d\n", timeUntilClose);
    //    printf("connections left %d\n", connectionsLeft);
    unsigned long idle_start = osTotalTime();
    connectionsLeft = niceCloseProgress();
    osUnblockSignals();
    timeToSleep = 50;
    osBlockSelect(timeToSleep);
    osBlockSignals(NO);
    timeUntilClose -= (osTotalTime() - idle_start);
    oz_io_handle();
  }
  //  printf("times left %d\n", timeUntilClose);
  //  printf("connections left %d\n", connectionsLeft);
  //  printf("Close done at %s\n", myDSite->stringrep());
}

void dpExitImpl() {
  dpExitWithTimer(ozconf.closetime);
}


