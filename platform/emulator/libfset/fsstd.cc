/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fsstd.hh"

//-----------------------------------------------------------------------------

Propagator_S_VD::Propagator_S_VD(OZ_Term s, OZ_Term vd) 
  : _s(s)
{
  _vd_size = OZ_vectorSize(vd);
  _vd = OZ_hallocOzTerms(_vd_size);
  OZ_getOzTermVector(vd, _vd);
}

Propagator_S_VD::~Propagator_S_VD(void) 
{
  OZ_hfreeOzTerms(_vd, _vd_size);
}

void Propagator_S_VD::updateHeapRefs(OZ_Boolean)
{
  OZ_updateHeapTerm(_s);

  OZ_Term * new_vd = OZ_hallocOzTerms(_vd_size);
  
  for (int i = _vd_size; i--; ) {
    new_vd[i] = _vd[i];
    OZ_updateHeapTerm(new_vd[i]);
  }
  _vd = new_vd;
}

OZ_Term Propagator_S_VD::getParameters(void) const
{
  TERMVECTOR2LIST(_vd, _vd_size, vd);
  RETURN_LIST2(_s, vd);
}

//-----------------------------------------------------------------------------

Propagator_VS::Propagator_VS(OZ_Term vs) 
{
  _vs_size = OZ_vectorSize(vs);
  _vs = OZ_hallocOzTerms(_vs_size);
  OZ_getOzTermVector(vs, _vs);
}

Propagator_VS::~Propagator_VS(void) 
{
  OZ_hfreeOzTerms(_vs, _vs_size);
}

void Propagator_VS::updateHeapRefs(OZ_Boolean)
{
  OZ_Term * new_vs = OZ_hallocOzTerms(_vs_size);
  
  for (int i = _vs_size; i--; ) {
    new_vs[i] = _vs[i];
    OZ_updateHeapTerm(new_vs[i]);
  }
  _vs = new_vs;
}

OZ_Term Propagator_VS::getParameters(void) const
{
  TERMVECTOR2LIST(_vs, _vs_size, vs);
  RETURN_LIST1(vs);
}

//-----------------------------------------------------------------------------

Propagator_VS_S::Propagator_VS_S(OZ_Term vs, OZ_Term s)
  : Propagator_VS(vs), _s(s)
{
}

void Propagator_VS_S::updateHeapRefs(OZ_Boolean dup)
{
  Propagator_VS::updateHeapRefs(dup);
  OZ_updateHeapTerm(_s);
}

OZ_Term Propagator_VS_S::getParameters(void) const
{
  TERMVECTOR2LIST(_vs, _vs_size, vs);
  RETURN_LIST2(vs,_s);
}

//-----------------------------------------------------------------------------
// eof
