/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __SCHED_HH__
#define __SCHED_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class SchedCardPropagator : public Propagator_D_I_D_I {
private:
  static OZ_CFun spawner;
public:
  SchedCardPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd)
    : Propagator_D_I_D_I(x, xd, y, yd) {}
  
  virtual OZ_Return run(void);
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};


//-----------------------------------------------------------------------------

class CPIteratePropagator : public Propagator_VD_VI {
private:
  static OZ_CFun spawner;
public:
  CPIteratePropagator(OZ_Term x, OZ_Term durations) 
    : Propagator_VD_VI(x, durations) {}
  
  virtual OZ_Return run(void);
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class CPIteratePropagatorCap : public Propagator_VD_VI_VI_I {
private:
  static OZ_CFun spawner;
public:
  CPIteratePropagatorCap(OZ_Term x, OZ_Term durations, OZ_Term use, OZ_Term cap) 
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}
  
  virtual OZ_Return run(void);
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};
//-----------------------------------------------------------------------------

class CPIteratePropagatorCapUp : public Propagator_VD_VI_VI_I {
private:
  static OZ_CFun spawner;
public:
  CPIteratePropagatorCapUp(OZ_Term x, OZ_Term durations, OZ_Term use, OZ_Term cap) 
    : Propagator_VD_VI_VI_I(x, durations, use, cap) {}
  
  virtual OZ_Return run(void);
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};

//-----------------------------------------------------------------------------

class DisjunctivePropagator : public Propagator_VD_VI {
private:
  static OZ_CFun spawner;
public:
  DisjunctivePropagator(OZ_Term x, OZ_Term durations) 
    : Propagator_VD_VI(x, durations) {}
  
  virtual OZ_Return run(void);
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};

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

  static OZ_CFun spawner;

public:
  DisjunctivePropagatorStream(OZ_Term, OZ_Term, OZ_Term);
  ~DisjunctivePropagatorStream();
  virtual size_t sizeOf(void) { return sizeof(DisjunctivePropagatorStream); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return run(void); 
  virtual OZ_Term getArguments(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};


#endif // __SCHED_HH__






