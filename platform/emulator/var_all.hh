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

#include "genvar.hh"

#include "fsgenvar.hh"
#include "fdgenvar.hh"
#include "fdbvar.hh"
#include "ofgenvar.hh"
#include "ctgenvar.hh"
#include "perdiovar.hh"
#include "simplevar.hh"
#include "future.hh"
#include "extvar.hh"

inline
// mm2: should be OZ_Return
Bool oz_cv_validINLINE(GenCVariable *cv,TaggedRef *ptr,TaggedRef val)
{
  switch (cv->getType()){
  case OZ_VAR_SIMPLE:   return ((SimpleVar *) cv)->valid(val);
  case OZ_VAR_FUTURE:   return ((Future *) cv)->valid(val);
  case PerdioVariable:  return ((PerdioVar *) cv)->valid(val);
  case BoolVariable:    return ((GenBoolVariable*) cv)->valid(val);
  case FDVariable:      return ((GenFDVariable*) cv)->valid(val);
  case OFSVariable:     return ((GenOFSVariable*) cv)->valid(val);
  case FSetVariable:    return ((GenFSetVariable*) cv)->valid(val);
  case CtVariable:      return ((GenCtVariable*) cv)->valid(val);
  case OZ_VAR_EXTENTED: return ((ExtentedVar *) cv)->validV(val);
  default: error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_cv_unifyINLINE(GenCVariable *cv,TaggedRef *ptr,TaggedRef val,
			    ByteCode *scp)
{
  switch (cv->getType()){
  case OZ_VAR_SIMPLE:   return ((SimpleVar *) cv)->unify(ptr,val,scp);
  case OZ_VAR_FUTURE:   return ((Future *) cv)->unify(ptr,val,scp);
  case PerdioVariable:  return ((PerdioVar *) cv)->unify(ptr,val,scp);
  case BoolVariable:    return ((GenBoolVariable*) cv)->unify(ptr,val,scp);
  case FDVariable:      return ((GenFDVariable*) cv)->unify(ptr,val,scp);
  case OFSVariable:     return ((GenOFSVariable*) cv)->unify(ptr,val,scp);
  case FSetVariable:    return ((GenFSetVariable*) cv)->unify(ptr,val,scp);
  case CtVariable:      return ((GenCtVariable*) cv)->unify(ptr,val,scp);
  case OZ_VAR_EXTENTED: return ((ExtentedVar *) cv)->unifyV(ptr,val,scp);
  default:  error("not impl"); return FAILED;
  }
}

inline
OZ_Return oz_cv_bindINLINE(GenCVariable *cv,TaggedRef *ptr,TaggedRef val,
			   ByteCode *scp)
{
  switch (cv->getType()){
    /*
  case OZ_VAR_SIMPLE:   return ((SimpleVar *) cv)->bind(ptr,val,scp);
  case OZ_VAR_FUTURE:   return ((Future *) cv)->bind(ptr,val,scp);
  case PerdioVariable:  return ((PerdioVar *) cv)->bind(ptr,val,scp);
  case BoolVariable:    return ((GenBoolVariable*) cv)->bind(ptr,val,scp);
  case FDVariable:      return ((GenFDVariable*) cv)->bind(ptr,val,scp);
  case OFSVariable:     return ((GenOFSVariable*) cv)->bind(ptr,val,scp);
  case FSetVariable:    return ((GenFSetVariable*) cv)->bind(ptr,val,scp);
  case CtVariable:      return ((GenCtVariable*) cv)->bind(ptr,val,scp);
  case OZ_VAR_EXTENTED: return ((ExtentedVar *) cv)->bindV(ptr,val,scp);
    */
  default:  
    return oz_cv_unify(cv,ptr,val,scp);
  }
}

inline
void oz_cv_addSuspINLINE(GenCVariable *cv, TaggedRef *v, Suspension susp,
			 int unstable = TRUE)
{
  switch(cv->getType()) {
  case OZ_VAR_FUTURE:
    ((Future *) cv)->addSusp(v, susp, unstable); return;
  case PerdioVariable:
    ((PerdioVar *) cv)->addSusp(v, susp, unstable); return;
  case OZ_VAR_EXTENTED:
    ((ExtentedVar *) cv)->addSuspV(v, susp, unstable); return;
  default:
    cv->addSuspSVar(susp,unstable);
  }
}

#endif //__allgenvar_hh__
