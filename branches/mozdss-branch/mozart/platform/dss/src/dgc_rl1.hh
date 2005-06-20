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
#ifndef __DGC_RL1_HH
#define __DGC_RL1_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dgc.hh"
#include "dss_rl_siteHandler.hh"
namespace _dss_internal{ // Start namespace
    //
  //  Reference Listing Version 1
  //
  //  This protocol uses a schema with an "inc and ack" operation to
  //  avoid having to involve the owner at every passing of a ref
  //
  //  Ex. when a borrower (1) passes a ref to another (2) who doesn't 
  //  have this already the new borrower (2) will tell the owner he got 
  //  it and to acknowledge the first (1). = 2 extra messages
  //
  //  if (2) already has this he might himself ack (1) to avoid involving
  //  the owner.  = 1 extra message to sender
  //
  //  This ensures that only new borrowers will tell the owner
  //

  // ******************** RLV1 HOME ***********************

  class RLV1_Home: public HomeGCalgorithm, private SiteHandler{
  public:
    RLV1_Home(HomeReference *p, GCalgorithm *g);
    virtual ~RLV1_Home();
  
    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_makeGCpreps();
  };


  // ******************* RLV1 REMOTE *********************

  class RLV1_Remote: public RemoteGCalgorithm, private SiteHandler{
  private:
    int decs;
  public:
    RLV1_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g);
    virtual ~RLV1_Remote();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_dropReference();
    void m_makeGCpreps();
  };

  
}

#endif
