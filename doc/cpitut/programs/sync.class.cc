class ConnectProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
  OZ_Term _fs;
  OZ_Term _fd;
public:
  ConnectProp(OZ_Term fsvar, OZ_Term fdvar) 
    : _fs(fsvar), _fd(fdvar)  {}
 
  virtual void gCollect(void) {
    OZ_gCollectTerm(_fd);
    OZ_gCollectTerm(_fs);
  }

  virtual void sClone(void) {
    OZ_sCloneTerm(_fd);
    OZ_sCloneTerm(_fs);
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
