#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lazyvar.hh"
#endif

#include "am.hh"
#include "genvar.hh"
#include "runtime.hh"

// if `function' is a procedure or an object, we simply call it
// with the variable itself as argument.
//
// for extension, `function' may also be a tuple, where typically
// the first element is a small integer that determines the
// operational interpretation:
//
//	1#P	==> thread {P ME} end
//	2#X	==> force request of X
//	3#URL	==> thread {Load URL ME} end
//	4#call(P X1 ... Xn) ==> thread {P X1 ... Xn ME} end

OZ_C_proc_proto(BIload);

void
GenLazyVariable::kickLazy()
{
  static RefsArray args = allocateStaticRefsArray(20);
  if (function!=0) {
    if (OZ_isProcedure(function)||OZ_isObject(function))
      {
	Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
	args[0] = result;
	thr->pushCall(function,args,1);
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
	      args[0] = result;
	      thr->pushCall(snd,args,1);
	      am.scheduleThread(thr);
	      break;
	    }
	  case 2:
	    // 2#X ==> force request of X
	    snd = OZ_deref(snd);
	    if (tagTypeOf(snd)==CVAR &&
		tagged2CVar(snd)->getType()==LazyVariable)
	      ((GenLazyVariable*)tagged2CVar(snd))->kickLazy();
	    break;
	  case 3:
	    // 3#URL ==> thread {Load URL ME} end
	    {
	      Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
	      args[0] = snd;
	      args[1] = result;
	      thr->pushCFun(BIload,args,2,TRUE);
	      am.scheduleThread(thr);
	      break;
	    }
	  case 4:
	    // 4#call(P X1 ... Xn) ==> thread {P X1 ... Xn ME} end
	    if (OZ_isTuple(snd)) {
	      Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,home);
	      int w = OZ_width(snd);
	      RefsArray args2 = (w>20)?allocateRefsArray(w,NO):args;
	      for(int i=1;i<w;i++) args2[i-1]=OZ_getArg(snd,i);
	      args2[w-1] = result;
	      thr->pushCall(OZ_getArg(snd,0),args2,w);
	      if (w>20) disposeRefsArray(args2);
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
  // Assert(am.rebindTrail.isEmpty());
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
  if (!OZ_isProcedure(oz_fun) &&
      !OZ_isObject(oz_fun)    &&
      !OZ_isTuple(oz_fun))
    return OZ_typeError(0,"Unary Procedure|Object|Tuple");
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

