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

#include "ozostream.hh"
#include "fddebug.hh"
#include "fset.hh"
#include "tagged.hh"
#include "value.hh"
#include "fddebug.hh"

//*****************************************************************************
//#define DEBUG_FSET
#ifdef DEBUG_FSET


#define _DEBUG_FSETIR(CODE) (*cpi_cout) << CODE << flush;
#define DEBUG_FSETIR(CODE) /* _DEBUG_FSETIR(CODE) */

#else

#define _DEBUG_FSETIR(CODE)
#define DEBUG_FSETIR(CODE)

#endif /* DEBUG_FSET */

#ifdef OUTLINE
#define inline
#endif

//*****************************************************************************

extern int toTheLowerEnd[];
extern int toTheUpperEnd[];

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }

inline 
unsigned char * initNumOfBitsInHalfWord(void)
{
  const unsigned int maxHalfWord = 0xffff;
  unsigned char * r = new unsigned char[maxHalfWord+1];
  Assert(r!=NULL);
  for(unsigned int i = 0; i <= maxHalfWord; i++) {
    r[i] = 0;
    int j = i;
    while (j>0) {
      if (j&1)
	r[i]++;
      j>>=1;
    }
  }
  return r;
}

inline 
int findBitsSet(int high, int * bv)
{
  static unsigned char * numOfBitsInHalfWord = initNumOfBitsInHalfWord();
  int s, i;
  for (s = 0, i = high; i--; ) {
    s += numOfBitsInHalfWord[unsigned(bv[i]) & 0xffff];
    s += numOfBitsInHalfWord[unsigned(bv[i]) >> 16];
  }    
  
  return s;
}


inline 
OZ_Boolean testBit(const int * bv, int i) 
{
  
  if (i >= 32 * fset_high || i < 0)
    return 0;

  return (bv[div32(i)] & (1 << mod32(i)));
}

inline
void setBit(int * bv, int i) 
{
  bv[div32(i)] |= (1 << mod32(i));
}

inline
void resetBit(int * bv, int i) 
{
  bv[div32(i)] &= ~(1 << mod32(i));
}

static
int mkRaw(int * list_left, int * list_right, const int * bv, int neg = 0)
{
  int i, r, l, len;
  for (i = 0, r = 1, len = 0, l = -1; i <= 32 * fset_high; i += 1)
    if ((i < 32*fset_high) && ((!neg && testBit(bv, i)) || (neg && !testBit(bv, i)))) {
      if (r) l = i;
      r = 0;
    } else {
      if (!r) {
	r = 1;
	int d = i - l;
	if (d == 1) {
	  list_left[len] = list_right[len] = l;
	  len += 1;
	} else {
	  list_left[len] = l;
	  list_right[len] = i - 1;
	  len += 1;
	}
      }
    }
  return len;
}

extern int * fd_bv_left_conv, * fd_bv_right_conv;

inline
LTuple * mkListEl(LTuple * &h, LTuple * a, OZ_Term el)
{
  if (h == NULL) {
    return h = new LTuple(el, AtomNil);
  } else {
    LTuple * aux = new LTuple(el, AtomNil);
    a->setTail(makeTaggedLTuple(aux));
    return aux;
  }
}   

static
OZ_Term getAsList(const int * bv, int neg = 0) 
{
  LTuple * hd = NULL, * l_ptr = NULL;
  int len = mkRaw(fd_bv_left_conv, fd_bv_right_conv, bv, neg);
  
  for (int i = 0; i < len; i += 1) 
    if (fd_bv_left_conv[i] == fd_bv_right_conv[i])
      l_ptr = mkListEl(hd, l_ptr, OZ_int(fd_bv_left_conv[i]));
    else
      l_ptr = mkListEl(hd, l_ptr, mkTuple(fd_bv_left_conv[i],
					  fd_bv_right_conv[i]));
  
  return hd ? makeTaggedLTuple(hd) : OZ_nil();
}

