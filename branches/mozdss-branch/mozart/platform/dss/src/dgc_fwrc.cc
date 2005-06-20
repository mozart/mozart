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
  
  
  void
  FracHandler::insertPair(const int& e, const int& k){
    if (k == 0) return;
    
    TwoTypeContainer<int,int>** tmp = &frac;
    while((*tmp)!=NULL && (*tmp)->a_contain2 < k){
      tmp = &((*tmp)->a_next);
    }
    
    if ((*tmp) == NULL || (*tmp)->a_contain2 > k){
      *tmp = new TwoTypeContainer<int,int>(e,k,*tmp);
      return;
    }
    if (e + (*tmp)->a_contain1 == MAXENUMERATOR) {
      TwoTypeContainer<int,int>* ttmp = (*tmp)->a_next;
      delete (*tmp);
      (*tmp) = ttmp;
      insertPair(1,k-1);
      return;
    }
    if (e + (*tmp)->a_contain1 > MAXENUMERATOR) {
      (*tmp)->a_contain1 = e + (*tmp)->a_contain1 - MAXENUMERATOR;
      insertPair(1,k-1);
      return;
    }
    
    (*tmp)->a_contain1 = e + (*tmp)->a_contain1;
  }
  
  void
  FracHandler::getNewRefWeightPair(int &e, int &d){
    Assert(frac != NULL);
    if (frac->a_contain1 > 1) {
      e = GiveSize(frac->a_contain1);
      frac->a_contain1 -= e;
      d = frac->a_contain2;
    } else {
      if (frac->a_next == NULL || frac->a_next->a_contain2 > (frac->a_contain2 + 1)) {
	e = GiveSize(MAXENUMERATOR);
	d = frac->a_contain2+1;
	frac->a_contain2 = d;
	frac->a_contain1 = (MAXENUMERATOR); // - e;
	frac->a_contain1 -= e; // don't put together this and the
	//above line (did that anyway but now with ()'s)
      } else {
	// "steal" from the one after
	TwoTypeContainer<int,int>* temp= frac->a_next;
	d = temp->a_contain2;
	e = GiveSize(temp->a_contain1); // 1 or E div alpha
	if (temp->a_contain1 > 1)
	  temp->a_contain1 -= e;
	else {
	  frac->a_next = temp->a_next;
	  delete temp;
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
  
  FracHandler::FracHandler(const int& e, const int& d, const int& alpha):
    frac(new TwoTypeContainer<int,int>(e,d,NULL)), wrc_alpha(alpha){
  }

  FracHandler::FracHandler(const int& alpha):frac(NULL),wrc_alpha(alpha){}
  inline void FracHandler::Frac_init(const int& e, const int& d){
    frac = new TwoTypeContainer<int,int>(e,d,NULL); 
  }


  FracHandler::~FracHandler(){
    t_deleteList(frac);
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
    getNewRefWeightPair(e,d);;
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
