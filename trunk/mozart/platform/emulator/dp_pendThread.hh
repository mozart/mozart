/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

#ifndef __pend_thread_hh
#define __pend_thread_hh

#include "value.hh"
#include "perdio_debug.hh"
#include "controlvar.hh"

#define DummyThread ((Thread*)0x1)
#define MoveThread  ((Thread*)NULL)

class PendThread{
public:
  PendThread *next;
  Thread *thread;
  TaggedRef controlvar;
  TaggedRef nw;
  TaggedRef old;
  ExKind    exKind;
  PendThread(Thread *th,PendThread *pt):
    next(pt), thread(th),old(0),nw(0), controlvar(0), exKind(NOEX) {}
  PendThread(Thread *th,PendThread *pt,TaggedRef o, TaggedRef n, TaggedRef cv,
	     ExKind e)
    :next(pt), thread(th),old(o),nw(n), exKind(e), controlvar(cv) {}
  USEFREELISTMEMORY;
  void dispose(){freeListDispose(this,sizeof(PendThread));}
};

inline Bool isRealThread(Thread* t){
  if((t==MoveThread) || (t==DummyThread)) return FALSE;
  return TRUE;}


void pendThreadRemove(PendThread *pt, Thread *th);
Thread* pendThreadResumeFirst(PendThread **pt);
void pendThreadRemoveFirst(PendThread **pt);
OZ_Return pendThreadAddToEnd(PendThread **pt,Thread *t, TaggedRef o, 
			     TaggedRef n, ExKind e, Board *home);

inline OZ_Return pendThreadAddToEnd(PendThread **pt,Thread *t, Board *home){
  return pendThreadAddToEnd(pt,t,0,0,NOEX,home);}

inline Bool basicThreadIsPending(PendThread *pt,Thread*t){
  while(pt!=NULL){
    if(pt->thread==t) return OK;
    pt=pt->next;}
  return NO;}

inline PendThread* getPendThread(PendThread *pt, Thread *t){
  while(pt!=NULL){
    if(pt->thread==t) return pt;
    pt=pt->next;}
  return NULL;}

#endif
