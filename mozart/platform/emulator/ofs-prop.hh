/*
 *  Authors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Peter van Roy (1996)
 *    Tobias Mueller (1996)
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

#ifndef __OFS_PROP_HH__

#define  __OFS_PROP_HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include <math.h>
#include "var_of.hh"
#include "var_fd.hh"
#include "builtins.hh"
#include "fdomn.hh"

class WidthPropagator : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
  OZ_Term rawrec, rawwid;  // Never change this order!
public:
  WidthPropagator(OZ_Term r, OZ_Term w)
    : rawrec(r), rawwid(w) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(WidthPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile; }
  virtual OZ_Term getParameters(void) const { return AtomNil; }
};

class MonitorArityPropagator : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
public:
  OZ_Term X, K, L, FH, FT; // Never change this order!
  MonitorArityPropagator(OZ_Term X1, OZ_Term K1, OZ_Term L1,
                         OZ_Term FH1, OZ_Term FT1)
    : X(X1), K(K1), L(L1), FH(FH1), FT(FT1) {}

  virtual void gCollect(void);
  virtual void sClone(void);
  virtual size_t sizeOf(void) { return sizeof(MonitorArityPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile; }
  virtual OZ_Term getParameters(void) const { return AtomNil; }
};

#endif
