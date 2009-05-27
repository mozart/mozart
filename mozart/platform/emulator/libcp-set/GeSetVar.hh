/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
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


#ifndef __GECODE_SET_VAR_HH__
#define __GECODE_SET_VAR_HH__

#include "GeVar.hh"
#include "SetValue.hh"
#include "../libcp-bool/GeBoolVar.hh"
using namespace Gecode;
using namespace Gecode::Set;

const int lim_inf = Set::Limits::min;
const int lim_sup = Set::Limits::max;

//*****************************************************************************

#define GEOZ_FSETDESCR_SYNTAX						\
  "The syntax of a " GEOZ_EM_FSETDESCR " is:\n"				\
  "   set_descr   ::= simpl_descr | compl(simpl_descr)\n"		\
  "   simpl_descr ::= range_descr | nil | [range_descr+]\n"		\
  "   range_descr ::= integer | integer#integer\n"			\
  "   integer     ::= {" _GEOZ_EM_FSETINF ",...," _GEOZ_EM_FSETSUP "}"

//*****************************************************************************

// A GeSetVar interfaces an SetVar inside a GenericSpace.
class GeSetVar : public GeVar {
protected:
  /// copy constructor
  GeSetVar(GeSetVar& gv) : GeVar(gv) {}

public:
  GeSetVar(int index) :
    GeVar(index,T_GeSetVar) {}

  /**
   * \brief Return a SetView from the corresponding SetVarImpl
   * associated with this GeSetVar.
   */
  SetView getSetView(void) {
    return SetView(static_cast<SetVarImp*>(getGSpace()->getVar(index)));
  }

  //this is really working?
  virtual void printDomain(void) {
    Assert(false);
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
    SetView lsetvar = (static_cast<GeSetVar*>(lgevar))->getSetView();
    SetView rsetvar = (static_cast<GeSetVar*>(rgevar))->getSetView();    
    rel(s,lsetvar, Gecode::SRT_EQ,rsetvar);
  }

  // TODO: see whether when getSetVarInfo is used is it possible to use a view.
  virtual ModEvent bind(GenericSpace *s, GeVar *v, OZ_Term val) {    
    //printf("bind GeSetVar.hh");fflush(stdout);
    IntSetRanges tmpLB(SetValueM::tagged2SetVal(val)->getLBValue());
    IntSetRanges tmpUB(SetValueM::tagged2SetVal(val)->getUBValue());
    SetView ViewVar = getSetView();

    if(ViewVar.intersectI(s,tmpUB)!=Gecode::ME_GEN_FAILED)    
      return ViewVar.includeI(s,tmpLB);  
    return Gecode::ME_GEN_FAILED;
  }
  
  virtual Bool validV(OZ_Term v);
    
  // reflection mechanism 
  virtual bool assigned(void) {
    return SetView(static_cast<SetVarImp*>(getGSpace()->getVar(index))).assigned();
  }
  
  virtual OZ_Term getVal(void) {
    Set::SetView vv(static_cast<SetVarImp*>(getGSpace()->getVar(index)));
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
    return SetView(static_cast<SetVarImp*>(getGSpace()->getVar(index))).degree(); 
  }
};

/// Register the reflection propagators for SetVars.
namespace {
  using namespace Gecode::Set;
  
  // ValReflector propagator
  Reflection::ActorRegistrar<ValReflector<SetView> > SetValRefl;

  // DomReflector
  Reflection::ActorRegistrar<DomReflector<SetView,PC_SET_ANY> > SetDomRefl;
}

inline OZ_Term new_GeSetVar(IntSet glb,  IntSet lub) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar *x = new SetVar(sp,glb, lub);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x->var()), ref);

  if (oz_onToplevel())
    oz_currentBoard()->getGenericSpace()->makeUnstable();

  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  delete x;
  return ref;
}


inline OZ_Term new_GeSetVar_init() {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetVar *x = new SetVar(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());
  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x->var()), ref);
  
  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  if (oz_onToplevel())
    oz_currentBoard()->getGenericSpace()->makeUnstable();
  
  delete x;
  return ref;
}


/**
   \brief Checks if the OZ_Term v represents a set constraint
   variable in the store.
*/
inline 
bool OZ_isGeSetVar(OZ_Term v) { 
  OZ_Term v_local = OZ_deref(v);

  if(SetValueM::OZ_isSetValueM(v_local))
    return true;
  
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
 * \briefd Retrieves a SetView from an OZ_Term.
 * @param v must be a finite set variable (GeSetVar)
 * Space stability is not afected with this function
 */
inline
SetView get_SetView(OZ_Term v){
  return get_GeSetVar(v)->getSetView();
}


inline OZ_Term new_GeSetVarComp(OZ_Term V1) {
  
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  SetVar *x = new SetVar(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());

  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x->var()), ref);
  rel(sp, *x, SRT_CMPL, get_SetView(V1));
  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);
  delete x;
  return ref;
}

inline OZ_Term new_GeSetVarComplIn(OZ_Term V1,OZ_Term V2) {
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();

  SetVar x(sp);
  GeSetVar *nv = new GeSetVar(sp->getVarsSize());

  OzVariable* ov   = extVar2Var(nv);
  OZ_Term ref      = makeTaggedRef(newTaggedVar(ov));
  int index        = sp->newVar(static_cast<VarImpBase*>(x.var()), ref);
  
  
  rel(sp, get_SetView(V2),SOT_MINUS, get_SetView(V1), SRT_EQ , x);


  //nv->ensureValReflection();
  postValReflector<SetView,SetVarImp>(sp,index);

  return ref;
}

inline OZ_Return inc_GeSetVarVal(OZ_Term V1,  int val){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  if(Gecode::ME_GEN_FAILED==get_SetView(V1).include(sp,val))    
    return FAILED;
  else
    return PROCEED;
}

inline OZ_Return exc_GeSetVarVal(OZ_Term V1,  int val){
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  if(Gecode::ME_GEN_FAILED==get_SetView(V1).exclude(sp,val))    
    return FAILED;
  else
    return PROCEED;
}

inline OZ_Term isIn_GeSetVar(OZ_Term V1, int val, OZ_Term VBool)
{
  GenericSpace* sp = oz_currentBoard()->getGenericSpace();
  SetView Var = get_SetView(V1);  
  BoolView tmpBool(get_BoolView(VBool));
  //OZ_Term boolVar = new_GeBoolVar(0,1);
  if(Var.notContains(val))
    tmpBool.zero(sp);
  else{
    if(Var.contains(val))
      tmpBool.one(sp);    
    else
      Gecode::dom(sp,get_SetView(V1),Gecode::SRT_SUB,val,tmpBool);}
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
