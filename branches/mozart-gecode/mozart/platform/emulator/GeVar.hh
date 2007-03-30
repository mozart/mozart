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

#ifndef __GECODE_VAR_HH__
#define __GECODE_VAR_HH__



#include "var_ext.hh"
#include "var_base.hh"
#include "susplist.hh"
#include "unify.hh"
#include "GeSpace.hh"


template<class Var>
class GeView: public Gecode::VariableViewBase<Var> {
public:
  GeView(Gecode::VarBase *var)
    : Gecode::VariableViewBase<Var>(static_cast<Var*>(var)) {} 
};

enum GeVarType {T_GeIntVar, T_GeSetVar};

/** 
 * \brief Abstract class for Oz variables that interface Gecode 
 * variables inside a GenericSpace
 */
template<class VarImp>
class GeVar : public ExtVar {
private:
  GeVarType type;    /// Type of variable (e.g IntVar, SetVar, etc)
protected:
  int index;        /// The index inside the corresponding GenericSpace
  bool hasValRefl,hasDomRefl;  /// Refelction Mechanism control

  /**
   \brief Counter for the number of unifications in that this variable 
   is involved. 
  */
  unsigned int unifyC;
  
 
  /// Copy constructor
  GeVar(GeVar& gv) : 
    ExtVar(extVar2Var(&gv)->getBoardInternal()), type(gv.type), 
    index(gv.index), hasValRefl(gv.hasValRefl), hasDomRefl(gv.hasDomRefl),
    unifyC(gv.unifyC)
  {
    // ensure a valid type of varable.
    Assert(type >= T_GeIntVar && type <= T_GeSetVar);
  }

  void incUnifyC(void) { unifyC++; }

public:
  /// \name Constructor
  //@{
  /** 
   * \brief Creates a new Oz variable that references a Gecode variable
   * 
   * @param h The corresponding GeSpace for the variable  
   * @param n The index inside the corresponding GenericSpace
   */
  GeVar(int n, GeVarType t) :
    ExtVar(oz_currentBoard()), type(t), index(n), 
    hasValRefl(false), hasDomRefl(false), unifyC(0) 
  {
    Assert(type >= T_GeIntVar && type <= T_GeSetVar);
  }
  //@}
  
  /// \name Mozart ExtVar stuff  
  //@{
  ExtVarType getIdV() { return OZ_EVAR_GEVAR;}
  virtual ExtVar* gCollectV() = 0;
  virtual ExtVar* sCloneV() = 0;
  
  virtual void gCollectRecurseV() { }
  virtual void sCloneRecurseV() { }
  virtual OZ_Term statusV() = 0;
  virtual VarStatus checkStatusV() {
    return EVAR_STATUS_KINDED;
  }

  int getIndex(void) {return index;}
  bool hasDomReflector(void) {return hasDomRefl; }
  
  virtual void printDomain(void) = 0;
  
  virtual void disposeV() {
    disposeS();     // free susplist
    // here we can free the memory taken by the variable itself (if we
    // really want to)
  }
  //@}

  /** 
      \brief Add a suspendable to the variable suspension list.
      By default this function returns PROCEED for an ExtVar. However, 
      for constraint variables it should return SUSPEND as buil-tin mozart 
      constraints variables (i.e FD, FS). Returning PROCCED will lead to an 
      infinite execution of BIwait built-in when it is used.
  */ 
  OZ_Return addSuspV(TaggedRef*, Suspendable* susp) {
    extVar2Var(this)->addSuspSVar(susp);
    return SUSPEND;
  }
  
  GeVarType getType(void) { return type; }

  // Method needed to clone pointed gecode variables.
  virtual Gecode::VarBase* clone(void) = 0;
 
  virtual bool In(TaggedRef x) = 0;
  virtual TaggedRef clone(TaggedRef v) = 0;

  virtual bool hasSameDomain(TaggedRef) = 0;

  int getUnifyC(void) { return unifyC; }

  /// \name Unification and Binding.
  //@{
  /**
     \brief Unify lPtr with rPtr.
  */
  OZ_Return unifyV(TaggedRef* lPtr, TaggedRef* rPtr);
  
  /**
     \brief Test whether x and y have an empty intersection.
     This method is called when unifying two variables and the flag am.inEqEq
     is on. If intersection is empty then there is enough information
     to make the test == fail.
  */
  virtual bool IsEmptyInter(TaggedRef *x, TaggedRef *y) = 0;
  
  /**
     \brief Unification in gecode is ensured by means of an "equality" 
     propagator. Equality must be defined according with the constraint
     system. For finite domains, the intersection between domains is enough.
     This method have to post a propagator in the gecode space that only 
     subsums when both variables get determined. It needs to ensure completeness,
     otherwise unification may work unexpectedly.
   */
  virtual void propagator(GenericSpace *s, GeVar* x,  GeVar* y) = 0;
  
  virtual TaggedRef newVar() = 0; 
  virtual bool intersect(TaggedRef x) = 0; 

  
  OZ_Return bindV(TaggedRef*, TaggedRef);
  virtual Bool validV(TaggedRef v) = 0;
  
  virtual Gecode::ModEvent bind(GenericSpace *s, GeVar *v, OZ_Term val) = 0;
  void test();
//@}
};


inline 
bool oz_isGeVar(OZ_Term t) {
  OZ_Term dt = OZ_deref(t);
  return oz_isExtVar(dt) && oz_getExtVar(dt)->getIdV() == OZ_EVAR_GEVAR;
}

