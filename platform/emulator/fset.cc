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
void setBits(OZ_Term t, int high, int * bv)
{
  for (; OZ_isCons(t); t = OZ_tail(t)) {
      int v = OZ_intToC(OZ_head(t));
      if (0 <= v && v < 32 * high)
        bv[div32(v)] |= (1 << mod32(v));
  }
}

static
void printBits(ostream &o, int high, const int * bv)
{
  for (int first = 1, i = 0; i < 32 * high; i++) {
    if (bv[div32(i)] & (1 << mod32(i)))
      o << (first ? first = 0, "" : ",") << i;
  }
}

//-----------------------------------------------------------------------------

FSetValue::FSetValue(OZ_FSetImpl &fs)
{
  Assert(fs.isFSetValue());

  _card = fs._card;
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
  o << '{';
  printBits(o, fset_high, _in);
  o << "}#" << _card;
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

OZ_FSetImpl::OZ_FSetImpl(int c, OZ_Term ins, OZ_Term outs) : _card(c)
{
  for (int i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;

  setBits(ins, fset_high, _in);
  setBits(outs, fset_high, _not_in);

  for (int i = fset_high; i--; )
    if (_in[i] & _not_in[i]) {
      _card = -1;
      return;
    }

  _known_in = findBitsSet(fset_high, _in);
  _known_not_in = findBitsSet(fset_high, _not_in);
  if (_card < _known_in)
    _card = -1;
}

ostream &OZ_FSetImpl::print(ostream &o) const
{
  o << "{in(";
  printBits(o, fset_high, _in);
  o << ") not_in(";
  printBits(o, fset_high, _not_in);
  o << ")}#" << _card;

  return o;
}

OZ_Boolean OZ_FSetImpl::unify(const FSetValue &fs) const
{
  if (_card != fs._card)
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

  if ((z._card = (_card == y._card) ? _card : -1) != -1) {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] | y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
      if (z._in[i] & z._not_in[i]) {
        z._card = -1;
        goto end;
      }
    }
  }
  z._known_in = findBitsSet(fset_high, z._in);
  z._known_not_in = findBitsSet(fset_high, z._not_in);
  if (z._card < z._known_in)
    z._card = -1;
end:
  return z;
}

OZ_Boolean OZ_FSetImpl::isWeakerThan(OZ_FSetImpl const &fs) const
{
  return _known_in < fs._known_in || _known_not_in < fs._known_not_in;
}