static
void setBits(OZ_Term t, int high, int * bv, int neg = 0)
{
  for (; OZ_isCons(t); t = OZ_tail(t)) {
    OZ_Term vt = OZ_head(t);
    DEREF(vt, vtptr, vttag);

    if (isSmallInt(vttag)) {
      int v = OZ_intToC(OZ_head(t));
      if (0 <= v && v < 32 * high)
	bv[div32(v)] |= (1 << mod32(v));
    } else if (isSTuple(vt)) {
      SRecord &t = *tagged2SRecord(vt);
      OZ_Term t0 = deref(t[0]), t1 = deref(t[1]);
      
      int l = OZ_intToC(t0);
      int r = OZ_intToC(t1);
      
      if (l <= r) 
	for (int i = l; i <= r; i += 1) 
	  if (0 <= i && i < 32 * high)
	    bv[div32(i)] |= (1 << mod32(i));
    } else {
      error("Unexpected case when creating finite set.");
    }
  }
  if (neg) 
    for (int i = high; i--; )
      bv[i] = ~bv[i];
}

static
void printBits(ostream &o, int high, const int * bv, int neg = 0) 
{
  int len = mkRaw(fd_bv_left_conv, fd_bv_right_conv, bv, neg);
  
  Bool flag = FALSE;

  o << '{';
  for (int i = 0; i < len; i += 1) {
    if (flag) o << ' '; else flag = TRUE; 
    o << fd_bv_left_conv[i];
    if (fd_bv_left_conv[i] != fd_bv_right_conv[i])
      if (fd_bv_left_conv[i] + 1 == fd_bv_right_conv[i])
	o << ' ' << fd_bv_right_conv[i];
      else
	o << '#' << fd_bv_right_conv[i];
  }
  o << '}';
}

//-----------------------------------------------------------------------------

inline 
void FSetValue::init(const FSetConstraint &fs)
{
  Assert(fs.isValue());

  _card = fs._card_min;
  for (int i = fset_high; i--; )
    _in[i] = fs._in[i];
}

inline 
void FSetValue::init(const OZ_Term t)
{
  for (int i = fset_high; i--; )
    _in[i] = 0;

  setBits(t, fset_high, _in);

  _card = findBitsSet(fset_high, _in);
}

inline 
void FSetValue::init(OZ_FSetState s) 
{
  switch(s) {
  case fs_empty: {
    for (int i = fset_high; i--; )
      _in[i] = 0;  
    _card = 0;
    break;
  }
  case fs_full: {
    for (int i = fset_high; i--; )
      _in[i] = ~0;  
    _card = 32*fset_high;
    break;
  }
  default:
    error("Unexpected case (%d) in \"FSetValue::init(OZ_FSetState\".", s);
  }
}
 
FSetValue::FSetValue(OZ_Term t)
{
  init(t);
}

inline 
FSetValue::FSetValue(OZ_FSetState s) 
{
  init(s);
}

 
FSetValue::FSetValue(const FSetConstraint &s) 
{
  init(s);
}

inline 
FSetValue::FSetValue(const int * in)
{
  for (int i = fset_high; i--; )
    _in[i] = in[i];

  _card = findBitsSet(fset_high, _in);
}

 
inline 
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

OZ_Boolean FSetValue::unify(OZ_Term t)
{
  DEREF(t, tptr, ttag);
  return (ttag == FSETVALUE) ? (*((FSetValue *) tagged2FSetValue(t)) == *this) : FALSE;
}
 
ostream &FSetValue::print2stream(ostream &o) const
{
  printBits(o, fset_high, _in);
  o << '#' << _card;
  return o;
}

inline 
FSetValue FSetValue::operator & (const FSetValue &y) const
{
  FSetValue z;

  for (int i = fset_high; i--; ) 
    z._in[i] = _in[i] & y._in[i];
  
  z._card = findBitsSet(fset_high, z._in);
  return z;
}

inline 
FSetValue FSetValue::operator | (const FSetValue &y) const
{
  FSetValue z;

  for (int i = fset_high; i--; ) 
    z._in[i] = _in[i] | y._in[i];
  
  z._card = findBitsSet(fset_high, z._in);
  return z;
}

inline 
FSetValue FSetValue::operator - (const FSetValue &y) const
{
  FSetValue z;

  for (int i = fset_high; i--; ) 
    z._in[i] = _in[i] & ~y._in[i];
  
  z._card = findBitsSet(fset_high, z._in);
  return z;
}

inline 
FSetValue FSetValue::operator &= (const FSetValue &y)
{
  for (int i = fset_high; i--; ) 
    _in[i] &= y._in[i];
  
  _card = findBitsSet(fset_high, _in);
  return *this;
}

