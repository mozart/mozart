/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    derived from lazyvar.cc by Denys Duchier (duchier@ps.uni-sb.de)
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
#pragma implementation "future.hh"
#endif

#include "future.hh"
#include "builtins.hh"
#include "threadInterface.hh"

inline
Bool isFuture(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_FUTURE);
}

inline
Future *tagged2Future(TaggedRef t) {
  Assert(isFuture(t));
  return (Future *) tagged2CVar(t);
}

// bind a future, don't care about the variable, e.g. for byNeed
void oz_bindFuture(OZ_Term fut,OZ_Term val)
{
  DEREF(fut,vPtr,_);
  Assert(isFuture(fut));
  oz_bind(vPtr,val);
}

// this builtin is only internally available
OZ_BI_define(BIbyNeedAssign,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr,_);
  OZ_Term val = OZ_in(1);

  Assert(isFuture(var));
  oz_bind(varPtr,val);
  return PROCEED;
} OZ_BI_end

/* call `function' as
 *    thread Tmp in {function Tmp} {`Assign` Future Tmp} end
 */
void Future::kick(TaggedRef *ptr)
{
  Assert(function!=0);
  Board* bb      = GETBOARD(this);
  Thread* thr    = oz_newThreadInject(DEFAULT_PRIORITY,bb);
  OZ_Term newvar = oz_newVar(bb);

  static RefsArray args = allocateStaticRefsArray(2);
  args[0]=makeTaggedRef(ptr);
  args[1]=newvar;

  thr->pushCFun(BIbyNeedAssign, args, 2);
  thr->pushCall(function,newvar);
  function=0;
}

OZ_Return Future::unify(TaggedRef *vPtr, TaggedRef t, ByteCode*scp)
{
  if (function) kick(vPtr);

  oz_suspendOnPtr(vPtr);
  return SUSPEND;
}

void Future::addSusp(TaggedRef *tPtr, Suspension susp, int unstable)
{
  if (function) kick(tPtr);

  addSuspSVar(susp, unstable);
}

void Future::printStream(ostream &out,int depth = 10)
{
  if (function) {
      out << "<future byNeed: ";
      oz_printStream(function,out,depth-1);
      out << ">";
  } else {
    out << "<future>";
  }
}

OZ_Term Future::inspect()
{
  OZ_Term k=function ? OZ_mkTupleC("byNeed",1,function) : oz_atom("simple");
  return OZ_mkTupleC("future", 1, k);
}

// this builtin/propagator is only internally available
OZ_BI_define(VarToFuture,2,0)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    oz_suspendOnVar(v);
  }
  OZ_Term f = OZ_in(1);
  DEREF(f,fPtr,_);
  Assert(isFuture(f));
  oz_bind(fPtr,v);
  return PROCEED;
} OZ_BI_end

OZ_Term oz_newFuture(Board *bb) {
  return makeTaggedRef(newTaggedCVar(new Future(bb)));
}

OZ_BI_define(BIfuture,1,1)
{
  OZ_Term v = OZ_in(0);
  v = oz_safeDeref(v);
  if (oz_isRef(v)) {
    OZ_Term f = oz_newFuture(oz_currentBoard());
    RefsArray args = allocateRefsArray(2, NO);
    args[0]=v;
    args[1]=f;
    Thread *thr = oz_newThreadSuspended();
    thr->pushCFun(VarToFuture,args,2);
    addSuspAnyVar(tagged2Ref(v),thr);
    OZ_RETURN(f);
  }
  OZ_RETURN(v);
} OZ_BI_end

OZ_BI_define(BIwaitQuiet,1,0)
{
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIbyNeed,1,1)
{
  oz_declareNonvarIN(0,p);
  if (!oz_isProcedure(p) || oz_procedureArity(p)!=1) {
    oz_typeError(0,"Unary Procedure");
  }
  OZ_RETURN(makeTaggedRef(newTaggedCVar(new Future(p,oz_currentBoard()))));
} OZ_BI_end

extern
OZ_Return portSend(Tertiary *p, TaggedRef msg);

// PORTS with Futures

OZ_BI_define(BInewPortF,0,2)
{
  OZ_Term fut = oz_newFuture(oz_currentBoard());
  OZ_Term port = oz_newPort(fut);

  OZ_out(0)= fut;
  OZ_out(1)= port;
  return PROCEED;
} OZ_BI_end

OZ_Return sendPortF(OZ_Term prt, OZ_Term val)
{
  Assert(oz_isPort(prt));

  Port *port  = tagged2Port(prt);

  CheckLocalBoard(port,"port");

  if(port->isProxy()) {
    return portSend(port,val);
  }
  OZ_Term newFut = oz_newFuture(oz_currentBoard());
  OZ_Term lt  = oz_cons(am.currentUVarPrototype(),newFut);
  OZ_Term oldFut = ((PortWithStream*)port)->exchangeStream(newFut);

  oz_bindFuture(oldFut,lt);
  OZ_unifyInThread(val,oz_head(lt)); // might raise exception if val is non exportable

  return PROCEED;
}

OZ_BI_define(BIsendPortF,2,0)
{
  oz_declareNonvarIN(0,prt);
  oz_declareIN(1,val);

  if (!oz_isPort(prt)) {
    oz_typeError(0,"Port");
  }

  return sendPortF(prt,val);
} OZ_BI_end
