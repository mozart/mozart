/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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
#include "dpBase.hh"
#include "msgType.hh"
#include "dpDebug.hh"
#include "msgContainer.hh"
#include "protocolCredit.hh"

/**********************************************************************/
/*   Credit protocol                                     */
/**********************************************************************/


void sendPrimaryCredit(DSite *sd,int OTI,Credit c){
  PD((CREDIT,"Sending PrimaryCreds c:%d", c));
  Assert(creditSiteOut==NULL);

  MsgContainer *msgC = msgContainerManager->newMsgContainer(sd);
  msgC->put_M_OWNER_CREDIT(OTI,c);
  sendTo(sd,msgC,3);
}

void sendSecondaryCredit(DSite *cs,DSite *sd,int OTI,Credit c){
  PD((CREDIT,"Sending SecondaryCreds c:%d", c));

  MsgContainer *msgC = msgContainerManager->newMsgContainer(cs);
  msgC->put_M_OWNER_SEC_CREDIT(sd,OTI,c); // no msg credit

  sendTo(cs,msgC,3);
}

void sendCreditBack(DSite* sd,int OTI,Credit c){
  if(creditSiteIn==NULL){
    sendPrimaryCredit(sd,OTI,c);
    return;}
  sendSecondaryCredit(creditSiteIn,sd,OTI,c);
  creditSiteIn=NULL;
  return;}
