/*
 *  Authors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Raphael Collet, 2008
 * 
 *  Last change:
 *    $Date: $ by $Author: $
 *    $Revision: $
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
#pragma implementation "protocol_sited.hh"
#endif

#include "protocol_sited.hh"

namespace _dss_internal{ //Start namespace

  // The implementation is easy.  Basically, there is no protocol...



  /******************** ProtocolSitedProxy ********************/

  OpRetVal
  ProtocolSitedProxy::operationKill() {
    return (isHomeProxy() ? (makePermFail(), DSS_SKIP) :
	    DSS_INTERNAL_ERROR_NO_OP);
  }

  OpRetVal
  ProtocolSitedProxy::operationRead(GlobalThread*,
				    PstOutContainerInterface**&) {
    return isHomeProxy() ? DSS_PROCEED : DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolSitedProxy::operationWrite(GlobalThread*,
				     PstOutContainerInterface**&) {
    return isHomeProxy() ? DSS_PROCEED : DSS_INTERNAL_ERROR_NO_OP;
  }

  OpRetVal
  ProtocolSitedProxy::operationWrite(PstOutContainerInterface**&) {
    return isHomeProxy() ? DSS_PROCEED : DSS_INTERNAL_ERROR_NO_OP;
  }

  FaultState
  ProtocolSitedProxy::siteStateChanged(DSite* s, const FaultState& state) {
    if (!isPermFail() && (a_proxy->m_getCoordinatorSite() == s)) {
      switch (state) {
      case FS_OK:          return FS_STATE_OK;
      case FS_TEMP:        return FS_STATE_TEMP;
      case FS_LOCAL_PERM:  makePermFail(state); return FS_STATE_LOCAL_PERM;
      case FS_GLOBAL_PERM: makePermFail(state); return FS_STATE_GLOBAL_PERM;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

} //end namespace
