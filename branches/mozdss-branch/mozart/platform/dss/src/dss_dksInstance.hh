/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

#ifndef __DSS_DKS_INSTANCE_HH
#define __DSS_DKS_INSTANCE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "DKSNode.hh"
#include "dssBase.hh"
#include "dss_netId.hh"

namespace _dss_internal{
  

  class DksInstance : public DKSNode, public NetIdNode,
		      public BucketHashNode<DksInstance>, public DSS_Environment_Base{
  private: 
    DSite* a_joins; 
  public: 
    DksInstance(int N, int K, int F, DKS_userClass* usr, DSS_Environment*); 
    DksInstance(int N, int K, int F, NetIdentity , DSite*,DSS_Environment*); 
    void m_marshal(DssWriteBuffer*); 
    int m_getMarshaledSize() const;
    virtual  MsgContainer *m_createDKSMsg();
    void m_joinDksRing(); 
    
    MACRO_NO_DEFAULT_CONSTRUCTORS(DksInstance);
  };

  class DksInstanceHT: public NetIdHT, public BucketHashTable<DksInstance> {
  public: 
    DksInstanceHT(int sz, DSS_Environment*); 

    bool m_unmarshalDksInstance(DssReadBuffer*, DksInstance*&);
    void m_redirectMessage(MsgContainer*, DSite*); 
    void m_addDksInstance(DksInstance*); 
    
    void m_gcResources();
    void m_siteStateChane(DSite* s, const DSiteState& state);
    MACRO_NO_DEFAULT_CONSTRUCTORS(DksInstanceHT); 
  };
  
}
#endif
