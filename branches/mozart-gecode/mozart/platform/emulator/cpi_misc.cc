/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#include "cpi.hh"
#include "fddebug.hh"

//-----------------------------------------------------------------------------

#ifdef CPI_FILE_PRINT

FILE *cpi_fileout = NULL;

ostream * init_cpi_cout(char * n) {
  cerr << endl << "CPI debug output goes to '" << n << "'."
       << endl << flush;

  cpi_fileout = fopen(n,"w");
  if (cpi_fileout==NULL )
    cerr << endl << "Cannot open '" << n << "' for output."
	 << endl << flush;

  return new ostream(cpi_fileout);
}

ostream *cpi_cout = init_cpi_cout("/tmp/cpi_debug.out");

#else

FILE *cpi_fileout = stdout;

ostream * cpi_cout = &cout;

#endif

//-----------------------------------------------------------------------------

OZ_Boolean OZ_isPosSmallInt(OZ_Term val)
{
  return isPosSmallFDInt(val);
}

// tmueller: (S * sizeof(T)) % 8 == 0
#define OZMALLOC(T, S) (T *) oz_freeListMalloc(S * sizeof(T))
#define OZDISPOSE(T, S, P) oz_freeListDisposeUnsafe(P, S * sizeof(T))

OZ_Term * OZ_hallocOzTerms(int n)
{
  return n == 0 ? (OZ_Term *) NULL : OZMALLOC(OZ_Term, n);
}

void OZ_hfreeOzTerms(OZ_Term * ts, int n)
{
  if (n) OZDISPOSE(OZ_Term, n, ts);
}

int *OZ_hallocCInts(int n)
{
  return n == 0 ? (int *) NULL : OZMALLOC(int, n);
}

void OZ_hfreeCInts(int * is, int n)
{
  if (n) OZDISPOSE(int, n, is);
}

char * OZ_hallocChars(int n)
{
  return n == 0 ? (char *) NULL : OZMALLOC(char, n);
}

char * OZ_copyChars(int n, char * frm) {
  if (n==0)
    return (char *) NULL;

  char * to = OZMALLOC(char, n);

  memcpy(to, frm, n);

  return to;
}

void OZ_hfreeChars(char * is, int n)
{
  if (n) OZDISPOSE(char, n, is);
}

static EnlargeableArray<int> is(1024);

struct OZ_TermTrail {
  OZ_Term   term;
  OZ_Term * ptr;
};

int * OZ_findEqualVars(int sz, OZ_Term * ts)
{
#ifdef __GNUC__
  /* gcc supports dynamic array sizes */
  OZ_TermTrail _term_trail[sz];
#else
  OZ_TermTrail * _term_trail = new OZ_TermTrail[sz];
#endif
  int te = 0;
  int i;

  is.request(sz);

  for (i = 0; i < sz; i += 1) {
    OZ_Term t = ts[i];
    DEREF(t, tptr);
    if (oz_isVar(t)) {
      Assert(oz_isVar(t));
      _term_trail[te].ptr  = tptr;
      _term_trail[te].term = t;
      te++;
      is[i] = i;
      *tptr = makeTaggedMarkInt(i);
    } else if (oz_isMark(t)) {
      is[i] = tagged2UnmarkedInt(*tptr);
    } else {
      Assert(oz_isSmallInt(t) || oz_isLiteral(t) || oz_isFSetValue(t));
      is[i] = -1;
    }
  }

  while (te--) {
    *(_term_trail[te].ptr) = _term_trail[te].term;
    Assert(OZ_isVariable(makeTaggedRef(_term_trail[te].ptr)));
  }

#ifndef __GNUC__
  delete _term_trail;
#endif

  return is;
}

OZ_Boolean OZ_hasEqualVars(int sz, OZ_Term * ts)
{
#ifdef __GNUC__
  /* gcc supports dynamic array sizes */
  OZ_TermTrail _term_trail[sz];
#else
  OZ_TermTrail * _term_trail = new OZ_TermTrail[sz];
#endif
  OZ_Boolean r = OZ_FALSE;
  int te = 0;
  int i;

  for (i = sz; i--; ) {
    OZ_Term t = ts[i];
    DEREF(t, tptr);
    if (oz_isVar(t)) {
      Assert(oz_isVar(t));
      _term_trail[te].ptr  = tptr;
      _term_trail[te].term = t;
      te++;
      *tptr = makeTaggedMarkInt(0);
    } else if (oz_isMark(t)) {
      r = OZ_TRUE;
    } else {
      Assert(oz_isSmallInt(t) || oz_isLiteral(t) || oz_isFSetValue(t));
    }
  }

  while (te--) {
    *(_term_trail[te].ptr) = _term_trail[te].term;
    Assert(OZ_isVariable(makeTaggedRef(_term_trail[te].ptr)));
  }

#ifndef __GNUC__
  delete _term_trail;
#endif

  return r;
}

