/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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

#include "disjoint.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>
#include <stdarg.h>

#define COMPOUND

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjoint, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);

  return pe.impose(new SchedCDPropagator(OZ_args[0], OZ_args[1],
                                        OZ_args[2], OZ_args[3]));
}
OZ_C_proc_end

OZ_Return SchedCDPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();

  if (xu + xd <= yl) return P.vanish();
  if (yu + yd <= xl) return P.vanish();

  if (xl + xd > yu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }

  int lowx = yu-xd+1, lowy = xu-yd+1;
  int upx = yl+yd-1, upy = xl+xd-1;
  if (lowx <= upx) {
    OZ_FiniteDomain la;
    la.initRange(lowx,upx);
    FailOnEmpty(*x -= la);
  }
  if (lowy <= upy) {
    OZ_FiniteDomain la;
    la.initRange(lowy,upy);
    FailOnEmpty(*y -= la);
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjointC, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int dummy;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT_SUSPEND(pe, 4, expectBoolVar, dummy);

  return pe.impose(new SchedCDBPropagator(OZ_args[0],
                                          OZ_args[1],
                                          OZ_args[2],
                                          OZ_args[3],
                                          OZ_args[4]));
}
OZ_C_proc_end

OZ_Return SchedCDBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y), b(reg_b);
  PropagatorController_V_V_V P(x, y, b);
  OZ_FiniteDomain la, lb, lc, ld, l1, l2;

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int lowx, lowy, upx, upy;

  if (xu + xd <= yl) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }
  if (yu + yd <= xl) {
    FailOnEmpty(*b &= 1);
    return P.vanish();
  }

  if (xl + xd > yu){
    FailOnEmpty(*b &= 1);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu){
    FailOnEmpty(*b &= 0);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }

  if (*b == fd_singl) {
    P.vanish();
    if (b->getSingleElem() == 0) {
      return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
    } else {
      return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
    }
  }
  lowx = yu-xd+1; lowy = xu-yd+1;
  upx = yl+yd-1; upy = xl+xd-1;
  if (lowx <= upx) {
    OZ_FiniteDomain la;
    la.initRange(lowx,upx);
    FailOnEmpty(*x -= la);
  }
  if (lowy <= upy) {
    OZ_FiniteDomain la;
    la.initRange(lowy,upy);
    FailOnEmpty(*y -= la);
  }

  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();
failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

#ifdef COMPOUND

//-----------------------------------------------------------------------------
// Compound propagator interface
//-----------------------------------------------------------------------------
class HeapAlloc {
protected:
  void * alloc(int n) { return OZ_hallocChars(n); }
  virtual void _gc(void) = 0;
public:
  void gc(void) { _gc(); }
};

class PropAlloc {
public:
  void * alloc(int n) { return OZ_FDIntVar::operator new(sizeof(char) * n); }
  void gc(void) { OZ_error("Unexpected call of `PropAlloc::gc()'."); }
};

