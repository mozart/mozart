/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifndef __FAILHH
#define __FAILHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "dpBase.hh"
#include "comm.hh"
#include "genhashtbl.hh"
#include "interFault.hh"

class EntityInfo{
  friend class Tertiary;
public:
  Watcher *watchers;
  EntityCond entityCond;

  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS(EntityInfo);

  EntityInfo(Watcher *w){
    entityCond=ENTITY_NORMAL;
    watchers = w;}

  EntityInfo(EntityCond c){
    entityCond=c;
    watchers=NULL;}

  EntityInfo(){
    entityCond=ENTITY_NORMAL;
    watchers=NULL;}

  EntityCond getEntityCond(){
    return entityCond;}

  Bool addEntityCond(EntityCond ec){
    if((entityCond | ec) ==entityCond) return FALSE;
    entityCond |= ec;
    return TRUE;}

  void subEntityCond(EntityCond ec){
    entityCond &= ~ec;}

  EntityCond getSummaryWatchCond();

  void dealWithWatchers(TaggedRef,EntityCond);

  void gcWatchers();

  Watcher** getWatcherBase(){return &watchers;}
};

#define TWIN_GC    1
extern Twin *usedTwins;
extern Watcher* globalWatcher;
void gcGlobalWatcher();
void gcTwins();

class Twin{
friend class Watcher;
public:
  unsigned int flags;
  Twin *next;
  int cellCtr;
  int lockCtr;
  
  void* operator new(size_t size){
    Assert(size==16);
    return (Chain *)genFreeListManager->getOne_4();}        
  
  void free(){
    genFreeListManager->putOne_4((FreeListEntry*) this);}    

  Twin(){
    flags=0;
    cellCtr=0;
    lockCtr=0;
    next=usedTwins;
    usedTwins=this;}

  void setGCMark(){
    flags = TWIN_GC;}

  Bool hasGCMark(){
    return flags & TWIN_GC;}    

  void resetGCMark(){
    flags = 0;}    
};


class Watcher{
friend class Tertiary;
friend class EntityInfo;
public:
  TaggedRef proc;
  Watcher *next;
  Thread *thread; // thread of injector
  short kind;
  unsigned short watchcond;
  Twin* twin;

  USEHEAPMEMORY;

  NO_DEFAULT_CONSTRUCTORS(Watcher);

  Watcher(TaggedRef p,Thread* t,EntityCond wc, short k){
    proc=p;
    next=NULL;
    thread=t;
    watchcond=wc;
    twin=NULL;
    kind=k;}
  
  Bool matches(TaggedRef p,Thread* t,EntityCond wc, short k){
    if((k & WATCHER_INJECTOR)){
      if(p!=AtomAny){
	if(oz_deref(p)!=oz_deref(proc)) return FALSE;}}
    else{
      if(oz_deref(p)!=oz_deref(proc)) return FALSE;}      
    if(t!=thread) return FALSE;
    if(watchcond!=wc && wc!=ANY_COND) return FALSE;
    if(kind!=k) return FALSE;
    return TRUE;}

  Twin* cellTwin(){
    kind |= WATCHER_CELL;
    Assert(twin==NULL);
    twin = new Twin();
    return twin;}

  void lockTwin(Twin* tw){
    Assert(twin==NULL);    
    twin=tw;}

  Bool fire(){
    if(kind & WATCHER_CELL){
      if(twin->lockCtr>twin->cellCtr) return FALSE;
      twin->cellCtr++;
      return TRUE;}
    else{
      if(twin->cellCtr>twin->lockCtr) return FALSE;
      twin->lockCtr++;
      return TRUE;}}

  Bool isFired(){
    if(twin==NULL) return FALSE;
    if(twin->cellCtr>0) return TRUE;
    if(twin->lockCtr>0) return TRUE;    
    return FALSE;}

  Bool isRetry(){return kind & WATCHER_RETRY;} 
  Bool isPersistent(){return kind & WATCHER_PERSISTENT;}
  Bool isSiteBased(){return kind & WATCHER_SITE_BASED;}
  Bool isInjector(){return kind & WATCHER_INJECTOR;}
  Bool isCellPartObject(){return kind & WATCHER_CELL;}
  
  void setNext(Watcher* w){next=w;}

  Watcher* getNext(){return next;}

  void varInvokeInjector(TaggedRef t,EntityCond,TaggedRef);
  void invokeInjector(Tertiary* t,EntityCond,TaggedRef,Thread*,TaggedRef);
  void invokeWatcher(TaggedRef t,EntityCond);

  Thread* getThread(){Assert(thread!=NULL);return thread;}

  Bool isTriggered(EntityCond ec){
    if(ec & watchcond) return OK;
    return NO;}

  EntityCond getWatchCond(){return watchcond;}

  EntityCond getEffWatching();

};


void gcEntityInfoImpl(Tertiary *t);

EntityInfo* gcEntityInfoInternal(EntityInfo*);

inline Bool errorIgnore(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info == NULL) return OK;
  return NO;}

inline EntityCond getEntityCond(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info == NULL) return ENTITY_NORMAL;
  return info->getEntityCond();}

