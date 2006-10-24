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
  triggerInforms(getChainFromTertiary(t)->getInformBase(),oe,
		 getEntityCond(t));}

inline void receiveGet_TokenLost(OwnerEntry* oe,DSite* toS,Tertiary* t){  
  PD((ERROR_DET,"TOKEN_LOST message bouncing"));
  sendTellError(oe,toS,(PERM_FAIL|PERM_ALL|PERM_SOME),true);
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

void
cellLockReceiveForward(BorrowEntry *be, DSite* toS, DSite* mS,
		       Ext_OB_TIndex mI)
{
  if(be->getTertiary()->getType()==Co_Cell){
    cellReceiveForward(be, toS, mS, mI);
    return;}
  lockReceiveForward(be, toS, mS, mI);}

void cellLockSendGet(BorrowEntry *be){  
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  //  installProbeNoRet(toS,PROBE_TYPE_ALL);
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s",na->index,toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS, am.currentThread()->getPriority());
  msgC->put_M_CELL_LOCK_GET(na->index,myDSite);
  send(msgC);}

void cellLockSendForward(DSite *toS, DSite *fS, Ext_OB_TIndex mI) {
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_LOCK_FORWARD(myDSite,mI,fS);
  send(msgC);}

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
  send(msgC);}

/**********************************************************************/
/*   Cell protocol - receive                            */
/**********************************************************************/

//
Bool CellSec::secForward(DSite* toS,TaggedRef &val)
{
  // kost@ : actually, manager site can be only 'Cell_Lock_Valid';
  if (state & Cell_Lock_Valid) {
    Assert(state == Cell_Lock_Valid || 
	   state == Cell_Lock_Valid|Cell_Lock_Dump_Asked);
    // 'Cell_Lock_Dump_Asked' is dropped, if any: the
    // 'BorrowTable::closeFrameToProxy' can proceed;
    state = Cell_Lock_Invalid;
    val = contents;
    return (OK);
  } else {
    Assert(state == Cell_Lock_Requested || 
	   state == Cell_Lock_Requested|Cell_Lock_Dump_Asked);
    // kost@ : 'Cell_Lock_Dump_Asked' is kept just to make sure we
    // don't send the 'take away the state from me' request again;
    state |= Cell_Lock_Next;
    next = toS;
    return (NO);
  }
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
  if (state & Cell_Lock_Next) {
    // 'Cell_Lock_Dump_Asked' is dropped, if any;
    state = Cell_Lock_Invalid;
    toS = next;
    return (OK);
  }
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

Bool CellSec::secReceiveRemoteRead(DSite* toS, DSite* mS, Ext_OB_TIndex mI)
{
  switch(state){
  case Cell_Lock_Invalid:
    return NO;
  case Cell_Lock_Valid:{
    cellSendReadAns(toS,mS,mI,contents);
    return TRUE;}
  case Cell_Lock_Requested:
  case Cell_Lock_Requested|Cell_Lock_Dump_Asked:
    pendThreadAddRAToEnd(&pending,toS,mS,mI);
    return (TRUE);
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
      cellSendContents(val, toS, myDSite, oe->getExtOTI());}
    return;}
  cellLockSendForward(current, toS, oe->getExtOTI());
}

void cellReceiveDump(CellManager *cm,DSite *fromS){
  if(tokenLostCheckManager(cm)) return; // FAIL-HOOK
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  
  if ((cm->getChain()->getCurrent() != fromS) ||
      (cm->getState() != Cell_Lock_Invalid)) {
    // kost@ : there is nothing weird here: the guy who's asked to
    // dump the state has got also a forward request, which it will
    // process in an ordinary fashion.
    //   PD((WEIRD,"CELL dump not needed"));
    ;
  } else {
    getCellSecFromTert(cm)->dummyExchange(cm);
  }
}
// MERGECON  (void) cellDoExchangeInternal((Tertiary *)cm,tr,tr,DummyThread,EXCHANGE);

