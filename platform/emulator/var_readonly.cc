/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    derived from var_future.cc by Raphael Collet <raph@info.ucl.ac.be>
 *    and Alfred Spiessens <fsp@info.ucl.ac.be>
 * 
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_readonly.hh"
#endif

#include "var_readonly.hh"
#include "dpInterface.hh"
#include "builtins.hh"
#include "unify.hh"
#include "thr_int.hh"
#include "value.hh"
#include "atoms.hh"

// this builtin binds a ReadOnly
// it is used by the definition of '!!' only
OZ_BI_define(BIbindReadOnly,2,0)
{
  // OZ_Term var = OZ_in(0);
  // DEREF(var,varPtr);
  oz_declareDerefIN(0, var);
  // MISSING: check the type of the variable !!!

  // oz_declareNonvarIN(1, val);
  OZ_Term val = OZ_in(1);

  oz_bindReadOnly(varPtr,val);
  return PROCEED;
} OZ_BI_end

OZ_Return ReadOnly::bind(TaggedRef *vPtr, TaggedRef t)
{
  if (oz_isLocalVar(this)) {
    return am.addSuspendVarListInline(vPtr);
  } else {
    oz_bindVar(this,vPtr, t);
    return PROCEED;
  }
}

OZ_Return ReadOnly::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  if (*vPtr != oz_deref(t))
    oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return ReadOnly::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr,makeTaggedRef(tPtr));
}

void ReadOnly::printStream(ostream &out,int depth)
{
  out << "<readonly>";
}

// this builtin/propagator is only internally available
/*
OZ_BI_define(BIvarToReadOnly,2,0)
{
  oz_declareDerefIN(0,v);
  if (oz_isVarOrRef(v)) {
    if (oz_isReadOnly(v)) {
      if (((ReadOnly*)tagged2Var(v))->isFailed()) {
	v = makeTaggedRef(vPtr);
	goto bind_fut;
      }
      else {
	((ReadOnly*)tagged2Var(v))->addSuspSVar(oz_currentThread());
	return SUSPEND;
      }
    }
    else {
      oz_suspendOnPtr(vPtr);
    }
  }
 bind_fut:
  oz_declareDerefIN(1,f);
  oz_bindReadOnly(fPtr,v);
  return PROCEED;
} OZ_BI_end
*/

