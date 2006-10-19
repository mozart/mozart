/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    (too long to list them all)
 *
 *  Anti-contributor (aka "Victor, the cleaner!")
 *    Kostja Popov <kost@sics.se>
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

#ifndef __MBUFFER_HH
#define __MBUFFER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

//
// **********************************************************************
// **********************************************************************
// **********************************************************************
//
//                YOU DO NOT NEED TO MODIFY THIS FILE!
//                           OR, simpler put,
//               KEEP YOUR FILTHY FINGERS OFF THIS STUFF!!!
//
// If you still think you have to, then
//  a) think
//  b) think again
//  c) think once again
//  d) speak to me (kost@sics.se)
//
// **********************************************************************
// **********************************************************************
// **********************************************************************
//

//
class MarshalerBuffer {
protected:
  BYTE* posMB;
  BYTE* endMB;

  // The idea is that since 'get()'/'put()' have to be fast (inlined,
  // non-virtual), the buffer operates on a set of contiguous memory
  // chunks that are accessed using these:
  virtual BYTE getNext() = 0;
  virtual void putNext(BYTE) = 0;

  //
public: 
  virtual void marshalBegin() = 0;
  virtual void marshalEnd() = 0;
  virtual void unmarshalBegin() = 0;
  virtual void unmarshalEnd() = 0;

  //
  BYTE get() {
    Assert(getDebug());
    if (posMB > endMB)
      return (getNext());
    else
      return (*posMB++);
  }
  void put(BYTE b) {
    Assert(putDebug());
    if (posMB > endMB)
      putNext(b);
    else
      *posMB++ = b;
  }

  // 
  // kost@ : if someone wants to have put/get debugging (aka former
  // 'maybeDebugBuffer{Put,Get}()'), then one HAS to declare virtual
  // 'putDebug()' methods here, and define them in corresponding
  // subclasses!! No "dpInterface" methods here!!!
  virtual Bool putDebug() { return (TRUE); }
  virtual Bool getDebug() { return (TRUE); }
};

#endif // __MBUFFER_HH
