/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "am.hh"
#include "genvar.hh"

//-----------------------------------------------------------------------------
// Introduce FD Built-ins to the Emulator

OZ_C_proc_begin(BImkFSetValue, 2)
{
  return OZ_unify(OZ_getCArg(1),
                  makeTaggedFSetValue(new FSetValue(OZ_getCArg(0))));
}
OZ_C_proc_end

OZ_C_proc_begin(BImkFSetVar, 4)
{
  OZ_FSetImpl fset(OZ_intToC(OZ_getCArg(0)), OZ_getCArg(1), OZ_getCArg(2));

  return OZ_unify(OZ_getCArg(3), makeTaggedRef(newTaggedCVar(new GenFSetVariable(fset))));
}
OZ_C_proc_end

static
BIspec fdSpec[] = {

// fsetcore.cc
  {"mkFSetValue", 2, BImkFSetValue},
  {"mkFSetVar", 4, BImkFSetVar},

  {0,0,0,0}
};

void BIinitFSet(void)
{
  BIaddSpec(fdSpec);
}
