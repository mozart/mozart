/*
 *  Authors:
 *    Erik Klintskog
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

#ifndef __DSS_DSS_DKS_HH
#define __DSS_DSS_DKS_HH

//#ifdef INTERFACE
//#pragma interface
//#endif


#include "dss_dksInstance.hh"
namespace _dss_internal{
  
  
  class PstDataContainer; 
  // The class is used to transport an abstract operation and 
  // a PstContainer. Primary used for broadcasting with 
  class DssDksBcMessage: public DksBcMessage{
    
    PstDataContainer *a_cnt; 
    int aop; 
  private: 
    DssDksBcMessage(const DssDksBcMessage&);
  public:
    DssDksBcMessage(PstDataContainer* p, int aop); 
    ~DssDksBcMessage(); 
    int m_getAop(); 
    PstDataContainer* m_getData(); 
  };

  

  // The DksBcClass is intended to be used for broadcasts only. All
  // the details regarding routing and interval spliting isregarded as 
  // noops. The class implements dummies for the reoutines regarding 
  // routing functionality, but does not handle the methods for
  // broadcast receive. 
  //
  // The class is an interafce that should be inherited from. 
  
  class DksBcClass: public DKS_userClass{
  public:  // not used methods(when broadcasting these makes no difference)
    virtual void m_receivedRoute(int Key, DksMessage*);
    virtual void m_receivedRouteNext(int Key, DksMessage*);
    virtual DksMessage* m_divideResp(int start, int stop, int n);   
    virtual void m_newResponsability(int begin, int end, int n, DksMessage*); 
    virtual void pushDksMessage(MsgContainer*, DksMessage*);
    virtual DksMessage *popDksMessage(MsgContainer*);
  public: 
    // internal methods used when the DKSNode works with 
    // broadcast messages. 
    virtual void pushDksBcMessage(MsgContainer*, DksBcMessage*);
    virtual DksBcMessage *popDksBcMessage(MsgContainer*);
    virtual void m_receivedBroadcast(DksBcMessage*);
    DksBcClass();  
  };
  
}
#endif

