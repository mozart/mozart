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

#ifndef __FAILHH
#define __FAILHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "dpBase.hh"
#include "comm.hh"

typedef unsigned int FaultInfo;

enum WatcherKind{
  HANDLER    = 1,
  WATCHER    = 2,
  RETRY      = 4,
  PERSISTENT = 8
};

enum EntityCondFlags{
  ENTITY_NORMAL = 0,
  PERM_BLOCKED  = 2,       
  TEMP_BLOCKED  = 1,
  PERM_ALL      = 4,
  TEMP_ALL      = 8,
  PERM_SOME     = 16,
  TEMP_SOME     = 32,
  PERM_ME       = 64,
  TEMP_ME       = 128,
  OBJECT_PART   = 256
};

typedef unsigned int EntityCond;
extern TaggedRef BI_probe;

class EntityInfo{
  friend class Tertiary;
public:
  Watcher *watchers;
  short entityCond;
  short managerEntityCond;
  Tertiary* object;

  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS(EntityInfo);
  EntityInfo(Watcher *w){
    entityCond=ENTITY_NORMAL;
    managerEntityCond=ENTITY_NORMAL;
    object   = NULL;
    watchers = w;}
  EntityInfo(EntityCond c){
    entityCond=c;
    managerEntityCond=ENTITY_NORMAL;    
    object = NULL;
    watchers=NULL;}
  EntityInfo(EntityCond c,EntityCond d){
    entityCond=c;
    managerEntityCond=d;
    object = NULL;
    watchers=NULL;}
  EntityInfo(Tertiary* o){
    entityCond=ENTITY_NORMAL;
    managerEntityCond=ENTITY_NORMAL;
    object = o;
    watchers=NULL;}
  void gcWatchers();
};

class Watcher{
friend class Tertiary;
friend class EntityInfo;
public:
  TaggedRef proc;
  Watcher *next;
  Thread *thread;
  short kind;
  short watchcond;

  USEHEAPMEMORY;

  NO_DEFAULT_CONSTRUCTORS(Watcher);
  // handler
  Watcher(TaggedRef p,Thread* t,EntityCond wc){
    proc=p;
    next=NULL;
    thread=t;
    kind=HANDLER;
    Assert((wc==PERM_BLOCKED) || (wc==TEMP_BLOCKED|PERM_BLOCKED));
    watchcond=wc;}

// watcher
  Watcher(TaggedRef p,EntityCond wc){
    proc=p;
    next=NULL;
    thread=NULL;
    kind=WATCHER;
    watchcond=wc;}

  Bool isHandler(){ return kind&HANDLER;}
  Bool isContinueHandler(){Assert(isHandler());return kind & RETRY;} 
  void setContinueHandler(){Assert(isHandler()); kind = kind | RETRY;} 
  Bool isPersistent(){return kind & PERSISTENT;}
  void setPersistent(){kind = kind | PERSISTENT;}
  

  void setNext(Watcher* w){next=w;}
  Watcher* getNext(){return next;}
  void invokeHandler(EntityCond ec,Tertiary* t,Thread *,TaggedRef);
  void invokeWatcher(EntityCond ec,Tertiary* t);
  Thread* getThread(){Assert(thread!=NULL);return thread;}
  Bool isTriggered(EntityCond ec){
    if(ec & watchcond) return OK;
    return NO;}
  EntityCond getWatchCond(){return watchcond;}
};

void gcEntityInfoImpl(Tertiary *t);

#define DefaultThread ((Thread*)0x3)
//
// kost@, PER-LOOK !!!
// The following artifacts.

//
inline Bool errorIgnore(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info == NULL) return OK;
  if(info->watchers == NULL) return OK;
  return NO;
}

inline EntityCond getEntityCond(Tertiary *t) {
  EntityInfo* info = t->getInfo();
  if (info == NULL) return ENTITY_NORMAL;
  return (info->entityCond | info->managerEntityCond);
}

inline Watcher** getWatcherBase(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL) return NULL;
  if(info->watchers==NULL) return NULL;
  return &(info->watchers);}

inline void setMasterTert(Tertiary *t, Tertiary *tOther) {
  EntityInfo* info = t->getInfo();
  if(info==NULL)
    info= new EntityInfo(tOther);
  else
    info->object = tOther;
}

void insertWatcher(Tertiary *t, Watcher* w);

inline EntityCond managerPart(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|TEMP_SOME|TEMP_BLOCKED|TEMP_ME);}

