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
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
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
  virtual void gCollect(void);
  virtual void sClone(void);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const;
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};
