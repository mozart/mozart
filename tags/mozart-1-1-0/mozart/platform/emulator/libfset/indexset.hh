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

#ifndef __INDEXSET_HH__
#define __INDEXSET_HH__

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }
inline int word32(int n) { return mod32(n) ? div32(n) + 1 : div32(n); }

#ifdef PROFILE
#define inline
#endif

class ItStack {
private:
  int * _elems;
  int _top;
  int _max_top; 
public:
  ItStack(int max_top, int elems[]);
  void push(int e);
  int pop(void);
  int isEmpty();
  int isIn(int e);
  void setEmpty(void);
};

inline
ItStack::ItStack(int max_top, int elems[]) 
  : _elems(elems), _top(0), _max_top(max_top) {}

inline
void ItStack::push(int e) 
{ 
  OZ_ASSERT(_top < _max_top); 
  _elems[_top++] = e; 
}

inline
int ItStack::pop(void) { 
  OZ_ASSERT(_top > 0);
  return _elems[--_top]; 
}

inline
int ItStack::isEmpty() 
{ 
  return !_top; 
}

inline
int ItStack::isIn(int e) { 
  if (isEmpty()) 
    return 0;
  for (int i = _top; i--; )
    if (_elems[i] == e)
      return 1;
  return 0;
}

inline
void ItStack::setEmpty(void) { 
  _top = 0;
}

//-----------------------------------------------------------------------------

class IndexSet {
friend class IndexSets;

private:
  int _card;
  int _high;

#ifdef OZ_DEBUG
  struct int_t {
    int * __els;
    int &operator [] (int i) /*const*/ {
      OZ_ASSERT(0 <= i && i < *(((int *)this) - 1)); 
      // _high must be directly before `_elems'
      return __els[i];
    } 
    int * operator = (int * els) { return __els = els; }
    operator int * () const { return __els; }
  } _elems;
#else
  int * _elems;
#endif
  

  int findCard(void);

  int updateCard(void);
public: 
  int * init(int high, int * elems);
  IndexSet(int high, int * elems);

  void setIgnore(void);
  int  isIgnore(void);

  int getCard(void) const;
  int getHigh(void) const;
  int * getElems(void) const;

  int isIn(int i);
  int set(int i);
  int reset(int i);
  void copy(IndexSet &target);
  int nextLargerElem(int i);
  int smallestElem(void);
  void print(void);
  void printHex(void);
};

inline
int IndexSet::getHigh(void) const 
{ 
  return _high; 
}

extern int get_num_of_bits_outline(const int m, const int * ia);

inline
int IndexSet::findCard(void) {
#ifdef OZ_DEBUG
  return get_num_of_bits_outline(getHigh(), (int *) _elems);
#else
  return get_num_of_bits_outline(getHigh(), _elems);
#endif
}

inline
int IndexSet::updateCard(void) 
{ 
  return _card = findCard(); 
}

inline
int * IndexSet::init(int high, int * elems) 
{
  _card = 0;
  _high = high;
  _elems = elems;
  
  for (int i = _high; i--; )
    _elems[i] = 0;
  
  return _elems + _high;
}

inline
IndexSet::IndexSet(int high, int * elems) 
{ 
  init(high, elems); 
}

inline
void IndexSet::setIgnore(void) 
{ 
  _card = -1; 
}

inline
int IndexSet::isIgnore(void) 
{ 
  return _card == -1; 
}

inline
int IndexSet::getCard(void) const 
{ 
  return _card; 
}

inline
int * IndexSet::getElems(void) const 
{ 
  return _elems; 
}

inline
int IndexSet::isIn(int i) 
{ 
  OZ_ASSERT(-1 < i && i < 32 * _high);
  
  return _elems[div32(i)] & (1 << mod32(i)); 
}

inline
int IndexSet::set(int i) 
{ 
  OZ_ASSERT(-1 < i && i < 32 * _high);
  
  if (! isIn(i)) {
    _card += 1;
    _elems[div32(i)] |= (1 << mod32(i)); 
  }
  OZ_ASSERT(findCard() == _card);
  return _card;
}

inline
int IndexSet::reset(int i) 
{ 
  OZ_ASSERT(-1 < i && i < 32 * _high);
  
  if (isIn(i)) {
    _card -= 1;
    _elems[div32(i)] &= ~(1 << mod32(i)); 
  }
  OZ_ASSERT(findCard() == _card);
  return _card;
}

inline
void IndexSet::copy(IndexSet &target)
{
  target._high = _high;
  target._card = _card;
  
  if (! isIgnore()) {
    for (int i = _high; i--; )
      target._elems[i] = _elems[i];
    OZ_ASSERT(target.findCard() == target._card);
  }
}

inline
int IndexSet::nextLargerElem(int i)
{
  i += 1;
  if (i < 0 || i >= 32 * _high) return -1;
  if (isIn(i)) return i;
  
  // is there any other bit set in the same word?
  int word = div32(i), bit = mod32(i) + 1;
  unsigned w = (unsigned) _elems[word];
  w = bit < 32 ? w >> bit : 0;
  if (w) {
    // search the same word
    i += 1;
    for (; ! (w & 0x1); i += 1, w >>= 1);
    return i;
  } else { // find the next word 
    i = 32 * (word + 1);
    int j;
    for (j = word + 1; j < _high && !_elems[j]; j += 1, i += 32);
    if (j < _high) {
      unsigned w = (unsigned) _elems[j];
      for (; ! (w & 0x1); i += 1, w >>= 1);
      return i;
    }
  }
  return -1;
}

