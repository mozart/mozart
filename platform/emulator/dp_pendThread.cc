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

#include "dp_pendThread.hh"
#include "am.hh"

void pendThreadRemove(PendThread *pt, Thread *th){
  while(pt!=NULL && pt->thread != th)
    pt = pt->next;
  Assert(pt!=NULL);
  pt->thread = DummyThread;
  pt->old = 0;
  pt->nw = 0;}

Thread* pendThreadResumeFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Thread *t=tmp->thread;
  Assert(isRealThread(t));
  PD((THREAD_D,"start thread ResumeFirst %x",t));
  ControlVarResume(tmp->controlvar);
  *pt=tmp->next;
  tmp->dispose();
  return t;}

void pendThreadRemoveFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Assert(!isRealThread(tmp->thread));
  *pt=tmp->next;  
  tmp->dispose();}
  
OZ_Return pendThreadAddToEnd(PendThread **pt,Thread *t, TaggedRef o, 
				    TaggedRef n, ExKind e, Board *home)
{
  while(*pt!=NULL){pt= &((*pt)->next);}

  if(isRealThread(t) && e != REMOTEACCESS) {
    ControlVarNew(controlvar,home);
    *pt=new PendThread(t,NULL,o,n,controlvar,e);
    PD((THREAD_D,"stop thread addToEnd %x",t));
    SuspendOnControlVar;
  }
  *pt=new PendThread(t,NULL,o,n,makeTaggedNULL(),e);
  return PROCEED;
}
