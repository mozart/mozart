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

class OzThread: public OZ_SituatedExtension {
public:

private:
  Thread *thread;
public:
  OzThread(Thread *thread)
    : OZ_SituatedExtension(GETBOARD(thread)), thread(thread) {}

  virtual
  int getIdV() { return OZ_E_THREAD; }

  virtual
  OZ_Term printV(int depth = 10) {
    return oz_pair2(oz_atom("<Thread "),
		    oz_pair2(oz_int(thread->getID() & THREAD_ID_MASK),
			     oz_atom(">")));
  }

  virtual
  OZ_Term typeV() { return oz_atom("thread"); }

  virtual
  OZ_Extension *gcV(void) { return new OzThread(*this); }

  // mm2: possible bug: eqV may fail when dead thread is compared!
  virtual
  void gcRecurseV(void) {
    Thread *tmpThread = thread->gcThread();
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
    oz_tagged2Extension(term)->getIdV() == OZ_E_THREAD;
}

Thread *oz_ThreadToC(TaggedRef term)
{
  Assert(oz_isThread(term));
  return ((OzThread *) oz_tagged2Extension(term))->getThread();
}

TaggedRef oz_thread(Thread *tt)
{
  return oz_makeTaggedExtension(new OzThread(tt));
}

OZ_Return OzThread::eqV(OZ_Term term)
{
  return (oz_isThread(term) &&
	  thread == oz_ThreadToC(term)) ? PROCEED : FAILED;
}
