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
#include "GeSpace.hh"

using namespace Gecode;

unsigned long int GenericSpace::unused_uli;

int GenericSpace::gscounter = 0;

GenericSpace::GenericSpace(Board* b) 
  : vars(), board(b), determined(0),
  gc_pred(NULL), gc_succ(GeSpaceCollectList), gc_marked(false)
{
  //printf("GenericSpace: constructor\n");fflush(stdout);
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
  //printf("GenericSpace: c-constructor\n");fflush(stdout);
  gscounter++;
  if(GeSpaceCollectList) GeSpaceCollectList->gc_pred = this;
  GeSpaceCollectList = this;
}

inline
GenericSpace::~GenericSpace(void) {
  // Release the memory pointed by variables in vars.
  //printf("Removing gespace %p from memory\n",this);fflush(stdout);
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
  //printf("GenericSpace::makeStable(void)\n");fflush(stdout);
  if (oz_isReadOnly(oz_deref(getTrigger()))) {
    return;
  }
  trigger = oz_newReadOnly(board);
}

inline
void GenericSpace::makeUnstable(void) {
  Assert(getTrigger());
  //printf("GenericSpace::makeUnStable(void)\n");fflush(stdout);
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

int GenericSpace::newVar(Gecode::VarBase *v, OZ_Term r, Gecode::VarTypeId vti) {
  vars.newVar(v,r,vti);
  return vars.getSize()-1;
}

Gecode::VarBase* GenericSpace::getVar(int n) { 
  Assert(n >= 0 && n<vars.getSize());
  makeUnstable();
  return &vars.getVar(n);
}

//should return a constant pointer
Gecode::VarBase* GenericSpace::getVarInfo(int n){
  Assert(n >= 0 && n < vars.getSize());
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
  //printf("collected GeSpaces\n");fflush(stdout);
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
  //printf("collected %d spaces\n",i);fflush(stdout);
}
