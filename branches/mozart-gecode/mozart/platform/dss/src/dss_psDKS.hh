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

#ifndef __DSS_PS_DKS_HH
#define __DSS_PS_DKS_HH

#ifdef INTERFACE
#pragma interface
#endif


#include "dss_dksInstance.hh"

namespace _dss_internal{
  
  class PS_DKS_userClass: public DKS_userClass, public DSS_Environment_Base{
    KbrInstance*          a_kbrInstance; 
    KbrCallbackInterface* a_kbrInterface; 
  public:
    virtual void m_receivedRoute(int Key, DksMessage*);
    virtual void m_receivedRouteNext(int Key, DksMessage*);
    virtual DksMessage* m_divideResp(int start, int stop, int n);   
    virtual void m_newResponsability(int begin, int end, int n, DksMessage*); 
    virtual void dks_functional(); 
    virtual void pushDksMessage(MsgContainer*, DksMessage*);
    virtual DksMessage *popDksMessage(MsgContainer*);
    
    virtual void m_receivedBroadcast(DksBcMessage*);
    virtual void pushDksBcMessage(MsgContainer*, DksBcMessage*);
    virtual DksBcMessage *popDksBcMessage(MsgContainer*);
    
    
    PS_DKS_userClass(DSS_Environment*, KbrCallbackInterface* );
    void m_setKbrInstance(KbrInstance*); 
    void m_setKbrCallBack(KbrCallbackInterface*); 
    KbrInstance* m_getKbrInstance(); 
    KbrCallbackInterface* m_getKbrCallBack();
    MACRO_NO_DEFAULT_CONSTRUCTORS(PS_DKS_userClass); 
  };


  class KbrInstanceImpl: public KbrInstance{
  private: 
    PS_DKS_userClass *a_usrClass; 
    DksInstance *a_node; 
  public:  // Inherited from KbrInstance
    virtual void m_setCallback(KbrCallbackInterface*);
    virtual KbrCallbackInterface* m_getCallback();
    virtual KbrResult m_broadcast(PstOutContainerInterface*);
    virtual KbrResult m_route(int, PstOutContainerInterface*); 
    virtual int  m_getId();
    virtual void m_join(); 
    virtual void m_leave(); 
    virtual void m_marshal(DssWriteBuffer*); 
  public: // DSS internals methods. 
    KbrInstanceImpl(DksInstance*, PS_DKS_userClass*); 
  public: 
    MACRO_NO_DEFAULT_CONSTRUCTORS(KbrInstanceImpl); 
  };
  
}
#endif
