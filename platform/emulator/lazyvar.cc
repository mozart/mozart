/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lazyvar.hh"
#endif

#include "lazyvar.hh"

// if `function' is a procedure or an object, we simply call it
// with the variable itself as argument.
//
// for extension, `function' may also be a tuple, where typically
// the first element is a small integer that determines the
// operational interpretation:
//
//      1#P     ==> thread {P ME} end
//      2#X     ==> force request of X
//      3#URL   ==> thread {Load URL ME} end
//      4#call(P X1 ... Xn) ==> thread {P X1 ... Xn ME} end

void
GenLazyVariable::kickLazy()
{
  if (function!=0) {
    if (OZ_isProcedure(function)||OZ_isObject(function))
      {
        Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
        thr->pushCall(function,result);
        am.scheduleThread(thr);
      }
    else if (OZ_isCons(function) ||
             (OZ_isTuple(function)&&OZ_width(function)>1))
      {
        OZ_Term fst = OZ_getArg(function,0);
        OZ_Term snd = OZ_getArg(function,1);
        if (OZ_isSmallInt(fst))
          switch (OZ_intToC(fst)) {
          case 1:
            // 1#P ==> thread {P ME} end
            {
              Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
              thr->pushCall(snd,result);
              am.scheduleThread(thr);
              break;
            }
          case 2:
            // 2#X ==> force request of X
            snd = OZ_deref(snd);
            if (isLazyVar(snd))
              tagged2LazyVar(snd)->kickLazy();
            break;
          case 3:
            // 3#URL ==> thread {Load URL ME} end
            {
              Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
              thr->pushCall(BI_load,snd,result);
              am.scheduleThread(thr);
              break;
            }
          case 4:
            // 4#call(P X1 ... Xn) ==> thread {P X1 ... Xn ME} end
            if (OZ_isTuple(snd)) {
              Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
              int w = OZ_width(snd);
              RefsArray args = allocateRefsArray(w,NO);
              for(int i=1;i<w;i++) args[i-1]=OZ_getArg(snd,i);
              args[w-1] = result;
              thr->pushCall(OZ_getArg(snd,0),args,w);
              disposeRefsArray(args);
              am.scheduleThread(thr);
              break;
            }
          default:
            goto illegal;
          }
        else goto illegal;
      }
    else goto illegal;
    function=0;
  }
  return;
  illegal:
  function=0;
  OZ_warning("Lazy variable contains illegal spec");
}

OZ_Return
GenLazyVariable::unifyV(TaggedRef *vPtr,TaggedRef v,
                        TaggedRef *tPtr,TaggedRef t,
                        ByteCode*scp)
{
  // if x:lazy=y:var y<-x if x is global, then trail
  // ^^^DONE AUTOMATICALLY
  // else x.kick() x=y

  kickLazy();

  oz_bind(vPtr,*vPtr,oz_isVariable(t)?makeTaggedRef(tPtr):t);
  return PROCEED;
}

void
GenLazyVariable::addSuspV(Suspension susp, TaggedRef *tPtr, int unstable)
{
  kickLazy();
  addSuspSVar(susp, unstable);
}

OZ_BI_define(BILazyNew,2,0)
{
  OZ_Term oz_fun = OZ_in(0);
  OZ_Term oz_res = OZ_in(1);
  if (!OZ_isProcedure(oz_fun) &&
      !OZ_isObject(oz_fun)    &&
      !OZ_isTuple(oz_fun))
    return OZ_typeError(0,"Unary Procedure|Object|Tuple");
  GenLazyVariable *lazy = new GenLazyVariable(oz_fun,oz_res);
  return oz_unify(oz_res,(OZ_Term)newTaggedCVar((GenCVariable*)lazy));
} OZ_BI_end

OZ_BI_define(BILazyIs,1,1)
{
  OZ_declareIN(0,var);
  OZ_RETURN(isLazyVar(oz_deref(var))?OZ_true():OZ_false());
} OZ_BI_end
