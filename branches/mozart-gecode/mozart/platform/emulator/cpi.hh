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

#ifndef __CPI__H__
#define __CPI__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include <stdarg.h>
#include "builtins.hh"
#include "var_base.hh"
#include "cpi_heap.hh"
#include "fdomn.hh"
#include "unify.hh"
#include "mozart_cpi.hh"

#if defined(OUTLINE)
#define inline
#endif

#define CPIINITSIZE 1000

struct _spawnVars_t {
  OZ_Term * var; 
  TypeOfVariable expected_type;
  union {
    OZ_FDPropState fd; 
    OZ_FSetPropState fs;
    struct {
      OZ_CtDefinition * def;
      OZ_CtWakeUp w;
    } ct;
  } state;
};

extern EnlargeableArray<_spawnVars_t> staticSpawnVars;
extern EnlargeableArray<_spawnVars_t> staticSpawnVarsProp;
extern EnlargeableArray<_spawnVars_t> staticSuspendVars;

extern int staticSpawnVarsNumber;
extern int staticSpawnVarsNumberProp;
extern int staticSuspendVarsNumber;


class ExpectOnly : public OZ_Expect {
public:
  ExpectOnly(void) : OZ_Expect() {}
  OZ_Term * getSuspVar(void) {
    if (staticSuspendVarsNumber == 0) 
      return NULL;
    return staticSuspendVars[--staticSuspendVarsNumber].var;
  }
};

OZ_Boolean isPosSmallInt(OZ_Term v);
OZ_Boolean isPosSmallBoolInt(OZ_Term v); 
OZ_Boolean isPosSmallFDInt(OZ_Term v); 
OZ_Boolean isPosSmallSetInt(OZ_Term v);

#define EXPECT_BLOCK(O, P, F, C)					\
{									\
  OZ_expect_t r = O.F(OZ_in(P));					\
  if (O.isFailing(r)) {							\
    TypeError(P, C);							\
  } else if (O.isSuspending(r) || O.isExceptional(r)) {			\
    for (OZ_Term * v = O.getSuspVar(); v != NULL; v = O.getSuspVar())	\
      (void) oz_addSuspendVarList(v);					\
    return SUSPEND;							\
  }									\
}

void staticAddSpawnPropBool(OZ_Term * v);
void staticAddSpawnBool(OZ_Term * v);

void staticAddSpawnProp(OZ_FDPropState ps, OZ_Term * v);
void staticAddSpawn(OZ_FDPropState ps, OZ_Term * v);

void staticAddSpawnProp(OZ_FSetPropState ps, OZ_Term * v);
void staticAddSpawn(OZ_FSetPropState ps, OZ_Term * v);

void staticAddSpawnProp(OZ_CtDefinition * def, OZ_CtWakeUp w, OZ_Term * v);
void staticAddSpawn(OZ_CtDefinition * def, 
		    OZ_CtWakeUp w, 
		    OZ_Term * v);


void initCPI(void);

#if !defined(OUTLINE) 
#include "cpi.icc"
#else
#undef inline
#endif

#endif /* __CPI__H__ */

// End of File
//-----------------------------------------------------------------------------

