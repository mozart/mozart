/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "fsstd.hh"

class BitVector {
private:
  int _bits[fset_high];
public:
  BitVector(void) {
    for (int i = fset_high; i --; )
      _bits[i] = 0;
  }
  int isIn(const int i) {
    return (i < 0 || i >= 32 * fset_high)
      ? 0
      : (_bits[i >> 5] & (1 << (i & 0x1f)));
  }
  BitVector &operator += (const int i) {
    if (0 <= i && i < 32 * fset_high)
      _bits[i >> 5] |= (1 << (i & 0x1f));
    return *this;
  }
};

class MonitorInPropagator : public OZ_Propagator {
private:
  BitVector _in_sofar;
  OZ_Term _fsetvar, _stream;
  static OZ_CFun header;
public:
  MonitorInPropagator(OZ_Term fsetvar, OZ_Term stream)
    : _fsetvar(fsetvar) , _stream(stream) { }
  virtual size_t sizeOf(void) { return sizeof(MonitorInPropagator); }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_stream);
    OZ_updateHeapTerm(_fsetvar);
  }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_fsetvar, OZ_cons(_stream, OZ_nil()));
  }
  virtual OZ_CFun getHeaderFunc(void) const { return header; }
};

#endif /* __MONITOR_HH__ */

//-----------------------------------------------------------------------------
// eof
