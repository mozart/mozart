/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __INTERFAULTHH
#define __INTERFAULTHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "value.hh"

enum WatcherKind{
  WATCHER_RETRY      = 1,
  WATCHER_PERSISTENT = 2,
  WATCHER_SITE_BASED = 4,
  WATCHER_INJECTOR   = 8,
  WATCHER_CELL       = 16,
  WATCHER_GLOBAL     = 32
};

enum EntityCondFlags{
  ENTITY_NORMAL = 0,
  PERM_FAIL  = 2,       
  TEMP_FAIL  = 1,
  PERM_ALL      = 4,
  TEMP_ALL      = 8,
  PERM_SOME     = 16,
  TEMP_SOME     = 32,
  UNREACHABLE   = 64,
  ANY_COND      = 128
};

#define IncorrectFaultSpecification \
oz_raise(E_ERROR,E_KERNEL,"type",1,oz_atom("incorrect fault specification"));

#define DerefVarTest(tt) { \
  if(OZ_isVariable(tt)){OZ_suspendOn(tt);} \
  tt=oz_deref(tt);}

typedef unsigned int EntityCond;

class DeferWatcher{
public:
  TaggedRef proc;
  Thread* thread;
  TaggedRef entity;
  short kind;
  EntityCond watchcond;
  DeferWatcher* next;

  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS(DeferWatcher);

  DeferWatcher(short wk,EntityCond c,
	       Thread* th,TaggedRef e,TaggedRef p){
    proc=p;
    thread=th;
    entity=e;
    next=NULL;
    watchcond=c;
    kind=wk;}

  Bool isEqual(short,EntityCond,Thread *,TaggedRef,TaggedRef);

  Bool preventAdd(short,Thread *,TaggedRef);

  void gCollect(void);

};


extern DeferWatcher* deferWatchers;
extern Bool perdioInitialized;

void gCollectDeferWatchers(void);

Bool addDeferWatcher(short, EntityCond, Thread*,
		     TaggedRef, TaggedRef);

Bool remDeferWatcher(short, EntityCond, Thread*,
		     TaggedRef, TaggedRef);

OZ_Return distHandlerInstallHelp(SRecord*, EntityCond&, Thread* &,TaggedRef &,
				 short&);
Bool isWatcherEligible(TaggedRef);

/* __INTERFAULTHH */
#endif 


