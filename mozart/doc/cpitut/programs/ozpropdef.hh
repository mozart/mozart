class OZ_Propagator {
public:
  OZ_Propagator(void);
  virtual OZ_Term getParameters(void) const             = 0;
  virtual size_t sizeOf(void)                           = 0;
  virtual void gCollect(void)                           = 0;
  virtual void sClone(void)                             = 0;
  virtual OZ_Return propagate(void)                     = 0;
  virtual OZ_PropagatorProfile * getProfile(void) const = 0;
}
