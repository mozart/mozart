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

inline
// mm2: should be OZ_Return
Bool oz_cv_validINLINE(OzVariable *cv,TaggedRef *ptr,TaggedRef val)
{
  switch (cv->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) cv)->valid(val);
  case OZ_VAR_FUTURE:  return ((Future *) cv)->valid(val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) cv)->valid(val);
  case OZ_VAR_FD:      return ((OzFDVariable*) cv)->valid(val);
  case OZ_VAR_OF:      return ((OzOFVariable*) cv)->valid(val);
  case OZ_VAR_FS:      return ((OzFSVariable*) cv)->valid(val);
  case OZ_VAR_CT:      return ((OzCtVariable*) cv)->valid(val);
  case OZ_VAR_EXT:     return ((ExtVar *) cv)->validV(val);
  default: error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_cv_unifyINLINE(OzVariable *cv,TaggedRef *ptr,TaggedRef val,
			    ByteCode *scp)
{
  switch (cv->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) cv)->unify(ptr,val,scp);
  case OZ_VAR_FUTURE:  return ((Future *) cv)->unify(ptr,val,scp);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_FD:      return ((OzFDVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_OF:      return ((OzOFVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_FS:      return ((OzFSVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_CT:      return ((OzCtVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_EXT:     return ((ExtVar *) cv)->unifyV(ptr,val,scp);
  default:  error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_cv_bindINLINE(OzVariable *cv,TaggedRef *ptr,TaggedRef val,
			   ByteCode *scp)
{
  switch (cv->getType()){
    /*
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) cv)->bind(ptr,val,scp);
  case OZ_VAR_FUTURE:  return ((Future *) cv)->bind(ptr,val,scp);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_FD:      return ((OzFDVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_OF:      return ((OzOFVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_FS:      return ((OzFSVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_CT:      return ((OzCtVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_EXT:     return ((ExtVar *) cv)->bindV(ptr,val,scp);
    */
  default:  
    return oz_cv_unify(cv,ptr,val,scp);
  }
}

inline
void oz_cv_addSuspINLINE(OzVariable *cv, TaggedRef *v, Suspension susp,
			 int unstable = TRUE)
{
  switch(cv->getType()) {
  case OZ_VAR_FUTURE:
    ((Future *) cv)->addSusp(v, susp, unstable); return;
  case OZ_VAR_EXT:
    ((ExtVar *) cv)->addSuspV(v, susp, unstable); return;
  default:
    cv->addSuspSVar(susp,unstable);
  }
}

#endif //__allgenvar_hh__
