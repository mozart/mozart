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
#ifndef __PROTOCOL_DKS_BROADCAST_HH
#define __PROTOCOL_DKS_BROADCAST_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_dssDks.hh"
#include "dss_templates.hh"
namespace _dss_internal{ //Start namespace

  class ProtocolDksBcManager: public ProtocolManager{
  public:
    ProtocolDksBcManager();
    virtual ~ProtocolDksBcManager(){ DebugCode(a_allocated--); }
    virtual void msgReceived(MsgContainer*,DSite*);
    virtual void makeGCpreps();
  };



  class ProtocolDksBcProxy: public ProtocolProxy, public DksBcClass {
  private:
    SimpleQueue<Pair<PstOutContainerInterface*, int> > a_unsentMsgs;
    bool a_isFunctional;
    bool a_isReferenceInstance;
    DksInstance *a_dks;

  private:
    ProtocolDksBcProxy(const ProtocolDksBcProxy&);
  public: // Exposed to the abstract entity
    virtual OpRetVal operationAppend(GlobalThread*,PstOutContainerInterface**&);
    ProtocolDksBcProxy();
  public: // From DksBcClass
    virtual void m_receivedBroadcast(DksBcMessage*);
    virtual void dks_functional();

  public: // Virtuals from Protocol Proxy
    virtual bool isWeakRoot();

    void m_initHome(DSS_Environment*);

    virtual void msgReceived(MsgContainer*,DSite*);
    // Usually if we were a weak root we cannot change the state
    virtual bool clearWeakRoot();
    virtual void makeGCpreps();

    // called when the proxy is fully installed in the system.
    // Typical usage case: registration of variable proxies.
    //
    // ZACHARIAS: explain what bool return means, thanks ;)
    virtual bool m_initRemoteProt(DssReadBuffer*);

    // Currently faults are ignored.
    // virtual FaultState siteStateChanged(DSite*, const DSiteState&);

    //located in dss_access.cc, the only placed accessed from
    virtual char *m_stringrep();

    // Marshaling and unmarshaling proxy information
    //+++++++++++++
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite *);
    virtual bool dispose_protocol_info(DssReadBuffer *buf);
    virtual int  getMarshaledSize() const;

    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
                                                  ::PstOutContainerInterface* pstOut);
    virtual void localInitatedOperationCompleted();

    // entity operations
    virtual OpRetVal operationMonitor() { return DSS_INTERNAL_ERROR_NO_OP; }
    virtual OpRetVal operationKill() { return DSS_INTERNAL_ERROR_NO_OP; }
  };


}

#endif
