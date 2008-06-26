/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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
#pragma implementation "dgc_rc.hh"
#endif

#include "dgc_rc.hh"
#include "referenceConsistency.hh"


namespace _dss_internal{

  
  // ****************************** PUBLIC RC_HOME *******************************

  enum RCmessageType {
    GC_RC_DROP,
    GC_RC_INC_AND_ACK,
    GC_RC_ACK
  }; 
  
  RC_Home::RC_Home(HomeReference *p, GCalgorithm *g):
    HomeGCalgorithm(p,g,RC_ALG_RC),
    counter(0){}


  RC_Home::~RC_Home(){}


  void RC_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    Assert(counter >= 0);
    bs->putByte(0); 
    counter++;
  }
  
  
  bool RC_Home::m_isRoot(){
    Assert(counter >= 0);
    return (counter != 0);
  }
  
  
  void RC_Home::m_getCtlMsg(DSite* msite, MsgContainer* msg){
    Assert(counter >= 0);
    RCmessageType mtype = static_cast<RCmessageType>(msg->popIntVal());
    switch(mtype){
    case GC_RC_DROP:
      {
	int number = msg->popIntVal();
	counter -= number;
	break;
      }
    case GC_RC_INC_AND_ACK:
      {
	counter++;
	DSite* rsite = msg->popDSiteVal();
	MsgContainer *msgC = m_createRemoteMsg();
	msgC->pushIntVal(GC_RC_ACK);
	m_sendToRemote(rsite, msgC);
	break;
      }
    default:
      dssError("RC_Home: unknown message %d",mtype);
      break;
    }
  }


  // ***************************** PUBLIC RC_REMOTE ******************************
  
  RC_Remote::RC_Remote(RemoteReference *p, DssReadBuffer *bs,
		       GCalgorithm *g):RemoteGCalgorithm(p, g,RC_ALG_RC), unacked(0), decs(1){ // no to dec from home
    if(bs->getByte()){
      MsgContainer *msgC = m_createHomeMsg();
      msgC->pushIntVal(GC_RC_INC_AND_ACK);
      DSite* msite = m_getEnvironment()->m_getSrcDSite();
      msgC->pushDSiteVal(msite);
      m_sendToHome(msgC);
    }
  }
  

  RC_Remote::~RC_Remote(){}
  
  void RC_Remote::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    Assert(unacked >= 0); Assert(decs > 0);
    bs->putByte(1); 
    unacked++;
  }


  void RC_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){
    Assert(unacked >= 0); Assert(decs > 0);
    DSite* ssite = m_getEnvironment()->m_getSrcDSite();
    if(bs->getByte()){
      MsgContainer *msgC =  m_createRemoteMsg();
      msgC->pushIntVal(GC_RC_ACK);
      m_sendToRemote(ssite, msgC);
    }
    else{
      decs++;
    }
  }
  

  bool RC_Remote::m_isRoot(){
    Assert(unacked >= 0); Assert(decs > 0);
    return (unacked > 0);
  }
  
  void RC_Remote::m_dropReference(){
    Assert(unacked >= 0); Assert(decs > 0);
    MsgContainer *msgC =  m_createHomeMsg();
    msgC->pushIntVal(GC_RC_DROP);
    msgC->pushIntVal(decs);
    decs = 0;
    m_sendToHome(msgC);
  }
  

  void RC_Remote::m_getCtlMsg(DSite* msite, MsgContainer* msg) {
    Assert(decs > 0);
    RCmessageType mtype = static_cast<RCmessageType>(msg->popIntVal());
    switch(mtype){
    case GC_RC_ACK:
      {
	Assert(unacked > 0);
	unacked--;
	break;
      }
    default:
      dssError("RC_Remote: unknown message %d",mtype);
      break;
    }
  }
}
