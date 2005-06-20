/* 
 *  Authors:
 *    Erik Klintskog
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

#if defined(INTERFACE)
#pragma implementation "dss_netId.hh"
#endif

#include "dss_netId.hh"
#include "dss_comService.hh"
#include "msl_serialize.hh"
#include "dssBase.hh"
namespace _dss_internal{ //Start namespace
  
  void gf_marshalNetIdentity(DssWriteBuffer *bb, NetIdentity ni){
    ni.a_site->m_marshalDSite(bb);
    gf_MarshalNumber(bb, ni.a_indx);
  }
  
  NetIdentity gf_unmarshalNetIdentity(DssReadBuffer *bb, DSS_Environment* env){
    DSite *sd = env->a_msgnLayer->m_UnmarshalDSite(bb);
    int si    = gf_UnmarshalNumber(bb);
    return NetIdentity(sd, si); 
  }

  void gf_pushNetIdentity(MsgContainer* msgC, NetIdentity ni){
    msgC->pushDSiteVal(ni.a_site); 
    msgC->pushIntVal(ni.a_indx); 
  }
  NetIdentity  gf_popNetIdentity(MsgContainer* msgC){
    DSite *site = msgC->popDSiteVal();
    int index   = msgC->popIntVal();
    return NetIdentity(site, index); 
  }
  
}
