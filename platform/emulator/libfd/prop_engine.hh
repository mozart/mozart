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

#ifndef __PROP_ENGINE_HH__
#define __PROP_ENGINE_HH__

#include <stdarg.h>
#include "mozart_cpi.hh"

//-----------------------------------------------------------------------------
// debug macros

//#define DEBUG_COMPOUND

#ifdef DEBUG_COMPOUND
#define CDM(A) printf A; fflush(stdout)
#else
#define CDM(A)
#endif

//-----------------------------------------------------------------------------

class HeapAlloc {
protected:
  void * alloc(int n) { return OZ_hallocChars(n); }
  virtual void _gCollect(void) = 0;
  virtual void _sClone(void) = 0;
public:
  void gCollect(void) {
    _gCollect();
  }
  void sClone(void) {
    _sClone();
  }
};

class PropAlloc {
public:
  void * alloc(int n) { return OZ_FDIntVar::operator new(sizeof(char) * n); }
  void gCollect(void) { OZ_error("Unexpected call of `PropAlloc::gCollect()'."); }
  void sClone(void) { OZ_error("Unexpected call of `PropAlloc::sClone()'."); }
};

static const int _EnlargeableArray_margin = 10;

template <class T, class M>
class EnlargeableArray : public M {

protected:
private:
  virtual void _gCollect(void) {
    T * new_array = (T *) alloc(_size * sizeof(T));
    for (int i = _size; i--; )
      new_array[i] = _array[i];
    _array = new_array;
  }
  virtual void _sClone(void) {
    T * new_array = (T *) alloc(_size * sizeof(T));
    for (int i = _size; i--; )
      new_array[i] = _array[i];
    _array = new_array;
  }
protected:
  int _size;
  T * _array;
  //
  void request(int s, int m = _EnlargeableArray_margin) {
    if (s >= _size) {
      int old_size = _size;
      _size = s + m;
      _array = realloc(_array, old_size, _size);
    }
  }
  //
  T * realloc(T * old, int old_n, int new_n) {
    if (old_n < new_n) {
      T * _new = (T *) alloc(new_n * sizeof(T));
      T * _old = old;
      for (int i = old_n; i--; )
	_new[i] = _old[i];
      return _new;
    } else {
      return old;
    }
  }
public:
  EnlargeableArray(void) : _size(0) {
    _array = (T *) NULL;
  }
  //
  EnlargeableArray(int s) : _size(s) {
    _array = s > 0 ? (T *) alloc(s * sizeof(T)) : (T *) NULL;
  }
  //
  T &operator [](int i) {
    Assert(0 <= i && i < _high);
    return _array[i];
  }
};

// intial size > 0; increase size by pushing new elements on it
template <class T, class M>
class PushArray : public EnlargeableArray<T,M> {
protected:
  int _high;
  //
  int push(T &d) {
    _array[_high] = d;
    _high += 1;
    request(_high);
    return _high-1;
  }
public:
  PushArray(int s = _EnlargeableArray_margin)
    : EnlargeableArray<T,M>(s), _high(0) { }
  //
  int getHigh(void) { return _high; }
  //
};

typedef PushArray<int, HeapAlloc> IntHeapPushArray;

// intial size == 0; increase size by explicit resizable
template <class T, class M>
class ResizeableArray : public EnlargeableArray<T,M> {
public:
  ResizeableArray(void)
    : EnlargeableArray<T,M>() { }
  //
  void resize(int new_size) {
    if (new_size > _size)
      _array = realloc(_array, _size, new_size);
  }
  void reset(void) {
    _size = 0;
  }
};

//*****************************************************************************
//
// Here starts the definition of the Propagation Engine Library
// Note all relevant names are prepended by PEL_
//
//*****************************************************************************

//-----------------------------------------------------------------------------
class PEL_PropQueue;
class PEL_PropFnctTable;

class PEL_ParamTable : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
};

//-----------------------------------------------------------------------------

class PEL_EventList : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
  void wakeup(PEL_PropQueue *, PEL_PropFnctTable *);
};

