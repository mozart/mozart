/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Zacharias El Banna (zeb@sics.se)
 *
 *  Contributors:
 * 
 *  Copyright:
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

#if defined(INTERFACE)
#pragma implementation "glue_entities.hh"
#endif


#include "dpMarshaler.hh"
#include "gname.hh"
#include "unify.hh"
#include "engine_interface.hh"
#include "glue_entities.hh"
#include "glue_tables.hh"
#include "glue_mediators.hh"

/*********************** LAZY VAR **************************/

void LazyVar::marshal(ByteBuffer *bs, Bool hasIndex){ Assert(0);}

// mm2: deep as future!
// kost@ : a masterpiece comment...
// cs: not bad either...
OZ_Return LazyVar::bindV(TaggedRef *lPtr, TaggedRef r)
{
  return oz_addSuspendVarList(lPtr);
}

// mm2: deep as future!
OZ_Return LazyVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  return oz_var_bind(tagged2Var(*rPtr),rPtr,makeTaggedRef(lPtr));
}

OZ_Return LazyVar::addSuspV(TaggedRef * v, Suspendable * susp)
{
  addSuspV(v,susp);
  if (!requested) {
    requested = 1;
    sendRequest(v);
  }
  return SUSPEND;
}

void LazyVar::gCollectRecurseV(void)
{
  Assert(0);
}

void LazyVar::disposeV()
{
  Assert(0);
}

OZ_Term LazyVar::statusV()
{
  SRecord *t = SRecord::newSRecord(AtomDet, 1);
  t->setArg(0, AtomObject);
  return makeTaggedSRecord(t);
}

VarStatus LazyVar::checkStatusV()
{
  return EVAR_STATUS_DET;
}
  


TaggedRef LazyVar::getTaggedRef() 
{
  return static_cast<RefMediator*>(e_name)->getRef();
}

/*********************** PROXY VAR **************************/

TaggedRef ProxyVar::getTaggedRef(){
  return static_cast<RefMediator*>(e_name)->getRef();}


OZ_Return ProxyVar::addSuspV(TaggedRef *, Suspendable * susp)
{
  extVar2Var(this)->addSuspSVar(susp);
  return SUSPEND;
}

void ProxyVar::gCollectRecurseV(void)
{ 
    if (status) {
    oz_gCollectTerm(status,status);
  }
  static_cast<RefMediator*>(e_name)->derefPtr(); 
} 

// Comes from glue_entityOpImpl
OZ_Return distVarDoUnify(ProxyVar*,ProxyVar*, TaggedRef *, TaggedRef);
bool distVarDoBind(ProxyVar* pv, TaggedRef *lPtr, TaggedRef r);

OZ_Return ProxyVar::bindV(TaggedRef *lPtr, TaggedRef r){
  if (oz_isLocalVar(this)) {
    /* 
       Here was a failure check done. If the variable had
       experienced any kinds of faults, this was checked againts
       the fault-reaction-strategies.
    */
    if(distVarDoBind(this, lPtr, r)){
      oz_bindLocalVar(extVar2Var(this),lPtr,r);
      return PROCEED; 
    }
    else
      return oz_addSuspendVarList(lPtr);
  } 
  OZ_error("We dont bind deep shit any longer");
  return PROCEED;
}

void ProxyVar::redoStatus(TaggedRef val, TaggedRef status)
{
  OZ_unifyInThread(status, oz_status(val));
}



OZ_Term ProxyVar::statusV()
{
  /* Hack, I dont understand the semantics of status, yet... */ 
  printf("Warning, checking status\n");
  if(status ==0){status= (OZ_Term) 0;}
  return status;
}

VarStatus ProxyVar::checkStatusV()
{
  return EVAR_STATUS_UNKNOWN;
}

ExtVar* ProxyVar::gCollectV(){
  e_name->engGC(ENGINE_GC_PRIMARY); 
  return new ProxyVar(*this);
}


ExtVar* ObjectVar::gCollectV(){
  e_name->engGC(ENGINE_GC_PRIMARY); 
  return new ObjectVar(*this);
}

