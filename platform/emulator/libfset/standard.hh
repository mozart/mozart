#include "fsstd.hh"

//*****************************************************************************

class FSetIntersectionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetIntersectionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

class FSetUnionPropagator : public Propagator_S_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetUnionPropagator(OZ_Term x, OZ_Term y, OZ_Term z)
    : Propagator_S_S_S(x, y, z) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

class FSetSubsumePropagator : public Propagator_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetSubsumePropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

class FSetDisjointPropagator : public Propagator_S_S {
private:
  static OZ_CFun spawner;
public:
  FSetDisjointPropagator(OZ_Term x, OZ_Term y)
    : Propagator_S_S(x, y) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

// end of file
//-----------------------------------------------------------------------------
