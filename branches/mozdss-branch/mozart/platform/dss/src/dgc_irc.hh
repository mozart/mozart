/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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
#ifndef __DGC_IRC_HH
#define __DGC_IRC_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dgc.hh"

namespace _dss_internal{ // Start namespace

  
  //
  //  Indirect reference counting
  //

  // ******************** IRC HOME ***********************

  class IRC_Home: public HomeGCalgorithm
  {
  private:
    int counter;
  public:
    IRC_Home(HomeReference *p, GCalgorithm *g);
    virtual ~IRC_Home();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
  };


  // ******************* IRC REMOTE *********************

  class IRC_Remote: public RemoteGCalgorithm
  {
  private:
    DSite* sender;
    int decs; // to owner
    int counter; // sent to others

    IRC_Remote(const IRC_Remote&):
      RemoteGCalgorithm(NULL,NULL,RC_ALG_PERSIST),
      sender(NULL), decs(0), counter(0){};
    IRC_Remote& operator=(const IRC_Remote&){ return *this; }

  public:
    IRC_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g);
    virtual ~IRC_Remote();

    bool m_isRoot();  
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_dropReference();
    void m_makeGCpreps();
  };
}

#endif
