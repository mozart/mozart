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
  SendTo(na->site,bs,M_UNASK_ERROR,na->site,na->index);}

// caused by installing remote watcher/handler
void receiveAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){  
  PD((NET_HANDLER,"Ask Error Received"));
  Tertiary *t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell: break;
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  Chain *ch=getChainFromTertiary(t);
  if(ch->hasFlag(TOKEN_LOST)){
    PD((NET_HANDLER,"Token Lost"));
    EntityCond tmp=ec & (PERM_SOME|PERM_ME);
    if(tmp != ENTITY_NORMAL){
      sendTellError(oe,toS,t->getIndex(),tmp,true);
      return;}
    ch->newInform(toS,ec);
    ch->dealWithTokenLostBySite(oe,t->getIndex(),toS);
    return;}
  Assert(!(ec & TEMP_ALL));
  if((ch->hasFlag(TOKEN_PERM_SOME)) && (ec & PERM_SOME)){
    PD((NET_HANDLER,"State and q match PERM_SOME"));
    sendTellError(oe,toS,t->getIndex(),PERM_SOME,TRUE);
    return;}
  ch->newInform(toS,ec);
  PD((NET_HANDLER,"Adding Inform Element"));
  if(someTempCondition(ec)){
    if(!ch->hasFlag(INTERESTED_IN_TEMP)){
      ch->setFlagAndCheck(INTERESTED_IN_TEMP);
      ch->probeTemp(t);
      return;}
    else   PD((NET_HANDLER,"Tmp q; allredy interested in that"));
    if(ch->hasFlag(INTERESTED_IN_OK)){
      PD((NET_HANDLER,"Manager is in TmpCond"));
      EntityCond ecS = (ch->getInform())->wouldTrigger(TEMP_SOME|TEMP_ME|TEMP_BLOCKED);
      PD((NET_HANDLER,"ecS %d",ecS));
      if(ecS !=ENTITY_NORMAL)
	sendTellError(oe,toS,t->getIndex(),
		      ecS,TRUE);}}}

void receiveUnAskError(OwnerEntry *oe,DSite *toS,EntityCond ec){ 
  Tertiary* t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  getChainFromTertiary(t)->receiveUnAsk(toS,ec);}



/**********************************************************************/
/*   SECTION 37:: handlers/watchers                       FAILURE            */
/**********************************************************************/

void informInstallHandler(Tertiary* t,EntityCond ec){
  switch(t->getType()){
  case Co_Cell: 
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  if(t->isManager()){
    Chain *ch=getChainFromTertiary(t);
    ch->newInform(myDSite,ec);
    if(someTempCondition(ec) && !ch->hasFlag(INTERESTED_IN_TEMP)){ 
      ch->setFlagAndCheck(INTERESTED_IN_TEMP);
      ch->probeTemp(t);}
    return;}
  sendAskError(t,managerPart(ec));
  if(someTempCondition(ec)) 
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_ALL,t); 
  else
    tertiaryInstallProbe(getSiteFromTertiaryProxy(t),PROBE_TYPE_PERM,t); }


void sendTellError(OwnerEntry *oe,DSite* toS,int mI,EntityCond ec,Bool set){
  if(toS==myDSite){
    receiveTellError(oe->getTertiary(),myDSite,mI,ec,set);
    return;}
  if(SEND_SHORT(toS)) {return;}
  oe->getOneCreditOwner();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_TELL_ERROR(bs,myDSite,mI,ec,set);
  SendTo(toS,bs,M_TELL_ERROR,myDSite,mI);}

void receiveTellError(Tertiary *t,DSite* mS,int mI,EntityCond ec,Bool set){
  if(set){
    if(setEntityCondManager(t,ec)){
      entityProblem(t);}
    return;}
  resetEntityCondManager(t,ec);}