inline 
FSetValue FSetValue::operator |= (const FSetValue &y)
{
  for (int i = fset_high; i--; ) 
    _in[i] |= y._in[i];
  
  _card = findBitsSet(fset_high, _in);
  return *this;
}

inline 
FSetValue FSetValue::operator &= (const int y)
{
  OZ_Boolean tb = testBit(_in, y);
  init(fs_empty);
  
  if (tb) {
    setBit(_in, y);
    _card =1;
  }
  return *this;
}

inline 
FSetValue FSetValue::operator += (const int y)
{  
  if (0 <= y && y < 32*fset_high)
    setBit(_in, y);

  _card = findBitsSet(fset_high, _in);
  return *this;
}

inline 
FSetValue FSetValue::operator -= (const int y)
{
  if (0 <= y && y < 32*fset_high)
    resetBit(_in, y);

  _card = findBitsSet(fset_high, _in);
  return *this;
}

inline 
FSetValue FSetValue::operator - (void) const
{
  FSetValue z;
  
  for (int i = fset_high; i--; ) 
    z._in[i] = ~_in[i];
  
  z._card = findBitsSet(fset_high, z._in);
  return *this;
}

inline 
OZ_Term FSetValue::getKnownInList(void) const
{
  return getAsList(_in);
}

inline 
OZ_Term FSetValue::getKnownNotInList(void) const
{
  return getAsList(_in, 1);
}

// returns -1 if there is no min element
inline 
int FSetValue::getMinElem(void) const
{
  int v, i;
  for (v = 0, i = 0; i < fset_high; v += 32, i += 1) 
    if (_in[i] != 0)
      break;

  if (i < fset_high) {
    int word = _in[i];

    if (!(word << 16)) {
      word >>= 16; v += 16;
    }
    if (!(word << 24)) {
      word >>= 8; v += 8;
    }
    if (!(word << 28)) {
      word >>= 4; v += 4;
    }
    if (!(word << 30)) {
      word >>= 2; v += 2;
    }
    if (!(word << 31))
      v++;

    return v;
  }
  return -1;
}

// returns -1 if there is no max element
inline 
int FSetValue::getMaxElem(void) const
{
  int v, i;
  for (v = 32 * fset_high - 1, i = fset_high - 1; i >= 0; v -= 32, i--) 
    if (_in[i] != 0)
      break;

  if (i >= 0) {
    int word = _in[i];
    
    if (!(word >> 16)) {
      word <<= 16; v -= 16;
    }
    if (!(word >> 24)) {
      word <<= 8; v -= 8;
    }
    if (!(word >> 28)) {
      word <<= 4; v -= 4;
    }
    if (!(word >> 30)) {
      word <<= 2; v -= 2;
    }
    if (!(word >> 31))
      v--;

    return v;
  }
  return -1;
}

inline 
int FSetValue::getNextLargerElem(int v) const
{
  for (int new_v = v + 1; new_v <= 32 * fset_high - 1; new_v += 1)
    if (testBit(_in, new_v))
      return new_v;
  
  return -1;
}

inline 
int FSetValue::getNextSmallerElem(int v) const
{
  for (int new_v = v - 1; new_v >= 0; new_v -= 1)
    if (testBit(_in, new_v))
      return new_v;
  
  return -1;
}

//-----------------------------------------------------------------------------


