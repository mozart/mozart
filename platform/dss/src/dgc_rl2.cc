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
#pragma implementation "dgc_rl2.hh"
#endif

#include "dgc_rl2.hh"
#include "referenceConsistency.hh"


namespace _dss_internal{


  enum     RL2messageType{
    GC_RLV2_DROP,
    GC_RLV2_INC
  };

    // ******************************** PUBLIC RLV2_HOME *******************************
  RLV2_Home::RLV2_Home(HomeReference *p, GCalgorithm *g):
    HomeGCalgorithm(p,g,RC_ALG_RLV2), SiteHandler(){
  }


  RLV2_Home::~RLV2_Home(){}


  void RLV2_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    insertDSite(dest);
  }

  bool RLV2_Home::m_isRoot(){  return (!isEmpty()); }

  void RLV2_Home::m_getCtlMsg(DSite* msite, MsgContainer* msg){
    RL2messageType mtype = static_cast<RL2messageType>(msg->popIntVal());
    switch(mtype){
    case GC_RLV2_DROP:{
      int dec = msg->popIntVal();
      removeDSite(msite,dec);
      break;
    }
    case GC_RLV2_INC:{
      DSite* rsite = msg->popDSiteVal();
      insertDSite(rsite);
      break;
    }
    default:
      dssError("RLV2_Home: unknown message %d",mtype);
      break;
    }
  }


  void RLV2_Home::m_makeGCpreps(){ gcPreps();}


  // ******************************* PUBLIC RLV2_REMOTE ******************************


  RLV2_Remote::RLV2_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g):
    RemoteGCalgorithm(p,g,RC_ALG_RLV2),decs(0){
  }


  RLV2_Remote::~RLV2_Remote(){}


  void RLV2_Remote::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    MsgContainer *msgC = m_createHomeMsg();
    msgC->pushIntVal(GC_RLV2_INC);
    msgC->pushDSiteVal(dest);
    m_sendToHome(msgC);
  }


  void RLV2_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){ decs++; }


  bool RLV2_Remote::m_isRoot(){
    Assert(decs > 0);
    // ZACHARIAS: Here we have a big problem if some rr_instance isn't sent, else ok
    return false;
  }


  void RLV2_Remote::m_dropReference(){
    Assert(decs > 0);
    MsgContainer *msgC = m_createHomeMsg();
    msgC->pushIntVal(GC_RLV2_DROP);
    msgC->pushIntVal(decs);
    decs = -0xabba;
    m_sendToHome(msgC);
  }


  void RLV2_Remote::m_getCtlMsg(DSite* msite, MsgContainer* msg) {  Assert(0); }



}
