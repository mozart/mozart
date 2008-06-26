/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 *
 *  Contributors:
 *
 *  Copyright:
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

#ifndef __DSS_LARGEMESSAGE_HH
#define __DSS_LARGEMESSAGE_HH

#include "dss_templates.hh"
#include "dss_msgLayerInterface.hh"
#include "dss_netId.hh"

namespace _dss_internal{

  // raph: LargeMessage's have been introduced because of a (slightly
  // artificial) limitation in the size of MsgContainer's.  Recently I
  // removed this limitation, so LargeMessage's should be considered
  // deprecated.  I might possibly remove them completely from the DSS
  // soon...

  // Externs
  class LargeMessage;

  void gf_pushLargeMessage(MsgContainer* , LargeMessage* );
  LargeMessage* gf_popLargeMessage(MsgContainer*);

  class LrgMsgEle;


  // A class to be used when creating large messages. The msgcontainer
  // class is restricted in size and thus realy large messages of integers,
  // sites and extdatacontainers should be created by large messages.
  class LargeMessage{
    friend void gf_pushLargeMessage(MsgContainer*, LargeMessage*);
    friend LargeMessage* gf_popLargeMessage(MsgContainer*);
  private:
    SimpleQueue<LrgMsgEle*> a_elements;
    LargeMessage(SimpleList<LrgMsgEle*> &elements);
  public:
    void pushInt(int i);
    void pushDSiteVal(DSite* s);
    void pushNetId(NetIdentity);
    void pushDC(ExtDataContainerInterface* e);
    void pushLM(LargeMessage*);
    int popInt();
    DSite* popDSiteVal();
    ExtDataContainerInterface* popDC();
    NetIdentity popNetId();
    bool isEmpty();
    LargeMessage* popLM();
    LargeMessage();
  };


  ExtDataContainerInterface* createLrgMsgContainer(DSS_Environment*);
}


#endif
