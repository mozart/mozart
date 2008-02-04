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

#ifndef __GECODE_INT_VAR_HH__
#define __GECODE_INT_VAR_HH__

#include "GeVar.hh"

using namespace Gecode;
using namespace Gecode::Int;

typedef GeVar<IntVarImp,PC_INT_DOM> GeVar_Int;

// A GeIntVar interfaces an IntVar inside a GenericSpace.
class GeIntVar : public GeVar_Int {
protected:
  /// copy constructor
  GeIntVar(GeIntVar& gv) :
    GeVar_Int(gv) {}

public:
  GeIntVar(int index) :
    GeVar_Int(index,T_GeIntVar) {}

  IntVar& getIntVar(void) {
    GeView<Int::IntVarImp> iv(getGSpace()->getVar(index));
    Int::IntView *vv = reinterpret_cast<Int::IntView*>(&iv);
    IntVar *tmp = new IntVar(*vv);
    return (*tmp);
  }

  // the returned reference should be constant
  IntVar& getIntVarInfo() {
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

  virtual ExtVar* gCollectV() { return new GeIntVar(*this); }
  virtual ExtVar* sCloneV() { return new GeIntVar(*this); }

  /** 
   * \brief Test whether \a v contains a valid element for the domain
   * of variable. \a v must be a smallInt between Gecode::Limits::Int::int_min
   * and Gecode::Limits::Int::int_max. This is a compatibility problem between
   * Oz and Gecode.
   * 
   * @param v An OZ_Term containing a possible value of the variable domain
   */ 
  virtual OZ_Term       statusV();
  //  virtual void printStreamV(ostream &out,int depth);
  virtual VarImpBase* clone(void);
  virtual bool intersect(TaggedRef x);
  
  virtual bool In(TaggedRef x);

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, 
			  GeVar_Int *lgevar,
			  GeVar_Int *rgevar) {
    IntVar& lintvar = (static_cast<GeIntVar*>(lgevar))->getIntVarInfo();
    IntVar& rintvar = (static_cast<GeIntVar*>(rgevar))->getIntVarInfo();    

    rel(s,lintvar,IRT_EQ, rintvar);

  }

  virtual ModEvent bind(GenericSpace *s, 
			GeVar_Int *v, 
			OZ_Term val) {
    int n = OZ_intToC(val);
    Int::IntView W(getIntVarInfo());
    return Int::IntView(getIntVarInfo()).eq(s,n);
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
  
  virtual void reflect(Reflection::VarMap &vmp, Support::Symbol &p, 
		       bool registerOnly = false) {
    vmp.put(getGSpace(),getIntVarInfo(),p,registerOnly);
  }
};


inline OZ_Term new_GeIntVar(const IntSet& dom) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  IntVar x(sp,dom);
  GeIntVar *nv = new GeIntVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);

  nv->ensureValReflection();
  return ref;
}

inline OZ_Term new_GeIntVarCompl(const IntSet& dom) {
  GenericSpace *sp = oz_currentBoard()->getGenericSpace();
  IntVar x(sp,dom);
  ViewRanges<IntView> xvr(x);
  IntVar xcompl(sp, Gecode::Int::Limits::int_min, Gecode::Int::Limits::int_max);
  IntView xcv(xcompl);
  xcv.minus_r(sp, xvr);
  GeIntVar *nv     = new GeIntVar(sp->getVarsSize());
  OzVariable *ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(xcompl.var()), ref);
  nv->ensureValReflection();
  return ref;
}

/**
   \brief Checks if the OZ_Term v represents a integer constraint
   variable in the store.
 */
inline 
bool OZ_isGeIntVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {
    GeVar<Int::IntVarImp,PC_INT_DOM> *gv = 
      static_cast< GeVar_Int * >(oz_getExtVar(v_local));
    return gv->getType() == T_GeIntVar;
  }
  return false;
}

/**
   \brief Retrieves a GeIntVar from an OZ_Term. cgv parameter indicates
   if checking for globality needs to be performed.
*/
inline
GeIntVar* get_GeIntVar(OZ_Term v, bool cgv = true) {
  Assert(OZ_isGeIntVar(v));
  return static_cast<GeIntVar*>(get_GeVar<IntVarImp,PC_INT_DOM>(v,cgv));
}

/**
   \brief Retrieve gecode variable from an OZ_Term afecting 
   space stability. A call to this method will make the gecode
   space unstable.
*/
inline IntVar& get_IntVar(OZ_Term v) {
    return get_GeIntVar(v)->getIntVar();
}

/**
   \brief Retrieve gecode variable from an OZ_Term without afecting 
   space stability. A call to this method will not make the gecode
   space unstable.
*/
inline IntVar& get_IntVarInfo(OZ_Term v) {
  return get_GeIntVar(v,false)->getIntVarInfo();
}

void module_init_geintvar(void);

// Init the the module containing the propagators
//void module_init_geintVarProp(void);
void geivp_init(void);
#endif
