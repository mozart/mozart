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
  return (tos-array)/frameSz - 1;
}

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


Bool TaskStack::findCatch() 
{
  Assert(this);

  while (!isEmpty()) {
    PopFrame(tos,PC,Y,G);

    if (PC==C_CATCH_Ptr) { 
      return TRUE;
    }

    if (PC==C_ACTOR_Ptr) {
      AWActor *aw = (AWActor *) Y;
      aw->setCommitted();
    }

    if (PC==C_LOCK_Ptr) { 
      OzLock *lck = (OzLock *) Y;
      lck->unlock();
    } else if (PC==C_SET_SELF_Ptr) { 
      Object *newSelf = (Object*)Y;
      am.setSelf(newSelf);
    }
  }

  return FALSE;
}


TaggedRef TaskStack::reflect(TaskStackEntry *from,TaskStackEntry *to,
			     ProgramCounter pc)
{
  Assert(this);

  TaskStackEntry *auxtos = (from == 0) ? getTop() : from;

  if (to == 0) { // reflect all
    to = array+frameSz;  // do not include EMPTY marker
  }

  TaggedRef out = nil();

  while (auxtos > to) {
    PopFrame(auxtos,PC,Y,G);
    TaggedRef tt=CodeArea::dbgGetDef(PC);
    if (tt!=nil()) { // NOCODE_GLOBALVARNAME
      out = cons(tt,out);
    }
  }

  out = reverseC(out);
  if (pc != NOCODE) {
    out = cons(CodeArea::dbgGetDef(pc),out);
  }
  return out;
}


