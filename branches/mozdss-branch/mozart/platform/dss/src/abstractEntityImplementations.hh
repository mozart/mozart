/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#ifndef __ABSTRACT_ENTITY_IMPLEMENTATIONS_HH
#define __ABSTRACT_ENTITY_IMPLEMENTATIONS_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dss_enums.hh"
#include "dssBase.hh"
namespace _dss_internal{ //Start namespace

  enum AbsOp{
    AO_NO_OP = 0, 
    AO_OO_BIND,
    AO_OO_ACCESS,
    AO_OO_UPDATE,
    AO_OO_CHANGES,
    AO_STATE_WRITE,
    AO_STATE_READ,
    AO_STATE_LOCK,
    AO_STATE_UNLOCK,
    AO_STATE_EXTRACT,
    AO_STATE_INSTALL,
    AO_LZ_FETCH,
    AO_DC_SEND,
    AO_DC_EXTRACT,
    AO_DC_INSTALL,
    AO_EP_W_EXEC,
    AO_EP_W_DONE,
    AO_EP_EXTRACT,
    AO_EP_INSTALL
  };
  
  
  class AE_ProxyCallbackInterface{
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    // This is an result of interface bulshit. 
    // The Proxy needs a ref to the abstract entity, 
    // which in turn needs a ref to the proxy... 
    // Some one of the two has to be set "laziliy" and
    // not at instantiation time... Because of simplicity
    // the abstract entity lost.
    //
    // This solution has the benefit that we can alloe the 
    // AE to point to the Proxy, instead of only to the 
    // CAinterface
    Proxy *a_coordinationProxy; 

    MACRO_NO_DEFAULT_CONSTRUCTORS(AE_ProxyCallbackInterface);

  public:

    void setCoordinationProxy(Proxy *pr);
        
    virtual AOcallback applyAbstractOperation(const AbsOp&,
					      DssThreadId*,
					      DssOperationId*,
					      PstInContainerInterface* , 
					      PstOutContainerInterface*&) = 0; 
    
    virtual PstOutContainerInterface* retrieveEntityState() = 0; 
    virtual void installEntityState(::PstInContainerInterface* builder) = 0; 

    virtual void reportFaultState(const FaultState& fs)=0; 
    
    virtual AbstractEntityName m_getName() = 0;
    virtual AbstractEntity *m_getAEreference() = 0; 

    AE_ProxyCallbackInterface();
    virtual ~AE_ProxyCallbackInterface();
  };
  

  class MutableAbstractEntityImpl: public MutableAbstractEntity,
				   public AE_ProxyCallbackInterface{
  public:
    virtual OpRetVal abstractOperation_Read(DssThreadId *id,
					    ::PstOutContainerInterface**& out);
    
    virtual OpRetVal abstractOperation_Write(DssThreadId *id,
					     ::PstOutContainerInterface**& out);
    
    virtual AOcallback applyAbstractOperation(const AbsOp&,
					      DssThreadId*,
					      DssOperationId*,
					      PstInContainerInterface* , 
					      PstOutContainerInterface*&); 
    
    virtual PstOutContainerInterface* retrieveEntityState(); 
    virtual void installEntityState(::PstInContainerInterface* builder); 
    
    virtual void reportFaultState(const FaultState& fs);
    
    virtual CoordinatorAssistantInterface *getCoordinatorAssistant() const;
    
    MutableAbstractEntityImpl();

    virtual AbstractEntityName m_getName(); 
    virtual AbstractEntity *m_getAEreference();
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut); 
  
    virtual void localInitatedOperationCompleted();
  };

  class RelaxedMutableAbstractEntityImpl: public RelaxedMutableAbstractEntity,
					  public AE_ProxyCallbackInterface{
  public:
    virtual OpRetVal abstractOperation_Read(DssThreadId *id,
					    ::PstOutContainerInterface**& out);
    
    virtual OpRetVal abstractOperation_Write(::PstOutContainerInterface**& out);
    
    virtual AOcallback applyAbstractOperation(const AbsOp&,
					       DssThreadId*,
					       DssOperationId*,
					       PstInContainerInterface* , 
					       PstOutContainerInterface*&);    

    virtual PstOutContainerInterface* retrieveEntityState(); 

    virtual void installEntityState(::PstInContainerInterface* builder); 
    
    virtual void reportFaultState(const FaultState& fs);
    
    virtual CoordinatorAssistantInterface *getCoordinatorAssistant() const;
    
    RelaxedMutableAbstractEntityImpl();

    virtual AbstractEntityName m_getName(); 
    virtual AbstractEntity *m_getAEreference();
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut); 

    virtual void localInitatedOperationCompleted(){;}
  };


  
  class MonotonicAbstractEntityImpl: public MonotonicAbstractEntity,
				     public AE_ProxyCallbackInterface{
  public:
    virtual OpRetVal abstractOperation_Bind(DssThreadId *id,
					    ::PstOutContainerInterface**& out);
    
    virtual OpRetVal abstractOperation_Append(DssThreadId *id,
					      ::PstOutContainerInterface**& out);
    
    virtual AOcallback applyAbstractOperation(const AbsOp&,
					      DssThreadId*,
					      DssOperationId*,
					      PstInContainerInterface*,
					      PstOutContainerInterface*&);


    
    
    virtual PstOutContainerInterface* retrieveEntityState(); 
    virtual void installEntityState(::PstInContainerInterface* builder); 

    virtual void reportFaultState(const FaultState& fs);
    
    virtual  CoordinatorAssistantInterface *getCoordinatorAssistant() const;

    MonotonicAbstractEntityImpl();
    
    virtual AbstractEntityName m_getName();
    virtual AbstractEntity    *m_getAEreference();
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut); 
    
    virtual void localInitatedOperationCompleted(){;}
  };
  
  class ImmutableAbstractEntityImpl: public ImmutableAbstractEntity,
				     public AE_ProxyCallbackInterface{
     
  public:
    ImmutableAbstractEntityImpl();
    virtual OpRetVal abstractOperation_Read(DssThreadId *id,
					    ::PstOutContainerInterface**& pstout);
    
    virtual AOcallback applyAbstractOperation(const AbsOp&,
					      DssThreadId*,
					      DssOperationId*,
					      PstInContainerInterface* , 
					      PstOutContainerInterface*&);    

    virtual ::PstOutContainerInterface* retrieveEntityState(); 
    virtual void installEntityState(::PstInContainerInterface* builder); 


 
    virtual void reportFaultState(const FaultState& fs);
    
    virtual CoordinatorAssistantInterface *getCoordinatorAssistant() const;
    virtual AbstractEntityName m_getName(); 
    virtual AbstractEntity *m_getAEreference();
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut); 
    virtual void localInitatedOperationCompleted(){;}
  };


}
#endif
