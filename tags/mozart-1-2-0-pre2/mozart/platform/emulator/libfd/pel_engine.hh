/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PEL_ENGINE_HH__
#define __PEL_ENGINE_HH__

#include "pel_internal.hh"

//-----------------------------------------------------------------------------

class PEL_Var {
protected:
  PEL_Engine * _engine;
public:
  PEL_Var(PEL_Engine * e) : _engine(e) {}
  PEL_Var(void) {}
public:
  virtual int leave(void) = 0;
  void fail(void) {}
};

//-----------------------------------------------------------------------------

class PEL_PersistentFDIntVar : public _PEL_PersistentVar {
  friend class PEL_PeristentEngine;
private:
  _PEL_FDEventLists _el;
  OZ_FiniteDomain _fd;
public:
  PEL_PersistentFDIntVar(void) : _PEL_PersistentVar() {}
  OZ_FiniteDomain &operator * (void) { return _fd; }
  _PEL_FDEventLists * operator -> (void) { return &_el; }
  _PEL_FDEventLists & getEventLists (void) { return _el; }
  void gCollect(void) {
    _fd.copyExtension();
    _el.gCollect();
  }
  void sClone(void) {
    _fd.copyExtension();
    _el.sClone();
  }
};

//-----------------------------------------------------------------------------

class PEL_PersistentFSetVar : public _PEL_PersistentVar {
private:
  _PEL_FSetEventLists _el;
  OZ_FSetConstraint _fs;
public:
  PEL_PersistentFSetVar(void) : _PEL_PersistentVar() {}
  OZ_FSetConstraint &operator * (void) { return _fs; }
  _PEL_FSetEventLists * operator -> (void) { return &_el; }
  _PEL_FSetEventLists & getEventLists (void) { return _el; }
  void gCollect(void) {
    _fs.copyExtension();
    _el.gCollect();
  }
  void sClone(void) {
    _fs.copyExtension();
    _el.sClone();
  }
};


//=============================================================================

#define MAX_PARAMS 100

class PEL_PersistentEngine {
  //
  friend class PEL_Engine;
  //
private:
  _PEL_PropagatorTable _prop_table;
  //
  int _alive_prop_fncts, _failed, _current_id;
  static _PEL_EventList * _ela[MAX_PARAMS];
  static int _current_params ;
public:
  PEL_PersistentEngine(void) 
    : _alive_prop_fncts(0), _failed(0), _current_id(0) { }
  void init(void) {
    PEL_Propagator::_pt = &_prop_table;
    PEL_Propagator::_pe = this;
  }
  //
  void sClone(void) {
    _prop_table.sClone();
  }
  void gCollect(void) {
    _prop_table.gCollect();
  }
  //
  // if one of the next is true then queue is empty
  void setFailed(void) { _failed = 1; }
  int isFailed(void) { return _failed; }
  //
  int hasNoPropsLeft(void) { return _alive_prop_fncts == 0; }
  // APF = alive propagation function
  void incAPF(void) { _alive_prop_fncts += 1; }
  void decAPF(void) { _alive_prop_fncts -= 1; }
  int &getCurrentId(void) { return _current_id; }
  int impose(PEL_Propagator * p) {
    incAPF();
    for (int i = 0; i < _current_params; i += 1) {
      _ela[i]->add(PEL_Propagator::_last_i);
    }
    _current_params = 0;
    return 0;
  }
  int expectVar(_PEL_EventList * el, int &r) {
    _PEL_EventList * _el = el;
    _ela[_current_params] = _el;
    _current_params += 1;
    CASSERT(_current_params < MAX_PARAMS);
    r = 0;
    return 0;
  }
  int expectIntVarBounds(PEL_PersistentFDIntVar &fdv) {
    _PEL_EventList * el = &(fdv.getEventLists().getBounds());
    int r;
    return expectVar(el, r);
  }
  int expectIntVarBounds(PEL_PersistentFDIntVar &fdv, int &r) {
    _PEL_EventList * el = &(fdv.getEventLists().getBounds());
    return expectVar(el, r);
  }
  int expectInt(OZ_Term v, int &r) {
    r = 0;
    return 0;
  }
};

//=============================================================================

