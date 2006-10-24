/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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
//#include "dpMarshaler.hh"
#include "dpInterface.hh"
#include "ozconfig.hh"
#include "msgContainer.hh"

/**********************************************************************/
/*   SECTION Port protocol                                       */
/**********************************************************************/

OZ_Return portSendInternal(Tertiary *p, TaggedRef msg){
  Assert(p->getTertType()==Te_Proxy);
  BorrowEntry* b  = borrowIndex2borrowEntry(MakeOB_TIndex(p->getTertPointer()));
  NetAddress *na  = b->getNetAddress();
  DSite* site     = na->site;
  Ext_OB_TIndex index = na->index;
  
  MsgContainer *msgC = msgContainerManager->newMsgContainer(site,am.currentThread()->getPriority());
  msgC->put_M_PORT_SEND(index,msg);
  
  PD((PORT,"sendingTo %s %d",site->stringrep(),index));
  send(msgC);
  if (site->getQueueStatus() < ozconf.dpFlowBufferSize)  
    {
      return PROCEED;
    }
  else
    {
      ControlVarNew(controlvar,oz_rootBoard());
      msgC->setCntrlVar(controlvar);
      return suspendOnControlVar();
    }
}

//

OZ_Return portSendImpl(Tertiary *p, TaggedRef msg) 
{
  Assert(p->getTertType()==Te_Proxy);
  if(getEntityCond(p)!= ENTITY_NORMAL){
    pendThreadAddToEnd(&(((PortProxy*)p)->pending),
		       msg,msg,NOEX);
    deferEntityProblem(p);
    return SuspendOnControlVarReturnValue;
  }
  return portSendInternal(p,msg);
}


void gcDistPortRecurseImpl(Tertiary *p) 
{
  Assert(!p->isLocal());
  //
  gcEntityInfoImpl(p);
  if (p->isProxy()) {
    gcProxyRecurseImpl(p);
    gCollectPendThread(&(((PortProxy*)p)->pending));
  } else {
    gcManagerRecurseImpl(p);
    PortWithStream *pws = (PortWithStream *) p;
    
    oz_gCollectTerm(pws->strm,pws->strm);
  }
}


/**************************************************************/
/*      Failure                                               */
/**************************************************************/


void port_Temp(PortProxy* pp){
  EntityCond ec = TEMP_FAIL;
  if(!addEntityCond(pp,ec)) return;
  entityProblem(pp);     
}
void port_Ok(PortProxy* pp){
  EntityCond ec = TEMP_FAIL;
  subEntityCond(pp,ec);
}

void port_Perm(PortProxy* pp){
  EntityCond ec = PERM_FAIL;
  if(!addEntityCond(pp,ec)) return;
  entityProblem(pp);     
}
PendThread* getPendThreadStartFromPort(Tertiary* t){
    return ((PortProxy*)t)->pending;}











