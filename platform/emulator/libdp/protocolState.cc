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
#include "gc.hh"
#include "protocolState.hh"
#include "perdio.hh"
#include "table.hh"
#include "state.hh"
#include "chain.hh"
#include "controlvar.hh"
#include "protocolFail.hh"
#include "fail.hh"
#include "dsite.hh"

/**********************************************************************/
/*  Failure-interface                       */
/**********************************************************************/

inline Bool tokenLostCheckProxy(Tertiary*t){ 
  if(getEntityCond(t) & PERM_ME){
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
  sendTellError(oe,toS,t->getIndex(),(PERM_BLOCKED|PERM_ME|PERM_SOME),true);
  return;}

/**********************************************************************/
/*   Object protocol                                     */
/**********************************************************************/

void sendObject(DSite* sd, Object *o, Bool sendClass){ // holding one credit
  int OTI = o->getIndex();
  OT->getOwner(OTI)->getOneCreditOwner();
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  if(sendClass){
    marshal_M_SEND_OBJECTANDCLASS(bs,myDSite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECTANDCLASS,myDSite,OTI);}
  else{
    marshal_M_SEND_OBJECT(bs,myDSite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECT,myDSite,OTI);}}

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

void cellLockSendGet(BorrowEntry *be){      
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  installProbeNoRet(toS,PROBE_TYPE_ALL);
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s",na->index,toS->stringrep()));
  marshal_M_CELL_LOCK_GET(bs,na->index,myDSite);
  SendTo(toS,bs,M_CELL_LOCK_GET,toS,na->index);}

void cellLockSendForward(DSite *toS,DSite *fS,int mI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_FORWARD(bs,myDSite,mI,fS);
  SendTo(toS,bs,M_CELL_LOCK_FORWARD,myDSite,mI);}

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
  be->getOneMsgCredit();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_DUMP(bs,na->index,myDSite);
  SendTo(toS,bs,M_CELL_LOCK_DUMP,toS,na->index);}

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
    tmp=unpendCell(pt,tmp);
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
  if((pb->exKind==ACCESS) || (pb->exKind==DEEPAT)){
    TaggedRef aux=unpendCell(pb,val);
    Assert(aux==val);
    pb->dispose();
    pending=pending->next;}
  Assert(secReceiveReadAnsCheck(pending));
}

Bool CellSec::secReceiveRemoteRead(DSite* toS,DSite* mS, int mI){
  switch(state){
  case Cell_Lock_Invalid:
    return NO;
  case Cell_Lock_Valid:{
    cellSendReadAns(toS,mS,mI,contents);
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
      cellSendContents(val,toS,myDSite,cm->getIndex());}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,cm->getIndex());
}

void cellReceiveDump(CellManager *cm,DSite *fromS){
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
  cf->resetDumpBit();
  if(!sec->secForward(toS,val)) return;
  be->getOneMsgCredit();
  cellSendContents(val,toS,mS,mI);
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
  cellSendContents(outval,toS,myDSite,mI);
  return;
}

void cellReceiveContentsFrame(BorrowEntry *be,TaggedRef val,DSite *mS,int mI){ 
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getType()==Co_Cell);
  Assert(cf->isFrame());
  if(tokenLostCheckProxy(cf)) return; // FAIL-HOOK
  be->getOneMsgCredit();
  chainSendAck(mS,mI);    
  CellSec *sec=cf->getCellSec();
  TaggedRef outval;
  DSite *toS;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  be->getOneMsgCredit();
  cellSendContents(outval,toS,mS,mI);
}

void cellReceiveRemoteRead(BorrowEntry *be,DSite* mS,int mI,DSite* fS){ 
  PD((CELL,"Receive REMOTEREAD toS:%s",fS->stringrep()));
  Tertiary* t=be->getTertiary();
  Assert(t->isFrame());
  Assert(t->getType()==Co_Cell);
  CellSec *sec=((CellFrame*)t)->getCellSec();
  be->getOneMsgCredit();
  TaggedRef val;
  if(sec->secReceiveRemoteRead(fS,mS,mI)) return;
  PD((WEIRD,"miss on read"));
  be->getOneMsgCredit();
  cellSendRead(be,fS);
}

void cellReceiveRead(OwnerEntry *oe,DSite* fS){ 
  PD((CELL,"Recevie READ toS:%s",fS->stringrep()));
  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->isManager());
  Assert(cm->getType()==Co_Cell);
  CellSec *sec=cm->getCellSec();
  oe->getOneCreditOwner();
  Chain* ch=cm->getChain();
  if(ch->getCurrent()==myDSite){
    PD((CELL,"Token at mgr site short circuit"));
    sec->secReceiveRemoteRead(fS,myDSite,cm->getIndex());
    return;}
  cellSendRemoteRead(ch->getCurrent(),myDSite,cm->getIndex(),fS);
}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){ 
  Assert((t->isManager())|| (t->isFrame()));
  getCellSecFromTert(t)->secReceiveReadAns(val);
}

