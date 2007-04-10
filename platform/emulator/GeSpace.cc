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
      //      vars.push_back(static_cast<GeVar<void>*>(oz_getExtVar(dt))->clone());      
      vars.push_back(static_cast<GeVarBase*>(oz_getExtVar(dt))->clone());
    } else {
      /*
	When vars[i] has been bound to a value, refs[i] no longer points to a GeVar but
	to a some structure in the heap (depending on the domain specific representation).
	In this way, vars[i] should never be used after that.
      */
      vars.push_back(static_cast<VarBase*>(NULL));
    }
  }
}

unsigned long int GenericSpace::unused_uli;

int GenericSpace::gscounter = 0;

GenericSpace::GenericSpace(Board* b) 
  : vars(), board(b), determined(0), foreignProps(0),
    unifyProps(0),gc_pred(NULL), gc_succ(NULL), gc_marked(false),
    allocatedMemory(usedMem())
{
  trigger = oz_newReadOnly(board);  
  registerGeSpace(this);
  GeSpaceAllocatedMem += allocatedMemory;
  //printf("Constructor: %u %p\n",GeSpaceAllocatedMem,this);fflush(stdout);
}

inline
GenericSpace::GenericSpace(GenericSpace& s, bool share) 
    : Space(share, s), vars(this, s.vars, share),      
      board(s.board),
      determined(s.determined), foreignProps(s.foreignProps),unifyProps(s.unifyProps),
      trigger(s.trigger), gc_pred(NULL), gc_succ(NULL), gc_marked(false),
      allocatedMemory(usedMem())
{
  registerGeSpace(this);
  GeSpaceAllocatedMem += allocatedMemory;
  //printf("Constructor Copia: %u %p\n",GeSpaceAllocatedMem,this);fflush(stdout);
}

inline
GenericSpace::~GenericSpace(void) {
  // Release the memory pointed by variables in vars.
  printf("deleting generic space %d\n",gscounter);fflush(stdout);
  if (gc_pred) {
    gc_pred->gc_succ = gc_succ;
  } else {
    GeSpaceCollectList = gc_succ;
  }
  if (gc_succ) {
    gc_succ->gc_pred = gc_pred;
  } else {
    GeSpaceCollectLast = gc_pred;
  }
  gscounter--;

  GeSpaceAllocatedMem = GeSpaceAllocatedMem <= allocatedMemory ?
    0 : GeSpaceAllocatedMem - allocatedMemory;
}

inline
void GenericSpace::setBoard(Board* b) {
  board = b;
}

//inline
bool GenericSpace::isStable(void) {
  Assert(getTrigger());
  return oz_isReadOnly(oz_deref(getTrigger()));
}

inline
void GenericSpace::makeStable(void) { 
  if (!isStable()) {
    trigger = oz_newReadOnly(board);
  }
  /*
    Assert(getTrigger());
    if (oz_isReadOnly(oz_deref(getTrigger()))) {
    return;
    }
    trigger = oz_newReadOnly(board);
  */
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

Gecode::SpaceStatus GenericSpace::mstatus(unsigned long int& pn) {
  //printf("Status: Allocated Memory before status:  %u  - %u in space %p\n", allocatedMemory,GeSpaceAllocatedMem,this);fflush(stdout);

  GeSpaceAllocatedMem = GeSpaceAllocatedMem <= allocatedMemory ?
    0 : GeSpaceAllocatedMem - allocatedMemory;
  
  Gecode::SpaceStatus ret = Gecode::Space::status(pn);
  allocatedMemory = usedMem();
  GeSpaceAllocatedMem += allocatedMemory;
  makeStable();
  //  printf("Status: Memory changed... %u in space %p\n", GeSpaceAllocatedMem,this);fflush(stdout);

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
  //printf("GeSpace gcollect\n");fflush(stdout);
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

void registerGeSpace(GenericSpace* gs) {
  //printf("registerSpace\n");fflush(stdout);
  //  gscounter++;
  if (!GeSpaceCollectList) {
    //printf("registerSpace vacia\n");fflush(stdout);
    GeSpaceCollectList = gs;
    GeSpaceCollectLast = gs;
  } else {
    //printf("registerSpace otro\n");fflush(stdout);
    //Assert(gs->gc_succ == NULL);
    gs->gc_succ = NULL;
    GenericSpace *tmp = GeSpaceCollectLast;
    GeSpaceCollectLast->gc_succ = gs;
    gs->gc_pred = tmp;
    GeSpaceCollectLast = gs;
  }
  GenericSpace::gscounter++;
  /*GenericSpace *tmp = GeSpaceCollectList;
  for (int i = 0;tmp != NULL; tmp = tmp->gc_succ, i++) {
    printf("uno mas %d\n",i);fflush(stdout);
  }
  */
    //GeSpaceAllocateMem += sizeof(GenericSpace) + gs->
}

// delete unmarked objects
void gCollectGeSpaces() {
  //Compute the allocated memory
  /* GenericSpace* ptr = GeSpaceCollectList;
  size_t geAlloc = 0;
  while (ptr != NULL) {
    geAlloc = geAlloc + sizeof(*ptr) + ptr->allocated() + ptr->cached();
    ptr = ptr->gc_succ;
  }
  printf("collecting memory: %d\n",geAlloc);fflush(stdout);
  */
  printf("Before collect there are %d generic space in memory\n",GenericSpace::gscounter);
  fflush(stdout);
  //  printf("collecting memory used by generic spaces\n");fflush(stdout);
  GenericSpace* cur = GeSpaceCollectList;
  unsigned int i = 0;
  unsigned int j = 0;
  while (cur != NULL) {
    j=j+1;
    if (cur->gc_marked) {
      cur->gc_marked = false;
      cur = cur->gc_succ;
    } else {
      //printf("collecting memory used by generic spaces\n");fflush(stdout);
      GenericSpace* dead = cur;
      cur = cur->gc_succ;
      delete dead;
      i++;
    }
  }
  //printf("collected memory for %d of %d spaces\n",i,j);
  printf("there are %d generic space in memory\n",GenericSpace::gscounter);
  fflush(stdout);
}