FSetConstraint::FSetConstraint(int c_min, int c_max, OZ_Term ins, OZ_Term outs) 
{
  _card_min = c_min;
  _card_max = c_max;
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


FSetConstraint::FSetConstraint(OZ_Term ins, OZ_Term outs) 
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

inline 
void FSetConstraint::init(void)
{
  _known_in = _known_not_in = 0;
  _card_min = 0;
  _card_max = fset_sup + 1;

  for (int i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;  
}

inline 
void FSetConstraint::init(const FSetValue &s) 
{
  _known_in = _card_min = _card_max = s._card;
  _known_not_in = 32 * fset_high - _known_in ;
  
  for (int i = fset_high; i--; )
    _not_in[i] = ~(_in[i] = s._in[i]);
}

inline 
void FSetConstraint::init(OZ_FSetState s) 
{
  switch(s) {
  case fs_empty: {
    for (int i = fset_high; i--; )
      _not_in[i] = ~(_in[i] = 0);  
    _card_min = _card_max = 0;
    break;
  }
  case fs_full: {
    for (int i = fset_high; i--; )
      _in[i] = ~(_not_in[i] = 0);  
    _card_min = _card_max = 32*fset_high;
    break;
  }
  default:
    error("Unexpected case (%d) in \"FSetConstraint::init(OZ_FSetState\".", s);
  }
}

inline 
void FSetConstraint::init(const FSetConstraint &s)
{
  for (int i = fset_high; i--; ) {
    _in[i] = s._in[i];
    _not_in[i] = s._not_in[i];
  }

  _known_in = s._known_in;
  _known_not_in = s._known_not_in;

  _card_min = s._card_min;
  _card_max = s._card_max;
}

inline 
FSetConstraint::FSetConstraint(const FSetValue& s) 
{ 
  init(s); 
}

inline 
FSetConstraint::FSetConstraint(void) 
{ 
  init(); 
}

inline 
FSetConstraint::FSetConstraint(const FSetConstraint &s)
{
  init(s);
}

inline 
FSetConstraint &FSetConstraint::operator = (const FSetConstraint &s)
{
  if (this != &s) 
    init(s);

  return *this;
}

inline 
void FSetConstraint::printGlb(ostream &o) const 
{
  printBits(o, fset_high, _in);
}

inline 
void FSetConstraint::printLub(ostream &o) const
{
  printBits(o, fset_high, _not_in, 1);
}


ostream &FSetConstraint::print(ostream &o) const
{
  o << "{";
  printGlb(o);
  o << "..";
  printLub(o);
  o << "}#";
  if (_card_min == _card_max) 
    o << _card_min;
  else 
    o << '{' << _card_min << '#' << _card_max << '}';

  return o;
}

inline 
void FSetConstraint::printDebug(void) const
{
  print(cout);
  cout << endl << flush;
}

inline 
OZ_Boolean FSetConstraint::normalize(void) 
{
  OZ_Boolean retval = OZ_FALSE;
  
  if (!isValid())
      goto end;

  // check if a value is in and out at the same time
  {
    for (int i = fset_high; i--; )
      if (_in[i] & _not_in[i]) {
	_card_min = -1;
	goto end;
      }
  }
  
  _known_in = findBitsSet(fset_high, _in);
  _known_not_in = findBitsSet(fset_high, _not_in);
  
  if (_known_in > _card_min) 
    _card_min = _known_in;
  if ((32 * fset_high - _known_not_in) < _card_max) 
    _card_max = (32 * fset_high - _known_not_in);
  
  // actually redundant, but ... 
  if ((_card_max < _known_in) || 
      (_card_min > (32 * fset_high - _known_not_in)) || 
      (_card_max < _card_min)) {
    _card_min = -1;
    goto end;
  }
  // but we can do better
  if (_card_max == _known_in) {
     _card_min = _card_max;
     _known_not_in = (32 * fset_high - _known_in);
     for (int i = fset_high; i--; )
       _not_in[i] = ~_in[i];
  }
  if (_card_min == (32 * fset_high - _known_not_in)) {
     _known_in = _card_max = _card_min;
     for (int i = fset_high; i--; )
       _in[i] = ~_not_in[i];
  }
  retval = OZ_TRUE;
end:
  DEBUG_FSETIR(*this << endl);
  return retval;
}


OZ_Boolean FSetConstraint::valid(const FSetValue &fs) const
{
  DEBUG_FSETIR("( " << *this << " valid " << fs << " ) = ");

  if (fs._card < _card_min || _card_max < fs._card) 
    goto failure;

  {
    for (int i = fset_high; i--; ) {
      if (_in[i] & ~fs._in[i])
	goto failure;
      if (_not_in[i] & fs._in[i])
	goto failure;
    }
    
    DEBUG_FSETIR("TRUE" << endl);
    return OZ_TRUE;
  }
failure:
  DEBUG_FSETIR("FALSE" << endl);
  return OZ_FALSE;
}


FSetConstraint FSetConstraint::unify(const FSetConstraint &y) const
{
  DEBUG_FSETIR("( " << *this << " unify " << y << " ) = ");

  FSetConstraint z;
  
  z._card_min = max(_card_min, y._card_min);
  z._card_max = min(_card_max, y._card_max);

  if (z._card_max < z._card_min) {
    goto failure;
  }

  {
    for (int i = fset_high; i--; ) {
      z._in[i]     = _in[i]     | y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
      if (z._in[i] & z._not_in[i]) {
	goto failure;
      }
    }
  }

  z.normalize();
  return z;

failure:
  DEBUG_FSETIR("FALSE" << endl);
  z._card_min = -1;
  return z;
}


OZ_Boolean FSetConstraint::isWeakerThan(FSetConstraint const &y) const
{
  DEBUG_FSETIR("( " << *this << " IS WEAKER THAN " << y << " ) = ");

  OZ_Boolean ret_val = ((_known_in < y._known_in) || 
			(_known_not_in < y._known_not_in) ||
			(getCardSize() > y.getCardSize()));
  DEBUG_FSETIR((ret_val ? "TRUE" : "FALSE") << endl);
  return ret_val;
}

inline 
OZ_Boolean FSetConstraint::isIn(int i) const
{
  return testBit(_in, i);
}

inline 
OZ_Boolean FSetConstraint::isNotIn(int i) const
{
  return testBit(_not_in, i);  
}

inline 
OZ_Boolean FSetConstraint::isEmpty(void) const
{
  return isValue() && (_card_min == 0);
}

inline 
OZ_Boolean FSetConstraint::isFull(void) const
{
  return isValue() && (_card_min == (32*fset_high));
}

inline 
OZ_Boolean FSetConstraint::isSubsumedBy(const FSetConstraint &y) const
{
  DEBUG_FSETIR(*this << " IS SUBSUMED BY " << y << " = ");

  if (isValue()) {
    /* All elements known to included in _x_ have to in _y_ */
    for (int i = fset_high; i--; )
      if (_in[i] & ~y._in[i])
	goto end;
  } else if (y.isValue()) {
    /* All elements known to be excluded from _x_ have 
       to be excluded from _y_ */
    for (int i = fset_high; i--; )
      if (~_not_in[i] & y._not_in[i])
	goto end;
  } else {
    goto end;
  }
  DEBUG_FSETIR("TRUE" << endl);
  return OZ_TRUE;
end:
  DEBUG_FSETIR("FALSE" << endl);
  return OZ_FALSE;
}

inline 
OZ_Term FSetConstraint::getKnownInList(void) const
{
  return getAsList(_in);
}

inline 
OZ_Term FSetConstraint::getKnownNotInList(void) const
{
  return getAsList(_not_in);
}

inline 
OZ_Term FSetConstraint::getUnknownList(void) const
{
  int unknown[fset_high];

  for (int i = fset_high; i--; )
    unknown[i] = ~(_in[i] | _not_in[i]);

  return getAsList(unknown);
}

inline 
OZ_Term FSetConstraint::getLubList(void) const
{
  return getAsList(_not_in, 1);
}

inline 
OZ_Term FSetConstraint::getCardTuple(void) const
{
  return ((_card_min == _card_max) 
	  ? OZ_int(_card_min) 
	  : mkTuple(_card_min, _card_max)); 
}

inline 
OZ_Boolean FSetConstraint::putCard(int min_card, int max_card)
{
  DEBUG_FSETIR(*this << " putCard [" << min_card << ',' << max_card << "] = ");

  _card_min = max(min_card, _card_min);
  _card_max = min(max_card, _card_max);

  return normalize();
}

inline 
FSetConstraint FSetConstraint::operator - (void) const
{
  DEBUG_FSETIR("( - " << *this << ") = ");

  FSetConstraint z;

  for (int i = fset_high; i--; ) {
    z._in[i]     = _not_in[i];
    z._not_in[i] = _in[i];
  }
  
  z.normalize();
  return z;
}

inline 
OZ_Boolean FSetConstraint::operator += (int i)
{
  DEBUG_FSETIR('(' << *this << " += " << i << ") = ");

  if (i < 0 || 32 * fset_high <= i)
    return OZ_TRUE;

  setBit(_in, i);
  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator -= (int i) 
{
  DEBUG_FSETIR('(' << *this << " -= " << i << ") = ");

  if (i < 0 || 32 * fset_high <= i)
    return OZ_TRUE;
 
  setBit(_not_in, i);
  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator <<= (const FSetConstraint& y)
{
  DEBUG_FSETIR('(' << *this << " <<= " << y << ") = ");

  for (int i = fset_high; i--; ) {
    _in[i]     |= y._in[i];
    _not_in[i] |= y._not_in[i];
  }
  
  _card_min = max(_card_min, y._card_min);
  _card_max = min(_card_max, y._card_max);

  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator <= (const FSetConstraint &y)
{
  // since _*this_ is subsumed by _y_, _*this_ must contain at least 
  // the amount of negative information as _y_ does

  DEBUG_FSETIR('(' << *this << " <= " << y << ") = ");

  for (int i = fset_high; i--; )
    _not_in[i] |= y._not_in[i];

  _card_max = min(_card_max, y._card_max);

  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator >= (const FSetConstraint &y)
{
  // since _*this_ subsumes _y_, _*this_ must contain at least 
  // the amount of positive information a _y_ does

  DEBUG_FSETIR('(' << *this << " >= " << y << ") = ");

  for (int i = fset_high; i--; )
    _in[i] |= y._in[i];

  _card_min = max(_card_min, y._card_min);

  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator != (const FSetConstraint &y)
{
  DEBUG_FSETIR('(' << *this << " != " << y << ") = ");

  for (int i = fset_high; i--; )
    _not_in[i] |= y._in[i];

  
  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator == (const FSetConstraint &y) const
{
  DEBUG_FSETIR('(' << *this << " != " << y << ") = ");

  if (_card_min != y._card_min || 
      _card_max != y._card_max ||
      _known_not_in != y._known_not_in ||
      _known_in != y._known_in) 
    return FALSE;

  for (int i = fset_high; i--; ) 
    if (_in[i] != y._in[i] || _not_in[i] != y._not_in[i])
      return FALSE;
  
  return TRUE;
}

inline 
FSetConstraint FSetConstraint::operator & (const FSetConstraint& y) const
{
  DEBUG_FSETIR(*this << " & " << y << " = ");

  FSetConstraint z;
  
  if (!isValid() || !y.isValid()) {
    z._card_min = -1;
    goto end;
  }

  {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] & y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
    }
  }
  
  z._card_min = 0;
  z._card_max = min(_card_max, y._card_max);
  
end:
  z.normalize();
  return z;
}

inline 
FSetConstraint FSetConstraint::operator | (const FSetConstraint& y) const
{
  DEBUG_FSETIR(*this << " | " << y << " = ");

  FSetConstraint z;
  
  if (!isValid() || !y.isValid()) {
    z._card_min = -1;
    goto end;
  }

  {  
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] | y._in[i];
      z._not_in[i] = _not_in[i] & y._not_in[i];
    }
  }
  
  z._card_min = max(_card_min, y._card_min);
  z._card_max = _card_max + y._card_max;

end:
  z.normalize();
  return z;
}

inline 
FSetConstraint FSetConstraint::operator - (const FSetConstraint& y) const
{
  DEBUG_FSETIR(*this << " - " << y << " = ");

  FSetConstraint z;
  
  if (!isValid() || !y.isValid()) {
    z._card_min = -1;
    goto end;
  }

  {  
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] & y._not_in[i];
      z._not_in[i] = _not_in[i] | y._in[i];
    }
  }

  z._card_min = 0;
  z._card_max = _card_max;

end:
  z.normalize();
  DEBUG_FSETIR(z << endl << flush);
  return z;
}

inline 
FSetValue FSetConstraint::getGlbSet(void) const
{
  return FSetValue(_in);
}

inline 
FSetValue FSetConstraint::getLubSet(void) const
{
  int i, lub[fset_high];
  
  for (i = fset_high; i--; )
    lub[i] = ~_not_in[i];

  return FSetValue(lub);
}

inline 
FSetValue FSetConstraint::getUnknownSet(void) const
{
  int i, unknown[fset_high];
  
  for (i = fset_high; i--; )
    unknown[i] = ~(_in[i] | _not_in[i]);

  return FSetValue(unknown);
}

inline 
FSetValue FSetConstraint::getNotInSet(void) const
{
  return FSetValue(_not_in);  
}

inline 
OZ_Boolean FSetConstraint::operator >= (const int ii)
{
  int lower_word = div32(ii), lower_bit = mod32(ii);

  for (int i = 0; i < lower_word; i += 1)
    _not_in[i] = ~0;
  _not_in[lower_word] |= ~toTheUpperEnd[lower_bit];

  return normalize();
}

inline 
OZ_Boolean FSetConstraint::operator <= (const int ii)
{
  int upper_word = div32(ii), upper_bit = mod32(ii);
  
  for (int i = upper_word + 1; i < fset_high; i += 1)
    _not_in[i] = ~0;
  _not_in[upper_word] |= ~toTheLowerEnd[upper_bit];
  
  return normalize();
}

//*****************************************************************************

#define CASTPTR (FSetValue *)
#define CASTREF * (FSetValue *) &
#define CASTTHIS (CASTPTR this)


OZ_FSetValue::OZ_FSetValue(const OZ_FSetConstraint &s) 
{
  CASTTHIS->init(* (FSetConstraint *) &s);
}

OZ_FSetValue::OZ_FSetValue(const OZ_Term t)
{
  CASTTHIS->init(t);
}

OZ_FSetValue::OZ_FSetValue(const OZ_FSetState s)
{
  CASTTHIS->init(s);
}

OZ_Term OZ_FSetValue::getKnownInList(void) const
{
  return CASTTHIS->getKnownInList();
}

OZ_Term OZ_FSetValue::getKnownNotInList(void) const
{
  return CASTTHIS->getKnownNotInList();
}

OZ_Boolean OZ_FSetValue::isIn(int i) const
{
  return CASTTHIS->isIn(i);
}

OZ_Boolean OZ_FSetValue::isNotIn(int i) const
{
  return CASTTHIS->isNotIn(i);
}

int OZ_FSetValue::getMinElem(void) const
{
  return CASTTHIS->getMinElem();
}

int OZ_FSetValue::getMaxElem(void) const
{
  return CASTTHIS->getMaxElem();
}

int OZ_FSetValue::getNextLargerElem(int i) const
{
  return CASTTHIS->getNextLargerElem(i);
}

int OZ_FSetValue::getNextSmallerElem(int i) const
{
  return CASTTHIS->getNextSmallerElem(i);
}

OZ_Boolean OZ_FSetValue::operator == (const OZ_FSetValue &y) const
{
  return CASTTHIS->operator == (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator & (const OZ_FSetValue &y) const
{
  return CASTTHIS->operator & (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator | (const OZ_FSetValue &y) const
{
  return CASTTHIS->operator | (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator &= (const OZ_FSetValue &y)
{
  return CASTTHIS->operator &= (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator |= (const OZ_FSetValue &y) 
{
  return CASTTHIS->operator |= (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator - (const OZ_FSetValue &y) const
{
  return CASTTHIS->operator - (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator &= (const int y)
{
  return CASTTHIS->operator &= (y);
}

OZ_FSetValue OZ_FSetValue::operator += (const int y)
{
  return CASTTHIS->operator += (y);
}

OZ_FSetValue OZ_FSetValue::operator -= (const int y)
{
  return CASTTHIS->operator -= (y);
}

OZ_FSetValue OZ_FSetValue::operator - (void) const
{
  return CASTTHIS->operator - ();
}

char * OZ_FSetValue::toString() const
{
  static ozstrstream str;
  str.reset();
  CASTTHIS->print(str);
  return str.str();
}

//-----------------------------------------------------------------------------

#undef CASTPTR
#undef CASTREF
#undef CASTTHIS


#define CASTPTR (FSetConstraint *)
#define CASTREF * (FSetConstraint *) &
#define CASTTHIS (CASTPTR this)

OZ_FSetConstraint::OZ_FSetConstraint(const OZ_FSetValue &s) 
{
  CASTTHIS->init(* (FSetValue *) &s);
}

OZ_FSetConstraint::OZ_FSetConstraint(OZ_FSetState s) 
{
  CASTTHIS->init(s);
}

OZ_FSetConstraint::OZ_FSetConstraint(const OZ_FSetConstraint &s) 
{
  CASTTHIS->init(* (FSetConstraint *) &s);
}

OZ_FSetConstraint &OZ_FSetConstraint::operator = (const OZ_FSetConstraint &s)
{
  return CASTTHIS->operator = (* (FSetConstraint *) &s);
}

void OZ_FSetConstraint::init(OZ_FSetState s) 
{
  CASTTHIS->init(s);
}

void OZ_FSetConstraint::init(void) 
{
  CASTTHIS->init();
}

OZ_Boolean OZ_FSetConstraint::isValue(void) const
{
  return CASTTHIS->isValue();
}

OZ_Boolean OZ_FSetConstraint::isIn(int i) const
{
  return CASTTHIS->isIn(i);
}

OZ_Boolean OZ_FSetConstraint::isNotIn(int i) const
{
  return CASTTHIS->isNotIn(i);
}

OZ_Boolean OZ_FSetConstraint::isEmpty(void) const
{
  return CASTTHIS->isEmpty();
}

OZ_Boolean OZ_FSetConstraint::isFull(void) const
{
  return CASTTHIS->isFull();
}

OZ_Boolean OZ_FSetConstraint::isSubsumedBy(const OZ_FSetConstraint &s) const
{
  return CASTTHIS->isSubsumedBy(CASTREF s);
}

OZ_Term OZ_FSetConstraint::getKnownInList(void) const
{
  return CASTTHIS->getKnownInList();
}

OZ_Term OZ_FSetConstraint::getKnownNotInList(void) const
{
  return CASTTHIS->getKnownNotInList();
}

OZ_Term OZ_FSetConstraint::getUnknownList(void) const
{
  return CASTTHIS->getUnknownList();
}

OZ_FSetConstraint OZ_FSetConstraint::operator - (void) const
{
  return CASTTHIS->operator - ();
}

OZ_Boolean OZ_FSetConstraint::operator += (int i)
{
  return CASTTHIS->operator += (i);
}

OZ_Boolean OZ_FSetConstraint::operator -= (int i) 
{
  return CASTTHIS->operator -= (i);
}

OZ_Boolean OZ_FSetConstraint::operator <<= (const OZ_FSetConstraint& y)
{
  return CASTTHIS->operator <<= (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator & (const OZ_FSetConstraint& y) const
{
  return CASTTHIS->operator & (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator | (const OZ_FSetConstraint& y) const
{
  return CASTTHIS->operator | (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator - (const OZ_FSetConstraint& y) const
{
  return CASTTHIS->operator - (CASTREF y);
}

OZ_Term OZ_FSetConstraint::getLubList(void) const
{
  return CASTTHIS->getLubList();
}

OZ_Term OZ_FSetConstraint::getCardTuple(void) const
{
  return CASTTHIS->getCardTuple();
}

OZ_Boolean OZ_FSetConstraint::operator <= (const OZ_FSetConstraint &y)
{
  return CASTTHIS->operator <= (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::operator >= (const OZ_FSetConstraint &y)
{
  return CASTTHIS->operator >= (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::operator != (const OZ_FSetConstraint &y)
{
  return CASTTHIS->operator != (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::operator == (const OZ_FSetConstraint &y) const
{
  return CASTTHIS->operator == (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::putCard(int min_card, int max_card)
{
  return CASTTHIS->putCard(min_card, max_card);
}

OZ_FSetValue OZ_FSetConstraint::getGlbSet(void) const
{
  return CASTTHIS->getGlbSet();
}

OZ_FSetValue OZ_FSetConstraint::getLubSet(void) const
{
  return CASTTHIS->getLubSet();
}

OZ_FSetValue OZ_FSetConstraint::getUnknownSet(void) const
{
  return CASTTHIS->getUnknownSet();
}

OZ_FSetValue OZ_FSetConstraint::getNotInSet(void) const
{
  return CASTTHIS->getNotInSet();
}

char * OZ_FSetConstraint::toString() const
{
  static ozstrstream str;
  str.reset();
  CASTTHIS->print(str);
  return str.str();
}

OZ_Boolean OZ_FSetConstraint::operator <= (const int i)
{
  return CASTTHIS->operator <= (i);
}

OZ_Boolean OZ_FSetConstraint::operator >= (const int i)
{
  return CASTTHIS->operator >= (i);
}

// eof 
//-----------------------------------------------------------------------------
