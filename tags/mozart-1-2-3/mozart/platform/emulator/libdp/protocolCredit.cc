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

//  void sendCreditTo(DSite *toS,DSite *entitysite,int entityOTI,Credit c) {
//    MsgContainer *msgC = msgContainerManager->newMsgContainer(toS);
  
//    if(c.owner==NULL) {
//      msgC->put_M_OWNER_CREDIT(entityOTI,c.credit);
//    }
//    else {
//      msgC->put_M_OWNER_SEC_CREDIT(entitysite,entityOTI,c.credit); 
//    }

//    send(msgC,-1);
//  }

void sendCreditBack(DSite *entitysite,int entityOTI,Credit c) {
  MsgContainer *msgC;
  
  if(c.owner==NULL) {
    msgC = msgContainerManager->newMsgContainer(entitysite);
    msgC->put_M_OWNER_CREDIT(entityOTI,c.credit);
  }
  else {
    Assert(c.owner!=entitysite);
//      printf("sending M_OWNER_SEC_CREDIT %x %d %d %x\n",
//  	   (int)entitysite,entityOTI,c.credit,(int)c.owner);
    msgC = msgContainerManager->newMsgContainer(c.owner);
    msgC->put_M_OWNER_SEC_CREDIT(entitysite,entityOTI,c.credit); 
  }

  send(msgC,-1);
}

void askForCredit(DSite *entitysite, int entityOTI) {
  Assert(entitysite!=myDSite);

  MsgContainer *msgC = msgContainerManager->newMsgContainer(entitysite);
  msgC->put_M_ASK_FOR_CREDIT(entityOTI,myDSite);
  send(msgC,-1);
}
