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
#include "msgContainer.hh"
#include "dpMarshaler.hh"
#include "flowControl.hh"
#include "ozconfig.hh"

#include "os.hh"
#include "connection.hh"

// from builtins.cc
void doPortSend(PortWithStream *port,TaggedRef val,Board*);

/* *********************************************************************/
/*   global variables                                                  */
/* *********************************************************************/

int  globalSendCounter = 0;
int  globalRecCounter  = 0;
int  globalOSWriteCounter = 0;
int  globalOSReadCounter = 0;
int  globalContCounter = 0;

OZ_Term defaultConnectionProcedure = 0;

FILE *logfile=stdout;

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
    oe->setUp(ind);
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
  //cerr << "sizeof(PortProxy) = " << sizeof(PortProxy) << endl;
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

//
void send(MsgContainer *msgC, int priority)
{
  globalSendCounter++;

  int ret=msgC->getDestination()->send(msgC, priority);

  switch (ret) {
  case PERM_NOT_SENT:
    msgC->getDestination()->communicationProblem(msgC, 
						 COMM_FAULT_PERM_NOT_SENT);
    msgContainerManager->deleteMsgContainer(msgC);
    break;
  default:
    break;
  }
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

//
void gcPerdioStartImpl()
{
  if (isPerdioInitializedImpl()) {
    comController_startGCComObjs();
    // Erik+kost: TODO: why can't we do 'frameToProxy' during GCing of
    // matching proxies and 'gCollectBorrowTableUnusedFrames'??!
    borrowTable->gcFrameToProxy();
  }
  Assert(OT->notGCMarked());
  Assert(BT->notGCMarked());
}

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

//
void gcPerdioFinalImpl()
{
  if (isPerdioInitializedImpl()) {
    BT->gcBorrowTableFinal();
    OT->gcOwnerTableFinal(); 
    RHT->gcResourceTable();
    gcDSiteTable();
    comController_finishGCComObjs();
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
  CellManager *cm=(CellManager *)t;
  TaggedRef tr=cm->getCellSec()->getContents();    
  // Removing the chain. Was forgeten.
  cm->getChain()->free();
  t->setTertType(Te_Local);
  t->setBoard(am.currentBoard());
  CellLocal *cl=(CellLocal*) t;
  cl->setValue(tr);
  return;}

void localizeLock(Tertiary*t){
  LockManager *lm=(LockManager *)t;
  Thread *th=lm->getLockSec()->getLocker();
  lm->getChain()->free();
  t->setTertType(Te_Local);
  t->setBoard(am.currentBoard());
  LockLocal *ll=(LockLocal*) t;
  ll->convertToLocal(th,lm->getLockSec()->getPending());
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
    return oe;
  }
  return NULL;
}

inline OwnerEntry* receiveAtOwner(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  return oe;
}

BorrowEntry* receiveAtBorrow(DSite* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  return be;
}

inline BorrowEntry* maybeReceiveAtBorrow(DSite* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);

  return be;
}

