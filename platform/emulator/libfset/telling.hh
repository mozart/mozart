/*
 *  Authors:
 *    Author's name (Author's email address)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */
/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __TELLING_HH__
#define __TELLING_HH__

#include "fsstd.hh"

class IncludePropagator : public Propagator_S_D {
private:
  static OZ_CFunHeader header;
public:
  IncludePropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

class ExcludePropagator : public Propagator_S_D {
private:
  static OZ_CFunHeader header;
public:
  ExcludePropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

class FSetCardPropagator : public Propagator_S_D {
private:
  static OZ_CFunHeader header;
public:
  FSetCardPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

#endif /* __TELLING_HH__ */

//-----------------------------------------------------------------------------
// eof
