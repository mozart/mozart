/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

#include "var.hh"
#include "protocolVar.hh"
#include "msgType.hh"
#include "dpMarshaler.hh"

//
#define UNIFY_ERRORMSG \
   "Unification of distributed variable with term containing resources"

//
OZ_Return sendRedirectToProxies(OldPerdioVar *pv, OZ_Term val,
				DSite* ackSite, int OTI)
{
  ProxyList *pl = pv->getProxies();
  OwnerEntry *oe = OT->getOwner(OTI);
  if (pl) { // perdio vars are not yet localized again
    do {
      DSite* sd = pl->sd;
      if (sd==ackSite) {
	sendAcknowledge(sd,OTI);
      } else {
	OZ_Return ret = sendRedirect(sd,OTI,val);
	if (ret != PROCEED) return ret;
      }
      ProxyList *tmp = pl->next;
      pl->dispose();
      pl = tmp;
    } while (pl);
  }
  return PROCEED;
}

void sendAcknowledge(DSite* sd,int OTI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);  
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,myDSite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,myDSite,OTI);
}

OZ_Return sendSurrender(BorrowEntry *be,OZ_Term val){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_SURRENDER(bs,na->index,myDSite,val);
  CheckNogoods(val,bs,"unify:resources",UNIFY_ERRORMSG,);
  SendTo(na->site,bs,M_SURRENDER,na->site,na->index);
  return PROCEED;
}

OZ_Term sendIsDet(BorrowEntry *be){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  OZ_Term var = OZ_newVariable();
  marshal_M_ISDET(bs,na->index,var);
  SendTo(na->site,bs,M_ISDET,na->site,na->index);
  return var;
}

OZ_Return sendRedirect(DSite* sd,int OTI,TaggedRef val){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_REDIRECT(bs,myDSite,OTI,val);
  CheckNogoods(val,bs,"unify:resources",UNIFY_ERRORMSG,);
  SendTo(sd,bs,M_REDIRECT,myDSite,OTI);
  return PROCEED;
}

void sendRegister(BorrowEntry *be) {
  Assert(creditSiteOut == NULL);
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,myDSite);
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);
}

