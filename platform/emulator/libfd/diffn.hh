/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "std.hh"

//-----------------------------------------------------------------------------

class DiffnPropagator : public OZ_Propagator {
private:
  static OZ_CFun spawner;
  int reg_size;
  OZ_Term * reg_x;
  OZ_Term * reg_y;
  int * reg_xdurs;
  int * reg_ydurs;
  int * reg_ordered;
public:
  DiffnPropagator(OZ_Term, OZ_Term, OZ_Term, OZ_Term);
  ~DiffnPropagator();
  virtual size_t sizeOf(void) { return sizeof(DiffnPropagator); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const;
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};
