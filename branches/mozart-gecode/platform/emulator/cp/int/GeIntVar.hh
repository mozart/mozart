
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



// A GeIntVar interfaces an IntVar inside a GenericSpace.

class GeIntVar : public GeVar {
protected:
  // copy constructor
  GeIntVar(GeIntVar& gv) : GeVar(gv) {}

public:
  GeIntVar(int index) : GeVar(index,T_GeIntVar) {}

  Gecode::IntVar& getIntVar(void) {
    GenericSpace* gs = extVar2Var(this)->getBoardInternal()->getGenericSpace(true);
    Assert(gs);
    //    return gs->getIntVar(index);

    GeView<Gecode::Int::IntVarImp> iv(gs->getVar(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    Gecode::IntVar *tmp = new Gecode::IntVar(*vv);
    return (*tmp);
  }
  /*
  const Gecode::IntVar& getIntVarInfo() {
    GenericSpace* gs = extVar2Var(this)->getBoardInternal()->getGenericSpace(true);
    Assert(gs);
    return gs->getIntVarInfo(index);
  }
  */
  
  const Gecode::IntVar& getIntVarInfo() {
      GenericSpace* gs = extVar2Var(this)->getBoardInternal()->getGenericSpace(true);
    Assert(gs);
    //    return gs->getIntVar(index);

    GeView<Gecode::Int::IntVarImp> iv(gs->getVarInfo(index));
    Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
    Gecode::IntVar *tmp = new Gecode::IntVar(*vv);
    return (*tmp);
  }

  GeVarType type() { return getType(); }

  virtual ExtVar* gCollectV() { return new GeIntVar(*this); }
  virtual ExtVar* sCloneV() { return new GeIntVar(*this); }

  virtual OZ_Return     unifyV(TaggedRef*, TaggedRef*);
  virtual OZ_Return     bindV(TaggedRef*, TaggedRef);

  /** 
   * \brief Test whether \a v contains a valid element for the domain
   * of variable. \a v must be a smallInt between Gecode::Limits::Int::int_min
   * and Gecode::Limits::Int::int_max. This is a compatibility problem between
   * Oz and Gecode.
   * 
   * @param v An OZ_Term containing a possible value of the variable domain
   */ 
  virtual Bool          validV(TaggedRef v);
  virtual OZ_Term       statusV();

  virtual void printStreamV(ostream &out,int depth);
};



void postIntVarReflector(GenericSpace* s, int index, OZ_Term ref);
/*
inline OZ_Term new_GeIntVar(Gecode::IntSet& dom) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  Gecode::IntVar x(sp,dom);
  cout<<"Estoy en Ge_intVar: "<<endl; fflush(stdout);
  OzVariable* ov   = extVar2Var(new GeIntVar(sp->getVarsSize()));
  cout<<"En este punto "<<endl; fflush(stdout);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  cout<<"En este otro punto "<<endl; fflush(stdout);
  cout<<"x["<<sp->getVarsSize()<<"]: "<<x.min()<<"-"<<x.max()<<endl; fflush(stdout);
  //Gecode::VarBase va =*( static_cast<Gecode::VarBase*>(x.variable()));
  int index        = sp->newIntVar(static_cast<Gecode::VarBase*>(x.variable()),ref,Gecode::VTI_INT);
  //  int index = sp->newIntVar(va,ref,Gecode::VTI_INT);
  cout<<"O en este tal vez" <<endl; fflush(stdout);
  //printf("GeIntVar.hh new_GeIntVar     GenericSpace= %p\n",sp);fflush(stdout);
  postIntVarReflector(sp, index, ref);
  cout<<"Termino new_GeIntVar"<<endl; fflush(stdout);
  return ref;
}
*/

inline OZ_Term new_GeIntVar(Gecode::IntSet& dom) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  Gecode::IntVar x(sp,dom);
  OzVariable* ov   = extVar2Var(new GeIntVar(sp->getVarsSize()));
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<Gecode::VarBase*>(x.variable()), ref, Gecode::VTI_INT);
  postIntVarReflector(sp,index,ref);
  return ref;
}

// test whether v is a var reference to a GeIntVar
inline 
bool OZ_isGeIntVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {
    GeVar *gv = static_cast<GeVar*>(oz_getExtVar(v_local));
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


// This Gecode propagator reflects an IntVar assignment inside Mozart

class IntVarReflector : public VarReflector<Gecode::Int::IntView> {
public:
  IntVarReflector(GenericSpace* s, Gecode::Int::IntView v, int idx) :
    VarReflector<Gecode::Int::IntView>(s, v, idx) {}

  IntVarReflector(GenericSpace* s, bool share, IntVarReflector& p) :
    VarReflector<Gecode::Int::IntView>(s, share, p) {}

  Gecode::Actor* copy(Gecode::Space* s, bool share) {
    return new (s) IntVarReflector(static_cast<GenericSpace*>(s), share, *this);
  }

  OZ_Term getVarRef(GenericSpace* s) { return s->getVarRef(index); }
  OZ_Term getVal() { return OZ_int(x0.val()); }
};

/*
inline void postIntVarReflector(GenericSpace* s, int index, OZ_Term ref) {
  cout<<"Inicio postIntVarReflector "<<endl; fflush(stdout);
  s->setIntVarRef(index, ref);
  cout<<"Despues en postIntVarReflector "<<endl; fflush(stdout);
  //printf("postIntVarReflector\n");fflush(stdout);
  cout<<"Par: "<<(s->getIntVar(index)).min()<<" index: "<<index<<endl; fflush(stdout);
  new (s) IntVarReflector(s, s->getIntVar(index), index);
  cout<<"Termino IntVarReflector "<<endl; fflush(stdout);
}
*/

inline void postIntVarReflector(GenericSpace* s, int index, OZ_Term ref) {
  s->setVarRef(index,ref);
  Gecode::Int::IntVarImp *ivp = reinterpret_cast<Gecode::Int::IntVarImp*>(s->getVar(index));
  GeView<Gecode::Int::IntVarImp> iv(ivp);
  Gecode::Int::IntView *vv = reinterpret_cast<Gecode::Int::IntView*>(&iv);
  new (s) IntVarReflector(s, *vv, index);
}

void module_init_geintvar(void);
#endif
