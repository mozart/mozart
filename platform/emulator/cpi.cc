/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE)
#pragma implementation "cpi.hh"
#endif

#include "cpi.hh"

char * ctHeap, * ctHeapTop = new char[CPIINITSIZE * 100];

#ifdef GAGA
EnlargeableArray<_spawnVars_t> staticSpawnVars(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSpawnVarsProp(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSuspendVars(CPIINITSIZE);
#else
_spawnVars_t * staticSpawnVars = new _spawnVars_t[CPIINITSIZE];
_spawnVars_t * staticSpawnVarsProp = new _spawnVars_t[CPIINITSIZE];
_spawnVars_t * staticSuspendVars = new _spawnVars_t[CPIINITSIZE];
#endif

int staticSpawnVarsNumber = 0;
int staticSpawnVarsNumberProp = 0;
int staticSuspendVarsNumber = 0;


#ifdef FDBISTUCK

OZ_Return constraintsSuspendOnVar(OZ_CFun, int, OZ_Term *,
                                  OZ_Term * t)
{
  OZ_suspendOn(makeTaggedRef(t));
}

#else

OZ_Return constraintsSuspendOnVar(OZ_CFun f, int a, OZ_Term * x,
                                  OZ_Term * t)
{
  OZ_addThread(makeTaggedRef(t), OZ_makeSuspendedThread(f, x, a));
  return PROCEED;
}

#endif

#if defined(OUTLINE)
#define inline
#include "cpi.icc"
#undef inline
#endif


// End of File
//-----------------------------------------------------------------------------
