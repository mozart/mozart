/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "intsets.hh"

// FSP_MIN WAS WRITTEN BY TOBIAS AS AN EXAMPLE

OZ_C_proc_begin(fsp_min, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);

  return pe.impose(new FSetsMinPropagator(OZ_args[0],
                                          OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetsMinPropagator::header = fsp_min;

OZ_Return FSetsMinPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // This is a naive and quick-and-dirty implementation but since your
  // current problem is the number of propagators, it will suffice. A
  // better solution needs support from the class representing the
  // finite set intervals. I will do this later. Tobias.

  // DENYS: d < 32*fset_high
  FailOnEmpty(*d <= (32*fset_high - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, 32*fset_high));

  {
    for (int i = 0; i < 32*fset_high; i += 1) {
      // i < min(d) ==> i not in s
      if (i < d->getMinElem())
        FailOnInvalid(*s -= i);

      // i in s ==> min(d) <= i
      if (s->isIn(i)) {
        FailOnEmpty(*d <= i);
      }
      // DENYS: i not in s ==> d=/=i
      else if (s->isNotIn(i)) {
        FailOnEmpty(*d -= i);
      }
    }
  }

  // d is in s
  {
    int i = d->getSingleElem();
    if (i != -1)
      FailOnInvalid(*s += i);
  }

  return P.leave1();

failure:
  return P.fail();
}

// -------------------------------------------------------------------
// Max Propagator

OZ_C_proc_begin(fsp_max, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);

  int susp_count_dummy;
  OZ_EXPECT_SUSPEND(pe, 1, expectIntVarMinMax, susp_count_dummy);

  return pe.impose(new FSetsMaxPropagator(OZ_args[0],
                                          OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetsMaxPropagator::header = fsp_max;

OZ_Return FSetsMaxPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  OZ_FDIntVar d(_d);
  PropagatorController_S_D P(s, d);

  // d < 32*fset_high
  FailOnEmpty(*d <= (32*fset_high - 1));

  // card(s) > 0
  FailOnInvalid(s->putCard(1, 32*fset_high));

  {
    for (int i = 0; i < 32*fset_high; i += 1) {
      // i > max(d) ==> i not in s
      if (i > d->getMaxElem())
        FailOnInvalid(*s -= i);

      // i in s ==> max(d) >= i
      if (s->isIn(i)) {
        FailOnEmpty(*d >= i);
      }
      // i not in s ==> d=/=i
      else if (s->isNotIn(i)) {
        FailOnEmpty(*d -= i);
      }
    }
  }

  // d is in s
  {
    int i = d->getSingleElem();
    if (i != -1)
      FailOnInvalid(*s += i);
  }

  return P.leave1();

failure:
  return P.fail();
}

//--------------------------------------------------------------------
// Convex Propagator

OZ_C_proc_begin(fsp_convex, 1)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET);

  PropagatorExpect pe;
  OZ_EXPECT(pe,0,expectFSetVarAny);
  return pe.impose(new FSetsConvexPropagator(OZ_args[0]));
}
OZ_C_proc_end

OZ_CFun FSetsConvexPropagator::header = fsp_convex;

