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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "fdomn.hh"
#endif

#include <stdarg.h>

#include "base.hh"
#include "ozostream.hh"
#include "fdomn.hh"
#include "sort.hh"

//-----------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#endif

//-----------------------------------------------------------------------------

#include "bits.hh"

#ifdef DEBUG_FD_CONSTRREP

void print_to_fdfile (const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(_fdomn_file, format, ap);
  fflush(_fdomn_file);
  va_end(ap);
}

#endif

#ifdef TO_FD_FILE
FILE * _fdomn_file = fopen("/tmp/fdomn_ir_debug__file.oz", "w+");
#else
FILE * _fdomn_file = stdout;
#endif

//-----------------------------------------------------------------------------
// Miscellaneous --------------------------------------------------------------

int toTheLowerEnd[32] = {
  0x00000001,0x00000003,0x00000007,0x0000000f,
  0x0000001f,0x0000003f,0x0000007f,0x000000ff,
  0x000001ff,0x000003ff,0x000007ff,0x00000fff,
  0x00001fff,0x00003fff,0x00007fff,0x0000ffff,
  0x0001ffff,0x0003ffff,0x0007ffff,0x000fffff,
  0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
  0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,
  0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
};

int toTheUpperEnd[32] = {
  0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,
  0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,
  0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,
  0xfffff000,0xffffe000,0xffffc000,0xffff8000,
  0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,
  0xfff00000,0xffe00000,0xffc00000,0xff800000,
  0xff000000,0xfe000000,0xfc000000,0xf8000000,
  0xf0000000,0xe0000000,0xc0000000,0x80000000
};

int fd_bv_max_high, fd_bv_max_elem, fd_bv_conv_max_high;
int * fd_bv_left_conv, * fd_bv_right_conv;
EnlargeableArray<intptr> fd_iv_ptr_sort(FDOMNINITSIZE);
EnlargeableArray<int> fd_iv_left_sort(FDOMNINITSIZE);
EnlargeableArray<int> fd_iv_right_sort(FDOMNINITSIZE);

inline
int _word32(int n) { return mod32(n) ? div32(n) + 1 : div32(n); }

void reInitFDs(int threshold)
{
  threshold = _word32(threshold);

  if (threshold >= 0 && threshold != fd_bv_max_high) {
    if (fd_bv_conv_max_high > 0) {
      delete [] fd_bv_left_conv;
      delete [] fd_bv_right_conv;
    }

    fd_bv_max_high = threshold;
    fd_bv_max_elem = 32 * fd_bv_max_high - 1;
    fd_bv_conv_max_high = fd_bv_max_elem / 2 + 2;
    
    if (fd_bv_conv_max_high > 0) {
      fd_bv_left_conv = ::new int[fd_bv_conv_max_high];
      fd_bv_right_conv = ::new int[fd_bv_conv_max_high];
    }
  }
}

void initFDs()
{
  fd_bv_max_high = 32;
  fd_bv_max_elem = 32 * fd_bv_max_high - 1;
  fd_bv_conv_max_high = fd_bv_max_elem / 2 + 2;
  
  fd_bv_left_conv = ::new int[fd_bv_conv_max_high];
  fd_bv_right_conv = ::new int[fd_bv_conv_max_high];

}

//-----------------------------------------------------------------------------
// FDInterval -----------------------------------------------------------------

#ifdef DEBUG_FD_CONSTRREP

OZ_Boolean FDIntervals::isConsistent(void) const {
  if (high < 0) {
    printf("high < 0"); fflush(stdout);
    return OZ_FALSE;
  }

  int i;
  for (i = 0; i < high; i++) {
    if (i_arr[i].left > i_arr[i].right) {
      printf("i_arr[%d].left > i_arr[%d].right", i, i); fflush(stdout);
      return OZ_FALSE;
    }
    if ((i + 1 < high) && (i_arr[i].right >= i_arr[i + 1].left))
      return OZ_FALSE;
  }
  for (i = 0; i < high - 1; i++) {
    if (! ((i_arr[i].right + 1) < i_arr[i + 1].left)) {
      printf("!((i_arr[%d].right + 1) < i_arr[i%d+1].left)", i, i); fflush(stdout);
      return OZ_FALSE;
    }
  }
  return OZ_TRUE;
}

#endif

inline
FDIntervals * newIntervals(int max_index) {
  return new (max_index) FDIntervals(max_index);
} 

inline
FDIntervals * FDIntervals::copy(void)
{
  FDIntervals * new_item = newIntervals(high);

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  memcpy(&new_item->i_arr, &i_arr, high * sizeof(_i_arr_type));
#else
  memcpy(new_item->i_arr, i_arr, high * sizeof(i_arr[0]));
#endif

  return new_item;
}

// Do not inline!
int FDIntervals::findSize(void) {
  int s, i;
  for (s = 0, i = high; i--; )
    s += (i_arr[i].right - i_arr[i].left);

  return s + high;
}

inline
int FDIntervals::findMinElem(void) {
  return high ? i_arr[0].left : 0;
}

inline
int FDIntervals::findMaxElem(void) {
  return high ? i_arr[high - 1].right : 0;
}

inline
void FDIntervals::init(int l0, int r0, int l1, int r1)
{
  i_arr[0].left = l0;
  i_arr[0].right = r0;
  i_arr[1].left = l1;
  i_arr[1].right = r1;

  AssertFD(isConsistent());
}

inline
const FDIntervals &FDIntervals::operator = (const FDIntervals &iv)
{
  AssertFD(high >= iv.high);

  high = iv.high;

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  memcpy(&i_arr, &iv.i_arr, high * sizeof(_i_arr_type));
#else
  memcpy(i_arr, iv.i_arr, high * sizeof(i_arr[0]));
#endif
  return *this;
}

inline
FDIntervals::FDIntervals(const FDIntervals &iv) {
  *this = iv;
}

inline
void FDIntervals::initList(int list_len,
			   int * list_left, int * list_right)
{
  AssertFD(list_len <= high);

  for (int i = list_len; i--; ) {
    i_arr[i].left = list_left[i];
    i_arr[i].right = list_right[i];
  }
  
  AssertFD(isConsistent());
}

inline
int FDIntervals::nextSmallerElem(int v, int min_elem) const
{
  if (v <= min_elem) return -1;

  for (int i = high; i--; ) {
    if (i_arr[i].left < v && v-1 <= i_arr[i].right)
      return v - 1;
    if (v > i_arr[i].right)
      return i_arr[i].right;
  }

  return -1;
} 

inline
int FDIntervals::nextLargerElem(int v, int max_elem) const
{
  if (v >= max_elem) return -1;

  for (int i = 0; i < high; i += 1) {
    if (v < i_arr[i].left)
      return i_arr[i].left;
    if (i_arr[i].left - 2 < v && v < i_arr[i].right)
      return v + 1;
  }
  return -1;
} 

// taken from stubbs&webre page 168
inline 
int FDIntervals::findPossibleIndexOf(int i) const
{
  int lo, hi;
  for (lo = 0, hi = high - 1; lo < hi; ) {
    int mid = (lo + hi + 1) / 2;
    if (i < i_arr[mid].left)
      hi = mid - 1;
    else 
      lo = mid;
  }
  return lo;
}

inline
OZ_Boolean FDIntervals::isIn(int i) const
{
  int index = findPossibleIndexOf(i);
  return (i_arr[index].left <= i && i <= i_arr[index].right);
}

// v is in domain
inline 
int FDIntervals::lowerBound(int v) const 
{
  return i_arr[findPossibleIndexOf(v)].left;
}

// v is in domain
inline 
int FDIntervals::upperBound(int v) const 
{
  return i_arr[findPossibleIndexOf(v)].right;
}

// i is not in the domain
inline
int FDIntervals::midElem(int i) const
{
  int j = 0;

  while (j < high - 1 && !(i_arr[j].right < i && i < i_arr[j + 1].left))
    j += 1;

  int l = i_arr[j].right, r = i_arr[j + 1].left;

  // prefer left neighbour against right one
  return ((r - i) >= (i - l)) ? l : r;
}

static
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
OZ_Term FDIntervals::getAsList(void) const
{
  LTuple * hd = NULL, * l_ptr = NULL;

  for (int i = 0; i < high; i += 1) 
      l_ptr = (i_arr[i].left == i_arr[i].right)
	? mkListEl(hd, l_ptr, oz_int(i_arr[i].left))
	: mkListEl(hd, l_ptr, oz_pairII(i_arr[i].left, i_arr[i].right));
  
  return makeTaggedLTuple(hd);
}

// fd_inf <= leq <= fd_sup
inline
int FDIntervals::operator <= (const int leq)
{
  int index = findPossibleIndexOf(leq);

  if (i_arr[index].left <= leq && leq <= i_arr[index].right) {
    i_arr[index].right = leq;
    index += 1;
  } else
    if (i_arr[index].right < leq) index += 1;

  AssertFD(high >= index);
  
  high = index;

  AssertFD(isConsistent());
  
  return findSize();
}

// fd_inf <= geq <= fd_sup
inline
int FDIntervals::operator >= (const int geq)
{
  int index = findPossibleIndexOf(geq);

  if (i_arr[index].left <= geq && geq <= i_arr[index].right) 
    i_arr[index].left = geq;
  else
    if (i_arr[index].right < geq) index += 1;

  if (index != 0) {
    for (int t = 0, f = index; f < high; ) i_arr[t++] = i_arr[f++];
    high -= index;
  }
  
  AssertFD(isConsistent());
  
  return findSize();
}

