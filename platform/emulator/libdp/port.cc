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

#if defined(INTERFACE)
#pragma implementation "port.hh"
#endif

#include "base.hh"
#include "builtins.hh"
#include "value.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "port.hh"
#include "table.hh"
#include "controlvar.hh"
#include "dpMarshaler.hh"
#include "dpInterface.hh"

int PortSendTreash = 100000;
int PortWaitTimeSlice = 800;
int PortWaitTimeK = 1;

/**********************************************************************/
/*   SECTION Port protocol                                       */
/**********************************************************************/

/* PER-HANDLE
EntityCond getEntityCondPort(Tertiary* p){
  EntityCond ec = getEntityCond(p);
  int dummy;
  if(ec!=ENTITY_NORMAL)return ec;
  if(getSiteFromTertiaryProxy(p)->getQueueStatus(dummy)>=PortSendTreash)
    return TEMP_BLOCKED|TEMP_ME;
  return ENTITY_NORMAL;}
*/

EntityCond getEntityCondPort(Tertiary* p){
  return ENTITY_NORMAL;}

/**********************************************************************/
/*   SECTION Port protocol                                       */
/**********************************************************************/

OZ_Return portWait(int queueSize, int restTime, Tertiary *t);

OZ_BI_define(BIportWait,2,0)
{
   oz_declareIN(0,prt);
   Assert(oz_isPort(prt));
   oz_declareIntIN(1,t);
   Tertiary *tert = tagged2Tert(prt);
   int dummy;
   return portWait(getSiteFromTertiaryProxy(tert)-> getQueueStatus(dummy),
		   t,tert);
} OZ_BI_end

TaggedRef BI_portWait;

OZ_Return portWait(int queueSize, int restTime, Tertiary *t)
{
  PD((ERROR_DET,"PortWait q: %d r: %d", queueSize, restTime));
  int v = queueSize - PortSendTreash;
  int time;
  if(v<0) return PROCEED;
  int tot=PortWaitTimeK * queueSize;
  if(restTime && restTime < tot) tot = restTime;
  if (v < PortWaitTimeSlice) {
    time = v; 
  } else {
    time = PortWaitTimeSlice;
    am.prepareCall(BI_portWait,makeTaggedTert(t),
		   oz_int(tot - PortWaitTimeSlice));
  }
  am.prepareCall(BI_Delay,oz_int(time));
  return BI_REPLACEBICALL;
}

OZ_Return portSendImpl(Tertiary *p, TaggedRef msg) 
{
  Assert(p->getTertType()==Te_Proxy);
  BorrowEntry* b = BT->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  DSite* site     = na->site;
  int index      = na->index;
  int dummy;
  Bool wait = FALSE;
  
  switch(getEntityCondPort(p)){
  case PERM_BLOCKED|PERM_ME:{
    /* PER-HANDLE
    PD((ERROR_DET,"Port is PERM"));
    if(p->startHandlerPort(oz_currentThread(), p, msg, PERM_BLOCKED|PERM_ME))
      return BI_REPLACEBICALL;
    ControlVarNew(var,p->getBoardInternal());
    SuspendOnControlVar;
    */
  }
  case TEMP_BLOCKED|TEMP_ME:{
    /* PER-HANDLE
    PD((ERROR_DET,"Port is Tmp size:%d treash:%d",
	site->getQueueStatus(dummy),PortSendTreash));
    wait = TRUE;
    if(p->startHandlerPort(oz_currentThread(), p, msg, TEMP_BLOCKED|TEMP_ME))
      return BI_REPLACEBICALL;
    break;
    */
  }
  case ENTITY_NORMAL: break;
  default: Assert(0);
  }

  MsgBuffer *bs=msgBufferManager->getMsgBuffer(site);
  b->getOneMsgCredit();
  marshal_M_PORT_SEND(bs,index,msg);
  
  OZ_Term nogoods = bs->getNoGoods();
  if (!literalEq(oz_nil(),nogoods)) {
  /*
    int portIndex;
    OZ_Term t;
    unmarshal_M_PORT_SEND(bs,portIndex,t);
    dumpRemoteMsgBuffer(bs);
    */
    return raiseGeneric("portSend:resources",
			"Resources found during send to port",
			oz_mklist(OZ_pairA("Resources",nogoods),
				  OZ_pairA("Port",makeTaggedTert(p))));
  }
  
  PD((PORT,"sendingTo %s %d",site->stringrep(),index));
  SendTo(site,bs,M_PORT_SEND,site,index);
  return wait ? portWait(site->getQueueStatus(dummy), 0, p)
    : PROCEED;
}

//
void gcDistPortRecurseImpl(Tertiary *p) 
{
  Assert(!p->isLocal());
  //
  gcEntityInfoImpl(p);
  if (p->isProxy()) {
    gcProxy(p);
  } else {
    gcManager(p);
    PortWithStream *pws = (PortWithStream *) p;
    OZ_collectHeapTerm(pws->strm,pws->strm);
  }
}


