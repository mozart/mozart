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

#ifndef __GECODE_SET_VAR_HH__
#define __GECODE_SET_VAR_HH__

#include "GeVar.hh"
#include "SetValue.hh"
#include "../libcp-bool/GeBoolVar.hh"
using namespace Gecode;
using namespace Gecode::Int;
using namespace Gecode::Set;

typedef GeVar<SetVarImp,PC_SET_ANY> GeVar_Set;

// A GeSetVar interfaces an SetVar inside a GenericSpace.
class GeSetVar : public GeVar_Set {
protected:
  /// copy constructor
  GeSetVar(GeSetVar& gv) :
    GeVar_Set(gv) {}

public:
  GeSetVar(int index) :
    GeVar_Set(index,T_GeSetVar) {}

  SetVar& getSetVar(void) {
    GeView<Set::SetVarImp> iv(getGSpace()->getVar(index));
    Set::SetView *vv = reinterpret_cast<Set::SetView*>(&iv);
    SetVar *tmp = new SetVar(*vv);
    return (*tmp);
  }

  // the returned reference should be constant
  SetVar& getSetVarInfo() {
    GeView<Set::SetVarImp> iv(getGSpace()->getVarInfo(index));
    Set::SetView *vv = reinterpret_cast<Set::SetView*>(&iv);
    SetVar *tmp = new SetVar(*vv);
    return (*tmp);
  }

  virtual void printDomain(void) {
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

  //clone para crear variable local desde los propagadores.
  virtual TaggedRef clone(TaggedRef v);

  virtual bool hasSameDomain(TaggedRef v);

  virtual bool IsEmptyInter(TaggedRef* var1, TaggedRef* var2);

  virtual TaggedRef newVar(void);

  virtual void propagator(GenericSpace *s, 
			  GeVar_Set *lgevar,
			  GeVar_Set *rgevar) {
    SetVar& lsetvar = (static_cast<GeSetVar*>(lgevar))->getSetVarInfo();
    SetVar& rsetvar = (static_cast<GeSetVar*>(rgevar))->getSetVarInfo();    
    rel(s,lsetvar, Gecode::SRT_EQ,rsetvar);
  }

  virtual ModEvent bind(GenericSpace *s, 
			GeVar_Set *v, 
			OZ_Term val) {    
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
    GeView<Set::SetVarImp> iv(getGSpace()->getVarInfo(index));
    Set::SetView *vv = reinterpret_cast<Set::SetView*>(&iv);
    return vv->assigned();
  }
  
  virtual OZ_Term getVal(void) {
    GeView<Set::SetVarImp> iv(getGSpace()->getVarInfo(index));
    Set::SetView *vv = reinterpret_cast<Set::SetView*>(&iv);
    Set::GlbRanges<Set::SetView> tmp(*vv);
    Set::LubRanges<Set::SetView> tmp2(*vv);
    IntSet valGlb(tmp);
    IntSet valLub(tmp2);
    IntSet card(vv->cardMin(),vv->cardMax());    
    return makeTaggedExtension(new SetValueM(valGlb, valLub, card));
  }
  
  virtual void serialize(Gecode::Reflection::VarMap &vmp) {
    // TODO: something must be replaced with other thing, maybe the index in the vector.
    vmp.put(getGSpace(),getSetVarInfo(),"something");
  }
};


inline OZ_Term new_GeSetVar(IntSet glb,  IntSet lub) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar x(sp,glb, lub);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  nv->ensureValReflection();
  return ref;
}


inline OZ_Term new_GeSetVar_init() {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar x(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  nv->ensureValReflection();
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
    GeVar_Set *gv = 
      static_cast< GeVar_Set * >(oz_getExtVar(v_local));
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
  nv->ensureValReflection();
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


  nv->ensureValReflection();
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



inline OZ_Return cardInt_GeSetVar(OZ_Term V1,  int min,  int max){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var(get_SetVar(V1));  
  if(Gecode::ME_GEN_FAILED==Var.cardMin(sp,min))    
    return FAILED;
  else{
    if(Gecode::ME_GEN_FAILED==Var.cardMax(sp,max))    
      return FAILED;
    return PROCEED;
  }
}

inline OZ_Return card_GeSetVarVal(OZ_Term V1,  int val){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var(get_SetVar(V1));  
  if(Gecode::ME_GEN_FAILED==Var.cardMin(sp,val))    
    return FAILED;
  else{
    if(Gecode::ME_GEN_FAILED==Var.cardMax(sp,val))    
      return FAILED;
    return PROCEED;
  }
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


void module_init_gesetvar(void);
#endif
