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
#include "var_class.hh"
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
#include "dpMarshalExt.hh"
#include "flowControl.hh"
#include "ozconfig.hh"

#include "os.hh"
#include "connection.hh"

// from builtins.cc
void doPortSend(PortWithStream *port,TaggedRef val,Board*);

/* *********************************************************************/
/*   global variables                                                  */
/* *********************************************************************/


MsgContainerManager* msgContainerManager;

int  globalSendCounter = 0;
int  globalRecCounter  = 0;
int  globalOSWriteCounter = 0;
int  globalOSReadCounter = 0;
int  globalContCounter = 0;

OZ_Term defaultAcceptProcedure = 0;
OZ_Term defaultConnectionProcedure = 0;
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

OZ_Term ConnectPortStream;
OZ_Term ConnectPort;

static void initConnectWstream()
{
  ConnectPortStream = oz_newVariable();
  ConnectPort = oz_newPort(ConnectPortStream);
  OZ_protect(&ConnectPort);
  OZ_protect(&ConnectPortStream);
}
//
static void initDPCore();

void initDP()
{
  //
  if (perdioInitialized)
    return;
  perdioInitialized = OK;
  msgContainerManager = new MsgContainerManager();

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

void SendTo(DSite* toS,MsgContainer *msgC,int priority) {
  globalSendCounter++;

  int ret=toS->sendTo(msgC,priority);
  
  if(ret==ACCEPTED) return;

  if(ret==PERM_NOT_SENT){
    toS->communicationProblem(msgC,COMM_FAULT_PERM_NOT_SENT);
    msgContainerManager->deleteMsgContainer(msgC);
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
    comController_gcComObjs();
    OT->gcOwnerTableRoots();
    BT->gcBorrowTableRoots();
    gcGlobalWatcher();
    flowControler->gcEntries();
    gcDeferEvents();
    // marshalers are not collected here, but through allocated
    // continuations ('MsgContainer::gcMsgC()');
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
  DebugCode(if(t->getType()==Co_Object)
  {
    //        printf("globalizingObject  index:%d %s %d\n",i,toC(makeTaggedConst(t)),osgetpid());
    PD((SPECIAL,"object:%x class%x",t,((Object *)t)->getClass()));})
  
  oe->mkTertiary(t);
  t->setIndex(i);
  //    DebugCode(if(t->getType()==Co_Object) {
  //      PD((SPECIAL,"object:%x class%x",t,((Object *)t)->getClass()));
  //    })
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
//      printf("localizingObject  index:%d %s %d\n",t->getIndex(),toC(makeTaggedConst(t)),osgetpid());
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

void msgReceived(MarshalerBuffer *mb) {printf("VS should not be used!!!\n");}

void msgReceived(MsgContainer* msgC,ByteBuffer *bs) //BS temp AN
{
  Assert(oz_onToplevel());
  Assert(creditSiteIn==NULL); 
  Assert(creditSiteOut==NULL);

  globalRecCounter++;

  MessageType mt = msgC->getMessageType();
  creditSiteIn=msgC->getImplicitMessageCredit();
  //  if(creditSiteIn!=NULL) printf("creditSiteIn: %x\n",creditSiteIn);

  // this is a necessary check - you should never receive
  // a message from a site that you think is PERM or TEMP
  // this can happen - though it is very rare
  // for virtual sites we do not know, for now, the sending site so
  // we can do nothing

  // AN this question should be asked to the comObj, which is not available
  // here. Save this for later.
//      DSite *ds=bs->getSite(); 
//      if(ds!=NULL && ds->siteStatus()!=SITE_OK){
//        return;}

  PD((MSG_RECEIVED,"msg type %d",mt));
  //printf("receiving msg:%d %s\n",mt,mess_names[mt]);
  switch (mt) {
  case M_PORT_SEND:   
    {
      int portIndex;
      OZ_Term t;
      msgC->get_M_PORT_SEND(portIndex,t);
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
      msgC->get_M_ASK_FOR_CREDIT(na_index,rsite);
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT index:%d site:%s",
	  na_index,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(na_index);
      Credit c= oe->giveMoreCredit();

      MsgContainer *newmsgC = msgContainerManager->newMsgContainer(rsite);
      newmsgC->put_M_BORROW_CREDIT(myDSite,na_index,c);

      SendTo(rsite,newmsgC,3);
      break;
    }

  case M_OWNER_CREDIT: 
    {
      int index;
      Credit c;
      msgC->get_M_OWNER_CREDIT(index,c);
      PD((MSG_RECEIVED,"OWNER_CREDIT index:%d credit:%d",index,c));
      receiveAtOwnerNoCredit(index)->returnCreditOwner(c,index);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
      int index;
      Credit c;
      DSite* s;
      msgC->get_M_OWNER_SEC_CREDIT(s,index,c);
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
      msgC->get_M_BORROW_CREDIT(sd,si,c);
      PD((MSG_RECEIVED,"BORROW_CREDIT site:%s index:%d credit:%d",
	  sd->stringrep(),si,c));
      //The entry might have been gc'ed. If so send the 
      //credit back. 
      //erik 
      NetAddress na=NetAddress(sd,si);
      BorrowEntry* be=BT->find(&na);
      if(be==NULL){
	sendCreditBack(na.site,na.index,c);}
      else {
	be->addPrimaryCredit(c);}
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      DSite* rsite;
      msgC->get_M_REGISTER(OTI,rsite);
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
      msgC->get_M_REGISTER(OTI,rsite);
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

  case M_GET_LAZY:
    {
      int OTI;
      DSite* rsite;
      int lazyFlag;
      //
      msgC->get_M_GET_LAZY(OTI, lazyFlag, rsite);
      PD((MSG_RECEIVED,"M_GET_LAZY index:%d site:%s",
	  OTI, rsite->stringrep()));
//        printf("M_GET_LAZY index:%d site:%s\n",
//  	     OTI, rsite->stringrep());
      //
      OwnerEntry *oe = receiveAtOwner(OTI);
      //
      OZ_Term t = oe->getTertTerm();

      //
      switch (lazyFlag) {
      case OBJECT_AND_CLASS:
	{
	  Assert(oz_isObject(t));
	  Object *o = (Object *) tagged2Const(t);
	  // kost@: 'SEND_LAZY' message with the class is associated
	  // with the object itself, so object's credits will be
	  // handed back when borrow entry is freed by
	  // 'ObjectVar::transfer()';
	  oe->getOneCreditOwner();
	  //
	  MsgContainer *msgC = msgContainerManager->newMsgContainer(rsite);

	  //
	  msgC->put_M_SEND_LAZY(myDSite, OTI, OBJECT_AND_CLASS,
				o->getClassTerm());
	  // printf("Class: %s\n",toC(o->getClassTerm()));
	  SendTo(rsite, msgC, 3);
	}
	// no break here! - proceed with the 'OBJECT' case;

      case OBJECT:
	{
	  Assert(oz_isObject(t));
	  oe->getOneCreditOwner();
	  //
	  MsgContainer *msgC = msgContainerManager->newMsgContainer(rsite);

	  //
	  msgC->put_M_SEND_LAZY(myDSite, OTI, OBJECT, t);
	  SendTo(rsite, msgC, 3);
	}	
	break;

      default:
	OZ_error("undefined/unimplemented lazy protocol!");
	break;
      }

      //
      break;
    }

  case M_SEND_LAZY:
    {
      DSite* sd;
      int si;
      int lazyFlag;
      OZ_Term t;

      msgC->get_M_SEND_LAZY(sd, si, lazyFlag, t);
      PD((MSG_RECEIVED,"M_SEND_LAZY site:%s index:%d", sd->stringrep(), si));
      BorrowEntry *be = receiveAtBorrow(sd, si);
      Assert(be->isVar()); // check for duplicate requests;

      //
      switch (lazyFlag) {
      case OBJECT_AND_CLASS:
	{
	  ObjectVar *ov = (ObjectVar *) GET_VAR(be, Lazy);
	  Assert(ov->getType() == OZ_VAR_EXT);
	  Assert(ov->getIdV() == OZ_EVAR_LAZY);
	  Assert(ov->getLazyType() == LT_OBJECT);

	  //
	  Assert(oz_isClass(t));
	  Assert(!ov->isObjectClassAvail());
	  OZ_Term cvt = ov->getClassProxy();
	  DEREF(cvt, cvtp, _cptt);
	  Assert(cvtp);
	  ClassVar *cv = (ClassVar *) tagged2CVar(cvt);
	  Assert(cv->getType() == OZ_VAR_EXT);
	  Assert(cv->getIdV() == OZ_EVAR_LAZY);
	  Assert(cv->getLazyType() == LT_CLASS);
	  cv->transfer(t, cvtp);
	}
	break;

      case OBJECT:
	{
	  ObjectVar *ov = (ObjectVar *) GET_VAR(be, Lazy);
	  Assert(ov->getType() == OZ_VAR_EXT);
	  Assert(ov->getIdV() == OZ_EVAR_LAZY);
	  Assert(ov->getLazyType() == LT_OBJECT);

	  //
	  Assert(oz_isObject(t));
	  Object *o = (Object *) tagged2Const(t);
	  Assert(o->getGName1() == ov->getGName());
	  o->setClassTerm(oz_deref(ov->getClass()));
	  ov->transfer(o, be);
	}
	break;

      default:
	Assert(0);
      }

      //
      break;
    }

  case M_REDIRECT:
    {
      DSite* sd;
      int si;
      TaggedRef val;
      msgC->get_M_REDIRECT(sd,si,val);
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
      msgC->get_M_SURRENDER(OTI,rsite,v);
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
      msgC->get_M_GETSTATUS(site,OTI);
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
      msgC->get_M_SENDSTATUS(site,OTI,status);
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
      msgC->get_M_ACKNOWLEDGE(sd,si);
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
      msgC->get_M_CELL_LOCK_GET(OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_GET index:%d site:%s",OTI,rsite->stringrep()));
      cellLockReceiveGet(receiveAtOwner(OTI),rsite);
      break;
    }
   case M_CELL_CONTENTS:
    {
      DSite* rsite;
      int OTI;
      TaggedRef val;
      msgC->get_M_CELL_CONTENTS(rsite,OTI,val);
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
      msgC->get_M_CELL_READ(OTI,fS);
      PD((MSG_RECEIVED,"M_CELL_READ"));
      cellReceiveRead(receiveAtOwner(OTI),fS,NULL); 
      break;
    }
  case M_CELL_REMOTEREAD:      
    {
      int OTI;
      DSite* fS,*mS;
      msgC->get_M_CELL_REMOTEREAD(mS,OTI,fS);
      PD((MSG_RECEIVED,"CELL_REMOTEREAD %s",fS->stringrep()));
      cellReceiveRemoteRead(receiveAtBorrow(mS,OTI),mS,OTI,fS); 
      break;
    }
  case M_CELL_READANS:
    {
      int index;
      DSite*mS;
      TaggedRef val;
      msgC->get_M_CELL_READANS(mS,index,val);
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
      msgC->get_M_CELL_LOCK_FORWARD(site,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_FORWARD index:%d site:%s rsite:%s",
	  OTI,site->stringrep(),rsite->stringrep()));

      cellLockReceiveForward(receiveAtBorrow(site,OTI),rsite,site,OTI);
      break;
    }
  case M_CELL_LOCK_DUMP:
    {
      int OTI;
      DSite* rsite;
      msgC->get_M_CELL_LOCK_DUMP(OTI,rsite);
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
      msgC->get_M_CELL_CANTPUT( OTI, rsite, val, ssite);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep(),toC(val)));
      cellReceiveCantPut(receiveAtOwner(OTI),val,OTI,ssite,rsite);
      break;
    }  
  case M_LOCK_TOKEN:
    {
      DSite* rsite;
      int OTI;
      msgC->get_M_LOCK_TOKEN(rsite,OTI);
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
      msgC->get_M_CHAIN_ACK(OTI,rsite);
      PD((MSG_RECEIVED,"M_CHAIN_ACK index:%d site:%s",
	  OTI,rsite->stringrep()));
      chainReceiveAck(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_LOCK_CANTPUT:
    {
      DSite* rsite, *ssite;
      int OTI;
      msgC->get_M_LOCK_CANTPUT( OTI, rsite, ssite);
      PD((MSG_RECEIVED,"M_LOCK_CANTPUT index:%d site:%s val:%s",
	  OTI,rsite->stringrep()));
      lockReceiveCantPut(receiveAtOwner(OTI),OTI,ssite,rsite);
      break;
    }
  case M_CHAIN_QUESTION:
   {
      DSite* site,*deadS;
      int OTI;
      msgC->get_M_CHAIN_QUESTION(OTI,site,deadS);
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
      msgC->get_M_CHAIN_ANSWER(OTI,rsite,ans,deadS);
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
      msgC->get_M_TELL_ERROR(site,OTI,ec,flag);
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
      msgC->get_M_ASK_ERROR(OTI,toS,ec);
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
      msgC->get_M_UNASK_ERROR(OTI,toS,ec);
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
      msgC->get_M_SEND_PING(fromS,unused);
      break;
    }
  case M_PING:
    {
      // Nothing specific needs to be done, the acknowledgement
      // that is or will be sent is enough.
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

void DSite::communicationProblem(MsgContainer *msgC, FaultCode fc) {
  int OTI;
  DSite* s1;
  TaggedRef tr;
  CommCase flag;

  switch(msgC->getMessageType()) {
  case M_CELL_CONTENTS: {
    if(fc == COMM_FAULT_PERM_NOT_SENT) {
      msgC->get_M_CELL_CONTENTS(s1,OTI,tr);
      returnSendCredit(s1,OTI);
      cellSendContentsFailure(tr,this,s1,OTI);
      return;
    }
    flag=USUAL_BORROW_CASE;
    break;
  }

  case M_LOCK_TOKEN: {
    if(fc == COMM_FAULT_PERM_NOT_SENT) {
      msgC->get_M_LOCK_TOKEN(s1,OTI);
      returnSendCredit(s1,OTI);
      lockSendTokenFailure(this,s1,OTI);
      return;
    }
    return;
  }

  default:
    //    printf("perdio.cc communicationProblem with %s?!\n",
    //    mess_names[msgC->getMessageType()]);
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

//  DebugCode(FILE *log;)
void initDPCore()
{
  // link interface...
//    DebugCode(char *s;sprintf(s,"~/tmp/%d",osgetpid()));
//    DebugCode(log=freopen(s,"w",stdout);)
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
  changeMaxTCPCache = changeMaxTCPCacheImpl;
  distHandlerInstall = distHandlerInstallImpl;
  distHandlerDeInstall = distHandlerDeInstallImpl;
  //
  DV = new DebugVector();

  dpAddExtensions();
  dpmInit();
  initNetwork();

  creditSiteIn = NULL;
  creditSiteOut = NULL;
  borrowTable      = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  ownerTable       = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  resourceTable    = new ResourceHashTable(RESOURCE_HASH_TABLE_DEFAULT_SIZE);
  flowControler    = new FlowControler();
  //  msgBufferManager = new MarshalerBufferManager();

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
  initConnectWstream();
  dealWithDeferredWatchers();
}

/**********************************************************************/
/*   MISC                                                */
/**********************************************************************/

void sendPing(DSite* s){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(s);
  msgC->put_M_SEND_PING(myDSite,42);

  SendTo(s,msgC,3);
}

void marshalDSite(MarshalerBuffer *buf, DSite* s)
{
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
  //  printf("dpExit in pid %d\n",getpid());
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


