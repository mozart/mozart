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
#include "msl_serialize.hh"

namespace _dss_internal{

  
  const int FracHandler::MAXENUMERATOR = (INT_MAX >> 4);
  
  inline int 
  FracHandler::GiveSize(const int& enumerator){
    Assert(wrc_alpha > 0);
    if (enumerator < wrc_alpha) return 1;
    return (enumerator / wrc_alpha);
  }
  
  FracHandler::FracHandler(const int& alpha) : frac(NULL), wrc_alpha(alpha)
  {}

  FracHandler::FracHandler(const int& e, const int& d, const int& alpha) :
    frac(new TwoTypeContainer<int,int>(e,d,NULL)), wrc_alpha(alpha)
  {}

  inline void FracHandler::Frac_init(const int& e, const int& d) {
    frac = new TwoTypeContainer<int,int>(e,d,NULL);
  }

  FracHandler::~FracHandler() {
    t_deleteList(frac);
  }

  // Invariant: frac is a list of pairs (e,d), such that
  // e <= MAXENUMERATOR, and the values of d are increasing

  void
  FracHandler::insertPair(const int& e, const int& k){
    if (k == 0) return;
    
    TwoTypeContainer<int,int>** nodep = &frac;
    while ((*nodep)!=NULL && (*nodep)->a_contain2 < k) {
      nodep = &((*nodep)->a_next);
    }
    // (*nodep) == NULL || (*nodep)->a_contain2 >= k

    if ((*nodep) == NULL || (*nodep)->a_contain2 > k) {
      // insert (e,k) in front of *nodep
      *nodep = new TwoTypeContainer<int,int>(e, k, *nodep);
      return;
    }
    // (*nodep) != NULL && (*nodep)->a_contain2 == k
    if (e + (*nodep)->a_contain1 == MAXENUMERATOR) {
      // remove the pair at *nodep, and insert (1,k-1)
      TwoTypeContainer<int,int>* next = (*nodep)->a_next;
      delete *nodep;
      *nodep = next;
      insertPair(1, k-1);
      return;
    }
    if (e + (*nodep)->a_contain1 > MAXENUMERATOR) {
      // add e here, decrease MAXENUMERATOR, and insert (1,k-1)
      (*nodep)->a_contain1 += e - MAXENUMERATOR;
      insertPair(1,k-1);
      return;
    }
    // (*nodep)->a_contain1 + e < MAXENUMERATOR
    (*nodep)->a_contain1 += e;
  }
  
  void
  FracHandler::getNewRefWeightPair(int &e, int &d) {
    // frac can be NULL if all references have been collected at the
    // home site, but the entity has not been localized
    if (frac == NULL) Frac_init(MAXENUMERATOR, 1);

    if (frac->a_contain1 > 1) {
      e = GiveSize(frac->a_contain1);
      frac->a_contain1 -= e;
      d = frac->a_contain2;
    } else {
      // frac is a pair of the form (1,k)
      if (frac->a_next == NULL ||
	  frac->a_next->a_contain2 > (frac->a_contain2 + 1)) {
	// there is no pair of the form (*,k+1).  Decompose frac into
	// (e,k+1) and (MAXENUMERATOR-e,k+1), and keep the latter.
	e = GiveSize(MAXENUMERATOR);
	d = frac->a_contain2+1;
	frac->a_contain1 = (MAXENUMERATOR - e);
	frac->a_contain2 = d;
      } else {
	// "steal" from the second pair
	TwoTypeContainer<int,int>* second = frac->a_next;
	e = GiveSize(second->a_contain1); // 1 or E div alpha
	d = second->a_contain2;
	// decrease e from second, and delete it if the left part nullify
	if (second->a_contain1 > 1)
	  second->a_contain1 -= e;
	else {
	  frac->a_next = second->a_next;
	  delete second;
	}
      }
    }
  }

  void
  FracHandler::removeHead(int& e, int& d){
    e = frac->a_contain1;
    d = frac->a_contain2;
    TwoTypeContainer<int,int>* tmp=frac;
    frac = frac->a_next;
    delete tmp;
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
    MsgContainer *msgC;
    int e,d;
    while(!isEmpty()){
      msgC = m_createHomeMsg();
      removeHead(e,d);
      msgC->pushIntVal(e);
      msgC->pushIntVal(d);
      m_sendToHome(msgC);
    }
  }


  bool
  WRC_Remote::m_isRoot(){ return false; }


  void
  WRC_Remote::m_getCtlMsg(DSite*, MsgContainer*) {
    Assert(0);
    dssError("Unexpected message received WRC_Remote");
  }

  

}
