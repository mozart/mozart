/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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

#ifndef __PROTOCOL_PILGRIM_HH
#define __PROTOCOL_PILGRIM_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  // for the coloring algorithm
  class PilgrimColor {
  private:
    static const int last = 1<<29;
    int value : 30;
    bool dark : 1;
  public:
    PilgrimColor() : value(0), dark(0) {}   // white, no real color
    PilgrimColor(const int &c) : value(c >> 1), dark(c & 1) {}
    void whiten() { value = dark = 0; }
    bool isWhite() const { return value == 0; }
    void darken() { dark = 1; }
    bool isDark() const { return dark; }
    operator int () const { return (value << 1) | dark; }
    void operator++ (int) { value = (value % last) + 1; dark = 0; }
    bool operator== (PilgrimColor const &c) { return value == c.value; }
    bool operator< (PilgrimColor const &c) { // beware: non-transitive!
      return (c.value - value + last) % last < (last / 2); }
  };



  class ProtocolPilgrimManager:public ProtocolManager {
  private:
    SimpleRing<DSite*> a_ring;      // the proxies in the ring
    bool               a_lastLeaving; // last proxy in ring wants to leave
    PilgrimColor       a_color;     // the current token color

    // Invariants:
    //  - a_ring is empty iff the token is lost;
    //  - all proxies in the ring are in a_proxies

    void m_lostToken();
    void m_removeFailed(DSite* s);

    ProtocolPilgrimManager(const ProtocolPilgrimManager&):
      a_lastLeaving(false) {}
    ProtocolPilgrimManager& operator=(const ProtocolPilgrimManager&){
      return *this; }

  public:
    ProtocolPilgrimManager(DSite* mysite);
    ProtocolPilgrimManager(::MsgContainer*);
    ~ProtocolPilgrimManager() {}
    void msgReceived(::MsgContainer*,DSite*);
    void sendMigrateInfo(MsgContainer*); 

    // check for failed proxies
    void m_siteStateChange(DSite*, const FaultState&);
  };


  class ProtocolPilgrimProxy:public ProtocolProxy{
  private:
    // the proxy status contains the following boolean flags:
    //  - hasToken: if the token is on this proxy;
    //  - isInRing: whether the proxy is inside the ring;
    //  - isReachable: this proxy might still be reachable from the ring;
    //  - isColoring: whether proxy must forward color;
    //  - isStrict: whether proxy rejects different colors.
    bool hasToken(void) const { return getStatus() & 1; }
    bool isInRing(void) const { return getStatus() & 2; }
    bool isReachable(void) const { return getStatus() & 4; }
    bool isColoring(void) const { return getStatus() & 8; }
    bool isStrict(void) const { return getStatus() & 16; }
    void hasToken(bool b) { setStatus((getStatus() & ~1) || (b ? 1 : 0)); }
    void isInRing(bool b) { setStatus((getStatus() & ~2) || (b ? 2 : 0)); }
    void isReachable(bool b) { setStatus((getStatus() & ~4) || (b ? 4 : 0)); }
    void isColoring(bool b) { setStatus((getStatus() & ~8) || (b ? 8 : 0)); }
    void isStrict(bool b) { setStatus((getStatus() & ~16) || (b ? 16 : 0)); }

    DSite*        a_successor;    // successor in the ring
    int           a_freeRounds;   // how many "free rounds" before leaving
    PilgrimColor  a_color;        // the current color
    int           a_jobsLeft;     // resumed operations not terminated yet

    ProtocolPilgrimProxy(const ProtocolPilgrimProxy&):
      ProtocolProxy(PN_NO_PROTOCOL) {}
    ProtocolPilgrimProxy& operator=(const ProtocolPilgrimProxy&){
      return *this; }

  public:
    ProtocolPilgrimProxy(DSite*);
    ~ProtocolPilgrimProxy(); 
    bool m_initRemoteProt(DssReadBuffer*);

    void makeGCpreps(); 
    bool isWeakRoot() { return isReachable(); }
    bool clearWeakRoot();    
    
    void m_enter();
    void m_leave();
    bool m_isAlone();
    void m_forwardToken();
    bool m_acceptTokenColor(PilgrimColor const &col);
    void m_forwardColor();
    void m_resumeOperations();
    void m_lostToken();
    
    OpRetVal protocol_Access(GlobalThread* const,
			     ::PstOutContainerInterface**&);

    void msgReceived(::MsgContainer*,DSite*);
    void remoteInitatedOperationCompleted(DssOperationId* opId,
					  ::PstOutContainerInterface* pstOut) {
      Assert(0); }
    void localInitatedOperationCompleted(); 

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif 
