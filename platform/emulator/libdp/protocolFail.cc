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
#include "protocolFail.hh"
#include "table.hh"
#include "perdio.hh"
#include "dpMarshaler.hh"
#include "chain.hh"
#include "state.hh"

void sendAskError(Tertiary *t,EntityCond ec){
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_ASK_ERROR(bs,na->index,myDSite,ec);
  SendTo(na->site,bs,M_ASK_ERROR,na->site,na->index);}

void sendUnAskError(Tertiary *t,EntityCond ec){
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_UNASK_ERROR(bs,na->index,myDSite,ec);
  SendTo(na->site,bs,M_UNASK_ERROR,na->site,na->index);
}

void Chain::receiveAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){
  if(hasFlag(TOKEN_LOST)){
    PD((NET_HANDLER,"Token Lost"));
    return;} // automatic inform has already been sent
  EntityCond aux=ENTITY_NORMAL;
  if(hasFlag(TOKEN_PERM_SOME)) aux |= (PERM_SOME & ec);
  if(hasFlag(TOKEN_TEMP_SOME)) aux |= (TEMP_SOME & ec);
  if(aux != ENTITY_NORMAL){
    sendTellError(oe,toS,oe->getTertiary()->getIndex(),aux,TRUE);
    return;}
  newInform(toS,ec);
  PD((NET_HANDLER,"Adding Inform Element"));}

// caused by installing remote watcher/handler
void receiveAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){
  PD((NET_HANDLER,"Ask Error Received"));
  Tertiary *t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:
    getChainFromTertiary(t)->receiveAskError(oe,toS,ec);
    return;
  default: NOT_IMPLEMENTED;}
}

void receiveUnAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){
  Tertiary* t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock:
    getChainFromTertiary(t)->receiveUnAsk(toS,ec);
    return;
  default: NOT_IMPLEMENTED;}
}

/**********************************************************************/

/**********************************************************************/


void sendTellError(OwnerEntry *oe,DSite* toS,int mI,EntityCond ec,Bool set){
  if(toS==myDSite){
    receiveTellError(oe->getTertiary(),ec,set);
    return;}
  if(SEND_SHORT(toS)) {return;}
  oe->getOneCreditOwner();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_TELL_ERROR(bs,myDSite,mI,ec,set);
  SendTo(toS,bs,M_TELL_ERROR,myDSite,mI);}

void receiveTellError(Tertiary *t,EntityCond ec,Bool set){
  if(set){
    if(addEntityCondMsg(t,ec))
      entityProblem(t);
    return;}
  subEntityCondMsg(t,ec);}
