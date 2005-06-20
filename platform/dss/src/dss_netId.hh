/*
 *  Authors:
 *    Erik Klintskog, 2003
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
class DSite;

namespace _dss_internal{ 


  // *********** Forwards to avoid inclusion of so so many files
  
  class NetIdHT;  


  class NetIdentity
  {
  public: 
    DSite *a_site; 
    u32    a_indx; 
    NetIdentity(DSite *s, u32 i):a_site(s), a_indx(i){;}
    NetIdentity():a_site(NULL), a_indx(0){;}
    bool operator<(const NetIdentity ni){
      return (a_indx !=  ni.a_indx)?(a_indx < ni.a_indx ):(*(a_site) < *(ni.a_site));
    }
    
    NetIdentity(const NetIdentity& n):a_site(n.a_site), a_indx(n.a_indx){;}
    NetIdentity& operator=(const NetIdentity& n){ a_site =n.a_site; a_indx = n.a_indx;  return *this;}
  };

  // The DSS makes heavy use of globaly unique netidentitied, called NetIds
  // A netwidentity is guaranteed to be uniqiue since it consiste out of the 
  // process the identity was creeated at and a number unique to that process. 
  // 
  // NetIds are sent over the network mostly when identfying entities(can be any thing 
  // form an abstract entity to a DKS instance). Thus finding the entity efficiently 
  // is important. NetIds are thus stored in hashstables for quick finding.

  
  class NetIdNode:protected BucketHashNode{
    friend class NetIdHT;  
  public: 
    inline ::DSite   *m_getGUIdSite() { return reinterpret_cast<DSite *>(getSecKey());}

    NetIdNode(DSite* const s, const unsigned int& mi):
      BucketHashNode(mi,reinterpret_cast<u32>(s))
    {;}

    NetIdNode(NetIdentity ni):BucketHashNode(ni.a_indx ,reinterpret_cast<u32>(ni.a_site))
    {;}
    
    NetIdNode():BucketHashNode(0,0){;}
    
    NetIdentity m_getNetId(){ 
      DSite *s = reinterpret_cast<DSite *>(getSecKey());
      u32    i = static_cast<unsigned int>(getPrimKey());
      return NetIdentity(s, i); 
    }
  private: 
    
  private: // Unsued functionality 
    NetIdNode(const NetIdNode&):BucketHashNode(0,0){;}
    NetIdNode& operator=(const NetIdNode&){ return *this;}
  };
  

  // The Hashtable class that holds all the NedId nodes. 
  
  class NetIdHT: private BucketHashTable, public DSS_Environment_Base{
     u32 a_nxtId;
   public:
    NetIdHT(int sz, DSS_Environment* env):BucketHashTable(sz),DSS_Environment_Base(env), a_nxtId(0){;}

    // The call returns the NedIdNode with the NetIdentity ni. 
    // If none is found, the call returns NULL. 
    inline NetIdNode* m_findNetId(NetIdentity ni){
      return static_cast<NetIdNode*>(htFindPkSk(ni.a_indx,reinterpret_cast<u32>(ni.a_site)));
    }
    
    inline NetIdentity m_createNetIdentity(){
      return NetIdentity( m_getEnvironment()->a_myDSite, a_nxtId ++); 
    }
      
    inline void m_removeNetId(NetIdNode* n){
      htSubPkSk(n->getPrimKey(), n->getSecKey());
    }
    
    // Used when a net id node allready has an identity
    inline void m_insertNetId(NetIdNode* n){
      htAdd(n->getPrimKey(), n); 
    }
    
    inline void m_addNetId(NetIdNode* n){
      NetIdentity ni = m_createNetIdentity(); 
      u32 pk = ni.a_indx; 
      u32 sk = reinterpret_cast<u32>(ni.a_site); 
      n->setKeys(pk, sk); 
      htAdd(pk, n); 
    }
    
    NetIdNode* m_getNext(NetIdNode* n){
      return static_cast<NetIdNode*>(htGetNext(n)); 
    }
  };

  void gf_marshalNetIdentity(DssWriteBuffer *, NetIdentity ni); 
  NetIdentity gf_unmarshalNetIdentity(DssReadBuffer *bb, DSS_Environment*); 

  void gf_pushNetIdentity(MsgContainer*, NetIdentity ni);
  NetIdentity  gf_popNetIdentity(MsgContainer*);
}
#endif
