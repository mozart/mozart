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
#pragma implementation "dgc_irc.hh"
#endif

#include "dgc_irc.hh"
#include "referenceConsistency.hh"
#include "msl_serialize.hh"

namespace _dss_internal{

  enum IRCmessageType{
    GC_IRC_DROP,
    GC_IRC_DEC
  };
  
  // ****************************** PUBLIC IRC_HOME *******************************

  IRC_Home::IRC_Home(HomeReference *p, GCalgorithm *g):HomeGCalgorithm(p,g,RC_ALG_IRC), counter(0){};


  IRC_Home::~IRC_Home(){}


  void IRC_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite *dest){
    Assert(counter >= 0);
    counter++;
    gf_Marshal8bitInt(bs, true);
  }


  bool IRC_Home::m_isRoot(){
    Assert(counter >= 0);
    return (counter != 0);
  }


  void IRC_Home::m_getCtlMsg(DSite* msite, MsgContainer* msg){
    Assert(counter > 0);
    IRCmessageType mtype = static_cast<IRCmessageType>(msg->popIntVal());
    switch(mtype){
    case GC_IRC_DROP: {
      int number = msg->popIntVal();
      counter -= number;
      break;
    }
    case GC_IRC_DEC:{
      counter--;
      break;
    }
    default:
      dssError("IRC_Home: unknown message %d",mtype);
      break;
    }
  }


  // ***************************** PUBLIC IRC_REMOTE ******************************
 
  IRC_Remote::IRC_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g):
    RemoteGCalgorithm(p,g,RC_ALG_IRC), sender(m_getEnvironment()->m_getSrcDSite()), decs(1), counter(0){
    gf_Unmarshal8bitInt(bs);
  }
  

  IRC_Remote::~IRC_Remote(){}
  
  
  void IRC_Remote::m_getReferenceInfo(DssWriteBuffer *bs , DSite* toSite){
    Assert(decs > 0);
    bool use = (toSite != sender); // If we send to our sender he don't have to do anything
    if (use) counter++; // else inc our counter, we're sending to someone new
    gf_Marshal8bitInt(bs, use);  
  }


  void IRC_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){
    Assert(counter >= 0); Assert(decs > 0);
    DSite* ssite = m_getEnvironment()->m_getSrcDSite();
    int use = gf_Unmarshal8bitInt(bs);
    if(ssite == sender)
      decs++; // We know that the sender can't have gotten it from us so assume use
    else if (use){
      // We have received from someone "new" (= we are not his sender) to us, tell him to drop one
      MsgContainer *msgC = m_createRemoteMsg();
      msgC->pushIntVal(GC_IRC_DEC);
      m_sendToRemote(ssite, msgC);
    }
  }
  
  
  bool IRC_Remote::m_isRoot(){
    Assert(counter >= 0); Assert(decs > 0);
    return (counter > 0);
  }


  void IRC_Remote::m_dropReference(){
    Assert(counter >= 0); Assert(decs > 0);
    MsgContainer *msgC = m_createRemoteMsg();
    msgC->pushIntVal(GC_IRC_DROP);
    msgC->pushIntVal(decs);
    m_sendToRemote(sender, msgC);
    decs = 0;
  }


  void IRC_Remote::m_getCtlMsg(DSite* msite, MsgContainer* msg){
    IRCmessageType mtype = static_cast<IRCmessageType>(msg->popIntVal());
    switch(mtype){
    case GC_IRC_DROP:{
      int number = msg->popIntVal();
      counter -= number;
      break;
    }
    case GC_IRC_DEC:{
      counter--;
      break;
    }
    default:
      dssError("IRC_Remote: unknown message %d",mtype);
      break;
    }
  }


  void IRC_Remote::m_makeGCpreps(){ sender->m_makeGCpreps(); }


  
  
}
