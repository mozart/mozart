class ConstDataHdl {
private:
  char _constData[100];

  int _refCount;
  ConstDataHdl * _newLoc;  
public:
  ConstDataHdl(char * str)
    : _refCount(1), _newLoc(NULL) {
      strcpy(_constData, str);
  }
  static void * operator new (size_t sz) {
    return OZ_hallocChars(sz);
  }
  static void operator delete (void * p) {
    if (0 == --((ConstDataHdl *) p)->_refCount)
      OZ_hfreeChars((char *) p,sizeof(ConstDataHdl));
  }
  ConstDataHdl * getRef(void) { 
    _refCount += 1; 
    return this; 
  }
  ConstDataHdl * copy (void) {
    if (_newLoc)
      _newLoc->getRef();
    else 
      _newLoc = new ConstDataHdl(_constData);
    return _newLoc;
  }
};

