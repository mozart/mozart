/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __INDEXSET_HH__
#define __INDEXSET_HH__


class ItStack {
private:
  int * _elems;
  int _top;
  int _max_top;
public:
  ItStack(int max_top, int elems[])
    : _elems(elems), _top(0), _max_top(max_top) {}
  void push(int e) { OZ_ASSERT(_top < _max_top); _elems[_top++] = e; }
  int pop(void) { OZ_ASSERT(_top > 0);return _elems[--_top]; }
  int isEmpty() { return !_top; }
};

inline int div32(int n) { return n >> 5; }
inline int mod32(int n) { return n & 0x1f; }
inline int word32(int n) { return mod32(n) ? div32(n) + 1 : div32(n); }

class IndexSet {
friend class IndexSets;

private:
  int _card;
  int _high;
  int * _elems;

  unsigned char * initNumOfBitsInHalfWord(void)
  {
    const unsigned int maxHalfWord = 0xffff;
    unsigned char * r = new unsigned char[maxHalfWord+1];
    OZ_ASSERT(r!=NULL);
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

  int findCard(void)
  {
    static unsigned char * numOfBitsInHalfWord = initNumOfBitsInHalfWord();
    int s, i;
    int high = getHigh();
    for (s = 0, i = high; i--; ) {
      s += numOfBitsInHalfWord[unsigned(_elems[i]) & 0xffff];
      s += numOfBitsInHalfWord[unsigned(_elems[i]) >> 16];
    }

    return s;
  }
  int updateCard(void) { return _card = findCard(); }
public:
  int * init(int high, int * elems)
  {
    _card = 0;
    _high = high;
    _elems = elems;

    for (int i = _high; i--; )
      _elems[i] = 0;

    return _elems + _high;
  }
  IndexSet(int high, int * elems) { init(high, elems); }

  void setIgnore(void) { _card = -1; }
  int  isIgnore(void) { return _card == -1; }

  int getCard(void) const { return _card; }
  int getHigh(void) const { return _high; }
  int * getElems(void) const { return _elems; }

  int isIn(int i)
  {
    OZ_ASSERT(-1 < i && i < 32 * _high);

    return _elems[div32(i)] & (1 << mod32(i));
  }
  int set(int i)
  {
    OZ_ASSERT(-1 < i && i < 32 * _high);

    if (! isIn(i)) {
      _card += 1;
      _elems[div32(i)] |= (1 << mod32(i));
    }
    OZ_ASSERT(findCard() == _card);
    return _card;
  }
  int reset(int i)
  {
    OZ_ASSERT(-1 < i && i < 32 * _high);

    if (isIn(i)) {
      _card -= 1;
      _elems[div32(i)] &= ~(1 << mod32(i));
    }
    OZ_ASSERT(findCard() == _card);
    return _card;
  }
  void copy(IndexSet &target)
  {
    target._high = _high;
    target._card = _card;

    if (! isIgnore()) {
      for (int i = _high; i--; )
        target._elems[i] = _elems[i];
      OZ_ASSERT(target.findCard() == target._card);
    }
  }
  int nextLargerElem(int i)
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
  void print(void)
  {
    printf("{ ");
    for (int k = nextLargerElem(-1); k != -1; k = nextLargerElem(k))
      printf("%d ", k);
    printf("}#%d", _card);

    fflush(stdout);

    OZ_ASSERT(findCard() == _card);
  }
  void printHex(void)
  {
    for (int k = 0; k < _high; k += 1)
      printf("0x%x ", _elems[k]);
  }
};


class IndexSets {
private:
  int _max_card_iset;
  int _nb_isets;
  IndexSet * _isets;

  void init(int nb_isets, int max_card_iset)
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
public:

  IndexSet &operator [](int i)
  {
    OZ_ASSERT(-1 < i && i < _nb_isets);

    return _isets[i];
  }

  static int sizeOf(int nb_isets, int max_card_iset) // nb of bytes (chars)
  {
    int size_per_iset = sizeof(int) * word32(max_card_iset) + sizeof(IndexSet);
    int size = sizeof(IndexSets) + nb_isets * size_per_iset;
    return size;
  }

  int getHigh(void) { return (*this)[0].getHigh(); }

  void unionAll(IndexSet &u)
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

  int resetAllBut(ItStack &st, IndexSet &aux, int k)
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
    aux.updateCard();
    aux.set(k);

    for (i = _nb_isets-1; i--; ) {
      IndexSet &tmp_i = (*this)[i];
      if (! tmp_i.isIgnore()) {
        int old_card = tmp_i.getCard();
        for (j = high; j--; )
          tmp_i._elems[j] &= aux._elems[j];
        if (! tmp_i.updateCard())
          return 0;
        if (old_card > 1 && tmp_i.getCard() == 1)
          st.push(i);
      }
    }
    return 1;
  }

  IndexSets * copy(void)
  {
    IndexSets * tmp = (IndexSets *) (void *)
      OZ_hallocChars(sizeOf(_nb_isets, _max_card_iset));
    tmp->init(_nb_isets, _max_card_iset);

    for (int i = _nb_isets; i--; )
      _isets[i].copy(tmp->_isets[i]);

    return tmp;
  }

  static IndexSets * create(int nb_isets, int max_card_iset)
  {
    IndexSets * tmp = (IndexSets *)(void *)
      OZ_hallocChars(sizeOf(nb_isets, max_card_iset));
    tmp->init(nb_isets, max_card_iset);
    return tmp;
  }
  void print(void)
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
};

#endif /* __INDEXSET_HH__ */

//-----------------------------------------------------------------------------
// eof
