/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of proper actors
  ------------------------------------------------------------------------
*/


#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "actor.hh"
#endif

#include "tagged.hh"

#include "constter.hh"
#include "suspensi.hh"

#include "actor.hh"
#include "board.hh"

/*
 * class Actor:
 *    may be any 'proper' actor;
 *    member data:
 *      flags: see enum ActorFlags
 *      board: board in which actor is installed
 *      priority: if a new thread must be created;
 *
 * class AWActor:
 *    may be conditional or disjunction
 *    member data:
 *      next: continuation for next clause
 *      childCount:
 *
 * class AskActor:
 *    member data
 *      elsePC: programm counter of else
 *
 * class WaitActor
 *    member data
 *      childs: list of childs
 */

/* ------------------------------------------------------------------------
   class WaitActor
   ------------------------------------------------------------------------ */

void WaitActor::addChildInternal(Board *bb)
{
  if (!childs) {
    childs=(Board **) freeListMalloc(3*sizeof(Board *));
    *childs++ = (Board *) 2;
    childs[0] = bb;
    childs[1] = NULL;
    return;
  }
  int32 maxx= ToInt32(childs[-1]);
  int i;
  for (i = 0; i < maxx; i++) {
    if (!childs[i]) {
      childs[i] = bb;
      return;
    }
  }
  int size = 2*maxx;
  Board **cc = (Board **) freeListMalloc((size+1)*sizeof(Board *));
  *cc++ = (Board *) ToPointer(size);
  for (i = 0; i < maxx; i++) {
    cc[i] = childs[i];
  }
  freeListDispose(childs-1,(ToInt32(childs[-1])+1)*sizeof(Board *));
  childs = cc;
  childs[maxx] = bb;
  for (i = maxx+1; i < size; i++) {
    childs[i] = NULL;
  }
}

void WaitActor::failChildInternal(Board *bb)
{
  int32 maxx = ToInt32(childs[-1]);
  for (int i = 0; i < maxx; i++) {
    if (childs[i] == bb) {
      for (; i < maxx-1; i++) {    // the order must be preserved (for solve);
        childs[i] = childs[i+1];
      }
      childs[maxx-1] = NULL;
      return;
    }
  }
  error("WaitActor::failChildInternal");
}

int WaitActor::selectOrFailChildren(int l, int r) {
  if (l<=r && l>=0 && r<childCount) {
    for (int i = l; i <= r; i++) {
      childs[i-l]->setFailed();
      childs[i-l] = childs[i];
    }
    for (int j = r+1; j < childCount; j++) {
      childs[j]->setFailed();
      childs[j] = NULL;
    }
    childCount = r-l+1;
    return childCount;
  } else {
    return 0;
  }

}

int WaitActor::selectOrFailChild(int i) {
  if (i>=0 && i<childCount) {
    for (int j = 0;   j < i;    j++)
      childs[j]->setFailed();
    for (int k = i+1; k < childCount; k++)
      childs[k]->setFailed();
    return 1;
  } else {
    return 0;
  }

}

#ifdef OUTLINE
#define inline
#include "actor.icc"
#undef inline
#endif
