/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog,     2004
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog,     2004
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

#ifndef __DSS_COORDINATOR_STATIONARY_HH
#define __DSS_COORDINATOR_STATIONARY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "coordinator.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class CoordinatorStationary: public Coordinator {
    friend class ProxyStationary;
  protected:
    void m_initProxy(Proxy *p);
  public:
    // ************** CONSTRUCTORS *******************
    CoordinatorStationary(ProtocolManager* const prot,
                          const RCalg& gc_annot, DSS_Environment* const env);
    virtual ~CoordinatorStationary();

    // ******************* MESSAGES ***********************
    bool  m_sendToProxy(DSite* dest, ::MsgContainer* msg);
    virtual ::MsgContainer *m_createProxyProtMsg();
    virtual ::MsgContainer *m_createProxyRefMsg();

    // ************** PROXY + AS ********************
    DSS_GC m_getDssDGCStatus();

    void m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    void m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite){ a_homeRef->m_msgToGcAlg(msgC,fromsite);}
    void m_receiveAsMsg(  MsgContainer *, DSite*){ Assert(0); }

    void m_makeGCpreps();

    virtual void m_siteStateChange(DSite *, const FaultState&);

    char *m_stringrep();
  };


  class ProxyStationary: public Proxy {
  public:
    // ************** CONSTRUCTORS *******************
    ProxyStationary(NetIdentity ni, ProtocolProxy* const prot,
                    DSS_Environment* const env);
    ~ProxyStationary();

    virtual AccessArchitecture getAccessArchitecture() const {
      return AA_STATIONARY_MANAGER; }

    virtual void m_initHomeProxy(Coordinator *m);
    virtual bool m_initRemoteProxy(DssReadBuffer *bs);

    // **************** REFERENCE *******************

    virtual void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    virtual int  m_getReferenceSize(DSite* dest);
    virtual void m_mergeReferenceInfo(DssReadBuffer *bs);
    virtual void m_makePersistent();

    // **************** COMMUNICATION **************

    virtual bool  m_sendToCoordinator(::MsgContainer* msg);
    virtual DSite* m_getCoordinatorSite();

    // ************** PROXY + AS ********************

    void  m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite);
    void  m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite);
    void  m_receiveAsMsg(  MsgContainer *msgC, DSite* fromsite);

    // ***************** REFERENCE *******************

    void  m_makeGCpreps();
    virtual DSS_GC getDssDGCStatus();

    // ***************** Failures *******************************
    virtual void m_siteStateChange(DSite *, const FaultState&);
    virtual void m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg);
    // *************** MISC *************************

    char* m_stringrep();

    // *************** INTERACTION *************************************
    virtual bool manipulateCNET(void* opaque);
  private:
    Reference* m_getReferenceStructure();
  };





}//End Namespace

#endif
