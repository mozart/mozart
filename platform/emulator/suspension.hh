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

#ifndef __SUSPENSIONHH
#define __SUSPENSIONHH

#include "base.hh"
#include "thr_class.hh"
#include "prop_class.hh"

// A suspension is a tagged pointer either to a propagator or a thread.

#define THREADTAG 0x1

class Suspension {

friend Bool operator == (Suspension, Suspension);

private:
  union _susp_t {
    Thread * _t;
    Propagator * _p;
    _susp_t(void) {
      DebugCode(_t = (Thread *) 0x5e5e5e5e);
    }
    _susp_t(Thread * t) : _t(t) {}
    _susp_t(Propagator * p) : _p(p) {}
  } _s;
  Bool _isThread(void) {  
    return ((_ToInt32(_s._t)) & THREADTAG); 
  }
  Thread * _getThread(void) { 
    return (Thread *) ((_ToInt32(_s._t)) & ~THREADTAG);
  }
  Propagator * _getPropagator(void) { 
    return _s._p;
  }
  void _markAsThread(Thread * t) { 
    _s._t = (Thread *) ((_ToInt32(t)) | THREADTAG);
  }
public:
  Suspension(void);

  Suspension(Thread * t) {
    _markAsThread(t);
  }
  Suspension(Propagator * p) : _s(p) {}

  static void *operator new(size_t);
  static void operator delete(void *, size_t);


  OZPRINTLONG;

  Bool isPropagator(void) { return !_isThread(); }
  Bool isThread(void) { return _isThread(); }
  Bool isNull(void) { return ((void *) _getThread()) == NULL;}

  // mm2
  void setPropagator(OZ_Propagator * p) {
    _s._p->setPropagator(p);
  }
  Propagator * getPropagator(void) {
    Assert(isPropagator());
    return _s._p;
  }
  Thread * getThread(void) {
    Assert(_isThread());
    return _getThread();
  }
  void setThread(Thread * t) {
    _markAsThread(t);
  }

  Bool isDead(void) {
    return (_isThread() 
	    ? _getThread()->isDead() 
	    : _getPropagator()->isDead());
  }
  Bool isRunnable(void) {
    return (_isThread() 
	    ? _getThread()->isRunnable() 
	    : _getPropagator()->isRunnable());
  }
  Board * getBoardInternal(void) {
    return (_isThread() 
	    ? _getThread()->getBoardInternal() 
	    : _getPropagator()->getBoardInternal());
  }
  
  void unsetLocal(void) {
    if (isPropagator())
      _getPropagator()->unsetLocal();
  }

  void setLocalPropagator(void) {
    if (isPropagator())
      _getPropagator()->setLocal();
  }
  Bool isLocal(void) {
    if (isPropagator())
      return _getPropagator()->isLocal();
    return FALSE;
  }

  void unsetUnify(void) {
    if (isPropagator())
      _getPropagator()->unsetUnify();
  }

  void setUnify(void) {
    if (isPropagator())
      _getPropagator()->setUnify();
  }
  Bool isUnify(void) {
    if (isPropagator())
      return _getPropagator()->isUnify();
    return FALSE;
  }

  void setTagged(void) { 
    if (_isThread())  
      _getThread()->setTagged();
    else
      _getPropagator()->setTagged();
  }
  void unsetTagged(void) { 
    if (_isThread())  
      _getThread()->unsetTagged();
    else
      _getPropagator()->unsetTagged();
  }
  Bool isTagged(void) { 
    return (_isThread() 
	    ? _getThread()->isTagged()
	    : _getPropagator()->isTagged());
  }

  void setExternal(void) {
    if (_isThread())
      _getThread()->setExternal();
    else
      _getPropagator()->setExternal();
  }

  Bool isExternal(void) {
    return (_isThread()
	    ? _getThread()->isExternal()
	    : _getPropagator()->isExternal());    
  }

  Bool isOFS(void) {
    if (isPropagator())
      return _getPropagator()->isOFS();
    return FALSE;
  }

  Suspension gcSuspension(int);
}; // class Suspension 

inline
Bool operator == (Suspension a, Suspension b)
{
  return (a._getPropagator() == b._getPropagator());
}

#endif