template <class T, class M>
class EnlargeableArray : public M {
private:
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
  const int _margin = 10;
  int _high;
public:
  PushArray(int s = _margin)
    : EnlargeableArray<T,M>(s), _high(0) { }
  //
  int push(T &d) {
    _array[_high] = d;
    request(_high+1);
    return _high++;
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
  int isDead(void) {
    return int(_prop_fn) & _dead;
  }
  void setScheduled(void) {
    _prop_fn = pf_fnct_t(int(_prop_fn) | _scheduled);
  }
  void unsetScheduled(void) {
    _prop_fn = pf_fnct_t(int(_prop_fn) & ~_scheduled);
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

  const int _init_maxsize = 0x10; // must be power of 2

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

  void setFailed(void) { _failed = 1; }
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
  int isEmpty(void) { return _size == 0; }
  // if one of the next is true then queue is empty
  int isFailed(void) {
    return _failed;
  }
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

void EventList::wakeup(PropQueue * pq, PropFnctTable * pft) {
  for (int i = _high; i--; ) {
    int idx = operator[](i);
    pq->enqueue(idx);
    (*pft)[idx].setScheduled();
  }
}

//-----------------------------------------------------------------------------

int PropFnctTable::add(ParamTable &pt, PropQueue &pq, pf_fnct_t fnct, ...)
{
  PropFnctTableEntry fnct_entry(fnct, pt.getHigh());
  int r = push(fnct_entry);
  pq.incAPF();

  va_list ap;

  va_start(ap, fnct);

  int param;
  do {
    param = va_arg(ap, int);
    pt.add(param);
  } while (param != -1);

  return r;
}

pf_return_t PropQueue::apply(PropFnctTable &pft,
                             ParamTable &pt,
                             SuspVar * x[])
{
  int idx = dequeue();
  PropFnctTableEntry fnct_entry = pft[idx];

  pf_fnct_t fnct = fnct_entry.getFnct();
  int paramIdx = fnct_entry.getParamIdx();

  pf_return_t r = fnct(&pt[paramIdx], x);

  if (r == pf_entailed) {
    decAPF();
    fnct_entry.setDead();
  }
  if (r == pf_failed) {
    setFailed();
  }
  fnct_entry.unsetScheduled();
  return r;
}


//--------------------------------------------------
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
  SuspFDIntVar * init(OZ_FDProfile &fdp, OZ_FiniteDomain &fd,
                      FDEventLists &fdel, PropQueue &pq, PropFnctTable &pft)
  {
    _init(fdel, pq);
    //
    _profile = fdp;
    _fd = &fd;
    _prop_fnct_table = &pft;
    wakeUp();
    return this;
  }
  SuspFDIntVar * init(OZ_FiniteDomain &fdl, OZ_FiniteDomain &fd,
                      FDEventLists &fdel, PropQueue &pq, PropFnctTable &pft)
  {
    _init(fdel, pq);
    //
    _profile.init(fdl);
    _fd = &fdl;
    _prop_fnct_table = &pft;
    wakeUp();
    return this;
  }
  //
  SuspFDIntVar(void) {}
  SuspFDIntVar(OZ_FDProfile &fdp, OZ_FiniteDomain &fdv,
               FDEventLists &fdel, PropQueue &pd, PropFnctTable &pft) {
    init(fdp, fdv, fdel, pd, pft);
  }
  SuspFDIntVar(OZ_FiniteDomain &fdl, OZ_FiniteDomain &fdv,
               FDEventLists &fdel, PropQueue &pd, PropFnctTable &pft) {
    init(fdl, fdv, fdel, pd, pft);
  }

  OZ_FiniteDomain &operator * (void) { return *_fd; }
  OZ_FiniteDomain * operator -> (void) { return _fd; }

  virtual OZ_Boolean wakeUp(void) {
    if (_profile.isTouchedSingleValue(*_fd))
      _event_list->getSingleValue().wakeup(_prop_queue, _prop_fnct_table);
    //
    if (_profile.isTouchedBounds(*_fd))
      _event_list->getBounds().wakeup(_prop_queue, _prop_fnct_table);
    //
    _profile.init(*_fd);
    //
    return _fd->getSize() != 1;
  }
};

//-----------------------------------------------------------------------------
// TasksOverlapPropagator

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_nit);

private:
  static OZ_PropagatorProfile profile;

  OZ_FDProfile _x_profile, _y_profile;
  PropFnctTable _prop_fnct_table;
  ParamTable _param_table;

  // clause 1: t1 + d1 >: t2 /\ t2 + d2 >: t1 /\ o =: 1
  enum {_cl1_t1 = 0, _cl1_t2, _cl1_o,

  // clause 2: t1 + d1 =<: t2 /\ o =: 0
        _cl2_t1, _cl2_t2, _cl2_o,

  // clause 3: t2 + d2 =<: t1 /\ o =: 0
        _cl3_t1, _cl3_t2, _cl3_o, nb_lvars };

  enum {_d1 = nb_lvars, _d2, nb_consts};

  OZ_FiniteDomain _ld[nb_lvars];
  FDEventLists    _el[nb_lvars];
  PropQueue       _prop_queue_cl1, _prop_queue_cl2, _prop_queue_cl3;

public:
  TasksOverlapPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd,
                         OZ_Term o);

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual void updateHeapRefs(OZ_Boolean duplicate = OZ_FALSE) {
    Propagator_D_I_D_I_D::updateHeapRefs(duplicate);

    // here goes the additional stuff:
    _prop_fnct_table.gc();
    _param_table.gc();
    for (int i = nb_lvars; i--; ) {
      _ld[i].copyExtension();
      _el[i].gc();
    }
  }
};

