/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Duration and Use of tasks can be finite domain variables.
  ------------------------------------------------------------------------
*/

#ifndef __SCHED_HH__
#define __SCHED_HH__

#include "std.hh"

class Propagator_Cumulative : public OZ_Propagator {
private:
  static OZ_CFun spawner;
protected:
  OZ_Term * reg_l;
  OZ_Term * reg_offset;
  int reg_sz;
  OZ_Term * reg_use;
  int * reg_surfaces;
  int reg_capacity;
public:
  Propagator_Cumulative(OZ_Term tasks, OZ_Term l, OZ_Term offset, 
			OZ_Term use, OZ_Term surfaces, OZ_Term cap);

  Propagator_Cumulative(int);
  virtual ~Propagator_Cumulative(void);

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }

  virtual void updateHeapRefs(OZ_Boolean);
  virtual size_t sizeOf(void) { return sizeof(Propagator_Cumulative); }
  virtual OZ_Term getParameters(void) const;
};


#endif // __SCHED_HH__
