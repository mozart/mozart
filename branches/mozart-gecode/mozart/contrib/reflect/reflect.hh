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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __REFLECT__HH__
#define __REFLECT__HH__

#include "var_bool.hh"
#include "var_fd.hh"
#include "var_fs.hh"
#include "var_ct.hh"
#include "var_simple.hh"
#include "prop_int.hh"
#include "cpi.hh"

//#define DEBUG

//-----------------------------------------------------------------------------

#ifdef DEBUG

#define DEBUG_ASSERT(Cond)                                              \
  if (! (Cond)) {                                                       \
    OZ_error("%s:%d assertion '%s' failed",__FILE__,__LINE__,#Cond);    \
  }

#define _DEBUGPRINT(A)                           \
printf("(%s:%d) ", __FILE__, __LINE__);         \
printf A;                                       \
printf("\n");                                   \
fflush(stdout)

#ifdef PROGRESS

#define DEBUGPRINT(A) printf("+"); fflush(stdout)

#else

#define DEBUGPRINT(A)                           \
printf("(%s:%d) ", __FILE__, __LINE__);         \
printf A;                                       \
printf("\n");                                   \
fflush(stdout)

#endif

#else
#define DEBUGPRINT(A)
#define _DEBUGPRINT(A)
#define DEBUG_ASSERT(EXPR)
#endif

//-----------------------------------------------------------------------------

#define INIT_FUNC(F_NAME) OZ_C_proc_interface * F_NAME(void)

extern "C" INIT_FUNC(oz_init_module);

class PropagatorReference : public OZ_Extension {
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

  virtual OZ_Extension * sCloneV(void) {
    DEBUGPRINT(("sCloneV\n"));
    return new PropagatorReference(_p);
  }

  virtual void sCloneRecurseV(void) {
    DEBUGPRINT(("sCloneRecursiveV\n"));
    if (_p) {
      _p = (Propagator *) ((Suspendable *) _p)->sCloneSuspendable();
    }
  }

  virtual OZ_Extension * gCollectV(void) {
    DEBUGPRINT(("gCollectV\n"));
    return new PropagatorReference(_p);
  }

  virtual void gCollectRecurseV(void) {
    DEBUGPRINT(("gCollectRecursiveV\n"));
    if (_p) {
      _p = (Propagator *) ((Suspendable *) _p)->gCollectSuspendable();
    }
  }


  virtual OZ_Term printV(int) {
    char buf[100];
    sprintf(buf, "<(Propagator *): %p>", _p);
    return OZ_atom(buf);
  }

  // A discarded propagator reference has a NULL reference but the
  // propagator may have been discarded by constraint solving any
  // (entailment), so check the corresponding suspendable
  OZ_Boolean isDiscarded(void) {
    Suspendable * susp = (Suspendable *) _p;
    return (_p == (Propagator *) NULL) || susp->isDead();
  }

  void discard(void) {
    if (isDiscarded())
      return;
    // do the right stuff here
    oz_closeDonePropagator(_p);

    _p = (Propagator *) NULL;
  }

  void activate(void) {
    Suspendable * susp = (Suspendable *) _p;
    (void) susp->_wakeup_outline(oz_currentBoard(), pc_propagator);
    susp->setActive();
  }

  void deactivate(void) {
    Suspendable * susp = (Suspendable *) _p;
    susp->unsetActive();
  }

  OZ_Boolean isActive(void) {
    Suspendable * susp = (Suspendable *) _p;
    return (_p == (Propagator *) NULL) || susp->isActive();
  }
};

//-----------------------------------------------------------------------------
// prototypes of reflection functions

OZ_Term reflect_propagator(Suspendable *);
OZ_Term reflect_thread(Suspendable *);
OZ_Term reflect_susplist(SuspList *);
OZ_Term reflect_variable(OZ_Term);
OZ_Term reflect_space(OZ_Term);

//-----------------------------------------------------------------------------
// prototypes of built-ins

OZ_BI_proto(BIReflectPropagator);
OZ_BI_proto(BIReflectPropagatorName);
OZ_BI_proto(BIIsPropagatorFailed);
OZ_BI_proto(BIReflectPropagatorCoordinates);
OZ_BI_proto(BIReflectVariable);
OZ_BI_proto(BIPropagatorEq);
OZ_BI_proto(BIReflectSpace);
OZ_BI_proto(BIIsPropagator);
OZ_BI_proto(BIIsDiscardedPropagator);
OZ_BI_proto(BIDiscardPropagator);
OZ_BI_proto(BIIdentifyParameter);
OZ_BI_proto(BIIsActivePropagator);
OZ_BI_proto(BIDeactivatePropagator);
OZ_BI_proto(BIActivatePropagator);

//-----------------------------------------------------------------------------

inline
OZ_Term propagator2Term(Propagator * p) {
  return makeTaggedExtension(new PropagatorReference(p));
}

OZ_Term prop_name(char * name);

//-----------------------------------------------------------------------------

#define MKARITY(Arity, ArityDef)                        \
OZ_Term Arity = OZ_nil();                               \
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)     \
  Arity = OZ_cons(ArityDef[i], Arity);

//-----------------------------------------------------------------------------

extern
OZ_Term atom_var, atom_any, atom_type, atom_fd, atom_fs, atom_bool,
  atom_bounds, atom_val, atom_glb, atom_lub, atom_oops, atom_prop,
  atom_params, atom_name, atom_susp, atom_thread, atom_ct,
  atom_susplists, atom_ref, atom_id, atom_loc, atom_vars, atom_props,
  atom_reflect, atom_reflect_vartable, atom_reflect_proptable, atom_nonevar;

//-----------------------------------------------------------------------------

class VarListExpect;

typedef OZ_expect_t (VarListExpect::*PropagatorExpectMeth) (OZ_Term);

class VarListExpect : public ExpectOnly {
public:
  OZ_expect_t expectList(OZ_Term t, PropagatorExpectMeth expectf) {
    return (OZ_isNil(t) 
	    ? OZ_expect_t(1, 1) 
	    : OZ_Expect::expectList(t, (OZ_ExpectMeth) expectf));
  }
  OZ_expect_t expectAny(OZ_Term) {
    return OZ_expect_t(1, 1);
  }
  OZ_expect_t expectListVar(OZ_Term t) {
    return expectList(t, &VarListExpect::expectAny);
  }
};

#endif /* __REFLECT__HH__ */