//-----------------------------------------------------------------------------
// propagation functions

// X + C <= Y
pf_return_t lessEqOff(int * map, SuspVar * regs[])
{
  SuspFDIntVar &x = *(SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  SuspFDIntVar &y = *(SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x <= (y->getMaxElem() - c));
  FailOnEmpty(*y >= (x->getMinElem() + c));

  if (x->getMaxElem() + c <= y->getMinElem()) {
    x.wakeUp();
    y.wakeUp();
    return pf_entailed;
  }

  if (x->getMinElem() + c > y->getMaxElem())
    goto failure;

  return (x.wakeUp() | y.wakeUp()) ? pf_sleep : pf_entailed;

 failure:
  return pf_failed;
}

// X + C > Y
pf_return_t greaterOff(int * map, SuspVar * regs[])
{
  SuspFDIntVar &x = *(SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  SuspFDIntVar &y = *(SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x >= (y->getMinElem() - c + 1));
  FailOnEmpty(*y <= (x->getMaxElem() + c - 1));

  if (x->getMinElem() + c > y->getMaxElem()) {
    x.wakeUp();
    y.wakeUp();
    return pf_entailed;
  }

  if (x->getMaxElem() + c <= y->getMinElem())
    goto failure;

  return (x.wakeUp() | y.wakeUp()) ? pf_sleep : pf_entailed;

 failure:
  return pf_failed;
}

//-----------------------------------------------------------------------------
// constructor

TasksOverlapPropagator::TasksOverlapPropagator(OZ_Term x,
                                               OZ_Term xd,
                                               OZ_Term y,
                                               OZ_Term yd,
                                               OZ_Term o)
  : Propagator_D_I_D_I_D(x, xd, y, yd, o)
{
  // clause 1
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl1, greaterOff,
                                    _cl1_t1, _d1, _cl1_t2);
    _el[_cl1_t1].getBounds().add(fnct_idx);
    _el[_cl1_t2].getBounds().add(fnct_idx);

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl1, greaterOff,
                                    _cl1_t2, _d2, _cl1_t1);
    _el[_cl1_t1].getBounds().add(fnct_idx);
    _el[_cl1_t2].getBounds().add(fnct_idx);

    _ld[_cl1_t1].initFull();
    _ld[_cl1_t2].initFull();
    _ld[_cl1_o].initSingleton(1);
  }
  // clause 2
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl2, lessEqOff,
                                    _cl2_t1, _d1, _cl2_t2);
    _el[_cl2_t1].getBounds().add(fnct_idx);
    _el[_cl2_t2].getBounds().add(fnct_idx);

    _ld[_cl2_t1].initFull();
    _ld[_cl2_t2].initFull();
    _ld[_cl2_o].initSingleton(0);
  }

  // clause 3
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl3, lessEqOff,
                                    _cl3_t2, _d2, _cl3_t1);
    _el[_cl3_t1].getBounds().add(fnct_idx);
    _el[_cl3_t2].getBounds().add(fnct_idx);

    _ld[_cl3_t1].initFull();
    _ld[_cl3_t2].initFull();
    _ld[_cl3_o].initSingleton(0);
  }
}

//-----------------------------------------------------------------------------
// propagation member function

