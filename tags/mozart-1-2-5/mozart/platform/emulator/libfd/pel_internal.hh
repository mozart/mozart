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

#ifndef __PEL_INTERNAL_HH__
#define __PEL_INTERNAL_HH__


#include <stdarg.h>
#include <ctype.h>
#include "mozart_cpi.hh"

//-----------------------------------------------------------------------------
// debug macros

//#define DEBUG_PEL

#ifdef DEBUG_PEL
#define CDM(A) printf A; fflush(stdout)
#define CMD(C) C
#define CASSERT(C)					\
  if (! (C)) {						\
    fprintf(stderr,"CASSERT %s failed (%s:%d).\n",	\
	    #C,__FILE__, __LINE__);			\
    fflush(stderr);					\
    OZ_error("CASSERT fired!"); \
  }
#else
#define CDM(A)
#define CMD(C)
#define CASSERT(C)
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
  void * alloc(int n) { 
    return OZ_CPIVar::operator new(sizeof(char) * n); 
  }
  void gCollect(void) { 
    OZ_error("Unexpected call of `PropAlloc::gCollect()'.");
  }
  void sClone(void) { 
    OZ_error("Unexpected call of `PropAlloc::sClone()'."); 
  }
};

static const int _EnlargeableArray_margin = 10;

template <class T, class M>
class EnlargeableArray : public M {
private:
  virtual void _gCollect(void) {
    T * new_array = (T *) alloc(_size * sizeof(T));
    for (int i = _size; i--; ) {
      new_array[i] = _array[i];
    }
    _array = new_array;
  }
  virtual void _sClone(void) {
    T * new_array = (T *) alloc(_size * sizeof(T));
    for (int i = _size; i--; ) {
      new_array[i] = _array[i];
    }
    _array = new_array;
  }
protected:
  int _size;
  T * _array;
  //
  T * realloc(T * old, int old_n, int new_n) {
    if (old_n < new_n) {
      T * _new = (T *) alloc(new_n * sizeof(T));
      T * _old = old;
      for (int i = old_n; i--; ) {
	_new[i] = _old[i];
      }
      return _new;
    } else {
      return old;
    }
  }
  //
  void request(int s, int m = _EnlargeableArray_margin) {
    if (s >= _size) {
      int old_size = _size;
      _size = s + m;
      _array = realloc(_array, old_size, _size);
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
    CASSERT(0 <= i && i < _size);
    return _array[i];
  }
};

// intial size > 0; increase size by pushing new elements on it
template <class T, class M>
class PushArray : public EnlargeableArray<T,M> {
protected:
  int _high;
  //
public:
  int push(T &d) {
    _array[_high] = d;
    _high += 1;
    request(_high);
    return _high-1;
  }
  //
  PushArray(int s = _EnlargeableArray_margin)
    : EnlargeableArray<T,M>(s), _high(0) { }
  //
  int getHigh(void) { return _high; }
  //
};

typedef PushArray<int, HeapAlloc> IntHeapPushArray;

// initial size == 0; increase size by explicit resizable
template <class T, class M>
class ResizeableArray : public EnlargeableArray<T,M> {
public:
  ResizeableArray(void) : EnlargeableArray<T,M>() { }
  //
  void resize(int new_size) {
    if (new_size > _size) {
      _array = realloc(_array, _size, new_size);
      _size = new_size;
    }
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
// forward declaration:
class _PEL_PropQueue;
class PEL_PersistentEngine;
class PEL_Engine;

//-----------------------------------------------------------------------------

class _PEL_EventList : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
  void wakeup(_PEL_PropQueue *, PEL_Engine *);
};

//-----------------------------------------------------------------------------
// propagation function: return type, signature, tables etc.

class PEL_Var;
typedef enum { pf_failed, pf_entailed, pf_sleep } pf_return_t;

//-----------------------------------------------------------------------------

// has to be power of 2
#define ALIGN_SIZE sizeof(double)

class _PEL_PropagatorHeap : public EnlargeableArray<char,HeapAlloc> {
protected:
  int _high;
public:
  _PEL_PropagatorHeap(int s = 0)
    : EnlargeableArray<char,HeapAlloc>(s), _high(0) { }
  //
  void * allocate(size_t s, int &i) {
    // round up to sizeof(double)
    int s_align = s + (ALIGN_SIZE - (s & (ALIGN_SIZE-1)));
    //
    request(_high + s_align);
    //
    void * r = &_array[_high];
    i = _high;
    _high += s_align;
    return r;
  }
  //
  char * getAddress(int i) { return _array + i; }
};

//-----------------------------------------------------------------------------

class _PEL_PropagatorEntries : public IntHeapPushArray {
public:
  int add(int i) { return push(i); }
};

//-----------------------------------------------------------------------------

class PEL_Propagator;

class _PEL_PropagatorTable {
private:
  _PEL_PropagatorEntries _entries;
  _PEL_PropagatorHeap _heap;
public:
  _PEL_PropagatorTable(void) {}
  void * allocate(size_t s, int &i) {
    int pi;
    void * r = _heap.allocate(s, pi);
    i = _entries.add(pi);
    return r;
  }
  PEL_Propagator * getPropagator(int i) { 
    return (PEL_Propagator *) _heap.getAddress(_entries[i]);
  }
  void gCollect(void);
  void sClone(void);
  void print(PEL_Engine &);
};

//-----------------------------------------------------------------------------

class PEL_Propagator {
  friend class PEL_PersistentEngine;
private:
  enum _fnct_flags { _none= 0, _dead = 1, _scheduled = 2} _flags;
  static int _last_i;
  static _PEL_PropagatorTable * _pt;
protected:
  static PEL_PersistentEngine * _pe;
public:
  PEL_Propagator(void) : _flags(_none) { }
  virtual pf_return_t propagate(PEL_Engine &) = 0;
  virtual void print(PEL_Engine &) { printf("no printing for propagator.\n"); }
  //
  virtual void gCollect(void) { }
  virtual void sClone(void) { }
  //
  void setPropagatorTable(_PEL_PropagatorTable * pt) { _pt = pt; }
  //
  static void * operator new(size_t s) {
    return _pt->allocate(s, _last_i); 
  }
  //
  int getLastIndex(void) { return _last_i; }
  //
  static void operator delete(void *, size_t) {}
  //
  int isScheduled(void) {
    return _flags & _scheduled;
  }
  void setScheduled(void) {
    _flags = PEL_Propagator::_fnct_flags(_flags | _scheduled);
  }
  void unsetScheduled(void) {
    _flags = PEL_Propagator::_fnct_flags(_flags & ~_scheduled);
  }
  int isDead(void) {
    return _flags & _dead;
  }
  void setDead(void) {
    _flags = PEL_Propagator::_fnct_flags(_flags | _dead);
  }
};

inline
void _PEL_PropagatorTable::gCollect(void) {
  _entries.gCollect();
  _heap.gCollect();
  for (int i = 0; i < _entries.getHigh(); i += 1) {
    getPropagator(i)->gCollect();
  }
}

inline
void _PEL_PropagatorTable::sClone(void) {
  _entries.sClone();
  _heap.sClone();
  for (int i = 0; i < _entries.getHigh(); i += 1) {
    getPropagator(i)->sClone();
  }
}

inline 
void _PEL_PropagatorTable::print(PEL_Engine &e) {
  for (int i = 0; i < _entries.getHigh(); i += 1) {
    printf("entry %d: ", i);
    getPropagator(i)->print(e);
  } 
}

//-----------------------------------------------------------------------------

typedef ResizeableArray<int, PropAlloc> IntPropResizeableArray;

static const int _PEL_PropQueue_init_maxsize = 0x10; // must be power of 2

class _PEL_PropQueue {
private:
  int _read, _write, _size, _maxsize;
  //
  IntPropResizeableArray _queue;
  //
  void resize(void) {
    int new_maxsize = ((_maxsize == 0) 
		       ? _PEL_PropQueue_init_maxsize 
		       : _maxsize *= 2);
    _queue.resize(new_maxsize);
    if (_write + 1 < _read) {
      for (int i = _read; i < _maxsize; i += 1)
	_queue[i + _maxsize] = _queue[i];
      _read += _maxsize;
    }
    _maxsize = new_maxsize;
  }
  //
public:
  _PEL_PropQueue(void)
    : _read(0), _write(_PEL_PropQueue_init_maxsize - 1),
      _size(0), _maxsize(0) { }
  //
  void reset(void) {
    CASSERT(_size == 0);
    _queue.reset();
    _read = 0;
    _write = _maxsize - 1;
    _size = 0;
  }
  //
  void enqueue (int fnct_idx) {
    if (_size == _maxsize) {
      resize();
    }
    _write = (_write + 1) & (_maxsize - 1); // reason for _maxsize to 2^n
    _queue[_write] = fnct_idx;
    _size += 1;
  }
  //
  int dequeue (void) {
    if (_size == 0) {
      OZ_error ( "Cannot dequeue from empty queue.");
    }
    int fnct_idx = _queue[_read];
    _read = (_read + 1) & (_maxsize - 1);
    _size -= 1;
    return fnct_idx;
  }
  //
  int isEmpty(void) { return (_size == 0); }
  //
  int getSize(void) { return _size; }
};

//-----------------------------------------------------------------------------

class _PEL_FSetEventLists {
private:
  _PEL_EventList _lowerBound;
  _PEL_EventList _upperBound;
  _PEL_EventList _singleValue;
public:
  _PEL_EventList &getLowerBound(void) { return _lowerBound; }
  _PEL_EventList &getUpperBound(void) { return _upperBound; }
  _PEL_EventList &getSingleValue(void) { return _singleValue; }
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

class _PEL_FDEventLists {
private:
  _PEL_EventList _bounds;
  _PEL_EventList _singleValue;
public:
  _PEL_EventList &getBounds(void) { return _bounds; }
  _PEL_EventList &getSingleValue(void) { return _singleValue; }
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

class _PEL_PersistentVar {
private:
  int _id;
public:
  _PEL_PersistentVar(void) : _id(-1) {}
  void setId(int id) { _id = id; }
  int getId(void) { return _id; }
  int newId(int &_new_id) { 
    if (_id == -1) {
      _id = _new_id;
      _new_id += 1;
    }
    return _id;
  }
  int newId(PEL_PersistentEngine &e);
};

//-----------------------------------------------------------------------------

#endif /* #ifndef __PEL_INTERNAL_HH__ */
