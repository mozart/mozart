/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
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

#ifndef __GECODE_SET_VAR_HH__
#define __GECODE_SET_VAR_HH__

#include "GeVar.hh"
#include "SetValue.hh"
#include "../libcp-bool/GeBoolVar.hh"
using namespace Gecode;
using namespace Gecode::Set;

// A GeSetVar interfaces an SetVar inside a GenericSpace.
class GeSetVar : public GeVar {
protected:
  /// copy constructor
  GeSetVar(GeSetVar& gv) : GeVar(gv) {}

public:
  GeSetVar(int index) :
    GeVar(index,T_GeSetVar) {}

  //TODO: Be carefull this function should return a view and not a variable
  SetVar& getSetVar(void) {
    Set::SetView sv(static_cast<SetVarImp*>(getGSpace()->getVar(index)));
    SetVar *tmp = new SetVar(sv);
    return (*tmp);
  }

  // the returned reference should be constant
  SetVar& getSetVarInfo() {
    Set::SetView sv(static_cast<SetVarImp*>(getGSpace()->getVarInfo(index)));
    SetVar *tmp = new SetVar(sv);
    return (*tmp);
  }

  virtual void printDomain(void) {
    Assert(false);
    SetVar tmp = getSetVarInfo();
  }
  
  /**
     \brief Put in out a text representation of the variable.
  */
  void toStream(ostream &out);

  GeVarType type() { return getType(); }

  virtual ExtVar* gCollectV() { return new GeSetVar(*this); }
  virtual ExtVar* sCloneV() { return new GeSetVar(*this); }

  virtual OZ_Term       statusV();
  //  virtual void printStreamV(ostream &out,int depth);
  virtual VarImpBase* clone(void);
  virtual bool intersect(TaggedRef x);


  virtual bool In(TaggedRef x);

  //clone to create local variable from propagators.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, GeVar *lgevar, GeVar *rgevar) {
    SetVar& lsetvar = (static_cast<GeSetVar*>(lgevar))->getSetVarInfo();
    SetVar& rsetvar = (static_cast<GeSetVar*>(rgevar))->getSetVarInfo();    
    rel(s,lsetvar, Gecode::SRT_EQ,rsetvar);
  }

  // TODO: see whether when getSetVarInfo is used is it possible to use a view.
  virtual ModEvent bind(GenericSpace *s, GeVar *v, OZ_Term val) {    
    printf("bind GeSetVar.hh");fflush(stdout);
    IntSetRanges tmpLB(SetValueM::tagged2SetVal(val)->getLBValue());
    IntSetRanges tmpUB(SetValueM::tagged2SetVal(val)->getUBValue());
    SetView ViewVar(getSetVarInfo());    
    if(ViewVar.intersectI(s,tmpUB)!=Gecode::ME_GEN_FAILED)    
      return ViewVar.includeI(s,tmpLB);  
    return Gecode::ME_GEN_FAILED;
  }
  
  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {
    return SetView(static_cast<SetVarImp*>(getGSpace()->getVarInfo(index))).assigned();
  }
  
  virtual OZ_Term getVal(void) {
    Set::SetView vv(static_cast<SetVarImp*>(getGSpace()->getVarInfo(index)));
    Set::GlbRanges<Set::SetView> tmp(vv);
    Set::LubRanges<Set::SetView> tmp2(vv);
    IntSet valGlb(tmp);
    IntSet valLub(tmp2);
    IntSet card(vv.cardMin(),vv.cardMax());    
    return makeTaggedExtension(new SetValueM(valGlb, valLub, card));
  }
  
  virtual void ensureDomReflection(void) {
    postDomReflector<SetView, SetVarImp, PC_SET_ANY>(getGSpace(),this);
  }

  virtual int degree(void) { 
    SetView vi(static_cast<SetVarImp*>(getGSpace()->getVarInfo(index))); 
    return vi.degree(); 
  }
};


inline OZ_Term new_GeSetVar(IntSet glb,  IntSet lub) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar x(sp,glb, lub);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);

  if (oz_onToplevel())
    oz_currentBoard()->getGenericSpace()->makeUnstable();

  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  return ref;
}


