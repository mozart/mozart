/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Copyright:
 *    Michael Mehl (1998)
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

#include "builtins.hh"
#include "board.hh"

Suspendable * (*suspendableSCloneSuspendableDynamic)(Suspendable *);

class OzThread: public OZ_Extension {
public:

private:
  Thread *thread;
  void *distVal[2];
public:
  OzThread(Thread *thread)
    : OZ_Extension(GETBOARD(thread)), thread(thread) {
    distVal[0] = NULL;
    distVal[1] = NULL;
  }

  virtual
  int getIdV() { return OZ_E_THREAD; }

  void *getDistVal(int i){
    return distVal[i]; 
  }

  void setDistVal(int i, void* d){
    distVal[i] = d; 
  }

  virtual
  OZ_Term printV(int depth = 10) {
    return oz_pair2(oz_atom("<Thread "),
		    oz_pair2(oz_int(thread->getID() & THREAD_ID_MASK),
			     oz_atom(">")));
  }

  virtual
  OZ_Term typeV() { return oz_atom("thread"); }

  virtual
  OZ_Extension *gCollectV(void) { return new OzThread(thread); }
  virtual
  OZ_Extension *sCloneV(void) { return new OzThread(thread); }

  // mm2: possible bug: eqV may fail when dead thread is compared!
  virtual
  void gCollectRecurseV(void) {
    Thread *tmpThread = SuspToThread(thread->gCollectSuspendable());
    if (!tmpThread) {
      tmpThread = new Thread(thread->getFlags(),thread->getPriority(),
			     oz_rootBoard(),thread->getID());
      thread->cacMark(tmpThread);
    }
    thread=tmpThread;
  }
  virtual
  void sCloneRecurseV(void) {
    Thread *tmpThread = 
      SuspToThread((*suspendableSCloneSuspendableDynamic)(thread));

    if (!tmpThread) {
      tmpThread=new Thread(thread->getFlags(),thread->getPriority(),
			   oz_rootBoard(),thread->getID());
    }
    thread=tmpThread;
  }

  virtual
  OZ_Return eqV(OZ_Term term);

  Thread *getThread() { return thread; }
};

Bool oz_isThread(TaggedRef term)
{
  return oz_isExtension(term) &&
    tagged2Extension(term)->getIdV() == OZ_E_THREAD;
}

Thread *oz_ThreadToC(TaggedRef term)
{
  Assert(oz_isThread(term));
  return ((OzThread *) tagged2Extension(term))->getThread();
}

TaggedRef oz_thread(Thread *tt)
{
  TaggedRef ozTh = tt->getOzThread();
  if(ozTh == 0) 
    {
      ozTh = makeTaggedExtension(new OzThread(tt));
      tt->setOzThread(ozTh); 
    }
  return ozTh; 
}

OZ_Return OzThread::eqV(OZ_Term term)
{
  return (oz_isThread(term) &&
	  thread == oz_ThreadToC(term)) ? PROCEED : FAILED;
}

void oz_thread_setDistVal(TaggedRef tr, int i, void* v){
  ((OzThread *) tagged2Extension(tr))->setDistVal(i, v);
}

void* oz_thread_getDistVal(TaggedRef tr, int i){
  return ((OzThread *) tagged2Extension(tr))->getDistVal(i);
}
