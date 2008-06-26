/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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
#ifndef __ABSTRACT_ENTITY_IMPLEMENTATIONS_HH
#define __ABSTRACT_ENTITY_IMPLEMENTATIONS_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dss_enums.hh"
#include "dssBase.hh"

namespace _dss_internal{ //Start namespace

  // The AbstractEntity* classes are declared in dss_classes.

  // raph: XxxAbstractEntity's and XxxMediatorInterface's have been
  // merged into a single set of abstract classes XxxAbstractEntity.
  // This is the partial implementation of XxxAbstractEntity, which
  // was formerly implemented by classes XxxAbstractEntityImpl.
  //
  // The formerly existing classes AE_ProxyCallbackInterface,
  // MutableAbstractEntityImpl, RelaxedMutableAbstractEntityImpl,
  // MonotonicAbstractEntityImpl, and ImmutableAbstractEntityImpl have
  // therefore been removed.  Code has been reduced, since there was a
  // lot of code duplication in those classes, and callbacks are now
  // more direct (no more mediators).

  enum AbsOp{
    AO_NO_OP = 0,
    AO_OO_BIND,
    AO_OO_ACCESS,
    AO_OO_UPDATE,
    AO_OO_CHANGES,
    AO_STATE_WRITE,
    AO_STATE_READ,
    AO_STATE_LOCK,
    AO_STATE_UNLOCK,
    AO_STATE_EXTRACT,
    AO_STATE_INSTALL,
    AO_LZ_FETCH,
    AO_DC_SEND,
    AO_DC_EXTRACT,
    AO_DC_INSTALL,
    AO_EP_W_EXEC,
    AO_EP_W_DONE,
    AO_EP_EXTRACT,
    AO_EP_INSTALL
  };

  // apply an abstract operation on a given abstract entity
  AOcallback applyAbstractOperation(AbstractEntity* ae, const AbsOp& aop,
                                    DssThreadId* tid, DssOperationId* oid,
                                    PstInContainerInterface* pstin,
                                    PstOutContainerInterface*& pstout);

}
#endif
