/*
 *  Authors:
 *    Erik Klintskog, 2003
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik KLintskog, 2002
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

#ifndef __DSS_NETID_HH
#define __DSS_NETID_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "bucketHashTable.hh"
#include "dssBase.hh"
#include "dss_comService.hh"

namespace _dss_internal{ 

  // The DSS makes heavy use of globaly unique net identitiers, called
  // NetIds.  A net identity is guaranteed to be unique since it
  // consists of the process the identity was creeated at, and a
  // number unique to that process.
  // 
  // NetIds are sent over the network, mostly for identifying entities
  // (can be any thing form an abstract entity to a DKS instance).
  // Thus finding the entity efficiently is important.  Components
  // with NetIds are thus stored in hash tables for quick finding.

  class NetIdentity {
  public:
    DSite* site;
    u32    index;

    NetIdentity() : site(NULL), index(0) {}
    NetIdentity(DSite *s, u32 i) : site(s), index(i) {}
    NetIdentity(const NetIdentity& n) : site(n.site), index(n.index) {}

    // good hashcodes
    unsigned int hashCode() const { return site->m_getShortId() ^ index; }
    bool hashMatch(DSite* const &s) const { return site == s; }

    bool operator< (const NetIdentity &ni) {
      return index < ni.index || (index < ni.index && *site < *(ni.site));
    }
    NetIdentity& operator=(const NetIdentity& n) {
      site = n.site; index = n.index; return *this;
    }
    bool operator== (const NetIdentity &n) {
      return index == n.index && site == n.site;
    }

    // marshaling; unmarshaling is done by constructor
    int getMarshaledSize() const {
      return site->m_getMarshaledSize() + sz_MNumberMax;
    }
    void marshal(DssWriteBuffer*);
    NetIdentity(DssReadBuffer*, DSS_Environment*);
  };


  // base class of components with a NetIdentity
  class NetIdNode {
    friend class NetIdHT;
  private:
    NetIdentity netid;     // key in hash table

  public: 
    NetIdNode(DSite* const s, const unsigned int& id) : netid(s, id) {}
    NetIdNode(NetIdentity ni) : netid(ni) {}
    NetIdNode() : netid() {}

    unsigned int hashCode() const { return netid.hashCode(); }
    bool hashMatch(const NetIdentity &n) { return netid == n; }
    
    DSite *m_getGUIdSite() { return netid.site; }
    NetIdentity m_getNetId() { return netid; }

  private:
    NetIdNode(const NetIdNode&);
    NetIdNode& operator=(const NetIdNode&){ return *this;}
  };


  // base class of hash tables that hold NetIdNodes
  class NetIdHT : public DSS_Environment_Base {
  private:
    u32 a_nextId;

  public:
    NetIdHT(DSS_Environment* env) : DSS_Environment_Base(env), a_nextId(0) {}

    // create a new NetIdentity (unique for a given category of NetIdNodes)
    NetIdentity m_createNetIdentity() {
      return NetIdentity(m_getEnvironment()->a_myDSite, a_nextId++);
    }
    void m_addNetIdentity(NetIdNode* const &n) {
      n->netid = m_createNetIdentity();
    }
  };

  void gf_marshalNetIdentity(DssWriteBuffer *, NetIdentity ni); 
  NetIdentity gf_unmarshalNetIdentity(DssReadBuffer *bb, DSS_Environment*); 

  void gf_pushNetIdentity(MsgContainer*, NetIdentity ni);
  NetIdentity  gf_popNetIdentity(MsgContainer*);
}

#endif