inline
OZ_Return ProxyVar::unifyV(TaggedRef *lPtr, TaggedRef *rPtr)
{
  TaggedRef rVal = *rPtr;

  if (!oz_isExtVar(rVal)) {
    /* The target for the binding was a local var or a term */
    if (oz_isFree(rVal))  {
      /* We tried to perfom a dist to local bind. By doing it 
	 in the oposit order we can perfom it localy. Remember that 
	 there is no problem binding a localy free variable to 
	 any data, including dist vars. */
      return oz_var_bind(tagged2Var(rVal),rPtr,makeTaggedRef(lPtr));
    } else {
      /* Binding the dist var to a term. The ptr is casted using a macro. */
      return bindV(lPtr,makeTaggedRef(rPtr));
    }
  }
  /* The variable was an Extvar; hence, a possible dist var binding 
     can happend */
  
  ExtVar *rVar = oz_getExtVar(rVal);
  int rTag=rVar->getIdV();
  if (rTag!=OZ_EVAR_PROXY) {
    /* Some nondist-junk. Bind-it */ 
    return bindV(lPtr,makeTaggedRef(rPtr));
  }
  
  // Note: for distr. variables: isLocal == onToplevel
  if (oz_isLocalVar(this)) {
    // Calls the routine for deducing the binding order. 
    // The bind op that will be perfomed by the Manager
    // needs a representation of the targ
    distVarDoUnify(this, (ProxyVar*)rVar, lPtr, OZ_cons(makeTaggedRef(rPtr), makeTaggedRef(lPtr)));
    // Since the order is unknown from this level I choose
    // to suspend on both variables. 
		  
    return oz_addSuspendVarList2(makeTaggedRef(lPtr),makeTaggedRef(rPtr));
  }
  OZ_error("We dont let non local vars out");
  return SUSPEND; 
}


/*
  Reinserted during the transition into a complete model. 
  Erik
  
*/ 
VarKind classifyVar(TaggedRef* tPtr)
{ 
  TaggedRef tr = *tPtr;
  if (oz_isExtVar(tr)) {
    ExtVarType evt = oz_getExtVar(tr)->getIdV();
    switch (evt) {
    case OZ_EVAR_PROXY:
      return (VAR_PROXY);
    case OZ_EVAR_LAZY:
      return (VAR_LAZY);
    default:
      Assert(0);
      return (VAR_PROXY);
    }
  } else if (oz_isFree(tr)) {
    return (VAR_FREE);
  } else if (oz_isReadOnly(tr)) {
    return (VAR_READONLY);
  } else {
    return (VAR_KINDED);
  }
  Assert(0);
}

// bmc: triggerVariable is not needed anymore because Futures has been
// replaced by ByNeed and ReadOnly, so now, the transient entity from
// the dss take care of that kind of entities.

/*********************** OBJECT VAR **************************/

//TaggedRef newObjectProxy(int bi, GName *gnobj, TaggedRef cl){
//  return (TaggedRef) NULL;
//}

LazyType ObjectVar::getLazyType(){
  return (LT_OBJECT);
}

void ObjectVar::sendRequest(TaggedRef *v){
  Assert(0);
  //if(isObjectClassAvail()) 
  //lazyVarFetch(this, v,oz_true());
  //else
  //lazyVarFetch(this, v,oz_false());
}

void ObjectVar::gCollectRecurseV(void){
  OZ_warning("The address object of ObjectVars must be collected");
  gCollectGName(gClass);
  gCollectGName(gname);
}

void ObjectVar::disposeV(){
  disposeS();
  // PER-LOOK
  // kost@ : ... so what? found something?
  // Don't touch gname, since it appears in the object itself!!!
  freeListDispose(sizeof(ObjectVar));
}

void ObjectVar::transfer(TaggedRef ObjRef, TaggedRef *Vptr){
  Assert(isObjectClassAvail());
  
  /* We'll have to bind the class */ 
  Assert(oz_isObject(ObjRef));
  Object *o = (Object *) tagged2Const(ObjRef);

  o->setClassTerm(oz_deref(oz_findGName(gClass)));
  
  oz_bindLocalVar(extVar2Var(this), Vptr, ObjRef);
  e_name->mkPassiveRef();
}

GName* ObjectVar::getGNameClass()  {
  return gClass;
}

Bool ObjectVar::isObjectClassAvail(){
  return (oz_findGName(gClass) != (TaggedRef) 0); 
}
