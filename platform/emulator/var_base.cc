/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: many
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Variables
  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "variable.hh"
#endif

#include "oz.h"

#include "tagged.hh"
#include "term.hh"
#include "constter.hh"

#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "thread.hh"
#include "susplist.hh"

#include "variable.hh"

void addSuspAnyVar(TaggedRefPtr v, SuspList * el)
{
  SVariable * sv;
  if (isSVar(*v)) {
    sv = tagged2SVar(*v);
  } else if (isCVar(*v)) {
    sv = taggedCVar2SVar(*v);
  } else {
    sv = new SVariable(tagged2VarHome(*v));
    *v = makeTaggedSVar(sv);
  }
  sv->suspList = addSuspToList(sv->suspList, el, sv->home);
}



/*
 * Class VariableNamer: assign names to variables
 */

VariableNamer *VariableNamer::allnames = NULL;

static char *getAtomName(TaggedRef n)
{
  DEREF(n,_1,_2);
  if (!OZ_isAtom(n)) {
    return "";
  }
  return tagged2Literal(n)->getPrintName();
}


#ifdef PRETTYVARNAMES
/* Different variables may have been unified, so we
 * return "X=Y" as printname if X and Y have been unified
 */
#endif
TaggedRef VariableNamer::getName(TaggedRef v)
{
  TaggedRef ret = AtomVoid;
  int found = 0;
  char buf[1000];
  buf[0] = '\0';
  for (VariableNamer *i = allnames; i!=NULL; i = i->next) {
#ifdef PRETTYVARNAMES
/* Browser cannot handle: declare A B in {Browse A#B}  A=B  */
    if (sameTerm(i->var,v)) {
      ret = i->name;
      found++;
      if (found > 1) {
	strcat(buf,"=");
      }
      strcat(buf,getAtomName(ret));
    }
#else
    if (OZ_isVariable(i->var) && sameTerm(i->var,v)) {
      return i->name;
    }
#endif
  }
  return (found<=1) ? ret : makeTaggedAtom(buf);
}

void VariableNamer::addName(TaggedRef v, TaggedRef nm)
{
  static int counter = 50;
  if (counter-- == 0) {
    cleanup();
    counter = 50;
  }
  VariableNamer *n = new VariableNamer();
  n->next = allnames;
  n->var = v;
  n->name = nm;
  allnames = n;
  OZ_protect(&n->var);
  OZ_protect(&n->name);
}

int VariableNamer::length()
{
  int ret = 0;
  VariableNamer *aux = this;
  while(aux) {
    ret++;
    aux = aux->next;
  }
  return ret;
}


/* remove all entries, that are not a variable */
void VariableNamer::cleanup()
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
      OZ_unprotect(&aux1->name);
      delete aux1;
    }
  }
}
