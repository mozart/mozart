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

#ifndef __STANDARD_HH__
#define __STANDARD_HH__

#include "fsstd.hh"

//*****************************************************************************

class FSetIntersectionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFunHeader spawner;
public:
  FSetIntersectionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

class FSetUnionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFunHeader spawner;
public:
  FSetUnionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

class FSetSubsumePropagator : public Propagator_S_S {
private:
  static OZ_CFunHeader spawner;
public:
  FSetSubsumePropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

class FSetDisjointPropagator : public Propagator_S_S {
private:
  static OZ_CFunHeader spawner;
public:
  FSetDisjointPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

class FSetDistinctPropagator : public Propagator_S_S {
private:
  static OZ_CFunHeader spawner;
public:
  FSetDistinctPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const {
    return &spawner;
  }
};

class FSetDiffPropagator : public Propagator_S_S_S {
private:
  static OZ_CFunHeader header;
public:
  FSetDiffPropagator(OZ_Term x,OZ_Term y,OZ_Term z)
    : Propagator_S_S_S(x,y,z) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {
    return &header;
  }
};

//*****************************************************************************
#endif /* __STANDARD_HH__ */
// end of file
