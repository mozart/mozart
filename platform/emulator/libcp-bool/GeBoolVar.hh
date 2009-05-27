/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
* 			Andres Felipe Barco <anfelbar@univalle.edu.co>
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
class GeBoolVar : public GeVar {
protected:
  /// copy constructor
  GeBoolVar(GeBoolVar& gv) : GeVar(gv) {}

public:
  GeBoolVar(int index) :
    GeVar(index,T_GeBoolVar) {}
  
  /**
   * \brief Return a BoolView from the corresponding BoolVarImpl
   * associated with this GeBoolVar.
   */
  BoolView getBoolView(void){
    return BoolView(static_cast<BoolVarImp*>(getGSpace()->getVar(index)));
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
  virtual VarImpBase* clone(void);
  virtual bool intersect(TaggedRef x);
  
  virtual bool In(TaggedRef x);

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, GeVar *lgevar, GeVar *rgevar) {
    BoolView lbvar = (static_cast<GeBoolVar*>(lgevar))->getBoolView();
    BoolView rbvar = (static_cast<GeBoolVar*>(rgevar))->getBoolView();    
    rel(s,lbvar,IRT_EQ,rbvar);
  }

  virtual ModEvent bind(GenericSpace *s, GeVar *v, OZ_Term val) {
    int n = OZ_intToC(val);
    return getBoolView().eq(s,n);
  }

  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {
    return BoolView(static_cast<BoolVarImp*>(getGSpace()->getVar(index))).assigned();
  }
  
  virtual OZ_Term getVal(void) {
    return OZ_int(BoolView(static_cast<BoolVarImp*>(getGSpace()->getVar(index))).val());
  }

  virtual void ensureDomReflection(void) {
    postDomReflector<BoolView, BoolVarImp, PC_INT_DOM>(getGSpace(),this);
  }
  
  virtual int degree(void) { 
    return BoolView(static_cast<BoolVarImp*>(getGSpace()->getVar(index))).degree(); 
  }
};

/// Register the reflection propagators for BoolVars.
namespace {
  using namespace Gecode::Int;
  
  // ValReflector propagator
  Reflection::ActorRegistrar<ValReflector<BoolView> > BoolValRefl;

  // DomReflector
  Reflection::ActorRegistrar<DomReflector<BoolView,PC_INT_DOM> > BoolDomRefl;
}


inline 
OZ_Term new_GeBoolVar(int min, int max) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  BoolVar x(sp,min,max);
  GeBoolVar *nv = new GeBoolVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);

  //nv->ensureValReflection();
  postValReflector<BoolView,BoolVarImp>(sp,index);
  if (oz_onToplevel())
    oz_currentBoard()->getGenericSpace()->makeUnstable();
  
  return ref;
}

/**
   \brief Checks if the OZ_Term v represents a bool constraint
   variable in the store.
 */
inline 
bool OZ_isGeBoolVar(OZ_Term v) { 
  if(OZ_isInt(v)){
    if(OZ_intToC(v) == 0 || OZ_intToC(v) == 1)
      return true;
  }
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {

    GeVar *gv = static_cast< GeVar* >(oz_getExtVar(v_local));
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
  return static_cast<GeBoolVar*>(get_GeVar<BoolVarImp,PC_INT_DOM>(v,cgv));
}

/**
 * \briefd Retrieves a BoolView from an OZ_Term.
 * @param v must be a boolean domain variable (GeBoolVar)
 * Space stability is not afected with this function
 */
inline
BoolView get_BoolView(OZ_Term v){
  return get_GeBoolVar(v)->getBoolView();
}

/**
		\brief Checks if the term is a BoolOpType.
*/
inline
bool OZ_isBoolOpType(OZ_Term t){
  if(OZ_isInt(t)){
	  int v = OZ_intToC(t);
	  // see GBD.oz for details about number's mean.
	  return v == BOT_AND 
        || v == BOT_OR 
        || v == BOT_IMP 
        || v == BOT_EQV 
        || v == BOT_XOR ? true : false;
  }else return false; 
}

void gebvp_init(void);
#endif
