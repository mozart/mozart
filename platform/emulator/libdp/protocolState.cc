/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
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

#include "base.hh"
#include "protocolState.hh"
#include "perdio.hh"
#include "table.hh"
#include "state.hh"
#include "chain.hh"
#include "controlvar.hh"
#include "protocolFail.hh"
#include "fail.hh"
#include "dsite.hh"
#include "msgContainer.hh"

/**********************************************************************/
/*  Failure-interface                       */
/**********************************************************************/

inline Bool tokenLostCheckProxy(Tertiary*t){
  if(getEntityCond(t) & PERM_FAIL){
    PD((WEIRD,"lost token found BUT cannot recover"));
    return TRUE;}
  return FALSE;}

inline Bool tokenLostCheckManager(Tertiary *t){
  if(getChainFromTertiary(t)->hasFlag(TOKEN_LOST)) {
    PD((WEIRD,"lost token found BUT cannot recover"));
    return OK;}
  return NO;}

inline void receiveGet_InterestOK(OwnerEntry* oe,DSite* toS,Tertiary* t){
  triggerInforms(getChainFromTertiary(t)->getInformBase(),oe,t->getIndex(),
                 getEntityCond(t));}

inline void receiveGet_TokenLost(OwnerEntry* oe,DSite* toS,Tertiary* t){
  PD((ERROR_DET,"TOKEN_LOST message bouncing"));
  sendTellError(oe,toS,t->getIndex(),(PERM_FAIL|PERM_ALL|PERM_SOME),true);
  return;}

/**********************************************************************/
/*   Cell lock protocol common  STATE                         */
/**********************************************************************/

void cellLockReceiveGet(OwnerEntry* oe,DSite* toS){
  Tertiary* t=oe->getTertiary();
  Chain *ch=getChainFromTertiary(t);
  if(ch->hasFlag(TOKEN_LOST)){
    receiveGet_TokenLost(oe,toS,t);// FAIL-HOOK
    return;}
  if(t->getType()==Co_Cell){
    cellReceiveGet(oe,(CellManager*) t,toS);}
  else{
    lockReceiveGet(oe,(LockManager*) t,toS);}
  if(ch->hasFlag(INTERESTED_IN_OK)){
    receiveGet_InterestOK(oe,toS,t);}}

void cellLockReceiveForward(BorrowEntry *be,DSite* toS,DSite* mS,int mI){
  if(be->getTertiary()->getType()==Co_Cell){
    cellReceiveForward(be,toS,mS,mI);
    return;}
  lockReceiveForward(be,toS,mS,mI);}