inline
FDIntervals * FDIntervals::operator -= (const int take_out)
{
  int index = findPossibleIndexOf(take_out);

  if (take_out < i_arr[index].left) // take_out is not in this domain
    return this;

  // take_out is in i_arr[index]
  if (i_arr[index].left == i_arr[index].right) {
    for (int i = index; i < (high - 1); i++)
      i_arr[i] = i_arr[i + 1];
    high -= 1;
  } else if (i_arr[index].left == take_out) {
    i_arr[index].left += 1;
  } else if (i_arr[index].right == take_out) {
    i_arr[index].right -= 1;
  } else {
    int i, new_max_high = high + 1;
    FDIntervals * new_iv = newIntervals(new_max_high);

    for (i = 0; i <= index; i += 1)
      new_iv->i_arr[i] = i_arr[i];
    new_iv->i_arr[index].right = take_out - 1;
    for (i = index; i < high; i += 1)
      new_iv->i_arr[i + 1] = i_arr[i];
    new_iv->i_arr[index + 1].left = take_out + 1;
    
    AssertFD(new_iv->isConsistent());
    
    //    dispose(); TMUELLER
    return new_iv;
  }
  
  AssertFD(isConsistent());

  return this;
}

inline  
FDIntervals * FDIntervals::operator += (const int put_in)
{
  int index = findPossibleIndexOf(put_in);

  if (i_arr[index].left <= put_in && put_in <= i_arr[index].right)
    return this;

  if (put_in == i_arr[index].right + 1) {
    // closing a gap
    if ((index + 1) < high && put_in == i_arr[index + 1].left - 1) {
      i_arr[index].right = i_arr[index + 1].right;
      for (int i = index + 1; (i + 1) < high; i += 1)
	i_arr[i] = i_arr[i + 1];
      high -= 1;
    } else {
      i_arr[index].right += 1;
    } 
  } else if (put_in == i_arr[index].left - 1) {
    i_arr[index].left = put_in;
  } else if ((index + 1) < high && put_in == i_arr[index + 1].left - 1) {
    i_arr[index + 1].left -= 1;
  } else {
    high += 1;
    if (i_arr[index].right < put_in) index += 1;
    
    FDIntervals * new_iv = newIntervals(high);
    int i;

    for (i = 0; i < index; i += 1)
      new_iv->i_arr[i] = i_arr[i];
    for (i = high - 1; index < i; i -= 1)
      new_iv->i_arr[i] = i_arr[i - 1];
    new_iv->i_arr[index].left = new_iv->i_arr[index].right = put_in;

    // dispose(); TMUELLER
    AssertFD(new_iv->isConsistent());
    return new_iv;
  }

  AssertFD(isConsistent());

  return this;
}

inline
FDIntervals * FDIntervals::complement(FDIntervals * x_iv)
{
  int c_i = 0, i = 0;
  if (0 < x_iv->i_arr[i].left) {
    i_arr[c_i].left = 0;
    i_arr[c_i].right = x_iv->i_arr[i].left - 1;
    c_i += 1;
  }
  for ( ; i < x_iv->high - 1; i += 1, c_i += 1) {
    i_arr[c_i].left = x_iv->i_arr[i].right + 1;
    i_arr[c_i].right = x_iv->i_arr[i + 1].left - 1;
  }
  if (x_iv->i_arr[i].right < fd_sup) {
    i_arr[c_i].left = x_iv->i_arr[i].right + 1;
    i_arr[c_i].right = fd_sup;
  }

  AssertFD(isConsistent());
  
  return this;
}

inline
FDIntervals * FDIntervals::complement(int a_high, int * x_left, int * x_right)
{
  int c_i = 0, i = 0;
  if (0 < x_left[i]) {
    i_arr[c_i].left = 0;
    i_arr[c_i].right = x_left[i] - 1;
    c_i += 1;
  }
  for ( ; i < a_high - 1; i += 1, c_i += 1) {
    i_arr[c_i].left = x_right[i] + 1;
    i_arr[c_i].right = x_left[i + 1] - 1;
  }
  if (x_right[i] < fd_sup) {
    i_arr[c_i].left = x_right[i] + 1;
    i_arr[c_i].right = fd_sup;
  }
  
  AssertFD(isConsistent());
  
  return this;
}

inline
int FDIntervals::union_iv(const FDIntervals &x, const FDIntervals &y)
{
  int x_c, y_c, z_c, r;
  for (x_c = 0, y_c = 0, z_c = 0, r=-1; x_c < x.high && y_c < y.high; ) {

    if (x.i_arr[x_c].left < y.i_arr[y_c].left) {
      i_arr[z_c].left = x.i_arr[x_c].left;
      r = x.i_arr[x_c].right;
      x_c += 1;
      for (; y_c < y.high && y.i_arr[y_c].right <= r; y_c += 1);
    } else {
      i_arr[z_c].left = y.i_arr[y_c].left;
      r = y.i_arr[y_c].right;
      y_c += 1;
      for (; x_c < x.high && x.i_arr[x_c].right <= r; x_c += 1);
    }
    
    for (OZ_Boolean cont = OZ_TRUE; cont; )
      if (x_c < x.high &&
	  x.i_arr[x_c].left <= r + 1 && r <= x.i_arr[x_c].right) {
	r = x.i_arr[x_c].right;
	x_c += 1;
	for (; y_c < y.high && y.i_arr[y_c].right <= r; y_c += 1);
      } else if (y_c < y.high &&
		 y.i_arr[y_c].left <= r + 1 && r <= y.i_arr[y_c].right) {
	r = y.i_arr[y_c].right;
	y_c += 1;
	for (; x_c < x.high && x.i_arr[x_c].right <= r; x_c += 1);
      } else {
	cont = OZ_FALSE;
      }

    for (; x_c < x.high && x.i_arr[x_c].right <= r; x_c += 1);
    for (; y_c < y.high && y.i_arr[y_c].right <= r; y_c += 1);
    
    i_arr[z_c].right = r;
    z_c += 1;
  }

  // copy remaining intervals
  if ((x_c + 1) < x.high && x.i_arr[x_c].left < r) x_c += 1;
  for (; x_c < x.high; x_c += 1, z_c += 1) i_arr[z_c] = x.i_arr[x_c];
  if ((y_c + 1) < y.high && y.i_arr[y_c].left < r) y_c += 1;
  for (; y_c < y.high; y_c += 1, z_c += 1) i_arr[z_c] = y.i_arr[y_c];

  AssertFD(high >= z_c);
  
  high = z_c;

  AssertFD(isConsistent());

  return findSize();
}

inline
int FDIntervals::intersect_iv(FDIntervals &z, const FDIntervals &y)
{
  int x_c, y_c, z_c;
  for (x_c = 0, y_c = 0, z_c = 0; x_c < high && y_c < y.high; )
    if (i_arr[x_c].left > y.i_arr[y_c].left) {
      if (y.i_arr[y_c].right < i_arr[x_c].left) { // no overlapping
	y_c += 1;
      } else if (y.i_arr[y_c].right <= i_arr[x_c].right) { // overlapping
	z.i_arr[z_c].left = i_arr[x_c].left;
	z.i_arr[z_c++].right = y.i_arr[y_c++].right;
      } else { // subsumption
	z.i_arr[z_c++] = i_arr[x_c++];
      }
    } else {
      if (i_arr[x_c].right < y.i_arr[y_c].left) { // no overlapping
	x_c += 1;
      } else if (i_arr[x_c].right <= y.i_arr[y_c].right) { // overlapping
	z.i_arr[z_c].left = y.i_arr[y_c].left;
	z.i_arr[z_c++].right = i_arr[x_c++].right;
      } else { // subsumption
	z.i_arr[z_c++] = y.i_arr[y_c++];
      }
    }
  AssertFD(z.high >= z_c);
  z.high = z_c;

  AssertFD(z.isConsistent());
  return z.findSize();
}

inline
int FDIntervals::subtract_iv(FDIntervals &z, const FDIntervals &y)
{
  int x_c, y_c, z_c;
  for (x_c = 0, y_c = 0, z_c = 0; x_c < high && y_c < y.high; ) {
    for (; y_c < y.high && y.i_arr[y_c].right < i_arr[x_c].left; y_c += 1);
    if (y_c >= y.high) break;
    
    if (y.i_arr[y_c].left <= i_arr[x_c].left &&
	i_arr[x_c].right <= y.i_arr[y_c].right) { // overlap
      x_c += 1;
    } else if (i_arr[x_c].right < y.i_arr[y_c].left) { // step over
      z.i_arr[z_c++] = i_arr[x_c++];
    } else if (i_arr[x_c].right <= y.i_arr[y_c].right) { // right cut
      z.i_arr[z_c].left = i_arr[x_c++].left;
      z.i_arr[z_c++].right = y.i_arr[y_c].left - 1;
    } else if (y.i_arr[y_c].right <= i_arr[x_c].right) {
      if (i_arr[x_c].left < y.i_arr[y_c].left) {
	z.i_arr[z_c].left = i_arr[x_c].left;
      } else {
	z.i_arr[z_c].left = y.i_arr[y_c++].right + 1;
      }
      while (y_c < y.high && y.i_arr[y_c].right < i_arr[x_c].right) {
	z.i_arr[z_c].right = y.i_arr[y_c].left - 1;
	z.i_arr[++z_c].left = y.i_arr[y_c++].right + 1;
      }
      if (y_c < y.high && y.i_arr[y_c].left <= i_arr[x_c].right) {
	z.i_arr[z_c++].right = y.i_arr[y_c].left - 1;
	x_c += 1;
      } else {
	z.i_arr[z_c++].right = i_arr[x_c++].right;
      }
    }
  }
  
  for (; x_c < high; x_c += 1, z_c += 1) z.i_arr[z_c] = i_arr[x_c];

  AssertFD(z.high >= z_c);
  z.high = z_c;

  AssertFD(z.isConsistent());
  return z.findSize();
}


