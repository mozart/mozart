/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/



#ifdef FSETVAR

#include "am.hh"
#include "cpi.hh"

//-----------------------------------------------------------------------------
// Introduce FD Built-ins to the Emulator

#define OZ_EM_FSETVAL   "finite set of integers value"
#define OZ_EM_FSET      "finite set of integers"
#define OZ_EM_FSETDESCR "description of finite set of integers"

OZ_C_proc_begin(BIfsSetValue, 2) 
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETVAL);

  ExpectOnly pe;
  
  EXPECT_BLOCK(pe, 0, expectSetDescr);

  return OZ_unify(OZ_getCArg(1),
		  makeTaggedFSetValue(new FSetValue(OZ_getCArg(0))));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsSet, 3) 
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETDESCR "," OZ_EM_FSET);
  
  
  ExpectOnly pe;
  
  EXPECT_BLOCK(pe, 0, expectSetDescr);
  EXPECT_BLOCK(pe, 1, expectSetDescr);
  
  OZ_FSetImpl fset(OZ_getCArg(0), OZ_getCArg(1));

  if (! fset.isValidSet()) {
    TypeError(2, "Invalid set description");
    return FAILED;
  }

  return OZ_unify(OZ_getCArg(2), 
		  makeTaggedRef(newTaggedCVar(new GenFSetVariable(fset))));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsSup, 1) 
{
  return OZ_unify(OZ_getCArg(0), OZ_int(fset_sup));
}
OZ_C_proc_end


OZ_C_proc_begin(BImkFSetVar, 5) 
{
  OZ_FSetImpl fset(OZ_intToC(OZ_getCArg(0)), OZ_intToC(OZ_getCArg(1)), 
		   OZ_getCArg(2), OZ_getCArg(3));

  return OZ_unify(OZ_getCArg(4), makeTaggedRef(newTaggedCVar(new GenFSetVariable(fset))));
}
OZ_C_proc_end

static
BIspec fdSpec[] = {

// fsetcore.cc
  {"fsSetValue", 2, BIfsSetValue},
  {"fsSet", 3, BIfsSet},
  {"fsSup", 1, BIfsSup},
  {"mkFSetVar", 5, BImkFSetVar},

  {0,0,0,0}
};

void BIinitFSet(void)
{
  BIaddSpec(fdSpec);
}


#endif /* FSETVAR */
