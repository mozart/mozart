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

#ifndef __GECODE_BOOL_VAR_HH__
#define __GECODE_BOOL_VAR_HH__

#include "GeVar.hh"

using namespace Gecode;
using namespace Gecode::Int;

// A GeBoolVar interfaces an IntVar inside a GenericSpace.
class GeBoolVar : public GeVar<IntVarImp,PC_INT_DOM> {
protected:
  /// copy constructor
  GeBoolVar(GeBoolVar& gv) :
    GeVar<IntVarImp,PC_INT_DOM>(gv) {}

public:
  GeBoolVar(int index) :
    GeVar<IntVarImp,PC_INT_DOM>(index,T_GeBoolVar) {}

  IntVar& getBoolVar(void) {
    GeView<Int::IntVarImp> iv(getGSpace()->getVar(index));
    Int::IntView *vv = reinterpret_cast<Int::IntView*>(&iv);
    IntVar *tmp = new IntVar(*vv);
    return (*tmp);
  }

  // the returned reference should be constant
  IntVar& getBoolVarInfo() {
    GeView<Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Int::IntView *vv = reinterpret_cast<Int::IntView*>(&iv);
    IntVar *tmp = new IntVar(*vv);
    return (*tmp);
  }
  
  /**
     \brief Put in out a text representation of the variable.
   */
  void toStream(ostream &out);

  GeVarType type() { return getType(); }

  virtual ExtVar* gCollectV() { return new GeBoolVar(*this); }
  virtual ExtVar* sCloneV() { return new GeBoolVar(*this); }

  /** 
   * \brief Test whether \a v contains a valid element for the domain
   * of variable. \a v must be a smallInt between 0 and 1. This is a 
   * compatibility problem between Oz and Gecode.
   * 
   * @param v An OZ_Term containing a possible value of the variable domain
   */ 
  virtual OZ_Term       statusV();
  //  virtual void printStreamV(ostream &out,int depth);
  virtual VarBase* clone(void);
  virtual bool intersect(TaggedRef x);
  
  virtual bool In(TaggedRef x);

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, 
			  GeVar<IntVarImp,PC_INT_DOM> *lgevar,
			  GeVar<IntVarImp,PC_INT_DOM> *rgevar) {
    IntVar& lintvar = (static_cast<GeBoolVar*>(lgevar))->getBoolVarInfo();
    IntVar& rintvar = (static_cast<GeBoolVar*>(rgevar))->getBoolVarInfo();    
    eq(s,lintvar, rintvar);
    //Gecode2.0
    //rel(s,lintvar,IRT_EQ,rintvar);
  }

  virtual ModEvent bind(GenericSpace *s, 
			GeVar<IntVarImp,PC_INT_DOM> *v, 
			OZ_Term val) {
    int n = OZ_intToC(val);
    return Int::IntView(getBoolVarInfo()).eq(s,n);
  }

  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {
    GeView<Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Int::IntView *vv = reinterpret_cast<Int::IntView*>(&iv);
    return vv->assigned();
  }
  
  virtual OZ_Term getVal(void) {
    GeView<Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Int::IntView *vv = reinterpret_cast<Int::IntView*>(&iv);
    return OZ_int(vv->val());
  }

};


inline OZ_Term new_GeBoolVar(const IntSet& dom) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  IntVar x(sp,dom);
  GeBoolVar *nv = new GeBoolVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarBase*>(x.variable()), ref);

  nv->ensureValReflection();
  return ref;
}

/**
   \brief Checks if the OZ_Term v represents a bool constraint
   variable in the store.
 */
inline 
bool OZ_isGeBoolVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {
    GeVar<Int::IntVarImp,PC_INT_DOM> *gv = 
      static_cast< GeVar <Int::IntVarImp,PC_INT_DOM > * >(oz_getExtVar(v_local));
    return gv->getType() == T_GeBoolVar;
  }
  return false;
}

// get the GeBoolVar inside the OZ_Term v
/**
   \brief Retrieves a GeBoolVar from an OZ_Term. cgv parameter indicates
   if checking for globality needs to be performed.
*/
inline
GeBoolVar* get_GeBoolVar(OZ_Term v, bool cgv = true) {
  Assert(OZ_isGeBoolVar(v));
  return static_cast<GeBoolVar*>(get_GeVar<IntVarImp,PC_INT_DOM>(v,cgv));
}

/**
   \brief Retrieve gecode variable from an OZ_Term afecting 
   space stability. A call to this method will make the gecode
   space unstable.
*/
inline IntVar& get_BoolVar(OZ_Term v) {
    return get_GeBoolVar(v)->getBoolVar();
}

/**
   \brief Retrieve gecode variable from an OZ_Term without afecting 
   space stability. A call to this method will not make the gecode
   space unstable.
*/
inline IntVar& get_BoolVarInfo(OZ_Term v) {
  return get_GeBoolVar(v,false)->getBoolVarInfo();
}

void module_init_geboolvar(void);
#endif
