/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE)
#pragma implementation "fset.hh"
#endif

#include "fset.hh"
#include "tagged.hh"

#define FSETGLBLUB

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }

static
int findBitsSet(int high, int * bv)
{
  int bits_set = 0;

  for (int i = high; i--; ) {
    for (int j = 32; j--; )
      if (bv[i] & (1 << j))
	bits_set += 1;
  }
  return bits_set;
}

static
void setBits(OZ_Term t, int high, int * bv, int neg = 0)
{
  for (; OZ_isCons(t); t = OZ_tail(t)) {
      int v = OZ_intToC(OZ_head(t));
      if (0 <= v && v < 32 * high)
	bv[div32(v)] |= (1 << mod32(v));
  }
  if (neg) 
    for (int i = high; i--; )
      bv[i] = ~bv[i];
}

static
void printBits(ostream &o, int high, const int * bv, int neg = 0) 
{
  if (neg) {
    for (int first = 1, i = 0; i < 32 * high; i++) {
      if (!(bv[div32(i)] & (1 << mod32(i)))) 
	o << (first ? first = 0, "" : " ") << i;
    }      
  } else {
    for (int first = 1, i = 0; i < 32 * high; i++) {
      if ((bv[div32(i)] & (1 << mod32(i)))) 
	o << (first ? first = 0, "" : " ") << i;
    }
  }
}

//-----------------------------------------------------------------------------

FSetValue::FSetValue(OZ_FSetImpl &fs)
{
  Assert(fs.isFSetValue());

  _card = fs._card_min;
  for (int i = fset_high; i--; )
    _in[i] = fs._in[i];
}

FSetValue::FSetValue(OZ_Term t)
{
  for (int i = fset_high; i--; )
    _in[i] = 0;

  setBits(t, fset_high, _in);

  _card = findBitsSet(fset_high, _in);
}

OZ_Boolean FSetValue::unify(OZ_Term t)
{
  DEREF(t, tptr, ttag);
  return (ttag == FSETVALUE) ? (*tagged2FSetValue(t) == *this) : FALSE;
}

ostream &FSetValue::print(ostream &o) const
{
  o << "{[";
  printBits(o, fset_high, _in);
  o << "]}#" << _card;
  return o;
}

OZ_Boolean FSetValue::operator == (const FSetValue &fs) const
{
  if (_card != fs._card) 
    return FALSE;

  for (int i = fset_high; i--; ) {
    if (_in[i] != fs._in[i])
      return FALSE;
  }

  return TRUE;
}

//-----------------------------------------------------------------------------

OZ_FSetImpl::OZ_FSetImpl(int c_min, int c_max, OZ_Term ins, OZ_Term outs) 
  : _card_min(c_min), _card_max(c_max)
{
  int i;
  for (i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;

  setBits(ins, fset_high, _in);
  setBits(outs, fset_high, _not_in);

  for (i = fset_high; i--; )
    if (_in[i] & _not_in[i]) {
      _card_min = -1;
      return;
    }
  
  _known_in = findBitsSet(fset_high, _in);
  _known_not_in = findBitsSet(fset_high, _not_in);
  if (_card_max < _known_in || _card_max < _card_min)
    _card_min = -1;
}


OZ_FSetImpl::OZ_FSetImpl(OZ_Term ins, OZ_Term outs) 
{
  int i;
  for (i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;
  
  setBits(ins, fset_high, _in);
  setBits(outs, fset_high, _not_in, 1);

  for (i = fset_high; i--; )
    if (_in[i] & _not_in[i]) {
      _card_min = -1;
      return;
    }
  
  _card_min = _known_in = findBitsSet(fset_high, _in);
  _known_not_in = findBitsSet(fset_high, _not_in);
  _card_max = 32*fset_high - _known_not_in;

  Assert(_card_min <= _card_max);
}


void OZ_FSetImpl::printGlb(ostream &o) const 
{
  printBits(o, fset_high, _in);
}

void OZ_FSetImpl::printLub(ostream &o) const
{
  printBits(o, fset_high, _not_in, 1);
}

ostream &OZ_FSetImpl::print(ostream &o) const
{
#ifdef FSETGLBLUB
  o << "{[";
  printGlb(o);
  o << "]..[";
  printLub(o);
  o << "]}";
#else
  o << "{in(";
  printBits(o, fset_high, _in);
  o << ") not_in(";
  printBits(o, fset_high, _not_in);
  o << ")}";
#endif

  o << '#';
  if (_card_min == _card_max) 
    o << _card_min;
  else 
    o << '[' << _card_min << '#' << _card_max << ']';

  return o;
}

OZ_Boolean OZ_FSetImpl::unify(const FSetValue &fs) const
{
  if (fs._card < _card_min || _card_max < fs._card) 
    return OZ_FALSE;

  for (int i = fset_high; i--; ) {
    if (_in[i] & ~fs._in[i])
      return OZ_FALSE;
    if (_not_in[i] & fs._in[i])
      return OZ_FALSE;
  }

  return OZ_TRUE;
}


OZ_FSetImpl OZ_FSetImpl::unify(const OZ_FSetImpl &y) const
{
  OZ_FSetImpl z;
  
  z._card_min = max(_card_min, y._card_min);
  z._card_max = max(_card_max, y._card_max);

  if (z._card_max < z._card_min) 
    z._card_min = -1;

  if (z._card_min != -1) {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] | y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
      if (z._in[i] & z._not_in[i]) {
	z._card_min = -1;
	goto end;
      }
    }
  }
  z._known_in = findBitsSet(fset_high, z._in);
  z._known_not_in = findBitsSet(fset_high, z._not_in);
  if (z._card_min < z._known_in)
    z._card_min = -1;
end:
  return z;
}

OZ_Boolean OZ_FSetImpl::isWeakerThan(OZ_FSetImpl const &fs) const
{
  return _known_in < fs._known_in || _known_not_in < fs._known_not_in; 
}