void msgReceived(MsgContainer* msgC)
{
  Assert(oz_onToplevel());

  globalRecCounter++;

  MessageType mt = msgC->getMessageType();

  PD((MSG_RECEIVED,"msg type %d",mt));

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
      Credit c= oe->getCreditBig();

      MsgContainer *newmsgC = msgContainerManager->newMsgContainer(rsite);
      Assert(c.owner==NULL);
      newmsgC->put_M_BORROW_CREDIT(myDSite,na_index,c.credit);

      send(newmsgC,-1);
      break;
    }

  case M_OWNER_CREDIT: 
    {
      int index;
      int cint;
      Credit c;
      msgC->get_M_OWNER_CREDIT(index,cint);
      c.owner=NULL;
      c.credit=cint;

      PD((MSG_RECEIVED,"OWNER_CREDIT index:%d credit:%d",index,c));
      receiveAtOwner(index)->addCredit(c);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
      int index;
      int cint;
      Credit c;
      DSite* s;
      msgC->get_M_OWNER_SEC_CREDIT(s,index,cint);
      c.owner=myDSite; // I am the owner of this secondary credit
      c.credit=cint;
      PD((MSG_RECEIVED,"OWNER_SEC_CREDIT site:%s index:%d credit:%d",
	  s->stringrep(),index,c));    
//        printf("received M_OWNER_SEC_CREDIT %x %d %d %x\n",
//  	     (int)s,index,c.credit,(int)c.owner);

      receiveAtBorrow(s,index)->addCredit(c);
      break;
    }

  case M_BORROW_CREDIT:  
    {
      int si;
      int cint;
      Credit c;
      DSite* sd;
      msgC->get_M_BORROW_CREDIT(sd,si,cint);
      c.owner=NULL;
      c.credit=cint;
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
	be->addCredit(c);}
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
	  //
	  MsgContainer *msgC = msgContainerManager->newMsgContainer(rsite);

	  //
	  msgC->put_M_SEND_LAZY(myDSite, OTI, OBJECT_AND_CLASS,
				o->getClassTerm());
	  send(msgC, -1);
	}
	// no break here! - proceed with the 'OBJECT' case;

      case OBJECT:
	{
	  Assert(oz_isObject(t));
	  //
	  MsgContainer *msgC = msgContainerManager->newMsgContainer(rsite);

	  //
	  msgC->put_M_SEND_LAZY(myDSite, OTI, OBJECT, t);
	  send(msgC, -1);
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
	  Assert(extVar2Var(ov)->getType() == OZ_VAR_EXT);
	  Assert(ov->getIdV() == OZ_EVAR_LAZY);
	  Assert(ov->getLazyType() == LT_OBJECT);

	  //
	  Assert(oz_isClass(t));
	  Assert(!ov->isObjectClassAvail());
	  OZ_Term cvt = ov->getClassProxy();
	  DEREF(cvt, cvtp);
	  Assert(cvtp);
	  ClassVar *cv = (ClassVar *) oz_getExtVar(cvt);
	  Assert(extVar2Var(cv)->getType() == OZ_VAR_EXT);
	  Assert(cv->getIdV() == OZ_EVAR_LAZY);
	  Assert(cv->getLazyType() == LT_CLASS);
	  cv->transfer(t, cvtp);
	}
	break;

      case OBJECT:
	{
	  ObjectVar *ov = (ObjectVar *) GET_VAR(be, Lazy);
	  Assert(extVar2Var(ov)->getType() == OZ_VAR_EXT);
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
	GET_VAR(oe,Manager)->surrender(oe->getPtr(),v,rsite);
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
	break;}
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
      if (be) {
	Assert(be->isVar());
	GET_VAR(be,Proxy)->acknowledge(be->getPtr(), be);
      }

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
}


/**********************************************************************/
/*   communication problem                             */
/**********************************************************************/

// ERIK-LOOK 

//  inline void returnSendCredit(DSite* s,int OTI){
//    printf("returnSendCredit\n");
//    if(s==myDSite){
//      OT->getOwner(OTI)->receiveCredit(OTI);
//      return;}
//    sendCreditBack(s,OTI,1);}

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
      cellSendContentsFailure(tr,this,s1,OTI);
      return;
    }
    flag=USUAL_BORROW_CASE;
    break;
  }

  case M_LOCK_TOKEN: {
    if(fc == COMM_FAULT_PERM_NOT_SENT) {
      msgC->get_M_LOCK_TOKEN(s1,OTI);
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
  gCollectEntityInfo = gcEntityInfoImpl;
  gCollectPerdioStart = gcPerdioStartImpl;
  gCollectPerdioRoots = gcPerdioRootsImpl;
  gCollectBorrowTableUnusedFrames = gcBorrowTableUnusedFramesImpl;
  gCollectPerdioFinal = gcPerdioFinalImpl;
  dpExit = dpExitImpl;
  changeTCPLimit = changeTCPLimitImpl;
  distHandlerInstall = distHandlerInstallImpl;
  distHandlerDeInstall = distHandlerDeInstallImpl;
  //
  DV = new DebugVector();

  initNetwork();

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


  if(ozconf.dpSeifHandler)
    installGlobalWatcher(PERM_FAIL|TEMP_FAIL,
	          makeTaggedConst(new Builtin("", "failureDefault",
					       3, 0, BIfailureDefault, OK)),
	 WATCHER_PERSISTENT|WATCHER_SITE_BASED|WATCHER_INJECTOR);
  
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
#ifndef STATICALLY_INCLUDED
  OZ_C_proc_interface * oz_init_module(void) {
    return mod_int_DPB(); }
  char oz_module_name[] = "DPB";
#endif
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
    int bi=borrowTable->newBorrowPersistent(sd,si);
    b=borrowTable->getBorrow(bi);
    PortProxy *pp = new PortProxy(bi);
    b->mkTertiary(pp);
//      b->makePersistent(); // Already is
    if(sd->siteStatus()!=SITE_OK){
      if(sd->siteStatus()==SITE_PERM){
	deferProxyTertProbeFault(pp,PROBE_PERM);}
      else{
	Assert(sd->siteStatus()==SITE_TEMP);
	deferProxyTertProbeFault(pp,PROBE_TEMP);}}      
    return b->getValue();}
  Assert(b->isPersistent());
  return b->getValue();}


