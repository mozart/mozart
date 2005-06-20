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
#ifndef __DGC_RC_HH
#define __DGC_RC_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dgc.hh"

namespace _dss_internal{ // Start namespace
  
  //
  //  Ordinary Reference Counting
  //
  //  - should never be used!
  //

  // ******************** RC HOME ***********************

  class RC_Home: public HomeGCalgorithm
  {
  private:
    int counter;
  public:
    RC_Home(HomeReference *p, GCalgorithm *g);
    virtual ~RC_Home();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
  };


  // ******************* RC REMOTE *********************

  class RC_Remote: public RemoteGCalgorithm
  {
  private:
    int unacked;
    int decs;
  public:
    RC_Remote(RemoteReference *p, DssReadBuffer *bs, GCalgorithm *g);
    virtual ~RC_Remote();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
    void m_dropReference();
  };


}

#endif
