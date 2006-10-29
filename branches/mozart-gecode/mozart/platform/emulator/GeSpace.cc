/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "var_readonly.hh"
#include "GeVar.hh"
#include "GeSpace.hh"

using namespace Gecode;

VarRefArray::VarRefArray(Gecode::Space* s, VarRefArray& v, bool share) {
  for (int i=0; i<v.vars.size(); i++) {
    refs.push_back(OZ_Term());
    refs[i] = *v.getRef(i);
    
    // clone the GeVar pointed by refs[i]
     OZ_Term dt = OZ_deref(refs[i]);
    if (oz_isExtVar(dt)) {
      // ensures that refs[i] is a gecode contrain variable 
      Assert(oz_getExtVar(dt)->getIdV() == OZ_EVAR_GEVAR);
      vars.push_back(static_cast<GeVar*>(oz_getExtVar(dt))->clone());
    } else {
      /*
	When vars[i] has been bound to a value, refs[i] no longer points to a GeVar but
	to a some structure in the heap (deending on the domain specific representation).
	In this way, vars[i] should never be used after that.
      */
      vars.push_back(static_cast<VarBase*>(NULL));
    }
  }
}

unsigned long int GenericSpace::unused_uli;

int GenericSpace::gscounter = 0;

GenericSpace::GenericSpace(Board* b) 
  : vars(), board(b), determined(0),
  gc_pred(NULL), gc_succ(GeSpaceCollectList), gc_marked(false)
{
  trigger = oz_newReadOnly(board);  
  gscounter++;
  if (GeSpaceCollectList) GeSpaceCollectList->gc_pred = this;
  GeSpaceCollectList = this;
}

inline
GenericSpace::GenericSpace(GenericSpace& s, bool share) 
    : Space(share, s), vars(this, s.vars, share),      
      board(s.board),
      determined(s.determined), trigger(s.trigger),
    gc_pred(NULL), gc_succ(GeSpaceCollectList), gc_marked(false)
{
  gscounter++;
  if(GeSpaceCollectList) GeSpaceCollectList->gc_pred = this;
  GeSpaceCollectList = this;
}

inline
GenericSpace::~GenericSpace(void) {
  // Release the memory pointed by variables in vars.
  if (gc_pred) {
    gc_pred->gc_succ = gc_succ;
  } else {
    GeSpaceCollectList = gc_succ;
  }
  if (gc_succ) {
    gc_succ->gc_pred = gc_pred;
  }
  gscounter--;
}

inline
void GenericSpace::setBoard(Board* b) {
  board = b;
}

inline
void GenericSpace::makeStable(void) { 
  Assert(getTrigger());
  if (oz_isReadOnly(oz_deref(getTrigger()))) {
    return;
  }
  trigger = oz_newReadOnly(board);
}

inline
void GenericSpace::makeUnstable(void) {
  Assert(getTrigger());
  TaggedRef t = getTrigger();
  DEREF(t,t_ptr);
  if (oz_isReadOnly(t)) {
    oz_bindReadOnly(t_ptr,NameUnit);
  }
}

Gecode::SpaceStatus GenericSpace::status(unsigned long int& pn) {
  Gecode::SpaceStatus ret = Gecode::Space::status(pn);
  makeStable();
  return ret;
}

int GenericSpace::newVar(Gecode::VarBase *v, OZ_Term r) {
  vars.newVar(v,r);
  return vars.getSize()-1;
}

Gecode::VarBase* GenericSpace::getVar(int n) { 
  Assert(n >= 0 && n<vars.getSize() && &vars.getVar(n));
  makeUnstable();
  return &vars.getVar(n);
}

Gecode::VarBase* GenericSpace::getVarInfo(int n){
  Assert(n >= 0 && n < vars.getSize() && &vars.getVar(n));
  return &vars.getVar(n);
}

// set the reference to the corresponding variable in Mozart
void GenericSpace::setVarRef(int n, OZ_Term t) {
  vars.setRef(n,t);
}

// update the references to the Mozart heap
void GenericSpace::gCollect() {
  //GEOZ_DEBUG_PRINT(("Called collection on references\n"));
  board = board->gCollectBoard();
  for (int i=0; i<vars.getSize(); i++) OZ_gCollect(vars.getRef(i));
  OZ_gCollect(&trigger);
  gc_marked = true;
}

void GenericSpace::sClone() {
  //GEOZ_DEBUG_PRINT(("Called cloning on references\n"));
  board = board->sCloneBoard();
  for (int i=0; i<vars.getSize(); i++) OZ_sClone(vars.getRef(i));
  OZ_sClone(&trigger);
}

// delete unmarked objects
void gCollectGeSpaces() {
  GenericSpace* cur = GeSpaceCollectList;
  unsigned int i = 0;
  while (cur != NULL) {
    if (cur->gc_marked) {
      cur->gc_marked = false;
      cur = cur->gc_succ;
    } else {
      GenericSpace* dead = cur;
      cur = cur->gc_succ;
      delete dead;
      i++;
    }
  }
}