void
cellReceiveForward(BorrowEntry *be, DSite *toS, DSite* mS, Ext_OB_TIndex mI)
{
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->isFrame());
  Assert(cf->getType()==Co_Cell);
  CellSec *sec=cf->getCellSec();
  TaggedRef val;
  if(!sec->secForward(toS,val)) return;
  cellSendContents(val, toS, mS, mI);
  return;
}

void
cellReceiveContentsManager(OwnerEntry *oe, TaggedRef val, Ext_OB_TIndex mI)
{
  CellManager *cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  if(tokenLostCheckManager(cm)) return; // FAIL-HOOK
  chainReceiveAck(oe,myDSite);
  CellSec *sec=cm->getCellSec();
  DSite *toS;
  TaggedRef outval;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  cellSendContents(outval, toS, myDSite, mI);
  return;
}

void
cellReceiveContentsFrame(BorrowEntry *be, TaggedRef val, DSite *mS,
			 Ext_OB_TIndex mI)
{
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getType()==Co_Cell);
  Assert(cf->isFrame());
  if(tokenLostCheckProxy(cf)) return; // FAIL-HOOK
  chainSendAck(mS,mI);    
  CellSec *sec=cf->getCellSec();
  TaggedRef outval;
  DSite *toS;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  cellSendContents(outval, toS, mS, mI);
}

void cellReceiveRemoteRead(BorrowEntry *be,
			   DSite* mS, Ext_OB_TIndex mI, DSite* fS)
{ 
  PD((CELL,"Receive REMOTEREAD toS:%s",fS->stringrep()));
  Tertiary* t=be->getTertiary();
  Assert(t->isFrame());
  Assert(t->getType()==Co_Cell);
  CellSec *sec=((CellFrame*)t)->getCellSec();
  if(sec->secReceiveRemoteRead(fS,mS,mI)) return;
  PD((WEIRD,"miss on read"));
  cellSendRead(be,fS);
}

void cellReceiveRead(OwnerEntry *oe, DSite* fS, DSite *cS)
{
  PD((CELL,"Recevie READ toS:%s",fS->stringrep()));
  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->isManager());
  Assert(cm->getType()==Co_Cell);
  CellSec *sec=cm->getCellSec();
  Chain* ch=cm->getChain();
  if(ch->getCurrent()==myDSite){
    PD((CELL,"Token at mgr site short circuit"));
    //  kost@ 'CellSec::secReceiveRemoteRead()' signature ??!
    //  was like that:
    // sec->secReceiveRemoteRead(fS,myDSite,MakeOB_TIndex(cm->getTertPointer()));
    sec->secReceiveRemoteRead(fS, myDSite, oe->getExtOTI());
  } else {
    //  kost@ 'CellSec::cellSendRemoteRead()' signature ??!
    //  was like that:
    // cellSendRemoteRead(ch->getCurrent(),myDSite,
    //		          MakeOB_TIndex(cm->getTertPointer()),fS,cS);
    cellSendRemoteRead(ch->getCurrent(), myDSite, oe->getExtOTI(), fS, cS);
  }
}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){ 
  Assert((t->isManager())|| (t->isFrame()));
  getCellSecFromTert(t)->secReceiveReadAns(val);
}

/**********************************************************************/
/*   cell send                        */
/**********************************************************************/

void cellSendReadAns(DSite* toS, DSite* mS, Ext_OB_TIndex mI, TaggedRef val)
{ 
  if(toS == myDSite) {
    OwnerEntry *oe=maybeReceiveAtOwner(mS,mI);
    if(mS!=myDSite)
      cellReceiveReadAns(receiveAtBorrow(mS,mI)->getTertiary(),val);
    else
      cellReceiveReadAns(maybeReceiveAtOwner(mS,mI)->getTertiary(),val); 
    return;}
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_READANS(mS,mI,val);
  send(msgC);
}