OZ_Return TasksOverlapPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &d1 = reg_xd, &d2 = reg_yd;

  OZ_FDIntVar _t1(reg_x), _t2(reg_y), _o(reg_b);
  PropagatorController_V_V_V P(_t1, _t2, _o);

  _prop_queue_cl1.reset();
  _prop_queue_cl2.reset();
  _prop_queue_cl3.reset();

  SuspFDIntVar _x[nb_lvars];
  SuspVar * x[nb_lvars+nb_consts];

  x[_d1] = (SuspVar *) d1;
  x[_d2] = (SuspVar *) d2;

  int not_init_cl1 = 1, not_init_cl2 = 1,  not_init_cl3 = 1;

  do {
    int nb_failed_clauses = 0;

    //--------------------------------------------------
    // clause 1
    if (!_prop_queue_cl1.isFailed()) {
      // init registers (only once)
      if (not_init_cl1) {
        x[_cl1_t1]=_x[_cl1_t1].init(_ld[_cl1_t1], *_t1, _el[_cl1_t1],
                                    _prop_queue_cl1, _prop_fnct_table);
        x[_cl1_t2]=_x[_cl1_t2].init(_ld[_cl2_t1], *_t2, _el[_cl2_t1],
                                    _prop_queue_cl1, _prop_fnct_table);
        x[_cl1_o] =_x[_cl1_o].init(_ld[_cl1_o], *_o, _el[_cl1_o],
                                   _prop_queue_cl1, _prop_fnct_table);
        not_init_cl1 = 0;
      }
      if (nb_failed_clauses == 2) { // this clause has to be committed
        // t1 + d1 > t2
        {
          addImpose(fd_prop_bounds, reg_x);
          addImpose(fd_prop_bounds, reg_y);
          impose(new LessEqOffPropagator(reg_y, reg_x, d1-1));
        }
        // t2 + d2 > t1
        {
          addImpose(fd_prop_bounds, reg_x);
          addImpose(fd_prop_bounds, reg_y);
          impose(new LessEqOffPropagator(reg_x, reg_y, d2-1));
        }
        // o = 1
        FailOnEmpty(*_o &= 1);
        goto vanish;
      }
      // run propagation functions
      while (!_prop_queue_cl1.isEmpty()) {
        pf_return_t r = _prop_queue_cl1.apply(_prop_fnct_table,
                                              _param_table, x);
        if (r == pf_failed) {
          nb_failed_clauses += 1;
          if (nb_failed_clauses == 3)
            goto failure;
          goto clause2;
        }
      }
      // if no prop fncts are left and basic constraints subsumed then
      // the clause is entailed
      if (_prop_queue_cl1.isBasic() &&
          _t1->getSize() <= _x[_cl1_t1]->getSize() &&
          _t2->getSize() <= _x[_cl1_t2]->getSize() &&
          _o->getSize()  <= _x[_cl1_o]->getSize()) {
        goto vanish;
      }
    }
    //
    //--------------------------------------------------
    // clause 2
  clause2:
    if (!_prop_queue_cl2.isFailed()) {
      // init registers (only once)
      if (not_init_cl2) {
        x[_cl2_t1]=_x[_cl2_t1].init(_ld[_cl2_t1], *_t1, _el[_cl2_t1],
                                    _prop_queue_cl2, _prop_fnct_table);
        x[_cl2_t2]=_x[_cl2_t2].init(_ld[_cl2_t1], *_t2, _el[_cl2_t1],
                                    _prop_queue_cl2, _prop_fnct_table);
        x[_cl2_o] =_x[_cl2_o].init(_ld[_cl2_o], *_o, _el[_cl2_o],
                                   _prop_queue_cl2, _prop_fnct_table);
        not_init_cl2 = 0;
      }
      if (nb_failed_clauses == 2) { // this clause has to be committed
        // t1 + d1 <= t2
        {
          addImpose(fd_prop_bounds, reg_x);
          addImpose(fd_prop_bounds, reg_y);
          impose(new LessEqOffPropagator(reg_x, reg_y, -d1));
        }
        // o = 1
        FailOnEmpty(*_o &= 0);
        goto vanish;
      }
      // run propagation functions
      while (!_prop_queue_cl2.isEmpty()) {
        int prop_fnct_idx = _prop_queue_cl2.dequeue();
        pf_return_t r = _prop_queue_cl2.apply(_prop_fnct_table,
                                              _param_table, x);
        if (r == pf_failed) {
          nb_failed_clauses += 1;
          if (nb_failed_clauses == 3)
            goto failure;
          goto clause3;
        }
      }
      // if no prop fncts are left and basic constraints subsumed then
      // the clause is entailed
      if (_prop_queue_cl2.isBasic() &&
          _t1->getSize() <= _x[_cl2_t1]->getSize() &&
          _t2->getSize() <= _x[_cl2_t2]->getSize() &&
          _o->getSize()  <= _x[_cl2_o]->getSize()) {
        goto vanish;
      }
    }
    //
    //--------------------------------------------------
    // clause 3
  clause3:
    if (!_prop_queue_cl3.isFailed()) {
      // init registers (only once)
      if (not_init_cl3) {
        x[_cl3_t1]=_x[_cl3_t1].init(_ld[_cl3_t1], *_t1, _el[_cl3_t1],
                                    _prop_queue_cl3, _prop_fnct_table);
        x[_cl3_t2]=_x[_cl3_t2].init(_ld[_cl3_t1], *_t2, _el[_cl3_t1],
                                    _prop_queue_cl3, _prop_fnct_table);
        x[_cl3_o] =_x[_cl3_o].init(_ld[_cl3_o], *_o, _el[_cl3_o],
                                   _prop_queue_cl3, _prop_fnct_table);
        not_init_cl3 = 0;
      }
      if (nb_failed_clauses == 2) { // this clause has to be committed
        // t2 + d2 <= t1
        {
          addImpose(fd_prop_bounds, reg_x);
          addImpose(fd_prop_bounds, reg_y);
          impose(new LessEqOffPropagator(reg_y, reg_x, -d2));
        }
        // o = 1
        FailOnEmpty(*_o &= 0);
        goto vanish;
      }
      // run propagation functions
      while (!_prop_queue_cl3.isEmpty()) {
        int prop_fnct_idx = _prop_queue_cl3.dequeue();
        pf_return_t r = _prop_queue_cl3.apply(_prop_fnct_table,
                                              _param_table, x);
        if (r == pf_failed) {
          nb_failed_clauses += 1;
          if (nb_failed_clauses == 3)
            goto failure;
          goto end;
        }
      }
      // if no prop fncts are left and basic constraints subsumed then
      // the clause is entailed
      if (_prop_queue_cl3.isBasic() &&
          _t1->getSize() <= _x[_cl3_t1]->getSize() &&
          _t2->getSize() <= _x[_cl3_t2]->getSize() &&
          _o->getSize()  <= _x[_cl3_o]->getSize()) {
        goto vanish;
      }
    }
    //
  end:
    ;
  } while (_prop_queue_cl1.isEmpty() &&
           _prop_queue_cl2.isEmpty() &&
           _prop_queue_cl3.isEmpty());
  // lift common information
  {
    OZ_FiniteDomain u_t1(fd_empty), u_t2(fd_empty), u_o(fd_empty);
    if (!_prop_queue_cl1.isFailed()) {
      u_t1 = u_t1 | *_x[_cl1_t1];
      u_t2 = u_t2 | *_x[_cl1_t2];
      u_o  = u_o  | *_x[_cl1_o];
    }
    if (!_prop_queue_cl2.isFailed()) {
      u_t1 = u_t1 | *_x[_cl2_t1];
      u_t2 = u_t2 | *_x[_cl2_t2];
      u_o  = u_o  | *_x[_cl2_o];
    }
    if (!_prop_queue_cl3.isFailed()) {
      u_t1 = u_t1 | *_x[_cl3_t1];
      u_t2 = u_t2 | *_x[_cl3_t2];
      u_o  = u_o  | *_x[_cl3_o];
    }
    FailOnEmpty(*_t1 &= u_t1);
    FailOnEmpty(*_t2 &= u_t2);
    FailOnEmpty(*_o  &= u_o);
  }
  //
 leave:
  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();
  //
 vanish:
  OZ_DEBUGPRINTTHIS("out: ");
  return P.vanish();
  //
 failure:
  OZ_DEBUGPRINT(("fail"));
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_tasksOverlap, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD ","
                   OZ_EM_INT "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int dummy;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT_SUSPEND(pe, 4, expectBoolVar, dummy);

  return pe.impose(new TasksOverlapPropagator(OZ_args[0],
                                              OZ_args[1],
                                              OZ_args[2],
                                              OZ_args[3],
                                              OZ_args[4]));
}
OZ_C_proc_end
#else
OZ_C_proc_begin(fdp_tasksOverlap, 5)
{
  return OZ_ENTAILED;
}
OZ_C_proc_end
#endif

//-----------------------------------------------------------------------------
// static member

OZ_PropagatorProfile SchedCDPropagator::profile;
OZ_PropagatorProfile SchedCDBPropagator::profile;
#ifdef COMPOUND
OZ_PropagatorProfile TasksOverlapPropagator::profile;
#endif