void dpExitWithTimer(unsigned int timeUntilClose)
{
  const unsigned int tuc = timeUntilClose;
  int remaining, connectionsLeft;
  //  printf("dpExit in pid %d\n",getpid());
  //  printf("Close started at %s\n", myDSite->stringrep());
  //printf("tiden:%d\n", timeUntilClose);
  //printf("tiden:%d\n", ozconf.closetime);

  if (!isPerdioInitialized())
    return;

  oz_rootBoard()->install();
  osSetAlarmTimer(0);

  //
  // This section was used to forcefully empty the borrowtable,
  // temporarily keeping only the cell frames and proxies that expect
  // to become a frame. This does not work however in the following
  // situation: imagine there is a cell frame, such that the cell's
  // content refers an (another) proxy. Now we cannot dump that later
  // proxy, because it is a part of the cell's state, and, thus, will
  // be needed when sending the state back to the manager.
  //
  // Instead let it be the responsibilty of the programmer to
  // drop all references and let them be garbagecollected before
  // calling Application.exit.
  /*
  if((int) timeUntilClose > 0)
    BT->closeFrameToProxy(timeUntilClose);
  do {
    //    printf("times left %d\n", timeUntilClose);
    //    printf("proxies left %d\n", remaining);
    unsigned long idle_start = osTotalTime();
    remaining = BT->closeProxyToFree(timeUntilClose);
    BT->closeFrameToProxy(timeUntilClose);
    osUnblockSignals();
    unsigned int ts = TIME_SLICE;
    osBlockSelect(ts);
    osBlockSignals(NO);
    timeUntilClose -= (osTotalTime() - idle_start);
    oz_io_handle();
  } while ((int) timeUntilClose > 0 && remaining);
  */
  //  printf("times left %d\n", timeUntilClose);
  //  printf("proxies left %d\n", remaining);

  //
  // kost@ : Another approach is to drop all the local refernces and
  // iterate with the GC/processing of incoming messages. The idea is
  // that once local references are dropped, proxies will eventually
  // go away, which can trigger reclamation of proxies at the manager
  // site, which, in turn, will trigger reclamation of managers here
  // etc. The trouble: if the manager deliberately keeps a proxy, then
  // the corresponding manager&everything reachable from it will stay
  // alive here.
  /*
  // Dump all the GC roots but the owner&borrow tables.
  //  + weak dicts
  //  - XRegs are dumped anyway by the GC
  //  + threads
  //      - current thread should not be accessed anymore anyway;
  //      + threads pool
  //      - suspended threads are explicitly reclaimed due to variables
  //        that are GCed away;
  //  - debugStreamTail is a variable
  //  - spaces
  //    - the root board should contain nothing 
  //    - other spaces should be unreachable;
  //  + defaultExceptionHdl
  //  - code area is not touched here:
  //    - not reachable code (purged threads) will be reclaimed;
  //    - reachable code - through owner&borrow entries - must stay!!
  //  - PrTabEntries - ditto;
  //  + ExtRefs - must be discarded explicitly (note: C++ destructors 
  //    should not access the Oz heap data structures after that, since
  //    that memory can be already unmapped etc.);
  //  - perdio roots: don't touch them! Owner&borrow entries may be
  //    needed for processing of incomming messages. Proxies will be
  //    reclaimed when there are no local references to them. Managers
  //    have to stay around 'cause there could be messages for them.
  //  
  gDropWeakDictionaries();
  DebugCode(setCurrentThread((Thread *) -1));
  am.threadsPool.init();
  am.setDefaultExceptionHdl(taggedVoidValue);
  oz_unprotectAllOnExit();

  // Now, do the loop with GC/IO:
  do {
    unsigned long idle_start = osTotalTime();
    osUnblockSignals();
    unsigned int ts = TIME_SLICE;
    osBlockSelect(ts);
    osBlockSignals(NO);
    timeUntilClose -= (osTotalTime() - idle_start);
    am.doGCollect();
    oz_io_handle();
  } while ((int) timeUntilClose > 0);
  timeUntilClose = tuc;
  */

  if (timeUntilClose) {
    //
    // kost@ : Now, the version of the first approach: let's first dump
    // all the frames we can handle, after what the input channels are
    // closed & flushed, and finally all proxies are forcefully
    // reclaimed;
    do {
      unsigned long idle_start = osTotalTime();
      remaining = BT->dumpFrames();
//        fprintf(stderr, "... closing, \n", remaining, osgetpid());
//        fprintf(stderr, "... closing, remaining %d frames (pid %d)!\n",
//  	      remaining, osgetpid());
//        fflush(stderr);
      oz_io_handle();
      osUnblockSignals();
      unsigned int ts = TIME_SLICE;
      osBlockSelect(ts);
      osBlockSignals(NO);
      oz_io_handle();
      timeUntilClose -= (osTotalTime() - idle_start);
    } while ((int) timeUntilClose > 0 && remaining);

    //
    oz_io_stopReadingOnShutdown();
    // 
    BT->dumpProxies();
    //
    timeUntilClose = tuc;
    do {
      unsigned long idle_start = osTotalTime();
      oz_io_handle();
      osUnblockSignals();
      unsigned int ts = TIME_SLICE;
      osBlockSelect(ts);
      osBlockSignals(NO);
      remaining = oz_io_numOfSelected();
      timeUntilClose -= (osTotalTime() - idle_start);
    } while ((int) timeUntilClose > 0 && remaining);
  }

  //
  // kost@ : 'nice close' does not work in this scenario since
  // no messages can be read;
  /*
  timeUntilClose = tuc;
  connectionsLeft =  startNiceClose();
  while ((int) timeUntilClose > 0 && connectionsLeft) {
    //    printf("times left %d\n", timeUntilClose);
    //    printf("connections left %d\n", connectionsLeft);
    unsigned long idle_start = osTotalTime();
    connectionsLeft = niceCloseProgress();
    osUnblockSignals();
    unsigned int ts = TIME_SLICE;
    osBlockSelect(ts);
    osBlockSignals(NO);
    timeUntilClose -= (osTotalTime() - idle_start);
    oz_io_handle();
  }
  */

  // Close any remaining connections violently.
  comController->closeAll();

  //  DebugCode(int dummy;
  //    Assert(getTransControllerInfo(dummy)==0);)
  //	    printf("left %d\n",getTransControllerInfo(dummy));)

  //  printf("times left %d\n", timeUntilClose);
  //  printf("connections left %d\n", connectionsLeft);
  //  printf("Close done at %s\n", myDSite->stringrep());

 if (logfile!=stdout) fclose(logfile);

 // For leaktracers to show the right thing delete as much as possible
 DebugCode(
	   delete msgContainerManager;
	   exitNetwork();
	   )
}

void dpExitImpl() {
  dpExitWithTimer(ozconf.closetime);
}