class PEL_Engine : public _PEL_PropQueue {
private:
  PEL_PersistentEngine * _pe;
  PEL_Var ** _vs;
public:
  PEL_Engine(PEL_PersistentEngine &pe, const char * f, ...);
  ~PEL_Engine(void) {
    CASSERT(isEmpty() || isFailed());
  }
  // makes adding of new constraints possible while running propagate
  _PEL_PropagatorTable &getPropTable(void) {return _pe->_prop_table; }
  //
  // if one of the next is true then queue is empty
  void setFailed(void) { _pe->setFailed(); }
  int isFailed(void) { return _pe->isFailed(); }
  //
  int hasNoPropsLeft(void) { return _pe->hasNoPropsLeft(); }
  // APF = alive propagation function
  void incAPF(void) { _pe->incAPF(); }
  void decAPF(void) { _pe->decAPF(); }
  //
  void propagate(void) {
    if (!isFailed()) {
      while (!isEmpty()) {
	if (pf_failed == apply()) {
	  break;
	}
      }
    }
  }
  int hasReachedFixPoint(void) {
    return isEmpty() || isFailed();
  }
  PEL_Var * operator [](int i) { return _vs[i]; }
  pf_return_t apply(void);
};

//=============================================================================

class PEL_FSetProfile {
private:
  int _known_in, _known_not_in, _card_size;

public:
  PEL_FSetProfile(void) {};

  void init(OZ_FSetConstraint &fset) {
    _known_in     = fset.getKnownIn();
    _known_not_in = fset.getKnownNotIn();
    _card_size    = fset.getCardSize();
  }

  int isTouched(OZ_FSetConstraint &fset) {
    return ((_known_in < fset.getKnownIn()) ||
	    (_known_not_in < fset.getKnownNotIn()) ||
	    (_card_size > fset.getCardSize()));
  }
  int isTouchedSingleValue(OZ_FSetConstraint &fset) {
    return (_known_in +_known_not_in < fs_sup + 1) && fset.isValue();
  }
  int isTouchedLowerBound(OZ_FSetConstraint &fset) {
    return _known_in < fset.getKnownIn();
  }
  int isTouchedUpperBound(OZ_FSetConstraint &fset) {
    return _known_not_in < fset.getKnownNotIn();
  }
};

//-----------------------------------------------------------------------------

class PEL_FSetVar : public PEL_Var {
private:
  OZ_FSetConstraint * _fset;
  //
  PEL_FSetProfile _profile;
  _PEL_PropQueue * _prop_queue;
  _PEL_FSetEventLists * _event_lists;
  //
  void _init(_PEL_FSetEventLists &fsetel, _PEL_PropQueue &pq) {
    _prop_queue = &pq;
    _event_lists = &fsetel;
  }
public:
  PEL_FSetVar(void) {}
  //
  //---------------------------------------------------------------------------
  // store variable and propagation variable are identical,
  // initialization and propagation are conjoined in a single function
  PEL_FSetVar * init(PEL_FSetProfile &fsetp, OZ_FSetConstraint &fset,
		     PEL_PersistentFSetVar &fsv,
		     PEL_Engine &engine, int first = 1) {
    _init(fsv.getEventLists(), engine);
    //
    _profile = fsetp;
    _fset = & fset;
    _engine = &engine;
    leave(first);
    return this;
  }
  //
  PEL_FSetVar(PEL_FSetProfile &fsetp, OZ_FSetConstraint &fset,
	      PEL_PersistentFSetVar &fsv,
	      PEL_Engine &engine, int first = 1) {
    (void) init(fsetp, fset, fsv, engine, first);
  }
  //---------------------------------------------------------------------------
  // store variable and propagation variable are separate,
  // initialization and propagation are separate functions
  PEL_FSetVar * init(PEL_PersistentFSetVar &fsv,
			 PEL_Engine &engine) {
    _init(fsv.getEventLists(), engine);
    //
    _profile.init(*fsv);
    _fset = & *fsv;
    _engine = &engine;
    return this;
  }
  //
  PEL_FSetVar(PEL_PersistentFSetVar &fsv,
	      PEL_Engine &engine) {
    (void) init(fsv, engine);
  }
  //
  int propagate_to(OZ_FSetConstraint &fset, int first = 0) {
    int r = (*_fset <<= fset);
    if (r == 0)
      return 0;
    leave(first);
    return 1;
  }
  //
  virtual int leave(int first) {
    if (first || _profile.isTouchedSingleValue(*_fset))
      _event_lists->getSingleValue().wakeup(_prop_queue, _engine);
    //
    if (first || _profile.isTouchedLowerBound(*_fset))
      _event_lists->getLowerBound().wakeup(_prop_queue, _engine);
    //
    if (first || _profile.isTouchedUpperBound(*_fset))
      _event_lists->getUpperBound().wakeup(_prop_queue, _engine);
    //
    _profile.init(*_fset);
    //
    return _fset->isValue();
  }
  virtual int leave(void) { return leave(0); }
  //
  OZ_FSetConstraint &operator * (void) { return *_fset; }
  OZ_FSetConstraint * operator -> (void) { return _fset; }
};

