/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifdef FSETVAR

#ifndef __FSET_HH__
#define __FSET_HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include <iostream.h>

#include "oz_cpi.hh"

const int fset_high = 2;

class FSetValue {
friend class OZ_FSetImpl;
private:
  int _card;
  int _in[fset_high];
public:
  FSetValue(void) {}
  FSetValue(OZ_FSetImpl &);
  FSetValue(OZ_Term);

  ostream &print(ostream &) const;

  FSetValue * gc(void);

  OZ_Boolean unify(OZ_Term t);
  OZ_Boolean operator == (const FSetValue &) const;
};

inline
ostream &operator << (ostream &ofile, const FSetValue &fs) {
  return fs.print(ofile);
}

const int fset_sup = (32 * fset_high) - 1;

class OZ_FSetImpl {
friend class FSetValue;
private:
  int _card_min, _card_max; /* _card_min is -1 if the set is not valid */
  int _known_not_in, _known_in;
  int _in[fset_high], _not_in[fset_high];

  void printGlb(ostream &) const;
  void printLub(ostream &) const;
public:
  OZ_FSetImpl(void) {};
  OZ_FSetImpl(int, int, OZ_Term, OZ_Term);
  OZ_FSetImpl(OZ_Term, OZ_Term);

  OZ_FSetImpl unify (const OZ_FSetImpl &) const;
  OZ_Boolean unify (const FSetValue &) const;

  int getCardMin(void) const { return _card_min; }
  int getCardMax(void) const { return _card_max; }
  OZ_Boolean isValidSet(void) { return _card_min != -1; }
  OZ_Boolean isWeakerThan(const OZ_FSetImpl &) const;
  OZ_Boolean isFSetValue(void) { 
    return _card_min == _card_max && _card_min == _known_in; 
  }
  ostream &print(ostream &) const;
};

inline
ostream &operator << (ostream &ofile, const OZ_FSetImpl &fs) {
  return fs.print(ofile);
}

typedef OZ_FSetImpl OZ_FSet;


#endif // __FSET_HH__

#endif /* FSETVAR */
