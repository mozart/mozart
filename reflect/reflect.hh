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
// prototypes of reflection functions

OZ_Term reflect_propagator(Suspension);
OZ_Term reflect_thread(Suspension);
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

//-----------------------------------------------------------------------------

inline
OZ_Term propagator2Term(Propagator * p) {
  return oz_makeTaggedExtension(new PropagatorReference(p));
}

OZ_Term prop_name(char * name);

//-----------------------------------------------------------------------------

#define MKARITY(Arity, ArityDef)			\
OZ_Term Arity = OZ_nil();				\
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)	\
  Arity = OZ_cons(ArityDef[i], Arity);

//-----------------------------------------------------------------------------

extern 
OZ_Term atom_var, atom_any, atom_type, atom_fd, atom_fs, atom_bool,  
  atom_bounds, atom_val, atom_glb, atom_lub, atom_oops, atom_prop, 
  atom_params, atom_name, atom_susp, atom_thread, atom_ct, 
  atom_susplists, atom_ref, atom_id, atom_loc, atom_vars, atom_props, 
  atom_reflect, atom_reflect_vartable, atom_reflect_proptable;
  
//-----------------------------------------------------------------------------

#ifdef DEBUG

#define DEBUG_ASSERT(Cond)						\
  if (! (Cond)) {							\
    OZ_error("%s:%d assertion '%s' failed",__FILE__,__LINE__,#Cond);	\
  }

#define DEBUGPRINT(A)				\
printf("(%s:%d) ", __FILE__, __LINE__); 	\
printf A;					\
printf("\n");					\
fflush(stdout)

#else
#define DEBUGPRINT(A)
#define DEBUG_ASSERT(EXPR)
#endif

#endif /* __REFLECT__HH__ */
