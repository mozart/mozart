/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

//  internal interface to AMOZ

#include <stdarg.h>
#include "runtime.hh"

// mm2: suspend/resume has to be overhauled
void oz_suspendOnNet(Thread *th)
{
  if (th->pStop()==0) {
    if (th == am.currentThread()) {
      am.setSFlag(StopThread);
    }
    if (th->isRunnable())
      th->unmarkRunnable();
  }
}

void oz_resumeFromNet(Thread *th)
{
  if (th->pCont()==0) {
    if (!th->isDeadThread()) {
      if (th == am.currentThread()) {
	Assert(am.isSetSFlag(StopThread));
	am.unsetSFlag(StopThread);
      } else {
	am.suspThreadToRunnable(th);
	if (!am.isScheduledSlow(th))
	  am.scheduleThread(th);
      }
    }
  }
}

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...)
{
  OZ_Term exc=OZ_tuple(key,arity+1);
  OZ_putArg(exc,0,OZ_atom(label));

  va_list ap;
  va_start(ap,arity);

  for (int i = 0; i < arity; i++) {
    OZ_putArg(exc,i+1,va_arg(ap,OZ_Term));
  }

  va_end(ap);


  OZ_Term ret = OZ_record(cat,
			  cons(OZ_int(1),
			       cons(OZ_atom("debug"),OZ_nil())));
  OZ_putSubtree(ret,OZ_int(1),exc);
  OZ_putSubtree(ret,OZ_atom("debug"),NameUnit);

  am.setException(ret,NameUnit,OZ_eq(cat,E_ERROR) ? TRUE : ozconf.errorDebug);
  return RAISE;
}

OZ_Term oz_getLocation(Board *bb)
{
  OZ_Term out = nil();
  while (!am.isRootBoard(bb)) {
    if (bb->isSolve()) {
      out = cons(OZ_atom("space"),out);
    } else if (bb->isAsk()) {
      out = cons(OZ_atom("cond"),out);
    } else if (bb->isWait()) {
      out = cons(OZ_atom("dis"),out);
    } else {
      out = cons(OZ_atom("???"),out);
    }
    bb=bb->getParent();
  }
  return out;
}

Board *ozx_rootBoard() { return am._rootBoard; }