void cellLockSendGet(BorrowEntry *be,DSite *cS){
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  //  installProbeNoRet(toS,PROBE_TYPE_ALL);
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s",na->index,toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_LOCK_GET(na->index,myDSite);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);}

void cellLockSendForward(DSite *toS,DSite *fS,int mI){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_LOCK_FORWARD(myDSite,mI,fS);
  sendTo(toS,msgC,3);}

void cellLockReceiveDump(OwnerEntry *oe,DSite* fromS){
  Tertiary *t=oe->getTertiary();
  if(t->getType()==Co_Cell){
    cellReceiveDump((CellManager*) t,fromS);}
  else{
    lockReceiveDump((LockManager*) t,fromS);}}

void cellLockSendDump(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  PD((GC,"CLDump %d ",na->index));
  DSite *toS=na->site;
  if(SEND_SHORT(toS)){return;}

  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_LOCK_DUMP(na->index,myDSite);
  msgC->setImplicitMessageCredit(be->getOneMsgCredit());
  sendTo(toS,msgC,3);}

/**********************************************************************/
/*   Cell protocol - receive                            */
/**********************************************************************/

Bool CellSec::secForward(DSite* toS,TaggedRef &val){
  if(state==Cell_Lock_Valid){
    state=Cell_Lock_Invalid;
    val=contents;
    return OK;}
  Assert(state==Cell_Lock_Requested);
  state=Cell_Lock_Requested|Cell_Lock_Next;
  next=toS;
  return NO;
}

Bool CellSec::secReceiveContents(TaggedRef val,DSite* &toS,TaggedRef &outval){
  PendThread *pt=pending;
  TaggedRef tmp=val;
  while(pt!=NULL){
    if (pt->thread != NULL) { // if the thread has done an exception
      tmp=unpendCell(pt,tmp); }
    pending=pt->next;
    pt->dispose();
    pt=pending;}
  outval = tmp;
  Assert(pending==NULL);
  if(state & Cell_Lock_Next){
    state = Cell_Lock_Invalid;
    toS=next;
    return OK;}
  contents=tmp;
  state = Cell_Lock_Valid;
  return NO;
}

Bool secReceiveReadAnsCheck(PendThread* pb){
  while(pb!=NULL){
    if((pb->exKind==ACCESS) || (pb->exKind==DEEPAT)) return FALSE;
    pb=pb->next;}
  return TRUE;
}

void CellSec::secReceiveReadAns(TaggedRef val){
  PendThread* pb=pending;
  while(pb!=NULL && ((pb->exKind==ACCESS) || (pb->exKind==DEEPAT))){
    TaggedRef aux=unpendCell(pb,val);
    Assert(aux==val);
    pending=pending->next;
    pb->dispose();
    pb = pending;
  }
  Assert(secReceiveReadAnsCheck(pending));
}

Bool CellSec::secReceiveRemoteRead(DSite* toS,DSite* mS, int mI, DSite *cS){
  switch(state){
  case Cell_Lock_Invalid:
    return NO;
  case Cell_Lock_Valid:{
    cellSendReadAns(toS,mS,mI,contents,cS);
    return TRUE;}
  case Cell_Lock_Requested:{
    pendThreadAddRAToEnd(&pending,toS,mS,mI);
    return TRUE;}
  default: Assert(0);}
  return NO;
}

void cellReceiveGet(OwnerEntry* oe,CellManager* cm,DSite* toS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  Chain *ch=cm->getChain();
  DSite* current=ch->setCurrent(toS,cm);
  PD((CELL,"CellMgr Received get from %s",toS->stringrep()));
  PD((CHAIN,"%d",printChain(ch)));
  if(current==myDSite){
    PD((CELL,"CELL - shortcut in cellReceiveGet"));
    TaggedRef val;
    if(cm->getCellSec()->secForward(toS,val)){
      oe->getOneCreditOwner();
      cellSendContents(val,toS,myDSite,cm->getIndex(),NULL);}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,cm->getIndex());
}

void cellReceiveDump(CellManager *cm,DSite *fromS){
  if(tokenLostCheckManager(cm)) return; // FAIL-HOOK
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  if((cm->getChain()->getCurrent()!=fromS) ||
     (cm->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"CELL dump not needed"));
    return;}
  getCellSecFromTert(cm)->dummyExchange(cm);
  return;
}
// MERGECON  (void) cellDoExchangeInternal((Tertiary *)cm,tr,tr,DummyThread,EXCHANGE);

void cellReceiveForward(BorrowEntry *be,DSite *toS,DSite* mS,int mI){
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->isFrame());
  Assert(cf->getType()==Co_Cell);
  CellSec *sec=cf->getCellSec();
  TaggedRef val;
  if(!sec->secForward(toS,val)) return;
  DSite *cS=be->getOneMsgCredit();
  cellSendContents(val,toS,mS,mI,cS);
  return;
}

void cellReceiveContentsManager(OwnerEntry *oe,TaggedRef val,int mI){
  CellManager *cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  if(tokenLostCheckManager(cm)) return; // FAIL-HOOK
  chainReceiveAck(oe,myDSite);
  CellSec *sec=cm->getCellSec();
  DSite *toS;
  TaggedRef outval;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  oe->getOneCreditOwner();
  cellSendContents(outval,toS,myDSite,mI,NULL);
  return;
}

void cellReceiveContentsFrame(BorrowEntry *be,TaggedRef val,DSite *mS,int mI){
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getType()==Co_Cell);
  Assert(cf->isFrame());
  if(tokenLostCheckProxy(cf)) return; // FAIL-HOOK
  DSite *cS1=be->getOneMsgCredit();
  chainSendAck(mS,mI,cS1);
  CellSec *sec=cf->getCellSec();
  TaggedRef outval;
  DSite *toS;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  DSite *cS=be->getOneMsgCredit();
  cellSendContents(outval,toS,mS,mI,cS);
}

void cellReceiveRemoteRead(BorrowEntry *be,DSite* mS,int mI,DSite* fS){
  PD((CELL,"Receive REMOTEREAD toS:%s",fS->stringrep()));
  Tertiary* t=be->getTertiary();
  Assert(t->isFrame());
  Assert(t->getType()==Co_Cell);
  CellSec *sec=((CellFrame*)t)->getCellSec();
  DSite *cS=be->getOneMsgCredit();
  if(sec->secReceiveRemoteRead(fS,mS,mI,cS)) return;
  PD((WEIRD,"miss on read"));
  DSite *cS2=be->getOneMsgCredit();
  cellSendRead(be,fS,cS2);
}

