/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Copyright:
 *    Per Brand, 1998
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
#pragma implementation "interFault.hh"
#endif

#include "base.hh"
#include "builtins.hh"
#include "interFault.hh"

Bool perdioInitialized=FALSE;

Bool translateWatcherCond(TaggedRef tr,EntityCond &ec){
  if(tr==AtomPermFail){
    ec |= PERM_FAIL;
    return TRUE;}
  if(tr== AtomTempFail){
    ec |= TEMP_FAIL;
    return TRUE;}
  SRecord *condStruct;
  if(oz_isSRecord(tr)){
    condStruct= tagged2SRecord(tr);}
  else{
    return FALSE;}
  if((condStruct->getLabel())!=AtomRemoteProblem) return FALSE;
  tr = condStruct->getArg(0);
  if(tr== AtomPermSome){
    ec |= PERM_SOME;
    return TRUE;}
  if(tr== AtomTempSome){
    ec |= TEMP_SOME;
    return TRUE;}
  if(tr== AtomPermAll){
    ec |= PERM_ALL;
    return TRUE;}
  if(tr== AtomTempAll){
    ec |= TEMP_ALL;
    return TEMP_ALL;}
  return FALSE;
}

OZ_Return translateWatcherConds(TaggedRef tr,EntityCond &ec){
  TaggedRef car,cdr;
  ec=ENTITY_NORMAL;
  cdr=tr;
  DerefVarTest(cdr);
  if(cdr==AtomAny){
    ec= ANY_COND;
    return PROCEED;}
  while(!oz_isNil(cdr)){
    if(!oz_isLTuple(cdr)){
      return IncorrectFaultSpecification;}
    car=tagged2LTuple(cdr)->getHead();
    cdr=tagged2LTuple(cdr)->getTail();
    DerefVarTest(car);
    DerefVarTest(cdr);
    if(!translateWatcherCond(car,ec))
      return IncorrectFaultSpecification;}
  if(ec == ENTITY_NORMAL) ec=UNREACHABLE;
  return PROCEED;
}

inline OZ_Return checkWatcherConds(EntityCond ec,EntityCond allowed){
  if((ec==ANY_COND)) return PROCEED;
  if((ec & ~allowed) != ENTITY_NORMAL) return IncorrectFaultSpecification;
  return PROCEED;}

#define FeatureTest(cond,tt,atom) { \
   tt = cond->getFeature(OZ_atom(atom)); \
   if(tt==0) {return IncorrectFaultSpecification;}}

OZ_Return checkRetry(SRecord *condStruct,short &kind){
  TaggedRef aux;

  aux = condStruct->getFeature(OZ_atom("prop"));
  if(aux==0) {
    return PROCEED;}
  DerefVarTest(aux);
  if(aux==AtomRetry){
    kind |= WATCHER_RETRY;    
    return PROCEED;}
  if(aux!=AtomSkip){
    return IncorrectFaultSpecification;}
  return PROCEED;
}

Bool isWatcherEligible(TaggedRef t){
  if(!oz_isConst(t)) return FALSE;
  switch(tagged2Const(t)->getType()){
  case Co_Object:
  case Co_Cell:
  case Co_Lock:
  case Co_Port: return TRUE;
  default: return FALSE;}
  Assert(0);
  return FALSE;
}

// the following should check for the existence of not allowed features 

OZ_Return distHandlerInstallHelp(SRecord *condStruct, 
		     EntityCond& ec,Thread* &th,TaggedRef &entity,short &kind){
  kind=0;
  ec=ENTITY_NORMAL;
  entity=0;
  th=NULL;

  TaggedRef aux;
  FeatureTest(condStruct,aux,"cond");

  OZ_Return ret = translateWatcherConds(aux,ec);
  if(ret!=PROCEED) return  ret;

  TaggedRef label=condStruct->getLabel();

  if(label==AtomInjector || label==AtomSafeInjector){
    kind |= (WATCHER_PERSISTENT|WATCHER_INJECTOR);

    FeatureTest(condStruct,aux,"entityType");  
    DerefVarTest(aux);
    if(aux==AtomAll) {
      entity=0;
      kind |= WATCHER_SITE_BASED;
      FeatureTest(condStruct,aux,"thread");  
      DerefVarTest(aux);
      if(aux!=AtomAll){return IncorrectFaultSpecification;}
      return checkRetry(condStruct,kind);}

    if(aux!=AtomSingle) {return IncorrectFaultSpecification;}
    FeatureTest(condStruct,aux,"entity");
    entity=aux;
    FeatureTest(condStruct,aux,"thread");  
    DerefVarTest(aux);
    if(aux==AtomAll){ 
      th=NULL;
      kind |= WATCHER_SITE_BASED;
      return checkRetry(condStruct,kind);}
    if(aux==AtomThis){
      th=oz_currentThread();
      return checkRetry(condStruct,kind);}      
    if(!oz_isThread(aux)) {return IncorrectFaultSpecification;}      
    th=oz_ThreadToC(aux);
    return checkRetry(condStruct,kind);}

  if(label==AtomSiteWatcher){
    FeatureTest(condStruct,aux,"entity");    
    entity=aux;
    return checkWatcherConds(ec,PERM_FAIL|TEMP_FAIL);}

  if(label==AtomNetWatcher) {
    FeatureTest(condStruct,aux,"entity");    
    entity=aux;
    return checkWatcherConds(ec,PERM_SOME|TEMP_SOME|PERM_ALL|TEMP_ALL);}

  if(label!=AtomWatcher) {return IncorrectFaultSpecification;}
  FeatureTest(condStruct,aux,"entity");    
  entity=aux;
  return checkWatcherConds(ec,
	 PERM_FAIL|TEMP_FAIL|PERM_SOME|TEMP_SOME|PERM_ALL|TEMP_ALL);
}