void cellSendRemoteRead(DSite* toS, DSite* mS, Ext_OB_TIndex mI,
			DSite* fS, DSite *cS)
{
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_REMOTEREAD(mS,mI,fS);
  send(msgC);
}

void cellSendContents(TaggedRef tr, DSite* toS, DSite *mS, Ext_OB_TIndex mI)
{
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  PD((SPECIAL,"CellContents %s",toC(tr)));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_CONTENTS(mS,mI,tr);
  send(msgC);
}

void cellSendRead(BorrowEntry *be,DSite *dS){
  NetAddress *na=be->getNetAddress();
  DSite *toS=na->site;
  // installProbeNoRet(toS,PROBE_TYPE_ALL);
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_READ(na->index,dS);
  send(msgC);
}

void chainSendAck(DSite* toS, Ext_OB_TIndex mI)
{
  if(SEND_SHORT(toS)) {return;}
  PD((CHAIN,"M_CHAIN_ACK indx:%d site:%s",mI,toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_ACK(mI,myDSite);
  send(msgC);
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

//
Bool LockSec::secReceiveToken(Tertiary* t,DSite* &toS)
{
  state = Cell_Lock_Valid | (state & (Cell_Lock_Next|Cell_Lock_Dump_Asked));

  // kost@ : three types of pending: valid thread(s), a "move" request
  // or a dead thread. Keep the lock if there are no "move" requests.
  while (pending != NULL) {
    if (pending->thread != NULL) {
      locker=pendThreadResumeFirst(&pending);
      break;
    } else if (pending->exKind == MOVEEX) {
      // kost@ : nothing weird here: e.g. the thread has been killed.
      //   PD((WEIRD,"lock requested but not used"));
      Assert(state == Cell_Lock_Next|Cell_Lock_Valid);
      // kost@ : drop 'Cell_Lock_Dump_Asked', if any;
      state = Cell_Lock_Invalid;
      toS = next;
      return (NO);
    } else {
      pendThreadRemoveFirst(getPendBase());
    }
  }

  Assert(state & Cell_Lock_Valid);
  return (OK);
}

Bool LockSec::secForward(DSite* toS)
{
  if (state & Cell_Lock_Valid) {
    if (locker == NULL) {
      state = Cell_Lock_Invalid;
      return (OK);
    } else {
      state |= Cell_Lock_Next;
      pendThreadAddMoveToEnd(getPendBase());
      next = toS;
      return (NO);
    }
  } else {
    Assert(state == Cell_Lock_Requested ||
	   state == Cell_Lock_Requested|Cell_Lock_Dump_Asked);
    state |= Cell_Lock_Next;	// keep 'Cell_Lock_Dump_Asked';
    pendThreadAddMoveToEnd(getPendBase());
    next = toS;
    return (NO);
  }
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
      lockSendToken(myDSite, oe->getExtOTI(), toS);}
    return;}
  cellLockSendForward(current, toS, oe->getExtOTI());
}

void lockReceiveDump(LockManager* lm,DSite *fromS){
  if(tokenLostCheckManager(lm)) return; // FAIL-HOOK
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  LockSec* sec=lm->getLockSec();

  if ((lm->getChain()->getCurrent() != fromS) || 
      (sec->getState() != Cell_Lock_Invalid)) {
    // kost@ : ditto, nothing weird here: the proxy"s "dump" request
    // has been effectively invalidated by a "forward" request sent to
    // that proxy;
    //   PD((WEIRD,"WEIRD- LOCK dump not needed"));
    ;
  } else {
    Assert(sec->getState() == Cell_Lock_Invalid);
    pendThreadAddDummyToEnd(sec->getPendBase());
    secLockGet(sec,lm,NULL);
  }
}

void lockReceiveTokenManager(OwnerEntry* oe, Ext_OB_TIndex mI)
{
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
  lockSendToken(myDSite, oe->getExtOTI(), toS);
}
  
void lockReceiveTokenFrame(BorrowEntry* be, DSite *mS, Ext_OB_TIndex mI)
{
  LockFrame *lf=(LockFrame*) be->getTertiary();
  Assert(lf->getType()==Co_Lock);
  Assert(lf->isFrame());
  if(tokenLostCheckProxy(lf)) return; // FAIL-HOOK
  chainSendAck(mS,mI);
  LockSec *sec=lf->getLockSec();
  DSite* toS;
  if(sec->secReceiveToken(lf,toS)) return;
  lockSendToken(mS, mI, toS);
}
  
void lockReceiveForward(BorrowEntry *be, DSite *toS, DSite* mS,
			Ext_OB_TIndex mI)
{
  LockFrame *lf= (LockFrame*) be->getTertiary();
  Assert(lf->isFrame());
  Assert(lf->getType()==Co_Lock);
  LockSec* sec=lf->getLockSec();
  if(!sec->secForward(toS)) return;
  lockSendToken(mS, mI, toS);
}

/**********************************************************************/
/*  Lock protocol - send                                */
/**********************************************************************/

void lockSendToken(DSite *mS, Ext_OB_TIndex mI, DSite* toS) {
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_LOCK_TOKEN(mS,mI);
  send(msgC);
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
  case Cell_Lock_Requested|Cell_Lock_Next|Cell_Lock_Dump_Asked:
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested|Cell_Lock_Dump_Asked:
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

void chainReceiveQuestion(BorrowEntry *be, DSite* site,
			  Ext_OB_TIndex extOTI, DSite* deadS)
{
  if(be==NULL){
    chainSendAnswer(be,site,extOTI,PAST_ME,deadS);}
  chainSendAnswer(be,site,extOTI,answerChainQuestion(be->getTertiary()),deadS);
}

void chainReceiveAnswer(OwnerEntry* oe, DSite* fS, int ans, DSite* deadS)
{
  Tertiary* t=oe->getTertiary();
  getChainFromTertiary(t)->receiveAnswer(t,fS,ans,deadS);
  PD((CHAIN,"%d",printChain(getChainFromTertiary(t))));
}

void maybeChainSendQuestion(ChainElem *ce,Tertiary *t,DSite* deadS)
{
  if(ce->getSite()!=myDSite){
    if(!(ce->flagIsSet(CHAIN_QUESTION_ASKED))){
      ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
      //  kost@ 'chainSendQuestion()' signature ??!
      //  was like that:
      // chainSendQuestion(ce->getSite(),MakeOB_TIndex(t->getTertPointer()),deadS);
      OB_TIndex ti = MakeOB_TIndex(t->getTertPointer());
      chainSendQuestion(ce->getSite(),ownerEntry2extOTI(ti),deadS);
    }
  } else {
    Chain *ch=getChainFromTertiary(t);
    ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
    ch->receiveAnswer(t,myDSite,answerChainQuestion(t),deadS);
  }
}

/**********************************************************************/
/*   failure-related                                         */
/**********************************************************************/

Bool CellSec::cellRecovery(TaggedRef tr){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    contents=tr;
    return NO;}
  Assert(state == Cell_Lock_Requested ||
	 state == Cell_Lock_Requested|Cell_Lock_Dump_Asked);
  return OK;
}

Bool LockSec::lockRecovery(){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    locker=NULL;
    return NO;}
  state &= ~Cell_Lock_Next;
  Assert(state == Cell_Lock_Requested ||
	 state == Cell_Lock_Requested|Cell_Lock_Dump_Asked);
  return OK;
}

