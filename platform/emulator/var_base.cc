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
#pragma implementation "variable.hh"
#endif

#include "variable.hh"
#include "oz.h"

#include "tagged.hh"
#include "value.hh"
#include "cont.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "am.hh"
#include "thread.hh"
#include "susplist.hh"

#include "variable.hh"
#include "genvar.hh"

/*
 * Class VariableNamer: assign names to variables
 */

VariableNamer *VariableNamer::allnames = NULL;

const char *VariableNamer::getName(TaggedRef v)
{
  for (VariableNamer *i = allnames; i!=NULL; i = i->next) {
    if (OZ_isVariable(i->var) && termEq(i->var,v)) {
      return i->name;
    }
  }
  return "_";
}

void VariableNamer::addName(TaggedRef v, const char *nm)
{
  /* check if already in there */
  VariableNamer *aux = allnames;
  while(aux) {
    if (strcmp(aux->name,nm)==0) {
      aux->var=v;
#if 0
      if (am.debugmode())
	OZ_warning("Redeclaration of toplevel variable `%s'", nm);
#endif
      return;
    }
    aux = aux->next;
  }
  
  /* not found so add a new one */
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
      delete aux1;
    }
  }
}

const char *getVarName(TaggedRef v)
{
  const char *s = VariableNamer::getName(v);
  return *s ? s : "_";
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