template <class VarImp>
inline 
GeVar<VarImp>* get_GeVar(OZ_Term v) {
  OZ_Term ref = OZ_deref(v);
  Assert(oz_isGeVar(ref));
  ExtVar *ev = oz_getExtVar(ref);
  return static_cast<GeVar<VarImp>*>(ev);
}

inline
bool oz_isGeVar(OzVariable *v) {
  /*printf("Estoy en esta funcion oz_isGeVar con OzVariable \n"); fflush(stdout);
  if(! (v->getType() == OZ_VAR_EXT)) return false;
  ExtVar *vextVar = var2ExtVar(v);

  bool res = vextVar->getIdV() == OZ_EVAR_GEVAR;
  printf("Despues de esta mierda: %d \n",res); fflush(stdout);
  return res;*/
  if ( v->getType() != OZ_VAR_EXT ) return false;
  return var2ExtVar(v)->getIdV() == OZ_EVAR_GEVAR;
}

template <class VarImp>
inline
void checkGlobalVar(OZ_Term v) {
  // Why this comparison is made with ints?
  //cout<<"Inicio check: "<<oz_isInt(v)<<endl; fflush(stdout);
  Assert(oz_isGeVar(v));

  ExtVar *ev = oz_getExtVar(oz_deref(v));
  if (!oz_isLocalVar(ev)) {
    TaggedRef nlv = static_cast<GeVar<VarImp>*>(ev)->clone(v);

    ExtVar *varTmp = var2ExtVar(tagged2Var(oz_deref(nlv)));
    GeVar<VarImp> *gvar = static_cast<GeVar<VarImp>*>(varTmp);

    //meter al trail v [v]
    TaggedRef nlvAux = oz_deref(nlv);

    Assert(oz_isVar(nlvAux));
    oz_unify(v,nlv);
  }
}

// This Gecode propagator reflects a Gecode variable assignment inside
// Mozart.

template <class View, class VarImp>
class VarReflector :
  public Gecode::UnaryPropagator<View, Gecode::PC_GEN_ASSIGNED>
{
protected:
  int index;

public:
  VarReflector(GenericSpace* s, View v, int idx) :
    Gecode::UnaryPropagator<View, Gecode::PC_GEN_ASSIGNED>(s, v), index(idx) {}

  VarReflector(GenericSpace* s, bool share, VarReflector& p) :
    Gecode::UnaryPropagator<View, Gecode::PC_GEN_ASSIGNED>(s, share, p), index(p.index) {}

  virtual Gecode::Actor* copy(Gecode::Space* s, bool share) = 0;

  //virtual OZ_Term getVarRef(GenericSpace*) = 0;
  virtual OZ_Term getVarRef(GenericSpace* s) {return s->getVarRef(index); }
  virtual OZ_Term getVal() = 0;
  virtual bool IsDet() = 0;

  // this propagator should never fail
  Gecode::ExecStatus propagate(Gecode::Space* s){
    //printf("Variable determined by gecode....%d\n",index);fflush(stdout);
    
    OZ_Term ref = getVarRef(static_cast<GenericSpace*>(s));
    if (!oz_isGeVar(ref))
      return  Gecode::ES_SUBSUMED;

    GeVar<VarImp> *gv = get_GeVar<VarImp>(ref);
    
    //printf("GeVar.hh ExecStatus index=%d\n",index);fflush(stdout);
    GenericSpace *gs = static_cast<GenericSpace*>(s);        
    OZ_Term val = getVal();
    OZ_Return ret = OZ_unify(ref, val);
    if (ret == FAILED) return Gecode::ES_FAILED;
    gs->incDetermined();
    if (gv->hasDomReflector()) {
      //printf("decrementing space dom reflection\n");fflush(stdout);
      gs->decForeignProps();
    }
    gs->decUnifyProps(gv->getUnifyC());
      
    return Gecode::ES_SUBSUMED;
  }
};


/*
  This Gecode propagator reflects variable's domains changes to mozart.
  PC_GEN_MAX is used as propagation condition in order to make it generic
  for all variables.
*/
template <class View, Gecode::PropCond pc>
class VarInspector :
  public Gecode::UnaryPropagator<View, pc>
{
protected:
  int index;
  
public:
  VarInspector(GenericSpace* s, View v, int idx) :
    Gecode::UnaryPropagator<View, pc>(s, v), index(idx) { }

  VarInspector(GenericSpace* s, bool share, VarInspector& p) :
    Gecode::UnaryPropagator<View, pc>(s, share, p), index(p.index) {}

  virtual Gecode::Actor* copy(Gecode::Space* s, bool share) {
    return new (s) VarInspector(static_cast<GenericSpace*>(s), share, *this);
  }

  virtual OZ_Term getVarRef(GenericSpace* s) {return s->getVarRef(index); }

  // this propagator should never fail nor subsume
  Gecode::ExecStatus propagate(Gecode::Space* s) {
    // printf("Variable inspected from mozart ....%d\n",index);fflush(stdout);    
    OZ_Term ref = getVarRef(static_cast<GenericSpace*>(s));
   
    Assert(oz_isVarOrRef(ref));	
    OzVariable *var=extVar2Var(oz_getExtVar(oz_deref(ref)));
    
    if(oz_isGeVar(ref)) {
      OzVariable *var=extVar2Var(oz_getExtVar(oz_deref(ref)));
      SuspList **sl = var->getSuspListRef();
      oz_checkAnySuspensionList(sl, var->getBoardInternal(), pc_all);
    }

    return Gecode::ES_FIX;
  }
};

#include "GeVar.icc"
#endif
