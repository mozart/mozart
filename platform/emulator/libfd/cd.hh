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
  virtual OZ_CFun getHeaderFunc(void) const { return NULL; }
};
