/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
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


#include "GeIntVar.hh"
#include "unify.hh"

using namespace Gecode;
using namespace Gecode::Int;



OZ_Return GeIntVar::unifyV(TaggedRef* lPtr, TaggedRef* rPtr) {

  //  if (!OZ_isGeIntVar(*rPtr)) return OZ_suspendOnInternal(*rPtr);

  GeIntVar* lgeintvar = this;
  GeIntVar* rgeintvar = get_GeIntVar(*rPtr);
  //printf("GeIntVar::unifyV lge=%d rg=%d\n",oz_isLocalVar(extVar2Var(lgeintvar)),oz_isLocalVar(extVar2Var(rgeintvar)));fflush(stdout);
  bool is_global = false;
  // Unification of variables from different spaces is not implemented
  // yet. What does this means for us??
  if (extVar2Var(lgeintvar)->getBoardInternal()->getGenericSpace() != extVar2Var(rgeintvar)->getBoardInternal()->getGenericSpace()) {    
    if(oz_isLocalVar(extVar2Var(lgeintvar)))
    {  
      GeIntVar *tmp = rgeintvar;
      TaggedRef *tmp_tagged = rPtr;
      rgeintvar = lgeintvar;
      rPtr = lPtr;
      lgeintvar = tmp;
      lPtr = tmp_tagged;
      is_global = true;
      //printf("Operation not implemented\n");fflush(stdout);
    }
    
    
    //return SUSPEND;
  }
  GenericSpace* space = extVar2Var(lgeintvar)->getBoardInternal()->getGenericSpace();
  //printf("GeIntVar::unifyV\n");fflush(stdout);
  IntVar& lintvar = lgeintvar->getIntVar();
  IntVar& rintvar = rgeintvar->getIntVar();

  // compute the intersection of the domains
  ViewRanges<IntView> lrange(lintvar);
  ViewRanges<IntView> rrange(rintvar);
  Iter::Ranges::Inter<ViewRanges<IntView>, ViewRanges<IntView> >
    irange(lrange, rrange);
  if (irange()) {
    //printf("GeIntVar::unifyV antes de bindLocal oz_isVar(lPtr)=%d -- oz_isVar(rPtr)=%d %d\n",oz_isVar(*lPtr),oz_isVar(*rPtr),*lPtr);fflush(stdout);
    if (is_global)
      {
	//lgeintvar->intersect(makeTaggedVar(extVar2Var(rgeintvar)));
	oz_bindGlobalVar2(extVar2Var(lgeintvar), lPtr, makeTaggedRef(rPtr));	
	//true==true;
      }
    else
      {
	oz_bindLocalVar(extVar2Var(this), lPtr, makeTaggedRef(rPtr));
	eq(space, lintvar, rintvar);
      }

	
    //printf("GeIntVar::unifyV despues de bindLocal oz_isVar(lPtr)=%d -- oz_isVar(rPtr)=%d %d\n",oz_isVar(*lPtr),oz_isVar(*rPtr),*lPtr);fflush(stdout);
    // intersect the domain of rvar with the domain of lvar
    ////    IntView(rintvar).inter(space, lrange);
    /* Unification is entailed by means of an eq propagator. After post this
       propagator the generic space must becomes unstable. The unstability is
       a result of posting the propagator */

    // wakeup space propagators to inmediatly update all related variables
    unsigned long alt = 0; //useless variable
    //    return (space->status(alt)== Gecode::SS_FAILED) ? FAILED: PROCEED ;
    if(space->status(alt) == Gecode::SS_FAILED) {
      extVar2Var(this)->getBoardInternal()->setFailed();
      return FAILED;
    }
    else{
      return PROCEED;
    }
    //xspace->status(alt);
        
    //return PROCEED;
  } else {
    return FAILED;
  }
}


