/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __PROP_CLASS_HH
#define __PROP_CLASS_HH

#include "base.hh"
#include "mem.hh"

enum PropagatorFlag {
  P_null     = 0x000000,
  P_gcmark   = 0x000001, // you must not change that
  P_dead     = 0x000002,
  P_tag      = 0x000004,
  P_nmo      = 0x000008,
  P_local    = 0x000010,
  P_runnable = 0x000020,
  P_ofs      = 0x000040,
  P_ext      = 0x000080,
  P_unify    = 0x000100,
  P_failed   = 0x000200,
  P_max      = 0x800000
};


#define MARKFLAG(F)        (_flags |= (F))
#define UNMARKFLAG(F)      (_flags &= ~(F))
#define UNMARKFLAGTO(T, F) ((T) (_flags & ~(F)))
#define ISMARKEDFLAG(F)    (_flags & (F))

class Propagator {
private:
  unsigned _flags;
  OZ_Propagator * _p;
  Board * _b;
  static Propagator * _runningPropagator;
public:
  Propagator(OZ_Propagator * p, Board * b) 
    : _p(p), _b(b), _flags(p->isMonotonic() ? P_null : P_nmo) 
  {
    Assert(_p);
    Assert(_b);
  }

  OZ_Propagator * getPropagator(void) { return _p; }

  void setPropagator(OZ_Propagator * p) { 
    Assert (p);
    Assert (_p);
    _p = p; 
    if (! _p->isMonotonic())
      MARKFLAG(P_nmo);
  }

  USEHEAPMEMORY;

  OZPRINTLONG;

  static void setRunningPropagator(Propagator * p) { _runningPropagator = p;} 
  static Propagator * getRunningPropagator(void) { return _runningPropagator;} 

  void dispose(void) {
    delete _p;
  }

  Propagator * gcPropagator(void);
  Propagator * gcPropagatorOutlined(void);
  void gcRecurse(void);
  Bool gcIsMarked(void);
  Bool gcIsMarkedOutlined(void);
  void gcMark(Propagator *);
  void ** gcGetMarkField(void);
  Propagator * gcGetFwdOutlined(void);
  Propagator * gcGetFwd(void);
  
  Board * getBoardInternal(void) {
    return _b;
  }
  
  Bool isFailed(void) {
    return ISMARKEDFLAG(P_failed);
  }
  void setFailed(void) {
    MARKFLAG(P_failed);
  }

  Bool isDead(void) {
    return ISMARKEDFLAG(P_dead);
  }
  void setDead(void) {
    MARKFLAG(P_dead);
  }
  
  Bool isRunnable(void) {
    return ISMARKEDFLAG(P_runnable);
  }
  void unsetRunnable(void) {
    UNMARKFLAG(P_runnable);
  }
  void setRunnable(void) {
    MARKFLAG(P_runnable);
  }
    
  Bool isLocal(void) {
    return ISMARKEDFLAG(P_local);
  }
  void unsetLocal(void) {
    UNMARKFLAG(P_local);
  }
  void setLocal(void) {
    MARKFLAG(P_local);
  }
  
  Bool isUnify(void) {
    return ISMARKEDFLAG(P_unify);
  }
  void unsetUnify(void) {
    UNMARKFLAG(P_unify);
  }
  void setUnify(void) {
    MARKFLAG(P_unify);
  }
  
  Bool isTagged(void) {
    return ISMARKEDFLAG(P_tag);
  }
  void setTagged(void) {
    MARKFLAG(P_tag);
  }
  void unsetTagged(void) {
    UNMARKFLAG(P_tag);
  }

  Bool isOFS(void) {
    return ISMARKEDFLAG(P_ofs); 
  }
  void setOFS(void) {
    MARKFLAG(P_ofs);
  }
  
  Bool isExternal(void) {
    return ISMARKEDFLAG(P_ext);
  }
  void setExternal(void) {
    MARKFLAG(P_ext);
  }
    
  OZ_NonMonotonic::order_t getOrder(void) {
    return _p->getOrder();
  }

  Bool isNMO(void) {
    return ISMARKEDFLAG(P_nmo);
  }

  void setNMO(void) {
    MARKFLAG(P_nmo);
  }

  OZ_Propagator * swapPropagator(OZ_Propagator * prop) {
    OZ_Propagator * p = _p; 
    setPropagator(prop);
    return p;
  }
};

inline 
Bool isUnifyCurrentPropagator () {
  return Propagator::getRunningPropagator()->isUnify();
}

#endif
