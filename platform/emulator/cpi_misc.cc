/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"


//-----------------------------------------------------------------------------

#ifdef CPI_FILE_PRINT

ofstream * init_cpi_cout(char * n) {
  static ofstream _cpi_cout(n, ios::out);

  cerr << endl << "CPI debug output goes to '" << n << "'."
       << endl << flush;

  if (! _cpi_cout )
    cerr << endl << "Cannot open '" << n << "' for output."
         << endl << flush;

  return &_cpi_cout;
}

ofstream * cpi_cout = init_cpi_cout("/tmp/cpi_debug.out");

#else

ostream * cpi_cout = &cout;

#endif

//-----------------------------------------------------------------------------

OZ_Boolean OZ_isPosSmallInt(OZ_Term val)
{
  return isPosSmallInt(val);
}

#define OZMALLOC(T, S) (T *) freeListMalloc(S * sizeof(T))
#define OZDISPOSE(T, S, P) freeListDispose(P, S * sizeof(T))

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

void OZ_hfreeChars(char * is, int n)
{
  if (n) OZDISPOSE(char, n, is);
}

#define FDTAG               OZCONST
#define MAKETAGGEDINDEX(I)  makeTaggedRef(FDTAG,(int32) (I<<2))
#define GETINDEX(T)         (ToInt32(tagValueOfVerbatim(T))>>2);

static int _is_size = 1024;
static int * is = (int *) malloc(_is_size * sizeof(int));

int * OZ_findEqualVars(int sz, OZ_Term * ts)
{
#ifdef __GNUC__
  /* gcc supports dynamic array suzes */
  OZ_Term * _ts_ptr[sz], _ts[sz];
#else
#if defined(__WATCOMC__) || defined(_MSC_VER)
  OZ_Term ** _ts_ptr = (OZ_Term**) alloca(sizeof(OZ_Term*)*sz);
  OZ_Term  * _ts     = (OZ_Term*)  alloca(sizeof(OZ_Term) *sz);
#endif
#endif
  int i;

  if (sz > _is_size)
    is = (int *) realloc(is, sizeof(int) * (_is_size = sz));
  for (i = 0; i < sz; i += 1) {
    OZ_Term t = ts[i];
    DEREF(t, tptr, ttag);
    if (isSmallInt(ttag) || isLiteral(ttag)) {
      is[i] = -1;
    } else {
      if (ttag == FDTAG) {
        is[i] = GETINDEX(*tptr);
      } else {
        Assert(isAnyVar(ttag));
        _ts_ptr[i] = tptr;
        _ts[i] = t;
        is[i] = i;
        *tptr = MAKETAGGEDINDEX(i);
      }
    }
  }

  for (i = sz; i--; )
    if (is[i] == i) {
      *_ts_ptr[i] = _ts[i];
      Assert(OZ_isVariable(makeTaggedRef(_ts_ptr[i])));
    }

  return is;
}

static int _sgl_size = 1024;
static int * sgl = (int *) malloc(_sgl_size * sizeof(int));

int * OZ_findSingletons(int sz, OZ_Term * ts)
{
  int i;

  if (sz > _sgl_size)
    sgl = (int *) realloc(is, sizeof(int) * (_sgl_size = sz));
  for (i = 0; i < sz; i += 1) {
    OZ_Term t = ts[i];
    DEREF(t, tptr, ttag);
    if (isSmallInt(ttag) || isLiteral(ttag)) {
      sgl[i] = smallIntValue(t);
    } else {
      sgl[i] = -1;
    }
  }

  return sgl;
}

OZ_Boolean OZ_isEqualVars(OZ_Term v1, OZ_Term v2)
{
  DEREF(v1, vptr1, vtag1);
  DEREF(v2, vptr2, vtag2);
  return isAnyVar(vtag1) && (vptr1 == vptr2);
}

OZ_Return OZ_typeError(char * __typeString,
                       int pos,
                       char * comment)
{
  TypeError(pos, comment);
}

int OZ_getFDInf(void)
{
  return fd_inf;
}

int OZ_getFDSup(void)
{
  return fd_sup;
}

int OZ_vectorSize(OZ_Term t)
{
  if (OZ_isCons(t)) {
    return OZ_length(t);
  } else if (OZ_isTuple(t)) {
    return OZ_width(t);
  } else if (OZ_isRecord(t)) {
    return OZ_width(t);
  } else if (OZ_isLiteral(t)) {
    return 0;
  }
  return -1;
}

OZ_Term * OZ_getOzTermVector(OZ_Term t, OZ_Term * v)
{
  int i = 0;

  if (OZ_isLiteral(t)) {

    ;

  } if (OZ_isCons(t)) {

    for (; OZ_isCons(t); t = OZ_tail(t))
      v[i++] = OZ_head(t);

  } else if (OZ_isTuple(t)) {

    for (int sz = OZ_width(t); i < sz; i += 1)
      v[i] = OZ_getArg(t, i);

  } else if (OZ_isRecord(t)) {

    OZ_Term al = OZ_arityList(t);

    for (; OZ_isCons(al); al = OZ_tail(al))
      v[i++] = OZ_subtree(t, OZ_head(al));

  } else {
    OZ_warning("OZ_getOzTermVector: Unexpected term, expected vector.");
    return NULL;
  }
  return v + i;
}

int * OZ_getCIntVector(OZ_Term t, int * v)
{
  int i = 0;

  if (OZ_isLiteral(t)) {

    ;

  } if (OZ_isCons(t)) {

    for (; OZ_isCons(t); t = OZ_tail(t))
      v[i++] = OZ_intToC(OZ_head(t));

  } else if (OZ_isTuple(t)) {

    for (int sz = OZ_width(t); i < sz; i += 1)
      v[i] = OZ_intToC(OZ_getArg(t, i));

  } else if (OZ_isRecord(t)) {

    OZ_Term al = OZ_arityList(t);

    for (; OZ_isCons(al); al = OZ_tail(al))
      v[i++] = OZ_intToC(OZ_subtree(t, OZ_head(al)));

  } else {
    OZ_warning("OZ_getCIntVector: Unexpected term, expected vector.");
    return NULL;
  }
  return v + i;
}

// End of File
//-----------------------------------------------------------------------------
