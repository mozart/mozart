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
 *    Erik Klintskog, 1998
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
#include "dpBase.hh"
#include "fail.hh"
#include "protocolFail.hh"
#include "table.hh"
#include "perdio.hh"
#include "msgContainer.hh"
#include "chain.hh"
#include "state.hh"
#include "var.hh"
#include "var_obj.hh"

void sendAskError(BorrowEntry* be,EntityCond ec){ 
  NetAddress* na=be->getNetAddress();

  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_ASK_ERROR(na->index,myDSite,ec);

  send(msgC);
}

void sendUnAskError(BorrowEntry *be,EntityCond ec){
  NetAddress* na=be->getNetAddress();

  MsgContainer *msgC = msgContainerManager->newMsgContainer(na->site);
  msgC->put_M_UNASK_ERROR(na->index,myDSite,ec);

  send(msgC);
}

void Chain::receiveAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){  
  if(hasFlag(TOKEN_LOST)){
    PD((NET_HANDLER,"Token Lost"));
    sendTellError(oe,toS,PERM_SOME|PERM_FAIL,TRUE);
    return;} // automatic inform has already been sent
  EntityCond aux=ENTITY_NORMAL;
  if(hasFlag(TOKEN_PERM_SOME)) aux |= (PERM_SOME & ec);
  if(hasFlag(TOKEN_TEMP_SOME)) aux |= (TEMP_SOME & ec);    
  if(aux != ENTITY_NORMAL){
    sendTellError(oe,toS,aux,TRUE);
    return;}
  newInform(toS,ec); 
  PD((NET_HANDLER,"Adding Inform Element"));}

// caused by installing remote watcher/injector
void receiveAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){  
  PD((NET_HANDLER,"Ask Error Received"));
  if(oe->isTertiary()){
    Tertiary *t=oe->getTertiary();
    switch(t->getType()){
    case Co_Cell: 
    case Co_Lock: 
      getChainFromTertiary(t)->receiveAskError(oe,toS,ec);
      return;
    default: Assert(0);}}
  if(oe->isRef()) return;
  Assert(oe->isVar());
  ManagerVar* mv=GET_VAR(oe,Manager);
  if(mv->getEntityCond() & ec){
    sendTellError(oe,toS,mv->getEntityCond() & ec,TRUE);}
  mv->newInform(toS,ec); 
}
 
void receiveUnAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){ 
  Tertiary* t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: 
    getChainFromTertiary(t)->receiveUnAsk(toS,ec);
    return;
  default: Assert(0);}
}

/**********************************************************************/

/**********************************************************************/
static
void receiveTellErrorTert(Tertiary *t,EntityCond ec,Bool set){
  if(set){
    if(addEntityCond(t,ec))
      entityProblem(t);
    return;}
  subEntityCond(t,ec);}

static
void  receiveTellErrorVar(BorrowEntry*b,EntityCond ec,Bool set){
  if(typeOfBorrowVar(b)==VAR_PROXY){
    if(set){
      GET_VAR(b,Proxy)->addEntityCond(ec);}
    else{
      GET_VAR(b,Proxy)->subEntityCond(ec);}
    return;}
  Assert(typeOfBorrowVar(b)==VAR_LAZY);
  if(set){
    GET_VAR(b, Lazy)->addEntityCond(ec);}
  else{
    GET_VAR(b, Lazy)->subEntityCond(ec);}
}

void receiveTellError(BorrowEntry* b,EntityCond ec,Bool set){
  if(b->isTertiary()){
    receiveTellErrorTert(b->getTertiary(),ec,set);
    return;}
  Assert(b->isVar());
  receiveTellErrorVar(b,ec,set);}

void sendTellError(OwnerEntry *oe,DSite* toS,EntityCond ec,Bool set){
  if(toS==myDSite){
    // kost@ : Erik told me the assertion is wrong.
    // Assert(0); // PER-LOOK is this possible
    receiveTellErrorTert(oe->getTertiary(),ec,set);
    return;}
  if(SEND_SHORT(toS)) {return;}

  MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  msgC->put_M_TELL_ERROR(myDSite, oe->getExtOTI(), ec, set);

  send(msgC);
}

    














