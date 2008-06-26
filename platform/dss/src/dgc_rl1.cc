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
#pragma implementation "dgc_rl1.hh"
#endif

#include "dgc_rl1.hh"
#include "referenceConsistency.hh"


namespace _dss_internal{

  enum RL1messageType{
    GC_RLV1_DROP,
    GC_RLV1_INC_AND_ACK,
    GC_RLV1_ACK_FROM_HOME,
    GC_RLV1_ACK_FROM_REMOTE
  };

  // ******************************** PUBLIC RLV1_HOME *******************************

  RLV1_Home::RLV1_Home(HomeReference *p, GCalgorithm *g):HomeGCalgorithm(p,g,RC_ALG_RLV1), SiteHandler(){
    dssError("Improve reference listings sitehandling ZACHARIAS\n");
    //
    //  RL one does not need the counting of sites since the site itself
    //  notifies the home, thus just deletion and insertion of sites is
    //  required which optimizes the behavior. Implement a new Sitehandler
    //  and DSiteContainer for this version
    //
  }


  RLV1_Home::~RLV1_Home(){}

  void RLV1_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite *dest){
    insertDSite(dest);
  }


  bool RLV1_Home::m_isRoot(){
    return (!isEmpty());
  }


  void RLV1_Home::m_getCtlMsg(DSite* msite, MsgContainer* msg){

    RL1messageType mtype = static_cast<RL1messageType>(msg->popIntVal());
    switch(mtype){
    case GC_RLV1_DROP:{
      int dec = msg->popIntVal();
      removeDSite(msite,dec);
      break;
    }
    case GC_RLV1_INC_AND_ACK:{
      DSite* rsite = msg->popDSiteVal();
      insertDSite(msite);
      MsgContainer *msgC = m_createRemoteMsg();
      msgC->pushIntVal(GC_RLV1_ACK_FROM_HOME);
      msgC->pushDSiteVal(msite);
      rsite->m_sendMsg(msgC);
      break;
    }
    default:
      Assert(0);
      dssError("RLV1_Home: unknown message %d",mtype);
      break;
    }
  }


  void RLV1_Home::m_makeGCpreps(){ gcPreps();}



  // ******************************* PUBLIC RLV1_REMOTE ******************************

  RLV1_Remote::RLV1_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g):
    RemoteGCalgorithm(p,g,RC_ALG_RLV1), SiteHandler(),decs(1){
    if (!m_isHomeSite(m_getEnvironment()->m_getSrcDSite())){
      MsgContainer *msgC = m_createHomeMsg();
      msgC->pushIntVal(GC_RLV1_INC_AND_ACK);
      msgC->pushDSiteVal(m_getEnvironment()->m_getSrcDSite());
      m_sendToHome(msgC);
    }
  }


  RLV1_Remote::~RLV1_Remote(){}


  void RLV1_Remote::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    insertDSite(dest);
  }


  void RLV1_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){
    DSite *src = m_getEnvironment()->m_getSrcDSite();
    if(m_isHomeSite(src))
       decs++;
    else {
      MsgContainer *msgC = m_createRemoteMsg();
      msgC->pushIntVal(GC_RLV1_ACK_FROM_REMOTE);
      m_sendToRemote(src,msgC);
    }
  }


  bool RLV1_Remote::m_isRoot(){
    Assert(decs > 0);
    return (!isEmpty());
  }

  void RLV1_Remote::m_dropReference(){
    Assert(decs > 0);
    MsgContainer *msgC = m_createHomeMsg();
    msgC->pushIntVal(GC_RLV1_DROP);
    msgC->pushIntVal(decs);
    decs = 0;
    m_sendToHome(msgC);
  }


  void RLV1_Remote::m_getCtlMsg(DSite* msite, MsgContainer* msg) {
    Assert(decs > 0);
    RL1messageType mtype = static_cast<RL1messageType>(msg->popIntVal());
    switch(mtype){
    case GC_RLV1_ACK_FROM_HOME:{
      DSite* rsite = msg->popDSiteVal();
      removeDSite(rsite,1);
      break;
    }
    case GC_RLV1_ACK_FROM_REMOTE:
      removeDSite(msite,1);
      break;
    default:
      Assert(0);
      dssError("RLV1_Remote: unknown message %d",mtype);
      break;
    }
  }


  void RLV1_Remote::m_makeGCpreps(){ gcPreps(); }






}
