/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FSET_HH__
#define __FSET_HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include <iostream.h>

#include "oz_cpi.hh"
#include "types.hh"

class FSetValue : public OZ_FSetValue {

friend class OZ_FSetImpl;

public:
  FSetValue(void) {}
  FSetValue(const OZ_FSetImpl &s) {
    init(s);
  }
  FSetValue(OZ_Term);

  void init(const OZ_FSetImpl &);

  ostream &print2stream(ostream &) const;

  OZPRINT;
  OZPRINTLONG;

  FSetValue * gc(void);

  OZ_Boolean unify(OZ_Term t);
  Bool operator == (const FSetValue&) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;
};

inline
ostream &operator << (ostream &ofile, const FSetValue &fs) {
  return fs.print2stream(ofile);
}

const int fset_sup = (32 * fset_high) - 1;

class OZ_FSetImpl : public OZ_FSetConstraint {
friend class FSetValue;
protected:
  void printGlb(ostream &) const;
  void printLub(ostream &) const;
  /* _card_min is -1 if the set is not valid */
  OZ_Boolean normalize(void);
public:
  OZ_FSetImpl(void) { init(); }
  OZ_FSetImpl(int, int, OZ_Term, OZ_Term);
  OZ_FSetImpl(OZ_Term, OZ_Term);
  OZ_FSetImpl(const FSetValue& s) { init(s); }

  void init(void);
  void init(const FSetValue&);
  void init(OZ_FSetState);

  OZ_FSetImpl unify (const OZ_FSetImpl &) const;
  OZ_Boolean unify (const FSetValue &) const;

  int getCardMin(void) const { return _card_min; }
  int getCardMax(void) const { return _card_max; }
  OZ_Boolean putCard(int, int);
  OZ_Boolean isValid(void) const { return _card_min != -1; }
  OZ_Boolean isValue(void) const {
    return (_card_min == _card_max) && (_card_min == _known_in);
  }
  OZ_Boolean isWeakerThan(const OZ_FSetImpl &) const;
  OZ_Boolean isFSetValue(void) const { 
    return _card_min == _card_max && _card_min == _known_in; 
  }
  ostream &print(ostream &) const;
  void printDebug(void) const;
  OZ_FSetImpl &operator =(const FSetValue &); 
  OZ_Boolean isIn(int) const;
  OZ_Boolean isNotIn(int) const;
  OZ_Boolean isEmpty(void) const;
  OZ_Boolean isFull(void) const;
  OZ_Boolean isSubsumedBy(const OZ_FSetImpl &) const;
  OZ_Term getKnownInList(void) const;
  OZ_Term getKnownNotInList(void) const;
  OZ_Term getUnknownList(void) const;
  OZ_Term getLubList(void) const;
  OZ_Term getCardTuple(void) const;

  OZ_FSetImpl operator - (void) const;
  OZ_Boolean operator += (int i); // assert i is in *this
  OZ_Boolean operator -= (int i); // assert i is in _not_ *this
  OZ_Boolean operator <<= (const OZ_FSetImpl &);
  OZ_FSetImpl operator & (const OZ_FSetImpl &) const;
  OZ_FSetImpl operator | (const OZ_FSetImpl &) const;
  OZ_Boolean operator <= (const OZ_FSetImpl &);
  OZ_Boolean operator >= (const OZ_FSetImpl &);
  OZ_Boolean operator != (const OZ_FSetImpl &);
  OZ_Boolean operator == (const OZ_FSetImpl &);
};

inline
ostream &operator << (ostream &ofile, const OZ_FSetImpl &fs) {
  return fs.print(ofile);
}

typedef OZ_FSetImpl OZ_FSet;


#endif // __FSET_HH__

// end of file
//-----------------------------------------------------------------------------
