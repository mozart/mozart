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

#define DEBUG_COMPOUND

#ifdef DEBUG_COMPOUND
#define CDM(A) printf A; fflush(stdout)
#else
#define CDM(A)
#endif

//-----------------------------------------------------------------------------

class HeapAlloc {
protected:
  void * alloc(int n) { return OZ_hallocChars(n); }
  virtual void _gc(void) = 0;
public:
  void gc(void) {
    _gc();
  }
};

class PropAlloc {
public:
  void * alloc(int n) { return OZ_FDIntVar::operator new(sizeof(char) * n); }
  void gc(void) { OZ_error("Unexpected call of `PropAlloc::gc()'."); }
};

template <class T, class M>
class EnlargeableArray : public M {
private:
  static const int _margin = 10;
  virtual void _gc(void) {
    T * new_array = (T *) alloc(_size * sizeof(T));
    for (int i = _size; i--; )
      new_array[i] = _array[i];
    _array = new_array;
  }
protected:
  int _size;
  T * _array;
  //
  void request(int s, int m = _margin) {
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
public:
  PushArray(int s = _margin)
    : EnlargeableArray<T,M>(s), _high(0) { }
  //
  int push(T &d) {
    _array[_high] = d;
    _high += 1;
    request(_high);
    return _high-1;
  }
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

//-----------------------------------------------------------------------------
class PropQueue;
class PropFnctTable;

class ParamTable : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
};

class EventList : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
  void wakeup(PropQueue *, PropFnctTable *);
};


//-----------------------------------------------------------------------------
// propagation function: return type, signature, tables etc.

class SuspVar;
typedef enum { pf_failed, pf_entailed, pf_sleep } pf_return_t;
typedef pf_return_t (* pf_fnct_t)(int *, SuspVar * []);