inline Bool someTempCondition(EntityCond ec){
  return ec & (TEMP_SOME|TEMP_BLOCKED|TEMP_ME|TEMP_ALL);}

inline Tertiary* getInfoTert(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL) return NULL;
  return info->object;}

inline Bool resetEntityCondManager(Tertiary *t, EntityCond c){
  EntityInfo* info = t->getInfo();
  Assert(!(c & (PERM_BLOCKED|PERM_ME|PERM_SOME|PERM_ALL)));
  Assert(info!=NULL);
  EntityCond old_ec=getEntityCond(t);    
  info->managerEntityCond &= ~c;
  if(getEntityCond(t)==old_ec) return NO;
  return OK;
}

inline Bool setEntityCondOwn(Tertiary *t, EntityCond c) {
  EntityInfo* info = t->getInfo();
  if(info==NULL) {
    t->setInfo(new EntityInfo(c));
    return OK;
  }
  EntityCond old_ec=getEntityCond(t);
  info->entityCond = (info->entityCond | c);
  if(getEntityCond(t)==old_ec) return NO;
  return OK;
}

inline Bool setEntityCondManager(Tertiary *t, EntityCond c) {
  EntityInfo* info = t->getInfo();
  if(info==NULL) {
    t->setInfo(new EntityInfo(ENTITY_NORMAL, c));
    return OK;
  }
  EntityCond old_ec=getEntityCond(t);
  info->managerEntityCond=info->managerEntityCond | c;
  if(getEntityCond(t)==old_ec) return NO;
  return OK;
}

void installProbe(DSite* s,ProbeType pt);
void tertiaryInstallProbe(DSite* s,ProbeType pt,Tertiary *t);
void deinstallProbe(DSite* s,ProbeType pt);



/* PER-HANDLE



inline Bool resetEntityCondProxy(Tertiary *t, EntityCond c) {
  EntityInfo* info = t->getInfo();
  Assert(info!=NULL);
  EntityCond old_ec=getEntityCond();
  info->entityCond &= ~c;
  if(getEntityCond()==old_ec) return NO;
  return OK;
}



  
inline Watcher *getWatchersIfExist(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL){return NULL;}
  return info->watchers;}


inline void setWatchers(Tertiary *t, Watcher* e){
  EntityInfo* info = t->getInfo();
  info->watchers=e;}

inline Bool maybeHasInform(Tertiary *t){
  EntityInfo* info = t->getInfo();
  if(info==NULL) return NO;
  return OK;}
  
void insertDangelingEvent(Tertiary*);
Watcher** findWatcherBase(Tertiary *t, Thread*,EntityCond);

void releaseWatcher(Tertiary *t, Watcher*);
Bool handlerExists(Tertiary *t, Thread *);
Bool handlerExistsThread(Tertiary *t, Thread *);




Bool installHandler(Tertiary *t, EntityCond,TaggedRef,Thread*,Bool,Bool);
Bool deinstallHandler(Tertiary *t, Thread*,TaggedRef);
void installWatcher(Tertiary *t, EntityCond,TaggedRef,Bool);
Bool deinstallWatcher(Tertiary *t, EntityCond,TaggedRef);
void entityProblem(Tertiary *t);
Bool startHandlerPort(Tertiary *t, Thread*,Tertiary* ,TaggedRef,EntityCond);
void managerProbeFault(Tertiary *t, Site*, int);
void proxyProbeFault(Tertiary *t, int);


EntityCond getEntityCondPort(Tertiary*);


inline Bool somePermCondition(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|PERM_ALL);}

*/

TaggedRef listifyWatcherCond(EntityCond);

OZ_Return HandlerInstall(Tertiary *entity, SRecord *condStruct, 
			 TaggedRef proc);
OZ_Return HandlerDeInstall(Tertiary *entity, SRecord *condStruct,
			   TaggedRef proc);
OZ_Return WatcherInstall(Tertiary *entity, SRecord *condStruct,
			 TaggedRef proc);
OZ_Return WatcherDeInstall(Tertiary *entity, SRecord *condStruct,
			   TaggedRef proc);

void maybeStateError(Tertiary*, Thread*);

void insertWatcher(Tertiary*, Watcher*);
void tertiaryInstallProbe(DSite* s,ProbeType pt,Tertiary *t);
void entityProblem(Tertiary*);

/* __DPFAILHH */
#endif 


