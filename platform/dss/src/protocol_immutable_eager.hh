/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Sahlin, 2003
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
#ifndef __PROTOCOL_IMMUTABLE_EAGER_HH
#define __PROTOCOL_IMMUTABLE_EAGER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  // The eager and lazy immutable protocols are almost identical.
  // Their managers are identical, and their proxies only differ in
  // the moment they request the state.  I have therefore factored out
  // the common stuff here.

  class ProtocolImmutableManager : public ProtocolManager {
  private:
    bool failed:1;                         // whether entity is permfail
  public:
    ProtocolImmutableManager();
    ~ProtocolImmutableManager() {}
    virtual void msgReceived(MsgContainer*,DSite*);
  };

  class ProtocolImmutableProxy : public ProtocolProxy {
  protected:
    bool failed:1;                         // whether entity is permfail
    bool stateHolder:1;                    // whether proxy has entity state
    SimpleList<GlobalThread*> a_readers;   // suspended read operations

    void m_requestState();

  public: 
    ProtocolImmutableProxy(const ProtocolName&);
    ~ProtocolImmutableProxy() { Assert(a_readers.isEmpty()); }

    OpRetVal protocol_Kill(GlobalThread* const th_id);

    virtual void msgReceived(MsgContainer*,DSite*);   
    virtual void remoteInitatedOperationCompleted(DssOperationId*,
						  PstOutContainerInterface*) {}
    virtual void localInitatedOperationCompleted() {}

    virtual void makeGCpreps();
    virtual bool isWeakRoot() { return false; }

    virtual FaultState siteStateChanged(DSite*, const DSiteState&);
  };



  // Now comes the specific stuff for the eager protocol:

  class ProtocolImmutableEagerManager : public ProtocolImmutableManager {
  public:
    ProtocolImmutableEagerManager() : ProtocolImmutableManager() {}
  };

  class ProtocolImmutableEagerProxy : public ProtocolImmutableProxy {
  public: 
    ProtocolImmutableEagerProxy() :
      ProtocolImmutableProxy(PN_IMMUTABLE_EAGER) {}

    virtual bool m_initRemoteProt(DssReadBuffer*); 

    OpRetVal protocol_Access(GlobalThread* const th_id);
  };

} //End namespace
#endif
