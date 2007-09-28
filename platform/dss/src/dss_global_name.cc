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
#if defined(INTERFACE)
#pragma implementation "dss_global_name.hh"
#endif

#include "msl_serialize.hh"
#include "dss_global_name.hh"

namespace _dss_internal{ //Start namespace
  
  void GlobalName::marshal(DssWriteBuffer* bb){
    gf_marshalNetIdentity(bb, m_getNetId());
  }

  GlobalName::~GlobalName() {
    table->remove(this);     // remove itself from table
  }

  GlobalName* GlobalNameTable::m_unmarshal(DssReadBuffer* bb) {
    NetIdentity ni = gf_unmarshalNetIdentity(bb, m_getEnvironment());
    GlobalName* gn = lookup(ni.hashCode(), ni);
    if (gn == NULL) {
      gn = new GlobalName(this, ni, NULL);
      insert(gn);
    }
    return gn;
  }

  GlobalName* GlobalNameTable::m_create(void* ref) {
    NetIdentity ni = m_createNetIdentity();
    GlobalName* gn = new GlobalName(this, ni, ref);
    insert(gn);
    return gn;
  }

  void GlobalNameTable::m_gcResources() {
    for (GlobalName* gn = getFirst(); gn; gn = getNext(gn)) {
      gn->m_getGUIdSite()->m_makeGCpreps();
    }
  }

} //end namespace
