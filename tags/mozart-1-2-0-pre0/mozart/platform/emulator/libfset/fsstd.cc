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

void Propagator_S_VD::gCollect(void)
{
  OZ_gCollectTerm(_s);

  _vd = OZ_gCollectAllocBlock(_vd_size, _vd);

}

void Propagator_S_VD::sClone(void)
{
  OZ_sCloneTerm(_s);

  _vd = OZ_sCloneAllocBlock(_vd_size, _vd);

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

void Propagator_VS::gCollect(void)
{
  _vs = OZ_gCollectAllocBlock(_vs_size, _vs);
}

void Propagator_VS::sClone(void)
{
  _vs = OZ_sCloneAllocBlock(_vs_size, _vs);
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

void Propagator_VS_S::gCollect(void)
{
  Propagator_VS::gCollect();
  OZ_gCollectTerm(_s);
}

void Propagator_VS_S::sClone(void)
{
  Propagator_VS::sClone();
  OZ_sCloneTerm(_s);
}

OZ_Term Propagator_VS_S::getParameters(void) const
{
  TERMVECTOR2LIST(_vs, _vs_size, vs);
  RETURN_LIST2(vs,_s);
}

//-----------------------------------------------------------------------------
// eof
