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

int * OZ_findEqualVars(int sz, OZ_Term * ts)
{
#ifdef __GNUC__
  /* gcc supports dynamic array sizes */
  OZ_Term * _ts_ptr[sz], _ts[sz];
#else
  OZ_Term ** _ts_ptr = new OZ_Term*[sz];
  OZ_Term  * _ts     = new OZ_Term[sz];
#endif
  int i;

  is.request(sz);

  for (i = 0; i < sz; i += 1) {
    OZ_Term t = ts[i];
    DEREF(t, tptr);
    if (oz_isSmallInt(t) || oz_isLiteral(t) || oz_isFSetValue(t)) {
      is[i] = -1;
    } else {
      if (oz_isMark(t)) {
	is[i] = tagged2UnmarkedInt(*tptr);
      } else {
	Assert(oz_isVar(t));
	_ts_ptr[i] = tptr;
	_ts[i] = t;
	is[i] = i;
	*tptr = makeTaggedMarkInt(i);
      }
    }
  }

  for (i = sz; i--; )
    if (is[i] == i) {
      *_ts_ptr[i] = _ts[i];
      Assert(OZ_isVariable(makeTaggedRef(_ts_ptr[i])));
    }

#ifndef __GNUC__
  delete _ts_ptr;
  delete _ts;
#endif

  return is;
}

static EnlargeableArray<int> sgl(1024);

int * OZ_findSingletons(int sz, OZ_Term * ts)
{
  int i;

  sgl.request(sz);

  for (i = 0; i < sz; i += 1) {
    OZ_Term t = ts[i];
    DEREF(t, tptr);
    if (oz_isSmallInt(t) || oz_isLiteral(t)) { // mm2
      sgl[i] = tagged2SmallInt(t);
    } else {
      sgl[i] = -1;
    }
  }

  return sgl;
}

OZ_Boolean OZ_isEqualVars(OZ_Term v1, OZ_Term v2)
{
  DEREF(v1, vptr1);
  DEREF(v2, vptr2);
  return oz_isVar(v1) && (vptr1 == vptr2);
}

OZ_Return OZ_typeErrorCPI(char * typeString, int pos, char * comment)
{
  return typeError(pos, comment, typeString);
}

int OZ_getFDInf(void)
{
  return fd_inf;
}

int OZ_getFDSup(void)
{
  return fd_sup;
}

int OZ_vectorSize(OZ_Term t) {
  t = oz_deref(t);
  if (oz_isCons(t)) {
    return OZ_length(t);
  } else if (oz_isSRecord(t)) {
    return tagged2SRecord(t)->getWidth();
  } else if (oz_isLiteral(t)) {
    return 0;
  }
  return -1;
}

OZ_Term * OZ_getOzTermVector(OZ_Term t, OZ_Term * v)
{
  int i = 0;

  t=oz_deref(t);

  if (oz_isLiteral(t)) {

    ;

  } else if (oz_isCons(t)) {

    do {
      v[i++] = oz_head(t);
      t = oz_deref(oz_tail(t));
    } while (oz_isCons(t));

  } else if (oz_isTuple(t)) {

    for (int sz = tagged2SRecord(t)->getWidth(); i < sz; i += 1)
      v[i] = tagged2SRecord(t)->getArg(i);

  } else if (oz_isRecord(t)) {

    OZ_Term al = OZ_arityList(t);

    for (; oz_isCons(al); al = oz_tail(al))
      v[i++] = tagged2SRecord(t)->getFeature(oz_head(al));

  } else {
    OZ_warning("OZ_getOzTermVector: Unexpected term, expected vector.");
    return NULL;
  }
  return v + i;
}

int * OZ_getCIntVector(OZ_Term t, int * v)
{
  int i = 0;

  t = oz_deref(t);

  if (oz_isLiteral(t)) {

    ;

  } if (oz_isCons(t)) {

    do {
      v[i++] = tagged2SmallInt(oz_deref((oz_head(t))));
      t = oz_deref(oz_tail(t));
    } while (oz_isCons(t));

  } else if (oz_isTuple(t)) {

    for (int sz = tagged2SRecord(t)->getWidth(); i < sz; i += 1)
      v[i] = tagged2SmallInt(oz_deref((tagged2SRecord(t)->getArg(i))));

  } else if (oz_isRecord(t)) {

    OZ_Term al = OZ_arityList(t);

    for (; oz_isCons(al); al = oz_tail(al))
      v[i++] = tagged2SmallInt(oz_deref((tagged2SRecord(t)->getFeature(al))));

  } else {
    OZ_warning("OZ_getCIntVector: Unexpected term, expected vector.");
    return NULL;
  }
  return v + i;
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
