/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PROMISE__HH__
#define __PROMISE__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "am.hh"
#include "genvar.hh"
#include "tagged.hh"
#include "value.hh"
#include "mem.hh"
#include "thread.hh"

class Promise {
  TaggedRef future;
  GName *gname;
public:
  USEHEAPMEMORY;

  Promise(TaggedRef f, GName *gn): future(f), gname(gn) {}

  TaggedRef getFuture() { return future; }
  Promise *gcPromise();
  GName *globalize();
};

class Future: public PerdioVar {
  TaggedRef requested;

public:
  NO_DEFAULT_CONSTRUCTORS2(Future);
  void init();
  Future()       : PerdioVar(OK)    { init(); }
  Future(int bi, TaggedRef req) : PerdioVar(bi,OK) { requested = req; }
  void gcFuture();
  OZ_Return unifyFuture(TaggedRef*);
  Bool valid(TaggedRef /* val */) { return TRUE; } // mm2
  void addSuspFuture(TaggedRef*,Thread*,int);
  Bool isKinded() { return false; } // mm2
  void dispose(void) { freeListDispose(this, sizeof(Future)); }
  void request();
  TaggedRef getRequested() { return requested; }
  OZ_Return waitRequest(OZ_Term *);
};


inline
Bool isFuture(TaggedRef term)
{
  GCDEBUG(term);
  return isPerdioVar(term) && tagged2PerdioVar(term)->isFuture();
}

inline
Future *tagged2Future(TaggedRef t) {
  Assert(isFuture(t));
  return (Future *) tagged2CVar(t);
}

#endif