inline Bool maybeProblem(Tertiary* t){
  EntityInfo* info = t->getInfo();
  if (info == NULL) return FALSE;
  EntityCond ec=info->getEntityCond();
  if(ec != ENTITY_NORMAL) return TRUE;
  return FALSE;}

inline Watcher** getWatcherBase(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL) return NULL;
  if(info->watchers==NULL) return NULL;
  return &(info->watchers);}

void insertWatcherAtProxy(Tertiary *t, Watcher* w);
void insertWatcherAtManager(Tertiary *t, Watcher* w);

inline Bool someTempCondition(EntityCond ec){
  return ec & (TEMP_SOME|TEMP_FAIL|TEMP_ALL);}

inline Bool somePermCondition(EntityCond ec){
  return ec & (PERM_SOME|PERM_FAIL|PERM_ALL);}

inline Bool addEntityCond(Tertiary *t, EntityCond c){
  EntityInfo* info = t->getInfo();
  if(info==NULL){
    t->setInfo(new EntityInfo(c));
    return TRUE;}
  return info->addEntityCond(c);}

inline void subEntityCond(Tertiary *t, EntityCond c){
  EntityInfo* info = t->getInfo();
  Assert(info!=NULL);
  info->subEntityCond(c);}

inline Watcher *getWatchersIfExist(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL){return NULL;}
  return info->watchers;}

EntityCond askPart(Tertiary*, EntityCond);

inline EntityCond getSummaryWatchCond(Tertiary* t){
  if(t->getInfo()==NULL) return ENTITY_NORMAL;
  return t->getInfo()->getSummaryWatchCond();}

void entityProblem(Tertiary *t);
void deferEntityProblem(Tertiary *t);
void managerProbeFault(Tertiary *t, DSite*, int);
void proxyProbeFault(Tertiary *t, int);

TaggedRef listifyWatcherCond(EntityCond);
TaggedRef listifyWatcherCond(EntityCond,Tertiary*);

void entityProblem(Tertiary*);
void gcTwins();

void initProxyForFailure(Tertiary*);

OZ_Return DistHandlerInstall(SRecord*, TaggedRef,Bool &);
OZ_Return DistHandlerDeInstall(SRecord*, TaggedRef,Bool &);
Bool isWatcherEligible(Tertiary*);
Bool installGlobalWatcher(EntityCond,TaggedRef,int);


/**********************   DeferEvents   ******************/
enum DeferType{ 
  DEFER_PROXY_VAR_PROBLEM,
  DEFER_PROXY_TERT_PROBLEM,
  DEFER_MANAGER_PROBLEM,
  DEFER_ENTITY_PROBLEM};
    
class DeferElement{
public:
  DeferElement *next;
  TaggedRef  tert;
  TaggedRef  pvar;
  DSite     *site;
  DeferType  type;
  int        prob;
  DeferElement(){Assert(0);}

  void setTert(Tertiary * t) {
    tert = t ? makeTaggedConst(t) : makeTaggedNULL();
  }
  Tertiary * getTert(void) {
    return tert ? (Tertiary *) tagged2Const(tert) : (Tertiary *) NULL;
  }
  void init(DSite* s,DeferType dt, int pr, Tertiary* t){
    site=s; type=dt; prob=pr; 
    setTert(t);
  }

  void init(DeferType dt, int pr, Tertiary* t){
    site=NULL; type=dt; prob=pr; 
    setTert(t);
  }

  void init(DeferType dt, Tertiary* t){
    site=NULL; type=dt; prob= 0; 
    setTert(t);
  }

  void init(DeferType dt, int pr,TaggedRef v){
    site=NULL; type=dt; prob= pr; setTert((Tertiary *) NULL); pvar=v;
  }

};

extern DeferElement* DeferdEvents;
extern TaggedRef BI_defer;
void gcDeferEvents();
void deferProxyTertProbeFault(Tertiary*,int);
void deferProxyVarProbeFault(TaggedRef,int);


void maybeUnask(Tertiary*);
Bool isVariableSpec(SRecord*);

void varAdjustPOForFailure(int,EntityCond,EntityCond);

void triggerInforms(InformElem**,OwnerEntry*,int,EntityCond);
void triggerInformsOK(InformElem**,OwnerEntry*,int,EntityCond);

void maybeHandOver(EntityInfo*, TaggedRef);
void transferWatchers(Object *o);

Bool distHandlerInstallImpl(unsigned short,unsigned short,
				 Thread*,TaggedRef,TaggedRef);
Bool distHandlerDeInstallImpl(unsigned short,unsigned short,
				   Thread*,TaggedRef,TaggedRef);

void dealWithDeferredWatchers();

TaggedRef mkOp1(char*,TaggedRef);
TaggedRef mkOp2(char*,TaggedRef,TaggedRef);
TaggedRef mkOp3(char*,TaggedRef,TaggedRef,TaggedRef);

OZ_Return tertiaryFailHandle(Tertiary*, TaggedRef,EntityCond,TaggedRef);
Bool tertiaryFail(Tertiary*, EntityCond &, TaggedRef&);

/* __FAILHH */
#endif 


