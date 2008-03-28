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


typedef GeVar<BoolVarImp,PC_INT_DOM> GeVar_Bool;


// A GeBoolVar interfaces an IntVar inside a GenericSpace.
class GeBoolVar : public GeVar_Bool {
protected:
  /// copy constructor
  GeBoolVar(GeBoolVar& gv) :
    GeVar_Bool(gv) {}


public:
  GeBoolVar(int index) :
    GeVar_Bool(index,T_GeBoolVar) {}



  BoolVar& getBoolVar(void) {
    GeView<Int::BoolVarImp> iv(getGSpace()->getVar(index));
    Int::BoolView *vv = reinterpret_cast<Int::BoolView*>(&iv);
    BoolVar *tmp = new BoolVar(*vv);

    return (*tmp);
  }

  // the returned reference should be constant

  BoolVar& getBoolVarInfo() {
    GeView<Int::BoolVarImp> iv(getGSpace()->getVarInfo(index));
    Int::BoolView *vv = reinterpret_cast<Int::BoolView*>(&iv);
    BoolVar *tmp = new BoolVar(*vv);

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
  virtual VarImpBase* clone(void);
  virtual bool intersect(TaggedRef x);
  
  virtual bool In(TaggedRef x);

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, 

			  GeVar_Bool *lgevar,
			  GeVar_Bool *rgevar) {
    BoolVar& lbvar = (static_cast<GeBoolVar*>(lgevar))->getBoolVarInfo();
    BoolVar& rbvar = (static_cast<GeBoolVar*>(rgevar))->getBoolVarInfo();    
    rel(s,lbvar,IRT_EQ,rbvar);
  }

  virtual ModEvent bind(GenericSpace *s, 

			GeVar_Bool *v, 
			OZ_Term val) {
    int n = OZ_intToC(val);

    return Int::BoolView(getBoolVarInfo()).eq(s,n);

  }

  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {

    GeView<Int::BoolVarImp> iv(getGSpace()->getVarInfo(index));
    Int::BoolView *vv = reinterpret_cast<Int::BoolView*>(&iv);

    return vv->assigned();
  }
  
  virtual OZ_Term getVal(void) {

    GeView<Int::BoolVarImp> iv(getGSpace()->getVarInfo(index));
    Int::BoolView *vv = reinterpret_cast<Int::BoolView*>(&iv);
    return OZ_int(vv->val());
  }

  virtual void reflect(Reflection::VarMap &vmp, Support::Symbol &p,
		       bool registerOnly = false) {
    vmp.put(getGSpace(),getBoolVarInfo(),p, registerOnly);
  }
};


inline 
OZ_Term new_GeBoolVar(int min, int max) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  BoolVar x(sp,min,max);
  GeBoolVar *nv = new GeBoolVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);

  nv->ensureValReflection();

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
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {

    GeVar<Int::BoolVarImp,PC_INT_DOM> *gv = 
      static_cast< GeVar_Bool * >(oz_getExtVar(v_local));
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
   \brief Retrieve gecode variable from an OZ_Term afecting 
   space stability. A call to this method will make the gecode
   space unstable.
*/
inline BoolVar& get_BoolVar(OZ_Term v) {
    return get_GeBoolVar(v)->getBoolVar();
}

/**
   \brief Retrieve gecode variable from an OZ_Term without afecting 
   space stability. A call to this method will not make the gecode
   space unstable.
*/
inline BoolVar& get_BoolVarInfo(OZ_Term v) {
  return get_GeBoolVar(v,false)->getBoolVarInfo();
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