//-----------------------------------------------------------------------------
// propagation function: return type, signature, tables etc.

class PEL_SuspVar;
typedef enum { pf_failed, pf_entailed, pf_sleep } pf_return_t;
typedef pf_return_t (* pf_fnct_t)(int *, PEL_SuspVar * []);

class PEL_PropFnctTableEntry {
private:
  enum _fnct_flags { _dead = 1, _scheduled = 2};
  int       _param_idx;
  pf_fnct_t _prop_fn;
public:
  PEL_PropFnctTableEntry(pf_fnct_t fn, int idx) {
    _prop_fn = fn;
    _param_idx = idx;
  }
  int isScheduled(void) {
    return int(_prop_fn) & _scheduled;
  }
  void setScheduled(void) {
    _prop_fn = pf_fnct_t(int(_prop_fn) | _scheduled);
  }
  void unsetScheduled(void) {
    _prop_fn = pf_fnct_t(int(_prop_fn) & ~_scheduled);
  }
  int isDead(void) {
    return int(_prop_fn) & _dead;
  }
  void setDead(void) {
    _prop_fn = pf_fnct_t(int(_prop_fn) | _dead);
  }
  pf_fnct_t getFnct(void) {
    return pf_fnct_t(int(_prop_fn) & ~(_scheduled | _dead));
  }
  int getParamIdx(void) {
    return _param_idx;
  }
};

typedef PushArray<PEL_PropFnctTableEntry, HeapAlloc>
PropFnctHeapPushArray;

class PEL_PropFnctTable : public PropFnctHeapPushArray {
private:
  int add(PEL_ParamTable &, PEL_PropQueue &, pf_fnct_t, ...);
  //
public:
  //
  PEL_PropFnctTable(void) {}
  //
  int add(PEL_ParamTable &pt, PEL_PropQueue &pq, 
	  pf_fnct_t fnct, int x, int y, int z) {
    return add(pt, pq, fnct, x, y, z, -1);
  }
  int add(PEL_ParamTable &pt, PEL_PropQueue &pq, 
	  pf_fnct_t fnct, int x, int y) {
    return add(pt, pq, fnct, x, y, -1);
  }
};

//-----------------------------------------------------------------------------
typedef ResizeableArray<int, PropAlloc> IntPropResizeableArray;


static const int _PEL_PropQueue_init_maxsize = 0x10; // must be power of 2

class PEL_PropQueue {
private:
  int _alive_prop_fncts;

  int _read, _write, _size, _maxsize;

  IntPropResizeableArray _queue;

  int _failed;

  void resize(void) {
    int new_maxsize = ((_maxsize == 0) ? _PEL_PropQueue_init_maxsize : _maxsize *= 2);
    _queue.resize(new_maxsize);
    if (_write + 1 < _read) {
      for (int i = _read; i < _maxsize; i += 1)
	_queue[i + _maxsize] = _queue[i];
      _read += _maxsize;
    }
    _maxsize = new_maxsize;
  }

public:
  PEL_PropQueue(void)
    : _alive_prop_fncts(0), _read(0), _write(_PEL_PropQueue_init_maxsize - 1),
      _size(0), _maxsize(0), _failed(0) { }
  //
  void enqueue (int fnct_idx) {
    if (_size == _maxsize) resize();
    _write = (_write + 1) & (_maxsize - 1); // reason for _maxsize to 2^n
    _queue[_write] = fnct_idx;
    _size += 1;
  }
  //
  int dequeue (void) {
    if (_size == 0)
      OZ_error ( "Cannot dequeue from empty queue.");
    int fnct_idx = _queue[_read];
    _read = (_read + 1) & (_maxsize - 1);
    _size -= 1;
    return fnct_idx;
  }
  //
  pf_return_t apply(PEL_PropFnctTable &, PEL_ParamTable &, PEL_SuspVar * []);
  //
  int isEmpty(void) { return (_size == 0) || _failed; }
  // if one of the next is true then queue is empty
  void setFailed(void) { _failed = 1; }
  int isFailed(void) { return _failed; }
  //
  int isBasic(void) { return _alive_prop_fncts == 0; }
  //APF = alive propagation function
  void incAPF(void) { _alive_prop_fncts += 1; }
  //
  void decAPF(void) { _alive_prop_fncts -= 1; }
  //
  void reset(void) {
    Assert(_size == 0);
    _queue.reset();
    _read = 0;
    _write = _maxsize - 1;
    _size = 0;
  }
  //
  int getSize(void) { return _size; }
};

