#include "fsstd.hh"

class TellIsInPropagator : public Propagator_S_I {
private:
  static OZ_CFun spawner;
public:
  TellIsInPropagator(OZ_Term v, OZ_Term i)
    : Propagator_S_I(v, i) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

class TellIsNotInPropagator : public Propagator_S_I {
private:
  static OZ_CFun spawner;
public:
  TellIsNotInPropagator(OZ_Term v, OZ_Term i)
    : Propagator_S_I(v, i) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};

class FSetCardPropagator : public Propagator_S_D {
private:
  static OZ_CFun spawner;
public:
  FSetCardPropagator(OZ_Term s, OZ_Term d)
    : Propagator_S_D(s, d) {}

  virtual OZ_Return run(void);

  virtual OZ_CFun getSpawner(void) const {
    return spawner;
  }
};
