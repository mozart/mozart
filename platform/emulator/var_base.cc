/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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
#pragma implementation "variable.hh"
#endif

#include "variable.hh"

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
  for (VariableNamer *i = allnames; i!=NULL; i = i->next) {
    if (OZ_isVariable(i->var) && oz_eq(i->var,v)) {
      return i->name;
    }
  }
  return "_";
}

void oz_varAddName(TaggedRef v, const char *nm)
{
  /* check if already in there */
  Assert(nm && *nm);
  VariableNamer *aux = allnames;
  while(aux) {
    if (strcmp(aux->name,nm)==0) {
      aux->var=v;
      return;
    }
    aux = aux->next;
  }
  
  /* not found so add a new one */
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

#ifdef DEBUG_STABLE
Thread *board_constraints_thr = NULL;
SuspList * board_constraints = NULL;

void printBCDebug(Board * b) { printBC(cerr, b); }

void printBC(ostream &ofile, Board * b)
{
  SuspList *sl;
  Board *hb;

  sl = board_constraints; 
  board_constraints = (SuspList *) NULL;

  while (sl != NULL) {
    Thread *thr = sl->getElem ();
    if (thr->isDeadThread () ||
        (hb = thr->getBoard()) == NULL ||
        hb->isFailed ()) {
      sl = sl->dispose ();
      continue;
    }

    thr->print (ofile);
    ofile << endl;
    if (b) { 
      ofile << "    ---> " << (void *) b << endl; 
    }

    sl = sl->getNext();
    board_constraints = new SuspList (thr, board_constraints);
  }

  ofile.flush();
}

#endif
