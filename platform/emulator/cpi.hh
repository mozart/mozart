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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
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
#include "cpi_heap.hh"

#if defined(OUTLINE)
#define inline
#endif

#define CPIINITSIZE 1000

struct _spawnVars_t {
  OZ_Term * var;
  TypeOfGenCVariable expected_type;
  union {OZ_FDPropState fd; OZ_FSetPropState fs;} state;
};

extern EnlargeableArray<_spawnVars_t> staticSpawnVars;
extern EnlargeableArray<_spawnVars_t> staticSpawnVarsProp;
extern EnlargeableArray<_spawnVars_t> staticSuspendVars;

extern int staticSpawnVarsNumber;
extern int staticSpawnVarsNumberProp;
extern int staticSuspendVarsNumber;


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

#if !defined(OUTLINE)
#include "cpi.icc"
#else
#undef inline
#endif

OZ_Return constraintsSuspendOnVar(OZ_CFun, int, OZ_Term *, OZ_Term *);

#endif /* __CPI__H__ */

// End of File
//-----------------------------------------------------------------------------