//-----------------------------------------------------------------------------
// calls FDBitVector ----------------------------------------------------------

inline
FDBitVector * newBitVector(int hi) {
  Assert(hi <= word32(fd_bv_max_elem));

  return new (hi) FDBitVector(hi);
}

inline
OZ_Boolean FDBitVector::isIn(int i) const {
  return i <= currBvMaxElem() ? (b_arr[div32(i)] & (1 << (mod32(i)))) : FALSE;
}

inline
void FDBitVector::setBit(int i) {
  b_arr[div32(i)] |= (1 << (mod32(i)));
}

inline
void FDBitVector::resetBit(int i) {
  b_arr[div32(i)] &= ~(1 << (mod32(i)));
}

inline
void FDBitVector::setEmpty(void) {
  for (int i = high; i--; )
    b_arr[i] = 0;
}


inline
const FDBitVector &FDBitVector::operator = (const FDBitVector &bv)
{
  AssertFD(high >= bv.high);

  high = bv.high;
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  memcpy(&b_arr, &bv.b_arr, high * sizeof(b_arr_t));
#else
  memcpy(b_arr, bv.b_arr, high * sizeof(b_arr[0]));
#endif

  return *this;
}

inline
FDBitVector * FDBitVector::copy(void)
{
  FDBitVector * new_item = newBitVector(high);

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  memcpy(&new_item->b_arr, &b_arr, high * sizeof(b_arr_t));
#else
  memcpy(new_item->b_arr, b_arr, high * sizeof(b_arr[0]));
#endif

  return new_item;
}

inline
void FDBitVector::addFromTo(int from, int to)
{
  AssertFD(0 <= from && from <= to && to <= currBvMaxElem());
  
  int low_word = div32(from), low_bit = mod32(from);
  int up_word = div32(to), up_bit = mod32(to);

  if (low_word == up_word) {
    b_arr[low_word] |= toTheLowerEnd[up_bit] & toTheUpperEnd[low_bit];
  } else {
    b_arr[low_word] |= toTheUpperEnd[low_bit];
    for (int i = low_word + 1; i < up_word; i++)
      b_arr[i] = int(~0);
    b_arr[up_word] |= toTheLowerEnd[up_bit];
  }
}

inline
void FDBitVector::initList(int list_len,
			   int * list_left, int * list_right)
{
  setEmpty();
  for (int i = list_len; i--; )
    addFromTo(list_left[i], list_right[i]);
}

inline
int FDBitVector::findSize(void) {
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  return get_num_of_bits(high, (int *) &b_arr);
#else
  return get_num_of_bits(high, b_arr);
#endif
}

inline
void FDBitVector::findHigh(int max_elem)
{
  high = word32(max_elem);
}

inline
int FDBitVector::findMinElem(void)
{
  int v, i;
  for (v = 0, i = 0; i < high; v += 32, i += 1) 
    if (b_arr[i] != 0)
      break;

  if (i < high) {
    int word = b_arr[i];

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
  } else { v = -1; }
  return v;
}

inline
int FDBitVector::findMaxElem(void)
{
  int v, i;
  for (v = currBvMaxElem(), i = high - 1; i >= 0; v -= 32, i--) 
    if (b_arr[i] != 0)
      break;

  if (i >= 0) {
    int word = b_arr[i];
    
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
  }
  findHigh(v);
  return v;
}

inline
void FDBitVector::setFromTo(int from, int to)
{
  AssertFD(0 <= from && from <= to && to <= currBvMaxElem());
  
  int low_word = div32(from), low_bit = mod32(from);
  int up_word = div32(to), up_bit = mod32(to);

  int i;
  for (i = 0; i < low_word; i++)
    b_arr[i] = 0;
  for (i = up_word + 1; i < high; i++)
    b_arr[i] = 0;    

  if (low_word == up_word) {
    b_arr[low_word] = toTheLowerEnd[up_bit] & toTheUpperEnd[low_bit];
  } else {
    b_arr[low_word] = toTheUpperEnd[low_bit];
    for (i = low_word + 1; i < up_word; i++)
      b_arr[i] = int(~0);
    b_arr[up_word] = toTheLowerEnd[up_bit];
  }
}

inline
int FDBitVector::nextSmallerElem(int v, int min_elem) const
{
  for (int new_v = v - 1; new_v >= min_elem; new_v -= 1)
    if (isIn(new_v))
      return new_v;
  
  return -1;
} 

inline
int FDBitVector::nextLargerElem(int v, int max_elem) const
{
  for (int new_v = v + 1; new_v <= max_elem; new_v += 1)
    if (isIn(new_v))
      return new_v;
  
  return -1;
} 

// v is in domain
inline
int FDBitVector::lowerBound(int v, int min_elem) const
{
  if (v == min_elem) 
    return v;
  else 
    for (int new_v = v - 1; new_v >= min_elem; new_v -= 1)
      if (! isIn(new_v)) 
	return new_v + 1;

  return min_elem;
} 

// v is in domain
inline
int FDBitVector::upperBound(int v, int max_elem) const
{
  if (v == max_elem)
    return v;
  else 
    for (int new_v = v + 1; new_v <= max_elem; new_v += 1)
      if (! isIn(new_v))
	return new_v - 1;

  return max_elem;
} 

inline
int FDBitVector::midElem(int i) const
{
  // find lower neighbour
  int lb = mod32(i), lw = div32(i), ub = lb, uw = lw;

  if (!(b_arr[lw] << (31 - lb))) {
    lb = 31;
    for (lw--; !b_arr[lw] && lw >= 0; lw--);
  }
  for (; lb >= 0 && !(b_arr[lw] & (1 << lb)); lb--);
  int l = 32 * lw + lb;

  // find upper neighbour
  if (!(b_arr[uw] >> ub)) {
    ub = 0;
    for (uw++ ; !b_arr[uw] && uw < high; uw++);
  }
  for (; ub < 32 && !(b_arr[uw] & (1 << ub)); ub++);
  int u = 32 * uw + ub;

  // prefer left neighbour against right one
  return ((u - i) >= (i - l)) ? l : u;
}