/**********************************************************************/
/*   cell send                        */
/**********************************************************************/

void cellSendReadAns(DSite* toS,DSite* mS,int mI,TaggedRef val){ 
  if(toS == myDSite) {
    OwnerEntry *oe=maybeReceiveAtOwner(mS,mI);
    if(mS!=myDSite)
      cellReceiveReadAns(receiveAtBorrow(mS,mI)->getTertiary(),val);
    else
      cellReceiveReadAns(maybeReceiveAtOwner(mS,mI)->getTertiary(),val); 
    return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READANS(bs,mS,mI,val);
  SendTo(toS,bs,M_CELL_READANS,mS,mI);
}

void cellSendRemoteRead(DSite* toS,DSite* mS,int mI,DSite* fS){ 
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_REMOTEREAD(bs,mS,mI,fS);
  SendTo(toS,bs,M_CELL_REMOTEREAD,mS,mI);
}

void cellSendContents(TaggedRef tr,DSite* toS,DSite *mS,int mI){
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_CONTENTS(bs,mS,mI,tr);
  PD((SPECIAL,"CellContents %s",toC(tr)));
  SendTo(toS,bs,M_CELL_CONTENTS,mS,mI);
}

void cellSendRead(BorrowEntry *be,DSite *dS){
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  installProbeNoRet(toS,PROBE_TYPE_ALL);
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READ(bs,na->index,dS);
  SendTo(toS,bs,M_CELL_READ,na->site,na->index);
}

void chainSendAck(DSite* toS, int mI){
  if(SEND_SHORT(toS)) {return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CHAIN,"M_CHAIN_ACK indx:%d site:%s",mI,toS->stringrep()));
  marshal_M_CHAIN_ACK(bs,mI,myDSite);
  SendTo(toS,bs,M_CHAIN_ACK,toS,mI);
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
    TaggedRef val;
    if(lm->getLockSec()->secForward(toS)){
      oe->getOneCreditOwner();
      lockSendToken(myDSite,lm->getIndex(),toS);}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,lm->getIndex());
}

void lockReceiveDump(LockManager* lm,DSite *fromS){
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
  lockSendToken(myDSite,mI,toS);
}
  
void lockReceiveTokenFrame(BorrowEntry* be, DSite *mS,int mI){
  LockFrame *lf=(LockFrame*) be->getTertiary();
  Assert(lf->getType()==Co_Lock);
  Assert(lf->isFrame());
  if(tokenLostCheckProxy(lf)) return; // FAIL-HOOK
  be->getOneMsgCredit();
  chainSendAck(mS,mI);
  LockSec *sec=lf->getLockSec();
  DSite* toS;
  if(sec->secReceiveToken(lf,toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);
}
  
void lockReceiveForward(BorrowEntry *be,DSite *toS,DSite* mS,int mI){
  LockFrame *lf= (LockFrame*) be->getTertiary();
  lf->resetDumpBit();
  Assert(lf->isFrame());
  Assert(lf->getType()==Co_Lock);
  LockSec* sec=lf->getLockSec();
  if(!sec->secForward(toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);
}

/**********************************************************************/
/*  Lock protocol - send                                */
/**********************************************************************/

void lockSendToken(DSite *mS,int mI,DSite* toS){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_LOCK_TOKEN(bs,mS,mI);
  SendTo(toS,bs,M_LOCK_TOKEN,mS,mI);
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
  getChainFromTertiary(t)->receiveAnswer(t,myDSite,answerChainQuestion(t),deadS);}

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
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_CELL_CANTPUT(bs, mI, toS, tr, myDSite);
  SendTo(mS,bs,M_CELL_CANTPUT,mS,mI);
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
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_LOCK_CANTPUT(bs, mI, toS, myDSite);
  SendTo(mS,bs,M_LOCK_CANTPUT,mS,mI);
  return;
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
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_QUESTION(bs,mI,myDSite,deadS);
  SendTo(toS,bs,M_CHAIN_QUESTION,myDSite,mI);
}

void chainSendAnswer(BorrowEntry* be,DSite* toS, int mI, int ans, DSite *deadS){
  be->getOneMsgCredit();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_ANSWER(bs,mI,myDSite,ans,deadS);
  SendTo(toS,bs,M_CHAIN_ANSWER,toS,mI);
}

