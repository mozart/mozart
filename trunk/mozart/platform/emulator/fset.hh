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

#ifndef __FSET_HH__
#define __FSET_HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "mozart_cpi.hh"
#include "fdomn.hh"

//-----------------------------------------------------------------------------

class FSetValue : public OZ_FSetValue {

friend class FSetConstraint;

public:
  FSetValue(void) {}
  FSetValue(const FSetConstraint &s);
  FSetValue(OZ_Term);
  FSetValue(const int *, bool);
  FSetValue(OZ_FSetState s);
  FSetValue(const OZ_FiniteDomain &fd);

  void init(const FSetConstraint &);
  void init(const OZ_Term);
  void init(OZ_FSetState);
  void init(int, int);
  void init(const OZ_FiniteDomain &fd);

  ostream &print2stream(ostream &) const;

  void print(ostream &stream, int depth=10, int offset=0) const;
  void printDebug(void) const {print(cerr,10,0); cerr << endl; cerr.flush();}

  FSetValue * gCollect(void);
  FSetValue * sClone(void);
  void copyExtension(void);
  void disposeExtension(void);

  OZ_Boolean unify(OZ_Term t);
  Bool operator == (const FSetValue&) const;
  Bool operator <= (const FSetValue&) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;

  int getMinElem(void) const;
  int getMaxElem(void) const;
  int getNextLargerElem(int) const;
  int getNextSmallerElem(int) const;
  
  FSetValue operator & (const FSetValue &) const;
  FSetValue operator | (const FSetValue &) const;
  FSetValue operator - (const FSetValue &) const;
  FSetValue operator &= (const FSetValue &);
  FSetValue operator |= (const FSetValue &);
  FSetValue operator &= (const int);
  FSetValue operator += (const int);
  FSetValue operator -= (const int);
  FSetValue operator - (void) const;

  OZ_Boolean isIn(int) const;

  const int * getBV(void) const { 
    //#ifndef BIGFSET
    return _in; 
    //#else
    //error("FSetValue::getBV() called with big FSets\n");
    //return NULL;
    //#endif
  }
  // conversion functions
  void toNormal(void);
  void toExtended(void);
  bool maybeToNormal(void);

  void DP(const char *s) const;
};

inline
ostream &operator << (ostream &ofile, const FSetValue &fs) {
  return fs.print2stream(ofile);
}

const int fset_inf = 0;
#ifdef BIGFSET
const int fset_sup = 134217726;
#else
const int fset_sup = (32 * fset_high) - 1;
#endif

class FSetConstraint : public OZ_FSetConstraint {
friend class FSetValue;
protected:
  void printGlb(ostream &) const;
  void printLub(ostream &) const;
  /* _card_min is -1 if the set is not valid */
  OZ_Boolean normalize(void);
public:
  FSetConstraint(void);
  FSetConstraint(int, int, OZ_Term, OZ_Term);
  FSetConstraint(OZ_Term, OZ_Term);
  FSetConstraint(const FSetValue& s);

  FSetConstraint(const FSetConstraint &);
  FSetConstraint &operator = (const FSetConstraint &);

  void init(void);
  void init(const FSetValue &);
  void init(const FSetConstraint &);
  void init(OZ_FSetState);

  FSetConstraint * gCollect(void);
  FSetConstraint * sClone(void);
  void copyExtension(void);
  void disposeExtension(void);

  FSetConstraint unify(const FSetConstraint &) const;
  OZ_Boolean valid(const FSetValue &) const;

  int getCardMin(void) const { return _card_min; }
  int getCardMax(void) const { return _card_max; }
  OZ_Boolean putCard(int, int);
  OZ_Boolean isValid(void) const { return _card_min != -1; }
  OZ_Boolean isValue(void) const {
    return (_card_min == _card_max) && (_card_min == _known_in);
  }
  OZ_Boolean isWeakerThan(const FSetConstraint &) const;

  ostream &print(ostream &) const;
  void printDebug(void) const;
  FSetConstraint &operator =(const FSetValue &); 
  OZ_Boolean isIn(int) const;
  OZ_Boolean isNotIn(int) const;
  OZ_Boolean isEmpty(void) const;
  OZ_Boolean isFull(void) const;
  OZ_Boolean isSubsumedBy(const FSetConstraint &) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;
  OZ_Term getUnknownList(void) const;
  OZ_Term getLubList(void) const;
  OZ_Term getCardTuple(void) const;

  FSetValue getGlbSet(void) const;
  FSetValue getLubSet(void) const;
  FSetValue getUnknownSet(void) const;
  FSetValue getNotInSet(void) const;

  int getGlbCard(void) const;
  int getLubCard(void) const;
  int getNotInCard(void) const;
  int getUnknownCard(void) const;

  int getGlbMinElem(void) const;
  int getLubMinElem(void) const;
  int getNotInMinElem(void) const;
  int getUnknownMinElem(void) const;
  int getGlbMaxElem(void) const;
  int getLubMaxElem(void) const;
  int getNotInMaxElem(void) const;
  int getUnknownMaxElem(void) const;
  int getGlbNextSmallerElem(int) const;
  int getLubNextSmallerElem(int) const;
  int getNotInNextSmallerElem(int) const;
  int getUnknownNextSmallerElem(int) const;
  int getGlbNextLargerElem(int) const;
  int getLubNextLargerElem(int) const;
  int getNotInNextLargerElem(int) const;
  int getUnknownNextLargerElem(int) const;

  FSetConstraint operator - (void) const;
  OZ_Boolean operator += (int i); // assert i is in *this
  OZ_Boolean operator -= (int i); // assert i is in _not_ *this
  OZ_Boolean operator <<= (const FSetConstraint &);
  OZ_Boolean operator % (const FSetConstraint &);
  FSetConstraint operator & (const FSetConstraint &) const;
  FSetConstraint operator | (const FSetConstraint &) const;
  FSetConstraint operator - (const FSetConstraint &) const;
  OZ_Boolean operator <= (const FSetConstraint &);
  OZ_Boolean operator >= (const FSetConstraint &);
  OZ_Boolean operator != (const FSetConstraint &);
  OZ_Boolean operator == (const FSetConstraint &) const;

  OZ_Boolean le (const int);
  OZ_Boolean ge (const int);

  OZ_Boolean operator <= (const int);
  OZ_Boolean operator >= (const int);
  OZ_Boolean operator |= (const FSetValue &);
  OZ_Boolean operator &= (const FSetValue &);

  // conversion functions
  void toNormal(void);
  void toExtended(void);
  bool maybeToNormal(void);
  void DP(const char *s) const;
};

inline
ostream &operator << (ostream &ofile, const FSetConstraint &fs) {
  return fs.print(ofile);
}


class FSetIterator {
private:
  const OZ_FSetValue &_fset;
  int _elem;

public:
  FSetIterator(const OZ_FSetValue &fs) : _fset(fs) {}
  FSetIterator(const OZ_FSetValue &fs, int elem) : _elem(elem), _fset(fs) {}

  void init(int elem) { _elem = elem; }

  int resetToMin(void) {
    return _elem = _fset.getMinElem();
  }
  int resetToMax(void) {
    return _elem = _fset.getMaxElem();
  }

  int getNextLarger(void) {
    return _elem = _fset.getNextLargerElem(_elem);
  }
  int getNextSmaller(void) {
    return _elem = _fset.getNextSmallerElem(_elem);
  }
};

#endif // __FSET_HH__

// end of file
//-----------------------------------------------------------------------------
