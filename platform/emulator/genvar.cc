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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "genvar.hh"
#endif

#include "genvar.hh"
#include "allgenvar.hh"

Bool oz_cv_valid(GenCVariable *cv,TaggedRef *ptr,TaggedRef val) {
  return oz_cv_validINLINE(cv,ptr,val);
}

OZ_Return oz_cv_unify(GenCVariable *cv,TaggedRef *ptr,TaggedRef val,
                      ByteCode *scp) {
  return oz_cv_unifyINLINE(cv,ptr,val,scp);
}

OZ_Return oz_cv_bind(GenCVariable *cv,TaggedRef *ptr,TaggedRef val,
                      ByteCode *scp) {
  return oz_cv_bindINLINE(cv,ptr,val,scp);
}

void oz_cv_addSusp(GenCVariable *cv, TaggedRef *v, Suspension susp,
                   int unstable = TRUE) {
  oz_cv_addSuspINLINE(cv, v, susp, unstable);
}

void oz_cv_printStream(ostream &out, const char *s, GenCVariable *cv,
                       int depth)
{
  switch (cv->getType()) {
  case OZ_VAR_SIMPLE:
    out << s;
    ((SimpleVar *)cv)->printStream(out,depth); return;
  case OZ_VAR_FUTURE:
    out << s;
    ((Future *)cv)->printStream(out,depth); return;
  case PerdioVariable:
    out << s;
    ((PerdioVar *)cv)->printStream(out,depth); return;
  case BoolVariable:
    out << s;
    ((GenBoolVariable*)cv)->printStream(out,depth); return;
  case FDVariable:
    out << s;
    ((GenFDVariable*)cv)->printStream(out,depth); return;
  case OFSVariable:
    ((GenOFSVariable*)cv)->printStream(out,depth); return;
  case FSetVariable:
    out << s;
    ((GenFSetVariable*)cv)->printStream(out,depth); return;
  case CtVariable:
    out << s;
    ((GenCtVariable*)cv)->printStream(out,depth); return;
  case OZ_VAR_EXTENTED:
    out << s;
    ((ExtentedVar *)cv)->printStreamV(out,depth); return;
  default:
    error("not impl"); return;
  }
}

int oz_cv_getSuspListLength(GenCVariable *cv)
{
  Assert(cv->getType()!=OZ_VAR_INVALID);

  switch (cv->getType()){
  case BoolVariable:    return ((GenBoolVariable*)cv)->getSuspListLength();
  case FDVariable:      return ((GenFDVariable*)cv)->getSuspListLength();
  case OFSVariable:     return ((GenOFSVariable*)cv)->getSuspListLength();
  case FSetVariable:    return ((GenFSetVariable*)cv)->getSuspListLength();
  case OZ_VAR_EXTENTED: return ((ExtentedVar *)cv)->getSuspListLengthV();
  default:              return cv->getSuspListLengthS();
  }
}