inline
int IndexSet::smallestElem(void)
{
  if (!_card)
    return -1;

  int j = 0;
  int r = 0;
  for (; !_elems[j]; j += 1, r += 32);
  unsigned w = (unsigned) _elems[j];
  for (; ! (w & 0x1); r += 1, w >>= 1);
  return r;  
}

inline
void IndexSet::print(void) 
{
  printf("{ ");
  for (int k = smallestElem(); k != -1; k = nextLargerElem(k))
    printf("%d ", k);
  printf("}#%d", _card);
  
  fflush(stdout);
  
  OZ_ASSERT(findCard() == _card);
}

inline
void IndexSet::printHex(void) 
{
  for (int k = 0; k < _high; k += 1)
    printf("0x%x ", _elems[k]);
}
//-----------------------------------------------------------------------------

class IndexSets {
private:
  int _max_card_iset; 
  int _nb_isets;
  IndexSet * _isets;

  void init(int nb_isets, int max_card_iset);
public:
  IndexSet &operator [](int i);

  static int sizeOf(int nb_isets, int max_card_iset) {
    int size_per_iset = sizeof(int) * word32(max_card_iset) + sizeof(IndexSet);
    int size = sizeof(IndexSets) + nb_isets * size_per_iset;
    return size;
  }
  
  int sizeOfMe(void) { return sizeOf(_nb_isets, _max_card_iset); }
  
  int getHigh(void);

  void unionAll(IndexSet &u);

  int resetAllBut(ItStack &st, IndexSet &aux, int k);

  IndexSets * copy(void);
  IndexSets * copy(char *);

  static IndexSets * create(int nb_isets, int max_card_iset);

  void print(void);
};

inline
void IndexSets::init(int nb_isets, int max_card_iset)
{
  _nb_isets = nb_isets;
  _max_card_iset = max_card_iset;
  
  _isets = (IndexSet *) (this + 1);
  
  int _high = word32(max_card_iset);
  int * _elems = (int *) (_isets + _nb_isets);
  
  for (int i = 0; i < _nb_isets; i += 1)
    _elems = _isets[i].init(_high, _elems);
  
  OZ_ASSERT(((char *) _elems) == ((char *) this) + sizeOf(_nb_isets, _max_card_iset));
}

inline
IndexSet &IndexSets::operator [](int i) 
{
  OZ_ASSERT(-1 < i && i < _nb_isets);
  
  return _isets[i]; 
}

inline
int IndexSets::getHigh(void) 
{ 
  return (*this)[0].getHigh(); 
}

inline
void IndexSets::unionAll(IndexSet &u) 
{
  int i, j;
  int high = getHigh();
  
  for (j = high; j--; )
    u._elems[j] = 0;
  
  for (i = _nb_isets-1; i--; ) {
    int * elems = (*this)[i].getElems();
    for (j = high; j--; )
      u._elems[j] |= elems[j];
  }
}

inline
int IndexSets::resetAllBut(ItStack &st, IndexSet &aux, int k) 
{
  int i, j, high = getHigh();
  
  for (j = high; j--; )
    aux._elems[j] = 0;
  
  for (i = _nb_isets-1; i--; ) {
    IndexSet &tmp_i = (*this)[i];
    if (tmp_i.isIn(k)) {
      int * elems = tmp_i.getElems();
      for (j = high; j--; )
	aux._elems[j] |= elems[j];
    }
  }
  for (j = high; j--; )
    aux._elems[j] = ~aux._elems[j];

  OZ_DEBUGCODE(aux.updateCard());
  OZ_NONDEBUGCODE(aux._card = 0);  // card is irrelevant here
  aux.set(k);
  
  for (i = _nb_isets-1; i--; ) {
    IndexSet &tmp_i = (*this)[i];
    if (! tmp_i.isIgnore()) {
      int old_card = tmp_i.getCard();
      
      if (tmp_i.isIn(k)) {
	for (j = high; j--; )
	  tmp_i._elems[j] = 0;
	tmp_i._card = 0;
	tmp_i.set(k);
      } else {
	for (j = high; j--; )
	  tmp_i._elems[j] &= aux._elems[j];
	
	if (! tmp_i.updateCard())
	  return 0;

	if (old_card > 1 && tmp_i.getCard() == 1)
	  st.push(i);
      }	
    }
  }
  return 1;
}

inline
IndexSets * IndexSets::copy(char * mem) 
{
  IndexSets * tmp = (IndexSets *) (void *) mem;

  tmp->init(_nb_isets, _max_card_iset);
  
  for (int i = _nb_isets; i--; )
    _isets[i].copy(tmp->_isets[i]);
  
  return tmp;
}

inline
IndexSets * IndexSets::copy(void) 
{
  IndexSets * tmp = (IndexSets *) (void *)
    OZ_hallocChars(sizeOf(_nb_isets, _max_card_iset));
  tmp->init(_nb_isets, _max_card_iset);
  
  for (int i = _nb_isets; i--; )
    _isets[i].copy(tmp->_isets[i]);
  
  return tmp;
}

inline
IndexSets * IndexSets::create(int nb_isets, int max_card_iset) 
{
  IndexSets * tmp = (IndexSets *)(void *) 
    OZ_hallocChars(sizeOf(nb_isets, max_card_iset));
  tmp->init(nb_isets, max_card_iset);
  return tmp;
}

inline
void IndexSets::print(void) 
{
  for (int i = 0; i < _nb_isets-1; i += 1) {
    if (! (*this)[i].isIgnore()) {
      printf("[%d]=", i);
      (*this)[i].print();
      printf("\n");
      fflush(stdout);
    }
  }
}

#endif /* __INDEXSET_HH__ */

//-----------------------------------------------------------------------------
// eof

