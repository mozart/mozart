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

#include "gecode/serialization.hh"


using namespace Gecode;

VarRefArray::VarRefArray(Gecode::Space* s, VarRefArray& v, bool share) 
  : size(v.size) {
  
  for (int i=0; i<size; i++) {
    refs[i] = *v.getRef(i);
    
    // clone the GeVar pointed by refs[i]
     OZ_Term dt = OZ_deref(refs[i]);
    if (oz_isExtVar(dt)) {
      // ensures that refs[i] is a gecode contrain variable 
      Assert(oz_getExtVar(dt)->getIdV() == OZ_EVAR_GEVAR);
      vars[i] = static_cast<GeVarBase*>(oz_getExtVar(dt))->clone();
    } else {
      /* When vars[i] has been bound to a value, refs[i] no longer points to a
	 GeVar but to some structure in the heap (depending on the domain 
	 specific representation). In this way, vars[i] should never be used 
	 after that.
      */
      vars[i]=static_cast<VarImpBase*>(NULL);
    }
  }
}

/*
void VarRefArray::setRoot(OZ_Term v) {
  int sz;

  printf("Called setRoot\n");fflush(stdout);
  if(OZ_isCons(v)) {
    printf("setRoot - is a list\n");fflush(stdout);
    sz = OZ_length(v);
    for(int i=0; OZ_isCons(v); v=OZ_tail(v)) {
      printf("setRoot - element\n");fflush(stdout);
      Assert(oz_isGeVar(OZ_head(v)));
    }
  } else if(OZ_isTuple(v)) {
    printf("setRoot - is a tuple\n");fflush(stdout);
    sz=OZ_width(v);
    for(int i=0; i < sz; i++) {	
      printf("setRoot - element\n");fflush(stdout);
      Assert(oz_isGeVar(OZ_getArg(v,i)));
	     //OZ_Term _tmp = OZ_getArg(v,i);
	     //_array[i] = OZ_intToC(_tmp);
    }
  } else {
    printf("setRoot - is a record\n");fflush(stdout);        
    assert(OZ_isRecord(v));
    OZ_Term al = OZ_arityList(v);
    sz = OZ_width(v);		
    for(int i = 0; OZ_isCons(al); al=OZ_tail(al))	
      printf("setRoot - element\n");fflush(stdout);
     Assert(oz_isGeVar(OZ_subtree(v,OZ_head(al))));
  }
  
}
*/
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

//inline
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

void GenericSpace::varReflect(Reflection::VarMap &vmp, bool registerOnly) {
  // Iterate on generic space references to fill the VarMap
  // TODO: create a prefix for this generic space

  printf("Called varReflect on %p with registerOnly set to %d\n",this,registerOnly);
  Support::Symbol p;
  for (int i=0; i<vars.getSize(); i++) {
    OZ_Term t =  *vars.getRef(i);
    OZ_Term dt = OZ_deref(t);
    if (oz_isExtVar(dt)) {
      Assert(oz_getExtVar(dt)->getIdV() == OZ_EVAR_GEVAR);
      //
      GeVarBase *var = static_cast<GeVarBase*>(oz_getExtVar(dt));
      std::stringstream s;
      s << var->getIndex();
      Support::Symbol nn = p.copy();
      nn += Support::Symbol(s.str().c_str(),true);
      var->reflect(vmp,nn,registerOnly);
      printf("Iteration %d Added symbol %s\n",i,nn.toString().c_str());fflush(stdout);
    }
  }
}

void GenericSpace::merge(GenericSpace *src) {
  printf("GeSpace.cc >> called space merge\n");fflush(stdout);

  // Extract variables from src and fill vm
  Reflection::VarMap svm;
  src->varReflect(svm);
  
  // Extract variables from this (register only) Ask Guido.
  
  Reflection::VarMap tvm;
  //varReflect(tvm,true);
  
  printf("GeSpace.cc >> finished VarMap fill\n");fflush(stdout);
  
  Serialization::Deserializer d(this, tvm);
  Reflection::VarMapIter vmi(svm);

  for (Reflection::SpecIter si(src,svm); si(); ++si) {
    try {
      Reflection::ActorSpec& s = si.actor();
      for (;vmi();++vmi) {
	try {
	  d.var(vmi.spec());
	} catch (Reflection::ReflectionException e) {
	   printf("unknown exception while creating VARIABLE\n");fflush(stdout);
	}
      }
	try {
	  d.post(s);
	} catch (Reflection::ReflectionException e) {
	   printf("unknown exception while creating ACTOR\n");fflush(stdout);
	}
    } catch (Reflection::ReflectionException e) {
      printf("FIXME: maybe a reflection actor\n");fflush(stdout);
    }
    printf("Iteration on actor spec\n");fflush(stdout);
  }
  printf("GeSpace.cc >> finished variable and actor creation\n");fflush(stdout);

  printf("GeSpace.cc >> finished space merge\n");fflush(stdout);
}

int GenericSpace::newVar(Gecode::VarImpBase *v, OZ_Term r) {
  vars.newVar(v,r);
  return vars.getSize()-1;
}

Gecode::VarImpBase* GenericSpace::getVar(int n) { 
  Assert(n >= 0 && n<vars.getSize() && &vars.getVar(n));
  makeUnstable();
  return &vars.getVar(n);
}

Gecode::VarImpBase* GenericSpace::getVarInfo(int n){
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
