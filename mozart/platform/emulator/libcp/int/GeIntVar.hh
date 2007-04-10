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

#ifndef __GECODE_INT_VAR_HH__
#define __GECODE_INT_VAR_HH__

#include "GeVar.hh"
//#include "GeVar.cc"

//template class GeVar<Gecode::Int::IntVarImpBase>;
// A GeIntVar interfaces an IntVar inside a GenericSpace.

class GeIntVar : public GeVar<Gecode::Int::IntVarImp> {
protected:
  /// copy constructor
  GeIntVar(GeIntVar& gv) :
    GeVar<Gecode::Int::IntVarImp>(gv) {}

public:
  GeIntVar(int index) :
    GeVar<Gecode::Int::IntVarImp>(index,T_GeIntVar) {}

  Gecode::IntVar& getIntVar(void) {
    GeView<Gecode::Int::IntVarImp> iv(getGSpace()->getVar(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    Gecode::IntVar *tmp = new Gecode::IntVar(*vv);
    return (*tmp);
  }

  // the returned reference should be constant
  Gecode::IntVar& getIntVarInfo() {
    GeView<Gecode::Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    Gecode::IntVar *tmp = new Gecode::IntVar(*vv);
    return (*tmp);
  }

  virtual void printDomain(void) {
    Gecode::IntVar tmp = getIntVarInfo();
    //cout<<"tmp ---> ["<<tmp.min()<<", "<<tmp.max()<<"]"<<endl; fflush(stdout);
  }

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
  virtual void printStreamV(ostream &out,int depth);
  virtual Gecode::VarBase* clone(void);
  virtual bool intersect(TaggedRef x);
  
  virtual bool In(TaggedRef x);

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, 
			  GeVar<Gecode::Int::IntVarImp> *lgevar,
			  GeVar<Gecode::Int::IntVarImp> *rgevar) {
    Gecode::IntVar& lintvar = (static_cast<GeIntVar*>(lgevar))->getIntVarInfo();
    Gecode::IntVar& rintvar = (static_cast<GeIntVar*>(rgevar))->getIntVarInfo();    
    eq(s,lintvar, rintvar);
  }

  virtual Gecode::ModEvent bind(GenericSpace *s, 
				GeVar<Gecode::Int::IntVarImp> *v, 
				OZ_Term val) {
    int n = OZ_intToC(val);
    return Gecode::Int::IntView(getIntVarInfo()).eq(s,n);
  }

  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {
    GeView<Gecode::Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    return vv->assigned();
  }
  
  virtual OZ_Term getVal(void) {
    GeView<Gecode::Int::IntVarImp> iv(getGSpace()->getVarInfo(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    return OZ_int(vv->val());
  }

  void ensureDomReflection(OZ_Term t);

};


inline OZ_Term new_GeIntVar(const Gecode::IntSet& dom) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  Gecode::IntVar x(sp,dom);
  GeIntVar *nv = new GeIntVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<Gecode::VarBase*>(x.variable()), ref);

  nv->ensureValReflection(ref);
  //  nv->ensureDomReflection(ref);
 
  return ref;
}

// test whether v is a var reference to a GeIntVar
inline 
bool OZ_isGeIntVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {
    GeVar<Gecode::Int::IntVarImpBase> *gv = static_cast<GeVar<Gecode::Int::IntVarImpBase>*>(oz_getExtVar(v_local));
    return gv->getType() == T_GeIntVar;
  }
  return false;
}

// get the GeIntVar inside the OZ_Term v
inline GeIntVar* get_GeIntVar(OZ_Term v) {
  OZ_Term ref = OZ_deref(v);
  Assert(OZ_isGeIntVar(ref));
  ExtVar *ev = oz_getExtVar(ref);
  return static_cast<GeIntVar*>(ev);
}

// get the Gecode::IntVar from the OZ_Term v
inline Gecode::IntVar& get_IntVar(OZ_Term v) {
  return get_GeIntVar(v)->getIntVar();
}

void module_init_geintvar(void);
#endif
