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
#pragma implementation "fdomn.hh"
#endif

#include "fdomn.hh"

//-----------------------------------------------------------------------------
// Miscellaneous --------------------------------------------------------------

extern unsigned char numOfBitsInByte[];
extern int toTheLowerEnd[];
extern int toTheUpperEnd[];

int fd_bv_left_conv[fd_bv_conv_max_high];
int fd_bv_right_conv[fd_bv_conv_max_high];

intptr fd_iv_left_sort[MAXFDBIARGS];
intptr fd_iv_right_sort[MAXFDBIARGS];

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }
inline int word32(int n) { return mod32(n) ? div32(n) + 1 : div32(n); }

//-----------------------------------------------------------------------------
// FDInterval -----------------------------------------------------------------

inline
FDIntervals * newIntervals(int max_index) {
  return (max_index > fd_iv_max_high)
    ? new (max_index) FDIntervals(max_index) : new FDIntervals(max_index);
} 

inline
int FDIntervals::findSize(void) {
  int s, i;
  for (s = 0, i = high; i--; )
    s += (i_arr[i].right - i_arr[i].left + 1);

  return s;
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
FDIntervals * FDIntervals::copy(void)
{
  FDIntervals * new_item = (high > fd_iv_max_high)
    ? new (high) FDIntervals(high) : new FDIntervals(high);

  for (int i = high; i--; ) 
    new_item->i_arr[i] = i_arr[i];

  return new_item;
}

inline
void FDIntervals::copy(FDIntervals * source)
{
  AssertFD(high >= source->high);
  
  high = source->high;
  for (int i = high; i--; )
    i_arr[i] = source->i_arr[i];
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
  for (int i = high; i--; ) 
    i_arr[i] = iv.i_arr[i];

  return *this;
}

inline
FDIntervals::FDIntervals(const FDIntervals &iv) {
  *this = iv;
}


OZ_Boolean FDIntervals::isConsistent(void) const {
  if (high < 0) return OZ_FALSE;
  int i;
  for (i = 0; i < high; i++) {
    if (i_arr[i].left > i_arr[i].right) return OZ_FALSE;
    if ((i + 1 < high) && (i_arr[i].right >= i_arr[i + 1].left)) return OZ_FALSE;
  }
  for (i = 0; i < high - 1; i++) 
    if (! ((i_arr[i].right + 1) < i_arr[i + 1].left)) return OZ_FALSE;
  return OZ_TRUE;
}


//-----------------------------------------------------------------------------
// class FDInterval -----------------------------------------------------------

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
int FDIntervals::nextBiggerElem(int v, int max_elem) const
{
  for (int i = 0; i < high; i += 1) {
    if (i_arr[i].left <= v && i_arr[i].right < v)
      return v + 1;
    if (v <= i_arr[i].left)
      return i_arr[i].left;
  }
  error ("no bigger element in domain");
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
OZ_Boolean FDIntervals::contains(int i) const
{
  int index = findPossibleIndexOf(i);
  return (i_arr[index].left <= i && i <= i_arr[index].right);
}

inline
OZ_Boolean FDIntervals::next(int i, int &n) const
{
  if (contains(i)) {
    n = i;
    return OZ_FALSE;
  }

  int j;
  for (j = 0;
       j < high - 1 &&
       !(i_arr[j].right < i && i < i_arr[j + 1].left);
       j += 1);

  int l = i_arr[j].right, r = i_arr[j + 1].left;

  if ((r - i) == (i - l)) {
    n = l;
    return OZ_TRUE;
  } else {
    n = ((r - i) < (i - l)) ? r : l;
    return OZ_FALSE;
  }
}

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
OZ_Term FDIntervals::getAsList(void) const
{
  LTuple * hd = NULL, * l_ptr = NULL;

  for (int i = 0; i < high; i += 1) 
      l_ptr = (i_arr[i].left == i_arr[i].right)
	? mkListEl(hd, l_ptr, OZ_int(i_arr[i].left))
	: mkListEl(hd, l_ptr, mkTuple(i_arr[i].left, i_arr[i].right));
  
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
    int new_max_high = high + 1;
    if (new_max_high <= fd_iv_max_high) {
      high = new_max_high;

      for (int i = high - 1; i > index; i -= 1)
	i_arr[i] = i_arr[i - 1];
      i_arr[index].right = take_out - 1;
      i_arr[index + 1].left = take_out + 1;
    } else {
      FDIntervals * new_iv = new(new_max_high) FDIntervals(new_max_high);
      int i;
      for (i = 0; i <= index; i += 1)
	new_iv->i_arr[i] = i_arr[i];
      new_iv->i_arr[index].right = take_out - 1;
      for (i = index; i < high; i += 1)
	new_iv->i_arr[i + 1] = i_arr[i];
      new_iv->i_arr[index + 1].left = take_out + 1;

      AssertFD(new_iv->isConsistent());
  
      return new_iv;
    }
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
    if (high <= fd_iv_max_high) {
      for (int i = high - 1; index < i; i -= 1)
	i_arr[i] = i_arr[i - 1];
      i_arr[index].left = i_arr[index].right = put_in;
    } else {
      FDIntervals * new_iv = new(high) FDIntervals(high);
      int i;
      for (i = 0; i < index; i += 1)
	new_iv->i_arr[i] = i_arr[i];
      for (i = high - 1; index < i; i -= 1)
	new_iv->i_arr[i] = i_arr[i - 1];
      new_iv->i_arr[index].left = new_iv->i_arr[index].right = put_in;
      AssertFD(new_iv->isConsistent());
      return new_iv;
    }
  }

  AssertFD(isConsistent());

  return this;
}

inline
FDIntervals * FDIntervals::complement(FDIntervals * x_iv)
{
  int c_i = 0, i = 0;
  if (0 < i_arr[i].left) {
    i_arr[c_i].left = 0;
    i_arr[c_i].right = x_iv->i_arr[i].left - 1;
    c_i += 1;
  }
  for ( ; i < x_iv->high - 1; i += 1, c_i += 1) {
    i_arr[c_i].left = x_iv->i_arr[i].right + 1;
    i_arr[c_i].right = x_iv->i_arr[i + 1].left - 1;
  }
  if (i_arr[i].right < fd_sup) {
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
    } else {
      i_arr[z_c].left = y.i_arr[y_c].left;
      r = y.i_arr[y_c].right;
      y_c += 1;
    }
    
    for (OZ_Boolean cont = OZ_TRUE; cont; )
      if (x_c < x.high &&
	  x.i_arr[x_c].left <= r + 1 && r <= x.i_arr[x_c].right) {
	r = x.i_arr[x_c].right;
	x_c += 1;
      } else if (y_c < y.high &&
		 y.i_arr[y_c].left <= r + 1 && r <= y.i_arr[y_c].right) {
	r = y.i_arr[y_c].right;
	y_c += 1;
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
OZ_Boolean FDBitVector::contains(int i) const {
  return (i > fd_bv_max_elem || i < 0)
    ? OZ_FALSE : (b_arr[div32(i)] & (1 << (mod32(i))));
}

// 0 <= i <= fd_bv_max_elem
inline
void FDBitVector::setBit(int i) {
  b_arr[div32(i)] |= (1 << (mod32(i)));
}

// 0 <= i <= fd_bv_max_elem
inline
void FDBitVector::resetBit(int i) {
  b_arr[div32(i)] &= ~(1 << (mod32(i)));
}

inline
void FDBitVector::setEmpty(void) {
  for (int i = fd_bv_max_high; i--; )
    b_arr[i] = 0;
}

inline
FDBitVector * FDBitVector::copy(void)
{
  FDBitVector * new_item = new FDBitVector;

  *new_item = *this;
  return new_item;
}

inline
void FDBitVector::addFromTo(int from, int to)
{
  AssertFD(0 <= from && from <= to && to <= fd_bv_max_elem);
  
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
int FDBitVector::findSize(void)
{
  int s, i;
  for (s = 0, i = fd_bv_max_high; i--; ) {
    s += numOfBitsInByte[unsigned(b_arr[i]) & 0xffff];
    s += numOfBitsInByte[unsigned(b_arr[i]) >> 16];
  }    

  return s;
}

inline
int FDBitVector::findMinElem(void)
{
  int v, i;
  for (v = 0, i = 0; i < fd_bv_max_high; v += 32, i += 1) 
    if (b_arr[i] != 0)
      break;

  if (i < fd_bv_max_high) {
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
  }
  return v;
}

inline
int FDBitVector::findMaxElem(void)
{
  int v, i;
  for (v = fd_bv_max_elem, i = fd_bv_max_high - 1; i >= 0; v -= 32, i--) 
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
  return v;
}

inline
void FDBitVector::setFromTo(int from, int to)
{
  AssertFD(0 <= from && from <= to && to <= fd_bv_max_elem);
  
  int low_word = div32(from), low_bit = mod32(from);
  int up_word = div32(to), up_bit = mod32(to);

  int i;
  for (i = 0; i < low_word; i++)
    b_arr[i] = 0;
  for (i = up_word + 1; i < fd_bv_max_high; i++)
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
int FDBitVector::nextBiggerElem(int v, int max_elem) const
{
  for (int new_v = v + 1; new_v <= max_elem; new_v += 1)
    if (contains(new_v))
      return new_v;
  
  error ("no bigger element in domain");
  return -1;
} 

inline
OZ_Boolean FDBitVector::next(int i, int &n) const
{
  if (contains(i)) {
    n = i;
    return OZ_FALSE;
  }

  // find lower neighbour
  int lb = mod32(i), lw = div32(i), ub = lb;
  if (!(b_arr[lw] << (32 - 1 - lb))) {
    lb = 32 - 1;    
    for (lw--; !b_arr[lw] && lw >= 0; lw--);
  }
  for (; lb >= 0 && !(b_arr[lw] & (1 << lb)); lb--);
  int l = 32 * lw + lb;

  // find upper neighbour
  int uw = div32(i);
  if (!(b_arr[uw] >> ub)) {
    ub = 0;
    for (uw++ ; !b_arr[uw] && uw < fd_bv_max_high; uw++);
  }
  for (; ub < 32 && !(b_arr[uw] & (1 << ub)); ub++);
  int u = 32 * uw + ub;

  if (u - i == i - l) {
    n = l;
    return OZ_TRUE;
  }

  n = (u - i < i - l) ? u : l;
  return OZ_FALSE;
}

inline
int FDBitVector::mkRaw(int * list_left, int * list_right) const
{
  int i, r, l, len;
  for (i = 0, r = 1, len = 0, l = -1; i < fd_bv_max_elem + 2; i += 1)
    if (contains(i)) {
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
      l_ptr = mkListEl(hd, l_ptr, OZ_int(fd_bv_left_conv[i]));
    else
      l_ptr = mkListEl(hd, l_ptr, mkTuple(fd_bv_left_conv[i],
					  fd_bv_right_conv[i]));
  
  return makeTaggedLTuple(hd);
}

// fd_inf <= leq <= fd_bv_max_elem
inline
int FDBitVector::operator <= (const int leq)
{
  int upper_word = div32(leq), upper_bit = mod32(leq);
  
  for (int i = upper_word + 1; i < fd_bv_max_high; i += 1)
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
int FDBitVector::union_bv(const FDBitVector &x, const int x_upper,
			  const FDBitVector &y, const int y_upper)
{
  for (int i = fd_bv_max_high; i--; )
    b_arr[i] = x.b_arr[i] | y.b_arr[i];

  return findSize();
}

inline
int FDBitVector::intersect_bv(FDBitVector &z, const FDBitVector &y)
{
  if (this == &z) 
    for (int i = fd_bv_max_high; i--; )
      b_arr[i] = b_arr[i] & y.b_arr[i];
  else
    for (int i = fd_bv_max_high; i--; )
      z.b_arr[i] = b_arr[i] & y.b_arr[i];

  return z.findSize();
}
     
inline
int FDBitVector::operator -= (const FDBitVector &y)
{
  for (int i = fd_bv_max_high; i--; )
    b_arr[i] = b_arr[i] & ~y.b_arr[i];
  return findSize();
}

inline
size_t FDBitVector::memory_required(void) { // used for profiling
  return 4 * size_t(ceil(float(findMaxElem())/32));
}


//-----------------------------------------------------------------------------
// class OZ_FiniteDomainImpl --------------------------------------------------

// private methods ------------------------------------------------------------

inline
void OZ_FiniteDomainImpl::setType(descr_type t) {
  descr = orPointer(andPointer(descr,~3),t);
}

inline  
void OZ_FiniteDomainImpl::setType(descr_type t, void * p) {
  descr = orPointer(p, t);
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
int OZ_FiniteDomainImpl::findSize(void) const {
  return max_elem - min_elem + 1;
}

inline
OZ_Boolean OZ_FiniteDomainImpl::isSingleInterval(void) const {
  return size == (max_elem - min_elem + 1);
}

inline
FDBitVector * OZ_FiniteDomainImpl::provideBitVector(void) const
{
  FDBitVector * bv = get_bv();

  return bv == NULL ? new FDBitVector : bv;
}

inline
FDBitVector * OZ_FiniteDomainImpl::asBitVector(void) const
{
  descr_type type = getType();
  if (type == bv_descr) {
    return get_bv();
  } else if (type == fd_descr) {
    FDBitVector * bv = provideBitVector();
    if (min_elem > fd_bv_max_elem)
      bv->setEmpty();
    else
      bv->setFromTo(min_elem, min(fd_bv_max_elem, max_elem));
    return bv;
  } else {
    FDBitVector * bv = new FDBitVector;
    FDIntervals &iv = *get_iv();
    bv->setEmpty();
    for (int i = 0; i < iv.high && iv.i_arr[i].left <= fd_bv_max_elem; i++) 
      bv->addFromTo(iv.i_arr[i].left, min(iv.i_arr[i].right, fd_bv_max_elem));
    return bv;
  }
}

inline
FDIntervals * OZ_FiniteDomainImpl::provideIntervals(int max_index) const
{
  FDIntervals * iv = get_iv();
  if (max_index > fd_iv_max_high) 
    return new (max_index) FDIntervals(max_index);
  else if (iv == NULL)
    return new FDIntervals(max_index);
  else
    iv->high = max_index;
  
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

inline
OZ_Boolean OZ_FiniteDomainImpl::isConsistent(void) const {
  if (size == 0) return OZ_TRUE;
  descr_type type = getType();
  if (type == fd_descr)
    return findSize() == size;
  else if (type == bv_descr)
    return get_bv()->findSize() == size;
  else
    return get_iv()->findSize() == size;
}
 
inline
OZ_Boolean OZ_FiniteDomainImpl::contains(int i) const
{
  if (size == 0) {
    return OZ_FALSE;
  } else {
    descr_type type = getType();
    if (type == fd_descr)
      return (min_elem <= i && i <= max_elem);
    else if (type == bv_descr)
      return get_bv()->contains(i);
    else
      return get_iv()->contains(i);
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
OZ_Boolean OZ_FiniteDomainImpl::operator == (const int v) const
{
  return (size == 1) && (min_elem == v);
}

inline
OZ_Boolean OZ_FiniteDomainImpl::operator != (const int v) const
{
  return (size != 1) || (min_elem != v);
}

inline
OZ_Boolean OZ_FiniteDomainImpl::operator == (const OZ_FDState state) const
{
  if (state == fd_singleton) {
    return size == 1;
  } else if (state == fd_bool) {
    return size == 2 && min_elem == 0 && max_elem == 1;
  } else {
    Assert(state == fd_empty);
    return size == 0;
  }
}

inline
OZ_Boolean OZ_FiniteDomainImpl::operator != (const OZ_FDState state) const
{
  if (state == fd_singleton) {
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
      FDBitVector * item = new FDBitVector;
      *item = *fd.get_bv();
      setType(item);
    } else {
      int max_index = fd.get_iv()->high;
      FDIntervals * item = (max_index > fd_iv_max_high)
	? new (max_index) FDIntervals(max_index)
	: new FDIntervals(max_index);
      *item = *fd.get_iv();
      setType(item);
    }
  }
  return *this;
}

inline
OZ_FiniteDomainImpl::OZ_FiniteDomainImpl(const OZ_FiniteDomainImpl &fd) {
  *this = fd;
}

inline
void OZ_FiniteDomainImpl::dispose(void) {
  switch (getType()) {
  case iv_descr: get_iv()->dispose(); return;
  case bv_descr: get_bv()->dispose(); return;
  default: return;
  }
}

unsigned OZ_FiniteDomainImpl::getDescrSize() {
  switch (getType()) {
  case iv_descr: 
    return sizeof(FDIntervals) + 2 * (get_iv()->getHigh() - fd_iv_max_high) * sizeof(int);
    case bv_descr: return sizeof(FDBitVector);
  default: return 0;
  }
}

inline
int OZ_FiniteDomainImpl::initBool(void)
{
  setType(fd_descr, NULL);
  min_elem = 0;
  max_elem = 1;
  return size = 2;
}

inline
int OZ_FiniteDomainImpl::init(int l, int r)
{
  l = max(l, fd_inf);
  r = min(r, fd_sup);

  setType(fd_descr, NULL);

  if (l > r) return size = 0;
  
  min_elem = l;
  max_elem = r;
  return size = findSize();
}  

inline
int OZ_FiniteDomainImpl::initFull(void)
{
  setType(fd_descr, NULL);
  min_elem = fd_inf;
  max_elem = fd_sup;
  return size = fd_full_size;
}  

inline
int OZ_FiniteDomainImpl::initEmpty(void)
{
  setType(fd_descr, NULL);
  return size = 0;
}

inline
int OZ_FiniteDomainImpl::initSingleton(int n)
{
  if (n < fd_inf || fd_sup < n)
    return initEmpty();
  setType(fd_descr, NULL);
  min_elem = max_elem = n;
  return size = 1;
}

extern int static_int_a[MAXFDBIARGS], static_int_b[MAXFDBIARGS];

/* gcc-2.6.3 on solaris has problems ...*/
int intcompare(const void * ii, const void  * jj) {
  int **i = (int **) ii;
  int **j = (int **) jj;
  return(**i - **j);
}

inline
int OZ_FiniteDomainImpl::simplify(int list_len, int * list_left, int * list_right)
{
  // producing list of sorted pointers in terms of this list 
  int i;
  for (i = list_len; i--; ) {
    fd_iv_left_sort[i] = list_left + i;
  }
  qsort(fd_iv_left_sort, list_len, sizeof(int **), intcompare);
  for (i = 0; i < list_len; i++) {
    fd_iv_right_sort[i] = list_right + (fd_iv_left_sort[i]-list_left);
  }
  
  // finding adjacent and overlapping intervals
  int len, p;
  for (len = 0, p = 0; p < list_len; len += 1) {
    if (p > len) {
      *fd_iv_left_sort[len] = *fd_iv_left_sort[p];
      *fd_iv_right_sort[len] = *fd_iv_right_sort[p];
    }
    while (++p < list_len && *fd_iv_right_sort[len] >= *fd_iv_left_sort[p]-1)
      if (*fd_iv_right_sort[p] > *fd_iv_right_sort[len])
	*fd_iv_right_sort[len] = *fd_iv_right_sort[p];
  }

  // writing sorted and collapsed intervals back to list_{left,right}
  static int aux[MAXFDBIARGS];

  for (i = 0; i < len; i++) aux[i] = *fd_iv_left_sort[i];
  for (i = 0; i < len; i++) list_left[i] = aux[i];
  
  for (i = 0; i < len; i++) aux[i] = *fd_iv_right_sort[i];
  for (i = 0; i < len; i++) list_right[i] = aux[i];
  
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
      FDBitVector * bv = provideBitVector();
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
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " = ~ ");
  
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
      } else if (max_elem == fd_sup) {
	y.max_elem = min_elem - 1;
	y.min_elem = 0;
	y.size = y.findSize();
      } else {
	FDIntervals * iv = newIntervals(2);
	iv->init(fd_inf, min_elem - 1, max_elem + 1, fd_sup);
	y.size = iv->findSize();
	y.min_elem = 0;
	y.max_elem = fd_sup;
	y.setType(iv);
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
      if (y.isSingleInterval()) y.setType(fd_descr);
    }
  }

  AssertFD(y.isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << y << endl);
  
  return y;
}

int OZ_FiniteDomainImpl::init(OZ_Term d)
{
  DEREF(d, d_ptr, d_tag);
  
  if (isSTuple(d) && tagged2SRecord(d)->getWidth() == 1) {
    init((*tagged2SRecord(d))[0]);
    *this = ~ *this;
    return size;
  } else if (isSmallInt(d_tag)) {
    return initSingleton(OZ_intToC(d));
  } else if (AtomSup == d) {
    return initSingleton(fd_sup);
  } else if (isSTuple(d)) {
    SRecord &t = *tagged2SRecord(d);
    OZ_Term t0 = deref(t[0]), t1 = deref(t[1]);
    return init(AtomSup == t0 ? fd_sup : OZ_intToC(t0),
		AtomSup == t1 ? fd_sup : OZ_intToC(t1));
  } else if (AtomBool == d) {
    return init(0, 1);
  } else if (isNil(d)) {
    return initEmpty();
  } else if (isLTuple(d_tag)) {
    int * left_arr = static_int_a, * right_arr = static_int_b;
    int min_arr = fd_sup, max_arr = 0;
    
    int len_arr;
    for (len_arr = 0; isLTuple(d) && len_arr < MAXFDBIARGS; ) {
      LTuple &list = *tagged2LTuple(d);
      OZ_Term val = list.getHead();
      
      DEREF(val, valptr, valtag);
      
      if (isSmallInt(valtag)) {
	int v = OZ_intToC(val);
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
      } else if (isSTuple(val)) {
	SRecord &t = *tagged2SRecord(val);
	OZ_Term t0 = deref(t[0]), t1 = deref(t[1]);

	int l = max(0, AtomSup == t0 ? fd_sup : OZ_intToC(t0));
	int r = min(fd_sup, AtomSup == t1 ? fd_sup : OZ_intToC(t1));

	if (l > r) goto for_loop;
	
	left_arr[len_arr] = l;
	right_arr[len_arr] = r;
	
	min_arr = min(min_arr, left_arr[len_arr]);
	max_arr = max(max_arr, right_arr[len_arr]);

	len_arr ++;
      } else {
	error("Unexpected case when creating finite domain.");
      }
    for_loop:
      d = deref(list.getTail());
    } // for
    if (len_arr >= MAXFDBIARGS) {
      warning("OZ_FiniteDomainImpl::init: "
	      "Probably elements of description are ignored");
    }
    return initList(len_arr, left_arr, right_arr, min_arr, max_arr);
  }
 error:
  error("Unexpected term in finite description list.");
  return -1;
}

inline
int OZ_FiniteDomainImpl::singl(void) const 
{
  AssertFD(size == 1);
  return min_elem;
}

// used for unification of fdvar with boolvar
int OZ_FiniteDomainImpl::intersectWithBool(void)
{
  if (contains(0))
    if (contains(1)) 
      return -1; // boolean
    else
      return 0; // 0
  else
    if (contains(1)) 
      return 1; // 1
    else
      return -2; // empty
}

inline
int OZ_FiniteDomainImpl::nextBiggerElem(int v) const
{
  descr_type type = getType();
  if (type == fd_descr) {
    if (v == max_elem) {
      error("no bigger element in domain");
      return -1;
    }
    return v + 1;
  } else if (type == bv_descr) {
    return get_bv()->nextBiggerElem(v, max_elem);
  } else {
    return get_iv()->nextBiggerElem(v, max_elem);
  }
} 

inline
OZ_Boolean OZ_FiniteDomainImpl::next(int i, int &n) const
{
  if (i <= min_elem) {
    n = min_elem;
    return OZ_FALSE;
  } else if (i >= max_elem) {
    n = max_elem;
    return OZ_FALSE;
  }
  
  descr_type type = getType();
  if (type == fd_descr) {
    n = i;
    return OZ_FALSE;
  } else if (type == bv_descr) {
    return get_bv()->next(i, n);
  } else {
    return get_iv()->next(i, n);
  }
}

inline
OZ_Term OZ_FiniteDomainImpl::getAsList(void) const
{
  if (size == 0) return AtomNil;
  
  descr_type type = getType();
  if (type == fd_descr) {
    return makeTaggedLTuple(new LTuple(mkTuple(min_elem, max_elem), AtomNil));
  } else if (type == bv_descr) {
    return get_bv()->getAsList();
  } else {
    return get_iv()->getAsList();
  }
}
  
inline
int OZ_FiniteDomainImpl::operator &= (const int i)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " &= " << i << " = ");
  if (contains(i)) {
    initSingleton(i);
    AssertFD(isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
    return 1;
  } else {
    initEmpty();
    AssertFD(isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
    return 0;
  }
}

inline
int OZ_FiniteDomainImpl::operator <= (const int leq)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this  << " <= " << leq << " = ");

  if (leq < min_elem) {
    AssertFD(isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << "{ - empty -}" << endl);
    return initEmpty();
  } else if (leq < max_elem) {
    descr_type type = getType();
    if (type == fd_descr) {
      max_elem = min(max_elem, leq);
      size = findSize();
    } else if (type == bv_descr) {
      if (leq <= fd_bv_max_elem) {
	FDBitVector * bv = get_bv();
	size = (*bv <= leq);
	if (size > 0) max_elem = bv->findMaxElem();
      }
    } else if (leq <= fd_sup) {
      FDIntervals * iv = get_iv();
      size = (*iv <= leq);
      if (size > 0) max_elem = iv->findMaxElem();
    }
  }
  if (isSingleInterval()) setType(fd_descr);
  AssertFD(isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
  return size;
}

inline
int OZ_FiniteDomainImpl::operator >= (const int geq)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this  << " >= " << geq << " = ");
  
  if (geq > max_elem) {
    AssertFD(isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << "{ - empty -}" << endl);
    return initEmpty();
  } else if (geq > min_elem) {
    descr_type type = getType();
    if (type == fd_descr) {
      min_elem = max(min_elem, geq);
      size = findSize();
    } else if (type == bv_descr) {
      FDBitVector * bv = get_bv();
      size = (geq > fd_bv_max_elem) ? initEmpty() : (*bv >= geq);
      if (size > 0) min_elem = bv->findMinElem();
    } else {
      FDIntervals * iv = get_iv();
      size = (geq > fd_sup) ? initEmpty() : (*iv >= geq);
      if (size > 0) min_elem = iv->findMinElem();
    }
  }
  if (isSingleInterval()) setType(fd_descr);
  AssertFD(isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
  return size;
}

inline
int OZ_FiniteDomainImpl::operator -= (const int take_out)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " -= " << take_out << " = ");
  if (contains(take_out)) {
    descr_type type = getType();
    if (type == fd_descr) {
      if (take_out == min_elem) {
	min_elem += 1;
      } else if (take_out == max_elem) {
	max_elem -= 1;
      } else {
	if (max_elem <= fd_bv_max_elem) {
	  FDBitVector * bv = provideBitVector();
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
    }
    size -= 1;
    if (isSingleInterval()) setType(fd_descr);
  }
  AssertFD(isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
  return size;
}

inline
int OZ_FiniteDomainImpl::operator -= (const OZ_FiniteDomainImpl &y)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " -= " << y << " = ");
  if (y != fd_empty) {
    descr_type x_type = getType(), y_type = y.getType();
    if (x_type == fd_descr) {
      if (y_type == fd_descr) {
	if (y.max_elem < min_elem || max_elem < y.min_elem) {
	  AssertFD(isConsistent());
	  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
	  return size;
	} else if (y.min_elem <= min_elem && max_elem <= y.max_elem) {
	  size = 0;
	} else if (min_elem < y.min_elem && y.max_elem < max_elem) {
	  if (max_elem <= fd_bv_max_elem) {
	    FDBitVector * bv = provideBitVector();
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
	FDBitVector * bv = asBitVector();
	size = (*bv -= *y.get_bv());
	min_elem = bv->findMinElem();
	max_elem = bv->findMaxElem();
	setType(bv);
      } else { // y_type == iv_descr
	FDIntervals * iv = newIntervals(1 + y.get_iv()->high);
	size = asIntervals()->subtract_iv(*iv, *y.get_iv());
	min_elem = iv->findMinElem();
	max_elem = iv->findMaxElem();
	setType(iv);
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
    }
    
    if (isSingleInterval()) setType(fd_descr);
  }
  AssertFD(isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
  return size;
}

inline
int OZ_FiniteDomainImpl::operator += (const int put_in)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " += " << put_in << " = ");
  
  if (put_in < fd_inf || fd_sup < put_in) return size;

  if (size == 0) {
    min_elem = max_elem = put_in;
    size = 1;
  } else if (!contains(put_in)) {
    descr_type type = getType();
    if (type == fd_descr) {
      if (put_in == min_elem - 1) {
	min_elem -= 1;
      } else if (put_in == max_elem + 1) {
	max_elem += 1;
      } else {
	if (max(max_elem, put_in) <= fd_bv_max_elem) {
	  FDBitVector * bv = asBitVector();
	  bv->setBit(put_in);
	  min_elem = bv->findMinElem();
	  max_elem = bv->findMaxElem();
	  setType(bv);
	} else {
	  FDIntervals * iv = provideIntervals(2);
	  if (put_in < min_elem) {
	    iv->init(put_in, put_in, min_elem, max_elem);
	  } else {
	    iv->init(min_elem, max_elem, put_in, put_in);
	  }
	  setType(iv);
	}
      }
    } else if (type == bv_descr) {
      if (put_in <= fd_bv_max_elem) {
	FDBitVector * bv = get_bv();
	bv->setBit(put_in);
	min_elem = bv->findMinElem();
	max_elem = bv->findMaxElem();
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
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);
  return size;
}

inline
OZ_FiniteDomainImpl OZ_FiniteDomainImpl::operator | (const OZ_FiniteDomainImpl &y) const
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " | " << y << " =  ");
  
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
    FDBitVector * z_v;
    z.setType(z_v = new FDBitVector);
    z.size = z_v->union_bv(*x_v, max_elem, *y_v, y.max_elem);
    z.min_elem = z_v->findMinElem();
    z.max_elem = z_v->findMaxElem();
  }
  if (z.isSingleInterval()) z.setType(fd_descr);

  AssertFD(z.isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << z << endl);
  
  return z;
}

inline
int OZ_FiniteDomainImpl::operator &= (const OZ_FiniteDomainImpl &y)
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " &= " << y << " = ");

  if (*this == fd_empty || y == fd_empty) {
    initEmpty();
    AssertFD(isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << "{ - empty -}" << endl);
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

    size = x_b->intersect_bv(*x_b, *y_b);
    min_elem = x_b->findMinElem();
    max_elem = x_b->findMaxElem();
    setType(x_b);
  }
  
  if (isSingleInterval()) setType(fd_descr);
  
  AssertFD(isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << *this << endl);

  return size;
}

inline
OZ_FiniteDomainImpl OZ_FiniteDomainImpl::operator & (const OZ_FiniteDomainImpl &y) const
{
  DEBUG_FD_IR(OZ_FALSE, cout << *this << " & " << y << " = ");
  
  OZ_FiniteDomainImpl z; z.initEmpty();

  if (*this == fd_empty || y == fd_empty) {
    AssertFD(z.isConsistent());
    DEBUG_FD_IR(OZ_FALSE, cout << "{ - empty -}" << endl);
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
    FDBitVector * z_b = new FDBitVector;
    
    z.size = x_b->intersect_bv(*z_b, *y_b);
    z.min_elem = z_b->findMinElem();
    z.max_elem = z_b->findMaxElem();
    z.setType(z_b);
  }
  
  if (z.isSingleInterval()) z.setType(fd_descr);

  AssertFD(z.isConsistent());
  DEBUG_FD_IR(OZ_FALSE, cout << z << endl);

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
#define CASTREF * (OZ_FiniteDomainImpl *) &
#define CASTTHIS (CASTPTR this)

void OZ_FiniteDomain::FiniteDomainInit(void * d)
{
  CASTTHIS->FiniteDomainInit(d);
}

OZ_FiniteDomain::OZ_FiniteDomain(void * d) 
{
  CASTTHIS->FiniteDomainInit(d);
}

OZ_FiniteDomain::OZ_FiniteDomain(OZ_Term t)
{
  CASTTHIS->FiniteDomainInit(NULL);
  CASTTHIS->init(t);
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
  default:
    error("Unexpected OZ_FDState.");
    break;
  }
}

OZ_FiniteDomain::OZ_FiniteDomain(const OZ_FiniteDomain &fd)
{
  CASTTHIS->operator =(CASTREF fd);
}

void OZ_FiniteDomain::dispose(void)
{
  CASTTHIS->dispose();
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

int OZ_FiniteDomain::init(OZ_Term t)
{
  return CASTTHIS->init(t);
}

int OZ_FiniteDomain::initSingleton(int s)
{
  return CASTTHIS->initSingleton(s);
}

int OZ_FiniteDomain::init(int l, int u)
{
  return CASTTHIS->init(l, u);
}

int OZ_FiniteDomain::initBool(void)
{
  return CASTTHIS->initBool();
}

int OZ_FiniteDomain::singl(void) const
{
  return CASTTHIS->singl();
}
   
OZ_Boolean OZ_FiniteDomain::isIn(int i) const
{
  return CASTTHIS->contains(i);
}

OZ_Term OZ_FiniteDomain::getAsList(void) const
{
  return CASTTHIS->getAsList();
}

OZ_Boolean OZ_FiniteDomain::next(int i, int &n) const
{
  return CASTTHIS->next(i, n);
}

int OZ_FiniteDomain::nextBiggerElem(int v) const
{
  return CASTTHIS->nextBiggerElem(v);
}

int OZ_FiniteDomain::constrainBool(void)
{
  return CASTTHIS->constrainBool();
}

OZ_FiniteDomain OZ_FiniteDomain::operator & (const OZ_FiniteDomain & y) const
{
  return CASTTHIS->operator & (CASTREF y);
}

OZ_FiniteDomain OZ_FiniteDomain::operator | (const OZ_FiniteDomain & y) const
{
  return CASTTHIS->operator | (CASTREF y);
}

OZ_FiniteDomain OZ_FiniteDomain::operator ~ (void) const
{
  return CASTTHIS->operator ~ ();
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
  return CASTTHIS->operator == (s);
}

OZ_Boolean OZ_FiniteDomain::operator != (const OZ_FDState s) const
{
  return CASTTHIS->operator != (s);
}

OZ_Boolean OZ_FiniteDomain::operator == (const int i) const
{
  return CASTTHIS->operator == (i);
}

OZ_Boolean OZ_FiniteDomain::operator != (const int i) const
{
  return CASTTHIS->operator != (i);
}

ostream &OZ_FiniteDomain::print(ostream &s) const
{
  CASTTHIS->print(s, 0);
  return s;
}

void OZ_FiniteDomain::copyExtension(void)
{
  CASTTHIS->copyExtensionInline();
}

