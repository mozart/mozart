/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "fsstd.hh"

#ifdef FSET_HIGH
class BitVector {
private:
  int _bits[fset_high];
public:
  BitVector(void) {
    for (int i = fset_high; i --; ) 
      _bits[i] = 0;
  }
  int isIn(const int i) {
    return (i < 0 || i >= fsethigh32) 
      ? 0 
      : (_bits[i >> 5] & (1 << (i & 0x1f)));
  }
  BitVector &operator += (const int i) {
    if (0 <= i && i < fsethigh32) 
      _bits[i >> 5] |= (1 << (i & 0x1f));
    return *this;
  }
};
#endif

class MonitorInPropagator : public OZ_Propagator {
private:
#ifdef FSET_HIGH
  BitVector _in_sofar;
#else
  OZ_FSetValue _in_sofar;
#endif
  OZ_Term _fsetvar, _stream;
  static OZ_CFunHeader header;
public:
  MonitorInPropagator(OZ_Term fsetvar, OZ_Term stream) 
#ifdef FSET_HIGH
    : _fsetvar(fsetvar) , _stream(stream) { }
#else
    : _fsetvar(fsetvar) , _stream(stream), _in_sofar(fs_empty) { }
#endif
  virtual size_t sizeOf(void) { return sizeof(MonitorInPropagator); }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_stream);
    OZ_updateHeapTerm(_fsetvar);
  }
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { 
    return OZ_cons(_fsetvar, OZ_cons(_stream, OZ_nil()));
  }
  virtual OZ_CFunHeader * getHeader(void) const { return &header; }
};

#endif /* __MONITOR_HH__ */

//-----------------------------------------------------------------------------
// eof

