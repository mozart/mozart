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


#if defined(INTERFACE)
#pragma implementation "fset.hh"
#endif

#include <stdarg.h>

#include "base.hh"
#include "ozostream.hh"
#include "fset.hh"
#include "tagged.hh"
#include "value.hh"
#include "bits.hh"

//*****************************************************************************

#ifdef DEBUG_FSET_CONSTRREP

void print_to_fsfile (const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(_fset_file, format, ap);
  fflush(_fset_file);
  va_end(ap);
}

#endif

#ifdef TO_FSET_FILE
FILE * _fset_file = fopen("/tmp/fset_ir_debug_file.oz", "w+");
#else
FILE * _fset_file = stdout;
#endif

//*****************************************************************************

#ifdef OUTLINE
#define inline
#endif

// ***************************************************************************

// helper functions

#ifdef BIGFSET
const int size_of_other = fs_sup - 32 * fset_high + 1;
#endif

extern int toTheLowerEnd[];
extern int toTheUpperEnd[];

extern int * fd_bv_left_conv, * fd_bv_right_conv;

static
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


inline
int findBitsSet(const int high, const int * bv) {
  return get_num_of_bits(high, bv);
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

inline
int setFromTo(int * bv, int from, int to)
{
  if (from < 0)
    from = 0;
  if (to >= 32 * fset_high)
    to = 32 * fset_high - 1;

  int i;
  if (from > to) { // make empty set
    for (i = 0; i < fset_high; i++)
      bv[i] = 0;
    return 0;
  }
  int low_word = div32(from), low_bit = mod32(from);
  int up_word = div32(to), up_bit = mod32(to);

  for (i = 0; i < low_word; i++)
    bv[i] = 0;
  for (i = up_word + 1; i < fset_high; i++)
    bv[i] = 0;

  if (low_word == up_word) {
    bv[low_word] = toTheLowerEnd[up_bit] & toTheUpperEnd[low_bit];
  } else {
    bv[low_word] = toTheUpperEnd[low_bit];
    for (i = low_word + 1; i < up_word; i++)
      bv[i] = int(~0);
    bv[up_word] = toTheLowerEnd[up_bit];
  }
  return to - from + 1;
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


static
OZ_Term getAsList(const int * bv, int neg = 0, int other = 0)
{
  LTuple * hd = NULL, * l_ptr = NULL;
  int len = mkRaw(fd_bv_left_conv, fd_bv_right_conv, bv, neg);

#ifdef BIGFSET
  if ((other && !neg) || (neg && !other)) {
    if (fd_bv_right_conv[len-1] == fset_high*32 - 1) {
      fd_bv_right_conv[len-1] = fs_sup;
    } else {
      fd_bv_left_conv[len] = fset_high*32;
      fd_bv_right_conv[len] = fs_sup;
      len++;
    }
  }
#endif

  for (int i = 0; i < len; i += 1) {
    if (fd_bv_left_conv[i] == fd_bv_right_conv[i]) {
      l_ptr = mkListEl(hd, l_ptr, OZ_int(fd_bv_left_conv[i]));
    } else {
      l_ptr = mkListEl(hd, l_ptr, oz_pairII(fd_bv_left_conv[i],
					    fd_bv_right_conv[i]));
    }
  }

  return hd ? makeTaggedLTuple(hd) : oz_nil();
}

#ifndef BIGFSET

static
void setBits(OZ_Term t, int high, int * bv, int neg = 0)
{
  DEREF(t, tptr);

  if (oz_isSTuple(t) && tagged2SRecord(t)->getWidth() == 1) {
    setBits((*tagged2SRecord(t))[0], high, bv, !neg);
  } else {
    if (oz_isSmallInt(t)) {
      int v = OZ_intToC(t);
      if (0 <= v && v < 32 * high)
	bv[div32(v)] |= (1 << mod32(v));
    } else if (OZ_isNil(t)) {
      ;
    } else if (oz_isSTuple(t)) {
      SRecord &st = *tagged2SRecord(t);
      OZ_Term t0 = oz_deref(st[0]), t1 = oz_deref(st[1]);

      int l = OZ_intToC(t0);
      int r = OZ_intToC(t1);

      if (l <= r)
	for (int i = l; i <= r; i += 1)
	  if (0 <= i && i < 32 * high)
	    bv[div32(i)] |= (1 << mod32(i));
    } else if (OZ_isCons(t)) {
      for (; OZ_isCons(t); t = OZ_tail(t)) {
	OZ_Term vt = OZ_head(t);
	DEREF(vt, vtptr);

	if (oz_isSmallInt(vt)) {
	  int v = OZ_intToC(vt);
	  if (0 <= v && v < 32 * high)
	    bv[div32(v)] |= (1 << mod32(v));
	} else if (oz_isSTuple(vt)) {
	  SRecord &t = *tagged2SRecord(vt);
	  OZ_Term t0 = oz_deref(t[0]), t1 = oz_deref(t[1]);

	  int l = OZ_intToC(t0);
	  int r = OZ_intToC(t1);

	  if (l <= r)
	    for (int i = l; i <= r; i += 1)
	      if (0 <= i && i < 32 * high)
		bv[div32(i)] |= (1 << mod32(i));
	} else {
	  OZD_error("Unexpected case when creating finite set.");
	}
      }
    } else {
      OZD_error("Unexpected case when creating finite set.");
    }

    if (neg)
      for (int i = high; i--; )
	bv[i] = ~bv[i];
  }
}
#endif

static
void printBits(ostream &o, int high, const int * bv, int neg = 0, int other=0)
{
  int len = mkRaw(fd_bv_left_conv, fd_bv_right_conv, bv, neg);

#ifdef BIGFSET
  // fill in "other" intervals:
  if ((other && !neg) || (!other && neg)) {
    if (fd_bv_right_conv[len-1] == 32 * fset_high -1) {
      // prolong interval up to fs_sup
      fd_bv_right_conv[len-1] = fs_sup;
    }
    else {
      // one interval more, starting at 32 * fset_high
      fd_bv_left_conv[len] = 32 * fset_high;
      fd_bv_right_conv[len] = fs_sup;
      len++;
    }
  }
#endif

  Bool flag = FALSE;

#ifdef DEBUG_FSET_CONSTRREP
  if (len == 0) {
    o << "nil";
    return;
  }
  o << '[';
#else
  o << '{';
#endif
  for (int i = 0; i < len; i += 1) {
    if (flag) o << ' '; else flag = TRUE;
    o << fd_bv_left_conv[i];
    if (fd_bv_left_conv[i] != fd_bv_right_conv[i]) {
      if (fd_bv_left_conv[i] + 1 == fd_bv_right_conv[i]) {
	o << ' ' << fd_bv_right_conv[i];
      } else {
	o << '#' << fd_bv_right_conv[i];
      }
    }
  }
#ifdef DEBUG_FSET_CONSTRREP
  o << ']';
#else
  o << '}';
#endif
}

// for conversions:
// having these two constantly allocated saves a lot of time in
// comparison to dynamically allocating them when needed. The space
// tradeoff is neglegible.

OZ_FiniteDomain _Auxin;
OZ_FiniteDomain _Auxout;

inline
void set_Auxin(const int *bv, bool other) {
  if (other)
    _Auxin.initRange(32 * fset_high, fs_sup);
  else
    _Auxin.initEmpty();
  for (int i = 0; i < 32*fset_high; i++)
    if (testBit(bv, i)) _Auxin += i;
}

inline
void set_Auxout(const int *bv, bool other) {
  if (other)
    _Auxout.initRange(32 * fset_high, fs_sup);
  else
    _Auxout.initEmpty();
  for (int i = 0; i < 32*fset_high; i++)
    if (testBit(bv, i)) _Auxout += i;
}

// ----------------------------------------------------------------------------
// FSETVALUE

inline
void FSetValue::copyExtension() {
#ifdef BIGFSET
  if (!_normal) {
    _IN.copyExtension();
  }
#endif
}

inline
void FSetValue::disposeExtension() {
#ifdef BIGFSET
  if (!_normal) {
    _IN.disposeExtension();
  }
#endif
}


inline
void FSetValue::DP(const char *s = NULL) const {
  // this one is ONLY inserted when debug is on.
  printf("fsc (");
  if (s) {
    printf("%s", s);
  }
  else {
    printf("this");
  }

#ifdef BIGFSET

  if (_normal) {
    set_Auxin(_in, _other);
    printf("n) in: %s#%d\n", _Auxin.toString(), _card);
  }
  else {
    printf("x) in: %s#%d\n", _IN.toString(), _card);
  }

#else
  set_Auxin(_in, 0);
  printf("s) in: %s#%d\n", _Auxin.toString(), _card);
#endif
  fflush(stdout);
}

inline
void FSetValue::toNormal(void)
{
#ifdef BIGFSET
  Assert(_normal == false);

  // reset bv
  {
    for (int i = fset_high; i--; )
      _in[i] = 0;
  }
  // set bv
  for (int i = _IN.getMinElem();
       ((i != -1) && (i < 32 * fset_high));
       i = _IN.getNextLargerElem(i))
    setBit(_in, i);

  if (_IN.getUpperIntervalBd(32 * fset_high) == fs_sup)
    // would be a bit faster + correct in normal context, but insecure:
    // if (_IN.getMaxElem() == fs_sup)
    _other = true;
  else
    _other = false;

  _normal = true;

#else
  OZD_error("FSetValue::toNormal called with small sets!\n");
#endif
}

inline
void FSetValue::toExtended(void)
{
#ifdef BIGFSET
  Assert(_normal == true);

  // init trivial part
  if (_other)
    _IN.initRange(32 * fset_high, fs_sup);
  else
    _IN.initEmpty();

  // init nontrivial part
  for (int i = 32 * fset_high; i--; )
    if (testBit(_in, i))
      _IN += i;

  _normal = false;

#else
  OZD_error("FSetValue::toExtended called with small sets!\n");
#endif
}

inline
bool FSetValue::maybeToNormal(void)
{
#ifdef BIGFSET
  Assert(_normal == false);

  int i = _IN.getMaxElem();

  if ((i >= 32 * fset_high) && (i < fs_sup)) {
    return false; // sup above boundary, but below fs_sup
  }
  if ((i >= 32 * fset_high) &&
      ((_IN.getLowerIntervalBd(fs_sup) > 32 * fset_high))) {
    return false; // hole above "simple boundary"
  }

  toNormal();
  return true;
#else
  OZD_error("FSetValue::maybeToNormal called with small sets!\n");
  return false;
#endif
}

inline
OZ_Boolean FSetValue::isIn(int i) const
{
#ifdef BIGFSET
  if (i < 0 || i > fs_sup)
    return OZ_FALSE;

  if (_normal) {
    if (i < 32 * fset_high)
      return testBit(_in, i);
    else
      return _other;
  }
  else
    return _IN.isIn(i);

#else
  return testBit(_in, i);
#endif
}

inline
void FSetValue::init(const FSetConstraint &fs)
{
  Assert(fs.isValue());

  _card = fs._card_min;

  FSDEBUG(printf("fsv::init(fsc) "); fs.DP(""); printf(" -> "));

#ifdef BIGFSET


  if (fs._normal) {
    _normal = true;
    for (int i = fset_high; i--; )
      _in[i] = fs._in[i];
    _other = fs._otherin;
  }
  else {
    _normal = false;
    _IN = fs._IN;
  }

#else
    for (int i = fset_high; i--; )
      _in[i] = fs._in[i];
#endif
    FSDEBUG(DP());
}

inline
void FSetValue::init(const OZ_Term t)
{
  FSDEBUG(printf("fsv::init(ozterm) -> "));
#ifdef BIGFSET

  _normal = false;
  _card = _IN.initDescr(t);
  maybeToNormal();
  FSDEBUG(DP());

#else
  for (int i = fset_high; i--; )
    _in[i] = 0;

  setBits(t, fset_high, _in);

  _card = findBitsSet(fset_high, _in);

  FSDEBUG(DP());

#endif
}

inline
void FSetValue::init(OZ_FSetState s)
{
  FSDEBUG(printf("fsv::init(state=%d) -> ", s));
#ifdef BIGFSET

  switch(s) {
  case fs_empty: {
    _normal = true;
    _other = false;
    for (int i = fset_high; i--; )
      _in[i] = 0;
    _card = 0;
    break;
  }
  case fs_full: {
    _normal = true;
    _other = true;
    for (int i = fset_high; i--; )
      _in[i] = ~0;
    _card = fs_sup + 1;
    break;
  }
  default:
    DebugCode(OZ_error("Unexpected case (%d) in \"FSetValue::init(OZ_FSetState\".", s););
  }
#else
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
    DebugCode(OZ_error("Unexpected case (%d) in \"FSetValue::init(OZ_FSetState\".", s););
  }
#endif
  FSDEBUG(DP());
}

inline
void FSetValue::init(int min_elem, int max_elem)
{
  FSDEBUG(printf("fsv::init(%d, %d)", min_elem, max_elem));
#ifdef BIGFSET

  // normal <-> max_elem < 32*fset_high or max_elem=fs_sup and no hole
  if ((max_elem < 32*fset_high) ||
      ((max_elem == fs_sup) && (min_elem <= 32 * fset_high))) {
    _card = setFromTo(_in, min_elem, max_elem);
    _normal = true;
    _other = (max_elem == fs_sup);
  }
  else {
    _card = _IN.initRange(min_elem, max_elem);
    _normal = false;
    _other = false;
#ifdef DEBUG
     if (maybeToNormal())
       OZD_error("big fsets: unexpected toNormal conversion\n");
#endif
  }

#else
  _card = setFromTo(_in, min_elem, max_elem);
#endif
  FSDEBUG(DP());
}

inline
void FSetValue::init(const OZ_FiniteDomain &fd)
{
  FSDEBUG(printf("fsv::init(fd)..."); printf("FD: %s\n", fd.toString()));
#ifdef BIGFSET
  _card = fd.getSize();

  if (_card == 0) {
    init(fs_empty);
  } else {
    //_IN.initDescr(fd.getDescr());
    // well...
    _IN = fd;

    _normal = false;

    maybeToNormal();
  }
#else
  if (fd.getMaxElem() >= fsethigh32) // tmueller!
    OZ_warning("Max elem of `fd' is to big (%s:%s)", __FILE__, __LINE__);

  for (int i = fset_high; i--; )
    _in[i] = 0;
  _card = 0;

  for (int next_elem = fd.getMinElem(); next_elem > -1;
       next_elem = fd.getNextLargerElem(next_elem)) {
    _card += 1;
    setBit(_in, next_elem);
  }
#endif
  FSDEBUG(printf("fsv::init(fd) -> "); DP());
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
FSetValue::FSetValue(const OZ_FiniteDomain &fd) {
  init(fd);
}


inline
FSetValue::FSetValue(const int * in, bool oth = false)
{
  FSDEBUG(printf("fsv::fsv(int*, bool %d) -> ", oth));
#ifdef BIGFSET
  _normal = true;
  for (int i = fset_high; i--; )
    _in[i] = in[i];
  _other = oth;
  _card = findBitsSet(fset_high, _in);

  if (_other) _card += size_of_other;

#else
  for (int i = fset_high; i--; )
    _in[i] = in[i];

  _card = findBitsSet(fset_high, _in);
#endif
  FSDEBUG(DP());
}


inline
OZ_Boolean FSetValue::operator == (const FSetValue &s) const
{
  FSDEBUG(printf("fsv::op== "); DP(); s.DP("s"));

#ifdef BIGFSET

  if (_card != s._card)
    return FALSE;

  // two sets are equal <=> they are equal in size and their
  // intersection has the same size.

  if (_normal == true) {
    if ((s._normal == false) || (_other != s._other))
      return FALSE;
    for (int i = fset_high; i--; ) {
      if (_in[i] != s._in[i])
	return FALSE;
    }
  }
  else {
    if (_IN.getSize() != s._IN.getSize())
      return OZ_FALSE;
    if ((_IN & s._IN).getSize() != _IN.getSize())
      return OZ_FALSE;
  }

#else
  if (_card != s._card)
    return FALSE;

  for (int i = fset_high; i--; ) {
    if (_in[i] != s._in[i])
      return FALSE;
  }
#endif
  FSDEBUG(printf("isequal\n"));
  return TRUE;
}

inline
OZ_Boolean FSetValue::operator <= (const FSetValue &s) const
{
  FSDEBUG(printf("fsv::op<=(fsv)\n"); DP(); s.DP("s"));

  if (_card > s._card) {
    return FALSE;
  }

  // all elements in '*this' must be in 'fs'.
#ifdef BIGFSET

  if (_normal && s._normal) {
    if (_other && !s._other)
      return FALSE;

    for (int i = fset_high; i--; ) {
      if ((_in[i] & s._in[i]) != _in[i])
	return FALSE;
    }
  }
  else if (!_normal && !s._normal) {
    if ((_IN & s._IN).getSize() != _IN.getSize())
      return FALSE;
  }
  else if (_normal && !s._normal) {
    for (int i = 32 * fset_high; i--; )
      if (testBit(_in, i) && !s._IN.isIn(i))
	return FALSE;

    if (_other)
      if ((!s._IN.isIn(32 * fset_high)) ||
	  (s._IN.getUpperIntervalBd(32 * fset_high) != fs_sup))
	return FALSE;
  }
  else { // !_normal && fs._normal
    for (int i = 32 * fset_high; i--; )
      if (_IN.isIn(i) && !testBit(s._in, i))
	return FALSE;

    if (s._other) {
	if ((!_IN.isIn(32 * fset_high)) ||
	    (_IN.getUpperIntervalBd(32 * fset_high) != fs_sup))
	  return FALSE;
    } else {
      // are there elements in _IN which are >= 32*fset_high?
      // i.e. not in s?
	if (_IN.getNextLargerElem(32*fset_high - 1) >= 0)
	  return FALSE;
    }
  }

#else
  for (int i = fset_high; i--; ) {
    if ((_in[i] & s._in[i]) != _in[i])
      return FALSE;
  }
#endif
  FSDEBUG(printf("<= : true\n"));
  return TRUE;
}

OZ_Boolean FSetValue::unify(OZ_Term t)
{
  DEREF(t, tptr);
  return oz_isFSetValue(t) ? (*((FSetValue *) tagged2FSetValue(t)) == *this) : FALSE;
}

ostream &FSetValue::print2stream(ostream &o) const
{
#ifdef BIGFSET
  if (_normal) {
    printBits(o, fset_high, _in, 0, _other);
  } else {
    ((const OZ_FiniteDomainImpl *) &_IN)->print(o, 0);
  }
#else
  printBits(o, fset_high, _in);
#endif
#ifndef DEBUG_FSET_CONSTRREP
  o << '#' << _card;
#endif
  return o;
}

void FSetValue::print(ostream &stream, int depth, int offset) const
{
  print2stream(stream);
}

inline
FSetValue FSetValue::operator & (const FSetValue &y) const
{
  FSetValue z;
  FSDEBUG(printf("fsv::op&(fsv)..."); DP(); y.DP("y"));
#ifdef BIGFSET

  if (_normal && y._normal) {
    z._normal = true;

    for (int i = fset_high; i--; )
      z._in[i] = _in[i] & y._in[i];

    z._card = findBitsSet(fset_high, z._in);
    z._other = _other & y._other;
    if (z._other) z._card += size_of_other;
  }
  else if (!_normal && !y._normal) {
    z._normal = false;
    z._IN = _IN & y._IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }
  else if (_normal) { // !y._normal
    z._normal = true;
    z._other = _other;
    for (int i = fset_high; i--; )
      z._in[i] = _in[i];
    z.toExtended();
    z._IN &= y._IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();

  }
  else { // !_normal && y._normal
    z._normal = true;
    z._other = y._other;
    for (int i = fset_high; i--; )
      z._in[i] = y._in[i];
    z.toExtended();
    z._IN &= _IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }

#else
  for (int i = fset_high; i--; )
    z._in[i] = _in[i] & y._in[i];

  z._card = findBitsSet(fset_high, z._in);
#endif
  FSDEBUG(z.DP("z"));
  return z;
}

inline
FSetValue FSetValue::operator | (const FSetValue &y) const
{
  FSetValue z;
  FSDEBUG(printf("fsv::op| "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal && y._normal) {
    z._normal = true;
    for (int i = fset_high; i--; )
      z._in[i] = _in[i] | y._in[i];

    z._card = findBitsSet(fset_high, z._in);
    z._other = _other | y._other;
    if (z._other)
      z._card += size_of_other;
  } else if (!_normal && !y._normal) {
    z._normal = false;
    z._IN = _IN | y._IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  } else if (_normal) { // !y._normal
    z._normal = true;
    z._other = _other;
    for (int i=fset_high; i--; )
      z._in[i] = _in[i];
    z.toExtended();
    z._IN = z._IN | y._IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  } else { // !_normal && y._normal
    z._normal = true;
    z._other = y._other;
    for (int i=fset_high; i--; )
      z._in[i] = y._in[i];
    z.toExtended();
    z._IN = z._IN | _IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }
#else
  for (int i = fset_high; i--; )
    z._in[i] = _in[i] | y._in[i];

  z._card = findBitsSet(fset_high, z._in);
#endif
  FSDEBUG(z.DP("z"));
  return z;
}

inline
FSetValue FSetValue::operator - (const FSetValue &y) const
{
  FSetValue z;
  FSDEBUG(printf("fsv::op-(fsv) "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal & y._normal) {
    z._normal = true;
    for (int i = fset_high; i--; )
      z._in[i] = _in[i] & ~y._in[i];
    z._card = findBitsSet(fset_high, z._in);
    z._other = _other & !y._other;
    if (z._other) z._card += size_of_other;
  }
  else if (!_normal & !y._normal) {
    z._normal = false;
    z._IN = _IN & (~(y._IN));
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }
  else if (_normal) { // !y._normal
    Assert(!y._normal);

    z._normal = true;
    for(int i = fset_high; i--; )
      z._in[i] = _in[i];
    z.toExtended();
    z._IN = z._IN & (~(y._IN));
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }
  else { // !_normal && y._normal
    Assert(!_normal && y._normal);

    z._normal = true;
    z._other = ! y._other;
    for (int i = fset_high; i--; )
      z._in[i] = ~y._in[i]; // !
    z.toExtended();
    z._IN &= _IN;
    z._card = z._IN.getSize();
    z.maybeToNormal();
  }

#else
  for (int i = fset_high; i--; )
    z._in[i] = _in[i] & ~y._in[i];

  z._card = findBitsSet(fset_high, z._in);
#endif
  FSDEBUG(z.DP("z"));
  return z;
}

inline
FSetValue FSetValue::operator &= (const FSetValue &y)
{
  FSDEBUG(printf("fsv::op&=(fsv) "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal && y._normal) {
    for (int i = fset_high; i--; )
      _in[i] &= y._in[i];

    _card = findBitsSet(fset_high, _in);
    _other &= y._other;
    if (_other) _card += fs_sup - 32*fset_high + 1;
  }
  else if (!_normal && !y._normal) {
    _IN &= y._IN;
    _card = _IN.getSize();
    maybeToNormal();
  }
  else if (_normal) { // !y._normal
    toExtended();
    _IN &= y._IN;
    _card = _IN.getSize();
    maybeToNormal();
  }
  else { // !_normal && y._normal
    // have to copy y for conversion since it is const
    OZ_FiniteDomain aux = _IN;
    _normal = true;
    _other = y._other;
    for (int i = fset_high; i--; )
      _in[i] = y._in[i];
    toExtended();
    _IN &= aux;
    _card = _IN.getSize();
    maybeToNormal();
  }

#else
  for (int i = fset_high; i--; )
    _in[i] &= y._in[i];

  _card = findBitsSet(fset_high, _in);
#endif
  FSDEBUG(DP());
  return *this;
}

inline
FSetValue FSetValue::operator |= (const FSetValue &y)
{
  FSDEBUG(printf("fsv::op|=(fsv) "); DP(); y.DP());
#ifdef BIGFSET

  if (_normal && y._normal) {
    for (int i = fset_high; i--; )
      _in[i] |= y._in[i];

    _card = findBitsSet(fset_high, _in);
    _other |= y._other;
    if (_other)
      _card += fs_sup - 32*fset_high + 1;
  } else if (!_normal && !y._normal) {
    _IN = _IN | y._IN;
    _card = _IN.getSize();
    maybeToNormal();
  } else if (_normal) { // !y._normal
    toExtended();
    _IN = _IN | y._IN;
    _card = _IN.getSize();
    maybeToNormal();
  } else { // !_normal && y._normal
    // have to copy y for conversion since it is const
    OZ_FiniteDomain aux = _IN;
    _normal = true;
    _other = y._other;
    for (int i = fset_high; i--; )
       _in[i] = y._in[i];
    toExtended();
    _IN = _IN | aux;
    _card = _IN.getSize();
    maybeToNormal();
  }

#else
  for (int i = fset_high; i--; )
    _in[i] |= y._in[i];

  _card = findBitsSet(fset_high, _in);
#endif
  FSDEBUG(DP());
  return *this;
}

inline
FSetValue FSetValue::operator &= (const int y)
{
  FSDEBUG(printf("fsv::op&=(%d) ", y); DP());
#ifdef BIGFSET

  if (_normal) {
    int tb = testBit(_in, y);
    init(fs_empty);
    if (tb) {
      setBit(_in, y);
      _card = 1;
    }
  }
  else {
    if (_IN.isIn(y))
      _card = _IN.initSingleton(y);
    else
      _card = _IN.initEmpty();

  }

#else

  OZ_Boolean tb = testBit(_in, y);
  init(fs_empty);

  if (tb) {
    setBit(_in, y);
    _card =1;
  }
#endif
  FSDEBUG(DP());
  return *this;
}

inline
FSetValue FSetValue::operator += (const int y)
{
  FSDEBUG(printf("fsv::op+=%d ", y); DP());
#ifdef BIGFSET

  if (0 <= y || y > fs_sup) return *this;
  if (_normal) {
    if (y < 32 * fset_high) {
      setBit(_in, y);
      _card = findBitsSet(fset_high, _in);
      if (_other) _card += fs_sup - 32*fset_high + 1;
    }
    else {
      // if other is set, nothing will happen anyway; so no "toNormal" at end.
      if (!_other) {
	toExtended();
	_card = _IN += y;
      }
    }
  }
  else { // !_normal
    _card = _IN += y;
    maybeToNormal();
  }
#else

  if (0 <= y && y < 32*fset_high)
    setBit(_in, y);

  _card = findBitsSet(fset_high, _in);
#endif
  FSDEBUG(DP());
  return *this;
}

inline
FSetValue FSetValue::operator -= (const int y)
{
  FSDEBUG(printf("fsv::op-=%d ", y); DP());
#ifdef BIGFSET

  if (0 <= y || y > fs_sup) return *this;
  if (_normal) {
    if (y < 32 * fset_high) {
      resetBit(_in, y);
      _card = findBitsSet(fset_high, _in);
      if (_other) _card += fs_sup - 32*fset_high + 1;
    }
    else {
      if (_other) { // otherwise, nothing will happen.
	toExtended();
	_card = _IN -= y;
	maybeToNormal();
      }
    }
  }
  else {
    _card = _IN -= y;
    maybeToNormal();
  }
#else
  if (0 <= y && y < 32*fset_high)
    resetBit(_in, y);

  _card = findBitsSet(fset_high, _in);
#endif
  FSDEBUG(DP());
  return *this;
}

inline
FSetValue FSetValue::operator - (void) const
{
  FSetValue z;
  FSDEBUG(printf("fsv::op-() "); DP());
#ifdef BIGFSET

  if (_normal) {
    z._normal = true;
    for (int i = fset_high; i--; )
      z._in[i] = ~_in[i];

    z._card = findBitsSet(fset_high, z._in);
    z._other = !_other;
    if (z._other) z._card += fs_sup - 32*fset_high + 1;

  }
  else {
    z._normal = false;
    z._IN = ~_IN;
    z._card = z._IN.getSize();
    if (z.maybeToNormal()) // won't succeed ?
      OZ_warning("fsv::op- : strange behaviour.\n");
  }

#else

  for (int i = fset_high; i--; )
    z._in[i] = ~_in[i];

  z._card = findBitsSet(fset_high, z._in);
#endif
  FSDEBUG(z.DP("z"));
  return z;
  // was: return *this; an error.
}

inline
OZ_Term FSetValue::getKnownInList(void) const
{
#ifdef BIGFSET
  if (_normal)
    return getAsList(_in, 0, _other);
  else
    return _IN.getDescr();
#else
  return getAsList(_in);
#endif
}

inline
OZ_Term FSetValue::getKnownNotInList(void) const
{
#ifdef BIGFSET
  if (_normal)
    return getAsList(_in, 1, _other);
  else
    return (~_IN).getDescr();
#else
  return getAsList(_in, 1);
#endif
}

// returns -1 if there is no min element
inline
int FSetValue::getMinElem(void) const
{
#ifdef BIGFSET
  if (_normal) {
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
    return (_other) ? 32*fset_high : -1;
  }
  else
    return _IN.getMinElem();
#else
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
#endif
}

// returns -1 if there is no max element
inline
int FSetValue::getMaxElem(void) const
{
#ifdef BIGFSET
  if (_normal) {
    if (_other)
      return fs_sup;
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
    return  -1;
  }
  else
    return _IN.getMaxElem();
#else
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
#endif
}

inline
int FSetValue::getNextLargerElem(int v) const
{
#ifdef BIGFSET
  if (_normal) {
    if ((v >= 32*fset_high - 1) && _other)
      return (v < fs_sup)? v + 1 : -1;
    else {
      for (int new_v = v + 1; new_v <= 32 * fset_high - 1; new_v += 1)
	if (testBit(_in, new_v))
	  return new_v;
      return -1;
    }
  }
  else // !_normal
    return _IN.getNextLargerElem(v);

#else
  for (int new_v = v + 1; new_v <= 32 * fset_high - 1; new_v += 1)
    if (testBit(_in, new_v))
      return new_v;

  return -1;
#endif
}

inline
int FSetValue::getNextSmallerElem(int v) const
{
#ifdef BIGFSET
  if (_normal) {
    if ((v > 32 * fset_high) && _other)
      return (v <= fs_sup) ? v - 1 : -1;
    else {
      for (int new_v = v - 1; new_v >= 0; new_v -= 1)
	if (testBit(_in, new_v))
	  return new_v;

      return -1;
    }
  }
  else
    return _IN.getNextSmallerElem(v);
#else
  for (int new_v = v - 1; new_v >= 0; new_v -= 1)
    if (testBit(_in, new_v))
      return new_v;

  return -1;
#endif
}

// ----------------------------------------------------------------------------
// FSETCONSTRAINT

inline
void FSetConstraint::copyExtension() {
#ifdef BIGFSET
  if (!_normal) {
    _IN.copyExtension();
    _OUT.copyExtension();
  }
#endif
}

inline
void FSetConstraint::disposeExtension() {
#ifdef BIGFSET
  if (!_normal) {
    _IN.disposeExtension();
    _OUT.disposeExtension();
  }
#endif
}


inline
void FSetConstraint::DP(const char *s = NULL) const {
  // this one is only included if DEBUG is set.
  printf("fsc (");
  if (s) {
    printf("%s", s);
  }
  else {
    printf("this");
  }

#ifdef BIGFSET
  if (_normal) {
    set_Auxin(_in, _otherin);
    set_Auxout(_not_in, _otherout);
    printf("|n) in: %s#%d ", _Auxin.toString(), _known_in); fflush(stdout);
    printf("out: %s#%d cmin:%d cmax:%d\n", _Auxout.toString(), _known_not_in,
	   _card_min, _card_max);

  }
  else {
    printf("|x) in: %s#%d ", _IN.toString(), _known_in); fflush(stdout);
    printf("out: %s#%d cmin:%d cmax:%d\n", _OUT.toString(), _known_not_in,
	   _card_min, _card_max);
  }
#else
  set_Auxin(_in, 0);
  set_Auxout(_not_in, 0);
  printf("|n) in: %s#%d ", _Auxin.toString(), _known_in); fflush(stdout);
  printf("out: %s#%d cmin:%d cmax:%d\n", _Auxout.toString(), _known_not_in,
	 _card_min, _card_max);
#endif
  fflush(stdout);
}

#ifdef BIGFSET
inline
void FSetConstraint::toNormal(void)
{
  Assert(_normal == false);

  // reset:
  {
    for (int i = fset_high; i--; )
      _in[i] = _not_in[i] = 0;
  }

  for (int i = 0; i < 32 * fset_high; i++) {
    if (_IN.isIn(i)) setBit(_in, i);
    if (_OUT.isIn(i)) setBit(_not_in, i);
  }
  _otherin = (_IN.getUpperIntervalBd(32 * fset_high) == fs_sup);
  _otherout = (_OUT.getUpperIntervalBd(32 * fset_high) == fs_sup);
  _normal = true;
}

inline
void FSetConstraint::toExtended(void)
{
  Assert(_normal == true);

  if (_otherin)
    _IN.initRange(32 * fset_high, fs_sup);
  else
    _IN.initEmpty();
  if (_otherout)
    _OUT.initRange(32 * fset_high, fs_sup);
  else
    _OUT.initEmpty();
  for (int i = 0; i < 32 * fset_high; i++) {
    if (testBit(_in, i)) _IN += i;
    if (testBit(_not_in, i)) _OUT += i;
  }
  _normal = false;
}

inline
bool FSetConstraint::maybeToNormal(void)
{
  Assert(_normal == false);

  // _IN and _OUT must both be convertable.
  int i = _IN.getMaxElem();
  int j = _OUT.getMaxElem();

  if ((i >= 32 * fset_high) && (i < fs_sup)) {
    return false; // supremum above boundary, but below fs_sup
  }

  if ((j >= 32 * fset_high) && (j < fs_sup)) {
    return false; // supremum above boundary, but below fs_sup
  }

  if ((i >= 32 * fset_high) &&
      ((_IN.getLowerIntervalBd(fs_sup) > 32 * fset_high))) {
    return false; // hole above "simple boundary"
  }

  if ((j >= 32 * fset_high) &&
      ((_OUT.getLowerIntervalBd(fs_sup) > 32 * fset_high))) {
    return false; // hole above "simple boundary"
  }

  toNormal();
  return true;
}
#endif // BIGFSET

FSetConstraint::FSetConstraint(int c_min, int c_max,
			       OZ_Term ins, OZ_Term outs)
{
  FSDEBUG(printf("fsc(cmin%d,cmax%d,interm,outterm) -> ", c_min, c_max));
  _card_min = c_min;
  _card_max = c_max;

#ifdef BIGFSET

  _normal = false;
  _known_in = _IN.initDescr(ins);
  _known_not_in = _OUT.initDescr(outs);

  if ((_IN & _OUT).getSize() != 0) {
    _card_min = -1;
    return;
  }

  maybeToNormal();
#else

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
#endif

  if (_card_max < _known_in || _card_max < _card_min)
    _card_min = -1;
  FSDEBUG(DP());
}


FSetConstraint::FSetConstraint(OZ_Term minimal_in, OZ_Term maximal_in)
{
  FSDEBUG(printf("fsc(minterm, maxterm) -> "));
#ifdef BIGFSET

  _normal = false;
  _card_min = _known_in = _IN.initDescr(minimal_in);

  _OUT.initDescr(maximal_in);
  _OUT = ~_OUT;

  _known_not_in = _OUT.getSize();
  _card_max = fs_sup - _known_not_in + 1;

  maybeToNormal();
#else

  int i;
  for (i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;

  setBits(minimal_in, fset_high, _in);
  setBits(maximal_in, fset_high, _not_in, 1);

  for (i = fset_high; i--; )
    if (_in[i] & _not_in[i]) {
      _card_min = -1;
      return;
    }

  _card_min = _known_in = findBitsSet(fset_high, _in);
  _known_not_in = findBitsSet(fset_high, _not_in);
  _card_max = 32*fset_high - _known_not_in;
#endif
  Assert(_card_min <= _card_max);
  FSDEBUG(DP());
}

inline
void FSetConstraint::init(void)
{
  _known_in = _known_not_in = _card_min = 0;

  // careful (fset_sup originally used, used too in cpi.icc,
  // fs_sup is defined statically in mozart_cpi.hh, fset_sup dynamically
  // in fset.hh)

  FSDEBUG(printf("fsc::init() => "));
#ifdef BIGFSET

  _normal = true;
  _otherin = _otherout = false;
  for (int i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;
  _card_max = fs_sup + 1;

#else
  for (int i = fset_high; i--; )
    _in[i] = _not_in[i] = 0;
  _card_max = fset_sup + 1;
#endif
  FSDEBUG(DP());
}

inline
void FSetConstraint::init(const FSetValue &s)
{
  _known_in = _card_min = _card_max = s._card;

  FSDEBUG(printf("fsc::init(fsv) "); s.DP("s"));

#ifdef BIGFSET

  if (s._normal) {
    _normal = true;
    _otherin = s._other;
    _otherout = !s._other;
    for (int i = fset_high; i--; )
      _not_in[i] = ~(_in[i] = s._in[i]);
  }
  else { // !s._normal
    _normal = false;
    _IN = s._IN;
    _OUT = ~_IN;

  }

  _known_not_in = fs_sup - _known_in + 1 ;

#else
  _known_not_in = 32 * fset_high - _known_in ;
  for (int i = fset_high; i--; )
    _not_in[i] = ~(_in[i] = s._in[i]);
#endif
  FSDEBUG(printf(" -> "); DP());
}

inline
void FSetConstraint::init(OZ_FSetState s)
{
  FSDEBUG(printf("fsc::init(state=%d) -> ", s));
#ifdef BIGFSET

  switch(s) {
  case fs_empty: {
    _normal = true;
    _otherin = false;
    _otherout = true;
    for (int i = fset_high; i--; ) {
      _in[i] = 0;
      _not_in[i] = ~0;
    }
    _card_min = _card_max = _known_in = 0;
    _known_not_in = fs_sup + 1;
    break;
  }
  case fs_full: {
    _normal = true;
    _otherin = true;
    _otherout = false;
    for (int i = fset_high; i--; ) {
      _in[i] = ~0;
      _not_in[i] = 0;
    }
    _card_min = _card_max = _known_in = fs_sup + 1;
    _known_not_in = 0;
    break;
  }
  default:
    DebugCode(OZ_error("Unexpected case (%d) in \"FSetConstraint::init(OZ_FSetState\".", s));
  }
#else
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
    DebugCode(OZ_error("Unexpected case (%d) in \"FSetConstraint::init(OZ_FSetState\".", s));
  }
#endif
  FSDEBUG(DP());
}

inline
void FSetConstraint::init(const FSetConstraint &y)
{
  FSDEBUG(printf("fsc::init(fsc)\n"); y.DP());
#ifdef BIGFSET

  _normal = y._normal;
  if (y._normal) {
    for (int i = fset_high; i--; ) {
      _in[i] = y._in[i];
      _not_in[i] = y._not_in[i];
    }
    _otherin = y._otherin;
    _otherout = y._otherout;
  }
  else {
    _IN = y._IN;
    _OUT = y._OUT;
  }
#else
  for (int i = fset_high; i--; ) {
    _in[i] = y._in[i];
    _not_in[i] = y._not_in[i];
  }
#endif

  _known_in     = y._known_in;
  _known_not_in = y._known_not_in;

  _card_min = y._card_min;
  _card_max = y._card_max;
  FSDEBUG(printf(" -> "); DP());
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
FSetConstraint::FSetConstraint(const FSetConstraint &s): OZ_FSetConstraint()
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
#ifdef BIGFSET
  if (_normal) {
    printBits(o, fset_high, _in, 0, _otherin);
  } else {
    ((const OZ_FiniteDomainImpl *) &_IN)->print(o, 0);
  }
#else
  printBits(o, fset_high, _in);
#endif
}

inline
void FSetConstraint::printLub(ostream &o) const
{
#ifdef BIGFSET
  if (_normal) {
    printBits(o, fset_high, _not_in, 1, _otherout);
  } else {
    OZ_FiniteDomain tmp = ~_OUT;
    ((const OZ_FiniteDomainImpl *) &tmp)->print(o, 0);
  }
#else
  printBits(o, fset_high, _not_in, 1);
#endif
}


ostream &FSetConstraint::print(ostream &o) const
{
#ifndef DEBUG_FSET_CONSTRREP
  o << "{";
#else
  o << "lb:";
#endif
  printGlb(o);
#ifndef DEBUG_FSET_CONSTRREP
  o << "..";
#else
  o << " ub:";
#endif
  printLub(o);
#ifndef DEBUG_FSET_CONSTRREP
  o << "}#";
  if (_card_min == _card_max)
    o << _card_min;
  else
    o << '{' << _card_min << '#' << _card_max << '}';
#endif
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
  FSDEBUG(printf("fsc::normalize "); DP());

  OZ_Boolean retval = OZ_FALSE;

  if (!isValid()) {
    FSDEBUG(printf("fsc::normalize reason not valid"); DP());
    goto end;
  }

#ifdef BIGFSET
  if (_normal) {
    if (_otherin & _otherout) {
      _card_min = -1;
      FSDEBUG(printf("fsc::normalize reason 1"); DP());
      goto end;
    }
    for (int i = fset_high; i--; )
      if (_in[i] & _not_in[i]) {
	_card_min = -1;
	FSDEBUG(printf("fsc::normalize reason 2"); DP());
	goto end;
      }
  } else { // !_normal
    if ((_IN & _OUT).getSize() != 0) {
      _card_min = -1;
      FSDEBUG(printf("fsc::normalize reason reason 3"); DP());
      goto end;
    }

    maybeToNormal();
  }

  // maybe we wouldn't need this, since _known_in and _known_not_in
  // could be set immediately in calling functions iff size may change.
  // (but it'd be really tedious, and would not gain much).

  if (_normal) {
    _known_in = findBitsSet(fset_high, _in);
    if (_otherin)
      _known_in += size_of_other;
    _known_not_in = findBitsSet(fset_high, _not_in);
    if (_otherout)
      _known_not_in += size_of_other;
  } else { // extended
    _known_in = _IN.getSize();
    _known_not_in = _OUT.getSize();
  }

  if (_known_in > _card_min)
    _card_min = _known_in;

  if ((fs_sup - _known_not_in + 1) < _card_max)
    _card_max = (fs_sup - _known_not_in + 1);

  // actually redundant, but ...
  if ((_card_max < _known_in) ||
      (_card_min > (fs_sup - _known_not_in + 1)) ||
      (_card_max < _card_min)) {
    _card_min = -1;
    FSDEBUG(printf("fsc::normalize reason 4"); DP());
    goto end;
  }
  // but we can do better
  if (_card_max == _known_in) {
     _card_min = _card_max;
     _known_not_in = (fs_sup - _known_in + 1);
     if (_normal) {
       for (int i=fset_high; i--; )
	 _not_in[i] = ~_in[i];
       _otherout = !_otherin;
     } else {
       _OUT = ~_IN;
       maybeToNormal();
     }
  }
  if (_card_min == (fs_sup - _known_not_in + 1)) {
     _known_in = _card_max = _card_min;
     if (_normal) {
       for (int i=fset_high; i--; )
	 _in[i] = ~_not_in[i];
       _otherin = !_otherout;
     } else {
       _IN = ~_OUT;
       maybeToNormal();
     }
  }

#else // normal fsets
  // check if a value is in and out at the same time
  {
    for (int i = fset_high; i--; )
      if (_in[i] & _not_in[i]) {
	_card_min = -1;
	FSDEBUG(printf("fsc::normalize reason 5"); DP());
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
    FSDEBUG(printf("fsc::normalize reason 6"); DP());
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

#endif

  retval = OZ_TRUE;

end:
  DEBUG_FSET_IR(("%s %s}\n",
		 this->toString(), (retval == OZ_TRUE ? "true" : "false")));

  FSDEBUG(printf("fsc::normalize returns %d and ", retval); DP());
  return retval;
}


OZ_Boolean FSetConstraint::valid(const FSetValue &fs) const
{
  DEBUG_FSET_IR(("{FSIR.'valid' %s %s ", this->toString(), fs.toString()));

  FSDEBUG(printf("fsc::valid(fsv) "); DP(); fs.DP("fs"));

  if (fs._card < _card_min || _card_max < fs._card)
    goto failure;

#ifdef BIGFSET
  {
    // everything in _IN must be in fs._IN
    // everything in fs._IN mustn't be in _OUT

    if (_normal) {
      if (fs._normal) {
	if (_otherin & !fs._other) goto failure;
	if (fs._other & _otherout) goto failure;
	for (int i = fset_high; i--; ) {
	  if (_in[i] & ~fs._in[i]) goto failure;
	  if (_not_in[i] & fs._in[i]) goto failure;
	}
      }
      else { // _normal && !fs._normal
	set_Auxin(_in, _otherin);
	if ((_Auxin & ~fs._IN).getSize()) goto failure;
	set_Auxout(_not_in, _otherout);
	if ((_Auxout & fs._IN).getSize()) goto failure;
      }
    }
    else { // !_normal
      if (fs._normal) {
	set_Auxin(fs._in, fs._other);
	if ((_OUT & _Auxin).getSize()) goto failure;
	if ((_IN & ~_Auxin).getSize()) goto failure;
      }
      else {
	if ((_IN & ~fs._IN).getSize()) goto failure;
	if ((_OUT & fs._IN).getSize()) goto failure;
      }
    }
#else
    {
      for (int i = fset_high; i--; ) {
	if (_in[i] & ~fs._in[i])
	  goto failure;
	if (_not_in[i] & fs._in[i])
	  goto failure;
    }
#endif
    FSDEBUG(printf("valid: true\n"));
    DEBUG_FSET_IR(("true}\n"));
    return OZ_TRUE;
  }
failure:
  FSDEBUG(printf("valid: false\n"));
  DEBUG_FSET_IR(("false}\n"));
  return OZ_FALSE;
}


FSetConstraint FSetConstraint::unify(const FSetConstraint &y) const
{
  DEBUG_FSET_IR(("{FSIR.'unify' %s %s ", this->toString(), y.toString()));

  FSetConstraint z;

  z._card_min = max(_card_min, y._card_min);
  z._card_max = min(_card_max, y._card_max);

  FSDEBUG(printf("fsc::unify\n"); DP(); y.DP("y"));
  if (z._card_max < z._card_min) {
    goto failure;
  }
#ifdef BIGFSET

  // inclusions and exclusions get merged.
  if (_normal) {
    if (y._normal) {
      z._normal = true;
      z._otherin = _otherin | y._otherin;
      z._otherout = _otherout | y._otherout;
      for (int i = fset_high; i--; ) {
	z._in[i] = _in[i] | y._in[i];
	z._not_in[i] = _not_in[i] | y._not_in[i];
      }
      // no overlapping test, normalize does this.
    }
    else { // _normal && !y._normal
      set_Auxin(_in, _otherin);
      set_Auxout(_not_in, _otherout);
      z._normal = false;
      z._IN = _Auxin | y._IN;
      z._OUT = _Auxout | y._OUT;
    }
  }
  else { // !_normal
    if (y._normal) {
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      z._normal = false;
      z._IN = _IN | _Auxin;
      z._OUT = _OUT | _Auxout;
    }
    else { // !_normal && !y._normal
      z._normal = false;
      z._IN = _IN | y._IN;
      z._OUT = _OUT | y._OUT;
    }
  }
#else
  {
    for (int i = fset_high; i--; ) {
      z._in[i]     = _in[i]     | y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
      if (z._in[i] & z._not_in[i]) {
	goto failure;
      }
    }
  }
#endif

  z.normalize();
  FSDEBUG(printf("unify: true "); z.DP("z"));
  return z;

failure:
  DEBUG_FSET_IR(("%s %s}\n", z.toString(), "false"));
  z._card_min = -1;
  FSDEBUG(printf("unify: false\n"));
  return z;
}


OZ_Boolean FSetConstraint::isWeakerThan(FSetConstraint const &y) const
{
  DEBUG_FSET_IR(("{FSIR.'isWeakerThan' %s %s ",
		 this->toString(), y.toString()));
  FSDEBUG(printf("fsc::isweakerthan(fsc)\n"); DP(); y.DP("y"));
  /*
  printf("%s => _known_in=%d, _known_not_in=%d, getCardSize()=%d\n",
	 toString(), _known_in, _known_not_in, getCardSize());
  printf("%s => _y.known_in=%d, y._known_not_in=%d, y.getCardSize()=%d\n",
	 y.toString(), y._known_in, y._known_not_in, y.getCardSize());
  */
  OZ_Boolean ret_val = ((_known_in < y._known_in) ||
			(_known_not_in < y._known_not_in) ||
			(getCardSize() > y.getCardSize()));
  DEBUG_FSET_IR(("%s}\n", (ret_val ? "true" : "false")));
  FSDEBUG(printf("isweakerthan: %d\n", ret_val));
  return ret_val;
}

inline
OZ_Boolean FSetConstraint::isIn(int i) const
{
  FSDEBUG(printf("fsc::isin(%d) ", i); DP());
#ifdef BIGFSET

  DEBUG_FSET_IR(("{FSIR.'isIn' %s %d ", this->toString(), i));

  OZ_Boolean r = OZ_FALSE;

  if (_normal) {
    if (i < 32 * fset_high) {
      r = testBit(_in, i);
    } else {
      r = (i <= fs_sup && _otherin);
    }
  } else {
    r = _IN.isIn(i);
  }

  DEBUG_FSET_IR(("%s}\n", (r ? "true" : "false")));

  return r;
#else
  return testBit(_in, i);
#endif
}

inline
OZ_Boolean FSetConstraint::isNotIn(int i) const
{
  FSDEBUG(printf("fsc::isnotin(%d) ", i); DP());
#ifdef BIGFSET
  DEBUG_FSET_IR(("{FSIR.'isNotIn' %s %d ", this->toString(), i));

  OZ_Boolean r = OZ_FALSE;

  if (_normal) {
    if (i < 32*fset_high) {
      r =  testBit(_not_in, i);
    } else {
      r = (i <= fs_sup && _otherout); // well, semantics. 'i is in _not_in'
    }
  } else {
    r = _OUT.isIn(i);
  }

  DEBUG_FSET_IR(("%s}\n", (r ? "true" : "false")));

  return r;
#else
  return testBit(_not_in, i);
#endif
}

inline
OZ_Boolean FSetConstraint::isEmpty(void) const
{
  DEBUG_FSET_IR(("{FSIR.'isEmpty' %s ", this->toString()));

  FSDEBUG(printf("fsc::isempty "); DP());

  OZ_Boolean r = (isValue() && (_card_min == 0));

  DEBUG_FSET_IR(("%s}\n", (r ? "true" : "false")));

  return r;
}

inline
OZ_Boolean FSetConstraint::isFull(void) const
{
  FSDEBUG(printf("fsc::isfull "); DP());
#ifdef BIGFSET
  DEBUG_FSET_IR(("{FSIR.'isFull' %s ", this->toString()));

  OZ_Boolean r = (isValue() && (_card_min == (fs_sup + 1)));

  DEBUG_FSET_IR(("%s}\n", (r ? "true" : "false")));

  return r;
#else
  return isValue() && (_card_min == (32*fset_high));
#endif
}

inline
OZ_Boolean FSetConstraint::isSubsumedBy(const FSetConstraint &y) const
{
  DEBUG_FSET_IR(("{FSIR.'isSubsumedBy' %s %s ",
		 this->toString(), y.toString()));

  FSDEBUG(printf("fsc::issubsumed "); DP(); y.DP("y"));
#ifdef BIGFSET

  if (isValue()) {
    // All elements known to included in _x_ have to in _y_
    if (_normal) {
      if (y._normal) {
	if (_otherin & !y._otherin)
	  goto end;
	for (int i = fset_high; i--; ) {
	  if (_in[i] & ~y._in[i])
	    goto end;
	}
      } else {
	set_Auxin(_in, _otherin);
	if ((_Auxin & y._IN).getSize() < _known_in)
	  goto end;
      }
    } else {
      if (y._normal) {
	set_Auxin(y._in, y._otherin);
	if ((_IN & _Auxin).getSize() < _known_in)
	  goto end;
      } else {
	if ((_IN & y._IN).getSize() < _known_in)
	  goto end;
      }
    }
  } else if (y.isValue()) {
    // All elements known to be excluded from _x_ have
    // to be excluded from _y_

    // I find this a bit strange.

    if (_normal) {
      if (y._normal) {
	if (!_otherout & y._otherout)
	  goto end;
	for (int i = fset_high; i--; ) {
	  if (~_not_in[i] & y._not_in[i])
	    goto end;
	}
      } else {
	set_Auxout(_not_in, _otherout);
	if ((~_Auxout & y._OUT).getSize())
	  goto end;
      }
    } else {
      if (y._normal) {
	set_Auxout(y._not_in, y._otherout);
	if ((~_OUT & _Auxout).getSize())
	  goto end;
      } else {
	if ((~_OUT & y._OUT).getSize())
	  goto end;
      }
    }
  } else {
    goto end;
  }

#else // normal fsets
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
#endif
  DEBUG_FSET_IR(("true}\n"));
  FSDEBUG(printf("issubsumed: true\n"));
  return OZ_TRUE;
end:
  DEBUG_FSET_IR(("false}\n"));
  FSDEBUG(printf("issubsumed: false\n"));
  return OZ_FALSE;
}

inline
OZ_Term FSetConstraint::getKnownInList(void) const
{
#ifdef BIGFSET
  if (_normal)
    return getAsList(_in, 0, _otherin);
  else
    return _IN.getDescr();
#else
  return getAsList(_in);
#endif
}

inline
OZ_Term FSetConstraint::getKnownNotInList(void) const
{
#ifdef BIGFSET
  if (_normal)
    return getAsList(_not_in, 0, _otherout);
  else
    return _OUT.getDescr();
#else
  return getAsList(_not_in);
#endif
}

inline
OZ_Term FSetConstraint::getUnknownList(void) const
{
#ifdef BIGFSET
  if (_normal) {
    int unknown[fset_high];
    for (int i= fset_high; i--; )
      unknown[i] = ~(_in[i] | _not_in[i]);
    return getAsList(unknown, 0, !(_otherin || _otherout));
  }
  else
    return (~(_IN | _OUT)).getDescr();
#else
  int unknown[fset_high];

  for (int i = fset_high; i--; )
    unknown[i] = ~(_in[i] | _not_in[i]);

  return getAsList(unknown);
#endif
}

inline
OZ_Term FSetConstraint::getLubList(void) const
{
#ifdef BIGFSET
  if (_normal) {
    return getAsList(_not_in, 1, _otherout); // correct?
  }
  else
    return (~_OUT).getDescr();
#else
  return getAsList(_not_in, 1);
#endif
}

inline
OZ_Term FSetConstraint::getCardTuple(void) const
{
  return ((_card_min == _card_max)
	  ? OZ_int(_card_min)
	  : oz_pairII(_card_min, _card_max));
}

inline
OZ_Boolean FSetConstraint::putCard(int min_card, int max_card)
{
  DEBUG_FSET_IR(("{FSIR.'putCard' %s %d %d ",
		 this->toString(), min_card, max_card));

  _card_min = max(min_card, _card_min);
  _card_max = min(max_card, _card_max);

  return normalize();
}

inline
FSetConstraint FSetConstraint::operator - (void) const
{
  DEBUG_FSET_IR(("{FSIR.'operator -u' %s ", this->toString()));

  FSetConstraint z;
  FSDEBUG(printf("fsc::op-() "); DP());

#ifdef BIGFSET
  if (_card_min == -1) {
    z._card_min = -1;
    DEBUG_FSET_IR(("%s %s}\n", z.toString(), "false"));
    return z;
  }

  if (_normal) {
    z._normal = true;
    z._otherin = _otherout;
    z._otherout = _otherin;
    for (int i = fset_high; i--; ) {
      z._in[i] = _not_in[i];
      z._not_in[i] = _in[i];
    }
  } else {
    z._normal = false;
    z._IN = _OUT;
    z._OUT = _IN;
  }

#else
  for (int i = fset_high; i--; ) {
    z._in[i]     = _not_in[i];
    z._not_in[i] = _in[i];
  }
#endif
  z.normalize();
  FSDEBUG(printf("op- -> "); z.DP("z"));
  return z;
}

inline
OZ_Boolean FSetConstraint::operator += (int i)
{
  DEBUG_FSET_IR(("{FSIR.'operator +=' %s %d ", this->toString(), i));
  FSDEBUG(printf("fsc::+=(%d) ", i); DP());

#ifdef BIGFSET

  if (i < 0 || i > fs_sup) {
    DEBUG_FSET_IR(("%s %s}\n", this->toString(), "true"));
    return OZ_TRUE;
  }
  if (_normal) {
    if (i < 32 * fset_high) {
      setBit(_in, i);
    } else {
      if (_otherin) {
	DEBUG_FSET_IR(("%s %s}\n", this->toString(), "true"));
	return OZ_TRUE;
      } else {
	toExtended();
	_IN += i;
      }
    }
  } else {
    _IN += i;
  }
#else
  if (i < 0 || 32 * fset_high <= i)
    return OZ_TRUE;

  setBit(_in, i);
#endif
  FSDEBUG(DP());
  return normalize();
}

inline
OZ_Boolean FSetConstraint::operator -= (int i)
{
  DEBUG_FSET_IR(("{FSIR.'operator -=' %s %d ", this->toString(), i));
  FSDEBUG(printf("fsc::-=(%d) ", i); DP());

#ifdef BIGFSET

  if (i < 0 || i > fs_sup) {
    DEBUG_FSET_IR(("%s %s}\n", this->toString(), "true"));
    return OZ_TRUE;
  }
  if (_normal) {
    if (i < 32 * fset_high) {
      setBit(_not_in, i);
    } else {
      if (_otherout) {
	DEBUG_FSET_IR(("%s %s}\n", this->toString(), "true"));
	return OZ_TRUE;
      } else {
	toExtended();
	_OUT += i;
      }
    }
  } else {
    _OUT += i;
  }
#else
  if (i < 0 || 32 * fset_high <= i)
    return OZ_TRUE;

  setBit(_not_in, i);
#endif
  FSDEBUG(DP());
  return normalize();
}

inline
OZ_Boolean FSetConstraint::operator <<= (const FSetConstraint& y)
{
  DEBUG_FSET_IR(("{FSIR.'operator <<=' %s %s ",
		 this->toString(), y.toString()));

  FSDEBUG(printf("fsc::<<= "); DP(); y.DP("y"));

#ifdef BIGFSET

  // invalidation:
  // no partner may contain elements that are not in the possibilities
  // of the other.
  // y._IN & (_IN | UNK) must be y._IN
  // _IN & (y._IN | y.UNK) must be _IN

  // if that's alright,
  // _IN and _OUT get unioned

  if (_normal) {
    if (y._normal) {
      _otherin |= y._otherin;
      _otherout |= y._otherout;
      for (int i = fset_high; i--; ) {
	_in[i] |= y._in[i];
	_not_in[i] |= y._not_in[i];
      }
    } else {
      toExtended();
      _IN = _IN | y._IN;
      _OUT = _OUT | y._OUT;
    }
  } else {
    if (y._normal) {
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      _IN = _IN | _Auxin;
      _OUT = _OUT | _Auxout;
    } else {
      _IN = _IN | y._IN;
      _OUT = _OUT | y._OUT;
    }
  }
#else
  for (int i = fset_high; i--; ) {
    _in[i]     |= y._in[i];
    _not_in[i] |= y._not_in[i];
  }
#endif

  _card_min = max(_card_min, y._card_min);
  _card_max = min(_card_max, y._card_max);
  FSDEBUG(DP());
  return normalize();
}

inline
OZ_Boolean FSetConstraint::operator % (const FSetConstraint &y)
{
  // new operator; means: not equal

  DEBUG_FSET_IR(("{FSIR.'operator %%' %s %s ", this->toString(), y.toString()));

  FSDEBUG(printf("fsc::op percent "); DP(); y.DP());

#ifdef BIGFSET
  if (_card_min > y._card_max || _card_max < y._card_min)
    return OZ_TRUE;

  // either operand:
  // something that is _IN must be _OUT in the other.

  if (_normal) {
    if (y._normal) {
      if ((_otherin & y._otherout) || (_otherout & y._otherin))
	return OZ_TRUE;
      for (int i = fset_high; i--; )
	if ((_in[i] & y._not_in[i]) || (_not_in[i] & y._in[i]))
	  return OZ_TRUE;
    }
    else {
      set_Auxin(_in, _otherin);
      set_Auxout(_not_in, _otherout);
      if (((_Auxin & y._OUT).getSize()) || ((y._IN & _Auxout).getSize()))
	return OZ_TRUE;
    }
  }
  else {
    if (y._normal) {
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      if (((_IN & _Auxout).getSize()) || ((_Auxin & _OUT).getSize()))
	return OZ_TRUE;
    }
    else {
      if (((_IN & y._OUT).getSize()) || ((y._IN & _OUT).getSize()))
	return OZ_TRUE;
    }
  }
#else

  if (_card_min > y._card_max || _card_max < y._card_min)
    return OZ_TRUE;

  for (int i = fset_high; i--; )
    if ((_in[i] & y._not_in[i]) || (_not_in[i] & y._in[i]))
      return OZ_TRUE;


#endif
  FSDEBUG(printf("fsc::op percent: false\n"));
  return OZ_FALSE;

}


inline
OZ_Boolean FSetConstraint::operator <= (const FSetConstraint &y)
{
  // since _*this_ is subsumed by _y_, _*this_ must contain at least
  // the amount of negative information as _y_ does

  DEBUG_FSET_IR(("{FSIR.'operator <=' %s %s ", this->toString(), y.toString()));
  FSDEBUG(printf("fsc::op<=(fsc) "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal) {
    if (y._normal) {
      _otherout |= y._otherout;
      for (int i = fset_high; i--; ) {
	_not_in[i] |= y._not_in[i];
      }
    } else {
      toExtended();
      _OUT = _OUT | y._OUT;
    }
  } else {
    if (y._normal) {
      set_Auxout(y._not_in, y._otherout);
      _OUT = _OUT | _Auxout;
    } else {
      _OUT = _OUT | y._OUT;
    }
  }
#else
  for (int i = fset_high; i--; )
    _not_in[i] |= y._not_in[i];
#endif
  _card_max = min(_card_max, y._card_max);
  OZ_Boolean retval = normalize();
  FSDEBUG(printf("result <= (%d): ", retval); DP());
  return retval;
}

inline
OZ_Boolean FSetConstraint::operator >= (const FSetConstraint &y)
{
  // since _*this_ subsumes _y_, _*this_ must contain at least
  // the amount of positive information a _y_ does

  DEBUG_FSET_IR(("{FSIR.'operator >=' %s %s ", this->toString(), y.toString()));

  FSDEBUG(printf("fsc::>= "); DP(); y.DP("y"));

#ifdef BIGFSET

  if (_normal) {
    if (y._normal) {
      _otherin |= y._otherin;
      for (int i=fset_high; i--; )
	_in[i] |= y._in[i];
    } else {
      toExtended();
      _IN = _IN | y._IN;
    }
  } else {
    if (y._normal) {
      set_Auxin(y._in, y._otherin);
      _IN = _IN | _Auxin;
    } else {
      _IN = _IN | y._IN;
    }
  }
#else
  for (int i = fset_high; i--; )
    _in[i] |= y._in[i];
#endif

  _card_min = max(_card_min, y._card_min);
  FSDEBUG(DP());

  return normalize();
}

// disjoint
inline
OZ_Boolean FSetConstraint::operator != (const FSetConstraint &y)
{
  DEBUG_FSET_IR(("{FSIR.'operator !=' %s %s ", this->toString(), y.toString()));
  FSDEBUG(printf("fsc::!= "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal) {
    if (y._normal) {
      _otherout |= y._otherin;
      for (int i=fset_high; i--; ) {
	_not_in[i] |= y._in[i];
      }
    } else {
      toExtended();
      _OUT = _OUT | y._IN;
    }
  } else {
    if (y._normal) {
      set_Auxin(y._in, y._otherin);
      _OUT = _OUT | _Auxin;
    } else {
      _OUT = _OUT | y._IN;
    }
  }
#else

  for (int i = fset_high; i--; )
    _not_in[i] |= y._in[i];
#endif
  FSDEBUG(printf("fsc::op!= result:\n"); DP());
  return normalize();
}

inline
OZ_Boolean FSetConstraint::operator == (const FSetConstraint &y) const
{
  DEBUG_FSET_IR(("{FSIR.'operator ==' %s %s ", this->toString(), y.toString()));

  FSDEBUG(printf("fsc::== "); DP(); y.DP("y"));

  if (_card_min != y._card_min ||
      _card_max != y._card_max ||
      _known_not_in != y._known_not_in ||
      _known_in != y._known_in) {

	DEBUG_FSET_IR(("false}\n"));

	return OZ_FALSE;
      }
#ifdef BIGFSET

  if (_normal != y._normal) {
    DEBUG_FSET_IR(("false}\n"));
    return OZ_FALSE;
  }

  if (_normal) {
    if ((_otherin != y._otherin) || (_otherout != y._otherout)) {
      DEBUG_FSET_IR(("false}\n"));
      return OZ_FALSE;
    }
    for (int i = fset_high; i--; ) {
      if (_in[i] != y._in[i] || _not_in[i] != y._not_in[i]) {
	DEBUG_FSET_IR(("false}\n"));
	return OZ_FALSE;
      }
    }
  } else {
    // equal sizes ensured above; additionally: size-idempotent intersection
    if (((_IN & y._IN).getSize() != _known_in) ||
	((_OUT & y._OUT).getSize() != _known_not_in)) {
	  DEBUG_FSET_IR(("false}\n"));

	  return OZ_FALSE;
	}
  }
#else
  for (int i = fset_high; i--; )
    if (_in[i] != y._in[i] || _not_in[i] != y._not_in[i])
      return OZ_FALSE;
#endif
  FSDEBUG(printf("op==: true\n"));
  DEBUG_FSET_IR(("true}\n"));
  return OZ_TRUE;
}

inline
FSetConstraint FSetConstraint::operator & (const FSetConstraint& y) const
{
  DEBUG_FSET_IR(("{FSIR.'operator &' %s %s ", this->toString(), y.toString()));

  FSetConstraint z;

  FSDEBUG(printf("fsc::&(fsc) "); DP(); y.DP("y"));

  if (!isValid() || !y.isValid()) {
    z._card_min = -1;
    DEBUG_FSET_IR(("%s %s}\n", z.toString(), "false"));
    return z;
  }
#ifdef BIGFSET
  if (_normal) {
    if (y._normal) {
      z._normal = true;
      z._otherin = _otherin & y._otherin;
      z._otherout = _otherout | y._otherout;
      for (int i = fset_high; i--; ) {
	z._in[i] =_in[i] & y._in[i];
	z._not_in[i] = _not_in[i] | y._not_in[i];
      }
    } else {
      z._normal = false;
      set_Auxin(_in, _otherin);
      set_Auxout(_not_in, _otherout);
      z._IN = _Auxin & y._IN;
      z._OUT = _Auxout | y._OUT;
    }
  } else {
    if (y._normal) {
      z._normal = false;
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      z._IN = _IN & _Auxin;
      z._OUT = _OUT | _Auxout;
    } else {
      z._normal = false;
      z._IN = _IN & y._IN;
      z._OUT = _OUT | y._OUT;
    }
  }
#else

  {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] & y._in[i];
      z._not_in[i] = _not_in[i] | y._not_in[i];
    }
  }
#endif
  z._card_min = 0;
  z._card_max = min(_card_max, y._card_max);

  FSDEBUG(z.DP("z"));
  z.normalize();
  return z;
}

inline
FSetConstraint FSetConstraint::operator | (const FSetConstraint& y) const
{
  DEBUG_FSET_IR(("{FSIR.'operator |' %s %s ", this->toString(), y.toString()));

  FSetConstraint z;

  FSDEBUG(printf("fsc::|(fsc) "); DP(); y.DP("y"));

  if (!isValid() || !y.isValid()) {
    DEBUG_FSET_IR(("%s %s}\n", z.toString(), "false"));
    z._card_min = -1;
    return z;
  }
#ifdef BIGFSET

  if (_normal) {
    if (y._normal) {
      z._normal = true;
      z._otherin = _otherin | y._otherin;
      z._otherout = _otherout & y._otherout;
      for (int i = fset_high; i--; ) {
	z._in[i] = _in[i] | y._in[i];
	z._not_in[i] = _not_in[i] & y._not_in[i];
      }
    } else {
      z._normal = false;
      set_Auxin(_in, _otherin);
      set_Auxout(_not_in, _otherout);
      z._IN = _Auxin | y._IN;
      z._OUT = _Auxout & y._OUT;
    }
  } else {
    if (y._normal) {
      z._normal = false;
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      z._IN = _IN | _Auxin;
      z._OUT = _OUT & _Auxout;
    } else {
      z._normal = false;
      z._IN = _IN | y._IN;
      z._OUT = _OUT & y._OUT;
    }
  }
#else
  {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] | y._in[i];
      z._not_in[i] = _not_in[i] & y._not_in[i];
    }
  }
#endif

  z._card_min = max(_card_min, y._card_min);
  z._card_max = _card_max + y._card_max;

  FSDEBUG(z.DP("z"));
  z.normalize();
  return z;
}

inline
FSetConstraint FSetConstraint::operator - (const FSetConstraint& y) const
{
  DEBUG_FSET_IR(("{FSIR.'operator -' %s %s ", this->toString(), y.toString()));

  FSDEBUG(printf("fsc::-(fsc) "); DP(); y.DP("y"));

  FSetConstraint z;

  if (!isValid() || !y.isValid()) {
    z._card_min = -1;
    DEBUG_FSET_IR(("%s %s}\n", z.toString(), "false"));
    return z;
  }
#ifdef BIGFSET
  if (_normal) {
    if (y._normal) {
      z._normal = true;
      z._otherin = _otherin & y._otherout;
      z._otherout = _otherout | y._otherin;
      for (int i = fset_high; i--; ) {
	z._in[i] = _in[i] & y._not_in[i];
	z._not_in[i] = _not_in[i] | y._in[i];
      }
    } else {
      z._normal = false;
      set_Auxin(_in, _otherin);
      set_Auxout(_not_in, _otherout);
      z._IN = _Auxin & y._OUT;
      z._OUT = _Auxout | y._IN;
    }
  } else {
    if (y._normal) {
      z._normal = false;
      set_Auxin(y._in, y._otherin);
      set_Auxout(y._not_in, y._otherout);
      z._IN = _IN & _Auxout;
      z._OUT = _OUT | _Auxin;
    } else {
      z._normal = false;
      z._IN = _IN & y._OUT;
      z._OUT = _OUT | y._IN;
    }
  }

#else
  {
    for (int i = fset_high; i--; ) {
      z._in[i] = _in[i] & y._not_in[i];
      z._not_in[i] = _not_in[i] | y._in[i];
    }
  }
#endif

  z._card_min = 0;
  z._card_max = _card_max;

  FSDEBUG(z.DP("z"));
  z.normalize();
  return z;
}

inline
FSetValue FSetConstraint::getGlbSet(void) const
{
#ifdef BIGFSET
  if (_normal)
    return FSetValue(_in, _otherin);
  else
    return FSetValue(_IN);
#else
  return FSetValue(_in);
#endif
}

inline
FSetValue FSetConstraint::getLubSet(void) const
{
#ifdef BIGFSET
  if (_normal) {
    int lub[fset_high];
    for (int i = fset_high; i--; )
      lub[i] = ~_not_in[i];
    return FSetValue(lub, !_otherout);
  }
  else
    return FSetValue(~_OUT);
#else
  int i, lub[fset_high];

  for (i = fset_high; i--; )
    lub[i] = ~_not_in[i];

  return FSetValue(lub);
#endif
}

inline
FSetValue FSetConstraint::getUnknownSet(void) const
{
#ifdef BIGFSET
  if (_normal) {
    int unk[fset_high];
    for (int i = fset_high; i--; )
      unk[i] = ~(_in[i] | _not_in[i]);
    return FSetValue(unk, !(_otherin | _otherout));
  }
  else
    return FSetValue(~(_IN | _OUT));
#else
  int i, unknown[fset_high];

  for (i = fset_high; i--; )
    unknown[i] = ~(_in[i] | _not_in[i]);

  return FSetValue(unknown);
#endif
}

inline
FSetValue FSetConstraint::getNotInSet(void) const
{
#ifdef BIGFSET
  if (_normal)
    return FSetValue(_not_in, _otherout);
  else
    return FSetValue(_OUT);
#else
  return FSetValue(_not_in);
#endif
}


inline
int FSetConstraint::getGlbCard(void) const
{
#ifdef BIGFSET
  if (_normal)
    return findBitsSet(fset_high, _in) + (_otherin ? size_of_other : 0);
  else
    return _IN.getSize();
#else
  return findBitsSet(fset_high, _in);
#endif
}

inline
int FSetConstraint::getLubCard(void) const
{
#ifdef BIGFSET
  if (_normal) {
    return fs_sup + 1 - findBitsSet(fset_high, _not_in)
		  - (_otherout ? size_of_other : 0);
  }
  else
    return fs_sup - _OUT.getSize();
#else
  return (32 * fset_high) - findBitsSet(fset_high, _not_in);
#endif
}

inline
int FSetConstraint::getNotInCard(void) const
{
#ifdef BIGFSET
  if (_normal)
    return findBitsSet(fset_high, _not_in) + (_otherout ? size_of_other : 0);
  else
    return _OUT.getSize();
#else
  return findBitsSet(fset_high, _not_in);
#endif
}

inline
int FSetConstraint::getUnknownCard(void) const
{
#ifdef BIGFSET
  if (_normal)
    return fs_sup - findBitsSet(fset_high, _not_in)
      - findBitsSet(fset_high, _in)
      - ((_otherout | _otherin)? size_of_other : 0);
  else
    return fs_sup - _IN.getSize() - _OUT.getSize();
#else

  return (32 * fset_high)
    - findBitsSet(fset_high, _not_in)
    - findBitsSet(fset_high, _in);
#endif
}

inline
OZ_Boolean FSetConstraint::ge(const int ii)
{
  FSDEBUG(printf("fsc::ge(%d) ", ii); DP());

#ifdef BIGFSET
  if (ii == 0) {
    return normalize(); // frequent special case
  }
  if (_normal) {
    if (ii < 32*fset_high) {
      int lower_word = div32(ii), lower_bit = mod32(ii);

      for (int i = 0; i < lower_word; i += 1)
	_not_in[i] = ~0;
      _not_in[lower_word] |= ~toTheUpperEnd[lower_bit];
    } else {
      toExtended();
      _Auxout.initRange(0, max(ii-1, 0));
      _OUT = _OUT | _Auxout;
      maybeToNormal();
    }
  } else {
    _Auxout.initRange(0, max(ii-1, 0));
    _OUT = _OUT | _Auxout;
  }
#else
  int lower_word = div32(ii), lower_bit = mod32(ii);

  for (int i = 0; i < lower_word; i += 1)
    _not_in[i] = ~0;
  _not_in[lower_word] |= ~toTheUpperEnd[lower_bit];
#endif
  OZ_Boolean retval = normalize();
  FSDEBUG(printf("fsc::ge : normalize returns %d\n", retval));
  return retval;
}

inline
OZ_Boolean FSetConstraint::le(const int ii)
{
  FSDEBUG(printf("fsc::le(%d) ", ii); DP());

#ifdef BIGFSET
  if (ii == fs_sup) {
    return normalize(); // frequent special case.
  }
  if (_normal) {
    if (ii < 32*fset_high) {
      int upper_word = div32(ii), upper_bit = mod32(ii);

      for (int i = upper_word + 1; i < fset_high; i += 1)
	_not_in[i] = ~0;
      _not_in[upper_word] |= ~toTheLowerEnd[upper_bit];
      _otherout = true;
    }
    else {
      toExtended();
      _Auxout.initRange(min(fs_sup, ii + 1), fs_sup);
      _OUT = _OUT | _Auxout;
      maybeToNormal();
    }

  }
  else {
    _Auxout.initRange(min(fs_sup, ii + 1), fs_sup);
    _OUT = _OUT | _Auxout;
  }
#else
  int upper_word = div32(ii), upper_bit = mod32(ii);

  for (int i = upper_word + 1; i < fset_high; i += 1)
    _not_in[i] = ~0;
  _not_in[upper_word] |= ~toTheLowerEnd[upper_bit];
#endif
  OZ_Boolean retval = normalize();
  FSDEBUG(printf("fsc::le : normalize returns %d\n", retval));
  return retval;
}


// the following could maybe be improved

int FSetConstraint::getGlbMinElem(void) const {
#ifdef BIGFSET
  if (!_normal)
    return _IN.getMinElem();
  else
    return getGlbSet().getMinElem();
#else
  return getGlbSet().getMinElem();
#endif
}

inline
int FSetConstraint::getLubMinElem(void) const {
  return getLubSet().getMinElem();
}

inline
int FSetConstraint::getNotInMinElem(void) const {
#ifdef BIGFSET
  if (!_normal)
    return _OUT.getMinElem();
  else
    return getNotInSet().getMinElem();
#else
  return getNotInSet().getMinElem();
#endif
}

inline
int FSetConstraint::getUnknownMinElem(void) const {
  return getUnknownSet().getMinElem();
}

inline
int FSetConstraint::getGlbMaxElem(void) const {
#ifdef BIGFSET
  if (!_normal)
    return _IN.getMaxElem();
  else
    return getGlbSet().getMaxElem();
#else
  return getGlbSet().getMaxElem();
#endif
}

inline
int FSetConstraint::getLubMaxElem(void) const {
  return getLubSet().getMaxElem();
}

inline
int FSetConstraint::getNotInMaxElem(void) const {
#ifdef BIGFSET
  if (!_normal)
    return _OUT.getMaxElem();
  else
    return getNotInSet().getMaxElem();
#else
  return getNotInSet().getMaxElem();
#endif
}

inline
int FSetConstraint::getUnknownMaxElem(void) const {
  return getUnknownSet().getMaxElem();
}

inline
int FSetConstraint::getGlbNextSmallerElem(int i) const {
#ifdef BIGFSET
  if (!_normal)
    return _IN.getNextSmallerElem(i);
  else
    return getGlbSet().getNextSmallerElem(i);
#else
  return getGlbSet().getNextSmallerElem(i);
#endif
}

inline
int FSetConstraint::getLubNextSmallerElem(int i) const {
  return getLubSet().getNextSmallerElem(i);
}

inline
int FSetConstraint::getNotInNextSmallerElem(int i) const {
#ifdef BIGFSET
  if (!_normal)
    return _IN.getNextSmallerElem(i);
  else
    return getNotInSet().getNextSmallerElem(i);
#else
  return getNotInSet().getNextSmallerElem(i);
#endif
}

inline
int FSetConstraint::getUnknownNextSmallerElem(int i) const {
  return getUnknownSet().getNextSmallerElem(i);
}

inline
int FSetConstraint::getGlbNextLargerElem(int i) const {
#ifdef BIGFSET
  if (!_normal)
    return _IN.getNextLargerElem(i);
  else
    return getGlbSet().getNextLargerElem(i);
#else
  return getGlbSet().getNextLargerElem(i);
#endif
}

inline
int FSetConstraint::getLubNextLargerElem(int i) const {
  return getLubSet().getNextLargerElem(i);
}

inline
int FSetConstraint::getNotInNextLargerElem(int i) const {
#ifdef BIGFSET
  if (!_normal)
    return _OUT.getNextLargerElem(i);
  else
    return getNotInSet().getNextLargerElem(i);
#else
  return getNotInSet().getNextLargerElem(i);
#endif
}

inline
int FSetConstraint::getUnknownNextLargerElem(int i) const {
  return getUnknownSet().getNextLargerElem(i);
}

inline
OZ_Boolean FSetConstraint::operator <= (const int i)
{
  _card_max = min(i, _card_max);
  return !normalize();
}

inline
OZ_Boolean FSetConstraint::operator >= (const int i)
{
  _card_min = max(i, _card_min);
  return !normalize();
}

inline
OZ_Boolean FSetConstraint::operator |= (const FSetValue &y)
{
  DEBUG_FSET_IR(("{FSIR.'operator |=' %s %s ",
		 this->toString(), y.toString()));

  FSDEBUG(printf("fsc::|= "); DP(); y.DP("y"));

#ifdef BIGFSET
  if (_normal) {
    if (y._normal) {
      _otherin |= y._other;
      for (int i = fset_high; i--; ) {
	_in[i] |= y._in[i];
      }
      _normal = true;
    } else {
      toExtended();
      _IN = _IN | y._IN;
      _normal = false;
    }
  } else {
    if (y._normal) {
      set_Auxin(y._in, y._other);
      _IN = _IN | _Auxin;
    } else {
      _IN = _IN | y._IN;
    }
    _normal = false;
  }
#else
  for (int i = fset_high; i--; ) {
    _in[i]     |= y._in[i];
  }
#endif

  FSDEBUG(DP());
  return !normalize();
}

inline
OZ_Boolean FSetConstraint::operator &= (const FSetValue & y)
{
  DEBUG_FSET_IR(("{FSIR.'operator &=' %s %s ",
		 this->toString(), y.toString()));

  FSDEBUG(printf("fsc::&= "); DP(); y.DP("y"));

#ifdef BIGFSET
  FSetValue neg_y = -y;
  if (_normal) {
    if (neg_y._normal) {
      _otherout |= neg_y._other;
      for (int i = fset_high; i--; ) {
	_not_in[i] |= neg_y._in[i];
      }
      _normal = true;
    } else {
      toExtended();
      _OUT = _OUT | neg_y._IN;
      _normal = false;
    }
  } else {
    if (neg_y._normal) {
      set_Auxout(neg_y._in, neg_y._other);
      _OUT = _OUT | _Auxout;
    } else {
      _OUT = _OUT | neg_y._IN;
    }
    _normal = false;
  }
#else
  for (int i = fset_high; i--; ) {
    _not_in[i] |= neg_y._in[i];
  }
#endif

  FSDEBUG(DP());
  return !normalize();
}


// ****************************************************************************

#define CASTPTR (FSetValue *)
#define CASTCONSTPTR (const FSetValue *)
#define CASTREF * (const FSetValue *) &
#define CASTTHIS (CASTPTR this)
#define CASTCONSTTHIS (CASTCONSTPTR this)


OZ_FSetValue::OZ_FSetValue(const OZ_FSetConstraint &s)
{
  CASTTHIS->init(* (const FSetConstraint *) &s);
}

OZ_FSetValue::OZ_FSetValue(const OZ_Term t)
{
  CASTTHIS->init(t);
}

OZ_FSetValue::OZ_FSetValue(const OZ_FSetState s)
{
  CASTTHIS->init(s);
}

void OZ_FSetValue::init(OZ_FSetState s)
{
  CASTTHIS->init(s);
}

OZ_FSetValue::OZ_FSetValue(int min_elem, int max_elem)
{
  CASTTHIS->init(min_elem, max_elem);
}

OZ_FSetValue::OZ_FSetValue(const OZ_FiniteDomain &fd)
{
  CASTTHIS->init(fd);
}

OZ_Term OZ_FSetValue::getKnownInList(void) const
{
  return CASTCONSTTHIS->getKnownInList();
}

OZ_Term OZ_FSetValue::getKnownNotInList(void) const
{
  return CASTCONSTTHIS->getKnownNotInList();
}

OZ_Boolean OZ_FSetValue::isIn(int i) const
{
  return CASTCONSTTHIS->isIn(i);
}

OZ_Boolean OZ_FSetValue::isNotIn(int i) const
{
  return CASTCONSTTHIS->isNotIn(i);
}

int OZ_FSetValue::getMinElem(void) const
{
  return CASTCONSTTHIS->getMinElem();
}

int OZ_FSetValue::getMaxElem(void) const
{
  return CASTCONSTTHIS->getMaxElem();
}

int OZ_FSetValue::getNextLargerElem(int i) const
{
  return CASTCONSTTHIS->getNextLargerElem(i);
}

int OZ_FSetValue::getNextSmallerElem(int i) const
{
  return CASTCONSTTHIS->getNextSmallerElem(i);
}

OZ_Boolean OZ_FSetValue::operator == (const OZ_FSetValue &y) const
{
  return CASTCONSTTHIS->operator == (CASTREF y);
}

OZ_Boolean OZ_FSetValue::operator <= (const OZ_FSetValue &y) const
{
  return CASTCONSTTHIS->operator <= (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator & (const OZ_FSetValue &y) const
{
  return CASTCONSTTHIS->operator & (CASTREF y);
}

OZ_FSetValue OZ_FSetValue::operator | (const OZ_FSetValue &y) const
{
  return CASTCONSTTHIS->operator | (CASTREF y);
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
  return CASTCONSTTHIS->operator - (CASTREF y);
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
  return CASTCONSTTHIS->operator - ();
}

char * OZ_FSetValue::toString() const
{
  static ozstrstream str;
  str.reset();
  const FSetValue * tmp = (const FSetValue *) this;
  tmp->print(str);

#ifdef DEBUG_FSET_CONSTRREP_DETAILED_OUTPUT
  static ozstrstream tmp_str;
  tmp_str.reset();
  tmp_str << "fset_val(set:" << str.str() << " card:" << _card << ")" 
	  << "@" << this
	  << flush;
  return tmp_str.str();
#else
  return str.str();
#endif
}

void OZ_FSetValue::copyExtension() {
  CASTTHIS->copyExtension();
}

void OZ_FSetValue::disposeExtension() {
  CASTTHIS->disposeExtension();
}

//-----------------------------------------------------------------------------

#undef CASTPTR
#undef CASTCONSTPTR
#undef CASTREF
#undef CASTTHIS
#undef CASTCONSTTHIS


#define CASTPTR (FSetConstraint *)
#define CASTCONSTPTR (const FSetConstraint *)
#define CASTREF * (const FSetConstraint *) &
#define CASTTHIS (CASTPTR this)
#define CASTCONSTTHIS (CASTCONSTPTR this)

OZ_FSetConstraint::OZ_FSetConstraint(const OZ_FSetValue &s)
{
  CASTTHIS->init(* (const FSetValue *) &s);
}

OZ_FSetConstraint::OZ_FSetConstraint(OZ_FSetState s)
{
  CASTTHIS->init(s);
}

OZ_FSetConstraint::OZ_FSetConstraint(const OZ_FSetConstraint &s)
{
  CASTTHIS->init(* (const FSetConstraint *) &s);
}

OZ_FSetConstraint &OZ_FSetConstraint::operator = (const OZ_FSetConstraint &s)
{
  return CASTTHIS->operator = (* (const FSetConstraint *) &s);
}

void OZ_FSetConstraint::init(const OZ_FSetValue & s)
{
  CASTTHIS->init(* (const FSetValue *) &s);
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
  return CASTCONSTTHIS->isValue();
}

OZ_Boolean OZ_FSetConstraint::isIn(int i) const
{
  return CASTCONSTTHIS->isIn(i);
}

OZ_Boolean OZ_FSetConstraint::isNotIn(int i) const
{
  return CASTCONSTTHIS->isNotIn(i);
}

OZ_Boolean OZ_FSetConstraint::isEmpty(void) const
{
  return CASTCONSTTHIS->isEmpty();
}

OZ_Boolean OZ_FSetConstraint::isFull(void) const
{
  return CASTCONSTTHIS->isFull();
}

OZ_Boolean OZ_FSetConstraint::isSubsumedBy(const OZ_FSetConstraint &s) const
{
  return CASTCONSTTHIS->isSubsumedBy(CASTREF s);
}

OZ_Term OZ_FSetConstraint::getKnownInList(void) const
{
  return CASTCONSTTHIS->getKnownInList();
}

OZ_Term OZ_FSetConstraint::getKnownNotInList(void) const
{
  return CASTCONSTTHIS->getKnownNotInList();
}

OZ_Term OZ_FSetConstraint::getUnknownList(void) const
{
  return CASTCONSTTHIS->getUnknownList();
}

OZ_FSetConstraint OZ_FSetConstraint::operator - (void) const
{
  return CASTCONSTTHIS->operator - ();
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

OZ_Boolean OZ_FSetConstraint::operator % (const OZ_FSetConstraint& y)
{
  return CASTTHIS->operator % (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator & (const OZ_FSetConstraint& y) const
{
  return CASTCONSTTHIS->operator & (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator | (const OZ_FSetConstraint& y) const
{
  return CASTCONSTTHIS->operator | (CASTREF y);
}

OZ_FSetConstraint OZ_FSetConstraint::operator - (const OZ_FSetConstraint& y) const
{
  return CASTCONSTTHIS->operator - (CASTREF y);
}

OZ_Term OZ_FSetConstraint::getLubList(void) const
{
  return CASTCONSTTHIS->getLubList();
}

OZ_Term OZ_FSetConstraint::getCardTuple(void) const
{
  return CASTCONSTTHIS->getCardTuple();
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
  return CASTCONSTTHIS->operator == (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::putCard(int min_card, int max_card)
{
  return CASTTHIS->putCard(min_card, max_card);
}

OZ_FSetValue OZ_FSetConstraint::getGlbSet(void) const
{
  return CASTCONSTTHIS->getGlbSet();
}

OZ_FSetValue OZ_FSetConstraint::getLubSet(void) const
{
  return CASTCONSTTHIS->getLubSet();
}

OZ_FSetValue OZ_FSetConstraint::getUnknownSet(void) const
{
  return CASTCONSTTHIS->getUnknownSet();
}

OZ_FSetValue OZ_FSetConstraint::getNotInSet(void) const
{
  return CASTCONSTTHIS->getNotInSet();
}

int OZ_FSetConstraint::getGlbCard(void) const
{
  return CASTCONSTTHIS->getGlbCard();
}

int OZ_FSetConstraint::getLubCard(void) const
{
  return CASTCONSTTHIS->getLubCard();
}

int OZ_FSetConstraint::getNotInCard(void) const
{
  return CASTCONSTTHIS->getNotInCard();
}

int OZ_FSetConstraint::getUnknownCard(void) const
{
  return CASTCONSTTHIS->getUnknownCard();
}

int OZ_FSetConstraint::getGlbMinElem(void) const
{
  return CASTCONSTTHIS->getGlbMinElem();
  //return getGlbSet().getMinElem();
}

int OZ_FSetConstraint::getLubMinElem(void) const
{
  return CASTCONSTTHIS->getLubMinElem();
  //return getLubSet().getMinElem();
}

int OZ_FSetConstraint::getNotInMinElem(void) const
{
  return CASTCONSTTHIS->getNotInMinElem();
  //return getNotInSet().getMinElem();
}

int OZ_FSetConstraint::getUnknownMinElem(void) const
{
  return CASTCONSTTHIS->getUnknownMinElem();
  //return getUnknownSet().getMinElem();
}

int OZ_FSetConstraint::getGlbMaxElem(void) const
{
  return CASTCONSTTHIS->getGlbMaxElem();
  //return getGlbSet().getMaxElem();
}

int OZ_FSetConstraint::getLubMaxElem(void) const
{
   return CASTCONSTTHIS->getLubMaxElem();
  //return getLubSet().getMaxElem();
}

int OZ_FSetConstraint::getNotInMaxElem(void) const
{
  return CASTCONSTTHIS->getNotInMaxElem();
  //return getNotInSet().getMaxElem();
}

int OZ_FSetConstraint::getUnknownMaxElem(void) const
{
  return CASTCONSTTHIS->getUnknownMaxElem();
  //return getUnknownSet().getMaxElem();
}

int OZ_FSetConstraint::getGlbNextSmallerElem(int i) const
{
  return CASTCONSTTHIS->getGlbNextSmallerElem(i);
  //return getGlbSet().getNextSmallerElem(i);
}

int OZ_FSetConstraint::getLubNextSmallerElem(int i) const
{
  return CASTCONSTTHIS->getLubNextSmallerElem(i);
  //return getLubSet().getNextSmallerElem(i);
}

int OZ_FSetConstraint::getNotInNextSmallerElem(int i) const
{
  return CASTCONSTTHIS->getNotInNextSmallerElem(i);
  //return getNotInSet().getNextSmallerElem(i);
}

int OZ_FSetConstraint::getUnknownNextSmallerElem(int i) const
{
  return CASTCONSTTHIS->getUnknownNextSmallerElem(i);
  //return getUnknownSet().getNextSmallerElem(i);
}

int OZ_FSetConstraint::getGlbNextLargerElem(int i) const
{
  return CASTCONSTTHIS->getGlbNextLargerElem(i);
  //return getGlbSet().getNextLargerElem(i);
}

int OZ_FSetConstraint::getLubNextLargerElem(int i) const
{
  return CASTCONSTTHIS->getLubNextLargerElem(i);
  //return getLubSet().getNextLargerElem(i);
}

int OZ_FSetConstraint::getNotInNextLargerElem(int i) const
{
  return CASTCONSTTHIS->getNotInNextLargerElem(i);
  //return getNotInSet().getNextLargerElem(i);
}

int OZ_FSetConstraint::getUnknownNextLargerElem(int i) const
{
  return CASTCONSTTHIS->getUnknownNextLargerElem(i);
  //return getUnknownSet().getNextLargerElem(i);
}

char * OZ_FSetConstraint::toString() const
{
  static ozstrstream str;
  str.reset();
  const FSetConstraint * tmp = (const FSetConstraint *) this;
  tmp->print(str);

#ifdef DEBUG_FSET_CONSTRREP_DETAILED_OUTPUT
  static ozstrstream tmp_str;
  tmp_str.reset();
  tmp_str << "fset(" << str.str()
	  << " card:" << _card_min << "#" << _card_max << ")"
	  << "@" << this
	  << flush;
  return tmp_str.str();
#else
  return str.str();
#endif
}

OZ_Boolean OZ_FSetConstraint::le (const int i)
{
  return CASTTHIS->le(i);
}

OZ_Boolean OZ_FSetConstraint::ge (const int i)
{
  return CASTTHIS->ge(i);
}

void OZ_FSetConstraint::copyExtension() {
  CASTTHIS->copyExtension();
}

void OZ_FSetConstraint::disposeExtension() {
  CASTTHIS->disposeExtension();
}

void
makeFSetValue(OZ_Term desc,OZ_Term*fs)
{
  *fs = makeTaggedFSetValue(new FSetValue(desc));
}


#undef CASTREF
#define CASTREF * (const FSetValue *) &

OZ_Boolean OZ_FSetConstraint::operator <= (const int i)
{
  return CASTTHIS->operator <= (i);
}

OZ_Boolean OZ_FSetConstraint::operator >= (const int i)
{
  return CASTTHIS->operator >= (i);
}

OZ_Boolean OZ_FSetConstraint::operator |= (const OZ_FSetValue &y)
{
  return CASTTHIS->operator |= (CASTREF y);
}

OZ_Boolean OZ_FSetConstraint::operator &= (const OZ_FSetValue &y)
{
   return CASTTHIS->operator &= (CASTREF y);
}


// eof
//-----------------------------------------------------------------------------
