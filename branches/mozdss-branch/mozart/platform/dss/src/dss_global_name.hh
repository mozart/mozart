/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
namespace _dss_internal{ //Start namespace

  class GlobalName: public GlobalNameInterface, protected BucketHashNode {
    friend class GlobalNameTable;
  private:
    unsigned int a_pk; // Primary key
    unsigned int a_sk; // Secondary key
  public:
    GlobalName(const unsigned int& pk, const unsigned int& sk, void* ref): 
      //      NamedImmutableInterface(pk, sk, ref),
      GlobalNameInterface(ref), BucketHashNode(pk, sk), a_pk(pk), a_sk(sk) {;}
    
    ~GlobalName();
    virtual void marshal(DssWriteBuffer* bb);
    
  };

  class GlobalNameTable: protected BucketHashTable, private DSS_Environment_Base{    
  private:
    unsigned int a_index;
  public:
    GlobalNameTable(const int& sz, DSS_Environment* const env):
      BucketHashTable(sz), DSS_Environment_Base(env),a_index(0) {}

    ~GlobalNameTable();
    
    inline unsigned int getNextId() {return a_index++;}
    
    inline void m_add(GlobalName* const ni){
      htAdd(ni->getPrimKey(), ni);
    }
    
    inline GlobalName* m_find(GlobalName* const ni){
      return static_cast<GlobalName *>(htFindPkSk(ni->getPrimKey(),
						  ni->getSecKey()));
    }
    inline GlobalName* m_find(const unsigned int& id, const unsigned int& id2){
      return static_cast<GlobalName *>(htFindPkSk(id, id2));
    }

    inline void m_del(GlobalName* const nim)
    { htSubEn(nim); }
    
  };
} 
#endif
