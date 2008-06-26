/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
#ifndef __DGC_FWRC_HH
#define __DGC_FWRC_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dgc.hh"

namespace _dss_internal{ // Start namespace

  // Fractional Weighted Reference Counting

  class FracHandler{
    //  Handling the list of enum denum pairs
    //
    SimpleList<Pair<int,int> > frac;
    int wrc_alpha;

    int GiveSize(const int& enumerator);

    FracHandler(const FracHandler&):frac(),wrc_alpha(0){}
    FracHandler& operator=(const FracHandler&){ return *this; }

  protected:
    static const int MAXENUMERATOR;

    FracHandler(const int& alpha);
    FracHandler(const int& e, const int& d, const int& alpha);
    virtual ~FracHandler();

    void Frac_init(const int& e, const int& d);

    inline bool isEmpty(){ return frac.isEmpty(); }
    void insertPair(const int& e, const int& k);
    void removeHead(int& e, int& d);

    void getNewRefWeightPair(int &e, int &d);

  public:
    inline bool setAlpha(const int& val){ return (val > 0) ? (wrc_alpha = val)!=0 : false; }
    inline int  getAlpha(){ return wrc_alpha; }
  };


  // ******************** WRC HOME ***********************

  class WRC_Home: public HomeGCalgorithm, public FracHandler{
  public:
    WRC_Home(HomeReference* const p, GCalgorithm* const g, const int& alpha);
    virtual ~WRC_Home();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return 2 * sz_MNumberMax; }
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
  };


  // ******************** WRC REMOTE *********************

  class WRC_Remote: public RemoteGCalgorithm, public FracHandler{
  public:
    WRC_Remote(RemoteReference* const p, DssReadBuffer *bs,
               GCalgorithm* const g, const int& alpha);
    virtual ~WRC_Remote();

    void m_dropReference();
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return 2 * sz_MNumberMax; }
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);
  };


}

#endif
