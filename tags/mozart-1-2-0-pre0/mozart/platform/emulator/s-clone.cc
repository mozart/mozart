/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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

#include "cac.hh"

#undef  G_COLLECT
#define S_CLONE

/*
 * The copy trail: CpTrail
 *
 * During copying fields that are overwritten must be saved in order 
 * to reestablish the space that has been copied.
 *
 */

class CpTrail: public FastStack {
public:
  CpTrail() : FastStack() {}
  ~CpTrail() {}
  
  void save(int * p) {
    // Save content and address
    push2((StackEntry) *p, (StackEntry) p);
  }

  void unwind(void) {
    while (!isEmpty()) {
      StackEntry e1, e2;
      pop2(e1,e2);
      int * p = (int *) e2;
      int   v = (int)   e1;
      *p = v;
    } 
  }
};

static CpTrail cpTrail;


#include "cac.cc"


/*
 * Main routine for cloning spaces
 *
 */

#ifdef CS_PROFILE
static Bool across_redid = NO;
#endif

Board * Board::clone(void) {

  cpTrail.init();
  vf.init();
  cacStack.init();
  am.nextCopyStep();

#ifdef CS_PROFILE
  across_redid  = NO;
  across_chunks = NO;
#endif

#ifdef DEBUG_CHECK
  isCollecting = OK;
#endif

  unsigned int starttime = 0;

  if (ozconf.timeDetailed)
    starttime = osUserTime();

#ifdef CS_PROFILE
redo: 
  if (across_redid)
    OZ_error("Redoing cloning across chunk boundaries. Giving up!\n");
  
  if (across_chunks)
    across_redid = OK;

  across_chunks = NO;

  cs_orig_start = (int32 *) _oz_heap_cur;
#endif

  Assert(!isCommitted());

  setGlobalMarks();
  
  Board * copy = sCloneBoard();
  
  Assert(copy);

  cacStack.sCloneRecurse();
  
  vf.sCloneFix();
  
#ifdef NEW_NAMER
  if (am.isPropagatorLocation()) {
    GCMeManager::sClone();
  }
#endif

  cpTrail.unwind();
  
  unsetGlobalMarks();
  
#ifdef CS_PROFILE
  if (across_chunks) {
    goto redo;
  }

  cs_copy_size = cs_orig_start - ((int32 *) _oz_heap_cur);
  cs_orig_start = (int32 *) _oz_heap_cur;
  cs_copy_start = (int32*) malloc(4 * cs_copy_size + 256);

  {
    int n = cs_copy_size;

    while (n) {
      *(cs_copy_start + n) = *(cs_orig_start + n);
      n--;
    }

  }
#endif

  if (ozconf.timeDetailed)
    ozstat.timeForCopy.incf(osUserTime()-starttime);

#ifdef DEBUG_CHECK
  isCollecting = NO;
#endif

  cpTrail.exit();
  vf.exit();
  cacStack.exit();

  return copy;
}

Suspendable * suspendableSCloneSuspendable(Suspendable * s) {
  return s->sCloneSuspendable();
}

