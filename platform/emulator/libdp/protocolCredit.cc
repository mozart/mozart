
/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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
#include "msgType.hh"
#include "dpDebug.hh"
#include "msgbuffer.hh"
#include "protocolCredit.hh"
#include "dpMarshaler.hh"

/**********************************************************************/
/*   Credit protocol                                     */
/**********************************************************************/


void sendPrimaryCredit(DSite *sd,int OTI,Credit c){
  PD((CREDIT,"Sending PrimaryCreds c:%d", c));
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  Assert(creditSiteOut==NULL);
  marshal_M_OWNER_CREDIT(bs,OTI,c); // no msg credit
  SendTo(sd,bs,M_OWNER_CREDIT,sd,OTI);}

void sendSecondaryCredit(DSite *cs,DSite *sd,int OTI,Credit c){
  PD((CREDIT,"Sending SecondaryCreds c:%d", c));
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(cs);
  marshal_M_OWNER_SEC_CREDIT(bs,sd,OTI,c); // no msg credit
  SendTo(cs,bs,M_OWNER_SEC_CREDIT,sd,OTI);}

void sendCreditBack(DSite* sd,int OTI,Credit c){
  int ret;
  if(creditSiteIn==NULL){
    sendPrimaryCredit(sd,OTI,c);
    return;}
  sendSecondaryCredit(creditSiteIn,sd,OTI,c);
  creditSiteIn=NULL;
  return;}
