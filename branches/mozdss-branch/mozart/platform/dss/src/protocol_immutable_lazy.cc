/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Sahlin, 2003
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
#pragma implementation "protocol_immutable_lazy.hh"
#endif

#include "protocol_immutable_lazy.hh"

namespace _dss_internal{ //Start namespace


  /******************** ProtocolImmutableLazyProxy ********************/

  bool
  ProtocolImmutableLazyProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false; 
    return true;
  }

  OpRetVal
  ProtocolImmutableLazyProxy::protocol_Access(GlobalThread* const th_id) {
    if (failed) return DSS_RAISE;
    if (stateHolder) return DSS_PROCEED;
    // ask manager if necessary, and wait
    if (a_readers.isEmpty()) m_requestState();
    a_readers.push(th_id);
    return DSS_SUSPEND;
  }

} //end namespace
