/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __allgenvar_hh__
#define __allgenvar_hh__

#include "var_base.hh"

#include "var_fs.hh"
#include "var_fd.hh"
#include "var_bool.hh"
#include "var_of.hh"
#include "var_ct.hh"
#include "var_simple.hh"
#include "var_future.hh"
#include "var_ext.hh"
#include "dpInterface.hh"

// import from builtins
int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...);

inline
// mm2: should be OZ_Return
Bool oz_var_validINLINE(OzVariable *ov,TaggedRef *ptr,TaggedRef val)
{
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->valid(val);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->valid(val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->valid(val);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->valid(val);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->valid(val);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->valid(val);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->valid(val);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->validV(val);
  default: OZ_error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_var_unifyINLINE(OzVariable *ov,TaggedRef *ptr,TaggedRef *val,
			     ByteCode *scp)
{
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->unify(ptr,val,scp);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->unify(ptr,val,scp);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->unify(ptr,val,scp);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->unify(ptr,val,scp);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->unify(ptr,val,scp);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->unify(ptr,val,scp);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->unify(ptr,val,scp);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->unifyV(ptr,val);
  default:  OZ_error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_var_bindINLINE(OzVariable *ov,TaggedRef *ptr,TaggedRef val,
			    ByteCode *scp)
{
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->bind(ptr,val,scp);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->bind(ptr,val,scp);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->bindV(ptr,val);
  default:  OZ_error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_var_forceBindINLINE(OzVariable *ov,TaggedRef *ptr,TaggedRef val,
			    ByteCode *scp)
{
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->bind(ptr,val,scp);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->forceBind(ptr,val,scp);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->bind(ptr,val,scp);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->forceBindV(ptr,val);
  default:  OZ_error("not impl"); return FAILED;
  }
}

inline
void oz_var_disposeINLINE(OzVariable *ov)
{
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  ((SimpleVar *) ov)->dispose(); break;
  case OZ_VAR_FUTURE:  ((Future *) ov)->dispose(); break;
  case OZ_VAR_BOOL:    ((OzBoolVariable*) ov)->dispose(); break;
  case OZ_VAR_FD:      ((OzFDVariable*) ov)->dispose(); break;
  case OZ_VAR_OF:      ((OzOFVariable*) ov)->dispose(); break;
  case OZ_VAR_FS:      ((OzFSVariable*) ov)->dispose(); break;
  case OZ_VAR_CT:      ((OzCtVariable*) ov)->dispose(); break;
  case OZ_VAR_EXT:     ((ExtVar *) ov)->disposeV(); break;
  default:  OZ_error("not impl");
  }
}

inline
Bool oz_var_addSuspINLINE(TaggedRef *v, Suspension susp, int unstable = TRUE)
{
  OzVariable *ov=oz_getVar(v);
  switch(ov->getType()) {
  case OZ_VAR_FUTURE:
    return ((Future *) ov)->addSusp(v, susp, unstable);
  case OZ_VAR_EXT:
    return ((ExtVar *) ov)->addSuspV(v, susp, unstable);
  case OZ_VAR_SIMPLE:
    if (ozconf.useFutures) {
      return oz_raise(E_ERROR,E_KERNEL,"suspendNoFuture",
		      1,makeTaggedRef(v));
    }
    // fall through
  default:
    ov->addSuspSVar(susp,unstable);
    return FALSE;
  }
}

#endif //__allgenvar_hh__
