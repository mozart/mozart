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

#include "fdaux.hh"

class CDSuppl : public OZ_Propagator {
protected:
  OZ_Term reg_b;
  OZ_Thread thr;
public:
  CDSuppl(OZ_Propagator * p, OZ_Term b);
  
  virtual void updateHeapRefs(OZ_Boolean);
  virtual size_t sizeOf(void) { return sizeof(CDSuppl); }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }
  virtual OZ_CFunHeader * getHeader(void) const { return NULL; }
};

