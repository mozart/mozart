class ConnectProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
  OZ_Term _fs;
  OZ_Term _fd;
public:
  ConnectProp(OZ_Term fsvar, OZ_Term fdvar)
    : _fs(fsvar), _fd(fdvar)  {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_fd);
    OZ_updateHeapTerm(_fs);
  }

  virtual size_t sizeOf(void) {
    return sizeof(ConnectProp);
  }

  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_fs, (OZ_cons(_fd, OZ_nil())));
  }

  virtual OZ_PropagatorProfile *getProfile(void) const {
    return &profile;
  }

  virtual OZ_Return propagate();
};
