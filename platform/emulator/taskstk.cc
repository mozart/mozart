/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "taskstk.hh"
#endif

#include "am.hh"

int TaskStack::tasks()
{
  /* we do not count the empty task */
#ifdef NEW_STACK
  return tos->count()-1;
#else
  return (tos-array)/frameSz - 1;
#endif
}

#ifndef NEW_STACK
void TaskStack::checkMax()
{
  int maxSize = getMaxSize();
  if (maxSize >= ozconf.stackMaxSize) {
    int newMaxSize = (maxSize*3)/2;

loop:
    prefixError();
    printf("\n\n*** Task stack maxsize exceeded. Increase from %d to %d? (y/n/b) ",
           ozconf.stackMaxSize,newMaxSize);
    fflush(stdout);
    char buf[1000];
    osfgets(buf,1000,stdin);
    switch (buf[0]) {
    case 'n':
      am.exitOz(1);
    case 'y':
      break;
    case 'b':
      printTaskStack();
      goto loop;
    default:
      goto loop;
    }
    ozconf.stackMaxSize = newMaxSize;
  }
}
#endif

Bool TaskStack::findCatch(TaggedRef *out)
{
  Assert(this);

  if (out) *out = nil();

  while (!isEmpty()) {
    PopFrame(tos,PC,Y,G);

    if (PC==C_CATCH_Ptr) {
      if (out) *out = reverseC(*out);
      return TRUE;
    } else if (PC==C_ACTOR_Ptr) {
      AWActor *aw = (AWActor *) Y;
      aw->setCommitted();
    } else if (PC==C_LOCK_Ptr) {
      OzLock *lck = (OzLock *) Y;
      switch(lck->getTertType()){
      case Te_Local: ((LockLocal*)lck)->unlock();break;
      case Te_Frame: ((LockFrame*)lck)->unlock();break;
      case Te_Manager: ((LockManager*)lck)->unlock();break;
      case Te_Proxy: error("lock proxy unlocking\n");break;}
    } else if (PC==C_SET_SELF_Ptr) {
      Object *newSelf = (Object*)Y;
      am.setSelf(newSelf);
    }
    if (out) {
      TaggedRef tt=CodeArea::dbgGetDef(PC);
      if (tt!=nil()) { // NOCODE_GLOBALVARNAME
        *out = cons(tt,*out);
      }
    }
  }
  if (out) *out = reverseC(*out);
  return FALSE;
}
