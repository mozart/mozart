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
#ifndef __DGC_RL2_HH
#define __DGC_RL2_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dgc.hh"
#include "dgc_rl_siteHandler.hh"
namespace _dss_internal{ // Start namespace

  //
  //  Reference Listing Version 2
  //
  //  This protocol uses a schema with a straightforward "always tell the owner"
  //
  //  Ex. when a borrower (1) passes a ref to another (2) he will always tell
  //  the owner. = 1 extra message
  //
  //  This is better if most of the time, references are passed to new borrowers
  //  but if not it will slow down the owner having to deal with "oldtimers"
  //

  // ******************** RLV2 HOME ***********************

  class RLV2_Home: public HomeGCalgorithm, public SiteHandler
  {
  public:
    RLV2_Home(HomeReference *p, GCalgorithm *g);
    virtual ~RLV2_Home();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return 0; }
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_makeGCpreps();
  };


  // ******************* RLV2 REMOTE *********************

  class RLV2_Remote: public RemoteGCalgorithm
  {
  private:
    int decs;
  public:
    RLV2_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g);
    virtual ~RLV2_Remote();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return 0; }
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_dropReference();
  };


}

#endif
