/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

// proper actors

#ifndef __ACTORH
#define __ACTORH

#include "mem.hh"
#include "base.hh"

// ------------------------------------------------------------------------
//  all 'proper' actors; 

enum ActorFlags {
  Ac_None	= 0,
  Ac_Committed	= 0x01,
};

class Actor {
protected:
  int flags;
  Board * board;
  Actor * gcField;// mm2: hack: flags and board seem to be needed for copying?
public:
  NO_DEFAULT_CONSTRUCTORS(Actor)

protected:
  Actor(Board *bb) : board(bb) {
    flags   = Ac_None;
    gcField = 0;
  }

public:
  USEHEAPMEMORY;

  Bool gcIsMarked(void);
  void gcMark(Actor * fwd);
  Actor * gcGetFwd(void);

  Bool isCommitted() { return flags & Ac_Committed; }
  Bool isSolve()     { return OK; }

  void setCommittedActor() { flags |= Ac_Committed; }

  void discardActor() { setCommittedActor(); }

  Actor * gcActor();
  void gcRecurse(void);
  OZPRINTLONG

  Board *getBoardInternal() { return board; }
};

#endif