static void cellManagerIsDown(TaggedRef tr, DSite* mS, Ext_OB_TIndex mI)
{
  BorrowEntry *be = BT->find(mI, mS);
  if(be==NULL) return; // has been gced 
  Tertiary* t=be->getTertiary();
  maybeConvertCellProxyToFrame(t);
  if(((CellFrame*)t)->getCellSec()->cellRecovery(tr)){
    cellReceiveContentsFrame(be, tr, mS, mI);}
}

static void lockManagerIsDown(DSite* mS, Ext_OB_TIndex mI)
{
  BorrowEntry *be = BT->find(mI, mS);
  if(be==NULL) return; // has been gced 
  Tertiary* t=be->getTertiary();
  maybeConvertLockProxyToFrame(t);
  if(((LockFrame*)t)->getLockSec()->lockRecovery()){  
    lockReceiveTokenFrame(be, mS, mI);}
}

static
void cellSendCantPut(TaggedRef tr, DSite* toS, DSite *mS, Ext_OB_TIndex mI)
{
  PD((ERROR_DET,"Proxy cant put to %s site: %s:%d",
      toS->stringrep(), mS->stringrep(),mI));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CELL_CANTPUT(mI, toS, tr, myDSite);
  send(msgC);
}

void cellSendContentsFailure(TaggedRef tr, DSite* toS, DSite *mS,
			     Ext_OB_TIndex mI)
{
  if(toS==mS) {// ManagerSite is down
    cellManagerIsDown(tr,toS,mI);
    return;}
  if(mS==myDSite){// At managerSite 
    cellReceiveCantPut(OT->extOTI2ownerEntry(mI),tr,mI,mS,toS);
    return;}  
  cellSendCantPut(tr,toS,mS,mI);
  return;
}

