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

CpiHeapClass CpiHeap;

EnlargeableArray<_spawnVars_t> staticSpawnVars(MAXFDBIARGS);
EnlargeableArray<_spawnVars_t> staticSpawnVarsProp(MAXFDBIARGS);
EnlargeableArray<OZ_Term *> staticSuspendVars(MAXFDBIARGS);

int staticSpawnVarsNumber = 0;
int staticSpawnVarsNumberProp = 0;
int staticSuspendVarsNumber = 0;

#if defined(OUTLINE)
#define inline
#include "cpi.icc"
#undef inline
#endif


// End of File
//-----------------------------------------------------------------------------