/*
  At this point vPtr must be a local variable (be careful)
*/
OZ_Return GeIntVar::bindV(TaggedRef* vPtr, TaggedRef val) {
  if (validV(val)) {
    //Board *tmp = oz_currentBoard();
    //am.setCurrent(extVar2Var(this)->getBoardInternal(),extVar2Var(this)->getBoardInternal()->getOptVar());
    if (oz_isLocalVar(extVar2Var(this))) {
      //if (true) {
      // first bind the variable in Mozart
      //printf("bindV var length=%d isVarOrRef=%d\n",extVar2Var(this)->getSuspListLengthS(),oz_isVarOrRef(val));fflush(stdout);
      oz_bindLocalVar(extVar2Var(this), vPtr, val);

      // then bind the IntVar in the GenericSpace
      GenericSpace* s =  extVar2Var(this)->getBoardInternal()->getGenericSpace();      
      int n = OZ_intToC(val);
      //printf("GeIntVar::bindV\n");fflush(stdout);
      ModEvent me = IntView(getIntVar()).eq(s, n);
      Assert(!me_failed(me));     // must succeed

      unsigned long alt = 0; //useless variable
      //      return (s->status(alt)== Gecode::SS_FAILED) ? FAILED: PROCEED ;
      if (s->status(alt) == Gecode::SS_FAILED) {
	extVar2Var(this)->getBoardInternal()->setFailed();
	return FAILED;
      }
      //printf("end bindV\n");fflush(stdout);
      return PROCEED;

      // wakeup space propagators to inmediatly update all related variables
      //unsigned int alt = 0; //useless variable
      //s->status(alt);
    } else {
      //cout<<"NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO global var"<<endl; fflush(stdout);
      // global binding...
      oz_bindGlobalVar(extVar2Var(this), vPtr, val);
    }
    //am.setCurrent(tmp,tmp->getOptVar());
    return PROCEED;
  }
  return OZ_FAILED;
}

Bool GeIntVar::validV(TaggedRef val) {
  // BigInts cannot be represented by normal c int type. 
  // SmallInts are just c ints so we only allow that kind of integer values
  // References:
  //   FD.sup = 134 217 726
  //   FD.inf = 0
  // In Gecode:
  //   Gecode::Limits::Int::int_max
  //   Gecode::Limits::Int::int_min
  if (OZ_isSmallInt(val)) {
    int n = OZ_intToC(val);
    if(n >= Gecode::Limits::Int::int_min &&
       n <= Gecode::Limits::Int::int_max)
      {
	//printf("GeIntVar::validV\n");fflush(stdout);
      return IntView(getIntVar()).in(n);
      }
    else {
      GEOZ_DEBUG_PRINT(("Invalid integer.\n All domain ranges must be between %d and %d",Gecode::Limits::Int::int_min, Gecode::Limits::Int::int_max));
      return false;
    }
  }
  return false;
}

OZ_Term GeIntVar::statusV() {
  //printf("called statusV\n");fflush(stdout);
  return OZ_mkTupleC("kinded", 1, OZ_atom("int"));
}


VarBase* GeIntVar::clone(void) {
  GenericSpace* gs = extVar2Var(this)->getBoardInternal()->getGenericSpace(true);
  Assert(gs);
  IntVar &v = getIntVarInfo();
  IntVar x;
  x.update(gs,false,v);
  return x.variable();
}


//(this) is the global variable
//x is the local variable
bool GeIntVar::intersect(TaggedRef x) {
  IntVar& gv = getIntVarInfo();
  ViewRanges<IntView> gvr(gv);

  IntVar& liv = get_IntVar(x);
  IntView vw(liv);
  return (vw.inter(oz_currentBoard()->getGenericSpace(),gvr)==Gecode::ME_GEN_FAILED ? false: true);
  //  return (vw.inter(oz_currentBoard()->getGenericSpace(),gvr) != Gecode::ME_GEN_FAILED)
  
  //  return vw.size()>0;
}

//(this) is the global variable
//lx is the local value
bool GeIntVar::In(TaggedRef lx) {
  IntVar gv = getIntVarInfo();
  IntView vw(gv);
  return vw.in(oz_intToC(lx));
}

TaggedRef GeIntVar::clone(TaggedRef v) {
  Assert(OZ_isGeIntVar(v));
  //  IntVar& gv = get_IntVar(v);
  //  ViewRange<IntView> gvr(gv);
  
  OZ_Term lv = new_GeIntVar(IntSet(Limits::Int::int_min,Limits::Int::int_max));
  //IntVar& liv = get_IntVar(lv);
  //IntView(liv).inter(oz_currentBoard()->getGenericSpace(),gvr);
  ////  get_GeIntVar(lv)->intersect(v);
  get_GeIntVar(v)->intersect(lv);
  return lv;
}

#include <iostream>
#include <sstream>
#include <string>

void GeIntVar::printStreamV(ostream &out,int depth) {

  std::stringstream oss;
  oss << getIntVarInfo();
  out << "<GeIntVar " << oss.str().c_str() << ">"; 
  //printf("called print stream-FINISHED\n");fflush(stdout);
}

void module_init_geintvar(void){
  
}

#define STATICALLY_INCLUDED
#include "modGeIntVar-table.cc"