//=============================================================================

class PEL_FDProfile {
private:
  int _size, _lb, _ub;

public:
  PEL_FDProfile(void) {}
  //
  void init(OZ_FiniteDomain &fd) {
    _size = fd.getSize();
    _lb   = fd.getMinElem();
    _ub   = fd.getMaxElem();
  }
  //
  int isTouchedWidth(OZ_FiniteDomain &fd) {
    return (fd.getMaxElem() - fd.getMinElem()) < (_ub - _lb);
  }
  int isTouchedLowerBound(OZ_FiniteDomain &fd) {
    return fd.getMinElem() > _lb;
  }
  int isTouchedUpperBound(OZ_FiniteDomain &fd) {
    return fd.getMaxElem() < _ub;
  }
  int isTouchedBounds(OZ_FiniteDomain &fd) {
    return isTouchedUpperBound(fd) || isTouchedLowerBound(fd);
  }
  int isTouchedSingleValue(OZ_FiniteDomain &fd) {
    int new_size = fd.getSize();
    return (new_size == 1) && (_size > 1);
  }
  int isTouched(OZ_FiniteDomain &fd) {
    return fd.getSize() < _size;
  }
};

//-----------------------------------------------------------------------------

class PEL_FDIntVar : public PEL_Var {
private:
  OZ_FiniteDomain * _fd;
  //
  PEL_FDProfile _profile;
  _PEL_PropQueue    * _prop_queue;
  _PEL_FDEventLists * _event_lists;
  //
  void _init(_PEL_FDEventLists &fdel, _PEL_PropQueue &pq) {
    _prop_queue = &pq;
    _event_lists = &fdel;
  }
public:
  PEL_FDIntVar(void) {}
  //
  //---------------------------------------------------------------------------
  // store variable and propagation variable are identical,
  // initialization and propagation are conjoined in a single function
  PEL_FDIntVar * init(PEL_FDProfile &fdp, OZ_FiniteDomain &fd,
		      PEL_PersistentFDIntVar &fdv,
		      PEL_Engine &engine, int first) {
    _init(fdv.getEventLists(), engine);
    //
    _profile = fdp;
    _fd = &fd;
      _engine = &engine;
      leave(first);
      return this;
  }
  PEL_FDIntVar(PEL_FDProfile &fdp, OZ_FiniteDomain &fd,
	       PEL_PersistentFDIntVar &fdv,
	       PEL_Engine &engine,int first = 1) {
    (void) init(fdp, fd, fdv, engine, first);
  }
  //---------------------------------------------------------------------------
  // store variable and propagation variable are separate,
  // initialization and propagation are separate functions
  PEL_FDIntVar * init(PEL_PersistentFDIntVar &fdv,
		      PEL_Engine &engine) {
    _init(fdv.getEventLists(), engine);
    //
    _profile.init(*fdv);
    _fd = & *fdv;
    _engine = &engine;
    CDM(("init fdintvar %s\n", _fd->toString()));
    return this;
  }
  //
  PEL_FDIntVar(PEL_PersistentFDIntVar &fdv,
		   PEL_Engine &engine) {
    (void) init(fdv, engine);
  }
  // propagate store constraints to encapsulated constraints
  int propagate_to(OZ_FiniteDomain &fd, int first = 0) {
    CDM(("propagate %s to ", fd.toString()));
    CDM(("%s -> ", _fd->toString()));
    int r = (*_fd &= fd);
    CDM(("#%d\n", r));
    if (r == 0)
      return 0;
    leave(first);
    return 1;
  }
  //
  int operator ==(PEL_FDIntVar &fdv) { return this == &fdv; }
  //
  virtual int leave(int first) {
    if (first || _profile.isTouchedSingleValue(*_fd))
      _event_lists->getSingleValue().wakeup(_prop_queue, _engine);
    //
    if (first || _profile.isTouchedBounds(*_fd))
      _event_lists->getBounds().wakeup(_prop_queue, _engine);
    //
    _profile.init(*_fd);
    //
    return _fd->getSize() != 1;
  }
  virtual int leave(void) { return leave(0); }
  //
  OZ_FiniteDomain &operator * (void) { return *_fd; }
  OZ_FiniteDomain * operator -> (void) { return _fd; }
};

//-----------------------------------------------------------------------------

inline
int _PEL_PersistentVar::newId(PEL_PersistentEngine &e) {
  return newId(e.getCurrentId());
}

//-----------------------------------------------------------------------------


#endif /* #ifndef __PEL_ENGINE_HH__ */
