/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

EntityCond translateWatcherCond(TaggedRef tr){
  if(tr==AtomPermBlocked)
    return PERM_BLOCKED;
 if(tr== AtomBlocked)
    return PERM_BLOCKED|TEMP_BLOCKED;
  if(tr== AtomPermWillBlock)
    return PERM_ME;
  if(tr== AtomWillBlock)
    return TEMP_ME|PERM_ME;
  if(tr== AtomPermSome)
    return PERM_SOME;
  if(tr== AtomSome)
    return TEMP_SOME|PERM_SOME;
  if(tr== AtomPermAll)
    return PERM_ALL;
  if(tr== AtomAll)
    return TEMP_ALL|PERM_ALL;
  Assert(0);
  return 0;
}

OZ_Return translateWatcherConds(TaggedRef tr,EntityCond &ec){
  TaggedRef car,cdr;
  ec=ENTITY_NORMAL;

  cdr=tr;
  DerefVarTest(cdr);
  while(!oz_isNil(cdr)){
    if(!oz_isLTuple(cdr)){
      return IncorrectFaultSpecification;}
    car=tagged2LTuple(cdr)->getHead();
    cdr=tagged2LTuple(cdr)->getTail();
    DerefVarTest(car);
    DerefVarTest(cdr);
    ec |= translateWatcherCond(car);}
  if(ec == ENTITY_NORMAL) ec=UNREACHABLE;
  return PROCEED;
}

inline OZ_Return checkWatcherConds(EntityCond ec,EntityCond allowed){
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
    ret=checkWatcherConds(ec,PERM_BLOCKED|TEMP_BLOCKED);
    if(ret!=PROCEED) return ret;
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
    return checkWatcherConds(ec,PERM_BLOCKED|TEMP_BLOCKED|PERM_ME|TEMP_ME);}

  if(label!=AtomNetWatcher) {return IncorrectFaultSpecification;}
  FeatureTest(condStruct,aux,"entity");
  entity=aux;
  return checkWatcherConds(ec,PERM_SOME|TEMP_SOME|PERM_ALL|TEMP_ALL);
}

OZ_BI_define(BIinterDistHandlerInstall,2,1){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc      = OZ_in(1);

  NONVAR(c0, c);
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
  if(perdioInitialized){
    if((*distHandlerInstall)(kind,ec,th,entity,proc))
      {OZ_RETURN(oz_bool(TRUE));}
    OZ_RETURN(oz_bool(FALSE));}
  if(addDeferWatcher(kind,ec,th,entity,proc))
    OZ_RETURN(oz_bool(TRUE));
  OZ_RETURN(oz_bool(FALSE));
}OZ_BI_end

OZ_BI_define(BIinterDistHandlerDeInstall,2,0){
  OZ_Term c0        = OZ_in(0);
  OZ_Term proc      = OZ_in(1);

  NONVAR(c0, c);
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
  if(perdioInitialized){
    if((*distHandlerDeInstall)(kind,ec,th,entity,proc)) {
      OZ_RETURN(oz_bool(TRUE));}
    OZ_RETURN(oz_bool(FALSE));}
  if(remDeferWatcher(kind,ec,th,entity,proc))
    OZ_RETURN(oz_bool(TRUE));
  OZ_RETURN(oz_bool(FALSE));
}OZ_BI_end

DeferWatcher* deferWatchers;

void gcDeferWatchers(){
  DeferWatcher *newW,**base;
  DeferWatcher* ow= deferWatchers;
  if(ow==NULL) return;
  deferWatchers=NULL;
  base=&(deferWatchers);
  while(ow!=NULL){
    newW= (DeferWatcher*) OZ_hrealloc(ow,sizeof(DeferWatcher));
    newW->gc();
    *base=newW;
    base= &(newW->next);}
  newW->next=NULL;}

void DeferWatcher::gc(){
  OZ_collectHeapTerm(proc,proc);
  thread=thread->gcThread();
  OZ_collectHeapTerm(entity,entity);}


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
       (p==proc) && (w==watchcond))
      return TRUE;
    return FALSE;}
  if((e==entity) &&
     (p==proc) && (w==watchcond))
    return TRUE;
  return FALSE;}