class PropFnctTableEntry {
private:
  enum _fnct_flags { _dead = 1, _scheduled = 2};
  int       _param_idx;
  pf_fnct_t _prop_fn;
public:
  PropFnctTableEntry(pf_fnct_t fn, int idx) {
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

typedef PushArray<PropFnctTableEntry, HeapAlloc>
PropFnctHeapPushArray;

class PropFnctTable : public PropFnctHeapPushArray {
private:
  int add(ParamTable &, PropQueue &, pf_fnct_t, ...);
public:
  PropFnctTable(void) {}

  int add(ParamTable &pt, PropQueue &pq, pf_fnct_t fnct, int x, int y) {
    return add(pt, pq, fnct, x, y, -1);
  }
  int add(ParamTable &pt, PropQueue &pq, pf_fnct_t fnct, int x, int y, int z) {
    return add(pt, pq, fnct, x, y, z, -1);
  }
};

//-----------------------------------------------------------------------------
typedef ResizeableArray<int, PropAlloc> IntPropResizeableArray;

class PropQueue {
private:
  int _alive_prop_fncts;

  int _read, _write, _size, _maxsize;

  static const int _init_maxsize = 0x10; // must be power of 2

  IntPropResizeableArray _queue;

  int _failed;

  void resize(void) {
    int new_maxsize = ((_maxsize == 0) ? _init_maxsize : _maxsize *= 2);
    _queue.resize(new_maxsize);
    if (_write + 1 < _read) {
      for (int i = _read; i < _maxsize; i += 1)
        _queue[i + _maxsize] = _queue[i];
      _read += _maxsize;
    }
    _maxsize = new_maxsize;
  }

public:
  PropQueue(void)
    : _alive_prop_fncts(0), _read(0), _write(_init_maxsize - 1),
      _size(0), _maxsize(0), _failed(0)
    {}

  void enqueue (int fnct_idx) {
    if (_size == _maxsize) resize();
    _write = (_write + 1) & (_maxsize - 1); // reason for _maxsize to 2^n
    _queue[_write] = fnct_idx;
    _size += 1;
  }

  int dequeue (void) {
    if (_size == 0)
      OZ_error ( "Cannot dequeue from empty queue.");
    int fnct_idx = _queue[_read];
    _read = (_read + 1) & (_maxsize - 1);
    _size -= 1;
    return fnct_idx;
  }
  //
  pf_return_t apply(PropFnctTable &, ParamTable &, SuspVar * []);
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

class OZ_FSetProfile {
private:
  int i;

public:
  OZ_FSetProfile(OZ_FSetConstraint &);
};

class FSetEventLists {
private:
  EventList _lowerBound;
  EventList _upperBound;
  EventList _singleValue;
public:
  EventList &getLowerBound(void) { return _lowerBound; }
  EventList &getUpperBound(void) { return _upperBound; }
  EventList &getSingleValue(void) { return _singleValue; }
  void gc(void) {
    _lowerBound.gc();
    _upperBound.gc();
    _singleValue.gc();
  }
};

//--------------------------------------------------
class OZ_FDProfile {
private:
  int _size, _lb, _ub;

public:
  OZ_FDProfile(void) {}
  void init(OZ_FiniteDomain &fd) {
    _size = fd.getSize();
    _lb   = fd.getMinElem();
    _ub   = fd.getMaxElem();
  }

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

class FDEventLists {
private:
  EventList _bounds;
  EventList _singleValue;
public:
  EventList &getBounds(void) { return _bounds; }
  EventList &getSingleValue(void) { return _singleValue; }
  void gc(void) {
    _bounds.gc();
    _singleValue.gc();
  }
};

//--------------------------------------------------
class CtEventLists {
};

//--------------------------------------------------
class SuspVar {
public:

  virtual OZ_Boolean wakeUp(void) = 0;
};

class SuspFSetVar : public SuspVar {
public:
  SuspFSetVar(OZ_FSetVar &, OZ_FSetProfile &, FSetEventLists &, PropQueue &);
  SuspFSetVar(OZ_FSetVar &, OZ_FSetConstraint &, FSetEventLists &, PropQueue &);

  OZ_FSetConstraint &operator * (void);
  OZ_FSetConstraint * operator -> (void);
};

class SuspFDIntVar : public SuspVar {
private:
  OZ_FDProfile _profile;
  PropQueue    * _prop_queue;
  FDEventLists * _event_list;

  OZ_FiniteDomain * _fd;

  PropFnctTable * _prop_fnct_table;

  void _init(FDEventLists &fdel, PropQueue &pq) {
    _prop_queue = &pq;
    _event_list = &fdel;
  }
public:
  SuspFDIntVar(void) {}
  //---------------------------------------------------------------------------
  //store variable and propagation variable are identical,
  //initialization and propagation are conjoined in a single function
  SuspFDIntVar * init(OZ_FDProfile &fdp, OZ_FiniteDomain &fd,
                      FDEventLists &fdel, PropQueue &pq, PropFnctTable &pft,
                      int first = 1)
  {
    _init(fdel, pq);
    //
    _profile = fdp;
    _fd = &fd;
    _prop_fnct_table = &pft;
    wakeUp();
    return this;
  }
  SuspFDIntVar(OZ_FDProfile &fdp, OZ_FiniteDomain &fdv,
               FDEventLists &fdel, PropQueue &pd, PropFnctTable &pft,
               int first = 1) {
    init(fdp, fdv, fdel, pd, pft, first);
  }
  //---------------------------------------------------------------------------
  //store variable and propagation variable are separate,
  //initialization and propagation are separate functions
  SuspFDIntVar * init(OZ_FiniteDomain &fdl,
                      FDEventLists &fdel, PropQueue &pq, PropFnctTable &pft)
  {
    _init(fdel, pq);
    //
    _profile.init(fdl);
    _fd = &fdl;
    _prop_fnct_table = &pft;
    return this;
  }
  // propagate store constraints to encapsulated constraints
  int propagate_to(OZ_FiniteDomain &fd, int first = 0) {
    int r = (*_fd &= fd);
    if (r == 0)
      return 0;
    wakeUp(first);
    return 1;
  }
  //
  SuspFDIntVar(OZ_FiniteDomain &fdl, FDEventLists &fdel,
               PropQueue &pd, PropFnctTable &pft) {
    init(fdl, fdel, pd, pft);
  }

  OZ_FiniteDomain &operator * (void) { return *_fd; }
  OZ_FiniteDomain * operator -> (void) { return _fd; }

  virtual OZ_Boolean wakeUp(int first  = 0) {
    if (first || _profile.isTouchedSingleValue(*_fd))
      _event_list->getSingleValue().wakeup(_prop_queue, _prop_fnct_table);
    //
    if (first || _profile.isTouchedBounds(*_fd))
      _event_list->getBounds().wakeup(_prop_queue, _prop_fnct_table);
    //
    _profile.init(*_fd);
    //
    return _fd->getSize() != 1;
  }
};

#endif