OZ_Return FSetsConvexPropagator::propagate(void)
{
  OZ_FSetVar s(_s);
  int minelem,maxelem;
  for (int i=0;i<32*fset_high;i++) {
    if (s->isIn(i)) {
      minelem=maxelem=i;
      for(i++;i<32*fset_high;i++)
        if (s->isIn(i)) maxelem=i;
      // all ints between the smallest and largest known elements
      // must also be in
      for(int k=minelem+1;k<maxelem;k++)
        FailOnInvalid(*s += k);
      // find a non-element below minelem: all ints below are out
      for(i=minelem-1;i>=0;i--)
        if (s->isNotIn(i)) {
          while (i--)
            FailOnInvalid(*s -= i);
          break;
        }
      // find a non-element above maxelem: all ints above are out
      for(i=maxelem+1;i<32*fset_high;i++)
        if (s->isNotIn(i)) {
          while (++i < 32*fset_high)
            FailOnInvalid(*s -= i);
          break;
        }
      break;
    }
  }
  return s.leave()?SLEEP:OZ_ENTAILED;
failure:
  s.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------
// match propagator

OZ_C_proc_begin(fsp_match, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMatchPropagator(OZ_args[0],
                                           OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetMatchPropagator::header = fsp_match;

#define MATCH_NOLOOP

OZ_Return FSetMatchPropagator::propagate(void)
{
  _OZ_DEBUGPRINTTHIS("\nin ");

  OZ_FSetVar s(_s);
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _vd_size);
  PropagatorController_S_VD P(s, _vd_size, vd);
  int max_fd = OZ_getFDInf();
  int min_fd = OZ_getFDSup();
  int i;

  for (i = _vd_size; i--; ) {
    vd[i].read(_vd[i]);
    min_fd = min(min_fd, vd[i]->getMinElem());
    max_fd = max(max_fd, vd[i]->getMaxElem());
  }

  if (_firsttime) {
    _OZ_DEBUGPRINT(("firsttime==1"));

    _firsttime = 0; // do it only once

    _k = 0; _l = _vd_size - 1;
    _last_min = s->getLubSet().getMinElem() - 1;
    _last_max = s->getLubSet().getMaxElem() + 1;

    // (1)
    //
    FailOnInvalid(s->putCard(_vd_size, _vd_size));
    _OZ_DEBUGPRINTTHIS("(1) ");
  }

#ifndef  MATCH_NOLOOP
  int old_size, new_size;
  FSetTouched st;

  for (old_size = 0, i = _k; i <= _l; i += 1)
    old_size += vd[i]->getSize();
#endif

loop:
  _OZ_DEBUGPRINT(("_k=%d _l=%d _last_min=%d _last_max=%d min_fd=%d max_fd=%d",
                  _k, _l , _last_min,  _last_max, min_fd,  max_fd));

#ifndef MATCH_NOLOOP
  st = s;
#endif

  {
    // (2)
    FailOnEmpty(*vd[_k] >= _last_min + 1);
    for (i = _k; i < _l; i += 1) {
      FailOnEmpty(*vd[i + 1] >= vd[i]->getMinElem() + 1);
    }
    FailOnEmpty(*vd[_l] <= _last_max - 1);
    for (i = _l; i > _k; i -= 1) {
      FailOnEmpty(*vd[i - 1] <= vd[i]->getMaxElem() - 1);
    }
    _OZ_DEBUGPRINTTHIS("(2) ");
  }
  {
    // (3)
    _OZ_DEBUGPRINT(("_k=%d _l=%d",_k, _l));

    if (_k == 0) { // TMUELLER
      for (i = OZ_getFSetInf(); i < vd[0]->getMinElem(); i += 1)
        FailOnInvalid(*s -= i);
    } else {
      for (i = vd[_k - 1]->getMaxElem() + 1; i < vd[_k]->getMinElem(); i += 1)
        FailOnInvalid(*s -= i);
    }

    if (_l == _vd_size - 1) { // TMUELLER
      for (i = OZ_getFSetSup(); i > vd[_l]->getMaxElem(); i -= 1)
        FailOnInvalid(*s -= i);
    } else {
      for (i = vd[_l + 1]->getMinElem() - 1; i > vd[_l]->getMaxElem(); i -= 1)
        FailOnInvalid(*s -= i);
    }

    _OZ_DEBUGPRINTTHIS("(3) ");
  }

  {
    // (4)
    for (i = _k; i <= _l; i += 1)
      if (*vd[i] == fd_singl)
        FailOnInvalid(*s += vd[i]->getMinElem());

    _OZ_DEBUGPRINTTHIS("(4) ");
  }

  {
    // (5)
    OZ_FSetValue glb_s = s->getGlbSet(), lub_s = s->getLubSet();
    FSetIterator glb_it(&glb_s, _last_min), lub_it(&lub_s, _last_min);

    int min_lub = lub_it.getNextLarger(), min_glb = glb_it.getNextLarger();
    for ( ; min_lub == min_glb && min_lub != -1;
          min_lub = lub_it.getNextLarger(),
            min_glb = glb_it.getNextLarger(), _k += 1 ) {
      FailOnEmpty(*vd[_k] &= min_glb);
      _last_min = min_lub;

      _OZ_DEBUGPRINTTHIS("(5) ");
    }

    // (6)
    if (_k != _l) {
      lub_it.init(_last_max);
      glb_it.init(_last_max);
      int max_lub = lub_it.getNextSmaller(), max_glb = glb_it.getNextSmaller();
      for ( ; max_lub == max_glb && max_lub != -1;
            max_lub = lub_it.getNextSmaller(),
              max_glb = glb_it.getNextSmaller(), _l -= 1) {
        FailOnEmpty(*vd[_l] &= max_glb);
        _last_max = max_lub;
      }

      _OZ_DEBUGPRINTTHIS("(6) ");
    }
  }

#ifndef MATCH_NOLOOP
  for (new_size = 0, i = _k; i <= _l; i += 1)
    new_size += vd[i]->getSize();

  if (((old_size != new_size && new_size > _vd_size) || st <= s) &&
      _k < _vd_size && _l > -1) {
    old_size = new_size;
    goto loop;
  }
#endif

  _OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// minN propagator

OZ_C_proc_begin(fsp_minN, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMinNPropagator(OZ_args[0],
                                          OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetMinNPropagator::header = fsp_minN;

#define MATCH_NOLOOP

OZ_Return FSetMinNPropagator::propagate(void)
{
  _OZ_DEBUGPRINTTHIS("\nin ");

  OZ_FSetVar s(_s);
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, _vd_size);
  PropagatorController_S_VD P(s, _vd_size, vd);
  int max_fd = OZ_getFDInf();
  int min_fd = OZ_getFDSup();
  int i;

  for (i = _vd_size; i--; ) {
    vd[i].read(_vd[i]);
    min_fd = min(min_fd, vd[i]->getMinElem());
    max_fd = max(max_fd, vd[i]->getMaxElem());
  }

  if (_firsttime) {
    _OZ_DEBUGPRINT(("firsttime==1"));

    _firsttime = 0; // do it only once

    _k = 0; _l = _vd_size - 1;
    _last_min = s->getLubSet().getMinElem() - 1;
    _last_max = s->getLubSet().getMaxElem() + 1;

    // (1)
    //
    FailOnInvalid(s->putCard(_vd_size, 32 * fset_high));
    _OZ_DEBUGPRINTTHIS("(1) ");
  }

#ifndef  MATCH_NOLOOP
  int old_size, new_size;
  FSetTouched st;

  for (old_size = 0, i = _k; i <= _l; i += 1)
    old_size += vd[i]->getSize();
#endif

loop:
  _OZ_DEBUGPRINT(("_k=%d _l=%d _last_min=%d _last_max=%d min_fd=%d max_fd=%d",
                  _k, _l , _last_min,  _last_max, min_fd,  max_fd));

#ifndef MATCH_NOLOOP
  st = s;
#endif

  {
    // (2)
    FailOnEmpty(*vd[_k] >= _last_min + 1);
    for (i = _k; i < _l; i += 1) {
      FailOnEmpty(*vd[i + 1] >= vd[i]->getMinElem() + 1);
    }

    FailOnEmpty(*vd[_l] <= _last_max - 1);
    for (i = _l; i > _k; i -= 1) {
      FailOnEmpty(*vd[i - 1] <= vd[i]->getMaxElem() - 1);
    }

    _OZ_DEBUGPRINTTHIS("(2) ");
  }
  {
    // (3)
    _OZ_DEBUGPRINT(("_k=%d _l=%d",_k, _l));

    if (_k == 0) { // TMUELLER
      for (i = OZ_getFSetInf(); i < vd[0]->getMinElem(); i += 1)
        FailOnInvalid(*s -= i);
    } else {
      for (i = vd[_k - 1]->getMaxElem() + 1; i < vd[_k]->getMinElem(); i += 1)
        FailOnInvalid(*s -= i);
    }
    _OZ_DEBUGPRINTTHIS("(3) ");
  }

  {
    // (4)
    for (i = _k; i <= _l; i += 1)
      if (*vd[i] == fd_singl)
        FailOnInvalid(*s += vd[i]->getMinElem());

    _OZ_DEBUGPRINTTHIS("(4) ");
  }

  {
    // (5)
    OZ_FSetValue glb_s = s->getGlbSet(), lub_s = s->getLubSet();
    FSetIterator glb_it(&glb_s, _last_min), lub_it(&lub_s, _last_min);

    int min_lub = lub_it.getNextLarger(), min_glb = glb_it.getNextLarger();
    for ( ; min_lub == min_glb && min_lub != -1;
          min_lub = lub_it.getNextLarger(),
            min_glb = glb_it.getNextLarger(), _k += 1 ) {
      FailOnEmpty(*vd[_k] &= min_glb);
      _last_min = min_lub;

      _OZ_DEBUGPRINTTHIS("(5) ");
    }
  }

#ifndef MATCH_NOLOOP
  for (new_size = 0, i = _k; i <= _l; i += 1)
    new_size += vd[i]->getSize();

  if (((old_size != new_size && new_size > _vd_size) || st <= s) &&
      _k < _vd_size && _l > -1) {
    old_size = new_size;
    goto loop;
  }
#endif

  _OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  return P.fail();
}

//-----------------------------------------------------------------------------
// maxN propagator

OZ_C_proc_begin(fsp_maxN, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectFSetVarAny);
  OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);

  return pe.impose(new FSetMaxNPropagator(OZ_args[0],
                                          OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun FSetMaxNPropagator::header = fsp_maxN;

#define MATCH_NOLOOP

OZ_Return FSetMaxNPropagator::propagate(void)
{
  return OZ_ENTAILED;
}


//-----------------------------------------------------------------------------
// eof
