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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "extension.hh"
#include "builtins.hh"
#include "board.hh"

class OzThread: public SituatedExtension {
public:

private:
  Thread *thread;
public:
  OzThread(Thread *thread)
    : SituatedExtension(GETBOARD(thread)), thread(thread) {}

  virtual
  int getIdV() { return OZ_E_THREAD; }

  virtual
  void printStreamV(ostream &out,int depth = 10) {
    out << "<Thread " << (thread->getID() & THREAD_ID_MASK) << ">";
  }

  virtual
  OZ_Term typeV() { return oz_atom("thread"); }

  virtual
  Extension *gcV(void) { return new OzThread(*this); }

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
    tagged2Extension(term)->getIdV() == OZ_E_THREAD;
}

Thread *oz_ThreadToC(TaggedRef term)
{
  Assert(oz_isThread(term));
  return ((OzThread *) tagged2Extension(term))->getThread();
}

TaggedRef oz_thread(Thread *tt)
{
  return makeTaggedConst(new OzThread(tt));
}

OZ_Return OzThread::eqV(OZ_Term term)
{
  return (oz_isThread(term) &&
	  thread == oz_ThreadToC(term)) ? PROCEED : FAILED;
}
