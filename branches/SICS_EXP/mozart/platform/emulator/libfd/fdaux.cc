/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include <stdarg.h>

#include "fdaux.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_init, 1)
{
  
#ifdef OZ_DEBUG
  oz_debugprint("*** DEBUG-FDLIB ***");
#elif OZ_PROFILE
  oz_debugprint("*** PROFILE-FDLIB ***");
#endif
  return OZ_unifyAtom(OZ_args[0], __DATE__ " (" __TIME__ ")");
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// convert vector to C++ arrays

int * vectorToInts(OZ_Term t, int &sz) 
{
  int * v;

  if (OZ_isLiteral(t)) {

    sz = 0;
    v = NULL;

  } else if (OZ_isCons(t)) {

    sz = OZ_length(t);
    v = OZ_hallocCInts(sz);
    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_intToC(OZ_head(t));

  } else if (OZ_isTuple(t)) {

    sz = OZ_width(t);
    v = OZ_hallocCInts(sz);
    for (int i = 0; i < sz; i += 1) 
      v[i] = OZ_intToC(OZ_getArg(t, i));

  } else {

    OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);
    sz = OZ_width(t);

    v = OZ_hallocCInts(sz);
    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_intToC(OZ_subtree(t, OZ_head(al)));

  } 

  return v;
}

// adds one field and initializes it with -1
int * vectorToInts1(OZ_Term t, int &sz) 
{
  int * v;

  if (OZ_isLiteral(t)) {

    sz = 1;
    v = OZ_hallocCInts(sz);

  } else if (OZ_isCons(t)) {

    sz = OZ_length(t) + 1;
    v = OZ_hallocCInts(sz);
    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_intToC(OZ_head(t));

  } else if (OZ_isTuple(t)) {

    sz = OZ_width(t) + 1;
    v = OZ_hallocCInts(sz);
    for (int i = 0; i < sz-1; i += 1) 
      v[i] = OZ_intToC(OZ_getArg(t, i));

  } else {
    
    OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);
    sz = OZ_width(t) + 1;

    v = OZ_hallocCInts(sz);
    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_intToC(OZ_subtree(t, OZ_head(al)));

  } 
  
  v[sz-1] = -1;
  return v;
}

OZ_Term * vectorToOzTerms(OZ_Term t, int &sz)
{
  OZ_Term * v;

  if (OZ_isLiteral(t)) {

    sz = 0; 
    v = NULL;

  } else if (OZ_isCons(t)) {

    sz = OZ_length(t);
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_head(t);

  } else if (OZ_isTuple(t)) {

    sz = OZ_width(t);
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; i < sz; i += 1) 
      v[i] = OZ_getArg(t, i);

  } else {

    OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);
    sz = OZ_width(t);

    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_subtree(t, OZ_head(al));

  } 

  return v;
}

OZ_Term * vectorToOzTerms(OZ_Term t, OZ_Term c, int &sz)
{
  OZ_Term * v;

  if (OZ_isLiteral(t)) {
    
    sz = 1; 
    v = OZ_hallocOzTerms(sz);
  
  } else if (OZ_isCons(t)) {

    sz = OZ_length(t) + 1;
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_head(t);

  } else if (OZ_isTuple(t)) {

    sz = OZ_width(t) + 1;
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; i < sz-1; i += 1) 
      v[i] = OZ_getArg(t, i);

  } else {

    OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);
    sz = OZ_width(t) + 1;

    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_subtree(t, OZ_head(al));

  } 

  v[sz - 1] = c;
  return v;
}
void vectorToOzTerms(OZ_Term t, OZ_Term * v)
{
  if (OZ_isLiteral(t)) {
    return;
  } if (OZ_isCons(t)) {

    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_head(t);

  } else if (OZ_isTuple(t)) {

    for (int i = 0, sz = OZ_width(t); i < sz; i += 1) 
      v[i] = OZ_getArg(t, i);

  } else {
    
    OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);

    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_subtree(t, OZ_head(al));

  } 
}

// returns true if no variable was found
void vectorToLinear(OZ_Term v, int &a, OZ_Term &x)
{
  OZ_Boolean no_var = OZ_TRUE;
  
  if (OZ_isCons(v)) {

    for ( ; OZ_isCons(v); v = OZ_tail(v)) {
      OZ_Term v_j = OZ_head(v);
      if (OZ_isInt(v_j)) {
	a *= OZ_intToC(v_j);
      } else {
	OZ_ASSERT(OZ_isVariable(v_j));
	x = v_j;
	no_var = OZ_FALSE;
      }
    }    

  } else if (OZ_isTuple(v)) {
    
    for (int j = OZ_width(v); j--; ) {
      OZ_Term v_j = OZ_getArg(v, j);
      if (OZ_isInt(v_j)) {
	a *= OZ_intToC(v_j);
      } else {
	OZ_ASSERT(OZ_isVariable(v_j));
	x = v_j;
	no_var = OZ_FALSE;
      }
    }

  } else {

    OZ_ASSERT(OZ_isRecord(v));

    for (OZ_Term al = OZ_arityList(v); OZ_isCons(al); al = OZ_tail(al)) {
      OZ_Term v_j = OZ_subtree(v, OZ_head(al));
      if (OZ_isInt(v_j)) {
	a *= OZ_intToC(v_j);
      } else {
	OZ_ASSERT(OZ_isVariable(v_j));
	x = v_j;
	no_var = OZ_FALSE;
      }
    }
  } 

  if (no_var) x = OZ_int(1);
}

//-----------------------------------------------------------------------------

extern FILE *cpi_fileout;

void oz_debugprint(char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  vfprintf(cpi_fileout,format,ap);
  va_end(ap);

  fprintf(cpi_fileout, "\n");
  fflush(cpi_fileout);
}
