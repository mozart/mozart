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
#pragma implementation "dgc_fwrc.hh"
#endif

#include "dgc_fwrc.hh"
#include "referenceConsistency.hh"
#include <limits.h>

namespace _dss_internal{

  
  const int FracHandler::MAXENUMERATOR = (INT_MAX >> 4);
  
  inline int 
  FracHandler::GiveSize(const int& enumerator){
    Assert(wrc_alpha > 0);
    if (enumerator < wrc_alpha) return 1;
    return (enumerator / wrc_alpha);
  }
  
  FracHandler::FracHandler(const int& alpha) : frac(), wrc_alpha(alpha)
  {}

  FracHandler::FracHandler(const int& e, const int& d, const int& alpha) :
    frac(), wrc_alpha(alpha)
  {
    Frac_init(e, d);
  }

  inline void FracHandler::Frac_init(const int& e, const int& d) {
    Assert(frac.isEmpty());
    frac.push(makePair(e, d));
  }

  FracHandler::~FracHandler() {}

  // Invariant: frac is a list of pairs (e,d), such that
  // 1 <= e <= MAXENUMERATOR, and the values of d are increasing

  void
  FracHandler::insertPair(const int& e, const int& k){
    if (k == 0) return;

    Position<Pair<int,int> > pos(frac);
    while (pos() && (*pos).second < k) pos++;
    // we are on the first position whose denum is >= k

    if (pos.isEmpty() || (*pos).second > k) {
      // insert (e,k) at this position
      pos.push(e, k);
    } else {
      // here (*pos).second == k, so we add e in this pair
      (*pos).first += e;
      if ((*pos).first >= MAXENUMERATOR) {
	// decrease MAXENUMERATOR, and insert (1,k-1)
	(*pos).first -= MAXENUMERATOR;
	if ((*pos).first == 0) pos.remove();
	insertPair(1, k-1);
      }
    }
  }
  
  void
  FracHandler::getNewRefWeightPair(int &e, int &d) {
    // frac can be empty if all references have been collected at the
    // home site, but the entity has not been localized yet
    if (frac.isEmpty()) Frac_init(MAXENUMERATOR, 1);

    Position<Pair<int,int> > pair1(frac);
    if ((*pair1).first > 1) {
      // take from *pair1
      e = GiveSize((*pair1).first);
      d = (*pair1).second;
      (*pair1).first -= e;
    } else {
      // *pair1 is of the form (1,k)
      Position<Pair<int,int> > pair2 = pair1;
      pair2++;   // move to second position
      if (pair2.isEmpty() || (*pair2).second > (*pair1).second + 1) {
	// there is no pair of the form (_,k+1).  Decompose *pair1
	// into (e,k+1) and (MAXENUMERATOR-e,k+1), and keep the latter.
	e = GiveSize(MAXENUMERATOR);
	d = (*pair1).second + 1;
	(*pair1) = makePair(MAXENUMERATOR - e, d);
      } else {
	// "steal" from *pair2
	e = GiveSize((*pair2).first); // 1 or E div alpha
	d = (*pair2).second;
	// decrease e from *pair2, and drop it if (*pair2).first nullifies
	if ((*pair2).first > 1) (*pair2).first -= e;
	else pair2.remove();
      }
    }
  }

  void
  FracHandler::removeHead(int& e, int& d) {
    Position<Pair<int,int> > pos(frac);
    e = (*pos).first;
    d = (*pos).second;
    pos.remove();
  }
  

  // ************************** PUBLIC WRC_HOME ********************************
  
  WRC_Home::WRC_Home(HomeReference* const p, GCalgorithm* const g, const int& alpha):
    HomeGCalgorithm(p,g,RC_ALG_WRC), FracHandler(MAXENUMERATOR,1,alpha){
  }
  
  
  WRC_Home::~WRC_Home(){}
  
  
  void
  WRC_Home::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    int e,d;
    getNewRefWeightPair(e,d);
    gf_MarshalNumber(bs, e);
    gf_MarshalNumber(bs, d);
  }
  
  bool
  WRC_Home::m_isRoot(){ return !isEmpty(); }
  
  
  void
  WRC_Home::m_getCtlMsg(DSite* , MsgContainer* msg) {
    int e,d;
    do{
      e = msg->popIntVal();
      d = msg->popIntVal();
      insertPair(e,d);
    }while(!msg->m_isEmpty());
  }

  // ************************* PUBLIC WRC_REMOTE ******************************


  WRC_Remote::WRC_Remote(RemoteReference* const p, DssReadBuffer *bs,
			 GCalgorithm* const g, const int& alpha):
    RemoteGCalgorithm(p,g,RC_ALG_WRC), FracHandler(alpha){
    int e = gf_UnmarshalNumber(bs);
    int d = gf_UnmarshalNumber(bs);
    Frac_init(e,d);
  }
  
  
  WRC_Remote::~WRC_Remote(){}

  void
  WRC_Remote::m_getReferenceInfo(DssWriteBuffer *bs, DSite *dest){
    int e, d;
    getNewRefWeightPair(e,d);
    gf_MarshalNumber(bs, e);
    gf_MarshalNumber(bs, d);
  }
  
  void
  WRC_Remote::m_mergeReferenceInfo(DssReadBuffer *bs){
    int e,d;
    e = gf_UnmarshalNumber(bs);
    d = gf_UnmarshalNumber(bs);
    insertPair(e,d);
  }

  void
  WRC_Remote::m_dropReference(){
    MsgContainer *msgC = m_createHomeMsg();
    int e,d;
    while (!isEmpty()) {
      removeHead(e,d);
      msgC->pushIntVal(e);
      msgC->pushIntVal(d);
    }
    m_sendToHome(msgC);
  }


  bool
  WRC_Remote::m_isRoot(){ return false; }


  void
  WRC_Remote::m_getCtlMsg(DSite*, MsgContainer*) {
    Assert(0);
    dssError("Unexpected message received WRC_Remote");
  }

  

}
