/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "fdaux.hh"

#ifdef FDCD
class CDSuppl : public OZ_Propagator {
protected:
  OZ_Term reg_b;
  void * prop; /* this is a `Propagator' on the emulator side of the
		  interface */ 
public:
  CDSuppl(OZ_Propagator * p, OZ_Term b);
  
  virtual void updateHeapRefs(OZ_Boolean);
  virtual size_t sizeOf(void) { return sizeof(CDSuppl); }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }
  virtual OZ_PropagatorProfile * getProfile(void) const { return NULL; }
};

#endif
