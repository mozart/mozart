/* 
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

#if defined(INTERFACE)
#pragma implementation "dss_netId.hh"
#endif

#include "dss_netId.hh"
#include "dss_comService.hh"
#include "dssBase.hh"

namespace _dss_internal{ //Start namespace

  void NetIdentity::marshal(DssWriteBuffer* bb) {
    site->m_marshalDSite(bb);
    gf_MarshalNumber(bb, index);
  }

  NetIdentity::NetIdentity(DssReadBuffer* bb, DSS_Environment* env) {
    site  = env->a_msgnLayer->m_UnmarshalDSite(bb);
    index = gf_UnmarshalNumber(bb);
  }

  void gf_marshalNetIdentity(DssWriteBuffer *bb, NetIdentity ni){
    ni.marshal(bb);
  }

  NetIdentity gf_unmarshalNetIdentity(DssReadBuffer *bb, DSS_Environment* env){
    return NetIdentity(bb, env);
  }

  void gf_pushNetIdentity(MsgContainer* msgC, NetIdentity ni){
    msgC->pushDSiteVal(ni.site);
    msgC->pushIntVal(ni.index);
  }

  NetIdentity  gf_popNetIdentity(MsgContainer* msgC){
    DSite *site = msgC->popDSiteVal();
    int index   = msgC->popIntVal();
    return NetIdentity(site, index);
  }
}
