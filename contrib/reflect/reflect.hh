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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __REFL_CONSTR__HH__
#define __REFL_CONSTR__HH__

#include "var_all.hh"

//#define DEBUG

//-----------------------------------------------------------------------------

#define INIT_FUNC(F_NAME) OZ_C_proc_interface * F_NAME(void)

extern "C" INIT_FUNC(oz_init_module);

class PropagatorReference : public OZ_SituatedExtension {
  friend INIT_FUNC(oz_init_module);

private:
  static int _id;

  Propagator * _p;

public:
  PropagatorReference(Propagator * p) : _p(p) {}

  OZ_Boolean operator == (PropagatorReference &pr) {
    return (_p == pr._p);
  }

  static int getId(void) { return _id; }

  Propagator * getPropagator(void) { return _p; }

  virtual int getIdV(void) { return _id; }

  virtual OZ_SituatedExtension * gcV(void) {
    return new PropagatorReference(_p);
  }

  virtual void gcRecurseV(void) {
    _p = _p->gcPropagatorOutlined();
  }

  virtual OZ_Term printV(int) {
    char buf[100];
    sprintf(buf, "<(Propagator *): %p>", _p);
    return OZ_atom(buf);
  }
};

//-----------------------------------------------------------------------------

OZ_Term reflect_propagator(Suspension);
OZ_Term reflect_thread(Suspension);

//-----------------------------------------------------------------------------

OZ_Term propagator2Term(Propagator * p) {
  return oz_makeTaggedExtension(new PropagatorReference(p));
}

//-----------------------------------------------------------------------------

#ifdef DEBUG
#define DEBUGPRINT(A) printf A; fflush(stdout)
#else
#define DEBUGPRINT(A)
#endif

#endif
