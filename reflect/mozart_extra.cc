// add core includes here

#include "mozart_extra.hh"

OzVarKind OZ_getVarKind(OZ_Term var)
{
  DEREF(var,varptr);
  OzVarKind result = OZ_VAR_OTHER;
  if      (oz_isFree(var)) result=OZ_VAR_FREE;
  else if (isGenFDVar(var)) result=OZ_VAR_FD;
  else if (isGenBoolVar(var)) result=OZ_VAR_BOOL;
  else if (isGenFSetVar(var)) result=OZ_VAR_FS;
  else if (isGenCtVar(var)) result=OZ_VAR_CT;
  return result;
}

  
