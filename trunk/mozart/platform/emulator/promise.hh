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

class Promise: public GenCVariable {
  TaggedRef requested;
  
public:
  NO_DEFAULT_CONSTRUCTORS2(Promise);
  Promise() : GenCVariable(PROMISE) {
    requested=oz_false();
  }
  void gcRecurse(void);
  OZ_Return unifyPromise(TaggedRef*);
  Bool valid(TaggedRef /* val */) { return TRUE; } // mm2
  void addSuspPromise(TaggedRef*,Thread*,int);
  Bool isKinded() { return false; } // mm2
  void dispose(void) { freeListDispose(this, sizeof(Promise)); }
  void request();
  OZ_Return waitRequest(OZ_Term *);
};


inline
Bool isPromise(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == PROMISE);
}

inline
Promise *tagged2Promise(TaggedRef t) {
  Assert(isPromise(t));
  return (Promise *) tagged2CVar(t);
}

#endif
