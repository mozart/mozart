/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller

  ------------------------------------------------------------------------
*/

#ifndef __CPI__H__
#define __CPI__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include <stdarg.h>
#include "oz_cpi.hh"
#include "runtime.hh"
#include "genvar.hh"

#if defined(OUTLINE)
#define inline
#endif

struct _spawnVars_t {
  OZ_Term * var;
  TypeOfGenCVariable expected_type;
  union {OZ_FDPropState fd; OZ_FSetPropState fs;} state;
};

extern _spawnVars_t * staticSpawnVars;
extern _spawnVars_t * staticSpawnVarsProp;
extern _spawnVars_t * staticSuspendVars;
extern int staticSpawnVarsNumber;
extern int staticSpawnVarsNumberProp;
extern int staticSuspendVarsNumber;

extern char * ctHeap, * ctHeapTop;


class ExpectOnly : public OZ_Expect {
private:
  OZ_Return spawn(OZ_Propagator *, int , OZ_PropagatorFlags ) { return PROCEED;}
  OZ_Return spawn(OZ_Propagator *, OZ_PropagatorFlags) { return PROCEED;}
  OZ_Return suspend(OZ_Thread) { return PROCEED; }
  OZ_Return fail(void) { return PROCEED; }
public:
  ExpectOnly(void) : OZ_Expect() {}
  OZ_Term * getSuspVar(void) {
    if (staticSuspendVarsNumber == 0)
      return NULL;
    return staticSuspendVars[--staticSuspendVarsNumber].var;
  }
};

OZ_Boolean isPosSmallInt(OZ_Term v);
OZ_Boolean isPosSmallFDInt(OZ_Term v);
OZ_Boolean isPosSmallSetInt(OZ_Term v);

#define MAXFDBIARGS 1000 // maximum number of arguments of fd built-ins

#define EXPECT_BLOCK(O, P, F)                                           \
{                                                                       \
  OZ_expect_t r = O.F(OZ_getCArg(P));                                   \
  if (O.isFailing(r)) {                                                 \
    TypeError(P, "");                                                   \
  } else if (O.isSuspending(r)) {                                       \
    for (OZ_Term * v = O.getSuspVar(); v != NULL; v = O.getSuspVar())   \
      am.addSuspendVarList(v);                                          \
    return SUSPEND;                                                     \
  }                                                                     \
}

void staticAddSpawnProp(OZ_FDPropState ps, OZ_Term * v);
void staticAddSpawn(OZ_FDPropState ps, OZ_Term * v);

void staticAddSpawnProp(OZ_FSetPropState ps, OZ_Term * v);
void staticAddSpawn(OZ_FSetPropState ps, OZ_Term * v);

void * heap_new(size_t s);

void heap_delete(void *, size_t);

#if !defined(OUTLINE)
#include "cpi.icc"
#else
#undef inline
#endif

OZ_Return constraintsSuspendOnVar(OZ_CFun, int, OZ_Term *, OZ_Term *);

#endif /* __CPI__H__ */

// End of File
//-----------------------------------------------------------------------------
