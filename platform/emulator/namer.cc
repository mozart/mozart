/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "var_base.hh"

#ifdef NEW_NAMER

//-----------------------------------------------------------------------------
// naming variables

typedef Namer<OZ_Term, const char *> VarNamer;
template VarNamer;

VarNamer * VarNamer::_head;

VarNamer varNamer; 

const char * oz_varGetName(OZ_Term v)
{
  const char * name = varNamer.getName(derefIndexNamer(v));
  return (name == (const char *) NULL) ? "_" : name;
}

void oz_varAddName(OZ_Term v, const char *nm)
{
  DEREF(v, vptr);
    
  Assert(!oz_isRef(v));
  if (!oz_isVarOrRef(v))
    return;
  varNamer.addName(makeTaggedRef(vptr), nm);
}

Bool isCacMarkedNamer(OZ_Term t) 
{ 
  OZ_Term t_deref = oz_deref(t);
  Assert(!oz_isRef(t_deref));
  return oz_isRef(t) && (oz_isMark(t_deref) || 
			 (oz_isVar(t_deref) && 
			  tagged2Var(t_deref)->cacIsMarked()));
}

void GCollectIndexNamer(OZ_Term &t)
{
  oz_gCollectTerm(t, t);
}

OZ_Term getCacForward(OZ_Term t) 
{
  OZ_Term t_deref = oz_deref(t);
  Assert(!oz_isRef(t_deref));
  return (oz_isVar(t_deref) 
	  ? makeTaggedRef(tagged2Var(t_deref)->cacGetFwd()) 
	  : (OZ_Term) tagged2UnmarkedPtr(t));
}

void GCollectDataNamer(const char * &)
{
  // nothing to be done
}

OZ_Term derefIndexNamer(OZ_Term t)
{
  return oz_derefPtr(t);
}

const char * toStringNamer(const char * s) 
{
  return s;
}

//-----------------------------------------------------------------------------
// naming propagators

typedef Namer<Propagator *, OZ_Term> PropNamer;
template PropNamer;

PropNamer * PropNamer::_head;

PropNamer propNamer; 

Bool isCacMarkedNamer(Propagator * p)
{ 
  return p->isCacMarked(); 
}
void GCollectIndexNamer(Propagator * &p) 
{
  p = SuspToPropagator(p->gCollectSuspendable());
}

Propagator *  getCacForward(Propagator * p) 
{
  return (Propagator *) p->cacGetFwd();
}

void GCollectDataNamer(OZ_Term &t)
{
  oz_gCollectTerm(t, t);
}

Propagator * derefIndexNamer(Propagator * p)
{
  return p;
}

const char * toStringNamer(OZ_Term t) 
{
  return OZ_toC(t, 10, 10);
}

OZ_Term oz_propGetName(Propagator * p)
{
  NEW_NAMER_DEBUG_PRINT(("oz_propgetName: %p\n", p));
  OZ_Term name = propNamer.getName(p);
  return (name == (OZ_Term) NULL) ? OZ_unit() : name;
}

void oz_propAddName(Propagator * p, OZ_Term name)
{
  /*
  NEW_NAMER_DEBUG_PRINT(("oz_propAddName: %p = %s\n", p, 
			 OZ_toC(name, 10, 10)));
  */
  propNamer.addName(p, name);
}

#else

/*
 * Class VariableNamer: assign names to variables
 */

class VariableNamer {
public:
  TaggedRef var;
  const char *name;
  VariableNamer *next;
  VariableNamer(TaggedRef var, const char *name, VariableNamer *next)
    : var(var), name(name), next(next) {}
};

static
VariableNamer *allnames = NULL;

const char *oz_varGetName(TaggedRef v)
{
  v = oz_safeDeref(v);
  for (VariableNamer *i = allnames; i!=NULL; i = i->next) {
    if (OZ_isVariable(i->var) && oz_eq(oz_safeDeref(i->var),v)) {
      return i->name;
    }
  }
  return "_";
}

void oz_varAddName(TaggedRef v, const char *nm)
{
  // check if already in there 
  Assert(nm && *nm);
  VariableNamer *aux = allnames;
  while(aux) {
    if (strcmp(aux->name,nm)==0) {
      aux->var=v;
      return;
    }
    aux = aux->next;
  }
  
  // not found so add a new one 
  static int counter = 50; // mm2: magic constant
  if (counter-- == 0) {
    oz_varCleanup();
    counter = 50;
  }
  VariableNamer *n = new VariableNamer(v,nm,allnames);
  allnames = n;
  OZ_protect(&n->var);
}

/* remove all entries, that are not a variable */
void oz_varCleanup()
{
  VariableNamer *aux = allnames;
  allnames = NULL;
  while(aux) {
    VariableNamer *aux1 = aux;
    aux = aux->next;
    if (OZ_isVariable(aux1->var)) {
      aux1->next = allnames;
      allnames = aux1;
    } else {      
      OZ_unprotect(&aux1->var);
      delete aux1;
    }
  }
}

#endif /* NEW_NAMER */
