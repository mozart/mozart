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

Board *WaitActor::getChild()
{
  int32 maxx = ToInt32(childs[-1]);
  for (int i = 0; i < maxx; i++) {
    if (childs[i]) {
      Board *wb = childs[i];
      for (; i < maxx-1; i++) {    // the order must be preserved (for solve); 
	childs[i] = childs[i+1];
      }
      childs[maxx-1] = NULL;
      return (wb);
    }
  }
  error("WaitActor::getChild");
  return NULL;
}

Board *WaitActor::getChildRef ()
{
  int32 maxx = ToInt32(childs[-1]);
  for (int i = 0; i < maxx; i++) {
    if (childs[i]) {
      return (childs[i]);
    }
  }
  error("WaitActor::getChild");
  return NULL;
}


#ifdef OUTLINE
#define inline
#include "actor.icc"
#undef inline
#endif
