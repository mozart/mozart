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
class DisjunctivePropagatorStream : public OZ_Propagator {
private:
  // The finite domains
  OZ_Term * reg_fds;
  // overall number of FDs
  int reg_size;

  // The durations
  int * reg_durs;

  OZ_Term stream;

  static OZ_CFunHeader spawner;
public:
  DisjunctivePropagatorStream(OZ_Term, OZ_Term, OZ_Term);
  ~DisjunctivePropagatorStream();
  virtual size_t sizeOf(void) { return sizeof(DisjunctivePropagatorStream); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------
class DistinctPropagatorStream : public OZ_Propagator {
private:
  // The finite domains
  OZ_Term * reg_fds;
  // overall number of FDs
  int reg_size;

  OZ_Term stream;

  static OZ_CFunHeader spawner;
public:
  DistinctPropagatorStream(OZ_Term, OZ_Term);
  ~DistinctPropagatorStream();
  virtual size_t sizeOf(void) { return sizeof(DistinctPropagatorStream); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