//-----------------------------------------------------------------------------

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

class PEL_FSetEventLists {
private:
  PEL_EventList _lowerBound;
  PEL_EventList _upperBound;
  PEL_EventList _singleValue;
public:
  PEL_EventList &getLowerBound(void) { return _lowerBound; }
  PEL_EventList &getUpperBound(void) { return _upperBound; }
  PEL_EventList &getSingleValue(void) { return _singleValue; }
  void gCollect(void) {
    _lowerBound.gCollect();
    _upperBound.gCollect();
    _singleValue.gCollect();
  }
  void sClone(void) {
    _lowerBound.sClone();
    _upperBound.sClone();
    _singleValue.sClone();
  }
};

//-----------------------------------------------------------------------------
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

class PEL_FDEventLists {
private:
  PEL_EventList _bounds;
  PEL_EventList _singleValue;
public:
  PEL_EventList &getBounds(void) { return _bounds; }
  PEL_EventList &getSingleValue(void) { return _singleValue; }
  void gCollect(void) {
    _bounds.gCollect();
    _singleValue.gCollect();
  }
  void sClone(void) {
    _bounds.sClone();
    _singleValue.sClone();
  }
};

//-----------------------------------------------------------------------------
class PEL_SuspVar {
public:
  virtual int wakeup(void) = 0;
};

class PEL_SuspFSetVar : public PEL_SuspVar {
private:
  OZ_FSetConstraint * _fset;
  //
  PEL_FSetProfile _profile;
  PEL_PropQueue * _prop_queue;
  PEL_FSetEventLists * _event_lists;
  PEL_PropFnctTable * _prop_fnct_table;
  //
  void _init(PEL_FSetEventLists &fsetel, PEL_PropQueue &pq) {
    _prop_queue = &pq;
    _event_lists = &fsetel;
  }
public:
  PEL_SuspFSetVar(void) {}
  //
  //---------------------------------------------------------------------------
  // store variable and propagation variable are identical,
  // initialization and propagation are conjoined in a single function
  PEL_SuspFSetVar * init(PEL_FSetProfile &fsetp, OZ_FSetConstraint &fset,
			 PEL_FSetEventLists &fsetel, PEL_PropQueue &pq,
			 PEL_PropFnctTable &pft, int first = 1) {
    _init(fsetel, pq);
    //
    _profile = fsetp;
    _fset = & fset;
    _prop_fnct_table = &pft;
    wakeup();
    return this;
  }
  //
  PEL_SuspFSetVar(PEL_FSetProfile &fsetp, OZ_FSetConstraint &fset,
		  PEL_FSetEventLists &fsetel, PEL_PropQueue &pq,
		  PEL_PropFnctTable &pft, int first = 1) {
    (void) init(fsetp, fset, fsetel, pq, pft, first);
  }
  //---------------------------------------------------------------------------
  // store variable and propagation variable are separate,
  // initialization and propagation are separate functions
  PEL_SuspFSetVar * init(OZ_FSetConstraint &fsetl,
			 PEL_FSetEventLists &fsetel, PEL_PropQueue &pq,
			 PEL_PropFnctTable &pft) {
    _init(fsetel, pq);
    //
    _profile.init(fsetl);
    _fset = & fsetl;
    _prop_fnct_table = &pft;
    return this;
  }
  //
  PEL_SuspFSetVar(OZ_FSetConstraint &fsetl, PEL_FSetEventLists &fsetel,
		  PEL_PropQueue &pq, PEL_PropFnctTable &pft) {
    (void) init(fsetl, fsetel, pq, pft);
  }
  //
  int propagate_to(OZ_FSetConstraint &fset, int first = 0) {
    int r = (*_fset <<= fset);
    if (r == 0)
      return 0;
    wakeup(first);
    return 1;
  }
  //
  virtual int wakeup(int first) {
    if (first || _profile.isTouchedSingleValue(*_fset))
      _event_lists->getSingleValue().wakeup(_prop_queue, _prop_fnct_table);
    //
    if (first || _profile.isTouchedLowerBound(*_fset))
      _event_lists->getLowerBound().wakeup(_prop_queue, _prop_fnct_table);
    //
    if (first || _profile.isTouchedUpperBound(*_fset))
      _event_lists->getUpperBound().wakeup(_prop_queue, _prop_fnct_table);
    //
    _profile.init(*_fset);
    //
    return _fset->isValue();
  }
  virtual int wakeup(void) { return wakeup(0); }
  //
  OZ_FSetConstraint &operator * (void) { return *_fset; }
  OZ_FSetConstraint * operator -> (void) { return _fset; }
};

