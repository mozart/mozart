/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Brand
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


#ifndef __FAULTMODULE_HH
#define __FAULTMODULE_HH

#ifdef INTERFACE
#pragma interface
#endif

// raph: The new Fault model no longer uses watchers.  Streams and
// dataflow synchronization are expressive enough to implement failure
// watchers at the language level.  Therefore we only need to describe
// fault states, with their language counterpart.

#include "tagged.hh"
#include "atoms.hh"

// entity fault states (do not change order!)
enum GlueFaultState {
  GLUE_FAULT_NONE = 0,     // must be zero; non-zero means failure
  GLUE_FAULT_TEMP,         // tempFail
  GLUE_FAULT_LOCAL,        // localFail
  GLUE_FAULT_PERM          // permFail
};

// conversion between GlueFaultState and atoms
inline
TaggedRef fsToAtom(GlueFaultState fs) {
  switch (fs) {
  case GLUE_FAULT_NONE: return AtomOk;
  case GLUE_FAULT_TEMP: return AtomTempFail;
  case GLUE_FAULT_LOCAL: return AtomLocalFail;
  case GLUE_FAULT_PERM: return AtomPermFail;
  }
  Assert(0);
}

// returns TRUE iff the conversion is successful
inline
Bool atomToFS(TaggedRef a, GlueFaultState &fs) {
  if (oz_eq(a, AtomPermFail)) { fs = GLUE_FAULT_PERM; return TRUE; }
  if (oz_eq(a, AtomLocalFail)) { fs = GLUE_FAULT_LOCAL; return TRUE; }
  if (oz_eq(a, AtomTempFail)) { fs = GLUE_FAULT_TEMP; return TRUE; }
  if (oz_eq(a, AtomOk))       { fs = GLUE_FAULT_NONE; return TRUE; }
  return FALSE;
}

// check state transition
inline
Bool validFSTransition(const GlueFaultState& s0, const GlueFaultState& s1) {
  return (s0 <= s1 || s0 == GLUE_FAULT_TEMP);
}

#endif