OZ_BI_define(BIinterDistHandlerInstall,2,1){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc0      = OZ_in(1);  

  NONVAR(c0, c);
  NONVAR(proc0,proc);
  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;
  short kind;
  EntityCond ec;
  Thread* th;
  TaggedRef entity;
  
  OZ_Return ret=distHandlerInstallHelp(condStruct,ec,th,entity,kind);
  if(ec==ANY_COND) return IncorrectFaultSpecification;
  if(ret!=PROCEED) return ret;
  if(ec == ANY_COND) return IncorrectFaultSpecification;
  if(kind & WATCHER_SITE_BASED) return IncorrectFaultSpecification;
  if(!oz_isAbstraction(proc)) 
    return IncorrectFaultSpecification;
  if(kind & WATCHER_INJECTOR) {
    if(tagged2Abstraction(proc)->getArity()!=3) 
      return IncorrectFaultSpecification;}
  else{
    if(tagged2Abstraction(proc)->getArity()!=2) 
      return IncorrectFaultSpecification;}
  Assert(entity!=0);
  if(!oz_isVarOrRef(oz_deref(entity))){
    if(!isWatcherEligible(oz_deref(entity))){
      OZ_RETURN(oz_bool(TRUE));}}
  if(perdioInitialized){
    if((*distHandlerInstall)(kind,ec,th,entity,proc)){
      OZ_RETURN(oz_bool(TRUE));}
    else{
      OZ_RETURN(oz_bool(FALSE));}}
  else{
    if(addDeferWatcher(kind,ec,th,entity,proc))
      OZ_RETURN(oz_bool(TRUE));
    OZ_RETURN(oz_bool(FALSE));}
}OZ_BI_end

OZ_BI_define(BIinterDistHandlerDeInstall,2,0){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc0      = OZ_in(1);  
  
  NONVAR(c0, c);
  NONVAR(proc0,proc);
  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;
  short kind;
  EntityCond ec;
  Thread* th;
  TaggedRef entity;
  
  OZ_Return ret=distHandlerInstallHelp(condStruct,ec,th,entity,kind);
  if(ret!=PROCEED) return ret;
  if(kind & WATCHER_SITE_BASED) return IncorrectFaultSpecification;
  Assert(entity!=0);
  if(!oz_isVarOrRef(oz_deref(entity))){
    if(!isWatcherEligible(oz_deref(entity))){
      OZ_RETURN(oz_bool(TRUE));}}
  if(perdioInitialized){
    if((*distHandlerDeInstall)(kind,ec,th,entity,proc)){
      OZ_RETURN(oz_bool(TRUE));}
    else{
      OZ_RETURN(oz_bool(FALSE));}}
  else{
    if(remDeferWatcher(kind,ec,th,entity,proc))
      OZ_RETURN(oz_bool(TRUE));
    OZ_RETURN(oz_bool(FALSE));}
}OZ_BI_end

DeferWatcher* deferWatchers;

void gCollectDeferWatchers(){
  DeferWatcher *newW,**base;
  if(deferWatchers==NULL) return;
  base=&(deferWatchers);
  while((*base)!=NULL){
    newW= (DeferWatcher*) oz_hrealloc((*base),sizeof(DeferWatcher));
    newW->gCollect();
    *base=newW;
    base= &(newW->next);}
  newW->next=NULL;}

void DeferWatcher::gCollect(){
  oz_gCollectTerm(proc,proc);
  thread=SuspToThread(thread->gCollectSuspendable());
  oz_gCollectTerm(entity,entity);}
  
    
Bool addDeferWatcher(short kind,EntityCond ec,Thread* th,TaggedRef entity,
		     TaggedRef proc){
  DeferWatcher** base= &deferWatchers;
  while((*base)!=NULL){
    if((*base)->preventAdd(kind,th,entity)) return FALSE;
    base= &((*base)->next);}
  *base=new DeferWatcher(kind,ec,th,entity,proc);
  return TRUE;}

Bool remDeferWatcher(short kind,EntityCond ec,Thread* th,TaggedRef entity,
		     TaggedRef proc){
  DeferWatcher** base= &deferWatchers;
  while((*base)!=NULL){
    if((*base)->isEqual(kind,ec,th,entity,proc)){
      *base=(*base)->next;
      return TRUE;}
    base= &((*base)->next);}
  return FALSE;}
  
Bool DeferWatcher::preventAdd(short k, Thread* th,TaggedRef e){
  if(!(k & WATCHER_INJECTOR)) return FALSE;
  if(th!=thread) return FALSE;
  if(e!=entity) return FALSE;
  return TRUE;}

Bool DeferWatcher::isEqual(short k,EntityCond w,
			      Thread* th,TaggedRef e,TaggedRef p){
  if(k & WATCHER_INJECTOR){
    if((th==thread) && (e==entity) &&
       ((p==proc) || (p==AtomAny)) && ((w==watchcond) || (w==ANY_COND)))
      return TRUE;
    return FALSE;}
  if((e==entity) &&
     (p==proc) && (w==watchcond))
    return TRUE;
  return FALSE;}
  