class PEL_SuspFDIntVar : public PEL_SuspVar {
private:
  OZ_FiniteDomain * _fd;
  //
  PEL_FDProfile _profile;
  PEL_PropQueue    * _prop_queue;
  PEL_FDEventLists * _event_lists;
  PEL_PropFnctTable * _prop_fnct_table;
  //
  void _init(PEL_FDEventLists &fdel, PEL_PropQueue &pq) {
    _prop_queue = &pq;
    _event_lists = &fdel;
  }
public:
  PEL_SuspFDIntVar(void) {}
  //
  //---------------------------------------------------------------------------
  // store variable and propagation variable are identical,
  // initialization and propagation are conjoined in a single function
  PEL_SuspFDIntVar * init(PEL_FDProfile &fdp, OZ_FiniteDomain &fd,
			  PEL_FDEventLists &fdel, PEL_PropQueue &pq,
			  PEL_PropFnctTable &pft, int first = 1)
  {
    _init(fdel, pq);
    //    
    _profile = fdp;
    _fd = &fd;
    _prop_fnct_table = &pft;
    wakeup();
    return this;
  }
  PEL_SuspFDIntVar(PEL_FDProfile &fdp, OZ_FiniteDomain &fdv,
		   PEL_FDEventLists &fdel, PEL_PropQueue &pd,
		   PEL_PropFnctTable &pft, int first = 1) {
    (void) init(fdp, fdv, fdel, pd, pft, first);
  }
  //---------------------------------------------------------------------------
  // store variable and propagation variable are separate,
  // initialization and propagation are separate functions
  PEL_SuspFDIntVar * init(OZ_FiniteDomain &fdl, PEL_FDEventLists &fdel,
			  PEL_PropQueue &pq, PEL_PropFnctTable &pft)
  {
    _init(fdel, pq);
    //
    _profile.init(fdl);
    _fd = &fdl;
    _prop_fnct_table = &pft;
    return this;
  }
  //
  PEL_SuspFDIntVar(OZ_FiniteDomain &fdl, PEL_FDEventLists &fdel,
		   PEL_PropQueue &pd, PEL_PropFnctTable &pft) {
    (void) init(fdl, fdel, pd, pft);
  }
  // propagate store constraints to encapsulated constraints
  int propagate_to(OZ_FiniteDomain &fd, int first = 0) {
    int r = (*_fd &= fd);
    if (r == 0)
      return 0;
    wakeup(first);
    return 1;
  }
  //
  virtual int wakeup(int first) {
    if (first || _profile.isTouchedSingleValue(*_fd))
      _event_lists->getSingleValue().wakeup(_prop_queue, _prop_fnct_table);
    //
    if (first || _profile.isTouchedBounds(*_fd))
      _event_lists->getBounds().wakeup(_prop_queue, _prop_fnct_table);
    //
    _profile.init(*_fd);
    //
    return _fd->getSize() != 1;
  }
  virtual int wakeup(void) { return wakeup(0); }
  //
  OZ_FiniteDomain &operator * (void) { return *_fd; }
  OZ_FiniteDomain * operator -> (void) { return _fd; }
};

#endif