void cellReceiveRead(OwnerEntry *oe,DSite* fS,DSite *cS){
  PD((CELL,"Recevie READ toS:%s",fS->stringrep()));
  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->isManager());
  Assert(cm->getType()==Co_Cell);
  CellSec *sec=cm->getCellSec();
  oe->getOneCreditOwner();
  Chain* ch=cm->getChain();
  if(ch->getCurrent()==myDSite){
    PD((CELL,"Token at mgr site short circuit"));
    sec->secReceiveRemoteRead(fS,myDSite,cm->getIndex(),cS);
    return;}
  cellSendRemoteRead(ch->getCurrent(),myDSite,cm->getIndex(),fS,cS);
}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){
  Assert((t->isManager())|| (t->isFrame()));
  getCellSecFromTert(t)->secReceiveReadAns(val);
}

/**********************************************************************/
/*   cell send                        */
/**********************************************************************/

void cellSendReadAns(DSite* toS,DSite* mS,int mI,TaggedRef val,DSite *cS){
  if(toS == myDSite) {
    OwnerEntry *oe=maybeReceiveAtOwner(mS,mI);
    if(mS!=myDSite)
      cellReceiveReadAns(receiveAtBorrow(mS,mI)->getTertiary(),val);
    else
      cellReceiveReadAns(maybeReceiveAtOwner(mS,mI)->getTertiary(),val);
    return;}
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_READANS(mS,mI,val);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

void cellSendRemoteRead(DSite* toS,DSite* mS,int mI,DSite* fS,DSite *cS){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_REMOTEREAD(mS,mI,fS);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

void cellSendContents(TaggedRef tr,DSite* toS,DSite *mS,int mI,DSite *cS){
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  PD((SPECIAL,"CellContents %s",toC(tr)));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_CONTENTS(mS,mI,tr);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

void cellSendRead(BorrowEntry *be,DSite *dS,DSite *cS){
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  // installProbeNoRet(toS,PROBE_TYPE_ALL);
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_READ(na->index,dS);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

void chainSendAck(DSite* toS, int mI, DSite *cS){
  if(SEND_SHORT(toS)) {return;}
  PD((CHAIN,"M_CHAIN_ACK indx:%d site:%s",mI,toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_ACK(mI,myDSite);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

/**********************************************************************/
/*   cell/lock receive                       */
/**********************************************************************/

void chainReceiveAck(OwnerEntry* oe,DSite* rsite){
  Tertiary *t=oe->getTertiary();
  Chain* chain=getChainFromTertiary(t);
  if(!(chain->siteExists(rsite))) {
    return;}
  chain->removeBefore(rsite);
  PD((CHAIN,"%d",printChain(chain)));
}

/**********************************************************************/
/*   Lock protocol - receive                             */
/**********************************************************************/

Bool LockSec::secReceiveToken(Tertiary* t,DSite* &toS){
  if(state & Cell_Lock_Next) state = Cell_Lock_Next|Cell_Lock_Valid;
  else state=Cell_Lock_Valid;
  while(pending!=NULL){
    if(pending->thread!=NULL){
      locker=pendThreadResumeFirst(&pending);
      return OK;}
    if(pending->exKind==MOVEEX){
      PD((WEIRD,"lock requested but not used"));
      Assert(state==Cell_Lock_Next|Cell_Lock_Valid);
      state=Cell_Lock_Invalid;
      toS=next;
      return NO;}
    pendThreadRemoveFirst(getPendBase());}
  if(state == Cell_Lock_Valid) return OK;
  toS=next;
  state=Cell_Lock_Invalid;
  return NO;
}

Bool LockSec::secForward(DSite* toS){
  if(state==Cell_Lock_Valid){
    if(locker==NULL){
      state=Cell_Lock_Invalid;
      return OK;}
    state=Cell_Lock_Valid|Cell_Lock_Next;
    pendThreadAddMoveToEnd(getPendBase());
    next=toS;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  state= Cell_Lock_Requested|Cell_Lock_Next;
  pendThreadAddMoveToEnd(getPendBase());
  next=toS;
  return NO;
}

void lockReceiveGet(OwnerEntry* oe,LockManager* lm,DSite* toS){
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  Chain *ch=lm->getChain();
  PD((LOCK,"LockMgr Received get from %s",toS->stringrep()));
  DSite* current=ch->setCurrent(toS,lm);
  PD((CHAIN,"%d",printChain(ch)));
  if(current==myDSite){                             // shortcut
    PD((LOCK," shortcut in lockReceiveGet"));
    if(lm->getLockSec()->secForward(toS)){
      oe->getOneCreditOwner();
      lockSendToken(myDSite,lm->getIndex(),toS,NULL);}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,lm->getIndex());
}

void lockReceiveDump(LockManager* lm,DSite *fromS){
  if(tokenLostCheckManager(lm)) return; // FAIL-HOOK
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  LockSec* sec=lm->getLockSec();
  if((lm->getChain()->getCurrent()!=fromS) ||
     (sec->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"WEIRD- LOCK dump not needed"));
    return;}
  Assert(sec->getState()==Cell_Lock_Invalid);
  pendThreadAddDummyToEnd(sec->getPendBase());
  secLockGet(sec,lm,NULL);
  return;
}

void lockReceiveTokenManager(OwnerEntry* oe,int mI){
  Tertiary *t=oe->getTertiary();
  Assert(t->getType()==Co_Lock);
  Assert(t->isManager());
  if(tokenLostCheckManager(t)) return; // FAIL-HOOK
  LockManager*lm=(LockManager*)t;
  chainReceiveAck(oe,myDSite);
  LockSec *sec=lm->getLockSec();
  DSite* toS;
  if(sec->secReceiveToken(t,toS)) return;
  PD((CHAIN,"%d",printChain(lm->getChain())));
  oe->getOneCreditOwner();
  lockSendToken(myDSite,mI,toS,NULL);
}

void lockReceiveTokenFrame(BorrowEntry* be, DSite *mS,int mI){
  LockFrame *lf=(LockFrame*) be->getTertiary();
  Assert(lf->getType()==Co_Lock);
  Assert(lf->isFrame());
  if(tokenLostCheckProxy(lf)) return; // FAIL-HOOK
  DSite *cS=be->getOneMsgCredit();
  chainSendAck(mS,mI,cS);
  LockSec *sec=lf->getLockSec();
  DSite* toS;
  if(sec->secReceiveToken(lf,toS)) return;
  DSite *cS2=be->getOneMsgCredit();
  lockSendToken(mS,mI,toS,cS2);
}

void lockReceiveForward(BorrowEntry *be,DSite *toS,DSite* mS,int mI){
  LockFrame *lf= (LockFrame*) be->getTertiary();
  Assert(lf->isFrame());
  Assert(lf->getType()==Co_Lock);
  LockSec* sec=lf->getLockSec();
  if(!sec->secForward(toS)) return;
  DSite *cS=be->getOneMsgCredit();
  lockSendToken(mS,mI,toS,cS);
}

/**********************************************************************/
/*  Lock protocol - send                                */
/**********************************************************************/

void lockSendToken(DSite *mS,int mI,DSite* toS,DSite *cS){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_LOCK_TOKEN(mS,mI);
  msgC->setImplicitMessageCredit(cS);
  sendTo(toS,msgC,3);
}

/**********************************************************************/
/*   CHAIN and failure-related                       */
/**********************************************************************/

ChainAnswer answerChainQuestion(Tertiary *t){
  if(t->isProxy()){
    return PAST_ME;}
  switch(getStateFromLockOrCell(t)){
  case Cell_Lock_Invalid:
    return PAST_ME;
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:
    return BEFORE_ME;
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:
    return AT_ME;
  default:
    Assert(0);}
  Assert(0);
  return BEFORE_ME;
}

void chainReceiveQuestion(BorrowEntry *be,DSite* site,int OTI,DSite* deadS){
  if(be==NULL){
    chainSendAnswer(be,site,OTI,PAST_ME,deadS);}
  chainSendAnswer(be,site,OTI,answerChainQuestion(be->getTertiary()),deadS);
}

void chainReceiveAnswer(OwnerEntry* oe,DSite* fS,int ans,DSite* deadS){
  Tertiary* t=oe->getTertiary();
  getChainFromTertiary(t)->receiveAnswer(t,fS,ans,deadS);
  PD((CHAIN,"%d",printChain(getChainFromTertiary(t))));
}

void maybeChainSendQuestion(ChainElem *ce,Tertiary *t,DSite* deadS){
  if(ce->getSite()!=myDSite){
    if(!(ce->flagIsSet(CHAIN_QUESTION_ASKED))){
      ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
      chainSendQuestion(ce->getSite(),t->getIndex(),deadS);}
    return;}
  Chain *ch=getChainFromTertiary(t);
  ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
  ch->receiveAnswer(t,myDSite,answerChainQuestion(t),deadS);}

/**********************************************************************/
/*   failure-related                                         */
/**********************************************************************/

Bool CellSec::cellRecovery(TaggedRef tr){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    contents=tr;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  return OK;
}

Bool LockSec::lockRecovery(){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    locker=NULL;
    return NO;}
  state &= ~Cell_Lock_Next;
  Assert(state==Cell_Lock_Requested);
  return OK;
}

void cellManagerIsDown(TaggedRef tr,DSite* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL) return; // has been gced
  Tertiary* t=be->getTertiary();
  maybeConvertCellProxyToFrame(t);
  if(((CellFrame*)t)->getCellSec()->cellRecovery(tr)){
    cellReceiveContentsFrame(be,tr,mS,mI);}
}

void lockManagerIsDown(DSite* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL) return; // has been gced
  Tertiary* t=be->getTertiary();
  maybeConvertLockProxyToFrame(t);
  if(((LockFrame*)t)->getLockSec()->lockRecovery()){
    lockReceiveTokenFrame(be,mS,mI);}
}

void cellSendCantPut(TaggedRef tr,DSite* toS, DSite *mS, int mI){
  PD((ERROR_DET,"Proxy cant put to %s site: %s:%d",
      toS->stringrep(), mS->stringrep(),mI));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_CANTPUT(mI, toS, tr, myDSite);
  sendTo(mS,msgC,3);
}

void cellSendContentsFailure(TaggedRef tr,DSite* toS,DSite *mS, int mI){
  if(toS==mS) {// ManagerSite is down
    cellManagerIsDown(tr,toS,mI);
    return;}
  if(mS==myDSite){// At managerSite
    cellReceiveCantPut(OT->getOwner(mI),tr,mI,mS,toS);
    return;}
  cellSendCantPut(tr,toS,mS,mI);
  return;
}

void lockSendCantPut(DSite* toS, DSite *mS, int mI){
  PD((ERROR_DET,"Proxy cant put - to %s site: %s:%d Nr %d",
      toS->stringrep(),mS->stringrep(),mI));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(mS);
  msgC->put_M_LOCK_CANTPUT(mI, toS, myDSite);
  sendTo(mS,msgC,3);
}

void lockSendTokenFailure(DSite* toS,DSite *mS, int mI){
  PD((ERROR_DET,"LockTokenFailure"));
  if(toS==mS) {// ManagerSite is down
    lockManagerIsDown(mS,mI);
    return;}
  if(mS==myDSite){// At managerSite
    lockReceiveCantPut(OT->getOwner(mI),mI,mS,toS);
    return;}
  lockSendCantPut(toS,mS,mI);
  return;
}

/**********************************************************************/
/*   STATE & FAILURE                        */
/**********************************************************************/

void lockReceiveCantPut(OwnerEntry *oe,int mI,DSite* rsite, DSite* bad){
  LockManager* lm=(LockManager*)oe->getTertiary();
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=lm->getChain();
  ch->removeBefore(bad);
  ch->shortcutCrashLock(lm);
  PD((CHAIN,"%d",printChain(ch)));
}

void cellReceiveCantPut(OwnerEntry* oe,TaggedRef val,int mI,DSite* rsite,
                        DSite* badS){
  CellManager* cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=cm->getChain();
  ch->removeBefore(badS);
  ch->shortcutCrashCell(cm,val);
  PD((CHAIN,"%d",printChain(ch)));
}

void chainSendQuestion(DSite* toS,int mI,DSite *deadS){
  OT->getOwner(mI)->getOneCreditOwner();
  PD((ERROR_DET,"chainSendQuestion  %s",toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_QUESTION(mI,myDSite,deadS);
  sendTo(toS,msgC,3);
}

void chainSendAnswer(BorrowEntry* be,DSite* toS, int mI, int ans, DSite *deadS){
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_ANSWER(mI,myDSite,ans,deadS);
  msgC->setImplicitMessageCredit(be->getOneMsgCredit());
  sendTo(toS,msgC,3);
}
