/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
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


#include "GeBoolVar.hh"
#include "unify.hh"
#include "misc.hh"
using namespace Gecode;
using namespace Gecode::Int;

Bool GeBoolVar::validV(OZ_Term val) {
  if (OZ_isSmallInt(val)) {
    int n = OZ_intToC(val);
    if(n == 0 || n == 1)
      	return true;      
    else {
      //GEOZ_DEBUG_PRINT(("Invalid bool value. It has to be either 1 or 0");
      return false;
    }
  }
  return false;
}

OZ_Term GeBoolVar::statusV() {
  return OZ_mkTupleC("kinded", 1, OZ_atom("int"));
}


VarBase* GeBoolVar::clone(void) {
  GenericSpace* gs = getGSpace(); //extVar2Var(this)->getBoardInternal()->getGenericSpace(true);
  Assert(gs);
  IntVar &v = getBoolVarInfo();
  IntVar x;
  x.update(gs,false,v);
  return x.variable();
}


//(this) is the global variable
//x is the local variable,  the one that its domain is modified
inline
bool GeBoolVar::intersect(TaggedRef x) {
  IntVar& gv = getBoolVarInfo();
  ViewRanges<IntView> gvr(gv);

  IntVar& liv = get_BoolVarInfo(x);
  IntView vw(liv);
  //Ask Alejandro about the use of getGSpace() instead of oz_currentBoard()
  return (vw.inter(oz_currentBoard()->getGenericSpace(),gvr)==ME_GEN_FAILED ? false: true);
}

//(this) is the global variable
//lx is the local value
inline
bool GeBoolVar::In(TaggedRef lx) {
  IntVar gv = getBoolVarInfo();
  IntView vw(gv);
  return vw.in(oz_intToC(lx));
}

TaggedRef GeBoolVar::clone(TaggedRef v) {
  Assert(OZ_isGeBoolVar(v));
  
  OZ_Term lv = new_GeBoolVar(IntSet(0,1));
  get_GeBoolVar(v,false)->intersect(lv);
  return lv;
}

inline
bool GeBoolVar::hasSameDomain(TaggedRef v) {
  
  Assert(OZ_isGeBoolVar(v));
  IntVar v1 = get_BoolVarInfo(v);
  ViewRanges< IntView > vr1 (v1);
  ViewRanges< IntView > vr2 (getBoolVarInfo());
  
  while(true) {
    if(!vr1() && !vr2()) return true;
    if(!vr1() || !vr2()) return false;
    if( (vr1.min() != vr2.min()) || (vr1.max() != vr2.max() ) ) return false;
    ++vr1; ++vr2;
  }
}

inline
TaggedRef GeBoolVar::newVar(void) {
  return new_GeBoolVar(IntSet(0,1));
}


inline
bool GeBoolVar::IsEmptyInter(TaggedRef* var1,  TaggedRef* var2) {
  
  IntVar& v1 = get_BoolVarInfo(*var1);
  IntVar& v2 = get_BoolVarInfo(*var2);
  
  ViewRanges<IntView > vr1 (v1);
  ViewRanges<IntView > vr2 (v2);
  
  while(true) {
    if(!vr1() || !vr2() ) return true;
    if( vr2.min() <= vr2.max() && vr2.max() <= vr1.max() && vr1.min() <= vr2.max() )
      return false;
    if( vr1.min() <= vr2.max() && vr1.max() <= vr2.max() && vr2.min() <= vr1.max() )
      return false;
    
    if( vr2.max() <= vr1.max() ) ++vr2;
    ++vr1;
  }
}

void GeBoolVar::toStream(ostream &out) {
  std::stringstream oss;
  oss << getBoolVarInfo();
  out << "<GeBoolVar " << oss.str().c_str() << ">"; 
}
  

void module_init_geboolvar(void){
  
}

#define STATICALLY_INCLUDED
#include "modGeBoolVar-table.cc"