static EnlargeableArray<int> sgl(1024);

int * OZ_findSingletons(int sz, OZ_Term * ts)
{
  sgl.request(sz);

  for (int i = sz; i--; ) {
    OZ_Term t = ts[i];
  retry:
    if (oz_isSmallInt(t)) {
      sgl[i] = tagged2SmallInt(t);
    } else if (oz_isRef(t)) {
      t = *tagged2Ref(t);
      goto retry;
    } else {
      sgl[i] = -1;
    }
  }
  
  return sgl;
}

OZ_Return OZ_typeErrorCPI(const char * typeString, int pos, const char * comment)
{
  return typeError(pos, comment, typeString);
}

int OZ_vectorSize(OZ_Term t) {
 retry:
  if (oz_isCons(t)) {
    return OZ_length(t);
  } else if (oz_isSRecord(t)) {
    return tagged2SRecord(t)->getWidth();
  } else if (oz_isLiteral(t)) {
    return 0;
  } else if (oz_isRef(t)) {
    t = *tagged2Ref(t);
    goto retry;
  }
  return -1;
}

OZ_Term * OZ_getOzTermVector(OZ_Term t, OZ_Term * v)
{
 retry:
  if (oz_isLiteral(t)) {
    
    return v;
    
  } else if (oz_isCons(t)) {
    
    int i = 0;
    
    do {
      v[i++] = oz_head(t);
      t = oz_deref(oz_tail(t));
    } while (oz_isCons(t));
    
    return v + i;
    
  } else if (oz_isSRecord(t)) {
    SRecord * sr = tagged2SRecord(t);
    if (sr->isTuple()) {
      
      int sz = sr->getWidth();
      
      for (int j = sz; j--; )
	v[j] = sr->getArg(j);

      return v + sz;
      
    } else {
      Assert(oz_isRecord(t));

      int i = 0;

      for (OZ_Term al = sr->getArityList(); oz_isCons(al); al = oz_tail(al))
	v[i++] = sr->getFeature(oz_head(al));

      return v + i;
    }
  } else if (oz_isRef(t)) {
    t = *tagged2Ref(t);
    goto retry;
  }
  OZ_warning("OZ_getOzTermVector: Unexpected term, expected vector.");
  return NULL;
}

int * OZ_getCIntVector(OZ_Term t, int * v)
{
 retry:
  if (oz_isLiteral(t)) {
    return v;
  } if (oz_isCons(t)) {
    int i = 0;
    do {
      v[i++] = tagged2SmallInt(oz_deref((oz_head(t))));
      t = oz_deref(oz_tail(t));
    } while (oz_isCons(t));
    return v + i;
  } else if (oz_isSRecord(t)) {
    SRecord * sr = tagged2SRecord(t);
    if (sr->isTuple()) {
      int sz = sr->getWidth();
      for (int j = sz; j--;)
	v[j] = tagged2SmallInt(oz_deref(sr->getArg(j)));
      return v + sz;
    } else {
      Assert(oz_isRecord(t));
      int i = 0;
      for (OZ_Term al = sr->getArityList(); oz_isCons(al); al = oz_tail(al))
	v[i++] = tagged2SmallInt(oz_deref(sr->getFeature(al)));
      return v + i;
    }
  } else if (oz_isRef(t)) {
    t = *tagged2Ref(t);
    goto retry;
  }
  OZ_warning("OZ_getCIntVector: Unexpected term, expected vector.");
  return NULL;
}

void * OZ_FSetValue::operator new(size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FSetValue::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FSetValue::operator new[](size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FSetValue::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif


void * OZ_FSetConstraint::operator new(size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FSetConstraint::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FSetConstraint::operator new[](size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FSetConstraint::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif

void * OZ_FiniteDomain::operator new(size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FiniteDomain::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FiniteDomain::operator new[](size_t s)
{
  return oz_freeListMalloc(s);
}

void OZ_FiniteDomain::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif

OZ_Term OZ_fsetValue(OZ_FSetValue * s)
{
  return makeTaggedFSetValue(s);
}

OZ_FSetValue * OZ_fsetValueToC(OZ_Term t)
{
  return  tagged2FSetValue(oz_deref(t));
}

OZ_Propagator::~OZ_Propagator(void) {}

OZ_Boolean OZ_Propagator::isMonotonic(void) const {
  return OZ_TRUE;
}

OZ_NonMonotonic::order_t OZ_Propagator::getOrder(void) const {
  return 0;
}

// End of File
//-----------------------------------------------------------------------------
