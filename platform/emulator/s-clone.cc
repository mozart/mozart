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

class CpTrail: public Stack {
public:
  CpTrail() : Stack(1024, Stack_WithMalloc) {}
  ~CpTrail() {}
  
  void save(int * p) {
    // Save content and address
    ensureFree(2);
    push((StackEntry) *p, NO);
    push((StackEntry) p,  NO);
  }

  void unwind(void) {
    while (!isEmpty()) {
      int * p = (int *) pop();
      int   v = (int)   pop();
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

  cs_orig_start = (int32 *) heapTop;
#endif

  Assert(!isCommitted());

  setGlobalMarks();
  
  Board * copy = sCloneBoard();
  
  Assert(copy);

  cacStack.sCloneRecurse();
  
  varFix.sCloneFix();
  
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

  cs_copy_size = cs_orig_start - ((int32 *) heapTop);
  cs_orig_start = (int32 *) heapTop;
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

  return copy;
}

Suspendable * suspendableSCloneSuspendable(Suspendable * s) {
  return s->sCloneSuspendable();
}

