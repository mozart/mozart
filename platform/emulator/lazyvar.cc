#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lazyvar.hh"
#endif

#include "am.hh"
#include "genvar.hh"
#include "runtime.hh"

void
GenLazyVariable::kickLazy()
{
  if (function!=0) {
    Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
    thr->pushCall(function,&result,1);
    am.scheduleThread(thr);
    function=0;
  }
}

void
GenLazyVariable::kickLazy(TaggedRef*me)
{
  kickLazy();
  // create a free var in the same home space
  // and bind this to the free var
  SuspList* suspl = suspList;
  if (suspl!=NULL) {
    SVariable* sv = new SVariable(home);
    sv->setSuspList(suspl);
    *me = makeTaggedSVar(sv);
  } else {
    *me = makeTaggedUVar(home);
  }
}

int
GenLazyVariable::unifyLazy(TaggedRef*vPtr,TaggedRef*tPtr,ByteCode*scp)
{
  // if x:lazy=y:var y<-x if x is global, then trail
  // ^^^DONE AUTOMATICALLY
  // else x.kick() x=y
  kickLazy(vPtr);
  Bool ret = am.performUnify(vPtr,tPtr,scp);
  Assert(am.rebindTrail.isEmpty());
  return ret;
}

void
GenLazyVariable::addSuspLazy(Thread*th)
{
  kickLazy();
  addSuspSVar(th);
}

OZ_C_proc_begin(BILazyNew,2)
{
  OZ_Term oz_fun = OZ_getCArg(0);
  OZ_Term oz_res = OZ_getCArg(1);
  if (!OZ_isProcedure(oz_fun))
    return OZ_typeError(0,"Unary Procedure");
  GenLazyVariable *lazy = new GenLazyVariable(oz_fun,oz_res);
  return OZ_unify(oz_res,(OZ_Term)newTaggedCVar((GenCVariable*)lazy));
}
OZ_C_proc_end

static BIspec lazySpecs[] = {
  {"Lazy.new", 2, BILazyNew, 0},
  {0,0,0,0},
};

void BIinitLazy()
{
  BIaddSpec(lazySpecs);
}