inline
int FDBitVector::mkRaw(int * list_left, int * list_right) const
{
  int i, r, l, len, bvms = currBvMaxElem();
  for (i = 0, r = 1, len = 0, l = -1; i < bvms + 2; i += 1)
    if (isIn(i)) {
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

int FDBitVector::mkRawOutline(int * list_left, int * list_right) const
{
  return mkRaw(list_left, list_right);
}

inline
OZ_Term FDBitVector::getAsList(void) const
{
  LTuple * hd = NULL, * l_ptr = NULL;
  int len = mkRaw(fd_bv_left_conv, fd_bv_right_conv);
  
  for (int i = 0; i < len; i += 1) 
    if (fd_bv_left_conv[i] == fd_bv_right_conv[i])
      l_ptr = mkListEl(hd, l_ptr, oz_int(fd_bv_left_conv[i]));
    else
      l_ptr = mkListEl(hd, l_ptr, oz_pairII(fd_bv_left_conv[i],
					    fd_bv_right_conv[i]));
  
  return makeTaggedLTuple(hd);
}

// fd_inf <= leq <= fd_bv_max_elem
inline
int FDBitVector::operator <= (const int leq)
{
  int upper_word = div32(leq), upper_bit = mod32(leq);
  
  for (int i = upper_word + 1; i < high; i += 1)
    b_arr[i] = 0;
  b_arr[upper_word] &= toTheLowerEnd[upper_bit];
  
  return findSize();
}

// fd_inf <= geq <= fd_bv_max_elem
inline
int FDBitVector::operator >= (const int geq)
{
  int lower_word = div32(geq), lower_bit = mod32(geq);

  for (int i = 0; i < lower_word; i += 1)
    b_arr[i] = 0;
  b_arr[lower_word] &= toTheUpperEnd[lower_bit];

  return findSize();
}

inline 
int FDBitVector::union_bv(const FDBitVector &x, const FDBitVector &y)
{
  int i, min_high = min(x.high, y.high);

  for (i = min_high; i--; )
    b_arr[i] = x.b_arr[i] | y.b_arr[i];
  i = min_high;

  for (; i < x.high; i += 1)
    b_arr[i] = x.b_arr[i];

  for (; i < y.high; i += 1)
    b_arr[i] = y.b_arr[i];
  
  return findSize();
}

inline
int FDBitVector::intersect_bv(const FDBitVector &x, const FDBitVector &y)
{
  high = min(x.high, y.high);
  for (int i = high; i--; )
    b_arr[i] = x.b_arr[i] & y.b_arr[i];
  
  return findSize();
}
     
inline
int FDBitVector::intersect_bv(const FDBitVector &y)
{
  high = min(high, y.high);
  for (int i = high; i--; )
    b_arr[i] &= y.b_arr[i];

  return findSize();
}
     
inline
int FDBitVector::operator -= (const FDBitVector &y)
{
  for (int i = min(high, y.high); i--; )
    b_arr[i] &= ~y.b_arr[i];
  return findSize();
}

//-----------------------------------------------------------------------------
// class OZ_FiniteDomainImpl --------------------------------------------------

// private methods ------------------------------------------------------------


inline
void OZ_FiniteDomainImpl::setType(descr_type t) {
  descr = orPointer(andPointer(NULL,~3),t);
}

inline
void OZ_FiniteDomainImpl::setType(FDBitVector * p) {
  descr = orPointer(p, bv_descr);
}

inline
void OZ_FiniteDomainImpl::setType(FDIntervals * p) {
  descr = orPointer(p, iv_descr);
}

inline
void OZ_FiniteDomainImpl::set_iv(void * p) {
  descr = orPointer(p, iv_descr);
}

inline
void OZ_FiniteDomainImpl::set_bv(void * p) {
  descr = p;
}

inline
OZ_Boolean OZ_FiniteDomainImpl::isSingleInterval(void) const {
  return size == (max_elem - min_elem + 1);
}

inline
FDBitVector * OZ_FiniteDomainImpl::provideBitVector(int hi) const
{
  FDBitVector * bv = (getType() == bv_descr ? get_bv() : (FDBitVector *) NULL);
  if (!bv) {
    return newBitVector(hi);
  } else if (hi > bv->high) {
    // replace old one by new one
    bv->dispose();
    return newBitVector(hi);
  } else {
    bv->high = hi;
  }
  return bv;
}

inline
FDBitVector * OZ_FiniteDomainImpl::asBitVector(void) const
{
  descr_type type = getType();
  if (type == bv_descr) {
    return get_bv();
  } else if (type == fd_descr) {
    int min_maxElem = min(fd_bv_max_elem, max_elem);
    FDBitVector * bv = provideBitVector(word32(min_maxElem));
    if (min_elem > fd_bv_max_elem)
      bv->setEmpty();
    else
      bv->setFromTo(min_elem, min_maxElem);
    return bv;
  } else {
    int min_maxElem = min(fd_bv_max_elem, max_elem);
    FDBitVector * bv = newBitVector(word32(min_maxElem));
    FDIntervals &iv = *get_iv();
    bv->setEmpty();
    for (int i = 0; i < iv.high && iv.i_arr[i].left <= fd_bv_max_elem; i++) 
      bv->addFromTo(iv.i_arr[i].left, min(iv.i_arr[i].right, fd_bv_max_elem));

    Assert(fd_bv_max_elem < iv.findMaxElem() 
	   || bv->findSize() == iv.findSize() );
    return bv;
  }
}

inline
FDIntervals * OZ_FiniteDomainImpl::provideIntervals(int max_index) const
{
  FDIntervals * iv = (getType() == iv_descr ? get_iv() : (FDIntervals *) NULL);
  if (!iv) {
    return newIntervals(max_index);
  } else if (max_index > iv->high) {
    // replace old one by new one
    iv->dispose();
    return newIntervals (max_index);
  } else {
    iv->high = max_index;
  }
  return iv;
}

inline
FDIntervals * OZ_FiniteDomainImpl::asIntervals(void) const
{
  descr_type type = getType();
  if (type == iv_descr) {
    return get_iv();
  } else if (type == bv_descr) {
    if (isSingleInterval()) {
      FDIntervals * iv = provideIntervals(1);
      iv->init(min_elem, max_elem);
      return iv;
    } else {
      int s = get_bv()->mkRaw(fd_bv_left_conv, fd_bv_right_conv);
      FDIntervals * iv = newIntervals(s);
      iv->initList(s, fd_bv_left_conv, fd_bv_right_conv);
      return iv;
    }
  } else {
    FDIntervals * iv = provideIntervals(1);
    iv->init(min_elem, max_elem);
    return iv;
  }
}

#ifdef DEBUG_FD_CONSTRREP

OZ_Boolean OZ_FiniteDomainImpl::isConsistent(void) const {
  if (size == 0) return OZ_TRUE;
  descr_type type = getType();
  if (type == fd_descr)
    return findSize() == size;
  else if (type == bv_descr)
    return get_bv()->findSize() == size && 
	 get_bv()->findMinElem() == min_elem &&
	 get_bv()->findMaxElem() == max_elem;
  else    
    return get_iv()->findSize() == size &&
    max_elem > fd_bv_max_elem &&
    get_iv()->findMinElem() == min_elem &&
    get_iv()->findMaxElem() == max_elem;
}

#endif
 
inline
OZ_Boolean OZ_FiniteDomainImpl::isIn(int i) const
{
  if (size == 0 || i < min_elem || max_elem  < i) {
    return OZ_FALSE;
  } else {
    descr_type type = getType();
    if (type == fd_descr) {
      return TRUE;
    } else if (type == bv_descr) {
      return get_bv()->isIn(i);
    } else {
      return get_iv()->isIn(i);
    }
  }
}

// public methods -------------------------------------------------------------

inline
void OZ_FiniteDomainImpl::FiniteDomainInit(void * d) { 
  setType(fd_descr, d); 
}

inline
OZ_FiniteDomainImpl::OZ_FiniteDomainImpl(void) { 
  FiniteDomainInit((void *) 0); 
}

inline
OZ_Boolean OZ_FiniteDomainImpl::operator != (const int v) const
{
  return (size != 1) || (min_elem != v);
}

inline
OZ_Boolean OZ_FiniteDomainImpl::operator != (const OZ_FDState state) const
{
  if (state == fd_singl) {
    return size != 1;
  } else if (state == fd_bool) {
    return size != 2 || min_elem != 0 || max_elem != 1;
  } else {
    Assert(state == fd_empty); 
    return size > 0;
  }
}

inline
const OZ_FiniteDomainImpl &OZ_FiniteDomainImpl::operator = (const OZ_FiniteDomainImpl &fd)
{
  if (this != &fd) {
    min_elem = fd.min_elem;
    max_elem = fd.max_elem;
    size = fd.size;
    
    descr_type type = fd.getType();
    if (type == fd_descr) {
      setType(fd_descr);
    } else if (type == bv_descr) {
      // optimization: check if there is already memory allocated and
      // reuse if possible
      FDBitVector * item = newBitVector(fd.get_bv()->getHigh());
      *item = *fd.get_bv();
      setType(item);
    } else {
      int max_index = fd.get_iv()->high;
      FDIntervals * item = newIntervals(max_index);
      *item = *fd.get_iv();
      setType(item);
    }
  }
  return *this;
}

inline
OZ_FiniteDomainImpl::OZ_FiniteDomainImpl(const OZ_FiniteDomainImpl &fd):
     OZ_FiniteDomain() {
  *this = fd;
}

inline
void OZ_FiniteDomainImpl::disposeExtension(void) {
  switch (getType()) {
  case iv_descr: get_iv()->dispose(); return;
  case bv_descr: get_bv()->dispose(); return;
  default: return;
  }
}

unsigned OZ_FiniteDomainImpl::getDescrSize() {
  switch (getType()) {
  case iv_descr: return get_iv()->sizeOf();
  case bv_descr: return get_bv()->sizeOf();
  default:       return 0;
  }
}

/* gcc-2.6.3 on solaris has problems ...*/
int intcompare(const void * ii, const void  * jj) {
  int * const *i = (int * const *) ii;
  int * const *j = (int * const *) jj;
  return(**i - **j);
}

typedef int* int_ptr;

class Order_IntPtr_Inc {
public:
  Bool operator()(const int_ptr& ip, const int_ptr& jp) {
    return *ip < *jp;
  }
};

inline
int OZ_FiniteDomainImpl::simplify(int list_len, 
				  int * list_left, 
				  int * list_right)
{

  fd_iv_ptr_sort.request(list_len);
  fd_iv_left_sort.request(list_len);
  fd_iv_right_sort.request(list_len);
  // producing list of sorted pointers in terms of this list 
  int i;

  for (i = list_len; i--; ) {
    fd_iv_ptr_sort[i] = list_left + i;
  }

  Order_IntPtr_Inc lt;
  fastsort((intptr*)fd_iv_ptr_sort, list_len, lt);

  //  qsort(fd_iv_ptr_sort, list_len, sizeof(int **), intcompare);

  // copy sorted arrays (copy needed, otherwise overwriting might occur!)
  for (i = list_len; i--; ) {
    fd_iv_left_sort[i]  = *fd_iv_ptr_sort[i];
    fd_iv_right_sort[i] = *(list_right + (fd_iv_ptr_sort[i]-list_left));
  }


  // Invariant: 
  //   (list_left[0],list_right[0]) ... (list_left[len],list_right[len])
  //   are disjoint, with dist > 1 intervals

  int len = 0;
  list_left[len]  = fd_iv_left_sort[len];
  list_right[len] = fd_iv_right_sort[len];
  
  for (i = 1; i < list_len; i++) {
    // does it overlap or is next?
    if (list_right[len] >= fd_iv_left_sort[i]-1) {
      // do not take over, just do union (keep in mind that left is sorted)
      list_right[len] = max(list_right[len],fd_iv_right_sort[i]);
    } else {
      // just copy
      len++;
      list_left[len]  = fd_iv_left_sort[i];
      list_right[len] = fd_iv_right_sort[i];
    }
  }
  
  len++;

  // Assert the invariant
  for (i = 1; i < len; i++) {
    Assert(list_right[i-1] <= list_left[i]);
  }

  return len;
}

// expects valid intervals, ie fd_inf <= left <= right <= fd_sup
inline
int OZ_FiniteDomainImpl::initList(int list_len,
				  int * list_left, int * list_right,
				  int list_min, int list_max)
{
  if (list_len == 0) {
    return initEmpty();
  } else if (list_len == 1) {
    min_elem = list_min;
    max_elem = list_max;
    size = max_elem - min_elem + 1;
    setType(fd_descr);
  } else {
    min_elem = list_min;
    max_elem = list_max;

    if (list_max <= fd_bv_max_elem) {
      FDBitVector * bv = provideBitVector(word32(list_max));
      bv->initList(list_len, list_left, list_right);
      size = bv->findSize();
      setType(bv);
    } else {
      int new_len = simplify(list_len, list_left, list_right);
      FDIntervals * iv = provideIntervals(new_len);
      iv->initList(new_len, list_left, list_right);
      size = iv->findSize();
      setType(iv);
    }
  }
  if (isSingleInterval()) setType(fd_descr);
  return size;
}

inline
OZ_FiniteDomainImpl OZ_FiniteDomainImpl::operator ~ (void) const
{
  DEBUG_FD_IR(("{FDIR.'operator ~' %s ", this->toString()));
  
  OZ_FiniteDomainImpl y; y.initEmpty();

  if (*this == fd_empty) {
    y.initFull();
  } else if (size == fd_full_size) {
    y.initEmpty();
  } else {
    descr_type type = getType();
    if (type == fd_descr) {
      if (min_elem == 0) {
	y.min_elem = max_elem + 1;
	y.max_elem = fd_sup;
	y.size = y.findSize();
	AssertFD(y.isConsistent());
      } else if (max_elem == fd_sup) {
	y.max_elem = min_elem - 1;
	y.min_elem = 0;
	y.size = y.findSize();
	AssertFD(y.isConsistent());
      } else {
	FDIntervals * iv = newIntervals(2);
	iv->init(fd_inf, min_elem - 1, max_elem + 1, fd_sup);
	y.size = iv->findSize();
	y.min_elem = 0;
	y.max_elem = fd_sup;
	y.setType(iv);
	AssertFD(y.isConsistent());
      }
    } else {      // reserve one interval too many !!!
      FDIntervals * iv;
      if (type == bv_descr) {
	int s = get_bv()->mkRaw(fd_bv_left_conv, fd_bv_right_conv);
	int t = s + (0 < min_elem);
	iv = newIntervals(t)->complement(s, fd_bv_left_conv, fd_bv_right_conv);
      } else {
	FDIntervals * x_iv = get_iv();
	int s = x_iv->high - 1 + (fd_inf < min_elem) + (max_elem < fd_sup);
	iv = newIntervals(s)->complement(x_iv);
      }
      y.size = iv->findSize();
      y.min_elem = iv->findMinElem();
      y.max_elem = iv->findMaxElem();
      y.setType(iv);
      
      if (y.max_elem <= fd_bv_max_elem) {
	y.setType(y.asBitVector());
	Assert(y.size == y.get_bv()->findSize());
      }

      if (y.isSingleInterval()) y.setType(fd_descr);
      AssertFD(y.isConsistent());
    }
  }

  AssertFD(y.isConsistent());
  DEBUG_FD_IR(("%s}\n", y.toString()));
  
  return y;
}

int OZ_FiniteDomainImpl::initDescr(OZ_Term d)
{
  DEREF(d, d_ptr);

  Assert(!oz_isRef(d));
  if (oz_isSTuple(d) && tagged2SRecord(d)->getWidth() == 1) {
    initDescr((*tagged2SRecord(d))[0]);
    *this = ~ *this;
    return size;
  } else if (oz_isSmallInt(d)) {
    return initSingleton(tagged2SmallInt(d));
  } else if (AtomSup == d) {
    return initSingleton(fd_sup);
  } else if (oz_isSTuple(d)) {
    SRecord &t = *tagged2SRecord(d);
    OZ_Term t0 = oz_deref(t[0]), t1 = oz_deref(t[1]);
    return initRange(AtomSup == t0 ? fd_sup : tagged2SmallInt(t0),
		     AtomSup == t1 ? fd_sup : tagged2SmallInt(t1));
  } else if (AtomBool == d) {
    return initRange(0, 1);
  } else if (oz_isNil(d)) {
    return initEmpty();
  } else if (oz_isLTupleOrRef(d)) {
    EnlargeableArray<int> left_arr(FDOMNINITSIZE), right_arr(FDOMNINITSIZE);

    int min_arr = fd_sup, max_arr = 0;
    
    int len_arr = 0;
    while (oz_isLTuple(d)) {
      LTuple &list = *tagged2LTuple(d);
      OZ_Term val = list.getHead();
      
      DEREF(val, valptr);
      
      if (oz_isSmallInt(val)) {
	int v = tagged2SmallInt(val);
	if (v < fd_inf || fd_sup < v) goto for_loop;
	
	left_arr[len_arr] = right_arr[len_arr] = v;
	min_arr = min(min_arr, left_arr[len_arr]);
	max_arr = max(max_arr, right_arr[len_arr]);
	len_arr ++;
      } else if (AtomSup == val) {
	left_arr[len_arr] = right_arr[len_arr] = fd_sup;
	min_arr = min(min_arr, left_arr[len_arr]);
	max_arr = max(max_arr, right_arr[len_arr]);
	len_arr ++;
      } else if (AtomBool == val) {
	left_arr[len_arr] = 0;
	right_arr[len_arr] = 1;
	
	min_arr = min(min_arr, left_arr[len_arr]);
	max_arr = max(max_arr, right_arr[len_arr]);

	len_arr ++;
      } else if (oz_isSTuple(val)) {
	SRecord &t = *tagged2SRecord(val);
	OZ_Term t0 = oz_deref(t[0]), t1 = oz_deref(t[1]);

	int l = max(0, AtomSup == t0 ? fd_sup : tagged2SmallInt(t0));
	int r = min(fd_sup, AtomSup == t1 ? fd_sup : tagged2SmallInt(t1));

	if (l > r) goto for_loop;
	
	left_arr[len_arr] = l;
	right_arr[len_arr] = r;
	
	min_arr = min(min_arr, left_arr[len_arr]);
	max_arr = max(max_arr, right_arr[len_arr]);

	len_arr ++;
      } else {
	OZD_error("Unexpected case when creating finite domain.");
      }
      left_arr.request(len_arr);
      right_arr.request(len_arr);
    for_loop:
      d = oz_deref(list.getTail());
      Assert(!oz_isRef(d));
    } // for
    return initList(len_arr, left_arr, right_arr, min_arr, max_arr);
  }
  OZD_error("Unexpected term in finite description list.");
  return -1;
}

// used for unification of fdvar with boolvar
int OZ_FiniteDomainImpl::intersectWithBool(void)
{
  if (isIn(0))
    if (isIn(1)) 
      return -1; // boolean
    else
      return 0; // 0
  else
    if (isIn(1)) 
      return 1; // 1
    else
      return -2; // empty
}

inline
int OZ_FiniteDomainImpl::nextSmallerElem(int v) const
{
  DEBUG_FD_IR(("{FDIR.'nextSmaller' %s %d", this->toString(), v));

  descr_type type = getType();
  if (type == fd_descr) {
    if (v <= min_elem) {
      DEBUG_FD_IR(("-1}\n"));
      return -1;
    }
    if (v > max_elem) {
      return max_elem;
      DEBUG_FD_IR(("%d}\n", max_elem));
    }
    DEBUG_FD_IR(("%d}\n", (v - 1)));    
    return v - 1;
  } else {
#ifdef DEBUG_CHECK
  int r = type == bv_descr 
      ? get_bv()->nextSmallerElem(v, min_elem) 
      : get_iv()->nextSmallerElem(v, min_elem);
  DEBUG_FD_IR(("%d}\n", r));
  return r;
#else
    return type == bv_descr 
      ? get_bv()->nextSmallerElem(v, min_elem) 
      : get_iv()->nextSmallerElem(v, min_elem);
#endif
  }
} 

inline
int OZ_FiniteDomainImpl::nextLargerElem(int v) const
{
  DEBUG_FD_IR(("{FDIR.'nextLarger' %s %d ", this->toString()));

  descr_type type = getType();
  if (type == fd_descr) {
    if (v >= max_elem) {
      DEBUG_FD_IR(("-1}\n")); 
      return -1;
    }
    if (v < min_elem) {
      DEBUG_FD_IR(("%d}\n", min_elem));
      return min_elem;
    }
    DEBUG_FD_IR(("%d}\n", (v + 1)));
    return v + 1;
  } else { 
#ifdef DEBUG_CHECK
  int r = type == bv_descr
      ? get_bv()->nextLargerElem(v, max_elem)
      : get_iv()->nextLargerElem(v, max_elem);
  DEBUG_FD_IR(("%d}\n}", r));
  return r;
#else
    return type == bv_descr
      ? get_bv()->nextLargerElem(v, max_elem)
      : get_iv()->nextLargerElem(v, max_elem);
#endif
  }
} 

inline
int OZ_FiniteDomainImpl::lowerBound(int v) const
{
  if (isIn(v)) {
    descr_type type = getType();
    if (type == fd_descr) {
      return min_elem;
    } else {
      return type == bv_descr 
	? get_bv()->lowerBound(v, min_elem) 
	: get_iv()->lowerBound(v);
    }
  } 
  return -1;
} 

inline
int OZ_FiniteDomainImpl::upperBound(int v) const
{
  if (isIn(v)) {
    descr_type type = getType();
    if (type == fd_descr) {
      return max_elem;
    } else { 
      return type == bv_descr
	? get_bv()->upperBound(v, max_elem)
	: get_iv()->upperBound(v);
    }
  }
  return -1;
} 

inline
int OZ_FiniteDomainImpl::midElem(void) const
{
  DEBUG_FD_IR(("{FDIR.'mid' %s ", this->toString()));
  
  int mid = (min_elem + max_elem) / 2;

  if (isIn(mid)) {
    DEBUG_FD_IR(("%d}\n", mid));

    return mid;
  }
  descr_type type = getType();
  Assert(type != fd_descr);
  
#ifdef DEBUG_CHECK
  int r = ((type == bv_descr) ? get_bv()->midElem(mid) : get_iv()->midElem(mid));
  DEBUG_FD_IR(("%d}\n", r));
  return r;
#else
  return type == bv_descr ? get_bv()->midElem(mid) : get_iv()->midElem(mid);
#endif
}

inline
OZ_Term OZ_FiniteDomainImpl::getAsList(void) const
{
  if (size == 0) return AtomNil;
  
  descr_type type = getType();
  if (type == fd_descr) {
    return makeTaggedLTuple(new LTuple(min_elem == max_elem 
				       ? OZ_int(min_elem)
				       : oz_pairII(min_elem, max_elem), 
				       AtomNil));
  } else if (type == bv_descr) {
    return get_bv()->getAsList();
  } else {
    return get_iv()->getAsList();
  }
}
  
inline
int OZ_FiniteDomainImpl::operator &= (const int i)
{
  DEBUG_FD_IR(("{FDIR.'operator &=i' %s %d ", this->toString(), i));
  if (isIn(i)) {
    initSingleton(i);
    AssertFD(isConsistent());
    DEBUG_FD_IR(("%s}\n", this->toString()));
    return 1;
  } else {
    initEmpty();
    AssertFD(isConsistent());
    DEBUG_FD_IR(("%s}\n", this->toString()));
    return 0;
  }
}

inline
int OZ_FiniteDomainImpl::operator <= (const int leq)
{
  DEBUG_FD_IR(("{FDIR.'operator <=' %s %d ", this->toString(), leq));

  if (leq < min_elem) {
    AssertFD(isConsistent());
    DEBUG_FD_IR(("nil}\n"));
    return initEmpty();
  } else if (leq < max_elem) {
    descr_type type = getType();
    if (type == fd_descr) {
      max_elem = min(max_elem, leq);
      size = findSize();
    } else if (type == bv_descr) {
      FDBitVector * bv = get_bv();
      size = (*bv <= leq);
      if (size > 0) max_elem = bv->findMaxElem();
    } else if (leq <= fd_sup) {
      FDIntervals * iv = get_iv();

      size = (*iv <= leq);
      if (size > 0) max_elem = iv->findMaxElem();
      if (max_elem <= fd_bv_max_elem) {
	setType(asBitVector());
	iv->dispose();
      }
    }
  }
  if (isSingleInterval()) setType(fd_descr);
  AssertFD(isConsistent());
  DEBUG_FD_IR(("%s}\n", this->toString()));
  return size;
}

inline
int OZ_FiniteDomainImpl::operator >= (const int geq)
{
  DEBUG_FD_IR(("{FDIR.'operator >=' %s %d ", this->toString(), geq));
  
  if (geq > max_elem) {
    AssertFD(isConsistent());
    DEBUG_FD_IR(("nil}\n"));
    return initEmpty();
  } else if (geq > min_elem) {
    descr_type type = getType();
    if (type == fd_descr) {
      min_elem = max(min_elem, geq);
      size = findSize();
    } else if (type == bv_descr) {
      FDBitVector * bv = get_bv();
      size = (geq > bv->currBvMaxElem()) ? initEmpty() : (*bv >= geq);
      if (size > 0) min_elem = bv->findMinElem();
    } else {
      FDIntervals * iv = get_iv();
      size = (*iv >= geq);
      if (size > 0) min_elem = iv->findMinElem();
    }
  }
  if (isSingleInterval()) setType(fd_descr);
  AssertFD(isConsistent());
  DEBUG_FD_IR(("%s}\n", this->toString()));
  return size;
}

inline
int OZ_FiniteDomainImpl::operator -= (const int take_out)
{
  DEBUG_FD_IR(("{FDIR.'operator -=i' %s %d ", this->toString(), take_out));
  if (isIn(take_out)) {
    descr_type type = getType();
    if (type == fd_descr) {
      if (take_out == min_elem) {
	min_elem += 1;
      } else if (take_out == max_elem) {
	max_elem -= 1;
      } else {
	if (max_elem <= fd_bv_max_elem) {
	  FDBitVector * bv = provideBitVector(word32(max_elem));
	  bv->setFromTo(min_elem, max_elem);
	  bv->resetBit(take_out);
	  min_elem = bv->findMinElem();
	  max_elem = bv->findMaxElem();
	  setType(bv);
	} else {
	  FDIntervals * iv = provideIntervals(2);
	  iv->init(min_elem, take_out - 1, take_out + 1, max_elem);
	  setType(iv);
	}
      }
    } else if (type == bv_descr) {
      FDBitVector * bv = get_bv();
      bv->resetBit(take_out);
      min_elem = bv->findMinElem();
      max_elem = bv->findMaxElem();
    } else {
      FDIntervals * iv = (*get_iv() -= take_out);
      min_elem = iv->findMinElem();
      max_elem = iv->findMaxElem();
      setType(iv);
      if (max_elem <= fd_bv_max_elem) {
	setType(asBitVector());
	iv->dispose();
      }
    }
    size -= 1;
    if (isSingleInterval()) setType(fd_descr);
  }
  AssertFD(isConsistent());
  DEBUG_FD_IR(("%s}\n", this->toString()));
  return size;
}

inline
int OZ_FiniteDomainImpl::operator -= (const OZ_FiniteDomainImpl &y)
{
  DEBUG_FD_IR(("FDIR.'operator -=' %s %s ", this->toString(), y.toString()));
  if (y != fd_empty) {
    descr_type x_type = getType(), y_type = y.getType();
    if (x_type == fd_descr) {
      if (y_type == fd_descr) {
	if (y.max_elem < min_elem || max_elem < y.min_elem) {
	  AssertFD(isConsistent());
	  DEBUG_FD_IR(("%s}\n", this->toString()));
	  return size;
	} else if (y.min_elem <= min_elem && max_elem <= y.max_elem) {
	  size = 0;
	} else if (min_elem < y.min_elem && y.max_elem < max_elem) {
	  if (max_elem <= fd_bv_max_elem) {
	    FDBitVector * bv = provideBitVector(word32(max_elem));
	    bv->setFromTo(min_elem, y.min_elem - 1);
	    bv->addFromTo(y.max_elem + 1, max_elem);
	    size = bv->findSize();
	    setType(bv);
	  } else {
	    FDIntervals * iv = provideIntervals(2);
	    iv->init(min_elem, y.min_elem - 1, y.max_elem + 1, max_elem);
	    size = iv->findSize();
	    setType(iv);
	  }
	} else if (y.max_elem < max_elem) {
	  min_elem = y.max_elem + 1;
	  size = findSize();
	} else {
	  max_elem = y.min_elem - 1;
	  size = findSize();
	}
      } else if (y_type == bv_descr) {
        // error if x_type == fd_descr and has elements above fd_bv_max_elem
        if (max_elem > fd_bv_max_elem) {
          FDIntervals *iv = asIntervals();
          FDIntervals *y_iv = y.asIntervals();
          FDIntervals *z_iv = provideIntervals(iv->high + y_iv->high);
          size=iv->subtract_iv(*z_iv, *y_iv);
          min_elem = z_iv->findMinElem();
          max_elem = z_iv->findMaxElem();
          setType(z_iv);
          if (max_elem <= fd_bv_max_elem) {
            setType(asBitVector());
            z_iv->dispose();
          }
        } else {
          FDBitVector * bv = asBitVector();
          size = (*bv -= *y.get_bv());
          min_elem = bv->findMinElem();
          max_elem = bv->findMaxElem();
          setType(bv);
        }
      } else { // y_type == iv_descr
	FDIntervals * iv = newIntervals(1 + y.get_iv()->high);
	size = asIntervals()->subtract_iv(*iv, *y.get_iv());
	min_elem = iv->findMinElem();
	max_elem = iv->findMaxElem();
	setType(iv);
	if (max_elem <= fd_bv_max_elem) {
	  setType(asBitVector());
	  iv->dispose();
	}
      }
    } else if (x_type == bv_descr) {
      FDBitVector * bv = get_bv();
      size = (*bv -= *y.asBitVector());
      min_elem = bv->findMinElem();
      max_elem = bv->findMaxElem();
    } else { // x_type == iv_descr
      FDIntervals * y_iv = y.asIntervals();
      FDIntervals * iv = newIntervals(get_iv()->high + y_iv->high);
      size = get_iv()->subtract_iv(*iv, *y_iv);
      min_elem = iv->findMinElem();
      max_elem = iv->findMaxElem();
      setType(iv);
      if (max_elem <= fd_bv_max_elem) {
	setType(asBitVector());
	iv->dispose();
      }
    }
    
    if (isSingleInterval()) setType(fd_descr);
  }
  AssertFD(isConsistent());
  DEBUG_FD_IR(("%s}\n", this->toString()));
  return size;
}

inline
int OZ_FiniteDomainImpl::operator += (const int put_in)
{
  DEBUG_FD_IR(("{FDIR.'operator +=i' %s %d ", this->toString(), put_in));
  
  if (put_in < fd_inf || fd_sup < put_in) {
    DEBUG_FD_IR(("%s}\n", this->toString()));
    return size;
  }
  if (size == 0) {
    min_elem = max_elem = put_in;
    size = 1;
  } else if (!isIn(put_in)) {
    descr_type type = getType();
    if (type == fd_descr) {
      if (put_in == min_elem - 1) {
	min_elem -= 1;
      } else if (put_in == max_elem + 1) {
	max_elem += 1;
      } else {
	int max_put_in = max(put_in, max_elem);
	if (max_put_in <= fd_bv_max_elem) {
	  FDBitVector * bv = provideBitVector(word32(max_put_in));
	  bv->setFromTo(min_elem, max_elem);
	  bv->setBit(put_in);
	  min_elem = bv->findMinElem();
	  max_elem = bv->findMaxElem();
	  setType(bv);
	} else {
	  FDIntervals * iv = provideIntervals(2);
	  if (put_in < min_elem) {
	    iv->init(put_in, put_in, min_elem, max_elem);
	    min_elem = put_in;
	  } else {
	    iv->init(min_elem, max_elem, put_in, put_in);
	    max_elem = put_in;
	  }
	  setType(iv);
	}
      }
    } else if (type == bv_descr) {
      FDBitVector * bv = get_bv();
      if (put_in <= bv->currBvMaxElem()) {
	bv->setBit(put_in);
	min_elem = bv->findMinElem();
	max_elem = bv->findMaxElem();
      } else if (put_in <= fd_bv_max_elem) {
	FDBitVector * bv_backup = bv;
	bv = newBitVector(word32(put_in));
	int i;
	for (i = bv_backup->high; i--; )
	  bv->b_arr[i] = bv_backup->b_arr[i];
	for (i = bv_backup->high; i < bv->high; i++)
	  bv->b_arr[i] = 0;
	bv_backup->dispose();
	bv->setBit(put_in);
	min_elem = bv->findMinElem();
	max_elem = bv->findMaxElem();
	setType(bv);
      } else {
	int c_len = get_bv()->mkRaw(fd_bv_left_conv, fd_bv_right_conv);
	FDIntervals * iv;
	if (put_in == max_elem + 1) {
	  iv = provideIntervals(c_len);
	  fd_bv_right_conv[c_len - 1] += 1;
	  iv->initList(c_len, fd_bv_left_conv, fd_bv_right_conv);
	} else {
	  iv = provideIntervals(c_len + 1);
	  fd_bv_left_conv[c_len] = fd_bv_right_conv[c_len] = put_in;
	  iv->initList(c_len + 1, fd_bv_left_conv, fd_bv_right_conv);
	}
	max_elem = put_in;
	setType(iv);
      }
    } else {
      FDIntervals * iv = (*get_iv() += put_in);
      min_elem = iv->findMinElem();
      max_elem = iv->findMaxElem();
      setType(iv);
    }
    size += 1;
  }
  if (isSingleInterval()) setType(fd_descr);

  AssertFD(isConsistent());
  DEBUG_FD_IR(("%s}\n", this->toString()));
  return size;
}

// this is a pretty dirty hack; it will disappear when Markus' changes
// get incorporated
inline
int OZ_FiniteDomainImpl::initFSetValue(const OZ_FSetValue &fs)
{
#ifdef BIGFSET
  FDBitVector * bv = newBitVector(fset_high);
  const int * set_bv = ((const FSetValue *) &fs)->getBV();
  
  for (int i = fset_high; i--; )
    bv->b_arr[i] = set_bv[i];
    
  setType(bv);
  min_elem = bv->findMinElem();
  max_elem = bv->findMaxElem();

  if (fs._other) {
    FDIntervals *f = asIntervals();
    FDIntervals *other = newIntervals(1);
    other->init(32*fset_high, fs_sup);
    FDIntervals *z = newIntervals(f->high + 1);
    z->union_iv(*f, *other);
    setType(z);
    min_elem = z->findMinElem();
    max_elem = z->findMaxElem();
  }
  return size = fs.getCard();

#else
  FDBitVector * bv = newBitVector(fset_high);
  const int * set_bv = ((const FSetValue *) &fs)->getBV();

  for (int i = fset_high; i--; )
      bv->b_arr[i] = set_bv[i];

  setType(bv);
  min_elem = bv->findMinElem();
  max_elem = bv->findMaxElem();


  return size = fs.getCard();
#endif
}

inline
OZ_FiniteDomainImpl OZ_FiniteDomainImpl::operator | (const OZ_FiniteDomainImpl &y) const
{
  DEBUG_FD_IR(("{FDIR.'operator |' %s %s ", this->toString(), y.toString()));
  
  OZ_FiniteDomainImpl z; z.initEmpty();

  if (*this == fd_empty) {
    z = y;
  } else if (y == fd_empty) {
    z = *this;
  } else if (max(max_elem, y.max_elem) > fd_bv_max_elem) {
    FDIntervals * x_v = asIntervals();
    FDIntervals * y_v = y.asIntervals();
    FDIntervals * z_v;
    z.setType(z_v = newIntervals(x_v->high + y_v->high));
    z.size = z_v->union_iv(*x_v, *y_v);
    z.min_elem = z_v->findMinElem();
    z.max_elem = z_v->findMaxElem();
  } else {
    FDBitVector * x_v = asBitVector();
    FDBitVector * y_v = y.asBitVector();
    FDBitVector * z_v = newBitVector(max(x_v->high, y_v->high));
    z.setType(z_v);
    z.size = z_v->union_bv(*x_v, *y_v);
    z.min_elem = z_v->findMinElem();
    z.max_elem = z_v->findMaxElem();
  }
  if (z.isSingleInterval()) z.setType(fd_descr);

  AssertFD(z.isConsistent());
  DEBUG_FD_IR(("%s}\n", z.toString()));
  
  return z;
}

inline
int OZ_FiniteDomainImpl::operator &= (const OZ_FiniteDomainImpl &y)
{
  DEBUG_FD_IR(("{FDIR.'operator &=' %s %s ", this->toString(), y.toString()));

  if (*this == fd_empty || y == fd_empty) {
    initEmpty();
    AssertFD(isConsistent());
    DEBUG_FD_IR(("nil}\n"));
    return 0;
  } else if (getType() == fd_descr && y.getType() == fd_descr) {
    if (max_elem < y.min_elem || y.max_elem < min_elem) {
      size = 0;
    } else {
      min_elem = max(min_elem, y.min_elem);
      max_elem = min(max_elem, y.max_elem);
      size = findSize();
    }
  } else if (min(max_elem, y.max_elem) > fd_bv_max_elem) {
    FDIntervals * x_i = asIntervals();
    FDIntervals * y_i = y.asIntervals();
    FDIntervals * z_i = newIntervals(x_i->high + y_i->high - 1);
    
    size = x_i->intersect_iv(*z_i, *y_i);
    min_elem = z_i->findMinElem();
    max_elem = z_i->findMaxElem();
    setType(z_i);
  } else {
    FDBitVector * x_b = asBitVector();
    FDBitVector * y_b = y.asBitVector();

    size = x_b->intersect_bv(*y_b);
    min_elem = x_b->findMinElem();
    max_elem = x_b->findMaxElem();
    setType(x_b);
  }
  
  if (isSingleInterval()) setType(fd_descr);
  
  AssertFD(isConsistent());
  DEBUG_FD_IR((" %s}\n", this->toString()));

  return size;
}

inline
OZ_FiniteDomainImpl OZ_FiniteDomainImpl::operator & (const OZ_FiniteDomainImpl &y) const
{
  DEBUG_FD_IR(("{FDIR.'operator &' %s %s ", this->toString(), y.toString()));
  
  OZ_FiniteDomainImpl z; z.initEmpty();

  if (*this == fd_empty || y == fd_empty) {
    AssertFD(z.isConsistent());
    DEBUG_FD_IR(("nil}\n"));
    return z;
  } else if (getType() == fd_descr && y.getType() == fd_descr) {
    if (max_elem < y.min_elem || y.max_elem < min_elem) {
      z.size = 0;
    } else {
      z.min_elem = max(min_elem, y.min_elem);
      z.max_elem = min(max_elem, y.max_elem);
      z.size = z.findSize();
    }
  } else if (min(max_elem, y.max_elem) > fd_bv_max_elem) {
    FDIntervals * x_i = asIntervals();
    FDIntervals * y_i = y.asIntervals();
    FDIntervals * z_i = newIntervals(x_i->high + y_i->high - 1);
    
    z.size = x_i->intersect_iv(*z_i, *y_i);
    z.min_elem = z_i->findMinElem();
    z.max_elem = z_i->findMaxElem();
    z.setType(z_i);
  } else {
    FDBitVector * x_b = asBitVector();
    FDBitVector * y_b = y.asBitVector();
    FDBitVector * z_b = newBitVector(min(x_b->getHigh(), y_b->getHigh()));
    
    z.size = z_b->intersect_bv(*x_b, *y_b);
    // tmueller: dispose x_b and y_b in case extra memory has been allocated
    z.min_elem = z_b->findMinElem();
    z.max_elem = z_b->findMaxElem();
    z.setType(z_b);
  }
  
  if (z.isSingleInterval()) z.setType(fd_descr);

  AssertFD(z.isConsistent());
  DEBUG_FD_IR(("%s}\n", z.toString()));

  return z;
}

inline
int OZ_FiniteDomainImpl::constrainBool(void)
{
  return *this <= 1;
}

inline
void OZ_FiniteDomainImpl::copyExtensionInline(void) 
{
  descr_type type = getType();
  if (type == fd_descr) {
    setType(fd_descr, NULL);
  } else if (type == bv_descr) {
    setType(get_bv()->copy());
  } else {
    setType(get_iv()->copy());
  }  
}

void OZ_FiniteDomainImpl::copyExtension(void) 
{
  copyExtensionInline();
}

//-----------------------------------------------------------------------------
// 
//

#define CASTPTR (OZ_FiniteDomainImpl *)
#define CASTCONSTPTR (const OZ_FiniteDomainImpl *)
#define CASTREF * (const OZ_FiniteDomainImpl *) &
#define CASTTHIS (CASTPTR this)
#define CASTCONSTTHIS (CASTCONSTPTR this)

OZ_FiniteDomain::OZ_FiniteDomain(OZ_Term t)
{
  CASTTHIS->FiniteDomainInit(NULL);
  CASTTHIS->initDescr(t);
}

OZ_FiniteDomain::OZ_FiniteDomain(OZ_FDState state)
{
  switch (state) {
  case fd_empty:
    CASTTHIS->initEmpty();
    break;
  case fd_full:
    CASTTHIS->initFull();
    break;
  case fd_bool:
    CASTTHIS->initBool();
    break;
  default:
    OZD_error("Unexpected OZ_FDState.");
    break;
  }
}

OZ_FiniteDomain::OZ_FiniteDomain(const OZ_FiniteDomain &fd)
{
  CASTTHIS->operator =(CASTREF fd);
}

OZ_FiniteDomain::OZ_FiniteDomain(const OZ_FSetValue &fs)
{
#ifdef BIGFSET
  if (fs._normal) {
    CASTTHIS->FiniteDomainInit(NULL);
    CASTTHIS->initFSetValue(fs);
  }
  else {
    CASTTHIS->operator =(CASTREF fs._IN);
  }
#else
  CASTTHIS->FiniteDomainInit(NULL);
  CASTTHIS->initFSetValue(fs);
#endif
}

void OZ_FiniteDomain::disposeExtension(void)
{
  CASTTHIS->disposeExtension();
}

const OZ_FiniteDomain &OZ_FiniteDomain::operator = (const OZ_FiniteDomain &fd)
{
  return CASTREF CASTTHIS->operator = (CASTREF fd);
}

int OZ_FiniteDomain::initFull(void)
{
  return CASTTHIS->initFull();
}

int OZ_FiniteDomain::initEmpty(void)
{
  return CASTTHIS->initEmpty();
}

int OZ_FiniteDomain::initDescr(OZ_Term t)
{
  return CASTTHIS->initDescr(t);
}

int OZ_FiniteDomain::initSingleton(int s)
{
  return CASTTHIS->initSingleton(s);
}

int OZ_FiniteDomain::initRange(int l, int u)
{
  return CASTTHIS->initRange(l, u);
}

int OZ_FiniteDomain::initBool(void)
{
  return CASTTHIS->initBool();
}

int OZ_FiniteDomain::getSingleElem(void) const
{
  return CASTCONSTTHIS->getSingleElem();
}
   
OZ_Boolean OZ_FiniteDomain::isIn(int i) const
{
  return CASTCONSTTHIS->isIn(i);
}

OZ_Term OZ_FiniteDomain::getDescr(void) const
{
  return CASTCONSTTHIS->getAsList();
}

int OZ_FiniteDomain::getMidElem(void) const
{
  return CASTCONSTTHIS->midElem();
}

int OZ_FiniteDomain::getNextSmallerElem(int v) const
{
  return CASTCONSTTHIS->nextSmallerElem(v);
}

int OZ_FiniteDomain::getNextLargerElem(int v) const
{
  return CASTCONSTTHIS->nextLargerElem(v);
}

int OZ_FiniteDomain::getLowerIntervalBd(int v) const
{
  return CASTCONSTTHIS->lowerBound(v);
}

int OZ_FiniteDomain::getUpperIntervalBd(int v) const
{
  return CASTCONSTTHIS->upperBound(v);
}

int OZ_FiniteDomain::constrainBool(void)
{
  return CASTTHIS->constrainBool();
}

int OZ_FiniteDomain::intersectWithBool(void)
{
  return CASTTHIS->intersectWithBool();
}

OZ_FiniteDomain OZ_FiniteDomain::operator & (const OZ_FiniteDomain & y) const
{
  return CASTCONSTTHIS->operator & (CASTREF y);
}

OZ_FiniteDomain OZ_FiniteDomain::operator | (const OZ_FiniteDomain & y) const
{
  return CASTCONSTTHIS->operator | (CASTREF y);
}

OZ_FiniteDomain OZ_FiniteDomain::operator ~ (void) const
{
  return CASTCONSTTHIS->operator ~ ();
}

int OZ_FiniteDomain::operator &= (const OZ_FiniteDomain &y)
{
  return CASTTHIS->operator &= (CASTREF y);
}

int OZ_FiniteDomain::operator &= (const int y)
{
  return CASTTHIS->operator &= (y);
}

int OZ_FiniteDomain::operator -= (const OZ_FiniteDomain &y)
{
  return CASTTHIS->operator -= (CASTREF y);
}

int OZ_FiniteDomain::operator -= (const int y)
{
  return CASTTHIS->operator -= (y);
}

int OZ_FiniteDomain::operator += (const int y)
{
  return CASTTHIS->operator += (y);
}

int OZ_FiniteDomain::operator <= (const int y)
{
  return CASTTHIS->operator <= (y);
}

int OZ_FiniteDomain::operator >= (const int y)
{
  return CASTTHIS->operator >= (y);
}

OZ_Boolean OZ_FiniteDomain::operator == (const OZ_FDState s) const
{
  return CASTCONSTTHIS->operator == (s);
}

OZ_Boolean OZ_FiniteDomain::operator != (const OZ_FDState s) const
{
  return CASTCONSTTHIS->operator != (s);
}

OZ_Boolean OZ_FiniteDomain::operator == (const int i) const
{
  return CASTCONSTTHIS->operator == (i);
}

OZ_Boolean OZ_FiniteDomain::operator != (const int i) const
{
  return CASTCONSTTHIS->operator != (i);
}

#define RANGESTR "#"

void FDBitVector::print(ostream &ofile, int idnt) const
{
  Bool flag = FALSE;

  int len = mkRawOutline(fd_bv_left_conv, fd_bv_right_conv);

#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
  if (len == 0) {
    ofile << "nil";
    return;
  }
  ofile << '[';
#else
  ofile << '{';
#endif

  for (int i = 0; i < len; i += 1) {
    if (flag) ofile << ' '; else flag = TRUE; 
    ofile << fd_bv_left_conv[i];
    if (fd_bv_left_conv[i] != fd_bv_right_conv[i])
      if (fd_bv_left_conv[i] + 1 == fd_bv_right_conv[i])
	ofile << ' ' << fd_bv_right_conv[i];
      else
	ofile << RANGESTR << fd_bv_right_conv[i];
  }
#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
  ofile << ']';
#else
  ofile << '}';
#endif
}

void printFromTo(ostream &ofile, int f, int t)
{
  if (f == t)
    ofile << f;
  else if ((t - f) == 1)
    ofile << f << ' ' << t;
  else
    ofile << f << RANGESTR << t;
}

void FDIntervals::print(ostream &ofile, int idnt) const
{
  Bool flag = FALSE;

#if  defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
  if (high == 0) {
    ofile << "nil";
    return;
  }
  ofile << '[';
#else
  ofile << '{';
#endif
  for (int i = 0; i < high; i += 1) {
    if (flag) ofile << ' '; else flag = TRUE; 
    printFromTo(ofile, i_arr[i].left, i_arr[i].right);
  }
#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
  ofile << ']';
#else
  ofile << '}';
#endif
}


void OZ_FiniteDomainImpl::print(ostream &ofile, int idnt) const
{
  if (getSize() == 0) {
#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
    ofile << "nil";
#else
    ofile << "{}";
#endif
  } else switch (getType()) {
  case fd_descr:    
#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
      ofile << '[';
#else
      ofile << '{';
#endif
      printFromTo(ofile, min_elem, max_elem);
#if defined(DEBUG_FD_CONSTRREP) || defined(DEBUG_FSET_CONSTRREP)
      ofile << ']';
#else
      ofile << '}';
#endif
    break;
  case bv_descr:
    get_bv()->print(ofile, idnt);
    break;
  case iv_descr:
    get_iv()->print(ofile, idnt);
    break;
  default:
    OZD_error("unexpected case");
  }
}

char *OZ_FiniteDomain::toString() const
{
  static ozstrstream str;
  str.reset();
  CASTCONSTTHIS->print(str, 0);

#ifdef DEBUG_FD_CONSTRREP_DETAILED_OUTPUT
  static ozstrstream tmp_str;
  tmp_str.reset();
  tmp_str << "fd(" << str.str() << ")" 
	  << "@" << this
	  << flush;
  return tmp_str.str();
#else
  return str.str();
#endif
}

void OZ_FiniteDomain::copyExtension(void)
{
  CASTTHIS->copyExtensionInline();
}