void lockSendCantPut(DSite* toS, DSite *mS, Ext_OB_TIndex mI)
{
  PD((ERROR_DET,"Proxy cant put - to %s site: %s:%d Nr %d",
      toS->stringrep(),mS->stringrep(),mI));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(mS);
  msgC->put_M_LOCK_CANTPUT(mI, toS, myDSite);
  send(msgC);
}

void lockSendTokenFailure(DSite* toS, DSite *mS, Ext_OB_TIndex mI)
{
  PD((ERROR_DET,"LockTokenFailure"));
  if (toS==mS) {// ManagerSite is down
    lockManagerIsDown(mS, mI);
    return;}
  if (mS==myDSite) {// At managerSite 
    //  kost@ 'lockReceiveCantPut()' signature ??!
    //  was like that:
    // lockReceiveCantPut(ownerIndex2ownerEntry(mI),mI,mS,toS);
    lockReceiveCantPut(OT->extOTI2ownerEntry(mI), mI, mS, toS);
  } else {
    lockSendCantPut(toS,mS,mI);
  }
}

/**********************************************************************/
/*   STATE & FAILURE                        */
/**********************************************************************/

void lockReceiveCantPut(OwnerEntry *oe, Ext_OB_TIndex mI,
			DSite* rsite, DSite* bad){ 
  LockManager* lm=(LockManager*)oe->getTertiary();
  Assert(lm->getType()==Co_Lock);
  Assert(lm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=lm->getChain();
  ch->removeBefore(bad);
  ch->shortcutCrashLock(lm);
  PD((CHAIN,"%d",printChain(ch)));
}

void cellReceiveCantPut(OwnerEntry* oe, TaggedRef val, Ext_OB_TIndex mI,
			DSite* rsite, DSite* badS){ 
  CellManager* cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->isManager());
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=cm->getChain();
  ch->removeBefore(badS);
  ch->shortcutCrashCell(cm,val);
  PD((CHAIN,"%d",printChain(ch)));
}

void chainSendQuestion(DSite* toS, Ext_OB_TIndex mI, DSite *deadS) {
  PD((ERROR_DET,"chainSendQuestion  %s",toS->stringrep()));
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_QUESTION(mI,myDSite,deadS);
  send(msgC);
}

void chainSendAnswer(BorrowEntry* be, DSite* toS, Ext_OB_TIndex mI,
		     int ans, DSite *deadS)
{
  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_CHAIN_ANSWER(mI,myDSite,ans,deadS);
  send(msgC);
}