inline OZ_Term new_GeSetVar_init() {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar x(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  
  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  if (oz_onToplevel())
    oz_currentBoard()->getGenericSpace()->makeUnstable();

  return ref;
}


/**
   \brief Checks if the OZ_Term v represents a set constraint
   variable in the store.
*/
inline 
bool OZ_isGeSetVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);
  if (oz_isGeVar(v_local)) {
    GeVar *gv = static_cast< GeVar*>(oz_getExtVar(v_local));
    return gv->getType() == T_GeSetVar;
  }
  return false;
}

// get the GeSetVar inside the OZ_Term v
/**
   \brief Retrieves a GeIntVar from an OZ_Term. cgv parameter indicates
   if checking for globality needs to be performed.
*/
inline
GeSetVar* get_GeSetVar(OZ_Term v, bool cgv = true) {
  Assert(OZ_isGeSetVar(v));
  return static_cast<GeSetVar*>(get_GeVar<SetVarImp,PC_SET_ANY>(v,cgv));
}

/**
   \brief Retrieve gecode variable from an OZ_Term afecting 
   space stability. A call to this method will make the gecode
   space unstable.
*/
inline SetVar& get_SetVar(OZ_Term v) {
  return get_GeSetVar(v)->getSetVar();
}

/**
   \brief Retrieve gecode variable from an OZ_Term without afecting 
   space stability. A call to this method will not make the gecode
   space unstable.
*/
inline SetVar& get_SetVarInfo(OZ_Term v) {
  return get_GeSetVar(v,false)->getSetVarInfo();
}



inline OZ_Term new_GeSetVarComp(OZ_Term V1) {
  
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  SetVar x(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());

  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  rel(sp, x, SRT_CMPL, get_SetVar(V1));
  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);
  return ref;
}

inline OZ_Term new_GeSetVarComplIn(OZ_Term V1,OZ_Term V2) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  SetVar x(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());

  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  
  
  rel(sp, get_SetVar(V2),SOT_MINUS, get_SetVar(V1), SRT_EQ , x);


  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  return ref;
}

inline OZ_Return inc_GeSetVarVal(OZ_Term V1,  int val){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var(get_SetVar(V1));
  if(Gecode::ME_GEN_FAILED==Var.include(sp,val))    
    return FAILED;
  else
    return PROCEED;
}

inline OZ_Return exc_GeSetVarVal(OZ_Term V1,  int val){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var(get_SetVar(V1));  
  if(Gecode::ME_GEN_FAILED==Var.exclude(sp,val))    
    return FAILED;
  else
    return PROCEED;
}

inline OZ_Term isIn_GeSetVar(OZ_Term V1, int val, OZ_Term VBool)
{
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var(get_SetVar(V1));  
  BoolView tmpBool(get_BoolVar(VBool));
  //OZ_Term boolVar = new_GeBoolVar(0,1);
  //BoolView tmpBool(get_BoolVar(boolVar));
  if(Var.notContains(val))
    tmpBool.zero(sp);
  else{
    if(Var.contains(val))
      tmpBool.one(sp);    
    else
      Gecode::dom(sp,get_SetVar(V1),Gecode::SRT_SUB,val,get_BoolVar(VBool));}
  return VBool;
}

/**
   \brief Checks if the term is a SetRelType.
*/
inline
bool OZ_isSetRelType(OZ_Term t){
  int v = OZ_intToC(t);
  return v == SRT_EQ 
    || v == SRT_NQ
    || v == SRT_SUB
    || v == SRT_SUP 
    || v == SRT_DISJ
    || v == SRT_CMPL ? true : false;
}

/**
   \brief Checks if the term is a SetOpType.
*/
inline
bool OZ_isSetOpType(OZ_Term t){
  int v = OZ_intToC(t);
  return v == SOT_UNION 
    || v == SOT_DUNION
    || v == SOT_INTER
    || v == SOT_MINUS ? true : false;
}

void gesvp_init(void);
#endif
