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

#include "runtime.hh"

#include <stdarg.h>

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...)
{
  Assert(!oz_isRef(cat));
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
			       cons(AtomDebug,OZ_nil())));
  OZ_putSubtree(ret,OZ_int(1),exc);
  OZ_putSubtree(ret,AtomDebug,NameUnit);

  am.setException(ret,NameUnit,
		  literalEq(cat,E_ERROR) ? TRUE : ozconf.errorDebug);
  return RAISE;
}

OZ_Term oz_getLocation(Board *bb)
{
  OZ_Term out = nil();
  while (!oz_isRootBoard(bb)) {
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

/*===================================================================
 * BuiltinTab
 *=================================================================== */

BuiltinTab builtinTab(750);

Builtin *BIadd(const char *name,int inArity, int outArity, OZ_CFun funn, 
	       Bool native)
{
  Builtin *builtin = new Builtin(name,inArity,outArity,funn,native);

  builtinTab.htAdd(name,builtin);

  return builtin;
}

// add specification to builtin table
void BIaddSpec(BIspec *spec)
{
  for (int i=0; spec[i].name; i++) {
    BIadd(spec[i].name,spec[i].inArity,spec[i].outArity,spec[i].fun,
	  spec[i].native);
  }
}

/*===================================================================
 * type errors
 *=================================================================== */

static
char *getTypeOfPos(char * t, int p)
{
  static char buffer[100];
  int i, bi, comma;

  for (i = 0, comma = 0; t[i] != '\0' && comma < p; i += 1) {
    if (t[i] == ',') comma += 1;
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
  } 

  for (bi = 0; t[i] != '\0' && t[i] != ','; i += 1, bi += 1) {
    if (t[i] == '\\' && t[i+1] == ',') i += 1;
    buffer[bi] = t[i];
  }

  buffer[bi] = '\0';
  
  return buffer;
}

OZ_Return typeError(int Pos, char *Comment, char *TypeString)
{
  (void) oz_raise(E_ERROR,E_KERNEL,
		  "type",5,NameUnit,NameUnit,
		  OZ_atom(getTypeOfPos(TypeString, Pos)),
		  OZ_int(Pos+1),
		  OZ_string(Comment));
  return BI_TYPE_ERROR;
}

#ifdef DEBUG_CHECK
int checkBIArity(OZ_CFun fn) {
  Builtin *bi=builtinTab.getEntry((void *) fn);
  return !bi || bi->getOutArity()==0;
}
#endif

/*
 * Control Vars
 */

OZ_Return suspendOnControlVar()
{
  am.prepareCall(BI_controlVarHandler,am.emptySuspendVarList());
  return BI_REPLACEBICALL;
}
