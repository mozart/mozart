/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Sahlin, 2004
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
#ifndef __DSS_GLOBAL_NAME_HH
#define __DSS_GLOBAL_NAME_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dss_classes.hh"
#include "bucketHashTable.hh"
#include "dssBase.hh"
#include "dss_netId.hh"

namespace _dss_internal{ //Start namespace

  class GlobalNameTable;

  class GlobalName: public GlobalNameInterface, public NetIdNode,
		    public BucketHashNode<GlobalName> {
    friend class GlobalNameTable;
  private:
    GlobalNameTable* table;
  public:
    GlobalName(GlobalNameTable* t, NetIdentity const &ni, void* ref) :
      GlobalNameInterface(ref), NetIdNode(ni), BucketHashNode<GlobalName>(), table(t) {}
    ~GlobalName();

    virtual void marshal(DssWriteBuffer* bb);
  };


  class GlobalNameTable: public NetIdHT, public BucketHashTable<GlobalName> {
  public:
    GlobalNameTable(const int& sz, DSS_Environment* const env):
      NetIdHT(env), BucketHashTable<GlobalName>(sz) {}
    ~GlobalNameTable() {}
    
    GlobalName* m_find(NetIdentity const &ni) {
      return lookup(ni.hashCode(), ni);
    }
    void m_insert(GlobalName* const &gn) { insert(gn); }
    void m_remove(GlobalName* const &gn) { remove(gn); }

    GlobalName* m_unmarshal(DssReadBuffer* bb);
    GlobalName* m_create(void*);

    void m_gcResources();
  };
} 

#endif
