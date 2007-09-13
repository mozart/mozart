/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
 *    Raphael Collet, 2006-2007
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
      vars.push_back(static_cast<GeVarBase*>(oz_getExtVar(dt))->clone());
    } else {
		/* When vars[i] has been bound to a value, refs[i] no longer points to a
		   GeVar but to a some structure in the heap (depending on the domain 
		   specific representation). In this way, vars[i] should never be used 
		   after that.
      */
      vars.push_back(static_cast<VarBase*>(NULL));
    }
  }
}

int GenericSpace::gscounter = 0;

GenericSpace::GenericSpace(Board* b) 
  : vars(), board(b), determined(0), foreignProps(0), trigger(true),
    unifyProps(0),gc_pred(NULL), gc_succ(NULL), gc_marked(false),
    allocatedMemory(usedMem())
{
	registerGeSpace(this);
	GeSpaceAllocatedMem += allocatedMemory;
  //printf("Constructor: %u %p\n",GeSpaceAllocatedMem,this);fflush(stdout);
}

inline
GenericSpace::GenericSpace(GenericSpace& s, bool share) 
    : Space(share, s), vars(this, s.vars, share), board(s.board), 
	determined(s.determined), foreignProps(s.foreignProps), 
	unifyProps(s.unifyProps), trigger(s.trigger), gc_pred(NULL), gc_succ(NULL),
	gc_marked(false), allocatedMemory(usedMem())
{
  registerGeSpace(this);
  GeSpaceAllocatedMem += allocatedMemory;
}

inline
GenericSpace::~GenericSpace(void) {
  // Release the memory pointed by variables in vars.
  //printf("deleting generic space %d\n",gscounter);fflush(stdout);
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
bool GenericSpace::isStable(void) {
  return trigger;
}

inline
void GenericSpace::makeStable(void) { 
  //printf("makeStable\n");fflush(stdout);
  if (!isStable())
	trigger = true;
}


void GenericSpace::makeUnstable(void) {
	if (trigger) {
		Board *cb = oz_currentBoard();
		trigger = false;
		cb->ensureLateThread();
		TaggedRef status = cb->getStatus();
		DEREF(status, statusPtr); 
	}
}

Gecode::SpaceStatus GenericSpace::mstatus(void) {
  //printf("Status: Allocated Memory before status:  %u  - %u in space %p\n", allocatedMemory,GeSpaceAllocatedMem,this);fflush(stdout);
  GeSpaceAllocatedMem = GeSpaceAllocatedMem <= allocatedMemory ?
    0 : GeSpaceAllocatedMem - allocatedMemory;
  Gecode::SpaceStatus ret = Gecode::Space::status();
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
  //OZ_gCollect(&trigger);
  gc_marked = true;
}

void GenericSpace::sClone() {
  //GEOZ_DEBUG_PRINT(("Called cloning on references\n"));
  //printf("GeSpace.cc sClone\n");fflush(stdout);
  board = board->sCloneBoard();  
  //printf("GeSpace.cc sClone1\n");fflush(stdout);
  for (int i=0; i<vars.getSize(); i++) OZ_sClone(vars.getRef(i));
  //printf("GeSpace.cc sClone2\n");fflush(stdout);
  //OZ_sClone(&trigger);
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
  //printf("Before collect there are %d generic space in memory\n",GenericSpace::gscounter);
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
  //printf("there are %d generic space in memory\n",GenericSpace::gscounter);
  fflush(stdout);
}
